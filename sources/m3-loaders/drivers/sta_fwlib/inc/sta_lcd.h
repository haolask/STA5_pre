/**
 * @file sta_lcd.h
 * @brief This file provides all the LCD definitions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_LCD_H_
#define _STA_LCD_H_

#include "sta_map.h"
#include "sta_common.h"

/**
 * @brief	Initialize LCD controller
 * @return	0 if no error, not 0 otherwise
 */
int lcd_init(void);

/**
 * @brief	Update LCD with image
 * @param	shadow_addr: address of the image
 * @return	0 if no error, not 0 otherwise
 */
int lcd_update(void *shadow_addr);

/**
 * @brief	Helper to detect and select display configuration
 * @return	0 if no error, not 0 otherwise
 */
int lcd_select_display_cfg(void);

/**
 * @brief	Helper to initialize misc and src display registers
 * @return	0 if no error, not 0 otherwise
 */
int lcd_init_misc_src_reg(void);

/**
 * @brief	Get display resolution
 * @param	width: display width
 * @param	height: display height
 * @return	void
 */
void lcd_get_display_resolution(uint32_t *width, uint32_t *height);

#endif /* _STA_LCD_H_ */
