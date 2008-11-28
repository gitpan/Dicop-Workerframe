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

#ifndef WIN32
#ifndef __sun
  #include <err.h>
#endif
#endif

/* ********************************************************************** */

/** Calculates and inserts the current time into the given format string
    and prints it. The given format string MUST contain "%s" and no
    other format specifiers.\n\n

    Example: pwdgen_print_time("%s\n");
    */

void pwdgen_print_time (const char* format)
  {
  time_t now;
  struct tm* tm_now;
  char time_str[128];
  int rc;

  now = time(NULL);
  tm_now = localtime(&now);
  rc = strftime(time_str, sizeof(time_str), "%c", tm_now);
  if (rc == 0)
    {
    /* error in strftime() */
    time_str[0] = 0;
    }
  printf (format, time_str);
  fflush(NULL);	/* make the output appear immidiately */
  }

void _exit_on_error(const int rc)
  {
  if (rc == -1)
    {
    /* some error happened */
#if defined (WIN32) || defined (__sun)
    printf (" Fatal IO error, aborting.\n");
    exit(-1);
#else
    err (1,NULL);		/* does not return */
#endif
    }
  /* no error, so return */
  }

/* ********************************************************************** */

off_t pwdgen_file_size(const FILE *fin)
  {
  off_t fs;
  fpos_t old_pos;

  /* clear errno */
  clearerr(fin);
  /* remember the current offset */
  _exit_on_error( fgetpos(fin, &old_pos) );
  /* seek to the end */
  _exit_on_error( fseeko(fin, 0, SEEK_END) );
  /* get the current position (at the end) */
  fs = ftello(fin);
  /* restore the position */
  _exit_on_error( fsetpos(fin, &old_pos) );
  return fs;
  }

/* ********************************************************************** */

/** Tries to find a file from its given name. Checks first in the current
    directory, then one or two directories up.

    @section XXX TODO

    This routine is platform-dependend (using ".." for updir, and only "\\"
    or "/" for the file separator.

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


