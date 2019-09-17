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
 * @file    CSE_Key_update.h
 * @brief   CSE Key update support module - header.
 * @details Set of functions used to compute required material to update keys.
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#ifndef _CSE_KEY_update_H_
#define _CSE_KEY_update_H_

#include "cse_typedefs.h"

/*
 * @note Module can be mapped either on sw crypto lib or CSE HW engine. It is also usable on little or big endian platforms
 *
 */

/*================================================================================================*/
/**
 * @brief          Key Derivation Function
 * @details        Loads a key value to be used when selecting index 0xE (RAM_KEY)
 *
 * @param[in]      UserKey		pointer to value to derive from
 * @param[in]      Key_Constant	pointer to constant to use for derivation
 * @param[out]     KeyOut 		pointer to derived Key
 *
 * @return         Error code
 * @retval 0  	  When key was loaded properly
 * @retval xx      In case of error
 *
 * @note           The function can use either the CSE HW or the sw crypto library to perform the AES operations
 * 				  hidden thanks to an abstraction function AES_ECB_Encrypt
 */
extern int KDF(uint8_t* UserKey, uint8_t* Key_Constant, uint8_t* KeyOut);

/*================================================================================================*/
/**
 * @brief          Generate the messages required to load a Key in the NVM using CSE_LOAD_KEY
 * @details        Computes the formatted messages and associated integrity and authenticity MACs
 *
 * @param[in]      new_key	pointer to cleartext key value (128bits)
 * @param[in]      k_authid	pointer to cleartext authorization key value (128bits)
 * @param[in]      uid		pointer to chip unique ID (120bits)
 * @param[in]      id     	Key index where the key will have to be stored in NVM
 * @param[in]      authid 	Authentication Key index to be used for authentication check when loading
 * @param[in]      cid 		Counter value
 * @param[in]      fid     	Security flags to store with the Key
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @param[out]     m1		pointer to message1
 * @param[out]     m2		pointer to message2
 * @param[out]     m3		pointer to message3
 * @param[out]     m4		pointer to message4
 * @param[out]     m5		pointer to message5
 *
 * @return         Error code
 * @retval 0  	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned ones
 *
 * @note           This function is not intended to be used on-board, but rather in an offline program
 * 				  It requires the knowledge of the authentication key
 * 				  M4 and M5 are not required to load a key, but they can be used as reference to check key_load succeeded
 */
extern uint32_t generate_M1_to_M5(uint32_t* new_key, uint32_t* k_authid,
                                  uint32_t* uid, uint8_t id, uint8_t authid,
                                  uint32_t cid, uint8_t fid,
                                  uint32_t ext_key_set,
                                  uint32_t* m1, uint32_t* m2, uint32_t* m3, 
                                  uint32_t* m4, uint32_t* m5);

#endif /* _CSE_KEY_update_H_ */

/** @} */
