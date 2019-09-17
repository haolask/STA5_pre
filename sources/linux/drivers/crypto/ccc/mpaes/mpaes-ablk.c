/*
 * ST C3 Micro-Programmable AES channel v0.3
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2016 STMicroelectronics Limited
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/crypto.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>

#include <crypto/aes.h>
#include <crypto/algapi.h>

#include "../ccc.h"
#include "mpaes.h"

#define INITIV 0x0c
#define ECB_ENC_APPEND ECB_MODE
#define ECB_DEC_APPEND (ECB_ENC_APPEND + DECRYPT)
#define CBC_ENC_APPEND CBC_MODE
#define CBC_DEC_APPEND (CBC_ENC_APPEND + DECRYPT)

int mpaes_ablk_set_key(struct crypto_ablkcipher *cipher, const u8 *key,
		       unsigned int key_size)
{
	struct mpaes_context *context = crypto_ablkcipher_ctx(cipher);

	context->tfm = &cipher->base;
	return mpaes_set_key(context, key, key_size);
}

static inline int set_iv(struct mpaes_context *context, u8 *iv)
{
	struct ccc_channel *channel = context->channel;

	dev_dbg(&context->channel->dev, "[%s]", __func__);

	memcpy(context->iv.for_cpu, iv, IV_SIZE);
	context->iv.size = IV_SIZE;
	ccc_dma_sync_for_device(&channel->dev, &context->iv, DMA_TO_DEVICE);
	return 0;
}

static unsigned char ablk_get_usequence(struct mpaes_context *context)
{
	return context->alg->mode + context->direction;
}

static struct operation ablk_get_opcode(int index,
					unsigned char usequence,
					unsigned short data_size)
{
	struct operation op;

	switch (usequence) {
	case ECB_ENC_APPEND:
	case ECB_DEC_APPEND:
	case CBC_ENC_APPEND:
	case CBC_DEC_APPEND:
		op.wn = 2;
		op.code = EXEC_SD;
		break;
	case INITIV:
		op.wn = 1;
		op.code = EXEC_S;
		break;
	default:
		return mpaes_get_opcode(index, usequence, data_size);
	}
	op.code <<= OP_ID_LSB;
	op.code |= (index << CHN_LSB) | (op.wn << WN_LSB);
	op.code |= (usequence << M_LSB) | (data_size << N_LSB);
	return op;
};

static bool ablk_need_iv(struct mpaes_context *context)
{
	return context->alg->mode != ECB_MODE;
}

static void ablk_program_prolog(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct operation op;

	if (ablk_need_iv(context)) {
		op = ablk_get_opcode(channel->index, INITIV, IV_SIZE);
		ccc_program(context->dispatcher, op.code, op.wn,
			    context->iv.for_dev);
	}
}

static int ablk_program_crypto_instructions(struct mpaes_context *context)
{
	return mpaes_program_crypto_instructions(context);
}

struct mpaes_alg mpaes_ablk_alg = {
	.program_prolog = ablk_program_prolog,
	.need_iv = ablk_need_iv,
	.program_crypto_instructions = ablk_program_crypto_instructions,
	.get_opcode = ablk_get_opcode,
	.get_usequence = ablk_get_usequence,
	.program_epilog = NULL,
	.postprocess = NULL,
};

static int ablk_xcrypt(struct ablkcipher_request *ablk_req,
		       unsigned int direction)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(ablk_req);
	struct mpaes_context *context = crypto_ablkcipher_ctx(cipher);

	context->ablk_req = ablk_req;
	return mpaes_xcrypt(context, direction);
}

int mpaes_ablk_decrypt(struct ablkcipher_request *ablk_req)
{
	return ablk_xcrypt(ablk_req, AES_DECRYPT);
}

int mpaes_ablk_encrypt(struct ablkcipher_request *ablk_req)
{
	return ablk_xcrypt(ablk_req, AES_ENCRYPT);
}
