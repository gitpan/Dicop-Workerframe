/*

  Dictionary reading, mutation of strings

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

  TODO: * sPWD.overflow could be set to some smaller values if only the last
	  character changed or something like this
	* timeout check is done after all the mutations/stages/append/prepend,
	  so if there are a lot per word, it might take a long while to timeout
	* Unicode support, especially uc/lc stuff
*/

#include "../include/pwdread.h"
#include "../include/pwddict.h"
#include "../include/dicop.h"

//#define DEBUG 1

char uc_vowels (char a)
  {
  if ((a >= 'a') && (a <= 'z'))
    {
    if (a == 'a') { return 'A'; }
    if (a == 'o') { return 'O'; }
    if (a == 'u') { return 'U'; }
    if (a == 'i') { return 'I'; }
    if (a == 'e') { return 'E'; }
    }
  else
    {
    if (a == '÷') { return '™'; }
    if (a == '„') { return 'Ž'; }
    if (a == '') { return 'š'; }
    }
  return a;
  }

char uc_consonants (char a)
  {
  if ((a == 'a') || (a == 'o') || (a == 'u') ||
   (a == 'i') || (a == 'e') ||
   (a == '') || (a == '÷') || (a == '„'))
    {
    return a;
    }
  if ((a >= 'a') && (a <= 'z'))
    {
    return a - ('a' - 'A');
    }
  return a;
  }

char uc (char a)
  {
  if ((a >= 'a') && (a <= 'z'))
    {
    return a - ('a' - 'A');
    }
  else
    {
    if (a == '÷') { return '™'; }
    if (a == '„') { return 'Ž'; }
    if (a == '') { return 'š'; }
    }
  return a;
  }

char lc (char a)
  {
  if ((a >= 'A') && (a <= 'Z'))
    {
    return a + ('a' - 'A');
    }
  else
    {
    if (a == '™') { return '÷'; }
    if (a == 'Ž') { return '„'; }
    if (a == 'š') { return 'š'; }
    }
  return a;
  }

/* ************************************************************************* */
/* mutate a basic word from the dictionary (org_word) into word
   until all mutations were done, then signal done and start all over
   This one mutates like forward, reverse etc. Each of these mutations stages
   is then mutated further with lowercase, uppercase etc
*/

void pwddict_stage ( struct ssPWD* sPWD )
  {
  unsigned int wl,i;

#ifdef DEBUG
  printf ("\n  current stage %i\n",sPWD->cur_stage);
  printf ("  done stages %i\n",sPWD->done_stages);
  printf ("  todo stages %i\n",sPWD->todo_stages);
#endif

  /* loop from the cur stages through all the possible ones */
  while ( ((sPWD->done_stages) & (sPWD->cur_stage)) == 0)
    {
    sPWD->cur_stage <<= 1;
    }

  /* big ole case statement to cover all the cases */

  wl = sPWD->org_word_length;
  switch (sPWD->cur_stage) {
    case DICT_FORWARD:
      {
      for (i = 0; i < wl; i++)
        {
        sPWD->word[i] = sPWD->org_word[i];
        }
      sPWD->word_length = wl;
      break;
      }
    case DICT_REVERSE:
      {
      for (i = 0; i < wl; i++)
        {
        sPWD->word[i] = sPWD->org_word[wl - i - 1];
        }
      sPWD->word_length = wl;
      break;
      }
    }
  /* zero terminate */
  sPWD->word[sPWD->word_length] = 0;
  
  /* clear the currently done stage from the bitmask */
  sPWD->done_stages &= ~ sPWD->cur_stage;

  }
  
int pwddict_trynext (struct ssPWD* pwd)
  {
  int rc;

  /* did all mutations on current word? */
  if (pwd->done_mutations == 0)
    {
    pwd->done_mutations = pwd->todo_mutations;
    pwd->cur_mutation = 1;
    /* looped through all the mutation stages (like forward, reverse etc) */

    /* if all stages done, so let's read us the next word first */

    if (pwd->done_stages == 0)
      {
      if ((rc = pwddict_nextword ( pwd )) != 0)
        {
        /* no more words or timeout */
#ifdef DEBUG
	printf ("no more words or timeout\n");
#endif
        return rc;
        }
      /* got one word, so do the first stage mutation again */
      pwd->done_stages = pwd->todo_stages;
      pwd->cur_stage = 1;	/* reset current stage */
      pwd->append = 0;		/* no append/prepend yet */
      pwd->prepend = 0;		/* first prepend, if ever */
      }
    pwddict_stage ( pwd );	/* cover the first stage of new word */
    }
  /* have still to work */
  return 0;
  }

/* ************************************************************************* */
/* mutate a basic word from the dictionary into pwd until all
   mutations were done, then get next word and start mutation series
   all over. This mutates lower/upper case combination. 
   return codes: 0 - okay, next pwd
                 1 - overshot, last pwd checked, stop now
                 4 - timeout, stopped in midst
*/

unsigned int pwddict_next ( struct ssPWD* sPWD )
  {
  unsigned int i;
  unsigned int pl,last;
  struct ssPWD* gen;

#ifdef DEBUG
  printf ("\n  current mutation %i\n",sPWD->cur_mutation);
  printf ("  done mutation %i\n",sPWD->done_mutations);
  printf ("  todo mutation %i\n",sPWD->todo_mutations);
#endif
  
  if (sPWD->append != 0)		/* currently appending/prepending? */
    {
    gen = &sPWD->gen_pwd[sPWD->append-1];

#ifdef DEBUG
      printf ("Still in append\n");
#endif

    if (sPWD->set->pos[sPWD->append-1] == 0)	/* 0 - append, 1 prepend */
      {
#ifdef DEBUG
	printf ("appending\n");
#endif
      /* append password to "$prefix$word" */
      pl = sPWD->prefix_length + sPWD->word_length;
      /* TODO: copy over only the changed characters (check overflow) */
      for (i = 0; i < gen->gen_length; i++)
        {
        sPWD->pwd[i + pl] = gen->pwd[i];
        }
      sPWD->pwd[i+pl] = 0;		/* zero terminate */
      }
    else
      {
#ifdef DEBUG
	printf ("prepending %i\n",sPWD->prepend);
#endif
      /* for the very first prepended pwd, move the word out of the way */
      pl = sPWD->prefix_length;
      if (sPWD->prepend == 0)
        {
        for (i = 0; i < sPWD->word_length; i++)
          {
          sPWD->word_2[i] = sPWD->pwd[i+pl];
          }
        sPWD->word_2[i] = 0;
        sPWD->prepend++;	/* do the move only once */
        }
      /* TODO: copy over only the changed characters (check overflow) */
      /* create "$prefix$generatedword"; */
      for (i = 0; i < gen->gen_length; i++)
        {
        sPWD->pwd[i + pl] = gen->pwd[i];
        }
      pl = i;
      /* create "$prefix$generatedword$word"; */
      for (i = 0; i < sPWD->word_length; i++)
        {
        sPWD->pwd[i + pl] = sPWD->word_2[i];
        }
      sPWD->pwd[i+pl] = 0;		/* zero terminate */
      }
    sPWD->length = sPWD->prefix_length + sPWD->word_length + gen->gen_length;

#ifdef DEBUG
    printf ("Next pwd (%li)\n", sPWD->pwds_done);
#endif

    /* generate next password */
    last = pwdgen_next(gen);

#ifdef DEBUG
    printf ("last %i\n",last);
#endif

    if (last != 0)
      {
      /* hit last pwd, so increment append (or reset it and get next
         mutation) */
      sPWD->prepend = 0;			/* restart prepending */
      sPWD->append++;
      if (sPWD->append > sPWD->set->paircnt)	/* 2 pairs, got from 1..2 */
        {
#ifdef DEBUG
        printf ("Already done all append stages\n");
#endif
        sPWD->append = 0;			/* next time stage/mutation */
        /* generator #0 will be reset below next round */
    
	/* get next stage/mutation or word */
        /* the next check is duplicated here from below (see lower check). The
           reason is that when doing only muatations, we need the lower check,
	   but when doing append, we also need the upper check. Currently I
	   see now way around this, otherwise we either duplicate one password
	   or miss one. */
	if (sPWD->done_mutations == 0)
	  {
          return pwddict_trynext (sPWD);
	  }
	return 0;
        }
      else
        {
#ifdef DEBUG
        printf ("Next append stage\n");
#endif
        /* re-init the particular pwd gen for next append/prepend */
        pwdgen_reinit ( &sPWD->gen_pwd[sPWD->append-1] );
        }
      }
    return 0;
    }

  pl = sPWD->prefix_length;
  sPWD->length = sPWD->word_length + sPWD->prefix_length;
 
#ifdef DEBUG
  printf ( "after append\n");
  printf ("  current mutation %i\n",sPWD->cur_mutation);
  printf ("  done mutation %i\n",sPWD->done_mutations);
  printf ("  prefix length %i\n",pl);
  printf ("  word: '%s'\n",sPWD->word);
#endif

  if (sPWD->cur_mutation == 1)
    {
    /* last mutation was done, we reset and read in a new word, so we now must
       set the length */
    sPWD->length = sPWD->word_length + pl;
    }

  /* loop from the cur mutation through all the possible ones */
  while ( ((sPWD->done_mutations) & (sPWD->cur_mutation)) == 0)
    {
    sPWD->cur_mutation <<= 1;
    }

  /* now we have in cur_mutation the one we need to do next */

#ifdef DEBUG
  printf ("  current mutation %i\n",sPWD->cur_mutation);
  printf ("  prefix length %i\n",pl);
  printf ("  word: '%s'\n",sPWD->word);
#endif
  
  /* big ole case statement to cover all the cases */

  switch (sPWD->cur_mutation) {
    case DICT_LOWER:
    {
#ifdef DEBUG
    printf ("DICT_LOWER\n");
#endif
    for (i = 0; i < sPWD->word_length; i++)
      {
      sPWD->pwd[pl+i] = lc(sPWD->word[i]);
      }
    break;
    }
    case DICT_UPPER:
    {
    for (i = 0; i < sPWD->word_length; i++)
      {
      sPWD->pwd[pl+i] = uc(sPWD->word[i]);
      }
    break;
    }
    case DICT_UPPER_FIRST:
    {
    sPWD->pwd[pl] = uc(sPWD->word[0]);
    for (i = 1; i < sPWD->word_length; i++)
      {
      sPWD->pwd[pl+i] = lc(sPWD->word[i]);
      }
    break;
    }
    case DICT_LOWER_FIRST:
    {
    sPWD->pwd[pl] = lc(sPWD->word[0]);
    for (i = 1; i < sPWD->word_length; i++)
      {
      sPWD->pwd[pl+i] = uc(sPWD->word[i]);
      }
    break;
    }
    case DICT_UPPER_ODD:
    {
    /* might touch the final 0x00 */
    for (i = 0; i < sPWD->word_length;i ++)
      {
      sPWD->pwd[pl+i] = lc(sPWD->word[i]); i++;
      sPWD->pwd[pl+i] = uc(sPWD->word[i]);
      }
    break;
    }
    case DICT_UPPER_EVEN:
    {
    /* might touch the final 0x00 */
    for (i = 0; i < sPWD->word_length;i ++)
      {
      sPWD->pwd[pl+i] = uc(sPWD->word[i]); i++;
      sPWD->pwd[pl+i] = lc(sPWD->word[i]);
      }
    break;
    }
    case DICT_UPPER_LAST:
    {
    int j = sPWD->word_length - 1;
    for (i = 0; i < j; i++)
      {
      sPWD->pwd[pl+i] = lc(sPWD->word[i]);
      }
    sPWD->pwd[pl+j] = uc(sPWD->word[j]);
    break;
    }
    case DICT_LOWER_LAST:
    {
    int j = sPWD->word_length - 1;
    for (i = 0; i < j; i++)
      {
      sPWD->pwd[pl+i] = uc(sPWD->word[i]);
      }
    sPWD->pwd[pl+j] = lc(sPWD->word[j]);
    break;
    }
    case DICT_UPPER_VOWELS:
    {
    int j = sPWD->word_length;
    for (i = 0; i < j; i++)
      {
      sPWD->pwd[pl+i] = uc_vowels(sPWD->word[i]);
      }
    break;
    }
    case DICT_UPPER_CONSONANTS:
    {
    int j = sPWD->word_length;
    for (i = 0; i < j; i++)
      {
      sPWD->pwd[pl+i] = uc_consonants(sPWD->word[i]);
      }
    break;
    }
  }

  /* zero terminate */
  sPWD->pwd[pl+sPWD->word_length] = 0;

  /* clear the currently done mutation from the bitmask */
  sPWD->done_mutations &= ~ sPWD->cur_mutation;

#ifdef DEBUG
  printf (" append: %i\n",sPWD->append);
#endif

  /* if we have a non-zero count of things to prepend/append, do so */
  if (sPWD->set->paircnt != 0)
    {
#ifdef DEBUG
    printf (" Have append/prepend\n");
#endif

    /* append must be 0 here, because append != is handled above */
    sPWD->append++;			/* next time append/prepend next set */
    /* re-init first pwd generator for appending/prepending */
#ifdef DEBUG
    printf (" first %p (cur %p)\n ", &sPWD->gen_pwd[0], sPWD );
#endif
    pwdgen_reinit( &sPWD->gen_pwd[0] );
    /* now we have set anything up for prepend/append next round */
    return 0;
    }

  if (sPWD->done_mutations == 0)
    {
    return pwddict_trynext(sPWD);
    }
  return 0;
 
  }

/* ************************************************************************* */
/* INTERNAL: read the next word from the dictionary file
	     used by fast_forward
   returns 0 for ok, and 1 for all words done, or PWD_ERROR
   */

unsigned int pwddict_nextword ( struct ssPWD* sPWD )
  {
  long timediff;
  unsigned int wl;

  /* check timeout and calculate number of pwds done */
  if (sPWD->pwds_done > max_pwds)
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
        /* correct our start value by the amount the clock warped */
        sPWD->tstart -= timediff;
        }
      last = now;       /* save for next check */
      sPWD->took = (long) difftime(now,sPWD->tstart);
      /* if zero seconds have elapsed, at least double the count til next check */
      if (sPWD->took == 0)
        {
        max_pwds *= 2;
        }
      if (sPWD->took > sPWD->timeout) { return PWD_TIMEOUT; }	/* timeout, break */
      }
    }

  /* if caller doesn't want us to read next word (driven by extract code) */
  if ((sPWD->flags & 1) != 0)
    {
    /* still work to do, e.g. no timeout */
    return PWD_ERROR;
    }

  if (feof(sPWD->dict_file) || sPWD->last_word != 0)
    {
#ifdef DEBUG
    printf ("last_word %i, hit end\n",sPWD->last_word);
#endif
    return 1;
    }

  /* read one line and get it's length */ 
  fgets(&sPWD->org_word[0], 254, sPWD->dict_file);
  sPWD->org_word_length = strlen(sPWD->org_word);
  if (sPWD->org_word_length == 0)
    {
    /* hm, no eof, but can't read any data - disk on fire? */
    return 1;
    }

  wl = sPWD->org_word_length - 1;	/* set to last char */

  /* kill any new line chars (0xa,0xd,0x0a0d,0x0d0a etc) */
  if ((sPWD->org_word[wl] == 0x0a) || 
      (sPWD->org_word[wl] == 0x0d))
    {
    sPWD->org_word[wl] = 0;
    wl --;
    if ((sPWD->org_word[wl] == 0x0a) || 
        (sPWD->org_word[wl] == 0x0d))
      {
      sPWD->org_word[wl] = 0;
      wl --;
      }
    }
  sPWD->org_word_length = wl + 1;	/* real length again */
  /* overflow goes from 1 to length */
  sPWD->overflow = sPWD->org_word_length;

  if (sPWD->org_word_length == sPWD->endlen)
    {
    /* reached last word? */
    if (strcmp(sPWD->org_word,sPWD->end) == 0)
      {
      /* just set a flag, since we need to do all the mutations, too */
      sPWD->last_word = 1;
#ifdef DEBUG
      printf ("reached last word %s\n",sPWD->org_word);
#endif
      }
    }
  return 0;
  }

/* ************************************************************************* */
/* INTERNAL: open the dictionary file and keep the file handle
   returns 0 for ok, and != 0 for error
   */

unsigned int pwddict_openfile ( struct ssPWD* sPWD )
  {
  char filename[256];

  /* assume we are in the worker/architecture dir and need to go up twice */
  strcpy(filename,"target/dictionaries/");
  strncat(filename,sPWD->dictionary,200);
  printf ("\n Opening dictionary file '%s'... ",filename);
  if (NULL == (sPWD->dict_file = pwdgen_find_file (filename, "r")))
    {
    return 1;
    }
  printf ("done.");
  return 0;
  }

/* ************************************************************************* */
/* INTERNAL: read lines from the dictionary file until the line matches the
	     wanted start pwd
	     This really could benefit from getting the offset of the first
	     word in the file.
   returns 0 for ok, and != 0 for error (not found, due to file end reached)
   */

unsigned int pwddict_fastforward ( struct ssPWD* sPWD )
  {
  unsigned int rc;
  unsigned int first_word = 0;

  /* if file offset > 0, we skip ahead and denote that the next read word
     is not the first one (e.g. we need to skip one more afterwards */
  if (0 != sPWD->dict_file_ofs)
    {
#ifdef DEBUG
    printf (" Using offset %li\n", sPWD->dict_file_ofs);
#endif
    fseek (sPWD->dict_file, sPWD->dict_file_ofs, SEEK_SET);
    /* set to 1, will be incr. next and denote skip */
    first_word = 1;
    }
  /* continue normally, the next read word will be the right one */

  /* fast forward in dictionary file */
  do {
    rc = pwddict_nextword ( sPWD );
    if (rc != 0)
      {
      /* file on fire */
      printf ("\nError: Cannot find start word in dictionary.\n");
      return 1;
      }
    rc = 1;
    if (sPWD->org_word_length == sPWD->startlen)
      {
      /* found the start password? */
      /* rc == < 0, 0, > 0 */
      rc = strcmp(sPWD->org_word,&sPWD->pwd[sPWD->prefix_length]);
      }
    first_word++;
    } while ( rc != 0 );

  /* if we did not read in the first word, but second or later word, then we
     need to skip one word more. The reason is that if we have:
	foo     <--
	bar        first chunk
	baz     <--
	fubar      second chunk
	fool    <--
     Then in the second chunk we need to skip 'baz', since it was already done
     by the client going from 'foo' to 'baz'. But we don't want to skip the
     very first word ('foo'), not any of the last or the very last word. This
     basically ensures that each word is done exactly once as the clients go
     through the list of words.  */
  if (first_word > 1)
    {
    rc = pwddict_nextword ( sPWD );
    }
#ifdef DEBUG
  printf (" dict start word: '%s'\n", sPWD->org_word);
  printf (" done_stages %i done_mutations %i\n", sPWD->done_stages, sPWD->done_mutations);
  printf (" todo_stages %i todo_mutations %i\n", sPWD->todo_stages, sPWD->todo_mutations);
#endif
  /* and cover the first stage */
  pwddict_stage( sPWD );

  return rc;
  }

