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
 * @file    CSE_ext_ECC_ECIES_TV.h
 * @brief   CSE ECC ECIES test vector header file
  */
#ifndef _CSE_EXT_ECC_ECIES_TV_H_
#define _CSE_EXT_ECC_ECIES_TV_H_

/*
 * @addtogroup SHE-ext_driver
 * @{
 */

#include "cse_typedefs.h"
#include"config.h"
#include "CSE_ext_ECC_TV_consts.h"

#define NB_OF_ECIES_ENC_TV 10
#define NB_OF_ECIES_DEC_TV 10
#define NB_OF_ECIES_ENC_DEC_TV 10

#define C_MAX_SHARED_DATA_LEN 16
#define C_MAX_KEY_DATA_LEN 128
#define C_MAX_TAG_SIZE 16
#define C_MAX_MESSAGE_LEN 256

typedef struct
{
    const uint8_t message[C_MAX_MESSAGE_LEN];                   /* Message to encrypt */
    const uint32_t messageLen;                                  /* Message Length in byte */
    const uint8_t recipientPublicXKey[C_MAX_PUB_KEY_SIZE];      /* Decryption (Recipient) Public Key X */
    const uint8_t recipientPublicYKey[C_MAX_PUB_KEY_SIZE];      /* Decryption (Recipient) Public Key Y */
    const uint8_t senderFmrPrivateKey[C_MAX_PRIV_KEY_SIZE];     /* Encryption (Sender) Ephemeral Private Key */
    const uint8_t sharedData1[C_MAX_SHARED_DATA_LEN];           /* Shared Data 1 of length sharedDataLen */
    const uint32_t sharedData1Len;                              /* Length in bits of the Shared Data 1 */
    const uint8_t sharedData2[C_MAX_SHARED_DATA_LEN];           /* Shared Data 2 of length sharedDataLen */
    const uint32_t sharedData2Len;                              /* Length in bits of the Shared Data 2 */
    const uint8_t expectedSenderFmrPublicXKey[C_MAX_PUB_KEY_SIZE]; /* Expected Encryption (Sender) Ephemeral Public Key X */
    const uint8_t expectedSenderFmrPublicYKey[C_MAX_PUB_KEY_SIZE]; /* Expected Encryption (Sender) Ephemeral Public Key Y */
    const uint8_t expectedEncryptedMessage[C_MAX_MESSAGE_LEN];  /* Expected Encrypted Message */
    const uint8_t expectedTag[C_MAX_TAG_SIZE];                  /* Expected Tag over encrypted message */
} ecies_encrypt_testVect_stt;

typedef struct
{
    const uint8_t recipientPrivateKey[C_MAX_PRIV_KEY_SIZE];     /* Decryption (Recipient) Private Key */
    const uint8_t senderFmrPublicXKey[C_MAX_PUB_KEY_SIZE];      /* Encryption (Sender) Ephemeral Public Key X */
    const uint8_t senderFmrPublicYKey[C_MAX_PUB_KEY_SIZE];      /* Encryption (Sender) Ephemeral Public Key Y */
    const uint8_t encryptedMessage[C_MAX_MESSAGE_LEN];          /* Encrypted Message */
    const uint32_t messageLen;                                  /* Encrypted Message Length in byte */
    const uint8_t tag[C_MAX_TAG_SIZE];                          /* Tag over encrypted message to be verified */
    const uint32_t tagLen;                                      /* Tag Length in byte */
    const uint8_t sharedData1[C_MAX_SHARED_DATA_LEN];           /* Shared Data 1 of length sharedDataLen */
    const uint32_t sharedData1Len;                              /* Length in bits of the Shared Data 1 */
    const uint8_t sharedData2[C_MAX_SHARED_DATA_LEN];           /* Shared Data 2 of length sharedDataLen */
    const uint32_t sharedData2Len;                              /* Length in bits of the Shared Data 2 */
    const uint8_t ExpectedMessage[C_MAX_MESSAGE_LEN];           /* ClearTest Expected message */
} ecies_decrypt_testVect_stt;

typedef struct
{
    const uint8_t message[C_MAX_MESSAGE_LEN];                   /* Message to encrypt */
    const uint32_t messageLen;                                  /* Message Length in byte */
    const uint8_t recipientPublicXKey[C_MAX_PUB_KEY_SIZE];      /* Decryption (Recipient) Public Key X */
    const uint8_t recipientPublicYKey[C_MAX_PUB_KEY_SIZE];      /* Decryption (Recipient) Public Key Y */
    const uint8_t recipientPrivateKey[C_MAX_PRIV_KEY_SIZE];     /* Decryption (Recipient) Private Key */
    const uint8_t sharedData1[C_MAX_SHARED_DATA_LEN];           /* Shared Data 1 of length sharedDataLen */
    const uint32_t sharedData1Len;                              /* Length in bits of the Shared Data 1 */
    const uint8_t sharedData2[C_MAX_SHARED_DATA_LEN];           /* Shared Data 2 of length sharedDataLen */
    const uint32_t sharedData2Len;                              /* Length in bits of the Shared Data 2 */
} ecies_encryptDecrypt_testVect_stt;

#endif /* _CSE_EXT_ECC_ECIES_TV_H_ */

/**
 * @}
 */
