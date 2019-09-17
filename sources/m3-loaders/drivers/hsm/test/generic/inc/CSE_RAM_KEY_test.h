/*
    CSE driver test - Copyright (C) 2014-2015 STMicroelectronics

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
 * @file    CSE_RAM_KEY_test.h
 * @brief   CSE load and export Key test
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#include "cse_typedefs.h"

#ifndef _CSE_RAM_KEY_TEST_H_
#define _CSE_RAM_KEY_TEST_H_

/**
 * @brief          Test export RAM key function
 * @details        Load a known key and compare the export_RAM generated message with expected ones
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 * @note		   The generated message is only valid for a particular chip since it involves its unique ID
 */
extern int CSE_export_RAM_test(int verbose);

/**
 * @brief          Perform RAM key functions test (load_cleartext, export, load_protected)
 * @details        Perform an AES operation to check that key has really been updated
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 * @note		   The test may not work if debugger is attached
 */
extern void ram_key_test( uint32_t verbose );

#endif //_CSE_RAM_KEY_TEST_H_
/**
 * @}
 */
