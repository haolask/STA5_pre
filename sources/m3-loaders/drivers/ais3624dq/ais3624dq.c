/**
 * @file ais3624dq.c
 * @brief Provide all the ais3624dq accelerometer driver functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: ma.ning-amk1@st.com
 */

#include "sta_acc.h"
#include "ais3624dq.h"
#include "FreeRTOS.h"

//#define AIS3624DQ_DEBUG
#ifdef AIS3624DQ_DEBUG
#define AIS3624DQ_TRACE(fmt, ...) do { \
	trace_print('D', fmt, ##__VA_ARGS__); \
	} while (0)
#else
#define AIS3624DQ_TRACE(fmt, ...)
#endif

/**
 * @brief  Set LIS3DSH Initialization.
 * @param  InitStruct: contains mask of different init parameters
 * @retval None
 */
static int ais3624dq_init(uint16_t init)
{
	uint8_t tmp = 0x0;

	/* Configure the low level interface */
	if (acc_init_io())
		return ACC_ERROR;

	/* Configure MEMS: power mode(ODR) and axes enable */
	tmp = (uint8_t)(init | AIS3624DQ_ACC_PM_NORMAL |
			AIS3624DQ_ACC_ENABLE_ALL_AXES);

	/* Write value to MEMS CTRL_REG1 register */
	if (acc_io_write(&tmp, CTRL_REG1))
		return ACC_ERROR;

	/* Configure MEMS: full scale and self test */
	tmp = AIS3624DQ_ACC_BDU_DIS | AIS3624DQ_ACC_G_6G;

	/* Write value to MEMS CTRL_REG4 register */
	if (acc_io_write(&tmp, CTRL_REG4))
		return ACC_ERROR;

	/* Configure MEMS: interrupt control  */
	tmp = AIS3624DQ_INT_IAH | AIS3624DQ_INT_PP;// | AIS3624DQ_LIR1_EN;

	/* Write value to MEMS CTRL_REG3 register */
	if (acc_io_write(&tmp, CTRL_REG3))
		return ACC_ERROR;

	return ACC_OK;
}

/**
 * @brief  Read AIS3624DQ device ID.
 * @param The Device ID
 * @retval status
 */
static int ais3624dq_read_id(uint8_t *id)
{
	/* Configure the low level interface */
	if (acc_init_io()) {
		AIS3624DQ_TRACE("read_id: acc_init_io failed ..");
		return ACC_ERROR;
	}
	/* Read WHO_AM_I register */
	if (acc_io_read(id, WHO_AM_I)) {
		AIS3624DQ_TRACE("read_id: acc_init_io failed ..");
		return ACC_ERROR;
	}
	/* Return the ID */
	return ACC_OK;
}

/**
 * @brief  Data Rate command.
 * @param  DataRateValue: Data rate value.
 * This parameter can be one of the following values:
 * @arg AIS3624DQ_DATARATE_50: 50 Hz output data rate
 * @arg AIS3624DQ_DATARATE_100: 100 Hz output data rate
 * @arg AIS3624DQ_DATARATE_400: 400 Hz output data rate
 * @arg AIS3624DQ_DATARATE_1000: 1000 Hz output data rate
 * @retval status
 */
static int ais3624dq_set_rate(uint8_t value)
{
	uint8_t tmpreg;

	/* Read CTRL_REG4 register */
	if (acc_io_read(&tmpreg, CTRL_REG1))
		return ACC_ERROR;

	/* Set new data rate configuration from 100 to 400Hz */
	tmpreg &= (uint8_t)~AIS3624DQ_ACC_ODR_MASK;
	tmpreg |= value;

	/* Write value to MEMS CTRL_REG1 register */
	if (acc_io_write(&tmpreg, CTRL_REG1))
		return ACC_ERROR;
	return ACC_OK;
}

/**
 * @brief  Change the Full Scale of AIS3624DQ.
 * @param  FS_value: new full scale value.
 * This parameter can be one of the following values:
 * @arg AIS3624DQ_ACC_G_6G: +-6g
 * @arg AIS3624DQ_ACC_G_12G: +-12g
 * @arg AIS3624DQ_ACC_G_24G: +-24g
 * @retval status
 */
static int ais3624dq_set_scale(uint8_t value)
{
	uint8_t tmpreg;

	/* Read CTRL_REG5 register */
	if (acc_io_read(&tmpreg, CTRL_REG4))
		return ACC_ERROR;

	/* Set new full scale configuration */
	tmpreg &= (uint8_t)~AIS3624DQ_ACC_FS_MASK;
	tmpreg |= value;

	/* Write value to MEMS CTRL_REG4 */
	if (acc_io_write(&tmpreg, CTRL_REG4))
		return ACC_ERROR;
	return ACC_OK;
}

/**
 * @brief  Return current Full Scale of AIS3624DQ.
 * @param  value: pointer to fill.
 * @retval status
 */
static int ais3624dq_get_scale(uint8_t *value)
{
	/* Read CTRL_REG5 register */
	if (acc_io_read(value, CTRL_REG4))
		return ACC_ERROR;
	*value &= AIS3624DQ_ACC_FS_MASK;
	return ACC_OK;
}

/**
 * @brief  Format a threshold to fit in THRSx register.
 * @param  value: threshold in mg.
 * @param  thrs: return value.
 * @retval status
 */
static int ais3624dq_compute_threshold(int value, uint8_t *thrs)
{
	uint8_t scale;
	int fs = 6000; /* in mg */

	if (ais3624dq_get_scale(&scale))
		return ACC_ERROR;
	switch (scale) {
	case AIS3624DQ_ACC_G_6G:
		fs = 6000;
		break;
	case AIS3624DQ_ACC_G_12G:
		fs = 12000;
		break;
	case AIS3624DQ_ACC_G_24G:
		fs = 24000;
		break;
	default:
		break;
	}
	*thrs = (uint8_t)(value / (fs / 128));
	return ACC_OK;
}

/**
 * @brief  Reboot memory content of AIS3624DQ.
 * @param  None
 * @retval status
 */
static int ais3624dq_reset(void)
{
	uint8_t tmp;
	/* Read CTRL_REG6 register */
	if (acc_io_read(&tmp, CTRL_REG2))
		return ACC_ERROR;

	/* Enable or Disable the reboot memory */
	tmp |= AIS3624DQ_BOOT_FORCED;

	/* Write value to MEMS CTRL_REG6 register */
	if (acc_io_write(&tmp, CTRL_REG2))
		return ACC_ERROR;
	return ACC_OK;
}

/**
 * @brief  Given raw XYZ data, calculate the acceleration
 *         ACC[mg]=SENSITIVITY * ((out_h << 8) | out_l).
 * @param  pointer to output buffer
 *         pointer to input buffer
 *         size of input buffer in number of uint16_t
 *         current fullscale
 * @retval status
 */
static int ais3624dq_calc_xyz(uint16_t *data_out, uint16_t *data_in,
	int size, uint8_t fs)
{
	int i, j;
	float sensitivity = SENSITIVITY_6G;
	float valueinfloat = 0;
	int16_t tmp;
	uint8_t *buf;

	switch (fs & AIS3624DQ_ACC_FS_MASK) {
	/* FS bit = 00 ==> Sensitivity typical value = 3 milligals/digit*/
	case AIS3624DQ_ACC_G_6G:
		sensitivity = SENSITIVITY_6G;
		break;
	/* FS bit = 10 ==> Sensitivity typical value = 6 milligals/digit*/
	case AIS3624DQ_ACC_G_12G:
		sensitivity = SENSITIVITY_12G;
		break;
	/* FS bit = 11 ==> Sensitivity typical value = 12 milligals/digit*/
	case AIS3624DQ_ACC_G_24G:
		sensitivity = SENSITIVITY_24G;
		break;
	default:
		break;
	}

	for (i = 0; i < size; i += 3) {
		buf = (uint8_t *)(data_in + i);
		/* Obtain the mg value for the three axis */
		for (j = 0; j < 3; j++) {
			/* Out data is 12 bit left just. */
			tmp = ((buf[2 * j + 1] << 8) | buf[2 * j]) >> 4;
			valueinfloat = tmp * sensitivity;
			data_out[i + j] = (int16_t)valueinfloat;
		}
	}
	return ACC_OK;
}

/**
 * @brief Set AIS3624DQ Interrupt configuration
 * @param value: requested tap threshold
 * @retval status
 */
static int ais3624dq_config_irq(struct acc_driver_data *acc_data)
{
	uint8_t tmp = 0x0;

	if (acc_config_io_irq())
		return ACC_ERROR;

	/* Set interrupt threshold */
	ais3624dq_compute_threshold(acc_data->tap_threshold, &tmp);
	if (acc_io_write(&tmp, INT_THS1))
		return ACC_ERROR;

	/* Set interrupt duration */
	tmp = 0x0;
	if (acc_io_write(&tmp, INT_DUR1))
		return ACC_ERROR;

	/* Configure Interrupt Selection , direction */
	tmp = (uint8_t)(AIS3624DQ_OR_INT |
			AIS3624DQ_6D_EN |
			AIS3624DQ_ZH_INT |
			AIS3624DQ_ZL_INT |
			AIS3624DQ_YH_INT |
			AIS3624DQ_YL_INT |
			AIS3624DQ_XH_INT |
			AIS3624DQ_XL_INT);
	/* Write value to MEMS INT_CFG1 register */
	if (acc_io_write(&tmp, INT_CFG1))
		return ACC_ERROR;

	return ACC_OK;
}

/**
 * @brief Set AIS3624DQ for tap detection
 * @param value: requested tap threshold
 * @retval status
 */
static int ais3624dq_handle_irq(struct acc_driver_data *acc_data)
{
	uint8_t tmp;

	/* Check interrupt source register */
	if (acc_io_read(&tmp, INT_SRC1))
		return ACC_ERROR;

	AIS3624DQ_TRACE("%s : stat:0x%x\n", __func__, tmp);

	if (tmp & AIS3624DQ_INT_ACTIVED) {
		if (!acc_data->tap) {
			acc_data->tap = true;
			AIS3624DQ_TRACE("%s: tap detected .\n", __func__);

			if (ais3624dq_set_rate(acc_data->odr))
				return ACC_ERROR;
#if 0
			/* Configure Interrupt Selection , direction */
			tmp = (uint8_t)(AIS3624DQ_OR_INT |
				AIS3624DQ_6D_EN |
				AIS3624DQ_ZH_INT |
				AIS3624DQ_ZL_INT |
				AIS3624DQ_YH_INT |
				AIS3624DQ_YL_INT |
				AIS3624DQ_XH_INT |
				AIS3624DQ_XL_INT);
			/* Write value to MEMS INT_CFG1 register */
			if (acc_io_write(&tmp, INT_CFG1))
				return ACC_ERROR;
#endif
		}
	}

	return ACC_OK;
}

static int ais3624dq_enable_irq(void)
{
	uint8_t tmp;
	/* Configure Interrupt Selection , direction */
	tmp = (uint8_t)(AIS3624DQ_OR_INT |
		AIS3624DQ_6D_EN |
		AIS3624DQ_ZH_INT |
		AIS3624DQ_ZL_INT |
		AIS3624DQ_YH_INT |
		AIS3624DQ_YL_INT |
		AIS3624DQ_XH_INT |
		AIS3624DQ_XL_INT);
	/* Write value to MEMS INT_CFG1 register */
	if (acc_io_write(&tmp, INT_CFG1))
		return ACC_ERROR;
	return ACC_OK;
}

static int ais3624dq_disable_irq(void)
{
	return ACC_OK;
}

static int ais3624dq_sby_prepare(void)
{
	return ACC_OK;
}

const struct acc_driver_ops ais3624dq_ops = {
	ais3624dq_init,
	ais3624dq_read_id,
	ais3624dq_reset,
	ais3624dq_config_irq,
	ais3624dq_handle_irq,
	ais3624dq_enable_irq,
	ais3624dq_disable_irq,
	ais3624dq_calc_xyz,
	ais3624dq_set_rate,
	ais3624dq_set_scale,
	ais3624dq_sby_prepare
};

