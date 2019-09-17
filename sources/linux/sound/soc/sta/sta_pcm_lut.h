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

#ifndef STA10XX_PCM_LUT_H
#define STA10XX_PCM_LUT_H

#include "sta_pcm.h"

struct dts_lut_val {
	char *name;
	u64 value;
};

static const struct dts_lut_val pcm_format_dts_lut[] = {
	{"s8", SNDRV_PCM_FMTBIT_S8},
	{"u8", SNDRV_PCM_FMTBIT_U8},
	{"s16-le", SNDRV_PCM_FMTBIT_S16_LE},
	{"s16-be", SNDRV_PCM_FMTBIT_S16_BE},
	{"u16-le", SNDRV_PCM_FMTBIT_U16_LE},
	{"u16-be", SNDRV_PCM_FMTBIT_U16_BE},
	{"s24-le", SNDRV_PCM_FMTBIT_S24_LE},
	{"s24-be", SNDRV_PCM_FMTBIT_S24_BE},
	{"u24-le", SNDRV_PCM_FMTBIT_U24_LE},
	{"u24-be", SNDRV_PCM_FMTBIT_U24_BE},
	{"s32-le", SNDRV_PCM_FMTBIT_S32_LE},
	{"s32-be", SNDRV_PCM_FMTBIT_S32_BE},
	{"u32-le", SNDRV_PCM_FMTBIT_U32_LE},
	{"u32-be", SNDRV_PCM_FMTBIT_U32_BE},
	{"float-le", SNDRV_PCM_FMTBIT_FLOAT_LE},
	{"float-be", SNDRV_PCM_FMTBIT_FLOAT_BE},
	{"float64-le", SNDRV_PCM_FMTBIT_FLOAT64_LE},
	{"float64-be", SNDRV_PCM_FMTBIT_FLOAT64_BE},
};

#endif
