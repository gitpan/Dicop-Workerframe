/*!
 * @file
 * @ingroup workerframe
 * @brief Just like \ref dicop.h, but also defines stopfunction() and endfunction().
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_UNUSED_H
#define DICOP_UNUSED_H

#include "dicop.h"

/* This include generates two functions that are usually empty. */

void stopfunction( void ) { }
int endfunction( const struct ssPWD *pwd ) { return PWD_FAIL; }

#endif
