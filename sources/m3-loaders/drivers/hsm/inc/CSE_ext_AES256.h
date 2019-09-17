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
 * @file    CSE_ext_AES256.h
 * @brief   CSE AES256 test
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#ifndef _CSE_ext_AES256_
#define _CSE_ext_AES256_

#include "cse_types.h"
#include "cse_typedefs.h"
#include "CSE_ext_extendKey.h"
#include "CSE_Constants.h"

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief  Structure type for AES256 secret key
 *
 * Four fields
 *  - 1) Pointer to a byte array containing the secret key
 *  - 2) Size of the secret key
 */
struct AES256secretKeyArray_stt {
	const uint8_t *pmSecretKey;	/*!< AES256 Secret Key */
	uint32_t mSecretKeySize;	/*!< Size of AES256 Secret Key */
};

/**
 * @brief  Structure type for AES256 secret key
 */
struct AES256_key_stt {
	/* @brief secret key structure */
	struct AES256secretKeyArray_stt secretKey;

	/** @brief Associated counter value (28 bits only) */
	union {
		uint32_t R;
		struct {
			uint32_t unused:4;
			uint32_t CNT28:28;
		} B;
	} CNT;

	/** @brief description and usage restriction flags */
	union EXTENDED_KEY_flags AES256_flags;

	/** @brief tell if key location is empty or not */
	uint32_t    empty;
};

/**
 * @brief   Structure type for AES CCM Header Data
 * @details Contains nonce N and associated data A used to provide message
 *          authenticity
 * Four fields
 *  - 1) Pointer to byte array containing the nonce N
 *  - 2) Size in bytes of nonce N
 *  - 3) Pointer to a byte array containing the Associated Data A
 *  - 4) Size in bytes of Associated Data A
 */

struct AES_CCM_headerData_stt {
	/*!< Byte array for Nonce N */
	uint8_t *pNonce;
	/*!< Nonce length in byte */
	uint32_t nonceByteSize;
	/*!< Byte array for Associated Data */
	uint8_t *pData;
	/*!< Length of associated Data in byte */
	uint32_t DataByteSize;
};

/**
 * @brief   Structure type for AES CCM Tag
 * @details Contains Tag value and tag size for the Decryption-Verification
 *          operation
 * Two fields
 *  - 1) Pointer to byte array containing the Tag (MAC) T
 *  - 2) Size in bytes of Tag T
 */

struct AES_CCM_tagData_stt {
	/*!< Byte array for Tag in Decryption-Verification operation */
	uint8_t *pTag;
	/*!< Length of Tag in byte */
	uint32_t tagByteSize;
};

/**
 * @brief   Structure type for AES CCM message
 * @details Contains the message to be ciphered in the
 *	    Generation-Encryption operation and its size
 * Two fields
 *  - 1) Pointer to byte array containing the Message
 *  - 2) Size in bytes of the message
 */

struct AES_CCM_msgData_stt {
	/*!< Byte array for the message to encrypt */
	uint8_t *pAddress;
	/*!< Length in byte of the message to encrypt */
	uint32_t messageByteSize;
};

/**
 * @brief   Structure type for AES CCM Deciphered message
 * @details Contains the deciphered message deciphered by the
 *	    Decryption-Verification operation and its size.
 *          Contains also the result of Authentication Verification
 * Three fields
 *  - 1) Pointer to byte array containing the Deciphered Message
 *  - 2) Size in bytes of the deciphered message
 *  - 3) Authentication result
 */

struct AES_CCM_deciphMsgData_stt {
	/*!< Byte array for the deciphered message, i.e. the payload */
	uint8_t *pDecipheredMessage;
	/*!< Length in byte of the deciphered message */
	uint32_t decipheredmessageByteSize;
	/*
	 *!< Authentication result. Either:
	 * AUTHENTICATION_SUCCESSFUL if the TAG is verified
	 * AUTHENTICATION_FAILED if the TAG is not verified
	 */
	uint32_t authResult;
};

/**
 * @brief   Structure type for AES CMAC message
 * @details Contains the message on which the CMAC MAC is generated
 * Two fields
 *  - 1) Pointer to byte array containing the Message
 *  - 2) Size in bytes of the message
 */

struct AES_CMAC_msg_stt {
	uint8_t *pAddress;	   /*!< Byte array for the input message */
	uint32_t messageByteSize;  /*!< Length in byte of the input message */
};

/**
 * @brief   Structure type for AES CMAC MAC
 * @details Contains the generated CMAC MAC
 * Two fields
 *  - 1) Pointer to byte array containing the Message
 *  - 2) Size in bytes of the message
 */

struct AES_CMAC_tag_stt {
	uint8_t *pAddress; /*!< Byte array for the CMAC tag */
};

/**
 * @brief    Structure type for AES GCM Header Data
 * @details  Contains nonce N (the IV) and Additional Authentication
 *	     Data AAD used to provide message authenticity
 * Four fields
 *  - 1) Pointer to byte array containing the IV N
 *  - 2) Size in bytes of IV N
 *  - 3) Pointer to a byte array containing the Additional Authentication Data
 *	 AAD
 *  - 4) Size in bytes of Additional Authenticated Data AAD
 */

struct AES_GCM_headerData_stt {
	/*!< Byte array for IV */
	uint8_t *pIv;
	/*!< IV length in byte */
	uint32_t ivByteSize;
	/*!< Byte array for Additional Authenticated Data */
	uint8_t *pAdditionalAuthenticationData;
	/*!< Length of Additional Authentication Data in byte */
	uint32_t AdditionalAuthenticationDataByteSize;
};

/**
 * @brief   Structure type for AES GCM message
 * @details Contains the message to be ciphered in the Generation-Encryption
 *	     operation and its size
 * Two fields
 *  - 1) Pointer to byte array containing the Message
 *  - 2) Size in bytes of the message
 */

struct AES_GCM_msgData_stt {
	/*!< Byte array for the message to encrypt */
	uint8_t *pAddress;
	/*!< Length in byte of the message to encrypt */
	uint32_t messageByteSize;
};

/**
 * @brief   Structure type for AES GCM Authenticated Ciphered Message
 * @details Contains the result of the GCM process, the Authenticated
 *	    Ciphered Message
 * Four fields
 *  - 1) Pointer to byte array containing the Ciphered Message
 *  - 2) Size in bytes of the ciphered message
 *  - 3) Pointer to a byte array containing the Authentication Tag T
 *  - 4) Byte length of the authenticated Tag
 */

struct AES_GCM_authCiphMsg_stt {
	/*!< Byte array for the message to encrypt */
	uint8_t *pAddress;
	/*!< Length in byte of the message to encrypt */
	uint32_t messageByteSize;
	/*!< Pointer to Byte Array for the Authentication Tag */
	uint8_t *pAuthTag;
	/*!< Length in byte of the Authentication Tag */
	uint32_t authTagByteSize;
};

/**
 * @brief   Structure type for AES GCM Deciphered message
 * @details Contains the deciphered message deciphered by
 *	    the Decryption-Verification operation and its size.
 *          Contains also the result of Authentication Verification
 * Three fields
 *  - 1) Pointer to byte array containing the Deciphered Message
 *  - 2) Size in bytes of the deciphered message
 *  - 3) Authentication result
 */

struct AES_GCM_deciphMsgData_stt {
	/*!< Byte array for the deciphered message to decrypt */
	uint8_t *pDecipheredMessage;
	/*!< Length in byte of the deciphered message */
	uint32_t decipheredmessageByteSize;
	/*!< Authentication result. Either:
	 * AUTHENTICATION_SUCCESSFUL if the TAG is verified
	 * AUTHENTICATION_FAILED if the TAG is not verified
	 */
	uint32_t authResult;
};

/*============================================================================*/
/**
 * @brief         Loads an AES256 Key cleartext value in the CSE AES256 RamKey
 *		  internal storage
 * @details       Loads a key value to be used when selecting index
 *		  AES256_RAM_KEY for AES256 dedicated commands
 *
 * @param[in]     P_pSecretKey    pointer to a \ref AES256secretKeyArray_stt
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
uint32_t CSE_AES256_loadRamKey(struct AES256secretKeyArray_stt *P_pSecretKey);

/*============================================================================*/
/**
 * @brief		Export AES256 Ram Key
 * @details		Compute and provide all fields required to load the key
 *			value that was in the RAMKEY location in a non volatile
 *			key location
 *
 * @param[out] P_M1     Pointer to buffer to store the M1 message
 *			(UID, KeyID, Auth ID - 128 bits total)
 * @param[out] P_M2     Pointer to buffer to store the M2 message
 *			(security flags,  - 256 bits )
 * @param[out] P_M3     Pointer to buffer to store the M3 message
 *			(MAC of M1 and M2 - 128 bits)
 * @param[out] P_M4     Pointer to buffer to store the M4 message
 *			(IDs | counter - 256 bits)
 * @param[out] P_M5     Pointer to buffer to store the M5 message
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
uint32_t CSE_AES256_exportRamKey(uint8_t *P_M1, uint8_t *P_M2, uint8_t *P_M3,
				 uint8_t *P_M4, uint8_t *P_M5);

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
uint32_t CSE_AES256_loadKey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
			    uint8_t *M4, uint8_t *M5);

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in ECB mode
 * @details
 *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *P_pSrc             Pointer to Message to encrypt
 * @param[out]    *P_pDest            Pointer to Encrypted message
 * @param[in]     blockCount          Length in byte of the input message to
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
uint32_t CSE_AES256_ECB_encrypt(vuint32_t P_AES256_key_idx,
				vuint8_t *P_pSrc, vuint8_t *P_pDest,
				vuint32_t blockCount);

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in ECB mode
 * @details
 *
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
				vuint32_t blockCount);

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in CBC mode
 * @details
 *
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
				vuint32_t blockCount);

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
				vuint32_t blockCount);

/*============================================================================*/
/**
 * @brief         Perform AES256 Encryption in CCM mode
 * @details       Perform CCM process with pre-formatted Additional Data.
 *		  The Additional Data are formated in this functions to
 *                avoid non-aligned access due to the concatenation of 2 or 6
 *		  bytes encoding the AD size.
 *
 * @param[in]     P_AES256_key_idx Identifier of the Secret Key
 * @param[in]     *p_Header        Pointer to \ref AES_CCM_headerData_stt
 *				   structure for Nonce and Associated Data
 * @param[out]    P_tagSize        Size of Tag used in authentication
 * @param[in]     *P_pMsg          Pointer to \ref AES_CCM_msgData_stt
 *				   structure containing the message to encrypt
 * @param[in]     *P_CipheredMsg  Pointer to \ref AES_CCM_msgData_stt
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
uint32_t CSE_AES256_CCM_encrypt(vuint32_t P_AES256_key_idx,
				const struct AES_CCM_headerData_stt *p_Header,
				const uint32_t P_tagSize,
				const struct AES_CCM_msgData_stt *P_pMsg,
				struct AES_CCM_msgData_stt *P_CipheredMsg);

/*============================================================================*/
/**
 * @brief         Perform AES256 Decryption in CCM mode
 * @details       Perform CCM process with pre-formatted Additional Data.
 *		  The Additional Data are formated in this functions to
 *                avoid non-aligned access due to the concatenation of 2 or 6
 *		  bytes encoding the AD size.
 *
 * @param[in]     P_AES256_key_idx    Identifier of the Secret Key
 * @param[in]     *p_Header           Pointer to \ref AES_CCM_headerData_stt
 *				      structure for Nonce and Associated Data
 * @param[out]    P_tagSize           Size of Tag used in authentication
 * @param[in]     *P_CipheredMsg      Pointer to \ref AES_CCM_msgData_stt
 *				      structure containing the message to
 *				      decrypt
 * @param[in]     *P_pPayloadMsg      Pointer to \ref
 *				      AES_CCM_deciphMsgData_stt
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
uint32_t CSE_AES256_CCM_decrypt(vuint32_t P_AES256_key_idx,
				const struct AES_CCM_headerData_stt *p_Header,
				const uint32_t P_tagSize,
				const struct AES_CCM_msgData_stt *P_CipheredMsg,
				struct AES_CCM_deciphMsgData_stt *P_PayloadMsg);

/*============================================================================*/
/**
 * @brief         Perform AES256 CMAC MAC Tag Generation
 * @details
 * *
 * @param[in]     P_AES256_key_idx Identifier of the Secret Key
 * @param[out]    P_tagSize        Requested CMAC Tag size
 * @param[in]     *P_Msg           Pointer to \ref AES_CMAC_msg_stt
 *				   structure containing the message on which
 *				   the tag is generated
 * @param[in]     *P_pTag          Pointer to \ref AES_CMAC_tag_stt
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
uint32_t CSE_AES256_CMAC_tagGeneration(vuint32_t P_AES256_key_idx,
				       const uint32_t P_tagSize,
				       const struct AES_CMAC_msg_stt *P_Msg,
				       struct AES_CMAC_tag_stt *P_pTag);

/*============================================================================*/
/**
 * @brief         Perform AES256 CMAC Tag verification
 * @details
 * *
 * @param[in]     P_AES256_key_idx Identifier of the Secret Key
 * @param[out]    P_tagSize        Size of requested CMAC Tag
 * @param[in]     *P_Msg          Pointer to \ref AES_CMAC_msg_stt
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
					 uint32_t *P_pSuccess);

/*============================================================================*/
/**
 * @brief         Perform AES256 GCM authenticated Encryption
 * @details
 * *
 * @param[in]     P_AES256_key_idx        Identifier of the Secret Key
 * @param[in]     *p_Header               Pointer to \ref AES_GCM_headerData_stt
 *					  structure for IV and Additional
 *					  Authentication Data
 * @param[out]    P_tagSize               Size of Tag used in authentication
 * @param[in]     *P_Msg                  Pointer to \ref
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
				struct AES_GCM_authCiphMsg_stt *P_pAuthCiphMsg);

/*============================================================================*/
/**
 * @brief         Perform AES256 GCM authenticated Decryption
 * @details
 * *
 * @param[in]     P_AES256_key_idx        Identifier of the Secret Key
 * @param[in]     *p_Header               Pointer to \ref AES_GCM_headerData_stt
 *					  structure for IV and Additional
 *					  Authentication Data
 * @param[out]    P_tagSize               Size of Tag used in authentication
 * @param[in]     *P_pAuthCiphMsg     Pointer to \ref
 *					  AES_GCM_authCiphMsg_stt structure
 *					  containing the message to decrypt with
 *					  its tag
 * @param[in]     *P_pDeCipheredMsg       Pointer to \ref
 *					  ES_GCM_decipheredMessageData_stt
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
			   struct AES_GCM_deciphMsgData_stt *P_pDeCipheredMsg);
#endif /* _CSE_ext_AES256_ */
/**
 * @}
 */
