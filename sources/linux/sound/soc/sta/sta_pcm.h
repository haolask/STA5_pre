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

#ifndef STA10XX_PCM_H
#define STA10XX_PCM_H

#include <linux/platform_device.h>
#include <sound/dmaengine_pcm.h>

#define MSP_FRAMERATE_CONTROL 0

int sta_pcm_register(struct platform_device *pdev);

int sta_pcm_unregister(struct device *dev);

int sta_dmaengine_pcm_register(
	struct device *dev,
	const struct snd_dmaengine_pcm_config *config,
	unsigned int flags);
void sta_dmaengine_pcm_unregister(struct device *dev);

int sta_dmaengine_pcm_prepare_slave_config(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct dma_slave_config *slave_config);

#endif
