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
 * @param  InputMessage: pointer to input message to be encrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  IV: Initialization Vector for CBC cipher
 * @param  OutputMessage: pointer to output parameter that will handle the
 *	   encrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 *
 */
uint32_t AES_CBC_Encrypt_HW(const uint8_t *InputMessage,
			    uint32_t InputMessageLength,
			    const uint8_t *AES_Key,
			    uint32_t KeyLength,
			    const uint8_t *IV,
			    uint8_t *OutputMessage)
{
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = InputMessageLength / 16;

	if (InputMessageLength - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (KeyLength != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			CSE_LoadRamKey((uint8_t *)AES_Key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			CSE_AES_encrypt_CBC(CSE_RAM_KEY,
					    (uint32_t *)InputMessage,
					    (uint32_t *)OutputMessage,
					    (uint32_t *)IV, block_nb, 0);
		}
	}
	return error_status;
}

/**
 * @brief  AES CBC Decryption
 * @param  InputMessage: pointer to input message to be decrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  IV: Initialization Vector for CBC cipher
 * @param  OutputMessage: pointer to output parameter that will handle the
 *	   decrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
uint32_t AES_CBC_Decrypt_HW(const uint8_t *InputMessage,
			    uint32_t InputMessageLength,
			    const uint8_t *AES_Key,
			    uint32_t KeyLength,
			    const uint8_t *IV,
			    uint8_t *OutputMessage) {
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = InputMessageLength / 16;

	if (InputMessageLength - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (KeyLength != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			CSE_LoadRamKey((uint8_t *)AES_Key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			CSE_AES_decrypt_CBC(CSE_RAM_KEY,
					    (uint32_t *)InputMessage,
					    (uint32_t *)OutputMessage,
					    (uint32_t *)IV, block_nb, 0);
		}
	}
	return error_status;
}

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
uint32_t AES_CMAC_Encrypt_HW(const uint8_t *InputMessage,
			     uint32_t InputMessageLength,
			     const uint8_t *AES_Key,
			     uint32_t KeyLength,
			     uint32_t TagSize,
			     uint8_t *OutputTag) {
	uint32_t error_status = AES_SUCCESS;
	uint64_t bitlength = 0;

	if (TagSize != 16) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (KeyLength != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			CSE_LoadRamKey((uint8_t *)AES_Key);
			bitlength = InputMessageLength * 8;
			/* RAM_KEY is not an extended key -> last param to 0 */
			error_status = CSE_AES_generate_MAC(CSE_RAM_KEY,
						      &bitlength,
						      (uint32_t *)InputMessage,
						      (uint32_t *)OutputTag, 0);
		}
	}
	return error_status;
}

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
 *	   pointer
 * @retval AES_ERR_BAD_INPUT_SIZE At least one of the parameters is of a non
 *	   supported size
 * @retval AUTHENTICATION_SUCCESSFUL if the TAG is verified
 * @retval AUTHENTICATION_FAILED if the TAG is \b not verified
 *
 */
int32_t AES_CMAC_Decrypt_HW(const uint8_t *InputMessage,
			    uint32_t InputMessageLength,
			    const uint8_t *AES_Key,
			    uint32_t KeyLength,
			    const uint8_t *tag,
			    uint32_t TagSize) {
	uint32_t error_status = AES_SUCCESS;
	uint64_t bitlength = 0;
	uint32_t verification_status = 1;

	if ((!InputMessage) || (!AES_Key) || (!tag)) {
		error_status = AES_ERR_BAD_PARAMETER;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if ((TagSize != 16) || (KeyLength != 16)) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			CSE_LoadRamKey((uint8_t *)AES_Key);
			bitlength = InputMessageLength * 8;
			/* RAM_KEY is not an extended key -> last param to 0 */
			error_status = CSE_AES_verify_MAC(CSE_RAM_KEY,
						      &bitlength,
						      (uint32_t *)InputMessage,
						      (uint32_t *)tag, TagSize,
						      &verification_status, 0);
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
 * @param  InputMessage: pointer to input message to be encrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  OutputMessage: pointer to output parameter that will handle the
 *	   encrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
uint32_t AES_ECB_Encrypt_HW(const uint8_t *InputMessage,
			    uint32_t InputMessageLength,
			    const uint8_t *AES_Key,
			    uint32_t KeyLength,
			    uint8_t *OutputMessage) {
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = InputMessageLength / 16;

	if (InputMessageLength - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (KeyLength != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			CSE_LoadRamKey((uint8_t *)AES_Key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			CSE_AES_encrypt_ECB(CSE_RAM_KEY,
					    (uint32_t *)InputMessage,
					    (uint32_t *)OutputMessage,
					    block_nb, 0);
		}
	}
	return error_status;
}

/**
 * @brief  AES ECB Decryption
 * @param  InputMessage: pointer to input message to be decrypted
 * @param  InputMessageLength: input data message length in byte
 * @param  AES_Key: pointer to the AES key to be used in the operation
 * @param  KeyLength: length of the AES key
 * @param  OutputMessage: pointer to output parameter that will handle the
 *	   decrypted message
 * @retval error status: can be AES_SUCCESS if success or one of
 *         AES_ERR_BAD_INPUT_SIZE, AES_ERR_BAD_OPERATION, AES_ERR_BAD_CONTEXT
 *         AES_ERR_BAD_PARAMETER if error occurred.
 */
uint32_t AES_ECB_Decrypt_HW(const uint8_t *InputMessage,
			    uint32_t InputMessageLength,
			    const uint8_t *AES_Key,
			    uint32_t KeyLength,
			    uint8_t *OutputMessage) {
	uint32_t error_status = AES_SUCCESS;
	uint32_t block_nb = 0;

	/* Check length - must be a multiple of 16 bytes */
	block_nb = InputMessageLength / 16;

	if (InputMessageLength - (block_nb * 16) != 0) {
		error_status = AES_ERR_BAD_INPUT_SIZE;
	} else {
		/* Check key length - only AES128 is supported by CSE HW  */
		if (KeyLength != 16) {
			error_status = AES_ERR_BAD_INPUT_SIZE;
		} else {
			CSE_LoadRamKey((uint8_t *)AES_Key);
			/* RAM_KEY is not an extended key -> last param to 0 */
			CSE_AES_decrypt_ECB(CSE_RAM_KEY,
					    (uint32_t *)InputMessage,
					    (uint32_t *)OutputMessage,
					    block_nb, 0);
		}
	}
	return error_status;
}

/**
 * @}
 * @}
 */

