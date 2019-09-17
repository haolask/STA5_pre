/*
 * Mailbox test driver
 *
 * Copyright (C) 2013 ST Microelectronics
 * License Terms: GNU General Public License v2
 * Author: Olivier Lebreton <olivier.lebreton@st.com> for ST Microelectronics
 */

#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mailbox_client.h>
#include <linux/mailbox_controller.h>
#include <linux/mailbox_sta.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define MAX_MBOX_TEST_DRIVER	1
#define MBOX_HEADER_SIZE	8
#define MBOX_DATA_SIZE		470
#define MBOX_NAME_LENGTH	16

#define MBOX_IRSR_REG		0
#define MBOX_IPSR_REG		4
#define MBOX_IMR_REG		8

/* mbox debugfs parent dir */
static struct dentry	*mbox_dbg;

static dev_t		mbox_test_dev;
static struct class	*mbox_test_class;

struct mbox_test_msg {
	u8		dlen;
	u8		data[MBOX_DATA_SIZE];
};

struct mbox_test_device {
	struct cdev		cdev;
	struct device		*dev;
	struct device		*ddev;
	struct list_head	list;
	/* protected access to list and device nb */
	struct mutex		lock;
	void __iomem		*rp_reg_base;
	void __iomem		*host_reg_base;
	int			nb;
};

static struct mbox_test_device  mtdev;

struct mbox_test_instance {
	struct list_head	next;
	struct mbox_test_device	*mtdev;
	char			name[MBOX_NAME_LENGTH];
	struct mbox_client	client;
	struct mbox_chan	*chan;
	wait_queue_head_t	readq;
	struct mbox_test_msg	rxmsg;
	struct mbox_test_msg	txmsg;
	/* protected access to empty flag */
	struct mutex		lock;
	bool			empty_flag;
};

static inline unsigned int mbox_read_reg(void __iomem *reg)
{
	return __raw_readl(reg);
}

static inline void mbox_write_reg(u32 val, void __iomem *reg)
{
	__raw_writel(val, reg);
}

/* expose the rp mailbox register via debugfs */

static int mbox_rp_irsr_get(void *data, u64 *val)
{
	struct mbox_test_device *mtdev = data;

	*val = mbox_read_reg(mtdev->rp_reg_base + MBOX_IRSR_REG);
	return 0;
}

static int mbox_rp_irsr_set(void *data, u64 val)
{
	struct mbox_test_device *mtdev = data;

	mbox_write_reg(val, mtdev->rp_reg_base + MBOX_IRSR_REG);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mbox_rp_irsr_fops, mbox_rp_irsr_get,
			mbox_rp_irsr_set, "%.8llx\n");

static int mbox_rp_ipsr_get(void *data, u64 *val)
{
	struct mbox_test_device *mtdev = data;

	*val = mbox_read_reg(mtdev->rp_reg_base + MBOX_IPSR_REG);
	return 0;
}

static int mbox_rp_ipsr_set(void *data, u64 val)
{
	struct mbox_test_device *mtdev = data;

	mbox_write_reg(val, mtdev->rp_reg_base + MBOX_IPSR_REG);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mbox_rp_ipsr_fops, mbox_rp_ipsr_get,
			mbox_rp_ipsr_set, "%.8llx\n");

static int mbox_rp_imr_get(void *data, u64 *val)
{
	struct mbox_test_device *mtdev = data;

	*val = mbox_read_reg(mtdev->rp_reg_base + MBOX_IMR_REG);
	return 0;
}

static int mbox_rp_imr_set(void *data, u64 val)
{
	struct mbox_test_device *mtdev = data;

	mbox_write_reg(val, mtdev->rp_reg_base + MBOX_IMR_REG);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mbox_rp_imr_fops, mbox_rp_imr_get,
			mbox_rp_imr_set, "%.8llx\n");

/* expose the host mailbox register via debugfs */

static int mbox_host_irsr_get(void *data, u64 *val)
{
	struct mbox_test_device *mtdev = data;

	*val = mbox_read_reg(mtdev->host_reg_base + MBOX_IRSR_REG);
	return 0;
}

static int mbox_host_irsr_set(void *data, u64 val)
{
	struct mbox_test_device *mtdev = data;

	mbox_write_reg(val, mtdev->host_reg_base + MBOX_IRSR_REG);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mbox_host_irsr_fops, mbox_host_irsr_get,
			mbox_host_irsr_set, "%.8llx\n");

static int mbox_host_ipsr_get(void *data, u64 *val)
{
	struct mbox_test_device *mtdev = data;

	*val = mbox_read_reg(mtdev->host_reg_base + MBOX_IPSR_REG);
	return 0;
}

static int mbox_host_ipsr_set(void *data, u64 val)
{
	struct mbox_test_device *mtdev = data;

	mbox_write_reg(val, mtdev->host_reg_base + MBOX_IPSR_REG);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mbox_host_ipsr_fops, mbox_host_ipsr_get,
			mbox_host_ipsr_set, "%.8llx\n");

static int mbox_host_imr_get(void *data, u64 *val)
{
	struct mbox_test_device *mtdev = data;

	*val = mbox_read_reg(mtdev->host_reg_base + MBOX_IMR_REG);
	return 0;
}

static int mbox_host_imr_set(void *data, u64 val)
{
	struct mbox_test_device *mtdev = data;

	mbox_write_reg(val, mtdev->host_reg_base + MBOX_IMR_REG);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(mbox_host_imr_fops, mbox_host_imr_get,
			mbox_host_imr_set, "%.8llx\n");

/* Mailbox driver file operations */

static void mbox_test_callback(struct mbox_client *mb_client, void *data)
{
	struct sta_mbox_msg *msg = (struct sta_mbox_msg *)data;
	struct mbox_test_instance *mbox =
		container_of(mb_client, struct mbox_test_instance, client);
	struct mbox_test_device *mtdev = mbox->mtdev;
	struct mbox_test_msg *pdata = &mbox->rxmsg;
	int i;

	if (!msg || !msg->pdata) {
		dev_err(mtdev->dev, "%s: empty mailbox message\n", __func__);
		return;
	}

	if (msg->dsize >= MBOX_DATA_SIZE)
		dev_warn(mtdev->dev, "%s: data payload truncated\n", __func__);

	dev_dbg(mtdev->dev,
		"%s: Read mbox [%s]:\n- data len [%d]\n",
		__func__, mbox->name, msg->dsize);
	print_hex_dump_debug("-- Payload: ", DUMP_PREFIX_NONE, 16, 1,
			     msg->pdata, msg->dsize, true);

	mutex_lock(&mbox->lock);

	for (i = 0; i < msg->dsize && i < MBOX_DATA_SIZE - 1; i++)
		pdata->data[i] = *(msg->pdata + i);

	/* make sure buffer is null terminated */
	pdata->data[i] = '\0';
	pdata->dlen = i;

	mbox->empty_flag = false;

	mutex_unlock(&mbox->lock);

	/* wake up any blocking processes, waiting for new data */
	wake_up_interruptible(&mbox->readq);
}

static void *mbox_test_chan_request(struct mbox_test_instance *mbox)
{
	struct mbox_test_device *mtdev = mbox->mtdev;

	mbox->client.dev = mtdev->ddev;
	mbox->client.tx_block = true;
	mbox->client.tx_tout = 1000;
	mbox->client.knows_txdone = false;
	mbox->client.rx_callback = mbox_test_callback;
	mbox->client.tx_done = NULL;
	mbox->client.tx_prepare = NULL;

	return mbox_request_channel(&mbox->client, mbox->mtdev->nb);
}

static int mbox_test_open(struct inode *inode, struct file *filp)
{
	struct mbox_test_instance *mbox;
	int ret = 0;

	mbox = kzalloc(sizeof(*mbox), GFP_KERNEL);
	if (!mbox)
		return -ENOMEM;

	mbox->mtdev = &mtdev;
	mutex_init(&mbox->lock);
	init_waitqueue_head(&mbox->readq);
	mbox->empty_flag = true;

	sprintf(mbox->name, "stm:mbox%d", mbox->mtdev->nb);

	mbox->chan = (struct mbox_chan *)mbox_test_chan_request(mbox);
	if (IS_ERR_OR_NULL(mbox->chan)) {
		ret = PTR_ERR(mbox->chan);
		dev_err(mtdev.dev,
			"%s: channel request failed: %d\n", __func__, ret);
		goto free_mbox;
	}

	/* associate filp with the new mbox instance */
	filp->private_data = mbox;

	mutex_lock(&mtdev.lock);
	/* Increase mbox number for next allocation */
	mtdev.nb++;
	list_add(&mbox->next, &mtdev.list);
	mutex_unlock(&mtdev.lock);

	dev_dbg(mtdev.dev, "%s: mailbox %s test driver opened\n",
		__func__, mbox->name);

	return 0;

free_mbox:
	kfree(mbox);
	return ret;
}

static int mbox_test_release(struct inode *inode, struct file *filp)
{
	struct mbox_test_instance *mbox = filp->private_data;
	struct mbox_test_device *mtdev = mbox->mtdev;

	dev_dbg(mtdev->dev, "%s: mailbox %s test driver released\n",
		__func__, mbox->name);

	mbox_free_channel(mbox->chan);

	mutex_lock(&mtdev->lock);
	/* Decrease mbox number allocated */
	mtdev->nb--;
	list_del(&mbox->next);
	mutex_unlock(&mtdev->lock);

	kfree(mbox);

	return 0;
}

static ssize_t mbox_test_read(struct file *filp, char __user *buf,
			      size_t len, loff_t *offp)
{
	struct mbox_test_instance *mbox = filp->private_data;
	struct mbox_test_device *mtdev = mbox->mtdev;
	struct mbox_test_msg *pdata = &mbox->rxmsg;
	int use;

	if (mutex_lock_interruptible(&mbox->lock))
		return -ERESTARTSYS;

	/* nothing to read ? */
	if (mbox->empty_flag) {
		mutex_unlock(&mbox->lock);
		/* non-blocking requested ? return now */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		/* otherwise block, and wait for data */
		if (wait_event_interruptible(mbox->readq, (!mbox->empty_flag)))
			return -ERESTARTSYS;
		if (mutex_lock_interruptible(&mbox->lock))
			return -ERESTARTSYS;
	}
	mbox->empty_flag = true;

	mutex_unlock(&mbox->lock);

	use = min(len, sizeof(*pdata));

	if (copy_to_user(buf, (char *)pdata, use)) {
		dev_err(mtdev->dev, "%s: copy_to_user fail\n", __func__);
		return -EFAULT;
	}

	dev_dbg(mtdev->dev,
		"%s: Read mbox [%s]:\n- data len [%d]\n", __func__,
		mbox->name, pdata->dlen);
	print_hex_dump_debug("-- Payload: ", DUMP_PREFIX_NONE, 16, 1,
			     pdata->data, (size_t)pdata->dlen, true);

	return use;
}

static ssize_t mbox_test_write(struct file *filp, const char __user *ubuf,
			       size_t len, loff_t *offp)
{
	struct mbox_test_instance *mbox = filp->private_data;
	struct mbox_test_device *mtdev = mbox->mtdev;
	struct mbox_test_msg *kdata = &mbox->txmsg;
	struct sta_mbox_msg msg;
	int use, ret;

	/* limit msg size to mbox_test_msg size */
	use = min(sizeof(*kdata), len);

	/* copy the data */
	if (copy_from_user((char *)kdata, ubuf, use))
		return -EMSGSIZE;

	msg.dsize = kdata->dlen;
	msg.pdata = kdata->data;

	ret = mbox_send_message(mbox->chan, (void *)&msg);
	if (ret < 0) {
		dev_err(mtdev->dev,
			"%s: mbox_send_message failed in mbox [%s]\n",
			__func__, mbox->name);
		return -ECOMM;
	}

	dev_dbg(mtdev->dev,
		"%s: Write in mbox [%s]:\n- data len [%d]\n",
		__func__, mbox->name, msg.dsize);
	print_hex_dump_debug("-- Payload: ", DUMP_PREFIX_NONE, 16, 1,
			     msg.pdata, msg.dsize, true);

	return use;
}

static unsigned int mbox_test_poll(struct file *filp,
				   struct poll_table_struct *wait)
{
	struct mbox_test_instance *mbox = filp->private_data;
	unsigned int mask = 0;

	if (mutex_lock_interruptible(&mbox->lock))
		return -ERESTARTSYS;

	poll_wait(filp, &mbox->readq, wait);

	/* Data available for reading */
	if (!mbox->empty_flag)
		mask |= POLLIN | POLLRDNORM;

	/* Device writable without blocking (always true for mailbox) */
	mask |= POLLOUT | POLLWRNORM;

	mutex_unlock(&mbox->lock);

	return mask;
}

static const struct file_operations mbox_test_fops = {
	.open		= mbox_test_open,
	.release	= mbox_test_release,
	.read		= mbox_test_read,
	.write		= mbox_test_write,
	.poll		= mbox_test_poll,
	.owner		= THIS_MODULE,
};

static int mbox_test_probe(struct platform_device *pdev)
{
	int ret, major;

	ret = alloc_chrdev_region(&mbox_test_dev, 0, MAX_MBOX_TEST_DRIVER,
				  KBUILD_MODNAME);
	if (ret) {
		pr_err("%s: alloc_chrdev_region failed: %d\n", __func__, ret);
		return ret;
	}

	major = MAJOR(mbox_test_dev);

	cdev_init(&mtdev.cdev, &mbox_test_fops);
	mtdev.cdev.owner = THIS_MODULE;
	ret = cdev_add(&mtdev.cdev, MKDEV(major, 0), 1);
	if (ret) {
		pr_err("%s: cdev_add failed: %d\n", __func__, ret);
		goto unreg_region;
	}

	mbox_test_class = class_create(THIS_MODULE, KBUILD_MODNAME "_dev");
	if (IS_ERR(mbox_test_class)) {
		ret = PTR_ERR(mbox_test_class);
		pr_err("%s: class_create failed: %d\n", __func__, ret);
		goto clean_cdev;
	}

	mtdev.dev = device_create(mbox_test_class, NULL, MKDEV(major, 0), NULL,
				  "mailbox_test");
	if (IS_ERR(mtdev.dev)) {
		ret = PTR_ERR(mtdev.dev);
		pr_err("%s: device_create failed: %d\n", __func__, ret);
		goto clean_class;
	}

	mtdev.ddev = &pdev->dev;

	INIT_LIST_HEAD(&mtdev.list);
	mutex_init(&mtdev.lock);
	mtdev.nb = 0;
	/* TODO:
	 * record the maximum number of mailboxes (nb of "mboxes "DT
	 * entries) into mtdev
	 */

	dev_info(mtdev.dev, "driver registered\n");

	/* create debugfs entries */
	if (debugfs_initialized()) {
		struct resource *mem;

		mbox_dbg = debugfs_create_dir(KBUILD_MODNAME, NULL);
		if (!mbox_dbg) {
			pr_err("%s: can't create debugfs dir\n", __func__);
			return 0;
		}
		mem = sta_mbox_rp_reg_mem();
		if (mem) {
			mtdev.rp_reg_base = ioremap(mem->start,
					resource_size(mem));
			debugfs_create_file("Remote_IRSR", 0400, mbox_dbg,
					    &mtdev, &mbox_rp_irsr_fops);
			debugfs_create_file("Remote_IPSR", 0400, mbox_dbg,
					    &mtdev, &mbox_rp_ipsr_fops);
			debugfs_create_file("Remote_IMR", 0400, mbox_dbg,
					    &mtdev, &mbox_rp_imr_fops);
		}

		mem = sta_mbox_reg_mem();
		if (mem) {
			mtdev.host_reg_base = ioremap(mem->start,
					resource_size(mem));
			debugfs_create_file("Host_IRSR", 0400, mbox_dbg,
					    &mtdev, &mbox_host_irsr_fops);
			debugfs_create_file("Host_IPSR", 0400, mbox_dbg,
					    &mtdev, &mbox_host_ipsr_fops);
			debugfs_create_file("Host_IMR", 0400, mbox_dbg,
					    &mtdev, &mbox_host_imr_fops);
		}
	}

	return 0;

clean_class:
	class_destroy(mbox_test_class);
clean_cdev:
	cdev_del(&mtdev.cdev);
unreg_region:
	unregister_chrdev_region(mbox_test_dev, MAX_MBOX_TEST_DRIVER);
	return ret;
}

static int mbox_test_remove(struct platform_device *pdev)
{
	struct mbox_test_instance *mbox, *tmp;
	int major = MAJOR(mbox_test_dev);

	list_for_each_entry_safe(mbox, tmp, &mtdev.list, next) {
		list_del(&mbox->next);
		kfree(mbox);
	}

	device_destroy(mbox_test_class, MKDEV(major, 0));
	cdev_del(&mtdev.cdev);
	class_destroy(mbox_test_class);
	unregister_chrdev_region(mbox_test_dev, MAX_MBOX_TEST_DRIVER);

	debugfs_remove(mbox_dbg);
	return 0;
}

static const struct of_device_id mbox_test_match[] = {
	{ .compatible = "st,mailbox-test" },
	{},
};

static struct platform_driver mbox_test_driver = {
	.driver = {
		.name = "mailbox_sta_test",
		.of_match_table = mbox_test_match,
	},
	.probe  = mbox_test_probe,
	.remove = mbox_test_remove,
};
module_platform_driver(mbox_test_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("mailbox test driver");
MODULE_AUTHOR("Olivier Lebreton <olivier.lebreton@st.com>");
