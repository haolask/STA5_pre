/*
 * ALSA SoC driver for STMicroelectronics TDA7569 4 channels amplifier
 *
 * Copyright (C) ST Microelectronics 2014
 *
 * Author:	Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef __TDA75XX_H__
#define __TDA75XX_H__

#define TDA75XX_IBNUM	2
#define TDA75XX_DBNUM	4

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

/* Data Registers */
/* TDA75XX DB1 */
#define DB1_THERMAL_WARNING1         BIT(7) /* Thermal warning Tj =160C */
#define DB1_DIAG_CYCLE_TERMINATED    BIT(6) /* Diag cycle terminated */

/* TDA75XX DB2 */
#define DB2_OFFSET_ACTIVE     BIT(7) /* =IB1(D5) */
#define DB2_CURRENT_ACTIVE    BIT(5) /* =IB2(D2) */

/* TDA7569 DB3 */
#define DB3_STANDBY_STATUS    BIT(7) /* =IB2(D4) */
#define DB3_DIAG_STATUS       BIT(6) /* =IB1(D6) */

/* TDA7569 DB4 */
#define DB4_THERMAL_WARNING2  BIT(7) /* Thermal warning Tj =145C */
#define DB4_THERMAL_WARNING3  BIT(6) /* Thermal warning Tj =125C */

/* TDA7569 DBX common to all the CH */
#define CHX_CURRENT_THRESHOLD BIT(5) /* Current threshold detected */
#define CHX_PERMANENT_DIAG    BIT(4) /* Permanent diagnostic */
#define CHX_SHORT_LOAD        BIT(3) /* Short Load */
#define CHX_OFFSET            BIT(2) /* Current threshold 0=high, 1=low */
#define CHX_SHORT_VCC         BIT(1) /* Short to Vcc */
#define CHX_SHORT_GND         BIT(0) /* Short to GND */

#endif /* __TDA75XX_H__ */
