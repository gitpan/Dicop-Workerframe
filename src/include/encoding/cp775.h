/**
 * @file
 * @ingroup workerframeenc
 * @brief Dicop Project - Character encoding tables for CP775
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
 * Used: Perl v5.010000, Encode v2.23
 *  
 * @version 0.02 
 * @date Tue Apr 15 15:13:23 2008
 *  
 * @copydoc copyrighttext
*/

#ifndef DICOP_CP775_H
#define DICOP_CP775_H

/** Table to convert Unicode to CP775 */
unsigned long encoding_cp775[243] = {
  121,
  128, 0x0, 1, 0x106, 1, 0xfc, 1, 0xe9, 1, 0x101, 1, 0xe4, 1, 0x123, 
  1, 0xe5, 1, 0x107, 1, 0x142, 1, 0x113, 2, 0x156, 1, 0x12b, 1, 0x179, 2, 0xc4, 
  1, 0xc9, 1, 0xe6, 1, 0xc6, 1, 0x14d, 1, 0xf6, 1, 0x122, 1, 0xa2, 2, 0x15a, 
  1, 0xd6, 1, 0xdc, 1, 0xf8, 1, 0xa3, 1, 0xd8, 1, 0xd7, 1, 0xa4, 1, 0x100, 1, 0x12a, 
  1, 0xf3, 2, 0x17b, 1, 0x17a, 1, 0x201d, 1, 0xa6, 1, 0xa9, 1, 0xae, 1, 0xac, 
  1, 0xbd, 1, 0xbc, 1, 0x141, 1, 0xab, 1, 0xbb, 3, 0x2591, 1, 0x2502, 1, 0x2524, 
  1, 0x104, 1, 0x10c, 1, 0x118, 1, 0x116, 1, 0x2563, 1, 0x2551, 1, 0x2557, 1, 0x255d, 
  1, 0x12e, 1, 0x160, 1, 0x2510, 1, 0x2514, 1, 0x2534, 1, 0x252c, 1, 0x251c, 
  1, 0x2500, 1, 0x253c, 1, 0x172, 1, 0x16a, 1, 0x255a, 1, 0x2554, 1, 0x2569, 
  1, 0x2566, 1, 0x2560, 1, 0x2550, 1, 0x256c, 1, 0x17d, 1, 0x105, 1, 0x10d, 1, 0x119, 
  1, 0x117, 1, 0x12f, 1, 0x161, 1, 0x173, 1, 0x16b, 1, 0x17e, 1, 0x2518, 1, 0x250c, 
  1, 0x2588, 1, 0x2584, 1, 0x258c, 1, 0x2590, 1, 0x2580, 1, 0xd3, 1, 0xdf, 1, 0x14c, 
  1, 0x143, 1, 0xf5, 1, 0xd5, 1, 0xb5, 1, 0x144, 2, 0x136, 2, 0x13b, 1, 0x146, 
  1, 0x112, 1, 0x145, 1, 0x2019, 1, 0xad, 1, 0xb1, 1, 0x201c, 1, 0xbe, 1, 0xb6, 
  1, 0xa7, 1, 0xf7, 1, 0x201e, 1, 0xb0, 1, 0x2219, 1, 0xb7, 1, 0xb9, 1, 0xb3, 
  1, 0xb2, 1, 0x25a0, 1, 0xa0
  };

/** Table to convert CP775 to UTF-8 */
unsigned long enc_cp775[257] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 
  0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 
  0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x86c4, 0xbcc3, 
  0xa9c3, 0x81c4, 0xa4c3, 0xa3c4, 0xa5c3, 0x87c4, 0x82c5, 0x93c4, 0x96c5, 0x97c5, 
  0xabc4, 0xb9c5, 0x84c3, 0x85c3, 0x89c3, 0xa6c3, 0x86c3, 0x8dc5, 0xb6c3, 0xa2c4, 
  0xa2c2, 0x9ac5, 0x9bc5, 0x96c3, 0x9cc3, 0xb8c3, 0xa3c2, 0x98c3, 0x97c3, 0xa4c2, 
  0x80c4, 0xaac4, 0xb3c3, 0xbbc5, 0xbcc5, 0xbac5, 0x9d80e2, 0xa6c2, 0xa9c2, 0xaec2, 
  0xacc2, 0xbdc2, 0xbcc2, 0x81c5, 0xabc2, 0xbbc2, 0x9196e2, 0x9296e2, 0x9396e2, 
  0x8294e2, 0xa494e2, 0x84c4, 0x8cc4, 0x98c4, 0x96c4, 0xa395e2, 0x9195e2, 0x9795e2, 
  0x9d95e2, 0xaec4, 0xa0c5, 0x9094e2, 0x9494e2, 0xb494e2, 0xac94e2, 0x9c94e2, 
  0x8094e2, 0xbc94e2, 0xb2c5, 0xaac5, 0x9a95e2, 0x9495e2, 0xa995e2, 0xa695e2, 
  0xa095e2, 0x9095e2, 0xac95e2, 0xbdc5, 0x85c4, 0x8dc4, 0x99c4, 0x97c4, 0xafc4, 
  0xa1c5, 0xb3c5, 0xabc5, 0xbec5, 0x9894e2, 0x8c94e2, 0x8896e2, 0x8496e2, 0x8c96e2, 
  0x9096e2, 0x8096e2, 0x93c3, 0x9fc3, 0x8cc5, 0x83c5, 0xb5c3, 0x95c3, 0xb5c2, 
  0x84c5, 0xb6c4, 0xb7c4, 0xbbc4, 0xbcc4, 0x86c5, 0x92c4, 0x85c5, 0x9980e2, 0xadc2, 
  0xb1c2, 0x9c80e2, 0xbec2, 0xb6c2, 0xa7c2, 0xb7c3, 0x9e80e2, 0xb0c2, 0x9988e2, 
  0xb7c2, 0xb9c2, 0xb3c2, 0xb2c2, 0xa096e2, 0xa0c2, 3
  };

/** Table to convert CP775 to Unicode codepoints */
unsigned long cp_cp775[256] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 
  114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 262, 
  252, 233, 257, 228, 291, 229, 263, 322, 275, 342, 343, 299, 377, 196, 197, 
  201, 230, 198, 333, 246, 290, 162, 346, 347, 214, 220, 248, 163, 216, 215, 
  164, 256, 298, 243, 379, 380, 378, 8221, 166, 169, 174, 172, 189, 188, 321, 
  171, 187, 9617, 9618, 9619, 9474, 9508, 260, 268, 280, 278, 9571, 9553, 9559, 
  9565, 302, 352, 9488, 9492, 9524, 9516, 9500, 9472, 9532, 370, 362, 9562, 9556, 
  9577, 9574, 9568, 9552, 9580, 381, 261, 269, 281, 279, 303, 353, 371, 363, 
  382, 9496, 9484, 9608, 9604, 9612, 9616, 9600, 211, 223, 332, 323, 245, 213, 
  181, 324, 310, 311, 315, 316, 326, 274, 325, 8217, 173, 177, 8220, 190, 182, 
  167, 247, 8222, 176, 8729, 183, 185, 179, 178, 9632, 160
  };

/** Table to convert one character to UPPERCASE */
unsigned char uc_cp775[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 
  0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 
  0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x9a, 
  0x90, 0xa0, 0x8e, 0x95, 0x8f, 0x80, 0xad, 0xed, 0x8a, 0x8a, 0xa1, 0x8d, 0x8e, 
  0x8f, 0x90, 0x92, 0x92, 0xe2, 0x99, 0x95, 0x96, 0x97, 0x97, 0x99, 0x9a, 0x9d, 
  0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xe0, 0xa3, 0xa3, 0x8d, 0xa6, 0xa7, 0xa8, 
  0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 
  0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
  0xb5, 0xb6, 0xb7, 0xb8, 0xbd, 0xbe, 0xc6, 0xc7, 0xcf, 0xd9, 0xda, 0xdb, 0xdc, 
  0xdd, 0xde, 0xdf, 0xe0, 0x53, 0xe2, 0xe3, 0xe5, 0xe5, 0x3f, 0xe3, 0xe8, 0xe8, 
  0xea, 0xea, 0xee, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
  };

/** Table to convert one character to lowercase */
unsigned char lc_cp775[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 
  0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 
  0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x87, 0x81, 
  0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8b, 0x8b, 0x8c, 0xa5, 0x84, 
  0x86, 0x82, 0x91, 0x91, 0x93, 0x94, 0x85, 0x96, 0x98, 0x98, 0x94, 0x81, 0x9b, 
  0x9c, 0x9b, 0x9e, 0x9f, 0x83, 0x8c, 0xa2, 0xa4, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 
  0xa9, 0xaa, 0xab, 0xac, 0x88, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xd0, 
  0xd1, 0xd2, 0xd3, 0xb9, 0xba, 0xbb, 0xbc, 0xd4, 0xd5, 0xbf, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xd6, 0xd7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xd8, 
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 
  0xdd, 0xde, 0xdf, 0xa2, 0xe1, 0x93, 0xe7, 0xe4, 0xe4, 0xe6, 0xe7, 0xe9, 0xe9, 
  0xeb, 0xeb, 0xec, 0x89, 0xec, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
  };

#endif