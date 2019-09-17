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

#define DRV_NAME "snd-soc-sta-card"

static int st_card_link_of_probe(
	struct platform_device *pdev,
	struct device_node *np,
	struct snd_soc_dai_link *dai_link)
{
	const char *codec_dai_name, *codec_name, *cpu_name, *stream_name;
	struct device_node *codec_np, *cpu_np;
	int ret, i;

	cpu_np = of_parse_phandle(np, "st,cpu-dai", 0);
	if (cpu_np) {
		dai_link->cpu_of_node = cpu_np;
		dev_dbg(&pdev->dev, "cpu_np: %s\n", cpu_np->name);
	} else {
		ret = of_property_read_string(
			np, "st,cpu-name", &cpu_name);
		if (ret) {
			dev_err(&pdev->dev, "cpu dai missing or invalid\n");
			return ret;
		}
		dai_link->cpu_name = cpu_name;
		dev_dbg(&pdev->dev, "cpu_name: %s\n", cpu_name);
	}
	ret = of_property_read_string(
			np, "st,stream-name", &stream_name);
	if (!ret) {
		dai_link->stream_name = stream_name;
		dai_link->platform_of_node = cpu_np;
	} else {
		dai_link->no_pcm = 1;
	}

	dai_link->name = np->name;

	dai_link->num_codecs =
		    of_property_count_strings(np, "st,codec-dai-name");
	dai_link->codecs = devm_kzalloc(&pdev->dev,
				dai_link->num_codecs *
				sizeof(struct snd_soc_dai_link_component),
				GFP_KERNEL);
	for (i = 0; i < dai_link->num_codecs; i++) {
		codec_np  = of_parse_phandle(np, "st,audio-codec", i);
		if (codec_np) {
			dai_link->codecs[i].of_node = codec_np;
			dev_dbg(&pdev->dev, "codec_np: %s\n", codec_np->name);
		} else {
			ret = of_property_read_string_index(
				np, "st,codec-name", i, &codec_name);
			if (ret) {
				dev_err(&pdev->dev, "codec %d missing\n", i);
				return ret;
			}
			dai_link->codecs[i].name = codec_name;
			dev_dbg(&pdev->dev, "codec_name: %s\n", codec_name);
		}

		ret = of_property_read_string_index(
			np, "st,codec-dai-name", i, &codec_dai_name);
		if (ret)
			return ret;

		dev_dbg(
			&pdev->dev, "codec_dai_name[%d]: %s\n",
			i, codec_dai_name);

		dai_link->codecs[i].dai_name = codec_dai_name;
	}

	return 0;
}

static int st_card_alsa_of_probe(
	struct platform_device *pdev,
	struct device_node *np)
{
	struct snd_soc_dai_link *dai_links;
	struct snd_soc_card *st_card_alsa_card;
	struct device_node *dev_np;
	int num_links, num_dev;
	int i = 0, ret;

	st_card_alsa_card = platform_get_drvdata(pdev);

	ret = snd_soc_of_parse_card_name(st_card_alsa_card, "st,card-name");
	if (ret)
		return ret;

	num_dev = of_get_child_count(np);
	num_links = num_dev ? : 1;
	dai_links = devm_kzalloc(
		&pdev->dev,
		num_links * sizeof(struct snd_soc_dai_link),
		GFP_KERNEL);

	if (!num_dev) { /* Legacy single device */
		ret = st_card_link_of_probe(pdev, np, &dai_links[0]);
		if (ret)
			return ret;
	} else {
		for_each_child_of_node(np, dev_np) {
			ret = st_card_link_of_probe(pdev, dev_np,
						    &dai_links[i++]);
			if (ret)
				return ret;
		}
	}

	st_card_alsa_card->num_links = num_links;
	st_card_alsa_card->dai_link = dai_links;

	dev_dbg(&pdev->dev, "alsa card registration\n");

	return 0;
}

static int st_card_alsa_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	static atomic_t dai_count;
	int ret = -EINVAL;
	int ndais;
	struct snd_soc_card *st_card_alsa_card;

	st_card_alsa_card = devm_kzalloc(
		&pdev->dev, sizeof(struct snd_soc_card), GFP_KERNEL);
	if (!st_card_alsa_card)
		return -ENOMEM;

	st_card_alsa_card->dev = &pdev->dev;
	platform_set_drvdata(pdev, st_card_alsa_card);

	if (np)
		ret = st_card_alsa_of_probe(pdev, np);

	if (ret) {
		dev_err(&pdev->dev,
			"Error: %s: Bad device tree parameters\n",
			__func__);
		return ret;
	}

	ndais = atomic_inc_return(&dai_count);
	pdev->id = ndais - 1;

	ret = devm_snd_soc_register_card(&pdev->dev, st_card_alsa_card);
	if (ret)
		dev_err(&pdev->dev,
			"Error: snd_soc_register_card failed (%d)!\n",
			ret);

	return ret;
}

static int st_card_alsa_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id sta_alsa_match[] = {
	{ .compatible = "st,snd-soc-sta-card-alsa", },
	{},
};

static struct platform_driver snd_soc_sta_alsa_driver = {
	.driver = {
		.name = DRV_NAME,
		.of_match_table = sta_alsa_match,
		.pm = &snd_soc_pm_ops,
	},
	.probe = st_card_alsa_probe,
	.remove = st_card_alsa_remove,
};

static int __init sta_card_alsa_init(void)
{
	return platform_driver_register(&snd_soc_sta_alsa_driver);
}
late_initcall(sta_card_alsa_init);

static void __exit sta_card_alsa_exit(void)
{
	platform_driver_unregister(&snd_soc_sta_alsa_driver);
}
module_exit(sta_card_alsa_exit);

MODULE_AUTHOR("Gabriele Simone gabriele.simone@st.com");
MODULE_DESCRIPTION("STA machine ASoC driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, sta_alsa_match);
