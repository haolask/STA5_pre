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
 * @brief   Bolero CSE key personalization support functions test - header
 * @details
 *
 *
 * @addtogroup CSE_support_test
 * @{
 */

#ifndef _CSE_BOOT_support_test_H_
#define _CSE_BOOT_support_test_H_

#include "cse_types.h"

/**
 * @brief         Computes BMAC as used in Bolero chip, differs from SHE Boot MAC computation
 * @details       Example of use of the BMAC computation support function, used to compute updated BMAC value for Bolero
 *
 * @param[in] verbose   enable display of intermediate results when set to 1
 * @return         Error code
 * @retval 0  	  When test failed
 * @retval 1   	  When test succeeded
 *
 */
extern int test_compute_BMAC(int verbose);

#endif /* _CSE_KEY_support_test_H_ */

/**
 * @}
 */
