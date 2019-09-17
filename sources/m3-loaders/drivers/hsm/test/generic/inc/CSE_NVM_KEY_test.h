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
 * @file    CSE_NVM_KEY_test.h
 * @brief   CSE Non Volatile Memory stored keys test - header
 * @details
 *
 *
 * @addtogroup CSE_driver
 * @{
 * @addtogroup CSE_driver_test
 * @{
 */

#ifndef _CSE_NVM_KEY_TEST_H_
#define _CSE_NVM_KEY_TEST_H_

/**
 * @brief         Test the load key function for NVM keys
 * @details       Load keys in the NVM locations in a virgin chip ( or at least a chip whose user keys are empty)
 *
 * @param[in]     verbose		enable display of input, computed and expected values when set to 1
 *
 * @return        Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note			  The expected message are only valid for a particular chip since it involves its unique ID
 */
extern int CSE_NVM_key_update_interactive_test(int verbose);

/**
 * @brief         Test the NVM keys to check if they're present and available for encryption or MAC operation
 * @details       perform an ECB and CMAC operation attempt for each key and display the low level status and error report
 *
 * @param[in]     verbose		enable display of input, computed and expected values when set to 1
 *
 * @return        Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
extern int CSE_NVM_Key_AES_test(int verbose);

/**
 * @brief         Test the NVM key load function with already pre-computed messages
 * @details       Let the user select which key he wants to load and perform an ECB and CMAC test before and after load
 *
 * @param[in]     verbose		enable display of input, computed and expected values when set to 1
 *
 * @return        Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note		      The pre-computed messages are only valid for one chip a particular chip since involving its unique ID
 */
extern int CSE_NVM_precomputed_key_load_test(int verbose);

/**
 * @brief         Erase all NVM user keys
 * @details       Perform the DEBUG_CHALLENGE/DEBUG_AUTHORIZATION sequence to erase the user keys in non volatile memory
 *
 * @param[in]     verbose		enable display of input, computed and expected values when set to 1
 * @param[in]     master_key 	pointer to master_key value
 *
 * @return        Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note			  Will only work if no key has the Write Protect flag set
 */
extern int CSE_erase_all_user_keys_test(int verbose, uint32_t* master_key);

/**
 * @brief          Erase all NVM user keys - with selection of the master key
 * @details        Perform the DEBUG_CHALLENGE/DEBUG_AUTHORIZATION sequence to erase the user keys in non volatile memory
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @note		   Will only work if no key has the Write Protect flag set
 */
extern void erase_keys_test( uint32_t verbose );

/**
 * @brief          Iterative key update test
 * @details        perform 64 updates of 2 keys starting from user specified counter
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @note		   Will only work if key has not the Write Protect flag set, and counter is greater than current one
 */
extern void iterative_nvm_key_update_test( uint32_t verbose );

/**
 * @brief          Loads default key set, with known values and different flags
 * @details        Loads a Set of key allowing to test various usage restriction conditions
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @note		   Will only work if keys are empty
 * @note		   No key has the Write Protect flag
 */
extern void load_default_key_set( uint32_t verbose );

#endif /* _CSE_NVM_KEY_TEST_H_ */

/**
 * @}
 */
/**
 * @}
 */
