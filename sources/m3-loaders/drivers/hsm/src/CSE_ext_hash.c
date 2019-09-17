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

#include "cse_typedefs.h"
#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_hash.h"

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
uint32_t CSE_SHA1(uint8_t *msg, uint32_t msg_size, uint8_t *digest,
		  uint32_t *pdigest_size)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * P1 is the hash algorithm descriptor
	 * P2 is the message buffer address
	 * P3 is the destination signature address
	 * P4 is the message byte size in input, the digest size in output
	 */

	ret = CSE_HAL_send_cmd4p(CSE_HASH, E_SHA1, (uint32_t)msg,
				 (uint32_t)digest, msg_size);

	*pdigest_size = CSE->P4.R;

	return ret;
}

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
uint32_t CSE_SHA224(uint8_t *msg, uint32_t msg_size, uint8_t *digest,
		    uint32_t *pdigest_size)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * P1 is the hash algorithm descriptor
	 * P2 is the message buffer address
	 * P3 is the destination signature address
	 * P4 is the message byte size in input, the digest size in output
	 */

	ret = CSE_HAL_send_cmd4p(CSE_HASH, E_SHA224, (uint32_t)msg,
				 (uint32_t)digest, msg_size);

	*pdigest_size = CSE->P4.R;

	return ret;
}

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
uint32_t CSE_SHA256(uint8_t *msg, uint32_t msg_size, uint8_t *digest,
		    uint32_t *pdigest_size)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 *P1 is the hash algorithm descriptor
	 * P2 is the message buffer address
	 * P3 is the destination signature address
	 * P4 is the message byte size in input, the digest size in output
	 */

	ret = CSE_HAL_send_cmd4p(CSE_HASH, E_SHA256, (uint32_t)msg,
				 (uint32_t)digest, msg_size);

	*pdigest_size = CSE->P4.R;

	return ret;
}

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
uint32_t CSE_SHA384(uint8_t *msg, uint32_t msg_size, uint8_t *digest,
		    uint32_t *pdigest_size)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * P1 is the hash algorithm descriptor
	 * P2 is the message buffer address
	 * P3 is the destination signature address
	 * P4 is the message byte size in input, the digest size in output
	 */

	ret = CSE_HAL_send_cmd4p(CSE_HASH, E_SHA384, (uint32_t)msg,
				 (uint32_t)digest, msg_size);

	*pdigest_size = CSE->P4.R;

	return ret;
}

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
uint32_t CSE_SHA512(uint8_t *msg, uint32_t msg_size, uint8_t *digest,
		    uint32_t *pdigest_size)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * P1 is the hash algorithm descriptor
	 * P2 is the message buffer address
	 * P3 is the destination signature address
	 * P4 is the message byte size in input, the digest size in output
	 */

	ret = CSE_HAL_send_cmd4p(CSE_HASH, E_SHA512, (uint32_t)msg,
				 (uint32_t)digest, msg_size);

	*pdigest_size = CSE->P4.R;

	return ret;
}

/**
 * @}
 */
