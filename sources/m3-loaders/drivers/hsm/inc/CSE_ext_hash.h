/*
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
 */

/**
 * @file    CSE_ext_hash.c
 * @brief   CSE hash commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#ifndef CSE_ext_HASH_H
#define CSE_ext_HASH_H

#include "cse_typedefs.h"

/* from hash_common */
/**
 * @brief  Enumeration of possible hash functions
 */
enum hashType_et {
	E_MD5,          /*!< MD5 */
	E_SHA1,         /*!< SHA-1 */
	E_SHA224,       /*!< SHA-224 */
	E_SHA256,       /*!< SHA-256 */
	E_SHA384,       /*!< SHA-384 */
	E_SHA512,       /*!< SHA-512 */
	E_INVALID_HASH  = 0xFFFFFFFF,
};

/*!< Number of bytes (uint8_t) to store a MD5 digest.*/
#define CRL_MD5_SIZE  16
/*!<  Number of bytes (uint8_t) to store a SHA-1 digest.*/
#define CRL_SHA1_SIZE 20
/*!<  Number of bytes (uint8_t) to store a SHA-224 digest.*/
#define CRL_SHA224_SIZE 28
/*!<  Number of bytes (uint8_t) to store a SHA-256 digest.*/
#define CRL_SHA256_SIZE 32
/*!<  Number of bytes (uint8_t) to store a SHA-384 digest.*/
#define CRL_SHA384_SIZE 48
/*!<  Number of bytes (uint8_t) to store a SHA-512 digest.*/
#define CRL_SHA512_SIZE 64

uint32_t HASH_tests(uint32_t P_verbose);

/*============================================================================*/
/**
 * @brief          Performs SHA1 digest computation
 * @details        Computes SHA1 digest of the provided message
 *
 * @param[in]      msg	       pointer to input message buffer
 * @param[in]      byte_size   message buffer size in bytes
 * @param[out]     digest      pointer to digest destination buffer
 * @param[out]     digest_size pointer to digest size (in byte)
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE
 *		   returned ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_SHA1(uint8_t *msg, uint32_t msg_size,
			 uint8_t *digest, uint32_t *pdigest_size);

/*============================================================================*/
/**
 * @brief          Performs SHA224 digest computation
 * @details        Computes SHA224 digest of the provided message
 *
 * @param[in]      msg	       pointer to input message buffer
 * @param[in]      byte_size   message buffer size in bytes
 * @param[out]     digest      pointer to digest destination buffer
 * @param[out]     digest_size pointer to digest size (in byte)
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_SHA224(uint8_t *msg, uint32_t msg_size,
			   uint8_t *digest, uint32_t *pdigest_size);

/*============================================================================*/
/**
 * @brief          Performs SHA256 digest computation
 * @details        Computes SHA256 digest of the provided message
 *
 * @param[in]      msg	       pointer to input message buffer
 * @param[in]      byte_size   message buffer size in bytes
 * @param[out]     digest      pointer to digest destination buffer
 * @param[out]     digest_size pointer to digest size (in byte)
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_SHA256(uint8_t *msg, uint32_t msg_size,
			   uint8_t *digest, uint32_t *pdigest_size);

/*============================================================================*/
/**
 * @brief          Performs SHA384 digest computation
 * @details        Computes SHA384 digest of the provided message
 *
 * @param[in]      msg	       pointer to input message buffer
 * @param[in]      byte_size   message buffer size in bytes
 * @param[out]     digest      pointer to digest destination buffer
 * @param[out]     digest_size pointer to digest size (in byte)
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_SHA384(uint8_t *msg, uint32_t msg_size,
			   uint8_t *digest, uint32_t *pdigest_size);

/*============================================================================*/
/**
 * @brief          Performs SHA512 digest computation
 * @details        Computes SHA512 digest of the provided message
 *
 * @param[in]      msg	       pointer to input message buffer
 * @param[in]      byte_size   message buffer size in bytes
 * @param[out]     digest      pointer to digest destination buffer
 * @param[out]     digest_size pointer to digest size (in byte)
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *		   see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_SHA512(uint8_t *msg, uint32_t msg_size,
			   uint8_t *digest, uint32_t *pdigest_size);

#endif /* CSE_ext_HASH_H */
/**
 * @}
 */
