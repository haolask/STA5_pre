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
 * @file    CSE_AES_HW_test.h
 * @brief   CSE AES driver computation function tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#ifndef _CSE_AES_HW_TEST_H_
#define _CSE_AES_HW_TEST_H_

#include "cse_typedefs.h"

/*================================================================================================*/
/**
 * @brief          Performs AES ECB, CBC, CMAC tests with NIST reference vectors
 * @details        Test the CSE driver AES computation functions using the RAM key
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
extern int CSE_AES_HW_test(int verbose);

/**
 * @brief          Performs CMAC tests to check support for bit granularity for message length
 * @details        Performs tests of generation and verification with variable message length
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern uint32_t test_CMAC_msg_bit_length( uint32_t verbose );

/**
 * @brief          Performs CMAC tests to check support for bit granularity for verification MAC length
 * @details        Performs tests of verification of MAC with variable mac length, with and without modified tag
 * 				   The modified tags are tests bit per bit, checking that only the specified ones are considered,
 * 				   the others are ignored
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern uint32_t test_CMAC_verify_mac_bit_length( uint32_t verbose );

/**
 * @brief          Performs CMAC tests to check support for bit granularity
 * @details        Calls the message and mac length bit granularity dedicated tests
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern uint32_t test_MAC(uint32_t verbose);



#endif //_CSE_AES_HW_TEST_H_
/**
 * @}
 */
