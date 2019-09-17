/**
  ******************************************************************************
  * @file    lis3dsh.c
  * @author  jean-nicolas.graux@st.com
  * @version V1.0.0
  * @date    27-March-2015
  * @brief   This file provides a set of functions needed to manage the LIS3DSH
  *          MEMS Accelerometer available on STA1078-CT2 board.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "sta_acc.h"
#include "lis3dsh.h"
#include "FreeRTOS.h"

#ifdef LISD3SH_DEBUG
#define LIS3DSH_TRACE(fmt, ...) do { \
	trace_print('D', fmt, ##__VA_ARGS__); } while (0)
#else
#define LIS3DSH_TRACE(fmt, ...)
#endif

/**
  * @brief  Set LIS3DSH Initialization.
  * @param  InitStruct: contains mask of different init parameters
  * @retval None
  */
static int lis3dsh_init(uint16_t InitStruct)
{
	uint8_t tmp = 0x0;

	/* Configure the low level interface */
	if (acc_init_io())
		return ACC_ERROR;

	/* Configure MEMS: power mode(ODR) and axes enable */
	tmp = (uint8_t)InitStruct;

	/* Write value to MEMS CTRL_REG4 register */
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG4_ADDR))
		return ACC_ERROR;

	/* Configure MEMS: full scale and self test */
	tmp = (uint8_t) (InitStruct >> 8);

	/* Write value to MEMS CTRL_REG5 register */
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG5_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

/**
  * @brief  Read LIS3DSH device ID.
  * @param The Device ID
  * @retval status
  */
static int lis3dsh_read_id(uint8_t *id)
{
	/* Configure the low level interface */
	if (acc_init_io())
		return ACC_ERROR;
	/* Read WHO_AM_I register */
	if (acc_io_read(id, LIS3DSH_WHO_AM_I_ADDR))
		return ACC_ERROR;
	/* Return the ID */
	return ACC_OK;
}

/**
  * @brief  Data Rate command.
  * @param  DataRateValue: Data rate value.
  *   This parameter can be one of the following values:
  *     @arg LIS3DSH_DATARATE_3_125: 3.125 Hz output data rate
  *     @arg LIS3DSH_DATARATE_6_25: 6.25 Hz output data rate
  *     @arg LIS3DSH_DATARATE_12_5: 12.5  Hz output data rate
  *     @arg LIS3DSH_DATARATE_25: 25 Hz output data rate
  *     @arg LIS3DSH_DATARATE_50: 50 Hz output data rate
  *     @arg LIS3DSH_DATARATE_100: 100 Hz output data rate
  *     @arg LIS3DSH_DATARATE_400: 400 Hz output data rate
  *     @arg LIS3DSH_DATARATE_800: 800 Hz output data rate
  *     @arg LIS3DSH_DATARATE_1600: 1600 Hz output data rate
  * @retval status
  */
static int lis3dsh_set_rate(uint8_t value)
{
	uint8_t tmpreg;

	/* Read CTRL_REG4 register */
	if (acc_io_read(&tmpreg, LIS3DSH_CTRL_REG4_ADDR))
		return ACC_ERROR;

	/* Set new data rate configuration from 100 to 400Hz */
	tmpreg &= (uint8_t)~LIS3DSH_DATARATE_MSK;
	tmpreg |= value;

	/* Write value to MEMS CTRL_REG4 register */
	if (acc_io_write(&tmpreg, LIS3DSH_CTRL_REG4_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

/**
  * @brief  Change the Full Scale of LIS3DSH.
  * @param  FS_value: new full scale value.
  *   This parameter can be one of the following values:
  *     @arg LIS3DSH_FULLSCALE_2: +-2g
  *     @arg LIS3DSH_FULLSCALE_4: +-4g
  *     @arg LIS3DSH_FULLSCALE_6: +-6g
  *     @arg LIS3DSH_FULLSCALE_8: +-8g
  *     @arg LIS3DSH_FULLSCALE_16: +-16g
  * @retval status
  */
static int lis3dsh_set_scale(uint8_t value)
{
	uint8_t tmpreg;

	/* Read CTRL_REG5 register */
	if (acc_io_read(&tmpreg, LIS3DSH_CTRL_REG5_ADDR))
		return ACC_ERROR;

	/* Set new full scale configuration */
	tmpreg &= (uint8_t)~LIS3DSH_FULLSCALE_MSK;
	tmpreg |= value;

	/* Write value to MEMS CTRL_REG5 register */
	if (acc_io_write(&tmpreg, LIS3DSH_CTRL_REG5_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

/**
  * @brief  Return current Full Scale of LIS3DSH.
  * @param  value: pointer to fill.
  * @retval status
  */
static int lis3dsh_get_scale(uint8_t *value)
{
	/* Read CTRL_REG5 register */
	if (acc_io_read(value, LIS3DSH_CTRL_REG5_ADDR))
		return ACC_ERROR;
	*value &= LIS3DSH_FULLSCALE_MSK;
	return ACC_OK;
}

/**
  * @brief  Format a threshold to fit in THRSx register.
  * @param  value: threshold in mg.
  * @param  thrs: return value.
  * @retval status
  */
static int lis3dsh_compute_threshold(int value, uint8_t *thrs)
{
	uint8_t scale;
	int fs = 2000; /* in mg */

	if(lis3dsh_get_scale(&scale))
		return ACC_ERROR;
	switch (scale) {
	case LIS3DSH_FULLSCALE_2:
		fs = 2000;
		break;
	case LIS3DSH_FULLSCALE_4:
		fs = 4000;
		break;
	case LIS3DSH_FULLSCALE_6:
		fs = 6000;
		break;
	case LIS3DSH_FULLSCALE_8:
		fs = 8000;
		break;
	case LIS3DSH_FULLSCALE_16:
		fs = 16000;
		break;
	default:
		break;
	}
	*thrs = (uint8_t)(value / (fs / 128));
	return ACC_OK;
}
/**
  * @brief  Reboot memory content of LIS3DSH.
  * @param  None
  * @retval status
  */
static int lis3dsh_reset(void)
{
	uint8_t tmp;
	/* Read CTRL_REG6 register */
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;

	/* Enable or Disable the reboot memory */
	tmp |= LIS3DSH_BOOT_FORCED;

	/* Write value to MEMS CTRL_REG6 register */
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG6_ADDR))
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
static int lis3dsh_calc_xyz(uint16_t *data_out, uint16_t *data_in, int size, uint8_t fs)
{
	int i, j;
	float sensitivity = LIS3DSH_SENSITIVITY_0_06G;
	float valueinfloat = 0;
	int16_t tmp;
	uint8_t *buf;

	switch(fs & LIS3DSH_FULLSCALE_MSK)
	{
		/* FS bit = 000 ==> Sensitivity typical value = 0.06milligals/digit*/
	case LIS3DSH_FULLSCALE_2:
		sensitivity = LIS3DSH_SENSITIVITY_0_06G;
		break;
		/* FS bit = 001 ==> Sensitivity typical value = 0.12milligals/digit*/
	case LIS3DSH_FULLSCALE_4:
		sensitivity = LIS3DSH_SENSITIVITY_0_12G;
		break;
		/* FS bit = 010 ==> Sensitivity typical value = 0.18milligals/digit*/
	case LIS3DSH_FULLSCALE_6:
		sensitivity = LIS3DSH_SENSITIVITY_0_18G;
		break;
		/* FS bit = 011 ==> Sensitivity typical value = 0.24milligals/digit*/
	case LIS3DSH_FULLSCALE_8:
		sensitivity = LIS3DSH_SENSITIVITY_0_24G;
		break;
		/* FS bit = 100 ==> Sensitivity typical value = 0.73milligals/digit*/
	case LIS3DSH_FULLSCALE_16:
		sensitivity = LIS3DSH_SENSITIVITY_0_73G;
		break;
	default:
		break;
	}

	for (i = 0; i < size; i += 3) {
		buf = (uint8_t *)(data_in + i);
		/* Obtain the mg value for the three axis */
		for (j = 0; j < 3; j++) {
			tmp = (buf[2 * j + 1] << 8) | buf[2 * j];
			valueinfloat = tmp * sensitivity;
			data_out[i + j] = (int16_t)valueinfloat;
		}
	}
	return ACC_OK;
}

/**
  * @brief  LIS3DSH fifo configuration
  * @param  None
  * @retval status
  */
static int lis3dsh_config_fifo(struct acc_driver_data * acc_data)
{
	uint8_t tmp;

	tmp = LIS3DSH_FIFO_STREAM_MODE |
		((acc_data->wtm - 2) & LIS3DSH_FIFO_WATERMARK_MSK);
	if (acc_io_write(&tmp, LIS3DSH_FIFO_CTRL_ADDR))
		return ACC_ERROR;

	tmp = LIS3DSH_P1_WTM | LIS3DSH_ADD_INC | LIS3DSH_FIFO_EN;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

#if 0
static int lis3dsh_enable_int1_fifo_ovr(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	tmp |= LIS3DSH_P1_OVERRUN;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_disable_int1_fifo_ovr(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	tmp &= ~LIS3DSH_P1_OVERRUN;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}
#endif

static int lis3dsh_enable_int1_fifo_wtm(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	tmp |= LIS3DSH_P1_WTM;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_disable_int1_fifo_wtm(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	tmp &= ~LIS3DSH_P1_WTM;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG6_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_enable_int1_sitr(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_SETT1_ADDR))
		return ACC_ERROR;
	tmp |= LIS3DSH_SM_SETT_SITR;
	if (acc_io_write(&tmp, LIS3DSH_SETT1_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_disable_int1_sitr(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_SETT1_ADDR))
		return ACC_ERROR;
	tmp &= ~LIS3DSH_SM_SETT_SITR;
	if (acc_io_write(&tmp, LIS3DSH_SETT1_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_enable_int1(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG3_ADDR))
		return ACC_ERROR;
	tmp |= LIS3DSH_INTERRUPT_1_ENABLE;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG3_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_disable_int1(void)
{
	uint8_t tmp;
	if (acc_io_read(&tmp, LIS3DSH_CTRL_REG3_ADDR))
		return ACC_ERROR;
	tmp &= ~LIS3DSH_INTERRUPT_1_ENABLE;
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG3_ADDR))
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_enable_irq(void)
{
	if (lis3dsh_enable_int1_sitr())
		return ACC_ERROR;
	if (lis3dsh_enable_int1_fifo_wtm())
		return ACC_ERROR;
	if (lis3dsh_enable_int1())
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_disable_irq(void)
{
	if (lis3dsh_disable_int1())
		return ACC_ERROR;
	if (lis3dsh_disable_int1_fifo_wtm())
		return ACC_ERROR;
	if (lis3dsh_disable_int1_sitr())
		return ACC_ERROR;
	return ACC_OK;
}

static int lis3dsh_empty_fifo(struct acc_driver_data *acc_data)
{
	uint8_t * buf;
	uint8_t tmp;
	int i, nb_samples;

	if(acc_data->xyz_buf2_full)
		return ACC_OK;

	if (acc_io_read(&tmp, LIS3DSH_FIFO_SRC_ADDR))
		return ACC_ERROR;

	nb_samples = (int)(tmp & LIS3DSH_FIFO_FSS_MSK) + 1;
	LIS3DSH_TRACE("%s: tap:%d: %d samples to read\n", __func__,
		acc_data->tap, nb_samples);

	if(!acc_data->tap) {
		if (acc_data->xyz_buf1_next + (nb_samples * 3) >
			acc_data->xyz_buf1 + acc_data->buf1_xyz_size) {
			/* Go back to start of xyz_buf1 */
			buf = (uint8_t *)acc_data->xyz_buf1;
			acc_data->xyz_buf1_full = true;
			LIS3DSH_TRACE("%s: xyz_buf1 is full\n",
				__func__);
		} else
			buf = (uint8_t *)acc_data->xyz_buf1_next;
	} else {
		if (acc_data->xyz_buf2_next + (nb_samples * 3) >
			acc_data->xyz_buf2 + acc_data->buf2_xyz_size) {
			/* We are full, let's disable interrupts */
			if (lis3dsh_disable_irq())
				return ACC_ERROR;
			acc_data->xyz_buf2_full = true;
			LIS3DSH_TRACE("%s: xyz_buf2 is full\n",
				__func__);
			return ACC_OK;
		} else
			buf = (uint8_t *)acc_data->xyz_buf2_next;
	}

	/**
	 * So let's loop on a 6 bytes multiple read, eg read one tuple
	 * of (X, Y, Z) datas per loop.
	 */
	for (i = nb_samples ; i > 0 ; i--) {
		if (acc_io_read_multiple(buf, LIS3DSH_OUT_X_L_ADDR, 6))
			return ACC_ERROR;
		buf += 6;
	}

	if(!acc_data->tap)
		acc_data->xyz_buf1_next = (uint16_t *)buf;
	else
		acc_data->xyz_buf2_next = (uint16_t *)buf;

	return ACC_OK;
}

/**
  * @brief Set LIS3DSH Interrupt configuration
  * @param value: requested tap threshold
  * @retval status
  */
static int lis3dsh_config_irq(struct acc_driver_data * acc_data)
{
	uint8_t tmp = 0x0;

	if (acc_config_io_irq())
		return ACC_ERROR;

	/* Configure Interrupt Selection , Request and Signal */
	tmp = (uint8_t)(LIS3DSH_INTERRUPT_1_ENABLE | \
			LIS3DSH_INTERRUPT_REQUEST_LATCHED | \
			LIS3DSH_INTERRUPT_SIGNAL_HIGH);
	/* Write value to MEMS CTRL_REG3 register */
	if (acc_io_write(&tmp, LIS3DSH_CTRL_REG3_ADDR))
		return ACC_ERROR;

	/* Configure State Machine 1 */
	tmp = (uint8_t)(LIS3DSH_SM_ENABLE);
	/* Write value to MEMS CTRL_REG1 register */
	acc_io_write(&tmp, LIS3DSH_CTRL_REG1_ADDR);

	/* Set LIS3DSH State Machine configuration */
	lis3dsh_compute_threshold(acc_data->tap_threshold, &tmp);
	if (acc_io_write(&tmp, LIS3DSH_THRS1_1_ADDR))
		return ACC_ERROR;
	tmp = LIS3DSH_OP_CODE_GNTH1;
	if (acc_io_write(&tmp, LIS3DSH_ST1_1_ADDR))
		return ACC_ERROR;
	tmp = LIS3DSH_OP_CODE_CONT;
	if (acc_io_write(&tmp, LIS3DSH_ST1_2_ADDR))
		return ACC_ERROR;
	tmp = LIS3DSH_MASK_XYZ;
	if (acc_io_write(&tmp, LIS3DSH_MASK1_B_ADDR))
		return ACC_ERROR;
	tmp = LIS3DSH_MASK_XYZ;
	if (acc_io_write(&tmp, LIS3DSH_MASK1_A_ADDR))
		return ACC_ERROR;
	tmp = LIS3DSH_SM_SETT_SITR |
		LIS3DSH_SM_SETT_ABS |
		LIS3DSH_SM_SETT_P_DET;
	if (acc_io_write(&tmp, LIS3DSH_SETT1_ADDR))
		return ACC_ERROR;

	if(lis3dsh_config_fifo(acc_data))
		return ACC_ERROR;

	return ACC_OK;
}

/**
  * @brief Set LIS3DSH for tap detection
  * @param value: requested tap threshold
  * @retval status
  */
static int lis3dsh_handle_irq(struct acc_driver_data *acc_data)
{
	uint8_t tmp;

	/* Check STAT register */
	if (acc_io_read(&tmp, LIS3DSH_STAT_ADDR))
		return ACC_ERROR;

	LIS3DSH_TRACE("%s : stat:0x%x\n", __func__, tmp);

	if (tmp & LIS3DSH_STAT_INT_SM1) {
		if (acc_io_read(&(acc_data->outs), LIS3DSH_OUTS1_ADDR))
			return ACC_ERROR;
		if (!acc_data->tap) {
			acc_data->tap = true;
			if (lis3dsh_disable_int1_sitr())
				return ACC_ERROR;
			if (lis3dsh_set_rate(acc_data->odr))
				return ACC_ERROR;
			LIS3DSH_TRACE("%s: tap detected, outs=0x%x\n",
				__func__, acc_data->outs);
		}
	}

	/* Check FIFO source register */
	if (acc_io_read(&tmp, LIS3DSH_FIFO_SRC_ADDR))
		return ACC_ERROR;

	LIS3DSH_TRACE("%s : fifo src:0x%x\n", __func__, tmp);

	if (tmp & LIS3DSH_FIFO_WTM) {
		if (lis3dsh_empty_fifo(acc_data))
			return ACC_ERROR;
	}
	return ACC_OK;
}

static int lis3dsh_sby_prepare(void)
{
	if (lis3dsh_disable_int1_fifo_wtm())
		return ACC_ERROR;
	return ACC_OK;
}

const struct acc_driver_ops lis3dsh_ops = {
	lis3dsh_init,
	lis3dsh_read_id,
	lis3dsh_reset,
	lis3dsh_config_irq,
	lis3dsh_handle_irq,
	lis3dsh_enable_irq,
	lis3dsh_disable_irq,
	lis3dsh_calc_xyz,
	lis3dsh_set_rate,
	lis3dsh_set_scale,
	lis3dsh_sby_prepare
};

