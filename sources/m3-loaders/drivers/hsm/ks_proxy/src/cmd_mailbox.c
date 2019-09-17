/*
 *  Copyright (C) 2018 STMicroelectronics
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
 * @file    cmd_mailbox.c
 * @brief   Command Mailbox abstraction source file.
 *
 * @addtogroup CMD_MAILBOX
 * @{
 */

#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include "sta_mtu.h"

#define RESERVE_MBX_BUFFER
#include "cmd_mailbox.h"

/*
 * Key encription message capability enable
 */
#define ENCRYPTION                      0

/*
 * Public variables
 */
union t_mbx_cmd_data mb_cmd_data;
uint8_t mb_cmd;

/*!
 * @brief		Sends Key Storage commands
 * @details
 *
 * @param[in]	cmd     pointer to input message buffer
 * @param[in]   data    pointer of data
 *
 * @return      Command status
 * @retval      0	When the command is well managed
 * @retval      1..5	In case of error
 */
mbxsts_t mbxSendCommand(mbxcmd_t cmd, uint8_t *data)
{
	struct mtu_device *mtu;

	/* Setting up the command buffer, including the (optional) data. */
	g_mbx_buffer.cmd = cmd;
	if (data != NULL)
		memcpy(g_mbx_buffer.data, data, sizeof(g_mbx_buffer.data));

	/* Notifying that a command is ready. */
	notify();

	/* Configure and start timer to expire after Timeout value */
	mtu = mtu_start_timer(MBX_TIMEOUT * 1000, ONE_SHOT_MODE, NULL);

	/* Waiting until an answer comes or a timeout happens. */
	while (mtu_read_timer_value(mtu)) {
		/* Checking for the answer. */
		if (answer_notification()) {
			/* Clearing answer flag, it can be done only on this side. */
			clear_notification();

			/* Copying back the data buffer in case the answer carries data. */
			if (data != NULL)
				memcpy(data, g_mbx_buffer.data,
				       sizeof(g_mbx_buffer.data));

			return g_mbx_buffer.sts;
		}
	}

	return MBX_STS_INTERNAL;
}

#if ENCRYPTION == 1
/**
 * @brief         performs the CBC encryption of the provided block
 *                with provided AES256 key & IV
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 *
 */
uint32_t m3_cryCrypt(uint8_t *dest, const uint8_t *src, uint32_t size,
		     uint8_t* SSKey_cryp, uint8_t* piv )
{
	uint32_t ret = AES_ERR_BAD_PARAMETER;
	AES256secretKeyByteArray_stt aes256SecretKey;
	ByteArrayDescriptor_stt messageToEncrypt;
	ByteArrayDescriptor_stt cipherMessage;
	ByteArrayDescriptor_stt iv;

	if ((size % 16) != 0) {
		/* size is not correct, must be a multiple of 16 */
	} else {
		aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_cryp;
		aes256SecretKey.mSecretKeySize = 32;

		/* Initialize Initialization Vector */
		iv.address = piv;
		iv.byteSize = 16;

		messageToEncrypt.address = (uint8_t*)src;
		messageToEncrypt.byteSize = size;

		cipherMessage.address = dest;
		cipherMessage.byteSize = size;

		ret = aes256_cbc_encryption(&aes256SecretKey, &messageToEncrypt,
					    &cipherMessage, &iv, size / 16, 0);
	}
	return (ret);
}

/**
 * @brief         performs the CBC decryption of the provided block
 *                with provided AES256 key & IV
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 *
 */
uint32_t m3_cryDecrypt(uint8_t *dest, const uint8_t *src, uint32_t size,
		       uint8_t* SSKey_cryp, uint8_t* piv )
{
	uint32_t ret = AES_ERR_BAD_PARAMETER;
	AES256secretKeyByteArray_stt aes256SecretKey;
	ByteArrayDescriptor_stt messageToDecrypt;
	ByteArrayDescriptor_stt cleartextMessage;
	ByteArrayDescriptor_stt iv;

	if ((size % 16) != 0) {
		/* size is not correct, must be a multiple of 16 */
	} else {
		aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_cryp;
		aes256SecretKey.mSecretKeySize = 32;

		/* Initialize Initialization Vector */
		iv.address = piv;
		iv.byteSize = 16;

		messageToDecrypt.address = (uint8_t*)src;
		messageToDecrypt.byteSize = size;

		cleartextMessage.address = dest;
		cleartextMessage.byteSize = size;

		ret = aes256_cbc_decryption(&aes256SecretKey,
					    &messageToDecrypt,
					    &cleartextMessage,
					    &iv, size / 16, 0);
	}
	return (ret);
}

/**
 * @brief         performs the external key image tag generation of the provided block with provided AES256 key
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]     size        size of the text (must be a multiple of 16)
 * @param[in]     src         text to be 'signed' (it's a tag)
 * @param[out]    signature   the signature buffer  (must be 32bytes long)
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 *
 */
uint32_t m3_crySign(const uint8_t *src, uint32_t size,
		    uint8_t *signature, uint8_t* SSKey_auth )
{
	uint32_t ret = AES_ERR_BAD_PARAMETER;
	AES256secretKeyByteArray_stt aes256SecretKey;
	AES_CMAC_messageData_stt messageToMAC;
	AES_CMAC_tagData_stt tag;

	if ((size % 16) != 0) {
		/* size is not correct, must be a multiple of 16 */
	} else {
		aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_auth;
		aes256SecretKey.mSecretKeySize = 32;

		messageToMAC.pAddress = (uint8_t*)src;
		messageToMAC.messageByteSize = size;

		tag.pAddress = signature;

		ret = aes256_cmac_tagGeneration(&aes256SecretKey, 16,
						&messageToMAC, &tag, 0);
		memcpy( signature + 16, signature, 16 );
	}
	return (ret);
}

/**
 * @brief         performs the external key image tag generation of the provided block with provided AES256 key
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]     size          size of the text (must be a multiple of 16)
 * @param[in]     src           text to be 'signed' (it's a tag)
 * @param[out]    signature     the signature buffer  (must be 32bytes long)
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 * @retval        CSE_GENERAL_ERR if signature does not match
 *
 */
uint32_t m3_cryVerify(const uint8_t *src, uint32_t size,
		      uint8_t *signature, uint8_t* SSKey_auth)
{
	uint32_t ret = AES_ERR_BAD_PARAMETER;
	AES256secretKeyByteArray_stt aes256SecretKey;
	AES_CMAC_messageData_stt messageToMAC;
	AES_CMAC_tagData_stt tag;

	uint8_t computed_tag[32];

	if ((size % 16) != 0) {
		/* size is not correct, must be a multiple of 16 */
	} else {
		aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_auth;
		aes256SecretKey.mSecretKeySize = 32;

		messageToMAC.pAddress = (uint8_t*)src;
		messageToMAC.messageByteSize = size;

		tag.pAddress = computed_tag;

		ret = aes256_cmac_tagGeneration(&aes256SecretKey, 16,
						&messageToMAC, &tag, 0);
		memcpy(computed_tag + 16, computed_tag, 16);

		/* compare */
		if (memcmp(computed_tag, signature, 32) == 0)
			ret = CSE_NO_ERR;
		else
			ret = CSE_GENERAL_ERR;
	}
	return ret;
}
#endif

/** @} */
