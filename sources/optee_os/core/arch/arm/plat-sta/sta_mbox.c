/*
 *
 * (C) Copyright 2016/2017 ST-microlectronics ADG
 *
 */
#include <kernel/cache_helpers.h>
#include <kernel/interrupt.h>
#include <stdint.h>
#include <string.h>
#include <io.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <trace.h>
#include <util.h>

#include <mbox.h>
#include <keep.h>

#define MBOX_MAX_NUMBER			16

/* AP MBOX Registers offsets */
#define MBOX_IRSR			0x00
#define MBOX_IPSR			0x04
#define MBOX_IMR			0x08

/**
 * struct mbox_input_desc - Mailbox Input description
 * @name:       mailbox name
 * @tx_offset:	tx mailbox offset in shared memory
 * @tx_size:	tx mailbox size in shared memory
 * @rx_offset:	rx mailbox offset in shared memory
 * @rx_size:	rx mailbox size in shared memory
 */
struct mbox_input_desc {
	char *name;
	uint32_t rx_offset;
	uint32_t rx_size;
	uint32_t tx_offset;
	uint32_t tx_size;
};

/**
 * struct mbox_chnl - Mailbox channel allocated by user
 * @user_data: user specific data, mbox driver doesn't touch it
 * @rx_cb: atomic callback to provide user the data received
 * @assigned: Flag set if mailbox channel is assigned
 */
struct mbox_chnl {
	void *user_data;
	void (*rx_cb)(void *user_data, struct mbox_msg *msg);
	bool assigned;
	uint8_t rx_stop;
};

/**
 * struct mbox_config - Mailbox Configuration data
 * id:		mailbox physical id in mailbox device
 * @mdesc:	mailbox buffer attribute info
 * @mchan:	mailbox channel request info
 * @mbuf:	mailbox buffer
 * @dev:	mbox device that mbox config belongs to
 */
struct mbox_config {
	uint32_t id;
	struct mbox_input_desc mdesc;
	struct mbox_chnl mchan;
	struct mbox_msg mbuf;
	struct mbox_device *dev;
};

/**
 * struct mbox_device - Mailbox device data
 * @name:		mailbox device name
 * @type:		can be either MBOX_HWIP or HSEM_HWIP
 * @mboxes:		array of rx mailbox device attributes
 * @nb_mboxes:		number of mailboxes in device
 * @mem_base:		base address of the shared memory mapping region
 * @reg_base:		base address of the M3 mailbox registers
 * @irq:		mailbox irq line
 * @init_done:		status whether device init is done
 * @buf_size:		size of rx, tx channel buffers
 */
struct mbox_device {
	const char *name;
	struct mbox_config *mboxes;
	uint32_t nb_mboxes;
	volatile uint8_t *mem_base;
	uint32_t reg_base;
	const uint32_t irq;
	bool init_done;
	const uint32_t buf_size;
};

static struct mbox_config mboxes[MBOX_MAX_NUMBER] = {
	{ .mdesc = {"psci",}, },
	{ .mdesc = {"regmap", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }  },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
	{ .mdesc = {"unused", }, },
};

static struct mbox_device mbox_dev = {
	.name = "std mailbox",
	.mboxes = mboxes,
	.nb_mboxes = MBOX_MAX_NUMBER,
	.mem_base = NULL,
	.reg_base = 0,
	.irq = 0, /* TO BE FILLED */
	.init_done = false,
	.buf_size = 20,
};

/**
 * @brief - Generate event to trigger IRQ on remote processor
 * @param mbox Mailbox pointer on which a message bas to be sent
 * @return none
 */
static inline void mbox_send_event(struct mbox_config *mbox)
{
	uint32_t val = read32(mbox->dev->reg_base + MBOX_IRSR);

	write32(val | BIT(mbox->id), mbox->dev->reg_base + MBOX_IRSR);
}

/**
 * @brief - Mailbox ISR for Interrupt handler framework
 * @param  Pointer on itr_handler structure to get input data for isr handling
 * @return ITRR_HANDLED or ITRR_NONE
 */
static enum itr_return mbox_itr_cb(struct itr_handler *h __unused)
{
	return ITRR_HANDLED;
}

static struct itr_handler mbox_itr = {
	.it = MBOX_A7_M3_ID,
	.handler = mbox_itr_cb,
};

KEEP_PAGER(mbox_itr);

/**
 * @brief - Init mailbox driver
 *              - Set mailbox register and configure shared memory buffer
 *              - Enable mailbox interrupt
 * @return None
 */
int mbox_init(void)
{
	struct mbox_device *dev = &mbox_dev;
	struct mbox_config *mbox;
	uint32_t i;

	if (dev->init_done)
		return 0;

	/* Get virtual device base addresses */
	dev->mem_base = (volatile uint8_t *)phys_to_virt_io(IPC_A7_M3_BASE);
	dev->reg_base = (uint32_t)phys_to_virt_io(MBOX_A7_M3_BASE);

	/*
	 * Register Mailbox interrupt to configure it as secure
	 * This interrupt has to only be enabled if needed
	 */
	itr_add(&mbox_itr);
	//itr_enable(MBOX_A7_M3_ID);

	/* Clean all mailboxes */
	write32(0, dev->reg_base + MBOX_IMR);
	write32(0, dev->reg_base + MBOX_IPSR);

	/* for each mailbox in device, init configuration datas */
	mbox = dev->mboxes;
	for (i = 0; i < dev->nb_mboxes; i++) {
		mbox->id = i;
		mbox->mchan.assigned = false;
		mbox->dev = dev;
		mbox->mdesc.rx_offset = i * dev->buf_size;
		mbox->mdesc.tx_offset = (dev->nb_mboxes * dev->buf_size) +
				       i * dev->buf_size;
		mbox->mdesc.rx_size = dev->buf_size;
		mbox->mdesc.tx_size = mbox->mdesc.rx_size;
		mbox++;
	}

	dev->init_done = true;
	DMSG("done\n");
	return 0;
}

/**
 * @brief - Resume mailbox driver
 * - Re-init request & status register
 * - Unmask interrupt for each assigned channel
 * @return None
 */
void mbox_resume(void)
{
	struct mbox_config *mbox;
	struct mbox_device *dev = &mbox_dev;
	uint32_t i, val;

	/* Clean all mailboxes */
	write32(0, dev->reg_base + MBOX_IMR);
	write32(0, dev->reg_base + MBOX_IPSR);

	/* Unmask interrupt for each assigned channel */
	mbox = dev->mboxes;
	for (i = 0; i < dev->nb_mboxes; i++) {
		if (mbox->mchan.assigned) {
			val = read32(dev->reg_base + MBOX_IMR);
			write32(val | BIT(mbox->id), dev->reg_base + MBOX_IMR);
		}
		mbox++;
	}
	DMSG("%s: done\n", __func__);
}

/**
 * @brief - Allocate a mailbox channel
 *      The channel is exclusively allocated and can't be used by
 *      another user before the owner calls mbox_free_channel
 * @param Mboxreq Pointer on Channel request parameters :
 *      - chan_name: Mailbox channel name requested
 *      - rxcb: User Rx callback to be called on Rx interrupt
 *      - user_data: user specific data used as parameter in rxcb
 * @return a handle to the allocated Mailbox channel or MBOX_ERROR(-1)
 */
int mbox_request_channel(struct mbox_chan_req *mbox_req)
{
	struct mbox_config *mbox;
	struct mbox_device *dev = &mbox_dev;
	bool found = false;
	uint32_t i, val;

	/* look-up for a matching mailbox */
	mbox = dev->mboxes;
	for (i = 0; i < dev->nb_mboxes; i++) {
		if (strcmp(mbox_req->chan_name, mbox->mdesc.name) == 0) {
			found = true;
			break;
		}
		mbox++;
	}

	if (!found) {
		EMSG("%s not found\n", mbox_req->chan_name);
		return MBOX_ERROR;
	}

	if (mbox->mchan.assigned) {
		EMSG("%s already assigned\n",  mbox->mdesc.name);
		return MBOX_ERROR;
	}

	mbox->mchan.assigned = true;
	mbox->mchan.rx_stop = 0;
	mbox->mchan.rx_cb = mbox_req->rx_cb;
	mbox->mchan.user_data = mbox_req->user_data;

	/* Enable interrupt for the selected mailbox channel */
	val = read32(dev->reg_base + MBOX_IMR);
	write32(val | BIT(mbox->id), dev->reg_base + MBOX_IMR);
	DMSG("channel %u has been allocated\n", mbox->id);
	return mbox->id;
}

/**
 * @brief - Release requested Mailbox channel
 * @param chan_id Mailbox channel id to be released
 * @return None
 */
void mbox_free_channel(int chan_id)
{
	struct mbox_device *dev = &mbox_dev;
	struct mbox_config *mbox;
	uint32_t val;

	if (chan_id < 0 || chan_id >= MBOX_MAX_NUMBER)
		return;

	mbox = &dev->mboxes[chan_id];

	if (!mbox->mchan.assigned)
		return;

	/* Free mailbox channel */
	mbox->mchan.assigned = false;
	mbox->mchan.rx_stop = 0;

	/* Disable interrupt for the selected mailbox channel */
	val = read32(dev->reg_base + MBOX_IMR);
	write32(val & ~BIT(mbox->id), dev->reg_base + MBOX_IMR);
}

/**
 * @brief - Send a message on the requested mailbox channel.
 * Return directly in case of error or mailbox channel busy.
 * By convention the data writing format is the following:
 *    - 1st byte     : Message Length (Max size = 255 bytes)
 *    - From 2nd byte: Data payload
 * @param ChandID Channel id on which user wants to send a message
 * @param msg Pointer on Message to be transmitted
 * @return MBOX_SUCCESS if message successfuly sent
 *      MBOX_BUSY if previous message sending is still ongoing
 *      MBOX_ERROR if message not sent due to error
 */
int mbox_send_msg(unsigned int chan_id, struct mbox_msg *msg)
{
	struct mbox_device *dev = &mbox_dev;
	struct mbox_config *mbox;
	uint32_t i, len;
	uint8_t *mbuf;

	if (chan_id < 0 || chan_id >= MBOX_MAX_NUMBER)
		return MBOX_ERROR;

	mbox = &dev->mboxes[chan_id];

	mbuf = (uint8_t *)(mbox->dev->mem_base + mbox->mdesc.tx_offset);

	if (!msg || !mbox->mchan.assigned)
		return MBOX_ERROR;

	/* TO DO: check previous message has already been handled by AP */
	/*
	 * if (mbox_is_ready(mbox) == false)
	 *	return MBOX_BUSY;
	 */

	/* Write message into channel buffer */
	len =
	    (msg->dsize <
	     (mbox->mdesc.tx_size -
	      sizeof(*mbuf))) ? msg->dsize : (mbox->mdesc.tx_size -
					      sizeof(*mbuf));
	*mbuf++ = (uint8_t)len;
	for (i = 0; i < len; i++, mbuf++)
		*mbuf = *(msg->pdata + i);

	/* mbuf allocate in iomem (non cachable) area */
	/* dcache_clean_range((void *)mbuf, len + 1); */

	/* Generate interrupt on the selected mailbox channel */
	mbox_send_event(mbox);
	return MBOX_SUCCESS;
}

/**
 * @brief - Get a message on the requested mailbox channel.
 * Return directly in case of error or mailbox channel empty.
 * By convention the data writing format is the following:
 *    - 1st byte     : Message Length (Max size = 255 bytes)
 *    - From 2nd byte: Data payload
 * @param ChandID Channel id on which user wants to get a message
 * @param msg Pointer on Message to be filled
 * @return MBOX_SUCCESS if message successfuly received
 *      MBOX_EMPTY if no message received
 *      MBOX_ERROR if message not sent due to error
 */
int mbox_get_msg(unsigned int chan_id, struct mbox_msg *msg)
{
	struct mbox_device *dev = &mbox_dev;
	struct mbox_config *mbox;
	uint32_t ipsr, msk, imr, i;
	uint8_t *mbuf;

	if (chan_id < 0 || chan_id >= MBOX_MAX_NUMBER)
		return MBOX_ERROR;

	msk = BIT(chan_id);
	ipsr = read32(dev->reg_base + MBOX_IPSR);
	imr = read32(dev->reg_base + MBOX_IMR);

	if (!(ipsr & msk))
		return MBOX_EMPTY;

	if (!(imr & msk)) /* unexpected interrupt */
		return MBOX_ERROR;

	/*
	 * There is a message pending for reading.
	 * Get message content and fill client struct
	 */
	mbox = &dev->mboxes[chan_id];

	if (!msg || !mbox->mchan.assigned)
		return MBOX_ERROR;

	mbuf = (uint8_t *)(mbox->dev->mem_base + mbox->mdesc.rx_offset);

	/* mbuf allocate in iomem (non cachable) area */
	/* inv_dcache_range((uintptr_t)mbuf, mbox->mdesc.rx_size); */

	msg->dsize = (uint8_t)*mbuf++;

	if (msg->dsize > mbox->mdesc.rx_size)
		return MBOX_ERROR;

	for (i = 0; i < msg->dsize; i++, mbuf++)
		*(msg->pdata + i) = *mbuf;

	/* Clear bit that match channel interrupt event */
	write32(read32(dev->reg_base + MBOX_IPSR) & ~msk,
		dev->reg_base + MBOX_IPSR);
	return MBOX_SUCCESS;
}
