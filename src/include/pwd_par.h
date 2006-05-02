
/*

  Copyright (c) 2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/pwdgen.h"

#ifndef PWD_PAR_INCLUDE
#define PWD_PAR_INCLUDE

#define PWD_NO_PADD	0
#define PWD_PADD_TO     -

/* ************************************************************************* */
/* prototypes */

/* public: */

/* setup gathering */
int pwdgen_par_init (
	unsigned int count,
	signed int padd,
	unsigned char paddchar,
	struct ssPWD* pwd
  );

#endif
