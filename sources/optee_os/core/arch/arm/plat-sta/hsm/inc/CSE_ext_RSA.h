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
 * @file    CSE_ext_RSA.c
 * @brief   CSE RSA commands management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup SHE-ext_driver
 * @{
 */

#ifndef _CSE_EXT_RSA_H_
#define _CSE_EXT_RSA_H_

#include "CSE_ext_extendKey.h"
#include "cse_types.h"

/**
 * @brief  Structure type for RSA key
 */
struct rsa_simple_struct {
	uint8_t *address;	/*!< pointer to buffer */
	int32_t bytesize;	/*!< size in bytes */
};

/**
 * @brief  Structure type for RSA public key
 */
struct rsa_pub_key_stt {
	uint8_t *pmodulus;	/*!< RSA Modulus */
	int32_t modulus_size;	/*!< Size of RSA Modulus */
	uint8_t *pexponent;	/*!< RSA Public Exponent */
	int32_t exponent_size;	/*!< Size of RSA Public Exponent */
};

/**
 * @brief  Structure type for RSA private key
 */
struct rsa_priv_key_stt {
	uint8_t *pmodulus;	/*!< RSA Modulus */
	int32_t modulus_size;	/*!< Size of RSA Modulus */
	uint8_t *pexponent;	/*!< RSA Private Exponent */
	int32_t exponent_size;	/*!< Size of RSA Private Exponent */
};

/**
 * @brief	CSE internal key index values
 * @details	Used to access Key structure in the CSE internal RAM array
 **/
typedef uint32_t CSE_internal_RSA_key_ID_t;

/**
 * @brief  Structure type for RSA public key
 */
struct rsa_key_stt {
	/* @brief public key structure */
	struct rsa_pub_key_stt pub_key;
	/* @brief private key structure */
	struct rsa_priv_key_stt priv_key;

	/** @brief Associated counter value (28 bits only) */
	union {
		uint32_t R;
		struct {
			uint32_t unused:4;
			uint32_t CNT28:28;
		} B;
	} CNT;

	/** @brief description and usage restriction flags */
	union extended_key_flags rsa_flags;

	/** @brief tell if key location is empty or not */
	uint32_t empty;
};

/*============================================================================*/
/**
 * @brief	Loads a RSA Key cleartext value in the CSE RSA RamKey internal
 *		storage
 * @details	Loads a key value to be used when selecting index RSA_RAM_KEY
 *		for RSA dedicated commands
 *		The provided key can be either a public key, a private key or
 *		a couple
 *
 * @param[in]	key_pt	pointer to cleartext key value
 *
 * @return	Error code
 * @retval	0	When key was loaded properly
 * @retval	1..21   In case of error - the error code values are the CSE
 *		returned ones
 *		see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_load_ramkey(struct rsa_simple_struct *pmodulus,
				    struct rsa_simple_struct *ppubkey,
				    struct rsa_simple_struct *pprivkey);

/*============================================================================*/
/**
 * @brief	Exports the RSA RAM Key
 * @details	Compute and provide all fields required to load the key value
 *		that was in the RSA RAM KEY location in a non volatile key
 *		location
 *
 * @param[out]	M1     pointer to buffer to store the M1 message
 *		       (UID, KeyID, Auth ID - 128 bits total)
 * @param[out]	M2     pointer to buffer to store the M2 message
 *		       (security flags,  - 256 bits )
 * @param[out]	M3     pointer to buffer to store the M3 message
 *		       (MAC of M1 and M2 - 128 bits)
 * @param[out]	M4     pointer to buffer to store the M4 message
 *		       (IDs | counter - 256 bits)
 * @param[out]	M5     pointer to buffer to store the M5 message
 *		       (MAC of M4 - 128 bits )
 *
 * @return	Error code
 * @retval	0       When key was exported properly
 * @retval	1..21   In case of error - the error code values are the CSE
 *			returned ones
 *		see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note	The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_export_ramkey(uint8_t *M1, uint8_t *M2,
				      uint8_t *M3, uint8_t *M4,
				      uint8_t *M5);

/*============================================================================*/
/**
 * @brief	Exports the RSA Public Key
 * @details	Exports the RSA Public Key identified by the Key index.
 *		Public Key and Modulus are exported.
 * @note	Only Public Key coordinates are exported: Counter and Flags
 *		are not exported
 *
 * @param[in]	rsa_key_idx	Index of the RSA key to export
 * @param[in]	*ppub_key	Pointer to a \ref rsa_pub_key_stt structure for
 *				the RSA Public Key
 *
 * @return	Error code
 * @retval	0       When key was exported properly
 * @retval	1..21   In case of error - the error code values are the CSE
 *		returned ones
 *		see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note    The function is blocking and waits for operation completion
 */
uint32_t cse_rsa_export_publickey(vuint32_t rsa_key_idx,
				  struct rsa_pub_key_stt *ppub_key);

/*============================================================================*/
/**
 * @brief	Exports the RSA Private Key either from RAM or NVM locations
 * @details	Compute and provide all M1, ..., M5 messages
 *
 * @param[out]	M1	pointer to buffer to store the M1 message
 *			(UID, KeyID, Auth ID, Size of M2 in Byte - 256 bits)
 * @param[out]	M2	pointer to buffer to store the M2 message
 *			(security flags,  - 4256 bits for RSA2048)
 * @param[out]	M3	pointer to buffer to store the M3 message
 *			(MAC of M1 and M2 - 128 bits)
 * @param[out]	M4	pointer to buffer to store the M4 message
 *			(IDs | counter - 256 bits)
 * @param[out]	M5	pointer to buffer to store the M5 message
 *			(MAC of M4 - 128 bits )
 *
 * @return	Error code
 * @retval	0       When key was exported properly
 * @retval	1..21   In case of error - the error code values are the CSE
 *		returned ones
 *		see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note	The function is blocking and waits for operation completion
 */
uint32_t cse_rsa_export_privatekey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				   uint8_t *M4, uint8_t *M5);

/*============================================================================*/
/**
 * @brief	Loads a RSA Key in internal non volatile memory
 * @details	 Key will be loaded at index described in the already prepared
 *		messages including key usage restrictions (security flags),
 *		keyindex, encrypted value and integrity & authenticity MACs
 *		for all the params.
 *
 * @param[in]	M1	pointer to message1
 * @param[in]	M2	pointer to message2
 * @param[in]	M3	pointer to message3
 *
 * @param[out]	M4	pointer to message4
 * @param[out]	M5	pointer to message5
 *
 * @return	Error code
 * @retval	0	When key was loaded properly
 * @retval	1..21	In case of error - the error code values are the CSE
 *		returned ones
 *		see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note	The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_load_key(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				 uint8_t *M4, uint8_t *M5);

/*============================================================================*/
/**
 * @brief	Performs a RSA PKCS1v1.5 signature generation
 * @details	Key will be loaded at index described in the already prepared
 *		messages including key usage restrictions (security flags),
 *		keyindex, encrypted value and integrity & authenticity MACs for
 *		all the params.
 *
 *
 * @param[in]	rsa_key_idx	RSA Key index
 * @param[in]	*psrc		pointer to message buffer
 * @param[in]	byte_size	message length in bytes
 * @param[out]	*psign		pointer to destination signature buffer
 * @param[out]	hash_algo	descriptor of the hash algorithm to use
 *				(E_SHA224, E_SHA256...)
 *
 * @return	Error code
 * @retval	0	When signature was generated properly
 *
 * @note	The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_pkcs1_15_sign(vuint32_t rsa_key_idx, vuint8_t *psrc,
				      vuint32_t byte_size, vuint8_t *psign,
				      vuint32_t hash_type);

/*============================================================================*/
/**
 * @brief	Performs a RSA PKCS1v1.5 signature verification
 * @details	Key will be loaded at index described in the already prepared
 *		messages including key usage restrictions (security flags),
 *		keyindex, encrypted value and integrity & authenticity MACs for
 *		all the params.
 *
 *
 * @param[in]	rsa_key_idx	RSA Key index
 * @param[in]	*psrc		pointer to message buffer
 * @param[in]	byte_size	message length in bytes
 * @param[in]	*psign		pointer to input reference signature buffer
 * @param[in]	hash_algo	descriptor of the hash algorithm to use
 *				(E_SHA224, E_SHA256...)
 * @param[out]	psuccess	pointer to result word location
 *				(0 if failed, 1 if succeeded)
 *
 *
 * @return	Error code
 * @retval	0	When signature was verified (but comparison result is in
 *			P5)
 *
 * @note	The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_pkcs1_15_verify(vuint32_t rsa_key_idx, vuint8_t *psrc,
					vuint32_t byte_size, vuint8_t *psign,
					vuint32_t hash_type,
					uint32_t *psuccess);

/*============================================================================*/
/**
 * @brief	Performs RSA PKCS1v1.5 encryption
 * @details	Key will be loaded at index described in the already prepared
 *		messages
 *
 * @param[in]	rsa_key_idx	RSA Key index
 * @param[in]	*psrc		pointer to input cleartext message buffer
 * @param[in]	src_size	message length in bytes
 * @param[in]	*pdst		pointer to output cyphertext message buffer
 * @param[out]	pdst_size	pointer to destination buffer size
 *
 *
 * @return	Error code
 * @retval	0	When signature was verified (but comparison result is in
 *			P5)
 *
 * @note	The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_pkcs1_15_encrypt(vuint32_t rsa_key_idx, vuint8_t *psrc,
					 vuint32_t src_size, vuint8_t *pdst,
					 vuint32_t *pdst_size);

/*============================================================================*/
/**
 * @brief	Performs RSA PKCS1v1.5 decryption
 * @details	Key will be loaded at index described in the already prepared
 *		messages
 *
 * @param[in]	rsa_key_idx	RSA Key index
 * @param[in]	*psrc		pointer to input cleartext message buffer
 * @param[in]	src_size	message length in bytes
 * @param[in]	*pdst		pointer to output cyphertext message buffer
 * @param[out]	*pdst_size	pointer to destination buffer size
 *
 *
 * @return	Error code
 * @retval	0	When signature was verified (but comparison result is
 *			in P5)
 *
 * @note	The function is blocking and waits for operation completion
 */
extern uint32_t cse_rsa_pkcs1_15_decrypt(vuint32_t rsa_key_idx, vuint8_t *psrc,
					 vuint32_t src_size, vuint8_t *pdst,
					 vuint32_t *pdst_size);

#endif /* _CSE_EXT_RSA_H_ */
/**
 * @}
 */
