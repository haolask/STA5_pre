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
 * @file    CSE_ext_ECC.c
 * @brief   CSE ECC commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#ifndef _CSE_ext_ECC_INCLUDED_
#define _CSE_ext_ECC_INCLUDED_

#include "cse_types.h"
#include "CSE_ext_extendKey.h"
#include "CSE_Constants.h"

/**
 * @brief  Structure type for ECC public key
 *
 * Four fields
 *  - 1) Pointer to a byte array containing the x coordinate of the public key
 *  - 2) Size of the x coordinate of the public key
 *  - 3) Pointer to a byte array containing the y coordinate of the public key
 *  - 4) Size of the y coordinate of the public key
 */
struct ECCpubKeyByteArray_stt {
	/*!< ECC Public Key X coordinate */
	const uint8_t *pmPubKeyCoordX;
	/*!< Size of ECC Public Key X coordinate */
	int32_t mPubKeyCoordXSize;
	/*!< ECC Public Key Y coordinate */
	const uint8_t *pmPubKeyCoordY;
	/*!< Size of ECC Public Key Y coordinate */
	int32_t mPubKeyCoordYSize;
};

/**
 * @brief  Structure type for ECC private key
 * Two fields
 *  - 1) Pointer to a byte array containing the private key
 *  - 2) Size of the private key
 */
struct ECCprivKeyByteArray_stt {
	const uint8_t *pmPrivKey;	/*!< ECC Private */
	int32_t mPrivKeySize;		/*!< Size of ECC Private Key */
};

struct ECDSA_Signature_stt {
	const vuint8_t *pSigR;
	vuint32_t sigR_size;
	const vuint8_t *pSigS;
	vuint32_t sigS_size;
};

/**
 * @brief  Structure type for ECIES Message key
 */
struct ECC_EciesMessage_struct {
	uint8_t  *address;	/*!< pointer to buffer */
	uint32_t  byteSize;	/*!< size in bytes */
};

/**
 * @brief  Structure type for ECIES Shared Data 1
 * @details Optional parameters to be used for Key Derivation Function
 * Two fields
 *  - 1) Pointer to a byte array containing Shared data 1
 *  - 2) Size of Shared Data 1
 */

struct ECIES_sharedData1_stt {
	uint8_t *pSharedData1;
	uint32_t sharedData1Size;
};

/**
 * @brief   Structure type for ECIES Shared Data 2
 * @details Optional parameters to be used for Message Authentication
 *	     Code function
 * Two fields
 *  - 3) Pointer to a byte array containing Shared Data 2
 *  - 4) Size of Shared Data 2
 */

struct ECIES_sharedData2_stt {
	uint8_t *pSharedData2;
	uint32_t sharedData2Size;
};

/**
 * @brief  Structure type for ECIES Shared Data
 * Two fields
 *  - 1) Structure \ref ECIES_sharedData1_stt defining Shared Data 1
 *  - 2) Structure \ref ECIES_sharedData1_stt defining Shared Data 2
 */

struct ECIES_sharedData_stt {
	struct ECIES_sharedData1_stt *pSharedData1_stt;
	struct ECIES_sharedData2_stt *pSharedData2_stt;
};

/**
 * @brief  Structure type for ECIES Encrypted Message
 * six fields
 *  - 1) Pointer to a byte array containing Encryption Ephemeral Public Key
 *  - 2) Size of the Encryption Ephemeral Key
 *  - 3) Pointer to a byte array containing Encrypted Message
 *  - 4) Size of the Encrypted Message
 *  - 5) Pointer to a byte array containing the tag computed over
 *	 the Encrypted Message
 *  - 6) Size of the Tag
 */

struct ECIES_encMsg_stt {
	struct ECCpubKeyByteArray_stt *pSenderFmrPubKey;
	uint8_t *pEciesEncryptedMessage;
	uint32_t eciesEncryptedMessageSize;
	uint8_t *pTag;
	uint32_t tagSize;
};

struct ECC_key_stt {
	/** @brief public key structure */
	struct ECCpubKeyByteArray_stt  PubKeyByteArray;
	/** @brief private key structure */
	struct ECCprivKeyByteArray_stt PrivKeyByteArray;

	/** @brief Associated counter value (28 bits only) */
	union {
		uint32_t R;
		struct {
			uint32_t unused:4;
			uint32_t CNT28:28;
		} B;
	} CNT;

	/** @brief Usage restriction flags */
	union EXTENDED_KEY_flags EC_flags;

};

/*============================================================================*/
/**
 * @brief         Loads an ECC Key cleartext value in the CSE ECC RamKey
 *		  internal storage
 * @details       Loads a key value to be used when selecting index ECC_RAM_KEY
 *		  for ECC dedicated commands
 *                The provided key can be either a public key, a private key or
 *		  a couple
 *
 * @param[in]     P_pPubKeyX    pointer to a \ref ECCpubKeyByteArray_stt
 *		  structure for the public key
 * @param[in]     P_pPrivKey    pointer to a \ref ECCprivKeyByteArray_stt
 *		  structure for the private key
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
uint32_t CSE_ECC_LoadRamKey(struct ECCpubKeyByteArray_stt *P_pPubKey,
			    struct ECCprivKeyByteArray_stt *P_pPrivKey);

/*============================================================================*/
/**
 * @brief         Loads an EC Key in internal Non Volatile Memory or in RAM
 *		  location
 * @details       Key will be loaded at index described in the already prepared
 *		  messages
 *                including key usage restrictions (security flags), keyindex,
 *		  encrypted value
 *                and integrity & authenticity MACs for all the params.
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

 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_LoadKey(uint8_t *M1,
			 uint8_t *M2,
			 uint8_t *M3,
			 uint8_t *M4,
			 uint8_t *M5);

/*============================================================================*/
/**
 * @brief         Generate ECC Key Pair and store it in the specified key
 *		  location
 * @details       Generate an ECC key Pair and tore it in the Key location
 *		  specified by the provided parameter
 *
 * @return        Error code
 * @retval        CSE_NO_ERROR
 * @retval        CSE_GENERAL_ERR
 *
 * @api
 *
 * @pre
 *
 * @implements
 *
 * @note
 */
uint32_t CSE_ECC_generateLoadKeyPair(const vuint32_t P_ECkeyID,
				     const vuint32_t P_ECflags);

/*============================================================================*/
/**
 * @brief         Exports an Elliptic Curve Public Key
 * @details       Export an Elliptic Curve Public Key
 * @note          Only Public Key coordinates are exported: Counter and Flags
 *		  are not exported
 *
 * @note          Parameter 2 provides length of the container in which key is
 *		  to be exported and will be updated by
 *                the length of the exported key
 *
 * @param[in]     P_ECC_key_idx      ECC Key index
 * @param[in]     P_pPubKey Pointer to a \ref ECCpubKeyByteArray_stt
 *		  structure for the public key coordinates
 *
 * @return        Error code
 * @retval 0      When key was exported properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_exportPublicKey(vuint32_t P_ECC_key_idx,
				 struct ECCpubKeyByteArray_stt *P_pPubKey);

/*============================================================================*/
/**
 * @brief         Exports an Elliptic Curve Private Key
 * @details       Export an Elliptic Curve Private Key
 * @note          If it exist the corresponding Public Key is also exported
 *
 * @param[in]     M1    pointer to message1 (out) and index of Key to export
 *			(in)
 * @param[in]     M2    pointer to message2 (out) and size of memory allocated
 *			to M2 message
 * @param[in]     M3    pointer to message3
 *
 * @param[out]    M4    pointer to message4
 * @param[out]    M5    pointer to message5
 *
 * @return        Error code
 * @retval 0      When Private Key was exported properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_exportPrivateKey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				  uint8_t *M4, uint8_t *M5);

/*============================================================================*/
/**
 * @brief         Performs an ECC ECDSA Signature Generation
 * @details       Key will be loaded at index described in the already prepared
 *		  messages
 *                including key usage restrictions (security flags), keyindex,
 *		  encrypted value
 *                and integrity & authenticity MACs for all the parameters.
 *
 * @param[in]     P_ECC_key_idx  ECC Key index
 * @param[in]     P_pSrc         pointer to message buffer
 * @param[in]     P_byte_size    message length in bytes
 * @param[in]     P_hash_algo    descriptor of the hash algorithm to use
 *				 (E_SHA224, E_SHA256...)
 * @param[out]    P_pSign        pointer to destination signature buffer
 *
 * @return        Error code
 * @retval 0      When signature was generated properly
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ECDSA_sign(vuint32_t P_ECC_key_idx,
			    vuint8_t *P_pSrc, vuint32_t P_byte_size,
			    struct ECDSA_Signature_stt *P_pSign,
			    vuint32_t P_hash_type);

/*============================================================================*/
/**
 * @brief         Performs an ECC ECDSA Signature Verification
 * @details       Key will be loaded at index described in the already prepared
 *		  messages
 *                including key usage restrictions (security flags), keyindex,
 *		  encrypted value
 *                and integrity & authenticity MACs for all the parameters.
 *
 *
 * @param[in]     P_ECC_key_idx  ECC Key index
 * @param[in]     P_pSrc         pointer to message buffer
 * @param[in]     P_byte_size    message length in bytes
 * @param[in]     P_pSign        pointer to input reference signature buffer
 * @param[in]     P_hash_algo    descriptor of the hash algorithm to use
 *				 (E_SHA224, E_SHA256...)
 * @param[out]    P_pSuccess     pointer to result word location
 *				 (0 if failed, 1 if succeeded)
 *
 *
 * @return        Error code
 * @retval 0      When signature was verified (but comparison result is in P5)
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ECDSA_verify(vuint32_t P_ECC_key_idx,
			      vuint8_t *P_pSrc, vuint32_t P_byte_size,
			      struct ECDSA_Signature_stt *P_pSign,
			      vuint32_t P_hash_type,
			      uint32_t *P_pSuccess);

/*============================================================================*/
/**
 * @brief         Change Elliptic Curve
 * @details       Change Elliptic Curve amongst the supported NIST curves
 *
 * @param[in]     P_ECcurveID Elliptic Curve Identifier
 *
 * @return        Error code
 * @retval        CSE_NO_ERROR
 * @retval        CSE_GENERAL_ERR
 *
 * @api
 *
 * @pre
 *
 * @implements
 *
 * @note
 */
uint32_t CSE_ECC_changeCurve(const vuint32_t P_ECcurveID);

/*============================================================================*/
/**
 * @brief         Get Elliptic Curve Identifier
 * @details       Get the Identifier of the currently selected Elliptic Curve
 *
 * @param[out]    P_ECcurveID Elliptic Curve Identifier
 *
 * @return        Error code
 * @retval        CSE_NO_ERROR
 * @retval        CSE_GENERAL_ERR
 *
 * @api
 *
 * @pre
 *
 * @implements
 *
 * @note
 */
uint32_t CSE_ECC_getECcurve(uint32_t *P_pECcurveID);

/*============================================================================*/
/**
 * @brief         Perform ECIES Encryption
 * @details
 * *
 * @param[in]     P_ECC_key_idx       Identifier of the Decryption Public Key
 * @param[in]     P_pInputToEncrypt   Pointer to \ref ByteArrayDescriptor_stt:
 *				      Message to encrypt and size
 * @param[in]     P_pEciesSharedData  Pointer to \ref ECIES_sharedData_stt:
 *				      Shared Data
 * @param[out]    P_pEncMsg Pointer to \ref
 *				      ECIES_encMsg_stt:
 *				      Encrypted message and Tag
 *
 * @return        Error code
 * @retval 0      When ECIES encryption was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_EciesEncrypt(vuint32_t P_ECC_key_idx,
			      struct ByteArrayDescriptor_stt *P_pInputToEncrypt,
			      struct ECIES_sharedData_stt *P_pEciesSharedData,
			      struct ECIES_encMsg_stt *P_pEncMsg);

/*============================================================================*/
/**
 * @brief         Perform ECIES Decryption
 * @details
 * *
 * @param[in]     P_ECC_key_idx       Identifier of the Decryption Private Key
 * @param[in]     P_pInputToDecrypt   Pointer to \ref
 *				      ECIES_encMsg_stt:
 *				      Message to decrypt and size
 * @param[in]     P_pEciesSharedData  Pointer to \ref ECIES_sharedData_stt:
 *				      Shared Data
 * @param[out]	  P_pDecMsg	      Pointer to \ref ByteArrayDescriptor_stt:
 *				      Decrypted message
 *
 * @return        Error code
 * @retval 0      When ECIES decryption was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *		  ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_EciesDecrypt(vuint32_t P_ECC_key_idx,
			      struct ECIES_encMsg_stt *P_pInputToDecrypt,
			      struct ECIES_sharedData_stt *P_pEciesSharedData,
			      struct ByteArrayDescriptor_stt *P_pDecMsg);

#endif  /* _CSE_ext_ECC_INCLUDED_*/

/**
 * @}
 */
