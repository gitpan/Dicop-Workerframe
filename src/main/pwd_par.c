/*

  Parallel execution - gather passwords in blocks

  Copyright (c) 2005-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

#define DEBUG	1

#include "../include/pwd_par.h"
#include "../include/dicop.h"

#define MAX_COUNT 1024 * 64

extern int   dofunction   (void);

/* ************************************************************************* 
   ************************************************************************* 
   internal helper functions */

/* process pwds in buffer via dofunction() */

int pwdgen_par_process(struct ssPWD* pwd)
  {
  int found;

  found = dofunction();
  pwd->par_cnt = 0;

  return found;
  }

/* return lenght per password (derive from padding and current pwd len) */
unsigned int pwdgen_par_len (struct ssPWD* pwd )
  {

  if (pwd->par_padd < 0)
    {
    /* -X means padd to X bytes */
    
#ifdef DEBUG
//  printf ("  DEBUG: Current pwd len in pwd buffer is fixed at %i.\n", abs(pwd->par_padd));
#endif

    /* what happens if pwd is longer than X? */
    return abs(pwd->par_padd);
    }

#ifdef DEBUG
//  printf ("  DEBUG: Current pwd len in pwd buffer is %li.\n", pwd->length + pwd->par_padd);
#endif

  /* padd > 0 means "pwd" + "padding" */
  return pwd->length + pwd->par_padd;
  }

/* release memory */

void pwdgen_par_done (struct ssPWD* pwd )
  {
  /* free memory */

#ifdef DEBUG
  printf ("\n  DEBUG: Freeing memory for pwd buffer.\n");
#endif

  pwd->par_cnt = 0;
  free (pwd->par_buff);
  pwd->par_buff = NULL;
  }

/* 
   * If buffer full or pwd length changed:
     + call dofunction
     + if pwd len changed: empty buffer, allocate new one
   * push the current password into the buffer.
*/

int pwdgen_par_push (struct ssPWD* pwd)
  {
  int found = PWD_FAIL;			/* result from dofunction */
  unsigned int realloc = 0;
  unsigned int newlen = pwdgen_par_len(pwd);
  unsigned int pwdlen = pwd->length;
  unsigned char *pbuff;
  unsigned char pchar;
  unsigned int i;

  if (pwd->par_len != newlen)
    {
    realloc = 1;
    }

  if (realloc != 0 || pwd->par_cnt >= pwd->par_maxcnt)
    {
    /* buffer full or pwd length changed*/

#ifdef DEBUG
    printf ("  DEBUG: Need to clear pwd buffer via dofunction().\n");
    printf ("  DEBUG: realloc: %i pwd->par_len vs newlen: %i vs %i, cnt vs. maxcnt: %i vs %i.\n",
      realloc, pwd->par_len, newlen, pwd->par_cnt, pwd->par_maxcnt);
#endif
 
    /* handle pwds in buffer with call to dofunction() */
    found = dofunction();

    pwd->par_cnt = 0;
    pwd->par_len = newlen;

    /* need a new buffer? */
    if (realloc != 0)
      {
#ifdef DEBUG
    printf ("\n  DEBUG: Need to re-allocate pwd buffer.\n");
#endif

      pwdgen_par_done(pwd);
      pwd->par_buff = malloc ( pwd->par_maxcnt * newlen );
    
      if (pwd->par_buff == NULL)
        {
        printf ("  Error: Couldn't allocate memory for %i pwds.\n", pwd->par_maxcnt);
        pwd->par_maxcnt = 1;

        /* call dofunction() with current password (it should check
	   pwd->par_buff being NULL and then use pwd->pwd instead) */
        /* par_buff == NULL signals main later that no block gathering should
	   be done, if the user wants to continue anyway */
        return dofunction();
        }
      }
    }

#ifdef DEBUG
//    printf ("  DEBUG: Push one pwd into pwd buffer.\n");
#endif
 
  /* push the current password into the buffer */
  pbuff = pwd->par_buff + pwd->par_cnt * pwd->par_len;

  for (i = 0; i < pwdlen; i++)
    {
    *pbuff = pwd->pwd[i]; pbuff++;
    }

  /* now padd */
  pchar = pwd->par_pchar;
  while (i < pwd->par_len)
    {
    *pbuff = pchar; pbuff++; i++;
    }
  /* we now have one pwd more in our buffer */
  pwd->par_cnt++;

  /* return either PWD_FAIL (signal: continue) or result from dofunction() */
  return found;
  }

/* ************************************************************************* 
   ************************************************************************* 

   Public functions

   ************************************************************************* 
   ************************************************************************* 
 
  Setup the gathering of passwords in blocks/groups:

  return code:  == 0 - ok
		!= 0 - some error

*/

int pwdgen_par_init ( 
  unsigned int max_count,
  signed int padd,
  unsigned char paddchar,
  struct ssPWD *pwd )
  {
 
  if (max_count > MAX_COUNT)
    {
    printf ("  Warning: Collect only %i pwds instead of requested %i.\n",
     MAX_COUNT, max_count);
    max_count = MAX_COUNT;
    } 

  /* copy the data into the ssPWD struct */
  pwd->par_pchar = paddchar;
  pwd->par_padd = padd;
  pwd->par_maxcnt = max_count;
  pwd->par_cnt = 0;		/* currently none */

  pwd->par_len = pwdgen_par_len (pwd);
 
  /* allocate memory for the buffer */
  pwd->par_buff = malloc ( max_count * pwd->par_len );
    
  if (pwd->par_buff == NULL)
    {
    pwd->par_maxcnt = 1;
   
    printf ("  Warning: Couldn't allocate memory for password buffer.\n");
    /* par_buff == NULL signals main that no block gathering should be done,
       if the user wants to continue anyway */
    return -1;
    }

  /* all okay */
  return 0;
  }

