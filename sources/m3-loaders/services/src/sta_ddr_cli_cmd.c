/**
 * @file sta_ddr_cli_cmd.c
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
#include "sta_map.h"
#include "sta_common.h"
#include "sta_ddr.h"
#include "utils.h"

#include "FreeRTOS_CLI.h"

#ifdef CLI_CMDS_DDR

static uint32_t sequence;

#define IS_STEP_DONE(m, x) (m & BIT(x))
#define STEP_DONE(m, x) (m |= BIT(x))

/**
 * @brief	start a DDR test loop to read/write the DDR
 *			usage: ddr_test
 * @return	pdFalse if error, pdTrue otherwise
 */
static BaseType_t ddr_test(char *buf, size_t len, const char *cmd)
{
#define FIXED_PATTERN			0
#define INCREMENTAL_PATTERN		1
#define RANDOM_PATTERN			2

	char *c = strtok((char *)cmd, " ");
	char *i = strtok(NULL, " ");
	char *t = strtok(NULL, " ");
	char *m = strtok(NULL, " ");
	char *p = strtok(NULL, " ");
	char *s = strtok(NULL, " ");
	uint32_t mode, pattern, split;
	int iter, temp;
	bool r = false, w = false;

	(void)c;

	if (!IS_STEP_DONE(sequence, 1))
		trace_printf
		    ("warning ! DDR init is not complete, test may not work\n");

	iter = (int)strtol(i, NULL, 10);
	temp = (int)strtol(t, NULL, 10);
	mode = strtoul(m, NULL, 10);
	pattern = strtoul(p, NULL, 16);
	split = strtoul(s, NULL, 10);

	if (split % 4) {
		trace_printf("error: split=%d (expected multiple of 4)\n",
			     split);
		return pdFALSE;
	}

	if (mode != FIXED_PATTERN && mode != INCREMENTAL_PATTERN
	    && mode != RANDOM_PATTERN) {
		trace_printf("error: invalid mode=%d\n (expected %d, %d, %d)",
			     mode, FIXED_PATTERN, INCREMENTAL_PATTERN,
			     RANDOM_PATTERN);
		return pdFALSE;
	}

	if (mode == RANDOM_PATTERN && pattern == 0) {
		trace_printf("error: using mode %d, p must be != 0\n",
			     RANDOM_PATTERN);
		return pdFALSE;
	}

	switch (cmd[strlen(cmd) - 1]) {
	case 'r':
		r = true;
		break;
	case 'w':
		w = true;
		break;
	default:
		r = true;
		w = true;
		break;
	}

	sta_ddr_ctl_test_pattern(iter, !!temp, pattern, split, r, w, mode);

	return pdFALSE;
}

/**
 * @brief	display DDR data rate
 * @return	pdFalse if error, pdTrue otherwise
 */
static BaseType_t ddr_rate(char *buf, size_t len, const char *cmd)
{
	sta_ddr_display_rate();

	return pdFALSE;
}

/**
 * @brief	full DDR init
 * @return	pdFalse if error, pdTrue otherwise
 */
static BaseType_t ddr_init(char *buf, size_t len, const char *cmd)
{
	int err;
	char *param = strtok((char *)cmd, " ");

	param = strtok(NULL, " ");

	trace_printf("%s:\n", cmd);
#if !defined(CFG_DDR_ALT1) && !defined(CFG_DDR_ALT2) && !defined(CFG_DDR_ALT3)
	trace_printf("ALT_DDR_CFG: CFG_DDR_ALT0\n");
#else
#ifdef CFG_DDR_ALT1
	trace_printf("ALT_DDR_CFG: CFG_DDR_ALT1\n");
#endif
#ifdef CFG_DDR_ALT2
	trace_printf("ALT_DDR_CFG: CFG_DDR_ALT2\n");
#endif
#ifdef CFG_DDR_ALT3
	trace_printf("ALT_DDR_CFG: CFG_DDR_ALT3\n");
#endif
#endif
	if (!atoi(param)) {
		trace_printf("Cold Boot Mode\n");
		err = sta_ddr_init(DDR_BOOT_REASON_COLD_BOOT);
	} else {
		trace_printf("STR Boot Mode\n");
		err = sta_ddr_init(DDR_BOOT_REASON_EXIT_SELF_REF);
	}

	if (err) {
		trace_printf("failed\n");
	} else {
		trace_printf("success\n");
		STEP_DONE(sequence, 1);
	}

	return pdFALSE;
}

/**
 * @brief	do DDR self-refresh management
 * @return	pdFalse if error, pdTrue otherwise
 */
static BaseType_t ddr_selfrefresh(char *buf, size_t len, const char *cmd)
{
	int ret;
	char *param0 = strtok((char *)cmd, " ");
	char *param1 = strtok(NULL, " ");

	(void) param0;

	if (!IS_STEP_DONE(sequence, 1))
		trace_printf("error: cannot set self-refresh while DDR is not initialized\n");

	if (!strncmp(param1, "on", strlen(param1)))
		ret = sta_ddr_suspend();
	else
		ret = sta_ddr_resume();

	trace_printf("%s\n", ret ? "failed" : "success");

	return pdFALSE;
}

/**
 * @brief	do a dump of register of DDR Ctontroller and DDR phy
 * @return	only register with a setting property are dumped
 */
static BaseType_t ddr_dump(char *buf, size_t len, const char *cmd)
{
	TRACE_NOTICE("%s: ", cmd);
	sta_ddr_dump();
	return pdFALSE;
}

/**
 * @brief	do a dump of register of DDR Ctontroller and DDR phy
 * @return	only register with a setting property are dumped
 */
static BaseType_t ddr_set(char *buf, size_t len, const char *cmd)
{
	__maybe_unused char *label;
	char *v;
	__maybe_unused uint32_t value;

	label = strtok((char *)cmd, " ");
	label = strtok(NULL, " ");
	v = strtok(NULL, " ");

	value = (int)strtoul(v, NULL, 16);

	TRACE_NOTICE("%s: %s %d", cmd, label, value);
	sta_ddr_set_setting(label, value);
	return pdFALSE;
}

/**
 * @brief	do a dump of register of DDR Ctontroller and DDR phy
 * @return	only register with a setting property are dumped
 */
static BaseType_t ddr_set_mask(char *buf, size_t len, const char *cmd)
{
	__maybe_unused char *label;
	char *v;
	char *m;
	uint32_t value;
	__maybe_unused uint32_t old;
	uint32_t mask;

	label = strtok((char *)cmd, " ");
	label = strtok(NULL, " ");
	m = strtok(NULL, " ");
	v = strtok(NULL, " ");

	mask = (int)strtoul(m, NULL, 16);
	value = (int)strtoul(v, NULL, 16);

	old = 0;
	sta_ddr_get_setting(label, &old);
	old  |= (mask & value);
	old &= (~mask | value);

	TRACE_NOTICE("%s: %s", cmd, label);
	sta_ddr_set_setting(label, old);
	return pdFALSE;
}

/**
 * @brief	do a dump of register of DDR Ctontroller and DDR phy
 * @return	only register with a setting property are dumped
 */
static BaseType_t ddr_get(char *buf, size_t len, const char *cmd)
{
	__maybe_unused char *label = strtok((char *)cmd, " ");

	label = strtok(NULL, " ");

	TRACE_NOTICE("%s: %s", cmd, label);
	sta_ddr_get_setting(label, NULL);
	return pdFALSE;
}

static CLI_Command_Definition_t sta_ddr_cli_cmds[] = {
	{"ddr.init", "ddr.init <0|1> : 0:cold boot 1:str boot\r\n",
	 ddr_init, 1},
	{"ddr.test.a",
	 "ddr.test.a <i> <t> <m> <p> <s>: do ddr read/write test\r\n", ddr_test,
	 5},
	{"ddr.test.r",
	 "ddr.test.r <i> <t> <m> <p> <s>: do ddr read test only\r\n", ddr_test,
	 5},
	{"ddr.test.w",
	 "ddr.test.w <i> <t> <m> <p> <s>: do ddr write test only\r\n", ddr_test,
	 5},
	{"ddr.rate", "ddr.rate                  : display ddr data rate\r\n",
	 ddr_rate, 0},
	{"ddr.selfrefresh", "ddr.selfrefresh <on|off>  : enter/exit ddr self-refresh mode\r\n",
	 ddr_selfrefresh, 1},
	{"ddr.dump", "ddr.dump  : dump all registers belonging to setting area of DDR ctrl & DDR phy\r\n",
	 ddr_dump, 0},
	{"ddr.set", "ddr.set <label>	<value> : modify a registers of DDR ctrl & DDR phy belonging to setting area \r\n",
	 ddr_set, 2},
	{"ddr.set.m", "ddr.set.m <label> <mask> <value> : modify a register in using a mask of DDR ctrl & DDR phy belonging to setting area \r\n",
	 ddr_set_mask, 3},
	{"ddr.get", "ddr.get <label> : read a register of DDR ctrl & DDR phy belonging to setting area\r\n",
	 ddr_get, 1},
};

/**
 * @brief	register CLI available commands
 * @return	0 if no error, not 0 otherwise
 */
int sta_ddr_cli_commands_register(void)
{
	uint32_t i;
	BaseType_t ret;

	for (i = 0; i < NELEMS(sta_ddr_cli_cmds); i++) {
		ret = FreeRTOS_CLIRegisterCommand(&sta_ddr_cli_cmds[i]);
		if (ret == pdFAIL) {
			TRACE_ERR("%s: Failed to register CLI command: %u",
				  __func__, i);
			return -EINVAL;
		}
	}

	return 0;
}

#endif /* CLI_CMDS_DDR */
