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
 * @file    CSE_Key_support_test.h
 * @brief   CSE key personalization support functions test - header
 * @details
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 * @addtogroup CSE_support_test
 * @{
 */

#ifndef _CSE_KEY_support_test_H_
#define _CSE_KEY_support_test_H_

#include "cse_types.h"

/**
 * @brief         Tests M1-M5 messages generation function, part of the support function set
 * @details       Example of use of the MA-M5 messages generation function to be used when loading a key with Mey_LOAD
 *
 * @param[in] 	  verbose   enable display of intermediate results when set to 1
 *
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
extern int M1_M5_generation_test(int verbose);

#endif /* _CSE_KEY_support_test_H_ */

/**
 * @}
 */

/**
 * @}
 */
