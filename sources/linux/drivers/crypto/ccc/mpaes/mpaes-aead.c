/*
 * ST C3 Micro-Programmable AES channel v0.3
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2017 STMicroelectronics Limited
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
#include <crypto/internal/aead.h>
#include <crypto/scatterwalk.h>

#include "../ccc.h"
#include "mpaes.h"

#define DEV_NAME "mpaes-aead"
#define PR_SEP ": "

#define IV_SIZE AES_BLOCK_SIZE
#define GCM_TAG_SIZE (128 / 8)
#define GCM_FINAL_SIZE (128 / 8)
#define GCM_INITIV 0x19
#define GCM_HEADER 0x1a
#define GCM_ENC_APPEND GCM_MODE
#define GCM_ENC_LAST_APPEND (GCM_ENC_APPEND + 1)
#define GCM_DEC_APPEND (GCM_ENC_APPEND + 2)
#define GCM_DEC_LAST_APPEND (GCM_DEC_APPEND + 1)
#define GCM_FINAL 0x1f

int mpaes_aead_set_key(struct crypto_aead *cipher, const u8 *key,
		       unsigned int key_size)
{
	struct mpaes_context *context = crypto_aead_ctx(cipher);

	context->tfm = &cipher->base;
	return mpaes_set_key(context, key, key_size);
}

static inline unsigned char get_iv_size(int mode)
{
	return IV_SIZE;
}

static inline int set_iv(struct mpaes_context *context, u8 *iv)
{
	struct ccc_channel *channel = context->channel;
	struct mpaes_alg *alg = context->alg;

	dev_dbg(&context->channel->dev, "[%s]", __func__);

	memcpy(context->iv.for_cpu, iv, get_iv_size(alg->mode));
	context->iv.size = get_iv_size(alg->mode);
	ccc_dma_sync_for_device(&channel->dev, &context->iv, DMA_TO_DEVICE);
	return 0;
}

static unsigned char aead_get_usequence(struct mpaes_context *context)
{
	int mode = context->alg->mode;
	unsigned int useq = 0;

	if (mode == GCM_MODE) {
		switch (context->direction) {
		case HEADER:
			useq = GCM_HEADER;
			break;
		case DECRYPT:
			useq = GCM_DEC_APPEND;
			break;
		case ENCRYPT:
			useq = GCM_ENC_APPEND;
			break;
		default:
			pr_err(DEV_NAME PR_SEP "Unknown qualifier\n");
			break;
		}
	} else {
		return mode + context->direction;
	}
	return useq;
}

static struct operation aead_get_opcode(int index, unsigned char usequence,
					unsigned short data_size)
{
	struct operation op;

	switch (usequence) {
	case GCM_HEADER:
	case GCM_ENC_APPEND:
	case GCM_DEC_APPEND:
	case GCM_FINAL:
		op.wn = 2;
		op.code = EXEC_SD;
		break;
	case GCM_INITIV:
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

static inline unsigned char get_initiv_usequence(int mode)
{
	return GCM_INITIV;
}

static void aead_program_prolog(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct operation op;
	struct mpaes_alg *alg = context->alg;

	op = aead_get_opcode(channel->index,
			     get_initiv_usequence(alg->mode),
			     get_iv_size(alg->mode));
	ccc_program(context->dispatcher, op.code, op.wn,
		    context->iv.for_dev);
}

static inline unsigned char get_final_usequence(int mode_id)
{
	return GCM_FINAL;
}

static inline unsigned char get_final_size(int mode_id)
{
	return GCM_FINAL_SIZE;
}

/* Maps final block expected by engine. */
struct final {
	unsigned int header_size_most;
	unsigned int header_size_least;
	unsigned int payload_size_most;
	unsigned int payload_size_least;
};

static inline void set_tag_size(struct mpaes_context *context)
{
	struct aead_request *aead_req = context->aead_req;

	context->tag.size = crypto_aead_authsize(crypto_aead_reqtfm(aead_req));
}

static inline void set_final(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct final *final;
	struct aead_request *req = context->aead_req;
	unsigned int nbytes = req->cryptlen;

	dev_dbg(&channel->dev, "[%s]", __func__);

	final = context->final.for_cpu;
	context->final.size = sizeof(*final);
	memset(final, '\0', sizeof(*final));

	/* 8 * MAX_DATA_SIZE is less or equal to UINT32_MAX */
	final->header_size_least = cpu_to_be32(8 * req->assoclen);
	if (context->direction == AES_DECRYPT)
		nbytes -= context->tag.size;
	final->payload_size_least = cpu_to_be32(8 * nbytes);
	ccc_dma_sync_for_device(&channel->dev, &context->final, DMA_TO_DEVICE);
}

static void aead_program_epilog(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct operation op;
	struct mpaes_alg *alg = context->alg;

	set_tag_size(context);
	set_final(context);
	op = aead_get_opcode(channel->index,
			     get_final_usequence(alg->mode),
			     get_final_size(alg->mode));
	ccc_program(context->dispatcher, op.code, op.wn, context->final.for_dev,
		    context->tag.for_dev);
}

static int aead_postprocess(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct aead_request *req = context->aead_req;
	unsigned int nbytes = req->assoclen + req->cryptlen;
	int ret = 0;

	ccc_dma_sync_for_cpu(&channel->dev, &context->tag, DMA_FROM_DEVICE);
	if (context->direction == AES_ENCRYPT) {
		scatterwalk_map_and_copy(context->tag.for_cpu, req->dst, nbytes,
					 context->tag.size, 1);
	} else {
		unsigned char ref_tag[AES_BLOCK_SIZE];

		nbytes -= context->tag.size;
		scatterwalk_map_and_copy(ref_tag, req->src, nbytes,
					 context->tag.size, 0);
		if (memcmp(ref_tag, context->tag.for_cpu, context->tag.size))
			ret = -EBADMSG;
	}
	return ret;
}

static bool aead_need_iv(struct mpaes_context *context)
{
	/* Prevent to change IV after AAD computation. */
	return context->direction != HEADER;
}

static int aead_program_crypto_instructions(struct mpaes_context *context)
{
	struct ablkcipher_request ablk_req;
	struct aead_request *req = context->aead_req;
	enum qualifier direction = context->direction;
	unsigned int nbytes = req->cryptlen;
	int ret;

	context->ablk_req = &ablk_req;
	ablk_req.base.tfm = context->tfm;
	if (req->assoclen) {
		ablkcipher_request_set_crypt(context->ablk_req, req->src,
					     req->src, req->assoclen, req->iv);
		context->direction = HEADER;
		ret = mpaes_program_crypto_instructions(context);
		context->direction = direction;
		if (ret)
			return ret;
	}
	if (direction == AES_DECRYPT)
		nbytes -= context->tag.size;
	ablkcipher_request_set_crypt(context->ablk_req,
				     req->src + req->assoclen,
				     req->dst + req->assoclen,
				     nbytes, req->iv);
	return mpaes_program_crypto_instructions(context);
}

struct mpaes_alg mpaes_aead_alg = {
	.program_prolog = aead_program_prolog,
	.need_iv = aead_need_iv,
	.program_crypto_instructions = aead_program_crypto_instructions,
	.get_opcode = aead_get_opcode,
	.get_usequence = aead_get_usequence,
	.program_epilog = aead_program_epilog,
	.postprocess = aead_postprocess,
};

static int aead_xcrypt(struct aead_request *aead_req,
		       unsigned int direction)
{
	struct crypto_aead *cipher = crypto_aead_reqtfm(aead_req);
	struct mpaes_context *context = crypto_aead_ctx(cipher);
	struct ccc_channel *channel = context->channel;

	if (aead_req->assoclen % AES_BLOCK_SIZE) {
		dev_err(&channel->dev, "[%s]: Unhandled AAD of %uB", __func__,
			aead_req->assoclen);
		return -EINVAL;
	}

	context->aead_req = aead_req;
	return mpaes_xcrypt(context, direction);
}

int mpaes_aead_decrypt(struct aead_request *aead_req)
{
	return aead_xcrypt(aead_req, AES_DECRYPT);
}

int mpaes_aead_encrypt(struct aead_request *aead_req)
{
	return aead_xcrypt(aead_req, AES_ENCRYPT);
}

int mpaes_aead_setauthsize(struct crypto_aead *tfm,
			   unsigned int authsize)
{
	if (authsize > crypto_aead_alg(tfm)->maxauthsize) {
		pr_err(DEV_NAME PR_SEP "Unhandled tag size of %uB\n", authsize);
		return -EINVAL;
	}

	return 0;
}
