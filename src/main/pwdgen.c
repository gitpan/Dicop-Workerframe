/*!
 * @file
 * @ingroup workerframe
 * @brief Main part of the password generator for simple/grouped charsets.
 * 
 * Implements the general password generator: 
 *  - initialization for all charsets
 *  - actual generator for simple and grouped charsets
 *
 * @copydoc copyrighttext
*/

#include <dicop.h>
#include <pwdread.h>	/* for reading in charsets */
#include <pwddict.h>
#include <locale.h>

/** The maximum number of charsets found in the charsets.def file.
    Is initialized by pwdgen_read_charsets().
 */
unsigned int MAXSETS = 0;

#define GEN_VERSION "v0.37"

/* ************************************************************************* */
/** Find a charset in the list read by pwdgen_read_charsets(), by the given
    ID and return a pointer to it. Returns NULL when no charset has been
    found. Also handles virtual charsets with an ID < 0 by returning a
    pointer to a fixed, preset charset for the given type (-1, -2 or -3). */

const struct ssCharset *
pwdgen_find_set (const struct ssCharset *charset, const long id)
{
  int i;

  for (i = 0; i < MAXSETS; i++)
    {
      if (charset[i].id == id)
	{
	  /* found the one */
	  return &charset[i];	/* return ptr to charset */
	}
    }
  /* found none */
  printf ("\n Error: Character set with id %i not found.\n", sPWD.id);
  return NULL;
}

/** Create a virtual charset of the given type (-1 .. -4) */
struct ssCharset* pwdgen_virtual_set(const int type)
  {
  int i;
  struct ssCharset* set;

  if ((type > -1) || (type < -4))
    {
    printf ("\n Error: Type for virtual set is %i and thus out of range of -1 .. -4.\n\n", type);
    return NULL;
    }

  if (NULL == (set = malloc(sizeof(struct ssCharset))))
    {
    printf ("\n Error: Could not allocate memory for new virtual charset.\n");
    return NULL;
    }

  set->id = type;
  /* -1 => 0, -2 => 1, -3 => 2, -4 => 3 */
  set->type = abs(type) - 1;
  set->length = 0;
  /* we do not append/prepend anything */
  set->paircnt = 0;

  if (set->type == SET_SIMPLE)
    {
    /* simulate a 256-byte charset */
    set->length = 256;
    for (i = 0; i < 256; i++)
      {
      set->chars[i] = i;
      }
    }
  else
    {
    /* XXX TODO 
    set->set[0] = ...
    set->def = ...
    */
    }

  return set;
  }

/* ************************************************************************* */
/** Parse the output from setlocale() to determine encoding of console. */

enum eEncodings pwdgen_parse_locale (const char *locale)
  {
  /* A few examples of strings encountered in the wild:
	de_DE.UTF-8@euro
	C
	en_EN.UTF-8
	English_United States.1252
	LC_CTYPE=en_US.UTF-8;LC_NUMERIC=C;LC_TIME=C;LC_COLLATE=C;LC_MONETARY=C;...
  */

  int s = strlen(locale);
  int src = s;
  int j = 0;
  char enc_name[64];

  /* "" => ASCII */
  if (s == 0)
    {
    return ASCII;
    }
  if ((s > 10) && (0 == memcmp(locale, "LC_CTYPE=", 9)))
    {
    /* skip ahead to the first ";" or the end */
    src = 9;
    while ((src < s) && locale[src] != ';')
      {
      src++;
      }
    }
  /* start at the last character */
  src--;
  /* go backwards from the last character until we are at either a
     [^A-Za-z0-9_-], or at the start */
  while ( (src >= 0) && 
	  ( (locale[src] >= 'a' && locale[src] <= 'z') ||
            (locale[src] >= 'A' && locale[src] <= 'Z') ||
            (locale[src] >= '0' && locale[src] <= '9') ||
            (locale[src] == '_') ||
            (locale[src] == '-') )) { src--; }
  src++;
  /* src now points to the first valid char */
  while (src < s && j < 30 && locale[src] != ';')
    {
    enc_name[j] = locale[src]; src++; j++;
    }
  enc_name[j] = 0;				/* zero-terminate */
  /* convert string to enum */
  return pwdgen_encoding_from_name(enc_name);
  }

/* ************************************************************************* */
/** Query the locale info and store it for proper output.
*/

int 
pwdgen_query_locale (struct ssPWD *sPWD)
  {
  char* locale;

  /* query the current locale and store it for later output via pwdgen_print */
  sPWD->locale = UTF_8;			/* a sensible default for modern systems */
  /* enable internationalization (I18N) support: */
  setlocale(LC_ALL, "");
  /* query the current locale, enabled by the line above */
  locale = setlocale(LC_ALL, NULL);
  if (NULL != locale)
    {
    sPWD->locale = pwdgen_parse_locale(locale);
    if (sPWD->locale == INVALID_ENCODING)
      {
      printf (" *** Warning: Cannot determine encoding from locale '%s', fallback to ASCII.\n", locale);
      sPWD->locale = ASCII;
      }
    printf (" Current locale            : %s\n Current encoding          : %s\n", locale,
      pwdgen_encoding(sPWD->locale));
    }

  return 0;
  }

/* ************************************************************************* */
/** Initialize the password generator.\n

   @todo This routine directly manipulates the global struct sPWD and returns a
   pointer to it. We should malloc() an ssPWD struct and use/return this.

*/

struct ssPWD *
pwdgen_init (const char *start, const char *end,
	     const char *target,
	     const char *set, const unsigned int timeout, const struct ssKey *keys)
  {
  /* #define SET_SIMPLE 0 
     #define SET_GROUPED 1
     #define SET_DICTIONARY 2
     #define SET_EXTRACT 3 */ 

  char* charset_type[12] = {
    "SIMPLE",
    "GROUPED",
    "DICTIONARY"
    "EXTRACT",
  };
  unsigned int i, j, s, found;
  unsigned char c;
  unsigned int *table;
  struct ssPWD *gen;
  struct ssCharset *setptr;
  struct sPwdString** strcur;

  sPWD.charsets = pwdgen_read_charsets ();	/* read charsets from file */
  if ((sPWD.charsets == NULL) || (MAXSETS == 0))
    {
    printf ("\n Couldn't read in charsets - aborting.\n");
    return NULL;
    }

  sPWD.identify = 0;
  sPWD.error = 0;
  sPWD.max_pwds = 32;		/* timeout check soon after start */

  strncpy (sPWD.target, target, PWD_LEN);	/* store target */
  strncpy (sPWD.start, start, PWD_LEN);		/* store start pwd */
  strncpy (sPWD.end, end, PWD_LEN);		/* set last password to end */

  /* in hex => ASCII len = hexlen / 2 */
  sPWD.startlen = strlen (start) >> 1;
  sPWD.endlen = strlen (end) >> 1;
  sPWD.targetlen = strlen (target);
  sPWD.gen_length = sPWD.startlen;
  sPWD.length = sPWD.gen_length;

  sPWD.replacement = '?';			/* default to "?" */

  sPWD.config = (struct ssKey*) keys;		/* ptr to list of nodes from chunk description file */
  sPWD.once = 0;				/* 0 => more than once */
  if ( (sPWD.startlen != 0) &&			/* startlen = 0 for dictionaries */
       (strncmp (start, end, PWD_LEN) == 0) )
    {
    sPWD.once = 1;				/* 1 => only once */
    }

  pwdgen_query_locale( &sPWD );

  sPWD.crc = 0;
  sPWD.overflow = sPWD.gen_length;

  /* the first password is tested by main(), so we start with the second
     encoding of the first passwort. Or in case there is no other requested
     encoding, with the second password. */
  sPWD.cur_encoding = 1;

  /* Our table-driven generator can only loop through charsets where 1 byte
     == 1 character, so use ISO_8859_1 as default - this can be overwritten
     with pwdgen_use_encoding() or pwdgen_use_encodings() or in the config
     file with "input_encoding" */
  sPWD.generator_encoding = ISO_8859_1;

  /* We init the generator without the prefix inside the "pwd" field. The
     reason is that the prefix might be in any encoding (like UTF-8), while
     the generator runs in a different encoding. Only after each password is
     generated, we will fuse together the fixed prefix and the current pwd. */

  h2a (sPWD.end);
  h2a (sPWD.start);

  sPWD.end[sPWD.endlen] = 0;		/* zero terminate just in case */
  sPWD.start[sPWD.startlen] = 0;	/* zero terminate just in case */

  sPWD.pwds = 1;		/* always do at least one pwd */
  sPWD.timeout = timeout;
  sPWD.report_status = 30;	/* report after 30 seconds, than every 60s and so on */
  sPWD.tstart = time (NULL);

  sPWD.tlast = sPWD.tstart;		/* fake a last check to now() */
  sPWD.tlast_report = sPWD.tstart;	/* fake a last report to now() */

  /* set some defaults */
  strcur = (struct sPwdString**)&sPWD.prefix;		/* prefix is read-only */
  *strcur = pwdgen_new_string(&sPWD, PWD_LEN * 6, UTF_8);

  strcur = (struct sPwdString**)&sPWD.word;		/* word is read-only */
  *strcur = pwdgen_new_string(&sPWD, PWD_LEN * 6, UTF_8);

  strcur = (struct sPwdString**)&sPWD.org_word;		/* org_word is read-only */
  *strcur = pwdgen_new_string(&sPWD, PWD_LEN * 6, UTF_8);

  /* will be overwritten with the config value "dictionary_file" or "dictionary" */
  strcpy (sPWD.dictionary,"not set");
  sPWD.todo_mutations = 0;
  sPWD.todo_stages = 0;
  sPWD.first_mutation = 1;
  sPWD.id = 0;
  sPWD.dict_file = NULL;
  sPWD.dict_file_ofs = 0;

  /* if prefix/dictionary etc were set in the chunk description file, use them */
  pwdgen_set_cfg_keys (&sPWD, keys);

  /* if set is not a plain number (like 42), but ending in .set (like 42.set)
     then read in this file as a job charset description file with further
     options */
  i = 0;
  /* handle "virtual" sets like "-1" */
  if (set[0] == '-')
    {
    i++;
    }
  while (set[i] >= '0' && set[i] <= '9')
    {
    i++;
    if (i > 250)
      {
      printf ("Error: charset name too long.\n");
      return NULL;
      }
    }
  if (set[i] != 0)
    {
    /* Stopped at some character different than 0..9 and 0x0 */
    /* call may override the following fields in the struct:
       id
       dictionary
       dict_file_ofs
       prefix
       todo_mutations
       todo_stages
     */
    if (0 != pwdgen_readset (set, &sPWD))
      {
      printf ("\n Error: Couldn't read charset description file\n");
      return NULL;
      }
    }
  else
    {
    /* set is a plain number */
    sPWD.id = atoi (set);
    }

  if (sPWD.id >= 0)
    {
    sPWD.set = (struct ssCharset*) pwdgen_find_set (sPWD.charsets, sPWD.id);
    }
  else
    {
    sPWD.set = pwdgen_virtual_set(sPWD.id);
    }
  /* error? */
  if (NULL == sPWD.set)
    {
    return NULL;
    }

  sPWD.type = sPWD.set->type;
  printf (" Using charset with id %i, type: %s.", sPWD.id, charset_type[sPWD.type]);
  
  if (sPWD.type == SET_SIMPLE)
    {
    printf ("  Contains %i charactesrs.\n", sPWD.set->length);
    }

  sPWD.length = sPWD.gen_length;
  sPWD.overflow = sPWD.length;
  sPWD.pwd[sPWD.length] = 0;	/* zero terminate just in case */

  /* set current password to the start password */
  memcpy (&sPWD.pwd, sPWD.start, sPWD.startlen);

  sPWD.flags = 0;		/* default off */

  /* init depending in type */
  if (sPWD.type == SET_DICTIONARY || sPWD.type == SET_EXTRACT)
    {
    sPWD.cur_mutation = 1;
    sPWD.cur_stage = 1;
    sPWD.last_word = 0;		/* last word read? not yet! */
    sPWD.append = 0;		/* no append/prepend at start */
    sPWD.prepend = 0;		/* first pwd to be prepended, if so */

    /*we always go at least through the "forward" stage */
    if (0 == sPWD.todo_stages)
      {
      sPWD.todo_stages = 1;
      }
    /* if nostage was set, use the words as they are written in the dictionary */
    if (0 == sPWD.todo_mutations)
      {
      sPWD.todo_mutations = DICT_ORIGINAL;
      }
    /* find the number of the first set bit as the start mutation */
    sPWD.first_mutation = 1;
    i = sPWD.todo_mutations;
    while ( (i & 1) == 0)
      {
      sPWD.first_mutation++; i >>= 1;
      }
    sPWD.done_mutations = sPWD.todo_mutations;
    sPWD.done_stages = sPWD.todo_stages;

    if (sPWD.type == SET_EXTRACT)
      {
      sPWD.pwds = 0;			/* we count the pwds manually */
      sPWD.flags = 1;			/* dictionary mutator: don't read next word */
      sPWD.startlen = atoi(start);	/* for EXTRACT, start/end give the len in characters */
      sPWD.endlen = atoi(end);
      sPWD.report_status = 0;		/* don't need to report status */

      /* the extractor only handles ISO_8859_1 for now */
      if (1 == pwdcfg_as_int( keys, "extract_utf8", CFG_NOFAIL))
        {
        sPWD.generator_encoding = UTF_8;
        printf ("\n The extractor code will extract strings in UTF-8.");
	/* unless the config already specified one */
	if (sPWD.encodings == 0)
	  {
          sPWD.encodings = 1;
          sPWD.encoding[0] = UTF_8;
	  }
        }
      else
        {
        sPWD.generator_encoding = ISO_8859_1;
	/* unless the config already specified one */
	if (sPWD.encodings == 0)
          {
          sPWD.encodings = 1;
	  sPWD.encoding[0] = ISO_8859_1;
	  }
        printf ("\n The extractor code will extract strings in ISO-8859-1.\n");

        sPWD.extract_set =
          (struct ssCharset*)
		pwdgen_find_set (sPWD.charsets, pwdcfg_as_int( keys, "extractset_id", CFG_FAIL));

        /* does charset exist? */
        if (NULL == sPWD.extract_set)
          {
          printf ("Extraction charset id %i does not exist - aborting.\n",
            pwdcfg_as_int( keys,"extractset_id", CFG_NOFAIL) );
          return NULL;
          }
        /* And is it a SIMPLE one? */
        if (sPWD.extract_set->type != SET_SIMPLE)
          {
          printf ("Cannot use charset %i (a non-simple one) as extract set - aborting.\n",
            sPWD.extract_set->id);
          return NULL;
          }
        if (sPWD.extract_set->type == SET_SIMPLE)
	  {
          printf (" Extracting according to set %i, type %s, %i characters.\n",
             sPWD.extract_set->id, charset_type[sPWD.extract_set->type], sPWD.extract_set->length);
          }
	else
	  {
	  printf (" Extracting characters according to set %i, type %s.\n",
             sPWD.extract_set->id, charset_type[sPWD.extract_set->type]);
	  }
        }
      }
    else /* type == SET_DICT */
      {
      /* the dictionary is supposed to be in UTF-8 */
      sPWD.generator_encoding = UTF_8;
      printf ("\n The dictionary is treated as encoded in UTF-8.");
      /* unless the config already specified one */
      if (sPWD.encodings == 0)
	{
	sPWD.encoding[0] = UTF_8;
	sPWD.encodings = 1;
	}

      /* word is read-only: */
      strcur = (struct sPwdString**)&sPWD.word;
      *strcur = pwdgen_new_string(&sPWD, PWD_LEN * 6, UTF_8);
      if (0 != sPWD.error)
	{
	/* some error allocating new string */
	return NULL;
	}

      /* open the dictionary file */
      if (pwddict_openfile (&sPWD) != 0)
	{
	return NULL;
	}
      /* and fastforward to the word matching startpwd */
      if (pwddict_fastforward (&sPWD) != 0)
	{
	return NULL;
	}
      }

      /* in case we have a dictionary, and want to append something on it, we
         need more than one sPWD struct, namely one holding the "primary"
         generator information, e.g. the one that goes through the dictionary
         word-by-word and mutates each of them, and one for each charset that
         we want to append or prepend */

      if (sPWD.set->paircnt > 0)
	{
	/* allocate memory */
	i = sizeof (struct ssPWD) * sPWD.set->paircnt;
	sPWD.gen_pwd = malloc (i);
	if (sPWD.gen_pwd == NULL)
	  {
	  printf ("Error: Can't alloc %i bytes for secondary generators.\n", i);
	  return NULL;
	  }
	/* init all these generators */
	setptr = sPWD.set;
	printf ("\n  Init %i secondary password generators... ",
		  setptr->paircnt);
	for (j = 0; j < setptr->paircnt; j++)
	  {
	  gen = &sPWD.gen_pwd[j];	/* take ptr to current generator */

	  gen->error = 0;
  
	  gen->generator_encoding = sPWD.generator_encoding;
	  gen->encoding[0] = sPWD.generator_encoding;
	  gen->encodings = 1;

	  /* The worst case is 4 bytes for each character in UTF-8, so we make the
	     buffer big enough to hold all possible strings that we could encounter,
	     saving on buffer resizing. */
          strcur = (struct sPwdString**)&gen->cur;
	  *strcur = pwdgen_new_string(gen, PWD_LEN * 4 + 4, gen->encoding[0]);
          if (0 != gen->error)
            {
            return NULL;
            }

	  gen->prefix = pwdgen_new_string(&sPWD, 16, sPWD.generator_encoding);
	  gen->word = NULL;
	  gen->org_word = NULL;
	  gen->dictionary[0] = 0;
	  gen->dict_file = NULL;
	  gen->dict_file_ofs = 0;
	  gen->timeout = 0;		/* no timeout checking */
	  gen->report_status = 0;	/* no status report */
	  gen->tstart = 0;
	  gen->tlast = 0;
	  gen->tlast_report = 0;
	  gen->crc = 0;
	  gen->set = setptr->set[j];
	  gen->charsets = sPWD.charsets;
#ifdef DEBUG
	  printf ("\nDebug: id %i type %i ", gen->set->id,
	      gen->set->type);
#endif
	  gen->id = gen->set->id;
	  gen->type = gen->set->type;
	  gen->gen_length = setptr->startlen[j];
	  gen->length = gen->gen_length;
	  gen->startlen = gen->gen_length;
	  gen->endlen = setptr->endlen[j];

	  /* set start pwd from start length (to first pwd with that length) */
	  pwdgen_init_tables (gen, 2);
	  for (i = 0; i < gen->startlen; i++)
	    {
	    gen->start[i] = gen->pwd[i];
	    }
	  gen->start[i] = 0;	/* zero terminate */

	  /* set end password from end length (to last pwd with that length) */
	  pwdgen_init_tables (gen, 2);
	  for (i = 0; i < gen->endlen; i++)
	    {
	    gen->end[i] = gen->pwd[i];
	    }
	  gen->end[i] = 0;	/* zero terminate */
#ifdef DEBUG
	  printf ("end: %s endlen %i ", gen->end, (int) gen->endlen);
#endif
	  pwdgen_finalize(gen);
	  }
	printf ("  done.");
	}			/* end init secondary generators */
    }
  else
    {
    /* if (sPWD.type == SET_GROUPED || sPWD.type == SET_SIMPLE) */
    /* unless the config already specified the output_encoding */
    if (sPWD.encodings == 0)
      {
      sPWD.encoding[0] = ISO_8859_1;	/* default for the table-driven generator */
      sPWD.encodings = 1;		/* we have only one */
      }

    if (sPWD.endlen < sPWD.startlen)
      {
      /* endlen must be at least equal to startlen */
      printf ("Error: End password shorter than start password.\n");
      return NULL;
      }

    /* type is SIMPLE or GROUPED */
    pwdgen_init_tables (&sPWD, 0);	/* 0 == first time init w/ end pwd */

    /* ** check whether end is valid with this set and create column ends ** */
    for (i = 0; i < sPWD.endlen; i++)
      {
      c = sPWD.end[i];
      found = 0;
      table = sPWD.tables[i];
      j = sPWD.lens[i];
      for (s = 0; s < j; s++)
	{
	if (table[s] == c)
	  {
	  sPWD.end_digit[i] = s;	/* store column counter */
	  found = 1;
	  break;
	  }
	}
      if (found == 0)
	{
	printf ("\n Error: Illegal character %.2X at pos %i in end pwd.\n",
	   c, i);
	return NULL;
	}
      }

    pwdgen_init_tables (&sPWD, 1);	/* 1 == first time init w/ start pwd */
    /** * check whether start is valid with this set * **/
    for (i = 0; i < sPWD.startlen; i++)
      {
      c = sPWD.pwd[i];
      found = 0;
      table = sPWD.tables[i];
      j = sPWD.lens[i];
      for (s = 0; s < j; s++)
	{
	if (table[s] == c)
	  {
	  sPWD.pwd_digit[i] = s;	/* store column counter */
	  found = 1;
	  break;
	  }
        }
      if (found == 0)
	{
	printf ("\n Error: Illegal character %.2X at pos %i in start pwd.\n",
	  c, i);
	return NULL;
	}
      } /* end for sPWD.starlen */
    }

  /* The worst case is 4 bytes for each character in UTF-8, so we make the buffer big
     enough to hold all possible strings that we could encounter, saving on buffer resizing. */
  /* sPWD.cur is read-only, so fix that up */

  strcur = (struct sPwdString**)&sPWD.cur;
  *strcur = pwdgen_string(&sPWD, (unsigned char*)sPWD.pwd, sPWD.length, 
				PWD_LEN * 4 + 4, sPWD.generator_encoding); 
  if (0 != sPWD.error)
    {
    /* some error allocating new string */
    return NULL;
    }

  if (sPWD.prefix->bytes != 0)
    {
    pwdgen_print (&sPWD, "\n Password prefix is: '%s'", sPWD.prefix);
    }

#ifdef DEBUG
  printf ("\n Debug:");
  printf ("  Using set %i of type %i.\n", sPWD.id, sPWD.type);
  printf ("  Dictionary '%s' (offset %i)\n", sPWD.dictionary,
	  sPWD.dict_file_ofs);
  printf ("  Current stage %i from %i, done %i.\n", sPWD.cur_stage,
	  sPWD.todo_stages, sPWD.done_stages);
  printf ("  Current mutation %i from %i, done %i.\n", sPWD.cur_mutation,
	  sPWD.todo_mutations, sPWD.done_mutations);
#endif

  printf ("\n Password generator %s successfully initialized.\n",
	  GEN_VERSION);
  return &sPWD;
}

/* ************************************************************************* */
/** INTERNAL: Finalize the password generator after the user initialization
    phase. This is used to skip charset conversions if they are unnecc. etc. */
int pwdgen_finalize (const struct ssPWD *pwd)
  {
  /* input is read-only: */
  struct sPwdString* cur_rw;
  struct ssPWD* pwd_rw = (struct ssPWD*)pwd;

  if (0 != pwd->error)
    {
    return PWD_ERROR;
    }

  /* if we only have one encoding, convert the prefix to the same encoding */
  if ( (pwd->encodings == 1) && 
       (pwd->generator_encoding != pwd->prefix->encoding)
     )
    {
    pwdgen_convert_to(pwd, pwd->prefix, pwd->generator_encoding, pwd->replacement);
    }

  if ( (pwd->encodings == 1) && 
       (pwd->generator_encoding == pwd->encoding[0]) &&
       (pwd->prefix->bytes == 0)
     )
    {
#ifdef DEBUG
    printf ("\n Need no charset conversion, pointing pwd->cur to pwd->pwd\n");
#endif
    /* The generator produces passwords in the same encoding that the user
       also requested to be produced, and it is the only encoding we need,
       and we do not have a fixed prefix, so make the content of pwd->cur
       simple point to pwd->pwd */
    /* cur is read-only: */
    cur_rw = (struct sPwdString*)pwd->cur;
    free(cur_rw->content);
    cur_rw->content = pwd->pwd;
    cur_rw->_buffer = pwd->pwd;
    cur_rw->encoding = pwd->generator_encoding;
    /* and prevent the user from modifying or freeing this string */
    cur_rw->flags = PWDSTR_RO_STATIC;
    }
  else
    {
    /* if the converter is running in the wrong encodig, fix this */
#ifdef DEBUG
    printf ("\n We might need to do charset conversions for each password.\n");
    printf (" Generator encoding: %s pwd->encoding: %s\n",
	pwdgen_encoding(pwd->generator_encoding), pwdgen_encoding(pwd->encoding[0]));
#endif
    if ((pwd->prefix->bytes != 0) ||
        (pwd->generator_encoding != pwd->encoding[0]))
      {
      pwd_rw->cur_encoding = 0;
      pwdgen_update_cur(pwd, 1);
      }
    pwd_rw->cur_encoding = 1;
    }

  pwd_rw->_last_pwd = 0;		/* 0 => more than one pwd and/or encoding */
  if (pwd->encodings > 1 && (0 != pwd->once))
    { 
    pwd_rw->once = 0;		/* we might need to generate more than one encoding
				   from the current password */
    pwd_rw->_last_pwd = 1;	/* but stop after all the encodings from this password */
    }

  if (0 != pwd->error)
    {
    return PWD_ERROR;
    }
  return PWD_INIT_OK;
  }

/* ************************************************************************* */
/** Reset the fields on the password generator (for SIMPLE/GROUPED charsets)
    so that it generates the password sequence again. This is usefull for
    appending the same sequence to each dictionary word.\n\n

    Returns 0 for ok, != 0 for error.\n\n

    This routine skips a lot of the checks that are done in pwdgen_init(),
    since we assume that the check is done once with pwdgen_init(), and thus
    further use of start/end pwd is ok (these two never change). */

int
pwdgen_reinit (struct ssPWD *sPWD)
  {
  unsigned int i;

#ifdef DEBUG
  printf ("Reinit set %i, type %i\n", sPWD->id, sPWD->type);
#endif

  if (sPWD->type == SET_DICTIONARY)
    {
    printf ("Error: Cannot re-init dictionary set.\n");
    return PWD_ERROR;
    }

  /* set current password back to start */
  for (i = 0; i < sPWD->startlen; i++)
    {
    sPWD->pwd[i] = sPWD->start[i];
    }
  sPWD->pwd[i] = 0;		/* zero terminate just in case */

  sPWD->once = 0;		/* 0 => more than once */
  if (strncmp (sPWD->start, sPWD->end, 255) == 0)
    {
    sPWD->once = 1;		/* 1 => only once */
    }
  sPWD->gen_length = sPWD->startlen;
  sPWD->length = sPWD->gen_length;
  sPWD->overflow = sPWD->gen_length;

  /* type is SIMPLE or GROUPED, so init tables */
  pwdgen_init_tables (sPWD, 1);	/* 1 == first time init w/ start pwd */

  return PWD_OK;
  }

/** Set the given error code inside the password generator and
    returns it. Zero indicates no error. */
int pwdgen_error (const struct ssPWD *pwd, int error)
  {
  /* pwd is read-only: */
  struct ssPWD *pwd_rw = (struct ssPWD*)pwd;
  
  pwd_rw->error = error;
  /* flush outputs in case of critical errors */
  fflush(NULL);
  return error;
  }

/* ************************************************************************* */
/** <b>INTERNAL</b>\n
    Called after the password generator has stopped. */

void pwdgen_stop (struct ssPWD *sPWD)
  {
  time_t now;

  sPWD->pwds += sPWD->pwds_done;
  sPWD->pwds_done = 0;
  now = time (NULL);
  sPWD->took = difftime (now, sPWD->tstart);
  }

/* ************************************************************************* */
/* INTERNAL 
   Set the took field in the password generator to the time elapsed from
   start until now. */

void pwdgen_took (struct ssPWD *sPWD)
  {
  time_t now = time(NULL);

  sPWD->took = difftime (now, sPWD->tstart);
  /* clock goes backward? (maybe someone adjusted for winter time) */
  if (sPWD->took < 0)
    {
    sPWD->took = 0;
    }
  }

/* ************************************************************************* */
/** <b>INTERNAL</b>\n 
   Check for timeout, or whether we need to do a status report \n
   return codes: 0 - okay, next pwd\n
		 4 - timeout, stop */

int pwdgen_time_check (struct ssPWD *sPWD)
  {
  long timediff;
  time_t now;

  now = time (NULL);
 
  /* status report? */
  if (sPWD->report_status != 0)
    {
    pwdgen_took(sPWD);

    /* clock goes backward? (maybe someone adjusted for winter time) */
    timediff = (long)difftime (now, sPWD->tlast_report);
    if (timediff < 0)
      {
      /* correct our start value by the amount of the timewarp */
      sPWD->tlast_report = now;
      timediff = 0;
      }
	
    if ((unsigned long)timediff >= sPWD->report_status)
      {
      pwdgen_print_time("\n %s  ");
      printf ("Did %.0Lf pwds in %.0Lfs (%.0Lf pwds/s), current:\n",
	sPWD->pwds, sPWD->took, sPWD->pwds / sPWD->took );
      hexdump_pwd ("", sPWD->pwd, sPWD->length);
      /* make the output to appear immidiately */
      fflush(NULL);
      sPWD->tlast_report = now;
      /* 30, 60, 120, 240, 480, 960, 1920 seconds */
      if (sPWD->report_status <= 1920)
	{
        sPWD->report_status *= 2;
	}
      }
    }

  /* timeout check requested? */
  if (sPWD->timeout != 0)
    {
    /* clock goes backward? (maybe someone adjusted for winter time) */
    timediff = (long)difftime (now, sPWD->tlast);
    if (timediff < 0)
      {
      /* correct our start value by the amount the timewarp */
      sPWD->tstart -= timediff;
      }
    sPWD->tlast = now;		/* save for next check */
    sPWD->took = difftime (now, sPWD->tstart);
    /* if zero seconds have elapsed, at least double the count */
    if (sPWD->took < 1)
      {
      sPWD->max_pwds *= 2;
      }
    if (sPWD->took > sPWD->timeout)
      {
      return PWD_TIMEOUT;
      }			/* timeout, break */
    }

  return PWD_OK;
  }

/* ************************************************************************* */
/** <b>INTERNAL</b>\n 
   Update the ->cur structure by converting it from the current password.\n
   If the parameter always_convert is non-zero, then the ->cur structure
   will be always updated with the current password and this routine returns
   zero.\n
   Otherwise, the routine will check if the password contains high-bit
   characters. If it doesn't, no conversion is done, and the routine returns
   non-zero, thus allowing skipping all the conversion for the current
   password. This prevents ASCII strings like "hello" being needlessly
   converted to "different" encodings, which would produce the same string,
   anyway.
   */

int pwdgen_update_cur (const struct ssPWD *sPWD, const int always_convert)
  {
  unsigned int i, highbit;
  struct sPwdString *cur, *temp;
  struct ssPWD *pwd;

  /* input is read-only: */
  pwd = (struct ssPWD *)sPWD;

  /* always_convert == 0: do the conversion only when nec. 
     we can check for highbit characters and skip the conversion
     unless the requested encoding is a multi-byte one, then 
     we need to convert anyway: */
  if ( (always_convert == 0) &&
       (sPWD->encoding[sPWD->cur_encoding] <= MULTIBYTE_ENCODING))
    {
    highbit = 0;
    /* check whether the current generated pwd contains only characters < 0x80 */
    for (i = 0; i < sPWD->length; i++)
      {
      if ( 0 != (sPWD->pwd[i] & 0x80))
        {
        highbit = 1; break;
        }
      }
    if (0 == highbit)
      {
      /* we can skip the encoding step as the password would be the
	 same again, so reset for next password and return */
#ifdef DEBUG
      printf ("No highbit bytes in pwd, skipping conversion\n");
#endif
      pwd->cur_encoding = 0;
      return -1;
      }
    }

  /* ->cur is read-only: */
  cur = (struct sPwdString*)sPWD->cur;

#ifdef DEBUG
  hexdump_string("update_cur: cur",cur);
  hexdump ("pwd", sPWD->pwd, sPWD->length);
#endif

  /* if we have a prefix, use this first */
  if (sPWD->prefix->bytes != 0)
    {
#ifdef DEBUG
    hexdump_string("prefix", sPWD->prefix);
    printf ("Prepending the prefix, length: %i cur->len: %i\n", sPWD->length, sPWD->cur->bytes);
#endif

    pwdgen_strdup(sPWD, cur, sPWD->prefix);
    if (sPWD->generator_encoding != sPWD->prefix->encoding)
      {
      temp = pwdgen_string(sPWD, sPWD->pwd, sPWD->length, PWD_LEN * 6, sPWD->generator_encoding);
      pwdgen_strcat(sPWD, cur, temp);
      }
    else
      {
      /* copy over the raw data */
      memcpy(&cur->content[cur->bytes], sPWD->pwd, sPWD->length);
      cur->characters = (size_t)-1;
      cur->bytes += sPWD->length;
      }
    }
  else
    {
    /* we assume that cur's buffer is big enough */
    cur->bytes = sPWD->length;
    cur->characters = (size_t)-1;
    cur->encoding = sPWD->generator_encoding;
    /* copy over the raw data */
#ifdef DEBUG
    printf ("Copy over the raw data into cur: ");
    printf ("%i len bytes from %p to %p\n", sPWD->length, sPWD->pwd, cur->content);
#endif
    memcpy(cur->content, sPWD->pwd, sPWD->length);
    }

  /* XXX TODO: when the string is marked as READ_ONLY + STATIC, prevent the
     conversion below from freeing the contained buffer. */
  /* Convert the password to the requested encoding, also calculating
     the length in characters. */
#ifdef DEBUG
  printf ("cur_encoding: %i '%s'\n", sPWD->cur_encoding, pwdgen_encoding(sPWD->encoding[sPWD->cur_encoding]));
  printf ("encodings: %i\n", sPWD->encodings);
  fflush(NULL);
#endif
  pwdgen_convert_to(sPWD, cur, sPWD->encoding[sPWD->cur_encoding], sPWD->replacement);

#ifdef DEBUG
  hexdump_string("Current PWD encoded", cur);
  printf ("Trying next encoding %i\n", pwd->cur_encoding + 1);
#endif

  /* next round, try to use the next encoding */
  pwd->cur_encoding++;

  /* only return  if the password really changed: */
  if ((cur->bytes == pwd->length) &&
      (0 == memcmp(cur->content, pwd->pwd, cur->bytes)))
    {
    /* the password wasn't really changed by the conversion */
    return -1;
    }
  return 0;
  }

/* ************************************************************************* */
/** <b>INTERNAL</b>\n 
   Generate the next password by updating the sPWD->pwd and sPWD->cur
   structures. The table-driven generator here does one password at a time.\n\n
   
   return codes: 0 - okay, next pwd\n
		 1 - overshot, last pwd checked, stop now\n
		 4 - timeout, stop\n
*/

int pwdgen_next (struct ssPWD *sPWD)
  {
  int digit;
  unsigned int *table;		/* current column */
  size_t *bytes;		/* to update cur->bytes */

  /* check how many encodings we have, and if we need to produce the
     current password in more than one encoding */
  if (sPWD->pwd != sPWD->cur->content)
    {
    if ( (sPWD->cur_encoding > 0) && 
	 (sPWD->cur_encoding < sPWD->encodings) )
      {
#ifdef DEBUG
      printf ("Producing next encoding: %s\n", 
	pwdgen_encoding(sPWD->encoding[sPWD->cur_encoding]));
#endif

      /* if 0, then the current password contained high-bit chars and
         a conversion to the other requested charsets was nec. */
      if (0 == pwdgen_update_cur(sPWD, 0))
	{
        /* this increment here might increase pwds_done higher than
	   pwds_max, so the timeout check might run a bit later. However,
	   the number of requested encodings is typically very small (<5),
           and this should not pose a problem */
        sPWD->pwds_done++;

        /* we are done here */
        return PWD_OK;
        }
      }

#ifdef DEBUG
    printf ("Produced last encoding, reset (%i) (%i)\n", sPWD->cur_encoding, sPWD->encodings);
#endif

    /* if we reached the last password, we need to stop after we reached all the
       encodings, so setup things to terminates the loop in main.c */
    if (0 != sPWD->_last_pwd)
      {
      return 1;
      }
    /* continue normally (with the first encoding, either because it is
       already 0, or we hit the last, resetting it to 0 again) */
    sPWD->cur_encoding = 0;
    }

  digit = sPWD->gen_length - 1;	/* last place first */

  /* check timeout and calculate number of pwds done */
  if (++sPWD->pwds_done > sPWD->max_pwds)
    {
    sPWD->pwds += sPWD->pwds_done;	/* add them up */
    sPWD->pwds_done = 0;
      
    /* status report or timeout check? */
    if (pwdgen_time_check(sPWD) != 0)
      {
      return PWD_TIMEOUT;
      }
    }				/* endif many pwds done, so check time */

  table = sPWD->tables[digit];
  while (digit >= 0)
    {
      /* table needs one more place */
      sPWD->pwd[digit] =
	(char) table[++sPWD->pwd_digit[digit]];
      if (sPWD->pwd_digit[digit] < sPWD->lens[digit])	/* no overflow? */
	{
	  if ((sPWD->gen_length != sPWD->endlen) ||
	      (sPWD->pwd_digit[digit] != sPWD->end_digit[digit]))
	    {
	      /* ****************************************************
		 this is the HOT PATH, e.g. the generator comes almost
		 always through here and then exits */

	      /* overflow goes from 1 to length */
	      sPWD->overflow = sPWD->gen_length - digit;

	      if (sPWD->pwd != sPWD->cur->content)
		{
#ifdef DEBUG
      		printf ("Producing first encoding: %s\n", 
		  pwdgen_encoding(sPWD->encoding[0]));
#endif
		pwdgen_update_cur(sPWD, 1);
		}

	      return PWD_OK;		/* no more overflow, and can't be last pwd */

	      /* end of HOT PATH 
		**************************************************** */
	    }
	  break;			/* no more overflow, but possible last pwd */
	}
      /* overflow occured, so check next digit */

      sPWD->pwd_digit[digit] = 0;				/* reset counter */
      sPWD->pwd[digit] = (char)table[0];			/* reset column */
      digit--;							/* next place */
      table = sPWD->tables[digit];				/* next column table */
    }
  if (digit < 0)
    {
      /* all places overflowed, so extend password by one character */
      sPWD->gen_length++;	/* increment gen length */
      sPWD->length++;		/* increment length */

      /* cur is read-only: */
      bytes = (size_t*)&sPWD->cur->bytes;
      *bytes = sPWD->length;
      sPWD->cur_encoding = 0;		/* start with requested encoding 0 again */
      pwdgen_init_tables (sPWD, 2);	/* set up new tables 
					   2 = also init all pwd columns
					 */

      /* overshot and done all passwords? */
      if (sPWD->gen_length > sPWD->endlen)
	{
#ifdef DEBUG
	  printf ("length greater than endlen, stopping\n");
#endif
	  sPWD->overflow = sPWD->gen_length;	/* calc correct overflow */

	  if (sPWD->pwd != sPWD->cur->content)
	    {		
	    pwdgen_update_cur(sPWD, 1);

	    /* do not return 1 if we need to do more encodings for the last
	       password, or we miss some encodings of the last password! */
	    if (sPWD->cur_encoding < sPWD->encodings)
	      {
#ifdef DEBUG
	      printf ("Last password, but not yet last encoding (1).\n");
#endif
	      sPWD->_last_pwd = 1;
	      return PWD_OK;
              }
	    }
	  return 1;			/* hit last password, so stop */
	}
      digit = 0;
    }

  /* If the table generator runs in a different charset than the one the
     use requested (typically if you request UTF-8, since the generator
     can only use charsets where 1 byte == 1 character), then we need to
     convert the current password in this path, too. */
  if ((sPWD->prefix->bytes != 0) ||
      (sPWD->pwd != sPWD->cur->content))
    {		
    pwdgen_update_cur(sPWD, 1);
    }

  sPWD->overflow = sPWD->gen_length - digit;	/* calc correct overflow */
  /*** overflowed to the left, but either
       more than one time, or last place
       was equal to last_end_digit ***/
  if ((sPWD->gen_length != sPWD->endlen) ||
      (sPWD->pwd_digit[digit] != sPWD->end_digit[digit]))
    {
    return PWD_OK;		/* can't be end pwd then */
    }
  digit = sPWD->endlen;
  while (digit-- > 0)
    {
    if (sPWD->pwd[digit] != sPWD->end[digit])
      {
      return PWD_OK;		/* 0 => go on */
      }
    }

  if ((sPWD->pwd != sPWD->cur->content) && (sPWD->cur_encoding < sPWD->encodings))
    {
#ifdef DEBUG
    printf ("Last password, but not last encoding (2).\n");
    printf ("Cur encoding: %i encodings: %i\n", sPWD->cur_encoding, sPWD->encodings);
#endif
    sPWD->_last_pwd = 1;
    return PWD_OK;
    }

  return 1;			/* hit last password */
  }

/* ************************************************************************* */

/** INTERNAL USE ONLY:
   Generate the tables for a GROUPED or SIMPLE charset for one pwd length.
   Although costly, we need to do it only when the pwd length changes, which is
   very seldom.\n
   For a simple charset, the table of tablepointers contains just X times the
   same pointer.\n
   Also initializes the password to contain the proper characters (f.i. when
   the pwd length changes). Zero terminates the pwd just in case.\n

   When just changing the length, certain optimizations could be done (only
   init last table ptr etc), but these are not worth it, since that happens so
   seldom...\n
   type == 0: init end password\n
        == 1: init start password\n
        == 2: init current password\n
*/

void
pwdgen_init_tables (struct ssPWD *sPWD, const int type)
{
  int forward, backward;
  unsigned int i,j, l;
  struct ssCharset *set;

  if (sPWD->type == SET_SIMPLE)
    {
    l = sPWD->endlen;		/* end pwd? */
    if (type != 0)
      {
      l = sPWD->gen_length;
      }				/* current pwd ? */
    for (i = 0; i < l; i++)
      {
      sPWD->tables[i] = sPWD->set->chars;
      sPWD->lens[i] = sPWD->set->length;
      if (type == 2)
        {
	/* table doesn't matter */
	sPWD->pwd[i] = (char)sPWD->tables[0][0];
	}
      }
    }
  if (sPWD->type == SET_GROUPED)
    {
    /* set is a GROUPED one */
    /* figure out at which position which simple character must be used */
    l = sPWD->endlen;		/* end pwd? */
    if (type != 0)
      {
      l = sPWD->gen_length;
      }			/* current pwd ? */
    for (i = 0; i < l; i++)
      {
      /* the default set to be taken is the one with number zero */
      set = sPWD->set->def;
      /* i is from 0, so adjust to count from 1, and also backwards from -1 */
      forward = i + 1;
      backward = 0 - (l - i);
      for (j = 0; j < sPWD->set->paircnt; j++)
	{
	if ((sPWD->set->pos[j] == forward) ||
	  (sPWD->set->pos[j] == backward))
	  {
	  /* match! */
	  set = sPWD->set->set[j];
	  break;
	}
      }
      if (set == NULL)
        {
	printf ("\nError, set at pos %i is NULL.\n", i);
	exit (1);
	}
      sPWD->tables[i] = set->chars;	/* store the found set for */
      if (sPWD->tables[i] == NULL)
	{
	printf ("Error: Set id %i contains no characters", set->id);
	exit (1);
	}
      sPWD->lens[i] = set->length;	/* this column */
      sPWD->pwd_digit[i] = 0;		/* init counters */
      if (type == 2)
        {
	sPWD->pwd[i] = (char)sPWD->tables[i][0];	/* this char */
	}
      }
    }
  sPWD->pwd[sPWD->length] = 0;			/* zero terminate just so */
  sPWD->pwd_digit[sPWD->gen_length - 1] = 0;	/* init last counter */

  }

/* PUBLIC functions */

/* ************************************************************************* */
/** Take a piece of data (preferable plain text depending on current password)
    and modify the CRC to reflect this. The goal is to compute a CRC that
    depends on the fact that we covered all passwords and actually did something
    with them, so nobody can just fake the chunk as DONE without actually doing
    any work (well, they still can, but we can verify it more easily by
    crosschecking and thus are able to tell fake DONE results from real ones) */

void
pwdgen_update_crc (struct ssPWD *sPWD, const char *data, const unsigned int len)
{
  unsigned int i;

  i = 0;
  while (i < len)
    {
      /* should be really a strong MAC instead of simple sum */
      sPWD->crc += (unsigned long) data[i];
      i++;
    }
}

/* ************************************************************************* */

/** When each call to dofunction() checks more than one password (maybe by
    mutating each provided password), then the counter of how many passwords
    were checked all in all is wrong. This routine can be used to correct
    the counter by adding the provided number to the internal total sum.
*/

void
pwdgen_add (struct ssPWD *sPWD, const unsigned long pwds)
  {
  sPWD->pwds += (long double)pwds;
  }

/* ************************************************************************* */
/** if the found solution isn't exactly what the pwd generator is looping
   through, you can override the output by specifying it via pwdgen_found().  */

void
pwdgen_found (const struct ssPWD *pwd, const char *solution, const unsigned int len)
  {
  unsigned int i;
  /* pwd is read-only: */
  struct ssPWD *sPWD = (struct ssPWD*) pwd;

  /* XXX TODO: support sPwdString struct here */
  for (i = 0; i < len; i++)
    {
    sPWD->pwd[i] = solution[i];
    }
  sPWD->pwd[i] = 0;		/* zero terminate */
  sPWD->length = len;
  }
    
/* ************************************************************************* */
/** Given an encoding, add it to the list of encodings the password generator
   runs through. If the encoding was already present in the list, it is not
   added. Returns 0 if the encoding was added, 1 if it was already present
   and -1 if the list was already full. */

int pwdgen_add_encoding (struct ssPWD *sPWD, const enum eEncodings encoding)
  {
  unsigned long i;

  /* list is full */
  if (sPWD->encodings >= 32)
    {
    return -1;
    }

  for (i = 0; i < sPWD->encodings; i++)
    {
    if (sPWD->encoding[i] == encoding)
      {
      /* already in list */
      return 1;
      }
    }
  /* add to list */
  sPWD->encoding[sPWD->encodings] = encoding;
  sPWD->encodings++;
  return 0;
  }

/* ************************************************************************* */
/** Given a list of encodings, like UTF_8, ISO_8859-1, etc., adds all the
   given encodings to the list of encodings that the generated passwords
   should be in to the given list.\n
   See pwdgen_use_encoding() for clearing the list.\n
   If this list contains anything else than
   ASCII, then you <i>must</i> use pwd->cur to access the current password! */

void
pwdgen_use_encodings (
    const struct ssPWD *pwd, const enum eEncodings encoding[], const unsigned char count)
  {
  /* pwd is read-only: */
  struct ssPWD *sPWD = (struct ssPWD*)pwd;
  int i, j;

  for (i = 0; i < count; i++)
    {
    j = pwdgen_add_encoding(sPWD, encoding[i]);
    if (i == -1)
      {
      printf (" Warning: List of possible encodings is full.\n Skipping encoding %s.\n",
        pwdgen_encoding(encoding[i]));
      }
    }
  }

/* ************************************************************************* */
/** Given one encoding such as UTF_8, sets this as the one encoding the
   generated password will be in. This will clear the list of encodings
   and set the given encoding as the <b>only</b> encoding produced by the
   password generator!\n
   See pwdgen_use_encodings() for adding more encodings to the list.\n
   If this function is used with anything else than ASCII, then you
   <i>must</i> use pwd->cur to access the current password! */

void
pwdgen_use_encoding (
    const struct ssPWD *pwd, const enum eEncodings encoding)
  {
  /* pwd is read-only: */
  struct ssPWD *sPWD = (struct ssPWD*)pwd;

  sPWD->encoding[0] = encoding;
  sPWD->encodings = 1;
  }

/** Print the list of encodings the password generator will use. This
    routine is automatically called by main(). */

void
pwdgen_print_encodings (const struct ssPWD *sPWD)
  {
  unsigned long i;

  if (sPWD->encodings == 1)
    {
    printf ("Passwords will be produced in the following encoding:\n");
    }
  else
    {
    printf ("Passwords will be produced in the following %li encodings:\n",
      sPWD->encodings);
    }

  for (i = 0; i < sPWD->encodings; i++)
    {
    printf ("  %s\n", pwdgen_encoding(sPWD->encoding[i]));
    }
  }
