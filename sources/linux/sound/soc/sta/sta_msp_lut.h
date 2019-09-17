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

#ifndef STA_MSP_LUT_H
#define STA_MSP_LUT_H

#define AUSS_MUX_CR		0x1000
#define MSP2REFCLKINEN	BIT(11)
#define MSP1REFCLKINEN1	BIT(10)
#define MSP1REFCLKINEN0	BIT(5)
#define MSP0REFCLKINEN	BIT(9)

struct dts_lut {
	char *name;
	u32 value;
};

struct dts_key {
	struct dts_lut *lut;
	int lut_length;
	u32 field_offset;
};

enum msp_protocol {
	MSP_I2S_PROTOCOL,
	MSP_TDM_PROTOCOL,
	MSP_INVALID_PROTOCOL
};

enum msp_rx_comparison_enable_mode {
	MSP_COMPARISON_DISABLED = 0,
	MSP_COMPARISON_NONEQUAL_ENABLED = 2,
	MSP_COMPARISON_EQUAL_ENABLED = 3
};

enum i2s_fsync_pol {
	I2S_ACTIVE_LOW,
	I2S_ACTIVE_HIGH,
};

enum i2s_clk_pol {
	I2S_RISING,
	I2S_FALLING,
};

enum msp_ms_mode {
	I2S_SLAVE,
	I2S_MASTER,
};

enum msp_ie_mode {
	I2S_INTERNAL,
	I2S_EXTERNAL,
};

enum msp_chord {
	MSP_CHORD_ALL_LEFT_FIRST,
	MSP_CHORD_LEFT_RIGHT_INTERLEAVED,
};

struct msp_protdesc {
	u32 max_channels;
	enum msp_chord chord;
	/* GCR */
	u32 rx_fsync_pol;
	u32 rx_fsync_sel;
	u32 rx_clk_pol;
	u32 rx_clk_sel;
	u32 tx_fsync_pol;
	u32 tx_fsync_sel;
	u32 tx_clk_pol;
	u32 tx_clk_sel;
	u32 tx_extra_delay;
	u32 srg_clk_pol;
	u32 srg_clk_sel;
	/* TCF */
	u32 tx_phase2_on;
	u32 tx_elem_len_1;
	u32 tx_frame_len_1;
	u32 tx_byte_order;
	u32 tx_data_delay;
	u32 tx_elem_len_2;
	u32 tx_frame_len_2;
	u32 tx_phase2_start_mode;
	u32 tx_half_word_swap;
	/* RCF */
	u32 rx_phase2_on;
	u32 rx_elem_len_1;
	u32 rx_frame_len_1;
	u32 rx_byte_order;
	u32 rx_data_delay;
	u32 rx_elem_len_2;
	u32 rx_frame_len_2;
	u32 rx_phase2_start_mode;
	u32 rx_half_word_swap;
	u32 compression_mode;
	u32 expansion_mode;
	/* SRG */
	u32 frame_sync_ignore;
	u32 frame_period;
	u32 frame_width;
};

static struct dts_lut fsync_pol_dts_lut[] = {
{"active_high", I2S_ACTIVE_HIGH},
{"active_low", I2S_ACTIVE_LOW},
};

static struct dts_lut clk_pol_dts_lut[] = {
{"falling", I2S_FALLING},
{"rising", I2S_RISING},
};

static struct dts_lut dir_dts_lut[] = {
{"master", I2S_MASTER},
{"slave", I2S_SLAVE},
};

static struct dts_lut source_dts_lut[] = {
{"int", I2S_INTERNAL},
{"ext", I2S_EXTERNAL},
};

#define MSP_PROPERTY(p, l, f) { \
			.name = p, \
			.value = (u32)&(struct dts_key){ \
			.lut = l, \
			.lut_length = sizeof(l) / sizeof(struct dts_lut), \
			.field_offset = offsetof(struct msp_protdesc, f)} }

static struct dts_lut msp_dts_keys_lut[] = {
	MSP_PROPERTY("tx-fsync-pol", fsync_pol_dts_lut, tx_fsync_pol),
	MSP_PROPERTY("rx-fsync-pol", fsync_pol_dts_lut, rx_fsync_pol),
	MSP_PROPERTY("tx-clk-pol", clk_pol_dts_lut, tx_clk_pol),
	MSP_PROPERTY("rx-clk-pol", clk_pol_dts_lut, rx_clk_pol),
	MSP_PROPERTY("tx-fsync-sel", dir_dts_lut, tx_fsync_sel),
	MSP_PROPERTY("rx-fsync-sel", dir_dts_lut, rx_fsync_sel),
	MSP_PROPERTY("tx-clk-sel", dir_dts_lut, tx_clk_sel),
	MSP_PROPERTY("rx-clk-sel", dir_dts_lut, rx_clk_sel),
	MSP_PROPERTY("srg-clk-sel", source_dts_lut, srg_clk_sel),
	MSP_PROPERTY("srg-clk-pol", clk_pol_dts_lut, srg_clk_pol),
	MSP_PROPERTY("tx-data-delay", NULL, tx_data_delay),
	MSP_PROPERTY("tx-extra-delay", NULL, tx_extra_delay),
	MSP_PROPERTY("tx-elem-len-1", NULL, tx_elem_len_1),
	MSP_PROPERTY("tx-frame-len-1", NULL, tx_frame_len_1),
	MSP_PROPERTY("tx-phase2-on", NULL, tx_phase2_on),
	MSP_PROPERTY("tx-elem-len-2", NULL, tx_elem_len_2),
	MSP_PROPERTY("tx-frame-len-2", NULL, tx_frame_len_2),
	MSP_PROPERTY("tx-phase2-start-mode", NULL, tx_phase2_start_mode),
	MSP_PROPERTY("rx-data-delay", NULL, rx_data_delay),
	MSP_PROPERTY("rx-elem-len-1", NULL, rx_elem_len_1),
	MSP_PROPERTY("rx-frame-len-1", NULL, rx_frame_len_1),
	MSP_PROPERTY("rx-phase2-on", NULL, rx_phase2_on),
	MSP_PROPERTY("rx-elem-len-2", NULL, rx_elem_len_2),
	MSP_PROPERTY("rx-frame-len-2", NULL, rx_frame_len_2),
	MSP_PROPERTY("rx-phase2-start-mode", NULL, rx_phase2_start_mode),
	MSP_PROPERTY("frame-sync-ignore", NULL, frame_sync_ignore),
	MSP_PROPERTY("frame-period", NULL, frame_period),
	MSP_PROPERTY("frame-width", NULL, frame_width),
	MSP_PROPERTY("compression-mode", NULL, compression_mode),
	MSP_PROPERTY("expansion-mode", NULL, expansion_mode),
	MSP_PROPERTY("tx-byte-order", NULL, tx_byte_order),
	MSP_PROPERTY("tx-half-word-swap", NULL, tx_half_word_swap),
	MSP_PROPERTY("rx-byte-order", NULL, rx_byte_order),
	MSP_PROPERTY("rx-half-word-swap", NULL, rx_half_word_swap),
};

static s32 msp_dts_lut_match(
	struct dts_lut *lut,
	int lut_length,
	const char *str,
	u32 *val)
{
	int i;

	for (i = 0; i < lut_length; i++) {
		if (strcmp(lut[i].name, str) == 0) {
			*val = lut[i].value;
			return 0;
		}
	}

	return -EINVAL;
}

static struct msp_protdesc protdesc_default = {
	.max_channels = 8,
	/* GCR */
	.tx_fsync_pol = I2S_ACTIVE_LOW,
	.rx_fsync_pol = I2S_ACTIVE_LOW,
	.tx_clk_pol = I2S_FALLING,
	.rx_clk_pol = I2S_FALLING,
	.tx_fsync_sel = I2S_MASTER,
	.rx_fsync_sel = I2S_MASTER,
	.tx_clk_sel = I2S_MASTER,
	.rx_clk_sel = I2S_MASTER,
	.srg_clk_sel = I2S_INTERNAL,
	/* TCF */
	.tx_data_delay = 0,
	.tx_phase2_start_mode = 1,
	/* RCF */
	.rx_data_delay = 0,
	.rx_phase2_start_mode = 1,
	/* TCE0/RCE0 8 slots */
	.chord = MSP_CHORD_ALL_LEFT_FIRST,
	/* SRG */
	/* fsync 16 bits low and 16 high */
	.frame_width = 16,
	.frame_period = 32,
	.frame_sync_ignore = 0,
};

#endif
