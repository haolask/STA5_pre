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
#ifndef _CSE_EXT_ECC_INCLUDED_
#define _CSE_EXT_ECC_INCLUDED_

#include "cse_types.h"
#include "CSE_ext_extendKey.h"
#include "CSE_Constants.h"

/**
  * @brief Structure type for Scalar
  */
struct ecc_scalar_stt {
    uint8_t  *address;    /*!< pointer to buffer */
    uint32_t  byteSize;   /*!< size in bytes */
};

/**
 * @brief  Structure type for ECC public key
 *
 * Four fields
 *  - 1) Pointer to a byte array containing the x coordinate of the public key
 *  - 2) Size of the x coordinate of the public key
 *  - 3) Pointer to a byte array containing the y coordinate of the public key
 *  - 4) Size of the y coordinate of the public key
 */
struct ecc_pub_key_stt {
	/*!< ECC Public Key X coordinate */
	const uint8_t *pub_key_coordX;
	/*!< Size of ECC Public Key X coordinate */
	int32_t pub_key_coordX_size;
	/*!< ECC Public Key Y coordinate */
	const uint8_t *pub_key_coordY;
	/*!< Size of ECC Public Key Y coordinate */
	int32_t pub_key_coordY_size;
};

/**
 * @brief  Structure type for ECC private key
 * Two fields
 *  - 1) Pointer to a byte array containing the private key
 *  - 2) Size of the private key
 */
struct ecc_priv_key_stt {
	/*!< ECC Private */
	const uint8_t *priv_key;
	/*!< Size of ECC Private Key */
	int32_t priv_key_size;
};

struct ecdsa_signature_stt {
	const vuint8_t *sigR;
	vuint32_t sigR_size;
	const vuint8_t *sigS;
	vuint32_t sigS_size;
};

/**
 * @brief  Structure type for ECIES Message key
 */
struct ecc_ecies_msg_stt {
	/*!< pointer to buffer */
	uint8_t *address;
	/*!< size in bytes */
	uint32_t byteSize;
};

/**
 * @brief  Structure type for ECIES Shared Data 1
 * @details Optional parameters to be used for Key Derivation Function
 * Two fields
 *  - 1) Pointer to a byte array containing Shared data 1
 *  - 2) Size of Shared Data 1
 */

struct ecies_shared_data1_stt {
	uint8_t *shared_data;
	uint32_t shared_data_size;
};

/**
 * @brief   Structure type for ECIES Shared Data 2
 * @details Optional parameters to be used for Message Authentication
 *	     Code function
 * Two fields
 *  - 3) Pointer to a byte array containing Shared Data 2
 *  - 4) Size of Shared Data 2
 */

struct ecies_shared_data2_stt {
	uint8_t *shared_data;
	uint32_t shared_data_size;
};

/**
 * @brief  Structure type for ECIES Shared Data
 * Two fields
 *  - 1) Structure \ref ECIES_sharedData1_stt defining Shared Data 1
 *  - 2) Structure \ref ECIES_sharedData1_stt defining Shared Data 2
 */

struct ecies_shared_data_stt {
	struct ecies_shared_data1_stt *pshared_data1_stt;
	struct ecies_shared_data2_stt *pshared_data2_stt;
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

struct ecies_enc_msg_stt {
	struct ecc_pub_key_stt *pephem_pub_key;
	uint8_t *pecies_enc_msg;
	uint32_t ecies_enc_msg_size;
	uint8_t *ptag;
	uint32_t tag_size;
};

struct ecc_key_stt {
	/*!< public key structure */
	struct ecc_pub_key_stt  pub_key_array;
	/*!< private key structure */
	struct ecc_priv_key_stt priv_key_array;

	/*!< Associated counter value (28 bits only) */
	union {
		uint32_t R;
		struct {
			uint32_t unused:4;
			uint32_t CNT28:28;
		} B;
	} CNT;

	/*!< Usage restriction flags */
	union extended_key_flags EC_flags;

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
 * @param[in]     *ppub_Key	pointer to a \ref ecc_pub_key_stt
 *				structure for the public key
 * @param[in]     *ppriv_key	pointer to a \ref ecc_priv_key_stt
 *				structure for the private key
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
uint32_t cse_ecc_load_ramkey(struct ecc_pub_key_stt *ppub_key,
			     struct ecc_priv_key_stt *ppriv_key);

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
uint32_t cse_ecc_load_key(uint8_t *M1, uint8_t *M2, uint8_t *M3, uint8_t *M4,
			  uint8_t *M5);

/*============================================================================*/
/**
 * @brief         Convert size of M2 for the RAM Key Export patch for TC3p Cut2BF
 * @details *
 * @param[in]      M1    pointer to message1
 * @param[in]      M2    pointer to message2
 * @param[in]      M3    pointer to message3
 *
 * @return        Error code
 * @retval 0      When key was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned ones
 *                see details in CSE_ECR register field description table 743

 * @api
 *
 * @implements
 *
 * @note    The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_convert_M2_size_ram_key_export(uint8_t* M1, uint8_t* M2,
						uint8_t* M3);

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
uint32_t cse_ecc_generate_load_keypair(const vuint32_t ecc_key_id,
				       const vuint32_t ecc_flags);

/*============================================================================*/
/**
 * @brief         Regenerate ECC Key Pair and store it in the specified key
 *                location
 * @details       Regenerate an ECC key Pair with security control following
 *                SHE specification and store it in the Key location
 *                specified by the provided parameter
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
uint32_t cse_ecc_regenerate_nvm_keypair(uint8_t *m1, uint8_t *m2, uint8_t *m3);

/**
 * @brief    Compute scalar multiplication and addition
 * @details  Compute ((d * r) mod n + e) mod n
 *
 * @param[in]   ec_seed_key_id    Seed Key index
 * @param[in]   pscalar_mult      Pointer to multiplier descriptor
 * @param[in]   pscalar_addend    Pointer to addend descriptor
 * @param[in]   ec_priv_key_id    Key index of the calculated private key
 *
 * @return    Error code
 * @retval    CSE_NO_ERROR
 * @retval    CSE_GENERAL_ERR
 *
 * @api
 *
 * @pre
 *
 * @implements
 *
 * @note
 */
uint32_t cse_ecc_scalar_mult_add(const vuint32_t ec_seed_key_id,
				 struct ecc_scalar_stt * pscalar_mult,
				 struct ecc_scalar_stt * pscalar_addend,
				 const vuint32_t ec_priv_key_id);

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
 * @param[in]     ecc_key_idx		ECC Key index
 * @param[in]     *ppub_key		Pointer to a \ref ecc_pub_key_stt
 *					structure for the public key coordinates
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
uint32_t cse_ecc_export_publickey(vuint32_t ecc_key_idx,
				  struct ecc_pub_key_stt *ppub_key);

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
uint32_t cse_ecc_export_privatekey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
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
 * @param[in]     ecc_key_idx    ECC Key index
 * @param[in]     *psrc          pointer to message buffer
 * @param[in]     msg_size       message length in bytes
 * @param[in]     hash_type      descriptor of the hash algorithm to use
 *				 (E_SHA224, E_SHA256...)
 * @param[out]    *psign         pointer to destination signature buffer
 *
 * @return        Error code
 * @retval 0      When signature was generated properly
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecdsa_sign(vuint32_t ecc_key_idx,
			    vuint8_t *psrc, vuint32_t msg_size,
			    struct ecdsa_signature_stt *psign,
			    vuint32_t hash_type);

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
 * @param[in]     ecc_key_idx    ECC Key index
 * @param[in]     *psrc          pointer to message buffer
 * @param[in]     msg_size       message length in bytes
 * @param[in]     *psign         pointer to input reference signature buffer
 * @param[in]     hash_type      descriptor of the hash algorithm to use
 *				 (E_SHA224, E_SHA256...)
 * @param[out]    success        pointer to result word location
 *				 (0 if failed, 1 if succeeded)
 *
 *
 * @return        Error code
 * @retval 0      When signature was verified (but comparison result is in P5)
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecdsa_verify(vuint32_t ecc_key_idx,
			      vuint8_t *psrc, vuint32_t msg_size,
			      struct ecdsa_signature_stt *psign,
			      vuint32_t hash_type,
			      uint32_t *success);

/*============================================================================*/
/**
 * @brief         Performs an ECC ECDSA Digest Signature Generation
 * @details       Key will be loaded at index described in the already prepared
 *		  messages
 *                including key usage restrictions (security flags), keyindex,
 *		  encrypted value
 *                and integrity & authenticity MACs for all the parameters.
 *
 * @param[in]     ecc_key_idx    ECC Key index
 * @param[in]     *pdigest       pointer to digest buffer
 * @param[in]     digest_size    digest length in bytes
 * @param[out]    *psign         pointer to destination signature buffer
 *
 * @return        Error code
 * @retval 0      When signature was generated properly
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecdsa_digest_sign(vuint32_t ecc_key_idx,
				   vuint8_t *pdigest, vuint32_t digest_size,
				   struct ecdsa_signature_stt *psign);

/*============================================================================*/
/**
 * @brief         Performs an ECC ECDSA Digest Signature Verification
 * @details       Key will be loaded at index described in the already prepared
 *		  messages
 *                including key usage restrictions (security flags), keyindex,
 *		  encrypted value
 *                and integrity & authenticity MACs for all the parameters.
 *
 *
 * @param[in]     ecc_key_idx    ECC Key index
 * @param[in]     *pdigest       pointer to message buffer
 * @param[in]     digest_size    message length in bytes
 * @param[in]     *psign         pointer to input reference signature buffer
 * @param[out]    *psuccess      pointer to result word location
 *				 (0 if failed, 1 if succeeded)
 *
 *
 * @return        Error code
 * @retval 0      When signature was verified (but comparison result is in P5)
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecdsa_digest_verify(vuint32_t ecc_key_idx, vuint8_t *pdigest,
				     vuint32_t digest_size,
				     struct ecdsa_signature_stt *psign,
				     uint32_t *psuccess);

/*============================================================================*/
/**
 * @brief         Change Elliptic Curve
 * @details       Change Elliptic Curve amongst the supported NIST curves
 *
 * @param[in]     ecc_curve_id		Elliptic Curve Identifier
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
uint32_t cse_ecc_change_curve(const vuint32_t ecc_curve_id);

/*============================================================================*/
/**
 * @brief         Get Elliptic Curve Identifier
 * @details       Get the Identifier of the currently selected Elliptic Curve
 *
 * @param[out]    *pecc_curve_id	Elliptic Curve Identifier
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
uint32_t cse_ecc_get_ecc_curve(uint32_t *pecc_curve_id);

/*============================================================================*/
/**
 * @brief         Perform ECIES Encryption
 * @details
 * *
 * @param[in]     ecc_key_idx         Identifier of the Decryption Public Key
 * @param[in]     *pinput_toencrypt   Pointer to \ref byte_array_desc_stt:
 *                                    Message to encrypt and size
 * @param[in]     *pecies_shared_data Pointer to \ref ecies_shared_data_stt:
 *                                    Shared Data
 * @param[out]    *penc_msg	      Pointer to \ref ecies_enc_msg_stt
 *                                    Encrypted message and Tag
 *
 * @return        Error code
 * @retval 0      When ECIES encryption was loaded properly
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecies_encrypt(vuint32_t ecc_key_idx,
			       struct byte_array_desc_stt *pinput_toencrypt,
			       struct ecies_shared_data_stt *pecies_shared_data,
			       struct ecies_enc_msg_stt *penc_msg);

/*============================================================================*/
/**
 * @brief         Perform ECIES Decryption
 * @details
 * *
 * @param[in]     ecc_key_idx         Identifier of the Decryption Private Key
 * @param[in]     *pinput_toencrypt   Pointer to \ref ecies_enc_msg_stt
 *				      Message to decrypt and size
 * @param[in]     *pecies_shared_data Pointer to \ref ecies_shared_data_stt:
 *				      Shared Data
 * @param[out]    *pdec_msg           Pointer to \ref byte_array_desc_stt:
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
uint32_t cse_ecc_ecies_decrypt(vuint32_t ecc_key_idx,
			       struct ecies_enc_msg_stt *pinput_toencrypt,
			       struct ecies_shared_data_stt *pecies_shared_data,
			       struct byte_array_desc_stt *pdec_msg);

#endif  /* _CSE_ext_ECC_INCLUDED_*/

/**
 * @}
 */
