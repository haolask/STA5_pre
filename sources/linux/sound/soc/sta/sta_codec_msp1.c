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

#include <linux/module.h>
#include <sound/soc.h>

#include "sta_codec_aif.h"

static struct snd_soc_codec_driver st_codec_msp1;

static void st_codec_msp1_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return st_codec_aif_shutdown(substream, dai, 1);
}

static int st_codec_msp1_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	return st_codec_aif_hw_params(substream, params, dai, 1);
}

static int st_codec_msp1_set_fmt(
	struct snd_soc_dai *dai,
	unsigned int fmt)
{
	return st_codec_aif_set_fmt(dai, fmt, 1);
}

#define ST_CODEC_MSP1_MIN_CHANNELS 1
#define ST_CODEC_MSP1_MAX_CHANNELS 8
#define ST_CODEC_MSP1_RATES	(SNDRV_PCM_RATE_8000_48000 \
			       | SNDRV_PCM_RATE_CONTINUOUS)
#define ST_CODEC_MSP1_FORMATS SNDRV_PCM_FMTBIT_S16_LE

struct snd_soc_dai_ops st_codec_msp1_dai_ops = {
	.hw_params    = st_codec_msp1_hw_params,
	.set_fmt      = st_codec_msp1_set_fmt,
	.shutdown     = st_codec_msp1_shutdown,
};

static struct snd_soc_dai_driver st_codec_msp1_dai = {
	.id = 0,
	.playback = {
		.stream_name = "MSP1 Playback",
		.channels_min = ST_CODEC_MSP1_MIN_CHANNELS,
		.channels_max = ST_CODEC_MSP1_MAX_CHANNELS,
		.rates = ST_CODEC_MSP1_RATES,
		.formats = ST_CODEC_MSP1_FORMATS,
	},
	.capture = {
		.stream_name = "MSP1 Capture",
		.channels_min = ST_CODEC_MSP1_MIN_CHANNELS,
		.channels_max = ST_CODEC_MSP1_MAX_CHANNELS,
		.rates = ST_CODEC_MSP1_RATES,
		.formats = ST_CODEC_MSP1_FORMATS,
	},
	.ops = &st_codec_msp1_dai_ops,
	.symmetric_rates = 1,
};

static const struct of_device_id st_codec_msp1_dt_ids[] = {
	{ .compatible = "st,codec-msp1", },
	{ }
};
MODULE_DEVICE_TABLE(of, st_codec_msp1_dt_ids);

static int st_codec_msp1_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;
	struct device_node *np = pdev->dev.of_node;

	ret = of_property_read_string(
		np, "codec-dai-name", &st_codec_msp1_dai.name);
	if (ret) {
		dev_err(&pdev->dev, "missing codec-dai-name!\n");
		return ret;
	}

	ret = snd_soc_register_codec(
		dev, &st_codec_msp1, &st_codec_msp1_dai, 1);
	if (ret)
		dev_err(dev, "Failed to register codec!\n");

	return ret;
}

static int st_codec_msp1_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);

	return 0;
}

static struct platform_driver st_codec_msp1_driver = {
	.driver = {
		.name = "st-codec-msp1",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(st_codec_msp1_dt_ids),
	},
	.probe =    st_codec_msp1_probe,
	.remove =   st_codec_msp1_remove,
};

module_platform_driver(st_codec_msp1_driver);

MODULE_DESCRIPTION("SoC  st_codec_msp1 driver");
MODULE_AUTHOR("Gabriele Simone gabriele.simone@st.com");
MODULE_LICENSE("GPL");
