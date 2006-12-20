/**
 * @defgroup workerframetest Test worker
 * @ingroup workerframe 
 *
 * Dicop test worker.
 *
 * @copydoc copyrighttext
*/

/**
 * @file
 * @ingroup workerframetest
 * @brief A test worker and a worker template/example.
 * 
 * @copydoc copyrighttext
 *  
 * <b>This program does not password check at all, e.g. it is just an empty loop.</b>
 *
 * Return codes in dofunction: 
 * -  PWD_SUCCESS - found password
 * -  PWD_FAIL    - not found
 * -  PWD_ERROR   - fatal error
*/

#include <dicop_unused.h>

/* make these globals so we can use them in dofunction, too */
char target[PWD_LEN];

void printinfo(void)
  {
  /* print here your version and copyright */
  printf ("DiCoP test worker v0.09  (c) Copyright by BSI 1998-2006\n\n");
  PRINT_INFO;
  }

int initfunction(const struct ssPWD *pwd)
  {
  /* You can get access to the config file via:
  struct ssConfig* config = pwd->config;
  pwdcfg_find_key( config, "keyname", CFG_NOFAIL);
  etc. */
  
  /* As an example, we convert 32 byte from the target hash */

  /* pwdgen_from_hex( target, pwd->target, 32); */

  return PWD_INIT_OK;		/* init was okay */
  }

int dofunction( const struct ssPWD *pwd )
  {
  /* found password? */
  /* if (strncmp(pwd->pwd,target, pwd->length) == 0) { return PWD_SUCCESS; } */

  return PWD_FAIL;					/* not found yet */
  }


