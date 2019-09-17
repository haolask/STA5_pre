/*
 * Copyright (c) 2017-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arm.h>
#include <console.h>
#include <kernel/cache_helpers.h>
#include <kernel/generic_boot.h>
#include <kernel/thread.h>
#include <kernel/misc.h>
#include <kernel/panic.h>
#include <kernel/spinlock.h>
#include <initcall.h>
#include <stdint.h>
#include <string.h>
#include <io.h>
#include <sm/optee_smc.h>
#include <sm/std_smc.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <sta_helper.h>
#include <sta_smc.h>
#include <mbox.h>
#include <trace.h>

static unsigned int sta_svc_lock = SPINLOCK_UNLOCK;

/*
 * If CFG_STA_REMOTEPROC_CTRL is set, then read, write and update register
 * requests are forwarded to the remote processor, relying on "regmap" mailbox
 * channel.
 * In such a case, sanity check as well as execution of requests are delegated
 * to the remote processor. So the secure monitor behaves like a serving hatch.
 */
#if defined(CFG_STA_REMOTEPROC_CTRL)

/* mailbox channel id used to send remote requests to C-M3 Firmware */
static uint32_t msg_chan_id;

/*
 * Send a request to the remote micro-controller sub-system
 * through hardware mailbox device.
 */
static inline void sta_svc_send_msg(struct regmap_request *req)
{
	struct mbox_msg msg;

	msg.dsize = sizeof(struct regmap_request);
	msg.pdata = (uint8_t *)req;
	if (mbox_send_msg(msg_chan_id, &msg) != MBOX_SUCCESS)
		panic("mbox message sending failed");
}

/**
 * Get a response from the remote micro-controller sub-system
 * through hardware mailbox device.
 */
static inline void sta_svc_get_msg(struct regmap_request *req)
{
	struct mbox_msg msg;
	int ret;

	msg.pdata = (uint8_t *)req;
	ret = mbox_get_msg(msg_chan_id, &msg);

	while (ret != MBOX_SUCCESS) {
		if (ret == MBOX_ERROR)
			panic("mbox message reading failed");
		ret = mbox_get_msg(msg_chan_id, &msg);
	}
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
		panic("regmap smc read failed");

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
		panic("regmap smc write failed");

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
		panic("regmap smc update failed");

	return STA_SMC_OK;
}

static TEE_Result sta_svc_init(void)
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
		panic("sta svc mbox channel request failed");

	return TEE_SUCCESS;
}
driver_init(sta_svc_init);

#else /* CFG_STA_REMOTEPROC_CTRL */

static uint32_t sta_svc_smc_read(uint32_t val_bits, uint32_t reg, uint32_t *val)
{
	vaddr_t addr = (vaddr_t)phys_to_virt_io(reg);

	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		*val = read8(addr);
		break;
	case STA_REGMAP_SMC_VALBITS_16:
		*val = read16(addr);
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		*val = read32(addr);
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
	vaddr_t addr = (vaddr_t)phys_to_virt_io(reg);

	/* FIXME: Do a sanity check of the request, possibly by applying a mask
	 * on the value to be written.
	 */

	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		write8((uint8_t)val, addr);
		break;
	case STA_REGMAP_SMC_VALBITS_16:
		write16((uint16_t)val, addr);
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		write32((uint32_t)val, addr);
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
	vaddr_t addr = (vaddr_t)phys_to_virt_io(reg);

	/* FIXME: Do a sanity check of the request, possibly by applying a mask
	 * on the requested bits to be updated.
	 */

	switch (val_bits) {
	case STA_REGMAP_SMC_VALBITS_8:
		tmp = read8(addr);
		tmp &= ~mask;
		write8((uint8_t)(tmp | val), addr);
		break;
	case STA_REGMAP_SMC_VALBITS_16:
		tmp = read16(addr);
		tmp &= ~mask;
		write16((uint16_t)(tmp | val), addr);
		break;
	case STA_REGMAP_SMC_VALBITS_32:
		tmp = read32(addr);
		tmp &= ~mask;
		write32((uint32_t)(tmp | val), addr);
		break;
	case STA_REGMAP_SMC_VALBITS_64: /* FIXME: really handle 64 bits case */
		return STA_SMC_NOT_SUPPORTED;
	default:
		return STA_SMC_INVALID_PARAMS;
	}

	return STA_SMC_OK;
}

#endif /* CFG_STA_REMOTEPROC_CTRL */

/*
 * Top-level Platform Service SMC handler.
 *
 * Return: false
 *         -> if this is a platform service
 *            should return back to the non-secure state after handling
 *         true
 *         -> if this is not a platform service
 */
bool sm_platform_handler(struct sm_ctx *ctx)
{
	uint32_t return_value = 0;
	uint32_t smc_fid = ctx->nsec.r0;
	uint32_t x1 = ctx->nsec.r1;
	uint32_t x2 = ctx->nsec.r2;
	uint32_t x3 = ctx->nsec.r3;
	uint32_t x4 = ctx->nsec.r4;

	if (OPTEE_SMC_OWNER_NUM(smc_fid) != OPTEE_SMC_OWNER_SIP)
		return true;

	if (x1 != STA_REGMAP_SMC_VALBITS_8 &&
	    x1 != STA_REGMAP_SMC_VALBITS_16 &&
	    x1 != STA_REGMAP_SMC_VALBITS_32 &&
	    x1 != STA_REGMAP_SMC_VALBITS_64) {
		ctx->nsec.r0 = STA_SMC_INVALID_PARAMS;
		ctx->nsec.r1 = 0;
		return false;
	}

	cpu_spin_lock(&sta_svc_lock);

	switch (smc_fid & STA_REGMAP_SMC_TYPE_MSK) {
	case STA_REGMAP_SMC_READ:
		ctx->nsec.r0 = sta_svc_smc_read(x1, x2, &return_value);
		ctx->nsec.r1 = return_value;
		break;

	case STA_REGMAP_SMC_WRITE:
		ctx->nsec.r0 = sta_svc_smc_write(x1, x2, x3);
		break;

	case STA_REGMAP_SMC_UPDATE_BITS:
		ctx->nsec.r0 = sta_svc_smc_update_bits(x1, x2, x3, x4);
		break;

	default:
		EMSG("Unknown STA Service Call: 0x%x\n", smc_fid);
		ctx->nsec.r0 = OPTEE_SMC_RETURN_UNKNOWN_FUNCTION;
		break;
	}

	cpu_spin_unlock(&sta_svc_lock);
	return false;
}

