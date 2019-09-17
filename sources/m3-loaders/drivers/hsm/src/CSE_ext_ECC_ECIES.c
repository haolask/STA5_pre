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
 * @param[in]     P_ECC_key_idx       Identifier of the Decryption Public Key
 * @param[in]     P_pInputToEncrypt   Pointer to \ref ByteArrayDescriptor_stt:
 *                                    Message to encrypt and size
 * @param[in]     P_pEciesSharedData  Pointer to \ref ECIES_sharedData_stt:
 *                                    Shared Data
 * @param[out]    P_pEncMsg	      Pointer to \ref ECIES_encMsg_stt
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
uint32_t CSE_ECC_EciesEncrypt(vuint32_t P_ECC_key_idx,
			      struct ByteArrayDescriptor_stt *P_pInputToEncrypt,
			      struct ECIES_sharedData_stt *P_pEciesSharedData,
			      struct ECIES_encMsg_stt *P_pEncMsg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_ECC_ECIES_ENCRYPT,
				 (uint32_t)P_ECC_key_idx,
				 (uint32_t)P_pInputToEncrypt,
				 (uint32_t)P_pEciesSharedData,
				 (uint32_t)P_pEncMsg);

	return ret;
}

/*============================================================================*/
/**
 * @brief         Perform ECIES Decryption
 * @details
 * *
 * @param[in]     P_ECC_key_idx       Identifier of the Decryption Private Key
 * @param[in]     P_pInputToDecrypt   Pointer to \ref ECIES_encMsg_stt
 *				      Message to decrypt and size
 * @param[in]     P_pEciesSharedData  Pointer to \ref ECIES_sharedData_stt:
 *				      Shared Data
 * @param[out]    P_pDecMsg Pointer to \ref ByteArrayDescriptor_stt:
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
uint32_t CSE_ECC_EciesDecrypt(vuint32_t P_ECC_key_idx,
			      struct ECIES_encMsg_stt *P_pInputToDecrypt,
			      struct ECIES_sharedData_stt *P_pEciesSharedData,
			      struct ByteArrayDescriptor_stt *P_pDecMsg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_ECC_ECIES_DECRYPT,
				 (uint32_t)P_ECC_key_idx,
				 (uint32_t)P_pInputToDecrypt,
				 (uint32_t)P_pEciesSharedData,
				 (uint32_t)P_pDecMsg);

	return ret;
}

/**
 * @}
 */
