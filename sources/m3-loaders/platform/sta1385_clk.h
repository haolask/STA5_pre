/**
 * @file sta1385_clk.h
 * @brief This file provides all the STA1385 specific clocks declarations
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "sta_src.h"
#include "sta_src_a7.h"
#include "sta_platform.h"

static const struct sta_clk sta1385_clk_m3[] = {
	{ CLCD_CLK,		UNUSED },
	{ SSPCLK,		UNUSED },
	{ UART0CLK,		UNUSED },
	{ SDICLK,		UNUSED },
	{ I2C0CLK,		UNUSED },
	{ I2C1CLK,		UNUSED },
	{ UART1CLK,		USED },
	{ SQI_CLK,		USED },
	{ AUDIO_SS_512,		USED },
	{ AUDIO_SS_MCLK,	USED },
	{ DSP_CLK,		UNUSED },
	{ SSP2CLK,		UNUSED },
	{ UART2CLK,		USED },
	{ SDICLK1,		UNUSED },
	{ SSP1CLK,		UNUSED },
	{ I2C2CLK,		UNUSED },
	{ UART3CLK,		UNUSED },
	{ MSP0CLK,		UNUSED },
	{ MSP1CLK,		UNUSED },
	{ MSP2CLK,		UNUSED },
	{ A7CNTCLK,		USED },
	{ A7_DBGCLK,		USED },
	{ SDICLK2,		USED },
	{ C3RNGCLK,		UNUSED },
	{ SAFMEMCLK,		USED },
	{ DDRCLK,		USED },
	{ GFXCLK2X,		UNUSED },
	{ VDOCLK,		UNUSED },
	{ A7PCLK,		USED },
	{ A7CLKEXT2F,		USED },
	{ TSENSECLK,		UNUSED },
	{ CANFDCLK,		USED },
	{ ETHPTPCLK,		UNUSED },
	{ FDCANPCLK,		UNUSED },
};

static const struct sta_clk sta1385_clk_ap[] = {
	{ HCLKDMA0,		UNUSED },
	{ HCLKDMA1,		UNUSED },
	{ HCLKFSMC,		USED },
	{ HCLKSSP0,		USED },
	{ HCLKCLCD,		UNUSED },
	{ PCLKSSP1,		UNUSED },
	{ PCLKSSP2,		UNUSED },
	{ PCLKSDI0,		UNUSED },
	{ PCLKSDMMC1,		USED },
	{ PCLKI2C1,		UNUSED },
	{ PCLKI2C2,		UNUSED },
	{ PCLKUART1,		USED },
	{ PCLKUART2,		USED },
	{ PCLKUART3,		UNUSED },
	{ PCLKHSEM,		USED },
	{ HCLKGAE,		UNUSED },
	{ HCLKVIP,		UNUSED },
	{ HCLKUSB,		UNUSED },
	{ PCLK_APB_REG,		USED },
	{ PCLKSARADC,		UNUSED },
	{ PCLKSMSP0,		UNUSED },
	{ PCLKSMSP1,		UNUSED },
	{ PCLKSMSP2,		UNUSED },
	{ PCLKCDSUBSYS,		UNUSED },
	{ HCLKGFX,		UNUSED },
	{ PCLKETH,		UNUSED },
	{ HCLKC3,		UNUSED },
	{ ACLKDDRC,		USED },
	{ ACLKA7,		USED },
	{ PCLKI2C0,		USED },
	{ PCLKUART0,		UNUSED },
	{ PCLKSDMMC2,		USED },
	{ TRACECLK_M3,		USED },
	{ TRACECLK_A7,		USED },
	{ ATCLK_A7,		USED },
	{ ATCLK_DBG,		USED },
	{ PCLKDDRCTRL,		USED },
	{ ACLKGFX,		UNUSED },
};
