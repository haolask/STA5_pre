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
 * @file    CSE_internal_tests.c
 * @brief   Module containing CSE internal tests
 * @details Set of functions used to exercise CSE Cryptographic extensions.
 *
 *
 * @addtogroup CSE_internal_tests
 * @{
 */

#include "cse_typedefs.h"
#include "CSE_Constants.h"
#include "CSE_internal_tests.h"
#include "err_codes.h"

#include "CSE_HAL.h"

/*================================================================================================*/
/**
 * @brief          Configure RNG to support pre-defined test vectors
 * @details        Configure the RNG to draw random number coming from pre_defined test vectors
 *
 * @param[in]     P_pRandomSrc Pointer to a byte array containing random test vectors
 * @param[in]     P_byte_size  Number of bytes of the random test vector
 *
 * @return         Error code
 * @retval 0       When the initialization was done properly
 * @retval 1..21   In case of error - the error code values are the CSE returned ones
 *                see details in CSE_ECR register field description table 743
 *
 * @api
 *
 * @pre
 *
 * @implements
 *
 * @note           -
 */
uint32_t CSE_RNG_config4InternalTest(const vuint8_t* P_pRandomSrc, vuint32_t P_byte_size)
{
    uint32_t ret = CSE_NO_ERR;

    /* Write parameters */
    /* P1 is the address of the buffer of pre-defined random number
     * P2 is the number of random bytes
     */

    ret = CSE_HAL_send_cmd2p(CSE_RNG_CONF4_INTERNAL_TESTS,
                             (uint32_t)P_pRandomSrc,
                             (uint32_t)P_byte_size);

    /* Translate CSE error code */
    if( CSE_NO_ERR == ret )
    {
         ret = ECC_SUCCESS;
    }

    return(ret);
}

/**
 * @}
 */
