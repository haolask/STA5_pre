/*
 * Copyright (C) ST Microelectronics 2018
 *
 * Author:
 *		GianAntonio Sampietro <gianantonio.sampietro@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */
#ifndef CODEC_DSP_H
#define CODEC_DSP_H

#include <linux/kernel.h>
#include <linux/regmap.h>
#include <linux/vmalloc.h>

#define NUM_DSP 3

#define ST_CODEC_DSP_FW_SUBPATH "st/dspfw/"

struct dsp_data {
	int core;
	void __iomem *dsp_addr;
	void __iomem *dsp_dpmem;
#ifdef CONFIG_PM_SLEEP
	u32 reg_abc;
	void *dsp_p, *dsp_x, *dsp_y;
#endif
};

#define PRAM 0x0
#define XRAM 0x80000
#define YRAM 0xC0000

#define XIN(core)	(((core) == 2) ? 0x8c000 : 0x84000)
#define XOUT(core)	(((core) == 2) ? 0x8c200 : 0x84200)
#define PRAM_SIZE(core)	(((core) == 2) ? 0x8000 : 0x6000)
#define XRAM_SIZE(core)	(((core) == 2) ? 0x20000 : 0x4000)
#define XRAM_PAGE_SIZE(core)	0x4000
#define XRAM_NUM_PAGES(core)	(((core) == 2) ? 8 : 1)
#define REG_A_OFFSET	0x8C410
#define REG_B_OFFSET	0x8C414
#define REG_C_OFFSET	0x8C418
#define REG_ABC_OFFSET	0x8C41C
#define XRAM_SELECT_PAGE(core, base, page) ({ \
	if ((core) == 2) \
		writel(page, (base) + REG_A_OFFSET); \
})
#define YRAM_SIZE(core)	(((u32)(core) == 2) ? 0x10000 : 0x4000)
#define XIN_SIZE	512
#define XOUT_SIZE	512

#define AUSS_SSY_CR		0x100
#define SSY_EMECLKEN		0x6
#define SSY_EMERCORERESET	0x3

static inline u32 _dsp_readl(const void *addr)
{
	return readl(addr);
}

/* write a word (u32) to DSP followed by a read so to "ensure" a valid write */
static inline int _dsp_writel(void *addr, u32 value)
{
	writel(value, addr);
	return readl(addr);
}

static inline int _dsp_memset(u32 *b2, u32 val, int len)
{
	int i;

	for (i = 0; i < len / 4; i++)
		_dsp_writel(b2 + i, val);

	return 0;
}

static inline void _dsp_xin_dmabus_clear(struct dsp_data *dsp_data)
{
	if (dsp_data->dsp_dpmem)
		_dsp_memset((u32 *)dsp_data->dsp_dpmem,	0, XIN_SIZE);
}

static inline void _dsp_xout_dmabus_clear(struct dsp_data *dsp_data)
{
	if (dsp_data->dsp_dpmem)
		_dsp_memset((u32 *)(dsp_data->dsp_dpmem + XIN_SIZE),
			    0, XOUT_SIZE);
}

static inline int _dsp_clken(struct regmap *auss_regmap, int core)
{
	return regmap_update_bits(auss_regmap, AUSS_SSY_CR,
				  BIT(SSY_EMECLKEN + core),
				  BIT(SSY_EMECLKEN + core));
}

static inline int _dsp_clkdis(struct regmap *auss_regmap, int core)
{
	return regmap_update_bits(auss_regmap, AUSS_SSY_CR,
				  BIT(SSY_EMECLKEN + core),
				  0);
}

static inline int _dsp_clk_is_en(struct regmap *auss_regmap, int core)
{
	u32 ssy;

	regmap_read(auss_regmap, AUSS_SSY_CR, &ssy);
	return !!(ssy & BIT(SSY_EMECLKEN + core));
}

static inline int _dsp_start(struct regmap *auss_regmap, int core)
{
	return regmap_update_bits(auss_regmap, AUSS_SSY_CR,
				  BIT(SSY_EMERCORERESET + core),
				  BIT(SSY_EMERCORERESET + core));
}

static inline int _dsp_stop(struct regmap *auss_regmap, int core)
{
	return regmap_update_bits(auss_regmap, AUSS_SSY_CR,
				  BIT(SSY_EMERCORERESET + core),
				  0);
}

static inline int _dsp_load(u32 *b2, u32 *b1, int len)
{
	int i;

	for (i = 0; i < len / 4; i++)
		_dsp_writel(b2 + i, b1[i]);

	return 0;
}

static inline void _dsp_save(u32 *b2, u32 *b1, int len)
{
	int i;

	for (i = 0; i < len / 4; i++)
		b2[i] = b1[i];
}

static inline int _dsp_alloc(struct dsp_data *dsp_data)
{
	int core = dsp_data->core;

	dsp_data->dsp_p = vzalloc(PRAM_SIZE(core));
	if (!dsp_data->dsp_p)
		return -ENOMEM;
	dsp_data->dsp_x = vzalloc(XRAM_SIZE(core));
	if (!dsp_data->dsp_x) {
		vfree(dsp_data->dsp_p);
		return -ENOMEM;
	}
	dsp_data->dsp_y = vzalloc(YRAM_SIZE(core));
	if (!dsp_data->dsp_y) {
		vfree(dsp_data->dsp_p);
		vfree(dsp_data->dsp_x);
		return -ENOMEM;
	}

	return 0;
}

static inline void _dsp_free(struct dsp_data *dsp_data)
{
	vfree(dsp_data->dsp_p);
	vfree(dsp_data->dsp_x);
	vfree(dsp_data->dsp_y);
}

static inline void _dsp_suspend(struct dsp_data *dsp_data,
				struct regmap *auss_regmap)
{
	int page, core;
	void __iomem *dspaddr;

	dspaddr = dsp_data->dsp_addr;
	core = dsp_data->core;

	_dsp_stop(auss_regmap, core);

	if (core == 2)
		dsp_data->reg_abc =
		    readl(dsp_data->dsp_addr + REG_ABC_OFFSET);

	_dsp_save(dsp_data->dsp_p,
		  (u32 *)(dspaddr + PRAM), PRAM_SIZE(core));
	_dsp_save(dsp_data->dsp_y,
		  (u32 *)(dspaddr + YRAM), YRAM_SIZE(core));
	for (page = 0; page < XRAM_NUM_PAGES(core); page++) {
		XRAM_SELECT_PAGE(core, dspaddr, page);
		_dsp_save(dsp_data->dsp_x +
			   page * XRAM_PAGE_SIZE(core),
			 (u32 *)(dspaddr + XRAM), XRAM_PAGE_SIZE(core));
	}
	_dsp_clkdis(auss_regmap, core);
}

static inline void _dsp_resume(struct dsp_data *dsp_data,
			       struct regmap *auss_regmap)
{
	int page, core;
	void __iomem *dspaddr;

	dspaddr = dsp_data->dsp_addr;
	core = dsp_data->core;

	_dsp_clken(auss_regmap, core);

	_dsp_load((u32 *)(dspaddr + PRAM),
		  dsp_data->dsp_p, PRAM_SIZE(core));
	_dsp_load((u32 *)(dspaddr + YRAM),
		  dsp_data->dsp_y, YRAM_SIZE(core));
	for (page = 0; page < XRAM_NUM_PAGES(core); page++) {
		XRAM_SELECT_PAGE(core, dspaddr, page);
		_dsp_load((u32 *)(dspaddr + XRAM),
			  dsp_data->dsp_x +
			   (page * XRAM_PAGE_SIZE(core)),
			 XRAM_PAGE_SIZE(core));
	}

	if (dsp_data->core == 2)
		writel(dsp_data->reg_abc,
		       dsp_data->dsp_addr + REG_ABC_OFFSET);

	/* Clear XIN/XOUT */
	_dsp_memset((u32 *)(dspaddr + XIN(core)), 0x0, XIN_SIZE);
	_dsp_memset((u32 *)(dspaddr + XOUT(core)), 0x0, XOUT_SIZE);
	_dsp_xin_dmabus_clear(dsp_data);

	_dsp_start(auss_regmap, core);
}

#endif
