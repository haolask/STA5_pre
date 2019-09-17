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
#include <sound/soc.h>

#ifndef CODEC_CONTROLS_H
#define CODEC_CONTROLS_H

static const char * const switch_mode[] = {"Off", "On"};
static const struct soc_enum switch_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(switch_mode), switch_mode);

struct st_audiolib_mixer_control {
	int count;
	int minmax[16];
};

static inline int sta_info_multi_range(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct st_audiolib_mixer_control *mc =
		(struct st_audiolib_mixer_control *)kcontrol->private_value;
	int i, min = INT_MAX, max = INT_MIN;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = mc->count;
	for (i = 0; i < mc->count; i++) {
		min = min(min, mc->minmax[i * 2]);
		max = max(max, mc->minmax[i * 2 + 1]);
	}
	uinfo->value.integer.min = min;
	uinfo->value.integer.max = max;

	return 0;
}

#define STA_MULTI_RANGE(xname, \
	xhandler_get, xhandler_put, n, ...) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = sta_info_multi_range, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct st_audiolib_mixer_control) \
	{.minmax = {__VA_ARGS__}, .count = n} }

struct sta_mixer_control {
	int min, max, count;
	unsigned int items;
	const char * const *texts;
	struct soc_enum *soc_enum;
};

#define STA_RANGE(xname, \
	 xhandler_get, xhandler_put, xcount, xmin, xmax) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_int_info, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.min = xmin, .max = xmax, .count = xcount} \
}

static inline int st_int_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct sta_mixer_control *mc =
		(struct sta_mixer_control *)kcontrol->private_value;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = mc->count;
	uinfo->value.integer.min = mc->min;
	uinfo->value.integer.max = mc->max;
	return 0;
}

#endif
