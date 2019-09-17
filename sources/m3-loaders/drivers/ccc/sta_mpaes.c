/**
 * @file sta_mpaes.c
 * @brief CCC MPAES channel driver.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <errno.h>
#include <string.h>

#include "sta_ccc_plat.h"
#include "sta_ccc_osal.h"
#include "sta_ccc_if.h"
#include "sta_ccc.h"

#define DEV_NAME "mpaes"
#define PREFIX DEV_NAME ": "

#define IV_SIZE AES_BLOCK_SIZE
#define GCM_TAG_SIZE (128 / 8)
#define GCM_FINAL_SIZE (128 / 8)

struct final {
	unsigned int header_size_most;
	unsigned int header_size_least;
	unsigned int payload_size_most;
	unsigned int payload_size_least;
};

struct ccm_blocks {
	unsigned char b0[AES_BLOCK_SIZE];
	unsigned char header_padding[AES_BLOCK_SIZE];
	unsigned char payload_padding[AES_BLOCK_SIZE];
	void *header_chunk[AES_NR_CHUNKS];
};

struct mpaes_shared_data {
	unsigned char iv[IV_SIZE];
	unsigned char key[AES_KEYSIZE_256];
	struct final final;
	unsigned char tag[GCM_TAG_SIZE];
	struct ccm_blocks ccm_blocks;
};

/*
 * Cryptographic materials memory region shared with MPAES channel.
 */
struct mpaes_shared_data _mpaes_shared_data
__aligned(4) __attribute__((section(".c3_programs")));

enum qualifier {
	ENCRYPT = AES_ENCRYPT,
	DECRYPT = AES_DECRYPT,
	HEADER
};

struct mpaes_context {
	struct ccc_channel channel;
	struct ccc_crypto_context *parent;
	struct mpaes_shared_data *data;
	struct mpaes_alg *alg;
	enum qualifier qualifier;
	int key_slot;
	struct ccc_dma key;
	bool load_key;
	bool lock_special_keys;
	struct ccc_dma iv;
	struct ccc_dma tag;
	struct ccc_dma final;
	unsigned char *out_tag;
	struct ccc_chunk chunks[AES_NR_CHUNKS];
	unsigned int nr_chunks;
	bool enable_byte_bucket;
};

struct mpaes_data {
	unsigned short sp_key_slots;
	unsigned short gp_key_slots;
	const int *ids;
	const unsigned int nr_id;
};

/* List all ids compatible with this driver. */
static const int ids[] = {0x0000e003};

static struct mpaes_data mpaes_data = {.ids = &ids[0],
				       .nr_id = ARRAY_SIZE(ids)};

struct mpaes_alg {
	unsigned int mode;
	bool (*prepare_chunks)(struct ccc_chunk *chunk, void *arg, void *arg2,
			       unsigned int *nr_chunks);
	bool (*check_size)(struct ccc_crypto_req *req);
	void (*set_final_block)(struct mpaes_context *context,
				struct ccc_crypto_req *req);
	bool (*format)(struct mpaes_context *context,
		       struct ccc_crypto_req *req, unsigned char **iv);

};

static struct mpaes_context ctx = {{0},};

static struct ccc_channel *mpaes_get_channel(void *arg)
{
	if (arg)
		return &((struct mpaes_context *)arg)->channel;

	return NULL;
}

#define NONE AES_LOAD_ALG
#define ECB AES_ECB_ALG
#define CBC AES_CBC_ALG
#define CMAC AES_CMAC_ALG
#define GCM AES_GCM_ALG
#define CCM AES_CCM_ALG

static inline bool need_iv(int alg)
{
	return (alg != ECB && alg != NONE);
}

static inline bool produce_tag(int alg)
{
	bool ret = true;

	if ((alg != CMAC) &&
	    (alg != GCM) &&
	    (alg != CCM))
		ret = false;

	return ret;
}

static inline bool is_special_key_slot(struct mpaes_context *context)
{
	struct mpaes_data *data = ccc_get_channel_data(&context->channel);

	return (data->sp_key_slots & (1 << context->key_slot)) != 0;
}

static inline bool is_general_purpose_key_slot(struct mpaes_context *context)
{
	struct mpaes_data *data = ccc_get_channel_data(&context->channel);

	return (data->gp_key_slots & (1 << context->key_slot)) != 0;
}

static inline bool is_key_slot_valid(struct mpaes_context *context)
{
	struct mpaes_data *data = ccc_get_channel_data(&context->channel);

	return ((data->sp_key_slots | data->gp_key_slots) &
		(1 << context->key_slot)) != 0;
}

static inline bool check_key_size(unsigned int key_size)
{
	bool ret = true;

	if ((key_size != AES_KEYSIZE_128) &&
	    (key_size != AES_KEYSIZE_192) &&
	    (key_size != AES_KEYSIZE_256)) {
		TRACE_INFO(PREFIX "Unhandled key size of %dB\n", key_size);
		ret = false;
	}

	return ret;
}

static inline unsigned short get_iv_size(__maybe_unused int alg)
{
	if (alg == CMAC)
		return 0;
	else
		return IV_SIZE;
}

/*
 * Input data formatting for CCM mode as defined in CCM standard
 * [SP800-38C] http://dx.doi.org/10.6028/NIST.SP.800-38C
 */
#define CCM_TAG_MIN_SIZE 4
#define CCM_TAG_MAX_SIZE 16
#define CCM_IV_MIN_SIZE 7
#define CCM_IV_MAX_SIZE 13
#define CCM_HEADER_MASK BIT(6)
static inline bool ccm_format(struct mpaes_context *context,
			      struct ccc_crypto_req *req, unsigned char **iv)
{
	struct ccm_blocks *ccm_blocks = &context->data->ccm_blocks;
	unsigned char modulus, flags = 0;
	unsigned int nr_block;

	if ((req->sym.tag_size < CCM_TAG_MIN_SIZE) ||
	    (req->sym.tag_size > CCM_TAG_MAX_SIZE) ||
	    (ODD(req->sym.tag_size)) ||
	    (req->sym.iv_size < CCM_IV_MIN_SIZE) ||
	    (req->sym.iv_size > CCM_IV_MAX_SIZE)) {
		TRACE_INFO(PREFIX "tag or iv size not valid\n");
		return false;
	}

	/* Block B0 : Formatting of the Control Information and the IV */
	flags = (((((unsigned char)req->sym.tag_size - 2) / 2) & 0x7) << 3) |
		(((unsigned char)(15 - req->sym.iv_size) - 1) & 0x7);
	if (req->sym.header_size)
		flags |= CCM_HEADER_MASK;

	memset(&ccm_blocks->b0[0], 0, AES_BLOCK_SIZE);
	ccm_blocks->b0[0] = flags;
	memcpy(&ccm_blocks->b0[1], req->sym.iv, req->sym.iv_size);

	/* Supports only up to 2^32 bytes of payload */
	ccm_blocks->b0[12] |= (unsigned char)((req->sym.payload_size >> 24)
					      & 0xFF);
	ccm_blocks->b0[13] |= (unsigned char)((req->sym.payload_size >> 16)
					      & 0xFF);
	ccm_blocks->b0[14] |= (unsigned char)((req->sym.payload_size >> 8)
					      & 0xFF);
	ccm_blocks->b0[15] |= (unsigned char)(req->sym.payload_size & 0xFF);
	ccc_dma_sync_for_device();

	/*  Set IV to formatted B0 block */
	*iv = &ccm_blocks->b0[0];

	/* Prepare Associated Data padding block */
	memset(&ccm_blocks->header_padding[0], 0, AES_BLOCK_SIZE);
	modulus = req->sym.header_size % AES_BLOCK_SIZE;
	if (modulus) {
		nr_block = req->sym.header_size - modulus;
		memcpy(&ccm_blocks->header_padding[0],
		       req->sym.header + nr_block, modulus);
	}

	/* Prepare Payload padding block */
	memset(&ccm_blocks->payload_padding[0], 0, AES_BLOCK_SIZE);
	modulus = req->sym.payload_size % AES_BLOCK_SIZE;
	if (modulus) {
		nr_block = req->sym.payload_size - modulus;
		memcpy(&ccm_blocks->payload_padding[0],
		       req->sym.payload + nr_block, modulus);
	}

	return true;
}

static inline bool cmac_format(struct mpaes_context *context,
			       struct ccc_crypto_req *req,
			       __maybe_unused unsigned char **iv)
{
	struct ccm_blocks *ccm_blocks = &context->data->ccm_blocks;
	struct ccc_scatter *scatter = &req->scatter;
	struct entry *entry;
	unsigned int i, nr_block = 0;
	unsigned char *last_byte;
	unsigned char modulus;

	if (!scatter->nr_entries)
		return false;

	/* Check all the entry buffers except the last one are modulus
	 * AES_BLOCK_SIZE */
	for (i = scatter->nr_entries - 1; i--;) {
		entry = &scatter->entries[i];

		if (entry->size % AES_BLOCK_SIZE)
			return false;
	}

	/* Get last block entry */
	entry = &scatter->entries[scatter->nr_entries - 1];

	/* Prepare Payload padding block */
	memset(&ccm_blocks->payload_padding[0], 0, AES_BLOCK_SIZE);
	modulus = entry->size % AES_BLOCK_SIZE;
	if (modulus) {
		nr_block = entry->size - modulus;
		memcpy(&ccm_blocks->payload_padding[0],
		       (unsigned char *)entry->src + nr_block, modulus);
		last_byte = &ccm_blocks->payload_padding[modulus - 1];
	} else {
		last_byte = entry->size ?
			    (unsigned char *)entry->src + entry->size - 1 :
			    (unsigned char *)entry->src;
	}
	/*
	 * CMAC standard requires padding by setting the bit right after
	 * the last significant one and filling the remaining with 0
	 */
	if (req->sym.extra_bit_size) {
		/*
		 * Last payload byte is partial. Set the bit right and
		 * clear all bits after the last meaningful one
		 */
		const unsigned char mask[8] = {0x00, 0x80, 0xC0, 0xE0,
					       0xF0, 0xF8, 0xFC, 0xFE};
		const unsigned char padding[8] = {0x80, 0x40, 0x20, 0x10,
						  0x08, 0x04, 0x02, 0x01};

		*last_byte = (*last_byte & mask[req->sym.extra_bit_size]) |
			     padding[req->sym.extra_bit_size];
	} else {
		ccm_blocks->payload_padding[modulus] = 0x80;
	}

	return true;
}

static inline bool gcm_format(struct mpaes_context *context,
			      struct ccc_crypto_req *req,
			      __maybe_unused unsigned char **iv)
{
	struct ccm_blocks *ccm_blocks = &context->data->ccm_blocks;
	unsigned char modulus;
	unsigned int nr_block;

	/* Prepare Header Data padding block */
	memset(&ccm_blocks->header_padding[0], 0, AES_BLOCK_SIZE);
	modulus = req->sym.header_size % AES_BLOCK_SIZE;
	if (modulus) {
		nr_block = req->sym.header_size - modulus;
		memcpy(&ccm_blocks->header_padding[0],
		       req->sym.header + nr_block, modulus);
	}

	/* Prepare Payload padding block */
	memset(&ccm_blocks->payload_padding[0], 0, AES_BLOCK_SIZE);
	modulus = req->sym.payload_size % AES_BLOCK_SIZE;
	if (modulus) {
		nr_block = req->sym.payload_size - modulus;
		memcpy(&ccm_blocks->payload_padding[0],
		       req->sym.payload + nr_block, modulus);
	}

	return true;
}

static inline void set_iv(struct mpaes_context *context, unsigned char *iv)
{
	context->iv.addr = &(context->data->iv[0]);
	context->iv.size = get_iv_size(context->alg->mode);
	memcpy(context->iv.addr, iv, context->iv.size);
	ccc_dma_sync_for_device();
}

#define TAG_SIZE_MASK 0x38
static inline unsigned int get_tag_size(struct mpaes_context *context)
{
	unsigned char *ccm_b0 = context->iv.addr;

	if (context->alg->mode == CCM)
		/*
		 * Get tag size from CCM B0 block as defined in CCM standard
		 * [SP800-38C] http://dx.doi.org/10.6028/NIST.SP.800-38C
		 */
		return ((*ccm_b0 & TAG_SIZE_MASK) >> 2) + 2;
	else
		return GCM_TAG_SIZE;
}

static inline void set_tag(struct mpaes_context *context, unsigned char *tag)
{
	context->tag.addr = &(context->data->tag[0]);
	context->tag.size = tag ? get_tag_size(context) : 0;
	context->out_tag = tag;
}

static inline unsigned short get_final_size(__maybe_unused int alg)
{
	/* For CCM, must cope with the size being 0 in usequence. */
	if (alg == CCM)
		return 0;
	else
		return GCM_FINAL_SIZE;
}

static inline void gcm_set_final_block(struct mpaes_context *context,
				       struct ccc_crypto_req *req)
{
	struct final *final;
	unsigned int header_size = req->sym.header_size;
	unsigned int payload_size = req->sym.payload_size;

	ASSERT(sizeof(*final) == get_final_size(context->alg->mode));
	final = context->final.addr = &(context->data->final);
	context->final.size = sizeof(struct final);
	memset(final, '\0', sizeof(struct final));
	ASSERT((8 * MAX_CHUNK_SIZE) <= UINT32_MAX);
	final->header_size_least = 8 * header_size;
	final->payload_size_least = 8 * payload_size;
	swap_bytes_if((unsigned char *)&(final->header_size_least),
		      sizeof(final->header_size_least) / sizeof(unsigned int));
	swap_bytes_if((unsigned char *)&(final->payload_size_least),
		      sizeof(final->payload_size_least) / sizeof(unsigned int));
}

static inline void cmac_set_final_block(struct mpaes_context *context,
					struct ccc_crypto_req *req)
{
	struct ccm_blocks *ccm_blocks = &context->data->ccm_blocks;
	struct entry *entry;

	/* Get last block entry */
	entry = &req->scatter.entries[req->scatter.nr_entries - 1];

	if (entry->size < AES_BLOCK_SIZE) {
		/* Only CMAC final block to be handled */
		context->final.size = entry->size;
		context->final.addr = &ccm_blocks->payload_padding[0];
		entry->size = 0;
	} else if (entry->size == AES_BLOCK_SIZE) {
		/* Only CMAC final block to be handled */
		context->final.size = entry->size;
		context->final.addr = entry->src;
		entry->size = 0;
	} else if (!(entry->size % AES_BLOCK_SIZE)) {
		/* Keep last 16 bytes for CMAC final block */
		entry->size -= AES_BLOCK_SIZE;
		context->final.size = AES_BLOCK_SIZE;
		context->final.addr = (unsigned char *)entry->src +
				      entry->size;
	} else {
		context->final.size = entry->size % AES_BLOCK_SIZE;
		context->final.addr = &ccm_blocks->payload_padding[0];
		entry->size = entry->size - (entry->size % AES_BLOCK_SIZE);
	}
}

static inline bool prepare_chunks(struct mpaes_context *context,
				  struct ccc_crypto_req *req)
{
	struct ccm_blocks *ccm_blocks = &context->data->ccm_blocks;
	unsigned int i;

	for (i = 0; i < AES_NR_CHUNKS; i++)
		ccm_blocks->header_chunk[i] = NULL;

	if (context->alg->prepare_chunks)
		return context->alg->prepare_chunks(&context->chunks[0],
						    context->data, req,
						    &context->nr_chunks);
	return false;
}

static bool none_prepare_chunks(__maybe_unused struct ccc_chunk *chunk,
				__maybe_unused void *arg,
				__maybe_unused void *arg2,
				unsigned int *nr_chunks)
{
	*nr_chunks = 0;
	return true;
}

/*
 * The C3 HW IP is able to route requests to a byte bucket.
 * When enabled, any write transaction requests that are within an address
 * window of 64KB starting from a programmed Byte Bucket Base Address are
 * thrown away.
 */
#define BYTE_BUCKET_BASE_ADDR 0x10000
static bool ccm_prepare_chunks(struct ccc_chunk *chunk,
			       void *for_data,
			       void *for_req,
			       unsigned int *nr_chunks)
{
	struct mpaes_shared_data *data = (struct mpaes_shared_data *)for_data;
	struct ccc_crypto_req *req = (struct ccc_crypto_req *)for_req;
	struct ccm_blocks *ccm_blocks = &data->ccm_blocks;
	unsigned int payload_size = req->sym.payload_size;
	unsigned int header_size = req->sym.header_size;
	unsigned char *out = req->sym.dst;
	unsigned char *in;
	unsigned int *in_size;
	unsigned char *in_pad;
	unsigned int i, modulus;
	bool out_discard;

	*nr_chunks = header_size / MAX_CHUNK_SIZE;
	modulus = header_size % MAX_CHUNK_SIZE;
	if ((header_size >= AES_BLOCK_SIZE) && modulus &&
	    (modulus >= AES_BLOCK_SIZE))
		(*nr_chunks)++;
	if (header_size % AES_BLOCK_SIZE)
		(*nr_chunks)++;
	*nr_chunks += payload_size / MAX_CHUNK_SIZE;
	modulus = payload_size % MAX_CHUNK_SIZE;
	if ((payload_size >= AES_BLOCK_SIZE) && modulus &&
	    (modulus >= AES_BLOCK_SIZE))
		(*nr_chunks)++;
	if (payload_size % AES_BLOCK_SIZE)
		(*nr_chunks)++;
	if (*nr_chunks > AES_NR_CHUNKS) {
		TRACE_INFO(PREFIX "Data too big\n");
		return false;
	}
	/* Start by Associated Data parsing */
	in_size = &header_size;
	in = req->sym.header;
	in_pad = &ccm_blocks->header_padding[0];
	out_discard = (req->sym.header_discard == true);
	for (i = 0; i < *nr_chunks; i++) {
		if (!(*in_size)) {
			/* Header parsing complete then switch to payload */
			in_size = &payload_size;
			in = req->sym.payload;
			in_pad = &ccm_blocks->payload_padding[0];
			out_discard = (req->sym.dst == NULL);
		}
		if (*in_size / MAX_CHUNK_SIZE) {
			chunk->in.addr = in;
			chunk->in.size = MAX_CHUNK_SIZE;
			*in_size -= chunk->in.size;
		} else if (*in_size / AES_BLOCK_SIZE) {
			chunk->in.addr = in;
			chunk->in.size = *in_size - (*in_size % AES_BLOCK_SIZE);
			*in_size -= chunk->in.size;
		} else if (*in_size % AES_BLOCK_SIZE) {
			chunk->in.addr = in_pad;
			if (in_pad == &ccm_blocks->header_padding[0])
				chunk->in.size = AES_BLOCK_SIZE;
			else
				chunk->in.size = *in_size;
			*in_size = 0;
		} else {
			continue;
		}

		if (in_pad == &ccm_blocks->header_padding[0])
			ccm_blocks->header_chunk[i] = chunk->in.addr;
		if (out_discard) {
			chunk->out.addr = (void *)BYTE_BUCKET_BASE_ADDR;
		} else {
			chunk->out.addr = out;
			out += chunk->in.size;
		}
		chunk->out.size = chunk->in.size;
		in += chunk->in.size;
		chunk++;
	}
	return true;
}

static inline bool check_request(struct mpaes_context *context,
				 struct ccc_crypto_req *req)
{
	if (context->alg->check_size)
		if (!context->alg->check_size(req)) {
			TRACE_INFO(PREFIX
				   "Data size null or not multiple of %dB\n",
				   AES_BLOCK_SIZE);
			return false;
		}
#ifdef REJECT_UNALIGNED_DATA
	if (!(is_src_32bits_aligned(&req->scatter) &&
	      is_dst_32bits_aligned(&req->scatter) &&
	      THIRTY_TWO_BITS_ALIGNED((unsigned int)req->sym.header) &&
	      THIRTY_TWO_BITS_ALIGNED((unsigned int)req->sym.payload))) {
		TRACE_INFO(PREFIX "Data not aligned on 32bits\n");
		return false;
	}
#endif
	if (!is_src_dst_overlap(&req->scatter)) {
		TRACE_INFO(PREFIX "Unhandled data overlapping\n");
		return false;
	}
	return true;
}

static bool check_null_modulus_size(struct ccc_crypto_req *req)
{
	if (!is_size_null(&req->scatter) ||
	    !is_size_multiple_of(&req->scatter, AES_BLOCK_SIZE))
		return false;

	return true;
}

#define MPAES_USEQ_SCR 0x11c
#define DMA_REG (1 << 31)
static inline void set_reg_mode(struct mpaes_context *context)
{
	struct ccc_channel *channel = &context->channel;
	unsigned int status;

	status = read_reg(channel->base + MPAES_USEQ_SCR);
	status |= DMA_REG;
	write_reg(status, channel->base + MPAES_USEQ_SCR);
}

static inline void set_dma_mode(struct mpaes_context *context)
{
	struct ccc_channel *channel = &context->channel;
	unsigned int status;

	status = read_reg(channel->base + MPAES_USEQ_SCR);
	status &= ~DMA_REG;
	write_reg(status, channel->base + MPAES_USEQ_SCR);
}

#define AESW_STATUS 0x184
#define KEYSEL_SHIFT 8
#define KEYSEL_MASK 0xf
static inline void select_general_purpose_key(struct mpaes_context *context)
{
	struct ccc_channel *channel = &context->channel;
	unsigned int status;

	/* All other bits are RO thus no need to preserve them. */
	status = (context->key_slot & KEYSEL_MASK) << KEYSEL_SHIFT;
	write_reg(status, channel->base + AESW_STATUS);
}

#define KEYSEL_ADDR (C3APB4 + 0x20)
static inline void select_special_key(struct mpaes_context *context)
{
	write_reg(context->key_slot, KEYSEL_ADDR);
}

static inline void lock_special_keys(void)
{
#define LOCK 0x0000000f
	write_reg(LOCK, KEYSEL_ADDR);
}

static void set_key(struct mpaes_context *context, unsigned char *key)
{
	if (is_special_key_slot(context)) {
#ifdef LITTLE_ENDIAN
		unsigned char swapped_key[AES_KEYSIZE_256];

		memcpy(&(swapped_key[0]), key, context->key.size);
		key = &(swapped_key[0]);
		swap_bytes(key, context->key.size / sizeof(unsigned int));
#endif
		select_special_key(context);

#define KEYHHH_ADDR (C3APB4 + 0)
		memcpy((unsigned char *)KEYHHH_ADDR, key, context->key.size);

		if (context->lock_special_keys) {
			/*
			 * Flush write buffer to ensure that key will be written
			 * entirely before locking.
			 */
			ccc_dma_sync_for_device();
			lock_special_keys();
		}
	} else {
		context->key.addr = &(context->data->key[0]);
		memcpy(context->key.addr, key, context->key.size);
		if (!context->load_key)
			select_general_purpose_key(context);
	}
	ccc_dma_sync_for_device();
}

static inline bool cast_ccc_prepare_chunks(struct ccc_chunk *chunk, void *arg,
					   void *for_req,
					   unsigned int *nr_chunks)
{
	return ccc_prepare_chunks(chunk, arg,
				  &((struct ccc_crypto_req *)for_req)->scatter,
				  nr_chunks);
}

struct mpaes_alg mpaes_none_alg = {
	.prepare_chunks = none_prepare_chunks,
	.check_size = NULL,
	.set_final_block = NULL,
	.format = NULL
};

struct mpaes_alg mpaes_ecb_alg = {
	.prepare_chunks = cast_ccc_prepare_chunks,
	.check_size = check_null_modulus_size,
	.set_final_block = NULL,
	.format = NULL
};

struct mpaes_alg mpaes_cbc_alg = {
	.prepare_chunks = cast_ccc_prepare_chunks,
	.check_size = check_null_modulus_size,
	.set_final_block = NULL,
	.format = NULL
};

struct mpaes_alg mpaes_cmac_alg = {
	.prepare_chunks = cast_ccc_prepare_chunks,
	.check_size = NULL,
	.set_final_block = cmac_set_final_block,
	.format = cmac_format
};

struct mpaes_alg mpaes_gcm_alg = {
	.prepare_chunks = ccm_prepare_chunks,
	.check_size = NULL,
	.set_final_block = gcm_set_final_block,
	.format = gcm_format
};

struct mpaes_alg mpaes_ccm_alg = {
	.prepare_chunks = ccm_prepare_chunks,
	.check_size = NULL,
	.set_final_block = NULL,
	.format = ccm_format
};

static struct mpaes_alg *get_alg(struct ccc_crypto_req *req)
{
	switch (req->sym.alg) {
	case NONE:
		return &mpaes_none_alg;
	case ECB:
		return &mpaes_ecb_alg;
	case CBC:
		return &mpaes_cbc_alg;
	case CMAC:
		return &mpaes_cmac_alg;
	case GCM:
		return &mpaes_gcm_alg;
	case CCM:
		return &mpaes_ccm_alg;
	default:
		break;
	}
	return NULL;
}

static bool empty_ok(struct ccc_crypto_req *req)
{
	return (req->sym.alg == NONE ||
		req->sym.alg == CMAC ||
		req->sym.alg == GCM ||
		req->sym.alg == CCM);
}

static void *mpaes_crypto_init(struct ccc_crypto_req *req,
			       struct ccc_crypto_context *parent)
{
	struct mpaes_context *context = &ctx;
	unsigned char *iv = req->sym.iv;

	context->parent = parent;
	context->qualifier = req->sym.direction;
	context->data = &(_mpaes_shared_data);

	context->alg = get_alg(req);
	if (!context->alg) {
		TRACE_INFO(PREFIX "Unhandled mode\n");
		return NULL;
	}
	context->alg->mode = req->sym.alg;

	if (!empty_ok(req) && !req->scatter.nr_bytes) {
		TRACE_INFO(PREFIX "No data to compute\n");
		return NULL;
	}

	context->key_slot = req->sym.key_slot;
	if (!is_key_slot_valid(context)) {
		TRACE_INFO(PREFIX "Key slot is invalid\n");
		return NULL;
	}

	if (!check_key_size(req->sym.key_size))
		return NULL;
	context->key.size = req->sym.key_size;

	if (!check_request(context, req)) {
		TRACE_INFO(PREFIX "Request inconsistent\n");
		return NULL;
	}

	if (context->alg->format)
		if (!context->alg->format(context, req, &iv)) {
			TRACE_INFO(PREFIX "Input data inconsistent\n");
			return NULL;
		}

	if (need_iv(context->alg->mode))
		set_iv(context, iv);

	if (produce_tag(context->alg->mode)) {
		if (!req->sym.tag ||
		    (!req->sym.payload && !is_src_null(&req->scatter))) {
			TRACE_INFO(PREFIX "Tag or payload is missing\n");
			return NULL;
		}
	} else if (req->sym.tag) {
		TRACE_INFO(PREFIX "No tag production for this mode\n");
		return NULL;
	}

	if (produce_tag(context->alg->mode)) {
		set_tag(context, req->sym.tag);

		if (context->alg->set_final_block)
			context->alg->set_final_block(context, req);
	}

	context->nr_chunks = AES_NR_CHUNKS;
	if (!prepare_chunks(context, req)) {
		TRACE_INFO(PREFIX "Unable to prepare chunks\n");
		return NULL;
	}

	context->enable_byte_bucket = (context->alg->mode == CCM ||
				       req->sym.header_discard == true ||
				       req->sym.dst == NULL);

	context->load_key = req->sym.load_key;
	context->lock_special_keys = req->sym.lock_special_keys;
	if (req->sym.key)
		set_key(context, req->sym.key);

	set_dma_mode(context);
	return context;
}

#define OP_ID_SHIFT 22
#define M_SHIFT 16
#define N_SHIFT 0

#define SETUP (1 << 3)
#define EXEC (SETUP | (1 << 2))
#define EXEC_P (EXEC | 0)
#define EXEC_D (EXEC | 1)
#define EXEC_S (EXEC | 2)
#define EXEC_PD (EXEC | 1)
#define EXEC_PS (EXEC | 2)
#define EXEC_SD (EXEC | 3)
#define EXEC_PSD (EXEC | 3)

#define HALT 0
#define KEY_LOAD 0x02
#define KEY_LOAD_SCHED 0x04
#define INITIV 0x0c
#define ECB_ENC_APPEND ECB
#define ECB_DEC_APPEND (ECB_ENC_APPEND + DECRYPT)
#define CBC_ENC_APPEND CBC
#define CBC_DEC_APPEND (CBC_ENC_APPEND + DECRYPT)
#define CMAC_INIT 0x10
#define CMAC_APPEND CMAC
#define CMAC_FINAL (CMAC_APPEND + 1)
#define GCM_INITIV 0x19
#define GCM_HEADER 0x1a
#define GCM_ENC_APPEND GCM
#define GCM_ENC_LAST_APPEND (GCM_ENC_APPEND + 1)
#define GCM_DEC_APPEND (GCM_ENC_APPEND + 2)
#define GCM_DEC_LAST_APPEND (GCM_DEC_APPEND + 1)
#define GCM_FINAL 0x1f
#define CCM_INITIV 0x20
#define CCM_HEADER 0x21
#define CCM_ENC_APPEND CCM
#define CCM_ENC_LAST_APPEND (CCM_ENC_APPEND + 1)
#define CCM_DEC_APPEND (CCM_ENC_APPEND + 2)
#define CCM_DEC_LAST_APPEND (CCM_DEC_APPEND + 1)
#define CCM_FINAL 0x26

static inline unsigned char get_initiv_usequence(int alg)
{
	if (alg == CMAC)
		return CMAC_INIT;
	else if (alg == GCM)
		return GCM_INITIV;
	else if (alg == CCM)
		return CCM_INITIV;
	else
		return INITIV;
}

static unsigned char get_usequence(int alg, enum qualifier qualifier)
{
	unsigned int useq = 0;

	switch (qualifier) {
	case HEADER:
		if (alg == GCM)
			useq = GCM_HEADER;
		else if (alg == CCM)
			useq = CCM_HEADER;
		else
			ASSERT(0);
		break;
	case DECRYPT:
		if (alg == GCM)
			useq = GCM_DEC_APPEND;
		else if (alg == CCM)
			useq = CCM_DEC_APPEND;
		else
			useq = alg + qualifier;
		break;
	case ENCRYPT:
		useq = alg + qualifier;
		break;
	default:
		ASSERT(0);
		break;
	}

	return useq;
}


static unsigned char get_last_usequence(int alg, enum qualifier qualifier)
{
	unsigned int useq = 0;

	switch (qualifier) {
	case DECRYPT:
		if (alg == GCM)
			useq = GCM_DEC_LAST_APPEND;
		else if (alg == CCM)
			useq = CCM_DEC_LAST_APPEND;
		else
			ASSERT(0);
		break;
	case ENCRYPT:
		if (alg == GCM)
			useq = GCM_ENC_LAST_APPEND;
		else if (alg == CCM)
			useq = CCM_ENC_LAST_APPEND;
		else
			ASSERT(0);
		break;
	default:
		ASSERT(0);
		break;
	}

	return useq;
}

static inline unsigned char get_final_usequence(__maybe_unused int alg)
{
	if (alg == CMAC)
		return CMAC_FINAL;
	else if (alg == CCM)
		return CCM_FINAL;
	else
		return GCM_FINAL;
}

static struct operation mpaes_get_opcode(int index, unsigned char usequence,
					 unsigned short data_size)
{
	struct operation op;

	switch (usequence) {
	case CMAC_FINAL:
	case GCM_ENC_LAST_APPEND:
	case GCM_DEC_LAST_APPEND:
	case CCM_ENC_LAST_APPEND:
	case CCM_DEC_LAST_APPEND:
		op.wn = 3;
		op.code = EXEC_PSD;
		break;
	case ECB_ENC_APPEND:
	case ECB_DEC_APPEND:
	case CBC_ENC_APPEND:
	case CBC_DEC_APPEND:
	case GCM_HEADER:
	case GCM_ENC_APPEND:
	case GCM_DEC_APPEND:
	case GCM_FINAL:
	case CCM_INITIV:
	case CCM_HEADER:
	case CCM_ENC_APPEND:
	case CCM_DEC_APPEND:
		op.wn = 2;
		op.code = EXEC_SD;
		break;
	case KEY_LOAD:
	case KEY_LOAD_SCHED:
		op.wn = 2;
		op.code = EXEC_PS;
		break;
	case CCM_FINAL:
		op.wn = 2;
		op.code = EXEC_PD;
		break;
	case INITIV:
	case GCM_INITIV:
	case CMAC_APPEND:
		op.wn = 1;
		op.code = EXEC_S;
		break;
	case CMAC_INIT:
		op.wn = 0;
		op.code = EXEC;
		break;
	case HALT:
		/* Fall through. */
	default:
		/* Can't fail. */
		op.code = HALT;
		op.wn = 0;
		break;
	}
	op.code <<= OP_ID_SHIFT;
	op.code |= (index << CHN_SHIFT) | (op.wn << WN_SHIFT);
	op.code |= (usequence << M_SHIFT) | (data_size << N_SHIFT);

	return op;
}

#define BYTE_BUCKET_ENABLE 1
int mpaes_program_prologue(void *arg)
{
	struct mpaes_context *context = (struct mpaes_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	if (context->load_key) {
		op = mpaes_get_opcode(channel->index,
				      context->qualifier == ENCRYPT ?
				      KEY_LOAD : KEY_LOAD_SCHED,
				      context->key.size);
		ccc_program(dispatcher, op.code, op.wn, context->key_slot,
			    context->key.addr);
	}

	if (context->enable_byte_bucket) {
		/* Set and enable Byte Bucket to avoid unnecessary copy */
		ccc_set_hif_nbar(channel->controller, BYTE_BUCKET_BASE_ADDR);
		ccc_set_hif_ncr(channel->controller, BYTE_BUCKET_ENABLE);
	}

	if (need_iv(context->alg->mode)) {
		op = mpaes_get_opcode(channel->index,
				      get_initiv_usequence(context->alg->mode),
				      get_iv_size(context->alg->mode));
		if (context->alg->mode == CCM)
			ccc_program(dispatcher, op.code, op.wn,
				    context->iv.addr,
				    BYTE_BUCKET_BASE_ADDR);
		else
			ccc_program(dispatcher, op.code, op.wn,
				    context->iv.addr);
	}

	return 0;
}

int mpaes_program(void *arg)
{
	struct mpaes_context *context = (struct mpaes_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct ccc_chunk *chunk = &(context->chunks[0]);
	struct ccm_blocks *ccm_blocks = &context->data->ccm_blocks;
	struct operation op;
	unsigned int i;
	unsigned char useq, useq_payload;
	unsigned short size;
	void *in_addr, *out_addr;
	bool do_produce_tag = produce_tag(context->alg->mode);

	useq_payload = get_usequence(context->alg->mode, context->qualifier);

	for (i = 0; i < context->nr_chunks; i++) {
		size = chunk->in.size;
		useq = useq_payload;
		in_addr = chunk->in.addr;
		out_addr = chunk->out.addr;
		if (do_produce_tag) {
			if (chunk->in.addr == ccm_blocks->header_chunk[i]) {
				useq = get_usequence(context->alg->mode,
						     HEADER);
			} else if (chunk->in.addr ==
					&ccm_blocks->payload_padding[0]) {
				useq = get_last_usequence(context->alg->mode,
							  context->qualifier);
				op = mpaes_get_opcode(channel->index, useq,
						      GCM_TAG_SIZE);
				ccc_program(dispatcher, op.code, op.wn, size,
					    in_addr, out_addr);
				chunk++;
				continue;
			}
		}
		op = mpaes_get_opcode(channel->index, useq, size);
		ccc_program(dispatcher, op.code, op.wn, in_addr, out_addr);
		chunk++;
	}

	return 0;
}

int mpaes_program_epilogue(void *arg)
{
	struct mpaes_context *context = (struct mpaes_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	if (produce_tag(context->alg->mode)) {
		op = mpaes_get_opcode(channel->index,
				      get_final_usequence(context->alg->mode),
				      get_final_size(context->alg->mode));
		if (context->alg->mode == CMAC)
			ccc_program(dispatcher, op.code, op.wn,
				    context->final.size, context->final.addr,
				    context->tag.addr);
		else if (context->alg->mode == CCM)
			ccc_program(dispatcher, op.code, op.wn,
				    context->tag.size, context->tag.addr);
		else
			ccc_program(dispatcher, op.code, op.wn,
				    context->final.addr, context->tag.addr);
	}

	return 0;
}

#define BYTE_BUCKET_DISABLE 0
int mpaes_post_processing(void *arg)
{
	struct mpaes_context *context = (struct mpaes_context *)arg;
	struct ccc_channel *channel = &context->channel;

	if (produce_tag(context->alg->mode))
		memcpy(context->out_tag, context->tag.addr, context->tag.size);

	if (context->enable_byte_bucket)
		ccc_set_hif_ncr(channel->controller, BYTE_BUCKET_DISABLE);

	set_reg_mode(context);
	return 0;
}

static inline struct mpaes_data *mpaes_get_channel_data(__maybe_unused int id)
{
	return &mpaes_data;
}

static inline int mpaes_configure(struct ccc_channel *channel)
{
	struct mpaes_data *data = ccc_get_channel_data(channel);

	data->sp_key_slots = ccc_plat_get_mpaes_sp_key_slots();
	data->gp_key_slots = ccc_plat_get_mpaes_gp_key_slots();
	return 0;
}

static struct channel_methods mpaes_methods = {
	.get_channel = mpaes_get_channel,
	.configure = mpaes_configure,
	.crypto_init = mpaes_crypto_init,
	.program_prologue = mpaes_program_prologue,
	.program = mpaes_program,
	.program_epilogue = mpaes_program_epilogue,
	.post_processing = mpaes_post_processing
};

int mpaes_init(struct ccc_controller *c3, int index,
	       struct channel_methods **methods)
{
	struct ccc_channel *channel = &ctx.channel;
	int ret;
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

	channel->error_mask = BERR | DERR | PERR | IERR | AERR;
	ccc_set_channel_name(channel, DEV_NAME);
	ccc_set_channel_data(channel, mpaes_get_channel_data(channel->id));
	ret = mpaes_configure(channel);
	if (ret)
		return ret;
	*methods = &mpaes_methods;
#ifdef DEBUG
	scr = ccc_read_channel_scr(channel);
	TRACE_INFO(PREFIX "Endianness: %s\n",
		   ccc_get_channel_scr_endian(scr) ?
		   "little/swap" : "big/no swap");
#endif
	return 0;
}
