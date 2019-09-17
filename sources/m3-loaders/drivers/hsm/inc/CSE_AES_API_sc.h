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
 * @file    CSE_AES_API_sc.h
 * @brief   AES computation functions leveraging CSE AES - header
 * @details Set of functions used to perform AES operations with CSE, but hiding
 *          the HW driver.
 *          Only user level parameters are required, and the RAM key is used
 *
 *
 * @addtogroup UserAPI Single Call user level API
 * @{
 */

#ifndef _CSE_AES_API_SC_H_
#define _CSE_AES_API_SC_H_

#include "cse_typedefs.h"

/**
 * @brief  AES CBC Encryption
 * @param  InputMessage: pointer to input message to be encrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  IV: Initialization Vector for CBC cipher
 * @param  OutputMessage: pointer to output parameter that will handle the
 *         encrypted message.
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t AES_CBC_Encrypt_HW(const uint8_t *InputMessage,
				   uint32_t InputMessageLength,
				   const uint8_t *AES_Key, uint32_t KeyLength,
				   const uint8_t *IV, uint8_t *OutputMessage);

/**
 * @brief  AES CBC Decryption
 * @param  InputMessage: pointer to input message to be decrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  IV: Initialization Vector for CBC cipher
 * @param  OutputMessage: pointer to output parameter that will handle the
 *         decrypted message.
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t AES_CBC_Decrypt_HW(const uint8_t *InputMessage,
				   uint32_t InputMessageLength,
				   const uint8_t *AES_Key, uint32_t KeyLength,
				   const uint8_t *IV, uint8_t *OutputMessage);

/**
 * @brief  AES CMAC Encryption
 * @param  InputMessage: pointer to input message to be encrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  TagSize: length of generated tag
 * @param  OutputTag: generated tag
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t AES_CMAC_Encrypt_HW(const uint8_t *InputMessage,
				    uint32_t InputMessageLength,
				    const uint8_t *AES_Key, uint32_t KeyLength,
				    uint32_t TagSize, uint8_t *OutputTag);

/**
 * @brief  AES CMAC Verification
 * @param  InputMessage: pointer to input message to be decrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  tag: associated input message tag
 * @param  TagSize: length of tag
 * @retval error status:
 * @retval AES_ERR_BAD_PARAMETER At least one of the parameters is a NULL
 *         pointer
 * @retval AES_ERR_BAD_INPUT_SIZE At least one of the parameters is of a non
 *         supported size
 * @retval AUTHENTICATION_SUCCESSFUL if the TAG is verified
 * @retval AUTHENTICATION_FAILED if the TAG is \b not verified
 *
 */
extern int32_t AES_CMAC_Decrypt_HW(const uint8_t *InputMessage,
				   uint32_t InputMessageLength,
				   const uint8_t *AES_Key, uint32_t KeyLength,
				   const uint8_t *tag, uint32_t TagSize);

/**
 * @brief  AES ECB Encryption
 * @param  InputMessage: pointer to input message to be encrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  OutputMessage: pointer to output parameter that will handle the
 *         encrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t AES_ECB_Encrypt_HW(const uint8_t *InputMessage,
				   uint32_t InputMessageLength,
				   const uint8_t *AES_Key, uint32_t KeyLength,
				   uint8_t *OutputMessage);

/**
 * @brief  AES ECB Decryption
 * @param  InputMessage: pointer to input message to be decrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  OutputMessage: pointer to output parameter that will handle the
 *         decrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t AES_ECB_Decrypt_HW(const uint8_t *InputMessage,
				   uint32_t InputMessageLength,
				   const uint8_t *AES_Key, uint32_t KeyLength,
				   uint8_t *OutputMessage);

#endif //_CSE_AES_HW_MODES_H_

/**
 * @}
 */
