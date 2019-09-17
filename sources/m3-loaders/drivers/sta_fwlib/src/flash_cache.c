/**
 * @file sta_cache.c
 * @brief This file provides STA ARM CG092 flash cache IP driver functions.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include "sta_common.h"
#include "trace.h"

#include "flash_cache.h"

/*
 * @brief  Initialise Flash Cache at given memory address
 * @param  address: Flash cache Intruction memory area start address to use
 *                  (DDRAM, AP_ESRAM or SQI)
 * @return None
 */
void fc_init_cache(uint32_t address)
{
	if ((address >= DDRAM_BASE && address < (DDRAM_BASE + DDRAM_SIZE))
	    || (address >= ESRAM_A7_BASE &&
		address < (ESRAM_A7_BASE + ESRAM_A7_SIZE))
	    || (address >= SQI0_NOR_BB_BASE &&
		address < (SQI1_NOR_BB_BASE + 0x08000000))) {
		misc_a7_regs->misc_reg75.reg = (address >> 22);
		while (misc_a7_regs->misc_reg75.reg != (address >> 22))
			;
	} else {
		TRACE_ERR("%s: Bad addr\n", __func__);
	}
}

/**
 * @brief  Enable Instruction Flash Cache with given type
 * @param  type: Flash cache type see enum fc_type
 * @return None
 */
void fc_enable_cache(enum fc_type type)
{
	switch (type) {
	default:
	case FC_AUTO_POWER_AUTO_INVAL:
		/* Auto power, auto invalidate */
		cg092_fc_regs->ccr.bit.set_man_pow = 0;
		cg092_fc_regs->ccr.bit.set_man_inv = 0;
		break;

	case FC_MANUAL_POWER_AUTO_INVAL:
		/* Manual power, auto invalidate */
		cg092_fc_regs->ccr.bit.set_man_pow = 1;
		cg092_fc_regs->ccr.bit.set_man_inv = 0;
		/* Request power */
		cg092_fc_regs->ccr.bit.pow_req = 1;
		/* Wait until power up has completed */
		while (!cg092_fc_regs->sr.bit.pow_stat)
			;
		break;

	case FC_MANUAL_POWER_MANUAL_INVAL_SRAM:
		/* Set operation mode */
		cg092_fc_regs->ccr.bit.set_man_pow = 1;
		cg092_fc_regs->ccr.bit.set_man_inv = 1;
		cg092_fc_regs->ccr.bit.statistic_en = 1;
		cg092_fc_regs->ccr.bit.set_prefetch = 1;
		/* Request power */
		cg092_fc_regs->ccr.bit.pow_req = 1;
		/* Wait until power up has completed */
		while (!cg092_fc_regs->sr.bit.pow_stat)
			;
		/* Request manual invalidation */
		cg092_fc_regs->ccr.bit.inv_req = 1;

		/* Wait until the invalidation has finished */
		while (cg092_fc_regs->ccr.bit.inv_req)
			;
		break;

	case FC_MANUAL_POWER_MANUAL_INVAL_WITHOUT_SRAM:
		/* Set operation mode */
		cg092_fc_regs->ccr.bit.set_man_pow = 1;
		cg092_fc_regs->ccr.bit.set_man_inv = 1;
		/* Request power */
		cg092_fc_regs->ccr.bit.pow_req = 1;
		/* Wait until power up has completed */
		while (!cg092_fc_regs->sr.bit.pow_stat)
			;
		break;
	}

	/* Enable Cache */
	cg092_fc_regs->ccr.bit.en = 1;
	/* Wait until cache is enabled */
	while (cg092_fc_regs->sr.bit.cs != 2)
		;
}

/*
 * @brief  Disable Instruction Flash Cache
 * @param  None
 * @return None
 */
void fc_disable_cache(void)
{
	/* Disable Cache */
	cg092_fc_regs->ccr.bit.en = 0;
}

/*
 * @brief  Invalidate Flash Cache
 * @param  None
 * @return None
 */
void fc_invalidate_cache(void)
{
	/* Disable the cache */
	cg092_fc_regs->ccr.bit.en = 0;
	/* Wait until the cache disabled */
	while (cg092_fc_regs->sr.bit.cs != 0)
		;
	/* Request manual invalidation */
	cg092_fc_regs->ccr.bit.inv_req = 1;
	/* Wait until the invalidation has finished */
	while (cg092_fc_regs->ccr.bit.inv_req != 0)
		;
	/* Enable the cache back */
	cg092_fc_regs->ccr.bit.en = 1;
	/* wait until cache is enabled */
	while (cg092_fc_regs->sr.bit.cs != 2)
		;
}
