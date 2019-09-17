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
 * @file    CSE_ext_ECC.c
 * @brief   CSE ECC commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */

#include "cse_typedefs.h"
#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_ECC.h"

/*============================================================================*/
/**
 * @brief         Perform ECIES Encryption
 * @details
 * *
 * @param[in]     ecc_key_idx         Identifier of the Decryption Public Key
 * @param[in]     *pinput_toencrypt   Pointer to \ref byte_array_desc_stt:
 *                                    Message to encrypt and size
 * @param[in]     *pecies_shared_data Pointer to \ref ecies_shared_data_stt:
 *                                    Shared Data
 * @param[out]    *penc_msg	      Pointer to \ref ecies_enc_msg_stt
 *                                    Encrypted message and Tag
 *
 * @return        Error code
 * @retval 0      When ECIES encryption was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecies_encrypt(vuint32_t ecc_key_idx,
			       struct byte_array_desc_stt *pinput_toencrypt,
			       struct ecies_shared_data_stt *pecies_shared_data,
			       struct ecies_enc_msg_stt *penc_msg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_ECC_ECIES_ENCRYPT,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)pinput_toencrypt,
				 (uint32_t)pecies_shared_data,
				 (uint32_t)penc_msg);

	return ret;
}

/*============================================================================*/
/**
 * @brief         Perform ECIES Decryption
 * @details
 * *
 * @param[in]     ecc_key_idx         Identifier of the Decryption Private Key
 * @param[in]     *pinput_toencrypt   Pointer to \ref ecies_enc_msg_stt
 *				      Message to decrypt and size
 * @param[in]     *pecies_shared_data Pointer to \ref ecies_shared_data_stt:
 *				      Shared Data
 * @param[out]    *pdec_msg           Pointer to \ref byte_array_desc_stt:
 *				      Decrypted message
 *
 * @return        Error code
 * @retval 0      When ECIES decryption was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecies_decrypt(vuint32_t ecc_key_idx,
			       struct ecies_enc_msg_stt *pinput_toencrypt,
			       struct ecies_shared_data_stt *pecies_shared_data,
			       struct byte_array_desc_stt *pdec_msg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_ECC_ECIES_DECRYPT,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)pinput_toencrypt,
				 (uint32_t)pecies_shared_data,
				 (uint32_t)pdec_msg);

	return ret;
}

/**
 * @}
 */
