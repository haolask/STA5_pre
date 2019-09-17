/*
 * arch/arm/mach-sta/pm.c
 *
 * Copyright (C) 2017 STMicroelectronics (R&D) Limited.
 *		http://www.st.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/device.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox_sta.h>
#include <linux/of_device.h>
#include <linux/suspend.h>
#include <asm/suspend.h>

#include "smp.h"

/**
 * The payload data of mailbox message to be send to C-M3 remote processor,
 * thus to move the SoC in stand-by state.
 */
#define PM_MSG_SUSPEND 0xFFFF0002

extern int sta_finish_suspend(unsigned long cpu_state);
extern void sta_cpu_resume(void);

struct sta_suspend_data {
	struct mbox_client cl;
	struct mbox_chan *mb_chan;
};

/* ugly global used to get device handle at machine suspend finish time */
static struct platform_device *sta_suspend_dev;

int sta_send_suspend_request(void)
{
	int ret;
	struct sta_mbox_msg msg;
	u8 data[sizeof(u32)];

	struct sta_suspend_data *suspend_data =
		platform_get_drvdata(sta_suspend_dev);
	struct device *dev = &sta_suspend_dev->dev;

	*(u32 *)data = PM_MSG_SUSPEND;
	msg.dsize = sizeof(data);
	msg.pdata = data;

	ret = mbox_send_message(suspend_data->mb_chan, (void *)&msg);
	if (ret < 0) {
		dev_err(dev, "mbox send failed: %d\n", ret);
		return ret;
	}
	return 0;
}

static int sta_suspend_enter(suspend_state_t state)
{
	switch (state) {
	case PM_SUSPEND_MEM:
		/* go zzz */
		cpu_suspend(0, sta_finish_suspend);
		/**
		 * Restore the address of secondary startup into the
		 * system-wide location
		 */
		sta_smp_prepare_cpus(0);
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static const struct platform_suspend_ops sta_suspend_ops = {
	.enter	      = sta_suspend_enter,
	.valid	      = suspend_valid_only_mem,
};

static int sta_suspend_probe(struct platform_device *pdev)
{
	int ret;
	struct sta_suspend_data *suspend_data;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	void __iomem *bootcpu_reg;
	u32 addr;

	suspend_data = devm_kzalloc(&pdev->dev, sizeof(*suspend_data),
				    GFP_KERNEL);
	if (!suspend_data)
		return -ENOMEM;

	suspend_data->cl.dev = dev;
	suspend_data->cl.tx_block = true;
	suspend_data->cl.tx_tout = 1000;
	suspend_data->cl.knows_txdone = false;
	suspend_data->cl.rx_callback = NULL;
	suspend_data->cl.tx_done = NULL;

	suspend_data->mb_chan = mbox_request_channel(&suspend_data->cl, 0);
	if (IS_ERR(suspend_data->mb_chan)) {
		ret = IS_ERR(suspend_data->mb_chan);
		dev_err(dev, "mbox_request_channel failed: %d\n", ret);
		return ret;
	}

	platform_set_drvdata(pdev, suspend_data);
	sta_suspend_dev = pdev;

	if (of_property_read_u32(np, "st,cpu-resume-addr", &addr)) {
		dev_err(dev, "no cpu resume addr\n");
		return -EINVAL;
	}

	bootcpu_reg = ioremap(addr, 0x4);
	if (!bootcpu_reg)
		return -ENOMEM;

	writel(virt_to_phys(sta_cpu_resume), bootcpu_reg);
	iounmap(bootcpu_reg);

	suspend_set_ops(&sta_suspend_ops);
	return 0;
}

static const struct of_device_id sta_suspend_match[] = {
	{ .compatible = "st,sta-suspend" },
	{},
};

static struct platform_driver sta_suspend_driver = {
	.probe	= sta_suspend_probe,
	.driver = {
		.name = "sta-suspend",
		.of_match_table = sta_suspend_match,
	},
};

static int __init sta_suspend_init(void)
{
	return platform_driver_register(&sta_suspend_driver);
}
device_initcall(sta_suspend_init);
