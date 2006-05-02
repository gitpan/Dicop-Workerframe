/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

#include "../include/pwdgen.h"

/* how many position=charset pairs (not counting default one) can a grouped
   charset have? */
#define MAXPAIRS 32

/* how many different characters per simple set? */
#define MAXCHARS 256


/* charset types */
#define SET_MAX_TYPE 3

#define SET_SIMPLE 0
#define SET_GROUPED 1
#define SET_DICTIONARY 2
#define SET_EXTRACT 3

/* for DICTIONARY:

   paircnt = 2 (two 'pair' or set) 
   pos[0] = 0 (append)
   set[0] = ptr to charset
   startlen[0] = 1 (one char)
   endlen[0] = 2 (two chars)
   pos[0] = 1 (append)
   set[0] = ptr to charset
   startlen[0] = 2 (one char)
   endlen[0] = 3 (two chars)

   Would mean append 1 to 2 chars of set[0] and then prepend 2 to 3 chars of
   set[1].
*/

struct ssCharset
  {
  unsigned int id;			/* charset id */
  unsigned int type;			/* SET_(SIMPLE|GROUPED|DICTIONARY|EXTRACT) */
  /* only for simple ones */
  unsigned int chars[MAXCHARS]; 	/* the 'characters' */
  unsigned int length;			/* nr of chars */

  /* GROUPED:   how many pos/set pairs do we have
     DICTIONARY: how many append/prepend pairs do we have */
  unsigned int paircnt;			
  /* GROUPED: position this pair applies to
     DICTIONARY: position where to put: 0 - append, 1 - prepent */
  unsigned int pos[MAXPAIRS];
  /* DICTIONARY: start and endlen of set to append/prepend */
  unsigned int startlen[MAXPAIRS];
  unsigned int endlen[MAXPAIRS];
  /* DICITONARY: list of charsets from which to append/prepend */
  /* GROUPED: list of charsets from which to form strings */
  struct ssCharset *set[MAXPAIRS];	/* set-ptr for this position 
    ptrs to other charset in the same list, e.g. [1]->set[0] might point to
    [0], and [1]->set[1] might also point to [1], or [2] etc. But never points
    to the same set (because it only points from a GROUPED to a SIMPLE or from
    a DICTIONARY to either a GROUPED or a SIMPLE one. */
  struct ssCharset *def;		/* ptr to default (simple) set */
  };

/* reading routines */
struct ssCharset* pwdgen_read ( struct ssPWD* sPWD );
unsigned int pwdgen_readset ( unsigned char* set, struct ssPWD* sPWD );
void pwdgen_set_cfg_keys ( struct ssPWD* sPWD, struct ssKey* cfg );

