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
 * @param[in]     P_pPubKeyX    pointer to a \ref ECCpubKeyByteArray_stt
 *                              structure for the public key
 * @param[in]     P_pPrivKey    pointer to a \refECCprivKeyByteArray_stt
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
uint32_t CSE_ECC_LoadRamKey(struct ECCpubKeyByteArray_stt *P_pPubKey,
			    struct ECCprivKeyByteArray_stt *P_pPrivKey)
{
	/* Just ask the CSE to load the Key in the RAM key internal location */
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the address of location containing the EC Public Key x & y
	 * coordinates
	 * P2 is the address of location containing the EC private Key
	 */

	ret = CSE_HAL_send_cmd2p(CSE_ECC_LOAD_RAM_KEY,
				 (uint32_t)P_pPubKey, (uint32_t)P_pPrivKey);

	return ret;
}

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
				     const vuint32_t P_ECflags)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the index of the Key location in which the generated key will
	 * be stored
	 * P2 is the Usage Restriction Flags that must be applied to the key
	 */

	ret = CSE_HAL_send_cmd2p(CSE_ECC_GENERATE_LOAD_KEY_PAIR,
				 (uint32_t)P_ECkeyID,
				 (uint32_t)P_ECflags);

	return ret;
} /* CSE_ECC_changeCurve */

/*============================================================================*/
/**
 * @brief          Exports an Elliptic Curve Public Key
 * @details        Export an Elliptic Curve Public Key
 * @note           Only Public Key coordinates are exported:
 *		   Counter and Flags are not exported
 *
 * @note           Parameter 2 provides length of the container in which key
 *		   is to be exported and will be updated by the length of the
 *		   exported key
 *
 * @param[in]      P_ECC_key_idx      ECC Key index
 * @param[in]      P_pPubKey Pointer to a \ref ECCpubKeyByteArray_stt
 *		   structure for the public key coordinates
 *
 * @return         Error code
 * @retval 0       When key was exported properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_exportPublicKey(vuint32_t P_ECC_key_idx,
				 struct ECCpubKeyByteArray_stt *P_pPubKey)
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
	ret = CSE_HAL_send_cmd2p(CSE_ECC_EXPORT_PUBLIC_KEY,
				 (uint32_t)P_ECC_key_idx,
				 (uint32_t)P_pPubKey);

	return ret;
} /* End of CSE_ECC_exportPublicKey */

/*============================================================================*/
/**
 * @brief          Exports an Elliptic Curve Private Key
 * @details        Export an Elliptic Curve Private Key
 * @note           If it exist the corresponding Public Key is also exported
 *
 * @param[in]      M1    pointer to message1 (out) and index of Key to export
 *			 (in)
 * @param[in]      M2    pointer to message2 (out) and size of memory allocated
 *			 to M2 message
 * @param[in]      M3    pointer to message3
 *
 * @param[out]     M4    pointer to message4
 * @param[out]     M5    pointer to message5
 *
 * @return         Error code
 * @retval 0       When Private Key was exported properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_exportPrivateKey(uint8_t *P_pM1, uint8_t *P_pM2,
				  uint8_t *P_pM3, uint8_t *P_pM4,
				  uint8_t *P_pM5)
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
	ret = CSE_HAL_send_cmd5p(CSE_ECC_EXPORT_PRIVATE_KEY,
				 (uint32_t)P_pM1, (uint32_t)P_pM2,
				 (uint32_t)P_pM3, (uint32_t)P_pM4,
				 (uint32_t)P_pM5);

	return ret;
} /* End of CSE_ECC_exportPrivateKey */

/*============================================================================*/
/**
 * @brief          Loads an EC Key in internal Non Volatile Memory or in RAM
 *		   location
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

 * @api
 *
 * @implements
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_LoadKey(uint8_t *P_pM1, uint8_t *P_pM2, uint8_t *P_pM3,
			 uint8_t *P_pM4, uint8_t *P_pM5)
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

	ret = CSE_HAL_send_cmd5p(CSE_ECC_LOAD_KEY,
				 (uint32_t)P_pM1, (uint32_t)P_pM2,
				 (uint32_t)P_pM3, (uint32_t)P_pM4,
				 (uint32_t)P_pM5);

	return ret;
}   /* End of CSE_ECC_LoadKey */

/*============================================================================*/
/**
 * @brief          Performs an ECC ECDSA Signature Generation
 * @details        Key will be loaded at index described in the already prepared
 *		   messages including key usage restrictions (security flags),
 *		   keyindex, encrypted value and integrity & authenticity MACs
 *		   for all the parameters.
 *
 * @param[in]      P_ECC_key_idx  ECC Key index
 * @param[in]      P_pSrc         pointer to message buffer
 * @param[in]      P_byte_size    message length in bytes
 * @param[in]      P_hash_algo    descriptor of the hash algorithm to use
 *				  (E_SHA224, E_SHA256...)
 * @param[out]     P_pSign        pointer to destination signature buffer
 *
 * @return         Error code
 * @retval 0       When signature was generated properly
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ECDSA_sign(vuint32_t P_ECC_key_idx,
			    vuint8_t *P_pSrc, vuint32_t P_byte_size,
			    struct ECDSA_Signature_stt *P_pSign,
			    vuint32_t P_hash_type)
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

	ret = CSE_HAL_send_cmd5p(CSE_ECC_ECDSA_SIGN,
				 (uint32_t)P_ECC_key_idx,
				 (uint32_t)P_pSrc, (uint32_t)P_byte_size,
				 (uint32_t)P_pSign, (uint32_t)P_hash_type);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Performs an ECC ECDSA Signature Verification
 * @details        Key will be loaded at index described in the already prepared
 *		   messages including key usage restrictions (security flags),
 *		   keyindex, encrypted value and integrity & authenticity MACs
 *		   for all the parameters.
 *
 *
 * @param[in]      P_ECC_key_idx  ECC Key index
 * @param[in]      P_pSrc         pointer to message buffer
 * @param[in]      P_byte_size    message length in bytes
 * @param[in]      P_pSign        pointer to input reference signature buffer
 * @param[in]      P_hash_algo    descriptor of the hash algorithm to use
 *				  (E_SHA224, E_SHA256...)
 * @param[out]     P_pSuccess     pointer to result word location
 *				  (1 if failed, 0 if succeeded)
 *
 *
 * @return         Error code
 * @retval 0       When signature was verified (but comparison result is in P5)
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_ECC_ECDSA_verify(vuint32_t P_ECC_key_idx,
			      vuint8_t *P_pSrc, vuint32_t P_byte_size,
			      struct ECDSA_Signature_stt *P_pSign,
			      vuint32_t P_hash_type, uint32_t *P_pSuccess)
{
	/*
	 * ask the CSE to load the Key in the key internal location described
	 * in the provide messages
	 */
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_ECC_ECDSA_VERIFY,
				 (uint32_t)P_ECC_key_idx,
				 (uint32_t)P_pSrc, (uint32_t)P_byte_size,
				 (uint32_t)P_pSign, (uint32_t)P_hash_type);

	*P_pSuccess = CSE->P5.R;

	return ret;
}

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
uint32_t CSE_ECC_changeCurve(const vuint32_t P_ECcurveID)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the curve identifier
	 */

	ret = CSE_HAL_send_cmd1p(CSE_ECC_CHANGE_EC_CURVE,
				 (uint32_t)P_ECcurveID);

	return ret;
} /* CSE_ECC_changeCurve */

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
uint32_t CSE_ECC_getECcurve(uint32_t *P_pECcurveID)
{
	uint32_t ret = CSE_NO_ERR;

	/* Write parameters */
	/*
	 * P1 is the curve identifier
	 */

	ret = CSE_HAL_send_cmd(CSE_ECC_GET_CURRENT_EC_CURVE);

	*P_pECcurveID = CSE->P1.R;

	return ret;
} /* CSE_ECC_getECcurve */

/**
 * @}
 */
