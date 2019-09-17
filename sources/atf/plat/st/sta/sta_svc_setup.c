/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdarg.h>
#include <debug.h>
#include <mbox.h>
#include <mmio.h>
#include <runtime_svc.h>
#include <smccc.h>
#include <spinlock.h>
#include <sta_smc.h>
#include <sta_private.h>
#include <xlat_tables_v2.h>

spinlock_t sta_svc_lock;

/*
 * In case REMOTEPROC_CTRL is set, then read, write and update register requests
 * are forwarded to the remote processor, relying on "regmap" mailbox channel.
 * In such a case, sanity check as well as execution of requests are delegated
 * to the remote processor. So the secure monitor behaves like a serving hatch.
 */
#if REMOTEPROC_CTRL

/* mailbox channel id used to send remote requests to C-M3 Firmware */
static uint32_t msg_chan_id;

/*
 * Send a request to the remote micro-controller sub-system
 * through hardware mailbox device.
 */
static inline void sta_svc_send_msg(struct regmap_request *req)
{
	struct mbox_msg msg;
	uint32_t timeout = 100000;

	msg.dsize = sizeof(struct regmap_request);
	msg.pdata = (uint8_t *)req;

	while (mbox_send_msg(msg_chan_id, &msg) != MBOX_SUCCESS && timeout)
		timeout--;

	if (!timeout)
		panic();
}

/**
 * Get a response from the remote micro-controller sub-system
 * through hardware mailbox device.
 */
static inline void sta_svc_get_msg(struct regmap_request *req)
{
	struct mbox_msg msg;
	uint32_t timeout = 100000;

	msg.pdata = (uint8_t *)req;

	while (mbox_get_msg(msg_chan_id, &msg) != MBOX_SUCCESS && timeout)
		timeout--;

	if (!timeout)
		panic();
}

static uint32_t sta_svc_smc_read(uint32_t val_bits, uint32_t reg, uint32_t *val)
{
	struct regmap_request req;
	int ret;

	req.val_bits = val_bits;
	req.type = STA_REGMAP_SMC_READ;
	req.status = STA_SMC_OK;
	req.reg = reg;

	sta_svc_send_msg(&req);
	sta_svc_get_msg(&req);

	ret = (int)req.status;
	if (ret != STA_SMC_OK)
		return (uint32_t)ret;

	if (req.reg != reg || req.type != STA_REGMAP_SMC_READ)
		panic();

	*val = req.val;

	return STA_SMC_OK;
}

static uint32_t sta_svc_smc_write(uint32_t val_bits, uint32_t reg, uint32_t val)
{
	struct regmap_request req;
	int ret;

	req.val_bits = val_bits;
	req.type = STA_REGMAP_SMC_WRITE;
	req.status = STA_SMC_OK;
	req.reg = reg;
	req.val = val;

	sta_svc_send_msg(&req);
	sta_svc_get_msg(&req);

	ret = (int)req.status;
	if (ret != STA_SMC_OK)
		return (uint32_t)ret;

	if (req.reg != reg || req.type != STA_REGMAP_SMC_WRITE)
		panic();

	return STA_SMC_OK;
}

static uint32_t sta_svc_smc_update_bits(uint32_t val_bits, uint32_t reg,
					uint32_t mask, uint32_t val)
{
	struct regmap_request req;
	int ret;

	req.val_bits = val_bits;
	req.type = STA_REGMAP_SMC_UPDATE_BITS;
	req.status = STA_SMC_OK;
	req.reg = reg;
	req.mask = mask;
	req.val = val;

	sta_svc_send_msg(&req);
	sta_svc_get_msg(&req);

	ret = (int)req.status;
	if (ret != STA_SMC_OK)
		return (uint32_t)ret;

	if (req.reg != reg || req.type != STA_REGMAP_SMC_UPDATE_BITS)
		panic();

	return STA_SMC_OK;
}

static int32_t sta_svc_setup(void)
{
	int ret;
	struct mbox_chan_req req;

	ret = mbox_init();
	if (ret)
		return ret;

	req.chan_name = "regmap";
	req.user_data = NULL;
	req.rx_cb = NULL;

	msg_chan_id = mbox_request_channel(&req);
	if (msg_chan_id < 0)
		return msg_chan_id;

	return 0;
}

#else /* REMOTEPROC_CTRL */

mmap_reserve(MAP_DEVICE1);

static uint32_t sta_svc_smc_read(uint32_t val_bits, uint32_t reg, uint32_t *val)
{
	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		*val = mmio_read_8((uintptr_t)reg);
		break;
	case STA_REGMAP_SMC_VALBITS_16:
		*val = mmio_read_16((uintptr_t)reg);
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		*val = mmio_read_32((uintptr_t)reg);
		break;
	case STA_REGMAP_SMC_VALBITS_64: /* FIXME: really handle 64 bits case */
		return STA_SMC_NOT_SUPPORTED;
	default:
		return STA_SMC_INVALID_PARAMS;
	}

	return STA_SMC_OK;
}

static uint32_t sta_svc_smc_write(uint32_t val_bits, uint32_t reg, uint32_t val)
{
	/* FIXME: Do a sanity check of the request, possibly by applying a mask
	 * on the value to be written.
	 */

	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		mmio_write_8((uintptr_t)reg, (uint8_t)val);
		break;
	case STA_REGMAP_SMC_VALBITS_16:
		mmio_write_16((uintptr_t)reg, (uint16_t)val);
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		mmio_write_32((uintptr_t)reg, (uint32_t)val);
		break;
	case STA_REGMAP_SMC_VALBITS_64: /* FIXME: really handle 64 bits case */
		return STA_SMC_NOT_SUPPORTED;
	default:
		return STA_SMC_INVALID_PARAMS;
	}

	return STA_SMC_OK;
}

static uint32_t sta_svc_smc_update_bits(uint32_t val_bits, uint32_t reg,
					uint32_t mask, uint32_t val)
{
	uint32_t tmp;

	/* FIXME: Do a sanity check of the request, possibly by applying a mask
	 * on the requested bits to be updated.
	 */

	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		tmp = mmio_read_8((uintptr_t)reg);
		tmp &= ~mask;
		mmio_write_8((uintptr_t)reg, (uint8_t)(tmp | val));
		break;
	case STA_REGMAP_SMC_VALBITS_16:
		tmp = mmio_read_16((uintptr_t)reg);
		tmp &= ~mask;
		mmio_write_16((uintptr_t)reg, (uint16_t)(tmp | val));
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		tmp = mmio_read_32((uintptr_t)reg);
		tmp &= ~mask;
		mmio_write_32((uintptr_t)reg, (uint32_t)(tmp | val));
		break;
	case STA_REGMAP_SMC_VALBITS_64: /* FIXME: really handle 64 bits case */
		return STA_SMC_NOT_SUPPORTED;
	default:
		return STA_SMC_INVALID_PARAMS;
	}

	return STA_SMC_OK;
}

static int32_t sta_svc_setup(void)
{
	return 0;
}

#endif /* REMOTEPROC_CTRL */

/*
 * Top-level Standard Service SMC handler. This handler will in turn dispatch
 * calls to PSCI SMC handler
 */
static uintptr_t sta_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
				   u_register_t x2, u_register_t x3,
				   u_register_t x4, void *cookie, void *handle,
				   u_register_t flags)
{
	uint32_t return_value = 0;
	uint32_t return_error = 0;
	uint32_t val_bits = x1;
	uint8_t type = smc_fid & STA_REGMAP_SMC_TYPE_MSK;

	if (val_bits != STA_REGMAP_SMC_VALBITS_8 &&
	    val_bits != STA_REGMAP_SMC_VALBITS_16 &&
	    val_bits != STA_REGMAP_SMC_VALBITS_32 &&
	    val_bits != STA_REGMAP_SMC_VALBITS_64) {
		if (type == STA_REGMAP_SMC_READ) {
			SMC_RET2(handle, (uint32_t)STA_SMC_INVALID_PARAMS, 0);
		} else {
			SMC_RET1(handle, (uint32_t)STA_SMC_INVALID_PARAMS);
		}
	}

	spin_lock(&sta_svc_lock);
	switch (type) {
	case STA_REGMAP_SMC_READ:
		return_error = sta_svc_smc_read(x1, x2, &return_value);
		spin_unlock(&sta_svc_lock);
		SMC_RET2(handle, return_error, return_value);

	case STA_REGMAP_SMC_WRITE:
		return_error = sta_svc_smc_write(x1, x2, x3);
		spin_unlock(&sta_svc_lock);
		SMC_RET1(handle, return_error);

	case STA_REGMAP_SMC_UPDATE_BITS:
		return_error = sta_svc_smc_update_bits(x1, x2, x3, x4);
		spin_unlock(&sta_svc_lock);
		SMC_RET1(handle, return_error);

	default:
		WARN("Unknown STA Service Call: 0x%x\n", smc_fid);
		spin_unlock(&sta_svc_lock);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		std_svc,
		OEN_SIP_START,
		OEN_SIP_END,
		SMC_TYPE_FAST,
		sta_svc_setup,
		sta_svc_smc_handler
);

