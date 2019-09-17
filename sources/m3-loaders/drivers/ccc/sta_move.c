/**
 * @file sta_move.c
 * @brief CCC MOVE channel driver.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <errno.h>
#include <stdarg.h>

#include "sta_ccc_plat.h"
#include "sta_ccc_osal.h"
#include "sta_ccc_if.h"
#include "sta_ccc.h"

#define DEV_NAME "move"
#define PREFIX DEV_NAME ": "

struct move_context {
	struct ccc_channel channel;
	struct ccc_crypto_context *parent;
	struct ccc_chunk chunks[MOVE_NR_CHUNKS];
	unsigned int nr_chunks;
};

static struct move_context ctx = {{0},};

struct move_data {
	const int *ids;
	const unsigned int nr_id;
};

/* List all ids compatible with this driver. */
static const int ids[] = {0x00001020, 0x00001021};

static struct move_data move_data = {.ids = &ids[0],
				     .nr_id = ARRAY_SIZE(ids)};

static struct ccc_channel *move_get_channel(void *arg)
{
	if (arg)
		return &((struct move_context *)arg)->channel;

	return NULL;
}

static void *move_crypto_init(struct ccc_crypto_req *req,
			      struct ccc_crypto_context *parent)
{
	struct move_context *context = &ctx;

	context->parent = parent;

	if (!is_dst_32bits_aligned(&req->scatter)) {
		TRACE_INFO(PREFIX "Output not aligned on 32bits\n");
		return NULL;
	}

	if (req->scatter.nr_bytes == 0) {
		TRACE_INFO(PREFIX "No data to compute\n");
		return NULL;
	}
	if (!is_size_32bits_aligned(&req->scatter)) {
		TRACE_INFO(PREFIX "Data size not multiple of 4B\n");
		return NULL;
	}
	if (!is_src_32bits_aligned(&req->scatter)) {
		TRACE_INFO(PREFIX "Input not aligned on 32bits\n");
		return NULL;
	}
	context->nr_chunks = MOVE_NR_CHUNKS;
	if (!ccc_prepare_chunks(&context->chunks[0], NULL, &req->scatter,
				&context->nr_chunks))
		return NULL;

	return context;
}

#define OP_ID_SHIFT 23
#define INIT 4
#define LOGICAL_OP_SHIFT 21
#define DATA 5
#define LENGTH_SHIFT 0
static struct operation move_get_opcode(int index, unsigned char code,
					unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case INIT:
		ASSERT(nr_param == 1);
		op.code = code << OP_ID_SHIFT;
		op.wn = 1;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << LOGICAL_OP_SHIFT;
		va_end(params);
		break;
	case DATA:
		ASSERT(nr_param == 2);
		op.code = code << OP_ID_SHIFT;
		op.wn = 2;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << LOGICAL_OP_SHIFT;
		op.code |= va_arg(params, unsigned int) << LENGTH_SHIFT;
		va_end(params);
		break;
	default:
		ASSERT(0);
		op.wn = 0;
		break;
	}
	op.code |= (index << CHN_SHIFT) | (op.wn << WN_SHIFT);

	return op;
}

static inline int move_program_prologue(__maybe_unused void *arg)
{
	return 0;
}

#define NOP 0
#define AND 1
#define OR 2
#define XOR 3
int move_program(void *arg)
{
	struct move_context *context = (struct move_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct ccc_chunk *chunk = &(context->chunks[0]);
	struct operation op;
	unsigned int i;

	op = move_get_opcode(channel->index, INIT, 1,
			     NOP);
	ccc_program(dispatcher, op.code, op.wn, 0);

	for (i = 0; i < context->nr_chunks; i++) {
		op = move_get_opcode(channel->index, DATA, 2,
				     NOP,
				     chunk->in.size);
		ccc_program(dispatcher, op.code, op.wn, chunk->in.addr,
			    chunk->out.addr);
		chunk++;
	}

	return 0;
}

static inline int move_program_epilogue(__maybe_unused void *arg)
{
	return 0;
}

static inline int move_post_processing(__maybe_unused void *arg)
{
	return 0;
}

static inline struct move_data *move_get_channel_data(__maybe_unused int id)
{
	return &move_data;
}

static struct channel_methods move_methods = {
	.get_channel = move_get_channel,
	.crypto_init = move_crypto_init,
	.program_prologue = move_program_prologue,
	.program = move_program,
	.program_epilogue = move_program_epilogue,
	.post_processing = move_post_processing
};

int move_init(struct ccc_controller *c3, int index,
	      struct channel_methods **methods)
{
	struct ccc_channel *channel = &ctx.channel;

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
	ccc_set_channel_data(channel, move_get_channel_data(channel->id));
	*methods = &move_methods;
	return 0;
}
