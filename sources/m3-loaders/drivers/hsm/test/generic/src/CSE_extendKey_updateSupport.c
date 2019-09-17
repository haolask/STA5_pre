/*
    SPC5-CRYPTO - Copyright (C) 2014 STMicroelectronics

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
 * @file    CSE_extendKey_updateSupport.c
 * @brief   CSE Asymmetric Key update support module.
 * @details Set of functions used to compute required material to update asymmetric keys.
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#include "cse_typedefs.h"
#include "crypto_al.h"
#include "err_codes.h"
#include "CSE_extendKey_updateSupport.h"

#include "serialprintf.h"

#undef DEBUG_PRINT
//#define DEBUG_PRINT

/*================================================================================================*/
/**
 * @brief      Copy EC Public Key coordinates or private key into M2 buffer
  *
 * @param[in]    P_pKey        Pointer to asymmetric key field 1 to copy
 * @param[in]    P_keySize     Size in byte of asymmetric key field
 * @param[out]   P_pM2Input    Pointer to message 2 internal buffer
 * @param[out]   P_pM2WordSize Pointer to location holding length in word of M2 message
 */
static void CSE_extendKey_CopyKeyToM2Buf(uint32_t * P_pKey,
                                         uint32_t P_keySize,
                                         uint32_t * P_pM2Input,
                                         uint32_t * P_pM2WordSize)
{
    uint32_t i = 0U;
    uint32_t nbByteLeft = 0U;

    /* Key exist: copy it */
    for (i = 0U; i < P_keySize / 4U; i++)
    {
        P_pM2Input[*P_pM2WordSize + i] = P_pKey[i];
    }

    /* Is there some bytes remaining */
    nbByteLeft = P_keySize - (i * 4U);
    if (0U != nbByteLeft)
    {
        uint32_t j = 0U;
        uint8_t* src;
        uint8_t* dst;

        src = (uint8_t*)(&P_pKey[(i)]);
        dst = (uint8_t*)(&P_pM2Input[*P_pM2WordSize + i]);

        for (j = 0; j < nbByteLeft; j++)
        {
            dst[j] = src[j];
        }
        for (j = nbByteLeft; j < 4; j++)
        {
            dst[j] = 0U;
        }

        *P_pM2WordSize += i + 1U;
    }
    else
    {
        *P_pM2WordSize += i;
    }
} /* End of CSE_extendKey_CopyKeyToM2Buf */

/*
 * @note Module can be mapped either on SW crypto lib or CSE HW engine. It is also usable on little or big endian platforms
 *
 */

/*================================================================================================*/
/**
 * @brief      Generates the messages required to load aa asymmetric  Key in RAM or NVM location using CSE_LOAD_KEY
 * @details    Computes M1 to M5 formatted messages and associated integrity and authenticity MACs
 *
 * @param[in]    P_pKey1          Pointer to asymmetric key field 1 (Modulus for RSA, Public Key x for ECC)
 * @param[in]    P_key1Size       Size in byte of asymmetric key field 1
 * @param[in]    P_pKey2          Pointer to asymmetric key field 2 (Public Key for RSA, Public Key y for ECC)
 * @param[in]    P_key2Size       Size in byte of asymmetric key field 2
 * @param[in]    P_pKey3          Pointer to asymmetric key field 3 (Private Key for RSA, Private Key for ECC)
 * @param[in]    P_key3Size       Size in byte of asymmetric key field 1

 * @param[in]    P_pKeyAuthId     Pointer to cleartext authorization key value (128bits)
 * @param[in]    P_pUid           Pointer to chip unique ID (120bits)
 * @param[in]    P_keyIndex       Key index where the key will have to be stored (RAM or NVM)
 * @param[in]    P_authId         Authentication Key index to be used for authentication check when loading
 * @param[in]    P_cid            Counter value
 * @param[in]    P_fid            Security flags to store with the Key
 *
 * @param[out]   P_pM1            Pointer to message1
 * @param[out]   P_pM2            Pointer to message2
 * @param[out]   P_pM3            Pointer to message3
 * @param[out]   P_pM4            Pointer to message4
 * @param[out]   P_pM5            Pointer to message5
 *
 * @return       Error code
 * @retval 0     When key was loaded properly
 * @retval 1..21 In case of error - the error code values are the CSE returned ones
 *
 * @note         This function is not intended to be used on-board, but rather in an offline program
 *                  It requires the knowledge of the authentication key
 *                  M4 and M5 are not required to load a key, but they can be used as reference to check if key_load succeeded
 */
uint32_t CSE_extendKeyGenerate_M1_to_M5(uint32_t * P_pKey1,
                                        uint32_t P_key1Size,
                                        uint32_t * P_pKey2,
                                        uint32_t P_key2Size,
                                        uint32_t * P_pKey3,
                                        uint32_t P_key3Size,

                                        uint32_t * P_pKeyAuthId, uint32_t* P_pUid,
                                        uint32_t P_keyIndex, uint32_t P_authId,
                                        uint32_t P_cid, uint32_t P_fid,
                                        uint32_t * P_pM1, uint32_t* P_pM2, uint32_t* P_pM3,
                                        uint32_t * P_pM4, uint32_t* P_pM5)
{

#define C_M1_BUFFER_WORD_SIZE 8
#define C_M2_PREAMBULE_WORD_SIZE 4
#define C_KEYWORDSIZE (64+32)                     /* To support up to RSA 3072 */
#define C_M2_LENGTH_COUNTER 1
#define C_M2_BUFFER_WORD_SIZE C_M2_PREAMBULE_WORD_SIZE + (3U * C_KEYWORDSIZE) + 1U    /* 1 word more to be multiple of AES block length for P521 */
#define C_M3_BUFFER_WORD_SIZE C_M1_BUFFER_WORD_SIZE + C_M2_BUFFER_WORD_SIZE

    uint32_t initial_value_cbc[4] = {0U, 0U, 0U, 0U};
    uint32_t m2_input[C_M2_BUFFER_WORD_SIZE] = {0U};
    uint32_t m2WordSize = 0U;    /* Index to write key values in buffer and size of M2 */

    uint32_t m3_input[C_M3_BUFFER_WORD_SIZE];
    uint32_t m4_star_input[4];
    uint32_t m4_star[4];
    uint8_t* m4_star_input_byte_array;

    uint8_t* m1_byte_array;
    uint8_t* m2_input_byte_array;
    uint8_t* m4_byte_array;
    uint8_t* uid_byte_array;
    int ret = 0U;

    m1_byte_array = (uint8_t*)P_pM1;
    m2_input_byte_array = (uint8_t*)m2_input;
    m4_byte_array = (uint8_t*)P_pM4;
    m4_star_input_byte_array = (uint8_t*)m4_star_input;
    uid_byte_array = (uint8_t*)P_pUid;

    /* Derive K1 and K2 keys */
    KDF((uint8_t *)P_pKeyAuthId, (uint8_t *)KEY_UPDATE_ENC_C, (uint8_t *)K1);
    KDF((uint8_t *)P_pKeyAuthId, (uint8_t *)KEY_UPDATE_MAC_C, (uint8_t *)K2);

#ifdef DEBUG_PRINT
    display_buf( "Auth KeyK :", (uint8_t*)P_pKeyAuthId, 16);
    display_buf( "K1 :       ", (uint8_t*)K1, 16);
    display_buf( "K2 :       ", (uint8_t*)K2, 16);
#endif

    /* uid is 120 bits */
    /* Counter (cid) is 28 bits */
    /* id is 4 bits */
    /* authid is 4 bits */

    /* Fill M1 */
    P_pM1[0] = P_pUid[0];
    P_pM1[1] = P_pUid[1];
    P_pM1[2] = P_pUid[2];

    m1_byte_array[12] = uid_byte_array[12];
    m1_byte_array[13] = uid_byte_array[13];
    m1_byte_array[14] = uid_byte_array[14];
    m1_byte_array[15] = ((P_keyIndex & 0x0F) << 4) + (P_authId & 0x0F);

    P_pM1[4] = 0x0U;
    P_pM1[5] = 0x0U;
    P_pM1[6] = 0x0U;
    /* P_pM1[7] is set after have computed M2 size - see below */

    /* Fill M2 cleartext */
    /* Counter on 28 bits but stored on 32 bits; next field FID aligned on 32 bits: 28 bits + 0b0000 */
    m2_input_byte_array[0] = (P_cid & 0x0FF00000) >> 20;
    m2_input_byte_array[1] = (P_cid & 0x000FF000) >> 12;
    m2_input_byte_array[2] = (P_cid & 0x00000FF0) >> 4;
    m2_input_byte_array[3] = ((P_cid & 0x0000000F) << 4);

    /* Next word: FID on 16 bits | Modulus (RSA) or Public key x (ECC) size on 16 bits */
    m2_input_byte_array[4] = (P_fid & 0x0000FF00) >> 8;
    m2_input_byte_array[5] = (P_fid & 0x000000FF);
    m2_input_byte_array[6] = (P_key1Size & 0x0000FF00) >> 8;
    m2_input_byte_array[7] = (P_key1Size & 0x000000FF);


    /* Next word: Public Key (RSA) or Public Key y (ECC) size | Private Key size (RSA and ECC) */
    m2_input_byte_array[8] =  (P_key2Size & 0x0000FF00) >> 8;
    m2_input_byte_array[9] =  (P_key2Size & 0x000000FF);
    m2_input_byte_array[10] = (P_key3Size & 0x0000FF00) >> 8;
    m2_input_byte_array[11] = (P_key3Size & 0x000000FF);

    /* Next Word: 32-bit at zero to have M2 size multiple of 16 bytes */
    m2_input[3] = 0x0;

    m2WordSize = C_M2_PREAMBULE_WORD_SIZE;    /* Already 4 words written in M2 */

    /* Next field: Modulus (RSA) or Public Key x (ECC) if size if different from 0
     * Key field must be coded on entire number of 32-bit words */
    if (P_key1Size != 0)
    {
        CSE_extendKey_CopyKeyToM2Buf(P_pKey1, P_key1Size,
                                     m2_input, &m2WordSize);
    }

    /* Next field: Public Key (RSA) or Public Key y (ECC) if size if different from 0
     * Key field must be coded on entire number of 32-bit words */
    if (P_key2Size != 0)
    {
        CSE_extendKey_CopyKeyToM2Buf(P_pKey2, P_key2Size,
                                     m2_input, &m2WordSize);
    }

    /* Next field: Private Key (RSA) or Private Key (ECC) if size if different from 0
     * Key field must be coded on entire number of 32-bit words */
    if (P_key3Size != 0)
    {
        CSE_extendKey_CopyKeyToM2Buf(P_pKey3, P_key3Size,
                                     m2_input, &m2WordSize);
    }

    /* Fix buffer size to be multiple of AES block length for next AES encrypt */
    if (0U != (m2WordSize % 4U))
    {
        m2WordSize = m2WordSize + ((4U - (m2WordSize % 4U)));
    }

    /* Complete M1 message with length in bytes of M2 message - write it in endianess independent way */
    m1_byte_array[28] = ((m2WordSize * 4U) & 0xFF000000) >> 24;
    m1_byte_array[29] = ((m2WordSize * 4U) & 0x00FF0000) >> 16;
    m1_byte_array[30] = ((m2WordSize * 4U) & 0x0000FF00) >> 8;
    m1_byte_array[31] = ((m2WordSize * 4U) & 0x000000FF);

#ifdef DEBUG_PRINT
    display_buf( "m1_input buffer : ", (uint8_t*)P_pM1, C_M1_BUFFER_WORD_SIZE * 4);
#endif

#ifdef DEBUG_PRINT
    display_buf( "m2_input buffer : ", (uint8_t*)m2_input, m2WordSize * 4U);
#endif

    /* Encrypt M2, AES CBC mode, Key = K1, IV = 0 */
    ret = AES_CBC_Encrypt((uint8_t *)m2_input,
                          m2WordSize * 4U,
                          (const uint8_t *)K1,
                          16U,
                          (const uint8_t *)initial_value_cbc,
                          (uint8_t *)P_pM2);

#ifdef DEBUG_PRINT
    display_buf( "m2 res : ", (uint8_t*)P_pM2, m2WordSize * 4U);
#endif

/* Fill M3 CMAC payload */
    m3_input[0] = P_pM1[0];
    m3_input[1] = P_pM1[1];
    m3_input[2] = P_pM1[2];
    m3_input[3] = P_pM1[3];
    m3_input[4] = P_pM1[4];
    m3_input[5] = P_pM1[5];
    m3_input[6] = P_pM1[6];
    m3_input[7] = P_pM1[7];

    {
        uint32_t i = 0U;

        for (i = 0U; i < m2WordSize; i++)
        {
            m3_input[i + C_M1_BUFFER_WORD_SIZE] = P_pM2[i];
        }
    }

#ifdef DEBUG_PRINT
    display_buf("m3_input: ", (uint8_t*)m3_input, (C_M1_BUFFER_WORD_SIZE + m2WordSize) * 4U);
#endif

    /* Compute M3 = CMAC K2( M1 | M2) */
    ret = AES_CMAC_Encrypt((uint8_t *)m3_input, (C_M1_BUFFER_WORD_SIZE + m2WordSize) * 4U,
                           (uint8_t *)K2, 16U, 16U,
                           (uint8_t *)P_pM3);

#ifdef DEBUG_PRINT
    display_buf("CMAC (M3): ", (uint8_t*)P_pM3, 16);
#endif

    /* Generate CMAC over Key Pair fields
     * Prepare buffer with all keys
     * If key size is not multiple of AES block size add padding on the right
     */
    {
        unsigned char K5[16];
        unsigned char key[16];
        uint32_t keyBuf[3 * 96] = {0};    /* To support RSA 3072 */
        uint32_t keyWordSize = 0U;

        /* Derive K5 from Authentication Key with new constant to generate key different from K2 */
        KDF((uint8_t *)P_pKeyAuthId, (uint8_t *)ASYMKEY_UPDATE_ENC_C, (uint8_t *)K5);

#ifdef DEBUG_PRINT
        display_buf( "K5 :       ", (uint8_t*)K5, 16);
#endif


        if (0U != P_key1Size)
        {
            CSE_extendKey_CopyKeyToM2Buf(P_pKey1, P_key1Size,
                                       keyBuf, &keyWordSize);
            if (0U != P_key1Size % 16)    /* Padding to number of AES Block size */
            {
                keyWordSize += (4 - keyWordSize % 4);
            }
        }

        /* Next field: Public Key (RSA) or Public Key y (ECC) if size if different from 0
         * Key field must be coded on entire number of 32-bit words */
        if (P_key2Size != 0)
        {
            CSE_extendKey_CopyKeyToM2Buf(P_pKey2, P_key2Size,
                                       keyBuf, &keyWordSize);
            if (0U != P_key3Size)
            {
                /* If there's a private key exponent after the pub key, then we padd it on a 16bytes boundary
                  otherwise wekeep it only 'rounded' to 4 bytes size */
                if (0U != P_key2Size % 16)
                {
                    keyWordSize += (4 - keyWordSize % 4);
                }
            }
        }

        /* Next field: Private Key (RSA) or Private Key (ECC) if size if different from 0
         * Key field must be coded on entire number of 32-bit words */
        if (0U != P_key3Size)
        {
            CSE_extendKey_CopyKeyToM2Buf(P_pKey3, P_key3Size,
                                       keyBuf, &keyWordSize);
        }

#ifdef DEBUG_PRINT
        printf("keyWordSize : %d\n",keyWordSize);
        display_buf( "keyBuf :      ", (uint8_t*)keyBuf, keyWordSize*4);
#endif

        /* Compute CMAC[K5] over Key Pair Fields if they exist */
        ret = AES_CMAC_Encrypt((uint8_t *)keyBuf, keyWordSize * 4,
                               (uint8_t *)K5, 16, 16,
                               (uint8_t *)key);

        /* Derive K3 and K4 from Result of CMAC generated over Key Pair field */
        KDF((uint8_t *)key, (uint8_t *)KEY_UPDATE_ENC_C, (uint8_t *)K3);
        KDF((uint8_t *)key, (uint8_t *)KEY_UPDATE_MAC_C, (uint8_t *)K4);

#ifdef DEBUG_PRINT
        display_buf( "Key :      ", (uint8_t*)key, 16);
        display_buf( "K3 :       ", (uint8_t*)K3, 16);
        display_buf( "K4 :       ", (uint8_t*)K4, 16);
#endif
    }

    /* Generate M4 */
    m4_star_input_byte_array[0] = (P_cid & 0x0FF00000) >> 20;
    m4_star_input_byte_array[1] = (P_cid & 0x000FF000) >> 12;
    m4_star_input_byte_array[2] = (P_cid & 0x00000FF0) >> 4;
    m4_star_input_byte_array[3] = ((P_cid & 0x0000000F) << 4) + 8U;
    m4_star_input[1] = 0;
    m4_star_input[2] = 0;
    m4_star_input[3] = 0;

#ifdef DEBUG_PRINT
    display_buf("m4_star_input: ", (uint8_t*)m4_star_input, 16);
#endif

    ret = AES_ECB_Encrypt((uint8_t *)m4_star_input, 16,
                          (const uint8_t *)K3, 16,
                          (uint8_t *)m4_star);

    P_pM4[0] = P_pUid[0];
    P_pM4[1] = P_pUid[1];
    P_pM4[2] = P_pUid[2];

    m4_byte_array[12] = uid_byte_array[12];
    m4_byte_array[13] = uid_byte_array[13];
    m4_byte_array[14] = uid_byte_array[14];
    m4_byte_array[15] = ((P_keyIndex & 0x0F) << 4) + (P_authId & 0x0F);

    P_pM4[4] = m4_star[0];
    P_pM4[5] = m4_star[1];
    P_pM4[6] = m4_star[2];
    P_pM4[7] = m4_star[3];

#ifdef DEBUG_PRINT
    display_buf("m4   : ", (uint8_t*)P_pM4, 32);
#endif

    /* Compute M5 = CMAC K4(M4) */
    ret = AES_CMAC_Encrypt((uint8_t *)P_pM4, 32, (uint8_t *)K4, 16, 16,
                           (uint8_t *)P_pM5);

#ifdef DEBUG_PRINT
    display_buf("m5   : ", (uint8_t*)P_pM5, 16);
#endif

    return(ret);
}

/** @} */
