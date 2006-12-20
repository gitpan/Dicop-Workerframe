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
  /* call hexdump with no footer and 2 spaces as indend */
  hexdump_fancy (title, "", data, size, "  ");
  }

void hexdump_fancy(const char* title, const char* footer, const void *data,
		const unsigned int size, const char* indend)
  {
  /* Dump SIZE bytes of *DATA like:
     0000  75 6E 6B 6E 6F 77 6E 20  30 FF 00 00 00 00 39 00 unknown 0.....9.
   */
  const unsigned char *p = data;
  unsigned int i, n;
  
  if (NULL != title && title[0] != 0)
    {
    printf("%s\n",title);
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
    printf ("%.4x  ", n );
    /* print 16 bytes w/ space at 8 */
    for ( i = 0; n+i < size && i < 16; i++)
      {
      printf("%02X ", *(p + i));
      if (i == 7) { printf("  "); }
      }
    /* pad with spaces */
    while (i++ < 16)
      {
      if (i == 8) { printf("  "); }
      printf("   ");
      }
    /* print text dump w/ space at 8 */
    printf(" ");
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
      /* extra space after 8 bytes */
      if (i == 7) { printf(" "); } 
      }
    printf ("\n");
    p += 16;
    }
 
  if (NULL != footer && footer[0] != 0)
    {
    printf("%s\n",footer);
    }
  }
