/*
 * mailbox-sta.h
 *
 * STMicroelectronics STA mailbox driver public header
 *
 * Copyright (C) 2013 STMicroelectronics
 * Author:  <olivier.lebreton@st.com> for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#include <linux/ioport.h>

/**
 * struct sta_mbox_msg - STA mailbox message description
 * @dsize:		data payload size
 * @pdata:		message data payload
 */
struct sta_mbox_msg {
	u8		dsize;
	u8		*pdata;
};

struct resource *sta_mbox_reg_mem(void);
struct resource *sta_mbox_rp_reg_mem(void);
