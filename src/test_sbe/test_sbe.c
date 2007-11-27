/**
 * @file
 * @ingroup workerframetestcnv
 * @brief Test single-byte encodings worker
 * 
 * Dicop test worker for testing that the charset conversion
 * and encoding routines work correctly with the single-byte encodings.
 *
 * This test worker does a systematic conversion test for each single-byte
 * encoding by converting each possible input byte to UTF-8 and back,
 * comparing the results.
 *
 * The output format of this worker is:
 * # 1..2
 * # ok 1 - comment
 * # ok 2 - comment
 *
 * etc. e.g. the same output format as the Perl TAP output with '#'
 * prepended. This output is read by t/enc.t and converted to normal TAP
 * output so the testsuite is happy.
 *
 * The worker ignores the input arguments, so run it like so:
 *
 * <code>test_sbe 30 31 30 2</code>
 *
 * @copydoc copyrighttext
*/

#include <dicop_unused.h>

int test = 0;
struct ssPWD* spwd;

void printinfo(void)
  {
  printf ("DiCoP test_sbe worker v0.01  (c) Copyright by BSI 1998-2007\n\n");
  PRINT_INFO;
  }

/* setup the test plan */
void plan_tests(const int N)
  {
  test = 0;
  printf ("# 1..%i\n", N);
  }

/* next test or abort the testing due to a critical error */
void next_test(void)
  {
  fflush(NULL);
  test++;
  if (spwd->error == 0)
    {
    return;
    }
  printf ("A critical error occured. Aborting.\n");
  exit(1);
  }

/* skip N tests */
void skip(const int N, const char* reason)
  {
  int i;

  for (i = 0; i < N; i++)
    {
    test++;
    printf ("# ok %i - skip: %s\n", test, reason);
    }
  fflush(NULL);
  }

/* assert the pointer is not null */
int non_null(const void* p, const char* name)
  {
  next_test();
  if (NULL != p)
    {
    printf ("# ok %i - %s is not NULL\n", test, name);
    return 1;
    }
  printf ("# not ok %i - %s\n", test, name);
  printf ("# #   Failed test - pointer '%s' was NULL.\n", name);
  return 0;
  }

/* compares got to expect as integers */
int equals(const int got, const int expect, const char* name)
  {
  next_test();
  if (got == expect)
    {
    printf ("# ok %i - %s\n", test, name);
    return 1;
    }
  printf ("# not ok %i - %s\n", test, name);
  printf ("# #   Failed test '%s'\n", name);
  printf ("# #          got: %i\n# #     expected: %i\n", got, expect);
  return 0;
  }

/* compares got to expect as pointer */
int equal_ptr(const void* got, const void* expect, const char* name)
  {
  next_test();
  if (got == expect)
    {
    printf ("# ok %i - %s\n", test, name);
    return 1;
    }
  printf ("# not ok %i - %s\n", test, name);
  printf ("# #   Failed test '%s'\n", name);
  printf ("# #          got: %p\n# #     expected: %p\n", got, expect);
  return 0;
  }

/* compares got to expect as memory areas */
int cmp(const char* got, const char* expect, const unsigned int len, const char* name)
  {
  unsigned int i, l;

  next_test();

  if (0 == memcmp(got, expect, len))
    {
    printf ("# ok %i - %s\n", test, name);
    return 1;
    }

  printf ("# not ok %i - %s\n", test, name);
  if (len <= 16)
    {
    printf ("# #   Failed test '%s':\n", name);
    hexdump_fancy ("# #          got: ", "", got,    len, "", " ", "  ", 0, 0);
    hexdump_fancy ("# #     expected: ", "", expect, len, "", " ", "  ", 0, 0);
    }
  else
    {
    printf ("# #   Failed test '%s' - differences are marked with '-->':\n", name);
    printf ("# #          got:\n");
    for (i = 0; i < len; i += 16)
      {
      l = 16; if ((len - i) < l) { l = len - i; }
      if (0 == memcmp(&got[i], &expect[i], l) )
        {
        printf ("# #               %04x ", i);
        hexdump_fancy ("", "", &got[i], l, "", " ", "  ", 0, 0);
        }
      else
        {
        printf ("# #        -->    %04x ", i);
        hexdump_fancy ("", "", &got[i], l, "", " ", "  ", 0, 0);
        }
      }
    printf ("# #     expected:\n");
    for (i = 0; i < len; i += 16)
      {
      l = 16; if ((len - i) < l) { l = len - i; }
      if (0 == memcmp(&got[i], &expect[i], l) )
        {
        printf ("# #               %04x ", i);
        hexdump_fancy ("", "", &expect[i], l, "", " ", "  ", 0, 0);
        }
      else
        {
        printf ("# #        -->    %04x ", i);
        hexdump_fancy ("", "", &expect[i], l, "", " ", "  ", 0, 0);
        }
      }
    }
  return 0;
  }

/* run a systematic test converting characters from one encoding into
   UTF-8 and back, and see if everything survived */
void test_conversion (const struct ssPWD* pwd, const enum eEncodings encoding)
  {
  struct sPwdString* str;
  int i;
  /* character was converted to a replacement char U+FFFD? */
  int replaced;
  unsigned char fffd[4] = { 0xef, 0xbf, 0xbd, 0 };

  str = pwdgen_new_string(pwd, 16, encoding);
  if (1 == non_null (str, "string for test_conversion"))
    {
    for (i = 0; i < 256; i++)
      {
      str->content[0] = i;
      str->content[1] = 0x0;
      str->characters = 1;
      str->bytes = 1;
      str->encoding = encoding;

      /* printf ("# # At char %c (0x%02x), encoding %s\n", i, i, pwdgen_encoding(encoding)); */

      pwdgen_convert(pwd, str, UTF_8);

      if (str->encoding != UTF_8)
	{
	equals (str->encoding, UTF_8, "char could be converted to UTF-8");
	}

      if (str->characters != 1)
	{
	equals (str->characters, 1, "number of characters is 1");
	}
      replaced = memcmp(str->content, (char*)fffd, 3);

      pwdgen_convert(pwd, str, encoding);
      if (str->encoding != encoding)
	{
	equals (str->encoding, encoding, "char could be converted back");
	}
      if (str->characters != 1)
	{
	equals (str->characters, 1, "number of characters is still 1");
	}
      if (str->bytes != 1)
        {
	equals (str->bytes, 1, "number of bytes is still 1");
	}
      if (0 != replaced)
        {
        if ((unsigned char)str->content[0] != i)
	  {
	  if (!equals ( (unsigned char)str->content[0], i, "could be converted back"))
	    {
	    printf ("# # Char: %i (0x%02x, '%c') in %s\n", i, i, i, pwdgen_encoding(encoding));
            hexdump_string("pwd",str);
	    }
	  }
        }
      else
        {
	if ( (unsigned char)str->content[0] != '?')
	  {
	  equals ( (unsigned char)str->content[0], '?', "could be converted back to '?'");
	  }
        }
      }
    }
  else { skip ( 256 * 6 + 1, "Couldn't allocate string for test_conversion"); }
 
  }

int initfunction(const struct ssPWD *pwd)
  {
  /* global copy so the test routines can check for critical errors */
  spwd = pwd;

  /* ******************************************************************** */
  printf ("### start ###\n");
  plan_tests(51 + 1);
 
  /* systematic conversion tests in single-byte encodings */
  test_conversion (pwd, ISO_8859_1);
  test_conversion (pwd, ISO_8859_2);
  test_conversion (pwd, ISO_8859_3);
  test_conversion (pwd, ISO_8859_4);
  test_conversion (pwd, ISO_8859_5);
  test_conversion (pwd, ISO_8859_6);
  test_conversion (pwd, ISO_8859_7);
  test_conversion (pwd, ISO_8859_8);
  test_conversion (pwd, ISO_8859_9);
  test_conversion (pwd, ISO_8859_10);
  test_conversion (pwd, ISO_8859_11);
  test_conversion (pwd, ISO_8859_13);
  test_conversion (pwd, ISO_8859_14);
  test_conversion (pwd, ISO_8859_15);
  test_conversion (pwd, ISO_8859_16);
  test_conversion (pwd, KOI8_R);
  test_conversion (pwd, KOI8_U);
  test_conversion (pwd, CP437);
  test_conversion (pwd, CP850);
 
  test_conversion (pwd, CP852);
  test_conversion (pwd, CP855);
  test_conversion (pwd, CP857);
  /* test_conversion (pwd, CP858); */
  test_conversion (pwd, CP860);
  test_conversion (pwd, CP861);
  test_conversion (pwd, CP862);
  test_conversion (pwd, CP863);
  test_conversion (pwd, CP865);
  test_conversion (pwd, CP866);
  test_conversion (pwd, CP869);
  test_conversion (pwd, CP874);

  test_conversion (pwd, CP1250);
  test_conversion (pwd, CP1251);
  test_conversion (pwd, CP1252);
  test_conversion (pwd, CP1253);
  test_conversion (pwd, CP1254);
  test_conversion (pwd, CP1255);
  test_conversion (pwd, CP1256);
  test_conversion (pwd, CP1257);
  test_conversion (pwd, CP1258);

  /* is an alias for ISO_8859_11 */
  equals (TIS_620, ISO_8859_11, "TIS-620 is also known as ISO-8859-1");
  
  test_conversion (pwd, MacRoman);
  test_conversion (pwd, MacCentralEurRoman);
  test_conversion (pwd, MacIcelandic);
  test_conversion (pwd, MacIcelandic);
  test_conversion (pwd, MacCroatian);
  test_conversion (pwd, MacRomanian);
  test_conversion (pwd, MacRumanian);
  test_conversion (pwd, MacCyrillic);
  test_conversion (pwd, MacUkrainian);
  test_conversion (pwd, MacGreek);
  test_conversion (pwd, MacTurkish);
  test_conversion (pwd, MacSami);

  /* not done yet:
  test_conversion (pwd, MacHebrew);
  test_conversion (pwd, MacThai);
  test_conversion (pwd, MacArabic);
  test_conversion (pwd, MacFarsi);
  */

  /* ******************************************************************** */
  printf ("### end ###\n");

  return PWD_INIT_OK;		/* init was okay */
  }

int dofunction( const struct ssPWD *pwd )
  {
  hexdump_string("Current", pwd->cur);

  return PWD_FAIL;					/* not found yet */
  }

