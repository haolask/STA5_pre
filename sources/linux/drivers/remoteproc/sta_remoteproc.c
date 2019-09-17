/*
 * STMicroelectronics STA Remote Processor driver
 *
 * Copyright (C) 2014 STMicroelectronics.
 *
 * Author: Olivier Lebreton <olivier.lebreton@st.com> for STMicroelectronics
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/remoteproc.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox_sta.h>
#include <linux/mutex.h>
#include <linux/rpmsg.h>
#include <linux/virtio_ids.h>

#include "sta_remoteproc.h"
#include "remoteproc_internal.h"

#define RSC_TABLE_SIZE	0x1000
#define RSC_TABLE_VER	1

#define RSC_VDEV_ALIGN		4

/**
 * struct sta_rproc_attribute - STA rproc device attribute info
 * @type:	mailbox device type: legacy or new IP
 * @fw_ops:	maximum number of mailbox supported by the HW IP
 */
struct sta_rproc_attribute {
	int				type;
	const struct rproc_fw_ops	*fw_ops;
	struct sta_rproc_resources	*resources;
};

/**
 * struct sta_rproc_data - sta remote processor data
 * @name:	the remoteproc's name
 * @mbox_name:	name of mailbox device to use with this rproc
 * @firmware:	name of firmware file to load
 * @nbrsc:	number of resource entries
 * @rsctablesz: resource table size
 * @rsctable:	resource entry tables
 * @rsctablepa:	physical address of the resource table
 */
struct sta_rproc_data {
	const char		*name;
	const char		*mbox_name;
	const char		*firmware;
	int			nbrsc;
	int			rsctablesz;
	struct resource_table	*rsctable;
	struct phys_addr_t	*rsctablepa;
};

/**
 * struct sta_rproc - sta remote processor state
 * @rproc:		rproc handle
 * @mbchan:		mailbox channels
 * @mb_client_vq0:	mailbox client for virtqueue 0
 * @sdata:		STA remote processor data
 * @scfg:		STA rproc device attribute info
 */
struct sta_rproc {
	struct rproc			*rproc;
	struct mbox_chan		*mb_chan;
	struct mbox_client		mb_client_vq0;
	struct sta_rproc_data		*sdata;
	struct sta_rproc_attribute	*scfg;
	/* protect 'initialized' from concurrent access */
	struct mutex			init_lock;
	bool				initialized;
	struct work_struct		restart_work;
};

/**
 * struct sta_rproc_resources - STA rproc resource entry
 * @rsc_read:	function pointer to handle the resource entry
 * @da:		device address of the resource
 * @pa:		physical address to map
 * @len:	memory size of the resource
 * @flags:	iommu protection flags
 * @rsc_nb:	number of the current resource entry type
 */
struct sta_rproc_resources {
	int (*rsc_read)(struct device *dev,
			struct sta_rproc_resources *rsc,
			struct sta_rproc_data *sdata, int *offset, int *idx);
	u32 da;
	u32 pa;
	u32 len;
	u32 flags;
	char name[32];
	int rsc_nb;
};

/**
 * sta_rproc_mbox_callback() - inbound mailbox message handler
 * @context: context pointer passed during registrations
 * @data: mailbox payload
 *
 * This handler is invoked by sta's mailbox driver whenever a mailbox
 * message is received. Usually, the mailbox payload simply contains
 * the index of the virtqueue that is kicked by the remote processor,
 * and we let remoteproc core handle it.
 *
 * In addition to virtqueue indices, we also have some out-of-band values
 * that indicates different events. Those values are deliberately very
 * big so they don't coincide with virtqueue indices.
 */
static void sta_rproc_mbox_callback(struct mbox_client *mb_client, void *data)
{
	struct sta_mbox_msg *msg = (struct sta_mbox_msg *)data;
	struct sta_rproc *sproc = container_of(mb_client, struct sta_rproc,
					       mb_client_vq0);
	struct device *dev = sproc->rproc->dev.parent;
	const char *name = sproc->rproc->name;
	u32 *vqid = (u32 *)msg->pdata;

	if (msg->dsize > 4) {
		/* unexpected message length */
		dev_warn(dev,
			 "receive unexpected msg length from sta rproc %s\n",
			 name);
		return;
	}
	dev_dbg(dev, "mbox msg: 0x%x\n", *vqid);

	switch (*vqid) {
	case RP_MBOX_CRASH:
		/* just log this for now. later, we'll also do recovery */
		dev_err(dev, "sta rproc %s crashed\n", name);
		break;
	case RP_MBOX_INIT_ACK:
		dev_info(dev, "received ack init from %s\n", name);
		break;
	case RP_MBOX_INIT_FAILED:
		dev_info(dev, "received init failure from %s\n", name);
		break;
	default:
		/* msg contains the index of the triggered vring */
		if (rproc_vq_interrupt(sproc->rproc, *vqid) == IRQ_NONE)
			dev_dbg(dev, "no message found in vqid %d\n", *vqid);
	}
}

/* kick a virtqueue */
static void sta_rproc_kick(struct rproc *rproc, int vqid)
{
	struct sta_rproc *sproc = rproc->priv;
	struct device *dev = rproc->dev.parent;
	struct sta_rproc_data *sdata = sproc->sdata;
	struct sta_mbox_msg msg;
	u32 data[2];
	int ret;

	mutex_lock(&sproc->init_lock);

	if (!sproc->initialized) {
		/*
		 * For the first kick only:
		 * Now the Virtio VRING has been created and so the Remote
		 * Processor can be notified to start its RPMSG framework
		 * initialisation.
		 * In order to synchronise both processors, the HOST sends the
		 * Resource Table address filled with the Virtio Device
		 * parameters.
		 *
		 * Note that the reply will only arrive when the RPMSG
		 * framework init on remote processor is completed.
		 */
		msg.dsize = (u8)sizeof(data);
		data[0] = RP_MBOX_INIT_REQUEST;
		data[1] = (u32)sdata->rsctablepa;
		msg.pdata = (u8 *)data;

		ret = mbox_send_message(sproc->mb_chan, (void *)&msg);
		if (ret < 0)
			dev_err(dev, "%s: mbox send failed: %d\n",
				__func__, ret);

		/* Send init only once */
		sproc->initialized = true;
	}

	mutex_unlock(&sproc->init_lock);

	msg.dsize = (u8)sizeof(vqid);
	msg.pdata = (u8 *)&vqid;

	/* send the index of the triggered virtqueue in the mailbox payload */
	ret = mbox_send_message(sproc->mb_chan, (void *)&msg);
	if (ret < 0)
		dev_err(dev, "sta_mbox_msg_send failed: %d\n", ret);
}

/*
 * Power up the remote processor.
 *
 * This function will be invoked only after the firmware for this rproc
 * was loaded, parsed successfully, and all of its resource requirements
 * were met.
 */
static int sta_rproc_start(struct rproc *rproc)
{
	struct device *dev = rproc->dev.parent;

	/*
	 * Nothing to do.
	 * The RPMsg buffers may not be allocated yet at this point.
	 * The resource table still has to be updated with their address.
	 * So, wait for the first 'kick' before sending RP_MBOX_INIT_REQUEST
	 * together with the resource table address.
	 */

	dev_info(dev, "Starting remote processor...\n");
	return 0;
}

/* power off the remote processor */
static int sta_rproc_stop(struct rproc *rproc)
{
	struct sta_rproc *sproc = rproc->priv;

	mbox_free_channel(sproc->mb_chan);
	mutex_lock(&sproc->init_lock);
	sproc->initialized = false;
	mutex_unlock(&sproc->init_lock);

	return 0;
}

static struct rproc_ops sta_rproc_ops = {
	.start		= sta_rproc_start,
	.stop		= sta_rproc_stop,
	.kick		= sta_rproc_kick,
};

/**
 * sta_rproc_find_rsc_table() - find the resource table
 * @rproc: the rproc handle
 * @fw: the ELF firmware image: MUST equal to NULL
 * @tablesz: place holder for providing back the table size
 *
 * This function finds the resource table inside the STA remote processor.
 * The resource table must be define via DT entries to be compliant with
 * this handler operation.
 *
 * Returns the pointer to the resource table if it is found, and write its
 * size into @tablesz. If a valid table isn't found, NULL is returned
 * (and @tablesz isn't set).
 */
static struct resource_table *
sta_rproc_find_rsc_table(struct rproc *rproc, const struct firmware *fw,
			 int *tablesz)
{
	struct sta_rproc *sproc = rproc->priv;
	struct resource_table *table;

	table = sproc->sdata->rsctable;
	*tablesz = sproc->sdata->rsctablesz;

	return table;
}

/* Find the resource table inside the STA remote processor */
static struct resource_table *
sta_rproc_find_loaded_rsc_table(struct rproc *rproc, const struct firmware *fw)
{
	struct sta_rproc *sproc = rproc->priv;
	struct resource_table *table;

	table = sproc->sdata->rsctable;

	return table;
}

/* Loads the firmware to shared memory. */
static int
sta_rproc_load_segments(struct rproc *rproc, const struct firmware *fw)
{
	return 0;
}

/* STA remoteproc firmware handler operations */
static const struct rproc_fw_ops sta_rproc_fw_ops = {
	.load = sta_rproc_load_segments,
	.find_rsc_table = sta_rproc_find_rsc_table,
	.find_loaded_rsc_table = sta_rproc_find_loaded_rsc_table,
};

/* Set remoteproc resource table from DT config */
static int sta_rproc_fill_table_header(struct device *dev,
				       struct resource_table *table, int idx,
				       int offset, int rsclen, int nbmax)
{
	/* Update resource table header */
	if (++table->num > nbmax) {
		dev_err(dev, "Nb of rsc entries [%d/%d] out of range\n",
			table->num, nbmax);
		return -EINVAL;
	}
	if (offset + rsclen > RSC_TABLE_SIZE) {
		dev_err(dev, "Size of rsc table [%d/%d] out of range\n",
			offset + rsclen, RSC_TABLE_SIZE);
		return -EINVAL;
	}
	table->offset[idx] = offset;

	return 0;
}

/* Handle Vdev resource entry */
static int sta_rproc_handle_vdev(struct device *dev,
				 struct sta_rproc_resources *rsc,
				 struct sta_rproc_data *sdata,
				 int *offset, int *idx)
{
	struct resource_table *table = sdata->rsctable;
	struct fw_rsc_hdr *hdr;
	struct fw_rsc_vdev *vdev;
	struct fw_rsc_vdev_vring *vring;
	int j, rsclen;

	hdr = (void *)table + *offset;
	vdev = (void *)hdr + sizeof(*hdr);

	/* Virtio device header */
	hdr->type = RSC_VDEV;
	vdev->id = VIRTIO_ID_RPMSG;
	vdev->dfeatures = rsc->flags;
	vdev->config_len = rsc->len;
	vdev->num_of_vrings = RVDEV_NUM_VRINGS;

	/* Vring descriptor entry */
	vring = (void *)vdev + sizeof(*vdev);
	for (j = 0; j < vdev->num_of_vrings; j++) {
		vring->align = RSC_VDEV_ALIGN;
		vring->num = MAX_RPMSG_NUM_BUFS / RVDEV_NUM_VRINGS;
		vring++;
	}

	rsclen = sizeof(*hdr) + sizeof(*vdev)
		+ vdev->num_of_vrings * sizeof(*vring)
		+ vdev->config_len;
	if (sta_rproc_fill_table_header(dev, table, *idx, *offset,
					rsclen, sdata->nbrsc)) {
		dev_err(dev, "Vdev rsc update out of range\n");
		return -EINVAL;
	}

	*offset += rsclen;
	(*idx)++;

	return 0;
}

/* Handle Trace resource entry */
static int sta_rproc_handle_trace(struct device *dev,
				  struct sta_rproc_resources *rsc,
				  struct sta_rproc_data *sdata,
				  int *offset, int *idx)
{
	struct resource_table *table = sdata->rsctable;
	struct fw_rsc_hdr *hdr;
	struct fw_rsc_trace *trace;
	int rsclen;

	rsclen = sizeof(*hdr) + sizeof(*trace);
	if (sta_rproc_fill_table_header(dev, table, *idx, *offset,
					rsclen, sdata->nbrsc)) {
		dev_err(dev, "Trace rsc update out of range\n");
		return -EINVAL;
	}
	hdr = (void *)table + *offset;
	trace = (void *)hdr + sizeof(*hdr);

	hdr->type = RSC_TRACE;
	trace->da = rsc->da;
	trace->len = rsc->len;
	snprintf(trace->name, 32, "%s", rsc->name);

	*offset += rsclen;
	(*idx)++;

	return 0;
}

/* Handle Carveout resource entry */
static int sta_rproc_handle_carveout(struct device *dev,
				     struct sta_rproc_resources *rsc,
				     struct sta_rproc_data *sdata,
				     int *offset, int *idx)
{
	struct resource_table *table = sdata->rsctable;
	struct fw_rsc_hdr *hdr;
	struct fw_rsc_carveout *carveout;
	int rsclen;

	rsclen = sizeof(*hdr) + sizeof(*carveout);
		if (sta_rproc_fill_table_header(dev, table, *idx, *offset,
						rsclen, sdata->nbrsc)) {
			dev_err(dev, "Carveout rsc update out of range\n");
			return -EINVAL;
		}
		hdr = (void *)table + *offset;
		carveout = (void *)hdr + sizeof(*hdr);

		hdr->type = RSC_CARVEOUT;
		carveout->pa = rsc->pa;
		carveout->da = rsc->da;
		carveout->len = rsc->len;
		carveout->flags = rsc->flags;

		snprintf(carveout->name, 32, "%s", rsc->name);

		*offset += rsclen;
		(*idx)++;
	return 0;
}

/*
 * A lookup table for resource handlers. The indices are defined in
 * enum fw_resource_type.
 */
static struct sta_rproc_resources sta_rproc_rsc_handlers[RSC_LAST] = {
	[RSC_CARVEOUT] = {
		.rsc_read = sta_rproc_handle_carveout,
		.pa = 0,
		.da = FW_RSC_ADDR_ANY,
		.len = 0x10000,
		.flags = 0,
		.name = "rp-cvout0",
		.rsc_nb = 1
	},
	[RSC_DEVMEM] = {
		.rsc_read = NULL,
		.rsc_nb = 0
	},
	[RSC_TRACE] = {
		.rsc_read = sta_rproc_handle_trace,
		.da = FW_RSC_ADDR_ANY,
		.len = 0xf000,
		.name = "rp-trace0",
		.rsc_nb = 1
	},
	[RSC_VDEV] = {
		.rsc_read = sta_rproc_handle_vdev,
		.len = 0,
		.flags = 1,
		.name = "rp-vdev0",
		.rsc_nb = 1
	}
};

/* Extract remoteproc config from DT and set resource table accordingly */
static int sta_rproc_get_dt_config(struct device *dev,
				   struct sta_rproc_data *sdata)
{
	struct device_node *np = dev->of_node;

	if (of_property_read_string(np, "st,rp-name", &sdata->name)) {
		dev_err(dev, "rp-name read failed\n");
		return -ENODEV;
	}

	if (of_property_read_string(np, "st,rp-fw-name", &sdata->firmware)) {
		dev_dbg(dev, "No firmware to download\n");
		sdata->firmware = FW_RSC_NOT_REQUIRED;
	}
	return 0;
}

static int sta_rproc_set_rsc_table(struct device *dev,
				   struct sta_rproc_data *sdata,
				   struct sta_rproc_attribute *cfg)
{
	struct sta_rproc_resources *rsc;
	dma_addr_t dma;
	int ret, i, offset, idx;

	/* Look for number of resource entries in DT */
	for (i = 0; i < RSC_LAST; i++) {
		rsc = &cfg->resources[i];
		sdata->nbrsc += rsc->rsc_nb;

	}
	/* Build resource table if resourse entry found in DT */
	if (sdata->nbrsc > 0) {
		/*
		 * Allocate one page non-cacheable memory for the rsc table
		 */
		sdata->rsctable = dma_zalloc_coherent(dev,
					RSC_TABLE_SIZE, &dma, GFP_KERNEL);
		if (!sdata->rsctable) {
			dev_err(dev, "dma_alloc_coherent failed\n");
			return -ENOMEM;
		}
		sdata->rsctablepa = (void *)dma;

		/* 1st offset just after resource table header */
		offset = sizeof(*sdata->rsctable)
			+ sdata->nbrsc * sizeof(*sdata->rsctable->offset);

		/*** Fill resource table ***/
		sdata->rsctable->ver = RSC_TABLE_VER;
		idx = 0;

		/* Handle resource entries from DT */
		for (i = 0; i < RSC_LAST; i++) {
			rsc = &cfg->resources[i];

			if (rsc->rsc_nb && rsc->rsc_read) {
				ret = rsc->rsc_read(dev, rsc, sdata,
							&offset, &idx);
				if (ret) {
					dev_err(dev, "rsc_read failed\n");
					return ret;
				}
			}
		}
		/* Save resource table size */
		sdata->rsctablesz = offset;
	}
	return 0;
}

static const struct sta_rproc_attribute sproc_config0 = {
	.type		= 0,
	.fw_ops		= &sta_rproc_fw_ops,
	.resources	= sta_rproc_rsc_handlers,
};

static const struct of_device_id sta_rproc_match[] = {
	{ .compatible = "st,sta-rproc",
		.data = (void *)&sproc_config0,
	},
	{ /* sentinel */}
};

static void sta_restart_proc(struct work_struct *work)
{
	struct sta_rproc *sproc = container_of(work, struct sta_rproc,
					       restart_work);
	struct rproc *rproc = sproc->rproc;
	struct sta_rproc_data *sdata;

	if ((!rproc) || (!rproc->priv))  {
		pr_err("rproc struct from drvdata is damaged");
		return;
	}

	sdata = sproc->sdata;
	rproc->table_ptr = sdata->rsctable;

	sproc->mb_chan = mbox_request_channel(&sproc->mb_client_vq0, 0);
	if (IS_ERR(sproc->mb_chan)) {
		dev_err(sproc->mb_client_vq0.dev,
			"mbox_request_channel failed: %d\n", -EBUSY);
		return;
	}

	rproc_boot(rproc);
}

static int sta_rproc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct sta_rproc_data *sdata;
	struct sta_rproc_attribute *cfg;
	struct sta_rproc *sproc;
	struct rproc *rproc;
	int ret;

	if (!np) {
		/* No DT device creation */
		dev_err(dev, "%s: platform not supported\n", __func__);
		return -ENODEV;
	}

	ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(32));
	if (ret) {
		dev_err(dev, "dma_set_coherent_mask: %d\n", ret);
		return ret;
	}

	/* Get remoteproc config from DT */
	sdata = devm_kzalloc(dev, sizeof(*sdata), GFP_KERNEL);
	if (!sdata)
		return -ENOMEM;
	ret = sta_rproc_get_dt_config(dev, sdata);
	if (ret) {
		dev_err(dev, "rproc get DT config failed: %d\n", ret);
		return -ENODEV;
	}

	cfg = (struct sta_rproc_attribute *)of_match_device(
				sta_rproc_match, &pdev->dev)->data;
	if (!cfg) {
		/* No rproc configuration */
		dev_err(dev, "No configuration found\n");
		return -ENODEV;
	}
	ret = sta_rproc_set_rsc_table(dev, sdata, cfg);
	if (ret) {
		dev_err(dev, "rproc set resource table failed: %d\n", ret);
		return ret;
	}

	rproc = rproc_alloc(dev, sdata->name, &sta_rproc_ops,
			    sdata->firmware, sizeof(*sproc));
	if (!rproc)
		return -ENOMEM;

	/* Set the STA remoteproc specific firmware handler */
	rproc->fw_ops = cfg->fw_ops;

	/* Store resource table in rproc if already fetched from DT */
	/* Will avoid firmware loading to retrieve it later on */
	rproc->table_ptr = sdata->rsctable;

	sproc = rproc->priv;
	sproc->rproc = rproc;
	sproc->sdata = sdata;
	sproc->scfg = cfg;
	mutex_init(&sproc->init_lock);
	mutex_lock(&sproc->init_lock);
	sproc->initialized = false;
	mutex_unlock(&sproc->init_lock);
	INIT_WORK(&sproc->restart_work, sta_restart_proc);

	/* Request a mailbox channel for RPMSG */
	sproc->mb_client_vq0.dev = dev;
	sproc->mb_client_vq0.tx_block = true;
	sproc->mb_client_vq0.tx_tout = 1000;
	sproc->mb_client_vq0.knows_txdone = false;
	sproc->mb_client_vq0.rx_callback = sta_rproc_mbox_callback;
	sproc->mb_client_vq0.tx_done = NULL;

	sproc->mb_chan = mbox_request_channel(&sproc->mb_client_vq0, 0);
	if (IS_ERR(sproc->mb_chan)) {
		ret = -EBUSY;
		dev_err(dev, "mbox_request_channel failed: %d\n", ret);
		return ret;
	}

	dev_set_drvdata(&pdev->dev, rproc);

	ret = rproc_add(rproc);
	if (ret)
		goto free_rproc;

	return 0;

free_rproc:
	rproc_free(rproc);
	return ret;
}

static int sta_rproc_remove(struct platform_device *pdev)
{
	struct rproc *rproc = dev_get_drvdata(&pdev->dev);
	struct sta_rproc *sproc = rproc->priv;

	cancel_work_sync(&sproc->restart_work);
	rproc_del(rproc);
	rproc_free(rproc);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int sta_rproc_suspend(struct device *dev)
{
	struct rproc *rproc = dev_get_drvdata(dev);

	rproc_shutdown(rproc);

	return 0;
}

static int sta_rproc_resume(struct device *dev)
{
	struct rproc *rproc = dev_get_drvdata(dev);
	struct sta_rproc *sproc = rproc->priv;

	schedule_work(&sproc->restart_work);

	return 0;
}
#endif

static const struct dev_pm_ops sta_rproc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sta_rproc_suspend, sta_rproc_resume)
};

static struct platform_driver sta_rproc_driver = {
	.probe = sta_rproc_probe,
	.remove = sta_rproc_remove,
	.driver = {
		.name = "sta-rproc",
		.owner = THIS_MODULE,
		.of_match_table = sta_rproc_match,
		.pm = &sta_rproc_pm_ops,
	},
};

module_platform_driver(sta_rproc_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("STMicroelectronics STA Remote Processor control driver");
