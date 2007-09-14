/*!
 * @file
 * @ingroup workerframe
 * @brief Password/string extraction from images.
 * 
 * @copydoc copyrighttext
*/

/*
#define DEBUG_SORT 		1
*/

#include <pwd_img.h>

/* ************************************************************************* 

 INTERNAL:
  Read from a file blockwise, filling a buffer.
  Extract all strings of a given length, dropping shorter and ignoring
  longer strings. Returns a sorted list of unique strings all having the
  same length 'len' (in bytes, not characters!).
  The number of extracted strings is stored in 'pwds'.

  This routine is repeatedly called with len going from MIN_LEN to MAX_LEN,
  reads in data blockwise and passes it to pwdgen_extract.

  Strings are terminated at "invalid" characters (depending on charset or
  extracton mode like UTF_8), or their maximum length N (len). Optionally,
  "invalid" bytes are just ignored.
  e.g. [A|B|C|0x00|F] (N = 3) would result in ABC, BC, C, and F beeing
  extracted.
  However, at stage N (len == N) we discard all strings that are shorter due
  to the following reasons:
    if N is MIN_LEN, we do not want to test strings shorter than len N
    if N is not MIN_LEN, we already did test strings with len N-1, N-2 etc
  Therefore, in the above example, with N == 3, only ABC would be extracted,
  while BC, C and F would be ignored.

  *valid contains table of 256 flags, 0 for invalid, 1 for valid character
  skip is the number of bytes to skip ahead, usually 1. Set to 2 to extract every
  odd/even byte (with suitable buffer start of [0] or [1]).

  return codes: ptr to list of pwd's	- okay
		NULL 		 	- error	*/

/* maximum file input buffer size */

#ifdef DEBUG
#define BUFSIZE 81
#else
#ifdef DEBUG_SORT
#define BUFSIZE 120
#else
#define BUFSIZE 128 * 1024
#endif
#endif

#define PRINT_DEBUG(msg) if (0 != (options & EXTRACT_DEBUG)) { printf (msg); }
#define PRINT_TRACE(msg) if (0 != (options & EXTRACT_TRACE)) { printf (msg); }
#define TRACE(msg) if (0 != (options & EXTRACT_TRACE)) { msg; }

unsigned long pwdgen_img_read ( 
  struct ssPWD* pwd,
  const unsigned int len,
  const char* filename,
  unsigned int* valid,
  const unsigned long options)
  {
  
  FILE *file;

  /* We use a slightly larger buffer, and place the read data into its end.
     The space in front of the actual data will be zeros for the first loop,
     which will be ignored be the extraction routine, and for subsequent
     loops it will be filled with the last bytes from the previous buffer.
     This ensures that it will work with overlap, e.g. if the image is
     much larger than the BUFSIZE, which it usually is. The overlap between
     the different parts of the image is ensured by the server giving out
     overlapping image parts to each client, so we need not to care for that
     here. */

  unsigned char buffer[BUFSIZE + 256 + 1];	/* input buffer, slightly larger to
						   have space for +len bytes */
  unsigned char *pwd_list;			/* all extracted pwds */
  unsigned long p = 0;				/* count extracted from file */
  unsigned long sorted = 0;			/* how many are already sorted */
  unsigned char *pwd_buf;			/* extracted pwds from buffer */
  unsigned long extracted;			/* count extracted from buffer */
  unsigned long ofs;				/* offset into read buffer */
  long len_adjust;				/* len-ofs */
  long read;					/* bytes read from file per call to read() */
  unsigned long rc;				/* return code from img_check */
  off_t size, read_bytes;			/* file size, file already read in total */
  double percent_done;
  unsigned int buffer_size;

  unsigned int max_pwds;			/* how many to extract at max before
						   handing them to check */
  unsigned int max_merge;			/* how many to extract before merging
						   them with the rest */
  unsigned long appended;			/* for merging in extracted strings */

  /* these three are for converting mutations and stages */
  unsigned int mutations = 0;
  unsigned int stages = 0;
  unsigned int bits;
  time_t t;					/* temp. time variable */

  /* get X, sort them, merge them in, get next X */
  buffer_size = BUFSIZE;

  /* count the bits in stages */
  bits = pwd->todo_stages;
  while (bits != 0)
    {
    stages += (bits & 1);
    bits >>= 1;
    }

  /* count the bits in mutations */
  bits = pwd->todo_mutations;
  while (bits != 0)
    {
    mutations += (bits & 1);
    bits >>= 1;
    }

  /* extracting with many mutations, so limit the buffer size */
  if (3 < stages * mutations)
    {
    /* stages=1, mutations=5 => 5, => BUF_SIZE / 5 */
    printf ("\n Dropping buffer size from %i to ", buffer_size);
    buffer_size = 2 * buffer_size / (stages * mutations);
    printf ("%i to limit memory usage.", buffer_size);
    }
  /* when extracting very long passwords, limit the buffer size, too */
  if (len >= 128)
    {
    buffer_size = buffer_size / 2;
    }
  if (len >= 64)
    {
    buffer_size = buffer_size / 2;
    }
  /* 32 .. 64 => /2, 64 ..128 => /4, 128..256 => /8 */
  if (len >= 32)
    {
    buffer_size = buffer_size / 2;
    }

  /* if the buffer has buffer_size bytes, we can extract at most so many strings,
     each starting at the next position, plus one as safety margin. */
  max_merge = buffer_size + 1;
  if (0 != (options & EXTRACT_EVEN_ODD))
    {
    /* if odd/even, too, we expect twice (1 + 1/2 + 1/2) as many. */
    max_merge *= 2;
    }
  /* If we mutate the extracted passwords, the worst case is that each string
     mutates through each stage/mutation: */
  if ((stages != 0) && (mutations != 0))
    {
    max_merge *= (stages * mutations) + 1; /* 2 stages, 1 mutation => 2 plus
					      one in original extracted form */
					   /* 2 stages, 10 mutations plus one
					      => 21 times as many */
    printf ("\n %i stages * %i mutations + 1 original = %i combinations per extracted string.",
      stages, mutations, stages * mutations + 1); 
    }
  max_merge += 16;		/* safety margin */

  /* how many pwds to extract at once */

  /* if we extract/sort/merge/gather more passwords, then the sorting stage
     will take a much longer time. The time does not scale linear, e.g.
     doubling the number of passwords more than doubles the time to sort and
     merge them. However, gathering more strings potentially removes more
     doubles, so we save time on checking the strings later. Since usually the
     sorting stage is much faster than the checking stage, we can gather quite
     a few strings at a time. */
  max_pwds = 1024 * 1024;	/* one meg of pwds at once */

  if (len <= 6)
    {
    max_pwds *= 7;		/* 2,3,4,5,6 => max. 42 Mb memory */
    }
  else 
    {
    if (len <= 8)
      {
      max_pwds *= 5;		/* 7,8 => max. 40 Mb memory */
      }
    else
      {
      if (len <= 10)
        {
        max_pwds *= 4;		/* 9,10,11,12 => max. 40 Mb memory */
        }
      else
        {
        if (len <= 12)
          {
          max_pwds *= 3;		/* 9,10,11,12 => max. 36 Mb memory */
          }
        }
      }
    }

  /* if we extract X passwords at a time, but can only store Y, it will crash */ 
  if (max_pwds <= max_merge)
    {
    max_pwds = max_merge + 1; 	/* +1 one safety */
    } 

  printf ("\n Will extract %i strings at a time, until I have %i strings.", max_merge, max_pwds);

  if (len > PWD_LEN)
    {
    printf ("Error: Cannot extract strings of len %i: maximum is %i.\n", len, PWD_LEN);
    return PWD_ERROR;
    }
  
  printf ("\n Opening image file '%s' ('%s') ...\n", filename, "rb");
  if (NULL == (file = pwdgen_find_file(filename,"rb")))
    {
    return PWD_ERROR;
    }
  fflush(NULL);

  /* get file size */
  size = pwdgen_file_size(file);
  /* and seek to the beginning again */
  fseek(file,0, SEEK_SET);

  /* allocate for maximum number of strings from buffer */
  pwd_buf = malloc (len * max_merge);
#ifdef DEBUG
  printf ("alloc %p with %i bytes\n", pwd_buf, max_merge * len);
#endif

  if (pwd_buf == NULL)
    {
    printf ("Error: Cannot allocate list of strings, size %i bytes.\n",
     max_merge * len);
    return PWD_ERROR;
    }
  memset (pwd_buf, 0, max_merge * len);

  /* allocate for maximum number of strings from file */
  pwd_list = malloc (len * max_pwds);
#ifdef DEBUG
  printf ("alloc %p with %i bytes\n", pwd_list, max_pwds * len);
#endif

  if (pwd_list == NULL)
    {
    printf ("Error: Cannot allocate list of strings, size %i bytes.\n",
     max_pwds * len);
    return PWD_ERROR;
    }
  memset (pwd_list, 0, max_pwds * len);

  /* run in a loop until the entire file has been done */

  /* clear buffer (would only need buffer_size bytes to clear, but play safe) */
  memset (buffer, 0, buffer_size+len);

  read_bytes = 0;
  rc = PWD_FAIL;
  ofs = len;			/* first buffer: start at len, after this at 0 */
  len_adjust = len - ofs;
  while (!feof(file) && (rc == PWD_FAIL))
    {
    /* read data into buffer (after first len bytes) */
    read = fread(&buffer[len], 1, buffer_size, file);
    read_bytes += read;

#ifdef DEBUG
    printf ("Read %li bytes (%li bytes total), ofs %li, len %i\n", read, read_bytes, ofs, len);
#endif

    /* feof or some error, or not enough bytes read to extract? */
    if (read + len_adjust <= 0)
      {
      break;
      }
    if (0 != (options & EXTRACT_UTF8))
      {
      extracted = pwdgen_img_extract_utf8(buffer+ofs, pwd_buf, read + (len-ofs), len, options, pwd);
      }
    else
      {
      /* extract strings from buffer from 0, at every byte */
      extracted = pwdgen_img_extract(buffer+ofs, pwd_buf, read + (len-ofs), 
	 	len, valid, options, 1, pwd);

      /* extract odd/even, but do not this if EXTRACT_UTF8 is on */
      if (0 != (options & EXTRACT_EVEN_ODD))
        {
        TRACE( printf ("  Extracted %i at each byte. Now even: ", (unsigned int)extracted) );
        /* extract strings from buffer from 0, at every other byte */
        extracted += pwdgen_img_extract(buffer+ofs, &pwd_buf[extracted*len],
	  read + (len-ofs), len, valid, options, 2, pwd);
        TRACE( printf (" extracted %i all in all. Now odd: ", (unsigned int)extracted) );
        /* extract strings from buffer from 1, at every other byte, if enough bytes read */
        if ((read + len_adjust - 1) > 0)
	  {
          extracted += pwdgen_img_extract(buffer+ofs + 1, &pwd_buf[extracted*len],
	   read + len_adjust - 1, len, valid, options, 2, pwd);
	  }
        }
      }

    /* when skipping invalids, don't set ofs to 0 (e.g. ignore first bytes */
    if (0 == (options & EXTRACT_SKIP_INVALIDS))
      {
      ofs = 0;				/* after first buffer, start at 0 */
      len_adjust = len - ofs;
      /* keep the overlapping bytes by moving them to the front*/
      memmove (&buffer[0], &buffer[len], len);
      }
   
    /* output trace? */
    TRACE ( 
      printf (" Extracted %i", (unsigned int)extracted);
      if (extracted != 0)
	{
        printf (", ");
        }
      else
	{
	printf (".");
	}
     ); /* end of trace */

    if (extracted > 0)
      {
      PRINT_TRACE("merging... ");

      appended = extracted;
      /* if we have no passwords yet in buffer, just copy them over */
      if (p == 0)
	{
        memcpy (pwd_list,  pwd_buf, appended * len);
        p = appended;
	sorted = 0;		/* reset sorted count JIC */
	}
      else
	{
        memcpy (&pwd_list[p*len],  pwd_buf, appended * len);
        p += appended;

        }
      PRINT_TRACE("done.\n");

      if (p >= (max_pwds - max_merge))
        {
	/* the strings from 0 .. sorted-1 are already sorted, so we need only
	   to sort from sorted .. p-1, and then merge the two sorted lists */

        if (sorted > 0)
	  {  
	  appended = p - sorted;
	  if (0 != (options & EXTRACT_DEBUG))
	    {
	    printf (" Sorting %li strings... ", appended);
	    }
	  pwdgen_img_sort (&pwd_list[sorted*len], appended, len);
          PRINT_DEBUG(" merging... ");
	  pwdgen_img_merge (pwd_list, sorted, appended, len);
	  }
	else
	  {
	  if (0 != (options & EXTRACT_DEBUG))
	    {
	    printf (" Sorting %li strings... ", p);
	    }
	  pwdgen_img_sort (pwd_list, p, len);
	  }
        PRINT_DEBUG(" removing doubles...");

        p = pwdgen_img_remove_doubles_in_place (pwd_list, p, len);
	if (0 != (options & EXTRACT_DEBUG))
	  {
          printf (" got %li unique strings.\n", p);
          fflush(NULL);
          }
	/* these are now sorted and unique, so remember the count */
	sorted = p;
        }

      /* after sort and weed-out are there still too many? */
      if (p >= (max_pwds - max_merge))
        {
        if (0 != (options & EXTRACT_DEBUG))
          {
          t = time(NULL);
          printf (" %s Going to check %li strings of len %i.\n", ctime(&t), p, len );
          fflush(NULL);
          }
        /* too many in pwd_list, so check them and empty list */
        rc = pwdgen_img_check ( pwd, pwd_list, p, len);
        percent_done = 100.0 * read_bytes / size; 
        t = time(NULL);
        printf (" %s At %0.2f%%. Checked %li pwd of len %i.\n", 
          ctime(&t), percent_done, p, len);
        pwdgen_took(pwd);
        printf (" Done %.0Lf pwds in %.0Lf seconds (%.0Lf pwds/s).\n", 
		  pwd->pwds, pwd->took, pwd->pwds / pwd->took );
        fflush(NULL);
        /* clear list and set count of sorted part to 0 */
        p = 0; sorted = 0;
        }
      }
    else
      {
      PRINT_TRACE("\n");
      }
    /* end of file or found something (or error) */
    }
  
  /* left over in list? check them! */
  if (p > 0)
    {
    printf (" At 100%%. Sorting %i leftover strings... ", (unsigned int)p);
    pwdgen_img_sort (pwd_list, p, len);
    printf ("done.");
    printf (" Weeding out doubles: ");
    p = pwdgen_img_remove_doubles_in_place (pwd_list, p, len);
    printf (" %i survived,\n", (unsigned int)p);
    fflush(NULL);

    rc = pwdgen_img_check ( pwd, pwd_list, p, len);
    printf (" Checked %li pwds of len %i. ", p, len);
    pwdgen_took(pwd);
    printf (" Overall, I did %.0Lf pwds in %.0Lfs (%.0Lf pwds/s).\n",
        pwd->pwds, pwd->took, pwd->pwds / pwd->took );
    }
  printf (" Freeing memory.\n");
  fflush(NULL);
  free (pwd_list); free(pwd_buf);
  return rc;
  }

/* ************************************************************************* */
/** INTERNAL: Extract all valid UTF-8 strings of a given length in bytes,
              dropping shorter strings.
   See pwdgen_read_img() for further details.\n
   options & EXTRACT_SKIP_INVALIDS\n
	0: break on invalid bytes\n
	1: skip invalid bytes\n
*/
  
unsigned long pwdgen_img_extract_utf8 (
      const unsigned char* buffer, 
      unsigned char* pwd_list,
      const unsigned long bufsize,
      const unsigned long len,
      const unsigned long options,
      struct ssPWD* pwd)
  {
  unsigned long extracted, i, j, cur_len, last, need, pos, stop;
  unsigned char cur;
  struct sPwdString* org_word;

  extracted = 0;
  /* org_word and word are read-only: */
  org_word = (struct sPwdString*)pwd->org_word;

  /* for mutation/stages */
  org_word->bytes = len;
  org_word->characters = -1;

  /* for all bytes in buffer except last len */
  last = bufsize - len;
  /* current pos in the buffer */
  i = 0;
  while (i < last)
    {

#ifdef DEBUG
    fflush(NULL); printf ("At %li in buffer\n", i); sleep(1);
#endif

    cur = buffer[i]; 

    /* UTF-8 encoded strings have the following bit properties:
       0x00000000 - 0x0000007F:
           0xxxxxxx

       0x00000080 - 0x000007FF:
           110xxxxx 10xxxxxx

       0x00000800 - 0x0000FFFF:
           1110xxxx 10xxxxxx 10xxxxxx

       0x00010000 - 0x001FFFFF:
           11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

       0x00200000 - 0x03FFFFFF:
           111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

       0x04000000 - 0x7FFFFFFF:
           1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    /* Scan ahead to a valid start position for a string */
    if ((cur < 0x1f) || ((cur & 0xC0) == 0x80) || ((cur & 0xFE) == 0xFE))
      {
#ifdef DEBUG
      printf (" Ignoring invalid byte %02X at pos %li\n", cur, i);
#endif
      i++; continue;
      }
    /* cur is now either 0x20 .. 0x7f, or a valid start byte */

    /* if the remaining bytes in the buffer do not give us a full string, stop right here */
    if (i + len > bufsize)
      {
#ifdef DEBUG
      printf (" At pos %i + len %i bytes we would exceed bufsize %i\n", i, len, bufsize);
#endif
      break;
      }

    /* len in bytes of current string */
    cur_len = 0; pos = i;
    /* as long as we do not hit the end or have still bytes to extract */
    while ((cur_len < len) && (pos < bufsize))
      {
      /* first we look for a valid single byte */
      cur = buffer[pos]; 
      pwd_list[cur_len] = cur;
      if ( ((cur & 0x80) == 0) && (cur > 0x1F))
        {
#ifdef DEBUG
        printf (" Found normal byte 0x%2X at pos %li\n", cur, pos);
#endif
        cur_len++;
        }
      /* any 10xxxxxx or 1111111x or 0x0 .. 0x1F byte is considered, so skip ahead */
      else if (
             ((cur & 0xC0) != 0x80) && 
             ((cur & 0xFE) != 0xFE) &&
             (cur > 0x1f) )
        {
#ifdef DEBUG
        printf (" Found start byte %02X\n", cur);
#endif
	/* now cur matches any of the valid start bytes, so find out which */
	/* cur matches 0xFE */
	need = 5;
	if ((cur & 0xE0) == 0xC0)
	  {
	  need = 1;
	  }
	else if ((cur & 0xF0) == 0xE0)
	  {
	  need = 2;
	  }
	else if ((cur & 0xF8) == 0xF0)
	  {
	  need = 3;
	  }
	else if ((cur & 0xFC) == 0xF8)
	  {
	  need = 4;
	  }
	 /* else: cur & 0xFE matches 0xFC */
#ifdef DEBUG
	printf (" Expect %i continuation bytes (cur_len %i, pos %i).\n", need, cur_len, pos);
#endif
	/* ">=" to consider the the start byte, too */
	if (pos + need >= bufsize)
	  {
#ifdef DEBUG
          printf (" At pos %i + 1 start byte + %i cont. bytes we would exceed bufsize %i\n", pos, need, bufsize);
#endif
          break;
	  }
	/* ">=" to consider the the start byte, too */
	if (cur_len + need >= len)
	  {
#ifdef DEBUG
          printf (" At cur_len %i + 1 start byte + %i cont. bytes we would exceed len %i\n", cur_len, need, len);
#endif
          break;
	  }
	for (j = 1; j <= need; j++)
	  {
	  cur = buffer[pos+j]; 
	  pwd_list[cur_len+j] = cur;
#ifdef DEBUG
	  printf ("  Found continuation byte %02X\n", cur);
#endif
	  /* check the bytes */
	  if ((cur & 0xC0) != 0x80)
	    {
	    /* invalid follow-up byte, advance to the byte after it and retry */
#ifdef DEBUG
	    printf (" Invalid followup at pos %i, going to %i\n", pos+j, pos + j - 1);
#endif
	    pos = pos + j - 1;
	    break;
	    }
	  }
	if (j > need)
	  {
	  /* all continuation bytes were ok */
	  cur_len += j;
	  }
        } /* end found valid start byte */
      pos++;
      }

    /* found len valid bytes? */
    if (cur_len == len)
      {
#ifdef DEBUG
      printf (" Reached len %i, adding one password\n", len);
      hexdump_line (pwd_list, len);
#endif
      /* If it's the first extracted string, or the last string and this one
         are different, then include it.
         If all strings are unique, makes it a bit slower (1%), but if many of
         them are alike (like a stream of all-alike bytes) then it saves a lot
         of time in sorting later on - up to a factor 10! */
      if ((0 == extracted) ||
        (0 != _compare( pwd_list - len, pwd_list, len)))
        {
        extracted++;		/* extracted one */
        pwd_list += len;	/* set to next pwd */

        /* If we are told to mutate the extracted passwords, do so now and
	   include them in the list right-away. This will allow the
	   sorting/remove doubles stages later on to remove double-mutations
	   like '0000' beeing the same in forward/reverse/lower/upper case.

	   lowercase/uppercase etc mutations through the dict generator and feed
	   result to dofunction(), too
           XXX TODO: manually generate mutations via sub routine (faster?)
         */
        if ((pwd->todo_mutations != 0) && (pwd->todo_stages != 0))
          {
          /* put current extracted string into org_word */
	  memcpy (org_word->content, pwd_list - len, len);
	  org_word->encoding = UTF_8;
	  org_word->characters = -1;
	  org_word->content[len] = 0;			/* zero terminate just in case */

	  pwd->done_mutations = pwd->todo_mutations;
	  pwd->done_stages = pwd->todo_stages;
	  pwd->cur_mutation = 1;
	  pwd->cur_stage = 1;
	  pwd->last_word = 0;			/* last word read? not yet! */
	  pwd->append = 0;			/* no append/prepend at start */
	  pwd->prepend = 0;			/* first pwd to be prepended, if so */

#ifdef DEBUG
	  pwdgen_print (pwd, "Mutating to: '%s'\n", pwd->org_word);
#endif
	  pwddict_stage( pwd );			/* cover the first stage */

	  do
	    {
	    stop = pwddict_next( pwd );
#ifdef DEBUG
	    printf ("Mutated to: '%s'\n", pwd->pwd);
#endif
	    /* add the mutated word into the extracted-list */
            for (j = 0; j < len; j++)
              {
	      *pwd_list++ = pwd->pwd[j];
              }
	    extracted++;			/* extracted one */
            } while ( stop != PWD_ERROR );
	  }
	}
      }
    /* try at the next byte in the buffer */
#ifdef DEBUG
    printf (" Advancing to pos %li\n", i+1);
#endif
    i++;
    }

  return extracted;
  }

/* ************************************************************************* */
/** INTERNAL: Extract all strings of a given length, dropping shorter strings.\n
   See pwdgen_read_img() for further details.\n
   options & EXTRACT_SKIP_INVALIDS\n
	0: break on invalid bytes\n
	1: skip invalid bytes\n
   skip = how many bytes to advance after each character (1 or 2)
*/
  
unsigned long pwdgen_img_extract (
      const unsigned char* buffer, 
      unsigned char* pwd_list,
      const unsigned long bufsize,
      const unsigned long len,
      unsigned int *valid,
      const unsigned long options,
      const unsigned long skip,
      struct ssPWD* pwd)
  {
  unsigned long extracted, i, j, n, stop;
  struct sPwdString* org_word;

  if (bufsize <= len)
    {
    return 0;
    }

  extracted = 0;
          
  /* org_word and word are read-only: */
  org_word = (struct sPwdString*)pwd->org_word;

  /* for mutation/stages */
  org_word->bytes = len;
  org_word->characters = -1;

#ifdef DEBUG
    printf ("extracting to buffer pos '%p', options %li\n", pwd_list, options);
#endif

  /* for all bytes in buffer except last len */
  for (i = 0; i < (bufsize - len); i += skip )
    {
#ifdef DEBUG
    printf ("At %li in buffer\n",i);
#endif

    /* if extract_skip_invalids was requested, skip them, otherwise
       break on them */

    if (0 != (options & EXTRACT_SKIP_INVALIDS))
      {
      /* skip invalids branch - skip bytes at current position (otherwise we
	would need to skip them over and over) */

      /* as long as we have invalids, skip forward */
      while (valid[buffer[i]] == 0)
        {
        i += skip;
        if (i >= (bufsize - len))
	  {
	  /* end of buffer */
          break;
          }
        }
#ifdef DEBUG
    printf ("Initial skipped to byte %li in buffer\n",i);
#endif
      j = 0; 				/* amount extracted */
      n = i;				/* ptr into buffer */
      while (n < bufsize && j < len)
	{
        if (valid[buffer[n]] != 0)
	  {
          pwd_list[j++] = buffer[n];
	  }
	n += skip;
        }
#ifdef DEBUG
      printf (" Extracted %li bytes\n",j);
#endif
      }
    else
      {
      /* break on invalids branch */

      j = 0; 				/* amount extracted */
      n = i;				/* ptr into buffer */
      while (n < bufsize && j < len)
	{
#ifdef DEBUG
      printf (" Looking at %i (%c)\n", buffer[n], buffer[n]);
#endif
        if (valid[buffer[n]] == 0)
	  {
	  break;
	  }
        pwd_list[j++] = buffer[n];
	n += skip;
	}
      /* the string at the current position is too short, this means
 	 we can skip len(string) bytes forward, because all other
	 strings will also be shorter.
	 [A|B|C|D|E|\x00|A|B|...] If we see that BCDE is to short, CDE,
	 DE and E will also be too short, so we skip to the next A.
	 This speeds up the scanning a bit.
       */
      if (j != len)
        {
#ifdef DEBUG
        printf ("skip ahead from %li to %li\n",i, n + skip);
#endif
        i = n;	/* set to next sensible pos (i will be "incremented" next) */
        }
      } /* end extract code */

    /* all went ok, extracted len characters */
    if (j == len)
      {
      /* If it's the first extracted string, or the last string and this one
	 are different, then include it. 
	 If all strings are unique, makes it a bit slower (1%), but if many of
	 them are alike (like a stream of all-alike bytes) then it saves a lot
	 of time in sorting later on - up to a factor 10! */
      if ((0 == extracted) ||
	(0 != _compare( pwd_list - len, pwd_list, len)))
	{
	extracted++;		/* extracted one */
	pwd_list += len;	/* set to next pwd */
#ifdef DEBUG
	printf ("Not skipped (pwds %li, pwd_list %p)\n",extracted, pwd_list);
#endif
	/* If we are told to mutate the extracted passwords, do so now and
	   include them in the list right-away. This will allow the
	   sorting/remove doubles stages later on to remove double-mutations
	   like '0000' beeing the same in forward/reverse/lower/upper case.
    
           lowercase/uppercase etc mutations through the dict generator and feed
           result to dofunction(), too
	   XXX TODO: manually generate mutations via sub routine (faster?)
	 */

        if ((pwd->todo_mutations != 0) && (pwd->todo_stages != 0))
          {
          /* put current extracted string into org_word */
	  memcpy (org_word->content, pwd_list - len, len);
	  org_word->encoding = pwd->generator_encoding;
	  org_word->characters = -1;
	  org_word->content[len] = 0;			/* zero terminate just in case */

	  pwd->done_mutations = pwd->todo_mutations;
	  pwd->done_stages = pwd->todo_stages;
	  pwd->cur_mutation = 1;
	  pwd->cur_stage = 1;
	  pwd->last_word = 0;			/* last word read? not yet! */
	  pwd->append = 0;			/* no append/prepend at start */
	  pwd->prepend = 0;			/* first pwd to be prepended, if so */
#ifdef DEBUG
	  printf ("Mutating to: '%s'\n", pwd->org_word);
#endif
	  pwddict_stage( pwd );			/* cover the first stage */

	  do
	    {
	    stop = pwddict_next( pwd );
#ifdef DEBUG
	    printf ("Mutated to: '%s'\n", pwd->pwd);
#endif
	    /* add the mutated word into the extracted-list */
            for (j = 0; j < len; j++)
              {
	      *pwd_list++ = pwd->pwd[j];
              }
	    extracted++;			/* extracted one */
            } while ( stop != PWD_ERROR );
          } /* endif mutations and stages */
	}
      }
    }
  return extracted;
  }

unsigned long pwdgen_img_sort (
      const unsigned char* pwd_list,
      const unsigned long pwds,
      const unsigned long len
      )
  {
  unsigned char* temp;			/* temp for sorting */

  /* one or less need not be sorted */
  if (pwds < 2)
    {
    return pwds;
    }

  /* allocate temp storage */
  temp = malloc (len * pwds);

  if (temp == NULL)
    {
    printf (" Warning: Cannot allocate scratch memory for merge sort.\n");
    /* fail softly, e.g. just don't sort */
    return pwds;
    }

  /* sort strings */
  pwdgen_img_merge_sort (pwd_list, temp, 0, pwds-1, len);

  /* free scratch pad */
  free (temp);

  return pwds;
  }

/* ************************************************************************ */
/* take list of (probably sorted) extracted passwords, and check them all by
   feeding them to dofunction() 
   return code PWD_FAIL or PWD_SUCCESS
  */

unsigned int pwdgen_img_check (
	struct ssPWD* pwd, 
	const unsigned char* pwd_list,
	const unsigned long pwds,
	const unsigned long len )
  {
  unsigned long j, checked;
  struct sPwdString *cur;

  /* cur is read-only: */
  cur = (struct sPwdString *)pwd->cur;

  /* nothing to do? */
  if (pwds == 0)
    {
    printf (" Nothing to check.\n");
    return PWD_FAIL;
    }

  pwd->length = len;		/* set to current length, does not change */
  pwd->overflow = len;		/* set all to overflowed (not optimal) */

  cur->bytes = len;
  cur->characters = -1;
  cur->encoding = pwd->generator_encoding;

  checked = 0;			/* none yet */
  while (checked++ < pwds)
    {
    /* patch the current password into the pwd struct */
    for (j = 0; j < len; j++)
      {
      pwd->pwd[j] = *pwd_list++;
      }
    pwd->pwd[j] = 0;			/* zero terminate just in case */
#ifdef DEBUG
    printf ("Checking: '%s'\n", pwd->pwd);
#endif
    /* if pwd->cur->content differs from pwd->pwd: */
    if (pwd->pwd != pwd->cur->content)
      {
#ifdef DEBUG
      printf ("Updating cur\n");
#endif
     /* XXX TODO: update cur with all requested encodings and call dofunction() for each */
      pwdgen_update_cur(pwd, 1);
      }
    /* check the password and if the check was successful, return */
    if (0 != dofunction( pwd ))
      {
      pwdgen_add( pwd, checked);
      return PWD_SUCCESS;
      }
    }
  /* add the number of passwords we check to the global counter */
  pwdgen_add( pwd, pwds);
  return PWD_FAIL;
  }

/* ************************************************************************ */
/* extract passwords, weed out doubles, then call dofunction() for each pwd */

int pwdgen_extract (struct ssPWD* pwd)
  {
  struct ssKey *image_file;
  unsigned int startlen, endlen;
  unsigned int len;
  unsigned int valid[256];	/* for each possible byte: valid or not */
  unsigned long i, rc;
  unsigned int options;

  image_file = pwdcfg_find_key(pwd->config,"image_file", CFG_FAIL);
  startlen = pwdcfg_as_int( pwd->config,"start", CFG_FAIL );
  endlen = pwdcfg_as_int( pwd->config,"end", CFG_FAIL );
 
  /* if these keys exist, use them, otherwise default to 0 */ 
  options = (pwdcfg_as_int(pwd->config,"extract_skip_invalids", CFG_NOFAIL) & 0x1);

  i = (pwdcfg_as_int(pwd->config,"debug_trace", CFG_NOFAIL) & 0x3);
  if (i >= 1)
    {
    options += EXTRACT_DEBUG;
    if (i == 2)
      {
      options += EXTRACT_TRACE;
      }
    }

  if ( 1 == (pwdcfg_as_int(pwd->config,"extract_even_odd", CFG_NOFAIL) & 0x1))
    {
    options += EXTRACT_EVEN_ODD;
    }
  if ( 1 == (pwdcfg_as_int(pwd->config,"extract_utf8", CFG_NOFAIL) & 0x1))
    {
    options += EXTRACT_UTF8;
    }

  if (0 == (options & EXTRACT_UTF8))
    {
    /* clear table completely */
    for (i = 0; i < 256; i ++) { valid[i] = 0; }
    /* enable only the characters described as valid by the simple set */
    for (i = 0; i < pwd->extract_set->length; i++)
      {
      valid[ pwd->extract_set->chars[i] ] = 1;		/* enable this char as valid */
      }
    }

  printf ("Extracting strings from len %i to len %i with options 0x%02x.\n", startlen, endlen, options);
  fflush(NULL);

  /* ********************************************************************** */
  /* main loop from startlen ... endlen */

  /* if we don't extract any passwords at all */
  pwd->pwd[0] = 0;		/* set empty password */
  pwd->length = 0;

  for (len = startlen; len <= endlen; len++)
    {
    time_t t = time(NULL);
    printf ("%s Extracting and checking all strings with len %i...", ctime(&t), len);
    fflush(NULL);

    /* status report or timeout check? */
    if (pwdgen_time_check(pwd) != 0)
      {
      return PWD_TIMEOUT;
      }

    /* Extract strings from image and check them. */
    rc = pwdgen_img_read(pwd, len, image_file->value, valid, options);

    /* return a PWD_SUCCESS or PWD_ERROR early */
    if (rc != PWD_FAIL)
      {	
      return rc;
      }
    }

  return PWD_FAIL;
  }
