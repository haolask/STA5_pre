/* Inclusion of the main header files of all the imported components in the
   order specified in the application wizard. The file is generated
   automatically.*/

#include "config.h"
#include "cse_typedefs.h"

#ifndef _MENU_EXT_H_
#define _MENU_EXT_H_

/*
 *  Interactive menu entries values
 */
/* Various driver tests (AES256, ECC, RSA, HMAC) */
#define EXTENDED_DRIVER_TEST                            20

/* RSA computation commands */
#define RSA_PKCS_SIGN                                   30
#define RSA_PKCS_CRYPT                                  31
#define RSA_PKCS_SIGN_PROTECTED_KEY                     32
#define RSA_PKCS_CRYPT_PROTECTED_KEY                    33

/* ECC computation commands */
#define ECC_ECDSA_SIGN_VERIFY                           34
#define ECC_ECDSA_VERIFY                                35
#define ECC_GEN_NVM_KEY_SIGN_VERIFY                     36
#define ECC_ECDSA_SIGN_VERIFY_PROTECTED_KEY             37
#define ECC_ECDSA_VERIFY_PROTECTED_KEY                  38

/* ECIES computation commands */
#define ECC_ECIES_ENCRYPT_DECRYPT                       39
#define ECC_ECIES_DECRYPT                               40

/* Hash computation commands */
#define HASH_TEST                                       41

/* HMAC computation commands */
#define HMAC_MAC_GENERATION                             42
#define HMAC_MAC_VERIFICATION                           43
#define HMAC_MAC_GENERATION_PROTECTED_KEY               44
#define HMAC_MAC_VERIFICATION_PROTECTED_KEY             45

/* AES128 GCM & CCM commands */

/* AES256 Key management commands */
#define AES256_ECB_ENCRYPT_DECRYPT                      60
#define AES256_CBC_ENCRYPT_DECRYPT                      61
#define AES256_CCM_ENCRYPT_DECRYPT                      62
#define AES256_CMAC_TAG_GENERATION                      63
#define AES256_CMAC_TAG_VERIFICATION                    64
#define AES256_GCM_AUTHENTICATED_ENCRYPTION             65
#define AES256_GCM_AUTHENTICATED_DECRYPTION             66
#define AES256_EXPORT_RAM_KEY                           67
#define AES256_ECB_ENCRYPT_DECRYPT_PROTECTED_KEY        68
#define AES256_CBC_ENCRYPT_DECRYPT_PROTECTED_KEY        69
#define AES256_CCM_ENCRYPT_DECRYPT_PROTECTED_KEY        70
#define AES256NVM_KEY_UPDATE                            71

/* Key derivation commands */
#define ECC_ECDH_KEY_AGREEMENT_TEST_with_PK             80
#define ECC_ECDH_KEY_AGREEMENT_TEST                     81
#define ECC_ECDH_FIXED_KEY_AGREEMENT_TEST               82
#define ECC_ECDH_FIXED_SERVER_KEY_AGREEMENT_TEST        83

/* PBKDF2 commands */

/* TLS commands */
#define TLSV12_PRF                                      90

/* OTP commands */
#define OTP_READ_TEST                                   100
#define OTP_WRITE_TEST                                  101
#define OTP_WRITE_ENCBOOT_KEY                           102
#define OTP_READ_TEST_WITH_EBK                          103
#define OTP_WRITE_TEST_WITH_EBK                         104

/* Memory management commands */
#define EXT_MEM_RANGE                                   110
#define LOCK_EXT_MEM_RANGE                              111
#define M3_EMUL_REG_CONFIG                              112
#define LOCK_M3_EMUL_REG_CONFIG                         113
#define A7_EMUL_REG_CONFIG                              114
#define LOCK_A7_EMUL_REG_CONFIG                         115

/* KS commands */
#define NVM_KS_RW_TEST                                  120
#define IMPORT_KEY_IMAGE                                121
#define GET_RAM_MONO_CNT                                122
#define SET_RAM_MONO_CNT                                123
#define DUMP_LS_AREA                                    124

extern uint32_t reduced_test_vector_set;
extern void display_menu_ext( void );
extern void display_test_configuration_settings(void);
extern uint32_t menu_ext_entry(uint32_t operation, uint32_t* pverbose);

#endif //_MENU_EXT_H_
/**
 * @}
 */
