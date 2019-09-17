/*
 * ST C3 Micro-Programmable PKA channel v0.7
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
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/io.h>
#include <linux/module.h>

#include <crypto/akcipher.h>
#include <crypto/internal/akcipher.h>
#include <crypto/internal/rsa.h>
#include <crypto/scatterwalk.h>

#include "../ccc.h"
#include "pka.h"

#define DEV_NAME "pka"
#define PR_SEP ": "

#define MAX_RSA_MODULUS_SIZE_IN_BITS 3072
#define MOD_SIZE_IN_WORDS (MAX_RSA_MODULUS_SIZE_IN_BITS / 32)

/*
 * This structure is used to reserve memory only. It can not be passed to
 * channel directly because exp_len field location is variable i.e. after
 * modulus.
 */
struct pka_rsa_io {
	unsigned int op_len;
	unsigned int mod[MOD_SIZE_IN_WORDS];
	unsigned int exp_len;
	unsigned int exp[MOD_SIZE_IN_WORDS];
};

static inline unsigned int get_alg(struct crypto_akcipher *tfm)
{
	const char *alg_name = akcipher_alg_name(tfm);

	pr_debug(DEV_NAME PR_SEP "Alg %s requested\n", alg_name);
	if (!strncmp("rsa", alg_name, 3))
		return RSA_ALG;
	return UNKNOWN_ALG;
}

static struct operation pka_get_opcode(int index, unsigned char code,
				       unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case MONTY_PAR:
		op.code = code << OP_ID_LSB;
		op.wn = 2;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << LENGTH_LSB;
		va_end(params);
		break;
	case MOD_EXP:
		op.code = code << OP_ID_LSB;
		op.wn = 3;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << COUNTERMEASURE_LSB;
		op.code |= va_arg(params, unsigned int) << LENGTH_LSB;
		va_end(params);
		break;
	case MONTY_EXP:
		op.code = code << OP_ID_LSB;
		op.wn = 3;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << COUNTERMEASURE_LSB;
		op.code |= va_arg(params, unsigned int) << LENGTH_LSB;
		va_end(params);
		break;
	default:
		/* Can't fail. */
		op.wn = 0;
		break;
	}
	op.code |= (index << CHN_LSB);
	return op;
}

static const struct ccc_channel_id pka_id[] = {
	{"st,pka-v0.7", 0x00006009},
	{},
};

MODULE_DEVICE_TABLE(ccc, pka_id)

static int register_algs(void);

int pka_probe(struct ccc_channel *channel)
{
	phys_addr_t physbase;
	int i = 0;
	bool match = false;
	int number_of_algs;
	struct ccc_channel_driver *driver;
	struct pka_data *data;
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

	while (pka_id[i].name[0] != '\0' && !match)
		match = (channel->id == pka_id[i++].id);
	if (!match) {
		dev_info(&channel->dev, "Unmatched channel identifier: %x\n",
			 channel->id);
		return -ENODEV;
	}

	if (!ccc_is_channel_present(channel)) {
		dev_err(&channel->dev,
			"Channel %d does not exist\n", channel->index);
		return -EINVAL;
	}

	data = devm_kzalloc(&channel->dev, sizeof(struct pka_data),
			    GFP_KERNEL);
	if (!data)
		return -ENOMEM;

#define POOL_NAME "cryptographic material"
	data->pool = dmam_pool_create(POOL_NAME, &channel->dev,
				      sizeof(struct pka_rsa_io),
				      CCC_ALIGN_SIZE, 0);
	if (!data->pool) {
		dev_err(&channel->dev, "No suitable DMA pool for: "
			POOL_NAME "\n");
		return -ENOMEM;
	}

	/* TODO: Make use of DT for countermeasure selection. */
	data->countermeasure = AGAINST_DPA;

	channel->error_mask = BERR | DERR | PERR | IERR | AERR | OERR;
	ccc_set_channel_data(channel, data);
	ccc_init_channel(channel);

	scr = ccc_read_channel_scr(channel);
	dev_dbg(&channel->dev, "Endianness: %s\n",
		ccc_get_channel_scr_endian(scr) ?
		"little/swap" : "big/no swap");

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

static int pka_remove(struct ccc_channel *channel)
{
	struct ccc_channel_driver *driver;

	list_del(&channel->probed);

	driver = to_ccc_channel_driver(channel->dev.driver);
	if (list_empty(&driver->channels))
		unregister_algs();
	return 0;
}

static struct ccc_channel_driver pka_driver = {
	.driver = {
		.name = DEV_NAME,
	},
	.probe = pka_probe,
	.remove = pka_remove,
	.id_table = pka_id,
};

static int pka_rsa_check_key_length(unsigned int len)
{
	if (len <= 3072)
		return 0;
	return -EINVAL;
}

static int prepare_chunk(struct pka_context *context)
{
	struct device *dev = &context->channel->dev;
	struct ccc_dma *in, *out;
	struct akcipher_request *req = context->req;
	int ret;

	/*
	 * Input and output of modular exponentiation.
	 * "in" is either m, c, m or s and "out" is either c, m, s or m for
	 * respectively encrypt, decrypt, sign or verify operation.
	 */
	in = &context->chunk.in;
	if (sg_is_last(req->src) && req->src_len == context->n_size) {
		in->for_cpu = NULL;
		in->for_dev = dma_map_single(dev, sg_virt(req->src),
					     req->src_len, DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(dev, in->for_dev)))
			return -ENOMEM;
	} else {
		int offset = context->n_size - req->src_len;

		in->for_cpu = dma_zalloc_coherent(dev, context->n_size,
						  &in->for_dev, GFP_KERNEL);
		if (unlikely(!in->for_cpu))
			return -ENOMEM;
		scatterwalk_map_and_copy(in->for_cpu + offset, req->src, 0,
					 req->src_len, 0);
	}
	in->size = context->n_size;
	ret = -ENOMEM;
	out = &context->chunk.out;
	if (sg_is_last(req->dst) && req->dst_len == context->n_size) {
		out->for_cpu = NULL;
		out->for_dev = dma_map_single(dev, sg_virt(req->dst),
					      req->dst_len, DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(dev, out->for_dev)))
			goto unmap_input;
	} else {
		out->for_cpu = dma_zalloc_coherent(dev, context->n_size,
						   &out->for_dev, GFP_KERNEL);
		if (unlikely(!out->for_cpu))
			goto unmap_input;
	}
	out->size = context->n_size;
	return 0;
unmap_input:
	if (in->for_cpu)
		dma_free_coherent(dev, in->size, in->for_cpu, in->for_dev);
	else
		dma_unmap_single(dev, in->for_dev, in->size, DMA_TO_DEVICE);
	return ret;
}

static inline void program_epilog(struct pka_context *context)
{
	struct operation op;

	op = ccc_get_opcode(STOP, 0);
	ccc_program(context->dispatcher, op.code, op.wn);
}

static inline int run_program(struct pka_context *context)
{
	int run_ret, check_ret;

	run_ret = ccc_run_program(context->dispatcher);
	check_ret = ccc_check_channel_state(context->channel);
	return run_ret ? run_ret : check_ret;
}

static inline void close_program(struct pka_context *context)
{
	return ccc_put_dispatcher(context->dispatcher);
}

static void program_crypto_instructions(struct pka_context *context,
					struct ccc_dma *input)
{
	struct ccc_channel *channel = context->channel;
	struct ccc_dispatcher *dispatcher;
	struct operation op;
	struct ccc_chunk *chunk = &context->chunk;

	context->dispatcher = ccc_get_dispatcher(channel->controller);
	dispatcher = context->dispatcher;

	op = pka_get_opcode(channel->index, MONTY_EXP, 2,
			    get_pka_data(context)->countermeasure, input->size);
	ccc_program(dispatcher, op.code, op.wn, input->for_dev,
		    chunk->in.for_dev, chunk->out.for_dev);
}

static void read_program_output(struct pka_context *context)
{
	struct device *dev = &context->channel->dev;
	struct ccc_dma *in, *out;
	struct akcipher_request *req = context->req;

	dev_dbg(dev, "[%s]\n", __func__);

	in = &context->chunk.in;
	if (in->for_cpu)
		dma_free_coherent(dev, in->size, in->for_cpu, in->for_dev);
	else
		dma_unmap_single(dev, in->for_dev, in->size, DMA_TO_DEVICE);
	out = &context->chunk.out;
	if (out->for_cpu) {
		scatterwalk_map_and_copy(out->for_cpu, req->dst, 0,
					 req->dst_len, 1);
		dma_free_coherent(dev, out->size, out->for_cpu, out->for_dev);
	} else {
		dma_unmap_single(dev, out->for_dev, out->size, DMA_FROM_DEVICE);
	}
}

static int pka_rsa_mod_exp(struct akcipher_request *req, bool private)
{
	struct crypto_akcipher *tfm = crypto_akcipher_reqtfm(req);
	struct pka_context *context = akcipher_tfm_ctx(tfm);
	int exponent_size = private ? context->d_size : context->e_size;
	int ret;

	if (unlikely(!context->n_size || !exponent_size)) {
		dev_err(&context->channel->dev, "%s key not set\n", private ?
			"Private" : "Public");
		return -EINVAL;
	}
	context->req = req;
	ret = prepare_chunk(context);
	if (ret)
		return ret;

	program_crypto_instructions(context, private ?
				    &context->rsa_prv : &context->rsa_pub);

	program_epilog(context);
	ret = run_program(context);

	close_program(context);
	read_program_output(context);
	return ret;
}

static int pka_rsa_enc(struct akcipher_request *req)
{
	return pka_rsa_mod_exp(req, false);
}

static int pka_rsa_dec(struct akcipher_request *req)
{
	return pka_rsa_mod_exp(req, true);
}

/* Returns the dimension of big-endian big number in bytes. */
static unsigned int get_byte_dimension(const unsigned char *bytes,
				       unsigned int nr_bytes)
{
	unsigned int i;

	for (i = 0; i < nr_bytes; i++) {
		if (*bytes)
			return nr_bytes - i;
		bytes++;
	}
	return 0;
}

/* Returns the dimension of big-endian big number in bits. */
static unsigned int get_dimension(const unsigned char *bytes,
				  unsigned int nr_bytes)
{
	unsigned int i, nr;

	for (i = 0; i < nr_bytes; i++) {
		nr = *bytes ? fls(*bytes & 0xff) : 0;
		if (nr)
			return nr + (nr_bytes - (i + 1)) * 8;
		bytes++;
	}
	return 0;
}

static inline unsigned char *word_padd(unsigned char *bignum, unsigned int size)
{
	unsigned int remainder, padding = 0;

	remainder = size % sizeof(unsigned int);
	if (remainder) {
		padding = sizeof(unsigned int) - remainder;
		memset(bignum, '\0', padding);
	}
	return bignum + padding;
}

static int append_bignum(unsigned char **buf, const u8 *bignum, size_t size,
			 unsigned int *real_size)
{
	unsigned int nr_bits, nr_zeros;

	if (!*real_size)
		*real_size = get_byte_dimension(bignum, size);
	if (!*real_size)
		return -EINVAL;
	nr_bits = cpu_to_be32(get_dimension(bignum, size));
	memcpy(*buf, (unsigned char *)&(nr_bits), sizeof(nr_bits));
	*buf += sizeof(nr_bits);
	*buf = word_padd(*buf, *real_size);
	nr_zeros = size - *real_size;
	memcpy(*buf, bignum + nr_zeros, *real_size);
	*buf += *real_size;
	return 0;
}

static int pka_rsa_set_pub_key(struct crypto_akcipher *tfm, const void *key,
			       unsigned int keylen)
{
	struct pka_context *context = akcipher_tfm_ctx(tfm);
	struct device *dev = &context->channel->dev;
	struct ccc_dma *rsa_pub = &context->rsa_pub;
	unsigned char *pub = (unsigned char *)(rsa_pub->for_cpu);
	struct rsa_key raw_key = {0};
	int ret;

	ret = rsa_parse_pub_key(&raw_key, key, keylen);
	if (ret)
		return ret;

	/* Modulus */
	context->n_size = get_byte_dimension(raw_key.n, raw_key.n_sz);
	if (pka_rsa_check_key_length(context->n_size)) {
		dev_err(dev, "[%s] Unhandled modulus length of %dbits\n",
			__func__, context->n_size);
		context->n_size = 0;
		return -EINVAL;
	}
	ret = append_bignum(&pub, raw_key.n, raw_key.n_sz, &context->n_size);
	if (ret) {
		dev_err(dev, "[%s] Bad modulus\n", __func__);
		context->n_size = 0;
		return ret;
	}

	/* Public exponent */
	ret = append_bignum(&pub, raw_key.e, raw_key.e_sz, &context->e_size);
	if (ret) {
		dev_err(dev, "[%s] Bad public exponent\n", __func__);
		context->e_size = 0;
		return ret;
	}

	context->rsa_pub.size = pub - (unsigned char *)(rsa_pub->for_cpu);

	ccc_dma_sync_for_device(dev, &context->rsa_pub, DMA_TO_DEVICE);
	return 0;
}

static int pka_rsa_set_prv_key(struct crypto_akcipher *tfm, const void *key,
			       unsigned int keylen)
{
	struct pka_context *context = akcipher_tfm_ctx(tfm);
	struct device *dev = &context->channel->dev;
	struct ccc_dma *rsa_prv = &context->rsa_prv;
	unsigned char *prv = (unsigned char *)(rsa_prv->for_cpu);
	struct ccc_dma *rsa_pub = &context->rsa_pub;
	unsigned char *pub = (unsigned char *)(rsa_pub->for_cpu);
	struct rsa_key raw_key = {0};
	int ret;

	ret = rsa_parse_priv_key(&raw_key, key, keylen);
	if (ret)
		return ret;

	/* Modulus */
	context->n_size = get_byte_dimension(raw_key.n, raw_key.n_sz);
	if (pka_rsa_check_key_length(context->n_size)) {
		dev_err(dev, "[%s] Unhandled modulus length of %dbits\n",
			__func__, context->n_size);
		context->n_size = 0;
		return -EINVAL;
	}
	ret = append_bignum(&prv, raw_key.n, raw_key.n_sz, &context->n_size);
	if (ret) {
		dev_err(dev, "[%s] Bad modulus\n", __func__);
		context->n_size = 0;
		return ret;
	}
	append_bignum(&pub, raw_key.n, raw_key.n_sz, &context->n_size);

	/* Private exponent */
	ret = append_bignum(&prv, raw_key.d, raw_key.d_sz, &context->d_size);
	if (ret) {
		dev_err(dev, "[%s] Bad private exponent\n", __func__);
		context->d_size = 0;
		return ret;
	}

	/* Public exponent */
	ret = append_bignum(&pub, raw_key.e, raw_key.e_sz, &context->e_size);
	if (ret) {
		dev_err(dev, "[%s] Bad public exponent\n", __func__);
		context->e_size = 0;
		return ret;
	}

	context->rsa_prv.size = prv - (unsigned char *)(rsa_prv->for_cpu);
	context->rsa_pub.size = pub - (unsigned char *)(rsa_pub->for_cpu);

	ccc_dma_sync_for_device(dev, &context->rsa_prv, DMA_TO_DEVICE);
	ccc_dma_sync_for_device(dev, &context->rsa_pub, DMA_TO_DEVICE);
	return 0;
}

static int pka_rsa_max_size(struct crypto_akcipher *tfm)
{
	return ((struct pka_context *)akcipher_tfm_ctx(tfm))->n_size;
}

static int pka_alloc_crypto_material(struct pka_context *context)
{
	struct dma_pool *pool = get_pka_data(context)->pool;

	context->dma_owner = &context->channel->dev;
	context->rsa_pub.for_cpu = dma_pool_alloc(pool, GFP_KERNEL,
						  &context->rsa_pub.for_dev);
	if (!context->rsa_pub.for_cpu) {
		dev_err(context->dma_owner, "Cannot allocate public data\n");
		return -ENOMEM;
	}
	context->rsa_prv.for_cpu = dma_pool_alloc(pool, GFP_KERNEL,
						  &context->rsa_prv.for_dev);
	if (!context->rsa_prv.for_cpu) {
		dev_err(context->dma_owner, "Cannot allocate private data\n");
		return -ENOMEM;
	}
	return 0;
}

static int pka_rsa_init(struct crypto_akcipher *tfm)
{
	struct pka_context *context = akcipher_tfm_ctx(tfm);

	pr_debug(DEV_NAME PR_SEP "[%s]", __func__);

	context->n_size = 0;
	context->e_size = 0;
	context->d_size = 0;
	context->alg = get_alg(tfm);
	if (context->alg == UNKNOWN_ALG) {
		pr_err(DEV_NAME PR_SEP "Requested alg is not supported\n");
		return -ENOENT;
	}
	context->channel = ccc_get_channel(&pka_driver);
	if (!context->channel) {
		pr_err(DEV_NAME PR_SEP "No such device\n");
		return -ENODEV;
	}
	return pka_alloc_crypto_material(context);
}

static void pka_free_crypto_material(struct pka_context *context)
{
	struct dma_pool *pool = get_pka_data(context)->pool;

	dma_pool_free(pool, context->rsa_prv.for_cpu, context->rsa_prv.for_dev);
	dma_pool_free(pool, context->rsa_pub.for_cpu, context->rsa_pub.for_dev);
}

static void pka_rsa_exit(struct crypto_akcipher *tfm)
{
	struct pka_context *context = akcipher_tfm_ctx(tfm);
	struct ccc_channel *channel = context->channel;

	dev_dbg(&channel->dev, "[%s]", __func__);

	pka_free_crypto_material(context);
	ccc_put_channel(channel);
	context->d_size = 0;
	context->e_size = 0;
	context->n_size = 0;
}

static struct akcipher_alg pka_crypto_alg = {
	.base = {
		.cra_name = "rsa",
		.cra_driver_name = "pka-rsa",
		.cra_priority = 300,
		.cra_module = THIS_MODULE,
		.cra_ctxsize = sizeof(struct pka_context),
	},
	.encrypt = pka_rsa_enc,
	.decrypt = pka_rsa_dec,
	.sign = pka_rsa_dec,
	.verify = pka_rsa_enc,
	.set_priv_key = pka_rsa_set_prv_key,
	.set_pub_key = pka_rsa_set_pub_key,
	.max_size = pka_rsa_max_size,
	.init = pka_rsa_init,
	.exit = pka_rsa_exit,
};

static int register_algs(void)
{
	if (crypto_register_akcipher(&pka_crypto_alg)) {
		pr_err(DEV_NAME PR_SEP "[%s] alg registration failed\n",
		       pka_crypto_alg.base.cra_driver_name);
		return 0;
	}
	return 1;
}

static void unregister_algs(void)
{
	crypto_unregister_akcipher(&pka_crypto_alg);
}

static int __init pka_init(void)
{
	return ccc_register_channel_driver(THIS_MODULE, &pka_driver);
}
module_init(pka_init);

static void __exit pka_exit(void)
{
	ccc_delete_channel_driver(&pka_driver);
}
module_exit(pka_exit);

MODULE_DESCRIPTION("ST C3 Micro-Programmable RSA channel driver");
MODULE_ALIAS_CRYPTO("rsa");
MODULE_LICENSE("GPL");
