/*
* CSE_ext_AES256_GCM.h
*/

/*#  CAVS 14.0*/

#ifndef _CSE_EXT_AES256_GCM_H_
#define _CSE_EXT_AES256_GCM_H_

#include "cse_types.h"
#include "CSE_ext_AES256_tests.h"

#define C_MAX_IV_BYTE_SIZE 128
#define C_MAX_PT_BYTE_SIZE 52	//51
#define C_MAX_AAD_BYTE_SIZE 92	//90
#define C_MAX_TAG_BYTE_SIZE 16
#define C_MAX_CT_BYTE_SIZE C_MAX_PT_BYTE_SIZE

#define C_NB_OF_AES_GCM_DECRPT_TV 175
#define C_NB_OF_AES_GCM_ENCRPT_TV 175
#define C_NB_AES256_GCM_TV 15

#define C_GCM_DECRYPT_TEST_PASS ((uint32_t) 0x50415353)
#define C_GCM_DECRYPT_TEST_FAIL ((uint32_t) 0x4641494C)


/* Structure for AES256 GCM Encrypt tests */
typedef struct
{
    const uint8_t key[C_AES256_KEY_SIZE];
    const uint8_t iv[C_MAX_IV_BYTE_SIZE];
    const uint8_t pt[C_MAX_PT_BYTE_SIZE];
    const uint8_t aad[C_MAX_AAD_BYTE_SIZE];
    const uint8_t ct[C_MAX_CT_BYTE_SIZE];
    const uint8_t tag[C_MAX_TAG_BYTE_SIZE];
} aes256GcmEncryptTestVector_stt;

typedef struct
{
    const uint32_t keySize;
    const uint32_t ivSize;
    const uint32_t ptSize;
    const uint32_t aadSize;
    const uint32_t tagSize;
    aes256GcmEncryptTestVector_stt aes256GcmEncrypt_TV[C_NB_AES256_GCM_TV];
} aes256Gcm_encrypt_testVect_stt;

/* Structure for AES256 GCM Decrypt tests */

typedef struct
{
    const uint8_t key[C_AES256_KEY_SIZE];
    const uint8_t iv[C_MAX_IV_BYTE_SIZE];
    const uint8_t ct[C_MAX_CT_BYTE_SIZE];
    const uint8_t aad[C_MAX_AAD_BYTE_SIZE];
    const uint8_t tag[C_MAX_TAG_BYTE_SIZE];
    const uint32_t expectedTestResult;
    const uint8_t pt[C_MAX_PT_BYTE_SIZE];
} aes256GcmDecryptTestVector_stt;

typedef struct
{
    const uint32_t keySize;
    const uint32_t ivSize;
    const uint32_t ptSize;
    const uint32_t aadSize;
    const uint32_t tagSize;
    aes256GcmDecryptTestVector_stt aes256GcmDecrypt_TV[C_NB_AES256_GCM_TV];
} aes256Gcm_decrypt_testVect_stt;

#endif /* _CSE_EXT_AES256_GCM_H_ */
