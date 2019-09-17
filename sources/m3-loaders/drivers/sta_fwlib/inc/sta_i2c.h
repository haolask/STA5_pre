/**
 * @file sta_i2c.h
 * @brief This file provides all the I2C firmware definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_I2C_H_
#define _STA_I2C_H_

#include "sta_map.h"

#define I2C_IRQ_TRANSMIT_FIFO_EMPTY  0x1
#define I2C_IRQ_TRANSMIT_FIFO_NEARLY_EMPTY 0x2
#define I2C_IRQ_TRANSMIT_FIFO_FULL  0x4
#define I2C_IRQ_TRANSMIT_FIFO_OVERRUN  0x8
#define I2C_IRQ_RECEIVE_FIFO_EMPTY  0x10
#define I2C_IRQ_RECEIVE_FIFO_NEARLY_FULL  0x20
#define I2C_IRQ_RECEIVE_FIFO_FULL  0x40
#define I2C_IRQ_READ_FROM_SLAVE_REQUEST  0x10000
#define I2C_IRQ_READ_FROM_SLAVE_EMPTY  0x20000
#define I2C_IRQ_WRITE_TO_SLAVE_REQUEST  0x40000
#define I2C_IRQ_MASTER_TRANSACTION_DONE  0x80000
#define I2C_IRQ_SLAVE_TRANSACTION_DONE  0x100000
#define I2C_IRQ_MASTER_ARBITRATION_LOST  0x1000000
#define I2C_IRQ_BUS_ERROR  0x2000000
#define I2C_IRQ_MASTER_TRANSACTION_DONE_WS  0x10000000
#define I2C_IRQ_ALL_CLEARABLE  0x131F0008
#define I2C_IRQ_ALL  0x131F007F

#define I2C_BUSCTRLMODE_SLAVE				0
#define I2C_BUSCTRLMODE_MASTER				1
#define I2C_BUSCTRLMODE_MASTER_SLAVE		2

#define I2C_SM_STANDARD_MODE 0
#define I2C_SM_FAST_MODE 1
#define I2C_SM_HIGH_SPEED_MODE 2
#define I2C_SM_FAST_MODE_PLUS 3

#define I2C_STANDARD_MODE 100000
#define I2C_FAST_MODE 400000
#define I2C_HIGH_SPEED_MODE 3400000
#define I2C_FAST_MODE_PLUS 10000000

#define I2C_SGCM_TRANSPARENT 0
#define I2C_SGCM_DIRECT 1

#define I2C_DGTLFILTER_OFF 0
#define I2C_DGTLFILTER_1CLK_SPIKES 1
#define I2C_DGTLFILTER_2CLK_SPIKES 2
#define I2C_DGTLFILTER_4CLK_SPIKES 3

#define I2C_ADDRMODE_GENERALCALL 0
#define I2C_ADDRMODE_7BITS 1
#define I2C_ADDRMODE_10BITS 2

#define I2C_MSTMODE_WRITE 0
#define I2C_MSTMODE_READ 1

#define I2C_STARTPROC_DISABLED 0
#define I2C_STARTPROC_ENABLED 1

#define I2C_STOPCOND_REPEATEDSTART 0
#define I2C_STOPCOND_STOP 1

#define I2C_OPSTATUS_MASTER_TX 0
#define I2C_OPSTATUS_MASTER_RX 1
#define I2C_OPSTATUS_SLAVE_RX 2
#define I2C_OPSTATUS_SLAVE_TX 3

#define I2C_CTRLSTATUS_NOP 0
#define I2C_CTRLSTATUS_ON_GOING 1
#define I2C_CTRLSTATUS_OK 2
#define I2C_CTRLSTATUS_ABORT 3

#define I2C_ABORTCAUSE_NACK_ADDR 0
#define I2C_ABORTCAUSE_NACK_DATA 1
#define I2C_ABORTCAUSE_ACK_MCODE 2
#define I2C_ABORTCAUSE_ARB_LOST 3
#define I2C_ABORTCAUSE_BERR_START 4
#define I2C_ABORTCAUSE_BERR_STOP 5
#define I2C_ABORTCAUSE_OVFL 6

#define I2C_RXTYPE_FRAME 0
#define I2C_RXTYPE_GCALL 1
#define I2C_RXTYPE_HW_GCALL 2

struct i2c_config {
	uint8_t digital_filter;
	uint32_t speed_mode;
	uint32_t input_freq;
	uint16_t fifo_tx_threshold;
	uint16_t fifo_rx_threshold;
	uint16_t slave_address;
	uint16_t slave_data_setup_time;
	uint8_t hs_master_code;
	uint8_t start_byte_proc;
};

/**
 * @brief	initialize i2c controller
 * @param	i2c: i2c base address
 * @return	0 if error, not 0 otherwise
 */
int i2c_init(t_i2c *i2c);

/**
 * @brief	exit i2c controller
 * @param	i2c: i2c base address
 * @return	0 if error, not 0 otherwise
 */
int i2c_exit(t_i2c *i2c);

/**
 * @brief	reset i2c registers
 * @param	i2c: i2c base address
 */
void i2c_reset_reg(t_i2c *i2c);

/**
 * @brief	enable i2c
 * @param	i2c: i2c base address
 */
void i2c_enable(t_i2c *i2c);

/**
 * @brief	disable i2c
 * @param	i2c: i2c base address
 */
void i2c_disable(t_i2c *i2c);

/**
 * @brief	set i2c operating mode
 * @param	i2c: i2c base address
 * @param	mode: operating mode
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_operating_mode(t_i2c *i2c, uint8_t mode);

/**
 * @brief	get i2c operating mode
 * @param	i2c: i2c base address
 * @return	mode: operating mode
 */
int i2c_get_operating_mode(t_i2c *i2c);

/**
 * @brief	set i2c speed mode
 * @param	i2c: i2c base address
 * @param	mode: speed mode
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_speed_mode(t_i2c *i2c, uint32_t mode);

/**
 * @brief	get i2c speed mode
 * @param	i2c: i2c base address
 * @return	mode: speed mode
 */
uint32_t i2c_get_speed_mode(t_i2c *i2c);

/**
 * @brief	set i2c slave general mode
 * @param	i2c: i2c base address
 * @param	mode: slave general mode
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_slave_general_call_mode(t_i2c *i2c, uint8_t mode);

/**
 * @brief	get i2c slave general mode
 * @param	i2c: i2c base address
 * @return	mode: slave general mode
 */
uint8_t i2c_get_slave_general_call_mode(t_i2c *i2c);

/**
 * @brief	flush tx circuitry
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_flush_tx(t_i2c *i2c);

/**
 * @brief	flush rx circuitry
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_flush_rx(t_i2c *i2c);

/**
 * @brief	enable loopback mode
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_enable_loop_back(t_i2c *i2c);

/**
 * @brief	disable loopback mode
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_disable_loop_back(t_i2c *i2c);

/**
 * @brief	set status of digital filter on i2c lines
 * @param	i2c: i2c base address
 * @param	filter: filter value
 * @return	none
 */
int i2c_set_digital_filter(t_i2c *i2c, uint8_t filter);

/**
 * @brief	get status of digital filter on i2c lines
 * @param	i2c: i2c base address
 * @return	filter: filter value
 */
uint8_t i2c_get_digital_filter(t_i2c *i2c);

/**
 * @brief	set slave address and corresponding mode
 * @param	i2c: i2c base address
 * @param	sa: slave address
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_slave_addr(t_i2c *i2c, const uint32_t sa);

/**
 * @brief	get slave address mode
 * @param	i2c: i2c base address
 * @return	sa: slave address
 */
uint32_t i2c_get_slave_addr(t_i2c *i2c);

/**
 * @brief	get slave address mode
 * @param	i2c: i2c base address
 * @return	mode: slave address mode
 */
uint8_t i2c_get_slave_addrMode(t_i2c *i2c);

/**
 * @brief	set slave data setup time
 * @param	i2c: i2c base address
 * @param	time: setup time
 */
void i2c_set_slave_data_setup_time(t_i2c *i2c, uint16_t time);

/**
 * @brief	get slave data setup time
 * @param	i2c: i2c base address
 * @return	time: setup time
 */
uint16_t i2c_get_slave_data_setup_time(t_i2c *i2c);

/**
 * @brief	set high speed master code
 * @param	i2c: i2c base address
 * @param	code: master code
 */
void i2c_set_high_speed_master_code(t_i2c *i2c, const uint32_t code);

/**
 * @brief	get high speed master code
 * @param	i2c: i2c base address
 * @return	code: master code
 */
uint8_t i2c_get_high_speed_master_code(t_i2c *i2c);

/**
 * @brief	set master mode
 * @param	i2c: i2c base address
 * @param	mode: master mode
 */
void i2c_set_master_mode(t_i2c *i2c, const uint8_t mode);

/**
 * @brief	get master mode
 * @param	i2c: i2c base address
 * @return	mode: master mode
 */
uint8_t i2c_get_master_mode(t_i2c *i2c);

/**
 * @brief	set destination address
 * @param	i2c: i2c base address
 * @param	da: destination address
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_dest_addr(t_i2c *i2c, const uint32_t da);

/**
 * @brief	get address mode
 * @param	i2c: i2c base address
 * @return	mode: address mode
 */
uint8_t i2c_get_dest_addr_mode(t_i2c *i2c);

/**
 * @brief	set master start byte
 * @param	i2c: i2c base address
 * @param	proc: start byte procedure mode
 */
void i2c_set_master_start_byte_procedure(t_i2c *i2c,
				     const uint8_t proc);

/**
 * @brief	set master stop condition
 * @param	i2c: i2c base address
 * @param	cond: stop condition
 */
void i2c_set_master_stop_condition(t_i2c *i2c, const uint8_t cond);

/**
 * @brief	set number of bytes to transmit/receive during current transaction
 * @param	i2c: i2c base address
 * @param	len: length of bytes to transmit/receive
 */
void i2c_set_master_transaction_length(t_i2c *i2c, const uint8_t len);

/**
 * @brief	get operation status
 * @param	i2c: i2c base address
 * @return	op: operation status
 */
uint8_t i2c_get_operation_status(t_i2c *i2c);

/**
 * @brief	get controller status
 * @param	i2c: i2c base address
 * @return	status: ctrl status
 */
uint8_t i2c_get_controller_status(t_i2c *i2c);

/**
 * @brief	get abort cause
 * @param	i2c: i2c base address
 * @return	cause: abort cause
 */
uint8_t i2c_get_abort_cause(t_i2c *i2c);

/**
 * @brief	get xmit length
 * @param	i2c: i2c base address
 * @return	length: xmit length
 */
uint32_t i2c_get_transmit_length(t_i2c *i2c);

/**
 * @brief	set tx threshold
 * @param	i2c: i2c base address
 * @param	threshold: threshold value
 */
void i2c_set_tx_threshold(t_i2c *i2c, const uint32_t threshold);

/**
 * @brief	set rx threshold
 * @param	i2c: i2c base address
 * @param	threshold: threshold value
 */
void i2c_set_rx_threshold(t_i2c *i2c, const uint32_t threshold);

/**
 * @brief	set baudrate
 * @param	i2c: i2c base address
 * @param	input_freq: input frequency
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_baud_rate(t_i2c *i2c, uint32_t input_freq);

/**
 * @brief	enable interrupt
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
void i2c_interrupt_enable(t_i2c *i2c, const uint32_t irq_mask);

/**
 * @brief	disable interrupt
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
void i2c_interrupt_disable(t_i2c *i2c, const uint32_t irq_mask);

/**
 * @brief	get interrupt status
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
uint32_t i2c_get_interrupt_status(t_i2c *i2c,
				    const uint32_t irq_mask);

/**
 * @brief	get raw interrupt status
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
uint32_t i2c_get_raw_interrupt_status(t_i2c *i2c,
				       const uint32_t irq_mask);

/**
 * @brief	clear interrupt
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
void i2c_clear_interrupt(t_i2c *i2c, const uint32_t irq_mask);

/**
 * @brief	write bytes to fifo until it becomes full
 * @param	i2c: i2c base address
 * @param	buf_ptr: buffer to write
 * @param	buf_size: buffer size
 * @return	number of written bytes
 */
uint32_t i2c_write_fifo(t_i2c *i2c, const uint8_t * buf_ptr, const uint32_t buf_size);

/**
 * @brief	read bytes from fifo until it is empty
 * @param	i2c: i2c base address
 * @param	buf_ptr: buffer to write
 * @param	buf_size: buffer size
 * @return	number of read bytes
 */
uint32_t i2c_read_fifo(t_i2c *i2c, uint8_t * buf_ptr, const uint32_t buf_size);

/**
 * @brief	read bytes from data register
 * @param	i2c: i2c base address
 * @param	p_data: output buffer
 */
void i2c_read_data(t_i2c *i2c, uint8_t * p_data);

/**
 * @brief	configure selected i2c device
 * @param	i2c: i2c base address
 * @param	cfg: configuration structure
 */
void i2c_config(t_i2c *i2c, const struct i2c_config * cfg);

/**
 * @brief	set MCR register
 * @param	i2c: i2c base address
 * @param	op_mode
 * @param	dest_addr
 * @param	start_proc
 * @param	stop_cond
 * @param	len
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_mcr(t_i2c *i2c, const uint8_t op_mode,
	      const uint16_t dest_addr, const uint8_t start_proc,
	      const uint8_t stop_cond, const uint32_t len);

#endif /* _STA_I2C_H_ */
