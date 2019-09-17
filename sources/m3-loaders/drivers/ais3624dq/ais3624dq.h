/**
 * @file ais3624dq.c
 * @brief Provide all the ais3624dq accelerometer driver definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: ma.ning-amk1@st.com
 */

/* Define to prevent recursive inclusion */
#ifndef __AIS3624DQ_H
#define __AIS3624DQ_H

/******************************************************************************/
#define I_AM_AIS3624DQ				0x32

/* Accelerometer Sensor Full Scale bit */
#define AIS3624DQ_ACC_FS_MASK	0x30
#define AIS3624DQ_ACC_G_6G		0x00
#define AIS3624DQ_ACC_G_12G		0x10
#define AIS3624DQ_ACC_G_24G		0x30

/* Accelerometer Sensor Operating Mode */
#define AIS3624DQ_ACC_ENABLE		0x01
#define AIS3624DQ_ACC_DISABLE		0x00
#define AIS3624DQ_ACC_PM_NORMAL		0x20
#define AIS3624DQ_ACC_PM_OFF		AIS3624DQ_ACC_DISABLE

#define SENSITIVITY_6G		3	/**	mg/LSB	*/
#define SENSITIVITY_12G		6	/**	mg/LSB	*/
#define SENSITIVITY_24G		12	/**	mg/LSB	*/

#define AXISDATA_REG		0x28
#define WHOAMI_AIS3624DQ_ACC	0x32	/*	Expctd content for WAI	*/

/*	CONTROL REGISTERS ADDRESSES	*/
#define WHO_AM_I		0x0F	/*	WhoAmI register		*/
#define CTRL_REG1		0x20	/*				*/
#define CTRL_REG2		0x21	/*				*/
#define CTRL_REG3		0x22	/*				*/
#define CTRL_REG4		0x23	/*				*/
#define	CTRL_REG5		0x24	/*				*/

#define	INT_CFG1		0x30	/*	interrupt 1 config	*/
#define	INT_SRC1		0x31	/*	interrupt 1 source	*/
#define	INT_THS1		0x32	/*	interrupt 1 threshold	*/
#define	INT_DUR1		0x33	/*	interrupt 1 duration	*/

#define	INT_CFG2		0x34	/*	interrupt 2 config	*/
#define	INT_SRC2		0x35	/*	interrupt 2 source	*/
#define	INT_THS2		0x36	/*	interrupt 2 threshold	*/
#define	INT_DUR2		0x37	/*	interrupt 2 duration	*/
/*	end CONTROL REGISTRES ADDRESSES	*/

#define AIS3624DQ_ACC_ENABLE_ALL_AXES	0x07
#define AIS3624DQ_SELFTEST_EN	0x02
#define AIS3624DQ_SELFTEST_DIS	0x00
#define AIS3624DQ_SELFTEST_POS	0x00
#define AIS3624DQ_SELFTEST_NEG	0x08

#define AIS3624DQ_ACC_BDU_EN	0x80
#define AIS3624DQ_ACC_BDU_DIS	0x00

/* Accelerometer output data rate  */
#define AIS3624DQ_ACC_ODR_MASK		0x18
#define AIS3624DQ_ACC_ODRHALF	0x40	/* 0.5Hz output data rate */
#define AIS3624DQ_ACC_ODR1	0x60	/* 1Hz output data rate */
#define AIS3624DQ_ACC_ODR2	0x80	/* 2Hz output data rate */
#define AIS3624DQ_ACC_ODR5	0xA0	/* 5Hz output data rate */
#define AIS3624DQ_ACC_ODR10	0xC0	/* 10Hz output data rate */
#define AIS3624DQ_ACC_ODR50	0x00	/* 50Hz output data rate */
#define AIS3624DQ_ACC_ODR100	0x08	/* 100Hz output data rate */
#define AIS3624DQ_ACC_ODR400	0x10	/* 400Hz output data rate */
#define AIS3624DQ_ACC_ODR1000	0x18	/* 1000Hz output data rate */

#define AIS3624DQ_BOOT_NORMALMODE	0x00
#define AIS3624DQ_BOOT_FORCED		0x80

#define AIS3624DQ_AND_INT	0x80
#define AIS3624DQ_OR_INT	0x00
#define AIS3624DQ_6D_EN		0x40
#define AIS3624DQ_6D_DIS	0x00

#define AIS3624DQ_ZH_INT	0x20	/* interrupt on Z high event */
#define AIS3624DQ_ZL_INT	0x10	/* interrupt on Z low event */
#define AIS3624DQ_YH_INT	0x08	/* interrupt on Y high event */
#define AIS3624DQ_YL_INT	0x04	/* interrupt on Y low event */
#define AIS3624DQ_XH_INT	0x02	/* interrupt on X high event */
#define AIS3624DQ_XL_INT	0x01	/* interrupt on X low event */

#define AIS3624DQ_INT_IAH	0x00	/* interrupt active high */
#define AIS3624DQ_INT_IAL	0x80	/* interrupt active low */
#define AIS3624DQ_INT_PP	0x00	/* push-pull on interrupt pads */
#define AIS3624DQ_INT_OD	0x40	/* open-drain on interrupt pads */
#define AIS3624DQ_LIR2_DIS	0x00	/* interrupt 2 request not latched */
#define AIS3624DQ_LIR2_EN	0x20	/* latch int2_src interrupt request */
#define AIS3624DQ_LIR1_DIS	0x00	/* interrupt 1 request not latched */
#define AIS3624DQ_LIR1_EN	0x04	/* latch int1_src interrupt request */

/* one or more interrupts have been generated */
#define AIS3624DQ_INT_ACTIVED	0x40

#if 0
static struct {
	unsigned int cutoff_ms;
	unsigned int mask;
} ais3624dq_acc_odr_table[] = {
	{1, AIS3624DQ_ACC_PM_NORMAL | AIS3624DQ_ACC_ODR1000},
	{3, AIS3624DQ_ACC_PM_NORMAL | AIS3624DQ_ACC_ODR400},
	{10, AIS3624DQ_ACC_PM_NORMAL | AIS3624DQ_ACC_ODR100},
	{20, AIS3624DQ_ACC_PM_NORMAL | AIS3624DQ_ACC_ODR50},
	/* low power settings, max low pass filter cut-off freq */
	{100, AIS3624DQ_ACC_ODR10 | AIS3624DQ_ACC_ODR1000},
	{200, AIS3624DQ_ACC_ODR5 | AIS3624DQ_ACC_ODR1000},
	{5000, AIS3624DQ_ACC_ODR2 | AIS3624DQ_ACC_ODR1000 },
	{1000, AIS3624DQ_ACC_ODR1 | AIS3624DQ_ACC_ODR1000 },
	{2000, AIS3624DQ_ACC_ODRHALF | AIS3624DQ_ACC_ODR1000 },
};
#endif

#endif
