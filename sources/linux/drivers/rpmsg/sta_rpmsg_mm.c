/**
 * @file sta_rpmsg_mm.c
 * @brief RPMsg communication between M3 and A7 for multimedia tasks
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/idr.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/rpmsg.h>
#include <linux/completion.h>
#include <linux/jiffies.h>
#include "rpmsg_internal.h"
#include "sta_rpmsg_mm.h"

struct sta_rpmsg_mm_instance {
	struct list_head next;
	struct sta_rpmsg_mm_ctx *srpm;
	struct rpmsg_endpoint *ept;
	u32 dst;
	int state;
	int instance_type;
	char name[MAX_NAME_LENGTH];
	char shortname[MAX_NAME_LENGTH];

	/* Info used with RPMSG_MM_APPLI instance */
	struct sk_buff_head queue;
	struct sk_buff *cur_skb;
	int cur_skb_bytes_read;
	struct mutex lock; /* mutex to lock */
	wait_queue_head_t readq;
	struct completion reply_arrived;

	/* Info used with RPMSG_MM_DRIVER instance */
	t_pfct_rpmsg_mm_cb p_driver_callback;
	void *priv;
};

struct sta_rpmsg_mm_ctx {
	struct list_head next;
	struct cdev cdev;
	struct device *dev;
	struct rpmsg_device *rpdev;
	int minor;
	struct list_head list;
	/* protect list access */
	struct mutex lock;
	/* Used to protect rpmsg exchange */
	struct mutex exchange_lock;
	int state;
};

struct rpmsg_ack_info {
	u32 status;
	struct completion wait_for_ack;
};

struct rpmsg_mm_payload {
	char emitter[MAX_NAME_LENGTH];
	/*!< String used to described the message emitter */
	char receiver[MAX_NAME_LENGTH];
	/*!< String used to described the message receiver */
	void *reserved;
	/*!< Reserved data used by rpmsg_mm */
	struct rpmsg_mm_msg_hdr user_data;
	/* Nothing after...*/
};

struct rpmsg_mm_probe {
	struct completion wait_for_finalization;
	bool ended;
};

static struct rpmsg_mm_probe *rpmsg_mm_probe_status;
static DEFINE_MUTEX(rpmsg_mm_lock_probe);

static struct class *sta_rpmsg_mm_class;
static dev_t sta_rpmsg_mm_cdev;
static struct sta_rpmsg_mm_ctx *rpmsg_mm_ctx;

static int sta_rpmsg_mm_base_cb(struct rpmsg_device *rpdev,
				void *data, int len, void *priv, u32 src)
{
	/*
	 * Default EndPoint created at driver probing should not be directly
	 * used by application.
	 * A dedicated ept with its unique local address is created for each
	 * MM instance opened
	 */
	dev_warn(&rpdev->dev, "unexpected message for default ept\n");

	print_hex_dump(KERN_DEBUG, __func__, DUMP_PREFIX_NONE, 16, 1,
		       data, len,  true);
	return -ENXIO;
}

static struct rpmsg_device_id sta_rpmsg_mm_id_table[] = {
	{ .name = "service_mm" },
	{ },
};
MODULE_DEVICE_TABLE(platform, sta_rpmsg_mm_id_table);

static int sta_rpmsg_mm_send_message(void *handle,
				     char *receiver,
				     struct rpmsg_mm_msg_hdr *user_data,
				     void *reserved,
				     bool ack_requested)
{
	struct sta_rpmsg_mm_instance *mm;
	struct sta_rpmsg_mm_ctx *srpm;
	struct rpmsg_mm_payload *payload_data = NULL;
	struct rpmsg_mm_msg_hdr *payload_user = NULL;
	struct rpmsg_ack_info *ack = NULL;
	int ret = 0;
	int paysize = 0;

	mm = (struct sta_rpmsg_mm_instance *)handle;
	if (!mm)
		return -1;

	if (!mm->ept)
		return -1;

	if (!user_data)
		return -1;

	srpm = mm->srpm;

	paysize = sizeof(struct rpmsg_mm_payload) + user_data->len;
	if (paysize > MAX_PAYLOAD_SIZE) {
		dev_warn(srpm->dev, "Invalid payload size\n");
		return -1;
	}

	payload_data = kzalloc(paysize, GFP_KERNEL);
	if (!payload_data)
		return -1;

	payload_user = &payload_data->user_data;

	strncpy(payload_data->emitter, mm->name, MAX_NAME_LENGTH);
	strncpy(payload_data->receiver, receiver, MAX_NAME_LENGTH);
	payload_user->info = RPMSG_MM_RESET_ACK_INFO(user_data->info);

	if (user_data->data) {
		/* User wants to transfer additional data */
		memcpy(&payload_user->data[0], user_data->data, user_data->len);
		payload_user->len = user_data->len;
	}

	if (ack_requested) {
		ack = kzalloc(sizeof(*ack), GFP_KERNEL);
		if (!ack) {
			ret = -1;
			goto go_out;
		}
		ack->status = 0;
		init_completion(&ack->wait_for_ack);
		payload_user->info = RPMSG_MM_SET_ACK_INFO(payload_user->info);
		payload_data->reserved = (void *)ack;
	} else {
		payload_data->reserved = reserved;
	}
	mutex_lock(&srpm->exchange_lock);

	ret = rpmsg_send(mm->ept, payload_data, paysize);
	if (ret)
		dev_err(srpm->dev, "rpmsg_send failed: %d\n", ret);

	mutex_unlock(&srpm->exchange_lock);

	if (ack_requested) {
		if (!ret) {
			if (!wait_for_completion_timeout(
					&ack->wait_for_ack,
					msecs_to_jiffies(5000))) {
				dev_err(srpm->dev, "[RPMSG] : Timeout!!!\n");
				ret = -1;
			} else {
				ret = ack->status;
			}
		}
		kfree(ack);
	}
go_out:
	kfree(payload_data);
	return ret;
}

static int sta_rpmsg_mm_transfer_service_info(
			struct sta_rpmsg_mm_instance *mm,
			int service, int len, void *data)
{
	struct sta_rpmsg_mm_ctx *srpm;
	int ret = 0;
	int llen = len + 1; // to send the service itself in data
	struct rpmsg_mm_msg_hdr *user_data = NULL;

	srpm = mm->srpm;

	user_data = kzalloc(sizeof(*user_data) + llen, GFP_KERNEL);
	if (!user_data)
		return -1;

	user_data->info = RPMSG_MM_USER_SERVICE;
	user_data->len = llen;
	user_data->data[0] = service;
	/* Copy data associated to the service if any */
	if (len && data)
		memcpy(&user_data->data[1], data, len);

	ret = sta_rpmsg_mm_send_message((void *)mm, RPSMG_MM_EPT_CORTEXM_MM,
					user_data, NULL, false);
	if (ret)
		dev_err(srpm->dev, "rpmsg_send failed: %d\n", ret);
	kfree(user_data);

	return ret;
}

static ssize_t sta_rpmsg_mm_write(struct file *filp, const char __user *ubuf,
				  size_t len, loff_t *offp)
{
	struct sta_rpmsg_mm_instance *mm = filp->private_data;
	char buffer[MAX_PAYLOAD_SIZE];
	void *pdata = NULL;

	if (len <= 0)
		return -EMSGSIZE;

	if (!mm)
		return -EINVAL;

	if (copy_from_user(buffer, ubuf, len))
		return -EFAULT;

	pdata = (len > 1 ? &buffer[1] : NULL);
	if (sta_rpmsg_mm_transfer_service_info(mm, buffer[0],
					       len - 1, pdata) >= 0)
		return len;

	return -EFAULT;
}

int sta_rpmsg_get_remote_resources_usage(void *handle)
{
	struct sta_rpmsg_mm_instance *mm;
	struct rpmsg_mm_msg_hdr request;
	int res = -1;

	mm = (struct sta_rpmsg_mm_instance *)handle;
	if (!mm)
		return -1;

	memset(&request, 0, sizeof(request));
	request.info = RPMSG_MM_SHARED_RES_STATUS_REQ;

	res = sta_rpmsg_mm_send_message(handle, RPSMG_MM_EPT_CORTEXM_MM,
					&request, NULL, true);
	return res;
}
EXPORT_SYMBOL(sta_rpmsg_get_remote_resources_usage);

int sta_rpmsg_mm_get_reargear_status(void *handle)
{
	struct sta_rpmsg_mm_instance *mm;
	struct rpmsg_mm_msg_hdr request;
	int res = -1;

	mm = (struct sta_rpmsg_mm_instance *)handle;
	if (!mm)
		return -1;

	memset(&request, 0, sizeof(request));
	request.info = RPMSG_MM_REARGEAR_STATUS_REQ;

	res = sta_rpmsg_mm_send_message(handle, RPSMG_MM_EPT_CORTEXM_RVC,
					&request, NULL, true);
	return res;
}

static
long sta_rpmsg_mm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct sta_rpmsg_mm_instance *mm = filp->private_data;
	struct sta_rpmsg_mm_ctx *srpm = mm->srpm;
	int ret = 0;
	enum rpmsg_mm_rvc_debug_mode mode;

	if (_IOC_TYPE(cmd) != RPMSG_MM_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > RPMSG_MM_IOC_MAXNR)
		return -ENOTTY;

	switch (cmd) {
	case RPMSG_MM_IOC_GET_RG_STATUS:
		if (_IOC_DIR(cmd) != _IOC_READ)
			return -EINVAL;

		ret = sta_rpmsg_mm_get_reargear_status((void *)mm);
		if (ret < 0)
			return -EFAULT;

		if (copy_to_user((void __user *)arg,
				 &ret, sizeof(int)))
			return -EFAULT;
		break;
	case RPMSG_MM_IOC_STOP_ANIM:
		ret = sta_rpmsg_mm_transfer_service_info(
					mm, RPMSG_MM_SERV_STOP_ANIM,
					0, NULL);
		break;
	case RPMSG_MM_IOC_START_RVC:
		ret = sta_rpmsg_mm_transfer_service_info(
					mm, RPMSG_MM_SERV_START_RVC,
					0, NULL);
		break;
	case RPMSG_MM_IOC_STOP_RVC:
		ret = sta_rpmsg_mm_transfer_service_info(
					mm, RPMSG_MM_SERV_STOP_RVC,
					0, NULL);
		break;
	case RPMSG_MM_IOC_RVC_DEBUG_MODE:
		ret = copy_from_user(&mode, (void __user *)arg, sizeof(int));
		if (ret)
			return -EFAULT;

		ret = sta_rpmsg_mm_transfer_service_info(
					mm, RPMSG_MM_SERV_RVC_DEBUG_MODE,
					sizeof(mode), &mode);
		break;
	case RPMSG_MM_IOC_RVC_AX_TAKEOVER_REQ:
		ret = sta_rpmsg_mm_transfer_service_info(
					mm, RPMSG_MM_SERV_RVC_AX_TAKEOVER,
					0, NULL);
		break;
	default:
		dev_warn(srpm->dev, "unhandled ioctl cmd: 0x%x\n",
			 (unsigned int)cmd);
		break;
	}
	return ret;
}

static unsigned int sta_rpmsg_mm_poll(struct file *filp,
				      struct poll_table_struct *wait)
{
	struct sta_rpmsg_mm_instance *mm = filp->private_data;
	unsigned int mask = 0;

	if (mutex_lock_interruptible(&mm->lock))
		return -ERESTARTSYS;

	poll_wait(filp, &mm->readq, wait);

	if (!skb_queue_empty(&mm->queue))
		mask |= POLLIN | POLLRDNORM;

	/* implement missing rpmsg virtio functionality here */
	if (true)
		mask |= POLLOUT | POLLWRNORM;

	mutex_unlock(&mm->lock);

	return mask;
}

static ssize_t sta_rpmsg_mm_read(struct file *filp, char __user *buf,
				 size_t len, loff_t *offp)
{
	struct sta_rpmsg_mm_instance *mm = filp->private_data;
	int use;

	if (mutex_lock_interruptible(&mm->lock))
		return -ERESTARTSYS;

	/**
	 * check if there is a pending skb that
	 * would not be fully read by user
	 */
	if (!mm->cur_skb) {
		/* nothing to read ? */
		if (skb_queue_empty(&mm->queue)) {
			mutex_unlock(&mm->lock);
			/* non-blocking requested ? return now */
			if (filp->f_flags & O_NONBLOCK)
				return -EAGAIN;
			/* otherwise block, and wait for data */
			if (wait_event_interruptible(
				mm->readq,
				(!skb_queue_empty(&mm->queue))))
				return -ERESTARTSYS;
			if (mutex_lock_interruptible(&mm->lock))
				return -ERESTARTSYS;
		}

		mm->cur_skb = skb_dequeue(&mm->queue);
		if (!mm->cur_skb) {
			mutex_unlock(&mm->lock);
			dev_err(mm->srpm->dev, "err is rpmsg_mm racy ?\n");
			return -EIO;
		}
	}

	use = min(len, mm->cur_skb->len - mm->cur_skb_bytes_read);

	if (copy_to_user(buf,
			 mm->cur_skb->data + mm->cur_skb_bytes_read, use)) {
		dev_err(mm->srpm->dev, "%s: copy_to_user fail\n", __func__);
		use = -EFAULT;
	}

	mm->cur_skb_bytes_read += use;

	if (mm->cur_skb->len == mm->cur_skb_bytes_read) {
		kfree_skb(mm->cur_skb);
		mm->cur_skb = NULL;
		mm->cur_skb_bytes_read = 0;
	}
	mutex_unlock(&mm->lock);
	return use;
}

int sta_rpmsg_send_private_message(void *handle, char *remote_endpoint,
				   void *data, int len)
{
	struct sta_rpmsg_mm_instance *mm;
	struct sta_rpmsg_mm_ctx *srpm;
	int ret = 0;
	struct rpmsg_mm_msg_hdr *user_data = NULL;

	mm = (struct sta_rpmsg_mm_instance *)handle;
	if (!mm)
		return -1;
	srpm = mm->srpm;

	user_data = kzalloc(sizeof(*user_data) + len, GFP_KERNEL);
	if (!user_data)
		return -1;

	user_data->len = len;
	memcpy(user_data->data, data, user_data->len);
	user_data->info = RPMSG_MM_PRIVATE_MESSAGE;

	ret = sta_rpmsg_mm_send_message(handle, remote_endpoint,
					user_data, NULL, false);
	if (ret)
		dev_err(srpm->dev, "rpmsg_send failed: %d\n", ret);
	kfree(user_data);

	return ret;
}
EXPORT_SYMBOL(sta_rpmsg_send_private_message);

static int sta_rpmsg_mm_ept_cb(struct rpmsg_device *rpdev, void *data,
			       int len, void *priv, u32 src)
{
	struct rpmsg_mm_msg_hdr *user_data = NULL;
	struct rpmsg_mm_payload *payload_data = NULL;
	struct sta_rpmsg_mm_instance *mm = priv;
	struct sk_buff *skb;
	unsigned char *skbdata;
	bool answer_requested = false;

	payload_data = (struct rpmsg_mm_payload *)data;

	if (!payload_data) {
		dev_warn(&rpdev->dev, "%s: No data !!!\n", __func__);
		return -EINVAL;
	}

	user_data = &payload_data->user_data;
	if (len < sizeof(*payload_data) ||
	    len < (sizeof(*payload_data) + user_data->len)) {
		dev_warn(&rpdev->dev,
			 "%s:truncated msg (len:%d,userdata:%d,payload:%d\n",
			 __func__, len, user_data->len,
			 sizeof(*payload_data));
		return -EINVAL;
	}

	if (!mm || mm->instance_type > RPMSG_MM_DRIVER)
		return -EINVAL;

	/* In case of requested ACK, rpmsg_mm returns it to emitter */
	answer_requested = RPMSG_MM_IS_ACK_REQUESTED(user_data->info);
	user_data->info = RPMSG_MM_RESET_ACK_INFO(user_data->info);

	dev_dbg(&rpdev->dev, "Message %d received from %s to %s\n",
		user_data->info, payload_data->emitter, payload_data->receiver);

	switch (user_data->info) {
	case RPMSG_MM_REARGEAR_STATUS_ACK:
	case RPMSG_MM_REGISTRATION_ACK:
	case RPMSG_MM_UNREGISTRATION_ACK:
	case RPMSG_MM_SHARED_RES_STATUS_ACK:
	{
		struct rpmsg_ack_info *ack_info;

		ack_info = (struct rpmsg_ack_info *)payload_data->reserved;
		if (ack_info && user_data->len == sizeof(u32)) {
			u32 *pstatus = (u32 *)&user_data->data[0];

			ack_info->status = *pstatus;
			complete(&ack_info->wait_for_ack);
		}
		break;
	}
	default:
		break;
	}

	if (mm->instance_type == RPMSG_MM_APPLI) {
		u32 data2copy;

		dev_dbg(&rpdev->dev, "Store data in queue to user-land\n");
		/* Data written into the pipe are just those included in
		 * rpmsg_mm_msg_hdr structure (including data)
		 */
		data2copy = sizeof(struct rpmsg_mm_msg_hdr) + user_data->len;

		skb = alloc_skb(data2copy, GFP_KERNEL);
		if (!skb)
			return -ENOMEM;

		skbdata = skb_put(skb, data2copy);
		memcpy(skbdata, (unsigned char *)user_data, data2copy);

		mutex_lock(&mm->lock);
		skb_queue_tail(&mm->queue, skb);
		mutex_unlock(&mm->lock);
		/* wake up any blocking processes, waiting for new data */
		wake_up_interruptible(&mm->readq);
	} else {
		dev_dbg(&rpdev->dev, "Transfer data to registered driver\n");
		if (mm->p_driver_callback)
			mm->p_driver_callback(
				(struct rpmsg_mm_msg_hdr *)user_data,
				mm->priv);
	}

	if (!answer_requested)
		return 0;

	/* Treat cases where an answer is requested */
	if (user_data->info == RPMSG_MM_SHARED_RES_LOCKED ||
	    user_data->info == RPMSG_MM_SHARED_RES_UNLOCKED) {
		char buffer[MAX_PAYLOAD_SIZE];
		struct rpmsg_mm_msg_hdr *pdata;

		pdata = (struct rpmsg_mm_msg_hdr *)buffer;
		pdata->info = RPMSG_MM_SHARED_RES_LOCKED_ACK;
		pdata->len = 0;
		sta_rpmsg_mm_send_message((void *)mm,
					  payload_data->emitter,
					  pdata,
					  payload_data->reserved,
					  false);
	} else {
		dev_warn(&rpdev->dev,
			 "!!! Answer not treated (message %d from %s)\n",
			 user_data->info, payload_data->emitter);
	}
	return 0;
}

/* Service used to inform registered driver about communication
 * status btw M3 and A7
 */
static void sta_rpmsg_mm_transfer_comm_status(int status,
					      struct sta_rpmsg_mm_instance *mm)
{
	if (!mm)
		return;

	if (mm->instance_type == RPMSG_MM_DRIVER && mm->p_driver_callback) {
		char buffer[MAX_PAYLOAD_SIZE];
		struct rpmsg_mm_msg_hdr *pdata;

		pdata = (struct rpmsg_mm_msg_hdr *)buffer;
		memset(pdata, 0, sizeof(*pdata));
		pdata->info = status;
		pdata->len = 0;
		mm->p_driver_callback((struct rpmsg_mm_msg_hdr *)pdata,
				      mm->priv);
	}
}

static int sta_rpmsg_mm_create_endpoint(struct sta_rpmsg_mm_instance *mm)
{
	struct rpmsg_channel_info chinfo = {};
	struct sta_rpmsg_mm_ctx *srpm = mm->srpm;

	strncpy(chinfo.name, srpm->rpdev->id.name, RPMSG_NAME_SIZE);
	chinfo.src = RPMSG_ADDR_ANY;
	chinfo.dst = srpm->rpdev->dst;

	mm->ept = rpmsg_create_ept(srpm->rpdev, sta_rpmsg_mm_ept_cb,
				   mm, chinfo);
	if (!mm->ept) {
		dev_err(srpm->dev, "create ept failed\n");
		return -1;
	}

	/* Concatenate "A7_MM" with endpoint address
	 * to have a uniq reference
	 */
	if (mm->instance_type == RPMSG_MM_APPLI)
		snprintf(mm->name, MAX_NAME_LENGTH, "%s_%d",
			 mm->shortname, mm->ept->addr);

	dev_dbg(srpm->dev, "New endpoint created : %s [%d]\n",
		mm->name, mm->ept->addr);

	/*
	 * Reuse dst address assciated to Rpmsg channel on which the current
	 * Endpoint has been attached on
	 */
	mm->dst = srpm->rpdev->dst;
	return 0;
}

static struct sta_rpmsg_mm_instance *sta_rpmsg_mm_create_instance(
				struct sta_rpmsg_mm_ctx *srpm,
				char *endpoint_name,
				int instance_type,
				t_pfct_rpmsg_mm_cb driver_callback,
				void *driver_priv)
{
	struct sta_rpmsg_mm_instance *mm;
	struct rpmsg_mm_msg_hdr request;

	mm = kzalloc(sizeof(*mm), GFP_KERNEL);
	if (!mm)
		return NULL;

	mutex_init(&mm->lock);
	skb_queue_head_init(&mm->queue);
	init_waitqueue_head(&mm->readq);
	mm->srpm = srpm;
	mm->instance_type = instance_type;
	if (instance_type == RPMSG_MM_DRIVER) {
		mm->p_driver_callback = driver_callback;
		mm->priv = driver_priv;
	}
	strncpy(mm->shortname, endpoint_name, MAX_NAME_LENGTH);
	strncpy(mm->name, endpoint_name, MAX_NAME_LENGTH);

	if (sta_rpmsg_mm_create_endpoint(mm) < 0) {
		dev_err(srpm->dev, "Unable to create endpoint\n");
		kfree(mm);
		return NULL;
	}

	mutex_lock(&srpm->lock);
	list_add(&mm->next, &srpm->list);
	mutex_unlock(&srpm->lock);

	/* Declare this instance on remote core side */
	memset(&request, 0, sizeof(request));
	request.info = RPMSG_MM_REGISTRATION_REQ;

	if (sta_rpmsg_mm_send_message((void *)mm, RPSMG_MM_EPT_CORTEXM_MM,
				      &request, NULL, true) < 0) {
		dev_err(srpm->dev, "Unable to register remotely\n");
		if (mm->ept)
			rpmsg_destroy_ept(mm->ept);

		mutex_lock(&srpm->lock);
		list_del(&mm->next);
		mutex_unlock(&srpm->lock);

		kfree(mm);
		return NULL;
	}
	return mm;
}

static int sta_rpmsg_mm_open(struct inode *inode, struct file *filp)
{
	struct sta_rpmsg_mm_ctx *srpm;
	struct sta_rpmsg_mm_instance *mm;

	srpm = container_of(inode->i_cdev, struct sta_rpmsg_mm_ctx, cdev);

	mm = sta_rpmsg_mm_create_instance(srpm, RPSMG_MM_EPT_CORTEXA_MM,
					  RPMSG_MM_APPLI, NULL, NULL);
	if (!mm)
		return -ENOMEM;

	/* Associate filp with the mm instance */
	filp->private_data = mm;
	return 0;
}

int sta_rpmsg_unregister_endpoint(void *handle)
{
	struct sta_rpmsg_mm_instance *mm;
	struct sta_rpmsg_mm_ctx *srpm;
	struct rpmsg_mm_msg_hdr request;

	if (!handle)
		return -1;

	mm = (struct sta_rpmsg_mm_instance *)handle;
	srpm = mm->srpm;

	/* REMOVE this instance on remote core side */
	memset(&request, 0, sizeof(request));
	request.info = RPMSG_MM_UNREGISTRATION_REQ;

	if (sta_rpmsg_mm_send_message(handle, RPSMG_MM_EPT_CORTEXM_MM,
				      &request, NULL, true) < 0)
		dev_warn(srpm->dev, "Unregistration failed remotely\n");

	if (mm->ept)
		rpmsg_destroy_ept(mm->ept);

	mutex_lock(&srpm->lock);
	list_del(&mm->next);

	mutex_unlock(&srpm->lock);

	dev_dbg(srpm->dev, "Endpoint deleted : %s [%d]\n",
		mm->name, mm->ept->addr);
	kfree(mm);
	return 0;
}
EXPORT_SYMBOL(sta_rpmsg_unregister_endpoint);

static int sta_rpmsg_mm_finalize_probe(void)
{
	mutex_lock(&rpmsg_mm_lock_probe);
	if (rpmsg_mm_probe_status) {
		rpmsg_mm_probe_status->ended = true;
		complete_all(&rpmsg_mm_probe_status->wait_for_finalization);
	} else {
		rpmsg_mm_probe_status = kzalloc(sizeof(*rpmsg_mm_probe_status),
						GFP_KERNEL);
		init_completion(&rpmsg_mm_probe_status->wait_for_finalization);
		rpmsg_mm_probe_status->ended = true;
	}
	mutex_unlock(&rpmsg_mm_lock_probe);
	return 0;
}

static int sta_rpmsg_mm_wait_for_end_of_probe(void)
{
	mutex_lock(&rpmsg_mm_lock_probe);
	if (!rpmsg_mm_probe_status) {
		rpmsg_mm_probe_status = kzalloc(sizeof(*rpmsg_mm_probe_status),
						GFP_KERNEL);
		init_completion(&rpmsg_mm_probe_status->wait_for_finalization);
		rpmsg_mm_probe_status->ended = false;
	}

	if (rpmsg_mm_probe_status->ended) {
		mutex_unlock(&rpmsg_mm_lock_probe);
		return 0;
	}
	mutex_unlock(&rpmsg_mm_lock_probe);
	if (!wait_for_completion_timeout(
		&rpmsg_mm_probe_status->wait_for_finalization,
		msecs_to_jiffies(5000))) {
		/* Timeout after 5sec, consider the
		 * resource as available to avoid freeze
		 */
		dev_warn(NULL, "%s TIMEOUT!!!\n", __func__);
		return -1;
	}
	return 0;
}

void *sta_rpmsg_register_endpoint(char *endpoint_name,
				  t_pfct_rpmsg_mm_cb callback,
				  void *priv)
{
	if (sta_rpmsg_mm_wait_for_end_of_probe() < 0)
		return NULL;
	return sta_rpmsg_mm_create_instance(rpmsg_mm_ctx, endpoint_name,
				       RPMSG_MM_DRIVER,
				       callback, priv);
}
EXPORT_SYMBOL(sta_rpmsg_register_endpoint);

static int sta_rpmsg_mm_release(struct inode *inode, struct file *filp)
{
	return sta_rpmsg_unregister_endpoint((void *)filp->private_data);
}

static const struct file_operations sta_rpmsg_mm_fops = {
	.open      = sta_rpmsg_mm_open,
	.release   = sta_rpmsg_mm_release,
	.read      = sta_rpmsg_mm_read,
	.write     = sta_rpmsg_mm_write,
	.poll      = sta_rpmsg_mm_poll,
	.unlocked_ioctl = sta_rpmsg_mm_ioctl,
	.owner     = THIS_MODULE,
};

static int sta_rpmsg_mm_probe(struct rpmsg_device *rpdev)
{
	int ret, major, minor;
	struct sta_rpmsg_mm_instance *mm;
	char buffer[MAX_PAYLOAD_SIZE];
	struct rpmsg_mm_msg_hdr *pdata;

	dev_dbg(&rpdev->dev, "%s\n", __func__);
	if (rpmsg_mm_ctx) {
		/* if rpmsg_mm_ctx is non NULL it means that we probably
		 * come from a suspend/resume sequence (i.e remove/probe).
		 * During this phase, we wants to keep /dev/rpmsg_mm0
		 * unchanged.
		 * That's why we don't destroy the device in remove function
		 * Additionnaly, we have to restore the connections
		 * existing btw M3 and A7 before the suspend/resume occurred
		 * Actually, M3 has totally rebooted during this phase but
		 * not linux.
		 */
		dev_dbg(&rpdev->dev,
			"Context and device already exists\n");
		rpmsg_mm_ctx->rpdev = rpdev;

		device_move(rpmsg_mm_ctx->dev, &rpdev->dev,
			    DPM_ORDER_DEV_AFTER_PARENT);

		dev_set_drvdata(&rpdev->dev, rpmsg_mm_ctx);
		rpmsg_mm_ctx->dev->devt = (dev_t)&rpdev->dev;

		/* Register again endpoints which were registered
		 * before remove/probe call
		 * We do not use an acknowledge (usage of send instead of
		 * send_sync) in this case because, we're in the probe
		 * service.
		 */

		/* same message for every instances */
		pdata = (struct rpmsg_mm_msg_hdr *)buffer;
		memset(pdata, 0, sizeof(*pdata));
		pdata->info =
			RPMSG_MM_RESET_ACK_INFO(RPMSG_MM_REGISTRATION_REQ);

		list_for_each_entry(mm, &rpmsg_mm_ctx->list, next) {
			/* reCreate the endpoint */
			if (!mm->ept)
				sta_rpmsg_mm_create_endpoint(mm);

			/* reRegister it on remote side */
			sta_rpmsg_mm_send_message((void *)mm,
						  RPSMG_MM_EPT_CORTEXM_MM,
						  pdata, NULL, false);

			/* Inform drivers about the communication status */
			sta_rpmsg_mm_transfer_comm_status(
				RPMSG_MM_COMM_AVAILABLE,
				mm);
		}
		return sta_rpmsg_mm_finalize_probe();
	}

	rpmsg_mm_ctx = kzalloc(sizeof(*rpmsg_mm_ctx), GFP_KERNEL);

	INIT_LIST_HEAD(&rpmsg_mm_ctx->list);
	mutex_init(&rpmsg_mm_ctx->lock);
	mutex_init(&rpmsg_mm_ctx->exchange_lock);

	major = MAJOR(sta_rpmsg_mm_cdev);
	minor = MINOR(sta_rpmsg_mm_cdev);

	/* Creating cdev structure */
	cdev_init(&rpmsg_mm_ctx->cdev, &sta_rpmsg_mm_fops);
	rpmsg_mm_ctx->cdev.owner = THIS_MODULE;

	ret = cdev_add(&rpmsg_mm_ctx->cdev, sta_rpmsg_mm_cdev, 1);
	if (ret) {
		dev_err(&rpdev->dev, "cdev_add failed: %d\n", ret);
		goto go_out;
	}

	rpmsg_mm_ctx->dev = device_create(sta_rpmsg_mm_class, &rpdev->dev,
			MKDEV(major, minor), NULL,
			"rpmsg_mm%d", minor);
	if (IS_ERR(rpmsg_mm_ctx->dev)) {
		ret = PTR_ERR(rpmsg_mm_ctx->dev);
		dev_err(&rpdev->dev, "device_create failed: %d\n", ret);
		goto clean_cdev;
	}

	rpmsg_mm_ctx->rpdev = rpdev;
	rpmsg_mm_ctx->minor = minor;
	dev_set_drvdata(&rpdev->dev, rpmsg_mm_ctx);
	dev_dbg(&rpdev->dev, "/dev/rpmsg_mm%d created\n", minor);
	return sta_rpmsg_mm_finalize_probe();

clean_cdev:
	cdev_del(&rpmsg_mm_ctx->cdev);
go_out:
	return ret;
}

static void sta_rpmsg_mm_remove(struct rpmsg_device *rpdev)
{
	struct sta_rpmsg_mm_ctx *srpm = dev_get_drvdata(&rpdev->dev);
	int major = MAJOR(sta_rpmsg_mm_cdev);
	struct sta_rpmsg_mm_instance *mm;

	mutex_lock(&srpm->lock);
	/* If list is empty it means that noone is registered, we can
	 * destroy the device character and free the previous context
	 */
	if (list_empty(&srpm->list)) {
		dev_dbg(&rpdev->dev,
			"destroy /dev/rpmsg_mm%d\n",
			srpm->minor);
		device_destroy(sta_rpmsg_mm_class, MKDEV(major, srpm->minor));
		cdev_del(&srpm->cdev);
		list_del(&srpm->next);
		mutex_unlock(&srpm->lock);
		kfree(rpmsg_mm_ctx);
		rpmsg_mm_ctx = NULL;
		return;
	}

	/* At least one instance exists (userland or driver), we destroy
	 * the endpoint and inform about communication status but we keep
	 * the context for the next probe which occurs after
	 * suspend/resume sequence
	 */
	list_for_each_entry(mm, &srpm->list, next) {
		/* We're removing the rpmsg_device, so endpoints
		 * created previously are obsolete.
		 * If needed, we'll create new ones at next probe
		 */
		if (mm->ept) {
			rpmsg_destroy_ept(mm->ept);
			mm->ept = NULL;
		}
		/* Inform drivers about the communication status */
		sta_rpmsg_mm_transfer_comm_status(
			RPMSG_MM_COMM_UNAVAILABLE,
			mm);
	}
	kfree(rpmsg_mm_probe_status);
	rpmsg_mm_probe_status = NULL;
	mutex_unlock(&srpm->lock);
}

static struct rpmsg_driver sta_rpmsg_mm_driver = {
	.drv.name   = KBUILD_MODNAME,
	.drv.owner  = THIS_MODULE,
	.id_table   = sta_rpmsg_mm_id_table,
	.probe    = sta_rpmsg_mm_probe,
	.callback   = sta_rpmsg_mm_base_cb,
	.remove  = sta_rpmsg_mm_remove,
};

static int __init init(void)
{
	int ret;

	ret = alloc_chrdev_region(&sta_rpmsg_mm_cdev,
				  0, 1, KBUILD_MODNAME);
	if (ret) {
		pr_err("alloc_chrdev_region failed: %d\n", ret);
		goto out;
	}

	sta_rpmsg_mm_class = class_create(THIS_MODULE, KBUILD_MODNAME);
	if (IS_ERR(sta_rpmsg_mm_class)) {
		ret = PTR_ERR(sta_rpmsg_mm_class);
		pr_err("class_create failed: %d\n", ret);
		goto unreg_region;
	}

	return register_rpmsg_driver(&sta_rpmsg_mm_driver);

unreg_region:
	unregister_chrdev_region(sta_rpmsg_mm_cdev, 1);
out:
	return ret;
}
subsys_initcall(init);

static void __exit fini(void)
{
	int major = MAJOR(sta_rpmsg_mm_cdev);

	unregister_rpmsg_driver(&sta_rpmsg_mm_driver);

	if (rpmsg_mm_ctx) {
		device_destroy(sta_rpmsg_mm_class,
			       MKDEV(major, rpmsg_mm_ctx->minor));
		cdev_del(&rpmsg_mm_ctx->cdev);
		list_del(&rpmsg_mm_ctx->next);
	}

	class_destroy(sta_rpmsg_mm_class);
	unregister_chrdev_region(sta_rpmsg_mm_cdev, 1);
}
module_exit(fini);

MODULE_DESCRIPTION("STMicroelectroncis STA rpmsg mm driver");
MODULE_LICENSE("GPL v2");
