
/* 
  
  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/ 

/* for Makefile.PL 
$VERSION = "2.16"
*/

/* for C code */
#define DICOP_VERSION "v2.16 build 1"


void a2h (unsigned char * a, const unsigned int len);
void h2a (unsigned char * h);
int cmpStrings (unsigned char* a, unsigned char* b, int la, int lb);

/* return codes for dofunction() */

#define PWD_SUCCESS 2
#define PWD_FAIL 0
#define PWD_ERROR 7
#define PWD_TIMEOUT 4

/* return code(s) for initfunction() */

#define PWD_INIT_OK 0

/* other things we generally want to include */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/hexdump.h"

