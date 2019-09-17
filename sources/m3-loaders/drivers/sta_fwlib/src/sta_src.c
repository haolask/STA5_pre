/**
 * @file sta_src.c
 * @brief System Reset Controller functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "utils.h"

#include "sta_map.h"
#include "sta_mtu.h"
#include "sta_src.h"
#include "sta_a7.h"
#include "sta_platform.h"

#define SRCM3_REMAPCLR_REQ						0
#define SRCM3_REMAPSTAT_DEFAULT					0
#define SRCM3_RTTEN								1

#define SRCM3_MODESTATUS_INTOSC					1
#define SRCM3_MODESTATUS_EXTOSC					2
#define SRCM3_MODESTATUS_XTALCTL				3
#define SRCM3_MODESTATUS_NORMAL					4
#define SRCM3_MODESTATUS_INTOSC					1

/* SRCM3_CLKDIVCR bits */
#define CLCKDIV_SQI_PLL2FVCOBY2_BY4				0
#define CLCKDIV_SQI_PLL2FVCOBY2_BY3				1
#define CLCKDIV_SQI_PLL2PHI						2

#define CLKDIV_SDMMC_PLL2VCOBY2_BY7				0
#define CLKDIV_SDMMC_PLL2VCOBY2_BY6				1
#define CLKDIV_SDMMC_PLL2PHI					2

#define CLKDIV_SDIO_PLL2VCOBY2_BY7				0
#define CLKDIV_SDIO_PLL2PHI					1
#define CLKDIV_SDIO_PLL2VCOBY2_BY6				2

#define CLKDIV_AUDIO_SSM_CLK_PLL2VCOBY2_BY5		0
#define CLKDIV_AUDIO_SSM_CLK_PLL2VCOBY2_BY4		1

#define CLKDIV_AUDIO_SSF_512_PLL2VCOBY2_BY25	0
#define CLKDIV_AUDIO_SSF_512_PLL2VCOBY2_BY26	1

#define SRCM3_CLKDIV_CAN_PLL2VCOBY2_BY3			0
#define SRCM3_CLKDIV_CAN_PLL2VCOBY2_BY4			1

#define CLKDIV_GFX_PLL4VCOBY2					0
#define CLKDIV_GFX_PLL2VCOBY3					1

#define CLKDIV_VDO_PLL4PHI						0
#define CLKDIV_VDO_PLL2VCOBY2_BY3				1

/* SRCM3_RESSTAT bits */
#define SRCM3_RESSTAT_SWRESREQM3				1

/*
 * Global to store the hclkdiv value that was set by the ROM.
 * This global is used to restore proper value at soft reset time,
 * thus to work-around a hardware issue in the SRC-M3 FSM.
 */
extern unsigned int hclk_div_rom;

typedef struct {
	int cp_value;
	unsigned int ndiv_range;
} t_sta_src_cp_table;

#define LAST_NDIV 0xFFFFFFF
t_sta_src_cp_table sta_src_cp_table[] = {
	{-1, 7},
	{ 0x07, 71 },
	{ 0x08, 79 },
	{ 0x09, 87 },
	{ 0x0A, 95 },
	{ 0x0B, 103 },
	{ 0x0C, 111 },
	{ 0x0D, 119 },
	{ 0x0E, 127 },
	{ 0x0F, 135 },
	{ 0x10, 143 },
	{ 0x11, 151 },
	{ 0x12, 159 },
	{ 0x13, 167 },
	{ 0x14, 175 },
	{ 0x15, 183 },
	{ 0x16, 191 },
	{ 0x17, 199 },
	{ 0x18, 200  },
	{ -1, LAST_NDIV}
};

/**
 * @brief  identify correct charge pump value regarding ndiv parameter
 * @param      ndiv: ndiv parameter of PLL
 * @retval <0 if not consistent ndiv, else charge pump value to set
 */
int src_charge_pump_conversion(unsigned int ndiv)
{
unsigned int idx;

	for (idx = 0;
	     (sta_src_cp_table[idx].ndiv_range != LAST_NDIV) &&
	     (sta_src_cp_table[idx].ndiv_range < ndiv);
	     idx++)
		;

	return sta_src_cp_table[idx].cp_value;
}

/**
  * @brief  changes the state of a given clock
  * @param	src: current SRC M3 device
  * @param	pclk: the clock to be updated
  * @param	enable: true to enable it, false otherwise
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pclk_change_state(t_src_m3 *src, unsigned int pclk, bool enable)
{
	if (pclk >= MAX_SRCM3_2_CLK)
		return -EINVAL;

	if (enable) {
		if (pclk < MAX_SRCM3_1_CLK)
			src->pcken1 |= BIT(pclk);
		else
			src->pcken2 |= BIT(pclk - MAX_SRCM3_1_CLK);
	} else {
		if (pclk < MAX_SRCM3_1_CLK)
			src->pckdis1 |= BIT(pclk);
		else
			src->pckdis2 |= BIT(pclk - MAX_SRCM3_1_CLK);
	}

	return 0;
}

/* private routine to change all pclks states (enabled/disabled) */
static void srcm3_pclk_change_state_all(t_src_m3 *src, bool enable)
{
	if (enable) {
		src->pcken1 = 0xFFFFFFFF;
		src->pcken2 = 0xFFFFFFFF;
	} else {
		src->pckdis1 = 0xDFDFFFFF; /* keep debug clocks enabled */
		src->pckdis2 = 0xFFFFFFFF;
	}
}

/* private routine to change pll state (enabled/disabled) */
static int srcm3_pll_change_state(t_src_m3 *src, unsigned int pll, bool enable)
{
	switch(pll) {
	case PLL2:
		/*
		 * PLL2 case: allow PLL2EN override by setting PLL2_OVER;
		 */
		if (enable)
			src->pllctrl.bit.pll2over = 1;
		src->pllctrl.bit.pll2en = enable;
		break;
	case PLL3:
		src->pllctrl.bit.pll3en = enable;
		break;
	case PLL4:
		src->pllctrl.bit.pll4en = enable;
		break;
	case PLLD:
		src->pllctrl.bit.pllden = enable;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
  * @brief  disables the given pclk
  * @param	src: current SRC M3 device
  * @param  pclk
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pclk_disable(t_src_m3 *src, unsigned int pclk)
{
	return srcm3_pclk_change_state(src, pclk, false);
}

/**
  * @brief  enables the given pclk
  * @param	src: current SRC M3 device
  * @param  pclk
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pclk_enable(t_src_m3 *src, unsigned int clk)
{
	return srcm3_pclk_change_state(src, clk, true);
}

/**
  * @brief  disables all pclk
  * @param	src: current SRC M3 device
  * @retval None
  */
void srcm3_pclk_disable_all(t_src_m3 *src)
{
	srcm3_pclk_change_state_all(src, false);
}

/**
  * @brief  enables all pclk
  * @param	src: current SRC M3 device
  * @retval None
  */
void srcm3_pclk_enable_all(t_src_m3 *src)
{
	srcm3_pclk_change_state_all(src, true);
}

/**
  * @brief  disables the given pll
  * @param	src: current SRC M3 device
  * @param  pll
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pll_disable(t_src_m3 *src, unsigned int pll)
{
	return srcm3_pll_change_state(src, pll, false);
}

/**
  * @brief  enables the given pll
  * @param	src: current SRC M3 device
  * @param  pll
  * @retval 0 if no error, not 0 otherwise
  */
inline int srcm3_pll_enable(t_src_m3 *src, unsigned int pll)
{
	return srcm3_pll_change_state(src, pll, true);
}

/**
  * @brief  provides status of the given pll
  * @param	src: current SRC M3 device
  * @param  pll
  * @retval true if enabled, false otherwise
 */
bool srcm3_is_pll_enabled(t_src_m3 *src, unsigned int pll)
{
	switch (pll) {
	case PLL2:
		return src->pllctrl.bit.pll2stat;
	case PLL3:
		return src->pllctrl.bit.pll3stat;
	case PLL4:
		return src->pllctrl.bit.pll4stat;
	case PLLD:
		return src->pllctrl.bit.plldstat;
	}

	return false;
}

/**
  * @brief  updates frequency parameters for the given pll
  * @param	src: current SRC M3 device
  * @param	pll refer to @srcm3_plls
  * @param	idf: input division factor (0 = 1, 1 = 1, 2 = 2 .. 7) applicable
  * for every pll
  * @param	ndiv: applicable for every pll (8..200)
  * @param	cp: charge pump value (0 = 0.5, 1 = 1.0, 2 = 2.5 .. 16) applicable
  * for PLL2 and PLL3
  * @param	odf: output divison factor (0..15) applicable for PLL2
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pll_update_freq(t_src_m3 *src, unsigned int pll, int idf,
		int ndiv, int cp, int odf)
{
	switch (pll) {
	case PLL2: /* PLL2 has ODF, STROBE_ODF, CP, NDIV and IDF fields */

		if (odf > 0) {
			/*
			 * ODF is applied when STROBE is low, ODF values are then
			 * loaded when STROBE is high. After that ODF can be set
			 * again to low.
			 */

			/* set STROBE to low if needed */
			if (src->pll2fctrl.bit.odf_strobe)
				src->pll2fctrl.bit.odf_strobe = 0;

			src->pll2fctrl.bit.odf = odf & 0xf;
			src->pll2fctrl.bit.odf_strobe = 1;
		}

		if (cp > 0)
			src->pll2fctrl.bit.cp = cp & 0x1f;
		else
			src->pll2fctrl.bit.cp = src_charge_pump_conversion(ndiv) & 0x1f;

		src->pll2fctrl.bit.idf = idf & 0x7;
		src->pll2fctrl.bit.ndiv = ndiv;

		break;
	case PLL3: /* PLL3 has CP, NDIV and IDF fields */

		if (cp > 0)
			src->pll3fctrl.bit.cp = cp & 0x1f;
		else
			src->pll3fctrl.bit.cp = src_charge_pump_conversion(ndiv) & 0x1f;

		src->pll3fctrl.bit.idf = idf & 0x7;
		src->pll3fctrl.bit.ndiv = ndiv & 0xff;

		/* set STROBE to low if needed */
		if (src_a7_regs->scpll3fctrl.bit.odf_strobe)
			src_a7_regs->scpll3fctrl.bit.odf_strobe = 0;

		src_a7_regs->scpll3fctrl.bit.odf = odf & 0xf;
		src_a7_regs->scpll3fctrl.bit.odf_strobe = 1;

		break;
	case PLL4: /* PLL3 has CP, NDIV, IDF and ODF fields */

		if (cp > 0)
			src->pll4fctrl.bit.cp = cp & 0x1f;
		else
			src->pll4fctrl.bit.cp = src_charge_pump_conversion(ndiv) & 0x1f;

		src->pll4fctrl.bit.idf = idf & 0x7;
		src->pll4fctrl.bit.ndiv = ndiv & 0xff;

		if (odf > 0) {
			/*
			 * ODF is applied when STROBE is low, ODF values are then
			 * loaded when STROBE is high. After that ODF can be set
			 * again to low.
			 */
			if (src->pll4fctrl.bit.strobe_odf)
				src->pll4fctrl.bit.strobe_odf = 0;
			src->pll4fctrl.bit.odf = odf & 0xf;
			src->pll4fctrl.bit.strobe_odf = 1;
		}

		break;
	case PLLD: /* PLLD has NDIV field */

		src->plldfctrl.bit.ndiv = ndiv & 0xff;
		src_a7_regs->scplldfctrl.bit.pll_odf = odf & 0xf;

		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
  * @brief  updates frequency parameters for the pll2
  * @param	src: current SRC M3 device
  * @param	idf: input division factor (0 = 1, 1 = 1, 2 = 2 .. 7)
  * @param	ndiv: 8..200
  * @param	cp: charge pump value (0 = 0.5, 1 = 1.0, 2 = 2.5 .. 16)
  * @param	odf: output divison factor (0..15)
  * @retval 0 if no error, not 0 otherwise
  */
static inline int srcm3_pll2_update_freq(t_src_m3 *src, int idf, int ndiv, int cp,
		int odf)
{
	/* Fvco2 = 2 x ndiv x Fxtal/idf */
	return srcm3_pll_update_freq(src, PLL2, idf, ndiv, cp, odf);
}

/**
  * @brief  updates frequency parameters for the pll3
  * @param	src: current SRC M3 device
  * @param	idf: input division factor (0 = 1, 1 = 1, 2 = 2 .. 7)
  * @param	ndiv: 8..200
  * @param	cp: charge pump value (0 = 0.5, 1 = 1.0, 2 = 2.5 .. 16)
  * @param	odf: output divison factor (0..15)
  * @retval 0 if no error, not 0 otherwise
  */
static inline int srcm3_pll3_update_freq(t_src_m3 *src, int idf, int ndiv, int cp,
		int odf)
{
	return srcm3_pll_update_freq(src, PLL3, idf, ndiv, cp, odf);
}

/**
  * @brief  updates frequency parameters for the pll4
  * @param	src: current SRC M3 device
  * @param	idf: input division factor (0 = 1, 1 = 1, 2 = 2 .. 7)
  * @param	ndiv: 8..200
  * @param	cp: charge pump value (0 = 0.5, 1 = 1.0, 2 = 2.5 .. 16)
  * @param	odf: output divison factor (0..15)
  * @retval 0 if no error, not 0 otherwise
  */
static inline int srcm3_pll4_update_freq(t_src_m3 *src, int idf, int ndiv, int cp,
		int odf)
{
	return srcm3_pll_update_freq(src, PLL4, idf, ndiv, cp, odf);
}

/**
  * @brief  updates frequency parameters for the plld (PLL DDR)
  * @param	src: current SRC M3 device
  * @param	ndiv: 8..200
  * @param	odf: output division factor
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_plld_update_freq(t_src_m3 *src, int ndiv, int odf)
{
	return srcm3_pll_update_freq(src, PLLD, -1, ndiv, -1, odf);
}

/**
  * @brief  returns the current REMAP configuration
  * @param	src: current SRC M3 device
  * @param	None
  * @retval REMAP configuration, see @srcm3_remap for details;
  */
unsigned int srcm3_get_remap(t_src_m3 *src)
{
	switch (get_soc_id()) {
	default:
	case SOCID_STA1195:
	case SOCID_STA1295:
	case SOCID_STA1275:
		return ((src->resstat.bit_sta1x95.remap1 << 1)
				| src->resstat.bit_sta1x95.remap0);
	case SOCID_STA1385:
		return ((src->resstat.bit_sta1385.remap2 << 2)
				| (src->resstat.bit_sta1385.remap1 << 1)
				| src->resstat.bit_sta1385.remap0);
	}
}

/**
 * @brief  return true if reset origin is a WDG
 * @param	src: current SRC M3 device
 * @param	None
 * @retval true if reset origin is a WDG;
 */
bool srcm3_wdg_reset(t_src_m3 *src)
{
	switch (get_soc_id()) {
	default:
	case SOCID_STA1195:
	case SOCID_STA1295:
	case SOCID_STA1275:
		return src->resstat.bit_sta1x95.wdg_rst_stat;
	case SOCID_STA1385:
		return src->resstat.bit_sta1385.wdg_rst_stat;
	}
}

/**
  * @brief  performs a reset by setting MODECR to EXTERNAL OSC, halting CPU
  * @param	src: current SRC M3 device
  * and doing a SW reset
  * @param	None
  * @retval None
  */
void srcm3_reset(t_src_m3 *src)
{
	/* Set MODECR to EXTERNAL to be able to reboot */
	src->ctrl.bit.mode_cr = SRCM3_MODECR_EXTOSC;

	/* Make sure we reached external osc mode */
	while(src->ctrl.bit.mode_status != SRCM3_MODECR_EXTOSC);

	/**
	 * Before triggering software reset request, restoring another
	 * HCLKDIV value than one that is set by ROM is required to make
	 * sure that HCLKDIV value that will be programmed by the ROM
	 * following a software reset will be really taken into account
	 * by the hardware.
	 *
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
	src_a7_regs->scclkdivcr.bit.hclk_div = hclk_div_rom + 1;

	/* Do software reset */
	if (get_soc_id() == SOCID_STA1385)
		src->resstat.bit_sta1385.m3_soft_rst_req = SRCM3_RESSTAT_SWRESREQM3;
	else
		src->resstat.bit_sta1x95.m3_soft_rst_req = SRCM3_RESSTAT_SWRESREQM3;

	/* Loop forever, waiting for SoC to reset */
	while(1);
}

/**
 * @brief Set the Mode Control State Machine of SRC_M3
 */
void srcm3_set_mode(t_src_m3 *src, int mode)
{
	src->ctrl.bit.mode_cr = mode;

	/* Wait until we enter mode */
	while(src->ctrl.bit.mode_status != mode);
}

/**
 * @brief Configure the Debug Mode for A7 cpus, stm & cti
 *
 * To do: a7, audio tdo/tdi muxing mngt
 */
void srcm3_enable_debug(t_src_m3 *src, bool cpu_on, bool cti_on, bool stm_on)
{
	if (cpu_on) {
		/* cpu0 global invasive debug enable */
		src->debug.bit.cpu0_dbgen = 0;
		/* cpu1 global invasive debug enable */
		src->debug.bit.cpu1_dbgen = 0;
		/* cpu0/1 global non-invasive (trace) debug enable */
		src->debug.bit.cpu_niden = 1;
		/* cpu0/1 secure priviled invasive (JTAG) debug enable */
		src->debug.bit.cpu_spiden = 1;
		/* cpu0/1 secure priviled non-invasive (trace) debug enable */
		src->debug.bit.cpu_spinden = 1;
	} else {
		/* cpu0 global invasive debug enable */
		src->debug.bit.cpu0_dbgen = 1;
		/* cpu1 global invasive debug enable */
		src->debug.bit.cpu1_dbgen = 1;
		/* cpu0/1 global non-invasive (trace) debug enable */
		src->debug.bit.cpu_niden = 0;
		/* cpu0/1 secure priviled invasive (JTAG) debug enable */
		src->debug.bit.cpu_spiden = 0;
		/* cpu0/1 secure priviled non-invasive (trace) debug enable */
		src->debug.bit.cpu_spinden = 0;
	}

	if (cti_on)
		/* system cross trigger interface */
		src->debug.bit.sys_cti_en = 0;
	else
		src->debug.bit.sys_cti_en = 1;

	if (stm_on) {
		/* system stm tpiu enable */
		src->debug.bit.sys_stm_tpiu_en = 0;
		src->debug.bit.stm_niden = 1;
		src->debug.bit.stm_spiden = 1;
		src->debug.bit.stm_spniden = 1;
		src->debug.bit.stm_dbgswen = 0;
		/* src->debug.bit.stm_qreqn = 1; */
	} else {
		src->debug.bit.sys_stm_tpiu_en = 1;
		src->debug.bit.stm_niden = 0;
		src->debug.bit.stm_spiden = 0;
		src->debug.bit.stm_spniden = 0;
		src->debug.bit.stm_dbgswen = 1;
		/* src->debug.bit.stm_qreqn = 0; */
	}
}

/**
  * @brief  initialize SRC M3 controller
  * @param	src: current SRC M3 device
  */
void srcm3_init(t_src_m3 *src)
{
	srcm3_pll_disable(src, PLL2);
	if (get_soc_id() == SOCID_STA1385)
		/* Let's configure PLL2 @ 1248 */
		srcm3_pll2_update_freq(src, 1, 24, -1, 13);
	else
		/* Let's configure PLL2 @ 1228.8 */
		srcm3_pll2_update_freq(src, 5, 128, -1, 13);
	srcm3_pll_enable(src, PLL2);

    while (!srcm3_is_pll_enabled(src, PLL2));

	if (get_soc_id() != SOCID_STA1385) {
		srcm3_pll_disable(src, PLL3);
		/* Let's configure PLL3 @ 1228.8 */
		srcm3_pll3_update_freq(src, 5, 128, -1, 5);
		srcm3_pll_enable(src, PLL3);

		while (!srcm3_is_pll_enabled(src, PLL3));
	}

	srcm3_pll_disable(src, PLL4);
	if (get_soc_id() == SOCID_STA1385)
		/* Let's configure PLL4 @ 998.4 */
		srcm3_pll4_update_freq(src, 5, 96, 0, 11);
	else
		/* Let's configure PLL4 @ 864, and PLL4PHY @ 240*/
		srcm3_pll4_update_freq(src, 1, 20, 0, 4);
	srcm3_pll_enable(src, PLL4);

    while (!srcm3_is_pll_enabled(src, PLL4));

	srcm3_pll_disable(src, PLLD);
	if (get_soc_id() == SOCID_STA1385)
#ifdef LPDDR2
		/* using lpDDR2 we use a ddrclkout @400MHz */
		srcm3_plld_update_freq(src, 30, 1);
#else
		/* using DDR3 we use a ddrclkout @520MHz */
		srcm3_plld_update_freq(src, 40, 1);
#endif
	else
		/*
		 * For STA1195, STA1295 and STA1275 we configure PLLD to
		 * half its nominal frequency before lifting the reset.
		 */
		srcm3_plld_update_freq(src, 28, 2);
	srcm3_pll_enable(src, PLLD);

    while (!srcm3_is_pll_enabled(src, PLLD));

	/* set peripherals dividers */

	src->clkdivcr.bit.sqi = CLCKDIV_SQI_PLL2FVCOBY2_BY3;                    /* 204.800 MHz */

	src->clkdivcr.bit.audio_ssm_clk = CLKDIV_AUDIO_SSM_CLK_PLL2VCOBY2_BY5;	/* 122.88 MHz */
	src->clkdivcr.bit.audio_ssf_512 = CLKDIV_AUDIO_SSF_512_PLL2VCOBY2_BY25; /* 24.576 MHz */

	src->clkdivcr.bit.sdmmc0 = CLKDIV_SDIO_PLL2PHI;						/* 94.523 MHz */
	src->clkdivcr.bit.sdmmc1 = CLKDIV_SDMMC_PLL2PHI;						/* 94.523 MHz */
	src->clkdivcr.bit.sdmmc2 = CLKDIV_SDMMC_PLL2PHI;						/* 94.523 MHz */

	src->clkdivcr.bit.gfx_sel = CLKDIV_GFX_PLL4VCOBY2;						/* 480 MHz */
	src->clkdivcr.bit.vdo_sel = CLKDIV_VDO_PLL4PHI;							/* PLL4 Phi : 240MHz */

	/*
	 * Updating this divider value can will change the Cortex-M3 frequency:
	 * FVOBY2/3 => 204.8 MHz
	 * FVOBY2/4 => 153.6 MHz
	 */
	src->clkdivcr.bit.can_ss = SRCM3_CLKDIV_CAN_PLL2VCOBY2_BY3;
}

