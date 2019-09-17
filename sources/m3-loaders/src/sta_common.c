/**
 * @file sta_common.c
 * @brief Common functions and utilities used by both xloader and
 * xl_uflasher binaries.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>
#include <string.h>

/* Declare variables shared between M3 and AP processors */
#define DECLARE_SHARED_DATA

#include "FreeRTOS.h"
#include "list.h"
#include "trace.h"

#include "sta_type.h"
#include "sta_common.h"
#include "sta_ahbapb.h"
#include "sta_nic_security.h"
#include "sta_qos.h"
#include "sta_mtu.h"
#include "sta_src.h"
#include "sta_src_a7.h"
#include "sta_nvic.h"
#include "sta_nand.h"
#include "sdmmc.h"
#include "sta_sqi.h"
#include "sta_m3_irq.h"
#include "sta_mbox.h"
#include "sta_hsem.h"
#include "sta_a7.h"
#include "sta_pm.h"
#include "sta_mscr.h"
#include "stmpe1600.h"
#include "sta_uart.h"
#include "sta_adc.h"
#include "sta_platform.h"
#include "sta_mem_map.h"

#ifndef SOC_ID
#define SOC_ID SOCID_STA1295
#endif

#ifndef BOARD_ID
#define BOARD_ID BOARD_A5_EVB
#endif


/**
 * @brief	initializes Cortex-M3 low level core clocks, nic, qos
 * @return	0 if no error, not 0 otherwise
 */
int m3_lowlevel_core_setup(void)
{
	int err;

	ahbapb_init();

	/**
	 * Re-configure the clock tree:
	 * - disable the clock gates first
	 * - move to slow mode
	 * - reconfigure plls & clock muxes, clock dividers
	 * - move again to normal mode
	 * - enable again the clock gates
	 * */
	srcm3_pclk_disable_all(src_m3_regs);
	srca7_pclk_disable_all(src_a7_regs);
	srcm3_set_mode(src_m3_regs, SRCM3_MODECR_EXTOSC);
	srcm3_init(src_m3_regs);
	srca7_init(src_a7_regs);
	srcm3_set_mode(src_m3_regs, SRCM3_MODECR_NORMAL);
	srcm3_pclk_enable_all(src_m3_regs);
	srca7_pclk_enable_all(src_a7_regs);

	/* peripheral clocks must be on to program the nic, qos, etc... */
	nic_security_init();
	qos_init();

	/* Finally, only enable the clock gates that M3 OS really need */
	srcm3_pclk_disable_all(src_m3_regs);
	srca7_pclk_disable_all(src_a7_regs);
	err = sta_enable_clocks(src_m3_regs, src_a7_regs);
	if (!err) {
		srcm3_pclk_enable_all(src_m3_regs);
		srca7_pclk_enable_all(src_a7_regs);

		/*
		 * Enable C-A7s JTAG & STM debug
		 * Call to this function should be disabled whenever moving
		 * to production.
		 */
		srcm3_enable_debug(src_m3_regs, true, true, true);

		/* Set the priority group */
		err = nvic_set_priority_grp_config(NVIC_PRIORITY_GRP4);
	}

	return err;
}

/**
 * @brief	initializes Cortex-M3 other core controllers
 * (HSEM, Trace, MTU, ADC and WDT)
 * @param	context: current application context
 * @return	0 if no error, not 0 otherwise
 */
int m3_core_setup(struct sta *context)
{
	int err;

	/* Init MTU timers and enable background timer */
	err = mtu_init(mtu_regs, BG_TIMER_ON);
	if (err)
		return err;

	TRACE_INFO("%s:%s: GO\n", PLATFORM_NAME, context->bin);

	/* Init HSEM between M3 and A7 */
	hsem_init();

	/* SoC specifities */
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1275:
		/* Initialize DCOADC */
		err = sta_adc_init(dco_adc_regs);
		/* FALLTHROUGH */
		break;
	default:
		break;
	}

	return err;
}

/**
 * @brief	initializes MMC boot memory device
 * @param	context: current application context
 * @param	mmc_forced: given forced MMC port to boot on
 *			    SDMMC_NOPORT by default for automatic detection
 * @return	None
 */
void m3_get_mmc_boot_dev(struct sta *context, int mmc_forced)
{
	t_mmc *p_mmci;
	uint8_t mmc_bootnum;

	if (mmc_forced != SDMMC_NOPORT) {
		mmc_bootnum = mmc_forced;
		/*
		 * Workaround for A5 cut3 Errata#496427:
		 * Clear SDI DATA CONTROL reg
		 */
		switch (mmc_forced) {
		case SDMMC1_PORT:
			((t_mmc *)SDMMC1_BASE)->mmc_data_ctrl = 0;
			break;

		case SDMMC2_PORT:
			((t_mmc *)SDMMC2_BASE)->mmc_data_ctrl = 0;
			break;
		}
	} else {

#define MMCI_POWERON		0x00000003
		/*
		 * Search the MMC boot device number, following which
		 * MMCI is already power on by ROM code
		 */
		if (get_soc_id() == SOCID_STA1385) {
			if (((t_sdhc *)SDMMC0_BASE)->powercontrol.bit.sdbuspower)
				mmc_bootnum = SDMMC0_PORT;
			else
				mmc_bootnum = SDMMC1_PORT;
		} else {
			if (((t_sdhc *)SDMMC0_BASE)->powercontrol.bit.sdbuspower) {
				mmc_bootnum = SDMMC0_PORT;
			} else {
				p_mmci = (t_mmc *)SDMMC1_BASE;
				if ((p_mmci->mmc_power & MMCI_POWERON) == MMCI_POWERON) {
					mmc_bootnum = SDMMC1_PORT;
				} else {
					p_mmci = (t_mmc *)SDMMC2_BASE;
					mmc_bootnum = SDMMC2_PORT;
				}
				/*
				 * Workaround for A5 cut3 Errata#496427:
				 * Clear SDI DATA CONTROL reg */
				if (get_boot_dev_base() == 0)
					p_mmci->mmc_data_ctrl = 0;
			}
		}
	}

	set_mmc_boot_dev(mmc_bootnum);

	switch (mmc_bootnum) {
	case SDMMC0_PORT:
		context->boot_dev_name = "MMC0";
		break;
	case SDMMC1_PORT:
		context->boot_dev_name = "MMC1";
		break;
	case SDMMC2_PORT:
		context->boot_dev_name = "MMC2";
		break;
	}
}

/**
 * @brief	initializes boot memory device
 * @param	context: current application context
 * @return	0 if no error, not 0 otherwise
 */
int m3_boot_dev_setup(struct sta *context)
{
	int err = 0;

	TRACE_INFO("S:NAND/EMMC/SQI init\n");
#if defined SQI || defined MMC /* In SQI we use MMC also */
	m3_get_mmc_boot_dev(context, SDMMC_FORCED_PORT);
#endif
#if defined SQI
	context->boot_dev_name = "SQI";
	set_boot_dev_type(BOOT_SQI);
	context->sqi_ctx = sqi_init(0);
#elif defined NAND
	context->boot_dev_name = "NAND";
	set_boot_dev_type(BOOT_NAND);
	err = nand_init();
#elif defined MMC
	set_boot_dev_type(BOOT_MMC);

	context->mmc_ctx = sdmmc_init(get_mmc_boot_dev(), false);
	if (!context->mmc_ctx) {
		TRACE_ERR("%s: MMC%d init failed\n", __func__,
			  get_mmc_boot_dev());
		err = MMC_GENERIC_ERROR;
	}
#else
#error "No boot device set !"
#endif
	TRACE_INFO("E:NAND/EMMC/SQI init\n");

	/* Call memories tests if enabled */
	sdmmc_tests();
	sqi_tests(context->sqi_ctx);

	return err;
}

/**
 * @brief	Init trace on serial link or JTAG DCC
 * @param	context: current application context
 * @return	NA
 */
void m3_init_trace(struct sta *context)
{
#if defined TRACE_JTAG_DCC
	trace_init(JTAG_DCC_PORT, NULL, 0);
#else
	trace_init(context->trace_port, NULL, 0);
#endif
}

/**
 * @brief	initializes inter-OS communication such as IRQ forwarding,
 * Mailboxes, Hardware semaphore and IPC channel.
 * @param	context: current application context
 * @return	0 if no error, not 0 otherwise
 */
int m3_ipc_setup(struct sta *context)
{
	int err;

	/* Init interrupt forwarding from AP to M3 */
	err = m3irq_init();
	if (err) {
		TRACE_ERR("%s: init m3irq\n", __func__);
		return err;
	}

	/* Init Mailbox devices*/
	err = mbox_init("hsem mailbox");
	if (err)
		return err;
#ifdef ATF
	err = mbox_init("std mailbox");
	if (err)
		return err;
#endif
	return 0;
}


/**
 * @brief	get die temperature
 * @param	timeout: in ms
 * @return	the measured temperature (0 if not measured successfully)
 */
uint32_t m3_get_soc_temperature(uint32_t timeout)
{
	t_thsensor *thsensor_regs;
	uint32_t temperature = 0;

	switch(get_soc_id()) {
		case SOCID_STA1195:
		case SOCID_STA1295:
		case SOCID_STA1275:
			thsensor_regs = ((t_thsensor *) 0x481E0000);
			break;
		case SOCID_STA1385:
			thsensor_regs = ((t_thsensor *) 0x482B0000);
			break;
		default:
			return 0;
	}

	/* select temperature correction from SW */
	thsensor_regs->th_ctrl.bit.dcorrect_sw_sel = 1;

	/*
	 * some samples are not calibrated, as a consequence, we need to apply
	 * correction.
	 * We arbitrary set the correction to 22°C
	 */
	thsensor_regs->th_ctrl.bit.dcorrect_sw = 22;

	/* start temperature acquisition */
	thsensor_regs->th_ctrl.bit.pdn = 1;

	while (timeout > 0) {
		if (thsensor_regs->th_status.bit.dtrdy) {
			temperature = (thsensor_regs->th_status.bit.data - 87);
			goto end;
		}
		mdelay(1);
		timeout--;
	}

end:
	/* stop temperature acquisition */
	thsensor_regs->th_ctrl.bit.pdn = 0;

	return temperature;
}

/**
 * @brief Set the magic addr that is polled by the C-A7
 * ARM Trusted Firmware to figure out when M3 OS reached a state
 * so that it's safe to start the AP OS.
 * This sync mechanism is required to make sure critical SCP init
 * code execute before start of AP OS.
 */
void m3_set_ap_sync_point(uint8_t sync)
{
	set_sync_point(sync);
}

