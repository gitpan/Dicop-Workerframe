/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for config file management for Dicop workers.
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDCFG_H
#define DICOP_PWDCFG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pwd_util.h"

/** when searching for a key, return 0 if it doesn't exist */
#define CFG_NOFAIL 0
#define CFG_OPTIONAL 0
/** when searching for a key, die/exit if it doesn't exist */
#define CFG_FAIL 1
#define CFG_MANDATORY 1

/** @brief Defines one key=value pair in a Dicop config file.

    Lengths include the terminating zero, so 'abc\\0' has len==4
*/
struct ssKey
  {
  unsigned int len;			/**< length of key */
  unsigned int valuelen;		/**< length of value */
  char key[128];			/**< the key name */
  char* value;				/**< ptr to the value of this key */
  struct ssKey* next;			/**< next key in the double linked list */
  struct ssKey* prev;			/**< prvious key in the double linked list */
  };

/** Read in the entire config file. */
struct ssKey* pwdcfg_read ( const char* filename );

/** Find a specific key or return NULL. */
struct ssKey* pwdcfg_find_key (
	const struct ssKey* keys,
	const char* key,
	const unsigned long fail);

/** Check that the all keys in the comma separated list exist. */
unsigned long pwdcfg_keys_exist (
	const struct ssKey* keys,
	const char* keynames,
	const unsigned long options );

/** Find a key, and return it's value as int. */
int pwdcfg_as_int (
	const struct ssKey* cfg,
	const char* keyname,
	const unsigned long fail);

/** Check all keys against a list of valid ones and exit()/return error on
   invalid ones. */
unsigned long pwdcfg_valid_keys (
	const struct ssKey* cfg,
	const char* keynames,
	const unsigned long options );

#endif
