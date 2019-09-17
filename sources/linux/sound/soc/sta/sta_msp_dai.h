/*
 * Copyright (C) STMicroelectronics 2016
 *
 * Authors:	Gabriele Simone <gabriele.simone@st.com>,
 *		Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef STA_MSP_DAI_H
#define STA_MSP_DAI_H

/* Autorate defines */
#define MAX_THR 10
#define MSP_LEV_UNDEF (-1)

enum clk_sel_enum {
	CLK_SEL_INTERNAL,
	CLK_SEL_EXTERNAL,
};

enum i2s_direction_t {
	MSP_DIR_TX = 0x01,
	MSP_DIR_RX = 0x02,
};

struct msp_drvdata {
	struct device *dev;
	int id;
	const char *prefix;
	/* DMA */
	struct snd_dmaengine_dai_dma_data playback_dma_data;
	struct snd_dmaengine_dai_dma_data capture_dma_data;
	/* Resources */
	phys_addr_t phys;
	void __iomem *registers;
	struct regmap *auss_regmap;
	/* Clocks */
	struct clk *mspclk;
	struct clk *mspsck;
	struct clk *pclk;
	struct clk *genclk;
	enum clk_sel_enum clk_sel;
	int forced_clock;
	int fixed_clock;
	int autorate, cur_thr, target, num_thr, rate_delta;
	int thr[MAX_THR];
	u32 msp_extrefclk_mask;
	int extref_bclk;
	/* MSP settings */
	struct msp_protdesc *protdesc;
	unsigned int rx_fifo_config;
	unsigned int tx_fifo_config;
	unsigned int iodelay;
	enum msp_ms_mode ms_mode;
	unsigned int loopback_enable;
	unsigned int dir_busy;
	unsigned int msp_run;
	u32 irq_status;
	u32 irq_errors;
	unsigned int max_channels;
	unsigned int data_delay;
	unsigned int frame_freq;
	unsigned int tdm_mask[4];
	unsigned int m3_busy;
	struct completion m3_wait;
	struct snd_pcm_substream *substream_tx, *substream_rx;
	struct work_struct task;
	struct workqueue_struct *workqueue;
	struct hrtimer hr_timer;
	int hr_timeout;
#ifdef CONFIG_PM_SLEEP
	u32 save_area[32];
#endif
};

struct msp_params {
	u32 elem_len_1, frame_len_1, elem_len_2, frame_len_2, half_word_swap;
	u32 frame_period, frame_width;
};

#define SOC_RANGE(xname, range_info, xhandler_get, xhandler_put) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = range_info, \
	.get = xhandler_get, .put = xhandler_put, \
}

static int msp_dai_clk_enable(struct device *dev);
static void setup_clkpol(struct msp_drvdata *msp);
static int setup_clkgen(struct msp_drvdata *msp);
#endif
