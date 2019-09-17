/*
 *  arch/arm/mach-sta/platsmp.c
 *
 * Copyright (C) 2016 STMicroelectronics (R&D) Limited.
 *		http://www.st.com
 *
 * Cloned from linux/arch/arm/mach-vexpress/platsmp.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/of.h>
#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include "smp.h"

static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	sync_cache_w(&pen_release);
}

static DEFINE_SPINLOCK(boot_lock);

static void sta_secondary_init(unsigned int cpu)
{
	trace_hardirqs_off();

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

static int sta_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	/*
	 * set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 *
	 * Note that "pen_release" is the hardware CPU ID, whereas
	 * "cpu" is Linux's internal ID.
	 */
	write_pen_release(cpu_logical_map(cpu));

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * it to jump to the secondary entrypoint.
	 */
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

void sta_smp_prepare_cpus(unsigned int max_cpus)
{
	struct device_node *np;
	void __iomem *start_ptr;
	u32 release_phys;
	int cpu;

	for_each_possible_cpu(cpu) {
		np = of_get_cpu_node(cpu, NULL);
		if (!np)
			continue;

		if (of_property_read_u32(np, "cpu-release-addr",
					 &release_phys)) {
			pr_err("CPU %d: missing or invalid cpu-release-addr "
			       "property\n", cpu);
			continue;
		}

		start_ptr = ioremap(release_phys, sizeof(release_phys));

		/*
		 * Write the address of secondary startup into the system-wide
		 * location (usually in Back-up RAM). The BootMonitor waits
		 * until it receives a soft interrupt, and then the secondary
		 * CPU branches to this address.
		 */
		__raw_writel(virt_to_phys(sta_secondary_startup),
			     start_ptr);

		iounmap(start_ptr);
	}
}

static void sta_cpu_die(unsigned int cpu)
{
	/*
	 * there is power-control hardware on this platform, so all
	 * we can do is put the core into WFI; this is safe as the calling
	 * code will have already disabled interrupts
	 */
	for (;;) {
		cpu_do_idle();
		if (pen_release == cpu_logical_map(cpu)) {
			/*
			 * OK, proper wakeup, we're done
			 */
			break;
		}
	}
}

const struct smp_operations sta_smp_ops __initconst = {
	.smp_prepare_cpus	= sta_smp_prepare_cpus,
	.smp_secondary_init	= sta_secondary_init,
	.smp_boot_secondary	= sta_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= sta_cpu_die,
#endif
};

CPU_METHOD_OF_DECLARE(sta_smp_ops, "st,sta", &sta_smp_ops);
