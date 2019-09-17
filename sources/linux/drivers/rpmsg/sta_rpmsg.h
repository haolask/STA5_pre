/*
 * STA micro-controller SubSystem (uCSS) remote processor messaging client
 *
 * Copyright (C) 2014 STMicroelectronics.
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

#ifndef STA_RPMSG_H
#define STA_RPMSG_H

#include <linux/ioctl.h>

#define CSS_IOC_MAGIC	'X'

#define CSS_IOCCONTROL		_IOW(CSS_IOC_MAGIC, 1, char *)
#define CSS_IOCCANLISTEN	_IOW(CSS_IOC_MAGIC, 2, char *)
#define CSS_IOCACCLISTEN	_IOW(CSS_IOC_MAGIC, 3, char *)

#define CSS_IOC_MAXNR	(3)

#ifdef __KERNEL__

/**
 * enum css_msg_types - various message types currently supported
 *
 * @CSS_CTRL_REQ: a control request message type.
 *
 * @CSS_CTRL_RSP: a response to a control request.
 *
 * @CSS_RAW_MSG: a message that should be propagated as-is to the user.
 *
 * @CSS_CAN_LISTEN: user request to listen to incoming can0 frames
 *
 * @CSS_ACC_LISTEN: user request to listen to incoming accelerometer data
 *
 * @CSS_CAN_RX: CAN data from remote processor
 *
 * @CSS_ACC_RX: accelerometer data from remote processor
 */
enum css_msg_types {
	CSS_CTRL_REQ = 0,
	CSS_CTRL_RSP = 1,
	CSS_RAW_MSG = 2,
	CSS_CAN_LISTEN = 3,
	CSS_ACC_LISTEN = 4,
	CSS_CAN_RX = 5,
	CSS_ACC_RX = 6,
};

/**
 * enum css_ctrl_types - various control codes that will be used
 *
 * @CSS_CONNECTION: connection request
 *
 * @CSS_DISCONNECT: connction disconnection
 *
 * @CSS_ERROR: remote processor error
 */
enum css_ctrl_types {
	CSS_CONNECTION = 0,
	CSS_DISCONNECT = 1,
	CSS_SUCCESS = 2,
	CSS_ERROR = 3,
};

/* keep documenting... */
enum css_state {
	CSS_UNCONNECTED,
	CSS_CONNECTED,
	CSS_FAILED,
};

/**
 * struct css_msg_hdr - common header for all CSS messages
 * @type:	type of message, see enum css_msg_types
 * @len:	length of msg payload (in bytes)
 * @data:	the msg payload (depends on the message type)
 *
 * All CSS messages will start with this common header (which will begin
 * right after the standard rpmsg header ends).
 */
struct css_msg_hdr {
	u32 type;
	u32 len;
	char data[0];
} __packed;

struct css_ctrl_rsp {
	u32 status;
	u32 addr;
} __packed;

#endif /* __KERNEL__ */

/* Exposed to user space too */
struct css_ctrl_req {
	u32 request;
	u32 addr;
} __packed;

#endif /* STA_RPMSG_H */
