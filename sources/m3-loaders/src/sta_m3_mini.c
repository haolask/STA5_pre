/**
 * @file sta_m3_mini.c
 * @brief Minimal M3 binary entry point file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>
#include <string.h>

#include "utils.h"

#include "FreeRTOS.h"
#include "list.h"
#include "queue.h"
#include "semphr.h"

#include "sta_type.h"
#include "sta_common.h"
#include "sta_cssmisc.h"
#include "sta_mscr.h"
#include "sta_acc.h"
#include "sta_it.h"
#include "trace.h"
#include "sta_uart.h"
#include "sta_mtu.h"
#include "sta_a7.h"
#include "sta_ddr.h"
#include "sta_pinmux.h"
#include "sta_platform.h"
#include "sta_usb.h"
#include "sta_cli.h"
#include "sta_pmu.h"
#include "sta_ddr.h"

/* M3 context */
static struct sta context;

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
/**
 * @brief	get called if a task overflows its stack
 * @param	task: task handle
 * @param	task_name: task name
 */
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
	/* This function will get called if a task overflows its stack.   If the
	   parameters are corrupt then inspect pxCurrentTCB to find which was the
	   offending task. */

	(void) pxTask;

	TRACE_ERR("Stack OVF %s\n", pcTaskName);

	wait_forever;
}
#endif /* configCHECK_FOR_STACK_OVERFLOW */

/**
 * @brief	Update SoC and board IDs from OTP_VR4 register or from
 * an autodetection mecanism
 */
static int platform_init(struct sta *context)
{
	int err;

	if (!context)
		return -EINVAL;

	err = m3_get_board_id();
	if (err)
		goto end;

	/* SoC specifities */
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1275:
		pinmux_request("uart3_mux");
		context->features.uart_console_no = UART_P3;
		context->trace_port = UART3;
		break;
	case SOCID_STA1385:
		pinmux_request("uart2_mux");
		context->features.uart_console_no = UART_P2;
		context->trace_port = UART2;
		break;
	default:
		break;
	}

	m3_init_trace(context);

end:
	return err;
}

/**
 * @brief	initializes platform HW interfaces
 * @param	context: main application context
 * @return	0 if no error, not 0 otherwise
 */
static int m3_mini_hardware_setup(struct sta *context)
{
	int err;

	/* Identify the SoC at early boot stage to discriminate specific settings */
	m3_get_soc_id();

	if (srcm3_wdg_reset(src_m3_regs)) {
		sta_ddr_axi_reset();
		sta_ddr_core_reset();
		srcm3_reset(src_m3_regs);
	}

	err = platform_early_init(context);
	if (err)
		goto end;

	err = m3_lowlevel_core_setup();
	if (err)
		goto end;

	err = m3_core_setup(context);
	if (err)
		goto end;

	/*
	 * need to maintain retention signal upon out-of-standby as long as
	 * PMU has not set ioctrl_gpioitf bit
	 */
	sta_ddr_pub_set_io_retention(true);
	pmu_free_ao_gpio();

	err = platform_init(context);
	if (err)
		goto end;

	platform_welcome_message(context);

	/* Always divide A7 timers clock by 8 */
	a7_timers_clk_set();

	cli_task(NULL);

	err = sta_ddr_init(DDR_BOOT_REASON_COLD_BOOT);
	if (err)
		goto end;

	platform_recopy_bootinfo_in_ddr(false);

end:
	if (err)
		TRACE_ERR("%s: failed to init hardware\n", __func__);

	return err;
}

/**
 * @brief	main entry point function
 * @param	none
 * @return	never
 */
int main(void)
{
	context.bin = BIN_NAME_M3MIN;

	/*
	 * Use UART0 config: 115200 bds, NoParity, 1stop, 8bits data
	 * because UART3 is already used for U-boot flasher loading
	 */
	if (get_soc_id() == SOCID_STA1385) {
		context.trace_port = UART1;
	} else {
		context.trace_port = UART3; /* or JTAG_DCC_PORT */
	}

	/* Setup all required HW interfaces */
	m3_mini_hardware_setup(&context);

	trace_printf("Waiting for ever...\n");

	wait_forever;

	return 0;
}


