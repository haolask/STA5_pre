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
 * @file    CSE_ext_ECC_NVM_KEY_Test.c
 * @brief   ECC Non Volatile Key Tests
 * @details
 * *
 * @addtogroup CSE_driver_test
 * @{
 */

#include "config.h"
#include "serialprintf.h"
#include "string.h"

#include "cse_types.h"
#include "CSE_ext_ECC.h"
#include "CSE_ext_ECC_ECDSA_SignGenVerif_TV.h"
#include "CSE_ext_ECC_NVM_KEY_test.h"
#include "CSE_extendKey_updateSupport.h"
#include "CSE_Manager.h"
#include "CSE_ext_test_globals.h"

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif

#undef DEBUG_PRINT
//#define DEBUG_PRINT

/* Private functions ---------------------------------------------------------*/

/* Exported functions ---------------------------------------------------------*/
extern const verify_test_vect_stt verify_test_vect_array_P256[];

/**
 * @brief  The test ask for the generation by the HSM of an Elliptic Curve stored in NVM and
 *         for generation and verification of a signature with this NVM key.
 *
 * @param verbose
 *
 * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
 * */

uint32_t ecc_generateEcKeyInNVM_SignAndVerify_test(uint32_t verbose)
{
    uint32_t status = 0U;

    uint32_t success = 0U;

    struct ECDSA_Signature_stt ECDSA_Signature;
    uint8_t signatureR[C_MAX_SIG_SIZE];    /* Buffer that will contain R field of ECDSA signature */
    uint8_t signatureS[C_MAX_SIG_SIZE];    /* Buffer that will contain S field of ECDSA signature */

    uint32_t failed = 0U;

    uint32_t tv = 0U;
    uint32_t currentECId = 0U;

    /* Initializes ECDSA structure for signature (r, s) */
    ECDSA_Signature.pSigR = signatureR;
    ECDSA_Signature.pSigS = signatureS;
    ECDSA_Signature.sigR_size = 0U;
    ECDSA_Signature.sigS_size = 0U;

    if(verbose)
    {
        printf("\n");
        printf("Load Elliptic Curve key in NVM \n");
    }
    /* First switch to NIST P256 curve: default value, but allows to re-start the test */
    status = CSE_ECC_changeCurve(C_NIST_P_256);
    if( CSE_NO_ERR == status)
    {
        status = CSE_ECC_getECcurve(&currentECId);

        if (verbose)
        {
        printf("Current Elliptic Curve is: ");
        switch (currentECId)
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
        for (tv = 0U; tv < 8; tv ++)
        {
            uint32_t keyIndex = 0U;
            union EXTENDED_KEY_flags ECflags;
            ECflags.R = 0U;

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
                if (verbose)
                {
                    printf(" ECDSA test vector #1.%d: \n", (tv + 1));
                }
            }

            /* Ask for ECC key pair generation in NVM location at index provided by test vector number */
            keyIndex = (tv % 4) + 1;
            ECflags.B.sign = 1U;      /* Key allowed for signature generation */
            ECflags.B.verify = 1U;    /* Key allowed for signature verification */

            status = CSE_ECC_generateLoadKeyPair(keyIndex, ECflags.R);
            if (verbose)
            {
                printf("Use NVM location index %d\n", keyIndex);
            }

            if (ECC_SUCCESS == status)
            {
                /* Generate signature with NVM key */
                status = CSE_ECC_ECDSA_sign(keyIndex,
                                            (vuint8_t*)verify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                            &ECDSA_Signature, verify_test_vect_array_P256[tv].hashType);
                if (ECC_SUCCESS == status)
                {
                    status = CSE_ECC_ECDSA_verify(keyIndex,
                                                  (vuint8_t*)verify_test_vect_array_P256[tv].msg, C_TV_MSG_SIZE,
                                                  &ECDSA_Signature,
                                                  verify_test_vect_array_P256[tv].hashType,
                                                  &success);
                    if( CSE_NO_ERR == status )
                    {
                        if( 0x0U == success )
                        {
                            status = SIGNATURE_VALID;
                        }
                        else
                        {
                            status = SIGNATURE_INVALID;
                        }
                    }
                }

                /* Set error flag */
                failed += (SIGNATURE_VALID != status);
                if (verbose)
                {
                    printf("Verification check: %d (expected: 1)\n", status == SIGNATURE_VALID);
                }
            }
            else
            {
                if (ECC_NVM_KEY_SLOT_NOT_EMPTY == status)
                {
                    if (verbose)
                    {
                        printf("Key slot in NVM is not empty.\n");
                    }
                }
                else
                {
                    /* unexpected error case during generation and loading */
                    if (verbose)
                    {
                        printf(" generation and storage in NVM failed \n");
                    }
                    failed += 1;
                }

            }
        }
    }
    else
    {
    	failed++;
    }

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
    return(failed == 0);

} /* End of ecc_generateEcKeyInNVM_SignAndVerify_test */

/**
 * @}
 */
