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

#include "sta_pcm.h"
#include "sta_pcm_lut.h"

#define STA_PCM_MAX_PERIOD_BYTES	65536
#define STA_PCM_MIN_PERIOD_BYTES	32

static int sta_pcm_dts_lut_match(
	struct platform_device *pdev,
	const struct dts_lut_val *lut_check, int lut_length,
	const char *str_input, u64 *lut_out_value)
{
	struct device *dev = &pdev->dev;
	int i;

	for (i = 0; i < lut_length; i++) {
		if (strcmp(lut_check[i].name, str_input) == 0) {
			*lut_out_value = lut_check[i].value;
			return 0;
		}
	}

	dev_err(dev, "%s:%s wrong LUT entry", __func__, str_input);

	return -EINVAL;
}

static int of_sta_pcm(
	struct platform_device *pdev,
	struct snd_pcm_hardware *pcm_hardware)
{
	struct device_node *nc;
	int ret = -EINVAL;
	u32 max_buffer;
	const char *fmts;
	int num_formats;
	int i;
	u64 out_fmt = 0;

	pcm_hardware->info = (SNDRV_PCM_INFO_INTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER |
		 SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID |
		 SNDRV_PCM_INFO_PAUSE | SNDRV_PCM_INFO_RESUME),
	pcm_hardware->fifo_size = 0, /* fifo size in bytes */

	nc  = of_get_child_by_name(pdev->dev.of_node, "pcm");
	if (nc) {
		num_formats = of_property_count_strings(nc, "formats");
		if (num_formats <= 0)
			return ret;

		for (i = 0; i < num_formats; i++) {
			ret = of_property_read_string_index(
				nc, "formats", i, &fmts);

			if (sta_pcm_dts_lut_match(
				pdev,
				pcm_format_dts_lut,
				ARRAY_SIZE(pcm_format_dts_lut),
				fmts, &out_fmt))
				return -EINVAL;

			pcm_hardware->formats |= out_fmt;
		}
		/* Optional parameters (currently useless): */
		of_property_read_u32(
			nc, "min-rate", &pcm_hardware->rate_min);

		of_property_read_u32(
			nc, "max-rate", &pcm_hardware->rate_max);

		of_property_read_u32(
			nc, "min-channels", &pcm_hardware->channels_min);

		of_property_read_u32(
			nc, "max-channels", &pcm_hardware->channels_max);

		/* Mandatory parameters: */
		ret = of_property_read_u32(
			nc, "min-period", &pcm_hardware->periods_min);
		if (ret)
			return ret;

		ret = of_property_read_u32(
			nc, "max-period", &pcm_hardware->periods_max);
		if (ret)
			return ret;

		ret = of_property_read_u32(
			nc, "max-buffer-bytes", &max_buffer);
		if (ret)
			return ret;
	}

	pcm_hardware->buffer_bytes_max = (size_t)max_buffer;
	pcm_hardware->period_bytes_min = STA_PCM_MIN_PERIOD_BYTES;
	pcm_hardware->period_bytes_max = STA_PCM_MAX_PERIOD_BYTES;

	return 0;
}

int sta_pcm_register(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct snd_dmaengine_pcm_config *pcm_config;
	struct snd_pcm_hardware *pcm_hardware;
	int ret = -EINVAL;

	pcm_hardware = devm_kzalloc(
		dev, sizeof(struct snd_pcm_hardware), GFP_KERNEL);

	if (!pcm_hardware) {
		dev_err(
			dev,
			"%s: ERROR: Failed to allocate pcm hardware!\n",
			__func__);
		return -ENOMEM;
	}

	if (dev->of_node)
		ret = of_sta_pcm(pdev, pcm_hardware);

	if (ret) {
		dev_err(dev, "Bad pcm device tree parameters\n");
		return -EINVAL;
	}

	pcm_config = devm_kzalloc(
		dev, sizeof(struct snd_dmaengine_pcm_config), GFP_KERNEL);

	if (!pcm_config) {
		dev_err(
			dev,
			"%s: ERROR: Failed to allocate pcm config!\n",
			__func__);
		return -ENOMEM;
	}

	pcm_config->pcm_hardware = pcm_hardware;
	pcm_config->prepare_slave_config =
		sta_dmaengine_pcm_prepare_slave_config;
	pcm_config->prealloc_buffer_size = pcm_hardware->buffer_bytes_max;

	return sta_dmaengine_pcm_register(
		dev, pcm_config,
		SND_DMAENGINE_PCM_FLAG_COMPAT);
}
EXPORT_SYMBOL(sta_pcm_register);

int sta_pcm_unregister(struct device *dev)
{
	snd_dmaengine_pcm_unregister(dev);
	return 0;
}
EXPORT_SYMBOL(sta_pcm_unregister);
