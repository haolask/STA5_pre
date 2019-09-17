/*
 * CSE_ext_extendedDriverTestsFuncList.h
 *
 *  Created on: 19 juil. 2017
 *      Author: pierre guillemin
 */

#ifndef CSE_DRIVER_TEST_CSE_EXT_EXTENDEDDRIVERTESTSFUNCLIST_H_
#define CSE_DRIVER_TEST_CSE_EXT_EXTENDEDDRIVERTESTSFUNCLIST_H_

#include "cse_types.h"
#include "CSE_ext_AES256.h"
#include "CSE_ext_ECC.h"
#include "CSE_ext_RSA.h"

uint32_t test_CSE_extendedDriver_AES256_ECB(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_AES256_CBC(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_AES256_CCM(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_AES256_CMAC(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_AES256_GCM(uint32_t P_verbose);

uint32_t HASH_tests(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hash_nonValidHashAlgoDetection_test(uint32_t P_verbose);
uint32_t CSE_ext_valTest_hash_digestGeneration_invalidMemRange_test(uint32_t P_verbose);


uint32_t test_CSE_extendedDriver_HMAC_macGeneration(uint32_t P_verbose, uint32_t P_reducedTestVectorSet);
uint32_t test_CSE_extendedDriver_HMAC_macVerification(uint32_t P_verbose, uint32_t P_reducedTestVectorSet);

uint32_t test_CSE_extendedDriver_HASH(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_ECC_ECDSA(uint32_t P_verbose, uint32_t P_curveIDToTest);
uint32_t test_CSE_extendedDriver_ECC_P256_ECIES(uint32_t P_verbose);

uint32_t test_CSE_extendedDriver_RSA1024_PKCS1v15SignVerify(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_RSA2048_PKCS1v15SignVerify(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_RSA3072_PKCS1v15SignVerify(uint32_t P_verbose);
uint32_t test_CSE_extendedDriver_RSA2048_PKCS1V15Decryption (uint32_t P_verbose);

/* Elementary all in one Functions for extended Driver Tests functions */
uint32_t aes256_ecb_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                               struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                               struct ByteArrayDescriptor_stt *P_pEncryptedMessage,
                               uint32_t blockCount,
                               uint32_t P_verbose);

uint32_t aes256_ecb_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                               struct ByteArrayDescriptor_stt * P_pMessageToDecrypt,
                               struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                               uint32_t blockCount,
                               uint32_t P_verbose);

uint32_t aes256_cbc_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                               struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                               struct ByteArrayDescriptor_stt *P_pEncryptedMessage,
                               struct ByteArrayDescriptor_stt *P_pIv,
                               uint32_t blockCount,
                               uint32_t P_verbose);

uint32_t aes256_cbc_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                               struct ByteArrayDescriptor_stt * P_pMessageToDecrypt,
                               struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                               struct ByteArrayDescriptor_stt *P_pIv,
                               uint32_t blockCount,
                               uint32_t P_verbose);

uint32_t aes256_ccm_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              const struct AES_CCM_headerData_stt * p_pHeaderData,
                              const uint32_t P_tagSize,
                              const struct AES_CCM_msgData_stt * P_pMsg,
                              struct AES_CCM_msgData_stt * P_pCipheredMsg,
                              uint32_t P_verbose);

uint32_t aes256_ccm_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                              const struct AES_CCM_headerData_stt * p_pHeaderData,
                              const uint32_t P_tagSize,
                              const struct AES_CCM_msgData_stt * P_pCipheredMsg,
                              struct AES_CCM_deciphMsgData_stt * P_pPayloadMsg,
                              uint32_t P_verbose);

uint32_t aes256_cmac_tagGeneration(struct AES256secretKeyArray_stt * P_pSecretKey,
                                  const uint32_t P_tagSize,
                                  const struct AES_CMAC_msg_stt * P_pMsg,
                                  struct AES_CMAC_tag_stt * P_pCmacTag,
                                  uint32_t P_verbose);

uint32_t aes256_cmac_tagVerification(struct AES256secretKeyArray_stt * P_pSecretKey,
                                    const uint32_t P_tagSize,
                                    const struct AES_CMAC_msg_stt * P_pMsg,
                                    const struct AES_CMAC_tag_stt * P_pCmacTag,
                                    uint32_t P_verbose);

uint32_t aes256_gcm_authenticatedEncryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                                           const struct AES_GCM_headerData_stt * p_pHeaderData,
                                           const uint32_t P_tagSize,
                                           const struct AES_GCM_msgData_stt * P_pMsg,
                                           struct AES_GCM_authCiphMsg_stt * P_pAuthCipheredMsg,
                                           uint32_t P_verbose);

uint32_t aes256_gcm_authenticatedDecryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                                           const struct AES_GCM_headerData_stt * p_pHeaderData,
                                           const uint32_t P_tagSize,
                                           const struct AES_GCM_authCiphMsg_stt * P_pAuthCipheredMsg,
                                           struct AES_GCM_deciphMsgData_stt * P_pDeCipheredMsg,
                                           uint32_t P_verbose);

int32_t ECC_signGenerate_ECDSA(struct ECCprivKeyByteArray_stt * P_pPrivKey,
                               const uint8_t * P_pInputMessage, int32_t P_MessageSize,
                               uint32_t hash_type,
                               struct ECDSA_Signature_stt * P_pECDSA_Signature,
                               uint32_t P_verbose);

int32_t ECC_signVerify_ECDSA(struct ECCpubKeyByteArray_stt * P_pPubKey,
                             const uint8_t * P_pInputMessage, int32_t P_MessageSize,
                             uint32_t hash_type,
                             struct ECDSA_Signature_stt * P_pECDSA_Signature,
                             uint32_t P_verbose);

int32_t RSA_Sign_PKCS1v15(struct RSAprivKey_stt * P_pPrivKey,
                          const uint8_t * P_pInputMessage, int32_t P_MessageSize,
                          uint32_t hash_type,
                          uint8_t *P_pOutput, uint32_t P_verbose);

int32_t RSA_Verify_PKCS1v15(struct RSApubKey_stt *P_pPubKey,
                            const uint8_t *P_pInputMessage,
                            int32_t P_MessageSize,
                            uint32_t hash_type,
                            const uint8_t *P_pSignature, uint32_t P_verbose);

int32_t RSA_Decrypt_PKCS1v15(struct RSAprivKey_stt * P_pPrivKey,
                             const uint8_t * P_pInputMessage,
                             uint32_t P_InputSize,
                             uint8_t *P_pOutput,
                             uint32_t *P_OutputSize, uint32_t P_verbose);

int32_t ECC_ecies_encryption(struct ECCpubKeyByteArray_stt * P_pPubKey,
                             struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                             struct ECIES_sharedData_stt * P_pEciesSharedData,
                             struct ECIES_encMsg_stt *P_pEncryptedMessage,
                             uint32_t P_verbose);

int32_t ECC_ecies_decryption(struct ECCprivKeyByteArray_stt * P_pPrivKey,
                             struct ECIES_encMsg_stt * P_pMessageToDecrypt,
                             struct ECIES_sharedData_stt * P_pEciesSharedData,
                             struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                             uint32_t P_verbose);
#endif /* CSE_DRIVER_TEST_CSE_EXT_EXTENDEDDRIVERTESTSFUNCLIST_H_ */
