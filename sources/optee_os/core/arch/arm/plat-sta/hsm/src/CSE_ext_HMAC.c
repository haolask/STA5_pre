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
 * @param[in]    *secret_key	pointer to a \ref hmac_key_stt structure
 *				for the secret key
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
uint32_t cse_hmac_load_ramkey(struct hmac_key_stt *secret_key)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/* P1 is the address of location containing the HMAC secret Key */

	ret = cse_hal_send_cmd1p(CSE_HMAC_LOAD_RAM_KEY, (uint32_t)secret_key);

	return ret;
} /* End of cse_hmac_load_ramkey */

/*============================================================================*/
/**
 * @brief        Export HMAC Ram Key
 * @details      Compute and provide all fields required to load the key value
 *               that was in the RAMKEY location in a non volatile key location
 *
 * @param[out]   * M1 Pointer to buffer to store the M1 message
 *               (UID, KeyID, Auth ID - 128 bits total)
 * @param[out]   * M2 Pointer to buffer to store the M2 message
 *               (security flags,  - 256 bits )
 * @param[out]   * M3 Pointer to buffer to store the M3 message
 *               (MAC of M1 and M2 - 128 bits)
 * @param[out]   * M4 Pointer to buffer to store the M4 message
 *               (IDs | counter - 256 bits)
 * @param[out]   * M5 Pointer to buffer to store the M5 message
 *               (MAC of M4 - 128 bits )
 *
 * @return       Error code
 * @retval 0     When key was exported properly
 * @retval 1..21 In case of error - the error code values are the CSE returned
 *               ones
 *
 * @note         The function is blocking and waits for operation completion
 */
uint32_t cse_hmac_export_ramkey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				uint8_t *M4, uint8_t *M5)
{
	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_HMAC_EXPORT_RAM_KEY, (uint32_t)M1,
				 (uint32_t)M2, (uint32_t)M3, (uint32_t)M4,
				 (uint32_t)M5);

	return ret;
} /* End of cse_hmac_export_ramkey */

/*============================================================================*/
/**
 * @brief        Loads an HMAC Key in internal non volatile memory
 * @details      Key will be loaded at index described in the already prepared
 *               messages including key usage restrictions (security flags),
 *               keyindex, encrypted value and integrity & authenticity MACs for
 *               all the params.
 *
 * @param[in]    * M1 Pointer to message1
 * @param[in]    * M2 Pointer to message2
 * @param[in]    * M3 Pointer to message3
 *
 * @param[out]   * M4 Pointer to message4
 * @param[out]   * M5 Pointer to message5
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
uint32_t cse_hmac_load_key(uint8_t *M1, uint8_t *M2, uint8_t *M3,
			   uint8_t *M4, uint8_t *M5)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_HMAC_LOAD_KEY, (uint32_t)M1, (uint32_t)M2,
				 (uint32_t)M3, (uint32_t)M4, (uint32_t)M5);

	return ret;
} /* End of cse_hmac_load_key */

/**
 * @brief        Perform HMAC Mac Generation
 * @details
 *
 * @param[in]    hmac_key_idx    Identifier of the HMAC Secret Key
 * @param[in]    *msg            Pointer to message input buffer
 * @param[in]    msg_size        Message byte size
 * @param[out]   *mac            Pointer to HMAC MAC Tag descriptor
 *                               (buffer + requested Mac Tag Size)
 * @param[in]*   hash_algo       Hash algorithm involved in HAC computation
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
uint32_t cse_hmac_mac_generation(vuint32_t hmac_key_idx, vuint8_t *msg,
				 uint32_t msg_size,
				 struct byte_array_desc_stt *mac,
				 vuint32_t hash_algo)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_HMAC_GENERATE, (uint32_t)hmac_key_idx,
				 (uint32_t)msg, (uint32_t)msg_size,
				 (uint32_t)mac, (uint32_t)hash_algo);

	return ret;
} /* End of cse_hmac_mac_generation */

/*============================================================================*/
/**
 * @brief        Perform HMAC Mac verification
 * @details
 *
 * @param[in]    hmac_key_idx    Identifier of the HMAC Secret Key
 * @param[in]    *msg            Pointer to message input buffer
 * @param[in]    msg_size        Message byte size
 * @param[in]    *mac            Pointer to HMAC MAC Tag descriptor
 *                               (buffer + Mac Tag Size)
 * @param[in]    hash_algo       Hash algorithm involved in HAC computation
 * @param[out]   success         Pointer to result word location
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
uint32_t cse_hmac_mac_verification(vuint32_t hmac_key_idx, vuint8_t *msg,
				   uint32_t msg_size,
				   struct byte_array_desc_stt *mac,
				   vuint32_t hash_algo, uint32_t *success)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_HMAC_VERIFY, (uint32_t)hmac_key_idx,
				 (uint32_t)msg, (uint32_t)msg_size,
				 (uint32_t)mac, (uint32_t)hash_algo);

	/* Return result of HMAC Mac verification operation */
	*success = CSE->P5.R;

	return ret;
} /* End of cse_hmac_mac_verification */

/**
 * @}
 */
