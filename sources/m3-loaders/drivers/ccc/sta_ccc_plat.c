/**
 * @file sta_ccc_plat.c
 * @brief CCC driver platform configuration.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include "sta_ccc_types.h"
#include "sta_nic_security.h"

int ccc_plat_init(void)
{
	nic_reset_security_mem_cfg();
	return 0;
}

void ccc_plat_deinit(void)
{
	nic_set_security_mem_cfg();
}

enum trng_noise_source ccc_plat_get_trng_noise_source(void)
{
	return RING_OSCILLATOR;
}

unsigned int ccc_plat_get_trng_charge_pump_current(void)
{
	/*
	 * Dummy value if version is too low as the PLL noise source is
	 * supported from v1.7
	 */
	return 0x15;
}

unsigned int ccc_plat_get_trng_pll0_reference_clock(void)
{
	/*
	 * Dummy value if version is too low as the PLL noise source is
	 * supported from v1.7
	 */
	return 26;
}
