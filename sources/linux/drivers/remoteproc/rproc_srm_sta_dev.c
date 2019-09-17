/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author: Seraphin Bonnaffe <seraphin.bonnaffe@st.com>.
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

#include <linux/component.h>
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rproc_srm_sta_dev.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

struct rproc_srm_sta_dev {
	struct device *dev;
	struct list_head node;
	struct device_node *shared_rsc_np;
	struct of_changeset chgset;
	struct dentry *dbg_dir;
};

static LIST_HEAD(sta_dev_list);
static struct dentry *dbg_dir_root;

static void
rproc_srm_sta_dev_unbind(struct device *dev, struct device *master, void *data)
{
	dev_dbg(dev, "%s\n", __func__);
}

static int shared_device_enable(struct device *dev, bool state)
{
	struct rproc_srm_sta_dev *rproc_srm_sta_dev = dev_get_drvdata(dev);
	struct device_node *np = rproc_srm_sta_dev->shared_rsc_np;
	struct property *prop;
	int err;

	if (!np)
		return -EINVAL;

	prop = devm_kzalloc(dev, sizeof(*prop), GFP_KERNEL);
	if (!prop)
		return -ENOMEM;

	prop->name = "status";
	if (state)
		prop->value = "okay";
	else
		prop->value = "disabled";

	prop->length = strlen((char *)prop->value) + 1;

	of_changeset_init(&rproc_srm_sta_dev->chgset);
	of_changeset_update_property(&rproc_srm_sta_dev->chgset, np, prop);
	err = of_changeset_apply(&rproc_srm_sta_dev->chgset);
	if (err) {
		dev_err(dev, "%s: failed to apply changeset\n", __func__);
		return err;
	}

	return 0;
}

int rproc_srm_sta_dev_enable(char *device_name, bool state)
{
	struct rproc_srm_sta_dev *r;

	list_for_each_entry(r, &sta_dev_list, node) {
		if (!strcmp(r->dev->of_node->name, device_name)) {
			dev_info(r->dev, "Found registered device %s\n",
				 device_name);

			return shared_device_enable(r->dev, state);
		}
	}
	pr_info("rproc_srm_sta_dev_enable: Could not find device %s\n",
		device_name);

	return -ENODEV;
}
EXPORT_SYMBOL_GPL(rproc_srm_sta_dev_enable);

static ssize_t list_read(struct file *file, char __user *buf, size_t count,
			 loff_t *offset)
{
	struct rproc_srm_sta_dev *r;
	int err = 0;
	int use = 0;
	int ret;
	char *touser;
	char *realloc;
	int alloc_size = 256;

	touser = kzalloc(alloc_size, GFP_KERNEL);
	if (!touser)
		return -ENOMEM;

	list_for_each_entry(r, &sta_dev_list, node) {
		while (use + strlen(r->dev->of_node->name) > alloc_size) {
			alloc_size *= 2;
			realloc = krealloc(touser, alloc_size, GFP_KERNEL);
			if (!realloc)
				return -ENOMEM;

			touser = realloc;
		}

		ret = sprintf(&touser[use], "%s\n", r->dev->of_node->name);
		use += ret;
	}

	*(touser + use) = '\0';

	err = simple_read_from_buffer(buf, count, offset, touser, use);
	if (err < 0)
		dev_err(r->dev, "cannot copy to user, err: %d\n", err);

	kfree(touser);
	return err;
}

ssize_t rproc_rdm_get_devices(char *str, uint32_t len)
{
	struct rproc_srm_sta_dev *r;
	int use = 0;
	int ret;

	list_for_each_entry(r, &sta_dev_list, node) {
		ret = snprintf(&str[use], len - use - 1, "%s ",
			       r->dev->of_node->name);
		use += ret;
		if (use >= len - 1)
			break;
	}
	str[use] = '\0';
	return use;
}
EXPORT_SYMBOL_GPL(rproc_rdm_get_devices);

static const struct file_operations list_traceops = {
	.read = list_read,
	.open = simple_open,
	.llseek	= generic_file_llseek,
};

static ssize_t state_write(struct file *filp, const char __user *user_buf,
			   size_t count, loff_t *ppos)
{
	struct rproc_srm_sta_dev *rproc_srm_sta_dev = filp->private_data;
	char buf[10];
	int ret;

	if (count < 1 || count > sizeof(buf))
		return -EINVAL;

	ret = copy_from_user(buf, user_buf, count);
	if (ret)
		return -EFAULT;

	/* remove end of line */
	if (buf[count - 1] == '\n')
		buf[count - 1] = '\0';

	if (!strncmp(buf, "okay", count))
		shared_device_enable(rproc_srm_sta_dev->dev, true);
	else if (!strncmp(buf, "disabled", count))
		shared_device_enable(rproc_srm_sta_dev->dev, false);
	else
		pr_err("Unknown command: %s", buf);

	return count;
}

static const struct file_operations state_traceops = {
	.write = state_write,
	.open = simple_open,
	.llseek	= generic_file_llseek,
};

static void srm_sta_dev_debugfs_create_file(struct device *dev)
{
	struct rproc_srm_sta_dev *rproc_srm_sta_dev = dev_get_drvdata(dev);

	rproc_srm_sta_dev->dbg_dir = debugfs_create_dir(dev->of_node->name,
							dbg_dir_root);
	if (!rproc_srm_sta_dev->dbg_dir)
		dev_err(dev, "can't create debugfs dir\n");

	debugfs_create_file("state", 0200, rproc_srm_sta_dev->dbg_dir,
			    rproc_srm_sta_dev, &state_traceops);
}

static int
rproc_srm_sta_dev_bind(struct device *dev, struct device *master, void *data)
{
	dev_dbg(dev, "%s\n", __func__);

#ifdef CONFIG_DEBUG_FS
	/* Create debugFS to enable/disable DT nodes */
	srm_sta_dev_debugfs_create_file(dev);
#endif

	return 0;
}

static const struct component_ops rproc_srm_sta_dev_ops = {
	.bind = rproc_srm_sta_dev_bind,
	.unbind = rproc_srm_sta_dev_unbind,
};

static int rproc_srm_sta_dev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct device_node *npp = of_get_parent(np);
	struct rproc_srm_sta_dev *rproc_srm_sta_dev;

	dev_err(dev, "%s for node %s\n", __func__, dev->of_node->name);

	rproc_srm_sta_dev = devm_kzalloc(dev, sizeof(struct rproc_srm_sta_dev),
					 GFP_KERNEL);
	if (!rproc_srm_sta_dev)
		return -ENOMEM;

	rproc_srm_sta_dev->dev = dev;

	rproc_srm_sta_dev->shared_rsc_np = of_parse_phandle(np,
							    "st,shared-dev",
							    0);

#ifdef CONFIG_DEBUG_FS
	/* Create a debugFS to enable and disable DT nodes from userland */
	if (debugfs_initialized() && !dbg_dir_root) {
		dbg_dir_root = debugfs_create_dir(npp->name, NULL);
		if (!dbg_dir_root)
			dev_err(dev, "can't create debugfs dir\n");
		/* add information node for debug purpose */
		debugfs_create_file("list_devices", 0400, dbg_dir_root,
				    rproc_srm_sta_dev, &list_traceops);
	}
#endif

	dev_set_drvdata(dev, rproc_srm_sta_dev);

	list_add(&rproc_srm_sta_dev->node, &sta_dev_list);

	return component_add(dev, &rproc_srm_sta_dev_ops);
}

static int rproc_srm_sta_dev_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rproc_srm_sta_dev *rproc_srm_sta_dev = dev_get_drvdata(dev);

	list_del(&rproc_srm_sta_dev->node);
	component_del(dev, &rproc_srm_sta_dev_ops);

	return 0;
}

static const struct of_device_id rproc_srm_sta_dev_match[] = {
	{ .compatible = "rproc-srm-sta-dev", },
	{},
};

MODULE_DEVICE_TABLE(of, rproc_srm_sta_dev_match);

static struct platform_driver rproc_srm_sta_dev_driver = {
	.probe = rproc_srm_sta_dev_probe,
	.remove = rproc_srm_sta_dev_remove,
	.driver = {
		.name = "rproc-srm-sta-dev",
		.of_match_table = of_match_ptr(rproc_srm_sta_dev_match),
	},
};

module_platform_driver(rproc_srm_sta_dev_driver);

MODULE_AUTHOR("Seraphin Bonnaffe <seraphin.bonnaffe@st.com>");
MODULE_DESCRIPTION("Remoteproc System Resource Manager driver - dev (STA)");
MODULE_LICENSE("GPL v2");
