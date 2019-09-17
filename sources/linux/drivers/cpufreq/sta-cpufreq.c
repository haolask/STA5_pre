/*
 * Copyright (C) STMicroelectronics 2014
 *
 * License Terms: GNU General Public License v2
 * Author: Jean-Nicolas Graux <jean-nicolas.graux@st.com>
 */
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>

/* sta cpufreq driver data */
struct sta_cpufreq_data {
	struct clk *clk;
	unsigned int transition_latency;
	struct cpufreq_frequency_table *cpufreq_tbl;
};

/*
 * This global is required as used by cpufreq_driver ops.
 * More in details, there is no way to retrieve driver data in
 * sta_cpufreq_getspeed().
 */
struct sta_cpufreq_data *sta_cpufreq;

static int sta_cpufreq_target(struct cpufreq_policy *policy,
			      unsigned int index)
{
	unsigned int new_freq, old_freq;
	int ret;

	old_freq = policy->cur;
	new_freq = sta_cpufreq->cpufreq_tbl[index].frequency;

	if (old_freq == new_freq)
		return 0;

	pr_debug("%s: new freq: %u,  old freq: %u\n", __func__,
		 new_freq, old_freq);

	/* update armss clk frequency */
	ret = clk_set_rate(sta_cpufreq->clk, new_freq * 1000);

	if (ret) {
		pr_err("%s: failed to set clk to %d Hz: error %d\n",
		       __func__, new_freq * 1000, ret);
		new_freq = old_freq;
	}

	return ret;
}

static unsigned int sta_cpufreq_getspeed(unsigned int cpu)
{
	int i = 0;
	unsigned long freq = clk_get_rate(sta_cpufreq->clk) / 1000;

	pr_debug("%s: freq: %lu\n", __func__, freq);

	/* The value is rounded to closest frequency in the defined table. */
	while (sta_cpufreq->cpufreq_tbl[i + 1].frequency !=
			CPUFREQ_TABLE_END) {
		if (freq < sta_cpufreq->cpufreq_tbl[i].frequency +
		   (sta_cpufreq->cpufreq_tbl[i + 1].frequency -
		   sta_cpufreq->cpufreq_tbl[i].frequency) / 2)
			break;
		i++;
	}
	pr_debug("%s: speed: %u\n", __func__,
		 sta_cpufreq->cpufreq_tbl[i].frequency);
	return sta_cpufreq->cpufreq_tbl[i].frequency;
}

static int sta_cpufreq_init(struct cpufreq_policy *policy)
{
	int res;

	policy->cur = sta_cpufreq_getspeed(policy->cpu);
	policy->cpuinfo.transition_latency =
		sta_cpufreq->transition_latency;
	res = cpufreq_table_validate_and_show(policy,
					      sta_cpufreq->cpufreq_tbl);
	if (res) {
		pr_err("%s: failed to validate and show\n", __func__);
		return res;
	}
	policy->suspend_freq = policy->max;
	return 0;
}

static int sta_cpufreq_exit(struct cpufreq_policy *policy)
{
	return 0;
}

static struct cpufreq_driver sta_cpufreq_driver = {
	.flags  = CPUFREQ_STICKY | CPUFREQ_NEED_INITIAL_FREQ_CHECK,
	.verify = cpufreq_generic_frequency_table_verify,
	.target_index = sta_cpufreq_target,
	.get    = sta_cpufreq_getspeed,
	.init   = sta_cpufreq_init,
	.exit   = sta_cpufreq_exit,
	.name   = "sta-cpufreq",
	.attr = cpufreq_generic_attr,
	.suspend = cpufreq_generic_suspend,
};

static int sta_cpufreq_probe(struct platform_device *pdev)
{
	struct device_node *np;
	const struct property *prop;
	const __be32 *val;
	int cnt, i, ret;

	sta_cpufreq = devm_kzalloc(&pdev->dev, sizeof(struct sta_cpufreq_data),
				   GFP_KERNEL);
	if (!sta_cpufreq)
		return -ENOMEM;

	np = of_cpu_device_node_get(0);
	if (!np) {
		dev_err(&pdev->dev, "no cpu node found\n");
		return -ENODEV;
	}

	if (of_property_read_u32(np, "clock-latency",
				 &sta_cpufreq->transition_latency))
		sta_cpufreq->transition_latency = 20000; /* FIX ME */

	prop = of_find_property(np, "cpufreq_tbl", NULL);
	if (!prop || !prop->value) {
		dev_err(&pdev->dev, "invalid cpufreq_tbl\n");
		of_node_put(np);
		return -ENODEV;
	}
	of_node_put(np);

	cnt = prop->length / sizeof(u32);
	dev_dbg(&pdev->dev, "found %i scaling frequencies\n", cnt);

	val = prop->value;

	sta_cpufreq->cpufreq_tbl = devm_kzalloc(&pdev->dev,
			sizeof(struct cpufreq_frequency_table) * (cnt + 1),
			GFP_KERNEL);
	if (!sta_cpufreq->cpufreq_tbl)
		return -ENOMEM;

	for (i = 0; i < cnt; i++) {
		sta_cpufreq->cpufreq_tbl[i].driver_data = i;
		sta_cpufreq->cpufreq_tbl[i].frequency = be32_to_cpup(val++);
		dev_dbg(&pdev->dev, "cpufreq_tbl[%i]: %u\n", i,
			sta_cpufreq->cpufreq_tbl[i].frequency);
	}

	sta_cpufreq->cpufreq_tbl[i].driver_data = i;
	sta_cpufreq->cpufreq_tbl[i].frequency = CPUFREQ_TABLE_END;

	sta_cpufreq->clk = devm_clk_get(&pdev->dev, "cpu_clk");
	if (IS_ERR(sta_cpufreq->clk)) {
		dev_err(&pdev->dev, "unable to get cpu clock\n");
		ret = PTR_ERR(sta_cpufreq->clk);
		return ret;
	}

	platform_set_drvdata(pdev, sta_cpufreq);

	ret = cpufreq_register_driver(&sta_cpufreq_driver);
	if (ret)
		return ret;

	dev_info(&pdev->dev, "initialized\n");
	return 0;
}

static int sta_cpufreq_remove(struct platform_device *pdev)
{
	cpufreq_unregister_driver(&sta_cpufreq_driver);
	return 0;
}

static const struct of_device_id sta_cpufreq_match[] = {
	{ .compatible = "st,sta-cpufreq" },
	{},
};

static struct platform_driver sta_cpufreq_platdrv = {
	.driver = {
		.name	= "sta-cpufreq",
		.of_match_table = sta_cpufreq_match,
	},
	.probe		= sta_cpufreq_probe,
	.remove		= sta_cpufreq_remove,
};
module_platform_driver(sta_cpufreq_platdrv);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cpufreq driver for sta");
