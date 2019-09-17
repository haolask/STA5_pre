/**
 * @file sta_car_radio.c
 * @brief M3 car radio application main
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: APG-MID team
 */

#include <string.h>
#include <errno.h>
#include "trace.h"

#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sta_type.h"
#include "sta_common.h"
#include "sta_cssmisc.h"
#include "sta_mscr.h"
#include "sta_acc.h"
#include "sta_it.h"
#include "sta_gpio.h"
#include "sta_image.h"
#include "sta_systick.h"
#include "sta_pm.h"
#include "sta_pmu.h"
#include "sta_a7.h"
#include "sta_ccc_if.h"
#include "sta_lcd.h"
#include "sta_pinmux.h"
#include "sta_platform.h"
#include "sta_remote_gpio.h"
#include "sta_rpmsg.h"
#include "sta_rpmsg_rdm.h"
#include "sta_usb.h"
#include "sta_wdt.h"
#include "sta_wdt_a7.h"
#include "sta_dsp.h"
#include "sta_audio.h"
#include "sta_board_config.h"
#include "sta_cli.h"
#ifdef EARLY_TUNER_FTR
#include "sta_tuner.h"
#include "etal_api.h"
#endif

/* Test Tasks, in case of scheduler */
void RpmsgTestTask(void *p);
void MboxTestTask(void);
void can_task(void *p);
void ccc_tests(void);
void brd_cfg(void);
#ifdef SQINOR_CONACC_TEST
void sqi_nor_tests(void);
#endif
void early_audio_task(void *p);
int sta_mm_init_env(struct sta *ctxt);

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
	if (!context)
		return -EINVAL;

	/* Board specificities */
	switch (get_board_id()) {
	case BOARD_A5_EVB:
		context->features.rpmsg_css = true;
		context->features.rpmsg_mm = true;
		context->features.rpmsg_rdm = true;
		context->features.mbox_test = true;
		context->features.remote_gpio = true;
		context->features.splash_screen = true;
		context->features.splash_animation = true;
		context->features.rear_camera = true;
		context->features.can0 = true;
		context->features.can0_hello = true;
		context->features.early_audio = true;
#ifdef EARLY_TUNER_FTR
		context->features.early_tuner = true;
#endif
		context->audio_context.aux_detect = M3_GPIO(3);
		context->features.hdmi = true;
		break;
	default:
		break;
	}

	/* SoC specifities */
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1275:
#ifdef CLI_ENABLED
		context->trace_port = UART0;
#else
		context->trace_port = UART3;
#endif
		break;
	case SOCID_STA1385:
#ifdef CLI_ENABLED
		context->trace_port = UART1;
#else
		context->trace_port = UART2;
#endif
		break;
	default:
		break;
	}

	context->features.uart_console_no = context->trace_port;
#if !defined TRACE_EARLY_BUF
	/* Trace will be initialized later if TRACE_EARLY_BUF */
	m3_init_trace(context);
#endif

	/* Display detection */
	(void)lcd_select_display_cfg();

	return 0;
}

/**
 * @brief	This function boot the system
 * @return	0 if no error, not 0 otherwise
 */
static int boot_system(struct sta *context)
{
	int err = 0;
	entry_point_t *m3_entry;
	struct xl1_part_info_t part_info;
	int dsp, j;
	bool have_m3_os = false;
	struct audio_context *audio_context = &context->audio_context;

	TRACE_NOTICE("Loading images from %s...\n", context->boot_dev_name);

	if (!context->features.early_audio)
		goto no_early_audio;

	dac_power_on();
	early_src_init();

	if (pm_str_check_resume())
		goto no_early_audio;

	/* Load DSPs firmwares if any */
	for (dsp = DSP0; dsp < DSP_MAX_NO; dsp++) {
		dsp_clk_enable(dsp);
		audio_context->have_dsp[dsp] = true;
		for (j = 0; j < 3; j++) {
			if (read_image(DSP0_X_ID + dsp * 3 + j, context, &part_info, 0)) {
				dsp_clk_disable(dsp);
				audio_context->have_dsp[dsp] = false;
				break; /* No dsp try next one */
			}
		}
	}

	/* Load early audiolib if any */
	if (read_image(AUDIO_LIB_ID, context, &part_info, IMAGE_SHADOW_ALLOC_LIBC) == 0)
		audio_context->audiolib_addr = part_info.shadow_address;

	/* Create Early audio task */
	if (!audio_context->audiolib_addr)
		goto no_early_audio;

	/* Load audio files if any */
	if (read_image(FILE1_ID, context, &part_info, IMAGE_SHADOW_ALLOC_LIBC) == 0)
		audio_context->w1 = part_info.shadow_address;

	if (read_image(FILE2_ID, context, &part_info, IMAGE_SHADOW_ALLOC_LIBC) == 0)
		audio_context->w2 = part_info.shadow_address;

	err = xTaskCreate((pdTASK_CODE) early_audio_task,
			  (char *)"Early Audio", 110,
			  (void *)audio_context, TASK_PRIO_MAX, NULL);
	if (err != pdPASS)
	    TRACE_ERR("Error to create early audio task\n");

no_early_audio:

#ifdef EARLY_TUNER_FTR
	if (context->features.early_tuner) {
		if (read_image(FIRMWARE1_ID, context,
			       &part_info, IMAGE_SHADOW_ALLOC_LIBC) == 0) {
			tuner_FWM_update(part_info.shadow_address,
					 part_info.size);
		}

		/* Create early tuner task */
		err = xTaskCreate((pdTASK_CODE) early_tuner_task,
				  (char *)"Early tuner task", 256 * 2,
				  (void *)context, TASK_PRIO_MAX, NULL);
		if (err != pdPASS)
			TRACE_ERR("Error to create early tuner task\n");
	}
#endif

	sta_mm_init_env(context);

	/* Load M3 OS2 if any */
	have_m3_os = !read_image(M3_OS2_ID, context, &part_info, 0);
	m3_entry = part_info.entry_address;

#if !defined(ATF)
	/* Load AP xloader */
	err = read_image(AP_XL_ID, context, &part_info, 0);
	if (err)
		return err;
#endif

	if (get_soc_id() != SOCID_STA1385)
		ccc_deinit();
#if !defined(ATF)
	/*
	 * All images must be read before Starting the AP,
	 * to avoid boot device access conflict
	 * Starting the AP core always at given address
	 */
	TRACE_INFO("%s:%s: AP GO\n", PLATFORM_NAME, context->bin);
	a7_start((uint32_t) part_info.entry_address);
#endif

	/* Start M3 OS if necessary */
	if (have_m3_os) {
		TRACE_NOTICE("STARTING M3 OS\n");
		/* Jump to M3 entry point */
		m3_entry();
		/* If there, it's an error */
		return -EFAULT;
	}

	return 0;
}

/**
 * @brief	boots up the system platform HW interfaces
 * @param	none
 * @return	none
 */
static void bootflow_task(void *p)
{
	int err;
	struct sta *ctx = (struct sta *)p;
	EventBits_t event_bits_set;
	/*
	 * In case no event is triggered, let's proceed with booting
	 * This entity is in charge of boting directly
	 */
	TRACE_NOTICE("Booting...\n");

	ctx->event_group = xEventGroupCreate();
	if (!ctx->event_group) {
		TRACE_ERR("Cannot create event group\n");
		wait_forever;
	}

	err = boot_system(ctx);
#if defined(TRACE_EARLY_BUF)
	/* Drain trace early buf after booting */
	m3_init_trace(&context);
#endif
	if(err) {
		TRACE_ERR("BOOT FAILED: %d\n", err);
		wait_forever;
	}

	/* Wait for all tasks that declared an event bit to set it.
	 * The tasks for which event_bitmask has been set should call
	 * xEventGroupSetBits once they have finished to used shared
	 * resources.
	 */
	event_bits_set =
		xEventGroupWaitBits(ctx->event_group,
				    ctx->event_bitmask,
				    pdFALSE,
				    pdTRUE,
				    pdMS_TO_TICKS(TASK_SYNC_TIMEOUT_MS));
	if ((event_bits_set & ctx->event_bitmask) == ctx->event_bitmask) {
		TRACE_INFO("Event group sync PASSED - bits set: 0x%X\n",
			   event_bits_set);
	} else {
		TRACE_ERR("Event group sync TIMEOUT\n");
		TRACE_ERR("  => bitmask: 0x%X - bits set: 0x%X\n",
			  ctx->event_bitmask, event_bits_set);
		wait_forever;
	}

#if defined(ATF)
	m3_set_ap_sync_point(1);
#endif

	vTaskDelete(NULL);
}

/**
 * @brief	initializes platform HW interfaces
 * @param	context: main application context
 * @return	0 if no error, not 0 otherwise
 */
static int hardware_setup(struct sta *context)
{
	int err;

	err = platform_early_init(context);
	if (err)
		goto end;

	err = m3_core_setup(context);
	if (err)
		goto end;

	if (context->features.wdt) {
		struct sta_wdt_pdata wdt_pdata;

		/* Watchdog init */
		wdt_pdata.timeout = 60;
		wdt_pdata.reload_interval = 5;
		wdt_pdata.clk_rate = m3_get_mxtal() / 8;
		wdt_pdata.reg_base = (t_wdt *)WDT_BASE;
		err = wdt_init(&wdt_pdata);
		if (!err)
			err = wdt_enable();
	}

	err = platform_init(context);
	if (err)
		goto end;

	err = m3_boot_dev_setup(context);
	if (err)
		goto end;

	/* Initialize misc and src display registers */
	err = lcd_init_misc_src_reg();
	if (err)
		goto end;

	platform_welcome_message(context);

	err = m3_ipc_setup(context);
	if (err)
		goto end;

	/* Init Power Management */
	err = pm_init(context);
	if (err)
		goto end;

	/* Configure HCLK clock as SysTick clock source. */
	systick_clk_source_config(SYSTICK_CLK_SOURCE_HCLK);

	err = usb_phy_init();
	if (err)
		goto end;

	if (get_soc_id() != SOCID_STA1385) {
		err = ccc_init();
		if (err)
			goto end;
	}

	if (!context->features.no_gfx)
		src_a7_regs->scresctrl1.bit.gfx_sw_rst = 1;

end:
	if (err)
		TRACE_ERR("%s: failed to init hardware\n", __func__);
	return err;
}



/**
 * @brief	the main task, responsible for peripherals init and
 * starting tasks.
 * @param	none
 * @return	never
 */
int main(void)
{
	int err;

	context.bin = BIN_NAME_OS;
	context.features.wdt = true;

	/* Setup all required HW interfaces */
	err = hardware_setup(&context);
	if (err)
		goto end;

	/* Create Boot Flow task */
	err = xTaskCreate((pdTASK_CODE) bootflow_task,
			(char *) "Boot Flow", 300,
			(void *) &context, TASK_PRIO_BOOTFLOW, NULL);
	if (err != pdPASS)
		goto end;

	/* Create PM task */
	err = xTaskCreate((pdTASK_CODE) pm_task,
			(char *)"Power & Reset", 120,
			(void *)&context, TASK_PRIO_PM, NULL);
	if (err != pdPASS)
		goto end;

	/* RPMSG resource pre-init */
	err = RPMSG_PreInit();
	if (err)
		goto end;

	if (context.features.wdt) {
		/* Create M3 Watchdog task */
		err = xTaskCreate((pdTASK_CODE)wdt_task,
				(char *) "Watchdog M3", 68,
				(void *) &context.features, TASK_PRIO_WDT_M3, NULL);
		if (err != pdPASS)
			goto end;

		/* Create A7 Watchdog task */
		err = xTaskCreate((pdTASK_CODE)wdt_a7_task,
				(char *) "Watchdog A7", 100,
				(void *) NULL, TASK_PRIO_WDT_A7, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create Mailbox test task */
	if (context.features.mbox_test) {
		err = xTaskCreate((pdTASK_CODE) MboxTestTask,
				(char *) "Mbox Test", 130,
				(void *) NULL, TASK_PRIO_TEST_MAILBOX, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create remote gpio task */
	if (context.features.remote_gpio) {
		err = xTaskCreate((pdTASK_CODE) remote_gpio_task,
				  (char *)"Remote GPIO control", 100,
				  (void *)NULL, TASK_PRIO_TEST_MAILBOX, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create RPMSG task */
	if (context.features.rpmsg_css) {
		err = xTaskCreate((pdTASK_CODE) RpmsgTestTask,
				(char *) "Rpmsg Test", 128,
				(void *) &context.features, TASK_PRIO_TEST_RPMSG, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create RPMSG RDM task */
	if (context.features.rpmsg_rdm) {
		err = xTaskCreate((pdTASK_CODE) rdm_task,
				(char *)"Rpmsg Remote Device Manager task", 128,
				(void *)&context.features, TASK_PRIO_RPMSG_RDM, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create CAN task */
	if (context.features.can0) {
		err = xTaskCreate((pdTASK_CODE) can_task,
			(char *) "Can", 100,
			(void *) &context.features, TASK_PRIO_TEST_CAN, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create CCC test task */
	if (context.features.ccc_test) {
		err = xTaskCreate((pdTASK_CODE) ccc_tests,
				(char *) "CCC Test", 128,
				(void *) NULL, TASK_PRIO_TEST_CCC, NULL);
		if (err != pdPASS)
			goto end;
	}

	/* Optionally create board_config test task */
	if (context.features.hdmi) {
		err = xTaskCreate((pdTASK_CODE) brd_cfg,
				(char *) "Board config", 84,
				(void *) &context, TASK_PRIO_BRD_CFG, NULL);
		if (err != pdPASS)
			goto end;
	}

#ifdef CLI_ENABLED
	err = xTaskCreate((pdTASK_CODE) cli_task,
			  (char *) "console task", 128,
			  (void *) NULL, TASK_PRIO_CLI, NULL);
	if (err != pdPASS)
		goto end;
#endif

#ifdef SQINOR_CONACC_TEST
	err = xTaskCreate((pdTASK_CODE) sqi_nor_tests,
			(char *) "SQINOR Con. Acc. Test 0", 128,
			(void *) &context, TASK_PRIO_TEST_SQI, NULL);
	if (err != pdPASS)
		goto end;

#ifdef MULTIPLE_SQINOR_TEST_TASKS
	/*
	 * HW semaphore are preventing accesses from different cores, but they
	 * are not preventing simultaneous accesses from the same core. As our
	 * current driver implementation does not support reentrant code, we do
	 * not manage more than one task accessing the SQI-NOR
	 */
#if 0
	err = xTaskCreate((pdTASK_CODE) sqi_nor_tests,
			(char *) "SQINOR Con. Acc. Test 1", 128,
			(void *) &context, TASK_PRIO_TEST_SQI_1, NULL);
	if (err != pdPASS)
		goto end;

	err = xTaskCreate((pdTASK_CODE) sqi_nor_tests,
			(char *) "SQINOR Con. Acc. Test 2", 128,
			(void *) &context, TASK_PRIO_TEST_SQI_2, NULL);
	if (err != pdPASS)
		goto end;
#endif
#endif
#endif

	/* Start the scheduler. */
	vTaskStartScheduler();

end:
	/* Will only get here if there was insufficient memory to create the idle
	   task.  The idle task is created within vTaskStartScheduler(). */

	wait_forever;
}

