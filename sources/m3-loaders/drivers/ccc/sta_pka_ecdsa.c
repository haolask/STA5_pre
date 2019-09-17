/**
 * @file sta_pka_ecdsa.c
 * @brief CCC PKA channel driver.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "sta_ccc_plat.h"
#include "sta_ccc_osal.h"
#include "sta_ccc_if.h"
#include "sta_ccc.h"
#include "sta_pka.h"

#define PREFIX DEV_NAME ": ecdsa: "

#define ECDSA_SIGN_SIZE(d) (sizeof((d)->op_len_in_bytes) +	\
			    (d)->op_len_in_bytes +		\
			    sizeof(unsigned int) +		\
			    (d)->op_len_in_bytes +		\
			    2 * (d)->op_len_in_bytes +		\
			    sizeof((d)->n_len_in_bytes) +	\
			    (d)->n_len_in_bytes +		\
			    (d)->n_len_in_bytes +		\
			    (d)->n_len_in_bytes +		\
			    (d)->n_len_in_bytes)

#define ECDSA_SIGNATURE_R_SIZE(d) ((d)->n_len_in_bytes)
#define ECDSA_SIGNATURE_S_SIZE(d) ((d)->n_len_in_bytes)
#define ECDSA_SIGNATURE_SIZE(d) (ECDSA_SIGNATURE_R_SIZE(d) +	\
				 ECDSA_SIGNATURE_S_SIZE(d))

#define ECDSA_VERIFY_SIZE(d) (sizeof((d)->op_len_in_bytes) +	\
			      (d)->op_len_in_bytes +		\
			      sizeof(unsigned int) +		\
			      (d)->op_len_in_bytes +		\
			      2 * (d)->op_len_in_bytes +	\
			      2 * (d)->op_len_in_bytes +	\
			      sizeof((d)->n_len_in_bytes) +	\
			      (d)->n_len_in_bytes +		\
			      (d)->n_len_in_bytes +		\
			      (d)->n_len_in_bytes +		\
			      (d)->n_len_in_bytes)

struct ecdsa_sign {
	unsigned int op_len;
	unsigned int mod[ECC_SIZE_IN_WORDS];
	unsigned int a_sign;
	unsigned int a[ECC_SIZE_IN_WORDS];
	unsigned int p[2 * ECC_SIZE_IN_WORDS];
	unsigned int n_len;
	unsigned int n[ECC_SIZE_IN_WORDS];
	unsigned int d[ECC_SIZE_IN_WORDS];
	unsigned int k[ECC_SIZE_IN_WORDS];
	unsigned int e[ECC_SIZE_IN_WORDS];
};

struct ecdsa_signature {
	unsigned int fault;
	unsigned int r[ECC_SIZE_IN_WORDS];
	unsigned int s[ECC_SIZE_IN_WORDS];
};

struct ecdsa_verify {
	unsigned int op_len;
	unsigned int mod[ECC_SIZE_IN_WORDS];
	unsigned int a_sign;
	unsigned int a[ECC_SIZE_IN_WORDS];
	unsigned int p[2 * ECC_SIZE_IN_WORDS];
	unsigned int q[2 * ECC_SIZE_IN_WORDS];
	unsigned int n_len;
	unsigned int n[ECC_SIZE_IN_WORDS];
	unsigned int r[ECC_SIZE_IN_WORDS];
	unsigned int s[ECC_SIZE_IN_WORDS];
	unsigned int e[ECC_SIZE_IN_WORDS];
};

struct ecdsa_verification {
	unsigned int fault;
};

#define MAX_ECDSA_DATA_SIZE (MAX(sizeof(struct ecdsa_sign) +		\
				 sizeof(struct ecdsa_signature),	\
				 sizeof(struct ecdsa_verify) +		\
				 sizeof(struct ecdsa_verification)))
struct ecdsa_shared_data {
	bool is_generation;
	struct ecdsa_sign *ecdsa_sign;
	struct ecdsa_verify *ecdsa_verify;
	unsigned char *output;
	unsigned int op_len_in_bytes, n_len_in_bytes;
};

/*
 * Cryptographic materials memory region shared by ECDSA operations.
 */
struct ecdsa_shared_data ecdsa_shared_data;

static int set_ecdsa_sign(struct pka_context *context,
			  struct ccc_crypto_req *req,
			  unsigned char *point,
			  unsigned int coord_size)
{
	struct ecdsa_shared_data *data = context->data;
	int ret;
	unsigned char *p;
	unsigned int bit_size;

	if (!req)
		return -EINVAL;

	/*
	 * Map signature generation parameters at the PKA shared data beginning
	 * and output just after.
	 */
	if ((sizeof(struct ecdsa_sign) + sizeof(struct ecdsa_signature))
	    > MAX_ECDSA_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	data->ecdsa_sign = (struct ecdsa_sign *)p;
	data->output = p + sizeof(struct ecdsa_sign);

	struct curve *curve = &req->asym.curve;

	/* Append modulus. */
	ret = append_bit_size(&p, req->asym.modulus,
			      req->asym.modulus_size,
			      &bit_size);
	if (ret)
		return ret;
	data->op_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, req->asym.modulus,
				  req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append 'a' parameter sign. */
	ret = append_align_bignum(&p, (unsigned char *)&curve->a_sign,
				  sizeof(curve->a_sign),
				  sizeof(unsigned int));
	if (ret)
		return ret;

	/* Append 'a' parameter. */
	ret = append_align_bignum(&p, curve->a,
				  req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append point Px coordinate. */
	ret = append_align_bignum(&p, point,
				  coord_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;
	/* Append point Py coordinate. */
	ret = append_align_bignum(&p, point + coord_size,
				  coord_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append order n. */
	ret = append_bit_size(&p, curve->n,
			      curve->n_size,
			      &bit_size);
	if (ret)
		return ret;
	data->n_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, curve->n,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	/* Append secret key d. */
	ret = append_align_bignum(&p, curve->d,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	/* Append random k. */
	ret = append_align_bignum(&p, curve->k,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	/* Append hash e. */
	ret = append_align_bignum(&p, curve->e,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	if (p != (unsigned char *)data->ecdsa_sign + ECDSA_SIGN_SIZE(data))
		return -EFAULT;

	return 0;
}

static int set_ecdsa_verify(struct pka_context *context,
			    struct ccc_crypto_req *req,
			    unsigned char *point,
			    unsigned int coord_size)
{
	struct ecdsa_shared_data *data = context->data;
	int ret;
	unsigned char *p;
	unsigned int bit_size;

	if (!req)
		return -EINVAL;
	/*
	 * Map signature verification parameters at the PKA shared data
	 * beginning and output just after.
	 */
	if ((sizeof(struct ecdsa_verify) + sizeof(struct ecdsa_verification))
	    > MAX_ECDSA_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	data->ecdsa_verify = (struct ecdsa_verify *)p;
	data->output = p + sizeof(struct ecdsa_verify);

	struct curve *curve = &req->asym.curve;

	/* Append modulus. */
	ret = append_bit_size(&p, req->asym.modulus,
			      req->asym.modulus_size,
			      &bit_size);
	if (ret)
		return ret;
	data->op_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, req->asym.modulus,
				  req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append 'a' parameter sign. */
	ret = append_align_bignum(&p, (unsigned char *)&curve->a_sign,
				  sizeof(curve->a_sign),
				  sizeof(unsigned int));
	if (ret)
		return ret;

	/* Append 'a' parameter. */
	ret = append_align_bignum(&p, curve->a,
				  req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append base point Px coordinate. */
	ret = append_align_bignum(&p, point,
				  coord_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;
	/* Append base point Py coordinate. */
	ret = append_align_bignum(&p, point + coord_size,
				  coord_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append public key point Qx coordinate. */
	ret = append_align_bignum(&p, curve->q,
				  req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;
	/* Append public key point Qy coordinate. */
	ret = append_align_bignum(&p, curve->q + req->asym.modulus_size,
				  req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;

	/* Append order n. */
	ret = append_bit_size(&p, curve->n,
			      curve->n_size,
			      &bit_size);
	if (ret)
		return ret;
	data->n_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, curve->n,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	/* Append first part of the signature to be verified r. */
	ret = append_align_bignum(&p, curve->r,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	/* Append second part of the signature to be verified s. */
	ret = append_align_bignum(&p, curve->s,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	/* Append hash e. */
	ret = append_align_bignum(&p, curve->e,
				  curve->n_size,
				  data->n_len_in_bytes);
	if (ret)
		return ret;

	if (p != (unsigned char *)data->ecdsa_verify + ECDSA_VERIFY_SIZE(data))
		return -EFAULT;

	return 0;
}

void *ecdsa_crypto_init(struct ccc_crypto_req *req,
			struct pka_context *context)
{
	struct ecdsa_shared_data *data;
	struct curve *curve = &req->asym.curve;

	if (MAX_ECDSA_DATA_SIZE > MAX_PKA_DATA_SIZE) {
		TRACE_INFO(PREFIX "Shared data region too small for ECDSA\n");
		return NULL;
	}
	if (!req->asym.modulus) {
		TRACE_INFO(PREFIX "Modulus null pointer\n");
		return NULL;
	}
	if (!ecc_check_modulus_size(req->asym.modulus_size)) {
		TRACE_INFO(PREFIX "Bad modulus size\n");
		return NULL;
	}
	if (!*(unsigned int *)&req->asym.modulus[0]) {
		TRACE_INFO(PREFIX "First 32bits word of ECC modulus is zero\n");
		return NULL;
	}
	if (!get_dimension(req->asym.modulus, req->asym.modulus_size)) {
		TRACE_INFO(PREFIX "Modulus is zero\n");
		return NULL;
	}
	if (!curve->a) {
		TRACE_INFO(PREFIX "'a' parameter null pointer\n");
		return NULL;
	}
	if (!curve->n) {
		TRACE_INFO(PREFIX "'n' parameter null pointer\n");
		return NULL;
	}
	if (!ecc_check_scalar_size(curve->n_size)) {
		TRACE_INFO(PREFIX "Bad 'n' len\n");
		return NULL;
	}
	if (!is_size_multiple_of(&req->scatter, 2)) {
		TRACE_INFO(PREFIX "Data size not divisible by two\n");
		return NULL;
	}
	/* Update max chunk size constraint. */
	req->scatter.max_chunk_size = 2 * ROUND_UP(req->asym.modulus_size,
						   sizeof(unsigned int));
	/* Multiple chunks handling not supported. */
#define ECDSA_NR_CHUNKS 1
	context->nr_chunks = ECDSA_NR_CHUNKS;
	if (!ccc_prepare_chunks(&context->chunks[0], NULL, &req->scatter,
				&context->nr_chunks))
		return NULL;

	context->data = &ecdsa_shared_data;
	data = context->data;
	memset(data, '\0', sizeof(ecdsa_shared_data));

	data->is_generation = (curve->d != NULL);
	if (data->is_generation) {
		if (!curve->k) {
			TRACE_INFO(PREFIX "'k' parameter null pointer\n");
			return NULL;
		}
		if (set_ecdsa_sign(context, req, context->chunks[0].in.addr,
				   context->chunks[0].in.size / 2))
			return NULL;
	} else {
		if (!curve->q) {
			TRACE_INFO(PREFIX "'q' parameter null pointer\n");
			return NULL;
		}
		if (!curve->r) {
			TRACE_INFO(PREFIX "'r' parameter null pointer\n");
			return NULL;
		}
		if (!curve->s) {
			TRACE_INFO(PREFIX "'s' parameter null pointer\n");
			return NULL;
		}
		if (set_ecdsa_verify(context, req, context->chunks[0].in.addr,
				     context->chunks[0].in.size / 2))
			return NULL;
	}

	return context;
}

void ecdsa_program(struct pka_context *context)
{
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;
	struct ecdsa_shared_data *data = context->data;

	if (data->is_generation) {
		op = arith_get_opcode(channel->index, ECDSA_SIGN, 2,
				      get_countermeasure(context),
				      ECDSA_SIGN_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->ecdsa_sign,
			    data->output);
	} else {
		op = arith_get_opcode(channel->index, ECDSA_VERIFY, 2,
				      get_countermeasure(context),
				      ECDSA_VERIFY_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->ecdsa_verify,
			    data->output);
	}
}

int ecdsa_post_process(struct pka_context *context)
{
	struct ecdsa_shared_data *data = context->data;
	unsigned char *hw_output = data->output;
	unsigned int no_fault = get_ecdsa_no_fault_value(context);

	if (data->is_generation) {
		struct ccc_chunk *chunk = &context->chunks[0];
		struct ecdsa_signature *signature =
			(struct ecdsa_signature *)hw_output;
		unsigned char *sw_output = (unsigned char *)chunk->out.addr;
		unsigned int sw_output_size = chunk->out.size / 2;
		int ret;

		/* Give signature only in case of success. */
		if (signature->fault != no_fault) {
			TRACE_INFO(PREFIX "Fault reported: %08x\n",
				   signature->fault);
			return -EFAULT;
		}
		/* Skip error fault value before copy. */
		hw_output += sizeof(signature->fault);
		ret = append_bignum(&sw_output, hw_output,
				    ECDSA_SIGNATURE_R_SIZE(data),
				    sw_output_size);
		if (ret)
			return ret;
		hw_output += ECDSA_SIGNATURE_R_SIZE(data);
		ret = append_bignum(&sw_output, hw_output,
				    ECDSA_SIGNATURE_S_SIZE(data),
				    sw_output_size);
		if (ret)
			return ret;
	} else {
		struct ecdsa_verification *verification =
			(struct ecdsa_verification *)hw_output;

		if (verification->fault != no_fault) {
			TRACE_INFO(PREFIX "Fault reported: %08x\n",
				   verification->fault);
			return -EFAULT;
		}
	}
	return 0;
}

struct pka_alg pka_ecdsa_alg = {
	.crypto_init = ecdsa_crypto_init,
	.program = ecdsa_program,
	.post_process = ecdsa_post_process
};
