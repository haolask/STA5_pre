/*
    SPC5-CRYPTO - Copyright (C) 2016 STMicroelectronics

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
 * @file    CSE_ext_ECC_ECDSA_SignGenVerif_TV.h
 * @brief   CSE ECC ECDSA Signature Generation and Verification test vector header file.
  */
#ifndef _CSE_EXT_ECC_ECDSA_SIGN_GEN_TV_H_
#define _CSE_EXT_ECC_ECDSA_SIGN_GEN_TV_H_

/*
 * @addtogroup SHE-ext_driver
 * @{
 */

#include "cse_typedefs.h"
#include"config.h"
#include "CSE_ext_hash.h"
#include "CSE_ext_ECC_TV_consts.h"
#include "CSE_ext_ECC_ECDSA_test.h"

/* Number of Test VEctors for Key Pair Generation Tests */
#define C_NB_OF_EC_KEYPAIRGEN_P256_TV 10
#define C_NB_OF_EC_KEYPAIRGEN_P384_TV 10
#define C_NB_OF_EC_KEYPAIRGEN_P521_TV 10

#define C_TV_MSG_SIZE 128

/* Number of ECDSA elementary Test Vectors */
#define NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA256 15
#define NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA384 15
#define NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA512 15
#define NB_OF_ECDSA_SIGN_GEN_TV_P384_SHA384 15
#define NB_OF_ECDSA_SIGN_GEN_TV_P384_SHA512 15
#define NB_OF_ECDSA_SIGN_GEN_TV_P521_SHA512 15

#define NB_OF_ECDSA_SIGN_GEN_TV_brainpoolP256r1_SHA256 15
#define NB_OF_ECDSA_SIGN_GEN_TV_brainpoolP384r1_SHA384 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_brainpoolP256r1_SHA256 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_brainpoolP384r1_SHA384 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_brainpoolP256r1_SHA256 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_brainpoolP384r1_SHA384 15

#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA256 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA384 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA512 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P384_SHA384 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P384_SHA512 15
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P521_SHA512 15

#define NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA256 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA384 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA512 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_P384_SHA384 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_P384_SHA512 15
#define NB_OF_ECDSA_SIGN_VERIF_TV_P521_SHA512 15

/* Number of embedded test vectors; Test configuration is made in config.h header file with Elliptic Curve and SHA algorithm definitions */
/* Default size of test vectors: NIST P_256 with SHA256 */
#define NB_OF_ECDSA_SIGN_GEN_TV_P256        NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA256
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256     NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA256
#define NB_OF_ECDSA_SIGN_VERIF_TV_P256      NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA256

/* For NIST P256, SHA384 and/or SHA512 can be selected */
#if defined (INCLUDE_SHA384) && defined (INCLUDE_SHA512)
#undef NB_OF_ECDSA_SIGN_GEN_TV_P256
#undef NB_OF_ECDSA_SIGN_VERIFY_TV_P256
#undef NB_OF_ECDSA_SIGN_VERIF_TV_P256
#define NB_OF_ECDSA_SIGN_GEN_TV_P256    NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA256 + NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA384 + NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA512
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256 NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA256 + NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA384 + NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA512
#define NB_OF_ECDSA_SIGN_VERIF_TV_P256  NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA256 + NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA384 + NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA512
#elif defined (INCLUDE_SHA384) && !defined(INCLUDE_SHA512)
#undef NB_OF_ECDSA_SIGN_GEN_TV_P256
#undef NB_OF_ECDSA_SIGN_VERIFY_TV_P256
#undef NB_OF_ECDSA_SIGN_VERIF_TV_P256
#define NB_OF_ECDSA_SIGN_GEN_TV_P256    NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA256 + NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA384
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256 NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA256 + NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA384
#define NB_OF_ECDSA_SIGN_VERIF_TV_P256  NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA256 + NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA384
#elif defined INCLUDE_SHA512 & !defined(INCLUDE_SHA384)
#undef NB_OF_ECDSA_SIGN_GEN_TV_P256
#undef NB_OF_ECDSA_SIGN_VERIFY_TV_P256
#undef NB_OF_ECDSA_SIGN_VERIF_TV_P256
#define NB_OF_ECDSA_SIGN_GEN_TV_P256    NB_OF_ECDSA_SIGN_GEN_TV_P256_SHA5152
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P256 NB_OF_ECDSA_SIGN_VERIFY_TV_P256_SHA512
#define NB_OF_ECDSA_SIGN_VERIF_TV_P256  NB_OF_ECDSA_SIGN_VERIF_TV_P256_SHA512
#endif /* defined (INCLUDE_SHA384) && defined (INCLUDE_SHA512) */

/* If NIST P_384 is supported, SHA384 and/or SHA512 must be selected */
#if defined (INCLUDE_NIST_P384)
#if defined (INCLUDE_SHA384) && defined (INCLUDE_SHA512)
#define NB_OF_ECDSA_SIGN_GEN_TV_P384    NB_OF_ECDSA_SIGN_GEN_TV_P384_SHA384 + NB_OF_ECDSA_SIGN_GEN_TV_P384_SHA512
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P384 NB_OF_ECDSA_SIGN_VERIFY_TV_P384_SHA384 + NB_OF_ECDSA_SIGN_VERIFY_TV_P384_SHA512
#define NB_OF_ECDSA_SIGN_VERIF_TV_P384  NB_OF_ECDSA_SIGN_VERIF_TV_P384_SHA384 + NB_OF_ECDSA_SIGN_VERIF_TV_P384_SHA512
#elif defined (INCLUDE_SHA384) && !defined(INCLUDE_SHA512)
#define NB_OF_ECDSA_SIGN_GEN_TV_P384    NB_OF_ECDSA_SIGN_GEN_TV_P384_SHA384
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P384 NB_OF_ECDSA_SIGN_VERIFY_TV_P384_SHA384
#define NB_OF_ECDSA_SIGN_VERIF_TV_P384  NB_OF_ECDSA_SIGN_VERIF_TV_P384_SHA384
#elif defined INCLUDE_SHA512 & !defined(INCLUDE_SHA384)
#define NB_OF_ECDSA_SIGN_GEN_TV_P384    NB_OF_ECDSA_SIGN_GEN_TV_P384_SHA5152
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P384 NB_OF_ECDSA_SIGN_VERIFY_TV_P384_SHA512
#define NB_OF_ECDSA_SIGN_VERIF_TV_P384  NB_OF_ECDSA_SIGN_VERIF_TV_P384_SHA512
#else
#define NB_OF_ECDSA_SIGN_GEN_TV_P384    0
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P384 0
#define NB_OF_ECDSA_SIGN_VERIF_TV_P384  0
#endif /* defined (INCLUDE_SHA384) && defined (INCLUDE_SHA512) */
#endif /* INCLUDE_NIST_P384 */

/* if NIST P_521 is supported, SHA512 must be selected */
#if defined (INCLUDE_NIST_P521)
#if defined (INCLUDE_SHA512)
#define NB_OF_ECDSA_SIGN_GEN_TV_P521    NB_OF_ECDSA_SIGN_GEN_TV_P521_SHA512
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P521 NB_OF_ECDSA_SIGN_VERIFY_TV_P521_SHA512
#define NB_OF_ECDSA_SIGN_VERIF_TV_P521  NB_OF_ECDSA_SIGN_VERIF_TV_P521_SHA512
#else
#define NB_OF_ECDSA_SIGN_GEN_TV_P521    0
#define NB_OF_ECDSA_SIGN_VERIFY_TV_P521 0
#define NB_OF_ECDSA_SIGN_VERIF_TV_P521  0
#endif /* defined (INCLUDE_SHA512) */
#endif /* INCLUDE_NIST_P521 */

typedef struct
{
    const ECC_ValidCurves_et ecType;                  /* Elliptic Curve Size: P_256, P_384, P_521 */
    const enum hashType_et hashType;                  /* Hash type: SHA256, SHA384, SHA512 */
    const uint8_t msg[C_MAX_MSG_SIZE];                /* Message to sign */
    const uint8_t privateKey[C_MAX_PRIV_KEY_SIZE];    /* Private Key used for signature generation */
    const uint8_t publicXKey[C_MAX_PUB_KEY_SIZE];     /* Public Key X used for signature verification */
    const uint8_t publicYKey[C_MAX_PUB_KEY_SIZE];     /* Public Key Y used for signature verification */
} signVerify_test_vect_stt;

typedef struct
{
    const ECC_ValidCurves_et ecType;                  /* Elliptic Curve Size: P_256, P_384, P_521 */
    const enum hashType_et hashType;                  /* Hash type: SHA256, SHA384, SHA512 */
    const uint8_t msg[C_MAX_MSG_SIZE];                /* Message to sign */
    const uint8_t publicXKey[C_MAX_PUB_KEY_SIZE];     /* Public Key X used for signature verification */
    const uint8_t publicYKey[C_MAX_PUB_KEY_SIZE];     /* Public Key Y used for signature verification */
    const uint8_t sigR[C_MAX_SIG_SIZE];               /* ECDSA expected signature R field */
    const uint8_t sigS[C_MAX_SIG_SIZE];               /* ECDSA expected signature S field */
    const uint32_t expectedResult;                    /* Expected verification result: SIGNATURE_VALID or SIGNATURE_INVALID */
} verify_test_vect_stt;

typedef struct
{
    const ECC_ValidCurves_et ecType;                  /* Elliptic Curve Size: P_256, P_384, P_521 */
    const uint8_t privateKey[C_MAX_PRIV_KEY_SIZE];    /* Private Key */
    const uint8_t publicXKey[C_MAX_PUB_KEY_SIZE];     /* Expected Public Key X */
    const uint8_t publicYKey[C_MAX_PUB_KEY_SIZE];     /* Expected Public Key Y */
} keyPairGen_test_vect_stt;

#endif /* _CSE_EXT_ECC_ECDSA_SIGN_GEN_TV_H_ */
/**
 * @}
 */
