/**
 * @defgroup workerframetestcnv Test charset conversion worker
 * @ingroup workerframe 
 *
 * Dicop test worker for testing that the charset conversion
 * routines work correctly.
 *
 * @section config Config settings:
 *
 * The following config file settings are used by this worker:
 *
 * @section input_encoding input_encoding:
 *
 * Example: <code>input_encoding=UTF-8</code>
 *
 * This specifies that the raw bytes given as target key are to be interpreted
 * in this encoding.
 *
 * @section produce_encoding produce_encoding:
 *
 * Example: <code>produce_encoding=KOIR-8</code>
 *
 * In addition to producing the each password in ISO-8859-1 and UTF-8, the given
 * encoding will also be produced from each password.
 *
 * @section compare_encoding compare_encoding:
 *
 * Example: <code>compare_encoding=KOIR-8</code>
 *
 * The target bytes (interpreted in the encoding "input_encoding") will be
 * converted to the encoding specified in "compare_encoding" and then compared
 * to each produced password. The password is only likely to be "found" when the
 * "compare_encoding" is in the list of encodings given by "produce_encoding",
 * or the password consists only of ASCII characters.
 *
 * @copydoc copyrighttext
*/

/**
 * @file
 * @ingroup workerframetestcnv
 * @brief Test charset conversion routines.
 * 
 * @copydoc copyrighttext
*/

#include <dicop_unused.h>

enum eEncodings encoding;
char target[PWD_LEN * 2];
struct sPwdString* target_string;

void printinfo(void)
  {
  /* print here your version and copyright */
  printf ("DiCoP test_cnv worker v0.03  (c) Copyright by BSI 1998-2008\n\n");
  PRINT_INFO;
  }

int initfunction(const struct ssPWD *pwd)
  {
  struct ssKey* key;
  enum eEncodings input_encoding;

  /* let the default be these two encodings */
  pwdgen_add_encoding(pwd, ISO_8859_1);
  pwdgen_add_encoding(pwd, UTF_8);

  encoding = UTF_8;
  key = pwdcfg_find_key(pwd->config, "compare_encoding", CFG_NOFAIL);
  if (NULL != key)
    {
    /* store what we need to compare with */
    encoding = pwdgen_encoding_from_name(key->value);
    }

  printf (" And we will compare them against a target in %s.\n", 
    pwdgen_encoding(encoding));

  input_encoding = UTF_8;
  key = pwdcfg_find_key(pwd->config, "input_encoding", CFG_NOFAIL);
  if (NULL != key)
    {
    /* store input encoding */
    input_encoding = pwdgen_encoding_from_name(key->value);
    }

  printf (" Our input is in %s.\n", pwdgen_encoding(input_encoding));

  key = pwdcfg_find_key(pwd->config, "produce_encoding", CFG_NOFAIL);
  if (NULL != key)
    {
    /* store what we need to produce */
    pwdgen_add_encoding(pwd, pwdgen_encoding_from_name(key->value) );
    }

  input_encoding = UTF_8;
  /* generate the string struct from the target bytes */
  pwdgen_from_hex(target, pwd->target, pwd->targetlen);

  hexdump(" Target: ", target, pwd->targetlen >> 1);

  target_string =
    pwdgen_decode_string(pwd, target, pwd->targetlen >> 1, encoding, input_encoding);

  hexdump_string(" Target after conversion", target_string);

  printf ("Init done\n"); fflush(NULL);
  return PWD_INIT_OK;		/* init was okay */
  }

int dofunction( const struct ssPWD *pwd )
  {
  hexdump_string(" Current pwd:", pwd->cur);

  if (pwdgen_strcmp(pwd, pwd->cur, target_string) == 0)
    {
    return PWD_SUCCESS;
    }

  return PWD_FAIL;					/* not found yet */
  }

