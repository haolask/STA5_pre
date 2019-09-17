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

#include "cse_typedefs.h"
#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_RSA.h"

/*============================================================================*/
/**
 * @brief          Loads a RSA Key cleartext value in the CSE RSA RamKey
 *                 internal storage
 * @details        Loads a key value to be used when selecting index RSA_RAM_KEY
 *		   for RSA dedicated commands
 *                 The provided key can be either a public key, a private key or
 *		   a couple
 *
 * @param[in]      key_pt    pointer to cleartext key value
 *
 * @return         Error code
 * @retval 0       When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *                 see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_LoadRamKey(struct RSA_simple_struct *pmodulus,
			    struct RSA_simple_struct *ppubkey,
			    struct RSA_simple_struct *pprivkey)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd3p(CSE_RSA_LOAD_RAM_KEY,
				 (uint32_t)pmodulus, (uint32_t)ppubkey,
				 (uint32_t)pprivkey);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Exports the RSA Public Key
 * @details        Exports the RSA Public Key identified by the Key index.
 *		   Public Key and Modulus are exported.
 * @note           Only Public Key coordinates are exported: Counter and Flags
 *		   are not exported
 *
 * @param[in]      P_RSA_key_idx  Index of the RSA key to export
 * @param[in]      P_pPubKey      Pointer to a \ref RSApubKey_stt structure for
 *		   the RSA Public Key

 * @return         Error code
 * @retval 0       When key was exported properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *                 see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_exportPublicKey(vuint32_t P_RSA_key_idx,
				 struct RSApubKey_stt *P_pPubKey)
{
	/*
	 * Ask the CSE to export all the fields we will need to store the Key in
	 * a non volatile location
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd2p(CSE_RSA_EXPORT_PUBLIC_KEY,
				 (uint32_t)P_RSA_key_idx,
				 (uint32_t)P_pPubKey);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Exports the RSA Private Key either from RAM or NVM locations
 * @details        Compute and provide all M1, ..., M5 messages
 *
 * @param[out]     M1     pointer to buffer to store the M1 message
 *			  (UID, KeyID, Auth ID, Size of M2 in Byte - 256 bits)
 * @param[out]     M2     pointer to buffer to store the M2 message
 *			  (security flags,  - 4256 bits for RSA2048)
 * @param[out]     M3     pointer to buffer to store the M3 message
 *			  (MAC of M1 and M2 - 128 bits)
 * @param[out]     M4     pointer to buffer to store the M4 message
 *			  (IDs | counter - 256 bits)
 * @param[out]     M5     pointer to buffer to store the M5 message
 *			  (MAC of M4 - 128 bits )
 *
 * @return         Error code
 * @retval 0       When key was exported properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *                 see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_exportPrivateKey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				  uint8_t *M4, uint8_t *M5)
{
	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_RSA_EXPORT_PRIVATE_KEY,
				 (uint32_t)M1, (uint32_t)M2, (uint32_t)M3,
				 (uint32_t)M4, (uint32_t)M5);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Loads a RSA Key in internal non volatile memory
 * @details        Key will be loaded at index described in the already prepared
 *		   messages including key usage restrictions (security flags),
 *		   keyindex, encrypted value and integrity & authenticity MACs
 *		   for all the params.
 *
 * @param[in]      M1    pointer to message1
 * @param[in]      M2    pointer to message2
 * @param[in]      M3    pointer to message3
 *
 * @param[out]     M4    pointer to message4
 * @param[out]     M5    pointer to message5
 *
 * @return         Error code
 * @retval 0       When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *                 see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_LoadKey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
			 uint8_t *M4, uint8_t *M5)
{
	/*
	 * Ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_RSA_LOAD_KEY, (uint32_t)M1, (uint32_t)M2,
				 (uint32_t)M3, (uint32_t)M4, (uint32_t)M5);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs a RSA PKCS1v1.5 signature generation
 * @details        Key will be loaded at index described in the already prepared
 *		   messages including key usage restrictions (security flags),
 *		   keyindex, encrypted value and integrity & authenticity MACs
 *		   for all the params.
 *
 *
 * @param[in]      RSA_key_idx  RSA Key index
 * @param[in]      src          pointer to message buffer
 * @param[in]      byte_size    message length in bytes
 * @param[out]     sign         pointer to destination signature buffer
 * @param[in]      hash_algo    descriptor of the hash algorithm to use
 *				(E_SHA224, E_SHA256...)
 *
 * @return         Error code
 * @retval 0       When signature was generated properly
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_PKCS1_15_sign(vuint32_t RSA_key_idx, vuint8_t *src,
			       vuint32_t byte_size, vuint8_t *sign,
			       vuint32_t hash_type)
{
	/*
	 * Ask the CSE to load the Key in the key internal location described in
	 * the provided messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_RSA_PKCS1_15_SIGN, (uint32_t)RSA_key_idx,
				 (uint32_t)src, (uint32_t)byte_size,
				 (uint32_t)sign, (uint32_t)hash_type);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs a RSA PKCS1v1.5 signature verification
 * @details        Key will be loaded at index described in the already prepared
 *		   messages including key usage restrictions (security flags),
 *		   keyindex, encrypted value and integrity & authenticity MACs
 *		   for all the params.
 *
 *
 * @param[in]      RSA_key_idx  RSA Key index
 * @param[in]      src          pointer to message buffer
 * @param[in]      byte_size    message length in bytes
 * @param[in]      sign          pointer to input reference signature buffer
 * @param[in]      hash_algo    descriptor of the hash algorithm to use
 *				(E_SHA224, E_SHA256...)
 * @param[out]     psuccess     pointer to result word location
 *				(1 if failed, 0 if succeeded)
 *
 *
 * @return         Error code
 * @retval 0       When signature was verified (but comparison result is in P5)
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_PKCS1_15_verify(vuint32_t RSA_key_idx, vuint8_t *src,
				 vuint32_t byte_size, vuint8_t *sign,
				 vuint32_t hash_type, uint32_t *psuccess)
{
	/*
	 * Ask the CSE to load the Key in the key internal location described in
	 * the provided messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_RSA_PKCS1_15_VERIFY, (uint32_t)RSA_key_idx,
				 (uint32_t)src, (uint32_t)byte_size,
				 (uint32_t)sign, (uint32_t)hash_type);

	*psuccess = CSE->P5.R;

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs RSA PKCS1v1.5 encryption
 * @details        Key will be loaded at index described in the already prepared
 *		   messages
 *
 * @param[in]      RSA_key_idx  RSA Key index
 * @param[in]      src          pointer to input cleartext message buffer
 * @param[in]      src_size     message length in bytes
 * @param[in]      dst          pointer to output cyphertext message buffer
 * @param[out]     pdst_size    pointer to destination buffer size
 *
 *
 * @return         Error code
 * @retval 0       When signature was verified (but comparison result is in P5)
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_PKCS1_15_encrypt(vuint32_t RSA_key_idx, vuint8_t *src,
				  vuint32_t src_size, vuint8_t *dst,
				  vuint32_t *pdst_size)
{
	/*
	 * Ask the CSE to load the Key in the key internal location described in
	 * the provided messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_RSA_PKCS1_15_ENCRYPT,
				 (uint32_t)RSA_key_idx, (uint32_t)src,
				 (uint32_t)src_size, (uint32_t)dst,
				 (uint32_t)(*pdst_size));

	*pdst_size = CSE->P5.R;

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs RSA PKCS1v1.5 decryption
 * @details        Key will be loaded at index described in the already prepared
 *		   messages
 *
 * @param[in]      RSA_key_idx  RSA Key index
 * @param[in]      src          pointer to input cleartext message buffer
 * @param[in]      src_size     message length in bytes
 * @param[in]      dst          pointer to output cyphertext message buffer
 * @param[out]     pdst_size    pointer to destination buffer size
 *
 *
 * @return         Error code
 * @retval 0       When signature was verified (but comparison result is in P5)
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_RSA_PKCS1_15_decrypt(vuint32_t RSA_key_idx, vuint8_t *src,
				  vuint32_t src_size, vuint8_t *dst,
				  vuint32_t *pdst_size)
{
	/*
	 * Ask the CSE to load the Key in the key internal location described in
	 * the provided messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_RSA_PKCS1_15_DECRYPT,
				 (uint32_t)RSA_key_idx, (uint32_t)src,
				 (uint32_t)src_size, (uint32_t)dst,
				 (uint32_t)(*pdst_size));

	*pdst_size = CSE->P5.R;

	return ret;
}

/**
 * @}
 */
