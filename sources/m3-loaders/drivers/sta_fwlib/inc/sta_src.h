/**
 * @file sta_src.h
 * @brief System Reset Controller header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_SRC_H_
#define _STA_SRC_H_

#include "utils.h"

#include "sta_type.h"
#include "sta_map.h"

#define PLL1 0 /**< PLL1 */
#define PLL2 1 /**< PLL2 will feed FDCAN, USB, SSP, MSP, DSP, etc. */
#define PLL3 2 /**< PLL3 */
#define PLL4 3 /**< PLL4 will feed the GFX and video decode */
#define PLLD 5 /**< PLLD will feed the PLL-DDR */


/* SRCM3_CR bits */
#define SRCM3_MODECR_INTOSC						1
#define SRCM3_MODECR_EXTOSC						2
#define SRCM3_MODECR_NORMAL						4

/* SRC M3 specific */

enum srcm3_clks {
	CLCD_CLK = 0,
	SSPCLK,
	UART0CLK,
	SDICLK,
	I2C0CLK,
	I2C1CLK,
	UART1CLK,
	SQI_CLK,
	AUDIO_SS_512,
	AUDIO_SS_MCLK,
	DSP_CLK,
	SSP2CLK,
	UART2CLK,
	SDICLK1,
	SSP1CLK,
	I2C2CLK,
	UART3CLK,
	MSP0CLK,
	MSP1CLK,
	MSP2CLK,
	A7CNTCLK,
	A7_DBGCLK,
	SDICLK2,
	C3RNGCLK,
	SAFMEMCLK,
	DDRCLK,
	GFXCLK2X,
	VDOCLK,
	A7PCLK,
	A7CLKEXT2F,
	TSENSECLK,
	CANFDCLK,
	MAX_SRCM3_1_CLK,
	ETHPTPCLK = MAX_SRCM3_1_CLK,
	FDCANPCLK,
	FRAYPECLK,
	/* unused */
	ETH1PTPCLK = FRAYPECLK + 2,
	WLANCLK,
	ECULTRACLK,
	AESCLK,
	SHACLK,
	MAX_SRCM3_2_CLK,
};

#define SRCM3_A5_REMAP_BOOT_FROM_NAND_MMC1	0
#define SRCM3_A5_REMAP_BOOT_FROM_SQI0_MMC2	1
#define SRCM3_A5_REMAP_BOOT_FROM_OTP		2

#define SRCM3_TC3P_REMAP_BOOT_FROM_NAND_MMC1	0
#define SRCM3_TC3P_REMAP_BOOT_FROM_SQI0_MMC1	1
#define SRCM3_TC3P_REMAP_BOOT_FROM_USBDFU_UART	2
#define SRCM3_TC3P_REMAP_BOOT_FROM_MMC0_MMC1	3
#define SRCM3_TC3P_REMAP_BOOT_FROM_OTP		4

#define SRCM3_DBGCFG_M3_ONLY			0
#define SRCM3_DBGCFG_M3_R4			1
#define SRCM3_DEBUG_DBGCFG_DSPTDI_TDO		0
#define SRCM3_DEBUG_DBGCFG_DSPTDI_JTAG		1
#define SRCM3_DEBUG_DBGCFG_DSPTMS_TIED		0
#define SRCM3_DEBUG_DBGCFG_DSPTMS_JTAG		1

#define SRCM3_DBGCFG_JTAGSEL_R4			0
#define SRCM3_DBGCFG_JTAGSEL_M3_R4_CHAINED	1
#define SRCM3_DBGCFG_JTAGSEL_M3_R4_ALT1		2
#define SRCM3_DBGCFG_JTAGSEL_M3_R4_ALT2		3

#define SRCM3_DBGCFG_TDOSEL_FROM_M3_R4		0
#define SRCM3_DBGCFG_TDOSEL_FROM_R4		1
#define SRCM3_DBGCFG_TDOSEL_FROM_M3		2
#define SRCM3_DBGCFG_TDOSEL_FROM_DSP		3

/* SRCM3_DEBUG bits */
#define SRCM3_CR_MTU_TIMERS_MASK		0xFF

/**
 * @brief  identify correct charge pump value regarding ndiv parameter
 * @param      ndiv: ndiv parameter of PLL
 * @retval <0 if not consistent ndiv, else charge pump value to set
 */
int src_charge_pump_conversion(unsigned int ndiv);

/**
  * @brief  changes the state of a given clock
  * @param	src: current SRC M3 device
  * @param	pclk: the clock to be updated
  * @param	enable: true to enable it, false otherwise
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pclk_change_state(t_src_m3 *src, unsigned int pclk, bool enable);

/**
  * @brief  returns the current REMAP configuration
  * @param	src: current SRC M3 device
  * @retval REMAP configuration, see @srcm3_remap for details;
  */
unsigned int srcm3_get_remap(t_src_m3 *src);

/**
 * @brief  return true if reset origin is a WDG
 * @param	src: current SRC M3 device
 * @param	None
 * @retval true if reset origin is a WDG;
 */
bool srcm3_wdg_reset(t_src_m3 *src);

/**
  * @brief  performs a reset by setting MODECR to EXTERNAL OSC, halting CPU
  * @param	src: current SRC M3 device
  * and doing a SW reset
  * @param	None
  * @retval None
  */
void srcm3_reset(t_src_m3 *src);

/**
 * @brief Set the Mode Control State Machine of SRC_M3
 */
void srcm3_set_mode(t_src_m3 *src, int mode);

/**
 * @brief Configure the Debug Mode for A7 cpus, stm & cti
 */
void srcm3_enable_debug(t_src_m3 *src, bool cpu_on, bool cti_on, bool stm_on);

/**
  * @brief  sets MTU timers reference clocks (MXTAL or MXTAL/8)
  * @param	src: current SRC M3 device
  * @param	ref_clks: a bitstream describing for each MTU the reference clock to be used
  * 0b0: ref clk is MXTAL
  * 0b1: ref clk is MXTAL divided by 8
  * Bit[11] - Timer 3 of MTU_1
  * Bit[10] - Timer 2 of MTU_1
  * Bit[9]  - Timer 1 of MTU_1
  * Bit[8]  - Timer 0 of MTU_1
  * Bit[7]  - Timer 3 of MTU_0
  * Bit[6]  - Timer 2 of MTU_0
  * Bit[5]  - Timer 1 of MTU_0
  * Bit[4]  - Timer 0 of MTU_0
  * Bit[0-3]  Unused
  * @retval None
  */
static inline void srcm3_set_mtu_timers_clk(t_src_m3 *src, uint32_t ref_clks)
{
	src->ctrl.bit.timersel = ref_clks & SRCM3_CR_MTU_TIMERS_MASK;
}

/**
  * @brief  initialize SRC M3 controller
  * @param	src: current SRC M3 device
  */
void srcm3_init(t_src_m3 *src);

/**
  * @brief  enables the given pclk
  * @param	src: current SRC M3 device
  * @param  pclk
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pclk_enable(t_src_m3 *src, unsigned int clk);

/**
  * @brief  disables the given pclk
  * @param  src: current SRC M3 device
  * @param  pclk
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pclk_disable(t_src_m3 *src, unsigned int pclk);

/**
  * @brief  disables all pclk
  * @param	src: current SRC M3 device
  * @retval None
  */
void srcm3_pclk_disable_all(t_src_m3 *src);

/**
  * @brief  enables all pclk
  * @param	src: current SRC M3 device
  * @retval None
  */
void srcm3_pclk_enable_all(t_src_m3 *src);

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
		int ndiv, int cp, int odf);

/**
  * @brief  disables the given pll
  * @param	src: current SRC M3 device
  * @param  pll
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pll_disable(t_src_m3 *src, unsigned int pll);

/**
  * @brief  enables the given pll
  * @param	src: current SRC M3 device
  * @param  pll
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_pll_enable(t_src_m3 *src, unsigned int pll);

/**
  * @brief  provides status of the given pll
  * @param	src: current SRC M3 device
  * @param  pll
  * @retval true if enabled, false otherwise
 */
bool srcm3_is_pll_enabled(t_src_m3 *src, unsigned int pll);

/**
  * @brief  updates frequency parameters for the plld (PLL DDR)
  * @param	src: current SRC M3 device
  * @param	ndiv: 8..200
  * @param	odf: output division factor
  * @retval 0 if no error, not 0 otherwise
  */
int srcm3_plld_update_freq(t_src_m3 *src, int ndiv, int odf);
#endif /* _STA_SRC_H_ */
