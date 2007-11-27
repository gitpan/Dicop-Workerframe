/*!
 * @file
 * @ingroup workerframe
 * @brief Implements the various config file reading routines.
 * 
 * @copydoc copyrighttext
*/

#include <dicop.h>
#include <pwdread.h>
#include <pwd_util.h>

/* ************************************************************************* */
/* INTERNAL: read file with charset definitions and build character sets from
             them, inclusive grouped charsets from the simple ones
   return codes: ptr to charsets - okay
		 NULL 		 - error 	*/

/* defined in pwdgen.c */
extern unsigned int MAXSETS;

/* Thou Shallst Not Add Arbitrary Limits To Thy Programme - but I did ... */

/** Buffer size for reading character sets */
#define BUFSIZE 16384
/** Length of one field in a charset definition */
#define MAXFIELDSIZE 16

struct ssCharset *
pwdgen_read_charsets (void)
{
  int linenr, convert, mul, stage, s, idx;
  unsigned int pos, set;
  FILE *file;
  unsigned int k, j, i, count;
  unsigned char line[BUFSIZE + 16];	/* input lines up to BUFSIZE characters */
  unsigned char buffer[16];	/* temp. buffer for conversations */
  unsigned char temp[BUFSIZE];	/* temp. buffer for conversations */
  unsigned int temp_values[8];	/* temp. buffer for values */
  struct ssCharset *charset;	/* to store read in charsets */
  int alloc = 0;		/* already allocated memory? */

  printf ("\n Opening global charset description file...");

  if (NULL == (file = pwdgen_find_file ("charsets.def", "r")))
    {
      return NULL;
    }

  MAXSETS = 0;

  /* read in line by line, format is as follows:
     count=6
     # comments
     0:1:41:42...       simple charset, fields:
                        type id  chars
                        0    1.. 41 42 43 ...
     1:2:1=1:2=1...     grouped charset, fields: 
                        type id  pairs (position 1..length or -1..-length)
                        1    1.. position=charset
     2:2:0,32,1,2:...   dictionary, fields: 
                        type id  pairs (pos= 0|1) 
                        2    1.. pos,set,startlen,endlen: 
   */
  charset = NULL;		/* silence the might be used unitialized warning */
  linenr = -1;
  set = 0;			/* index of set we are reading in */
  while (!feof (file))
    {
      linenr++;
      count = 0;
      line[0] = 0;
      while (count < BUFSIZE && !feof (file))
	{
	  line[count] = (unsigned char)fgetc (file);
	  /* handles only 0xd0xa or 0xa files, not 0xd (like MAC) */
	  if (line[count] == 0x0d || line[count] == 0x0a)
	    {
	      line[count] = 0;
	      break;		/* kill line end/ line feed */
	    }
	  line[++count] = 0;
	}
      if (count >= BUFSIZE)
	{
	  printf ("Line %i to long (more than %i chars).\n", linenr, BUFSIZE);
	  fclose (file);
	  return NULL;
	}
      if (count != 0)
	{
#ifdef DEBUG
	  printf ("Line: '%s'\n", line);
#endif
	}

      /* skip comments and empty lines */
      if ((line[0] == '#') || (count == 0))
	{
	  continue;
	}

      /* XXX TODO could eliminate this check */
      if (count != 0)
	{
	  if (strncmp ((char *)line, "count=", 6) == 0)
	    {
	      /* read out the count to alloc memory */
	      /* 9999 diff. charsets should be enough for everyone... */
	      strncpy ((char *)buffer,(const char *) &line[6], 4);
	      buffer[4] = 0;
	      MAXSETS = atoi ((const char *)buffer);
	      if (MAXSETS == 0)
		{
		  printf ("At least one charset should exist!\n");
		  fclose (file);
		  return NULL;
		}
#ifdef DEBUG
	      printf ("count = %i\n", MAXSETS);
#endif
	    }

	  if ((line[0] == '0') || (line[0] == '1') || (line[0] == '2') ||
	      (line[0] == '3'))
	    {
	      /* start of charset definitions, so make sure we have memory */
	      if ((MAXSETS == 0) && (alloc != 0))
		{
		  printf ("Charset count must come before definitions!\n");
		  fclose (file);
		  return NULL;
		}
	      if ((MAXSETS == 0) && (alloc == 0))
		{
		  printf ("At least one charset should exist!\n");
		  fclose (file);
		  return NULL;
		}
	      if (alloc == 0)
		{
#ifdef DEBUG
		  printf ("malloc for %i charsets\n", MAXSETS);
#endif
		  charset = malloc (sizeof (struct ssCharset) * MAXSETS);
		  if (charset == NULL)
		    {
		      printf ("Cannot alloc memory!\n");
		      fclose (file);
		      return NULL;
		    }
		  for (j = 0; j < MAXSETS; j++)
		    {
		      charset[j].id = 0;	/* not yet read */
		    }
		  alloc = 1;	/* flag that we already allocated */
		}
#ifdef DEBUG
	      printf ("parsing now line %i\n", linenr);
#endif
	      /* now parse the charset definition line */
	      if (line[1] != ':')
		{
		  printf
		    ("Error: Charset type must be followed by ':' in line %i\n",
		     linenr);
		  fclose (file);
		  return NULL;
		}
	      charset[set].type = SET_SIMPLE;
	      if (line[0] == '1')
		{
		  charset[set].type = SET_GROUPED;
		}
	      if (line[0] == '2')
		{
		  charset[set].type = SET_DICTIONARY;
		}
	      if (line[0] == '3')
		{
		  charset[set].type = SET_EXTRACT;
		}
	      pos = 2;		/* skip /^(0|1|2|3): */
	      /* convert first the id of the set */
	      if (line[pos] == ':')
		{
		  printf ("Error: Charset ID must not be empty in line %i\n",
			  linenr);
		  fclose (file);
		  return NULL;
		}
	      i = 0;
	      while (pos < count && line[pos] != ':' && i <= 4)
		{
		  buffer[i++] = line[pos++];
		  buffer[i] = 0;
		}
	      if (i > 4)
		{
		  printf ("Error: Charset ID to big in line %i\n", linenr);
		  fclose (file);
		  return NULL;
		}
	      convert = atoi ((const char *)buffer);
	      if (convert <= 0)
		{
		  printf
		    ("Error: Charset ID must be > 0, but is %i, in line %i\n",
		     convert, linenr);
		  fclose (file);
		  return NULL;
		}
	      /* test that the charset number in convert is one of the existing ones */
	      i = 0;
	      while (i < MAXSETS)
		{
		  /* only for read in ones, since unread ones have still id==0
		     convert is >0 here, this is tested above */
		  if (charset[i].id == (unsigned int)convert)
		    {
		      printf
			("Error: Charset ID %i defined twice in line %i\n",
			 convert, linenr);
		      fclose (file);
		      return NULL;
		    }
		  i++;
		}
	      charset[set].id = convert;

#ifdef DEBUG
	      printf ("Reading charset %i ... ", charset[set].id);
#endif
	      /* now read either the chars or the pos=set fields */
	      charset[set].length = 0;	/* start anew (simple) */
	      charset[set].def = NULL;	/* clear def (grouped) */
	      charset[set].paircnt = 0;	/* also current pair (grouped) */
	      while (pos < count)
		{
		  pos++;	/* skip the ':' separator */
		  if (pos == count)
		    {
		      /* pos == count => trailing ':' (or garbage, but we don't look) */
		      break;
		    }
		  /* convert the characters */
		  i = 0;
		  while (pos < count && line[pos] != ':' && i <= 8)
		    {
		      buffer[i++] = line[pos++];
		    }
		  buffer[i] = 0;
		  if (i > MAXFIELDSIZE)
		    {
		      printf ("Error: Field to big in line %i\n", linenr);
		      fclose (file);
		      return NULL;
		    }
		  if (charset[set].type == SET_SIMPLE)
		    {
		    /* convert hex to dec */
		    idx = i - 1;
		    convert = 0;
		    mul = 1;
		    while (idx >= 0)
		      {
		      /* assumes some ASCIIism */
		      if (buffer[idx] >= '0' && buffer[idx] <= '9')
			{
			convert += mul * (buffer[idx] - '0');
			}
		      else
			{
			if (buffer[idx] > 'f')
			  {
			  buffer[idx] -= 'A';
			  }	/* to lower */
			if (buffer[idx] < 'a' || buffer[idx] > 'f')
			  {
			  printf ("Error: Illegal hex character %c in line %i\n",
			     buffer[idx], linenr);
			  fclose (file);
			  return NULL;
			  }
			convert += mul * (buffer[idx] - 'a' + 10);
			}
			mul *= 16;
			idx--;
		      }	/* for each nibble */
		      /* got one, so put it in and add one more to len */
		      charset[set].chars[charset[set].length++] = convert;
		    }
		  if (charset[set].type == SET_GROUPED)
		    {
		      /* split pos=id into pos and id at = */
		      j = 0;
		      convert = 0;
		      mul = 1;
		      while (j < i)
			{
			  if (buffer[j] == '=')
			    {
			      break;
			    }
			  if ((buffer[j] != '-')
			      && (buffer[j] < '0' || buffer[j] > '9'))
			    {
			      printf
				("Error: Illegal character '%c' in line %i\n",
				 buffer[j], linenr);
			      fclose (file);
			      return NULL;
			    }
			  temp[j] = buffer[j];
			  j++;
			}
		      temp[j] = 0;
		      if (buffer[j] != '=')
			{
			  printf
			    ("Error: Illegal character '%c' (expect: '=') in line %i\n",
			     buffer[j], linenr);
			}
		      convert = atoi ((const char *)temp);	/* position */
#ifdef DEBUG
		      printf ("convert = %i (from %s)\n", convert, temp);
#endif
		      if (convert != 0)	/* not default position? */
			{
			  charset[set].pos[charset[set].paircnt] = convert;
			}
		      j++;
		      k = 0;	/* skip = */
		      while (j < i)
			{
			  if ((buffer[j] < '0' || buffer[j] > '9'))
			    {
			      printf
				("Error: Illegal character %c (expect: 0..9) in line %i\n",
				 buffer[j], linenr);
			      fclose (file);
			      return NULL;
			    }
			  temp[k++] = buffer[j++];
			}
		      temp[k] = 0;
		      j = atoi ((const char *)temp);
#ifdef DEBUG
		      printf ("j = %i (from %s)\n", j, temp);
#endif
		      /* check if simple set for this pos is valid */
		      charset[set].set[charset[set].paircnt] = NULL;
		      i = 0;
		      while (i < MAXSETS)
			{
			  /* only for read in ones, since unread ones have still id==0 */
			  if (charset[i].id == j)
			    {
			      if (charset[i].type != SET_SIMPLE)
				{
				  printf
				    ("Error: Set %i referenced in set %i in line %i must be a simple set, not another grouped or dictionary one.\n",
				     i, set, linenr);
				  fclose (file);
				  return NULL;
				}
			      /* store pointer to set in set[] or def, depending on */
			      if (convert != 0)
				{
				  charset[set].set[charset[set].paircnt] =
				    &charset[i];
				}
			      else
				{
				  charset[set].def = &charset[i];
				}
			      break;
			    }
			  i++;
			}
		      if (i >= MAXSETS)
			{
			  printf
			    ("Error: Simple set with id %i not found in line %i\n",
			     j, linenr);
			  fclose (file);
			  return NULL;
			}

		      if (convert != 0)	/* not for default one */
			{
			  charset[set].paircnt++;	/* next pair */
			  if (charset[set].paircnt > MAXPAIRS)	/* oups */
			    {
			      printf
				("Error: Too many pairs in grouped set in line %i. Recompile with bigger table.\n",
				 linenr);
			      fclose (file);
			      return NULL;
			    }
			}
		    }		/* for GROUPED one */
		  if (charset[set].type == SET_DICTIONARY)
		    {
#ifdef DEBUG
		      printf ("DEBUG: dictionary set found (field len %i)\n",
			      i);
#endif
		      /* format is: position,charset_id,startlen,endlen 
		         position is either 0 (append) or 1 (prepend)     */
		      j = 0;
		      s = 0;
		      stage = 0;	/* how I hate text parsing in C... */
		      while (j < i)	/* i = strlen(value) */
			{
			  temp[s++] = buffer[j];
			  if (buffer[j] == ',')
			    {
			      if (stage >= 3)
				{
				  printf
				    ("  Error: '%s' contains more than 4 values.\n",
				     buffer);
				  fclose (file);
				  return NULL;
				}
			      temp[s] = 0;	/* zero terminate */
			      s = 0;	/* restart in temp */
			      /* printf ("Split %s\n",temp); */
			      temp_values[stage++] = atoi ((const char *)temp);
			    }
			  j++;
			}
		      /* convert last value, too */
		      temp_values[stage] = atoi ((const char *)temp);
#ifdef DEBUG
		      printf ("Decoded %i %i %i %i\n",
			      temp_values[0], temp_values[1],
			      temp_values[2], temp_values[3]);
#endif
		      /* check that the given setid is valid */
		      charset[set].set[charset[set].paircnt] = NULL;
		      i = 0;
		      while (i < MAXSETS)
			{
			  /* only for read in ones, since unread ones have still id==0 */
			  if (charset[i].id == temp_values[1])
			    {
			      if ((charset[i].type != SET_SIMPLE) &&
				  (charset[i].type != SET_GROUPED))
				{
				  printf
				    ("Error: Set %i referenced in set %i in line %i must be a simple or grouped set, not another dictionary one.\n",
				     i, set, linenr);
				  fclose (file);
				  return NULL;
				}
			      /* store pointer to set in set[] */
			      charset[set].set[charset[set].paircnt] =
				&charset[i];
			      break;
			    }
			  i++;
			}
		      if (i >= MAXSETS)
			{
			  printf ("Error: Set id %i not found in line %i\n",
				  temp_values[0], linenr);
			  fclose (file);
			  return NULL;
			}
		      /* store read values */
		      s = charset[set].paircnt++;	/* shortcut and inc */
		      charset[set].pos[s] = temp_values[0];
		      charset[set].startlen[s] = temp_values[2];
		      charset[set].endlen[s] = temp_values[3];
		    }		/* for DICTIONARY one */
		}		/* for each field */

#ifdef DEBUG
	      if (charset[set].type == SET_SIMPLE)
		{
		  printf (" done. %x .. %x (%i chars)\n",
			  charset[set].chars[0],
			  charset[set].chars[charset[set].length - 1],
			  charset[set].length);
		}
#endif
	      if (charset[set].type == SET_GROUPED)
		{
		  if (charset[set].def == NULL)
		    {
		      printf
			("Error: Grouped set id %i has no default set in line %i\n",
			 charset[set].id, linenr);
		      fclose (file);
		      return NULL;
		    }
#ifdef DEBUG
		  printf (" done for grouped (pairs: %i)\n",
			  charset[set].paircnt);
#endif
		}
#ifdef DEBUG
	      if (charset[set].type == SET_DICTIONARY)
		{
		  /* SET_DICTIONARY */
		  printf (" done for dictionary (append/prepend sets: %i)\n",
			  charset[set].paircnt);
		}
#endif
	      set++;		/* have one more now */
	    }			/* charset definition line? */
	  /* lines not starting with 'c', '0', '1' or '2' are simple ignored */
	}			/* empty line? */
      else
	{
#ifdef DEBUG
	  printf ("Skipping line %i\n", linenr);
#endif
	}
      if (set > MAXSETS)
	{
	  printf ("Error: More sets defined than declared in line %i\n",
		  linenr);
	  fclose (file);
	  return NULL;
	}
    }				/* while not eof */

  if (set < MAXSETS)
    {
      printf ("Error: Less sets defined than declared at eof.\n");
      fclose (file);
      return NULL;
    }

  printf ("read %i charsets. Closing file.\n", MAXSETS);
  fclose (file);
  return charset;
}

/* ************************************************************************ */
/* INTERNAL: read job/charset description file
   return codes: ptr to charsets - okay
		 NULL 		 - error 	*/

unsigned int
pwdgen_readset (const char *set, struct ssPWD *sPWD)
  {
  unsigned long rc;
  struct ssKey *cfg, *key;
  char valid[256] =
    "password_prefix,prefix,config_encoding,dictionary_file,dictionary_mutations,dictionary_stages,charset_id,input_encoding,output_encoding";

  /* read the config file, if we find it */
  cfg = pwdcfg_read (set);

  if (NULL == cfg)
    {
      return 1;
    }

  /* check that we have only allowed keys, exit() otherwise */
  rc = pwdcfg_valid_keys (cfg, &valid[0], 0);
  if (0 != rc)
    {
      return rc;
    }

  key = pwdcfg_find_key (cfg, "charset_id", CFG_NOFAIL);
  if (NULL != key)
    {
      sPWD->id = atoi (key->value);
    }

  /* set all the others */
  pwdgen_set_cfg_keys (sPWD, cfg);

  /* XXX TODO: free memory from cfg */

  return 0;
  }

/** <b>INTERNAL</b>
    Interpret the following set of keys from a given chunk or job/charset
    description file:\n
    - prefix
    - config_encoding
    - password_prefix (legacy support)
    - dictionary_file
    - dictionary_file_offset
    - dictionary_mutations
    - dictionary_stages
    - input_encoding
    - output_encoding
    */

void
pwdgen_set_cfg_keys (struct ssPWD *sPWD, const struct ssKey *cfg)
  {
  struct ssKey *key;
  struct sPwdString* prefix;

  key = pwdcfg_find_key (cfg, "password_prefix", CFG_OPTIONAL);
  if (NULL != key)
    {
    /* legacy support for prefix in hex */
    h2a(key->value);
    /* prefix is read-only */
    prefix = (struct sPwdString*)sPWD->prefix;
    strncpy (prefix->content, key->value, 250);
    prefix->bytes = strlen(prefix->content);
    prefix->characters = -1;
    prefix->encoding = ISO_8859_1;
    printf ("\n Warning: Found legacy key 'password_prefix', assuming hex and ISO-8859-1.\n");
    fflush(NULL);
    }
  key = pwdcfg_find_key (cfg, "prefix", CFG_OPTIONAL);
  if (NULL != key)
    {
    if (sPWD->prefix->bytes != 0)
      {
      printf (" Warning: Encountered both 'password_prefix' and 'prefix' in config file.\n");
      printf ("          'prefix' will take precedence and overwrite 'password_prefix'!\n");
      }
    /* prefix is read-only */
    prefix = (struct sPwdString*)sPWD->prefix;
    strncpy (prefix->content, key->value, 250);
    prefix->content[250] = 0;			/* zero term. in case */
    prefix->bytes = strlen(prefix->content);
    prefix->characters = -1;
    prefix->encoding = UTF_8;
    key = pwdcfg_find_key (cfg, "config_encoding", CFG_OPTIONAL);
    if (NULL != key)
      {
      prefix->encoding = pwdgen_encoding_from_name(key->value);
      if (prefix->encoding == INVALID_ENCODING)
	{
        printf (" Error: 'config_encoding=%s' is invalid.\n", key->value);
	pwdgen_error(sPWD, -1);
	return;
	}
      }
    else
      {
      printf (" Warning: 'config_encoding' not set, assuming UTF-8.\n");
      }
    }

  key = pwdcfg_find_key (cfg, "dictionary_file", CFG_OPTIONAL);
  if (NULL != key)
    {
    strncpy (sPWD->dictionary, key->value, 250);
    sPWD->dictionary[250] = 0;
    }

  /* allow dictionary as alias for dictionary_file */
  key = pwdcfg_find_key (cfg, "dictionary", CFG_OPTIONAL);
  if (NULL != key)
    {
    strncpy (sPWD->dictionary, key->value, 250);
    sPWD->dictionary[250] = 0;
    }

  /* this one will only be used from a chunk description file, not from
     a job description file, but we handle both branches here */
  key = pwdcfg_find_key (cfg, "dictionary_file_offset", CFG_OPTIONAL);
  if (NULL != key)
    {
    sPWD->dict_file_ofs = atol (key->value);
    }

  /* don't use as_int() because it warns needlessly */
  key = pwdcfg_find_key (cfg, "dictionary_mutations", CFG_OPTIONAL);
  if (NULL != key)
    {
    sPWD->todo_mutations = atoi (key->value);
    }

  key = pwdcfg_find_key (cfg,"dictionary_stages", CFG_OPTIONAL);
  if (NULL != key)
    {
    sPWD->todo_stages = atoi (key->value);
    }

  key = pwdcfg_find_key (cfg,"input_encoding", CFG_OPTIONAL);
  if (NULL != key)
    {
    sPWD->generator_encoding = pwdgen_encoding_from_name(key->value);
    if (sPWD->generator_encoding == INVALID_ENCODING)
      {
      printf (" Error: input_encoding '%s' is not a valid encoding.\n",
        key->value);
      pwdgen_error(sPWD, -1);
      return;
      }
    }

  /* the default is no encodings set, so that pwdgen_init() will set a default */
  sPWD->encodings = 0;

  /* after we have set some default encodings, see if the config file wants
     to override them */
  key = pwdcfg_find_key (cfg,"output_encoding", CFG_OPTIONAL);
  if (NULL != key)
    {
    /* XXX TODO: handle more than one ("UTF-8, ISO-8859-1") and also errors */
    printf (" Config requests to produce: %s\n", key->value);
    pwdgen_use_encoding(sPWD, pwdgen_encoding_from_name(key->value));
    }

  }

