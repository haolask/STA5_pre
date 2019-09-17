/*
 * ST C3 Channel Controller v3.0
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2016 STMicroelectronics Limited
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "ccc.h"

struct device_type ccc_channel_type = {
	.name = "ccc-channel",
};

static struct ccc_channel *ccc_verify_channel(struct device *dev)
{
	return dev->type == &ccc_channel_type ? to_ccc_channel(dev) : NULL;
}

static int ccc_device_probe(struct device *dev)
{
	struct ccc_channel *channel = ccc_verify_channel(dev);
	struct ccc_channel_driver *driver;

	if (!channel)
		return 0;

	driver = to_ccc_channel_driver(dev->driver);
	if (!driver->probe)
		return -ENODEV;
	return driver->probe(channel);
}

static int ccc_device_remove(struct device *dev)
{
	struct ccc_channel *channel = to_ccc_channel(dev);
	struct ccc_channel_driver *driver;

	if (!channel || !dev->driver)
		return 0;

	driver = to_ccc_channel_driver(dev->driver);
	if (driver->remove)
		return driver->remove(channel);
	return 0;
}

struct bus_type ccc_bus_type = {
	.name = "ccc",
};
EXPORT_SYMBOL_GPL(ccc_bus_type);

struct of_ccc_chn_info {
	struct device_node *node;
	char name[CCC_NAME_SIZE];
	char type[CCC_NAME_SIZE];
	unsigned char index;
};

int ccc_register_channel_driver(struct module *owner,
				struct ccc_channel_driver *driver)
{
	driver->driver.owner = owner;
	driver->driver.bus = &ccc_bus_type;
	driver->driver.probe = ccc_device_probe;
	driver->driver.remove = ccc_device_remove;

	INIT_LIST_HEAD(&driver->channels);
	return driver_register(&driver->driver);
}
EXPORT_SYMBOL(ccc_register_channel_driver);

void ccc_delete_channel_driver(struct ccc_channel_driver *driver)
{
	driver_unregister(&driver->driver);
}
EXPORT_SYMBOL(ccc_delete_channel_driver);

static struct ccc_channel *ccc_new_channel(struct ccc_controller *controller,
					   struct of_ccc_chn_info const *info)
{
	struct ccc_channel *channel;
	struct device *dev;
	int err;

	mutex_lock(&controller->lock);
	if (controller->registrations & BIT(info->index)) {
		dev_err(&controller->dev, "Channel %d already registered\n",
			info->index);
		return NULL;
	}

	channel = kzalloc(sizeof(*channel), GFP_KERNEL);
	if (!channel)
		return NULL;

	channel->controller = controller;
	channel->index = info->index;
	channel->node = info->node;
	strlcpy(channel->name, info->name, sizeof(channel->name));

	dev = &channel->dev;
	dev->parent = &controller->dev;
	dev->bus = &ccc_bus_type;
	dev->type = &ccc_channel_type;
	dev_set_name(dev, "%s:%s@%d", dev_name(dev->parent), channel->name,
		     channel->index);

	if (dma_set_coherent_mask(dev, DMA_BIT_MASK(32))) {
		dev_err(&controller->dev, "No suitable DMA available\n");
		kfree(channel);
		return NULL;
	}

	err = device_register(dev);
	if (err) {
		dev_err(&controller->dev,
			"Channel %d registration fails with error %d\n",
			info->index, err);
		kfree(channel);
		return  NULL;
	}

	controller->registrations |= BIT(channel->index);
	mutex_unlock(&controller->lock);

	dev_info(&controller->dev, "Channel %s registered\n", dev_name(dev));
	return channel;
}

void ccc_delete_channel(struct ccc_channel *channel)
{
	struct ccc_controller *controller = channel->controller;

	mutex_lock(&controller->lock);
	controller->registrations &= ~BIT(channel->index);
	device_unregister(&channel->dev);
	memset(&channel->dev, 0, sizeof(struct device));
	kfree(channel);
	mutex_unlock(&controller->lock);
}
EXPORT_SYMBOL_GPL(ccc_delete_channel);

struct criterion {
	struct ccc_channel_driver *driver;
	bool available;
};

static int match_channel(struct device *device, void *data)
{
	struct criterion *criterion = (struct criterion *)data;
	struct ccc_channel *channel = ccc_verify_channel(device);

	if (!channel)
		return 0;

	if (&criterion->driver->driver != device->driver)
		return 0;

	if (criterion->available) {
		if (try_wait_for_completion(&channel->available))
			return 1;
	}
	return 0;
}

struct ccc_channel *ccc_get_channel(struct ccc_channel_driver *driver)
{
	struct criterion criterion = {.driver = driver, .available = true};
	struct ccc_channel *channel;
	struct device *device;

	device = bus_find_device(&ccc_bus_type, NULL, &criterion,
				 match_channel);

	if (device) {
		channel = to_ccc_channel(device);
	} else {
		criterion.available = false;
		device = bus_find_device(&ccc_bus_type, NULL, &criterion,
					 match_channel);
		if (!device)
			return NULL;
		channel = to_ccc_channel(device);
		/*
		 * The risk here is that all threads will wait for the same
		 * channel. Each should wait for the least scheduled channel
		 * that matches the expected driver.
		 */
		wait_for_completion(&channel->available);
	}
	return channel;
}
EXPORT_SYMBOL_GPL(ccc_get_channel);

static struct ccc_channel *of_ccc_register_channel(struct ccc_controller *c3,
						   struct device_node *node)
{
	struct of_ccc_chn_info info = {.node = node,};
	struct ccc_channel *channel;

	dev_dbg(&c3->dev, "Register %s\n", node->full_name);

	strlcpy(info.name, node->name, sizeof(info.name));

	if (of_modalias_node(node, info.type, sizeof(info.type)) < 0) {
		dev_err(&c3->dev, "Modalias failure on %s\n", node->full_name);
		return ERR_PTR(-EINVAL);
	}
	dev_dbg(&c3->dev, "Channel type: %s\n", info.type);

	if (of_property_read_u8(node, "index", &info.index)) {
		dev_err(&c3->dev, "Missing index property on %s\n",
			node->full_name);
		return ERR_PTR(-EINVAL);
	}
	if (info.index > MAX_CHANNEL_INDEX) {
		dev_err(&c3->dev, "Invalid index on %s\n", node->full_name);
		return ERR_PTR(-EINVAL);
	}

	channel = ccc_new_channel(c3, &info);
	if (!channel) {
		dev_err(&c3->dev, "Failure registering %s\n", node->full_name);
		return ERR_PTR(-EINVAL);
	}
	return channel;
}

static void of_ccc_register_channels(struct ccc_controller *controller)
{
	struct device_node *node;

	if (!controller->dev.of_node)
		return;

	dev_dbg(&controller->dev, "Walking child nodes\n");

	for_each_available_child_of_node(controller->dev.of_node, node)
		of_ccc_register_channel(controller, node);
}

int ccc_register_controller(struct ccc_controller *controller)
{
	int ret;

	dev_set_name(&controller->dev, "ccc@%x", controller->physbase);
	controller->dev.bus = &ccc_bus_type;
	mutex_init(&controller->lock);
	controller->registrations = 0;

	ret = device_register(&controller->dev);
	if (ret)
		return ret;

	of_ccc_register_channels(controller);
	return ret;
}
EXPORT_SYMBOL(ccc_register_controller);

static int __process_removed_controller(struct device_driver *d, void *data)
{
	struct ccc_channel_driver *driver = to_ccc_channel_driver(d);
	struct ccc_controller *controller = data;
	struct ccc_channel *channel, *next;

	list_for_each_entry_safe(channel, next, &driver->channels, probed) {
		if (channel->controller == controller) {
			list_del(&channel->probed);
			ccc_delete_channel(channel);
		}
	}
	return 0;
}

void ccc_delete_controller(struct ccc_controller *controller)
{
	bus_for_each_drv(&ccc_bus_type, NULL, controller,
			 __process_removed_controller);

	device_unregister(&controller->dev);

	memset(&controller->dev, 0, sizeof(struct device));
}
EXPORT_SYMBOL(ccc_delete_controller);

static int __init ccc_init(void)
{
	return bus_register(&ccc_bus_type);
}
postcore_initcall(ccc_init);

static void __exit ccc_exit(void)
{
	bus_unregister(&ccc_bus_type);
}
module_exit(ccc_exit);

MODULE_AUTHOR("Gerald Lejeune <gerald.lejeune@st.com>");
MODULE_DESCRIPTION("ST C3 Channel Controller bus driver");
MODULE_LICENSE("GPL");
