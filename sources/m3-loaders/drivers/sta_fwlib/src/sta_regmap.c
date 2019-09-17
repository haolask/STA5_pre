#if 0
#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LEVEL_INFO
#endif

#include <errno.h>
#include "sta_mbox.h"
#include "sta_regmap.h"
#include "sta_regmap_firewall.h"
#include "trace.h"
#include "utils.h"

static int regmap_chan_id;

static uint32_t regmap_read(uint32_t val_bits, uint32_t reg, uint32_t *val)
{

	switch (val_bits) {
	case STA_REGMAP_VALBITS_8:
		*val = read_byte_reg(reg);
		break;
	case STA_REGMAP_VALBITS_16:
		*val = read_short_reg(reg);
		break;
	case STA_REGMAP_VALBITS_32:
	case STA_REGMAP_VALBITS_64: /* FIXME: really handle 64 bits case */
		*val = read_reg(reg);
		break;
	default:
		return STA_REGMAP_INVALID_PARAMS;
	}

	TRACE_VERBOSE("%s @ 0x%x, val=0x%x\n", __func__, reg, val);
	return STA_REGMAP_OK;
}

static uint32_t regmap_update_bits(uint32_t val_bits, uint32_t reg,
				   uint32_t mask, uint32_t val,
				   uint32_t access_rights)
{
	uint32_t masked_val, tmp;

	/* apply access_rights mask to the requested register value */
	masked_val = (mask & val) & access_rights;

	switch (val_bits) {
	case STA_REGMAP_VALBITS_8:
		tmp = read_byte_reg(reg);
		tmp &= ~access_rights;
		write_byte_reg((uint8_t)(tmp | masked_val), reg);
		break;
	case STA_REGMAP_VALBITS_16:
		tmp = read_short_reg(reg);
		tmp &= ~access_rights;
		write_short_reg((uint16_t)(tmp | masked_val), reg);
		break;
	case STA_REGMAP_VALBITS_32:
	case STA_REGMAP_VALBITS_64: /* FIXME: really handle 64 bits case */
		tmp = read_reg(reg);
		tmp &= ~access_rights;
		write_reg(tmp | masked_val, reg);
		break;
	default:
		return STA_REGMAP_INVALID_PARAMS;
	}

	TRACE_VERBOSE("%s @ 0x%x, msk=0x%x, val=0x%x, access_rights=0x%x, written=0x%x\n",
		   __func__, reg, mask, val, access_rights, tmp | masked_val);
	return STA_REGMAP_OK;
}

static void regmap_cb(void *context, struct mbox_msg *msg)
{
	struct regmap_request *req;
	struct regmap_request rsp;
	struct mbox_msg msg_rsp;
	int ret;
	uint32_t access_rights;

	if (msg->dsize != sizeof(struct regmap_request)) {
		TRACE_ERR("%s: invalid msg\n", __func__);
		return;
	}

	req = (struct regmap_request *)msg->pdata;

	if (req->val_bits != STA_REGMAP_VALBITS_8 &&
	    req->val_bits != STA_REGMAP_VALBITS_16 &&
	    req->val_bits != STA_REGMAP_VALBITS_32 &&
	    req->val_bits != STA_REGMAP_VALBITS_64) {
		TRACE_ERR("%s: invalid valbits\n", __func__);
		ret = STA_REGMAP_INVALID_PARAMS;
		goto send_rsp;
	}

	ret = regmap_find_register_access_rights(req->reg, &access_rights);
	if (ret) {
		TRACE_ERR("%s: 0x%x has no access rights\n", __func__,
			  req->reg);
		return;
	}

	switch (req->type) {
	case STA_REGMAP_READ:
		ret = regmap_read(req->val_bits, req->reg, &rsp.val);
		break;
	case STA_REGMAP_WRITE:
		ret = regmap_update_bits(req->val_bits, req->reg, 0xFFFFFFFF,
					 req->val, access_rights);
		break;
	case STA_REGMAP_UPDATE_BITS:
		ret = regmap_update_bits(req->val_bits, req->reg, req->mask,
					 req->val, access_rights);
		break;
	default:
		TRACE_ERR("%s: invalid command\n", __func__);
		ret = STA_REGMAP_INVALID_PARAMS;
	}
send_rsp:
	rsp.type = req->type;
	rsp.reg = req->reg;
	rsp.status = (uint8_t)ret;
	msg_rsp.dsize = sizeof(struct regmap_request);
	msg_rsp.pdata = (uint8_t *)(&rsp);
	mbox_send_msg(regmap_chan_id, &msg_rsp);
}

/**
 * @brief	init regmap driver
 * @return	0 if no error, not 0 otherwise
 */
int regmap_init(void)
{
	struct mbox_chan_req mbreq;

	mbreq.chan_name = "regmap";
	mbreq.user_data = NULL;
	mbreq.rx_cb = regmap_cb;

	regmap_chan_id = mbox_request_channel(&mbreq);
	if (regmap_chan_id == MBOX_ERROR) {
		TRACE_INFO("%s: failed to allocate mbox channel\n", __func__);
		return -ENODEV;
	}

	TRACE_INFO("%s: done\n", __func__);
	return 0;
}

