/*!
 * @file
 * @ingroup workerframe
 * @brief Config file reading/management for Dicop workers.
 * 
 * @copydoc copyrighttext
*/

#include <pwdgen.h>
#include <pwd_util.h>

/* ************************************************************************* */
/* INTERNAL: read a config/description file with key=value format
   return codes: 0 	- okay
		 != 0	- error */

/* Thou Shallst Not Add Arbitrary Limits To Thy Programme - but I did ... */

/* maximum length of a line */
#define BUFSIZE 64 * 1024

struct ssKey* pwdcfg_read ( const char* cfgfile )
  {
  int linenr;
  unsigned int count, i, n;
  unsigned int extracted = 0;
  FILE *file;
  struct ssKey* keys, *current, *last;
  char line[BUFSIZE+16];		/* input lines up to BUFSIZE characters */
  char name[BUFSIZE+16];		/* key */
  char value[BUFSIZE+16];		/* value */


  printf (" Opening config file '%s'.\n", cfgfile);
  if (NULL == (file = pwdgen_find_file (cfgfile, "r")))
    {
    return NULL;
    }

  /* allocate memory for the head node */
  keys = malloc (sizeof(struct ssKey));
  if (NULL == keys)
    {
    printf (" Error: Cannot allocate memory for head node.\n");
    return NULL;
    }
  keys->next = NULL;
  keys->prev = NULL;
  keys->key[0] = 0x00;		/* empty string so that it doesn't match */
  keys->len = 0;
  last = keys;

  /* read in line by line 
     format is as follows:
     key=value
     # comments

     # or empty lines are skipped
     secondkey=value
  */

  linenr = -1;
  while (!feof(file))
    {
    linenr ++; count = 0; line[0] = 0; 
    while (count < BUFSIZE)
      {
      line[count] = fgetc(file);
      if (feof(file))
        {
        line[count] = 0;
        break;
        }
      /* handles only 0xd0xa or 0xa files, not 0xd (like MAC) */
      if (line[count] == 0x0d || line[count] == 0x0a)
        {
        line[count] = 0; break;	/* kill line end/ line feed */
        }
      line[++count] = 0;
      }
    if (count >= BUFSIZE)
      {
      printf ("Line %i to long (more than %i chars).\n",linenr,BUFSIZE);
      fclose(file);
      return NULL;
      }
    if (count != 0)
      {
#ifdef DEBUG
      printf ("Line: '%s'\n",line);
#endif
      }

    /* skip comments and empty lines */
    if ((line[0] == '#') || (count == 0)) 
      {
#ifdef DEBUG
      printf ("Skipping empty/comment line %i\n",linenr);
#endif
      continue;
      }

    /* count != 0 now */

    /* match the line to key=value, extracting the key and the value */

    /* start at first character in line */
    i = 0; n = 0;
    name[0] = 0;				/* zero terminate (default empty) */
    while (i <= count)			/* as long as not end of line */
      {
      /* skip spaces in name (should only skip them at start...) */
      if (line[i] == ' ')
        {
        i++; continue;
        }
      if (line[i] == 0 || line[i] == '=' ||
          line[i] == 0x0a || line[i] == 0x0d)
        {
        break;
        }
      name[n] = line[i]; i++; n++;
      }
    name[n] = 0;	/* zero terminate name */
 
   
    if (i >= count && n == 0)
      {
#ifdef DEBUG
      printf (" i %i count %i n %i ",i, count, n);
      printf ("Skipping empty line %i\n",linenr);
#endif
      continue;
      }

    /* something like "name\n" or "=value" */
    if (n == 0 || line[i] != '=')
      {
      printf ("\n  Error: Malformed line at line %i\n",linenr);
      fclose(file);
      return NULL;
      }

    extracted++;

#ifdef DEBUG
    printf ("   Extracted '%s' ",name);
#endif
 
    /* now extract the value */

    i++;      /* skip the = sign */
    if (line[i] == '"')
      {
      i++;    /* skip quota */
      }
    /* now extract the part after the '=' */
    value[0] = 0;     /* zero terminate */
    n = 0;
    while (i <= count)
      {
      if (line[i] == 0 || line[i] == '"' ||
         line[i] == 0x0a || line[i] == 0x0d)
        {
        break;
        }
      value[n] = line[i]; i++; n++;
      }
    value[n] = 0;
#ifdef DEBUG
    printf ("= '%s' (len %i)\n", value, n);
#endif

    /* check that the list so far does not already include this key */
    current = pwdcfg_find_key (keys, name, CFG_NOFAIL);
    if (NULL != current)
      {
      printf (" Error: Key '%s' re-defined at line %i.\n", name, linenr);
      fclose(file);
      return NULL;
      }

    /* allocate memory for a new node */
    current = malloc (sizeof(struct ssKey));
    if (NULL == current)
      {
      printf (" Error: Cannot allocate memory for list node.\n");
      fclose(file);
      return NULL;
      }
    last->next = current;
    current->prev = last;
    current->next = NULL;
    /* key names are at most 128 characters */
    strncpy ((char *)current->key, name, 128);
    current->len = strlen((char *)current->key) + 1;
    /* allocate memory for value and copy it */
    n++;
    current->value = malloc ( sizeof(char) * n);
    strncpy ((char *)current->value,value, n);
    current->valuelen = n;
    printf ("  Extracted '%s=%s'\n", current->key, current->value);
    last = current;
    } /* while not eof */ 

  printf (" Extracted %i lines. Closing file.", extracted);
  fclose(file);
  return keys;
  }

/* ************************************************************************ */
/* find a specific key in list of config nodes.
   If key does not exist, and fail == CFG_FAIL, exits() with error
   message, otherwise returns NULL (or the ptr to the key)
*/

struct ssKey* pwdcfg_find_key (const struct ssKey* keys, const char* key, const unsigned long fail )
  {
  const struct ssKey* current = keys;
  unsigned int len;

  len = strlen(key) + 1;

#ifdef DEBUG
  printf ("Finding key '%s' with len %i\n", key, len);
#endif 
  /* walk the list starting at keys and follow the next ptr until we
     find the key we search for or reach the end of the list */
  while (NULL != current)
    {
#ifdef DEBUG
    printf (" at key '%s' with len %i\n", current->key, current->len);
#endif 
    if ((len == current->len) && (0 == strncmp (current->key, key, len)))
      {
      /* found it */
      return current;
      }
    current = current->next;
    }
  if (fail == CFG_FAIL)
    {
    printf ("Key '%s' not found in config file - aborting.\n", key);
    exit(1);
    }
  return NULL;
  }

/* ************************************************************************ */
/* check that the given list of keys exists. List is ',' separated.
   if fail == CFG_FAIL, calls exit(1) upon error

   return codes: 0	- okay
		 X	- count of not found keys 	*/

unsigned long pwdcfg_keys_exist (const struct ssKey* cfg, const char* keynames, const unsigned long fail) 
  {
  struct ssKey* key;
  unsigned long i = 0;		/* index into string */
  unsigned long n;		/* index into name */
  char name[256];		/* current key name */
  unsigned long len;		/* length */
  unsigned long rc = 0;		/* return code */

  len = strlen(keynames);

  while (i < len)
    {
    n = 0;
    while (i < len && keynames[i] != ',' && n < 250)
      {
      name[n] = keynames[i]; i++; n++;
      }
    i++;			/* skip the ',' */
    name[n] = 0;
    if (n > 0)
      {
#ifdef DEBUG
	printf ("Checking key '%s'\n",name);
#endif
      /* will exit() if options is set */	
      key = pwdcfg_find_key (cfg, name, fail);
      if (NULL == key)
	{
	/* not found */
	printf ("Warning: Key '%s' not found in config file.",name);
	rc++;			/* error */
	}
      }
    }

  return rc;	/* count of keys not found */
  }

/* ************************************************************************ */
/* Check that _only_ the given list of keys exists. List is ',' separated.
   Any other key is considered invalid.
   if options == 1, calls exit(1) upon error

   return codes: 0	- okay
		 X	- count of invalid keys 	*/

unsigned long pwdcfg_valid_keys (
	const struct ssKey* cfg,
	const char* keynames, 
	const unsigned long options )
  {
  struct ssKey* key;
  unsigned int valid, i, invalid, n;
  unsigned long len;			/* length */
  unsigned long keys;			/* count of key names */
  char name[256];			/* current key name */
  char names[256][128];			/* key names */

  if (NULL == cfg)
    {
    return 0;
    }
  key = cfg->next;
  
  /* create an array from a list of comma seperated strings */
  len = strlen(keynames);
  keys = 0; i = 0;

  while (i < len)
    {
    n = 0;
    while (i < len && keynames[i] != ',' && n < 128)
      {
      name[n] = keynames[i]; i++; n++;
      }
    i++;			/* skip the ',' */
    name[n] = 0;		/* zero terminate */
    if (n > 0)
      {
      /* copy the name */
      strncpy (names[keys],name, 128); keys++;
      }
    } /* end while for all characters */

  invalid = 0;
  /* for each key in chain, test if it is valid */
  while (key != NULL)
    {
    valid = 0;
    for (i = 0; i < keys; i++)
      {
      if (0 == strncmp((char *)key->key, &names[i][0], 128))
        {
        valid = 1;
        }
      }
    if (0 == valid)
      {
      printf ("\nError: Invalid key '%s'\n", key->key);
      if (options != 0)
	{
	exit(1);
	}
      invalid++;
      }
    key = key->next;		/* next key */
    }
  return invalid;
  }

/* ************************************************************************ */
/** Find a key, and return its value as int. Returns 0 if it doesn't exist.
    You should use pwdcfg_find_key() or pwdcfg_keys_exist() to check
    beforehand that the key really exists.\n\n

    return code: int value
*/

int pwdcfg_as_int (const struct ssKey* cfg, const char* keyname, const unsigned long fail)
  {
  struct ssKey* key;

  key = pwdcfg_find_key (cfg, keyname, fail);
  if (NULL == key)
    {
    printf (" Warning: Key '%s' does not exist in config - defaulting to 0.\n", keyname);
    return 0;
    }
  /* XXX TODO: should check that value is really a number */
  return atoi (key->value);
  }

