/*
 * CSE_ext_ECC_ECDH_test.c
 *
 *  Created on: 6 avr. 2018
  */

/**
 * @file    CSE_ext_ECC_ECDH_test.c
 * @brief   ECDH tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include <string.h>

#include "config.h"
#include "serialprintf.h"

#include "CSE_ext_ECC.h"
#include "CSE_RNG.h"
#include "CSE_ext_test_globals.h"

#include "CSE_internal_tests.h"
#include "CSE_ext_ECC_TV_consts.h"
#include "CSE_ext_ECC_ECDH_short_TV.h"
#include "CSE_ext_ECC_ECDH_test.h"
#include "CSE_ext_ECC_ECDH.h"
#include "CSE_ext_ECC.h"

#include "test_values.h"

#include "CSE_extendKey_updateSupport.h"
#include "CSE_Manager.h"
#include "CSE_extendKey_updateSupport.h"

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif
/******************************************************************************/
/****************************** ECDH Test Vectors *****************************/
/******************************************************************************/
/* Private functions ---------------------------------------------------------*/
extern const ecc_ecdh_testVector_stt ecc_ecdh_short_testVectors[];
extern const ecc_ecdh_keyPair_testVector_stt ecc_ecdh_KeyPair_testVectors[];

/**
  * @brief  ECC ECDH-fixed Key Agreement fixed Test
  * @detail The function performs the ECDH Key Agreement operation
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdh_keyAgreement_fixed_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb)
{
    uint32_t status = CSE_NO_ERR;
    uint32_t ret = 0U;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    uint32_t i = 0U;

    struct ECC_key_stt key1;        /* Key Pair of entity 1 */
    struct ECC_key_stt key2;        /* Key Pair of entity 2 */

    struct EcdhPreMasterSecret ecdhPreMasterKey;

    uint8_t ecdhResult[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 17) + 1] = {0x00000000};    /* To support P521; Must contain at least a multiple of entire number of AES block */
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resLoadM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                             0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resLoadM5[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1;

    uint32_t keyIndex = ECC_KEY_1_IDX;        /* Fix NVM key index for this test */

    int32_t check = 0;
    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t currentCurveSize = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("ECDH-fixed Key Agreement tests !\n");
    }
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

    ret = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", ret );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    tv_max_number = C_NB_TEST_ECDH_SHORT_TV;
    if(reduced_test_vector_nb)
    {
        if(tv_max_number > MAX_REDUCED_TV_NB)
        {
            tv_max_number = MAX_REDUCED_TV_NB;
        }
    }

    for(tv = 0U; tv < tv_max_number; tv ++)
    {
        /* Initialize Public Key with values from test vectors */
        key1.PubKeyByteArray.pmPubKeyCoordX = ecc_ecdh_short_testVectors[tv].pEcdhPublicKeyX;
        key1.PubKeyByteArray.pmPubKeyCoordY = ecc_ecdh_short_testVectors[tv].pEcdhPublicKeyY;

        key2.PrivKeyByteArray.pmPrivKey = ecc_ecdh_short_testVectors[tv].pEcdhPrivateKey;

        switch(ecc_ecdh_short_testVectors[tv].ecdhCurveID)
        {
        case C_NIST_P_256:
            key1.PubKeyByteArray.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            key1.PubKeyByteArray.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_256: ");
            }
            break;

        case C_NIST_P_384:
            key1.PubKeyByteArray.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            key1.PubKeyByteArray.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_384: ");
            }
            break;

        case C_NIST_P_521:
            key1.PubKeyByteArray.mPubKeyCoordXSize = C_P521_MOD_SIZE;
            key1.PubKeyByteArray.mPubKeyCoordYSize = C_P521_MOD_SIZE;

            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P521_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_521: ");
            }
            break;

        case C_BRAINPOOL_P256R1:
            key1.PubKeyByteArray.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            key1.PubKeyByteArray.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_256: ");
            }
            break;

        case C_BRAINPOOL_P384R1:
            key1.PubKeyByteArray.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            key1.PubKeyByteArray.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_384: ");
            }
            break;

        default:
            /* What else ? */
            status = CSE_INVALID_PARAMETER;
            if(P_verbose)
            {
                printf("Curve is Unknown! ");
            }
            break;
        } /* End switch */

        if (CSE_NO_ERR == ret)
        {
            /* Switch to requested curve for the test */
            status = CSE_ECC_changeCurve(currentECId);
            if( CSE_NO_ERR == status)
            {
                {
                    /* Load Key Pair of entity 1 in ECC RAM location */
                    /* No private key part for entity 1 */
                    key1.PrivKeyByteArray.pmPrivKey = NULL;
                    key1.PrivKeyByteArray.mPrivKeySize = 0U;

                    status = CSE_ECC_LoadRamKey(&key1.PubKeyByteArray, &key1.PrivKeyByteArray);
                    if( CSE_NO_ERR == status)
                    {
                        /* Load Key Pair of entity 2, only private key */
                        key2.PubKeyByteArray.pmPubKeyCoordX = NULL;
                        key2.PubKeyByteArray.mPubKeyCoordXSize = 0;
                        key2.PubKeyByteArray.pmPubKeyCoordY = NULL;
                        key2.PubKeyByteArray.mPubKeyCoordYSize = 0;

                        key2.PrivKeyByteArray.mPrivKeySize = currentCurveSize;

                        key2.CNT.R = G_asymKeyCounter;
                        key2.EC_flags.R = 0U;            /* No restriction on Keys */
                        key2.EC_flags.B.sign = 1U;       /* Set key as Signature Key */

                        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
                        status = CSE_extendKeyGenerate_M1_to_M5((uint32_t *)key2.PubKeyByteArray.pmPubKeyCoordX,
                                                              key2.PubKeyByteArray.mPubKeyCoordXSize,
                                                              (uint32_t *)key2.PubKeyByteArray.pmPubKeyCoordY,
                                                              key2.PubKeyByteArray.mPubKeyCoordYSize,
                                                              (uint32_t *)key2.PrivKeyByteArray.pmPrivKey,
                                                              key2.PrivKeyByteArray.mPrivKeySize,

                                                              authKey, (uint32_t*) UID,
                                                              keyIndex, authKeyId ,
                                                              key2.CNT.R, key2.EC_flags.R,
                                                              M1, M2, M3,
                                                              M4, M5);

                        status += CSE_ECC_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resLoadM4, (uint8_t*)resLoadM5);
                        G_asymKeyCounter++;                     /* Increment global anti roll back counter for ECC Keys */

#ifdef DEBUG_PRINT
                        display_buf("resM4   : ", (uint8_t*)resM4, 32);
                        display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

                        check = memcmp(M4, resLoadM4, 32);
                        if(P_verbose)
                        {
                            printf("M4 Message check: %d (expected: 1), ", check == 0U);
                        }

                        check = memcmp(M5, resLoadM5, 16);
                        if(P_verbose)
                        {
                            printf("M5 Message check: %d (expected: 1)\n", check == 0U);
                        }

                        ecdhPreMasterKey.address = ecdhResult;
                        *(uint32_t *)&(ecdhPreMasterKey.address[0]) = C_MAX_SIG_SIZE;
                        ecdhPreMasterKey.byteSize = 0U;

                        status = CSE_ECC_ecdhFixedKeyAgreement(ECC_KEY_1_IDX, &ecdhPreMasterKey);
                        if (ECC_SUCCESS == status)
                        {
                            /* Compare Signature fields r, s with expected signature from test vectors */
                            check = memcmp(ecdhResult, (uint8_t*)ecc_ecdh_short_testVectors[tv].pEcdhExpectedOutput, currentCurveSize);
                            if(P_verbose)
                            {
                                printf("ECDH-fixed Key Agreement check: %d (expected: 1)\n", check == 0);
                            }
                        }
                        else
                        {
                            if(P_verbose)
                            {
                                printf("ECDH Key Agreement Failed.\n");
                            }
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf("Unitary test mode Failed.\n");
                        }
                    }
                }

                /* Set Error Flag */
                failed += ((ECC_SUCCESS != status) || ((ECC_SUCCESS == status) && (0 != check)));

            }
        }

    } /* End loop on test */


    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (P_verbose)
        {
            printf("Curve restored to NIST P_256.\n");
        }
    }
    else
    {
        if (P_verbose)
        {
            printf("Curve restoration to default value failed !\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(failed == 0);
}

/**
  * @brief  ECC ECDH Key Agreement Client Server Test
  * @detail The function performs the ECDH Key Agreement test
  *         The test checks that two shared secrets are equally generated betwee, Client
  *         and server side
  *         It performs the generation of the shared secret on the server side
  *         with the ECDH function and performs the generation of the shared secret on
  *         the client side by using the ECDH-fixed command. Both secret values must be
  *         identical.
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdh_keyAgreementClientServer_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb)
{
    uint32_t status = 0U;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    struct ECC_key_stt key;        /* Key Pair */

    struct EcdhPreMasterSecret ecdhPreMasterKeyA;
    struct EcdhPreMasterSecret ecdhPreMasterKeyB;

    uint8_t ecdhResultA[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */
    uint8_t ecdhResultB[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */

    uint32_t failed = 0U;
    int32_t check = 0;
    uint32_t i = 0U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t currentCurveSize = 0U;

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 17) + 1] = {0x00000000};    /* To support P521; Must contain at least a multiple of entire number of AES block */
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resLoadM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                             0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resLoadM5[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1;

    uint32_t keyIndex = ECC_KEY_1_IDX;        /* Fix NVM key index for this test */

    if(P_verbose)
    {
        printf("\n");
        printf("ECDH Client/Server Key Agreement tests !\n");
    }
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

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    tv_max_number = C_NB_TEST_ECDH_KEY_PAIR_TV;
    if(reduced_test_vector_nb)
    {
        if(tv_max_number > MAX_REDUCED_TV_NB)
        {
            tv_max_number = MAX_REDUCED_TV_NB;
        }
    }

    for(tv = 0U; tv < tv_max_number; tv ++)
    {
        if(P_verbose)
        {
            printf("Test number: %d\n", tv);
        }

        /* Initialize Public Key with values from test vectors */
        key.PubKeyByteArray.pmPubKeyCoordX = ecc_ecdh_KeyPair_testVectors[tv].pEcdhPublicKeyX;
        key.PubKeyByteArray.pmPubKeyCoordY = ecc_ecdh_KeyPair_testVectors[tv].pEcdhPublicKeyY;

        switch(ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID)
        {
        case C_NIST_P_256:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_256: ");
            }
            break;

        case C_NIST_P_384:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_384: ");
            }
            break;

        case C_NIST_P_521:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P521_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_521: ");
            }
            break;

        case C_BRAINPOOL_P256R1:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_256: ");
            }
            break;

        case C_BRAINPOOL_P384R1:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_384: ");
            }
            break;

        default:
            /* What else ? */
            if(P_verbose)
            {
                printf("Curve is Unknown! ");
            }
            break;
        } /* End switch */

        /* Switch to requested curve for the test */
        status = CSE_ECC_changeCurve(currentECId);
        if( CSE_NO_ERR == status)
        {
            /* Load public key in ECC RAM location */
            /* No private key */
            key.PrivKeyByteArray.pmPrivKey = NULL;
            key.PrivKeyByteArray.mPrivKeySize = 0U;

            key.PubKeyByteArray.mPubKeyCoordXSize = currentCurveSize;
            key.PubKeyByteArray.mPubKeyCoordYSize = currentCurveSize;

            status = CSE_ECC_LoadRamKey(&key.PubKeyByteArray, &key.PrivKeyByteArray);
            if( CSE_NO_ERR == status)
            {
                ecdhPreMasterKeyA.address = ecdhResultA;
                *(uint32_t *)&(ecdhPreMasterKeyA).address[0] = C_MAX_SIG_SIZE;
                ecdhPreMasterKeyA.byteSize = 0U;

                status = CSE_ECC_ecdhKeyAgreement(ECC_RAM_KEY_IDX, &ecdhPreMasterKeyA);
                if (ECC_SUCCESS == status)
                {
                    {   /* Export and display for test */
                        struct ECCpubKeyByteArray_stt pubKey;

                        uint8_t pubKeyCoordX[C_MAX_PUB_KEY_SIZE];    /* Buffer that will receive public key x coordinate */
                        uint8_t pubKeyCoordY[C_MAX_PUB_KEY_SIZE];    /* Buffer that will receive public key y coordinate */

                        /* Export ECC RAM Public key generated during ECDH operation */
                        pubKey.pmPubKeyCoordX = pubKeyCoordX;
                        pubKey.mPubKeyCoordXSize = C_MAX_PUB_KEY_SIZE;
                        pubKey.pmPubKeyCoordY = pubKeyCoordY;
                        pubKey.mPubKeyCoordYSize = C_MAX_PUB_KEY_SIZE;

                        status = CSE_ECC_exportPublicKey(ECC_RAM_KEY_IDX, &pubKey);
                        if (P_verbose)
                        {
                            printf("Exporting Public Key check: %d (expected: 1)\n", CSE_NO_ERR == status);
                            display_buf("FMR Public Key x      : ", (uint8_t*)pubKey.pmPubKeyCoordX, pubKey.mPubKeyCoordXSize);
                            display_buf("FMR Public Key y      : ", (uint8_t*)pubKey.pmPubKeyCoordY, pubKey.mPubKeyCoordYSize);
                        }
                    }

                    /* Load Private key in NVM and perform ECDH-fixed to compute shared secret */
                    key.PrivKeyByteArray.pmPrivKey = ecc_ecdh_KeyPair_testVectors[tv].pEcdhPrivateKey;
                    key.PrivKeyByteArray.mPrivKeySize = currentCurveSize;

                    /* No public key required */
                    key.PubKeyByteArray.pmPubKeyCoordX = NULL;
                    key.PubKeyByteArray.pmPubKeyCoordY = NULL;
                    key.PubKeyByteArray.mPubKeyCoordXSize = 0U;
                    key.PubKeyByteArray.mPubKeyCoordYSize = 0U;

                    key.CNT.R = G_asymKeyCounter;
                    key.EC_flags.R = 0U;            /* No restriction on Keys */
                    key.EC_flags.B.sign = 1U;       /* Set key as signature Key */

                    /* Load Key in Non Volatile Memory. Key index varies with test number ! */
                    status = CSE_extendKeyGenerate_M1_to_M5((uint32_t *)key.PubKeyByteArray.pmPubKeyCoordX,
                                                          key.PubKeyByteArray.mPubKeyCoordXSize,
                                                          (uint32_t *)key.PubKeyByteArray.pmPubKeyCoordY,
                                                          key.PubKeyByteArray.mPubKeyCoordYSize,
                                                          (uint32_t *)key.PrivKeyByteArray.pmPrivKey,
                                                          key.PrivKeyByteArray.mPrivKeySize,

                                                          authKey, (uint32_t*) UID,
                                                          keyIndex, authKeyId ,
                                                          key.CNT.R, key.EC_flags.R,
                                                          M1, M2, M3,
                                                          M4, M5);

                    status += CSE_ECC_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resLoadM4, (uint8_t*)resLoadM5);
                    G_asymKeyCounter++;                     /* Increment global anti roll back counter for ECC Keys */

#ifdef DEBUG_PRINT
                    display_buf("resM4   : ", (uint8_t*)resM4, 32);
                    display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

                    check = memcmp(M4, resLoadM4, 32);
                    if(P_verbose)
                    {
                        printf("M4 Message check: %d (expected: 1), ", check == 0U);
                    }

                    check = memcmp(M5, resLoadM5, 16);
                    if(P_verbose)
                    {
                        printf("M5 Message check: %d (expected: 1)\n", check == 0U);
                    }
                        ecdhPreMasterKeyB.address = ecdhResultB;
                        *(uint32_t *)&(ecdhPreMasterKeyB).address[0] = C_MAX_SIG_SIZE;
                        ecdhPreMasterKeyB.byteSize = 0U;

                        status = CSE_ECC_ecdhFixedKeyAgreement(ECC_KEY_1_IDX, &ecdhPreMasterKeyB);
                        if (ECC_SUCCESS == status)
                        {
                            /* Compare the two shared secrets */
                            if (P_verbose)
                            {
                                display_buf("Shared Secret A      : ", ecdhResultA, currentCurveSize);
                                display_buf("Shared Secret B      : ", ecdhResultB, currentCurveSize);
                            }
                            check = memcmp(ecdhResultA, ecdhResultB, currentCurveSize);
                            if(P_verbose)
                            {
                                printf("ECDH Client/Server Key Agreement check: %d (expected: 1)\n\n", check == 0);
                            }
                        }
                        else
                        {
                            if(P_verbose)
                            {
                                printf("ECDH_fixed Key Agreement Failed.\n\n");
                            }
                        }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf("ECDH Key Agreement Failed.\n\n");
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("Load RAM Key Failed.\n\n");
                }
            }
        }

            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status) || ((ECC_SUCCESS == status) && (0 != check)));

    } /* End loop on test */

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (P_verbose)
        {
            printf("Curve restored to NIST P_256.\n");
        }
    }
    else
    {
        if (P_verbose)
        {
            printf("Curve restoration to default value failed !\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(failed == 0);
} /*  End of ecc_ecdh_keyAgreementClientServer_test */

/**
  * @brief  ECC ECDH Key Agreement Server Test
  * @detail The function performs the ECDH Szerver Key Agreement test
  *         It performs the generation of the shared secret on the server side
  *         a server private key that must already be present in server side
  *         and a public key provided with P1 parameter.
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/
uint32_t ecc_ecdh_serverKeyAgreement_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb)
{
    uint32_t status = 0U;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    struct ECC_key_stt serverKey;        /* Key Pair */
    struct ECCpubKeyByteArray_stt clientPubKey;

    struct EcdhPreMasterSecret ecdhPreMasterKey;

    uint8_t ecdhResult[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */

    uint32_t failed = 0U;
    int32_t check = 0;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t currentCurveSize = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("ECDH-fixed Server Key Agreement tests !\n");
    }

    tv_max_number = C_NB_TEST_ECDH_SHORT_TV;
    if(reduced_test_vector_nb)
    {
        if(tv_max_number > MAX_REDUCED_TV_NB)
        {
            tv_max_number = MAX_REDUCED_TV_NB;
        }
    }

    for(tv = 0U; tv < tv_max_number; tv ++)
    {
        if(P_verbose)
        {
            printf("Test number: %d\n", tv);
        }

        switch(ecc_ecdh_short_testVectors[tv].ecdhCurveID)
        {
        case C_NIST_P_256:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_256: ");
            }
            break;

        case C_NIST_P_384:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_384: ");
            }
            break;

        case C_NIST_P_521:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P521_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_521: ");
            }
            break;

        case C_BRAINPOOL_P256R1:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_256: ");
            }
            break;

        case C_BRAINPOOL_P384R1:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_384: ");
            }
            break;

        default:
            /* What else ? */
            if(P_verbose)
            {
                printf("Curve is Unknown! ");
            }
            break;
        } /* End switch */

        /* Switch to requested curve for the test */
        status = CSE_ECC_changeCurve(currentECId);
        if( CSE_NO_ERR == status)
        {
            /* Load Server key in ECC RAM location */
            /* Initialize Private Key with values from test vectors */
            serverKey.PrivKeyByteArray.pmPrivKey = ecc_ecdh_short_testVectors[tv].pEcdhPrivateKey;
            serverKey.PrivKeyByteArray.mPrivKeySize = currentCurveSize;

            /* No need for public key */
            serverKey.PubKeyByteArray.pmPubKeyCoordX = NULL;
            serverKey.PubKeyByteArray.pmPubKeyCoordY = NULL;
            serverKey.PubKeyByteArray.mPubKeyCoordXSize = 0U;
            serverKey.PubKeyByteArray.mPubKeyCoordYSize = 0U;

            status = CSE_ECC_LoadRamKey(&serverKey.PubKeyByteArray, &serverKey.PrivKeyByteArray);
            if( CSE_NO_ERR == status)
            {
                /* Initialize Client Public Key */
                clientPubKey.pmPubKeyCoordX = ecc_ecdh_short_testVectors[tv].pEcdhPublicKeyX;
                clientPubKey.pmPubKeyCoordY = ecc_ecdh_short_testVectors[tv].pEcdhPublicKeyY;
                clientPubKey.mPubKeyCoordXSize = currentCurveSize;
                clientPubKey.mPubKeyCoordYSize = currentCurveSize;

                ecdhPreMasterKey.address = ecdhResult;
                *(uint32_t *)&(ecdhPreMasterKey).address[0] = C_MAX_SIG_SIZE;
                ecdhPreMasterKey.byteSize = 0U;

                status = CSE_ECC_ecdhFixedServerKeyAgreement(&clientPubKey, &ecdhPreMasterKey);
                if (ECC_SUCCESS == status)
                {
                    /* Compare the two shared secrets */
                    if (P_verbose)
                    {
                        display_buf("Shared Secret      : ", ecdhResult, currentCurveSize);
                    }
                    check = memcmp(ecdhResult, ecc_ecdh_short_testVectors[tv].pEcdhExpectedOutput, currentCurveSize);
                    if(P_verbose)
                    {
                        printf("ECDH-fixed Server Key Agreement check: %d (expected: 1)\n\n", check == 0);
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf("ECDH-fixed Server Key Agreement Failed.\n\n");
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("Load RAM Key Failed.\n\n");
                }
            }
        }

            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status) || ((ECC_SUCCESS == status) && (0 != check)));

    } /* End loop on test */

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (P_verbose)
        {
            printf("Curve restored to NIST P_256.\n");
        }
    }
    else
    {
        if (P_verbose)
        {
            printf("Curve restoration to default value failed !\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(failed == 0);
} /*  End of ecc_ecdh_serverKeyAgreement_test */

/**
  * @brief  ECC ECDH Key Agreement Client Server Test using protected keys
  * @detail The function performs the ECDH Key Agreement test
  *         The test checks that two shared secrets are equally generated betwee, Client
  *         and server side
  *         It performs the generation of the shared secret on the server side
  *         with the ECDH function and performs the generation of the shared secret on
  *         the client side by using the ECDH-fixed command. Both secret values must be
  *         identical.
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdh_keyAgreementClientServer_test_with_PK(uint32_t P_verbose, uint32_t reduced_test_vector_nb)
{
    uint32_t status = 0U;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    struct ECC_key_stt key;        /* Key Pair */

    struct EcdhPreMasterSecret ecdhPreMasterKeyA;
    struct EcdhPreMasterSecret ecdhPreMasterKeyB;

    uint8_t ecdhResultA[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */
    uint8_t ecdhResultB[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */

    uint32_t failed = 0U;
    int32_t check = 0;
    uint32_t i = 0U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t currentCurveSize = 0U;

    uint32_t keyIndex = 0U;

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 17) + 1] = {0x00000000};    /* To support P521; Must contain at least a multiple of entire number of AES block */
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resLoadM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                             0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t resLoadM5[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1;

    if(P_verbose)
    {
        printf("\n");
        printf("ECDH Client/Server Key Agreement tests with protected keys!\n");
    }
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

    status = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", status );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    tv_max_number = C_NB_TEST_ECDH_KEY_PAIR_TV;
    if(reduced_test_vector_nb)
    {
        if(tv_max_number > MAX_REDUCED_TV_NB)
        {
            tv_max_number = MAX_REDUCED_TV_NB;
        }
    }

    for(tv = 0U; tv < tv_max_number; tv ++)
    {
        if(P_verbose)
        {
            printf("Test number: %d\n", tv);
        }

        /* Initialize Public Key with values from test vectors */
        key.PubKeyByteArray.pmPubKeyCoordX = ecc_ecdh_KeyPair_testVectors[tv].pEcdhPublicKeyX;
        key.PubKeyByteArray.pmPubKeyCoordY = ecc_ecdh_KeyPair_testVectors[tv].pEcdhPublicKeyY;

        switch(ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID)
        {
        case C_NIST_P_256:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_256: ");
            }
            break;

        case C_NIST_P_384:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_384: ");
            }
            break;

        case C_NIST_P_521:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P521_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_521: ");
            }
            break;

        case C_BRAINPOOL_P256R1:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_256: ");
            }
            break;

        case C_BRAINPOOL_P384R1:
            currentECId = ecc_ecdh_KeyPair_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_384: ");
            }
            break;

        default:
            /* What else ? */
            if(P_verbose)
            {
                printf("Curve is Unknown! ");
            }
            break;
        } /* End switch */

        /* Switch to requested curve for the test */
        status = CSE_ECC_changeCurve(currentECId);
        if( CSE_NO_ERR == status)
        {
            /* Load public key in ECC RAM location */
            /* No private key */
            key.PrivKeyByteArray.pmPrivKey = NULL;
            key.PrivKeyByteArray.mPrivKeySize = 0U;

            key.PubKeyByteArray.mPubKeyCoordXSize = currentCurveSize;
            key.PubKeyByteArray.mPubKeyCoordYSize = currentCurveSize;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.verify = 1U;     /* Set key as verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad Public Key at NVM index %d.\n", keyIndex);
            }

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            status = CSE_extendKeyGenerate_M1_to_M5((uint32_t *)key.PubKeyByteArray.pmPubKeyCoordX,
                                                  key.PubKeyByteArray.mPubKeyCoordXSize,
                                                  (uint32_t *)key.PubKeyByteArray.pmPubKeyCoordY,
                                                  key.PubKeyByteArray.mPubKeyCoordYSize,
                                                  (uint32_t *)key.PrivKeyByteArray.pmPrivKey,
                                                  key.PrivKeyByteArray.mPrivKeySize,

                                                  authKey, (uint32_t*) UID,
                                                  keyIndex, authKeyId ,
                                                  key.CNT.R, key.EC_flags.R,
                                                  M1, M2, M3,
                                                  M4, M5);

            status += CSE_ECC_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resLoadM4, (uint8_t*)resLoadM5);
            G_asymKeyCounter++;                     /* Increment global anti roll back counter for ECC Keys */

#ifdef DEBUG_PRINT
            display_buf("resM4   : ", (uint8_t*)resM4, 32);
            display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif
            if( CSE_NO_ERR == status)
            {
                check = memcmp(M4, resLoadM4, 32);
                if(P_verbose)
                {
                    printf("M4 Message check: %d (expected: 1), ", check == 0U);
                }

                check = memcmp(M5, resLoadM5, 16);
                if(P_verbose)
                {
                    printf("M5 Message check: %d (expected: 1)\n", check == 0U);
                }

                ecdhPreMasterKeyA.address = ecdhResultA;
                *(uint32_t *)&(ecdhPreMasterKeyA).address[0] = C_MAX_SIG_SIZE;
                ecdhPreMasterKeyA.byteSize = 0U;

                status = CSE_ECC_ecdhKeyAgreement(keyIndex, &ecdhPreMasterKeyA);
                if (ECC_SUCCESS == status)
                {
                    {   /* Export and display for test */
                        struct ECCpubKeyByteArray_stt pubKey;

                        uint8_t pubKeyCoordX[C_MAX_PUB_KEY_SIZE];    /* Buffer that will receive public key x coordinate */
                        uint8_t pubKeyCoordY[C_MAX_PUB_KEY_SIZE];    /* Buffer that will receive public key y coordinate */

                        /* Export ECC RAM Public key generated during ECDH operation */
                        pubKey.pmPubKeyCoordX = pubKeyCoordX;
                        pubKey.mPubKeyCoordXSize = C_MAX_PUB_KEY_SIZE;
                        pubKey.pmPubKeyCoordY = pubKeyCoordY;
                        pubKey.mPubKeyCoordYSize = C_MAX_PUB_KEY_SIZE;

                        status = CSE_ECC_exportPublicKey(ECC_RAM_KEY_IDX, &pubKey);
                        if (P_verbose)
                        {
                            printf("Exporting Public Key check: %d (expected: 1)\n", CSE_NO_ERR == status);
                            display_buf("FMR Public Key x: ", (uint8_t*)pubKey.pmPubKeyCoordX, pubKey.mPubKeyCoordXSize);
                            display_buf("FMR Public Key y: ", (uint8_t*)pubKey.pmPubKeyCoordY, pubKey.mPubKeyCoordYSize);
                        }
                    }

                    /* Load Private key in NVM and perform ECDH-fixed to compute shared secret */
                    key.PrivKeyByteArray.pmPrivKey = ecc_ecdh_KeyPair_testVectors[tv].pEcdhPrivateKey;
                    key.PrivKeyByteArray.mPrivKeySize = currentCurveSize;

                    /* No public key required */
                    key.PubKeyByteArray.pmPubKeyCoordX = NULL;
                    key.PubKeyByteArray.pmPubKeyCoordY = NULL;
                    key.PubKeyByteArray.mPubKeyCoordXSize = 0U;
                    key.PubKeyByteArray.mPubKeyCoordYSize = 0U;

                    key.CNT.R = G_asymKeyCounter;
                    key.EC_flags.R = 0U;            /* No restriction on Keys */
                    key.EC_flags.B.sign = 1U;       /* Set key as signature Key */

                    /* Load Key in Non Volatile Memory. Key index varies with test number ! */
                    status = CSE_extendKeyGenerate_M1_to_M5((uint32_t *)key.PubKeyByteArray.pmPubKeyCoordX,
                                                          key.PubKeyByteArray.mPubKeyCoordXSize,
                                                          (uint32_t *)key.PubKeyByteArray.pmPubKeyCoordY,
                                                          key.PubKeyByteArray.mPubKeyCoordYSize,
                                                          (uint32_t *)key.PrivKeyByteArray.pmPrivKey,
                                                          key.PrivKeyByteArray.mPrivKeySize,

                                                          authKey, (uint32_t*) UID,
                                                          keyIndex, authKeyId ,
                                                          key.CNT.R, key.EC_flags.R,
                                                          M1, M2, M3,
                                                          M4, M5);

                    status += CSE_ECC_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resLoadM4, (uint8_t*)resLoadM5);
                    G_asymKeyCounter++;                     /* Increment global anti roll back counter for ECC Keys */

#ifdef DEBUG_PRINT
                    display_buf("resM4   : ", (uint8_t*)resM4, 32);
                    display_buf("resM5   : ", (uint8_t*)resM5, 16);
#endif

                    check = memcmp(M4, resLoadM4, 32);
                    if(P_verbose)
                    {
                        printf("M4 Message check: %d (expected: 1), ", check == 0U);
                    }

                    check = memcmp(M5, resLoadM5, 16);
                    if(P_verbose)
                    {
                        printf("M5 Message check: %d (expected: 1)\n", check == 0U);
                    }
                    ecdhPreMasterKeyB.address = ecdhResultB;
                    *(uint32_t *)&(ecdhPreMasterKeyB).address[0] = C_MAX_SIG_SIZE;
                    ecdhPreMasterKeyB.byteSize = 0U;

                    status = CSE_ECC_ecdhFixedKeyAgreement(keyIndex, &ecdhPreMasterKeyB);
                    if (ECC_SUCCESS == status)
                    {
                        /* Compare the two shared secrets */
                        if (P_verbose)
                        {
                            display_buf("Shared Secret A      : ", ecdhResultA, currentCurveSize);
                            display_buf("Shared Secret B      : ", ecdhResultB, currentCurveSize);
                        }
                        check = memcmp(ecdhResultA, ecdhResultB, currentCurveSize);
                        if(P_verbose)
                        {
                            printf("ECDH Client/Server Key Agreement check: %d (expected: 1)\n\n", check == 0);
                        }
                    }
                    else
                    {
                        if(P_verbose)
                        {
                            printf("ECDH_fixed Key Agreement Failed.\n\n");
                        }
                    }
            }
                else
                {
                    if(P_verbose)
                    {
                        printf("ECDH Key Agreement Failed.\n\n");
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("Load RAM Key Failed.\n\n");
                }
            }
        }

            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status) || ((ECC_SUCCESS == status) && (0 != check)));

    } /* End loop on test */

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (P_verbose)
        {
            printf("Curve restored to NIST P_256.\n");
        }
    }
    else
    {
        if (P_verbose)
        {
            printf("Curve restoration to default value failed !\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(failed == 0);
} /*  End of ecc_ecdh_keyAgreementClientServer_test_with_PK */

#if 0
/**
  * @brief  ECC ECDH Key Agreement Test
  * @detail The function performs the ECDH Key Agreement operation
  * @detail The function also perform an export of the Ephemeral Public Key and display it
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdh_keyAgreement_test(uint32_t P_verbose)
{
    uint32_t status = 0U;
    ECCpubKeyByteArray_stt pubKey_st;    /* Structure that will contain the public key */
    ECCprivKeyByteArray_stt privKey_st;  /* Structure that will contain the private key */

    EcdhPreMasterSecret ecdhPreMasterKey;

    uint8_t ecdhResult[C_MAX_SIG_SIZE];    /* Buffer that will contain result of ECDH operation */
    ECCpubKeyByteArray_stt pubKey;

    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t currentCurveSize = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("ECDH Key Agreement Tests !\n");
    }

    for(tv = 0U; tv < C_NB_TEST_ECDH_TV; tv ++)
    {
        /* Initialize Public Key with values from test vectors */
        pubKey_st.pmPubKeyCoordX = ecc_ecdh_short_testVectors[tv].pEcdhPublicKeyX;
        pubKey_st.pmPubKeyCoordY = ecc_ecdh_short_testVectors[tv].pEcdhPublicKeyY;

        switch(ecc_ecdh_short_testVectors[tv].ecdhCurveID)
        {
        case C_NIST_P_256:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_256: ");
            }
            break;

        case C_NIST_P_384:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_384: ");
            }
            break;

        case C_NIST_P_521:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P521_MOD_SIZE;
            if(P_verbose)
            {
                printf("NIST P_521: ");
            }
            break;

        case C_BRAINPOOL_P256R1:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P256_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_256: ");
            }
            break;

        case C_BRAINPOOL_P384R1:
            currentECId = ecc_ecdh_short_testVectors[tv].ecdhCurveID;
            currentCurveSize = C_P384_MOD_SIZE;
            if(P_verbose)
            {
                printf("BRAINPOOL P_384: ");
            }
            break;

        default:
            /* What else ? */
            if(P_verbose)
            {
                printf("Curve is Unknown! ");
            }
            break;
        } /* End switch */

        /* Switch to requested curve for the test */
        status = CSE_ECC_changeCurve(currentECId);
        if( CSE_NO_ERR == status)
        {
            /* Load public key in ECC RAM location */
            /* No private key */
            privKey_st.pmPrivKey = NULL;
            privKey_st.mPrivKeySize = 0U;

            pubKey_st.mPubKeyCoordXSize = currentCurveSize;
            pubKey_st.mPubKeyCoordYSize = currentCurveSize;

            status = CSE_ECC_LoadRamKey(&pubKey_st, &privKey_st);
            if( CSE_NO_ERR == status)
            {
                ecdhPreMasterKey.address = ecdhResult;
                *(uint32_t *)&(ecdhPreMasterKey).address[0] = C_MAX_SIG_SIZE;
                ecdhPreMasterKey.byteSize = 0U;

                status = CSE_ECC_ecdhKeyAgreement(ECC_RAM_KEY_IDX, &ecdhPreMasterKey);
                if (ECC_SUCCESS == status)
                {
                    {
                        uint8_t pubKeyCoordX[C_MAX_PUB_KEY_SIZE];    /* Buffer that will receive public key x coordinate */
                        uint8_t pubKeyCoordY[C_MAX_PUB_KEY_SIZE];    /* Buffer that will receive public key y coordinate */

                        /* Export ECC RAM Public key generated during ECDH operation */
                        pubKey.pmPubKeyCoordX = pubKeyCoordX;
                        pubKey.mPubKeyCoordXSize = C_MAX_PUB_KEY_SIZE;
                        pubKey.pmPubKeyCoordY = pubKeyCoordY;
                        pubKey.mPubKeyCoordYSize = C_MAX_PUB_KEY_SIZE;

                        status = CSE_ECC_exportPublicKey(ECC_RAM_KEY_IDX, &pubKey);
                        if (P_verbose)
                        {
                            printf("Exporting Public Key check: %d (expected: 1)\n", CSE_NO_ERR == status);
                            display_buf("FMR Public Key x      : ", (uint8_t*)pubKey.pmPubKeyCoordX, pubKey.mPubKeyCoordXSize);
                            display_buf("FMR Public Key y      : ", (uint8_t*)pubKey.pmPubKeyCoordY, pubKey.mPubKeyCoordYSize);
                        }
                    }
                }
                else
                {
                    if(P_verbose)
                    {
                        printf("ECDH Key Agreement Failed.\n");
                    }
                }
            }
            else
            {
                if(P_verbose)
                {
                    printf("Load RAM Key Failed.\n");
                }
            }
        }

            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

    } /* End loop on test */

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (P_verbose)
        {
            printf("Curve restored to NIST P_256.\n");
        }
    }
    else
    {
        if (P_verbose)
        {
            printf("Curve restoration to default value failed !\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(failed == 0);
} /*  End of ecc_ecdh_keyAgreement_test */
#endif
/**
 * @}
 */

