/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/
/* pwdtest.c - Program template for calculating DiCoP test job 
 
 this program works just like the testworker, but it prints the tested
 passwords, too.

 It is used by the testsuite to check whether the password generator works
 properly and the worker frame in itself is checking the arguments properly

 usage: test firstkey lastkey targetkey

 return codes in dofunction: 	PWD_SUCCESS - found password
				PWD_FAIL    - not found
              
*/

#include <stdio.h>
#include <stdlib.h>
#include "../include/pwdgen.h"
#include "../include/dicop.h"

/* make these globals so we can use them in dofunction, too */

struct ssPWD *pwd;
unsigned char target[512];

/* called once when the worker stops */
void stopfunction(void)
  {
  }

/* called once after all pwds done, to give works doing pwds in batches a
   chance to complete left-over pwds */
int endfunction(void)
  {
  return PWD_FAIL;
  }

/* called once when the worker starts */
void printinfo(void)
  {
  printf ("DiCoP Testsuite worker v0.09 (C) Copyright by BSI 1998-2006\n\n");
  }

/* called once when the worker starts */
int initfunction(struct ssPWD *password, char *targetkey)
  {
  pwd = password;		/* store struct with password */
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
  return 0;			/* init was okay */
  }

/* called for each password */
int dofunction( void )
  {
  unsigned char b[1024];
  unsigned int i;

  /* make copy of org password */
  for (i = 0; i < pwd->length; i++)
    {
    b[i] = pwd->pwd[i];
    }
  b[i] = 0;						/* zero terminate */
  a2h(b,pwd->length);

  if (pwd->timeout == 0)				/* for testsuite */
    {
    printf ("\nAt '%s' len %i",b,(int)pwd->length);
    }

  /* update the crc with data that depends on the password, since we do not
     use the password to decrypt any actual data or something along these
     lines, we just add up the passwords. In reality, this should not be
     done, but rather some sort of password-depended cleartext must be used */
  pwdgen_update_crc(pwd, pwd->pwd, pwd->length);

  if (strcmp(b,target) == 0) { return PWD_SUCCESS; }	/* found password */

  return PWD_FAIL;					/* not found yet */
  }

