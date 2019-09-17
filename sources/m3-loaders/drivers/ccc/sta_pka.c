/**
 * @file sta_pka.c
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

#define PREFIX DEV_NAME ": "

struct pka_shared_data pka_shared_data
__aligned(4) __attribute__((section(".c3_programs")));

static struct pka_context ctx = {{0}, };

struct pka_data {
	const int *ids;
	const unsigned int nr_id;
	const int countermeasure;
	const int ecdsa_no_fault_value;
};

/* List all ids compatible with this driver. */
static const int ids[] = {0x00006009, 0x0000600a, 0x0000600b, 0x0000600c};

/* List ids compatible with PKAv7.x but PKAv7.2 and properties. */
static const int pka_v7x_ids[] = {0x00006009, 0x0000600a, 0x0000600b};

static struct pka_data pka_v7x_data = {.ids = &pka_v7x_ids[0],
				       .nr_id = ARRAY_SIZE(pka_v7x_ids),
				       .countermeasure = AGAINST_DPA,
				       .ecdsa_no_fault_value = 0};

/* List ids compatible with PKAv7.2 and properties. */
static const int pka_v72_ids[] = {0x0000600c};

static struct pka_data pka_v72_data = {.ids = &pka_v72_ids[0],
				       .nr_id = ARRAY_SIZE(pka_v72_ids),
				       .countermeasure = AGAINST_SPA,
				       .ecdsa_no_fault_value = 0x0dd60000};

#define LOG2_8 3
#define LOG2_8_MASK (8 - 1)
inline unsigned int size_in_bytes(unsigned int size_in_bits)
{
	unsigned int nr = size_in_bits >> LOG2_8;
	unsigned int remainder = size_in_bits & LOG2_8_MASK;

	if (remainder)
		nr++;
	return nr;
}

#define LOG2_32 5
#define LOG2_32_MASK (32 - 1)
inline unsigned int size_rounded_to_word_in_bytes(unsigned int size_in_bits)
{
	unsigned int nr = size_in_bits >> LOG2_32;
	unsigned int remainder = size_in_bits & LOG2_32_MASK;

	if (remainder)
		nr++;
	return sizeof(unsigned int) * nr;
}

unsigned int get_dimension(unsigned char *bytes,
			   unsigned int nr_bytes)
{
	unsigned int i, nr;

	/* Assume that input number is big-endian. */
	for (i = 0; i < nr_bytes; i++) {
		nr = get_byte_dimension(*bytes);
		if (nr)
			return nr + (nr_bytes - (i + 1)) * 8;
		bytes++;
	}
	return 0;
}

int append_bit_size(unsigned char **buf, unsigned char *bignum,
		    unsigned int size, unsigned int *bit_size)
{
	if (!bit_size)
		return -EINVAL;
	*bit_size = get_dimension(bignum, size);
	memcpy(*buf, (unsigned char *)bit_size, sizeof(*bit_size));
	swap_bytes_if(*buf, sizeof(*bit_size));
	*buf += sizeof(*bit_size);
	return 0;
}

/*
 * Append to a big number another one padded with leading zeroes up to an
 * integer number of words.
 *
 * dst is the destination buffer
 * src is the big number to be appended including possible leading zeroes
 * src_size is the allocated size in bytes of the big number (possibly
 * including some leading zeros)
 * dst_size is the size of the number to be appended to the destination buffer
 */
inline int append_bignum(unsigned char **dst, unsigned char *src,
			 unsigned int src_size, unsigned int dst_size)
{
	unsigned int bit_size, bare_size, padding_size, nr_zeros;

	/*
	 * Compute the bare size of the number: its size without the leading
	 * zeros.
	 */
	bit_size = get_dimension(src, src_size);
	bare_size = size_in_bytes(bit_size);
	ASSERT(bare_size <= src_size);
	if (dst_size < bare_size)
		return -ERANGE;

	/* Pad on the left if dst_size > bare_size */
	padding_size = dst_size - bare_size;
	if (padding_size) {
		memset(*dst, '\0', padding_size);
		*dst += padding_size;
	}
	/* Copy bignum skipping possible leading zeros. */
	nr_zeros = src_size - bare_size;
	memcpy(*dst, src + nr_zeros, bare_size);
	*dst += bare_size;
	return 0;
}

int append_align_bignum(unsigned char **dst, unsigned char *src,
			unsigned int src_size, unsigned int dst_size)
{
	if (!THIRTY_TWO_BITS_ALIGNED(dst_size))
		return -EINVAL;
#ifdef REJECT_UNALIGNED_DATA
	ASSERT(THIRTY_TWO_BITS_ALIGNED(src_size));
#endif
	return append_bignum(dst, src, src_size, dst_size);
}

unsigned int get_countermeasure(void *arg)
{
	struct ccc_channel *channel = &((struct pka_context *)arg)->channel;
	struct pka_data *data = ccc_get_channel_data(channel);

	return data->countermeasure;
}

unsigned int get_ecdsa_no_fault_value(void *arg)
{
	struct ccc_channel *channel = &((struct pka_context *)arg)->channel;
	struct pka_data *data = ccc_get_channel_data(channel);

	return data->ecdsa_no_fault_value;
}

static struct ccc_channel *pka_get_channel(void *arg)
{
	if (arg)
		return &((struct pka_context *)arg)->channel;

	return NULL;
}

#define NONE PK_NONE_ALG
#define RSA PK_RSA_ALG
#define ECC PK_ECC_ALG
#define ECDSA PK_ECDSA_ALG
#define ARITHMETIC PK_ARITHMETIC_ALG
static struct pka_alg *check_alg(struct ccc_crypto_req *req)
{
	switch (req->asym.alg) {
	case RSA:
		return &pka_rsa_alg;
	case ECC:
		return &pka_ecc_alg;
	case ECDSA:
		return &pka_ecdsa_alg;
	case ARITHMETIC:
		return &pka_arithmetic_alg;
	default:
		TRACE_INFO(PREFIX "Unhandled algorithm\n");
		break;
	}

	return NULL;
}

static void *pka_crypto_init(struct ccc_crypto_req *req,
			     struct ccc_crypto_context *parent)
{
	struct pka_context *context = &ctx;

	context->shared_data = &pka_shared_data;
	memset(context->shared_data, '\0', sizeof(struct pka_shared_data));
	context->parent = parent;
	context->alg = check_alg(req);
	if (!context->alg)
		return NULL;
	if (req->scatter.nr_bytes == 0) {
		TRACE_INFO(PREFIX "No data to compute\n");
		return NULL;
	}

	struct pka_alg *alg = context->alg;

	if (!alg->crypto_init)
		return NULL;
	return alg->crypto_init(req, context);
}

struct operation get_opcode(int index, unsigned char code,
			    unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case MONTY_PAR:
		ASSERT(nr_param == 1);
		op.code = code << OP_ID_SHIFT;
		op.wn = 2;
		va_start(params, nr_param);
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

int pka_program_prologue(void *arg)
{
	struct pka_context *context = (struct pka_context *)arg;
	struct pka_alg *alg = context->alg;

	if (alg->program_prologue)
		alg->program_prologue(context);

	return 0;

}

int pka_program(void *arg)
{
	struct pka_context *context = (struct pka_context *)arg;
	struct pka_alg *alg = context->alg;

	if (!alg->program)
		return -EFAULT;

	alg->program(context);

	return 0;
}

int pka_program_epilogue(void *arg)
{
	struct pka_context *context = (struct pka_context *)arg;
	struct pka_alg *alg = context->alg;

	if (alg->program_epilogue)
		alg->program_epilogue(context);

	return 0;
}

int pka_post_processing(void *arg)
{
	struct pka_context *context = (struct pka_context *)arg;
	struct pka_alg *alg = context->alg;

	if (alg->post_process)
		return alg->post_process(context);

	return 0;
}

static inline struct pka_data *pka_get_channel_data(int id)
{
	int i;

	for (i = pka_v7x_data.nr_id; i--;) {
		if (id == pka_v7x_data.ids[i])
			return &pka_v7x_data;
	}
	for (i = pka_v72_data.nr_id; i--;) {
		if (id == pka_v72_data.ids[i])
			return &pka_v72_data;
	}
	return NULL;
}

static struct channel_methods pka_methods = {
	.get_channel = pka_get_channel,
	.crypto_init = pka_crypto_init,
	.program_prologue = pka_program_prologue,
	.program = pka_program,
	.program_epilogue = pka_program_epilogue,
	.post_processing = pka_post_processing
};

int pka_init(struct ccc_controller *c3, int index,
	     struct channel_methods **methods)
{
	struct ccc_channel *channel = &ctx.channel;
#ifdef DEBUG
	unsigned int scr;
#endif

	channel->controller = c3;
	channel->index = index;
	channel->base = ccc_get_ch_physbase(c3, channel->index);

	channel->id = ccc_get_ch_id(channel->base);
	if (channel->id == 0) {
		TRACE_ERR(PREFIX "Unused channel\n");
		return -ENODEV;
	}
	TRACE_INFO(PREFIX "Channel identifier: %08x\n", channel->id);
	/*
	 * Driving an unknown version of the channel may lead to an
	 * unpredictable behaviour.
	 */
	if (!ccc_is_channel_supported(channel->id, ids, ARRAY_SIZE(ids)))
		TRACE_ERR(PREFIX "Bad channel identifier: %x\n", channel->id);

	if (!ccc_is_channel_present(channel)) {
		TRACE_ERR(PREFIX
			"Channel %d does not exist\n", channel->index);
		return -EINVAL;
	}

	channel->error_mask = BERR | DERR | PERR | IERR | AERR | OERR;
	ccc_set_channel_name(channel, DEV_NAME);
	ccc_set_channel_data(channel, pka_get_channel_data(channel->id));
	*methods = &pka_methods;
#ifdef DEBUG
	scr = ccc_read_channel_scr(channel);
	TRACE_INFO(PREFIX "Endianness: %s\n",
		   ccc_get_channel_scr_endian(scr) ?
		   "little/swap" : "big/no swap");
#endif
	return 0;
}
