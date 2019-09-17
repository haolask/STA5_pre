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
#include <linux/mfd/syscon.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "sta_pcm.h"
#include "sta_msp_lut.h"
#include "sta_msp_dai.h"
#include "sta_msp_i2s.h"
#include "sta_audio_msg.h"

#define MAX_MSP_DAI_NAME 32

#define MSP_I2S_FORMATS	(SNDRV_PCM_FMTBIT_U8 | \
			 SNDRV_PCM_FMTBIT_S16_LE | \
			 SNDRV_PCM_FMTBIT_S32_LE)
#define MSP_MIN_CHANNELS	1
#define MSP_MAX_TDM_CHANNELS	8

/* MSP KControls */
static const char * const clk_mode[] = {"int", "ext"};
static const struct soc_enum clk_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(clk_mode), clk_mode);
static const char * const switch_mode[] = {"Off", "On"};
static const struct soc_enum switch_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(switch_mode), switch_mode);

static int master_clk_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	ucontrol->value.enumerated.item[0] =
		(msp->clk_sel != CLK_SEL_INTERNAL);

	return 0;
}

static int master_clk_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int val = ucontrol->value.integer.value[0];
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	switch (val) {
	case 0:
		if (PTR_ERR(msp->mspclk) != -ENOENT)
			msp->clk_sel = CLK_SEL_INTERNAL;
		break;
	case 1:
		if (PTR_ERR(msp->mspsck) != -ENOENT)
			msp->clk_sel = CLK_SEL_EXTERNAL;
		break;
	}

	return 0;
}

static int msp_i2s_get_rate(struct msp_drvdata *msp)
{
	u32 temp_reg, inputclk, sckdiv, period;

	temp_reg = readl(msp->registers + MSP_GCR);
	if (!(temp_reg & SRG_ENABLE) || msp->extref_bclk)
		return 0;

	inputclk = clk_get_rate(msp->genclk);
	temp_reg = readl(msp->registers + MSP_SRG);
	sckdiv = (temp_reg & SCK_DIV_MASK) + 1;
	period = (temp_reg >> FRPER_SHIFT) + 1;

	return DIV_ROUND_CLOSEST(inputclk, sckdiv * period);
}

static int msp_i2s_set_rate(struct msp_drvdata *msp, u32 rate)
{
	u32 temp_reg, target_inputclk, sckdiv, period;

	temp_reg = readl(msp->registers + MSP_GCR);
	if (!(temp_reg & SRG_ENABLE) || msp->extref_bclk)
		return 0;

	temp_reg = readl(msp->registers + MSP_SRG);
	sckdiv = (temp_reg & SCK_DIV_MASK) + 1;
	period = (temp_reg >> FRPER_SHIFT) + 1;
	target_inputclk = rate * sckdiv * period;

	return clk_set_rate(msp->genclk, target_inputclk);
}

static int msp_i2s_min_rate(struct msp_drvdata *msp)
{
	u32 temp_reg, inputclk, sckdiv, period;

	temp_reg = readl(msp->registers + MSP_GCR);
	if (!(temp_reg & SRG_ENABLE) || msp->extref_bclk)
		return 0;

	inputclk = clk_round_rate(msp->genclk, 0);
	temp_reg = readl(msp->registers + MSP_SRG);
	sckdiv = (temp_reg & SCK_DIV_MASK) + 1;
	period = (temp_reg >> FRPER_SHIFT) + 1;

	return DIV_ROUND_CLOSEST(inputclk, sckdiv * period);
}

static int msp_i2s_max_rate(struct msp_drvdata *msp)
{
	u32 temp_reg, inputclk, sckdiv, period;

	temp_reg = readl(msp->registers + MSP_GCR);
	if (!(temp_reg & SRG_ENABLE) || msp->extref_bclk)
		return 0;

	inputclk = clk_round_rate(msp->genclk, ULONG_MAX);
	temp_reg = readl(msp->registers + MSP_SRG);
	sckdiv = (temp_reg & SCK_DIV_MASK) + 1;
	period = (temp_reg >> FRPER_SHIFT) + 1;

	return DIV_ROUND_CLOSEST(inputclk, sckdiv * period);
}

static int msp_i2s_max_rate_ext(struct msp_drvdata *msp)
{
	u32 inputclk, sckdiv, period;

	inputclk = clk_get_rate(msp->genclk);
	period = msp->protdesc->frame_period;
	sckdiv = 1;

	return DIV_ROUND_CLOSEST(inputclk, sckdiv * period);
}

static int msp_rate_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);
	int rate;

	rate = msp_i2s_get_rate(msp);
	ucontrol->value.integer.value[0] = rate;

	return 0;
}

static int msp_rate_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);
	int rate = ucontrol->value.integer.value[0];

	return msp_i2s_set_rate(msp, rate);
}

static int auto_get(struct snd_kcontrol *kcontrol,
		    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	ucontrol->value.integer.value[0] = msp->autorate;

	return 0;
}

static int auto_put(struct snd_kcontrol *kcontrol,
		    struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	msp->autorate = ucontrol->value.integer.value[0];

	return 0;
}

static int msp_rate_force(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);
	int rate = ucontrol->value.integer.value[0];
	u32 temp_reg;

	if (rate && !msp->forced_clock) {
		msp->forced_clock++;
		ret = msp_dai_clk_enable(component->dev);
	} else if (rate == 0 && msp->forced_clock) {
		msp->forced_clock--;
		clk_disable_unprepare(msp->genclk);
		clk_disable_unprepare(msp->pclk);
	}

	if (rate == 0) {
		msp->forced_clock = 0;
		temp_reg = readl(msp->registers + MSP_GCR);
		temp_reg &= ~(FRAME_GEN_ENABLE | SRG_ENABLE);
		writel(temp_reg, msp->registers + MSP_GCR);
	} else{
		msp->frame_freq = rate;
		setup_clkpol(msp);
		setup_clkgen(msp);
		temp_reg = readl(msp->registers + MSP_GCR);
		temp_reg |= (FRAME_GEN_ENABLE | SRG_ENABLE);
		writel(temp_reg, msp->registers + MSP_GCR);
		msp->forced_clock = 1;
	}

	return 0;
}

static inline int msp_rate_range(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = msp_i2s_min_rate(msp);
	uinfo->value.integer.max = msp_i2s_max_rate(msp);

	return 0;
}

static inline int msp_rate_range_ext(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = msp_i2s_max_rate_ext(msp);

	return 0;
}

static inline int msp_rate_step_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 10;

	return 0;
}

static int msp_rate_step_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	ucontrol->value.integer.value[0] = msp->rate_delta;

	return 0;
}

static int msp_rate_step_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	msp->rate_delta = ucontrol->value.integer.value[0];

	return 0;
}

static inline int msp_thr_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = MAX_THR;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 100;

	return 0;
}

static int msp_thr_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);
	int i;

	for (i = 0; i < msp->num_thr; i++)
		ucontrol->value.integer.value[i] = msp->thr[i];

	return 0;
}

static int msp_thr_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);
	int i;

	for (i = 0; i < MAX_THR; i++) {
		if (ucontrol->value.integer.value[i] == 0 ||
		    (i && (ucontrol->value.integer.value[i] <=
			   ucontrol->value.integer.value[i - 1]))) {
			msp->num_thr = i;
			break;
		}
		msp->thr[i] = ucontrol->value.integer.value[i];
	}
	for (i = msp->num_thr; i < MAX_THR; i++)
		msp->thr[i] = 0;

	return 0;
}

static int m3_play_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	ucontrol->value.enumerated.item[0] = msp->m3_busy;

	return 0;
}

static int m3_play_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int val = ucontrol->value.integer.value[0];
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	switch (val) {
	case 0:
		reinit_completion(&msp->m3_wait);
		st_codec_early_audio_send(EARLY_AUDIO_END_PLAY);
		return wait_for_completion_timeout(&msp->m3_wait,
						   msecs_to_jiffies(500));
	}

	return -EINVAL;
}

static int loopback_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	ucontrol->value.enumerated.item[0] = msp->loopback_enable;

	return 0;
}

static int loopback_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct msp_drvdata *msp =
		(struct msp_drvdata *)dev_get_drvdata(component->dev);

	msp->loopback_enable = ucontrol->value.enumerated.item[0];

	return 0;
}

static struct snd_kcontrol_new msp_jitter_controls[] = {
	SOC_RANGE("Rate", msp_rate_range, msp_rate_get, msp_rate_put),
	SOC_ENUM_EXT("Autorate", switch_enum, auto_get, auto_put),
	SOC_RANGE("Thresholds", msp_thr_info, msp_thr_get, msp_thr_put),
	SOC_RANGE("Rate Step", msp_rate_step_info,
		  msp_rate_step_get, msp_rate_step_put),
};

static struct snd_kcontrol_new msp_audio_controls[] = {
	SOC_ENUM_EXT("Clock", clk_enum, master_clk_get, master_clk_put),
	SOC_RANGE("Force Rate", msp_rate_range_ext, msp_rate_get,
		  msp_rate_force),
	SOC_ENUM_EXT("M3 play", switch_enum, m3_play_get, m3_play_put),
	SOC_ENUM_EXT("Loopback", switch_enum, loopback_get, loopback_put),
};

/* End MSP KControls */

static void msp_i2s_tx(
	struct msp_drvdata *msp)
{
	u32 temp_reg = 0;
	struct msp_protdesc *protdesc = msp->protdesc;

	temp_reg |= msp_elen(protdesc->tx_elem_len_1) << TP1ELEN_SHIFT;
	temp_reg |= TX_FRAME_1(protdesc->tx_frame_len_1) << TP1FLEN_SHIFT;
	temp_reg |= (protdesc->compression_mode << TDTYP_SHIFT);
	temp_reg |= (protdesc->tx_byte_order << TENDN_SHIFT);
	temp_reg |= (protdesc->tx_data_delay << TDDLY_SHIFT);
	temp_reg |= (protdesc->frame_sync_ignore << TFSIG_SHIFT);
	if (protdesc->tx_frame_len_2) {
		temp_reg |= msp_elen(protdesc->tx_elem_len_2) << TP2ELEN_SHIFT;
		temp_reg |= TX_FRAME_2(protdesc->tx_frame_len_2)
				<< TP2FLEN_SHIFT;
		temp_reg |= (1 << TP2SM_SHIFT);
		temp_reg |= BIT(TP2EN_SHIFT);
	}
	temp_reg |= (protdesc->tx_half_word_swap << TBSWAP_SHIFT);

	dev_dbg(
		msp->dev,
		"%s: MSP_TCF = 0x%08x\n",
		__func__, temp_reg);

	writel(temp_reg, msp->registers + MSP_TCF);
}

static void msp_i2s_rx(
	struct msp_drvdata *msp)
{
	u32 temp_reg = 0;
	struct msp_protdesc *protdesc = msp->protdesc;

	temp_reg |= msp_elen(protdesc->rx_elem_len_1) << RP1ELEN_SHIFT;
	temp_reg |= RX_FRAME_1(protdesc->rx_frame_len_1) << RP1FLEN_SHIFT;
	temp_reg |= (protdesc->compression_mode << RDTYP_SHIFT);
	temp_reg |= (protdesc->rx_byte_order << RENDN_SHIFT);
	temp_reg |= (protdesc->rx_data_delay << RDDLY_SHIFT);
	temp_reg |= (protdesc->frame_sync_ignore << RFSIG_SHIFT);
	if (protdesc->rx_frame_len_2) {
		temp_reg |= msp_elen(protdesc->rx_elem_len_2) << RP2ELEN_SHIFT;
		temp_reg |= RX_FRAME_2(protdesc->rx_frame_len_2)
				<< RP2FLEN_SHIFT;
		temp_reg |= (1 << RP2SM_SHIFT);
		temp_reg |= BIT(RP2EN_SHIFT);
	}
	temp_reg |= (protdesc->rx_half_word_swap << RBSWAP_SHIFT);

	dev_dbg(
		msp->dev,
		"%s: MSP_RCF = 0x%08x\n",
		__func__, temp_reg);

	writel(temp_reg, msp->registers + MSP_RCF);
}

static int msp_tdm_tx(
	struct msp_drvdata *msp)
{
	u32 temp_reg;

	/* enable multichannel */
	temp_reg = readl(msp->registers + MSP_MCR);
	temp_reg |= TMCEN_BIT;
	writel(temp_reg, msp->registers + MSP_MCR);

	/* enable channels according to hw params */
	writel(msp->tdm_mask[0], msp->registers + MSP_TCE0);
	writel(msp->tdm_mask[1], msp->registers + MSP_TCE1);
	writel(msp->tdm_mask[2], msp->registers + MSP_TCE2);
	writel(msp->tdm_mask[3], msp->registers + MSP_TCE3);

	return 0;
}

static int msp_tdm_rx(
	struct msp_drvdata *msp)
{
	u32 temp_reg;

	/* enable multichannel */
	temp_reg = readl(msp->registers + MSP_MCR);
	temp_reg |= RMCEN_BIT;
	writel(temp_reg, msp->registers + MSP_MCR);

	/* enable channels according to hw params */
	writel(msp->tdm_mask[0], msp->registers + MSP_RCE0);
	writel(msp->tdm_mask[1], msp->registers + MSP_RCE1);
	writel(msp->tdm_mask[2], msp->registers + MSP_RCE2);
	writel(msp->tdm_mask[3], msp->registers + MSP_RCE3);
	return 0;
}

static void msp_dma_enable_tx(
	struct msp_drvdata *msp)
{
	u32 reg_val_DMACR;

	reg_val_DMACR = readl(msp->registers + MSP_DMACR);
	reg_val_DMACR |= TX_DMA_ENABLE;
	writel(reg_val_DMACR, msp->registers + MSP_DMACR);
}

static void msp_dma_enable_rx(
	struct msp_drvdata *msp)
{
	u32 reg_val_DMACR;

	reg_val_DMACR = readl(msp->registers + MSP_DMACR);
	reg_val_DMACR |= RX_DMA_ENABLE;
	writel(reg_val_DMACR, msp->registers + MSP_DMACR);
}

static void flush_fifo_rx(struct msp_drvdata *msp)
{
	u32 reg_val_DR, reg_val_FLR;
	u32 limit = 32;

	reg_val_FLR = readl(msp->registers + MSP_FLR);
	while (!(reg_val_FLR & RX_FIFO_EMPTY) && limit--) {
		reg_val_DR = readl(msp->registers + MSP_DR);
		reg_val_FLR = readl(msp->registers + MSP_FLR);
	}
}

static void flush_fifo_tx(struct msp_drvdata *msp)
{
	u32 reg_val_TSTDR, reg_val_FLR;
	u32 limit = 32;

	writel(MSP_ITCR_TESTFIFO, msp->registers + MSP_ITCR);

	reg_val_FLR = readl(msp->registers + MSP_FLR);
	while (!(reg_val_FLR & TX_FIFO_EMPTY) && limit--) {
		reg_val_TSTDR = readl(msp->registers + MSP_TSTDR);
		reg_val_FLR = readl(msp->registers + MSP_FLR);
	}
	writel(0x0, msp->registers + MSP_ITCR);
}

static irqreturn_t msp_i2s_irq(int irq, void *arg)
{
	u32 reg_val;
	struct msp_drvdata *msp = arg;

	reg_val = readl(msp->registers + MSP_MIS);

	writel(reg_val, msp->registers + MSP_ICR);

	if (reg_val & ALL_ERR) {
		msp->irq_status = reg_val;
		return IRQ_WAKE_THREAD;
	}
	return IRQ_HANDLED;
}

static irqreturn_t msp_i2s_irq_thread(int irq, void *arg)
{
	u32 reg_val;
	struct msp_drvdata *msp = arg;

	reg_val = msp->irq_status;
	msp->irq_errors++;

	if (reg_val & TX_UNDERRUN_ERR_INT)
		dev_err(msp->dev, "MSP Transmit Underrun Error\n");
	if (reg_val & RX_OVERRUN_ERROR_INT)
		dev_err(msp->dev, "MSP Receive Overrun Error\n");
	if (reg_val & TX_FSYNC_ERR_INT)
		dev_err(msp->dev, "MSP Transmit Frame Sync Error\n");
	if (reg_val & RX_FSYNC_ERR_INT)
		dev_err(msp->dev, "MSP Receive Frame Sync Error\n");

	if (msp->irq_errors > 30) {
		dev_err(msp->dev, "MSP too many errors, disable interrupt\n");
		reg_val = readl(msp->registers + MSP_IMSC);
		writel(
			reg_val & ~ALL_ERR,
			msp->registers + MSP_IMSC);
	}
	return IRQ_HANDLED;
}

static int msp_protdesc_of_probe(struct msp_drvdata *msp)
{
	int i, ret;
	struct device_node *np = msp->dev->of_node;
	struct device_node *nc;
	struct dts_key *dts_key;
	s32 val;
	const char *strval;

	/* MSP protocol settings */
	nc = of_get_child_by_name(np, "i2s");

	for (i = 0; nc && i < ARRAY_SIZE(msp_dts_keys_lut); i++) {
		dts_key = (struct dts_key *)msp_dts_keys_lut[i].value;
		if (!dts_key->lut) {
			ret = of_property_read_u32(
				nc, msp_dts_keys_lut[i].name, &val);
			if (ret < 0)
				continue;
		} else {
			ret = of_property_read_string(
				nc, msp_dts_keys_lut[i].name, &strval);
			if (ret < 0)
				continue;

			ret = msp_dts_lut_match(
				dts_key->lut, dts_key->lut_length,
				strval, &val);
			if (ret < 0) {
				dev_err(msp->dev,
					"%s: invalid value %s\n",
					msp_dts_keys_lut[i].name,
					(char *)strval);
				return ret;
			}
		}
		dev_dbg(msp->dev, "%s: %d\n", msp_dts_keys_lut[i].name, val);
		*(u32 *)((int)msp->protdesc + dts_key->field_offset) = val;
	}

	return 0;
}

static int msp_dai_of_probe(struct platform_device *pdev)
{
	struct msp_drvdata *msp = dev_get_drvdata(&pdev->dev);
	struct device_node *np = pdev->dev.of_node;
	int ret;
	int i, cnt;
	s32 val;

	ret = of_property_read_u32(
		np, "enable-loopback", &msp->loopback_enable);
	if (ret < 0)
		msp->loopback_enable = 0;

	ret = of_property_read_string(
		np, "prefix", &msp->prefix);

	cnt = of_property_count_u32_elems(np, "refclk");
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_u32_index(np, "refclk", i, &val);
		if (ret < 0)
			return ret;
		msp->msp_extrefclk_mask |= BIT(val);
	}

	msp->protdesc = devm_kmemdup(
		&pdev->dev, &protdesc_default, sizeof(protdesc_default),
		GFP_KERNEL);
	if (!msp->protdesc)
		return -ENOMEM;

	return msp_protdesc_of_probe(msp);
}

static int get_refclk(int samplerate, u32 *pclk)
{
	switch (samplerate) {
	case 8000:
	case 16000:
	case 32000:
	case 12000:
	case 24000:
	case 48000:
	case 96000:
		*pclk = 159744000;
		break;
	case 11025:
	case 22050:
	case 44100:
		*pclk = 158054400;
		break;
	default:
		return -EINVAL;
	}

	return 0;
};

static void setup_clkpol(struct msp_drvdata *msp)
{
	struct msp_protdesc *protdesc = msp->protdesc;
	u32 temp_reg;

	temp_reg = readl(msp->registers + MSP_GCR);

	set_mask_bits(
		&temp_reg,
		RX_FSYNC_POL_MASK,
		protdesc->rx_fsync_pol == I2S_ACTIVE_LOW ?
		RX_FSYNC_ACTIVE_LOW : RX_FSYNC_ACTIVE_HIGH);

	set_mask_bits(
		&temp_reg,
		TX_FSYNC_POL_MASK,
		protdesc->tx_fsync_pol == I2S_ACTIVE_LOW ?
		TX_FSYNC_ACTIVE_LOW : TX_FSYNC_ACTIVE_HIGH);

	set_mask_bits(
		&temp_reg,
		RX_CLK_POL_MASK,
		protdesc->rx_clk_pol == I2S_FALLING ?
		RX_CLK_FALLING : RX_CLK_RISING);

	set_mask_bits(
		&temp_reg,
		TX_CLK_POL_MASK,
		protdesc->tx_clk_pol == I2S_FALLING ?
		TX_CLK_FALLING : TX_CLK_RISING);

	set_mask_bits(
		&temp_reg,
		TX_FSYNC_SEL_MASK,
		protdesc->tx_fsync_sel == I2S_MASTER ?
		TX_FSYNC_FGEN : TX_FSYNC_EXT);

	set_mask_bits(
		&temp_reg,
		RX_FSYNC_SEL_MASK,
		protdesc->rx_fsync_sel == I2S_MASTER ?
		RX_FSYNC_FGEN : RX_FSYNC_EXT);

	set_mask_bits(
		&temp_reg,
		TX_CLK_SEL_MASK,
		protdesc->tx_clk_sel == I2S_MASTER ?
		TX_CLK_SGEN : TX_CLK_EXT);

	set_mask_bits(
		&temp_reg,
		RX_CLK_SEL_MASK,
		protdesc->rx_clk_sel == I2S_MASTER ?
		RX_CLK_SGEN : RX_CLK_EXT);

	set_mask_bits(
		&temp_reg,
		SRG_CLK_SEL_MASK,
		protdesc->srg_clk_sel == I2S_INTERNAL ?
		SRG_CLK_INT : SRG_CLK_EXT);

	set_mask_bits(
		&temp_reg,
		SRG_CLK_POL_MASK,
		protdesc->srg_clk_pol == I2S_FALLING ?
		SRG_CLK_FALLING : SRG_CLK_RISING);

	writel(temp_reg, msp->registers + MSP_GCR);
}

static int setup_clkgen(struct msp_drvdata *msp)
{
	u32 sck_div = 0;
	u32 temp_reg = 0;
	u32 inputclk;
	struct msp_protdesc *protdesc = msp->protdesc;

	temp_reg = readl(msp->registers + MSP_GCR);

	set_mask_bits(
		    &temp_reg,
		    SRG_CLK_SEL_MASK,
		    protdesc->srg_clk_sel == I2S_INTERNAL ?
		    SRG_CLK_INT : SRG_CLK_EXT);

	set_mask_bits(
		    &temp_reg,
		    SRG_CLK_POL_MASK,
		    protdesc->srg_clk_pol == I2S_FALLING ?
		    SRG_CLK_FALLING : SRG_CLK_RISING);

	writel(temp_reg, msp->registers + MSP_GCR);

	if (!msp->fixed_clock && !msp->extref_bclk &&
	    msp->genclk == msp->mspsck) {
		if (get_refclk(msp->frame_freq, &inputclk) < 0) {
			dev_err(
				msp->dev,
				"inputclk not defined for samplerate %d\n",
				msp->frame_freq);
			return -EINVAL;
		}
		if (clk_set_rate(msp->genclk, inputclk)) {
			dev_err(
				msp->dev,
				"cannot set clk at %u\n", inputclk);
			return -EINVAL;
		}
	}

	if (!msp->extref_bclk) {
		inputclk = clk_get_rate(msp->genclk);

		sck_div = DIV_ROUND_CLOSEST(
			inputclk,
			(msp->frame_freq * protdesc->frame_period));
		dev_dbg(
			msp->dev,
			"%s: SRG (0x%08x) SCK_DIV=(%d) = %d / (%d * %d)\n",
			__func__,
			temp_reg,
			sck_div,
			inputclk,
			msp->frame_freq,
			protdesc->frame_period);

	} else {
		dev_dbg(msp->dev, "sck_div = 1\n");
		sck_div = 1;
	}

	dev_dbg(msp->dev, "sck_div %d, per %d wid %d\n",
		    sck_div - 1, protdesc->frame_period, protdesc->frame_width);

	temp_reg = (sck_div - 1) & SCK_DIV_MASK;
	temp_reg |= FRAME_WIDTH_BITS(protdesc->frame_width);
	temp_reg |= FRAME_PERIOD_BITS(protdesc->frame_period);
	writel(temp_reg, msp->registers + MSP_SRG);

	return 0;
}

static void disable_msp_rx(struct msp_drvdata *msp)
{
	u32 reg_val_GCR, reg_val_DMACR, reg_val_IMSC;

	reg_val_IMSC = readl(msp->registers + MSP_IMSC);
	writel(
		reg_val_IMSC & ~(RX_SERVICE_INT | RX_OVERRUN_ERROR_INT),
		msp->registers + MSP_IMSC);

	reg_val_GCR = readl(msp->registers + MSP_GCR);
	writel(reg_val_GCR & ~RX_ENABLE, msp->registers + MSP_GCR);
	reg_val_DMACR = readl(msp->registers + MSP_DMACR);
	writel(reg_val_DMACR & ~RX_DMA_ENABLE, msp->registers + MSP_DMACR);

	msp->dir_busy &= ~MSP_DIR_RX;
}

static void disable_msp_tx(struct msp_drvdata *msp)
{
	u32 reg_val_GCR, reg_val_DMACR, reg_val_IMSC;
	u32 limit = loops_per_jiffy << 1;

	reg_val_IMSC = readl(msp->registers + MSP_IMSC);
	writel(
		reg_val_IMSC & ~(TX_SERVICE_INT | TX_UNDERRUN_ERR_INT),
		msp->registers + MSP_IMSC);
	writel(0xFFFFFFFF, msp->registers + MSP_ICR);

	/* flush out the tx fifo before disable the transmitter */
	while (!(readl(msp->registers + MSP_FLR) & TX_FIFO_EMPTY) && limit--)
		cpu_relax();

	reg_val_GCR = readl(msp->registers + MSP_GCR);
	writel(reg_val_GCR & ~TX_ENABLE, msp->registers + MSP_GCR);
	reg_val_DMACR = readl(msp->registers + MSP_DMACR);
	writel(reg_val_DMACR & ~TX_DMA_ENABLE, msp->registers + MSP_DMACR);

	msp->dir_busy &= ~MSP_DIR_TX;
}

static int disable_msp(struct msp_drvdata *msp, unsigned int dir)
{
	u32 reg_val_GCR;
	unsigned int disable_tx, disable_rx;

	reg_val_GCR = readl(msp->registers + MSP_GCR);
	disable_tx = dir & MSP_DIR_TX;
	disable_rx = dir & MSP_DIR_RX;
	if (disable_tx && disable_rx) {
		reg_val_GCR = readl(msp->registers + MSP_GCR);
		writel(
			reg_val_GCR | LOOPBACK_MASK,
			msp->registers + MSP_GCR);

		/* Flush TX-FIFO */
		flush_fifo_tx(msp);

		/* Disable TX-channel */
		writel((readl(
				msp->registers + MSP_GCR) &
			       (~TX_ENABLE)), msp->registers + MSP_GCR);

		/* Flush RX-FIFO */
		flush_fifo_rx(msp);

		/* Disable Loopback and Receive channel */
		writel((readl(
				msp->registers + MSP_GCR) &
				(~(RX_ENABLE | LOOPBACK_MASK))),
				msp->registers + MSP_GCR);

		disable_msp_tx(msp);
		disable_msp_rx(msp);
	} else if (disable_tx) {
		disable_msp_tx(msp);
	} else if (disable_rx) {
		disable_msp_rx(msp);
	}

	return 0;
}

static int msp_dai_clk_enable(struct device *dev)
{
	int ret;
	struct msp_drvdata *msp = dev_get_drvdata(dev);

	/* Prepare and enable clocks */
	ret = clk_prepare_enable(msp->pclk);
	if (ret) {
		dev_err(dev,
			"%s: ERROR: Failed to prepare/enable pclk!\n",
			__func__);
		return ret;
	}

	if (msp->clk_sel == CLK_SEL_INTERNAL)
		msp->genclk = msp->mspclk;
	else
		msp->genclk = msp->mspsck;

	ret = clk_prepare_enable(msp->genclk);
	if (ret) {
		dev_err(dev,
			"%s: ERROR: Failed to prepare/enable mspclk!\n",
			__func__);
		goto err_genclk;
	}

	return 0;

err_genclk:
	clk_disable_unprepare(msp->pclk);
	return ret;
}

static int msp_dai_startup(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int ret = 0;
	struct msp_drvdata *msp = dev_get_drvdata(dai->dev);
	int dir = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ?
			   MSP_DIR_TX : MSP_DIR_RX);
	unsigned int tx_sel, rx_sel, tx_busy, rx_busy;
	u32 temp;

	if (in_interrupt()) {
		dev_err(
			msp->dev,
			"%s: ERROR: Open called in interrupt context!\n",
			__func__);
		return -1;
	}
	if (dir & MSP_DIR_TX)
		msp->substream_tx = substream;
	if (dir & MSP_DIR_RX)
		msp->substream_rx = substream;

	tx_sel = (dir & MSP_DIR_TX) > 0;
	rx_sel = (dir & MSP_DIR_RX) > 0;
	if (!tx_sel && !rx_sel) {
		dev_err(msp->dev, "%s: ERROR: No direction selected!\n",
			__func__);
		return -EINVAL;
	}

	tx_busy = (msp->dir_busy & MSP_DIR_TX) > 0;
	rx_busy = (msp->dir_busy & MSP_DIR_RX) > 0;
	if (tx_busy && tx_sel) {
		dev_err(msp->dev, "%s: ERROR: TX is in use!\n", __func__);
		return -EBUSY;
	}
	if (rx_busy && rx_sel) {
		dev_err(msp->dev, "%s: ERROR: RX is in use!\n", __func__);
		return -EBUSY;
	}

	msp->dir_busy |= (tx_sel ? MSP_DIR_TX : 0) | (rx_sel ? MSP_DIR_RX : 0);
	msp->irq_errors = 0;

	snd_pcm_hw_constraint_minmax(
		substream->runtime,
		SNDRV_PCM_HW_PARAM_CHANNELS,
		MSP_MIN_CHANNELS,
		msp->protdesc->max_channels);

	ret = msp_dai_clk_enable(dai->dev);
	if (ret < 0)
		return ret;

	if (msp->msp_extrefclk_mask & MSP1REFCLKINEN0) {
		ret = regmap_read(msp->auss_regmap, AUSS_MUX_CR,
				  &temp);
		if (ret < 0)
			return ret;

		msp->extref_bclk = (msp->clk_sel == CLK_SEL_EXTERNAL) &&
				    !(temp & MSP1REFCLKINEN0);
	}

	return ret;
}

static void msp_dai_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct msp_drvdata *msp = dev_get_drvdata(dai->dev);
	bool is_playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	int dir = is_playback ? MSP_DIR_TX : MSP_DIR_RX;
	int status = 0;

	status = disable_msp(msp, dir);
	if (dir & MSP_DIR_TX)
		msp->substream_tx = NULL;
	if (dir & MSP_DIR_RX)
		msp->substream_rx = NULL;
	if (msp->dir_busy == 0) {
		/* disable sample rate and frame generators */
		if (!msp->forced_clock) {
			writel(0, msp->registers + MSP_GCR);
			writel(0, msp->registers + MSP_SRG);
		}

		writel(0, msp->registers + MSP_TCF);
		writel(0, msp->registers + MSP_RCF);
		writel(0, msp->registers + MSP_DMACR);
		writel(0, msp->registers + MSP_MCR);
		writel(0, msp->registers + MSP_RCM);
		writel(0, msp->registers + MSP_RCV);
		writel(0, msp->registers + MSP_TCE0);
		writel(0, msp->registers + MSP_TCE1);
		writel(0, msp->registers + MSP_TCE2);
		writel(0, msp->registers + MSP_TCE3);
		writel(0, msp->registers + MSP_RCE0);
		writel(0, msp->registers + MSP_RCE1);
		writel(0, msp->registers + MSP_RCE2);
		writel(0, msp->registers + MSP_RCE3);
		writel(0, msp->registers + MSP_IMSC);
	}

	clk_disable_unprepare(msp->genclk);
	clk_disable_unprepare(msp->pclk);
}

static int msp_dai_prepare(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct msp_drvdata *msp = dev_get_drvdata(dai->dev);

	if (msp->autorate) {
		msp->target = msp->frame_freq;
		msp->cur_thr = MSP_LEV_UNDEF;
	}

	return 0;
}

static void msp_params_to_protdesc(struct msp_protdesc *protdesc,
				   struct msp_params *params,
				   int direction)
{
	if (direction == MSP_DIR_TX) {
		protdesc->tx_elem_len_1 = params->elem_len_1;
		protdesc->tx_frame_len_1 = params->frame_len_1;
		protdesc->tx_elem_len_2 = params->elem_len_2;
		protdesc->tx_frame_len_2 = params->frame_len_2;
		protdesc->tx_half_word_swap = params->half_word_swap;
	} else {
		protdesc->rx_elem_len_1 = params->elem_len_1;
		protdesc->rx_frame_len_1 = params->frame_len_1;
		protdesc->rx_elem_len_2 = params->elem_len_2;
		protdesc->rx_frame_len_2 = params->frame_len_2;
		protdesc->rx_half_word_swap = params->half_word_swap;
	}
	protdesc->frame_period = params->frame_period;
	protdesc->frame_width = params->frame_width;
}

static int msp_dai_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params,
	struct snd_soc_dai *dai)
{
	struct msp_drvdata *msp = dev_get_drvdata(dai->dev);
	struct snd_dmaengine_dai_dma_data *dma_data;
	int channels = params_channels(hw_params);
	struct msp_protdesc *protdesc = msp->protdesc;
	struct msp_params msp_params;
	int direction = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ?
			 MSP_DIR_TX : MSP_DIR_RX);
	u32 temp_reg;
	int datalen;
	int ch, slot;

	dma_data = substream->stream == SNDRV_PCM_STREAM_PLAYBACK ?
		&msp->playback_dma_data : &msp->capture_dma_data;

	/* Configure DMA */
	switch (params_format(hw_params)) {
	case SNDRV_PCM_FORMAT_U8:
		dma_data->addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		datalen = 8;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		dma_data->addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		datalen = 16;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		dma_data->addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		datalen = 32;
		break;
	default:
		dev_err(dai->dev, "format %d not supported\n",
			params_format(hw_params));
		return -EINVAL;
	}
	dma_data->maxburst = 4;

	if (datalen == 16 && channels == 2) {
		/* 2 x 16bits samples packed in 1 x 32 FIFO element */
		msp_params.frame_period = 32;
		msp_params.frame_width = 16;
		msp_params.frame_len_1 = 1;
		msp_params.elem_len_1 = 32;
		msp_params.frame_len_2 = 0;
		msp_params.elem_len_2 = 0;
		/* In FIFO samples are in the following order:
		 * LSB left, MSB left, LSB right, MSB right
		 * MSB right is shifted out first by default.
		 * To have left first, we need to set half-word swap.
		 */
		dma_data->addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		msp_params.half_word_swap = 3;
	} else if (datalen == 16 && channels == 1 && direction == MSP_DIR_TX) {
		/* DMA width 16bit duplicates samples in 32bit FIFO
		 * To send mono we should config MSP 1 elem x 16bit
		 * But we're happy to play mono as stereo,
		 * so let's keep 1 elem x 32bit
		 */
		msp_params.frame_period = 32;
		msp_params.frame_width = 16;
		msp_params.frame_len_1 = 1;
		msp_params.elem_len_1 = 32;
		msp_params.frame_len_2 = 0;
		msp_params.elem_len_2 = 0;
		msp_params.half_word_swap = 0;
	} else if (channels <= 2) {
		msp_params.frame_width = datalen;
		msp_params.frame_period = datalen * 2;
		/* In all the other cases use 1st phase for left
		 * and 2nd phase for right
		 */
		msp_params.elem_len_1 = datalen;
		msp_params.frame_len_1 = 1;
		msp_params.frame_len_2 = 0;
		msp_params.elem_len_2 = 0;
		if (channels == 2) {
			msp_params.frame_len_2 = 1;
			msp_params.elem_len_2 = datalen;
		}
		msp_params.half_word_swap = 0;

	} else {
		msp_params.frame_width = 1;
		msp_params.frame_period = datalen * channels;
		msp_params.elem_len_1 = datalen;
		msp_params.frame_len_2 = 0;
		msp_params.elem_len_2 = 0;
		msp_params.half_word_swap = 0;
		msp_params.frame_len_1 = channels;
		memset(msp->tdm_mask, 0, sizeof(msp->tdm_mask));
		for (ch = slot = 0;
			 ch < channels && slot < msp_params.frame_len_1;
			 ch++, slot++) {
			if (protdesc->chord == MSP_CHORD_ALL_LEFT_FIRST &&
			    (channels > 1) &&
			    (ch == (channels / 2) ||
			     (slot == (msp_params.frame_len_1 / 2)))) {
				/* jump to right channels/slots */
				slot = msp_params.frame_len_1 / 2;
				ch = channels / 2;
			}
			msp->tdm_mask[slot / 32] |= (1 << (slot % 32));
		}
		dma_data->addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	}
	snd_soc_dai_set_dma_data(dai, substream, dma_data);

	msp_params_to_protdesc(protdesc, &msp_params, direction);

	/* DTS static settings override hw_params*/
	msp_protdesc_of_probe(msp);

	/* Clock control overrides DTS settings */
	if (msp->clk_sel == CLK_SEL_INTERNAL)
		protdesc->srg_clk_sel = I2S_INTERNAL;
	else
		protdesc->srg_clk_sel = I2S_EXTERNAL;

	if (
		protdesc->tx_fsync_sel == I2S_MASTER ||
		protdesc->rx_fsync_sel == I2S_MASTER ||
		protdesc->tx_clk_sel == I2S_MASTER ||
		protdesc->rx_clk_sel == I2S_MASTER)
		msp->ms_mode = I2S_MASTER;
	else
		msp->ms_mode = I2S_SLAVE;

	msp->tx_fifo_config = TX_FIFO_ENABLE;
	msp->rx_fifo_config = RX_FIFO_ENABLE;

	msp->frame_freq = params_rate(hw_params);

	setup_clkpol(msp);
	if (msp->ms_mode == I2S_MASTER && !msp->forced_clock)
		setup_clkgen(msp);

	temp_reg = readl(msp->registers + MSP_GCR);
	temp_reg &= ~TX_EXTRA_DELAY_MASK;
	temp_reg |= (protdesc->tx_extra_delay << TXDDL_SHIFT);

	temp_reg &= ~LOOPBACK_MASK;
	temp_reg |= (msp->loopback_enable << LBM_SHIFT);

	temp_reg &= ~RX_FIFO_ENABLE_MASK;
	temp_reg |= (msp->rx_fifo_config << RFFEN_SHIFT);

	temp_reg &= ~TX_FIFO_ENABLE_MASK;
	temp_reg |= (msp->tx_fifo_config << TFFEN_SHIFT);

	usleep_range(100, 101);
	writel(temp_reg, msp->registers + MSP_GCR);

	if (direction & MSP_DIR_TX) {
		msp_i2s_tx(msp);
		if (channels > 2)
			msp_tdm_tx(msp);
	}
	if (direction & MSP_DIR_RX) {
		msp_i2s_rx(msp);
		if (channels > 2)
			msp_tdm_rx(msp);
		flush_fifo_rx(msp);
	}

	msp->iodelay = 0x40;
	writel(msp->iodelay, msp->registers + MSP_IODLY);

	dev_dbg(
		msp->dev,
		"%s %d: GCR: 0x%08x\n",
		__func__, __LINE__, temp_reg);

	return 0;
}

static int msp_dai_set_tdm_slot(
	struct snd_soc_dai *dai,
	unsigned int tx_mask, unsigned int rx_mask,
	int slots, int slot_width)
{
	return 0;
}

static int msp_dai_trigger(
	struct snd_pcm_substream *substream,
	int cmd, struct snd_soc_dai *dai)
{
	int ret = 0;
	struct msp_drvdata *msp = dev_get_drvdata(dai->dev);
	u32 reg_val_GCR, enable_bit, reg_val_IMSC, enable_interrupt;
	int direction = substream->stream;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (direction == SNDRV_PCM_STREAM_PLAYBACK) {
			enable_bit = TX_ENABLE;
			enable_interrupt = TX_UNDERRUN_ERR_INT;
			msp_dma_enable_tx(msp);
		} else {
			enable_bit = RX_ENABLE;
			enable_interrupt = RX_OVERRUN_ERROR_INT;
			msp_dma_enable_rx(msp);
		}

		reg_val_GCR = readl(msp->registers + MSP_GCR);
		if (msp->ms_mode == I2S_MASTER) {
			reg_val_GCR |= SRG_ENABLE;
			reg_val_GCR |= FRAME_GEN_ENABLE;
		}
		writel(
			reg_val_GCR | enable_bit,
			msp->registers + MSP_GCR);

		writel(0xFFFFFFFF, msp->registers + MSP_ICR);
		reg_val_IMSC = readl(msp->registers + MSP_IMSC);
		writel(
			reg_val_IMSC | enable_interrupt,
			msp->registers + MSP_IMSC);

		if (msp->autorate && !msp->msp_run)
			hrtimer_start(&msp->hr_timer, ms_to_ktime(0),
				      HRTIMER_MODE_REL);

		msp->msp_run++;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		msp->msp_run--;
		if (!msp->msp_run)
			hrtimer_cancel(&msp->hr_timer);
		if (direction == SNDRV_PCM_STREAM_PLAYBACK)
			disable_msp_tx(msp);
		else
			disable_msp_rx(msp);
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

/*
 * Automatic Rate control based on  the filling level of the ALSA buffer.
 * msp->thr is an array of thresholds, ordered from low to high.
 * The rate is increased/decreased by a delta every time it exceeds a threshold
 * and then it comes back to previous rate with hysteresis.
 * The middle threshold(s) is the target: when crossing it the rate goes back
 * to target as well.
 * Every time a threshold is exceeded, the target rate is finely adjusted,
 * trying to converge to the ideal rate.
 */
static void jitter_control(struct work_struct *work)
{
	struct msp_drvdata *msp = container_of(work, struct msp_drvdata, task);
	int rate, cur_rate;
	int delta = msp->frame_freq * msp->rate_delta / 1000;
	int i;		/* index of msp->thr */
	int h, l;	/* index of lower/higher halves of msp->thr */
	unsigned int level;
	struct snd_pcm_runtime *runtime = NULL;
	snd_pcm_uframes_t avail;

	if (msp->substream_tx) {
		runtime = msp->substream_tx->runtime;
		avail = snd_pcm_playback_avail(runtime);
	} else if (msp->substream_rx) {
		runtime = msp->substream_rx->runtime;
		avail = snd_pcm_capture_avail(runtime);
	}
	if (!runtime)
		return;

	if (runtime->status->state != SNDRV_PCM_STATE_RUNNING)
		return;

	level = (runtime->buffer_size - avail) * 100 / runtime->buffer_size;

	if (msp->cur_thr == MSP_LEV_UNDEF)
		msp->cur_thr = 50;

	cur_rate = msp_i2s_get_rate(msp);
	rate = cur_rate;
	for (h = 0, i = msp->num_thr / 2; i < msp->num_thr; h++, i++) {
		if (h && msp->cur_thr < msp->thr[i] && level >= msp->thr[i]) {
			/* exceed high threshold
			 * note: middle threshold (h = 0) is the target level
			 */
			msp->cur_thr = msp->thr[i];
			msp->target++;
			rate = msp->target + (h * delta);
			dev_dbg(msp->dev, "hi %d>=%d %d\n",
				level, msp->thr[i], rate);
		} else if (msp->cur_thr > msp->thr[i] && level <= msp->thr[i]) {
			/* back from high threshold with hysteresis */
			msp->cur_thr = msp->thr[i];
			rate = msp->target + (h * delta);
			dev_dbg(msp->dev, "dn %d<=%d %d\n",
				level, msp->thr[i], rate);
		}
	}
	for (l = 0, i = (msp->num_thr - 1) / 2; i >= 0; l++, i--) {
		if (l && msp->cur_thr > msp->thr[i] && level <= msp->thr[i]) {
			/* exceed low threshold
			 * note: middle threshold (l = 0) is the target level
			 */
			msp->target--;
			rate = msp->target - (l * delta);
			dev_dbg(msp->dev, "lo %d<=%d %d\n",
				level, msp->thr[i], rate);
			msp->cur_thr = msp->thr[i];
		} else if (msp->cur_thr < msp->thr[i] && level >= msp->thr[i]) {
			/* back from low threshold with hysteresis */
			msp->cur_thr = msp->thr[i];
			rate = msp->target - (l * delta);
			dev_dbg(msp->dev, "up %d>=%d %d\n",
				level, msp->thr[i], rate);
		}
	}

	if (rate != cur_rate) {
		msp->hr_timeout = 1;
		msp_i2s_set_rate(msp, rate);
	} else if (msp->hr_timeout < runtime->buffer_size / 10) {
		msp->hr_timeout++;
	}
}

static enum hrtimer_restart jitter_control_hr_handler(struct hrtimer *hr_timer)
{
	struct msp_drvdata *msp = container_of(hr_timer, struct msp_drvdata,
					       hr_timer);

	queue_work(msp->workqueue, &msp->task);
	hrtimer_forward_now(hr_timer, ms_to_ktime(msp->hr_timeout));
	return HRTIMER_RESTART;
}

static int early_audio_play(struct audio_data *audio_data, void *arg)
{
	struct msp_drvdata *msp = (struct msp_drvdata *)arg;

	if (audio_data->arg == (u32)msp->phys) {
		dev_dbg(msp->dev, "early msp2 playing\n");
		msp->m3_busy = 1;
	}
	return 0;
}

static int early_audio_end_play(struct audio_data *audio_data, void *arg)
{
	struct msp_drvdata *msp = (struct msp_drvdata *)arg;

	if (audio_data->arg == (u32)msp->phys) {
		dev_dbg(msp->dev, "early msp2 end play\n");
		msp->m3_busy = 0;
		complete(&msp->m3_wait);
	}
	return 0;
}

static int msp_dai_drv_probe(struct snd_soc_dai *dai)
{
	dai->id = dai->driver->id;
	return 0;
}

static struct snd_soc_dai_ops msp_dai_ops = {
	.startup = msp_dai_startup,
	.shutdown = msp_dai_shutdown,
	.trigger = msp_dai_trigger,
	.prepare = msp_dai_prepare,
	.hw_params = msp_dai_hw_params,
	.set_tdm_slot = msp_dai_set_tdm_slot,
};

static struct snd_soc_dai_driver msp_dai_drv_init = {
	.probe = msp_dai_drv_probe,
	.playback = {
		.channels_min = MSP_MIN_CHANNELS,
		.channels_max = MSP_MAX_TDM_CHANNELS,
		.rates = SNDRV_PCM_RATE_CONTINUOUS,
		.rate_min = 8000,
		.rate_max = 192000,
		.formats = MSP_I2S_FORMATS,
	},
	.capture = {
		.channels_min = MSP_MIN_CHANNELS,
		.channels_max = MSP_MAX_TDM_CHANNELS,
		.rates = SNDRV_PCM_RATE_CONTINUOUS,
		.rate_min = 8000,
		.rate_max = 192000,
		.formats = MSP_I2S_FORMATS,
	},
	.ops = &msp_dai_ops,
};

static int snd_soc_msp_probe(struct snd_soc_component *component)
{
	struct msp_drvdata *msp = dev_get_drvdata(component->dev);
	int ret;
	int i;

	if (msp->prefix)
		component->name_prefix = msp->prefix;

	for (i = 0; i < ARRAY_SIZE(msp_audio_controls); i++)
		msp_audio_controls[i].device = msp->id;
	ret = snd_soc_add_component_controls(component,
					     msp_audio_controls,
					     ARRAY_SIZE(msp_audio_controls));
	if (ret)
		return ret;

	if (msp->fixed_clock)
		return 0;

	for (i = 0; i < ARRAY_SIZE(msp_jitter_controls); i++)
		msp_jitter_controls[i].device = msp->id;
	ret = snd_soc_add_component_controls(component,
					     msp_jitter_controls,
					     ARRAY_SIZE(msp_jitter_controls));
	if (ret)
		return ret;

	return 0;
}

static const struct snd_soc_component_driver msp_component = {
	.name		= "sta-msp",
	.probe = snd_soc_msp_probe,
};

static const struct of_device_id msp_dai_match[] = {
	{ .compatible = "st,sta-msp-dai", },
	{},
};
MODULE_DEVICE_TABLE(of, msp_dai_match);

static int msp_dai_probe(struct platform_device *pdev)
{
	struct msp_drvdata *msp;
	struct snd_soc_dai_driver *dai_drv;
	static atomic_t i2s_port_count;
	struct resource *res;
	int ret, ndais;
	char *name;
	char *name_irq;
	u32 msp_irq;
	struct device_node *np = pdev->dev.of_node;

	ndais = atomic_inc_return(&i2s_port_count);

	msp = devm_kzalloc(
		&pdev->dev, sizeof(struct msp_drvdata), GFP_KERNEL);
	if (!msp)
		return -ENOMEM;

	dev_set_drvdata(&pdev->dev, msp);
	msp->dev = &pdev->dev;
	msp->id = ndais - 1;

	/* Autorate */
	msp->num_thr = 5;
	msp->thr[0] = 10;
	msp->thr[1] = 30;
	msp->thr[2] = 50;
	msp->thr[3] = 70;
	msp->thr[4] = 90;
	msp->rate_delta = 5; /* 0.5 percent */

	/* DTS settings */
	ret = msp_dai_of_probe(pdev);
	if (ret < 0)
		return ret;

	/* Clocks */
	msp->pclk = devm_clk_get(&pdev->dev, "apb_pclk");
	if (IS_ERR(msp->pclk))
		return PTR_ERR(msp->pclk);

	msp->mspclk = devm_clk_get(&pdev->dev, "mspclk");
	if (IS_ERR(msp->mspclk) && (PTR_ERR(msp->mspclk) != -ENOENT))
		return PTR_ERR(msp->mspclk);

	msp->mspsck = devm_clk_get(&pdev->dev, "mspsck");
	if (PTR_ERR(msp->mspsck) == -ENOENT) {
		msp->mspsck = devm_clk_get(&pdev->dev, "mspsck-fixed");
		msp->fixed_clock = 1;
	}
	if (IS_ERR(msp->mspsck) && PTR_ERR(msp->mspsck) != -ENOENT)
		return PTR_ERR(msp->mspsck);

	if (PTR_ERR(msp->mspclk) == -ENOENT &&
	    PTR_ERR(msp->mspsck) == -ENOENT) {
		dev_err(&pdev->dev, "no mspclk nor mspsck!\n");
		return -ENOENT;
	}

	if (msp->protdesc->srg_clk_sel == I2S_EXTERNAL &&
	    PTR_ERR(msp->mspsck) == -ENOENT) {
		dev_err(&pdev->dev, "srg-clk-sel = \"ext\" but no mspsck!\n");
		return -ENOENT;
	}

	if (msp->protdesc->srg_clk_sel == I2S_EXTERNAL ||
	    (PTR_ERR(msp->mspclk) == -ENOENT)) {
		msp->clk_sel = CLK_SEL_EXTERNAL;
		msp->genclk = msp->mspsck;
	} else {
		msp->clk_sel = CLK_SEL_INTERNAL;
		msp->genclk = msp->mspclk;
	}

	/* Resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	msp->registers = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(msp->registers))
		return PTR_ERR(msp->registers);

	msp->phys = res->start;
	msp->playback_dma_data.addr = msp->phys + MSP_DR;
	msp->capture_dma_data.addr = msp->phys + MSP_DR;
	init_completion(&msp->m3_wait);

	msp->auss_regmap =
		syscon_regmap_lookup_by_phandle(np, "syscon-auss");
	if (IS_ERR(msp->auss_regmap)) {
		dev_err(&pdev->dev, "could not find AUSS syscon regmap\n");
		return PTR_ERR(msp->auss_regmap);
	}

	/* IRQ */
	name_irq = devm_kzalloc(&pdev->dev, MAX_MSP_I2S_NAME, GFP_KERNEL);
	snprintf(
		name_irq, MAX_MSP_I2S_NAME, "msp_i2s_irq.%d", msp->id);

	msp_irq = platform_get_irq(pdev, 0);
	ret = devm_request_threaded_irq(
		&pdev->dev, msp_irq, msp_i2s_irq, msp_i2s_irq_thread,
		0, name_irq, (void *)msp);
	if (ret) {
		dev_err(&pdev->dev,
			"Error: %s: Failed to register IRQ!\n",
			__func__);
		return ret;
	}

	/* Register DAI */
	dai_drv = devm_kzalloc(
		&pdev->dev, sizeof(*dai_drv), GFP_KERNEL);
	if (!dai_drv) {
		ret = -ENOMEM;
		goto err_reg_plat;
	}
	*dai_drv = msp_dai_drv_init;

	dai_drv->id = ndais - 1;
	name = devm_kzalloc(&pdev->dev, MAX_MSP_DAI_NAME, GFP_KERNEL);
	if (!name) {
		dev_err(
			&pdev->dev,
			"Error: %s: Failed to allocate MSP drvdata!\n",
			__func__);
		ret = -ENOMEM;
		goto err_reg_plat;
	}
	snprintf(
		name, MAX_MSP_DAI_NAME, "msp%d", dai_drv->id);
	dai_drv->name = name;

	msg_audio_register(EARLY_AUDIO_PLAY, early_audio_play, msp);
	msg_audio_register(EARLY_AUDIO_END_PLAY, early_audio_end_play, msp);

	ret = snd_soc_register_component(
		&pdev->dev, &msp_component, dai_drv, 1);
	if (ret < 0) {
		dev_err(
			&pdev->dev,
			"Error: %s: Failed to register MSP!\n",
			__func__);
		goto err_reg_plat;
	}

	/* Register PCM platform driver */
	ret = sta_pcm_register(pdev);
	if (ret < 0) {
		dev_err(
			&pdev->dev,
			"Error: %s: Failed to register PCM platform device!\n",
			__func__);
		goto err_reg_plat;
	}

	INIT_WORK(&msp->task, jitter_control);
	msp->workqueue = alloc_workqueue("jitter", WQ_HIGHPRI, 1);
	hrtimer_init(&msp->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	msp->hr_timer.function = jitter_control_hr_handler;
	msp->hr_timeout = 1;

	return 0;

err_reg_plat:
	snd_soc_unregister_component(&pdev->dev);

	return ret;
}

static int msp_dai_remove(struct platform_device *pdev)
{
	struct msp_drvdata *msp = dev_get_drvdata(&pdev->dev);

	sta_pcm_unregister(&pdev->dev);

	snd_soc_unregister_component(&pdev->dev);

	destroy_workqueue(msp->workqueue);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int msp_dai_suspend(struct device *dev)
{
	struct msp_drvdata *msp = dev_get_drvdata(dev);
	u32 *save_area = msp->save_area;

	*save_area++ = readl(msp->registers + MSP_TCF);
	*save_area++ = readl(msp->registers + MSP_RCF);
	*save_area++ = readl(msp->registers + MSP_GCR);
	*save_area++ = readl(msp->registers + MSP_SRG);
	*save_area++ = readl(msp->registers + MSP_MCR);
	*save_area++ = readl(msp->registers + MSP_TCE0);
	*save_area++ = readl(msp->registers + MSP_TCE1);
	*save_area++ = readl(msp->registers + MSP_TCE2);
	*save_area++ = readl(msp->registers + MSP_TCE3);
	*save_area++ = readl(msp->registers + MSP_RCE0);
	*save_area++ = readl(msp->registers + MSP_RCE1);
	*save_area++ = readl(msp->registers + MSP_RCE2);
	*save_area++ = readl(msp->registers + MSP_RCE3);
	*save_area++ = readl(msp->registers + MSP_DMACR);
	*save_area++ = readl(msp->registers + MSP_IODLY);
	*save_area++ = readl(msp->registers + MSP_IMSC);
	pinctrl_pm_select_sleep_state(dev);

	return 0;
}

static int msp_dai_resume(struct device *dev)
{
	struct msp_drvdata *msp = dev_get_drvdata(dev);
	u32 *save_area = msp->save_area;

	writel(*save_area++, msp->registers + MSP_TCF);
	writel(*save_area++, msp->registers + MSP_RCF);
	writel(*save_area++, msp->registers + MSP_GCR);
	writel(*save_area++, msp->registers + MSP_SRG);
	writel(*save_area++, msp->registers + MSP_MCR);
	writel(*save_area++, msp->registers + MSP_TCE0);
	writel(*save_area++, msp->registers + MSP_TCE1);
	writel(*save_area++, msp->registers + MSP_TCE2);
	writel(*save_area++, msp->registers + MSP_TCE3);
	writel(*save_area++, msp->registers + MSP_RCE0);
	writel(*save_area++, msp->registers + MSP_RCE1);
	writel(*save_area++, msp->registers + MSP_RCE2);
	writel(*save_area++, msp->registers + MSP_RCE3);
	writel(*save_area++, msp->registers + MSP_DMACR);
	writel(*save_area++, msp->registers + MSP_IODLY);
	writel(*save_area++, msp->registers + MSP_IMSC);
	pinctrl_pm_select_default_state(dev);

	msp_dai_clk_enable(dev);

	return 0;
}
#endif

static const struct dev_pm_ops msp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(msp_dai_suspend, msp_dai_resume)
};

static struct platform_driver msp_dai_driver = {
	.driver = {
		.name = "sta-msp-dai",
		.of_match_table = msp_dai_match,
		.pm = &msp_pm_ops,
	},
	.probe = msp_dai_probe,
	.remove = msp_dai_remove,
};
module_platform_driver(msp_dai_driver);

MODULE_LICENSE("GPL v2");
