/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

 This program does not check passwords at all, instead it writes them to a
 file. It should be used as:

 ./extract chunk_description.txt

 return codes in dofunction: 	PWD_SUCCESS - found password
				PWD_FAIL    - not found
				PWD_ERROR   - fatal error
              
*/

#include <stdio.h>
#include <stdlib.h>
#include "../include/pwdgen.h"
#include "../include/dicop.h"

//#define DEBUG 1

/* make these globals so we can use them in dofunction, too */
struct ssPWD *pwd;
FILE* output;
unsigned long as_hex;
struct ssKey* key;

void printinfo(void)
  {
  /* print here your version and copyright */
  printf ("DiCoP extract worker v0.03  (c) Copyright by BSI 1998-2006\n\n");
  }

int initfunction(struct ssPWD *password, char *targetkey)
  {
  pwd = password;		/* store struct with password */

  /* get access to the config */
  key = pwdcfg_find_key ( pwd->config, "extract_to", CFG_FAIL);

  printf ("\n Opening output file '%s'.\n", key->value);
  output = fopen (key->value, "w");
  if (NULL == output)
    {
    printf ("Cannot open '%s' for writing. Check permissions.\n", key->value);
    return 1;
    }

  /* 0 or != 0 */
  as_hex = pwdcfg_as_int ( pwd->config, "extract_as_hex", CFG_NOFAIL);
  return 0;			/* init was okay */
  }

void stopfunction( void )
  {
  printf ("Closing output file '%s'.\n", key->value);
  /* close the output file */
  fclose (output);
  }

/* called once after all pwds done, to give works doing pwds in batches a
   chance to complete left-over pwds */
int endfunction(void)
  {
  return PWD_FAIL;
  }

int dofunction( void )
  {
  unsigned long i, len;
  unsigned char b[PWD_LEN*2];

  /* make copy */
  /* if requested, convet the password to hex before writing it out */
  if (0 != as_hex)
    {
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


