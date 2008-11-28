/**
 * @file
 * @ingroup workerframeenc
 * @brief Dicop Project - Character encoding tables for CP852
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

#ifndef DICOP_CP852_H
#define DICOP_CP852_H

/** Table to convert Unicode to CP852 */
unsigned long encoding_cp852[225] = {
  112,
  128, 0x0, 1, 0xc7, 1, 0xfc, 1, 0xe9, 1, 0xe2, 1, 0xe4, 1, 0x16f, 
  1, 0x107, 1, 0xe7, 1, 0x142, 1, 0xeb, 2, 0x150, 1, 0xee, 1, 0x179, 1, 0xc4, 
  1, 0x106, 1, 0xc9, 2, 0x139, 1, 0xf4, 1, 0xf6, 2, 0x13d, 2, 0x15a, 1, 0xd6, 
  1, 0xdc, 2, 0x164, 1, 0x141, 1, 0xd7, 1, 0x10d, 1, 0xe1, 1, 0xed, 1, 0xf3, 
  1, 0xfa, 2, 0x104, 2, 0x17d, 2, 0x118, 1, 0xac, 1, 0x17a, 1, 0x10c, 1, 0x15f, 
  1, 0xab, 1, 0xbb, 3, 0x2591, 1, 0x2502, 1, 0x2524, 2, 0xc1, 1, 0x11a, 1, 0x15e, 
  1, 0x2563, 1, 0x2551, 1, 0x2557, 1, 0x255d, 2, 0x17b, 1, 0x2510, 1, 0x2514, 
  1, 0x2534, 1, 0x252c, 1, 0x251c, 1, 0x2500, 1, 0x253c, 2, 0x102, 1, 0x255a, 
  1, 0x2554, 1, 0x2569, 1, 0x2566, 1, 0x2560, 1, 0x2550, 1, 0x256c, 1, 0xa4, 
  1, 0x111, 1, 0x110, 1, 0x10e, 1, 0xcb, 1, 0x10f, 1, 0x147, 2, 0xcd, 1, 0x11b, 
  1, 0x2518, 1, 0x250c, 1, 0x2588, 1, 0x2584, 1, 0x162, 1, 0x16e, 1, 0x2580, 
  1, 0xd3, 1, 0xdf, 1, 0xd4, 2, 0x143, 1, 0x148, 2, 0x160, 1, 0x154, 1, 0xda, 
  1, 0x155, 1, 0x170, 1, 0xfd, 1, 0xdd, 1, 0x163, 1, 0xb4, 1, 0xad, 1, 0x2dd, 
  1, 0x2db, 1, 0x2c7, 1, 0x2d8, 1, 0xa7, 1, 0xf7, 1, 0xb8, 1, 0xb0, 1, 0xa8, 
  1, 0x2d9, 1, 0x171, 2, 0x158, 1, 0x25a0, 1, 0xa0
  };

/** Table to convert CP852 to UTF-8 */
unsigned long enc_cp852[257] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 
  0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 
  0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x87c3, 0xbcc3, 
  0xa9c3, 0xa2c3, 0xa4c3, 0xafc5, 0x87c4, 0xa7c3, 0x82c5, 0xabc3, 0x90c5, 0x91c5, 
  0xaec3, 0xb9c5, 0x84c3, 0x86c4, 0x89c3, 0xb9c4, 0xbac4, 0xb4c3, 0xb6c3, 0xbdc4, 
  0xbec4, 0x9ac5, 0x9bc5, 0x96c3, 0x9cc3, 0xa4c5, 0xa5c5, 0x81c5, 0x97c3, 0x8dc4, 
  0xa1c3, 0xadc3, 0xb3c3, 0xbac3, 0x84c4, 0x85c4, 0xbdc5, 0xbec5, 0x98c4, 0x99c4, 
  0xacc2, 0xbac5, 0x8cc4, 0x9fc5, 0xabc2, 0xbbc2, 0x9196e2, 0x9296e2, 0x9396e2, 
  0x8294e2, 0xa494e2, 0x81c3, 0x82c3, 0x9ac4, 0x9ec5, 0xa395e2, 0x9195e2, 0x9795e2, 
  0x9d95e2, 0xbbc5, 0xbcc5, 0x9094e2, 0x9494e2, 0xb494e2, 0xac94e2, 0x9c94e2, 
  0x8094e2, 0xbc94e2, 0x82c4, 0x83c4, 0x9a95e2, 0x9495e2, 0xa995e2, 0xa695e2, 
  0xa095e2, 0x9095e2, 0xac95e2, 0xa4c2, 0x91c4, 0x90c4, 0x8ec4, 0x8bc3, 0x8fc4, 
  0x87c5, 0x8dc3, 0x8ec3, 0x9bc4, 0x9894e2, 0x8c94e2, 0x8896e2, 0x8496e2, 0xa2c5, 
  0xaec5, 0x8096e2, 0x93c3, 0x9fc3, 0x94c3, 0x83c5, 0x84c5, 0x88c5, 0xa0c5, 0xa1c5, 
  0x94c5, 0x9ac3, 0x95c5, 0xb0c5, 0xbdc3, 0x9dc3, 0xa3c5, 0xb4c2, 0xadc2, 0x9dcb, 
  0x9bcb, 0x87cb, 0x98cb, 0xa7c2, 0xb7c3, 0xb8c2, 0xb0c2, 0xa8c2, 0x99cb, 0xb1c5, 
  0x98c5, 0x99c5, 0xa096e2, 0xa0c2, 3
  };

/** Table to convert CP852 to Unicode codepoints */
unsigned long cp_cp852[256] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 
  114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 199, 
  252, 233, 226, 228, 367, 263, 231, 322, 235, 336, 337, 238, 377, 196, 262, 
  201, 313, 314, 244, 246, 317, 318, 346, 347, 214, 220, 356, 357, 321, 215, 
  269, 225, 237, 243, 250, 260, 261, 381, 382, 280, 281, 172, 378, 268, 351, 
  171, 187, 9617, 9618, 9619, 9474, 9508, 193, 194, 282, 350, 9571, 9553, 9559, 
  9565, 379, 380, 9488, 9492, 9524, 9516, 9500, 9472, 9532, 258, 259, 9562, 9556, 
  9577, 9574, 9568, 9552, 9580, 164, 273, 272, 270, 203, 271, 327, 205, 206, 
  283, 9496, 9484, 9608, 9604, 354, 366, 9600, 211, 223, 212, 323, 324, 328, 
  352, 353, 340, 218, 341, 368, 253, 221, 355, 180, 173, 733, 731, 711, 728, 
  167, 247, 184, 176, 168, 729, 369, 344, 345, 9632, 160
  };

/** Table to convert one character to UPPERCASE */
unsigned char uc_cp852[256] = {
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
  0x90, 0xb6, 0x8e, 0xde, 0x8f, 0x80, 0x9d, 0xd3, 0x8a, 0x8a, 0xd7, 0x8d, 0x8e, 
  0x8f, 0x90, 0x91, 0x91, 0xe2, 0x99, 0x95, 0x95, 0x97, 0x97, 0x99, 0x9a, 0x9b, 
  0x9b, 0x9d, 0x9e, 0xac, 0xb5, 0xd6, 0xe0, 0xe9, 0xa4, 0xa4, 0xa6, 0xa6, 0xa8, 
  0xa8, 0xaa, 0x8d, 0xac, 0xb8, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 
  0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbd, 0xbf, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xc6, 0xc6, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
  0xd1, 0xd1, 0xd2, 0xd3, 0xd2, 0xd5, 0xd6, 0xd7, 0xb7, 0xd9, 0xda, 0xdb, 0xdc, 
  0xdd, 0xde, 0xdf, 0xe0, 0x53, 0xe2, 0xe3, 0xe3, 0xd5, 0xe6, 0xe6, 0xe8, 0xe9, 
  0xe8, 0xeb, 0xed, 0xed, 0xdd, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xeb, 0xfc, 0xfc, 0xfe, 0xff
  };

/** Table to convert one character to lowercase */
unsigned char lc_cp852[256] = {
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
  0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8b, 0x8b, 0x8c, 0xab, 0x84, 
  0x86, 0x82, 0x92, 0x92, 0x93, 0x94, 0x96, 0x96, 0x98, 0x98, 0x94, 0x81, 0x9c, 
  0x9c, 0x88, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa5, 0xa5, 0xa7, 0xa7, 0xa9, 
  0xa9, 0xaa, 0xab, 0x9f, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xa0, 
  0x83, 0xd8, 0xad, 0xb9, 0xba, 0xbb, 0xbc, 0xbe, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xc7, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
  0xd0, 0xd0, 0xd4, 0x89, 0xd4, 0xe5, 0xa1, 0x8c, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 
  0xee, 0x85, 0xdf, 0xa2, 0xe1, 0x93, 0xe4, 0xe4, 0xe5, 0xe7, 0xe7, 0xea, 0xa3, 
  0xea, 0xfb, 0xec, 0xec, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfd, 0xfd, 0xfe, 0xff
  };

#endif