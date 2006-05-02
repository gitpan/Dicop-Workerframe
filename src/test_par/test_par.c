/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

 test_par.c - Show password testing in blocks
 
 return codes in dofunction: 	PWD_SUCCESS - found password
				PWD_FAIL    - not found
				PWD_ERROR   - fatal error
              
*/

#include "../include/pwd_par.h"
#include "../include/dicop.h"

/* make these globals so we can use them in dofunction, too */
struct ssPWD *pwd;
unsigned char target[PWD_LEN];
int targetlen;

#define DEBUG 1

void printinfo(void)
  {
  /* print here your version and copyright */
  printf ("DiCoP test parallel worker v0.02  (c) Copyright by BSI 1998-2006\n\n");
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

  targetlen = strlen(targetkey) / 2;

  /* print out a hexdump of our target data */
  hexdump ("\n  Target:", "", targetkey, strlen(targetkey)+1, "  ");

  /* initialize the parallel interface:
   
     possibilities for padding:

     PWD_NO_PADD		none
     PWD_PADD_TO X		padd to X bytes fixed length
     -X			 	same as previous, padd to X bytes fixed width			
     X				padd with X bytes after pwd
  */

  if ( pwdgen_par_init (
	8192,			/* we want at max 8192 passwords */
	1,			/* padded w/ 1 byte */
	0x00,			/* padded with zeros */
	pwd			/* hand over the pwd struct */
	) != PWD_INIT_OK)
    {
    printf ("\n  Error, couldn't initialize parallel interface.\n");
    return PWD_ERROR;		/* init was not okay */
    }	   

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

  /* we need first to test whether we actually got passwords via the parallel
     method: */

  /* variables we use:

  pwd->par_buff		if != NULL, contains the password buffer
  pwd->par_cnt		contains the number of pwds in par_buff buffer
  pwd->par_len		contains the length of one entry in buffer in bytes

  */

  /* get the number of pwds we got */
  unsigned long cnt = pwd->par_cnt;
  unsigned char *p = pwd->par_buff;
  unsigned long len = pwd->par_len;

#ifdef DEBUG
  printf (" DEBUG: cnt: %li, len: %li, buffer at: %p\n", cnt, len, p);
#endif

  /* do something with each password */
  while (cnt-- > 0)
    {

    /* for testsuite, output what we are doing */

    unsigned char b[1024];
    unsigned int i;

    /* make copy of org password (minus the padded zero at the end) */
    for (i = 0; i < len - 1; i++)
      {
      b[i] = p[i];
      }
    b[i] = 0;                                             /* zero terminate */
    a2h(b,len-1);

    if (pwd->timeout == 0)                                /* for testsuite */
      {
      printf ("\nAt '%s' len %li", b, len-1);
      }

    /* update the crc with data that depends on the password, since we do not
       use the password to decrypt any actual data or something along these
       lines, we just add up the passwords. In reality, this should not be
       done, but rather some sort of password-depended cleartext must be used */
    pwdgen_update_crc(pwd, p, len);

    /* now check the current pwd against the target*/
    if (targetlen == len-1 && memcmp(p, target, len-1) == 0) 
      {
#ifdef DEBUG
      printf ("  DEBUG: Found target.\n");
#endif

      /* Mark the current tested password as the last one.
         This is important for the testsuite.
      */
      memcpy (pwd->pwd, p, len-1);

      return PWD_SUCCESS;
      }

    /* advance to next password */
    p += len;
    }

  return PWD_FAIL;					/* not found yet */
  }


