/**
 * @file sta_mbox.h
 * @brief Provide all mailboxes definitions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _STA_MBOX_H_
#define _STA_MBOX_H_

#include "sta_lib.h"

#define MBOX_SUCCESS        1
#define MBOX_BUSY           0
#define MBOX_ERROR          -1

#define MBOX_MAX_NUMBER     16

/* MBOX_HWIP:  Mailbox HW IP */
#define MBOX_HWIP 0
/* @HSEM_HWIP:	HSEM HW IP */
#define HSEM_HWIP 1

/**
 * struct mbox_msg - Mailbox message description
 * @dsize:		data payload size
 * @pdata:		message data payload
 */
struct mbox_msg {
	uint32_t dsize;
	volatile uint8_t *pdata;
};


/**
 * struct mbox_chan_req - Mailbox channel request by user
 * @chan_name: channel name requested by user this client wants
 * @user_data: user specific data, mbox driver doesn't touch it
 * @rx_cb: atomic callback to provide user the data received
 */
struct mbox_chan_req {
	char *chan_name;
	void *user_data;
	void (*rx_cb) (void *user_data, struct mbox_msg *msg);
};

/**
 * @brief - Init mailbox device by name
 * @name: name of mailbox device to initialize
 * @return 0 if no error, or MBOX_ERROR.
 */
int mbox_init(char *name);

/**
 * @brief - Allocate a mailbox channel
 *      The channel is exclusively allocated and can't be used by
 *      another user before the owner calls mbox_free_channel
 * @param Mboxreq Pointer on Channel request parameters :
 *      - chan_name: Mailbox channel name requested
 *      - rxcb: User Rx callback to be called on Rx interrupt
 *      - user_data: user specific data used as parameter in rxcb
 * @return Mailbox channel index or MBOX_ERROR(-1)
 */
int mbox_request_channel(struct mbox_chan_req *mbox_req);

/**
 * @brief - Release requested Mailbox channel
 * @param chan_id Mailbox channel id to be released
 * @return None
 */
void mbox_free_channel(int chan_id);

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
void mbox_stop_rx_msg(int chan_id);

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
void mbox_resume_rx_msg(int chan_id);

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
int mbox_send_msg(unsigned int chan_id, struct mbox_msg *msg);

/**
 * @brief - Send a Message over the specified Mailbox channel with timeout
 * waiting handling if Channel is busy.
 * API function to be used to in OS process context.
 * This function must NOT be used in an interrupt service routine.
 * See mbox_send_msg for an alternative that can.
 * @param chan_id Channel id on which user wants to send a message
 * @param msg Pointer on Message to be transmitted
 * @return 0 if no error, not 0 otherwise
 */
int mbox_os_wait_send_msg(int chan_id, struct mbox_msg *msg);

/**
 * @brief - Mailbox ISR
 * @return none
 */
void mailbox_irq_handler(void);

#endif /* _STA_MBOX_H_ */
