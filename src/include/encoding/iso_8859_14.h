/*!
 * @file
 * @ingroup workerframeenc
 * @brief Dicop Project - Character encoding tables for ISO-8859-14
 *  
 * @english
 *
 * <b>This file has been automatically generated by <code>build/gen_tables.pl</code>.
 * Do not edit, all changes are lost!</b>
 *
 * @endenglish
 *
 * @deutsch
 *
 * <b>Diese Datei wurde automatisch generiert von <code>build/gen_tables.pl</code>.
 * Hier nicht editieren, da alle &Auml;nderungen verloren gehen!</b>
 *
 * @enddeutsch
 *
 * Used: Perl v5.008008, Encode v2.23
 *  
 * @version 0.02 
 * @date Tue Aug 28 14:00:50 2007
 *  
 * @copydoc copyrighttext
*/

#ifndef DICOP_ISO_8859_14_H
#define DICOP_ISO_8859_14_H

/** Table to convert Unicode to ISO-8859-14 */
unsigned long encoding_iso_8859_14[77] = {
  38,
  161, 0x0, 2, 0x1e02, 1, 0xa3, 2, 0x10a, 1, 0x1e0a, 1, 0xa7, 
  1, 0x1e80, 1, 0xa9, 1, 0x1e82, 1, 0x1e0b, 1, 0x1ef2, 2, 0xad, 1, 0x178, 2, 0x1e1e, 
  2, 0x120, 2, 0x1e40, 1, 0xb6, 1, 0x1e56, 1, 0x1e81, 1, 0x1e57, 1, 0x1e83, 1, 0x1e60, 
  1, 0x1ef3, 2, 0x1e84, 1, 0x1e61, 16, 0xc0, 1, 0x174, 6, 0xd1, 1, 0x1e6a, 6, 0xd8, 
  1, 0x176, 17, 0xdf, 1, 0x175, 6, 0xf1, 1, 0x1e6b, 6, 0xf8, 1, 0x177, 1, 0xff
  };

/** Table to convert ISO-8859-14 to UTF-8 */
unsigned long enc_iso_8859_14[257] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 
  0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 
  0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80c2, 0x81c2, 
  0x82c2, 0x83c2, 0x84c2, 0x85c2, 0x86c2, 0x87c2, 0x88c2, 0x89c2, 0x8ac2, 0x8bc2, 
  0x8cc2, 0x8dc2, 0x8ec2, 0x8fc2, 0x90c2, 0x91c2, 0x92c2, 0x93c2, 0x94c2, 0x95c2, 
  0x96c2, 0x97c2, 0x98c2, 0x99c2, 0x9ac2, 0x9bc2, 0x9cc2, 0x9dc2, 0x9ec2, 0x9fc2, 
  0xa0c2, 0x82b8e1, 0x83b8e1, 0xa3c2, 0x8ac4, 0x8bc4, 0x8ab8e1, 0xa7c2, 0x80bae1, 
  0xa9c2, 0x82bae1, 0x8bb8e1, 0xb2bbe1, 0xadc2, 0xaec2, 0xb8c5, 0x9eb8e1, 0x9fb8e1, 
  0xa0c4, 0xa1c4, 0x80b9e1, 0x81b9e1, 0xb6c2, 0x96b9e1, 0x81bae1, 0x97b9e1, 0x83bae1, 
  0xa0b9e1, 0xb3bbe1, 0x84bae1, 0x85bae1, 0xa1b9e1, 0x80c3, 0x81c3, 0x82c3, 0x83c3, 
  0x84c3, 0x85c3, 0x86c3, 0x87c3, 0x88c3, 0x89c3, 0x8ac3, 0x8bc3, 0x8cc3, 0x8dc3, 
  0x8ec3, 0x8fc3, 0xb4c5, 0x91c3, 0x92c3, 0x93c3, 0x94c3, 0x95c3, 0x96c3, 0xaab9e1, 
  0x98c3, 0x99c3, 0x9ac3, 0x9bc3, 0x9cc3, 0x9dc3, 0xb6c5, 0x9fc3, 0xa0c3, 0xa1c3, 
  0xa2c3, 0xa3c3, 0xa4c3, 0xa5c3, 0xa6c3, 0xa7c3, 0xa8c3, 0xa9c3, 0xaac3, 0xabc3, 
  0xacc3, 0xadc3, 0xaec3, 0xafc3, 0xb5c5, 0xb1c3, 0xb2c3, 0xb3c3, 0xb4c3, 0xb5c3, 
  0xb6c3, 0xabb9e1, 0xb8c3, 0xb9c3, 0xbac3, 0xbbc3, 0xbcc3, 0xbdc3, 0xb7c5, 0xbfc3, 
  3
  };

/** Table to convert ISO-8859-14 to Unicode codepoints */
unsigned long cp_iso_8859_14[256] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 
  114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 
  129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 
  144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 
  159, 160, 7682, 7683, 163, 266, 267, 7690, 167, 7808, 169, 7810, 7691, 7922, 
  173, 174, 376, 7710, 7711, 288, 289, 7744, 7745, 182, 7766, 7809, 7767, 7811, 
  7776, 7923, 7812, 7813, 7777, 192, 193, 194, 195, 196, 197, 198, 199, 200, 
  201, 202, 203, 204, 205, 206, 207, 372, 209, 210, 211, 212, 213, 214, 7786, 
  216, 217, 218, 219, 220, 221, 374, 223, 224, 225, 226, 227, 228, 229, 230, 
  231, 232, 233, 234, 235, 236, 237, 238, 239, 373, 241, 242, 243, 244, 245, 
  246, 7787, 248, 249, 250, 251, 252, 253, 375, 255
  };

/** Table to convert one character to UPPERCASE */
unsigned char uc_iso_8859_14[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 
  0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 
  0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 
  0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 
  0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 
  0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa1, 0xa3, 0xa4, 0xa4, 0xa6, 0xa7, 0xa8, 
  0xa9, 0xaa, 0xa6, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb0, 0xb2, 0xb2, 0xb4, 0xb4, 
  0xb6, 0xb7, 0xa8, 0xb7, 0xaa, 0xbb, 0xac, 0xbd, 0xbd, 0xbb, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 
  0xdd, 0xde, 0x53, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 
  0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 
  0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xaf
  };

/** Table to convert one character to lowercase */
unsigned char lc_iso_8859_14[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 
  0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 
  0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 
  0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 
  0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 
  0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa2, 0xa2, 0xa3, 0xa5, 0xa5, 0xab, 0xa7, 0xb8, 
  0xa9, 0xba, 0xab, 0xbc, 0xad, 0xae, 0xff, 0xb1, 0xb1, 0xb3, 0xb3, 0xb5, 0xb5, 
  0xb6, 0xb9, 0xb8, 0xb9, 0xba, 0xbf, 0xbc, 0xbe, 0xbe, 0xbf, 0xe0, 0xe1, 0xe2, 
  0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 
  0xfd, 0xfe, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 
  0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
  };

#endif
