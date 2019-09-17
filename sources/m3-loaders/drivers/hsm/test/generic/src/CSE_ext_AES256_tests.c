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

/**
 * @file    CSE_ext_AES256_tests.c
 * @brief   AES256 encryption / decryption tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include <string.h>        /* For memcmp */

#include "config.h"
#include "serialprintf.h"
#include "serial_input.h"

#include "cse_typedefs.h"
#include "CSE_Manager.h"
#include "CSE_extendKey_updateSupport.h"

#include "CSE_ext_AES256.h"
#include "CSE_ext_AES256_tests.h"

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif

#include "CSE_ext_AES256_CCM_VADT_TV.h"
#include "CSE_ext_AES256_CCM_DVPT_TV.h"
#include "CSE_ext_AES256_CMAC_cmac_TV.h"
#include "CSE_ext_AES256_GCM.h"

#include "cse_client.h"
#include "test_values.h"

//#define DEBUG_PRINT

extern const aes256ccm_vadt_test_vect_stt aes256ccm_vadt_test_vect [C_NB_TEST_CCM_VADT];
extern const aes256ccm_dvpt_test_vect_stt aes256ccm_dvpt_test_vect [C_NB_TEST_CCM_DVPT_TVS];
extern const aes256cmac_testVectSerie_stt aes256cmac_test_vect [C_NB_TEST_CMAC_TVS];
extern const aes256cmac_verifyTestVectSerie_stt aes256cmac_verifyTest[C_NB_TEST_CMAC_VERIFY_TVS];
extern const aes256Gcm_encrypt_testVect_stt aes256Gcm_encrypt_testVect_array[C_NB_OF_AES_GCM_ENCRPT_TV];
extern const aes256Gcm_decrypt_testVect_stt aes256Gcm_decrypt_testVect_array[C_NB_OF_AES_GCM_DECRPT_TV];

uint32_t G_aes256KeyCounter = 2U;

/* AES256 Test vectors --------------------------------------------------------*/
    /* AES256 ECB test vector coming from NIST CAVS 11.1 AESVS KeySbox test data for ECB ECBKeySbox256.rsp
     * Used also for CBC with IV = 0 */
ALIGN static const unsigned char key_tv1[C_AES256_KEY_SIZE] =
    {
        0xc4, 0x7b, 0x02, 0x94, 0xdb, 0xbb, 0xee, 0x0f, 0xec, 0x47, 0x57, 0xf2, 0x2f, 0xfe, 0xee, 0x35,
        0x87, 0xca, 0x47, 0x30, 0xc3, 0xd3, 0x3b, 0x69, 0x1d, 0xf3, 0x8b, 0xab, 0x07, 0x6b, 0xc5, 0x58
    };

ALIGN static const unsigned char plainText_tv1[16] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

ALIGN static const unsigned char iV_tv1[16] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

ALIGN static const unsigned char cipherText_tv1[16] =
    {
        0x46, 0xf2, 0xfb, 0x34, 0x2d, 0x6f, 0x0a, 0xb4, 0x77, 0x47, 0x6f, 0xc5, 0x01, 0x24, 0x2c, 0x5f
    };

ALIGN static const unsigned char key_tv2[C_AES256_KEY_SIZE] =
    {
            0x28, 0xd4, 0x6c, 0xff, 0xa1, 0x58, 0x53, 0x31, 0x94, 0x21, 0x4a, 0x91, 0xe7, 0x12, 0xfc, 0x2b,
            0x45, 0xb5, 0x18, 0x07, 0x66, 0x75, 0xaf, 0xfd, 0x91, 0x0e, 0xde, 0xca, 0x5f, 0x41, 0xac, 0x64
    };
#if 0
ALIGN static const unsigned char cipherText_tv2[16] =
    {
            0x4b, 0xf3, 0xb0, 0xa6, 0x9a, 0xeb, 0x66, 0x57, 0x79, 0x4f, 0x29, 0x01, 0xb1, 0x44, 0x0a, 0xd4
    };
#endif
#define C_RESULT_BUFFER_SIZE 40

/* Exported functions ---------------------------------------------------------*/

/**
 * @}
 */

/**
* @brief  AES256 ECB Encryption
*
* @param    *P_pSecretKeyrivKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param    *P_pMessageToEncrypt    Pointer to \ref ByteArrayDescriptor_stt describing the message to encrypt
* @param    *P_pEncryptedMessage    Pointer to \ref ByteArrayDescriptor_stt: encrypted Message
* @param     blockCount             Number of AES 16-byte block of the input message to cipher
* @param     P_verbose              To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_ecb_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                              struct ByteArrayDescriptor_stt *P_pEncryptedMessage,
                              uint32_t blockCount,
                              uint32_t P_verbose)
{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Encryption in ECB mode */
        ret = CSE_AES256_ECB_encrypt(AES256_RAM_KEY_IDX,
                                     (vuint8_t*)P_pMessageToEncrypt->address,
                                     (vuint8_t*)P_pEncryptedMessage->address, blockCount);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_ecb_encryption */

/**
* @brief  AES256 ECB Decryption
*
* @param    *P_pSecretKeyrivKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param    *P_pMessageToDecrypt    Pointer to \ref ByteArrayDescriptor_stt describing the message to decrypt
* @param    *P_pDecryptedMessage    Pointer to \ref ByteArrayDescriptor_stt: cleartext Message
* @param     blockCount             Number of AES 16-byte block of the input message to decipher
* @param     P_verbose              To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_ecb_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              struct ByteArrayDescriptor_stt * P_pMessageToDecrypt,
                              struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                              uint32_t blockCount,
                              uint32_t P_verbose)
{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Decryption in ECB mode */
        ret = CSE_AES256_ECB_decrypt(AES256_RAM_KEY_IDX,
                                     (vuint8_t*)P_pMessageToDecrypt->address,
                                     (vuint8_t*)P_pDecryptedMessage->address, blockCount);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_ecb_encryption */

/**
* @brief  AES256 CBC Encryption
*
* @param    *P_pSecretKeyrivKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param    *P_pMessageToEncrypt    Pointer to \ref ByteArrayDescriptor_stt describing the message to encrypt
* @param    *P_pEncryptedMessage    Pointer to \ref ByteArrayDescriptor_stt: encrypted Message
* @param    *P_pIv                  Pointer to \ref ByteArrayDescriptor_stt Initialization Vector
* @param     blockCount             Number of AES 16-byte block of the input message to cipher
* @param     P_verbose              To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_cbc_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                              struct ByteArrayDescriptor_stt *P_pEncryptedMessage,
                              struct ByteArrayDescriptor_stt *P_pIv,
                              uint32_t blockCount,
                              uint32_t P_verbose)
{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Encryption in CBC mode */
        ret = CSE_AES256_CBC_encrypt(AES256_RAM_KEY_IDX,
                                     (vuint8_t*)P_pMessageToEncrypt->address,
                                     (vuint8_t*)P_pEncryptedMessage->address,
                                     (vuint8_t*)P_pIv->address,
                                     blockCount);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_cbc_encryption */

/**
* @brief  AES256 CBC Decryption
*
* @param    *P_pSecretKeyrivKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param    *P_pMessageToDecrypt    Pointer to \ref ByteArrayDescriptor_stt describing the message to decrypt
* @param    *P_pDecryptedMessage    Pointer to \ref ByteArrayDescriptor_stt: cleartext Message
* @param    *P_pIv                  Pointer to \ref ByteArrayDescriptor_stt Initialization Vector
* @param     blockCount             Number of AES 16-byte block of the input message to decipher
* @param     P_verbose              To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_cbc_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              struct ByteArrayDescriptor_stt * P_pMessageToDecrypt,
                              struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                              struct ByteArrayDescriptor_stt *P_pIv,
                              uint32_t blockCount,
                              uint32_t P_verbose)
{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Decryption in ECB mode */
        ret = CSE_AES256_CBC_decrypt(AES256_RAM_KEY_IDX,
                                     (vuint8_t*)P_pMessageToDecrypt->address,
                                     (vuint8_t*)P_pDecryptedMessage->address,
                                     (vuint8_t*)P_pIv->address,
                                     blockCount);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_cbc_decryption */

/**
* @brief  AES256 CCM Encryption
*
* @param[in]  *P_pSecretKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param[in]  *p_pHeaderData    Pointer to \ref AES_CCM_headerData_stt structure describing Nonce and Associated Data
* @param[in]   P_tagSize        Size of Tag used to authenticate the message
* @param[in]  *P_pMsg           Pointer to \ref AES_CCM_msgData_stt: Message to encrypt
* @param[in]  *P_pCipheredMsg   Pointer to \ref AES_CCM_msgData_stt Encrypted message
* @param[in]   P_verbose        To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_ccm_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              const struct AES_CCM_headerData_stt * p_pHeaderData,
                              const uint32_t P_tagSize,
                              const struct AES_CCM_msgData_stt * P_pMsg,
                              struct AES_CCM_msgData_stt * P_pCipheredMsg,
                              uint32_t P_verbose)

{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Encryption in CCM mode */
        ret = CSE_AES256_CCM_encrypt(AES256_RAM_KEY_IDX,
                                     p_pHeaderData,
                                     P_tagSize,
                                     P_pMsg,
                                     P_pCipheredMsg);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_ccm_encryption */

/**
* @brief  AES256 CCM Decryption
*
* @param[in]  *P_pSecretKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param[in]  *p_pHeaderData    Pointer to \ref AES_CCM_headerData_stt structure describing Nonce and Associated Data
* @param[in]   P_tagSize        Size of Tag used to authenticate the message
* @param[in]  *P_pCipheredMsg   Pointer to \ref AES_CCM_msgData_stt structure containing the Message to decrypt
* @param[in]  *P_pPayloadMsg    Pointer to \ref AES_CCM_deciphMsgData_stt structure receiving the decrypted message, i.e. the payload message
*                                 and the verification status
* @param[in]   P_verbose        To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_ccm_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              const struct AES_CCM_headerData_stt * p_pHeaderData,
                              const uint32_t P_tagSize,
                              const struct AES_CCM_msgData_stt * P_pCipheredMsg,
                              struct AES_CCM_deciphMsgData_stt * P_pPayloadMsg,
                              uint32_t P_verbose)

{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Decryption in CCM mode */
        ret = CSE_AES256_CCM_decrypt(AES256_RAM_KEY_IDX,
                                     p_pHeaderData,
                                     P_tagSize,
                                     P_pCipheredMsg,
                                     P_pPayloadMsg);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_ccm_decryption */

/**
* @brief  AES256 CMAC Tag Generation
*
* @param[in]  *P_pSecretKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param[in]   P_tagSize        Size of Tag used to authenticate the message
* @param[in]  *P_pMsg           Pointer to \ref AES_CMAC_msg_stt: Message on which tag is generated
* @param[in]  *P_pCmacTag       Pointer to \ref AES_CMAC_tag_stt CMAC Tag
* @param[in]   P_verbose        To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_cmac_tagGeneration(struct AES256secretKeyArray_stt * P_pSecretKey,
                                  const uint32_t P_tagSize,
                                  const struct AES_CMAC_msg_stt * P_pMsg,
                                  struct AES_CMAC_tag_stt * P_pCmacTag,
                                  uint32_t P_verbose)
{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Encryption in CMAC mode */
        ret = CSE_AES256_CMAC_tagGeneration(AES256_RAM_KEY_IDX,
                                            P_tagSize,
                                            P_pMsg,
                                            P_pCmacTag);

#ifdef PERF_MEASURMENT
            PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_cmac_tagGeneration */

/**
* @brief  AES256 CMAC Tag Verification
*
* @param[in]  *P_pSecretKey Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param[in]   P_tagSize    Requested Size of Tag to be generated
* @param[in]  *P_pMsg       Pointer to \ref AES_CMAC_msg_stt: Message on which tag is verified
* @param[in]  *P_pCmacTag   Pointer to \ref AES_CMAC_tag_stt CMAC reference Tag
* @param[in]   P_verbose    To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_cmac_tagVerification(struct AES256secretKeyArray_stt * P_pSecretKey,
                                    const uint32_t P_tagSize,
                                    const struct AES_CMAC_msg_stt * P_pMsg,
                                    const struct AES_CMAC_tag_stt * P_pCmacTag,
                                    uint32_t P_verbose)
{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;
    uint32_t success = 0U;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 Encryption in CMAC mode */
        ret = CSE_AES256_CMAC_tagVerification(AES256_RAM_KEY_IDX,
                                              P_tagSize,
                                              P_pMsg,
                                              P_pCmacTag,
                                              &success);

#ifdef PERF_MEASURMENT
            PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
            if( success == 0 )
            {
                status = AUTHENTICATION_SUCCESSFUL;
            }
            else
            {
                status = AUTHENTICATION_FAILED;
            }
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_cmac_tagVerification */

/**
* @brief  AES256 GCM Authenticated Encryption
*
* @param[in]  *P_pSecretKey     Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param[in]  *p_pHeaderData    Pointer to \ref AES_CCM_headerData_stt structure describing Nonce and Associated Data
* @param[in]   P_tagSize        Size of Tag used to authenticate the message
* @param[in]  *P_pMsg           Pointer to \ref AES_CCM_msgData_stt: Message to encrypt
* @param[in]  *P_pAuthCipheredMsg Pointer to \ref AES_GCM_authCiphMsg_stt GCM Authenticated Encrypted message
* @param[in]   P_verbose        To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_gcm_authenticatedEncryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                                           const struct AES_GCM_headerData_stt * p_pHeaderData,
                                           const uint32_t P_tagSize,
                                           const struct AES_GCM_msgData_stt * P_pMsg,
                                           struct AES_GCM_authCiphMsg_stt * P_pAuthCipheredMsg,
                                           uint32_t P_verbose)

{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 GCM Authenticated Encryption */
        ret = CSE_AES256_GCM_encrypt(AES256_RAM_KEY_IDX,
                                     p_pHeaderData,
                                     P_tagSize,
                                     P_pMsg,
                                     P_pAuthCipheredMsg);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_gcm_authenticatedEncryption */

/**
* @brief  AES256 GCM Authenticated Decryption
*
* @param[in]  *P_pSecretKey         Pointer to \ref AES256secretKeyArray_stt structure for AES256 secret Key
* @param[in]  *p_pHeaderData        Pointer to \ref AES_CCM_headerData_stt structure describing Nonce and Associated Data
* @param[in]   P_tagSize            Size of Tag used to authenticate the message
* @param[in]  *P_pAuthCipheredMsg    Pointer to \ref AES_GCM_authCiphMsg_stt: Message to decrypt with the tag to verify
* @param[in]  *P_pDeCipheredMsg     Pointer to \ref AES_GCM_deciphMsgData_stt GCM Authenticated Decrypted message
* @param[in]   P_verbose            To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t aes256_gcm_authenticatedDecryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                                           const struct AES_GCM_headerData_stt * p_pHeaderData,
                                           const uint32_t P_tagSize,
                                           const struct AES_GCM_authCiphMsg_stt * P_pAuthCipheredMsg,
                                           struct AES_GCM_deciphMsgData_stt * P_pDeCipheredMsg,
                                           uint32_t P_verbose)

{
    struct AES256secretKeyArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mSecretKeySize =  P_pSecretKey->mSecretKeySize;
    secretKey.pmSecretKey = P_pSecretKey->pmSecretKey;

    ret = CSE_AES256_loadRamKey(&secretKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch AES256 GCM Authenticates Decryption */
        ret = CSE_AES256_GCM_decrypt(AES256_RAM_KEY_IDX,
                                     p_pHeaderData,
                                     P_tagSize,
                                     P_pAuthCipheredMsg,
                                     P_pDeCipheredMsg);

#ifdef PERF_MEASURMENT
    PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of aes256_gcm_authenticatedDecryption */

uint32_t aes256_ecb_encryptionDecryption_test(uint32_t P_verbose)
{
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt deCipherMessage;

    uint8_t encryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Decrypted Message  */

    struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    int32_t check;
    uint32_t failed = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 Encryption Decryption in ECB tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.1 !\n");
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.mSecretKeySize = 32;

    messageToEncrypt.address = (uint8_t*)plainText_tv1;
    messageToEncrypt.byteSize = 16;

    cipherMessage.address = encryptedMessageBuf;
    cipherMessage.byteSize = 16;

    deCipherMessage.address = decryptedMessageBuf;

    /* AES256 Encryption */
    status = aes256_ecb_encryption(&aes256SecretKey, &messageToEncrypt,
                                   &cipherMessage, 1, P_verbose);

    if(P_verbose)
    {
        printf(".");
    }
    if (CSE_NO_ERR == status)
    {
        /* Compare Expected ClearText from test vector with computed one */

        check = memcmp(encryptedMessageBuf, (uint8_t*)cipherText_tv1, 16);
        if(P_verbose)
        {
            printf("\nECB Encryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }
    failed += ((status != CSE_NO_ERR) || (check != 0));

    /* AES256 Decryption */
    status = aes256_ecb_decryption(&aes256SecretKey, &cipherMessage,
                                   &deCipherMessage, 1, P_verbose);

    if(P_verbose)
    {
        printf(".");
    }
    if (CSE_NO_ERR == status)
    {
        /* Compare Expected ClearText from test vector with computed one */

        check = memcmp(decryptedMessageBuf, (uint8_t*)plainText_tv1, 16);
        if(P_verbose)
        {
            printf("\nECB Decryption test: Clear Text check: %d, (expected: 1)\n", check == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    failed += ((status != CSE_NO_ERR) || (check != 0));

/* return 1 if successful (no test failed) */
return (failed == 0);

} /* End of aes256_ecb_encryptionDecryption_test */

/**
* @brief      AES256 in CBC mode encryption decryption tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_cbc_encryptionDecryption_test(uint32_t P_verbose)
{
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt deCipherMessage;
    struct ByteArrayDescriptor_stt aesIv;

    uint8_t encryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Decrypted Message  */

    struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    int32_t check;
    uint32_t failed = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 Encryption Decryption in CBC tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.1 !\n");
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.mSecretKeySize = 32;

    /* Initialize Initialization Vector */
    aesIv.address = iV_tv1;
    aesIv.byteSize = 16;

    messageToEncrypt.address = (uint8_t*)plainText_tv1;
    messageToEncrypt.byteSize = 16;

    cipherMessage.address = encryptedMessageBuf;
    cipherMessage.byteSize = 16;

    deCipherMessage.address = decryptedMessageBuf;

    /* AES256 CBC Encryption */
    status = aes256_cbc_encryption(&aes256SecretKey, &messageToEncrypt,
                                   &cipherMessage, &aesIv, 1, P_verbose);

    if(P_verbose)
    {
        printf(".");
    }
    if (CSE_NO_ERR == status)
    {
        /* Compare Expected ClearText from test vector with computed one */

        check = memcmp(encryptedMessageBuf, (uint8_t*)cipherText_tv1, 16);
        if(P_verbose)
        {
            printf("\nCBC Encryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }
    failed += ((status != CSE_NO_ERR) || (check != 0));

    /* AES256 CBC Decryption */
    status = aes256_cbc_decryption(&aes256SecretKey, &cipherMessage,
                                   &deCipherMessage, &aesIv, 1, P_verbose);

    if (CSE_NO_ERR == status)
    {
        /* Compare Expected ClearText from test vector with computed one */

        check = memcmp(decryptedMessageBuf, (uint8_t*)plainText_tv1, 16);
        if(P_verbose)
        {
            printf("\nCBC Decryption test: Clear Text check: %d, (expected: 1)\n", check == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    failed += ((status != CSE_NO_ERR) || (check != 0));

/* return 1 if successful (no test failed) */
return (failed == 0);

} /* End of aes256_cbc_encryptionDecryption_test */

/**
* @brief      AES256 in CCM mode encryption decryption tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_ccm_encryptionDecryption_test(uint32_t P_verbose)
{

    struct AES_CCM_msgData_stt msg;                    /* Message to cipher */
    struct AES_CCM_headerData_stt headerData;              /* Header data, Nonce and associated data */
    struct AES_CCM_msgData_stt cipheredMsg;            /* Location for the ciphered message */
    struct AES_CCM_deciphMsgData_stt payloadMsg;   /* Location for the deciphered message, the payload */

    uint8_t encryptedMessageBuf[C_RESULT_BUFFER_SIZE] = {0};          /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[C_RESULT_BUFFER_SIZE] = {0};          /* Buffer that will receive the AES256 Decrypted Message  */

    struct AES256secretKeyArray_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    int32_t check = 1U;        /* Set to error */
    uint32_t failed = 0U;

    uint32_t tv = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 CCM mode Encryption Decryption tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.0 !\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CCM_VADT; tv++)
    {
        uint32_t i = 0U;

        /* Initialize secret key */
        aes256SecretKey.pmSecretKey = (uint8_t*)aes256ccm_vadt_test_vect[tv].aes256ccm_Key_Alen;
        aes256SecretKey.mSecretKeySize = 32;

        /* Initialize Nonce */
        headerData.pNonce = (unsigned char *)aes256ccm_vadt_test_vect[tv].aes256ccm_Nonce_Alen;
        headerData.nonceByteSize = C_NLEN_VADT_SIZE;

        if (P_verbose)
        {
            printf("\nSize of Associated Data = %d\n", aes256ccm_vadt_test_vect[tv].AlenSize);
        }
        for (i = 0U; i < C_NB_TEST_CCM; i++)
        {
            /* Erase result buffers */
            uint32_t j = 0U;
            for (j = 0; j < C_RESULT_BUFFER_SIZE; j++)
            {
                encryptedMessageBuf[j] = 0U;
                decryptedMessageBuf[j] = 0U;
            }

            /* Initialize Associated Data */
            headerData.pData = (unsigned char *)aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pAdata;
            headerData.DataByteSize = aes256ccm_vadt_test_vect[tv].AlenSize;

            msg.pAddress = (unsigned char *)aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pPayload;
            msg.messageByteSize = C_PLEN_VADT_SIZE;

            cipheredMsg.pAddress = encryptedMessageBuf;
            cipheredMsg.messageByteSize = C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE;    /* Expected size of cipher Text, size allocated to result buffer */

            payloadMsg.pDecipheredMessage = decryptedMessageBuf;
            payloadMsg.decipheredmessageByteSize = C_PLEN_VADT_SIZE;
            payloadMsg.authResult = AUTHENTICATION_FAILED;

            /* AES256 CCM Encryption */
            status = aes256_ccm_encryption(&aes256SecretKey, &headerData, C_TLEN_VADT_SIZE, &msg,
                                           &cipheredMsg, P_verbose);

            if (CSE_NO_ERR == status)
            {
                /* Compare Expected CipherText from test vector with computed one */

                check = memcmp(cipheredMsg.pAddress, (uint8_t*)aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pCT, C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE);
                if(P_verbose)
                {
                    printf("\nCCM Encryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));

            /* AES256 CCM Decryption */
            status = aes256_ccm_decryption(&aes256SecretKey, &headerData, C_TLEN_VADT_SIZE, &cipheredMsg,
                                           &payloadMsg, P_verbose);

            if (CSE_NO_ERR == status)
            {
                /* Compare Expected ClearText from test vector with computed one */

                check = memcmp(payloadMsg.pDecipheredMessage, (uint8_t*)(aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pPayload), C_PLEN_VADT_SIZE);
                if(P_verbose)
                {
                    printf("\nCCM Decryption test: Clear Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
        }
        failed += ((status != CSE_NO_ERR) || (check != 0));
    }


    if(P_verbose)
    {
        printf("\n");
        printf("AES256 CCM mode Decryption tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.0 DVPT test suite!\n");
        printf("Most of verification tests must fail, Authentication verification results are provided in test vectors !\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CCM_DVPT_TVS; tv++)
    {
        uint32_t i = 0U;

        /* Initialize secret key */
        aes256SecretKey.pmSecretKey = (uint8_t*)aes256ccm_dvpt_test_vect[tv].aes256ccm_Key_Alen;
        aes256SecretKey.mSecretKeySize = 32;

        if (P_verbose)
        {
            printf("\nSize of Associated Data = %d\n", aes256ccm_dvpt_test_vect[tv].AlenSize);
            printf("Size of Payload Data = %d\n", aes256ccm_dvpt_test_vect[tv].PlenSize);
            printf("Size of Nonce = %d\n", aes256ccm_dvpt_test_vect[tv].NlenSize);
            printf("Size of Tag = %d\n\n", aes256ccm_dvpt_test_vect[tv].TlenSize);
        }
        for (i = 0U; i < C_NB_TEST_CCM_DVPT_TV; i++)
        {
            /* Erase result buffers */
            uint32_t j = 0U;
            for (j = 0; j < C_RESULT_BUFFER_SIZE; j++)
            {
                decryptedMessageBuf[j] = 0U;
            }

            /* Initialize Nonce */
            headerData.pNonce = (unsigned char *)aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pNonce;
            headerData.nonceByteSize = aes256ccm_dvpt_test_vect[tv].NlenSize;

            /* Initialize Associated Data */
            headerData.pData = (unsigned char *)aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pAdata;
            headerData.DataByteSize = aes256ccm_dvpt_test_vect[tv].AlenSize;

            cipheredMsg.pAddress = (unsigned char *)aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pCT;
            cipheredMsg.messageByteSize = aes256ccm_dvpt_test_vect[tv].PlenSize + aes256ccm_dvpt_test_vect[tv].TlenSize;    /* Expected size of cipher Text, size allocated to result buffer */

            payloadMsg.pDecipheredMessage = decryptedMessageBuf;
            payloadMsg.decipheredmessageByteSize = aes256ccm_dvpt_test_vect[tv].PlenSize;
            payloadMsg.authResult = AUTHENTICATION_FAILED;

            /* AES256 CCM Decryption */
            status = aes256_ccm_decryption(&aes256SecretKey, &headerData, aes256ccm_dvpt_test_vect[tv].TlenSize, &cipheredMsg,
                                           &payloadMsg, P_verbose);
            /* Set error flag */
            failed += (CSE_NO_ERR != status);

            if (CSE_NO_ERR == status)
            {
                /* Check if authentication of message is verified and expected authentication result is the same */
                if ((0x00U == payloadMsg.authResult) && (C_TEST_CCM_DVPT_PASS == *aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pResult))
                {
                    /* Compare Expected ClearText from test vector with expected one */
                    check = memcmp(payloadMsg.pDecipheredMessage, (uint8_t*)(aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pPayload), aes256ccm_dvpt_test_vect[tv].PlenSize);
                    if(P_verbose)
                    {
                        printf("\nCCM Decryption test: Authentication Verified, Clear Text check: %d, (expected: 1)\n", check == 0);
                    }
                }
                else
                {
                    /* Verification is not correct, check expected result */
                    if ((0x01U == payloadMsg.authResult) && (C_TEST_CCM_DVPT_FAIL == *aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pResult))
                    {
                        check = 0;
                        check = !((0x01U == payloadMsg.authResult) && (C_TEST_CCM_DVPT_FAIL == *aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pResult));
                        if(P_verbose)
                        {
                            printf("\nCCM Decryption test: Authentication not verified: Authentication check: %d, (expected: 1)\n", check == !((0x01U == payloadMsg.authResult) && (C_TEST_CCM_DVPT_FAIL == *aes256ccm_dvpt_test_vect[tv].testCcmDvpt[i].pResult)));
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf("\nCCM decryption Failed.\n");
                        }
                        failed++;
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }

        failed += ((status != CSE_NO_ERR) || (check != 0));

        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_ccm_encryptionDecryption_test */


/**
* @brief     AES256 CMAC Tag Generation tests
* @details
* @note
*
* @param[in] P_verbose
*
*/

uint32_t aes256_cmac_tagGenerationVerification_test(uint32_t P_verbose)
{

    struct AES_CMAC_msg_stt msg;           /* Message on which CMAC tag is generated */
    struct AES_CMAC_tag_stt cmacTag;           /* CMAC Tag */

    uint8_t cmacTagBuf[40] = {0};           /* Buffer that receive the generated CMAC Tag */

    struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    int32_t check = 1U;        /* Set to error */
    uint32_t failed = 0U;

    uint32_t tv = 0U;

    if(P_verbose)
    {
        /* Test vector coming from NIST CAVP 11.0 cmactestvectors.zip */
        printf("\n");
        printf("AES256 CMAC Tag Generation and Verification tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.0 !\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CMAC_TVS; tv++)
    {
        uint32_t i = 0U;
        if (P_verbose)
        {
            printf("\nSize of Message = %d\n", aes256cmac_test_vect[tv].msgSize);
            printf("Size of requested CMAC Tag = %d\n", aes256cmac_test_vect[tv].tagSize);
        }
        for (i = 0U; i < C_NB_TEST_CMAC_TV; i++)
        {
            /* Initialize secret key */
            aes256SecretKey.pmSecretKey = (uint8_t*)aes256cmac_test_vect[tv].testCmac[i].pKey;
            aes256SecretKey.mSecretKeySize = 32;

            msg.pAddress = (unsigned char *)aes256cmac_test_vect[tv].testCmac[i].pMsg;
            msg.messageByteSize = aes256cmac_test_vect[tv].msgSize;

            cmacTag.pAddress = cmacTagBuf;

            /* AES256 CMAC Tag Generation */
            status = aes256_cmac_tagGeneration(&aes256SecretKey, aes256cmac_test_vect[tv].tagSize, &msg, &cmacTag, P_verbose);

            if (CSE_NO_ERR == status)
            {
                /* Compare Expected CipherText from test vector with computed one */
                check = memcmp(cmacTag.pAddress, (uint8_t*)aes256cmac_test_vect[tv].testCmac[i].pCmacTag, aes256cmac_test_vect[tv].tagSize);
                if(P_verbose)
                {
                    printf("\nCMAC Tag Generation test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));

            /* Test CMAC Tag verification on same test vectors */
            /* Initialize secret key */
            aes256SecretKey.pmSecretKey = (uint8_t*)aes256cmac_test_vect[tv].testCmac[i].pKey;
            aes256SecretKey.mSecretKeySize = 32;

            msg.pAddress = (unsigned char *)aes256cmac_test_vect[tv].testCmac[i].pMsg;
            msg.messageByteSize = aes256cmac_test_vect[tv].msgSize;

            cmacTag.pAddress = (uint8_t *)aes256cmac_test_vect[tv].testCmac[i].pCmacTag;

            /* AES256 CMAC Tag Verification */
            status = aes256_cmac_tagVerification(&aes256SecretKey, aes256cmac_test_vect[tv].tagSize, &msg, &cmacTag, P_verbose);

            if(P_verbose)
            {
                printf("CMAC Tag Verification check: %d (expected: 1),\n", (status == AUTHENTICATION_SUCCESSFUL));
            }

            failed += ((status != AUTHENTICATION_SUCCESSFUL) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_cmac_tagGenerationVerification_test */

/**
* @brief     AES256 CMAC Tag Verification tests
* @details
* @note
*
* @param[in] P_verbose
*
*/

uint32_t aes256_cmac_tagVerification_test(uint32_t P_verbose)
{

    struct AES_CMAC_msg_stt msg;           /* Message on which CMAC tag is generated */
    struct AES_CMAC_tag_stt cmacTag;           /* CMAC Tag */

    struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    uint32_t failed = 0U;

    uint32_t tv = 0U;

    if(P_verbose)
    {
        /* Test vector coming from NIST CAVP 11.0 cmactestvectors.zip */
        printf("\n");
        printf("AES256 CMAC Tag Verification tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.0 !\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CMAC_VERIFY_TVS; tv++)
    {
        uint32_t i = 0U;
        if (P_verbose)
        {
            printf("\nSize of Message = %d\n", aes256cmac_verifyTest[tv].msgSize);
            printf("Size of requested CMAC Tag = %d\n", aes256cmac_verifyTest[tv].tagSize);
        }
        for (i = 0U; i < C_NB_TEST_CMAC_VERIFY_TV; i++)
        {
            /* Initialize secret key */
            aes256SecretKey.pmSecretKey = (uint8_t*)aes256cmac_verifyTest[tv].verifyTestCmac[i].pKey;
            aes256SecretKey.mSecretKeySize = 32;

            msg.pAddress = (unsigned char *)aes256cmac_verifyTest[tv].verifyTestCmac[i].pMsg;
            msg.messageByteSize = aes256cmac_verifyTest[tv].msgSize;

            cmacTag.pAddress = (uint8_t *)aes256cmac_verifyTest[tv].verifyTestCmac[i].pCmacTag;

            /* AES256 CMAC Tag Verification */
            status = aes256_cmac_tagVerification(&aes256SecretKey, aes256cmac_verifyTest[tv].tagSize, &msg, &cmacTag, P_verbose);

            if(P_verbose)
            {
                printf("CMAC Tag Verification check: %d (expected : %d), ", (status == AUTHENTICATION_SUCCESSFUL), (*aes256cmac_verifyTest[tv].verifyTestCmac[i].pResult == C_TEST_CMAC_TAG_VERIF_PASS));
            }
            if(((status == AUTHENTICATION_SUCCESSFUL) && (*aes256cmac_verifyTest[tv].verifyTestCmac[i].pResult == C_TEST_CMAC_TAG_VERIF_PASS)) ||
               ((status == AUTHENTICATION_FAILED) && (*aes256cmac_verifyTest[tv].verifyTestCmac[i].pResult == C_TEST_CMAC_TAG_VERIF_FAIL)))
            {
                if(P_verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(P_verbose)
            {
                printf("\n");
            }
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_cmac_tagVerification_test */

/**
* @brief      AES256 in GCM mode Authenticated Encryption tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_gcm_authenticatedEncryption_test(uint32_t P_verbose)
{
    struct AES_GCM_msgData_stt msg;                    /* Message to cipher */
    struct AES_GCM_headerData_stt headerData;              /* Header data, Nonce and associated data */
    struct AES_GCM_authCiphMsg_stt authCipheredMsg;    /* GCM authenticated ciphered message */

    uint8_t encryptedMessageBuf[51] = {0};          /* Buffer that will receive the AES256 Encrypted Message */
    uint8_t authTagBuf[16] = {0};                      /* Buffer that will receive the authentication tag */

    struct AES256secretKeyArray_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    int32_t check;
    uint32_t failed = 0U;

    uint32_t tv = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 GCM mode Authenticated Encryption tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 14.0 !\n");
    }

    for (tv = 0U; tv < C_NB_OF_AES_GCM_ENCRPT_TV; tv++)
    {
        uint32_t i = 0U;

        if (P_verbose)
        {
            printf("\nTest Vector number: %d\n", tv);
            printf("Size of Initialization Vector = %d\n", aes256Gcm_encrypt_testVect_array[tv].ivSize);
            printf("Size of PlainText = %d\n", aes256Gcm_encrypt_testVect_array[tv].ptSize);
            printf("Size of Additional Authentication Data = %d\n", aes256Gcm_encrypt_testVect_array[tv].aadSize);
            printf("Size of Authentication Tag = %d\n", aes256Gcm_encrypt_testVect_array[tv].tagSize);
        }
        for (i = 0U; i < C_NB_AES256_GCM_TV; i++)
        {

            /* Initialize secret key */
            aes256SecretKey.pmSecretKey = (uint8_t*)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].key;
            aes256SecretKey.mSecretKeySize = 32;

            /* Initialize IV */
            headerData.pIv = (unsigned char *)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].iv;
            headerData.ivByteSize = aes256Gcm_encrypt_testVect_array[tv].ivSize / 8;

            /* Initialize Additional Authentication Data */
            headerData.pAdditionalAuthenticationData = (unsigned char *)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].aad;
            headerData.AdditionalAuthenticationDataByteSize = aes256Gcm_encrypt_testVect_array[tv].aadSize / 8;

            msg.pAddress = (unsigned char *)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].pt;
            msg.messageByteSize = aes256Gcm_encrypt_testVect_array[tv].ptSize / 8;

            authCipheredMsg.pAddress = encryptedMessageBuf;
            authCipheredMsg.messageByteSize = 0;
            authCipheredMsg.pAuthTag = authTagBuf;
            authCipheredMsg.authTagByteSize = 0;

            /* AES256 GCM Encryption */
            status = aes256_gcm_authenticatedEncryption(&aes256SecretKey, &headerData, aes256Gcm_encrypt_testVect_array[tv].tagSize / 8,
                                                        &msg, &authCipheredMsg, P_verbose);
            if (CSE_NO_ERR == status)
            {
                /* Compare Expected Authenticated CipherText and Authentication Tag from test vector with computed ones */

                check = memcmp(authCipheredMsg.pAddress, (uint8_t*)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].ct, aes256Gcm_encrypt_testVect_array[tv].ptSize / 8);
                check += memcmp(authCipheredMsg.pAuthTag, (uint8_t*)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].tag, aes256Gcm_encrypt_testVect_array[tv].tagSize / 8);
                if(P_verbose)
                {
                    printf("\nGCM Encryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_gcm_authenticatedEncryption_test */

/**
* @brief      AES256 in GCM mode Authenticated Decryption tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_gcm_authenticatedDecryption_test(uint32_t P_verbose)
{
#define C_GCM_RESULT_BUFFER_SIZE 51

    struct AES_GCM_authCiphMsg_stt authCipheredMsg;        /* Message to decipher */
    struct AES_GCM_headerData_stt headerData;                  /* Header data, Nonce and associated data */
    struct AES_GCM_deciphMsgData_stt deCipheredMsg;    /* GCM deciphered message */

    uint8_t decryptedMessageBuf[C_GCM_RESULT_BUFFER_SIZE] = {0};          /* Buffer that will receive the AES256 Encrypted Message */

    struct AES256secretKeyArray_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t status = 0U;
    int32_t check = 1U;        /* Set to error */
    uint32_t failed = 0U;

    uint32_t tv = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 GCM mode Authenticated Decryption tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 14.0 !\n");
    }

    for (tv = 0U; tv < C_NB_OF_AES_GCM_DECRPT_TV; tv++)
    {
        uint32_t i = 0U;

        if (P_verbose)
        {
            printf("\nTest Vector number: %d\n", tv);
            printf("Size of Initialization Vector = %d\n", aes256Gcm_decrypt_testVect_array[tv].ivSize);
            printf("Size of PlainText = %d\n", aes256Gcm_decrypt_testVect_array[tv].ptSize);
            printf("Size of Additional Authentication Data = %d\n", aes256Gcm_decrypt_testVect_array[tv].aadSize);
            printf("Size of Authentication Tag = %d\n", aes256Gcm_decrypt_testVect_array[tv].tagSize);
        }
        for (i = 0U; i < C_NB_AES256_GCM_TV; i++)
        {
            /* Erase result buffers */
            uint32_t j = 0U;
            for (j = 0; j < C_GCM_RESULT_BUFFER_SIZE; j++)
            {
                decryptedMessageBuf[j] = 0U;
            }

            /* Initialize secret key */
            aes256SecretKey.pmSecretKey = (uint8_t*)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].key;
            aes256SecretKey.mSecretKeySize = 32;

            /* Initialize IV */
            headerData.pIv = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].iv;
            headerData.ivByteSize = aes256Gcm_decrypt_testVect_array[tv].ivSize / 8;

            /* Initialize Additional Authentication Data */
            headerData.pAdditionalAuthenticationData = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].aad;
            headerData.AdditionalAuthenticationDataByteSize = aes256Gcm_decrypt_testVect_array[tv].aadSize / 8;

            authCipheredMsg.pAddress = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].ct;
            authCipheredMsg.messageByteSize = aes256Gcm_decrypt_testVect_array[tv].ptSize / 8;
            authCipheredMsg.pAuthTag = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].tag;
            authCipheredMsg.authTagByteSize = aes256Gcm_decrypt_testVect_array[tv].tagSize / 8;

            deCipheredMsg.pDecipheredMessage = decryptedMessageBuf;
            deCipheredMsg.decipheredmessageByteSize = 0;
            deCipheredMsg.authResult = AUTHENTICATION_FAILED;

            /* AES256 GCM Decryption */
            status = aes256_gcm_authenticatedDecryption(&aes256SecretKey, &headerData, aes256Gcm_decrypt_testVect_array[tv].tagSize / 8,
                                                        &authCipheredMsg, &deCipheredMsg, P_verbose);

            if (CSE_NO_ERR == status)
            {
                if ((CSE_NO_ERR == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_PASS == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult))
                {
                    /* Compare Expected plainText from test vector with computed one */
                    check = memcmp(deCipheredMsg.pDecipheredMessage, (uint8_t*)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].pt, aes256Gcm_decrypt_testVect_array[tv].ptSize / 8);
                    if(P_verbose)
                    {
                        printf("\nGCM Decryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                    }
                }
                else
                {
                    /* Verification is not correct, check expected result */
                    if ((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult))
                    {
                        check = 0;
                        check = !((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult));
                        if(P_verbose)
                        {
                            printf("\nGCM Decryption test: Authentication not verified: Authentication check: %d, (expected: 1)\n", check == !((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult)));
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf("\nGCM decryption Failed.\n");
                        }
                        failed++;
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }

        failed += ((status != CSE_NO_ERR) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_gcm_authenticatedDecryption_test */

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

void aes256_exportRamKey_test(uint32_t P_verbose)
{
    uint32_t retVal = CSE_GENERAL_ERR;

    uint8_t encryptedMessageBuf[32] = {0};                /* Buffer that will receive the AES256 Encrypted Message  */

    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt cipherMessage;

    struct AES256secretKeyArray_stt aes256SecretKey;        /* Structure for the AES256 Secret Key */

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 8] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t expectedM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                              0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t expectedM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};


    printf("*\n* RAM Key load cleartext/export/load protected test\n*\n");
    printf("Note that the test Must fail if secure boot was not successful or if debugger is / was attached to the platform\n");
    printf("\n");

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.mSecretKeySize = 32;

    messageToEncrypt.address = (uint8_t*)plainText_tv1;
    messageToEncrypt.byteSize = 16;

    cipherMessage.address = encryptedMessageBuf;
    cipherMessage.byteSize = 16;

    /* Loading first RAM Key K1 */
    printf(" - Load Key K1 in ClearText in RAM location:\n");
    display_buf(" Key K1: ", (uint8_t*)aes256SecretKey.pmSecretKey, aes256SecretKey.mSecretKeySize);

    retVal = CSE_AES256_loadRamKey(&aes256SecretKey);
    printf("return = %x\n", retVal);

    /* Launch AES256 Encryption in ECB mode with RAM K1 */
    printf("-ECB Encryption with K1 test : \n");
    display_buf("Plaintext :  ", (uint8_t*)messageToEncrypt.address, 16 );
    retVal = CSE_AES256_ECB_encrypt(AES256_RAM_KEY_IDX,
                                    (vuint8_t*)messageToEncrypt.address,
                                    (vuint8_t*)cipherMessage.address, 1);
    display_buf("CipherText: ", (uint8_t*)cipherMessage.address, 16 );
    printf("\n");

    if(P_verbose)
    {
        printf("-Exporting RAM Key K1 : \n");
    }
    /* Export RAM Key */
    *M1 = AES256_RAM_KEY_IDX;    /* Index of Key to export */
    *M2 = 48;                     /* Size of memory allocated to receive M2 Message */

    retVal = CSE_AES256_exportRamKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)expectedM4, (uint8_t*)expectedM5);
    if(P_verbose)
    {
        display_buf(" M1 = ", (uint8_t*)M1, 32);
        display_buf(" M2 = ", (uint8_t*)M2, 48);
        display_buf(" M3 = ", (uint8_t*)M3, 16);
        display_buf(" expected M4 = ", (uint8_t*)expectedM4, 32);
        display_buf(" expected M5 = ", (uint8_t*)expectedM5, 16);
        printf("\n");
    }

    /* Loading another RAM Key K2 */
    aes256SecretKey.pmSecretKey = (uint8_t*)key_tv2;
    aes256SecretKey.mSecretKeySize = 32;

    printf(" - Load Key K2 in ClearText in RAM location:\n");
    display_buf(" Key K2: ", (uint8_t*)aes256SecretKey.pmSecretKey, aes256SecretKey.mSecretKeySize);

    retVal = CSE_AES256_loadRamKey(&aes256SecretKey);
    printf("return = %x\n", retVal);

    /* Launch AES256 Encryption in ECB mode with RAM K2 */
    printf("-ECB Encryption with K2 test : \n");
    display_buf(" Plaintext: ", (uint8_t*)messageToEncrypt.address, 16 );
    retVal = CSE_AES256_ECB_encrypt(AES256_RAM_KEY_IDX,
                                    (vuint8_t*)messageToEncrypt.address,
                                    (vuint8_t*)cipherMessage.address, 1);
    display_buf(" CipherText: ", (uint8_t*)cipherMessage.address, 16 );
    printf("\n");

    /* Loading RAM Key K1 (with protected material returned by export_ram_key */
    printf("-Loading RAM Key K1 (with protected material returned by export_ram_key) : \n");
    retVal = CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)M4, (uint8_t*)M5);
    printf(" ret : %x\n", retVal );
    display_buf(" result M4 ", (uint8_t*)M4, 32 );
    display_buf(" result M5 ", (uint8_t*)M5, 16 );

    if (retVal != 0 )
    {
        /* Check status */
        printf("\n");
        printf(" CSE status : %02x\n", CSE->SR.R);
        if( CSE->SR.B.BOK != 1 )
        {
            printf(" Secure boot is not successful (or not ran)\n");
        }
        if( CSE->SR.B.EDB == 1 )
        {
            printf(" Debugger is/was attached\n");
        }
    }

    printf("-ECB Encryption with Key K1 test : \n");
    display_buf("Plaintext: ", (uint8_t*)messageToEncrypt.address, 16 );
    retVal = CSE_AES256_ECB_encrypt(AES256_RAM_KEY_IDX,
                                    (vuint8_t*)messageToEncrypt.address,
                                    (vuint8_t*)cipherMessage.address, 1);

    display_buf("CipherText: ", (uint8_t*)cipherMessage.address, 16 );
    printf("\n");
    if (memcmp((uint8_t*)cipherMessage.address, cipherText_tv1, 16) == 0)
    {
        printf("Key load with protected material was successful \n");
    }
    else
    {
        printf(" Key load with protected material failed - key was not updated \n");
    }
} /* End of aes256_exportRamKey_test */

/**
* @brief      AES256 encryption decryption with protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_ecb_encryptionDecryption_PK_test(uint32_t P_verbose)
{
    uint32_t status;

    uint32_t check;
    uint32_t global_pass = 1;
    uint32_t i = 0U;

    uint8_t encryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Decrypted Message  */

    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt deCipherMessage;

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 8] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t keyIndex = AES256_KEY_1_IDX;
    uint32_t pass = 0;

    struct AES256_key_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
        printf(" - Test AES256 Encryption / Decryption in ECB with NVM key!\n");
    }

    /* Initialize AES256 Key */
    aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.secretKey.mSecretKeySize = 32;
    aes256SecretKey.AES256_flags.R = 0U;                /* No restriction for that key */
    aes256SecretKey.CNT.R = G_aes256KeyCounter;
    aes256SecretKey.AES256_flags.B.encrypt = 1U;    /* Key is allowed for encryption */
    aes256SecretKey.AES256_flags.B.decrypt = 1U;    /* Key is allowed for decryption */

    for (keyIndex = AES256_KEY_1_IDX; keyIndex <= AES256_KEY_MAX_IDX; keyIndex++)
    {
        pass = 1;

        if(P_verbose)
        {
            printf("\nLoad NVM Key at index %d.\n", keyIndex);
        }

        status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                              (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                              aes256SecretKey.secretKey.mSecretKeySize,

                                              authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                              M1, M2, M3,
                                              M4, M5);
        if(status != 0)
        {
            if(P_verbose)
            {
                printf(" - Problem when generating M1..M5 messages\n");
            }
            pass = 0;
        }
        else
        {
            status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
            G_aes256KeyCounter++;    /* For next test */

            if( P_verbose)
            {
                printf("load command returned error code Err=%x\n", status );
            }

            check = memcmp(M4, resM4, 32);
            if( check == 0 )
            {
                if(P_verbose)
                {
                    printf(" - M4 Message as expected\n");
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf(" - M4 Message is not the expected one\n");
                    display_buf(" - resM4   : ", (uint8_t*)resM4, 32);
                    display_buf(" - expected: ", (uint8_t*)M4, 32);
                }
                pass = 0;
            }
            check = memcmp(M5, resM5, 16);
            if( check == 0 )
            {
                if(P_verbose)
                {
                    printf(" - M5 Message as expected\n");
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf(" - M5 Message is not the expected one\n");
                    display_buf(" - resM4   : ", (uint8_t*)resM5, 32);
                    display_buf(" - expected: ", (uint8_t*)M5, 32);
                }
                pass = 0;
            }

            if(pass == 1 )
            {

                /*
                 * Launch AES256 Encryption in ECB mode with key at current NVM location
                 */
                messageToEncrypt.address = (uint8_t*)plainText_tv1;
                cipherMessage.address = encryptedMessageBuf;

                status += CSE_AES256_ECB_encrypt(keyIndex,
                                                 (vuint8_t*)messageToEncrypt.address,
                                                 (vuint8_t*)cipherMessage.address, 1);
                if (CSE_NO_ERR == status)
                {
                    /* Compare Expected ClearText from test vector with computed one */
                    check = memcmp(encryptedMessageBuf, (uint8_t*)cipherText_tv1, 16);
                    if( check == 0 )
                    {
                        if(P_verbose)
                        {
                            printf(" - ECB Encryption test: result as expected\n");
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf(" - ECB Encryption test result is not the expected one\n");
                            display_buf(" - result  : ", (uint8_t*)encryptedMessageBuf, 16);
                            display_buf(" - expected: ", (uint8_t*)cipherText_tv1, 16);
                        }
                        pass = 0;
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf(" - Encryption attempt failed\n");
                    }
                    pass = 0;
                }


                /*
                 * Launch AES256 Decryption in ECB mode with key at current NVM location
                 */
                cipherMessage.address = encryptedMessageBuf;
                deCipherMessage.address = decryptedMessageBuf;

                status += CSE_AES256_ECB_decrypt(keyIndex,
                                                (vuint8_t*)cipherMessage.address,
                                                (vuint8_t*)deCipherMessage.address, 1);
                if (CSE_NO_ERR == status)
                {
                    /* Compare Expected ClearText from test vector with computed one */
                    check = memcmp(decryptedMessageBuf, (uint8_t*)plainText_tv1, 16);
                    if( check == 0 )
                    {
                        if(P_verbose)
                        {
                            printf(" - ECB Decryption test: result as expected\n");
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf(" - ECB Decryption test result is not the expected one\n");
                            display_buf(" - result  : ", (uint8_t*)decryptedMessageBuf, 16);
                            display_buf(" - expected: ", (uint8_t*)plainText_tv1, 16);
                        }
                        pass = 0;
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf(" - Decryption attempt failed\n");
                    }
                    pass = 0;
                }
            }
        }

        if(P_verbose)
        {
            display_pass_fail(pass);
        }

        global_pass &= pass;
    }

    /* return 1 if successful (no test failed) */
    return (global_pass);

} /* End of aes256_ecb_encryptionDecryption_PK_test */

/**
* @brief      AES256 CBC encryption decryption with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_cbc_encryptionDecryption_PK_test(uint32_t P_verbose)
{
    uint32_t status;

    uint32_t check;
    uint32_t global_pass = 1;
    uint32_t i = 0U;

    uint8_t encryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[32] = {0};    /* Buffer that will receive the AES256 Decrypted Message  */

    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt deCipherMessage;
    struct ByteArrayDescriptor_stt aesIv;

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t keyIndex = AES256_KEY_1_IDX;
    uint32_t pass = 0;

    struct AES256_key_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
        printf(" - Test AES256 Encryption / Decryption in ECB with NVM key!\n");
    }

    /* Initialize AES256 Key */
    aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.secretKey.mSecretKeySize = 32;
    aes256SecretKey.AES256_flags.R = 0U;                /* No restriction for that key */
    aes256SecretKey.CNT.R = G_aes256KeyCounter;
    aes256SecretKey.AES256_flags.B.encrypt = 1U;    /* Key is allowed for encryption */
    aes256SecretKey.AES256_flags.B.decrypt = 1U;    /* Key is allowed for decryption */

    for (keyIndex = AES256_KEY_1_IDX; keyIndex <= AES256_KEY_MAX_IDX; keyIndex++)
    {
        pass = 1;

        if(P_verbose)
        {
            printf("\nLoad NVM Key at index %d.\n", keyIndex);
        }
        status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                              (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                              aes256SecretKey.secretKey.mSecretKeySize,

                                              authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                              M1, M2, M3,
                                              M4, M5);
        if(status != 0)
        {
            if(P_verbose)
            {
                printf(" - Problem when generating M1..M5 messages\n");
            }
            pass = 0;
        }
        else
        {
            status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
            G_aes256KeyCounter++;    /* For next test */

            if(P_verbose)
            {
                printf("load command returned error code Err=%x\n", status );
            }

            check = memcmp(M4, resM4, 32);
            if( check == 0 )
            {
                if(P_verbose)
                {
                    printf(" - M4 Message as expected\n");
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf(" - M4 Message is not the expected one\n");
                    display_buf(" - resM4   : ", (uint8_t*)resM4, 32);
                    display_buf(" - expected: ", (uint8_t*)M4, 32);
                }
                pass = 0;
            }
            check = memcmp(M5, resM5, 16);
            if( check == 0 )
            {
                if(P_verbose)
                {
                    printf(" - M5 Message as expected\n");
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf(" - M5 Message is not the expected one\n");
                    display_buf(" - resM4   : ", (uint8_t*)resM5, 32);
                    display_buf(" - expected: ", (uint8_t*)M5, 32);
                }
                pass = 0;
            }

            if(pass == 1 )
            {

                /*
                 * Launch AES256 Encryption in CBC mode with key at current NVM location
                 */
                messageToEncrypt.address = (uint8_t*)plainText_tv1;
                cipherMessage.address = encryptedMessageBuf;

                /* Initialize Initialization Vector */
                aesIv.address = iV_tv1;
                aesIv.byteSize = 16;

                status += CSE_AES256_CBC_encrypt(keyIndex,
                                                (vuint8_t*)messageToEncrypt.address,
                                                (vuint8_t*)cipherMessage.address,
                                                (vuint8_t*)aesIv.address, 1);
                if (CSE_NO_ERR == status)
                {
                    /* Compare Expected ClearText from test vector with computed one */
                    check = memcmp(encryptedMessageBuf, (uint8_t*)cipherText_tv1, 16);
                    if( check == 0 )
                    {
                        if(P_verbose)
                        {
                            printf(" - CBC Encryption test: result as expected\n");
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf(" - CBC Encryption test result is not the expected one\n");
                            display_buf(" - result  : ", (uint8_t*)encryptedMessageBuf, 16);
                            display_buf(" - expected: ", (uint8_t*)cipherText_tv1, 16);
                        }
                        pass = 0;
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf(" - Encryption attempt failed\n");
                    }
                    pass = 0;
                }


                /*
                 * Launch AES256 Decryption in CBC mode with key at current NVM location
                 */
                cipherMessage.address = encryptedMessageBuf;
                deCipherMessage.address = decryptedMessageBuf;

                status += CSE_AES256_CBC_decrypt(keyIndex,
                                                (vuint8_t*)cipherMessage.address,
                                                (vuint8_t*)deCipherMessage.address,
                                                (vuint8_t*)aesIv.address, 1);
                if (CSE_NO_ERR == status)
                {
                    /* Compare Expected ClearText from test vector with computed one */
                    check = memcmp(decryptedMessageBuf, (uint8_t*)plainText_tv1, 16);
                    if( check == 0 )
                    {
                        if(P_verbose)
                        {
                                printf(" - CBC Decryption test: result as expected\n");
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf(" - CBC Decryption test result is not the expected one\n");
                            display_buf(" - result  : ", (uint8_t*)decryptedMessageBuf, 16);
                            display_buf(" - expected: ", (uint8_t*)plainText_tv1, 16);
                        }
                        pass = 0;
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf(" - Decryption attempt failed\n");
                    }
                    pass = 0;
                }
            }
        }

        if(P_verbose)
        {
            display_pass_fail(pass);
        }

        global_pass &= pass;
    }

    /* return 1 if successful (no test failed) */
    return (global_pass);

} /* End of aes256_cbc_encryptionDecryption_PK_test */

/**
* @brief      AES256 CCM encryption decryption with Protected Keys tests
* @details
* @note
*
* @param[in]     P_verbose
*
*/

uint32_t aes256_ccm_encryptionDecryption_PK_test(uint32_t P_verbose)
{
    uint32_t status;

    uint32_t check;
    uint32_t failed = 0;
    uint32_t i = 0U;

    uint32_t tv = 0U;

    struct AES_CCM_msgData_stt msg;                    /* Message to cipher */
    struct AES_CCM_headerData_stt headerData;              /* Header data, Nonce and associated data */
    struct AES_CCM_msgData_stt cipheredMsg;            /* Location for the ciphered message */
    struct AES_CCM_deciphMsgData_stt payloadMsg;   /* Location for the deciphered message, the payload */

    uint8_t encryptedMessageBuf[C_RESULT_BUFFER_SIZE] = {0};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[C_RESULT_BUFFER_SIZE] = {0};    /* Buffer that will receive the AES256 Decrypted Message  */

    struct AES256_key_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
        printf(" - Test AES256 CCM Generation-Encryption / Decryption-Verification with NVM key!\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CCM_VADT; tv++)
    {
        uint32_t keyIndex = 0U;

        /* Initialize secret key */
        aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)aes256ccm_vadt_test_vect[tv].aes256ccm_Key_Alen;
        aes256SecretKey.secretKey.mSecretKeySize = 32;
        aes256SecretKey.CNT.R = G_aes256KeyCounter;
        aes256SecretKey.AES256_flags.R = 0U;            /* No restriction on Keys */
        aes256SecretKey.AES256_flags.B.mac = 1U;        /* Key is allowed for MAC generation and verification */

        /* Initialize Nonce */
        headerData.pNonce = (unsigned char *)aes256ccm_vadt_test_vect[tv].aes256ccm_Nonce_Alen;
        headerData.nonceByteSize = C_NLEN_VADT_SIZE;

        if (P_verbose)
        {
            printf("\nSize of Associated Data = %d\n", aes256ccm_vadt_test_vect[tv].AlenSize);
        }

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        if(P_verbose)
        {
            printf("\nLoad NVM Key at index %d.\n", keyIndex);
        }

        status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                                (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                                aes256SecretKey.secretKey.mSecretKeySize,

                                                authKey, (uint32_t *)UID,
                                                keyIndex, authKeyId ,
                                                aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                                M1, M2, M3,
                                                M4, M5);

        status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
        G_aes256KeyCounter++;    /* For next test */

#ifdef DEBUG_PRINT
        display_buf("resM4   : ", (uint8_t*)resM4, 32);
        display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

        check = memcmp(M4, resM4, 32);
        if(P_verbose)
        {
            printf("M4 Message check: %d (expected: 1), ", check == 0U);
        }

        check = memcmp(M5, resM5, 16);
        if(P_verbose)
        {
            printf("M5 Message check: %d (expected: 1)\n", check == 0U);
        }

        for (i = 0U; i < C_NB_TEST_CCM; i++)
        {
            /* Erase result buffers */
            uint32_t j = 0U;
            for (j = 0; j < C_RESULT_BUFFER_SIZE; j++)
            {
                encryptedMessageBuf[j] = 0U;
                decryptedMessageBuf[j] = 0U;
            }

            /* Initialize Associated Data */
            headerData.pData = (unsigned char *)aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pAdata;
            headerData.DataByteSize = aes256ccm_vadt_test_vect[tv].AlenSize;

            msg.pAddress = (unsigned char *)aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pPayload;
            msg.messageByteSize = C_PLEN_VADT_SIZE;

            cipheredMsg.pAddress = encryptedMessageBuf;
            cipheredMsg.messageByteSize = C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE;    /* Expected size of cipher Text, size allocated to result buffer */

            payloadMsg.pDecipheredMessage = decryptedMessageBuf;
            payloadMsg.decipheredmessageByteSize = C_PLEN_VADT_SIZE;
            payloadMsg.authResult = AUTHENTICATION_FAILED;

            status += CSE_AES256_CCM_encrypt(keyIndex,
                                             &headerData, C_TLEN_VADT_SIZE,
                                             &msg, &cipheredMsg);
            if (CSE_NO_ERR == status)
            {
                /* Compare Expected CipherText from test vector with computed one */

                check = memcmp(cipheredMsg.pAddress, (uint8_t*)aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pCT, C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE);
                if(P_verbose)
                {
                    printf("\nCCM Encryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));

            /* AES256 CCM Decryption */
            status += CSE_AES256_CCM_decrypt(keyIndex,
                                             &headerData, C_TLEN_VADT_SIZE,
                                             &cipheredMsg, &payloadMsg);

            if (CSE_NO_ERR == status)
            {
                /* Compare Expected ClearText from test vector with computed one */

                check = memcmp(payloadMsg.pDecipheredMessage, (uint8_t*)(aes256ccm_vadt_test_vect[tv].testCcmVadt[i].pPayload), C_PLEN_VADT_SIZE);
                if(P_verbose)
                {
                    printf("\nCCM Decryption test: Clear Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_ccm_encryptionDecryption_PK_test */

/**
* @brief     AES256 CMAC Tag Generation tests with protected key
* @details
* @note
*
* @param[in] P_verbose
*
*/

uint32_t aes256_cmac_tagGenerationVerification_PK_test(uint32_t P_verbose)
{
    uint32_t status = 0U;
    int32_t check = 1U;        /* Set to error */
    uint32_t failed = 0U;
    uint32_t i = 0U;

    uint32_t tv = 0U;

    struct AES_CMAC_msg_stt msg;           /* Message on which CMAC tag is generated */
    struct AES_CMAC_tag_stt cmacTag;           /* CMAC Tag */

    uint8_t cmacTagBuf[40] = {0};           /* Buffer that receive the generated CMAC Tag */

    struct AES256_key_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    if(P_verbose)
    {
        /* Test vector coming from NIST CAVP 11.0 cmactestvectors.zip */
        printf("\n");
        printf("AES256 CMAC Tag Generation and Verification tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.0 !\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CMAC_TVS; tv++)
    {
        if (P_verbose)
        {
            printf("\nSize of Message = %d\n", aes256cmac_test_vect[tv].msgSize);
            printf("Size of requested CMAC Tag = %d\n", aes256cmac_test_vect[tv].tagSize);
        }

        for (i = 0U; i < C_NB_TEST_CMAC_TV; i++)
        {
            uint32_t keyIndex = 0U;
            uint32_t success = 0U;

            /* Initialize secret key */
            aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)aes256cmac_test_vect[tv].testCmac[i].pKey;
            aes256SecretKey.secretKey.mSecretKeySize = 32;
            aes256SecretKey.CNT.R = G_aes256KeyCounter;
            aes256SecretKey.AES256_flags.R = 0U;            /* No restriction on Keys */
            aes256SecretKey.AES256_flags.B.mac = 1U;        /* Key is allowed for MAC generation and verification */

            msg.pAddress = (unsigned char *)aes256cmac_test_vect[tv].testCmac[i].pMsg;
            msg.messageByteSize = aes256cmac_test_vect[tv].msgSize;

            cmacTag.pAddress = cmacTagBuf;

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

            status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                                    (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                                    aes256SecretKey.secretKey.mSecretKeySize,

                                                    authKey, (uint32_t *)UID,
                                                    keyIndex, authKeyId ,
                                                    aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                                    M1, M2, M3,
                                                    M4, M5);

            status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
            G_aes256KeyCounter++;    /* For next test */

#ifdef DEBUG_PRINT
            display_buf("resM4   : ", (uint8_t*)resM4, 32);
            display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

            check = memcmp(M4, resM4, 32);
            if(P_verbose)
            {
                printf("M4 Message check: %d (expected: 1), ", check == 0U);
            }

            check = memcmp(M5, resM5, 16);
            if(P_verbose)
            {
                printf("M5 Message check: %d (expected: 1)\n", check == 0U);
            }

            /* AES256 CMAC Tag Generation */
            status = CSE_AES256_CMAC_tagGeneration(keyIndex, aes256cmac_test_vect[tv].tagSize, &msg, &cmacTag);

            if (CSE_NO_ERR == status)
            {
                /* Compare Expected CipherText from test vector with computed one */
                check = memcmp(cmacTag.pAddress, (uint8_t*)aes256cmac_test_vect[tv].testCmac[i].pCmacTag, aes256cmac_test_vect[tv].tagSize);
                if(P_verbose)
                {
                    printf("\nCMAC Tag Generation test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));

            /* Test CMAC Tag verification on same test vectors */
            /* Initialize secret key */
            msg.pAddress = (unsigned char *)aes256cmac_test_vect[tv].testCmac[i].pMsg;
            msg.messageByteSize = aes256cmac_test_vect[tv].msgSize;

            cmacTag.pAddress = (uint8_t *)aes256cmac_test_vect[tv].testCmac[i].pCmacTag;

            /* AES256 CMAC Tag Verification */
            status = CSE_AES256_CMAC_tagVerification(keyIndex, aes256cmac_test_vect[tv].tagSize, &msg, &cmacTag, &success);
            /* Translate CSE error code */
            if(CSE_NO_ERR == status)
            {
                if( success == 0 )
                {
                    status = AUTHENTICATION_SUCCESSFUL;
                }
                else
                {
                    status = AUTHENTICATION_FAILED;
                }
            }

            if(P_verbose)
            {
                printf("CMAC Tag Verification check: %d (expected: 1), ", (status == AUTHENTICATION_SUCCESSFUL));
            }

            failed += ((status != AUTHENTICATION_SUCCESSFUL) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_cmac_tagGenerationVerification_PK_test */

/**
* @brief     AES256 CMAC Tag Verification tests with protected key
* @details
* @note
*
* @param[in] P_verbose
*
*/

uint32_t aes256_cmac_tagVerification_PK_test(uint32_t P_verbose)
{
    uint32_t status = 0U;
    uint32_t failed = 0U;
    uint32_t check = 1U;
    uint32_t i = 0U;

    uint32_t tv = 0U;

    struct AES_CMAC_msg_stt msg;           /* Message on which CMAC tag is generated */
    struct AES_CMAC_tag_stt cmacTag;           /* CMAC Tag */

    struct AES256_key_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    if(P_verbose)
    {
        /* Test vector coming from NIST CAVP 11.0 cmactestvectors.zip */
        printf("\n");
        printf("AES256 CMAC Tag Verification tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 11.0 !\n");
    }

    for (tv = 0U; tv < C_NB_TEST_CMAC_VERIFY_TVS; tv++)
    {
        uint32_t keyIndex = 0U;
        uint32_t success = 0U;

        if (P_verbose)
        {
            printf("\nSize of Message = %d\n", aes256cmac_verifyTest[tv].msgSize);
            printf("Size of requested CMAC Tag = %d\n", aes256cmac_verifyTest[tv].tagSize);
        }
        for (i = 0U; i < C_NB_TEST_CMAC_VERIFY_TV; i++)
        {
            /* Initialize secret key */
            aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)aes256cmac_verifyTest[tv].verifyTestCmac[i].pKey;
            aes256SecretKey.secretKey.mSecretKeySize = 32;
            aes256SecretKey.CNT.R = G_aes256KeyCounter;
            aes256SecretKey.AES256_flags.R = 0U;            /* No restriction on Keys */
            aes256SecretKey.AES256_flags.B.mac = 1U;        /* Key is allowed for MAC generation and verification */

            msg.pAddress = (unsigned char *)aes256cmac_verifyTest[tv].verifyTestCmac[i].pMsg;
            msg.messageByteSize = aes256cmac_verifyTest[tv].msgSize;

            cmacTag.pAddress = (uint8_t *)aes256cmac_verifyTest[tv].verifyTestCmac[i].pCmacTag;

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("Load NVM Key at index %d.\n", keyIndex);
            }

            status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                                    (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                                    aes256SecretKey.secretKey.mSecretKeySize,

                                                    authKey, (uint32_t *)UID,
                                                    keyIndex, authKeyId ,
                                                    aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                                    M1, M2, M3,
                                                    M4, M5);

            status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
            G_aes256KeyCounter++;    /* For next test */

#ifdef DEBUG_PRINT
            display_buf("resM4   : ", (uint8_t*)resM4, 32);
            display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

            check = memcmp(M4, resM4, 32);
            if(P_verbose)
            {
                printf("M4 Message check: %d (expected: 1), ", check == 0U);
            }

            check = memcmp(M5, resM5, 16);
            if(P_verbose)
            {
                printf("M5 Message check: %d (expected: 1)\n", check == 0U);
            }

            /* AES256 CMAC Tag Verification */
            status = CSE_AES256_CMAC_tagVerification(keyIndex, aes256cmac_verifyTest[tv].tagSize, &msg, &cmacTag, &success);
            if(CSE_NO_ERR == status)
            {
                if( success == 0 )
                {
                    status = AUTHENTICATION_SUCCESSFUL;
                }
                else
                {
                    status = AUTHENTICATION_FAILED;
                }
            }

            if(P_verbose)
            {
                printf("CMAC Tag Verification check: %d (expected : %d),\n", (status == AUTHENTICATION_SUCCESSFUL), (*aes256cmac_verifyTest[tv].verifyTestCmac[i].pResult == C_TEST_CMAC_TAG_VERIF_PASS));
            }
            if(((status == AUTHENTICATION_SUCCESSFUL) && (*aes256cmac_verifyTest[tv].verifyTestCmac[i].pResult == C_TEST_CMAC_TAG_VERIF_PASS)) ||
               ((status == AUTHENTICATION_FAILED) && (*aes256cmac_verifyTest[tv].verifyTestCmac[i].pResult == C_TEST_CMAC_TAG_VERIF_FAIL)))
            {
                if(P_verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(P_verbose)
            {
                printf("\n");
            }
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_cmac_tagVerification_PK_test */

/**
* @brief      AES256 in GCM mode Authenticated Encryption with protected key tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_gcm_authenticatedEncryption_PK_test(uint32_t P_verbose)
{
    uint32_t status = 0U;
    int32_t check = 1U;
    uint32_t failed = 0U;
    uint32_t i = 0U;

    uint32_t tv = 0U;

    struct AES_GCM_msgData_stt msg;                    /* Message to cipher */
    struct AES_GCM_headerData_stt headerData;              /* Header data, Nonce and associated data */
    struct AES_GCM_authCiphMsg_stt authCipheredMsg;    /* GCM authenticated ciphered message */

    uint8_t encryptedMessageBuf[51] = {0};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t authTagBuf[16] = {0};                      /* Buffer that will receive the authentication tag */

    struct AES256_key_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }


    if(P_verbose)
    {
        printf("\n");
        printf("AES256 GCM mode Authenticated Encryption tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 14.0 !\n");
    }

    for (tv = 0U; tv < C_NB_OF_AES_GCM_ENCRPT_TV; tv++)
    {
        if (P_verbose)
        {
            printf("\nTest Vector number: %d\n", tv);
            printf("Size of Initialization Vector = %d\n", aes256Gcm_encrypt_testVect_array[tv].ivSize);
            printf("Size of PlainText = %d\n", aes256Gcm_encrypt_testVect_array[tv].ptSize);
            printf("Size of Additional Authentication Data = %d\n", aes256Gcm_encrypt_testVect_array[tv].aadSize);
            printf("Size of Authentication Tag = %d\n", aes256Gcm_encrypt_testVect_array[tv].tagSize);
        }
        for (i = 0U; i < C_NB_AES256_GCM_TV; i++)
        {
            uint32_t keyIndex = 0U;

            /* Initialize secret key */
            aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].key;
            aes256SecretKey.secretKey.mSecretKeySize = 32;
            aes256SecretKey.CNT.R = G_aes256KeyCounter;
            aes256SecretKey.AES256_flags.R = 0U;            /* No restriction on Keys */
            aes256SecretKey.AES256_flags.B.mac = 1U;        /* Key is allowed for MAC generation and verification */

            /* Initialize IV */
            headerData.pIv = (unsigned char *)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].iv;
            headerData.ivByteSize = aes256Gcm_encrypt_testVect_array[tv].ivSize / 8;

            /* Initialize Additional Authentication Data */
            headerData.pAdditionalAuthenticationData = (unsigned char *)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].aad;
            headerData.AdditionalAuthenticationDataByteSize = aes256Gcm_encrypt_testVect_array[tv].aadSize / 8;

            msg.pAddress = (unsigned char *)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].pt;
            msg.messageByteSize = aes256Gcm_encrypt_testVect_array[tv].ptSize / 8;

            authCipheredMsg.pAddress = encryptedMessageBuf;
            authCipheredMsg.messageByteSize = 0;
            authCipheredMsg.pAuthTag = authTagBuf;
            authCipheredMsg.authTagByteSize = 0;

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

            status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                                    (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                                    aes256SecretKey.secretKey.mSecretKeySize,

                                                    authKey, (uint32_t *)UID,
                                                    keyIndex, authKeyId ,
                                                    aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                                    M1, M2, M3,
                                                    M4, M5);

            status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
            G_aes256KeyCounter++;    /* For next test */

    #ifdef DEBUG_PRINT
            display_buf("resM4   : ", (uint8_t*)resM4, 32);
            display_buf("resM5   : ", (uint8_t*)resM5, 16);
    #endif

            check = memcmp(M4, resM4, 32);
            if(P_verbose)
            {
                printf("M4 Message check: %d (expected: 1), ", check == 0U);
            }

            check = memcmp(M5, resM5, 16);
            if(P_verbose)
            {
                printf("M5 Message check: %d (expected: 1)\n", check == 0U);
            }

            /* AES256 GCM Encryption */
            status = CSE_AES256_GCM_encrypt(keyIndex, &headerData, aes256Gcm_encrypt_testVect_array[tv].tagSize / 8,
                                                        &msg, &authCipheredMsg);
            if (CSE_NO_ERR == status)
            {
                /* Compare Expected Authenticated CipherText and Authentication Tag from test vector with computed ones */

                check = memcmp(authCipheredMsg.pAddress, (uint8_t*)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].ct, aes256Gcm_encrypt_testVect_array[tv].ptSize / 8);
                check += memcmp(authCipheredMsg.pAuthTag, (uint8_t*)aes256Gcm_encrypt_testVect_array[tv].aes256GcmEncrypt_TV[i].tag, aes256Gcm_encrypt_testVect_array[tv].tagSize / 8);
                if(P_verbose)
                {
                    printf("\nGCM Encryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_gcm_authenticatedEncryption_PK_test */

/**
* @brief      AES256 in GCM mode Authenticated Decryption with protected keys_tests
* @details
* @note
*
* @param[in]    P_verbose
*
*/

uint32_t aes256_gcm_authenticatedDecryption_PK_test(uint32_t P_verbose)
{
#define C_GCM_RESULT_BUFFER_SIZE 51
    uint32_t status = 0U;
    int32_t check = 1U;        /* Set to error */
    uint32_t failed = 0U;
    uint32_t i = 0U;

    uint32_t tv = 0U;

    struct AES_GCM_authCiphMsg_stt authCipheredMsg;        /* Message to decipher */
    struct AES_GCM_headerData_stt headerData;                  /* Header data, Nonce and associated data */
    struct AES_GCM_deciphMsgData_stt deCipheredMsg;    /* GCM deciphered message */

    uint8_t decryptedMessageBuf[C_GCM_RESULT_BUFFER_SIZE] = {0};    /* Buffer that will receive the AES256 Decrypted Message  */

    struct AES256_key_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    /* Get UID of the target HSM */
    if(P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }

    /* Clear buffers */
    for(i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 GCM mode Authenticated Decryption tests !\n");
        printf("Test against Known Answer Test Vectors coming from NIST CAVP 14.0 !\n");
    }

    for (tv = 0U; tv < C_NB_OF_AES_GCM_DECRPT_TV; tv++)
    {
        uint32_t keyIndex = 0U;

        if (P_verbose)
        {
            printf("\nTest Vector number: %d\n", tv);
            printf("Size of Initialization Vector = %d\n", aes256Gcm_decrypt_testVect_array[tv].ivSize);
            printf("Size of PlainText = %d\n", aes256Gcm_decrypt_testVect_array[tv].ptSize);
            printf("Size of Additional Authentication Data = %d\n", aes256Gcm_decrypt_testVect_array[tv].aadSize);
            printf("Size of Authentication Tag = %d\n", aes256Gcm_decrypt_testVect_array[tv].tagSize);
        }
        for (i = 0U; i < C_NB_AES256_GCM_TV; i++)
        {
            /* Erase result buffers */
            uint32_t j = 0U;
            for (j = 0; j < C_GCM_RESULT_BUFFER_SIZE; j++)
            {
                decryptedMessageBuf[j] = 0U;
            }

            /* Initialize secret key */
            aes256SecretKey.secretKey.pmSecretKey = (uint8_t*)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].key;
            aes256SecretKey.secretKey.mSecretKeySize = 32;
            aes256SecretKey.CNT.R = G_aes256KeyCounter;
            aes256SecretKey.AES256_flags.R = 0U;            /* No restriction on Keys */
            aes256SecretKey.AES256_flags.B.mac = 1U;        /* Key is allowed for MAC generation and verification */

            /* Initialize IV */
            headerData.pIv = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].iv;
            headerData.ivByteSize = aes256Gcm_decrypt_testVect_array[tv].ivSize / 8;

            /* Initialize Additional Authentication Data */
            headerData.pAdditionalAuthenticationData = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].aad;
            headerData.AdditionalAuthenticationDataByteSize = aes256Gcm_decrypt_testVect_array[tv].aadSize / 8;

            authCipheredMsg.pAddress = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].ct;
            authCipheredMsg.messageByteSize = aes256Gcm_decrypt_testVect_array[tv].ptSize / 8;
            authCipheredMsg.pAuthTag = (unsigned char *)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].tag;
            authCipheredMsg.authTagByteSize = aes256Gcm_decrypt_testVect_array[tv].tagSize / 8;

            deCipheredMsg.pDecipheredMessage = decryptedMessageBuf;
            deCipheredMsg.decipheredmessageByteSize = 0;
            deCipheredMsg.authResult = AUTHENTICATION_FAILED;

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

            status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                                    (uint32_t *)aes256SecretKey.secretKey.pmSecretKey,
                                                    aes256SecretKey.secretKey.mSecretKeySize,

                                                    authKey, (uint32_t *)UID,
                                                    keyIndex, authKeyId ,
                                                    aes256SecretKey.CNT.R, aes256SecretKey.AES256_flags.R,
                                                    M1, M2, M3,
                                                    M4, M5);

            status += CSE_AES256_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
            G_aes256KeyCounter++;    /* For next test */

#ifdef DEBUG_PRINT
            display_buf("resM4   : ", (uint8_t*)resM4, 32);
            display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

            check = memcmp(M4, resM4, 32);
            if(P_verbose)
            {
                printf("M4 Message check: %d (expected: 1), ", check == 0U);
            }

            check = memcmp(M5, resM5, 16);
            if(P_verbose)
            {
                printf("M5 Message check: %d (expected: 1)\n", check == 0U);
            }

            /* AES256 GCM Decryption */
            status = CSE_AES256_GCM_decrypt(keyIndex, &headerData, aes256Gcm_decrypt_testVect_array[tv].tagSize / 8,
                                                        &authCipheredMsg, &deCipheredMsg);

            if (CSE_NO_ERR == status)
            {
                if ((CSE_NO_ERR == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_PASS == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult))
                {
                    /* Compare Expected plainText from test vector with computed one */
                    check = memcmp(deCipheredMsg.pDecipheredMessage, (uint8_t*)aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].pt, aes256Gcm_decrypt_testVect_array[tv].ptSize / 8);
                    if(P_verbose)
                    {
                        printf("\nGCM Decryption test: Cipher Text check: %d, (expected: 1)\n", check == 0);
                    }
                }
                else
                {
                    /* Verification is not correct, check expected result */
                    if ((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult))
                    {
                        check = 0;
                        check = !((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult));
                        if(P_verbose)
                        {
                            printf("\nGCM Decryption test: Authentication not verified: Authentication check: %d, (expected: 1)\n", check == !((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == aes256Gcm_decrypt_testVect_array[tv].aes256GcmDecrypt_TV[i].expectedTestResult)));
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf("\nGCM decryption Failed.\n");
                        }
                        failed++;
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }

        failed += ((status != CSE_NO_ERR) || (check != 0));
        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of aes256_gcm_authenticatedDecryption_PK_test */


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
uint32_t CSE_NVM_AES256_key_update_interactive_test(int verbose) {
  uint8_t UID[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t authKey[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint32_t err;

  int pass = 1;

  uint32_t authKeyId = 0x1;
  uint32_t keyIndex = 0x1;
  uint32_t cid = 1;
  uint32_t fid = 0;

  uint8_t key_val[16];

  uint8_t m1[32];
  uint8_t m2[48];
  uint8_t m3[16];
  uint32_t m4_w[8];
  uint32_t m5_w[4];
  uint32_t confirm;
  uint32_t flags = 0U;
  int end = 0;
  int i;

  uint32_t status;
  uint32_t MAC[4];

  uint32_t M4[8];
  uint32_t M5[4];
  uint32_t match;
  uint32_t uid_source;

  /* Test CSE NVM Key update */
  printf("*\n* AES256 NVM key update test\n*\n");

  if (verbose) {
    printf("Test Update key with user provided values\n");

    printf(" It will compute the M1..M5 messages required to load an AES256 key in a NVM location.\n");
    printf(" then you'll have to confirm if you want to perform the write operation or not\n");
  }

  while (!end) {
    /* Ask for extended key set or normal Key set*/
    get_int("chip UID (0) or Wildcard UID (1) or user provided UID (2): ", &uid_source);
    if(uid_source == 0 )
    {
        /* Read Chip UID */
        CSE_GetId(challenge, (uint32_t*)UID, &status, MAC);
    }
    else
    {
        if(uid_source == 1 )
        {
            for(i=0; i<15; i++)
            {
                UID[i] = 0U;
            }
        }
        else
        {
            if(uid_source == 2 )
            {
                /* Ask for UID value */
                get_hex("Please enter UID value (hex) - 15 bytes : ", UID, 15);
            }
            else
            {
                /* unsupported UID_source */
                printf(" Unsupported value -> Wildcard UID will be used \n");
                for(i=0; i<15; i++)
                {
                    UID[i] = 0U;
                }
            }
        }
    }

    /* Ask for key index */
    get_int("Key index (dec) : ", &keyIndex);

    /* Ask for new key value */
    get_hex("Key value (hex) - 32 bytes : ", key_val, 32);

    /* Ask for auth key index */
    get_int("Auth Key index (dec) : ", &authKeyId);

    /* Ask for auth key value */
    get_hex("Auth Key value (hex) - 16 bytes : ", authKey, 16);

    /* Ask for new flags */
    get_hex("Flags (hex) - 2 bytes (binary format = sign|verif|enc|dec| mac|mac_verif|rfu|rfu| extract|rfu|WP|BP| DP|WC|00b): ", (uint8_t*)(&flags), 2);
    fid = (uint32_t)flags;

    /* Ask for counter value */
    get_int("Counter value (dec) - !!! 28 bits max (268 435 456) : ", &cid);

    /* display summary */
    display_buf("UID      : ", (uint8_t*)UID, 15);
    display_buf("new key  : ", key_val, 32);
    display_buf("aut key  : ", authKey, 16);
    printf("key id : %d ", keyIndex);
    printf("aut id : %d ", authKeyId);
    printf("flags  : %x ", fid);
    printf("count  : %x ", cid);
    printf("\r\n");

    status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                            (uint32_t *)key_val, 32,

                                            (uint32_t*)authKey, (uint32_t *)UID,
                                            keyIndex, authKeyId ,
                                            cid, fid,
                                            (uint32_t *)m1, (uint32_t *)m2, (uint32_t *)m3,
                                            m4_w, m5_w);

    display_buf("M1 : ", m1, 32);
    display_buf("M2 : ", m2, 48);
    display_buf("M3 : ", m3, 16);
    display_buf("M4 : ", (uint8_t*)m4_w, 32);
    display_buf("M5 : ", (uint8_t*)m5_w, 16);
    printf("\r\n");

    /* ask for confirmation */
    get_int("Do you want to write key (1 : yes, any other value : no) ? ",
            &confirm);

    if (confirm != 1) {
      printf(" write operation skipped\n");
    }
    else {
      printf("Write key\n");

      for (i = 0; i < 8; i++)
      {
        M4[i] = 0x00000000;
      }
      for (i = 0; i < 4; i++)
      {
        M5[i] = 0x00000000;
      }

      err = CSE_AES256_loadKey((uint8_t*)m1, (uint8_t*)m2, (uint8_t*)m3, (uint8_t*)M4, (uint8_t*)M5);
      printf("Err=%x, ", err);
      display_CSE_status();
      printf("\n\r");

          printf("Received update verification messages \n");
        printf("M4 : ");
        serialdisplay_array((uint8_t*)M4, 32);
        printf("M5 : ");
        serialdisplay_array((uint8_t*)M5, 16);


        /* Check if they match with expected messages */
        match = 1;

        for (i = 0; i < 8; i++)
        {
            if(m4_w[i] != M4[i])
            {
                match = 0;
            }
          }
          if( match )
          {
              printf("M4 as expected\n");
          }
          else
          {
              printf("M4 does not match expected result\n");
          }

          match = 1;
          for (i = 0; i < 4; i++)
          {
            if(m5_w[i] != M5[i])
            {
                match = 0;
            }
          }
          if( match )
          {
              printf("M5 as expected\n");
          }
          else
          {
              printf("M5 does not match expected result\n");
          }

#if 0
      printf("\nNow try to use key we just updated \n");

      /* Now check if the key changed */

      /* encryption test */
      printf("AES ECB encryption with key %d (0x%X) :\n\r", keyIndex, keyIndex);
      for (i = 0; i < 16 * 2; i++) {
        computed1[i] = 0;
      }
      err = CSE_AES_encrypt_ECB(id, (uint32_t*)cleartext, (uint32_t*)computed1,
                                2, 0);
      printf("Err=%x, ", err);
      display_CSE_status();
      serialdisplay_array((uint8_t*)computed1, 16 * 2);

      printf("AES CMAC with key %d (0x%X) :\n\r", keyIndex, keyIndex);
      for (i = 0; i < 16; i++) {
        computed1[i] = 0;
      }
      err = CSE_AES_generate_MAC(id, (uint64_t*)&bitlength,
                                 (uint32_t*)cleartext, (uint32_t*)computed1, 0);
      printf("Err=%x, ", err);
      display_CSE_status();
      serialdisplay_array((uint8_t*)computed1, 16);
      printf("\n\r");
#endif

    }
    /* ask for confirmation */
    get_int(
        "Do you want to write another key ? (1 : yes, any other value : no) ? ",
        &confirm);
    if (confirm != 1) {
      end = 1;
    }
  }
  return (pass);
}

