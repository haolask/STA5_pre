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
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/kthread.h>
#include <linux/clk.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/jiffies.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "sta_codec_aif.h"
#include "sta_codec_aif_lut.h"
#include "sta_audio_msg.h"

#define INIT_CHANNEL_MASK(m) u32 m = 0xffffffff
#define SET_CHANNEL(m, i, n) (((m) & ~(0xf << ((i) * 4))) | ((n) << ((i) * 4)))
#define GET_FIRST_CHANNEL(m) ((m) & 0xf)
#define GET_NEXT_CHANNEL(m) ((m) >>= 4, (m) & 0xf)

static char *dspnet;
module_param(dspnet, charp, 0644);
MODULE_PARM_DESC(dspnet, "DSP network");

static bool aif_regmap_volatile(struct device *dev, unsigned int reg)
{
	/* DMABUS must be stopped during cache sync TCM,
	 * so AIF_DMABUS_CR cannot be cached.
	 */
	return (reg == AIF_DMABUS_CR);
}

static struct regmap_config aif_regmap_config = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.name = "aif",
	.max_register = AIF_ADCAUX_CR,
	.volatile_reg = aif_regmap_volatile,
	.cache_type = REGCACHE_RBTREE,
};

static const u32 aif_regs[] = {
	AIF_SAI1_CSR,
	AIF_SAI2_CSR,
	AIF_SAI3_CSR,
	AIF_SAI4_CSR,
	AIF_AIMUX,
	AIF_LPF,
	AIF_SRC0_CSR,
	AIF_SRC1_CSR,
	AIF_SRC2_CSR,
	AIF_SRC3_CSR,
	AIF_SRC4_CSR,
	AIF_SRC5_CSR,
	AIF_AOMUX,
	AIF_ADCAUX_CR,
};

/* Read all the register in a dummy variable,
 * just to loads current values in regcache
 */
int regmap_load_cache(struct st_codec_aif_drvdata *aif_codec_drv)
{
	u32 temp_reg;
	int i, ret;
	bool change;

	for (i = 0; i < ARRAY_SIZE(aif_regs); i++) {
		ret = regmap_read(aif_codec_drv->aif_regmap,
				  aif_regs[i], &temp_reg);
		if (ret < 0)
			return ret;
	}

	/* DMABUS must be stopped when reading TCM */
	regmap_update_bits_check(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
				 AIF_DMABUS_RUN, 0, &change);
	for (i = 0; i < AIF_DMABUS_SIZE; i++) {
		ret = regmap_read(aif_codec_drv->aif_regmap,
				  AIF_DMABUS_TCM + (i * 4), &temp_reg);
		if (temp_reg == 0)
			break;
		if (ret < 0)
			return ret;
	}
	if (change)
		regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
				   AIF_DMABUS_RUN, AIF_DMABUS_RUN);

	return 0;
}

/* DMABUS functions */
static inline int dmabus_start(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	int i, ret;

	ret = regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
				 AIF_DMABUS_RUN, 0);
	if (ret < 0)
		return ret;
	for (i = aif_codec_drv->dmabus_pos;
	     i < aif_codec_drv->dmabus_prev; i++) {
		ret = regmap_write(aif_codec_drv->aif_regmap,
				   AIF_DMABUS_TCM + (i * 4), 0);
		if (ret < 0)
			return ret;
	}

	aif_codec_drv->dmabus_prev = aif_codec_drv->dmabus_pos;
	return regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
				  AIF_DMABUS_RUN, AIF_DMABUS_RUN);
}

static inline int dmabus_stop(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	aif_codec_drv->dmabus_pos = 0;
	return regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
				  AIF_DMABUS_RUN, 0);
}

static int dmabus_set(
	struct st_codec_aif_drvdata *aif_codec_drv,
	u16 src_addr, u16 dst_addr, u32 nch, u32 pos)
{
	int i, ret;
	int src_shift = 16;
	u32 ttype = 0;
	u32 src = src_addr, dst = dst_addr;

	dev_dbg(aif_codec_drv->dev, "[%d]%04x -> %04x, %d\n",
		pos, src_addr, dst_addr, nch);
	src_addr  = ((src & 0xF000) >> 2) | (src & 0x03FF);
	dst_addr  = ((dst & 0xF000) >> 2) | (dst & 0x03FF);
	src_shift = 14;
	ttype    = (0xF) << 28;

	for (i = pos; i < (pos + nch); i++) {
		u32 temp_reg = ttype | src_addr << src_shift | dst_addr;

		if (src != STA_DMA_FIFO_OUT)
			src_addr++;
		if (dst != STA_DMA_FIFO_IN)
			dst_addr++;

		ret = regmap_write(aif_codec_drv->aif_regmap,
				   AIF_DMABUS_TCM + (i * 4),
				   temp_reg);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static inline int dmabus_add(
	struct st_codec_aif_drvdata *aif_codec_drv,
	s16 src, s16 dst, u32 nch)
{
	int ret = dmabus_set(aif_codec_drv, src, dst, nch,
			  aif_codec_drv->dmabus_pos);

	aif_codec_drv->dmabus_pos += nch;

	return ret;
}

/* AMIXER CONTROLS */

static int adcmic_in_get(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_ADCMIC_CR,
			  &temp_reg);
	if (ret < 0)
		return ret;

	switch (temp_reg) {
	case (ADCMIC_STANDBY12):
		ucontrol->value.integer.value[0] = 0;
		break;
	case (ADCMIC1_MICIN | ADCMIC_ENABLE1 | ADCMIC_SDATA1):
		ucontrol->value.integer.value[0] = 1;
		break;
	case (ADCMIC2_AIN1 | ADCMIC_ENABLE2 | ADCMIC_SDATA2):
		ucontrol->value.integer.value[0] = 2;
		break;
	case (ADCMIC1_MICIN | ADCMIC2_MICIN |
	      ADCMIC_ENABLE12 | ADCMIC_SDATA12):
		ucontrol->value.integer.value[0] = 3;
		break;
	case (ADCMIC1_AIN1 | ADCMIC2_AIN1 |
	      ADCMIC_ENABLE12 | ADCMIC_SDATA12):
		ucontrol->value.integer.value[0] = 4;
		break;
	case (ADCMIC1_MICIN | ADCMIC2_AIN1 |
	      ADCMIC_ENABLE12 | ADCMIC_SDATA12):
		ucontrol->value.integer.value[0] = 5;
		break;
	case (ADCMIC1_MICIN | ADCMIC2_AIN1 |
	      ADCMIC_ENABLE12 | ADCMIC_SDATA21):
		ucontrol->value.integer.value[0] = 6;
		break;
	}

	return 0;
}

static int adcmic_in_put(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int val = ucontrol->value.integer.value[0];

	switch (val) {
	case 0: /* standby */
		temp_reg = (ADCMIC_STANDBY12);
		break;
	case 1: /* 1 */
		temp_reg = (ADCMIC1_MICIN | ADCMIC_ENABLE1 | ADCMIC_SDATA1);
		break;
	case 2: /* 2 */
		temp_reg = (ADCMIC2_AIN1 | ADCMIC_ENABLE2 | ADCMIC_SDATA2);
		break;
	case 3: /* 11 */
		temp_reg = (ADCMIC1_MICIN | ADCMIC2_MICIN |
					ADCMIC_ENABLE12 | ADCMIC_SDATA12);
		break;
	case 4: /* 22 */
		temp_reg = (ADCMIC1_AIN1 | ADCMIC2_AIN1 |
					ADCMIC_ENABLE12 | ADCMIC_SDATA12);
		break;
	case 5: /* 12 */
		temp_reg = (ADCMIC1_MICIN | ADCMIC2_AIN1 |
					ADCMIC_ENABLE12 | ADCMIC_SDATA12);
		break;
	case 6: /* 21 */
		temp_reg = (ADCMIC1_MICIN | ADCMIC2_AIN1 |
					ADCMIC_ENABLE12 | ADCMIC_SDATA21);
		break;
	default:
		return 0;
	}

	return regmap_write(aif_codec_drv->auss_regmap, AUSS_ADCMIC_CR,
			    temp_reg);
}

static int adcaux_in_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->aif_regmap, AIF_ADCAUX_CR, &temp_reg);
	if (ret < 0)
		return ret;

	if ((temp_reg & (AIF_ADCAUX_CHSEL_MASK)) == AIF_ADCAUX_CHSEL0)
		ucontrol->value.integer.value[0] = 0;
	if ((temp_reg & (AIF_ADCAUX_CHSEL_MASK)) == AIF_ADCAUX_CHSEL1)
		ucontrol->value.integer.value[0] = 1;

	return 0;
}

/* ADCAUX */

static int adcaux_in_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];
	u32 temp;

	if (val == 0)
		temp = AIF_ADCAUX_CHSEL0;
	else
		temp = AIF_ADCAUX_CHSEL1;

	return regmap_update_bits(aif_codec_drv->aif_regmap, AIF_ADCAUX_CR,
				  AIF_ADCAUX_CHSEL_MASK, temp);
}

static int adcaux_standby_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->aif_regmap, AIF_ADCAUX_CR, &temp_reg);
	if (ret < 0)
		return ret;

	if ((temp_reg & (AIF_ADCAUX_STANDBY_MASK)) == AIF_ADCAUX_STANDBY)
		ucontrol->value.integer.value[0] = 0;
	else
		ucontrol->value.integer.value[0] = 1;
	return 0;
}

static int adcaux_standby_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];
	u32 temp;

	if (val == 0)
		temp = AIF_ADCAUX_STANDBY;
	else
		temp = AIF_ADCAUX_ENABLE;

	return regmap_update_bits(aif_codec_drv->aif_regmap, AIF_ADCAUX_CR,
				  AIF_ADCAUX_STANDBY_MASK, temp);
}

/* MUX CR and SAI */

static int st_codec_aif_set_prot_desc_auss_mux(
	struct st_codec_aif_drvdata *aif_codec_drv,
	struct audioroutectl_protdesc *auss_mux_protdesc)
{
	u32 temp_reg = 0;

	temp_reg &= ~AUSS_MUX_MSP1EN_MASK;
	temp_reg |= (
		auss_mux_protdesc->msp1tx_sai4rx << AUSS_MUX_MSP1EN_SHIFT);

	temp_reg &= ~AUSS_MUX_MSP2EN_MASK;
	temp_reg |= (
		auss_mux_protdesc->msp2tx_sai2rx << AUSS_MUX_MSP2EN_SHIFT);

	temp_reg &= ~AUSS_MUX_MSP0REFCLKINEN_MASK;
	temp_reg |= (
		auss_mux_protdesc->msp0_extrefclk <<
		AUSS_MUX_MSP0REFCLKINEN_SHIFT);

	temp_reg &= ~AUSS_MUX_MSP1REFCLKINEN_MASK;
	temp_reg |= (
		auss_mux_protdesc->msp1_extrefclk &
			AUSS_MUX_MSP1REFCLKINEN_MASK);

	temp_reg &= ~AUSS_MUX_MSP2REFCLKINEN_MASK;
	temp_reg |= (
		auss_mux_protdesc->msp2_extrefclk <<
		AUSS_MUX_MSP2REFCLKINEN_SHIFT);

	temp_reg &= ~AUSS_MUX_SAI1MEN_MASK;
	temp_reg |= (
		auss_mux_protdesc->sai1_men & AUSS_MUX_SAI1MEN_MASK);

	temp_reg &= ~AUSS_MUX_SAI4TOI2S0_MASK;
	temp_reg |= (
		auss_mux_protdesc->sai4_i2s0 << AUSS_MUX_SAI4TOI2S0_SHIFT);

	temp_reg &= ~AUSS_MUX_SAI3DACONLY_MASK;
	temp_reg |= (
		auss_mux_protdesc->sai3_dac_only << AUSS_MUX_SAI3DACONLY_SHIFT);

	dev_dbg(
		aif_codec_drv->dev,
		"%s: AIF: MUX CR 0x%08x\n",
		__func__, temp_reg);

	return regmap_write(aif_codec_drv->auss_regmap, AUSS_MUX_CR, temp_reg);
}

static void st_codec_aif_set_prot_desc_sai(
	struct st_codec_aif_drvdata *aif_codec_drv,
	struct sai_protdesc *sain_protdesc,
	u32 sai_csr)
{
	u32 temp_reg = 0;

	temp_reg &= ~AIF_SAI_IO_MASK;
	temp_reg |= (sain_protdesc->io_mode << AIF_SAI_IO_SHIFT);

	temp_reg &= ~AIF_SAI_MME_MASK;
	temp_reg |= (sain_protdesc->mme_mode << AIF_SAI_MME_SHIFT);

	temp_reg &= ~AIF_SAI_WL_MASK;
	temp_reg |= (sain_protdesc->word_length << AIF_SAI_WL_SHIFT);

	temp_reg &= ~AIF_SAI_DIR_MASK;
	temp_reg |= (sain_protdesc->data_shift_dir << AIF_SAI_DIR_SHIFT);

	temp_reg &= ~AIF_SAI_LRP_MASK;
	temp_reg |= (sain_protdesc->lr_pol << AIF_SAI_LRP_SHIFT);

	temp_reg &= ~AIF_SAI_CKP_MASK;
	temp_reg |= (sain_protdesc->clk_pol << AIF_SAI_CKP_SHIFT);

	temp_reg &= ~AIF_SAI_REL_MASK;
	temp_reg |= (sain_protdesc->rel_timing << AIF_SAI_REL_SHIFT);

	temp_reg &= ~AIF_SAI_ADJ_MASK;
	temp_reg |= (sain_protdesc->word_adj << AIF_SAI_ADJ_SHIFT);

	temp_reg &= ~AIF_SAI_CNT_MASK;
	temp_reg |= (sain_protdesc->word_count << AIF_SAI_CNT_SHIFT);

	temp_reg &= ~AIF_SAI_SYN_MASK;
	temp_reg |= (sain_protdesc->frame_syn << AIF_SAI_SYN_SHIFT);

	temp_reg &= ~AIF_SAI_TM_MASK;
	temp_reg |= (sain_protdesc->tm_mode << AIF_SAI_TM_SHIFT);

	dev_dbg(
		aif_codec_drv->dev,
		"%s: AIF: SAI %08x: 0x%08x\n",
		__func__, sai_csr, temp_reg);

	regmap_write(aif_codec_drv->aif_regmap, sai_csr, temp_reg);
}

static void st_codec_aif_enable_sai(
	struct st_codec_aif_drvdata *aif_codec_drv,
	u32 sai_csr)
{
	regmap_update_bits(aif_codec_drv->aif_regmap, sai_csr,
			   AIF_SAI_ENA_MASK, AIF_SAI_ENABLE);
}

static void st_codec_aif_write_sai2_dio(
	struct st_codec_aif_drvdata *aif_codec_drv,
	u32 dio)
{
	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_SAI2_CSR,
			   AIF_SAI_IO_MASK, (dio << AIF_SAI_IO_SHIFT));
}

static int sai2_dio_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->aif_regmap, AIF_SAI2_CSR, &temp_reg);
	if (ret < 0)
		return ret;

	if ((temp_reg & (AIF_SAI_IO_MASK)) >> AIF_SAI_IO_SHIFT == AIF_SAI2_DI)
		ucontrol->value.integer.value[0] = AIF_SAI2_DI;
	else
		ucontrol->value.integer.value[0] = AIF_SAI2_DO;

	return 0;
}

static int sai2_dio_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];

	if (val == 0)
		st_codec_aif_write_sai2_dio(aif_codec_drv, AIF_SAI2_DI);
	else
		st_codec_aif_write_sai2_dio(aif_codec_drv, AIF_SAI2_DO);

	return 0;
}

/* DAC */

static int st_codec_aif_hwmuteon_aussdac(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	int ret;

	ret = regmap_update_bits(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
				 AUSS_DAC_MUTEALL_MASK, AUSS_DAC_MUTEALL_DAC);
	usleep_range(20000, 20100);

	return ret;
}

static int st_codec_aif_hwmuteoff_aussdac(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	int ret;

	ret = regmap_update_bits(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
				 AUSS_DAC_MUTEALL_MASK, AUSS_DAC_ENABLE_ALL);

	return ret;
}

static int dac_hwmute_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_DAC_CR, &temp_reg);
	if (ret < 0)
		return ret;

	if ((temp_reg & (AUSS_DAC_MUTEALL_MASK)) == AUSS_DAC_ENABLE_ALL)
		ucontrol->value.integer.value[0] = 0;
	else
		ucontrol->value.integer.value[0] = 1;

	return 0;
}

static int dac_hwmute_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];

	if (val == 0)
		st_codec_aif_hwmuteoff_aussdac(aif_codec_drv);
	else
		st_codec_aif_hwmuteon_aussdac(aif_codec_drv);

	return 0;
}

static int st_codec_aif_is_poweron_aussdac(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	u32 dac_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_DAC_CR, &dac_reg);
	if (ret < 0)
		return ret;

	return ((dac_reg & (AUSS_DAC_SBANA_MASK | AUSS_DAC_SB_MASK)) == 0);
}

static int st_codec_aif_poweron_aussdac(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	int ret;
	bool change;

	/* during dac poweron SAI3 clock must be on */
	ret = regmap_update_bits_check(aif_codec_drv->aif_regmap,
				       AIF_SAI3_CSR,
				       AIF_SAI_MME_MASK | AIF_SAI_ENA_MASK,
				       AIF_SAI_MME_MASK | AIF_SAI_ENA_MASK,
				       &change);
	if (ret < 0)
		return ret;

	ret = regmap_update_bits(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
				 AUSS_DAC_SBANA_MASK, 0);
	if (ret < 0)
		return ret;

	msleep(510);
	ret = regmap_update_bits(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
				 AUSS_DAC_SB_MASK, 0);
	if (ret < 0)
		return ret;

	if (change) {
		ret = regmap_update_bits(aif_codec_drv->aif_regmap,
					 AIF_SAI3_CSR,
					 AIF_SAI_MME_MASK | AIF_SAI_ENA_MASK,
					 0);
		if (ret < 0)
			return ret;
	}

	return ret;
}

static int st_codec_aif_poweroff_aussdac(
	struct st_codec_aif_drvdata *aif_codec_drv)
{
	int ret;
	bool change;

	/* during dac poweroff SAI3 clock must be on */
	ret = regmap_update_bits_check(aif_codec_drv->aif_regmap,
				       AIF_SAI3_CSR,
				       AIF_SAI_MME_MASK | AIF_SAI_ENA_MASK,
				       AIF_SAI_MME_MASK | AIF_SAI_ENA_MASK,
				       &change);
	if (ret < 0)
		return ret;

	ret = regmap_update_bits(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
				 AUSS_DAC_SB_MASK, AUSS_DAC_SB_MASK);
	if (ret < 0)
		return ret;

	msleep(510);
	ret = regmap_update_bits(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
				 AUSS_DAC_SB_MASK, AUSS_DAC_SB_MASK);
	if (ret < 0)
		return ret;

	if (change) {
		ret = regmap_update_bits(aif_codec_drv->aif_regmap,
					 AIF_SAI3_CSR,
					 AIF_SAI_MME_MASK | AIF_SAI_ENA_MASK,
					 0);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int dac_power_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_DAC_CR, &temp_reg);
	if (ret < 0)
		return ret;

	if ((temp_reg & (AUSS_DAC_SBALL_MASK)) == AUSS_DAC_ENABLE_ALL)
		ucontrol->value.integer.value[0] = 1;
	else
		ucontrol->value.integer.value[0] = 0;

	return 0;
}

static int dac_power_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];

	if (val == 0)
		st_codec_aif_poweroff_aussdac(aif_codec_drv);
	else
		st_codec_aif_poweron_aussdac(aif_codec_drv);

	return 0;
}

/* AUDIO SOURCE */

static int st_codec_aif_audio_source_configure_no_dma(
	struct st_codec_aif_drvdata *aif_codec_drv,
	int source_index)
{
	struct audio_source_desc *audio_source =
		aif_codec_drv->audio_source[source_index];
	int ret;

	if (audio_source->sai1_protdesc) {
		st_codec_aif_set_prot_desc_sai(
			aif_codec_drv,
			audio_source->sai1_protdesc,
			AIF_SAI1_CSR);
		st_codec_aif_enable_sai(aif_codec_drv, AIF_SAI1_CSR);
	}

	if (audio_source->sai2_protdesc) {
		st_codec_aif_set_prot_desc_sai(
			aif_codec_drv,
			audio_source->sai2_protdesc,
			AIF_SAI2_CSR);
		st_codec_aif_enable_sai(aif_codec_drv, AIF_SAI2_CSR);
	}

	if (audio_source->sai3_protdesc) {
		st_codec_aif_set_prot_desc_sai(
			aif_codec_drv,
			audio_source->sai3_protdesc,
			AIF_SAI3_CSR);
		st_codec_aif_enable_sai(aif_codec_drv, AIF_SAI3_CSR);
	}

	if (audio_source->sai4_protdesc) {
		st_codec_aif_set_prot_desc_sai(
			aif_codec_drv,
			audio_source->sai4_protdesc,
			AIF_SAI4_CSR);
		st_codec_aif_enable_sai(aif_codec_drv, AIF_SAI4_CSR);
	}

	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_AIMUX,
			   audio_source->aimux);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_LPF,
			   audio_source->lpf);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_SRC0_CSR,
			   audio_source->src[0]);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_SRC1_CSR,
			   audio_source->src[1]);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_SRC2_CSR,
			   audio_source->src[2]);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_SRC3_CSR,
			   audio_source->src[3]);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_SRC4_CSR,
			   audio_source->src[4]);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_SRC5_CSR,
			   audio_source->src[5]);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_AOMUX,
			   audio_source->aomux);
	if (ret < 0)
		return ret;

	ret = st_codec_aif_set_prot_desc_auss_mux(
		aif_codec_drv,
		&audio_source->auss_mux_protdesc);
	if (ret < 0)
		return ret;

	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_ADCAUX_CR,
			   audio_source->adcaux);
	if (ret < 0)
		return ret;

	ret = regmap_write(aif_codec_drv->auss_regmap, AUSS_ADCMIC_CR,
			   audio_source->adcmic);
	if (ret < 0)
		return ret;

	return 0;
}

static int st_codec_aif_audio_source_configure(
	struct st_codec_aif_drvdata *aif_codec_drv,
	int source_index)
{
	struct audio_source_desc *audio_source =
		    aif_codec_drv->audio_source[source_index];
	struct dma_transfer_desc *tr;
	int ret;

	dmabus_stop(aif_codec_drv);

	ret = st_codec_aif_audio_source_configure_no_dma(aif_codec_drv,
							 source_index);

	list_for_each_entry(tr, &audio_source->dma_transfer_list, node) {
		if (IS_DMA_DUMMY(tr->from) ||
		    IS_DMA_DUMMY(tr->to))
			dmabus_add(aif_codec_drv, 0, 0, tr->n_channels);
		else
			dmabus_add(aif_codec_drv,
				   tr->from + tr->from_shift,
				   tr->to + tr->to_shift,
				   tr->n_channels);
	}

	dmabus_start(aif_codec_drv);
	return ret;
}

static int source_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	ucontrol->value.integer.value[0] = aif_codec_drv->source_index;

	return 0;
}

static int source_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];

	if (!aif_codec_drv->audio_source[val])
		return -EINVAL;

	mutex_lock(&aif_codec_drv->source_lock);
	st_codec_aif_audio_source_configure(aif_codec_drv, val);
	aif_codec_drv->source_index = val;
	st_codec_aif_channel_realloc(aif_codec_drv);

	mutex_unlock(&aif_codec_drv->source_lock);

	return 0;
}

static int source_put_no_dma(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	int val = ucontrol->value.integer.value[0];

	if (!aif_codec_drv->audio_source[val])
		return -EINVAL;

	mutex_lock(&aif_codec_drv->source_lock);
	st_codec_aif_audio_source_configure_no_dma(aif_codec_drv, val);
	aif_codec_drv->source_index = val;
	st_codec_aif_channel_realloc(aif_codec_drv);

	mutex_unlock(&aif_codec_drv->source_lock);

	return 0;
}

static int aimux_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->aif_regmap, AIF_AIMUX, &temp_reg);
	if (ret < 0)
		return ret;

	ucontrol->value.integer.value[0] = (temp_reg & 0xF);
	ucontrol->value.integer.value[1] = (temp_reg & 0xF0) >> 4;
	ucontrol->value.integer.value[2] = (temp_reg & 0xF00) >> 8;
	ucontrol->value.integer.value[3] = (temp_reg & 0xF000) >> 12;
	ucontrol->value.integer.value[4] = (temp_reg & 0xF0000) >> 16;
	ucontrol->value.integer.value[5] = (temp_reg & 0xF00000) >> 20;
	return 0;
}

static int aimux_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg = ucontrol->value.integer.value[0];

	temp_reg |= (ucontrol->value.integer.value[1]) << 4;
	temp_reg |= (ucontrol->value.integer.value[2]) << 8;
	temp_reg |= (ucontrol->value.integer.value[3]) << 12;
	temp_reg |= (ucontrol->value.integer.value[4]) << 16;
	temp_reg |= (ucontrol->value.integer.value[5]) << 20;

	return regmap_write(aif_codec_drv->aif_regmap, AIF_AIMUX, temp_reg);
}

static int aomux_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg;
	int ret;

	ret = regmap_read(aif_codec_drv->aif_regmap, AIF_AOMUX, &temp_reg);
	if (ret < 0)
		return ret;

	ucontrol->value.integer.value[0] = (temp_reg & 0xF);
	ucontrol->value.integer.value[1] = (temp_reg & 0xF0) >> 4;
	ucontrol->value.integer.value[2] = (temp_reg & 0xF00) >> 8;
	ucontrol->value.integer.value[3] = (temp_reg & 0xF000) >> 12;
	ucontrol->value.integer.value[4] = (temp_reg & 0xF0000) >> 16;
	ucontrol->value.integer.value[5] = (temp_reg & 0xF00000) >> 20;
	return 0;
}

static int aomux_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(component->dev);
	u32 temp_reg = ucontrol->value.integer.value[0];

	temp_reg |= (ucontrol->value.integer.value[1]) << 4;
	temp_reg |= (ucontrol->value.integer.value[2]) << 8;
	temp_reg |= (ucontrol->value.integer.value[3]) << 12;
	temp_reg |= (ucontrol->value.integer.value[4]) << 16;
	temp_reg |= (ucontrol->value.integer.value[5]) << 20;

	return regmap_write(aif_codec_drv->aif_regmap, AIF_AOMUX, temp_reg);
}

static const char * const dio_mode[] = {"In", "Out"};
static const char * const switch_mode[] = {"Off", "On"};
static const char * const adcmic_in_mode[] = {
	"0", "1", "2", "11", "22", "12", "21"};
static const char * const adcaux_in_mode[] = {"0", "1"};
static const char *source_mode[MAX_NUM_AUDIOSOURCE];

static struct soc_enum source_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(source_mode), source_mode);
static const struct soc_enum adcmic_in_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(adcmic_in_mode), adcmic_in_mode);
static const struct soc_enum adcaux_in_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(adcaux_in_mode), adcaux_in_mode);
static const struct soc_enum switch_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(switch_mode), switch_mode);
static const struct soc_enum dio_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(dio_mode), dio_mode);

const struct snd_kcontrol_new st_codec_aif_audio_controls[] = {
	SOC_ENUM_EXT(
		"Source_no_dma",
		source_enum,
		source_get, source_put_no_dma),
	SOC_ENUM_EXT(
		"Source",
		source_enum,
		source_get, source_put),
	SOC_ENUM_EXT(
		"DAC Power",
		switch_enum,
		dac_power_get, dac_power_put),
	SOC_ENUM_EXT(
		"DAC Mute",
		switch_enum,
		dac_hwmute_get, dac_hwmute_put),
	SOC_ENUM_EXT(
		"ADCMIC CHSEL",
		adcmic_in_enum,
		adcmic_in_get, adcmic_in_put),
	SOC_ENUM_EXT(
		"ADCAUX Standby",
		switch_enum,
		adcaux_standby_get, adcaux_standby_put),
	SOC_ENUM_EXT(
		"ADCAUX CHSEL",
		adcaux_in_enum,
		adcaux_in_get, adcaux_in_put),
	SOC_ENUM_EXT(
		"SAI2 IO",
		dio_enum,
		sai2_dio_get, sai2_dio_put),
	SOC_MULTI(
		"AIMUX", aimux_get, aimux_put, 6, 0, 0xf),
	SOC_MULTI(
		"AOMUX", aomux_get, aomux_put, 6, 0, 0xf),
};

/* ALSA PARAMS SETTING */

/* Order of channel allocation for every type of LPF */
static const int lpf_bypass_order[] = {5, 4, 3, 2, 1, 0};
static const int lpf_fir0_order[] = {5, 4, 3, 2, 1, 0};
static const int lpf_upx2_order[] = {5, 4, 3, 2, 0};
static const int lpf_upx4_order[] = {0};
static const int lpf_downx2_order[] = {5, 4, 3, 2, 1};
static const int lpf_downx3_order[] = {2, 1};
static const int lpf_downx4_order[] = {1};
static const int lpf_downx6_order[] = {1}; /* add 2 to enable cascade ch3+2 */

#define LPF_ORDER(a) { .channels = a##_order, \
	.nchannels = ARRAY_SIZE(a##_order)}

static const struct lpf_order lpf_ch_order[] = {
	LPF_ORDER(lpf_bypass),
	LPF_ORDER(lpf_fir0),
	LPF_ORDER(lpf_upx2),
	LPF_ORDER(lpf_upx4),
	LPF_ORDER(lpf_downx2),
	LPF_ORDER(lpf_downx3),
	LPF_ORDER(lpf_downx4),
	LPF_ORDER(lpf_downx6),
};

char *lpf_str(u32 lpf)
{
	switch (lpf) {
	case LPF_BYPASS:
		return "bypass";
	case LPF_FIR0:
		return "fir0";
	case LPF_UPX2:
		return "upx2";
	case LPF_UPX4:
		return "upx4";
	case LPF_DOWNX2:
		return "downx2";
	case LPF_DOWNX3:
		return "downx3";
	case LPF_DOWNX4:
		return "downx4";
	case LPF_DOWNX6:
		return "downx6";
	}
	return "?";
}

/* filter passes if MSP, DIR match */
int chstream_filter(u32 chstream, u32 msp, u32 dir)
{
	return (((u32)((msp) | (dir)) & (chstream)) == (chstream));
}

static int aif_channel_alloc(struct st_codec_aif_drvdata *aif_codec_drv,
			     struct challoc *challoc)
{
	struct audio_source_desc *current_source =
		aif_codec_drv->audio_source[aif_codec_drv->source_index];
	u32 chstream_dir = challoc->dir ? CHSTREAM_PLAYBACK : CHSTREAM_CAPTURE;
	u32 chstream_msp = challoc->msp == 1 ? CHSTREAM_MSP1 : CHSTREAM_MSP2;
	const struct lpf_order *order = &lpf_ch_order[challoc->lpf];
	int i, ch, ret = -EINVAL;

	for (i = 0; i < order->nchannels; i++) {
		ch = order->channels[i];
		if (aif_codec_drv->challoc[ch].msp) {
			ret = -EBUSY;
			continue;
		}

		if (!chstream_filter(current_source->chstream[ch],
				     chstream_msp, chstream_dir))
			/* not supported by current source */
			continue;

		if (challoc->lpf == LPF_DOWNX6 && ch == 2) {
			/* ch2 cascade to ch3, verify that ch3 is available */
			if (aif_codec_drv->challoc[3].msp) {
				ret = -EBUSY;
				continue;
			}

			if (!chstream_filter(current_source->chstream[3],
					     chstream_msp, chstream_dir))
				continue;
			aif_codec_drv->challoc[3] = *challoc;
		}
		aif_codec_drv->challoc[ch] = *challoc;

		return ch;
	}

	return ret;
}

static int sta_codec_aif_channel_alloc(
			struct st_codec_aif_drvdata *aif_codec_drv,
			struct challoc *challoc)
{
	int src_ch;
	u32 lpf_sav;

	src_ch = aif_channel_alloc(aif_codec_drv, challoc);
	if (src_ch < 0) {
		/* try with FIR */
		lpf_sav = challoc->lpf;
		challoc->lpf = LPF_FIR0;
		src_ch = aif_channel_alloc(aif_codec_drv, challoc);
		if (src_ch < 0)
			return src_ch;

		dev_warn(aif_codec_drv->dev,
			 "LPF %s not available, fall back to FIR0 ch%d\n",
			 lpf_str(lpf_sav), src_ch);
	}

	return src_ch;
}

static inline void sta_codec_aif_channel_free(
			struct st_codec_aif_drvdata *aif_codec_drv,
			int is_playback, int msp)
{
	int chmsp, chdir;
	int ch;
	struct challoc *challoc = aif_codec_drv->challoc;

	for (ch = 0; ch < AIF_NUM_SRC; ch++) {
		chmsp = challoc[ch].msp;
		chdir = challoc[ch].dir;
		if (msp == chmsp && is_playback == chdir) {
			challoc[ch].msp = 0;
			regmap_update_bits(aif_codec_drv->aif_regmap, AIF_AIMUX,
			   (0xF << (ch * AIF_MUX_CH_SHIFT)), 0x00);
			regmap_update_bits(aif_codec_drv->aif_regmap, AIF_AOMUX,
			   (0xF << (ch * AIF_MUX_CH_SHIFT)), 0x00);
		}
	}
}

static u32 st_codec_aif_lpf(int lpf_mode, int channel, bool *cascade)
{
	u32 lpf = AIF_LPF_ERROR;

	/* cascade */
	if (channel == 2 && lpf_mode == LPF_DOWNX6) {
		lpf_mode = LPF_DOWNX3;
		*cascade = true;
	} else if (channel == 3 && lpf_mode == LPF_DOWNX6) {
		lpf_mode = LPF_DOWNX2;
		*cascade = true;
	}

	switch (lpf_mode) {
	case LPF_BYPASS:
		lpf = AIF_LPFCSR_BYPASS;
		break;
	case LPF_FIR0:
		if (channel == 0)
			lpf = AIF_LPFCSR_FS0_FIR0;
		else if (channel == 1)
			lpf = AIF_LPFCSR_FS1_FIR0;
		else if (channel == 2)
			lpf = AIF_LPFCSR_FS2_FIR0;
		else if (channel == 3)
			lpf = AIF_LPFCSR_FS3_FIR0;
		else if (channel == 4)
			lpf = AIF_LPFCSR_FS4_FIR0;
		else if (channel == 5)
			lpf = AIF_LPFCSR_FS5_FIR0;
		break;
	case LPF_UPX2:
		if (channel == 0)
			lpf = AIF_LPFCSR_FS0_UPX2;
		else if (channel == 2)
			lpf = AIF_LPFCSR_FS2_UPX2;
		else if (channel == 3)
			lpf = AIF_LPFCSR_FS3_UPX2;
		else if (channel == 4)
			lpf = AIF_LPFCSR_FS4_UPX2;
		else if (channel == 5)
			lpf = AIF_LPFCSR_FS5_UPX2;
		break;
	case LPF_UPX4:
		if (channel == 0)
			lpf = AIF_LPFCSR_FS0_UPX4;
		break;
	case LPF_DOWNX2:
		if (channel == 1)
			lpf = AIF_LPFCSR_FS1_DOWNX2;
		else if (channel == 2)
			lpf = AIF_LPFCSR_FS2_DOWNX2;
		else if (channel == 3)
			lpf = AIF_LPFCSR_FS3_DOWNX2;
		else if (channel == 4)
			lpf = AIF_LPFCSR_FS4_DOWNX2;
		else if (channel == 5)
			lpf = AIF_LPFCSR_FS5_DOWNX2;
		break;
	case LPF_DOWNX3:
		if (channel == 1)
			lpf = AIF_LPFCSR_FS1_DOWNX3;
		else if (channel == 2)
			lpf = AIF_LPFCSR_FS2_DOWNX3;
		break;
	case LPF_DOWNX4:
		if (channel == 1)
			lpf = AIF_LPFCSR_FS1_DOWNX4;
		break;
	case LPF_DOWNX6:
		if (channel == 1)
			lpf = AIF_LPFCSR_FS1_DOWNX6;
		break;
	}

	return lpf;
}

static u32 st_codec_aif_capture_rate(unsigned int rate)
{
	switch (rate) {
	case 48000:
	case 44100:
	case 32000:
		return LPF_FIR0;
	case 16000:
		return LPF_DOWNX3;
	case 8000:
		return LPF_DOWNX6;
	}
	return LPF_BYPASS;
}

static u32 st_codec_aif_playback_rate(unsigned int rate)
{
	switch (rate) {
	case 48000:
	case 44100:
	case 32000:
		return LPF_FIR0;
	case 22050:
	case 16000:
		return LPF_UPX2;
	case 12000:
	case 11025:
	case 8000:
		return LPF_UPX4;
	}
	return LPF_BYPASS;
}

static inline void sta_codec_aif_lpf_update(struct challoc *challoc, int src_ch)
{
	u32 lpf_val;
	int lpf_mode;
	struct challoc (*p)[] = (struct challoc (*)[])challoc;
	struct st_codec_aif_drvdata *aif_codec_drv =
			 container_of(p, struct st_codec_aif_drvdata, challoc);
	bool cascade = false;
	u32 temp;

	lpf_mode = challoc[src_ch].lpf;
	lpf_val = st_codec_aif_lpf(lpf_mode, src_ch, &cascade);
	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_LPF,
			   lpf_channel_mask[src_ch], lpf_val);
	regmap_read(aif_codec_drv->aif_regmap, AIF_LPF, &temp);
	if (cascade)
		regmap_update_bits(aif_codec_drv->aif_regmap, AIF_AIMUX,
				   AIF_AIMUX_LPF_DOWNX3X2_CASCADE_MASK,
				   AIF_AIMUX_LPF_DOWNX3X2_CASCADE_MASK);
}

static inline void sta_codec_aif_src_update(struct challoc *challoc, int src_ch)
{
	struct challoc (*p)[] = (struct challoc (*)[])challoc;
	struct st_codec_aif_drvdata *aif_codec_drv =
			 container_of(p, struct st_codec_aif_drvdata, challoc);
	struct audio_source_desc *current_source =
		aif_codec_drv->audio_source[aif_codec_drv->source_index];

	/* don't override static settings */
	if (current_source->src[src_ch])
		return;

	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_SRC_CSR(src_ch),
			   AIF_SRC_DRLL_THRES, AIF_SRC_DRLL_THRES);
}

static inline void sta_codec_aif_aimux_update(struct challoc *challoc,
					      int src_ch)
{
	u32 temp;
	int msp, msp_ch, is_playback;
	struct challoc (*p)[] = (struct challoc (*)[])challoc;
	struct st_codec_aif_drvdata *aif_codec_drv =
			 container_of(p, struct st_codec_aif_drvdata, challoc);
	struct audio_source_desc *current_source =
		 aif_codec_drv->audio_source[aif_codec_drv->source_index];

	msp = challoc[src_ch].msp;
	msp_ch = challoc[src_ch].mspch;
	is_playback = challoc[src_ch].dir;

	/* don't override static settings */
	if (current_source->aimux & (0xF << (src_ch * AIF_MUX_CH_SHIFT)))
		return;

	if (is_playback)
		temp = (SAIRX(msp, msp_ch) << (src_ch * AIF_MUX_CH_SHIFT));
	else
		temp = (AIF_AIMUX_LPFDI << (src_ch * AIF_MUX_CH_SHIFT));

	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_AIMUX,
			   (0xF << (src_ch * AIF_MUX_CH_SHIFT)), temp);
}

static inline void sta_codec_aif_aomux_update(struct challoc *challoc,
					      int src_ch)
{
	u32 temp;
	int msp, msp_ch, is_playback;
	struct challoc (*p)[] = (struct challoc (*)[])challoc;
	struct st_codec_aif_drvdata *aif_codec_drv =
			 container_of(p, struct st_codec_aif_drvdata, challoc);
	struct audio_source_desc *current_source =
		 aif_codec_drv->audio_source[aif_codec_drv->source_index];

	msp = challoc[src_ch].msp;
	msp_ch = challoc[src_ch].mspch;
	is_playback = challoc[src_ch].dir;

	/* don't override static settings */
	if (current_source->aomux & (0xF << (src_ch * AIF_MUX_CH_SHIFT)))
		return;

	/* cascade: output is on ch2, skip ch3 */
	if (src_ch == 3 && challoc[src_ch].lpf == LPF_DOWNX6)
		return;

	if (is_playback)
		temp = (AIF_AOMUX_SRCDO << (src_ch * AIF_MUX_CH_SHIFT));
	else
		temp = (SAITX(msp, msp_ch) << (src_ch * AIF_MUX_CH_SHIFT));

	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_AOMUX,
			   (0xF << (src_ch * AIF_MUX_CH_SHIFT)), temp);
}

static inline int sta_codec_aif_dmabus_update(
			struct st_codec_aif_drvdata *aif_codec_drv,
			int msp, int is_playback, u32 src_ch_all,
			struct audio_source_desc *current_source)
{
	struct challoc *challoc = aif_codec_drv->challoc;
	struct dma_transfer_desc *tr;
	int src_ch;
	int tcm_pos = 0;
	u32 from, to, n_ch, n_tr;

	for (src_ch = GET_FIRST_CHANNEL(src_ch_all);
	     src_ch < AIF_NUM_SRC;
	     src_ch = GET_NEXT_CHANNEL(src_ch_all)) {
		/* cascade: input is on ch3, skip ch2 */
		if (src_ch == 2 && challoc[src_ch].lpf == LPF_DOWNX6)
			continue;
		/* SRC channel is stereo, look for 2 DMABUS slots
		 * reserved to msp
		 */
		n_ch = 2;
		list_for_each_entry(
			    tr, &current_source->dma_transfer_list, node) {
			from = 0;
			to = 0;
			if (is_playback && tr->from == STA_DMA_MSP(msp)) {
				from = STA_DMA_SRC_DO(src_ch);
				to = tr->to;
			} else if (!is_playback && tr->to == STA_DMA_MSP(msp)) {
				from = tr->from;
				to = STA_DMA_LPF_DI(src_ch);
			}
			n_tr = tr->n_channels - tr->n_busy;
			if (from && to && n_tr > 0) {
				regmap_update_bits(aif_codec_drv->aif_regmap,
						   AIF_DMABUS_CR,
						   AIF_DMABUS_RUN, 0);

				n_tr = min(n_tr, n_ch);
				dmabus_set(
					aif_codec_drv,
					from + tr->from_shift + tr->n_busy,
					to + tr->to_shift + tr->n_busy,
					n_tr, tcm_pos);
				tr->n_busy += n_tr;
				n_ch -= n_tr;
				if (!n_ch)
					break;
			}
			tcm_pos += tr->n_channels;
		}
	}
	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
			   AIF_DMABUS_RUN, AIF_DMABUS_RUN);
	return 0;
}

static inline void sta_codec_aif_update_all(
			struct st_codec_aif_drvdata *aif_codec_drv,
			int msp, int is_playback)
{
	struct audio_source_desc *current_source =
		 aif_codec_drv->audio_source[aif_codec_drv->source_index];
	struct challoc *challoc = aif_codec_drv->challoc;
	int src_ch;
	int chmsp, chdir, msp_ch;

	INIT_CHANNEL_MASK(src_ch_all);

	for (src_ch = 0; src_ch < AIF_NUM_SRC; src_ch++) {
		chmsp = challoc[src_ch].msp;
		msp_ch = challoc[src_ch].mspch;
		chdir = challoc[src_ch].dir;

		if (msp == chmsp && is_playback == chdir) {
			dev_dbg(aif_codec_drv->dev,
				"update ch%d %s\n",
				src_ch,	lpf_str(challoc[src_ch].lpf));

			sta_codec_aif_lpf_update(challoc, src_ch);
			sta_codec_aif_src_update(challoc, src_ch);
			sta_codec_aif_aimux_update(challoc, src_ch);
			sta_codec_aif_aomux_update(challoc, src_ch);

			src_ch_all = SET_CHANNEL(src_ch_all, msp_ch, src_ch);
		}
	}

	sta_codec_aif_dmabus_update(aif_codec_drv, msp, is_playback,
				    src_ch_all, current_source);
}

static inline void sta_codec_aif_dmabus_clear_all(
		struct st_codec_aif_drvdata *aif_codec_drv,
		int msp, int is_playback)
{
	int source_index = aif_codec_drv->source_index;
	struct audio_source_desc *current_source =
		aif_codec_drv->audio_source[source_index];
	struct dma_transfer_desc *tr;
	int pos = 0;

	list_for_each_entry(tr,	&current_source->dma_transfer_list, node) {
		tr->n_busy = 0;
		if ((is_playback && (tr->from & STA_DMA_MSP(msp))) ||
		    (!is_playback && (tr->to & STA_DMA_MSP(msp)))) {
			regmap_update_bits(aif_codec_drv->aif_regmap,
					   AIF_DMABUS_CR, AIF_DMABUS_RUN, 0);
			dmabus_set(aif_codec_drv, 0, 0, tr->n_channels, pos);
		}
		pos += tr->n_channels;
	}
	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
			   AIF_DMABUS_RUN, AIF_DMABUS_RUN);
}

void st_codec_aif_shutdown(
	struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	int is_playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	int msp = dai->id;
	struct st_codec_aif_drvdata *aif_codec_drv = dev_get_drvdata(dai->dev);

	if (!aif_codec_drv)
		return;

	if (aif_codec_drv->source_index < 0)
		return;

	sta_codec_aif_channel_free(aif_codec_drv, is_playback, msp);

	sta_codec_aif_dmabus_clear_all(aif_codec_drv, msp, is_playback);
}

static inline u32 sai_is_tdm(
		struct st_codec_aif_drvdata *aif_codec_drv,
		int msp)
{
	struct audio_source_desc *current_source =
		aif_codec_drv->audio_source[aif_codec_drv->source_index];
	struct sai_protdesc *sai_protdesc;

	if (msp == 1)
		sai_protdesc = current_source->sai4_protdesc;
	else
		sai_protdesc = current_source->sai2_protdesc;

	return (sai_protdesc->word_count != AIF_SAI_CNT_2W);
}

int st_codec_aif_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	int rate = params_rate(params);
	int channels = params_channels(params);
	u32 lpf_mode;
	int i, msp_ch, src_ch;
	int is_playback = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK);
	struct challoc challoc;
	int msp = dai->id;
	struct st_codec_aif_drvdata *aif_codec_drv = dev_get_drvdata(dai->dev);
	struct sai_protdesc *sai_protdesc;
	struct audio_source_desc *audio_source;
	u32 sai_csr;

	if (!aif_codec_drv)
		return -ENODEV;

	mutex_lock(&aif_codec_drv->source_lock);
	if (aif_codec_drv->source_index < 0) {
		dev_err(dai->dev, "select a Source first\n");
		mutex_unlock(&aif_codec_drv->source_lock);
		return -EINVAL;
	}

	/* set the SAI in every audio sources where it's connected to MSP */
	for (i = 0; i < MAX_NUM_AUDIOSOURCE; i++) {
		audio_source = aif_codec_drv->audio_source[i];
		if (!audio_source)
			continue;
		if (msp == 1) {
			if (!audio_source->auss_mux_protdesc.msp1tx_sai4rx)
				continue;
			sai_protdesc = audio_source->sai4_protdesc;
			sai_csr = AIF_SAI4_CSR;
		} else {
			if (!audio_source->auss_mux_protdesc.msp2tx_sai2rx)
				continue;
			sai_protdesc = audio_source->sai2_protdesc;
			sai_csr = AIF_SAI2_CSR;
		}
		switch (channels) {
		case 1:
		case 2:
			sai_protdesc->word_count = AIF_SAI_CNT_2W;
			sai_protdesc->frame_syn = AIF_SAI_SIN_HALFFRAME;
			break;
		case 3:
		case 4:
			sai_protdesc->word_count = AIF_SAI_CNT_4W;
			sai_protdesc->frame_syn = AIF_SAI_SIN_FIRSTBIT;
			break;
		case 5:
		case 6:
			sai_protdesc->word_count = AIF_SAI_CNT_6W;
			sai_protdesc->frame_syn = AIF_SAI_SIN_FIRSTBIT;
			break;
		case 7:
		case 8:
			sai_protdesc->word_count = AIF_SAI_CNT_8W;
			sai_protdesc->frame_syn = AIF_SAI_SIN_FIRSTBIT;
			break;
		default:
			return -EINVAL;
		}
		switch (params_format(params)) {
		case SNDRV_PCM_FORMAT_S16_LE:
			sai_protdesc->word_length = AIF_SAI_WL_16_16;
			break;
		case SNDRV_PCM_FORMAT_S24_LE:
			sai_protdesc->word_length = AIF_SAI_WL_24_24;
			break;
		case SNDRV_PCM_FORMAT_S32_LE:
			sai_protdesc->word_length = AIF_SAI_WL_32_24;
			break;
		default:
			dev_err(dai->dev, "format %d not supported\n",
				params_format(params));
			mutex_unlock(&aif_codec_drv->source_lock);
			return -EINVAL;
		}
		if (i == aif_codec_drv->source_index) {
			st_codec_aif_set_prot_desc_sai(
					    aif_codec_drv,
					    sai_protdesc,
					    sai_csr);
			st_codec_aif_enable_sai(aif_codec_drv, sai_csr);
		}
	}

	/* Allocate LPF */
	if (is_playback)
		lpf_mode = st_codec_aif_playback_rate(rate);
	else
		lpf_mode = st_codec_aif_capture_rate(rate);

/*
 *	Workaround for HW bug: UPx4 not working in TDM
 *	In case of TDM use UPx2 in place of UPx4
 */
	if (lpf_mode == LPF_UPX4 && sai_is_tdm(aif_codec_drv, msp))
		lpf_mode = LPF_UPX2;

	for (msp_ch = 0; msp_ch < (channels + 1) / 2; msp_ch++) {
		challoc.msp = msp;
		challoc.mspch = msp_ch;
		challoc.dir = is_playback;
		challoc.lpf = lpf_mode;
		src_ch = sta_codec_aif_channel_alloc(aif_codec_drv, &challoc);
		if (src_ch < 0) {
			mutex_unlock(&aif_codec_drv->source_lock);
			return src_ch;
		}
		dev_dbg(aif_codec_drv->dev,
			"\nalloc ch%d msp%x.%x, %s, %s\n",
			src_ch,	challoc.msp, challoc.mspch,
			challoc.dir ? "play" : "rec", lpf_str(challoc.lpf));
	}
	sta_codec_aif_update_all(aif_codec_drv, msp, is_playback);

	mutex_unlock(&aif_codec_drv->source_lock);
	return 0;
}

int st_codec_aif_channel_realloc(struct st_codec_aif_drvdata *aif_codec_drv)
{
	struct challoc challoc[AIF_NUM_SRC];
	int src_ch, msp, dir;

	for (src_ch = 0; src_ch < AIF_NUM_SRC; src_ch++) {
		challoc[src_ch] = aif_codec_drv->challoc[src_ch];
		aif_codec_drv->challoc[src_ch].msp = 0;
	}

	for (src_ch = 0; src_ch < AIF_NUM_SRC; src_ch++) {
		if (!challoc[src_ch].msp)
			continue;
		dev_dbg(aif_codec_drv->dev, "realloc(%x,%x,%x,%x)\n",
			challoc[src_ch].msp, challoc[src_ch].mspch,
			challoc[src_ch].dir, challoc[src_ch].lpf);
		sta_codec_aif_channel_alloc(aif_codec_drv, &challoc[src_ch]);
	}

	for (msp = 1; msp <= 2; msp++) {
		for (dir = 0; dir <= 1; dir++) {
			sta_codec_aif_dmabus_clear_all(aif_codec_drv, msp, dir);
			sta_codec_aif_update_all(aif_codec_drv, msp, dir);
		}
	}

	return 0;
}

static struct snd_soc_dai_ops st_codec_msp_dai_ops = {
	.hw_params	= st_codec_aif_hw_params,
	.shutdown	= st_codec_aif_shutdown,
};

#define ST_CODEC_MSP_MIN_CHANNELS 1
#define ST_CODEC_MSP_MAX_CHANNELS 8
#define ST_CODEC_MSP_RATES	(SNDRV_PCM_RATE_8000_48000 \
			       | SNDRV_PCM_RATE_CONTINUOUS)
#define ST_CODEC_MSP_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE \
			       | SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver sta_aif_dai_drv[] = {
	{
		.id = 3,
		.name = "st-codec-aif",
	},
	{
		.id = 1,
		.name = "st-codec-msp1",
		.playback = {
			.stream_name = "MSP1 Playback",
			.channels_min = ST_CODEC_MSP_MIN_CHANNELS,
			.channels_max = ST_CODEC_MSP_MAX_CHANNELS,
			.rates = ST_CODEC_MSP_RATES,
			.formats = ST_CODEC_MSP_FORMATS,
		},
		.capture = {
			.stream_name = "MSP1 Capture",
			.channels_min = ST_CODEC_MSP_MIN_CHANNELS,
			.channels_max = ST_CODEC_MSP_MAX_CHANNELS,
			.rates = ST_CODEC_MSP_RATES,
			.formats = ST_CODEC_MSP_FORMATS,
		},
		.ops = &st_codec_msp_dai_ops,
		.symmetric_rates = 1,
	},
	{
		.id = 2,
		.name = "st-codec-msp2",
		.playback = {
			.stream_name = "MSP2 Playback",
			.channels_min = ST_CODEC_MSP_MIN_CHANNELS,
			.channels_max = ST_CODEC_MSP_MAX_CHANNELS,
			.rates = ST_CODEC_MSP_RATES,
			.formats = ST_CODEC_MSP_FORMATS,
		},
	/* capture supported only in loopback, rates = 0 skips hw_params */
		.capture = {
			.stream_name = "MSP2 Capture",
			.channels_min = ST_CODEC_MSP_MIN_CHANNELS,
			.channels_max = ST_CODEC_MSP_MAX_CHANNELS,
			.rates = 0,
			.formats = ST_CODEC_MSP_FORMATS,
		},
		.ops = &st_codec_msp_dai_ops,
		.symmetric_rates = 1,
	},
};

static const struct snd_soc_component_driver sta_aif_codec_drv[] = {
	{
	.controls = st_codec_aif_audio_controls,
	.num_controls = ARRAY_SIZE(st_codec_aif_audio_controls),
	},
	{},
	{},
};

static const struct of_device_id st_codec_aif_dt_ids[] = {
	{ .compatible = "st,audiointerface", },
	{ }
};
MODULE_DEVICE_TABLE(of, st_codec_aif_dt_ids);

static int aif_dts_lut_match(
	struct device *dev,
	const struct dts_lut_val *lut_check,
	int lut_length,
	const char *str_input,
	u32 *lut_out_value)
{
	int i;

	for (i = 0; i < lut_length; i++) {
		if (strcmp(lut_check[i].name, str_input) == 0) {
			*lut_out_value = lut_check[i].value;
			return 0;
		}
	}

	dev_err(
		dev, "%s:%s wrong LUT entry",
		__func__, str_input);

	return -EINVAL;
}

static int st_codec_aif_of_adcaux_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *adcaux)
{
	int ret;
	const char *s_adcaux;

	ret = of_property_read_string(
		np, "adcaux", (const char **)&s_adcaux);

	if (!ret)
		return aif_dts_lut_match(&pdev->dev, adcaux_dts_lut,
					 ARRAY_SIZE(adcaux_dts_lut),
					 s_adcaux, adcaux);

	return 0;
}

static int st_codec_aif_of_adcmic_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *adcmic)
{
	int ret;
	const char *s_adcmic;

	ret = of_property_read_string(
		np, "adcmic", (const char **)&s_adcmic);

	if (!ret)
		return aif_dts_lut_match(&pdev->dev, adcmic_dts_lut,
					 ARRAY_SIZE(adcmic_dts_lut),
					 s_adcmic, adcmic);

	return 0;
}

static int st_codec_aif_of_dma_transfer_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	struct list_head *dma_transfer_list)
{
	const char *s_from, *s_from_shift, *s_to, *s_to_shift, *s_nchannels;
	int i, ret;
	struct dma_transfer_desc *dma_transfer;
	int num_dma_transfer =
		of_property_count_strings(np, "dma-transfers");

	if (num_dma_transfer <= 0)
		return 0;

	if ((num_dma_transfer % 5) != 0) {
		dev_err(
			&pdev->dev,
			"ERROR: invalid number of dma transfers %d!\n",
			num_dma_transfer);
		return 0;
	}

	num_dma_transfer /= 5;

	for (i = 0; i < num_dma_transfer; i++) {
		dma_transfer = devm_kcalloc(
			&pdev->dev, 1,
			sizeof(struct dma_transfer_desc), GFP_KERNEL);
		if (!dma_transfer) {
			dev_err(&pdev->dev,
				"%s: ERROR: Failed to init dma_transfer drvdata-struct!\n",
				__func__);
			return -ENOMEM;
		}
		list_add_tail(&dma_transfer->node, dma_transfer_list);

		of_property_read_string_index(
			np, "dma-transfers",
			5 * i, (const char **)&s_from);
		of_property_read_string_index(
			np, "dma-transfers",
			(5 * i) + 1, (const char **)&s_from_shift);
		of_property_read_string_index(
			np, "dma-transfers",
			(5 * i) + 2, (const char **)&s_to);
		of_property_read_string_index(
			np, "dma-transfers",
			(5 * i) + 3, (const char **)&s_to_shift);
		of_property_read_string_index(
			np, "dma-transfers",
			(5 * i) + 4, (const char **)&s_nchannels);

		if (aif_dts_lut_match(&pdev->dev,
				      dma_transfer_dts_lut,
				      ARRAY_SIZE(dma_transfer_dts_lut),
				      s_from, &dma_transfer->from))
			return -EINVAL;

		ret = kstrtou32(s_from_shift, 10, &dma_transfer->from_shift);
		if (ret < 0)
			return ret;
		if (aif_dts_lut_match(&pdev->dev, dma_transfer_dts_lut,
				      ARRAY_SIZE(dma_transfer_dts_lut),
				      s_to, &dma_transfer->to))
			return -EINVAL;

		ret = kstrtou32(s_to_shift, 10, &dma_transfer->to_shift);
		if (ret < 0)
			return ret;
		ret = kstrtou32(s_nchannels, 10, &dma_transfer->n_channels);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int st_codec_aif_of_lpf_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *lpf, u32 *aimux)
{
	const char *lpf_str;
	struct property *prop;
	u32 lpf_val;
	int channel = 0, lpf_id;
	bool cascade = false;

	of_property_for_each_string(np, "lpf", prop, lpf_str) {
		if (aif_dts_lut_match(&pdev->dev, lpf_dts_lut,
				      ARRAY_SIZE(lpf_dts_lut),
				      lpf_str, &lpf_id))
			return -EINVAL;
		lpf_val = st_codec_aif_lpf(lpf_id, channel, &cascade);
		if (lpf_val == AIF_LPF_ERROR) {
			dev_err(
				&pdev->dev,
				"%s: LPF %s not supported in channel %d!\n",
				__func__, lpf_str, channel);

			return -EINVAL;
		}
		*lpf |= lpf_val;
		if (cascade)
			*aimux |= AIF_AIMUX_LPF_DOWNX3X2_CASCADE_MASK;
		channel++;
	}

	return 0;
}

static int st_codec_aif_of_src_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *src)
{
	const char *src_str;
	struct property *prop;
	u32 src_val = 0;
	int i = 0;

	of_property_for_each_string(np, "src", prop, src_str) {
		if (aif_dts_lut_match(&pdev->dev, src_dts_lut,
				      ARRAY_SIZE(src_dts_lut),
				      src_str, &src_val))
			return -EINVAL;

		src[i] = src_val;
		i++;
	}

	return 0;
}

static int st_codec_aif_of_aomux_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *aomux)
{
	const char *aomux_str;
	struct property *prop;
	u32 aomux_val = 0;
	int i = 0;

	of_property_for_each_string(np, "aomux", prop, aomux_str) {
		if (aif_dts_lut_match(&pdev->dev, aomux_dts_lut,
				      ARRAY_SIZE(aomux_dts_lut),
				      aomux_str, &aomux_val))
			return -EINVAL;

		*aomux = (*aomux) | (aomux_val << i);
		i = i + AIF_MUX_CH_SHIFT;
	}

	return 0;
}

static int st_codec_aif_of_aimux_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *aimux)
{
	const char *aimux_str;
	struct property *prop;
	u32 aimux_val = 0;
	int i = 0;

	of_property_for_each_string(np, "aimux", prop, aimux_str) {
		if (aif_dts_lut_match(&pdev->dev, aimux_dts_lut,
				      ARRAY_SIZE(aimux_dts_lut),
				      aimux_str, &aimux_val))
			return -EINVAL;

		(*aimux) = (*aimux) | (aimux_val << i);
		i = i + AIF_MUX_CH_SHIFT;
	}

	return 0;
}

static int st_codec_aif_of_chstream_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	u32 *chstream)
{
	const char *chstream_str;
	struct property *prop;
	int i = 0;

	of_property_for_each_string(np, "channel-stream", prop, chstream_str) {
		if (aif_dts_lut_match(&pdev->dev, chstream_dts_lut,
				      ARRAY_SIZE(chstream_dts_lut),
				      chstream_str, &chstream[i]))
			return -EINVAL;

		i++;
	}

	return 0;
}

static const struct audioroutectl_protdesc auss_mux_protdesc_default = {
	.msp1tx_sai4rx = AUSS_MUX_DISABLE,
	.msp2tx_sai2rx = AUSS_MUX_DISABLE,
	.msp0_extrefclk = AUSS_MUX_DISABLE,
	.msp1_extrefclk = AUSS_MUX_MSP1REFCLKINEN_PLL3,
	.msp2_extrefclk = AUSS_MUX_DISABLE,
	.sai1_men = AUSS_MUX_DISABLE,
	.sai4_i2s0 = AUSS_MUX_DISABLE,
	.sai3_dac_only = AUSS_MUX_ENABLE,
};

static int st_codec_aif_of_auss_mux_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	struct audioroutectl_protdesc *auss_mux_protdesc)
{
	int ret = -EINVAL;
	const char *s_aussmux;
	struct device_node *aussmux_np;

	aussmux_np = of_get_child_by_name(np, "auss-mux");

	if (!aussmux_np)
		return 0;

	ret = of_property_read_string(
		aussmux_np, "msp1tx-sai4rx", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_dts_lut,
				      ARRAY_SIZE(aussmux_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->msp1tx_sai4rx))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "msp2tx-sai2rx", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_dts_lut,
				      ARRAY_SIZE(aussmux_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->msp2tx_sai2rx))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "msp0-extrefclk", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_dts_lut,
				      ARRAY_SIZE(aussmux_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->msp0_extrefclk))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "msp1-extrefclk", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_msp1ref_dts_lut,
				      ARRAY_SIZE(aussmux_msp1ref_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->msp1_extrefclk))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "msp2-extrefclk", (const char **)&s_aussmux);

	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_dts_lut,
				      ARRAY_SIZE(aussmux_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->msp2_extrefclk))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "sai1-men", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_sai1_men_dts_lut,
				      ARRAY_SIZE(aussmux_sai1_men_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->sai1_men))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "sai4-i2s0", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_dts_lut,
				      ARRAY_SIZE(aussmux_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->sai4_i2s0))
			return -EINVAL;
	}

	ret = of_property_read_string(
		aussmux_np, "sai3-dac-only", (const char **)&s_aussmux);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, aussmux_dts_lut,
				      ARRAY_SIZE(aussmux_dts_lut),
				      s_aussmux,
				      &auss_mux_protdesc->sai3_dac_only))
			return -EINVAL;
	}

	return 0;
}

static const struct sai_protdesc sai_protdesc_default = {
	.mme_mode = AIF_SAI_SLAVE,
	.word_length = AIF_SAI_WL_16_16,
	.data_shift_dir = AIF_SAI_DIR_MSB,
	.lr_pol = AIF_SAI_LRP_LEFT,
	.clk_pol = AIF_SAI_CKP_NEGATIVE,
	.rel_timing = AIF_SAI_REL_FIRSTBIT,
	.word_adj = AIF_SAI_ADJ_LEFT,
	.word_count = AIF_SAI_CNT_2W,
	.frame_syn = AIF_SAI_SIN_HALFFRAME,
	.tm_mode = AIF_SAI_NORMAL,
	.io_mode = AIF_SAI2_DI, /* only for SAI2 */
};

static int st_codec_aif_of_sai_drv_probe(
	struct platform_device *pdev,
	struct device_node *npp, char *sainame,
	struct sai_protdesc **sai_protdesc)
{
	const char *sai_str;
	int ret = -EINVAL;
	struct sai_protdesc *sain_protdesc;
	struct device_node *np;

	if (*sai_protdesc) {
		sain_protdesc = *sai_protdesc;
	} else {
		sain_protdesc = devm_kzalloc(
			&pdev->dev,
			sizeof(struct sai_protdesc),
			GFP_KERNEL);
		if (!sain_protdesc) {
			dev_err(&pdev->dev,
				"%s: ERROR: Failed to alloc sai protdesc!\n",
				__func__);
			return -ENOMEM;
		}
		*sai_protdesc = sain_protdesc;

		memcpy(
			sain_protdesc,
			&sai_protdesc_default,
			sizeof(struct sai_protdesc));
	}

	np = of_get_child_by_name(npp, sainame);
	if (!np)
		return 0;

	if (strcmp(np->name, "sai2") == 0) {
		ret = of_property_read_string(
			np, "io-mode", &sai_str);
		if (!ret) {
			if (aif_dts_lut_match(&pdev->dev, io_mode_dts_lut,
					      ARRAY_SIZE(io_mode_dts_lut),
					      sai_str, &sain_protdesc->io_mode))
				return -EINVAL;
		}
	}

	ret = of_property_read_string(
		np, "mme-mode", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, mme_mode_dts_lut,
				      ARRAY_SIZE(mme_mode_dts_lut),
				      sai_str, &sain_protdesc->mme_mode))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "word-length", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, word_length_dts_lut,
				      ARRAY_SIZE(word_length_dts_lut),
				      sai_str, &sain_protdesc->word_length))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "data-shift-dir", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, data_shift_dir_dts_lut,
				      ARRAY_SIZE(data_shift_dir_dts_lut),
				      sai_str, &sain_protdesc->data_shift_dir))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "lr-pol", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, lr_pol_dts_lut,
				      ARRAY_SIZE(lr_pol_dts_lut),
				      sai_str, &sain_protdesc->lr_pol))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "clk-pol", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, clk_pol_dts_lut,
				      ARRAY_SIZE(clk_pol_dts_lut),
				      sai_str, &sain_protdesc->clk_pol))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "rel-timing", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, rel_timing_dts_lut,
				      ARRAY_SIZE(rel_timing_dts_lut),
				      sai_str, &sain_protdesc->rel_timing))
			return -EINVAL;
	}

	ret = of_property_read_string(
			np, "word-adj", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, word_adj_dts_lut,
				      ARRAY_SIZE(word_adj_dts_lut),
				      sai_str, &sain_protdesc->word_adj))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "word-count", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, word_count_dts_lut,
				      ARRAY_SIZE(word_count_dts_lut),
				      sai_str, &sain_protdesc->word_count))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "frame-syn", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, frame_syn_dts_lut,
				      ARRAY_SIZE(frame_syn_dts_lut),
				      sai_str, &sain_protdesc->frame_syn))
			return -EINVAL;
	}

	ret = of_property_read_string(
		np, "tm-mode", &sai_str);
	if (!ret) {
		if (aif_dts_lut_match(&pdev->dev, tm_mode_dts_lut,
				      ARRAY_SIZE(tm_mode_dts_lut),
				      sai_str, &sain_protdesc->tm_mode))
			return -EINVAL;
	}

	return 0;
}

static int st_codec_aif_of_audio_source_drv_probe(
	struct platform_device *pdev,
	struct device_node *np,
	struct audio_source_desc *audio_source)
{
	int ret;

	ret = st_codec_aif_of_sai_drv_probe(
		pdev, np, "sai1",
		&audio_source->sai1_protdesc);
	if (ret)
		return ret;

	ret = st_codec_aif_of_sai_drv_probe(
		pdev, np, "sai2",
		&audio_source->sai2_protdesc);
	if (ret)
		return ret;

	ret = st_codec_aif_of_sai_drv_probe(
		pdev, np, "sai3",
		&audio_source->sai3_protdesc);
	if (ret)
		return ret;

	ret = st_codec_aif_of_sai_drv_probe(
		pdev, np, "sai4",
		&audio_source->sai4_protdesc);
	if (ret)
		return ret;

	ret = st_codec_aif_of_auss_mux_drv_probe(
		pdev, np,
		&audio_source->auss_mux_protdesc);
	if (ret)
		return ret;

	ret = st_codec_aif_of_chstream_drv_probe(
		pdev, np,
		audio_source->chstream);
	if (ret)
		return ret;

	ret = st_codec_aif_of_aimux_drv_probe(
		pdev, np,
		&audio_source->aimux);
	if (ret)
		return ret;

	ret = st_codec_aif_of_aomux_drv_probe(
		pdev, np,
		&audio_source->aomux);
	if (ret)
		return ret;

	ret = st_codec_aif_of_lpf_drv_probe(
		pdev, np,
		&audio_source->lpf,
		&audio_source->aimux);
	if (ret)
		return ret;

	ret = st_codec_aif_of_src_drv_probe(
		pdev, np,
		audio_source->src);
	if (ret)
		return ret;

	ret = st_codec_aif_of_dma_transfer_drv_probe(
		pdev, np,
		&audio_source->dma_transfer_list);
	if (ret)
		return ret;

	ret = st_codec_aif_of_adcmic_drv_probe(
		pdev, np,
		&audio_source->adcmic);
	if (ret)
		return ret;

	ret = st_codec_aif_of_adcaux_drv_probe(
		pdev, np,
		&audio_source->adcaux);
	if (ret)
		return ret;

	return 0;
}

static char *dump_sai(char *buf, struct sai_protdesc *sai)
{
	buf += sprintf(buf, " io_mode %x\n", sai->io_mode);
	buf += sprintf(buf, " mme_mode %x\n", sai->mme_mode);
	buf += sprintf(buf, " word_length %x\n", sai->word_length);
	buf += sprintf(buf, " data_shift_dir %x\n", sai->data_shift_dir);
	buf += sprintf(buf, " lr_pol %x\n", sai->lr_pol);
	buf += sprintf(buf, " clk_pol %x\n", sai->clk_pol);
	buf += sprintf(buf, " rel_timing %x\n", sai->rel_timing);
	buf += sprintf(buf, " word_adj %x\n", sai->word_adj);
	buf += sprintf(buf, " word_count %x\n", sai->word_count);
	buf += sprintf(buf, " frame_syn %x\n", sai->frame_syn);
	buf += sprintf(buf, " tm_mode %x\n", sai->tm_mode);

	return buf;
}

static char *dump_aussmux(
	char *buf, struct audioroutectl_protdesc *auss_mux)
{
	buf += sprintf(buf, "AUSS MUX\n");
	buf += sprintf(buf, " msp1tx_sai4rx %x\n", auss_mux->msp1tx_sai4rx);
	buf += sprintf(buf, " msp2tx_sai2rx %x\n", auss_mux->msp2tx_sai2rx);
	buf += sprintf(buf, " sai1_men %x\n", auss_mux->sai1_men);
	buf += sprintf(buf, " msp0_extrefclk %x\n", auss_mux->msp0_extrefclk);
	buf += sprintf(buf, " msp1_extrefclk %x\n", auss_mux->msp1_extrefclk);
	buf += sprintf(buf, " msp2_extrefclk %x\n", auss_mux->msp2_extrefclk);
	buf += sprintf(buf, " sai4_i2s0 %x\n", auss_mux->sai4_i2s0);
	buf += sprintf(buf, " sai3_dac_only %x\n", auss_mux->sai3_dac_only);

	return buf;
}

static char *dump_audio_source(
	char *buf,
	struct audio_source_desc *audio_source)
{
	int i;
	struct dma_transfer_desc *dma_transfer;

	buf += sprintf(buf, "AUDIO SOURCE %s\n", audio_source->name);
	buf += sprintf(buf, "SAI1\n");
	if (audio_source->sai1_protdesc)
		buf = dump_sai(buf, audio_source->sai1_protdesc);
	buf += sprintf(buf, "SAI2\n");
	if (audio_source->sai2_protdesc)
		buf = dump_sai(buf, audio_source->sai2_protdesc);
	buf += sprintf(buf, "SAI3\n");
	if (audio_source->sai3_protdesc)
		buf = dump_sai(buf, audio_source->sai3_protdesc);
	buf += sprintf(buf, "SAI4\n");
	if (audio_source->sai4_protdesc)
		buf = dump_sai(buf, audio_source->sai4_protdesc);
	buf += sprintf(buf, "aimux %x\n", audio_source->aimux);
	buf += sprintf(buf, "aomux %x\n", audio_source->aomux);
	buf += sprintf(buf, "lpf %x\n", audio_source->lpf);
	for (i = 0; i < AIF_NUM_SRC; i++) {
		buf += sprintf(buf, "chstream[%d] %x\n", i,
			       audio_source->chstream[i]);
		buf += sprintf(buf, "src[%d] %x\n", i, audio_source->src[i]);
	}
	buf += sprintf(buf, "adcmic %x\n", audio_source->adcmic);
	buf += sprintf(buf, "adcaux %x\n", audio_source->adcaux);
	list_for_each_entry(dma_transfer,
			    &audio_source->dma_transfer_list, node) {
		buf += sprintf(buf, " from=0x%x+%x n=%d, to=0x%x+%x\n",
			dma_transfer->from,
			dma_transfer->from_shift,
			dma_transfer->n_channels,
			dma_transfer->to,
			dma_transfer->to_shift);
	}
	buf = dump_aussmux(buf, &audio_source->auss_mux_protdesc);

	return buf;
}

static ssize_t aif_dump(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct st_codec_aif_drvdata *aif_codec_drv = dev_get_drvdata(dev);
	char *start = buf;
	int i;
	u32 reg, dmabus_cr;

	regmap_read(aif_codec_drv->auss_regmap, AUSS_ADCMIC_CR, &reg);
	buf += sprintf(buf, "ADCMIC_CR %08x\n", reg);
	regmap_read(aif_codec_drv->auss_regmap, AUSS_DAC_CR, &reg);
	buf += sprintf(buf, "DAC_CR %08x\n", reg);
	regmap_read(aif_codec_drv->auss_regmap, AUSS_SSY_CR, &reg);
	buf += sprintf(buf, "SSY_CR %08x\n", reg);
	regmap_read(aif_codec_drv->auss_regmap, AUSS_MUX_CR, &reg);
	buf += sprintf(buf, "MUX_CR %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_DMABUS_CR, &dmabus_cr);
	buf += sprintf(buf, "DMABUS_CR %08x\n", dmabus_cr);
	buf += sprintf(buf, "DMABUS_TCM:\n");
	regmap_update_bits(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
			   AIF_DMABUS_RUN, 0);
	for (i = 0; i < AIF_DMABUS_SIZE; i++) {
		regmap_read(aif_codec_drv->aif_regmap,
			    AIF_DMABUS_TCM + (i * 4), &reg);
		if (reg == 0)
			break;
		buf += sprintf(buf, "%08x: %08x\n",	(i * 4), reg);
	}
	regmap_write(aif_codec_drv->aif_regmap, AIF_DMABUS_CR, dmabus_cr);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SAI1_CSR, &reg);
	buf += sprintf(buf, "SAI1 %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SAI2_CSR, &reg);
	buf += sprintf(buf, "SAI2 %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SAI3_CSR, &reg);
	buf += sprintf(buf, "SAI3 %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SAI4_CSR, &reg);
	buf += sprintf(buf, "SAI4 %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_AIMUX, &reg);
	buf += sprintf(buf, "AIMUX %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_LPF, &reg);
	buf += sprintf(buf, "LPF  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SRC0_CSR, &reg);
	buf += sprintf(buf, "SRC0  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SRC1_CSR, &reg);
	buf += sprintf(buf, "SRC1  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SRC2_CSR, &reg);
	buf += sprintf(buf, "SRC2  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SRC3_CSR, &reg);
	buf += sprintf(buf, "SRC3  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SRC4_CSR, &reg);
	buf += sprintf(buf, "SRC4  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_SRC5_CSR, &reg);
	buf += sprintf(buf, "SRC5  %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_AOMUX, &reg);
	buf += sprintf(buf, "AOMUX %08x\n", reg);
	regmap_read(aif_codec_drv->aif_regmap, AIF_ADCAUX_CR, &reg);
	buf += sprintf(buf, "ADCAUX %08x\n", reg);

	return buf - start;
}

static ssize_t source_dump(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct st_codec_aif_drvdata *aif_codec_drv = dev_get_drvdata(dev);
	struct audio_source_desc *audio_source;
	char *start = buf;

	if (aif_codec_drv->source_index >= 0) {
		audio_source =
		 aif_codec_drv->audio_source[aif_codec_drv->source_index];
		buf = dump_audio_source(buf, audio_source);
	}
	return buf - start;
}

static DEVICE_ATTR(aif_dump, 0444, aif_dump, NULL);
static DEVICE_ATTR(source_dump, 0444, source_dump, NULL);

static int st_codec_source_match(char *s,
				 struct st_codec_aif_drvdata *aif_codec_drv)
{
	int i;

	for (i = 0; i < MAX_NUM_AUDIOSOURCE; i++) {
		if (!aif_codec_drv->audio_source[i])
			break;
		if (!strcmp(s, aif_codec_drv->audio_source[i]->name))
			return i;
	}
	return -1;
}

static int aif_poweron_thread(void *p)
{
	struct st_codec_aif_drvdata *aif_codec_drv =
		(struct st_codec_aif_drvdata *)p;

	if (!st_codec_aif_is_poweron_aussdac(aif_codec_drv)) {
		mutex_lock(&aif_codec_drv->source_lock);
		st_codec_aif_hwmuteon_aussdac(aif_codec_drv);
		st_codec_aif_poweron_aussdac(aif_codec_drv);
		if (aif_codec_drv->source_index >= 0)
			st_codec_aif_hwmuteoff_aussdac(aif_codec_drv);
		mutex_unlock(&aif_codec_drv->source_lock);
	}

	return 0;
}

static int early_audio_source(struct audio_data *audio_data, void *arg)
{
	struct st_codec_aif_drvdata *aif_codec_drv =
				(struct st_codec_aif_drvdata *)arg;
	int i;

	del_timer_sync(&aif_codec_drv->source_timer);
	dev_dbg(aif_codec_drv->dev, "early source [%s]\n", audio_data->desc);
	i = st_codec_source_match(audio_data->desc, aif_codec_drv);
	if (i >= 0) {
		mutex_lock(&aif_codec_drv->source_lock);
		aif_codec_drv->source_index = i;
		mutex_unlock(&aif_codec_drv->source_lock);
	}

	return 0;
}

static void default_audio_source(unsigned long arg)
{
	struct st_codec_aif_drvdata *aif_codec_drv =
				(struct st_codec_aif_drvdata *)arg;
	int source_index = aif_codec_drv->source_index;

	dev_info(aif_codec_drv->dev, "default source [%s]\n",
		 aif_codec_drv->audio_source[source_index]->name);
	st_codec_aif_audio_source_configure(
	    aif_codec_drv, aif_codec_drv->source_index);
}

static int st_codec_aif_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct audio_source_desc *audio_source;
	struct device_node *np, *dsp_np, *net_nc, *audio_np;
	struct resource *res = NULL;
	struct task_struct *kthread;
	char *net_id = "";
	char *source_id = "";
	int ret = -EINVAL;
	int i = 0, ch;
	struct st_codec_aif_drvdata *aif_codec_drv;

	np = pdev->dev.of_node;

	aif_codec_drv = devm_kzalloc(
		&pdev->dev,
		sizeof(struct st_codec_aif_drvdata),
		GFP_KERNEL);

	if (!aif_codec_drv) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to init aif_codec drvdata-struct!\n",
			__func__);
		return -ENOMEM;
	}
	/* Resources */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "aif-reg");
	aif_codec_drv->aif_registers = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(aif_codec_drv->aif_registers))
		return PTR_ERR(aif_codec_drv->aif_registers);

	aif_codec_drv->aif_regmap = devm_regmap_init_mmio(&pdev->dev,
						aif_codec_drv->aif_registers,
						&aif_regmap_config);
	if (IS_ERR(aif_codec_drv->aif_regmap))
		return PTR_ERR(aif_codec_drv->aif_regmap);

	aif_codec_drv->auss_regmap =
		syscon_regmap_lookup_by_phandle(np, "syscon-auss");
	if (IS_ERR(aif_codec_drv->auss_regmap)) {
		dev_err(&pdev->dev, "could not find AUSS syscon regmap\n");
		return PTR_ERR(aif_codec_drv->auss_regmap);
	}

	ret = regmap_load_cache(aif_codec_drv);
	if (ret < 0)
		return ret;

	/* Clocks */
	aif_codec_drv->mclk = devm_clk_get(&pdev->dev, "mclk");
	if (IS_ERR(aif_codec_drv->mclk))
		return PTR_ERR(aif_codec_drv->mclk);

	aif_codec_drv->fs512 = devm_clk_get(&pdev->dev, "fs512");
	if (IS_ERR(aif_codec_drv->fs512))
		return PTR_ERR(aif_codec_drv->fs512);

	ret = clk_prepare_enable(aif_codec_drv->mclk);
	if (ret) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to prepare/enable mclk!\n",
			__func__);
		goto err_clk1;
	}

	ret = clk_prepare_enable(aif_codec_drv->fs512);
	if (ret) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to prepare/enable fs512!\n",
			__func__);
		goto err_clk2;
	}

	aif_codec_drv->dev = &pdev->dev;
	aif_codec_drv->dmabus_prev = AIF_DMABUS_SIZE;
	aif_codec_drv->source_index = -1;

	mutex_init(&aif_codec_drv->source_lock);

	/* Find network id from codec_dsp node */
	dsp_np = of_get_child_by_name(of_get_parent(np), "codec_dsp");
	if (dsp_np) {
		ret = of_property_read_string(
		    dsp_np, "network-id", (const char **)&net_id);
		if (!ret)
			dev_info(&pdev->dev, "network-id %s\n", net_id);
		if (dspnet && strlen(dspnet)) {
			dev_info(&pdev->dev, "force dspnet = %s\n", dspnet);
			net_id = dspnet;
		}
	}

	ret = of_property_read_string(
		np, "source-id", (const char **)&source_id);
	if (!ret)
		dev_info(&pdev->dev, "default source %s\n", source_id);

	for_each_child_of_node(np, audio_np) {

		audio_source = devm_kcalloc(
			&pdev->dev,
			1,
			sizeof(struct audio_source_desc),
			GFP_KERNEL);
		if (!audio_source) {
			ret = -ENOMEM;
			goto err_of_probe;
		}

		aif_codec_drv->audio_source[i] = audio_source;
		audio_source->aif_codec_drv = aif_codec_drv;
		audio_source->name = audio_np->name;
		source_mode[i] = audio_np->name;
		audio_source->adcmic = ADCMIC_STANDBY12;
		audio_source->adcaux = AIF_ADCAUX_STANDBY;
		memcpy(
			&audio_source->auss_mux_protdesc,
			&auss_mux_protdesc_default,
			sizeof(auss_mux_protdesc_default));

		for (ch = 0; ch < AIF_NUM_SRC; ch++)
			audio_source->chstream[ch] = CHSTREAM_NONE;

		INIT_LIST_HEAD(&audio_source->dma_transfer_list);

		ret = st_codec_aif_of_audio_source_drv_probe(
			pdev, audio_np, audio_source);
		if (ret)
			goto err_of_probe;

		net_nc = of_get_child_by_name(audio_np, net_id);
		if (net_nc) {
		/* network specific section may override some source fields */
			ret = st_codec_aif_of_audio_source_drv_probe(
				pdev, net_nc, audio_source);
			if (ret)
				goto err_of_probe;
		}

		if (source_id && !strcmp(audio_source->name, source_id))
			aif_codec_drv->source_index = i;

		i++;
	}
	source_mode[i] = "";
	source_enum.items = i + 1;

	platform_set_drvdata(pdev, aif_codec_drv);

	for (i = 0; i < ARRAY_SIZE(sta_aif_codec_drv); i++) {
		ret = snd_soc_register_component(dev,
						 &sta_aif_codec_drv[i],
						 &sta_aif_dai_drv[i],
						 1);
		if (ret) {
			dev_err(&pdev->dev, "Failed to register codec!\n");
			goto err_codec;
		}
	}

	/* Early audio */
	msg_audio_register(EARLY_AUDIO_SOURCE, early_audio_source,
			   (void *)aif_codec_drv);
	st_codec_early_audio_send(EARLY_AUDIO_SOURCE);
	if (aif_codec_drv->source_index >= 0) {
	/* if early source didn't come on time, apply the default one */
		setup_timer(&aif_codec_drv->source_timer, default_audio_source,
			    (unsigned long)aif_codec_drv);
		mod_timer(&aif_codec_drv->source_timer,
			  jiffies + msecs_to_jiffies(500));
	}

	kthread = kthread_run(aif_poweron_thread, aif_codec_drv, "aif_poweron");
	if (IS_ERR(kthread)) {
		dev_err(&pdev->dev, "AIF: Thread Creation failed");
		ret = -ENOMEM;
		goto err_dac;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_aif_dump);
	if (ret) {
		dev_err(dev, "error creating sysfs files\n");
		goto err_file1;
	}

	ret = device_create_file(&pdev->dev, &dev_attr_source_dump);
	if (ret) {
		dev_err(dev, "error creating sysfs files\n");
		goto err_file2;
	}
	return 0;
err_file2:
	device_remove_file(&pdev->dev, &dev_attr_aif_dump);
err_file1:
	st_codec_aif_poweroff_aussdac(aif_codec_drv);
err_dac:
	snd_soc_unregister_codec(&pdev->dev);
err_codec:
err_of_probe:
	clk_disable_unprepare(aif_codec_drv->fs512);
err_clk2:
	clk_disable_unprepare(aif_codec_drv->mclk);
err_clk1:
	aif_codec_drv = NULL;

	return ret;
}

static int st_codec_aif_remove(struct platform_device *pdev)
{
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(&pdev->dev);

	st_codec_aif_poweroff_aussdac(aif_codec_drv);

	snd_soc_unregister_codec(&pdev->dev);

	clk_disable_unprepare(aif_codec_drv->mclk);
	clk_disable_unprepare(aif_codec_drv->fs512);

	device_remove_file(&pdev->dev, &dev_attr_aif_dump);
	device_remove_file(&pdev->dev, &dev_attr_source_dump);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int aif_suspend(struct device *dev)
{
	int ret;
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(dev);

	dev_info(dev, "%s\n", __func__);
	ret = regmap_read(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
			  &aif_codec_drv->aif_dmabus_cr);
	if (ret)
		return ret;

	regcache_cache_only(aif_codec_drv->aif_regmap, true);

	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_ADCMIC_CR,
			  &aif_codec_drv->auss_adcmic_cr);
	if (ret)
		return ret;
	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
			  &aif_codec_drv->auss_dac_cr);
	if (ret)
		return ret;
	ret = regmap_read(aif_codec_drv->auss_regmap, AUSS_MUX_CR,
			  &aif_codec_drv->auss_mux_cr);
	if (ret)
		return ret;

	clk_disable_unprepare(aif_codec_drv->mclk);
	clk_disable_unprepare(aif_codec_drv->fs512);

	return 0;
}

static int aif_resume(struct device *dev)
{
	struct st_codec_aif_drvdata *aif_codec_drv =
		dev_get_drvdata(dev);
	int ret;

	dev_info(dev, "%s\n", __func__);
	ret = clk_prepare_enable(aif_codec_drv->mclk);
	if (ret)
		return ret;

	ret = clk_prepare_enable(aif_codec_drv->fs512);
	if (ret)
		return ret;

	regcache_cache_only(aif_codec_drv->aif_regmap, false);

	/* Be sure DMABUS is stopped when during cache sync TCM */
	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_DMABUS_CR, 0);
	if (ret < 0)
		return ret;

	regcache_mark_dirty(aif_codec_drv->aif_regmap);
	ret = regcache_sync(aif_codec_drv->aif_regmap);
	if (ret < 0)
		return ret;

	ret = regmap_write(aif_codec_drv->aif_regmap, AIF_DMABUS_CR,
			   aif_codec_drv->aif_dmabus_cr);
	if (ret < 0)
		return ret;
	ret = regmap_write(aif_codec_drv->auss_regmap, AUSS_ADCMIC_CR,
			   aif_codec_drv->auss_adcmic_cr);
	if (ret)
		return ret;
	ret = regmap_write(aif_codec_drv->auss_regmap, AUSS_DAC_CR,
			   aif_codec_drv->auss_dac_cr);
	if (ret)
		return ret;
	ret = regmap_write(aif_codec_drv->auss_regmap, AUSS_MUX_CR,
			   aif_codec_drv->auss_mux_cr);
	if (ret)
		return ret;

	return 0;
}
#endif

static const struct dev_pm_ops aif_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(aif_suspend, aif_resume)
};

static struct platform_driver st_codec_aif_driver = {
	.driver = {
		.name = "sta-aif",
		.of_match_table = of_match_ptr(st_codec_aif_dt_ids),
		.pm =		&aif_pm_ops,
	},
	.probe =    st_codec_aif_probe,
	.remove =   st_codec_aif_remove,
};

module_platform_driver(st_codec_aif_driver);

MODULE_DESCRIPTION("STA ASoC Audio Interface driver");
MODULE_AUTHOR("Gabriele Simone gabriele.simone@st.com");
MODULE_LICENSE("GPL");
