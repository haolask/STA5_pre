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
#include <crypto/internal/aead.h>
#include <crypto/algapi.h>

#include "../ccc.h"
#include "mpaes.h"
#include "mpaes-ablk.h"
#include "mpaes-aead.h"

#define DEV_NAME "mpaes"
#define PR_SEP ": "

int mpaes_set_key(struct mpaes_context *context, const u8 *key,
		  unsigned int key_size)
{
	struct ccc_channel *channel = context->channel;
	u32 *flags = &context->tfm->crt_flags;

	dev_dbg(&channel->dev, "[%s]", __func__);

	if ((AES_MIN_KEY_SIZE > key_size) || (key_size > AES_MAX_KEY_SIZE)) {
		dev_err(&channel->dev, "[%s]: Unhandled key size of %dB",
			__func__, key_size);
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	/* key member of context is aligned on 32 bits. */
	memcpy(context->key.for_cpu, key, key_size);
	context->key.size = key_size;
	ccc_dma_sync_for_device(&channel->dev, &context->key, DMA_TO_DEVICE);
	return 0;
}

static inline void set_iv(struct mpaes_context *context, u8 *iv)
{
	struct ccc_channel *channel = context->channel;

	dev_dbg(&context->channel->dev, "[%s]", __func__);

	memcpy(context->iv.for_cpu, iv, AES_BLOCK_SIZE);
	context->iv.size = AES_BLOCK_SIZE;
	ccc_dma_sync_for_device(&channel->dev, &context->iv, DMA_TO_DEVICE);
}

struct operation mpaes_get_opcode(int index, unsigned char usequence,
				  unsigned short data_size)
{
	struct operation op;

	switch (usequence) {
	case KEY_LOAD:
	case KEY_LOAD_SCHED:
		op.wn = 2;
		op.code = EXEC_PS;
		break;
	case HALT:
		/* Fall through. */
	default:
		/* Can't fail. */
		op.code = HALT;
		op.wn = 0;
		break;
	}
	op.code <<= OP_ID_LSB;
	op.code |= (index << CHN_LSB) | (op.wn << WN_LSB);
	op.code |= (usequence << M_LSB) | (data_size << N_LSB);
	return op;
};

static inline unsigned char get_key_slot(struct mpaes_context *context)
{
	struct mpaes_data *data = get_mpaes_data(context);

	return ffs(data->gp_key_slots);
}

static inline void program_prolog(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct ccc_dispatcher *dispatcher;
	struct operation op;
	struct mpaes_alg *alg = context->alg;

	context->dispatcher = ccc_get_dispatcher(channel->controller);
	dispatcher = context->dispatcher;

	op = mpaes_get_opcode(channel->index,
			      context->direction == AES_ENCRYPT ?
			      KEY_LOAD : KEY_LOAD_SCHED,
			      context->key.size);
	ccc_program(dispatcher, op.code, op.wn, get_key_slot(context),
		    context->key.for_dev);

	if (alg->program_prolog)
		alg->program_prolog(context);
}

static inline int get_crypto_inst_len(void)
{
	/* Operation, source and destination tuple. */
	return 3 * sizeof(unsigned int);
}

static inline void program_epilog(struct mpaes_context *context)
{
	struct operation op;
	struct mpaes_alg *alg = context->alg;

	if (alg->program_epilog)
		alg->program_epilog(context);

	op = ccc_get_opcode(STOP, 0);
	ccc_program(context->dispatcher, op.code, op.wn);
}

static inline int run_program(struct mpaes_context *context)
{
	int run_ret, check_ret;

	run_ret = ccc_run_program(context->dispatcher);
	check_ret = ccc_check_channel_state(context->channel);
	return run_ret ? run_ret : check_ret;
}

static inline void close_program(struct mpaes_context *context)
{
	return ccc_put_dispatcher(context->dispatcher);
}

void mpaes_program_crypto_instruction(struct mpaes_context *context,
				      struct ccc_chunk *chunk,
				      bool *last)
{
	struct ccc_channel *channel = context->channel;
	struct ccc_dispatcher *dispatcher = context->dispatcher;
	struct mpaes_alg *alg = context->alg;
	struct operation op;

	if ((!alg->get_opcode) || (!alg->get_usequence)) {
		dev_err(&channel->dev, "Some function pointers not set\n");
		return;
	}

	op = alg->get_opcode(channel->index, alg->get_usequence(context),
			     chunk->in.size);
	ccc_program(dispatcher, op.code, op.wn, chunk->in.for_dev,
		    chunk->out.for_dev);
	*last = ccc_get_program_free_len(dispatcher) < get_crypto_inst_len();
}

static int program_crypto_instructions(struct mpaes_context *context)
{
	struct ccc_channel *channel = context->channel;
	struct mpaes_alg *alg = context->alg;

	if (alg->program_crypto_instructions)
		return alg->program_crypto_instructions(context);
	dev_err(&channel->dev, "No crypto instructions available\n");
	return -ENOENT;
}

int mpaes_program_crypto_instructions(struct mpaes_context *context)
{
	struct ablkcipher_request *ablk_req = context->ablk_req;
	struct ablkcipher_walk *walk = context->walk;
	struct ccc_channel *channel = context->channel;
	struct ccc_chunk *chunk;
	bool last;
	int nbytes, nents;
	bool in_place;
	struct ccc_step in, out;
	int ret;

	ablkcipher_walk_init(walk, ablk_req->dst, ablk_req->src,
			     ablk_req->nbytes);
	ret = ablkcipher_walk_phys(ablk_req, walk);
	if (ret) {
		dev_err(&channel->dev, "ablkcipher_walk_phys() failed\n");
		return ret;
	}
	nents = sg_nents(walk->in.sg);
	dev_dbg(&channel->dev, "walk.in.sg nents = %d", nents);

#ifdef CCC_DEBUG
	if (nents > 1)
		ccc_enable_single_step_command(context->dispatcher);
#endif

	INIT_LIST_HEAD(&context->chunks);

	if (context->alg->need_iv(context))
		set_iv(context, walk->iv);

	while ((nbytes = walk->nbytes) > 0) {
		chunk = kmalloc(sizeof(*chunk), GFP_KERNEL);
		if (!chunk)
			return -ENOMEM;
		INIT_LIST_HEAD(&chunk->entry);

		chunk->in.size = nbytes - (nbytes % context->blocksize);
		chunk->out.size = chunk->in.size;

		in.page = walk->src.page; in.offset = walk->src.offset;
		out.page = walk->dst.page; out.offset = walk->dst.offset;
		in_place = (in.page == out.page) && (in.offset == out.offset);

		if (in_place) {
			ret = ccc_map_step(&channel->dev, &in, &chunk->in,
					   DMA_BIDIRECTIONAL);
			if (ret)
				return ret;
			chunk->out.for_dev = chunk->in.for_dev;
		} else {
			ret = ccc_map_step(&channel->dev, &in, &chunk->in,
					   DMA_TO_DEVICE);
			if (ret)
				return ret;
			ret = ccc_map_step(&channel->dev, &out, &chunk->out,
					   DMA_FROM_DEVICE);
			if (ret)
				return ret;
		}

		list_add_tail(&chunk->entry, &context->chunks);

		mpaes_program_crypto_instruction(context, chunk, &last);
		nbytes -= chunk->in.size;
		if (last && nbytes) {
			dev_err(&channel->dev, "No more program memory left\n");
			return -EINVAL;
		}

		ret = ablkcipher_walk_done(ablk_req, walk, nbytes);
		if (ret)
			return ret;
	}
	return ret;
}

static void read_program_output(struct mpaes_context *context)
{
	struct device *dev = &context->channel->dev;
	struct ccc_chunk *chunk, *next;

	dev_dbg(&context->channel->dev, "[%s]\n", __func__);

	list_for_each_entry_safe(chunk, next, &context->chunks, entry) {
		if (chunk->out.for_dev == chunk->in.for_dev) {
			ccc_unmap_step(dev, &chunk->in, DMA_BIDIRECTIONAL);
		} else {
			ccc_unmap_step(dev, &chunk->in, DMA_TO_DEVICE);
			ccc_unmap_step(dev, &chunk->out, DMA_FROM_DEVICE);
		}
		list_del(&chunk->entry);
		kfree(chunk);
	}
	ablkcipher_walk_complete(context->walk);
}

static const struct ccc_channel_id mpaes_id[] = {
	{"st,mpaes-v0.3", 0x0000e003},
	{},
};

MODULE_DEVICE_TABLE(ccc, mpaes_id);

static int register_algs(void);

static int mpaes_probe(struct ccc_channel *channel)
{
	phys_addr_t physbase;
	int i = 0;
	bool match = false;
	int number_of_algs;
	struct ccc_channel_driver *driver;
	struct mpaes_data *data;
	unsigned int scr;

	if (!channel)
		return -ENODEV;

	physbase = ccc_get_ch_physbase(channel->controller, channel->index);
	channel->base = devm_ioremap(&channel->dev, physbase, CH_SIZE);
	if (IS_ERR(channel->base)) {
		dev_err(&channel->dev, "Failed to reserve memory region\n");
		return PTR_ERR(channel->base);
	}

	channel->id = ccc_get_ch_id(channel->base);
	if (!channel->id) {
		dev_err(&channel->dev, "Unused channel\n");
		return -ENODEV;
	}
	dev_dbg(&channel->dev, "Channel identifier: %08x\n", channel->id);

	while (mpaes_id[i].name[0] != '\0' && !match)
		match = (channel->id == mpaes_id[i++].id);
	if (!match) {
		dev_err(&channel->dev, "Unmatched channel identifier: %x\n",
			channel->id);
		return -ENODEV;
	}

	if (!ccc_is_channel_present(channel)) {
		dev_err(&channel->dev,
			"Channel %d does not exist\n", channel->index);
		return -EINVAL;
	}

	data = devm_kzalloc(&channel->dev, sizeof(struct mpaes_data),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	if (of_property_read_u16(channel->node, "gp-key-slots",
				 &data->gp_key_slots)) {
		dev_err(&channel->dev, "General purpose key slots undefined\n");
		return -ENODEV;
	}
	if (!data->gp_key_slots) {
		dev_err(&channel->dev, "No general purpose key slot\n");
		return -ENODEV;
	}

#define POOL_NAME "cryptographic material"
	data->pool = dmam_pool_create(POOL_NAME, &channel->dev,
				      max(AES_MAX_KEY_SIZE, AES_BLOCK_SIZE),
				      CCC_ALIGN_SIZE, 0);
	if (!data->pool) {
		dev_err(&channel->dev, "No suitable DMA pool for: "
			POOL_NAME "\n");
		return -ENOMEM;
	}

	channel->error_mask = BERR | DERR | PERR | IERR | AERR;
	ccc_set_channel_data(channel, data);
	ccc_init_channel(channel);

	scr = ccc_read_channel_scr(channel);
	dev_dbg(&channel->dev, "Endianness: %s\n",
		ccc_get_channel_scr_endian(scr) ? "little" : "big");

	number_of_algs = register_algs();
	if (!number_of_algs) {
		dev_err(&channel->dev, "No registered algorithm\n");
		return -ENODEV;
	}
	dev_info(&channel->dev, "%d algorithm%s registered", number_of_algs,
		 number_of_algs > 1 ? "s" : "");

	driver = to_ccc_channel_driver(channel->dev.driver);
	list_add_tail(&channel->probed, &driver->channels);
	return 0;
}

static void unregister_algs(void);

static int mpaes_remove(struct ccc_channel *channel)
{
	struct ccc_channel_driver *driver;

	list_del(&channel->probed);

	driver = to_ccc_channel_driver(channel->dev.driver);
	if (list_empty(&driver->channels))
		unregister_algs();
	return 0;
}

static struct ccc_channel_driver mpaes_driver = {
	.driver = {
		.name = DEV_NAME,
	},
	.probe = mpaes_probe,
	.remove = mpaes_remove,
	.id_table = mpaes_id,
};

static struct mpaes_alg *get_alg(struct crypto_tfm *tfm)
{
	u32 alg_type = crypto_tfm_alg_type(tfm);

	switch (alg_type) {
	case CRYPTO_ALG_TYPE_ABLKCIPHER:
		return &mpaes_ablk_alg;
	case CRYPTO_ALG_TYPE_AEAD:
		return &mpaes_aead_alg;
	default:
		break;
	}
	return NULL;
}

static unsigned int get_alg_mode(struct crypto_tfm *tfm)
{
	const char *alg_name = crypto_tfm_alg_name(tfm);

	pr_debug(DEV_NAME PR_SEP "Mode %s requested\n", alg_name);
	if (!strncmp("ecb", alg_name, 3))
		return ECB_MODE;
	if (!strncmp("cbc", alg_name, 3))
		return CBC_MODE;
	if (!strncmp("gcm", alg_name, 3))
		return GCM_MODE;
	return UNKNOWN_MODE;
}

static int mpaes_alloc_crypto_material(struct mpaes_context *context)
{
	struct dma_pool *pool = get_mpaes_data(context)->pool;

	context->dma_owner = &context->channel->dev;
	context->key.for_cpu = dma_pool_alloc(pool, GFP_KERNEL,
					      &context->key.for_dev);
	if (!context->key.for_cpu) {
		dev_err(context->dma_owner, "Cannot allocate key\n");
		goto err_key;
	}

	if (context->alg->need_iv(context)) {
		context->iv.for_cpu = dma_pool_alloc(pool, GFP_KERNEL,
						     &context->iv.for_dev);
		if (!context->iv.for_cpu) {
			dev_err(context->dma_owner, "Cannot allocate IV\n");
			goto err_iv;
		}
	}
	if (produce_tag(context->alg)) {
		context->final.for_cpu = dma_pool_alloc(pool, GFP_KERNEL,
							&context->
							final.for_dev);
		if (!context->final.for_cpu) {
			dev_err(context->dma_owner, "Cannot allocate final\n");
			goto err_final;
		}
		context->tag.for_cpu = dma_pool_alloc(pool, GFP_KERNEL,
						      &context->tag.for_dev);
		if (!context->tag.for_cpu) {
			dev_err(context->dma_owner, "Cannot allocate tag\n");
			goto err_tag;
		}
	}
	return 0;
err_tag:
	dma_pool_free(pool, context->final.for_cpu, context->iv.for_dev);
err_final:
	if (context->iv.for_cpu)
		dma_pool_free(pool, context->iv.for_cpu, context->iv.for_dev);
err_iv:
	dma_pool_free(pool, context->key.for_cpu, context->key.for_dev);
err_key:
	return -ENOMEM;
}

static int mpaes_cra_init(struct crypto_tfm *tfm)
{
	struct mpaes_context *context = crypto_tfm_ctx(tfm);

	pr_debug(DEV_NAME PR_SEP "[%s]", __func__);

	context->blocksize = crypto_tfm_alg_blocksize(tfm);
	context->alg = get_alg(tfm);
	if (!context->alg) {
		pr_err(DEV_NAME PR_SEP "Requested alg is not supported\n");
		return -ENOENT;
	}
	context->alg->mode = get_alg_mode(tfm);
	if (context->alg->mode == UNKNOWN_MODE) {
		pr_err(DEV_NAME PR_SEP "Requested mode is not supported\n");
		return -ENOENT;
	}
	context->channel = ccc_get_channel(&mpaes_driver);
	if (!context->channel) {
		pr_err(DEV_NAME PR_SEP "No such device\n");
		return -ENODEV;
	}
	return mpaes_alloc_crypto_material(context);
}

int mpaes_xcrypt(struct mpaes_context *context, unsigned int direction)
{
	struct ccc_channel *channel = context->channel;
	struct ablkcipher_walk walk;
	struct mpaes_alg *alg = context->alg;
	int ret;

	context->direction = direction;
	dev_dbg(&channel->dev, "%scryption is beginning\n",
		direction == AES_ENCRYPT ? "En" : "De");
	context->walk = &walk;

	program_prolog(context);

	ret = program_crypto_instructions(context);
	if (ret)
		goto exit;

	program_epilog(context);
	ret = run_program(context);

exit:
	close_program(context);
	read_program_output(context);
	if (alg->postprocess)
		return alg->postprocess(context);
	return ret;
}

static void mpaes_free_crypto_material(struct mpaes_context *context)
{
	struct dma_pool *pool = get_mpaes_data(context)->pool;

	if (produce_tag(context->alg)) {
		dma_pool_free(pool, context->tag.for_cpu, context->tag.for_dev);
		dma_pool_free(pool, context->final.for_cpu,
			      context->final.for_dev);
	}
	if (context->alg->need_iv(context))
		dma_pool_free(pool, context->iv.for_cpu, context->iv.for_dev);
	dma_pool_free(pool, context->key.for_cpu, context->key.for_dev);
}

static void mpaes_cra_exit(struct crypto_tfm *tfm)
{
	struct mpaes_context *context = crypto_tfm_ctx(tfm);
	struct ccc_channel *channel = context->channel;

	dev_dbg(&channel->dev, "[%s]", __func__);

	mpaes_free_crypto_material(context);
	ccc_put_channel(channel);
}

static struct crypto_alg mpaes_crypto_algs[] = {
	{
		.cra_name = "ecb(aes)",
		.cra_driver_name = "mpaes-ecb-aes",
		.cra_priority = 300,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct mpaes_context),
		.cra_alignmask = CCC_ALIGN_SIZE - 1,
		.cra_type = &crypto_ablkcipher_type,
		.cra_init = mpaes_cra_init,
		.cra_exit = mpaes_cra_exit,
		.cra_module = THIS_MODULE,
		.cra_u = {
			.ablkcipher = {
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.setkey = mpaes_ablk_set_key,
				.encrypt = mpaes_ablk_encrypt,
				.decrypt = mpaes_ablk_decrypt,
			}
		}
	},
	{
		.cra_name = "cbc(aes)",
		.cra_driver_name = "mpaes-cbc-aes",
		.cra_priority = 300,
		.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		.cra_blocksize = AES_BLOCK_SIZE,
		.cra_ctxsize = sizeof(struct mpaes_context),
		.cra_alignmask = CCC_ALIGN_SIZE - 1,
		.cra_type = &crypto_ablkcipher_type,
		.cra_init = mpaes_cra_init,
		.cra_exit = mpaes_cra_exit,
		.cra_module = THIS_MODULE,
		.cra_u = {
			.ablkcipher = {
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.setkey = mpaes_ablk_set_key,
				.encrypt = mpaes_ablk_encrypt,
				.decrypt = mpaes_ablk_decrypt,
				.ivsize = AES_BLOCK_SIZE,
			}
		}
	}
};

static struct aead_alg mpaes_aead_algs[] = {
	{
		.base = {
			.cra_name = "gcm(aes)",
			.cra_driver_name = "mpaes-gcm-aes",
			.cra_priority = 300,
			.cra_flags = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC,
			.cra_blocksize = AES_BLOCK_SIZE,
			.cra_ctxsize = sizeof(struct mpaes_context),
			.cra_alignmask = CCC_ALIGN_SIZE - 1,
			.cra_init = mpaes_cra_init,
			.cra_exit = mpaes_cra_exit,
			.cra_module = THIS_MODULE,
		},
		.setkey = mpaes_aead_set_key,
		.setauthsize = mpaes_aead_setauthsize,
		.encrypt = mpaes_aead_encrypt,
		.decrypt = mpaes_aead_decrypt,
		.ivsize = 12,
		.maxauthsize = AES_BLOCK_SIZE,
	}

};

static int register_algs(void)
{
	int i, count = 0;

	for (i = ARRAY_SIZE(mpaes_crypto_algs); i--; ) {
		if (crypto_register_alg(&mpaes_crypto_algs[i]))
			pr_err(DEV_NAME PR_SEP "[%s] alg registration failed\n",
			       mpaes_crypto_algs[i].cra_driver_name);
		else
			count++;
	}
	for (i = ARRAY_SIZE(mpaes_aead_algs); i--; ) {
		if (crypto_register_aead(&mpaes_aead_algs[i]))
			pr_err(DEV_NAME PR_SEP "[%s] alg registration failed\n",
			       mpaes_aead_algs[i].base.cra_driver_name);
		else
			count++;
	}
	return count;
}

static void unregister_algs(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mpaes_aead_algs); i++)
		crypto_unregister_aead(&mpaes_aead_algs[i]);
	for (i = 0; i < ARRAY_SIZE(mpaes_crypto_algs); i++)
		crypto_unregister_alg(&mpaes_crypto_algs[i]);
}

static int __init mpaes_init(void)
{
	return ccc_register_channel_driver(THIS_MODULE, &mpaes_driver);
}
module_init(mpaes_init);

static void __exit mpaes_exit(void)
{
	ccc_delete_channel_driver(&mpaes_driver);
}
module_exit(mpaes_exit);

MODULE_DESCRIPTION("ST C3 Micro-Programmable AES channel driver");
MODULE_ALIAS_CRYPTO("aes-ecb");
MODULE_ALIAS_CRYPTO("aes-cbc");
MODULE_ALIAS_CRYPTO("aes-gcm");
MODULE_LICENSE("GPL");
