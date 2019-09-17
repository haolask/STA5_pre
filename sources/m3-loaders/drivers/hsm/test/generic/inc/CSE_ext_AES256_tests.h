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

#ifndef _CSE_EXT_AES256_TEST_H_
#define _CSE_EXT_AES256_TEST_H_

#include "cse_types.h"

#define C_AES256_KEY_SIZE 32
#define C_RESULT_BUFFER_SIZE 40

/**
 * @file    CSE_ext_AES256_test.h
 * @brief   CSE AES256 test header file.
 */

 /* @addtogroup SHE-ext_driver
 * @{
 */

extern uint32_t G_aes256KeyCounter;

/**
* @brief      AES256 encryption decryption tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_ecb_encryptionDecryption_test(uint32_t verbose);

/**
* @brief AES256: Export RAM Key tests
* @details Performs AES256 encryption with RAM after export and re-loading. Check that key has been correctly
*          exported and reloaded. The functions performs the following steps:
*           - Load RAM Key K1 in Cleartext (no protection mechanism, do encryption with K1,
*           - Export RAM key K1 with protection mechanism
*           - Load another RAM key K2 and do encryption with K2
*           - Reload RAM K1 with protected mechanism and do encryption to check the RAM K1 is correctly loaded
* @note
*
* @param[in]    P_verbose
*/

void aes256_exportRamKey_test(uint32_t P_verbose);

/**
* @brief      AES256 encryption decryption with protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_ecb_encryptionDecryption_PK_test(uint32_t P_verbose);


/**
* @brief      AES256 in CBC mode encryption decryption tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_cbc_encryptionDecryption_test(uint32_t P_verbose);

/**
* @brief      AES256 CBC encryption decryption with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_cbc_encryptionDecryption_PK_test(uint32_t P_verbose);

/**
* @brief      AES256 CCM encryption decryption with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_ccm_encryptionDecryption_PK_test(uint32_t P_verbose);

/**
* @brief      AES256 in CCM mode encryption decryption tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_ccm_encryptionDecryption_test(uint32_t P_verbose);

/**
* @brief     AES256 CMAC Tag Generation tests
* @details
* @note
*
* @param[in] P_verbose
*
*/

uint32_t aes256_cmac_tagGenerationVerification_test(uint32_t P_verbose);

/**
* @brief     AES256 CMAC Tag Verification tests
* @details
* @note
*
* @param[in] P_verbose
*
*/

uint32_t aes256_cmac_tagVerification_test(uint32_t P_verbose);

/**
* @brief      AES256 in GCM mode Authenticated Encryption tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_gcm_authenticatedEncryption_test(uint32_t P_verbose);

/**
* @brief      AES256 in GCM mode Authenticated Decryption tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_gcm_authenticatedDecryption_test(uint32_t P_verbose);
/**
 * @}
 */

/**
* @brief      AES256 CMAC Tag Generation / Verification with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/
uint32_t aes256_cmac_tagGenerationVerification_PK_test(uint32_t P_verbose);

/**
* @brief      AES256 CMAC Tag verification with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/
uint32_t aes256_cmac_tagVerification_PK_test(uint32_t P_verbose);

/**
* @brief      AES256 GCM authenticated encryption with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/
uint32_t aes256_gcm_authenticatedEncryption_PK_test(uint32_t P_verbose);

/**
* @brief      AES256 GCM authenticated decryption with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/
uint32_t aes256_gcm_authenticatedDecryption_PK_test(uint32_t P_verbose);

/**
 * @brief          Test the load key function for AES256 NVM keys
 * @details        Load keys in the NVM locations in a virgin chip ( or at least a chip whose user keys are empty)
 *
 * @param[in]      verbose        enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0        When test failed
 * @retval 1         When test succeeded
 *
 * @note              The expected message are only valid for a particular chip since it involves its unique ID
 */
uint32_t CSE_NVM_AES256_key_update_interactive_test(int verbose);
#endif /* _CSE_EXT_AES256_TEST_H_ */
