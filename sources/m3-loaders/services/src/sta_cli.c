/**
 * @file sta_ddr_cli.c
 * @brief This file describes and handles utility functions to offer CLI to
 * interact with DDR configuration
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "sta_uart.h"
#include "utils.h"
#include "printf-stdarg.h"

#include "FreeRTOS_CLI.h"

#define MAX_INPUT_LENGTH    50
#define MAX_OUTPUT_LENGTH   100

#define COMMAND_CONSOLE_RETURN '\r'
#define COMMAND_CONSOLE_NEW_LINE '\n'

#define STA_CLI_PROMPT "$ "

extern int sta_cli_commands_register(void);
#ifdef CLI_CMDS_DDR
	/* Register all CLI commands */
extern int sta_ddr_cli_commands_register(void);
#endif

static const char *welcome_msg =
    "Type 'help' to view a list of registered commands" ".\r\n";

static int console_read(enum port_e port, const char *c, uint32_t size)
{
	t_uart **const uart_address = uart_address_array_get();
	int ret;

	ret = uart_get_data(uart_address[port], (char *)c, size);
	if (ret < 0) {
		trace_printf("uart_get_data() ERROR: ret=%d\n", ret);
		if (!uart_init(port, BR115200BAUD, NOPARITY_BIT,
			       ONE_STOPBIT, DATABITS_8)) {
			trace_printf("failed to re-init uart\n");
			return -EIO;
		}
	}
	return ret;
}

void console_write(enum port_e port, const char *c, uint32_t size)
{
	t_uart **const uart_address = uart_address_array_get();

	uart_send_data(uart_address[port], (char *)c, size);
}

void console_wait_for_data(enum port_e port) {
	t_uart **const uart_address = uart_address_array_get();

	uart_wait_for_data(uart_address[port]);
}

void console_prompt_print(enum port_e port)
{
	size_t prompt_strlen = strlen(STA_CLI_PROMPT);

	console_write(port, (const char *)STA_CLI_PROMPT, prompt_strlen);
}

void cli_task(void *p)
{
	char rx_char;
	unsigned int input_idx = 0;
	BaseType_t more_data_to_follow;
	static char output_string[MAX_OUTPUT_LENGTH];
	static char input_string[MAX_INPUT_LENGTH];
	enum port_e port = trace_port();

	/* Register all CLI commands */
	sta_cli_commands_register();
#ifdef CLI_CMDS_DDR
	sta_ddr_cli_commands_register();
#endif
	/* Send a welcome message to the user knows they are connected. */
	console_write(port, welcome_msg, strlen((char *)welcome_msg));

	/* Print prompt */
	console_prompt_print(port);

	for (;;) {
		/* Wait until one character is received */
		console_wait_for_data(port);

		if (console_read(port, &rx_char, sizeof(rx_char)))
			continue;

		if (rx_char == COMMAND_CONSOLE_RETURN) {
			/*
			 * Carriage return character was received so input
			 * command is complete. Transmit a line separator
			 * first before processing command.
			 */
			console_write(port, "\r\n", strlen("\r\n"));

			/* If no input data, no need to process command */
			if (!input_idx) {
				console_prompt_print(port);
				continue;
			}

			/*
			 * The command interpreter is called repeatedly until it
			 * returns pdFALSE. See the "Implementing a command"
			 * documentation for an exaplanation of why this is. */
			do {
				memset(output_string, 0x00, MAX_OUTPUT_LENGTH);
				more_data_to_follow = FreeRTOS_CLIProcessCommand(
					input_string, output_string,
					MAX_OUTPUT_LENGTH);

				/* Write command output to console. */
				console_write(port, output_string,
				       strlen((char *)output_string));

			} while (more_data_to_follow != pdFALSE);

			input_idx = 0;
			memset(input_string, 0x00, MAX_INPUT_LENGTH);
			console_prompt_print(port);
			continue;
		}

		if (rx_char == '\b') {
			char tmp = ' ';
			/* Backspace was pressed. Erase the last
			 * character in the input buffer if there
			 * are any. */
			if (input_idx > 0) {
				input_idx--;
				input_string[input_idx] = '\0';
				console_write(port, &rx_char, sizeof(rx_char));
				console_write(port, &tmp, sizeof(tmp));
				console_write(port, &rx_char, sizeof(rx_char));
			}
			continue;
		}

		/* Echo received char */
		console_write(port, &rx_char, sizeof(rx_char));

		/* Add character to input command string */
		if (input_idx < MAX_INPUT_LENGTH) {
			input_string[input_idx] = rx_char;
			input_idx++;
		}
	}
}
