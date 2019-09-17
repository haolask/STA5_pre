/**
 * @file sta_uart.c
 * @brief This file provides all the UART firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <errno.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "utils.h"

#include "sta_mtu.h"
#include "sta_gpio.h"
#include "sta_uart.h"
#include "sta_common.h"
#include "sta_pinmux.h"
#include "sta_nvic.h"

/*
 * Register bits definitions
 */
/* Control Register */
#define CR_UARTEN           0	/* bit 0 */
#define CR_OVSFACT          3	/* bit 3 */
#define CR_LBE              7	/* bit 7 */
#define CR_TXE              8	/* bit 8 */
#define CR_RXE              9	/* bit 7 */
#define CR_DTR              10	/* bit 10 */
#define CR_RTS              11	/* bit 11 */
#define CR_RTSEN            14	/* bit 14 */
#define CR_CTSEN            15	/* bit 15 */

/* TX line control register */
#define LCRH_TX_BRK         0	/* bit 0 */
#define LCRH_TX_PEN_TX      1	/* bit 1 */
#define LCRH_TX_EPS_TX      2	/* bit 2 */
#define LCRH_TX_STP2_TX     3	/* bit 3 */
#define LCRH_TX_FEN_TX      4	/* bit 4 */
#define LCRH_TX_WLEN_TX     5	/* bit 5 & 6 */
#define LCRH_TX_SPS_TX      7	/* bit 7 */

/* Interrupt FIFO Level Select Register */
#define IFLS_TXIFLSEL       0	/* bit 0, 1 & 2 */
#define IFLS_RXIFLSEL       3	/* bit 3, 4 & 5 */

/* Interrupt Clear Register */
#define ICR_CTSRIC          1	/* bit 1 */
#define ICR_RXIC            4	/* bit 4 */
#define ICR_TXIC            5	/* bit 5 */
#define ICR_RTIC            6	/* bit 6 */
#define ICR_FEIC            7	/* bit 7 */
#define ICR_PEIC            8	/* bit 8 */
#define ICR_BEIC            9	/* bit 9 */
#define ICR_OEIC            10	/* bit 10 */
#define ICR_XOFFIC          11	/* bit 11 */
#define ICR_TXFEIC          12	/* bit 12 */

/* Interrupt Mask Set/Clear Register */
#define IMSC_CTSMIM         1	/* bit 1 */
#define IMSC_RXIM           4	/* bit 4 */
#define IMSC_TXIM           5	/* bit 5 */
#define IMSC_RTIM           6	/* bit 6 */
#define IMSC_FEIM           7	/* bit 7 */
#define IMSC_PEIM           8	/* bit 8 */
#define IMSC_BEIM           9	/* bit 9 */
#define IMSC_OEIM           10	/* bit 10 */
#define IMSC_XOFFIM         11	/* bit 11 */
#define IMSC_TXFEIM         12	/* bit 12 */

/* DMA Watermark Register */
#define DMA_W_TXDMAWM       0	/* bit 0, 1 & 2 */
#define DMA_W_RXDMAWM       3	/* bit 3, 4 & 5 */

/* DMA Control Register */
#define DMACR_RXDMAE        0	/* bit 0 */
#define DMACR_TXDMAE        1	/* bit 1 */
#define DMACR_DMAONERR      2	/* bit 2 */

/* Flag Register */
#define FR_CTS              0	/* bit 0 */
#define FR_BUSY             3	/* bit 3 */
#define FR_RXFE             4	/* bit 4 */
#define FR_TXFF             5	/* bit 5 */
#define FR_RXFF             6	/* bit 6 */
#define FR_TXFE             7	/* bit 7 */
#define FR_DCTS             9	/* bit 9 */
#define FR_RTXDIS           13	/* bit 13 */

/* Data Register */
#define DR_DATA             0	/* bit 0 to 7 */
#define DR_FE               8	/* bit 8 */
#define DR_PE               9	/* bit 9 */
#define DR_BE               10	/* bit 10 */
#define DR_OE               11	/* bit 11 */
#define DR_DATA_MSK         0xFF

#ifndef NO_SCHEDULER
struct uart_drv_data {
	uint32_t port;
	xSemaphoreHandle sem;
};

static struct uart_drv_data uarts[4];
#endif

/**
 * @brief	compute UART baud rate and set registers accordingly.
 *		- IBRD = UART_CLK / (16 * BAUD_RATE)
 *		- FBRD = RND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
 *
 * @param	uart_addr: pointer on UART registers
 * @param	port_number: uart port number
 * @param	baudrate: requested baudrate
 * @return	0 if no error, not 0 otherwise
 */
static int uart_set_baud_rate(t_uart *uart, unsigned int port_number, unsigned int baudrate)
{
	uint32_t divisor, fraction;
	uint32_t temp, remainder;

	/* baudrate out of range */
	if (baudrate < BR_MIN_SPEED || baudrate > BR_MAX_SPEED)
		return -EINVAL;

	/* Baud rate divisor computed for OVSFACT = 0 */
	temp = 16 * baudrate;

	if (port_number == UART_P4 || port_number == UART_P5) {
		divisor = UART45_CLK_CONFIG / temp;
		remainder = UART45_CLK_CONFIG % temp;
	} else {
		divisor = UART0123_CLK_CONFIG / temp;
		remainder = UART0123_CLK_CONFIG % temp;
	}

	temp = (8 * remainder) / baudrate;
	fraction = (temp >> 1) + (temp & 1);

	/* Configure baudrate registers */
	uart->ibrd = divisor;
	uart->fbrd = fraction;

	return 0;
}

/**
 * @brief	set parity register according to the parity bit
 * @param	uart_addr: pointer on UART registers
 * @param	parity: parity bit
 * @return	0 if no error, not 0 otherwise
 */
static int uart_set_parity(t_uart *uart, unsigned int parity)
{
	uint32_t reg = uart->lcrh_tx;

	switch (parity) {
	case EVEN_PARITY_BIT:
		reg |= BIT(LCRH_TX_PEN_TX);
		reg |= BIT(LCRH_TX_EPS_TX);
		reg &= ~BIT(LCRH_TX_SPS_TX);
		break;

	case ODD_PARITY_BIT:
		reg |= BIT(LCRH_TX_PEN_TX);
		reg &= ~BIT(LCRH_TX_EPS_TX);
		reg &= ~BIT(LCRH_TX_SPS_TX);
		break;

	case PARITY_0_BIT:
		reg |= BIT(LCRH_TX_PEN_TX);
		reg |= BIT(LCRH_TX_EPS_TX);
		reg |= BIT(LCRH_TX_SPS_TX);
		break;

	case PARITY_1_BIT:
		reg |= BIT(LCRH_TX_PEN_TX);
		reg &= ~BIT(LCRH_TX_EPS_TX);
		reg |= BIT(LCRH_TX_SPS_TX);
		break;

	case NOPARITY_BIT:
	default:
		/* by default NOPARITY_BIT */
		break;
	}

	uart->lcrh_tx = reg;
	return 0;
}

/**
 * @brief	clear all interrupts in a row
 * @param	uart_addr: pointer on UART registers
 * @return	none
 */
static void uart_clear_all_interrupts(t_uart *uart)
{
	uint32_t reg = uart->icr;

	/* All It are cleared */
	reg |= BIT(ICR_CTSRIC) |	/* ClearToSend Modem */
	    BIT(ICR_RXIC) |	/* Received */
	    BIT(ICR_TXIC) |	/* Transmit */
	    BIT(ICR_RTIC) |	/* ReceivedTimeout */
	    BIT(ICR_FEIC) |	/* FramingError */
	    BIT(ICR_PEIC) |	/* ParityError */
	    BIT(ICR_BEIC) |	/* BreakError */
	    BIT(ICR_OEIC) |	/* OverrunError */
	    BIT(ICR_XOFFIC) |	/* XOFF */
	    BIT(ICR_TXFEIC);	/* TX FIFO Empty */

	uart->icr = reg;
}

/**
 * @brief	check if the Tx Fifo is full
 * @param	uart_addr: pointer on UART registers
 * @return	true if Tx fifo is full, false otherwise
 */
static inline bool uart_is_tx_fifo_full(t_uart *uart)
{
	return ((uart->fr & BIT(FR_TXFF)) != 0);
}

/**
 * @brief	check if the Tx Fifo is empty
 * @param	uart_addr: pointer on UART registers
 * @return	true if Tx fifo is empty, false otherwise
 */
static inline bool uart_is_tx_fifo_empty(t_uart *uart)
{
	return ((uart->fr & BIT(FR_TXFE)) != 0);
}

/**
 * @brief	check if the Rx Fifo is empty
 * @param	uart_addr: pointer on UART registers
 * @return	true if Rx fifo is empty, false otherwise
 */
static inline uint32_t uart_is_rx_fifo_empty(t_uart *uart)
{
	return ((uart->fr & BIT(FR_RXFE)) != 0);
}

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
			unsigned int stop_bits, unsigned int data_bits)
{
	uint32_t reg;
	t_uart *uart;

	uint32_t dataflushed;	/* Variable to flush Rx fifo */

	switch (port_number) {
	case UART_P0: {
		pinmux_request("uart0_mux");
		uart = uart0_regs;
#ifndef NO_SCHEDULER
		struct nvic_chnl irq_chnl;

		irq_chnl.id = UART0_IRQChannel;
		irq_chnl.preempt_prio = IRQ_LOW_PRIO;
		irq_chnl.enabled = true;
		nvic_chnl_init(&irq_chnl);
		uarts[port_number].sem = xSemaphoreCreateBinary();
#endif
		break;
	}
	case UART_P1: {
		pinmux_request("uart1_mux");
		uart = uart1_regs;
		break;
	}
	case UART_P2: {
		pinmux_request("uart2_mux");
		uart = uart2_regs;
		break;
	}
	case UART_P3: {
		pinmux_request("uart3_mux");
		uart = uart3_regs;
		break;
	}
	case UART_P4: {
		misc_a7_regs->misc_reg66.bit.soc_uart_select = 1;
		pinmux_request("uart4_mux");
		uart = uart4_regs;
		break;
	}
	case UART_P5: {
		misc_a7_regs->misc_reg66.bit.soc_uart_select = 1;
		pinmux_request("uart5_mux");
		uart = uart5_regs;
		break;
	}

	case NO_UART:
		/* No initialization */
	default:
		return NULL;
	}

	/* clear receive status register/error clear register */
	uart->rsr = 0;

	/* enable Uart functionality */
	uart->cr = BIT(CR_UARTEN);

	/* Configure baudrate (MUST be performed before a LCRH write) */
	uart_set_baud_rate(uart, port_number, baudrate);

	/*
	 * Configure TX line control register:
	 *   * 0=1 stop bits, 1=2 stop bits
	 *   * Enable Tx Fifo
	 *   * 00=5bits, 01=6bits, 10=7bits, 11=8bits
	 */
	reg = (stop_bits << LCRH_TX_STP2_TX) |
	    BIT(LCRH_TX_FEN_TX) | (data_bits << LCRH_TX_WLEN_TX);
	uart->lcrh_tx = reg;

	uart_set_parity(uart, parity_bit);

	/* Configure RX line control register iso TX line */
	uart->lcrh_rx = uart->lcrh_tx;

	/* Set to 0 characters the interrupt FIFO level for transmit and receive */
	uart->ifls = 0;

	uart_clear_all_interrupts(uart);

	/* Enable IT */
	uart->imsc = BIT(IMSC_FEIM) |	/* FramingError */
		  BIT(IMSC_PEIM) |	/* ParityError */
		  BIT(IMSC_BEIM) |	/* BreakError */
		  BIT(IMSC_OEIM);

	/* Transmit DMA Watermark set to 8 characters */
	uart->dma_w = 3 << DMA_W_TXDMAWM;

	/* Transmit DMA Enable */
	uart->dmacr = BIT(DMACR_TXDMAE);

	/* enable RX */
	uart->cr |= BIT(CR_RXE);

	while (!uart_is_rx_fifo_empty(uart))	/* flush Rxfifo: Do a Rx fifo reading to empty it */
		dataflushed = uart->data; //read_reg(uart_addr + UART_DR);
	(void)dataflushed;	/* Unused variable: make compiler happy */

	/* enable TX */
	uart->cr |= BIT(CR_TXE);
	while (!uart_is_tx_fifo_empty(uart)) ;	/* Flush the Tx FIFO */

	return uart;
}

/**
 * @brief	send synchronously the given number of bytes from given address in
 * polling mode.
 * @param	uart_addr: pointer on UART registers
 * @param	pdatabuf: pointer on the buffer of data to transmit
 * @param	nbchar:  number of characters to transmit
 * @return	0 if no error, not 0 otherwise
 */
int uart_send_data(t_uart *uart, char *pdatabuf,
		   uint32_t nbchar)
{
	while (nbchar) {	/* stops writing if transfer's end */
		while (uart_is_tx_fifo_full(uart)) ;

		uart->data = *((uint8_t *) pdatabuf++);
		nbchar--;
	}

	return 0;
}

/**
 * @brief	copy synchronously the received data in the buffer supplied as
 * parameter until the total amount of bytes has been received. This function
 * use polling instead of interrupts.
 * @param	uart_addr: pointer on UART registers
 * @param	pdatabuf: pointer on the buffer of data to transmit
 * @param	nbchar:  number of characters to transmit
 * @return	0 if no error, not 0 otherwise
 */
int uart_get_data(t_uart *uart, char *pdatabuf, uint32_t nbchar)
{
	uint32_t rx_data;

	while (nbchar) {
		/* check that Rx Fifo not empty in polling mode: in this case, wait for a new byte */
		while (uart_is_rx_fifo_empty(uart)) ;

		/* read receive data */
		rx_data = uart->data; //read_reg(uart_addr + UART_DR);

		/* Check if FramingError or ParityError or BreakError or OverunError */
		if (rx_data &
		    (BIT(DR_FE) | BIT(DR_PE) | BIT(DR_BE) | BIT(DR_OE)))
			return -EIO;

		*pdatabuf++ = (char)rx_data;	/* 1 byte read */
		nbchar--;
	}

	return 0;
}

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
			      uint32_t nbchar, uint32_t udelay)
{
	struct mtu_device *mtu;
	uint32_t rx_data;
	uint32_t timeout;

	while (nbchar != 0) {
		/* Configure and start timer to expire after Timeout value */
		mtu = mtu_start_timer(udelay, ONE_SHOT_MODE, NULL);

		/* Loop till that Rx Fifo is empty in polling mode
		   and the TIMER value is not zero (timeout condition) */
		do {
			timeout = mtu_read_timer_value(mtu);
		} while (uart_is_rx_fifo_empty(uart) && timeout);

		mtu_stop_timer(mtu);

		if (timeout) {
			/* read receive data */
			rx_data = uart->data; //read_reg(uart_addr + UART_DR);

			/* Check if FramingError or ParityError or BreakError or OverunError */
			if (rx_data &
			    (BIT(DR_FE) | BIT(DR_PE) | BIT(DR_BE) | BIT(DR_OE)))
				return -EIO;

			*pdatabuf++ = (char)rx_data;	/* 1 byte read */
			nbchar--;
		} else
			return -ETIMEDOUT;
	}
	return 0;
}

/**
 * @brief	wait for Rx data
 * @param	uart_addr: pointer on UART registers
 */
void uart_wait_for_data(t_uart *uart)
{
#ifdef NO_SCHEDULER
	while (uart_is_rx_fifo_empty(uart));
#else
	if (!uart_is_rx_fifo_empty(uart))
		return;
	if (uart == uart0_regs) {
		uart->imsc |= BIT(IMSC_RXIM);
		xSemaphoreTake(uarts[UART_P0].sem, portMAX_DELAY);
	} else {
		while (uart_is_rx_fifo_empty(uart))
			vTaskDelay(pdMS_TO_TICKS(50));
	}
#endif
}

/**
 * @brief	Flush the uart tx fifo
 * @param	uart_addr: pointer on UART registers
 * @return	None
 */
void uart_flush_data(t_uart *uart)
{
	while (!uart_is_tx_fifo_empty(uart));
}

#ifndef NO_SCHEDULER
void UART0_IRQHandler(void)
{
	t_uart *uart = uart0_regs;

	if (uart->ris & BIT(IMSC_RXIM)) {
		uart->icr = BIT(IMSC_RXIM);
		uart->imsc &= ~BIT(IMSC_RXIM);
		xSemaphoreGiveFromISR(uarts[UART_P0].sem, NULL);
	}
}
#endif
