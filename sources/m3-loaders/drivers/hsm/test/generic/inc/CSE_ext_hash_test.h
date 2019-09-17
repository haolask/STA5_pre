/*
    SPC5-CRYPTO - Copyright (C) 2015 STMicroelectronics

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
 * @file    CSE_ext_hash_test.h
 * @brief   hashtests  header file
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */
#include "cse_types.h"

extern uint32_t hash_test( uint32_t verbose );
extern uint32_t CSE_ext_valTest_hash_nonValidHashAlgoDetection_test(uint32_t P_verbose);
extern uint32_t CSE_ext_valTest_hash_digestGeneration_invalidMemRange_test(uint32_t P_verbose);


/**
 * @}
 */
