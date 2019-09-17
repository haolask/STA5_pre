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

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <sound/soc.h>
#include <sound/dmaengine_pcm.h>

#include "sta_pcm.h"

#define MAX_DPMAILBOX_DAI_NAME 32

#define DPM_I2S_RATES (SNDRV_PCM_RATE_48000)
#define DPM_I2S_FORMATS (SNDRV_PCM_FMTBIT_S16_LE)

#define DPM_MIN_CHANNELS	1
#define	DPM_MAX_CHANNELS	32

#define DPM_FIFO1	0x0
#define DPM_FIFO2	0x200
#define DPM_FIFO_LEN	32
#define DPM_CR		0x100
#define MINFL(lev)	(((lev) & 0x1F) << 8)
#define MAXFL(lev)	(((lev) & 0x1F) << 0)
#define WRPTRST		BIT(17)
#define RDPTRST		BIT(18)

#define SSY_CR		0x100
#define WRFFREQEN	BIT(11)
#define RDFFREQEN	BIT(10)

struct dpmailbox_drvdata {
	struct snd_dmaengine_dai_dma_data playback_dma_data;
	struct snd_dmaengine_dai_dma_data capture_dma_data;
	void __iomem *registers;
	struct regmap *auss_regmap;
	u32 minfl, maxfl, playback_burst, capture_burst;
#ifdef CONFIG_PM_SLEEP
	u32 dpm_cr;
#endif
};

static int dpmailbox_dai_startup(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static void dpmailbox_dai_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int i;
	u32 temp;
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dai->dev);

	temp = readl(dpmailbox->registers + DPM_CR);
	writel(temp & ~(WRPTRST | RDPTRST), dpmailbox->registers + DPM_CR);

	for (i = 0; i < DPM_FIFO_LEN; i++)
		writel(0, dpmailbox->registers + DPM_FIFO2);
}

static int dpmailbox_dai_prepare(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int i;
	u32 temp;
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dai->dev);

	temp = readl(dpmailbox->registers + DPM_CR);
	writel(temp & ~(WRPTRST | RDPTRST), dpmailbox->registers + DPM_CR);
	for (i = 0; i < DPM_FIFO_LEN; i++)
		writel(0, dpmailbox->registers + DPM_FIFO2);
	writel(temp | (WRPTRST | RDPTRST), dpmailbox->registers + DPM_CR);

	return 0;
}

static int dpmailbox_dai_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dai->dev);
	struct snd_dmaengine_dai_dma_data *dma_data;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		dma_data = &dpmailbox->playback_dma_data;
		dma_data->maxburst = dpmailbox->playback_burst;
	} else {
		dma_data = &dpmailbox->capture_dma_data;
		dma_data->maxburst = dpmailbox->capture_burst;
	}

	writel(MINFL(dpmailbox->minfl) | MAXFL(dpmailbox->maxfl),
	       dpmailbox->registers + DPM_CR);

	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S16_LE:
		dma_data->addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		break;
	}

	snd_pcm_hw_constraint_minmax(
		runtime, SNDRV_PCM_HW_PARAM_CHANNELS,
		DPM_MIN_CHANNELS, DPM_MAX_CHANNELS);

	snd_soc_dai_set_dma_data(dai, substream, dma_data);

	return 0;
}

static int dpmailbox_dai_set_tdm_slot(
		struct snd_soc_dai *dai,
		unsigned int tx_mask, unsigned int rx_mask,
		int slots, int slot_width)
{
	return 0;
}

static int dpmailbox_dai_set_dai_fmt(
	struct snd_soc_dai *dai,
	unsigned int fmt)
{
	return 0;
}

static int dpmailbox_dai_trigger(
	struct snd_pcm_substream *substream,
	int cmd, struct snd_soc_dai *dai)
{
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dai->dev);
	int i;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		for (i = 0; i < DPM_FIFO_LEN; i++)
			writel(0, dpmailbox->registers + DPM_FIFO2);
		break;
	}

	return 0;
}

static int dpmailbox_dai_probe(struct snd_soc_dai *dai)
{
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dai->dev);

	dai->playback_dma_data = &dpmailbox->playback_dma_data;
	dai->capture_dma_data = &dpmailbox->capture_dma_data;

	return 0;
}

static struct snd_soc_dai_ops dpmailbox_dai_ops = {
	.set_fmt = dpmailbox_dai_set_dai_fmt,
	.prepare = dpmailbox_dai_prepare,
	.startup = dpmailbox_dai_startup,
	.shutdown = dpmailbox_dai_shutdown,
	.trigger = dpmailbox_dai_trigger,
	.hw_params = dpmailbox_dai_hw_params,
	.set_tdm_slot = dpmailbox_dai_set_tdm_slot,
};

static struct snd_soc_dai_driver dpmailbox_dai_drv_init = {
	.probe = dpmailbox_dai_probe,
	.suspend = NULL,
	.resume = NULL,
	.playback = {
		.channels_min = DPM_MIN_CHANNELS,
		.channels_max = DPM_MAX_CHANNELS,
		.rates = DPM_I2S_RATES,
		.formats = DPM_I2S_FORMATS,
	},
	.capture = {
		.channels_min = DPM_MIN_CHANNELS,
		.channels_max = DPM_MAX_CHANNELS,
		.rates = DPM_I2S_RATES,
		.formats = DPM_I2S_FORMATS,
	},
	.ops = &dpmailbox_dai_ops,
};

static const struct snd_soc_component_driver dpmailbox_component = {
	.name		= "sta-dpmailbox",
};

static const struct of_device_id dpmailbox_match[] = {
	{ .compatible = "st,sta-dpmailbox", },
	{},
};
MODULE_DEVICE_TABLE(of, dpmailbox_match);

static int dpmailbox_probe(struct platform_device *pdev)
{
	struct dpmailbox_drvdata *dpmailbox;
	struct snd_soc_dai_driver *dai_drv;
	static atomic_t dp_port_count;
	struct device_node *np = pdev->dev.of_node;
	struct resource *res = NULL;
	int ret, ndais;
	char *name;
	int i;

	ndais = atomic_inc_return(&dp_port_count);
	pdev->id = ndais - 1;

	dpmailbox = devm_kzalloc(
		&pdev->dev,
		sizeof(struct dpmailbox_drvdata),
		GFP_KERNEL);
	if (!dpmailbox) {
		dev_err(&pdev->dev,
			"%s: ERROR: Failed to allocate drvdata-struct!\n",
			__func__);
		return -ENOMEM;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dpmailbox->registers = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(dpmailbox->registers))
		return PTR_ERR(dpmailbox->registers);

	dpmailbox->playback_dma_data.addr = res->start + DPM_FIFO2;
	dpmailbox->capture_dma_data.addr = res->start + DPM_FIFO1;

	dpmailbox->auss_regmap =
		syscon_regmap_lookup_by_phandle(np, "syscon-auss");
	if (IS_ERR(dpmailbox->auss_regmap)) {
		dev_err(&pdev->dev, "could not find AUSS syscon regmap\n");
		return PTR_ERR(dpmailbox->auss_regmap);
	}

	dpmailbox->playback_burst = 1;
	of_property_read_u32(np, "playback-burst", &dpmailbox->playback_burst);
	dpmailbox->capture_burst = 1;
	of_property_read_u32(np, "capture-burst", &dpmailbox->capture_burst);
	dpmailbox->minfl = 0;
	of_property_read_u32(np, "minfl", &dpmailbox->minfl);
	dpmailbox->maxfl = 1;
	of_property_read_u32(np, "maxfl", &dpmailbox->maxfl);

	ret = regmap_update_bits(dpmailbox->auss_regmap, SSY_CR,
				 (WRFFREQEN | RDFFREQEN),
				 (WRFFREQEN | RDFFREQEN));
	if (ret)
		return ret;

	for (i = 0; i < DPM_FIFO_LEN; i++)
		writel(0, dpmailbox->registers + DPM_FIFO2);

	dev_set_drvdata(&pdev->dev, dpmailbox);

	dai_drv = devm_kzalloc(
		&pdev->dev, sizeof(*dai_drv), GFP_KERNEL);
	if (!dai_drv) {
		ret = -ENOMEM;
		goto err_reg_plat;
	}
	*dai_drv = dpmailbox_dai_drv_init;
	dai_drv->id = ndais - 1;

	name = devm_kzalloc(
		&pdev->dev, MAX_DPMAILBOX_DAI_NAME, GFP_KERNEL);
	if (!name) {
		ret = -ENOMEM;
		goto err_reg_plat;
	}
	snprintf(
		name, MAX_DPMAILBOX_DAI_NAME,
		"dpmailbox_dai.%d\n", dai_drv->id);
	dai_drv->name = name;

	ret = snd_soc_register_component(
		&pdev->dev, &dpmailbox_component, dai_drv, 1);
	if (ret < 0) {
		dev_err(
			&pdev->dev,
			"Error: %s: Failed to register DPMAILBOX!\n",
			__func__);
		goto err_reg_plat;
	}

	ret = sta_pcm_register(pdev);
	if (ret < 0) {
		dev_err(
			&pdev->dev,
			"Error: %s: Failed to register PCM platform device!\n",
			__func__);
		goto err_reg_plat;
	}

	return 0;

err_reg_plat:
	snd_soc_unregister_component(&pdev->dev);

	return ret;
}

static int dpmailbox_remove(struct platform_device *pdev)
{
	sta_pcm_unregister(&pdev->dev);

	snd_soc_unregister_component(&pdev->dev);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int dpmailbox_suspend(struct device *dev)
{
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dev);

	dpmailbox->dpm_cr = readl(dpmailbox->registers + DPM_CR);
	return 0;
}

static int dpmailbox_resume(struct device *dev)
{
	struct dpmailbox_drvdata *dpmailbox = dev_get_drvdata(dev);
	int i, ret;

	writel(dpmailbox->dpm_cr, dpmailbox->registers + DPM_CR);
	/* Clear DPmailbox FIFOOUT */
	ret = regmap_update_bits(dpmailbox->auss_regmap, SSY_CR,
				 (WRFFREQEN | RDFFREQEN),
				 (WRFFREQEN | RDFFREQEN));
	if (ret)
		return ret;

	for (i = 0; i < DPM_FIFO_LEN; i++)
		writel(0, dpmailbox->registers + DPM_FIFO2);

	return 0;
}
#endif

static const struct dev_pm_ops dpmailbox_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dpmailbox_suspend, dpmailbox_resume)
};

static struct platform_driver dpmailbox_driver = {
	.driver = {
		.name = "sta-dpmailbox",
		.of_match_table = dpmailbox_match,
		.pm = &dpmailbox_pm_ops,
	},
	.probe = dpmailbox_probe,
	.remove = dpmailbox_remove,
};
module_platform_driver(dpmailbox_driver);

MODULE_LICENSE("GPL v2");
