
/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

#ifndef PWDGEN
#define PWDGEN

#include "dicop.h"
#include "pwdcfg.h"

/*

  This struct is constructed upon initialization and returned to the caller 
  !!!! It should only be read, and never modified inside dofunction() !!!!

*/

#define PWD_LEN 256

struct ssPWD
  {
  /* the following fields are interesting for dofunction(): */
  unsigned char pwd[PWD_LEN];	/* current password, not necc. zero
				   terminated, see length */
  long length;			/* length of current password 
				   including prefix */
  long overflow;		/* how many chars overflowed
				   (e.g changed) from last password to current,
				   goes from 1..length of pwd */

  struct ssKey* config;			/* list of options from config file 
					   use pwdgen_find_key() to access it */
  unsigned char target[4*PWD_LEN];	/* copy of targetkey */
  long targetlen;			/* length of targetkey */

  /* for collecting blocks of passwords w/ padding */

  unsigned char *par_buff; 		/* current block of passwords */
  unsigned int	 par_cnt; 		/* how many are in buff? */
  unsigned int	 par_len; 		/* current len of one pwd in buff */

  /* these for are internal usage and probably not interesting to the caller */
  unsigned int crc;			/* CRC of current chunk */	
  unsigned char end[PWD_LEN];		/* last password */
  unsigned char start[PWD_LEN];		/* first password */
  long endlen;				/* length of last pwd */
  long startlen;			/* length of first pwd */
  struct ssCharset* set;		/* pointer to character set */
  unsigned int id;			/* id of character set */
  unsigned int type;			/* character set type */
  unsigned int timeout;			/* timeout in minutes */
  double pwds;				/* pwds done all in all */
  double pwds_done;			/* how many pwds since last check for
					   timeout */
  time_t tstart;			/* time of init */
  double took;				/* seconds from init() to stop() */
  unsigned int pwd_digit[PWD_LEN];	/* column of cur pwd */
  unsigned int end_digit[PWD_LEN];	/* column of end pwd */
  unsigned int once;			/* do only one password */

  unsigned char prefix[PWD_LEN];	/* fixed prefix prepended to pwd */
  unsigned long prefix_length;		/* and it's length */
 
  /* length of current generator pwd (length = prefix_length + gen_length) */
  unsigned long gen_length;

  /* for simple sets, these contain all the same table and length, for grouped
     sets they can differ */ 
  unsigned int *tables[PWD_LEN];	/* ptrs to char table for each column */
  unsigned int lens[PWD_LEN];		/* length of tables */


  /* for extract set with mutation/stages */
  unsigned long flags;

  /* for dictionary sets */
  unsigned int todo_mutations;		/* bit mask of mutations todo */
  unsigned int done_mutations;  	/* copy of todo, then set to 0 bit by
					   bit */
  unsigned int cur_mutation;  		/* the current mutation */
  
  unsigned int todo_stages;		/* bit mask of stages todo */
  unsigned int done_stages;	  	/* copy of todo, then set to 0 bit by
					   bit */
  unsigned int cur_stage;  		/* the current stage */

  unsigned char org_word[PWD_LEN];	/* orginal word from dictionary */
  unsigned char word[PWD_LEN];		/* word forward/reverse etc */
  unsigned long word_length;		/* length of current word */
  unsigned long org_word_length;	/* length of current org word */
  unsigned int last_word;		/* read in last word from dict */

  /* set by pwdgen_readset */
  unsigned char dictionary[PWD_LEN];	/* dict file name */
  FILE* dict_file;			/* file handle for dict file */
  unsigned long dict_file_ofs;		/* ofs of first word in dict file */
  
  /* for dictionary sets that append stuff */
  struct ssPWD* gen_pwd;		/* ptr to secondary generators */

  unsigned int append;			/* append/prepend stage */ 
  unsigned int prepend;			/* flag for first/other prepend */ 
  unsigned char word_2[PWD_LEN];	/* temp. word for prepending */
  struct ssCharset* charsets;		/* pointer to table w/ all charsets */

  /* for collecting blocks of passwords w/ padding */
  int par_padd; 			/* how to padd */
  unsigned char par_pchar; 		/* with what to padd */
  unsigned int  par_maxcnt; 		/* how many we can have in buff */

  } sPWD;

/* globals for a tad more speed */

long max_pwds;  /* how often to do timeout check, adjusted dynamically */
time_t now;     /* temp time variable */
time_t last;    /* last time we checked the time */

/* ************************************************************************* */
/* prototypes */

/* init the password generator */
extern struct ssPWD* pwdgen_init (
  unsigned char* start,
  unsigned char* end,
  unsigned char* target,
  unsigned char* set,
  unsigned int timeout,
  struct ssKey* keys
  );

/* find a charset by it's ID number or return NULL */
struct ssCharset* pwdgen_find_set (struct ssCharset* charset, unsigned long id);

/* re-init the password generator */
int pwdgen_reinit ( struct ssPWD* sPWD ); 

/* advance password by one, return 0 for okay, and 1 for past-end-pwd */
extern int pwdgen_next ( struct ssPWD* sPWD );

/* update the took field, and convert the password with a2h() */
extern void pwdgen_stop ( struct ssPWD* sPWD );

/* set up the tables (and init the new password) */
extern void pwdgen_init_tables ( struct ssPWD* sPWD, int type );

/* extract pwds from image, weed out doubles, call dofunction() on each */
int pwdgen_extract ( struct ssPWD* pwd );

/* *********************************************************************** */
/* PUBLIC functions */

/* set the password you want to have printed out as solution */ 
extern void pwdgen_found ( struct ssPWD* sPWD, unsigned  char * solution, 
			   unsigned int len );

/* try to open a file from it's given name (in current dir and one
   or two dirs up) */

extern FILE* pwdgen_find_file ( unsigned char* file, unsigned char* options );

/* did check more than one pwd, so modify counter */
extern void pwdgen_add ( struct ssPWD* sPWD, unsigned long pwds );

/* modify the crc of this chunk */
extern void pwdgen_update_crc ( struct ssPWD* sPWD, unsigned char *data,
				unsigned int len );

#endif
