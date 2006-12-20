/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for the password sorting code (used by the string extractor).
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDSORT_H
#define DICOP_PWDSORT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ************************************************************************* */
/* prototypes */

/** sort 'pwds' strings of len 'len' using merge sort */
void pwdgen_img_merge_sort (
	unsigned char* pwd_list,
	unsigned char* temp,
	const unsigned long first,
	const unsigned long last,
	const unsigned long len
  );

/** sort 'pwds' strings of len 'len' using shell sort */
void pwdgen_img_shell_sort (
	unsigned char* pwd_list,
	unsigned char* temp,
	const unsigned long first,
	const unsigned long last,
	const unsigned long len
  );

unsigned long pwdgen_img_remove_doubles_in_place (
  unsigned char* pwd_list,
  const unsigned long count,
  const unsigned long len	/* len including zero termination or padding! */
  );

void pwdgen_img_merge (
  unsigned char* pwd_list,
  const unsigned long first,
  const unsigned long second,
  const unsigned long len	/* len including zero termination or padding! */
  );

/* memcpy and memcmp that are faster for small data sizes */
int _compare (const unsigned char* a, const unsigned char* b, const unsigned int len);
void _copy (unsigned char* a, const unsigned char* b, unsigned int len);

#endif
