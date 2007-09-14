/*!
 * @file
 * @ingroup workerframe
 * @brief Character set conversions and printing of strings.
 *
 * @copydoc copyrighttext
*/

#include <pwd_conv.h>
#include <hexdump.h>
#include <pwdgen.h>

#include "encoding/mappings.h"

/* *********************************************************************** */

/** Warn about running out of memory and return -1 */
int _out_of_memory(const struct ssPWD* pwd, const char* text, const size_t size)
  {
  pwdgen_error(pwd, -1);
  printf (" Error: Out of memory allocating %lli bytes in %s.\n",
	(long long)size, text); 
  return -1;
  }

/** Creates a new, empty string, with the given size in bytes as the maximum
    buffer size. See also pwdgen_empty_string(). */
struct sPwdString* pwdgen_new_string (const struct ssPWD *pwd, const size_t size, const enum eEncodings encoding)
  {
  struct sPwdString* rc;

  if (size == 0)
    {
    printf (" Error: Cannot create string with zero-sized buffer.\n");
    pwdgen_error(pwd, -1);
    return NULL;
    }
  if (NULL == (rc = malloc (sizeof(struct sPwdString))))
    {
    printf (" Error: Cannot allocate struct for new string.\n");
    pwdgen_error(pwd, -1);
    return NULL;
    }
 
  rc->bytes = 0;
  rc->characters = 0;
  rc->encoding = encoding;
  rc->size = size;
  rc->flags = 0;

  if (NULL == (rc->_buffer = malloc (size)))
    {
    free(rc);
    _out_of_memory(pwd, "pwdgen_new_string()", size);
    return NULL;
    }
  rc->content = rc->_buffer;

  return rc;
  }

/** Creates a string from the given byte sequence, with the given
    encoding. The input is not checked nor converted. The given buffer_size
    must either be zero, at least as big as size. The string will contain
    "size" bytes in a buffer that is "buffer_size" bytes big. If the given
    buffer_size is smaller then size, it will be adjusted so that the
    data actually fits into the buffer. */
struct sPwdString* pwdgen_string (
	const struct ssPWD* pwd, const unsigned char* buffer, const size_t size, const size_t buffer_size, const enum eEncodings enc)
  {
  struct sPwdString* rc;
  size_t m_size = buffer_size;

  if (NULL == (rc = malloc (sizeof(struct sPwdString))))
    {
    _out_of_memory(pwd, "pwdgen_string()", sizeof(struct sPwdString));
    return NULL;
    }
 
  rc->flags = 0;
  rc->bytes = size;
  rc->encoding = enc;
  if (rc->encoding == UTF_32)
    {
    if (0 != (rc->bytes & 3))
      {
      printf (" Error: UTF-32 input has odd-numbered count of bytes.\n");
      pwdgen_make_string_invalid(pwd, rc);
      return NULL;
      } 
    /* each character is four bytes */
    rc->characters = size >> 2;
    }
  else if (rc->encoding == UTF_16)
    {
    if (1 == (rc->bytes & 1))
      {
      printf (" Error: UTF-16 input has odd-numbered count of bytes.\n");
      pwdgen_make_string_invalid(pwd, rc);
      return NULL;
      } 
    /* XXX TODO: handle surrogate pairs here */
    /* each character is two bytes */
    rc->characters = size >> 1;
    }
  else if (rc->encoding >= MULTIBYTE_ENCODING)
    {
    /* we do not yet know how many characters there are */
    rc->characters = (size_t)-1;
    }
  else
    {
    /* bytes == characters */
    rc->characters = size;
    }

  /* plus one byte for zero terminating */
  /* XXX TODO: 2 or 4 for UTF-16 and UTF-32? */
  if (m_size < (size + 1))
    {
    /* use the string size and make some room */
    m_size = (16 - size % 16) + size;
    }
  rc->size = m_size;
  if (NULL == (rc->_buffer = malloc (m_size)))
    {
    free(rc);
    _out_of_memory(pwd, "pwdgen_string()", m_size);
    return NULL;
    }
  rc->content = rc->_buffer;
  memcpy (rc->_buffer, buffer, size);
  /* zero terminate the buffer for access to the raw bytes */
  rc->_buffer[size] = 0;

  return rc;
  }

/** Creates a string from the given byte sequence in the given
    encoding, encoding it to the given second encoding. The input is
    checked to be well-formed. */
struct sPwdString* pwdgen_decode_string (
	const struct ssPWD* pwd, const char* buffer, const size_t size, 
	const enum eEncodings input_enc,
	const enum eEncodings output_enc)
  {
  /* create the new string */
  struct sPwdString* str = pwdgen_string(pwd, (unsigned char*)buffer, size, 0, input_enc);

  if (str == NULL)
    {
    return str;
    }

  /* check it for being valid */
  if (0 != pwdgen_check_string(pwd, str))
    {
    return NULL;
    }
 
  /* possible convert the string */ 
  if (str->encoding != output_enc)
    {
    pwdgen_convert(pwd, str, output_enc);
    }

  return str;
  }

/** Internal: Abort the string conversion with an error. */

int _convert_error(const enum eEncodings in, const enum eEncodings out, const int id)
  {
  printf ("Error: Cannot yet convert from '%s' to '%s' (#%i)\n",
    pwdgen_encoding(in), pwdgen_encoding(out), id);
  return -1;
  }

/** Convert a string in-place to the given encoding. Returns 0 for success, 
    or an error code for errors. The given replace character is used for
    any character that does not exist in the target encoding. For instance,
    if you convert "Müller" to ASCII with replace = ".", you get "M.ller".
    Setting the replacement character to 0x00 means the character will
    instead be dropped, resulting in "Mller" from the example above.\n
    Usage:\n

    @verbatim
    struct sPwdString* input;

    input = pwdgen_string(pwd, "Müller", strlen("Müller"), 0, ISO_8859_1);
    pwdgen_convert_to(pwd, input, ASCII, '?');
    if (0 != pwd->error)
      {
      // error
      }@endverbatim
    */
int
  pwdgen_convert_to (const struct ssPWD *pwd, struct sPwdString* input,
			 const enum eEncodings encoding,
			 const char replace)
  {
  size_t highbytes;
  size_t outsize, characters, outbytes, i, inbytes;
  char *output, *put;
  unsigned char cur, mask;
  unsigned long *table;
  unsigned long factor, final_byte, followers, codepoint, k, j;

#ifdef DEBUG
  printf ("\n***** Attempting to covert from '%s' to '%s' (with '%c')\n",
         pwdgen_encoding(input->encoding), pwdgen_encoding(encoding), replace);
  printf ("input content: %p\n", input->content);
  printf ("input _buffer: %p\n", input->_buffer);
  printf ("input flags: 0x%04x\n", input->flags);
  hexdump_string("input", input);
  printf ("pwd->pwd: %p\n", pwd->pwd);
  printf ("pwd->cur: %p\n", pwd->cur);
  printf ("pwd->cur->content: %p\n", pwd->cur->content);
  printf ("pwd->cur->_buffer: %p\n", pwd->cur->_buffer);

  hexdump_string("cur", pwd->cur);
#endif

  if ( (NULL == input) ||
       (!PWDSTR_IS_MUTABLE(input)) ||
       (encoding == INVALID_ENCODING) ||
       (pwd->error != 0)
     )
    {
    if (encoding == INVALID_ENCODING)
      {
      printf (" Error: Target encoding is invalid.\n");
      }
    else if (input->encoding == INVALID_ENCODING)
      {
      printf (" Error: Source encoding is invalid.\n");
      }
    else if (PWDSTR_IS_READ_ONLY(input))
      {
      printf (" Error: Cannot convert read-only string.\n");
      }
    /* XXX TODO: a non read-only but static string could maybe be converted */
    else if (PWDSTR_IS_STATIC(input))
      {
      printf (" Error: Cannot yet convert static string.\n");
      }
    else
      { 
      printf (" Error: Attempting to convert invalid input.\n");
      }
    return pwdgen_error(pwd,-1);
    } 

  if ((input->encoding == encoding) || (input->bytes == 0))
    {
    /* nothing to do */
#ifdef DEBUG
    printf ("Nothing to do (target encoding already set, or 0 bytes)\n");
#endif
    input->encoding = encoding;
    if (0 == input->bytes)
      {
      input->characters = input->bytes;
      }
    return 0;
    }

  if ((input->encoding == ASCII) &&
      (encoding != UTF_16) &&
      (encoding != UTF_32))
    {
    if (encoding > MULTIBYTE_ENCODING)
      {
      return _convert_error(input->encoding, encoding, 0);
      }
    /* else: nothing to do */
    input->encoding = encoding;
    input->characters = input->bytes;
    return 0;
    }

  /* convert a single byte encoding to UTF-8: */
  if (encoding == UTF_8)
    {
    if (input->encoding > MULTIBYTE_ENCODING)
      {
      return _convert_error(input->encoding, encoding, 1);
      }
    inbytes = input->bytes;
    highbytes = 0;
    /* to UTF-8 */
    /* count how many high bytes we have */
    for (i = 0; i < inbytes; i++)
      {
      highbytes += ((unsigned char)input->content[i] & 0x80) >> 7;
      }
    if (highbytes != 0)
      {
      table = encoding_mappings[input->encoding];
      if (NULL == table)
        {
        return _convert_error(input->encoding, encoding, 2);
        }
      /* Take the stored maximum bytes-per-character in UTF-8 that can occur: */
      outsize = highbytes * table[256] + inbytes + 1;
      /* now round the allocated memory up to a multiply of 16 */
      outsize = ((outsize / 16)+1) * 16;
      /* but don't make the new buffer smaller than the input buffer: */
      if (outsize < input->size)
	{
        outsize = input->size;
	}
      if (NULL == (output = malloc(outsize)))
        {
        return _out_of_memory(pwd, " pwdgen_convert_to()", outsize);
        }
      put = output;
      characters = inbytes;
      outbytes = 0;
      /* convert the string */
      for (i = 0; i < inbytes; i++)
        {
        cur = (unsigned char)input->content[i];
        /* printf ("Byte is 0x%02X ", cur); */
        if (0 == (cur & 0x80))
          {
#ifdef DEBUG
          printf ("Byte is lowbit, result 0x%02X ", (char)table[cur]);
#endif
          *put = (char)table[cur];
          put++;
          }
        else
          {
#ifdef DEBUG
          printf ("Byte is highbit, result 0x%08lX ", table[cur]);
#endif
          *(unsigned long*)put = table[cur];
          /* XXX TODO: support up to 6 byte UTF-8 */
          put += 2;
          if (*put != 0)
            {
            put++;
            if (*put != 0)
              {
              put++;
              }
            } 
          }
        /* printf ("\n"); */
        }
      outbytes = put - output;
      }
    else
      {
      /* highbytes == 0: only ASCII input => no conversion nec. */
      input->encoding = encoding;
      input->characters = input->bytes;
      return 0;
      }
    }
  else if (input->encoding == UTF_8)
    {
    /* from UTF-8 */
    inbytes = input->bytes;
    highbytes = 0;
    outsize = inbytes;
    if ((encoding != UTF_16) && (encoding != UTF_32))
      {
      if (encoding > MULTIBYTE_ENCODING)
        {
        return _convert_error(input->encoding, encoding, 33);
        }
      /* if the number of characters is known, and equals the number of
         bytes, we do know that we don't have highbytes. This test also
         fails for unknown characters (since then it is -1): */
      if (inbytes != input->characters)
        {
        /* count how many high bytes we have */
        for (i = 0; i < inbytes; i++)
          {
          highbytes += ((unsigned char)input->content[i] & 0x80) >> 7;
          }
        }
      if (highbytes == 0)
        {
	/* XXX TODO CHECK */
	/* we assume that 0..128 in UTF-8 can be mapped 1-to-1 to the other set */
	input->characters = input->bytes;
	input->encoding = encoding;
	/* nothing to do */
	return 0;
	}
      /* We can work in the same buffer as the output will be at most
         as long as the input (in case we have no highbytes, otherwise
         it will be shorter) */
      output = input->_buffer;
      table = encoding_tables[encoding];
      if (NULL == table)
        {
        return _convert_error(input->encoding, encoding, 4);
        }
      outbytes = 0;
      put = output;
      /* now convert the string */
      for (i = 0; i < inbytes; i++)
	{
        /* for each character in the input, compute the codepoint */
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
        cur = output[i];
        outbytes++;

        /* printf ("At pos %i byte 0x%02x\n", i, cur); */
	if (0 == (cur & 0x80))
	  {
	  *put = cur;
	  put++; continue;
	  }

	UTF_8_FOLLOWERS(cur,i,pwd,input);

        /* build the bitmask for the first byte */
        /* 8 bits minus the followers minus 2 => bits to set to 1:
           followers = 1 => 5 bits needed => 2 ** (8-1-2) -1 => 2 ** 5 -1 */
        mask = (2 << (6 - followers)) - 1;
        /* gather the first bits */
        codepoint = cur & mask;
#ifdef DEBUG 
        printf ("  Debug: Codepoint %li, mask 0x%02x.\n", codepoint, mask);
#endif
	while (followers > 0)
	  {
	  i++; followers --;
	  if (i >= inbytes)
	    {
            printf (" Error: Unexpected end of string (partial data) at pos %lu in UTF-8 string.\n",
	      (unsigned long)i);
	    return pwdgen_make_string_invalid(pwd, input);
	    }
	  cur = input->content[i];
#ifdef DEBEUG
          printf ("At follower, pos %i byte 0x%02x\n", i, cur);
#endif
          /* gather 6 more bits */
          codepoint <<= 6;
          codepoint += (cur & 0x3F);
	  }
	/* for ISO_8859_1, we can skip the table stuff */
	if (encoding == ISO_8859_1)
	  {
	  if (codepoint > 0xff)
	    {
	    codepoint = replace;
	    }
	  }
        else
	  {
	  if (codepoint == 0xfffd)
	    {
	    codepoint = replace;
	    }
	  else
	    {
            /* look up that codepoint in the in table to determine its byte value */
	    j = table[0] * 2;
	    /* each entry in the table consists of two values, the number
	       of consecutive codepoints and the starting byte */
	    final_byte = 0;
	    for (k = 1; k < j; k += 2)
	      {
	      /* printf (" entry %i (%i %i) - byte 0x%02x (%i, codepoint %i)\n", 
	  	k, table[k], table[k+1], final_byte, final_byte, codepoint); */
	      /* 5,2 => 2..6 thus "<" */
	      if ( (codepoint >= table[k+1]) &&
	           (codepoint < table[k+1] + table[k]) )
	        {
	        codepoint = final_byte + (codepoint - table[k+1]);
	        /* done */
	        break;
	        }
	      /* if the current entry has 3 codepoints, then the next starts at 3 more: */
	      final_byte += table[k];
	      }
	    if (k >= j)
	      {
	      /* char cannot be found => replace with the wanted replacement char */
	      codepoint = replace;
	      }
	    }
	  }
#ifdef DEBUG
	printf ("Replacing with codepoint %02x\n", codepoint);
#endif
	*put = (unsigned char)codepoint; put++;	
        }
      input->characters = outbytes;
      } /* end of UTF-8 => Single byte encoding */
    else
      {
      /* to UTF-16 or UTF-32 */
      if ((encoding != UTF_16) && (encoding != UTF_32))
        {
        return _convert_error(input->encoding, encoding, 3);
        }

      /* we need a new buffer since the output grows */
      factor = 1;
      if (encoding == UTF_16)
	{
	factor = 2;
	}
      if (encoding == UTF_32)
	{
	factor = 4;
	}
      outsize *= factor;
      /* now round the allocated memory up to a multiply of 16 */
      outsize = ((outsize / 16)+1) * 16;
      /* but don't make the new buffer smaller than the input buffer: */
      if (outsize < input->size)
        {
        outsize = input->size;
        }

      if (NULL == (output = malloc(outsize)))
        {
        return _out_of_memory(pwd, " pwdgen_convert_to()", outsize);
        }

      put = output;
      characters = 0; inbytes = input->bytes;
      /* now convert the string */
      for (i = 0; i < inbytes; i++)
	{
        /* for each character in the input, compute the codepoint */
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
        cur = input->content[i];
        characters++;

        /* printf ("At pos %i byte 0x%02x\n", i, cur); */
	if (0 == (cur & 0x80))
	  {
          if (encoding == UTF_16)
	    {
	    *(unsigned short*)put = (unsigned short)cur;
	    }
	  else
	    {
	    *(unsigned long*)put = (unsigned long)cur;
	    }
	  put += factor; continue;
	  }

	UTF_8_FOLLOWERS(cur,i,pwd,input);

        /* build the bitmask for the first byte */
        /* 8 bits minus the followers minus 2 => bits to set to 1:
           followers = 1 => 5 bits needed => 2 ** (8-1-2) -1 => 2 ** 5 -1 */
        mask = (2 << (6 - followers)) - 1;
        /* gather the first bits */
        codepoint = cur & mask; 
        /*  printf ("  Debug: Codepoint %li, mask 0x%02x.\n", codepoint, mask); */
	while (followers > 0)
	  {
	  i++; followers --;
	  if (i >= inbytes)
	    {
            printf (" Error: Unexpected end of string (partial data) at pos %lu in UTF-8 string.\n",
	      (unsigned long)i);
	    return pwdgen_make_string_invalid(pwd, input);
	    }
	  cur = input->content[i];
          /* printf ("At follower, pos %i byte 0x%02x\n", i, cur); */
          /* gather 6 more bits */
          codepoint <<= 6;
          codepoint += (cur & 0x3F);
	  }
        if (encoding == UTF_16)
	  {
	  *(unsigned short*)put = (unsigned short)codepoint;
	  }
	else
	  {
	  *(unsigned long*)put = codepoint;
	  }
	put += factor;
        }
      input->characters = characters;
      outbytes = put - output;
      free(input->_buffer);
      input->_buffer = output;
      input->content = output;
      }

    input->encoding = encoding;
    input->bytes = outbytes;
    input->size = outsize;
    /* zero-terminate the string */
    input->_buffer[outbytes] = 0x0;
#ifdef DEBUG
    printf ("Clearing the reminder (%i - %i = %i) of the input buffer.\n", 
	inbytes, outbytes, inbytes - outbytes);
    hexdump ("_buffer", input->_buffer, input->size);
    if (inbytes - outbytes > 1)
      {
      memset(&input->_buffer[outbytes], 0, inbytes - outbytes);
      }
#endif

    /* end UTF-8 to something */
#ifdef DEBUG
    printf ("Done encoding, result:\n");
    hexdump_string ("result:", input);
#endif
    return 0;
    }
  else
    {
    /* non UTF-8 into UTF-16 or UTF-32? */
    if (encoding == UTF_16)
      {
      /* XXX TODO: handle surrogate pairs here */
      outbytes = input->bytes * 2;
      outsize = outbytes + 2;
      }
    else if (encoding == UTF_32)
      {
      outbytes = input->bytes * 4;
      outsize = outbytes + 4;
      }
    else
      {
      /* anything else to something else */
      return _convert_error(input->encoding, encoding, 6);
      }
   
    /* now round the allocated memory up to a multiply of 16 */
    outsize = ((outsize / 16)+1) * 16;
    /* but don't make the new buffer smaller than the input buffer: */
    if (outsize < input->size)
      {
      outsize = input->size;
      }

    table = codepoint_tables[input->encoding];
    if (NULL == table)
      {
      return _convert_error(input->encoding, encoding, 7);
      }
    if (NULL == (output = malloc(outsize)))
      {
      return _out_of_memory(pwd, " pwdgen_convert_to()", outsize);
      }

    /* for each byte, convert to codepoint */
    inbytes = input->bytes;
    put = output;
    /* zero-terminate the string */
    if (encoding == UTF_32)
      {
      for (i = 0; i < inbytes; i++)
        {
        *(unsigned long*)put = table[ (unsigned char)input->content[i] ]; put += 4;
        } 
      *(unsigned long*)&output[outbytes] = 0;
      }
    else
      {
      for (i = 0; i < inbytes; i++)
        {
        *(unsigned short*)put = table[ (unsigned char)input->content[i] ]; put += 2;
        } 
      /* zero-terminate the string */
      output[outbytes] = 0x0;
      output[outbytes+1] = 0x0;
      }
    characters = inbytes;
    input->encoding = encoding;
    }

  input->encoding = encoding;
  input->size = outsize;
  /* did we actually allocate a new buffer? */
  if (input->_buffer != output)
    {
    free(input->_buffer);
    input->_buffer = output;
    input->content = output;
    }

  /* zero-terminate buffer */
  input->_buffer[outbytes] = 0;
  input->bytes = outbytes;
  input->characters = characters;
  if (input->characters == (size_t)-1) 
    {
    input->characters = pwdgen_strlen(pwd, input);
    if (pwd->error != 0)
      {
      /* strlen() failed? */
      printf (" Error: pwdgen_strlen() failed after conversion to %s",
        pwdgen_encoding(encoding));
      return -1;
      }
    }

  return 0;   
  }

/** Prints the given format string (including the given string) to the console. The given
    format string must contain exactly one '%s' directive. Example:\n
    pwdgen_print (pwd, "Password: '%s'\n", pwd->cur);
    */
void pwdgen_print(const struct ssPWD* pwd, const char* format, struct sPwdString* string)
  {
  /* do we need to convert the string? */
  if (pwd->locale != string->encoding)
    {
#ifdef DEBUG
    printf ("Converting to locale\n");
#endif
    pwdgen_convert_to(pwd, string, pwd->locale, '?');
    }
  /* Now output the string */
  printf (format, string->content);
  }

/** Dump the string contents including the encoding. */
void hexdump_string(const char* title, const struct sPwdString *string)
  {
  printf ("%s (%i characters, %i bytes, buf size: %i, flags: 0x%02x) in %s):\n", title, 
	string->characters, string->bytes, string->size, (unsigned int)string->flags,
	pwdgen_encoding(string->encoding));
  hexdump (NULL, string->content, string->bytes);
  }

/** Free the string struct including the contained buffer. */
void pwdgen_free_string (const struct ssPWD* pwd, struct sPwdString* str)
  {
  if (NULL == str)
    {
    printf (" Error: Got NULL pointer in pwdgen_free_string.\n");
    pwdgen_error (pwd, -1);
    }
  else
    {
    if (PWDSTR_IS_STATIC(str))
      {
      printf (" Error: Got static string in pwdgen_free_string.\n");
      pwdgen_error (pwd, -1);
      }
    free(str->_buffer);
    free(str);
    }
  }

/** Set the string contents to the empty string, without changing the
    string buffer size or encoding. Returns -1 if the string had an
    invalid encoding, or 0 otherwise. */
int pwdgen_empty_string(const struct ssPWD* pwd, struct sPwdString* str)
  {
  if (!PWDSTR_IS_WRITABLE(str))
    {
    return pwdgen_error (pwd, -1);
    }
  str->bytes = 0;
  str->characters = 0;
  return 0;
  }

/** Marks the string as invalid and sets the length to zero. Sets a global error
    flag in the password generator, and returns -1. */
int pwdgen_make_string_invalid(const struct ssPWD* pwd, struct sPwdString* str)
  {
  str->encoding = INVALID_ENCODING;
  str->characters = 0;
  str->bytes = 0;
  str->flags = PWDSTR_INVALID;
  return pwdgen_error (pwd, -1);
  }

/** Check the bytes of the string to be valid, e.g. inside the range
    that is specified by the encoding. As a side-effect, this routine
    will set the correct count of characters inside the string. */
int pwdgen_check_string(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned long i, followers;
  unsigned char cur;

  if (PWDSTR_IS_INVALID(string))
    {
    /* it is invalid, anyway */
    return -1;
    }
  if (string->encoding < MULTIBYTE_ENCODING)
    {
    /* no need to check this string */
    return 0;
    }

  if (string->encoding == ASCII)
    {
    /* check for a valid ASCII string, where all bytes are below 0x80 */
    i = 0;
    while (i < string->bytes)
      {
      cur = (unsigned char)string->content[i];
      if (0x80 == (cur & 0x80))
        {
        printf (" Error: Unexpected high-byte 0x%02X at pos %lu in ASCII string.\n",
	  cur, i);
        return pwdgen_make_string_invalid(pwd, string);
        }
      i++;
      }
    return 0;
    }

  if (string->encoding == UTF_32)
    {
    if (0 != (string->bytes & 3))
      {
      printf (" Error: UTF-32 string has odd-numbered count of bytes.\n");
      return pwdgen_make_string_invalid(pwd, string);
      } 
    string->characters = string->bytes >> 2;
    return 0;
    }

  if (string->encoding == UTF_16)
    {
    if (1 == (string->bytes & 1))
      {
      printf (" Error: UTF-16 string has odd-numbered count of bytes.\n");
      return pwdgen_make_string_invalid(pwd, string);
      } 
    /* XXX TODO: Handle UTF-16 surrogate pairs here */
    string->characters = string->bytes >> 1;
    return 0;
    }

  if (string->encoding != UTF_8)
    {
    printf (" Error: Cannot check string in encoding %s.\n",
	pwdgen_encoding(string->encoding));
    return pwdgen_error (pwd, -1);
    }

  string->characters = 0;
  /* check for a valid UTF-8 string */
  i = 0;
  while (i < string->bytes)
    {
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

    cur = (unsigned char)string->content[i];
    string->characters++;
    /* printf ("At pos %i byte 0x%02x\n", i, cur); */
    if (0 == (cur & 0x80))
      {
      i++; continue;
      }

    UTF_8_FOLLOWERS(cur,i,pwd,string);

    while (followers > 0)
      {
      i++; followers --;
      if (i >= string->bytes)
        {
        printf (" Error: Unexpected end of string (partial data) at pos %lu in UTF-8 string.\n",
	  i);
        return pwdgen_make_string_invalid(pwd, string);
        }
      cur = string->content[i];
      if (0x80 != (cur & 0xC0))
        {
        printf (" Error: Invalid continuation byte at pos %lu in UTF-8 string.\n",
	  i);
        return pwdgen_make_string_invalid(pwd, string);
        }
      }
    i++;
    string->characters += followers;
    }
  return 0;
  }

/** Resize the storage buffer of a string to make more or
    less room.  The min_size parameter gives the minimum number of bytes
    the string should have after this operation, and it should be
    greater than 0. 
    If the string contains more bytes than min_size specifies, it will
    be truncated.
    Returns 0 for success, or something else for an error. */
int pwdgen_resize_string(const struct ssPWD *pwd, struct sPwdString* str, const size_t min_size)
  {
  char *rc;
  long ptr_diff;
  size_t msize;

  /* STATIC? */
  if (PWDSTR_IS_STATIC(str))
    {
    printf (" Error: Cannot change buffer for static string in pwdgen_resize_string.\n");
    return pwdgen_error(pwd, -1);
    }
 
  /* read-only strings can have their buffer size changed, unless it would
     truncate the string */
  msize = min_size;
  if (msize < str->bytes)
    {
    /* XXX TODO: truncating an UTF-8 string does not work like this! */
    if (PWDSTR_IS_READ_ONLY(str))
      {
      printf (" Error: Cannot truncate read-only string in pwdgen_resize_string.\n");
      return pwdgen_error(pwd, -1);
      }
    if (str->encoding >= MULTIBYTE_ENCODING)
      {
      printf (" Error: Cannot yet truncate string with multi-byte encoding in pwdgen_resize_string.\n");
      return pwdgen_error(pwd, -1);
      }
    str->bytes = msize;
    str->characters = (size_t)-1;
    /* shortcut so we don't need to call pwdgen_strlen() */
    if (msize == 0)
      {
      str->characters = 0;
      }
    }

  /* XXX TODO: check that this really works with offsets */
  if (str->size != msize)
    {
    ptr_diff = str->content - str->_buffer;	/* have some offset? */
    rc = realloc (str->_buffer, msize);
    if (NULL == rc)
      {
      printf (" Error: Could not change string size from %lli to %lli bytes.\n",
	(long long)str->size, (long long)msize);
      return pwdgen_error(pwd, -1);
      }

    str->size = msize;
    str->_buffer = rc;
    str->content = rc + ptr_diff;
    }
  return 0;
  }

/** Count the characters in the string, set them, and also return the count. */
size_t pwdgen_strlen(const struct ssPWD *pwd, struct sPwdString* string)
  {
  long i;
  unsigned char cur;

  if (PWDSTR_IS_INVALID(string))
    {
    printf (" Error: Cannot compute strlen for string with invalid encoding.\n");
    return pwdgen_make_string_invalid(pwd, string);
    }
  if (string->characters != (size_t)-1)
    {
    /* already computed the value, so just return it */
    return string->characters;
    }

  if (string->encoding == UTF_32)
    {
    if (0 != (string->bytes & 3))
      {
      printf (" Error: UTF-32 string has odd-numbered count of bytes.\n");
      return pwdgen_make_string_invalid(pwd, string);
      } 
    string->characters = string->bytes >> 2;
    return string->characters;
    }

  if (string->encoding == UTF_16)
    {
    if (1 == (string->bytes & 1))
      {
      printf (" Error: UTF-16 string has odd-numbered count of bytes.\n");
      return pwdgen_make_string_invalid(pwd, string);
      } 
    /* XXX TODO: Handle UTF-16 surrogate pairs here */
    string->characters = string->bytes >> 1;
    return string->characters;
    }

  if (string->encoding < MULTIBYTE_ENCODING)
    {
    string->characters = string->bytes;
    return string->characters;
    }

  if (string->encoding != UTF_8)
    {
    printf (" Error: Cannot yet compute string length for encoding %s.\n",
      pwdgen_encoding(string->encoding));
    return 0;
    }

  /* the code below does not test all the bytes to be wellformed, it
     just checks the first byte of each sequence */
  string->characters = 0;
  i = 0;
  while (i < string->bytes)
    {
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

    cur = string->content[i];
    if (0 == (cur & 0x80))
      {
      i++;
      }
    else if (0xc0 == (cur & 0xe0))
      {
      i += 2;
      }
    else if (0xe0 == (cur & 0xf0))
      {
      i += 3;
      }
    else if (0xf0 == (cur & 0xf8))
      {
      i += 4;
      }
    else if (0xf8 == (cur & 0xfc))
      {
      i += 5;
      }
    else if (0xfc == (cur & 0xfe))
      {
      i += 6;
      }
    else
      {
      printf (" Error: Unexpected continuation byte at pos %li in UTF-8 string.\n",
	i);
      return pwdgen_make_string_invalid(pwd, string);
      }
    string->characters++; 
    }   
  return string->characters;
  }

/** Make a full copy of a string struct. If the given encoding is different
    from the encoding of the source string, the target string will 
    be converted to the given encoding, too. */
struct sPwdString* pwdgen_strcpy(const struct ssPWD *pwd,
    const struct sPwdString* src, const enum eEncodings encoding)
  {
  struct sPwdString* dst;
  int rc;

  if (NULL == (dst = malloc (sizeof(struct sPwdString))))
    {
    printf (" Error: Cannot allocate struct for new string.\n");
    pwdgen_error(pwd, -1);
    return NULL;
    }
 
  dst->bytes = src->bytes;
  dst->characters = src->characters;
  dst->encoding = src->encoding;
  dst->size = src->size;
  /* the copy is valid, writable and can be freed: */
  dst->flags = 0;

  if (NULL == (dst->_buffer = malloc (src->size)))
    {
    free(dst);
    _out_of_memory(pwd, "pwdgen_strcpy()", src->size);
    return NULL;
    }
  dst->content = dst->_buffer;

  memcpy (dst->_buffer, src->_buffer, src->size);
  if ((dst->bytes > 0) && (encoding != dst->encoding))
    {
    rc = pwdgen_convert(pwd, dst, encoding);
    if (rc != 0)
      {
      pwdgen_error(pwd, -1);
      pwdgen_free_string(pwd, dst);
      return NULL;
      }
    }
  dst->encoding = encoding;	/* set encoding for 0 byte strings, too */
    
  return dst;
  }

/** Duplicate contents of src into dst string. Return -1 for error, 0 if everything
    worked. */
int pwdgen_strdup(const struct ssPWD* pwd,
	struct sPwdString* dst, const struct sPwdString* src)
  {

  if (dst->size <= src->bytes)
    {
    /* add some space for zero-termination */
    if (0 != pwdgen_resize_string(pwd, dst, src->bytes + 16))
      {
      return -1;
      }
    }
  dst->encoding = src->encoding;
  dst->bytes = src->bytes;
  dst->characters = src->characters;
  /* align content at the start of the buffer */
  dst->content = dst->_buffer;
  memcpy(dst->content, src->content, src->bytes);
  /* zero-terminate just in case */
  dst->content[dst->bytes] = 0;

  return 0;
  }

/** Concatenate two strings by appending the second string to the first.
    Returns NULL for error, or the first string when everything was okay. */
struct sPwdString* pwdgen_strcat(const struct ssPWD *pwd, 
	struct sPwdString* first, const struct sPwdString* second)
  {
  struct sPwdString *s;
  size_t new_size;
  int rc;

  if (!PWDSTR_IS_MUTABLE(first))
    {
    return NULL;
    }

  /* nothing to do */
  if (second->bytes == 0)
    {
    return first;
    }

  if ((first->encoding == second->encoding) ||
      (first->bytes == 0))
    {
    /* same encoding, so we can just copy the data over */
    new_size = first->bytes + second->bytes + 1;
    /* do we need more space in the buffer? */
    if (new_size > first->size)
      {
      rc = pwdgen_resize_string(pwd, first, new_size);
      if (rc != 0)
        {
        return NULL;
        }
      }
    /* in case first->bytes == 0 and first->encoding != second->encoding */
    first->encoding = second->encoding;
    memcpy (first->content + first->bytes, second->content, second->bytes);
    first->bytes += second->bytes;
    /* zero-terminate */
    first->content[first->bytes] = 0;
    if ((first->characters != (size_t)-1) && (second->characters != (size_t)-1))
      {
      /* shortcut */
      first->characters += second->characters;
      }
    else
      {
      /* recompute */
      first->characters = (size_t)-1;
      pwdgen_strlen(pwd, first);
      }
    return first;
    }

  s = (struct sPwdString*)second;

  if (first->encoding != second->encoding)
    {
    /* we need to upgrade the first string if it isn't already Unicode */
    if ((first->encoding != UTF_8) &&
        (first->encoding != UTF_16) &&
        (first->encoding != UTF_32) )
      {
#ifdef DEBUG
      printf ("Upgrading first string to UTF-8\n");
#endif
      pwdgen_convert_to_utf8(pwd, first);
      }
    /* we need to make a temp. copy of the second string */
    if (second->encoding != first->encoding)
      {
      s = pwdgen_strcpy(pwd, second, first->encoding);
      }
    if (pwd->error != 0)
      {
      /* error */
      if (s != second && s != NULL)
        {
	printf ("Error, freeing s\n"); fflush(NULL);
        pwdgen_free_string(pwd, s);
        }
      return NULL;
      }
    }

  /* now we have the same encoding, so we can just copy the data over */
  new_size = first->bytes + second->bytes + 1;
  new_size = ((new_size / 16)+1) * 16;
  /* do we need more space in the buffer? */
  if (new_size > first->size)
    {
    rc = pwdgen_resize_string(pwd, first, new_size);
    if (rc != 0)
      {
      return NULL;
      }
    }

  memcpy (first->content + first->bytes, s->content, s->bytes);
  first->bytes += s->bytes;
  /* zero-terminate */
  first->content[first->bytes] = 0;
  if (first->characters == (size_t)-1)
    {
    /* recompute */
    first->characters = (size_t)-1;
    pwdgen_strlen(pwd, first);
    }
  else
    {
    /* pwdgen_convert() already called pwdgen_strlen() */
    first->characters += s->characters;
    }
  return first;
  }

/** Compare two strings. Return 0 for equal strings, and non-zero
    if the strings differ. If both strings are in different encodings,
    a temp. copy of the second string is made and compared against
    the first string. */
int pwdgen_strcmp(const struct ssPWD *pwd, const struct sPwdString* first, const struct sPwdString* second)
  {
  struct sPwdString *f, *s;
  int rc;

  if ( (NULL == first) || (NULL == second) ||
       (PWDSTR_IS_INVALID(first)) ||
       (PWDSTR_IS_INVALID(second)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  f = (struct sPwdString*)first;
  s = (struct sPwdString*)second;

  if (first->encoding != second->encoding)
    {
#ifdef DEBUG
    printf ("Comparing %s vs. %s\n", 
      pwdgen_encoding(first->encoding),
      pwdgen_encoding(second->encoding));
#endif

    /* we need to upgrade either one or both strings to UTF-8 */
    if (first->encoding != UTF_8)
      {
      f = pwdgen_strcpy(pwd, first, UTF_8);
      }
    if (second->encoding != UTF_8)
      {
      s = pwdgen_strcpy(pwd, second, UTF_8);
      }
    if (pwd->error != 0)
      {
      /* error */
      if (f != first)
        {
        pwdgen_free_string(pwd, f);
        }
      if (s != second)
        {
        pwdgen_free_string(pwd, s);
        }
      return -1;
      }
    }

  /* While in UTF-8, more bytes mean "bigger character code", this
     cannot be the basis for a compare:
     0x41 0xc3 0xb6 0x41 0x41			<-- 5 bytes, 4 chars
     0xc3 0xb6 0xc3 0xb6 0xc3 0xb6		<-- 6 bytes, 3 chars
  */
  if (f->characters == (size_t)-1)
    {
    pwdgen_strlen(pwd, f);
    }
  if (s->characters == (size_t)-1)
    {
    pwdgen_strlen(pwd, s);
    }
  rc = (s->characters - f->characters);
  if (rc == 0)
    {
    /* strings have equal number of characters? */
    /* XXX TODO: We should go through all codepoints
       and compare them. Here we just return wether they are
       equal, or not, but not which is "greater". */
    rc = memcmp(f->content, s->content, s->bytes);
    }
  else if (rc < 0)
    {
    rc = -1;
    }
  else
    {
    rc = 1;
    }

  /* free temp. strings */
  if (f != first)
    {
    pwdgen_free_string(pwd, f);
    }
  if (s != second)
    {
    pwdgen_free_string(pwd, s);
    }
  return rc;
  }

/** Return 1 if strings are equal, 0 otherwise. */
int pwdgen_streq(const struct ssPWD *pwd, const struct sPwdString* first, const struct sPwdString* second)
  {
  struct sPwdString *f, *s;
  int rc;

  if ( (NULL == first) || (NULL == second) ||
       (PWDSTR_IS_INVALID(first)) ||
       (PWDSTR_IS_INVALID(second)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  /* shortcut: if number of characters know, compare them */
  if ((first->characters != (size_t)-1) &&
      (second->characters != (size_t)-1) &&
      (first->characters != second->characters))
    {
    return 1;
    } 

  f = (struct sPwdString*)first;
  s = (struct sPwdString*)second;

  if (first->encoding != second->encoding)
    {
    /* make a temp. copy of first and convert it to the second encoding */
#ifdef DEBUG
    printf ("Comparing %s vs. %s\n", 
      pwdgen_encoding(first->encoding),
      pwdgen_encoding(second->encoding));
#endif

    /* we need to upgrade either one or both strings to UTF-8 */
    if (first->encoding != UTF_8)
      {
      f = pwdgen_strcpy(pwd, first, UTF_8);
      }
    if (second->encoding != UTF_8)
      {
      s = pwdgen_strcpy(pwd, second, UTF_8);
      }
    if (pwd->error != 0)
      {
      /* error */
      if (f != first)
        {
        pwdgen_free_string(pwd, f);
        }
      if (s != second)
        {
        pwdgen_free_string(pwd, s);
        }
      return -1;
      }
    }

  rc = 1;
  /* must have the same number of bytes if they are in the same encoding */
  if (f->bytes == s->bytes)
    {
    if (0 == memcmp(f->content, s->content, f->bytes))
      {
      rc = 0;
      }
    }
  if (f != first)
    {
    pwdgen_free_string(pwd, f);
    }
  if (s != second)
    {
    pwdgen_free_string(pwd, s);
    }
  return rc;
  }

/* ************************************************************************ */
/* ************************************************************************ */
/* Routines to convert between cases and retrieve/skip characters in UTF-8. */

/* Internal: convert one character from the UTF-8 buffer to lower case. */
int _utf8_char_to_lc(const struct ssPWD* pwd, const struct sPwdString* string, unsigned long* ofs)
  {
  unsigned long old, new, size_old, size_new;
  int i;
  unsigned long followers;
  unsigned char cur;

  cur = string->content[*ofs];
  UTF_8_FOLLOWERS(cur, *ofs, pwd, string);

  i = followers; old = 0;
  while (i >= 0)
    {
    old <<= 8; old += (unsigned char)string->content[*ofs+i]; i--;
    }
  /* find the new codepoint */
  new = 0;
  for (i = 0; i < CASE_FOLDINGS; i += 2)
    {
    if (utf8_to_lower[i] == old)
      {
      new = utf8_to_lower[i+1];
      break;
      }
    }
   if (new == 0)
     {
     *ofs += followers + 1;
     return 0;
     }
 
  /* patch in the new code point */
  /* first, calculate how many bytes we need to insert */
  /* single-bytes are handled seperatedly */
  size_old = 1;
  if (0 != (old & 0xFF00)) { size_old++; }
  if (0 != (old & 0xFF0000)) { size_old++; }
  if (0 != (old & 0xFF000000)) { size_old++; }

  /* but the new codepoint might be represented by a single byte: */
  size_new = 1;
  if (0 != (new & 0xFF00)) { size_new++; }
  if (0 != (new & 0xFF0000)) { size_new++; }
  if (0 != (new & 0xFF000000)) { size_new++; }

  if (size_new != size_old)
    {
    /* XXX TODO: handle here different byte lenghts */
    printf (" Error: Cannot do uc() for string in %s yet due to differently sizeed byte representations.\n",
    pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }
 
  for (i = 0; i < size_new; i++)
    {
    string->content[*ofs] = (new & 0xFF); new >>= 8; (*ofs)++;
    }
  return 1;
  }

/* Internal: convert one character from the UTF-8 buffer to UPPER case. */
int _utf8_char_to_uc(const struct ssPWD* pwd, const struct sPwdString* string, unsigned long* ofs)
  {
  unsigned long old, new, size_old, size_new;
  int i;
  unsigned char cur;
  unsigned long followers;

  cur = string->content[*ofs];
  UTF_8_FOLLOWERS(cur, *ofs, pwd, string);

  i = followers; old = 0;
  while (i >= 0)
    {
    old <<= 8; old += (unsigned char)string->content[*ofs+i]; i--;
    }
  /* find the new codepoint */
  new = 0;
  for (i = 0; i < CASE_FOLDINGS; i += 2)
    {
    if (utf8_to_upper[i] == old)
      {
      new = utf8_to_upper[i+1];
      break;
      }
    }
   if (new == 0)
     {
     *ofs += followers + 1;
     return 0;
     }
 
  /* patch in the new code point */
  /* first, calculate how many bytes we need to insert */
  /* single-bytes are handled seperatedly */
  size_old = 1;
  if (0 != (old & 0xFF00)) { size_old++; }
  if (0 != (old & 0xFF0000)) { size_old++; }
  if (0 != (old & 0xFF000000)) { size_old++; }

  /* but the new codepoint might be represented by a single byte: */
  size_new = 1;
  if (0 != (new & 0xFF00)) { size_new++; }
  if (0 != (new & 0xFF0000)) { size_new++; }
  if (0 != (new & 0xFF000000)) { size_new++; }

  if (size_new != size_old)
    {
    /* XXX TODO: handle here different byte lenghts */
    printf (" Error: Cannot do uc() for string in %s yet due to different size byte representations.\n",
    pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }
  
  for (i = 0; i < size_new; i++)
    {
    string->content[*ofs] = (new & 0xFF); new >>= 8; (*ofs)++;
    }
  return 1;
  }

/* *********************************************************************** */

/** Make the string all UPPERCASE. Returns one if the string was changed,
    zero for unchanged and -1 for errors.  */
int pwdgen_uc(const struct ssPWD* pwd, struct sPwdString* string)
  {
  int i,j, rc;
  unsigned long ofs;
  unsigned char* table, in, out;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  if (string->encoding == UTF_8)
    {
    ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      rc |= _utf8_char_to_uc(pwd, string, &ofs);
      }
    return rc;
    }

  table = uppercase_tables[string->encoding];
  if ((string->encoding >= MULTIBYTE_ENCODING) ||
     (NULL == table))
    {
    printf (" Error: Cannot do uc() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  rc = 0; j = string->bytes;
  for (i = 0; i < j; i++)
    {
    in = string->content[i]; out = table[in];
    if (in != out)
      {
      string->content[i] = out; rc |= 1;
      }
    }
  return rc;
  }

/** Make the string's first char Uppercase: "müLLer" => "MüLLer".
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_uc_first(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0;
    rc = _utf8_char_to_uc(pwd, string, &ofs);
    while (ofs < string->bytes)
      {
      rc |= _utf8_char_to_lc(pwd, string, &ofs);
      }
    return rc;
    }

  /* for single-byte encodings, just replace the bytes */
  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do uc() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  /* for single-byte encodings, just replace the first byte with the
     uppercase variant */

  rc = 0;
  in = string->content[0]; out = table_uc[in];
  if (in != out)
    {
    /* byte will be changed */
    string->content[0] = out; rc = 1;
    }

  ofs = 1;
  while (ofs < string->bytes)
    {
    in = string->content[ofs]; out = table_lc[in];
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    }
  return rc;
  }

/** Make the string's last char Uppercase: "müLLer" => "mülleR".
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_uc_last(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs, last_ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0; last_ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      last_ofs = ofs;
      rc |= _utf8_char_to_lc(pwd, string, &ofs);
      }
    ofs = last_ofs;
    rc |= _utf8_char_to_uc(pwd, string, &ofs);
    return rc;
    }

  /* for single-byte encodings, just replace the bytes */
  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do uc() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  /* for single-byte encodings, just replace the first byte with the
     uppercase variant */

  ofs = 0; rc = 0;
  while (ofs < string->bytes - 1)
    {
    in = string->content[ofs]; out = table_lc[in];
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    }
  in = string->content[ofs]; out = table_uc[in];
  if (in != out)
    {
    string->content[ofs] = out; rc = 1;
    }
  return rc;
  }

/** Make every odd character uPpErCaSe. 
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_uc_odd(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      rc |= _utf8_char_to_lc(pwd, string, &ofs);
      if (ofs < string->bytes)
	{
	rc |= _utf8_char_to_uc(pwd, string, &ofs);
	}
      }
    return rc;
    }

  /* for single-byte encodings, just replace the bytes */
  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do uc_odd() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  ofs = 0; rc = 0;
  while (ofs < string->bytes)
    {
    in = string->content[ofs]; out = table_lc[in];
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    if (ofs < string->bytes)
      {
      in = string->content[ofs]; out = table_uc[in];
      if (in != out)
        {
        string->content[ofs] = out; rc = 1;
        }
      ofs++;
      }
    }
  return rc;
  }

/** Make every even character UpPeRcAsE. 
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_uc_even(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      rc |= _utf8_char_to_uc(pwd, string, &ofs);
      if (ofs < string->bytes)
	{
	rc |= _utf8_char_to_lc(pwd, string, &ofs);
	}
      }
    return rc;
    }

  /* for single-byte encodings, just replace the bytes */
  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do uc_even() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  ofs = 0; rc = 0;
  while (ofs < string->bytes)
    {
    in = string->content[ofs]; out = table_uc[in];
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    if (ofs < string->bytes)
      {
      in = string->content[ofs]; out = table_lc[in];
      if (in != out)
        {
        string->content[ofs] = out; rc = 1;
        }
      ofs++;
      }
    }
  return rc;
  }

/** Make every vowel character UppErcAsE. 
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_uc_vowels(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      in = string->content[ofs];
      if ( (in == 'a') ||
           (in == 'o') ||
           (in == 'u') ||
           (in == 'i') ||
           (in == 'e') )
        {
        /* XXX TODO: simple do uc(in) for speed? */
        rc |= _utf8_char_to_uc(pwd, string, &ofs);
        }
      else if ( (in != 'A') &&
           (in != 'O') &&
           (in != 'U') &&
           (in != 'I') &&
           (in != 'E') )
        {
	rc |= _utf8_char_to_lc(pwd, string, &ofs);
	}
      /* aoiue */
      else { ofs++; }
      }
    return rc;
    }

  /* for single-byte encodings, just replace the bytes */
  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do uc_vowels() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  ofs = 0; rc = 0;
  while (ofs < string->bytes)
    {
    in = string->content[ofs]; out = in;
    if ( (in == 'a') ||
         (in == 'o') ||
         (in == 'u') ||
         (in == 'i') ||
         (in == 'e') )
      {
      out = table_uc[in];
      }
    else if ( (in != 'A') &&
         (in != 'O') &&
         (in != 'U') &&
         (in != 'I') &&
         (in != 'E') )
      {
      out = table_lc[in];
      } 
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    }
  return rc;
  }

/** Make every consonant character uPPeRCaSe. 
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_uc_consonants(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      in = string->content[ofs];
      if ( (in == 'A') ||
           (in == 'O') ||
           (in == 'U') ||
           (in == 'I') ||
           (in == 'E') )
        {
        /* XXX TODO: simple do uc(in) for speed? */
        rc |= _utf8_char_to_lc(pwd, string, &ofs);
        }
      else if ( (in != 'a') &&
           (in != 'o') &&
           (in != 'u') &&
           (in != 'i') &&
           (in != 'e') )
        {
	rc |= _utf8_char_to_uc(pwd, string, &ofs);
	}
      /* aoiue */
      else { ofs++; }
      }
    return rc;
    }

  /* for single-byte encodings, just replace the bytes */
  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do uc_consonants() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  ofs = 0; rc = 0;
  while (ofs < string->bytes)
    {
    in = string->content[ofs]; out = in;
    if ( (in == 'A') ||
         (in == 'O') ||
         (in == 'U') ||
         (in == 'I') ||
         (in == 'E') )
      {
      out = table_lc[in];
      }
    else if ( (in != 'a') &&
         (in != 'o') &&
         (in != 'u') &&
         (in != 'i') &&
         (in != 'e') )
      {
      out = table_uc[in];
      } 
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    }
  return rc;
  }

/* *********************************************************************** */

/** Make the string's first char lOWERCASE: "müLLer" => "mÜLLER". 
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_lc_first(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0;
    rc = _utf8_char_to_lc(pwd, string, &ofs);
    while (ofs < string->bytes)
      {
      rc |= _utf8_char_to_uc(pwd, string, &ofs);
      }
    return rc;
    }

  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do lc_first() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  /* for single-byte encodings, just replace the first byte with the
     lowercase variant */

  rc = 0;
  in = string->content[0]; out = table_lc[in];
  if (in != out)
    {
    /* byte will be changed */
    string->content[0] = out; rc = 1;
    }

  ofs = 1;
  while (ofs < string->bytes)
    {
    in = string->content[ofs]; out = table_uc[in];
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    }
  return rc;
  }

/** Make the string's last char LOWERCASe. 
    Returns one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_lc_last(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned char* table_lc, *table_uc, in, out;
  unsigned long ofs, last_ofs;
  int rc;
  enum eEncodings encoding;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  encoding = string->encoding;

  if (string->encoding == UTF_8)
    {
    ofs = 0; last_ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      last_ofs = ofs;
      rc |= _utf8_char_to_uc(pwd, string, &ofs);
      }
    ofs = last_ofs;
    rc |= _utf8_char_to_lc(pwd, string, &ofs);
    return rc;
    }

  table_lc = lowercase_tables[encoding];
  table_uc = uppercase_tables[encoding];
  if ((encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table_lc) ||
      (NULL == table_uc))
    {
    printf (" Error: Cannot do lc_first() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  /* for single-byte encodings, just replace the first byte with the
     lowercase variant */

  rc = 0; ofs = 0;
  while (ofs < string->bytes - 1)
    {
    in = string->content[ofs]; out = table_uc[in];
    if (in != out)
      {
      string->content[ofs] = out; rc = 1;
      }
    ofs++;
    }
  in = string->content[ofs]; out = table_lc[in];
  if (in != out)
    {
    string->content[ofs] = out; rc = 1;
    }
  return rc;
  }

/** Make the string's all lowercase: "MüLLer" => "müller". Returns
    one if the string was changed, zero for unchanged or -1 for errors. */
int pwdgen_lc(const struct ssPWD* pwd, struct sPwdString* string)
  {
  int i,j, rc;
  unsigned long ofs;
  unsigned char* table, in, out;

  if ( (NULL == string) || (PWDSTR_IS_INVALID(string)) ||
       (pwd->error != 0) )
    {
    return pwdgen_error(pwd, -1);
    }

  if (string->encoding == UTF_8)
    {
    ofs = 0; rc = 0;
    while (ofs < string->bytes)
      {
      rc |= _utf8_char_to_lc(pwd, string, &ofs);
      }
    return rc;
    }

  table = lowercase_tables[string->encoding];
  if ((string->encoding >= MULTIBYTE_ENCODING) ||
      (NULL == table) )
    {
    printf (" Error: Cannot do lc() for string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  rc = 0; j = string->bytes;
  for (i = 0; i < j; i++)
    {
    in = string->content[i]; out = table[in];
    if (in != out)
      {
      string->content[i] = out; rc |= 1;
      }
    }
  return rc;
  }

/* ********************************************************************** */
/** Reverse the string: "Müller" => "rellüM". Return codes:\n
    @verbatim

    -1 - error
     0 - string unchanged
     1 - string (possible) changed

    @endverbatim */
int pwdgen_strrev(const struct ssPWD* pwd, struct sPwdString* string)
  {
  unsigned long n, x, f;
  int rc;
  unsigned char *temp;
  unsigned short *sl, *sr, s;
  unsigned long *ll, *lr, l;
  unsigned char *cl, *cr, c;

  if (string->bytes == 0)
    {
    /* nothing to do */
    return 0;
    }
  if (pwd->error != 0)
    {
    /* error */
    return -1;
    }

  if (string->encoding < MULTIBYTE_ENCODING)
    {
    /* single byte encoding, so we can just flip the bytes */

    cl = string->content; cr = &string->content[ string->bytes - 1 ]; rc = 0;
    while (cl < cr)
      {
      if (*cl != *cr)
	{
        rc = 1; c = *cl; *cl = *cr; *cr = c;
	}
      cl++; cr--;
      }
    return rc;
    }

  if (string->encoding == UTF_16)
    {
    /* XXX TODO: handle surrogate pairs here */
    sl = (unsigned short*)string->content;
    sr = (unsigned short*)&string->content[ string->bytes - 2 ];
    rc = 0;
    while (sl < sr)
      {
      if (*sl != *sr)
	{
        rc = 1; s = *sl; *sl = *sr; *sr = s;
	}
      sl++; sr--;
      }
    return rc;
    }

  if (string->encoding == UTF_32)
    {
    ll = (unsigned long*)string->content;
    lr = (unsigned long*)&string->content[ string->bytes - 4 ];
    rc = 0;
    while (ll < lr)
      {
      if (*ll != *lr)
	{
        rc = 1; l = *ll; *ll = *lr; *lr = l;
	}
      ll++; lr--;
      }
    return rc;
    }

  if (string->encoding != UTF_8)
    {
    printf (" Error: Cannot reverse string in %s yet.\n",
		pwdgen_encoding(string->encoding));
    return pwdgen_error(pwd, -1);
    }

  /* flip UTF-8 characters */

  /* get a buffer big enough to hold the string copy */
  if (NULL == (temp = malloc (string->bytes)))
    {
#ifndef WIN32
    printf (" Error: Cannot allocate temp buffer (size: %lli) for reversing string.\n",
	(unsigned long long)string->bytes);
#else
    printf (" Error: Cannot allocate temp buffer for reversing string.\n");
#endif
    pwdgen_error(pwd, -1);
    return 0;
    }

  memset(temp, 0, string->bytes);
  rc = 0;
  n = string->bytes; f = 0;
  while (n > 0)
    {
    c = string->content[n-1];
    if (0 == (c & 0x80))
      {
      /* 0 .. 0x7f => single byte, copy it over */
      temp[f] = c; f++; n--;
      }
    else
      {
      if ((c & 0xc0) != 0x80)
	{
	printf ("Error: Found non-continuation byte %x at pos %li in string.\n",
		c, n);
	pwdgen_error(pwd, -1);
	return 0;
	}
      /* found a follower byte, so go backwards until we find the first
         non-follower: */
      l = 0;
      while (n > 0 && ((string->content[n-1] & 0xc0) == 0x80))
	{
	l++; n --;
	}
      /* include the start byte */
      l++; n--;
      /* copy N bytes over */
      x = 0;
      while (x < l)
	{
        temp[f] = string->content[n+x]; f++; x++;
	}
      }
    /* always report the string as changed */
    rc = 1;
    }

  /* copy the reversed string back */
  memcpy (string->content, temp, string->bytes);
  /* free the scratch buffer */
  free(temp);

  return rc;
  }

/* ********************************************************************** */

/** Convert an encoding name to the number. If the given name could not
    be matched to some known encoding, INVALID_ENCODING is returned. */
enum eEncodings pwdgen_encoding_from_name(const char* encoding)
  {
  enum eEncodings rc = 0;
  char normalized[64];
  int idx, i, len;

  /* some common alias names */
  if ( (strncmp("C",encoding,2) == 0) ||
       (strncmp("POSIX",encoding,6) == 0) )
    {
    return ASCII;
    }
  if (strncmp("euro",encoding,5) == 0)
    {
    return ISO_8859_15;
    }
  if ((strncmp("utf8",encoding,5) == 0) ||
      (strncmp("utf-8",encoding,6) == 0))
    {
    return UTF_8;
    }
  if ((strncmp("tis-620",encoding,8) == 0) ||
      (strncmp("TIS-620",encoding,8) == 0))
    {
    return ISO_8859_11;
    }
  if ( (strncmp("latin",encoding,6) == 0) ||
       (strncmp("latin1",encoding,7) == 0) ||
       (strncmp("Latin1",encoding,7) == 0) ||
       (strncmp("CP819",encoding,6) == 0) ||
       (strncmp("cp819",encoding,6) == 0) ||
       (strncmp("IBM819",encoding,6) == 0) ||
       (strncmp("ibm819",encoding,6) == 0) ||
       (strncmp("l1",encoding,3) == 0) )
    {
    return ISO_8859_1;
    }
  if ( (strncmp("wchar_t",encoding,8) == 0) ||
       (strncmp("WCHAR_T",encoding,8) == 0))
    {
    return UTF_16;
    }

  if (strncmp("big5",encoding,5) == 0)
    {
    return CP950;
    }

  while (rc <= INVALID_ENCODING)
    {
    /* normalize the name for comparisation first */
    strncpy(normalized, _encoding_names[rc], 30);
    /* CP1252 => 1252 */ 
    if (memcmp(_encoding_names[rc], "CP", 2) == 0)
      {
      strncpy(normalized, &_encoding_names[rc][2], 30); 
      }
    /* find the first space and terminate the string there, converting
       "1234 (foo)" into "1234" */
    idx = 0; len = strlen(normalized);
    while (idx < len && normalized[idx] != ' ') { idx ++; }
    normalized[idx] = 0;
    
    if (strncmp(normalized, encoding, 30) == 0)
      {
      return rc;
      }
    /* convert to lowercase */
    i = 0;
    while (i < idx && normalized[i] >= 'A' && normalized[i] <= 'Z') { normalized[i] += 0x20; i++; }
    if (strncmp(normalized, encoding, 30) == 0)
      {
      return rc;
      }

    rc++;
    }

  return INVALID_ENCODING;
  }
