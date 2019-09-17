/**
 * @file trace.h
 * @brief trace & debug header file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_TRACE_H__
#define __STA_TRACE_H__

#include <stdint.h>
#include "sta_type.h"

/*
 * IMPORTANT!!!: Avoid to use trace_* functions or macro in ISR routines
 * it impacts the real time behavior
 */

#define TRACE_EARLY_BUF_SIZE 1024

#define	_CR_CRLF		1	/* 1: Convert \n ==> \r\n in the output char */

enum port_e {
	UART0,  /* UART_P0 */
	UART1,  /* UART_P1 */
	UART2,  /* UART_P2 */
	UART3,  /* UART_P3 */
	UART4,  /* UART_P4 */
	UART5,  /* UART_P5 */
	UART_MAX_NO,
	JTAG_DCC_PORT = 0x80,
	NO_TRACE_PORT = -1,
};

/**
 * @struct trace_t
 * @brief Trace structure
 */
struct trace_t {
	char *start; /**< start address of the trace buffer */
	char *end; /**< end address of the trace buffer */
	char *wrap; /**< wrap pointer if needed */
	char *buf; /**< trace buffer */
	enum port_e port; /**< default port */
	bool buf_ovf;
#if defined TRACE_EARLY_BUF
	char trace_early_buf[TRACE_EARLY_BUF_SIZE];
#endif
};

extern struct trace_t trace;

/**
  * @brief  This routine initializes the trace logging path. The messages can be
  * printed either over a circular buffer passed as parameters or over an UART.
  * MUST be call at least once at boot stage to define the default trace port pointer.
  * Keep both init steps in this order not to print message during the boot.
  * @param	trcportid: the trace port to be used.
  * @param	buf: the trace buffer pointer.
  * @param	size: the buffer size.
  * @retval None
  */
void trace_init(enum port_e port, char *buf, uint32_t size);

/**
  * @brief  provide service to print string format trace messages over
  * an UART or a circular buffer (printf equivalent).
  * @param	variable argument including string and its format to print
  * Return	None
  */
void trace_printf(const char *format, ...);

/**
  * @brief  provide service to print string format trace messages over
  * an UART or a circular buffer. Insert automatically a header with the
  * system tick (in ms) and the log type.
  * @param	type: Log type 'I' for information, 'E' for error message and 'B' for boot
  * phase so no header printed
  * @param	variable argument including string and its format to print
  * Return	None
  */
void trace_print(const char type, const char *format, ...);


void  trace_stm_print(const char *buff, uint32_t size);

/**
  * @brief Flush the tx data over uart console, if enabled
  * Return None
  */
void trace_flush(void);

/**
  * @brief  This routine returns the default trace port to be used
  * @retval Default trace port pointer
  */
enum port_e trace_port(void);


#define TRACE_LEVEL_NONE	0
#define TRACE_LEVEL_ERROR	10
#define TRACE_LEVEL_NOTICE	20
#define TRACE_LEVEL_WARNING	30
#define TRACE_LEVEL_INFO	40
#define TRACE_LEVEL_VERBOSE	50

#ifndef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LEVEL_NOTICE
#endif

#if TRACE_LEVEL >= TRACE_LEVEL_ERROR
#define TRACE_ASSERT(x) ((x) ? (void)0 \
		: trace_print('A', "ASSERT %s:%d\n", __func__, __LINE__))
#define TRACE_ERR(format, ...)	trace_print('E', format, ## __VA_ARGS__)
#else
#define TRACE_ASSERT(x)         (void)0
#define TRACE_ERR(format, ...)  (void)0
#endif

#if TRACE_LEVEL >= TRACE_LEVEL_NOTICE
#define TRACE_NOTICE(format, ...)  trace_printf(format, ## __VA_ARGS__)
#else
#define TRACE_NOTICE(format, ...) (void)0
#endif

#if TRACE_LEVEL >= TRACE_LEVEL_WARNING
#define TRACE_WARN(format, ...)  trace_print('W', format, ## __VA_ARGS__)
#else
#define TRACE_WARN(format, ...) (void)0
#endif

#if TRACE_LEVEL >= TRACE_LEVEL_INFO
#define TRACE_INFO(format, ...)  trace_print('I', format, ## __VA_ARGS__)
#else
#define TRACE_INFO(format, ...) (void)0
#endif

#if TRACE_LEVEL >= TRACE_LEVEL_VERBOSE
#define TRACE_VERBOSE(format, ...)  trace_print('V', format, ## __VA_ARGS__)
#else
#define TRACE_VERBOSE(format, ...) (void)0
#endif

#if defined STM
#define TRACE_STM(buff, size) trace_stm_print(buff, size)
#else
#define TRACE_STM(buff, size)        (void)0
#endif


#endif /* __STA_TRACE_H__ */
