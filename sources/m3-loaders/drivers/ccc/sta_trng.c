/**
 * @file sta_trng.c
 * @brief CCC TRNG channel driver.
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

#define DEV_NAME "trng"
#define PREFIX DEV_NAME ": "

struct trng_context {
	struct ccc_channel channel;
	struct ccc_crypto_context *parent;
	struct ccc_dma random;
};

static struct trng_context ctx = {{0},};

struct trng_data {
	const int *ids;
	const unsigned int nr_id;
	const bool pll_noise_source_support;
};

/* List all ids compatible with this driver. */
static const int ids[] = {0x0000f806, 0x0000f807, 0x0000f808};

/* List ids compatible with TRNGv1.6 */
static const int trng_v16_ids[] = {0x0000f806, 0x0000f807};

static struct trng_data trng_v16_data = {.ids = &trng_v16_ids[0],
					 .nr_id = ARRAY_SIZE(trng_v16_ids),
					 .pll_noise_source_support = false};

/* List ids compatible with TRNGv1.7 */
static const int trng_v17_ids[] = {0x0000f808};

static struct trng_data trng_v17_data = {.ids = &trng_v17_ids[0],
					 .nr_id = ARRAY_SIZE(trng_v17_ids),
					 .pll_noise_source_support = true};

static struct ccc_channel *trng_get_channel(void *arg)
{
	if (arg)
		return &((struct trng_context *)arg)->channel;

	return NULL;
}

#define TRNG_CSCR 0x018
#define DMA_REG (1 << 31)
static inline void set_reg_mode(struct trng_context *context)
{
	struct ccc_channel *channel = &context->channel;
	unsigned int status;

	status = read_reg(channel->base + TRNG_CSCR);
	status |= DMA_REG;
	write_reg(status, channel->base + TRNG_CSCR);
}

static inline void set_dma_mode(struct trng_context *context)
{
	struct ccc_channel *channel = &context->channel;
	unsigned int status;

	status = read_reg(channel->base + TRNG_CSCR);
	status &= ~DMA_REG;
	write_reg(status, channel->base + TRNG_CSCR);
}

static void *trng_crypto_init(struct ccc_crypto_req *req,
			      struct ccc_crypto_context *parent)
{
	struct trng_context *context = &ctx;

	context->parent = parent;

	if (req->scatter.nr_entries > 1) {
		TRACE_INFO(PREFIX "Scattered output not handled\n");
		return NULL;
	}
	if (!is_dst_32bits_aligned(&req->scatter)) {
		TRACE_INFO(PREFIX "Output not aligned on 32bits\n");
		return NULL;
	}
	context->random.addr = req->scatter.entries[0].dst;
	if (req->scatter.nr_bytes == 0) {
		TRACE_INFO(PREFIX "No data to produce\n");
		return NULL;
	}
	if (!is_size_32bits_aligned(&req->scatter)) {
		TRACE_INFO(PREFIX "Data size not multiple of 4B\n");
		return NULL;
	}
	context->random.size = req->scatter.nr_bytes;
	set_dma_mode(context);
	return context;
}

#define OP_ID_SHIFT 22
#define ENABLE 0xa
#define DISABLE 0xb
#define GET_VALUE 0x18
#define LENGTH_SHIFT 0
static struct operation trng_get_opcode(int index, unsigned char code,
					unsigned int nr_param, ...)
{
	struct operation op;
	va_list params;

	switch (code) {
	case ENABLE:
		ASSERT(nr_param == 0);
		op.code = code << OP_ID_SHIFT;
		op.wn = 0;
		break;
	case DISABLE:
		ASSERT(nr_param == 0);
		op.code = code << OP_ID_SHIFT;
		op.wn = 0;
		break;
	case GET_VALUE:
		ASSERT(nr_param == 1);
		op.code = code << OP_ID_SHIFT;
		op.wn = 1;
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

int trng_program_prologue(void *arg)
{
	struct trng_context *context = (struct trng_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	op = trng_get_opcode(channel->index, DISABLE, 0);
	ccc_program(dispatcher, op.code, op.wn);
	/*
	 * Wait as long as the low frequency domain of TRNG core can see the
	 * disable/enable sequence.
	 */
#define WAIT_CYCLES (1 + C3_CLOCK_MAX_FREQ / TRNG_CLOCK_MIN_FREQ)
	ASSERT(WAIT_CYCLES <= UINT16_MAX);
	op = ccc_get_wait_opcode(WAIT_CYCLES);
	ccc_program(dispatcher, op.code, op.wn);
	op = trng_get_opcode(channel->index, ENABLE, 0);
	ccc_program(dispatcher, op.code, op.wn);

	return 0;
}

int trng_program(void *arg)
{
	struct trng_context *context = (struct trng_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	op = trng_get_opcode(channel->index, GET_VALUE, 1,
			     context->random.size);
	ccc_program(dispatcher, op.code, op.wn, context->random.addr);

	return 0;
}

int trng_program_epilogue(void *arg)
{
	struct trng_context *context = (struct trng_context *)arg;
	struct ccc_channel *channel = &context->channel;
	struct ccc_dispatcher *dispatcher = ccc_get_dispatcher(context->parent);
	struct operation op;

	op = trng_get_opcode(channel->index, DISABLE, 0);
	ccc_program(dispatcher, op.code, op.wn);

	return 0;
}

static inline int trng_post_processing(__maybe_unused void *arg)
{
	return 0;
}

static inline struct trng_data *trng_get_channel_data(int id)
{
	int i;

	for (i = trng_v16_data.nr_id; i--;) {
		if (id == trng_v16_data.ids[i])
			return &trng_v16_data;
	}
	for (i = trng_v17_data.nr_id; i--;) {
		if (id == trng_v17_data.ids[i])
			return &trng_v17_data;
	}
	return NULL;
}

#define PLL_CR 0x60
#define PLL_CR_SEL_PLL_SRC (1 << 0)
#define PLL_CR_ENA_TRNG (1 << 1)
#define PLL_CR_PLL_PWRDOWN (1 << 2)
#define PLL_CR_CLEAR_PLL_UNLOCKED (1 << 7)
#define PLL_CR_KD_SHIFT 8
#define PLL_CR_KD_MASK 0xFF00
#define PLL_SR 0x64
#define PLL_SR_LOCKED (1 << 0)
#define CKPLL_CR 0x68
#define CKPLL_CR_INDIVFACT_SHIFT 0
#define CKPLL_CR_OUTDIVFACT_SHIFT 4
#define CKPLL_CR_LOOP_NDIV_SHIFT 12
#define CKPLL_CR_CHPUMP_SHIFT 24
#define CKPLL_CR_CHPUMP_MASK ((1 << 5) - 1)
static inline void set_ring_oscillator_as_noise_source(
	struct ccc_channel *channel)
{
	unsigned int base = channel->base;
	unsigned int pll_cr;

	pll_cr = read_reg(base + PLL_CR);
	pll_cr &= ~PLL_CR_SEL_PLL_SRC;
	write_reg(pll_cr, base + PLL_CR);
}

static inline void set_pll_as_noise_source(struct ccc_channel *channel)
{
	unsigned int base = channel->base;
	unsigned int pll_cr;

	pll_cr = read_reg(base + PLL_CR);
	pll_cr |= PLL_CR_SEL_PLL_SRC;
	write_reg(pll_cr, base + PLL_CR);
}

static inline int setup_pll_noise_source(struct ccc_channel *channel)
{
	unsigned int base = channel->base;
	unsigned int pll_cr, ckpll_cr;
	unsigned int loop_ndiv, charge_pump_current;

	pll_cr = read_reg(base + PLL_CR);
	ckpll_cr = read_reg(base + CKPLL_CR);
	pll_cr &= ~PLL_CR_KD_MASK;
	charge_pump_current = ccc_plat_get_trng_charge_pump_current();
	charge_pump_current &= CKPLL_CR_CHPUMP_MASK;
	ckpll_cr = charge_pump_current << CKPLL_CR_CHPUMP_SHIFT;
	switch (ccc_plat_get_trng_pll0_reference_clock()) {
	case 24:
		/* set PLL1 KD parameter */
		loop_ndiv = 199;
		pll_cr |= (loop_ndiv - 1) << PLL_CR_KD_SHIFT;
		/* set PLL0 parameters */
		/* default provided by AST=0x170C7086 */
		ckpll_cr |= 6 << CKPLL_CR_INDIVFACT_SHIFT;
		ckpll_cr |= 8 << CKPLL_CR_OUTDIVFACT_SHIFT;
		ckpll_cr |= loop_ndiv << CKPLL_CR_LOOP_NDIV_SHIFT;
		break;
	case 26:
		/* set PLL1 KD parameter */
		loop_ndiv = 181;
		pll_cr |= (loop_ndiv - 1) << PLL_CR_KD_SHIFT;
		/* set PLL0 parameters */
		/* default provided by AST=0x150B5086 */
		ckpll_cr |= 6 << CKPLL_CR_INDIVFACT_SHIFT;
		ckpll_cr |= 8 << CKPLL_CR_OUTDIVFACT_SHIFT;
		ckpll_cr |= loop_ndiv << CKPLL_CR_LOOP_NDIV_SHIFT;
		break;
	case 40:
		/* set PLL1 KD parameter */
		loop_ndiv = 139;
		pll_cr |= (loop_ndiv - 1) << PLL_CR_KD_SHIFT;
		/* set PLL0 parameters */
		/* default provided by AST=0x1008B087 */
		ckpll_cr |= 7 << CKPLL_CR_INDIVFACT_SHIFT;
		ckpll_cr |= 8 << CKPLL_CR_OUTDIVFACT_SHIFT;
		ckpll_cr |= loop_ndiv << CKPLL_CR_LOOP_NDIV_SHIFT;
		break;
	default:
		return -EINVAL;
	}
	write_reg(pll_cr, base + PLL_CR);
	write_reg(ckpll_cr, base + CKPLL_CR);
	udelay(10);
	pll_cr &= ~PLL_CR_PLL_PWRDOWN;
	write_reg(pll_cr, base + PLL_CR);
	unsigned int locked;

	do {
		locked = read_reg(base + PLL_SR) & PLL_SR_LOCKED;
	} while (!locked);
	/* Enable the PLL0 */
	pll_cr |= PLL_CR_ENA_TRNG;
	write_reg(pll_cr, base + PLL_CR);
	/* Clear a potential old PLL unlocked status */
	write_reg(pll_cr | PLL_CR_CLEAR_PLL_UNLOCKED, base + PLL_CR);
	return 0;
}

#define CSCR 0x18
#define CSCR_IF_MODE (1 << 31)
static inline void set_smart_dma_mode(struct ccc_channel *channel)
{
	unsigned int base = channel->base;
	unsigned int cscr;

	cscr = read_reg(base + CSCR);
	cscr &= ~CSCR_IF_MODE;
	write_reg(cscr, base + CSCR);
}

#define CSCR_EN_DIS (1 << 17)
static inline void set_register_mode(struct ccc_channel *channel)
{
	unsigned int base = channel->base;
	unsigned int cscr;
	unsigned int register_mode, disable;

	cscr = read_reg(base + CSCR);
	cscr |= CSCR_IF_MODE;
	write_reg(cscr, base + CSCR);
	do {
		register_mode = read_reg(base + CSCR) & CSCR_IF_MODE;
	} while (!register_mode);
	cscr |= CSCR_EN_DIS;
	write_reg(cscr, base + CSCR);
	do {
		disable = read_reg(base + CSCR) & CSCR_EN_DIS;
	} while (!disable);
	cscr &= ~CSCR_EN_DIS;
	write_reg(cscr, base + CSCR);
}

static inline int trng_configure(struct ccc_channel *channel)
{
	struct trng_data *data = channel->data;
	int ret = 0;

	if (!data->pll_noise_source_support)
		return 0;
	set_register_mode(channel);
	if (ccc_plat_get_trng_noise_source() == PLL) {
		set_pll_as_noise_source(channel);
		ret = setup_pll_noise_source(channel);
	} else {
		set_ring_oscillator_as_noise_source(channel);
	}
	set_smart_dma_mode(channel);
	return ret;
}

static struct channel_methods trng_methods = {
	.get_channel = trng_get_channel,
	.configure = trng_configure,
	.crypto_init = trng_crypto_init,
	.program_prologue = trng_program_prologue,
	.program = trng_program,
	.program_epilogue = trng_program_epilogue,
	.post_processing = trng_post_processing
};

int trng_init(struct ccc_controller *c3, int index,
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

	channel->error_mask = BERR | DERR | IERR | AERR | OERR;
	ccc_set_channel_name(channel, DEV_NAME);
	ccc_set_channel_data(channel, trng_get_channel_data(channel->id));
	ret = trng_configure(channel);
	if (ret)
		return ret;
	*methods = &trng_methods;
#ifdef DEBUG
	scr = ccc_read_channel_scr(channel);
	TRACE_INFO(PREFIX "Endianness: %s\n",
		   ccc_get_channel_scr_endian(scr) ?
		   "little/swap" : "big/no swap");
#endif
	return 0;
}
