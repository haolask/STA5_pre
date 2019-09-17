/**
 * @file sta_uart.h
 * @brief This file provides all the UART firmware header.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_UART_H_
#define _STA_UART_H_

#include "sta_map.h"

#define UART0123_CLK_CONFIG (get_soc_id() == SOCID_STA1385 ? 52000000 : 51200000)
#define UART45_CLK_CONFIG 166400000

/* UART Port choice */
enum t_uart_number {
	UART_P0 = 0,
	UART_P1,
	UART_P2,
	UART_P3,
	UART_P4,
	UART_P5,
	NO_UART
};

enum t_uart_parity_bit {
	NOPARITY_BIT,
	EVEN_PARITY_BIT,
	ODD_PARITY_BIT,
	PARITY_0_BIT,		// stick parity mode
	PARITY_1_BIT		// stick parity mode
};

enum t_uart_data_bits {
	DATABITS_5 = 0x0,
	DATABITS_6 = 0x1,
	DATABITS_7 = 0x2,
	DATABITS_8 = 0x3
};

enum t_uart_stop_bits {
	ONE_STOPBIT = 0x0,
	TWO_STOPBITS = 0x1
};

enum t_uart_baudrate {
	BR_MIN_SPEED = 110,
	BR110BAUD = BR_MIN_SPEED,
	BR1200BAUD = 1200,
	BR2400BAUD = 2400,
	BR9600BAUD = 9600,
	BR38400BAUD = 38400,
	BR115200BAUD = 115200,
	BR230400BAUD = 230400,
	BR460800BAUD = 460800,
	BR921600BAUD = 921600,
	BR1843200BAUD = 1843200,
	BR3000000BAUD = 3000000,
	BR_MAX_SPEED = BR3000000BAUD
};

/**
 * @brief	configure UART registers and check PCell Id, flush Tx and Rx fifo
 * to enable a new transfer
 * @param	uart_addr: pointer on UART registers
 * @param	port_number: identify UART to access
 * @param	baudrate:  baudrate
 * @param	parity_bit:parity
 * @param	stop_bits: stop bits
 * @param	data_bits: data bits
 * @return	uart base address, NULL on error
 */
t_uart *uart_init(uint32_t port_number,
			unsigned int baudrate,
			unsigned int parity_bit,
			unsigned int stop_bits, unsigned int data_bits);

/**
 * @brief	send synchronously the given number of bytes from given address in
 * polling mode.
 * @param	uart_addr: pointer on UART registers
 * @param	pdatabuf: pointer on the buffer of data to transmit
 * @param	nbchar:  number of characters to transmit
 * @return	0 if no error, not 0 otherwise
 */
int uart_send_data(t_uart *uart, char *pdatabuf,
		   uint32_t nbchar);

/**
 * @brief	copy synchronously the received data in the buffer supplied as
 * parameter until the total amount of bytes has been received. This function
 * use polling instead of interrupts.
 * @param	uart_addr: pointer on UART registers
 * @param	pdatabuf: pointer on the buffer of data to transmit
 * @param	nbchar:  number of characters to transmit
 * @return	0 if no error, not 0 otherwise
 */
int uart_get_data(t_uart *uart, char *pdatabuf,
		  uint32_t nbchar);

/**
 * @brief	wait for the timer value to reach zero. Then, copy synchronously
 * the received data in the buffer supplied as parameter until the total
 * amount of bytes has been copied.
 * This function use polling instead of interrupts.
 * @param	uart_addr: pointer on UART registers
 * @param	pdatabuf: pointer on the buffer of data to transmit
 * @param	nbchar:  number of characters to transmit
 * @param	udelay:  dealy in us
 * @return	0 if no error, not 0 otherwise
 */
int uart_get_data_until_Timer(t_uart *uart, char *pdatabuf,
			      uint32_t nbchar, uint32_t udelay);

/**
 * @brief	wait for Rx data
 * @param	uart_addr: pointer on UART registers
 */
void uart_wait_for_data(t_uart *uart);

/**
 * @brief	Flush the uart tx fifo
 * @param	uart_addr: pointer on UART registers
 * @return	None
 */
void uart_flush_data(t_uart *uart);

#endif /* _STA_UART_H_ */
