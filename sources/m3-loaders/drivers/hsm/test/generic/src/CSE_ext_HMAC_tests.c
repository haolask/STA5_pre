/*
SPC5-CRYPTO - Copyright (C) 2018 STMicroelectronics

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
* @file    CSE_ext_HJMAC_tests.c
* @brief   HMAC Mac Generation and Verification tests
* @details
*
*
* @addtogroup CSE_driver_test
* @{
*/

#include <string.h>        /* For memcmp */

#include "config.h"
#include "serialprintf.h"

#include "cse_typedefs.h"
#include "CSE_Manager.h"
#include "CSE_extendKey_updateSupport.h"

#include "CSE_ext_hash.h"
#include "CSE_ext_HMAC.h"
#include "CSE_ext_HMAC_tests.h"
#include "CSE_ext_HMAC_TV.h"
#include "CSE_ext_test_globals.h"

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif

#include "test_values.h"

#include "cse_client.h"

/* Set of test Vectors: restricted to entire length for SHA mac, no truncated mac */
#if 0
extern const hmac_testVect_stt hmacSha1FullMac_testVect_array[C_NB_TV_HMAC_SHA1_RESTRICT];
extern const hmac_testVect_stt hmacSha224FullMac_testVect_array[C_NB_TV_HMAC_SHA224_RESTRICT];
extern const hmac_testVect_stt hmacSha256FullMac_testVect_array[C_NB_TV_HMAC_SHA256_RESTRICT];
extern const hmac_testVect_stt hmacSha384FullMac_testVect_array[C_NB_TV_HMAC_SHA384_RESTRICT];
extern const hmac_testVect_stt hmacSha512FullMac_testVect_array[C_NB_TV_HMAC_SHA512_RESTRICT];
#endif

extern const hmac_testVect_stt hmacSha1RestrictKey_testVect_array[C_NB_HMAC_SHA1_RESTRICT_KEY];
extern const hmac_testVect_stt hmacSha224RestrictKey_testVect_array[C_NB_HMAC_SHA224_RESTRICT_KEY];
extern const hmac_testVect_stt hmacSha256RestrictKey_testVect_array[C_NB_HMAC_SHA256_RESTRICT_KEY];
extern const hmac_testVect_stt hmacSha384RestrictKey_testVect_array[C_NB_HMAC_SHA384_RESTRICT_KEY];
extern const hmac_testVect_stt hmacSha512RestrictKey_testVect_array[C_NB_HMAC_SHA512_RESTRICT_KEY];

uint32_t G_hmacKeyCounter = 2U;

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
                            uint32_t P_verbose)
{
    struct HMACKeyByteArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mBytesize = P_pSecretKey->mBytesize;
    secretKey.pmAddress = P_pSecretKey->pmAddress;

    ret = CSE_HMAC_loadRamKey(&secretKey);
    if (CSE_NO_ERR == ret)
    {
        /* Launch HMAC Mac generation */
        ret = CSE_HMAC_macGeneration(HMAC_RAM_KEY_IDX,
                                     (vuint8_t*)P_pMsg->address,
                                     P_pMsg->byteSize,
                                     P_pMac,
                                     P_hashAlgo);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if (CSE_NO_ERR == ret)
        {
            status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of hmac_macGeneration */

/**
* @brief  HMAC Tag Verification
*
* @param[in]  *P_pSecretKey Pointer to \ref HMACKeyByteArray_stt structure for HMAC RAM secret Key
* @param[in]  *P_pMsg       Pointer to \ref ByteArrayDescriptor_stt Message on which Mac is generated
* @param[in]  *P_pMac       Pointer to \ref ByteArrayDescriptor_stt generated Mac
* @param[in]   P_hashAlgo   Identifier of hash algorithm
* @param[in]   P_verbose    To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR, AUTHENTICATION_SUCCESSFUL, AUTHENTICATION_FAILED
*/

uint32_t hmac_macVerification(struct HMACKeyByteArray_stt * P_pSecretKey,
                              struct ByteArrayDescriptor_stt * P_pMsg,
                              struct ByteArrayDescriptor_stt * P_pMac,
                              enum hashType_et P_hashAlgo,
                              uint32_t P_verbose)
{
    struct HMACKeyByteArray_stt secretKey;

    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;
    uint32_t success = 0U;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Private key */
    secretKey.mBytesize = P_pSecretKey->mBytesize;
    secretKey.pmAddress = P_pSecretKey->pmAddress;

    ret = CSE_HMAC_loadRamKey(&secretKey);
    if (CSE_NO_ERR == ret)
    {
        /* Launch HMAC Mac verification */
        ret = CSE_HMAC_macVerification(HMAC_RAM_KEY_IDX,
                                       (vuint8_t*)P_pMsg->address,
                                       P_pMsg->byteSize,
                                       P_pMac,
                                       P_hashAlgo,
                                       &success);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
#endif

        /* Translate CSE error code */
        if (CSE_NO_ERR == ret)
        {
            if (success == 0)
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

} /* End of hmac_macVerification */

/**
* @brief  HMAC Mac Generation with protected key
*
* @param[in]  *P_pSecretKeyDescr Pointer to \ref HMAC_key_stt structure for HMAC NVM secret Key
* @param[in]   P_keyIndex        Key index
* @param[in]  *P_pMsg            Pointer to \ref ByteArrayDescriptor_stt Message on which Mac is generated
* @param[out] *P_pMac            Pointer to \ref ByteArrayDescriptor_stt generated Mac
* @param[in]   P_hashAlgo        Identifier of hash algorithm
* @param[in]   P_verbose         To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t hmac_macGeneration_pk(struct HMAC_key_stt * P_pSecretKey,
                               uint32_t P_keyIndex,
                               struct ByteArrayDescriptor_stt * P_pMsg,
                               struct ByteArrayDescriptor_stt * P_pMac,
                               enum hashType_et P_hashAlgo,
                               uint32_t P_verbose)
{
    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};    /* 4 words + 32 words for HMAC key (max 128 bytes when SHA384 or SHA512 supported) */
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t i = 0U;
    uint32_t check = 0U;

    /* Get UID of the target HSM */
#if 0
    if (P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }
#endif
    /* Clear buffers */
    for (i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);
#if 0
    if (P_verbose)
    {
        printf("ERC : %x\n", status);
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }
#endif
    /* Load key in non volatile memory */
    status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                            (uint32_t *)P_pSecretKey->hmacKeyByteArray.pmAddress,
                                            P_pSecretKey->hmacKeyByteArray.mBytesize,

                                            authKey, (uint32_t *)UID,
                                            P_keyIndex, authKeyId,
                                            P_pSecretKey->CNT.R, P_pSecretKey->HMAC_flags.R,
                                            M1, M2, M3,
                                            M4, M5);

    status += CSE_HMAC_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
    G_hmacKeyCounter++;    /* For next test */

#ifdef DEBUG_PRINT
    display_buf("resM4   : ", (uint8_t*)resM4, 32);
    display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

    check = memcmp(M4, resM4, 32);
    if (P_verbose)
    {
        printf("M4 Message check: %d (expected: 1), ", check == 0U);
    }

    check = memcmp(M5, resM5, 16);
    if (P_verbose)
    {
        printf("M5 Message check: %d (expected: 1)\n", check == 0U);
    }

    if (CSE_NO_ERR == status)
    {
        /* Launch HMAC Mac generation */
        ret = CSE_HMAC_macGeneration(P_keyIndex,
                                     (vuint8_t*)P_pMsg->address,
                                     P_pMsg->byteSize,
                                     P_pMac,
                                     P_hashAlgo);

        /* Translate CSE error code */
        if (CSE_NO_ERR == ret)
        {
            status = CSE_NO_ERR;
        }
    }

    /* default value would be CSE_INVALID_PARAMETER unless generation ran successfully */
    return(status);

} /* End of hmac_macGeneration_pk */

/**
* @brief  HMAC Tag Verification with protected key
*
* @param[in]  *P_pSecretKeyDescr Pointer to \ref HMAC_key_stt structure for HMAC NVM secret Key
* @param[in]   P_keyIndex        Key index
* @param[in]  *P_pMsg            Pointer to \ref ByteArrayDescriptor_stt Message on which Mac is generated
* @param[in]  *P_pMac            Pointer to \ref ByteArrayDescriptor_stt generated Mac
* @param[in]   P_hashAlgo        Identifier of hash algorithm
* @param[in]   P_verbose         To activate verbose mode during test
*
* @retval error status: CSE_NO_ERR, CSE_INVALID_PARAMETER, CSE_GENERAL_ERR, AUTHENTICATION_SUCCESSFUL, AUTHENTICATION_FAILED
*/

uint32_t hmac_macVerification_pk(struct HMAC_key_stt * P_pSecretKey,
                                 uint32_t P_keyIndex,
                                 struct ByteArrayDescriptor_stt * P_pMsg,
                                 struct ByteArrayDescriptor_stt * P_pMac,
                                 enum hashType_et P_hashAlgo,
                                 uint32_t P_verbose)
{
    uint32_t status = CSE_INVALID_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

    uint32_t i = 0U;
    uint32_t check = 0U;

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + 32] = {0x00000000};    /* 4 words + 32 words for HMAC key (max 128 bytes when SHA384 or SHA512 supported)*/
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
#if 0
    if (P_verbose)
    {
        printf("\n");
        printf("Get ID \n");
    }
#endif
    /* Clear buffers */
    for (i = 0U; i < 4U; i++)
    {
        UID[i] = 0x00000000U;
        MAC[i] = 0x00000000U;
    }
    status = 0U;

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);
#if 0
    if (P_verbose)
    {
        printf("ERC : %x\n", status);
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }
#endif
    /* Load key in non volatile memory */
    status = CSE_extendKeyGenerate_M1_to_M5(NULL, 0, NULL, 0,
                                            (uint32_t *)P_pSecretKey->hmacKeyByteArray.pmAddress,
                                            P_pSecretKey->hmacKeyByteArray.mBytesize,

                                            authKey, (uint32_t *)UID,
                                            P_keyIndex, authKeyId,
                                            P_pSecretKey->CNT.R, P_pSecretKey->HMAC_flags.R,
                                            M1, M2, M3,
                                            M4, M5);

    status += CSE_HMAC_loadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
    G_hmacKeyCounter++;    /* For next test */

#ifdef DEBUG_PRINT
    display_buf("resM4   : ", (uint8_t*)resM4, 32);
    display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

    check = memcmp(M4, resM4, 32);
    if (P_verbose)
    {
        printf("M4 Message check: %d (expected: 1), ", check == 0U);
    }

    check = memcmp(M5, resM5, 16);
    if (P_verbose)
    {
        printf("M5 Message check: %d (expected: 1)\n", check == 0U);
    }
    if (CSE_NO_ERR == status)
    {
        uint32_t success = 0U;

        /* Launch HMAC Mac verification */
        ret = CSE_HMAC_macVerification(P_keyIndex,
                                       (vuint8_t*)P_pMsg->address,
                                       P_pMsg->byteSize,
                                       P_pMac,
                                       P_hashAlgo,
                                       &success);

        /* Translate CSE error code */
        if (CSE_NO_ERR == ret)
        {
            if (success == 0)
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

} /* End of hmac_macVerification_pk */

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
uint32_t hmac_macGeneration_test(uint32_t P_verbose, uint32_t P_reducedTestVectorNb)
{
    uint32_t status = 0U;
    uint32_t check = 0U;
    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t nbTestLoop = 0U;

    struct HMACKeyByteArray_stt hmacSecretKey;     /* Structure for the HMAC Secret Key */
    hmac_testVect_stt * hmacSha_testVect;
    hmac_testVect_stt * hmacSha_testVect_array[5];

    struct ByteArrayDescriptor_stt msg;            /* Descriptor for message */
    struct ByteArrayDescriptor_stt hmacMac;        /* Descriptor for HMAC Mac */

    uint8_t macBuffer[64] = { 0U };         /* Buffer for Mac */

    hmacSha_testVect_array[0] = (hmac_testVect_stt*)hmacSha1RestrictKey_testVect_array;
    hmacSha_testVect_array[1] = (hmac_testVect_stt*)hmacSha224RestrictKey_testVect_array;
    hmacSha_testVect_array[2] = (hmac_testVect_stt*)hmacSha256RestrictKey_testVect_array;
    hmacSha_testVect_array[3] = (hmac_testVect_stt*)hmacSha384RestrictKey_testVect_array;
    hmacSha_testVect_array[4] = (hmac_testVect_stt*)hmacSha512RestrictKey_testVect_array;

    for (nbTestLoop = 0U; nbTestLoop < 5U; nbTestLoop++)
    {
        enum hashType_et hashAlgo = E_SHA1;
        uint32_t nbTestVector = 1U;

        hmacSha_testVect = hmacSha_testVect_array[nbTestLoop];
        switch(nbTestLoop)
        {
        case 0:
            hashAlgo = E_SHA1;
            nbTestVector = C_NB_HMAC_SHA1_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA1 MAC generation with RAM key tests !\n");
            }
            break;
        case 1:
            hashAlgo = E_SHA224;
            nbTestVector = C_NB_HMAC_SHA224_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA224 MAC generation with RAM key tests !\n");
            }
            break;
        case 2:
            hashAlgo = E_SHA256;
            nbTestVector = C_NB_HMAC_SHA256_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA256 MAC generation with RAM key tests !\n");
            }
            break;
        case 3:
            hashAlgo = E_SHA384;
            nbTestVector = C_NB_HMAC_SHA384_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA384 MAC generation with RAM key tests !\n");
            }
            break;
        case 4:
            hashAlgo = E_SHA512;
            nbTestVector = C_NB_HMAC_SHA512_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA512 MAC generation with RAM key tests !\n");
            }
            break;
        default:
            /* What else ?*/
            break;
        }

        if(P_reducedTestVectorNb == 1)
        {
            if(nbTestVector > MAX_REDUCED_TV_NB)
            {
                nbTestVector = MAX_REDUCED_TV_NB;
            }
        }

        for (tv = 0U; tv < nbTestVector; tv++)
        {

            if (P_verbose)
            {
                printf("\nTest Vector number: %d\n", tv);
                printf("Size of Message = %d\n", C_TV_MESSAGE_SIZE);
                printf("Size of Key = %d\n", hmacSha_testVect[tv].keySize);
                printf("Size of Mac = %d\n", hmacSha_testVect[tv].tagSize);
            }

            /* Init Key*/
            hmacSecretKey.pmAddress = (uint8_t*)hmacSha_testVect[tv].hmac_TV.key;
            hmacSecretKey.mBytesize =  hmacSha_testVect[tv].keySize;

            /* Init Message */
            msg.address = hmacSha_testVect[tv].hmac_TV.msg;
            msg.byteSize = C_TV_MESSAGE_SIZE;

            hmacMac.address = macBuffer;
            hmacMac.byteSize = hmacSha_testVect[tv].tagSize;    /* Requested Mac Tag size */

            /* HMAC mac generation */
            status = hmac_macGeneration(&hmacSecretKey,
                                        &msg, &hmacMac, hashAlgo,
                                        P_verbose);
            if (CSE_NO_ERR == status)
            {
                /* Check returned Tag size equal expected one */
                check = hmacMac.byteSize - hmacSha_testVect[tv].tagSize;
                /* Compare Expected Mac from test vector with computed one and with size returned by the HSM */
                check += memcmp(hmacMac.address, (uint8_t*)hmacSha_testVect[tv].hmac_TV.tag, hmacMac.byteSize);
                if (P_verbose)
                {
                    printf("\nHMAC Mac Generation test: Mac check: %d, (expected: 1)\n", check == 0U);
                }
            }
            else
            {
                if (P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0U));
        }

    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);
} /* End of hmac_macGeneration_test */

/*@brief     HMAC MAC Verification tests with RAM key
* @details
* @note
*
* @param[in] P_verbose
*
*/
uint32_t hmac_macVerification_test(uint32_t P_verbose, uint32_t P_reducedTestVectorNb)
{
    uint32_t status = 0U;
    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t nbTestLoop = 0U;

    struct HMACKeyByteArray_stt hmacSecretKey;     /* Structure for the HMAC Secret Key */
    hmac_testVect_stt * hmacSha_testVect;
    hmac_testVect_stt * hmacSha_testVect_array[5];

    struct ByteArrayDescriptor_stt msg;            /* Descriptor for message */
    struct ByteArrayDescriptor_stt hmacMac;        /* Descriptor for HMAC */

    hmacSha_testVect_array[0] = (hmac_testVect_stt*)hmacSha1RestrictKey_testVect_array;
    hmacSha_testVect_array[1] = (hmac_testVect_stt*)hmacSha224RestrictKey_testVect_array;
    hmacSha_testVect_array[2] = (hmac_testVect_stt*)hmacSha256RestrictKey_testVect_array;
    hmacSha_testVect_array[3] = (hmac_testVect_stt*)hmacSha384RestrictKey_testVect_array;
    hmacSha_testVect_array[4] = (hmac_testVect_stt*)hmacSha512RestrictKey_testVect_array;

    for (nbTestLoop = 0U; nbTestLoop < 5U; nbTestLoop++)
    {
        enum hashType_et hashAlgo = E_SHA1;
        uint32_t nbTestVector = 0U;

        hmacSha_testVect = hmacSha_testVect_array[nbTestLoop];
        switch(nbTestLoop)
        {
        case 0:
            hashAlgo = E_SHA1;
            nbTestVector = C_NB_HMAC_SHA1_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA1 MAC verification with RAM key tests !\n");
            }
            break;
        case 1:
            hashAlgo = E_SHA224;
            nbTestVector = C_NB_HMAC_SHA224_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA224 MAC verification with RAM key tests !\n");
            }
            break;
        case 2:
            hashAlgo = E_SHA256;
            nbTestVector = C_NB_HMAC_SHA256_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA256 MAC verification with RAM key tests !\n");
            }
            break;
        case 3:
            hashAlgo = E_SHA384;
            nbTestVector = C_NB_HMAC_SHA384_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA384 MAC verification with RAM key tests !\n");
            }
            break;
        case 4:
            hashAlgo = E_SHA512;
            nbTestVector = C_NB_HMAC_SHA512_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA512 MAC verification with RAM key tests !\n");
            }
            break;
        default:
            /* What else ?*/
            break;
        }

        if(P_reducedTestVectorNb == 1)
        {
            if(nbTestVector > MAX_REDUCED_TV_NB)
            {
                nbTestVector = MAX_REDUCED_TV_NB;
            }
        }

        for (tv = 0U; tv < nbTestVector; tv++)
        {
            if (P_verbose)
            {
                printf("\nTest Vector number: %d\n", tv);
                printf("Size of Message = %d\n", C_TV_MESSAGE_SIZE);
                printf("Size of Key = %d\n", hmacSha_testVect[tv].keySize);
                printf("Size of Mac = %d\n", hmacSha_testVect[tv].tagSize);
            }

            /* Init Key*/
            hmacSecretKey.pmAddress = (uint8_t*)hmacSha_testVect[tv].hmac_TV.key;
            hmacSecretKey.mBytesize =  hmacSha_testVect[tv].keySize;

            /* Init Message */
            msg.address = hmacSha_testVect[tv].hmac_TV.msg;
            msg.byteSize = C_TV_MESSAGE_SIZE;

            /* Init MAC */
            hmacMac.address = hmacSha_testVect[tv].hmac_TV.tag;
            hmacMac.byteSize = hmacSha_testVect[tv].tagSize;

            /* HMAC mac verification */
            status = hmac_macVerification(&hmacSecretKey,
                                          &msg, &hmacMac, hashAlgo,
                                          P_verbose);
            if (P_verbose)
            {
                printf("HMAC Mac Verification check: %d (expected: 1),\n", (status == AUTHENTICATION_SUCCESSFUL));
            }

            failed += (status != AUTHENTICATION_SUCCESSFUL);

        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of hmac_macVerification_test */

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
uint32_t hmac_macGeneration_pk_test(uint32_t P_verbose, uint32_t P_reducedTestVectorNb)
{
    uint32_t status = 0U;
    uint32_t check = 0U;
    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t nbTestLoop = 0U;

    struct HMAC_key_stt hmacSecretKey;     /* Structure for the HMAC Secret Key */
    hmac_testVect_stt * hmacSha_testVect;
    hmac_testVect_stt * hmacSha_testVect_array[5];

    struct ByteArrayDescriptor_stt msg;            /* Descriptor for message */
    struct ByteArrayDescriptor_stt hmacMac;        /* Descriptor for HMAC */

    uint8_t macBuffer[64] = { 0U };           /* Buffer for Mac */

    hmacSha_testVect_array[0] = (hmac_testVect_stt*)hmacSha1RestrictKey_testVect_array;
    hmacSha_testVect_array[1] = (hmac_testVect_stt*)hmacSha224RestrictKey_testVect_array;
    hmacSha_testVect_array[2] = (hmac_testVect_stt*)hmacSha256RestrictKey_testVect_array;
    hmacSha_testVect_array[3] = (hmac_testVect_stt*)hmacSha384RestrictKey_testVect_array;
    hmacSha_testVect_array[4] = (hmac_testVect_stt*)hmacSha512RestrictKey_testVect_array;

    for (nbTestLoop = 0U; nbTestLoop < 5U; nbTestLoop++)
    {
    	enum hashType_et hashAlgo = E_SHA1;
        uint32_t nbTestVector = 0U;

        hmacSha_testVect = hmacSha_testVect_array[nbTestLoop];
        switch(nbTestLoop)
        {
        case 0:
            hashAlgo = E_SHA1;
            nbTestVector = C_NB_HMAC_SHA1_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA1 MAC generation with Protected key tests !\n");
            }
            break;
        case 1:
            hashAlgo = E_SHA224;
            nbTestVector = C_NB_HMAC_SHA224_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA224 MAC generation with Protected key tests !\n");
            }
            break;
        case 2:
            hashAlgo = E_SHA256;
            nbTestVector = C_NB_HMAC_SHA256_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA256 MAC generation with Protected key tests !\n");
            }
            break;
        case 3:
            hashAlgo = E_SHA384;
            nbTestVector = C_NB_HMAC_SHA384_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA384 MAC generation with Protected key tests !\n");
            }
            break;
        case 4:
            hashAlgo = E_SHA512;
            nbTestVector = C_NB_HMAC_SHA512_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA512 MAC generation with Protected key tests !\n");
            }
            break;
        default:
            /* What else ?*/
            break;
        }

        if(P_reducedTestVectorNb == 1)
        {
            if(nbTestVector > MAX_REDUCED_TV_NB)
            {
                nbTestVector = MAX_REDUCED_TV_NB;
            }
        }

        for (tv = 0U; tv < nbTestVector; tv++)
        {
            uint32_t keyIndex = 0U;

            if (P_verbose)
            {
                printf("\nTest Vector number: %d\n", tv);
                printf("Size of Message = %d\n", C_TV_MESSAGE_SIZE);
                printf("Size of Key = %d\n", hmacSha_testVect[tv].keySize);
                printf("Size of Mac = %d\n", hmacSha_testVect[tv].tagSize);
            }

            /* Init Key*/
            hmacSecretKey.hmacKeyByteArray.pmAddress = (uint8_t*)hmacSha_testVect[tv].hmac_TV.key;
            hmacSecretKey.hmacKeyByteArray.mBytesize =  hmacSha_testVect[tv].keySize;

            hmacSecretKey.HMAC_flags.R = 0U;                /* No restriction for that key */
            hmacSecretKey.CNT.R = G_hmacKeyCounter;
            hmacSecretKey.HMAC_flags.B.mac = 1U;            /* Key is allowed for MAC generation */

            msg.address = hmacSha_testVect[tv].hmac_TV.msg;
            msg.byteSize = C_TV_MESSAGE_SIZE;

            hmacMac.address = macBuffer;
            hmacMac.byteSize = hmacSha_testVect[tv].tagSize;    /* Requested Mac Tag size */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;

            keyIndex = 1;

            if (P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

            /* HMAC mac generation */
            status = hmac_macGeneration_pk(&hmacSecretKey, keyIndex,
                                           &msg, &hmacMac, hashAlgo,
                                           P_verbose);
            if (CSE_NO_ERR == status)
            {
                /* Check returned Tag size equal expected one */
                check = hmacMac.byteSize - hmacSha_testVect[tv].tagSize;
                /* Compare Expected Mac from test vector with computed one with the size returned by the HSM */
                check += memcmp(hmacMac.address, (uint8_t*)hmacSha_testVect[tv].hmac_TV.tag, hmacMac.byteSize);
                if (P_verbose)
                {
                    printf("\nHMAC Mac Generation test: Mac check: %d, (expected: 1)\n", check == 0U);
                }
            }
            else
            {
                if (P_verbose)
                {
                    printf("\nExecution Failed.\n");
                }
            }
            failed += ((status != CSE_NO_ERR) || (check != 0U));

        }
    }

    /* return 1 if successful (no test failed) */
    return (failed == 0);

} /* End of hmac_macGeneration_pk_test */

/**
* @brief     HMAC MAC Verification tests with Protected key
* @details
* @note
*
* @param[in] P_verbose
*
*/
uint32_t hmac_macVerification_pk_test(uint32_t P_verbose, uint32_t P_reducedTestVectorNb)
{
    uint32_t status = 0U;
    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t nbTestLoop = 0U;

    struct HMAC_key_stt hmacSecretKey;     /* Structure for the HMAC Secret Key */
    hmac_testVect_stt * hmacSha_testVect;
    hmac_testVect_stt * hmacSha_testVect_array[5];

    struct ByteArrayDescriptor_stt msg;            /* Descriptor for message */
    struct ByteArrayDescriptor_stt hmacMac;        /* Descriptor for HMAC */

    hmacSha_testVect_array[0] = (hmac_testVect_stt*)hmacSha1RestrictKey_testVect_array;
    hmacSha_testVect_array[1] = (hmac_testVect_stt*)hmacSha224RestrictKey_testVect_array;
    hmacSha_testVect_array[2] = (hmac_testVect_stt*)hmacSha256RestrictKey_testVect_array;
    hmacSha_testVect_array[3] = (hmac_testVect_stt*)hmacSha384RestrictKey_testVect_array;
    hmacSha_testVect_array[4] = (hmac_testVect_stt*)hmacSha512RestrictKey_testVect_array;

    for (nbTestLoop = 0U; nbTestLoop < 5U; nbTestLoop++)
    {
    	enum hashType_et hashAlgo = E_SHA1;
        uint32_t nbTestVector = 0U;

        hmacSha_testVect = hmacSha_testVect_array[nbTestLoop];
        switch(nbTestLoop)
        {
        case 0:
            hashAlgo = E_SHA1;
            nbTestVector = C_NB_HMAC_SHA1_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA1 MAC verification with Protected key tests !\n");
            }
            break;
        case 1:
            hashAlgo = E_SHA224;
            nbTestVector = C_NB_HMAC_SHA224_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA224 MAC verification with Protected key tests !\n");
            }
            break;
        case 2:
            hashAlgo = E_SHA256;
            nbTestVector = C_NB_HMAC_SHA256_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA256 MAC verification with Protected key tests !\n");
            }
            break;
        case 3:
            hashAlgo = E_SHA384;
            nbTestVector = C_NB_HMAC_SHA384_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA384 MAC verification with Protected key tests !\n");
            }
            break;
        case 4:
            hashAlgo = E_SHA512;
            nbTestVector = C_NB_HMAC_SHA512_RESTRICT_KEY;
            if (P_verbose)
            {
                printf("\n");
                printf("HMAC SHA512 MAC verification with Protected key tests !\n");
            }
            break;

        default:
            /* What else ?*/
            break;
        }

        if(P_reducedTestVectorNb == 1)
        {
            if(nbTestVector > MAX_REDUCED_TV_NB)
            {
                nbTestVector = MAX_REDUCED_TV_NB;
            }
        }

        for (tv = 0U; tv < nbTestVector; tv++)
        {
            uint32_t keyIndex = 0U;

            if (P_verbose)
            {
                printf("\nTest Vector number: %d\n", tv);
                printf("Size of Message = %d\n", C_TV_MESSAGE_SIZE);
                printf("Size of Key = %d\n", hmacSha_testVect[tv].keySize);
                printf("Size of Mac = %d\n", hmacSha_testVect[tv].tagSize);
            }

            /* Init Key*/
            hmacSecretKey.hmacKeyByteArray.pmAddress = (uint8_t*)hmacSha_testVect[tv].hmac_TV.key;
            hmacSecretKey.hmacKeyByteArray.mBytesize =  hmacSha_testVect[tv].keySize;

            hmacSecretKey.HMAC_flags.R = 0U;                /* No restriction for that key */
            hmacSecretKey.CNT.R = G_hmacKeyCounter;
            hmacSecretKey.HMAC_flags.B.mac = 1U;            /* Key is allowed for MAC verification */

            msg.address = hmacSha_testVect[tv].hmac_TV.msg;
            msg.byteSize = C_TV_MESSAGE_SIZE;

            hmacMac.address = hmacSha_testVect[tv].hmac_TV.tag;
            hmacMac.byteSize = hmacSha_testVect[tv].tagSize;

            keyIndex = (tv % 4) + 1;

            keyIndex = 1;

            if (P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

            /* HMAC mac verification */
            status = hmac_macVerification_pk(&hmacSecretKey, keyIndex,
                                             &msg, &hmacMac, hashAlgo,
                                             P_verbose);
            if (P_verbose)
            {
                printf("HMAC Mac Verification check: %d (expected: 1),\n", (status == AUTHENTICATION_SUCCESSFUL));
            }

            failed += (status != AUTHENTICATION_SUCCESSFUL);
        }
    }

        /* return 1 if successful (no test failed) */
        return (failed == 0);

} /* End of hmac_macVerification_pk_test */

/**
* @}
*/
