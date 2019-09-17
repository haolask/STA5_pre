/*
 * sta_i2c_service.h: This file provides all the I2C service definitions.
 *
 * Copyright (C); ST-Microelectronics SA 2015
 * Author: APG-MID team
 */

#ifndef _I2C_SERVICE_H_
#define _I2C_SERVICE_H_

#include "FreeRTOS.h"
#include "semphr.h"

#include "sta_map.h"
#include "sta_i2c.h"

/**
 * @brief Initialize I2C service
 *
 * @param bus_speed		Bus speed
 * @return				error condition
 *
 */
int i2c_service_init(uint32_t bus_speed);

/**
 * @brief Open I2C Port
 *
 * @param i2c_port		Port number (hw specific);
 * @param irq_pri		Interrupt priority of I2C service
 * @param slave_addr	Own address (7 bit);
 * @param start_mode	Initial operating mode (MASTER/SLAVE);
 * @return				error condition
 */
int i2c_open_port(t_i2c *i2c_port, int irq_pri, unsigned slave_addr,
		  uint8_t start_mode);

/**
 * @brief Create COM on I2C port
 *
 * @param i2c_port		I2C port used
 * @param speed_mode	Baud Rate
 * @param address		Slave Address (7 or 10 bits);
 * @return				COM handler pointer
 *
 */
struct i2c_com_handler_s *i2c_create_com(t_i2c *i2c_port,
				  uint32_t speed_mode, uint16_t address);

/**@brief Write buffer on I2C COM
 *
 * @param i2c_com_hdlr    COM handler pointer
 * @param addr_start      peripheral start address
 * @param addr_bytes      number of bytes of start address
 * @param out_buf         pointer to buffer to write
 * @param len             bytes to write
 * @param bytes_in_a_row  bytes to send during a single write operation
 * @param timeout         timeout
 * @return				  error_condition
 *
 */
int i2c_write(struct i2c_com_handler_s * i2c_com_hdlr, uint32_t addr_start, uint32_t addr_bytes,
	      uint8_t * out_buf, uint32_t len, uint32_t bytes_in_a_row, portTickType * timeout);

/**
 * @brief Read buffer on I2C COM
 *
 * @param i2c_com_hdlr  COM handler pointer
 * @param addr_start    peripheral start address
 * @param addr_bytes    number of bytes of start address
 * @param in_buf        pointer to buffer to fill
 * @param len           bytes to receive
 * @param timeout       timeout
 * @return				error_condition
 *
 */
int i2c_read(struct i2c_com_handler_s *i2c_com_hdlr, uint32_t addr_start, uint32_t addr_bytes,
	     uint8_t * in_buf, uint32_t len, portTickType * timeout);

/**
 * @brief   Change I2C bus mode
 *
 * @param   i2c_port    port to change
 * @param   mode        new I2C mode (MASTER/SLAVE);
 * @return			error_condition
 *
 */
int i2c_set_port_mode(t_i2c *i2c_port, uint8_t mode);

/**
 * @brief   Reset I2C port
 *
 * @param   i2c_port    I2C device to reset
 * @return				error_condition
 *
 */
int i2c_reset_port(t_i2c *i2c_port);

void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
void I2C2_IRQHandler(void);
#endif /* _I2C_SERVICE_H_ */
