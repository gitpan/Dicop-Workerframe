/*

  Password/String extraction from images

  Copyright (c) 2003-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

//#define DEBUG			1
//#define DEBUG_SORT 		1

#include "../include/pwd_img.h"
#include "../include/pwdsort.h"
#include "../include/pwddict.h"
#include "../include/dicop.h"

/* ************************************************************************* 

 INTERNAL:
  Read from a file blockwise, filling a buffer.
  Extract all strings of a given length, dropping shorter and ignoring
  longer strings. Returns a sorted list of unique strings all having the
  same length 'len'.
  The number of extracted strings is stored in 'pwds'.

  This routine is repeatedly called with len going from MIN_LEN to MAX_LEN,
  reads in data blockwise and passes it to pwdgen_extract.

  Strings are terminated at "invalid" characters (depending on charset),
  or their maximum length N (len). Optionally, "invalid" characters are just
  ignored.
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
#define BUFSIZE 64 * 1024
#endif
#endif

unsigned long pwdgen_img_read ( 
  struct ssPWD* pwd, unsigned int len, unsigned char* filename,
  unsigned int* valid, unsigned long options, unsigned long image_type )
  {
  
  FILE *file;

  /* we use a slightly larger buffer, and place the read data into it's end.
     The space in front of the actual data will be zeros for the first loop,
     which will be ignored be the extraction routine, and for subsequent
     loops it will be filled with the last bytes from the previous buffer.
     This ensures that it will work with overlap, e.g. if the image is
     much larger than the BUFSIZE, which it usually is. The overlap between
     the different parts of the image is ensured by the server giving out
     overlapping image parts to each client, so we need not to care for that
     here. */

  unsigned char buffer[BUFSIZE + 256 + 1];	/* input buffer */
  unsigned char *pwd_list;			/* all extracted pwds */
  unsigned long p = 0;				/* count extracted from file */
  unsigned long sorted = 0;			/* how many are already sorted */
  unsigned char *pwd_buf;			/* extracted pwds from buffer */
  unsigned long extracted;			/* count extracted from buffer */
  unsigned long ofs;				/* offset into read buffer */
  long len_adjust;				/* len-ofs */
  long read;					/* bytes read from file */
  unsigned long rc;				/* return code from img_check */
  unsigned long size, read_bytes;		/* file size, file already read */
  double percent_done;
  unsigned int buffer_size;

  unsigned max_pwds;				/* how many to extract at max before
						   handing them to check */
  unsigned max_merge;				/* how many to extract before merging
						   them with the rest */
  unsigned long appended;			/* for merging in extracted strings */

  /* these three are for converting mutations and stages */
  int factor = 0;
  int factor_2 = 0;
  unsigned int bits;
  time_t t;					/* temp. time variable */

  /* get X, sort them, merge them in, get next X */
  buffer_size = BUFSIZE;
  if ((pwd->todo_mutations != 0) && (pwd->todo_stages != 0))
    {
    /* when extracting with mutations, limit the buffer size as to not get
       overly many */
    buffer_size = BUFSIZE / 8;
    }

  /* if the buffer has buffer_size bytes, we can extract at most so many strings,
     each starting at the next position. Plus one as safety margin. */
  max_merge = buffer_size + 1;
  if (0 != (options & EXTRACT_ODD_EVEN))
    {
    /* if odd/even, too, we expect twice (1 + 1/2 + 1/2) as many. */
    max_merge *= 2;
    }
  /* If we mutate the extracted passwords, the worst case is that each string
     mutates through each stage/mutation: */
  if ((pwd->todo_mutations != 0) && (pwd->todo_stages != 0))
    {
    /* count the bits in stages */
    bits = pwd->todo_stages;

    while (bits != 0)
      {
      if ((bits & 1) != 0)
	{
	factor++;
	}
      bits >>= 1;
      }
    /* count the bits */
    bits = pwd->todo_mutations;
    while (bits != 0)
      {
      if ((bits & 1) != 0)
	{
	factor_2++;
	}
      bits >>= 1;
      }
    max_merge *= (factor_2 * factor) + 1;  /* 2 stages, 1 mutation => 2 plus
					      one in original extracted form */
					   /* 2 stages, 10 mutations plus one
					      => 21 times as many */
    printf ("\n %i stages * %i mutations + 1 original = %i combinations per extracted string.",
      factor_2, factor, factor_2 * factor + 1); 
    }
  max_merge += 16;		/* safety margin */

  /* how many pwds to extract at once */
  max_pwds = 1024 * 1024;	/* one meg of pwds at once */

  if (len <= 6)
    {
    max_pwds *= 4;		/* 2,3,4,5,6 => 4 meg (max. 24 meg memory) */
    }
  else 
    {
    if (len <= 8)
      {
      max_pwds *= 3;		/* 7,8 => 3 meg (max. 24 meg memory) */
      }
    else
      {
      if (len <= 12)
        {
        max_pwds *= 2;		/* 9,10,11,12 => 2 meg (max. 24 meg memory) */
        }
      }
    }

  /* if we extract X passwords at a time, but can only store Y, it will crash */ 
  if (max_merge > max_pwds)
    {
    max_pwds = max_merge * 1.1;
    } 

  printf ("\n Will extract %i strings at a time, until I have %i strings.", max_merge, max_pwds);

  if (len > PWD_LEN)
    {
    printf ("Error: Cannot extract strings of len %i: maximum is %i.\n", len, PWD_LEN);
    return PWD_ERROR;
    }
  
  printf ("\n Opening image file '%s' ('%s') ...\n", filename, "rb");
  if (NULL == (file = pwdgen_find_file( filename, "rb")))
    {
    return PWD_ERROR;
    }

  /* get file size */
  fseek(file,0, SEEK_END); size = ftell(file); fseek(file,0, SEEK_SET);

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
  memset (buffer, 0, BUFSIZE+len);

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
    /* extract strings from buffer from 0, at every byte */
    extracted = pwdgen_img_extract(buffer+ofs, pwd_buf, read + (len-ofs), 
	len, valid, options, 1, pwd);

    /* extract odd/even? */
    if (0 != (options & EXTRACT_ODD_EVEN))
      {
      if (0 != (options & EXTRACT_TRACE))
        {
        printf ("  Extracted %i at each byte. Now even: ", (unsigned int)extracted);
        }
      /* extract strings from buffer from 0, at every other byte */
      extracted += pwdgen_img_extract(buffer+ofs, &pwd_buf[extracted*len],
	read + (len-ofs), len, valid, options, 2, pwd);
      if (0 != (options & EXTRACT_TRACE))
        {
        printf (" extracted %i all in all. Now odd: ", (unsigned int)extracted);
        }
      /* extract strings from buffer from 1, at every other byte, if enough bytes read */
      if ((read + len_adjust - 1) > 0)
	{
        extracted += pwdgen_img_extract(buffer+ofs + 1, &pwd_buf[extracted*len],
	 read + len_adjust - 1, len, valid, options, 2, pwd);
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
    if (0 != (options & EXTRACT_TRACE))
      {
      printf (" Extracted %i", (unsigned int)extracted);
      if (extracted != 0)
	{
        printf (", ");
        }
      else
	{
	printf (".");
	}
      }

    if (extracted > 0)
      {
      if (0 != (options & EXTRACT_TRACE))
        {
        printf ("merging... ");
        }

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
      if (0 != (options & EXTRACT_TRACE))
        {
        printf ("done.\n");
        }

      if (p >= (max_pwds - max_merge))
        {
	/* the strings from 0 .. sorted-1 are already sorted, so we need only
	   to sort from sorted .. p -1, and then merge the two sorted lists */

        if (sorted > 0)
	  {  
	  appended = p - sorted;
	  if (0 != (options & EXTRACT_DEBUG))
	    {
	    printf (" Sorting %li strings... ", appended);
	    }
	  pwdgen_img_sort (&pwd_list[sorted*len], appended, len);
	  if (0 != (options & EXTRACT_DEBUG))
	    {
	    printf (" merging... ");
	    }
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
	if (0 != (options & EXTRACT_DEBUG))
	  {
          printf (" removing doubles...");
	  }

        p = pwdgen_img_remove_doubles_in_place (pwd_list, p, len);
	if (0 != (options & EXTRACT_DEBUG))
	  {
          printf (" got %li unique strings.\n", p);
          }
	/* these are now sorted and unique, so remember the count */
	sorted = p;
        }

      /* after sort and weed-out there are still too many? */
      if (p >= (max_pwds - max_merge))
        {
        if (0 != (options & EXTRACT_DEBUG))
          {
          t = time(NULL);
          printf (" %s", ctime(&t));
          fflush(NULL);
          }
        /* too many in pwd_list, so check them and empty list */
        rc = pwdgen_img_check ( pwd, pwd_list, p, len);
        percent_done = 100.0 * read_bytes / size; 
        t = time(NULL);
        printf (" %s At %0.2f%%. Checked %li strings of len %i.\n", ctime(&t), percent_done, p, len );
        fflush(NULL);
        /* clear list and set count of sorted part to 0 */
        p = 0; sorted = 0;
        }
      }
    else
      {
      if (0 != (options & EXTRACT_TRACE))
        {
        printf ("\n");
        }
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
    printf (" %i survied,\n", (unsigned int)p);
    fflush(NULL);
    rc = pwdgen_img_check ( pwd, pwd_list, p, len);
    printf (" Checked %li strings of len %i.\n", p, len);
    }
  printf (" Freeing memory.\n");
  fflush(NULL);
  free (pwd_list); free(pwd_buf);
  return rc;
  }

/* ************************************************************************* */
/* INTERNAL: Extract all strings of a given length, dropping shorter strings.
   See pwdgen_read_img() for further details.
   options: & 0b00000001 = 0: break on invalid bytes
			 = 1: skip invalid bytes
   options: & 0b00000010 = 0: no trace output
			 = 1: output debug trace
   skip			 = how many bytes to advance (1 or 2)
*/
  
unsigned long pwdgen_img_extract (
      unsigned char* buffer, 
      unsigned char* pwd_list,
      long bufsize,
      unsigned long len,
      unsigned int *valid,
      unsigned long options,
      unsigned long skip,
      struct ssPWD* pwd)
  {
  unsigned long extracted, i, j, n, stop;
  unsigned char* pwd_cur;

  if (bufsize <= len)
    {
    return 0;
    }

  extracted = 0;
          
  pwd->org_word_length = len;	/* for mutation/stages */
  pwd->word_length = len;	/* for mutation/stages */

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
	   like '0000' beeing the same in forward/reverse/lower/upper case. */
        //pwd_list[j++] = buffer[n];
    
        /* lowercase/uppercase etc mutations through the dict generator and feed
           result to dofunction(), too
	   XXX TODO: manually generate mutations via sub routine (faster?)
	 */

        if ((pwd->todo_mutations != 0) && (pwd->todo_stages != 0))
          {
	  pwd_cur = pwd_list - len;
          /* put current extracted string into org_word */
          for (j = 0; j < len; j++)
            {
            pwd->org_word[j] = *pwd_cur++;
            }
          pwd->org_word[j] = 0;			/* zero terminate just in case */
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
      unsigned char* pwd_list,
      unsigned long pwds,
      unsigned long len
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

extern int   dofunction   (void);

/* ************************************************************************ */
/* take list of (probably sorted) extracted passwords, and check them all by
   feeding them to dofunction() 
   return code PWD_FAIL or PWD_SUCCESS
  */

unsigned int pwdgen_img_check 
 ( struct ssPWD* pwd, unsigned char* pwd_list, unsigned long pwds, unsigned long len )
  {
  unsigned long j, checked;

  /* nothing to do? */
  if (pwds == 0)
    {
    printf (" Nothing to check.\n");
    return PWD_FAIL;
    }

  pwd->length = len;		/* set to current length, does not change */
  pwd->overflow = len;		/* set all to overflowed (not optimal) */
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
    /* check the password and if the check was successful, return */
    if (0 != dofunction())
      {
      pwdgen_add( pwd, checked);
      return PWD_SUCCESS;
      }
    }
  pwdgen_add( pwd, checked);
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
  unsigned int options, image_type;
  struct ssCharset* extractset;	/* ptr to charset to extraction */

  image_file = pwdcfg_find_key(pwd->config, "image_file", CFG_FAIL);
  startlen = pwdcfg_as_int( pwd->config, "start", CFG_FAIL );
  endlen = pwdcfg_as_int( pwd->config, "end", CFG_FAIL );
 
  /* if these keys exist, use them, otherwise default to 0 */ 
  options = (pwdcfg_as_int(pwd->config, "extract_skip_invalids", CFG_NOFAIL) & 0x1);

  i = (pwdcfg_as_int(pwd->config, "debug_trace", CFG_NOFAIL) & 0x3);
  if (i >= 1)
    {
    options += EXTRACT_DEBUG;
    if (i == 2)
      {
      options += EXTRACT_TRACE;
      }
    }

  options += ((pwdcfg_as_int(pwd->config, "extract_even_odd", CFG_NOFAIL) & 0x1) << 1);
  image_type = pwdcfg_as_int(pwd->config, "image_type", CFG_NOFAIL);

  printf ("Extraction options: 0x%02x  Image Type: %i\n", options, image_type);

  extractset = 
    pwdgen_find_set (pwd->charsets, pwdcfg_as_int( pwd->config, "extractset_id", CFG_FAIL));

  /* does charset exist? */
  if (NULL == extractset)
    {
    printf ("Extraction charset id %i does not exist - aborting.\n",
      pwdcfg_as_int( pwd->config, "extractset_id", CFG_NOFAIL) );
    return PWD_ERROR;
    }
  /* And is it a SIMPLE one? */
  if (extractset->type != SET_SIMPLE)
    {
    printf ("Cannot use charset %i (a non-simple one) as extract set - aborting.\n",
      extractset->id);
    return PWD_ERROR;
    }

  /* clear table completely */
  for (i = 0; i < 256; i ++) { valid[i] = 0; }
  /* enable only the characters described as valid by the simple set */
  for (i = 0; i < extractset->length; i++)
    {
    valid[ extractset->chars[i] ] = 1;		/* enable this char as valid */
    }

  printf ("Extracting strings from len %i to len %i with options 0x%02x.\n", startlen, endlen, options);

  /* ********************************************************************** */
  /* main loop from startlen ... endlen */

  /* if we don't extract any passwords at all */
  pwd->pwd[0] = 0;		/* set empty password */
  pwd->length = 0;

  /* XXX TODO timeout handling is missing here */
  for (len = startlen; len <= endlen; len++)
    {
    time_t t = time(NULL);
    printf ("%s Extracting and checking all strings with len %i...", ctime(&t), len);
    fflush(NULL);

    /* Extract strings from image and check them. */
    rc = pwdgen_img_read( 
	pwd, len, image_file->value, valid, options, image_type);

    /* return a PWD_SUCCESS or PWD_ERROR early */
    if (rc != PWD_FAIL)
      {	
      return rc;
      }
    }

  return PWD_FAIL;
  }
