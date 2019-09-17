/**
 * @file trace.c
 * @brief trace & debug functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "FreeRTOS.h"
#include "task.h"
#include "printf-stdarg.h"
#include "sta_uart.h"
#include "sta_mtu.h"
#include "trace.h"

#define panic() for(;;)   /* Loop forever */
#define TRACE_SPACE_MIN     150	/* Minimum space before wraparound */

struct trace_t trace __attribute__((section(".shared_m3_data")));

static char header[17];

/**
  * @brief  This routine returns the default trace port to be used
  * @retval Default trace port pointer
  */
enum port_e trace_port(void)
{
	return trace.port;
}

/**
  * @brief  This routine initializes the trace logging path. The messages can be
  * printed either over a circular buffer passed as parameters or over an UART.
  * MUST be call at least once at boot stage to define the default trace port pointer.
  * Keep both init steps in this order not to print message during the boot.
  * @param	trcportid: the trace port to be used.
  * @param	buf: the trace buffer pointer.
  * @param	size: the buffer size.
  * @return: None panic (loop forever) if problem
  */
void trace_init(enum port_e port, char *buf, uint32_t size)
{
	trace.port = port;

	/* Step 1: Initialise trace logging buffer if required */
	if (buf != NULL) {
		/* Don't reinitialize if it's the same buffer as current trace buffer */
		if (trace.start != buf) {
			/* Trace printed in circular buffer */
			trace.buf = buf;
			trace.start = buf;
			trace.end = trace.start + size;
			trace.buf_ovf = 0;
			trace.wrap =
				trace.end - (size >
						TRACE_SPACE_MIN ? TRACE_SPACE_MIN : size);
		}
	} else {
		trace.buf = NULL;
	}

	/* Step 2: Set trace port */
	if (port != NO_TRACE_PORT) {
		if (port >= UART0 && port < UART_MAX_NO) {
			/* Init debug trace UART if required */
			if (!uart_init(port, BR115200BAUD, NOPARITY_BIT,
						ONE_STOPBIT, DATABITS_8))
				panic();
		}
#if defined TRACE_EARLY_BUF
		/*  Drain early trace buffer if not empty */
		if (trace.trace_early_buf[0] != 0) {
			if (trace.buf_ovf)
				trace_printf(&trace.trace_early_buf[strlen(
					    trace.trace_early_buf) + 1]);
			trace_printf(trace.trace_early_buf);
			trace.trace_early_buf[0] = 0;
		}
#endif
	}
}

/**
  * @brief  provide service to print string format trace messages over
  * an UART or a circular buffer. Insert automatically a header with the
  * system tick (in ms) and the log type.
  * May be called in interrupt context.
  * @param	type: Log type 'I' for information and 'E' for error message
  * phase so no header printed
  * @param	variable argument including string and its format to print
  * Return	None
  */
void trace_print(const char type, const char *format, ...)
{
	va_list args;
	UBaseType_t saved_interrupt_status = 0;

	snprintf(header, sizeof(header), "[%5u.%06u]%c:",
		    (unsigned int)mtu_get_timebase(TIME_IN_SEC),
		    (unsigned int)mtu_get_timebase(TIME_IN_US), type);

	va_start(args, format);

	/* Enter critical region */
	if (portNVIC_INT_ACTIVE())  /* In IRQ context ? */
		saved_interrupt_status = taskENTER_CRITICAL_FROM_ISR();
	else
		taskENTER_CRITICAL();
	if (trace.buf != NULL) {
		if (trace.buf >= trace.wrap) {
			trace.buf = trace.start;
			trace.buf_ovf = 1;
		}
		prints(&trace.buf, header, 0, 0);
		print(&trace.buf, format, args);

		TRACE_ASSERT(trace.buf < trace.end);
	}

	if (trace.port != NO_TRACE_PORT) {
		prints(0, header, 0, 0);
		print(0, format, args);
	}
	if (portNVIC_INT_ACTIVE())  /* In IRQ context ? */
		taskEXIT_CRITICAL_FROM_ISR(saved_interrupt_status);
	else
		taskEXIT_CRITICAL();
}

/**
  * @brief  provide service to print string format trace messages over
  * an UART or a circular buffer (printf equivalent), may be called in
  * interrupt context
  * @param	variable argument including string and its format to print
  * Return	None
  */
void trace_printf(const char *format, ...)
{
	va_list args;
	UBaseType_t saved_interrupt_status = 0;

	va_start( args, format );

	/* Enter critical region */
	if (portNVIC_INT_CTRL_REG & 0xFF) /* In IRQ context ? */
		saved_interrupt_status = taskENTER_CRITICAL_FROM_ISR();
	else
		taskENTER_CRITICAL();
	if (trace.buf != NULL) {
		if (trace.buf >= trace.wrap) {
			trace.buf = trace.start;
			trace.buf_ovf = 1;
		}
		print(&trace.buf, format, args);

		TRACE_ASSERT(trace.buf < trace.end);
	}

	if (trace.port != NO_TRACE_PORT) {
		print(0, format, args);
	}
	if (portNVIC_INT_CTRL_REG & 0xFF) /* In IRQ context ? */
		taskEXIT_CRITICAL_FROM_ISR(saved_interrupt_status);
	else
		taskEXIT_CRITICAL();
}

void  trace_stm_print(const char *buff, uint32_t size)
{
	printstm(buff, size);
}

/**
  * @brief Flush the tx data over uart console, if enabled
  * Return None
  */
void trace_flush(void)
{
	enum port_e port = trace_port();
	t_uart **const uart_address = uart_address_array_get();

	if (port == NO_TRACE_PORT)
		return;
	uart_flush_data(uart_address[port]);
}
