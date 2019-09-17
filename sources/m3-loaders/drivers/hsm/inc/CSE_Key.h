/*
 *  Copyright (C) 2014 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file    CSE_Key.h
 * @brief   CSE Key management module.
 * @details Set of functions used to manage Keys (Non volatile or RAM Key).
 *
 *
 * @addtogroup CSE_driver
 * @{
 */
#ifndef _CSE_KEY_H_
#define _CSE_KEY_H_

#include "cse_typedefs.h"

/*============================================================================*/
/**
 * @brief          Loads a Key cleartext value in the CSE RamKey internal
 *                 storage
 * @details        Loads a key value to be used when selecting index 0xE
 *                 (RAM_KEY)
 *
 * @param[in]      key_pt	pointer to cleartext key value
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *                 ones
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_LoadRamKey(uint8_t *key_pt);

/*============================================================================*/
/**
 * @brief          Exports the RamKey
 * @details        Compute and proide all fields required to load the key value
 *		   that was in the RAMKEY location in a non volatile key
 *		   location
 *
 * @param[out]     M1	pointer to buffer to store the M1 message
 *			(UID, KeyID, Auth ID - 128 bits total)
 * @param[out]     M2	pointer to buffer to store the M2 message
 *			(security flags,  - 256 bits )
 * @param[out]     M3	pointer to buffer to store the M3 message
 *			(MAC of M1 and M2 - 128 bits)
 * @param[out]     M4	pointer to buffer to store the M4 message
 *			(IDs | counter - 256 bits)
 * @param[out]     M5	pointer to buffer to store the M5 message
 *			(MAC of M4 - 128 bits )
 *
 * @return         Error code
 * @retval 0	   When key was exported properly
 * @retval 1..21   In case of error - the error code values are the CSE
 *		   returned ones
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_ExportRamKey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
				 uint8_t *M4, uint8_t *M5);

/*============================================================================*/
/**
 * @brief          Loads a Key in internal non volatile memory
 * @details        Key will be loaded at index described in the already prepared
 *		   messages including key usage restrictions (security flags),
 *		   key index, encrypted value and integrity & authenticity MACs
 *		   for all the params.
 *
 * @param[in]      M1	pointer to message1
 * @param[in]      M2	pointer to message2
 * @param[in]      M3	pointer to message3
 *
 * @param[out]     M4	pointer to message4
 * @param[out]     M5	pointer to message5
 *
 * @param[in]      ext_key_set	1 to use extended key set, 0 for normal key set
 *
 * @return         Error code
 * @retval 0	   When key was loaded properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
extern uint32_t CSE_LoadKey(uint8_t *M1, uint8_t *M2, uint8_t *M3,
			    uint8_t *M4, uint8_t *M5, uint32_t ext_key_set);

#endif //_CSE_KEY_H_
/**
 * @}
 */
