/*
 * Copyright (C) ST Microelectronics 2015
 *
 * Author:	Gabriele Simone <gabriele.simone@st.com>,
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef ST_CODEC_DSP_H
#define ST_CODEC_DSP_H

#include <linux/kernel.h>

int g_skip_dsp_write;

static inline void dsp_skip_write(int onoff)
{
	g_skip_dsp_write = onoff;
}

#define SKIP_WRITE	(g_skip_dsp_write)

#include "staudio.h"
#include "sta_cmd_parser.h"
#include "trace.h"
#include "sta_codec_controls.h"
#include "codec_dsp.h"
#include "sta_audio_msg.h"
#include "user.h"

#define MAX_MODULE_NAME_LEN 32
#define DSP_FW_VER_OFFSET 0xC0020

#define DSP_XRAM_DBG_VALUE 0xfeed
#define DSP_YRAM_DBG_VALUE 0xfeed

#define DSP_XRAM_DBG 0x80020
#define DSP_YRAM_DBG 0xC0024

#define AUSS_SSY_CR		0x100
#define SSY_EMECLKEN	0x6
#define SSY_EMERCORERESET	0x3

#define AUSS_DCO_CR0		0x400
#define AUSS_DCO_CR1		0x404

#define STEREO_MODE			2
#define TDM51_MODE			6

/* Gain */
#define DB_GAIN_MUTE			-1200
#define DB_GAIN_INIT			-1200
#define DB_GAIN_STEP			1
#define GAIN_NONE			0x0

#define CH_ALL					0xFFFFFFFF
#define GAIN_VOLUME_MIN			0
#define GAIN_VOLUME_MAX			1440

/* Tones and Loudness*/
#define TONE_GAIN_MIN			-240
#define TONE_GAIN_MAX			240
#define TONE_FILT_QUAL_MIN		1
#define TONE_FILT_QUAL_MAX		100
#define TONE_FILT_FREQ_MIN		20
#define TONE_FILT_FREQ_MAX		20000

#define LOUD_GAIN_MIN			-240
#define LOUD_GAIN_MAX			240
#define LOUD_FILT_QUAL_MIN		1
#define LOUD_FILT_QUAL_MAX		100
#define LOUD_FILT_FREQ_MIN		20
#define LOUD_FILT_FREQ_MAX		20000


/* Mixer */
#define MIXER_VOLUME_MIN		0
#define MIXER_VOLUME_MAX		1200
#define MIXER_MAX_CHANNELS      STA_MAX_MIXER_CH
#define MIXER_MAX_INSETS        STA_MAX_MIXER_INS

/* Beep */
#define SINE_FREQ_MIN			20000	/* 200 Hz */
#define SINE_FREQ_MAX			2000000	/* 20 KHz */
#define SINE_FREQ_INIT			76000	/* 760 Hz */
#define SINE_DURATION_MIN		-1
#define SINE_DURATION_MAX		174762
#define	SINE_DURATION_INIT		1000
#define SINE_GAIN_MIN			1
#define SINE_GAIN_MAX			1200
#define	SINE_GAIN_INIT			100

/* Limiter */
#define LIMITER_ATTACK_TIME_MIN			0
#define LIMITER_ATTACK_TIME_MAX			32767
#define LIMITER_RELEASE_TIME_MIN		0
#define LIMITER_RELEASE_TIME_MAX		32767
#define LIMITER_THRESHOLD_MIN			(-1200)
#define LIMITER_THRESHOLD_MAX			0
#define LIMITER_ATTENUATION_MIN			0
#define LIMITER_ATTENUATION_MAX			1200
#define LIMITER_HYSTERESIS_MIN			0
#define LIMITER_HYSTERESIS_MAX			600
#define LIMITER_PEAK_ATTACK_TIME_MIN		0
#define LIMITER_PEAK_ATTACK_TIME_MAX		32767
#define LIMITER_PEAK_RELEASE_TIME_MIN		0
#define LIMITER_PEAK_RELEASE_TIME_MAX		32767

/* Clip Limiter */
#define CLIP_LIMITER_ATTENUATION_MIN		0
#define CLIP_LIMITER_ATTENUATION_MAX		1200
#define CLIP_LIMITER_TIME_MIN			0
#define CLIP_LIMITER_TIME_MAX			4000000

/* Compander */
#define COMPANDER_AVG_FACTOR_MIN		1
#define COMPANDER_AVG_FACTOR_MAX		SHRT_MAX
#define COMPANDER_AVG_FACTOR_INIT		120
#define COMPANDER_ATTENUATION_MIN		0
#define COMPANDER_ATTENUATION_MAX		240
#define COMPANDER_THRESHOLD_MIN			(-1920)
#define COMPANDER_THRESHOLD_MAX			0
#define COMPANDER_SLOPE_MIN			SHRT_MIN
#define COMPANDER_SLOPE_MAX			0
#define COMPANDER_HYSTERESIS_MIN		0
#define COMPANDER_HYSTERESIS_MAX		19200
#define COMPANDER_TIME_MIN			0
#define COMPANDER_TIME_MAX			32767

/* Equalizer */
#define EQ_PRESETS_MIN				0
#define EQ_PRESETS_MAX				10
#define EQ_BAND_MIN				0
#define EQ_BAND_MAX				15
#define EQ_GAINBOOST_MIN			-240
#define EQ_GAINBOOST_MAX			240
#define EQ_GAIN_MIN					-240
#define EQ_GAIN_MAX					240
#define EQ_FREQUENCIES_MIN			20
#define EQ_FREQUENCIES_MAX			20000
#define EQ_QUAL_MIN					1
#define EQ_QUAL_MAX					100

/* CHIMEGEN */
#define CHIME_REPEAT_COUNT_MIN			-1
#define CHIME_REPEAT_COUNT_MAX			SHRT_MAX
#define CHIME_REPEAT_COUNT_INIT			1
#define CHIME_POST_REPEAT_ID_INIT		-1 /* no post ramp */
#define CHIME_RAMP_AMPLITUDE_MIN		0
#define CHIME_RAMP_AMPLITUDE_MAX		1000
#define CHIME_RAMP_DURATION_MIN			-1
#define CHIME_RAMP_DURATION_MAX			ULONG_MAX
#define CHIME_RAMP_FREQ_MIN			20000	/* 200 Hz */
#define CHIME_RAMP_FREQ_MAX			2000000	/* 20 KHz */
#define CHIME_RAMP_STOP_TIME			200

/* Bit shifter */
#define BITSHIFTER_MIN				-32
#define BITSHIFTER_MAX				32
#define BITSHIFTER_INIT				0

/* Delay */
#define DELAY_SAMPLES_MIN			1
/* default max, if not set in dtsi */
#define DELAY_SAMPLES_MAX			96
#define DELAY_MAX_CHANNELS			6

enum DelayMode {
	DELAY_OFF,
	DELAY_ON,
};

/* Tuning */
#define TUNING_MAX_LENGHT 24

#define VAL_TO_DB(val) (((val) - 1200) * DB_GAIN_STEP)
#define DB_TO_VAL(db) (((db) / DB_GAIN_STEP) + 1200)

#define CLIPPING	0
#define NO_CLIPPING	1

/*
 * For STAudiolib DSP effects
 */
struct gain {
	s16		*vol;
	u8		*mute;
	u32		time_up, time_down;
	u32		mute_up, mute_down;
};

struct tones {
	u8		ON;
	u8		num_bands; /* Bass Middle and Treble */
	s16		GQF[3][3];
	u32		filter_type[3];
};

struct loudness {
	u8		ON;
	u8		num_bands; /* Bass and Treble */
	s16		GQF[2][3];
	u32		filter_type[2];
};

struct preset {
	s16		*gains;
};

struct equalizer {
	u8		ON;
	u8		num_bands;
	s16		**bandGQF;
	struct preset	*preset;
	struct soc_enum preset_enum;
	u8		mode;
	u32		*filter_type;
	s32 		*coefs;
};

struct sinegen {
	u8		ON;
	u32		sine_freq;
	u32		sine_duration;
	s16		sine_gain;
};

struct mux {
	u8		num_out;
	u8		*sel;
};

struct mixer {
	s16		vol[MIXER_MAX_CHANNELS][MIXER_MAX_INSETS];
};

struct limiter {
	u8		ON;
	u32		mask;
	s16		params[STA_LMTR_NUMBER_OF_IDX];
};

struct clip_limiter {
	u8		status;
	u32		mask;
	s32		params[STA_CLIPLMTR_NUMBER_OF_IDX];
};

struct compander {
	u8		status;
	u32		mask;
	s16		params[STA_COMP_NUMBER_OF_IDX];
};

struct pcmchime {
	const char *file;
	u8		loaded;
	u32 bytesize;
	s32		play_count;
	u32		left_delay;
	u32		left_post_delay;
	u32		right_delay;
	u32		right_post_delay;
};

struct chimegen {
	u8		ON;
	STA_ChimeParams		params;
	struct sta_module 	*master;
};

struct bitshifter {
	s32		shift;
};

struct delay {
	u8		ON;
	u32		samples[DELAY_MAX_CHANNELS];
	u32		ch;
	u32		length;
};

struct peakdetect {
	s32		nch;
};

struct tuning {
	u16		cmd[TUNING_MAX_LENGHT];
};

struct sta_control {
	struct list_head	list;
	unsigned int numid;
	int ch;
};

struct sta_connection {
	struct list_head	list;
	char			from_name[MAX_MODULE_NAME_LEN];
	char			to_name[MAX_MODULE_NAME_LEN];
	STAModule		from_mod, to_mod;
	int 			from_ch, to_ch;
};

struct sta_module {
	STAModule		mod;
	struct device 		*dev;
	struct list_head	list;
	unsigned int		first_numid, last_numid;
	STA_Dsp			dsp_id;
	const char		*name;
	const char		*prefix;
	int			num_channels;
	int			control_channels;
	int			num_ins, num_out;
	const char		**in_names, **out_names;
	struct snd_soc_component *component;
	int			type;
	int			early_init;
	union {
		struct gain	gains;
		struct loudness loudness;
		struct tones	tones;
		struct equalizer equalizer;
		struct sinegen	sinegen;
		struct mux	mux;
		struct mixer	mixer;
		struct limiter	limiter;
		struct clip_limiter clip_limiter;
		struct compander compander;
		struct pcmchime pcmchime;
		struct chimegen chimegen;
		struct bitshifter bitshifter;
		struct delay delay;
		struct peakdetect peakdetect;
	};
};

struct STA_ModuleType_dts_lut {
	char *name;
	STA_ModuleType value;
};

static struct STA_ModuleType_dts_lut modtype_dts_lut[] = {
	{"xin", STA_XIN},
	{"xout", STA_XOUT},
	{"mux-2out", STA_MUX_2OUT},
	{"mux-4out", STA_MUX_4OUT},
	{"mux-6out", STA_MUX_6OUT},
	{"mux-8out", STA_MUX_8OUT},
	{"mux-10out", STA_MUX_10OUT},
	{"cd-deemph", STA_CD_DEEMPH},
	{"gain-static", STA_GAIN_STATIC_NCH},
	{"gain-smooth", STA_GAIN_SMOOTH_NCH},
	{"gain-linear", STA_GAIN_LINEAR_NCH},
	{"loudness-static", STA_LOUD_STATIC_DP},
	{"loudness-static-stereo", STA_LOUD_STATIC_STEREO_DP},
	{"tone-static", STA_TONE_STATIC_DP},
	{"tone-static-stereo", STA_TONE_STATIC_STEREO_DP},
	{"equalizer-static-3bands", STA_EQUA_STATIC_3BANDS_DP},
	{"equalizer-static-4bands", STA_EQUA_STATIC_4BANDS_DP},
	{"equalizer-static-5bands", STA_EQUA_STATIC_5BANDS_DP},
	{"equalizer-static-6bands", STA_EQUA_STATIC_6BANDS_DP},
	{"equalizer-static-7bands", STA_EQUA_STATIC_7BANDS_DP},
	{"equalizer-static-8bands", STA_EQUA_STATIC_8BANDS_DP},
	{"equalizer-static-9bands", STA_EQUA_STATIC_9BANDS_DP},
	{"equalizer-static-10bands", STA_EQUA_STATIC_10BANDS_DP},
	{"equalizer-static-11bands", STA_EQUA_STATIC_11BANDS_DP},
	{"equalizer-static-12bands", STA_EQUA_STATIC_12BANDS_DP},
	{"equalizer-static-13bands", STA_EQUA_STATIC_13BANDS_DP},
	{"equalizer-static-14bands", STA_EQUA_STATIC_14BANDS_DP},
	{"equalizer-static-15bands", STA_EQUA_STATIC_15BANDS_DP},
	{"equalizer-static-16bands", STA_EQUA_STATIC_16BANDS_DP},
	{"equalizer-static-3bands-sp", STA_EQUA_STATIC_3BANDS_SP},
	{"equalizer-static-4bands-sp", STA_EQUA_STATIC_4BANDS_SP},
	{"equalizer-static-5bands-sp", STA_EQUA_STATIC_5BANDS_SP},
	{"equalizer-static-6bands-sp", STA_EQUA_STATIC_6BANDS_SP},
	{"equalizer-static-7bands-sp", STA_EQUA_STATIC_7BANDS_SP},
	{"equalizer-static-8bands-sp", STA_EQUA_STATIC_8BANDS_SP},
	{"equalizer-static-9bands-sp", STA_EQUA_STATIC_9BANDS_SP},
	{"equalizer-static-10bands-sp", STA_EQUA_STATIC_10BANDS_SP},
	{"equalizer-static-11bands-sp", STA_EQUA_STATIC_11BANDS_SP},
	{"equalizer-static-12bands-sp", STA_EQUA_STATIC_12BANDS_SP},
	{"equalizer-static-13bands-sp", STA_EQUA_STATIC_13BANDS_SP},
	{"equalizer-static-14bands-sp", STA_EQUA_STATIC_14BANDS_SP},
	{"equalizer-static-15bands-sp", STA_EQUA_STATIC_15BANDS_SP},
	{"equalizer-static-16bands-sp", STA_EQUA_STATIC_16BANDS_SP},
	{"mixer-2ins", STA_MIXE_2INS_NCH},
	{"mixer-3ins", STA_MIXE_3INS_NCH},
	{"mixer-4ins", STA_MIXE_4INS_NCH},
	{"mixer-5ins", STA_MIXE_5INS_NCH},
	{"mixer-6ins", STA_MIXE_6INS_NCH},
	{"mixer-7ins", STA_MIXE_7INS_NCH},
	{"mixer-8ins", STA_MIXE_8INS_NCH},
	{"mixer-9ins", STA_MIXE_9INS_NCH},
	{"mixer-balance", STA_MIXE_BALANCE},
	{"mixer-fader", STA_MIXE_FADER},
	{"compander-1ch", STA_COMP_1CH},
	{"compander-2ch", STA_COMP_2CH},
	{"compander-4ch", STA_COMP_4CH},
	{"compander-6ch", STA_COMP_6CH},
	{"limiter-1ch", STA_LMTR_1CH},
	{"limiter-2ch", STA_LMTR_2CH},
	{"limiter-4ch", STA_LMTR_4CH},
	{"limiter-6ch", STA_LMTR_6CH},
	{"clip-limiter", STA_CLIPLMTR_NCH},
	{"delay-y-2ch", STA_DLAY_Y_2CH},
	{"delay-y-4ch", STA_DLAY_Y_4CH},
	{"delay-y-6ch", STA_DLAY_Y_6CH},
	{"delay-x-2ch", STA_DLAY_X_2CH},
	{"delay-x-4ch", STA_DLAY_X_4CH},
	{"delay-x-6ch", STA_DLAY_X_6CH},
	{"sine2ch", STA_SINE_2CH},
	{"pcmchime-12bit-y-2ch", STA_PCMCHIME_12BIT_Y_2CH},
	{"pcmchime-12bit-x-2ch", STA_PCMCHIME_12BIT_X_2CH},
	{"chimegen", STA_CHIMEGEN},
	{"bitshifter", STA_BITSHIFTER_NCH},
	{"peakdetect", STA_PEAKDETECT_NCH},
	{"specmtr", STA_SPECMTR_NBANDS_1BIQ},
	{"dco-1ch", STA_DCO_1CH},
	{"dco-4ch-monozcf", STA_DCO_4CH_MONOZCF},
	{"user0", STA_USER_0},
	{"user1", STA_USER_0 + 1},
};

struct dts_lut_val {
	char *name;
	u32 value;
};

static struct dts_lut_val ramp_type_dts_lut[] = {
	{"static", STA_RAMP_STC},
	{"exponential", STA_RAMP_EXP},
	{"linear", STA_RAMP_LIN},
	{"samples", STA_RAMP_SMP},
};

static struct dts_lut_val eq_filter_type_dts_lut[] = {
	{"bypass", STA_BIQUAD_BYPASS},
	{"lp1bt", STA_BIQUAD_LOWP_BUTTW_1},
	{"lp2bt", STA_BIQUAD_LOWP_BUTTW_2},
	{"hp1bt", STA_BIQUAD_HIGP_BUTTW_1},
	{"hp2bt", STA_BIQUAD_HIGP_BUTTW_2},
	{"ba1sh", STA_BIQUAD_BASS_SHELV_1},
	{"ba2sh", STA_BIQUAD_BASS_SHELV_2},
	{"tr1sh", STA_BIQUAD_TREB_SHELV_1},
	{"tr2sh", STA_BIQUAD_TREB_SHELV_2},
	{"bb2", STA_BIQUAD_BAND_BOOST_2},
	{"ap2cb", STA_BIQUAD_AP2_CB},
	{"lp2cb", STA_BIQUAD_LP2_CB},
	{"hp2cb", STA_BIQUAD_HP2_CB},
	{"bp2cb", STA_BIQUAD_BP2_CB},
	{"ntccb", STA_BIQUAD_NOTCH_CB},
	{"pkcb", STA_BIQUAD_PEAKING_CB},
	{"lshcb", STA_BIQUAD_LSHELF_CB},
	{"hshcb", STA_BIQUAD_HSHELF_CB},
};

static struct dts_lut_val loudness_filter_type_dts_lut[] = {
	{"sh1", STA_LOUD_SHELVING_1},
	{"sh2", STA_LOUD_SHELVING_2},
	{"bb2", STA_LOUD_BANDBOOST_2},
};

static struct dts_lut_val tones_filter_type_dts_lut[] = {
	{"sh1", STA_TONE_SHELVING_1},
	{"sh2", STA_TONE_SHELVING_2},
	{"bb2", STA_TONE_BANDBOOST_2},
};

#ifdef CONFIG_STAUDIOLIB_EMBEDDED_FW
extern char *dsp_x_start, *dsp_y_start, *dsp_p_start;
extern u32 dsp_x_size, dsp_y_size, dsp_p_size;
#endif

#endif
