/**
 * @file sta_mbox.c
 * @brief Provide all mailboxes functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>
#include <errno.h>

#include "FreeRTOS.h"
#include "task.h"
#include "utils.h"
#include "semphr.h"

#include "sta_common.h"
#include "sta_mbox.h"
#include "sta_nvic.h"
#include "trace.h"
#include "sta_hsem.h"

/* M3 MBOX Registers offsets */
#define MBOX_M3_IRSR			0x00
#define MBOX_M3_IPSR			0x04
#define MBOX_M3_IMR			0x08

/* AP MBOX Registers offsets */
#define MBOX_AP_IRSR			0x00
#define MBOX_AP_IPSR			0x04
#define MBOX_AP_IMR			0x08

#define TX_DELAY            (1 / portTICK_RATE_MS)
#define TX_TIMEOUT          ( portTickType )(1000 / TX_DELAY)

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
	uint32_t tx_offset;
	uint32_t tx_size;
	uint32_t rx_offset;
	uint32_t rx_size;
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
 * @hsem:       HW semaphore attribute data
 * @mdev:	mbox device that mbox config belongs to
 */
struct mbox_config {
	uint32_t id;
	struct mbox_input_desc mdesc;
	struct mbox_chnl mchan;
	struct mbox_msg mbuf;
	void *hsem;
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
 * @ap_reg_base:	base address of the AP mailbox registers
 * @irq:		mailbox irq line
 * @sem:		semaphore to control device access
 * @init_done:		status whether device init is done
 * @buf_size:		size of rx, tx channel buffers
 */
struct mbox_device {
	const char *name;
	const bool type;
	struct mbox_config *mboxes;
	uint32_t nb_mboxes;
	volatile uint8_t *mem_base;
	volatile t_mbox *reg_base;
	volatile t_mbox *ap_reg_base;
	const uint32_t irq;
	xSemaphoreHandle sem;
	bool init_done;
	const uint32_t buf_size;
};

/**
 * Mailbox device configuration datas
 */
static struct mbox_device hsem_mbox_dev = {
	.name = "hsem mailbox",
	.type = HSEM_HWIP,
	.reg_base = NULL,
	.ap_reg_base = NULL,
	.init_done = false,
	.buf_size = 16,
};

static struct mbox_device std_mbox_dev = {
	.name = "std mailbox",
	.type = MBOX_HWIP,
	.reg_base = (t_mbox *)M3_MBOX_BASE,
	.ap_reg_base = (t_mbox *)AP_MBOX_BASE,
	.irq = MAILBOX_IRQChannel,
	.init_done = false,
	.buf_size = 20,
};

static struct mbox_device *mbox_devs[] = {
	&hsem_mbox_dev, /* dev id = 0 */
	&std_mbox_dev,	/* dev id = 1 */
	NULL,
};

/* Mailboxes configuration data structs
 *
 *  mdesc name is the name that client must give to request
 * a mailbox channel.
 */

static struct mbox_config hsem_mboxes[HSEM_FREE_MAX_ID] = {
	{ .mdesc = { "rpmsg",}, },
	{ .mdesc = { "rvc", }, },
	{ .mdesc = { "tuner-backup", }, },
	{ .mdesc = { "remote-gpio", }, },
	{ .mdesc = { "mbox-test-00", }, },
	{ .mdesc = { "mbox-test-01", }, },
	{ .mdesc = { "mbox-test-02", }, },
	{ .mdesc = { "mbox-test-03", }, },
	{ .mdesc = { "mbox-test-04", }, },
	{ .mdesc = { "pm", }, },
	{ .mdesc = {"unused", }, },
};

static struct mbox_config std_mboxes[MBOX_MAX_NUMBER] = {
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

/**
 * @brief - Get mbox_config struct and physical mbox id given a logical id
 * @id: logical id of the mbox
 * @return pointer to struct mbox_config
 */
static struct mbox_config *get_mbox_by_id(uint32_t id)
{
	struct mbox_device *dev;
	uint32_t phys_id;
	uint32_t dev_id = id / MBOX_MAX_NUMBER;

	dev = mbox_devs[dev_id];
	phys_id = id - (dev_id * MBOX_MAX_NUMBER);
	if (dev->mboxes[phys_id].id != phys_id) {
		/* should never happen and point out an issue in
		 * Mailbox device configuration datas */
		TRACE_ERR("%s: bad logical id: %d\n", __func__, id);
		return NULL;
	}
	return &(dev->mboxes[phys_id]);
}

/**
 * @brief - Check previous message has already been handled by AP
 * @param id Mailbox id on which a message has to be sent
 * @return true or false
 */
static bool mbox_is_ready(struct mbox_config *mbox)
{
	if (mbox->dev->type == HSEM_HWIP) {
		if (hsem_int_status(HSEM_IRQ_A) & BIT(mbox->id))
			/* HSEM still busy */
			return false;
		else
			return true;
	} else {
		/*
		 * WARNING:
		 * Checking remote IPSR register is no more possible
		 * on TC3P cut 2.0 as AP Mbox peripheral is no more
		 * memory mapped in C-M3 mem space...
		 */
		if ((get_soc_id() == SOCID_STA1385) && (get_cut_rev() >= CUT_20))
			return true;

		uint32_t ipsr = mbox->dev->ap_reg_base->ipsr.reg;
		if (ipsr & BIT(mbox->id))
			/* Mailbox still busy */
			return false;
		else
			return true;
	}
}

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
 * @brief - Init mailbox driver
 *              - Set mailbox register and configure shared memory buffer
 *              - Enable mailbox interrupt
 * @return None
 */
int mbox_init(char *name)
{
	struct mbox_device **dev_p = mbox_devs;
	struct mbox_device *dev;
	struct mbox_config *mbox;
	struct nvic_chnl irq_chnl;
	bool found = false;
	uint32_t i;

	dev = *dev_p;
	while(dev != NULL) {
		if (strcmp(name, dev->name) == 0) {
			found = true;
			break;
		}
		dev_p++;
		dev = *dev_p;
	}
	if (!found || dev->init_done)
		return MBOX_ERROR;

#ifndef NO_SCHEDULER
	dev->sem = xSemaphoreCreateBinary();
	if (!dev->sem) {
		TRACE_ERR("%s: failed to create sem\n", __func__);
		return MBOX_ERROR;
	}
#endif

	switch (dev->type) {
	case MBOX_HWIP:
		/* Enable Mailbox interrupt */
		irq_chnl.id = dev->irq;
		irq_chnl.preempt_prio = IRQ_LOW_PRIO;
		irq_chnl.enabled = true;

		nvic_chnl_init(&irq_chnl);

		/* Clean all mailboxes */
		write_reg(0, (uint32_t)dev->reg_base + MBOX_M3_IMR);
		write_reg(0, (uint32_t)dev->reg_base + MBOX_M3_IPSR);

		dev->mboxes = std_mboxes;
		dev->nb_mboxes = MBOX_MAX_NUMBER;
#ifdef ATF
		dev->mem_base = MBOX_MEM_BASE_SEC;
#else
		dev->mem_base = MBOX_MEM_BASE;
#endif
		break;
	case HSEM_HWIP:
		dev->mboxes = hsem_mboxes;
		dev->nb_mboxes = HSEM_FREE_MAX_ID;
		dev->mem_base = MBOX_MEM_BASE;
		break;
	default:
		TRACE_ERR("%s: unknown type: %i\n", __func__, dev->type);
		return MBOX_ERROR;
	}

	/* for each mailbox in device, init configuration datas */
	mbox = dev->mboxes;
	for (i = 0; i < dev->nb_mboxes; i++) {
		mbox->id = i;
		mbox->mchan.assigned = false;
		mbox->dev = dev;
		mbox->mdesc.tx_offset = i * dev->buf_size;
		mbox->mdesc.rx_offset = (dev->nb_mboxes * dev->buf_size) +
				       i * dev->buf_size;
		mbox->mdesc.tx_size = mbox->mdesc.rx_size = dev->buf_size;
		mbox++;
	}

#ifndef NO_SCHEDULER
	xSemaphoreGive(dev->sem);
#endif
	dev->init_done = true;
	return 0;
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
	struct mbox_device **dev_p = mbox_devs;
	struct mbox_device *dev;
	bool found = false;
	int dev_id = 0;
	uint32_t i;

	dev = *dev_p;

	/* for each mailbox device, look-up for a matching mailbox */
	while (dev != NULL) {
		mbox = dev->mboxes;
		for (i = 0; i < dev->nb_mboxes; i++) {
			if (strcmp(mbox_req->chan_name, mbox->mdesc.name) == 0) {
				found = true;
				break;
			}
			mbox++;
		}
		if (found)
			break;
		dev_p++;
		dev = *dev_p;
		dev_id++;
	}

	if (!found) {
		TRACE_INFO("%s: %s not found\n", __func__, mbox_req->chan_name);
		return MBOX_ERROR;
	}

	if (!dev->init_done) {
		TRACE_ERR("%s: Mailbox not initialized\n", __func__);
		return MBOX_ERROR;
	}

	if(mbox->mchan.assigned == true) {
		TRACE_ERR("%s: %s already assigned\n", __func__,  mbox->mdesc.name);
		return MBOX_ERROR;
	}

#ifndef NO_SCHEDULER
	xSemaphoreTake(dev->sem, portMAX_DELAY);
#endif
	mbox->mchan.assigned = true;
	mbox->mchan.rx_stop = 0;
	mbox->mchan.rx_cb = mbox_req->rx_cb;
	mbox->mchan.user_data = mbox_req->user_data;

	if (dev->type == HSEM_HWIP) {
		struct hsem_lock_req lock_req;

		/* Set requested Mailbox channel feature */
		lock_req.id = mbox->id;
		lock_req.lock_cb = mbox_rx_msg;
		lock_req.cookie = (void *)mbox;

		mbox->hsem = hsem_lock_request(&lock_req);
	} else {
		mbox->hsem = NULL;

		/* Enable interrupt for the selected mailbox channel */
		dev->reg_base->imr.reg |= BIT(mbox->id);
	}

#ifndef NO_SCHEDULER
	xSemaphoreGive(dev->sem);
#endif
	return (dev_id * MBOX_MAX_NUMBER) + mbox->id;
}

/**
 * @brief - Release requested Mailbox channel
 * @param chan_id Mailbox channel id to be released
 * @return None
 */
void mbox_free_channel(int chan_id)
{
	struct mbox_config *mbox = get_mbox_by_id(chan_id);
	if (!mbox) {
		TRACE_ERR("%s\n", __func__);
		return;
	}
	if (mbox->mchan.assigned == false)
		return;

#ifndef NO_SCHEDULER
	xSemaphoreTake(mbox->dev->sem, portMAX_DELAY);
#endif
	/* Free mailbox channel */
	mbox->mchan.assigned = false;
	mbox->mchan.rx_stop = 0;

	/* Disable interrupt for the selected mailbox channel */
	if (mbox->dev->type == HSEM_HWIP)
		hsem_lock_free(mbox->hsem);
	else
		mbox->dev->reg_base->imr.reg &= ~BIT(mbox->id);

#ifndef NO_SCHEDULER
	xSemaphoreGive(mbox->dev->sem);
#endif
	TRACE_INFO("mbox_free_channel: Channel %s [id:%d] released\n",
		   mbox->mdesc.name, chan_id);
}

/**
 * @brief - Stop message reception for requested Mailbox channel
 * by disabling the interrupt.
 * This function can be used in an interrupt service routine but
 * the function mbox_resume_rx_msg to resume the message reception
 * MUST be call in OS process context.
 * The service mbox_stop_rx_msg / mbox_resume_rx_msg can be used
 * for data flow managing purpose.
 * @param ChandID Mailbox channel id to be stopped
 * @return None
 */
void mbox_stop_rx_msg(int chan_id)
{
	struct mbox_config *mbox = get_mbox_by_id(chan_id);
	if (!mbox) {
		TRACE_ERR("%s\n", __func__);
		return;
	}

	taskENTER_CRITICAL();

	/* Disable interrupt for the selected mailbox channel */
	if (mbox->dev->type == HSEM_HWIP)
		hsem_disable_irq(mbox->hsem);
	else
		mbox->dev->reg_base->imr.reg &= ~BIT(mbox->id);

	mbox->mchan.rx_stop++;

	taskEXIT_CRITICAL();

	TRACE_INFO("mbox_stop_rx_msg: Channel %s [id:%d] stopped\n",
		   mbox->mdesc.name, chan_id);
}

/**
 * @brief - Resume message reception for requested Mailbox channel
 * by re-enabling the interrupt.
 * API function to be used to in OS process context.
 * This function must NOT be used in an interrupt service routine.
 * The service mbox_stop_rx_msg / mbox_resume_rx_msg can be used
 * for data flow managing purpose.
 * @param chan_id Mailbox channel id to be stopped
 * @return None
 */
void mbox_resume_rx_msg(int chan_id)
{
	struct mbox_config *mbox = get_mbox_by_id(chan_id);
	if (!mbox)
		return;

	if (!mbox->mchan.rx_stop)
		return;

	TRACE_INFO("mbox_resume_rx_msg: Channel %s [id:%d] resumed\n",
		   mbox->mdesc.name, chan_id);

#ifndef NO_SCHEDULER
	xSemaphoreTake(mbox->dev->sem, portMAX_DELAY);
#endif

	/* Enable interrupt for the selected mailbox channel */
	mbox->mchan.rx_stop--;
	if (mbox->dev->type == HSEM_HWIP)
		hsem_enable_irq(mbox->hsem);
	else
		mbox->dev->reg_base->imr.reg |= BIT(mbox->id);

#ifndef NO_SCHEDULER
	xSemaphoreGive(mbox->dev->sem);
#endif
}

/**
 * @brief - Send a message on the requested mailbox channel.
 * Return directly in case of error or mailbox channel busy
 * This function can be used in an interrupt service routine.
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
	uint32_t i, len;
	struct mbox_config *mbox = get_mbox_by_id(chan_id);
	if (!mbox)
		return MBOX_ERROR;

	uint8_t *mbuf = (uint8_t *) (mbox->dev->mem_base + mbox->mdesc.tx_offset);

	if (msg == NULL || mbox->mchan.assigned == false)
		return MBOX_ERROR;

	/* Check previous message has already been handled by AP */
	if (mbox_is_ready(mbox) == false)
		return MBOX_BUSY;

#ifndef NO_SCHEDULER
	if (mbox->dev->type == HSEM_HWIP) {
		if (portNVIC_INT_ACTIVE()) {
			TRACE_ASSERT("mbox_send_msg called in IRQ with type = HSEM_HWIP\n");
			for(;;);
		}
		xSemaphoreTake(mbox->dev->sem, portMAX_DELAY);
	} else {
		portDISABLE_INTERRUPTS();
	}
#endif
	/* Write message into channel buffer */
	len =
	    (msg->dsize <
	     (mbox->mdesc.tx_size -
	      sizeof(*mbuf))) ? msg->dsize : (mbox->mdesc.tx_size -
					      sizeof(*mbuf));
	*mbuf++ = (uint8_t) len;
	for (i = 0; i < len; i++, mbuf++) {
		*mbuf = *(msg->pdata + i);
	}

	/**
	 * Trigger an interrupt to make remote processor being aware that
	 * there is a new message to be read on the selected mailbox channel.
	 */
	if (mbox->dev->type == HSEM_HWIP) {
		while (hsem_try_lock(mbox->hsem) == HSEM_BUSY);
		hsem_unlock(mbox->hsem);
	} else {
		mbox->dev->reg_base->irsr.reg |= BIT(mbox->id);
	}

#ifndef NO_SCHEDULER
	if (mbox->dev->type == HSEM_HWIP)
		xSemaphoreGive(mbox->dev->sem);
	else
		portENABLE_INTERRUPTS();
#endif
	return MBOX_SUCCESS;
}

/**
 * @brief - Send a Message over the specified Mailbox channel with timeout
 * waiting handling if Channel is busy.
 * API function to be used to in OS process context.
 * This function must NOT be used in an interrupt service routine.
 * See mbox_send_msg for an alternative that can.
 * @param chan_id Channel id on which user wants to send a message
 * @param msg Pointer on Message to be transmitted
 * @return ERROR / SUCCESS
 */
int mbox_os_wait_send_msg(int chan_id, struct mbox_msg *msg)
{
	int tx_status;
	portTickType last_wake_time;
	const portTickType tx_delay = TX_DELAY;
	int tx_count = TX_TIMEOUT;

	/* Send message over specified Mailbox channel */
	tx_status = mbox_send_msg(chan_id, msg);
	last_wake_time = xTaskGetTickCount();

	while (tx_status == MBOX_BUSY) {
		/* Wait for the next cycle. */
		vTaskDelayUntil(&last_wake_time, tx_delay);

		/* try again sending message */
		tx_status = mbox_send_msg(chan_id, msg);
		if (!(--tx_count)) {
			tx_status = MBOX_ERROR;
		}
	}

	if (tx_status == MBOX_ERROR) {
		TRACE_ERR
		    ("mbox_os_wait_send_msg: Tx over Mailbox channel id:%d failed\n",
		     chan_id);
		return -EFAULT;
	} else {
		return 0;
	}
}

/**
 * @brief - Mailbox ISR
 * @return none
 */
void mailbox_irq_handler(void)
{
	struct mbox_device *dev = &std_mbox_dev;
	uint32_t ipsr, msk, n, imr;

	/* Get pending mailbox interrupts */
	ipsr = dev->reg_base->ipsr.reg;

	/* Handle pending interrupt */
	for (n = 0; n < MBOX_MAX_NUMBER; n++) {
		msk = BIT(n);
		if (ipsr & msk) {

			imr = dev->reg_base->imr.reg;
			if (imr & msk)
				/* Handle received message */
				mbox_rx_msg((void *)&dev->mboxes[n]);
			else
				TRACE_ERR
				    ("mailbox_irq_handler: Unexpected mailbox interrupt [id:%d]\n",
				     n);
		}
	}
	mbox_m3_regs->ipsr.reg &= ~ipsr;
}
