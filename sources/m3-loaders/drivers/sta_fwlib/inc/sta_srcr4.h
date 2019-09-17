/**
 * @file sta_srcr4.h
 * @brief System Reset Controller R4 header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_SRCR4_H_
#define _STA_SRCR4_H_

#include "utils.h"

#include "sta_type.h"
#include "sta_map.h"

enum srcr4_clks {
	MPMC_CLK = 0,
	HCLCKSMC,
	HCLKDMA1,
	HCLKDMA0,
	HCLKCLCD,
	HCLKSSP1,
	HCLKSSP2,
	PCLKSDI0,
	PCLKSDI1,
	PCLKI2C1,
	PCLKI2C2,
	PCLKUART1,
	PCLKUART2,
	PCLKUART3,
	PCLKHSEM,
	HCLKGAE,
	HCLKVIP,
	HCLKUSB,
	PCLK_APB_REG,
	PCLKSARADC,
	PCLKSMSP0,
	PCLKSMSP1,
	PCLKSMSP2,
	PCLKCDSUBSYS,
	MAX_SRCR4_CLK,
};

/**
  * @brief  disables all pclk
  * @param	src: current SRC R4 device
  * @retval None
  */
void srcr4_pclk_disable_all(t_src_r4 *src);

/**
  * @brief  enables all pclk
  * @param	src: current SRC R4 device
  * @retval None
  */
void srcr4_pclk_enable_all(t_src_r4 *src);

/**
  * @brief  updates frequency parameters for the pll1
  * @param	src: current SRC R4 device
  * @param	pll refer to @srcm3_plls
  * @param	odf: output divison factor (0..15)
  * @retval 0 if no error, not 0 otherwise
  */
int srcr4_pll1_update_freq(t_src_r4 *src, int odf);

/**
  * @brief  updates frequency parameters for the pll3
  * @param	src: current SRC R4 device
  * @param	pll refer to @srcm3_plls
  * @param	odf: output divison factor (0..15)
  * @param	fractl: enable/disable fractionnal control
  * @param	dither: enable/disable dither
  * @retval 0 if no error, not 0 otherwise
  */
int srcr4_pll3_update_freq(t_src_r4 *src, int odf, bool fractl, bool dither);

/**
  * @brief  initialize SRC R4 controller
  * @param	src: current SRC R4 device
  */
void srcr4_init(t_src_r4 *src);

#endif /* _STA_SRCR4_H_ */
