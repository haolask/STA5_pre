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
 * @param  input_msg: pointer to input message to be encrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  iv: Initialization Vector for CBC cipher
 * @param  output_msg: pointer to output parameter that will handle the
 *         encrypted message.
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t aes_cbc_encrypt_hw(const uint8_t *input_msg,
				   uint32_t input_msg_length,
				   const uint8_t *aes_key, uint32_t key_length,
				   const uint8_t *iv, uint8_t *output_msg);

/**
 * @brief  AES CBC Decryption
 * @param  input_msg: pointer to input message to be decrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  iv: Initialization Vector for CBC cipher
 * @param  output_msg: pointer to output parameter that will handle the
 *         decrypted message.
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t aes_cbc_decrypt_hw(const uint8_t *input_msg,
				   uint32_t input_msg_length,
				   const uint8_t *aes_key, uint32_t key_length,
				   const uint8_t *iv, uint8_t *output_msg);

/**
 * @brief  AES CMAC Encryption
 * @param  input_msg: pointer to input message to be encrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  tag_size: length of generated tag
 * @param  output_tag: generated tag
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t aes_cmac_encrypt_hw(const uint8_t *input_msg,
				    uint32_t input_msg_length,
				    const uint8_t *aes_key, uint32_t key_length,
				    uint32_t tag_size, uint8_t *output_tag);

/**
 * @brief  AES CMAC Verification
 * @param  input_msg: pointer to input message to be decrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  tag: associated input message tag
 * @param  tag_size: length of tag
 * @retval error status:
 * @retval AES_ERR_BAD_PARAMETER At least one of the parameters is a NULL
 *         pointer
 * @retval AES_ERR_BAD_INPUT_SIZE At least one of the parameters is of a non
 *         supported size
 * @retval AUTHENTICATION_SUCCESSFUL if the TAG is verified
 * @retval AUTHENTICATION_FAILED if the TAG is \b not verified
 *
 */
extern int32_t aes_cmac_decrypt_hw(const uint8_t *input_msg,
				   uint32_t input_msg_length,
				   const uint8_t *aes_key, uint32_t key_length,
				   const uint8_t *tag, uint32_t tag_size);

/**
 * @brief  AES ECB Encryption
 * @param  input_msg: pointer to input message to be encrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  output_msg: pointer to output parameter that will handle the
 *         encrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t aes_ecb_encrypt_hw(const uint8_t *input_msg,
				   uint32_t input_msg_length,
				   const uint8_t *aes_key, uint32_t key_length,
				   uint8_t *output_msg);

/**
 * @brief  AES ECB Decryption
 * @param  input_msg: pointer to input message to be decrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  output_msg: pointer to output parameter that will handle the
 *         decrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
extern uint32_t aes_ecb_decrypt_hw(const uint8_t *input_msg,
				   uint32_t input_msg_length,
				   const uint8_t *aes_key, uint32_t key_length,
				   uint8_t *output_msg);

#endif //_CSE_AES_HW_MODES_H_

/**
 * @}
 */
