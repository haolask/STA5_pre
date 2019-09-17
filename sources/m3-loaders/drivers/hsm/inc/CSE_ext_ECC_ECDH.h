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
 * @file    CSE_ext_ECC_ECDH.h
 * @brief   CSE ECC ECDH commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */

#ifndef _CSE_EXT_ECC_ECDH_H_
#define _CSE_EXT_ECC_ECDH_H_

/**
  * @brief  Structure type for ECDH Message
  */
struct EcdhPreMasterSecret {
  uint8_t *address;    /*!< pointer to buffer */
  uint32_t byteSize;   /*!< size in bytes */
};

/*============================================================================*/
/**
 * @brief         Perform ECDH Key Agreement
 * @details
 *
 * @param[in]     P_ECDH_key_idx        Identifier of the Public Key
 * @param[out]    P_pEncryptedMessage   Pointer to ref/
 *                EcdhPreMasterSecret: pre-master Key descriptor
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
uint32_t CSE_ECC_ecdhKeyAgreement(vuint32_t P_ECC_key_idx,
                                  struct EcdhPreMasterSecret *pPreMasterSecret);

/*============================================================================*/
/**
 * @brief         Perform ECDH-fixed Key Agreement
 * @details
 * *
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
                                  struct EcdhPreMasterSecret *pPreMasterSecret);

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
                                struct EcdhPreMasterSecret *pPreMasterSecret);
#endif /* _CSE_EXT_ECC_ECDH_H_ */
