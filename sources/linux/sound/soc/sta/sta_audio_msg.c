/*
 * Remote processor messaging - audio client driver
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Brian Swetland <swetland@google.com>
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
#include <linux/kfifo.h>
#include "sta_audio_msg.h"

struct msg_audio_hdl {
	u32 msg_id;
	msg_audio_cb_t cb;
	void *arg;
};

static struct audio_host {
	struct rpmsg_device *rpdev;
	struct msg_audio_hdl hdl[10];
} audio_host;

static DEFINE_MUTEX(msg_audio_lock);
static DEFINE_KFIFO(send_fifo, struct audio_data, 4);

static int rpmsg_audio_cb(struct rpmsg_device *rpdev, void *data, int len,
			  void *priv, u32 src)
{
	struct audio_data *audio_data = (struct audio_data *)data;
	int i;

	dev_dbg(&rpdev->dev, "incoming msg %d (src: 0x%x)\n", len, src);

	for (i = 0; i < ARRAY_SIZE(audio_host.hdl); i++) {
		if (audio_host.hdl[i].msg_id == audio_data->msg_id &&
		    audio_host.hdl[i].cb)
			audio_host.hdl[i].cb(audio_data, audio_host.hdl[i].arg);
	}

	return 0;
}

static int st_codec_early_audio_send_data(struct audio_data *audio_data,
					  int len)
{
	struct rpmsg_device *rpdev = audio_host.rpdev;
	int ret;

	mutex_lock(&msg_audio_lock);

	if (!rpdev) {
		kfifo_put(&send_fifo, *audio_data); /* deferred */
		mutex_unlock(&msg_audio_lock);
		return 0;
	}

	len += sizeof(audio_data->msg_id);
	ret = rpmsg_send(rpdev->ept, (void *)audio_data, len);
	if (ret) {
		mutex_unlock(&msg_audio_lock);
		dev_err(&rpdev->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}
	mutex_unlock(&msg_audio_lock);
	return 0;
}

int st_codec_early_audio_send(enum audio_msg msg_id)
{
	struct audio_data audio_data;

	audio_data.msg_id = msg_id;
	return st_codec_early_audio_send_data(&audio_data, 0);
}
EXPORT_SYMBOL(st_codec_early_audio_send);

int st_codec_early_audio_send_args(enum audio_msg msg_id,
				   char *name, u32 value, u32 id)
{
	struct audio_data audio_data;
	int len;

	len = sizeof(audio_data) - sizeof(audio_data.desc) + strlen(name) + 1;
	audio_data.msg_id = msg_id;
	audio_data.id = id;
	strlcpy(audio_data.desc, name, sizeof(audio_data.desc));
	audio_data.arg = value;
	return st_codec_early_audio_send_data(&audio_data, len);
}
EXPORT_SYMBOL(st_codec_early_audio_send_args);

int msg_audio_register(u32 msg_id, msg_audio_cb_t cb, void *arg)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(audio_host.hdl); i++) {
		if (!audio_host.hdl[i].cb) {
			audio_host.hdl[i].cb = cb;
			audio_host.hdl[i].msg_id = msg_id;
			audio_host.hdl[i].arg = arg;
			return 0;
		}
	}
	return -EBUSY;
}
EXPORT_SYMBOL(msg_audio_register);

static int rpmsg_audio_probe(struct rpmsg_device *rpdev)
{
	struct audio_data audio_data;

	dev_dbg(&rpdev->dev, "new rpmsg channel: 0x%x -> 0x%x!\n",
		rpdev->src, rpdev->dst);

	mutex_lock(&msg_audio_lock);
	audio_host.rpdev = rpdev;
	mutex_unlock(&msg_audio_lock);

	while (kfifo_get(&send_fifo, &audio_data))
		st_codec_early_audio_send(audio_data.msg_id);

	return 0;
}

static void rpmsg_audio_remove(struct rpmsg_device *rpdev)
{
	dev_dbg(&rpdev->dev, "rpmsg audio client driver is removed\n");
}

static struct rpmsg_device_id rpmsg_driver_audio_id_table[] = {
	{ .name	= "early-audio" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, rpmsg_driver_audio_id_table);

static struct rpmsg_driver rpmsg_audio_client = {
	.drv.name	= KBUILD_MODNAME,
	.id_table	= rpmsg_driver_audio_id_table,
	.probe		= rpmsg_audio_probe,
	.callback	= rpmsg_audio_cb,
	.remove		= rpmsg_audio_remove,
};
module_rpmsg_driver(rpmsg_audio_client);

MODULE_DESCRIPTION("rpmsg audio client driver");
MODULE_LICENSE("GPL v2");
