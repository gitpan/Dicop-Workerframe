/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

 test.c - Program template for calculating DiCoP test job 
 
 This program does not password check at all, e.g. it is just an empty loop.

 return codes in dofunction: 	PWD_SUCCESS - found password
				PWD_FAIL    - not found
				PWD_ERROR   - fatal error
              
*/

#include "../include/pwdgen.h"
#include "../include/dicop.h"

/* make these globals so we can use them in dofunction, too */

struct ssPWD *pwd;
unsigned char target[PWD_LEN];

void printinfo(void)
  {
  /* print here your version and copyright */
  printf ("DiCoP test worker v0.09  (c) Copyright by BSI 1998-2006\n\n");
  }

int initfunction(struct ssPWD *password, char *targetkey)
  {
  pwd = password;		/* store struct with password */

  /* You can get access to the config file via:
  struct ssConfig* config = pwd->config;
  pwdcfg_find_key( config, "keyname", CFG_NOFAIL);
  etc. */
  
  /* As an example, we just copy the target key. It is also possible
     to access it via pwd->target and pwd->targetlen */
  strcpy(target,targetkey);	/* store target password in ascii */
  h2a(target);			/* or whatever suits you */
  return PWD_INIT_OK;		/* init was okay */
  }

/* not used here */
void stopfunction( void )
  {
  }

/* called once after all pwds done, to give works doing pwds in batches a
   chance to complete left-over pwds */
int endfunction(void)
  {
  return PWD_FAIL;
  }

int dofunction( void )
  {
  /* found password? */
//  if (strncmp(pwd->pwd,target, pwd->length) == 0) { return PWD_SUCCESS; }

  return PWD_FAIL;					/* not found yet */
  }


