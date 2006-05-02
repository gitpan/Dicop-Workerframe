/*

  main.c - main program for cluster worker 

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

 exit code: 3 - not enough or too much arguments or other failures
            2 - timeout, not all passwords checked
            1 - found a solution (and checked all passwords up to last, or in
		case of timeout, up to timeout password)
            0 - no solution found at all
 
*/

#include "../include/pwdread.h"
#include "../include/pwddict.h"
#include "../include/pwd_par.h"
#include "../include/dicop.h"

/* the external functions are defined in the source code of each worker: */
extern void	printinfo    (void);
extern int	initfunction (struct ssPWD* pwd,char *targetkey);
extern int	dofunction   (void);
extern void	stopfunction (void);
extern int	endfunction  (void);

extern int	pwdgen_extract     (struct ssPWD* pwd);
extern int	pwdgen_par_push    (struct ssPWD* pwd);
extern void	pwdgen_par_done    (struct ssPWD* pwd);
extern int	pwdgen_par_process (struct ssPWD* pwd);

/* structure describing the internal state of the password generator: */
struct ssPWD* pwd;

//#define DEBUG 1

#define CFG_KEYS 4

/* ************************************************************************ */
/* take command line arguments and check them, init password generator */

struct ssPWD* init (int argc, char** argv)
  {
  char firstkey[PWD_LEN * 2];
  char lastkey[PWD_LEN * 2]; 
  char targetkey[PWD_LEN * 2];
  struct ssKey* keys, *current;
  int i;
  char pwdset [256];
  char chunkkeys[CFG_KEYS][16] = { "start", "end", "target", "charset_id" };
  char* keysptr[CFG_KEYS] = { firstkey, lastkey, targetkey, pwdset };
  int timeout = 0;

  keys = NULL;
  
  printf ("Checking commandline arguments...");
  
  if ((argc == 2) || (argc == 3))
    {
    printf (" count of %i looks good.\n", argc);
    /* called like: name chunk_description_file [timeout] */
    if (argc == 3)
      {
      timeout = atoi(argv[2]);
      }
    /* try to read the description file */
    keys = pwdcfg_read (argv[1]);
    if (NULL == keys)
      {
      printf (" Error: Couldn't read chunk description file '%s' - aborting.\n",argv[1]);
      return NULL;
      }
    firstkey[0] = 0;
    lastkey[0] = 0;
    targetkey[0] = 0;
    pwdset[0] = 0;
    /* set start/end/target from read-in info if applicable */
    for (i = 0; i < CFG_KEYS; i++)
      {
      current = pwdcfg_find_key( keys, chunkkeys[i], CFG_NOFAIL ); 
      if (NULL != current)
        {
        /* found the key, so copy it's value */
	strncpy (keysptr[i], current->value, current->valuelen);
        }
      }
    }
 else
    {
    if ((argc < 5) || (argc > 6))
      {
      printf ("\n Error: Expected 1, 2, 4 or 5 arguments, but got %i.\n", argc-1);
      printf ("\n Usage: %s chunk_description_file [timeout]\t\tor", argv[0]);
      printf ("\n        %s firstkey lastkey targetkey pwdset [timeout]\n",
        argv[0]);
      return NULL;
      }
    printf (" count of %i looks good.\n", argc);
    strncpy (firstkey,argv[1],255);
    strncpy (lastkey,argv[2],255);
    strncpy (targetkey,argv[3],255);
    strncpy (pwdset,argv[4],240);
    if (argc == 6)
      {
      timeout = atoi(argv[5]);
      }
    }
  printf ("\ndone.\nInitializing password generator...");
  return pwdgen_init( firstkey, lastkey, targetkey, pwdset, timeout, keys);
  }

/* ************************************************************************ */

/* generate and gather passwords until buffer full, then hand them to
   dofunction() */

int gather_pwds (struct ssPWD *pwd)
  {
  int found = 0;
  time_t t;
  
  if (pwd->type == SET_DICTIONARY)
    {
    t = time(NULL);
    printf ("%s Starting main dictionary loop\n", ctime(&t)); 
    /* check until timeout, no more passwords or matching password found */
    do
      {
      // generate next pwd       and push it into block (maybe calling dofunction())
      found = pwddict_next( pwd ) +  pwdgen_par_push (pwd);

      pwd->pwds_done++;
      } while (found == 0);
    }
 
  /* XXX TODO: pwdgen_extract() needs to do the right thing under parallel */ 
  if (pwd->type == SET_EXTRACT)
    {
    t = time(NULL);
    printf ("%s Starting main extraction loop.\n", ctime(&t)); 
    fflush(NULL);
    /* extract pwds, weed out doubles, and call dofunction() on each pwd left */
    found = pwdgen_extract( pwd );
    }

  /**************************************************************************/
  if ((pwd->type == SET_GROUPED) || (pwd->type == SET_SIMPLE))
    {
    t = time(NULL);
    printf ("%s Checking first key...\n", ctime(&t));
 
    found = pwdgen_par_push(pwd);

    printf ("  (once: %i found: %i)", sPWD.once, found);

    if (sPWD.once != 0)
      {
      found = pwdgen_par_process(pwd) + sPWD.once;
      }
    printf ("  (once: %i found: %i)", sPWD.once, found);

    t = time(NULL);
    printf (" done\n%s Starting main generator loop\n", ctime(&t)); 
    /* check until timeout, no more passwords or matching password found */
    while (found == 0)
      {
      found = pwdgen_next( pwd ) + pwdgen_par_push(pwd);
      }
    }

  /* clear the left over gathered pwds */
  if (pwd->par_cnt > 0)
    {
    found += dofunction();
    }

  /* release the memory */
  pwdgen_par_done(pwd);

  return found;
  }

/* ************************************************************************ */

/* generate passwords and hand them to dofunction() until done */

int cycle_pwds (struct ssPWD *pwd)
  {
  int found = 0;
  time_t t;
  unsigned int last_pwd_len, equal, i;
  unsigned char last_pwd[PWD_LEN];

  if (pwd->type == SET_DICTIONARY)
    {
    t = time(NULL);
    printf ("%s Starting main dictionary loop\n", ctime(&t)); 
    // reset last pwd
    last_pwd_len = 0; last_pwd[0] = 0;
    /* check until timeout, no more passwords or matching password found */
    do
      {
      // generate next pwd
      found = pwddict_next( pwd );
      // if current generated does not differ from last generated, skip it
      if ((last_pwd_len == pwd->length))
        {
#ifdef DEBUG
        last_pwd[last_pwd_len] = 0;
	printf ("\n Last pwd: '%s', cur pwd '%s'\n", last_pwd, pwd->pwd);
#endif
        equal = 1;
        for (i = 0; i < pwd->length; i++)
	  {
          if (pwd->pwd[i] != last_pwd[i])
	    {
	    equal = 0;
	    // remember last pwd (len already equal)
            last_pwd[i] = pwd->pwd[i];
	    }
	  }
        if (equal == 0)
	  {
          found += dofunction();
	  pwd->pwds_done++;
	  }
        }
      else
        {
        found += dofunction();
	pwd->pwds_done++;
#ifdef DEBUG
        last_pwd[last_pwd_len] = 0;
	printf ("\nLast pwd: '%s', cur pwd '%s'\n",last_pwd, pwd->pwd);
#endif
	// remember last pwd 
        last_pwd_len = pwd->length;
        for (i = 0; i < pwd->length; i++)
          {
	  last_pwd[i] = pwd->pwd[i];
	  }
        }
        
      } while (found == 0);
    }
  
  if (pwd->type == SET_EXTRACT)
    {
    t = time(NULL);
    printf ("%s Starting main extraction loop.\n", ctime(&t)); 
    fflush(NULL);
    /* extract pwds, weed out doubles, and call dofunction() on each pwd left */
    found = pwdgen_extract( pwd );
    }

  /**************************************************************************/
  if ((pwd->type == SET_GROUPED) || (pwd->type == SET_SIMPLE))
    {
    t = time(NULL);
    printf ("%s Checking first key...\n", ctime(&t));
 
    found = dofunction() +	/* check first password ever */
          sPWD.once;		/* only one password to check? */

    t = time(NULL);
    printf (" done\n%s Starting main generator loop\n", ctime(&t)); 
    /* check until timeout, no more passwords or matching password found */
    while (found == 0)
      {
      found = pwdgen_next( pwd ) + dofunction();
      }
    }
  return found;

  }

/* ************************************************************************ */

int main (int argc, char** argv)
  {
  int found = 0;
  time_t t;

  printf ("DiCoP Workerframe - Main %s (c) by BSI 1998-2005. See LICENSE file.\n\n", DICOP_VERSION);

  printinfo();
  
  pwd = init(argc,argv);

  if (pwd == NULL)
    {
    printf ("Couldn't initialize password generator - aborting.\n"); return 3;
    }

  printf ("done.\nInitializing user tables..."); 
  if (initfunction(pwd, pwd->target) != 0)
    {
    printf ("Error: Couldn't initialize tables.\n");
    return 3;
    }

  printf (" done.\n Today we will check pwds in blocks of %i.\n", pwd->par_maxcnt);

  /* flush all output buffers */
  fflush(NULL);
 
  if (pwd->par_buff != NULL)
    {
    /* gather pwds in blocks, then call dofunction() on them */
    found = gather_pwds( pwd );
    }
  else
    {
    /* call dofunction() for each pwd alone */
    found = cycle_pwds (pwd);
    } 

  /* if dofunction() was gathering pwds and doing them in batches, give worker
     a chance to complete last batch; only called if we didn't find anything
     so far (there can be only one result per chunk, so we ignore further
     possible results for now */
  if ((found < PWD_ERROR) && (((found >> 2) & 1) == 0))
    {
    found += endfunction();
    }

  /* some error in dofunction() or related functions? */
  if (found >= PWD_ERROR)
    {
    printf (" Error in dofunction() - Aborting.\n");
    return 3;
    }

  /**************************************************************************/
  /* found = 1 => 0b00 last password checked, nothing found
             2 => 0b01 found result
	     3 => 0b01 last password, and dofunction() did find something
	     4 => 0b10 timeout, nothing found
             6 => 0b11 timeout, and dofunction() did find something
   */
  found = found >> 1;
  if (found > 2) { found = 1; } 	/* timeout + found => found */
  t = time(NULL);
  printf ("%s done.\n\n", ctime(&t)); fflush(NULL);
  pwdgen_stop( pwd );
  stopfunction();
  printf ("Last tested password in hex was '%s'\n",pwd->pwd);
  printf ("Stopcode is '%i'\n", found );
  printf ("CRC of chunk is '%x'\n",pwd->crc);
  if (pwd->took < 1) { pwd->took = 1; }
  printf ("Did %.0f passwords in %.0f seconds => %.0f tries/s\n",
    pwd->pwds, pwd->took, pwd->pwds / pwd->took);
  printf ("All done\n\n");

  return found; 	// 0 = none, 1 found password
  }


