/*
SPC5-CRYPTO - Copyright (C) 2017 STMicroelectronics

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

#ifndef _CSE_EXT_HMAC_TEST_H_
#define _CSE_EXT_HMAC_TEST_H_

#define C_AES256_KEY_SIZE 32
#define C_RESULT_BUFFER_SIZE 40

/**
* @file    CSE_ext_HMAC_tests.h
* @brief   CSE HMAC test header file.
*/

/* @addtogroup SHE-ext_driver
* @{
*/
#include "CSE_ext_hash.h"
#include "CSE_ext_HMAC.h"

extern uint32_t G_hmacKeyCounter;

/*******************************************************************/
/*                    TEST FUNCTIONS WITH RAM KEY                  */
/*******************************************************************/

/**
* @brief     HMAC MAC Generation tests with RAM key
* @details
* @note
*
* @param[in] P_verbose
*
*/
uint32_t hmac_macGeneration_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb );

/*@brief     HMAC MAC Verification tests with RAM key
* @details
* @note
*
* @param[in] P_verbose
*
*/
uint32_t hmac_macVerification_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb );

/*******************************************************************/
/*                TEST FUNCTIONS WITH PROTECTED KEYS               */
/*******************************************************************/
/**
* @brief     HMAC MAC Generation tests with Protected key
* @details
* @note
*
* @param[in] P_verbose
*
*/
uint32_t hmac_macGeneration_pk_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb );

/**
* @brief     HMAC MAC Verification tests with Protected key
* @details
* @note
*
* @param[in] P_verbose
*
*/
uint32_t hmac_macVerification_pk_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb );


/**
* @brief  HMAC Mac Generation
*
* @param[in]  *P_pSecretKey Pointer to \ref HMACKeyByteArray_stt structure for HMAC RAM secret Key
* @param[in]  *P_pMsg       Pointer to \ref ByteArrayDescriptor_stt Message on which Mac is generated
* @param[out] *P_pMac       Pointer to \ref ByteArrayDescriptor_stt generated Mac
* @param[in]   P_hashAlgo   Identifier of hash algorithm
* @param[in]   P_verbose    To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t hmac_macGeneration(struct HMACKeyByteArray_stt * P_pSecretKey,
                            struct ByteArrayDescriptor_stt * P_pMsg,
                            struct ByteArrayDescriptor_stt * P_pMac,
                            enum hashType_et P_hashAlgo,
                            uint32_t P_verbose);
#endif /* _CSE_EXT_HMAC_TEST_H_ */
/**
* @}
*/
