/**
 * @file sta_stm.c
 * @brief This file provides the STM FIFO access feature
 * The initial dev is based on this requirement
 *	STM block setting is done by A7.
 *	so no control feature have been implemented in this driver
 *	only fifo writing access
 * It leads a major constraint :
 *		M3 alone can not evacuates STM traces
 *		early boot traces are not covered
 * in the future if needed this will be enhanced
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <errno.h>
#include <string.h>


#include "utils.h"

#include "sta_stm.h"
#include "sta_common.h"


#define STM_FLAG_TIMESTAMPED   BIT(3)
#define STM_FLAG_GUARANTEED    BIT(7)
#define STM_FLAG_MARKED		   BIT(4)


enum stm_pkt_type {
	STM_PKT_TYPE_DATA	= 0x98,
	STM_PKT_TYPE_FLAG	= 0xE8,
	STM_PKT_TYPE_TRIG	= 0xF8,
};

#define STM_32_CHANNEL			32
#define BYTES_PER_CHANNEL		256


#define stm_channel_addr(stm_fifo, ch)	(&stmf->fifo_channels[ch])


/**
 * @brief	return base address of FIFO location of STM block
 */
t_stm_fifo *stm_init(void)
{
	t_stm_fifo *stm;

	stm = stm_fifo_regs;
	return stm;
}


static inline uint32_t stm_channel_off(uint32_t type, uint32_t opts)
{
	uint32_t channel_off = 0;

	switch (type) {

	case STP_PAKET_DATA:
		channel_off = STM_PKT_TYPE_DATA;
		if (opts & STP_PACKET_MARKED)
			channel_off &= ~STM_FLAG_MARKED;
		if (opts & STP_PACKET_TIMESTAMPED)
			channel_off &= ~STM_FLAG_TIMESTAMPED;
		if (opts & STP_PACKET_GUARANTEED)
			channel_off &= ~STM_FLAG_GUARANTEED;
		break;

	case STP_PAKET_FLAG:
		channel_off = STM_PKT_TYPE_FLAG;
		if (opts & STP_PACKET_TIMESTAMPED)
			channel_off &= ~STM_FLAG_TIMESTAMPED;
		if (opts & STP_PACKET_GUARANTEED)
			channel_off &= ~STM_FLAG_GUARANTEED;
		break;

	case STP_PAKET_TRIG:
		channel_off = STM_PKT_TYPE_TRIG;
		if (opts & STP_PACKET_TIMESTAMPED)
			channel_off &= ~STM_FLAG_TIMESTAMPED;
		if (opts & STP_PACKET_GUARANTEED)
			channel_off &= ~STM_FLAG_GUARANTEED;
		break;
	}

	return channel_off;
}



static inline bool stm_addr_unaligned(const void *addr, uint8_t write_bytes)
{
	return ((unsigned long)addr & (write_bytes - 1));
}

static void stm_send(void *addr, const void *data, uint32_t size)
{
	uint8_t paload[8];

	if (stm_addr_unaligned(data, 4)) {
		memcpy(paload, data, size);
		data = paload;
	}

	/* now we are 64bit/32bit aligned */
	switch (size) {

	case 4:
		write_reg(*(uint32_t *)data, (uint32_t)addr);
		break;
	case 2:
		write_short_reg(*(uint16_t *)data, (uint32_t)addr);
		break;
	case 1:
		write_byte_reg(*(uint8_t *)data, (uint32_t)addr);
		break;
	default:
		break;
	}
}


/**
 * @brief	send buffer to stm channel with this fix pattern (D32TS D32...Dxs FLAG)
 *
 * @param	stmf: pointer on STM fifo registers
 * @param	channel: channel stm
 * @param	pdatabuf: pointer on the buffer of data to transmit
 * @param	nbchar:  number of characters to transmit
 * @return	0 if no error, not 0 otherwise
 */
int stm_send_data(t_stm_fifo *stmf, uint16_t channel, const char *pdatabuf, uint32_t nbchar)
{
	uint32_t flags = STP_PACKET_TIMESTAMPED;
	const char *p;
	uint32_t pos;
	uint32_t sz;
	t_stm_fifo_one_channel *ch_addr = NULL;
	uint8_t FlagPayload = 0;

	if (!stmf || !pdatabuf)
		return -1;

	for (pos = 0, p = pdatabuf; nbchar > pos; pos += sz, p += sz) {
		sz = nbchar-pos > 4?4:nbchar-pos;
		ch_addr = stm_channel_addr(stmf, channel);
		ch_addr = (t_stm_fifo_one_channel *)((uint32_t)ch_addr | stm_channel_off(STP_PAKET_DATA, flags));
		stm_send((void *)ch_addr, p, sz);
		flags = 0;
	}

	ch_addr = stm_channel_addr(stmf, channel);
	ch_addr = (t_stm_fifo_one_channel *)((uint32_t)ch_addr | stm_channel_off(STP_PAKET_FLAG, flags));
	stm_send((void *)ch_addr, &FlagPayload, 1);

	return pos;
}

/**
 * @brief	send a byte to stm channel with option requested
 *
 * @param	uart_addr: pointer on UART registers
 * @param	channel: channel stm
 * @param	byte : content to be evacuated
 * @param	flags:  STM option (TimeStamp / Mark / Guaranteed )
 * @return	0 if no error, not 0 otherwise
 */

int stm_send_char(t_stm_fifo *stmf, uint16_t channel, uint8_t byte, uint32_t flags)
{
	t_stm_fifo_one_channel *ch_addr = NULL;

	if (!stmf)
		return -1;

	ch_addr = stm_channel_addr(stmf, channel);
	ch_addr = (t_stm_fifo_one_channel *)((uint32_t)ch_addr | stm_channel_off(STP_PAKET_DATA, flags));
	stm_send((void *)ch_addr, &byte, 1);

	return 0;
}

