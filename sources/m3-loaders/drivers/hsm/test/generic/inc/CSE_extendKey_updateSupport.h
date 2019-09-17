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
 * @file    CSE_extendKey_updateSupport.h
 * @brief   CSE Asymmetric Key update support module - header.
 * @details Set of functions used to compute required material to update asymmetric keys.
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#ifndef _CSE_ASYSM_KEY_updateSupport_H_
#define _CSE_ASYSM_KEY_updateSupport_H_

#include "cse_typedefs.h"
/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern const uint8_t KEY_UPDATE_ENC_C[16];
extern const uint8_t KEY_UPDATE_MAC_C[16];
extern const uint8_t ASYMKEY_UPDATE_ENC_C[16];
extern const uint8_t KEY_UPDATE_ENC_C_EXT[16];
extern const uint8_t KEY_UPDATE_MAC_C_EXT[16];
extern const uint8_t DEBUG_KEY_C[16];
extern const uint8_t IV0_Key[16];

extern vuint32_t K1[4];
extern char K2[16];
extern char K3[16];
extern char K4[16];

/*
 * @note Module can be mapped either on sw crypto lib or CSE HW engine. It is also usable on little or big endian platforms
 *
 */

/*================================================================================================*/
/**
 * @brief          Key Derivation Function
 * @details        Loads a key value to be used when selecting index 0xE (RAM_KEY)
 *
 * @param[in]      UserKey        pointer to value to derive from
 * @param[in]      Key_Constant    pointer to constant to use for derivation
 * @param[out]     KeyOut         pointer to derived Key
 *
 * @return         Error code
 * @retval 0        When key was loaded properly
 * @retval xx      In case of error
 *
 * @note           The function can use either the CSE HW or the sw crypto library to perform the AES operations
 *                   hidden thanks to an abstraction function AES_ECB_Encrypt
 */
extern int KDF(uint8_t* UserKey, uint8_t* Key_Constant, uint8_t* KeyOut);

/*================================================================================================*/
/**
 * @brief      Generates the messages required to load aa asymmetric  Key in RAM or NVM location using CSE_LOAD_KEY
 * @details    Computes M1 to M5 formatted messages and associated integrity and authenticity MACs
 *
 * @param[in]    P_pKey1       Pointer to asymmetric key field 1 (Modulus for RSA, Public Key x for ECC)
 * @param[in]    P_key1Size    Size in byte of asymmetric key field 1
 * @param[in]    P_pKey2       Pointer to asymmetric key field 2 (Public Key for RSA, Public Key y for ECC)
 * @param[in]    P_key2Size    Size in byte of asymmetric key field 2
 * @param[in]    P_pKey3       Pointer to asymmetric key field 3 (Private Key for RSA, Private Key for ECC)
 * @param[in]    P_key3Size    Size in byte of asymmetric key field 1

  * @param[in]    P_pKeyAuthId Pointer to cleartext authorization key value (128bits)
 * @param[in]    P_pUid        Pointer to chip unique ID (120bits)
 * @param[in]    P_keyIndex    Key index where the key will have to be stored (RAM or NVM)
 * @param[in]    P_authId      Authentication Key index to be used for authentication check when loading
 * @param[in]    P_cid         Counter value
 * @param[in]    P_fid         Security flags to store with the Key
  *
 * @param[out]   P_pM1         Pointer to message1
 * @param[out]   P_pM2         Pointer to message2
 * @param[out]   P_pM3         Pointer to message3
 * @param[out]   P_pM4         Pointer to message4
 * @param[out]   P_pM5         Pointer to message5
 * *
 * @return       Error code
 * @retval 0     When key was loaded properly
 * @retval 1..21 In case of error - the error code values are the CSE returned ones
 *
 * @note         This function is not intended to be used on-board, but rather in an offline program
 *               It requires the knowledge of the authentication key
 *               M4 and M5 are not required to load a key, but they can be used as reference to check key_load succeeded
 */
uint32_t CSE_extendKeyGenerate_M1_to_M5(uint32_t * P_pKey1,
                                        uint32_t P_key1Size,
                                        uint32_t * P_pKey2,
                                        uint32_t P_key2Size,
                                        uint32_t * P_pKey3,
                                        uint32_t P_key3Size,

                                        uint32_t* P_pKeyAuthId, uint32_t* P_pUid,
                                        uint32_t P_keyIndex, uint32_t P_authId,
                                        uint32_t P_cid, uint32_t P_fid,
                                        uint32_t* P_pM1, uint32_t* P_pM2, uint32_t* P_pM3,
                                        uint32_t* P_pM4, uint32_t* P_pM5);

#endif /* _CSE_ASYSM_KEY_updateSupport_H_ */

/** @} */
