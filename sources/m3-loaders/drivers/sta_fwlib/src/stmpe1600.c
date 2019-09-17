/**
  ******************************************************************************
  * @file    stmpe1600.c
  * @author  MCD Application Team & APG-MID Application Team
  * @version V2.0.0
  * @date    22-MAy-2015
  * @brief   This file provides a set of functions needed to manage the STMPE1600
  *          IO Expander devices.
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

/* Includes ------------------------------------------------------------------*/
#include "sta_mtu.h"
#include "sta_i2c.h"
#include "stmpe1600.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* #define STMPE1600_POLARITY_INV */ /* Uncomment to enable polariry inv */
#define STMPE1600_NO_IT				/* Comment to enable IT management */

#define STMPE1600_MAX_INSTANCE  1
#define I2C_STMPE1600_PORT      i2c0_regs
#define I2C_READWRITE_DELAY     50
#define I2C_CLOCK               51200000

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t stmpe1600[STMPE1600_MAX_INSTANCE];

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
  * @brief  Check if the device instance of the selected address is already registered
  *         and return its index
  * @param  dev_addr: Device address on communication Bus.
  * @retval Index of the device instance if registered, 0xFF if not.
  */
static int stmpe1600_GetInstance(uint16_t dev_addr)
{
	int idx = 0;

	/* Check all the registered instances */
	for (idx = 0; idx < STMPE1600_MAX_INSTANCE ; idx ++) {
		/* Return index if there is address match */
		if (stmpe1600[idx] == dev_addr)
			return idx;
	}

	return -1;
}

/**
  * @brief  reads multiple data from STMPE1600
  * @param  i2c_addr: I2C address
  * @param  reg_addr: Register address
  * @param  buf: Pointer to data buffer
  * @param  len: Length of the data
  * @retval Number of read data or < 0 if error
  */
static int STMPE_ReadMultiple(uint8_t i2c_addr, uint8_t reg_addr,
		uint8_t *buf, int len)
{
	int instance;
	int buf_len = 0;;
	int timeout = 500000; /* 500ms */

	instance = stmpe1600_GetInstance(i2c_addr);
	if (instance < 0 || len <= 0)
		return -1;

	/* Send address */
	i2c_set_mcr(I2C_STMPE1600_PORT, I2C_MSTMODE_WRITE, i2c_addr, I2C_STARTPROC_DISABLED, I2C_STOPCOND_REPEATEDSTART, 1);
	i2c_enable(I2C_STMPE1600_PORT);
	i2c_write_fifo(I2C_STMPE1600_PORT, &reg_addr, 1);
	/* Wait a delay depending on bus speed and check operation status */
	mtu_wait_delay(I2C_READWRITE_DELAY);
	/* If after timeout, status is abort => exit error */
	if (i2c_get_controller_status(I2C_STMPE1600_PORT) == I2C_CTRLSTATUS_ABORT)
		return -1;

	i2c_set_mcr(I2C_STMPE1600_PORT, I2C_MSTMODE_READ,
			i2c_addr, I2C_STARTPROC_DISABLED, I2C_STOPCOND_STOP, len);
	i2c_enable(I2C_STMPE1600_PORT);

	/* Loop until all bytes are read */
	while (len > 0 && timeout > 0) {
		int rlen;

		mtu_wait_delay(I2C_READWRITE_DELAY);
		timeout -= I2C_READWRITE_DELAY;

		if (i2c_get_controller_status(I2C_STMPE1600_PORT) == I2C_CTRLSTATUS_ABORT)
			return -1;
		rlen = i2c_read_fifo(I2C_STMPE1600_PORT, buf, len);
		buf_len += rlen;
		len -= rlen;
		buf += rlen;
	}

	return buf_len;
}

/**
  * @brief  writes multiple data from STMPE1600
  * @param  i2c_addr: I2C address
  * @param  reg_addr: Register address
  * @param  buf: Pointer to data buffer
  * @param  len: Length of the data
  * @retval Number of write data or < 0 if error
  */
static int STMPE_WriteMultiple(uint8_t i2c_addr, uint8_t reg_addr,
		uint8_t *buf, int len)
{
	int instance;
	int wlen;
	int buf_len = len;
	int timeout = 500000; /* 500ms */
	uint8_t status;

	instance = stmpe1600_GetInstance(i2c_addr);
	if (instance < 0 || len <= 0)
		return -1;

	i2c_set_mcr( I2C_STMPE1600_PORT, I2C_MSTMODE_WRITE, i2c_addr,
			I2C_STARTPROC_DISABLED, I2C_STOPCOND_STOP, len + 1);
	i2c_enable(I2C_STMPE1600_PORT);

	/* Send start address for write operation */
	i2c_write_fifo(I2C_STMPE1600_PORT, &reg_addr, 1);

	while (len > 0) {
		/* Send bytes to FIFO till it is full or no more byte must be written */
		wlen =  i2c_write_fifo(I2C_STMPE1600_PORT, buf, len);
		buf += wlen;
		len -= wlen;

		/* Wait a delay depending on bus speed and check operation status */
		mtu_wait_delay(I2C_READWRITE_DELAY);

		status = i2c_get_controller_status(I2C_STMPE1600_PORT);
		switch (status) {
		case I2C_CTRLSTATUS_ABORT: /* exit with error */
			return -1;
		case I2C_CTRLSTATUS_OK:
			break;
		default:
			/* otherwise wait for end of write */
			while (status == I2C_CTRLSTATUS_ON_GOING && timeout > 0) {
				mtu_wait_delay(I2C_READWRITE_DELAY);
				timeout -= I2C_READWRITE_DELAY;
				status = i2c_get_controller_status(I2C_STMPE1600_PORT);
			}
			if (timeout <= 0 || status == I2C_CTRLSTATUS_ABORT)
				return -1;
			break;
		}
	}
	return buf_len - len;
}

#ifndef STMPE1600_NO_IT
/**
  * @brief  Reads single data from STMPE1600
  * @param  i2c_addr: I2C address
  * @param  reg_addr: Register address
  * @retval Read data or < 0 if error
  */
static int STMPE_Read(uint8_t i2c_addr, uint8_t reg_addr)
{
	uint8_t data;

	if (STMPE_ReadMultiple(i2c_addr, reg_addr, &data, 1) > 0)
		return (int) data;
	else
		return -1;
}
#endif /* STMPE1600_NO_IT */

/**
  * @brief  Writes single byte to the STMPE1600
  * @param  i2c_addr : STMPE1600's I2C address
  * @param  reg_addr : STMPE1600's internal register address to write to
  * @param  data: the data to be written to the STMPE register
  * @retval status
  */
static int STMPE_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t data)
{
	return STMPE_WriteMultiple(i2c_addr, reg_addr, &data, 1);
}


/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize the STMPE1600 and configure the needed hardware resources
  * @param  dev_addr: Device address on communication Bus.
  * @retval 0 if no error else < 0
  */
int stmpe1600_Init(uint16_t dev_addr)
{
	int instance;
	int empty;

	/* Check if device instance already exists */
	instance = stmpe1600_GetInstance(dev_addr);

	if (instance < 0) {
		/* Look for empty instance */
		empty = stmpe1600_GetInstance(0);

		if (empty < STMPE1600_MAX_INSTANCE) {
			/* Register the current device instance */
			stmpe1600[empty] = dev_addr;

			/* Here I2C bus layer must be enabled */
			if (!i2c_init(I2C_STMPE1600_PORT)) {
				i2c_reset_reg(I2C_STMPE1600_PORT);
				i2c_config(I2C_STMPE1600_PORT, &i2c0_config);
				i2c_set_operating_mode(I2C_STMPE1600_PORT, I2C_BUSCTRLMODE_MASTER);
				/* Generate STMPE1600 Software reset */
				return stmpe1600_Reset(dev_addr);
			}
		}
	}
	return -1;
}

/**
  * @brief  Exit STMPE1600
  * @param  dev_addr: Device address on communication Bus.
  * @retval 0 if no error else < 0
  */
int stmpe1600_Exit(uint16_t dev_addr)
{
	int idx = stmpe1600_GetInstance(dev_addr);
	if (idx < 0)
		return -1;

	stmpe1600[idx] = 0;
	i2c_exit(I2C_STMPE1600_PORT);
	return 0;
}

/**
  * @brief  Reset the STMPE1600 by Software.
  * @param  dev_addr: Device address on communication Bus.
  * @retval 0 if OK else < 0
  */
int stmpe1600_Reset(uint16_t dev_addr)
{
	/* Power Down the STMPE1600 */
	if (STMPE_write(dev_addr, STMPE1600_REG_SYS_CTRL, (uint16_t)0x80) < 0)
		return -1;

	/* Wait for a delay to ensure registers erasing */
	mdelay(1); /* 1ms */

	/* Power On the Codec after the power off: all regs are reinitialized */
	if (STMPE_write(dev_addr, STMPE1600_REG_SYS_CTRL, (uint16_t)0x00) < 0)
		return -1;

	/* Wait for a delay to ensure registers erasing */
	mdelay(1); /* 1ms */

	return 0;
}

/**
  * @brief  Read the STMPE1600 device ID.
  * @param  dev_addr: Device address on communication Bus.
  * @retval The Device ID (two bytes).
  */
uint16_t stmpe1600_ReadID(uint16_t dev_addr)
{
	uint8_t data[2];

	/* Read the STMPE1600 device ID */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_CHP_ID, data, 2);

	/* Return the device ID value */
	return ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));
}

#ifndef STMPE1600_NO_IT
/**
  * @brief  Set the global interrupt Polarity.
  * @param  dev_addr: Device address on communication Bus.
  * @param  Polarity: could be one of these values;
  *   @arg  STMPE1600_POLARITY_LOW: Interrupt line is active Low/Falling edge
  *   @arg  STMPE1600_POLARITY_HIGH: Interrupt line is active High/Rising edge
  * @retval None
  */
void stmpe1600_SetITPolarity(uint16_t dev_addr, uint8_t Polarity)
{
	uint8_t tmp;

	/* Get the current register value */
	tmp = (uint8_t) STMPE_Read(dev_addr, STMPE1600_REG_SYS_CTRL);

	/* Mask the polarity bit */
	tmp &= ~0x01;

	/* Set the Interrupt Output line polarity */
	tmp |= Polarity;

	/* Set the new register value */
	STMPE_write(dev_addr, STMPE1600_REG_SYS_CTRL, tmp);
}

/**
  * @brief  Enable the Global interrupt.
  * @param  dev_addr: Device address on communication Bus.
  * @retval None
  */
void stmpe1600_EnableGlobalIT(uint16_t dev_addr)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Configure NVIC IT for STMPE1600 */
	#error "TODO: port STMPE_ITConfig() on STA"

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_SYS_CTRL, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Set the global interrupts to be Enabled */
	tmp |= STMPE1600_IT_ENABLE;

	/* Write Back the Interrupt Control register */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_SYS_CTRL, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Disable the Global interrupt.
  * @param  dev_addr: Device address on communication Bus.
  * @retval None
  */
void stmpe1600_DisableGlobalIT(uint16_t dev_addr)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_SYS_CTRL, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Set the global interrupts to be Enabled */
	tmp &= ~(uint16_t)STMPE1600_IT_ENABLE;

	/* Write Back the Interrupt Control register */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_SYS_CTRL, (uint8_t *)&tmp, 2);
}

#endif /* STMPE1600_NO_IT */

/**
  * @brief  Initialize the selected pin(s) direction.
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be configured.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @param  direction: could be STMPE1600_DIRECTION_IN or STMPE1600_DIRECTION_OUT.
  * @retval None
  */
void stmpe1600_IO_InitPin(uint16_t dev_addr,
		uint16_t IO_Pin,
		uint8_t direction)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_GPDR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Set the Pin direction */
	if (direction != STMPE1600_DIRECTION_IN)
		tmp |= IO_Pin;
	else
		tmp &= ~IO_Pin;

	/* Set the new register value */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_GPDR, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Configure the IO pin(s) according to IO mode structure value.
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: The output pin to be set or reset. This parameter can be one
  *         of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 7.
  * @param  IO_Mode: The IO pin mode to configure, could be one of the following values:
  *   @arg  IO_MODE_INPUT
  *   @arg  IO_MODE_OUTPUT
  *   @arg  IO_MODE_IT_RISING_EDGE
  *   @arg  IO_MODE_IT_FALLING_EDGE
  * @retval None
  */
void stmpe1600_IO_Config(uint16_t dev_addr,
		uint16_t IO_Pin,
		IO_ModeTypedef IO_Mode)
{
#ifndef STMPE1600_NO_IT
	uint8_t buffer[2];
#endif

	/* Configure IO pin according to selected IO mode */
	switch (IO_Mode) {
	case IO_MODE_INPUT: /* Input mode */
		stmpe1600_IO_DisablePinIT(dev_addr, IO_Pin);
		stmpe1600_IO_InitPin(dev_addr, IO_Pin, STMPE1600_DIRECTION_IN);
		break;

	case IO_MODE_OUTPUT: /* Output mode */
		stmpe1600_IO_DisablePinIT(dev_addr, IO_Pin);
		stmpe1600_IO_InitPin(dev_addr, IO_Pin, STMPE1600_DIRECTION_OUT);
		break;

#ifndef STMPE1600_NO_IT
	case IO_MODE_IT_RISING_EDGE: /* Interrupt rising edge mode */
		stmpe1600_SetITPolarity(dev_addr, STMPE1600_POLARITY_HIGH);
		stmpe1600_IO_EnablePinIT(dev_addr, IO_Pin);
		stmpe1600_IO_InitPin(dev_addr, IO_Pin, STMPE1600_DIRECTION_IN);
		/* Clear all IO IT pending bits if any */
		stmpe1600_IO_ClearIT(dev_addr, IO_Pin);

		/* Read GMPR to enable interrupt */
		STMPE_ReadMultiple(dev_addr , STMPE1600_REG_GPMR, buffer, 2);
		break;

	case IO_MODE_IT_FALLING_EDGE: /* Interrupt falling edge mode */
		stmpe1600_SetITPolarity(dev_addr, STMPE1600_POLARITY_LOW);
		stmpe1600_IO_EnablePinIT(dev_addr, IO_Pin);
		stmpe1600_IO_InitPin(dev_addr, IO_Pin, STMPE1600_DIRECTION_IN);

		/* Clear all IO IT pending bits if any */
		stmpe1600_IO_ClearIT(dev_addr, IO_Pin);

		/* Read GMPR to enable interrupt */
		STMPE_ReadMultiple(dev_addr , STMPE1600_REG_GPMR, buffer, 2);
		break;
#endif /* STMPE1600_NO_IT */

	default:
		break;
	}
}

#ifdef STMPE1600_POLARITY_INV

/**
  * @brief  Enable polarity inversion of the selected IO pin(s).
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be configured.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval None
  */
void stmpe1600_IO_PolarityInv_Enable(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_GPPIR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Enable pin polarity inversion */
	tmp |= IO_Pin;

	/* Set the new register value */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_GPPIR, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Disable polarity inversion of the selected IO pins.
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be configured.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval None
  */
void stmpe1600_IO_PolarityInv_Disable(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_GPPIR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Disable pin polarity inversion */
	tmp &= ~IO_Pin;

	/* Set the new register value */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_GPPIR, (uint8_t *)&tmp, 2);
}

#endif

/**
  * @brief  Set the value of the selected IO pins.
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be set.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @param  pin_state: The value to be set.
  * @retval None
  */
void stmpe1600_IO_WritePin(uint16_t dev_addr, uint16_t IO_Pin,
		uint8_t pin_state)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_GPMR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Set the pin state */
	if (pin_state != 0)
		tmp |= IO_Pin;
	else
		tmp &= ~IO_Pin;

	/* Set the new register value */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_GPSR, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Read the state of the selected IO pin(s).
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be read.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval State of the selected IO pin(s).
  */
uint16_t stmpe1600_IO_ReadPin(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_GPMR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Return the pin(s) state */
	return (tmp & IO_Pin);
}

#ifndef STMPE1600_NO_IT
/**
  * @brief  Enable the interrupt mode for the selected IO pin(s).
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be configured.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval None
  */
void stmpe1600_IO_EnablePinIT(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Enable global interrupt */
	stmpe1600_EnableGlobalIT(dev_addr);

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_IEGPIOR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Put pin in IT mode */
	tmp |= IO_Pin;

	/* Write the new register value */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_IEGPIOR, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Disable the interrupt mode for the selected IO pin(s).
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be configured.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval None
  */
void stmpe1600_IO_DisablePinIT(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the current register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_IEGPIOR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Disable the IT pin mode */
	tmp &= ~IO_Pin;

	/* Set the new register value */
	STMPE_WriteMultiple(dev_addr, STMPE1600_REG_IEGPIOR, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Read the IT status of the selected IO pin(s)
  *         (clears all the pending bits if any).
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be checked.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval IT Status of the selected IO pin(s).
  */
uint8_t stmpe1600_IO_ITStatus(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_ISGPIOR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Return the pin IT status */
	return ((tmp & IO_Pin) == IO_Pin);
}

/**
  * @brief  Detect an IT pending bit from the selected IO pin(s).
  *         (clears all the pending bits if any).
  * @param  dev_addr: Device address on communication Bus.
  * @param  IO_Pin: IO pin(s) to be checked.
  *         This parameter could be any combination of the following values:
  *   @arg  STMPE1600_PIN_x: where x can be from 0 to 15.
  * @retval IT pending bit detection status.
  */
uint8_t stmpe1600_IO_ReadIT(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];
	uint16_t tmp;

	/* Get the register value */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_ISGPIOR, data, 2);

	tmp = ((uint16_t)data[0] | (((uint16_t)data[1]) << 8));

	/* Return if there is an IT pending bit or not */
	return (tmp & IO_Pin);
}

/**
  * @brief  Clear all the IT pending bits if any.
  * @param  dev_addr: Device address on communication Bus.
  * @retval None
  */
void stmpe1600_IO_ClearIT(uint16_t dev_addr, uint16_t IO_Pin)
{
	uint8_t data[2];

	/* Get the register value to clear all pending bits */
	STMPE_ReadMultiple(dev_addr, STMPE1600_REG_ISGPIOR, data, 2);
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
