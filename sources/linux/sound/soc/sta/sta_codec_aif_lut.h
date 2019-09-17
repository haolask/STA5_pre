/*
 * Copyright (C) STMicroelectronics 2016
 *
 * Authors:	Gabriele Simone <gabriele.simone@st.com>,
 *		Gian Antonio Sampietro <gianantonio.sampietro@st.com>
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef ST_CODEC_AIF_LUT_H
#define ST_CODEC_AIF_LUT_H

#include "sta_codec_aif.h"

struct dts_lut_val {
	char *name;
	u32 value;
};

static const struct dts_lut_val tm_mode_dts_lut[] = {
	{"normal", AIF_SAI_NORMAL},
	{"loopback", AIF_SAI_TESTMODE},
};

static const struct dts_lut_val frame_syn_dts_lut[] = {
	{"half-frame", AIF_SAI_SIN_HALFFRAME},
	{"first-word", AIF_SAI_SIN_FIRSTWORD},
	{"first-bit", AIF_SAI_SIN_FIRSTBIT},
};

static const struct dts_lut_val word_count_dts_lut[] = {
	{"2-words", AIF_SAI_CNT_2W},
	{"4-words", AIF_SAI_CNT_4W},
	{"6-words", AIF_SAI_CNT_6W},
	{"8-words", AIF_SAI_CNT_8W},
};

static const struct dts_lut_val word_adj_dts_lut[] = {
	{"left", AIF_SAI_ADJ_LEFT},
	{"right", AIF_SAI_ADJ_RIGHT},
};

static const struct dts_lut_val rel_timing_dts_lut[] = {
	{"first-bit", AIF_SAI_REL_FIRSTBIT},
	{"i2s", AIF_SAI_REL_I2S},
};

static const struct dts_lut_val clk_pol_dts_lut[] = {
	{"negative", AIF_SAI_CKP_NEGATIVE},
	{"positive", AIF_SAI_CKP_POSITIVE},
};

static const struct dts_lut_val lr_pol_dts_lut[] = {
	{"left", AIF_SAI_LRP_LEFT},
	{"right", AIF_SAI_LRP_RIGHT},
};

static const struct dts_lut_val data_shift_dir_dts_lut[] = {
	{"msb", AIF_SAI_DIR_MSB},
	{"lsb", AIF_SAI_DIR_LSB},
};

static const struct dts_lut_val word_length_dts_lut[] = {
	{"wl-16-16", AIF_SAI_WL_16_16},
	{"wl-24-16", AIF_SAI_WL_24_16},
	{"wl-24-24", AIF_SAI_WL_24_24},
	{"wl-32-16", AIF_SAI_WL_32_16},
	{"wl-32-24", AIF_SAI_WL_32_24},
};

static const struct dts_lut_val mme_mode_dts_lut[] = {
	{"slave", AIF_SAI_SLAVE},
	{"master", AIF_SAI_MASTER},
};

static const struct dts_lut_val io_mode_dts_lut[] = {
	{"input", AIF_SAI2_DI},
	{"output", AIF_SAI2_DO},
};

static const struct dts_lut_val aimux_dts_lut[] = {
	{"noinput", AIF_AIMUX_NO_INPUT},
	{"sai1rx1", AIF_AIMUX_SAI1RX1},
	{"sai2rx1", AIF_AIMUX_SAI2RX1},
	{"sai2rx2", AIF_AIMUX_SAI2RX2},
	{"sai2rx3", AIF_AIMUX_SAI2RX3},
	{"sai2rx4", AIF_AIMUX_SAI2RX4},
	{"sai3rx1", AIF_AIMUX_SAI3RX1},
	{"sai3rx2", AIF_AIMUX_SAI3RX2},
	{"sai3rx3", AIF_AIMUX_SAI3RX3},
	{"sai3rx4", AIF_AIMUX_SAI3RX4},
	{"sai4rx1", AIF_AIMUX_SAI4RX1},
	{"sai4rx2", AIF_AIMUX_SAI4RX2},
	{"sai4rx3", AIF_AIMUX_SAI4RX3},
	{"sai4rx4", AIF_AIMUX_SAI4RX4},
	{"spdif", AIF_AIMUX_SPDIF},
	{"lpfdi", AIF_AIMUX_LPFDI},
};

static const struct dts_lut_val aomux_dts_lut[] = {
	{"nooutput", AIF_AOMUX_NO_OUTPUT},
	{"sai2tx1", AIF_AOMUX_SAI2TX1},
	{"sai3tx1", AIF_AOMUX_SAI3TX1},
	{"sai3tx2", AIF_AOMUX_SAI3TX2},
	{"sai3tx3", AIF_AOMUX_SAI3TX3},
	{"sai3tx4", AIF_AOMUX_SAI3TX4},
	{"sai4tx1", AIF_AOMUX_SAI4TX1},
	{"sai4tx2", AIF_AOMUX_SAI4TX2},
	{"sai4tx3", AIF_AOMUX_SAI4TX3},
	{"sai4tx4", AIF_AOMUX_SAI4TX4},
	{"srcdo", AIF_AOMUX_SRCDO},
};

static const struct dts_lut_val src_dts_lut[] = {
	{"noinput", AIF_SRC_NO_INPUT},
	{"drllt", AIF_SRC_DRLL_THRES},
	{"dither", AIF_SRC_DITHER_EN},
	{"rounding", AIF_SRC_ROUNDING},
	{"bypass", AIF_SRC_BYPASS},
};

static const struct dts_lut_val lpf_dts_lut[] = {
	{"bypass", LPF_BYPASS},
	{"fir", LPF_FIR0},
	{"upx2", LPF_UPX2},
	{"upx4", LPF_UPX4},
	{"downx2", LPF_DOWNX2},
	{"downx3", LPF_DOWNX3},
	{"downx4", LPF_DOWNX4},
	{"downx6", LPF_DOWNX6},
};

static const struct dts_lut_val dma_transfer_dts_lut[] = {
	{"src0do", STA_DMA_SRC0_DO},
	{"src1do", STA_DMA_SRC1_DO},
	{"src2do", STA_DMA_SRC2_DO},
	{"src3do", STA_DMA_SRC3_DO},
	{"src4do", STA_DMA_SRC4_DO},
	{"src5do", STA_DMA_SRC5_DO},
	{"sai1rx1", STA_DMA_SAI1_RX1},
	{"sai2rx1", STA_DMA_SAI2_RX1},
	{"sai2rx2", STA_DMA_SAI2_RX2},
	{"sai2rx3", STA_DMA_SAI2_RX3},
	{"sai2rx4", STA_DMA_SAI2_RX4},
	{"sai3rx1", STA_DMA_SAI3_RX1},
	{"sai3rx2", STA_DMA_SAI3_RX2},
	{"sai3rx3", STA_DMA_SAI3_RX3},
	{"sai3rx4", STA_DMA_SAI3_RX4},
	{"sai4rx1", STA_DMA_SAI4_RX1},
	{"sai4rx2", STA_DMA_SAI4_RX2},
	{"sai4rx3", STA_DMA_SAI4_RX3},
	{"sai4rx4", STA_DMA_SAI4_RX4},
	{"fifoout", STA_DMA_FIFO_OUT},
	{"adcaux", STA_DMA_ADC_DATA},
	{"adcgpin", STA_DMA_ADC_GPIN},
	{"xout0", STA_DMA_DSP0_XOUT},
	{"xout1", STA_DMA_DSP1_XOUT},
	{"xout2", STA_DMA_DSP2_XOUT},
	{"lpf0di", STA_DMA_LPF0_DI},
	{"lpf1di", STA_DMA_LPF1_DI},
	{"lpf2di", STA_DMA_LPF2_DI},
	{"lpf3di", STA_DMA_LPF3_DI},
	{"lpf4di", STA_DMA_LPF4_DI},
	{"lpf5di", STA_DMA_LPF5_DI},
	{"sai2tx1", STA_DMA_SAI2_TX1},
	{"sai3tx1", STA_DMA_SAI3_TX1},
	{"sai3tx2", STA_DMA_SAI3_TX2},
	{"sai3tx3", STA_DMA_SAI3_TX3},
	{"sai3tx4", STA_DMA_SAI3_TX4},
	{"sai4tx1", STA_DMA_SAI4_TX1},
	{"sai4tx2", STA_DMA_SAI4_TX2},
	{"sai4tx3", STA_DMA_SAI4_TX3},
	{"sai4tx4", STA_DMA_SAI4_TX4},
	{"fifoin", STA_DMA_FIFO_IN},
	{"dco", STA_DMA_DCO_DATA},
	{"xin0", STA_DMA_DSP0_XIN},
	{"xin1", STA_DMA_DSP1_XIN},
	{"xin2", STA_DMA_DSP2_XIN},
	{"msp1", STA_DMA_DUMMY_MSP1},
	{"msp2", STA_DMA_DUMMY_MSP2},
	{"trigger", STA_DMA_SAI3_TX1},
};

static const struct dts_lut_val adcmic_dts_lut[] = {
	{"standby", ADCMIC_STANDBY12},
	{"enable1", ADCMIC1_MICIN | ADCMIC_ENABLE1 | ADCMIC_SDATA1},
	{"enable2", ADCMIC2_AIN1 | ADCMIC_ENABLE2 | ADCMIC_SDATA2},
	{"enable11", ADCMIC1_MICIN | ADCMIC2_MICIN
		       | ADCMIC_ENABLE12 | ADCMIC_SDATA12},
	{"enable22", ADCMIC1_AIN1 | ADCMIC2_AIN1
		       | ADCMIC_ENABLE12 | ADCMIC_SDATA12},
	{"enable12", ADCMIC1_MICIN | ADCMIC2_AIN1
		       | ADCMIC_ENABLE12 | ADCMIC_SDATA12},
	{"enable21", ADCMIC1_MICIN | ADCMIC2_AIN1
		       | ADCMIC_ENABLE12 | ADCMIC_SDATA21},
};

static const struct dts_lut_val adcaux_dts_lut[] = {
	{"standby", AIF_ADCAUX_STANDBY},
	{"enablech0", AIF_ADCAUX_CHSEL0 | AIF_ADCAUX_ENABLE},
	{"enablech1", AIF_ADCAUX_CHSEL1 | AIF_ADCAUX_ENABLE},
};

static const struct dts_lut_val aussmux_dts_lut[] = {
	{"disable", AUSS_MUX_DISABLE},
	{"enable", AUSS_MUX_ENABLE},
};

static const struct dts_lut_val aussmux_msp1ref_dts_lut[] = {
	{"pll3", AUSS_MUX_MSP1REFCLKINEN_PLL3},
	{"ext",  AUSS_MUX_MSP1REFCLKINEN_EXT},
	{"i2s0", AUSS_MUX_MSP1REFCLKINEN_I2S0},
};

static const struct dts_lut_val aussmux_sai1_men_dts_lut[] = {
	{"slave", (AUSS_MUX_SAI1MEN_SLAVE << AUSS_MUX_SAI1MEN_SHIFT)},
	{"sai2", (AUSS_MUX_SAI1SEL_SAI2 << AUSS_MUX_SAI1SEL_SHIFT)},
	{"sai3", (AUSS_MUX_SAI1SEL_SAI3 << AUSS_MUX_SAI1SEL_SHIFT)},
	{"sai4", (AUSS_MUX_SAI1SEL_SAI4 << AUSS_MUX_SAI1SEL_SHIFT)},
	{"msp1", (AUSS_MUX_SAI1SEL_MSP1 << AUSS_MUX_SAI1SEL_SHIFT)},
};

static const struct dts_lut_val chstream_dts_lut[] = {
	{"nostream", CHSTREAM_RESERVED},
	{"playback", CHSTREAM_PLAYBACK},
	{"capture", CHSTREAM_CAPTURE},
	{"playback1", CHSTREAM_PLAYBACK | CHSTREAM_MSP1},
	{"capture1", CHSTREAM_CAPTURE | CHSTREAM_MSP1},
	{"playback2", CHSTREAM_PLAYBACK | CHSTREAM_MSP2},
	{"capture2", CHSTREAM_CAPTURE | CHSTREAM_MSP2},
};

#endif
