/*
 * STA micro-controller SubSystem (uCSS) remote processor messaging client
 *
 * Copyright (C) 2014 STMicroelectronics.
 *
 * Author: Olivier Lebreton <olivier.lebreton@st.com> for STMicroelectronics
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
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>

#include "rpmsg_internal.h"
#include "sta_rpmsg.h"

/* maximum CSS devices this driver can handle */
#define MAX_CSS_DEVICES		8

enum {
	CSS_SERVICE_DOWN,
	CSS_SERVICE_UP
};

struct sta_rpmsg {
	struct list_head next;
	struct cdev cdev;
	struct device *dev;
	struct rpmsg_device *rpdev;
	int minor;
	struct list_head list;
	/* protect list access */
	struct mutex lock;
	struct completion comp;
	int state;
};

struct sta_rpmsg_instance {
	struct list_head next;
	struct sta_rpmsg *srpm;
	struct sk_buff_head queue;
	struct sk_buff *cur_skb;
	int cur_skb_bytes_read;
	/* protect read queue access */
	struct mutex lock;
	wait_queue_head_t readq;
	struct completion reply_arrived;
	struct rpmsg_endpoint *ept;
	u32 dst;
	int state;
};

static struct class *sta_rpmsg_class;
static dev_t sta_rpmsg_cdev;

/* store all remote CSS connection services (usually one per remoteproc) */
static DEFINE_IDR(sta_rpmsg_css);
static DEFINE_SPINLOCK(sta_rpmsg_css_lock);
static LIST_HEAD(sta_rpmsg_css_list);

static int sta_rpmsg_cb(struct rpmsg_device *rpdev,
			void *data, int len, void *priv, u32 src)
{
	struct css_msg_hdr *hdr = data;
	struct sta_rpmsg_instance *css = priv;
	struct css_ctrl_rsp *rsp;
	struct sk_buff *skb;
	char *skbdata;

	if (len < sizeof(*hdr) || hdr->len < len - sizeof(*hdr)) {
		dev_warn(&rpdev->dev, "%s: truncated message\n", __func__);
		return -EINVAL;
	}

	dev_dbg(&rpdev->dev,
		"%s: incoming msg src 0x%x type %d len %d\n",
		__func__, src, hdr->type, hdr->len);

	print_hex_dump_debug("sta_rpmsg RX: ",
			     DUMP_PREFIX_NONE, 16, 1, data, len,  true);

	switch (hdr->type) {
	case CSS_CTRL_RSP:
		if (hdr->len < sizeof(*rsp)) {
			dev_warn(&rpdev->dev, "incoming empty response msg\n");
			return -EINVAL;
		}
		rsp = (struct css_ctrl_rsp *)hdr->data;
		dev_info(&rpdev->dev,
			 "conn rsp: status %d addr %d\n",
			 rsp->status, rsp->addr);
		if (rsp->status != CSS_SUCCESS)
			css->state = CSS_FAILED;
		else
			css->state = CSS_CONNECTED;
		complete(&css->reply_arrived);
		break;
	case CSS_RAW_MSG:
	case CSS_CAN_RX:
	case CSS_ACC_RX:
		skb = alloc_skb(hdr->len, GFP_KERNEL);
		if (!skb)
			return -ENOMEM;

		skbdata = skb_put(skb, hdr->len);
		memcpy(skbdata, hdr->data, hdr->len);

		mutex_lock(&css->lock);
		skb_queue_tail(&css->queue, skb);
		mutex_unlock(&css->lock);
		/* wake up any blocking processes, waiting for new data */
		wake_up_interruptible(&css->readq);
		break;
	default:
		dev_warn(&rpdev->dev, "unexpected msg type: %d\n", hdr->type);
		return -EINVAL;
	}

	return 0;
}

static int sta_rpmsg_listen(struct sta_rpmsg_instance *css, u32 type)
{
	struct sta_rpmsg *srpm = css->srpm;
	struct css_msg_hdr *hdr;
	u8 listen_msg[sizeof(*hdr)] = { 0 };
	int ret;

	hdr = (struct css_msg_hdr *)listen_msg;
	hdr->type = type;
	hdr->len = 0;

	/*
	 * Send a listen req to the remote CSS server.
	 * Use the local address that was just allocated by ->open
	 */
	ret = rpmsg_send(css->ept, listen_msg, sizeof(listen_msg));
	if (ret) {
		dev_err(srpm->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}
	return 0;
}

static int sta_rpmsg_control(struct sta_rpmsg_instance *css, void *data)
{
	struct sta_rpmsg *srpm = css->srpm;
	struct css_msg_hdr *hdr;
	struct css_ctrl_req *ctrl_req;
	u8 ctrl_msg[sizeof(*hdr) + sizeof(*ctrl_req)] = { 0 };
	int ret;

	hdr = (struct css_msg_hdr *)ctrl_msg;
	hdr->type = CSS_CTRL_REQ;
	hdr->len = sizeof(*ctrl_req);
	ctrl_req = (struct css_ctrl_req *)hdr->data;
	memcpy((void *)ctrl_req, data, sizeof(*ctrl_req));

	init_completion(&css->reply_arrived);

	/*
	 * Send a control req to the remote CSS server.
	 * Use the local address that was just allocated by ->open
	 */
	ret = rpmsg_send(css->ept, ctrl_msg, sizeof(ctrl_msg));
	if (ret) {
		dev_err(srpm->dev, "rpmsg_send failed: %d\n", ret);
		return ret;
	}

	/* wait until a connection reply arrives or 5 seconds elapse */
	ret = wait_for_completion_interruptible_timeout(&css->reply_arrived,
							msecs_to_jiffies(5000));
	if (css->state == CSS_CONNECTED)
		return 0;

	if (css->state == CSS_FAILED)
		return -ENXIO;

	if (ret) {
		dev_err(srpm->dev, "Premature wakeup: %d\n", ret);
		return -EIO;
	}

	return -ETIMEDOUT;
}

static
long sta_rpmsg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct sta_rpmsg_instance *css = filp->private_data;
	struct sta_rpmsg *srpm = css->srpm;
	struct css_ctrl_req data;
	int ret = 0;

	dev_dbg(srpm->dev, "%s: cmd %d, arg 0x%lx\n", __func__, cmd, arg);

	if (_IOC_TYPE(cmd) != CSS_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > CSS_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
	case CSS_IOCCONTROL:
		ret = copy_from_user(&data, (char __user *)arg, sizeof(data));
		if (ret) {
			dev_err(srpm->dev,
				"%s: %d: copy_from_user fail: %d\n", __func__,
				_IOC_NR(cmd), ret);
			ret = -EFAULT;
			break;
		}
		ret = sta_rpmsg_control(css, &data);
		break;
	case CSS_IOCCANLISTEN:
		ret = sta_rpmsg_listen(css, CSS_CAN_LISTEN);
		break;
	case CSS_IOCACCLISTEN:
		ret = sta_rpmsg_listen(css, CSS_ACC_LISTEN);
		break;
	default:
		dev_warn(srpm->dev, "unhandled ioctl cmd: %d\n", cmd);
		break;
	}

	return ret;
}

static int sta_rpmsg_open(struct inode *inode, struct file *filp)
{
	struct sta_rpmsg *srpm;
	struct sta_rpmsg_instance *css;
	struct rpmsg_channel_info chinfo = {};
	struct css_ctrl_req data;
	int ret;

	srpm = container_of(inode->i_cdev, struct sta_rpmsg, cdev);

	if (srpm->state == CSS_SERVICE_DOWN)
		if (filp->f_flags & O_NONBLOCK ||
		    wait_for_completion_interruptible(&srpm->comp))
			return -EBUSY;

	css = kzalloc(sizeof(*css), GFP_KERNEL);
	if (!css)
		return -ENOMEM;

	mutex_init(&css->lock);
	skb_queue_head_init(&css->queue);
	init_waitqueue_head(&css->readq);
	css->srpm = srpm;
	css->state = CSS_UNCONNECTED;

	/* Assign a new, unique, local address and associate CSS with it */
	strncpy(chinfo.name, srpm->rpdev->id.name, RPMSG_NAME_SIZE);
	chinfo.src = RPMSG_ADDR_ANY;
	chinfo.dst = srpm->rpdev->dst;

	css->ept = rpmsg_create_ept(srpm->rpdev, sta_rpmsg_cb, css, chinfo);
	if (!css->ept) {
		dev_err(srpm->dev, "create ept failed\n");
		kfree(css);
		return -ENOMEM;
	}
	/*
	 * Reuse dst address assciated to Rpmsg channel on which the current
	 * Endpoint has been attached on
	 */
	css->dst = srpm->rpdev->dst;

	/* Associate filp with the new css instance */
	filp->private_data = css;
	mutex_lock(&srpm->lock);
	list_add(&css->next, &srpm->list);
	mutex_unlock(&srpm->lock);

	/* Send control message to inform CSS */
	data.request = CSS_CONNECTION;
	data.addr = css->dst;
	ret = sta_rpmsg_control(css, &data);
	if (ret) {
		dev_err(srpm->dev, "Connection request error: %d\n", ret);
		kfree(css);
		return -EIO;
	}

	dev_info(srpm->dev, "local addr assigned: 0x%x\n", css->ept->addr);

	return 0;
}

static int sta_rpmsg_release(struct inode *inode, struct file *filp)
{
	struct sta_rpmsg_instance *css = filp->private_data;
	struct sta_rpmsg *srpm = css->srpm;
	struct css_ctrl_req data;
	int ret;

	/* todo: release resources here */
	/*
	 * If state == fail, remote processor crashed, so don't send it
	 * any message.
	 */
	if (css->state == CSS_FAILED)
		goto out;

	/* Send a disconnect msg with the CSS instance addr */
	data.request = CSS_DISCONNECT;
	data.addr = css->dst;
	ret = sta_rpmsg_control(css, &data);
	if (ret) {
		dev_err(srpm->dev, "Connection request error: %d\n", ret);
		return ret;
	}

	dev_info(srpm->dev, "Disconnecting from CSS service at %d\n", css->dst);
out:
	rpmsg_destroy_ept(css->ept);
	mutex_lock(&srpm->lock);
	list_del(&css->next);
	mutex_unlock(&srpm->lock);
	kfree(css);

	return 0;
}

static ssize_t sta_rpmsg_read(struct file *filp, char __user *buf,
			      size_t len, loff_t *offp)
{
	struct sta_rpmsg_instance *css = filp->private_data;
	int use;

	if (mutex_lock_interruptible(&css->lock))
		return -ERESTARTSYS;

	if (css->state == CSS_FAILED) {
		mutex_unlock(&css->lock);
		return -ENXIO;
	}

	if (css->state != CSS_CONNECTED) {
		mutex_unlock(&css->lock);
		return -ENOTCONN;
	}

	/**
	 * check if there is a pending skb that
	 * would not be fully read by user
	 */
	if (!css->cur_skb) {
		/* nothing to read ? */
		if (skb_queue_empty(&css->queue)) {
			mutex_unlock(&css->lock);
			/* non-blocking requested ? return now */
			if (filp->f_flags & O_NONBLOCK)
				return -EAGAIN;
			/* otherwise block, and wait for data */
			if (wait_event_interruptible(css->readq,
						     (!skb_queue_empty(&css->queue) ||
						      css->state == CSS_FAILED)))
				return -ERESTARTSYS;
			if (mutex_lock_interruptible(&css->lock))
				return -ERESTARTSYS;
		}

		if (css->state == CSS_FAILED) {
			mutex_unlock(&css->lock);
			return -ENXIO;
		}

		css->cur_skb = skb_dequeue(&css->queue);
		if (!css->cur_skb) {
			mutex_unlock(&css->lock);
			dev_err(css->srpm->dev, "err is rpmsg_css racy ?\n");
			return -EIO;
		}
	}

	use = min(len, css->cur_skb->len - css->cur_skb_bytes_read);

	if (copy_to_user(buf,
			 css->cur_skb->data + css->cur_skb_bytes_read, use)) {
		dev_err(css->srpm->dev, "%s: copy_to_user fail\n", __func__);
		use = -EFAULT;
	}

	css->cur_skb_bytes_read += use;

	if (css->cur_skb->len == css->cur_skb_bytes_read) {
		kfree_skb(css->cur_skb);
		css->cur_skb = NULL;
		css->cur_skb_bytes_read = 0;
	}
	mutex_unlock(&css->lock);
	return use;
}

static ssize_t sta_rpmsg_write(struct file *filp, const char __user *ubuf,
			       size_t len, loff_t *offp)
{
	struct sta_rpmsg_instance *css = filp->private_data;
	struct sta_rpmsg *srpm = css->srpm;
	struct css_msg_hdr *hdr;
	char *kbuf;
	int use, ret, max_size;

	if (css->state != CSS_CONNECTED)
		return -ENOTCONN;

	max_size = RPMSG_BUF_SIZE - rpmsg_sizeof_hdr() -
		   sizeof(struct css_msg_hdr);

	if (len > max_size) {
		dev_err(srpm->dev, "Message too big (%d). Max size is %d\n",
			len, max_size);
		return -EMSGSIZE;
	}

	use = len + sizeof(*hdr);
	kbuf = kzalloc(use, GFP_KERNEL);
	if (!kbuf) {
		dev_err(srpm->dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	hdr = (struct css_msg_hdr *)kbuf;
	/*
	 * copy the data.
	 */
	if (copy_from_user(hdr->data, ubuf, len)) {
		ret = -EFAULT;
		goto err;
	}

	hdr->type = CSS_RAW_MSG;
	hdr->len = len;

	ret = rpmsg_send(css->ept, kbuf, use);
	if (ret) {
		dev_err(srpm->dev, "rpmsg_send failed: %d\n", ret);
		goto err;
	}

	ret = use;

err:
	kfree(kbuf);
	return ret;
}

static
unsigned int sta_rpmsg_poll(struct file *filp, struct poll_table_struct *wait)
{
	struct sta_rpmsg_instance *css = filp->private_data;
	unsigned int mask = 0;

	if (mutex_lock_interruptible(&css->lock))
		return -ERESTARTSYS;

	poll_wait(filp, &css->readq, wait);
	if (css->state == CSS_FAILED) {
		mutex_unlock(&css->lock);
		return -ENXIO;
	}

	if (!skb_queue_empty(&css->queue))
		mask |= POLLIN | POLLRDNORM;

	/* implement missing rpmsg virtio functionality here */
	if (true)
		mask |= POLLOUT | POLLWRNORM;

	mutex_unlock(&css->lock);

	return mask;
}

static const struct file_operations sta_rpmsg_fops = {
	.open		= sta_rpmsg_open,
	.release	= sta_rpmsg_release,
	.unlocked_ioctl	= sta_rpmsg_ioctl,
	.read		= sta_rpmsg_read,
	.write		= sta_rpmsg_write,
	.poll		= sta_rpmsg_poll,
	.owner		= THIS_MODULE,
};

static int sta_rpmsg_probe(struct rpmsg_device *rpdev)
{
	int ret, major, minor;
	struct sta_rpmsg *srpm = NULL, *tmp;

	/* dynamically assign a new minor number */
	spin_lock(&sta_rpmsg_css_lock);
	minor = idr_alloc(&sta_rpmsg_css, srpm, 0, MAX_CSS_DEVICES, GFP_KERNEL);
	if (minor < 0) {
		spin_unlock(&sta_rpmsg_css_lock);
		dev_err(&rpdev->dev, "idr_alloc failed: %d\n", minor);
		return minor;
	}

	/* look for an already created css service */
	list_for_each_entry(tmp, &sta_rpmsg_css_list, next) {
		if (tmp->minor == minor) {
			srpm = tmp;
			idr_replace(&sta_rpmsg_css, srpm, minor);
			break;
		}
	}
	spin_unlock(&sta_rpmsg_css_lock);
	if (srpm)
		goto out;

	srpm = devm_kzalloc(&rpdev->dev, sizeof(*srpm), GFP_KERNEL);
	if (!srpm) {
		ret = -ENOMEM;
		goto free_idr;
	}

	spin_lock(&sta_rpmsg_css_lock);
	idr_replace(&sta_rpmsg_css, srpm, minor);
	spin_unlock(&sta_rpmsg_css_lock);
	INIT_LIST_HEAD(&srpm->list);
	mutex_init(&srpm->lock);
	init_completion(&srpm->comp);

	list_add(&srpm->next, &sta_rpmsg_css_list);

	major = MAJOR(sta_rpmsg_cdev);

	cdev_init(&srpm->cdev, &sta_rpmsg_fops);
	srpm->cdev.owner = THIS_MODULE;
	ret = cdev_add(&srpm->cdev, MKDEV(major, minor), 1);
	if (ret) {
		dev_err(&rpdev->dev, "cdev_add failed: %d\n", ret);
		goto free_idr;
	}

	srpm->dev = device_create(sta_rpmsg_class, &rpdev->dev,
				MKDEV(major, minor), NULL,
				"rpmsg_css%d", minor);
	if (IS_ERR(srpm->dev)) {
		ret = PTR_ERR(srpm->dev);
		dev_err(&rpdev->dev, "device_create failed: %d\n", ret);
		goto clean_cdev;
	}
out:
	srpm->rpdev = rpdev;
	srpm->minor = minor;
	srpm->state = CSS_SERVICE_UP;
	dev_set_drvdata(&rpdev->dev, srpm);
	complete_all(&srpm->comp);

	dev_info(srpm->dev,
		 "new CSS connection srv channel: %u -> %u!\n",
		 rpdev->src, rpdev->dst);
	return 0;

clean_cdev:
	cdev_del(&srpm->cdev);
free_idr:
	spin_lock(&sta_rpmsg_css_lock);
	idr_remove(&sta_rpmsg_css, minor);
	spin_unlock(&sta_rpmsg_css_lock);
	return ret;
}

static void sta_rpmsg_remove(struct rpmsg_device *rpdev)
{
	struct sta_rpmsg *srpm = dev_get_drvdata(&rpdev->dev);
	int major = MAJOR(sta_rpmsg_cdev);
	struct sta_rpmsg_instance *css;

	dev_info(srpm->dev, "rpmsg css driver is removed\n");

	spin_lock(&sta_rpmsg_css_lock);
	idr_remove(&sta_rpmsg_css, srpm->minor);
	spin_unlock(&sta_rpmsg_css_lock);

	mutex_lock(&srpm->lock);
	/* If there is css instances that means it is a revovery. */
	if (list_empty(&srpm->list)) {
		device_destroy(sta_rpmsg_class, MKDEV(major, srpm->minor));
		cdev_del(&srpm->cdev);
		list_del(&srpm->next);
		mutex_unlock(&srpm->lock);
		return;
	}
	/* If it is a recovery, don't clean the srpm */
	init_completion(&srpm->comp);
	srpm->state = CSS_SERVICE_DOWN;
	list_for_each_entry(css, &srpm->list, next) {
		/* set css instance to fail state */
		css->state = CSS_FAILED;
		/* unblock any pending css thread*/
		complete_all(&css->reply_arrived);
		wake_up_interruptible(&css->readq);
	}
	mutex_unlock(&srpm->lock);
}

static int sta_rpmsg_driver_cb(struct rpmsg_device *rpdev,
			       void *data, int len, void *priv, u32 src)
{
	/*
	 * Default EndPoint created at driver probing should not be directly
	 * used by application.
	 * A dedicated ept with its unique local address is created for each
	 * CSS instance opened
	 */
	dev_warn(&rpdev->dev, "unexpected message for default ept\n");

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
		       data, len,  true);
	return -ENXIO;
}

static struct rpmsg_device_id sta_rpmsg_id_table[] = {
	{ .name	= "sta-rpmsg-CSS" },
	{ },
};
MODULE_DEVICE_TABLE(platform, sta_rpmsg_id_table);

static struct rpmsg_driver sta_rpmsg_driver = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= sta_rpmsg_id_table,
	.probe		= sta_rpmsg_probe,
	.callback	= sta_rpmsg_driver_cb,
	.remove		= sta_rpmsg_remove,
};

static int __init init(void)
{
	int ret;

	ret = alloc_chrdev_region(&sta_rpmsg_cdev,
				  0, MAX_CSS_DEVICES, KBUILD_MODNAME);
	if (ret) {
		pr_err("alloc_chrdev_region failed: %d\n", ret);
		goto out;
	}

	sta_rpmsg_class = class_create(THIS_MODULE, KBUILD_MODNAME);
	if (IS_ERR(sta_rpmsg_class)) {
		ret = PTR_ERR(sta_rpmsg_class);
		pr_err("class_create failed: %d\n", ret);
		goto unreg_region;
	}

	return register_rpmsg_driver(&sta_rpmsg_driver);

unreg_region:
	unregister_chrdev_region(sta_rpmsg_cdev, MAX_CSS_DEVICES);
out:
	return ret;
}
module_init(init);

static void __exit fini(void)
{
	struct sta_rpmsg *srpm, *tmp;
	int major = MAJOR(sta_rpmsg_cdev);

	unregister_rpmsg_driver(&sta_rpmsg_driver);
	list_for_each_entry_safe(srpm, tmp, &sta_rpmsg_css_list, next) {
		device_destroy(sta_rpmsg_class, MKDEV(major, srpm->minor));
		cdev_del(&srpm->cdev);
		list_del(&srpm->next);
	}
	class_destroy(sta_rpmsg_class);
	unregister_chrdev_region(sta_rpmsg_cdev, MAX_CSS_DEVICES);
}
module_exit(fini);

MODULE_DESCRIPTION("STMicroelectroncis STA rpmsg driver");
MODULE_LICENSE("GPL v2");
