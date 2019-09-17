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
 * @file    CSE_AES_API_sc.c
 * @brief   AES computation functions leveraging CSE AES
 * @details Set of functions used to perform AES operations with CSE, but hiding
 *	    the HW driver.
 *          Only user level parameters are required, and the RAM key is used
 *
 * @addtogroup UserAPI Single Call user level API
 * @{
 * @addtogroup API Functions
 * @{

 */

#include "cse_typedefs.h"
#include "err_codes.h"

#include "CSE_AES_HW_Modes.h"
#include "CSE_Key.h"
#include "CSE_Constants.h"

/**
 * @brief  AES CBC Encryption
 * @param  input_msg: pointer to input message to be encrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  iv: Initialization Vector for CBC cipher
 * @param  output_msg: pointer to output parameter that will handle the
 *	   encrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 *
 */
uint32_t aes_cbc_encrypt_hw(const uint8_t *input_msg, uint32_t input_msg_length,
			    const uint8_t *aes_key, uint32_t key_length,
			    const uint8_t *iv, uint8_t *output_msg)
{
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = input_msg_length / 16;

	if (input_msg_length - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (key_length != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			cse_load_ramkey((uint8_t *)aes_key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			cse_aes_encrypt_cbc(CSE_RAM_KEY,
					    (uint32_t *)input_msg,
					    (uint32_t *)output_msg,
					    (uint32_t *)iv, block_nb, 0);
		}
	}
	return error_status;
}

/**
 * @brief  AES CBC Decryption
 * @param  input_msg: pointer to input message to be decrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  iv: Initialization Vector for CBC cipher
 * @param  output_msg: pointer to output parameter that will handle the
 *	   decrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
uint32_t aes_cbc_decrypt_hw(const uint8_t *input_msg, uint32_t input_msg_length,
			    const uint8_t *aes_key, uint32_t key_length,
			    const uint8_t *iv, uint8_t *output_msg)
{
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = input_msg_length / 16;

	if (input_msg_length - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (key_length != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			cse_load_ramkey((uint8_t *)aes_key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			cse_aes_decrypt_cbc(CSE_RAM_KEY,
					    (uint32_t *)input_msg,
					    (uint32_t *)output_msg,
					    (uint32_t *)iv, block_nb, 0);
		}
	}
	return error_status;
}

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
uint32_t aes_cmac_encrypt_hw(const uint8_t *input_msg,
			     uint32_t input_msg_length, const uint8_t *aes_key,
			     uint32_t key_length, uint32_t tag_size,
			     uint8_t *output_tag)
{
	uint32_t error_status = AES_SUCCESS;
	uint64_t bitlength = 0;

	if (tag_size != 16) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (key_length != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			cse_load_ramkey((uint8_t *)aes_key);
			bitlength = input_msg_length * 8;
			/* RAM_KEY is not an extended key -> last param to 0 */
			error_status = cse_aes_generate_mac(CSE_RAM_KEY,
							&bitlength,
							(uint32_t *)input_msg,
							(uint32_t *)output_tag,
							0);
		}
	}
	return error_status;
}

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
 *	   pointer
 * @retval AES_ERR_BAD_INPUT_SIZE At least one of the parameters is of a non
 *	   supported size
 * @retval AUTHENTICATION_SUCCESSFUL if the TAG is verified
 * @retval AUTHENTICATION_FAILED if the TAG is \b not verified
 *
 */
int32_t aes_cmac_decrypt_hw(const uint8_t *input_msg, uint32_t input_msg_length,
			    const uint8_t *aes_key, uint32_t key_length,
			    const uint8_t *tag, uint32_t tag_size)
{
	uint32_t error_status = AES_SUCCESS;
	uint64_t bitlength = 0;
	uint32_t verification_status = 1;

	if ((!input_msg) || (!aes_key) || (!tag)) {
		error_status = AES_ERR_BAD_PARAMETER;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if ((tag_size != 16) || (key_length != 16)) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			cse_load_ramkey((uint8_t *)aes_key);
			bitlength = input_msg_length * 8;
			/* RAM_KEY is not an extended key -> last param to 0 */
			error_status = cse_aes_verify_mac(CSE_RAM_KEY,
							  &bitlength,
							  (uint32_t *)input_msg,
							  (uint32_t *)tag,
							  tag_size,
							  &verification_status,
							  0);
			if (error_status == 0) {
				if (verification_status == 0) {
					/*
					 * Computed Tag match with provided ref
					 */
					error_status =
						AUTHENTICATION_SUCCESSFUL;
				} else {
					/*
					 * Computed Tag does not match with
					 * provided ref
					 */
					error_status = AUTHENTICATION_FAILED;
				}
			}
		}
	}
	return error_status;
}

/**
 * @brief  AES ECB Encryption
 * @param  input_msg: pointer to input message to be encrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  output_msg: pointer to output parameter that will handle the
 *	   encrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
uint32_t aes_ecb_encrypt_hw(const uint8_t *input_msg, uint32_t input_msg_length,
			    const uint8_t *aes_key, uint32_t key_length,
			    uint8_t *output_msg)
{
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = input_msg_length / 16;

	if (input_msg_length - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (key_length != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			cse_load_ramkey((uint8_t *)aes_key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			cse_aes_encrypt_ecb(CSE_RAM_KEY,
					    (uint32_t *)input_msg,
					    (uint32_t *)output_msg,
					    block_nb, 0);
		}
	}
	return error_status;
}

/**
 * @brief  AES ECB Decryption
 * @param  input_msg: pointer to input message to be decrypted
 * @param  input_msg_length: input data message length in byte
 * @param  aes_key: pointer to the AES key to be used in the operation
 * @param  key_length: length of the AES key
 * @param  output_msg: pointer to output parameter that will handle the
 *	   decrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
uint32_t aes_ecb_decrypt_hw(const uint8_t *input_msg, uint32_t input_msg_length,
			    const uint8_t *aes_key, uint32_t key_length,
			    uint8_t *output_msg)
{
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = input_msg_length / 16;

	if (input_msg_length - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (key_length != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			cse_load_ramkey((uint8_t *)aes_key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			cse_aes_decrypt_ecb(CSE_RAM_KEY,
					    (uint32_t *)input_msg,
					    (uint32_t *)output_msg,
					    block_nb, 0);
		}
	}
	return error_status;
}

/**
 * @}
 * @}
 */

