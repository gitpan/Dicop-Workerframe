/*!
 * @file
 * @ingroup workerframe
 * @brief Header file for character set conversions
 * 
 * @copydoc copyrighttext
*/

#ifndef DICOP_PWD_CONV_H
#define DICOP_PWD_CONV_H

#include "pwdgen.h"

/* this is defined in pwd_conv.c and linked in together with libdicop */
extern const char _encoding_names[][40];

/** Convert an encoding number to the name. */
/** The first encoding number that represents a Multi-byte sequence. Encodings
    smaller than this are one character per byte */
#define MULTIBYTE_ENCODING UTF_8

/** Describes a string object, with the length given in bytes and characters,
    including the encoding of the string. */
struct sPwdString
  {
  size_t characters;		/**< the number of characters in this string */
  size_t bytes;			/**< the number of bytes the string contains */
  size_t size;			/**< the maximum buffer size for the content */
  enum eEncodings encoding;	/**< the current encoding of this string */
  char *content;		/**< pointer into buffer containing the string data */
  unsigned long flags;		/**< a read-only flag field denoting:\n
					- 0 : string/encoding is valid
					- 1 : string is read-only
					- 2 : string cannot be freed */
  char *_buffer;		/**< INTERNAL: pointer to the raw buffer */
  };

#define PWDSTR_INVALID		0x01
#define PWDSTR_READ_ONLY	0x02
#define PWDSTR_STATIC		0x04
#define PWDSTR_WRITABLE		(PWDSTR_INVALID + PWDSTR_READ_ONLY)
#define PWDSTR_RO_STATIC	(PWDSTR_READ_ONLY + PWDSTR_STATIC)

/** Return true if the given string is valid. */
#define PWDSTR_IS_VALID(str) (0 == (str->flags & PWDSTR_INVALID))
/** Return true if the given string is invalid. */
#define PWDSTR_IS_INVALID(str) (0 != (str->flags & PWDSTR_INVALID))

/** Return true if the given string is valid and writable and not static. */
#define PWDSTR_IS_MUTABLE(str) (0 == str->flags)

/** Return true if the given string is read-only. */
#define PWDSTR_IS_READ_ONLY(str) (0 != (str->flags & PWDSTR_READ_ONLY))
/** Return true if the given string is writable. */
#define PWDSTR_IS_WRITABLE(str) (0 == (str->flags & PWDSTR_WRITABLE))

/** Return true if the given string cannot be freed. */
#define PWDSTR_IS_STATIC(str) (0 != (str->flags & PWDSTR_STATIC))

/** Mark the given string as static. */
#define pwdgen_make_string_static(str) str->flags |= PWDSTR_STATIC;
/** Mark the given string as read-only. */
#define pwdgen_make_string_read_only(str) str->flags |= PWDSTR_READ_ONLY;

/* mark the string as invalid */
int pwdgen_make_string_invalid(const struct ssPWD* pwd, struct sPwdString* str);

/** The first encoding number that represents a Multi-byte sequence. Encodings
    smaller than this are one character per byte */
#define MULTIBYTE_ENCODING UTF_8

/* Warn about running out of memory and return -1 */
int _out_of_memory(const struct ssPWD* pwd, const char* text, const size_t size);

/* creates an empty string, with the given size in bytes as the maximum
    buffer size */
struct sPwdString* pwdgen_new_string (const struct ssPWD* pwd, const size_t size, const enum eEncodings encoding);

/* creates a string from the given byte sequence string without checking it */
struct sPwdString* pwdgen_string (const struct ssPWD* pwd, const unsigned char* string, const size_t size, const size_t buffer_size, const enum eEncodings encoding);

/* Create a string from input bytes in input_enc encoded as output_enc.
   Does validate the input sequence. */
struct sPwdString* pwdgen_decode_string (
	const struct ssPWD* pwd, const char* buffer, const size_t size, 
	const enum eEncodings input_enc,
	const enum eEncodings output_enc);

/* free a string struct */
void pwdgen_free_string (const struct ssPWD* pwd, struct sPwdString* string);

/* Set the string contents to the empty string, without changing the
    string buffer size or encoding. */
int pwdgen_empty_string(const struct ssPWD* pwd, struct sPwdString* str);

/* Resize the storage buffer of a string */
int pwdgen_resize_string(const struct ssPWD* pwd, struct sPwdString* str, const size_t min_size);

/* Convert a string in-place to the given encoding. */
int pwdgen_convert_to (const struct ssPWD* pwd, struct sPwdString* input, const enum eEncodings encoding, const char replace);

/** Convert a string in-place to the given encoding, using the set replacement
    char (defaulting to '?') for any invalid or non-expressable input char. */
#define pwdgen_convert(pwd, input, enc)\
  pwdgen_convert_to(pwd, input, enc, pwd->replacement)

/** Convert a string in-place to UTF-8. */
#define pwdgen_convert_to_utf8(pwd, input)\
  pwdgen_convert_to(pwd, input, UTF_8, pwd->replacement)

/** Convert a string in-place to UTF-16. */
#define pwdgen_convert_to_utf16(pwd, input)\
  pwdgen_convert_to(pwd, input, UTF_16, pwd->replacement)

/** Convert a string in-place to UTF-32. */
#define pwdgen_convert_to_utf32(pwd, input)\
  pwdgen_convert_to(pwd, input, UTF_32, pwd->replacement)

/* print a given string to the console (possible converting it) */
void pwdgen_print(const struct ssPWD* pwd, const char* format, struct sPwdString* string);

/* Dump the string contents including the encoding */
void hexdump_string(const char* title, const struct sPwdString *string);

/* check the string to contain only valid bytes for the its encoding */
int pwdgen_check_string(const struct ssPWD* pwd, struct sPwdString* string);

#define pwdgen_encoding(encoding) _encoding_names[encoding]

/* Convert an encoding name to the number. */
enum eEncodings pwdgen_encoding_from_name(const char* encoding);

/* Count the characters in the string, and return them */
size_t pwdgen_strlen(const struct ssPWD* pwd, struct sPwdString* string);

/* Make a full copy of a string struct. */
struct sPwdString* pwdgen_strcpy(const struct ssPWD* pwd, const struct sPwdString* src, const enum eEncodings encoding);

/* Duplicate contents of source string into target string */
int pwdgen_strdup(const struct ssPWD* pwd, struct sPwdString* dst, const struct sPwdString* src);

/* Concatenate two strings */
struct sPwdString* pwdgen_strcat(const struct ssPWD* pwd, 
		struct sPwdString* first, const struct sPwdString* second);

/* Compare two strings. */
int pwdgen_strcmp(const struct ssPWD *pwd, const struct sPwdString* first, const struct sPwdString* second);

/* Return 1 if strings are equal, 0 otherwise. */
int pwdgen_streq(const struct ssPWD *pwd, const struct sPwdString* first, const struct sPwdString* second);

/* *********************************************************************** */
/* converting between upper and lower case */

/* Internal: convert one character from the UTF-8 buffer to lower case. */
int _utf8_char_to_lc(const struct ssPWD* pwd, const struct sPwdString* string, unsigned long* ofs);

/* Internal: convert one character from the UTF-8 buffer to UPPER case. */
int _utf8_char_to_uc(const struct ssPWD* pwd, const struct sPwdString* string, unsigned long* ofs);

/* Make the string all UPPERCASE */
int pwdgen_uc(const struct ssPWD* pwd, struct sPwdString* string);

/* Make the string's first character upper case: "Uppercase" */
int pwdgen_uc_first(const struct ssPWD* pwd, struct sPwdString* string);

/* Make the string all lowercase */
int pwdgen_lc(const struct ssPWD* pwd, struct sPwdString* string);

/* Make the string's first character in lower case: "lOWERCASE" */
int pwdgen_lc_first(const struct ssPWD* pwd, struct sPwdString* string);

/* Make every odd character in the string to uPpErCaSe */
int pwdgen_uc_odd(const struct ssPWD* pwd, struct sPwdString* string);

/* Make every even character in the string to UpPeRcAsE */
int pwdgen_uc_even(const struct ssPWD* pwd, struct sPwdString* string);

/* Make every the last character LOWERCASe */
int pwdgen_lc_last(const struct ssPWD* pwd, struct sPwdString* string);

/* Make every the last character uppercasE */
int pwdgen_uc_last(const struct ssPWD* pwd, struct sPwdString* string);

/* Make all the vowels UppErcAsE */
int pwdgen_uc_vowels(const struct ssPWD* pwd, struct sPwdString* string);

/* Make all the consonants uPPeRCaSe */
int pwdgen_uc_consonants(const struct ssPWD* pwd, struct sPwdString* string);

/* *********************************************************************** */

/* Reverse the string: "Müller" => "rellüM" */
int pwdgen_strrev(const struct ssPWD* pwd, struct sPwdString* string);

/* Internal: return error from string conversion */
int _convert_error(const enum eEncodings in, const enum eEncodings out, const int id);
#endif

