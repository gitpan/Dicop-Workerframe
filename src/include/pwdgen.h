/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for the password generator of Dicop
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDGEN_H
#define DICOP_PWDGEN_H

#include <pwdcfg.h>
#include <time.h>

/** Maximum length of a password in bytes. */

#define PWD_LEN 256

/**
  @brief This struct is the "memory" of the password generator.

  It is constructed upon initialization and returned to the caller. 
  <b>It should only be read, and never modified inside dofunction()!</b>
*/

struct ssPWD
  {
  /* the following fields are interesting for dofunction(): */
  char pwd[PWD_LEN];			/**< current password, not necc. zero
					     terminated, see length */
  unsigned long length;			/**< length of current password 
					     including prefix */
  unsigned long overflow;		/**< how many chars overflowed
					     (e.g changed) from last password to current,
					     goes from 1..length of pwd */

  struct ssKey* config;			/**< list of options from config file 
					     use pwdgen_find_key() to access it */
  char target[4*PWD_LEN];		/**< copy of targetkey */
  unsigned long targetlen;		/**< length of targetkey */

  /** for collecting blocks of passwords w/ padding */

  char *par_buff; 			/**< current block of passwords */
  unsigned int	 par_cnt; 		/**< how many are in buff? */
  unsigned int	 par_len; 		/**< current len of one pwd in buff */

  /** these for are internal usage and probably not interesting to the caller */

  unsigned int crc;			/**< CRC of current chunk */	
  char end[PWD_LEN];			/**< last password */
  char start[PWD_LEN];			/**< first password */
  unsigned long endlen;			/**< length of last pwd */
  unsigned long startlen;		/**< length of first pwd */
  struct ssCharset* set;		/**< pointer to character set */
  unsigned int id;			/**< id of character set */
  unsigned int type;			/**< character set type */
  unsigned int timeout;			/**< timeout in minutes */
  unsigned int report_status;		/**< report every X seconds your status */
  double pwds;				/**< pwds done all in all */
  double pwds_done;			/**< how many pwds since last check for
					     timeout */
  double took;				/**< seconds from init() to stop() */
  time_t tstart;			/**< time of init */
  time_t tlast;				/**< last time we checked the timeout */
  time_t tlast_report;			/**< last time we reported our status */
  unsigned long max_pwds;		/**< how often to do timeout check, adjusted dynamically */
  unsigned int pwd_digit[PWD_LEN];	/**< column of cur pwd */
  unsigned int end_digit[PWD_LEN];	/**< column of end pwd */
  unsigned int once;			/**< do only one password */

  char prefix[PWD_LEN];			/**< fixed prefix prepended to pwd */
  unsigned long prefix_length;		/**< and it's length */
 
  /** length of current generator pwd (length = prefix_length + gen_length) */
  unsigned long gen_length;

  /** for simple sets, these contain all the same table and length, for grouped
     sets they can differ */ 
  unsigned int *tables[PWD_LEN];	/**< ptrs to char table for each column */
  unsigned int lens[PWD_LEN];		/**< length of tables */


  /** for extract set with mutation/stages */
  unsigned long flags;

  /** for dictionary sets */
  unsigned int todo_mutations;		/**< bit mask of mutations todo */
  unsigned int done_mutations;  	/**< copy of todo, then set to 0 bit by
					     bit */
  unsigned int cur_mutation;  		/**< the current mutation */
  
  unsigned int todo_stages;		/**< bit mask of stages todo */
  unsigned int done_stages;	  	/**< copy of todo, then set to 0 bit by
					     bit */
  unsigned int cur_stage;  		/**< the current stage */

  char org_word[PWD_LEN];		/**< orginal word from dictionary */
  char word[PWD_LEN];			/**< word forward/reverse etc */
  unsigned long word_length;		/**< length of current word */
  unsigned long org_word_length;	/**< length of current org word */
  unsigned int last_word;		/**< read in last word from dict */

  /** These are set by pwdgen_readset. */
  char dictionary[PWD_LEN];		/**< dict file name */
  FILE* dict_file;			/**< file handle for dict file */
  unsigned long dict_file_ofs;		/**< ofs of first word in dict file */
  
  /** For dictionary sets that append or prepend stuff. */
  struct ssPWD* gen_pwd;		/**< ptr to secondary generators */

  unsigned int append;			/**< append/prepend stage */ 
  unsigned int prepend;			/**< flag for first/other prepend */ 
  char word_2[PWD_LEN];			/**< temp. word for prepending */
  struct ssCharset* charsets;		/**< pointer to table w/ all charsets */

  /** for collecting blocks of passwords w/ padding */
  int par_padd; 			/**< how to padd 
    					     -X means padd pwd to X bytes
					     +X means "pwd" + "padding" */
  unsigned int  par_maxcnt; 		/**< how many we can have in buff */
  char par_pchar; 			/**< the byte we padd with */

  } sPWD;

/** check for timeout or whether a status report is nec. */
int pwdgen_time_check (struct ssPWD *sPWD);

/** init the password generator */
struct ssPWD* pwdgen_init (
  const char* start,
  const char* end,
  const char* target,
  const char* set,
  const unsigned int timeout,
  const struct ssKey* keys
  );

/** find a charset by it's ID number or return NULL */
struct ssCharset* pwdgen_find_set (const struct ssCharset* charset, const unsigned long id);

/** re-init the password generator */
int pwdgen_reinit ( struct ssPWD* sPWD ); 

/** advance password by one, return 0 for okay, and 1 for past-end-pwd */
extern int pwdgen_next ( struct ssPWD* sPWD );

/** update the took field, and convert the password with a2h() */
extern void pwdgen_stop ( struct ssPWD* sPWD );

/** set up the tables (and init the new password) */
extern void pwdgen_init_tables ( struct ssPWD* sPWD, const int type );

/** extract pwds from image, weed out doubles, call dofunction() on each */
int pwdgen_extract ( struct ssPWD* pwd );

/* *********************************************************************** */
/* PUBLIC functions */

/** set the password you want to have printed out as solution */ 
extern void pwdgen_found ( struct ssPWD* sPWD, const char * solution, 
			   const unsigned int len );

/** did check more than one pwd, so modify the counter the pwd generator */
void pwdgen_add ( struct ssPWD* sPWD, const unsigned long pwds );

/* modify the crc of this chunk */
void pwdgen_update_crc 
  ( struct ssPWD* sPWD, const char *data, const unsigned int len );

int	pwdgen_extract		(struct ssPWD* pwd);

#endif
