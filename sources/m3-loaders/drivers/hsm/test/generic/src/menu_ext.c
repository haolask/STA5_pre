/* Inclusion of the main header files of all the imported components in the
   order specified in the application wizard. The file is generated
   automatically.*/

#include <string.h>
#include "sta_common.h"

#include "cse_client.h"
#include "CSE_Constants.h"
#include "CSE_ext_manager.h"
#include "config.h"
#include "serialprintf.h"
#include "serial_input.h"

#include "CSE_ext_AES256_tests.h"
#include "CSE_ext_ECC_ECDH_test.h"
#include "CSE_ext_ECC_ECDSA_test.h"
#include "CSE_ext_ECC_ECIES_test.h"
#include "CSE_ext_ECC_NVM_KEY_test.h"
#include "CSE_ext_extendedDriverTests.h"
#include "CSE_ext_hash_test.h"
#include "CSE_ext_HMAC_tests.h"
#include "CSE_Manager_test.h"
#include "CSE_ext_OTP_test.h"
#include "CSE_ext_RSA_PKCS_test.h"
#include "CSE_ext_TLSv12_PRF_test.h"
#include "ks_proxy.h"
#include "menu_ext.h"
#include "test_support.h"

uint32_t reduced_test_vector_set = 0;

/* First set of tests */
#define RSA_TESTS_ENABLED
#define HASH_TESTS_ENABLED
#define ECC_TESTS_ENABLED
#define AES256_KEY_TESTS_ENABLED
/* Second set of tests */
#define AES256_TESTS_ENABLED
#define AES256_CCM_TESTS_ENABLED
/* 3rd set of tests */
#define AES256_GCM_TESTS_ENABLED
/* fourth set of tests */
#define ECDH_PRF_TESTS_ENABLED

void display_menu_ext( void )
{
	printf("\n\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("|                                                             |\n");
    printf("|                     SHE_ext services                        |\n");
    printf("|                                                             |\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("\nA/ SHE_ext - crypto services: ECC, RSA, HASH, AES256 and HMAC extensions dedicated commands\n");
    printf("--------------------------------------------------------------------------------------------\n");
	printf("--> Set of AES256 + ECC + RSA + HMAC crypto services\n");
    printf(" %d - Run extended driver tests\n", EXTENDED_DRIVER_TEST);
    printf("\n");

#if defined(RSA_TESTS_ENABLED)
    printf("--> RSA crypto services\n");
    printf("<> With RAM key\n");
    printf("---------------\n");
    printf(" %d - RSA PKCS1v1.5 signature tests\n", RSA_PKCS_SIGN);
    printf(" %d - RSA PKCS1v1.5 encryption tests\n", RSA_PKCS_CRYPT);
    printf("<> With NVM keys\n");
    printf("----------------\n");
    printf(" %d - RSA PKCS1v1.5 signature tests (NVM)\n", RSA_PKCS_SIGN_PROTECTED_KEY);
    printf(" %d - RSA PKCS1v1.5 encryption tests (NVM)\n", RSA_PKCS_CRYPT_PROTECTED_KEY);
    printf("\n");
#endif
#if defined(ECC_TESTS_ENABLED)
    printf("--> ECC/ECDSA crypto services\n");
    printf("<> With RAM key\n");
    printf("---------------\n");
    printf(" %d - ECC ECDSA signature generation and verification tests\n", ECC_ECDSA_SIGN_VERIFY);
    printf(" %d - ECC ECDSA signature verification tests\n", ECC_ECDSA_VERIFY);
    printf("<> With NVM keys\n");
    printf("----------------\n");
    printf(" %d - ECC key pair generation and ECDSA signature generation & verification tests (NVM)\n", ECC_GEN_NVM_KEY_SIGN_VERIFY);
    printf(" %d - ECC ECDSA signature generation and verification (NVM)\n", ECC_ECDSA_SIGN_VERIFY_PROTECTED_KEY);
    printf(" %d - ECC ECDSA signature verification (NVM)\n", ECC_ECDSA_VERIFY_PROTECTED_KEY);
    printf("\n");
    printf("--> ECIES crypto services\n");
    printf(" %d - ECC ECIES Encryption / Decryption tests\n", ECC_ECIES_ENCRYPT_DECRYPT);
    printf(" %d - ECC ECIES Decryption tests\n", ECC_ECIES_DECRYPT);
    printf("\n");
#endif
#if defined(HASH_TESTS_ENABLED)
    printf("--> HASH crypto services\n");
    printf(" %d - HASH tests (SHA1, SHA224, SHA256 ..)\n", HASH_TEST);
    printf("\n");
#endif
#if defined(HMAC_TESTS_ENABLED)
    printf("--> HMAC crypto services\n");
    printf("<> With RAM key\n");
    printf("---------------\n");
    printf(" %d - HMAC MAC generation tests\n", HMAC_MAC_GENERATION);
    printf(" %d - HMAC MAC verification tests\n", HMAC_MAC_VERIFICATION);
    printf("<> With NVM keys\n");
    printf("----------------\n");
    printf(" %d - HMAC MAC generation tests (NVM)\n", HMAC_MAC_GENERATION_PROTECTED_KEY);
    printf(" %d - HMAC MAC verification tests (NVM)\n", HMAC_MAC_VERIFICATION_PROTECTED_KEY);
#endif

    printf("\nB/ SHE_ext - AES256 crypto services: ECB, CBC, CCM, GCM, CMAC\n");
    printf("-------------------------------------------------------------\n");
    printf("<> With RAM key\n");
    printf("---------------\n");
#if defined(AES256_TESTS_ENABLED)
    printf(" %d - AES256 ECB encryption / decryption test\n", AES256_ECB_ENCRYPT_DECRYPT);
    printf(" %d - AES256 CBC encryption / decryption test\n", AES256_CBC_ENCRYPT_DECRYPT);
#endif
#if defined(AES256_CCM_TESTS_ENABLED)
    printf(" %d - AES256 CCM encryption / decryption test\n", AES256_CCM_ENCRYPT_DECRYPT);
#endif
#if defined(AES256_TESTS_ENABLED)
    printf(" %d - AES256 CMAC Tag generation test\n", AES256_CMAC_TAG_GENERATION);
    printf(" %d - AES256 CMAC Tag verification test\n", AES256_CMAC_TAG_VERIFICATION);
#endif
#if defined(AES256_GCM_TESTS_ENABLED)
    printf(" %d - AES256 GCM Authenticated Encryption test\n", AES256_GCM_AUTHENTICATED_ENCRYPTION);
    printf(" %d - AES256 GCM Authenticated Decryption test\n", AES256_GCM_AUTHENTICATED_DECRYPTION);
#endif
#if defined(AES256_KEY_TESTS_ENABLED)
    printf(" %d - AES256 Export RAM Key test\n", AES256_EXPORT_RAM_KEY);
    printf("<> With NVM keys\n");
    printf("-----------------\n");
    printf(" %d - AES256 ECB encryption / decryption test (NVM)\n", AES256_ECB_ENCRYPT_DECRYPT_PROTECTED_KEY);
    printf(" %d - AES256 CBC encryption / decryption test (NVM)\n", AES256_CBC_ENCRYPT_DECRYPT_PROTECTED_KEY);
    printf(" %d - AES256 CCM encryption / decryption test (NVM)\n", AES256_CCM_ENCRYPT_DECRYPT_PROTECTED_KEY);
	printf(" %d - AES256 key update (NVM)\n", AES256NVM_KEY_UPDATE);
#endif

    printf("\nC/ SHE_ext - Key derivation commands\n");
    printf("------------------------------------\n");
#if defined(ECC_TESTS_ENABLED) && defined(ECDH_PRF_TESTS_ENABLED)
    printf("<> With RAM key\n");
    printf("---------------\n");
    printf(" %d - ECC ECDH-FIXED Server Key Agreement tests \n", ECC_ECDH_FIXED_SERVER_KEY_AGREEMENT_TEST);
    printf("<> With NVM keys\n");
    printf("-----------------\n");
    printf(" %d - ECC ECDH Client/Server Key Agreement tests (NVM)\n", ECC_ECDH_KEY_AGREEMENT_TEST_with_PK);
    printf(" %d - ECC ECDH Client/Server Key Agreement tests (RAM + NVM)\n", ECC_ECDH_KEY_AGREEMENT_TEST);
    printf(" %d - ECC ECDH-FIXED Key Agreement tests (NVM)\n", ECC_ECDH_FIXED_KEY_AGREEMENT_TEST);
#endif

    printf("\nD/ SHE_ext - TLS command\n");
    printf("------------------------\n");
#if defined(ECDH_PRF_TESTS_ENABLED)
    printf(" %d - TLS V1.2 PRF secret generation test\n", TLSV12_PRF);
#endif

    printf("\nE/ SHE_ext - OTP commands\n");
    printf("-------------------------\n");
    printf(" %d - OTP read test\n", OTP_READ_TEST);
    printf(" %d - OTP write test\n", OTP_WRITE_TEST);
    printf(" %d - OTP write Encrypted Boot Key test\n", OTP_WRITE_ENCBOOT_KEY);
    printf(" %d - OTP read test with Encrypted Boot Key programmed\n", OTP_READ_TEST_WITH_EBK);
    printf(" %d - OTP write test with Encrypted Boot Key programmed\n", OTP_WRITE_TEST_WITH_EBK);

    printf("\nF/ SHE_ext - Memory management commands\n");
    printf("---------------------------------------\n");
    printf(" %d - Configuration of external memory range used by HSM\n", EXT_MEM_RANGE);
    printf(" %d - Lock External memory range configuration service\n", LOCK_EXT_MEM_RANGE);
    printf(" %d - M3 Emulated registers configuration\n", M3_EMUL_REG_CONFIG);
    printf(" %d - Lock of M3 Emulated registers configuration\n", LOCK_M3_EMUL_REG_CONFIG);
    printf(" %d - A7 Emulated registers configuration\n", A7_EMUL_REG_CONFIG);
    printf(" %d - Lock of A7 Emulated registers configuration\n", LOCK_A7_EMUL_REG_CONFIG);

    printf("\nG/ SHE_ext - Key storage commands\n");
    printf("---------------------------------------\n");
    printf(" %d - Key Storage: NVM autotest\n", NVM_KS_RW_TEST);
    printf(" %d - External protected key image import from NVM to eHSM \n", IMPORT_KEY_IMAGE);
    printf(" %d - Get RAM monotonic counter value\n", GET_RAM_MONO_CNT);
    printf("       [Note - Will only succeed if RAM_cnt is the current selected monotonic counter source]\n");
    printf(" %d - Set RAM monotonic counter value\n", SET_RAM_MONO_CNT );
    printf("       [Note - Will only succeed if RAM_cnt is the current selected monotonic counter source]\n");
    printf(" %d - Dump Load STore area \n", DUMP_LS_AREA);
    printf("\n");
}

uint32_t menu_ext_entry( uint32_t operation, uint32_t* pverbose )
{
    uint32_t ret = 0;
    uint32_t pass = 0;
    uint32_t dram_start = EXT_MEM_RANGE_INOUT_START;
    uint32_t dram_end = EXT_MEM_RANGE_INOUT_END;
    uint32_t flash_start = EXT_MEM_RANGE_INPUT_START;
    uint32_t flash_end = EXT_MEM_RANGE_INPUT_END;
    uint32_t counter;

    /* Check it is a supported option and call the associated code */
    switch(operation) {
	case EXTENDED_DRIVER_TEST:
        pass = test_CSE_extendedDriver(*pverbose);
        display_pass_fail(pass);
        break;
#if defined(RSA_TESTS_ENABLED)
    case RSA_PKCS_SIGN:
        pass = pkcs_signature_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case RSA_PKCS_CRYPT:
        pass = pkcs_encryption_test( *pverbose );
        display_pass_fail(pass);
        break;
    case RSA_PKCS_SIGN_PROTECTED_KEY:
        pass = pkcs_signatureWithProtectedKeys_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case RSA_PKCS_CRYPT_PROTECTED_KEY:
        pass = pkcs_encryptionWithProtectedKeys_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
#endif
#if defined(ECC_TESTS_ENABLED)
    case ECC_ECDSA_SIGN_VERIFY:
        pass = ecc_ecdsa_sha_signVerify_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_ECDSA_VERIFY:
        pass = ecc_ecdsa_sha_signatureVerification_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_GEN_NVM_KEY_SIGN_VERIFY:
        pass = ecc_generateEcKeyInNVM_SignAndVerify_test(*pverbose);
        display_pass_fail(pass);
        break;
    case ECC_ECDSA_SIGN_VERIFY_PROTECTED_KEY:
        pass = ecc_ecdsa_sha_signVerify_PK_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_ECDSA_VERIFY_PROTECTED_KEY:
        pass = ecc_ecdsa_sha_signatureVerification_PK_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_ECIES_ENCRYPT_DECRYPT:
        pass = ecc_ecies_encryptionDecryption_test(*pverbose);
        display_pass_fail(pass);
        break;
    case ECC_ECIES_DECRYPT:
        pass = ecc_ecies_decryption_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(HASH_TESTS_ENABLED)
    case HASH_TEST:
        pass = hash_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(HMAC_TESTS_ENABLED)
    case HMAC_MAC_GENERATION:
        pass = hmac_macGeneration_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case HMAC_MAC_VERIFICATION:
        pass = hmac_macVerification_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case HMAC_MAC_GENERATION_PROTECTED_KEY:
        pass = hmac_macGeneration_pk_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case HMAC_MAC_VERIFICATION_PROTECTED_KEY:
        pass = hmac_macVerification_pk_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
#endif
#if defined(AES256_TESTS_ENABLED)
    case AES256_ECB_ENCRYPT_DECRYPT:
        pass = aes256_ecb_encryptionDecryption_test(*pverbose);
        display_pass_fail(pass);
        break;
    case AES256_CBC_ENCRYPT_DECRYPT:
        pass = aes256_cbc_encryptionDecryption_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(AES256_CCM_TESTS_ENABLED)
    case AES256_CCM_ENCRYPT_DECRYPT:
        pass = aes256_ccm_encryptionDecryption_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(AES256_TESTS_ENABLED)
    case AES256_CMAC_TAG_GENERATION:
        pass = aes256_cmac_tagGenerationVerification_test(*pverbose);
        display_pass_fail(pass);
        break;
    case AES256_CMAC_TAG_VERIFICATION:
        pass = aes256_cmac_tagVerification_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(AES256_GCM_TESTS_ENABLED)
    case AES256_GCM_AUTHENTICATED_ENCRYPTION:
        pass = aes256_gcm_authenticatedEncryption_test(*pverbose);
        display_pass_fail(pass);
        break;
    case AES256_GCM_AUTHENTICATED_DECRYPTION:
        pass = aes256_gcm_authenticatedDecryption_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(AES256_KEY_TESTS_ENABLED)
    case AES256_EXPORT_RAM_KEY:
        aes256_exportRamKey_test( *pverbose );
        break;
	case AES256_ECB_ENCRYPT_DECRYPT_PROTECTED_KEY:
        pass = aes256_ecb_encryptionDecryption_PK_test(*pverbose);
        display_pass_fail(pass);
        break;
	case AES256_CBC_ENCRYPT_DECRYPT_PROTECTED_KEY:
        pass = aes256_cbc_encryptionDecryption_PK_test(*pverbose);
        display_pass_fail(pass);
        break;
	case AES256_CCM_ENCRYPT_DECRYPT_PROTECTED_KEY:
        pass = aes256_ccm_encryptionDecryption_PK_test(*pverbose);
        display_pass_fail(pass);
        break;
	case AES256NVM_KEY_UPDATE:
        pass = CSE_NVM_AES256_key_update_interactive_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
#if defined(ECC_TESTS_ENABLED) && defined(ECDH_PRF_TESTS_ENABLED)
    case ECC_ECDH_KEY_AGREEMENT_TEST:
        pass = ecc_ecdh_keyAgreementClientServer_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_ECDH_FIXED_KEY_AGREEMENT_TEST:
        pass = ecc_ecdh_keyAgreement_fixed_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_ECDH_FIXED_SERVER_KEY_AGREEMENT_TEST:
        pass = ecc_ecdh_serverKeyAgreement_test(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
    case ECC_ECDH_KEY_AGREEMENT_TEST_with_PK:
        pass = ecc_ecdh_keyAgreementClientServer_test_with_PK(*pverbose, reduced_test_vector_set);
        display_pass_fail(pass);
        break;
#endif
#if defined(ECDH_PRF_TESTS_ENABLED)
    case TLSV12_PRF:
        pass = tlsv12_prf_test(*pverbose);
        display_pass_fail(pass);
        break;
#endif
    case OTP_READ_TEST:
        pass = otp_read_test(*pverbose, 0);
        display_pass_fail(pass);
        break;
    case OTP_WRITE_TEST:
        pass = otp_write_test(*pverbose, 0);
        display_pass_fail(pass);
        break;
    case OTP_READ_TEST_WITH_EBK:
        pass = otp_read_test(*pverbose, 1);
        display_pass_fail(pass);
        break;
    case OTP_WRITE_TEST_WITH_EBK:
        pass = otp_write_test(*pverbose, 1);
        display_pass_fail(pass);
        break;
#ifndef HSM_RELEASE
	case OTP_WRITE_ENCBOOT_KEY:
        pass = otp_write_encrypted_boot_key(*pverbose);
		display_pass_fail(pass);
		break;
#endif
    case EXT_MEM_RANGE:
		printf("Set 0x%08X .. 0x%08X range as valid external memory range for requests buffers\n", dram_start, dram_end);
        ret = CSE_ext_set_valid_ext_mem_range(dram_start, dram_end, flash_start, flash_end);
        display_pass_fail(ret == CSE_NO_ERR);
        break;
    case LOCK_EXT_MEM_RANGE:
		printf("Lock valid external memory range configuration service\n");
        ret = CSE_ext_lock_valid_ext_mem_range();
        display_pass_fail(ret == CSE_NO_ERR);
        break;
	case M3_EMUL_REG_CONFIG:
		ret = m3_emul_reg_config_management(*pverbose);
		display_pass_fail(ret == CSE_NO_ERR);
		break;
    case LOCK_M3_EMUL_REG_CONFIG:
		printf(" Lock valid emulated register configuration\n");
        ret = CSE_API_ext_lock_m3_she_reg_mem_config();
        display_pass_fail(ret == CSE_NO_ERR);
        break;
	case A7_EMUL_REG_CONFIG:
        ret = a7_emul_reg_config_management(*pverbose);
		display_pass_fail(ret == CSE_NO_ERR);
		break;
    case LOCK_A7_EMUL_REG_CONFIG:
		printf(" Lock valid emulated register configuration\n");
        ret = CSE_API_ext_lock_a7_she_reg_mem_config();
        display_pass_fail(ret == CSE_NO_ERR);
        break;
    case NVM_KS_RW_TEST:
        printf("--> Perform KS NVM autotest - Try writing payload and then erase block used for Key storage\n");
        NVM_autotest();
		break;
    case IMPORT_KEY_IMAGE:
        printf("--> Import of external key image\n");
        ret = remote_key_import_config_management(*pverbose);
        display_pass_fail(ret == CSE_NO_ERR);
        break;
    case GET_RAM_MONO_CNT:
        printf(" Get RAM monotonic counter value (if active)\n");
        ret = CSE_ext_Get_ram_monotonic_counter_value(&counter);
        printf("returned ERC : %02x\n", ret);
        if(CSE_NO_ERR == ret) {
            counter = CSE->P1.R;
            printf("RAM monotonic counter value : %d\n", counter);
        }
        display_pass_fail(ret == CSE_NO_ERR);
        break;
    case SET_RAM_MONO_CNT:
        printf(" Set RAM monotonic counter value (if active)\n");
        get_int(" Please enter the value to use : ", &counter );
        ret = CSE_ext_Set_ram_monotonic_counter_value(counter);
        printf("returned ERC : %02x\n", ret);
        if(CSE_NO_ERR == ret)
            printf("RAM monotonic counter value is now : %d\n", counter);
        display_pass_fail(ret == CSE_NO_ERR);
        break;
    case DUMP_LS_AREA:
	NVM_dump_LS_area();
	break;
    default:
        ret = 1;
        break;
    }
    return(ret);
}
