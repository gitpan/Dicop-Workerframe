
/*

  Copyright (c) 1998-2006,
  Bundesamt fuer Sicherheit in der Informationstechnik (BSI)

  This file is part of Dicop-Workerframe. For licencing information see the
  file LICENCE in the distribution, or http://www.bsi.bund.de/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ************************************************************************* */
/* prototypes */

/* sort 'pwds' strings of len 'len' using merge sort */
void pwdgen_img_merge_sort (
	unsigned char* pwd_list,
	unsigned char* temp,
	unsigned long first,
	unsigned long last,
	unsigned long len
  );

/* sort 'pwds' strings of len 'len' using shell sort */
void pwdgen_img_shell_sort (
	unsigned char* pwd_list,
	unsigned char* temp,
	unsigned long first,
	unsigned long last,
	unsigned long len
  );

unsigned long pwdgen_img_remove_doubles_in_place (
  unsigned char* pwd_list,
  unsigned long count,
  unsigned long len             /* len including zero termination or padding! */
  );

void pwdgen_img_merge (
  unsigned char* pwd_list,
  unsigned long first,
  unsigned long second,
  unsigned long len             /* len including zero termination or padding! */
  );

/* memcpy and memcmp that are faster for small data sizes */
int _compare (const unsigned char* a, const unsigned char* b, unsigned int len);
void _copy (unsigned char* a, const unsigned char* b, unsigned int len);
