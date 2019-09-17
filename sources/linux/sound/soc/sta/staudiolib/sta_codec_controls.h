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

struct sta_module *get_sta_module(struct snd_kcontrol *kcontrol);
int get_sta_channel(struct snd_kcontrol *kcontrol);

typedef int (*sta_mixer_func) (struct sta_module *module);
typedef int (*sta_mixer_ch_func) (struct sta_module *module, int ch);

struct sta_mixer_control {
	sta_mixer_func minf, maxf, countf;
	sta_mixer_ch_func minfch, maxfch;
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

#define STA_ENUM(xname, xenum, \
	 xhandler_get, xhandler_put, xcount) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_enum_info_ext, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.soc_enum = (struct soc_enum *)&(xenum), .count = xcount} \
}

#define STA_COUNT_EXT(xname, \
	 xhandler_get, xhandler_put, xhandler_count, xmin, xmax) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_int_info_ext, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.min = xmin, .max = xmax, .countf = xhandler_count} \
}

#define STA_ENUM_EXT(xname, xenum, \
	 xhandler_get, xhandler_put, xhandler_count) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_enum_info_ext, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.soc_enum = (struct soc_enum *)&(xenum), .countf = xhandler_count} \
}

#define STA_RANGE_EXT(xname, \
	 xhandler_get, xhandler_put, xcount, xhandler_min, xhandler_max) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_int_info_ext, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.minf = xhandler_min, .maxf = xhandler_max, .count = 1} \
}

#define STA_RANGE_CH_EXT(xname, \
	 xhandler_get, xhandler_put, xhandler_min, xhandler_max) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_int_info_ext, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.minfch = xhandler_min, .maxfch = xhandler_max, .count = 1} \
}

#define STA_RANGE_COUNT_EXT(xname, \
	 xhandler_get, xhandler_put, xhandler_count, \
	 xhandler_min, xhandler_max) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = st_int_info_ext, \
	.get = xhandler_get, .put = xhandler_put, \
	.private_value = (unsigned long)&(struct sta_mixer_control) \
	{.minf = xhandler_min, .maxf = xhandler_max, .countf = xhandler_count} \
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

static inline int st_int_info_ext(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct sta_mixer_control *mc =
		(struct sta_mixer_control *)kcontrol->private_value;
	struct sta_module *module = NULL;
	int ch = -1;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	if (mc->countf || mc->minf || mc->maxf || mc->minfch || mc->maxfch) {
		module = get_sta_module(kcontrol);
		if (!module)
			return -1;
	}
	if (mc->minfch || mc->maxfch) {
		ch = get_sta_channel(kcontrol);
		if (ch < 0)
			return -1;
	}

	if (mc->minfch)
		uinfo->value.integer.min = mc->minfch(module, ch);
	else if (mc->minf)
		uinfo->value.integer.min = mc->minf(module);
	else
		uinfo->value.integer.min = mc->min;

	if (mc->maxfch)
		uinfo->value.integer.max = mc->maxfch(module, ch);
	else if (mc->maxf)
		uinfo->value.integer.max = mc->maxf(module);
	else
		uinfo->value.integer.max = mc->max;

	uinfo->count = mc->countf ? mc->countf(module) : mc->count;

	return 0;
}

static inline int st_enum_info_ext(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct sta_module *module = get_sta_module(kcontrol);
	struct sta_mixer_control *mc =
		(struct sta_mixer_control *)kcontrol->private_value;
	struct soc_enum *e = mc->soc_enum;

	if (!module)
		return -1;

	uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	uinfo->count = mc->countf ? mc->countf(module) : mc->count;
	uinfo->value.enumerated.items = e->items;
	if (!e->items)
		return 0;
	if (uinfo->value.enumerated.item >= e->items)
		uinfo->value.enumerated.item = e->items - 1;
	WARN(strlen(e->texts[uinfo->value.enumerated.item]) >=
	      sizeof(uinfo->value.enumerated.name),
		 "ALSA: too long item name '%s'\n",
		 e->texts[uinfo->value.enumerated.item]);
	strlcpy(uinfo->value.enumerated.name,
		e->texts[uinfo->value.enumerated.item],
		sizeof(uinfo->value.enumerated.name));
	return 0;
}

#endif
