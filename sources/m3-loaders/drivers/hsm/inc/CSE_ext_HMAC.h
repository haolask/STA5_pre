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
 * @file    CSE_ext_HMAC.h
 * @brief   CSE HMAC test
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#ifndef _CSE_ext_HMAC_
#define _CSE_ext_HMAC_

#include "cse_types.h"
#include "cse_typedefs.h"
#include "CSE_ext_extendKey.h"
#include "CSE_Constants.h"

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief  Structure type for HMAC MAC
 * Two fields
 *  - 1) Byte array descriptor for Signature field R
 */

struct HMAC_Mac_stt {
	struct ByteArrayDescriptor_stt mHmacMacTag;
};

/**
 * @brief  Structure type for HMAC key
 */
struct HMACKeyByteArray_stt {
	uint8_t *pmAddress;  /*!< pointer to buffer */
	uint32_t mBytesize;  /*!< size in bytes */
};

struct HMAC_key_stt {
	/* @brief HAMC key structure */
	struct HMACKeyByteArray_stt hmacKeyByteArray;

	/** @brief Associated counter value (28 bits only) */
	union {
		uint32_t R;
		struct {
			uint32_t unused : 4;
			uint32_t CNT28 : 28;
		} B;
	} CNT;

	/** @brief description and usage restriction flags */
	union EXTENDED_KEY_flags HMAC_flags;

	/** @brief tell if key location is empty or not */
	uint32_t empty;
};

/*============================================================================*/
/**
 * @brief        Loads an HMAC Key cleartext value in the CSE HMAC RamKey
 *               internal storage
 * @details      Loads a key value to be used when selecting index HMAC_RAM_KEY
 *               for HMAC dedicated commands
 *
 * @param[in]    *P_pSecretKey pointer to a \ref HMACsecretKeyByteArray_stt
 *               structure for the secret key
 *
 * @return       Error code
 * @retval 0     When key was loaded properly
 * @retval 1..21 In case of error - the error code values are the CSE returned
 *               ones
 *               see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note The function is blocking and waits for operation completion
 */
uint32_t CSE_HMAC_loadRamKey(struct HMACKeyByteArray_stt *P_pSecretKey);

/*============================================================================*/
/**
 * @brief        Export HMAC Ram Key
 * @details      Compute and provide all fields required to load the key value
 *               that was in the RAMKEY location in a non volatile key location
 *
 * @param[out]   * P_pM1 Pointer to buffer to store the M1 message
 *               (UID, KeyID, Auth ID - 128 bits total)
 * @param[out]   * P_pM2 Pointer to buffer to store the M2 message
 *               (security flags,  - 256 bits )
 * @param[out]   * P_pM3 Pointer to buffer to store the M3 message
 *               (MAC of M1 and M2 - 128 bits)
 * @param[out]   * P_pM4 Pointer to buffer to store the M4 message
 *               (IDs | counter - 256 bits)
 * @param[out]   * P_pM5 Pointer to buffer to store the M5 message
 *               (MAC of M4 - 128 bits )
 *
 * @return       Error code
 * @retval 0     When key was exported properly
 * @retval 1..21 In case of error - the error code values are the CSE returned
 *               ones
 *
 * @note         The function is blocking and waits for operation completion
 */
uint32_t CSE_HMAC_exportRamKey(uint8_t *P_pM1, uint8_t *P_pM2, uint8_t *P_pM3,
			       uint8_t *P_pM4, uint8_t *P_pM5);

/*============================================================================*/
/**
 * @brief        Loads an HMAC Key in internal non volatile memory
 * @details      Key will be loaded at index described in the already prepared
 *               messages including key usage restrictions (security flags),
 *               keyindex, encrypted value and integrity & authenticity MACs for
 *               all the params.
 *
 * @param[in]    * P_pM1 Pointer to message1
 * @param[in]    * P_pM2 Pointer to message2
 * @param[in]    * P_pM3 Pointer to message3
 *
 * @param[out]   * P_pM4 Pointer to message4
 * @param[out]   * P_pM5 Pointer to message5
 *
 * @return       Error code
 * @retval 0     When key was loaded properly
 * @retval 1..21 In case of error - the error code values are the CSE returned
 *               ones
 *               see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note         The function is blocking and waits for operation completion
 */
uint32_t CSE_HMAC_loadKey(uint8_t *P_pM1, uint8_t *P_pM2, uint8_t *P_pM3,
			  uint8_t *P_pM4, uint8_t *P_pM5);

/**
 * @brief        Perform HMAC Mac Generation
 * @details
 *
 * @param[in]    P_HMAC_keyIdx Identifier of the HMAC Secret Key
 * @param[in]    * P_pMsg        Pointer to message input buffer
 * @param[in]    P_msgSize       Message byte size
 * @param[out]   * P_pMac        Pointer to HMAC MAC Tag descriptor
 *                               (buffer + requested Mac Tag Size)
 * @param[in]*   P_hashAlgo      Hash algorithm involved in HAC computation
 *
 * @return       Error code
 * @retval 0     When HMAC MAC generation operation successes
 * @retval 1..21 In case of error - the error code values are the CSE returned
 *               ones
 *
 * @api
 *
 * @implements
 *
 * @note         The function is blocking and waits for operation completion
 */
uint32_t CSE_HMAC_macGeneration(vuint32_t P_HMAC_keyIdx,
				vuint8_t *P_pMsg,
				uint32_t P_msgSize,
				struct ByteArrayDescriptor_stt *P_pMac,
				vuint32_t P_hashAlgo);

/*============================================================================*/
/**
 * @brief        Perform HMAC Mac verification
 * @details
 *
 * @param[in]    P_HMAC_keyIdx   Identifier of the HMAC Secret Key
 * @param[in]    * P_pMsg        Pointer to message input buffer
 * @param[in]    P_msgSize       Message byte size
 * @param[in]    * P_pMac        Pointer to HMAC MAC Tag descriptor
 *                               (buffer + Mac Tag Size)
 * @param[in]    P_hashAlgo      Hash algorithm involved in HAC computation
 * @param[out]   P_pSuccess      Pointer to result word location
 *                               (0 if failed, 1 if succeeded)
 *
 * @return       Error code
 * @retval 0     When AES256 CMAC Tag verification operation successes
 * @retval 1..21 In case of error - the error code values are the CSE returned
 *               ones
 *
 * @api
 *
 * @implements
 *
 * @note The function is blocking and waits for operation completion
 */
uint32_t CSE_HMAC_macVerification(vuint32_t P_HMAC_keyIdx,
				  vuint8_t *P_pMsg,
				  uint32_t P_msgSize,
				  struct ByteArrayDescriptor_stt *P_pMac,
				  vuint32_t P_hashAlgo,
				  uint32_t *P_pSuccess);

#endif /* _CSE_ext_HMAC_ */
/**
 * @}
 */
