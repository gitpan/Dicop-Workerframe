/**
 * @file
 * @ingroup workerframe
 * @brief Defines constants, macros and often used functions for Dicop workers
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_H
#define DICOP_H

/* define _DEBUG for MSVC users */
#ifdef DEBUG
#define _DEBUG
#endif

/* turn off the deprecated warnings from VC8+ */
#if (_MSC_VER >= 1400)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#endif

/* include the current version, revision and last-modifed defines */
#include "patch.h"

/*@{ 

  return codes for dofunction(): */

#define PWD_SUCCESS	2
#define PWD_FAIL	0
#define PWD_ERROR	7
#define PWD_TIMEOUT	4

/*@}*/ 

/* return code(s) for initfunction() and misc. functions */

#define PWD_INIT_OK 0
#define PWD_OK 0

/* other things we generally want to include */

#include <hexdump.h>
#include <pwd_par.h>
#include <pwd_conv.h>
/* macros and shortcuts for libtomcrypt support */
#include <pwd_ltc.h>

/* function prototypes */

/** Convert a ASCII string to a zero terminated hexadecimal string. Example: "a" to "41\0". */
void	a2h		(char * a, const unsigned int len);
/** Convert a zero terminated hexadecimal string back to ASCII. Example: "41\0" to "a\0" */
void	h2a   		(char * h);

/** Compare two strings. */
int 	cmpStrings	(const char* a, const char* b, const int la, const int lb);

void	printinfo	(void);
int	initfunction	(const struct ssPWD *pwd);
int	dofunction	(const struct ssPWD *pwd);
void	stopfunction	(void);
int	endfunction	(const struct ssPWD *pwd);

/* handy macros */

/** Print out date/timestampe info about the file that is compiled. */

#define PRINT_INFO printf (" Compiled on %s %s from file:\n  '%s'\n\n", __DATE__, __TIME__, __FILE__)

#endif
