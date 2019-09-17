/*
    SPC5-CRYPTO - Copyright (C) 2016 STMicroelectronics

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
 * @file    CSE_ext_ECC_ECIES_test.c
 * @brief   ECIES encryption / decryption tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include <string.h>

#include "config.h"
#include "serialprintf.h"

#include "cse_types.h"
#include "CSE_ext_ECC.h"
#include "CSE_ext_ECC_ECIES_test.h"
#include "CSE_ext_ECC_ECIES_TV.h"

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif

/* Exported functions ---------------------------------------------------------*/
extern const ecies_decrypt_testVect_stt eciesDecryptTestVect_array[NB_OF_ECIES_DEC_TV];
extern const ecies_encryptDecrypt_testVect_stt eciesEncryptDecryptTestVect_array[NB_OF_ECIES_ENC_DEC_TV];

/**
* @brief  ECC ECIES Encryption
*
* @param    *P_pPubKey           Pointer to \ref ECCpubKeyByteArray_stt structure describing the Recipient's Public Key
* @param    *P_pMessageToEncrypt Pointer to \ref ByteArrayDescriptor_stt describing message to encrypt
* @param    *P_pEciesSharedData  Pointer to \ref ECIES_sharedData_stt describing the ECIES Shared Data (shared information)
* @param    *P_pEncryptedMessage Pointer to \ref ECIES_encMsg_stt describing the Encrypted Message
* @param    verbose              To activate verbose mode during test
*
* @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

int32_t ECC_ecies_encryption(struct ECCpubKeyByteArray_stt * P_pPubKey,
                             struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                             struct ECIES_sharedData_stt * P_pEciesSharedData,
                             struct ECIES_encMsg_stt *P_pEncryptedMessage,
                             uint32_t P_verbose)
{
//  ECC_keyPair_stt key;
    struct ECCpubKeyByteArray_stt pubKey;
    struct ECCprivKeyByteArray_stt privKey;

    int32_t status = ECC_ERR_BAD_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    pubKey.pmPubKeyCoordX = P_pPubKey->pmPubKeyCoordX;
    pubKey.mPubKeyCoordXSize = P_pPubKey->mPubKeyCoordXSize;
    pubKey.pmPubKeyCoordY = P_pPubKey->pmPubKeyCoordY;
    pubKey.mPubKeyCoordYSize = P_pPubKey->mPubKeyCoordYSize;

    /* No Private key */
    privKey.pmPrivKey = NULL;
    privKey.mPrivKeySize = 0;

    ret = CSE_ECC_LoadRamKey(&pubKey, &privKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch ECIES Encryption */
        ret = CSE_ECC_EciesEncrypt(ECC_RAM_KEY_IDX, P_pMessageToEncrypt,
                                   P_pEciesSharedData, P_pEncryptedMessage);

#ifdef PERF_MEASURMENT
    	PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

    /* Translate CSE error code */
    if( CSE_NO_ERR == ret )
    {
         status = ECC_SUCCESS;
    }
}

    /* default value would be ECC_ERR_BAD_PARAMETER unless generation ran successfully */
    return(status);
} /* End of ECC_ecies_encryption */

/**
* @brief  ECC ECIES Decryption
*
* @param    *P_pPrivKey          Pointer to \ref ECCprivKeyByteArray_stt structure describing Recipient's Private Key
* @param    *P_pMessageToDecrypt Pointer to \ref ECIES_encMsg_stt describing the message to decrypt
* @param    *P_pEciesSharedData  Pointer to \ref ECIES_sharedData_stt describing the ECIES Shared Data (shared information)
* @param    *P_pDecryptedMessage Pointer to \ref ByteArrayDescriptor_stt: decrypted Message
* @param    verbose              To activate verbose mode during test
*
* @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

int32_t ECC_ecies_decryption(struct ECCprivKeyByteArray_stt * P_pPrivKey,
                             struct ECIES_encMsg_stt * P_pMessageToDecrypt,
                             struct ECIES_sharedData_stt * P_pEciesSharedData,
                             struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                             uint32_t P_verbose)
{
//    ECC_keyPair_stt key;
    struct ECCpubKeyByteArray_stt pubKey;
    struct ECCprivKeyByteArray_stt privKey;

    int32_t status = ECC_ERR_BAD_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* No public key */
    pubKey.pmPubKeyCoordX = NULL;
    pubKey.mPubKeyCoordXSize = 0U;
    pubKey.pmPubKeyCoordY = NULL;
    pubKey.mPubKeyCoordYSize = 0U;

    /* Private key */
    privKey.pmPrivKey =  P_pPrivKey->pmPrivKey;
    privKey.mPrivKeySize = P_pPrivKey->mPrivKeySize;

    ret = CSE_ECC_LoadRamKey(&pubKey, &privKey);
    if( CSE_NO_ERR == ret)
    {
        /* Launch ECIES Decryption */
        ret = CSE_ECC_EciesDecrypt(ECC_RAM_KEY_IDX, P_pMessageToDecrypt,
                                   P_pEciesSharedData, P_pDecryptedMessage);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
             status = ECC_SUCCESS;
        }
    }

    /* default value would be ECC_ERR_BAD_PARAMETER unless generation ran successfully */
    return(status);
} /* End of ECC_ecies_decryption */

/**
* @brief      ECC ECIES decryption test
* @details    The function implements test of following ECIES decryption scheme:
*              - Diffie-Hellman for Key Agreement
*              - ANS X9.63 for Key Derivation Function
*              - AES-128 in CBC mode for Encryption and IV value at zero
*              - CMAC-AES-128 for CMAC tag computation
*              - Elliptic Curve Cryptography with NIST P256 curve
* @note        The test performs decryption of Known Answer Test Vectors generated by OpenSSL
* @param[in]   verbose
*
*/
uint32_t ecc_ecies_decryption_test(uint32_t verbose)
{
    uint32_t status = 0U;

    struct ByteArrayDescriptor_stt messageToDecrypt;
    struct ByteArrayDescriptor_stt clearTextMessage;

    struct ECIES_sharedData_stt eciesSharedData;
    struct ECIES_sharedData1_stt eciesSharedData1;
    struct ECIES_sharedData2_stt eciesSharedData2;
    struct ECIES_encMsg_stt encryptedMessage;

    uint8_t clearTextMessageBuf[C_MAX_MESSAGE_LEN] = {0};    /* Buffer that will receive the ECIES Decrypted Message  */

    struct ECCprivKeyByteArray_stt PrivKey_st;     /* Structure that will contain the public key */
    struct ECCpubKeyByteArray_stt FmrPubKey_st;    /* Structure for the Encryption Ephemeral Public Key */

    int32_t check;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

    if(verbose)
    {
        printf("\n");
        printf("ECIES Test against known Answer Test Vectors.\n");
        printf("ECIES Decryption of Message encrypted by ECIES encryption implemented in OpenSSL !\n");
    }
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        for(tv = 0U; tv < NB_OF_ECIES_DEC_TV; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC ECIES Decryption with ");
                printf("test vector #1.%d: \n", (tv + 1));
            }

            /* No Public Key for Decryption */
            /* No Private Key for Encryption test */
            PrivKey_st.pmPrivKey = eciesDecryptTestVect_array[tv].recipientPrivateKey;
            PrivKey_st.mPrivKeySize = C_P256_MOD_SIZE;

            /* InitializesSender Public Key from Test Vectors */
            FmrPubKey_st.pmPubKeyCoordX = eciesDecryptTestVect_array[tv].senderFmrPublicXKey;
            FmrPubKey_st.pmPubKeyCoordY = eciesDecryptTestVect_array[tv].senderFmrPublicYKey;
            FmrPubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            FmrPubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            /* Initializes Message to decrypt from test vector */
            messageToDecrypt.address = eciesDecryptTestVect_array[tv].encryptedMessage;
            messageToDecrypt.byteSize = eciesDecryptTestVect_array[tv].messageLen;

            /* Initializes Shared Data with test vectors */
            eciesSharedData1.pSharedData1 = (uint8_t *)eciesDecryptTestVect_array[tv].sharedData1;
            eciesSharedData1.sharedData1Size = eciesDecryptTestVect_array[tv].sharedData1Len;
            eciesSharedData2.pSharedData2 = (uint8_t *)eciesDecryptTestVect_array[tv].sharedData2;
            eciesSharedData2.sharedData2Size = eciesDecryptTestVect_array[tv].sharedData2Len;

            eciesSharedData.pSharedData1_stt = &eciesSharedData1;
            eciesSharedData.pSharedData2_stt = &eciesSharedData2;

            encryptedMessage.pEciesEncryptedMessage = (uint8_t*)messageToDecrypt.address;
            encryptedMessage.eciesEncryptedMessageSize = messageToDecrypt.byteSize;
            encryptedMessage.pSenderFmrPubKey = &FmrPubKey_st;
            encryptedMessage.pTag = (uint8_t*)eciesDecryptTestVect_array[tv].tag;
            encryptedMessage.tagSize = eciesDecryptTestVect_array[tv].tagLen;

            /* Initializes ClearText Message buffer */
            clearTextMessage.address = clearTextMessageBuf;
            clearTextMessage.byteSize = messageToDecrypt.byteSize;

            status = ECC_ecies_decryption(&PrivKey_st, &encryptedMessage,
                                          &eciesSharedData, &clearTextMessage, verbose);

            if (ECC_SUCCESS == status)
            {
                /* Compare Expected ClearText from test vector with computed one */

                check = memcmp(clearTextMessage.address, eciesDecryptTestVect_array[tv].ExpectedMessage, eciesDecryptTestVect_array[tv].messageLen);
                if(verbose)
                {
                    printf("\nClear Text check: %d, (expected: 1)\n", check == 0);
                }
            }
            else
            {
                if(verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }

            failed += ((status != ECC_SUCCESS) || (check != 0));
        }
    }
    else
    {
        failed++;
    }

    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of ecc_ecies_encryption_test */

/**
* @brief      ECC ECIES encryption & decryption test
* @details    The function implements tests of following ECIES scheme:
*              - Diffie-Hellman for Key Agreement
*              - ANS X9.63 for Key Derivation Function
*              - AES-128 in CBC mode for Encryption and IV value at zero
*              - CMAC-AES-128 for CMAC tag computation
*              - Elliptic Curve Cryptography with NIST P256 curve
* @note    The test performs encryption and decryption of an input message with the recipient's key pair. Sender's
*          Ephemeral Key material is randomly initialized by the test application.
* @note    The test chains encryption and decryption functions. The test is done only on the decrypted message which must be identical
*          to the input message. Tests of intermediate encryption step (Ephemeral Public Key, Tag) are not done.
*
* @param[in]     verbose
*
*/
uint32_t ecc_ecies_encryptionDecryption_test(uint32_t verbose)
{
    uint32_t status = 0U;

    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt decryptedMessage;

    struct ECIES_sharedData_stt eciesSharedData;
    struct ECIES_sharedData1_stt eciesSharedData1;
    struct ECIES_sharedData2_stt eciesSharedData2;
    struct ECIES_encMsg_stt encryptedMessage;

    struct ECCpubKeyByteArray_stt FmrPubKey_st;    /* Structure for the Encryption Ephemeral Public Key */
    struct ECCprivKeyByteArray_stt PrivKey_st;     /* Structure that will contain the private key */
    struct ECCpubKeyByteArray_stt PubKey_st;       /* Structure that will contain the public key */

    uint8_t senderFmrPubKeyX[C_MAX_PUB_KEY_SIZE];      /* Buffer that will receive the Encryption Ephemeral Public Key X coordinate */
    uint8_t senderFmrPubKeyY[C_MAX_PUB_KEY_SIZE];      /* Buffer that will receive the Encryption Ephemeral Public Key y coordinate */

    uint8_t encryptedMessageBuf[C_MAX_MESSAGE_LEN] = {0};    /* Buffer that will receive the ECIES Encrypted Message  */
    uint8_t decryptedMessageBuf[C_MAX_MESSAGE_LEN] = {0};    /* Buffer that will receive the ECIES Decrypted Message  */

    uint8_t messageTag[C_MAX_TAG_SIZE];                /* Buffer that will receive the message tag */

    uint32_t check = 0U;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

    if(verbose)
    {
        printf("\n");
    }

    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        for(tv = 0U; tv < NB_OF_ECIES_ENC_DEC_TV; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC ECIES Encryption Decryption with ");
                printf("test vector #1.%d: \n", (tv + 1));
            }

            /* Initializes Recipient's Public Key structure with test vectors */
            PubKey_st.pmPubKeyCoordX = eciesEncryptDecryptTestVect_array[tv].recipientPublicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = eciesEncryptDecryptTestVect_array[tv].recipientPublicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            /* Initializes Recipient's Private Key structure with test vectors */
            PrivKey_st.pmPrivKey = eciesEncryptDecryptTestVect_array[tv].recipientPrivateKey;
            PrivKey_st.mPrivKeySize = C_P256_MOD_SIZE;

            /* Initializes Message to encrypt from test vector */
            messageToEncrypt.address = eciesEncryptDecryptTestVect_array[tv].message;
            messageToEncrypt.byteSize = eciesEncryptDecryptTestVect_array[tv].messageLen;

            /* Initializes Shared Data with test vectors */
            eciesSharedData1.pSharedData1 = (uint8_t *)eciesEncryptDecryptTestVect_array[tv].sharedData1;
            eciesSharedData1.sharedData1Size = eciesEncryptDecryptTestVect_array[tv].sharedData1Len;
            eciesSharedData2.pSharedData2 = (uint8_t *)eciesEncryptDecryptTestVect_array[tv].sharedData2;
            eciesSharedData2.sharedData2Size = eciesEncryptDecryptTestVect_array[tv].sharedData2Len;

            eciesSharedData.pSharedData1_stt = &eciesSharedData1;
            eciesSharedData.pSharedData2_stt = &eciesSharedData2;

            /* Initializes structure that will receive Encrypted message */
            encryptedMessage.pEciesEncryptedMessage = encryptedMessageBuf;
            encryptedMessage.eciesEncryptedMessageSize = eciesEncryptDecryptTestVect_array[tv].messageLen;
            encryptedMessage.pTag = messageTag;
            encryptedMessage.tagSize = C_MAX_TAG_SIZE;

            /* Initialize locations that will receive the Sender Ephemeral public Key */
            FmrPubKey_st.pmPubKeyCoordX = senderFmrPubKeyX;
            FmrPubKey_st.pmPubKeyCoordY = senderFmrPubKeyY;
            FmrPubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            FmrPubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;
            encryptedMessage.pSenderFmrPubKey = &FmrPubKey_st;

            if (verbose)
            {
                printf("Encryption.\n");
            }
            status = ECC_ecies_encryption(&PubKey_st, &messageToEncrypt,
                                          &eciesSharedData, &encryptedMessage, verbose);
            if (ECC_SUCCESS == status)
            {
                /* Launch Decryption */
                if (verbose)
                {
                    printf("\nDecryption.\n");
                }
                decryptedMessage.address = decryptedMessageBuf;
                decryptedMessage.byteSize = C_MAX_MESSAGE_LEN;
                status = ECC_ecies_decryption(&PrivKey_st, &encryptedMessage,
                                              &eciesSharedData, &decryptedMessage, verbose);

                if (ECC_SUCCESS == status)
                {
                    /* Compare Expected Expanded Key Data from test vector with computed one */
                    check = memcmp(decryptedMessage.address, eciesEncryptDecryptTestVect_array[tv].message, eciesEncryptDecryptTestVect_array[tv].messageLen);
                    if(verbose)
                    {
                        printf("\nClear Text check: %d, (expected: 1)\n", check == 0);
                    }
                }
                else
                {
                    if(verbose)
                    {
                        printf("\nExecution Failed.\n");
                    }
                }
            }

            failed += ((status != ECC_SUCCESS) || (check != 0));

        }
    }
    else
    {
        failed++;
    }
    /* return 1 if successful (no test failed) */

    return(failed == 0);

} /* End of ecc_ecies_encryption_test */
/**
 * @}
 */

