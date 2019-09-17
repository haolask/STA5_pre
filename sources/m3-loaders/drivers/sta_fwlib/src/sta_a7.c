/**
 * @file sta_a7.c
 * @brief This file provides all the Cortex A7 utilities functions
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */
#include <errno.h>
#include "trace.h"
#include "utils.h"

#include "sta_common.h"
#include "sta_mtu.h"
#include "sta_src_a7.h"
#include "sta_a7.h"

#define SBY_WFI	(BIT(0) | BIT(1))

/* System Time Stamp generators APB0-1 */
static const uint32_t sys_time_stamp_gen_apb0[SOCID_MAX] = {
	0x54910000, 0x54910000, 0x482C0000, 0x54910000
};
static const uint32_t sys_time_stamp_gen_apb1[SOCID_MAX] = {
	0x54920000, 0x54920000, 0x482D0000, 0x54920000
};

/**
 * @brief	enter a given CPU to reset
 * @param	cpuid: CPU processor id
 * @return	0 if no error, not 0 otherwise
 */
int a7_cpu_stop(int cpuid)
{
	switch (get_soc_id()) {
		case SOCID_STA1275:
			/* single core */
			if (cpuid != CPU0) return -EINVAL;
			break;
		case SOCID_STA1295:
		case SOCID_STA1195:
		case SOCID_STA1385:
			/* dual core */
			if (cpuid != CPU0 && cpuid != CPU1) return -EINVAL;
			break;
		default:
			break;
	}

	misc_a7_regs->misc_reg16.bit.ncpuporeset &= ~BIT(cpuid);
	return 0;
}

/**
 * @brief	start a given CPU
 * @param	cpuid: CPU processor id
 * @return	0 if no error, not 0 otherwise
 */
int a7_cpu_start(int cpuid)
{
	switch (get_soc_id()) {
		case SOCID_STA1275:
			/* single core */
			if (cpuid != CPU0) return -EINVAL;
		break;
		case SOCID_STA1295:
		case SOCID_STA1195:
		case SOCID_STA1385:
			/* dual core */
			if (cpuid != CPU0 && cpuid != CPU1) return -EINVAL;
		default:
			break;
	}

	misc_a7_regs->misc_reg16.bit.ncpuporeset |= BIT(cpuid);
	wait_n_cycles(16); /* wait at least 16 cycles */
	return 0;
}

/**
 * @brief	A7 Power up sequence, by:
 *	- setting PLL ARM as input clock of A7 sub-system
 *	- set reset trampoline code @0 to jump in start @ != 0
 *	- removing reset
 * @param	start: AP start address
 * @return	NA
 */
void a7_start(uint32_t start)
{
	if (start && start != 0x70000000) {
		/*
		 * To jump in DDR after AP reset:
		 *
		 * Write trampoline reset code to jump in DDRAM
		 * reset_trampoline:
		 *	ldr	r0, =Start@
		 *	bx	r0
		 * 00000008 <reset_trampoline>:
		 *  8:	e51f0000	ldr r0, [pc, #0]
		 *  c:	e12fff10	bx	r0
		 *  10:	Start@	.word	Start@
		 */
		write_reg(0xe51f0000, ESRAM_A7_BASE);	/* ldr r0, [pc, #0] */
		write_reg(0xe12fff10, ESRAM_A7_BASE + 4);	/* bx r0 */
		write_reg(start, ESRAM_A7_BASE + 8);	/* .word @ */
	}

	/**
	 * Enable the SoC timestamp so that ARM v7 generic timer
	 * will start incrementing.
	 */
	write_reg(0x1, sys_time_stamp_gen_apb0[get_soc_id()]);

	/*
	 * Remove clamp signal that isolate the cpu outputs
	 * to Vsoc domain.
	 */
	misc_a7_regs->misc_reg16.bit.outclamp_i = 0;
	wait_n_cycles(16); /* wait at least 16 cycles */

	/*
	 * Remove isolation of the bridge outputs
	 */
	misc_a7_regs->misc_reg16.bit.bridge_fct_iso = 0;
	wait_n_cycles(16); /* wait at least 16 cycles */

	misc_a7_regs->misc_reg16.bit.nporeset_varm = 1;
	wait_n_cycles(16); /* wait at least 16 cycles */

	/* Sanity check to make sure PLL ARM is enabled */
	if (!a7_ssc_regs->pllarm_ctrl.bit.enable ||
	    !a7_ssc_regs->pllarm_lockp.bit.clk_good) {
		TRACE_ERR("%s: PLL ARM not alive\n", __func__);
		wait_forever;
	}

	/*
	 * Move to internal clock selection (eg PLL ARM)
	 * and wait for ack
	 */
	a7_ssc_regs->chclkreq.bit.chgclkreq = 0;
	while(a7_ssc_regs->chclkreq.bit.chgclkreq);

	switch (get_soc_id()) {
	case SOCID_STA1275:
		/* single core */
		a7_cpu_start(CPU0);
		break;
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1385:
		/* dual core */
		a7_cpu_start(CPU0);
		a7_cpu_start(CPU1);
		break;
	default:
		TRACE_ERR("%s: Undefined SoC\n", __func__);
		break;
	}


	misc_a7_regs->misc_reg16.bit.nhardreset = 1;
	wait_n_cycles(16); /* wait at least 16 cycles */
}

/**
 * @brief	A7 Power off sequence, by:
 *	- moving A7 sub-system in reset
 */
void a7_stop(void)
{
	int wfi;

	srca7_pclk_enable_all(src_a7_regs);

	mdelay(1);
	wfi = misc_a7_regs->misc_reg17.bit.standbywfi;
	if (wfi != SBY_WFI)
		TRACE_INFO("%s: A7 CPUs not in WFI: %d\n", __func__, wfi);

	/*
	 * Set isolation of the bridge outputs
	 */
	misc_a7_regs->misc_reg16.bit.bridge_fct_iso = 1;
	udelay(1);

	/*
	 * Set clamp signal that isolate the cpu outputs
	 * to Vsoc domain.
	 */
	misc_a7_regs->misc_reg16.bit.outclamp_i = 1;
	udelay(1);

	misc_a7_regs->misc_reg16.bit.nhardreset = 0;

	switch (get_soc_id()) {
		case SOCID_STA1275:
			/* single core */
			a7_cpu_stop(CPU0);
			break;
		case SOCID_STA1295:
		case SOCID_STA1195:
		case SOCID_STA1385:
			/* dual core */
			a7_cpu_stop(CPU0);
			a7_cpu_stop(CPU1);
			break;
		default:
			TRACE_ERR("%s: undefined SoC\n", __func__);
			break;
	}

	misc_a7_regs->misc_reg16.bit.nporeset_varm = 0;
}
