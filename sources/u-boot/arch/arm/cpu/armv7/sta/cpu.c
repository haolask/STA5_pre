/*
 * Copyright (C) 2016 STMicroelectronics
 * Author: Philippe LANGLAIS <philippe.langlais@st.com> for STMicroelectronics.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ipc.h>

#define CPU_MSG_RESET_REQUEST	0xFFFF0001

void s_init(void)
{
}

__attribute__((noreturn)) void reset_cpu(ulong addr __attribute__((unused)))
{
	u32	msg = CPU_MSG_RESET_REQUEST;

	while (ipc_send_data(&msg, sizeof(msg)))
		;
	for (;;)
		;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
	/* Enable D-cache for ESRAM too (first 512KB) */
#if defined(CONFIG_SYS_ARM_CACHE_WRITETHROUGH)
	mmu_set_region_dcache_behaviour(STA_ESRAM_START,
			STA_ESRAM_SIZE, DCACHE_WRITETHROUGH);
#else
	mmu_set_region_dcache_behaviour(STA_ESRAM_START,
			STA_ESRAM_SIZE, DCACHE_WRITEBACK);
#endif
}
#endif

#ifdef CONFIG_ARMV7_NONSEC
extern void *sta_smp_pen;

/* Set the address at which the secondary core starts from.*/
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
	u32 __iomem *boot_addr = (void *)&sta_smp_pen;
	writel(addr, boot_addr);
	writel(addr, CONFIG_SMP_PEN_ADDR);
}
#endif

/* board_deinit weak function */
__weak void board_deinit(void)
{
	/* please define board specific deinit() */
}

/* Here we deinitialize drivers if needed before OS boot */
void arch_preboot_os(void)
{
	board_deinit();
}
