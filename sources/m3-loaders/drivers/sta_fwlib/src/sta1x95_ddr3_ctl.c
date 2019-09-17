/**
 * @file sta1x95_ddr3_ctl.c
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

#include "sta_mtu.h"
#include "sta_gpio.h"
#include "sta_src.h"
#include "sta_common.h"
#include "sta_crc.h"
#include "sta_ddr.h"
#include "sta1x95_ddr3_setting.h"

#include "sta_pinmux.h"

#define SWCTL_SWDONE BIT(0)
#define DFI_CTL_IDLE_EN BIT(4)
#define DFI_INIT_COMPL_EN BIT(0)

/******************************************************************************
 * Default DDR Config for SOC STA1x95
 * 2* DDR micron MT41K256M16 (so 32bits width) but defined as 2 MT41K128M16 ==>  512MB
 *		256 Meg x 16
 *		32 Meg x 16 x 8 banks
 *		Row Address 32K(R[14:0])   ==> defined as Row Address 16K(R[13:0])
 *		Bank  8 (BA[2:0])
 *		Column 1K (A[9:0])
 *		Page Size 2KB
 *
 *
 * Alternate 1 : CFG_DDR_ALT1
 * 1 DDR micron MT41K256M16 (so 16bits width) 512MB
 *	       256 Meg x 16
 *	       32 Meg x 16 x 8 banks
 *	       Row Address	32K(R[14:0])
 *	       Bank  8 (BA[2:0])
 *	       Column 1K (A[9:0])
 *	       Page Size 2KB
 *
 * Alternate 2 : CFG_DDR_ALT2
 * 1 DDR ISSI IS46TR16640BL (so 16bits width) 128MB
 *	       64 Meg x 16
 *	       8 Meg x 16 x 8 banks
 *	       Row Address	32K(R[12:0])
 *	       Bank  8 (BA[2:0])
 *	       Column 1K (A[9:0])
 *	       Page Size 2KB
 *
 * Alternate 3 : CFG_DDR_ALT3
 * 2* DDR micron MT41K256M16 (so 32bits width)  1GB
 *		256 Meg x 16
 *		32 Meg x 16 x 8 banks
 *		Row Address	 32K(R[14:0])
 *		Bank  8 (BA[2:0])
 *		Column 1K (A[9:0])
 *		Page Size 2KB
 *****************************************************************************/

static void sta1x95_ddr3_ctl_enable_all_ports(void)
{
	enable_ddr_port(0);
	enable_ddr_port(1);
	enable_ddr_port(2);
	enable_ddr_port(3);
}

static int sta1x95_ddr3_ctl_disable_all_ports(void)
{
	disable_ddr_port(0);
	disable_ddr_port(1);
	disable_ddr_port(2);
	disable_ddr_port(3);

	poll_reg(DDR3_CTRL_BASE + PSTAT, DDR3_ALL_PORT_DISABLED_MASK);

	return 0;
}

static int sta1x95_ddr3_ctl_enable_self_refresh(void)
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

static int sta1x95_ddr3_ctl_disable_self_refresh(void)
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

	/* PUB STA1x95 overwrittes 128Bytes in DDR
	 * for training during resume action
	 */
	ddr3_save_data_written_by_training();

	/* Block AXI ports from taking transactions */
	sta1x95_ddr3_ctl_disable_all_ports();

	ret = sta1x95_ddr3_ctl_enable_self_refresh();
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

int sta_ddr_resume(void)
{
#ifdef _SELF_REF_TEST_DDR_CHECKSUM
	uint32_t reg;
#endif

	sta_ddr_pub_set_io_retention(false);

	sta1x95_ddr3_ctl_disable_self_refresh();
	udelay(10);
	sta_ddr_pub_do_training(PUB_FULL_TRAINING);

	/* Allow AXI ports to take transactions */
	sta1x95_ddr3_ctl_enable_all_ports();

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

int sta_ddr_ctl_configure(uint32_t boot_reason)
{
	uint32_t reg;

#if defined(CFG_DDR_ALT1) || defined(CFG_DDR_ALT2)
		ddr_write_reg(DDRVAL(MSTR) | BIT(12), DDRADDR(MSTR));
#else
		ddr_write_reg(DDRVAL(MSTR), DDRADDR(MSTR));
#endif

	ddr_write_reg(DDRVAL(MRCTRL0), DDRADDR(MRCTRL0));
	ddr_write_reg(DDRVAL(MRCTRL1), DDRADDR(MRCTRL1));

	reg = 0x00070003;
	if (boot_reason == DDR_BOOT_REASON_COLD_BOOT)
		reg |= SKIP_DRAM_INIT_NORMAL;
	else
		reg |= SKIP_DRAM_INIT_SELFREF;
	ddr_write_reg(reg, DDRADDR(INIT0));

	ddr_write_reg(DDRVAL(INIT1), DDRADDR(INIT1));
	ddr_write_reg(DDRVAL(INIT3), DDRADDR(INIT3));
	ddr_write_reg(DDRVAL(INIT4), DDRADDR(INIT4));
	ddr_write_reg(DDRVAL(INIT5), DDRADDR(INIT5));

	/* Timing management */
	ddr_write_reg(DDRVAL(DRAMTMG0), DDRADDR(DRAMTMG0));
	ddr_write_reg(DDRVAL(DRAMTMG1), DDRADDR(DRAMTMG1));
	ddr_write_reg(DDRVAL(DRAMTMG2), DDRADDR(DRAMTMG2));
	ddr_write_reg(DDRVAL(DRAMTMG3), DDRADDR(DRAMTMG3));
	ddr_write_reg(DDRVAL(DRAMTMG4), DDRADDR(DRAMTMG4));
	ddr_write_reg(DDRVAL(DRAMTMG5), DDRADDR(DRAMTMG5));
	ddr_write_reg(DDRVAL(DRAMTMG8), DDRADDR(DRAMTMG8));

	/* ZQ Calibration management */
	ddr_write_reg(DDRVAL(ZQCTL0), DDRADDR(ZQCTL0));
	ddr_write_reg(DDRVAL(ZQCTL1), DDRADDR(ZQCTL1));

	/* DFI specific */
	/* Timing management */
	ddr_write_reg(DDRVAL(DFITMG0), DDRADDR(DFITMG0));
	ddr_write_reg(DDRVAL(DFITMG1), DDRADDR(DFITMG1));

	/* LP config */
	ddr_write_reg(DDRVAL(DFILPCFG0), DDRADDR(DFILPCFG0));
	/* Update registers */
	ddr_write_reg(DDRVAL(DFIUPD0), DDRADDR(DFIUPD0));
	ddr_write_reg(DDRVAL(DFIUPD1), DDRADDR(DFIUPD1));
	ddr_write_reg(DDRVAL(DFIUPD2), DDRADDR(DFIUPD2));
	/* Misc */
	ddr_write_reg(DFI_CTL_IDLE_EN | DFI_INIT_COMPL_EN, DDRADDR(DFIMISC));
	/* Address map specific */
#if defined(CFG_DDR_ALT1) || defined(CFG_DDR_ALT2)
		ddr_write_reg(0x00000016, DDR3_CTRL_BASE + ADDRMAP(0));
		ddr_write_reg(0x00070707, DDR3_CTRL_BASE + ADDRMAP(1));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(2));
		ddr_write_reg(0x0F000000, DDR3_CTRL_BASE + ADDRMAP(3));
		ddr_write_reg(0x00000F0F, DDR3_CTRL_BASE + ADDRMAP(4));
		ddr_write_reg(0x06060606, DDR3_CTRL_BASE + ADDRMAP(5));
#if defined(CFG_DDR_ALT2)
		ddr_write_reg(0x0F0F0F06, DDR3_CTRL_BASE + ADDRMAP(6));
#else
		ddr_write_reg(0x0F060606, DDR3_CTRL_BASE + ADDRMAP(6));
#endif
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(9));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(10));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(11));
#else
		ddr_write_reg(0x0000001F, DDR3_CTRL_BASE + ADDRMAP(0));
		ddr_write_reg(0x00080808, DDR3_CTRL_BASE + ADDRMAP(1));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(2));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(3));
		ddr_write_reg(0x00000F0F, DDR3_CTRL_BASE + ADDRMAP(4));
		ddr_write_reg(0x07070707, DDR3_CTRL_BASE + ADDRMAP(5));
#if defined(CFG_DDR_ALT3)
		ddr_write_reg(0x0F070707, DDR3_CTRL_BASE + ADDRMAP(6));
#else
		ddr_write_reg(0x0F0F0707, DDR3_CTRL_BASE + ADDRMAP(6));
#endif
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(9));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(10));
		ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(11));
#endif

	/* ODT specific */
	ddr_write_reg(DDRVAL(ODTCFG), DDRADDR(ODTCFG));
	ddr_write_reg(DDRVAL(ODTMAP), DDRADDR(ODTMAP));
	/* Scheduler control */
	ddr_write_reg(DDRVAL(SCHED), DDRADDR(SCHED));
	ddr_write_reg(DDRVAL(SCHED1), DDRADDR(SCHED1));
	/* Priority CAM registers */
	ddr_write_reg(DDRVAL(PERFHPR1), DDRADDR(PERFHPR1));
	ddr_write_reg(DDRVAL(PERFLPR1), DDRADDR(PERFLPR1));
	ddr_write_reg(DDRVAL(PERFWR1), DDRADDR(PERFWR1));
	ddr_write_reg(DDRVAL(PERFVPR1), DDRADDR(PERFVPR1));
	ddr_write_reg(DDRVAL(PERFVPW1), DDRADDR(PERFVPW1));

	ddr_write_reg(DDRVAL(DBG0), DDRADDR(DBG0));
	ddr_write_reg(DDRVAL(DBG1), DDRADDR(DBG1));
	ddr_write_reg(DDRVAL(DBGCMD), DDRADDR(DBGCMD));

	/* enable quasi-dynamic register programming outside reset */
	ddr_write_reg(SWCTL_SWDONE, DDRADDR(SWCTL));
	ddr_write_reg(DDRVAL(POISONCFG), DDRADDR(POISONCFG));

	disable_ddr_port(0);
	disable_ddr_port(1);
	disable_ddr_port(2);
	disable_ddr_port(3);

	/* uMCTL2_MP (Multi-port) registers */
	ddr_write_reg(DDRVAL(PCCFG), DDRADDR(PCCFG));
	/* Port 0 */
	ddr_write_reg(DDRVAL(PCFGR_0), DDRADDR(PCFGR_0));
	ddr_write_reg(DDRVAL(PCFGW_0), DDRADDR(PCFGW_0));
	ddr_write_reg(DDRVAL(PCFGQOS0_0), DDRADDR(PCFGQOS0_0));
	ddr_write_reg(DDRVAL(PCFGQOS1_0), DDRADDR(PCFGQOS1_0));
	ddr_write_reg(DDRVAL(PCFGWQOS0_0), DDRADDR(PCFGWQOS0_0));
	ddr_write_reg(DDRVAL(PCFGWQOS1_0), DDRADDR(PCFGWQOS1_0));
	/* Port 1 */
	ddr_write_reg(DDRVAL(PCFGR_1), DDRADDR(PCFGR_1));
	ddr_write_reg(DDRVAL(PCFGW_1), DDRADDR(PCFGW_1));
	ddr_write_reg(DDRVAL(PCFGQOS0_1), DDRADDR(PCFGQOS0_1));
	ddr_write_reg(DDRVAL(PCFGQOS1_1), DDRADDR(PCFGQOS1_1));
	ddr_write_reg(DDRVAL(PCFGWQOS0_1), DDRADDR(PCFGWQOS0_1));
	ddr_write_reg(DDRVAL(PCFGWQOS1_1), DDRADDR(PCFGWQOS1_1));
	/* Port 2 */
	ddr_write_reg(DDRVAL(PCFGR_2), DDRADDR(PCFGR_2));
	ddr_write_reg(DDRVAL(PCFGW_2), DDRADDR(PCFGW_2));
	ddr_write_reg(DDRVAL(PCFGQOS0_2), DDRADDR(PCFGQOS0_2));
	ddr_write_reg(DDRVAL(PCFGQOS1_2), DDRADDR(PCFGQOS1_2));
	ddr_write_reg(DDRVAL(PCFGWQOS0_2), DDRADDR(PCFGWQOS0_2));
	ddr_write_reg(DDRVAL(PCFGWQOS1_2), DDRADDR(PCFGWQOS1_2));
	/* Port 3 */
	ddr_write_reg(DDRVAL(PCFGR_3), DDRADDR(PCFGR_3));
	ddr_write_reg(DDRVAL(PCFGW_3), DDRADDR(PCFGW_3));
	ddr_write_reg(DDRVAL(PCFGQOS0_3), DDRADDR(PCFGQOS0_3));
	ddr_write_reg(DDRVAL(PCFGQOS1_3), DDRADDR(PCFGQOS1_3));
	ddr_write_reg(DDRVAL(PCFGWQOS0_3), DDRADDR(PCFGWQOS0_3));
	ddr_write_reg(DDRVAL(PCFGWQOS1_3), DDRADDR(PCFGWQOS1_3));

	ddr_write_reg(DDRVAL(DBG1), DDRADDR(DBG1));

	if (boot_reason == DDR_BOOT_REASON_EXIT_SELF_REF)
		sta1x95_ddr3_ctl_enable_self_refresh();

	/* disable quasi-dynamic register programming outside reset */
	ddr_write_reg(~SWCTL_SWDONE, DDRADDR(SWCTL));
	ddr_write_reg(DFI_CTL_IDLE_EN, DDRADDR(DFIMISC));

	return 0;
}

int sta_ddr_ctl_power_up(uint32_t boot_reason)
{
	uint32_t reg;

	/* dfi_init_complete_en to trigger SDRAM initialization */
	ddr_write_reg(DFI_CTL_IDLE_EN | DFI_INIT_COMPL_EN,
		      DDR3_CTRL_BASE + DFIMISC);

	if (boot_reason == DDR_BOOT_REASON_EXIT_SELF_REF) {
		sta_ddr_resume();
	} else {
		/* disable quasi-dynamic register programming outside reset */
		ddr_write_reg(SWCTL_SWDONE, DDR3_CTRL_BASE + SWCTL);

		do {
			reg = ddr_read_reg(DDR3_CTRL_BASE + SWSTAT);
		} while ((reg & SWCTL_SWDONE) != SWCTL_SWDONE);

		sta1x95_ddr3_ctl_enable_all_ports();
	}

#ifdef VERBOSE_DDR
	sta_ddr_dump();
#endif

	return 0;
}
