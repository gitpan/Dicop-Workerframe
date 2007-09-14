/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for extracting strings from image files
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDIMG_H
#define DICOP_PWDIMG_H

#include <dicop.h>
#include <pwdsort.h>
#include <pwddict.h>
#include <pwdread.h>

/** When extracting characters, simple skip over invalid characters */
#define EXTRACT_SKIP_INVALIDS   0x01
/** Extract characters at odd and even positions, too. */
#define EXTRACT_EVEN_ODD	0x02
/** Enable debug output for extraction. */
#define EXTRACT_DEBUG		0x04
/** Enable tracing output for extraction. */
#define EXTRACT_TRACE		0x08
/** Extract only valid UTF-8 */
#define EXTRACT_UTF8		0x10

/* ************************************************************************* */
/* prototypes */

/* extract strings of len N */
unsigned long pwdgen_img_extract (
	const unsigned char* buffer,
	unsigned char* pwd_list,
	const unsigned long bufsize,
	const unsigned long len,
	unsigned int* valid,
	const unsigned long options,
	const unsigned long skip,
	struct ssPWD* pwd
  );

/* extract valid UTF8 strings of N bytes */
unsigned long pwdgen_img_extract_utf8 (
	const unsigned char* buffer,
	unsigned char* pwd_list,
	const unsigned long bufsize,
	const unsigned long len,
	const unsigned long options,
	struct ssPWD* pwd
  );

/* read image blockwise and extract strings of len N */
unsigned long pwdgen_img_read( 
	struct ssPWD* pwd,
	const unsigned int len,
	const char *filename, 
	unsigned int* valid, 
	const unsigned long options
  );

/* take the list of pwd's, and sort it and then weed out doubles */
unsigned long pwdgen_img_sort (
	const unsigned char* pwd_list,
	const unsigned long pwds,
	const unsigned long len);

unsigned int pwdgen_img_check (
	struct ssPWD* pwd,
	const unsigned char* pwd_list,
	const unsigned long pwds,
	const unsigned long len
  );
#endif
