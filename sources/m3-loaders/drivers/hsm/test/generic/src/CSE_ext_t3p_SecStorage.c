/**
 * @file    CSE_ext_t3p_SecStorage.c
 * @brief   telemaco3P remote key storage support
 * @details
 *
 *
 * @addtogroup CSE_support
 * @{
 */

#include "string.h" /* for memcpy & memcmp */
#include "cse_typedefs.h"
#include "CSE_Constants.h"
#include "CSE_ext_AES256.h"
#include "err_codes.h"

extern uint32_t aes256_cbc_decryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                               struct ByteArrayDescriptor_stt * P_pMessageToEncrypt,
                               struct ByteArrayDescriptor_stt *P_pEncryptedMessage,
                               struct ByteArrayDescriptor_stt *P_pIv,
                               uint32_t blockCount,
                               uint32_t P_verbose);

extern uint32_t aes256_cbc_encryption(struct AES256secretKeyArray_stt * P_pSecretKey,
                               struct ByteArrayDescriptor_stt * P_pMessageToDecrypt,
                               struct ByteArrayDescriptor_stt *P_pDecryptedMessage,
                               struct ByteArrayDescriptor_stt *P_pIv,
                               uint32_t blockCount,
                               uint32_t P_verbose);

extern uint32_t aes256_cmac_tagGeneration(struct AES256secretKeyArray_stt * P_pSecretKey,
                                  const uint32_t P_tagSize,
                                  const struct AES_CMAC_msg_stt * P_pMsg,
                                  struct AES_CMAC_tag_stt * P_pCmacTag,
                                  uint32_t P_verbose);


/**
 * @brief         performs the CBC encryption of the provided block with provided AES256 key & IV
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 *
 */
uint32_t cryCrypt(uint8_t *dest, const uint8_t *src, uint32_t size, uint8_t* SSKey_cryp, uint8_t* piv )
{
    uint32_t ret = AES_ERR_BAD_PARAMETER;
    struct AES256secretKeyArray_stt aes256SecretKey;
    struct ByteArrayDescriptor_stt messageToEncrypt;
    struct ByteArrayDescriptor_stt cipherMessage;
    struct ByteArrayDescriptor_stt iv;

    if((size % 16) != 0)
    {
        /* size is not correct, must be a multiple of 16 */
    }
    else
    {
        aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_cryp;
        aes256SecretKey.mSecretKeySize = 32;

        /* Initialize Initialization Vector */
        iv.address = piv;
        iv.byteSize = 16;

        messageToEncrypt.address = (uint8_t*)src;
        messageToEncrypt.byteSize = size;

        cipherMessage.address = dest;
        cipherMessage.byteSize = size;

        ret = aes256_cbc_encryption( &aes256SecretKey,
                                     &messageToEncrypt,
                                     &cipherMessage,
                                     &iv,
                                     size / 16,
                                     0
                                     );
    }
    return (ret);
}

/**
 * @brief         performs the CBC decryption of the provided block with provided AES256 key & IV
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 *
 */
uint32_t cryDecrypt(uint8_t *dest, const uint8_t *src, uint32_t size, uint8_t* SSKey_cryp, uint8_t* piv )
{
    uint32_t ret = AES_ERR_BAD_PARAMETER;
    struct AES256secretKeyArray_stt aes256SecretKey;
    struct ByteArrayDescriptor_stt messageToDecrypt;
    struct ByteArrayDescriptor_stt cleartextMessage;
    struct ByteArrayDescriptor_stt iv;

    if((size % 16) != 0)
    {
        /* size is not correct, must be a multiple of 16 */
    }
    else
    {
        aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_cryp;
        aes256SecretKey.mSecretKeySize = 32;

        /* Initialize Initialization Vector */
        iv.address = piv;
        iv.byteSize = 16;

        messageToDecrypt.address = (uint8_t*)src;
        messageToDecrypt.byteSize = size;

        cleartextMessage.address = dest;
        cleartextMessage.byteSize = size;

        ret = aes256_cbc_decryption( &aes256SecretKey,
                                     &messageToDecrypt,
                                     &cleartextMessage,
                                     &iv,
                                     size / 16,
                                     0
                                     );
    }
    return (ret);
}


/**
 * @brief         performs the external key image tag generation of the provided block with provided AES256 key
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]     size          size of the text (must be a multiple of 16)
 * @param[in]     src           text to be 'signed' (it's a tag)
 * @param[out]    signature     the signature buffer  (must be 32bytes long)
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 *
 */
uint32_t crySign(const uint8_t *src, uint32_t size, uint8_t *signature, uint8_t* SSKey_auth )
{
    uint32_t ret = AES_ERR_BAD_PARAMETER;
    struct AES256secretKeyArray_stt aes256SecretKey;
    struct AES_CMAC_msg_stt messageToMAC;
    struct AES_CMAC_tag_stt tag;

    if((size % 16) != 0)
    {
        /* size is not correct, must be a multiple of 16 */
    }
    else
    {
        aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_auth;
        aes256SecretKey.mSecretKeySize = 32;

        messageToMAC.pAddress = (uint8_t*)src;
        messageToMAC.messageByteSize = size;

        tag.pAddress = signature;

        ret = aes256_cmac_tagGeneration( &aes256SecretKey,
                                      16,
                                      &messageToMAC,
                                      &tag,
                                      0);
        memcpy( signature + 16, signature, 16 );
    }
    return (ret);
}

/**
 * @brief         performs the external key image tag generation of the provided block with provided AES256 key
 * @details       uses the HSM AES256 RAM key to perform the operation
 *
 * @param[in]     size          size of the text (must be a multiple of 16)
 * @param[in]     src           text to be 'signed' (it's a tag)
 * @param[out]    signature     the signature buffer  (must be 32bytes long)
 * @return        Error code
 * @retval        CSE_NO_ERR  if successfull
 * @retval        AES_ERR_BAD_PARAMETER if issue with params
 * @retval        CSE_GENERAL_ERR if signature does not match
 *
 */
uint32_t cryVerify(const uint8_t *src, uint32_t size, uint8_t *signature, uint8_t* SSKey_auth )
{
    uint32_t ret = AES_ERR_BAD_PARAMETER;
    struct AES256secretKeyArray_stt aes256SecretKey;
    struct AES_CMAC_msg_stt messageToMAC;
    struct AES_CMAC_tag_stt tag;

    uint8_t computed_tag[32];

    if((size % 16) != 0)
    {
        /* size is not correct, must be a multiple of 16 */
    }
    else
    {
        aes256SecretKey.pmSecretKey = (uint8_t*)SSKey_auth;
        aes256SecretKey.mSecretKeySize = 32;

        messageToMAC.pAddress = (uint8_t*)src;
        messageToMAC.messageByteSize = size;

        tag.pAddress = computed_tag;

        ret = aes256_cmac_tagGeneration( &aes256SecretKey,
                                      16,
                                      &messageToMAC,
                                      &tag,
                                      0);
        memcpy( computed_tag + 16, computed_tag, 16 );

        /* compare */
        if( memcmp(computed_tag, signature, 32 ) == 0)
        {
            ret = CSE_NO_ERR;
        }
        else
        {
            ret = CSE_GENERAL_ERR;
        }
    }
    return (ret);
}


/**
 * @}
 */

