/**
 * @file sta_acc.c
 * @brief This file provides a set of functions needed to manage the
 *        MEMS(LIS3DSH/AIS3624DQ) accelerometer.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>
#include "utils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sta_acc.h"
#include "sta_gpio.h"
#include "sta_nvic.h"
#include "sta_pm.h"
#include "sta_pmu.h"
#include "sta_i2c_service.h"
#include "lis3dsh.h"
#include "ais3624dq.h"
#include "sta_pinmux.h"
#include "sta_platform.h"

#define ACCEL_INT PMU_WAKE2
/**
 * Set a quite low tap threshold to allow
 * simulating a car crash by shaking the board.
 */
#define ACCEL_WAKEUP_THRESHOLD 1500 /* in mg */
#define I2C_CLOCK 51200000
#define LIS3DSH_ADDR 0x3C
#define AIS3624DQ_ADDR 0x30

extern struct acc_driver_ops lis3dsh_ops;
extern struct acc_driver_ops ais3624dq_ops;
static struct acc_driver_ops *acc_drv_ops;
static struct i2c_com_handler_s *i2c_h;
static xSemaphoreHandle irq_sem;
static struct pm_notifier acc_sby_notifier;
static struct acc_driver_data acc_data;

/**
  * @brief Accelerometer Top half ISR.
  * This routine is called in IRQ mode.
  */
static int acc_irq_handler_top(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	if (!xSemaphoreGiveFromISR(irq_sem, &xHigherPriorityTaskWoken))
		return ACC_ERROR;
	if (xHigherPriorityTaskWoken)
		taskYIELD();
	return 0;
}

/**
  * @brief  Configures ACCELEROMETER I2C interface.
  * @param  None
  * @retval None
  */
int acc_init_io(void)
{
	static bool initialized;

	if (initialized)
		return ACC_OK;
	if (i2c_service_init(I2C_CLOCK))
		return ACC_ERROR;
	if (i2c_open_port(i2c0_regs, 0, 0, I2C_BUSCTRLMODE_MASTER))
		return ACC_ERROR;
	i2c_set_port_mode(i2c0_regs, I2C_BUSCTRLMODE_MASTER);
	if (get_board_id() != BOARD_TC3P_MTP)
		i2c_h = i2c_create_com(i2c0_regs, I2C_FAST_MODE, LIS3DSH_ADDR >> 1);
	else
		i2c_h = i2c_create_com(i2c0_regs, I2C_FAST_MODE, AIS3624DQ_ADDR >> 1);
	if (!i2c_h)
		return ACC_ERROR;
	initialized = 1;
	return ACC_OK;
}

/**
  * @brief Setup accelerometer interrupt.
  * @param None
  * @retval None
  */
int acc_config_io_irq(void)
{
	struct gpio_config pin;
	struct nvic_chnl irq_chnl;

	/* Setup M3_GPIO2 aka "ACCEL_INT" */
	pin.direction = GPIO_DIR_INPUT;
	pin.mode = GPIO_MODE_SOFTWARE;
	pin.level = GPIO_LEVEL_PULLDOWN;
	pin.trig = GPIO_TRIG_RISING_EDGE;
	if (get_board_id() == BOARD_TC3P_MTP) {
		gpio_set_pin_config(AO_GPIO(1), &pin);

		if (gpio_request_irq(AO_GPIO(1), acc_irq_handler_top)) {
			TRACE_ERR("%s: failed to request irq\n", __func__);
			return ACC_ERROR;
		}
	} else {
		gpio_set_pin_config(M3_GPIO(2), &pin);

		if (gpio_request_irq(M3_GPIO(2), acc_irq_handler_top)) {
			TRACE_ERR("%s: failed to request irq\n", __func__);
			return ACC_ERROR;
		}
	}
	/**
	 * Enable the M3 GPIO interrupt. This should be sone elsewhere,
	 * eg in some general system init area.
	 */
	irq_chnl.id = GPIO_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	nvic_chnl_init(&irq_chnl);
	return ACC_OK;
}

/**
  * @brief  Writes one byte to the ACCELEROMETER.
  * @param  pBuffer : pointer to the buffer  containing the data to be written to the ACCELEROMETER.
  * @param  WriteAddr : ACCELEROMETER's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
int acc_io_write(uint8_t *buf, uint8_t addr)
{
	portTickType timeout = 1000;
	if (!i2c_h)
		return ACC_ERROR;
	/*TRACE_INFO("%s: 0x%x at 0x%x\n", __func__,
		*buf, addr);*/
	if (i2c_write(i2c_h, addr, 1, buf, 1, 1, &timeout)) {
		TRACE_ERR("%s: failed to write at 0x%x\n",
			__func__, addr);
		return ACC_ERROR;
	}
	return ACC_OK;
}

/**
  * @brief  Reads a single byte from the ACCELEROMETER.
  * @param  buf : pointer to the buffer that receives the data read from the ACCELEROMETER.
  * @param  addr : ACCELEROMETER's internal address to read from.
  * @retval status
  */
int acc_io_read(uint8_t *buf, uint8_t addr)
{
	portTickType timeout = 1000;
	if (!i2c_h)
		return ACC_ERROR;
	if (i2c_read(i2c_h, addr, 1, buf, 1, &timeout)) {
		TRACE_ERR("%s: failed to read at 0x%x\n",
			__func__, addr);
		return ACC_ERROR;
	}
	/*TRACE_INFO("%s: 0x%x at 0x%x\n", __func__,
		*buf, addr);*/
	return ACC_OK;
}

/**
  * @brief  Reads a block of data from the ACCELEROMETER.
  * @param  buf : pointer to the buffer that receives the data read from the ACCELEROMETER.
  * @param  addr : ACCELEROMETER's internal address to read from.
  * @param  length : number of bytes to read from the ACCELEROMETER.
  * @retval status
  */
int acc_io_read_multiple(uint8_t *buf, uint8_t addr, int length)
{
	portTickType timeout = 1000;
	if (!i2c_h)
		return ACC_ERROR;
	if (i2c_read(i2c_h, addr, 1, buf, length, &timeout)) {
		TRACE_ERR("%s: failed to read at 0x%x\n",
			__func__, addr);
		return ACC_ERROR;
	}
	return ACC_OK;
}

/**
  * @brief Accelerometer Bottom half ISR.
  * This routine is called in a FreeRTOS Task.
  * @param  pointer to struct acc_driver_data
  * @retval status
  */
static int acc_irq_handler_bottom(struct acc_driver_data *data)
{
	if (!acc_drv_ops->handle_irq)
		return ACC_OK;
	if (acc_drv_ops->handle_irq(data))
		return ACC_ERROR;
	return ACC_OK;
}

/**
  * @brief  ACCELEROMETER xyz buffers Initialization.
  * @param  struct acc_driver_data
  * @retval status
  */
static inline void acc_init_xyz(struct acc_driver_data *acc_data)
{
	acc_data->tap = false;
	if (get_board_id() != BOARD_TC3P_MTP) {
		acc_data->xyz_buf1_full = false;
		acc_data->xyz_buf2_full = false;
	} else {
		/* Fill with dummy date first */
		acc_data->xyz_buf1_full = true;
		acc_data->xyz_buf2_full = true;
	}
	acc_data->xyz_buf1_next = acc_data->xyz_buf1;
	acc_data->xyz_buf2_next = acc_data->xyz_buf2;
}

/**
  * @brief  Set lis3dsh ACCELEROMETER Initialization.
  * @param  struct acc_driver_data
  * @retval status
  */
static int acc_init_lis3dsh(struct acc_driver_data *acc_data)
{
	uint16_t ctrl = 0x0;
	uint8_t id;
	int buf1_nb_samples = 0;
	int buf2_nb_samples = 0;

	acc_data->wtm = 32;
	if (!lis3dsh_ops.read_id(&id)) {
		if (id == I_AM_LIS3DSH) {
			/* Initialize the accelerometer driver structure */
			acc_drv_ops = &lis3dsh_ops;
			/**
			 * About Accident Acceleration Profile collection,
			 * Here is what the ERA-GLONASS spec says:
			 * - CRASH_PRE_RECORD_TIME = 20s
			 * - CRASH_PRE_RECORD_RESOLUTION <= 100ms
			 * - CRASH_RECORD_TIME = 250ms
			 * - CRASH_RECORD_RESOLUTION <= 5ms
			 *
			 * Thus, if relying on LIS3DSH accelerometer:
			 *
			 * a) Considering a pre crash ODR of 12.5Hz,
			 * which means a period between two samples = 80 ms,
			 * size of "buf1" must be equal to 20000 / 80 = 250 samples.
			 * Let's set 240 samples for pre-crash which a little bit under
			 * the requirement.

			 * thus 256 samples if rounded to the size of the hardware fifos.
			 *
			 * b) CRASH_RECORD_RESOLUTION <= 5ms means an ODR >= 200Hz.
			 * Unfortunalely, there is no way to select 200Hz in LIS3DSH,
			 * so let's select 400Hz. With such a high rate,
			 * I2C link @ 400Khz is too slow to empty the whole hardware fifo
			 * in a raw without risking to loose one sample.
			 * That's why we need to set a fifo watermark = 16.
			 *
			 * Considering a post crash ODR of 400Hz,
			 * which means a period between two samples = 2.5 ms,
			 * size of "buf2" must be equal to 250 / 2.5 = 100 samples.
			 * Let's set 96 samples for post crash history
			 * which a little bit under the requirement.
			 */
			acc_data->odr_pre = LIS3DSH_DATARATE_12_5;
			acc_data->odr = LIS3DSH_DATARATE_400;
			acc_data->axes_enable = LIS3DSH_XYZ_MSK;
			acc_data->self_test = LIS3DSH_SELFTEST_NORMAL;
			acc_data->full_scale = LIS3DSH_FULLSCALE_16;
			acc_data->filter_bw = LIS3DSH_FILTER_BW_800;
			acc_data->wtm = 16;
			buf1_nb_samples = acc_data->wtm * 15; /* 240 samples @ 12.5Hz */
			buf2_nb_samples = acc_data->wtm * 6; /* 96 samples @ 400Hz */
			TRACE_INFO("%s: found lis3dsh\n", __func__);
			goto init;
		}
	}

	TRACE_ERR("%s: failed to read id\n", __func__);
	return ACC_ERROR;

init:

	/* Configure MEMS: power mode(ODR) and axes enable */
	ctrl = (uint16_t) (acc_data->odr_pre | acc_data->axes_enable);

	/* Configure MEMS: full scale and self test */
	ctrl |= (uint16_t) ((acc_data->self_test |
		acc_data->full_scale |
		acc_data->filter_bw) << 8);

	/* Configure the accelerometer main parameters */
	if (acc_drv_ops->init(ctrl)) {
		TRACE_ERR("%s: failed\n", __func__);
		return ACC_ERROR;
	}

	acc_data->tap_threshold = ACCEL_WAKEUP_THRESHOLD;
	/*
	 * TAKE CARE that (acc_data->buf1_xyz_size * 2 + acc_data->buf2_xyz_size * 2)
	 * must NOT be greater than RP_MSG_PAYLOAD_SIZE.
	 * Otherwise we will get buffer overflow in acc_on_tap_get_xyz_datas() function.
	 *
	 * Taking LIS3DSH config as example as assuming RP_MSG_PAYLOAD_SIZE = 2048 bytes:
	 * (96 + 240) * 3 * 2 = 2016 bytes < RP_MSG_PAYLOAD_SIZE
	 */
	acc_data->buf1_xyz_size = buf1_nb_samples * 3;
	acc_data->buf2_xyz_size = buf2_nb_samples * 3;

	acc_data->xyz_buf1 = pvPortMalloc(acc_data->buf1_xyz_size * sizeof(uint16_t));
	if (!acc_data->xyz_buf1)
		return ACC_ERROR;
	acc_data->xyz_buf2 = pvPortMalloc(acc_data->buf2_xyz_size * sizeof(uint16_t));
	if (!acc_data->xyz_buf2)
		return ACC_ERROR;

	acc_init_xyz(acc_data);

	if (!acc_drv_ops->config_irq)
		return ACC_ERROR;
	if (acc_drv_ops->config_irq(acc_data))
		return ACC_ERROR;

	TRACE_INFO("%s: done\n", __func__);
	return ACC_OK;
}

/**
  * @brief  Set AIS3624dq ACCELEROMETER Initialization.
  * @param  struct acc_driver_data
  * @retval status
  */
static int acc_init_ais3624dq(struct acc_driver_data *acc_data)
{
	uint16_t ctrl = 0x0;
	uint8_t id;
	int buf1_nb_samples = 0;
	int buf2_nb_samples = 0;

	if (!ais3624dq_ops.read_id(&id)) {
		if (id == I_AM_AIS3624DQ) {
			acc_drv_ops = &ais3624dq_ops;
			/**
			 * About Accident Acceleration Profile collection,
			 * Here is what the ERA-GLONASS spec says:
			 * - CRASH_PRE_RECORD_TIME = 20s
			 * - CRASH_PRE_RECORD_RESOLUTION <= 100ms
			 * - CRASH_RECORD_TIME = 250ms
			 * - CRASH_RECORD_RESOLUTION <= 5ms
			 *
			 * Thus, if relying on AIS3624DQ accelerometer:
			 *
			 * a) Considering a pre crash ODR of 50Hz,
			 * which means a period between two samples = 20 ms,
			 * size of "buf1" must be equal to 20000 / 20 = 1000 samples.

			 *
			 * b) CRASH_RECORD_RESOLUTION <= 5ms means an ODR >= 200Hz.
			 * so let's select 400Hz.
			 * which means a period between two samples = 2.5 ms
			 * size of "buf2" must be equal to 250 / 2.5 = 100 samples.
			 */
			acc_data->odr_pre = AIS3624DQ_ACC_ODR50;
			acc_data->odr = AIS3624DQ_ACC_ODR400;
			acc_data->full_scale = AIS3624DQ_ACC_G_6G;
			/* Use dummy date to wakup A7 first */
			buf1_nb_samples = 20; /* 240 samples @ 50Hz */
			buf2_nb_samples = 20; /* 100 samples @ 400Hz */
			TRACE_INFO("%s: found ais3624dq sensor.\n", __func__);
		} else {
			goto error_id;
		}
	} else {
		goto error_id;
	}
	ctrl = (uint16_t)acc_data->odr_pre;
	/* Configure the accelerometer main parameters */
	if (acc_drv_ops->init(ctrl)) {
		TRACE_ERR("%s: failed\n", __func__);
		return ACC_ERROR;
	}
	/*
	 * TAKE CARE that (acc_data->buf1_xyz_size * 2 + acc_data->buf2_xyz_size * 2)
	 * must NOT be greater than RP_MSG_PAYLOAD_SIZE.
	 * Otherwise we will get buffer overflow in acc_on_tap_get_xyz_datas() function.
	 *
	 * Taking above config as example as assuming RP_MSG_PAYLOAD_SIZE = 2048 bytes:
	 * (240 + 100) * 3 * 2 = 2040 bytes < RP_MSG_PAYLOAD_SIZE
	 */
	acc_data->buf1_xyz_size = buf1_nb_samples * 3;
	acc_data->buf2_xyz_size = buf2_nb_samples * 3;

	acc_data->xyz_buf1 = pvPortMalloc(acc_data->buf1_xyz_size * sizeof(uint16_t));
	if (!acc_data->xyz_buf1)
		return ACC_ERROR;
	acc_data->xyz_buf2 = pvPortMalloc(acc_data->buf2_xyz_size * sizeof(uint16_t));
	if (!acc_data->xyz_buf2)
		return ACC_ERROR;

	acc_init_xyz(acc_data);
	acc_data->tap_threshold = ACCEL_WAKEUP_THRESHOLD;
	if (!acc_drv_ops->config_irq)
		return ACC_ERROR;
	if (acc_drv_ops->config_irq(acc_data))
		return ACC_ERROR;

	TRACE_INFO("%s: done\n", __func__);
	return ACC_OK;

error_id:
	TRACE_ERR("%s: can't detect sensor\n", __func__);
	return ACC_ERROR;
}

static void acc_sby(void)
{
	if (!acc_drv_ops->sby_prepare)
		return;
	acc_drv_ops->sby_prepare();
}

/**
  * @brief User function to get ACCELEROMETER datas on a "tap" event.
  * @param a buffer to put XYZ datas. Buf must be allocated by caller.
  * TAKE CARE that buf size must be greater than
  * (acc_data->buf1_xyz_size * 2 + acc_data->buf2_xyz_size * 2) otherwise
  * we will get buffer overflow.
  * @retval number of data bytes.
  */
int acc_on_tap_get_xyz_datas(uint16_t *buf)
{
	int s; /* size in uint16_t */

	if (!acc_data.tap)
		return ACC_OK;
	if (!acc_data.xyz_buf2_full)
		return ACC_OK;

	/* tap might occured before we had time to fill buf1 */
	if (acc_data.xyz_buf1_full) {
		s = (acc_data.xyz_buf1 + acc_data.buf1_xyz_size) -
			acc_data.xyz_buf1_next;
		acc_drv_ops->calc_xyz(buf, acc_data.xyz_buf1_next,
			s, acc_data.full_scale);
		acc_drv_ops->calc_xyz(buf + s, acc_data.xyz_buf1,
			acc_data.xyz_buf1_next - acc_data.xyz_buf1,
			acc_data.full_scale);
		s = acc_data.buf1_xyz_size;
	} else {
		s = acc_data.xyz_buf1_next - acc_data.xyz_buf1;
		acc_drv_ops->calc_xyz(buf, acc_data.xyz_buf1, s,
			acc_data.full_scale);
	}
	acc_drv_ops->calc_xyz(buf + s, acc_data.xyz_buf2, acc_data.buf2_xyz_size,
		acc_data.full_scale);
	acc_init_xyz(&acc_data);
	acc_drv_ops->set_rate(acc_data.odr_pre);
	acc_drv_ops->enable_irq();
	TRACE_INFO("%s: done\n", __func__);
	return (s + acc_data.buf2_xyz_size) * 2; /* in bytes */
}

/**
  * @brief ACCELEROMETER FreeRTOS task.
  */
void acc_task(void)
{
	uint16_t wake;
	acc_sby_notifier.call_back = acc_sby;

	irq_sem = xSemaphoreCreateCounting(1, 0);
	if (!irq_sem) {
		TRACE_ERR("%s: failed to create irq sem\n", __func__);
		goto end;
	}
	if (get_board_id() == BOARD_TC3P_MTP) {
		if (acc_init_ais3624dq(&acc_data))
			goto end;
	} else {
		if (acc_init_lis3dsh(&acc_data))
			goto end;
	}

	if (get_board_id() == BOARD_TC3P_MTP)
		wake = PMU_WAKE1;
	else
		wake = ACCEL_INT;

	if (pmu_get_wkup_stat() & wake) {
		if (acc_irq_handler_bottom(&acc_data)) {
			TRACE_ERR("%s: wake-up on accelerometer: failed to handle interrupt\n",
				__func__);
			goto end;
		}
		if (acc_data.tap)
			TRACE_INFO("%s: wake-up on accelerometer: outs=0x%x\n",
				__func__, acc_data.outs);
	}

	/* register to go to stand-by events */
	pm_register_standby(&acc_sby_notifier);

	TRACE_INFO("%s: init done\n", __func__);

	while (1) {
		xSemaphoreTake(irq_sem, portMAX_DELAY);
		if (acc_irq_handler_bottom(&acc_data)) {
			TRACE_ERR("%s: failed to handle interrupt\n", __func__);
			goto sem_del;
		}
	}
sem_del:
	vSemaphoreDelete(irq_sem);
end:
	while (1)
		vTaskDelay(pdMS_TO_TICKS(10));
}

