/**
 * @file
 * @ingroup workerframetestcnv
 * @brief Test charset conversion and encoding routines.
 *
 * Dicop test worker for testing that the charset conversion
 * and encoding routines work correctly.
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
 * <code>test_enc 30 31 30 2</code>
 *
 * @version
 * - 2007-09-04 v0.03 te add tests for pwdgen_strrev()
 * - 2008-04-22 v0.04 te add tests for UTF-16 and UTF-32 to UTF-8
 * - 2008-09-22 v0.05 te add tests for ISO-8859-1 => ASCII
 * 
 * @copydoc copyrighttext
*/

#include <dicop_unused.h>

int test = 0;
struct ssPWD* spwd;

void printinfo(void)
  {
  printf ("DiCoP test_enc worker v0.05  (c) Copyright by BSI 1998-2008\n\n");
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

/* compares got to expect as pointers */
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

int initfunction(const struct ssPWD *pwd)
  {
  struct sPwdString* foo, *bar, *russian, *mixed, *temp, *rc, *temp_1;
  int rc_cmp = 0;
  unsigned long r_len;
  int i;
  /* Müller */
  unsigned char Mueller[8] = { 'M',0xfc,'l','l','e','r', 0 };
  /* Müller in ASCII */
  unsigned char Mueller_ascii[8] = { 'M','?','l','l','e','r', 0 };
  /* Mller in ASCII */
  unsigned char Mller_ascii[8] = { 'M','l','l','e','r', 0 };
  /* for truncating */
  unsigned char Muell[8]   = { 'M',0xfc,'l','l', 0 };
  unsigned char Mue[8]     = { 'M',0xfc, 0 };
  /* rellüM */
  unsigned char rellueM[8] = { 'r','e','l','l',0xfc,'M', 0 };
  /* AnnA */
  unsigned char anna[8] = { 'A','n','n','A', 0 };
  /* müller */
  unsigned char mueller[8] = { 'm',0xfc,'l','l','e','r', 0 };
  /* MÜLLER */
  unsigned char MUELLER[8] = { 'M',0xdc,'L','L','E','R', 0 };
  /* rellüM */
  unsigned char rellueM_utf8[16] = { 'r','e','l','l',0xc3,0xbc,'M',0 };
  /* rellüM */
  unsigned char rellueM_utf16[16] = { 'r',0,'e',0,'l',0,'l',0,0xfc,0,'M',0,0 };
  /* rellüM */
  unsigned char rellueM_utf32[32] = { 'r',0,0,0,'e',0,0,0,'l',0,0,0,'l',0,0,0,0xfc,0,0,0,'M',0,0,0,0 };
  /* Müller in UTF-8*/
  unsigned char Mueller_utf8[8] = { 'M',0xc3,0xbc,'l','l','e','r',0x0 };
  unsigned char Muell_utf8[8]   = { 'M',0xc3,0xbc,'l','l',0x0 };
  unsigned char M_utf8[8]       = { 'M',0x0 };
  /* MüllerMüller */
  unsigned char Mueller2[16] = { 'M',0xc3,0xbc,'l','l','e','r','M',0xc3,0xbc,'l','l','e','r',0x0 };
  /* MüllerMueller */
  unsigned char Mueller3[16] = { 'M',0xc3,0xbc,'l','l','e','r','M','u','e','l','l','e','r',0x0 };
  /* Русский in UTF-8 */
  unsigned char russki[16] = { 0xd0, 0xa0, 0xd1, 0x83, 0xd1, 0x81, 0xd1, 0x81,
				0xd0, 0xba, 0xd0, 0xb8, 0xd0, 0xb9, 0x00, 0x00 };
  /* Русский in KOI8-R */
  unsigned char koi8r[16] = { 0xf2, 0xd5, 0xd3, 0xd3, 0xcb, 0xc9, 0xca, 0 };
  /* MüllerРусскийMüller in various encodings: */
  unsigned char tmp[32] = { 'M', 0xc3, 0xbc, 'l', 'l', 'e', 'r',
				0xd0, 0xa0, 0xd1, 0x83, 0xd1, 0x81, 0xd1, 0x81,
				0xd0, 0xba, 0xd0, 0xb8, 0xd0, 0xb9,
				'M', 0xc3, 0xbc, 'l', 'l', 'e', 'r', 0 };
  unsigned char conv[32] = { 'M', 0xfc, 'l', 'l', 'e', 'r',
				'?','?','?','?','?','?','?',
				'M', 0xfc, 'l', 'l', 'e', 'r', 0 };
  unsigned char koi8_r[32] = { 'M', '?', 'l', 'l', 'e', 'r',
 				0xf2, 0xd5, 0xd3, 0xd3, 0xcb, 0xc9, 0xca,
  				'M', '?', 'l', 'l', 'e', 'r', 0 };
  
  /* MüllerРусский in various encodings */
  unsigned char utf_16[32] = {  'M', 0, 0xfc, 0, 'l', 0, 'l', 0, 'e', 0, 'r', 0,
				0x20,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x3a,0x04,
				0x38,0x04,0x39,0x04, 0,0 };

  unsigned char utf_32[64] = {  'M', 0,0,0, 0xfc, 0,0,0, 'l',0,0,0, 'l',0,0,0, 'e', 0,0,0, 'r',0,0,0,
				0x43,4,0,0, 0x41,4,0,0, 0x41,4,0,0, 0x3a,4,0,0, 0x38,4,0,0, 0x39,4,0,0,
				0,0,0,0, };

  /* for truncating */
  unsigned char utf_16_truncated[32] = {  'M', 0, 0xfc, 0, 'l', 0, 'l', 0, 0, 0, };

  unsigned char utf_32_truncated[32] = {  'M', 0,0,0, 0xfc, 0,0,0, 'l',0,0,0, 'l',0,0,0, 0,0,0,0, };

  /* for converting various encodings to/from UTF-16 */
  /* for converting various encodings to/from UTF-16 */
  unsigned char utf_16_iso_8859_1[32] = {  'M', 0, 0xfc, 0, 'l', 0, 'l', 0, 'e', 0, 'r', 0, 0, 0 };
  unsigned char utf_16_koi8_r[32] = { 0x20,0x04,0x43,0x04,0x41,0x04,0x41,0x04,0x3a,0x04,
				0x38,0x04,0x39,0x04, 0,0 };
  unsigned char utf_32_iso_8859_1[32] = {  'M',0,0,00, 0xfc,0,0,0, 'l',0,0,0, 'l',0,0,0,
						 'e',0,0,0, 'r',0,0,0, 0,0,0,0 };

  unsigned char utf_32_koi8_r[32] = { 0x20,4,0,0, 0x43,4,0,0, 0x41,4,0,0, 0x41,4,0,0,
					0x3a,4,0,0, 0x38,4,0,0, 0x39,4,0,0, 0,0,0,0 };

  unsigned char strcmp_1[8] = { 0x41, 0xc3, 0xb6, 0x41, 0x41, 0 }; 	 /* 5 bytes, 4 chars */
  unsigned char strcmp_2[8] = { 0xc3, 0xb6, 0xc3, 0xb6, 0xc3, 0xb6, 0 }; /* 6 bytes, 3 chars */

  /* ölgemälde (oil painting) */
  unsigned char oil[16] = { 0xc3, 0xb6,'l','g','e','m', 0xc3, 0xa4,'l','d','e'};
  /* Ölgemälde (oil painting) */
  unsigned char Oil[16] = { 0xc3, 0x96,'l','g','e','m', 0xc3, 0xa4,'l','d','e'};
  /* öLGEMÄLDE (oil painting) */
  unsigned char oIL[16] = { 0xc3, 0xb6,'L','G','E','M', 0xc3, 0x84,'L','D','E'};
  /* ÖlGeMäLdE (oil painting) */
  unsigned char OiL[16] = { 0xc3, 0x96,'l','G','e','M', 0xc3, 0xa4,'L','d','E'};
  /* öLgEmÄlDe (oil painting) */
  unsigned char oIl[16] = { 0xc3, 0xb6,'L','g','E','m', 0xc3, 0x84,'l','D','e'};
  /* ÖLGeMÄLDe (oil painting) */
  unsigned char oiL[16] = { 0xc3, 0x96,'L','G','e','M', 0xc3, 0x84,'L','D','e'};
  /* ölgEmäldE (oil painting) */
  unsigned char OIl[16] = { 0xc3, 0xb6,'l','g','E','m', 0xc3, 0xa4,'l','d','E'};

  /* this single byte is represented by 3 bytes in UTF-8 */
  unsigned char cp437[8] = { 0x9e, 0x0 };

  /* 日本語 (Japanese) tests */
  unsigned char japanese_utf8[16] = { 0xE6, 0x97, 0xA5, 0xE6, 0x9C,0xAC,0xE8,0xAA,0x9E,0x0 };
  unsigned char japanese_utf16[16] = { 0xE5,0x65,0x2C,0x67,0x9E,0x8A,0x0, 0x0 };
  unsigned char japanese_utf32[16] = { 0xE5,0x65,0,0,0x2C,0x67,0,0,0x9E,0x8A,0,0,0,0,0,0 };

  /* global copy so the test routines can check for critical errors */
  spwd = pwd;

  /* let the default be these two encodings */
  pwdgen_add_encoding(pwd, ISO_8859_1);
  pwdgen_add_encoding(pwd, UTF_8);

  /* For debugging the test output itself:
  equals(1,1,"pass");
  equals(1,2,"fail");
  
  cmp("foo", "foo", 3, "foo vs foo");
  cmp("foo", "bar", 3, "foo vs bar");
 
  cmp(  "foofoofoofooooooooooooooooooooooooooooo2ooooooo", 
	"foofoofoofoooooo1oooooooooooooooooooooooooooooo", 33, "foo vs foo");
  */

  /* ******************************************************************** */
  printf ("### start ###\n");
  plan_tests(294);
 
  /* now run the individiual tests */

  /* ******************************************************************** */
  /* charset alias names */

  equals (pwdgen_encoding_from_name("UTF-8"), UTF_8, "UTF-8 => UTF_8");
  equals (pwdgen_encoding_from_name("iso-8859-1"), ISO_8859_1, "iso-8859-1 => ISO_8859_1");
  equals (pwdgen_encoding_from_name("TIS-620"), ISO_8859_11, "TIS-620 => ISO_8859_1");
  equals (pwdgen_encoding_from_name("euro"), ISO_8859_15, "euro => ISO_8859_15");
  equals (pwdgen_encoding_from_name("utf-8"), UTF_8, "utf-8 => UTF_8");
  equals (pwdgen_encoding_from_name("utf8"), UTF_8, "utf88 => UTF_8");
  equals (pwdgen_encoding_from_name("foo"), INVALID_ENCODING, "foo => INVALID_ENCODING");

  /* ******************************************************************** */
  /* string creation */

  foo = pwdgen_string(pwd, (unsigned char*)"foo", 3, 255, ISO_8859_1);
  if (1 == non_null (foo, "foo"))
    {
    equals (foo->bytes, 3, "foo has correct length in bytes");
    equals (foo->encoding, ISO_8859_1, "foo has correct encoding");
    equals (foo->characters, 3, "foo's number of characters is 3");
    pwdgen_empty_string(pwd, foo);
    equals (foo->bytes, 0, "0 bytes after empty_string");
    equals (foo->characters, 0, "0 chars after empty_string");
    equals (foo->encoding, ISO_8859_1, "foo has correct encoding");
    pwdgen_free_string(pwd, foo);
    }
  else { skip (3, "Couldn't allocate foo"); }
  
  bar = pwdgen_string(pwd, (unsigned char*)"Müller", 7, 255, UTF_8);
  if (1 == non_null (bar, "bar"))
    {
    equals (bar->bytes, 7, "bar has correct length in bytes");
    equals (bar->encoding, UTF_8, "bar has correct encoding");
    equals (bar->characters, -1, "bar's number of characters is not yet known");
    }
  else { skip (3, "Couldn't allocate bar"); } 

  r_len = strlen("Русский");
  russian = pwdgen_string(pwd, (unsigned char*)"Русский", r_len, 255, UTF_8);
  if (1 == non_null (russian, "russian"))
    {
    equals (russian->bytes, r_len, "russian has correct length in bytes");
    equals (russian->encoding, UTF_8, "russian has correct encoding");
    equals (russian->characters, -1, "russian's number of characters is not yet known");
    }
  else { skip (3, "Couldn't allocate russian"); } 
  
  /* ******************************************************************** */
  /* copy string */

  temp = pwdgen_strcpy(pwd, russian, russian->encoding);
  if (1 == non_null (temp, "temp"))
    {
    equals (temp->bytes, russian->bytes, "temp has correct length in bytes");
    equals (temp->encoding, russian->encoding, "temp has correct encoding");
    equals (temp->characters, russian->characters, "temp's number of characters is not yet known");
    equals (russian->characters, -1, "russian's number of characters is not yet known");
    equals (temp->size, russian->size, "temp's buffer has the same size");
    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); } 

  cmp (russian->content, (char*)russki, 15, "russian is still correct");

  /* ******************************************************************** */
  /* check_string */

  equals (pwdgen_check_string(pwd, bar), 0, "pwdgen_check_string(Müller) works");

  pwdgen_print (pwd, "# # bar: '%s'\n", bar);
  equals (pwdgen_strlen(pwd, bar), 6, "pwdgen_strlen(Müller) is 6 characters");
  equals (russian->characters, -1, "russian's number of characters is not yet known");
  equals (pwdgen_strlen(pwd, russian), 7, "pwdgen_strlen(Русский) is 7 characters");
  pwdgen_print (pwd, "# # bar: '%s'\n", bar);

  equals (pwdgen_check_string(pwd, bar), 0, "pwdgen_check_string(Müller) works");

  /* ******************************************************************** */
  /* convert CP437 to UTF-8 */

  foo = pwdgen_string(pwd, cp437, 1, 255, CP437);
  if (1 == non_null (foo, "cp437"))
    {
    equals (foo->bytes, 1, "foo has correct length in bytes");
    equals (foo->encoding, CP437, "foo has correct encoding");
    equals (foo->characters, 1, "CP437's number of characters is 1");
  
    pwdgen_convert_to_utf8 (pwd, foo);

    equals (foo->bytes, 3, "foo has correct length in bytes");
    equals (foo->encoding, UTF_8, "foo has correct encoding");
    equals (foo->characters, 1, "number of characters is still 1");
    pwdgen_free_string(pwd, foo);
    }

  /* ******************************************************************** */
  /* strcmp() in UTF-8 */

  temp = pwdgen_string(pwd, strcmp_1, 5, 255, UTF_8);
  if (1 == non_null (temp, "strcmp_1"))
    {
    temp_1 = pwdgen_string(pwd, strcmp_2, 6, 255, UTF_8);
    if (1 == non_null (temp, "strcmp_2"))
      {
      rc_cmp = pwdgen_strcmp(pwd, temp, temp_1);
      equals (rc_cmp, -1, " aüaa < üüü");
      rc_cmp = pwdgen_strcmp(pwd, temp_1, temp);
      equals (rc_cmp, 1, " üüü > aüaa");
      rc_cmp = pwdgen_strcmp(pwd, temp_1, temp_1);
      equals (rc_cmp, 0, " üüü == üüü");
      rc_cmp = pwdgen_strcmp(pwd, temp, temp);
      equals (rc_cmp, 0, " aüaa == aüaa");
      pwdgen_free_string(pwd, temp_1);
      }
    pwdgen_free_string(pwd, temp);
    }
  else { skip (2, "Couldn't allocate temp or temp_1"); }
  
  /* ******************************************************************** */
  /* convert UTF-8 to ISO-8859-1 with Umlauts */

  pwdgen_convert_to (pwd, bar, ISO_8859_1, '?');
  equals (pwdgen_strlen(pwd, bar), 6, "pwdgen_strlen(Müller) is 6 characters");
  equals (bar->bytes, 6, "bar has now a length of 6 bytes in ISO-8859-1");

  equals (pwdgen_check_string(pwd, bar), 0, "pwdgen_check_string(Müller) after ISO-8859-1");
  
  cmp (bar->content, (char*)Mueller, 7, "bar converted correctly");

  /* ******************************************************************** */
  /* pwdgen_strcat in different encodings (ISO-8859-1, UTF-8, ISO-88591) */

  /* 'Müller' . 'Русский' . 'Müller' */

  mixed = pwdgen_strcpy(pwd, bar, bar->encoding);
  if (1 == non_null (mixed, "mixed"))
    {
    rc = pwdgen_strcat(pwd, mixed, russian);
    equal_ptr (rc, mixed, "strcat worked");
    if (1 == non_null (rc, "rc"))
      {
      cmp (mixed->content, (char*)tmp, 6+r_len+1, "strcat successfull");

      rc = pwdgen_strcat(pwd, mixed, bar);
      equal_ptr (rc, mixed, "second strcat worked");
      cmp (mixed->content, (char*)tmp, 6+6+r_len+1, "strcat successfull");

      /* **************************************************************** */
      /* convert UTF-8 to ISO-8859-1 with "missing" characters */

      temp = pwdgen_strcpy(pwd, mixed, mixed->encoding);

      pwdgen_convert_to (pwd, mixed, ISO_8859_1, '?');
      cmp (mixed->content, (char*)conv, 6+6+7+1, "convert to ISO-8859-1 with '?'");

      /* ******************************************************************** */
      /* KOI8-R tests */

      if (1 == non_null (temp, "temp"))
	{
        pwdgen_convert_to (pwd, temp, KOI8_R, '?');
        cmp (temp->content, (char*)koi8_r, 6+6+7+1, "convert to KOI8-R with '?'");

        pwdgen_free_string(pwd, temp);
	}
      else { skip (1, "Couldn't strcpy temp"); } 
      }
    else { skip (4, "Couldn't strcat"); } 

    pwdgen_free_string(pwd, mixed);
    }
  else { skip (5, "Couldn't allocate mixed"); } 
  pwdgen_free_string(pwd, bar);

  /* ******************************************************************** */
  /* strcat with the same string */

  bar = pwdgen_string(pwd, (unsigned char*)"Müller", 7, 255, UTF_8);

  rc = pwdgen_strcat(pwd, bar, bar);
  equal_ptr (rc, bar, "strcat worked");
  if (1 == non_null (rc, "rc after strcat"))
    {
    cmp (bar->content, (char*)Mueller2, 6*2, "strcat (bar,bar) successfull");
    equals (bar->bytes, 14, "7+7 == 14");
    equals (bar->characters, 12, "6+6 == 12");
    }
  else { skip (3, "Couldn't do strcat with same string"); } 
  pwdgen_free_string(pwd, bar);

  /* ******************************************************************** */
  /* strcat with first string being 0 bytes long */

  bar = pwdgen_string(pwd, "", 0, 255, ISO_8859_1);
  temp = pwdgen_string(pwd, (unsigned char*)"Müller", 7, 255, UTF_8);

  if ((1 == non_null (bar, "bar after string")) &&
      (1 == non_null (temp, "temp after string")))
    {
    equals (bar->bytes, 0, "0 bytes");
    equals (bar->characters, 0, "0 chars");
    rc = pwdgen_strcat(pwd, bar, temp);
    equal_ptr (rc, bar, "strcat worked");
    if (1 == non_null (rc, "rc after string"))
      {
      equals (rc->bytes, 7, "7 bytes in UTF-8");
      equals (rc->characters, 6, "6 chars in UTF-8");
      equals (rc->encoding, UTF_8, "proper UTF-8");
      }
    else { skip (3, "Couldn't do strcat with 0-byte string"); } 
    }
  else { skip (6, "Couldn't do strcat with 0-byte string"); } 
  if (bar != NULL) { pwdgen_free_string(pwd, bar); }
  if (temp !=NULL) { pwdgen_free_string(pwd, temp); }

  /* ******************************************************************** */
  /* convert_to() on an already UTF-8 string (test that the characters
     stay at -1 and do not get set to the number of bytes */
  bar = pwdgen_string(pwd, (unsigned char*)"Müller", 7, 255, UTF_8);
  if (1 == non_null (bar, "bar after new"))
    {
    equals (bar->characters, -1, "-1 characters");
    pwdgen_convert_to(pwd, bar, UTF_8, '?');
    equals (bar->characters, -1, "still -1 characters");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (2, "Couldn't do strcat"); } 

  /* ******************************************************************** */
  /* strcat with unknown characters in the first string */

  bar = pwdgen_string(pwd, (unsigned char*)"Müller", 7, 255, UTF_8);
  temp = pwdgen_string(pwd, (unsigned char*)"Mueller", 7, 255, ISO_8859_1);

  /* this also tests that a temp. copy of a read-only string is possible
     inside pwdgen_strcat */
  pwdgen_make_string_read_only(temp);

  bar->characters = (size_t) -1;
  rc = pwdgen_strcat(pwd, bar, temp);
  if (1 == non_null (bar, "bar after strcat"))
    {
    cmp (bar->content, (char*)Mueller3, 7*2, "strcat (bar,temp) successfull");
    equals (bar->encoding, UTF_8, "bar is still UTF-8");
    equals (bar->bytes, 14, "7+7 == 14");
    equals (bar->characters, 13, "7+6 == 13");
    equals (bar->flags, 0, "flags(bar) == 0");

    /* test that temp was left unmodified */
    equals (temp->encoding, ISO_8859_1, "temp encoding is not UTF-8");
    equals (temp->bytes, 7, "7 bytes in Mueller");
    equals (temp->characters, 7, "7 characters in Mueller");
    equals (temp->flags, PWDSTR_READ_ONLY, "flags(temp) != 0");
    }
  else { skip (3, "Couldn't do strcat"); } 
  pwdgen_free_string(pwd, bar);
  pwdgen_free_string(pwd, temp);

  /* ******************************************************************** */
  /* KOI8-R tests */

  temp = pwdgen_string(pwd, koi8r, 7, 255, KOI8_R);
  if (1 == non_null (temp, "koi8-r"))
    {
    equals (temp->bytes, 7, "koi8-r has correct length in bytes");
    equals (temp->encoding, KOI8_R, "koi8-r has correct encoding");
    equals (temp->characters, 7, "koi8-r's number of characters is 7");
  
    pwdgen_free_string(pwd, temp);
    }
  else { skip (3, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* UTF-16 tests */

  temp = pwdgen_string(pwd, utf_16, (6+7)*2, 255, UTF_16);
  if (1 == non_null (temp, "utf_16"))
    {
    equals (temp->bytes, 26, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 6+7, "UTF-16's number of characters is 13");
 
    /* nuke the character count */   
    temp->characters = (size_t)-1;
    /* and re-count them */
    equals (pwdgen_strlen(pwd, temp), 6+7, "pwdgen_strlen(UTF_16) is 13 characters");
 
    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* UTF-16 to UTF-8 tests */

  /* Müller */
  temp = pwdgen_string(pwd, utf_16_iso_8859_1, 6*2, 255, UTF_16);
  if (1 == non_null (temp, "utf_16_to_utf_8"))
    {
    equals (temp->bytes, 12, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 6, "UTF-16's number of characters is 6");

    pwdgen_convert (pwd, temp, UTF_8);

    equals (temp->bytes, 7, "utf-8 has correct length in bytes after convert");
    equals (temp->encoding, UTF_8, "UTF-8 has correct encoding");
    equals (temp->characters, 6, "UTF-8's number of characters is still 6");

    cmp (temp->content, (char*)Mueller_utf8, 8, "UTF-16 => UTF-8 worked");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (7, "Couldn't allocate temp"); } 

  /* Russkij */
  temp = pwdgen_string(pwd, utf_16_koi8_r, 14, 255, UTF_16);
  if (1 == non_null (temp, "utf_16_to_utf_8 russki"))
    {
    equals (temp->bytes, 14, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 7, "UTF-16's number of characters is 6");

    pwdgen_convert (pwd, temp, UTF_8);

    equals (temp->bytes, 14, "utf-8 has correct length in bytes after convert");
    equals (temp->encoding, UTF_8, "UTF-8 has correct encoding");
    equals (temp->characters, 7, "UTF-8's number of characters is still 6");

    cmp (temp->content, (char*)russki, 15, "UTF-16 => UTF-8 worked");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (7, "Couldn't allocate temp"); } 

  /* 日本語 (Japanese) tests */
  temp = pwdgen_string(pwd, japanese_utf16, 6, 255, UTF_16);
  if (1 == non_null (temp, "utf_16_to_utf_8 japanese"))
    {
    equals (temp->bytes, 6, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 3, "UTF-16's number of characters is 3");

    pwdgen_convert (pwd, temp, UTF_8);

    equals (temp->bytes, 9, "utf-8 has correct length in bytes after convert");
    equals (temp->encoding, UTF_8, "UTF-8 has correct encoding");
    equals (temp->characters, 3, "UTF-8's number of characters is still 3");

    cmp (temp->content, (char*)japanese_utf8, 10, "UTF-16 => UTF-8 worked");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (7, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* single-byte to UTF-16 conversion tests */
  
  temp = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (temp, "Mueller in ISO-8859-1"))
    {
    pwdgen_convert (pwd, temp, UTF_16);
    equals (temp->bytes, 12, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 6, "UTF-16's number of characters is 6");
    cmp (temp->content, (char*)utf_16_iso_8859_1, 6*2, "ISO-8859-1 converted to UTF-16");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); } 

  temp = pwdgen_string(pwd, koi8r, 7, 255, KOI8_R);
  if (1 == non_null (temp, "Русский in KOI8-R"))
    {
    /* test pwdgen_convert_to_utf16, too */
    pwdgen_convert_to_utf16 (pwd, temp);
    equals (temp->bytes, 14, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 7, "UTF-16's number of characters is 7");
    cmp (temp->content, (char*)utf_16_koi8_r, 7*2, "KOI8-R converted to UTF-16");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* UTF-8 to UTF-16 tests */

  temp = pwdgen_string(pwd, russki, r_len, 255, UTF_8);
  if (1 == non_null (temp, "Русский from UTF-8 to UTF-16"))
    {
    /* test pwdgen_convert_to_utf16, too */
    pwdgen_convert_to_utf16 (pwd, temp);
    equals (temp->bytes, 14, "utf-16 has correct length in bytes");
    equals (temp->encoding, UTF_16, "UTF-16 has correct encoding");
    equals (temp->characters, 7, "UTF-16's number of characters is 7");
    cmp (temp->content, (char*)utf_16_koi8_r, 7*2, "UTF-8 converted to UTF-16");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* single-byte to UTF-32 conversion tests */
  
  temp = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (temp, "Mueller in ISO-8859-1"))
    {
    pwdgen_convert (pwd, temp, UTF_32);
    equals (temp->bytes, 24, "utf-32 has correct length in bytes");
    equals (temp->encoding, UTF_32, "UTF-32 has correct encoding");
    equals (temp->characters, 6, "UTF-32's number of characters is 6");
    cmp (temp->content, (char*)utf_32_iso_8859_1, 6*2, "ISO-8859-1 converted to UTF-32");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); }
 
  temp = pwdgen_string(pwd, koi8r, 7, 255, KOI8_R);
  if (1 == non_null (temp, "Русский in KOI8-R"))
    {
    /* test pwdgen_convert_to_utf32, too */
    pwdgen_convert_to_utf32 (pwd, temp);
    equals (temp->bytes, 28, "utf-32 has correct length in bytes");
    equals (temp->encoding, UTF_32, "UTF-32 has correct encoding");
    equals (temp->characters, 7, "UTF-32's number of characters is 6");
    cmp (temp->content, (char*)utf_32_koi8_r, 7*4, "KOI8-R converted to UTF-32");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* single-byte to ASCII conversion tests */
  
  temp = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (temp, "Mueller in ISO-8859-1"))
    {
    pwdgen_convert (pwd, temp, ASCII);
    equals (temp->bytes, 6, "ASCII has correct length in bytes");
    equals (temp->encoding, ASCII, "ASCII has correct encoding");
    equals (temp->characters, 6, "ASCII's number of characters is 6");
    cmp (temp->content, (char*)Mueller_ascii, 6, "ISO-8859-1 converted to ASCII");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); }
 
  temp = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (temp, "Mueller in ISO-8859-1"))
    {
    /* test with replace being 0, meaning ü gets dropped */
    pwdgen_convert_to (pwd, temp, ASCII, 0);
    equals (temp->bytes, 5, "ASCII has correct length in bytes");
    equals (temp->encoding, ASCII, "ASCII has correct encoding");
    equals (temp->characters, 5, "ASCII's number of characters is 5");
    cmp (temp->content, (char*)Mller_ascii, 5, "ISO-8859-1 converted to ASCII");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (4, "Couldn't allocate temp"); }

  /* ******************************************************************** */
  /* UTF-32 tests */

  temp = pwdgen_string(pwd, utf_32, (6+7)*4, 255, UTF_32);
  if (1 == non_null (temp, "utf_32"))
    {
    equals (temp->bytes, 52, "utf-32 has correct length in bytes");
    equals (temp->encoding, UTF_32, "UTF-32 has correct encoding");
    equals (temp->characters, 6+7, "UTF-32's number of characters is 13");
 
    /* nuke the character count */   
    temp->characters = (size_t)-1;
    /* and re-count them */
    equals (pwdgen_strlen(pwd, temp), 6+7, "pwdgen_strlen(UTF_32) is 13 characters");
 
    pwdgen_free_string(pwd, temp);
    }
  else { skip (3, "Couldn't allocate temp"); } 

  /* 日本語 (Japanese) UTF-32 tests */
  temp = pwdgen_string(pwd, japanese_utf32, 12, 255, UTF_32);
  if (1 == non_null (temp, "utf_32_to_utf_8 japanese"))
    {
    equals (temp->bytes, 12, "utf-32 has correct length in bytes");
    equals (temp->encoding, UTF_32, "UTF-32 has correct encoding");
    equals (temp->characters, 3, "UTF-32's number of characters is 3");

    pwdgen_convert (pwd, temp, UTF_8);

    equals (temp->bytes, 9, "utf-8 has correct length in bytes after convert");
    equals (temp->encoding, UTF_8, "UTF-8 has correct encoding");
    equals (temp->characters, 3, "UTF-8's number of characters is still 3");

    cmp (temp->content, (char*)japanese_utf8, 10, "UTF-32 => UTF-8 worked");

    pwdgen_free_string(pwd, temp);
    }
  else { skip (7, "Couldn't allocate temp"); } 

  /* ******************************************************************** */
  /* uc, lc, and uc_first in ISO-8859-1 */

  bar = pwdgen_string(pwd, mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (bar, "bar"))
    {
    equals (bar->bytes, 6, "bar has correct length in bytes");
    rc_cmp = pwdgen_uc(pwd, bar);
    equals (rc_cmp, 1, "string changed to UPPERCASE");
    equals (bar->bytes, 6, "bar has correct length in bytes");
    equals (bar->characters, 6, "bar has correct length in characters");
    cmp (bar->content, (char*)MUELLER, 6, "MÜLLER");

    rc_cmp = pwdgen_lc(pwd, bar);
    equals (rc_cmp, 1, "string changed to lowercase");
    equals (bar->bytes, 6, "bar has correct length in bytes");
    equals (bar->characters, 6, "bar has correct length in characters");
    cmp (bar->content, (char*)mueller, 6, "müller");

    rc_cmp = pwdgen_uc_first(pwd, bar);
    equals (rc_cmp, 1, "string changed to Uppercase");
    equals (bar->bytes, 6, "bar has correct length in bytes");
    equals (bar->characters, 6, "bar has correct length in characters");
    cmp (bar->content, (char*)Mueller, 6, "Müller");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (3, "Couldn't allocate bar"); } 

  /* ******************************************************************** */
  /* uc, lc, uc_odd, uc_even, lc_first, uc_first, uc_vowels, uc_consonants
     in UTF-8 */

  bar = pwdgen_string(pwd, oil, 11, 255, UTF_8);
  if (1 == non_null (bar, "oil"))
    {
    equals (bar->bytes, 11, "Oelgemaelde has correct length in bytes");

    rc_cmp = pwdgen_uc_first(pwd, bar);
    equals (rc_cmp, 1, "string changed to Uppercase");
    equals (bar->bytes, 11, "bar has still correct length in bytes");
    equals (bar->characters, -1, "bar has correct length in characters");

    cmp (bar->content, (char*)Oil, 11, "Ölgemälde");

    rc_cmp = pwdgen_lc_first(pwd, bar);
    equals (rc_cmp, 1, "string changed to lOWERCASE");
    equals (bar->bytes, 11, "bar has still correct length in bytes");
    equals (bar->characters, -1, "bar has correct length in characters");

    cmp (bar->content, (char*)oIL, 11, "öLGEMÄLDE");

    rc_cmp = pwdgen_uc_odd(pwd, bar);
    equals (rc_cmp, 1, "uc_odd: string changed to öLgEmÄlDe");
    equals (bar->bytes, 11, "bar has still correct length in bytes");
    equals (bar->characters, -1, "bar has correct length in characters");

    cmp (bar->content, (char*)oIl, 11, "öLgEmÄlDe");

    rc_cmp = pwdgen_uc_even(pwd, bar);
    equals (rc_cmp, 1, "uc_even: string changed to ÖlGeMäLdE");
    equals (bar->bytes, 11, "bar has still correct length in bytes");
    equals (bar->characters, -1, "bar has correct length in characters");

    cmp (bar->content, (char*)OiL, 11, "ÖlGeMäLdE");

    rc_cmp = pwdgen_uc_consonants(pwd, bar);
    equals (rc_cmp, 1, "uc_consonants: string changed to ÖLGeMÄLDe");
    equals (bar->bytes, 11, "bar has still correct length in bytes");
    equals (bar->characters, -1, "bar has correct length in characters");

    cmp (bar->content, (char*)oiL, 11, "ÖLGeMÄLDe");

    rc_cmp = pwdgen_uc_vowels(pwd, bar);
    equals (rc_cmp, 1, "uc_vowels: string changed to ölgEmäldE");
    equals (bar->bytes, 11, "bar has still correct length in bytes");
    equals (bar->characters, -1, "bar has correct length in characters");

    cmp (bar->content, (char*)OIl, 11, "ölgEmäldE");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (21, "Couldn't allocate bar"); }

  /* ******************************************************************** */
  /* parse locale tests */

  equals (pwdgen_parse_locale("C"), ASCII, "C");
  equals (pwdgen_parse_locale(""), ASCII, "");
  equals (pwdgen_parse_locale("de_DE.UTF-8@euro"), ISO_8859_15, "de_DE.UTF-8@euro");
  equals (pwdgen_parse_locale("de_DE.UTF-8@euro UTF-8"), UTF_8, "de_DE.UTF-8@euro UTF-8");
  equals (pwdgen_parse_locale("en_EN.UTF-8"), UTF_8, "en_EN.UTF-8");
  equals (pwdgen_parse_locale("English_United States.1250"), CP1250, "English_United States.1250");
  equals (pwdgen_parse_locale("German_Germany.1252"), CP1252, "German_Germany.1252");
  equals (pwdgen_parse_locale("LC_CTYPE=en_US.UTF-8;LC_NUMERIC=C;LC_TIME=C;LC_COLLATE=C;"),
				UTF_8, "LC_CTYPE=...");

  /* ******************************************************************** */
  /* string reverse tests */

  bar = pwdgen_string(pwd, oil, 0, 255, UTF_8);
  if (1 == non_null (bar, "oil"))
    {
    equals (bar->bytes, 0, "'' for reverse has zero bytes");
    
    rc_cmp = pwdgen_strrev(pwd, bar);
    equals (rc_cmp, 0, "no bytes, nothing to change");
    pwdgen_free_string(pwd, bar);
    }
  else { skip (2, "Couldn't allocate bar"); }

  bar = pwdgen_string(pwd, anna, 4, 255, ISO_8859_1);
  if (1 == non_null (bar, "anna in ISO-8859-1"))
    {
    equals (bar->bytes, 4, "anna has 4 bytes in ISO-8859-1");
    
    rc_cmp = pwdgen_strrev(pwd, bar);
    equals (rc_cmp, 0, "4 bytes unchanged");
    cmp (bar->content, (char*)anna, 4, "AnnA => AnnA");
    pwdgen_free_string(pwd, bar);
    }
  else { skip (3, "Couldn't allocate bar"); }

  bar = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (bar, "Mueller in ISO-8859-1"))
    {
    equals (bar->bytes, 6, "Müller has 6 bytes in ISO-8859-1");
    
    rc_cmp = pwdgen_strrev(pwd, bar);
    equals (rc_cmp, 1, "6 bytes reversed");
    cmp (bar->content, (char*)rellueM, 6, "rellüM");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (3, "Couldn't allocate bar"); }

  bar = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (bar, "Mueller in ISO-8859-1"))
    {
    equals (bar->bytes, 6, "Müller has 6 bytes in ISO-8859-1");
    pwdgen_convert_to_utf8(pwd, bar);
    equals (bar->bytes, 7, "Müller has 7 bytes in UTF-8");

    rc_cmp = pwdgen_strrev(pwd, bar);
    equals (rc_cmp, 1, "7 bytes reversed");
    cmp (bar->content, (char*)rellueM_utf8, 7, "rellüM");
    pwdgen_free_string(pwd, bar);
    }
  else { skip (4, "Couldn't allocate bar"); }

  /* reverse in UTF-16 */
  bar = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (bar, "Mueller in ISO-8859-1"))
    {
    equals (bar->bytes, 6, "Müller has 6 bytes in ISO-8859-1");
    pwdgen_convert_to_utf16(pwd, bar);
    equals (bar->bytes, 12, "Müller has 12 bytes in UTF-16");

    rc_cmp = pwdgen_strrev(pwd, bar);
    equals (rc_cmp, 1, "12 bytes reversed");
    cmp (bar->content, (char*)rellueM_utf16, 12, "rellüM");
    pwdgen_free_string(pwd, bar);
    }
  else { skip (4, "Couldn't allocate bar"); }

  /* reverse in UTF-32 */
  bar = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (bar, "Mueller in ISO-8859-1"))
    {
    equals (bar->bytes, 6, "Müller has 6 bytes in ISO-8859-1");
    pwdgen_convert_to_utf32(pwd, bar);
    equals (bar->bytes, 24, "Müller has 24 bytes in UTF-32");

    rc_cmp = pwdgen_strrev(pwd, bar);
    equals (rc_cmp, 1, "24 bytes reversed");
    cmp (bar->content, (char*)rellueM_utf32, 24, "rellüM");
    pwdgen_free_string(pwd, bar);
    }
  else { skip (4, "Couldn't allocate bar"); }
  
  /* ******************************************************************** */
  /* truncate string */

  /* in ISO-8859-1 */
  bar = pwdgen_string(pwd, Mueller, 6, 255, ISO_8859_1);
  if (1 == non_null (bar, "truncate_string"))
    {
    equals (bar->bytes, 6, "Müller has 6 bytes in ISO-8859-1");
    equals (bar->characters, 6, "Müller has 6 characters in ISO-8859-1");

    i = pwdgen_truncate_string(pwd, bar, 4);
    equals (i, 1, "Müller got truncated to 4 characters");
    equals (bar->bytes, 4, "'Müll' has 4 bytes in ISO-8859-1");
    equals (bar->characters, 4, "'Müll' has 4 characters in ISO-8859-1");
    cmp (bar->content, (char*)Muell, 5, "compare with zero-termination");

    i = pwdgen_truncate_string(pwd, bar, 2);
    equals (i, 1, "Müller got truncated to 2 characters");
    equals (bar->bytes, 2, "Mü has 2 bytes in ISO-8859-1");
    equals (bar->characters, 2, "Mü has 2 characters in ISO-8859-1");
    cmp (bar->content, (char*)Mue, 3, "compare with zero-termination");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (11, "Couldn't allocate bar"); }

  /* truncate in UTF-8 */
  bar = pwdgen_string(pwd, Mueller_utf8, 7, 255, UTF_8);
  if (1 == non_null (bar, "truncate_string"))
    {
    equals (bar->bytes, 7, "Müller has 7 bytes in UTF-8");
    i = pwdgen_check_string(pwd, bar);
    equals (bar->characters, 6, "Müller has 6 characters in UTF-8");

    i = pwdgen_truncate_string(pwd, bar, 4);
    equals (i, 1, "Müller got truncated to 4 characters");
    equals (bar->bytes, 5, "'Müll' has 5 bytes in UTF-8");
    equals (bar->characters, 4, "'Müll' has 4 characters in UTF-8");
    cmp (bar->content, (char*)Muell_utf8, 6, "compare with zero-termination");

    i = pwdgen_truncate_string(pwd, bar, 1);
    equals (i, 1, "Müll got truncated to 1 characters");
    equals (bar->bytes, 1, "M has 1 bytes in UTF-8");
    equals (bar->characters, 1, "M has 1 characters in UTF-8");
    cmp (bar->content, (char*)M_utf8, 2, "compare with zero-termination");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (11, "Couldn't allocate bar"); }

  /* truncate in UTF-32 */
  bar = pwdgen_string(pwd, utf_32, 12 * 4, 255, UTF_32);
  if (1 == non_null (bar, "truncate_string UTF-32"))
    {
    equals (bar->bytes, 12*4, "MüllerRusskij has 12*4 bytes in UTF-32");
    i = pwdgen_check_string(pwd, bar);
    equals (bar->characters, 12, "MüllerRusskij has 12 characters in UTF-32");

    i = pwdgen_truncate_string(pwd, bar, 4);
    equals (i, 1, "Müller got truncated to 4 characters");
    equals (bar->bytes, 4*4, "'Müll' has 16 bytes in UTF-32");
    equals (bar->characters, 4, "'Müll' has 4 characters in UTF-32");
    cmp (bar->content, (char*)utf_32_truncated, 20, "compare with zero-termination");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (7, "Couldn't allocate bar"); }

  /* truncate in UTF-16 */
  bar = pwdgen_string(pwd, utf_16, 12 * 2, 255, UTF_16);
  if (1 == non_null (bar, "truncate_string UTF-16"))
    {
    equals (bar->bytes, 12*2, "MüllerRusskij has 24 bytes in UTF-16");
    i = pwdgen_check_string(pwd, bar);
    equals (bar->characters, 12, "MüllerRusskij has 12 characters in UTF-16");

    i = pwdgen_truncate_string(pwd, bar, 4);
    equals (i, 1, "Müller got truncated to 4 characters");
    equals (bar->bytes, 4*2, "'Müll' has 8 bytes in UTF-16");
    equals (bar->characters, 4, "'Müll' has 4 characters in UTF-16");
    cmp (bar->content, (char*)utf_16_truncated, 10, "compare with zero-termination");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (7, "Couldn't allocate bar"); }

  /* ******************************************************************** */
  /* strdup() */

  bar = pwdgen_string(pwd, tmp, 28, 255, UTF_8);
  if (1 == non_null (bar, "strdup"))
    {
    temp = pwdgen_new_string(pwd, 16, ISO_8859_1);
    if (1 == non_null (temp, "strdup"))
      {
      equals (bar->bytes, 28, "Müller has 27 bytes in UTF-8");
      equals (temp->bytes, 0, "temp is empty");
      equals (temp->encoding, ISO_8859_1, "temp is empty");
      equals (temp->size, 16, "temp is empty");

      /* temp <- bar */
      pwdgen_strdup(pwd, temp, bar);
      equals (temp->encoding, UTF_8, "temp is now UTF-8");
      equals (temp->encoding, bar->encoding, "temp is now UTF-8");
      equals (temp->bytes, 28, "temp has now 28 bytes");
      equals (temp->bytes, bar->bytes, "temp has now 28 bytes");
      equals (temp->size, 44, "temp has now 32 bytes (28 + 16)");
      cmp (temp->content, tmp, 29, "temp has now 28+1 (zero) bytes");

      pwdgen_free_string(pwd, temp);
      }
    pwdgen_free_string(pwd, bar);
    }
  else { skip (11, "Couldn't allocate bar"); }

  /* ******************************************************************** */
  /* string buffer resize tests */

  /* truncate through string buffer resize in UTF-32 */
  bar = pwdgen_string(pwd, utf_32, 12 * 4, 255, UTF_32);
  if (1 == non_null (bar, "truncate_string UTF-32"))
    {
    equals (bar->bytes, 12*4, "MüllerRusskij has 12*4 bytes in UTF-32");

    pwdgen_resize_string(pwd, bar, 12*4);
    equals (bar->bytes, 44, "44 bytes + 0 bytes zero-termination");
    equals (bar->size, 48, "minimum 46 bytes, but padded to 48 bytes");

    pwdgen_free_string(pwd, bar);
    }
  else { skip (4, "Couldn't allocate bar"); }

  /* ******************************************************************** */
  printf ("### end ###\n");

  return PWD_INIT_OK;		/* init was okay */
  }

int dofunction( const struct ssPWD *pwd )
  {
  hexdump_string("Current", pwd->cur);

  return PWD_FAIL;					/* not found yet */
  }

