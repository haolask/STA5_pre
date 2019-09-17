/**
 * @file sta1385_ddr3_ctl.c
 * @brief This is the driver for the Synopsys universal DDR Memory Controller
 * (uMCTL2)
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"
#include "trace.h"
#include "sta_common.h"

#include "sta_mtu.h"
#include "sta_src.h"
#include "sta_crc.h"
#include "sta_ddr.h"
#include "sta1385_ddr3_setting.h"

/******************************************************************************
 * Default DDR Config for SOC STA1385
 * 1 DDR micron MT41K256M16 (so 16 bits width) ==>  512MB
 *		256 Meg x 16
 *		32 Meg x 16 x 8 banks
 *		Row Address 32K(R[14:0])
 *		Bank  8 (BA[2:0])
 *		Column 1K (A[9:0])
 *		Page Size 2KB
 *
 *
 * Alternate 1 : CFG_DDR_ALT1
 * 1 DDR ISSI 43-46TR16128C-82560CL ==> 256MB
 *	       128 Meg x 16
 *	       Row Address	16K(R[13:0])
 *	       Bank  8 (BA[2:0])
 *	       Column 1K (A[9:0])
 *	       Page Size 2KB
 *
 ******************************************************************************/

static int sta1385_ddr3_ctl_enable_self_refresh(void)
{
	uint32_t pwrctl;
	uint32_t stat;
	uint8_t mode;
	uint8_t type;

	stat = ddr_read_reg(DDR3_CTRL_BASE + STAT);
	mode = (stat & STAT_OPMODE_MASK);

	/* Cannot enter self-refresh if the mode is not "Normal" */
	if (mode != STAT_OPMODE_NORMAL)
		return -EPERM;

	pwrctl = ddr_read_reg(DDR3_CTRL_BASE + PWRCTL);
	pwrctl &= ~BIT(5);
	pwrctl |= BIT(5);
	ddr_write_reg(pwrctl, DDR3_CTRL_BASE + PWRCTL);

	/* now poll for the state */
	do {
		stat = ddr_read_reg(DDR3_CTRL_BASE + STAT);
		mode = (stat & STAT_OPMODE_MASK);
	} while(mode != STAT_OPMODE_SELFREF);

	type = (stat >> STAT_SELFREF_TYPE_OFFSET) & STAT_SELFREF_TYPE_MASK;
	(void) type;

	return 0;
}

static int sta1385_ddr3_ctl_disable_all_ports(void)
{
	disable_ddr_port(0);
	disable_ddr_port(1);
	disable_ddr_port(2);
	disable_ddr_port(3);

	poll_reg(DDR3_CTRL_BASE + PSTAT, DDR3_ALL_PORT_DISABLED_MASK);

	return 0;
}

static void sta1385_ddr3_ctl_enable_all_ports(void)
{
	enable_ddr_port(0);
	enable_ddr_port(1);
	enable_ddr_port(2);
	enable_ddr_port(3);
}

static int sta1385_ddr3_ctl_disable_self_refresh(void)
{
	uint32_t pwrctl;
	uint32_t stat;
	uint8_t mode;

	stat = ddr_read_reg(DDR3_CTRL_BASE + STAT);
	mode = (stat & STAT_OPMODE_MASK);

	/* Cannot go back to normal mode if the mode is not "self-refresh" */
	if (mode != STAT_OPMODE_SELFREF)
		return -EPERM;

	pwrctl = ddr_read_reg(DDR3_CTRL_BASE + PWRCTL);
	pwrctl &= ~BIT(5);
	ddr_write_reg(pwrctl, DDR3_CTRL_BASE + PWRCTL);

	/* now poll for the state */
	do {
		stat = ddr_read_reg(DDR3_CTRL_BASE + STAT);
		mode = (stat & STAT_OPMODE_MASK);
	} while(mode != STAT_OPMODE_NORMAL);

	return 0;
}

int sta_ddr_suspend(void)
{
	int ret;

#ifdef _SELF_REF_TEST_DDR_CHECKSUM
	ddr_write_reg(compute_crc32(0, (uint8_t *)DDR_CHECKSUM_AREA_START,
				    DDR_CHECKSUM_AREA_SIZE), CHECKSUM_STORAGE);
#endif

	ddr3_pub_save_zq_cal();

	/* PUBL STA1385 overwrittes 32Bytes in DDR
	 * for training during resume action
	 */
	ddr3_save_data_written_by_training();

	/* Block AXI ports from taking transactions */
	sta1385_ddr3_ctl_disable_all_ports();

	ret = sta1385_ddr3_ctl_enable_self_refresh();
	if (ret)
		return ret;

	/*
	 * Place IOs in retention mode
	 * Note: on sta1x95 SoCs, CKE and RESETn lines are not connected directly to
	 * the DRAM.
	 *	- CKE is driven low by a resistor bridge
	 *	- RESETn is connected to a GPIO (M3_GPIO_10) and is managed
	 */
	sta_ddr_pub_set_io_retention(true);

	return ret;
}

#ifdef _SELF_REF_TEST_DDR_CHECKSUM
void sta_ddr_std_check(bool resume_stage)
{
	int reg;

	if (!resume_stage) {
		write_reg(compute_crc32(0, (uint8_t *)DDRAM_ATF_BL32_SAVED_BASE,
					DDRAM_ATF_BL32_SAVED_SIZE), CHECKSUM_STORAGE);
		TRACE_NOTICE("Test DDR \tArea : 0x%x++0x%x\n\t\tComputed Checksum:\t0x%x\n",
			     DDRAM_ATF_BL32_SAVED_BASE, DDRAM_ATF_BL32_SAVED_SIZE,
			     read_reg(CHECKSUM_STORAGE));
	} else {
		reg = read_reg(CHECKSUM_STORAGE);
		TRACE_NOTICE("Test DDR \tArea : 0x%x++0x%x\n\t\tCurrent Checksum:\t0x%x\n\t\tPrevious Checksum:\t0x%x\n",
			     DDRAM_ATF_BL32_SAVED_BASE, DDRAM_ATF_BL32_SAVED_SIZE,
			     compute_crc32(0, (uint8_t *)DDRAM_ATF_BL32_SAVED_BASE,
					   DDRAM_ATF_BL32_SAVED_SIZE), reg);
	}
}
#endif

int sta_ddr_resume(void)
{
#ifdef _SELF_REF_TEST_DDR_CHECKSUM
	uint32_t reg;
#endif
	sta_ddr_pub_set_io_retention(false);

	sta1385_ddr3_ctl_disable_self_refresh();
	udelay(10);
	sta_ddr_pub_do_training(PUB_FULL_TRAINING);

	sta1385_ddr3_ctl_enable_all_ports();

	udelay(1000);

	ddr3_load_data_written_by_training();

#ifdef _SELF_REF_TEST_DDR_CHECKSUM
	reg = read_reg(CHECKSUM_STORAGE);
	TRACE_NOTICE("Test DDR \tArea : 0x%x++0x%x\n\t\tCurrent Checksum:\t0x%x\n\t\tPrevious Checksum:\t0x%x\n",
		     DDR_CHECKSUM_AREA_START, DDR_CHECKSUM_AREA_SIZE,
		    compute_crc32(0, (uint8_t *)DDR_CHECKSUM_AREA_START,
				  DDR_CHECKSUM_AREA_SIZE), reg);
#endif

	return 0;
}

/**
 */
int sta_ddr_ctl_configure(uint32_t boot_reason)
{
	uint32_t reg;


	/*  1.  Preform DDR init sequence
		; 1.1 DDR3_ctrl_programming */
	ddr_write_reg(0x00000001, DDRADDR(DBG1));
	if (boot_reason == DDR_BOOT_REASON_COLD_BOOT)
		ddr_write_reg(0x00000001, DDRADDR(PWRCTL));

	ddr_write_reg(DDRVAL(MSTR), DDRADDR(MSTR));
	ddr_write_reg(DDRVAL(MRCTRL0), DDRADDR(MRCTRL0));
	ddr_write_reg(DDRVAL(MRCTRL0), DDRADDR(MRCTRL0));
	ddr_write_reg(DDRVAL(MRCTRL1), DDRADDR(MRCTRL1));
	ddr_write_reg(DDRVAL(PWRTMG), DDRADDR(PWRTMG));
	ddr_write_reg(DDRVAL(HWLPCTL), DDRADDR(HWLPCTL));
	ddr_write_reg(DDRVAL(RFSHCTL0), DDRADDR(RFSHCTL0));
	ddr_write_reg(DDRVAL(RFSHTMG), DDRADDR(RFSHTMG));

	reg = 0x00070003;
	if (boot_reason == DDR_BOOT_REASON_COLD_BOOT)
		reg |= SKIP_DRAM_INIT_NORMAL;
	else
		reg |= SKIP_DRAM_INIT_SELFREF;
	ddr_write_reg(reg, DDRADDR(INIT0));

	if (boot_reason == DDR_BOOT_REASON_EXIT_SELF_REF)
		sta1385_ddr3_ctl_enable_self_refresh();

	ddr_write_reg(DDRVAL(INIT1), DDRADDR(INIT1));
	ddr_write_reg(DDRVAL(INIT3), DDRADDR(INIT3));
	ddr_write_reg(DDRVAL(INIT4), DDRADDR(INIT4));
	ddr_write_reg(DDRVAL(INIT5), DDRADDR(INIT5));

	ddr_write_reg(DDRVAL(DRAMTMG0), DDRADDR(DRAMTMG0));
	ddr_write_reg(DDRVAL(DRAMTMG1), DDRADDR(DRAMTMG1));
	ddr_write_reg(DDRVAL(DRAMTMG2), DDRADDR(DRAMTMG2));
	ddr_write_reg(DDRVAL(DRAMTMG3), DDRADDR(DRAMTMG3));
	ddr_write_reg(DDRVAL(DRAMTMG4), DDRADDR(DRAMTMG4));
	ddr_write_reg(DDRVAL(DRAMTMG5), DDRADDR(DRAMTMG5));
	ddr_write_reg(DDRVAL(DRAMTMG8), DDRADDR(DRAMTMG8));

	ddr_write_reg(DDRVAL(ZQCTL0), DDRADDR(ZQCTL0));
	/* ddr_write_reg(DDRVAL(ZQCTL1), DDRADDR(ZQCTL1));
	 * ddr_write_reg(DDRVAL(ZQCTL2), DDRADDR(ZQCTL2));
	 */
	ddr_write_reg(DDRVAL(DFITMG0), DDRADDR(DFITMG0));
	ddr_write_reg(DDRVAL(DFITMG1), DDRADDR(DFITMG1));
	/* ddr_write_reg(DDRVAL(DFILPCFG0), DDRADDR(DFILPCFG0)); */
	ddr_write_reg(DDRVAL(DFIUPD0), DDRADDR(DFIUPD0));
	ddr_write_reg(DDRVAL(DFIUPD1), DDRADDR(DFIUPD1));
	ddr_write_reg(DDRVAL(DFIUPD2), DDRADDR(DFIUPD2));
	if (boot_reason == DDR_BOOT_REASON_COLD_BOOT)
		ddr_write_reg(DDRVAL(DFIMISC) | BIT(0), DDRADDR(DFIMISC));

	/* 1.2 SYNOPSYS address mapping (2Gb) */
	if (get_board_id() == BOARD_CR2_VAB) {
		ddr_write_reg(0x00161616, DDR3_CTRL_BASE + ADDRMAP(1));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(2));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(3));
		ddr_write_reg(0x00000f0f, DDR3_CTRL_BASE + ADDRMAP(4));
		ddr_write_reg(0x04040404, DDR3_CTRL_BASE + ADDRMAP(5));
		ddr_write_reg(0x0f0f0404, DDR3_CTRL_BASE + ADDRMAP(6));
		ddr_write_reg(0x00000008, DDR3_CTRL_BASE + ADDRMAP(9));
		ddr_write_reg(0x00000308, DDR3_CTRL_BASE + ADDRMAP(9));
		ddr_write_reg(0x00090308, DDR3_CTRL_BASE + ADDRMAP(9));
		ddr_write_reg(0x0b090308, DDR3_CTRL_BASE + ADDRMAP(9));
		ddr_write_reg(0x00000008, DDR3_CTRL_BASE + ADDRMAP(10));
		ddr_write_reg(0x00000a08, DDR3_CTRL_BASE + ADDRMAP(10));
		ddr_write_reg(0x00070a08, DDR3_CTRL_BASE + ADDRMAP(10));
		ddr_write_reg(0x09070a08, DDR3_CTRL_BASE + ADDRMAP(10));
		ddr_write_reg(0x0000000a, DDR3_CTRL_BASE + ADDRMAP(11));
	} else {
		ddr_write_reg(0x00080808, DDR3_CTRL_BASE + ADDRMAP(1));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(2));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(3));
		ddr_write_reg(0x00000f0f, DDR3_CTRL_BASE + ADDRMAP(4));
		ddr_write_reg(0x07070707, DDR3_CTRL_BASE + ADDRMAP(5));
#if defined(CFG_DDR_ALT1)
		ddr_write_reg(0x0f0f0707, DDR3_CTRL_BASE + ADDRMAP(6));
#else
		ddr_write_reg(0x0f070707, DDR3_CTRL_BASE + ADDRMAP(6));
#endif
	}

	ddr_write_reg(DDRVAL(ODTCFG), DDRADDR(ODTCFG));
	ddr_write_reg(DDRVAL(ODTMAP), DDRADDR(ODTMAP));
	ddr_write_reg(DDRVAL(SCHED), DDRADDR(SCHED));
	ddr_write_reg(DDRVAL(SCHED1), DDRADDR(SCHED1));
	ddr_write_reg(DDRVAL(PERFHPR1), DDRADDR(PERFHPR1));
	ddr_write_reg(DDRVAL(PERFLPR1), DDRADDR(PERFLPR1));
	ddr_write_reg(DDRVAL(PERFWR1), DDRADDR(PERFWR1));
	if (get_soc_id() == SOCID_STA1385 &&
	    get_cut_rev() < CUT_20) {
		ddr_write_reg(0x0000006f, DDR3_CTRL_BASE + PERFVPR1);
	}
	ddr_write_reg(DDRVAL(DBG0), DDRADDR(DBG0));
	ddr_write_reg(DDRVAL(DBG1), DDRADDR(DBG1));
	ddr_write_reg(DDRVAL(DBGCMD), DDRADDR(DBGCMD));
	ddr_write_reg(DDRVAL(POISONCFG), DDRADDR(POISONCFG));

	disable_ddr_port(0);
	disable_ddr_port(1);
	disable_ddr_port(2);
	disable_ddr_port(3);

	ddr_write_reg(DDRVAL(DBG1), DDRADDR(DBG1));
	ddr_write_reg(DDRVAL(CRCPARSTAT), DDRADDR(CRCPARSTAT));
	ddr_write_reg(DDRVAL(DFIMISC), DDRADDR(DFIMISC));

	return 0;
}

/**
 */
int sta_ddr_ctl_power_up(uint32_t boot_reason)
{
	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + SWCTL);
	ddr_write_reg(0x00000001, DDR3_CTRL_BASE + DFIMISC);
	ddr_write_reg(0x00000001, DDR3_CTRL_BASE + SWCTL);
	udelay(100);

	if (boot_reason == DDR_BOOT_REASON_EXIT_SELF_REF) {
		sta_ddr_resume();
	} else {
		/* strange! Autoref disable!!! */
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + RFSHCTL3);
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + PWRCTL);
		ddr_write_reg(0x13842214, 0x50900008);
		ddr_write_reg(0x000001fd, 0x50900004);
		udelay(100);

		/* Allow AXI ports to take transactions */
		sta1385_ddr3_ctl_enable_all_ports();

		udelay(1000);
	}

#ifdef VERBOSE_DDR
	sta_ddr_dump();
#endif

	return 0;
}

