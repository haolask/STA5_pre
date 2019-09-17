/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Declare variables shared between M3 and AP processors */
#define DECLARE_SHARED_DATA

#include <arch_helpers.h>
#include <mmio.h>
#include <gicv2.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <xlat_tables_v2.h>
#include <sta_private.h>

mmap_reserve(MAP_ESRAM_M3);
#ifdef ESRAM_BASE
mmap_reserve(MAP_ESRAM);
#endif
mmap_reserve(MAP_SHARED_DATA);
mmap_reserve(MAP_DRAM);
mmap_reserve(MAP_DRAM_NS);

extern mmap_region_t __mmap_regions_start[];
extern mmap_region_t __mmap_regions_end[];

void sta_configure_mmu(void)
{
#ifndef MMU_OFF
	unsigned int flags = 0;

	mmap_add_range(__mmap_regions_start, __mmap_regions_end);
	/* set up translation tables */
	init_xlat_tables();
#ifdef DCACHE_OFF
	flags |= DISABLE_DCACHE;
#endif
	enable_mmu_svc_mon(flags);
#endif
}

/*
 * Common
 */
uintptr_t plat_get_ns_image_entrypoint(void)
{
	return BL33_BASE + BL33_EXE_OFFSET;
}

mmap_reserve(MAP_SRC_A7);
unsigned int plat_get_syscnt_freq2(void)
{
#define STA1295_TIMER_CLK_FREQ 51200000
#define STA1395_TIMER_CLK_FREQ 52000000

	/**
	 * If MXTAL_FREQ_SEL is set, it means MXTAL = 26Mhz.
	 * And if so, we make a shortcut assuming that PLL2FVCO is set to
	 * 1248Mhz so that timer clk freq = 1248 / 2 / 12 = 52Mhz.
	 * On the other hand, MXTAL_FREQ_SEL is not set, it means MXTAL = 24Mhz.
	 * And in such a case, we also make a shortcut assuming that PLL2FVCO
	 * is set to 1228.8Mhz so that timer clk freq = 1228.8 / 2 / 12 = 51.2Mhz.
	 * TODO: secure this setting by reading PLL2 registers.
	 */
	assert(IN_MMAP(SRC_A7_RESSTAT, MAP_SRC_A7));
	if (mmio_read_32(SRC_A7_RESSTAT) & MXTAL_FREQ_SEL_BIT)
		return STA1395_TIMER_CLK_FREQ;
	else
		return STA1295_TIMER_CLK_FREQ;
}

#ifdef AARCH32_SP_OPTEE
/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t sta_get_spsr_for_bl32_entry(void)
{
	return SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
			   DISABLE_ALL_EXCEPTIONS);
}
#endif

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t sta_get_spsr_for_bl33_entry(void)
{
	unsigned int hyp_status, mode, spsr;

	hyp_status = GET_VIRT_EXT(read_id_pfr1());

	mode = (hyp_status) ? MODE32_hyp : MODE32_svc;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_MODE32(mode, plat_get_ns_image_entrypoint() & 0x1,
			SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

/******************************************************************************
 * Find first bit set
 ******************************************************************************/
int ffs(int i)
{
    /* Use gcc's builtin ffs. */
    return __builtin_ffs(i);
}

/******************************************************************************
 * Wait for SCP (aka the C-M3 micro-ctrl sub-system) to set sync point
 * announcing it reached a state so that it's safe to start the AP OS.
 * This is to make sure critical SCP init code execute before
 * start of AP OS.
 ******************************************************************************/
void sta_scp_wait_sync(void)
{
	while (!get_sync_point())
		;
}

