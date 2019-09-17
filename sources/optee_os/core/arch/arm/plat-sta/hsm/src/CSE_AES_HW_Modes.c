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
 * @file    CSE_AES_HW_Modes.c
 * @brief   CSE general management module.
 * @details Set of functions used to check status, version, enable or disable
 *	    features.
 *
 *
 * @addtogroup CSE_driver CSE driver
 * @{
 * @addtogroup API Functions
 * @{
 */
#include "cse_typedefs.h"
#include "CSE_Constants.h"
#include "CSE_AES_HW_Modes.h"

#include "CSE_HAL.h"

/*============================================================================*/
/**
 * @brief          Performs AES encryption in ECB mode
 * @details        Encrypts the specified number of AES blocs from source to
 *                 destination
 *
 * @param[in]      key_idx      key index value
 * @param[in]      *psrc        pointer to the buffer to encrypt (plaintext)
 * @param[in]      block_nb     number of AES blocks to encrypt (one block is
 *				16 bytes)
 *
 * @param[out]     *pdst        pointer to the destination buffer where to store
 *				result (cyphertext)
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @return         Error code
 * @retval 0       When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre		   The specified key security flags must allow a encryption use
 *		   for the operation to succeed
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t cse_aes_encrypt_ecb(vuint32_t key_idx, vuint32_t *psrc,
			     vuint32_t *pdst, vuint32_t block_nb,
			     uint32_t ext_key_set)
{
	uint32_t ret = CSE_NO_ERR;

	/* !!! number of AES blocks (one block is 16 bytes) */
	if (ext_key_set) {
		ret = cse_hal_send_cmd4p(CSE_ENC_ECB_EXT, key_idx, block_nb,
					 (uint32_t)psrc, (uint32_t)pdst);
	} else {
		ret = cse_hal_send_cmd4p(CSE_ENC_ECB, key_idx, block_nb,
					 (uint32_t)psrc, (uint32_t)pdst);
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs AES decryption in ECB mode
 * @details        Decrypts the specified number of AES blocs from source to
 *		   destination
 *
 * @param[in]      key_idx      key index value
 * @param[in]      *psrc        pointer to the buffer to decrypt (cyphertext)
 * @param[in]      block_nb     number of AES blocks to encrypt (one block is
 *				16 bytes)
 *
 * @param[out]     *pdst        pointer to the destination buffer where to store
 *				result (plaintext)
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @return         Error code
 * @retval 0       When decryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre		   The specified key security flags must allow a encryption use
 *		   for the operation to succeed
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t cse_aes_decrypt_ecb(vuint32_t key_idx, vuint32_t *psrc,
			     vuint32_t *pdst, vuint32_t block_nb,
			     uint32_t ext_key_set)
{
	uint32_t ret = CSE_NO_ERR;

	/* !!! number of AES blocks (one block is 16 bytes) */
	if (ext_key_set) {
		ret = cse_hal_send_cmd4p(CSE_DEC_ECB_EXT, key_idx, block_nb,
					 (uint32_t)psrc, (uint32_t)pdst);
	} else {
		ret = cse_hal_send_cmd4p(CSE_DEC_ECB, key_idx, block_nb,
					 (uint32_t)psrc, (uint32_t)pdst);
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs AES encryption in CBC mode
 * @details        Encrypts the specified number of AES blocs from source to
 *		   destination
 *
 * @param[in]      key_idx      key index value
 * @param[in]      *psrc        pointer to the buffer to encrypt (plaintext)
 * @param[in]      *piv         pointer to the initialization vector to use
 * @param[in]      block_nb     number of AES blocks to encrypt (one block is
 *				16 bytes)
 *
 * @param[out]     *pdst        pointer of the destination buffer where to store
 *				result (cyphertext)
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @return         Error code
 * @retval 0       When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre		   The specified key security flags must allow a encryption use
 *		   for the operation to succeed
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t cse_aes_encrypt_cbc(vuint32_t key_idx, vuint32_t *psrc,
			     vuint32_t *pdst, vuint32_t *piv,
			     vuint32_t block_nb, uint32_t ext_key_set)
{
	uint32_t ret = CSE_NO_ERR;

	/* !!! number of AES blocks (one block is 16 bytes) */
	if (ext_key_set) {
		ret = cse_hal_send_cmd5p(CSE_ENC_CBC_EXT, key_idx,
					 (uint32_t)piv, block_nb,
					 (uint32_t)psrc, (uint32_t)pdst);
	} else {
		ret = cse_hal_send_cmd5p(CSE_ENC_CBC, key_idx, (uint32_t)piv,
					 block_nb, (uint32_t)psrc,
					 (uint32_t)pdst);
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs AES decryption in CBC mode
 * @details        Decrypts the specified number of AES blocks from source to
 *		   destination
 *
 * @param[in]      key_idx      key index value
 * @param[in]      *psrc        pointer to the buffer to decrypt (cyphertext)
 * @param[in]      *piv         pointer to the initialization vector to use
 * @param[in]      block_nb     number of AES blocks to encrypt (one block is
 *				16 bytes)
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @param[out]     *pdst	pointer to the destination buffer where to store
 *				result (plaintext)
 *
 * @return         Error code
 * @retval 0       When decryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre		   The specified key security flags must allow a encryption use
 *		   for the operation to succeed
 *
 * @implements     cf. CSE decryption function
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t cse_aes_decrypt_cbc(uint32_t key_idx, vuint32_t *psrc, vuint32_t *pdst,
			     uint32_t *piv, uint32_t block_nb,
			     uint32_t ext_key_set)
{
	uint32_t ret = CSE_NO_ERR;

	/* !!! number of AES blocks (one block is 16 bytes) */
	if (ext_key_set) {
		ret = cse_hal_send_cmd5p(CSE_DEC_CBC_EXT, key_idx,
					 (uint32_t)piv, block_nb,
					 (uint32_t)psrc, (uint32_t)pdst);
	} else {
		ret = cse_hal_send_cmd5p(CSE_DEC_CBC, key_idx, (uint32_t)piv,
					 block_nb, (uint32_t)psrc,
					 (uint32_t)pdst);
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs AES based CMAC computation
 * @details        Computes the MAC of message of xxx bits using the specified
 *		   Key
 *
 * @param[in]      key_idx      key index value
 * @param[in]      *pbitlength  pointer to 64bit length of message in bits
 *				( !!! address of the location of the length
 *				variable, not the value )
 * @param[in]      *pmessage    pointer to the message
 *
 * @param[out]     *output_mac  pointer to the buffer location to store computed
 *				MAC
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @return         Error code
 * @retval 0       When MAC was computed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre		   The specified key security flags must allow a MAC use for the
 *		   operation to succeed
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t cse_aes_generate_mac(vuint32_t key_idx, vuint64_t *pbitlength,
			      vuint32_t *pmessage, vuint32_t *output_mac,
			      uint32_t ext_key_set)
{
	uint32_t ret = CSE_NO_ERR;

	/* !!! number of AES blocks (one block is 16 bytes) */
	if (ext_key_set) {
		ret = cse_hal_send_cmd4p(CSE_GENERATE_MAC_EXT, key_idx,
					 (uint32_t)pbitlength,
					 (uint32_t)pmessage,
					 (uint32_t)output_mac);
	} else {
		ret = cse_hal_send_cmd4p(CSE_GENERATE_MAC, key_idx,
					 (uint32_t)pbitlength,
					 (uint32_t)pmessage,
					 (uint32_t)output_mac);
	}

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs AES based CMAC verification
 * @details        Computes the MAC of message of xxx bits using the specified
 *		   Key and compares it with the provided reference
 *
 * @param[in]      key_idx          key index value
 * @param[in]      *pbitlength      pointer to 64bit length of message in bits
 *				    ( !!! address of the location of the length
 *				    variable, not the value )
 * @param[in]      *pmsg	    pointer to the message
 * @param[in]      *input_mac       pointer to the reference MAC to compare with
 * @param[in]      mac_bitlength    length of message in bits (value, not
 *				    pointer)
 *
 * @param[out]     *status	    pointer to a variable where the MAC
 *				    verification status will be stored
 *
 * @param[in]      ext_key_set	    1 to use extended key set, 0 for normal key
 *				    set
 *
 * @return         Error code
 * @retval         CSE_NO_ERR   When MAC was computed & checked properly
 *				- DOES NOT TELL IF reference MAC is matching
 *				the computed one
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @pre            The specified key security flags must allow a MAC use for the
 *		   operation to succeed
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t cse_aes_verify_mac(vuint32_t key_idx, vuint64_t *pbitlength,
			    vuint32_t *pmsg, vuint32_t *input_mac,
			    vuint32_t mac_bitlength, vuint32_t *status,
			    uint32_t ext_key_set)
{
	uint32_t ret = CSE_NO_ERR;

	/* !!! number of AES blocks (one block is 16 bytes) */
	if (ext_key_set) {
		ret = cse_hal_send_cmd5p(CSE_VERIFY_MAC_EXT, key_idx,
					 (uint32_t)pbitlength, (uint32_t)pmsg,
					 (uint32_t)input_mac, mac_bitlength);
	} else {
		ret = cse_hal_send_cmd5p(CSE_VERIFY_MAC, key_idx,
					 (uint32_t)pbitlength, (uint32_t)pmsg,
					 (uint32_t)input_mac, mac_bitlength);
	}

	/*
	 * read comparison result - might be not applicable if operation failed
	 * - see error code
	 */
	*status = CSE->P5.R;

	return ret;
}

/**
 * @}
 * @}
 */
