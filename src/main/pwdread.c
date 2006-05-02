/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

  Implements the various config file reading routines.

*/

//#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/pwdread.h"
#include "../include/dicop.h"

/* ************************************************************************* */
/* INTERNAL: read file with charset definitions and build character sets from
             them, inclusive grouped charsets from the simple ones
   return codes: ptr to charsets - okay
		 NULL 		 - error 	*/

extern unsigned int MAXSETS;

/* Thou Shallst Not Add Arbitrary Limits To Thy Programme - but I did ... */
#define BUFSIZE 16384
#define MAXFIELDSIZE 16

struct ssCharset* pwdgen_read ( struct ssPWD* sPWD )
  {
  int pos, linenr, i, j, k, set, convert, mul, stage, s;
  FILE *file;
  unsigned int count;
  unsigned char line[BUFSIZE+16]; /* input lines up to BUFSIZE characters */
  unsigned char buffer[16];	  /* temp. buffer for conversations */
  unsigned char temp[BUFSIZE];	  /* temp. buffer for conversations */
  unsigned int temp_values[8];	  /* temp. buffer for values */
  struct ssCharset *charset;	  /* to store read in charsets */
  int alloc = 0;		  /* already allocated memory? */

  printf ("\n Opening global charset description file...");

  if (NULL == (file = pwdgen_find_file ("charsets.def","r")))
    {
    return NULL;
    }

  MAXSETS = 0;

  /* read in line by line 
     format is as follows:
     count=6
     # comments
     0:1:41:42...	simple charset, fields:
			type id  chars
			0    1.. 41 42 43 ...
     1:2:1=1:2=1...	grouped charset, fields: 
			type id  pairs (position 1..length or -1..-length)
			1    1.. position=charset
     2:2:0,32,1,2:...	dictionary, fields: 
			type id  pairs (pos= 0|1) 
			2    1.. pos,set,startlen,endlen: 
  */
  charset = NULL;	/* silence the might be used unitialized warning */
  linenr = -1;
  set = 0;		/* index of set we are reading in */
  while (!feof(file))
    {
    linenr ++; count = 0; line[0] = 0; 
    while (count < BUFSIZE && !feof(file))
      {
      line[count] = fgetc(file);
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
    if ((line[0] == '#') || (count == 0)) { continue; }

    /* XXX TODO could eliminate this check */
    if (count != 0)
      {
      if (strncmp(line,"count=",6) == 0)
	{
	/* read out the count to alloc memory */
	/* 9999 diff. charsets should be enough for everyone...*/
        strncpy (buffer,&line[6],4); buffer[4] = 0;		
        MAXSETS = atoi(buffer);
	if (MAXSETS <= 0)
	  {
          printf ("At least one charset should exist!\n");
          fclose(file);
          return NULL;
	  }
#ifdef DEBUG
	printf ("count = %i\n",MAXSETS);
#endif
        }

      if ((line[0] == '0') || (line[0] == '1') || (line[0] == '2') ||
          (line[0] == '3'))
	{
	/* start of charset definitions, so make sure we have memory */
	if ((MAXSETS == 0) && (alloc != 0))	
	  {
          printf ("Charset count must come before definitions!\n");
          fclose(file);
          return NULL;
	  }
	if ((MAXSETS == 0) && (alloc == 0))	
	  {
          printf ("At least one charset should exist!\n");
          fclose(file);
          return NULL;
	  }
	if (alloc == 0)
	  {	
#ifdef DEBUG
	  printf ("malloc for %i charsets\n",MAXSETS);
#endif
	  charset = malloc (sizeof (struct ssCharset) * MAXSETS);
          if (charset == NULL)
            {
            printf ("Cannot alloc memory!\n");
            fclose(file);
            return NULL;
            }
	  for (j=0; j < MAXSETS; j++)
	    {
            charset[j].id = 0;		/* not yet read */
	    }
	  alloc = 1;			/* flag that we already allocated */
	  }
#ifdef DEBUG
        printf ("parsing now line %i\n",linenr);
#endif
        /* now parse the charset definition line */
	if (line[1] != ':')
	  {
	  printf ("Error: Charset type must be followed by ':' in line %i\n",
	  	   linenr);
          fclose(file);
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
	pos = 2;	/* skip /^(0|1|2|3): */
	/* convert first the id of the set */
	if (line[pos] == ':')
	  {
	  printf ("Error: Charset ID must not be empty in line %i\n",linenr);
          fclose(file);
          return NULL;
	  }
	i = 0;
	while (pos < count && line[pos] != ':' && i <= 4)
	  {
	  buffer[i++] = line[pos++]; buffer[i] = 0;
	  }
	if (i > 4)
	  {
	  printf ("Error: Charset ID to big in line %i\n",linenr);
          fclose(file);
          return NULL;
	  }
	convert = atoi(buffer);
	if (convert <= 0)
	  {
	  printf ("Error: Charset ID must be > 0, but is %i, in line %i\n",
	   convert,linenr);
          fclose(file);
          return NULL;
	  }
        i = 0;
        while (i < MAXSETS)
          {
          /* only for read in ones, since unread ones have still id==0 */
          if (charset[i].id == convert)
            {
	    printf ("Error: Charset ID %i defined twice in line %i\n",
  	     convert,linenr);
            fclose(file);
            return NULL;
	    }
	  i++;
	  }
	charset[set].id = convert;

#ifdef DEBUG
	printf ("Reading charset %i ... ",charset[set].id);
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
	    printf ("Error: Field to big in line %i\n",linenr);
            fclose(file);
            return NULL;
	    }
#ifdef DEBUG
//          printf ("Parsing now field '%s' with len %i\n",buffer,i);
#endif
	  if (charset[set].type == SET_SIMPLE)
	    {
	    /* convert hex to dec */
	    i--; convert = 0; mul = 1;
	    while (i >= 0)
	      {
	      /* assumes some ASCIIism */
	      if (buffer[i] >= '0' && buffer[i] <= '9')
	        {
	        convert += mul * (buffer[i] - '0');
	        }
	      else
	        {
	        if (buffer[i] > 'f') { buffer[i] -= 'A'; }	/* to lower */
	        if (buffer[i] < 'a' || buffer[i] > 'f')
	  	  {
	  	  printf ("Error: Illegal hex character %c in line %i\n",
		 	  buffer[i],linenr);
          	  fclose(file);
		  return NULL;
		  }
	        convert += mul * (buffer[i] - 'a' + 10);
	        }
	      mul *= 16;
	      i--;
	      } /* for each nibble */
	    /* got one, so put it in and add one more to len */
	    charset[set].chars[ charset[set].length++ ] = convert;
	    }
	  if (charset[set].type == SET_GROUPED)
	    {
	    /* split pos=id into pos and id at = */
	    j = 0; convert = 0; mul = 1;
	    while (j < i)
	      {
	      if (buffer[j] == '=') { break; } 
	      if ((buffer[j] != '-') && (buffer[j] < '0' || buffer[j] > '9'))
		{	
		printf ("Error: Illegal character '%c' in line %i\n",
		 	  buffer[j],linenr);
		fclose(file);
		return NULL;
		}
	      temp[j] = buffer[j];
	      j++;
	      }
	    temp[j] = 0;	
	    if (buffer[j] != '=')
	      {
  	      printf (
		"Error: Illegal character '%c' (expect: '=') in line %i\n",
		 buffer[j],linenr);
	      }
	    convert = atoi(temp);	/* position */
#ifdef DEBUG
		printf ("convert = %i (from %s)\n",convert,temp);
#endif
	    if (convert != 0)		/* not default position? */
	      {
	      charset[set].pos[charset[set].paircnt] = convert;
	      }
	    j++; k = 0; /* skip = */
	    while (j < i)
	      {
	      if ((buffer[j] < '0' || buffer[j] > '9'))
		{	
		printf (
		    "Error: Illegal character %c (expect: 0..9) in line %i\n",
		     buffer[j],linenr);
		fclose(file);
		return NULL;
		}
	      temp[k++] = buffer[j++];
	      }
	    temp[k] = 0;
	    j = atoi(temp);
#ifdef DEBUG
		printf ("j = %i (from %s)\n",j,temp);
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
	      	  printf ("Error: Set %i referenced in set %i in line %i must be a simple set, not another grouped or dictionary one.\n",i,set,linenr);
	          fclose(file);
		  return NULL;
		  }
		/* store pointer to set in set[] or def, depending on */
		if (convert != 0)
		  {
	    	  charset[set].set[charset[set].paircnt] = &charset[i];
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
	      printf ("Error: Simple set with id %i not found in line %i\n",
		     j,linenr);
	      fclose(file);
	      return NULL;
	      }

	    if (convert != 0)	/* not for default one */
	      {
 	      charset[set].paircnt++; 	/* next pair */
 	      if (charset[set].paircnt > MAXPAIRS) 	/* oups */
		{
		printf (
		    "Error: Too many pairs in grouped set in line %i. Recompile with bigger table.\n",
		     linenr);
		fclose (file);
		return NULL;
		}
	      }
	    } /* for GROUPED one */
	  if (charset[set].type == SET_DICTIONARY)
            {
#ifdef DEBUG
	    printf ("DEBUG: dictionary set found (field len %i)\n",i);
#endif
	    /* format is: position,charset_id,startlen,endlen 
	       position is either 0 (append) or 1 (prepend)	*/
            j = 0; s = 0; stage = 0;	/* how I hate text parsing in C... */
            while (j < i)		/* i = strlen(value) */
              {
	      temp[s++] = buffer[j];
              if (buffer[j] == ',')
                {
                if (stage >= 3)
	          {
	          printf ("  Error: '%s' contains more than 4 values.\n",buffer);
		  fclose (file);
	          return NULL;
		  }
	        temp[s] = 0;		/* zero terminate */
	        s = 0;			/* restart in temp */
		/* printf ("Split %s\n",temp); */
	        temp_values[stage++] = atoi(temp);
	        }
              j++;
              }
	    /* convert last value, too */
            temp_values[stage] = atoi(temp);
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
	      	  printf ("Error: Set %i referenced in set %i in line %i must be a simple or grouped set, not another dictionary one.\n",i,set,linenr);
	          fclose(file);
		  return NULL;
		  }
		/* store pointer to set in set[] */
		charset[set].set[charset[set].paircnt] = &charset[i];
		break;
		}
	      i++;
	      }
	    if (i >= MAXSETS)
	      {	
	      printf ("Error: Set id %i not found in line %i\n", temp_values[0],linenr);
	      fclose(file);
	      return NULL;
              }
	    /* store read values */
            s = charset[set].paircnt++;		/* shortcut and inc */
	    charset[set].pos[s] = temp_values[0];
	    charset[set].startlen[s] = temp_values[2];
	    charset[set].endlen[s] = temp_values[3];
            } /* for DICTIONARY one */
	  } /* for each field */

#ifdef DEBUG
	if (charset[set].type == SET_SIMPLE)
	  {
	  printf (" done. %x .. %x (%i chars)\n",
	    charset[set].chars[0],
	    charset[set].chars[charset[set].length-1],
	    charset[set].length);
	  }
#endif
	if (charset[set].type == SET_GROUPED)
	  {
	  if (charset[set].def == NULL)
	    {
	    printf
	     ("Error: Grouped set id %i has no default set in line %i\n",
		charset[set].id,linenr);
	    fclose(file);
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
	set++;	/* have one more now */
	} /* charset definition line? */
      /* lines not starting with 'c', '0', '1' or '2' are simple ignored */
      } /* empty line? */
    else
      {
#ifdef DEBUG
      printf ("Skipping line %i\n",linenr);
#endif
      }
    if (set > MAXSETS)
      {
      printf ("Error: More sets defined than declared in line %i\n", linenr);
      fclose(file);
      return NULL;
      }
    } /* while not eof */ 

  if (set < MAXSETS)
    {
    printf ("Error: Less sets defined than declared at eof.\n");
    fclose(file);
    return NULL;
    }

  printf ("read %i charsets. Closing file.\n",MAXSETS);
  fclose(file);
  return charset;
  }

/* ************************************************************************ */
/* INTERNAL: read job/charset description file
   return codes: ptr to charsets - okay
		 NULL 		 - error 	*/

unsigned int pwdgen_readset ( unsigned char* set, struct ssPWD* sPWD )
  {
  unsigned long rc;
  struct ssKey* cfg, *key;
  unsigned char valid[256] = 
    "password_prefix,dictionary_file,dictionary_mutations,dictionary_stages,charset_id"; 
 
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
    sPWD->id = atoi(key->value);
    }

  /* set all the others */
  pwdgen_set_cfg_keys ( sPWD, cfg);

  /* XXX TODO: free memory from cfg */

  return 0;
  }

/* INTERNAL: from chunk or job/charset description file set certain keys */

void pwdgen_set_cfg_keys ( struct ssPWD* sPWD, struct ssKey* cfg )
  {
  struct ssKey* key;

  key = pwdcfg_find_key (cfg, "password_prefix", CFG_NOFAIL);
  if (NULL != key)
    {
    h2a(key->value);
    strncpy (sPWD->prefix,key->value,250);
    sPWD->prefix[250] = 0;			  /* zero term. in case */
    sPWD->prefix_length = strlen (sPWD->prefix);  /* cache string length */
    }
  
  key = pwdcfg_find_key (cfg, "dictionary_file", CFG_NOFAIL);
  if (NULL != key)
    {
    strncpy (sPWD->dictionary,key->value,250);
    sPWD->dictionary[250] = 0;
    }

  /* this one will only be used from a chunk description file, not from
     a job description file, but we handle both branches here */ 
  key = pwdcfg_find_key (cfg, "dictionary_file_offset", CFG_NOFAIL);
  if (NULL != key)
    {
    sPWD->dict_file_ofs = atol(key->value);
    }

  /* don't use as_int() because it warns needlessly */ 
  key = pwdcfg_find_key (cfg, "dictionary_mutations", CFG_NOFAIL);
  if (NULL != key)
    {
    sPWD->todo_mutations = atoi(key->value);
    }
      
  key = pwdcfg_find_key (cfg, "dictionary_stages", CFG_NOFAIL);
  if (NULL != key)
    {
    sPWD->todo_stages = atoi(key->value);
    }
      
  return;
  }

