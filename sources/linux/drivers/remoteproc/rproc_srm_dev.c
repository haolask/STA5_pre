/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author: Fabien Dessenne <fabien.dessenne@st.com>.
 *
 * License type: GPLv2
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/component.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#define SUPPLY_SUFFIX   "-supply"

struct rproc_srm_clk_info {
	struct list_head list;
	unsigned int index;
	struct clk *clk;
	const char *name;
	bool enabled;
};

struct rproc_srm_pin_info {
	struct list_head list;
	unsigned int index;
	char *name;
};

struct rproc_srm_regu_info {
	struct list_head list;
	unsigned int index;
	struct regulator *regu;
	const char *name;
	bool enabled;
};

struct rproc_srm_dev {
	struct device *dev;
	struct pinctrl *pctrl;

	struct list_head clk_list_head;
	struct list_head regu_list_head;
	struct list_head pin_list_head;
};

/* Clocks */
static void rproc_srm_dev_deconfig_clocks(struct rproc_srm_dev *rproc_srm_dev)
{
	struct rproc_srm_clk_info *c;
	struct list_head *clk_head = &rproc_srm_dev->clk_list_head;

	list_for_each_entry(c, clk_head, list) {
		if (!c->enabled)
			continue;

		clk_disable_unprepare(c->clk);
		c->enabled = false;
		dev_dbg(rproc_srm_dev->dev, "clk %d (%s) deconfigured\n",
			c->index, c->name);
	}
}

static int rproc_srm_dev_config_clocks(struct rproc_srm_dev *rproc_srm_dev)
{
	struct rproc_srm_clk_info *c;
	struct list_head *clk_head = &rproc_srm_dev->clk_list_head;
	int ret;

	/* Note: not only configuring, but also enabling */

	list_for_each_entry(c, clk_head, list) {
		if (c->enabled)
			continue;

		ret = clk_prepare_enable(c->clk);
		if (ret) {
			dev_err(rproc_srm_dev->dev, "clk %d (%s) cfg failed\n",
				c->index, c->name);
			rproc_srm_dev_deconfig_clocks(rproc_srm_dev);
			return ret;
		}
		c->enabled = true;
		dev_dbg(rproc_srm_dev->dev, "clk %d (%s) configured\n",
			c->index, c->name);
	}

	return 0;
}

static void rproc_srm_dev_put_clocks(struct rproc_srm_dev *rproc_srm_dev)
{
	struct device *dev = rproc_srm_dev->dev;
	struct rproc_srm_clk_info *c, *tmp;
	struct list_head *clk_head = &rproc_srm_dev->clk_list_head;

	list_for_each_entry_safe(c, tmp, clk_head, list) {
		clk_put(c->clk);
		dev_dbg(dev, "put clock %d (%s)\n", c->index, c->name);
		list_del(&c->list);
	}
}

static int rproc_srm_dev_get_clocks(struct rproc_srm_dev *rproc_srm_dev)
{
	struct device *dev = rproc_srm_dev->dev;
	struct device_node *np = dev->of_node;
	struct rproc_srm_clk_info *c;
	struct list_head *clk_head = &rproc_srm_dev->clk_list_head;
	const char *name;
	int nb_c, ret;
	unsigned int i;

	if (!np)
		return 0;

	nb_c = of_clk_get_parent_count(np);
	if (!nb_c)
		return 0;

	for (i = 0; i < nb_c; i++) {
		c = devm_kzalloc(dev, sizeof(*c), GFP_KERNEL);
		if (!c) {
			ret = -ENOMEM;
			goto err;
		}

		c->clk = of_clk_get(np, i);
		if (IS_ERR(c->clk)) {
			dev_err(dev, "clock %d KO (%ld)\n", i,
				PTR_ERR(c->clk));
			ret = -ENOMEM;
			goto err;
		}

		if (!of_property_read_string_index(np, "clock-names", i,
						   &name))
			c->name = devm_kstrdup(dev, name, GFP_KERNEL);

		c->index = i;

		list_add_tail(&c->list, clk_head);
		dev_dbg(dev, "got clock %d (%s)\n", c->index, c->name);
	}

	return 0;

err:
	rproc_srm_dev_put_clocks(rproc_srm_dev);
	return ret;
}

/* Regulators */
static void rproc_srm_dev_deconfig_regus(struct rproc_srm_dev *rproc_srm_dev)
{
	struct rproc_srm_regu_info *r;
	struct list_head *regu_head = &rproc_srm_dev->regu_list_head;

	list_for_each_entry(r, regu_head, list) {
		if (!r->enabled)
			continue;

		regulator_disable(r->regu);
		r->enabled = false;
		dev_dbg(rproc_srm_dev->dev, "regu %d (%s) disabled\n",
			r->index, r->name);
	}
}

static int rproc_srm_dev_config_regus(struct rproc_srm_dev *rproc_srm_dev)
{
	struct rproc_srm_regu_info *r;
	struct list_head *regu_head = &rproc_srm_dev->regu_list_head;
	int ret;

	/* Enable all the regulators */
	list_for_each_entry(r, regu_head, list) {
		if (r->enabled)
			continue;

		ret = regulator_enable(r->regu);
		if (ret) {
			dev_err(rproc_srm_dev->dev, "regu %d (%s) failed\n",
				r->index, r->name);
			rproc_srm_dev_deconfig_regus(rproc_srm_dev);
			return ret;
		}
		r->enabled = true;
		dev_dbg(rproc_srm_dev->dev, "regu %d (%s) enabled\n",
			r->index, r->name);
	}

	return 0;
}

static void rproc_srm_dev_put_regus(struct rproc_srm_dev *rproc_srm_dev)
{
	struct device *dev = rproc_srm_dev->dev;
	struct rproc_srm_regu_info *r, *tmp;
	struct list_head *regu_head = &rproc_srm_dev->regu_list_head;

	list_for_each_entry_safe(r, tmp, regu_head, list) {
		devm_regulator_put(r->regu);
		dev_dbg(dev, "put regu %d (%s)\n", r->index, r->name);
		list_del(&r->list);
	}
}

static int rproc_srm_dev_get_regus(struct rproc_srm_dev *rproc_srm_dev)
{
	struct device *dev = rproc_srm_dev->dev;
	struct device_node *np = dev->of_node;
	struct property *p;
	const char *n;
	char *name;
	struct rproc_srm_regu_info *r;
	struct list_head *regu_head = &rproc_srm_dev->regu_list_head;
	int ret, nb_s = 0;

	if (!np)
		return 0;

	for_each_property_of_node(np, p) {
		n = strstr(p->name, SUPPLY_SUFFIX);
		if (!n || n == p->name)
			continue;

		r = devm_kzalloc(dev, sizeof(*r), GFP_KERNEL);
		if (!r) {
			ret = -ENOMEM;
			goto err_list;
		}

		name = devm_kstrdup(dev, p->name, GFP_KERNEL);
		name[strlen(p->name) - strlen(SUPPLY_SUFFIX)] = '\0';
		r->name = name;

		r->regu = devm_regulator_get(dev, r->name);
		if (IS_ERR(r->regu)) {
			dev_err(dev, "cannot get regu %s\n", r->name);
			ret = -EINVAL;
			goto err_list;
		}

		r->index = nb_s++;

		list_add_tail(&r->list, regu_head);
		dev_dbg(dev, "got regu %d (%s)\n", r->index, r->name);
	}

	return 0;

err_list:
	rproc_srm_dev_put_regus(rproc_srm_dev);
	return ret;
}

/* Pins */
static void rproc_srm_dev_put_pins(struct rproc_srm_dev *rproc_srm_dev)
{
	struct device *dev = rproc_srm_dev->dev;
	struct rproc_srm_pin_info *p, *tmp;
	struct list_head *pin_head = &rproc_srm_dev->pin_list_head;

	list_for_each_entry_safe(p, tmp, pin_head, list) {
		devm_kfree(dev, p->name);
		devm_kfree(dev, p);
		dev_dbg(dev, "remove pin cfg %d (%s)\n", p->index, p->name);
		list_del(&p->list);
	}

	if (!IS_ERR_OR_NULL(rproc_srm_dev->pctrl))
		devm_pinctrl_put(rproc_srm_dev->pctrl);
}

static int rproc_srm_dev_get_pins(struct rproc_srm_dev *rproc_srm_dev)
{
	struct device *dev = rproc_srm_dev->dev;
	struct device_node *np = dev->of_node;
	struct rproc_srm_pin_info *p;
	struct list_head *pin_head = &rproc_srm_dev->pin_list_head;
	int ret, nb_p;
	unsigned int i;
	const char *name;

	if (!np)
		return 0;

	rproc_srm_dev->pctrl = devm_pinctrl_get(dev);
	if (IS_ERR(rproc_srm_dev->pctrl))
		return 0;

	nb_p = of_property_count_strings(np, "pinctrl-names");
	if (!nb_p) {
		dev_err(dev, "pinctrl-names not defined\n");
		devm_pinctrl_put(rproc_srm_dev->pctrl);
		return -EINVAL;
	}

	for (i = 0; i < nb_p; i++) {
		p = devm_kzalloc(dev, sizeof(*p), GFP_KERNEL);
		if (!p) {
			ret = -ENOMEM;
			goto err_list;
		}

		if (of_property_read_string_index(np, "pinctrl-names", i,
						  &name)) {
			dev_err(dev, "no pinctrl-names (pin %d)\n", i);
			ret = -EINVAL;
			goto err_list;
		}
		p->name = devm_kstrdup(dev, name, GFP_KERNEL);

		p->index = i;

		list_add_tail(&p->list, pin_head);
		dev_dbg(dev, "found pin cfg %d (%s)\n", p->index, p->name);
	}
	return 0;

err_list:
	rproc_srm_dev_put_pins(rproc_srm_dev);
	return ret;
}

/* Core */
static void
rproc_srm_dev_unbind(struct device *dev, struct device *master, void *data)
{
	struct rproc_srm_dev *rproc_srm_dev = dev_get_drvdata(dev);

	dev_dbg(dev, "%s\n", __func__);

	rproc_srm_dev_deconfig_regus(rproc_srm_dev);
	rproc_srm_dev_deconfig_clocks(rproc_srm_dev);

	/* For pins: nothing to deconfig */
}

static int
rproc_srm_dev_bind(struct device *dev, struct device *master, void *data)
{
	struct rproc_srm_dev *rproc_srm_dev = dev_get_drvdata(dev);
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	ret = rproc_srm_dev_config_clocks(rproc_srm_dev);
	if (ret)
		return ret;

	ret = rproc_srm_dev_config_regus(rproc_srm_dev);
	if (ret)
		return ret;

	/* No need for pins config ("default" pinctrl applied before probe) */

	return 0;
}

static const struct component_ops rproc_srm_dev_ops = {
	.bind = rproc_srm_dev_bind,
	.unbind = rproc_srm_dev_unbind,
};

static int rproc_srm_dev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rproc_srm_dev *rproc_srm_dev;
	int ret;

	dev_dbg(dev, "%s for node %s\n", __func__, dev->of_node->name);

	rproc_srm_dev = devm_kzalloc(dev, sizeof(struct rproc_srm_dev),
				     GFP_KERNEL);
	if (!rproc_srm_dev)
		return -ENOMEM;

	rproc_srm_dev->dev = dev;
	INIT_LIST_HEAD(&rproc_srm_dev->clk_list_head);
	INIT_LIST_HEAD(&rproc_srm_dev->pin_list_head);
	INIT_LIST_HEAD(&rproc_srm_dev->regu_list_head);

	/* Get clocks, regu and pinctrl */
	ret = rproc_srm_dev_get_clocks(rproc_srm_dev);
	if (ret)
		return ret;

	ret = rproc_srm_dev_get_regus(rproc_srm_dev);
	if (ret)
		goto err;

	ret = rproc_srm_dev_get_pins(rproc_srm_dev);
	if (ret)
		goto err;

	dev_set_drvdata(dev, rproc_srm_dev);

	return  component_add(dev, &rproc_srm_dev_ops);

err:
	rproc_srm_dev_put_pins(rproc_srm_dev);
	rproc_srm_dev_put_regus(rproc_srm_dev);
	rproc_srm_dev_put_clocks(rproc_srm_dev);
	return ret;
}

static int rproc_srm_dev_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rproc_srm_dev *rproc_srm_dev = dev_get_drvdata(dev);

	dev_dbg(dev, "%s\n", __func__);

	component_del(dev, &rproc_srm_dev_ops);

	rproc_srm_dev_put_regus(rproc_srm_dev);
	rproc_srm_dev_put_pins(rproc_srm_dev);
	rproc_srm_dev_put_clocks(rproc_srm_dev);

	return 0;
}

static const struct of_device_id rproc_srm_dev_match[] = {
	{ .compatible = "rproc-srm-dev", },
	{},
};

MODULE_DEVICE_TABLE(of, rproc_srm_dev_match);

static struct platform_driver rproc_srm_dev_driver = {
	.probe = rproc_srm_dev_probe,
	.remove = rproc_srm_dev_remove,
	.driver = {
		.name = "rproc-srm-dev",
		.of_match_table = of_match_ptr(rproc_srm_dev_match),
	},
};

module_platform_driver(rproc_srm_dev_driver);

MODULE_AUTHOR("Fabien Dessenne <fabien.dessenne@st.com>");
MODULE_DESCRIPTION("Remoteproc System Resource Manager driver - dev");
MODULE_LICENSE("GPL v2");
