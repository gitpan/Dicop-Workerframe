/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for the password generator of Dicop
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDGEN_H
#define DICOP_PWDGEN_H

#include "pwdcfg.h"
#include <time.h>
#include "encoding/encodings.h"

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

  const struct sPwdString* cur;		/**< the current password, as a string struct
						with encoding, length in bytes and characters */
  enum eEncodings encoding[32];		/**< a list of encodings in that the current
						password should be presented. For each password
						all the encodings will be generated and for
						each unique representation, dofunction()
						will be called */
  struct ssKey* config;			/**< list of options from config file 
					     use pwdgen_find_key() to access it */
  char target[4*PWD_LEN];		/**< copy of targetkey */
  unsigned long targetlen;		/**< length of targetkey */

  /** for collecting blocks of passwords w/ padding */

  char *par_buff; 			/**< current block of passwords */
  unsigned int	 par_cnt; 		/**< how many are in buff? */
  unsigned int	 par_len; 		/**< current len of one pwd in buff */

  /** these for are internal usage and probably not interesting to the caller */

  int error;				/**< set to non-zero upon encountering any error */

  unsigned long cur_encoding;		/**< if the user requested the passwords to be generated
						in more than one encoding, this variable tracks
						which encoding we currently did produce */

  unsigned long encodings;		/**< how many entries are in use_encodings? */
  enum eEncodings locale;		/**< the encoding of the current locale, used for
						printing strings to the console */
  enum eEncodings generator_encoding;	/**< the encoding in which the generator produces
						each password (either via table, dictionary,
						or extract). These passwords will then be
						converted to all the user requested encodings. */

  unsigned int crc;			/**< CRC of current chunk */	
  char end[PWD_LEN];			/**< last password */
  char start[PWD_LEN];			/**< first password */
  unsigned long endlen;			/**< length of last pwd */
  unsigned long startlen;		/**< length of first pwd */
  struct ssCharset* set;		/**< pointer to character set */
  struct ssCharset* extract_set;	/**< pointer to character set used for extracting bytes */
  int id;				/**< id of character set */
  unsigned int type;			/**< character set type */
  unsigned int report_status;		/**< report every X seconds your status */
  long double pwds;			/**< pwds done all in all */
  long double pwds_done;		/**< how many pwds since last check for
					     timeout */
  long double took;			/**< seconds from init() to stop() */
  long double timeout;			/**< timeout in seconds */
  time_t tstart;			/**< time of init */
  time_t tlast;				/**< last time we checked the timeout */
  time_t tlast_report;			/**< last time we reported our status */
  unsigned long max_pwds;		/**< how often to do timeout check, adjusted dynamically */
  unsigned int pwd_digit[PWD_LEN];	/**< column of cur pwd */
  unsigned int end_digit[PWD_LEN];	/**< column of end pwd */
  unsigned int once;			/**< if set to non-zero, do only one password */
  unsigned int _last_pwd;		/**< if set to non-zero, the current password is
						the last one, but we still need to do all the
						different encodings for it */

  const struct sPwdString* prefix;	/**< a struct for converting the current word
						to reverse, upper/lower case etc */
 
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
  unsigned int first_mutation;		/**< the very first mutation we do */
  
  unsigned int todo_stages;		/**< bit mask of stages todo */
  unsigned int done_stages;	  	/**< copy of todo, then set to 0 bit by
					     bit */
  unsigned int cur_stage;  		/**< the current stage */

  const struct sPwdString* org_word;	/**< orginal word from dictionary */
  const struct sPwdString* word;	/**< a struct for converting the current word
						to reverse, upper/lower case etc */

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
  struct ssCharset* charsets;		/**< pointer to table with all known charsets */

  /** for collecting blocks of passwords w/ padding */
  int par_padd; 			/**< how to padd 
    					     -X means padd pwd to X bytes
					     +X means "pwd" + "padding" */
  unsigned int par_maxcnt; 		/**< how many we can have in buff */
  char par_pchar; 			/**< the byte we padd with */
  char replacement;			/**< the character that is to be used in place of invalid
						input bytes when converting strings. Defaults to "?" */
  unsigned int identify;		/**<  0 => normal, 1 => --identify was used */
  } sPWD;

/* check for timeout or whether a status report is nec. */
int pwdgen_time_check (struct ssPWD *sPWD);

/* Parse output from setlocale() and return encoding. */
enum eEncodings pwdgen_parse_locale (const char *locale);

/* query locale information */
int pwdgen_query_locale (struct ssPWD *sPWD);

/* init the password generator */
struct ssPWD* pwdgen_init (
  const char* start,
  const char* end,
  const char* target,
  const char* set,
  const unsigned int timeout,
  const struct ssKey* keys
  );

/* find a charset by its ID number or return NULL */
const struct ssCharset* pwdgen_find_set (const struct ssCharset* charset, const long id);

/* create a virtual charset */
struct ssCharset* pwdgen_virtual_set(const int type);

/* re-init the password generator */
int pwdgen_reinit (struct ssPWD* sPWD); 

/* second init phase after user table initialization */
int pwdgen_finalize (const struct ssPWD *pwd);

/* Set the error flag in the password generator. Returns the given
    error code. A zero indicates no error. */
int pwdgen_error (const struct ssPWD* sPWD, int error); 

/* advance password by one, return 0 for okay, and 1 for past-end-pwd */
extern int pwdgen_next (struct ssPWD* sPWD);

/* update the took field, and convert the password with a2h() */
void pwdgen_stop (struct ssPWD* sPWD);
   
/* Set the took field with the time elapsed from start until now. */
void pwdgen_took (struct ssPWD *sPWD);

/* set up the tables (and init the new password) */
void pwdgen_init_tables ( struct ssPWD* sPWD, const int type );

/* extract pwds from image, weed out doubles, call dofunction() on each */
int pwdgen_extract ( struct ssPWD* pwd );

/* update the pwd->cur struct with the current encoding of the current pwd */
int pwdgen_update_cur (const struct ssPWD *sPWD, const int always_convert);

#define PWDGEN_ERROR_CHECK if (0 != pwd->error) { return PWD_ERROR; }

/* *********************************************************************** */
/* PUBLIC functions */

/* set the password you want to have printed out as solution */ 
void pwdgen_found ( const struct ssPWD* sPWD, const char * solution, 
			   const unsigned int len );

/* did check more than one pwd, so modify the counter the pwd generator */
void pwdgen_add ( struct ssPWD* sPWD, const unsigned long pwds );

/* modify the crc of this chunk */
void pwdgen_update_crc 
  ( struct ssPWD* sPWD, const char *data, const unsigned int len );

int	pwdgen_extract		(struct ssPWD* pwd);

/* Tell the password generator that generated password should be converted
   to all these encodings, too. */
void pwdgen_use_encodings (
    const struct ssPWD *sPWD, const enum eEncodings encoding[], const unsigned char count);

/* Tell the password generator that generated password should be in this
   encoding. */
void pwdgen_use_encoding (
    const struct ssPWD *sPWD, const enum eEncodings encoding);

/* print a list of the generated encodings (done automatically by main()) */
void pwdgen_print_encodings (const struct ssPWD *sPWD);

/* add one encoding to the list, but only if it wasn't already in */
int pwdgen_add_encoding (struct ssPWD *sPWD, const enum eEncodings encoding);

#endif
