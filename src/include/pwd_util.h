/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for utility functions like file I/O and error handling
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWD_UTIL_H
#define DICOP_PWD_UTIL_H

#include <errno.h>

#ifdef WIN32

#include <sys/types.h>
#define fseeko _fseeki64
#define ftello _ftelli64

#endif

/* Try to open a file from its name (in current dir and one or two dirs up) */
FILE* pwdgen_find_file (const char* file, const char* options);

/** Open a file and check for errors, printing an error message if nec. 
    Upon error returns with PWD_ERROR from the current function. */
#define pwdgen_file_open(filename, options, file)\
  file = fopen (filename, options);\
  if (NULL == file)\
    {\
    printf ("Error: Cannot open file '%s': '%s'\n", filename, strerror(errno));\
    return PWD_ERROR;\
    }

/** Close a file */
#define pwdgen_file_close(file) fclose(file)

/** Read X bytes from the given file stream, the given name is used to report
    the proper error message. Upon error returns with PWD_ERROR from the current
    function. */
#define pwdgen_file_read(target,count,fin,filename)\
  if ((fread(target,sizeof(unsigned char),count,fin)) != count)\
    {\
    pwdgen_read_error(count, filename);\
    }

/** Read X bytes from the given offset in the given file stream, the given name
    is used to report the proper error message. Upon error returns with PWD_ERROR
    from the current function. */
#define pwdgen_file_read_from(offset,target,count,fin,filename)\
  if (offset < 0)\
    {\
    pwdgen_file_seek(offset, SEEK_END, fin, filename);\
    } else {\
    pwdgen_file_seek(offset, SEEK_SET, fin, filename);\
    }\
  pwdgen_file_read(target, count, fin, filename);

/** Seek to the given position in the file stream. 
    Upon error returns with PWD_ERROR from the current function. */
#define pwdgen_file_seek(where, whence, fin, filename)\
  if (fseeko (fin, where, whence) != 0)\
    {\
    printf ("Error: Cannot seek to position %ld in '%s': '%s'\n", (long int)where, filename, strerror(errno));\
    return PWD_ERROR;\
    }

/** Return the size of the file */
off_t pwdgen_file_size(FILE *fin);

/** report a file I/O error and return with PWD_ERROR from the current function. */
#define pwdgen_io_error(msg, filename)\
  printf ("Error: %s from '%s': '%s'\n", msg, filename, strerror(errno));\
  return PWD_ERROR;

/** report a file read error and return with PWD_ERROR from the current function. */
#define pwdgen_read_error(count, filename)\
  printf ("Error: Could not read %i bytes from '%s': '%s'\n", count, filename, strerror(errno));\
  return PWD_ERROR;

/** Convert a hexstring with LEN charaters back to a buffer of LEN/2 bytes.
    <br>
    Should not be used, use pwdgen_from_hex() instead!
  */

void
pwdgen_convert_hex(char *target, const char *targetkey, const unsigned int len);

#define pwdgen_from_hex(target,targetkey,len) if (strlen(targetkey) != len)\
    {\
    printf ("Error! Targetkey must be %i chars long, but is %i.\n",\
	len, strlen(targetkey));\
    return PWD_ERROR;\
    }\
  pwdgen_convert_hex(target, targetkey, len);

#endif
