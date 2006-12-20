/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for dictionary support for Dicop workers
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDDICT_H
#define DICOP_PWDDICT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DICT_LOWER                      1	/* lowercase */
#define DICT_UPPER                      2	/* UPPERCASE */
#define DICT_LOWER_FIRST                4	/* lOWERFIRST */
#define DICT_UPPER_FIRST                8	/* Upperfirst */
#define DICT_LOWER_LAST                 16	/* LOWERLASt */
#define DICT_UPPER_LAST                 32	/* upperlasT */
#define DICT_UPPER_ODD                  64	/* uPpErOdD */
#define DICT_UPPER_EVEN                 128	/* UpPeReVEn */
#define DICT_UPPER_VOWELS               256	/* UppErvOwEls */
#define DICT_UPPER_CONSONANTS           512	/* uPPeRCoNSoNaNTS */

#define DICT_FORWARD		        1	/* forward */
#define DICT_REVERSE			2	/* desrever */

/** advance password by one, return 0 for okay, and 1 for past-end-pwd */
unsigned int pwddict_next ( struct ssPWD* sPWD );

/** INTERNAL: read in the next work from the dictionary */
unsigned int pwddict_nextword ( struct ssPWD* sPWD );

/** INTERNAL: mutate org_word into word (forward/reverse etc) */
void pwddict_stage ( struct ssPWD* sPWD );

/** INTERNAL: open the dictionary file */
unsigned int pwddict_openfile ( struct ssPWD* sPWD );

/** INTERNAL: read lines until start pwd found */
unsigned int pwddict_fastforward ( struct ssPWD* sPWD );

#endif
