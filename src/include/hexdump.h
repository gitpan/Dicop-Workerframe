/*!
 * @file
 * @ingroup workerframe
 * @brief A handy hexdump routine
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_HEXDUMP_H
#define DICOP_HEXDUMP_H

#include <stdio.h>
/* for isprint() */
#include <ctype.h>
#include <locale.h>

void hexdump(const char* title, const void *p, const unsigned int size);
void hexdump_pwd(const char* title, const void *p, const unsigned int size);
void hexdump_line(const void *p, const unsigned int size);
void hexdump_inline(const char *title, const void *p, const unsigned int size);
void hexdump_fancy(const char* title, const char* footer, const void *p, const unsigned int size, const char* indend, const char* sep, const char* center_sep, const int adr, const int newline);

#endif
