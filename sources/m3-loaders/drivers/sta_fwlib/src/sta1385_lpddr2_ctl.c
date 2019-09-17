/**
 * @file sta1385_lpddr2_ctl.c
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
#include "sta_ddr.h"
#include "sta1385_lpddr2_setting.h"

#define SWCTL_SWDONE BIT(0)

/******************************************************************************
 * Default DDR Config for SOC STA1x95
 * 1 DDR ISSI IS46LD16128B-18BLA2 ===> 256MB
 *		128 Meg x 16
 *		8 Meg x 16 x 8 banks
 *		Row Address 16K(R[13:0])
 *		Bank  8 (BA[2:0])
 *		Column 1K (A[9:0])
 *		Page Size 2KB
 *
 * Alternate 1 : CFG_DDR_ALT1
 * 1 DDR micron B1316BDBH-1DIT-F ==>  128MB
 *		64 Meg x 16
 *		8 Meg x 16 x 8 banks
 *		Row Address 8K(R[12:0])
 *		Bank  8 (BA[2:0])
 *		Column 1K (A[9:0])
 *		Page Size 2KB
 *
 *
 * Alternate 2 : CFG_DDR_ALT2
 *
 * Alternate 3 : CFG_DDR_ALT3
 *
 *****************************************************************************/

static int sta1385_lpddr2_ctl_enable_self_refresh(void)
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
	} while (mode != STAT_OPMODE_SELFREF);

	type = (stat >> STAT_SELFREF_TYPE_OFFSET) & STAT_SELFREF_TYPE_MASK;
	(void)type;

	return 0;
}

static int sta1385_lpddr2_ctl_disable_self_refresh(void)
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
	} while (mode != STAT_OPMODE_NORMAL);

	return 0;
}

static int sta1385_lpddr2_ctl_disable_all_ports(void)
{
	disable_ddr_port(0);
	disable_ddr_port(1);
	disable_ddr_port(2);
	disable_ddr_port(3);

	poll_reg(DDR3_CTRL_BASE + PSTAT, DDR3_ALL_PORT_DISABLED_MASK);

	return 0;
}

static void sta1385_lpddr2_ctl_enable_all_ports(void)
{
	enable_ddr_port(0);
	enable_ddr_port(1);
	enable_ddr_port(2);
	enable_ddr_port(3);
}

int sta_ddr_suspend(void)
{
	int ret;

#ifdef _SELF_REF_TEST_DDR_CHECKSUM
	ddr_write_reg(_LZG_CalcChecksum((const unsigned char *)DDR_CHECKSUM_AREA_START,
					DDR_CHECKSUM_AREA_SIZE), CHECKSUM_STORAGE);
#endif

	ddr3_pub_save_zq_cal();

	/* PUBL STA1385 overwrittes 32Bytes in DDR
	 * for training during resume action
	 */
	ddr3_save_data_written_by_training();

	/* Block AXI ports from taking transactions */
	sta1385_lpddr2_ctl_disable_all_ports();

	ret = sta1385_lpddr2_ctl_enable_self_refresh();
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

	sta1385_lpddr2_ctl_disable_self_refresh();

	/* Allow AXI ports to take transactions */
	sta1385_lpddr2_ctl_enable_all_ports();

	ddr3_load_data_written_by_training();

#ifdef _SELF_REF_TEST_DDR_CHECKSUM
	reg = read_reg(CHECKSUM_STORAGE);
	TRACE_NOTICE("Test DDR \tArea : 0x%x++0x%x\n\t\tCurrent Checksum:\t0x%x\n\t\tPrevious Checksum:\t0x%x\n",
		     DDR_CHECKSUM_AREA_START, DDR_CHECKSUM_AREA_SIZE,
		     _LZG_CalcChecksum((const unsigned char *)DDR_CHECKSUM_AREA_START,
				       DDR_CHECKSUM_AREA_SIZE), reg);
#endif

	return 0;
}

int sta_ddr_ctl_configure(uint32_t boot_reason)
{
	(void)boot_reason;

	ddr_write_reg(0x00000001, DDRADDR(DBG1));
	ddr_write_reg(0x00000001, DDRADDR(PWRCTL));
	ddr_write_reg(DDRVAL(MSTR), DDRADDR(MSTR));
	ddr_write_reg(DDRVAL(MRCTRL0), DDRADDR(MRCTRL0));
	ddr_write_reg(DDRVAL(MRCTRL1), DDRADDR(MRCTRL1));
	ddr_write_reg(DDRVAL(DERATEEN), DDRADDR(DERATEEN));
	ddr_write_reg(DDRVAL(DERATEINT), DDRADDR(DERATEINT));
	ddr_write_reg(0x0000000a, DDRADDR(PWRCTL));
	ddr_write_reg(DDRVAL(PWRTMG), DDRADDR(PWRTMG));
	ddr_write_reg(DDRVAL(HWLPCTL), DDRADDR(HWLPCTL));
	ddr_write_reg(0x00210000, DDRADDR(RFSHCTL0));
	ddr_write_reg(DDRVAL(RFSHCTL0), DDRADDR(RFSHCTL0));
	ddr_write_reg(DDRVAL(RFSHCTL3), DDRADDR(RFSHCTL3));
	ddr_write_reg(0x00620009, DDRADDR(RFSHTMG));
	ddr_write_reg(DDRVAL(RFSHTMG), DDRADDR(RFSHTMG));     /* 400Mhz */
	ddr_write_reg(DDRVAL(CRCPARCTL0), DDRADDR(CRCPARCTL0));
	ddr_write_reg(DDRVAL(INIT0), DDRADDR(INIT0));
	ddr_write_reg(DDRVAL(INIT1), DDRADDR(INIT1));
	ddr_write_reg(DDRVAL(INIT2), DDRADDR(INIT2));
	ddr_write_reg(DDRVAL(INIT3), DDRADDR(INIT3));
	ddr_write_reg(DDRVAL(INIT4), DDRADDR(INIT4));
	ddr_write_reg(DDRVAL(INIT5), DDRADDR(INIT5));
	ddr_write_reg(DDRVAL(DIMMCTL), DDRADDR(DIMMCTL));
	ddr_write_reg(DDRVAL(DRAMTMG0), DDRADDR(DRAMTMG0));
	ddr_write_reg(DDRVAL(DRAMTMG1), DDRADDR(DRAMTMG1));
	ddr_write_reg(DDRVAL(DRAMTMG2), DDRADDR(DRAMTMG2));
	ddr_write_reg(DDRVAL(DRAMTMG3), DDRADDR(DRAMTMG3));
	ddr_write_reg(DDRVAL(DRAMTMG4), DDRADDR(DRAMTMG4));
	/* 400Mhz Trcd */
	ddr_write_reg(DDRVAL(DRAMTMG5), DDRADDR(DRAMTMG5));
	ddr_write_reg(DDRVAL(DRAMTMG6), DDRADDR(DRAMTMG6));
	ddr_write_reg(0x0000020d, DDRADDR(DRAMTMG7));
	ddr_write_reg(DDRVAL(DRAMTMG7), DDRADDR(DRAMTMG7));
	ddr_write_reg(DDRVAL(DRAMTMG8), DDRADDR(DRAMTMG8));
	ddr_write_reg(DDRVAL(DRAMTMG14), DDRADDR(DRAMTMG14));
	ddr_write_reg(0x02000009, DDRADDR(ZQCTL0));
	ddr_write_reg(DDRVAL(ZQCTL0), DDRADDR(ZQCTL0));
	ddr_write_reg(0x02000070, DDRADDR(ZQCTL1));
	ddr_write_reg(DDRVAL(ZQCTL1), DDRADDR(ZQCTL1));
	ddr_write_reg(DDRVAL(ZQCTL2), DDRADDR(ZQCTL2));
	ddr_write_reg(0x07020000, DDRADDR(DFITMG0));
	ddr_write_reg(0x07010100, DDRADDR(DFITMG0));
	ddr_write_reg(DDRVAL(DFITMG0), DDRADDR(DFITMG0));
	ddr_write_reg(0x00000402, DDRADDR(DFITMG1));
	ddr_write_reg(0x00000202, DDRADDR(DFITMG1));
	ddr_write_reg(DDRVAL(DFITMG1), DDRADDR(DFITMG1));
	ddr_write_reg(0x07000121, DDRADDR(DFILPCFG0));
	ddr_write_reg(0x07003121, DDRADDR(DFILPCFG0));
	ddr_write_reg(DDRVAL(DFILPCFG0), DDRADDR(DFILPCFG0));
	ddr_write_reg(0x00400004, DDRADDR(DFIUPD0));
	ddr_write_reg(0x20400004, DDRADDR(DFIUPD0));
	ddr_write_reg(DDRVAL(DFIUPD0), DDRADDR(DFIUPD0));
	ddr_write_reg(0x000000a4, DDRADDR(DFIUPD1));
	ddr_write_reg(DDRVAL(DFIUPD1), DDRADDR(DFIUPD1));
	ddr_write_reg(DDRVAL(DFIUPD2), DDRADDR(DFIUPD2));
	ddr_write_reg(0x00000001, DDRADDR(DFIMISC));
#if defined(CFG_DDR_ALT1)
	ddr_write_reg(0x00151515, DDR3_CTRL_BASE + ADDRMAP(1));
#else
	ddr_write_reg(0x00161616, DDR3_CTRL_BASE + ADDRMAP(1));
#endif
	/*bank bits mapped to :   axi [26:24], hif [25:23]*/
	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(2));
	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + ADDRMAP(3));
	ddr_write_reg(0x00000f0f, DDR3_CTRL_BASE + ADDRMAP(4));
	ddr_write_reg(0x04040404, DDR3_CTRL_BASE + ADDRMAP(5));
#if defined(CFG_DDR_ALT1)
	ddr_write_reg(0x0f0f0f04, DDR3_CTRL_BASE + ADDRMAP(6));
#else
	ddr_write_reg(0x0f0f0404, DDR3_CTRL_BASE + ADDRMAP(6));
#endif
	ddr_write_reg(DDRVAL(ODTCFG), DDRADDR(ODTCFG));
	ddr_write_reg(DDRVAL(ODTMAP), DDRADDR(ODTMAP));
	ddr_write_reg(DDRVAL(SCHED), DDRADDR(SCHED));
	ddr_write_reg(DDRVAL(SCHED1), DDRADDR(SCHED1));
	ddr_write_reg(DDRVAL(PERFHPR1), DDRADDR(PERFHPR1));
	ddr_write_reg(DDRVAL(PERFLPR1), DDRADDR(PERFLPR1));
	ddr_write_reg(DDRVAL(PERFWR1), DDRADDR(PERFWR1));
	if (get_soc_id() == SOCID_STA1385 &&
	    get_cut_rev() < CUT_20) {
		ddr_write_reg(0x00000762, DDR3_CTRL_BASE + PERFVPR1);
		ddr_write_reg(0x0000061c, DDR3_CTRL_BASE + PERFVPW1);
	}
	ddr_write_reg(DDRVAL(DBG0), DDRADDR(DBG0));
	ddr_write_reg(0x00000000, DDRADDR(DBG1));
	ddr_write_reg(DDRVAL(DBGCMD), DDRADDR(DBGCMD));
	ddr_write_reg(0x00000001, DDRADDR(SWCTL));
	ddr_write_reg(0x00110011, DDRADDR(POISONCFG));
	ddr_write_reg(0x00100011, DDRADDR(POISONCFG));
	ddr_write_reg(DDRVAL(POISONCFG), DDRADDR(POISONCFG));

	disable_ddr_port(0);
	disable_ddr_port(1);
	disable_ddr_port(2);
	disable_ddr_port(3);

	ddr_write_reg(DDRVAL(PCCFG), DDRADDR(PCCFG));

	/* Port 0 */
	ddr_write_reg(DDRVAL(PCFGR_0), DDRADDR(PCFGR_0));
	ddr_write_reg(DDRVAL(PCFGW_0), DDRADDR(PCFGW_0));
	ddr_write_reg(DDRVAL(PCTRL_0), DDRADDR(PCTRL_0));
	ddr_write_reg(DDRVAL(PCFGQOS0_0), DDRADDR(PCFGQOS0_0));
	ddr_write_reg(DDRVAL(PCFGQOS1_0), DDRADDR(PCFGQOS1_0));
	ddr_write_reg(DDRVAL(PCFGWQOS0_0), DDRADDR(PCFGWQOS0_0));
	ddr_write_reg(DDRVAL(PCFGWQOS1_0), DDRADDR(PCFGWQOS1_0));
	/* Port 1 */
	ddr_write_reg(DDRVAL(PCFGR_1), DDRADDR(PCFGR_1));
	ddr_write_reg(DDRVAL(PCFGW_1), DDRADDR(PCFGW_1));
	ddr_write_reg(DDRVAL(PCTRL_1), DDRADDR(PCTRL_1));
	ddr_write_reg(DDRVAL(PCFGQOS0_1), DDRADDR(PCFGQOS0_1));
	ddr_write_reg(DDRVAL(PCFGQOS1_1), DDRADDR(PCFGQOS1_1));
	ddr_write_reg(DDRVAL(PCFGWQOS0_1), DDRADDR(PCFGWQOS0_1));
	ddr_write_reg(DDRVAL(PCFGWQOS1_1), DDRADDR(PCFGWQOS1_1));
	/* Port 2 */
	ddr_write_reg(DDRVAL(PCFGR_2), DDRADDR(PCFGR_2));
	ddr_write_reg(DDRVAL(PCFGW_2), DDRADDR(PCFGW_2));
	ddr_write_reg(DDRVAL(PCTRL_2), DDRADDR(PCTRL_2));
	ddr_write_reg(DDRVAL(PCFGQOS0_2), DDRADDR(PCFGQOS0_2));
	ddr_write_reg(DDRVAL(PCFGQOS1_2), DDRADDR(PCFGQOS1_2));
	ddr_write_reg(DDRVAL(PCFGWQOS0_2), DDRADDR(PCFGWQOS0_2));
	ddr_write_reg(DDRVAL(PCFGWQOS1_2), DDRADDR(PCFGWQOS1_2));
	/* Port 3 */
	ddr_write_reg(DDRVAL(PCFGR_3), DDRADDR(PCFGR_3));
	ddr_write_reg(DDRVAL(PCFGW_3), DDRADDR(PCFGW_3));
	ddr_write_reg(DDRVAL(PCTRL_3), DDRADDR(PCTRL_3));
	ddr_write_reg(DDRVAL(PCFGQOS0_3), DDRADDR(PCFGQOS0_3));
	ddr_write_reg(DDRVAL(PCFGQOS1_3), DDRADDR(PCFGQOS1_3));
	ddr_write_reg(DDRVAL(PCFGWQOS0_3), DDRADDR(PCFGWQOS0_3));
	ddr_write_reg(DDRVAL(PCFGWQOS1_3), DDRADDR(PCFGWQOS1_3));

	ddr_write_reg(0x00000000, DDRADDR(DBG1));

	ddr_write_reg(0x0000000a, DDRADDR(PWRCTL));
	ddr_write_reg(0x00000008, DDRADDR(PWRCTL));

	ddr_write_reg(~SWCTL_SWDONE, DDRADDR(SWCTL));
	ddr_write_reg(0x00000000, DDRADDR(DFIMISC));

	return 0;
}

/**
 */
int sta_ddr_ctl_power_up(uint32_t boot_reason)
{
	uint32_t reg;

	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + SWCTL);
	ddr_write_reg(0x00000001, DDR3_CTRL_BASE + DFIMISC);
	ddr_write_reg(0x00000001, DDR3_CTRL_BASE + SWCTL);
	udelay(100);

	do {
		reg = ddr_read_reg(DDR3_CTRL_BASE + SWSTAT);
	} while((reg & SWCTL_SWDONE) != SWCTL_SWDONE);

	udelay(100);

	do {
		reg = ddr_read_reg(DDR3_CTRL_BASE + STAT);
	} while((reg & STAT_OPMODE_MASK) != STAT_OPMODE_NORMAL);

	udelay(100);

	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + RFSHCTL3);
	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + PWRCTL);
	ddr_write_reg(0x00000000, DDR3_CTRL_BASE + DFIMISC);

	ddr_write_reg(0x13842202, 0x50900008);
	ddr3_pub_do_pir(0x000001e1);
	udelay(10);

	sta_ddr_resume();


#ifdef VERBOSE_DDR
	sta_ddr_dump();
#endif

	return 0;
}

