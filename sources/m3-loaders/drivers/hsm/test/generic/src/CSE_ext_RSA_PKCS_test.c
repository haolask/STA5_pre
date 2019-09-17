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
 * @file    CSE_ext_RSA_PKCS_test.c
 * @brief   pkcs signature tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */
#include "config.h"
#include "serialprintf.h"

#include "config.h"
#include "cse_types.h"
#include "CSE_ext_RSA.h"

#include "CSE_RNG.h"
#include "string.h"

#ifdef ST_TEST_MODE
#include "cse_st_ext_rsa_pkcs_test.h"
#endif

#include "err_codes.h"
#ifdef PERF_MEASURMENT
#include "pit_perf_meas.h"
#endif
#include "CSE_Manager.h"
#include "CSE_Constants.h"
#include "CSE_extendKey_updateSupport.h"

#include "CSE_ext_RSA_PKCS_TV.h"
#include "CSE_ext_test_globals.h"
#include "test_values.h"

#undef DEBUG_PRINT

uint32_t M3[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
uint32_t M4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                  0x00000000, 0x00000000, 0x00000000, 0x00000000};
uint32_t M5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

uint32_t resM4[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                     0x00000000, 0x00000000, 0x00000000, 0x00000000};
uint32_t resM5[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};


/* Private functions ---------------------------------------------------------*/
static uint32_t load_RSA_key_test( struct RSA_key_stt* key,
                                   uint32_t* authKey, uint32_t* UID,
                                   uint32_t keyIndex, uint32_t authKeyId,
                                   uint32_t* M1, uint32_t* M2, uint32_t* M3,
                                   uint32_t* M4, uint32_t* M5,
                                   uint32_t verbose)
{
    uint32_t pass = 1;
    uint32_t status;
    uint32_t check;

    if (verbose)
    {
        printf("Load RSA NVM key at index %d\n", keyIndex);
    }

    status = CSE_extendKeyGenerate_M1_to_M5((uint32_t *)key->PrivKey.pmModulus,
                                            key->PrivKey.mModulusSize,
                                            (uint32_t *)key->PubKey.pmExponent,
                                            key->PubKey.mExponentSize,
                                            (uint32_t *)key->PrivKey.pmExponent,
                                            key->PrivKey.mExponentSize,

                                            authKey, (uint32_t *)UID,
                                            keyIndex, authKeyId ,
                                            key->CNT.R, key->RSA_flags.R,
                                            M1, M2, M3,
                                            M4, M5);

    if(status != CSE_NO_ERR)
    {
        if(verbose)
        {
            printf(" - Problem when generating M1..M5 messages\n");
        }
        pass = 0;
    }
    else
    {
        status += CSE_RSA_LoadKey((uint8_t*)M1, (uint8_t*)M2, (uint8_t*)M3, (uint8_t*)resM4, (uint8_t*)resM5);
        G_asymKeyCounter++;    /* For next test */

        if( verbose)
        {
            printf("load command returned error code Err=%x\n", status );
        }

        check = memcmp(M4, resM4, 32);
        if( check == 0 )
        {
            if(verbose)
            {
                printf(" - M4 Message as expected\n");
            }
        }
        else
        {
            if(verbose)
            {
                printf(" - M4 Message is not the expected one\n");
                display_buf("  - resM4   : ", (uint8_t*)resM4, 32);
                display_buf("  - expected: ", (uint8_t*)M4, 32);
            }
            pass = 0;
        }
        check = memcmp(M5, resM5, 16);
        if( check == 0 )
        {
            if(verbose)
            {
                printf(" - M5 Message as expected\n");
            }
        }
        else
        {
            if(verbose)
            {
                printf(" - M5 Message is not the expected one\n");
                display_buf("  - resM5   : ", (uint8_t*)resM5, 16);
                display_buf("  - expected: ", (uint8_t*)M5, 16);
            }
            pass = 0;
        }
    }
    return(pass);
}

uint32_t signature_verification_test( uint32_t keyIndex, vuint8_t* msg, uint32_t msg_size, vuint8_t* sig, uint32_t hash_type, uint32_t* psuccess, uint32_t expected, uint32_t verbose)
{
    uint32_t ret;
    uint32_t pass = 1;

    /* Verify signature with RSA */
    ret = CSE_RSA_PKCS1_15_verify((vuint32_t)keyIndex, (vuint8_t*)msg, msg_size, (vuint8_t*)sig, hash_type, psuccess );

    /* Translate CSE error code */
    if( CSE_NO_ERR == ret )
    {
        if (0U == *psuccess )
        {
            *psuccess = SIGNATURE_VALID;
        }
        else
        {
            *psuccess = SIGNATURE_INVALID;
            pass = 0;
        }

		if(expected == *psuccess)
		{
			if(verbose)
			{
				printf(" - Verification ");
				if(expected == SIGNATURE_VALID)
				{
					printf("succeeded ");
				}
				else
				{
					printf("failed ");
				}
				printf("as expected\n");
			}
		}
		else
		{
			if(verbose)
			{
				printf(" - Verification did not ");
				if(expected == SIGNATURE_VALID)
				{
					printf("succeed ");
				}
				else
				{
					printf("fail ");
				}
				printf("as it should have\n");
			}
			pass = 0;
		}
    }
    else
    {
		*psuccess = SIGNATURE_INVALID;
        if(verbose)
        {
            printf(" - Execution Failed\n ");
        }
        pass = 0;
    }

    return(pass);
}


uint32_t signature_generation_test( uint32_t keyIndex,
                                    vuint8_t * msg,
                                    uint32_t msg_size,
                                    vuint8_t* sig, uint32_t hash_type,
                                    const uint8_t* ref_sig, uint32_t ref_sig_size,
                                    uint32_t verbose)
{
    uint32_t check;
    uint32_t pass = 1;
    uint32_t status;

    status = CSE_RSA_PKCS1_15_sign(keyIndex, (vuint8_t*)msg, msg_size, (vuint8_t*) sig, hash_type );
    if (status == RSA_SUCCESS)
    {
        /* Compare the Signature */
        check = memcmp((uint8_t*)sig, ref_sig, ref_sig_size);
        if(check == 0)
        {
            if(verbose)
            {
                printf(" - Signature is the expected one\n");
            }
        }
        else
        {
            if(verbose)
            {
                printf(" - Signature is not the expected one - failed \n");
            }
            pass = 0;
        }

        if(check != 0) pass = 0;
    }
    else
    {
        if(verbose)
        {
            printf(" - Execution Failed, ");
        }
        pass = 0;
    }
    return(pass);
}

static inline bool hsm_addr_unaligned(const void *addr, uint8_t write_bytes)
{
	return ((unsigned long)addr & (write_bytes - 1));
}

/**
  * @brief  RSA Signature Generation with PKCS#1v1.5
  * @param  P_pPrivKey The RSA private key structure, already initialized
  * @param  P_pInputMessage Input Message to be signed
  * @param  P_MessageSize Size of input message
  * @param  hash_type type of hash to use (E_SHA1, E_SHA224, E_SHA256 ...)
  * @param  P_pOutput Pointer to output buffer
  * @retval error status: can be RSA_SUCCESS if success or one of
  * RSA_ERR_BAD_PARAMETER, RSA_ERR_UNSUPPORTED_HASH, RSA_ERR_BAD_KEY, ERR_MEMORY_FAIL
  * RSA_ERR_MODULUS_TOO_SHORT
*/
int32_t RSA_Sign_PKCS1v15(struct RSAprivKey_stt * P_pPrivKey,
                          const uint8_t * P_pInputMessage, int32_t P_MessageSize,
                          uint32_t hash_type,
                          uint8_t *P_pOutput, uint32_t verbose)
{
    int32_t status = RSA_ERR_BAD_PARAMETER;
    uint32_t ret = CSE_GENERAL_ERR;;

    struct RSA_simple_struct modulus;
    struct RSA_simple_struct pubkey;
    struct RSA_simple_struct privkey;

    modulus.address = P_pPrivKey->pmModulus;
    modulus.bytesize = P_pPrivKey->mModulusSize;

    /* NO public key */
    pubkey.address = NULL;
    pubkey.bytesize = 0;

    privkey.address = P_pPrivKey->pmExponent;
    privkey.bytesize = P_pPrivKey->mExponentSize;

    ret = CSE_RSA_LoadRamKey( &modulus, &pubkey, &privkey );
    if( CSE_NO_ERR == ret)
    {
        /* Sign with RSA */
        ret = CSE_RSA_PKCS1_15_sign(RSA_RAM_KEY, (vuint8_t*)P_pInputMessage, P_MessageSize, (vuint8_t*) P_pOutput, hash_type );

#ifdef PERF_MEASURMENT
        display_elapsed_time64( verbose );
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
            status = RSA_SUCCESS;
        }
        else
        {
            status = ret;
        }
    }

  /* default value would be RSA_ERR_BAD_PARAMETER unless generation ran successfully */
  return (status);
}

/**
  * @brief  RSA Signature Verification with PKCS#1v1.5
  * @param  P_pPubKey The RSA public key structure, already initialized
  * @param  P_pInputMessage Input Message
  * @param  P_MessageSize Size of input message
  * @param  hash_type type of hash to use (E_SHA1, E_SHA224, E_SHA256 ...)
  * @param  P_pSignature Signature that will be checked
  *
  * @retval error status: can be RSA_SUCCESS if success or one of
  * RSA_ERR_BAD_PARAMETER, RSA_ERR_UNSUPPORTED_HASH, RSA_ERR_BAD_KEY, ERR_MEMORY_FAIL
  * RSA_ERR_MODULUS_TOO_SHORT
  *
  * SIGNATURE_VALID
  * SIGNATURE_INVALID
*/
int32_t RSA_Verify_PKCS1v15(struct RSApubKey_stt *P_pPubKey,
                            const uint8_t *P_pInputMessage,
                            int32_t P_MessageSize,
                            uint32_t hash_type,
                            const uint8_t *P_pSignature, uint32_t verbose)
{
    int32_t retval = SIGNATURE_INVALID;
    uint32_t ret  = CSE_GENERAL_ERR;
    uint32_t status;

	uint8_t paload[2048 / 8];
	void *data = (uint8_t *) P_pSignature;

	if (hsm_addr_unaligned(data, 4)) {
		memcpy(paload, data, 256);
		P_pSignature = paload;
	}

    struct RSA_simple_struct modulus;
    struct RSA_simple_struct pubkey;
    struct RSA_simple_struct privkey;

    modulus.address = P_pPubKey->pmModulus;
    modulus.bytesize = P_pPubKey->mModulusSize;

    /* NO public key */
    privkey.address = NULL;
    privkey.bytesize = 0;

    pubkey.address = P_pPubKey->pmExponent;
    pubkey.bytesize = P_pPubKey->mExponentSize;

    ret = CSE_RSA_LoadRamKey( &modulus, &pubkey, &privkey );

    if( CSE_NO_ERR == ret )
    {
        /* Verify signature with RSA */
        ret = CSE_RSA_PKCS1_15_verify(RSA_RAM_KEY, (vuint8_t*)P_pInputMessage, P_MessageSize, (vuint8_t*) P_pSignature, hash_type, &status );

#ifdef PERF_MEASURMENT
        display_elapsed_time64( verbose );
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
            if( 0x00 == status )
            {
                retval = SIGNATURE_VALID;
            }
            else
            {
                retval = SIGNATURE_INVALID;
            }
        }
        else
        {
            retval = ret;
        }
    }
    return (retval);
}


int32_t RSA_Encrypt_PKCS1v15(struct RSApubKey_stt *P_pPubKey,
                             const uint8_t *P_pInputMessage,
                             uint32_t P_InputSize,
                             uint8_t *P_pOutput,
                             uint32_t* P_OutputSize, uint32_t verbose )
{
    int32_t retval = RSA_ERR_BAD_PARAMETER;
    uint32_t ret  = CSE_GENERAL_ERR;

    struct RSA_simple_struct modulus;
    struct RSA_simple_struct pubkey;
    struct RSA_simple_struct privkey;

    /* Encrypt the message, this function will write sizeof(modulus) data */

    modulus.address = P_pPubKey->pmModulus;
    modulus.bytesize = P_pPubKey->mModulusSize;

    /* NO private key needed */
    privkey.address = NULL;
    privkey.bytesize = 0;

    pubkey.address = P_pPubKey->pmExponent;
    pubkey.bytesize = P_pPubKey->mExponentSize;

    ret = CSE_RSA_LoadRamKey(&modulus, &pubkey, &privkey);

    if( CSE_NO_ERR == ret )
    {
        /* Perform RSA PKCS1v1.5 encryption with public key */
        ret = CSE_RSA_PKCS1_15_encrypt(RSA_RAM_KEY, (vuint8_t*)P_pInputMessage, P_InputSize, (vuint8_t*) P_pOutput, P_OutputSize );

#ifdef PERF_MEASURMENT
        display_elapsed_time64( verbose );
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
                retval = RSA_SUCCESS;
        }
        else
        {
            retval = ret;
        }
    }
    return (retval);
}

int32_t RSA_Decrypt_PKCS1v15(struct RSAprivKey_stt * P_pPrivKey,
                             const uint8_t * P_pInputMessage,
                             uint32_t P_InputSize,
                             uint8_t *P_pOutput,
                             uint32_t *P_OutputSize, uint32_t verbose)
{
    int32_t retval = RSA_ERR_BAD_PARAMETER;
    uint32_t ret  = CSE_GENERAL_ERR;

    struct RSA_simple_struct modulus;
    struct RSA_simple_struct pubkey;
    struct RSA_simple_struct privkey;

    /* Encrypt the message, this function will write sizeof(modulus) data */

    modulus.address = P_pPrivKey->pmModulus;
    modulus.bytesize = P_pPrivKey->mModulusSize;

    /* NO private key needed */
    privkey.address = P_pPrivKey->pmExponent;
    privkey.bytesize = P_pPrivKey->mExponentSize;

    pubkey.address = NULL;
    pubkey.bytesize = 0;

    ret = CSE_RSA_LoadRamKey( &modulus, &pubkey, &privkey );

    if( CSE_NO_ERR == ret )
    {
        /* Perform RSA PKCS1v1.5 encryption with public key */
        ret = CSE_RSA_PKCS1_15_decrypt(RSA_RAM_KEY, (vuint8_t*)P_pInputMessage, P_InputSize, (vuint8_t*) P_pOutput, P_OutputSize );

#ifdef PERF_MEASURMENT
        display_elapsed_time64( verbose );
#endif

        /* Translate CSE error code */
        if( CSE_NO_ERR == ret )
        {
            retval = RSA_SUCCESS;
        }
        else
        {
            retval = ret;
        }
    }
    return (retval);
}

/* exported functions */
uint32_t pkcs_sha1_1024_signature_test( uint32_t verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[1024 / 8]; /* Buffer that will contain the signature */
    int32_t check = 1;
    uint32_t failed = 0;
    uint32_t tv;

    PubKey_st.mExponentSize = TV_sha1_1024.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_sha1_1024.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_sha1_1024.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;

    PrivKey_st.mExponentSize = TV_sha1_1024.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_sha1_1024.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_sha1_1024.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;

    printf (" - Launch RSA1024 PKCS#1v1.5 signature generation and verification for SHA1\n");
    for(tv = 0; tv < nb_tv_1024; tv ++)
    {
        if(verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv+1) );
        }
        status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_sha1_1024.array[tv].msg, TV_sha1_1024.array[tv].msg_size,
                                   E_SHA1, Signature, verbose);
        if (status == RSA_SUCCESS)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_sha1_1024.array[tv].sig, TV_sha1_1024.array[tv].sig_size);
            if(verbose)
            {
                printf("Signature check: %d, ", check==0);
            }
        }
        else
        {
            if(verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((status != RSA_SUCCESS) ||  ( (status == RSA_SUCCESS) && (check != 0) ));

        status = RSA_Verify_PKCS1v15(&PubKey_st, TV_sha1_1024.array[tv].msg, TV_sha1_1024.array[tv].msg_size,
                                     E_SHA1,
                                     TV_sha1_1024.array[tv].sig, verbose);
        if(verbose)
        {
            printf("Verification check: %d\n", status==SIGNATURE_VALID);
        }
        failed += (status != SIGNATURE_VALID);
        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_sha1_1024_signature_test */

uint32_t pkcs_allSha_1024_signature_test( uint32_t verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[1024 / 8]; /* Buffer that will contain the signature */
    int32_t check = 1;
    uint32_t failed = 0;
    uint32_t tv;

    PubKey_st.mExponentSize = TV_allSsha_1024.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_allSsha_1024.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_allSsha_1024.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_allSsha_1024.key.mod;

    PrivKey_st.mExponentSize = TV_allSsha_1024.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_allSsha_1024.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_allSsha_1024.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_allSsha_1024.key.mod;

    printf (" - Launch RSA1024 PKCS#1v1.5 signature generation and verification with 10 test vectors from NIST CAVP for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    for(tv = 0; tv < C_NB_TV_ALL_SHA_1024; tv ++)
    {
        if(verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1) );
        }
        status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_allSsha_1024.array[tv].msg, TV_allSsha_1024.array[tv].msg_size,
                                   TV_allSsha_1024.array[tv].hashType, Signature, verbose);
        if (status == RSA_SUCCESS)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_allSsha_1024.array[tv].sig, TV_allSsha_1024.array[tv].sig_size);
            if(verbose)
            {
                printf("Signature check: %d, ", check==0);
            }
        }
        else
        {
            if(verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((status != RSA_SUCCESS) ||  ( (status == RSA_SUCCESS) && (check != 0) ));

        status = RSA_Verify_PKCS1v15(&PubKey_st, TV_allSsha_1024.array[tv].msg, TV_allSsha_1024.array[tv].msg_size,
                                     TV_allSsha_1024.array[tv].hashType,
                                     TV_allSsha_1024.array[tv].sig, verbose);
        if(verbose)
        {
            printf("Verification check: %d\n", status==SIGNATURE_VALID);
        }
        failed += (status != SIGNATURE_VALID);
        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_allSha_1024_signature_test */

uint32_t pkcs_allSha_1024_signatureVerify_test( uint32_t P_verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */

    uint32_t failed = 0;
    uint32_t tv;
    uint32_t signVerify = 0U;

    printf (" - Launch RSA1024 PKCS#1v1.5 signature verification against known test vectors from (NIST CAVS 11.0) for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    for(tv = 0; tv < C_NB_TV_SIGN_VERIFY_ALL_SHA_1024; tv ++)
    {
        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1) );
        }

        PubKey_st.mExponentSize = test_vect_signVerify_allSha_1024[tv].pubexp_byte_size;
        PubKey_st.mModulusSize = test_vect_signVerify_allSha_1024[tv].mod_byte_size;
        PubKey_st.pmExponent = (uint8_t *)test_vect_signVerify_allSha_1024[tv].pubexp;
        PubKey_st.pmModulus = (uint8_t *)test_vect_signVerify_allSha_1024[tv].mod;

        status = RSA_Verify_PKCS1v15(&PubKey_st, test_vect_signVerify_allSha_1024[tv].msg, test_vect_signVerify_allSha_1024[tv].msg_size,
                                     test_vect_signVerify_allSha_1024[tv].hashType,
                                     test_vect_signVerify_allSha_1024[tv].sig, P_verbose);
        /* Translate CSE error code */
        if ((SIGNATURE_VALID == status) && ('P' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
        {
            signVerify = SIGNATURE_VALID;
            failed += (signVerify != SIGNATURE_VALID);
            if(P_verbose)
            {
                printf ("Verification check: ");
                if (signVerify == SIGNATURE_VALID)
                {
                    printf("Pass; ");
                }
                else
                {
                    printf("Fail; ");
                }
                printf("(expected: ");
                if (('P' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
                {
                    printf("Pass;) Test Pass.\n");
                }
                else
                {
                    printf("Fail; Test Fail.\n)");
                    failed++;
                }
            }
        }
        else
        {
            if ((SIGNATURE_INVALID == status) && ('F' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
            {
                signVerify = SIGNATURE_INVALID;
                failed += (signVerify != SIGNATURE_INVALID);
                if(P_verbose)
                {
                    printf ("Verification check: ");
                    if (signVerify == SIGNATURE_INVALID)
                    {
                        printf("Fail; ");
                    }
                    else
                    {
                        printf("Pass; ");
                    }
                    printf("(expected: ");
                    if (('F' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
                    {
                        printf("Fail;) Test Pass.\n");
                    }
                    else
                    {
                        printf("Pass;) Test Fail.\n");
                        failed++;
                    }
                }
            }
        }

        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_allSha_1024_signatureVerify_test */

uint32_t pkcs_allSha_2048_signatureVerify_test( uint32_t P_verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */

    uint32_t failed = 0;
    uint32_t tv;
    uint32_t signVerify = 0U;

    printf (" - Launch RSA2048 PKCS#1v1.5 signature verification against known test vectors from (NIST CAVS 11.0) for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    for(tv = 0; tv < C_NB_TV_SIGN_VERIFY_ALL_SHA_2048; tv ++)
    {
        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1) );
        }

        PubKey_st.mExponentSize = test_vect_signVerify_allSha_2048[tv].pubexp_byte_size;
        PubKey_st.mModulusSize = test_vect_signVerify_allSha_2048[tv].mod_byte_size;
        PubKey_st.pmExponent = (uint8_t *)test_vect_signVerify_allSha_2048[tv].pubexp;
        PubKey_st.pmModulus = (uint8_t *)test_vect_signVerify_allSha_2048[tv].mod;

        status = RSA_Verify_PKCS1v15(&PubKey_st, test_vect_signVerify_allSha_2048[tv].msg, test_vect_signVerify_allSha_2048[tv].msg_size,
                                     test_vect_signVerify_allSha_2048[tv].hashType,
                                     test_vect_signVerify_allSha_2048[tv].sig, P_verbose);
        /* Translate CSE error code */
        if ((SIGNATURE_VALID == status) && ('P' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
        {
            signVerify = SIGNATURE_VALID;
            failed += (signVerify != SIGNATURE_VALID);
            if(P_verbose)
            {
                printf ("Verification check: ");
                if (signVerify == SIGNATURE_VALID)
                {
                    printf("Pass; ");
                }
                else
                {
                    printf("Fail; ");
                }
                printf("(expected: ");
                if (('P' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
                {
                    printf("Pass;) Test Pass.\n");
                }
                else
                {
                    printf("Fail; Test Fail.\n)");
                    failed++;
                }
            }
        }
        else
        {
            if ((SIGNATURE_INVALID == status) && ('F' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
            {
                signVerify = SIGNATURE_INVALID;
                failed += (signVerify != SIGNATURE_INVALID);
                if(P_verbose)
                {
                    printf ("Verification check: ");
                    if (signVerify == SIGNATURE_INVALID)
                    {
                        printf("Fail; ");
                    }
                    else
                    {
                        printf("Pass; ");
                    }
                    printf("(expected: ");
                    if (('F' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
                    {
                        printf("Fail;) Test Pass.\n");
                    }
                    else
                    {
                        printf("Pass;) Test Fail.\n");
                        failed++;
                    }
                }
            }
        }

        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_allSha_1024_signatureVerify_test */

uint32_t pkcs_allSha_3072_signatureVerify_test( uint32_t P_verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */

    uint32_t failed = 0;
    uint32_t tv;
    uint32_t signVerify = 0U;

    printf (" - Launch RSA3072 PKCS#1v1.5 signature verification against known test vectors from (NIST CAVS 11.0) for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    for(tv = 0; tv < C_NB_TV_SIGN_VERIFY_ALL_SHA_3072; tv ++)
    {
        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1) );
        }

        PubKey_st.mExponentSize = test_vect_signVerify_allSha_3072[tv].pubexp_byte_size;
        PubKey_st.mModulusSize = test_vect_signVerify_allSha_3072[tv].mod_byte_size;
        PubKey_st.pmExponent = (uint8_t *)test_vect_signVerify_allSha_3072[tv].pubexp;
        PubKey_st.pmModulus = (uint8_t *)test_vect_signVerify_allSha_3072[tv].mod;

        status = RSA_Verify_PKCS1v15(&PubKey_st, test_vect_signVerify_allSha_3072[tv].msg, test_vect_signVerify_allSha_3072[tv].msg_size,
                                     test_vect_signVerify_allSha_3072[tv].hashType,
                                     test_vect_signVerify_allSha_3072[tv].sig, P_verbose);
        /* Translate CSE error code */
        if ((SIGNATURE_VALID == status) && ('P' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
        {
            signVerify = SIGNATURE_VALID;
            failed += (signVerify != SIGNATURE_VALID);
            if(P_verbose)
            {
                printf ("Verification check: ");
                if (signVerify == SIGNATURE_VALID)
                {
                    printf("Pass; ");
                }
                else
                {
                    printf("Fail; ");
                }
                printf("(expected: ");
                if (('P' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
                {
                    printf("Pass;) Test Pass.\n");
                }
                else
                {
                    printf("Fail; Test Fail.\n)");
                    failed++;
                }
            }
        }
        else
        {
            if ((SIGNATURE_INVALID == status) && ('F' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
            {
                signVerify = SIGNATURE_INVALID;
                failed += (signVerify != SIGNATURE_INVALID);
                if(P_verbose)
                {
                    printf ("Verification check: ");
                    if (signVerify == SIGNATURE_INVALID)
                    {
                        printf("Fail; ");
                    }
                    else
                    {
                        printf("Pass; ");
                    }
                    printf("(expected: ");
                    if (('F' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
                    {
                        printf("Fail;) Test Pass.\n");
                    }
                    else
                    {
                        printf("Pass;) Test Fail.\n");
                        failed++;
                    }
                }
            }
        }

        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_allSha_3072_signatureVerify_test */


uint32_t pkcs_sha1_2048_signature_test( uint32_t verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[2048 / 8]; /* Buffer that will contain the signature */
    int32_t check = 1;
    uint32_t failed = 0;
    uint32_t tv;

    /* TEST VECTOR FOR RSA-2048 */
    PubKey_st.mExponentSize = TV_sha1_2048.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_sha1_2048.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_sha1_2048.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;

    PrivKey_st.mExponentSize = TV_sha1_2048.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_sha1_2048.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_sha1_2048.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;

    printf (" - Launch RSA2048 PKCS#1v1.5 signature generation and verification for SHA1\n");
    for(tv = 0; tv < nb_tv_2048; tv++)
    {
        if(verbose)
        {
            printf("Testing RSA test vector #15.%d: ", (tv+1) );
        }
        status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_sha1_2048.array[tv].msg, TV_sha1_2048.array[tv].msg_size, E_SHA1, Signature, verbose);
        if (status == RSA_SUCCESS)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_sha1_2048.array[tv].sig, TV_sha1_2048.array[tv].sig_size);
            if(verbose)
            {
                printf("Signature check: %d, ", check==0);
            }
        }
        else
        {
            if(verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((status != RSA_SUCCESS) || ((status == RSA_SUCCESS) && (check != 0)));

        status = RSA_Verify_PKCS1v15(&PubKey_st, TV_sha1_2048.array[tv].msg, TV_sha1_2048.array[tv].msg_size, E_SHA1, TV_sha1_2048.array[tv].sig, verbose);
        if(verbose)
        {
            printf("Verification check: %d\n", status==SIGNATURE_VALID);
        }
        failed += (status != SIGNATURE_VALID);
        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
}

uint32_t pkcs_allSha_2048_signature_test( uint32_t verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[2048 / 8]; /* Buffer that will contain the signature */
    int32_t check = 1;
    uint32_t failed = 0;
    uint32_t tv;

    PubKey_st.mExponentSize = TV_allSsha_2048.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_allSsha_2048.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_allSsha_2048.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_allSsha_2048.key.mod;

    PrivKey_st.mExponentSize = TV_allSsha_2048.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_allSsha_2048.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_allSsha_2048.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_allSsha_2048.key.mod;

    printf (" - Launch RSA2048 PKCS#1v1.5 signature generation and verification with 10 test vectors from NIST CAVP for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    for(tv = 0; tv < C_NB_TV_ALL_SHA_2048; tv ++)
    {
        if(verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1) );
        }
        status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_allSsha_2048.array[tv].msg, TV_allSsha_2048.array[tv].msg_size,
                                   TV_allSsha_2048.array[tv].hashType, Signature, verbose);
        if (status == RSA_SUCCESS)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_allSsha_2048.array[tv].sig, TV_allSsha_2048.array[tv].sig_size);
            if(verbose)
            {
                printf("Signature check: %d, ", check==0);
            }
        }
        else
        {
            if(verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((status != RSA_SUCCESS) || ((status == RSA_SUCCESS) && (check != 0)));

        status = RSA_Verify_PKCS1v15(&PubKey_st, TV_allSsha_2048.array[tv].msg, TV_allSsha_2048.array[tv].msg_size,
                                     TV_allSsha_2048.array[tv].hashType,
                                     TV_allSsha_2048.array[tv].sig, verbose);
        if(verbose)
        {
            printf("Verification check: %d\n", status==SIGNATURE_VALID);
        }
        failed += (status != SIGNATURE_VALID);
        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_allSha_2048_signature_test */

uint32_t pkcs_allSha_3072_signature_test( uint32_t verbose )
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[3072 / 8]; /* Buffer that will contain the signature */
    int32_t check = 1;
    uint32_t failed = 0;
    uint32_t tv;

    PubKey_st.mExponentSize = TV_allSsha_3072.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_allSsha_3072.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_allSsha_3072.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_allSsha_3072.key.mod;

    PrivKey_st.mExponentSize = TV_allSsha_3072.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_allSsha_3072.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_allSsha_3072.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_allSsha_3072.key.mod;

    printf (" - Launch RSA3072 PKCS#1v1.5 signature generation and verification with 10 test vectors from NIST CAVP for SHA1, SHA224, SHA256, SHA384 and SHA512\n");
    for(tv = 0; tv < C_NB_TV_ALL_SHA_3072; tv ++)
    {
        if(verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1) );
        }
        status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_allSsha_3072.array[tv].msg, TV_allSsha_3072.array[tv].msg_size,
                                   TV_allSsha_3072.array[tv].hashType, Signature, verbose);
        if (status == RSA_SUCCESS)
        {
            /* Compare the Signature */
            check = memcmp(Signature, TV_allSsha_3072.array[tv].sig, TV_allSsha_3072.array[tv].sig_size);
            if(verbose)
            {
                printf("Signature check: %d, ", check==0);
            }
        }
        else
        {
            if(verbose)
            {
                printf("Execution Failed, ");
            }
        }
        failed += ((status != RSA_SUCCESS) || ((status == RSA_SUCCESS) && (check != 0)));

        status = RSA_Verify_PKCS1v15(&PubKey_st, TV_allSsha_3072.array[tv].msg, TV_allSsha_3072.array[tv].msg_size,
                                     TV_allSsha_3072.array[tv].hashType,
                                     TV_allSsha_3072.array[tv].sig, verbose);
        if(verbose)
        {
            printf("Verification check: %d\n", status == SIGNATURE_VALID);
        }
        failed += (status != SIGNATURE_VALID);
        printf(".");
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);
} /* End of pkcs_allSha_3072_signature_test */


uint32_t pkcs_sha1_1024_signatureWithProtectedKeys_test (uint32_t P_verbose)
{
    uint32_t status;

    uint8_t Signature[1024U / 8U]; /* Buffer that will contain the signature */
    uint32_t tv;
    uint32_t i = 0U;

    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 32)] = {0x00000000};    /* To support RSA 1024 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t pass = 1;
    uint32_t global_pass = 1;

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

    printf(" - Test RSA1024 Signature generation and verification with NVM key!\n");

    for(tv = 0; tv < nb_tv_1024; tv ++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1));
            printf(" - load key for signature generation\n");
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;
        key.PubKey.mModulusSize = 0;
        key.PubKey.pmExponent = (uint8_t *)TV_sha1_1024.key.pubexp;
        key.PubKey.mExponentSize = 0;
        key.PrivKey.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;
        key.PrivKey.mModulusSize = TV_sha1_1024.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_sha1_1024.key.privexp;
        key.PrivKey.mExponentSize = TV_sha1_1024.key.privexp_byte_size;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.sign = 1U;    /* Key is allowed for signature generation */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              M1, M2, M3,
                                              M4, M5, P_verbose);
        if(pass)
        {
            /* Sign with RSA */
            pass = signature_generation_test(keyIndex, (vuint8_t*)TV_sha1_1024.array[tv].msg, TV_sha1_1024.array[tv].msg_size,
                                             (vuint8_t*) Signature, E_SHA1,
                                             TV_sha1_1024.array[tv].sig, TV_sha1_1024.array[tv].sig_size, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

        if(P_verbose)
        {
            printf(" - load key for signature verification\n");
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature verification: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;
        key.PubKey.mModulusSize = TV_sha1_1024.key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)TV_sha1_1024.key.pubexp;
        key.PubKey.mExponentSize = TV_sha1_1024.key.pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;
        key.PrivKey.mModulusSize = TV_sha1_1024.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_sha1_1024.key.privexp;
        key.PrivKey.mExponentSize = 0U;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              M1, M2, M3,
                                              M4, M5, P_verbose);
        if(pass)
        {
            pass = signature_verification_test(keyIndex, (vuint8_t*)TV_sha1_1024.array[tv].msg, TV_sha1_1024.array[tv].msg_size,
                                               (vuint8_t*) TV_sha1_1024.array[tv].sig, E_SHA1, &signVerify, SIGNATURE_VALID, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(global_pass);
} /* End of pkcs_sha1_1024_signatureWithProtectedKeys_test */

uint32_t pkcs_sha1_2048_signatureWithProtectedKeys_test(uint32_t P_verbose)
{
    uint32_t status;

    uint8_t Signature[2048U / 8U];    /* Buffer that will contain the signature */
    uint32_t tv;
    uint32_t i = 0U;
    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 64)] = {0x00000000};    /* To support RSA 2048 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;
    uint32_t pass = 1;
    uint32_t global_pass = 1;

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

    /* TEST VECTOR FOR RSA-2048 */
    printf(" - Test RSA2048 Signature generation and verification with NVM key!\n");

    for(tv = 0; tv < nb_tv_2048; tv++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #15.%d: ", (tv+1) );
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;
        key.PubKey.mModulusSize = 0;
        key.PubKey.pmExponent = (uint8_t *)TV_sha1_2048.key.pubexp;
        key.PubKey.mExponentSize = 0;
        key.PrivKey.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;
        key.PrivKey.mModulusSize = TV_sha1_2048.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_sha1_2048.key.privexp;
        key.PrivKey.mExponentSize = TV_sha1_2048.key.privexp_byte_size;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.sign = 1U;    /* Key is allowed for signature generation */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Sign with RSA */
            pass = signature_generation_test(keyIndex, (vuint8_t*)TV_sha1_2048.array[tv].msg, TV_sha1_2048.array[tv].msg_size,
                                             (vuint8_t*) Signature, E_SHA1,
                                             TV_sha1_2048.array[tv].sig, TV_sha1_2048.array[tv].sig_size, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

        /* Initializes Public and Private Key structures with test vectors */
        /* Signature verification: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;
        key.PubKey.mModulusSize = TV_sha1_2048.key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)TV_sha1_2048.key.pubexp;
        key.PubKey.mExponentSize = TV_sha1_1024.key.pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;
        key.PrivKey.mModulusSize = TV_sha1_2048.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_sha1_2048.key.privexp;
        key.PrivKey.mExponentSize = 0U;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              M1, M2, M3,
                                              M4, M5, P_verbose);
        if(pass)
        {
            pass = signature_verification_test(keyIndex, (vuint8_t*)TV_sha1_2048.array[tv].msg, TV_sha1_2048.array[tv].msg_size,
                                               (vuint8_t*) TV_sha1_2048.array[tv].sig, E_SHA1, &signVerify, SIGNATURE_VALID, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(global_pass);
}

uint32_t pkcs_allSha_1024_signatureWithProtectedKeys_test (uint32_t P_verbose)
{
    uint32_t status;

    uint8_t Signature[1024U / 8U]; /* Buffer that will contain the signature */
    uint32_t tv;
    uint32_t i = 0U;

    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 32)] = {0x00000000};    /* To support RSA 1024 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t pass;
    uint32_t global_pass = 1;

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

    printf(" - Test RSA1024 Signature generation and verification with NVM key for SHA1, SHA224, SHA256, SHA384 and SHA512!\n");

    for(tv = 0; tv < C_NB_TV_ALL_SHA_1024; tv ++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1));
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)TV_allSsha_1024.key.mod;
        key.PubKey.mModulusSize = 0;
        key.PubKey.pmExponent = (uint8_t *)TV_allSsha_1024.key.pubexp;
        key.PubKey.mExponentSize = 0;
        key.PrivKey.pmModulus = (uint8_t *)TV_allSsha_1024.key.mod;
        key.PrivKey.mModulusSize = TV_allSsha_1024.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_allSsha_1024.key.privexp;
        key.PrivKey.mExponentSize = TV_allSsha_1024.key.privexp_byte_size;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.sign = 1U;    /* Key is allowed for signature generation */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              M1, M2, M3,
                                              M4, M5, P_verbose);
        if(pass)
        {
            /* Sign with RSA */
            pass = signature_generation_test(keyIndex, (vuint8_t*)TV_allSsha_1024.array[tv].msg, TV_allSsha_1024.array[tv].msg_size,
                                             (vuint8_t*) Signature, E_SHA1,
                                             TV_allSsha_1024.array[tv].sig, TV_allSsha_1024.array[tv].sig_size, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

        /* Initializes Public and Private Key structures with test vectors */
        /* Signature verification: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)TV_allSsha_1024.key.mod;
        key.PubKey.mModulusSize = TV_allSsha_1024.key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)TV_allSsha_1024.key.pubexp;
        key.PubKey.mExponentSize = TV_allSsha_1024.key.pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)TV_allSsha_1024.key.mod;
        key.PrivKey.mModulusSize = TV_allSsha_1024.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_allSsha_1024.key.privexp;
        key.PrivKey.mExponentSize = 0U;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                              keyIndex, authKeyId ,
                                              M1, M2, M3,
                                              M4, M5, P_verbose);
        if(pass)
        {
            /* Verify signature with RSA */
            pass = signature_verification_test( keyIndex, (vuint8_t*)TV_allSsha_1024.array[tv].msg, TV_allSsha_1024.array[tv].msg_size,
                                                (vuint8_t*) TV_allSsha_1024.array[tv].sig,
                                                TV_allSsha_1024.array[tv].hashType, &signVerify, SIGNATURE_VALID, P_verbose );
        }

        display_pass_fail(pass);
        global_pass &= pass;
    }
    if(P_verbose)
    {
        printf("\n");
    }
    /* return 1 if successful (no test failed) */
    return(global_pass);
} /* End of pkcs_allSha_1024_signatureWithProtectedKeys_test */

uint32_t pkcs_allSha_2048_signatureWithProtectedKeys_test(uint32_t P_verbose)
{
    uint32_t status;

    uint8_t Signature[2048U / 8U];    /* Buffer that will contain the signature */
    uint32_t tv;
    uint32_t i = 0U;
    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 64)] = {0x00000000};    /* To support RSA 2048 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;
    uint32_t pass;
    uint32_t global_pass = 1;

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

    /* TEST VECTOR FOR RSA-2048 */
    printf(" - Test RSA2048 Signature generation and verification with NVM key for SHA1, SHA224, SHA256, SHA384 and SHA512!\n");

    for(tv = 0; tv < C_NB_TV_ALL_SHA_2048; tv++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #15.%d: ", (tv+1) );
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)TV_allSsha_2048.key.mod;
        key.PubKey.mModulusSize = 0;
        key.PubKey.pmExponent = (uint8_t *)TV_allSsha_2048.key.pubexp;
        key.PubKey.mExponentSize = 0;
        key.PrivKey.pmModulus = (uint8_t *)TV_allSsha_2048.key.mod;
        key.PrivKey.mModulusSize = TV_allSsha_2048.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_allSsha_2048.key.privexp;
        key.PrivKey.mExponentSize = TV_allSsha_2048.key.privexp_byte_size;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.sign = 1U;    /* Key is allowed for signature generation */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Sign with RSA */
            pass = signature_generation_test(keyIndex, (vuint8_t*)TV_allSsha_2048.array[tv].msg, TV_allSsha_2048.array[tv].msg_size,
                                             (vuint8_t*) Signature, TV_allSsha_2048.array[tv].hashType,
                                             TV_allSsha_2048.array[tv].sig, TV_allSsha_2048.array[tv].sig_size, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

        /* Initializes Public and Private Key structures with test vectors */
        /* Signature verification: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)TV_allSsha_2048.key.mod;
        key.PubKey.mModulusSize = TV_allSsha_2048.key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)TV_allSsha_2048.key.pubexp;
        key.PubKey.mExponentSize = TV_allSsha_2048.key.pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)TV_allSsha_2048.key.mod;
        key.PrivKey.mModulusSize = TV_allSsha_2048.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_allSsha_2048.key.privexp;
        key.PrivKey.mExponentSize = 0U;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Verify signature with RSA */
            pass = signature_verification_test( keyIndex, (vuint8_t*)TV_allSsha_2048.array[tv].msg, TV_allSsha_2048.array[tv].msg_size,
                                                (vuint8_t*) TV_allSsha_2048.array[tv].sig,
                                                TV_allSsha_2048.array[tv].hashType, &signVerify, SIGNATURE_VALID, P_verbose );

        }

        display_pass_fail(pass);
        global_pass &= pass;
    }
    if(P_verbose)
    {
        printf("\n");
    }
    /* return 1 if successful (no test failed) */
    return(global_pass);
} /* End of pkcs_allSha_2048_signatureWithProtectedKeys_test */


uint32_t pkcs_allSha_3072_signatureWithProtectedKeys_test(uint32_t P_verbose)
{
    uint32_t status;

    uint8_t Signature[3072U / 8U];    /* Buffer that will contain the signature */
    uint32_t tv;
    uint32_t i = 0U;
    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 96)] = {0x00000000};    /* To support RSA 3072 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;
    uint32_t pass;
    uint32_t global_pass = 1;

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

    /* TEST VECTOR FOR RSA-3072 */
    printf(" - Test RSA3072 Signature generation and verification with NVM key for SHA1, SHA224, SHA256, SHA384 and SHA512!\n");

    for(tv = 0; tv < C_NB_TV_ALL_SHA_3072; tv++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #15.%d: ", (tv+1) );
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)TV_allSsha_3072.key.mod;
        key.PubKey.mModulusSize = 0;
        key.PubKey.pmExponent = (uint8_t *)TV_allSsha_3072.key.pubexp;
        key.PubKey.mExponentSize = 0;
        key.PrivKey.pmModulus = (uint8_t *)TV_allSsha_3072.key.mod;
        key.PrivKey.mModulusSize = TV_allSsha_3072.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_allSsha_3072.key.privexp;
        key.PrivKey.mExponentSize = TV_allSsha_3072.key.privexp_byte_size;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.sign = 1U;    /* Key is allowed for signature generation */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Sign with RSA */
            pass = signature_generation_test(keyIndex, (vuint8_t*)TV_allSsha_3072.array[tv].msg, TV_allSsha_3072.array[tv].msg_size,
                                             (vuint8_t*) Signature, TV_allSsha_3072.array[tv].hashType,
                                             TV_allSsha_3072.array[tv].sig, TV_allSsha_3072.array[tv].sig_size, P_verbose);
        }
        display_pass_fail(pass);
        global_pass &= pass;

        /* Initializes Public and Private Key structures with test vectors */
        /* Signature verification: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)TV_allSsha_3072.key.mod;
        key.PubKey.mModulusSize = TV_allSsha_3072.key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)TV_allSsha_3072.key.pubexp;
        key.PubKey.mExponentSize = TV_allSsha_3072.key.pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)TV_allSsha_3072.key.mod;
        key.PrivKey.mModulusSize = TV_allSsha_3072.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_allSsha_3072.key.privexp;
        key.PrivKey.mExponentSize = 0U;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;         /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Verify signature with RSA */
            pass = signature_verification_test( keyIndex, (vuint8_t*)TV_allSsha_3072.array[tv].msg, TV_allSsha_3072.array[tv].msg_size,
                                                (vuint8_t*) TV_allSsha_3072.array[tv].sig,
                                                TV_allSsha_3072.array[tv].hashType, &signVerify, SIGNATURE_VALID, P_verbose );
        }

        display_pass_fail(pass);
        global_pass &= pass;
    }
    if(P_verbose)
    {
        printf("\n");
    }
    /* return 1 if successful (no test failed) */
    return(global_pass);
} /* End of pkcs_allSha_3072_signatureWithProtectedKeys_test */


uint32_t pkcs_signature_test( uint32_t verbose )
{
    uint32_t pass = 1;

    pass = pkcs_sha1_1024_signature_test(verbose);
    pass = pass & pkcs_sha1_2048_signature_test(verbose);

    return(pass);
}

uint32_t pkcs_signatureAllSha_test( uint32_t verbose )
{
    uint32_t pass = 1;

    pass = pkcs_allSha_1024_signature_test(verbose);
    pass = pass & pkcs_allSha_2048_signature_test(verbose);
    pass = pass & pkcs_allSha_3072_signature_test(verbose);

    return(pass);
} /* End of pkcs_signatureAllSha_test */

uint32_t pkcs_allSha_signatureVerifyRamKeys_test (uint32_t P_verbose)
{
    uint32_t pass = 1;

    pass = pkcs_allSha_1024_signatureVerify_test(P_verbose);
    pass = pass & pkcs_allSha_2048_signatureVerify_test(P_verbose);
    pass = pass & pkcs_allSha_3072_signatureVerify_test(P_verbose);

    return(pass);

} /* End of pkcs_allSha_signatureVerifyRamKeys_test */

uint32_t pkcs_allSha_1024_signatureVerifyWithProtectedKeys_test (uint32_t P_verbose)
{
    uint32_t status;

    uint32_t failed = 0;
    uint32_t tv;
    uint32_t i = 0U;

    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 32)] = {0x00000000};    /* To support RSA 1024 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t ret;
    uint32_t pass;

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

    printf(" - Test RSA1024 Signature verification against known test vectors (NIST CAVS 11.0) with NVM key for SHA1, SHA224, SHA256, SHA384 and SHA512!\n");

    for(tv = 0; tv < C_NB_TV_SIGN_VERIFY_ALL_SHA_1024; tv ++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1));
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)test_vect_signVerify_allSha_1024[tv].mod;
        key.PubKey.mModulusSize = test_vect_signVerify_allSha_1024[tv].mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)test_vect_signVerify_allSha_1024[tv].pubexp;
        key.PubKey.mExponentSize = test_vect_signVerify_allSha_1024[tv].pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)test_vect_signVerify_allSha_1024[tv].mod;
        key.PrivKey.mModulusSize = test_vect_signVerify_allSha_1024[tv].mod_byte_size;
        key.PrivKey.pmExponent = NULL;
        key.PrivKey.mExponentSize = 0;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;           /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Verify signature with RSA */
            ret = CSE_RSA_PKCS1_15_verify(keyIndex, (vuint8_t*)test_vect_signVerify_allSha_1024[tv].msg, test_vect_signVerify_allSha_1024[tv].msg_size,
                                          (vuint8_t*) test_vect_signVerify_allSha_1024[tv].sig,
                                          test_vect_signVerify_allSha_1024[tv].hashType, &signVerify );

            /* Translate CSE error code */
            if( CSE_NO_ERR == ret )
            {
                if ((0U == signVerify) && ('P' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
                {
                    signVerify = SIGNATURE_VALID;
                    failed += (signVerify != SIGNATURE_VALID);
                    if(P_verbose)
                    {
                        printf ("Verification check: ");
                        if (signVerify == SIGNATURE_VALID)
                        {
                            printf("Pass; ");
                        }
                        else
                        {
                            printf("Fail; ");
                        }
                        printf("(expected: ");
                        if (('P' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
                        {
                            printf("Pass;) Test Pass.\n");
                        }
                        else
                        {
                            printf("Fail; Test Fail.\n)");
                            failed++;
                        }
                    }
                }
                else
                {
                    if ((0U == signVerify) && ('F' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
                    {
                        signVerify = SIGNATURE_INVALID;
                        failed += (signVerify != SIGNATURE_INVALID);
                        if(P_verbose)
                        {
                            printf ("Verification check: ");
                            if (signVerify == SIGNATURE_INVALID)
                            {
                                printf("Fail; ");
                            }
                            else
                            {
                                printf("Pass; ");
                            }
                            printf("(expected: ");
                            if (('F' == *test_vect_signVerify_allSha_1024[tv].expectedVerifCheck))
                            {
                                printf("Fail;) Test Pass.\n");
                            }
                            else
                            {
                                printf("Pass;) Test Fail.\n");
                                failed++;
                            }
                        }
                    }
                }
            }
            printf(".");
        }
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0U);
} /* End of pkcs_allSha_1024_signatureVerifyWithProtectedKeys_test */

uint32_t pkcs_allSha_2048_signatureVerifyWithProtectedKeys_test (uint32_t P_verbose)
{
    uint32_t status;

    uint32_t failed = 0;
    uint32_t tv;
    uint32_t i = 0U;

    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 64)] = {0x00000000};    /* To support RSA 2048 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t ret;
    uint32_t pass;

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

    printf(" - Test RSA2048 Signature verification against known test vectors (NIST CAVS 11.0) with NVM key for SHA1, SHA224, SHA256, SHA384 and SHA512!\n");

    for(tv = 0; tv < C_NB_TV_SIGN_VERIFY_ALL_SHA_2048; tv ++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1));
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)test_vect_signVerify_allSha_2048[tv].mod;
        key.PubKey.mModulusSize = test_vect_signVerify_allSha_2048[tv].mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)test_vect_signVerify_allSha_2048[tv].pubexp;
        key.PubKey.mExponentSize = test_vect_signVerify_allSha_2048[tv].pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)test_vect_signVerify_allSha_2048[tv].mod;
        key.PrivKey.mModulusSize = test_vect_signVerify_allSha_2048[tv].mod_byte_size;
        key.PrivKey.pmExponent = NULL;
        key.PrivKey.mExponentSize = 0;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;           /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;
        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Verify signature with RSA */
            ret = CSE_RSA_PKCS1_15_verify(keyIndex, (vuint8_t*)test_vect_signVerify_allSha_2048[tv].msg, test_vect_signVerify_allSha_2048[tv].msg_size,
                                          (vuint8_t*) test_vect_signVerify_allSha_2048[tv].sig,
                                          test_vect_signVerify_allSha_2048[tv].hashType, &signVerify );

            /* Translate CSE error code */
            if( CSE_NO_ERR == ret )
            {
                if ((0U == signVerify) && ('P' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
                {
                    signVerify = SIGNATURE_VALID;
                    failed += (signVerify != SIGNATURE_VALID);
                    if(P_verbose)
                    {
                        printf ("Verification check: ");
                        if (signVerify == SIGNATURE_VALID)
                        {
                            printf("Pass; ");
                        }
                        else
                        {
                            printf("Fail; ");
                        }
                        printf("(expected: ");
                        if (('P' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
                        {
                            printf("Pass;) Test Pass.\n");
                        }
                        else
                        {
                            printf("Fail; Test Fail.\n)");
                            failed++;
                        }
                    }
                }
                else
                {
                    if ((0U == signVerify) && ('F' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
                    {
                        signVerify = SIGNATURE_INVALID;
                        failed += (signVerify != SIGNATURE_INVALID);
                        if(P_verbose)
                        {
                            printf ("Verification check: ");
                            if (signVerify == SIGNATURE_INVALID)
                            {
                                printf("Fail; ");
                            }
                            else
                            {
                                printf("Pass; ");
                            }
                            printf("(expected: ");
                            if (('F' == *test_vect_signVerify_allSha_2048[tv].expectedVerifCheck))
                            {
                                printf("Fail;) Test Pass.\n");
                            }
                            else
                            {
                                printf("Pass;) Test Fail.\n");
                                failed++;
                            }
                        }
                    }
                }
            }
            printf(".");
        }
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0U);
} /* End of pkcs_allSha_2048_signatureVerifyWithProtectedKeys_test */

uint32_t pkcs_allSha_3072_signatureVerifyWithProtectedKeys_test (uint32_t P_verbose)
{
    uint32_t status;

    uint32_t failed = 0;
    uint32_t tv;
    uint32_t i = 0U;

    uint32_t signVerify = 0U;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 96)] = {0x00000000};    /* To support RSA 3072 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1U;

    uint32_t ret;
    uint32_t pass;

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

    printf(" - Test RSA3072 Signature verification against known test vectors (NIST CAVS 11.0) with NVM key for SHA1, SHA224, SHA256, SHA384 and SHA512!\n");

    for(tv = 0; tv < C_NB_TV_SIGN_VERIFY_ALL_SHA_3072; tv ++)
    {
        uint32_t keyIndex = 0U;

        if(P_verbose)
        {
            printf("Testing RSA test vector #1.%d: ", (tv + 1));
        }
        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Private Key */
        key.PubKey.pmModulus = (uint8_t *)test_vect_signVerify_allSha_3072[tv].mod;
        key.PubKey.mModulusSize = test_vect_signVerify_allSha_3072[tv].mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)test_vect_signVerify_allSha_3072[tv].pubexp;
        key.PubKey.mExponentSize = test_vect_signVerify_allSha_3072[tv].pubexp_byte_size;
        key.PrivKey.pmModulus = (uint8_t *)test_vect_signVerify_allSha_3072[tv].mod;
        key.PrivKey.mModulusSize = test_vect_signVerify_allSha_3072[tv].mod_byte_size;
        key.PrivKey.pmExponent = NULL;
        key.PrivKey.mExponentSize = 0;
        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;           /* No restriction on Keys */
        key.RSA_flags.B.verify = 1U;    /* Key is allowed for signature verification */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (tv % 4) + 1;

        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if(pass)
        {
            /* Verify signature with RSA */
            ret = CSE_RSA_PKCS1_15_verify(keyIndex, (vuint8_t*)test_vect_signVerify_allSha_3072[tv].msg, test_vect_signVerify_allSha_3072[tv].msg_size,
                                          (vuint8_t*) test_vect_signVerify_allSha_3072[tv].sig,
                                          test_vect_signVerify_allSha_3072[tv].hashType, &signVerify );

            /* Translate CSE error code */
            if( CSE_NO_ERR == ret )
            {
                if ((0U == signVerify) && ('P' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
                {
                    signVerify = SIGNATURE_VALID;
                    failed += (signVerify != SIGNATURE_VALID);
                    if(P_verbose)
                    {
                        printf ("Verification check: ");
                        if (signVerify == SIGNATURE_VALID)
                        {
                            printf("Pass; ");
                        }
                        else
                        {
                            printf("Fail; ");
                        }
                        printf("(expected: ");
                        if (('P' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
                        {
                            printf("Pass;) Test Pass.\n");
                        }
                        else
                        {
                            printf("Fail; Test Fail.\n)");
                            failed++;
                        }
                    }
                }
                else
                {
                    if ((0U == signVerify) && ('F' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
                    {
                        signVerify = SIGNATURE_INVALID;
                        failed += (signVerify != SIGNATURE_INVALID);
                        if(P_verbose)
                        {
                            printf ("Verification check: ");
                            if (signVerify == SIGNATURE_INVALID)
                            {
                                printf("Fail; ");
                            }
                            else
                            {
                                printf("Pass; ");
                            }
                            printf("(expected: ");
                            if (('F' == *test_vect_signVerify_allSha_3072[tv].expectedVerifCheck))
                            {
                                printf("Fail;) Test Pass.\n");
                            }
                            else
                            {
                                printf("Pass;) Test Fail.\n");
                                failed++;
                            }
                        }
                    }
                }
            }
            printf(".");
        }
    }
    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0U);
} /* End of pkcs_allSha_3072_signatureVerifyWithProtectedKeys_test */


uint32_t pkcs_signature_notSupportedSha_test (uint32_t P_verbose)
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[1024 / 8]; /* Buffer that will contain the signature */
    uint32_t failed = 0;

    PubKey_st.mExponentSize = TV_sha1_1024.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_sha1_1024.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_sha1_1024.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;

    PrivKey_st.mExponentSize = TV_sha1_1024.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_sha1_1024.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_sha1_1024.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;

    printf (" - Launch RSA1024 PKCS#1v1.5 signature generation and verification for non supported Hash algorithm\n");
    printf ("   Must return General Error!\n");

    status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_sha1_1024.array[0].msg, TV_sha1_1024.array[0].msg_size,
                               E_MD5, Signature, P_verbose);
    failed += (status != CSE_GENERAL_ERR);

    status = RSA_Verify_PKCS1v15(&PubKey_st, TV_sha1_1024.array[0].msg, TV_sha1_1024.array[0].msg_size,
                                 E_MD5,
                                 TV_sha1_1024.array[0].sig, P_verbose);
    failed += (status != CSE_GENERAL_ERR);

    printf("\n");
    /* return 1 if successful (no test failed) */
    return(failed == 0);

} /* End of pkcs_signature_notSupportedSha_test */

uint32_t pkcs_signature_notexisitingShaIndex_test (uint32_t P_verbose)
{
    int32_t status;
    struct RSApubKey_stt PubKey_st; /* Structure that will contain the public key */
    struct RSAprivKey_stt PrivKey_st; /* Structure that will contain the private key*/
    uint8_t Signature[1024 / 8]; /* Buffer that will contain the signature */

    uint32_t failed = 0;

    PubKey_st.mExponentSize = TV_sha1_1024.key.pubexp_byte_size;
    PubKey_st.mModulusSize = TV_sha1_1024.key.mod_byte_size;
    PubKey_st.pmExponent = (uint8_t *)TV_sha1_1024.key.pubexp;
    PubKey_st.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;

    PrivKey_st.mExponentSize = TV_sha1_1024.key.privexp_byte_size;
    PrivKey_st.mModulusSize = TV_sha1_1024.key.mod_byte_size;
    PrivKey_st.pmExponent = (uint8_t *)TV_sha1_1024.key.privexp;
    PrivKey_st.pmModulus = (uint8_t *)TV_sha1_1024.key.mod;

    printf (" - Launch RSA1024 PKCS#1v1.5 signature generation and verification with Hash algorithm not in list\n");
    printf ("   Must return General Error!\n");

    status = RSA_Sign_PKCS1v15(&PrivKey_st, TV_sha1_1024.array[0].msg, TV_sha1_1024.array[0].msg_size,
                               E_SHA512 + 1, Signature, P_verbose);
    failed += (status != CSE_GENERAL_ERR);

    status = RSA_Verify_PKCS1v15(&PubKey_st, TV_sha1_1024.array[0].msg, TV_sha1_1024.array[0].msg_size,
                                 E_SHA512 + 1,
                                 TV_sha1_1024.array[0].sig, P_verbose);
    failed += (status != CSE_GENERAL_ERR);

    /* return 1 if successful (no test failed) */
    return(failed == 0);

} /* End of pkcs_signature_notSupportedSha_test */

uint32_t pkcs_signature_notSupportedHash_test (uint32_t P_verbose)
{
    uint32_t pass = 1;

    pass = pkcs_signature_notSupportedSha_test(P_verbose);
    pass = pass & pkcs_signature_notexisitingShaIndex_test(P_verbose);

    return(pass);

} /* End of pkcs_signature_notSupportedHash_test */

uint32_t pkcs_encryption_test( uint32_t P_verbose )
{
    uint32_t failed = 0;

    struct RSAprivKey_stt privKey_st;
    struct RSApubKey_stt pubKey_st;

    uint32_t retval = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t inSize;
    uint32_t outSize;
    uint32_t resSize;
    uint8_t plaintext[384], cipher[384], result[384] = {0,};
    uint32_t* plaintext_w = (uint32_t*)plaintext;

    uint32_t ret;

    if (P_verbose)
    {
        printf (" - Launch RSA1024 PKCS#1v1.5 encryption / decryption\n");
    }
    if(P_verbose)
    {
        printf("\n-*- RSA with PKCS#1v1.5 1024 bit -*-\n\n");
        display_buf("Modulus:", (uint8_t *) RSA1024_encrypt_key.mod,RSA1024_encrypt_key.mod_byte_size);
        display_buf("Exponent:", (uint8_t *)RSA1024_encrypt_key.pubexp, RSA1024_encrypt_key.pubexp_byte_size);
        display_buf("privexp:", (uint8_t *)RSA1024_encrypt_key.privexp, RSA1024_encrypt_key.privexp_byte_size);
    }

    pubKey_st.mExponentSize = RSA1024_encrypt_key.pubexp_byte_size;
    pubKey_st.mModulusSize = RSA1024_encrypt_key.mod_byte_size;
    pubKey_st.pmExponent = (uint8_t *)RSA1024_encrypt_key.pubexp;
    pubKey_st.pmModulus = (uint8_t *)RSA1024_encrypt_key.mod;

    privKey_st.mExponentSize = RSA1024_encrypt_key.privexp_byte_size;
    privKey_st.mModulusSize = RSA1024_encrypt_key.mod_byte_size;
    privKey_st.pmExponent = (uint8_t *)RSA1024_encrypt_key.privexp;
    privKey_st.pmModulus = (uint8_t *)RSA1024_encrypt_key.mod;

    for (i = 0; i < 5; i++ )
    {
        for(j = 0; j < 16; j++)
        {
            ret = CSE_TRNG_Getrand(&plaintext[16 * j]);
            if(ret != CSE_NO_ERR )
            {
                failed ++;
            }
        }

        /* floating point crashes, replace by integer computation
         * Because of encoding scheme, Max Input size is modulus size - 11
         * */
        inSize = (plaintext_w[0]) % (pubKey_st.mModulusSize - 11);
        //inSize = (int32_t) (((double) (((uint32_t *)plaintext)[0])) / 0xFFFFFFFF * (pubKey_st.mModulusSize - 11) + 1);

        if(P_verbose)
        {
            printf("\nTesting RSA PKCS#1v1.5 Encryption with random message of size %d:\n",inSize);
        }
        if ((retval = RSA_Encrypt_PKCS1v15(&pubKey_st, plaintext, inSize, cipher, (uint32_t*)&outSize, P_verbose)) != RSA_SUCCESS)
        {
            if(P_verbose)
            {
                printf("Error in RSA Encryption!\n");
            }
        }
        if(P_verbose)
        {
            display_buf("Plaintext:    ", plaintext, inSize);
            display_buf("Ciphertext:   ", cipher, pubKey_st.mModulusSize);
        }
        if ((retval = RSA_Decrypt_PKCS1v15(&privKey_st, cipher, pubKey_st.mModulusSize, result, &resSize, P_verbose)) != RSA_SUCCESS)
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
            if (memcmp(plaintext,result,inSize) != 0 || (inSize != resSize))
            {
                if(P_verbose)
                {
                    printf("De-Ciphered text differs from cleartext\n");
                }
                failed += 1;
            }
        }
        printf(".");
    }

    /* Test with RSA2048 */
    if(P_verbose)
    {
        printf("\n-*- RSA with PKCS#1v1.5 2048 bit -*-\n\n");
        display_buf("Modulus:", (uint8_t *) TV_sha1_2048.key.mod,TV_sha1_2048.key.mod_byte_size);
        display_buf("Exponent:", (uint8_t *)TV_sha1_2048.key.pubexp, TV_sha1_2048.key.pubexp_byte_size);
        display_buf("privexp:", (uint8_t *)TV_sha1_2048.key.privexp, TV_sha1_2048.key.privexp_byte_size);
    }

    pubKey_st.mExponentSize = TV_sha1_2048.key.pubexp_byte_size;
    pubKey_st.mModulusSize = TV_sha1_2048.key.mod_byte_size;
    pubKey_st.pmExponent = (uint8_t *)TV_sha1_2048.key.pubexp;
    pubKey_st.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;

    privKey_st.mExponentSize = TV_sha1_2048.key.privexp_byte_size;
    privKey_st.mModulusSize = TV_sha1_2048.key.mod_byte_size;
    privKey_st.pmExponent = (uint8_t *)TV_sha1_2048.key.privexp;
    privKey_st.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;

    for (i = 0; i < 5; i++ )
    {
        for(j = 0; j < 16; j++)
        {
            ret = CSE_TRNG_Getrand(&plaintext[16 * j]);
            if(ret != CSE_NO_ERR )
            {
                failed ++;
            }
        }

        /* floating point crashes, replace by integer computation
         * Because of encoding scheme, Max Input size is modulus size - 11
         * */
        inSize = (plaintext_w[0]) % (pubKey_st.mModulusSize - 11);
        //inSize = (int32_t) (((double) (((uint32_t *)plaintext)[0])) / 0xFFFFFFFF * (pubKey_st.mModulusSize - 11) + 1);

        if(P_verbose)
        {
            printf("\nTesting RSA PKCS#1v1.5 Encryption with random message of size %d:\n",inSize);
        }
        if ((retval = RSA_Encrypt_PKCS1v15(&pubKey_st, plaintext, inSize, cipher, (uint32_t*)&outSize, P_verbose)) != RSA_SUCCESS)
        {
            printf("Error in RSA Encryption!\n");
        }

        if(P_verbose)
        {
            display_buf("Plaintext:    ", plaintext, inSize);
            display_buf("Ciphertext:   ", cipher, pubKey_st.mModulusSize);
        }

        /* Decryption */
        if ((retval = RSA_Decrypt_PKCS1v15(&privKey_st, cipher, pubKey_st.mModulusSize, result, &resSize, P_verbose)) != RSA_SUCCESS)
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
            if (memcmp(plaintext, result, inSize) != 0 || (inSize != resSize))
            {
                if(P_verbose)
                {
                    printf("De-Ciphered text differs from cleartext\n");
                }
                failed += 1;
            }
        }
        printf(".");
    }

    /* Test with RSA3072 */
    if(P_verbose)
    {
        printf("\n-*- RSA with PKCS#1v1.5 3072 bit -*-\n\n");
        display_buf("Modulus:", (uint8_t *) RSA3072_encrypt_key.mod,RSA3072_encrypt_key.mod_byte_size);
        display_buf("Exponent:", (uint8_t *)RSA3072_encrypt_key.pubexp, RSA3072_encrypt_key.pubexp_byte_size);
        display_buf("privexp:", (uint8_t *)RSA3072_encrypt_key.privexp, RSA3072_encrypt_key.privexp_byte_size);
    }

    pubKey_st.mExponentSize = RSA3072_encrypt_key.pubexp_byte_size;
    pubKey_st.mModulusSize = RSA3072_encrypt_key.mod_byte_size;
    pubKey_st.pmExponent = (uint8_t *)RSA3072_encrypt_key.pubexp;
    pubKey_st.pmModulus = (uint8_t *)RSA3072_encrypt_key.mod;

    privKey_st.mExponentSize = RSA3072_encrypt_key.privexp_byte_size;
    privKey_st.mModulusSize = RSA3072_encrypt_key.mod_byte_size;
    privKey_st.pmExponent = (uint8_t *)RSA3072_encrypt_key.privexp;
    privKey_st.pmModulus = (uint8_t *)RSA3072_encrypt_key.mod;

    for (i = 0; i < 5; i++ )
    {
        for(j = 0; j < 16; j++)
        {
            ret = CSE_TRNG_Getrand(&plaintext[16 * j]);
            if(ret != CSE_NO_ERR )
            {
                failed ++;
            }
        }

        /* floating point crashes, replace by integer computation
         * Because of encoding scheme, Max Input size is modulus size - 11
         * */
        inSize = (plaintext_w[0]) % (pubKey_st.mModulusSize - 11);
        //inSize = (int32_t) (((double) (((uint32_t *)plaintext)[0])) / 0xFFFFFFFF * (pubKey_st.mModulusSize - 11) + 1);

        if(P_verbose)
        {
            printf("\nTesting RSA PKCS#1v1.5 Encryption with random message of size %d:\n",inSize);
        }
        if ((retval = RSA_Encrypt_PKCS1v15(&pubKey_st, plaintext, inSize, cipher, (uint32_t*)&outSize, P_verbose)) != RSA_SUCCESS)
        {
            printf("Error in RSA Encryption!\n");
        }

        if(P_verbose)
        {
            display_buf("Plaintext:    ", plaintext, inSize);
            display_buf("Ciphertext:   ", cipher, pubKey_st.mModulusSize);
        }

        /* Decryption */
        if ((retval = RSA_Decrypt_PKCS1v15(&privKey_st, cipher, pubKey_st.mModulusSize, result, &resSize, P_verbose)) != RSA_SUCCESS)
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
            if (memcmp(plaintext, result, inSize) != 0 || (inSize != resSize))
            {
                if(P_verbose)
                {
                    printf("De-Ciphered text differs from cleartext\n");
                }
                failed += 1;
            }
        }
        printf(".");
    }

    return(failed == 0);
} /* End of pkcs_encryption_test */

uint32_t pkcs_encryptionWithProtectedKeys_test (uint32_t P_verbose)
{
    uint32_t failed = 0;
    uint32_t status = 0U;
    uint32_t ret = 0U;

    uint32_t i = 0;
    uint32_t j = 0;

    uint32_t inSize;
    uint32_t outSize;
    uint32_t resSize;

    uint8_t plaintext[384], cipher[384], result[384] = {0x0u, };
    uint32_t* plaintext_w = (uint32_t*)plaintext;

    struct RSA_key_stt key;

    uint32_t challenge[4] = {0x00000000, 0x00000001, 0x00000002, 0x00000003};
    uint32_t UID[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t MAC[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

    uint32_t M1[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000,
                      0x00000000, 0x00000000, 0x00000000, 0x00000000};
    uint32_t M2[4 + (3 * 96)] = {0x00000000};    /* To support RSA 3072 */

    uint32_t* authKey = master_key;
    uint32_t authKeyId = 0x1;
    uint32_t pass;
    uint32_t global_pass = 1;
    uint32_t key_size;
    rsa_key_stt* pRSA_encrypt_key;


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


    for(key_size = 1024; key_size<=3072; key_size += 1024 )
    {
        if (P_verbose)
        {
            printf ("\n-*- RSA Encryption Scheme with NVM keys with PKCS#1v1.5 %d bit -*-\n", key_size);
            printf ("Test with random messages!\n");
        }

        if( key_size == 1024 )
        {
            pRSA_encrypt_key = &RSA1024_encrypt_key;
        }
        if( key_size == 2048 )
        {
            pRSA_encrypt_key = &TV_sha1_2048.key;
        }
        if( key_size == 3072 )
        {
            pRSA_encrypt_key = &RSA3072_encrypt_key;
        }


        if(P_verbose)
        {
            printf("\n-*- RSA with PKCS#1v1.5 %d bit -*-\n\n", key_size);
            display_buf("Modulus:", (uint8_t *)pRSA_encrypt_key->mod, pRSA_encrypt_key->mod_byte_size);
            display_buf("Public Exponent:", (uint8_t *)pRSA_encrypt_key->pubexp, pRSA_encrypt_key->pubexp_byte_size);
            display_buf("Private Exponent:", (uint8_t *)pRSA_encrypt_key->privexp, pRSA_encrypt_key->privexp_byte_size);
        }

        for (i = 0U; i < 10U; i++ )
        {
            uint32_t keyIndex = 0U;

            /* Initializes Public and Private Key structures with test vectors */
            /* Signature generation: no need for Public Key */
            key.PubKey.pmModulus = (uint8_t *)pRSA_encrypt_key->mod;
            key.PubKey.mModulusSize = pRSA_encrypt_key->mod_byte_size;
            key.PubKey.pmExponent = (uint8_t *)pRSA_encrypt_key->pubexp;
            key.PubKey.mExponentSize = pRSA_encrypt_key->pubexp_byte_size;

            key.PrivKey.pmModulus = (uint8_t *)pRSA_encrypt_key->mod;
            key.PrivKey.mModulusSize = pRSA_encrypt_key->mod_byte_size;
            key.PrivKey.pmExponent = (uint8_t *)pRSA_encrypt_key->privexp;
            key.PrivKey.mExponentSize = pRSA_encrypt_key->privexp_byte_size;

            key.CNT.R = G_asymKeyCounter;
            key.RSA_flags.R = 0U;            /* No restriction on Keys */
            key.RSA_flags.B.encrypt = 1U;    /* Key is allowed for encryption */
            key.RSA_flags.B.decrypt = 1U;    /* Key is also allowed for decryption */

            /* Load Key in Non Volatile Memory. Key index varies with test number ! */
            keyIndex = (i % 4) + 1;

            pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                                  keyIndex, authKeyId ,
                                                  M1, M2, M3,
                                                  M4, M5, P_verbose);
            if(pass)
            {
                /* Generate random message to cipher */
                for (j = 0U; j < 16U; j++)
                {
                    ret = CSE_TRNG_Getrand(&plaintext[16 * j]);
                    if(ret != CSE_NO_ERR )
                    {
                        failed ++;
                    }
                }

                inSize = (plaintext_w[0])  % (key.PubKey.mModulusSize - 11U);

                if(P_verbose)
                {
                    printf("Testing RSA PKCS#1v1.5 Encryption with random message of size %d:\n", inSize);
                }
                /* Perform RSA PKCS1v1.5 encryption with public key */
                ret = CSE_RSA_PKCS1_15_encrypt(keyIndex, (vuint8_t*)plaintext, inSize, (vuint8_t*) cipher, &outSize);
                if (RSA_SUCCESS != ret)
                {
                    if(P_verbose)
                    {
                        printf("Error in RSA Encryption!\n");
                    }
                    pass = 0;
                }
                if(P_verbose)
                {
                    display_buf("Plaintext:    ", plaintext, inSize);
                    display_buf("Ciphertext:   ", cipher, key.PubKey.mModulusSize);
                }
                /* Perform RSA PKCS1v1.5 encryption with public key */
                ret = CSE_RSA_PKCS1_15_decrypt(keyIndex, (vuint8_t*)cipher, key.PubKey.mModulusSize, (vuint8_t*) result, &resSize);
                if (ret != RSA_SUCCESS)
                {
                    if(P_verbose)
                    {
                        printf("Error in RSA Decryption!\n");
                    }
                    pass = 0;
                }
                else
                {
                    if(P_verbose)
                    {
                        display_buf("De-Ciphertext:", result, resSize);
                    }
                    if (memcmp(plaintext, result, inSize) != 0 || (inSize != resSize))
                    {
                        if(P_verbose)
                        {
                            printf("De-Ciphered text differs from cleartext\n\n");
                        }
                        pass = 0;
                    }
                    else
                    {
                        if (P_verbose)
                        {
                            printf("De-Ciphered text equals cleartext\n\n");
                        }
                    }
                }
            }
            display_pass_fail(pass);
            global_pass &= pass;
        }
    }
#if 0
    if (P_verbose)
    {
        printf ("\n-*- RSA Encryption Scheme with NVM keys with PKCS#1v1.5 2048 bit -*-\n");
        printf ("Test with random messages!\n");
    }

    if(P_verbose)
    {
        printf("\n-*- RSA with PKCS#1v1.5 2048 bit -*-\n\n");
        display_buf("Modulus:", (uint8_t *)TV_sha1_2048.key.mod, TV_sha1_2048.key.mod_byte_size);
        display_buf("Public Exponent:", (uint8_t *)TV_sha1_2048.key.pubexp, TV_sha1_2048.key.pubexp_byte_size);
        display_buf("Private Exponent:", (uint8_t *)TV_sha1_2048.key.privexp, TV_sha1_2048.key.privexp_byte_size);
    }

    for (i = 0U; i < 10U; i++ )
    {
        uint32_t keyIndex = 0U;

        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;
        key.PubKey.mModulusSize = TV_sha1_2048.key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)TV_sha1_2048.key.pubexp;
        key.PubKey.mExponentSize = TV_sha1_2048.key.pubexp_byte_size;

        key.PrivKey.pmModulus = (uint8_t *)TV_sha1_2048.key.mod;
        key.PrivKey.mModulusSize = TV_sha1_2048.key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)TV_sha1_2048.key.privexp;
        key.PrivKey.mExponentSize = TV_sha1_2048.key.privexp_byte_size;

        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;            /* No restriction on Keys */
        key.RSA_flags.B.encrypt = 1U;    /* Key is allowed for encryption */
        key.RSA_flags.B.decrypt = 1U;    /* Key is also allowed for decryption */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (i % 4) + 1;

        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);

        if (pass)
        {
            /* Generate random message to cipher */
            for (j = 0U; j < 16U; j++)
            {
                ret = CSE_TRNG_Getrand(&plaintext[16 * j]);
                if(ret != CSE_NO_ERR )
                {
                    failed ++;
                }
            }

            inSize = (plaintext_w[0]) % (key.PubKey.mModulusSize - 11U);

            if(P_verbose)
            {
                printf("Testing RSA PKCS#1v1.5 Encryption with random message of size %d:\n", inSize);
            }

            /* Perform RSA PKCS1v1.5 encryption with public key */
            ret = CSE_RSA_PKCS1_15_encrypt(keyIndex, (vuint8_t*)plaintext, inSize, (vuint8_t*) cipher, &outSize);
            if (RSA_SUCCESS != ret)
            {
                if(P_verbose)
                {
                    printf("Error in RSA Encryption!\n");
                }
                failed += 1;
            }
            if(P_verbose)
            {
                display_buf("Plaintext:    ", plaintext, inSize);
                display_buf("Ciphertext:   ", cipher, key.PubKey.mModulusSize);
            }
            /* Perform RSA PKCS1v1.5 encryption with public key */
            ret = CSE_RSA_PKCS1_15_decrypt(keyIndex, (vuint8_t*)cipher, key.PubKey.mModulusSize, (vuint8_t*) result, &resSize);
            if (ret != RSA_SUCCESS)
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
                if (memcmp(plaintext, result, inSize) != 0 || (inSize != resSize))
                {
                    if(P_verbose)
                    {
                        printf("De-Ciphered text differs from cleartext\n\n");
                    }
                    failed += 1;
                }
                else
                {
                    if (P_verbose)
                    {
                        printf("De-Ciphered text equals cleartext\n\n");
                    }
                }
            }
        }
        else
        {
            failed++;
        }
        printf(".");
    }
    if (P_verbose)
    {
        printf ("\n-*- RSA Encryption Scheme with NVM keys with PKCS#1v1.5 3072 bit -*-\n");
        printf ("Test with random messages!\n");
    }

    if(P_verbose)
    {
        printf("\n-*- RSA with PKCS#1v1.5 3072 bit -*-\n\n");
        display_buf("Modulus:", (uint8_t *)TV_sha1_3072.key.mod, TV_sha1_3072.key.mod_byte_size);
        display_buf("Public Exponent:", (uint8_t *)RSA3072_encrypt_key.pubexp, RSA3072_encrypt_key.pubexp_byte_size);
        display_buf("Private Exponent:", (uint8_t *)RSA3072_encrypt_key.privexp, RSA3072_encrypt_key.privexp_byte_size);
    }

    for (i = 0U; i < 10U; i++ )
    {
        uint32_t keyIndex = 0U;

        /* Initializes Public and Private Key structures with test vectors */
        /* Signature generation: no need for Public Key */
        key.PubKey.pmModulus = (uint8_t *)RSA3072_encrypt_key.mod;
        key.PubKey.mModulusSize = RSA3072_encrypt_key.mod_byte_size;
        key.PubKey.pmExponent = (uint8_t *)RSA3072_encrypt_key.pubexp;
        key.PubKey.mExponentSize = RSA3072_encrypt_key.pubexp_byte_size;

        key.PrivKey.pmModulus = (uint8_t *)RSA3072_encrypt_key.mod;
        key.PrivKey.mModulusSize = RSA3072_encrypt_key.mod_byte_size;
        key.PrivKey.pmExponent = (uint8_t *)RSA3072_encrypt_key.privexp;
        key.PrivKey.mExponentSize = RSA3072_encrypt_key.privexp_byte_size;

        key.CNT.R = G_asymKeyCounter;
        key.RSA_flags.R = 0U;            /* No restriction on Keys */
        key.RSA_flags.B.encrypt = 1U;    /* Key is allowed for encryption */
        key.RSA_flags.B.decrypt = 1U;    /* Key is also allowed for decryption */

        /* Load Key in Non Volatile Memory. Key index varies with test number ! */
        keyIndex = (i % 4) + 1;

        pass = load_RSA_key_test( &key, authKey, (uint32_t *)UID,
                                  keyIndex, authKeyId ,
                                  M1, M2, M3,
                                  M4, M5, P_verbose);
        if (pass)
        {
            /* Generate random message to cipher */
            for (j = 0U; j < 16U; j++)
            {
                ret = CSE_TRNG_Getrand(&plaintext[16 * j]);
                if(ret != CSE_NO_ERR )
                {
                    failed ++;
                }
            }

            inSize = (plaintext_w[0]) % (key.PubKey.mModulusSize - 11U);

            if(P_verbose)
            {
                printf("Testing RSA PKCS#1v1.5 Encryption with random message of size %d:\n", inSize);
            }
            /* Perform RSA PKCS1v1.5 encryption with public key */
            ret = CSE_RSA_PKCS1_15_encrypt(keyIndex, (vuint8_t*)plaintext, inSize, (vuint8_t*) cipher, &outSize);
            if (RSA_SUCCESS != ret)
            {
                if(P_verbose)
                {
                    printf("Error in RSA Encryption!\n");
                }
                failed += 1;
            }
            if(P_verbose)
            {
                display_buf("Plaintext:    ", plaintext, inSize);
                display_buf("Ciphertext:   ", cipher, key.PubKey.mModulusSize);
            }
            /* Perform RSA PKCS1v1.5 encryption with public key */
            ret = CSE_RSA_PKCS1_15_decrypt(keyIndex, (vuint8_t*)cipher, key.PubKey.mModulusSize, (vuint8_t*) result, &resSize);
            if (ret != RSA_SUCCESS)
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
                if (memcmp(plaintext, result, inSize) != 0 || (inSize != resSize))
                {
                    if(P_verbose)
                    {
                        printf("De-Ciphered text differs from cleartext\n\n");
                    }
                    failed += 1;
                }
                else
                {
                    if (P_verbose)
                    {
                        printf("De-Ciphered text equals cleartext\n\n");
                    }
                }
            }
        }
        else
        {
            failed++;
        }
        printf(".");
    }
#endif

    return(global_pass);
} /* End of pkcs_encryptionWithProtectedKeys_test */

    /* Test with Protected Keys RAM and NVM */
uint32_t pkcs_signatureWithProtectedKeys_test( uint32_t P_verbose )
{
    uint32_t pass = 1;

    pass = pkcs_sha1_1024_signatureWithProtectedKeys_test(P_verbose);
    pass = pass & pkcs_sha1_2048_signatureWithProtectedKeys_test(P_verbose);

    return(pass);
} /* End of pkcs_signatureWithProtectedKeys_test */

uint32_t pkcs_allSha_signatureWithProtectedKeys_test( uint32_t P_verbose )
{
    uint32_t pass = 1;

    pass = pkcs_allSha_1024_signatureWithProtectedKeys_test(P_verbose);
    pass = pass & pkcs_allSha_2048_signatureWithProtectedKeys_test(P_verbose);
    pass = pass & pkcs_allSha_3072_signatureWithProtectedKeys_test(P_verbose);

    return(pass);
} /* End of pkcs_allSha_signatureWithProtectedKeys_test */

uint32_t pkcs_allSha_signatureVerifyWithProtectedKeys_test( uint32_t P_verbose )
{
    uint32_t pass = 1;

    pass = pkcs_allSha_1024_signatureVerifyWithProtectedKeys_test(P_verbose);
    pass = pass & pkcs_allSha_2048_signatureVerifyWithProtectedKeys_test(P_verbose);
    pass = pass & pkcs_allSha_3072_signatureVerifyWithProtectedKeys_test(P_verbose);

    return(pass);
} /* End of pkcs_allSha_signatureVerifyWithProtectedKeys_test */

/**
 * @}
 */
