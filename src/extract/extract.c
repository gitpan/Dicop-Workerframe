/**
 * @defgroup workerframeextract Extract test worker
 * @ingroup workerframe 
 *
 * @brief Dicop extract test worker - tests extraction of strings/passwords from files.
 *
 * This program does not check passwords at all, instead it writes them to a
 * file. It should be used as:
 * 
 * @verbatim ./extract chunk_description.txt @endverbatim
 *
 * @copydoc copyrighttext
*/

/**
 * @file
 * @ingroup workerframeextract
 * @brief Dicop \ref workerframeextract "extract test worker" - tests extraction of strings/passwords.
 *
 * Return codes in dofunction: 
 * -  PWD_SUCCESS - found password
 * -  PWD_FAIL    - not found
 * -  PWD_ERROR   - fatal error
 *
 * @copydoc copyrighttext
*/

#include <dicop.h>

/* make these globals so we can use them in dofunction, too */
FILE* output;
unsigned long as_hex;
struct ssKey* key;

/** Prints version and copyright. */
void printinfo(void)
  {
  printf ("DiCoP extract worker v0.03  (c) Copyright by BSI 1998-2006\n\n");
  }

int initfunction(const struct ssPWD *pwd)
  {

  /* get access to the config */
  key = pwdcfg_find_key ( pwd->config, "extract_to", CFG_FAIL);

  printf ("\n Opening output file '%s'.\n", key->value);
  pwdgen_file_open(key->value, "w", output);

  /* 0 or != 0 */
  as_hex = pwdcfg_as_int ( pwd->config, "extract_as_hex", CFG_NOFAIL);
  return PWD_INIT_OK;		/* init was okay */
  }

void stopfunction( void )
  {
  printf ("Closing output file '%s'.\n", key->value);
  /* close the output file */
  fclose (output);
  }

/* called once after all pwds done, to give works doing pwds in batches a
   chance to complete left-over pwds */
int endfunction( const struct ssPWD *pwd )
  {
  return PWD_FAIL;
  }

int dofunction( const struct ssPWD *pwd )
  {
  unsigned long i, len;
  char b[PWD_LEN*2];

  /* if requested, convert the password to hex before writing it out */
  if (0 != as_hex)
    {
    /* make copy */
    len = pwd->length;
    for (i = 0; i < len; i++)
      {
      b[i] = pwd->pwd[i];
      }
    a2h(b, len); 
    len *= 2;
    /* write the password to the file */
    fwrite (b, sizeof(char), len, output);
    }
  else
    {
    /* just write the password to the file */
    fwrite (pwd->pwd, sizeof(char), pwd->length, output);
    }

  /* write a line end */
  fputc ( '\n', output);
#ifdef DEBUG
  fflush(output);
  printf ("at '%s'\n", pwd->pwd);
#endif

  if (0 != ferror (output))
    {
    printf ("Couldn't write to '%s', disk full?\n", key->value);
    return PWD_ERROR;					/* error on writing */
    }
  return PWD_FAIL;					/* continue */
  }

