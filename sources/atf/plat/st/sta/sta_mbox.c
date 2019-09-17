/*
 *
 * (C) Copyright 2016/2017 ST-microlectronics ADG
 *
 */
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <stdbool.h>
#include <string.h>
#include <utils.h>
#include <xlat_tables_v2.h>
#include <sta_private.h>

#include <mbox.h>

#define MBOX_MAX_NUMBER 16

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
	void (*rx_cb) (void *user_data, struct mbox_msg *msg);
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
	uint32_t m3_reg_base;
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
	.mem_base = (volatile uint8_t *)IPC_BASE,
	.reg_base = STA_MBOX_BASE,
	.m3_reg_base = STA_MBOX_M3_BASE,
	.irq = 0, /* TO BE FILLED */
	.init_done = false,
	.buf_size = 20,
};

mmap_reserve(MAP_MBOX);
mmap_reserve(MAP_MBOX_M3);

/**
 * @brief - Generate event to trigger IRQ on remote processor
 * @param mbox Mailbox pointer on which a message bas to be sent
 * @return none
 */
static inline void mbox_send_event(struct mbox_config *mbox)
{
	uint32_t val = mmio_read_32(mbox->dev->reg_base + MBOX_IRSR);
	mmio_write_32(mbox->dev->reg_base + MBOX_IRSR, val | BIT(mbox->id));
}

/**
 * @brief - Check previous message has already been handled by AP
 * @param id Mailbox id on which a message has to be sent
 * @return true or false
 */
static bool mbox_is_ready(struct mbox_config *mbox)
{
	uint32_t ipsr;

	assert(IN_MMAP(mbox->dev->m3_reg_base, MAP_MBOX_M3));
	ipsr = mmio_read_32(mbox->dev->m3_reg_base + MBOX_IPSR);

	if (ipsr & BIT(mbox->id))
		/* Mailbox still busy */
		return false;
	else
		return true;
}

#if 0
/**
 * @brief - Mailbox received message handling
 * @param id Mailbox id on which a message bas been received
 * @return none
 */
static void mbox_rx_msg(void * cookie)
{
	struct mbox_config *mbox = (struct mbox_config *)cookie;
	volatile uint8_t *mbuf = mbox->dev->mem_base + mbox->mdesc.rx_offset;

	/* Read data in mailbox buffer */
	mbox->mbuf.dsize = *mbuf++;
	mbox->mbuf.pdata = mbuf;

	/* Call the Rx callback */
	if (mbox->mchan.rx_cb)
		mbox->mchan.rx_cb(mbox->mchan.user_data, (void *)&mbox->mbuf);
}

/**
 * @brief - Mailbox ISR
 * @return none
 */
void mailbox_irq_handler(void)
{
	struct mbox_device *dev = &mbox_dev;
	uint32_t ipsr, msk, n, imr;

	/* Get pending mailbox interrupts */
	ipsr = mmio_read_32(dev->reg_base + MBOX_IPSR);

	/* Handle pending interrupt */
	for (n = 0; n < MBOX_MAX_NUMBER; n++) {
		msk = BIT(n);
		if (ipsr & msk) {

			imr = mmio_read_32(dev->reg_base + MBOX_IMR);
			if (imr & msk)
				/* Handle received message */
				mbox_rx_msg((void *)&dev->mboxes[n]);
			else
				ERROR
				    ("mailbox_irq_handler: Unexpected mailbox interrupt [id:%d]\n",
				     n);
		}
	}
	mmio_write_32(dev->reg_base + MBOX_IPSR,
		      mmio_read_32(dev->reg_base + MBOX_IPSR) & ~ipsr);
}
#endif

/**
 * @brief - Init mailbox driver
 * - Set mailbox register and configure shared memory buffer
 * - Enable mailbox interrupt
 * @return None
 */
int mbox_init(void)
{
	struct mbox_device *dev = &mbox_dev;
	struct mbox_config *mbox;
	uint32_t i;

	if (dev->init_done)
		return 0;

	/* TO DO: Enable Mailbox interrupt */

	/* Clean all mailboxes */
	assert(IN_MMAP(dev->reg_base, MAP_MBOX));

	mmio_write_32(dev->reg_base + MBOX_IMR, 0);
	mmio_write_32(dev->reg_base + MBOX_IPSR, 0);

	/* for each mailbox in device, init configuration datas */
	mbox = dev->mboxes;
	for (i = 0; i < dev->nb_mboxes; i++) {
		mbox->id = i;
		mbox->mchan.assigned = false;
		mbox->dev = dev;
		mbox->mdesc.rx_offset = i * dev->buf_size;
		mbox->mdesc.tx_offset = (dev->nb_mboxes * dev->buf_size) +
				       i * dev->buf_size;
		mbox->mdesc.tx_size = mbox->mdesc.rx_size = dev->buf_size;
		mbox++;
	}

	dev->init_done = true;
	INFO("%s done\n", __func__);
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
	mmio_write_32(dev->reg_base + MBOX_IMR, 0);
	mmio_write_32(dev->reg_base + MBOX_IPSR, 0);

	/* Unmask interrupt for each assigned channel */
	mbox = dev->mboxes;
	for (i = 0; i < dev->nb_mboxes; i++) {
		if (mbox->mchan.assigned) {
			val = mmio_read_32(dev->reg_base + MBOX_IMR);
			mmio_write_32(dev->reg_base + MBOX_IMR,
				      val | BIT(mbox->id));
		}
		mbox++;
	}
	INFO("%s: done\n", __func__);
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
		ERROR("%s: %s not found\n", __func__, mbox_req->chan_name);
		return MBOX_ERROR;
	}

	if (mbox->mchan.assigned) {
		ERROR("%s: %s already assigned\n", __func__,  mbox->mdesc.name);
		return MBOX_ERROR;
	}

	mbox->mchan.assigned = true;
	mbox->mchan.rx_stop = 0;
	mbox->mchan.rx_cb = mbox_req->rx_cb;
	mbox->mchan.user_data = mbox_req->user_data;

	/* Enable interrupt for the selected mailbox channel */
	val = mmio_read_32(dev->reg_base + MBOX_IMR);
	mmio_write_32(dev->reg_base + MBOX_IMR, val | BIT(mbox->id));
	INFO("%s: channel %u has been allocated\n", __func__, mbox->id);
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

	if (mbox->mchan.assigned == false)
		return;

	/* Free mailbox channel */
	mbox->mchan.assigned = false;
	mbox->mchan.rx_stop = 0;

	/* Disable interrupt for the selected mailbox channel */
	val = mmio_read_32(dev->reg_base + MBOX_IMR);
	mmio_write_32(dev->reg_base + MBOX_IMR, val & ~BIT(mbox->id));
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
	uint8_t *mbuf, *tmp;

	if (chan_id < 0 || chan_id >= MBOX_MAX_NUMBER)
		return MBOX_ERROR;

	mbox = &dev->mboxes[chan_id];

	mbuf = (uint8_t *) (mbox->dev->mem_base + mbox->mdesc.tx_offset);
	tmp = mbuf;

	if (msg == NULL || mbox->mchan.assigned == false)
		return MBOX_ERROR;

	/* Check previous message has already been handled by AP */
	if (mbox_is_ready(mbox) == false)
		return MBOX_BUSY;

	/* Write message into channel buffer */
	len =
	    (msg->dsize <
	     (mbox->mdesc.tx_size -
	      sizeof(*tmp))) ? msg->dsize : (mbox->mdesc.tx_size -
					      sizeof(*tmp));
	*tmp++ = (uint8_t)len;
	for (i = 0; i < len; i++, tmp++) {
		*tmp = *(msg->pdata + i);
	}
	flush_dcache_range((uintptr_t)mbuf, len + 1);

	/* Generate interrupt on the selected mailbox channel */
	mbox_send_event(mbox);
	INFO("%s: channel id %u\n", __func__, chan_id);
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
	uint8_t *mbuf, *tmp;

	if (chan_id < 0 || chan_id >= MBOX_MAX_NUMBER)
		return MBOX_ERROR;

	msk = BIT(chan_id);
	ipsr = mmio_read_32(dev->reg_base + MBOX_IPSR);
	imr = mmio_read_32(dev->reg_base + MBOX_IMR);

	if (!(ipsr & msk))
		return MBOX_EMPTY;

	if (!(imr & msk)) /* unexpected interrupt */
		return MBOX_ERROR;

	/* There is a message pending for reading.
	 * Get message content and fill client struct */

	mbox = &dev->mboxes[chan_id];

	if (msg == NULL || mbox->mchan.assigned == false)
		return MBOX_ERROR;

	mbuf = (uint8_t *) (mbox->dev->mem_base + mbox->mdesc.rx_offset);
	tmp = mbuf;

	inv_dcache_range((uintptr_t)mbuf, mbox->mdesc.rx_size);

	msg->dsize = (uint8_t)*tmp++;

	if (msg->dsize > mbox->mdesc.rx_size)
		return MBOX_ERROR;

	for (i = 0; i < msg->dsize; i++, tmp++)
		*(msg->pdata + i) = *tmp;

	/* Clear bit that match channel interrupt event */
	mmio_write_32(dev->reg_base + MBOX_IPSR,
		      mmio_read_32(dev->reg_base + MBOX_IPSR) & ~msk);
	return MBOX_SUCCESS;
}
