/*
 * Copyright (C) STMicroelectronics 2016
 *
 * Authors:	Gabriele Simone <gabriele.simone@st.com>,
 *		Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>

static const struct of_device_id st_sai_dt_ids[] = {
	{ .compatible = "st,sai1", },
	{ .compatible = "st,sai2", },
	{ .compatible = "st,sai3", },
	{ .compatible = "st,sai4", },
	{ }
};
MODULE_DEVICE_TABLE(of, st_sai_dt_ids);

static int st_sai_probe(struct platform_device *pdev)
{
	return 0;
}

static int st_sai_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sai_suspend(struct device *dev)
{
	return pinctrl_pm_select_sleep_state(dev);
}

static int sai_resume(struct device *dev)
{
	return pinctrl_pm_select_default_state(dev);
}
#endif

static const struct dev_pm_ops sai_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sai_suspend, sai_resume)
};

static struct platform_driver st_sai_driver = {
	.driver = {
		.name = "st-sai",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(st_sai_dt_ids),
		.pm = &sai_pm_ops,
	},
	.probe =    st_sai_probe,
	.remove =   st_sai_remove,
};

module_platform_driver(st_sai_driver);

MODULE_DESCRIPTION("SoC  st_sai driver");
MODULE_AUTHOR("Gabriele Simone gabriele.simone@st.com");
MODULE_LICENSE("GPL");
