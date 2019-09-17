/**
 * @file tca9535.c
 * @brief TI TCA9535 GPIO expander driver
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG team
 */

/* Includes ------------------------------------------------------------------*/
#include "sta_mtu.h"
#include "sta_i2c.h"
#include "tca9535.h"

#define TCA9535_MAX_INSTANCE    1
#define I2C_TCA9535_PORT        i2c0_regs
#define I2C_READWRITE_DELAY     50
#define I2C_CLOCK               51200000

/* Private variables ---------------------------------------------------------*/
static uint8_t tca9535[TCA9535_MAX_INSTANCE];

/* I2C bus configuration */
static const struct i2c_config i2c0_config = {
	I2C_DGTLFILTER_OFF,
	I2C_FAST_MODE,
	I2C_CLOCK,
	8,
	8,
	0,
	0,
	0,
	I2C_STARTPROC_DISABLED
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Check if the device instance of the selected address is already
 *         registered and return its index
 * @param  dev_addr: Device address on communication Bus.
 * @retval Index of the device instance if registered, 0xFF if not.
 */
static int tca9535_get_instance(uint16_t dev_addr)
{
	int idx = 0;

	/* Check all the registered instances */
	for (idx = 0; idx < TCA9535_MAX_INSTANCE ; idx++) {
		/* Return index if there is address match */
		if (tca9535[idx] == dev_addr)
			return idx;
	}

	return -1;
}

/**
 * @brief  reads multiple data from TCA9535
 * @param  i2c_addr: I2C address
 * @param  reg_addr: Register address
 * @param  buf: Pointer to data buffer
 * @param  len: Length of the data
 * @retval Number of read data or < 0 if error
 */
static int tca_read_multiple(uint8_t i2c_addr, uint8_t reg_addr,
			     uint8_t *buf, int len)
{
	int instance;
	int buf_len = 0;
	int timeout = 500000; /* 500ms */

	instance = tca9535_get_instance(i2c_addr);
	if (instance < 0 || len <= 0)
		return -1;

	/* Configure R/W = 0 */
	i2c_set_mcr(I2C_TCA9535_PORT, I2C_MSTMODE_WRITE, i2c_addr,
		    I2C_STARTPROC_DISABLED, I2C_STOPCOND_REPEATEDSTART, 1);
	/* Peripheral Enable */
	i2c_enable(I2C_TCA9535_PORT);
	/* Send Read Register Address */
	i2c_write_fifo(I2C_TCA9535_PORT, &reg_addr, 1);
	/* Wait a delay depending on bus speed and check operation status */
	mtu_wait_delay(I2C_READWRITE_DELAY);
	/* If after timeout, status is abort => exit error */
	if (i2c_get_controller_status(I2C_TCA9535_PORT) == I2C_CTRLSTATUS_ABORT)
		return -1;

	/* Configure R/W = 1 */
	i2c_set_mcr(I2C_TCA9535_PORT, I2C_MSTMODE_READ, i2c_addr,
		    I2C_STARTPROC_DISABLED, I2C_STOPCOND_STOP, len);
	/* Peripheral Enable */
	i2c_enable(I2C_TCA9535_PORT);

	/* Loop until all bytes are read */
	while (len > 0 && timeout > 0) {
		int rlen;

		mtu_wait_delay(I2C_READWRITE_DELAY);
		timeout -= I2C_READWRITE_DELAY;

		if (i2c_get_controller_status(I2C_TCA9535_PORT)
					== I2C_CTRLSTATUS_ABORT) {
			return -1;
		}
		rlen = i2c_read_fifo(I2C_TCA9535_PORT, buf, len);
		buf_len += rlen;
		len -= rlen;
		buf += rlen;
	}

	return buf_len;
}

/**
 * @brief  writes multiple data from TCA9535
 * @param  i2c_addr: I2C address
 * @param  reg_addr: Register address
 * @param  buf: Pointer to data buffer
 * @param  len: Length of the data
 * @retval Number of write data or < 0 if error
 */
static int tca_write_multiple(uint8_t i2c_addr, uint8_t reg_addr,
			      uint8_t *buf, int len)
{
	int instance;
	int wlen;
	int buf_len = len;
	int timeout = 500000; /* 500ms */
	uint8_t status;

	instance = tca9535_get_instance(i2c_addr);
	if (instance < 0 || len <= 0)
		return -1;

	i2c_set_mcr(I2C_TCA9535_PORT, I2C_MSTMODE_WRITE, i2c_addr,
		    I2C_STARTPROC_DISABLED, I2C_STOPCOND_STOP, len + 1);
	i2c_enable(I2C_TCA9535_PORT);

	/* Send start address for write operation */
	i2c_write_fifo(I2C_TCA9535_PORT, &reg_addr, 1);

	while (len > 0) {
		/* Send bytes to FIFO till it is full or no more byte must be written */
		wlen =  i2c_write_fifo(I2C_TCA9535_PORT, buf, len);
		buf += wlen;
		len -= wlen;

		/* Wait a delay depending on bus speed and check operation status */
		mtu_wait_delay(I2C_READWRITE_DELAY);

		status = i2c_get_controller_status(I2C_TCA9535_PORT);
		switch (status) {
		case I2C_CTRLSTATUS_ABORT: /* exit with error */
			return -1;
		case I2C_CTRLSTATUS_OK:
			break;
		default:
			/* otherwise wait for end of write */
			while (status == I2C_CTRLSTATUS_ON_GOING &&
			       timeout > 0) {
				mtu_wait_delay(I2C_READWRITE_DELAY);
				timeout -= I2C_READWRITE_DELAY;
				status = i2c_get_controller_status(I2C_TCA9535_PORT);
			}
			if (timeout <= 0 || status == I2C_CTRLSTATUS_ABORT)
				return -1;
			break;
		}
	}
	return buf_len - len;
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief  Initialize the TCA9535 and configure the needed hardware resources
 * @param  dev_addr: Device address on communication Bus.
 * @retval 0 if no error else < 0
 */
int tca9535_init(uint16_t dev_addr)
{
	int instance;
	int empty;

	/* Check if device instance already exists */
	instance = tca9535_get_instance(dev_addr);

	if (instance < 0) {
		/* Look for empty instance */
		empty = tca9535_get_instance(0);

		if (empty < TCA9535_MAX_INSTANCE) {
			/* Register the current device instance */
			tca9535[empty] = dev_addr;

			/* Here I2C bus layer must be enabled */
			if (!i2c_init(I2C_TCA9535_PORT)) {
				i2c_reset_reg(I2C_TCA9535_PORT);
				i2c_config(I2C_TCA9535_PORT, &i2c0_config);
				i2c_set_operating_mode(I2C_TCA9535_PORT,
						       I2C_BUSCTRLMODE_MASTER);
				return 0;
			}
		}
	}
	return -1;
}

/**
 * @brief  Exit TCA9535
 * @param  dev_addr: Device address on communication Bus.
 * @retval 0 if no error else < 0
 */
int tca9535_exit(uint16_t dev_addr)
{
	int idx;

	idx = tca9535_get_instance(dev_addr);
	if (idx < 0)
		return -1;

	tca9535[idx] = 0;
	i2c_exit(I2C_TCA9535_PORT);
	return 0;
}

/**
 * @brief  Initialize the selected pin(s) direction.
 * @param  dev_addr: Device address on communication Bus.
 * @param  io_pin: IO pin(s) to be configured.
 *         This parameter could be any combination of the following values:
 *   @arg  TCA9535_PIN_x: where x can be from 0 to 15.
 * @param  direction: could be TCA9535_DIRECTION_IN or STMPE1600_DIRECTION_OUT.
 * @retval status >= 0 if no error
 */
int tca9535_io_init_pin(uint16_t dev_addr,
			uint16_t io_pin,
			uint8_t direction)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	tca_read_multiple(dev_addr, TCA9535_REG_CFG_PORT_0, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Set the Pin direction */
	if (direction != TCA9535_DIRECTION_IN)
		tmp |= io_pin;
	else
		tmp &= ~io_pin;

	/* Set the new register value */
	return tca_write_multiple(dev_addr, TCA9535_REG_CFG_PORT_0,
				  (uint8_t *)&tmp, 2);
}

/**
 * @brief  Configure the IO pin(s) according to IO mode structure value.
 * @param  dev_addr: Device address on communication Bus.
 * @param  io_pin: The output pin to be set or reset. This parameter can be one
 *         of the following values:
 *   @arg  TCA9535_PIN_x: where x can be from 0 to 7.
 * @param  direction: The IO pin mode to configure, could be one of the following
 *         values:
 *   @arg  TCA9535_DIRECTION_IN
 *   @arg  TCA9535_DIRECTION_OUT
 * @retval None
 */
void tca9535_io_config(uint16_t dev_addr,
		       uint16_t io_pin,
		       uint8_t direction) {
	/* Configure IO pin according to selected direction */
	tca9535_io_init_pin(dev_addr, io_pin, direction);
}

/**
 * @brief  Set the value of the selected IO pins.
 * @param  dev_addr: Device address on communication Bus.
 * @param  io_pin: IO pin(s) to be set.
 *         This parameter could be any combination of the following values:
 *   @arg  TCA9535_PIN_x: where x can be from 0 to 15.
 * @param  pin_state: The value to be set.
 * @retval None
 */
void tca9535_io_write_pin(uint16_t dev_addr, uint16_t io_pin,
			  uint8_t pin_state)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	tca_read_multiple(dev_addr, TCA9535_REG_RD_PORT_0, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Set the pin state */
	if (pin_state != 0)
		tmp |= io_pin;
	else
		tmp &= ~io_pin;

	/* Set the new register value */
	tca_write_multiple(dev_addr, TCA9535_REG_WR_PORT_0, (uint8_t *)&tmp, 2);
}

/**
 * @brief  Read the state of the selected IO pin(s).
 * @param  dev_addr: Device address on communication Bus.
 * @param  io_pin: IO pin(s) to be read.
 *         This parameter could be any combination of the following values:
 *   @arg  TCA9535_PIN_x: where x can be from 0 to 15.
 * @retval State of the selected IO pin(s).
 */
uint16_t tca9535_io_read_pin(uint16_t dev_addr, uint16_t io_pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the register value */
	tca_read_multiple(dev_addr, TCA9535_REG_RD_PORT_0, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Return the pin(s) state */
	return (tmp & io_pin);
}

