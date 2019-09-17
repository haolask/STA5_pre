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
 * @file    CSE_BMAC_update.c
 * @brief   Bolero CSE Boot MAC update support module.
 * @details Set of functions used to compute required material to update Boot MAC.
 *
 *
 * @addtogroup CSE_personalization_support
 * @{
 */

#include "cse_typedefs.h"
#include "crypto_al.h"

/*================================================================================================*/
/**
 * @brief          Boot MAC computation function
 * @details        Computes the reference MAC value to store in BOOT_MAC_KEY non volatile location for Bolero CSE engine
 *
 * @param[in]      start_ad			pointer to the beginning of the boot code to consider
 * @param[in]      byte_size 		byte size of the binary boot code to consider
 * @param[in]      key 				pointer to cleartext value of the key to use to compute MAC
 * @param[out]     result_MAC 		pointer to computed MAC
 *
 * @return         Error code
 * @retval 0  	   When MAC was computed properly
 * @retval xx      In case of error
 *
 *
 * @note          * The function can use either the CSE HW or the sw crypto library to perform the AES operations
 * 				    hidden thanks to an abstraction function AES_CMAC_Encrypt
 * 				  * For better run-time check performances, the start address should be aligned on a 64bits boundary (0x0, 0x8 ..)
 *
 * 				  * Not compliant with SHE Boot_mac value algorithm, only valid for Bolero
 */
int Compute_BMAC(uint8_t* start_ad, uint32_t byte_size, uint8_t* key,
                 uint8_t* result_MAC) {
    uint32_t ret = 0;

    /* This MAC computation is only suitable for Bolero chip, BOOT_MAC value described in SHE is  */
    /*     CMAC[BOOT_MAC_KEY] ( 00...00|SIZE|BOOTLOADER_PAYLOAD) */
	/*                          <.16Bytes..> <...size Bytes...> */

    ret = AES_CMAC_Encrypt(start_ad, byte_size, key, 16, 16, result_MAC);
    return (ret);
}


/**
 * @}
 */
