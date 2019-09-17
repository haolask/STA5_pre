/**
 * @file sta_src_a7.c
 * @brief System Reset Controller Application Processor A7 functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "utils.h"

#include "sta_map.h"
#include "sta_src.h"
#include "sta_src_a7.h"
#include "sta_platform.h"

/* SRCR4_CLKDIV bits */
#define CLKDIV_SDRAM_ASYNC						0
#define CLKDIV_SDRAM_SYNC						1

#define CLKDIV_HCLCKDIV(x)						(x & 7)
#define CLKDIV_SDRAMDIV(x)						(x & 7)

#define HCLKDIV_EXPECTED 2
/*
 * Global to store he hclkdiv value that was set by the ROM.
 * This global is used to restore proper value at soft reset time,
 * thus to work-around a hardware issue in the SRC-M3 FSM.
 */
unsigned int hclk_div_rom;

/**
  * @brief  changes the state of a given clock
  * @param	src: current SRC A7 device
  * @param	pclk: the clock to be updated
  * @param	enable: true to enable it, false otherwise
  * @retval 0 if no error, not 0 otherwise
  */
int srca7_pclk_change_state(t_src_a7 *src, unsigned int pclk, bool enable)
{
	if (pclk == 27 || pclk >= MAX_SRCA7_1_CLK)
		return -EINVAL;

	if (enable) {
		if (pclk < MAX_SRCA7_0_CLK)
			src->pcken0 |= BIT(pclk);
		else
			src->pcken1 |= BIT(pclk - MAX_SRCA7_0_CLK);
	} else {
		if (pclk < MAX_SRCA7_0_CLK)
			src->pckdis0 |= BIT(pclk);
		else
			src->pckdis1 |= BIT(pclk - MAX_SRCA7_0_CLK);
	}

	return 0;
}

/* private routine to change all pclks states (enabled/disabled) */
static void srca7_pclk_change_state_all(t_src_a7 *src, bool enable)
{
	if (enable) {
		src->pcken0 = 0xFFFFFFFF;
		src->pcken1 = 0xFFFFFFFF;
	} else {
		src->pckdis0 = 0xFFFFFFFF;
		src->pckdis1 = 0xFFFFFFFF;
	}
}

/**
  * @brief  disables all pclk
  * @param	src: current SRC R4 device
  * @retval None
  */
void srca7_pclk_disable_all(t_src_a7 *src)
{
	srca7_pclk_change_state_all(src, false);
}

/**
  * @brief  enables all pclk
  * @param	src: current SRC R4 device
  * @retval None
  */
void srca7_pclk_enable_all(t_src_a7 *src)
{
	srca7_pclk_change_state_all(src, true);
}

/**
  * @brief  updates frequency parameters for the given pll
  * @param	src: current SRC R4 device
  * @param	pll refer to @srcm3_plls
  * @param	odf: output divison factor (0..15) applicable for PLL3
  * @retval 0 if no error, not 0 otherwise
  */
static int srca7_pll_update_freq(t_src_a7 *src, unsigned int pll,
		int odf, bool fractl, bool ditherdis)
{
	switch (pll) {
		case PLL3: /* PLL3 has strobe_bypass (not managed here),
					  fractionnal ctl and dither fields */

			src->scpll3fctrl.bit.fractl = fractl;
			src->scpll3fctrl.bit.ditherdis = ditherdis;

			if (odf > 0) {
				/*
				 * ODF is applied when STROBE is low, ODF values are then
				 * loaded when STROBE is high. After that ODF can be set
				 * again to low.
				 */

				/* set STROBE to low if needed */
				if (src->scpll3fctrl.bit.odf_strobe)
					src->scpll3fctrl.bit.odf_strobe = 0;

				src->scpll3fctrl.bit.odf = odf & 0xf;

				/* set STROBE to high */
				src->scpll3fctrl.bit.odf_strobe = 1;
			}

		break;
		default:
			return -EINVAL;
	}

	return 0;
}

/**
  * @brief  updates frequency parameters for the pll3
  * @param	src: current SRC R4 device
  * @param	pll refer to @srcm3_plls
  * @param	odf: output divison factor (0..15)
  * @param	fractl: enable/disable fractionnal control
  * @param	dither: enable/disable dither
  * @retval 0 if no error, not 0 otherwise
  */
int srca7_pll3_update_freq(t_src_a7 *src, int odf, bool fractl, bool dither)
{
	return srca7_pll_update_freq(src, PLL3, odf, fractl, dither);
}

/**
 * @brief	configure and enable the pll ARM
 * @param	idf: input division factor
 * @param	ndiv: divisor
 * @param	odf: output division factor (phi)
 */
static void a7_configure_pllarm(t_src_a7 *src, uint8_t idf, uint8_t ndiv, uint8_t odf)
{
	a7_ssc_regs->pllarm_ctrl.bit.enable = 0;

	a7_ssc_regs->pllarm_freq.bit.strb_odf = 0;
	a7_ssc_regs->pllarm_freq.bit.odf = odf & 0x3f;
	a7_ssc_regs->pllarm_freq.bit.strb_odf = 1;

	a7_ssc_regs->pllarm_freq.bit.idf = idf & 0x7;
	a7_ssc_regs->pllarm_freq.bit.ndiv = ndiv & 0xff;

	a7_ssc_regs->pllarm_freq.bit.cp = src_charge_pump_conversion(ndiv) & 0x1f;

	a7_ssc_regs->pllarm_ctrl.bit.enable = 1;

	/* Wait for the PLL to be locked */
	while(!a7_ssc_regs->pllarm_lockp.bit.clk_good);
}

/**
  * @brief  initialize SRC R4 controller
  * @param	src: current SRC R4 device
  */
void srca7_init(t_src_a7 *src)
{
	/* Select external clock for the A7 sub-system and wait for ack */
	a7_ssc_regs->chclkreq.bit.chgclkreq = 1;
	while(!a7_ssc_regs->chclkreq.bit.chgclkreq);

	a7_ssc_regs->chclkreq.bit.divsel = 1;

	if (get_soc_id() == SOCID_STA1385)
		/* configure the PLL ARM @ 1196MHz and PHI @ 600MHz */
		a7_configure_pllarm(src, 1, 23, 2);
	else {
		/* configure the PLL ARM @ 1200MHz and PHI @ 600MHz */
		a7_configure_pllarm(src, 1, 25, 2);
		/* enable pll3 fract control, disable dithering */
		srca7_pll3_update_freq(src, -1, 1, 1);
	}

	/**
	 * Let's store the hclkdiv value that was intially set by the ROM.
	 * This is required to later restore proper value at soft reset time,
	 * thus to work-around a hardware issue in the SRC-M3 FSM.
	 * More in details:
	 *
	 * Assuming HCLKDIV was set to 2 before triggering a software reset,
	 * once software reset is done, the ROM programming of HCLKDIV=2 does
	 * not have effect as the flops holding such values do not get reset
	 * by software reset. So they keep the previous value of 2 and the
	 * logic of divider is such that if the value you write is same as what
	 * you already have, no action takes place. However the flops
	 * controlling the actual pre-scaler do get reset by software reset to
	 * their initial value of 5, so the result is that after a software
	 * reset, we might think that HCLKDIV=2. But instead the division
	 * factor used will be 5, which is one of the “forbidden” values for
	 * the SRC-M3 FSM hardware clock-freeze problem, hence the random
	 * failures that may occur following a software reset...
	 * By making HCLKDIV=(HCLKDIV previously set by ROM + 1), this before
	 * issueing a software reset, we create a condition where the ROM action
	 * of writing HCLKDIV=2 actually takes place and things become as they
	 * should.
	 */
	hclk_div_rom = src_a7_regs->scclkdivcr.bit.hclk_div;
	/**
	 * if expected HCKDIV is same than one already set by ROM,
	 * first set the expected value + 1, thus to make sure that
	 * expected value will be really taken into account.
	 */
	if (hclk_div_rom == HCLKDIV_EXPECTED)
		src->scclkdivcr.bit.hclk_div = HCLKDIV_EXPECTED + 1;
	if (get_soc_id() == SOCID_STA1385)
		src->scclkdivcr.bit.hclk_div = HCLKDIV_EXPECTED + 1;
	else
		src->scclkdivcr.bit.hclk_div = HCLKDIV_EXPECTED;
}

