/**
 * @file sta_platform.c
 * @brief Platform dedicated functions and utilities used by both xloader and
 * xl_uflasher binaries.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "sta_common.h"

#include "sta_cssmisc.h"
#include "sta_ddr.h"
#include "sta_mtu.h"
#include "sta_gpio.h"
#include "sta_pinmux.h"

#include "stmpe1600.h"
#include "tca9535.h"
#include "flash_cache.h"

#include "sta1295_clk.h"
#include "sta1385_clk.h"
#include "sta_platform.h"

#define STMPE1600_I2C_ADDR 0x42
#define TCA9535_I2C_ADDR   0x20

#define ROM_VERSION_ADDR	0x10047FF8
#define ROM_BIST_ADDR		0x10047FFC

/* Private variables: */
static struct shared_data_bi_t __attribute__((section(".shared_m3_data"))) shared_data_bi;

/* Public variables: */
struct shared_data_bi_t *p_shared_data_bi = &shared_data_bi;

/**
 * @brief	Platform early init
 */
int __attribute__((weak)) platform_early_init(struct sta *context)
{
#if (SOC_ID == SOCID_STA1385)
	if (get_cut_rev() >= CUT_20) {
#if defined(XIP) && defined(SQI)
		/* FIXME not necessary if the ROM already do it */
		/* Init flash cache instruction for SQI */
		fc_init_cache(SQI0_NOR_BB_BASE);
		fc_disable_cache();
		fc_enable_cache(FC_MANUAL_POWER_AUTO_INVAL);
#elif defined(ESRAM_A7_M3_CODE_BASE)
		/* Init flash cache instruction for AP_ESRAM */
		fc_init_cache(ESRAM_A7_BASE);
		fc_disable_cache();
		fc_enable_cache(FC_MANUAL_POWER_AUTO_INVAL);
#endif
	}
#endif /* (SOC_ID == SOCID_STA1385) */

#if defined TRACE_EARLY_BUF
	/* Early trace initialization (in a buffer by default) */
	trace_init(NO_TRACE_PORT, trace.trace_early_buf,
		   sizeof(trace.trace_early_buf));
#else
	trace_init(NO_TRACE_PORT, NULL, 0); /*  No trace */
#endif

	/* Unmask Arasan SDHCI interrupts */
	misc_a7_regs->misc_reg11.bit.sdio_ahb_intr = 1;
	misc_a7_regs->misc_reg11.bit.sdio_ahb_wake_up = 1;
	/* 3.3V supply is selected for SDIO IOs (CLK,CMD, DATA[3:0]) */
	misc_a7_regs->misc_reg3.bit.sdio_voltage_select = 0;
	/* SDIO feedback clock without IO delay */
	misc_a7_regs->misc_reg3.bit.sdio_fbclk_select = 1;

	if (!context)
		return -EINVAL;

	return 0;
}

/**
 * @brief	Identifies the SoC identification and fills the information
 * accordingly.
 * @param	None
 * @return	0 if no error, not 0 otherwise
 */
int m3_get_soc_id(void)
{
	int i;
	struct otp_vr5_reg *p_otp_vr5 =
		(struct otp_vr5_reg *)(&shared_data_bi.otp_regs[5]);
	uint8_t erom_version = (*((int *)ROM_VERSION_ADDR) & 0xFF);

	/* First reset M3 ESRAM shared boot info */
	memset(&shared_data_bi, 0, sizeof(shared_data_bi));

	/* Recopy OTP registers at End of AP ESRAM for AP access */
	for (i = 0; i < OTP_SECR_VR_MAX; i++)
		shared_data_bi.otp_regs[i] =
			read_reg(OTP_MISC_M3_BASE + OTP_SECR_VR(i));

	shared_data_bi.erom_version = erom_version;
	shared_data_bi.soc_id = SOC_ID; /* Get SoC ID from compilation flag */

	/* Get SoC cut revision from OTP */
#if SOC_ID != SOCID_STA1385
	/*  Accordo5 SoCs familly */
	switch (shared_data_bi.otp_regs[4]) {
	case 0:
		/* Engeneering sample: default cut1 */
		if (erom_version == 0x30) /* If ROM v3.0 => cut 3 */
			shared_data_bi.cut_rev = CUT_30; /* It's a cut 3 */
		else
			shared_data_bi.cut_rev = CUT_10;
		break;

	case 0x1001: /* This is a STA1295 cut2 ROM 1.0 (not a STA1195) */
		shared_data_bi.cut_rev = CUT_20; /* It's a cut 2 */
		break;

	default: /* OTP programmed */
		/*
		 * On Accordo5 familly: 0 => Cut2.0, 1 => Cut3.0, etc... => +2
		 */
		shared_data_bi.cut_rev = (p_otp_vr5->cut_rev + 2) << 4;
		break;
	}
#else
	/* TC3P SoCs familly */
	switch (erom_version) {
	case 0x10: /* if ROM v1.0 => cut1 */
		shared_data_bi.cut_rev = CUT_10;
		break;

	case 0x20: /* if ROM v2.0 => cut2 (2.0 or 2.1)*/
		shared_data_bi.cut_rev = CUT_20;
		break;

	default: /* Use cut rev field in OTP if any */
		switch (p_otp_vr5->cut_rev) {
		case 0x00:
			shared_data_bi.cut_rev = CUT_10;
			break;

		case 0x06:
			shared_data_bi.cut_rev = CUT_22; /* Cut 2 BF */
			break;

		default:
			shared_data_bi.cut_rev = CUT_UNKNOWN;
			break;
		}
		break;
	}
#endif /* SOC_ID */

	return 0;
}

/**
 * @brief	Identifies the board identification
 * @param	None
 * @return	0 if no error, not 0 otherwise
 */
int m3_get_board_id(void)
{
	uint8_t board_rev_id;
	uint8_t board_id;
	uint16_t val = 0;

	/* Default board and rev */
	board_id = BOARD_ID;
	board_rev_id = 0;
	/* Try to read Board rev ID from STMPE1600 & TCA9535 GPIO expanders */
	if (!stmpe1600_Init(STMPE1600_I2C_ADDR) &&
	    stmpe1600_ReadID(STMPE1600_I2C_ADDR) == STMPE1600_ID) {
		stmpe1600_IO_InitPin(STMPE1600_I2C_ADDR, STMPE1600_PIN_ALL,
				     STMPE1600_DIRECTION_IN);
		val = stmpe1600_IO_ReadPin(STMPE1600_I2C_ADDR,
					   STMPE1600_PIN_ALL);
		stmpe1600_Exit(STMPE1600_I2C_ADDR);
	} else if (!tca9535_init(TCA9535_I2C_ADDR) &&
		   tca9535_io_init_pin(TCA9535_I2C_ADDR, TCA9535_PIN_ALL,
				       TCA9535_DIRECTION_IN) >= 0) {
		val = tca9535_io_read_pin(TCA9535_I2C_ADDR,
					  TCA9535_PIN_ALL);
		tca9535_exit(TCA9535_I2C_ADDR);
	}

	shared_data_bi.board_extensions = val & 0x000F;

	if (get_soc_id() == SOCID_STA1385) {
		board_id = BOARD_TC3P_MTP; /* TC3P MTP by default */
		switch ((val & 0x00F0) >> 4) {
		case 4:
			board_id = BOARD_TC3P_CARRIER;
			break;
		case 8:
			/* revA */
			break;
		case 9:
			board_rev_id = 1; /* revB */
			break;
		case 10:
			board_rev_id = 2; /* revC */
			break;
		case 11:
			board_rev_id = 3; /* revD */
			break;
		case 12:
			board_rev_id = 4; /* revE */
			break;
		default:
			board_rev_id = 3; /* default revD */
			break;
		}
	} else {
		/* Accordo5/TC3 EVB boards */
		board_id = BOARD_A5_EVB; /* A5 EVB by default */
		switch ((val & 0x00F0) >> 4) {
		case 0:
			board_rev_id = 0; /* revA */
			break;
		case 1:
			board_rev_id = 1; /* revB */
			break;
		case 3:
			board_id = BOARD_TC3_EVB;
			board_rev_id = 1; /* revB (revA doesn't exist) */
			break;
		case 4:
			board_rev_id = 2; /* revC */
			break;
		case 5:
			board_rev_id = 3; /* revD */
			break;
		case 6:
			board_id = BOARD_TC3_EVB;
			board_rev_id = 2; /* revC */
			break;
		case 7:
			board_id = BOARD_TC3_EVB;
			board_rev_id = 3; /* revD */
			break;
		case 8:
			board_rev_id = 4; /* revE */
			break;
		case 9:
			board_id = BOARD_TC3_EVB;
			board_rev_id = 4; /* revE */
			break;
		default:
			board_rev_id = 0; /* default revA */
			break;
		}
	}
	shared_data_bi.board_id = board_id;
	shared_data_bi.board_rev_id = board_rev_id;

	return 0;
}

/**
 * @brief	Recopy M3 (ESRAM) boot info shared data in DDR for AP usage
 * @return	NA
 */
void platform_recopy_bootinfo_in_ddr(bool bi_only)
{
	shared_data.bi = shared_data_bi;
	if (!bi_only)
		memset(&shared_data.rw, 0, sizeof(shared_data.rw));
}

/**
 * @brief	Init trace on serial link and displays welcome banner
 * @param	context: current application context
 * @return	NA
 */
void platform_welcome_message(struct sta *context)
{
	/* Print a title as soon as debug is available */
	TRACE_NOTICE("\n%s %s %s\n", PLATFORM_NAME, context->bin, VERSION);
	if (!strcmp(context->bin, BIN_NAME_XL))
		TRACE_NOTICE("Board ID: %d rev%c ext %d, SoC ID: %d, Cut: %X, Rom version: %02X\n\n",
			     get_board_id(), 'A' + get_board_rev_id(),
			     get_board_extensions(), get_soc_id(),
			     get_cut_rev(), get_erom_version());
#if TRACE_LEVEL >= TRACE_LEVEL_INFO
	{
		unsigned int i;

		/* Display OTP registers */
		for (i = 0; i < NELEMS(shared_data_bi.otp_regs); i++)
			TRACE_NOTICE("OTP_VR%d_REG: 0x%08X\n", i,
				     get_otp_regs()[i]);
	}
#endif
}

int m3_get_mxtal(void)
{
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1275:
	case SOCID_STA1195:
		return (src_m3_regs->resstat.bit_sta1x95.mxtal_fre_sel ?
			   26000000 : 24000000);

	case SOCID_STA1385:
		switch (src_m3_regs->resstat.bit_sta1385.mxtal_fre_sel & 0x3) {
		case 0:
			return 24000000;
		case 1:
			return 26000000;
		case 2:
			return 40000000;
		default:
			return -EINVAL;
		}

	default:
		return -EINVAL;
	}
}

/**
 * @brief  enables used clocks. Actually parses the list of available clocks
 * and enable them if they are used. This allows next boot stages (Linux) to
 * not take care about the unused clocks management.
 *
 * @retval 0 if no error, not 0 otherwise
 */
int sta_enable_clocks(t_src_m3 *src_m3, t_src_a7 *src_a7)
{
	int ret;
	unsigned int i;
	unsigned int nelems_m, nelems_a;
	const struct sta_clk *m, *a;

	switch (get_soc_id()) {
	case SOCID_STA1195:
	case SOCID_STA1295:
	case SOCID_STA1275:
		m = sta1x95_clk_m3;
		nelems_m = NELEMS(sta1x95_clk_m3);
		a = sta1x95_clk_ap;
		nelems_a = NELEMS(sta1x95_clk_ap);
		break;
	case SOCID_STA1385:
		m = sta1385_clk_m3;
		nelems_m = NELEMS(sta1385_clk_m3);
		a = sta1385_clk_ap;
		nelems_a = NELEMS(sta1385_clk_ap);
		break;
	default:
		return -ENODEV;
	}

	for (i = 0; i < nelems_m; i++) {
		ret = srcm3_pclk_change_state(src_m3, m[i].id, m[i].state);
		if (ret)
			goto end;
	}

	for (i = 0; i < nelems_a; i++) {
		ret = srca7_pclk_change_state(src_a7, a[i].id, a[i].state);
		if (ret)
			goto end;
	}

end:
	return ret;
}

/**
 * @brief  Reset ETH0 PHY for next boot stages
 *         to be called only in Xloaders
 *
 * @retval None
 */
void platform_eth0_reset_phy(void)
{
	struct gpio_config pin;
	uint8_t eth0_rstpin;

	if (get_soc_id() == SOCID_STA1385) {
		if  (get_cut_rev() == CUT_10)
			eth0_rstpin = M3_GPIO(14);
		else
			eth0_rstpin = M3_GPIO(11);
	} else {
		eth0_rstpin = A7_GPIO(35);
	}

	pin.direction   = GPIO_DIR_OUTPUT;
	pin.mode        = GPIO_MODE_SOFTWARE;
	pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
	pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
	gpio_set_pin_config(eth0_rstpin, &pin);

	gpio_set_gpio_pin(eth0_rstpin);
	udelay(2);
	gpio_clear_gpio_pin(eth0_rstpin);
	udelay(10);
	gpio_set_gpio_pin(eth0_rstpin);
	udelay(10);
}
