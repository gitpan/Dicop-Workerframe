/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

/* when searching for a key, return 0 if it doesn't exist */
#define CFG_NOFAIL 0
/* when searching for a key, die/exit it doesn't exist */
#define CFG_FAIL 1

/* one key=value pair */

struct ssKey
  {
  /* lengths include the terminating zero, so 'abc\x00' has len==4 */
  unsigned int len;			/* length of key */
  unsigned int valuelen;		/* length of value */
  unsigned char key[128];		/* the key */
  unsigned char* value;			/* ptr to the value */
  struct ssKey* next;			/* double linked list */
  struct ssKey* prev;			/* double linked list */
  };

/* read in the config file */
struct ssKey* pwdcfg_read ( unsigned char* filename );

/* find a specific key or return NULL */
struct ssKey* pwdcfg_find_key (
	struct ssKey* keys,
	unsigned char* key,
	unsigned long fail);

/* check that the all keys in the comma separated list exists */
unsigned long pwdcfg_keys_exist (
	struct ssKey* keys,
	unsigned char* keynames,
	unsigned long options );

/* Find a key, and return it's value as int. */
int pwdcfg_as_int (
	struct ssKey* cfg,
	unsigned char* keyname,
	unsigned long fail);

/* check all keys against a list of valid ones and exit()/return error on
   invalid ones */
unsigned long pwdcfg_valid_keys (
	struct ssKey* cfg,
	unsigned char* keynames,
	unsigned long options );

