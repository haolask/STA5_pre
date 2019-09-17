/*
 * Copyright (C) 2015 ST Microelectronics
 * Jean-Nicolas Graux <jean-nicolas.graux@st.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/bitops.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/reboot.h>
#include <linux/rpmsg.h>
#include <linux/sched.h>
#include <linux/slab.h>

/**
 * Common definitions between back-end and front-end driver.
 * Content must be updated in case remote front-end driver definitions
 * are updated and vice-versa. Make sure that both match.
 */

#define	PMU_LVI BIT(0)
#define	PMU_ONOFF BIT(1)
#define	PMU_IGNKEY BIT(2)
#define	PMU_VDDOK BIT(3) /* STAND-BY source only */

#define PMU_WAKES GENMASK(11, 4)
#define PMU_WAKE0 BIT(4)
#define PMU_WAKE1 BIT(5)
#define PMU_WAKE2 BIT(6)
#define PMU_WAKE3 BIT(7)
#define PMU_WAKE4 BIT(8)
#define PMU_WAKE5 BIT(9)
#define PMU_WAKE6 BIT(10)
#define PMU_WAKE7 BIT(11)

#define	PMU_RTC BIT(12)
#define	PMU_SBY2NCTR_DONE BIT(19)
#define	PMU_PORLVD BIT(20)

#define PMU_WKUP_SRC_MASK (PMU_LVI | PMU_ONOFF | PMU_IGNKEY | PMU_WAKES)
#define PMU_SBY_SRC_MASK (PMU_LVI | PMU_ONOFF | PMU_IGNKEY | PMU_VDDOK)

#define PMU_WKUP_STAT_MASK (PMU_LVI | PMU_ONOFF | PMU_IGNKEY | PMU_WAKES |\
	PMU_RTC | PMU_PORLVD)

#define	PMU_N2SBYCTR_TO_MAX 3

/**
 * enum prm_msg_types - various message types currently supported
 */
enum prm_msg_types {
	PRM_MSG_GET_WKUP_STAT, /* front-end driver -> PRM driver */
	PRM_MSG_WKUP_STAT, /* PRM driver -> front-end driver */
	PRM_MSG_REGISTER_SBY, /* front-end driver -> PRM driver */
	PRM_MSG_SBY, /* PRM driver -> front-end driver */
	PRM_MSG_SBY_READY, /* front-end driver -> PRM driver */
	PRM_MSG_SW_GOTO_SBY, /* front-end driver -> PRM driver */
	PRM_MSG_UPD_WKUP_SRC, /* front-end driver -> PRM driver */
	PRM_MSG_UPD_SBY_SRC, /* front-end driver -> PRM driver */
	PRM_MSG_REGISTER_SW_RESET, /* front-end driver -> PRM driver */
	PRM_MSG_SW_RESET, /* PRM driver -> front-end driver */
	PRM_MSG_SW_RESET_READY, /* front-end driver -> PRM driver */
	PRM_MSG_DO_SW_RESET, /* front-end driver -> PRM driver */
};

struct prm_msg_hdr {
	u32 type;
	u32 len;
	char data[0];
} __packed;

/**
 * struct prm_msg_sby - PRM_MSG_SBY notification message
 *
 * Used to notify host about pending STAND-BY.
 * message direction: from PRM back-end driver to front-end PM driver (host).
 *
 * @sby_status:	current value of PMU ON to STAND-BY status register.
 */
struct prm_msg_sby {
	u32 sby_status;
} __packed;

/**
 * struct prm_msg_wkup_stat - PRM_MSG_WKUP_STAT notification message
 *
 * Sent by back-end driver to answer to PRM_MSG_GET_WKUP_STAT.
 * message direction: from PRM back-end driver to front-end PM driver (host).
 *
 * @wkup_status: content of STANDBY to ON status register (red at boot time).
 */
struct prm_msg_wkup_stat {
	u32 wkup_status;
} __packed;

/**
 * struct prm_msg_upd_wkup_src - PRM_MSG_UPD_WKUP_SRC request message
 *
 * Sent by host to update wake-up sources.
 * message direction: from front-end PM driver (host) to PRM back-end driver.
 *
 * @enable_src: wake-up sources to enable.
 * @disable_src: wake-up sources to disable.
 * @rising_edge: to select active edges as rising edges.
 * @falling_edge: to select active edges as falling edges.

 */
struct prm_msg_upd_wkup_src {
	u32 enable_src;
	u32 disable_src;
	u32 rising_edge;
	u32 falling_edge;
} __packed;

/**
 * struct prm_msg_upd_sby_src - PRM_MSG_UPD_SBY_SRC request message
 *
 * Sent by host to update STAND-BY sources.
 * message direction: from front-end PM driver (host) to PRM back-end driver.
 *
 * @enable_src: STAND-BY sources to enable.
 * @disable_src: STAND-BY sources to disable.
 * @rising_edge: to select active edges as rising edges.
 * @falling_edge: to select active edges as falling edges.
 * @enable_irq: to enable one or several interrupt sources.
 * @disable_irq: to disable one or several interrupt sources.
 */
struct prm_msg_upd_sby_src {
	u32 enable_src;
	u32 disable_src;
	u32 rising_edge;
	u32 falling_edge;
	u32 enable_irq;
	u32 disable_irq;
} __packed;

/**
 * Linux driver specific definitions
 */

/**
 * struct sta1295_prm_platform_data - Power & Reset management configuration data
 * @enable_wkup_src: stand-by to on sources to enable
 * @disable_wkup_src: stand-by to on sources to disable
 * @rising_wkup_edge: to select rising edges for wake-up sources
 * @falling_wkup_edge: to select falling edges for wake-up sources
 * @enable_sby_src: on to stand-by sources to enable
 * @disable_sby_src: on to stand-by sources to disable
 * @rising_sby_edge: to select rising edges for stand-by sources
 * @falling_sby_edge: to select falling edges for stand-by sources
 * @enable_sby_irq: on to stand-by irqs to enable
 * @disable_sby_irq: on to stand-by irqs to disable
 * @register_sw_reset: to be notified on a software reset event
 * @register_sby: to be notified on a Go To STAND-BY event
 */
struct sta1295_prm_platform_data {
	u32 enable_wkup_src;
	u32 disable_wkup_src;
	u32 rising_wkup_edge;
	u32 falling_wkup_edge;
	u32 enable_sby_src;
	u32 disable_sby_src;
	u32 rising_sby_edge;
	u32 falling_sby_edge;
	u32 enable_sby_irq;
	u32 disable_sby_irq;
	bool register_sw_reset;
	bool register_sby;
	bool do_poweroff_on_sby;
};

struct sta1295_prm_device {
	struct mutex lock;
	struct device *dev;
	const struct sta1295_prm_platform_data *pdata;
	struct rpmsg_device *rpdev;
	struct workqueue_struct *init_wq;
	struct work_struct init_work;
	struct completion reply_arrived;
	u32 wkup_status;
	u32 sby_status;
	bool sw_rst;
	bool goto_sby;
	bool init_done;
};

static struct class *sta1295_prm_class;
static dev_t sta1295_prm_dev_t;

#ifndef CONFIG_ARM_PSCI
/* ugly global used in sta1295_power_off & sta1295_restart functions */
static struct sta1295_prm_device *sta1295_prm;
#endif

static int sta1295_prm_cb(struct rpmsg_device *rpdev, void *data, int len,
			   void *priv, u32 src)
{
	struct prm_msg_hdr *hdr = data;
	struct sta1295_prm_device *prm = priv;

	if (len < sizeof(*hdr) || hdr->len < len - sizeof(*hdr)) {
		dev_warn(prm->dev, "%s: truncated message\n", __func__);
		return -EINVAL;
	}

	dev_dbg(prm->dev, "%s: incoming msg src 0x%x type %d len %d\n",
		__func__, src, hdr->type, hdr->len);
	print_hex_dump_debug("stm_rpmsg RX: ", DUMP_PREFIX_NONE, 16, 1,
			     data, len,  true);

	switch (hdr->type) {
	case PRM_MSG_WKUP_STAT:
	{
		struct prm_msg_wkup_stat *msg;

		if (hdr->len < sizeof(*msg)) {
			dev_warn(prm->dev, "invalid PRM_MSG_WKUP_STAT msg\n");
			break;
		}
		msg = (struct prm_msg_wkup_stat *)hdr->data;
		dev_info(prm->dev, "wake-up status: 0x%x\n",
			 msg->wkup_status);
		prm->wkup_status = msg->wkup_status;
		complete(&prm->reply_arrived);
		break;
	}
	case PRM_MSG_SBY:
	{
		struct prm_msg_sby *msg;

		if (!prm->pdata->register_sby) {
			dev_warn(prm->dev,
			   "unexpected goto stand-by request from back-end\n");
			return -EPERM;
		}
		if (hdr->len < sizeof(*msg)) {
			dev_warn(prm->dev, "invalid PRM_MSG_SBY msg\n");
			break;
		}
		msg = (struct prm_msg_sby *)hdr->data;
		dev_info(prm->dev,
			 "goto stand-by request from back-end, sby status:%x\n",
			msg->sby_status);
		prm->sby_status = msg->sby_status;
		prm->goto_sby = true;
		/* let's notify init process about pending system shutdown */
		if (prm->pdata->do_poweroff_on_sby)
			kill_cad_pid(SIGUSR2, 1);
		break;
	}
	case PRM_MSG_SW_RESET:
		if (!prm->pdata->register_sw_reset) {
			dev_warn(prm->dev,
				 "unexpected sw reset request from back-end\n");
			return -EPERM;
		}
		dev_info(prm->dev, "sw reset back-end request\n");
		prm->sw_rst = true;
		/* let's notify init process about pending system reset */
		kill_cad_pid(SIGTERM, 1);
		break;
	default:
		dev_warn(prm->dev, "unexpected msg type: %d\n", hdr->type);
		break;
	}
	return 0;
}

static int sta1295_prm_send_rpmsg(struct sta1295_prm_device *prm,
				  u32 type, void *data)
{
	struct prm_msg_hdr *hdr;
	bool reply_expected = false;
	u8 *rpmsg_data;
	int len, ret;

	len = sizeof(*hdr);

	mutex_lock(&prm->lock);

	switch (type) {
	case PRM_MSG_GET_WKUP_STAT:
	{
		init_completion(&prm->reply_arrived);
		reply_expected = true;
		/* fall through */
	}
#ifndef CONFIG_ARM_PSCI
	case PRM_MSG_SBY_READY:
		/* fall through */
	case PRM_MSG_SW_GOTO_SBY:
		/* fall through */
	case PRM_MSG_SW_RESET_READY:
		/* fall through */
	case PRM_MSG_DO_SW_RESET:
		/* fall through */
#endif
	case PRM_MSG_REGISTER_SBY:
		/* fall through */
	case PRM_MSG_REGISTER_SW_RESET:
		/* fall through */
	{
		rpmsg_data = kzalloc(sizeof(*hdr), GFP_KERNEL);
		if (!rpmsg_data) {
			mutex_unlock(&prm->lock);
			return -ENOMEM;
		}

		hdr = (struct prm_msg_hdr *)rpmsg_data;
		hdr->type = type;
		hdr->len = 0;
		break;
	}
	case PRM_MSG_UPD_WKUP_SRC:
	{
		rpmsg_data = kzalloc(sizeof(*hdr) +
			sizeof(struct prm_msg_upd_wkup_src),
			GFP_KERNEL);
		if (!rpmsg_data) {
			mutex_unlock(&prm->lock);
			return -ENOMEM;
		}

		hdr = (struct prm_msg_hdr *)rpmsg_data;
		hdr->type = type;
		hdr->len = sizeof(struct prm_msg_upd_wkup_src);
		len += hdr->len;
		memcpy(hdr->data, data, sizeof(struct prm_msg_upd_wkup_src));
		break;
	}
	case PRM_MSG_UPD_SBY_SRC:
	{
		rpmsg_data = kzalloc(sizeof(*hdr) +
				     sizeof(struct prm_msg_upd_sby_src),
				     GFP_KERNEL);
		if (!rpmsg_data) {
			mutex_unlock(&prm->lock);
			return -ENOMEM;
		}

		hdr = (struct prm_msg_hdr *)rpmsg_data;
		hdr->type = type;
		hdr->len = sizeof(struct prm_msg_upd_sby_src);
		len += hdr->len;
		memcpy(hdr->data, data, sizeof(struct prm_msg_upd_sby_src));
		break;
	}
	default:
		dev_warn(prm->dev, "unknown msg type: %d\n", type);
		mutex_unlock(&prm->lock);
		return -EINVAL;
	}

	ret = rpmsg_send(prm->rpdev->ept, rpmsg_data, len);
	if (ret) {
		dev_err(prm->dev, "rpmsg_send failed: %d\n", ret);
		kfree(rpmsg_data);
		mutex_unlock(&prm->lock);
		return ret;
	}

	if (reply_expected) {
		/* wait until a reply arrives or 5 seconds elapse */
		ret = wait_for_completion_interruptible_timeout(
			&prm->reply_arrived, msecs_to_jiffies(5000));
		if (!ret) {
			dev_err(prm->dev,
				"msg type:%u, timeout occurred before reply\n",
				type);
			kfree(rpmsg_data);
			mutex_unlock(&prm->lock);
			return -ENODEV;
		}
	}
	kfree(rpmsg_data);
	mutex_unlock(&prm->lock);
	return 0;
}

/* sysfs device definitions */

/* to allow any user to get latest stand-by to on status */
static ssize_t sta1295_prm_wkup_status(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return 0;
	}

	return sprintf(buf, "%x\n", prm->wkup_status);
}
static DEVICE_ATTR(wkup_status, S_IRUGO, sta1295_prm_wkup_status,
	NULL);

/* to allow any user to enable one or several hardware wake-up sources */
static ssize_t sta1295_prm_enable_wkup_src(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int ret;
	u32 val;
	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_wkup_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_wkup_src));
	msg.enable_src = val & PMU_WKUP_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_WKUP_SRC, &msg);
	if (ret)
		return ret;

	return count;
}

static DEVICE_ATTR(enable_wkup_src, S_IWUSR, NULL,
		   sta1295_prm_enable_wkup_src);

/* to allow any user to disable one or several hardware wake-up sources */
static ssize_t sta1295_prm_disable_wkup_src(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_wkup_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_wkup_src));
	msg.disable_src = val & PMU_WKUP_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_WKUP_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(disable_wkup_src, S_IWUSR, NULL,
		   sta1295_prm_disable_wkup_src);

/**
 * to allow any user to select active edge as rising for one or several
 * hardware wake-up sources
 */
static ssize_t sta1295_prm_set_wkup_rising_edge(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_wkup_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_wkup_src));
	msg.rising_edge = val & PMU_WKUP_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_WKUP_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(set_wkup_rising_edge, S_IWUSR, NULL,
		   sta1295_prm_set_wkup_rising_edge);

/**
 * to allow any user to select active edge as falling for one or several
 * hardware wake-up sources
 */
static ssize_t sta1295_prm_set_wkup_falling_edge(struct device *dev,
						 struct device_attribute *attr,
						 const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_wkup_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_wkup_src));
	msg.falling_edge = val & PMU_WKUP_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_WKUP_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(set_wkup_falling_edge, S_IWUSR, NULL,
		   sta1295_prm_set_wkup_falling_edge);

/* to allow any user to enable one or several hardware stand-by sources */
static ssize_t sta1295_prm_enable_sby_src(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_sby_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_sby_src));
	msg.enable_src = val & PMU_SBY_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(enable_sby_src, S_IWUSR, NULL,
		   sta1295_prm_enable_sby_src);

/* to allow any user to disable one or several hardware stand-by sources */
static ssize_t sta1295_prm_disable_sby_src(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_sby_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_sby_src));
	msg.disable_src = val & PMU_SBY_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(disable_sby_src, S_IWUSR, NULL,
	sta1295_prm_disable_sby_src);

/**
 * to allow any user to select active edge as rising for one or several
 * hardware wake-up sources
 */
static ssize_t sta1295_prm_set_sby_rising_edge(struct device *dev,
					       struct device_attribute *attr,
					       const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_sby_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_sby_src));
	msg.rising_edge = val & PMU_SBY_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(set_sby_rising_edge, S_IWUSR, NULL,
		   sta1295_prm_set_sby_rising_edge);

/**
 * to allow any user to select active edge as falling for one or several
 * hardware wake-up sources
 */
static ssize_t sta1295_prm_set_sby_falling_edge(struct device *dev,
						struct device_attribute *attr,
						const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_sby_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_sby_src));
	msg.falling_edge = val & PMU_SBY_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(set_sby_falling_edge, S_IWUSR, NULL,
		   sta1295_prm_set_sby_falling_edge);

/* to allow any user to enable one or several hardware stand-by irqs */
static ssize_t sta1295_prm_enable_sby_irq(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_sby_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_sby_src));
	msg.enable_irq = val & PMU_SBY_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(enable_sby_irq, S_IWUSR, NULL,
		   sta1295_prm_enable_sby_irq);

/* to allow any user to disable one or several hardware stand-by irqs */
static ssize_t sta1295_prm_disable_sby_irq(struct device *dev,
					   struct device_attribute *attr,
					   const char *buf, size_t count)
{
	int ret;
	u32 val;

	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);
	struct prm_msg_upd_sby_src msg;

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return count;
	}

	ret = kstrtou32(buf, 0, &val);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(struct prm_msg_upd_sby_src));
	msg.disable_irq = val & PMU_SBY_SRC_MASK;

	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
	if (ret)
		return ret;

	return count;
}
static DEVICE_ATTR(disable_sby_irq, S_IWUSR, NULL,
		   sta1295_prm_disable_sby_irq);

/* to allow any user to monitor current hardware go to stand-by status */
static ssize_t sta1295_prm_sby_status(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return 0;
	}

	return sprintf(buf, "%x\n", prm->sby_status);
}
static DEVICE_ATTR(sby_status, S_IRUGO, sta1295_prm_sby_status, NULL);

/**
 * to allow any user to poll on a hw/sw go to stand-by event
 * raised by the back-end driver
 */
static ssize_t sta1295_prm_pending_goto_sby(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return 0;
	}

	return sprintf(buf, "%i\n", prm->goto_sby);
}
static DEVICE_ATTR(pending_goto_sby, S_IRUGO, sta1295_prm_pending_goto_sby,
		   NULL);

/**
 * to allow any user to poll on a software reset event
 * raised by the back-end driver
 */
static ssize_t sta1295_prm_pending_sw_reset(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	struct device *rpdev = dev->parent;
	struct sta1295_prm_device *prm = dev_get_drvdata(rpdev);

	if (!prm->init_done) {
		dev_warn(prm->dev, "%s: init not finalized!\n", __func__);
		return 0;
	}

	return sprintf(buf, "%i\n", prm->sw_rst);
}
static DEVICE_ATTR(pending_sw_reset, S_IRUGO, sta1295_prm_pending_sw_reset,
		   NULL);

static struct attribute *sta1295_prm_attributes[] = {
	&dev_attr_wkup_status.attr,
	&dev_attr_enable_wkup_src.attr,
	&dev_attr_disable_wkup_src.attr,
	&dev_attr_set_wkup_rising_edge.attr,
	&dev_attr_set_wkup_falling_edge.attr,
	&dev_attr_enable_sby_src.attr,
	&dev_attr_disable_sby_src.attr,
	&dev_attr_set_sby_rising_edge.attr,
	&dev_attr_set_sby_falling_edge.attr,
	&dev_attr_enable_sby_irq.attr,
	&dev_attr_disable_sby_irq.attr,
	&dev_attr_sby_status.attr,
	&dev_attr_pending_goto_sby.attr,
	&dev_attr_pending_sw_reset.attr,
	NULL
};

static const struct attribute_group sta1295_prm_attr_group = {
	.attrs = sta1295_prm_attributes,
};

#ifndef CONFIG_ARM_PSCI
static void sta1295_power_off(void)
{
	if (!sta1295_prm->init_done) {
		dev_warn(sta1295_prm->dev,
			 "%s: init not finalized!\n", __func__);
	}

	if (sta1295_prm->goto_sby)
		/* goto stand-by event came from back-end driver */
		sta1295_prm_send_rpmsg(sta1295_prm, PRM_MSG_SBY_READY, NULL);
	else
		sta1295_prm_send_rpmsg(sta1295_prm, PRM_MSG_SW_GOTO_SBY, NULL);

	/* wait for death */
	wfi();
	while (1)
		;
}

static int sta1295_restart(struct notifier_block *nb, unsigned long action,
			   void *data)
{
	if (!sta1295_prm->init_done) {
		dev_warn(sta1295_prm->dev,
			 "%s: init not finalized!\n", __func__);
	}

	if (sta1295_prm->sw_rst)
		/* software reset event came from back-end driver */
		sta1295_prm_send_rpmsg(sta1295_prm,
				       PRM_MSG_SW_RESET_READY, NULL);
	else
		sta1295_prm_send_rpmsg(sta1295_prm, PRM_MSG_DO_SW_RESET, NULL);

	return NOTIFY_DONE;
}

static struct notifier_block restart_nb = {
	.notifier_call = sta1295_restart,
	.priority = 128,
};
#endif

static void sta1295_prm_init_work(struct work_struct *work)
{
	int ret, major;
	struct sta1295_prm_device *prm =
		container_of(work, struct sta1295_prm_device, init_work);

	major = MAJOR(sta1295_prm_dev_t);

	prm->dev = device_create(sta1295_prm_class, &prm->rpdev->dev,
				 MKDEV(major, 0), NULL,
				 "sta1295-prm%d", 0);
	if (IS_ERR(prm->dev)) {
		ret = PTR_ERR(prm->dev);
		dev_err(&prm->rpdev->dev, "device_create failed: %d\n", ret);
		return;
	}

	/**
	 * get wake-up status from PRM back-end driver.
	 */
	ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_GET_WKUP_STAT, NULL);
	if (ret) {
		dev_err(prm->dev,
			"%s: failed to send PRM_MSG_GET_WKUP_STAT\n", __func__);
		return;
	}

	/**
	 * optionally ask to be notified by back-end driver whenever
	 * a Go To STAND-BY event occur.
	 */
	if (prm->pdata->register_sby) {
		ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_REGISTER_SBY, NULL);
		if (ret) {
			dev_err(prm->dev,
				"%s: failed to send PRM_MSG_REGISTER_SBY\n",
					__func__);
			return;
		}
	}

	/**
	 * optionally ask to be notified by back-end driver whenever
	 * a software reset occur in Cortex-M3 side.
	 */
	if (prm->pdata->register_sw_reset) {
		ret = sta1295_prm_send_rpmsg(prm,
					     PRM_MSG_REGISTER_SW_RESET, NULL);
		if (ret) {
			dev_err(prm->dev,
				"%s: failed to send PRM_MSG_REGISTER_SW_RESET\n",
				__func__);
			return;
		}
	}

	/* optionally setup stand-by to on sources */
	if (prm->pdata->enable_wkup_src || prm->pdata->disable_wkup_src ||
	    prm->pdata->rising_wkup_edge || prm->pdata->falling_wkup_edge) {
		struct prm_msg_upd_wkup_src msg;

		msg.enable_src = prm->pdata->enable_wkup_src;
		msg.disable_src = prm->pdata->disable_wkup_src;
		msg.rising_edge = prm->pdata->rising_wkup_edge;
		msg.falling_edge = prm->pdata->falling_wkup_edge;
		ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_WKUP_SRC, &msg);
		if (ret) {
			dev_err(prm->dev,
				"%s: failed to send PRM_MSG_UPD_WKUP_SRC\n",
				__func__);
			return;
		}
	}

	/* optionally setup on to stand-by sources */
	if (prm->pdata->enable_sby_src || prm->pdata->disable_sby_src) {
		struct prm_msg_upd_sby_src msg;

		msg.enable_src = prm->pdata->enable_sby_src;
		msg.disable_src = prm->pdata->disable_sby_src;
		msg.rising_edge = prm->pdata->rising_sby_edge;
		msg.falling_edge = prm->pdata->falling_sby_edge;
		msg.enable_irq = prm->pdata->enable_sby_irq;
		msg.disable_irq = prm->pdata->disable_sby_irq;
		ret = sta1295_prm_send_rpmsg(prm, PRM_MSG_UPD_SBY_SRC, &msg);
		if (ret) {
			dev_err(prm->dev,
				"%s: failed to send PRM_MSG_UPD_SBY_SRC\n",
				__func__);
			return;
		}
	}

	ret = sysfs_create_group(&prm->dev->kobj, &sta1295_prm_attr_group);
	if (ret) {
		dev_err(prm->dev, "failed to create sysfs group\n");
		return;
	}

#ifndef CONFIG_ARM_PSCI
	pm_power_off = sta1295_power_off;
	register_restart_handler(&restart_nb);
#endif

	prm->init_done = true;
	dev_info(prm->dev, "%s: done\n", __func__);
}

static void sta1295_prm_dt_populate_pdata(struct device_node *np,
				struct sta1295_prm_platform_data *pdata)
{
	u32 val;

	if (np) {
		if (!of_property_read_u32(np, "st,enable_wkup_src", &val))
			pdata->enable_wkup_src = val & PMU_WKUP_SRC_MASK;
		if (!of_property_read_u32(np, "st,disable_wkup_src", &val))
			pdata->disable_wkup_src = val & PMU_WKUP_SRC_MASK;
		if (!of_property_read_u32(np, "st,rising_wkup_edge", &val))
			pdata->rising_wkup_edge = val & PMU_WKUP_SRC_MASK;
		if (!of_property_read_u32(np, "st,falling_wkup_edge", &val))
			pdata->falling_wkup_edge = val & PMU_WKUP_SRC_MASK;
		if (!of_property_read_u32(np, "st,enable_sby_src", &val))
			pdata->enable_sby_src = val & PMU_SBY_SRC_MASK;
		if (!of_property_read_u32(np, "st,disable_sby_src", &val))
			pdata->disable_sby_src = val & PMU_SBY_SRC_MASK;
		if (!of_property_read_u32(np, "st,rising_sby_edge", &val))
			pdata->rising_sby_edge = val & PMU_SBY_SRC_MASK;
		if (!of_property_read_u32(np, "st,falling_sby_edge", &val))
			pdata->falling_sby_edge = val & PMU_SBY_SRC_MASK;
		if (!of_property_read_u32(np, "st,enable_sby_irq", &val))
			pdata->enable_sby_irq = val & PMU_SBY_SRC_MASK;
		if (!of_property_read_u32(np, "st,disable_sby_irq", &val))
			pdata->disable_sby_irq = val & PMU_SBY_SRC_MASK;
		if (of_property_read_bool(np, "st,register_sby"))
			pdata->register_sby = true;
		if (of_property_read_bool(np, "st,register_sw_reset"))
			pdata->register_sw_reset = true;
		if (of_property_read_bool(np, "st,do_poweroff_on_sby"))
			pdata->do_poweroff_on_sby = true;

	}
}

static int sta1295_prm_probe(struct rpmsg_device *rpdev)
{
	struct sta1295_prm_device *prm;
	struct device_node *np = rpdev->dev.of_node;
	struct sta1295_prm_platform_data *pdata = rpdev->dev.platform_data;

	prm = devm_kzalloc(&rpdev->dev, sizeof(struct sta1295_prm_device),
			   GFP_KERNEL);
	if (!prm)
		return -ENOMEM;
#ifndef CONFIG_ARM_PSCI
	sta1295_prm = prm;
#endif
	mutex_init(&prm->lock);

	np = of_find_compatible_node(NULL, NULL, "st,sta1295-prm");

	if (!np && !pdata) {
		dev_err(&rpdev->dev, "no platform data or DT found\n");
		return -EINVAL;
	}

	if (!pdata) {
		pdata = devm_kzalloc(&rpdev->dev, sizeof(*pdata), GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;
	}

	if (np)
		sta1295_prm_dt_populate_pdata(np, pdata);

	prm->init_wq =
		create_singlethread_workqueue("sta1295_prm_init_wq");
	if (!prm->init_wq)
		return -ENOMEM;

	prm->rpdev = rpdev;
	prm->pdata = pdata;
	prm->rpdev->ept->priv = prm;

	dev_set_drvdata(&rpdev->dev, prm);

	INIT_WORK(&prm->init_work, sta1295_prm_init_work);

	queue_work(prm->init_wq, &prm->init_work);

	return 0;
}

void sta1295_prm_remove(struct rpmsg_device *rpdev)
{
	struct sta1295_prm_device *prm = dev_get_drvdata(&rpdev->dev);
#ifndef CONFIG_ARM_PSCI
	int ret;

	ret = unregister_restart_handler(&restart_nb);
	if (ret)
		dev_warn(&rpdev->dev, "failed to unregister restart handler\n");
#endif
	sysfs_remove_group(&prm->dev->kobj, &sta1295_prm_attr_group);

	destroy_workqueue(prm->init_wq);
	device_destroy(sta1295_prm_class, sta1295_prm_dev_t);

	dev_dbg(&rpdev->dev, "sta1295-prm rpmsg driver is removed\n");
}

static struct rpmsg_device_id sta1295_prm_id_table[] = {
	{ .name	= "sta1295-prm" },
	{ },
};
MODULE_DEVICE_TABLE(platform, stm_rpmsg_id_table);

static struct rpmsg_driver sta1295_prm_driver = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= sta1295_prm_id_table,
	.probe		= sta1295_prm_probe,
	.remove		= sta1295_prm_remove,
	.callback	= sta1295_prm_cb,
};

static int __init sta1295_prm_init(void)
{
	int ret;

	sta1295_prm_class = class_create(THIS_MODULE, KBUILD_MODNAME);
	if (IS_ERR(sta1295_prm_class)) {
		ret = PTR_ERR(sta1295_prm_class);
		pr_err("class_create failed: %d\n", ret);
		return ret;
	}

	return register_rpmsg_driver(&sta1295_prm_driver);
}
module_init(sta1295_prm_init);

static void __exit sta1295_prm_exit(void)
{
	class_destroy(sta1295_prm_class);
	unregister_rpmsg_driver(&sta1295_prm_driver);
}
module_exit(sta1295_prm_exit);

MODULE_AUTHOR("Jean-Nicolas Graux - ST Microelectronics");
MODULE_DESCRIPTION("STA1295 Power and Reset Management");
MODULE_ALIAS("sta1295-prm");
