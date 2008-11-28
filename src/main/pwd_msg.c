/*!
 * @file
 * @ingroup workerframe
 * @brief Utility functions for error/warning message support.
 * 
 * Implements support for output of error and warning messages by number
 * instead of hard-coding the message text in the source code.
 *
 * @copydoc copyrighttext
*/

#include <dicop.h>

/* ********************************************************************** */

const char pwdgen_messages[PWDGEN_MESSAGES+1][80] = {
  "Unknown error.",
  "The file format is not recognized by this worker.",
  "The file format was recognized, but this specific version is not supported.",
  "This worker does not support --identify.",
  };

/* ********************************************************************** */

char pwdgen_msg_category[2][10] = {
  "Error",
  "Warning",
  };

/** Outputs the message with the given number.

    Example: pwdgen_msg( 400, "file.dat" );
    */

void pwdgen_print_msg (const unsigned int msg)
  {
  time_t now;
  struct tm* tm_now;
  char time_str[128];
  int rc, error, num;

  now = time(NULL);
  tm_now = localtime(&now);
  rc = strftime(time_str, sizeof(time_str), "%c", tm_now);
  if (rc == 0)
    {
    /* error in strftime() */
    time_str[0] = 0;
    }
  /* TODO: read text from file */
  error = (msg & 0x10000) >> 16;
  num = msg & 0xFFFF;
  if (num > PWDGEN_MESSAGES)
    {
    num = PWDGEN_MESSAGES;
    }
  printf ("%s\n%s #%.5X: %s\n", time_str, pwdgen_msg_category[error], msg, pwdgen_messages[num]);
  fflush(NULL);	/* make the output appear immidiately */
  }

