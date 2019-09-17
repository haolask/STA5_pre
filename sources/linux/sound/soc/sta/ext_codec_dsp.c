/*
 * Copyright (C) ST Microelectronics 2015
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>
#include <linux/kernel.h>

#include "codec_dsp.h"
#include "codec_controls.h"

#define FW_VERSION 10

struct codec_dsp_drvdata {
	struct dsp_data dsp_data[NUM_DSP];
	struct device *dev;
	int num_dsp;
	struct regmap *auss_regmap;
	const char *fwcustom[NUM_DSP];
	/* Clocks */
	struct clk *emrclk, *mclk;
	struct snd_card *card;
};

static int ver_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = FW_VERSION;

	return 0;
}

static const struct snd_kcontrol_new ext_dsp_controls[] = {
	STA_RANGE(
		"FW ver",
		ver_get, NULL, 1, FW_VERSION, FW_VERSION),
};

int ext_codec_dsp_load(struct device *dev)
{
	const char *suffix[3] = {"P.noheader", "X.noheader", "Y.noheader"};
	const struct firmware *fw[3];
	char *file;
	int err, i, xyp, core;
	static char *dspaddr;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

#if IS_BUILTIN(CONFIG_SND_SOC_EXT_DSP)
	dev_err(dev, "can't load DSP if not built as module!\n");
	return -ENOENT;
#endif

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		core = dsp_codec_drv->dsp_data[i].core;

		if (!dsp_codec_drv->fwcustom[core])
			continue;

		dspaddr = dsp_codec_drv->dsp_data[i].dsp_addr;

		/* Read the DSP FW from firmware files */
		for (xyp = 0; xyp < ARRAY_SIZE(suffix); xyp++) {
			file = kasprintf(GFP_KERNEL, "%s/%s.%s",
					 ST_CODEC_DSP_FW_SUBPATH,
					 dsp_codec_drv->fwcustom[core],
					 suffix[xyp]);
			err = request_firmware_direct(&fw[xyp], file, dev);
			if (err < 0)
				return -EINVAL;
			kfree(file);
		}

		/* Upload DSP FW */
		_dsp_clken(dsp_codec_drv->auss_regmap, core);
		_dsp_load((u32 *)(dspaddr + PRAM),
			  (u32 *)fw[0]->data, fw[0]->size);
		_dsp_load((u32 *)(dspaddr + XRAM),
			  (u32 *)fw[1]->data, fw[1]->size);
		_dsp_load((u32 *)(dspaddr + YRAM),
			  (u32 *)fw[2]->data, fw[2]->size);

		/* Clear XIN/XOUT */
		_dsp_memset((u32 *)(dspaddr + XIN(core)), 0x0, XIN_SIZE);
		_dsp_memset((u32 *)(dspaddr + XOUT(core)), 0x0, XOUT_SIZE);
		_dsp_xin_dmabus_clear(&dsp_codec_drv->dsp_data[i]);

		_dsp_start(dsp_codec_drv->auss_regmap, core);

		release_firmware(fw[0]);
		release_firmware(fw[1]);
		release_firmware(fw[2]);

		dev_warn(dev, "DSP%d %s loaded\n", core,
			 dsp_codec_drv->fwcustom[core]);
	}

	return 0;
}

static int ext_codec_dsp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct resource *res = NULL;
	struct codec_dsp_drvdata *dsp_codec_drv;
	int ret, i, core;
	char key[9];

	dsp_codec_drv =
		devm_kzalloc(
			&pdev->dev,
			sizeof(struct codec_dsp_drvdata),
			GFP_KERNEL);
	if (!dsp_codec_drv) {
		dev_err(
			dev,
			"%s: ERROR: Failed to init aif drvdata-struct!\n",
			__func__);
		return -ENOMEM;
	}

	dsp_codec_drv->dev = dev;

	platform_set_drvdata(pdev, dsp_codec_drv);

	for (core = 0; core < NUM_DSP; core++) {
		i = dsp_codec_drv->num_dsp;
		sprintf(key, "dsp%d-mem", core);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, key);
		if (res <= 0) {
			/* DSP not managed by this device */
			dsp_codec_drv->dsp_data[i].dsp_addr = NULL;
			continue;
		}
		dsp_codec_drv->dsp_data[i].dsp_addr =
			devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(dsp_codec_drv->dsp_data[i].dsp_addr))
			return PTR_ERR(dsp_codec_drv->dsp_data[i].dsp_addr);
		sprintf(key, "dpmem%d", core);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, key);
		dsp_codec_drv->dsp_data[i].dsp_dpmem =
			devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(dsp_codec_drv->dsp_data[i].dsp_dpmem))
			return PTR_ERR(dsp_codec_drv->dsp_data[i].dsp_dpmem);
		dsp_codec_drv->dsp_data[i].core = core;
		dsp_codec_drv->num_dsp++;
		sprintf(key, "fw%d", core);
		of_property_read_string(np, key,
					&dsp_codec_drv->fwcustom[core]);
	}

	dsp_codec_drv->auss_regmap =
		syscon_regmap_lookup_by_phandle(np, "syscon-auss");
	if (IS_ERR(dsp_codec_drv->auss_regmap)) {
		dev_err(&pdev->dev, "could not find AUSS syscon regmap\n");
		return PTR_ERR(dsp_codec_drv->auss_regmap);
	}

	dsp_codec_drv->emrclk = devm_clk_get(&pdev->dev, "emrclk");
	if (IS_ERR(dsp_codec_drv->emrclk))
		return PTR_ERR(dsp_codec_drv->emrclk);

	ret = clk_prepare_enable(dsp_codec_drv->emrclk);
	if (ret) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to prepare/enable emrclk!\n",
			__func__);
		return ret;
	}

	dsp_codec_drv->mclk = devm_clk_get(&pdev->dev, "mclk");
	if (IS_ERR(dsp_codec_drv->mclk))
		return PTR_ERR(dsp_codec_drv->mclk);

	ret = clk_prepare_enable(dsp_codec_drv->mclk);
	if (ret) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to prepare/enable mclk!\n",
			__func__);
		goto err_dsp_clk;
	}

	ret = ext_codec_dsp_load(&pdev->dev);
	if (ret)
		goto err_dsp_load;

	ret = snd_card_new(&pdev->dev, 6, "ext_codec_dsp", THIS_MODULE,
			   0, &dsp_codec_drv->card);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to create card\n");
		goto err_dsp_load;
	}

	ret = snd_card_register(dsp_codec_drv->card);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register card\n");
		goto err_snd_card_new;
	}

	for (i = 0; i < ARRAY_SIZE(ext_dsp_controls); i++) {
		ret = snd_ctl_add(dsp_codec_drv->card,
				  snd_ctl_new1(&ext_dsp_controls[i],
					       dsp_codec_drv));
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to add controls\n");
			goto err_snd_card_new;
		}
	}

	return 0;

err_snd_card_new:
	snd_card_free(dsp_codec_drv->card);
err_dsp_load:
	clk_disable_unprepare(dsp_codec_drv->mclk);
err_dsp_clk:
	clk_disable_unprepare(dsp_codec_drv->emrclk);
	return ret;
}

static int ext_codec_dsp_remove(struct platform_device *pdev)
{
	struct codec_dsp_drvdata *dsp_codec_drv	=
		dev_get_drvdata(&pdev->dev);

	snd_card_free(dsp_codec_drv->card);

	clk_disable_unprepare(dsp_codec_drv->emrclk);
	clk_disable_unprepare(dsp_codec_drv->mclk);

	return 0;
}

static const struct of_device_id ext_codec_dsp_dt_ids[] = {
	{ .compatible = "st,extdspfw", },
	{ }
};
MODULE_DEVICE_TABLE(of, ext_codec_dsp_dt_ids);

#ifdef CONFIG_PM_SLEEP
static int dsp_suspend(struct device *dev)
{
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);
	int i, ret, n;

	dev_info(dev, "%s\n", __func__);

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		ret = _dsp_alloc(&dsp_codec_drv->dsp_data[i]);
		if (ret)
			goto nomem;
	}

	for (i = 0; i < dsp_codec_drv->num_dsp; i++)
		_dsp_suspend(&dsp_codec_drv->dsp_data[i],
			     dsp_codec_drv->auss_regmap);

	clk_disable_unprepare(dsp_codec_drv->emrclk);
	clk_disable_unprepare(dsp_codec_drv->mclk);

	return 0;

nomem:
	dev_err(dev, "%s alloc error DSP %d\n", __func__,
		dsp_codec_drv->dsp_data[i].core);
	n = i;
	for (i = 0; i < n; i++)
		_dsp_free(&dsp_codec_drv->dsp_data[i]);

	return -ENOMEM;
}

static int dsp_resume(struct device *dev)
{
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);
	int ret, i;

	dev_info(dev, "%s\n", __func__);
	ret = clk_prepare_enable(dsp_codec_drv->emrclk);
	if (ret)
		return ret;
	ret = clk_prepare_enable(dsp_codec_drv->mclk);
	if (ret) {
		clk_disable_unprepare(dsp_codec_drv->emrclk);
		return ret;
	}

	for (i = 0; i < dsp_codec_drv->num_dsp; i++)
		_dsp_resume(&dsp_codec_drv->dsp_data[i],
			    dsp_codec_drv->auss_regmap);

	for (i = 0; i < dsp_codec_drv->num_dsp; i++)
		_dsp_free(&dsp_codec_drv->dsp_data[i]);

	return 0;
}
#endif

static const struct dev_pm_ops dsp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(dsp_suspend, dsp_resume)
};

static struct platform_driver ext_codec_dsp_driver = {
	.driver = {
		.name = "ext-codec-dsp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(ext_codec_dsp_dt_ids),
		.pm =		&dsp_pm_ops,
	},
	.probe =    ext_codec_dsp_probe,
	.remove =   ext_codec_dsp_remove,
};

module_platform_driver(ext_codec_dsp_driver);

MODULE_DESCRIPTION("SoC ext_codec_dsp driver");
MODULE_AUTHOR("Gian Antonio Sampietro gianantonio.sampietro@st.com");
MODULE_LICENSE("GPL");
