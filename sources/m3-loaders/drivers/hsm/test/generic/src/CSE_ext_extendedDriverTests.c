/*
 * CSE_ext_extendedDriverTests.c
 *
 *  Created on: 18 juil. 2017
 *      Author: pierre guillemin
 */

#include <string.h>        /* For memcmp */

#include "config.h"
#include "serialprintf.h"
#include "test_support.h"

#include "cse_types.h"

#include "CSE_ext_extendedDriverTestsFuncList.h"
#include "CSE_ext_ECC_TV_consts.h"

uint32_t test_CSE_extendedDriver_HASH(uint32_t P_verbose)
{
    uint32_t pass = 1U;

    pass = HASH_tests(P_verbose);

    return (pass);
}

uint32_t test_CSE_extendedDriver(uint32_t P_verbose)
{
    uint32_t success = 1U;
    uint32_t pass = 1U;

#if 0
    if(P_verbose){

        printf("\n******************************\n");
        printf("\nRunning tests in 'silent' mode\n");
        printf("\n******************************\n\n");
    }
#endif
    if(P_verbose)
    {
        printf("\n*******************************\n");
        printf("\nRunning tests in 'verbose' mode\n");
        printf("\n*******************************\n\n");
    }

    /*************************************************/
    /* Launch AES256 tests, one test vector per test */
    /*************************************************/
    /* AES256 ECB Encryption / Decryption */
    pass = test_CSE_extendedDriver_AES256_ECB(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* AES256 CBC Encryption / Decryption */
    pass = test_CSE_extendedDriver_AES256_CBC(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* AES256 CCM Generation-Encryption and Decryption-Verification */
    pass = test_CSE_extendedDriver_AES256_CCM(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* AES256 CMAC MAC tag Generation and Verification */
    pass = test_CSE_extendedDriver_AES256_CMAC(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* AES256 GCM Authenticated-Encryption and Authenticated-Decryption */
    pass = test_CSE_extendedDriver_AES256_GCM(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /*****************************************************************************************/
    /* Launch HASH tests (SHA1, SHA224, SHA256, SHA384, SHA512), three test vectors per test */
    /*****************************************************************************************/
    pass = test_CSE_extendedDriver_HASH(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;
#if defined(HMAC_TESTS_ENABLED)
    /************************************************************************************************************/
    /* Launch HMAC MAC TAG generation tests with SHA1, SHA224, SHA256, SHA384, SHA512, one test vector per test */
    /************************************************************************************************************/
    pass = test_CSE_extendedDriver_HMAC_macGeneration(P_verbose, 1);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /**************************************************************************************************************/
    /* Launch HMAC MAC TAG verification tests with SHA1, SHA224, SHA256, SHA384, SHA512, one test vector per test */
    /**************************************************************************************************************/
    pass = test_CSE_extendedDriver_HMAC_macVerification(P_verbose, 1);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;
#endif
    /***********************************************************************************************************************/
    /* Launch ECC tests series: one Test vector for each curves: NIST P256, P384, P521, Brainpool P256r1, Brainpool P384r1 */
    /*  - ECDSA signature generation and verification (verify what was signed, no test vector)                             */
    /*  - ECDSA signature verification against test vector                                                                 */
    /*  - ECIES NIST P256 encryption decryption (decrypt what was encrypted, no test vector)                               */
    /*  - ECIES NIST P256 decryption against one known test vector                                                         */
    /**********************************************************************************************************************/
    {
        /* ECC ECDSA NIST P256 */
        pass = test_CSE_extendedDriver_ECC_ECDSA(P_verbose, C_NIST_P_256);
        if (P_verbose)
        {
            display_pass_fail(pass);
        }
        success &= pass;

        /* ECC ECDSA NIST P384 */
        pass = test_CSE_extendedDriver_ECC_ECDSA(P_verbose, C_NIST_P_384);
        if (P_verbose)
        {
            display_pass_fail(pass);
        }
        success &= pass;

        /* ECC ECDSA BRAINPOOL P256R1 */
        pass = test_CSE_extendedDriver_ECC_ECDSA(P_verbose, C_BRAINPOOL_P256R1);
        if (P_verbose)
        {
            display_pass_fail(pass);
        }
        success &= pass;

        /* ECC ECDSA BARINPOOL P384R1 */
        pass = test_CSE_extendedDriver_ECC_ECDSA(P_verbose, C_BRAINPOOL_P384R1);
        if (P_verbose)
        {
            display_pass_fail(pass);
        }
        success &= pass;

        /* ECC ECDSA NIST P521 */
        pass = test_CSE_extendedDriver_ECC_ECDSA(P_verbose, C_NIST_P_521);
        if (P_verbose)
        {
            display_pass_fail(pass);
        }
        success &= pass;
    }

    {
        /* ECC ECIES NIST P256
         * Only one implementation of ECIES is supported by the library
         * - Diffie-Hellman for Key Agreement
         * - ANS X9.63 for Key Derivation Function
         * - AES-128 in CBC mode for Encryption and IV value at zero
         * - CMAC-AES-128 for CMAC tag computation
         * - Elliptic Curve Cryptography with NIST P256 curve
         */
        pass = test_CSE_extendedDriver_ECC_P256_ECIES(P_verbose);
        if (P_verbose)
        {
            display_pass_fail(pass);
        }
        success &= pass;
    }
    /************************************************************************************************/
    /* Launch RSA tests series: one Test vector for each modulus size: RSA 1024, RSA2048, RSA 3072  */
    /*  - RSA PKCS#1 V1.5 signature generation and verification against test vector                 */
    /*  - RSA PKCS#1 V1.5 decryption against one known test vector                                  */
    /************************************************************************************************/

    /* RSA1024 PKCS#1V1.5 Signature Generation and Verification */
    pass = test_CSE_extendedDriver_RSA1024_PKCS1v15SignVerify(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* RSA2048 PKCS#1V1.5 Signature Generation and Verification */
    pass = test_CSE_extendedDriver_RSA2048_PKCS1v15SignVerify(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* RSA3072 PKCS#1V1.5 Signature Generation and Verification */
    pass = test_CSE_extendedDriver_RSA3072_PKCS1v15SignVerify(P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /* RSA2048 PKCS#1V1.5 Message Decryption */
    pass = test_CSE_extendedDriver_RSA2048_PKCS1V15Decryption (P_verbose);
    if (P_verbose)
    {
        display_pass_fail(pass);
    }
    success &= pass;

    /**********************/
    /* End of test series */
    /**********************/
    if (P_verbose)
    {
        printf("\n");
    }

    return (success);

} /* End of test_CSE_extendedDriver */
