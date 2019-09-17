/*
 * CSE_ext_UT_vectors.h
 *
 *  Created on: 10 mars 2016
 *      Author: pierre guillemin
 */

#ifndef CSE_DRIVER_TEST_VALIDATIONTEST_CSE_EXT_UT_VECTORS_H_
#define CSE_DRIVER_TEST_VALIDATIONTEST_CSE_EXT_UT_VECTORS_H_

#include "cse_typedefs.h"
#include "serialprintf.h"
#include "CSE_ext_ECC.h"
//#include "startup.h"
#include "CSE_ext_test_globals.h"


extern uint32_t passed;
extern uint32_t ret;

#define C_NB_OF_NVM_KEY_UT_TV 15
#define C_NB_OF_MEMORY_RANGE_MAX 10

/* Description of valid memory range really present on the silicon */
#define C_NB_OF_MEMORY_RANGE (3 + CORE_HAS_OVERLAY_FLASH \
                                + CORE_HAS_IRAM0 + CORE_HAS_DRAM0 \
                                + CORE_HAS_IRAM1 + CORE_HAS_DRAM1 \
                                + CORE_HAS_IRAM2 + CORE_HAS_DRAM2 )

#define C_PUB_KEY_X 0
#define C_PUB_KEY_Y 1
#define C_PRIV_KEY  2

#define C_RSA_PUB_KEY 0
#define C_RSA_MODULUS 1

typedef struct
{
    char mMemName[32];        /* Memory name */
    uint32_t mMemStartAdd;    /* Memory start address, address of first byte */
    uint32_t mMemEndAdd;      /* Memory end address, address of last byte */
    uint32_t type;            /* 0 for flash, 1 for ram */
} memRangeDescriptor_stt;

typedef struct
{
    uint32_t * mpM1Add;
    uint32_t * mpM2Add;
    uint32_t * mpM3Add;
    uint32_t * mpM4Add;
    uint32_t * mpM5Add;
} CSE_ext_valTest_MAddr_stt;

typedef struct
{
    /** @brief public key structure */
    struct ECCpubKeyByteArray_stt  PubKeyByteArray;
    /** @brief private key structure */
    struct ECCprivKeyByteArray_stt PrivKeyByteArray;
} ECC_keyPair_stt;


/* Memory range descriptor */
extern const memRangeDescriptor_stt G_memRangeAdd[];

void CSE_ext_valTest_generateMessageAddressInit_startBufAdd(uint32_t P_messageNb,
                                                            CSE_ext_valTest_MAddr_stt P_messageAdd,
                                                            CSE_ext_valTest_MAddr_stt * P_pMessageWrongAdd,
                                                            memRangeDescriptor_stt P_memRangeAdd, uint32_t P_verbose);

void CSE_ext_valTest_generateMessageAddressInit_endBufAdd(uint32_t P_messageNb,
                                                          CSE_ext_valTest_MAddr_stt P_messageAdd,
                                                          CSE_ext_valTest_MAddr_stt * P_pMessageWrongAdd,
                                                          memRangeDescriptor_stt P_memRangeAdd, uint32_t P_verbose);

void CSE_ext_valTest_ecc_initMasterKeyForEccTests(void);
uint32_t CSE_ext_valTest_ecc_PubKeyCryptoVal_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_PrivKeyCryptoVal_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_RamPubKeyCryptoVal_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_RamPrivKeyCryptoVal_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_NVMKeyLoadingIndexTest_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_NVMKeyLoadingInWrongAddRange_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_RamKeyLoadingInWrongAddRange_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_NVMKeyLoadingWithWP_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaSignGenerateAndVerify_test(uint32_t verbose, uint32_t reduced_tv_nb);
uint32_t CSE_ext_valTest_ecc_EcdsaSignGenerateAndVerify_PK_test(uint32_t verbose,  uint32_t reduced_tv_nb);
uint32_t CSE_ext_valTest_ecc_EcdsaSignVerify_test(uint32_t verbose, uint32_t reduced_tv_nb);
uint32_t CSE_ext_valTest_ecc_EcdsaSignVerify_PK_test(uint32_t P_verbose, uint32_t reduced_tv_nb);
uint32_t CSE_ext_valTest_ecc_EcdsaSignGenerateAndVerifySecurityStrength_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaGenKeyInNvmSignAndVerify_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaLoadKeyInNvmSignAndVerify_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaLoadKeyInNvmSignAndVerifyFlagTest_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaSignGenWrongAddRange_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaSignVerifWrongAddRange_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaWithDP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EcdsaRamKeyEmptyDetection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportKeyIndexTest_test(uint32_t verbose);
uint32_t CSE_ext_valTest_ecc_KeyExportToWrongAddRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_KeyExportFromEmptyLoc_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportRAMPubKey_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportNVMPubKey_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_generateAndLoadKeyPairIndexTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_keyInitTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_generateAndLoadKeyPairInRam_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_generateAndLoadKeyPairInNVMEmptyLocation_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_generateAndLoadKeyPairInNVMFlagTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesEncryptionDecryption_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesDecryption_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesX963KDF_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesEncryptionUnitary_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesWrongKeyIndexDetection_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesRamKeyEmptyDetection_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesUrfDetectionNvmKeys_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesUrfDetectionRamKey_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesEncryptWrongAddRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesDecryptWrongAddRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_EciesWithDP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportRamPrivateKeyTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportNvmPrivateKeyTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportNvmPrivateKeyNotExtractTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportNvmPrivateKeyNotExistTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_generateAndExportNvmPrivateKeyTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_ecc_exportNvmPrivateKeyWrongAddRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramKeyPairLoading_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramPubKeyLoading_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramPrivKeyLoading_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramKeyNoModulusLoading_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramKeyPairCryptoValidity_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramKeyPairLoadingOnNonEmptyLoc_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramKeyLoadingMemoryProtection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramKeyPairInitAtZero_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_protectedKeyPairLoading_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_pubKeyLoading_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_privateKeyLoading_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_KeyNoModulusLoading_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_nvmKeyPairCryptoValidity_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_nvmRamKeyPairCryptoValidity_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_protectedKeyLoadWrongAddRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_protectedKeyLoadIndexTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_nvmProtectedKeyLoadCntTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_ramProtectedKeyLoadCntTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaNvmProtectedKeyLoadWithWP_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaRamProtectedKeyLoadWithBP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaRamUsageWithBP_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_nvmKeyEmptyAfterProduction_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_loadNVMKeyAuthKeyEmpty_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_loadNVMKeyCorruptMessages_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaNvmProtectedKeyLoad_WPNotSetAfterProd_test(uint32_t P_verbose);

void CSE_ext_valTest_rsa_initMasterKeyForRsaTests(void);
uint32_t CSE_ext_valTest_rsa_nvmKeyLoadWithWildCard_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_nvmKeyLoad_WCNotSetAfterProd_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaRamUsageWithDP_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportNvmPrivateKeyTest_test (uint32_t P_verbose, uint32_t reduced_test_vector_nb );
uint32_t CSE_ext_valTest_rsa_exportNvmPrivateKeyWrongAddRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportNvmPrivateKeyNotExtractTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportNvmPrivateKeyNotExistTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_privKeyExportFromEmptyLoc_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportProtectedRamPrivateKey_NotExtractableTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportProtectedRamPrivateKey_ExtractableTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportRamPrivateKeyTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportNVMPubKey_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportRAMPubKey_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_pubKeyExportToWrongAddRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_pubKeyExportFromEmptyLoc_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportPubKeyIndexTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportPubKeyBuffSizeTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsa_exportNvmPrivateKeyBuffSizeTest_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_signatureGeneration_invalidMemRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_signatureVerification_invalidMemRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_messageEncryption_invalidMemRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_messageDecryption_invalidMemRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_signatureGeneration_signFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_signatureVerification_verifyFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_messageEncryption_encryptFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaPkcs_messageDecryption_decryptFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_rsaUsageRestrictFlag_BP_test ( uint32_t P_verbose );
uint32_t CSE_ext_valTest_rsaUsageRestrictFlag_DP_test ( uint32_t P_verbose );

void CSE_ext_valTest_aes256_initMasterKeyForAes256Tests(void);
uint32_t CSE_ext_valTest_aes256Ecb_encryptionDecryption_invalidMemRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256Cbc_encryptionDecryption_invalidMemRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256Ccm_encryptionDecryption_invalidMemRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256Cmac_generationVerificaton_invalidMemRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256Gcmc_encryptionDecryption_invalidMemRange_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_ramKeyLoadingOnNonEmptyLoc_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_ramKeyLoadingMemoryProtection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_ramKeyLoadingKeySize_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_ramProtectedKeyLoadCntTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_protectedKeyLoadingMemoryProtection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_nvmProtectedKeyLoadCntTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_protectedKeyLoadIndexTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256RamKeyLoadingWithBP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256RamProtectedKeyLoadWithBP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256RamProtectedKeyLoadingWithDP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_loadNVMKeyAuthKeyEmpty_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_nvmKeyEmptyAfterProduction_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_nvmKeyLoad_WCNotSetAfterProd_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_nvmKeyLoadWithWildCard_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256NvmProtectedKeyLoad_WPNotSetAfterProd_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256NvmProtectedKeyLoadWithWP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_loadNVMKeyCorruptMessages_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_aes256_exportRamKey_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_macGenerationVerificaton_invalidMemRange_test( uint32_t P_verbose );

uint32_t CSE_ext_valTest_aes256_keyExportFromRamEmptyLoc_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_exportKeyBuffSizeTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_keyExportToWrongAddRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_exportProtectedRamKey_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_aes256ccm_paramLenght_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256cmac_paramLenght_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256gcm_paramLenght_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256gcm_tagLenghtDecrypt_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_ramKeyLenght_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_nvmKeyLenght_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_AES256_ECB_encryption_encryptFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_AES256_ECB_decryption_decryptFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_AES256_CBC_encryption_encryptFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_AES256_CBC_decryption_decryptFlag_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_AES256_CCM_urFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_AES256_CMAC_urFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_AES256_GCM_urFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256UsageRestrictFlag_BP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256UsageRestrictFlag_DP_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_AES256_emptyRamKeyDetection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_aes256_wrongKeyIndexDetection_test(uint32_t P_verbose);

void CSE_ext_valTest_hmac_initMasterKeyForHmacTests(void);
uint32_t CSE_ext_valTest_hmac_exportRamKey_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_keyExportFromRamEmptyLoc_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_exportKeyBuffSizeTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_keyExportToWrongAddRange_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_exportProtectedRamKey_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_exportRamKey_variableKeySize_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_HMAC_emptyRamKeyDetection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_wrongKeyIndexDetection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_nonValidHashAlgoDetection_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_hmac_ramKeyLoadingOnNonEmptyLoc_test (uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_ramKeyLoadingMemoryProtection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_ramKeyLoadingKeySize_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_ramProtectedKeyLoadCntTest_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_hmac_protectedKeyLoadingMemoryProtection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_nvmProtectedKeyLoadCntTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_protectedKeyLoadIndexTest_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmacRamKeyLoadingWithBP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmacRamProtectedKeyLoadWithBP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmacRamProtectedKeyLoadingWithDP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_loadNVMKeyAuthKeyEmpty_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_nvmKeyEmptyAfterProduction_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_nvmKeyLoad_WCNotSetAfterProd_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_nvmKeyLoadWithWildCard_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmacNvmProtectedKeyLoad_WPNotSetAfterProd_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmacNvmProtectedKeyLoadWithWP_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmac_loadNVMKeyCorruptMessages_test(uint32_t P_verbose);

uint32_t CSE_ext_valTest_HMAC_urFlag_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hmacUsageRestrictFlag_BP_test( uint32_t P_verbose );
uint32_t CSE_ext_valTest_hmacUsageRestrictFlag_DP_test(uint32_t P_verbose);

extern const uint8_t C_P256n[32];
extern const uint8_t C_P256nPlus1[32];
extern const uint8_t C_P256nMinus1[32];
extern const uint8_t C_P256p[32];
extern const uint8_t C_P256pMinus1[32];
extern const uint8_t C_P256pPlus1[32];
extern const uint8_t C_PRIV_KEY0[1];

extern const uint8_t TV_evenModulus[128];
extern const uint8_t TV_pubExpEven[1];
extern const uint8_t TV_pubExp2[1];
extern const uint8_t TV_ModulusPlus2[128];

/* AES256 Test vectors --------------------------------------------------------*/
    /* AES256 ECB test vector coming from NIST CAVS 11.1 AESVS KeySbox test data for ECB ECBKeySbox256.rsp
     * Used also for CBC with IV = 0 */
extern const unsigned char key_tv1[32];
extern const unsigned char iV_tv1[16];
extern const unsigned char plainText_tv1[16];
extern const unsigned char cipherText_tv1[16];
extern const unsigned char key_tv2[32];

/* HMAC SHA256 Test vectors -----------------------------------------------*/
#define C_HMAC256_KEY_SIZE_TV 64
#define C_HMAC256_TAG_SIZE_TV 32
#define C_HMAC_TV_MESSAGE_SIZE 128

extern  const unsigned char hmac256KeyTv1[C_HMAC256_KEY_SIZE_TV];
extern const unsigned char hmac256MessageTv1[C_HMAC_TV_MESSAGE_SIZE];
extern const unsigned char hmac256TagTv1[C_HMAC256_TAG_SIZE_TV];

extern const unsigned char hmac256KeyTv2[C_HMAC256_KEY_SIZE_TV];
extern const unsigned char hmac256MessageTv2[C_HMAC_TV_MESSAGE_SIZE];
extern const unsigned char hmac256TagTv2[C_HMAC256_TAG_SIZE_TV];

#endif /* CSE_DRIVER_TEST_VALIDATIONTEST_CSE_EXT_UT_VECTORS_H_ */
