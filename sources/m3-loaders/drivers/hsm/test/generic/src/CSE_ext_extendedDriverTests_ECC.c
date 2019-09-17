/*
 * CSE_ext_extendedDriverTests_ECC.c
 *
 *  Created on: 20 juil. 2017
 *      Author: pierre guillemin
 */

/* The file provides a series of tests for ECC with only one test vector per curves.
 * It make one test for NIST P256, P384, P521 and Brainpool P256r1 and Brainpool P384r1
 * It uses RAM key. */

#include <string.h>        /* For memcmp */

#include "config.h"
#include "serialprintf.h"

#include "cse_types.h"
#include "CSE_ext_extendedDriverTestsFuncList.h"
#include "err_codes.h"

#include "CSE_ext_ECC.h"
#include "CSE_ext_ECC_ECIES_test.h"
#include "CSE_RNG.h"
#include "CSE_ext_ECC_ECDSA_SignGenVerif_TV.h"
#include "CSE_ext_ECC_ECIES_TV.h"

extern const signVerify_test_vect_stt test_CSE_ECDSAsignVerify_TV[8];
extern const verify_test_vect_stt test_CSE_ECDSAsignatureVerify_TV[8];
extern const ecies_encryptDecrypt_testVect_stt test_CSE_eciesEncryptDecryptTestVect[1];
extern const ecies_decrypt_testVect_stt test_CSE_eciesDecryptTestVect[1];

static uint32_t test_CSE_extendedDriver_ECC_ECDSAsignVerify(uint32_t P_verbose, uint32_t P_curveIDToTest)
{
    uint32_t retVal = 0U;
    uint32_t tv = 0U;
    uint32_t failed = 0U;
    uint32_t modSize = 0U;

    struct ECCpubKeyByteArray_stt PubKey_st;    /* Structure that will contain the public key */
    struct ECCprivKeyByteArray_stt PrivKey_st;  /* Structure that will contain the private key*/

    struct ECDSA_Signature_stt ECDSA_Signature;
    uint8_t signatureR[C_MAX_SIG_SIZE];    /* Buffer that will contain R field of ECDSA signature */
    uint8_t signatureS[C_MAX_SIG_SIZE];    /* Buffer that will contain S field of ECDSA signature */

    uint32_t currentECId = 0U;

    /* Initializes ECDSA structure for signature (r, s) */
    ECDSA_Signature.pSigR = signatureR;
    ECDSA_Signature.pSigS = signatureS;
    ECDSA_Signature.sigR_size = 0U;
    ECDSA_Signature.sigS_size = 0U;


    if(P_verbose)
    {
        printf("\n");
        printf("The test generates a signature of the provided message and verify the generated signature.\n");
        printf("It used randomly selected key materials.\n");
    }

    retVal = CSE_ECC_changeCurve(P_curveIDToTest);
    if( CSE_NO_ERR != retVal)
    {
        if(P_verbose)
        {
            printf("\n");
            printf("Problem when attempting to select curveID %0x08x\n", P_curveIDToTest);
        }
    }
    else
    {
        for(tv = 0U; tv < 8; tv++)
        {
            /* Only perform operation if TV curve is the one we want to test */
            if( P_curveIDToTest == test_CSE_ECDSAsignVerify_TV[tv].ecType)
            {
                retVal = CSE_ECC_getECcurve(&currentECId);

                if(P_verbose)
                {
                    printf("Current Elliptic Curve is: ");
                }
                switch(currentECId)
                {
                case C_NIST_P_256:
                    modSize = C_P256_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("NIST P_256 !\n");
                    }
                    break;
#ifdef INCLUDE_NIST_P384
                case C_NIST_P_384:
                    modSize = C_P384_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("NIST P_384 !\n");
                    }
                    break;
#endif
#ifdef INCLUDE_NIST_P521
                case C_NIST_P_521:
                    modSize = C_P521_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("NIST P_521 !\n");
                    }
                    break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
                case C_BRAINPOOL_P256R1:
                    modSize = C_P256_MOD_SIZE;

                    if(P_verbose)
                    {
                        printf("Brainpool P256r1 !\n");
                    }
                    break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
                case C_BRAINPOOL_P384R1:
                    modSize = C_P384_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("Brainpool P384r1 !\n");
                    }
                    break;
#endif
                default:
                    if (P_verbose)
                    {
                        printf("Unknown!\n");
                    }
                    break;
                }

                PubKey_st.mPubKeyCoordXSize = modSize;
                PubKey_st.mPubKeyCoordYSize = modSize;
                PrivKey_st.mPrivKeySize = modSize;
                ECDSA_Signature.sigR_size = modSize;
                ECDSA_Signature.sigS_size = modSize;

                if(P_verbose)
                {
                    printf("Testing with ");
                    if (test_CSE_ECDSAsignVerify_TV[tv].hashType == E_SHA256)
                    {
                        printf("SHA256");
                    }
                    else
                    {
                        if (test_CSE_ECDSAsignVerify_TV[tv].hashType == E_SHA384)
                        {
                            printf("SHA384");
                        }
                        else
                        {
                            if (test_CSE_ECDSAsignVerify_TV[tv].hashType == E_SHA512)
                            {
                                printf("SHA512");
                            }
                        }
                    }
                    if(P_verbose)
                    {
                        printf(" ECDSA test vector #1.%d: \n", (tv + 1));
                    }
                }

                /* Initializes Public and Private Key structures with test vectors */
                PubKey_st.pmPubKeyCoordX = test_CSE_ECDSAsignVerify_TV[tv].publicXKey;
                PubKey_st.pmPubKeyCoordY = test_CSE_ECDSAsignVerify_TV[tv].publicYKey;

                PrivKey_st.pmPrivKey = test_CSE_ECDSAsignVerify_TV[tv].privateKey;

                retVal = ECC_signGenerate_ECDSA(&PrivKey_st,
                                                test_CSE_ECDSAsignVerify_TV[tv].msg, C_TV_MSG_SIZE,
                                                test_CSE_ECDSAsignVerify_TV[tv].hashType, &ECDSA_Signature, P_verbose);
                /* Set Error Flag */
                failed += ((ECC_SUCCESS != retVal));

                if (ECC_SUCCESS == retVal)
                {
                    if (test_CSE_ECDSAsignVerify_TV[tv].ecType == C_NIST_P_521)
                    {
                        ECDSA_Signature.sigR_size = C_P521_MOD_SIZE;
                        ECDSA_Signature.sigS_size = C_P521_MOD_SIZE;
                    }

                    retVal = ECC_signVerify_ECDSA(&PubKey_st,
                                                  test_CSE_ECDSAsignVerify_TV[tv].msg, C_TV_MSG_SIZE,
                                                  test_CSE_ECDSAsignVerify_TV[tv].hashType,
                                                  &ECDSA_Signature, P_verbose);
                    /* Set error flag */
                    failed += (SIGNATURE_VALID != retVal);
                    if (P_verbose)
                    {
                        printf("Verification check: %d (expected: 1)\n", retVal == SIGNATURE_VALID);
                    }
                }
            }
        }
    }

    /* return 1 if successful (no test Failed) */
    return (failed == 0);

} /* End of test_CSE_extendedDriver_ECC_ECDSAsignVerify */

static uint32_t test_CSE_extendedDriver_ECC_ECDSAsignatureVerification(uint32_t P_verbose, uint32_t P_curveIDToTest)
{
    uint32_t status = 0U;
    struct ECCpubKeyByteArray_stt PubKey_st;    /* Structure that will contain the public key */
    struct ECDSA_Signature_stt ECDSA_Signature;

    uint32_t failed = 0U;
    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t modSize = 0U;
    uint32_t retVal;

    retVal = CSE_ECC_changeCurve(P_curveIDToTest);
    if( CSE_NO_ERR != retVal)
    {
        if(P_verbose)
        {
            printf("\n");
            printf("Problem when attempting to select curveID %0x08x\n", P_curveIDToTest);
        }
    }
    else
    {
        for(tv = 0U; tv < 8; tv++)
        {
            /* Only perform operation if TV curve is the one we want to test */
            if( P_curveIDToTest == test_CSE_ECDSAsignatureVerify_TV[tv].ecType)
            {
                status = CSE_ECC_getECcurve(&currentECId);
                if(P_verbose)
                {
                    printf("Current Elliptic Curve is: ");
                }
                switch(currentECId)
                {
                case C_NIST_P_256:
                    modSize = C_P256_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("NIST P_256 !\n");
                    }
                    break;
#ifdef INCLUDE_NIST_P384
                case C_NIST_P_384:
                    modSize = C_P384_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("NIST P_384 !\n");
                    }
                    break;
#endif
#ifdef INCLUDE_NIST_P521
                case C_NIST_P_521:
                    modSize = C_P521_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("NIST P_521 !\n");
                    }
                    break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
                case C_BRAINPOOL_P256R1:
                    modSize = C_P256_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("Brainpool P256r1 !\n");
                    }
                    break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
                case C_BRAINPOOL_P384R1:
                    modSize = C_P384_MOD_SIZE;
                    if(P_verbose)
                    {
                        printf("Brainpool P384r1 !\n");
                    }
                    break;
#endif
                default:
                    if (P_verbose)
                    {
                        printf("Unknown!\n");
                    }
                    break;
                }

                PubKey_st.mPubKeyCoordXSize = modSize;
                PubKey_st.mPubKeyCoordYSize = modSize;
                ECDSA_Signature.sigR_size = modSize;
                ECDSA_Signature.sigS_size = modSize;

                if(P_verbose)
                {
                    printf("Testing with ");
                    if (test_CSE_ECDSAsignatureVerify_TV[tv].hashType == E_SHA256)
                    {
                        printf("SHA256");
                    }
                    else
                    {
                        if (test_CSE_ECDSAsignatureVerify_TV[tv].hashType == E_SHA384)
                        {
                            printf("SHA384");
                        }
                        else
                        {
                            if (test_CSE_ECDSAsignatureVerify_TV[tv].hashType == E_SHA512)
                            {
                                printf("SHA512");
                            }
                        }
                    }

                    printf(" Expected test result: ");
                    if (test_CSE_ECDSAsignatureVerify_TV[tv].expectedResult == TEST_PASS)
                    {
                        printf("Signature verified! \n");
                    }
                    else
                    {
                        printf("Signature not verified! \n");
                    }
                }

                /* Initializes Public and Private Key structures with test vectors */
                PubKey_st.pmPubKeyCoordX = test_CSE_ECDSAsignatureVerify_TV[tv].publicXKey;
                PubKey_st.pmPubKeyCoordY = test_CSE_ECDSAsignatureVerify_TV[tv].publicYKey;

                /* Initializes ECDSA structure with expected signature (r, s) */
                ECDSA_Signature.pSigR = test_CSE_ECDSAsignatureVerify_TV[tv].sigR;
                ECDSA_Signature.pSigS = test_CSE_ECDSAsignatureVerify_TV[tv].sigS;

                status = ECC_signVerify_ECDSA(&PubKey_st, test_CSE_ECDSAsignatureVerify_TV[tv].msg, C_TV_MSG_SIZE,
                                              test_CSE_ECDSAsignatureVerify_TV[tv].hashType,
                                              &ECDSA_Signature, P_verbose);

                if(P_verbose)
                {
                    printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (test_CSE_ECDSAsignatureVerify_TV[tv].expectedResult == TEST_PASS));
                }
                if (((status == SIGNATURE_VALID) && (test_CSE_ECDSAsignatureVerify_TV[tv].expectedResult == TEST_PASS)) ||
                    ((status == SIGNATURE_INVALID) && (test_CSE_ECDSAsignatureVerify_TV[tv].expectedResult == TEST_FAIL)) ||
                    ((status == ECC_ERR_BAD_PARAMETER) && (test_CSE_ECDSAsignatureVerify_TV[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
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
    }

    /* return 1 if successful (no test Failed) */
    return (failed == 0);

} /* End of test_CSE_extendedDriver_ECC_ECDSAsignatureVerification */

static uint32_t test_CSE_extendedDriver_ECC_ECIESEncryptionDecryption(uint32_t P_verbose)
{
    uint32_t retVal = 0U;

    uint32_t check = 0U;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

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

    if(P_verbose)
    {
        printf("\n");
    }
    retVal = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == retVal)
    {
        if(P_verbose)
        {
            printf("Testing ECC ECIES Encryption Decryption with ");
            printf("test vector #1.%d: \n", (tv + 1));
        }

        /* Initializes Recipient's Public Key structure with test vectors */
        PubKey_st.pmPubKeyCoordX = test_CSE_eciesEncryptDecryptTestVect[tv].recipientPublicXKey;
        PubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
        PubKey_st.pmPubKeyCoordY = test_CSE_eciesEncryptDecryptTestVect[tv].recipientPublicYKey;
        PubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

        /* Initializes Recipient's Private Key structure with test vectors */
        PrivKey_st.pmPrivKey = test_CSE_eciesEncryptDecryptTestVect[tv].recipientPrivateKey;
        PrivKey_st.mPrivKeySize = C_P256_MOD_SIZE;

        /* Initializes Message to encrypt from test vector */
        messageToEncrypt.address = test_CSE_eciesEncryptDecryptTestVect[tv].message;
        messageToEncrypt.byteSize = test_CSE_eciesEncryptDecryptTestVect[tv].messageLen;

        /* Initializes Shared Data with test vectors */
        eciesSharedData1.pSharedData1 = (uint8_t *)test_CSE_eciesEncryptDecryptTestVect[tv].sharedData1;
        eciesSharedData1.sharedData1Size = test_CSE_eciesEncryptDecryptTestVect[tv].sharedData1Len;
        eciesSharedData2.pSharedData2 = (uint8_t *)test_CSE_eciesEncryptDecryptTestVect[tv].sharedData2;
        eciesSharedData2.sharedData2Size = test_CSE_eciesEncryptDecryptTestVect[tv].sharedData2Len;

        eciesSharedData.pSharedData1_stt = &eciesSharedData1;
        eciesSharedData.pSharedData2_stt = &eciesSharedData2;

        /* Initializes structure that will receive Encrypted message */
        encryptedMessage.pEciesEncryptedMessage = encryptedMessageBuf;
        encryptedMessage.eciesEncryptedMessageSize = test_CSE_eciesEncryptDecryptTestVect[tv].messageLen;
        encryptedMessage.pTag = messageTag;
        encryptedMessage.tagSize = C_MAX_TAG_SIZE;

        /* Initialize locations that will receive the Sender Ephemeral public Key */
        FmrPubKey_st.pmPubKeyCoordX = senderFmrPubKeyX;
        FmrPubKey_st.pmPubKeyCoordY = senderFmrPubKeyY;
        FmrPubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
        FmrPubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;
        encryptedMessage.pSenderFmrPubKey = &FmrPubKey_st;

        if (P_verbose)
        {
            printf("Encryption.\n");
        }
        retVal = ECC_ecies_encryption(&PubKey_st, &messageToEncrypt,
                                      &eciesSharedData, &encryptedMessage, P_verbose);
        if (ECC_SUCCESS == retVal)
        {
            /* Launch Decryption */
            if (P_verbose)
            {
                printf("\nDecryption.\n");
            }
            decryptedMessage.address = decryptedMessageBuf;
            decryptedMessage.byteSize = C_MAX_MESSAGE_LEN;
            retVal = ECC_ecies_decryption(&PrivKey_st, &encryptedMessage,
                                          &eciesSharedData, &decryptedMessage, P_verbose);

            if (ECC_SUCCESS == retVal)
            {
                /* Compare Expected Expanded Key Data from test vector with computed one */
                check = memcmp(decryptedMessage.address, test_CSE_eciesEncryptDecryptTestVect[tv].message, test_CSE_eciesEncryptDecryptTestVect[tv].messageLen);
                if(P_verbose)
                {
                    printf("\nClear Text check: %d, (expected: 1)\n", check == 0);
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

            failed += ((retVal != ECC_SUCCESS) || (check != 0));

    }
    else
    {
        failed++;
    }

    /* return 1 if successful (no test Failed) */
    return (failed == 0);

} /* End of test_CSE_extendedDriver_ECC_ECIESEncryptionDecryption */

static uint32_t test_CSE_extendedDriver_ECC_ECIESDecryption(uint32_t P_verbose)
{
    uint32_t retVal = 0U;

    uint32_t check = 0U;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

    struct ByteArrayDescriptor_stt messageToDecrypt;
    struct ByteArrayDescriptor_stt clearTextMessage;

    struct ECIES_encMsg_stt encryptedMessage;

    struct ECIES_sharedData_stt eciesSharedData;
    struct ECIES_sharedData1_stt eciesSharedData1;
    struct ECIES_sharedData2_stt eciesSharedData2;

    struct ECCpubKeyByteArray_stt FmrPubKey_st;    /* Structure for the Encryption Ephemeral Public Key */
    struct ECCprivKeyByteArray_stt PrivKey_st;     /* Structure that will contain the private key */

    uint8_t clearTextMessageBuf[C_MAX_MESSAGE_LEN] = {0};    /* Buffer that will receive the ECIES Decrypted Message  */

    if(P_verbose)
    {
        printf("\n");
    }
    retVal = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == retVal)
    {
        if(P_verbose)
        {
            printf("Testing ECC ECIES Decryption with ");
            printf("test vector #1.%d: \n", (tv + 1));
        }

        /* Initializes Recipient's Private Key structure with test vectors */
        PrivKey_st.pmPrivKey = test_CSE_eciesDecryptTestVect[tv].recipientPrivateKey;
        PrivKey_st.mPrivKeySize = C_P256_MOD_SIZE;

        /* Initializes Message to decrypt from test vector */
        messageToDecrypt.address = test_CSE_eciesDecryptTestVect[tv].encryptedMessage;
        messageToDecrypt.byteSize = test_CSE_eciesDecryptTestVect[tv].messageLen;

        /* Initializes Shared Data with test vectors */
        eciesSharedData1.pSharedData1 = (uint8_t *)test_CSE_eciesDecryptTestVect[tv].sharedData1;
        eciesSharedData1.sharedData1Size = test_CSE_eciesDecryptTestVect[tv].sharedData1Len;
        eciesSharedData2.pSharedData2 = (uint8_t *)test_CSE_eciesDecryptTestVect[tv].sharedData2;
        eciesSharedData2.sharedData2Size = test_CSE_eciesDecryptTestVect[tv].sharedData2Len;

        eciesSharedData.pSharedData1_stt = &eciesSharedData1;
        eciesSharedData.pSharedData2_stt = &eciesSharedData2;

        encryptedMessage.pEciesEncryptedMessage = (uint8_t*)messageToDecrypt.address;
        encryptedMessage.eciesEncryptedMessageSize = messageToDecrypt.byteSize;
        encryptedMessage.pSenderFmrPubKey = &FmrPubKey_st;
        encryptedMessage.pTag = (uint8_t*)test_CSE_eciesDecryptTestVect[tv].tag;
        encryptedMessage.tagSize = test_CSE_eciesDecryptTestVect[tv].tagLen;

        /* Initialize locations that will receive the Sender Ephemeral public Key */
        FmrPubKey_st.pmPubKeyCoordX = test_CSE_eciesDecryptTestVect[tv].senderFmrPublicXKey;
        FmrPubKey_st.pmPubKeyCoordY = test_CSE_eciesDecryptTestVect[tv].senderFmrPublicYKey;
        FmrPubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
        FmrPubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;
        encryptedMessage.pSenderFmrPubKey = &FmrPubKey_st;

        /* Initializes ClearText Message buffer */
        clearTextMessage.address = clearTextMessageBuf;
        clearTextMessage.byteSize = messageToDecrypt.byteSize;

        /* Launch Decryption */
        if (P_verbose)
        {
            printf("\nDecryption.\n");
        }

        retVal = ECC_ecies_decryption(&PrivKey_st, &encryptedMessage,
                                      &eciesSharedData, &clearTextMessage, P_verbose);

        if (ECC_SUCCESS == retVal)
        {
            /* Compare Expected Expanded Key Data from test vector with computed one */
            check = memcmp(clearTextMessage.address, test_CSE_eciesDecryptTestVect[tv].ExpectedMessage, test_CSE_eciesDecryptTestVect[tv].messageLen);
            if(P_verbose)
            {
                printf("\nClear Text check: %d, (expected: 1)\n", check == 0);
            }
        }
        else
        {
            if(P_verbose)
            {
                printf("\nExecution Failed.\n");
            }
        }

        failed += ((retVal != ECC_SUCCESS) || (check != 0));

    }
    else
    {
        failed++;
    }

    /* return 1 if successful (no test Failed) */
    return (failed == 0);

} /* End of test_CSE_extendedDriver_ECC_ECIESDecryption */

uint32_t test_CSE_extendedDriver_ECC_ECDSA(uint32_t P_verbose, uint32_t P_curveIDToTest)
{
    uint32_t retVal = 0U;

    /* Launch ECDSA Signature generation and signature verification
     * Verify signature that was generated, no test vector for signature result (ECDSA signature depends on random number) */
    retVal = test_CSE_extendedDriver_ECC_ECDSAsignVerify(P_verbose, P_curveIDToTest);
    if (1U == retVal)
    {
        /* Launch ECDSA Signature verification */
        retVal = test_CSE_extendedDriver_ECC_ECDSAsignatureVerification(P_verbose, P_curveIDToTest);
    }

    return (retVal);

} /* End of test_CSE_extendedDriver_ECC_ECDSA */

uint32_t test_CSE_extendedDriver_ECC_P256_ECIES(uint32_t P_verbose)
{
    uint32_t retVal = 0U;

    retVal = test_CSE_extendedDriver_ECC_ECIESEncryptionDecryption(P_verbose);
    if (1U == retVal)
    {
        /* Launch ECIES NIST P256 Decryption against known test vector */
        retVal = test_CSE_extendedDriver_ECC_ECIESDecryption(P_verbose);
    }

    return (retVal);
} /* End of test_CSE_extendedDriver_ECC_ECEIS */


