/*
 * Copyright (C) 2013 STMicroelectronics
 *
 * License Terms: GNU General Public License v2
 * Author: Loic Pallardy <loic.pallardy@st.com> for STMicroelectronics
 * Author: Olivier Lebreton <olivier.lebreton@st.com> for STMicroelectronics
 * STA Mailbox driver
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kfifo.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/irqdomain.h>
#include <linux/mailbox_sta.h>
#include <linux/hwspinlock.h>
#include <linux/sta_hsem.h>
#include <linux/device.h>
#include <linux/mailbox_controller.h>

/* CPU mailbox registers */
#define REG_MBOX_IRSR_SET	0x00
#define REG_MBOX_IPSR_VAL	0x04
#define REG_MBOX_IMR_MSK	0x08
#define REG_MBOX_REMOTE_VAL	0x04
#define REG_MBOX_UNUSED		0xFFFF

#define MBOX_DEFAULT_RX_SIZE	0x10
#define MBOX_DEFAULT_TX_SIZE	0x10

#define STA_MAILBOX		1
#define STA_MAILBOX_HSEM	2	/* HSEM mailbox */

#define MBOX_BIT		BIT
#define ALL_MBOX_BITS(_nb)	(MBOX_BIT(_nb) - 1)

#define MAILBOX_MAX_SIZE	((u8)(~0U))

#define MAILBOX_NB_MSG		8
#define MAILBOX_KFIFO_SIZE(m)	(MAILBOX_NB_MSG * (m))

static u32 sta_mbox_irq_mask;		/* masked by default */
static DEFINE_SPINLOCK(sta_mbox_irqs_lock);

/**
 * struct sta_mbox_attribute - STA mailbox device attribute info
 * @type:		mailbox device type: mailbox or hsem IP
 * @num_mbox:		maximum number of mailbox supported by the HW IP
 * @reg_set:		register offset to generate mailbox interrupt
 * @reg_clr:		register offset to clear pending interrupt
 * @reg_val:		register offset to read pending interrupt
 * @reg_ack:		register offset to read remote pending interrupt
 * @reg_msk:		register offset to mask mailbox interrupt
 */
struct sta_mbox_attribute {
	int		type;
	int		num_mbox;
	int		reg_set;
	int		reg_clr;
	int		reg_val;
	int		reg_ack;
	int		reg_msk;
};

/**
 * struct sta_mbox_data - STA mailbox buffer attribute info
 * @rx_offset:		rx mailbox offset in shared memory
 * @rx_size:		rx mailbox size in shared memory
 * @tx_offset:		tx mailbox offset in shared memory
 * @tx_size:		tx mailbox size in shared memory
 */
struct sta_mbox_data {
	u32		rx_offset;
	u32		rx_size;
	u32		tx_offset;
	u32		tx_size;
};

/**
 * struct sta_mbox_queue - STA mailbox queue parameters
 * @lock:	spinlock for mailbox queue parameters access
 * @fifo:	the fifo to be used for STA mailbox data
 * @msg:	message buffer for rx data
 * @full:	Fifo full flag notification
 */
struct sta_mbox_queue {
	/* mailbox queue parameter access */
	spinlock_t		lock;
	struct kfifo		fifo;
	struct sta_mbox_msg	msg;
	bool			full;
};

/**
 * struct sta_mbox_dev_data - STA mailbox info
 * @irq:		interrupt number related to mbox irq domain
 * @mb_data:		mailbox buffer attribute info
 * @mrxq:		mailbox rx queue info
 * @parent:		mailbox device to which it is attached
 * @hsem:		hsem instance used by the current mailbox
 * @link:		representation of a communication link
 */
struct sta_mbox_dev_data {
	unsigned int			id;
	unsigned int			irq;
	struct sta_mbox_data		mdata;
	struct sta_mbox_queue		*mrxq;
	struct sta_mbox_device		*parent;
	struct hwspinlock		*hsem;
	char				name[32];
};

/**
 * struct sta_mbox_device - STA mailbox device data
 * @irq:		Interrupt number assigned to mailbox
 * @mboxes:		array of rx mailbox device attributes
 * @cfg:		mailbox attribute data
 * @dev:		device to which it is attached
 * @mbox_reg_base:	base address of the register mapping region
 * @mbox_reg_remote:	base address of the remote register mapping region
 * @mbox_mem_base:	base address of the shared memory mapping region
 * @mbox_mem_len:	size of the shared memory mapping region
 * @controller:		representation of a mailbox controller
 */
struct sta_mbox_device {
	int				irq;
	struct sta_mbox_dev_data	*mboxes;
	struct sta_mbox_attribute	*cfg;
	struct device			*dev;
	void __iomem			*mbox_reg_base;
	void __iomem			*mbox_reg_remote;
	void __iomem			*mbox_mem_base;
	resource_size_t			mbox_mem_len;
	struct mbox_controller		controller;
};

static inline struct sta_mbox_dev_data
	*chan_to_sta_mbox(struct mbox_chan *chan)
{
	if (!chan || !chan->con_priv)
		return NULL;

	return (struct sta_mbox_dev_data *)chan->con_priv;
}

static inline unsigned int mbox_read_reg(void __iomem *reg)
{
	return __raw_readl(reg);
}

static inline void mbox_write_reg(u32 val, void __iomem *reg)
{
	__raw_writel(val, reg);
}

static inline u32 mbox_mask_to_id(u32 bitmask)
{
	u32 i;

	for (i = 0; i <= 15; i++)
		if (((bitmask >> i) &  0x1) == 1)
			break;
	return i;
}

/* debugfs management - Export register base address */
static struct resource *sta_mbox_reg_base;
static struct resource *sta_mbox_rp_reg_base;

struct resource *sta_mbox_reg_mem(void)
{
	return sta_mbox_reg_base;
}
EXPORT_SYMBOL(sta_mbox_reg_mem);

struct resource *sta_mbox_rp_reg_mem(void)
{
	return sta_mbox_rp_reg_base;
}
EXPORT_SYMBOL(sta_mbox_rp_reg_mem);

/* Mailbox H/W preparations */
static struct irq_chip sta_mbox_irq_chip;
static struct irq_domain *sta_mbox_irq_domain;

/*
 * message management
 */

static int sta_mbox_read(struct sta_mbox_dev_data *mbox,
			 struct sta_mbox_msg *msg)
{
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_data *md = &mbox->mdata;
	int size;
	u8 *hdr;

	size = 0;

	/* read data payload */
	hdr = (u8 *)(mdev->mbox_mem_base + md->rx_offset);
	msg->dsize = readb((void __iomem *)hdr);
	msg->pdata = ++hdr;
	size += (msg->dsize + sizeof(msg->dsize));

	return size;
}

static void sta_mbox_write(struct sta_mbox_dev_data *mbox, void *data)
{
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_data *md = &mbox->mdata;
	struct sta_mbox_msg *msg = (struct sta_mbox_msg *)data;
	u8 *wrb, *hdr;
	u32 len;
	int j;

	/* Write payload */
	hdr = (u8 *)(mdev->mbox_mem_base + md->tx_offset);
	/* Payload size */
	/* Size (1 byte max) stored in the 1st byte of the buffer */
	len = min((u32)msg->dsize, (md->tx_size - sizeof(*hdr)));
	writeb((u8)len, (void __iomem *)hdr);

	/* Payload data */
	for (j = 0, wrb = msg->pdata, hdr++;
				wrb && j < len; j++, wrb++, hdr++) {
		writeb(*wrb, (void __iomem *)hdr);
	}
}

static void
sta_mbox_memset(struct sta_mbox_device *mdev, u8 data, int off, int size)
{
	int j;

	for (j = 0; j < size && j < mdev->mbox_mem_len; j++)
		writeb(data, mdev->mbox_mem_base + off + j);
}

/*
 * Mailbox IRQ handle functions
 */

static void sta_mbox_enable_irq(struct sta_mbox_device *mdev, u32 msk)
{
	struct sta_mbox_attribute *p = mdev->cfg;
	unsigned long flags;
	u32 bits = msk;

	spin_lock_irqsave(&sta_mbox_irqs_lock, flags);

	if (p->reg_msk != REG_MBOX_UNUSED) {
		bits |= mbox_read_reg(mdev->mbox_reg_base + p->reg_msk);
		mbox_write_reg(bits, mdev->mbox_reg_base + p->reg_msk);
	} else {
		/* Manage irq mask in SW if no register available */
		bits |= sta_mbox_irq_mask;
	}

	sta_mbox_irq_mask = bits;

	spin_unlock_irqrestore(&sta_mbox_irqs_lock, flags);
}

static void sta_mbox_disable_irq(struct sta_mbox_device *mdev, u32 msk)
{
	struct sta_mbox_attribute *p = mdev->cfg;
	unsigned long flags;
	u32 bits = ~msk;

	spin_lock_irqsave(&sta_mbox_irqs_lock, flags);

	if (p->reg_msk != REG_MBOX_UNUSED) {
		bits &= mbox_read_reg(mdev->mbox_reg_base + p->reg_msk);
		mbox_write_reg(bits, mdev->mbox_reg_base + p->reg_msk);
	} else {
		/* Manage irq mask in SW if no register available */
		bits &= sta_mbox_irq_mask;
	}

	sta_mbox_irq_mask = bits;

	spin_unlock_irqrestore(&sta_mbox_irqs_lock, flags);
}

static void sta_mbox_clr_irq(struct sta_mbox_device *mdev, u32 msk)
{
	struct sta_mbox_attribute *p = mdev->cfg;
	unsigned long flags;
	u32 bits;

	/* HSEM IP used to generate IRQ to remote processor */
	if (p->type == STA_MAILBOX_HSEM) {
		struct sta_mbox_dev_data *mbox;
		u32 id = mbox_mask_to_id(msk);

		if (msk != ALL_MBOX_BITS(mdev->cfg->num_mbox) &&
		    id < mdev->controller.num_chans) {
			mbox = mdev->mboxes + id;
			sta_hsem_irq_clr(mbox->hsem, msk);
		}
	} else {
	/* Mailbox IP used to generate IRQ to remote processor */
		spin_lock_irqsave(&sta_mbox_irqs_lock, flags);

		bits = mbox_read_reg(mdev->mbox_reg_base + p->reg_clr);

		/* Mailbox requires writing to 0 the pending interrupt */
		bits &= ~msk;

		mbox_write_reg(bits, mdev->mbox_reg_base + p->reg_clr);

		spin_unlock_irqrestore(&sta_mbox_irqs_lock, flags);
	}
}

static void sta_mbox_send_event(struct sta_mbox_dev_data *mbox, u32 msk)
{
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_attribute *p = mdev->cfg;
	unsigned long flags;
	u32 bits;

	spin_lock_irqsave(&sta_mbox_irqs_lock, flags);

	/* HSEM IP used to access to virtual mailbox register */
	if (p->type == STA_MAILBOX_HSEM) {
		while (sta_hsem_trylock(mbox->hsem) == -EBUSY)
			;
		sta_hsem_unlock(mbox->hsem);
	} else {
		bits = mbox_read_reg(mdev->mbox_reg_base + p->reg_set) | msk;
		mbox_write_reg(bits, mdev->mbox_reg_base + p->reg_set);
	}

	spin_unlock_irqrestore(&sta_mbox_irqs_lock, flags);
}

static irqreturn_t sta_mbox_thread_interrupt(int irq, void *data)
{
	struct mbox_chan *chan = (struct mbox_chan *)data;
	struct sta_mbox_dev_data *mbox = chan_to_sta_mbox(chan);
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_queue *mq = mbox->mrxq;
	struct sta_mbox_msg *msg = &mq->msg;
	int len;

	while (!kfifo_is_empty(&mq->fifo)) {
		len = kfifo_out(&mq->fifo, (u8 *)&msg->dsize,
				sizeof(msg->dsize));
		WARN_ON(len != sizeof(msg->dsize));
		WARN_ON(msg->dsize > mbox->mdata.rx_size);
		len = kfifo_out(&mq->fifo, msg->pdata, msg->dsize);
		WARN_ON(len != msg->dsize);

		mbox_chan_received_data(chan, (void *)msg);
	}

	/* Fifo was full so now it is emptied the last received message can be
	 * read and the mailbox interrupt re-enabled
	 */
	spin_lock_irq(&mq->lock);
	if (mq->full) {
		struct sta_mbox_msg rxmsg;

		sta_mbox_read(mbox, &rxmsg);
		msg->dsize = rxmsg.dsize;
		memcpy(msg->pdata, rxmsg.pdata, rxmsg.dsize);

		mq->full = false;
		sta_mbox_enable_irq(mdev, MBOX_BIT(mbox->id));
		spin_unlock_irq(&mq->lock);

		mbox_chan_received_data(chan, (void *)msg);
	} else {
		spin_unlock_irq(&mq->lock);
	}

	return IRQ_HANDLED;
}

static irqreturn_t sta_mbox_interrupt(int irq, void *data)
{
	struct mbox_chan *chan = (struct mbox_chan *)data;
	struct sta_mbox_dev_data *mbox = chan_to_sta_mbox(chan);
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_queue *mq = mbox->mrxq;
	struct sta_mbox_msg msg;
	int size, len;

	if (!mbox)
		pr_err("mbox is NULL\n");

	/* read mailbox data */
	size = sta_mbox_read(mbox, &msg);

	if (unlikely(kfifo_avail(&mq->fifo) < size)) {
		sta_mbox_disable_irq(mdev, MBOX_BIT(mbox->id));
		mq->full = true;
		dev_warn(mdev->dev,
			 "Mailbox %s kfifo full\n", mbox->name);
		return IRQ_WAKE_THREAD;
	}
	len = kfifo_in(&mq->fifo, (u8 *)&msg.dsize, sizeof(msg.dsize));
	WARN_ON(len != sizeof(msg.dsize));
	len = kfifo_in(&mq->fifo, msg.pdata, msg.dsize);
	WARN_ON(len != msg.dsize);

	return IRQ_WAKE_THREAD;
}

static void sta_mbox_irq_rxmsg(void *priv, void *data)
{
	generic_handle_irq(irq_find_mapping(sta_mbox_irq_domain, (int)data));
}

static irqreturn_t sta_mbox_irq_handler(int irq, void *data)
{
	struct sta_mbox_device *mdev = (struct sta_mbox_device *)data;
	u32 bits, msk, n;

	bits = mbox_read_reg(mdev->mbox_reg_base + mdev->cfg->reg_val);

	if (unlikely(!bits))
		return IRQ_NONE;

	for (n = 0; bits; n++) {
		msk = MBOX_BIT(n);
		if (bits & msk) {
			bits -= msk;
			if (sta_mbox_irq_mask & msk)
				sta_mbox_irq_rxmsg(NULL, (void *)n);
			else
				dev_warn(mdev->dev,
					 "Unexpected mailbox interrupt\n");

			sta_mbox_clr_irq(mdev, msk);
		}
	}
	return IRQ_HANDLED;
}

/*
 * link management
 */

static
struct sta_mbox_queue *sta_mbox_queue_alloc(struct sta_mbox_dev_data *mbox)
{
	struct sta_mbox_queue *mq;
	u8 *payload;
	unsigned int size;

	mq = kzalloc(sizeof(*mq), GFP_KERNEL);
	if (!mq)
		return NULL;

	spin_lock_init(&mq->lock);

	payload = kzalloc(mbox->mdata.rx_size, GFP_KERNEL);
	if (!payload)
		goto free_mq;
	mq->msg.pdata = payload;

	size = MAILBOX_KFIFO_SIZE(mbox->mdata.rx_size);
	if (kfifo_alloc(&mq->fifo, size, GFP_KERNEL))
		goto free_pdata;

	return mq;

free_pdata:
	kfree(mq->msg.pdata);
free_mq:
	kfree(mq);
	return NULL;
}

static void sta_mbox_queue_free(struct sta_mbox_queue *mq)
{
	kfifo_free(&mq->fifo);
	kfree(mq->msg.pdata);
	kfree(mq);
}

static int sta_mbox_startup(struct mbox_chan *chan)
{
	struct sta_mbox_dev_data *mbox = chan_to_sta_mbox(chan);
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_queue *mq;
	int ret;

	mq = sta_mbox_queue_alloc(mbox);
	if (!mq)
		return -ENOMEM;
	mbox->mrxq = mq;

	/* IRQ automatically enabled */
	ret = request_threaded_irq(mbox->irq,
				   sta_mbox_interrupt,
				   sta_mbox_thread_interrupt,
				   IRQF_SHARED, mbox->name, chan);
	if (unlikely(ret)) {
		dev_err(mdev->dev,
			"failed to register mailbox interrupt:%d\n", ret);
		goto failed_out;
	}

	/* Get HW Semaphore if required */
	if (mdev->cfg->type == STA_MAILBOX_HSEM) {
		mbox->hsem = sta_hsem_lock_request(mbox->id, HSEM_INTM3,
				NULL, sta_mbox_irq_rxmsg);

		if (!mbox->hsem) {
			dev_err(mdev->dev,
				"failed to request HW Semaphore\n");
			ret = -ENODEV;
			goto failed_out;
		}
	}

	dev_dbg(mdev->dev, "Mailbox %s [%d] startup\n", mbox->name, mbox->id);

	/* ipc free channel is called by mailbox core in case of error */
	/* So irq and mbox queue will be released accordingly */
failed_out:
	return ret;
}

static void sta_mbox_shutdown(struct mbox_chan *chan)
{
	struct sta_mbox_dev_data *mbox = chan_to_sta_mbox(chan);
	struct sta_mbox_device *mdev = mbox->parent;

	/* IRQ automatically disabled */
	free_irq(mbox->irq, chan);

	sta_mbox_queue_free(mbox->mrxq);

	sta_hsem_lock_free(mbox->hsem);

	dev_dbg(mdev->dev, "Mailbox %s [%d] shutdown\n", mbox->name, mbox->id);
}

static bool sta_mbox_last_tx_done(struct mbox_chan *chan)
{
	struct sta_mbox_dev_data *mbox = chan_to_sta_mbox(chan);
	struct sta_mbox_device *mdev = mbox->parent;
	struct sta_mbox_attribute *p = mdev->cfg;
	u32 bits;

	if (p->type == STA_MAILBOX_HSEM)
		/* Fetch HSEM IP irq status register */
		bits = sta_hsem_irq_status(mbox->hsem, HSEM_INTM3)
						& MBOX_BIT(mbox->id);
	else
		/* Fetch Mailbox IP irq status register */
		bits = mbox_read_reg(mdev->mbox_reg_remote + p->reg_ack)
						& MBOX_BIT(mbox->id);

	if (bits)
		return false;
	else
		return true;
}

static int sta_mbox_send_data(struct mbox_chan *chan, void *data)
{
	struct sta_mbox_dev_data *mbox = chan_to_sta_mbox(chan);

	if (!data)
		return -EINVAL;

	if (sta_mbox_last_tx_done(chan)) {
		sta_mbox_write(mbox, data);

		/* send event */
		sta_mbox_send_event(mbox, MBOX_BIT(mbox->id));

		return 0;
	}
	return -EBUSY;
}

static const struct mbox_chan_ops sta_mbox_ops = {
	.startup	= sta_mbox_startup,
	.send_data	= sta_mbox_send_data,
	.shutdown	= sta_mbox_shutdown,
	.last_tx_done	= sta_mbox_last_tx_done,
};

/*
 * interrupt domain management
 */

/*  mask/unmask must be managed by SW */

static void mbox_irq_enable(struct irq_data *d)
{
	struct sta_mbox_device *mdev =
			(struct sta_mbox_device *)d->domain->host_data;

	sta_mbox_enable_irq(mdev, MBOX_BIT(d->hwirq));
}

static void mbox_irq_disable(struct irq_data *d)
{
	struct sta_mbox_device *mdev =
			(struct sta_mbox_device *)d->domain->host_data;

	sta_mbox_disable_irq(mdev, MBOX_BIT(d->hwirq));
}

static void mbox_irq_ack(struct irq_data *d)
{
	struct sta_mbox_device *mdev =
			(struct sta_mbox_device *)d->domain->host_data;

	sta_mbox_clr_irq(mdev, MBOX_BIT(d->hwirq));
}

static struct irq_chip sta_mbox_irq_chip = {
	.name		= "sta_mbox",
	.irq_enable	= mbox_irq_enable,
	.irq_disable	= mbox_irq_disable,
	.irq_ack	= mbox_irq_ack,
	.irq_mask	= mbox_irq_disable,
	.irq_unmask	= mbox_irq_enable,
};

static int sta_mbox_irq_map(struct irq_domain *d, unsigned int irq,
			    irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &sta_mbox_irq_chip, handle_simple_irq);

	return 0;
}

static const struct irq_domain_ops sta_mbox_irq_ops = {
	.map    = sta_mbox_irq_map,
	.xlate  = irq_domain_xlate_onecell,
};

static const struct sta_mbox_attribute mbox_config1 = {
	.type		= STA_MAILBOX,
	.num_mbox	= 16,
	.reg_set	= REG_MBOX_IRSR_SET,
	.reg_clr	= REG_MBOX_IPSR_VAL,
	.reg_val	= REG_MBOX_IPSR_VAL,
	.reg_ack	= REG_MBOX_REMOTE_VAL,
	.reg_msk	= REG_MBOX_IMR_MSK,
};

static const struct sta_mbox_attribute mbox_config2 = {
	.type		= STA_MAILBOX_HSEM,
	.num_mbox	= HSEM_FREE_MAX_ID,
	.reg_set	= REG_MBOX_UNUSED,
	.reg_clr	= REG_MBOX_UNUSED,
	.reg_val	= REG_MBOX_UNUSED,
	.reg_ack	= REG_MBOX_UNUSED,
	.reg_msk	= REG_MBOX_UNUSED,
};

static const struct of_device_id sta_mailbox_match[] = {
	{ .compatible = "st,sta-mailbox",
		.data = (void *)&mbox_config1,
	},
	{ .compatible = "st,sta-mailboxhsem",
		.data = (void *)&mbox_config2,
	},
	{ /* sentinel */}
};

static int sta_mbox_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct resource *mem;
	struct mbox_chan *chans;
	struct sta_mbox_attribute *cfg;
	struct sta_mbox_dev_data *mbox, *mboxblk;
	struct sta_mbox_device *mdev;
	const char *name;
	int ret, i, rx_size, tx_size;
	u32 data_len, max_off;

	if (!np) {
		/* No DT device creation */
		dev_err(&pdev->dev, "%s: platform not supported\n", __func__);
		return -ENODEV;
	}

	cfg = (struct sta_mbox_attribute *)of_match_device(
				sta_mailbox_match, &pdev->dev)->data;
	if (!cfg) {
		/* No mailbox configuration */
		dev_err(&pdev->dev, "No configuration found\n");
		return -ENODEV;
	}

	/* Check mailbox configuration from DT */

	ret = of_property_read_u32_index(np, "st,mbox-rx-size", 0, &rx_size);
	if (ret)
		rx_size = MBOX_DEFAULT_RX_SIZE;

	ret = of_property_read_u32_index(np, "st,mbox-tx-size", 0, &tx_size);
	if (ret)
		tx_size = MBOX_DEFAULT_TX_SIZE;

	mdev = devm_kzalloc(&pdev->dev, sizeof(*mdev), GFP_KERNEL);
	if (!mdev)
		return -ENOMEM;

	/* allocate one extra for marking end of list */
	chans = devm_kzalloc(&pdev->dev, (cfg->num_mbox + 1) * sizeof(*chans),
			     GFP_KERNEL);
	if (!chans)
		return -ENOMEM;

	mboxblk = devm_kzalloc(&pdev->dev, cfg->num_mbox * sizeof(*mbox),
			       GFP_KERNEL);
	if (!mboxblk)
		return -ENOMEM;

	mem = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mbox-shm");
	if (!mem)
		return -ENOENT;
	if (resource_size(mem) < (cfg->num_mbox * (rx_size + tx_size))) {
		dev_err(&pdev->dev,
			"mbox-shm too small for selected configuration\n");
		return -EINVAL;
	}
	if (!devm_request_mem_region(&pdev->dev, mem->start, resource_size(mem),
				     pdev->name)) {
		dev_err(&pdev->dev, "mbox-shm: region already claimed\n");
		return -EBUSY;
	}
	mdev->mbox_mem_base = devm_ioremap(&pdev->dev, mem->start,
						resource_size(mem));
	if (!mdev->mbox_mem_base)
		return -ENOMEM;
	mdev->mbox_mem_len = resource_size(mem);
	sta_mbox_memset(mdev, 0xA5, 0, mdev->mbox_mem_len);

	/*
	 * If HSEM IP is selected in DT to generate IRQ to remote processor
	 * then Mailbox HW IP is no more used and therefore initialisation of
	 * its registers and IRQ are useless.
	 */
	if (cfg->type != STA_MAILBOX_HSEM) {
		/*
		 * Mailbox IP is used to generate IRQ to remote processor
		 */
		/* Init Mailbox local processor register */
		mem = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						   "mbox-reg");
		if (!mem)
			return -ENOENT;
		if (!devm_request_mem_region(&pdev->dev, mem->start,
					     resource_size(mem), pdev->name)) {
			dev_err(&pdev->dev,
				"mbox-reg: region already claimed\n");
			return -EBUSY;
		}
		sta_mbox_reg_base = mem;
		mdev->mbox_reg_base = devm_ioremap(&pdev->dev, mem->start,
						   resource_size(mem));
		if (!mdev->mbox_reg_base)
			return -ENOMEM;

		/* Init Mailbox Remote processor register */
		mem = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						   "mbox-rp-reg");
		if (!mem) {
			/* Optional remote mbox registers not defined in DT */
			dev_dbg(&pdev->dev,
				"Remote mailbox registers not specified\n");
			mdev->mbox_reg_remote = mdev->mbox_reg_base;
			sta_mbox_rp_reg_base = sta_mbox_reg_base;
		} else {
			if (!devm_request_mem_region(&pdev->dev, mem->start,
						     resource_size(mem),
						     pdev->name)) {
				dev_err(&pdev->dev,
					"mbox-rp-reg region already claimed\n");
				return -EBUSY;
			}
			mdev->mbox_reg_remote = devm_ioremap(&pdev->dev,
						mem->start, resource_size(mem));
			if (!mdev->mbox_reg_remote)
				return -ENOMEM;
			sta_mbox_rp_reg_base = mem;
		}

		/* Init Mailbox IRQ */
		mdev->irq = platform_get_irq(pdev, 0);
		ret = devm_request_irq(&pdev->dev,
				       mdev->irq, sta_mbox_irq_handler,
				       IRQF_NO_SUSPEND, "sta_mbox", mdev);
		if (ret)
			return -EINVAL;
	}

	/*
	 * Mailbox IRQ domain still required whatever is the IRQ source:
	 * Mailbox IP or HSEM IP
	 */
	sta_mbox_irq_domain = irq_domain_add_linear(np, cfg->num_mbox,
						    &sta_mbox_irq_ops, mdev);
	if (!sta_mbox_irq_domain) {
		dev_err(&pdev->dev, "Failed to create irqdomain\n");
		return -ENOMEM;
	}

	mbox = mboxblk;

	for (i = 0, data_len = 0, max_off = 0; i < cfg->num_mbox; i++, mbox++) {
		mbox->id = i;
		mbox->irq = irq_create_mapping(sta_mbox_irq_domain, mbox->id);
		snprintf(mbox->name, 16, "mbox%d", i);

		/* Configure shared memory usage */
		mbox->mdata.rx_offset = rx_size * i;
		mbox->mdata.rx_size = rx_size;
		mbox->mdata.tx_offset = cfg->num_mbox * rx_size + tx_size * i;
		mbox->mdata.tx_size = tx_size;

		if (mbox->mdata.rx_size > MAILBOX_MAX_SIZE ||
		    mbox->mdata.tx_size > MAILBOX_MAX_SIZE) {
			dev_err(&pdev->dev,
				"mbox[%d] size higher than required\n", i);
			return -ENODEV;
		}
		data_len += (mbox->mdata.rx_size + mbox->mdata.tx_size);
		max_off = max(max_off,
			      (mbox->mdata.rx_offset + mbox->mdata.rx_size));
		max_off = max(max_off,
			      (mbox->mdata.tx_offset + mbox->mdata.tx_size));

		mbox->parent = mdev;
		chans[i].con_priv = (struct sta_mbox_dev_data *)mbox;
	}
	/* mark end of list */
	/* chans[i] = NULL; */

	if (data_len > mdev->mbox_mem_len || max_off > mdev->mbox_mem_len) {
		dev_err(&pdev->dev,
			"Mailbox (len[%d], off[%d]) out of memory: limit= %d\n",
			data_len, max_off, mdev->mbox_mem_len);
		return -ENOMEM;
	}

	mdev->mboxes = mboxblk;
	mdev->cfg = cfg;
	mdev->dev = &pdev->dev;

	/* STA mailbox does not have a Tx-Done or Tx-Ready IRQ */
	mdev->controller.txdone_irq = false;
	mdev->controller.txdone_poll = true;
	mdev->controller.txpoll_period = 1;
	mdev->controller.ops = &sta_mbox_ops;
	mdev->controller.chans = chans;
	mdev->controller.num_chans = cfg->num_mbox;
	mdev->controller.dev = mdev->dev;

	ret = mbox_controller_register(&mdev->controller);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, mdev);

	/* clean all mailboxes */
	sta_mbox_disable_irq(mdev, ALL_MBOX_BITS(mdev->cfg->num_mbox));
	sta_mbox_clr_irq(mdev, ALL_MBOX_BITS(mdev->cfg->num_mbox));

	name = (const char *)of_match_device(
				sta_mailbox_match, &pdev->dev)->compatible;
	dev_info(&pdev->dev, "driver %s with [%d] mboxes registered\n",
		 name, mdev->controller.num_chans);

	return 0;
}

static int sta_mbox_remove(struct platform_device *pdev)
{
	struct sta_mbox_device *mdev = platform_get_drvdata(pdev);

	mbox_controller_unregister(&mdev->controller);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sta_mbox_driver = {
	.probe = sta_mbox_probe,
	.remove = sta_mbox_remove,
	.driver = {
		.name = "sta-mailbox",
		.of_match_table = sta_mailbox_match,
	},
};

static int __init sta_mbox_init(void)
{
	return platform_driver_register(&sta_mbox_driver);
}

static void __exit sta_mbox_exit(void)
{
	platform_driver_unregister(&sta_mbox_driver);
}

postcore_initcall(sta_mbox_init);
module_exit(sta_mbox_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("STMicroelectronics mailbox: st architecture specific functions");
MODULE_AUTHOR("Loic Pallardy <loic.pallardy@st.com>");
MODULE_AUTHOR("Olivier Lebreton <olivier.lebreton@st.com>");
MODULE_ALIAS("platform:sta-mailbox");
