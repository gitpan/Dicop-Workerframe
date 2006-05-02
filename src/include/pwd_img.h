
/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

#include "dicop.h"

#include "../include/pwdread.h"

#define EXTRACT_SKIP_INVALIDS   0x01
#define EXTRACT_ODD_EVEN        0x02
#define EXTRACT_DEBUG           0x04
#define EXTRACT_TRACE           0x08

/* ************************************************************************* */
/* prototypes */

/* extract strings of len N */
unsigned long pwdgen_img_extract (
	unsigned char* buffer,
	unsigned char* pwd_list,
	long bufsize,
	unsigned long len,
	unsigned int* valid,
	unsigned long options,
	unsigned long skip,
	struct ssPWD* pwd
  );

/* read image blockwise and extract strings of len N */
unsigned long pwdgen_img_read( 
	struct ssPWD* pwd,
	unsigned int len,
	unsigned char *filename, 
	unsigned int* valid, 
	unsigned long options,
	unsigned long image_type
  );

/* take the list of pwd's, and sort it and then weed out doubles */
unsigned long pwdgen_img_sort (
	unsigned char* pwd_list,
	unsigned long pwds,
	unsigned long len);

unsigned int pwdgen_img_check (
	struct ssPWD* pwd,
	unsigned char* pwd_list,
	unsigned long pwds,
	unsigned long len
  );
