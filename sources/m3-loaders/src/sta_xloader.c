/**
 * @file sta_xloader.c
 * @brief M3 Xloader entry point file
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>
#include <errno.h>
#include "trace.h"

#include "utils.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#include "sta_type.h"
#include "sta_common.h"
#include "sta_a7.h"
#include "sta_mtu.h"
#include "sta_nic_security.h"
#include "sta_ccc_if.h"
#include "sta_platform.h"
#include "sta_image.h"
#include "sta_pm.h"
#include "sta_ddr.h"
#include "sta_sqi.h"
#include "sta_uart.h"
#include "sta_gpio.h"
#include "sta_pinmux.h"
#include "sta_pmu.h"
#include "CSE_HAL.h"
#include "CSE_ext_manager.h"

#include "sdmmc.h"

#include "sta_cot.h"

#if defined(XIP) && defined(SQI)
/*
 * M3OS entry is __M3OS_FLASH_start + M3_XL size max (256 KB)
 * defined in m3_os.ld.E
 */
#define m3os_entry ((entry_point_t *)(FC_SLAVE_BASE + 256 * 1024))
#endif

/* M3 context */
static struct sta context;

/*
 * M3 TOC is embedded in M3 xloader in .sysconfig_init section
 * and it's inserted in M3 xloder image by STA flahloader tool
 */
static const struct partition_config_t m3_toc_init
		__attribute__((section(".sysconfig_init")));

/**
 * @brief	Set-up partition information in the shared data structure
 * @return	NA
 */
static inline void platform_std_partition_information(void)
{
	uint32_t snapshot_addr, snapshot_size;

	if (get_partition_info(AP_BL32_SNAPSHOT_ID, &snapshot_addr,
			       &snapshot_size)) {
		TRACE_INFO("%s: get_partition_info:%d failure - STD not functonal\n",
			   __func__, AP_BL32_SNAPSHOT_ID);
		set_std_info(STD_PART1, 0, 0);
	} else {
#ifdef MMC
		snapshot_addr *= MMC_BLOCK_SIZE;
#endif
		set_std_info(STD_PART1, snapshot_addr, snapshot_size);
	}

	if (get_partition_info(AP_TEE_SNAPSHOT_ID, &snapshot_addr,
			       &snapshot_size)) {
		TRACE_INFO("%s: get_partition_info:%d failure  - STD not functonal\n",
			   __func__, AP_TEE_SNAPSHOT_ID);
		set_std_info(STD_PART1, 0, 0);
	} else {
#ifdef MMC
		snapshot_addr *= MMC_BLOCK_SIZE;
#endif
		set_std_info(STD_PART2, snapshot_addr, snapshot_size);
	}
}

/**
 * @brief	Update SoC and board IDs from OTP_VR4 register or from
 *		an autodetection mecanism
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
		context->trace_port = UART3;
		break;
	case SOCID_STA1385:
		context->trace_port = UART2;
		break;
	default:
		break;
	}

	/* Reset ETH0 PHY for next boot stages */
	platform_eth0_reset_phy();

#if defined TRACE_EARLY_BUF
	/*
	 * Trace will be initialised later in M3OS,
	 * only initialise UART for next ATF boot stages
	 */
	uart_init(context->trace_port, BR115200BAUD,
			NOPARITY_BIT, ONE_STOPBIT, DATABITS_8);
#else
	m3_init_trace(context);
#endif
end :
	return err;
}

/**
 * @brief	Initializes platform HW interfaces
 * @param	context: xloader application context
 * @return	0 if no error, not 0 otherwise
 */
static int hardware_setup(struct sta *context)
{
	int err;

	/* Call pmu_init as earlier as possible */
	pmu_init();

	/* Identify the SoC at early boot stage to discriminate specific settings */
	m3_get_soc_id();

	if (srcm3_wdg_reset(src_m3_regs)) {
		sta_ddr_axi_reset();
		sta_ddr_core_reset();
		srcm3_reset(src_m3_regs);
	}

	/*
	 * Init pm suspend boot mode according to the type of wake-up:
	 * Cold boot, resume from STR or resume from STD
	 */
	pm_suspend_get_boot_mode();

	/* Reset trace buffers only in M3 xloader */
	memset(&trace, 0, sizeof(trace));

	err = platform_early_init(context);
	if (err)
		goto end;

	err = m3_lowlevel_core_setup();
	if (err)
		goto end;

#ifdef GPIO_BOOT_MONITORING
	{
		struct gpio_config pin;

		pin.direction = GPIO_DIR_OUTPUT;
		pin.mode = GPIO_MODE_SOFTWARE;
		pin.level	= GPIO_LEVEL_LEAVE_UNCHANGED;
		pin.trig	= GPIO_TRIG_LEAVE_UNCHANGED;
		gpio_set_pin_config(GPIO_BOOT_MONITORING, &pin);
		pmu_free_ao_gpio();
		gpio_set_gpio_pin(GPIO_BOOT_MONITORING);
	}
#endif

	err = m3_core_setup(context);
	if (err)
		goto end;

	TRACE_INFO("Bef sta_ddr_set_retention\n");
	/*
	 * need to maintain retention signal upon out-of-standby as long as
	 * PMU has not set ioctrl_gpioitf bit
	 */
	sta_ddr_pub_set_io_retention(true);
	pmu_free_ao_gpio();

	TRACE_INFO("After sta_ddr_set_retention\n");

	err = platform_init(context);
	if (err)
		goto end;

	TRACE_INFO("After platform_init\n");

	err = sta_ddr_init(pm_str_check_resume() == true ?
			   DDR_BOOT_REASON_EXIT_SELF_REF :
			   DDR_BOOT_REASON_COLD_BOOT);
	if (err)
		goto end;

	err = m3_boot_dev_setup(context);
	if (err)
		goto end;

	platform_recopy_bootinfo_in_ddr(false);

	platform_welcome_message(context);

	/* Always divide AP timers clock by 8 */
	a7_timers_clk_set();

	/*
	 * Init platform security:
	 * Init security driver according to the current SoC.
	 * Ensure the continuity of the Chain of Trust.
	 * Overwrite default non-secure config by the expected security
	 * setting.
	 */
	/* Set platform peripheral secure accesses */
	nic_set_security_periph_cfg();

	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1275:
#if defined(COT)
		if (get_cut_rev() >= CUT_30) {
			err = cot_init();
			if (err)
				goto end;
		}
#endif
		/* Set platform memory secure accesses */
		nic_set_security_mem_cfg();
		break;
	case SOCID_STA1385:
		/* Set platform security memory access configuration */
		nic_set_security_mem_cfg();

#if defined ESRAM_M3_HSM_MAILBOXES_BASE
		if (get_cut_rev() >= CUT_20) {
			err = hsm_init();
			if (err)
				goto end;
			err = hsm_update();
			if (err)
				goto end;
			/*
			 * Set the valid external memory ranges for eHSM
			 * interactions:
			 * 2 areas are defined:
			 * - Memory range useable in input and output
			 * - Memory range useable in input only
			 */
			err = CSE_ext_set_valid_ext_mem_range(
					EXT_MEM_RANGE_INOUT_START,
					EXT_MEM_RANGE_INOUT_END,
					EXT_MEM_RANGE_INPUT_START,
					EXT_MEM_RANGE_INPUT_END);
			if (err)
				goto end;

#if defined(COT)
			err = cot_init();
			if (err)
				goto end;
#endif

#ifdef A7_SHE_REG_MEM_START
			/*
			 * Set emulated register area exposed to A7 core for
			 * eHSM interactions if default config has been
			 * changed.
			 */
			err = CSE_API_ext_set_a7_she_reg_mem_config(
					A7_SHE_REG_MEM_START);
#endif
		}
#endif /* ESRAM_M3_HSM_MAILBOXES_BASE */
		break;
	default:
		break;
	}

end :
	if (err)
		TRACE_ERR("%s: failed to init hardware\n", __func__);
	return err;
}


/**
 * @brief	The main entry, responsible for peripherals init and
 *		application
 * @param	none
 * @return	never
 */
int main(void)
{
	int __maybe_unused err;
	struct xl1_part_info_t part_info;

	context.bin = BIN_NAME_XL;

	/*
	 * The M3 TOC (.sysconfig_init section) is embedded at start
	 * of M3 XLoader, it must be recopied in .sysconfig (m3_toc)
	 * at definitive place: last 512Bytes of M3 ESRAM to be persistent
	 * for next M3 boot stages
	 */
	m3_toc = m3_toc_init;

	/* Setup all required HW interfaces */
	if (hardware_setup(&context))
		goto exit;

#if defined(EHSM_TEST)
	/*
	 * eHSM test vectors are mapped in .rodata section and
	 * required memory size higher than area available in M3 eSRAM.
	 * So M3 OS is split in 2 parts and .rodata are loaded in DRAM.
	 */
	TRACE_NOTICE("Loading M3 OS part2 image from %s...\n",
		     context.boot_dev_name);
	err = read_image(M3_OS_PART2_ID, &context, NULL, 0);
	if (err)
		goto exit;
#endif

#if defined(ATF)
	set_m3_pen(NULL);
	m3_set_ap_sync_point(0);
	platform_std_partition_information();

#ifdef FORCE_STD_BOOT
	if (get_boot_mode() == COLD_BOOT)
		set_boot_mode(STD_BOOT);
	platform_recopy_bootinfo_in_ddr(true);
retry:
#endif

	switch (get_boot_mode()) {
	case STD_BOOT:
		set_m3_pen(NULL);

		TRACE_NOTICE("Loading AP ATF image for a STD Wake-Up from %s...\n",
			     context.boot_dev_name);
		err = read_image(AP_XL_ID, &context, &part_info, 0);
		if (err)
			goto exit;
		a7_start((uint32_t)part_info.entry_address);
		do {
			udelay(100);
			part_info.entry_address = (entry_point_t *)get_m3_pen();
		} while (!part_info.entry_address);
		set_m3_pen(NULL);
		a7_stop();
		sta_ddr_std_check(true);
		TRACE_NOTICE("Now the system is in a state to process a STR boot...\n");
#ifdef FORCE_STD_BOOT
		if (pm_get_ap_resume_entry() ==  PM_STR_RESUME_NOT_VALID) {
			TRACE_NOTICE("Snapshot Image Not valid so COLD boot...\n");
			set_boot_mode(COLD_BOOT);
			platform_recopy_bootinfo_in_ddr(true);
			goto retry;
		}
#endif
		/*
		 * Now the system is in a state to process a STR boot
		 * so no break there
		 */
		/* Intentionnal FALLTHROUGH */
	case STR_BOOT:
		/*
		 * Restore backup-ed trusted firmware context from DDR
		 * to esram A7.
		 */
		TRACE_NOTICE("Restoring AP ATF from DDR...\n");
		pm_suspend_restore_secure_mem();

		a7_start(pm_get_ap_resume_entry());
		break;

	case COLD_BOOT:
		TRACE_NOTICE("Loading AP ATF image from %s...\n",
			     context.boot_dev_name);
		err = read_image(AP_XL_ID, &context, &part_info, 0);
		if (err)
			goto exit;

		a7_start((uint32_t)part_info.entry_address);
		break;
	}

#if ! defined(BOOT_M3OS_FROM_M3XL)
	/* Wait M3 OS pen release from ATF BL2 */
	do {
		udelay(100);
		part_info.entry_address = (entry_point_t *)get_m3_pen();
	} while (part_info.entry_address == NULL);
#endif /* BOOT_M3OS_FROM_M3XL */
#endif /* ATF */

	/* Set platform security remap configuration */
	nic_set_security_remap_cfg();

#if defined(XIP)
	m3os_entry();
	goto exit;
#elif !defined(ATF) || defined(BOOT_M3OS_FROM_M3XL)

#if defined(ESRAM_A7_M3_CODE_BASE)
	/* M3 OS is split in 2 parts, the second part is in AP ESRAM */
	TRACE_NOTICE("Loading M3 OS part2 image from %s...\n",
		     context.boot_dev_name);
	err = read_image(M3_OS_PART2_ID, &context, NULL, 0);
	if (err)
		goto exit;
#endif /* ESRAM_A7_M3_CODE_BASE */
	TRACE_NOTICE("Loading M3 OS images from %s...\n",
		     context.boot_dev_name);
	err = read_image(M3_OS_ID, &context, &part_info, 0);
	if (err)
		goto exit;
#endif /* XIP */
	TRACE_NOTICE("\n");

	/* Jump to M3 entry point */
	part_info.entry_address();
	/* Normaly never returns here */

exit :
	wait_forever;
}

