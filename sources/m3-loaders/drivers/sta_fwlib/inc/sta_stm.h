/**
 * @file sta_stm.h
 * @brief This file provides all STM external objects
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_STM_H_
#define _STA_STM_H_

#include "sta_map.h"


/**
 * enum stp_packet_flags - STP packet modifiers
 */
enum stp_packet_flags {
	STP_PACKET_MARKED	= 0x1,
	STP_PACKET_TIMESTAMPED	= 0x2,
	STP_PACKET_GUARANTEED	= 0x4,
	STP_PAKET_DATA	= 0x8,
	STP_PAKET_FLAG	= 0x10,
	STP_PAKET_TRIG	= 0x20
};

int stm_send_data(t_stm_fifo *stmf, uint16_t channel, const char *pdatabuf, uint32_t nbchar);
int stm_send_char(t_stm_fifo *stmf, uint16_t channel, uint8_t byte, uint32_t flags);


#endif /* _STA_STM_H_ */
