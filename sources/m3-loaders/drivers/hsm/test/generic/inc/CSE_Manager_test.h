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
 * @file    CSE_Manager_test.h
 * @brief   CSE Manager commands tests - header
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */


/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*
 * Addresses should be aligned with emulated register set exposed to A7 core
 * in HSM side
 */
#define SHE_REGS1_REMOTE_BASE	(0x7001FFC0UL)
#define SHE_REGS1_REMOTE	(*((struct she_registers_t *)SHE_REGS1_REMOTE_BASE))


/**
 * @brief          Chip UID test
 * @details        Asks for the chip UID and reports the value
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern int CSE_GetID_test(int verbose);

/**
 * @brief          Firmware ID test
 * @details        Asks for the firmware version ID and reports the value
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
extern int CSE_Get_Firmware_ID_test( int verbose );

/**
 * @brief          CSE init test
 * @details        Calls CSE_init command and display returned error code and status register
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 *
 */
extern void CSE_init_test( uint32_t verbose );

/**
 * @brief          Cancel while using HSM test
 * @details        Calls CANCEL command while already running a crypto operation
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 */
extern uint32_t cancel_while_mac_test( uint32_t verbose );

/**
 * @brief          Management of M3 emulated registers configuration
 * @details        Allow to see and change adresses used in emulated registers configuration
 *
 * @param[in]      verbose    enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0	   When test failed
 * @retval 1	   When test succeeded
 *
 */
uint32_t m3_emul_reg_config_management( uint32_t verbose );

/**
 * @brief          Management of A7 emulated registers configuration
 * @details        Allow to see and change adresses used in emulated registers configuration
 *
 * @param[in]      verbose    enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0	   When test failed
 * @retval 1	   When test succeeded
 *
 */
uint32_t a7_emul_reg_config_management( uint32_t verbose );

/**
 * @brief	Management of key storage initialization command
 * @details	This function handles key storage initialization command
 *		configuring mailbox command buffer.
 *
 * @param[in]	verbose    enable display of input, computed and expected
 *		values when set to 1
 *
 * @return	Error code
 * @retval 0	When test failed
 * @retval 1	When test succeeded
 *
 */
uint32_t remote_key_import_config_management(uint32_t verbose);

/**
 * @}
 */
