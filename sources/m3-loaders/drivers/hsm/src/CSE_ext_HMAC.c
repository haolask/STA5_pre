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
 * @file    CSE_ext_HMAC.c
 * @brief   CSE HMAC commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#include <stdlib.h>
#include <string.h>

#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_HMAC.h"

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
uint32_t CSE_HMAC_loadRamKey(struct HMACKeyByteArray_stt *P_pSecretKey)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/* P1 is the address of location containing the HMAC secret Key */

	ret = CSE_HAL_send_cmd1p(CSE_HMAC_LOAD_RAM_KEY, (uint32_t)P_pSecretKey);

	return ret;
} /* End of CSE_HMAC_loadRamKey */

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
			       uint8_t *P_pM4, uint8_t *P_pM5)
{
	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_HMAC_EXPORT_RAM_KEY,
				 (uint32_t)P_pM1, (uint32_t)P_pM2,
				 (uint32_t)P_pM3, (uint32_t)P_pM4,
				 (uint32_t)P_pM5);

	return ret;
} /* End of CSE_HMAC_exportRamKey */

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
			  uint8_t *P_pM4, uint8_t *P_pM5)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_HMAC_LOAD_KEY,
				 (uint32_t)P_pM1, (uint32_t)P_pM2,
				 (uint32_t)P_pM3, (uint32_t)P_pM4,
				 (uint32_t)P_pM5);

	return ret;
} /* End of CSE_HMAC_loadKey */

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
uint32_t CSE_HMAC_macGeneration(vuint32_t P_HMAC_keyIdx, vuint8_t *P_pMsg,
				uint32_t P_msgSize,
				struct ByteArrayDescriptor_stt *P_pMac,
				vuint32_t P_hashAlgo)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_HMAC_GENERATE, (uint32_t)P_HMAC_keyIdx,
				 (uint32_t)P_pMsg, (uint32_t)P_msgSize,
				 (uint32_t)P_pMac, (uint32_t)P_hashAlgo);

	return ret;
} /* End of CSE_HMAC_macGeneration */

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
uint32_t CSE_HMAC_macVerification(vuint32_t P_HMAC_keyIdx, vuint8_t *P_pMsg,
				  uint32_t P_msgSize,
				  struct ByteArrayDescriptor_stt *P_pMac,
				  vuint32_t P_hashAlgo, uint32_t *P_pSuccess)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_HMAC_VERIFY, (uint32_t)P_HMAC_keyIdx,
				 (uint32_t)P_pMsg, (uint32_t)P_msgSize,
				 (uint32_t)P_pMac, (uint32_t)P_hashAlgo);

	/* Return result of HMAC Mac verification operation */
	*P_pSuccess = CSE->P5.R;

	return ret;
} /* End of CSE_HMAC_macVerification */

/**
 * @}
 */
