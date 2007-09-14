/**
 * @file
 * @ingroup workerframe
 * @brief Defines some handy macros for libtomcrypt support
 * 
 * @copydoc copyrighttext
*/

#ifndef TOM_CRYPT_H
#define TOM_CRYPT_H

/************************************************************************* */
/* These macros are defined only when tomcrypt.h was included */

#ifdef CRYPT
#ifdef SCRYPT

/* temp. variables for macros */
int _crypt_err;
int _cipher_idx;

/* hash */

#define SELFTEST_MD5 if ((_crypt_err = md5_test()) != CRYPT_OK) {\
    printf("MD5 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_MD4 if ((_crypt_err = md4_test()) != CRYPT_OK) {\
    printf("MD4 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_SHA1 if ((_crypt_err = sha1_test()) != CRYPT_OK) {\
    printf("SHA1 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_SHA256 if ((_crypt_err = sha256_test()) != CRYPT_OK) {\
    printf("SHA256 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_SHA512 if ((_crypt_err = sha512_test()) != CRYPT_OK) {\
    printf("SHA512 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

/* crypt/encrypt */

#define SELFTEST_AES if ((_crypt_err = aes_test()) != CRYPT_OK) {\
    printf("AES did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_BLOWFISH if ((_crypt_err = blowfish_test()) != CRYPT_OK) {\
    printf("BLOWFISH did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_TWOFISH if ((_crypt_err = twofish_test()) != CRYPT_OK) {\
    printf("TWOFISH did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_DES if ((_crypt_err = des_test()) != CRYPT_OK) {\
    printf("DES did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_DES3 if ((_crypt_err = des3_test()) != CRYPT_OK) {\
    printf("DES3 (triple DES) did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

#define SELFTEST_CAST128 if ((_crypt_err = cast5_test()) != CRYPT_OK) {\
    printf("CAST128 did not pass selftest: '%s'\n", error_to_string(_crypt_err));\
    return PWD_ERROR;\
    }

/** Hash the given LEN bytes pointed to by SRC with SHA1, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[32];

    hash_with_sha1(digest,pwd->pwd,pwd->len,&ctx);
*/
#define hash_with_sha1(dst,src,len,ctx) sha1_init(ctx); sha1_process(ctx,(unsigned char*)src,len); sha1_done(ctx,dst);

/** Hash the given LEN bytes pointed to by SRC with MD5, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[20];

    hash_with_md5(digest,pwd->cur->content,pwd->cur->bytes,&ctx);
*/
#define hash_with_md5(dst,src,len,ctx) md5_init(ctx); md5_process(ctx,(unsigned char*)src,len); md5_done(ctx,dst);

/** Hash the given LEN bytes pointed to by SRC with SHA256, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[32];

    hash_with_sha256(digest,pwd->cur->content,pwd->cur->bytes,&ctx);
*/
#define hash_with_sha256(dst,src,len,ctx) sha256_init(ctx); sha256_process(ctx,(unsigned char*)src,len); sha256_done(ctx,dst);

/** Hash the given LEN bytes pointed to by SRC with SHA512, and store the digest at
    destination DST. The storage for hashstate is located in CTX.

    Example usage:

    hash_state ctx;
    unsigned char digest[64];

    hash_with_sha512(digest,pwd->cur->content,pwd->cur->bytes,&ctx);
*/
#define hash_with_sha512(dst,src,len,ctx) sha512_init(ctx); sha512_process(ctx,(unsigned char*)src,len); sha512_done(ctx,dst);

/** Decrypt the given bytes with AES in CBC mode.

    Example usage:@verbatim

    symmetric_CBC aes;
    unsigned char iv[16];
    unsigned char key[16];
    unsigned char input[16];
    unsigned char output[16];
    int aes_index;

    // setup key:
    if ((err = cbc_start(aes_index, iv, key, sizeof(key), 10, &aes)) != CRYPT_OK) {
      printf("Error: AES CBC setup failed: %s\n", error_to_string(err));
      return PWD_ERROR;
      }

    aes_cbc_decrypt(input, output, 16, &ctx); @endverbatim
*/

#define aes_cbc_decrypt(encrypted, decrypted, len, ctx)\
    if ((_crypt_err = cbc_decrypt( encrypted, decrypted, len, ctx)) != CRYPT_OK) {\
      printf("Error: AES CBC decrypt failed: %s\n", error_to_string(_crypt_err));\
      return PWD_ERROR; }

#endif
#endif

#endif
