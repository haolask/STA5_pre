/**
 * @file sta1295_clk.h
 * @brief This file provides all the STA1x95 specific clocks declarations
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "sta_src.h"
#include "sta_src_a7.h"
#include "sta_platform.h"

static const struct sta_clk sta1x95_clk_m3[] = {
	{ CLCD_CLK,		USED },
	{ SSPCLK,		UNUSED },
	{ UART0CLK,		USED },
	{ SDICLK,		USED },
	{ I2C0CLK,		UNUSED },
	{ I2C1CLK,		UNUSED },
	{ UART1CLK,		USED },
	{ SQI_CLK,		USED },
	{ AUDIO_SS_512,		USED },
	{ AUDIO_SS_MCLK,	USED },
	{ DSP_CLK,		UNUSED },
	{ SSP2CLK,		UNUSED },
	{ UART2CLK,		USED },
	{ SDICLK1,		USED },
	{ SSP1CLK,		UNUSED },
	{ I2C2CLK,		UNUSED },
	{ UART3CLK,		USED },
	{ MSP0CLK,		UNUSED },
	{ MSP1CLK,		UNUSED },
	{ MSP2CLK,		UNUSED },
	{ A7CNTCLK,		USED },
	{ A7_DBGCLK,		USED },
	{ SDICLK2,		USED },
	{ C3RNGCLK,		USED },
	{ SAFMEMCLK,		USED },
	{ DDRCLK,		USED },
	{ GFXCLK2X,		USED },
	{ VDOCLK,		UNUSED },
	{ A7PCLK,		USED },
	{ A7CLKEXT2F,		USED },
	{ TSENSECLK,		UNUSED },
	{ CANFDCLK,		UNUSED },
	{ ETHPTPCLK,		UNUSED },
	{ FDCANPCLK,		UNUSED },
};

static const struct sta_clk sta1x95_clk_ap[] = {
	{ HCLKDMA0,		UNUSED },
	{ HCLKDMA1,		UNUSED },
	{ HCLKFSMC,		USED },
	{ HCLKSSP0,		USED },
	{ HCLKCLCD,		USED },
	{ PCLKSSP1,		UNUSED },
	{ PCLKSSP2,		UNUSED },
	{ PCLKSDI0,		UNUSED },
	{ PCLKSDMMC1,		USED },
	{ PCLKI2C1,		UNUSED },
	{ PCLKI2C2,		UNUSED },
	{ PCLKUART1,		UNUSED },
	{ PCLKUART2,		UNUSED },
	{ PCLKUART3,		USED },
	{ PCLKHSEM,		USED },
	{ HCLKGAE,		USED },
	{ HCLKVIP,		UNUSED },
	{ HCLKUSB,		USED },
	{ PCLK_APB_REG,		USED },
	{ PCLKSARADC,		UNUSED },
	{ PCLKSMSP0,		UNUSED },
	{ PCLKSMSP1,		UNUSED },
	{ PCLKSMSP2,		UNUSED },
	{ PCLKCDSUBSYS,		USED },
	{ HCLKGFX,		USED },
	{ PCLKETH,		UNUSED },
	{ HCLKC3,		USED },
	{ ACLKDDRC,		USED },
	{ ACLKA7,		USED },
	{ PCLKI2C0,		USED },
	{ PCLKUART0,		USED },
	{ PCLKSDMMC2,		USED },
	{ TRACECLK_M3,		USED },
	{ TRACECLK_A7,		USED },
	{ ATCLK_A7,		USED },
	{ ATCLK_DBG,		USED },
	{ PCLKDDRCTRL,		USED },
	{ ACLKGFX,		UNUSED },
};
