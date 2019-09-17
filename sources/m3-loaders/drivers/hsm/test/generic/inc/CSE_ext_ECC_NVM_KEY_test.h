/*
    SPC5-CRYPTO - Copyright (C) 2016 STMicroelectronics

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
 * @file    CSE_ext_ECC_NVM_KEY_Test.h
 * @brief   ECC ECIES tests header file
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */
#ifndef _CSE_EXT_ECC_NVM_KEY_TEST_H_
#define _CSE_EXT_ECC_NVM_KEY_TEST_H_

#include "cse_types.h"

#define C_NB_OF_NVM_KEY_TV    45
#define C_NB_OF_NVM_KEY384_TV 30
#define C_NB_OF_NVM_KEY521_TV 15
#define C_NB_OF_SIGN_VERIFY_KEY_TV 15
extern uint32_t ecc_generateEcKeyInNVM_SignAndVerify_test(uint32_t verbose);

#endif /* _CSE_EXT_ECC_NVM_KEY_TEST_H_ */
/**
 * @}
 */
