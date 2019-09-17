/*
 * sta_rpmsg_rdm.c
 * RPMSG interface for Remote Device Manager on STA SoC
 *
 * Copyright (C) 2018 STMicroelectronics.
 *
 * Author: Seraphin Bonnaffe <seraphin.bonnaffe@st.com> for STMicroelectronics
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/of_device.h>
#include <linux/rproc_srm_sta_dev.h>
#include "rpmsg_internal.h"

/* Start of common definitions */

#define RDM_MAX_DEV_NAME_SZ		25
#define RDM_RPMSG_BUF_SIZE		200

enum rpmsg_rdm_types {
	RDM_CONNECT = 0,
	RDM_REQUEST_DEV,
	RDM_RELEASE_DEV,
	RDM_ERROR,
};

struct rpmsg_rdm_hdr {
	u32 type;
	u32 len;
	int data[0];
} __packed;

enum rpmsg_rdm_cn_status {
	RDM_CN_REQ = 0,
	RDM_CN_ACK,
	RDM_CN_NACK,
};

struct rpmsg_rdm_cn {
	u32 status;
	int data[0];
} __packed;

struct rpmsg_rdm_release {
	char dev_name[0];
} __packed;

/* End of common definitions */

enum rpmsg_rdm_states {
	RDM_UNCONNECTED = 0,
	RDM_CONNECTED,
	RDM_STATE_ERROR,
};

static int rdm_internal_state = RDM_UNCONNECTED;

static int rdm_cb(struct rpmsg_device *rpdev, void *data, int len,
		  void *priv, u32 src)
{
	struct rpmsg_rdm_hdr *hdr = data;
	struct rpmsg_rdm_cn *cn;
	struct rpmsg_rdm_release *rel;
	int ret;

	switch (hdr->type) {
	case RDM_CONNECT:
		cn = (struct rpmsg_rdm_cn *)hdr->data;
		if (cn->status == RDM_CN_ACK) {
			rdm_internal_state = RDM_CONNECTED;
			dev_info(&rpdev->dev, "RDM service connected !\n");
		} else if (cn->status == RDM_CN_NACK)  {
			rdm_internal_state = RDM_UNCONNECTED;
			dev_err(&rpdev->dev, "RDM connection issue\n");
		} else {
			rdm_internal_state = RDM_STATE_ERROR;
			dev_err(&rpdev->dev, "Rx RDM message not supported\n");
		}
		break;
	case RDM_REQUEST_DEV:
	case RDM_RELEASE_DEV:
		rel = (struct rpmsg_rdm_release *)hdr->data;
		dev_dbg(&rpdev->dev, "rel->dev_name: %s\n",
			(char *)rel->dev_name);
		ret = rproc_srm_sta_dev_enable((char *)rel->dev_name, true);
		break;
	case RDM_ERROR:
	default:
		break;
	}

	return 0;
}

static int rdm_probe(struct rpmsg_device *rpdev)
{
	char buf[RDM_RPMSG_BUF_SIZE] = {0};
	struct rpmsg_rdm_hdr *hdr = (struct rpmsg_rdm_hdr *)buf;
	struct rpmsg_rdm_cn *cn = (struct rpmsg_rdm_cn *)hdr->data;
	int ret;

	/* Prepare Connect request with the list of available devices */
	cn->status = RDM_CN_REQ;
	ret = rproc_rdm_get_devices((char *)&cn->data,
				    RDM_RPMSG_BUF_SIZE - sizeof(*cn));
	hdr->type = RDM_CONNECT;
	hdr->len = sizeof(*cn) + ret + 1;

	ret = rpmsg_send(rpdev->ept, buf, sizeof(*hdr) + hdr->len);
	if (ret)
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);

	return ret;
}

void rdm_remove(struct rpmsg_device *rpdev)
{
}

static struct rpmsg_device_id rdm_id_table[] = {
	{ .name	= "rpmsg-rdm" },
	{ },
};
MODULE_DEVICE_TABLE(platform, stm_rpmsg_id_table);

/* RPMSG-RDM: Remote Device Manager */
static struct rpmsg_driver rdm_driver = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= rdm_id_table,
	.probe		= rdm_probe,
	.remove		= rdm_remove,
	.callback	= rdm_cb,
};

static int __init rdm_init(void)
{
	return register_rpmsg_driver(&rdm_driver);
}
module_init(rdm_init);

static void __exit rdm_exit(void)
{
	unregister_rpmsg_driver(&rdm_driver);
}
module_exit(rdm_exit);

MODULE_AUTHOR("Seraphin Bonnaffe <seraphin.bonnaffe@st.com>");
MODULE_DESCRIPTION("STA Remote Device Manager driver");
MODULE_ALIAS("rpmsg-rdm");
