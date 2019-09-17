/*
 * Device Tree support for STMicroelectronics Automotive
 * car processors.
 *
 * Copyright (C) 2015 ST Microelectronics
 *
 * Jean-Nicolas Graux <jean-nicolas.graux@st.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clk-provider.h>
#include <linux/clocksource.h>
#include <linux/memblock.h>
#include <asm/cacheflush.h>

#include <asm/mach/arch.h>

#ifndef CONFIG_ARM_PSCI
#include "smp.h"
#endif

#include "ramdump.h"

void __init sta_late_init(void)
{
	sta_ramdump_init();
}

static void __init sta_dt_init_time(void)
{
	of_clk_init(NULL);
	clocksource_probe();
}

static const char * const sta_dt_board_compat[] = {
	"st,sta1295",
	"st,sta1195",
	"st,sta1385",
	"st,sta1275",
	NULL,
};

DT_MACHINE_START(STA_DT, "ST Automotive SoC with Flattened Device Tree")
	.init_time	= sta_dt_init_time,
	.init_late	= sta_late_init,
	.dt_compat	= sta_dt_board_compat,
#ifndef CONFIG_ARM_PSCI
	.smp		= smp_ops(sta_smp_ops),
#endif
MACHINE_END
