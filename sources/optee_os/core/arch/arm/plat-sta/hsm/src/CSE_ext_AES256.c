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
 * @file    CSE_ext_AES256.c
 * @brief   CSE AES256 commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#include <stdlib.h>

#include "cse_typedefs.h"
#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_AES256.h"
#include <string.h>

/*============================================================================*/
/**
 * @brief         Loads an AES256 Key cleartext value in the CSE AES256 RamKey
 *		  internal storage
 * @details       Loads a key value to be used when selecting index
 *		  AES256_RAM_KEY for AES256 dedicated commands
 *
 * @param[in]     secretkey    pointer to a \ref aes256_secretkey_stt
 *		  structure for the secret key
 *
 * @return        Error code
 * @retval 0      When key was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *                see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_load_ramkey(struct aes256_secretkey_stt *secretkey)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/* P1 is the address of location containing the AES256 secret Key */

	ret = cse_hal_send_cmd1p(CSE_AES256_LOAD_RAM_KEY,
				 (uint32_t)secretkey);

	return ret;
} /* End of cse_aes256_load_ramkey */

/*============================================================================*/
/**
 * @brief		Export AES256 Ram Key
 * @details		Compute and provide all fields required to load the key
 *			value that was in the RAMKEY location in a non volatile
 *			key location
 *
 * @param[out] M1       Pointer to buffer to store the M1 message
 *			(UID, KeyID, Auth ID - 128 bits total)
 * @param[out] M2       Pointer to buffer to store the M2 message
 *			(security flags,  - 256 bits )
 * @param[out] M3       Pointer to buffer to store the M3 message
 *			(MAC of M1 and M2 - 128 bits)
 * @param[out] M4       Pointer to buffer to store the M4 message
 *			(IDs | counter - 256 bits)
 * @param[out] M5       Pointer to buffer to store the M5 message
 *			(MAC of M4 - 128 bits )
 *
 * @return		Error code
 * @retval 0		When key was exported properly
 * @retval 1..21	In case of error - the error code values are the
 *			CSE returned ones
 *
 * @note		The function is blocking and waits for operation
 *			completion
 */
uint32_t cse_aes256_export_ramkey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				  uint8_t *M4, uint8_t *M5)
{
	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_AES256_EXPORT_RAM_KEY,
				 (uint32_t)M1, (uint32_t)M2, (uint32_t)M3,
				 (uint32_t)M4, (uint32_t)M5);

	return ret;
} /* End of cse_aes256_export_ramkey */

/*============================================================================*/
/**
 * @brief         Loads an AES256 Key in internal non volatile memory
 * @details       Key will be loaded at index described in the already prepared
 *		  messages
 *                including key usage restrictions (security flags), keyindex,
 *		  encrypted value and integrity & authenticity MACs for all the
 *		  params.
 *
 * @param[in]     M1    pointer to message1
 * @param[in]     M2    pointer to message2
 * @param[in]     M3    pointer to message3
 *
 * @param[out]    M4    pointer to message4
 * @param[out]    M5    pointer to message5
 *
 * @return        Error code
 * @retval 0      When key was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *                see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_load_key(uint8_t *M1, uint8_t *M2, uint8_t *M3,
			     uint8_t *M4, uint8_t *M5)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_AES256_LOAD_KEY, (uint32_t)M1,
				 (uint32_t)M2, (uint32_t)M3, (uint32_t)M4,
				 (uint32_t)M5);

	return ret;
} /* End of cse_aes256_load_key */

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in ECB mode
 * @details
 *
 * @param[in]     aes256_key_idx      Identifier of the Secret Key
 * @param[in]     *psrc               Pointer to Message to encrypt
 * @param[out]    *pdest              Pointer to Encrypted message
 * @param[in]     block_count         Length in byte of the input message to
 *				      cipher
 *
 * @return        Error code
 * @retval 0      When AES256 decryption was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_ecb_encrypt(vuint32_t aes256_key_idx, vuint8_t *psrc,
				vuint8_t *pdest, vuint32_t block_count)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_AES256_ECB_ENCRYPT,
				 (uint32_t)aes256_key_idx, block_count,
				 (uint32_t)psrc, (uint32_t)pdest);

	return ret;
} /* End of cse_aes256_ecb_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in ECB mode
 * @details
 *
 * @param[in]     aes256_key_idx      Identifier of the Secret Key
 * @param[in]     *psrc               Pointer to Message to decrypt
 * @param[out]    *pdest              Pointer to destination buffer cleartext
 *				      message
 * @param[in]     block_count         Length in byte of the input message to
 *				      cipher
 *
 * @return        Error code
 * @retval 0      When AES256 decryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_ecb_decrypt(vuint32_t aes256_key_idx, vuint8_t *psrc,
				vuint8_t *pdest, vuint32_t block_count)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_AES256_ECB_DECRYPT,
				 (uint32_t)aes256_key_idx, block_count,
				 (uint32_t)psrc, (uint32_t)pdest);

	return ret;
} /* End of cse_aes256_ecb_decrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in CBC mode
 * @details
 *
 * @param[in]     aes256_key_idx      Identifier of the Secret Key
 * @param[in]     *psrc               Pointer to Message to encrypt
 * @param[out]    *pdest              Pointer to destination buffer Encrypted
 *				      message
 * @param[in]     *piv                Pointer to Initialization Vector
 * @param[in]     block_count         Length in byte of the input message to
 *				      cipher
 *
 * @return        Error code
 * @retval 0      When AES256 encryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_cbc_encrypt(vuint32_t aes256_key_idx, vuint8_t *psrc,
				vuint8_t *pdest, vuint8_t *piv,
				vuint32_t block_count)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_AES256_CBC_ENCRYPT,
				 (uint32_t)aes256_key_idx, (uint32_t)piv,
				 block_count, (uint32_t)psrc, (uint32_t)pdest);

	return ret;
} /* End of cse_aes256_cbc_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in CBC mode
 * @details
 * *
 * @param[in]     aes256_key_idx      Identifier of the Secret Key
 * @param[in]     *psrc               Pointer to Message to decrypt
 * @param[out]    *pdest              Pointer to destination buffer cleartext
 *				      message
 * @param[in]     *piv                Pointer to Initialization Vector
 * @param[in]     blockCount          Length in byte of the input message to
 *				      cipher
 *
 * @return        Error code
 * @retval 0      When AES256 decryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_cbc_decrypt(vuint32_t aes256_key_idx, vuint8_t *psrc,
				vuint8_t *pdest, vuint8_t *piv,
				vuint32_t block_count)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_AES256_CBC_DECRYPT,
				 (uint32_t)aes256_key_idx, (uint32_t)piv,
				 block_count, (uint32_t)psrc, (uint32_t)pdest);

	return ret;
} /* End of cse_aes256_cbc_decrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in CCM mode
 * @details       Perform CCM process with pre-formatted Additional Data.
 *		  The Additional Data are formated in this functions to
 *                avoid non-aligned access due to the concatenation of 2 or 6
 *		  bytes encoding the AD size.
 *
 * @param[in]     aes256_key_idx   Identifier of the Secret Key
 * @param[in]     *pheader         Pointer to \ref aes_ccm_header_data_stt
 *				   structure for Nonce and Associated Data
 * @param[out]    tag_size         Size of Tag used in authentication
 * @param[in]     *pmsg            Pointer to \ref aes_ccm_msg_stt
 *				   structure containing the message to encrypt
 * @param[in]     *pciph_msg       Pointer to \ref aes_ccm_msg_stt
 *				   structure receiving the encrypted message
 *
 * @return        Error code
 * @retval 0      When AES256 encryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_ccm_encrypt(vuint32_t aes256_key_idx,
				const struct aes_ccm_header_data_stt *pheader,
				const uint32_t tag_size,
				const struct aes_ccm_msg_stt *pmsg,
				struct aes_ccm_msg_stt *pciph_msg)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * Buffer for pre-formatted Associated Data, formatted size |
	 * Associated Data
	 */
	unsigned char *data_buf = NULL;

#if 0
	DataBuf = (unsigned char *)calloc(p_Header->data_size + 6U,
					  sizeof(unsigned char));
#else
	unsigned char data_buf_local[64];

	if ((pheader->data_size + 6U) <= 64)
		data_buf = data_buf_local;
	else
		data_buf = (unsigned char *)calloc(pheader->data_size + 6U,
						  sizeof(unsigned char));
#endif

	if (data_buf) {
		/* Pre formatted Header data, Nonce and associated data */
		struct aes_ccm_header_data_stt header_data;
		uint32_t buf_used = 0U;
		uint32_t i = 0U;

		/* Unchanged for Nonce field */
		header_data.pnonce = pheader->pnonce;
		header_data.nonce_size = pheader->nonce_size;

		/* Initialize pointer to pre-formatted Associated data */
		header_data.pdata = data_buf;

		/*
		 * Formatting the Associated Data: add the encoded size of
		 * Associated Data to the buffer and concatenate the data
		 */
		if (pheader->data_size != 0U) {
			if (pheader->data_size < 0x0000FF00U) {
			/* Length of Associated Data < [2^(16) - 2^8] */
			header_data.pdata[0] = (unsigned char)
				((pheader->data_size >> 8) & 0xFF);
			header_data.pdata[1] = (unsigned char)
				(pheader->data_size & 0xFF);
			/*
			 * Two additional byte for the pre-formatted
			 * associated Data size
			 */
			header_data.data_size += 2U;
			buf_used = 2U;
			} else {
				/* Length of Associated Data > [2^(16) - 2^8] */
				header_data.pdata[0] = 0xFF;
				header_data.pdata[1] = 0xFE;
				header_data.pdata[2] = (unsigned char)
					((pheader->data_size >> 24) & 0xFF);
				header_data.pdata[3] = (unsigned char)
					((pheader->data_size >> 16) & 0xFF);
				header_data.pdata[4] = (unsigned char)
					((pheader->data_size >> 8) & 0xFF);
				header_data.pdata[5] = (unsigned char)
					(pheader->data_size & 0xFF);
				/*
				 * Six additional bytes for the size of the
				 * pre-formatted Associated Data
				 */
				header_data.data_size += 6U;
				buf_used = 6U;
			}
		}

		/*
		 * Copy Associated Data in pre-formatted buffer
		 * after encoded length
		 */
		for (i = 0U; i < pheader->data_size; i++)
			header_data.pdata[buf_used + i] = pheader->pdata[i];

		/* Update size of Associated Data */
		header_data.data_size = pheader->data_size + buf_used;

		/*
		 * Launch CCM encrypt with pre-formatted buffer for Associated
		 * Data
		 */
		ret = cse_hal_send_cmd5p(CSE_AES256_CCM_ENCRYPT,
					 (uint32_t)aes256_key_idx,
					 (uint32_t)&header_data,
					 tag_size,
					 (uint32_t)pmsg,
					 (uint32_t)pciph_msg);

#if 0
		/* Free allocated buffer */
		free(DataBuf);
#else
		if ((pheader->data_size + 6U) > 64)
			free(data_buf);
#endif
	} else {
		ret = CSE_GENERAL_ERR;
	}

	return ret;
} /* End of cse_aes256_ccm_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in CCM mode
 * @details       Perform CCM process with pre-formatted Additional Data.
 *		  The Additional Data are formated in this functions to
 *                avoid non-aligned access due to the concatenation of 2 or 6
 *		  bytes encoding the AD size.
 *
 * @param[in]     aes256_key_idx      Identifier of the Secret Key
 * @param[in]     *pheader            Pointer to \ref aes_ccm_header_data_stt
 *				      structure for Nonce and Associated Data
 * @param[out]    tag_size            Size of Tag used in authentication
 * @param[in]     *pciph_msg          Pointer to \ref aes_ccm_msg_stt
 *				      structure containing the message to
 *				      decrypt
 * @param[in]     *ppayload_msg       Pointer to \ref
 *				      aes_ccm_deciph_msg_stt
 *				      structure receiving the decrypted message,
 *				      i.e. the payload
 *
 * @return        Error code
 * @retval 0      When AES256 encryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_ccm_decrypt(vuint32_t aes256_key_idx,
				const struct aes_ccm_header_data_stt *pheader,
				const uint32_t tag_size,
				const struct aes_ccm_msg_stt *pciph_msg,
				struct aes_ccm_deciph_msg_stt *ppayload_msg)
{
	uint32_t ret = CSE_NO_ERR;
	unsigned char *data_buf = NULL;

#if 0
	DataBuf = (unsigned char *)calloc(p_Header->data_size + 6U,
					  sizeof(unsigned char));
#else
	unsigned char data_buf_local[64];

	if ((pheader->data_size + 6U) <= 64) {
		data_buf = data_buf_local;
	} else {
		data_buf = (unsigned char *)calloc(pheader->data_size + 6U,
						  sizeof(unsigned char));
	}
#endif

	if (data_buf) {
		/* Pre formatted Header data, Nonce and associated data */
		struct aes_ccm_header_data_stt header_data;
		uint32_t buf_used = 0U;
		uint32_t i = 0U;

		/* Unchanged for Nonce field */
		header_data.pnonce = pheader->pnonce;
		header_data.nonce_size = pheader->nonce_size;

		/* Initialize pointer to pre-formatted Associated data */
		header_data.pdata = data_buf;

		/*
		 * Formatting the Associated Data: add the encoded size of
		 * Associated Data to the buffer and concatenate the data
		 */
		if (pheader->data_size != 0U) {
			if (pheader->data_size < 0x0000FF00U) {
				/* Length of Associated Data < [2^(16) - 2^8] */
				header_data.pdata[0] = (unsigned char)
					((pheader->data_size >> 8) & 0xFF);
				header_data.pdata[1] = (unsigned char)
					(pheader->data_size & 0xFF);
				/*
				 * Two additional byte for the pre-formatted
				 * associated Data size
				 */
				header_data.data_size += 2U;
				buf_used = 2U;
			} else {
				/* Length of Associated Data > [2^(16) - 2^8] */
				header_data.pdata[0] = 0xFF;
				header_data.pdata[1] = 0xFE;
				header_data.pdata[2] = (unsigned char)
					((pheader->data_size >> 24) & 0xFF);
				header_data.pdata[3] = (unsigned char)
					((pheader->data_size >> 16) & 0xFF);
				header_data.pdata[4] = (unsigned char)
					((pheader->data_size >> 8) & 0xFF);
				header_data.pdata[5] = (unsigned char)
					(pheader->data_size & 0xFF);
				/*
				 * Six additional bytes for the size of the
				 * pre-formatted Associated Data
				 */
				header_data.data_size += 6U;
				buf_used = 6U;
			}
		}

		/*
		 * Copy Associated Data in pre-formatted buffer after
		 * encoded length
		 */
		for (i = 0U; i < pheader->data_size; i++)
			header_data.pdata[buf_used + i] = pheader->pdata[i];

		/* Update size of Associated Data */
		header_data.data_size = pheader->data_size + buf_used;

		/*
		 * Launch CCM decrypt with pre-formatted buffer for
		 * Associated Data
		 */
		ret = cse_hal_send_cmd5p(CSE_AES256_CCM_DECRYPT,
					 (uint32_t)aes256_key_idx,
					 (uint32_t)&header_data,
					 tag_size,
					 (uint32_t)pciph_msg,
					 (uint32_t)ppayload_msg);
#if 0
		/* Free allocated buffer */
		free(DataBuf);
#else
		if ((pheader->data_size + 6U) > 64)
			free(data_buf);
#endif
	} else {
		ret = CSE_GENERAL_ERR;
	}

	return ret;
} /* End of cse_aes256_ccm_decrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 CMAC MAC Tag Generation
 * @details
 * *
 * @param[in]     aes256_key_idx   Identifier of the Secret Key
 * @param[out]    tag_size         Requested CMAC Tag size
 * @param[in]     *pmsg            Pointer to \ref aes_cmac_msg_stt
 *				   structure containing the message on which
 *				   the tag is generated
 * @param[in]     *ptag            Pointer to \ref aes_cmac_tag_stt
 *				   structure receiving the generated CMAC tag
 *
 * @return        Error code
 * @retval 0      When AES256 CMAC tag generation operation successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_cmac_tag_generation(vuint32_t aes256_key_idx,
					const uint32_t tag_size,
					const struct aes_cmac_msg_stt *pmsg,
					struct aes_cmac_tag_stt *ptag)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_AES256_CMAC_GENERATE,
				 (uint32_t)aes256_key_idx, tag_size,
				 (uint32_t)pmsg, (uint32_t)ptag);

	return ret;
} /* End of cse_aes256_cmac_tag_generation */

/*============================================================================*/
/**
 * @brief         Perform AES256 CMAC Tag verification
 * @details
 * *
 * @param[in]     aes256_key_idx   Identifier of the Secret Key
 * @param[out]    tag_size         Size of requested CMAC Tag
 * @param[in]     *pmsg            Pointer to \ref aes_cmac_msg_stt
 *				   structure containing the message for which
 *				   the Tag is verified
 * @param[in]     *ptag            Pointer to \ref aes_cmac_tag_stt
 *				   structure containing the CMAC Tag to be
 *				   verified
 * @param[out]    *psuccess        Pointer to result word location
 *				   (0 if failed, 1 if succeeded)
 *
 * @return        Error code
 * @retval 0      When AES256 CMAC Tag verification operation successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_cmac_tag_verification(vuint32_t aes256_key_idx,
					  const uint32_t tag_size,
					  const struct aes_cmac_msg_stt *pmsg,
					  const struct aes_cmac_tag_stt *ptag,
					  uint32_t *psuccess)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_AES256_CMAC_VERIFY,
				 (uint32_t)aes256_key_idx, tag_size,
				 (uint32_t)pmsg, (uint32_t)ptag);

	/* Return result of CMAC Tag verification operation */
	*psuccess = CSE->P5.R;

	return ret;
} /* End of cse_aes256_cmac_tag_verification */

/*============================================================================*/
/**
 * @brief         Perform AES256 GCM authenticated Encryption
 * @details
 * *
 * @param[in]     aes256_key_idx         Identifier of the Secret Key
 * @param[in]     *pheader               Pointer to \ref aes_gcm_header_data_stt
 *					 structure for IV and Additional
 *					 Authentication Data
 * @param[out]    tag_size               Size of Tag used in authentication
 * @param[in]     *pmsg                  Pointer to \ref
 *					 aes_gcm_msg_stt structure
 *					 containing the message to encrypt
 * @param[in]     *pauth_ciph_msg        Pointer to \ref
 *					 aes_gcm_auth_ciph_msg_stt structure
 *					 receiving the encrypted message
 *
 * @return        Error code
 * @retval 0      When AES256 encryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_aes256_gcm_encrypt(vuint32_t aes256_key_idx,
			       const struct aes_gcm_header_data_stt *pheader,
			       const uint32_t tag_size,
			       const struct aes_gcm_msg_stt *pmsg,
			       struct aes_gcm_auth_ciph_msg_stt *pauth_ciph_msg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_AES256_GCM_ENCRYPT,
				 (uint32_t)aes256_key_idx, (uint32_t)pheader,
				 tag_size, (uint32_t)pmsg,
				 (uint32_t)pauth_ciph_msg);

	return ret;
} /* End of cse_aes256_gcm_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 GCM authenticated Decryption
 * @details
 * *
 * @param[in]     aes256_key_idx         Identifier of the Secret Key
 * @param[in]     *pheader               Pointer to \ref aes_gcm_header_data_stt
 *					 structure for IV and Additional
 *					 Authentication Data
 * @param[out]    tag_size               Size of Tag used in authentication
 * @param[in]     *pauth_ciph_msg        Pointer to \ref
 *					 aes_gcm_auth_ciph_msg_stt structure
 *					 containing the message to decrypt with
 *					 its tag
 * @param[in]     *pdeciph_msg           Pointer to \ref
 *					 ES_GCM_decipheredMessageData_stt
 *					 structure receiving the decrypted
 *					 message
 *
 * @return        Error code
 * @retval 0      When AES256 encryption successes
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */

uint32_t cse_aes256_gcm_decrypt(vuint32_t aes256_key_idx,
			 const struct aes_gcm_header_data_stt *pheader,
			 const uint32_t tag_size,
			 const struct aes_gcm_auth_ciph_msg_stt *pauth_ciph_msg,
			 struct aes_gcm_deciph_msg_data_stt *pdeciph_msg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_AES256_GCM_DECRYPT,
				 (uint32_t)aes256_key_idx, (uint32_t)pheader,
				 tag_size, (uint32_t)pauth_ciph_msg,
				 (uint32_t)pdeciph_msg);

	return ret;
} /* End of cse_aes256_gcm_decrypt */

/**
 * @}
 */
