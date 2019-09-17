/**
 * @file sta_i2c.c
 * @brief This file provides all the I2C firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>

#include "sta_i2c.h"
#include "sta_nvic.h"
#include "sta_common.h"
#include "trace.h"
#include "sta_pinmux.h"

/* I2C_CR register */
#define OM_MASK				3
#define SM_MASK				3
#define FON_MASK			3

/* I2C_SCR register */
#define SA7_MASK			0x7F
#define ESA10_MASK			0x3FF
#define SLSU_MASK			0xFFFF

/* I2C_HSMCR register */
#define MC_MASK				7

/* I2C_MCR register */
#define A7					1
#define A7_MASK				0x7F
#define EA10				1
#define EA10_MASK			0x3FF
#define SB					11
#define AM					12
#define AM_MASK				3
#define P					14
#define LENGTH				15
#define LENGTH_MASK			0x7FF

/* I2C_SR register */
#define OP_S_MASK			3
#define STATUS_MASK			3
#define CAUSE_MASK			7
#define TYPE_MASK			3
#define LENGTH_S_MASK		0x7FF

/* I2C_RFTR and I2C_TFTR registers */
#define THRESHOLD_MASK		0xF

/* I2C_BRCR register */
#define BRCNT_MASK			0xFFFF

/* I2C_RFR register */
#define RDATA_MASK			0xFF

#define A7_MAX				128
#define A10_MAX				1024

#define DELAY_STD 10000

/**
 * @brief	initialize i2c controller
 * @param	i2c: i2c base address
 * @return	0 if error, not 0 otherwise
 */
int i2c_init(t_i2c *i2c)
{
	struct nvic_chnl irq_chnl;
	uint32_t i2c_addr = (uint32_t) i2c;

	switch(i2c_addr) {
	case I2C0_BASE: {
		pinmux_request("i2c0_mux");
		irq_chnl.id = I2C0_IRQChannel;
		break;
	}
	case I2C1_BASE: {
		pinmux_request("i2c1_mux");
		irq_chnl.id = EXT7_IRQChannel;
		break;
	}
	case I2C2_BASE: {
		pinmux_request("i2c2_mux");
		irq_chnl.id = EXT11_IRQChannel;
		break;
	}
	default:
		TRACE_ERR("%s: invalid i2c port\n", __func__);
		return -EINVAL;
	}

	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;

	nvic_chnl_init(&irq_chnl);

	return 0;
}

/**
 * @brief	exit i2c controller
 * @param	i2c: i2c base address
 * @return	0 if error, not 0 otherwise
 */
int i2c_exit(t_i2c *i2c)
{
	struct nvic_chnl irq_chnl;
	uint32_t i2c_addr = (uint32_t) i2c;

	switch(i2c_addr) {
	case I2C0_BASE: {
		irq_chnl.id = I2C0_IRQChannel;
		break;
	}
	case I2C1_BASE: {
		irq_chnl.id = EXT7_IRQChannel;
		break;
	}
	case I2C2_BASE: {
		irq_chnl.id = EXT11_IRQChannel;
		break;
	}
	default:
		TRACE_ERR("%s: invalid i2c port\n", __func__);
		return -EINVAL;
	}
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = false;
	nvic_chnl_init(&irq_chnl);
	return 0;
}

/**
 * @brief	reset i2c registers
 * @param	i2c: i2c base address
 */
void i2c_reset_reg(t_i2c *i2c)
{
	i2c->i2c_cr.reg       = 0;
	i2c->i2c_scr.reg      = 0;
	i2c->i2c_hsmcr.reg    = 0;
	i2c->i2c_mcr.reg      = 0;
	i2c->i2c_tftr.reg     = 0;
	i2c->i2c_rftr.reg     = 0;
	i2c->i2c_brcr.reg     = 0;
	i2c->i2c_imscr.reg    = 0;
	i2c->i2c_icr.reg      = (uint32_t)~0;
}

/**
 * @brief	enable i2c
 * @param	i2c: i2c base address
 */
void i2c_enable(t_i2c *i2c)
{
	i2c->i2c_cr.bit.pe = 1;
}

/**
 * @brief	disable i2c
 * @param	i2c: i2c base address
 */
void i2c_disable(t_i2c *i2c)
{
	i2c->i2c_cr.bit.pe = 0;
}

/**
 * @brief	set i2c operating mode
 * @param	i2c: i2c base address
 * @param	mode: operating mode
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_operating_mode(t_i2c *i2c, uint8_t mode)
{
	if (mode > I2C_BUSCTRLMODE_MASTER_SLAVE)
		return -EINVAL;

	if (i2c->i2c_cr.bit.pe)
		return -EBUSY;

	i2c->i2c_cr.bit.om = mode & OM_MASK;

	return 0;
}

/**
 * @brief	get i2c operating mode
 * @param	i2c: i2c base address
 * @return	mode: operating mode
 */
int i2c_get_operating_mode(t_i2c *i2c)
{
	return i2c->i2c_cr.bit.om & OM_MASK;
}

/**
 * @brief	set i2c speed mode
 * @param	i2c: i2c base address
 * @param	mode: speed mode
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_speed_mode(t_i2c *i2c, uint32_t mode)
{
	switch(mode) {
	case I2C_STANDARD_MODE:
		i2c->i2c_cr.bit.sm = 0;
		break;
	case I2C_FAST_MODE:
		i2c->i2c_cr.bit.sm = 1;
		break;
	case I2C_HIGH_SPEED_MODE:
		i2c->i2c_cr.bit.sm = 2;
		break;
	case I2C_FAST_MODE_PLUS:
		i2c->i2c_cr.bit.sm = 3;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief	get i2c speed mode
 * @param	i2c: i2c base address
 * @return	mode: speed mode
 */
uint32_t i2c_get_speed_mode(t_i2c *i2c)
{
	uint8_t sm = i2c->i2c_cr.bit.sm;

	switch(sm) {
	case 0:
		return I2C_STANDARD_MODE;
	case 1:
		return I2C_FAST_MODE;
	case 2:
		return I2C_HIGH_SPEED_MODE;
	case 3:
		return I2C_FAST_MODE_PLUS;
	default: /* should not happen */
		break;
	}

	return I2C_STANDARD_MODE;
}

/**
 * @brief	set i2c slave general mode
 * @param	i2c: i2c base address
 * @param	mode: slave general mode
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_slave_general_call_mode(t_i2c *i2c, uint8_t mode)
{
	if (mode > I2C_SGCM_DIRECT)
		return -EINVAL;

	i2c->i2c_cr.bit.sgcm = !!mode;

	return 0;
}

/**
 * @brief	get i2c slave general mode
 * @param	i2c: i2c base address
 * @return	mode: slave general mode
 */
uint8_t i2c_get_slave_general_call_mode(t_i2c *i2c)
{
	return i2c->i2c_cr.bit.sgcm;
}

/**
 * @brief	flush tx circuitry
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_flush_tx(t_i2c *i2c)
{
	i2c->i2c_cr.bit.ftx = 1;
	while( i2c->i2c_cr.bit.ftx);
}

/**
 * @brief	flush rx circuitry
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_flush_rx(t_i2c *i2c)
{
	i2c->i2c_cr.bit.frx = 1;
	while( i2c->i2c_cr.bit.frx);
}

/**
 * @brief	enable loopback mode
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_enable_loop_back(t_i2c *i2c)
{
	i2c->i2c_cr.bit.lm = 1;
}

/**
 * @brief	disable loopback mode
 * @param	i2c: i2c base address
 * @return	none
 */
void i2c_disable_loop_back(t_i2c *i2c)
{
	i2c->i2c_cr.bit.lm = 0;
}

/**
 * @brief	set status of digital filter on i2c lines
 * @param	i2c: i2c base address
 * @param	filter: filter value
 * @return	none
 */
int i2c_set_digital_filter(t_i2c *i2c, uint8_t filter)
{
	if (filter > I2C_DGTLFILTER_4CLK_SPIKES)
		return -EINVAL;

	i2c->i2c_cr.bit.fon = filter & FON_MASK;

	return 0;
}

/**
 * @brief	get status of digital filter on i2c lines
 * @param	i2c: i2c base address
 * @return	filter: filter value
 */
uint8_t i2c_get_digital_filter(t_i2c *i2c)
{
	return i2c->i2c_cr.bit.fon;
}

/**
 * @brief	set slave address and corresponding mode
 * @param	i2c: i2c base address
 * @param	sa: slave address
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_slave_addr(t_i2c *i2c, const uint32_t sa)
{
	if (sa < A7_MAX) {
		i2c->i2c_scr.bit.sa10 =	(sa & SA7_MASK);
		/* 7 bits mode */
		i2c->i2c_cr.bit.sam = 0;
	} else if (sa >= A7_MAX && sa < A10_MAX) {
		i2c->i2c_scr.bit.sa10 =	(sa & ESA10_MASK);
		/* 10 bits mode */
		i2c->i2c_cr.bit.sam = 1;
	} else {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief	get slave address mode
 * @param	i2c: i2c base address
 * @return	sa: slave address
 */
uint32_t i2c_get_slave_addr(t_i2c *i2c)
{
	return i2c->i2c_scr.bit.sa10;
}

/**
 * @brief	get slave address mode
 * @param	i2c: i2c base address
 * @return	mode: slave address mode
 */
uint8_t i2c_get_slave_addr_mode(t_i2c *i2c)
{
	return i2c->i2c_cr.bit.sam;
}

/**
 * @brief	set slave data setup time
 * @param	i2c: i2c base address
 * @param	time: setup time
 */
void i2c_set_slave_data_setup_time(t_i2c *i2c, uint16_t time)
{
	i2c->i2c_scr.bit.slsu = time & SLSU_MASK;
}

/**
 * @brief	get slave data setup time
 * @param	i2c: i2c base address
 * @return	time: setup time
 */
uint16_t i2c_get_slave_data_setup_time(t_i2c *i2c)
{
	return i2c->i2c_scr.bit.slsu;
}

/**
 * @brief	set high speed master code
 * @param	i2c: i2c base address
 * @param	code: master code
 */
void i2c_set_high_speed_master_code(t_i2c *i2c, const uint32_t code)
{
	i2c->i2c_hsmcr.bit.mc = code & MC_MASK;
    i2c->i2c_cr.bit.sm = I2C_SM_HIGH_SPEED_MODE;
}

/**
 * @brief	get high speed master code
 * @param	i2c: i2c base address
 * @return	code: master code
 */
uint8_t i2c_get_high_speed_master_code(t_i2c *i2c)
{
	return i2c->i2c_hsmcr.bit.mc;
}

/**
 * @brief	set master mode
 * @param	i2c: i2c base address
 * @param	mode: master mode
 */
void i2c_set_master_mode(t_i2c *i2c, const uint8_t mode)
{
	i2c->i2c_mcr.bit.op = !!mode;
}

/**
 * @brief	get master mode
 * @param	i2c: i2c base address
 * @return	mode: master mode
 */
uint8_t i2c_get_master_mode(t_i2c *i2c)
{
	return i2c->i2c_mcr.bit.op;
}

/**
 * @brief	set destination address
 * @param	i2c: i2c base address
 * @param	da: destination address
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_dest_addr(t_i2c *i2c, const uint32_t da)
{
	if (da < A7_MAX) {
		i2c->i2c_mcr.bit.a10 = (da & A7_MASK);
		i2c->i2c_mcr.bit.am = I2C_ADDRMODE_7BITS;
	} else if (da >= A7_MAX && da < A10_MAX) {
		i2c->i2c_mcr.bit.a10 = (da & EA10_MASK);
		i2c->i2c_mcr.bit.am = I2C_ADDRMODE_10BITS;
	} else {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief	get address mode
 * @param	i2c: i2c base address
 * @return	mode: address mode
 */
uint8_t i2c_get_dest_addr_mode(t_i2c *i2c)
{
	return i2c->i2c_mcr.bit.am;
}

/**
 * @brief	set master start byte
 * @param	i2c: i2c base address
 * @param	proc: start byte procedure mode
 */
void i2c_set_master_start_byte_procedure(t_i2c *i2c,
				     const uint8_t proc)
{
	i2c->i2c_mcr.bit.sb = !!proc;
}

/**
 * @brief	set master stop condition
 * @param	i2c: i2c base address
 * @param	cond: stop condition
 */
void i2c_set_master_stop_condition(t_i2c *i2c, const uint8_t cond)
{
	i2c->i2c_mcr.bit.p = !!cond;
}

/**
 * @brief	set number of bytes to transmit/receive during current transaction
 * @param	i2c: i2c base address
 * @param	len: length of bytes to transmit/receive
 */
void i2c_set_master_transaction_length(t_i2c *i2c, const uint8_t len)
{
	i2c->i2c_mcr.bit.length = len & LENGTH_MASK;
}

/**
 * @brief	get operation status
 * @param	i2c: i2c base address
 * @return	op: operation status
 */
uint8_t i2c_get_operation_status(t_i2c *i2c)
{
	return i2c->i2c_sr.bit.op;
}

/**
 * @brief	get controller status
 * @param	i2c: i2c base address
 * @return	status: ctrl status
 */
uint8_t i2c_get_controller_status(t_i2c *i2c)
{
	return i2c->i2c_sr.bit.status;
}

/**
 * @brief	get abort cause
 * @param	i2c: i2c base address
 * @return	cause: abort cause
 */
uint8_t i2c_get_abort_cause(t_i2c *i2c)
{
	return i2c->i2c_sr.bit.cause;
}

/**
 * @brief	get xmit length
 * @param	i2c: i2c base address
 * @return	length: xmit length
 */
uint32_t i2c_get_transmit_length(t_i2c *i2c)
{
	return i2c->i2c_sr.bit.length;
}

/**
 * @brief	set tx threshold
 * @param	i2c: i2c base address
 * @param	threshold: threshold value
 */
void i2c_set_tx_threshold(t_i2c *i2c, const uint32_t threshold)
{
	i2c->i2c_tftr.bit.threshold_tx = threshold & THRESHOLD_MASK;
}

/**
 * @brief	set rx threshold
 * @param	i2c: i2c base address
 * @param	threshold: threshold value
 */
void i2c_set_rx_threshold(t_i2c *i2c, const uint32_t threshold)
{
	i2c->i2c_rftr.bit.threshold_rx = threshold & THRESHOLD_MASK;
}

/**
 * @brief	set baudrate
 * @param	i2c: i2c base address
 * @param	input_freq: input frequency
 * @return	0 if no error, not 0 otherwise
 */
int i2c_set_baud_rate(t_i2c *i2c, uint32_t input_freq)
{
	uint32_t status = i2c->i2c_cr.bit.pe;
	uint32_t speed_mode = i2c_get_speed_mode(i2c);

	if (input_freq == 0)
		return -EINVAL;

	i2c->i2c_cr.bit.pe = 0;

	switch (speed_mode) {
	case I2C_STANDARD_MODE:
		i2c->i2c_brcr.bit.brcnt2 = input_freq / (speed_mode * 2);
		i2c->i2c_brcr.bit.brcnt1 = 0;
		break;
	case I2C_FAST_MODE:
        i2c->i2c_brcr.bit.brcnt2 = (input_freq * 2) / (speed_mode * 3);
        i2c->i2c_brcr.bit.brcnt1 = 0;
		break;
	case I2C_HIGH_SPEED_MODE:
		i2c->i2c_brcr.bit.brcnt2 = 0;
        i2c->i2c_brcr.bit.brcnt1 = (input_freq * 2) / (speed_mode * 3);
		break;
	default:
		return -EINVAL;
	}

	i2c->i2c_cr.bit.pe = status;

	return 0;
}

/**
 * @brief	enable interrupt
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
void i2c_interrupt_enable(t_i2c *i2c, const uint32_t irq_mask)
{
	i2c->i2c_imscr.reg |= irq_mask;
}

/**
 * @brief	disable interrupt
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
void i2c_interrupt_disable(t_i2c *i2c, const uint32_t irq_mask)
{
	i2c->i2c_imscr.reg &= ~irq_mask;
}

/**
 * @brief	get interrupt status
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
uint32_t i2c_get_interrupt_status(t_i2c *i2c,
				    const uint32_t irq_mask)
{
	return i2c->i2c_misr.reg & irq_mask;
}

/**
 * @brief	get raw interrupt status
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
uint32_t i2c_get_raw_interrupt_status(t_i2c *i2c,
				       const uint32_t irq_mask)
{
	return i2c->i2c_risr.reg & irq_mask;
}

/**
 * @brief	clear interrupt
 * @param	i2c: i2c base address
 * @param	irq_mask: irq mask
 */
void i2c_clear_interrupt(t_i2c *i2c, const uint32_t irq_mask)
{
	i2c->i2c_icr.reg = irq_mask;
}

/**
 * @brief	write bytes to fifo until it becomes full
 * @param	i2c: i2c base address
 * @param	buf_ptr: buffer to write
 * @param	buf_size: buffer size
 * @return	number of written bytes
 */
uint32_t i2c_write_fifo(t_i2c *i2c, const uint8_t * buf_ptr, const uint32_t buf_size)
{
	uint8_t cnt = 0;

	while ((cnt < buf_size)
	       && !i2c_get_raw_interrupt_status(i2c, I2C_IRQ_TRANSMIT_FIFO_FULL)) {
		i2c->i2c_tfr.reg = buf_ptr[cnt++];
	}

	return cnt++;
}

/**
 * @brief	read bytes from fifo until it is empty
 * @param	i2c: i2c base address
 * @param	buf_ptr: buffer to write
 * @param	buf_size: buffer size
 * @return	number of read bytes
 */
uint32_t i2c_read_fifo(t_i2c *i2c, uint8_t * buf_ptr, const uint32_t buf_size)
{
	uint8_t cnt = 0;

	while ((cnt < buf_size)
	       && !i2c_get_raw_interrupt_status(i2c, I2C_IRQ_RECEIVE_FIFO_EMPTY)) {
		if (buf_ptr != NULL)
			buf_ptr[cnt++] = i2c->i2c_rfr.reg;
	}

	return cnt++;
}

/**
 * @brief	read bytes from data register
 * @param	i2c: i2c base address
 * @param	p_data: output buffer
 */
void i2c_read_data(t_i2c *i2c, uint8_t * p_data)
{
	while (!i2c_get_raw_interrupt_status(i2c, I2C_IRQ_RECEIVE_FIFO_EMPTY)) {
		*p_data = (i2c->i2c_rfr.bit.rdata & RDATA_MASK);
	}
}

/**
 * @brief	configure selected i2c device
 * @param	i2c: i2c base address
 * @param	cfg: configuration structure
 */
void i2c_config(t_i2c *i2c, const struct i2c_config * cfg)
{
	i2c_disable(i2c);
	i2c_interrupt_disable(i2c, I2C_IRQ_ALL);

	i2c_set_speed_mode(i2c, cfg->speed_mode);
	i2c_set_baud_rate(i2c, cfg->input_freq);

	i2c_set_digital_filter(i2c, cfg->digital_filter);
	i2c_set_slave_data_setup_time(i2c, cfg->slave_data_setup_time);

	i2c_disable_loop_back(i2c);
	i2c_set_slave_general_call_mode(i2c, I2C_SGCM_TRANSPARENT);

	i2c_set_tx_threshold(i2c, cfg->fifo_tx_threshold);
	i2c_set_rx_threshold(i2c, cfg->fifo_rx_threshold);

	i2c_set_slave_addr(i2c, cfg->slave_address);

	if (cfg->speed_mode == I2C_HIGH_SPEED_MODE) {
		i2c_set_high_speed_master_code(i2c, cfg->hs_master_code);
	}
	/* Set default operating mode to slave. */
	i2c_set_operating_mode(i2c, I2C_BUSCTRLMODE_SLAVE);
}

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
	      const uint8_t stop_cond, const uint32_t len)
{
	uint32_t mcr_temp = 0;
	uint32_t local_addr;
	uint8_t mode;

	mcr_temp = op_mode;

	if ((dest_addr > 7) && (dest_addr < 120)) {
		local_addr = dest_addr & 0x7f;
		mode = I2C_ADDRMODE_7BITS;
	} else if ((dest_addr > 127) && (dest_addr < 1024)) {
		local_addr = dest_addr;
		mode = I2C_ADDRMODE_10BITS;
	} else {
		return -EINVAL;
	}

	mcr_temp |= (local_addr & EA10_MASK) << EA10;
	mcr_temp |= (mode & AM_MASK) << AM;

	if (start_proc == I2C_STARTPROC_DISABLED)
		mcr_temp &= ~BIT(SB);
	else
		mcr_temp |= BIT(SB);

	if (stop_cond == I2C_STOPCOND_REPEATEDSTART)
		mcr_temp &= ~BIT(P);
	else
		mcr_temp |= BIT(P);

	mcr_temp |= (len & LENGTH_MASK) << LENGTH;
    i2c->i2c_mcr.reg = mcr_temp;

	return 0;
}

