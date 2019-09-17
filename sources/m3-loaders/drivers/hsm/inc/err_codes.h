/**
 ******************************************************************************
 * @file    err_codes.h
 * @author  ST
 * @version V2.0.3
 * @date    22-June-2012
 * @brief   Provides defines for error codes
 ******************************************************************************
 * @attention
 *
 *  Copyright (C) 2014 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRL_ERR_CODES_H__
#define __CRL_ERR_CODES_H__

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cse_types.h"

/** @addtogroup UserAPI
 * @{
 */

/** @addtogroup ErrCodes Error Codes Definitions
 * @{
 */

/** @defgroup GenericError Generic Error Codes
 * @{
 */

/*!<  Authentication successful */
#define AUTHENTICATION_SUCCESSFUL ((int32_t)(1003))
/*!<  Authentication failed */
#define AUTHENTICATION_FAILED     ((int32_t)(1004))
/*!<  Signature is valid */
#define SIGNATURE_VALID           AUTHENTICATION_SUCCESSFUL
/*!<  Signature is NOT valid */
#define SIGNATURE_INVALID         AUTHENTICATION_FAILED
/*!<  Problems with dynamic allocation (there's no more available memory) */
#define ERR_MEMORY_FAIL           ((int32_t)(1005))
/*
 *!<  Used when pointer passed to DMA is not a multiple of word size
 * as it should
 */
#define DMA_BAD_ADDRESS           ((int32_t)(2101))
/*!<  Generic error during DMA transfer */
#define DMA_ERR_TRANSFER          ((int32_t)(2102))
/**
 * @}
 */

/** @defgroup HWError Hardware Error Codes
 * @{
 */
/*!<  CRYPTO HW ENGINES Success */
#define HW_SUCCESS        ((int32_t)(0))
/*!<  CRYPTO HW ENGINES Input Size is not multiple of AES block size */
#define HW_BAD_INPUT_SIZE ((int32_t)(2001))
/**
 * @}
 */

/** @defgroup AESError AES Error Codes
 * @{
 */
/*!< AES of PRIVKEY Success */
#define AES_SUCCESS             ((int32_t)(0))
/*!<  AES of PRIVKEY Invalid input size */
#define AES_ERR_BAD_INPUT_SIZE  ((int32_t)(3101))
/*!<  AES of PRIVKEY Invalid operation */
#define AES_ERR_BAD_OPERATION   ((int32_t)(3102))
/*
 *!<  AES of PRIVKEY The AES context contains some invalid or
 * uninitialized values
 */
#define AES_ERR_BAD_CONTEXT     ((int32_t)(3103))
/*!<  AES of PRIVKEY One of the expected function parameters is invalid */
#define AES_ERR_BAD_PARAMETER   ((int32_t)(3104))
/**
 * @}
 */

/** @defgroup ARC4Error ARC4 Error Codes
 * @{
 */
/*!< ARC4 of PRIVKEY Success*/
#define ARC4_SUCCESS             ((int32_t)(0))
/*!<  ARC4 of PRIVKEY Invalid operation */
#define ARC4_ERR_BAD_OPERATION   ((int32_t)(3202))
/*
 *!<  ARC4 of PRIVKEY The ARC4 context contains some invalid or
 * uninitialized values
 */
#define ARC4_ERR_BAD_CONTEXT     ((int32_t)(3203))
/*!<  ARC4 of PRIVKEY One of the expected function parameters is invalid */
#define ARC4_ERR_BAD_PARAMETER   ((int32_t)(3204))
/**
 * @}
 */

/** @defgroup DESError DES Error Codes
 * @{
 */
/*!<  DES of PRIVKEY Success */
#define DES_SUCCESS             ((int32_t)(0))
/*!<  DES of PRIVKEY Invalid input size, it must be multiple of 8 */
#define DES_ERR_BAD_INPUT_SIZE  ((int32_t)(3301))
/*!<  DES of PRIVKEY Invalid operation */
#define DES_ERR_BAD_OPERATION   ((int32_t)(3302))
/*
 *!<  DES of PRIVKEY The DES context contains some invalid or
 * uninitialized values
 */
#define DES_ERR_BAD_CONTEXT     ((int32_t)(3303))
 /*!<  DES of PRIVKEY One of the expected function parameters is invalid*/
#define DES_ERR_BAD_PARAMETER   ((int32_t)(3304))
/**
 * @}
 */

/** @defgroup TDESError TDES Error Codes
 * @{
 */
/*!<  TDES of PRIVKEY Success */
#define TDES_SUCCESS             ((int32_t)(0))
/*!<  TDES of PRIVKEY Invalid input size, it must be multiple of 8 */
#define TDES_ERR_BAD_INPUT_SIZE  ((int32_t)(3311))
/*!<  TDES of PRIVKEY Invalid operation */
#define TDES_ERR_BAD_OPERATION   ((int32_t)(3312))
/*
 *!<  TDES of PRIVKEY The TDES context contains some invalid or
 * uninitialized values
 */
#define TDES_ERR_BAD_CONTEXT     ((int32_t)(3313))
/*!<  TDES of PRIVKEY One of the expected function parameters is invalid */
#define TDES_ERR_BAD_PARAMETER   ((int32_t)(3314))
/**
 * @}
 */

/** @defgroup HASHError HASH Error Codes
 * @{
 */
/*!<  hash Success */
#define HASH_SUCCESS             ((int32_t)(0))
/*!<  hash Invalid operation */
#define HASH_ERR_BAD_OPERATION   ((int32_t)(4001))
/*!<  hash The HASH context contains some invalid or uninitialized values */
#define HASH_ERR_BAD_CONTEXT     ((int32_t)(4002))
/*!<  hash One of the expected function parameters is invalid */
#define HASH_ERR_BAD_PARAMETER   ((int32_t)(4003))
/**
 * @}
 */

/** @defgroup RSAError RSA Error Codes
 * @{
 */
/*!<  rsa Success */
#define RSA_SUCCESS              ((int32_t)(0))
/*!<  rsa Invalid operation */
#define RSA_ERR_BAD_OPERATION    ((int32_t)(5102))
/*!<  rsa Invalid Key */
#define RSA_ERR_BAD_KEY          ((int32_t)(5103))
/*!<  rsa One of the expected function parameters is invalid */
#define RSA_ERR_BAD_PARAMETER    ((int32_t)(5104))
/*!<  rsa The hash function is not supported */
#define RSA_ERR_UNSUPPORTED_HASH ((int32_t)(5105))
/*!<  rsa Message too long */
#define RSA_ERR_MESSAGE_TOO_LONG ((int32_t)(5106))
/*!<  RSA modulus too short */
#define RSA_ERR_MODULUS_TOO_SHORT ((int32_t)(5107))
/**
 * @}
 */

/** @defgroup ECCError ECC Error Codes
 * @{
 */
/*!<  ecc Success */
#define ECC_SUCCESS              ((int32_t)(0))
/*!<  ecc Invalid operation */
#define ECC_ERR_BAD_OPERATION    ((int32_t)(5202))
/*!<  ecc The ECC context contains some invalid or initialized parameters */
#define ECC_ERR_BAD_CONTEXT      ((int32_t)(5203))
/*!<  ecc One of the expected function parameters is invalid */
#define ECC_ERR_BAD_PARAMETER    ((int32_t)(5204))
/*!<  ecc Invalid Public Key */
#define ECC_ERR_BAD_PUBLIC_KEY   ((int32_t)(5205))
/*!<  ecc Invalid Private Key */
#define ECC_ERR_BAD_PRIVATE_KEY  ((int32_t)(5206))
/*
 *!<  ecc The EC parameters structure miss some parameter required by the
 * function
 */
#define ECC_ERR_MISSING_EC_PARAMETER ((int32_t)(5207))
/*!<  ecc Returned Point is the point at infinity */
#define ECC_WARN_POINT_AT_INFINITY   ((int32_t)(5208))
/*!<  ecc Key slot in NVM Key Storage is not empty */
#define ECC_NVM_KEY_SLOT_NOT_EMPTY   ((int32_t)(32))
/**
 * @}
 */

/** @defgroup RNGError Random Number Error Codes
 * @{
 */
/*!<  RNG Success */
#define RNG_SUCCESS                  ((int32_t)(0))
/*!<  RNG has not been correctly initialized */
#define RNG_ERR_UNINIT_STATE         ((int32_t)(6001))
/*!<  RNG Invalid operation */
#define RNG_ERR_BAD_OPERATION        ((int32_t)(6002))
/*!<  RNG Reseed is needed */
#define RNG_ERR_RESEED_NEEDED        ((int32_t)(6003))
/*!<  RNG One of the expected function parameters is invalid */
#define RNG_ERR_BAD_PARAMETER        ((int32_t)(6004))
/*!<  RNG Check the size of the entropy string */
#define RNG_ERR_BAD_ENTROPY_SIZE     ((int32_t)(6006))
/*!<  RNG Check the size of the personalization string */
#define RNG_ERR_BAD_PERS_STRING_SIZE ((int32_t)(6007))
/*!<  RNG Check the size of the additional input string */
#define RNG_ERR_BAD_ADD_INPUT_SIZE   ((int32_t)(6008))
/*!<  RNG Check the size of the random request */
#define RNG_ERR_BAD_REQUEST          ((int32_t)(6009))
/*!<  RNG Check the size of the nocne */
#define RNG_ERR_BAD_NONCE_SIZE       ((int32_t)(6010))

/** @defgroup MathError Mathematical Error Codes
 * @{
 */
/*!<  Math Success */
#define MATH_SUCCESS                ((int32_t)(0))
/*
 *!<  Math Overflow, the returned BigNum would be greater than its maximum size
 */
#define MATH_ERR_BIGNUM_OVERFLOW    ((int32_t)(5301))
/*!<  Math This function can be used only with odd moduli */
#define MATH_ERR_EVEN_MODULUS       ((int32_t)(5302))
/*!<  Math One of the expected function parameters is invalid */
#define MATH_ERR_BAD_PARAMETER      ((int32_t)(5304))
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CRL_ERR_CODES_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
