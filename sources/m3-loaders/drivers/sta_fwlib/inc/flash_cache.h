/**
 * @file sta_cache.h
 * @brief This file provides STA ARM CG092 flash cache IP interface.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

enum fc_type {
	FC_AUTO_POWER_AUTO_INVAL,
	FC_MANUAL_POWER_AUTO_INVAL,
	FC_MANUAL_POWER_MANUAL_INVAL_SRAM,
	FC_MANUAL_POWER_MANUAL_INVAL_WITHOUT_SRAM,
};

/*
 * @brief  Initialise Flash Cache at given memory address
 * @param  address: Flash Cache Intruction memory area start address to use
 *                  (DDRAM, AP_ESRAM or SQI)
 * @return None
 */
void fc_init_cache(uint32_t address);

/**
 * @brief  Enable Instruction Flash Cache with given type
 * @param  type: Flash cache type see enum fc_type
 * @return None
 */
void fc_enable_cache(enum fc_type type);

/*
 * @brief  Disable Instruction Flash Cache
 * @param  None
 * @return None
 */
void fc_disable_cache(void);

/*
 * @brief  Invalidate Flash Cache
 * @param  None
 * @return None
 */
void fc_invalidate_cache(void);
