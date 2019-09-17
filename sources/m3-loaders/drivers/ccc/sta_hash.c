/**
 * @file sta_hash.c
 * @brief CCC HASH channel driver.
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

#define DEV_NAME "hash"
#define PREFIX DEV_NAME ": "

unsigned char _carried_data[NR_HASH_CRYPTO_CONTEXTS][sizeof(int)]
__attribute__((section(".c3_programs")));

#define UH_HW_CONTEXT_SIZE 152
#define UH_HW_CONTEXTS_OFFSET 0
#define UH_HW_CONTEXTS_SIZE (UH_HW_CONTEXT_SIZE *   \
			     NR_UH_CRYPTO_CONTEXTS * \
			     NR_UH_CHANNELS)
#define UH2_HW_CONTEXT_SIZE 296
#define UH2_HW_CONTEXTS_OFFSET UH_HW_CONTEXTS_SIZE
#define UH2_HW_CONTEXTS_SIZE (UH2_HW_CONTEXT_SIZE *   \
			      NR_UH2_CRYPTO_CONTEXTS * \
			      NR_UH2_CHANNELS)

unsigned char _hw_contexts[UH_HW_CONTEXTS_SIZE + UH2_HW_CONTEXTS_SIZE]
__attribute__((section(".c3_programs")));

struct hash_crypto_context {
	struct hash_context *hash_context;
	struct ccc_crypto_context *parent;
	unsigned int alg;
	struct ccc_chunk chunks[HASH_NR_CHUNKS];
	unsigned int nr_chunks;
	unsigned char pending_data[sizeof(int)];
	unsigned int nr_pending_data;
	struct ccc_dma carried_data;
	unsigned int nr_computed_data;
	unsigned char *digest;
	struct ccc_dma hw_context;
};

struct hash_context {
	bool inited;
	struct ccc_channel channel;
	semaphore_t semaphore;
	struct hash_crypto_context
	crypto_contexts[NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL];
};

static struct hash_context ctx[NR_HASH_CHANNELS] = {{0,} };

struct hash_data {
	const unsigned int version;
	const char *name;
	const int *ids;
	const unsigned int nr_id;
	const int *algs;
	const unsigned int nr_alg;
	const bool digest_truncation;
	const struct ccc_dma hw_contexts;
	void (*hash_program_append)(struct hash_crypto_context *,
				    struct ccc_dma *);
};

/* List all ids compatible with this driver. */
static const int ids[] = {0x00004016, 0x00004017, 0x00011004};

/* List ids compatible with and algs supported by UH driver. */
static const int uh_ids[] = {0x00004016, 0x00004017};
static const int uh_algs[] = {HASH_MD5_ALG, HASH_SHA_1_ALG, HASH_SHA_256_ALG,
			      HASH_SHA_224_ALG};

static inline void uh_program_append(struct hash_crypto_context *context,
				     struct ccc_dma *in);

static struct hash_data uh_data = {.version = UH_VERSION,
				   .name = "uh",
				   .ids = &uh_ids[0],
				   .nr_id = ARRAY_SIZE(uh_ids),
				   .algs = &uh_algs[0],
				   .nr_alg = ARRAY_SIZE(uh_algs),
				   .digest_truncation = true,
				   .hw_contexts = {
		&_hw_contexts[UH_HW_CONTEXTS_OFFSET],
		UH_HW_CONTEXT_SIZE},
				   .hash_program_append = uh_program_append};

/* List ids compatible with and algs supported by UH2 driver. */
static const int uh2_ids[] = {0x00011004};
static const int uh2_algs[] = {HASH_SHA_384_ALG, HASH_SHA_512_ALG};

static inline void uh2_program_append(struct hash_crypto_context *context,
				      struct ccc_dma *in);

static struct hash_data uh2_data = {.version = UH2_VERSION,
				    .name = "uh2",
				    .ids = &uh2_ids[0],
				    .nr_id = ARRAY_SIZE(uh2_ids),
				    .algs = &uh2_algs[0],
				    .nr_alg = ARRAY_SIZE(uh2_algs),
				    .digest_truncation = false,
				    .hw_contexts = {
		&_hw_contexts[UH2_HW_CONTEXTS_OFFSET],
		UH2_HW_CONTEXT_SIZE},
				    .hash_program_append = uh2_program_append};

static struct ccc_channel *hash_get_channel(void *arg)
{
	struct hash_crypto_context *context = (struct hash_crypto_context *)arg;

	if (context)
		if (context->hash_context)
			return &context->hash_context->channel;
	return NULL;
}

bool hash_is_alg_supported(unsigned int alg)
{
	switch (alg) {
	case HASH_MD5_ALG:
	case HASH_SHA_1_ALG:
	case HASH_SHA_256_ALG:
	case HASH_SHA_224_ALG:
	case HASH_SHA_384_ALG:
	case HASH_SHA_512_ALG:
		break;
	default:
		return false;
		break;
	}
	return true;
}

#define TO_ALG_BITS(a) (HASH_ALG_SHIFTED_MASK & (a))
static inline int get_alg_code(struct ccc_crypto_req *req)
{
	if (hash_is_alg_supported(req->hash.alg))
		return TO_ALG_BITS(req->hash.alg);
	ASSERT(0);

	/* Can't be reached. */
	return HASH_NONE_ALG;
}

static inline bool is_alg_supported(int alg, struct hash_data *hash_data)
{
	unsigned int i;

	if (!hash_data)
		return false;
	for (i = hash_data->nr_alg; i--;)
		if (alg == hash_data->algs[i])
			return true;
	return false;
}

static inline struct hash_context *get_context_from_alg(int alg)
{
	unsigned int i = ARRAY_SIZE(ctx) - 1;
	struct hash_data *data;

	do {
		if (ctx[i].inited) {
			data = ccc_get_channel_data(&ctx[i].channel);
			if (is_alg_supported(alg, data))
				return &ctx[i];
		}
	} while (i--);
	return NULL;
}

static struct hash_crypto_context *get_crypto_context(
	struct hash_context *hash_context,
	struct ccc_crypto_context *parent)
{
	struct hash_crypto_context *context = &hash_context->crypto_contexts[0];
	struct hash_crypto_context *found = NULL;
	unsigned int i = NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL;

	take_sem(&hash_context->semaphore);
	do {
		if (!context->parent)
			found = context;
		context++;
	} while (--i && !found);
	if (found)
		found->parent = parent;
	give_sem(&hash_context->semaphore);
	return found;
}

static void *hash_crypto_init(struct ccc_crypto_req *req,
			      struct ccc_crypto_context *parent)
{
	struct hash_context *hash_context = get_context_from_alg(req->hash.alg);
	struct hash_crypto_context *context;

	if (!hash_context) {
		TRACE_INFO(PREFIX "Hash algorithm is not supported\n");
		return NULL;
	}
	/* Assume that zero address is not valid on the platform. */
	if (NULL == req->hash.digest) {
		TRACE_INFO(PREFIX "Hash pointer is likely not set\n");
		return NULL;
	}
	context = get_crypto_context(hash_context, parent);
	if (!context) {
		TRACE_INFO(PREFIX "Hash is busy\n");
		return NULL;
	}
	context->alg = get_alg_code(req);
	context->digest = req->hash.digest;
	/*
	 * Multi-block mode is set if data are not already present in the
	 * request. Data will be set by one or more further calls to 'append'
	 * API.
	 */
	if (req->scatter.nr_bytes)
		ccc_set_scatter(parent, &req->scatter);
	else
		ccc_set_multi_block_mode(parent);
	return context;
}

#define OP_ID_SHIFT 20
#define INIT 0
#define ALG_SHIFT 23
#define INIT_CUSTOM_IV 0x41
#define APPEND 0x42
#define LENGTH_SHIFT 0
#define END 0x44
#define SAVE 0x46
#define RESTORE 0x47
#define T_SHIFT 20
static struct operation hash_get_opcode(int index, unsigned char code,
					unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case INIT:
		ASSERT(nr_param == 1);
		op.code = code << OP_ID_SHIFT;
		op.wn = 0;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << ALG_SHIFT;
		va_end(params);
		break;
	case APPEND:
		ASSERT(nr_param == 2);
		op.code = code << OP_ID_SHIFT;
		op.wn = 1;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << ALG_SHIFT;
		op.code |= va_arg(params, unsigned int) << LENGTH_SHIFT;
		va_end(params);
		break;
	case END:
		ASSERT(nr_param == 2);
		op.code = code << OP_ID_SHIFT;
		op.wn = 1;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << ALG_SHIFT;
		op.code |= va_arg(params, unsigned int) << T_SHIFT;
		va_end(params);
		break;
	case SAVE:
		ASSERT(nr_param == 0);
		op.code = code << OP_ID_SHIFT;
		op.wn = 1;
		break;
	case RESTORE:
		ASSERT(nr_param == 0);
		op.code = code << OP_ID_SHIFT;
		op.wn = 1;
		break;
	default:
		ASSERT(0);
		op.wn = 0;
		break;
	}
	op.code |= (index << CHN_SHIFT);

	return op;
}

static void put_crypto_context(struct hash_crypto_context *context)
{
	struct hash_context *hash_context = context->hash_context;

	take_sem(&hash_context->semaphore);
	context->parent = NULL;
	give_sem(&hash_context->semaphore);
}

static void program_hardware_context_save(struct hash_crypto_context *context)
{
	struct ccc_dma *hw_context = &context->hw_context;
	struct ccc_channel *channel = &context->hash_context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	op = hash_get_opcode(channel->index, SAVE, 0);
	ccc_program(dispatcher, op.code, op.wn, hw_context->addr);
}

int hash_program_prologue(void *arg)
{
	struct hash_crypto_context *context = (struct hash_crypto_context *)arg;
	struct ccc_channel *channel = &context->hash_context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	op = hash_get_opcode(channel->index, INIT, 1,
			     context->alg);
	ccc_program(dispatcher, op.code, op.wn);
	/* If requested, custom IV programmation should be inserted here. */
	if (ccc_is_multi_block_mode(context->parent))
		program_hardware_context_save(context);
	return 0;
}

static void flush_and_program_pending_data(struct hash_crypto_context *context)
{
	struct ccc_dma *carried_data = &context->carried_data;
	unsigned char *pending_data = &context->pending_data[0];
	unsigned int *nr_pending_data = &context->nr_pending_data;
	struct ccc_channel *channel = &context->hash_context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	carried_data->size = *nr_pending_data;
	memcpy(carried_data->addr, pending_data, carried_data->size);
	op = hash_get_opcode(channel->index, APPEND, 2,
			     context->alg,
			     carried_data->size);
	ccc_program(dispatcher, op.code, op.wn, carried_data->addr);
	*nr_pending_data = 0;
}

static void program_hardware_context_restore(
	struct hash_crypto_context *context)
{
	struct ccc_dma *hw_context = &context->hw_context;
	struct ccc_channel *channel = &context->hash_context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	op = hash_get_opcode(channel->index, RESTORE, 0);
	ccc_program(dispatcher, op.code, op.wn, hw_context->addr);
}

static inline int program_computing_in_multi_block_mode(
	struct hash_crypto_context *context)
{
	struct ccc_scatter *scatter = ccc_get_scatter(context->parent);
	struct ccc_chunk *chunk = &context->chunks[0];
	unsigned char *pending_data = &context->pending_data[0];
	unsigned int *nr_pending_data = &context->nr_pending_data;
	struct entry *entry = &scatter->entries[0];

	/* Should not happen else it is a mis-configuration. */
	ASSERT(!(scatter->max_chunk_size % sizeof(int)));
	if (*nr_pending_data) {
		unsigned int nr_bytes_to_copy =
			MIN(sizeof(int) - *nr_pending_data, entry->size);

		memcpy(pending_data + *nr_pending_data, entry->src,
		       nr_bytes_to_copy);
		entry->src += nr_bytes_to_copy;
		entry->dst += nr_bytes_to_copy;
		entry->size -= nr_bytes_to_copy;
		scatter->nr_bytes -= nr_bytes_to_copy;
		*nr_pending_data += nr_bytes_to_copy;
		/* Program pending data if the word is complete. */
		if (*nr_pending_data == sizeof(int))
			flush_and_program_pending_data(context);
	}
	context->nr_chunks = HASH_NR_CHUNKS;
	if (!ccc_prepare_chunks(chunk, NULL, scatter,
				&context->nr_chunks))
		return -EINVAL;
	if (context->nr_chunks) {
		struct ccc_chunk *last_chunk =
			&context->chunks[context->nr_chunks - 1];

		if (*nr_pending_data && last_chunk->in.size % sizeof(int))
			return -EFAULT;
		*nr_pending_data = last_chunk->in.size % sizeof(int);
		if (*nr_pending_data) {
			memcpy(pending_data,
			       last_chunk->in.addr + last_chunk->in.size
			       - *nr_pending_data,
			       *nr_pending_data);
			last_chunk->in.size -= *nr_pending_data;
			if (!last_chunk->in.size)
				context->nr_chunks--;
		}
	}
	return 0;
}

static inline int program_computing_in_single_block_mode(
	struct hash_crypto_context *context)
{
	struct ccc_chunk *chunk = &context->chunks[0];
	struct ccc_scatter *scatter = ccc_get_scatter(context->parent);

	context->nr_chunks = HASH_NR_CHUNKS;
	if (!ccc_prepare_chunks(chunk, NULL, scatter,
				&context->nr_chunks))
		return -EINVAL;
	return 0;
}

static inline void hash_program_hw_append(struct hash_crypto_context *context,
					  struct ccc_dma *in)
{
	struct ccc_channel *channel = &context->hash_context->channel;
	struct operation op;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);

	op = hash_get_opcode(channel->index, APPEND, 2,
			     context->alg,
			     in->size);
	ccc_program(dispatcher, op.code, op.wn, in->addr);
}

static inline void uh_program_append(struct hash_crypto_context *context,
				     struct ccc_dma *in)
{
	return hash_program_hw_append(context, in);
}

static inline void uh2_program_append(struct hash_crypto_context *context,
				      struct ccc_dma *in)
{
	unsigned int nr_computed_data = context->nr_computed_data;
#define BOUNDARY 128
	unsigned int nr_beyond = nr_computed_data % BOUNDARY;
	struct ccc_dma remaining = *in;

	if (!ccc_is_multi_block_mode(context->parent))
		return hash_program_hw_append(context, in);
	/*
	 * Multi-block mode denotes that "save/restore" hardware context
	 * operations are being used. Therefore, the following workaround must
	 * be applied.
	 */
	if (((nr_beyond + in->size) > BOUNDARY) || (!nr_beyond)) {
		if (nr_beyond) {
			/*
			 * A second boundary will be crossed. Hence there is
			 * enough data to fill the current block to the first
			 * boundary.
			 */
			struct ccc_dma filling = {remaining.addr,
						  BOUNDARY - nr_beyond};

			hash_program_hw_append(context, &filling);
			remaining.addr += filling.size;
			remaining.size -= filling.size;
		}
		 /* 1. TODO Test filling.size > 0 as well? */
		if (remaining.size > sizeof(int)) {
			/*
			 * There is enough data to append a single word
			 * computation in order to force the previous 1024-bit
			 * block computation if any (see 1).
			 */
			struct ccc_dma word = {remaining.addr,
					       sizeof(int)};

			hash_program_hw_append(context, &word);
			remaining.addr += word.size;
			remaining.size -= word.size;
		}
	}
	hash_program_hw_append(context, &remaining);
	context->nr_computed_data += in->size;
}

static inline void hash_program_appends(struct hash_crypto_context *context)
{
	unsigned int i;
	struct ccc_chunk *chunk = &context->chunks[0];
	struct ccc_channel *channel = &context->hash_context->channel;
	struct hash_data *hash_data = ccc_get_channel_data(channel);

	for (i = 0; i < context->nr_chunks; i++) {
		hash_data->hash_program_append(context, &chunk->in);
		chunk++;
	}
}

int hash_program(void *arg)
{
	struct hash_crypto_context *context = (struct hash_crypto_context *)arg;
	int ret;

	if (ccc_is_linking(context->parent)) {
		/*
		 * HASH input is plugged on another channel output.
		 */
		context->nr_chunks = 0;
	} else {
		/*
		 * HASH input is in memory.
		 */
		if (ccc_is_multi_block_mode(context->parent)) {
			program_hardware_context_restore(context);
			ret = program_computing_in_multi_block_mode(context);
			if (ret)
				return ret;
		} else {
			ret = program_computing_in_single_block_mode(context);
			if (ret)
				return ret;
		}
	}
	hash_program_appends(context);
	if (ccc_is_multi_block_mode(context->parent))
		program_hardware_context_save(context);
	return 0;
}

static inline bool is_digest_truncation_supported(struct hash_data *hash_data)
{
	return hash_data->digest_truncation;
}

#define T_RESET 0
#define FULL 0
#define TRUNCATED 1
int hash_program_epilogue(void *arg)
{
	struct hash_crypto_context *context = (struct hash_crypto_context *)arg;
	unsigned int nr_pending_data = context->nr_pending_data;
	struct ccc_channel *channel = &context->hash_context->channel;
	struct hash_data *data = ccc_get_channel_data(channel);
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	if (ccc_is_multi_block_mode(context->parent)) {
		program_hardware_context_restore(context);
		if (nr_pending_data) {
			/* Should have been programmed in last update. */
			if (nr_pending_data == sizeof(int))
				return -EFAULT;
			flush_and_program_pending_data(context);
		}
	}
	/* If requested, T bit should be inserted here. */
	op = hash_get_opcode(channel->index, END, 2,
			     context->alg,
			     is_digest_truncation_supported(data) ?
			     FULL : T_RESET);
	ccc_program(dispatcher, op.code, op.wn, context->digest);
	return 0;
}

#define FROM_ALG_BITS(a, v) ((a) | ((v) << HASH_VERSION_SHIFT))
/* Return digest size in number of 32bits words. */
static inline unsigned int get_digest_size(struct hash_crypto_context *context)
{
	struct hash_context *hash_context = context->hash_context;
	struct hash_data *data = ccc_get_channel_data(&hash_context->channel);
	unsigned int size = 0;

	switch (FROM_ALG_BITS(context->alg, data->version)) {
	case HASH_MD5_ALG:
		size = 16 / 4;
		break;
	case HASH_SHA_1_ALG:
		size = 20 / 4;
		break;
	case HASH_SHA_224_ALG:
		size = (224 / 8) / 4;
		break;
	case HASH_SHA_256_ALG:
		size = (256 / 8) / 4;
		break;
	case HASH_SHA_384_ALG:
		size = (384 / 8) / 4;
		break;
	case HASH_SHA_512_ALG:
		size = (512 / 8) / 4;
		break;
	default:
		ASSERT(0);
		break;
	}

	return size;
}

int hash_post_processing(void *arg)
{
	struct hash_crypto_context *context = (struct hash_crypto_context *)arg;

#if defined(LITTLE_ENDIAN) && defined(DISABLE_HIF_IF_CH_EN_SWAP)
	if (ccc_is_linking(context->parent))
		swap_bytes(context->digest, get_digest_size(context));
#endif
	put_crypto_context(context);
	return 0;
}

#define UHH_IR 0x1fc
static inline unsigned int read_uhh_ir(struct ccc_channel *c)
{
	return read_reg(c->base + UHH_IR);
}

static inline struct hash_context *get_context(void)
{
	struct hash_context *context = NULL;
	unsigned int i = ARRAY_SIZE(ctx) - 1;

	do {
		if (!ctx[i].inited) {
			context = &ctx[i];
			create_sem(&context->semaphore);
			give_sem(&context->semaphore);
			context->inited = true;
		}
	} while (i-- && !context);
	return context;
}

static inline void init_crypto_context(struct hash_crypto_context *context,
				       struct hash_context *hash_context,
				       unsigned char i)
{
	struct hash_data *data = ccc_get_channel_data(&hash_context->channel);

	memset(context, 0, sizeof(*context));
	context->hash_context = hash_context;
	context->nr_pending_data = 0;
	context->carried_data.addr = &_carried_data[i][0];
	context->carried_data.size = 0;
	context->hw_context = data->hw_contexts;
	context->hw_context.addr += i * context->hw_context.size;
	memset(context->hw_context.addr, 0, context->hw_context.size);
	context->parent = NULL;
}

static void init_crypto_contexts(struct hash_context *context)
{
	for (unsigned char i = NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL; i--;)
		init_crypto_context(&context->crypto_contexts[i], context, i);
}

static inline struct hash_data *hash_get_channel_data(int id)
{
	int i;

	for (i = uh_data.nr_id; i--;) {
		if (id == uh_data.ids[i])
			return &uh_data;
	}
	for (i = uh2_data.nr_id; i--;) {
		if (id == uh2_data.ids[i])
			return &uh2_data;
	}
	return NULL;
}

static struct channel_methods hash_methods = {
	.get_channel = hash_get_channel,
	.crypto_init = hash_crypto_init,
	.program_prologue = hash_program_prologue,
	.program = hash_program,
	.program_epilogue = hash_program_epilogue,
	.post_processing = hash_post_processing
};

int hash_init(struct ccc_controller *c3, int index,
	      struct channel_methods **methods)
{
	struct hash_context *hash_context = get_context();
	struct ccc_channel *channel;
	struct hash_data *channel_data;
#ifdef DEBUG
	unsigned int scr;
#endif

	if (!hash_context)
		return -ENOMEM;
	channel = &hash_context->channel;
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
	 * As channel capabilities regarding supported algorithms are determined
	 * by the channel ID, an unlisted ID will lead to reject some requested
	 * algorithms meant to be supported.
	 */
	if (!ccc_is_channel_supported(channel->id, ids, ARRAY_SIZE(ids)))
		TRACE_ERR(PREFIX "Bad channel identifier: %x\n", channel->id);

	if (!ccc_is_channel_present(channel)) {
		TRACE_ERR(PREFIX
			"Channel %d does not exist\n", channel->index);
		return -EINVAL;
	}

	channel->error_mask = BERR | DERR | PERR | IERR | AERR | OERR;
	channel_data = hash_get_channel_data(channel->id);
	ccc_set_channel_name(channel, channel_data->name);
	ccc_set_channel_data(channel, channel_data);
#ifdef DEBUG
	scr = ccc_read_channel_scr(channel);
	TRACE_INFO(PREFIX "Endianness: %s\n",
		   ccc_get_channel_scr_endian(scr) ?
		   "little/swap" : "big/no swap");
	TRACE_INFO(PREFIX "Cryptoblock Identification: %08x\n",
		   read_uhh_ir(channel));
#endif
	init_crypto_contexts(hash_context);
	*methods = &hash_methods;
	return 0;
}

struct channel_methods *hash_get_methods(void)
{
	return &hash_methods;
}
