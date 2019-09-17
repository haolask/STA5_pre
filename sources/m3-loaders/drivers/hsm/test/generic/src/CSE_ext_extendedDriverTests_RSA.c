/*
 * CSE_ext_extendedDriverTests_RSA.c
 *
 *  Created on: 25 juil. 2017
 *      Author: pierre guillemin
 */

/* The file provides a series of tests for RSA PKCS#1 V1.5 with only one test vector per curves.
 * It make one test for RSA1024, RSA2048 and RSA3072
 * It uses RAM key. */

#include <string.h>        /* For memcmp */

#include "config.h"
#include "serialprintf.h"

#include "cse_types.h"
#include "err_codes.h"

#include "CSE_ext_RSA.h"
#include "CSE_ext_RSA_PKCS_TV.h"
#include "CSE_ext_extendedDriverTestsFuncList.h"

extern const TV_allSha_stt TV_EXTDRIV_Rsa_allSsha_1024;
extern const TV_allSha_stt TV_EXTDRIV_Rsa_allSsha_2048;
extern const TV_allSha_stt TV_EXTDRIV_Rsa_allSsha_3072;
extern const uint8_t TV_EXTDRIV_2048_Message[16];
extern const uint8_t TV_EXTDRIV_2048_Modulus[256];
extern const uint8_t TV_EXTDRIV_2048_PrivateExponent[256];
extern const uint8_t TV_EXTDRIV_EncryptedMessage[256];

uint32_t test_CSE_extendedDriver_RSA1024_PKCS1v15SignVerify(uint32_t P_verbose)
{
    uint32_t retVal = 0U;

    struct RSApubKey_stt PubKey_st;        /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st;        /* Structure that will contain the private key*/

    uint8_t Signature[1024 / 8];    /* Buffer that will contain the signature */

    uint32_t check = 1U;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

    PubKey_st.mExponentSize = TV_EXTDRIV_Rsa_allSsha_1024.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_EXTDRIV_Rsa_allSsha_1024.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_1024.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_1024.key.mod;

    PrivKey_st.mExponentSize = TV_EXTDRIV_Rsa_allSsha_1024.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_EXTDRIV_Rsa_allSsha_1024.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_1024.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_1024.key.mod;

    if (P_verbose)
    {
        printf("\n");
        printf (" - Launch RSA1024 PKCS#1v1.5 signature generation and verification. One test vector from NIST CAVP for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    }

    for(tv = 0; tv < 5; tv ++)
    {
        if(P_verbose)
        {
            printf("Testing RSA1024 test vector #1.%d with ", (tv + 1));

            if (TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType == E_SHA1)
            {
                printf("SHA1\n");
            }
            else
            {
                if (TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType == E_SHA224)
                {
                    printf("SHA224\n");
                }
                else
                {
                    if (TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType == E_SHA256)
                    {
                        printf("SHA256\n");
                    }
                    else
                    {
                        if (TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType == E_SHA384)
                        {
                            printf("SHA384\n");
                        }
                        else
                        {
                            if (TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType == E_SHA512)
                            {
                                printf("SHA512\n");
                            }
                        }
                    }
                }
            }

        }
        retVal = RSA_Sign_PKCS1v15(&PrivKey_st, TV_EXTDRIV_Rsa_allSsha_1024.array[tv].msg, TV_EXTDRIV_Rsa_allSsha_1024.array[tv].msg_size,
                                   TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType, Signature, P_verbose);
        if (RSA_SUCCESS == retVal)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_EXTDRIV_Rsa_allSsha_1024.array[tv].sig, TV_EXTDRIV_Rsa_allSsha_1024.array[tv].sig_size);
            if(P_verbose)
            {
                printf("Signature check: %d, (expected: 1)", check == 0);
            }
        }
        else
        {
            if(P_verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((RSA_SUCCESS != retVal) ||  ((RSA_SUCCESS == retVal) && (check != 0)));

        retVal = RSA_Verify_PKCS1v15(&PubKey_st, TV_EXTDRIV_Rsa_allSsha_1024.array[tv].msg, TV_EXTDRIV_Rsa_allSsha_1024.array[tv].msg_size,
                                     TV_EXTDRIV_Rsa_allSsha_1024.array[tv].hashType,
                                     TV_EXTDRIV_Rsa_allSsha_1024.array[tv].sig, P_verbose);
        if(P_verbose)
        {
            printf("Verification check: %d, (expected: 1)\n", retVal == SIGNATURE_VALID);
        }
        failed += (retVal != SIGNATURE_VALID);

    }

    /* return 1 if successful (no test failed) */
    return(failed == 0);

} /* End of test_CSE_extendedDriver_RSA1024_PKCS1v15SignVerify */

uint32_t test_CSE_extendedDriver_RSA2048_PKCS1v15SignVerify(uint32_t P_verbose)
{
    uint32_t retVal = 0U;

    struct RSApubKey_stt PubKey_st;        /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st;        /* Structure that will contain the private key*/

    uint8_t Signature[2048 / 8];    /* Buffer that will contain the signature */

    uint32_t check = 1U;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

    PubKey_st.mExponentSize = TV_EXTDRIV_Rsa_allSsha_2048.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_EXTDRIV_Rsa_allSsha_2048.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_2048.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_2048.key.mod;

    PrivKey_st.mExponentSize = TV_EXTDRIV_Rsa_allSsha_2048.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_EXTDRIV_Rsa_allSsha_2048.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_2048.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_2048.key.mod;

    if (P_verbose)
    {
        printf("\n");
        printf (" - Launch RSA2048 PKCS#1v1.5 signature generation and verification. One test vector from NIST CAVP for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    }

    for(tv = 0; tv < 5; tv ++)
    {
        if(P_verbose)
        {
            printf("Testing RSA2048 test vector #1.%d with ", (tv + 1));

            if (TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType == E_SHA1)
            {
                printf("SHA1\n");
            }
            else
            {
                if (TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType == E_SHA224)
                {
                    printf("SHA224\n");
                }
                else
                {
                    if (TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType == E_SHA256)
                    {
                        printf("SHA256\n");
                    }
                    else
                    {
                        if (TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType == E_SHA384)
                        {
                            printf("SHA384\n");
                        }
                        else
                        {
                            if (TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType == E_SHA512)
                            {
                                printf("SHA512\n");
                            }
                        }
                    }
                }
            }

        }
        retVal = RSA_Sign_PKCS1v15(&PrivKey_st, TV_EXTDRIV_Rsa_allSsha_2048.array[tv].msg, TV_EXTDRIV_Rsa_allSsha_2048.array[tv].msg_size,
                                   TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType, Signature, P_verbose);
        if (RSA_SUCCESS == retVal)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_EXTDRIV_Rsa_allSsha_2048.array[tv].sig, TV_EXTDRIV_Rsa_allSsha_2048.array[tv].sig_size);
            if(P_verbose)
            {
                printf("Signature check: %d, (expected: 1)", check == 0);
            }
        }
        else
        {
            if(P_verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((RSA_SUCCESS != retVal) ||  ((RSA_SUCCESS == retVal) && (check != 0)));

        retVal = RSA_Verify_PKCS1v15(&PubKey_st, TV_EXTDRIV_Rsa_allSsha_2048.array[tv].msg, TV_EXTDRIV_Rsa_allSsha_2048.array[tv].msg_size,
                                     TV_EXTDRIV_Rsa_allSsha_2048.array[tv].hashType,
                                     TV_EXTDRIV_Rsa_allSsha_2048.array[tv].sig, P_verbose);
        if(P_verbose)
        {
            printf("Verification check: %d, (expected: 1)\n", retVal == SIGNATURE_VALID);
        }
        failed += (retVal != SIGNATURE_VALID);

    }

    /* return 1 if successful (no test failed) */
    return(failed == 0);

} /* End of test_CSE_extendedDriver_RSA2048_PKCS1v15SignVerify */

uint32_t test_CSE_extendedDriver_RSA3072_PKCS1v15SignVerify(uint32_t P_verbose)
{
    uint32_t retVal = 0U;

    struct RSApubKey_stt PubKey_st;        /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st;        /* Structure that will contain the private key*/

    uint8_t Signature[3072 / 8];    /* Buffer that will contain the signature */

    uint32_t check = 1U;
    uint32_t failed = 0U;
    uint32_t tv = 0U;

    PubKey_st.mExponentSize = TV_EXTDRIV_Rsa_allSsha_3072.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_EXTDRIV_Rsa_allSsha_3072.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_3072.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_3072.key.mod;

    PrivKey_st.mExponentSize = TV_EXTDRIV_Rsa_allSsha_3072.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_EXTDRIV_Rsa_allSsha_3072.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_3072.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_Rsa_allSsha_3072.key.mod;

    if (P_verbose)
    {
        printf("\n");
        printf (" - Launch RSA3072 PKCS#1v1.5 signature generation and verification. One test vector from NIST CAVP for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    }

    for(tv = 0; tv < 5; tv ++)
    {
        if(P_verbose)
        {
            printf("Testing RSA3072 test vector #1.%d with ", (tv + 1));

            if (TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType == E_SHA1)
            {
                printf("SHA1\n");
            }
            else
            {
                if (TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType == E_SHA224)
                {
                    printf("SHA224\n");
                }
                else
                {
                    if (TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType == E_SHA256)
                    {
                        printf("SHA256\n");
                    }
                    else
                    {
                        if (TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType == E_SHA384)
                        {
                            printf("SHA384\n");
                        }
                        else
                        {
                            if (TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType == E_SHA512)
                            {
                                printf("SHA512\n");
                            }
                        }
                    }
                }
            }

        }
        retVal = RSA_Sign_PKCS1v15(&PrivKey_st, TV_EXTDRIV_Rsa_allSsha_3072.array[tv].msg, TV_EXTDRIV_Rsa_allSsha_3072.array[tv].msg_size,
                                   TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType, Signature, P_verbose);
        if (RSA_SUCCESS == retVal)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_EXTDRIV_Rsa_allSsha_3072.array[tv].sig, TV_EXTDRIV_Rsa_allSsha_3072.array[tv].sig_size);
            if(P_verbose)
            {
                printf("Signature check: %d, (expected: 1)", check == 0);
            }
        }
        else
        {
            if(P_verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((RSA_SUCCESS != retVal) ||  ((RSA_SUCCESS == retVal) && (check != 0)));

        retVal = RSA_Verify_PKCS1v15(&PubKey_st, TV_EXTDRIV_Rsa_allSsha_3072.array[tv].msg, TV_EXTDRIV_Rsa_allSsha_3072.array[tv].msg_size,
                                     TV_EXTDRIV_Rsa_allSsha_3072.array[tv].hashType,
                                     TV_EXTDRIV_Rsa_allSsha_3072.array[tv].sig, P_verbose);
        if(P_verbose)
        {
            printf("Verification check: %d, (expected: 1)\n", retVal == SIGNATURE_VALID);
        }
        failed += (retVal != SIGNATURE_VALID);

    }

    /* return 1 if successful (no test failed) */
    return(failed == 0);

} /* End of test_CSE_extendedDriver_RSA3072_PKCS1v15SignVerify */

uint32_t test_CSE_extendedDriver_RSA2048_PKCS1V15Decryption (uint32_t P_verbose)
{
    uint32_t failed = 0U;

    uint32_t retVal = 0U;
    uint32_t resSize = 0U;
    uint8_t result[256] = {0,};

    struct RSAprivKey_stt privKey_st;

    if (P_verbose)
    {
        printf (" - Launch RSA2048 PKCS#1v1.5 decryption against one known test vector\n");
    }

    privKey_st.mExponentSize = sizeof(TV_EXTDRIV_2048_PrivateExponent);
    privKey_st.mModulusSize = sizeof(TV_EXTDRIV_2048_Modulus);
    privKey_st.pmExponent = (uint8_t *)TV_EXTDRIV_2048_PrivateExponent;
    privKey_st.pmModulus = (uint8_t *)TV_EXTDRIV_2048_Modulus;

    retVal = RSA_Decrypt_PKCS1v15(&privKey_st, TV_EXTDRIV_EncryptedMessage, privKey_st.mModulusSize, result, &resSize, P_verbose);
    if (RSA_SUCCESS != retVal)
    {
        if(P_verbose)
        {
            printf("Error in RSA Decryption!\n");
        }
        failed += 1;
    }
    else
    {
        if(P_verbose)
        {
            display_buf("De-Ciphertext:", result, resSize);
        }
        if (memcmp(TV_EXTDRIV_2048_Message, result, sizeof(TV_EXTDRIV_2048_Message)) != 0 || (sizeof(TV_EXTDRIV_2048_Message) != resSize))
        {
            if(P_verbose)
            {
                printf("De-Ciphered text differs from cleartext\n");
            }
            failed += 1;
        }
    }

    return (failed == 0);

} /* End of test_CSE_extendedDriver_RSA2048_PKCS1V15Decryption */
