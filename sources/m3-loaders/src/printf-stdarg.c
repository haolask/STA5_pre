/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)
	stdarg version contributed by Christian Ettinger

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "printf-stdarg.h"

#define putchar(c) c

#include <stdarg.h>
#include "sta_uart.h"
#include "sta_stm.h"
#include "trace.h"

/* Defines for Cortex-M Debug unit */
/* System Control Block DCRDR Register */
#define DCRDR			(*(volatile unsigned int*)0xE000EDF8)
#define	BUSY			1
#define TARGET_REQ_DEBUGCHAR	0x02

/* On Openocd (contrib/libdcc): we use the DCRDR reg to simulate DCC channel
 * DCRDR[7:0] is used by target for status
 * DCRDR[15:8] is used by target for write buffer
 * DCRDR[23:16] is used for by host for status
 * DCRDR[31:24] is used for by host for write buffer
 * Usage on Openocd from telnet prompt do:
 *     target_request debugmsgs enable
 *     trace point 1
 */

static t_uart * const uart_address[] = {
	uart0_regs, uart1_regs, uart2_regs, uart3_regs, uart4_regs, uart5_regs
};

static void dcc_write(unsigned long dcc_data)
{
	int len = 4;

	while (len--) {
		/* wait for data ready */
		while (DCRDR & BUSY)
			;

		/* write our data and set write flag - tell host there is data*/
		DCRDR = (unsigned short)(((dcc_data & 0xff) << 8) | BUSY);
		dcc_data >>= 8;
	}
}

/* STM print */
#define STM_PRINT_BUFFER 0x200
#define STM_PRINT_CHANNEL 0x10

#if defined STM
struct print_stm_t {
	unsigned long pos;
	unsigned char buffer[STM_PRINT_BUFFER];
} print_stm;

#define print_stm_init() print_stm.pos = 0
#define print_stm_flush() stm_send_data(stm_fifo_regs, STM_PRINT_CHANNEL, \
					print_stm.buffer, print_stm.pos)
static void print_stm_buff(char byte)
{
	if (print_stm.pos < STM_PRINT_BUFFER)
		print_stm.buffer[print_stm.pos++] = byte;
}
#else
#define print_stm_init()        (void)0
#define print_stm_flush()        (void)0
#define print_stm_buff(x) (void)0
#endif /* STM */

static void printchar(char **str, int c)
{
	if (_CR_CRLF && c == '\n')
		printchar(str, '\r');	/* CR -> CRLF */
	if (str) {
		**str = (char)c;
		(*str)++;
	} else {
		enum port_e port = trace_port();

		switch (port) {
		case NO_TRACE_PORT:
			break;

		case JTAG_DCC_PORT: /* JTAG Debug Control Channel for OpenOCD */
			dcc_write(TARGET_REQ_DEBUGCHAR | ((char)c << 16));
			break;

		case UART0:
		case UART1:
		case UART2:
		case UART3:
		case UART4:
		case UART5:
			uart_send_data(uart_address[port], (char *)&c, 1);
			print_stm_buff((char)c);
			break;
		default:
			break;
		}
	}
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

t_uart **uart_address_array_get(void)
{
	return (t_uart **)uart_address;
}

int prints(char **out, const char *string, int width, int pad)
{
	int pc = 0, padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;

		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (pad & PAD_ZERO)
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printchar(out, padchar);
			++pc;
		}
	}
	for (; *string ; ++string) {
		printchar(out, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printchar(out, padchar);
		++pc;
	}

	if (out)
		**out = '\0';

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width,
		  int pad, int letbase)
{
	char *s;
	int t, neg = 0, pc = 0;
	unsigned int u = (unsigned int)i;
	char print_buf[PRINT_BUF_LEN];

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = (unsigned int)-i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = (unsigned int)u % b;
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = (char)(t + '0');
		u /= b;
	}

	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar(out, '-');
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, s, width, pad);
}

int print(char **out, const char *format, va_list args)
{
	int width, pad;
	int pc = 0;
	char scr[2];

	print_stm_init();

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') {
				char *s = (char *)va_arg(args, int);

				pc += prints(out, s?s:"(null)", width, pad);
				continue;
			}
			if (*format == 'd') {
				pc += printi(out, va_arg(args, int), 10, 1,
					     width, pad, 'a');
				continue;
			}
			if (*format == 'x') {
				pc += printi(out, va_arg(args, int), 16, 0,
					     width, pad, 'a');
				continue;
			}
			if (*format == 'X') {
				pc += printi(out, va_arg(args, int), 16, 0,
					     width, pad, 'A');
				continue;
			}
			if (*format == 'u') {
				pc += printi(out, va_arg(args, int), 10, 0,
					     width, pad, 'a');
				continue;
			}
			if (*format == 'c') {
				/* char are converted to int */
				scr[0] = (char)va_arg(args, int);
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		} else {
out:
			printchar(out, *format);
			++pc;
		}
	}
	if (out)
		**out = '\0';
	print_stm_flush();
	va_end(args);

	return pc;
}

int printf(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	return print(0, format, args);
}

int sprintf(char *out, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	return print(&out, format, args);
}


int snprintf(char *buf, unsigned int count, const char *format, ...)
{
	va_list args;

	(void)count;

	va_start(args, format);
	return print(&buf, format, args);
}

void printstm(const char *buff, unsigned int size)
{
	stm_send_data(stm_fifo_regs, STM_PRINT_CHANNEL, buff, size);
}

#ifdef TEST_PRINTF
int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	char buf[80];

	mi = (1 << (bs-1)) + 1;
	printf("%s\n", ptr);
	printf("printf test\n");
	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)%", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

	return 0;
}

/*
 * if you compile this file with
 *   gcc -Wall $(YOUR_C_OPTIONS) -DTEST_PRINTF -c printf.c
 * you will get a normal warning:
 *   printf.c:214: warning: spurious trailing `%' in format
 * this line is testing an invalid % at the end of the format string.
 *
 * this should display (on 32bit int machine) :
 *
 * Hello world!
 * printf test
 * (null) is null pointer
 * 5 = 5
 * -2147483647 = - max int
 * char a = 'a'
 * hex ff = ff
 * hex 00 = 00
 * signed -3 = unsigned 4294967293 = hex fffffffd
 * 0 message(s)
 * 0 message(s) with %
 * justif: "left      "
 * justif: "     right"
 *  3: 0003 zero padded
 *  3: 3    left justif.
 *  3:    3 right justif.
 * -3: -003 zero padded
 * -3: -3   left justif.
 * -3:   -3 right justif.
 */

#endif

