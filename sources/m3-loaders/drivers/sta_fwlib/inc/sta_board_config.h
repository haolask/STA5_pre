/**
 * @file sta_board_config.h
 * @brief This file provides all the board configuration header
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: APG-MID team
 */
#ifndef _STA_BOARD_CONFIG_H_
#define _STA_BOARD_CONFIG_H_


#define HDMI_ERROR 				-1
#define HDMI_OK 				0

#define I2C_ADV7513_CLOCK			51200000
#define I2C_ADV7513_PORT			i2c0_regs

#define I2C_ADV7513_ADDR			0x72

#define I2C_ADV7513_REV_BANK			0xD6
#define I2C_ADV7513_HPD_CTRL_BANK		0xD6
#define I2C_ADV7513_PCKT_MEM_I2C_MAP_BANK	0x45

#define I2C_ADV7513_ENABLE_EXTRA_CFG		0xC0
#define I2C_ADV7513_NEW_PCKT_MEM_I2C_ADDR	0x68


int configure_hdmi(void);
void apply_board_config(struct sta *context);


#endif /*_STA_BOARD_CONFIG_H_*/
