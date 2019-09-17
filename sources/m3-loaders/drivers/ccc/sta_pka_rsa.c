/**
 * @file sta_pka_rsa.c
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

#define PREFIX DEV_NAME ": rsa: "

#define MONTY_PAR_IN_SIZE(d) (sizeof((d)->monty_par_in->op_len) +	\
			      (d)->op_len_in_bytes)

#define COMMON_DATA_SIZE(d) (sizeof((d)->op_len_in_bytes) +		\
			     (d)->op_len_in_bytes +			\
			     sizeof((d)->exp_len_in_bytes) +		\
			     (d)->exp_len_in_bytes)

#define MONTY_EXP_SIZE(d) (COMMON_DATA_SIZE(d) +	\
			   (d)->op_len_in_bytes)

#define MOD_EXP_SIZE(d) (COMMON_DATA_SIZE(d) +				\
			 (d)->op_len_in_bytes +				\
			 (ODD((d)->op_len_in_bytes / sizeof(unsigned int)) ? \
			  sizeof(unsigned int) : 0) +			\
			 (d)->op_len_in_bytes)

#define RSA_SIZE_IN_WORDS MAX_RSA_SIZE_IN_WORDS

struct montgomery_parameter_input {
	unsigned int op_len;
	unsigned int mod[RSA_SIZE_IN_WORDS];
};

struct montgomery_parameter {
	/*
	 * Reserve one more 32bits word for Montgomery parameter in case of an
	 * odd number of words of modulus.
	 */
	unsigned int square_r[RSA_SIZE_IN_WORDS + 1];
};

/*
 * This structure is used to reserve memory. It can not be passed to channel
 * directly because exp_len field location is variable.
 */
struct montgomery_exponentiation {
	unsigned int op_len;
	unsigned int mod[RSA_SIZE_IN_WORDS];
	unsigned int exp_len;
	unsigned int exp[RSA_SIZE_IN_WORDS];
};

/*
 * This structure is used to reserve memory. It can not be passed to channel
 * directly because exp_len field location is variable.
 */
struct modular_exponentiation {
	unsigned int op_len;
	unsigned int mod[RSA_SIZE_IN_WORDS];
	unsigned int exp_len;
	unsigned int exp[RSA_SIZE_IN_WORDS];
	/*
	 * Reserve one more 32bits word for Montgomery parameter in case of an
	 * odd number of words of modulus.
	 */
	unsigned int square_r[RSA_SIZE_IN_WORDS + 1];
};

#define MAX_RSA_DATA_SIZE (sizeof(struct montgomery_parameter_input) +	\
			   sizeof(struct modular_exponentiation))
struct rsa_shared_data {
	struct montgomery_parameter_input *monty_par_in;
	struct montgomery_parameter *monty_par;
	struct montgomery_exponentiation *monty_exp;
	struct modular_exponentiation *mod_exp;
	/* Lengths copy to ease access. */
	unsigned int op_len_in_bytes, exp_len_in_bytes;
};

/*
 * Cryptographic materials memory region shared by RSA operations.
 */
struct rsa_shared_data rsa_shared_data;

static inline bool rsa_check_modulus_size(unsigned int size)
{
	if (size > MAX_RSA_MODULUS_SIZE)
		return false;
	/* A 32bits modulus is too small. */
	if (size <= sizeof(unsigned int))
		return false;
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(size))
		return false;
#endif
	return true;
}

static inline bool rsa_check_exponent_size(unsigned int size)
{
	if (size > MAX_RSA_EXPONENT_SIZE)
		return false;
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(size))
		return false;
#endif
	return true;
}

static int set_monty_par_in(struct pka_context *context,
			    struct ccc_crypto_req *req)
{
	struct rsa_shared_data *data = context->data;
	unsigned char *p;
	struct montgomery_parameter_input *monty_par_in;

	/*
	 * Map Mongomery parameter input at the PKA shared data beginning.
	 */
	if (sizeof(struct montgomery_parameter_input) > MAX_RSA_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	data->monty_par_in = (struct montgomery_parameter_input *)p;
	monty_par_in = data->monty_par_in;
	monty_par_in->op_len = get_dimension(req->asym.modulus,
					     req->asym.modulus_size);
	swap_bytes_if((unsigned char *)&(monty_par_in->op_len),
		      sizeof(unsigned int));
	memcpy(monty_par_in->mod, req->asym.modulus, req->asym.modulus_size);
	return 0;
}

static int set_monty_par(struct pka_context *context)
{
	struct rsa_shared_data *data = context->data;
	unsigned char *p;

	/*
	 * Output Montgomery parameter calculation in modular exponentiation
	 * input.
	 */
	p = get_pka_shared_blob(context);
	p += sizeof(struct montgomery_parameter_input);
	p += COMMON_DATA_SIZE(data);
	data->monty_par = (struct montgomery_parameter *)p;
	return 0;
}

static int set_mod_exp(struct pka_context *context,
		       struct ccc_crypto_req *req)
{
	struct rsa_shared_data *data = context->data;
	int ret;
	unsigned char *p;
	unsigned int bit_size;

	/*
	 * As modular exponentiation is used along with MONTY_PAR_IN, map its
	 * parameters just after the latest.
	 */
	if ((sizeof(struct montgomery_parameter_input) +
	     sizeof(struct modular_exponentiation)) > MAX_RSA_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	p += sizeof(struct montgomery_parameter_input);
	data->mod_exp = (struct modular_exponentiation *)p;
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

	ret = append_bit_size(&p, req->asym.exponent,
			      req->asym.exponent_size,
			      &bit_size);
	if (ret)
		return ret;
	data->exp_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, req->asym.exponent,
				  req->asym.exponent_size,
				  data->exp_len_in_bytes);
	if (ret)
		return ret;

	if (p != (unsigned char *)data->mod_exp + COMMON_DATA_SIZE(data))
		return -EFAULT;

	return 0;
}

static int set_monty_exp(struct pka_context *context,
			 struct ccc_crypto_req *req)
{
	struct rsa_shared_data *data = context->data;
	int ret;
	unsigned char *p;
	unsigned int bit_size;

	/*
	 * Map Mongomery multiplication parameters at the PKA shared data
	 * beginning.
	 */
	if (sizeof(struct montgomery_exponentiation) > MAX_RSA_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	data->monty_exp = (struct montgomery_exponentiation *)p;
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

	ret = append_bit_size(&p, req->asym.exponent, req->asym.exponent_size,
			      &bit_size);
	if (ret)
		return ret;
	data->exp_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, req->asym.exponent,
				  req->asym.exponent_size,
				  data->exp_len_in_bytes);
	if (ret)
		return ret;

	if (p != get_pka_shared_blob(context) + COMMON_DATA_SIZE(data))
		return -EFAULT;

	return 0;
}

void *rsa_crypto_init(struct ccc_crypto_req *req,
		      struct pka_context *context)
{
	struct rsa_shared_data *data;

	if (MAX_RSA_DATA_SIZE > MAX_PKA_DATA_SIZE) {
		TRACE_INFO(PREFIX "Shared data region too small for RSA\n");
		return NULL;
	}
	if (!req->asym.modulus) {
		TRACE_INFO(PREFIX "Modulus null pointer\n");
		return NULL;
	}
	if (!rsa_check_modulus_size(req->asym.modulus_size)) {
		TRACE_INFO(PREFIX "Bad modulus size\n");
		return NULL;
	}
	if (!*(unsigned int *)&(req->asym.modulus[0])) {
		TRACE_INFO(PREFIX "First 32bits word of modulus is zero\n");
		return NULL;
	}
	if (!get_dimension(req->asym.modulus, req->asym.modulus_size)) {
		TRACE_INFO(PREFIX "Modulus is zero\n");
		return NULL;
	}
	if (!req->asym.exponent) {
		TRACE_INFO(PREFIX "Exponent null pointer\n");
		return NULL;
	}
	if (!rsa_check_exponent_size(req->asym.exponent_size)) {
		TRACE_INFO(PREFIX "Bad exponent size\n");
		return NULL;
	}
	if (!*(unsigned int *)&(req->asym.exponent[0])) {
		TRACE_INFO(PREFIX "First 32bits word of exponent is zero\n");
		return NULL;
	}
	if (!get_dimension(req->asym.exponent, req->asym.exponent_size)) {
		TRACE_INFO(PREFIX "Exponent is zero\n");
		return NULL;
	}
	if (!is_size_multiple_of(&req->scatter, req->asym.modulus_size)) {
		TRACE_INFO(PREFIX "Data size not multiple of modulus size\n");
		return NULL;
	}
	if (!is_src_32bits_aligned(&req->scatter) ||
	    !is_dst_32bits_aligned(&req->scatter)) {
		TRACE_INFO(PREFIX "Data not aligned on 32bits\n");
		return NULL;
	}

	/* Update max chunk size constraint. */
	req->scatter.max_chunk_size = req->asym.modulus_size;
	context->nr_chunks = RSA_NR_CHUNKS;
	if (!ccc_prepare_chunks(&context->chunks[0], NULL, &req->scatter,
				&context->nr_chunks))
		return NULL;

	context->data = &rsa_shared_data;
	data = context->data;
	memset(data, '\0', sizeof(rsa_shared_data));

	if (context->nr_chunks > 1) {
		if (set_monty_par_in(context, req))
			return NULL;
		if (set_mod_exp(context, req))
			return NULL;
		if (set_monty_par(context))
			return NULL;
	} else
		if (set_monty_exp(context, req))
			return NULL;

	return context;
}

#define MOD_EXP 0x1a
#define MONTY_EXP 0x1b
static struct operation rsa_get_opcode(int index, unsigned char code,
				       unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case MOD_EXP:
		ASSERT(2 == nr_param);
		op.code = code << OP_ID_SHIFT;
		op.wn = 3;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << COUNTERMEASURE_SHIFT;
		op.code |= va_arg(params, unsigned int) << LENGTH_SHIFT;
		va_end(params);
		break;
	case MONTY_EXP:
		ASSERT(2 == nr_param);
		op.code = code << OP_ID_SHIFT;
		op.wn = 3;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << COUNTERMEASURE_SHIFT;
		op.code |= va_arg(params, unsigned int) << LENGTH_SHIFT;
		va_end(params);
		break;
	default:
		ASSERT(0);
		op.wn = 0;
		break;
	}
	op.code |= (index << CHN_SHIFT);

	return op;
}

void rsa_program_prologue(struct pka_context *context)
{
	if (context->nr_chunks <= 1)
		return;
	/*
	 * If there is more than one modular exponentiation to perform
	 * then compute Montgomery parameter once for all.
	 */
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;
	struct rsa_shared_data *data = context->data;

	op = get_opcode(channel->index, MONTY_PAR, 1,
			MONTY_PAR_IN_SIZE(data));
	ccc_program(dispatcher, op.code, op.wn,
		    data->monty_par_in,
		    data->monty_par);
}

void rsa_program(struct pka_context *context)

{
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;
	struct rsa_shared_data *data = context->data;
	struct ccc_chunk *chunk = &(context->chunks[0]);

	if (context->nr_chunks > 1) {
		unsigned int i;

		for (i = 0; i < context->nr_chunks; i++) {
			op = rsa_get_opcode(channel->index, MOD_EXP, 2,
					    get_countermeasure(context),
					    MOD_EXP_SIZE(data));
			ccc_program(dispatcher, op.code, op.wn,
				    data->mod_exp,
				    chunk->in.addr,
				    chunk->out.addr);
			chunk++;
		}
	} else {
		op = rsa_get_opcode(channel->index, MONTY_EXP, 2,
				    get_countermeasure(context),
				    MONTY_EXP_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->monty_exp,
			    chunk->in.addr,
			    chunk->out.addr);
	}
}

struct pka_alg pka_rsa_alg = {
	.crypto_init = rsa_crypto_init,
	.program_prologue = rsa_program_prologue,
	.program = rsa_program,
};
