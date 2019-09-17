/**
 * @file sta_usb.c
 * @brief This file provides USB functions.
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#include "utils.h"

#include "sta_map.h"
#include "sta_platform.h"
#include "sta_mtu.h"

/**
 * @brief	initializes USB physical layer (basically remove PLL power down)
 * @return	0 if no error, not 0 otherwise
 */
int usb_phy_init(void)
{
	/* Remove USB from stdby */
	misc_a7_regs->misc_reg9.bit.iddq_en_for_pd = 1;

	/* UTMI pll_clk_sel: The same bit is set before jumping to AP,
	 * in order to make sure at least 10 ms have elapsed */

	/* Reset values */
	usb_wrapper_regs->usb_pll_ctrl1.reg = 0;

	usb_wrapper_regs->usb_pll_ctrl1.bit.ndiv_26 = 0x37; /* 26MHz crystal */
	usb_wrapper_regs->usb_pll_ctrl1.bit.ndiv_24 = 0x3C; /* 24MHz crystal */

	if (get_soc_id() == SOCID_STA1385) {
		usb_wrapper_regs->usb_pll_ctrl1.bit.frac_input = 1;
		usb_wrapper_regs->usb_pll_ctrl1.bit.utmi_pll_clk_sel = 0;

		usb_wrapper_regs->frac_input = 0x00006276;

		usb_wrapper_regs->usb_pll_ctrl1.bit.pll_pd = 0;
		mtu_wait_delay(100);

		usb_wrapper_regs->usb_pll_ctrl1.bit.strb = 1;
		mtu_wait_delay(10);

		usb_wrapper_regs->usb_pll_ctrl1.bit.strb = 0;
	} else {
		usb_wrapper_regs->usb_pll_ctrl1.bit.utmi_pll_clk_sel = 1;
	}
	return 0;
}

