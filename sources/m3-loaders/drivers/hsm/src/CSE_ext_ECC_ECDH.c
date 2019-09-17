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
 * @file    CSE_ext_ECC_ECDH.c
 * @brief   CSE ECC ECDH commands management module.
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
#include "CSE_ext_ECC_ECDH.h"

/*============================================================================*/
/**
 * @brief         Perform ECDH Key Agreement
 * @details
 * *
 * @param[in]       P_ECDH_key_idx      Identifier of the Public Key
 * @param[out]      P_pEncryptedMessage Pointer to \ref
 *                  EcdhPreMasterSecret: pre-master Secret descriptor
 *
 * @return          Error code
 * @retval 0        When ECDH key agreement was successfully performed
 * @retval 1..21    In case of error - the error code values are the CSE
 *                  returned ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ecdhKeyAgreement(vuint32_t P_ECC_key_idx,
                                  struct EcdhPreMasterSecret *pPreMasterSecret)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd2p(CSE_ECC_ECDH_KEY_AGREEMENT,
				 (uint32_t)P_ECC_key_idx,
				 (uint32_t)pPreMasterSecret);

	return ret;
}

/*============================================================================*/
/**
 * @brief         Perform ECDH-fixed Key Agreement
 * @details
 *
 * @param[in]     P_ECDH_privKey_idx  Identifier of the Private Key
 * @param[out]    P_pEncryptedMessage Pointer to \ref
 *                EcdhPreMasterSecret: pre-master Secret descriptor
 *
 * @return        Error code
 * @retval 0      When ECDH key agreement was successfully performed
 * @retval 1..21  In case of error - the error code values are the CSE returned ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ecdhFixedKeyAgreement(
                                  vuint32_t P_ECC_privKey_idx,
                                  struct EcdhPreMasterSecret *pPreMasterSecret)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd2p(CSE_ECC_ECDHfixed_KEY_AGREEMENT,
				 (uint32_t)P_ECC_privKey_idx,
				 (uint32_t)pPreMasterSecret);

	return ret;
} /* End of CSE_ECC_ecdhfixedKeyAgreement */

/*============================================================================*/
/**
 * @brief         Perform ECDH-fixed Server Key Agreement
 * @details
 * *
 * @param[in]     P_pClientPubKey  Pointer \ref
 *                ECCpubKeyByteArray_stt to Public Key descriptor
 * @param[out]    P_pEncryptedMessage Pointer to \ref
 *                EcdhPreMasterSecret: pre-master Secret descriptor
 *
 * @return        Error code
 * @retval 0      When ECDH key agreement was successfully performed
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ecdhFixedServerKeyAgreement(
                                struct ECCpubKeyByteArray_stt * P_pClientPubKey,
                                struct EcdhPreMasterSecret *pPreMasterSecret)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd2p(CSE_ECC_ECDHfixed_SERVER_KEY_AGREEMENT,
				 (uint32_t)P_pClientPubKey,
				 (uint32_t)pPreMasterSecret);

	return ret;
} /* End of CSE_ECC_ecdhFixedServerKeyAgreement */

/**
 * @}
 */

