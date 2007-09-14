/*!
 * @file
 * @ingroup workerframe
 * @brief A handy hexdump routine.
 * 
 * @copydoc copyrighttext
*/

#include <hexdump.h>

/** Dump SIZE bytes of *DATA with an header like:

    Header:
      0000  75 6E 6B 6E 6F 77 6E 20  30 FF 00 00 00 00 39 00 unknown 0.....9.
*/

void hexdump(const char* title, const void *data, const unsigned int size)
  {
  /* call hexdump with no footer and 2 spaces as indend, one extra space
     in the center, with adress in front, and a newline in the title */
  hexdump_fancy (title, NULL, data, size, "  ", " ", " ", 1, 1);
  }

/** Dump SIZE bytes of *DATA with an header, but no address, like:

    Password:
      75 6E 6B 6E 6F 77 6E 20 30 00                   unknown
*/

void hexdump_pwd(const char* title, const void *data, const unsigned int size)
  {
  /* call hexdump with no footer and 2 spaces as indend, no extra space 
     in the center, no adress in front */
  hexdump_fancy (title, NULL, data, size, "  ", " ", "", 0, 0);
  }

/** Dump SIZE bytes of *DATA without any header or address like:

  75 6E 6B 6E 6F 77 6E 20 30 00                   unknown
*/

void hexdump_line(const void *data, const unsigned int size)
  {
  /* call hexdump with no footer and no title, no spaces as indend,
     no extra space in the center, no adress in front */
  hexdump_fancy (NULL, NULL, data, size, "", " ", "", 0, 0);
  }

/** Dump SIZE bytes of *DATA without any header or address like:

  Salt:  75 6E 6B 6E 6F 77 6E 20 30 00                   unknown
*/

void hexdump_inline(const char* title, const void *data, const unsigned int size)
  {
  /* call hexdump with no footer, no spaces as indend,
     no extra space in the center, no adress in front */
  hexdump_fancy (title, NULL, data, size, "", " ", "", 0, 0);
  }

/** Dump SIZE bytes of *DATA like:\n
   @verbatim
  0000  75 6E 6B 6E 6F 77 6E 20  30 FF 00 00 00 00 39 00 unknown 0.....9. @endverbatim
*/
void hexdump_fancy(const char* title, const char* footer, const void *data,
		const unsigned int size, const char* indend,
		const char* sep, const char* center_sep, const int adr,
		const int newline)
  {
  const unsigned char *p = data;
  unsigned int i, n;

  /* switch to ASCII locale to avoid multi-byte character issues */  
  setlocale(LC_ALL, "C");

  if (NULL != title && title[0] != 0)
    {
    printf("%s",title);
    if (newline != 0) { printf("\n"); }
    }

  /* take 16 bytes for each line */
  for (n = 0; n < size; n += 16)
    {
    /* print indendation */
    if (NULL != indend)
      {
      printf ("%s", indend );
      }
    /* print offset/address */
    if (adr != 0) { printf ("%.4x  ", n ); }
    /* print 16 bytes (with spaces in between?) */
    for ( i = 0; n+i < size && i < 16; i++)
      {
      printf("%02X%s", *(p + i), sep);
      if (i == 7 && NULL != center_sep) { printf("%s", center_sep); } 
      }
    /* pad with spaces */
    while (i++ < 16)
      {
      printf("  %s", sep);
      if (i == 8 && NULL != center_sep) { printf("%s", center_sep); }
      }
    /* print text dump (with one space in front) */
    printf (" ");
    for ( i = 0; n+i < size && i < 16; i++)
      {
      if (isprint(*(p + i)) == 0)  
        {
        printf(".");
        }
      else
        {
        printf("%c", *(p + i));
        }
      /* extra space after 8 bytes? */
      if (i == 7 && NULL != center_sep) { printf("%s", center_sep); } 
      }
    printf ("\n");
    p += 16;
    }
 
  if (NULL != footer && footer[0] != 0)
    {
    printf("%s\n",footer);
    }

  /* switch back to I18N locale */  
  setlocale(LC_ALL, "");
  }
