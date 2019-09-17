/**
 * @file sta_platform.h
 * @brief This file provides all platform related functions headers
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_PLATFORM_H_
#define _STA_PLATFORM_H_

#include "sta_common.h"

struct sta_clk {
	uint8_t id;
	bool state;
};

#define UNUSED	false
#define USED	true

/**
 * @brief	Platform early init (Get board console port for example)
 */
int platform_early_init(struct sta *context);

/**
 * @brief	Identifies the SoC identification and fills the information
 * accordingly.
 * @return	0 if no error, not 0 otherwise
 */
int m3_get_soc_id(void);

/**
 * @brief	Identifies the board identification
 * @return	0 if no error, not 0 otherwise
 */
int m3_get_board_id(void);

/**
 * @brief	Recopy M3 (ESRAM) boot info shared data in DDR for AP usage
 * @return	NA
 */
void platform_recopy_bootinfo_in_ddr(bool bi_only);

void platform_welcome_message(struct sta *context);

/**
  * @brief  Reset ETH0 PHY for next boot stages
  *         to be called only in Xloaders
  *
  * @retval None
  */
void platform_eth0_reset_phy(void);

/**
 * @brief  enables used clocks. Actually parses the list of available clocks
 * and enable them if they are used. This allows next boot stages (Linux) to
 * not take care about the unused clocks management.
 *
 * @retval 0 if no error, not 0 otherwise
 */
int sta_enable_clocks(t_src_m3 *src_m3, t_src_a7 *src_a7);

/**
 * @brief	Identifies the mxtal
 * @return	>0 if no error, < 0 otherwise
 */
int m3_get_mxtal(void);

#endif /* _STA_PLATFORM_H_ */
