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
 * @file    CSE_Debug.h
 * @brief   CSE Debug support module - header.
 * @details Set of functions used to compute authentication message required to erase user keys
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#ifndef _CSE_Debug_H_
#define _CSE_Debug_H_

#include "cse_typedefs.h"

/*
 * @note Module can be mapped either on sw crypto lib or CSE HW engine. It is also usable on little or big endian platforms
 *
 */

/*================================================================================================*/
/**
 * @brief          Authentication message computation Function
 * @details        Generates the authentication message to be used with CSE_Debug_Auth command
 *
 * @param[in]      challenge		pointer to the 128 bit challenge value returned by the CSE_DebugChallenge command
 * @param[in]      UID				pointer to the chip UID value (120 bits)
 * @param[out]     authorization 	pointer to the authorization message
 * @param[in]      master_key 		pointer to master_key value
 *
 * @return         Error code
 * @retval 0  	  When key was loaded properly
 * @retval xx      In case of error
 *
 * @note           The function can use either the CSE HW or the sw crypto library to perform the AES operations
 * 				  hidden thanks to an abstraction function AES_CMAC_Encrypt
 */
extern int Compute_authorization(uint32_t* challenge, uint32_t* UID,
                                 uint32_t* authorization,
                                 uint32_t* master_key);

#endif /* _CSE_Debug_H_ */

/** @} */
