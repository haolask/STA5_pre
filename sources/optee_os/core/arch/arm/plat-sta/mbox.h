/*
 * (C) Copyright 2017 ST-microlectronics ADG
 *
 */

#ifndef __ASM_ARCH_MBOX_H
#define __ASM_ARCH_MBOX_H

#define MBOX_SUCCESS        1
#define MBOX_BUSY           0
#define MBOX_ERROR          -1
#define MBOX_EMPTY          -2

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
	void (*rx_cb)(void *user_data, struct mbox_msg *msg);
};

/**
 * @brief - Init mailbox device by name
 * @name: name of mailbox device to initialize
 * @return 0 if no error, or MBOX_ERROR.
 */
int mbox_init(void);

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
int mbox_get_msg(unsigned int chan_id, struct mbox_msg *msg);

/**
 * @brief - Resume mailbox driver
 * - Re-init request & status register
 * - Unmask interrupt for each assigned channel
 * @return None
 */
void mbox_resume(void);

#endif /* __ASM_ARCH_MBOX_H */
