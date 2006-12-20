/*!
 * @file
 * @ingroup workerframe
 * @brief Parallel execution - gather passwords in blocks. then check them all together.
 * 
 * @copydoc copyrighttext
*/

#include <dicop.h>
#include <pwd_par.h>

#define MAX_COUNT 1024 * 64

/* ************************************************************************* 

   internal helper functions */

/** When the "once" flag is set, we process only one password. main() then
    calls this function to do this and we simple call dofunction() once
    and set the number of further available passwords to zero. */

int pwdgen_par_process(struct ssPWD* pwd)
  {
  int found;

  found = dofunction( pwd );
  pwd->par_cnt = 0;

  return found;
  }

/** Return the lenght per password (derived from padding and the current
    password length. */
unsigned int pwdgen_par_len (const struct ssPWD* pwd )
  {

  if (pwd->par_padd < 0)
    {
    /* -X means padd to X bytes */
    
#ifdef DEBUG
    printf ("  DEBUG: Current pwd len in pwd buffer is fixed at %i.\n", abs(pwd->par_padd));
#endif

    /* what happens if pwd is longer than X? */
    return abs(pwd->par_padd);
    }

#ifdef DEBUG
    printf ("  DEBUG: Current pwd len in pwd buffer is %li.\n", pwd->length + pwd->par_padd);
#endif

  /* padd > 0 means "pwd" + "padding" */
  return pwd->length + pwd->par_padd;
  }

/* Release memory */

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
     + call dofunction for each password
     + empty buffer
     + if pwd len changed: allocate new buffer
   * push the current password into the buffer.
*/

int pwdgen_par_push (struct ssPWD* pwd)
  {
  int found = PWD_FAIL;			/* result from dofunction */
  unsigned int realloc = 0;
  unsigned int newlen = pwdgen_par_len(pwd);
  unsigned int pwdlen = pwd->length;
  char *pbuff;
  char pchar;
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
    found = dofunction( pwd );

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
        return dofunction( pwd );
        }
      }
    }

#ifdef DEBUG
    printf ("  DEBUG: Push one pwd into pwd buffer.\n");
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

  Public function:

*/

/** Setup the gathering of passwords in blocks/groups:\n

  return code:\n
	== PWD_INIT_OK - ok\n
	!= PWD_INIT_OK - some fatal error occured
*/

int pwdgen_par_init ( 
  const unsigned int max_count,
  const signed int padd,
  const char paddchar,
  struct ssPWD *pwd )
  {

  pwd->par_maxcnt = max_count;
  if (max_count > MAX_COUNT)
    {
    printf ("  Warning: Collecting only %i pwds instead of the requested %i.\n",
     MAX_COUNT, max_count);
    pwd->par_maxcnt = MAX_COUNT;
    } 

  /* copy the data into the ssPWD struct */
  pwd->par_pchar = paddchar;
  pwd->par_padd = padd;
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
    return PWD_ERROR;
    }

  /* all okay */
  return PWD_INIT_OK;
  }

