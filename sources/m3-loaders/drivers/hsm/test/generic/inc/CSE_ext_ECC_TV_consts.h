/*
    SPC5-CRYPTO - Copyright (C) 2014 STMicroelectronics

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
 * @file    CSE_ext_ECC_TV_consts.h
 * @brief   CSE ECC constant definition for ECC Test Vectors.
  */
#ifndef _CSE_EXT_ECC_TV_consts_H_
#define _CSE_EXT_ECC_TV_consts_H_

/*
 * @addtogroup SHE-ext_driver
 * @{
 */

#include "cse_typedefs.h"
#include"config.h"
#include "CSE_ext_hash.h"

/* Size of container in verify_test_vect_stt structures */
#if defined INCLUDE_NIST_P521
#define C_MAX_MSG_SIZE        128
#define C_MAX_PUB_KEY_SIZE     68    /* Avoid wrong alignment of Test vector table */
#define C_MAX_PRIV_KEY_SIZE    68
#define C_MAX_PMSN_SIZE        68
#define C_MAX_SIG_SIZE         68
#elif defined INCLUDE_NIST_P384 || defined INCLUDE_BRAINPOOL_P384R1
#define C_MAX_MSG_SIZE        128
#define C_MAX_PUB_KEY_SIZE     48
#define C_MAX_PRIV_KEY_SIZE    48
#define C_MAX_PMSN_SIZE        48
#define C_MAX_SIG_SIZE         48
#else
#define C_MAX_MSG_SIZE        128
#define C_MAX_PUB_KEY_SIZE     32
#define C_MAX_PRIV_KEY_SIZE    32
#define C_MAX_PMSN_SIZE        32
#define C_MAX_SIG_SIZE         32
#endif


#define C_TV_MSG_SIZE  128
#define C_P256_MOD_SIZE 32
#define C_P384_MOD_SIZE 48
#define C_P521_MOD_SIZE 66

/** Enumeration: List of supported NIST Curves */
/*! \enum ECC_ValidCurves_et CSE_CORE_ECC_key.h */
/*! \brief Enumeration of the supported NIST Elliptic Curves */
typedef enum ECC_ValidCurves_e
{
    C_NIST_P_256 = 0x50323536,
#ifdef INCLUDE_NIST_P384
    C_NIST_P_384 = 0x50333834,
#endif
#ifdef INCLUDE_NIST_P521
    C_NIST_P_521 = 0x50353231,
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
    C_BRAINPOOL_P256R1 = 0x42323536,
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
    C_BRAINPOOL_P384R1 = 0x42333834,
#endif
} ECC_ValidCurves_et;

#define TEST_PASS ((uint32_t) 0x50415353)
#define TEST_FAIL ((uint32_t) 0x4641494C)

#endif /* _CSE_EXT_ECC_TV_consts_H_ */

/**
 * @}
 */
