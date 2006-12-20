/*!
 * @file
 * @ingroup workerframe
 * @brief Utility functions.
 * 
 * Implements a few utility functions for often-used situations like
 * file I/O, error handling etc.
 *
 * @copydoc copyrighttext
*/

#include <dicop.h>

/* ********************************************************************** */

off_t pwdgen_file_size(FILE *fin)
  {
  fseeko(fin,0,SEEK_END); 
  return ftello(fin);
  }

/* ********************************************************************** */

/** Tries to find a file from its given name. Checks first in the current
    directory, then one or two directories up.

    @section XXX TODO

    This routine is platform-dependend (using ".." for updir, and only "\\"
    or "/" for the file seperator.

    Return codes:
	- ptr to FILE*: Okay, found file.
	- NULL: Error
*/

FILE *
pwdgen_find_file (const char *filename, const char *options)
  {
  char f[270];
  FILE *file;

  strncpy (f, filename, 250);
  file = fopen (f, options);
  if (NULL == file)
    {
      strcpy (f, "../");
      strncat (f, filename, 250);
      file = fopen (f, options);
      if (NULL == file)
	{
	  strcpy (f, "../../");
	  strncat (f, filename, 250);
	  file = fopen (f, options);
	  if (NULL == file)
	    {
	      strcpy (f, "..\\");
	      strncat (f, filename, 250);
	      file = fopen (f, options);
	      if (NULL == file)
		{
		  strcpy (f, "..\\..\\");
		  strncat (f, filename, 250);
		  file = fopen (f, options);
		  if (NULL == file)
		    {
		      printf ("Error: Cannot find file '%s'.\n", filename);
		      return NULL;
		    }
		}
	    }
	}
    }
  return file;
  }

/* should not be used, use pwdgen_from_hex() instead! */

void
pwdgen_convert_hex(char *target, const char *targetkey, const unsigned int len)
  {
  strncpy(target,targetkey,len);
  h2a(target);

#ifdef DEBUG
  hexdump ("Target:", target, len >> 1);
#endif
  }


