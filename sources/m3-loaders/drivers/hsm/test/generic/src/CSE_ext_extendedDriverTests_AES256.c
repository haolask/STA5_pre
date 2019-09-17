/*
 * CSE_ext_extendedDriverTests_AES256.c
 *
 *  Created on: 18 juil. 2017
 *      Author: pierre guillemin
 */

/* The file provides a series of tests for AES256 ECB, CBC CCM, CMAC and GCM with only one test vector per modes.
 * It uses RAM key. */

#include <string.h>        /* For memcmp */

#include "config.h"
#include "serialprintf.h"

#include "cse_types.h"
#include "CSE_ext_extendedDriverTestsFuncList.h"
#include "err_codes.h"

#include "CSE_ext_AES256.h"
#include "CSE_ext_AES256_tests.h"
#include "CSE_ext_AES256_CCM_VADT_TV.h"
#include "CSE_ext_AES256_CMAC_cmac_TV.h"
#include "CSE_ext_AES256_GCM.h"

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

#define C_RESULT_BUFFER_SIZE 40


/* GCM Test vector coming from NIST CAVS 14. NIST file */
    ALIGN const aes256Gcm_encrypt_testVect_stt enc_test_vect =
    { /* TV 174 - -1 */
        256,
        96,
        408,
        720,
        64,
        {
        { /* TV 174 - 0 */
            //key
            {0xe8,0xe8,0xf3,0xbf,0xc7,0x8d,0x76,0xdc,0x91,0x48,0xbf,0x57,0x13,0x07,0x5a,0xa6,
             0xc8,0xa1,0x9c,0xf5,0x68,0x57,0x5c,0xc9,0x21,0xd7,0x3c,0x59,0x96,0xdb,0xd7,0x41},
            //iv
            {0xaf,0xed,0x0a,0x33,0xe6,0x1b,0x7f,0x28,0x68,0x83,0x03,0x96},
            //pt
            {0x76,0xa0,0xd8,0x8c,0x69,0x03,0x6f,0x3c,0xa6,0x13,0x3b,0x1c,0xd4,0x0f,0xb7,0x56,
             0xf9,0x3d,0x92,0xe4,0x33,0xc6,0xb6,0x62,0x23,0x59,0xed,0xf9,0x70,0x4b,0x28,0x2e,
             0x2a,0x25,0x0a,0x7e,0xd0,0x01,0xd2,0xb1,0xf4,0x57,0xab,0x27,0x1a,0xb0,0xf9,0x2a,
             0x9c,0xaf,0x92},
            //aad
            {0x90,0xcf,0x49,0x2c,0xae,0xa8,0x4b,0x1b,0x3d,0x08,0x6c,0xf5,0x8a,0xc7,0xac,0xaa,
             0xaa,0x78,0x4c,0x8b,0x75,0x89,0x34,0x89,0xb7,0x10,0x02,0x21,0x76,0xab,0x9b,0x20,
             0x23,0x4b,0x08,0x45,0x26,0x97,0xa3,0x25,0xe3,0x22,0x5b,0x1e,0x56,0x5c,0x21,0x11,
             0x0c,0xec,0xe6,0x57,0x28,0x85,0xf2,0x9e,0x38,0x4f,0x32,0x2c,0x69,0xa2,0xfd,0x1b,
             0xf3,0xb9,0x2c,0x6a,0x86,0x4d,0x55,0x74,0x60,0xa9,0xea,0xd4,0x20,0x52,0x83,0x3b,
             0x6f,0xa3,0x91,0x7e,0xd2,0xa2,0xbf,0xa3,0x9a,0x8e},
            //ct
            {0x5b,0x24,0xa6,0xf9,0x80,0x91,0x8d,0xb4,0x24,0xab,0x3d,0xe9,0xaa,0x37,0xc1,0xb1,
             0xad,0xd3,0x85,0xd4,0xda,0x48,0xdf,0xa3,0xe9,0x12,0xb2,0x4f,0x9e,0x8c,0x05,0xc9,
             0x24,0xf6,0xe5,0xa5,0x87,0xfe,0xd6,0x3b,0x17,0xd9,0x40,0x3d,0x28,0x6d,0xba,0x87,
             0x4e,0xab,0x4a},
            //tag
            {0xaa,0xe1,0xaf,0xe2,0x91,0xa6,0x8c,0xd4},
        },
        }
    };

    /* Test vector coming from NIST CAVS 14. NIST file */
    ALIGN const aes256Gcm_decrypt_testVect_stt dec_test_vect =
    { /* TV 174 - -1 */
        256,    //Keysize
        96,     //ivSize
        408,    //ptSize
        720,    //aadSize
        64,     //tagSize
        {
        { /* TV 174 - 0 */
            //key
            {0x34,0xb6,0xb6,0x27,0x69,0x2d,0xa6,0x39,0x69,0x02,0xc0,0xfa,0x17,0x62,0x26,0x78,
             0x06,0xc5,0xb1,0xa1,0x3b,0xa4,0x30,0x6e,0x3b,0xd7,0x4f,0x84,0xb4,0xbb,0x15,0x1b},
            //iv
            {0xb3,0x6b,0xaf,0x55,0x49,0xea,0xd0,0x77,0x78,0x29,0xd2,0x8b},
            //ct
            {0x37,0x2d,0xe9,0xd3,0x77,0x73,0x14,0x74,0x0e,0x42,0xd8,0x75,0xb5,0x89,0x07,0x73,
             0x78,0x1d,0xa9,0x47,0xc8,0xb1,0x5e,0x93,0xcf,0x6e,0x56,0x3c,0x8f,0x21,0x43,0x84,
             0x29,0x9e,0x05,0x54,0x2a,0xf1,0x55,0x6e,0xb2,0x62,0x67,0x97,0xa4,0x62,0x3b,0x14,
             0x0c,0x53,0xe9,},
            //aad
            {0xa4,0x7b,0x83,0x80,0x4e,0xbc,0xb8,0x2b,0x88,0xa9,0xfd,0x3d,0xfc,0x51,0x8b,0xb4,
             0xf3,0xbe,0xae,0xe9,0xef,0x06,0x5b,0xd9,0x31,0x64,0x7f,0x3c,0x4b,0x2f,0x40,0xe6,
             0xf4,0x22,0x93,0x74,0x3a,0x98,0x4d,0xc8,0x8e,0xd2,0x26,0x08,0x28,0x94,0xaf,0x0e,
             0xc8,0xdb,0xc1,0xea,0x0d,0x8c,0xf2,0xf0,0x5a,0x95,0x74,0xdb,0x61,0x0a,0xcc,0x8b,
             0x7d,0xaa,0xe6,0x57,0x74,0xa5,0x2f,0x1a,0x7b,0xe0,0x87,0x1e,0x5f,0x7d,0xd7,0x96,
             0x4c,0x3f,0x2b,0xb2,0x0e,0x19,0x7c,0x36,0xe9,0x0c},
            //tag
            {0x79,0xbe,0xa7,0x6a,0x6f,0x9c,0xcd,0x28},
            //expected result
            C_GCM_DECRYPT_TEST_PASS,
            //pt
            {0xde,0x89,0x3f,0xf8,0x50,0x15,0x58,0x67,0x8c,0xcc,0xf4,0x25,0x0c,0x90,0xce,0x03,
             0x3c,0x2f,0xa3,0x6e,0x1f,0xbf,0x61,0x58,0x5e,0x49,0xe1,0xe3,0xec,0x28,0xf5,0xe4,
             0xc6,0x87,0xc8,0x89,0xe0,0x01,0x70,0xf7,0xbb,0x9e,0xca,0x6f,0x71,0x36,0x30,0xc5,
             0xe5,0x33,0x28},
        },
        }
    };

ALIGN uint8_t encryptedMessageBuf[51] = {0U};    /* Buffer that will receive the AES256 Encrypted Message */
ALIGN uint8_t decryptedMessageBuf[51] = {0U};    /* Buffer that will receive the AES256 Encrypted Message */
ALIGN uint8_t authTagBuf[16] = {0U};             /* Buffer that will receive the authentication tag */



uint32_t test_CSE_extendedDriver_AES256_ECB(uint32_t P_verbose)
{
    uint32_t retVal = 0U;
    uint32_t checkEnc = 0U;
    uint32_t checkDec = 0U;

    /* AES256 Encryption Decryption in ECB tests */
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt deCipherMessage;

    ALIGN struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint8_t encryptedMessageBuf[32] = {0U};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[32] = {0U};    /* Buffer that will receive the AES256 Decrypted Message  */

    if(P_verbose)
    {
        printf("AES256 Encryption Decryption in ECB tests !\n");
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.mSecretKeySize = 32;

    messageToEncrypt.address = (uint8_t*)plainText_tv1;
    messageToEncrypt.byteSize = 16;

    cipherMessage.address = encryptedMessageBuf;
    cipherMessage.byteSize = 16;

    deCipherMessage.address = decryptedMessageBuf;

    /* Launch AES256 encryption in ECB mode */
    retVal = aes256_ecb_encryption(&aes256SecretKey,
                                   &messageToEncrypt, &cipherMessage,
                                   1, P_verbose);

    if (CSE_NO_ERR == retVal)
    {
        /* Compare Expected ClearText from test vector with computed one */

        checkEnc = memcmp(encryptedMessageBuf, (uint8_t*)cipherText_tv1, 16);
        if(P_verbose)
        {
            printf("ECB Encryption test: Cipher Text check: %d, (expected: 1)\n", checkEnc == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    if (CSE_NO_ERR == retVal)
    {
        cipherMessage.address = cipherText_tv1;
        cipherMessage.byteSize = 16;

        decryptedMessageBuf[0] = 0;

        /* Launch AES256 decryption in ECB mode */
        retVal = aes256_ecb_decryption(&aes256SecretKey,
                                       &cipherMessage, &deCipherMessage,
                                       1, P_verbose);

        if (CSE_NO_ERR == retVal)
        {
            /* Compare Expected ClearText from test vector with computed one */
            checkDec = memcmp(decryptedMessageBuf, (uint8_t*)plainText_tv1, 16);
            if(P_verbose)
            {
                printf("ECB Decryption test: Clear Text check: %d, (expected: 1)\n", checkDec == 0);
            }
        }
        else
        {
            if(P_verbose)
            {
                printf("Execution Failed.\n");
            }
        }
    }

    return ((CSE_NO_ERR == retVal) && (0U == checkEnc) && (0U == checkDec));

} /* End of test_CSE_extendedDriver_AES256_ECB */

uint32_t test_CSE_extendedDriver_AES256_CBC(uint32_t P_verbose)
{
    uint32_t retVal = 0U;
    uint32_t checkEnc = 0U;
    uint32_t checkDec = 0U;

    /* AES256 Encryption Decryption in ECB tests */
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt deCipherMessage;
    struct ByteArrayDescriptor_stt iv;

    struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */

    uint8_t encryptedMessageBuf[32] = {0U};    /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[32] = {0U};    /* Buffer that will receive the AES256 Decrypted Message  */

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 Encryption Decryption in CBC tests !\n");
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)key_tv1;
    aes256SecretKey.mSecretKeySize = 32;

    /* Initialize Initialization Vector */
    iv.address = iV_tv1;
    iv.byteSize = 16;

    messageToEncrypt.address = (uint8_t*)plainText_tv1;
    messageToEncrypt.byteSize = 16;

    cipherMessage.address = encryptedMessageBuf;
    cipherMessage.byteSize = 16;

    deCipherMessage.address = decryptedMessageBuf;

    /* Launch AES256 encryption in CBC mode */
    retVal = aes256_cbc_encryption(&aes256SecretKey,
                                   &messageToEncrypt, &cipherMessage,
                                   &iv, 1, P_verbose);

    if (CSE_NO_ERR == retVal)
    {
        /* Compare Expected ClearText from test vector with computed one */

        checkEnc = memcmp(encryptedMessageBuf, (uint8_t*)cipherText_tv1, 16);
        if(P_verbose)
        {
            printf("CBC Encryption test: Cipher Text check: %d, (expected: 1)\n", checkEnc == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    if (CSE_NO_ERR == retVal)
    {

        cipherMessage.address = cipherText_tv1;
        cipherMessage.byteSize = 16;

        decryptedMessageBuf[0] = 0;

        /* Launch AES256 decryption in CBC mode */
        retVal = aes256_cbc_decryption(&aes256SecretKey,
                                       &cipherMessage, &deCipherMessage,
                                       &iv, 1, P_verbose);

        if (CSE_NO_ERR == retVal)
        {
            /* Compare Expected ClearText from test vector with computed one */
            checkDec = memcmp(decryptedMessageBuf, (uint8_t*)plainText_tv1, 16);
            if(P_verbose)
            {
                printf("CBC Decryption test: Clear Text check: %d, (expected: 1)\n", checkDec == 0);
            }
        }
        else
        {
            if(P_verbose)
            {
                printf("Execution Failed.\n");
            }
        }
    }

    return ((CSE_NO_ERR == retVal) && (0U == checkEnc) && (0U == checkDec));

} /* End of test_CSE_extendedDriver_AES256_CBC */

uint32_t test_CSE_extendedDriver_AES256_CCM(uint32_t P_verbose)
{
    uint32_t retVal = 0U;
    uint32_t checkEnc = 1U;        /* Set to error */
    uint32_t checkDec = 1U;        /* Set to error */

    struct AES_CCM_msgData_stt msg;                    /* Message to cipher */
    struct AES_CCM_headerData_stt headerData;              /* Header data, Nonce and associated data */
    struct AES_CCM_msgData_stt cipheredMsg;            /* Location for the ciphered message */
    struct AES_CCM_deciphMsgData_stt payloadMsg;   /* Location for the deciphered message, the payload */

    uint8_t encryptedMessageBuf[C_RESULT_BUFFER_SIZE] = {0U};          /* Buffer that will receive the AES256 Encrypted Message  */
    uint8_t decryptedMessageBuf[C_RESULT_BUFFER_SIZE] = {0U};          /* Buffer that will receive the AES256 Decrypted Message  */

    struct AES256secretKeyArray_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */

    /* Test vector coming from NIST CAVS 11.0 NIST file */

    /* Alen[] = 32 */
    ALIGN const unsigned char aes256ccm_Key_Alen32_buf[C_AES256_KEY_SIZE] = {
    0x2e,0x6e,0x34,0x07,0x0c,0xaf,0x1b,0x88,0x20,0xed,0x39,0xed,0xfa,0x83,0x45,0x9a,0xbe,0x1c,0x15,0xa1,0x82,0x7f,0x1c,0x39,0xf7,0xac,0x31,0x6c,0x4c,0x27,0x91,0x0f };
    ALIGN const unsigned char aes256ccm_Nonce_Alen32_buf[C_NLEN_VADT_SIZE] = {
    0xc4,0x9c,0xce,0xf8,0x69,0xbb,0x86,0xd2,0x19,0x32,0xcb,0x44,0x3b };

    /*  Count 320 */
    ALIGN const unsigned char aes256ccm_Adata_TV320_buf[] = {
    0xd3,0x7e,0x35,0xd7,0xcd,0xcc,0xd9,0x82,0x4a,0x1a,0xe4,0xc7,0x87,0x81,0x97,0x35,0xe4,0xaf,0x79,0x8a,0x3b,0xeb,0x49,0xd4,0x70,0x53,0x36,0xd6,0x49,0x68,0x53,0xad };
    ALIGN const unsigned char aes256ccm_Payload_TV320_buf[C_PLEN_VADT_SIZE] = {
    0x77,0x1a,0x7b,0xaa,0x9c,0xf8,0x3a,0xa2,0x53,0x34,0x9f,0x64,0x75,0xd5,0xe7,0x4d,0xba,0x45,0x25,0x30,0x7b,0x02,0x2b,0xa7 };
    ALIGN const unsigned char aes256ccm_CT_TV320_buf[C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE] = {
    0xee,0xba,0xc2,0x47,0x50,0x04,0x97,0x00,0x71,0xdf,0xa2,0xcf,0xb8,0x55,0xc4,0xe7,0x8b,0x1a,0xdd,0x8d,0xcb,0xcc,0xfc,0x0b,0xd6,0xb1,0x40,0x27,0x32,0x4b,0x65,0x7a,0x56,0x26,0x3d,0xf1,0x48,0x66,0x53,0x93 };

    const aes256ccm_vadt_test_vect_stt test_vect =
    {
        32,
        aes256ccm_Key_Alen32_buf,
        aes256ccm_Nonce_Alen32_buf,
        {
            {aes256ccm_Adata_TV320_buf, aes256ccm_Payload_TV320_buf, aes256ccm_CT_TV320_buf},
        },
    };

    aes256ccm_vadt_test_vect_stt* ptest_vect = (aes256ccm_vadt_test_vect_stt*)&test_vect;
    /* ptest_vect = &aes256ccm_vadt_test_vect[32];*/


    if(P_verbose)
    {
        printf("\n");
        printf("AES256 CCM mode Encryption Decryption tests !\n");
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)ptest_vect->aes256ccm_Key_Alen;
    aes256SecretKey.mSecretKeySize = 32;

    /* Initialize Nonce */
    headerData.pNonce = (unsigned char *)ptest_vect->aes256ccm_Nonce_Alen;
    headerData.nonceByteSize = C_NLEN_VADT_SIZE;

    if (P_verbose)
    {
        printf("\nSize of Associated Data = %d\n", ptest_vect->AlenSize);
    }

    /* Erase result buffers */
    uint32_t j = 0U;
    for (j = 0; j < C_RESULT_BUFFER_SIZE; j++)
    {
        encryptedMessageBuf[j] = 0U;
        decryptedMessageBuf[j] = 0U;
    }

    /* Initialize Associated Data */
    headerData.pData = (unsigned char *)ptest_vect->testCcmVadt[0].pAdata;
    headerData.DataByteSize = ptest_vect->AlenSize;

    msg.pAddress = (unsigned char *)ptest_vect->testCcmVadt[0].pPayload;
    msg.messageByteSize = C_PLEN_VADT_SIZE;

    cipheredMsg.pAddress = encryptedMessageBuf;
    cipheredMsg.messageByteSize = C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE;    /* Expected size of cipher Text, size allocated to result buffer */

    payloadMsg.pDecipheredMessage = decryptedMessageBuf;
    payloadMsg.decipheredmessageByteSize = C_PLEN_VADT_SIZE;
    payloadMsg.authResult = AUTHENTICATION_FAILED;

    /* AES256 CCM Encryption */
    retVal = aes256_ccm_encryption(&aes256SecretKey, &headerData, C_TLEN_VADT_SIZE, &msg,
                                   &cipheredMsg, P_verbose);

    if (CSE_NO_ERR == retVal)
    {
        /* Compare Expected CipherText from test vector with computed one */
        checkEnc = memcmp(cipheredMsg.pAddress, (uint8_t*)ptest_vect->testCcmVadt[0].pCT, C_PLEN_VADT_SIZE + C_TLEN_VADT_SIZE);
        if(P_verbose)
        {
            printf("\nCCM Encryption test: Cipher Text check: %d, (expected: 1)\n", checkEnc == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    if (CSE_NO_ERR == retVal)
    {
        /* AES256 CCM Decryption */
        retVal = aes256_ccm_decryption(&aes256SecretKey, &headerData, C_TLEN_VADT_SIZE, &cipheredMsg,
                                       &payloadMsg, P_verbose);
        if (CSE_NO_ERR == retVal)
        {
            /* Compare Expected ClearText from test vector with computed one */

            checkDec = memcmp(payloadMsg.pDecipheredMessage, (uint8_t*)(ptest_vect->testCcmVadt[0].pPayload), C_PLEN_VADT_SIZE);
            if(P_verbose)
            {
                printf("\nCCM Decryption test: Clear Text check: %d, (expected: 1)\n", checkDec == 0);
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

    return ((CSE_NO_ERR == retVal) && (0U == checkEnc) && (0U == checkDec));

} /* End of test_CSE_extendedDriver_AES256_CCM */

uint32_t test_CSE_extendedDriver_AES256_CMAC(uint32_t P_verbose)
{
    uint32_t retVal = 0U;
    uint32_t checkEnc = 1U;        /* Set to error */

    struct AES_CMAC_msg_stt msg;           /* Message on which CMAC tag is generated */
    struct AES_CMAC_tag_stt cmacTag;           /* CMAC Tag */

    uint8_t cmacTagBuf[40] = {0U};           /* Buffer that receive the generated CMAC Tag */

    struct AES256secretKeyArray_stt aes256SecretKey;    /* Structure for the AES256 Secret Key */
    uint32_t test_msgSize;
    uint32_t test_tagSize;
    aes256CmacTestVect_stt* ptest_vector;

    /* Test vector coming from NIST CAVP 11.0 cmactestvectors.zip */
    ALIGN const unsigned char aes256CmacKey_TV72[] = {
    0x2f,0x4a,0x65,0x01,0xd8,0xfe,0x7b,0x65,0xf6,0x07,0x75,0x7d,0xdf,0xf6,0xed,0x87,0xae,0x06,0x81,0xb9,0x8b,0x53,0x33,0x1d,0x2d,0x46,0x10,0x9f,0x9c,0x54,0x10,0x65 };
    ALIGN const unsigned char aes256CmacMsg_TV72[] = {
    0x4f,0xa9,0xac,0x1b,0x54,0x4a,0xfc,0xd8,0x5a,0xc3,0x2a,0xc0,0x90,0x9c,0x74 };
    ALIGN const unsigned char aes256CmacMac_TV72[] = {
    0xc0,0x2e,0x8b,0x66,0xf9,0xfc,0x26,0x3b,0x8f,0xb0 };

    const aes256CmacTestVect_stt aes256cmac_test_vect_11_buffers =
    {
        aes256CmacKey_TV72, aes256CmacMsg_TV72, aes256CmacMac_TV72
    };

    test_msgSize = 15;
    test_tagSize = 10;
    ptest_vector = (aes256CmacTestVect_stt*)(&aes256cmac_test_vect_11_buffers);

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 CMAC Tag Generation and Verification tests !\n");
    }

    if (P_verbose)
    {
        printf("\nSize of Message = %d\n", test_msgSize);
        printf("Size of requested CMAC Tag = %d\n", test_tagSize);
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)(ptest_vector->pKey);
    aes256SecretKey.mSecretKeySize = 32;

    msg.pAddress = (unsigned char *)ptest_vector->pMsg;
    msg.messageByteSize = test_msgSize;

    cmacTag.pAddress = cmacTagBuf;

    /* AES256 CMAC Tag Generation */
    retVal = aes256_cmac_tagGeneration(&aes256SecretKey, test_tagSize, &msg, &cmacTag, P_verbose);
    if (CSE_NO_ERR == retVal)
    {
        /* Compare Expected CipherText from test vector with computed one */
        checkEnc = memcmp(cmacTag.pAddress, (uint8_t*)ptest_vector->pCmacTag, test_tagSize);
        if(P_verbose)
        {
            printf("\nCMAC Tag Generation test: Cipher Text check: %d, (expected: 1)\n", checkEnc == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    if (CSE_NO_ERR == retVal)
    {
        /* Test CMAC Tag verification on same test vectors */
        /* Initialize secret key */
        aes256SecretKey.pmSecretKey = (uint8_t*)ptest_vector->pKey;
        aes256SecretKey.mSecretKeySize = 32;

        msg.pAddress = (unsigned char *)ptest_vector->pMsg;
        msg.messageByteSize = test_msgSize;

        cmacTag.pAddress = (uint8_t *)ptest_vector->pCmacTag;

        /* AES256 CMAC Tag Verification */
        retVal = aes256_cmac_tagVerification(&aes256SecretKey, test_tagSize, &msg, &cmacTag, P_verbose);
        if(P_verbose)
        {
            printf("CMAC Tag Verification check: %d (expected: 1),\n", (retVal == AUTHENTICATION_SUCCESSFUL));
        }
    }

    return ((AUTHENTICATION_SUCCESSFUL == retVal) && (0U == checkEnc));

} /* End of test_CSE_extendedDriver_AES256_CMAC */

uint32_t test_CSE_extendedDriver_AES256_GCM(uint32_t P_verbose)
{
    uint32_t retVal = 0U;
    uint32_t checkEnc = 1U;        /* Set to error */
    uint32_t checkDec = 1U;        /* Set to error */

    struct AES_GCM_msgData_stt msg;                        /* Message to cipher */
    struct AES_GCM_headerData_stt headerData;                  /* Header data, Nonce and associated data */
    struct AES_GCM_authCiphMsg_stt authCipheredMsg;        /* GCM authenticated ciphered message */
    struct AES_GCM_deciphMsgData_stt deCipheredMsg;    /* GCM deciphered message */


    struct AES256secretKeyArray_stt aes256SecretKey;   /* Structure for the AES256 Secret Key */



    aes256Gcm_encrypt_testVect_stt* pgcm_enc_vect = (aes256Gcm_encrypt_testVect_stt*)&enc_test_vect;
    aes256Gcm_decrypt_testVect_stt* pgcm_dec_vect = (aes256Gcm_decrypt_testVect_stt*)&dec_test_vect;
    /* pgcm_enc_vect = &aes256Gcm_encrypt_testVect_array[174];*/
    /* pgcm_dec_vect = &aes256Gcm_decrypt_testVect_array[174];*/

    if(P_verbose)
    {
        printf("\n");
        printf("AES256 GCM mode Authenticated Encryption tests !\n");
    }

    if (P_verbose)
    {
        printf("Size of Initialization Vector = %d\n", pgcm_enc_vect->ivSize);
        printf("Size of PlainText = %d\n", pgcm_enc_vect->ptSize);
        printf("Size of Additional Authentication Data = %d\n", pgcm_enc_vect->aadSize);
        printf("Size of Authentication Tag = %d\n", pgcm_enc_vect->tagSize);
    }

    /* Initialize secret key */
    aes256SecretKey.pmSecretKey = (uint8_t*)pgcm_enc_vect->aes256GcmEncrypt_TV[0].key;
    aes256SecretKey.mSecretKeySize = 32;

    /* Initialize IV */
    headerData.pIv = (unsigned char *)pgcm_enc_vect->aes256GcmEncrypt_TV[0].iv;
    headerData.ivByteSize = pgcm_enc_vect->ivSize / 8;

    /* Initialize Additional Authentication Data */
    headerData.pAdditionalAuthenticationData = (unsigned char *)pgcm_enc_vect->aes256GcmEncrypt_TV[0].aad;
    headerData.AdditionalAuthenticationDataByteSize = pgcm_enc_vect->aadSize / 8;

    msg.pAddress = (unsigned char *)pgcm_enc_vect->aes256GcmEncrypt_TV[0].pt;
    msg.messageByteSize = pgcm_enc_vect->ptSize / 8;

    authCipheredMsg.pAddress = encryptedMessageBuf;
    authCipheredMsg.messageByteSize = 0U;
    authCipheredMsg.pAuthTag = authTagBuf;
    authCipheredMsg.authTagByteSize = 0U;

    /* AES256 GCM Encryption */
    retVal = aes256_gcm_authenticatedEncryption(&aes256SecretKey, &headerData, pgcm_enc_vect->tagSize / 8,
                                                &msg, &authCipheredMsg, P_verbose);
    if (CSE_NO_ERR == retVal)
    {
        /* Compare Expected Authenticated CipherText and Authentication Tag from test vector with computed ones */
        checkEnc = memcmp(authCipheredMsg.pAddress, (uint8_t*)pgcm_enc_vect->aes256GcmEncrypt_TV[0].ct, pgcm_enc_vect->ptSize / 8);
        checkEnc += memcmp(authCipheredMsg.pAuthTag, (uint8_t*)pgcm_enc_vect->aes256GcmEncrypt_TV[0].tag, pgcm_enc_vect->tagSize / 8);
        if(P_verbose)
        {
            printf("\nGCM Encryption test: Cipher Text check: %d, (expected: 1)\n", checkEnc == 0);
        }
    }
    else
    {
        if(P_verbose)
        {
            printf("\nExecution Failed.\n");
        }
    }

    if (CSE_NO_ERR == retVal)
    {
        if(P_verbose)
        {
            printf("AES256 GCM mode Authenticated Decryption tests !\n");
        }

        if (P_verbose)
        {
            printf("Size of Initialization Vector = %d\n", pgcm_dec_vect->ivSize);
            printf("Size of PlainText = %d\n", pgcm_dec_vect->ptSize);
            printf("Size of Additional Authentication Data = %d\n", pgcm_dec_vect->aadSize);
            printf("Size of Authentication Tag = %d\n", pgcm_dec_vect->tagSize);
        }

        /* Erase result buffers */
        {
            uint32_t j = 0U;
            for (j = 0; j < 51; j++)
            {
                decryptedMessageBuf[j] = 0U;
            }
        }

        /* Initialize secret key */
        aes256SecretKey.pmSecretKey = (uint8_t*)pgcm_dec_vect->aes256GcmDecrypt_TV[0].key;
        aes256SecretKey.mSecretKeySize = 32;

        /* Initialize IV */
        headerData.pIv = (unsigned char *)pgcm_dec_vect->aes256GcmDecrypt_TV[0].iv;
        headerData.ivByteSize = pgcm_dec_vect->ivSize / 8;

        /* Initialize Additional Authentication Data */
        headerData.pAdditionalAuthenticationData = (unsigned char *)pgcm_dec_vect->aes256GcmDecrypt_TV[0].aad;
        headerData.AdditionalAuthenticationDataByteSize = pgcm_dec_vect->aadSize / 8;

        authCipheredMsg.pAddress = (unsigned char *)pgcm_dec_vect->aes256GcmDecrypt_TV[0].ct;
        authCipheredMsg.messageByteSize = pgcm_dec_vect->ptSize / 8;
        authCipheredMsg.pAuthTag = (unsigned char *)pgcm_dec_vect->aes256GcmDecrypt_TV[0].tag;
        authCipheredMsg.authTagByteSize = pgcm_dec_vect->tagSize / 8;

        deCipheredMsg.pDecipheredMessage = decryptedMessageBuf;
        deCipheredMsg.decipheredmessageByteSize = 0;
        deCipheredMsg.authResult = AUTHENTICATION_FAILED;

        /* AES256 GCM Decryption */
        retVal = aes256_gcm_authenticatedDecryption(&aes256SecretKey, &headerData, pgcm_dec_vect->tagSize / 8,
                                                    &authCipheredMsg, &deCipheredMsg, P_verbose);

        if (CSE_NO_ERR == retVal)
        {
            if ((CSE_NO_ERR == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_PASS == pgcm_dec_vect->aes256GcmDecrypt_TV[0].expectedTestResult))
            {
                /* Compare Expected plainText from test vector with computed one */
                checkDec = memcmp(deCipheredMsg.pDecipheredMessage, (uint8_t*)pgcm_dec_vect->aes256GcmDecrypt_TV[0].pt, pgcm_dec_vect->ptSize / 8);
                if(P_verbose)
                {
                    printf("\nGCM Decryption test: Cipher Text check: %d, (expected: 1)\n", checkDec == 0);
                }
            }
            else
            {
                /* Verification is not correct, check expected result */
                if ((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == pgcm_dec_vect->aes256GcmDecrypt_TV[0].expectedTestResult))
                {
                    checkDec = 0;
                    checkDec = !((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == pgcm_dec_vect->aes256GcmDecrypt_TV[0].expectedTestResult));
                    if(P_verbose)
                    {
                        printf("\nGCM Decryption test: Authentication not verified: Authentication check: %d, (expected: 1)\n", checkDec == !((0x01U == deCipheredMsg.authResult) && (C_GCM_DECRYPT_TEST_FAIL == pgcm_dec_vect->aes256GcmDecrypt_TV[0].expectedTestResult)));
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf("\nGCM decryption Failed.\n");
                    }
                    checkDec = 1U;
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
    }

    return ((CSE_NO_ERR == retVal) && (0U == checkEnc) && (0U == checkDec));

} /* End of test_CSE_extendedDriver_AES256_GCM */
