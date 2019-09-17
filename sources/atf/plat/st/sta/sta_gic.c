/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl_common.h>
#include <gicv2.h>
#include <platform_def.h>
#include <sta_private.h>
#include <utils.h>
#include <xlat_tables_v2.h>

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t g0_interrupt_props[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
	PLATFORM_G0_PROPS(GICV2_INTR_GROUP0)
};

static const gicv2_driver_data_t platform_gic_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
	.interrupt_props = g0_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(g0_interrupt_props),
};

mmap_reserve(MAP_GIC);
void sta_gic_driver_init(void)
{
	assert(IN_MMAP(GICD_BASE, MAP_GIC));
	assert(IN_MMAP(GICC_BASE, MAP_GIC));
	gicv2_driver_init(&platform_gic_data);
}

void sta_gic_init(void)
{
	assert(IN_MMAP(GICD_BASE, MAP_GIC));
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	assert(IN_MMAP(GICC_BASE, MAP_GIC));
	gicv2_cpuif_enable();
}

void sta_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

void sta_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

void sta_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
}

