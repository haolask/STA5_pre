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
 * @file    CSE_ext_ECC_ECDSA_test.c
 * @brief   ECDSA signature tests
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
#include "CSE_RNG.h"
#include "CSE_ext_test_globals.h"

#include "CSE_ext_ECC_ECDSA_SignGenVerif_TV.h"
#include "CSE_extendKey_updateSupport.h"
#include "CSE_Manager.h"
#include "CSE_extendKey_updateSupport.h"

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif
#include "test_values.h"

/******************************************************************************/
/************************** ECDSA P_256 Test Vector ***************************/
/******************************************************************************/

/* Private functions ---------------------------------------------------------*/

extern const signVerify_test_vect_stt signVerify_test_vect_array_P256[];
extern const signVerify_test_vect_stt signVerify_test_vect_array_P384[];
extern const signVerify_test_vect_stt signVerify_test_vect_array_P521[];
extern const signVerify_test_vect_stt signVerify_test_vect_array_brainpool256r1[];
extern const signVerify_test_vect_stt signVerify_test_vect_array_brainpool384r1[];

extern const verify_test_vect_stt verify_test_vect_array_P256[];
extern const verify_test_vect_stt verify_test_vect_array_P384[];
extern const verify_test_vect_stt verify_test_vect_array_P521[];
extern const verify_test_vect_stt verify_test_vect_array_brainpoolP256r1[];
extern const verify_test_vect_stt verify_test_vect_array_brainpoolP384r1[];

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
                               uint32_t P_verbose)
{
    struct ECCpubKeyByteArray_stt pubKey;
    struct ECCprivKeyByteArray_stt privKey;

    int32_t status = ECC_ERR_BAD_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* NO public key */
    pubKey.pmPubKeyCoordX = NULL;
    pubKey.mPubKeyCoordXSize = 0;
    pubKey.pmPubKeyCoordY = NULL;
    pubKey.mPubKeyCoordYSize = 0;

    privKey.pmPrivKey = P_pPrivKey->pmPrivKey;
    privKey.mPrivKeySize = P_pPrivKey->mPrivKeySize;

    ret = CSE_ECC_LoadRamKey(&pubKey, &privKey);
    if( CSE_NO_ERR == ret)
    {

        /* Sign with ECDSA */
        ret = CSE_ECC_ECDSA_sign(ECC_RAM_KEY_IDX, (vuint8_t*)P_pInputMessage, P_MessageSize, P_pECDSA_Signature, hash_type);

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
}

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
                             uint32_t P_verbose)
{
    struct ECCpubKeyByteArray_stt pubKey;
    struct ECCprivKeyByteArray_stt privKey;

    int32_t status = ECC_ERR_BAD_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;
    uint32_t success = 0U;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    /* Initializes public key */
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

        /* Verify with ECDSA */
        ret = CSE_ECC_ECDSA_verify(ECC_RAM_KEY_IDX, (vuint8_t*)P_pInputMessage, P_MessageSize, P_pECDSA_Signature, hash_type, &success);

#ifdef PERF_MEASURMENT
        PIT_perfMeasurementInNanoSec(&delay_ticks, P_verbose);
        if (P_verbose)
        {
            printf(" elapsed microsec : %d\n", (uint32_t)PIT_perf_elapsed_microsec64(delay_ticks));

        }
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
            if (success == 0)
            {
                status = SIGNATURE_VALID;
            }
            else
            {
                status = SIGNATURE_INVALID;
            }
        }
    }

    /* default value would be ECC_ERR_BAD_PARAMETER unless generation ran successfully */
    return(status);
}

/**
  * @brief  ECC key pair Generation and store it in specified key location
  * @param  P_eccKeyIndex the index of the key location in which the generated key pair will be stored
  * @param  P_ECflags Key Flag to be applied to generated key
  * @param  P_verbose Verbose or not
  *
  * @retval error status: can be RSA_SUCCESS if success or one of
  * RSA_ERR_BAD_PARAMETER, RSA_ERR_UNSUPPORTED_HASH, RSA_ERR_BAD_KEY, ERR_MEMORY_TEST_FAIL
  * RSA_ERR_MODULUS_TOO_SHORT
*/
int32_t ECC_generateLoadKeyPair(uint32_t P_eccKeyIndex, union EXTENDED_KEY_flags P_ECflags, uint32_t P_verbose)
{
    int32_t status = ECC_ERR_BAD_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;

#ifdef PERF_MEASURMENT
    uint64_t delay_ticks = 0U;
#endif

    ret = CSE_ECC_generateLoadKeyPair(P_eccKeyIndex, P_ECflags.R);
    if( CSE_NO_ERR == ret)
    {

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
}

/* Exported functions ---------------------------------------------------------*/
/**
  * @brief  ECC ECDSA Signature Generation and Verification test
  * @detail The function generates a signature of the provided message and verify the signature
  *         during the same test. It uses pre-defined private and public Keys provided in test vector.
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
  **/
uint32_t ecc_ecdsa_sha_signVerify_test(uint32_t verbose, uint32_t reduced_test_vector_nb )
{
    uint32_t status = 0U;
    struct ECCpubKeyByteArray_stt PubKey_st;    /* Structure that will contain the public key */
    struct ECCprivKeyByteArray_stt PrivKey_st;  /* Structure that will contain the private key*/

    struct ECDSA_Signature_stt ECDSA_Signature;
    uint8_t signatureR[C_MAX_SIG_SIZE];    /* Buffer that will contain R field of ECDSA signature */
    uint8_t signatureS[C_MAX_SIG_SIZE];    /* Buffer that will contain S field of ECDSA signature */

    uint32_t failed = 0U;
    uint32_t pass = 1U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    /* Initializes ECDSA structure for signature (r, s) */
    ECDSA_Signature.pSigR = signatureR;
    ECDSA_Signature.pSigS = signatureS;
    ECDSA_Signature.sigR_size = 0U;
    ECDSA_Signature.sigS_size = 0U;

    if(verbose)
    {
        printf("\n");
        printf("The test generates a signature of the provided message and verified the generated signature.\n");
        printf("It used randomly selected key materials.\n");
    }

    failed = 0;
    /* First switch to NIST P256 curve: default value, but allows to re-start the test */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_P256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC P256 with ");
                if (signVerify_test_vect_array_P256[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (signVerify_test_vect_array_P256[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (signVerify_test_vect_array_P256[tv].hashType == E_SHA512)
                        {
                            printf("SHA512");
                        }
                    }
                }
                if(verbose)
                {
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = signVerify_test_vect_array_P256[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = signVerify_test_vect_array_P256[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            PrivKey_st.pmPrivKey = signVerify_test_vect_array_P256[tv].privateKey;
            PrivKey_st.mPrivKeySize = C_P256_MOD_SIZE;

            status = ECC_signGenerate_ECDSA(&PrivKey_st,
                                            signVerify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                            signVerify_test_vect_array_P256[tv].hashType, &ECDSA_Signature, verbose);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = ECC_signVerify_ECDSA(&PubKey_st,
                                              signVerify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                              signVerify_test_vect_array_P256[tv].hashType,
                                              &ECDSA_Signature, verbose);
                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);

#ifdef INCLUDE_NIST_P384
    if(verbose)
    {
        printf("\n");
    }
    failed = 0;
    /* Switch to NIST P384 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_384);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_P384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC P384 with ");
                if (signVerify_test_vect_array_P384[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (signVerify_test_vect_array_P384[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (signVerify_test_vect_array_P384[tv].hashType == E_SHA512)
                        {
                            printf("SHA512");
                        }
                    }
                }
                if(verbose)
                {
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = signVerify_test_vect_array_P384[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = signVerify_test_vect_array_P384[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            PrivKey_st.pmPrivKey = signVerify_test_vect_array_P384[tv].privateKey;
            PrivKey_st.mPrivKeySize = C_P384_MOD_SIZE;

            status = ECC_signGenerate_ECDSA(&PrivKey_st,
                                            signVerify_test_vect_array_P384[tv].msg, C_TV_MSG_SIZE,
                                            signVerify_test_vect_array_P384[tv].hashType, &ECDSA_Signature, verbose);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = ECC_signVerify_ECDSA(&PubKey_st,
                                              signVerify_test_vect_array_P384[tv].msg, C_TV_MSG_SIZE,
                                              signVerify_test_vect_array_P384[tv].hashType,
                                              &ECDSA_Signature, verbose);
                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
#endif

#ifdef INCLUDE_NIST_P521
    if(verbose)
    {
        printf("\n");
    }
    failed = 0;
    /* Switch to NIST P521 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_521);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_P521;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC P521 with ");
                if (signVerify_test_vect_array_P521[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (signVerify_test_vect_array_P521[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (signVerify_test_vect_array_P521[tv].hashType == E_SHA512)
                        {
                            printf("SHA512");
                        }
                    }
                }
                if(verbose)
                {
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = signVerify_test_vect_array_P521[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P521_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = signVerify_test_vect_array_P521[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P521_MOD_SIZE;

            PrivKey_st.pmPrivKey = signVerify_test_vect_array_P521[tv].privateKey;
            PrivKey_st.mPrivKeySize = C_P521_MOD_SIZE;

            ECDSA_Signature.pSigR = signatureR;
            ECDSA_Signature.pSigS = signatureS;
            ECDSA_Signature.sigR_size = 0U;
            ECDSA_Signature.sigS_size = 0U;

            status = ECC_signGenerate_ECDSA(&PrivKey_st,
                                            signVerify_test_vect_array_P521[tv].msg, C_TV_MSG_SIZE,
                                            signVerify_test_vect_array_P521[tv].hashType, &ECDSA_Signature, verbose);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                /* Initializes ECDSA structure for signature (r, s) */
                ECDSA_Signature.pSigR = signatureR;
                ECDSA_Signature.pSigS = signatureS;

                ECDSA_Signature.sigR_size = C_P521_MOD_SIZE;
                ECDSA_Signature.sigS_size = C_P521_MOD_SIZE;

                status = ECC_signVerify_ECDSA(&PubKey_st,
                                              signVerify_test_vect_array_P521[tv].msg, C_TV_MSG_SIZE,
                                              signVerify_test_vect_array_P521[tv].hashType,
                                              &ECDSA_Signature, verbose);
                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);

#endif

#ifdef INCLUDE_BRAINPOOL_P256R1
    if(verbose)
    {
        printf("\n");
    }
    failed = 0;
    /* Switch to brainpoolP256r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P256R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_brainpoolP256r1_SHA256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing Brainpool P256r1 with ");
                if (signVerify_test_vect_array_brainpool256r1[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = signVerify_test_vect_array_brainpool256r1[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = signVerify_test_vect_array_brainpool256r1[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            PrivKey_st.pmPrivKey = signVerify_test_vect_array_brainpool256r1[tv].privateKey;
            PrivKey_st.mPrivKeySize = C_P256_MOD_SIZE;

            status = ECC_signGenerate_ECDSA(&PrivKey_st,
                                            signVerify_test_vect_array_brainpool256r1[tv].msg, C_TV_MSG_SIZE,
                                            signVerify_test_vect_array_brainpool256r1[tv].hashType, &ECDSA_Signature, verbose);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = ECC_signVerify_ECDSA(&PubKey_st,
                                              signVerify_test_vect_array_brainpool256r1[tv].msg, C_TV_MSG_SIZE,
                                              signVerify_test_vect_array_brainpool256r1[tv].hashType,
                                              &ECDSA_Signature, verbose);
                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
#endif

#ifdef INCLUDE_BRAINPOOL_P384R1
    if(verbose)
    {
        printf("\n");
    }
    /* Switch to brainpoolP384r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P384R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_brainpoolP384r1_SHA384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing Brainpool P384r1 with ");
                if (signVerify_test_vect_array_brainpool384r1[tv].hashType == E_SHA384)
                {
                    printf("SHA384");
                }
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = signVerify_test_vect_array_brainpool384r1[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = signVerify_test_vect_array_brainpool384r1[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            PrivKey_st.pmPrivKey = signVerify_test_vect_array_brainpool384r1[tv].privateKey;
            PrivKey_st.mPrivKeySize = C_P384_MOD_SIZE;

            status = ECC_signGenerate_ECDSA(&PrivKey_st,
                                            signVerify_test_vect_array_brainpool384r1[tv].msg, C_TV_MSG_SIZE,
                                            signVerify_test_vect_array_brainpool384r1[tv].hashType, &ECDSA_Signature, verbose);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = ECC_signVerify_ECDSA(&PubKey_st,
                                              signVerify_test_vect_array_brainpool384r1[tv].msg, C_TV_MSG_SIZE,
                                              signVerify_test_vect_array_brainpool384r1[tv].hashType,
                                              &ECDSA_Signature, verbose);
                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
#endif

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (verbose)
        {
            printf("\nCurve restored to NIST P_256.\n\n");
        }
    }
    else
    {
        if (verbose)
        {
            printf("\nCurve restoration to default value failed !\n\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(pass);


} /* End of ecc_ecdsa_sha_signVerify_test */

/**
  * @brief  ECC ECDSA Signature Generation and Verification test with protected Keys
  * @detail The function generates a signature of the provided message and verify the signature
  *         during the same test. It uses pre-defined private and public Keys provided in test vector.
  *         It loads key in protected form in NVM location
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
  **/
uint32_t ecc_ecdsa_sha_signVerify_PK_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb)
{
    uint32_t status = 0U;
    uint32_t ret = 0U;

    struct ECC_key_stt key;

    struct ECDSA_Signature_stt ECDSA_Signature;
    uint8_t signatureR[C_MAX_SIG_SIZE];    /* Buffer that will contain R field of ECDSA signature */
    uint8_t signatureS[C_MAX_SIG_SIZE];    /* Buffer that will contain S field of ECDSA signature */

    uint32_t failed = 0U;
    uint32_t pass = 1;
    uint32_t check = 0U;
    uint32_t success = 0U;
    uint32_t i = 0U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t keyIndex = 0U;

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 17) + 1] = {0x00000000};    /* To support P521; Must contain at least a multiple of entire number of AES block */
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resLoadM4[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
    uint32_t resLoadM5[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    /* Initializes ECDSA structure for signature (r, s) */
    ECDSA_Signature.pSigR = signatureR;
    ECDSA_Signature.pSigS = signatureS;
    ECDSA_Signature.sigR_size = 0U;
    ECDSA_Signature.sigS_size = 0U;

    if(P_verbose)
    {
        printf("\n");
        printf("The test generates a signature of the provided message and verified the generated signature.\n");
        printf("It used randomly selected key materials.\n");
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
    status = 0U;

    ret = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", ret );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    /* First switch to NIST P256 curve: default value, but allows to re-start the test */
    failed = 0;
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(P_verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_P256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC P256 with ");
                if (signVerify_test_vect_array_P256[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (signVerify_test_vect_array_P256[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (signVerify_test_vect_array_P256[tv].hashType == E_SHA512)
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
            key.PubKeyByteArray.pmPubKeyCoordX = signVerify_test_vect_array_P256[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = signVerify_test_vect_array_P256[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            key.PrivKeyByteArray.pmPrivKey = signVerify_test_vect_array_P256[tv].privateKey;
            key.PrivKeyByteArray.mPrivKeySize = C_P256_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.sign = 1U;          /* Set key as Signature Key */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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

            status = CSE_ECC_ECDSA_sign(keyIndex,
                                        (vuint8_t*)signVerify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                         &ECDSA_Signature, signVerify_test_vect_array_P256[tv].hashType);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = CSE_ECC_ECDSA_verify(keyIndex,
                                              (vuint8_t*)signVerify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                              &ECDSA_Signature,
                                              signVerify_test_vect_array_P256[tv].hashType,
                                              &success);
                /* Translate CSE error code */
                if( CSE_NO_ERR == status )
                {
                    if (success == 0)
                    {
                        status = SIGNATURE_VALID;
                    }
                    else
                    {
                        status = SIGNATURE_INVALID;
                    }
                }

                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (P_verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);

#ifdef INCLUDE_NIST_P384
    if(P_verbose)
    {
        printf("\n");
    }
    /* Switch to NIST P384 curve */
    failed = 0;
    status = CSE_ECC_changeCurve(C_NIST_P_384);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(P_verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_P384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC P384 with ");
                if (signVerify_test_vect_array_P384[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (signVerify_test_vect_array_P384[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (signVerify_test_vect_array_P384[tv].hashType == E_SHA512)
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
            key.PubKeyByteArray.pmPubKeyCoordX = signVerify_test_vect_array_P384[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = signVerify_test_vect_array_P384[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            key.PrivKeyByteArray.pmPrivKey = signVerify_test_vect_array_P384[tv].privateKey;
            key.PrivKeyByteArray.mPrivKeySize = C_P384_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.sign = 1U;          /* Set key as Signature Key */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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

            status = CSE_ECC_ECDSA_sign(keyIndex,
                                        (vuint8_t*)signVerify_test_vect_array_P384[tv].msg, C_TV_MSG_SIZE,
                                        &ECDSA_Signature, signVerify_test_vect_array_P384[tv].hashType);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = CSE_ECC_ECDSA_verify(keyIndex,
                                              (vuint8_t*)signVerify_test_vect_array_P384[tv].msg, C_TV_MSG_SIZE,
                                              &ECDSA_Signature,
                                              signVerify_test_vect_array_P384[tv].hashType,
                                              &success);
                /* Translate CSE error code */
                if( CSE_NO_ERR == status )
                {
                    if (success == 0)
                    {
                        status = SIGNATURE_VALID;
                    }
                    else
                    {
                        status = SIGNATURE_INVALID;
                    }
                }

                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (P_verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;

#endif

#ifdef INCLUDE_NIST_P521
    if(P_verbose)
    {
        printf("\n");
    }
    /* Switch to NIST P521 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_521);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(P_verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_P521;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC P521 with ");
                if (signVerify_test_vect_array_P521[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (signVerify_test_vect_array_P521[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (signVerify_test_vect_array_P521[tv].hashType == E_SHA512)
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
            key.PubKeyByteArray.pmPubKeyCoordX = signVerify_test_vect_array_P521[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P521_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = signVerify_test_vect_array_P521[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P521_MOD_SIZE;

            key.PrivKeyByteArray.pmPrivKey = signVerify_test_vect_array_P521[tv].privateKey;
            key.PrivKeyByteArray.mPrivKeySize = C_P521_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.sign = 1U;          /* Set key as Signature Key */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            ECDSA_Signature.pSigR = signatureR;
            ECDSA_Signature.pSigS = signatureS;
            ECDSA_Signature.sigR_size = 0U;
            ECDSA_Signature.sigS_size = 0U;

            status = CSE_ECC_ECDSA_sign(keyIndex,
                                        (vuint8_t*)signVerify_test_vect_array_P521[tv].msg, C_TV_MSG_SIZE,
                                        &ECDSA_Signature, signVerify_test_vect_array_P521[tv].hashType);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                /* Initializes ECDSA structure for signature (r, s) */
                ECDSA_Signature.pSigR = signatureR;
                ECDSA_Signature.pSigS = signatureS;
                ECDSA_Signature.sigR_size = C_P521_MOD_SIZE;
                ECDSA_Signature.sigS_size = C_P521_MOD_SIZE;

                status = CSE_ECC_ECDSA_verify(keyIndex,
                                              (vuint8_t*)signVerify_test_vect_array_P521[tv].msg, C_TV_MSG_SIZE,
                                              &ECDSA_Signature,
                                              signVerify_test_vect_array_P521[tv].hashType, &success);
                /* Translate CSE error code */
                if( CSE_NO_ERR == status )
                {
                    if (success == 0)
                    {
                        status = SIGNATURE_VALID;
                    }
                    else
                    {
                        status = SIGNATURE_INVALID;
                    }
                }

                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (P_verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }
    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_BRAINPOOL_P256R1
    if(P_verbose)
    {
        printf("\n");
    }
    /* Switch to brainpoolP256r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P256R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(P_verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_brainpoolP256r1_SHA256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing Brainpool P256r1 with ");
                if (signVerify_test_vect_array_brainpool256r1[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = signVerify_test_vect_array_brainpool256r1[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = signVerify_test_vect_array_brainpool256r1[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            key.PrivKeyByteArray.pmPrivKey = signVerify_test_vect_array_brainpool256r1[tv].privateKey;
            key.PrivKeyByteArray.mPrivKeySize = C_P256_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.sign = 1U;          /* Set key as Signature Key */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            status = CSE_ECC_ECDSA_sign(keyIndex,
                                        (vuint8_t*)signVerify_test_vect_array_brainpool256r1[tv].msg, C_TV_MSG_SIZE,
                                        &ECDSA_Signature, signVerify_test_vect_array_brainpool256r1[tv].hashType);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = CSE_ECC_ECDSA_verify(keyIndex,
                                              (vuint8_t*)signVerify_test_vect_array_brainpool256r1[tv].msg, C_TV_MSG_SIZE,
                                              &ECDSA_Signature, signVerify_test_vect_array_brainpool256r1[tv].hashType,
                                              &success);
                /* Translate CSE error code */
                if( CSE_NO_ERR == status )
                {
                    if (success == 0)
                    {
                        status = SIGNATURE_VALID;
                    }
                    else
                    {
                        status = SIGNATURE_INVALID;
                    }
                }

                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (P_verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_BRAINPOOL_P384R1
    if(P_verbose)
    {
        printf("\n");
    }
    /* Switch to brainpoolP384r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P384R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if(P_verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch(currentECId)
        {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#if defined INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#if defined INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIFY_TV_brainpoolP384r1_SHA384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing Brainpool P384r1 with ");
                if (signVerify_test_vect_array_brainpool384r1[tv].hashType == E_SHA384)
                {
                    printf("SHA384");
                }
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = signVerify_test_vect_array_brainpool384r1[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = signVerify_test_vect_array_brainpool384r1[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            key.PrivKeyByteArray.pmPrivKey = signVerify_test_vect_array_brainpool384r1[tv].privateKey;
            key.PrivKeyByteArray.mPrivKeySize = C_P384_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.sign = 1U;          /* Set key as Signature Key */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            status = CSE_ECC_ECDSA_sign(keyIndex,
                                        (vuint8_t*)signVerify_test_vect_array_brainpool384r1[tv].msg, C_TV_MSG_SIZE,
                                        &ECDSA_Signature, signVerify_test_vect_array_brainpool384r1[tv].hashType);
            /* Set Error Flag */
            failed += ((ECC_SUCCESS != status));

            printf(".");

            if (ECC_SUCCESS == status)
            {
                status = CSE_ECC_ECDSA_verify(keyIndex,
                                              (vuint8_t*)signVerify_test_vect_array_brainpool384r1[tv].msg, C_TV_MSG_SIZE,
                                              &ECDSA_Signature, signVerify_test_vect_array_brainpool384r1[tv].hashType,
                                              &success);
                /* Translate CSE error code */
                if( CSE_NO_ERR == status )
                {
                    if (success == 0)
                    {
                        status = SIGNATURE_VALID;
                    }
                    else
                    {
                        status = SIGNATURE_INVALID;
                    }
                }

                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (P_verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
        }
    }
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (P_verbose)
        {
            printf("\nCurve restored to NIST P_256.\n\n");
        }
    }
    else
    {
        if (P_verbose)
        {
            printf("\nCurve restoration to default value failed !\n\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(pass);


} /* End of ecc_ecdsa_sha_signVerify_PK_test */

/**
  * @brief  ECC ECDSA Signature Verification Test
  * @detail The function performs ECDSA signature verification against known signature provided in test vectors
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdsa_sha_signatureVerification_test(uint32_t verbose, uint32_t reduced_test_vector_nb )
{
    uint32_t status = 0U;
    struct ECCpubKeyByteArray_stt PubKey_st;    /* Structure that will contain the public key */
    struct ECDSA_Signature_stt ECDSA_Signature;

    uint32_t failed = 0U;
    uint32_t pass = 1;
    uint32_t tv = 0U;
    uint32_t currentECId = 0U;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

    /* First switch to NIST P256 curve: default value, but allows to re-start the test */
    failed = 0;
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_P256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC P256 with ");
                if (verify_test_vect_array_P256[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (verify_test_vect_array_P256[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (verify_test_vect_array_P256[tv].hashType == E_SHA512)
                        {
                            printf("SHA512");
                        }
                    }
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_P256[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = verify_test_vect_array_P256[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = verify_test_vect_array_P256[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_P256[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_P256[tv].sigS;
            ECDSA_Signature.sigR_size = C_P256_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P256_MOD_SIZE;

            status = ECC_signVerify_ECDSA(&PubKey_st, verify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                          verify_test_vect_array_P256[tv].hashType,
                                          &ECDSA_Signature, verbose);

            printf(".");
            if(verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_P256[tv].expectedResult == TEST_PASS));
            }
            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_P256[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_P256[tv].expectedResult == TEST_FAIL)) ||
                ((status == ECC_ERR_BAD_PARAMETER) && (verify_test_vect_array_P256[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
            {
                if(verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(verbose)
            {
                printf("\n");
            }
        }
    }
    else
    {
        failed++;
    }
    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;

#ifdef INCLUDE_NIST_P384
    /* Test With NIST P_384 curve */
    /* Switch to NIST P384 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_384);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_P384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC P384 with ");
                if (verify_test_vect_array_P384[tv].hashType == E_SHA384)
                {
                    printf("SHA384");
                }
                else
                {
                    if (verify_test_vect_array_P384[tv].hashType == E_SHA512)
                    {
                        printf("SHA512");
                    }
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_P384[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = verify_test_vect_array_P384[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = verify_test_vect_array_P384[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_P384[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_P384[tv].sigS;
            ECDSA_Signature.sigR_size = C_P384_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P384_MOD_SIZE;

            status = ECC_signVerify_ECDSA(&PubKey_st, verify_test_vect_array_P384[tv].msg, C_TV_MSG_SIZE,
                                          verify_test_vect_array_P384[tv].hashType,
                                          &ECDSA_Signature, verbose);

            printf(".");
            if(verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_P384[tv].expectedResult == TEST_PASS));
            }
            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_P384[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_P384[tv].expectedResult == TEST_FAIL)) ||
                ((status == ECC_ERR_BAD_PARAMETER) && (verify_test_vect_array_P384[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
            {
                if(verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(verbose)
            {
                printf("\n");
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_NIST_P521
    /* Test With NIST P_521 curve */
    /* Switch to NIST P521 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_521);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_P521;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC P521 with ");
                if (verify_test_vect_array_P521[tv].hashType == E_SHA512)
                {
                    printf("SHA512");
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_P521[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = verify_test_vect_array_P521[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P521_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = verify_test_vect_array_P521[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P521_MOD_SIZE;

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_P521[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_P521[tv].sigS;
            ECDSA_Signature.sigR_size = C_P521_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P521_MOD_SIZE;

            status = ECC_signVerify_ECDSA(&PubKey_st, verify_test_vect_array_P521[tv].msg, C_TV_MSG_SIZE,
                                          verify_test_vect_array_P521[tv].hashType,
                                          &ECDSA_Signature, verbose);

            printf(".");
            if(verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_P521[tv].expectedResult == TEST_PASS));
            }

            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_P521[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_P521[tv].expectedResult == TEST_FAIL)) ||
                ((status == ECC_ERR_BAD_PARAMETER) && (verify_test_vect_array_P521[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
            {
                if(verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(verbose)
            {
                printf("\n");
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_BRAINPOOL_P256R1
    /* Test with brainpoolP256r1 curve */
    /* Switch to brainpoomP521r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P256R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_brainpoolP256r1_SHA256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC brainpoolP256r1 with ");
                if (verify_test_vect_array_brainpoolP256r1[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = verify_test_vect_array_brainpoolP256r1[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = verify_test_vect_array_brainpoolP256r1[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_brainpoolP256r1[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_brainpoolP256r1[tv].sigS;
            ECDSA_Signature.sigR_size = C_P256_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P256_MOD_SIZE;

            status = ECC_signVerify_ECDSA(&PubKey_st, verify_test_vect_array_brainpoolP256r1[tv].msg, C_TV_MSG_SIZE,
                                          verify_test_vect_array_brainpoolP256r1[tv].hashType,
                                          &ECDSA_Signature, verbose);

            printf(".");
            if(verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_PASS));
            }

            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_FAIL)) ||
                ((status == ECC_ERR_BAD_PARAMETER) && (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
            {
                if(verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(verbose)
            {
                printf("\n");
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_BRAINPOOL_P384R1
    /* Test with brainpoolP384r1 curve */
    /* Switch to brainpoomP384r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P384R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_brainpoolP384r1_SHA384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(verbose)
            {
                printf("Testing ECC brainpoolP384r1 with ");
                if (verify_test_vect_array_brainpoolP384r1[tv].hashType == E_SHA384)
                {
                    printf("SHA384");
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            PubKey_st.pmPubKeyCoordX = verify_test_vect_array_brainpoolP384r1[tv].publicXKey;
            PubKey_st.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            PubKey_st.pmPubKeyCoordY = verify_test_vect_array_brainpoolP384r1[tv].publicYKey;
            PubKey_st.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_brainpoolP384r1[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_brainpoolP384r1[tv].sigS;
            ECDSA_Signature.sigR_size = C_P384_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P384_MOD_SIZE;

            status = ECC_signVerify_ECDSA(&PubKey_st, verify_test_vect_array_brainpoolP384r1[tv].msg, C_TV_MSG_SIZE,
                                          verify_test_vect_array_brainpoolP384r1[tv].hashType,
                                          &ECDSA_Signature, verbose);

            printf(".");
            if(verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_PASS));
            }

            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_FAIL)) ||
                ((status == ECC_ERR_BAD_PARAMETER) && (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_FAIL)) /* In some test vectors, key is not on curve or is not valid */)
            {
                if(verbose)
                {
                    printf("Test succeeded!\n");
                }
            }
            else
            {
                if(verbose)
                {
                    printf("Test Failed!\n");
                }
                failed += 1;
            }
            if(verbose)
            {
                printf("\n");
            }
        }
    }
    else
    {
        failed++;
    }

    if(verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

    /* Restore NIST P256 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        if (verbose)
        {
            printf("Curve restored to NIST P_256.\n");
        }
    }
    else
    {
        if (verbose)
        {
            printf("Curve restoration to default value failed !\n");
        }
    }

    /* return 1 if successful (no test Failed) */
    return(pass);
}

/**
  * @brief  ECC ECDSA Signature Verification Test with NVM Keys
  * @detail The function performs ECDSA signature verification against known signature provided in test vectors
  *         The function uses protected keys located in NVM
  *
  * @param P_verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdsa_sha_signatureVerification_PK_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb )
{
    uint32_t status = 0U;
    uint32_t ret = 0U;

    struct ECC_key_stt key;
    struct ECDSA_Signature_stt ECDSA_Signature;

    uint32_t failed = 0U;
    uint32_t pass = 1;
    uint32_t check = 0U;
    uint32_t success = 0U;
    uint32_t tv = 0U;
    uint32_t i = 0U;
    uint32_t currentECId = 0U;
    uint32_t keyIndex = 0U;

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 17) + 1] = {0x00000000};    /* To support P521; Must contain at least a multiple of entire number of AES block */
    uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t resLoadM4[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
    uint32_t resLoadM5[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };

    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1;
    uint32_t tv_max_number = MAX_REDUCED_TV_NB;

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

    ret = CSE_GetId(challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

    if(P_verbose)
    {
        printf("ERC : %x\n", ret );
        display_buf("UID       \n", (uint8_t*)UID, 16);
    }

    /* Verification only ==>> Private key does not exist ! */
    key.PrivKeyByteArray.pmPrivKey = NULL;
    key.PrivKeyByteArray.mPrivKeySize = 0;

    /* First switch to NIST P256 curve: default value, but allows to re-start the test */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(P_verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_P256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC P256 with ");
                if (verify_test_vect_array_P256[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }
                else
                {
                    if (verify_test_vect_array_P256[tv].hashType == E_SHA384)
                    {
                        printf("SHA384");
                    }
                    else
                    {
                        if (verify_test_vect_array_P256[tv].hashType == E_SHA512)
                        {
                            printf("SHA512");
                        }
                    }
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_P256[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = verify_test_vect_array_P256[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = verify_test_vect_array_P256[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_P256[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_P256[tv].sigS;
            ECDSA_Signature.sigR_size = C_P256_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P256_MOD_SIZE;

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            status = CSE_ECC_ECDSA_verify(keyIndex,
                                          (vuint8_t*)verify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                          &ECDSA_Signature, verify_test_vect_array_P256[tv].hashType,
                                          &success);
            /* Translate CSE error code */
            if( CSE_NO_ERR == status )
            {
                if (success == 0)
                {
                    status = SIGNATURE_VALID;
                }
                else
                {
                    status = SIGNATURE_INVALID;
                }
            }
            else
            {
                status = SIGNATURE_INVALID;
            }

            printf(".");
            if(P_verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_P256[tv].expectedResult == TEST_PASS));
            }
            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_P256[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_P256[tv].expectedResult == TEST_FAIL)) ||
                ((status == CSE_GENERAL_ERR) && (verify_test_vect_array_P256[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
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
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#ifdef INCLUDE_NIST_P384
    /* Test With NIST P_384 curve */
    /* Switch to NIST P384 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_384);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(P_verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_P384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC P384 with ");
                if (verify_test_vect_array_P384[tv].hashType == E_SHA384)
                {
                    printf("SHA384");
                }
                else
                {
                    if (verify_test_vect_array_P384[tv].hashType == E_SHA512)
                    {
                        printf("SHA512");
                    }
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_P384[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = verify_test_vect_array_P384[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = verify_test_vect_array_P384[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_P384[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_P384[tv].sigS;
            ECDSA_Signature.sigR_size = C_P384_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P384_MOD_SIZE;

            status = CSE_ECC_ECDSA_verify(keyIndex,
                                          (vuint8_t*)verify_test_vect_array_P384[tv].msg, C_TV_MSG_SIZE,
                                          &ECDSA_Signature, verify_test_vect_array_P384[tv].hashType,
                                          &success);
            /* Translate CSE error code */
            if( CSE_NO_ERR == status )
            {
                if (success == 0)
                {
                    status = SIGNATURE_VALID;
                }
                else
                {
                    status = SIGNATURE_INVALID;
                }
            }

            printf(".");
            if(P_verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_P384[tv].expectedResult == TEST_PASS));
            }
            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_P384[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_P384[tv].expectedResult == TEST_FAIL)) ||
                ((status == CSE_GENERAL_ERR) && (verify_test_vect_array_P384[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
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
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_NIST_P521
    /* Test With NIST P_521 curve */
    /* Switch to NIST P521 curve */
    status = CSE_ECC_changeCurve(C_NIST_P_521);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(P_verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_P521;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }
        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC P521 with ");
                if (verify_test_vect_array_P521[tv].hashType == E_SHA512)
                {
                    printf("SHA512");
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_P521[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = verify_test_vect_array_P521[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P521_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = verify_test_vect_array_P521[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P521_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_P521[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_P521[tv].sigS;
            ECDSA_Signature.sigR_size = C_P521_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P521_MOD_SIZE;

            status = CSE_ECC_ECDSA_verify(keyIndex,
                                          (vuint8_t*)verify_test_vect_array_P521[tv].msg, C_TV_MSG_SIZE,
                                          &ECDSA_Signature, verify_test_vect_array_P521[tv].hashType,
                                          &success);
            /* Translate CSE error code */
            if( CSE_NO_ERR == status )
            {
                if (success == 0)
                {
                    status = SIGNATURE_VALID;
                }
                else
                {
                    status = SIGNATURE_INVALID;
                }
            }

            printf(".");
            if(P_verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_P521[tv].expectedResult == TEST_PASS));
            }

            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_P521[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_P521[tv].expectedResult == TEST_FAIL)) ||
                ((status == CSE_GENERAL_ERR) && (verify_test_vect_array_P521[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
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
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_BRAINPOOL_P256R1
    /* Test with brainpoolP256r1 curve */
    /* Switch to brainpoomP521r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P256R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(P_verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_brainpoolP256r1_SHA256;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC brainpoolP256r1 with ");
                if (verify_test_vect_array_brainpoolP256r1[tv].hashType == E_SHA256)
                {
                    printf("SHA256");
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = verify_test_vect_array_brainpoolP256r1[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P256_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = verify_test_vect_array_brainpoolP256r1[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P256_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_brainpoolP256r1[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_brainpoolP256r1[tv].sigS;
            ECDSA_Signature.sigR_size = C_P256_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P256_MOD_SIZE;

            status = CSE_ECC_ECDSA_verify(keyIndex,
                                          (vuint8_t*)verify_test_vect_array_brainpoolP256r1[tv].msg, C_TV_MSG_SIZE,
                                          &ECDSA_Signature, verify_test_vect_array_brainpoolP256r1[tv].hashType,
                                          &success);

            /* Translate CSE error code */
            if( CSE_NO_ERR == status )
            {
                if (success == 0)
                {
                    status = SIGNATURE_VALID;
                }
                else
                {
                    status = SIGNATURE_INVALID;
                }
            }

            printf(".");
            if(P_verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_PASS));
            }

            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_FAIL)) ||
                ((status == CSE_GENERAL_ERR) && (verify_test_vect_array_brainpoolP256r1[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
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
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

#ifdef INCLUDE_BRAINPOOL_P384R1
    /* Test with brainpoolP384r1 curve */
    /* Switch to brainpoomP384r1 curve */
    status = CSE_ECC_changeCurve(C_BRAINPOOL_P384R1);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);
        if(P_verbose)
        {
            printf("Current Elliptic Curve is: ");
            switch(currentECId)
            {
            case C_NIST_P_256:
                printf("NIST P_256 !\n");
                break;
#ifdef INCLUDE_NIST_P384
            case C_NIST_P_384:
                printf("NIST P_384 !\n");
                break;
#endif
#ifdef INCLUDE_NIST_P521
            case C_NIST_P_521:
                printf("NIST P_521 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P256R1
            case C_BRAINPOOL_P256R1:
                printf("Brainpool P256r1 !\n");
                break;
#endif
#ifdef INCLUDE_BRAINPOOL_P384R1
            case C_BRAINPOOL_P384R1:
                printf("Brainpool P384r1 !\n");
                break;
#endif
            default:
                printf("Unknown!\n");
                break;
            }
        }

        tv_max_number = NB_OF_ECDSA_SIGN_VERIF_TV_brainpoolP384r1_SHA384;
        if(reduced_test_vector_nb)
        {
            if(tv_max_number > MAX_REDUCED_TV_NB)
            {
                tv_max_number = MAX_REDUCED_TV_NB;
            }
        }

        for(tv = 0; tv < tv_max_number; tv ++)
        {
            if(P_verbose)
            {
                printf("Testing ECC brainpoolP384r1 with ");
                if (verify_test_vect_array_brainpoolP384r1[tv].hashType == E_SHA384)
                {
                    printf("SHA384");
                }

                printf(" Expected test result: ");
                if (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_PASS)
                {
                    printf("Signature verified! \n");
                }
                else
                {
                    printf("Signature not verified! \n");
                }
            }

            /* Initializes Public and Private Key structures with test vectors */
            key.PubKeyByteArray.pmPubKeyCoordX = verify_test_vect_array_brainpoolP384r1[tv].publicXKey;
            key.PubKeyByteArray.mPubKeyCoordXSize = C_P384_MOD_SIZE;
            key.PubKeyByteArray.pmPubKeyCoordY = verify_test_vect_array_brainpoolP384r1[tv].publicYKey;
            key.PubKeyByteArray.mPubKeyCoordYSize = C_P384_MOD_SIZE;

            /* Initializes ECDSA structure with expected signature (r, s) */
            ECDSA_Signature.pSigR = verify_test_vect_array_brainpoolP384r1[tv].sigR;
            ECDSA_Signature.pSigS = verify_test_vect_array_brainpoolP384r1[tv].sigS;
            ECDSA_Signature.sigR_size = C_P384_MOD_SIZE;
            ECDSA_Signature.sigS_size = C_P384_MOD_SIZE;

            key.CNT.R = G_asymKeyCounter;
            key.EC_flags.R = 0U;            /* No restriction on Keys */
            key.EC_flags.B.verify = 1U;      /* Set key as Verification Key */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (tv % 4) + 1;
            if(P_verbose)
            {
                printf("\nLoad NVM Key at index %d.\n", keyIndex);
            }

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
        display_buf("resM4   : ", (uint8_t*)resLoadM4, 32);
        display_buf("resM5   : ", (uint8_t*)resLoadM5, 16);
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

            status = CSE_ECC_ECDSA_verify(keyIndex,
                                          (vuint8_t*)verify_test_vect_array_brainpoolP384r1[tv].msg, C_TV_MSG_SIZE,
                                          &ECDSA_Signature, verify_test_vect_array_brainpoolP384r1[tv].hashType,
                                          &success);

            /* Translate CSE error code */
            if( CSE_NO_ERR == status )
            {
                if (success == 0)
                {
                    status = SIGNATURE_VALID;
                }
                else
                {
                    status = SIGNATURE_INVALID;
                }
            }

            printf(".");
            if(P_verbose)
            {
                printf("Verification check: %d (expected : %d),\n", (status == SIGNATURE_VALID), (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_PASS));
            }

            if (((status == SIGNATURE_VALID) && (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_PASS)) ||
                ((status == SIGNATURE_INVALID) && (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_FAIL)) ||
                ((status == CSE_GENERAL_ERR) && (verify_test_vect_array_brainpoolP384r1[tv].expectedResult == TEST_FAIL))) /* In some test vectors, key is not on curve or is not valid */
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
    else
    {
        failed++;
    }

    if(P_verbose)
    {
        if( failed )
        {
            printf(" - FAILED\n");
        }
        else
        {
            printf(" - PASSED\n");
        }
    }

    pass &= (failed == 0);
    failed = 0;
#endif

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
    return(pass);
}

/**
 * @}
 */
