/*

  Main part of the password generator for simple/grouped charsets

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

  Implements the general password generator: 
    * initialization for all charsets
    * actual generator for simple and grouped charsets

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/pwdread.h"  	/* for reading in charsets */
#include "../include/pwddict.h"
#include "../include/dicop.h"

/* globals for a tad more speed */
unsigned int MAXSETS = 0;		/* initialized by pwdgen_read() */

#define GEN_VERSION "v0.33"

//#define DEBUG 1 

/* ************************************************************************* */
/* find set that matches the setid or abort with error */

struct ssCharset* pwdgen_find_set (struct ssCharset* charset, unsigned long id)
  {
  unsigned int i;

  for (i = 0; i < MAXSETS; i++)
    {
    if (charset[i].id == id)
      {
      /* found the one */
      return &charset[i];		/* return ptr to charset */
      }
    }
  /* found none */
  return NULL;
  }

/* ************************************************************************* */

struct ssPWD* pwdgen_init ( unsigned char* start, unsigned char* end,
			    unsigned char* target,
                            unsigned char* set, unsigned int timeout,
                            struct ssKey* keys
			   ) {

  int found,i,j,s;
  unsigned char c;
  unsigned int *table;
  struct ssPWD *gen;
  struct ssCharset *setptr;

  sPWD.charsets = pwdgen_read( &sPWD );	/* read charsets from file */
  if ((sPWD.charsets == NULL) || (MAXSETS == 0))
    {
    printf ("\n Couldn't read in charsets - aborting.\n");	
    return NULL;
    }
 
  max_pwds = 32;			/* timeout check soon after start */
  
  strncpy (sPWD.target,target,PWD_LEN);	/* store target */
  strncpy (sPWD.start,start,PWD_LEN);	/* store start pwd */
  strncpy (sPWD.end,end,PWD_LEN);	/* set last password to end */

  sPWD.config = keys;			/* ptr to list of nodes from 
					   chunk description file */
  sPWD.once = 0;			/* 0 => more than once */
  if (strncmp(start,end,PWD_LEN) == 0)
    {
    sPWD.once = 1;			/* 1 => only once */
    }
  /* in hex => ASCII len = hexlen / 2 */
  sPWD.startlen = strlen(start) >> 1;
  sPWD.targetlen = strlen(target) >> 1;
  sPWD.gen_length = sPWD.startlen;
  sPWD.endlen = strlen(end) >> 1;
  sPWD.length = sPWD.gen_length;
  sPWD.crc = 0;
  sPWD.overflow = sPWD.gen_length;
  
  /* while we init the generator, the password is left-aligned, without the
     prefix. After init, we shift the password to the right and copy in the
     fixed prefix. */

  h2a(sPWD.end);
  h2a(sPWD.start);

  sPWD.end[sPWD.endlen] = 0;		/* zero terminate just in case */
  sPWD.start[sPWD.startlen] = 0;	/* zero terminate just in case */

  sPWD.pwds = 1;			/* always does at least one pwd */
  sPWD.timeout = timeout;
  sPWD.tstart = time(NULL);
  last = sPWD.tstart;			/* fake a last check to now() */
 
  /* set some defaults */
  sPWD.prefix[0] = 0;
  sPWD.prefix_length = 0;
  strcpy (sPWD.dictionary,"not set");
  sPWD.todo_mutations = 0;
  sPWD.todo_stages = 0;
  sPWD.id = 0; 
  sPWD.word_length = 0;
  sPWD.org_word_length = 0;
  sPWD.dict_file = NULL;
  sPWD.dict_file_ofs = 0;

  /* if prefix/dictionary etc were set in the chunk description file, use them */
  pwdgen_set_cfg_keys( &sPWD, keys );
 
  /* if set is not a plain number (like 42), but ending in .set (like 42.set)
     then read in this file as a job charset description file with further
     options */
  i = 0;
  while (set[i] >= '0' && set[i] <= '9')
    {
    i++;
    if (i > 250) { printf ("set name too long\n"); return NULL; }
    }
  if (set[i] != 0)
    {
    /* stopped at some character different than 0..9 and 0x0 */
    /* call may override the following fields in the struct:
	id
	dictionary
	dict_file_ofs
	prefix
	prefix_length
	todo_mutations
	todo_stages
       */
    if (0 != pwdgen_readset(set,&sPWD))
      {
      printf ("\n Error: Couldn't read charset description file\n");
      return NULL;
      }
    }
  else
    {
    /* set is a plain number */
    sPWD.id = atoi(set);
    }

  sPWD.set = pwdgen_find_set (sPWD.charsets, sPWD.id);

  if (sPWD.set == NULL)
    {
    printf ("\n Error: Character set with id %i not found.\n", sPWD.id);
    return NULL;
    }					/* illegal set */
  sPWD.type = sPWD.set->type;
  printf (" Using charset with id %i, type %i.", sPWD.id, sPWD.type);
  
  /* set current password to start and copy the prefix before it */
  for (i = 0; i < sPWD.startlen; i++)
    {
    sPWD.pwd[sPWD.prefix_length + i] = sPWD.start[i];
    }
  sPWD.length = sPWD.gen_length + sPWD.prefix_length;
  sPWD.overflow = sPWD.length;
  sPWD.pwd[sPWD.length] = 0;	/* zero terminate just in case */
  /* prepend prefix */
  for (i = 0; i < sPWD.prefix_length; i++)
    {
    sPWD.pwd[i] = sPWD.prefix[i];
    }

  sPWD.flags = 0;			/* default off */

  /* init depending in type */
  if (sPWD.type == SET_DICTIONARY || sPWD.type == SET_EXTRACT)
    {
    sPWD.cur_mutation = 1;
    sPWD.cur_stage = 1;
    sPWD.last_word = 0;			/* last word read? not yet! */
    sPWD.append = 0;			/* no append/prepend at start */
    sPWD.prepend = 0;			/* first pwd to be prepended, if so */

    if (sPWD.type == SET_EXTRACT)
      {
      sPWD.flags = 1;			/* dictionary mutator: dont read next word */
      sPWD.startlen = atoi(start);
      sPWD.endlen = atoi(end);
      }
    else
      {
      if (0 == sPWD.todo_mutations)
        {
        sPWD.todo_mutations = 1;
        }
      if (0 == sPWD.todo_stages)
        {
        sPWD.todo_stages = 1;
        }
      sPWD.done_mutations = sPWD.todo_mutations;
      sPWD.done_stages = sPWD.todo_stages;

      /* open the dictionary file */
      if (pwddict_openfile(&sPWD) != 0) { return NULL; }
      /* and fastforward to the word matching startpwd */
      if (pwddict_fastforward(&sPWD) != 0) { return NULL; }
      }
  
    /*  in case we have a dictionary, and want to append something on it, we
	need more than one sPWD struct, namely one holding the "primary"
	generator information, e.g. the one that goes through the dictionary
	word-by-word and mutates each of them, and one for each charset, that
	we want to append or prepend */

    if (sPWD.set->paircnt > 0)
      {
      /* allocate memory */
      i = sizeof(struct ssPWD) * sPWD.set->paircnt;
      sPWD.gen_pwd = malloc ( i );
      if (sPWD.gen_pwd == NULL)
	{
	printf ("Error: Can't alloc %i bytes for secondary generators.\n",i);
	return NULL; 
	}
      /* init all these generators */
      setptr = sPWD.set;
      printf ("\n  Init %i secondary password generators... ", setptr->paircnt);
      for (j = 0; j < setptr->paircnt; j++ )
        {
        gen = &sPWD.gen_pwd[j];		/* take ptr to current generator */
        gen->prefix[0] = 0;
        gen->prefix_length = 0;
        gen->word_length = 0;
        gen->org_word_length = 0;
        gen->dictionary[0] = 0;
        gen->dict_file = NULL;
        gen->dict_file_ofs = 0;
        gen->timeout = 0;		/* no timeout checking */
        gen->crc = 0;
        gen->set = setptr->set[j];
        gen->charsets = sPWD.charsets;
#ifdef DEBUG
        printf ("\nDebug: id %i type %i ",gen->set->id, gen->set->type);
#endif
        gen->id = gen->set->id;
        gen->type = gen->set->type;
        gen->gen_length = setptr->startlen[j];
        gen->length = gen->gen_length;
        gen->startlen = gen->gen_length;
        gen->endlen = setptr->endlen[j];
  
        /* set start pwd from start length (to first pwd with that length) */
	pwdgen_init_tables(gen, 2); 
        for (i = 0; i < gen->startlen; i++)
	  {
	  gen->start[i] = gen->pwd[i];
	  }
	gen->start[i] = 0; /* zero terminate */
 
        /* set end password from end length (to last pwd with that length) */
	pwdgen_init_tables(gen, 2); 
        for (i = 0; i < gen->endlen; i++)
	  {
	  gen->end[i] = gen->pwd[i];
	  }
	gen->end[i] = 0; /* zero terminate */ 
#ifdef DEBUG
	printf ("end: %s endlen %i ", gen->end, (int)gen->endlen);
#endif
        }
      printf ("  done.");
      } /* end init secondary generators */
    }

  if (sPWD.type == SET_GROUPED || sPWD.type == SET_SIMPLE)
    {
    if (sPWD.endlen < sPWD.startlen)
      {
      // endlen must be at least equal to startlen
      printf ("Error: End password shorter than start password.\n");
      return NULL; 
      }

    /* prefix_len = sPWD.prefix_length; sPWD.prefix_length = 0; */
    /* type is SIMPLE or GROUPED */
    pwdgen_init_tables(&sPWD,0);	/* 0 == first time init w/ end pwd */
  
    /*** check whether end is valid with this set and create column ends ***/
    for (i = 0; i < sPWD.endlen; i++)
      {
      c = sPWD.end[i]; found = 0;
      table = sPWD.tables[i];
      j = sPWD.lens[i];
      for (s = 0; s < j; s++)
        {
        if (table[s] == c)
          {
          sPWD.end_digit[i] = s;		/* store column counter*/
          found = 1; break;
          }
        }
      if (found == 0)
        {
        printf ("\n Error: Illegal character %.2X at pos %i in end pwd.\n",
	 c,i);	
        return NULL;
        }
      }

    pwdgen_init_tables(&sPWD,1);	/* 1 == first time init w/ start pwd */
    /*** check whether start is valid with this set ***/
    for (i = 0; i < sPWD.startlen; i++)
      {
      c = sPWD.pwd[sPWD.prefix_length+i]; found = 0;
      table = sPWD.tables[i];
      j = sPWD.lens[i];
      for (s = 0; s < j; s++)
        {
        if (table[s] == c)
          {
          sPWD.pwd_digit[i] = s;		/* store column counter*/
          found = 1; break;
          }
        }
      if (found == 0)
        {
        printf ("\n Error: Illegal character %.2X at pos %i in start pwd.\n",
	 c,i);
        return NULL;
        }
      }
  
    }

#ifdef DEBUG
  printf ("\n Debug:");
  printf ("  Using set %i of type %i.\n", sPWD.id, sPWD.type);
  printf ("  Dictionary '%s' (offset %i)\n", sPWD.dictionary, sPWD.dict_file_ofs);
  printf ("  Current stage %i from %i, done %i.\n", sPWD.cur_stage, sPWD.todo_stages, sPWD.done_stages);
  printf ("  Current mutation %i from %i, done %i.\n", sPWD.cur_mutation, sPWD.todo_mutations, sPWD.done_mutations);
#endif
 
  printf ("\n Password generator '%s' successfully initialized.\n", GEN_VERSION);
  return &sPWD;
  }

/* ************************************************************************* */
/* reset the fields on the password generator (for SIMPLE/GROUPED ones) so
   that it generates the password sequence again (usefull for appending the
   same sequence to each dictionary word 

   returns 0 for ok, != 0 for error 

   this routine skips a lot of the checks that are done in pwdgen_init, since
   we assume that the check is done once with pwdgen_init, and thus further
   use of start/end pwd is ok since these two never change. */

int pwdgen_reinit ( struct ssPWD* sPWD ) {
  int i;

#ifdef DEBUG
  printf ("Reinit set %i, type %i\n",sPWD->id,sPWD->type);
#endif

  if (sPWD->type == SET_DICTIONARY)
    {
    printf ("Error: Cannot re-init dictionary set.\n");
    return 1;
    }

  /* set current password back to start */
  for (i = 0; i < sPWD->startlen; i++)
    {
    sPWD->pwd[i] = sPWD->start[i];
    }
  sPWD->pwd[i] = 0;			/* zero terminate just in case */

  sPWD->once = 0;			/* 0 => more than once */
  if (strncmp(sPWD->start,sPWD->end,255) == 0)
    {
    sPWD->once = 1;			/* 1 => only once */
    }
  sPWD->gen_length = sPWD->startlen;
  sPWD->length = sPWD->gen_length;
  sPWD->overflow = sPWD->gen_length;

  /* type is SIMPLE or GROUPED, so init tables */
  pwdgen_init_tables(sPWD,1);		/* 1 == first time init w/ start pwd */

  return 0;
  }

/* ************************************************************************* */
/* INTERNAL */

void pwdgen_stop ( struct ssPWD* sPWD )
  {
  sPWD->pwds += sPWD->pwds_done; sPWD->pwds_done = 0;
  now = time(NULL);
  sPWD->took = difftime(now,sPWD->tstart);
  a2h(sPWD->pwd,sPWD->length);
  }

/* ************************************************************************* */
/* INTERNAL 
   generate the next password 
   return codes: 0 - okay, next pwd
		 1 - overshot, last pwd checked, stop now
		 4 - timeout, stopped in midst
*/

int pwdgen_next ( struct ssPWD* sPWD )
  {
  int digit;
  unsigned int *table;				/* current column */
  long timediff;

  digit = sPWD->gen_length-1;			/* last place first */
//  printf ("overflow %i \n",digit);

  /* check timeout and calculate number of pwds done */
  if (++sPWD->pwds_done > max_pwds)
    {
    sPWD->pwds += sPWD->pwds_done;		/* add them up */
    sPWD->pwds_done = 0;
    if (sPWD->timeout != 0)			/* timeout check requested? */
      {
      now = time(NULL);
      /* clock goes backward? (maybe someone adjusted for winter time) */
      timediff = difftime(now,last);
      if ( timediff < 0 )
        {
        /* correct our start value by the amount the timewarp */
        sPWD->tstart -= timediff;
        }
      last = now;	/* save for next check */
      sPWD->took = difftime(now,sPWD->tstart);
      /* if zero seconds have elapsed, at least double the count */
      if (sPWD->took < 1)
        {
        max_pwds *= 2;
        }
      if (sPWD->took > sPWD->timeout) { return 4; }	/* timeout, break */
      }
    }

  table = sPWD->tables[digit]; 
  while (digit >= 0)
    {
    /* table needs one more place */
    sPWD->pwd[sPWD->prefix_length+digit] = 
      (char)table[++sPWD->pwd_digit[digit]];
    if (sPWD->pwd_digit[digit] < sPWD->lens[digit])	/* no overflow? */
      {
      if ((sPWD->gen_length != sPWD->endlen) ||
          (sPWD->pwd_digit[digit] != sPWD->end_digit[digit]))
        {
        /* overflow goes from 1 to length */
        sPWD->overflow = sPWD->gen_length-digit;
        return 0; 		/* no more overflow, and can't be last pwd */
        }
      break;			/* no more overflow, but possible last pwd */
      }
    /* overflow occured, so check next digit */
    
    sPWD->pwd_digit[digit] = 0;				/* reset counter */
    sPWD->pwd[sPWD->prefix_length+digit] = table[0]; 	/* reset column */
    digit--;						/* next place */ 
    table = sPWD->tables[digit];			/* next column table */
    }
  if (digit < 0)
    {
    /* all places overflowed, so extend password by one character */
    sPWD->gen_length++;				/* increment gen length */
    sPWD->length++;				/* increment length */
    pwdgen_init_tables(sPWD,2);			/* set up new tables 
						2 = also init all pwd columns
						*/
    /* overshot and done all passwords? */
    if (sPWD->gen_length > sPWD->endlen) 
      {
#ifdef DEBUG
	printf ("length greater than endlen, stopping\n");
#endif
      sPWD->overflow = sPWD->gen_length;	/* calc correct overflow */
      return 1;
      }
    digit = 0;
    }
  sPWD->overflow = sPWD->gen_length-digit;	/* calc correct overflow */
  /*** overflowed to the left, but either
       more than one time, or last place
       was equal to last_end_digit ***/
  if ((sPWD->gen_length != sPWD->endlen) ||
      (sPWD->pwd_digit[digit] != sPWD->end_digit[digit]))
    {
    return 0; 					/* can't be end pwd then */
    }
  digit = sPWD->endlen;
  while (digit-- > 0)
    {
    if (sPWD->pwd[sPWD->prefix_length + digit] != sPWD->end[digit])
      {
      return 0; 				/* 1 => end, 0 => go on */
      }
    }
  return 1;					/* hit last password */

  }

/* ************************************************************************* */

/* INTERNAL USE ONLY:
   Generate the tables for a GROUPED or SIMPLE charset for one pwd length.
   Although costly, we need to do it only when the pwd length changes, which is
   very seldom. 
   For a simple charset, the table of tablepointers contains just X times the
   same pointer.
   Also initializes the password to contain the proper characters (f.i. when
   the pwd length changes). Zero terminates the pwd just in case.

   When just changing the length, certain optimizations could be done (only
   init last table ptr etc), but these are not worth it, since that happens so
   seldom...
   type == 0: init end password
        == 1: init start password
        == 2: init current password
*/

void pwdgen_init_tables ( struct ssPWD* sPWD, int type )
  {
  int i,l,j,forward,backward;
  struct ssCharset *set;

  if (sPWD->type == SET_SIMPLE)
    {
    l = sPWD->endlen;					/* end pwd? */
    if (type != 0) { l = sPWD->gen_length; }		/* current pwd ? */
    for (i = 0; i < l; i++)
      {
      sPWD->tables[i] = sPWD->set->chars;
      sPWD->lens[i] = sPWD->set->length;
      if (type == 2)
        {
	/* table doesn't matter */
        sPWD->pwd[sPWD->prefix_length + i] = sPWD->tables[0][0];	
        }
      }
    } 
  if (sPWD->type == SET_GROUPED)
    {
    /* set is a GROUPED one */
    /* figure out at which position which simple character must be used */
    l = sPWD->endlen;				/* end pwd? */
    if (type != 0) { l = sPWD->gen_length; }	/* current pwd ? */
    for (i = 0; i < l; i++)
      {
      /* the default set to be taken is the one with number zero */
      set = sPWD->set->def;
      /* i is from 0, so adjust to count from 1, and also backwards from -1 */
      forward = i + 1;
      backward = -(l - i);
      for (j = 0; j < sPWD->set->paircnt; j++)
        {
        if ((sPWD->set->pos[j] == forward) ||      
            (sPWD->set->pos[j] == backward) )
          {
          /* match! */
          set = sPWD->set->set[j]; break;
          }
        }
      j = l-i-1;
      if (set == NULL)
	{
	printf ("\nError, set at pos %i is NULL.\n",i); exit(1);
	}
      sPWD->tables[i] = set->chars;		/* store the found set for */
      if (sPWD->tables[i] == NULL)
	{
	printf ("Error: Set id %i contains no characters",set->id);
	exit(1);
	}
      sPWD->lens[i] = set->length;		/* this column */
      sPWD->pwd_digit[i] = 0;			/* init counters */
      if (type == 2)
        {
        sPWD->pwd[sPWD->prefix_length+i] = sPWD->tables[i][0]; /* this char */
	}
      }
    }
  sPWD->pwd[sPWD->length] = 0;			/* zero terminate just so */
  sPWD->pwd_digit[sPWD->gen_length-1] = 0;	/* init last counter */

  }

/* PUBLIC functions */

/* ************************************************************************* 
   take a piece of data (preferable plain text depending on current password)
   and modify the CRC to reflect this. The goal is to compute a CRC that
   depends on the fact that we covered all passwords and actually did something
   with them, so nobody can just fake the chunk as DONE without actually doing
   any work (well, they still can, but we can verify it more easily by
   crosschecking and thus are able to tell fake DONE results from real ones) */

void pwdgen_update_crc (
  struct ssPWD* sPWD, unsigned char * data, unsigned int len )
  {
  unsigned int i;

  i = 0;
  while (i < len)
    {
    /* should be really a strong MAC instead of simple sum, does it matter? */
    sPWD->crc += (unsigned long)data[i]; i++;
    }
  }

/* ************************************************************************* 
  we did check some more passwords than one in one go, so add to counter */

void pwdgen_add ( struct ssPWD* sPWD, unsigned long pwds)
  {
  sPWD->pwds += pwds;
  }

/* ************************************************************************* */
/* if the found solution isn't exactly what the pwd generator is looping
   through, you can override the output by specifying it via pwdgen_found().  */

void pwdgen_found (
  struct ssPWD* sPWD, unsigned char * solution, unsigned int len)
  {
  unsigned int i;
  
  for (i = 0; i < len; i++)
    {
    sPWD->pwd[i] = solution[i];
    }
  sPWD->pwd[i] = 0;		/* zero terminate */
  sPWD->length = len;
  }

/* ************************************************************************ */
/* try to find a file from it's given name (in current dir and 1 or 2 dirs up)
   return codes: ptr to FILE* - okay, found it
                 NULL         - error */

FILE* pwdgen_find_file ( unsigned char* filename, unsigned char* options )
  {
  unsigned char f[270];
  FILE *file;

  strncpy (f, filename, 250);
  file = fopen (&f[0],options);
  if (NULL == file)
    {
    strcpy (f, "../");
    strncat (f, filename, 250);
    file = fopen (f,options);
    if (NULL == file)
      {
      strcpy (f, "../../");
      strncat (f, filename, 250);
      file = fopen (f,options);
      if (NULL == file)
        {
        strcpy (f, "..\\");
        strncat (f, filename, 250);
        file = fopen (f,options);
        if (NULL == file)
          {
          strcpy (f, "..\\..\\");
          strncat (f, filename, 250);
          file = fopen (f,options);
          if (NULL == file)
            {
            printf ("Error: Cannot find file '%s'.\n", filename);
            return NULL;
            }
          }
        }
      }
    }
  return file;
  }

