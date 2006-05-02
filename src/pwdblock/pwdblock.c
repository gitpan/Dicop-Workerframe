/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

 pwdblockt.c - Program template for calculating DiCoP test job 

 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

 This example is outdated - please use src/test_par/ as a template to develop
 worker that check passwords en block.

 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 

 This program works just like the testworker, but it prints the tested
 passwords, too. It works on the passwords in a "block" mode, e.g. gathers
 X passwords, then does them all in one go. This specifically tests the
 addition of endfunction().

 usage: test firstkey lastkey targetkey

 return codes in dofunction: 	PWD_SUCCESS - found password
				PWD_FAIL    - not found
              
 2003-04-17 v0.01 te first version from pwdtest.c
 2005-03-17 v0.02 te update (c), mark as deprecated

*/

#include "../include/pwdgen.h"
#include "../include/dicop.h"

/* make these globals so we can use them in dofunction, too */

struct ssPWD *pwd;
unsigned char target[PWD_LEN*2];
unsigned int pwds_gathered;

#define MAX_BLOCK_SIZE 16

unsigned char pwd_stack [MAX_BLOCK_SIZE][PWD_LEN];
unsigned int pwd_len_stack [MAX_BLOCK_SIZE];

#define DEBUG 0

/* called once when the worker stops */
void stopfunction(void)
  {
  }

/* called once when the worker starts */
void printinfo(void)
  {
  printf ("DiCoP Testsuite worker v0.02 (C) Copyright by BSI 1998-2005\n\n");
  }

/* called once when the worker starts */
int initfunction(struct ssPWD *password, char *targetkey)
  {
  pwd = password;		/* store struct with password */
  /* if targetkey contains a targetfile name, read it in */
  /* return read_target_file(targetkey); */
  /* otherwise proceed normal */
  target[0] = 0;		/* zero terminate */
  if (pwd->prefix_length != 0)
    {
    strncpy(target,pwd->prefix,128);		/* store prefix */
    a2h(target,pwd->prefix_length);
    /* append target password in ascii */
    strncat(target,targetkey,128);
    }
  else
    {
    /* copy target */
    strncpy(target,targetkey,128);
    }
  printf ("Target key is '%s'\n",target);	
  pwds_gathered = 0;
  return 0;			/* init was okay */
  }

/* called for each password by checkblock() or endfunction() */
int check_pwd( int index )
  {
  unsigned char b[1024];
  unsigned int i;
  unsigned int len;
  unsigned char* pwd_str;

  pwd_str = pwd_stack[index];
  len = pwd_len_stack[index];
 
  /* make copy of org password */
  for (i = 0; i < len; i++)
    {
    b[i] = pwd_str[i];
    }
  a2h(b,len);

  if (pwd->timeout == 0)				/* for testsuite */
    {
    printf ("At '%s'\n",b);
    // printf ("len %i %i\n",pwd->length,pwd->prefix_length);
    }

  /* update the crc with data that depends on the password, since we do not
     use the password to decrypt any actual data or something along these
     lines, we just add up the passwords. In reality, this should not be
     done, but rather some sort of password-depended cleartext must be used */
  pwdgen_update_crc(pwd, pwd_str, len);

  if (strcmp((char*)b,(char*)target) == 0) { return PWD_SUCCESS; }	/* found password */

  return PWD_FAIL;					/* not found yet */
  }

int check_block ( void )
  {
  int i;
  int rc;
  unsigned int max;

#ifdef DEBUG
  printf ("in check_block() with %i gathered pwds\n",pwds_gathered);
#endif

  max = pwds_gathered; pwds_gathered = 0;
  /* this is just a simple function, in reality some paralellism should be
     exploited to speed this up... */
  for (i = 0; i < max; i++)
    {
    rc = check_pwd(i);
    if (rc == PWD_SUCCESS)
      {
      /* the current pwd in the generator is not the one representing the
	 solution, so better fix the output */
      pwdgen_found( pwd, pwd_stack[i], pwd_len_stack[i]);
      return rc;
      } 
    }
  return PWD_FAIL;
  }
  
/* called once after all pwds done, to give works doing pwds in batches a
   chance to complete left-over pwds */
int endfunction(void)
  {
#ifdef DEBUG
  printf ("in endfunction() with %i gathered pwds\n",pwds_gathered);
#endif
  return check_block();
  }

/* called for each password by the pwd generator */
int dofunction( void )
  {
  int i;

#ifdef DEBUG
  printf ("in dofunction() with %i gathered pwds\n",pwds_gathered);
#endif

  /* put the current pwd into our stack */
  pwd_len_stack[pwds_gathered] = pwd->length;
  for (i = 0; i < pwd->length; i++)
    {
    pwd_stack[pwds_gathered][i] = pwd->pwd[i];
    }
  /* zero terminate */
  pwd_stack[pwds_gathered][i] = 0;

  pwds_gathered++;

  /* not yet enough? */
  if (pwds_gathered < MAX_BLOCK_SIZE)
    {
    return PWD_FAIL;
    }

  /* else check them all and return result */
  return check_block();
  }

