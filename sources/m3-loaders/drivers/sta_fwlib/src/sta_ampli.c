/**
 * @file sta_ampli.c
 * @brief This file provides a set of functions needed to manage 
 *        the TDA7569 audio amplifier.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */

#include <string.h>
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"

#include "sta_gpio.h"
#include "sta_i2c_service.h"
#include "sta_pinmux.h"
#include "sta_platform.h"

/* Instruction Registers */
/* TDA75XX IB1 */
#define IB1_MUTE_THRESHOLD_HIGH      BIT(7) /* voltage mute threshold high */
#define IB1_MUTE_THRESHOLD_LOW       (0 << 7) /* voltage mute threshold low */
#define IB1_DIAGNOSTIC_ENABLE        BIT(6) /* Diagnostic enable */
#define IB1_DIAGNOSTIC_DISABLE       (0 << 6) /* Diagnostic disable */
#define IB1_OFFSET_DETECTION_ENABLE  BIT(5) /* Offset Detection enable */
#define IB1_OFFSET_DETECTION_DISABLE (0 << 5) /* Offset Detection disable */
#define IB1_FRONT_GAIN_26DB          (0 << 4) /* Front (CH1, CH3) Gain 26dB */
#define IB1_FRONT_GAIN_16DB          BIT(4) /* Front (CH1, CH3) Gain 16dB */
#define IB1_REAR_GAIN_26DB           (0 << 3) /* Rear (CH2, CH4) Gain 26dB */
#define IB1_REAR_GAIN_16DB           BIT(3) /* Rear (CH2, CH4) Gain 16dB */
#define IB1_FRONT_UNMUTE             BIT(2) /* Unmute front channels */
#define IB1_FRONT_MUTE               (0 << 2) /* Mute front channels */
#define IB1_REAR_UNMUTE              BIT(1) /* Unmute rear channels */
#define IB1_REAR_MUTE                (0 << 1) /* Mute rear channels */
#define IB1_CD_10                    BIT(0) /* Clip detection 10% */
#define IB1_CD_2                     (0 << 0) /* Clip detection 2% */

/* TDA75XX IB2 */
#define IB2_CURRENT_THRESHOLD_LOW    BIT(7) /* <125mA Open, >250mA Load */
#define IB2_CURRENT_THRESHOLD_HIGH   (0 << 7) /* <250mA Open, >500mA Load */
#define IB2_FAST_MUTING_TIME         BIT(5) /* Fast muting time */
#define IB2_NORMAL_MUTING_TIME       (0 << 5) /* Normal muting time */
#define IB2_STANDBY_OFF              BIT(4) /* Standby off, amp working */
#define IB2_STANDBY_ON               (0 << 4) /* Standby on, amp not working */
#define IB2_LINE_DRIVER_DIAG         BIT(3) /* Line driver mode diagnostic */
#define IB2_POWER_AMPLIFIER_DIAG     (0 << 3) /* Power mode diagnostic */
#define IB2_CURRENT_DIAG_ENABLED     BIT(2) /* Current diagnostic enabled */
#define IB2_CURRENT_DIAG_DISABLED    (0 << 2) /* Current diagnostic disabled */
#define IB2_RIGHT_AMPLI_EFFICIENT    BIT(1) /* Right Ch in efficiency mode */
#define IB2_RIGHT_AMPLI_NORMAL       (0 << 1) /* Right Ch in standard mode */
#define IB2_LEFT_AMPLI_EFFICIENT     BIT(0) /* Left Ch in efficiency mode */
#define IB2_LEFT_AMPLI_NORMAL        (0 << 0) /* Left Ch in standard mode */


#define I2C_CLOCK 	51200000
#define TDA7569_ADDR 	0x6D
#define TDA7569_PORT	i2c2_regs
#define SBY_GPIO	S_GPIO(6)

/**
  * @brief  Turn on TDA7569.
  * @param  None
  * @retval None
  */
int ampli_poweron(void)
{
	struct i2c_com_handler_s *i2c_h;
	portTickType timeout = 1000;
	uint8_t ireg[2];
	struct gpio_config pin;
	int ret;

	pin.direction   = GPIO_DIR_OUTPUT;
	pin.mode        = GPIO_MODE_SOFTWARE;
	pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
	pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
	gpio_set_pin_config(SBY_GPIO, &pin);
	gpio_clear_gpio_pin(SBY_GPIO);

	if (i2c_service_init(I2C_CLOCK))
		goto i2c_close;
	if (i2c_open_port(TDA7569_PORT, 0, 0, I2C_BUSCTRLMODE_MASTER))
		goto i2c_close;
	i2c_h = i2c_create_com(TDA7569_PORT, I2C_FAST_MODE, TDA7569_ADDR);
	if (!i2c_h)
		goto i2c_close;

	ireg[0] = IB1_MUTE_THRESHOLD_LOW
		| IB1_DIAGNOSTIC_ENABLE	| IB1_OFFSET_DETECTION_ENABLE
		| IB1_FRONT_GAIN_26DB | IB1_REAR_GAIN_26DB
		| IB1_FRONT_UNMUTE | IB1_REAR_UNMUTE
		| IB1_CD_2;

	ireg[1] = IB2_CURRENT_THRESHOLD_HIGH
		| IB2_NORMAL_MUTING_TIME | IB2_STANDBY_OFF
		| IB2_POWER_AMPLIFIER_DIAG | IB2_CURRENT_DIAG_ENABLED
		| IB2_RIGHT_AMPLI_NORMAL | IB2_LEFT_AMPLI_NORMAL;

	if (i2c_write(i2c_h, 0, 0, ireg, 2, 2, &timeout)) {
		TRACE_ERR("%s: failed to write\n", __func__);
		goto i2c_close;
	}

	ret = 0;
i2c_close:
	if (i2c_reset_port(TDA7569_PORT)) {
		TRACE_ERR("%s: failed to release I2C\n", __func__);
		return -1;
	}

	return ret;
}


