/*!
 * @file
 * @ingroup workerframeenc
 * @brief Dicop Project - Character encoding tables for CP862
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
 * @date Tue Aug 28 14:00:49 2007
 *  
 * @copydoc copyrighttext
*/

#ifndef DICOP_CP862_H
#define DICOP_CP862_H

/** Table to convert Unicode to CP862 */
unsigned long encoding_cp862[189] = {
  94,
  128, 0x0, 27, 0x5d0, 2, 0xa2, 1, 0xa5, 1, 0x20a7, 1, 0x192, 
  1, 0xe1, 1, 0xed, 1, 0xf3, 1, 0xfa, 1, 0xf1, 1, 0xd1, 1, 0xaa, 1, 0xba, 1, 0xbf, 
  1, 0x2310, 1, 0xac, 1, 0xbd, 1, 0xbc, 1, 0xa1, 1, 0xab, 1, 0xbb, 3, 0x2591, 
  1, 0x2502, 1, 0x2524, 2, 0x2561, 1, 0x2556, 1, 0x2555, 1, 0x2563, 1, 0x2551, 
  1, 0x2557, 1, 0x255d, 1, 0x255c, 1, 0x255b, 1, 0x2510, 1, 0x2514, 1, 0x2534, 
  1, 0x252c, 1, 0x251c, 1, 0x2500, 1, 0x253c, 2, 0x255e, 1, 0x255a, 1, 0x2554, 
  1, 0x2569, 1, 0x2566, 1, 0x2560, 1, 0x2550, 1, 0x256c, 2, 0x2567, 2, 0x2564, 
  1, 0x2559, 1, 0x2558, 2, 0x2552, 1, 0x256b, 1, 0x256a, 1, 0x2518, 1, 0x250c, 
  1, 0x2588, 1, 0x2584, 1, 0x258c, 1, 0x2590, 1, 0x2580, 1, 0x3b1, 1, 0xdf, 1, 0x393, 
  1, 0x3c0, 1, 0x3a3, 1, 0x3c3, 1, 0xb5, 1, 0x3c4, 1, 0x3a6, 1, 0x398, 1, 0x3a9, 
  1, 0x3b4, 1, 0x221e, 1, 0x3c6, 1, 0x3b5, 1, 0x2229, 1, 0x2261, 1, 0xb1, 1, 0x2265, 
  1, 0x2264, 2, 0x2320, 1, 0xf7, 1, 0x2248, 1, 0xb0, 1, 0x2219, 1, 0xb7, 1, 0x221a, 
  1, 0x207f, 1, 0xb2, 1, 0x25a0, 1, 0xa0
  };

/** Table to convert CP862 to UTF-8 */
unsigned long enc_cp862[257] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 
  0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 
  0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 
  0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 
  0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 
  0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x90d7, 0x91d7, 
  0x92d7, 0x93d7, 0x94d7, 0x95d7, 0x96d7, 0x97d7, 0x98d7, 0x99d7, 0x9ad7, 0x9bd7, 
  0x9cd7, 0x9dd7, 0x9ed7, 0x9fd7, 0xa0d7, 0xa1d7, 0xa2d7, 0xa3d7, 0xa4d7, 0xa5d7, 
  0xa6d7, 0xa7d7, 0xa8d7, 0xa9d7, 0xaad7, 0xa2c2, 0xa3c2, 0xa5c2, 0xa782e2, 0x92c6, 
  0xa1c3, 0xadc3, 0xb3c3, 0xbac3, 0xb1c3, 0x91c3, 0xaac2, 0xbac2, 0xbfc2, 0x908ce2, 
  0xacc2, 0xbdc2, 0xbcc2, 0xa1c2, 0xabc2, 0xbbc2, 0x9196e2, 0x9296e2, 0x9396e2, 
  0x8294e2, 0xa494e2, 0xa195e2, 0xa295e2, 0x9695e2, 0x9595e2, 0xa395e2, 0x9195e2, 
  0x9795e2, 0x9d95e2, 0x9c95e2, 0x9b95e2, 0x9094e2, 0x9494e2, 0xb494e2, 0xac94e2, 
  0x9c94e2, 0x8094e2, 0xbc94e2, 0x9e95e2, 0x9f95e2, 0x9a95e2, 0x9495e2, 0xa995e2, 
  0xa695e2, 0xa095e2, 0x9095e2, 0xac95e2, 0xa795e2, 0xa895e2, 0xa495e2, 0xa595e2, 
  0x9995e2, 0x9895e2, 0x9295e2, 0x9395e2, 0xab95e2, 0xaa95e2, 0x9894e2, 0x8c94e2, 
  0x8896e2, 0x8496e2, 0x8c96e2, 0x9096e2, 0x8096e2, 0xb1ce, 0x9fc3, 0x93ce, 0x80cf, 
  0xa3ce, 0x83cf, 0xb5c2, 0x84cf, 0xa6ce, 0x98ce, 0xa9ce, 0xb4ce, 0x9e88e2, 0x86cf, 
  0xb5ce, 0xa988e2, 0xa189e2, 0xb1c2, 0xa589e2, 0xa489e2, 0xa08ce2, 0xa18ce2, 
  0xb7c3, 0x8889e2, 0xb0c2, 0x9988e2, 0xb7c2, 0x9a88e2, 0xbf81e2, 0xb2c2, 0xa096e2, 
  0xa0c2, 3
  };

/** Table to convert CP862 to Unicode codepoints */
unsigned long cp_cp862[256] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 
  60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 
  79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 
  114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 1488, 
  1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500, 1501, 
  1502, 1503, 1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 
  162, 163, 165, 8359, 402, 225, 237, 243, 250, 241, 209, 170, 186, 191, 8976, 
  172, 189, 188, 161, 171, 187, 9617, 9618, 9619, 9474, 9508, 9569, 9570, 9558, 
  9557, 9571, 9553, 9559, 9565, 9564, 9563, 9488, 9492, 9524, 9516, 9500, 9472, 
  9532, 9566, 9567, 9562, 9556, 9577, 9574, 9568, 9552, 9580, 9575, 9576, 9572, 
  9573, 9561, 9560, 9554, 9555, 9579, 9578, 9496, 9484, 9608, 9604, 9612, 9616, 
  9600, 945, 223, 915, 960, 931, 963, 181, 964, 934, 920, 937, 948, 8734, 966, 
  949, 8745, 8801, 177, 8805, 8804, 8992, 8993, 247, 8776, 176, 8729, 183, 8730, 
  8319, 178, 9632, 160
  };

/** Table to convert one character to UPPERCASE */
unsigned char uc_cp862[256] = {
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
  0x9c, 0x9d, 0x9e, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xa5, 0xa5, 0xa6, 0xa7, 0xa8, 
  0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 
  0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 
  0xdd, 0xde, 0xdf, 0x3f, 0x53, 0xe2, 0x3f, 0xe4, 0xe4, 0x3f, 0x3f, 0xe8, 0xe9, 
  0xea, 0x3f, 0xec, 0xe8, 0x3f, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
  };

/** Table to convert one character to lowercase */
unsigned char lc_cp862[256] = {
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
  0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa4, 0xa6, 0xa7, 0xa8, 
  0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 
  0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 
  0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0x3f, 0xe3, 0xe5, 0xe5, 0xe6, 0xe7, 0xed, 0x3f, 
  0x3f, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 
  0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
  };

#endif
