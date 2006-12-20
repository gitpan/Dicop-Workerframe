/**
 * @defgroup workerframepwdtest Worker for testsuite
 * @ingroup workerframe 
 *
 * @brief Dicop test worker for testsuite.
 *
 * This program works just like the testworker, but it prints the tested
 * passwords, too.
 *
 * It is used by the testsuite to check whether the password generator works,
 * and the worker frame in itself is checking the arguments properly.
 *
 * @copydoc copyrighttext
*/

/**
 * @file
 * @ingroup workerframepwdtest
 * @brief Dicop test worker - test passwords and print them to STDOUT for testsuite.
 * 
 * @copydoc copyrighttext
 * 
 * This program works just like the testworker, but it prints the tested
 * passwords, too. This enables to test for the right sequence of passwords
 * being generated by the password generator.
 *
 * Return codes in dofunction: 
 * -  PWD_SUCCESS - found password
 * -  PWD_FAIL    - not found
 * -  PWD_ERROR   - critical error occured
*/

#include <dicop_unused.h>

/* make these globals so we can use them in dofunction, too */
char target[512];

/* called once when the worker starts */
void printinfo(void)
  {
  printf ("DiCoP Testsuite worker v0.09 (C) Copyright by BSI 1998-2006\n\n");
  }

/* called once when the worker starts */
int initfunction(const struct ssPWD *pwd)
  {
  target[0] = 0;		/* zero terminate */
  if (pwd->prefix_length != 0)
    {
    strncpy(target,pwd->prefix,128);		/* store prefix */
    a2h(target,pwd->prefix_length);
    /* append target password in ascii */
    strncat(target,pwd->target,128);
    }
  else
    {
    /* copy target */
    strncpy(target,pwd->target,128);
    }
  printf ("Target key is '%s'\n",target);	
  return PWD_INIT_OK;				/* init was okay */
  }

/* called for each password */
int dofunction( const struct ssPWD *pwd )
  {
  char b[1024];
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

