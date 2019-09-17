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
 *                internal storage
 * @details       Loads a key value to be used when selecting index
 *                AES256_RAM_KEY for AES256 dedicated commands
 *
 * @param[in]     P_pSecretKey    pointer to a \ref AES256secretKeyByteArray_stt
 *                                structure for the secret key
 *
 * @return        Error code
 * @retval 0      When key was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *                see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_AES256_loadRamKey(struct AES256secretKeyArray_stt *P_pSecretKey)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/* P1 is the address of location containing the AES256 secret Key */

	ret = CSE_HAL_send_cmd1p(CSE_AES256_LOAD_RAM_KEY,
				 (uint32_t)P_pSecretKey);

	return ret;
} /* End of CSE_AES256_loadRamKey */

/*============================================================================*/
/**
 * @brief	  Export AES256 Ram Key
 * @details	  Compute and provide all fields required to load the key value
 *		  that was in the RAMKEY location in a non volatile key location
 *
 * @param[out]	  P_M1     Pointer to buffer to store the M1 message
 *			   (UID, KeyID, Auth ID - 128 bits total)
 * @param[out]	  P_M2     Pointer to buffer to store the M2 message
 *			   (security flags,  - 256 bits )
 * @param[out]	  P_M3     Pointer to buffer to store the M3 message
 *			   (MAC of M1 and M2 - 128 bits)
 * @param[out]	  P_M4     Pointer to buffer to store the M4 message
 *			   (IDs | counter - 256 bits)
 * @param[out]	  P_M5     Pointer to buffer to store the M5 message
 *			   (MAC of M4 - 128 bits )
 *
 * @return        Error code
 * @retval 0      When key was exported properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_AES256_exportRamKey(uint8_t *P_M1, uint8_t *P_M2, uint8_t *P_M3,
				 uint8_t *P_M4, uint8_t *P_M5)
{
	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_AES256_EXPORT_RAM_KEY,
				 (uint32_t)P_M1, (uint32_t)P_M2,
				 (uint32_t)P_M3, (uint32_t)P_M4,
				 (uint32_t)P_M5);

	return ret;
} /* End of CSE_AES256_exportRamKey */

/*============================================================================*/
/**
 * @brief         Loads an AES256 Key in internal non volatile memory
 * @details       Key will be loaded at index described in the already prepared
 *		  messages including key usage restrictions (security flags),
 *		  keyindex, encrypted value and integrity & authenticity MACs
 *		  for all the params.
 *
 * @param[in]     P_M1    pointer to message1
 * @param[in]     P_M2    pointer to message2
 * @param[in]     P_M3    pointer to message3
 *
 * @param[out]    P_M4    pointer to message4
 * @param[out]    P_M5    pointer to message5
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
uint32_t CSE_AES256_loadKey(uint8_t *P_M1, uint8_t *P_M2, uint8_t *P_M3,
			    uint8_t *P_M4, uint8_t *P_M5)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_AES256_LOAD_KEY, (uint32_t)P_M1,
				 (uint32_t)P_M2, (uint32_t)P_M3,
				 (uint32_t)P_M4, (uint32_t)P_M5);

	return ret;
} /* End of CSE_AES256_loadKey */

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in ECB mode
 * @details
 * *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *P_pSrc             Pointer to Message to encrypt
 * @param[out]    *P_pDest            Pointer to destination buffer Encrypted
 *				      message
 * @param[in]     blockCount          Length in byte of the input message to
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
uint32_t CSE_AES256_ECB_encrypt(vuint32_t P_AES256_key_idx,
				vuint8_t *P_pSrc, vuint8_t *P_pDest,
				vuint32_t blockCount)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_AES256_ECB_ENCRYPT,
				 (uint32_t)P_AES256_key_idx,
				 blockCount,
				 (uint32_t)P_pSrc,
				 (uint32_t)P_pDest);

	return ret;
} /* End of CSE_AES256_ECB_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in ECB mode
 * @details
 * *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *P_pSrc             Pointer to Message to decrypt
 * @param[out]    *P_pDest            Pointer to destination buffer cleartext
 *				      message
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
uint32_t CSE_AES256_ECB_decrypt(vuint32_t P_AES256_key_idx,
				vuint8_t *P_pSrc, vuint8_t *P_pDest,
				vuint32_t blockCount)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_AES256_ECB_DECRYPT,
				 (uint32_t)P_AES256_key_idx,
				 blockCount,
				 (uint32_t)P_pSrc,
				 (uint32_t)P_pDest);

	return ret;
} /* End of CSE_AES256_ECB_decrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in CBC mode
 * @details
 * *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *P_pSrc             Pointer to Message to encrypt
 * @param[out]    *P_pDest            Pointer to destination buffer Encrypted
 *				      message
 * @param[in]     *P_pIv              Pointer to Initialization Vector
 * @param[in]     blockCount          Length in byte of the input message to
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
uint32_t CSE_AES256_CBC_encrypt(vuint32_t P_AES256_key_idx,
				vuint8_t *P_pSrc, vuint8_t *P_pDest,
				vuint8_t *P_pIv,
				vuint32_t blockCount)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_AES256_CBC_ENCRYPT,
				 (uint32_t)P_AES256_key_idx,
				 (uint32_t)P_pIv,
				 blockCount,
				 (uint32_t)P_pSrc,
				 (uint32_t)P_pDest);

	return ret;
} /* End of CSE_AES256_CBC_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in CBC mode
 * @details
 * *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *P_pSrc             Pointer to Message to decrypt
 * @param[out]    *P_pDest            Pointer to destination buffer cleartext
 *				      message
 * @param[in]     *P_pIv              Pointer to Initialization Vector
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
uint32_t CSE_AES256_CBC_decrypt(vuint32_t P_AES256_key_idx,
				vuint8_t *P_pSrc, vuint8_t *P_pDest,
				vuint8_t *P_pIv,
				vuint32_t blockCount)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_AES256_CBC_DECRYPT,
				 (uint32_t)P_AES256_key_idx,
				 (uint32_t)P_pIv,
				 blockCount,
				 (uint32_t)P_pSrc,
				 (uint32_t)P_pDest);

	return ret;
} /* End of CSE_AES256_CBC_decrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in CCM mode
 * @details       Perform CCM process with pre-formatted Additional Data.
 *		  The Additional Data are formated in this functions to
 *                avoid non-aligned access due to the concatenation of 2 or 6
 *		  bytes encoding the AD size.
 *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *p_Header           Pointer to \ref AES_CCM_headerData_stt
 *				      structure for Nonce and Associated Data
 * @param[out]    P_tagSize           Size of Tag used in authentication
 * @param[in]     *P_pMsg             Pointer to \ref AES_CCM_msgData_stt
 *				      structure containing the message to
 *				      encrypt
 * @param[in]     *P_CipheredMsg     Pointer to \ref AES_CCM_msgData_stt
 *				      structure receiving the encrypted message
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
uint32_t CSE_AES256_CCM_encrypt(vuint32_t P_AES256_key_idx,
				const struct AES_CCM_headerData_stt *p_Header,
				const uint32_t P_tagSize,
				const struct AES_CCM_msgData_stt *P_pMsg,
				struct AES_CCM_msgData_stt *P_CipheredMsg)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * Buffer for pre-formatted Associated Data, formatted size |
	 * Associated Data
	 */
	unsigned char *DataBuf = NULL;

#if 0
	DataBuf = (unsigned char *)calloc(p_Header->DataByteSize + 6U,
					  sizeof(unsigned char));
#else
	unsigned char DataBuf_local[64];

	if ((p_Header->DataByteSize + 6U) <= 64)
		DataBuf = DataBuf_local;
	else
		DataBuf = (unsigned char *)calloc(p_Header->DataByteSize + 6U,
						  sizeof(unsigned char));
#endif

	if (DataBuf) {
		/* Pre formatted Header data, Nonce and associated data */
		struct AES_CCM_headerData_stt HeaderData;
		uint32_t bufUsed = 0U;
		uint32_t i = 0U;

		/* Unchanged for Nonce field */
		HeaderData.pNonce = p_Header->pNonce;
		HeaderData.nonceByteSize = p_Header->nonceByteSize;

		/* Initialize pointer to pre-formatted Associated data */
		HeaderData.pData = DataBuf;

		/*
		 * Formatting the Associated Data: add the encoded size of
		 * Associated Data to the buffer and concatenate the data
		 */
		if (p_Header->DataByteSize != 0U) {
			if (p_Header->DataByteSize < 0x0000FF00U) {
			/* Length of Associated Data < [2^(16) - 2^8] */
			HeaderData.pData[0] = (unsigned char)
				((p_Header->DataByteSize >> 8) & 0xFF);
			HeaderData.pData[1] = (unsigned char)
				(p_Header->DataByteSize & 0xFF);
			/*
			 * Two additional byte for the pre-formatted
			 * associated Data size
			 */
			HeaderData.DataByteSize += 2U;
			bufUsed = 2U;
			} else {
				/* Length of Associated Data > [2^(16) - 2^8] */
				HeaderData.pData[0] = 0xFF;
				HeaderData.pData[1] = 0xFE;
				HeaderData.pData[2] = (unsigned char)
					((p_Header->DataByteSize >> 24) & 0xFF);
				HeaderData.pData[3] = (unsigned char)
					((p_Header->DataByteSize >> 16) & 0xFF);
				HeaderData.pData[4] = (unsigned char)
					((p_Header->DataByteSize >> 8) & 0xFF);
				HeaderData.pData[5] = (unsigned char)
					(p_Header->DataByteSize & 0xFF);
				/*
				 * Six additional bytes for the size of the
				 * pre-formatted Associated Data
				 */
				HeaderData.DataByteSize += 6U;
				bufUsed = 6U;
			}
		}

		/*
		 * Copy Associated Data in pre-formatted buffer
		 * after encoded length
		 */
		for (i = 0U; i < p_Header->DataByteSize; i++)
			HeaderData.pData[bufUsed + i] = p_Header->pData[i];

		/* Update size of Associated Data */
		HeaderData.DataByteSize = p_Header->DataByteSize + bufUsed;

		/*
		 * Launch CCM encrypt with pre-formatted buffer for Associated
		 * Data
		 */
		ret = CSE_HAL_send_cmd5p(CSE_AES256_CCM_ENCRYPT,
					 (uint32_t)P_AES256_key_idx,
					 (uint32_t)&HeaderData,
					 P_tagSize,
					 (uint32_t)P_pMsg,
					 (uint32_t)P_CipheredMsg);

#if 0
		/* Free allocated buffer */
		free(DataBuf);
#else
		if ((p_Header->DataByteSize + 6U) > 64)
			free(DataBuf);
#endif
	} else {
		ret = CSE_GENERAL_ERR;
	}

	return ret;
} /* End of CSE_AES256_CCM_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in CCM mode
 * @details       Perform CCM process with pre-formatted Additional Data.
 *		  The Additional Data are formated in this functions to
 *                avoid non-aligned access due to the concatenation of 2 or 6
 *		  bytes encoding the AD size.
 *
 * @param[in]     P_AES256_key_idx   Identifier of the Secret Key
 * @param[in]     *p_Header          Pointer to \ref AES_CCM_headerData_stt
 *				     structure for Nonce and Associated Data
 * @param[out]    P_tagSize          Size of Tag used in authentication
 * @param[in]     *P_CipheredMsg    Pointer to \ref AES_CCM_msgData_stt
 *				     structure containing the message to decrypt
 * @param[in]     *P_PayloadMsg     Pointer to \ref
 *				     AES_CCM_deciphMsgData_stt
 *				     structure receiving the decrypted message,
 *				     i.e. the payload
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
uint32_t CSE_AES256_CCM_decrypt(vuint32_t P_AES256_key_idx,
				const struct AES_CCM_headerData_stt *p_Header,
				const uint32_t P_tagSize,
				const struct AES_CCM_msgData_stt *P_CipheredMsg,
				struct AES_CCM_deciphMsgData_stt *P_PayloadMsg)
{
	uint32_t ret = CSE_NO_ERR;
	unsigned char *DataBuf = NULL;

#if 0
	DataBuf = (unsigned char *)calloc(p_Header->DataByteSize + 6U,
					  sizeof(unsigned char));
#else
	unsigned char DataBuf_local[64];

	if ((p_Header->DataByteSize + 6U) <= 64) {
		DataBuf = DataBuf_local;
	} else {
		DataBuf = (unsigned char *)calloc(p_Header->DataByteSize + 6U,
						  sizeof(unsigned char));
	}
#endif

	if (DataBuf) {
		/* Pre formatted Header data, Nonce and associated data */
		struct AES_CCM_headerData_stt HeaderData;
		uint32_t bufUsed = 0U;
		uint32_t i = 0U;

		/* Unchanged for Nonce field */
		HeaderData.pNonce = p_Header->pNonce;
		HeaderData.nonceByteSize = p_Header->nonceByteSize;

		/* Initialize pointer to pre-formatted Associated data */
		HeaderData.pData = DataBuf;

		/*
		 * Formatting the Associated Data: add the encoded size of
		 * Associated Data to the buffer and concatenate the data
		 */
		if (p_Header->DataByteSize != 0U) {
			if (p_Header->DataByteSize < 0x0000FF00U) {
				/* Length of Associated Data < [2^(16) - 2^8] */
				HeaderData.pData[0] = (unsigned char)
					((p_Header->DataByteSize >> 8) & 0xFF);
				HeaderData.pData[1] = (unsigned char)
					(p_Header->DataByteSize & 0xFF);
				/*
				 * Two additional byte for the pre-formatted
				 * associated Data size
				 */
				HeaderData.DataByteSize += 2U;
				bufUsed = 2U;
			} else {
				/* Length of Associated Data > [2^(16) - 2^8] */
				HeaderData.pData[0] = 0xFF;
				HeaderData.pData[1] = 0xFE;
				HeaderData.pData[2] = (unsigned char)
					((p_Header->DataByteSize >> 24) & 0xFF);
				HeaderData.pData[3] = (unsigned char)
					((p_Header->DataByteSize >> 16) & 0xFF);
				HeaderData.pData[4] = (unsigned char)
					((p_Header->DataByteSize >> 8) & 0xFF);
				HeaderData.pData[5] = (unsigned char)
					(p_Header->DataByteSize & 0xFF);
				/*
				 * Six additional bytes for the size of the
				 * pre-formatted Associated Data
				 */
				HeaderData.DataByteSize += 6U;
				bufUsed = 6U;
			}
		}

		/*
		 * Copy Associated Data in pre-formatted buffer after
		 * encoded length
		 */
		for (i = 0U; i < p_Header->DataByteSize; i++)
			HeaderData.pData[bufUsed + i] = p_Header->pData[i];

		/* Update size of Associated Data */
		HeaderData.DataByteSize = p_Header->DataByteSize + bufUsed;

		/*
		 * Launch CCM decrypt with pre-formatted buffer for
		 * Associated Data
		 */
		ret = CSE_HAL_send_cmd5p(CSE_AES256_CCM_DECRYPT,
					 (uint32_t)P_AES256_key_idx,
					 (uint32_t)&HeaderData,
					 P_tagSize,
					 (uint32_t)P_CipheredMsg,
					 (uint32_t)P_PayloadMsg);
#if 0
		/* Free allocated buffer */
		free(DataBuf);
#else
		if ((p_Header->DataByteSize + 6U) > 64)
			free(DataBuf);
#endif
	} else {
		ret = CSE_GENERAL_ERR;
	}

	return ret;
} /* End of CSE_AES256_CCM_decrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 CMAC MAC Tag Generation
 * @details
 * *
 * @param[in]     P_AES256_key_idx      Identifier of the Secret Key
 * @param[out]    P_tagSize             Requested CMAC Tag size
 * @param[in]     *P_pMsg               Pointer to \ref AES_CMAC_msg_stt
 *					structure containing the message on
 *					which the tag is generated
 * @param[in]     *P_pTag               Pointer to \ref AES_CMAC_tag_stt
 *					structure receiving the generated CMAC
 *					tag
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
uint32_t CSE_AES256_CMAC_tagGeneration(vuint32_t P_AES256_key_idx,
				       const uint32_t P_tagSize,
				       const struct AES_CMAC_msg_stt *P_pMsg,
				       struct AES_CMAC_tag_stt *P_pTag)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_AES256_CMAC_GENERATE,
				 (uint32_t)P_AES256_key_idx,
				 P_tagSize,
				 (uint32_t)P_pMsg,
				 (uint32_t)P_pTag);

	return ret;
} /* End of CSE_AES256_CMAC_tagGeneration */

/*============================================================================*/
/**
 * @brief         Perform AES256 CMAC Tag verification
 * @details
 * *
 * @param[in]     P_AES256_key_idx Identifier of the Secret Key
 * @param[out]    P_tagSize        Size of requested CMAC Tag
 * @param[in]     *P_Msg           Pointer to \ref AES_CMAC_msg_stt
 *				   structure containing the message for which
 *				   the Tag is verified
 * @param[in]     *P_pTag          Pointer to \ref AES_CMAC_tag_stt
 *				   structure containing the CMAC Tag to be
 *				   verified
 * @param[out]    P_pSuccess       Pointer to result word location
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
uint32_t CSE_AES256_CMAC_tagVerification(vuint32_t P_AES256_key_idx,
					 const uint32_t P_tagSize,
					 const struct AES_CMAC_msg_stt *P_Msg,
					 const struct AES_CMAC_tag_stt *P_pTag,
					 uint32_t *P_pSuccess)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_AES256_CMAC_VERIFY,
				 (uint32_t)P_AES256_key_idx,
				 P_tagSize,
				 (uint32_t)P_Msg,
				 (uint32_t)P_pTag);

	/* Return result of CMAC Tag verification operation */
	*P_pSuccess = CSE->P5.R;

	return ret;
} /* End of CSE_AES256_CMAC_tagVerification */

/*============================================================================*/
/**
 * @brief         Perform AES256 GCM authenticated Encryption
 * @details
 * *
 * @param[in]     P_AES256_key_idx        Identifier of the Secret Key
 * @param[in]     *p_Header		  Pointer to \ref AES_GCM_headerData_stt
 *					  structure for IV and Additional
 *					  Authentication Data
 * @param[out]    P_tagSize               Size of Tag used in authentication
 * @param[in]     *P_pMsg                 Pointer to \ref
 *					  AES_GCM_msgData_stt structure
 *					  containing the message to encrypt
 * @param[in]     *P_pAuthCiphMsg         Pointer to \ref
 *					  AES_GCM_authCiphMsg_stt structure
 *					  receiving the encrypted message
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
uint32_t CSE_AES256_GCM_encrypt(vuint32_t P_AES256_key_idx,
				const struct AES_GCM_headerData_stt *p_Header,
				const uint32_t P_tagSize,
				const struct AES_GCM_msgData_stt *P_Msg,
				struct AES_GCM_authCiphMsg_stt *P_pAuthCiphMsg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_AES256_GCM_ENCRYPT,
				 (uint32_t)P_AES256_key_idx,
				 (uint32_t)p_Header,
				 P_tagSize,
				 (uint32_t)P_Msg,
				 (uint32_t)P_pAuthCiphMsg);

	return ret;
} /* End of CSE_AES256_GCM_encrypt */

/*============================================================================*/
/**
 * @brief         Perform AES256 GCM authenticated Decryption
 * @details
 * *
 * @param[in]     P_AES256_key_idx        Identifier of the Secret Key
 * @param[in]     *p_Header		  Pointer to \ref AES_GCM_headerData_stt
 *					  structure for IV and Additional
 *					  Authentication Data
 * @param[out]    P_tagSize               Size of Tag used in authentication
 * @param[in]     *P_pAuthCiphMsg         Pointer to \ref
 *					  AES_GCM_authCiphMsg_stt structure
 *					  containing the message to decrypt
 *					  with its tag
 * @param[in]     *P_pDeCipheredMsg       Pointer to \ref
 *					  AES_GCM_deciphMsgData_stt
 *					  structure receiving the decrypted
 *					  message
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

uint32_t CSE_AES256_GCM_decrypt(vuint32_t P_AES256_key_idx,
			   const struct AES_GCM_headerData_stt *p_Header,
			   const uint32_t P_tagSize,
			   const struct AES_GCM_authCiphMsg_stt *P_pAuthCiphMsg,
			   struct AES_GCM_deciphMsgData_stt *P_pDeCipheredMsg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_AES256_GCM_DECRYPT,
				 (uint32_t)P_AES256_key_idx,
				 (uint32_t)p_Header,
				 P_tagSize,
				 (uint32_t)P_pAuthCiphMsg,
				 (uint32_t)P_pDeCipheredMsg);

	return ret;
} /* End of CSE_AES256_GCM_decrypt */

/**
 * @}
 */
