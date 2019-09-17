/**
 * @file sta_cli_cmd.c
 * @brief This file describes and handles utility functions to offer CLI
 * commands
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
#include "utils.h"
#include "sta_uart.h"
#include "sta_src.h"
#include "sta_src_a7.h"
#include "sta_map.h"
#include "sta_ddr.h"
#include "sta_common.h"
#include "sta_mtu.h"
#include "sta_pm.h"
#include "sta_can_test.h"
#include "sta_wdt.h"
#include "sta_pmu.h"
#include "sta_platform.h"

#include "FreeRTOS_CLI.h"

const char *strtok_delimiters = " ";

static BaseType_t reboot(char *w_buff, size_t w_buff_len, const char *cmd)
{
#ifdef NO_SCHEDULER
	 __pm_reboot("CLI", NULL);
#else
	char *param = strtok((char *)cmd, strtok_delimiters);

	param = strtok(NULL, strtok_delimiters);
	pm_reboot((bool)atoi(param));
#endif
	return pdFALSE;
}

#ifdef NO_SCHEDULER
static BaseType_t watchdog_init(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param = strtok((char *)cmd, strtok_delimiters);

	param = strtok(NULL, strtok_delimiters);
	trace_printf("param:%s\n", param);
	switch (atoi(param)) {
	case 1:
		{
		struct sta_wdt_pdata wdt_pdata;

		param = strtok(NULL, strtok_delimiters);
		if (!param) {
			trace_printf("missing parameter fro delay\n");
			return pdFALSE;
		}
		wdt_pdata.timeout = atoi(param);
		wdt_pdata.reload_interval = 1;
		wdt_pdata.clk_rate = m3_get_mxtal() / 8;
		if ((get_soc_id() == SOCID_STA1385) &&
		    (get_cut_rev() >= CUT_20))
			wdt_pdata.reg_base = (t_wdt *)WDT_ASYNC_APB_BASE;
		else
			wdt_pdata.reg_base = (t_wdt *)WDT_BASE;
		if (wdt_init(&wdt_pdata))
			trace_printf("wdt_init fails\n");
		if (wdt_enable())
			trace_printf("wdt_enable fails\n");
		}
		return pdFALSE;

	case 0:
		if (wdt_disable())
			trace_printf("wdt_disable fails\n");
		return pdFALSE;

	default:
		trace_printf("watchdog_init : Only 0 or 1 to be used\n");
		return pdFALSE;
	}
}
#endif

static BaseType_t poweroff(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param = strtok((char *)cmd, strtok_delimiters);

	param = strtok(NULL, strtok_delimiters);
#ifdef NO_SCHEDULER
	pmu_goto_sby();
#endif
	pm_goto_sby((bool)atoi(param));
	return pdFALSE;
}

static BaseType_t md_long_cmd_cli(char *w_buff, size_t w_buff_len,
				  const const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	volatile uint32_t *address;

	(void)param0;

	if (!param1) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	address = (volatile uint32_t *)strtoul(param1, NULL, 16);

	trace_printf("[0x%08x] = 0x%08x\r\n", address, *address);

	return pdFALSE;
}

static BaseType_t
mw_long_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	volatile uint32_t *address;
	uint32_t value;

	(void)param0;

	if (!param1 || !param2) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	address = (volatile uint32_t *)strtoul(param1, NULL, 16);
	value = strtoul(param2, NULL, 16);

	/* Write value to address */
	*address = value;

	trace_printf("Written 0x%08x to [0x%08x]\r\n", value, address);

	return pdFALSE;
}

static BaseType_t get_temp(char *w_buff, size_t w_buff_len, const char *cmd)
{

	uint32_t temp;

	temp = m3_get_soc_temperature(100);
	trace_printf("Temperature: %u C\n", temp);

	return pdFALSE;
}

static BaseType_t soc_id(char *w_buff, size_t w_buff_len, const char *cmd)
{
	switch (get_soc_id()) {
	case SOCID_STA1275:
		trace_printf("SoC STA1275\n");
		break;
	case SOCID_STA1195:
		trace_printf("SoC STA1195\n");
		break;
	case SOCID_STA1295:
		trace_printf("SoC STA1295\n");
		break;
	case SOCID_STA1385:
		trace_printf("SoC STA1385\n");
		break;
	default:
		trace_printf("Unknown SoC id:%d\n", get_soc_id());
		break;
	}
	return pdFALSE;
}

static BaseType_t board_id(char *w_buff, size_t w_buff_len, const char *cmd)
{
	switch (get_board_id()) {
	case BOARD_A5_EVB:
		trace_printf("Board Accordo 5 Evaluation board (EVB)\n");
		break;
	case BOARD_A5_VAB:
		trace_printf("Board Accordo 5 Validation board (VAB)\n");
		break;
	case BOARD_TC3_EVB:
		trace_printf("Board TelemaCo 3 Evaluation board (EVB)\n");
		break;
	case BOARD_CR2_VAB:
		trace_printf("Board Craton 2 Validation board (VAB)\n");
		break;
	case BOARD_TC3P_CARRIER:
		trace_printf("Board TelemaCo 3P telmo + carrier board\n");
		break;
	case BOARD_TC3P_MTP:
		trace_printf("Board TelemaCo 3P telmo + MTP board\n");
		break;
	default:
		trace_printf("Unknown board id:%d\n", get_board_id());
		break;
	}
	return pdFALSE;
}


static BaseType_t stats(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *stats_buf = pvPortMalloc(1024);

	if (!stats_buf) {
		TRACE_ERR("%s: failed to allocate memory\n", __func__);
		return pdFALSE;
	}
	vTaskGetRunTimeStats(stats_buf);
	trace_printf("\n");
	trace_printf("Name            Absolute time   Percentage time\n");
	trace_printf("===============================================\n");
	trace_printf(stats_buf);
	trace_printf("\n");
	vTaskList(stats_buf);
	trace_printf("\n");
	trace_printf("Name            State   Prio    Stack   Num\n");
	trace_printf("===========================================\n");
	trace_printf(stats_buf);
	trace_printf("\n");
	vPortFree(stats_buf);

	return pdFALSE;
}

static BaseType_t can_loopback(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param = strtok((char *)cmd, strtok_delimiters);
	param = strtok(NULL, strtok_delimiters);

	can_set_loopback((bool)atoi(param));
	return pdFALSE;
}

static BaseType_t can_send(char *w_buff, size_t w_buff_len, const char *cmd)
{
	struct can_frame cf;
	char *param;
	int i;

	param = strtok((char *)cmd, strtok_delimiters);
	param = strtok(NULL, strtok_delimiters);
	cf.id = atoi(param);
	param = strtok(NULL, strtok_delimiters);
	cf.length = atoi(param);

	for (i = 0; i < (int)cf.length; i++) {
		param = strtok(NULL, strtok_delimiters);
		cf.data[i] = (uint8_t)(atoi(param));
	}
	can_send_frame(&cf);
	return pdFALSE;
}

#ifndef NO_SCHEDULER
static BaseType_t watchdog_auto(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param = strtok((char *)cmd, strtok_delimiters);

	param = strtok(NULL, strtok_delimiters);
	wdt_reload_auto((bool)atoi(param));
	return pdFALSE;
}
static BaseType_t watchdog_reload(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param = strtok((char *)cmd, strtok_delimiters);

	param = strtok(NULL, strtok_delimiters);
	wdt_reload((uint32_t)atoi(param));
	return pdFALSE;
}
#endif

#ifdef CLI_CMDS_MEM_EXTRA
static void mem_write_8bit(volatile uint8_t * address, uint8_t value)
{
	*address = value;
}

static void mem_write_16bit(volatile uint16_t * address, uint16_t value)
{
	*address = value;
}

static void mem_write_32bit(volatile uint32_t * address, uint32_t value)
{
	*address = value;
}

static void mem_read_8bit(volatile uint8_t * address, uint8_t * value)
{
	*value = *address;
}

static void mem_read_16bit(volatile uint16_t * address, uint16_t * value)
{
	*value = *address;
}

static void mem_read_32bit(volatile uint32_t * address, uint32_t * value)
{
	*value = *address;
}

static BaseType_t rmw_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	volatile uint32_t *address;
	uint32_t write_data;
	uint32_t write_mask;
	uint32_t value;

	(void)param0;

	if (!param1 || !param2 || !param3) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	address = (volatile uint32_t *)strtoul(param1, NULL, 16);
	write_data = strtoul(param2, NULL, 16);
	write_mask = strtoul(param3, NULL, 16);

	/* Read-modify-write */
	mem_read_32bit(address, &value);
	value = (value & (~write_mask)) | (write_data & write_mask);
	mem_write_32bit(address, value);

	trace_printf("PASS\r\n");

	return pdFALSE;
}

static BaseType_t
memdump_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	volatile uint32_t *address;
	uint32_t *end_address;
	uint32_t size;
	uint32_t num_words_printed;
	uint32_t value_read;

	(void)param0;

	if (!param1 || !param2) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	address = (volatile uint32_t *)strtoul(param1, NULL, 16);
	size = strtoul(param2, NULL, 16);

	end_address = (uint32_t *) ((uint8_t *) address + size);

	num_words_printed = 0;
	while (address < end_address) {
		/* Read from memory */
		mem_read_32bit(address, &value_read);

		if ((num_words_printed & 0x3) == 0) {
			trace_printf("\r\n0x%08x: ", address);
		}
		trace_printf(" 0x%08x", value_read);

		address++;
		num_words_printed++;
	}

	trace_printf("\r\n");

	return pdFALSE;
}

static BaseType_t
memfill_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	volatile uint32_t *address;
	uint32_t *end_address;
	uint32_t size;
	uint32_t value_to_write;

	(void)param0;

	if (!param1 || !param2 || !param3) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	address = (volatile uint32_t *)strtoul(param1, NULL, 16);
	size = strtoul(param2, NULL, 16);
	value_to_write = strtoul(param3, NULL, 16);

	end_address = (uint32_t *) ((uint8_t *) address + size);

	while (address < end_address) {
		/* Read from memory */
		mem_write_32bit(address, value_to_write);
		address++;
	}

	return pdFALSE;
}

static BaseType_t
poll_register_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	volatile uint32_t *register_address;
	uint32_t bitmask_to_wait_for;
	uint32_t match_value;
	uint32_t value_read;
	TickType_t ticks;
	TickType_t ticks_start;
	const TickType_t ticks_timeout = 1000;

	(void)param0;

	if (!param1 || !param2 || !param3) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	register_address = (volatile uint32_t *)strtoul(param1, NULL, 16);
	match_value = strtoul(param2, NULL, 16);
	bitmask_to_wait_for = strtoul(param3, NULL, 16);

	/* Wait for Rising OR Falling event? */
	ticks_start = xTaskGetTickCount();

	/* Perform the polling... */
	do {
		mem_read_32bit(register_address, &value_read);
		ticks = xTaskGetTickCount();
	} while (((value_read & bitmask_to_wait_for) !=
		  (match_value & bitmask_to_wait_for))
		 && ((ticks - ticks_start) < ticks_timeout));;

	/* Check if PASS or FAIL */
	if ((ticks - ticks_start) >= ticks_timeout) {
		trace_printf("FAIL: timeout!\r\n");
	} else {
		trace_printf("PASS\r\n");
	}

	return pdFALSE;
}

static BaseType_t
memcmp_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	char *param4 = strtok(NULL, strtok_delimiters);
	char *param5 = strtok(NULL, strtok_delimiters);
	volatile uint8_t *address_base;
	volatile uint8_t *address_8bit;
	volatile uint16_t *address_16bit;
	volatile uint32_t *address_32bit;
	uint32_t length;
	uint32_t width;
	uint32_t value;
	uint32_t num_words;
	uint32_t mask;
	uint32_t i;

	(void)param0;

	if (!param1 || !param2 || !param3) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	address_base = (volatile uint8_t *)strtoul(param1, NULL, 16);
	length = strtoul(param2, NULL, 16);
	value = strtoul(param3, NULL, 16);
	width = (uint32_t) strtoul(param4, NULL, 10);
	mask = (uint32_t) strtoul(param5, NULL, 16);

	switch (width) {
	case 8:
		/* Compare value to address range (8-bit) */
		address_8bit = (volatile uint8_t *)address_base;
		num_words = length / sizeof(*address_8bit);
		for (i = 0; i < num_words; i++) {
			if ((*address_8bit & (uint8_t) mask) != (uint8_t) value) {
				trace_printf("FAIL: [%x] == 0x%02x\r\n",
					     address_8bit, *address_8bit);
				return pdFALSE;
			}
			address_8bit++;
		}
		break;

	case 16:
		/* Compare value to address range (16-bit) */
		address_16bit = (volatile uint16_t *)address_base;
		num_words = length / sizeof(*address_16bit);
		for (i = 0; i < num_words; i++) {
			if ((*address_16bit & (uint16_t) mask) !=
			    (uint16_t) value) {
				trace_printf("FAIL: [%x] == 0x%04x\r\n",
					     address_16bit, *address_16bit);
				return pdFALSE;
			}
			address_16bit++;
		}
		break;

	case 32:
		/* Compare value to address range (16-bit) */
		address_32bit = (volatile uint32_t *)address_base;
		num_words = length / sizeof(*address_32bit);
		for (i = 0; i < num_words; i++) {
			if ((*address_32bit & (uint32_t) mask) !=
			    (uint32_t) value) {
				trace_printf("FAIL: [%x] == 0x%08x\r\n",
					     address_32bit, *address_32bit);
				return pdFALSE;
			}
			address_32bit++;
		}
		break;

	default:
		trace_printf("FAIL: Bad parameter width=%u\r\n", width);
		return pdFALSE;
	}

	trace_printf("PASS: all bytes in the range: [%x..%x] are equal to "
		     "value=0x%02x\r\n", address_base, (address_base + length),
		     (uint8_t) value);

	return pdFALSE;
}

static BaseType_t
mem_test_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	const char usage[] =
	    "Usage: memtest <start_addr> <value> <size> <width> <iterations>"
	    " (hexadecimal), [width:8/16/32]\r\n";
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	char *param4 = strtok(NULL, strtok_delimiters);
	char *param5 = strtok(NULL, strtok_delimiters);
	char *param6 = strtok(NULL, strtok_delimiters);
	unsigned long phys_address;
	unsigned long phys_addr;
	unsigned long end_address;
	unsigned long size;
	unsigned long access_width;
	uint32_t value;
	uint32_t starting_value;
	uint32_t value_read;
	uint32_t num_iterations = 1;
	uint32_t i;
	uint32_t incremental_value;
	uint32_t increase_size;

	(void)param0;

	/* Check arguments */
	if (!param1 || !param2 || !param3 || !param4 || !param5 || !param6) {
		trace_printf("FAIL: %s\n", usage);
		return pdFALSE;
	}

	/* Get physical address to read from */
	phys_address = strtoul(param1, NULL, 16);
	starting_value = strtoul(param2, NULL, 16);
	size = strtoul(param3, NULL, 16);
	access_width = strtoul(param4, NULL, 10);
	num_iterations = strtoul(param5, NULL, 16);
	incremental_value = strtoul(param6, NULL, 10);

	trace_printf("Starting memtest phy_address=0x%08x, starting_value=%x"
		     " size=0x%08x, access_width=%u, num_iterations=%x"
		     " incremental_value=%u\r\n",
		     phys_address, starting_value, size, access_width,
		     num_iterations, incremental_value);

	end_address = (phys_address + size);
	value = starting_value;
	increase_size = access_width / 8;
	for (phys_addr = phys_address; phys_addr < end_address;
	     phys_addr += increase_size) {
		for (i = 0; i < num_iterations; i++) {
			/* Write to memory */
			switch (access_width) {
			case 8:
				mem_write_8bit((volatile uint8_t *)phys_addr,
					       (uint8_t) value);
				break;
			case 16:
				mem_write_16bit((volatile uint16_t *)phys_addr,
						(uint16_t) value);
				break;
			case 32:
				mem_write_32bit((volatile uint32_t *)phys_addr,
						value);
				break;
			default:
				trace_printf("FAIL: Invalid access width=%u",
					     access_width);
				return pdFALSE;
			}

			/* Read from memory */
			switch (access_width) {
			case 8:
				mem_read_8bit((volatile uint8_t *)phys_addr,
					      (uint8_t *) & value_read);
				break;
			case 16:
				mem_read_16bit((volatile uint16_t *)phys_addr,
					       (uint16_t *) & value_read);
				break;
			case 32:
				mem_read_32bit((volatile uint32_t *)phys_addr,
					       &value_read);
				break;
			default:
				trace_printf("FAIL: Invalid access width=%u",
					     access_width);
				return pdFALSE;
			}

			if (value != value_read) {
				trace_printf
				    ("FAIL: Error read after write from address 0x%08lx yields"
				     " wrong value.\r\n value=0x%08x, value_read=0x%08x => "
				     "exiting test\r\n", phys_addr, value,
				     value_read);
				return pdFALSE;
			}
		}

		if (incremental_value) {
			value++;
		}
	}

	/* Print the value */
	trace_printf("PASS: memtest successful.\r\n");

	return pdFALSE;
}

static BaseType_t
mem_checker_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	const char usage[] =
	    "Usage: memchecker <start_address> <end_address> <value>\r\n";
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	unsigned long phys_address;
	unsigned long phys_addr;
	unsigned long end_address;
	uint32_t expected_value;
	uint32_t starting_value;
	uint32_t value_read;

	(void)param0;

	/* Check arguments */
	if (!param1 || !param2 || !param3) {
		trace_printf("FAIL: %s\n", usage);
		return pdFALSE;
	}

	/* Get physical address to read from */
	phys_address = strtoul(param1, NULL, 16);
	end_address = strtoul(param2, NULL, 16);
	starting_value = strtoul(param3, NULL, 16);

	trace_printf("Starting memtest phy_address=0x%08x, starting_value=%x"
		     " size=0x%08x\r\n",
		     phys_address, starting_value,
		     (end_address - phys_address));

	expected_value = starting_value;
	for (phys_addr = phys_address; phys_addr < end_address;
	     phys_addr += sizeof(uint32_t)) {
		/* Read from memory */
		mem_read_32bit((volatile uint32_t *)phys_addr, &value_read);

		if (expected_value != value_read) {
			trace_printf
			    ("FAIL: Error read after write from address 0x%08lx yields"
			     " wrong value.\r\n expected_value=0x%08x, value_read=0x%08x => "
			     "exiting test\r\n", phys_addr, expected_value,
			     value_read);
			return pdFALSE;
		}

		expected_value++;
	}

	/* Print the value */
	trace_printf("PASS: memchecker successful.\r\n");

	return pdFALSE;
}
#endif

#ifdef CLI_CMDS_CLK
static BaseType_t
set_pll_config(char *w_buff, size_t w_buff_len, const char *cmd)
{
	const char usage[] =
	    "Usage: pll.cfg <id> <ndiv> <idf> <odf>\r\n";
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	char *param2 = strtok(NULL, strtok_delimiters);
	char *param3 = strtok(NULL, strtok_delimiters);
	char *param4 = strtok(NULL, strtok_delimiters);
	char id;
	uint32_t ndiv, idf, odf;
	uint32_t the_pll = 0;

	(void)param0;

	/* Check arguments */
	if (!param1 || !param2 || !param3 || !param4) {
		trace_printf("FAIL: %s\n", usage);
		return pdFALSE;
	}

	id = param1[0];
	ndiv = strtoul(param2, NULL, 10);
	idf = strtoul(param3, NULL, 10);
	odf = strtoul(param4, NULL, 10);

	switch (id) {
	case '2':
		the_pll = PLL2;
		break;
	case '3':
		the_pll = PLL3;
		break;
	case '4':
		the_pll = PLL4;
		break;
	case 'd':
		the_pll = PLLD;
		break;
	default:
		trace_printf("Error: invalid id\n");
		return pdFALSE;
	}

	srcm3_pclk_disable_all(src_m3_regs);
	srcm3_set_mode(src_m3_regs, SRCM3_MODECR_EXTOSC);

	srcm3_pll_disable(src_m3_regs, the_pll);
	srcm3_pll_update_freq(src_m3_regs, the_pll, idf, ndiv, -1, odf);
	srcm3_pll_enable(src_m3_regs, the_pll);

	while (!srcm3_is_pll_enabled(src_m3_regs, the_pll));

	srcm3_set_mode(src_m3_regs, SRCM3_MODECR_NORMAL);
	srcm3_pclk_enable_all(src_m3_regs);

	return pdFALSE;
}

static BaseType_t
get_pll_config(char *w_buff, size_t w_buff_len, const char *cmd)
{

	trace_printf
	    ("PLLARM:\n=======\n\t- NDIV = %d [0x%x]\n\t- IDF  = %d [0x%x]\n\t- ODF  = %d [0x%x]\n\t- CP   = %d [0x%x]\n",
	     a7_ssc_regs->pllarm_freq.bit.ndiv,
	     a7_ssc_regs->pllarm_freq.bit.ndiv,
	     a7_ssc_regs->pllarm_freq.bit.idf, a7_ssc_regs->pllarm_freq.bit.idf,
	     a7_ssc_regs->pllarm_freq.bit.odf, a7_ssc_regs->pllarm_freq.bit.odf,
	     a7_ssc_regs->pllarm_freq.bit.cp, a7_ssc_regs->pllarm_freq.bit.cp);
	trace_printf
	    ("PLL2:\n=====\n\t- NDIV = %d [0x%x]\n\t- IDF  = %d [0x%x]\n\t- ODF  = %d [0x%x]\n\t- CP   = %d [0x%x]\n",
	     src_m3_regs->pll2fctrl.bit.ndiv, src_m3_regs->pll2fctrl.bit.ndiv,
	     src_m3_regs->pll2fctrl.bit.idf, src_m3_regs->pll2fctrl.bit.idf,
	     src_m3_regs->pll2fctrl.bit.odf, src_m3_regs->pll2fctrl.bit.odf,
	     src_m3_regs->pll2fctrl.bit.cp, src_m3_regs->pll2fctrl.bit.cp);
	trace_printf
	    ("PLL3:\n=====\n\t- NDIV = %d [0x%x]\n\t- IDF  = %d [0x%x]\n\t- ODF  = %d [0x%x]\n\t- CP   = %d [0x%x]\n",
	     src_m3_regs->pll3fctrl.bit.ndiv, src_m3_regs->pll3fctrl.bit.ndiv,
	     src_m3_regs->pll3fctrl.bit.idf, src_m3_regs->pll3fctrl.bit.idf,
	     src_a7_regs->scpll3fctrl.bit.odf, src_a7_regs->scpll3fctrl.bit.odf,
	     src_m3_regs->pll3fctrl.bit.cp, src_m3_regs->pll3fctrl.bit.cp);
	trace_printf
	    ("PLL4:\n=====\n\t- NDIV = %d [0x%x]\n\t- IDF  = %d [0x%x]\n\t- ODF  = %d [0x%x]\n\t- CP   = %d [0x%x]\n",
	     src_m3_regs->pll4fctrl.bit.ndiv, src_m3_regs->pll4fctrl.bit.ndiv,
	     src_m3_regs->pll4fctrl.bit.idf, src_m3_regs->pll4fctrl.bit.idf,
	     src_m3_regs->pll4fctrl.bit.odf, src_m3_regs->pll4fctrl.bit.odf,
	     src_m3_regs->pll4fctrl.bit.cp, src_m3_regs->pll4fctrl.bit.cp);
	trace_printf
	    ("PLLD:\n=====\n\t- NDIV = %d [0x%x]\n\t- ODF  = %d [0x%x]\n",
	     src_m3_regs->plldfctrl.bit.ndiv, src_m3_regs->plldfctrl.bit.ndiv,
	     src_a7_regs->scplldfctrl.bit.pll_odf,
	     src_a7_regs->scplldfctrl.bit.pll_odf);
	trace_printf
	    ("PLLU:\n=====\n\t- NDIV_26 = %d [0x%x]\n\t- NDIV_24 = %d [0x%x]\n\t- FRAC    = %d [0x%x]\n",
	     usb_wrapper_regs->usb_pll_ctrl1.bit.ndiv_26,
	     usb_wrapper_regs->usb_pll_ctrl1.bit.ndiv_26,
	     usb_wrapper_regs->usb_pll_ctrl1.bit.ndiv_24,
	     usb_wrapper_regs->usb_pll_ctrl1.bit.ndiv_24,
	     usb_wrapper_regs->frac_input, usb_wrapper_regs->frac_input);
	return pdFALSE;
}
#endif

#ifdef CLI_CMDS_MISC
static BaseType_t wait_cmd_cli(char *w_buff, size_t w_buff_len, const char *cmd)
{
	char *param0 = strtok((char *)cmd, strtok_delimiters);
	char *param1 = strtok(NULL, strtok_delimiters);
	uint32_t ms;

	(void)param0;

	if (!param1) {
		trace_printf("FAIL: not enough params\r\n");
		return pdFALSE;
	}

	ms = strtoul(param1, NULL, 10);

	mdelay(ms);

	return pdFALSE;
}
#endif

static CLI_Command_Definition_t sta_cli_cmds[] = {
	{"reboot", "reboot <0|1> : system restart bypassing or not AP OS\r\n", reboot, 1},
#ifdef NO_SCHEDULER
	{"watchdog_init", "watchdog_init <0|1> <delay_seconds>: disable|enable watchdog M3\r\n", watchdog_init, 2},
#endif
	{"poweroff", "poweroff <0|1> : system shutdown bypassing or not AP OS\r\n", poweroff, 1},
	{"temp", "temp : get SoC temperature\r\n", get_temp, 0},
	{"soc", "soc : get SoC identifier\r\n", soc_id, 0},
	{"board", "board : get board identifier\r\n", board_id, 0},
	{"md", "md <address> : memory dump (long 32bit)\r\n",
	 md_long_cmd_cli, 1},
	{"mw", "mw <address> <value> : memory write (long 32bit)\r\n",
	 mw_long_cmd_cli, 2},
	{"stats", "stats : get task statistics\r\n", stats, 0},
	{"can_loopback", "can_loopback <0|1>: disable|enable can loopback\r\n", can_loopback, 1},
	{"can_send", "can_send <id length dat0 dat1 ...>: send can frame\r\n", can_send, 10},
#ifndef NO_SCHEDULER
	{"watchdog_auto", "watchdog_auto <0|1>: disable|enable watchdog auto reload\r\n", watchdog_auto, 1},
	{"watchdog_reload", "watchdog_reload <timeout>: reload watchdog with timeout\r\n", watchdog_reload, 1},
#endif
#ifdef CLI_CMDS_MEM_EXTRA
	{"md.l", "md.l <address> : memory dump (long 32bit)\r\n",
	 md_long_cmd_cli, 1},
	{"mw.l", "mw.l <address> <value> : memory write (long 32bit)\r\n",
	 mw_long_cmd_cli, 2},
	{"rmw", "rmw <addr> <data> <mask> : read-modify-write (32bit)\r\n",
	 rmw_cmd_cli, 3},
	{"memdump", "memdump <address> <size> : Dump memory range\r\n",
	 memdump_cmd_cli, 2},
	{"memfill",
	 "memfill <address> <size> <value> : Memory fill with value\r\n",
	 memfill_cmd_cli, 3},
	{"poll_reg",
	 "poll_reg <address> <value> <bitmask> : polling mechanism\r\n",
	 poll_register_cmd_cli, 3},
	{"memcmp",
	 "memcmp <address> <length> <value> <width=8/16/32> <mask> : memory compare to a specific value\r\n",
	 memcmp_cmd_cli, 5},
	{"memtest",
	 "memtest <start_addr> <starting_value> <size> <width=8/16/32> <iterations> <incremental_value=0/1>\r\n",
	 mem_test_cmd_cli, 6},
	{"memchecker", "memchecker <start_address> <end_address> <value>\r\n",
	 mem_checker_cmd_cli, 3},
#endif
#ifdef CLI_CMDS_CLK
	{"pll", "pll : get PLL config\r\n", get_pll_config, 0},
	{"pll.cfg", "pll.cfg <id> <ndiv> <idf> <odf> : set PLL config\r\n",
	 set_pll_config, 4},
#endif
#ifdef CLI_CMDS_MISC
	{"wait",
	 "wait <OS_ticks>           : Wait for OS_ticks period (1 OS_tick == 1msec)\r\n",
	 wait_cmd_cli, 1},
#endif
};

/**
 * @brief	register CLI available commands
 * @return	0 if no error, not 0 otherwise
 */
int sta_cli_commands_register(void)
{
	uint32_t i;
	BaseType_t ret;

	for (i = 0; i < NELEMS(sta_cli_cmds); i++) {
		ret = FreeRTOS_CLIRegisterCommand(&sta_cli_cmds[i]);
		if (ret == pdFAIL) {
			TRACE_ERR("%s: Failed to register CLI command: %u",
				  __func__, i);
			return -EINVAL;
		}
	}

	return 0;
}
