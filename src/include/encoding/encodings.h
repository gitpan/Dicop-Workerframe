/*!
 * @file
 * @ingroup workerframe
 * @brief Dicop Project - Definition of character encoding constants and tables
 * 
 * @copydoc copyrighttext
 *  
 * This file has been automatically generated. Do not edit!
 *  
 * @version 0.01 
 * @date Tue Apr 15 15:13:25 2008
 * @author Bundesamt f&uuml;r Sicherheit in der Informationstechnik (BSI)
*/

#ifndef DICOP_ENCODINGS_H
#define DICOP_ENCODINGS_H

#define WCHAR_T UTF_16
#define TIS_620 ISO_8859_11
#define Macintosh MacRoman

/* define the known encodings as constants */
enum eEncodings {
  ASCII = 0,
  ISO_8859_1,
  ISO_8859_2,
  ISO_8859_3,
  ISO_8859_4,
  ISO_8859_5,
  ISO_8859_6,
  ISO_8859_7,
  ISO_8859_8,
  ISO_8859_9,
  ISO_8859_10,
  ISO_8859_11,
  ISO_8859_13,
  ISO_8859_14,
  ISO_8859_15,
  ISO_8859_16,
  KOI8_R,
  KOI8_U,
  CP437,
  CP737,
  CP775,
  CP850,
  CP852,
  CP855,
  CP857,
  CP858,
  CP860,
  CP861,
  CP862,
  CP863,
  CP865,
  CP866,
  CP869,
  CP874,
  CP1250,
  CP1251,
  CP1252,
  CP1253,
  CP1254,
  CP1255,
  CP1256,
  CP1257,
  CP1258,
  MacRoman,
  MacCentralEurRoman,
  MacIcelandic,
  MacCroatian,
  MacRomanian,
  MacRumanian,
  MacCyrillic,
  MacUkrainian,
  MacGreek,
  MacTurkish,
  MacArabic,
  MacFarsi,
  MacSami,
  MacHebrew,
  UTF_8,
  UTF_16,
  UTF_16BE,
  UTF_16LE,
  UTF_32,
  UTF_32BE,
  UTF_32LE,
  EUC_KISX0213,
  SHIFT_JISX0213,
  SHIFT_JIS,
  HZ,
  GBK,
  GB18030,
  CP950,
  BIG5_HKSCS,
  INVALID_ENCODING  };

#endif
