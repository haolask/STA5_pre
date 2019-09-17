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
 * @file    CSE_RNG_test.h
 * @brief   CSE RNG commands tests - header
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

/**
 * @brief          True Random Number generator test
 * @details        Get a random stream and perform online test
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern int CSE_TRNG_test(int verbose);

/**
 * @brief          Pseudo Random Number generator test
 * @details        Init, get a random stream, re-seed and ask for another stream
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern int CSE_PRNG_test(int verbose);

/**
 * @}
 */
