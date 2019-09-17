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
#ifndef _PKA_H_
#define _PKA_H_

#define UNKNOWN_ALG 0xffffffff
#define RSA_ALG 0

struct pka_data {
	unsigned int countermeasure;
	struct dma_pool *pool;
};

struct pka_context {
	struct crypto_tfm *tfm;
	struct akcipher_request *req;
	unsigned int alg;
	struct ccc_dma rsa_pub;
	struct ccc_dma rsa_prv;
	/* Modulus, public and private exponents dimensions in bytes */
	int n_size, e_size, d_size;
	struct ccc_chunk chunk;
	struct ccc_channel *channel;
	struct ccc_dispatcher *dispatcher;
	struct device *dma_owner;
};

static inline struct pka_data *get_pka_data(struct pka_context *context)
{
	return (struct pka_data *)ccc_get_channel_data(context->channel);
}

#define OP_ID_LSB 23
#define COUNTERMEASURE_LSB 21
#define LENGTH_LSB 0
#define MONTY_PAR 0x11
#define MOD_EXP 0x1a
#define MONTY_EXP 0x1b

#define NO_COUNTERMEASURE 0
#define AGAINST_SPA 1
#define AGAINST_DPA 2

#endif /* _PKA_H_ */
