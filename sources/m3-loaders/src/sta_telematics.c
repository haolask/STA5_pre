/**
 * @file sta_telematics.c
 * @brief M3 telematics application main
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
#include "sta_pinmux.h"
#include "sta_platform.h"
#include "sta_regmap.h"
#include "sta_remote_gpio.h"
#include "sta_rpmsg.h"
#include "sta_rpmsg_rdm.h"
#include "sta_usb.h"
#include "sta_wdt.h"
#include "sta_wdt_a7.h"
#include "sta_mtu.h"
#include "sta_ks.h"
#include "CSE_HAL.h"
#include "rpmx_common.h"
#include "sta_cli.h"

#define MODEM_RING PMU_WAKE1

/* Test Tasks, in case of scheduler */
void RpmsgTestTask(void *p);
void MboxTestTask(void);
void can_task(void *p);
#ifdef EHSM_TEST
void hsm_test_menu_task(void);
#else
void ccc_tests(void);
#endif
#ifdef SQINOR_CONACC_TEST
void sqi_nor_tests(void);
#endif

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

#if ( INCLUDE_xTaskGetSchedulerState == 1 )

static inline void msTaskSleep(uint32_t mtime)
{
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		mdelay(mtime);
	else
		vTaskDelay(pdMS_TO_TICKS(mtime));
}

#else

static inline void msTaskSleep(uint32_t mtime)
{
	mdelay(mtime);
}

#endif

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
#ifdef EHSM_TEST
	case BOARD_TC3P_MTP:
	case BOARD_TC3P_CARRIER:
		/* Task for key storage */
		context->features.ks = true;
		/* Task for eHSM test framework */
		context->features.hsm_test = true;
		break;
#else
	case BOARD_TC3P_MTP:
		if (get_cut_rev() >= CUT_20)
			context->features.ks = true; /* Task for key storage */
		context->features.accelerometer = true;
		/* intentional fallthrough */
	case BOARD_TC3_EVB:
		context->features.modem = true;
		/* intentional fallthrough */
	case BOARD_TC3P_CARRIER:
		/* Test tasks for IPC */
		context->features.rpmsg_css = true;
		context->features.rpmsg_rdm = true;
		context->features.mbox_test = true;
		context->features.remote_gpio = true;
		context->features.can0 = true;
		context->features.can0_hello = true;
		context->features.aux_detect = -1;
		break;
#endif
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
		/* enable the muxing logic that bind uart4, uart5 to DMACs */
		misc_a7_regs->misc_reg66.bit.soc_uart_select = 1;
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
	bool have_m3_os = false;

	TRACE_NOTICE("Loading images from %s...\n", context->boot_dev_name);

	/* Load File1 if any */
	read_image(FILE1_ID, context, NULL, 0);

	/* Load File2 if any */
	read_image(FILE2_ID, context, NULL, 0);

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

	return err;
}

/**
 * @brief	boots up the system platform HW interfaces
 * @param	none
 * @return	none
 */
static void bootflow_task(void *p)
{
	int err;
	/*
	 * In case no event is triggered, let's proceed with booting
	 * This entity is in charge of boting directly
	 */
	TRACE_NOTICE("Booting...\n");

	err = boot_system((struct sta *)p);
#if defined(TRACE_EARLY_BUF)
	/* Drain trace early buf after booting */
	m3_init_trace(&context);
#endif
	if(err) {
		TRACE_ERR("BOOT FAILED: %d\n", err);
		wait_forever;
	}

	vTaskDelete(NULL);
}

static void gpio_modem_init(struct sta *context)
{
	struct gpio_config pin;
	if (get_board_id() == BOARD_TC3P_MTP) {
		if ((context->features.modem) && !(pmu_get_wkup_stat() & PMU_WAKE3)) {
			pinmux_request("modem_mux");

			pin.direction   = GPIO_DIR_INPUT;
			pin.mode        = GPIO_MODE_SOFTWARE;
			pin.level       = GPIO_LEVEL_PULLDOWN;
			pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
			/* Export the AO_GPIO 3 MODEM_RING */
			gpio_set_pin_config(AO_GPIO(3), &pin);

			pin.direction   = GPIO_DIR_OUTPUT;
			pin.mode        = GPIO_MODE_SOFTWARE;
			pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
			pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
			/* Export and reset the GPIO 53 MODEM_SHUTDOWN */
			gpio_set_pin_config(A7_GPIO(53), &pin);
			gpio_clear_gpio_pin(A7_GPIO(53));
			/* Export the M3_GPIO 7 MODEM_WAKEUP_CTRL */
			gpio_set_pin_config(M3_GPIO(7), &pin);
			/* Export and manage the GPIO 56 MODEM_ON_OFF */
			gpio_set_pin_config(A7_GPIO(56), &pin);
			gpio_clear_gpio_pin(A7_GPIO(56));
			gpio_set_gpio_pin(A7_GPIO(56));
		}
	} else {
		if ((context->features.modem) && !(pmu_get_wkup_stat() & MODEM_RING)) {
			pinmux_request("modem_mux");

			pin.direction   = GPIO_DIR_INPUT;
			pin.mode        = GPIO_MODE_SOFTWARE;
			pin.level       = GPIO_LEVEL_PULLDOWN;
			pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
			gpio_set_pin_config(M3_GPIO(2),&pin);

			pin.direction   = GPIO_DIR_OUTPUT;
			pin.mode        = GPIO_MODE_SOFTWARE;
			pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
			pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
			/* Export and reset the GPIO 16 */
			gpio_set_pin_config(A7_GPIO(16), &pin);
			gpio_clear_gpio_pin(A7_GPIO(16));
			/* Export the GPIO 34 */
			gpio_set_pin_config(A7_GPIO(50), &pin);
			/* Export and manage the GPIO 30 */
			gpio_set_pin_config(A7_GPIO(17), &pin);
			gpio_clear_gpio_pin(A7_GPIO(17));
			gpio_set_gpio_pin(A7_GPIO(17));
		}
	}
}

/**
 * @brief	initializes platform HW interfaces
 * @param	context: main application context
 * @return	0 if no error, not 0 otherwise
 */
static int hardware_setup(struct sta *context)
{
	int err;
	struct gpio_config pin;

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
		if ((get_soc_id() == SOCID_STA1385) &&
		    (get_cut_rev() >= CUT_20))
			wdt_pdata.reg_base = (t_wdt *)WDT_ASYNC_APB_BASE;
		else
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

	platform_welcome_message(context);

	err = m3_ipc_setup(context);
	if (err)
		goto end;

	/* Init Power Management */
	err = pm_init(context);
	if (err)
		goto end;
#if defined(ATF)
	err = regmap_init();
	if (err)
		goto end;
#endif
	/* Configure HCLK clock as SysTick clock source. */
	systick_clk_source_config(SYSTICK_CLK_SOURCE_HCLK);

	err = usb_phy_init();
	if (err)
		goto end;

	/* Init platform security */
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1275:
		err = ccc_init();
		break;
	case SOCID_STA1385:
		if (get_cut_rev() >= CUT_20) {
			err = hsm_init();

			/*
			 * Set timer function for eHSM timeout handling.
			 *
			 * Disable by default in order to emphasise ehsm real
			 * time performance compared to multitask time sharing
			 */
			//MS_DELAY = msTaskSleep;
		}
		break;
	default:
		break;
	}
	if (err)
		goto end;

	if (context->features.modem)
		gpio_modem_init(context);

	if ((get_board_id() == BOARD_TC3P_MTP) ||
	    (get_board_id() == BOARD_TC3P_CARRIER)) {
		/* Configure AO_GPIO4 pin to use for SOS wakeup */
		pin.direction   = GPIO_DIR_INPUT;
		pin.mode        = GPIO_MODE_SOFTWARE;
		pin.level       = GPIO_LEVEL_PULLDOWN;
		pin.trig        = GPIO_TRIG_RISING_EDGE;
		gpio_set_pin_config(AO_GPIO(4), &pin);
	}
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
#if defined(ATF) && (!defined(EHSM_TEST) || defined(CLI_ENABLED))
	/*
	 * In case of EHSM test, keep A7 blocked in atf in order to keep
	 * console on M3.
	 */
	m3_set_ap_sync_point(1);
#endif
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

	/* Optionally create Accelerometer task */
	if (context.features.accelerometer) {
		err = xTaskCreate((pdTASK_CODE) acc_task,
				(char *) "ACC", 100,
				(void *) NULL, TASK_PRIO_TEST_ACC, NULL);
		if (err != pdPASS)
			goto end;
	}

#if defined(CLI_ENABLED) && !defined(EHSM_TEST)
	err = xTaskCreate((pdTASK_CODE) cli_task,
			  (char *) "console task", 128,
			  (void *) NULL, TASK_PRIO_CLI, NULL);
	if (err != pdPASS)
		goto end;
#endif

#if KS_MEMTYPE != NVM_NONE
	/* Optionally create key storage task */
	if (context.features.ks) {
		err = xTaskCreate((pdTASK_CODE) key_storage_task,
				  (char *)"Key Storage", 300,
				  (void *) NULL, TASK_PRIO_KS, NULL);
		if (err != pdPASS)
			goto end;
	}
#endif /* KS_MEMTYPE != NVM_NONE */

#ifdef EHSM_TEST
	/* Optionally create HSM test task */
	if (context.features.hsm_test) {
		err = xTaskCreate((pdTASK_CODE) hsm_test_menu_task,
				  (char *)"HSM Test", 1536,
				  (void *)NULL, TASK_PRIO_TEST_HSM, NULL);
		if (err != pdPASS)
			goto end;
	}
#else
	/* Optionally create CCC test task */
	if (context.features.ccc_test) {
		err = xTaskCreate((pdTASK_CODE) ccc_tests,
				(char *) "CCC Test", 256,
				(void *) NULL, TASK_PRIO_TEST_CCC, NULL);
		if (err != pdPASS)
			goto end;
	}
#endif

#ifdef SQINOR_CONACC_TEST
	err = xTaskCreate((pdTASK_CODE) sqi_nor_tests,
			(char *) "SQINOR Con. Acc. Test task 0", 128,
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
			(char *) "SQINOR Con. Acc. Test task 1", 128,
			(void *) &context, TASK_PRIO_TEST_SQI_1, NULL);
	if (err != pdPASS)
		goto end;

	err = xTaskCreate((pdTASK_CODE) sqi_nor_tests,
			(char *) "SQINOR Con. Acc. Test task 2", 128,
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

