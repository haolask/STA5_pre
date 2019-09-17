/**
 * @file sta_pka_ecc.c
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

#define PREFIX DEV_NAME ": ecc: "

#define MONTY_PAR_IN_SIZE(d) (sizeof((d)->monty_par_in->op_len) +	\
			      (d)->op_len_in_bytes)

#define ECC_MONTY_MUL_SIZE(d) (sizeof((d)->op_len_in_bytes) +	\
			       (d)->op_len_in_bytes +		\
			       sizeof(unsigned int) +		\
			       (d)->op_len_in_bytes +		\
			       2 * (d)->op_len_in_bytes +	\
			       sizeof((d)->k_len_in_bytes) +	\
			       (d)->k_len_in_bytes)

#define ECC_MUL_SIZE(d) (ECC_MONTY_MUL_SIZE(d) + \
			 (d)->op_len_in_bytes +	\
			 (ODD((d)->op_len_in_bytes / sizeof(unsigned int)) ? \
			  sizeof(unsigned int) : 0))

struct montgomery_parameter_input {
	unsigned int op_len;
	unsigned int mod[ECC_SIZE_IN_WORDS];
};

struct montgomery_parameter {
	/*
	 * Reserve one more 32bits word for Montgomery parameter in case of an
	 * odd number of words of modulus.
	 */
	unsigned int square_r[ECC_SIZE_IN_WORDS + 1];
};

struct ecc_multiplication {
	unsigned int op_len;
	unsigned int mod[ECC_SIZE_IN_WORDS];
	unsigned int a_sign;
	unsigned int a[ECC_SIZE_IN_WORDS];
	unsigned int p[2 * ECC_SIZE_IN_WORDS];
	unsigned int k_len;
	unsigned int k[ECC_SIZE_IN_WORDS];
	/*
	 * Reserve one more 32bits word for Montgomery parameter in case of an
	 * odd number of words of modulus.
	 */
	unsigned int square_r[ECC_SIZE_IN_WORDS + 1];
};

struct ecc_montgomery_multiplication {
	unsigned int op_len;
	unsigned int mod[ECC_SIZE_IN_WORDS];
	unsigned int a_sign;
	unsigned int a[ECC_SIZE_IN_WORDS];
	unsigned int p[2 * ECC_SIZE_IN_WORDS];
	unsigned int k_len;
	unsigned int k[ECC_SIZE_IN_WORDS];
};

#define ARITH_MULTIPLICATION_SIZE(d) (sizeof(unsigned int) +		\
				      (d)->op_len_in_bytes +		\
				      (d)->op_len_in_bytes)

#define MOD_REDUCTION_SIZE(d) (sizeof(unsigned int) +			\
			       (d)->op_len_in_bytes +			\
			       sizeof(unsigned int) +			\
			       (d)->op_len_in_bytes)

#define MOD_ADDITION_SIZE(d) (sizeof(unsigned int) +			\
			      (d)->op_len_in_bytes +			\
			      (d)->op_len_in_bytes +			\
			      (d)->op_len_in_bytes)

struct arith_multiplication {
	unsigned int op_len;
	unsigned int a[ECC_SIZE_IN_WORDS];
	unsigned int b[ECC_SIZE_IN_WORDS];
};

struct arith_multiplication_result {
	unsigned int c[2 * ECC_SIZE_IN_WORDS];
};

struct mod_reduction {
	unsigned int a_len;
	unsigned int a[2 * ECC_SIZE_IN_WORDS];
	unsigned int mod_len;
	unsigned int n[ECC_SIZE_IN_WORDS];
};

struct mod_reduction_result {
	unsigned int c[ECC_SIZE_IN_WORDS];
};

struct mod_addition {
	unsigned int op_len;
	unsigned int a[ECC_SIZE_IN_WORDS];
	unsigned int b[ECC_SIZE_IN_WORDS];
	unsigned int n[ECC_SIZE_IN_WORDS];
};

struct mod_addition_result {
	unsigned int c[ECC_SIZE_IN_WORDS];
};

#define MAX_ECC_DATA_SIZE (sizeof(struct montgomery_parameter_input) +	\
			   sizeof(struct ecc_multiplication[ECC_NR_CHUNKS]))
#define MAX_ARITH_DATA_SIZE (sizeof(struct arith_multiplication) +	\
			     sizeof(struct mod_reduction) +		\
			     sizeof(struct mod_addition))
struct ecc_shared_data {
	struct montgomery_parameter_input *monty_par_in;
	struct montgomery_parameter *monty_par;
	struct ecc_montgomery_multiplication *ecc_monty_mul;
	struct ecc_multiplication *ecc_mul[ECC_NR_CHUNKS];
	struct arith_multiplication *arith_mul;
	struct arith_multiplication_result *arith_mul_res;
	struct mod_reduction *mod_red;
	struct mod_reduction_result *mod_red_res;
	struct mod_addition *mod_add;
	struct mod_addition_result *mod_add_res;
	unsigned int arith_mul_res_msb;
	/* Lengths copy to ease access. */
	unsigned int op_len_in_bytes, k_len_in_bytes;
};

/*
 * Cryptographic materials memory region shared by ECC operations.
 */
struct ecc_shared_data ecc_shared_data;

static int set_monty_par_in(struct pka_context *context,
			    struct ccc_crypto_req *req)
{
	struct ecc_shared_data *data = context->data;
	unsigned char *p;
	struct montgomery_parameter_input *monty_par_in;

	/*
	 * Map Mongomery parameter input at the PKA shared data beginning.
	 */
	if (sizeof(struct montgomery_parameter_input) > MAX_ECC_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	data->monty_par_in = (struct montgomery_parameter_input *)p;
	monty_par_in = data->monty_par_in;
	monty_par_in->op_len = get_dimension(req->asym.modulus,
					     req->asym.modulus_size);
	swap_bytes_if((unsigned char *)&monty_par_in->op_len,
		      sizeof(unsigned int));
	memcpy(monty_par_in->mod, req->asym.modulus, req->asym.modulus_size);
	return 0;
}

static int set_monty_par(struct pka_context *context)
{
	struct ecc_shared_data *data = context->data;
	unsigned char *p;

	/*
	 * Output Montgomery parameter calculation in scalar multiplication
	 * input.
	 */
	p = get_pka_shared_blob(context);
	p += context->nr_chunks * ECC_MONTY_MUL_SIZE(data);
	data->monty_par = (struct montgomery_parameter *)p;
	return 0;
}

/*
 * If 'req' is not null and index is zero then fill ECC_MUL input structure at
 * index 0 as a template for further calls and leave point P coordinates
 * untouched.
 * If 'req' is null and index is not zero then copy ECC_MUL input structure at
 * index 0 into ECC_MUL input structure indexed by 'index' and copy point P
 * coordinates.
 */
static int set_ecc_mul(struct pka_context *context,
		       struct ccc_crypto_req *req,
		       unsigned int index,
		       unsigned char *point,
		       unsigned int coord_size)
{
	struct ecc_shared_data *data = context->data;
	int ret;
	unsigned char *p;

	if (!index && !req)
		return -EINVAL;
	/*
	 * As scalar multiplications are used along with MONTY_PAR_IN, map their
	 * parameters just after the latest.
	 */
	if ((sizeof(struct montgomery_parameter_input) +
	     ECC_NR_CHUNKS * sizeof(struct ecc_multiplication))
	    > MAX_ECC_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	p += sizeof(struct montgomery_parameter_input);
	p += index * sizeof(struct ecc_multiplication);
	data->ecc_mul[index] = (struct ecc_multiplication *)p;
	if (req) {
		struct curve *curve = &req->asym.curve;
		unsigned int bit_size;

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

		/* Skip base point P. */
		p += 2 * data->op_len_in_bytes;

		/* Append scalar k. */
		ret = append_bit_size(&p, req->asym.modulus,
				      req->asym.modulus_size,
				      &bit_size);
		if (ret)
			return ret;
		data->k_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
		ret = append_align_bignum(&p, req->asym.k,
					  req->asym.k_size,
					  data->k_len_in_bytes);
		if (ret)
			return ret;

		if (p != (unsigned char *)data->ecc_mul[index] +
		    ECC_MUL_SIZE(data))
			return -EFAULT;
	} else {
		struct ecc_multiplication *ecc_mul_template = data->ecc_mul[0];

		/* Copy template. */
		memcpy(p, (unsigned char *)ecc_mul_template,
		       sizeof(*ecc_mul_template));

		/* Move to base point P location. */
		p += data->op_len_in_bytes + sizeof(unsigned int) +
			data->op_len_in_bytes;

		/* Insert base point Px coordinate. */
		ret = append_align_bignum(&p, point,
					  coord_size,
					  data->op_len_in_bytes);
		if (ret)
			return ret;
		/* Insert base point Py coordinate. */
		ret = append_align_bignum(&p, point + coord_size,
					  coord_size,
					  data->op_len_in_bytes);
		if (ret)
			return ret;
	}
	return 0;
}

static int set_ecc_monty_mul(struct pka_context *context,
			     struct ccc_crypto_req *req,
			     unsigned char *point,
			     unsigned int coord_size)
{
	struct ecc_shared_data *data = context->data;
	int ret;
	unsigned char *p;
	unsigned int bit_size;
	struct curve *curve = &req->asym.curve;

	/*
	 * Map Mongomery multiplication parameters at the PKA shared data
	 * beginning.
	 */
	if (sizeof(struct ecc_montgomery_multiplication) > MAX_ECC_DATA_SIZE)
		return -ENOMEM;
	p = get_pka_shared_blob(context);
	data->ecc_monty_mul = (struct ecc_montgomery_multiplication *)p;
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

	/*
	 * 'a' parameter, Px and Py big numbers are meant to be as big
	 * as modulus.
	 * Append 'a' parameter.
	 */
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

	/* Append scalar k. */
	ret = append_bit_size(&p, req->asym.modulus,
			      req->asym.modulus_size,
			      &bit_size);
	if (ret)
		return ret;
	data->k_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	ret = append_align_bignum(&p, req->asym.k,
				  req->asym.k_size,
				  data->k_len_in_bytes);
	if (ret)
		return ret;

	if (p != get_pka_shared_blob(context) + ECC_MONTY_MUL_SIZE(data))
		return -EFAULT;

	return 0;
}

void *ecc_crypto_init(struct ccc_crypto_req *req,
		      struct pka_context *context)
{
	struct ecc_shared_data *data;
	struct curve *curve = &req->asym.curve;

	if (MAX_ECC_DATA_SIZE > MAX_PKA_DATA_SIZE) {
		TRACE_INFO(PREFIX "Shared data region too small for ECC\n");
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
	if (!req->asym.k) {
		TRACE_INFO(PREFIX "'k' parameter null pointer\n");
		return NULL;
	}
	if (!ecc_check_scalar_size(req->asym.k_size)) {
		TRACE_INFO(PREFIX "Bad 'k' len\n");
		return NULL;
	}
	if (!is_size_multiple_of(&req->scatter, 2)) {
		TRACE_INFO(PREFIX "Data size not divisible by two\n");
		return NULL;
	}
	/* Update max chunk size constraint. */
	req->scatter.max_chunk_size = 2 * ROUND_UP(req->asym.modulus_size,
						   sizeof(unsigned int));
	context->nr_chunks = ECC_NR_CHUNKS;
	if (!ccc_prepare_chunks(&context->chunks[0], NULL, &req->scatter,
				&context->nr_chunks))
		return NULL;

	context->data = &ecc_shared_data;
	data = context->data;
	memset(data, '\0', sizeof(ecc_shared_data));

	if (context->nr_chunks > 1) {
		if (set_monty_par_in(context, req))
			return NULL;
		/* Set ECC_MUL input data structure template. */
		if (set_ecc_mul(context, req, 0, NULL, 0))
			return NULL;
		if (set_monty_par(context))
			return NULL;
	} else {
		/* Set ECC_MONTY_MUL input data structure. */
		if (set_ecc_monty_mul(context, req, context->chunks[0].in.addr,
				      context->chunks[0].in.size / 2))
			return NULL;
	}

	return context;
}

static int set_arith_mul(struct pka_context *context,
			 struct ccc_crypto_req *req)
{
	int ret;
	unsigned char *p, *a, *b;
	unsigned int max_bit_size, a_size, a_bit_size, b_size, b_bit_size;
	struct arithmetic *arithmetic = &req->asym.arithmetic;
	struct ecc_shared_data *data = context->data;
	struct ccc_chunk *chunk = &context->chunks[0];

	a = arithmetic->alpha;
	a_size = arithmetic->alpha_size;
	b = arithmetic->beta;
	b_size = arithmetic->beta_size;
	if (arithmetic->arith_op == ARITH_MUL__MOD_RED__MOD_ADD) {
		struct mod_reduction *mod_red = data->mod_red;

		data->arith_mul_res =
			(struct arith_multiplication_result *)&mod_red->a[0];
	} else if (arithmetic->arith_op == ARITH_MUL) {
		data->arith_mul_res = chunk->out.addr;
	} else {
		return -EFAULT;
	}
	p = (unsigned char *)data->arith_mul;

	/* Append 'op_len'. */
	a_bit_size = get_dimension(a, a_size);
	b_bit_size = get_dimension(b, b_size);
	data->arith_mul_res_msb = a_bit_size + b_bit_size - 2;
	max_bit_size = MAX(a_bit_size, b_bit_size);
	if (a_bit_size == max_bit_size)
		ret = append_bit_size(&p, a, a_size, &max_bit_size);
	else
		ret = append_bit_size(&p, b, b_size, &max_bit_size);
	if (ret)
		return ret;
	data->op_len_in_bytes = size_rounded_to_word_in_bytes(max_bit_size);
	/* Append 'a' factor. */
	ret = append_align_bignum(&p, a, a_size, data->op_len_in_bytes);
	if (ret)
		return ret;
	/* Append 'b' factor. */
	ret = append_align_bignum(&p, b, b_size, data->op_len_in_bytes);
	if (ret)
		return ret;
	return 0;
}

static int set_mod_red(struct pka_context *context,
		       struct ccc_crypto_req *req)
{
	int ret;
	unsigned char *p, *a;
	unsigned int a_size, bit_size;
	struct arithmetic *arithmetic = &req->asym.arithmetic;
	struct ecc_shared_data *data = context->data;
	struct ccc_chunk *chunk = &context->chunks[0];

	if (arithmetic->arith_op == ARITH_MUL__MOD_RED__MOD_ADD) {
		struct mod_addition *mod_add = data->mod_add;
		unsigned int most_significant_byte;

		data->mod_red_res =
			(struct mod_reduction_result *)&mod_add->a[0];
		a = (unsigned char *)&data->arith_mul_res->c[0];
		a_size = 2 * data->op_len_in_bytes;
		/*
		 * Set the most significant bit of 'a' to fool the bit size
		 * calculation below.
		 */
		most_significant_byte = a_size - 1;
		most_significant_byte -= data->arith_mul_res_msb / 8;
		a[most_significant_byte] = 1 << data->arith_mul_res_msb % 8;
	} else if (arithmetic->arith_op == MOD_RED) {
		data->mod_red_res = chunk->out.addr;
		a = arithmetic->alpha;
		a_size = arithmetic->alpha_size;
	} else {
		return -EFAULT;
	}
	p = (unsigned char *)data->mod_red;

	/* Append 'a_len'. */
	ret = append_bit_size(&p, a, a_size, &bit_size);
	if (ret)
		return ret;
	/* Append 'a' input. */
	if (arithmetic->arith_op == MOD_RED) {
		data->op_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
		ret = append_align_bignum(&p, a, a_size, data->op_len_in_bytes);
		if (ret)
			return ret;
	} else {
		/* 'a' already set thus skip copy. */
		p += a_size;
	}
	/* Append 'mod_len'. */
	ret = append_bit_size(&p, req->asym.modulus, req->asym.modulus_size,
			      &bit_size);
	if (ret)
		return ret;
	data->op_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	/* Append 'n' modulus. */
	ret = append_align_bignum(&p, req->asym.modulus, req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;
	return 0;
}

static int set_mod_add(struct pka_context *context,
		       struct ccc_crypto_req *req)
{
	int ret;
	unsigned char *p, *a, *b;
	unsigned int bit_size;
	struct arithmetic *arithmetic = &req->asym.arithmetic;
	struct ecc_shared_data *data = context->data;
	struct ccc_chunk *chunk = &context->chunks[0];

	if (arithmetic->arith_op == ARITH_MUL__MOD_RED__MOD_ADD) {
		data->mod_add_res = chunk->out.addr;
		a = (unsigned char *)&data->mod_red_res->c[0];
		b = arithmetic->gamma;
	} else if (arithmetic->arith_op == MOD_ADD) {
		data->mod_add_res = chunk->out.addr;
		a = arithmetic->alpha;
		b = arithmetic->beta;
	} else {
		return -EFAULT;
	}
	p = (unsigned char *)data->mod_add;

	/* Append 'op_len'. */
	ret = append_bit_size(&p, req->asym.modulus, req->asym.modulus_size,
			      &bit_size);
	if (ret)
		return ret;
	data->op_len_in_bytes = size_rounded_to_word_in_bytes(bit_size);
	/* Append 'a' input. */
	if (arithmetic->arith_op == MOD_ADD) {
		ret = append_align_bignum(&p, a, req->asym.modulus_size,
					  data->op_len_in_bytes);
		if (ret)
			return ret;
	} else {
		/*
		 * TODO: Verify that 'a' is represented with 'W' bytes
		 * i.e. data->op_len_in_bytes bytes.
		 */
		p += data->op_len_in_bytes;
	}
	/* Append 'b' input. */
	ret = append_align_bignum(&p, b, req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;
	/* Append 'n' modulus. */
	ret = append_align_bignum(&p, req->asym.modulus, req->asym.modulus_size,
				  data->op_len_in_bytes);
	if (ret)
		return ret;
	return 0;
}

#define ARITH_OPCODE_SET 0x14
#define ARITH_OP_ID_SHIFT 17
struct operation arith_get_opcode(int index, unsigned char code,
				  unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case MOD_REDUCTION:
	case MOD_ADDITION:
	case ECDSA_SIGN:
	case ECDSA_VERIFY:
	case ARITH_MULTIPLICATION:
		ASSERT(nr_param == 2);
		op.code = ARITH_OPCODE_SET << OP_ID_SHIFT;
		op.code |= code << ARITH_OP_ID_SHIFT;
		break;
	case ECC_MUL:
	case ECC_MONTY_MUL:
		ASSERT(nr_param == 2);
		op.code = code << OP_ID_SHIFT;
		break;
	default:
		ASSERT(0);
		break;
	}
	op.wn = 2;
	va_start(params, nr_param);
	op.code |= va_arg(params, unsigned int) << COUNTERMEASURE_SHIFT;
	op.code |= va_arg(params, unsigned int) << LENGTH_SHIFT;
	va_end(params);
	op.code |= (index << CHN_SHIFT);
	return op;
}

void ecc_program_prologue(struct pka_context *context)
{
	if (context->nr_chunks <= 1)
		return;
	/*
	 * If there is more than one scalar multiplication to perform
	 * then compute Montgomery parameter once for all.
	 */
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;
	struct ecc_shared_data *data = context->data;

	op = get_opcode(channel->index, MONTY_PAR, 1,
			MONTY_PAR_IN_SIZE(data));
	ccc_program(dispatcher, op.code, op.wn,
		    data->monty_par_in,
		    data->monty_par);
}

void ecc_program(struct pka_context *context)
{
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;
	struct ecc_shared_data *data = context->data;
	struct ccc_chunk *chunk = &context->chunks[0];

	if (context->nr_chunks > 1) {
		unsigned int i;

		for (i = 0; i < context->nr_chunks; i++) {
			(void)set_ecc_mul(context, NULL, i, chunk->in.addr,
					  chunk->in.size / 2);
			op = arith_get_opcode(channel->index, ECC_MUL,
					      2,
					      get_countermeasure(context),
					      ECC_MUL_SIZE(data));
			ccc_program(dispatcher, op.code, op.wn,
				    data->ecc_mul,
				    chunk->out.addr);
			chunk++;
		}
	} else {
		op = arith_get_opcode(channel->index, ECC_MONTY_MUL, 2,
				      get_countermeasure(context),
				      ECC_MONTY_MUL_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->ecc_monty_mul,
			    chunk->out.addr);
	}
}

static int arithmetic_check_modulus(struct ccc_crypto_req *req)
{
	unsigned char *modulus = req->asym.modulus;
	unsigned int size = req->asym.modulus_size;

	if (!modulus) {
		TRACE_INFO(PREFIX "Modulus null pointer\n");
		return -EINVAL;
	}
	if (size > MAX_ECC_MODULUS_SIZE) {
		TRACE_INFO(PREFIX "Modulus too big\n");
		return -EINVAL;
	}
	if (size <= sizeof(unsigned int)) {
		TRACE_INFO(PREFIX "Modulus too small\n");
		return -EINVAL;
	}
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(size)) {
		TRACE_INFO(PREFIX "Modulus size not aligned\n");
		return -EINVAL;
	}
#endif
	if (!*(unsigned int *)modulus) {
		TRACE_INFO(PREFIX "First word of modulus is zero\n");
		return -EINVAL;
	}
	if (!get_dimension(modulus, size)) {
		TRACE_INFO(PREFIX "Modulus is zero\n");
		return -EINVAL;
	}
	return 0;
}

static int arithmetic_check_operand(unsigned char *operand, unsigned int size)
{
	if (!operand) {
		TRACE_INFO(PREFIX "Operand null pointer\n");
		return -EINVAL;
	}
	if (size > MAX_ECC_MODULUS_SIZE) {
		TRACE_INFO(PREFIX "Operand too big\n");
		return -EINVAL;
	}
	if (size <= sizeof(unsigned int)) {
		TRACE_INFO(PREFIX "Operand too small\n");
		return -EINVAL;
	}
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(size)) {
		TRACE_INFO(PREFIX "Operand size not aligned\n");
		return -EINVAL;
	}
#endif
	return 0;
}

static int arithmetic_check_factors(unsigned char *a, unsigned int a_size,
				    unsigned char *b, unsigned int b_size)
{
	if (arithmetic_check_operand(a, a_size) ||
	    arithmetic_check_operand(b, b_size)) {
		return -EINVAL;
	}
	if (!get_dimension(a, a_size) && !get_dimension(b, b_size)) {
		TRACE_INFO(PREFIX "All factors are zero\n");
		return -EINVAL;
	}
	return 0;
}

static int arithmetic_check_output(struct ccc_crypto_req *req)
{
	struct ccc_scatter *scatter = &req->scatter;
	struct arithmetic *arithmetic = &req->asym.arithmetic;
	unsigned int expected_size;

	if (!scatter) {
		TRACE_INFO(PREFIX "Scatter null pointer\n");
		return -EINVAL;
	}
	if (scatter->nr_entries != 1) {
		TRACE_INFO(PREFIX "Too many entries\n");
		return -EINVAL;
	}
	if (arithmetic->arith_op == ARITH_MUL)
		expected_size = 2 * arithmetic->alpha_size;
	else
		expected_size = req->asym.modulus_size;
	if (scatter->entries[0].size != expected_size) {
		TRACE_INFO(PREFIX "Unexpected scatter size\n");
		return -EINVAL;
	}
	if (scatter->entries[0].size <= sizeof(unsigned int)) {
		TRACE_INFO(PREFIX "Scatter too small\n");
		return -EINVAL;
	}
#ifdef REJECT_UNALIGNED_DATA
	if (!THIRTY_TWO_BITS_ALIGNED(scatter->entries[0].size)) {
		TRACE_INFO(PREFIX "Scatter size not aligned\n");
		return -EINVAL;
	}
#endif
	return 0;
}

void *arithmetic_crypto_init(struct ccc_crypto_req *req,
			     struct pka_context *context)
{
	struct ecc_shared_data *data;
	unsigned char *p;
	struct arithmetic *arithmetic = &req->asym.arithmetic;

	if (MAX_ARITH_DATA_SIZE > MAX_PKA_DATA_SIZE) {
		TRACE_INFO(PREFIX "Shared data region too small for ARITH\n");
		return NULL;
	}
	switch (arithmetic->arith_op) {
	case ARITH_MUL:
		if (arithmetic_check_factors(arithmetic->alpha,
					     arithmetic->alpha_size,
					     arithmetic->beta,
					     arithmetic->beta_size))
			return NULL;
		break;
	case MOD_RED:
		if (arithmetic_check_modulus(req))
			return NULL;
		if (arithmetic_check_operand(arithmetic->alpha,
					     arithmetic->alpha_size))
			return NULL;
		break;
	case MOD_ADD:
		if (arithmetic_check_modulus(req))
			return NULL;
		if (arithmetic_check_operand(arithmetic->alpha,
					     arithmetic->alpha_size))
			return NULL;
		if (arithmetic_check_operand(arithmetic->beta,
					     arithmetic->beta_size))
			return NULL;
		break;
	case ARITH_MUL__MOD_RED__MOD_ADD:
		if (arithmetic_check_modulus(req))
			return NULL;
		if (arithmetic_check_factors(arithmetic->alpha,
					     arithmetic->alpha_size,
					     arithmetic->beta,
					     arithmetic->beta_size))
			return NULL;
		if (arithmetic_check_operand(arithmetic->gamma,
					     arithmetic->gamma_size))
			return NULL;
		break;
	default:
		TRACE_INFO(PREFIX "Unhandled operation\n");
		return NULL;
	}
	if (arithmetic_check_output(req))
		return NULL;

	/* Update max chunk size constraint TODO: Check max value. */
	req->scatter.max_chunk_size = MAX_ECC_SIZE_IN_BYTES;
	context->nr_chunks = 1;
	if (!ccc_prepare_chunks(&context->chunks[0], NULL, &req->scatter,
				&context->nr_chunks))
		return NULL;

	context->data = &ecc_shared_data;
	data = context->data;
	memset(data, '\0', sizeof(ecc_shared_data));
	p = get_pka_shared_blob(context);

	switch (arithmetic->arith_op) {
	case ARITH_MUL:
		data->arith_mul = (struct arith_multiplication *)p;
		if (set_arith_mul(context, req))
			return NULL;
		break;
	case MOD_RED:
		data->mod_red = (struct mod_reduction *)p;
		if (set_mod_red(context, req))
			return NULL;
		break;
	case MOD_ADD:
		data->mod_add = (struct mod_addition *)p;
		if (set_mod_add(context, req))
			return NULL;
		break;
	case ARITH_MUL__MOD_RED__MOD_ADD:
		data->arith_mul = (struct arith_multiplication *)p;
		data->mod_red = (struct mod_reduction *)(data->arith_mul + 1);
		data->mod_add = (struct mod_addition *)(data->mod_red + 1);
		if (set_arith_mul(context, req))
			return NULL;
		if (set_mod_red(context, req))
			return NULL;
		if (set_mod_add(context, req))
			return NULL;
		break;
	}
	return context;
}

void arithmetic_program(struct pka_context *context)
{
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;
	struct ecc_shared_data *data = context->data;

	if (data->arith_mul) {
		op = arith_get_opcode(channel->index, ARITH_MULTIPLICATION, 2,
				      get_countermeasure(context),
				      ARITH_MULTIPLICATION_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->arith_mul,
			    data->arith_mul_res);
	}
	if (data->mod_red) {
		op = arith_get_opcode(channel->index, MOD_REDUCTION, 2,
				      get_countermeasure(context),
				      MOD_REDUCTION_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->mod_red,
			    data->mod_red_res);
	}
	if (data->mod_add) {
		op = arith_get_opcode(channel->index, MOD_ADDITION, 2,
				      get_countermeasure(context),
				      MOD_ADDITION_SIZE(data));
		ccc_program(dispatcher, op.code, op.wn,
			    data->mod_add,
			    data->mod_add_res);
	}
}

struct pka_alg pka_ecc_alg = {
	.crypto_init = ecc_crypto_init,
	.program_prologue = ecc_program_prologue,
	.program = ecc_program,
};

struct pka_alg pka_arithmetic_alg = {
	.crypto_init = arithmetic_crypto_init,
	.program = arithmetic_program,
};
