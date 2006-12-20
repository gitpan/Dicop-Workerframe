/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for the parallell interface for Dicop
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWDPAR_H
#define DICOP_PWDPAR_H

#include <pwdgen.h>

#define PWD_NO_PADD	0
/** to be used as in "PWD_PADD_TO 9" resulting in "- 9" */
#define PWD_PADD_TO     -

/* setup gathering of passwords */
int pwdgen_par_init (
	const unsigned int count,
	const signed int padd,
	const char paddchar,
	struct ssPWD* pwd
  );

/* return password length (derived from padding and current length) */
unsigned int pwdgen_par_len (const struct ssPWD* pwd );

int	pwdgen_par_push		(struct ssPWD* pwd);
void	pwdgen_par_done		(struct ssPWD* pwd);
int	pwdgen_par_process	(struct ssPWD* pwd);

#endif
