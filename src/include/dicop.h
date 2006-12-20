/*!
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

/* include the current version, revision and last-modifed defines */
#include "patch.h"

/*@{ 

  return codes for dofunction(): */

#define PWD_SUCCESS	2
#define PWD_FAIL	0
#define PWD_ERROR	7
#define PWD_TIMEOUT	4

/*@}*/ 

/* return code(s) for initfunction() */

#define PWD_INIT_OK 0

/* other things we generally want to include */

#include <hexdump.h>
#include <pwd_par.h>

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

/************************************************************************* */
/* These macros are defined only when tomcrypt.h was included */

#ifdef CRYPT
#ifdef SCRYPT

int _crypt_err;

/* hash */

#define SELFTEST_MD5 if ((_crypt_err = md5_test()) != CRYPT_OK) {\
    printf("MD5 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_MD4 if ((_crypt_err = md4_test()) != CRYPT_OK) {\
    printf("MD4 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_SHA1 if ((_crypt_err = sha1_test()) != CRYPT_OK) {\
    printf("SHA1 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_SHA256 if ((_crypt_err = sha256_test()) != CRYPT_OK) {\
    printf("SHA256 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

/* crypt/encrypt */

#define SELFTEST_AES if ((_crypt_err = aes_test()) != CRYPT_OK) {\
    printf("AES did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_BLOWFISH if ((_crypt_err = blowfish_test()) != CRYPT_OK) {\
    printf("BLOWFISH did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_TWOFISH if ((_crypt_err = twofish_test()) != CRYPT_OK) {\
    printf("TWOFISH did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_DES if ((_crypt_err = des_test()) != CRYPT_OK) {\
    printf("DES did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_DES3 if ((_crypt_err = des3_test()) != CRYPT_OK) {\
    printf("DES3 (triple DES) did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

/** Hash the given LEN bytes pointed to by SRC with SHA1, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[32];

    hash_with_sha1(digest,pwd->pwd,pwd->len,&ctx);
*/
#define hash_with_sha1(dst,src,len,ctx) sha1_init(ctx); sha1_process(ctx,(unsigned char*)src,len); sha1_done(ctx,dst);

/** Hash the given LEN bytes pointed to by SRC with MD5, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[32];

    hash_with_md5(digest,pwd->pwd,pwd->len,&ctx);
*/
#define hash_with_md5(dst,src,len,ctx) md5_init(ctx); md5_process(ctx,(unsigned char*)src,len); md5_done(ctx,dst);

/** Hash the given LEN bytes pointed to by SRC with SHA256, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[32];

    hash_with_sha256(digest,pwd->pwd,pwd->len,&ctx);
*/
#define hash_with_sha256(dst,src,len,ctx) sha256_init(ctx); sha256_process(ctx,(unsigned char*)src,len); sha256_done(ctx,dst);

#endif
#endif

#endif
