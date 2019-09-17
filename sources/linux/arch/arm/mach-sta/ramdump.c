/*
 * Ramdump support for STMicroelectronics Automotive
 * car processors.
 *
 * Copyright (C) 2015 ST Microelectronics
 *
 * Nicolas Guion <nicolas.guion@st.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/memblock.h>
#include <linux/of.h>
#include <asm/cacheflush.h>

#define RAMDUMP_MAGIC 0x444D4152

static int sta_ramdump_magic_ptr;

static int sta_panic_notify(struct notifier_block *self,
			    unsigned long event, void *data)
{
	int *ptr;

	if (sta_ramdump_magic_ptr) {
		ptr = phys_to_virt(sta_ramdump_magic_ptr);
		*ptr = RAMDUMP_MAGIC;
		/*  Flush all Cache areas in order to get
		 *  MainMemory consistent for µCSS
		 */
		mb();
		flush_cache_all();
	}

	return NOTIFY_OK;
}

static struct notifier_block on_panic_sta_nb = {
	.notifier_call = sta_panic_notify,
	.priority = 0,
};

void __init sta_ramdump_init(void)
{
	struct device_node *np;
	int *ptr;

	sta_ramdump_magic_ptr =  0;

	np = of_find_node_by_path("/reserved-memory");
	if (!np)
		return;

	if (of_property_read_u32(np, "ramdump-magic-addr",
				 &sta_ramdump_magic_ptr)) {
		sta_ramdump_magic_ptr =  0;
		return;
	}

	if (memblock_add(sta_ramdump_magic_ptr, SZ_4K)) {
		sta_ramdump_magic_ptr = 0;
		pr_err("%s error to reserve memory\n", __func__);
	} else {
		ptr = phys_to_virt(sta_ramdump_magic_ptr);
		*ptr = 0;
		atomic_notifier_chain_register(&panic_notifier_list,
					       &on_panic_sta_nb);
	}
}
