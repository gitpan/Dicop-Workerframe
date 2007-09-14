/*!
 * @file
 * @ingroup workerframe
 * @brief Dictionary reading, mutation of strings.
 * 
 * @copydoc copyrighttext
*/

#include <dicop.h>
#include <pwdread.h>
#include <pwddict.h>

/* ************************************************************************* */
/* mutate a basic word from the dictionary (org_word) into word
   until all mutations are done, then signal done and start all over
   This one mutates like forward, reverse etc. Each of these mutations stages
   is then mutated further with lowercase, uppercase etc
*/

void
pwddict_stage (struct ssPWD *sPWD)
  {
  struct sPwdString *word;

#ifdef DEBUG
  printf ("\n  current stage %i\n", sPWD->cur_stage);
  printf ("  done stages %i\n", sPWD->done_stages);
  printf ("  todo stages %i\n", sPWD->todo_stages);
#endif

  /* loop from the cur stages through all the possible ones */
  while (((sPWD->done_stages) & (sPWD->cur_stage)) == 0)
    {
      sPWD->cur_stage <<= 1;
    }

  /* word is read-only */
  word = (struct sPwdString *) sPWD->word;
  /* copy over the original word */
  pwdgen_strdup(sPWD, word, sPWD->org_word);

  if (sPWD->cur_stage == DICT_REVERSE)
    {
    pwdgen_strrev(sPWD, word);
    }

#ifdef DEBUG
  printf ("\nCovered stage %i\n", sPWD->cur_stage);
  hexdump_string(sPWD, sPWD->word);
  hexdump("pwd:", sPWD->pwd, sPWD->length);
#endif

  /* clear the currently done stage from the bitmask */
  sPWD->done_stages &= ~sPWD->cur_stage;
  }

int
pwddict_trynext (struct ssPWD *pwd)
{
  int rc;

  /* did all mutations on current word? */
  if (pwd->done_mutations == 0)
    {
      pwd->done_mutations = pwd->todo_mutations;
      pwd->cur_mutation = pwd->first_mutation;
      /* looped through all the mutation stages (like forward, reverse etc) */

      /* if all stages done, so let read us the next word first */
      if (pwd->done_stages == 0)
	{
	  if ((rc = pwddict_nextword (pwd)) != 0)
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
	  pwd->append = 0;	/* no append/prepend yet */
	  pwd->prepend = 0;	/* first prepend, if ever */
	}
      pwddict_stage (pwd);	/* cover the first stage of new word */
    }
  if (pwd->pwd != pwd->cur->content)
    {
#ifdef DEBUG
    printf ("Producing first encoding: %s\n", pwdgen_encoding(pwd->encoding[0]));
#endif
    pwdgen_update_cur(pwd, 1);
    }
  /* still have work to do */
  return 0;
}

/* ************************************************************************* */
/* mutate a basic word from the dictionary into pwd until all
   mutations were done, then get next word and start mutation series
   all over. This mutates lowercase/uppercase etc. combinations. 
   return codes: 0 - okay, next pwd
                 1 - overshot, last pwd checked, stop now
                 4 - timeout, stopped in midst
*/

unsigned int
pwddict_next (struct ssPWD *sPWD)
{
  unsigned int i;
  unsigned int pl, last;
  struct ssPWD *gen;
  struct sPwdString *cur;

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
    printf ("Produced last encoding, reset (%lli) (%lli)\n", sPWD->cur_encoding, sPWD->encodings);
#endif

    /* if start pwd == end pwd, we need to stop after we reached all the
       encodings, so setup things to terminates the loop in main.c */
    if ( (0 != sPWD->cur_encoding) && (0 != sPWD->_last_pwd) )
      {
      return 1;
      }
    /* continue normally (with the first encoding, either because it is
       already 0, or we hit the last, resetting it to 0 again) */
    sPWD->cur_encoding = 0;
    }

#ifdef DEBUG
  printf ("\n  current mutation %i\n", sPWD->cur_mutation);
  printf ("  done mutation %i\n", sPWD->done_mutations);
  printf ("  todo mutation %i\n", sPWD->todo_mutations);
#endif

  if (sPWD->append != 0)	/* currently appending/prepending? */
    {
      gen = &sPWD->gen_pwd[sPWD->append - 1];

#ifdef DEBUG
      printf ("Still in append/prepend\n");
#endif

      if (sPWD->set->pos[sPWD->append - 1] == 0)	/* 0 - append, 1 prepend */
	{
#ifdef DEBUG
	  printf ("appending\n");
#endif
	  /* append password to "$word" */
	  pl = sPWD->word->bytes;
	  /* TODO: copy over only the changed characters (check overflow) */
	  for (i = 0; i < gen->gen_length; i++)
	    {
	      sPWD->pwd[i + pl] = gen->pwd[i];
	    }
	  sPWD->pwd[i + pl] = 0;	/* zero terminate */
	}
      else
	{
#ifdef DEBUG
	  printf ("prepending %i\n", sPWD->prepend);
#endif
	  /* for the very first prepended pwd, make a copy of the current word */
	  if (sPWD->prepend == 0)
	    {
	      for (i = 0; i < sPWD->word->bytes; i++)
		{
		  sPWD->word_2[i] = sPWD->pwd[i];
		}
	      sPWD->word_2[i] = 0;
	      sPWD->prepend++;	/* do the move only once */
	    }
	  /* TODO: copy over only the changed characters (check overflow) */
	  /* create "$generatedword"; */
	  for (i = 0; i < gen->gen_length; i++)
	    {
	      sPWD->pwd[i] = gen->pwd[i];
	    }
	  pl = i;
	  /* create "$generatedword$word"; */
	  for (i = 0; i < sPWD->word->bytes; i++)
	    {
	      sPWD->pwd[i + pl] = sPWD->word_2[i];
	    }
	  sPWD->pwd[i + pl] = 0;	/* zero terminate */
	}
      /* calculate the new current password length (not including the prefix) */
      sPWD->length = sPWD->word->bytes + gen->gen_length;
      /* for case where we do not convert pwd=>cur->content, set the length, too */
      /* cur is read-only: */
      cur = (struct sPwdString *) sPWD->cur;
      cur->bytes = sPWD->length;
      cur->characters = -1;
      cur->encoding = sPWD->generator_encoding;

#ifdef DEBUG
      printf ("Next pwd (%li)\n", (long int)sPWD->pwds_done);
      printf ("curlen %i wordlen %i genlen %i\n", sPWD->length, sPWD->word->bytes, gen->gen_length);
#endif

      /* generate next password */
      last = pwdgen_next (gen);

#ifdef DEBUG
      printf ("last %i\n", last);
#endif

      if (last != 0)
	{
	  /* hit last pwd, so increment append (or reset it and get next
	     mutation) */
	  sPWD->prepend = 0;	/* restart prepending */
	  sPWD->append++;
	  if (sPWD->append > sPWD->set->paircnt)	/* 2 pairs, got from 1..2 */
	    {
#ifdef DEBUG
	      printf ("Already done all append stages\n");
#endif
	      sPWD->append = 0;	/* next time stage/mutation */
	      /* generator #0 will be reset below next round */

	      /* get next stage/mutation or word */
	      /* the next check is duplicated here from below (see lower check). The
	         reason is that when doing only mutations, we need the lower check,
	         but when doing append, we also need the upper check. Currently I
	         see now way around this, otherwise we either duplicate one password
	         or miss one. */
	      return pwddict_trynext (sPWD);
	    }
	  else
	    {
#ifdef DEBUG
	      printf ("Next append stage\n");
#endif
	      /* re-init the particular pwd gen for next append/prepend */
	      pwdgen_reinit (&sPWD->gen_pwd[sPWD->append - 1]);
	    }
	}
      if (sPWD->pwd != sPWD->cur->content)
        {
#ifdef DEBUG
        printf ("Producing first encoding: %s\n", pwdgen_encoding(sPWD->encoding[0]));
#endif
        pwdgen_update_cur(sPWD, 1);
        }
      return 0;
    }

  pl = sPWD->prefix->bytes;
  sPWD->length = sPWD->word->bytes;
  /* cur is read-only: */
  cur = (struct sPwdString *) sPWD->cur;

  /* copy in the word */
  pwdgen_strdup(sPWD, cur, sPWD->word);

#ifdef DEBUG
  printf ("after append\n");
  printf ("  current mutation %i\n", sPWD->cur_mutation);
  printf ("  done mutation %i\n", sPWD->done_mutations);
  printf ("  prefix length %i\n", pl);
  pwdgen_print (sPWD, "  word: '%s'\n", sPWD->word);
#endif

  /* loop from the cur mutation through all the possible ones */
  while (((sPWD->done_mutations) & (sPWD->cur_mutation)) == 0)
    {
    sPWD->cur_mutation <<= 1;
    }

  /* now we have in cur_mutation the one we need to do next */

#ifdef DEBUG
  printf ("  current mutation %i\n", sPWD->cur_mutation);
#endif

  /* big ole case statement to cover all the cases */

  switch (sPWD->cur_mutation)
    {
    case DICT_ORIGINAL:
      {
	/* nothing to do */
	break;
      }
    case DICT_LOWER:
      {
	pwdgen_lc(sPWD, cur);
	break;
      }
    case DICT_UPPER:
      {
	pwdgen_uc(sPWD, cur);
	break;
      }
    case DICT_UPPER_FIRST:
      {
	pwdgen_uc_first(sPWD, cur);
	break;
      }
    case DICT_LOWER_FIRST:
      {
	pwdgen_lc_first(sPWD, cur);
	break;
      }
    case DICT_UPPER_ODD:
      {
	pwdgen_uc_odd(sPWD, cur);
	break;
      }
    case DICT_UPPER_EVEN:
      {
	pwdgen_uc_even(sPWD, cur);
	break;
      }
    case DICT_UPPER_LAST:
      {
	pwdgen_uc_last(sPWD, cur);
	break;
      }
    case DICT_LOWER_LAST:
      {
	pwdgen_lc_last(sPWD, cur);
	break;
      }
    case DICT_UPPER_VOWELS:
      {
	pwdgen_uc_vowels(sPWD, cur);
	break;
      }
    case DICT_UPPER_CONSONANTS:
      {
	pwdgen_uc_consonants(sPWD, cur);
	break;
      }
    }

  /* now move the word into the pwd var */
  if (sPWD->pwd != sPWD->cur->content)
    {
    memcpy (sPWD->pwd, cur->content, cur->bytes);
    }

  /* do we need to update cur, too? */
  if ((pl != 0) || (sPWD->pwd != sPWD->cur->content))
    {
    pwdgen_update_cur(sPWD, 1);
    sPWD->cur_encoding = 0;
    }

#ifdef DEBUG
  hexdump_string ("cur", sPWD->cur);
  printf ("update_cur: len: %i",sPWD->length);
#endif
 
  /* clear the currently done mutation from the bitmask */
  sPWD->done_mutations &= ~sPWD->cur_mutation;

#ifdef DEBUG
  printf (" append: %i\n", sPWD->append);
#endif

  /* if we have a non-zero count of things to prepend/append, do so */
  if (sPWD->set->paircnt != 0)
    {
#ifdef DEBUG
      printf (" Have append/prepend\n");
#endif

      /* append must be 0 here, because append != is handled above */
      sPWD->append++;		/* next time append/prepend next set */
      /* re-init first pwd generator for appending/prepending */
#ifdef DEBUG
      printf (" first %p (cur %p)\n ", &sPWD->gen_pwd[0], sPWD);
#endif
      pwdgen_reinit (sPWD->gen_pwd);
      /* now we have set anything up for prepend/append next round */
      if (sPWD->pwd != sPWD->cur->content)
        {
#ifdef DEBUG
        printf ("Producing first encoding: %s\n", pwdgen_encoding(sPWD->encoding[0]));
#endif
        pwdgen_update_cur(sPWD, 1);
        }
      return 0;
    }

  return pwddict_trynext (sPWD);
}

/* ************************************************************************* */
/**
   INTERNAL:<br>
	read the next word from the dictionary file, used by
	pwdgen_fastforward()<br>
   returns:<br>
	- 0 for ok,
	- 1 for all words done,
	- PWD_TIMEOUT for timeout
	- PWD_ERROR for error
   */

unsigned int
pwddict_nextword (struct ssPWD *sPWD)
{
  unsigned int wl;
  struct sPwdString* org_word;

  /* check timeout and calculate number of pwds done */
  if (sPWD->pwds_done > sPWD->max_pwds)
    {
    sPWD->pwds += sPWD->pwds_done;	/* add them up */
    sPWD->pwds_done = 0;

    /* status report or timeout check? */
    if (pwdgen_time_check(sPWD) != 0)
      {
      return PWD_TIMEOUT;
      }
    }

  /* if caller doesn't want us to read next word (driven by extract code) */
  if ((sPWD->flags & 1) != 0)
    {
      /* still work to do, e.g. no timeout */
      return PWD_ERROR;
    }

  if (feof (sPWD->dict_file) || sPWD->last_word != 0)
    {
#ifdef DEBUG
      printf ("last_word %i, hit end\n", sPWD->last_word);
#endif
      return 1;
    }

  /* org_word is read-only: */
  org_word = (struct sPwdString*)sPWD->org_word;

  /* read one line and get its length */
  fgets ((char *)org_word->content, 254, sPWD->dict_file);
  /* zero terminate just in case */
  org_word->content[254] = 0;
  org_word->bytes = strlen (org_word->content);
  org_word->characters = -1;
  org_word->encoding = sPWD->generator_encoding;

  if (org_word->bytes == 0)
    {
      /* hm, no eof, but can't read any data - disk on fire? */
      return 1;
    }

  wl = org_word->bytes - 1;		/* set to last char */

  /* kill any new line chars (0xa,0xd,0x0a0d,0x0d0a etc) */
  /* XXX TODO this doesn't work if the file is in UTF-16 etc. */
  if ((org_word->content[wl] == 0x0a) || (org_word->content[wl] == 0x0d))
    {
      org_word->content[wl] = 0;
      wl--;
      if ((org_word->content[wl] == 0x0a) || (org_word->content[wl] == 0x0d))
	{
	  org_word->content[wl] = 0;
	  wl--;
	}
    }
  org_word->bytes = wl + 1;	/* real length again */
  org_word->characters = -1;
  org_word->encoding = sPWD->generator_encoding;

  /* overflow goes from 1 to length */
  sPWD->overflow = org_word->bytes;

  /* if the end password is set to "", then we will just go until we hit
     the end of the dictionary file */
  if (sPWD->org_word->bytes == sPWD->endlen)
    {
      /* reached last word? */
      if (strcmp (sPWD->org_word->content, sPWD->end) == 0)
	{
	  /* just set a flag, since we need to do all the mutations, too */
	  sPWD->last_word = 1;
#ifdef DEBUG
	  printf ("reached last word %s\n", sPWD->org_word);
#endif
	}
    }
  return 0;
}

/* ************************************************************************* */
/* INTERNAL: open the dictionary file and keep the file handle
   returns 0 for ok, and != 0 for error
   */

unsigned int
pwddict_openfile (struct ssPWD *sPWD)
{
  char filename[256];

  /* assume we are in the worker/architecture dir and need to go up twice */
  strcpy (filename, "target/dictionaries/");
  strncat (filename, sPWD->dictionary, 200);
  printf ("\n Opening dictionary file '%s'... ", filename);
  if (NULL == (sPWD->dict_file = pwdgen_find_file (filename, "r")))
    {
      return 1;
    }
  printf ("done.");
  return 0;
}

/* ************************************************************************* */
/* INTERNAL: read lines from the dictionary file until the line matches the
	     wanted start pwd.\n
   returns 0 for ok, and != 0 for error (word not found, eof etc.)
   */

unsigned int
pwddict_fastforward (struct ssPWD *sPWD)
{
  unsigned int rc;
  unsigned int first_word = 0;
  struct ssPWD *pwd = sPWD;

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
  do
    {
      rc = pwddict_nextword (sPWD);
      if (rc != 0)
	{
	  /* file on fire */
	  printf ("\nError: Cannot find start word in dictionary.\n");
	  return 1;
	}
      /* if pwd->start eq '', then we do the dictionary from the very start */
      if (pwd->startlen == 0)
        {
        break;
        }
      rc = 1;
      if (sPWD->org_word->bytes == sPWD->startlen)
	{
	  /* found the start password? */
	  /* rc == < 0, 0, > 0 */
	  rc = strcmp (sPWD->org_word->content, sPWD->pwd);
	}
      first_word++;
    }
  while (rc != 0);

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
      rc = pwddict_nextword (sPWD);
    }
#ifdef DEBUG
  printf (" dict start word: '%s'\n", sPWD->org_word);
  printf (" done_stages %i done_mutations %i\n", sPWD->done_stages,
	  sPWD->done_mutations);
  printf (" todo_stages %i todo_mutations %i\n", sPWD->todo_stages,
	  sPWD->todo_mutations);
#endif
  /* and cover the first stage */
  pwddict_stage (sPWD);

  return rc;
}

