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

#include "config.h"
#include "cse_typedefs.h"
#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_ECC.h"

/*============================================================================*/
/**
 * @brief         Loads an ECC Key cleartext value in the CSE ECC RamKey
 *                internal storage
 * @details       Loads a key value to be used when selecting index ECC_RAM_KEY
 *                for ECC dedicated commands
 *                The provided key can be either a public key, a private key or
 *                a couple
 *
 * @param[in]     *ppub_key     pointer to a \ref ecc_pub_key_stt
 *                              structure for the public key
 * @param[in]     *ppriv_key    pointer to a \refecc_priv_key_stt
 *                              structure for the private key
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
uint32_t cse_ecc_load_ramkey(struct ecc_pub_key_stt *ppub_key,
			     struct ecc_priv_key_stt *ppriv_key)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the address of location containing the EC Public Key x & y
	 * coordinates
	 * P2 is the address of location containing the EC private Key
	 */

	ret = cse_hal_send_cmd2p(CSE_ECC_LOAD_RAM_KEY,
				 (uint32_t)ppub_key, (uint32_t)ppriv_key);

	return ret;
}

/*============================================================================*/
/**
 * @brief         Generate ECC Key Pair and store it in the specified key
 *		  location
 * @details       Generate an ECC key Pair and store it in the Key location
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
				       const vuint32_t ecc_flags)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the index of the Key location in which the generated key will
	 * be stored
	 * P2 is the Usage Restriction Flags that must be applied to the key
	 */

	ret = cse_hal_send_cmd2p(CSE_ECC_GENERATE_LOAD_KEY_PAIR,
				 (uint32_t)ecc_key_id,
				 (uint32_t)ecc_flags);

	return ret;
} /* End of cse_ecc_generate_load_keypair */

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
uint32_t cse_ecc_regenerate_nvm_keypair(uint8_t *m1, uint8_t *m2, uint8_t *m3)
{
	uint32_t ret = CSE_NO_ERR;

	/*
	 * Write parameters: i.e. address of location in which M1 to M3 message
	 * will be written.
	 * P1 is the index of the EC private key to be exported (in).
	 * P2 contains the size allocated to the buffer that will receive
	 * the M2 message (in) address and is the address in which the M2
	 * message will be written (out).
	 * P3 is the address in which M3 message will be written (out)
	 */

	ret = cse_hal_send_cmd3p(CSE_ECC_REGENERATE_AND_LOAD_KEY_PAIR,
				 (uint32_t)m1, (uint32_t)m2, (uint32_t)m3);

	return ret;
} /* End of cse_ecc_regenerate_nvm_keypair */

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
				 const vuint32_t ec_priv_key_id)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the index of the Seed Key location
	 * P2 is a pointer to multiplier descriptor (byte array and length)
	 * P3 is a pointer to addend descriptor (byte array and length)
	 * P4 is the index of the Key location in which the calculated key will
	 *    be stored
	 */
	ret = cse_hal_send_cmd4p(CSE_ECC_SCALAR_MULT_ADD,
				 (uint32_t)ec_seed_key_id,
				 (uint32_t)pscalar_mult,
				 (uint32_t)pscalar_addend,
				 (uint32_t)ec_priv_key_id);

	return(ret);
} /* cse_ecc_scalar_mult_add */

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
 * @param[in]     ecc_key_idx	ECC Key index
 * @param[in]     *ppub_key	Pointer to a \ref ecc_pub_key_stt
 *				structure for the public key coordinates
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
				  struct ecc_pub_key_stt *ppub_key)
{
	/*
	 * ask the CSE to export the public key from the key internal location
	 * and store it at location described in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the Key index
	 * P2 is the address of location in which public key coordinates will
	 * be exported and Length of the container (in) and length of the
	 * exported key (out)
	 */

	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	ret = cse_hal_send_cmd2p(CSE_ECC_EXPORT_PUBLIC_KEY,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)ppub_key);

	return ret;
} /* End of CSE_ECC_exportPublicKey */

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
				   uint8_t *M4, uint8_t *M5)
{
	/*
	 * ask the CSE to export the public key from the key internal location
	 * and store it at location described in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	/*
	 * Write parameters: i.e. address of location in which M1 to M5 message
	 * will be written:
	 * P1 is the index of the EC private key to be exported (in) and the
	 * address of the produced M1 message (out)
	 * P2 contains the size allocated to the buffer that will receive the
	 * M2 message (in) address and is the address in which the M2 message
	 * will be written (out)
	 * P3 is the address in which M3 message will be written (out)
	 * P4 is the address in which M4 message will be written (out)
	 * P5 is the address in which M5 message will be written (out)
	 */

	/*
	 * Ask the CSE to export all the fields we will need to store the Key
	 * in a non volatile location
	 */
	ret = cse_hal_send_cmd5p(CSE_ECC_EXPORT_PRIVATE_KEY,
				 (uint32_t)M1, (uint32_t)M2, (uint32_t)M3,
				 (uint32_t)M4, (uint32_t)M5);

	return ret;
} /* End of CSE_ECC_exportPrivateKey */

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
			  uint8_t *M5)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the index of the location in which key materials will be stored
	 * P2 is the address of location containing the EC key materials
	 */

	ret = cse_hal_send_cmd5p(CSE_ECC_LOAD_KEY,
				 (uint32_t)M1, (uint32_t)M2, (uint32_t)M3,
				 (uint32_t)M4, (uint32_t)M5);

	return ret;
}   /* End of CSE_ECC_LoadKey */

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
						uint8_t* M3)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd3p(CSE_CONVERT_M2_SIZE_4_RAM_KEY_EXPORT,
				 (uint32_t)M1, (uint32_t)M2, (uint32_t)M3);

	return(ret);
}   /* End of cse_ecc_convert_M2_size_ram_key_export */

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
 * @param[in]     src            pointer to message buffer
 * @param[in]     msg_size       message length in bytes
 * @param[in]     hash_type      descriptor of the hash algorithm to use
 *				 (E_SHA224, E_SHA256...)
 * @param[out]    sign           pointer to destination signature buffer
 *
 * @return        Error code
 * @retval 0      When signature was generated properly
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecdsa_sign(vuint32_t ecc_key_idx,
			    vuint8_t *psrc, vuint32_t msg_size,
			    struct ecdsa_signature_stt *psign,
			    vuint32_t hash_type)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/* P1 is the Key index
	 * P2 is the message buffer address
	 * P3 is the message byte size
	 * P4 is the destination signature address
	 * P5 is the hash algorithm descriptor
	 */

	ret = cse_hal_send_cmd5p(CSE_ECC_ECDSA_SIGN,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)psrc, (uint32_t)msg_size,
				 (uint32_t)psign, (uint32_t)hash_type);

	return ret;
}

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
 * @param[out]    *psuccess      pointer to result word location
 *				 (0 if failed, 1 if succeeded)
 *
 *
 * @return        Error code
 * @retval 0      When signature was verified (but comparison result is in P5)
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t cse_ecc_ecdsa_verify(vuint32_t ecc_key_idx, vuint8_t *psrc,
			      vuint32_t msg_size,
			      struct ecdsa_signature_stt *psign,
			      vuint32_t hash_type, uint32_t *psuccess)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_ECC_ECDSA_VERIFY,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)psrc, (uint32_t)msg_size,
				 (uint32_t)psign, (uint32_t)hash_type);

	*psuccess = CSE->P5.R;
#ifdef VERBOSE
	printf("RETVAL : 0x%08x\n", CSE->P5.R);
#endif /* VERBOSE */

	return ret;
}

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
				   struct ecdsa_signature_stt *psign)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/* P1 is the Key index
	 * P2 is the digest buffer address
	 * P3 is the digest byte size
	 * P4 is the destination signature address
	 */
	ret = cse_hal_send_cmd4p(CSE_ECC_ECDSA_DIGEST_SIGN,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)pdigest, (uint32_t)digest_size,
				 (uint32_t)psign);

	return ret;
}

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
				     uint32_t *psuccess)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd4p(CSE_ECC_ECDSA_DIGEST_VERIFY,
				 (uint32_t)ecc_key_idx,
				 (uint32_t)pdigest, (uint32_t)digest_size,
				 (uint32_t)psign);

	*psuccess = CSE->P5.R;
#ifdef VERBOSE
	printf("RETVAL : 0x%08x\n", CSE->P5.R);
#endif /* VERBOSE */

	return ret;
}

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
uint32_t cse_ecc_change_curve(const vuint32_t ecc_curve_id)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the curve identifier
	 */

	ret = cse_hal_send_cmd1p(CSE_ECC_CHANGE_EC_CURVE,
				 (uint32_t)ecc_curve_id);

	return ret;
} /* CSE_ECC_changeCurve */

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
uint32_t cse_ecc_get_ecc_curve(uint32_t *pecc_curve_id)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the curve identifier
	 */

	ret = cse_hal_send_cmd(CSE_ECC_GET_CURRENT_EC_CURVE);

	*pecc_curve_id = CSE->P1.R;

	return ret;
} /* CSE_ECC_getECcurve */

/**
 * @}
 */
