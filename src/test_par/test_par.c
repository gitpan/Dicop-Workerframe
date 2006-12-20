/**
 * @defgroup workerframetestpar Test Par
 * @ingroup workerframe 
 *
 * Dicop test worker for parallel interface.
 *
 * This test worker setsup the password generator to generate passwords
 * in batches, and then tests them all in dofunction() in one go.
 *
 * The advantage of doing this is that the generated batch of passwords
 * has all passwords with the same length (and the same optional padding),
 * so the test can be done more quickly in some cases. The passwords could
 * even handed to some external hardware or machine for testing.
 *
 * @copydoc copyrighttext
*/

/**
 * @file
 * @ingroup workerframetestpar
 * @brief A \ref workerframetestpar "test worker" for the parallel interface.
 * 
 * Return codes in dofunction: 
 * -  PWD_SUCCESS - found password
 * -  PWD_FAIL    - not found
 * -  PWD_ERROR   - fatal error
 *
 * @todo The left-over passwords should be handled in endfunction().
 *
 * @copydoc copyrighttext
*/

#include <dicop.h>

/* make these globals so we can use them in dofunction, too */
char target[PWD_LEN];
unsigned long targetlen;

void printinfo(void)
  {
  /* print here your version and copyright */
  printf ("DiCoP test parallel worker v0.02  (c) Copyright by BSI 1998-2006\n\n");
  PRINT_INFO;
  }

int initfunction(const struct ssPWD *pwd)
  {
  /* You can get access to the config file via:
  struct ssConfig* config = pwd->config;
  pwdcfg_find_key( config, "keyname", CFG_NOFAIL);
  etc. */
  
  /* As an example, we just copy the target key. It is also possible
     to access it via pwd->target and pwd->targetlen */
  strcpy(target,pwd->target);	/* store target password in ascii */
  h2a(target);			/* or whatever suits you */

  targetlen = pwd->targetlen >> 1;

  /* padd with 0x20 and the zero teminate */
  target[targetlen++] = 0x20;
  target[targetlen+1] = 0x0;

#ifdef DEBUG
  /* print out a hexdump of our target data */
  hexdump ("\n  Target:", target, targetlen);
#endif

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
	0x20,			/* padded with space */
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
int endfunction(const struct ssPWD *pwd)
  {
  /* XXX TODO we should test leftover passwords here */
  return PWD_FAIL;
  }

int dofunction( const struct ssPWD *pwd )
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
  char *p = pwd->par_buff;
  unsigned long len = pwd->par_len;

#ifdef DEBUG
  printf (" DEBUG: cnt: %li, len: %li, buffer at: %p\n", cnt, len, p);
#endif

  printf ("\n");
  /* do something with each password */
  while (cnt-- > 0)
    {
    /* for testsuite, output what we are doing */

    char b[1024];
    unsigned int i;

    if (pwd->timeout == 0)                                /* for testsuite */
      {
      /* make copy of org password (minus the padding) for printing */
      for (i = 0; i < len - 1; i++)
        {
        b[i] = p[i];
        }
      b[i] = 0;                                             /* zero terminate */
      a2h(b,len-1);

      printf ("At '%s' len %li\n", b, len);
      }

    /* make copy of org password for comparisation */
    for (i = 0; i < len; i++)
      {
      b[i] = p[i];
      }
    b[i] = 0;                                             /* zero terminate */
    a2h(b,len);

    /* update the crc with data that depends on the password, since we do not
       use the password to decrypt any actual data or something along these
       lines, we just add up the passwords. In reality, this should not be
       done, but rather some sort of password-depended cleartext must be used */
    pwdgen_update_crc(pwd, p, len);

    /* now check the current pwd against the target*/
    if (targetlen == len && memcmp(p, target, len) == 0) 
      {
#ifdef DEBUG
      printf ("  DEBUG: Found target.\n" );
#endif

      /* Mark the current tested password as one that matched.
         Technically, we should include the padding here. But since the testsuite
         checks for the password without the padding, we just set the first
         len (= pwd->par_len - 1) bytes */

      pwdgen_found(pwd, p, len - 1);

      printf ("\n");

      return PWD_SUCCESS;
      }

    /* advance to next password */
    p += len;
    }

  return PWD_FAIL;					/* not found yet */
  }
