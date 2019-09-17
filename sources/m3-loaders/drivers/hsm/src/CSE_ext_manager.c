/*
 *  Copyright (C) 2014 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "CSE_ext_manager.h"
#include "CSE_Constants.h"
#include "CSE_HAL.h"

#ifdef HSM_RELEASE
/**
 * @brief       Let the HSM start the external key image secure import /
 *		initialization
 * @details	To be called when M3 proxies are up and running
 *		(LoadSTore, Monotonic counter ...)
 *		HSM will then use the callback services to ask for the latest
 *		image, import and check it
 *
 * @param[in]	mbx_buffer: pointer to input mailbox command buffer
 *
 * @return	Error code
 * @retval	CSE_NO_ERR
 * @retval	CSE_GENERAL_ERR
 *
 */
uint32_t CSE_ext_set_ext_remote_key_import(uint32_t mbx_buffer)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd1p(CSE_EXT_IMPORT_KEY_IMAGE, mbx_buffer);

	return ret;
}
#endif

/**
 * @brief	Set external valid external memory ranges to be considered
 *		when doing parameters checking
 * @details	Set external memory range to be considered as valid for external
 *		DDR and flash
 *		start address must a multiple of 4, end addresses must be a
 *		(multiple of 4)-1
 *
 * @param[in]	dram_start	start of external DDR area
 *				(must be a multiple of 4)
 * @param[in]	dram_end	end of external DDR area
 *				(must be a multiple of 4 -1)
 * @param[in]	flash_start	start of external flash area
 *				(must be a multiple of 4)
 * @param[in]	flash_end	end of external flash area
 *				(must be a multiple of 4 -1)
 *
 * @return	Error code
 * @retval	CSE_NO_ERR	When configuration is applied
 * @retval	CSE_GENERAL_ERR
 *
 */
uint32_t CSE_ext_set_valid_ext_mem_range(uint32_t dram_start, uint32_t dram_end,
					 uint32_t flash_start,
					 uint32_t flash_end)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_EXT_SET_VALID_EXT_MEM_RANGE,
				 dram_start, dram_end, flash_start, flash_end);

	return ret;
}

/**
 * @brief	Locks valid external memory ranges to be considered when doing
 *		parameters checking
 * @details	Prevent any further update of the valid external memory range
 *
 * @return	Error code
 * @retval	CSE_NO_ERR
 * @retval	CSE_GENERAL_ERR
 *
 */
uint32_t CSE_ext_lock_valid_ext_mem_range(void)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd(CSE_EXT_LOCK_EXT_MEM_RANGE);

	return ret;
}

#ifdef HSM_RELEASE
/*!
 * @brief	Get ram monotonic counter value if selected
 * @details	This routine returns the current value of the monotnic
 *		counter if the ram counter is active,
 *		an error if any other monotonic counter source is selected
 *
 * @param[out]	pcounter - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: error code
 * @retval	: CSE_NO_ERR	When value can be used
 * @retval	CSE_GENERAL_ERR (when another source is enabled)
 */
uint32_t CSE_ext_Get_ram_monotonic_counter_value(uint32_t *pcounter)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	uint32_t  ret = CSE_GENERAL_ERR;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_GET_RAM_MONOTONIC,
				     (uint32_t)pcounter);
	if (hsm_ret == CSE_NO_ERR)
		ret = CSE_NO_ERR;

	return ret;
}

/*!
 * @brief	Set ram monotonic counter value if selected
 * @details	This routine change the ram monotonic counter value
 *		- if the ram counter is active, an error is returned if any
 *		other monotonic counter source is selected
 *
 * @param[in]	counter - value to be used
 * @return	: error code
 * @retval	: CSE_NO_ERR	When value can be used
 * @retval	CSE_GENERAL_ERR (when another source is enabled)
 */

uint32_t CSE_ext_Set_ram_monotonic_counter_value(uint32_t counter)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	uint32_t  ret = CSE_GENERAL_ERR;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_SET_RAM_MONOTONIC, counter);
	if (hsm_ret == CSE_NO_ERR)
		ret = CSE_NO_ERR;

	return ret;
}

/**
 * @brief	Configure M3 SHE registers memory mapping
 * @details	Set emulated register area exposed to M3 core
 *
 * @param[in]	prequest	pointer to input request structure
 *
 * @return	Error code
 * @retval	ERC_NO_ERROR	When setup was done
 * @retval	ERC_GENERAL_ERROR
 *
 * @note	ERC_SEQUENCE_ERROR is returned as soon as the service is
 *		disabled
 */
uint32_t CSE_API_ext_set_m3_she_reg_mem_config(uint32_t m3_she_reg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd1p(CSE_EXT_SET_M3_SHE_REGISTERS_MEM_CONFIG,
				 m3_she_reg);

	return ret;
}

/**
 * @brief	Lock the configuration of M3 SHE registers memory mapping
 * @details	Lock the new configuration of emulated registers for m3 core
 *
 * @param[in]	prequest	pointer to input request structure
 *
 * @return	Error code
 * @retval	ERC_NO_ERROR	When setup was done
 * @retval	ERC_GENERAL_ERROR
 *
 * @note	ERC_SEQUENCE_ERROR is returned as soon as the service is
 *		disabled
 */
uint32_t CSE_API_ext_lock_m3_she_reg_mem_config(void)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd(CSE_EXT_LOCK_M3_SHE_REGISTERS_MEM_CONFIG);

	return ret;
}
#endif

/**
 * @brief	Configure A7 SHE registers memory mapping
 * @details	Set emulated register area exposed to A7 core
 *
 * @param[in]	prequest	pointer to input request structure
 *
 * @return	Error code
 * @retval	ERC_NO_ERROR	When setup was done
 * @retval	ERC_GENERAL_ERROR
 *
 * @note	ERC_SEQUENCE_ERROR is returned as soon as the service is
 *		disabled
 */
uint32_t CSE_API_ext_set_a7_she_reg_mem_config(uint32_t a7_she_reg)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd1p(CSE_EXT_SET_A7_SHE_REGISTERS_MEM_CONFIG,
				 a7_she_reg);

	return ret;
}

/**
 * @brief	Lock the configuration of A7 SHE registers memory mapping
 * @details	Lock the new configuration of emulated registers for a7 core
 *
 * @param[in]	prequest	pointer to input request structure
 *
 * @return	Error code
 * @retval	ERC_NO_ERROR	When setup was done
 * @retval	ERC_GENERAL_ERROR
 *
 * @note	ERC_SEQUENCE_ERROR is returned as soon as the service is
 *		disabled
 */
uint32_t CSE_API_ext_lock_a7_she_reg_mem_config(void)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd(CSE_EXT_LOCK_A7_SHE_REGISTERS_MEM_CONFIG);

	return ret;
}
