/*
 * ST C3 Channel Controller v3.0
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2016 STMicroelectronics Limited
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef _CCC_H_
#define _CCC_H_

#define CCC_NAME_SIZE 30

struct ccc_controller {
	phys_addr_t physbase;
	void __iomem *base;
	int irq;
	struct clk *hclk;
	/* Prevent re-entrance of channel insertion and removal. */
	struct mutex lock;
	/* Handle registered channels. */
	unsigned short registrations;
	/* List of available instruction dispatchers. */
	struct ccc_dispatcher *dispatchers;
	unsigned char en_dispatchers;
	unsigned char nr_dispatchers;
	/*
	 * Availability of this controller. Controller is available if at least
	 * one of its instruction dispatchers is available.
	 */
	struct completion available;
	/* DMA pool for CCC programs. */
	struct dma_pool *pool;
	char name[CCC_NAME_SIZE];
	struct module *owner;
	struct platform_device *pdev;
	struct device dev;
};

#define to_ccc_controller(d) container_of(d, struct ccc_controller, dev)

#define CH_SCR 0x200

struct ccc_channel_id {
	char name[CCC_NAME_SIZE];
	int id;
};

struct ccc_channel {
	void __iomem *base;
	/* Position in host Controller. */
	int index;
	/* Read channel identifier. */
	int id;
	/* Reported errors. */
	unsigned int error_mask;
	/* Denote availability of this channel. */
	struct completion available;
	/* Host controller. */
	struct ccc_controller *controller;
	char name[CCC_NAME_SIZE];
	struct device dev;
	struct device_node *node;
	struct list_head probed;
};

#define to_ccc_channel(d) container_of(d, struct ccc_channel, dev)

struct ccc_channel_driver {
	int (*probe)(struct ccc_channel *);
	int (*remove)(struct ccc_channel *);
	struct device_driver driver;
	const struct ccc_channel_id *id_table;
	struct list_head channels;
};

#define to_ccc_channel_driver(d) \
	container_of(d, struct ccc_channel_driver, driver)

int ccc_register_controller(struct ccc_controller *);
void ccc_delete_controller(struct ccc_controller *);

struct ccc_dma {
	void *for_cpu;
	dma_addr_t for_dev;
	u32 size;
};

struct ccc_chunk {
	struct list_head entry;
	struct ccc_dma in;
	struct ccc_dma out;
};

/* Mnemonics */
#define MAX_NR_INSTRUCTIONS 16
#define MAX_INSTRUCTION_SIZE (5 * 4)
#define PROGRAM_SIZE (MAX_NR_INSTRUCTIONS * MAX_INSTRUCTION_SIZE)

#define MAX_PROGRAM_SIZE (63 * 1024)

#define MAX_NR_DISPATCHERS 4
#define C3_ID0 0x1000
#define C3_ID(i) (C3_ID0 + (i) * ID_SIZE)
#define ID_SCR 0x000
#define ID_IP 0x010
#define ID_SIZE 0x400

struct ccc_dispatcher {
	void __iomem *base;
	/* Position in host Controller. */
	int index;
	/* Program counter. */
	unsigned int *pc;
	/* Current space left for program. */
	unsigned int program_free_len;
	/* DMA region for program of this dispatcher. */
	struct ccc_dma program;
	/* Availability of this dispatcher. */
	struct completion available;
	/* Program execution state for this dispatcher. */
	struct completion execution;
	/* Host controller. */
	struct ccc_controller *controller;
};

#define BITS(v, h, l) (((v) & GENMASK(h, l)) >> l)

#define CHN_LSB 28
#define WN_LSB 26

#define STOP 0
#define NOP 3

struct operation {
	unsigned int code;
	unsigned char wn;
};

struct operation ccc_get_opcode(unsigned char, unsigned int, ...);
int ccc_check_channel_state(struct ccc_channel *);

#define BERR BIT(29)
#define DERR BIT(28)
#define PERR BIT(27)
#define IERR BIT(26)
#define AERR BIT(25)
#define OERR BIT(24)

#define SCR_BITS(v) BITS(v, 31, 30)
#define S_NOT_PRESENT 0
#define S_IDLE 2
#define S_ERROR 1
#define S_BUSY 3

static inline unsigned int ccc_get_channel_status(unsigned int scr)
{
	return SCR_BITS(scr);
}

#define CH_SCR 0x200
static inline unsigned int ccc_read_channel_scr(struct ccc_channel *c)
{
	return readl_relaxed(c->base + CH_SCR);
}

static inline unsigned int ccc_get_channel_scr_endian(unsigned int scr)
{
	return BITS(scr, 18, 18);
}

#define ENDIAN BIT(18)
static inline void ccc_set_channel_endianness(struct ccc_channel *c,
					      bool little)
{
	unsigned int scr;

	scr = readl_relaxed(c->base + CH_SCR);
	if (little)
		scr |= ENDIAN;
	else
		scr &= ~ENDIAN;

	writel_relaxed(scr, c->base + CH_SCR);
}

static inline bool ccc_is_channel_present(struct ccc_channel *c)
{
	unsigned int cs;

	cs = SCR_BITS(readl_relaxed(c->base + CH_SCR));
	return S_NOT_PRESENT != cs;
}

static inline unsigned int ccc_get_dispatcher_status(unsigned int scr)
{
	return SCR_BITS(scr);
}

static inline bool ccc_is_dispatcher_present(struct ccc_dispatcher *d)
{
	unsigned int ids;

	ids = SCR_BITS(readl_relaxed(d->base + ID_SCR));
	return S_NOT_PRESENT != ids;
}

static inline bool ccc_is_dispatcher_available(struct ccc_dispatcher *d)
{
	unsigned int ids;

	ids = SCR_BITS(readl_relaxed(d->base + ID_SCR));
	return (S_IDLE == ids) || (S_ERROR == ids);
}

static inline unsigned int ccc_read_id_scr(struct ccc_dispatcher *d)
{
	return readl_relaxed(d->base + ID_SCR);
}

#define IDS_MSB(n) (25 + 2 * (n))
#define IDS_LSB(n) (24 + 2 * (n))
static inline bool ccc_is_indexed_dispatcher_present(unsigned int scr,
						     unsigned char n)
{
	unsigned int ids;

	ids = BITS(scr, IDS_MSB(n), IDS_LSB(n));
	return S_NOT_PRESENT != ids;
}

static inline unsigned int ccc_get_sys_scr_cisr(unsigned int scr)
{
	return BITS(scr, 18, 18);
}

static inline unsigned int ccc_get_sys_scr_endian(unsigned int scr)
{
	return BITS(scr, 17, 17);
}

static inline unsigned int ccc_get_sys_scr_isdn(unsigned int scr)
{
	return BITS(scr, 23, 20);
}

#define C3_SYS 0x0000
#define SYS_SCR 0x000
static inline unsigned int ccc_read_sys_scr(struct ccc_controller *c3)
{
	return readl_relaxed(c3->base + C3_SYS + SYS_SCR);
}

static inline unsigned int ccc_get_sys_ver_v(unsigned int ver)
{
	return BITS(ver, 31, 24);
}

static inline unsigned int ccc_get_sys_ver_r(unsigned int ver)
{
	return BITS(ver, 23, 16);
}

static inline unsigned int ccc_get_sys_ver_s(unsigned int ver)
{
	return BITS(ver, 31, 24);
}

#define SYS_VER 0x3f0
static inline unsigned int ccc_read_sys_ver(struct ccc_controller *c3)
{
	return readl_relaxed(c3->base + C3_SYS + SYS_VER);
}

#define IS BIT(23)
static inline int ccc_is_interrupt_pending(struct ccc_dispatcher *d)
{
	return IS & readl_relaxed(d->base + ID_SCR);
}

static inline void ccc_clear_interrupt(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = readl_relaxed(d->base + ID_SCR);
	scr |= IS;
	writel_relaxed(scr, d->base + ID_SCR);
}

#define IES BIT(22)
#define IER BIT(21)
static inline void ccc_enable_interrupts(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = readl_relaxed(d->base + ID_SCR);
	scr |= IES | IER;
	writel_relaxed(scr, d->base + ID_SCR);
}

static inline void ccc_disable_interrupts(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = readl_relaxed(d->base + ID_SCR);
	scr &= ~(IES | IER);
	writel_relaxed(scr, d->base + ID_SCR);
}

static inline void ccc_set_instruction_pointer(struct ccc_dispatcher *d)
{
	writel_relaxed(d->program.for_dev, d->base + ID_IP);
}

#define SSE BIT(19)
static inline void ccc_enable_single_step_command(struct ccc_dispatcher *d)
{
	unsigned int scr;

	scr = readl_relaxed(d->base + ID_SCR);
	scr |= SSE;
	writel_relaxed(scr, d->base + ID_SCR);
}

#define C3_HIF 0x0400
#define HIF_MSIZE 0x300
static inline unsigned int ccc_read_hif_memory_size(struct ccc_controller *c3)
{
	return readl_relaxed(c3->base + C3_HIF + HIF_MSIZE);
}

#define HIF_MCR 0x308
#define EMM BIT(0)
static inline void ccc_disable_hif_memory(struct ccc_controller *c3)
{
	unsigned int mcr;

	mcr = readl_relaxed(c3->base + C3_HIF + HIF_MCR);
	mcr &= ~EMM;
	writel_relaxed(mcr, c3->base + C3_HIF + HIF_MCR);
}

#define HIF_IFCR 0x380
static inline unsigned int ccc_read_hif_ifcr(struct ccc_controller *c3)
{
	return readl_relaxed(c3->base + C3_HIF + HIF_IFCR);
}

#define ID_END BIT(0)
static inline unsigned int ccc_get_hif_ifcr_id_end(unsigned int ifcr)
{
	return ifcr & ID_END;
}

#define CH_END BIT(1)
static inline unsigned int ccc_get_hif_ifcr_ch_end(unsigned int ifcr)
{
	return ifcr & CH_END;
}

static inline void ccc_set_hif_endianness(struct ccc_controller *c3,
					  bool little)
{
	unsigned int ifcr;

	ifcr = readl_relaxed(c3->base + C3_HIF + HIF_IFCR);
	if (little)
		ifcr |= ID_END | CH_END;
	else
		ifcr &= ~(ID_END | CH_END);
	writel_relaxed(ifcr, c3->base + C3_HIF + HIF_IFCR);
}

static inline unsigned int ccc_get_program_free_len(struct ccc_dispatcher *d)
{
	return d->program_free_len;
}

struct ccc_dispatcher *ccc_get_dispatcher(struct ccc_controller *);
void ccc_put_dispatcher(struct ccc_dispatcher *);
void ccc_program(struct ccc_dispatcher *, unsigned int, unsigned char, ...);
int ccc_run_program(struct ccc_dispatcher *);

#define MAX_CHANNEL_INDEX 15
#define CH_SIZE 0x400
#define C3_CH0 0x2000
#define C3_CH(i) (C3_CH0 + (i) * CH_SIZE)
static inline phys_addr_t ccc_get_ch_physbase(struct ccc_controller *cont,
					      int index)
{
	return cont->physbase + C3_CH(index);
}

#define CH_ID 0x3fc
static inline int ccc_get_ch_id(void __iomem *channel_base)
{
	return readl_relaxed(channel_base + CH_ID);
}

static inline void ccc_set_channel_data(struct ccc_channel *channel, void *data)
{
	dev_set_drvdata(&channel->dev, data);
}

static inline void *ccc_get_channel_data(struct ccc_channel *channel)
{
	return dev_get_drvdata(&channel->dev);
}

int ccc_register_channel_driver(struct module *, struct ccc_channel_driver *);
void ccc_delete_channel_driver(struct ccc_channel_driver *);

static inline void ccc_init_channel(struct ccc_channel *channel)
{
	init_completion(&channel->available);
	complete(&channel->available);
}

struct ccc_channel *ccc_get_channel(struct ccc_channel_driver *);

static inline void ccc_put_channel(struct ccc_channel *channel)
{
	complete(&channel->available);
}

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CCC_ALIGN_SIZE MAX(ARCH_DMA_MINALIGN, __alignof__(u32))

static inline void ccc_dma_sync_for_device(struct device *dev,
					   struct ccc_dma *dma,
					   enum dma_data_direction dir)
{
	dma_sync_single_for_device(dev, dma->for_dev, dma->size, dir);
}

static inline void ccc_dma_sync_for_cpu(struct device *dev,
					struct ccc_dma *dma,
					enum dma_data_direction dir)
{
	dma_sync_single_for_cpu(dev, dma->for_dev, dma->size, dir);
}

struct ccc_step {
	struct page *page;
	unsigned int offset;
};

static inline int ccc_map_step(struct device *dev, struct ccc_step *s,
			       struct ccc_dma *dma, enum dma_data_direction dir)
{
	dma->for_dev = dma_map_page(dev, s->page, s->offset, dma->size, dir);
	if (dma_mapping_error(dev, dma->for_dev))
		return -ENOMEM;

	if (DMA_BIDIRECTIONAL == dir || DMA_TO_DEVICE == dir)
		ccc_dma_sync_for_device(dev, dma, dir);
	return 0;
}

static inline void ccc_unmap_step(struct device *dev, struct ccc_dma *dma,
				  enum dma_data_direction dir)
{
	if (DMA_BIDIRECTIONAL == dir || DMA_FROM_DEVICE == dir)
		ccc_dma_sync_for_cpu(dev, dma, dir);

	dma_unmap_page(dev, dma->for_dev, dma->size, dir);
}

#endif /* _CCC_H_ */
