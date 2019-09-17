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
 * @file    CSE_Debug.c
 * @brief   CSE Debug support module
 * @details Set of functions used to compute authentication message required to erase user keys
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#include "cse_typedefs.h"
#include "crypto_al.h"
#include "err_codes.h"

#include "CSE_Key_update.h"
#include "CSE_Debug.h"

#include "serialprintf.h"

/*
 * @note Module can be mapped either on sw crypto lib or CSE HW engine. It is also usable on little or big endian platforms
 *
 */

const uint8_t DEBUG_KEY_C[16] = { 0x01,0x03,0x53,0x48, 0x45,0x00,0x80,0x00, 
                                  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xB0 };

/*================================================================================================*/
/**
 * @brief          Authentication message computation Function
 * @details        Generates the authentication message to be used with CSE_Debug_Auth command
 *
 * @param[in]      challenge		pointer to the 128 bit challenge value returned by the CSE_DebugChallenge command
 * @param[in]      UID			    pointer to the chip UID value (120 bits)
 * @param[out]     authorization    pointer to the authorization message
 * @param[in]      master_key 		pointer to master_key value
 *
 * @return         Error code
 * @retval 0  	  When key was loaded properly
 * @retval xx      In case of error
 *
 * @note           The function can use either the CSE HW or the sw crypto library to perform the AES operations
 * 				  hidden thanks to an abstraction function AES_CMAC_Encrypt
 */
int Compute_authorization(uint32_t* challenge, uint32_t* UID,
                          uint32_t* authorization, uint32_t* master_key)
{
	#define CHALLENGE_UID_LENGTH 31

	uint8_t challenge_UID[CHALLENGE_UID_LENGTH];
	uint32_t length = CHALLENGE_UID_LENGTH;

    uint32_t KDEBUG [4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

	uint8_t* challenge_byte = (uint8_t*)challenge;
	uint8_t* UID_byte = (uint8_t*)UID;

	int i;
	uint32_t ret = 0;

	/* concatenate challenge and UID in challenge_UID input message */
	for (i = 0; i < 16; i++)
	{
		challenge_UID[i] = challenge_byte[i];
	}
	for (i = 0; i < 15; i++)
	{
		challenge_UID[16 + i] = UID_byte[i];
	}

	/* 
     * compute the DEBUG_KEY value associated to the master key value
     */
	ret = KDF( (uint8_t*)master_key, (uint8_t*)DEBUG_KEY_C, (uint8_t*)KDEBUG );

	/* Compute authorization message */
    ret = AES_CMAC_Encrypt((uint8_t*)challenge_UID, (uint32_t)length,
		  	  	  	  (const uint8_t*)KDEBUG, 16, 16, (uint8_t*)authorization);
	return(ret);
}

/** @} */
