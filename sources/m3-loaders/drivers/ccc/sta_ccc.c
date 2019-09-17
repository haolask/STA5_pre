/**
 * @file sta_ccc.c
 * @brief CCC driver core.
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

#define DEV_NAME "ccc"
#define PREFIX DEV_NAME ": "

#define PROGRAMS_SIZE_IN_WORDS ((PROGRAM_SIZE_IN_BYTES / sizeof(unsigned int)) \
				* NR_DISPATCHERS)
unsigned int const _c3_programs[PROGRAMS_SIZE_IN_WORDS]
__attribute__((section(".c3_programs")));
#define PROGRAMS _c3_programs

#define NR_CHANNELS 16
static enum ccc_channel_type plat_topology[NR_CHANNELS];

struct ccc_context {
	struct ccc_controller controller;
	const unsigned int *programs;
	semaphore_t semaphore;
	struct ccc_crypto_context crypto_contexts[NR_CRYPTO_CONTEXTS];
	struct ccc_methods methods;
	struct channel_methods *channels_methods[NR_CHANNEL_TYPES];
};

static struct ccc_context driver_context = {{0},};

struct walk {
	void *src;
	void *dst;
	unsigned int nr_bytes;
	struct ccc_scatter *scatter;
	unsigned int scatter_index;
};

static inline struct ccc_context *get_context(void)
{
	return &driver_context;
}

bool is_in_place(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (entry->src != entry->dst)
			return false;
	}
	return true;
}

bool is_src_32bits_aligned(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (!THIRTY_TWO_BITS_ALIGNED((unsigned int)entry->src))
			return false;
	}
	return true;
}

bool is_dst_32bits_aligned(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (!THIRTY_TWO_BITS_ALIGNED((unsigned int)entry->dst))
			return false;
	}
	return true;
}

bool is_size_32bits_aligned(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (!THIRTY_TWO_BITS_ALIGNED(entry->size))
			return false;
	}
	return true;
}

bool is_size_null(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (!entry->size)
			return false;
	}
	return true;
}

bool is_size_multiple_of(struct ccc_scatter *scatter, unsigned int modulus)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (entry->size % modulus)
			return false;
	}
	return true;
}

bool is_src_dst_overlap(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if ((((unsigned int)entry->dst > (unsigned int)entry->src)) &&
		    ((unsigned int)entry->dst <
		     ((unsigned int)entry->src + entry->size)))
			return false;
	}
	return true;
}

bool is_src_null(struct ccc_scatter *scatter)
{
	unsigned int i;

	for (i = scatter->nr_entries; i--;) {
		struct entry *entry = &scatter->entries[i];

		if (entry->src)
			return true;
	}
	return false;
}

static int walk_init(struct walk *walk, struct ccc_scatter *scatter)
{
	struct entry *entry;

	if (!walk || !scatter)
		return -EINVAL;
	if (scatter->control != SCATTER_CONTROL_WORD) {
		TRACE_INFO(PREFIX "Junk data\n");
		return -EINVAL;
	}
	walk->scatter = scatter;
	walk->scatter_index = 0;
	entry = &scatter->entries[walk->scatter_index];
	walk->src = entry->src;
	walk->dst = entry->dst;
	walk->nr_bytes = entry->size;
	return 0;
}

static int walk_done(struct walk *walk, unsigned int nr_bytes)
{
	struct ccc_scatter *scatter;
	struct entry *entry;
	unsigned int remaining;

	if (!walk)
		return -EINVAL;
	if (nr_bytes > walk->nr_bytes)
		return -EINVAL;
	scatter = walk->scatter;
	entry = &scatter->entries[walk->scatter_index];
	remaining = walk->nr_bytes - nr_bytes;
	walk->nr_bytes = nr_bytes;
	if (walk->nr_bytes == 0) {
		if (++walk->scatter_index >= scatter->nr_entries)
			return 0;
		entry++;
		walk->src = entry->src;
		walk->dst = entry->dst;
		walk->nr_bytes = entry->size;
	} else {
		walk->src += remaining;
		walk->dst += remaining;
	}
	return 0;
}

/*
 * chunk input points to chunks to program.
 * arg is not used.
 * req input handles the scatter buffers.
 * nr_chunks handles at input the number of chunks to program.
 * nr_chunks outputs the number of chunks actually programmed.
 */
bool ccc_prepare_chunks(struct ccc_chunk *chunk,
			__maybe_unused void *arg,
			struct ccc_scatter *scatter,
			unsigned int *nr_chunks)
{
	struct walk walk;
	unsigned int max_nr_chunks = *nr_chunks;
	unsigned int nr_bytes;

	if (walk_init(&walk, scatter))
		return false;

	*nr_chunks = 0;
	while ((nr_bytes = walk.nr_bytes) > 0) {
		if (*nr_chunks >= max_nr_chunks) {
			TRACE_INFO(PREFIX "Data too big\n");
			return false;
		}
		chunk->in.size = MIN(nr_bytes, scatter->max_chunk_size);
		chunk->out.size = chunk->in.size;

		chunk->in.addr = walk.src;
		chunk->out.addr = walk.dst;
		nr_bytes -= chunk->in.size;

		walk_done(&walk, nr_bytes);
		chunk++;
		(*nr_chunks)++;
	}
	return true;
}

void ccc_set_channel_name(struct ccc_channel *channel, const char *name)
{
	ASSERT(channel);
	ASSERT(name);

	memset(channel->name, '\0', CCC_NAME_SIZE);
	strncpy(channel->name, name, CCC_NAME_SIZE - sizeof('\0'));
}

bool ccc_is_channel_supported(int id, const int *ids, unsigned int nr_id)
{
	if (!ids || !nr_id)
		return false;
	do {
		if (*ids == id)
			return true;
		ids++;
	} while (--nr_id);
	return false;
}

#define ARST (1 << 16)
static inline void controller_reset(struct ccc_controller *c3)
{
	write_reg(ARST, c3->base + C3_SYS + SYS_SCR);
	udelay(1);
}

#define RST (1 << 16)
static inline void channel_reset(struct ccc_channel *channel)
{
	write_reg(RST, channel->base + CH_SCR);
	udelay(1);
}

#define OP_ID_SHIFT 23
#define STOP 0
#define WAIT 1
#define CLOCK_CYCLES_SHIFT 0
#define NOP 3
#define COUPLE 6
#define MASTER_SHIFT 19
#define PORT_SHIFT 18
#define SLAVE_SHIFT 14
#define PATH_SHIFT 11
#define UNCOUPLE 7
struct operation ccc_get_opcode(unsigned char code, unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	/* Flow type instructions make use of channel index 0. */
	switch (code) {
	case COUPLE:
		ASSERT(nr_param == 4);
		op.code = code << OP_ID_SHIFT;
		op.wn = 0;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << MASTER_SHIFT;
		op.code |= va_arg(params, unsigned int) << PORT_SHIFT;
		op.code |= va_arg(params, unsigned int) << SLAVE_SHIFT;
		op.code |= va_arg(params, unsigned int) << PATH_SHIFT;
		va_end(params);
		break;
	case UNCOUPLE:
		ASSERT(nr_param == 1);
		op.code = code << OP_ID_SHIFT;
		op.wn = 0;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << PATH_SHIFT;
		va_end(params);
		break;
	case STOP:
		ASSERT((nr_param == 1) || (nr_param == 0));
		op.code = code << OP_ID_SHIFT;
		/* Caller must take care of word0 if status is requested. */
		op.wn = nr_param ? 1 : 0;
		op.code |= op.wn << WN_SHIFT;
		break;
	case WAIT:
		ASSERT(nr_param == 1);
		op.code = code << OP_ID_SHIFT;
		op.wn = 0;
		va_start(params, nr_param);
		op.code |= va_arg(params, unsigned int) << CLOCK_CYCLES_SHIFT;
		va_end(params);
		break;
	case NOP:
		/* Fall through. */
	default:
		/* Can't fail. */
		op.code = NOP << OP_ID_SHIFT;
		op.wn = 0;
		break;
	}

	return op;
}

struct operation ccc_get_wait_opcode(unsigned short nr_cycles)
{
	return ccc_get_opcode(WAIT, 1, nr_cycles);
}

static inline bool is_dispatcher_busy(struct ccc_dispatcher *disp)
{
	unsigned int scr = ccc_read_id_scr(disp);
	unsigned int ids = ccc_get_dispatcher_status(scr);

	return (ids == S_BUSY);
}

#ifndef HAVE_INTERRUPT
static inline void init_completion(completion_t *completion,
				   struct ccc_dispatcher *disp)
{
	ASSERT(completion);
	ASSERT(disp);
	*completion = disp;
}

static inline void reinit_completion(__maybe_unused completion_t *completion)
{
}

static inline void wait_for_completion(completion_t *completion)
{
	struct ccc_dispatcher *disp = *completion;

	ASSERT(completion);
	ASSERT(disp);
	while (is_dispatcher_busy(disp))
		;
}

static inline void complete(__maybe_unused completion_t *completion)
{
}
#endif /* !HAVE_INTERRUPT */

static inline void reinit_dispatcher(struct ccc_dispatcher *disp)
{
	disp->pc = disp->program.addr;
	reinit_completion(&disp->execution);
}

static int attach_dispatcher(struct ccc_crypto_context *context)
{
	struct ccc_controller *c3 = context->controller;
	struct ccc_dispatcher *disp = &c3->dispatchers[0];
	struct ccc_dispatcher *found = NULL;
	unsigned int i = c3->nr_dispatchers;

	if (context->dispatcher)
		return -EFAULT;
	take_sem(&c3->semaphore);
	do {
		if (!disp->crypto_context)
			found = disp;
		disp++;
	} while (--i && !found);
	if (found) {
		reinit_dispatcher(found);
		found->crypto_context = context;
		context->dispatcher = found;
	}
	give_sem(&c3->semaphore);
	return found ? 0 : -EBUSY;
}

static void detach_dispatcher(struct ccc_crypto_context *context)
{
	struct ccc_dispatcher *disp = context->dispatcher;
	struct ccc_controller *c3 = disp->controller;

	take_sem(&c3->semaphore);
	disp->crypto_context = NULL;
	context->dispatcher = NULL;
	give_sem(&c3->semaphore);
}

static void init_crypto_context(struct ccc_crypto_context *context,
				struct ccc_controller *c3)
{
	memset(context, 0, sizeof(*context));
	context->controller = c3;
	context->control = CRYPTO_CONTEXT_CONTROL_WORD;
	context->multi_block_mode = false;
	context->link = none;
	context->state = free;
}

static inline void put_crypto_context(struct ccc_crypto_context *context)
{
	struct ccc_controller *c3 = context->dispatcher->controller;

#if defined(LITTLE_ENDIAN) && defined(DISABLE_HIF_IF_CH_EN_SWAP)
	bool *use = &context->channel_uses[0];

	/* Set back HIF_IFCR.CH_END to normal value.*/
	if (use[MOVE] && use[HASH]) {
		ccc_set_hif_ifcr_ch_end(c3, true);
		TRACE_INFO(PREFIX "HIF_IFCR.CH_END is set.\n");
	}
#endif
	take_sem(&get_context()->semaphore);
	if (context->dispatcher)
		detach_dispatcher(context);
	init_crypto_context(context, c3);
	give_sem(&get_context()->semaphore);
}

void ccc_program(struct ccc_dispatcher *d, unsigned int opcode,
		 unsigned char nr_param, ...)
{
	va_list params;

	ASSERT(d);

	/* Increase MAX_NR_INSTRUCTIONS if this assertion fails. */
	ASSERT(ccc_get_program_free_len(d) >=
	       (nr_param + 1) * sizeof(unsigned int));

	*d->pc++ = opcode;
	va_start(params, nr_param);
	while (nr_param) {
		unsigned int param = va_arg(params, unsigned int);

		*d->pc++ = param;
		nr_param--;
	}
	va_end(params);
}

#define BERR (1 << 29)
#define CERR (1 << 26)
#define CBSY (1 << 25)
#define CDNX (1 << 24)
static int report_dispatcher_error(struct ccc_dispatcher *disp)
{
	unsigned int scr = ccc_read_id_scr(disp);
	int ret = 0;

	if (scr & BERR) {
		ret = -EFAULT;
		TRACE_INFO(PREFIX "BERR\n");
	}
	if (scr & CERR) {
		ret = -ENOEXEC;
		TRACE_INFO(PREFIX "CERR\n");
	}
	if (scr & CBSY) {
		ret = -EBUSY;
		TRACE_INFO(PREFIX "CBSY\n");
	}
	if (scr & CDNX) {
		ret = -ENODEV;
		TRACE_INFO(PREFIX "CDNX\n");
	}

	if (!ret) {
		ret = -EIO;
		TRACE_INFO(PREFIX "Error handling error ");
	}

	return ret;
}

static int check_dispatcher_state(struct ccc_dispatcher *disp)
{
	unsigned int scr = ccc_read_id_scr(disp);
	unsigned int ids = ccc_get_dispatcher_status(scr);
	int ret = 0;

	switch (ids) {
	case S_ERROR:
		TRACE_INFO(PREFIX "Dispatcher in error state\n");
		/*
		 * Caller does not need to reset dispatcher as Error state
		 * will be exited at next program execution.
		 */
		ret = report_dispatcher_error(disp);
		break;
	case S_NOT_PRESENT:
		TRACE_INFO(PREFIX "Dispatcher not present\n");
		ret = -ENODEV;
		break;
	case S_BUSY:
		TRACE_INFO(PREFIX "Dispatcher busy\n");
		ret = -EBUSY;
		break;
	case S_IDLE:
		break;
	default:
		ret = -EFAULT;
		TRACE_INFO(PREFIX "Error handling error\n");
		break;
	}

	return ret;
}

/*
 * Coupling and Chaining Module programming capabilities.
 * This cross table is sorted by ccc_channel_type order and should be read
 * "The slave input at column j is linked to the master output at row i."
 */
static enum ccm_link ccm_caps[NR_CHANNEL_TYPES][NR_CHANNEL_TYPES] = {
	{x, _, _, _, _},
	{x, x, _, _, c},
	{x, x, x, _, _},
	{x, x, x, x, o},
	{x, x, x, x, x}
};

static inline enum ccm_link get_ccm_link(struct ccc_crypto_context *context,
					 enum ccc_channel_type *master,
					 enum ccc_channel_type *slave)
{
	bool *use = &context->channel_uses[0];

	/*
	 * Check chaining and coupling capabilities.
	 */
	for (int i = NR_CHANNEL_TYPES; i--;)
		if (use[i])
			for (int j = NR_CHANNEL_TYPES; j--;)
				if (i < j  && use[j])
					if (ccm_caps[i][j] != none) {
						if (master)
							*master = i;
						if (slave)
							*slave = j;
						return ccm_caps[i][j];
					}
	return none;
}

#define INPUT_PORT 0
#define OUTPUT_PORT 1
#define CCM_GENERIC_PATH 0
static inline void program_prologue(struct ccc_crypto_context *context)
{
	enum ccc_channel_type master_type, slave_type;
	struct ccc_channel *(*get_channel)(void *);
	void **channel_contexts = &context->channel_contexts[0];
	int master_index, slave_index;
	unsigned int port;
	struct operation op;

	context->link = get_ccm_link(context, &master_type, &slave_type);
	if (context->link == none)
		return;
	get_channel = get_context()->channels_methods[master_type]->get_channel;
	master_index = get_channel(channel_contexts[master_type])->index;
	get_channel = get_context()->channels_methods[slave_type]->get_channel;
	slave_index = get_channel(channel_contexts[slave_type])->index;
	port = context->link == chain ? OUTPUT_PORT : INPUT_PORT;
	op = ccc_get_opcode(COUPLE, 4,
			    master_index,
			    port,
			    slave_index,
			    CCM_GENERIC_PATH);
	ccc_program(context->dispatcher, op.code, op.wn);
}

static inline void program_epilogue(struct ccc_crypto_context *context)
{
	struct operation op;

	if (context->link != none) {
		op = ccc_get_opcode(UNCOUPLE, 1,
				    CCM_GENERIC_PATH);
		ccc_program(context->dispatcher, op.code, op.wn);
	}
	op = ccc_get_opcode(STOP, 0);
	ccc_program(context->dispatcher, op.code, op.wn);
}

static int run_program(struct ccc_dispatcher *disp)
{
	ccc_dma_sync_for_device();
#ifdef HAVE_INTERRUPT
	ccc_enable_interrupts(disp);
#endif
	start_counter();
	ccc_set_instruction_pointer(disp);
	wait_for_completion(&disp->execution);

	return check_dispatcher_state(disp);
}

void ccc_isr(void)
{
	struct ccc_controller *c3 = &get_context()->controller;
	struct ccc_dispatcher *disp = c3->dispatchers;
	unsigned char i;
	unsigned int scr, isdn;

	scr = ccc_read_sys_scr(c3);
	isdn = ccc_get_sys_scr_isdn(scr) & c3->en_dispatchers;
	for (i = 0; i < c3->nr_dispatchers; i++) {
		if (isdn & (1 << i)) {
			ccc_disable_interrupts(disp);
			ccc_clear_interrupt(disp);
			complete(&disp->execution);
		}
		disp++;
	}
}

static inline void init_crypto_contexts(struct ccc_controller *c3)
{
	for (unsigned char i = NR_CRYPTO_CONTEXTS; i--;)
		init_crypto_context(&get_context()->crypto_contexts[i], c3);
}

static inline struct ccc_crypto_context *get_crypto_context(void)
{
	struct ccc_crypto_context *context = &get_context()->crypto_contexts[0];
	struct ccc_crypto_context *found = NULL;
	unsigned int i = NR_CRYPTO_CONTEXTS;

	take_sem(&get_context()->semaphore);
	do {
		if (context->state == free)
			found = context;
		context++;
	} while (--i && !found);

	if (found) {
		if (attach_dispatcher(found))
			found = NULL;
		else
			found->state = init;
	}
	give_sem(&get_context()->semaphore);
	return found;
}

static inline int report_channel_error(struct ccc_channel *channel)
{
	unsigned int scr = ccc_read_channel_scr(channel);
	int ret = 0;

	if (scr & BERR & channel->error_mask) {
		ret = -EFAULT;
		TRACE_INFO(PREFIX "%s: BERR\n", channel->name);
	}
	if (scr & DERR & channel->error_mask) {
		ret = -ENOEXEC;
		TRACE_INFO(PREFIX "%s: DERR\n", channel->name);
	}
	if (scr & PERR & channel->error_mask) {
		ret = -EBUSY;
		TRACE_INFO(PREFIX "%s: PERR\n", channel->name);
	}
	if (scr & IERR & channel->error_mask) {
		ret = -ENODEV;
		TRACE_INFO(PREFIX "%s: IERR\n", channel->name);
	}
	if (scr & AERR & channel->error_mask) {
		ret = -ENODEV;
		TRACE_INFO(PREFIX "%s: AERR\n", channel->name);
	}
	if (scr & OERR & channel->error_mask) {
		ret = -ENODEV;
		TRACE_INFO(PREFIX "%s: OERR\n", channel->name);
	}

	if (!ret) {
		ret = -EIO;
		TRACE_INFO(PREFIX "%s: Error handling error\n",
			   channel->name);
		ASSERT(0);
	}

	return ret;
}

static int check_channel_state(struct ccc_channel *channel)
{
	unsigned int scr = ccc_read_channel_scr(channel);
	unsigned int cs = ccc_get_channel_status(scr);
	int ret = 0;

	switch (cs) {
	case S_ERROR:
		TRACE_INFO(PREFIX "Channel in error state\n");
		/*
		 * Caller does not need to reset channel as Error state will be
		 * exited at next program execution.
		 */
		ret = report_channel_error(channel);
		break;
	case S_NOT_PRESENT:
		TRACE_INFO(PREFIX "%s: Channel not present\n", channel->name);
		ret = -ENODEV;
		break;
	case S_BUSY:
		TRACE_INFO(PREFIX "%s: Channel busy\n", channel->name);
		ret = -EBUSY;
		break;
	case S_IDLE:
		break;
	default:
		ret = -EFAULT;
		TRACE_INFO(PREFIX "%s: Error handling error\n", channel->name);
		ASSERT(0);
		break;
	}

	return ret;
}

static inline int perform_standalone_crypto(struct ccc_crypto_context *context,
					    enum ccc_channel_type type)
{
	struct ccc_channel *channel;
	void *channel_context;
	struct channel_methods *channel_methods;
	int ret;

	channel_context = context->channel_contexts[type];
	channel_methods = get_context()->channels_methods[type];
	program_prologue(context);
	ret = channel_methods->program_prologue(channel_context);
	if (ret)
		goto exit;
	ret = channel_methods->program(channel_context);
	if (ret)
		goto exit;
	program_epilogue(context);
	ret = run_program(context->dispatcher);
	channel = channel_methods->get_channel(channel_context);
	ASSERT(channel);
	ret |= check_channel_state(channel);
	if (!ret)
		ret = channel_methods->post_processing(channel_context);
exit:
	return ret;
}

static int perform_multi_block_crypto(struct ccc_crypto_context *context)
{
	struct ccc_channel *channel;
	bool *use = &context->channel_uses[0];
	void *channel_context;
	struct channel_methods *channel_methods;
	int ret = 0;

	/*
	 * TODO: Check multi-block feature support instead of this particular
	 * channel usage.
	 */
	if (!use[HASH]) {
		ret = -EOPNOTSUPP;
		goto exit;
	}
	channel_context = context->channel_contexts[HASH];
	channel_methods = get_context()->channels_methods[HASH];
	switch (context->state) {
	case init:
		program_prologue(context);
		ret = channel_methods->program_prologue(channel_context);
		if (ret)
			goto exit;
		break;
	case update:
		if (context->scatter->nr_entries != 1) {
			ret = -EINVAL;
			goto exit;
		}
		ret = channel_methods->program(channel_context);
		if (ret)
			goto exit;
		program_epilogue(context);
		ret = run_program(context->dispatcher);
		channel = channel_methods->get_channel(channel_context);
		ret |= check_channel_state(channel);
		if (ret)
			goto exit;
		break;
	case finish:
		ret = channel_methods->program_epilogue(channel_context);
		if (ret)
			goto exit;
		program_epilogue(context);
		ret = run_program(context->dispatcher);
		channel = channel_methods->get_channel(channel_context);
		ret |= check_channel_state(channel);
		if (ret)
			goto exit;
		ret = channel_methods->post_processing(channel_context);
		if (ret)
			goto exit;
		break;
	case free:
	default:
		ret = -EFAULT;
		goto exit;
	}
exit:
	return ret;
}

static inline int perform_combinable_crypto(struct ccc_crypto_context *context)
{
	struct ccc_channel *channel;
	bool *use = &context->channel_uses[0];
	void **channel_contexts;
	struct channel_methods **channels_methods;
	enum ccc_channel_type types[] = {HASH, MPAES, MOVE};
	enum ccc_channel_type type;
	unsigned int i;
	int ret;

	channel_contexts = &context->channel_contexts[0];
	channels_methods = &get_context()->channels_methods[0];
	program_prologue(context);

	for (i = 0; i < ARRAY_SIZE(types); i++) {
		type = types[i];
		if (use[type]) {
			ret = channels_methods[type]->program_prologue(
				channel_contexts[type]);
			if (ret)
				goto exit;
		}
	}
	for (i = 0; i < ARRAY_SIZE(types); i++) {
		type = types[i];
		if (use[type]) {
			ret = channels_methods[type]->program(
				channel_contexts[type]);
			if (ret)
				goto exit;
		}
	}
	for (i = ARRAY_SIZE(types); i--;) {
		type = types[i];
		if (use[type]) {
			ret = channels_methods[type]->program_epilogue(
				channel_contexts[type]);
			if (ret)
				goto exit;
		}
	}

	program_epilogue(context);
	ret = run_program(context->dispatcher);

	for (i = ARRAY_SIZE(types); i--;) {
		type = types[i];
		if (use[type]) {
			channel = channels_methods[type]->get_channel(
				channel_contexts[type]);
			ret |= check_channel_state(channel);
		}
	}
	if (ret)
		goto exit;
	for (i = ARRAY_SIZE(types); i--;) {
		type = types[i];
		if (use[type]) {
			ret = channels_methods[type]->post_processing(
				channel_contexts[type]);
			if (ret)
				goto exit;
		}
	}
exit:
	return ret;
}

static inline int perform_single_block_crypto(
	struct ccc_crypto_context *context)
{
	bool *use = &context->channel_uses[0];

	if (use[TRNG])
		return perform_standalone_crypto(context, TRNG);

	if (use[PKA])
		return perform_standalone_crypto(context, PKA);

	return perform_combinable_crypto(context);
}

static int check_request(struct ccc_crypto_req *req,
			 struct ccc_crypto_context *context)
{
	bool *use = &context->channel_uses[0];
	unsigned int nr_use = 0;

	/*
	 * Randomization
	 */
	use[TRNG] = req->randomize.alg != RNG_NONE_ALG;

	/*
	 * Symmetric crypto
	 * Can assume 'move' operation as well.
	 */
	use[MPAES] = req->sym.alg != AES_NONE_ALG;

	/*
	 * Asymmetric crypto
	 */
	use[PKA] = req->asym.alg != PK_NONE_ALG;

	/*
	 * Move
	 * Implicitly set if source and destination are different and if
	 * some other cryptographic operation are not requested as well.
	 */
	use[MOVE] = !use[TRNG] && !use[MPAES] && !use[PKA] &&
		req->scatter.nr_bytes && !is_in_place(&req->scatter);

	/*
	 * Hash
	 */
	use[HASH] = req->hash.alg != HASH_NONE_ALG;

	/*
	 * Check chaining and coupling capabilities.
	 */
	for (int i = NR_CHANNEL_TYPES; i--;)
		if (use[i]) {
			nr_use++;
			for (int j = NR_CHANNEL_TYPES; j--;)
				if (i < j  && use[j] && ccm_caps[i][j] == none)
					return -EPERM;
		}
	if (!nr_use)
		return -EINVAL;
	return 0;
}

void *ccc_crypto_init(struct ccc_crypto_req *req)
{
	struct ccc_crypto_context *context;
	bool *use;
	void **channel_contexts;
	struct channel_methods **channels_methods;
	enum ccc_channel_type type;
	int ret;

	if (!req)
		return NULL;
	if (req->control != CRYPTO_REQ_CONTROL_WORD)
		return NULL;
	if (req->scatter.control != SCATTER_CONTROL_WORD)
		return NULL;
	context = get_crypto_context();
	if (!context)
		return NULL;
	ret = check_request(req, context);
	if (ret) {
		TRACE_ERR(PREFIX "Incompatible operations requested %d\n", ret);
		goto error;
	}
	/*
	 * Don't care of requested channels availability. Therefore if more than
	 * one context need it then its busy state will be reported.
	 */
	use = &context->channel_uses[0];
	channel_contexts = &context->channel_contexts[0];
	channels_methods = &get_context()->channels_methods[0];
	for (type = NR_CHANNEL_TYPES; type--;) {
		if (use[type]) {
			channel_contexts[type] = channels_methods[type]->
				crypto_init(req, context);
			if (!channel_contexts[type])
				goto error;
		}
	}
#if defined(LITTLE_ENDIAN) && defined(DISABLE_HIF_IF_CH_EN_SWAP)
	if (use[HASH] && use[MOVE]) {
		/*
		 * When coupling MOVE and HASH channel, input data are not
		 * swapped by HIF interface as expected and therefore HASH
		 * output is wrong.
		 * Consequently, in this use-case, HIF_IFCR.CH_END must be reset
		 * so that the 32-bit word input is swapped by HIF. Output word
		 * swapping must be done by software.
		 *
		 * As HIF_IFCR.CH_END scope is the whole C3, other channels will
		 * misbehave.
		 */
		ccc_set_hif_ifcr_ch_end(&get_context()->controller, false);
		TRACE_INFO(PREFIX "HIF_IFCR.CH_END is reset.\n");
	}
#endif
	/*
	 * Chaining or coupling is not supported along with multi-block mode.
	 */
	if (context->multi_block_mode && context->link != none)
		goto error;
	return context;
error:
	put_crypto_context(context);
	return NULL;
}

int ccc_crypto_run(void *arg)
{
	struct ccc_crypto_context *context = (struct ccc_crypto_context *)arg;
	int ret;

	if (!arg)
		return -EINVAL;
	if (context->control != CRYPTO_CONTEXT_CONTROL_WORD)
		return -EINVAL;
	if (context->multi_block_mode) {
		TRACE_ERR(PREFIX "'run' not permitted\n");
		return -EPERM;
	}
	if (context->state != init) {
		TRACE_ERR(PREFIX "'run' before 'init' forbidden\n");
		return -EACCES;
	}
	context->state = finish;
	ret = perform_single_block_crypto(context);
	put_crypto_context(context);
	return ret;
}

int ccc_crypto_update(void *arg, struct ccc_scatter *scatter)
{
	struct ccc_crypto_context *context = (struct ccc_crypto_context *)arg;
	int ret = 0;

	if (!arg || !scatter)
		return -EINVAL;
	if (scatter->control != SCATTER_CONTROL_WORD)
		return -EINVAL;
	if (context->control != CRYPTO_CONTEXT_CONTROL_WORD)
		return -EINVAL;
	if (!context->multi_block_mode) {
		TRACE_ERR(PREFIX "'update' not permitted\n");
		ret = -EPERM;
		goto exit;
	}
	if (!is_in_place(scatter)) {
		TRACE_ERR(PREFIX "Data move not permitted during 'update'\n");
		ret = -EPERM;
		goto exit;
	}
	switch (context->state) {
	case free:
		TRACE_ERR(PREFIX "'update' before 'init' forbidden\n");
		ret = -EACCES;
		goto exit;
	case init:
		if (!context->dispatcher) {
			ret = -EFAULT;
			goto exit;
		}
		ret = perform_multi_block_crypto(context);
		if (ret)
			goto exit;
		context->state = update;
		break;
	case update:
		ret = attach_dispatcher(context);
		if (ret)
			goto exit;
		break;
	case finish:
		TRACE_ERR(PREFIX "'update' after 'finish' forbidden\n");
		ret = -EACCES;
		goto exit;
	default:
		ret = -EFAULT;
		goto exit;
	}
	context->scatter = scatter;
	ret = perform_multi_block_crypto(context);
	detach_dispatcher(context);
	/* Crypto context state remains 'update'. */
exit:
	if (ret && (ret != -EBUSY))
		put_crypto_context(context);
	return ret;
}

int ccc_crypto_finish(void *arg)
{
	struct ccc_crypto_context *context = (struct ccc_crypto_context *)arg;
	int ret;

	if (!arg)
		return -EINVAL;
	if (context->control != CRYPTO_CONTEXT_CONTROL_WORD)
		return -EINVAL;
	if (!context->multi_block_mode) {
		TRACE_ERR(PREFIX "'finish' not permitted\n");
		ret = -EPERM;
		goto exit;
	}
	if (context->state != update) {
		TRACE_ERR(PREFIX "'finish' is allowed only after 'update'\n");
		ret = -EACCES;
		goto exit;
	}
	ret = attach_dispatcher(context);
	if (ret)
		goto exit;
	context->state = finish;
	ret = perform_multi_block_crypto(context);
exit:
	if (ret != -EBUSY)
		put_crypto_context(context);
	return ret;
}

int init_dispatchers(struct ccc_controller *c3)
{
	int i;
	struct ccc_dispatcher *disp;
	unsigned int scr;

	c3->nr_dispatchers = NR_DISPATCHERS;
	c3->en_dispatchers = EN_DISPATCHERS;
	get_context()->programs = PROGRAMS;

	scr = ccc_read_sys_scr(c3);
	disp = &c3->dispatchers[0];
	for (i = 0; i < c3->nr_dispatchers; i++) {
		if (!ccc_is_indexed_dispatcher_present(scr, i))
			return -EINVAL;
		disp->base = c3->base + C3_ID(i);
		disp->program.addr = (unsigned int *)get_context()->programs;
		disp->program.addr += i * PROGRAM_SIZE_IN_BYTES;
		disp->program.size = PROGRAM_SIZE_IN_BYTES;
		disp->index = i;
		disp->pc = NULL;
		init_completion(&disp->execution, disp);
		disp->controller = c3;
		disp->crypto_context = NULL;
		disp++;
	}
	return 0;
}

#ifdef HAVE_INTERRUPT
static inline int irq_init(struct ccc_controller *controller)
{
	struct nvic_chnl *irq_chnl = &controller->irq_chnl;

	irq_chnl->id = C3_IRQ_ID;
	irq_chnl->preempt_prio = IRQ_LOW_PRIO;
	irq_chnl->enabled = true;

	return nvic_chnl_init(irq_chnl);
}

static inline void irq_deinit(struct ccc_controller *controller)
{
	nvic_chnl_disable(&controller->irq_chnl);
}
#endif

/*
 * Platform definitions might have been passed here instead of including
 * "sta_ccc_plat.h".
 */
int ccc_init(void)
{
	struct ccc_context *context = get_context();
	struct ccc_controller *controller = &context->controller;
	struct ccc_methods *methods = &context->methods;
	struct channel_methods **channels_methods = context->channels_methods;

#ifdef DEBUG
	unsigned int scr, ver, ifcr;
#endif
	int ret;

	/*
	 * This init function should be only once by the hardware setup.
	 * Nevertheless there is no need to use a semaphore because there is no
	 * concurrent access during init.
	 */
	if (controller->base == C3_BASE) {
		TRACE_ERR(PREFIX "Already inited\n");
		return -EEXIST;
	}
	controller->base = C3_BASE;

	create_sem(&get_context()->semaphore);
	give_sem(&get_context()->semaphore);
	create_sem(&controller->semaphore);
	give_sem(&controller->semaphore);
	methods->get_context = get_context;
	methods->get_crypto_context = get_crypto_context;
	methods->run_program = run_program;
	methods->put_crypto_context = put_crypto_context;
	methods->controller_reset = controller_reset;
	methods->channel_reset = channel_reset;
	ret = ccc_plat_set_topology(&plat_topology[0], NR_CHANNELS);
	if (ret)
		return ret;

#ifdef HAVE_INTERRUPT
	ret = irq_init(controller);
	if (ret)
		return ret;
#endif

	/* Requested c3_clk clock is enabled. */

	ret = init_dispatchers(controller);
	if (ret)
		return ret;

#ifdef DEBUG
	/*
	 * Endianness control bits meaning interpretation dumped below is
	 * subject to caution.
	 * Most C3 channels are natively big-endian. Therefore on little-endian
	 * platforms, channel io must be swapped by HIF interface.
	 */
	ver = ccc_read_sys_ver(controller);
	TRACE_INFO(PREFIX "Hardware Version: %d.%d.%d\n",
		   ccc_get_sys_ver_v(ver),
		   ccc_get_sys_ver_r(ver),
		   ccc_get_sys_ver_s(ver));
	scr = ccc_read_sys_scr(controller);
	TRACE_INFO(PREFIX "Clear Interrupts on SYS_SCR Read: %s\n",
		   ccc_get_sys_scr_cisr(scr) ? "yes" : "no");
	TRACE_INFO(PREFIX "SIF Endianness: %s\n",
		   ccc_get_sys_scr_endian(scr) ? "little" : "big");
	TRACE_INFO(PREFIX "%d Instruction Dispatcher%s enabled\n",
		   controller->nr_dispatchers,
		   controller->nr_dispatchers > 1 ? "s" : "");
	TRACE_INFO(PREFIX "HIF memory size: %dB\n",
		   ccc_read_hif_memory_size(controller));
	ifcr = ccc_read_hif_ifcr(controller);
	TRACE_INFO(PREFIX "HIF Instructions Dispatchers Endianness: %s\n",
		   ccc_get_hif_ifcr_id_end(ifcr) ? "little" : "big");
	TRACE_INFO(PREFIX "HIF Channels Endianness: %s\n",
		   ccc_get_hif_ifcr_ch_end(ifcr) ? "little" : "big");
#endif

	/* Initializations of implemented channels. */
#ifdef TRNG_INDEX
	trng_init(controller, TRNG_INDEX, &channels_methods[TRNG]);
#endif
#ifdef MPAES_INDEX
	mpaes_init(controller, MPAES_INDEX, &channels_methods[MPAES]);
#endif
#ifdef PKA_INDEX
	pka_init(controller, PKA_INDEX, &channels_methods[PKA]);
#endif
#ifdef MOVE_INDEX
	move_init(controller, MOVE_INDEX, &channels_methods[MOVE]);
#endif
#ifdef UH_INDEX
	hash_init(controller, UH_INDEX, &channels_methods[UH]);
#endif
#ifdef UH2_INDEX
	hash_init(controller, UH2_INDEX, &channels_methods[UH2]);
#endif
	init_crypto_contexts(controller);
	ret = ccc_plat_init();
	if (ret)
		return ret;
	return 0;
}

void ccc_deinit(void)
{
	struct ccc_controller *controller = &get_context()->controller;

	if (controller->base != C3_BASE)
		return;
	ccc_plat_deinit();
#ifdef HAVE_INTERRUPT
	irq_deinit(controller);
#endif
	controller_reset(controller);
	controller->base = 0;
}

#ifdef HAVE_HOOKS
__code_hook void *ccc_get_hooks(void)
{
	return &get_context()->methods;
}

__code_hook void *trng_get_hooks(void)
{
	return get_context()->channels_methods[TRNG];
}

__code_hook void *mpaes_get_hooks(void)
{
	return get_context()->channels_methods[MPAES];
}

__code_hook void *pka_get_hooks(void)
{
	return get_context()->channels_methods[PKA];
}

__code_hook void *move_get_hooks(void)
{
	return get_context()->channels_methods[MOVE];
}

__code_hook void *hash_get_hooks(void)
{
	return get_context()->channels_methods[HASH];
}
#endif
