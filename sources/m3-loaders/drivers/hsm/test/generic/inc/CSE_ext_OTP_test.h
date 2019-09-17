/*
    CSE driver test - Copyright (C) 2017 STMicroelectronics

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
 * @file    CSE_ext_OTP_test.h
 * @brief   CSE _ext OTP commands tests - header
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include "cse_typedefs.h"


/**
 * @brief          OTP read test
 * @details        try reading OTP area
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *                 sec_boot_key_enabled  (set it to 0 if test has to be ran on a part where secure boot key is not yet programmed - all public words available)
 *                 set it to 1 if secure boot key is programmed --> some words are no longer availble as public words
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern uint32_t otp_read_test( uint32_t verbose, uint32_t  sec_boot_key_enabled);

/**
 * @brief          Secure boot key write test
 * @details        writes secure boot key and check it can't be updated
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern uint32_t  otp_write_encrypted_boot_key_test( uint32_t verbose );

/**
 * @brief          OTP write test
 * @details        try writing - and reading back OTP area
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *                 sec_boot_key_enabled  (set it to 0 if test has to be ran on a part where secure boot key is not yet programmed - all public words available)
 *                 set it to 1 if secure boot key is programmed --> some words are no longer availble as public words
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern uint32_t otp_write_test( uint32_t verbose, uint32_t  sec_boot_key_enabled);

/**
 * @}
 */
