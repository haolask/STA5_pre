/*
    SPC5-CRYPTO - Copyright (C) 2015 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    CSE_ext_ECC_ECDSA_test.h
 * @brief   ECC ECDSA tests header file
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#ifndef _CSE_EXT_ECC_ECDSA_TEST_H_
#define _CSE_EXT_ECC_ECDSA_TEST_H_

#include "cse_types.h"
#include "CSE_ext_ECC_TV_consts.h"
#include "CSE_ext_ECC.h"

extern uint32_t ecc_ecdsa_sha_signVerify_test( uint32_t verbose, uint32_t reduced_test_vector_nb );
extern uint32_t ecc_ecdsa_sha_signatureVerification_test( uint32_t verbose, uint32_t reduced_test_vector_nb );
extern uint32_t ecc_ecdsa_sha_signVerify_PK_test(uint32_t verbose, uint32_t reduced_test_vector_nb);
extern uint32_t ecc_ecdsa_sha_signVerify_PK_test(uint32_t verbose, uint32_t reduced_test_vector_nb);
extern uint32_t ecc_ecdsa_sha_signatureVerification_PK_test(uint32_t verbose, uint32_t reduced_test_vector_nb );

/**
  * @brief  ECC ECDSA Signature Generation
  * @param  P_pPrivKey The ECC private key structure, already initialized
  * @param  P_pInputMessage Input Message to be signed
  * @param  P_MessageSize Size of input message
  * @param  hash_type type of hash to use (E_SHA1, E_SHA224, E_SHA256 ...)
  * @param  P_pOutput Pointer to output buffer
  * @param  verbose or not
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/
int32_t ECC_signGenerate_ECDSA(struct ECCprivKeyByteArray_stt * P_pPrivKey,
                               const uint8_t * P_pInputMessage, int32_t P_MessageSize,
                               uint32_t hash_type,
                               struct ECDSA_Signature_stt * P_pECDSA_Signature,
                               uint32_t verbose);

/**
  * @brief  ECC ECDSA Signature Verification
  * @param  P_pPrivKey The ECC public key structure, already initialized
  * @param  P_pInputMessage Input Message to be signed
  * @param  P_MessageSize Size of input message
  * @param  hash_type type of hash to use (E_SHA1, E_SHA224, E_SHA256 ...)
  * @param  P_pOutput Pointer to output buffer
  * @param verbose or not
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
  * */
int32_t ECC_signVerify_ECDSA(struct ECCpubKeyByteArray_stt * P_pPubKey,
                             const uint8_t * P_pInputMessage, int32_t P_MessageSize,
                             uint32_t hash_type,
                             struct ECDSA_Signature_stt * P_pECDSA_Signature,
                             uint32_t verbose);

/**
  * @brief  ECC key pair Generation and store it in specified key location
  * @param  P_eccKeyIndex the index of the key location in which the generated key pair will be stored
  * @param  P_ECflags Key Flag to be applied to generated key
  * @param  P_verbose verbose or not
  *
  * @retval error status: can be RSA_SUCCESS if success or one of  *
  * RSA_ERR_BAD_PARAMETER, RSA_ERR_UNSUPPORTED_HASH, RSA_ERR_BAD_KEY, ERR_MEMORY_TEST_FAIL
  * RSA_ERR_MODULUS_TOO_SHORT
*/
int32_t ECC_generateLoadKeyPair(uint32_t P_eccKeyIndex, union EXTENDED_KEY_flags P_ECflags, uint32_t P_verbose);

#endif /* _CSE_EXT_ECC_ECDSA_TEST_H_ */
/**
 * @}
 */
