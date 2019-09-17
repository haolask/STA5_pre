/*
    SPC5-CRYPTO - Copyright (C) 2015 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    CSE_ext_RSA_PKCS_TV.h
 * @brief   RSA pkcs signature and encryption test vectors - header
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */
#ifndef CSE_ext_RSA_PKCS_TV
#define CSE_ext_RSA_PKCS_TV

#include "cse_types.h"
#include "CSE_ext_hash.h"

typedef struct
{
    const uint8_t* mod;
    uint32_t mod_byte_size;
    const uint8_t* pubexp;
    uint32_t pubexp_byte_size;
    const uint8_t* privexp;
    uint32_t privexp_byte_size;
}
rsa_key_stt;

typedef struct
{
    const uint8_t* msg;
    uint32_t msg_size;
    const uint8_t* sig;
    uint32_t sig_size;
}
test_vect_stt;

typedef struct
{
    enum hashType_et hashType;
    const uint8_t* msg;
    uint32_t msg_size;
    const uint8_t* sig;
    uint32_t sig_size;
}
tv_allSha_stt;

typedef struct
{
    rsa_key_stt    key;
    test_vect_stt* array;
}
TV_stt;

typedef struct
{
    rsa_key_stt    key;
    const tv_allSha_stt* array;
}
TV_allSha_stt;

typedef struct
{
    enum hashType_et hashType;
    const uint8_t* mod;
    uint32_t mod_byte_size;
    const uint8_t* pubexp;
    uint32_t pubexp_byte_size;
    const uint8_t* msg;
    uint32_t msg_size;
    const uint8_t* sig;
    uint32_t sig_size;
    const uint8_t* expectedVerifCheck;
} tv_signVerify_allSha_stt;

#define nb_tv_1024 20
#define nb_tv_2048 20

#define C_NB_TV_ALL_SHA_1024 50
#define C_NB_TV_ALL_SHA_2048 50
#define C_NB_TV_ALL_SHA_3072 50

#define C_NB_TV_SIGN_VERIFY_ALL_SHA_1024 90
#define C_NB_TV_SIGN_VERIFY_ALL_SHA_2048 90
#define C_NB_TV_SIGN_VERIFY_ALL_SHA_3072 90

/******************************************************************************/
/************************** RSA encryption Test Vector ************************/
/******************************************************************************/
extern rsa_key_stt RSA1024_encrypt_key;
extern rsa_key_stt RSA2048_encrypt_key;
extern rsa_key_stt RSA3072_encrypt_key;

/******************************************************************************/
/************************** RSA 1024 Test Vectors *****************************/
/******************************************************************************/
extern TV_stt TV_sha1_1024;
extern TV_allSha_stt TV_allSsha_1024;
extern const tv_signVerify_allSha_stt test_vect_signVerify_allSha_1024[];

/******************************************************************************/
/************************** RSA 2048 Test Vectors *****************************/
/******************************************************************************/
extern TV_stt TV_sha1_2048;
extern TV_allSha_stt TV_allSsha_2048;
extern TV_allSha_stt TV_allSsha_3072;
extern const tv_signVerify_allSha_stt test_vect_signVerify_allSha_2048[];

/******************************************************************************/
/************************** RSA 3072 Test Vectors *****************************/
/******************************************************************************/
extern TV_allSha_stt TV_allSsha_3072;
extern const tv_signVerify_allSha_stt test_vect_signVerify_allSha_3072[];

extern const uint8_t e_3[1];
extern const uint8_t message_1024_2[128];
extern const uint8_t enc_e3_message_1024_2[128];
extern const uint8_t message_2048[256];
extern const uint8_t enc_e3_message_2048[256];
extern const uint8_t message_3072[384];
extern const uint8_t enc_e3_message_3072[384];

#endif /* CSE_ext_RSA_PKCS_TV */
/**
 * @}
 */
