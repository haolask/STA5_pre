/**
 * @file sta_ccc.h
 * @brief CCC driver private header.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _CCC_H_
#define _CCC_H_

struct ccc_dma {
	void *addr;
	unsigned int size;
};

#ifndef HAVE_INTERRUPT
#define completion_t struct ccc_dispatcher *
#endif

struct ccc_dispatcher {
	uint32_t base;
	int index;
	unsigned int *pc;
	struct ccc_dma program;
	completion_t execution;
	struct ccc_controller *controller;
	struct ccc_crypto_context *crypto_context;
};

struct ccc_controller {
	uint32_t base;
#ifdef HAVE_INTERRUPT
	struct nvic_chnl irq_chnl;
#endif
	struct ccc_dispatcher dispatchers[NR_DISPATCHERS];
	unsigned char en_dispatchers;
	unsigned char nr_dispatchers;
	semaphore_t semaphore;
};

#define CCC_NAME_SIZE 30

struct ccc_channel {
	unsigned int base;
	int index;
	int id;
	unsigned int error_mask;
	void *data;
	struct ccc_controller *controller;
	char name[CCC_NAME_SIZE];
};

struct ccc_methods {
	struct ccc_context *(*get_context)(void);
	struct ccc_crypto_context *(*get_crypto_context)(void);
	int (*run_program)(struct ccc_dispatcher *dispatcher);
	void (*put_crypto_context)(struct ccc_crypto_context *context);
	void (*controller_reset)(struct ccc_controller *controller);
	void (*channel_reset)(struct ccc_channel *channel);
};

struct channel_methods {
	struct ccc_channel *(*get_channel)(void *);
	int (*configure)(struct ccc_channel *);
	void *(*crypto_init)(struct ccc_crypto_req *req,
			     struct ccc_crypto_context *parent);
	int (*program_prologue)(void *);
	int (*program)(void *);
	int (*program_epilogue)(void *);
	int (*post_processing)(void *);
};

struct ccc_chunk {
	struct ccc_dma in;
	struct ccc_dma out;
};

enum ccc_crypto_state {
	free,
	init,
	update,
	finish
};

/*
 * For the time being, only UH and UH2 support multi-block mode and rely on
 * a save/restore mechanism of hardware contexts to support concurrent accesses.
 */
#define NR_UH_CRYPTO_CONTEXTS NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL
#define NR_UH2_CRYPTO_CONTEXTS NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL
#define NR_HASH_CRYPTO_CONTEXTS (NR_HASH_CHANNELS *			\
				 NR_HASH_CRYPTO_CONTEXTS_PER_CHANNEL)
#define NR_CRYPTO_CONTEXTS (NR_DISPATCHERS + NR_HASH_CRYPTO_CONTEXTS)

enum ccm_link {
	none,
	_ =  none,
	couple,
	o = couple,
	chain,
	c = chain,
	ignore,
	x = ignore
};

#define CRYPTO_CONTEXT_CONTROL_WORD 0x61706548
struct ccc_crypto_context {
	unsigned int control;
	struct ccc_dispatcher *dispatcher;
	enum ccc_crypto_state state;
	bool multi_block_mode;
	enum ccm_link link;
	struct ccc_scatter *scatter;
	bool channel_uses[NR_CHANNEL_TYPES];
	void *channel_contexts[NR_CHANNEL_TYPES];
	struct ccc_controller *controller;
};

#define THIRTY_TWO_BITS_ALIGNED(a) (((a) & (32 / 8 - 1)) == 0)

#define MIN_NR_INSTRUCTIONS 16
#define BIGGEST_INSTRUCTION_SIZE (5 * 4)
#define MIN_PROGRAM_SIZE_IN_BYTES (MIN_NR_INSTRUCTIONS * \
				   BIGGEST_INSTRUCTION_SIZE)
#if PROGRAM_SIZE_IN_BYTES < MIN_PROGRAM_SIZE_IN_BYTES
#error Reserved space for microprogram is too small
#endif

#define MAX_PROGRAM_SIZE (63 * 1024)
#if PROGRAM_SIZE_IN_BYTES > MAX_PROGRAM_SIZE
#error Reserved space for microprogram is too big
#endif

/*
 * MPAES maximum payload is the smallest one of all channels maximum payloads.
 * Unit is the byte.
 */
#define MAX_CHUNK_SIZE 65520

#define MAX_NR_DISPATCHERS 4
#define C3_ID0 0x1000
#define C3_ID(i) (C3_ID0 + i * ID_SIZE)
#define ID_SCR 0x000
#define ID_IP 0x010
#define ID_SIZE 0x400

#define CHN_SHIFT 28
#define WN_SHIFT 26

struct operation {
	unsigned int code;
	unsigned char wn;
};

void ccc_set_channel_name(struct ccc_channel *, const char *);
bool ccc_is_channel_supported(int, const int*, unsigned int);

#define BERR (1 << 29)
#define DERR (1 << 28)
#define PERR (1 << 27)
#define IERR (1 << 26)
#define AERR (1 << 25)
#define OERR (1 << 24)

#define S_SHIFT 30
#define S_MASK 3
#define S_NOT_PRESENT 0
#define S_IDLE 2
#define S_ERROR 1
#define S_BUSY 3
static inline unsigned int ccc_get_channel_status(unsigned int scr)
{
	return S_MASK & (scr >> S_SHIFT);
}

#define CH_SCR 0x200
static inline unsigned int ccc_read_channel_scr(struct ccc_channel *c)
{
	return read_reg(c->base + CH_SCR);
}

#define CH_SCR_ENDIAN_SHIFT 18
#define CH_SCR_ENDIAN_MASK 1
static inline unsigned int ccc_get_channel_scr_endian(unsigned int scr)
{
	return CH_SCR_ENDIAN_MASK & (scr >> CH_SCR_ENDIAN_SHIFT);
}

static inline void ccc_set_channel_scr_endian(struct ccc_channel *c,
					      bool swap)
{
	unsigned int scr;

	scr = read_reg(c->base + CH_SCR);
	if (swap)
		scr |= 1 << CH_SCR_ENDIAN_SHIFT;
	else
		scr &= ~(1 << CH_SCR_ENDIAN_SHIFT);

	write_reg(scr, c->base + CH_SCR);
}

static inline unsigned int ccc_is_channel_present(struct ccc_channel *c)
{
	unsigned int cs;

	cs = S_MASK & (read_reg(c->base + CH_SCR) >> S_SHIFT);

	return S_NOT_PRESENT != cs;
}

static inline unsigned int ccc_get_dispatcher_status(unsigned int scr)
{
	return S_MASK & (scr >> S_SHIFT);
}

static inline unsigned int ccc_read_id_scr(struct ccc_dispatcher *d)
{
	return read_reg(d->base + ID_SCR);
}

#define IDS_SHIFT(n) (24 + 2 * n)
#define IDS_MASK 3
static inline bool ccc_is_indexed_dispatcher_present(unsigned int scr,
						     unsigned char n)
{
	unsigned int ids;

	ids = IDS_MASK & (scr >> IDS_SHIFT(n));
	ids &= IDS_MASK;

	return S_NOT_PRESENT != ids;
}

#define CISR_SHIFT 18
#define CISR_MASK 1
static inline unsigned int ccc_get_sys_scr_cisr(unsigned int scr)
{
	return CISR_MASK & (scr >> CISR_SHIFT);
}

#define SYS_SCR_ENDIAN_SHIFT 17
#define SYS_SCR_ENDIAN_MASK 1
static inline unsigned int ccc_get_sys_scr_endian(unsigned int scr)
{
	return SYS_SCR_ENDIAN_MASK & (scr >> SYS_SCR_ENDIAN_SHIFT);
}

#define SYS_SCR_ISDN_SHIFT 20
#define SYS_SCR_ISDN_MASK 0xf
static inline unsigned int ccc_get_sys_scr_isdn(unsigned int scr)
{
	return SYS_SCR_ISDN_MASK & (scr >> SYS_SCR_ISDN_SHIFT);
}

#define C3_SYS 0x0000
#define SYS_SCR 0x000
static inline unsigned int ccc_read_sys_scr(struct ccc_controller *c3)
{
	return read_reg(c3->base + C3_SYS + SYS_SCR);
}

#define VER_V_SHIFT 24
#define VER_V_MASK 0xff
static inline unsigned int ccc_get_sys_ver_v(unsigned int ver)
{
	return VER_V_MASK & (ver >> VER_V_SHIFT);
}

#define VER_R_SHIFT 16
#define VER_R_MASK 0xff
static inline unsigned int ccc_get_sys_ver_r(unsigned int ver)
{
	return VER_R_MASK & (ver >> VER_R_SHIFT);
}

#define VER_S_SHIFT 0
#define VER_S_MASK 0xffff
static inline unsigned int ccc_get_sys_ver_s(unsigned int ver)
{
	return VER_S_MASK & (ver >> VER_S_SHIFT);
}

#define SYS_VER 0x3f0
static inline unsigned int ccc_read_sys_ver(struct ccc_controller *c3)
{
	return read_reg(c3->base + C3_SYS + SYS_VER);
}

#define IS_SHIFT 23
#define IS_MASK 1
static inline int ccc_is_interrupt_pending(struct ccc_dispatcher *d)
{
	return IS_MASK & (read_reg(d->base + ID_SCR) >> IS_SHIFT);
}

static inline void ccc_clear_interrupt(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = read_reg(d->base + ID_SCR);
	scr |= 1 << IS_SHIFT;

	write_reg(scr, d->base + ID_SCR);
}

#define IES (1 << 22)
#define IER (1 << 21)
static inline void ccc_enable_interrupts(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = read_reg(d->base + ID_SCR);
	scr |= IES | IER;

	write_reg(scr, d->base + ID_SCR);
}

static inline void ccc_disable_interrupts(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = read_reg(d->base + ID_SCR);
	scr &= ~(IES | IER);

	write_reg(scr, d->base + ID_SCR);
}

static inline void ccc_set_instruction_pointer(struct ccc_dispatcher *d)
{
	write_reg((uint32_t)(d->program.addr), d->base + ID_IP);
}

#define SSE (1 << 19)
static inline void ccc_enable_single_step_command(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = read_reg(d->base + ID_SCR);
	scr |= SSE;

	write_reg(scr, d->base + ID_SCR);
}

#define C3_HIF 0x0400
#define HIF_MSIZE 0x300
static inline unsigned int ccc_read_hif_memory_size(struct ccc_controller *c3)
{
	return read_reg(c3->base + C3_HIF + HIF_MSIZE);
}

#define HIF_NBAR 0x344
static inline void ccc_set_hif_nbar(struct ccc_controller *c3,
				    unsigned int nbar)
{
	write_reg(nbar, c3->base + C3_HIF + HIF_NBAR);
}

#define HIF_NCR 0x348
static inline void ccc_set_hif_ncr(struct ccc_controller *c3, unsigned int ncr)
{
	write_reg(ncr, c3->base + C3_HIF + HIF_NCR);
}

#define HIF_IFCR 0x380
static inline unsigned int ccc_read_hif_ifcr(struct ccc_controller *c3)
{
	return read_reg(c3->base + C3_HIF + HIF_IFCR);
}

#define ID_END (1 << 0)
static inline unsigned int ccc_get_hif_ifcr_id_end(unsigned int ifcr)
{
	return ifcr & ID_END;
}

static inline void ccc_set_hif_ifcr_id_end(struct ccc_controller *c3, bool swap)
{
	unsigned int ifcr;

	ifcr = read_reg(c3->base + C3_HIF + HIF_IFCR);
	if (swap)
		ifcr |= ID_END;
	else
		ifcr &= ~ID_END;

	write_reg(ifcr, c3->base + C3_HIF + HIF_IFCR);
}

#define CH_END (1 << 1)
static inline unsigned int ccc_get_hif_ifcr_ch_end(unsigned int ifcr)
{
	return ifcr & CH_END;
}

static inline void ccc_set_hif_ifcr_ch_end(struct ccc_controller *c3, bool swap)
{
	unsigned int ifcr;

	ifcr = read_reg(c3->base + C3_HIF + HIF_IFCR);
	if (swap)
		ifcr |= CH_END;
	else
		ifcr &= ~CH_END;

	write_reg(ifcr, c3->base + C3_HIF + HIF_IFCR);
}

static inline int ccc_get_program_epilog_len(void)
{
/* Biggest epilog size is the size of STOP instruction with parameter. */
	return 2 * sizeof(unsigned int);
}

static inline unsigned int ccc_get_program_free_len(struct ccc_dispatcher *d)
{
	return (unsigned int)(d->program.addr)
		+ PROGRAM_SIZE_IN_BYTES
		- (unsigned int)(d->pc);
}

void ccc_program(struct ccc_dispatcher *, unsigned int, unsigned char, ...);

#define MAX_CHANNEL_INDEX 15
#define CH_SIZE 0x400
#define C3_CH0 0x2000
#define C3_CH(i) (C3_CH0 + i * CH_SIZE)
static inline uint32_t ccc_get_ch_physbase(struct ccc_controller *cont,
					   int index)
{
	return cont->base + C3_CH(index);
}

#define CH_ID 0x3fc
static inline int ccc_get_ch_id(uint32_t channel_base)
{
	return read_reg(channel_base + CH_ID);
}

static inline void ccc_set_channel_data(struct ccc_channel *channel, void *data)
{
	channel->data = data;
}

static inline void *ccc_get_channel_data(struct ccc_channel *channel)
{
	return channel->data;
}

static inline void ccc_dma_sync_for_device(void)
{
	__DSB();
}

static inline unsigned int get_byte_dimension(unsigned char byte)
{
	unsigned int nr = 8;

	if (!byte)
		return 0;
	do {
		if (byte & (1 << (nr - 1)))
			return nr;
	} while (--nr > 0);

	return 0;
}

static inline void swap_bytes(unsigned char *bytes, unsigned int nr_words)
{
	unsigned int i;
	unsigned int swapped;

	for (i = 0; i < nr_words; i++) {
		swapped = *(bytes + 3) << 0;
		swapped += *(bytes + 2) << 8;
		swapped += *(bytes + 1) << 16;
		swapped += *(bytes + 0) << 24;
		*(unsigned int *)bytes = swapped;
		bytes += sizeof(unsigned int);
	}
}

static inline void swap_bytes_if(unsigned char *bytes,
				 unsigned int nr_words)
{
#ifdef LITTLE_ENDIAN
	swap_bytes(bytes, nr_words);
#else
	bytes = bytes;
	nr_words = nr_words;
#endif
}

static inline struct ccc_scatter *ccc_get_scatter(
	struct ccc_crypto_context *context)
{
	return context->scatter;
}

#define SCATTER_CONTROL_WORD 0x13df5c46
static inline void ccc_set_scatter(struct ccc_crypto_context *context,
				   struct ccc_scatter *scatter)
{
	ASSERT(scatter->control == SCATTER_CONTROL_WORD);
	context->scatter = scatter;
}

static inline struct ccc_dispatcher *ccc_get_dispatcher(
	struct ccc_crypto_context *context)
{
	return context->dispatcher;
}

static inline enum ccc_crypto_state ccc_get_state(
	struct ccc_crypto_context *context)
{
	return context->state;
}

static inline bool ccc_is_multi_block_mode(struct ccc_crypto_context *context)
{
	return context->multi_block_mode;
}

static inline void ccc_set_multi_block_mode(struct ccc_crypto_context *context)
{
	context->multi_block_mode = true;
}

static inline bool ccc_is_linking(struct ccc_crypto_context *context)
{
	return context->link != none;
}

bool is_in_place(struct ccc_scatter *scatter);
bool is_src_32bits_aligned(struct ccc_scatter *scatter);
bool is_dst_32bits_aligned(struct ccc_scatter *scatter);
bool is_size_32bits_aligned(struct ccc_scatter *scatter);
bool is_size_null(struct ccc_scatter *scatter);
bool is_size_multiple_of(struct ccc_scatter *scatter, unsigned int modulus);
bool is_src_dst_overlap(struct ccc_scatter *scatter);
bool is_src_null(struct ccc_scatter *scatter);
bool ccc_prepare_chunks(struct ccc_chunk *chunk, void *arg,
			struct ccc_scatter *scatter, unsigned int *nr_chunks);
struct operation ccc_get_wait_opcode(unsigned short);

#define AES_MIN_KEY_SIZE 16
#define AES_MAX_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

int trng_init(struct ccc_controller *controller, int index,
	      struct channel_methods **methods);

int mpaes_init(struct ccc_controller *controller, int index,
	       struct channel_methods **methods);

int pka_init(struct ccc_controller *controller, int index,
	     struct channel_methods **methods);

int move_init(struct ccc_controller *controller, int index,
	      struct channel_methods **methods);

int hash_init(struct ccc_controller *controller, int index,
	      struct channel_methods **methods);
bool hash_is_alg_supported(unsigned int alg);

#ifdef HAVE_HOOKS
/*
 * This header file should not be included outside the driver unless for
 * patching purposes.
 */
void *ccc_get_hooks(void);
#endif
#endif /* _CCC_H_ */
