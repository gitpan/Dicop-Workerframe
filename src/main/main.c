/**
 * @defgroup copyrighttext Copyright
 * @ingroup workerframe
 *
 * @author Copyright &copy; 1998-2007,
 * <a title="Go to our homepage" href="http://www.bund.bsi.de">
 * Bundesamt f&uuml;r Sicherheit in der Informationstechnik</a> (BSI)\n\n
 * This file is part of Dicop-Workerframe. For licencing information see the
 * file LICENCE in the distribution.
 *
*/

/**
 * @defgroup workerframe Dicop-Workerframe
 *
 * @english
 * A framework for easy development of Dicop workers in C.
 * @endenglish
 *
 * @deutsch
 * Eine Bibliothek, um einfach und schnell Dicop-Worker in C zu entwickeln.
 * @enddeutsch
 *
 * @copydoc copyrighttext
*/

/**
 * @file
 * @ingroup workerframe
 * @brief main() routine for Dicop workers. 
 *
 * @copydoc copyrighttext
 *
 * @section arch main():
 *
 * @verbatim
   exit code:
	      3 - not enough or too much arguments or other failures,
 	          or "--identfiy filename" failed
              2 - timeout, not all passwords checked
              1 - found a solution (and checked all passwords up to last, or in
                  case of timeout, up to timeout password)
              0 - no solution found at all, or "--identify filename" passed @endverbatim
		
*/

#include <dicop.h>
#include <pwdread.h>
#include <pwddict.h>

/* Number of predefined config file keys for default arguments. */ 
#define CFG_KEYS 5

/* ************************************************************************ */
/** Take command line arguments and check them, then init the password
    generator. */

struct ssPWD* init (int argc, char** argv)
  {
  FILE* file;
  char firstkey[PWD_LEN * 2];
  char lastkey[PWD_LEN * 2]; 
  char targetkey[PWD_LEN * 2];
  struct ssKey* keys, *current;
  int i;
  char pwdset [PWD_LEN];
  char charset_type [PWD_LEN];
  char chunkkeys[CFG_KEYS][16] = { "start", "end", "target", "charset_id", "charset_type" };
  char* keysptr[CFG_KEYS];
  int timeout = 0;

  keysptr[0] = firstkey;
  keysptr[1] = lastkey;
  keysptr[2] = targetkey;
  keysptr[3] = pwdset;
  keysptr[4] = charset_type;
  keys = NULL;
  
  printf ("Checking commandline arguments...");
  
  if ((argc == 2) || (argc == 3))
    {
    printf (" count of %i looks good.\n", argc);
    /* called like: ./worker --identify filename */
    if ((argc == 3) && (strncmp("--identify",argv[1],10) == 0))
      {
      printf ("\nIdentifying file '%s':\n\n", argv[2]);
      file = fopen (argv[2],"r");
      if (NULL == file)
	{
	printf ("\n Error: Cannot open file '%s': %s\n", argv[2], strerror(errno));
        exit(3);
        }
      pwdgen_file_close(file);
      /* setup some fake keys in the password generator struct to make
         initfunction() happy */
      strcpy(sPWD.start,"00000000");
      strcpy(sPWD.end,"00000000");
      sPWD.startlen = 8;
      sPWD.endlen = 8;
      strncpy(sPWD.target,argv[2],255);
      sPWD.target[255] = 0;
      sPWD.targetlen = strlen(sPWD.target);
      if ( 0 != initfunction(&sPWD) )
        {
        printf ("\nFile '%s' was not recognized by this worker.\n\n", argv[2]);
        exit(3);
        }
      printf ("\nFile '%s' looks ok to me.\n\n",argv[2]);
      exit(0);
      }
    /* called like: ./worker chunk_description_file [timeout] */
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
    charset_type[0] = 0;
    /* set start/end/target from read-in info if applicable */
    for (i = 0; i < CFG_KEYS; i++)
      {
      current = pwdcfg_find_key( keys, chunkkeys[i], CFG_NOFAIL ); 
      if (NULL != current)
        {
        /* found the key, so copy its value */
	strncpy (keysptr[i], current->value, current->valuelen);
        }
      }
    /* if a charset_type was given, but no charset_id, use a virtual charset_id */
    if (0 == pwdset[0])
      {
      /* find out which virtual ID we need */
      if (0 == memcmp("simple",charset_type, 5))
        {
        strcpy(pwdset, "-1");
        }
      else if (0 == memcmp("grouped",charset_type, 7))
        {
        strcpy(pwdset, "-2");
        }
      else if (0 == memcmp("dictionary",charset_type, 10))
        {
        strcpy(pwdset, "-3");
        }
      else if (0 == memcmp("extract",charset_type, 7))
        {
        strcpy(pwdset, "-4");
        }
      }
    }
 else
    {
    if ((argc < 5) || (argc > 6))
      {
      printf ("\n Error: Expected 1, 2, 4 or 5 arguments, but got %i.\n", argc-1);
      printf ("\n Usage: %s chunk_description_file [timeout]", argv[0]);
      printf ("\n        %s firstkey lastkey targetkey pwdset [timeout]",argv[0]);
      printf ("\n        %s --identify filename\n",argv[0]);
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
    /* we do not have a config file */
    keys = NULL;
    }
  printf ("done.\nInitializing password generator...");
  return pwdgen_init(firstkey, lastkey, targetkey, pwdset, timeout, keys);
  }

/* ************************************************************************ */

/** Generate and gather passwords until buffer full, then hand them to
   dofunction(). */

int gather_pwds (struct ssPWD *pwd)
  {
  int found = 0;
  
  if (pwd->type == SET_DICTIONARY)
    {
    pwdgen_print_time ("%s Starting main dictionary loop\n");
    /* check until timeout, no more passwords or matching password found */
    do
      {
      /* generate next pwd and push it into block (maybe calling dofunction()) */
      found = pwddict_next( pwd ) +  pwdgen_par_push (pwd);

      pwd->pwds_done++;
      } while (found == 0);
    }
 
  /* XXX TODO: pwdgen_extract() needs to do the right thing under parallel */ 
  if (pwd->type == SET_EXTRACT)
    {
    pwdgen_print_time ("%s Starting main extraction loop.\n");
    fflush(NULL);
    /* extract pwds, weed out doubles, and call dofunction() on each pwd left */
    found = pwdgen_extract( pwd );
    }

  /**************************************************************************/
  if ((pwd->type == SET_GROUPED) || (pwd->type == SET_SIMPLE))
    {
    pwdgen_print_time ("%s Checking first key...\n");
 
    found = pwdgen_par_push(pwd);

    if (sPWD.once != 0)
      {
      found = pwdgen_par_process(pwd) + sPWD.once;
      }

    if (found == 0)
      {
      pwdgen_print_time (" done\n%s Starting main generator loop\n"); 
      }
    /* check until timeout, no more passwords or matching password found */
    while (found == 0)
      {
      found = pwdgen_next( pwd ) + pwdgen_par_push(pwd);
      }
    }

  /* clear the left over gathered pwds */
  if (pwd->par_cnt > 0)
    {
    found += dofunction(pwd);
    }

  /* release the memory */
  pwdgen_par_done(pwd);

  return found;
  }

/* ************************************************************************ */

/** Generate passwords and then hand them to dofunction() until done. */

int cycle_pwds (struct ssPWD *pwd)
  {
  int found = 0;
  unsigned int last_pwd_len, equal, i;
  char last_pwd[PWD_LEN];

  if (pwd->type == SET_DICTIONARY)
    {
    pwdgen_print_time ("%s Starting main dictionary loop\n"); 
    /* reset last pwd */
    last_pwd_len = 0; last_pwd[0] = 0;
    /* check until timeout, no more passwords or matching password found */
    do
      {
      /* generate next pwd */
      found = pwddict_next( pwd );
      PWDGEN_ERROR_CHECK;
      /* if current generated does not differ from last generated, skip it */
      if ((last_pwd_len == pwd->length))
        {
#ifdef DEBUG
	printf ("\nLast len %i == cur len %i", last_pwd_len, pwd->length);
	printf ("\n Last pwd: '%s', cur pwd '%s'\n", last_pwd, pwd->pwd);
        last_pwd[last_pwd_len] = 0;
#endif
        equal = 1;
        for (i = 0; i < pwd->length; i++)
	  {
          if (pwd->pwd[i] != last_pwd[i])
	    {
	    equal = 0;
	    /* remember last pwd (len already equal) */
            last_pwd[i] = pwd->pwd[i];
	    }
	  }
        if (equal == 0)
	  {
          found += dofunction( pwd );
          PWDGEN_ERROR_CHECK;
	  pwd->pwds_done++;
	  }
        }
      else
        {
#ifdef DEBUG
	printf ("\nLast len %i != cur len %i", last_pwd_len, pwd->length);
#endif
        found += dofunction( pwd );
        PWDGEN_ERROR_CHECK;
	pwd->pwds_done++;
#ifdef DEBUG
        last_pwd[last_pwd_len] = 0;
	printf ("\nLast pwd: '%s', cur pwd '%s'\n",last_pwd, pwd->pwd);
#endif
	/* remember last pwd */
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
    pwdgen_print_time ("%s Starting main extraction loop.\n"); 
    /* extract pwds, weed out doubles, and call dofunction() on each pwd left */
    found = pwdgen_extract( pwd );
    PWDGEN_ERROR_CHECK;
    }

  /**************************************************************************/
  if ((pwd->type == SET_GROUPED) || (pwd->type == SET_SIMPLE))
    {
    pwdgen_print_time ("%s Checking first key...\n");

    found = dofunction( pwd ) +		/* check first password ever */
          pwd->once;			/* only one password to check? */

    PWDGEN_ERROR_CHECK;
    if (found == 0)
      {
      pwdgen_print_time (" done\n%s Starting main generator loop\n"); 
      }
    /* check until timeout, no more passwords or matching password found */
    while (found == 0)
      {
      found = pwdgen_next( pwd ) + dofunction( pwd );
      PWDGEN_ERROR_CHECK;
      }
    }
  return found;
  }

/* ************************************************************************ */

int main (int argc, char** argv)
  {
  /** Structure describing the internal state of the password generator. */
  struct ssPWD* pwd;
  int found = 0;

  printf ("DiCoP Workerframe - Copyright (c) by BSI 1998-2007.\n\n");
  printf (" Compiled: %s %s with %s rev %s (check-in: %s).\n\n",
         __DATE__, __TIME__, DICOP_VERSION, DICOP_REVISION, DICOP_MODIFIED);
  printf (" See the LICENSE file for licensing information.\n\n");

  printinfo();
  
  pwd = init(argc,argv);
  if (pwd == NULL)
    {
    printf ("\n Error: Couldn't initialize password generator - aborting.\n");
    return PWD_ERROR;
    }

  printf ("done.\nInitializing user tables...\n"); 
  if ( (0 != initfunction(pwd)) || (0 != pwd->error))
    {
    printf (" Error: initfunction() returned an error - aborting.\n");
    return PWD_ERROR;
    }

  printf ("done.\nFinalizing password generator... ");
  if (PWD_INIT_OK != pwdgen_finalize(pwd))
    {
    printf ("\n Error: Couldn't finalize generator - aborting.\n");
    return PWD_ERROR;
    }
  printf ("done.\n");

  /* produce the final list for verification */
  pwdgen_print_encodings(pwd);

  if (pwd->par_maxcnt == 0)
    {
    printf ("Parallel pwd generator unused, will check passwords one by one.\n");
    }
  else
    {
    printf ("Parallel pwd generator successfully initialized, batch size is %i.\n",
      pwd->par_maxcnt);
    }

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
    found += endfunction( pwd );
    }

  /* some error in dofunction() or related functions? */
  if (found >= PWD_ERROR || (pwd->error != 0))
    {
    printf (" Error in dofunction() - Aborting.\n");
    return PWD_ERROR;
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
  pwdgen_print_time ("\n%s done.\n\n");

  pwdgen_stop( pwd );
  stopfunction();

  printf ("Last tested password (in %s):",
	pwdgen_encoding(pwd->cur->encoding));
  hexdump_pwd ("\n", pwd->cur->content, pwd->cur->bytes);

  a2h(pwd->cur->content, pwd->cur->bytes);
  printf ("\nLast tested password in hex was '%s'\n",pwd->cur->content);

  printf ("Stopcode is '%i'\n", found );
  printf ("CRC of chunk is '%x'\n",pwd->crc);
  if (pwd->took < 1) { pwd->took = 1; }
  printf ("Did %.0Lf passwords in %.0Lf seconds => %.0Lf tries/s\n",
    pwd->pwds, pwd->took, pwd->pwds / pwd->took);
  printf ("All done.\n\n");

  /* 0 = none, 1 = found password, 2 = timeout, 3 = timeout but found, >3 = error */
  return found; 
  }
