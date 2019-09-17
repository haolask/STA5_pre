/*
 * Copyright (C) ST Microelectronics 2015
 *
 * Author:	Gabriele Simone <gabriele.simone@st.com>,
 *		Giancarlo Asnaghi <giancarlo.asnaghi@st.com>
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
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>
#include <linux/workqueue.h>
#include <linux/kernel.h>
#include <linux/timer.h>

#include "st_codec_dsp.h"

#define ST_CODEC_DSP_MAX_FILENAME 64

struct codec_dsp_drvdata {
	struct dsp_data dsp_data[NUM_DSP];
	struct device *dev;
	int num_dsp;
	int dspmask;
	int early_init;
	struct regmap *auss_regmap;
	/* Clocks */
	struct clk *emrclk, *mclk;
};

DECLARE_COMPLETION(dsp_init_completion);

/* parameter: force
 *
 * force loading of the firmware even if detects the firmware already
 * present into the DSP memories
 */
static char *dspnet;
module_param(dspnet, charp, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dspnet, "DSP network");
static bool force;
module_param(force, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(bool, "Force load DSP fw");

static LIST_HEAD(module_list);
static LIST_HEAD(control_list);
static LIST_HEAD(connection_list);
static struct codec_dsp_drvdata *dsp_codec_drv;
static struct tuning tuning;

static const char * const clip_mode[] = {"Off", "Attack-Release", "Full-Attack"};
static const struct soc_enum clip_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(clip_mode), clip_mode);
static const char * const comp_mode[] = {"Mean", "Max"};
static const struct soc_enum comp_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(comp_mode), comp_mode);
static const char * const polarity_mode[] = {"Pos", "Neg"};
static const struct soc_enum polarity_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(polarity_mode), polarity_mode);
static const char * const eq_filters[] = {
	"bypass", "lp1bt", "lp2bt", "hp1bt", "hp2bt", "ba1sh", "ba2sh", "tr1sh",
	"tr2sh", "bb2", "ap2cb", "lp2cb", "hp2cb", "bp2cb", "ntccb", "pkcb",
	"lshcb", "hshcb"
};
static const struct soc_enum eq_filter_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(eq_filters), eq_filters);
static const char * const eq_tones[] = {
	"sh1", "sh2", "bb2"};
static const struct soc_enum tones_filter_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(eq_tones), eq_tones);
static const char * const eq_loudness[] = {
	"sh1", "sh2", "bb2"};
static const struct soc_enum loud_filter_enum =
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(eq_loudness), eq_loudness);

static void wait_dsp_init(void)
{
	if (!completion_done(&dsp_init_completion))
		wait_for_completion_interruptible(&dsp_init_completion);
}

static STAModule get_dsp_component(struct device *dev,
				   const char *dsp_comp)
{
	struct sta_module *module;

	if (strcmp(dsp_comp, "xin0") == 0)
		return (STAModule) STA_XIN0;
	else if (strcmp(dsp_comp, "xout0") == 0)
		return (STAModule) STA_XOUT0;
	else if (strcmp(dsp_comp, "xin1") == 0)
		return (STAModule) STA_XIN1;
	else if (strcmp(dsp_comp, "xout1") == 0)
		return (STAModule) STA_XOUT1;
	else if (strcmp(dsp_comp, "xin2") == 0)
		return (STAModule) STA_XIN2;
	else if (strcmp(dsp_comp, "xout2") == 0)
		return (STAModule) STA_XOUT2;

	list_for_each_entry(module, &module_list, list) {
		if (strcmp(dsp_comp, module->name) == 0)
			return module->mod;
	}
	dev_err(dev, "dsp component %s not found!", dsp_comp);

	return -EINVAL;
}

static struct sta_module *get_sta_module_by_name(const char *name)
{
	struct sta_module *module;

	list_for_each_entry(module, &module_list, list) {
		if (!strcmp(name, module->name))
			return module;
	}
	return NULL;
}

static struct sta_connection *get_sta_conn_to(STAModule to_mod, u32 to_ch)
{
	struct sta_connection *conn;

	list_for_each_entry(conn, &connection_list, list) {
		if (conn->to_mod == to_mod && conn->to_ch == to_ch)
			return conn;
	}
	return NULL;
}

struct sta_module *get_sta_module(struct snd_kcontrol *kcontrol)
{
	struct sta_module *module;

	wait_dsp_init();

	list_for_each_entry(module, &module_list, list) {
		if (kcontrol->id.numid >= module->first_numid
			&& kcontrol->id.numid <= module->last_numid)
			return module;
	}
	return NULL;
}

int get_sta_channel(struct snd_kcontrol *kcontrol)
{
	struct sta_control *control;

	list_for_each_entry(control, &control_list, list) {
		if (kcontrol->id.numid == control->numid)
			return control->ch;
	}

	return -1;
}

int sta_add_channel_controls(struct snd_soc_component *component,
			     struct snd_kcontrol_new *controls,
			     unsigned int num_controls,
			     int ch, const char *suffix1, const char *suffix2)
{
	struct sta_control *chinfo;
	char *name;
	const char *base;
	int i;

	for (i = 0; i < num_controls; i++) {
		base = controls[i].name;
		if (suffix1 && suffix2)
			name = kasprintf(GFP_KERNEL, "%s %s %s", base,
					 suffix1, suffix2);
		else if (suffix1)
			name = kasprintf(GFP_KERNEL, "%s %s", base, suffix1);
		else
			name = kasprintf(GFP_KERNEL, "%s", base);
		controls[i].name = name;
		snd_soc_add_component_controls(component, &controls[i], 1);
		controls[i].name = base;
		kfree(name);

		chinfo = devm_kzalloc(
			component->dev,
			sizeof(struct sta_control),
			GFP_KERNEL);
		if (!chinfo)
			return -ENOMEM;
		list_add(&chinfo->list, &control_list);
		chinfo->numid = component->card->snd_card->last_numid;
		chinfo->ch = ch;
	}

	return 0;
}

static int module_ch_num(struct sta_module *module)
{
	return module->num_channels;
}

/* PCM Chime Play */

static int pcmchime_load_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
    		return -ENOENT;

	ucontrol->value.integer.value[0] = module->pcmchime.loaded;

	return 0;
}

static int pcmchime_load_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	struct device *dev;
	int val = ucontrol->value.integer.value[0];
	const struct firmware *fw_pcm;
	char filename[ST_CODEC_DSP_MAX_FILENAME];
	int err;

	if (!module)
		return -ENOENT;

	dev = module->component->dev;
	if (val == 0)
		return -EINVAL;

	if (module->pcmchime.loaded)
		return 0;

	snprintf(filename, ST_CODEC_DSP_MAX_FILENAME, "%s%s", ST_CODEC_DSP_FW_SUBPATH, module->pcmchime.file);
	err = request_firmware(&fw_pcm, filename, dev);
	if (err < 0)
		return -EINVAL;

	STA_PCMLoadData_16bit(module->mod, fw_pcm->data, fw_pcm->size);
	module->pcmchime.loaded = 1;

	release_firmware(fw_pcm);

	dev_info(dev, "PCM chime loaded\n");

	return 0;
}

static int pcmchime_play_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

static int pcmchime_play_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	static STA_PCMChimeParams pcm_chime_params;
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	if (val) {
		pcm_chime_params.playCount = module->pcmchime.play_count;
		pcm_chime_params.leftDelay = module->pcmchime.left_delay;
		pcm_chime_params.leftPostDelay =
					module->pcmchime.left_post_delay;
		pcm_chime_params.rightDelay = module->pcmchime.right_delay;
		pcm_chime_params.rightPostDelay =
					module->pcmchime.right_post_delay;
		STA_PCMPlay(module->mod, &pcm_chime_params);
	} else {
		STA_PCMStop(module->mod);
	}

	return 0;
}

static const struct snd_kcontrol_new st_codec_dsp_pcmchime_controls[] = {
	SOC_ENUM_EXT(
		"PCM Chime Play",
		switch_enum,
		pcmchime_play_get, pcmchime_play_put),
	SOC_ENUM_EXT(
		"PCM Chime Load",
		switch_enum,
		pcmchime_load_get, pcmchime_load_put),
};

/* Chime Generator */

static int chime_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	if (!module->chimegen.ON)
		return 0;
	module->chimegen.ON = !!(STA_ChimeGenGetParam(module->mod, STA_CHIME_REPEAT_COUNT));
	ucontrol->value.integer.value[0] = module->chimegen.ON;

	return 0;
}

static int chime_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	struct sta_module *slave;
	int val = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	STA_ChimeGenSetParam(module->mod, STA_CHIME_MASTER_CHIME, 0);
	list_for_each_entry(slave, &module_list, list)
		if (slave->type == STA_CHIMEGEN && slave->chimegen.master == module)
			STA_ChimeGenSetParam(slave->mod, STA_CHIME_MASTER_CHIME, module->mod);

	module->chimegen.ON = val;
	if (val == 0) {
		STA_ChimeGenStop(module->mod, STA_RAMP_LIN, CHIME_RAMP_STOP_TIME);
		list_for_each_entry(slave, &module_list, list)
			if (slave->type == STA_CHIMEGEN && slave->chimegen.master == module) {
				STA_ChimeGenStop(slave->mod, STA_RAMP_LIN, CHIME_RAMP_STOP_TIME);
				slave->chimegen.ON = 0;
			}
	} else {
		STA_ChimeGenPlay(module->mod, &module->chimegen.params);
		list_for_each_entry(slave, &module_list, list)
			if (slave->type == STA_CHIMEGEN && slave->chimegen.master == module) {
				STA_ChimeGenPlay(slave->mod, &slave->chimegen.params);
				slave->chimegen.ON = 1;
			}
	}

	return 0;
}


static int chime_repeat_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] =
		STA_ChimeGenGetParam(module->mod, STA_CHIME_REPEAT_COUNT);

	return 0;
}

static int chime_repeat_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->chimegen.params.repeatCount = ucontrol->value.integer.value[0];
	STA_ChimeGenSetParam(module->mod, STA_CHIME_REPEAT_COUNT,
			     module->chimegen.params.repeatCount);

	return 	0;
}

static int chime_postrepeat_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->chimegen.params.postRepeatRampIdx;
	return 0;
}

static int chime_postrepeat_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	if (i < module->chimegen.params.numRamps)
		module->chimegen.params.postRepeatRampIdx = i;
	else
		module->chimegen.params.postRepeatRampIdx = CHIME_POST_REPEAT_ID_INIT;

	return 	0;

}

static int chime_postrepeat_enable_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] =
		STA_ChimeGenGetParam(module->mod, STA_CHIME_POST_REPEAT_RAMPS);

	return 0;
}

static int chime_postrepeat_enable_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	STA_ChimeGenSetParam(module->mod, STA_CHIME_POST_REPEAT_RAMPS,
			     ucontrol->value.integer.value[0]);

	return 	0;
}

static int ramp_type_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		ucontrol->value.integer.value[i] = module->chimegen.params.ramps[i].type;
	return 0;
}

static int ramp_type_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		module->chimegen.params.ramps[i].type = ucontrol->value.integer.value[i];
	return 0;
}

static int ramp_amp_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		ucontrol->value.integer.value[i] = module->chimegen.params.ramps[i].ampl;
	return 0;
}

static int ramp_amp_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		module->chimegen.params.ramps[i].ampl = ucontrol->value.integer.value[i];
	return 0;
}

static int ramp_dur_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		ucontrol->value.integer.value[i] = module->chimegen.params.ramps[i].msec;
	return 0;
}

static int ramp_dur_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		module->chimegen.params.ramps[i].msec = ucontrol->value.integer.value[i];
	return 0;
}

static int ramp_freq_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		ucontrol->value.integer.value[i] = module->chimegen.params.ramps[i].freq;
	return 0;
}

static int ramp_freq_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	static int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->chimegen.params.numRamps; i++)
		module->chimegen.params.ramps[i].freq = ucontrol->value.integer.value[i];
	return 0;
}

static int ramp_count(struct sta_module *module)
{
	return module->chimegen.params.numRamps;
}

static const struct snd_kcontrol_new st_codec_dsp_chime_controls[] = {
	SOC_ENUM_EXT("Chime", switch_enum, chime_get, chime_put),
	STA_RANGE("Chime Repeat", chime_repeat_get, chime_repeat_put, 1,
		  CHIME_REPEAT_COUNT_MIN, CHIME_REPEAT_COUNT_MAX),
	STA_RANGE_EXT("Chime Post Repeat", chime_postrepeat_get, chime_postrepeat_put, 1,
		      0, ramp_count),
	SOC_ENUM_EXT("Chime Post Enable", switch_enum,
		     chime_postrepeat_enable_get, chime_postrepeat_enable_put),
};

static struct snd_kcontrol_new st_codec_dsp_chime_ramp_controls[] = {
	STA_COUNT_EXT("Chime Ramp Type", ramp_type_get, ramp_type_put,
		      ramp_count, STA_RAMP_STC, STA_RAMP_SMP),
	STA_COUNT_EXT("Chime Ramp Amp", ramp_amp_get, ramp_amp_put, ramp_count,
		      CHIME_RAMP_AMPLITUDE_MIN, CHIME_RAMP_AMPLITUDE_MAX),
	STA_COUNT_EXT("Chime Ramp Dur", ramp_dur_get, ramp_dur_put, ramp_count,
		      CHIME_RAMP_DURATION_MIN, CHIME_RAMP_DURATION_MAX),
	STA_COUNT_EXT("Chime Ramp Freq", ramp_freq_get, ramp_freq_put,
		      ramp_count, CHIME_RAMP_FREQ_MIN, CHIME_RAMP_FREQ_MAX),
};

/* Beep Generator */

static int sine_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->sinegen.sine_freq;
	ucontrol->value.integer.value[1] = module->sinegen.sine_duration;
	ucontrol->value.integer.value[2] = -module->sinegen.sine_gain;

	return 0;
}

static int sine_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int val2 = ucontrol->value.integer.value[1];
	int val3 = ucontrol->value.integer.value[2];

	if (!module)
		return -ENOENT;
	if (val < SINE_FREQ_MIN || val > SINE_FREQ_MAX)
		return -EINVAL;

	if (val2 < SINE_DURATION_MIN || val2 > SINE_DURATION_MAX)
		return -EINVAL;

	if (val3 < SINE_GAIN_MIN || val3 > SINE_GAIN_MAX)
		return -EINVAL;

	module->sinegen.sine_freq = val;
	module->sinegen.sine_duration = val2;
	module->sinegen.sine_gain = -val3;
	if (module->sinegen.ON == STA_SINE_ON) {
	    STA_SinewaveGenSet(
			    module->mod,
			    module->sinegen.sine_freq,
			    module->sinegen.sine_duration,
			    module->sinegen.sine_gain,
			    module->sinegen.sine_gain);
	}
	return 0;
}

static int beep_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

static int beep_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	if (val == 0) {
		module->sinegen.ON = STA_SINE_OFF;
		STA_SetMode(module->mod, STA_SINE_OFF);
		STA_SinewaveGenSet(
				module->mod,
				module->sinegen.sine_freq,
				0,
				module->sinegen.sine_gain,
				module->sinegen.sine_gain);
	} else {
		module->sinegen.ON = STA_SINE_ON;
		STA_SetMode(module->mod, STA_SINE_ON);
		STA_SinewaveGenSet(
			module->mod,
			module->sinegen.sine_freq,
			module->sinegen.sine_duration,
			module->sinegen.sine_gain,
			module->sinegen.sine_gain);
	}

	return 0;
}

static const struct snd_kcontrol_new st_codec_dsp_beep_controls[] = {
	SOC_ENUM_EXT(
		"Beep",
		switch_enum,
		beep_get, beep_put),
	STA_MULTI_RANGE(
		"Sine", sine_get, sine_put, 3,
		SINE_FREQ_MIN, SINE_FREQ_MAX,
		SINE_DURATION_MIN, SINE_DURATION_MAX,
		SINE_GAIN_MIN, SINE_GAIN_MAX),
};

/*
 * LIMITER
 */

static int limiter_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = (module->limiter.ON == STA_LMTR_ON);

	return 0;
}

static int limiter_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	if (val == 1) {
		STA_SetMode(module->mod, STA_LMTR_ON);
		module->limiter.ON = STA_LMTR_ON;
	} else {
		STA_SetMode(module->mod, STA_LMTR_OFF);
		module->limiter.ON = STA_LMTR_OFF;
	}

	return 0;
}

static int limiter_threshold_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_THRES];

	return 0;
}

static int limiter_threshold_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_THRES] = val0;
	STA_LimiterSetParams(
		module->mod, STA_LMTR_THRES, module->limiter.params);

	return 0;
}

static int limiter_attenuation_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_ATTENUATION];

	return 0;
}

static int limiter_attenuation_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_ATTENUATION] = val0;
	STA_LimiterSetParams(
		module->mod, STA_LMTR_ATTENUATION, module->limiter.params);

	return 0;
}

static int limiter_attack_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_ATTACK_TIME];

	return 0;
}

static int limiter_attack_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_ATTACK_TIME] = val0;
	STA_LimiterSetParams(module->mod,
		STA_LMTR_ATTACK_TIME, module->limiter.params);

	return 0;
}

static int limiter_release_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_RELEASE_TIME];

	return 0;
}

static int limiter_release_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_RELEASE_TIME] = val0;
	STA_LimiterSetParams(
		module->mod, STA_LMTR_RELEASE_TIME, module->limiter.params);

	return 0;
}

static int limiter_hysteresis_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_HYSTERESIS];

	return 0;
}

static int limiter_hysteresis_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_HYSTERESIS] = val0;
	STA_LimiterSetParams(
		module->mod, STA_LMTR_HYSTERESIS, module->limiter.params);

	return 0;
}

static int limiter_peak_attack_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_PEAK_ATK_TIME];

	return 0;
}

static int limiter_peak_attack_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_PEAK_ATK_TIME] = val0;
	STA_LimiterSetParams(module->mod,
		STA_LMTR_PEAK_ATK_TIME, module->limiter.params);

	return 0;
}

static int limiter_peak_release_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->limiter.params[STA_LMTR_IDX_PEAK_REL_TIME];

	return 0;
}

static int limiter_peak_release_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->limiter.params[STA_LMTR_IDX_PEAK_REL_TIME] = val0;
	STA_LimiterSetParams(
		module->mod, STA_LMTR_PEAK_REL_TIME, module->limiter.params);

	return 0;
}

static struct snd_kcontrol_new st_codec_dsp_limiter_controls[] = {
	SOC_ENUM_EXT(
		"Limiter",
		switch_enum,
		limiter_get, limiter_put),
	STA_RANGE(
		"Limiter Threshold",
		limiter_threshold_get, limiter_threshold_put, 1,
		LIMITER_THRESHOLD_MIN, LIMITER_THRESHOLD_MAX),
	STA_RANGE(
		"Limiter Attenuation",
		limiter_attenuation_get, limiter_attenuation_put, 1,
		LIMITER_ATTENUATION_MIN, LIMITER_ATTENUATION_MAX),
	STA_RANGE(
		"Limiter Attack Time",
		limiter_attack_time_get, limiter_attack_time_put, 1,
		LIMITER_ATTACK_TIME_MIN, LIMITER_ATTACK_TIME_MAX),
	STA_RANGE(
		"Limiter Release Time",
		limiter_release_time_get, limiter_release_time_put, 1,
		LIMITER_RELEASE_TIME_MIN, LIMITER_RELEASE_TIME_MAX),
	STA_RANGE(
		"Limiter Hysteresis",
		limiter_hysteresis_get, limiter_hysteresis_put, 1,
		LIMITER_HYSTERESIS_MIN, LIMITER_HYSTERESIS_MAX),
	STA_RANGE(
		"Limiter Peak Attack",
		limiter_peak_attack_time_get, limiter_peak_attack_time_put, 1,
		LIMITER_PEAK_ATTACK_TIME_MIN, LIMITER_PEAK_ATTACK_TIME_MAX),
	STA_RANGE(
		"Limiter Peak Release",
		limiter_peak_release_time_get, limiter_peak_release_time_put, 1,
		LIMITER_PEAK_RELEASE_TIME_MIN, LIMITER_PEAK_RELEASE_TIME_MAX),
};

/*
 * CLIP LIMITER
 */

static int clip_limiter_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->clip_limiter.status;

	return 0;
}

static int clip_limiter_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->clip_limiter.status = ucontrol->value.integer.value[0];
	STA_SetMode(module->mod, module->clip_limiter.status);

	return 0;
}

static int clip_limiter_max_attenuation_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->clip_limiter.params[STA_CLIPLMTR_IDX_MAX_ATTENUATION];

	return 0;
}

static int clip_limiter_max_attenuation_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->clip_limiter.params[STA_CLIPLMTR_IDX_MAX_ATTENUATION] = val0;
	STA_ClipLimiterSetParams(
		module->mod, STA_CLIPLMTR_MAX_ATTENUATION, module->clip_limiter.params);

	return 0;
}

static int clip_limiter_min_attenuation_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->clip_limiter.params[STA_CLIPLMTR_IDX_MIN_ATTENUATION];

	return 0;
}

static int clip_limiter_min_attenuation_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->clip_limiter.params[STA_CLIPLMTR_IDX_MIN_ATTENUATION] = val0;
	STA_ClipLimiterSetParams(
		module->mod, STA_CLIPLMTR_MIN_ATTENUATION, module->clip_limiter.params);

	return 0;
}

static int clip_limiter_adj_attenuation_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->clip_limiter.params[STA_CLIPLMTR_IDX_ADJ_ATTENUATION];

	return 0;
}

static int clip_limiter_adj_attenuation_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->clip_limiter.params[STA_CLIPLMTR_IDX_ADJ_ATTENUATION] = val0;
	STA_ClipLimiterSetParams(
		module->mod, STA_CLIPLMTR_ADJ_ATTENUATION, module->clip_limiter.params);

	return 0;
}

static int clip_limiter_attack_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->clip_limiter.params[STA_CLIPLMTR_IDX_ATTACK_TIME];

	return 0;
}

static int clip_limiter_attack_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->clip_limiter.params[STA_CLIPLMTR_IDX_ATTACK_TIME] = val0;
	STA_ClipLimiterSetParams(module->mod,
				 STA_CLIPLMTR_ATTACK_TIME,
				 module->clip_limiter.params);

	return 0;
}

static int clip_limiter_release_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->clip_limiter.params[STA_CLIPLMTR_IDX_RELEASE_TIME];

	return 0;
}

static int clip_limiter_release_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->clip_limiter.params[STA_CLIPLMTR_IDX_RELEASE_TIME] = val0;
	STA_ClipLimiterSetParams(
		module->mod, STA_CLIPLMTR_RELEASE_TIME, module->clip_limiter.params);

	return 0;
}

static struct snd_kcontrol_new clip_limiter_controls[] = {
	SOC_ENUM_EXT(
		"Clip Limiter",
		clip_enum,
		clip_limiter_get, clip_limiter_put),
	STA_RANGE(
		"Clip Limiter Max Attenuation",
		clip_limiter_max_attenuation_get, clip_limiter_max_attenuation_put, 1,
		CLIP_LIMITER_ATTENUATION_MIN, CLIP_LIMITER_ATTENUATION_MAX),
	STA_RANGE(
		"Clip Limiter Min Attenuation",
		clip_limiter_min_attenuation_get, clip_limiter_min_attenuation_put, 1,
		CLIP_LIMITER_ATTENUATION_MIN, CLIP_LIMITER_ATTENUATION_MAX),
	STA_RANGE(
		"Clip Limiter Adjust Attenuation",
		clip_limiter_adj_attenuation_get, clip_limiter_adj_attenuation_put, 1,
		CLIP_LIMITER_ATTENUATION_MIN, CLIP_LIMITER_ATTENUATION_MAX),
	STA_RANGE(
		"Clip Limiter Attack Time",
		clip_limiter_attack_time_get, clip_limiter_attack_time_put, 1,
		CLIP_LIMITER_TIME_MIN, CLIP_LIMITER_TIME_MAX),
	STA_RANGE(
		"Clip Limiter Release Time",
		clip_limiter_release_time_get, clip_limiter_release_time_put, 1,
		CLIP_LIMITER_TIME_MIN, CLIP_LIMITER_TIME_MAX),
};

/*
 * COMPANDER
 */

static int compander_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.status;

	return 0;
}

static int compander_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->compander.status = ucontrol->value.integer.value[0];
	STA_SetMode(module->mod, module->compander.status);

	return 0;
}

static int compander_mean_max_mode_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_MEAN_MAX_MODE];

	return 0;
}

static int compander_mean_max_mode_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_MEAN_MAX_MODE] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_MEAN_MAX_MODE, module->compander.params);

	return 0;
}

static int compander_avg_factor_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_AVG_FACTOR];

	return 0;
}

static int compander_avg_factor_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_AVG_FACTOR] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_AVG_FACTOR, module->compander.params);

	return 0;
}

static int compander_attenuation_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_ATTENUATION];

	return 0;
}

static int compander_attenuation_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_ATTENUATION] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_ATTENUATION, module->compander.params);

	return 0;
}

static int compander_cpr_thres_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_CPR_THRES];

	return 0;
}

static int compander_cpr_thres_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_CPR_THRES] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_CPR_THRES, module->compander.params);

	return 0;
}

static int compander_cpr_slope_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_CPR_SLOPE];

	return 0;
}

static int compander_cpr_slope_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_CPR_SLOPE] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_CPR_SLOPE, module->compander.params);

	return 0;
}

static int compander_cpr_hysteresis_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_CPR_HYSTERESIS];

	return 0;
}

static int compander_cpr_hysteresis_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_CPR_HYSTERESIS] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_CPR_HYSTERESIS, module->compander.params);

	return 0;
}

static int compander_cpr_atk_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_CPR_ATK_TIME];

	return 0;
}

static int compander_cpr_atk_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_CPR_ATK_TIME] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_CPR_ATK_TIME, module->compander.params);

	return 0;
}

static int compander_cpr_rel_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_CPR_REL_TIME];

	return 0;
}

static int compander_cpr_rel_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_CPR_REL_TIME] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_CPR_REL_TIME, module->compander.params);

	return 0;
}

static int compander_exp_thres_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_EXP_THRES];

	return 0;
}

static int compander_exp_thres_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_EXP_THRES] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_EXP_THRES, module->compander.params);

	return 0;
}

static int compander_exp_slope_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_EXP_SLOPE];

	return 0;
}

static int compander_exp_slope_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_EXP_SLOPE] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_EXP_SLOPE, module->compander.params);

	return 0;
}

static int compander_exp_hysteresis_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_EXP_HYSTERESIS];

	return 0;
}

static int compander_exp_hysteresis_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_EXP_HYSTERESIS] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_EXP_HYSTERESIS, module->compander.params);

	return 0;
}

static int compander_exp_atk_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_EXP_ATK_TIME];

	return 0;
}

static int compander_exp_atk_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_EXP_ATK_TIME] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_EXP_ATK_TIME, module->compander.params);

	return 0;
}

static int compander_exp_rel_time_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->compander.params[STA_COMP_IDX_EXP_REL_TIME];

	return 0;
}

static int compander_exp_rel_time_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val0 = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	module->compander.params[STA_COMP_IDX_EXP_REL_TIME] = val0;
	STA_CompanderSetParams(
		module->mod, STA_COMP_EXP_REL_TIME, module->compander.params);

	return 0;
}

static struct snd_kcontrol_new compander_controls[] = {
	SOC_ENUM_EXT(
		"Compander",
		switch_enum,
		compander_get, compander_put),
	SOC_ENUM_EXT(
		"Compander Mean Max Mode",
		comp_enum,
		compander_mean_max_mode_get, compander_mean_max_mode_put),
	STA_RANGE(
		"Compander Avg Factor",
		compander_avg_factor_get, compander_avg_factor_put, 1,
		COMPANDER_AVG_FACTOR_MIN, COMPANDER_AVG_FACTOR_MAX),
	STA_RANGE(
		"Compander Attenuation",
		compander_attenuation_get, compander_attenuation_put, 1,
		COMPANDER_ATTENUATION_MIN, COMPANDER_ATTENUATION_MAX),
	STA_RANGE(
		"Compander Compression Threshold",
		compander_cpr_thres_get, compander_cpr_thres_put, 1,
		COMPANDER_THRESHOLD_MIN, COMPANDER_THRESHOLD_MAX),
	STA_RANGE(
		"Compander Compression Slope",
		compander_cpr_slope_get, compander_cpr_slope_put, 1,
		COMPANDER_SLOPE_MIN, COMPANDER_SLOPE_MAX),
	STA_RANGE(
		"Compander Compression Hysteresis",
		compander_cpr_hysteresis_get, compander_cpr_hysteresis_put, 1,
		COMPANDER_HYSTERESIS_MIN, COMPANDER_HYSTERESIS_MAX),
	STA_RANGE(
		"Compander Compression Attack Time",
		compander_cpr_atk_time_get, compander_cpr_atk_time_put, 1,
		COMPANDER_TIME_MIN, COMPANDER_TIME_MAX),
	STA_RANGE(
		"Compander Compression Release Time",
		compander_cpr_rel_time_get, compander_cpr_rel_time_put, 1,
		COMPANDER_TIME_MIN, COMPANDER_TIME_MAX),
	STA_RANGE(
		"Compander Expansion Threshold",
		compander_exp_thres_get, compander_exp_thres_put, 1,
		COMPANDER_THRESHOLD_MIN, COMPANDER_THRESHOLD_MAX),
	STA_RANGE(
		"Compander Expansion Slope",
		compander_exp_slope_get, compander_exp_slope_put, 1,
		COMPANDER_SLOPE_MIN, COMPANDER_SLOPE_MAX),
	STA_RANGE(
		"Compander Expansion Hysteresis",
		compander_exp_hysteresis_get, compander_exp_hysteresis_put, 1,
		COMPANDER_HYSTERESIS_MIN, COMPANDER_HYSTERESIS_MAX),
	STA_RANGE(
		"Compander Expansion Attack Time",
		compander_exp_atk_time_get, compander_exp_atk_time_put, 1,
		COMPANDER_TIME_MIN, COMPANDER_TIME_MAX),
	STA_RANGE(
		"Compander Expansion Release Time",
		compander_exp_rel_time_get, compander_exp_rel_time_put, 1,
		COMPANDER_TIME_MIN, COMPANDER_TIME_MAX),
};

/*
 * MIXER
 */

static int mix_ins_count(struct sta_module *module)
{
	return 2 + module->type - STA_MIXE_2INS_NCH;
}

static int mix_ch_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);
	int in, out;

	if (ch < 0 || !module)
		return -ENOENT;

	in = ch / module->num_out;
	out = ch % module->num_out;
	ucontrol->value.integer.value[0] = DB_TO_VAL(module->mixer.vol[out][in]);

	return 0;
}

static int mix_ch_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);
	int in, out;
	s16 db_volume;

	if (ch < 0 || !module)
		return -ENOENT;

	in = ch / module->num_out;
	out = ch % module->num_out;
	db_volume = VAL_TO_DB(ucontrol->value.integer.value[0]);
	module->mixer.vol[out][in] = db_volume;
	STA_MixerSetRelativeInGain(module->mod, out, in, db_volume);

	return 0;
}

static int mix_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int in;

	if (!module)
		return -ENOENT;
	for (in = 0; in < mix_ins_count(module); in++)
		ucontrol->value.integer.value[in] = DB_TO_VAL(module->mixer.vol[0][in]);

	return 0;
}

static int mix_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int out, in;

	if (!module)
		return -ENOENT;
	for (out = 0; out < module->num_channels; out++) {
		for (in = 0; in < mix_ins_count(module); in++)
			module->mixer.vol[out][in] = VAL_TO_DB(ucontrol->value.integer.value[in]);
		STA_MixerSetChannelInGains(module->mod, out,
					   module->mixer.vol[out]);
	}

	return 0;
}


static struct snd_kcontrol_new mixer_multi_controls[] = {
	STA_COUNT_EXT(
		"Mix", mix_multi_get, mix_multi_put, mix_ins_count,
		MIXER_VOLUME_MIN, MIXER_VOLUME_MAX),
};

static struct snd_kcontrol_new mixer_ch_controls[] = {
	STA_RANGE(
		"Mix", mix_ch_get, mix_ch_put, 1,
		MIXER_VOLUME_MIN, MIXER_VOLUME_MAX),
};

static int mixer_add_controls(struct sta_module *module,
			      struct snd_soc_component *component)
{
	int i, j;

	snd_soc_add_component_controls(component,
				   mixer_multi_controls,
				   ARRAY_SIZE(mixer_multi_controls));
	for (i = 0; i < module->num_ins; i++)
		for (j = 0; j < module->num_out; j++)
			sta_add_channel_controls(component,
						 mixer_ch_controls,
						 ARRAY_SIZE(mixer_ch_controls),
						 i * module->num_out + j,
						 module->in_names[i],
						 module->out_names[j]);

	return 0;
}

/*
 * EQUALIZER
 */

static int equalizer_writepbg_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val_p = module->equalizer.mode;
	int val_b = 0;
	int val_g = module->equalizer.preset[val_p].gains[val_b];

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = val_p;
	ucontrol->value.integer.value[1] = val_b;
	ucontrol->value.integer.value[2] = val_g;

	return 0;
}

static int equalizer_writepbg_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val_p = ucontrol->value.integer.value[0];
	int val_b = ucontrol->value.integer.value[1];
	int val_g = ucontrol->value.integer.value[2];

	if (!module)
		return -ENOENT;
	if (val_p < EQ_PRESETS_MIN || val_p >  EQ_PRESETS_MAX)
		return -EINVAL;

	if (val_b < EQ_BAND_MIN || val_b > EQ_BAND_MAX)
		return -EINVAL;

	if (val_g < EQ_GAINBOOST_MIN || val_g > EQ_GAINBOOST_MAX)
		return -EINVAL;

	module->equalizer.mode = val_p;
	module->equalizer.preset[val_p].gains[val_b] = val_g;

	if (module->equalizer.ON == STA_EQ_OFF)
		return 0;

	STA_SetFilterGains(
		module->mod, -1, module->equalizer.preset[val_p].gains);

	return 0;
}

static int equalizer_num_bands(struct sta_module *module)
{
	return module->equalizer.num_bands;
}

static int equalizer_gain_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->equalizer.bandGQF[band][STA_GAIN];

	return 0;
}

static int equalizer_gain_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val_g = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->equalizer.bandGQF[band][STA_GAIN] = val_g;
	if (module->equalizer.ON == STA_EQ_ON)
		STA_SetFilterv(
			module->mod, band, module->equalizer.bandGQF[band]);

	return 0;
}

static int equalizer_qual_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->equalizer.bandGQF[band][STA_QUAL];

	return 0;
}

static int equalizer_qual_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val_q = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->equalizer.bandGQF[band][STA_QUAL] = val_q;
	if (module->equalizer.ON == STA_EQ_ON)
		STA_SetFilterv(
			module->mod, band, module->equalizer.bandGQF[band]);

	return 0;
}

static int equalizer_band_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->equalizer.bandGQF[band][STA_FREQ];

	return 0;
}

static int equalizer_band_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val_b = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->equalizer.bandGQF[band][STA_FREQ] = val_b;
	if (module->equalizer.ON == STA_EQ_ON)
		STA_SetFilterv(
			module->mod, band, module->equalizer.bandGQF[band]);

	return 0;
}

static int equalizer_gains_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->equalizer.num_bands; i++)
		ucontrol->value.integer.value[i] = module->equalizer.bandGQF[i][STA_GAIN];

	return 0;
}

static int equalizer_gains_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i, val_g;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->equalizer.num_bands; i++){
		val_g = ucontrol->value.integer.value[i];
		module->equalizer.bandGQF[i][STA_GAIN] = val_g;
		if (module->equalizer.ON == STA_EQ_ON)
			STA_SetFilterv(
				module->mod, i, module->equalizer.bandGQF[i]);
	}

	return 0;
}

static int equalizer_quals_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->equalizer.num_bands; i++)
		ucontrol->value.integer.value[i] = module->equalizer.bandGQF[i][STA_QUAL];

	return 0;
}

static int equalizer_quals_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i, val_q;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->equalizer.num_bands; i++){
		val_q = ucontrol->value.integer.value[i];
		module->equalizer.bandGQF[i][STA_QUAL] = val_q;
		if (module->equalizer.ON == STA_EQ_ON)
			STA_SetFilterv(
				module->mod, i, module->equalizer.bandGQF[i]);
	}

	return 0;
}

static int equalizer_bands_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->equalizer.num_bands; i++)
		ucontrol->value.integer.value[i] = module->equalizer.bandGQF[i][STA_FREQ];

	return 0;
}

static int equalizer_bands_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i, val_b;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->equalizer.num_bands; i++){
		val_b = ucontrol->value.integer.value[i];
		module->equalizer.bandGQF[i][STA_FREQ] = val_b;
		if (module->equalizer.ON == STA_EQ_ON)
			STA_SetFilterv(
				module->mod, i, module->equalizer.bandGQF[i]);
	}

	return 0;
}

static int equalizer_mode_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->equalizer.mode;

	return 0;
}

static int equalizer_mode_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int i;

	if (!module)
		return -ENOENT;
	module->equalizer.mode = val;

	for (i = 0; i < module->equalizer.num_bands; i++)
		module->equalizer.bandGQF[i][STA_GAIN] = module->equalizer.preset[val].gains[i];

	if (module->equalizer.ON == STA_EQ_OFF)
		return 0;

	STA_SetFilterGains(
		module->mod, -1, module->equalizer.preset[val].gains);

	return 0;
}

static int equalizer_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = (module->equalizer.ON == STA_EQ_ON);

	return 0;
}

static int equalizer_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int i;

	if (!module)
		return -ENOENT;
	if (val == 0) {
		if (module->equalizer.ON != STA_EQ_OFF)
			STA_SetMode(module->mod, STA_EQ_OFF);
		module->equalizer.ON = STA_EQ_OFF;
	} else {
		STA_SetMode(module->mod, STA_EQ_ON);
		module->equalizer.ON = STA_EQ_ON;
		for (i = 0; i < module->equalizer.num_bands; i++)
			STA_SetFilterv(
				module->mod, i, module->equalizer.bandGQF[i]);
	}

	return 0;
}

static int equalizer_filter_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->equalizer.filter_type[band];

	return 0;
}

static int equalizer_filter_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	module->equalizer.filter_type[band] = ucontrol->value.integer.value[0];
	STA_SetFilterType(module->mod, band, module->equalizer.filter_type[band]);

	return 0;
}

static const struct soc_enum equalizer_dummy;

static const struct snd_kcontrol_new eq_controls[] = {
	SOC_ENUM_EXT("Equalizer", switch_enum, equalizer_get, equalizer_put),
	STA_MULTI_RANGE("Equalizer Write PBG",
			equalizer_writepbg_get, equalizer_writepbg_put, 3,
			EQ_PRESETS_MIN, EQ_PRESETS_MAX,
			EQ_BAND_MIN, EQ_BAND_MAX,
			EQ_GAINBOOST_MIN, EQ_GAINBOOST_MAX),
	STA_COUNT_EXT("Equalizer Gains", equalizer_gains_get,
		      equalizer_gains_put, equalizer_num_bands,
		      EQ_GAIN_MIN, EQ_GAIN_MAX),
	STA_COUNT_EXT("Equalizer Qualities", equalizer_quals_get,
		      equalizer_quals_put, equalizer_num_bands,
		      EQ_QUAL_MIN, EQ_QUAL_MAX),
	STA_COUNT_EXT("Equalizer Bands", equalizer_bands_get,
		      equalizer_bands_put, equalizer_num_bands,
		      EQ_FREQUENCIES_MIN, EQ_FREQUENCIES_MAX),
};

static struct snd_kcontrol_new eq_ch_controls[] = {
	STA_RANGE(
		"Equalizer Gain", equalizer_gain_get, equalizer_gain_put, 1,
		EQ_GAIN_MIN, EQ_GAIN_MAX),
	STA_RANGE(
		"Equalizer Quality", equalizer_qual_get, equalizer_qual_put, 1,
		EQ_QUAL_MIN, EQ_QUAL_MAX),
	STA_RANGE(
		"Equalizer Band", equalizer_band_get, equalizer_band_put, 1,
		EQ_FREQUENCIES_MIN, EQ_FREQUENCIES_MAX),
	SOC_ENUM_EXT(
		"Equalizer Filter", eq_filter_enum,
		equalizer_filter_get, equalizer_filter_put),
};

static struct snd_kcontrol_new eq_preset_control =
	SOC_ENUM_EXT("Equalizer Mode", equalizer_dummy,
		     equalizer_mode_get, equalizer_mode_put);

static int equalizer_add_controls(struct sta_module *module,
				  struct snd_soc_component *component)
{
	int i;
	char str_i[3];

	if (module->equalizer.coefs)
		return 0;

	snd_soc_add_component_controls(component, eq_controls,
				   ARRAY_SIZE(eq_controls));
	/* change enum into the control to module specific presets */
	eq_preset_control.private_value =
			(unsigned long)&module->equalizer.preset_enum;
	snd_soc_add_component_controls(component, &eq_preset_control, 1);
	for (i = 0; i < module->equalizer.num_bands; i++) {
		sprintf(str_i, "%d", i);
		sta_add_channel_controls(component,
					 eq_ch_controls,
					 ARRAY_SIZE(eq_ch_controls),
					 i, str_i, NULL);
	}

	return 0;
}

static int initialize_EQ(struct sta_module *module)
{
	int i;

	if (module->equalizer.coefs) {
		STA_SetFilterHalvedCoefsAll(module->mod, 0x3F, module->equalizer.coefs);
		STA_SetMode(module->mod, STA_EQ_ON);
	} else {
	    STA_SetMode(module->mod, STA_EQ_OFF);
	    module->equalizer.ON = STA_EQ_OFF;
	    module->equalizer.mode = 0;

	    for (i = 0; i < module->equalizer.num_bands; i++) {
		    STA_SetFilterType(
			    module->mod, i, module->equalizer.filter_type[i]);
	    }
	}

	return 0;
}

/*
 * LOUDNESS: BASS AND TREBLE
 */

static int initialize_loudn(struct sta_module *module)
{
	int i;
	struct loudness	*loudn = &module->loudness;

	STA_SetMode(module->mod, STA_LOUD_OFF);
	loudn->ON = STA_LOUD_OFF;
	STA_LoudSetTypes(
		module->mod, loudn->filter_type[0], loudn->filter_type[1]);

	for (i = 0; i < ARRAY_SIZE(loudn->GQF); i++) {
		STA_SetFilter(
			module->mod, i,
			loudn->GQF[i][STA_GAIN],
			loudn->GQF[i][STA_QUAL],
			loudn->GQF[i][STA_FREQ]);
	}

	return 0;
}

static int loudness_status_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] =
		(module->loudness.ON != STA_LOUD_OFF);

	return 0;
}

static int loudness_status_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	if (ucontrol->value.integer.value[0] == 0) {
		STA_SetMode(module->mod, STA_LOUD_OFF);
		module->loudness.ON = STA_LOUD_OFF;
	} else {
		STA_SetMode(module->mod, STA_LOUD_MANUAL);
		module->loudness.ON = STA_LOUD_MANUAL;
	}

	return 0;
}

static int loudness_gqf_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band= get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->loudness.GQF[band][STA_GAIN];
	ucontrol->value.integer.value[1] = module->loudness.GQF[band][STA_QUAL];
	ucontrol->value.integer.value[2] = module->loudness.GQF[band][STA_FREQ];

	return 0;
}

static int loudness_gqf_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int val2 = ucontrol->value.integer.value[1];
	int val3 = ucontrol->value.integer.value[2];

	if (band < 0 || !module)
		return -ENOENT;

	if (val < LOUD_GAIN_MIN || val > LOUD_GAIN_MAX)
		return -EINVAL;

	if (val2 < LOUD_FILT_QUAL_MIN || val2 > LOUD_FILT_QUAL_MAX)
		return -EINVAL;

	if (val3 < LOUD_FILT_FREQ_MIN || val3 > LOUD_FILT_FREQ_MAX)
		return -EINVAL;

	module->loudness.GQF[band][STA_GAIN] = val;
	module->loudness.GQF[band][STA_QUAL] = val2;
	module->loudness.GQF[band][STA_FREQ] = val3;

	if (module->loudness.ON == STA_LOUD_OFF)
		return 0;

	STA_SetFilter(
		module->mod, band,
		module->loudness.GQF[band][STA_GAIN],
		module->loudness.GQF[band][STA_QUAL],
		module->loudness.GQF[band][STA_FREQ]);

	return 0;
}

static int loudness_gain_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->loudness.GQF[band][STA_GAIN];

	return 0;
}

static int loudness_gain_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->loudness.GQF[band][STA_GAIN] = val;

	if (module->loudness.ON == STA_LOUD_OFF)
		return 0;

	STA_SetFilterParam(
		module->mod, band, STA_GAIN, module->loudness.GQF[band][STA_GAIN]);

	return 0;
}

static int loudness_qual_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->loudness.GQF[band][STA_QUAL];

	return 0;
}

static int loudness_qual_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->loudness.GQF[band][STA_QUAL] = val;

	if (module->loudness.ON == STA_LOUD_OFF)
		return 0;

	STA_SetFilterParam(
		module->mod, band, STA_QUAL, module->loudness.GQF[band][STA_QUAL]);

	return 0;
}

static int loudness_freq_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->loudness.GQF[band][STA_FREQ];

	return 0;
}

static int loudness_freq_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->loudness.GQF[band][STA_FREQ] = val;

	if (module->loudness.ON == STA_LOUD_OFF)
		return 0;

	STA_SetFilterParam(
		module->mod, band, STA_FREQ, module->loudness.GQF[band][STA_FREQ]);

	return 0;
}

static int loudness_filter_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->loudness.filter_type[band];

	return 0;
}

static int loudness_filter_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	module->loudness.filter_type[band] = ucontrol->value.integer.value[0];
	STA_LoudSetTypes(module->mod,
			 module->loudness.filter_type[0],
			 module->loudness.filter_type[1]);

	return 0;
}

static int loudness_gain_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++)
		ucontrol->value.integer.value[i] = module->loudness.GQF[i][STA_GAIN];

	return 0;
}

static int loudness_gain_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++) {
		module->loudness.GQF[i][STA_GAIN] = ucontrol->value.integer.value[i];
		if (module->loudness.ON != STA_LOUD_OFF)
			STA_SetFilterParam(
				module->mod, i, STA_GAIN, module->loudness.GQF[i][STA_GAIN]);
	}

	return 0;
}

static int loudness_qual_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++)
		ucontrol->value.integer.value[i] = module->loudness.GQF[i][STA_QUAL];

	return 0;
}

static int loudness_qual_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++) {
		module->loudness.GQF[i][STA_QUAL] = ucontrol->value.integer.value[i];
		if (module->loudness.ON != STA_LOUD_OFF)
			STA_SetFilterParam(
				module->mod, i, STA_QUAL, module->loudness.GQF[i][STA_QUAL]);
	}

	return 0;
}

static int loudness_freq_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++)
		ucontrol->value.integer.value[i] = module->loudness.GQF[i][STA_FREQ];

	return 0;
}

static int loudness_freq_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++) {
		module->loudness.GQF[i][STA_FREQ] = ucontrol->value.integer.value[i];
		if (module->loudness.ON != STA_LOUD_OFF)
			STA_SetFilterParam(
				module->mod, i, STA_FREQ, module->loudness.GQF[i][STA_FREQ]);
	}

	return 0;
}

static int loudness_filter_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++)
	    ucontrol->value.integer.value[i] = module->loudness.filter_type[i];

	return 0;
}

static int loudness_filter_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->loudness.num_bands; i++)
	    module->loudness.filter_type[i] = ucontrol->value.integer.value[i];

	STA_LoudSetTypes(module->mod,
			 module->loudness.filter_type[0],
			 module->loudness.filter_type[1]);

	return 0;
}

static struct snd_kcontrol_new loud_controls[] = {
	SOC_ENUM_EXT(
		"Loudness",
		switch_enum,
		loudness_status_get, loudness_status_put),
	STA_ENUM(
		"Loudness Filters",
		loud_filter_enum,
		loudness_filter_multi_get, loudness_filter_multi_put, 2),
	STA_RANGE(
		"Loudness Gains",
		loudness_gain_multi_get, loudness_gain_multi_put, 2,
		LOUD_GAIN_MIN, LOUD_GAIN_MAX),
	STA_RANGE(
		"Loudness Quals",
		loudness_qual_multi_get, loudness_qual_multi_put, 2,
		LOUD_FILT_QUAL_MIN, LOUD_FILT_QUAL_MAX),
	STA_RANGE(
		"Loudness Freqs",
		loudness_freq_multi_get, loudness_freq_multi_put, 2,
		LOUD_FILT_FREQ_MIN, LOUD_FILT_FREQ_MAX),
};

static struct snd_kcontrol_new loud_band_controls[] = {
	STA_MULTI_RANGE(
		"Loudness",
		loudness_gqf_get, loudness_gqf_put, 3,
		LOUD_GAIN_MIN, LOUD_GAIN_MAX,
		LOUD_FILT_QUAL_MIN, LOUD_FILT_QUAL_MAX,
		LOUD_FILT_FREQ_MIN, LOUD_FILT_FREQ_MAX),
	STA_RANGE(
		"Loudness Gain", loudness_gain_get, loudness_gain_put, 1,
		LOUD_GAIN_MIN, LOUD_GAIN_MAX),
	STA_RANGE(
		"Loudness Qual", loudness_qual_get, loudness_qual_put, 1,
		LOUD_FILT_QUAL_MIN, LOUD_FILT_QUAL_MAX),
	STA_RANGE(
		"Loudness Freq", loudness_freq_get, loudness_freq_put, 1,
		LOUD_FILT_FREQ_MIN, LOUD_FILT_FREQ_MAX),
	SOC_ENUM_EXT(
		"Loudness Filter",
		loud_filter_enum,
		loudness_filter_get, loudness_filter_put),
};

static int loudness_add_controls(struct sta_module *module,
				  struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component,
				   loud_controls,
				   ARRAY_SIZE(loud_controls));
	sta_add_channel_controls(component, loud_band_controls,
				 ARRAY_SIZE(loud_band_controls),
				 0, "Bass", NULL);
	sta_add_channel_controls(component, loud_band_controls,
				 ARRAY_SIZE(loud_band_controls),
				 1, "Treble", NULL);

	return 0;
}

/*
 * TONES: BASS, MIDDLE AND TREBLE
 */

static int initialize_tones(struct sta_module *module)
{
	int i;
	struct tones *tones = &module->tones;

	STA_SetMode(module->mod, STA_TONE_OFF);
	tones->ON = STA_TONE_OFF;
	STA_ToneSetTypes(module->mod, tones->filter_type[0], tones->filter_type[2]);

	for (i = 0; i < ARRAY_SIZE(tones->GQF); i++) {
		STA_SetFilter(
			module->mod, i,
			tones->GQF[i][STA_GAIN],
			tones->GQF[i][STA_QUAL],
			tones->GQF[i][STA_FREQ]);
	}

	return 0;
}

static int tones_status_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] =
		(module->tones.ON != STA_TONE_OFF);

	return 0;
}

static int tones_status_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	if (ucontrol->value.integer.value[0] == 0) {
		STA_SetMode(module->mod, STA_TONE_OFF);
		module->tones.ON = STA_TONE_OFF;
	} else {
		STA_SetMode(module->mod, STA_TONE_MANUAL);
		module->tones.ON = STA_TONE_MANUAL;
	}

	return 0;
}

static int tones_gqf_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->tones.GQF[band][STA_GAIN];
	ucontrol->value.integer.value[1] = module->tones.GQF[band][STA_QUAL];
	ucontrol->value.integer.value[2] = module->tones.GQF[band][STA_FREQ];

	return 0;
}

static int tones_gqf_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int val2 = ucontrol->value.integer.value[1];
	int val3 = ucontrol->value.integer.value[2];

	if (band < 0 || !module)
		return -ENOENT;

	if (val < TONE_GAIN_MIN || val > TONE_GAIN_MAX)
		return -EINVAL;

	if (val2 < TONE_FILT_QUAL_MIN || val2 > TONE_FILT_QUAL_MAX)
		return -EINVAL;

	if (val3 < TONE_FILT_FREQ_MIN || val3 > TONE_FILT_FREQ_MAX)
		return -EINVAL;

	module->tones.GQF[band][STA_GAIN] = val;
	module->tones.GQF[band][STA_QUAL] = val2;
	module->tones.GQF[band][STA_FREQ] = val3;

	if (module->tones.ON == STA_TONE_OFF)
		return 0;

	STA_SetFilter(
		module->mod, band,
		module->tones.GQF[band][STA_GAIN],
		module->tones.GQF[band][STA_QUAL],
		module->tones.GQF[band][STA_FREQ]);

	return 0;
}

static int tones_gain_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->tones.GQF[band][STA_GAIN];

	return 0;
}

static int tones_gain_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->tones.GQF[band][STA_GAIN] = val;

	if (module->tones.ON == STA_TONE_OFF)
		return 0;

	STA_SetFilterParam(
		module->mod, band, STA_GAIN, module->tones.GQF[band][STA_GAIN]);

	return 0;
}

static int tones_qual_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->tones.GQF[band][STA_QUAL];

	return 0;
}

static int tones_qual_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->tones.GQF[band][STA_QUAL] = val;

	if (module->tones.ON == STA_TONE_OFF)
		return 0;

	STA_SetFilterParam(
		module->mod, band, STA_QUAL, module->tones.GQF[band][STA_QUAL]);

	return 0;
}

static int tones_freq_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->tones.GQF[band][STA_FREQ];

	return 0;
}

static int tones_freq_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);
	int val = ucontrol->value.integer.value[0];

	if (band < 0 || !module)
		return -ENOENT;

	module->tones.GQF[band][STA_FREQ] = val;

	if (module->tones.ON == STA_TONE_OFF)
		return 0;

	STA_SetFilterParam(
		module->mod, band, STA_FREQ, module->tones.GQF[band][STA_FREQ]);

	return 0;
}

static int tones_filter_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->tones.filter_type[band];

	return 0;
}

static int tones_filter_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int band = get_sta_channel(kcontrol);

	if (band < 0 || !module)
		return -ENOENT;

	module->tones.filter_type[band] = ucontrol->value.integer.value[0];
	STA_ToneSetTypes(module->mod,
			 module->tones.filter_type[0], module->tones.filter_type[2]);

	return 0;
}

static int tones_gain_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++)
		ucontrol->value.integer.value[i] = module->tones.GQF[i][STA_GAIN];

	return 0;
}

static int tones_gain_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++) {
		module->tones.GQF[i][STA_GAIN] = ucontrol->value.integer.value[i];
		if (module->tones.ON != STA_TONE_OFF)
			STA_SetFilterParam(
				module->mod, i, STA_GAIN, module->tones.GQF[i][STA_GAIN]);
	}

	return 0;
}

static int tones_qual_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++)
		ucontrol->value.integer.value[i] = module->tones.GQF[i][STA_QUAL];

	return 0;
}

static int tones_qual_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++) {
		module->tones.GQF[i][STA_QUAL] = ucontrol->value.integer.value[i];
		if (module->tones.ON != STA_TONE_OFF)
			STA_SetFilterParam(
				module->mod, i, STA_QUAL, module->tones.GQF[i][STA_QUAL]);
	}

	return 0;
}

static int tones_freq_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++)
		ucontrol->value.integer.value[i] = module->tones.GQF[i][STA_FREQ];

	return 0;
}

static int tones_freq_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++) {
		module->tones.GQF[i][STA_FREQ] = ucontrol->value.integer.value[i];
		if (module->tones.ON != STA_TONE_OFF)
			STA_SetFilterParam(
				module->mod, i, STA_FREQ, module->tones.GQF[i][STA_FREQ]);
	}

	return 0;
}

static int tones_filter_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->tones.num_bands; i++)
	    ucontrol->value.integer.value[i] = module->tones.filter_type[i];

	return 0;
}

static int tones_filter_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	if (ucontrol->value.integer.value[1] != STA_TONE_BANDBOOST_2)
		return -EINVAL;
	if (ucontrol->value.integer.value[2] > STA_TONE_BANDBOOST_2)
		return -EINVAL;
	module->tones.filter_type[0] = ucontrol->value.integer.value[0];
	module->tones.filter_type[2] = ucontrol->value.integer.value[2];
	STA_ToneSetTypes(module->mod,
			 module->tones.filter_type[0],
			 module->tones.filter_type[2]);

	return 0;
}

static const struct snd_kcontrol_new tones_controls[] = {
	SOC_ENUM_EXT(
		"Tones",
		switch_enum,
		tones_status_get, tones_status_put),
	STA_ENUM(
		"Tones Filters",
		tones_filter_enum,
		tones_filter_multi_get, tones_filter_multi_put, 3),
	STA_RANGE(
		"Tones Gains",
		tones_gain_multi_get, tones_gain_multi_put, 3,
		TONE_GAIN_MIN, TONE_GAIN_MAX),
	STA_RANGE(
		"Tones Quals",
		tones_qual_multi_get, tones_qual_multi_put, 3,
		TONE_FILT_QUAL_MIN, TONE_FILT_QUAL_MAX),
	STA_RANGE(
		"Tones Freqs",
		tones_freq_multi_get, tones_freq_multi_put, 3,
		TONE_FILT_FREQ_MIN, TONE_FILT_FREQ_MAX),
};

static struct snd_kcontrol_new tones_band_controls[] = {
	STA_MULTI_RANGE(
		"Tones",
		tones_gqf_get, tones_gqf_put, 3,
		TONE_GAIN_MIN, TONE_GAIN_MAX,
		TONE_FILT_QUAL_MIN, TONE_FILT_QUAL_MAX,
		TONE_FILT_FREQ_MIN, TONE_FILT_FREQ_MAX),
	STA_RANGE(
		"Tones Gain", tones_gain_get, tones_gain_put, 1,
		TONE_GAIN_MIN, TONE_GAIN_MAX),
	STA_RANGE(
		"Tones Qual", tones_qual_get, tones_qual_put, 1,
		TONE_FILT_QUAL_MIN, TONE_FILT_QUAL_MAX),
	STA_RANGE(
		"Tones Freq", tones_freq_get, tones_freq_put, 1,
		TONE_FILT_FREQ_MIN, TONE_FILT_FREQ_MAX),
};

static struct snd_kcontrol_new tones_filter_controls[] = {
	SOC_ENUM_EXT(
		"Tones Filter",
		tones_filter_enum,
		tones_filter_get, tones_filter_put),
};

static int tones_add_controls(struct sta_module *module,
				  struct snd_soc_component *component)
{
	snd_soc_add_component_controls(component,
			      tones_controls,
			      ARRAY_SIZE(tones_controls));
	sta_add_channel_controls(component, tones_band_controls,
					    ARRAY_SIZE(tones_band_controls),
					    0, "Bass", NULL);
	sta_add_channel_controls(component, tones_band_controls,
					    ARRAY_SIZE(tones_band_controls),
					    1, "Middle", NULL);
	sta_add_channel_controls(component, tones_band_controls,
					    ARRAY_SIZE(tones_band_controls),
					    2, "Treble", NULL);
	sta_add_channel_controls(component, tones_filter_controls,
					    ARRAY_SIZE(tones_filter_controls),
					    0, "Bass", NULL);
	/* cannot change the middle from STA_TONE_BANDBOOST_2, so skip the control */
	sta_add_channel_controls(component, tones_filter_controls,
					    ARRAY_SIZE(tones_filter_controls),
					    2, "Treble", NULL);

	return 0;
}

/*
 * GAIN: VOLUME AND MUTE
 */

static int mute_channel_get(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->gains.mute[ch];
	return 0;
}

static int mute_channel_put(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_value *ucontrol)
{
	int val = ucontrol->value.integer.value[0];
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	module->gains.mute[ch] = val;
	if (module->type != STA_GAIN_STATIC_NCH)
		STA_SmoothGainSetTC(module->mod, ch,
				    module->gains.mute_up,
				    module->gains.mute_down);

	if (val == 1)
		STA_GainSetGain(module->mod, ch, DB_GAIN_MUTE);
	else
		STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);

	return 0;
}

static int volume_channel_get(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = DB_TO_VAL(module->gains.vol[ch]);
	return 0;
}

static int volume_channel_put(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	int val = ucontrol->value.integer.value[0];
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	module->gains.vol[ch] = VAL_TO_DB(val);
	if (!module->gains.mute[ch]) {
		if (module->type != STA_GAIN_STATIC_NCH)
			STA_SmoothGainSetTC(module->mod, ch,
					    module->gains.time_up,
					    module->gains.time_down);
		STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
	}
	return 0;
}

static int volume_max_get(struct snd_kcontrol *kcontrol,
			  struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = STA_GainGetMaxGain(module->mod, ch);
	return 0;
}

static int volume_max_put(struct snd_kcontrol *kcontrol,
			  struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	STA_GainSetMaxGain(module->mod, ch, ucontrol->value.integer.value[0]);
	return 0;
}

static int volume_ch_min(struct sta_module *module, int ch)
{
	return GAIN_VOLUME_MIN;
}

static int volume_ch_max(struct sta_module *module, int ch)
{
	if (module->type == STA_GAIN_STATIC_NCH)
		return GAIN_VOLUME_MAX;

	return DB_TO_VAL(STA_GainGetMaxGain(module->mod, ch) * 10);
}

static int volume_min(struct sta_module *module)
{
	return GAIN_VOLUME_MIN;
}

static int volume_max(struct sta_module *module)
{
	int ch;
	u32 maxvol = 0;

	if (module->type == STA_GAIN_STATIC_NCH)
		return GAIN_VOLUME_MAX;

	for (ch = 0; ch < module->num_channels; ch++)
		maxvol = max(maxvol, STA_GainGetMaxGain(module->mod, ch));

	return DB_TO_VAL(maxvol * 10);
}

static int volume_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++)
		ucontrol->value.integer.value[ch] = DB_TO_VAL(module->gains.vol[ch]);

	return 0;
}

static int volume_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch, val;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++) {
		val = ucontrol->value.integer.value[ch];
		module->gains.vol[ch] = VAL_TO_DB(val);
		if (!module->gains.mute[ch]) {
			if (module->type != STA_GAIN_STATIC_NCH)
				STA_SmoothGainSetTC(module->mod, ch,
						    module->gains.time_up,
						    module->gains.time_down);
			STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
		}
	}

	return 0;
}

static int mute_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++)
		ucontrol->value.integer.value[ch] = module->gains.mute[ch];

	return 0;
}

static int mute_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch, val;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++) {
		if (module->type != STA_GAIN_STATIC_NCH)
			STA_SmoothGainSetTC(module->mod, ch,
					    module->gains.mute_up,
					    module->gains.mute_down);
		val = ucontrol->value.integer.value[ch];
		module->gains.mute[ch] = val;
		if (val == 1)
			STA_GainSetGain(module->mod, ch, DB_GAIN_MUTE);
		else
			STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
	}

	return 0;
}

static int mute_get(struct snd_kcontrol *kcontrol,
		    struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++) {
		ucontrol->value.integer.value[0] = module->gains.mute[ch];
		if (!module->gains.mute[ch])
			break;
	}

	return 0;
}

static int mute_put(struct snd_kcontrol *kcontrol,
		    struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int ch;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++) {
		if (module->type != STA_GAIN_STATIC_NCH)
			STA_SmoothGainSetTC(module->mod, ch,
					    module->gains.mute_up,
					    module->gains.mute_down);
		module->gains.mute[ch] = val;
	}

	if (val == 1) {
		STA_GainSetGains(module->mod, CH_ALL, DB_GAIN_MUTE);
	} else {
		for (ch = 0; ch < module->num_channels; ch++)
			STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
	}

	return 0;
}

static int volume_get(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = DB_TO_VAL(module->gains.vol[0]);
	return 0;
}

static int volume_put(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int val = ucontrol->value.integer.value[0];
	int ch;

	if (!module)
		return -ENOENT;
	for (ch = 0; ch < module->num_channels; ch++) {
		module->gains.vol[ch] = VAL_TO_DB(val);

		if (!module->gains.mute[ch]) {
			if (module->type != STA_GAIN_STATIC_NCH)
				STA_SmoothGainSetTC(module->mod, ch,
						    module->gains.time_up,
						    module->gains.time_down);
			STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
		}
	}

	return 0;
}

static int volume_tup_get(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->gains.time_up;
	return 0;
}

static int volume_tup_put(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->gains.time_up = ucontrol->value.integer.value[0];
	return 0;
}

static int volume_tdn_get(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->gains.time_down;
	return 0;
}

static int volume_tdn_put(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->gains.time_down = ucontrol->value.integer.value[0];
	return 0;
}

static int mute_tup_get(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->gains.mute_up;
	return 0;
}

static int mute_tup_put(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->gains.mute_up = ucontrol->value.integer.value[0];
	return 0;
}

static int mute_tdn_get(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->gains.mute_down;
	return 0;
}

static int mute_tdn_put(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->gains.mute_down = ucontrol->value.integer.value[0];
	return 0;
}

static int polarity_get(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	switch (module->type){
	case STA_GAIN_SMOOTH_NCH:
	case STA_GAIN_LINEAR_NCH:
		ucontrol->value.integer.value[0] =
		    (STA_GainGetPolarity(module->mod, ch) < 0);
		break;
	case STA_GAIN_STATIC_NCH:
		break;
	    }
	return 0;
}

static int polarity_put(struct snd_kcontrol *kcontrol,
		      struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);
	int pol = ucontrol->value.integer.value[0] ? -1 : 1;

	if (ch < 0 || !module)
		return -ENOENT;

	switch (module->type){
	case STA_GAIN_SMOOTH_NCH:
	case STA_GAIN_LINEAR_NCH:
                STA_GainSetPolarity(module->mod, ch, pol);
		break;
	case STA_GAIN_STATIC_NCH:
		break;
	}
	return 0;
}

static struct snd_kcontrol_new gain_controls[] = {
	STA_RANGE_EXT(
		"Volume Master", volume_get, volume_put, 1,
		volume_min, volume_max),
	SOC_ENUM_EXT(
		"Mute Master",
		switch_enum,
		mute_get, mute_put),
};

static struct snd_kcontrol_new gain_smooth_controls[] = {
	STA_RANGE(
		"Volume Ramp Up",
		volume_tup_get, volume_tup_put, 1,
		0, 10000),
	STA_RANGE(
		"Volume Ramp Down",
		volume_tdn_get, volume_tdn_put, 1,
		0, 10000),
	STA_RANGE(
		"Mute Ramp Up",
		mute_tup_get, mute_tup_put, 1,
		0, 10000),
	STA_RANGE(
		"Mute Ramp Down",
		mute_tdn_get, mute_tdn_put, 1,
		0, 10000),
};

static struct snd_kcontrol_new gain_multi_controls[] = {
	STA_RANGE_COUNT_EXT("Volume channels", volume_multi_get, volume_multi_put,
			    module_ch_num, volume_min, volume_max),
	STA_ENUM_EXT("Mute channels", switch_enum, mute_multi_get,
		     mute_multi_put, module_ch_num),
};

static struct snd_kcontrol_new gain_ch_controls[] = {
	STA_RANGE_CH_EXT(
		"Volume",
		volume_channel_get, volume_channel_put,
		volume_ch_min, volume_ch_max),
	SOC_ENUM_EXT(
		"Mute",
		switch_enum,
		mute_channel_get, mute_channel_put),
	SOC_ENUM_EXT(
		"Polarity", polarity_enum,
		polarity_get, polarity_put),
};
static struct snd_kcontrol_new gain_ch_smooth[] = {
	STA_RANGE(
		"Max",
		volume_max_get, volume_max_put, 1,
		0, 96),
};

static int gain_add_controls(struct sta_module *module,
			     struct snd_soc_component *component)
{
	int i;

	snd_soc_add_component_controls(component, gain_controls,
				   ARRAY_SIZE(gain_controls));
	snd_soc_add_component_controls(component, gain_multi_controls,
				   ARRAY_SIZE(gain_multi_controls));
	for (i = 0; i < module->num_ins; i++) {
		sta_add_channel_controls(component, gain_ch_controls,
					 ARRAY_SIZE(gain_ch_controls),
					 i, module->in_names[i], NULL);
		if (module->type != STA_GAIN_STATIC_NCH) {
			sta_add_channel_controls(component, gain_ch_smooth,
						 ARRAY_SIZE(gain_ch_smooth),
						 i, module->in_names[i], NULL);
		}
	}

	if (module->type != STA_GAIN_STATIC_NCH)
	    snd_soc_add_component_controls(component, gain_smooth_controls,
					   ARRAY_SIZE(gain_smooth_controls));

	return 0;
}

static int initialize_gainsmooth_time(struct sta_module *module)
{
	int ch;

	for (ch = 0; ch < module->num_channels; ch++)
		STA_SmoothGainSetTC(module->mod, ch,
				    module->gains.time_up,
				    module->gains.time_down);

	return 0;
}

static int initialize_gains(struct sta_module *module)
{
	int ch;

	if (module->early_init)
		return 0;

	for (ch = 0; ch < module->num_channels; ch++){
		module->gains.vol[ch] = DB_GAIN_INIT;
		STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
	}

	return 0;
}
/* Delay */
/* return the the delay of the first block */
static int delay_samples_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->delay.samples[0];

	return 0;
}
/* assign the same value to all the functions*/
static int delay_samples_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int j = 0;
	int val = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	for (j = 0; j < module->num_channels; j++) {
		module->delay.samples[j] = val;
		if (module->delay.ON)
			STA_DelaySetDelay(module->mod, j,
						module->delay.samples[j]);
	}
	return 0;
}

static int delay_samples_min(struct sta_module *module)
{
	return DELAY_SAMPLES_MIN;
}

static int delay_samples_max(struct sta_module *module)
{
	return module->delay.length;
}

/* retrieve the delay of each block */
static int delay_samples_multi_get(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->num_channels; i++)
		ucontrol->value.integer.value[i] = module->delay.samples[i];
	return 0;
}

static int delay_samples_multi_put(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;
	for (i = 0; i < module->num_channels; i++) {
		module->delay.samples[i] = ucontrol->value.integer.value[i];
		if (module->delay.ON)
			STA_DelaySetDelay(module->mod, i,
					  module->delay.samples[i]);
	}

	return 0;
}

static int delay_samples_ch_get(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->delay.samples[ch];
	return 0;
}

static int delay_samples_ch_put(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	module->delay.samples[ch] = ucontrol->value.integer.value[0];
	if (module->delay.ON)
		STA_DelaySetDelay(module->mod, ch, module->delay.samples[ch]);

	return 0;
}

static int delay_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->delay.ON;

	return 0;
}

static int delay_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int j = 0;
	int val = ucontrol->value.integer.value[0];

	if (!module)
		return -ENOENT;
	if (val == 0) {
		module->delay.ON = DELAY_OFF; /* all channels delay = 0*/
		for (j = 0; j < module->num_channels; j++)
			STA_DelaySetDelay(module->mod, j, 0);
	} else {
		if (val == 1) {
			module->delay.ON = DELAY_ON; /* Restore old value*/
			for (j = 0; j < module->num_channels; j++)
				STA_DelaySetDelay(module->mod, j,
						module->delay.samples[j]);
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

static struct snd_kcontrol_new delay_controls[] = {
	SOC_ENUM_EXT("Delay", switch_enum, delay_get, delay_put),
	STA_RANGE_EXT("Delay Samples", delay_samples_get, delay_samples_put, 1,
		      delay_samples_min, delay_samples_max),
};

static struct snd_kcontrol_new delay_multi_controls[] = {
	STA_RANGE_COUNT_EXT("Delay Samples channels", delay_samples_multi_get,
			    delay_samples_multi_put, module_ch_num,
			    delay_samples_min, delay_samples_max),
};

static struct snd_kcontrol_new delay_ch_controls[] = {
	STA_RANGE_EXT("Delay Samples", delay_samples_ch_get,
		      delay_samples_ch_put, 1, delay_samples_min,
		      delay_samples_max),
};

static int delay_add_controls(struct sta_module *module,
			      struct snd_soc_component *component)
{
	int i;

	snd_soc_add_component_controls(component, delay_controls,
				   ARRAY_SIZE(delay_controls));
	snd_soc_add_component_controls(component,
				   delay_multi_controls,
				   ARRAY_SIZE(delay_multi_controls));
	for (i = 0; i < module->num_ins; i++)
		sta_add_channel_controls(component,
					 delay_ch_controls,
					 ARRAY_SIZE(delay_ch_controls),
					 i, module->in_names[i], NULL);

	return 0;
}

/* MUX */
static int mux_multi_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;

	for (i = 0; i < module->num_out; i++)
		ucontrol->value.integer.value[i] = module->mux.sel[i];

	return 0;
}

static int mux_multi_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int i;

	if (!module)
		return -ENOENT;

	for (i = 0; i < module->num_out; i++)
		module->mux.sel[i] = ucontrol->value.integer.value[i];

	STA_MuxSet(module->mod, module->mux.sel);

	return 0;
}

static int mux_ch_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = module->mux.sel[ch];

	return 0;
}

static int mux_ch_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	module->mux.sel[ch] = ucontrol->value.integer.value[0];

	STA_MuxSetOutChannel(module->mod, module->mux.sel[ch], ch);

	return 0;
}

static int mux_num_out(struct sta_module *module)
{
	return module->mux.num_out;
}

static struct snd_kcontrol_new mux_multi_controls[] = {
	STA_COUNT_EXT("Mux", mux_multi_get, mux_multi_put,
			    mux_num_out, 0, 17),
};

static struct snd_kcontrol_new mux_ch_controls[] = {
	STA_RANGE("Mux", mux_ch_get, mux_ch_put, 1, 0, 17),
};

static int mux_add_controls(struct sta_module *module,
				  struct snd_soc_component *component)
{
	int i;

	snd_soc_add_component_controls(component,
				   mux_multi_controls,
				   ARRAY_SIZE(mux_multi_controls));
	for (i = 0; i < module->num_out; i++)
		sta_add_channel_controls(component,
					 mux_ch_controls,
					 ARRAY_SIZE(mux_ch_controls),
					 i, module->out_names[i], NULL);

	return 0;
}

static int initialize_mux(struct sta_module *module)
{
	STA_MuxSet(module->mod, module->mux.sel);

	return 0;
}

/* Peak detector */
static int peak_level_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	ucontrol->value.integer.value[0] = STA_PeakDetectorGetLevel(module->mod, ch, STA_PEAK_DB);

	return 0;
}

static int peak_level_reset(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);
	int ch = get_sta_channel(kcontrol);

	if (ch < 0 || !module)
		return -ENOENT;

	STA_PeakDetectorResetLevel(module->mod, ch);

	return 0;
}

static int peak_mode_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	ucontrol->value.integer.value[0] = module->peakdetect.nch;

	return 0;
}

static int peak_mode_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct sta_module *module = get_sta_module(kcontrol);

	if (!module)
		return -ENOENT;
	module->peakdetect.nch = ucontrol->value.integer.value[0];
	STA_SetMode(module->mod, module->peakdetect.nch);

	return 0;
}

static int peak_ch_min(struct sta_module *module)
{
	return 0;
}

static struct snd_kcontrol_new peakdetect_ch_controls[] = {
	STA_RANGE(
		"Level",
		peak_level_get, peak_level_reset, 1,
		-1400, 0),
};

static struct snd_kcontrol_new peakdetect_controls[] = {
	STA_RANGE_EXT("Channels", peak_mode_get, peak_mode_put, 1,
		      peak_ch_min, module_ch_num),
};

static int peak_add_controls(struct sta_module *module,
			     struct snd_soc_component *component)
{
    	int i;

	snd_soc_add_component_controls(component,
				   peakdetect_controls,
				   ARRAY_SIZE(peakdetect_controls));

	for (i = 0; i < module->num_ins; i++)
		sta_add_channel_controls(component, peakdetect_ch_controls,
					 ARRAY_SIZE(peakdetect_ch_controls),
					 i, module->in_names[i], NULL);

	return 0;
}

/* Tuning */
static int tuning_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	static int i;
	u32 cmd_id, byte_size;
	u8 cs = 0;
	u8 *packet;
	u16 len;

	for (i = 0; i < TUNING_MAX_LENGHT; i++)
		tuning.cmd[i] = (u16) ucontrol->value.integer.value[i];
	/* Compute checksum */
	len = tuning.cmd[0] + 2;
	packet = (u8 *) &(tuning.cmd[0]);
	if (len > 2 * TUNING_MAX_LENGHT - 2) {
		printk(KERN_ERR "Wrong tuning command len=%d", len);
		return -EINVAL;
	}
	while (len--)
		cs ^= *packet++;

	len = tuning.cmd[0] + 2;
	if (cs != tuning.cmd[len / 2]) {
		printk(KERN_ERR "Checksum error %d vs %d", cs, tuning.cmd[len / 2]);
		return -EINVAL;
	}
	cmd_id = tuning.cmd[1] & 0xFFFF;
	STA_ParserCmd(cmd_id, (void *) &(tuning.cmd[4]), &byte_size);
	for (i = 0; i < TUNING_MAX_LENGHT; i++)
		tuning.cmd[i] = 0;

	return 0;
}

static int tuning_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int i;

	for (i = 0; i < TUNING_MAX_LENGHT; i++)
		ucontrol->value.integer.value[i] = tuning.cmd[i];

	return 0;
}

static struct snd_kcontrol_new tuning_controls[] = {
	STA_RANGE("Tuning Cmd", tuning_get, tuning_put, TUNING_MAX_LENGHT,
		0, 65535),
};

static ssize_t peak_get(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct sta_module *peak;
	struct sta_connection *conn;
	int ch;
	const char *peak_name;
	char *p = buf;

	peak_name = attr->attr.name;
	peak = get_sta_module_by_name(peak_name);
	if (!peak)
    		return -EINVAL;
	
	for (ch = 0; ch < peak->num_channels; ch++) {
	    conn = get_sta_conn_to(peak->mod, ch);
	    if (!conn)
		    return -EINVAL;
    
	    p += sprintf(p, "%s %d %d\n", conn->from_name, conn->from_ch, ch);
	}
	p += sprintf(p, "\n");
	return p - buf;
}

static ssize_t
peak_set(struct device *dev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	STAModule from;
	struct sta_module *peak;
	struct sta_connection *conn;
	int ret, peak_ch, from_ch;
	const char *peak_name;
	char from_name[MAX_MODULE_NAME_LEN];

	peak_name = attr->attr.name;
	ret = sscanf(buf, "%s %d %d", from_name, &from_ch, &peak_ch);
	peak = get_sta_module_by_name(peak_name);
	if (!peak)
    		return -EINVAL;
	conn = get_sta_conn_to(peak->mod, peak_ch);
	if (!conn)
		return -EINVAL;
	from = get_dsp_component(dev, from_name);
	if (from == -EINVAL)
		return -EINVAL;

	STA_ReconnectFrom(conn->from_mod, conn->from_ch,
			  conn->to_mod, conn->to_ch,
			  from, from_ch);
	if (STA_GetError())
		return -EINVAL;

	strlcpy(conn->from_name, from_name, MAX_MODULE_NAME_LEN);
	conn->from_mod = from;
	conn->from_ch = from_ch;
	return count;
}

static DEVICE_ATTR(peak, 0644, peak_get, peak_set);

static int initialize_pre(void)
{
	struct sta_module *module;

	list_for_each_entry(module, &module_list, list) {
		switch (module->type) {
		case STA_EQUA_STATIC_1BAND_DP:
		case STA_EQUA_STATIC_2BANDS_DP:
		case STA_EQUA_STATIC_3BANDS_DP:
		case STA_EQUA_STATIC_4BANDS_DP:
		case STA_EQUA_STATIC_5BANDS_DP:
		case STA_EQUA_STATIC_6BANDS_DP:
		case STA_EQUA_STATIC_7BANDS_DP:
		case STA_EQUA_STATIC_8BANDS_DP:
		case STA_EQUA_STATIC_9BANDS_DP:
		case STA_EQUA_STATIC_10BANDS_DP:
		case STA_EQUA_STATIC_11BANDS_DP:
		case STA_EQUA_STATIC_12BANDS_DP:
		case STA_EQUA_STATIC_13BANDS_DP:
		case STA_EQUA_STATIC_14BANDS_DP:
		case STA_EQUA_STATIC_15BANDS_DP:
		case STA_EQUA_STATIC_16BANDS_DP:
		case STA_EQUA_STATIC_1BAND_SP:
		case STA_EQUA_STATIC_2BANDS_SP:
		case STA_EQUA_STATIC_3BANDS_SP:
		case STA_EQUA_STATIC_4BANDS_SP:
		case STA_EQUA_STATIC_5BANDS_SP:
		case STA_EQUA_STATIC_6BANDS_SP:
		case STA_EQUA_STATIC_7BANDS_SP:
		case STA_EQUA_STATIC_8BANDS_SP:
		case STA_EQUA_STATIC_9BANDS_SP:
		case STA_EQUA_STATIC_10BANDS_SP:
		case STA_EQUA_STATIC_11BANDS_SP:
		case STA_EQUA_STATIC_12BANDS_SP:
		case STA_EQUA_STATIC_13BANDS_SP:
		case STA_EQUA_STATIC_14BANDS_SP:
		case STA_EQUA_STATIC_15BANDS_SP:
		case STA_EQUA_STATIC_16BANDS_SP:
			initialize_EQ(module);
			break;
		case STA_GAIN_LINEAR_NCH:
		case STA_GAIN_SMOOTH_NCH:
			initialize_gainsmooth_time(module);
			initialize_gains(module);
			break;
		case STA_MUX_2OUT:
		case STA_MUX_4OUT:
		case STA_MUX_6OUT:
		case STA_MUX_8OUT:
		case STA_MUX_10OUT:
			initialize_mux(module);
			break;
		case STA_PCMCHIME_12BIT_Y_2CH:
		case STA_PCMCHIME_12BIT_X_2CH:
			STA_PCMSetMaxDataSize(module->mod, module->pcmchime.bytesize);
			break;
		case STA_PEAKDETECT_NCH:
			module->peakdetect.nch = 0;
			break;
		}
	}

	return 0;
}

static int initialize_post(void)
{
	struct sta_module *module;
	int out;

	list_for_each_entry(module, &module_list, list) {
		switch (module->type) {
		case STA_LOUD_STATIC_DP:
		case STA_LOUD_STATIC_STEREO_DP:
			initialize_loudn(module);
			break;
		case STA_TONE_STATIC_DP:
		case STA_TONE_STATIC_STEREO_DP:
			initialize_tones(module);
			break;
		case STA_GAIN_STATIC_NCH:
			initialize_gains(module);
			break;
		case STA_MIXE_2INS_NCH:
		case STA_MIXE_3INS_NCH:
		case STA_MIXE_4INS_NCH:
		case STA_MIXE_5INS_NCH:
		case STA_MIXE_6INS_NCH:
		case STA_MIXE_7INS_NCH:
		case STA_MIXE_8INS_NCH:
		case STA_MIXE_9INS_NCH:
			for (out = 0; out < module->num_channels; out++)
				STA_MixerSetChannelInGains(module->mod, out,
							   module->mixer.vol[out]);

			break;
		case STA_BITSHIFTER_NCH:
			STA_BitShifterSetLeftShifts(module->mod,
						    CH_ALL,
						    module->bitshifter.shift);
			break;
		case STA_LMTR_1CH:
		case STA_LMTR_2CH:
		case STA_LMTR_4CH:
		case STA_LMTR_6CH:
				if (module->limiter.mask)
					STA_LimiterSetParams(
						module->mod,
						module->limiter.mask,
						module->limiter.params);
			break;
		case STA_CLIPLMTR_NCH:
			if (module->clip_limiter.mask)
				STA_ClipLimiterSetParams(
					module->mod,
					module->clip_limiter.mask,
					module->clip_limiter.params);
			break;
		case STA_COMP_1CH:
		case STA_COMP_2CH:
		case STA_COMP_4CH:
		case STA_COMP_6CH:
			if (module->compander.mask)
				STA_CompanderSetParams(
					module->mod,
					module->compander.mask,
					module->compander.params);
			break;
		case STA_PCMCHIME_12BIT_Y_2CH:
		case STA_PCMCHIME_12BIT_X_2CH:
			break;

		}
	}
	return 0;
}

static int st_snd_soc_component_probe(struct snd_soc_component *component)
{
	struct sta_module *module;

	list_for_each_entry(module, &module_list, list) {
		component->name_prefix = module->prefix;
		module->first_numid = component->card->snd_card->last_numid + 1;
		module->component = component;
		switch (module->type) {
		case STA_GAIN_LINEAR_NCH:
		case STA_GAIN_STATIC_NCH:
		case STA_GAIN_SMOOTH_NCH:
			gain_add_controls(module, component);
			break;
		case STA_TONE_STATIC_DP:
		case STA_TONE_STATIC_STEREO_DP:
			tones_add_controls(module, component);
			break;
		case STA_LOUD_STATIC_DP:
		case STA_LOUD_STATIC_STEREO_DP:
			loudness_add_controls(module, component);
			break;
		case STA_MIXE_2INS_NCH:
		case STA_MIXE_3INS_NCH:
		case STA_MIXE_4INS_NCH:
		case STA_MIXE_5INS_NCH:
		case STA_MIXE_6INS_NCH:
		case STA_MIXE_7INS_NCH:
		case STA_MIXE_8INS_NCH:
		case STA_MIXE_9INS_NCH:
			mixer_add_controls(module, component);
			break;
		case STA_EQUA_STATIC_1BAND_DP:
		case STA_EQUA_STATIC_2BANDS_DP:
		case STA_EQUA_STATIC_3BANDS_DP:
		case STA_EQUA_STATIC_4BANDS_DP:
		case STA_EQUA_STATIC_5BANDS_DP:
		case STA_EQUA_STATIC_6BANDS_DP:
		case STA_EQUA_STATIC_7BANDS_DP:
		case STA_EQUA_STATIC_8BANDS_DP:
		case STA_EQUA_STATIC_9BANDS_DP:
		case STA_EQUA_STATIC_10BANDS_DP:
		case STA_EQUA_STATIC_11BANDS_DP:
		case STA_EQUA_STATIC_12BANDS_DP:
		case STA_EQUA_STATIC_13BANDS_DP:
		case STA_EQUA_STATIC_14BANDS_DP:
		case STA_EQUA_STATIC_15BANDS_DP:
		case STA_EQUA_STATIC_16BANDS_DP:
		case STA_EQUA_STATIC_1BAND_SP:
		case STA_EQUA_STATIC_2BANDS_SP:
		case STA_EQUA_STATIC_3BANDS_SP:
		case STA_EQUA_STATIC_4BANDS_SP:
		case STA_EQUA_STATIC_5BANDS_SP:
		case STA_EQUA_STATIC_6BANDS_SP:
		case STA_EQUA_STATIC_7BANDS_SP:
		case STA_EQUA_STATIC_8BANDS_SP:
		case STA_EQUA_STATIC_9BANDS_SP:
		case STA_EQUA_STATIC_10BANDS_SP:
		case STA_EQUA_STATIC_11BANDS_SP:
		case STA_EQUA_STATIC_12BANDS_SP:
		case STA_EQUA_STATIC_13BANDS_SP:
		case STA_EQUA_STATIC_14BANDS_SP:
		case STA_EQUA_STATIC_15BANDS_SP:
		case STA_EQUA_STATIC_16BANDS_SP:
			equalizer_add_controls(module, component);
			break;
		case STA_MUX_2OUT:
		case STA_MUX_4OUT:
		case STA_MUX_6OUT:
		case STA_MUX_8OUT:
		case STA_MUX_10OUT:
			mux_add_controls(module, component);
			break;
		case STA_LMTR_1CH:
		case STA_LMTR_2CH:
		case STA_LMTR_4CH:
		case STA_LMTR_6CH:
			snd_soc_add_component_controls(component,
				  st_codec_dsp_limiter_controls,
				  ARRAY_SIZE(st_codec_dsp_limiter_controls));
			break;
		case STA_CLIPLMTR_NCH:
			snd_soc_add_component_controls(component,
						   clip_limiter_controls,
					ARRAY_SIZE(clip_limiter_controls));
			break;
		case STA_COMP_1CH:
		case STA_COMP_2CH:
		case STA_COMP_4CH:
		case STA_COMP_6CH:
			snd_soc_add_component_controls(component,
						   compander_controls,
					ARRAY_SIZE(compander_controls));
			break;
		case STA_SINE_2CH:
			snd_soc_add_component_controls(component,
				  st_codec_dsp_beep_controls,
				  ARRAY_SIZE(st_codec_dsp_beep_controls));
			break;
		case STA_PCMCHIME_12BIT_Y_2CH:
		case STA_PCMCHIME_12BIT_X_2CH:
			snd_soc_add_component_controls(component,
				  st_codec_dsp_pcmchime_controls,
				  ARRAY_SIZE(st_codec_dsp_pcmchime_controls));
			break;
		case STA_CHIMEGEN:
			snd_soc_add_component_controls(component,
				  st_codec_dsp_chime_controls,
				  ARRAY_SIZE(st_codec_dsp_chime_controls));
			snd_soc_add_component_controls(component,
				  st_codec_dsp_chime_ramp_controls,
				  ARRAY_SIZE(st_codec_dsp_chime_ramp_controls));
			break;
		case STA_DLAY_Y_2CH:
		case STA_DLAY_X_2CH:
		case STA_DLAY_Y_4CH:
		case STA_DLAY_X_4CH:
		case STA_DLAY_Y_6CH:
		case STA_DLAY_X_6CH:
			delay_add_controls(module, component);
			break;
		case STA_PEAKDETECT_NCH:
			peak_add_controls(module, component);
    			break;
		}
		module->last_numid = component->card->snd_card->last_numid;
	}

	component->name_prefix = NULL;
	snd_soc_add_component_controls(component, tuning_controls,
				   ARRAY_SIZE(tuning_controls));

	return 0;
}

static struct snd_soc_component_driver st_snd_soc_component[] = {
    {
	.probe = st_snd_soc_component_probe,
    },
    {},
    {},
};

static struct snd_soc_dai_ops st_codec_dsp_dai_ops = {
};

#define ST_CODEC_MSP_MIN_CHANNELS 1
#define ST_CODEC_MSP_MAX_CHANNELS 8
#define ST_CODEC_MSP_RATES	(SNDRV_PCM_RATE_8000_48000 \
			       | SNDRV_PCM_RATE_CONTINUOUS)
#define ST_CODEC_MSP_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE \
			       | SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver st_codec_dsp_dai[] = {
	{
		.id = 3,
		.name = "st-codec-dsp",
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
		.ops = &st_codec_dsp_dai_ops,
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
		.ops = &st_codec_dsp_dai_ops,
		.symmetric_rates = 1,
	},
};

static const struct of_device_id st_codec_dsp_dt_ids[] = {
	{ .compatible = "st,audioeffects", },
	{ }
};
MODULE_DEVICE_TABLE(of, st_codec_dsp_dt_ids);

static int st_codec_dsp_dts_lut_match(
	struct dts_lut_val *lut_check,
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
		dsp_codec_drv->dev, "%s:%s DSP property wrong LUT entry",
		__func__, str_input);

	return -EINVAL;
}

static STA_ModuleType st_codec_dsp_dts_mdtypelut_match(struct STA_ModuleType_dts_lut *lut_check,
						int lut_length,
						const char *str_input)
{
	int i;

	for (i = 0; i < lut_length; i++) {
		if (strcmp(lut_check[i].name, str_input) == 0)
			return lut_check[i].value;
	}

	dev_err(
		NULL,
		"%s:%s wrong LUT entry",
		__func__,
		str_input);

	return -EINVAL;
}

static int st_codec_dsp_of_bitshifter(struct platform_device *pdev,
					struct device_node *np,
					struct sta_module *module)
{
	const char *str_shift;
	int ret = -EINVAL;

	module->bitshifter.shift = BITSHIFTER_INIT;

	ret = of_property_read_string(np, "left-shift", &str_shift);
	if (!ret) {
		ret = kstrtos32(str_shift, 10, &module->bitshifter.shift);
		if (ret < 0)
			return ret;
	}

	ret = of_property_read_string(np, "right-shift", &str_shift);
	if (!ret) {
		ret = kstrtos32(str_shift, 10, &module->bitshifter.shift);
		if (ret < 0)
			return ret;
		module->bitshifter.shift = 0 - module->bitshifter.shift;
	}

	return 0;
}

static int st_codec_dsp_of_pcmchime(struct platform_device *pdev,
				    struct device_node *np,
				    struct sta_module *module)
{
	const char *str;
	int ret = -EINVAL;

	ret = of_property_read_string(np, "file", &str);
	if (!ret)
		module->pcmchime.file = str;

	module->pcmchime.play_count = 1;
	of_property_read_s32(np, "count", &module->pcmchime.play_count);

	module->pcmchime.bytesize = 4096;
	of_property_read_u32(np, "max-size", &module->pcmchime.bytesize);
	of_property_read_u32(np, "left-delay", &module->pcmchime.left_delay);
	of_property_read_u32(np, "left-post-delay",
			     &module->pcmchime.left_post_delay);
	of_property_read_u32(np, "right-delay", &module->pcmchime.right_delay);
	of_property_read_u32(np, "right-post-delay",
			     &module->pcmchime.right_post_delay);

	return ret;
}

static int st_codec_dsp_of_chimegen(struct platform_device *pdev,
					struct device_node *np,
					struct sta_module *module)
{
	struct device_node *ncca;
	const char *str;
	u32 val;
	int i = 0;
	int ret = -EINVAL;
	struct sta_module *master;

	module->chimegen.params.repeatCount = CHIME_REPEAT_COUNT_INIT;
	ret = of_property_read_u32(np, "repeat-count", &val);
	if (!ret)
		module->chimegen.params.repeatCount = val;

	module->chimegen.params.postRepeatRampIdx = CHIME_POST_REPEAT_ID_INIT;
	ret = of_property_read_u32(np, "post-repeat-ramp-idx", &val);
	if (!ret)
		module->chimegen.params.postRepeatRampIdx = val;

	module->chimegen.params.numRamps = (u16) of_get_child_count(np);
	if (module->chimegen.params.numRamps == 0) {
		dev_err(&pdev->dev,
		        "%s: ERROR: no ramps defined\n",
			__func__);
		return -EINVAL;
	}
	STA_ChimeGenSetMaxNumRamps(module->mod, module->chimegen.params.numRamps);

	ret = of_property_read_string(np, "master", &str);
	if (!ret) {
		master = get_sta_module_by_name(str);
		if (master) 
			module->chimegen.master = master;
	}

	module->chimegen.params.ramps = devm_kcalloc(
		&pdev->dev,
		module->chimegen.params.numRamps,
		sizeof(STA_RampParams),
		GFP_KERNEL);
	if (module->chimegen.params.ramps == NULL) {
		dev_err(&pdev->dev,
			"%s: ERROR: Failed to init chimegen!\n",
			__func__);
		return -ENOMEM;
	}

	for_each_child_of_node(np, ncca) {
		module->chimegen.params.ramps[i].type = STA_RAMP_LIN;
		ret = of_property_read_string(ncca, "type", &str);
		if (!ret) {
			ret = st_codec_dsp_dts_lut_match(
				ramp_type_dts_lut,
				ARRAY_SIZE(ramp_type_dts_lut),
				str,
				&val);
			if (ret)
    				return -EINVAL;
			module->chimegen.params.ramps[i].type = val;
		}

		ret = of_property_read_u32(ncca, "amplitude", &val);
		if (ret) {
			dev_err(&pdev->dev,
				"%s: ERROR: missing ramp %d amplitude!\n",
				    __func__, i);
			return -EINVAL;
		}
		module->chimegen.params.ramps[i].ampl = val;

		ret = of_property_read_u32(ncca, "duration", &val);
		if (ret) {
			dev_err(&pdev->dev,
				"%s: ERROR: missing ramp %d duration!\n",
				    __func__, i);
			return -EINVAL;
		}
		module->chimegen.params.ramps[i].msec = val;

		ret = of_property_read_u32(ncca, "frequency", &val);
		if (ret) {
			dev_err(&pdev->dev,
				"%s: ERROR: missing ramp %d frequency!\n",
				    __func__, i);
			return -EINVAL;
		}
		module->chimegen.params.ramps[i].freq = val;

		i++;
	}

	return 0;
}

static int st_codec_dsp_of_sinegen(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	const char *str_freq, *str_dur, *str_gain;
	int ret = -EINVAL;
	s16 pos_gain;

	module->sinegen.sine_freq = SINE_FREQ_INIT;
	ret = of_property_read_string(np, "frequency", &str_freq);
	if (!ret) {
		ret = kstrtou32(str_freq, 10, &module->sinegen.sine_freq);
		if (ret < 0)
			return ret;
	}

	module->sinegen.sine_duration = SINE_DURATION_INIT;
	ret = of_property_read_string(np, "duration", &str_dur);
	if (!ret) {
		ret = kstrtou32(str_dur, 10, &module->sinegen.sine_duration);
		if (ret < 0)
			return ret;
	}

	module->sinegen.sine_gain = 0 - SINE_GAIN_INIT;
	ret = of_property_read_string(np, "gain", &str_gain);
	if (!ret) {
		ret = kstrtos16(str_gain, 10, &pos_gain);
		if (ret < 0)
			return ret;
		module->sinegen.sine_gain = 0 - pos_gain;
	}

	return 0;
}

static int st_codec_dsp_of_limiter(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	int ret;
	s32 val;

	ret = of_property_read_s32(np, "threshold", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_THRES] = val;
		module->limiter.mask |= STA_LMTR_THRES;
	}
	ret = of_property_read_u32(np, "attenuation", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_ATTENUATION] = val;
		module->limiter.mask |= STA_LMTR_ATTENUATION;
	}
	ret = of_property_read_u32(np, "attack-time", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_ATTACK_TIME] = val;
		module->limiter.mask |= STA_LMTR_ATTACK_TIME;
	}
	ret = of_property_read_u32(np, "release-time", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_RELEASE_TIME] = val;
		module->limiter.mask |= STA_LMTR_RELEASE_TIME;
	}
	ret = of_property_read_u32(np, "hysteresis", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_HYSTERESIS] = val;
		module->limiter.mask |= STA_LMTR_HYSTERESIS;
	}
	ret = of_property_read_u32(np, "peak-attack-time", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_PEAK_ATK_TIME] = val;
		module->limiter.mask |= STA_LMTR_PEAK_ATK_TIME;
	}
	ret = of_property_read_u32(np, "peak-release-time", &val);
	if (ret == 0) {
		module->limiter.params[STA_LMTR_IDX_PEAK_REL_TIME] = val;
		module->limiter.mask |= STA_LMTR_PEAK_REL_TIME;
	}

	return 0;
}

static int st_codec_dsp_of_clip_limiter(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	int ret, i, cnt;
	u32 val;

	cnt = of_property_count_u32_elems(np, "dco-ch");
	if (cnt > 6) {
		dev_err(&pdev->dev, "dco-ch > 6!\n");
		return -EINVAL;
	}
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_u32_index(np, "dco-ch", i, &val);
		if (ret < 0)
			return ret;
		module->clip_limiter.params[STA_CLIPLMTR_IDX_CLIP_BITMASK] |= 1 << (18 + val);
		module->clip_limiter.mask |= STA_CLIPLMTR_CLIP_BITMASK;
	}

	ret = of_property_read_u32(np, "polarity", &val);
	if (ret == 0) {
		module->clip_limiter.params[STA_CLIPLMTR_IDX_CLIP_POLARITY] = val;
		module->clip_limiter.mask |= STA_CLIPLMTR_CLIP_POLARITY;
	}

	ret = of_property_read_u32(np, "max-attenuation", &val);
	if (ret == 0) {
		module->clip_limiter.params[STA_CLIPLMTR_IDX_MAX_ATTENUATION] = val;
		module->clip_limiter.mask |= STA_CLIPLMTR_MAX_ATTENUATION;
	}

	ret = of_property_read_u32(np, "min-attenuation", &val);
	if (ret == 0) {
		module->clip_limiter.params[STA_CLIPLMTR_IDX_MIN_ATTENUATION] = val;
		module->clip_limiter.mask |= STA_CLIPLMTR_MIN_ATTENUATION;
	}

	ret = of_property_read_u32(np, "attack-time", &val);
	if (ret == 0) {
		module->clip_limiter.params[STA_CLIPLMTR_IDX_ATTACK_TIME] = val;
		module->clip_limiter.mask |= STA_CLIPLMTR_ATTACK_TIME;
	}

	ret = of_property_read_u32(np, "release-time", &val);
	if (ret == 0) {
		module->clip_limiter.params[STA_CLIPLMTR_IDX_RELEASE_TIME] = val;
		module->clip_limiter.mask |= STA_CLIPLMTR_RELEASE_TIME;
	}

	ret = of_property_read_u32(np, "adj-attenuation", &val);
	if (ret == 0) {
		module->clip_limiter.params[STA_CLIPLMTR_IDX_ADJ_ATTENUATION] = val;
		module->clip_limiter.mask |= STA_CLIPLMTR_ADJ_ATTENUATION;
	}

	return 0;
}

static int st_codec_dsp_of_compander(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	int ret;
	s32 val;

	ret = of_property_read_s32(np, "mean-max", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_MEAN_MAX_MODE] = val;
		module->compander.mask |= STA_COMP_MEAN_MAX_MODE;
	}

	val = COMPANDER_AVG_FACTOR_INIT;
	of_property_read_s32(np, "avg-factor", &val);
	module->compander.params[STA_COMP_IDX_AVG_FACTOR] = val;
	module->compander.mask |= STA_COMP_AVG_FACTOR;

	ret = of_property_read_s32(np, "attenuation", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_ATTENUATION] = val;
		module->compander.mask |= STA_COMP_ATTENUATION;
	}

	ret = of_property_read_s32(np, "cpr-threshold", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_CPR_THRES] = val;
		module->compander.mask |= STA_COMP_CPR_THRES;
	}

	ret = of_property_read_s32(np, "cpr-slope", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_CPR_SLOPE] = val;
		module->compander.mask |= STA_COMP_CPR_SLOPE;
	}

	ret = of_property_read_s32(np, "cpr-hysteresis", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_CPR_HYSTERESIS] = val;
		module->compander.mask |= STA_COMP_CPR_HYSTERESIS;
	}

	ret = of_property_read_s32(np, "cpr-attack-time", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_CPR_ATK_TIME] = val;
		module->compander.mask |= STA_COMP_CPR_ATK_TIME;
	}

	ret = of_property_read_s32(np, "cpr-release-time", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_CPR_REL_TIME] = val;
		module->compander.mask |= STA_COMP_CPR_REL_TIME;
	}

	ret = of_property_read_s32(np, "exp-threshold", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_EXP_THRES] = val;
		module->compander.mask |= STA_COMP_EXP_THRES;
	}

	ret = of_property_read_s32(np, "exp-slope", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_EXP_SLOPE] = val;
		module->compander.mask |= STA_COMP_EXP_SLOPE;
	}

	ret = of_property_read_s32(np, "exp-hysteresis", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_EXP_HYSTERESIS] = val;
		module->compander.mask |= STA_COMP_EXP_HYSTERESIS;
	}

	ret = of_property_read_s32(np, "exp-attack-time", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_EXP_ATK_TIME] = val;
		module->compander.mask |= STA_COMP_EXP_ATK_TIME;
	}

	ret = of_property_read_s32(np, "exp-release-time", &val);
	if (ret == 0) {
		module->compander.params[STA_COMP_IDX_EXP_REL_TIME] = val;
		module->compander.mask |= STA_COMP_EXP_REL_TIME;
	}

	return 0;
}

static int st_codec_dsp_of_equalizer(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module,
	int mod_type)
{
	const char *str;
	int i, j, num_freq, num_qual, num_presets, num_coefs, ret;
	const char **preset_names;

	switch (mod_type) {
	case STA_EQUA_STATIC_1BAND_DP:
	case STA_EQUA_STATIC_1BAND_SP:
		module->equalizer.num_bands = 1;
		break;
	case STA_EQUA_STATIC_2BANDS_DP:
	case STA_EQUA_STATIC_2BANDS_SP:
		module->equalizer.num_bands = 2;
		break;
	case STA_EQUA_STATIC_3BANDS_DP:
	case STA_EQUA_STATIC_3BANDS_SP:
		module->equalizer.num_bands = 3;
		break;
	case STA_EQUA_STATIC_4BANDS_DP:
	case STA_EQUA_STATIC_4BANDS_SP:
		module->equalizer.num_bands = 4;
		break;
	case STA_EQUA_STATIC_5BANDS_DP:
	case STA_EQUA_STATIC_5BANDS_SP:
		module->equalizer.num_bands = 5;
		break;
	case STA_EQUA_STATIC_6BANDS_DP:
	case STA_EQUA_STATIC_6BANDS_SP:
		module->equalizer.num_bands = 6;
		break;
	case STA_EQUA_STATIC_7BANDS_DP:
	case STA_EQUA_STATIC_7BANDS_SP:
		module->equalizer.num_bands = 7;
		break;
	case STA_EQUA_STATIC_8BANDS_DP:
	case STA_EQUA_STATIC_8BANDS_SP:
		module->equalizer.num_bands = 8;
		break;
	case STA_EQUA_STATIC_9BANDS_DP:
	case STA_EQUA_STATIC_9BANDS_SP:
		module->equalizer.num_bands = 9;
		break;
	case STA_EQUA_STATIC_10BANDS_DP:
	case STA_EQUA_STATIC_10BANDS_SP:
		module->equalizer.num_bands = 10;
		break;
	case STA_EQUA_STATIC_11BANDS_DP:
	case STA_EQUA_STATIC_11BANDS_SP:
		module->equalizer.num_bands = 11;
		break;
	case STA_EQUA_STATIC_12BANDS_DP:
	case STA_EQUA_STATIC_12BANDS_SP:
		module->equalizer.num_bands = 12;
		break;
	case STA_EQUA_STATIC_13BANDS_DP:
	case STA_EQUA_STATIC_13BANDS_SP:
		module->equalizer.num_bands = 13;
		break;
	case STA_EQUA_STATIC_14BANDS_DP:
	case STA_EQUA_STATIC_14BANDS_SP:
		module->equalizer.num_bands = 14;
		break;
	case STA_EQUA_STATIC_15BANDS_DP:
	case STA_EQUA_STATIC_15BANDS_SP:
		module->equalizer.num_bands = 15;
		break;
	case STA_EQUA_STATIC_16BANDS_DP:
	case STA_EQUA_STATIC_16BANDS_SP:
		module->equalizer.num_bands = 16;
		break;
	default:
	break;
	}

	num_coefs = of_property_count_strings(np, "coefs");
	if (num_coefs > 0) {
		module->equalizer.coefs = devm_kcalloc(
			&pdev->dev, num_coefs, sizeof(s32), GFP_KERNEL);
		if (module->equalizer.coefs == NULL) {
			dev_err(
				&pdev->dev,
				"%s: ERROR: Failed to alloc coefs!\n",
				__func__);
			return -ENOMEM;
		}
		for (i = 0; i < num_coefs; i++)	{
			of_property_read_string_index(
					np, "coefs", i, &str);
			ret = kstrtos32(str, 10, &module->equalizer.coefs[i]);
			if (ret < 0)
				return ret;
		}
		return 0;
	}


	module->equalizer.bandGQF = devm_kcalloc(
		&pdev->dev,
		module->equalizer.num_bands,
		sizeof(s16 *),
		GFP_KERNEL);
	if (module->equalizer.bandGQF == NULL) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to init bandGQF!\n",
			__func__);
		return -ENOMEM;
	}

	for (i = 0; i < module->equalizer.num_bands; i++)	{
		module->equalizer.bandGQF[i] = devm_kcalloc(
			&pdev->dev,
			3,
			sizeof(s16),
			GFP_KERNEL);
	}

	num_qual = of_property_count_strings(np, "quality");
	if (num_qual != module->equalizer.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid quality length!\n");
		return -EINVAL;
	}

	for (i = 0; i < num_qual; i++) {
		of_property_read_string_index(
			np, "quality", i, &str);
		ret = kstrtos16(str, 10, &module->equalizer.bandGQF[i][STA_QUAL]);
		if (ret < 0)
			return ret;
	}

	num_freq = of_property_count_strings(np, "frequency");
	if (num_freq != module->equalizer.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid frequency length!\n");
		return -EINVAL;
	}

	for (i = 0; i < num_freq; i++) {
		of_property_read_string_index(
			np, "frequency", i, &str);
		ret = kstrtos16(str, 10, &module->equalizer.bandGQF[i][STA_FREQ]);
		if (ret < 0)
			return ret;
	}

	num_presets = of_property_count_strings(np, "presets");
	if (!num_presets) {
		dev_err(
			&pdev->dev,
			"Invalid presets length!\n");
		return -EINVAL;
	}

	module->equalizer.preset = devm_kcalloc(
		&pdev->dev,
		num_presets,
		sizeof(struct preset),
		GFP_KERNEL);
	if (module->equalizer.preset == NULL) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to init eq preset!\n",
			__func__);
		return -ENOMEM;
	}

	preset_names = devm_kcalloc(&pdev->dev,
					num_presets,
					sizeof(char *),
					GFP_KERNEL);
	if (preset_names == NULL) {
		dev_err(&pdev->dev,
			"%s: ERROR: Failed to init eq preset_enum!\n",
			__func__);
		return -ENOMEM;
	}
	module->equalizer.preset_enum.texts = preset_names;
	module->equalizer.preset_enum.items = num_presets;

	for (i = 0; i < num_presets; i++) {
		of_property_read_string_index(
			np, "presets", i, &str);
		preset_names[i] = str;
		module->equalizer.preset[i].gains = devm_kcalloc(
			&pdev->dev,
			module->equalizer.num_bands,
			sizeof(s16),
			GFP_KERNEL);

		for (j = 0; j < module->equalizer.num_bands; j++)	{
			of_property_read_string_index(
				np, preset_names[i], j, &str);
			ret = kstrtos16(str, 10,
					&module->equalizer.preset[i].gains[j]);
			if (ret < 0)
				return ret;
		}
	}

	module->equalizer.filter_type = devm_kcalloc(&pdev->dev,
						   module->equalizer.num_bands,
						   sizeof(u32),
						   GFP_KERNEL);
	if (module->equalizer.filter_type == NULL)
		    return -ENOMEM;

	for (j = 0; j < module->equalizer.num_bands; j++) {
		module->equalizer.filter_type[j] = STA_BIQUAD_BAND_BOOST_2;
		ret = of_property_read_string_index(
				np, "filter-type", j, &str);
		if (!ret) {
			ret = st_codec_dsp_dts_lut_match(
				eq_filter_type_dts_lut,
				ARRAY_SIZE(eq_filter_type_dts_lut),
				str,
				&module->equalizer.filter_type[j]);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

static int st_codec_dsp_of_loudness(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	const char *str;
	int i, num_freq, num_gain, num_qual, ret;

	module->loudness.num_bands = 2;

	num_gain = of_property_count_strings(np, "gain");
	if (num_gain != module->loudness.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid gain length!\n");
		return -EINVAL;
	}

	for (i = 0; i < module->loudness.num_bands; i++) {
		of_property_read_string_index(
			np, "gain", i, &str);
		ret = kstrtos16(str, 10, &module->loudness.GQF[i][0]);
		if (ret < 0)
			return ret;
	}

	num_qual = of_property_count_strings(np, "quality");
	if (num_qual != module->loudness.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid quality length!\n");
		return -EINVAL;
	}

	for (i = 0; i < module->loudness.num_bands; i++) {
		of_property_read_string_index(
			np, "quality", i, &str);
		ret = kstrtos16(str, 10, &module->loudness.GQF[i][1]);
		if (ret < 0)
			return ret;
	}

	num_freq = of_property_count_strings(np, "frequency");
	if (num_freq != module->loudness.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid frequency length!\n");
		return -EINVAL;
	}

	for (i = 0; i < module->loudness.num_bands; i++) {
		of_property_read_string_index(
			np, "frequency", i, &str);
		ret = kstrtos16(str, 10, &module->loudness.GQF[i][2]);
		if (ret < 0)
			return ret;
	}

	for (i = 0; i < module->loudness.num_bands; i++) {
		module->loudness.filter_type[i] = STA_LOUD_BANDBOOST_2;
		ret = of_property_read_string_index(
				    np, "filter-type", i, &str);
		if (!ret) {
			ret = st_codec_dsp_dts_lut_match(
				    loudness_filter_type_dts_lut,
				    ARRAY_SIZE(loudness_filter_type_dts_lut),
				    str,
				    &module->loudness.filter_type[i]);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

static int st_codec_dsp_of_tones(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	const char *str;
	int i, num_freq, num_gain, num_qual, ret;

	module->tones.num_bands = 3;

	num_gain = of_property_count_strings(np, "gain");
	if (num_gain != module->tones.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid gain length!\n");
		return -EINVAL;
	}

	for (i = 0; i < module->tones.num_bands; i++) {
		of_property_read_string_index(
			np, "gain", i, &str);
		ret = kstrtos16(str, 10, &module->tones.GQF[i][0]);
		if (ret < 0)
			return ret;
	}

	num_qual = of_property_count_strings(np, "quality");
	if (num_qual != module->tones.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid quality length!\n");
		return -EINVAL;
	}

	for (i = 0; i < module->tones.num_bands; i++) {
		of_property_read_string_index(
			np, "quality", i, &str);
		ret = kstrtos16(str, 10, &module->tones.GQF[i][1]);
		if (ret < 0)
			return ret;
	}

	num_freq = of_property_count_strings(np, "frequency");
	if (num_freq != module->tones.num_bands) {
		dev_err(
			&pdev->dev,
			"Invalid frequency length!\n");
		return -EINVAL;
	}

	for (i = 0; i < module->tones.num_bands; i++) {
		of_property_read_string_index(
			np, "frequency", i, &str);
		ret = kstrtos16(str, 10, &module->tones.GQF[i][2]);
		if (ret < 0)
			return ret;
	}

	for (i = 0; i < module->tones.num_bands; i++) {
		module->tones.filter_type[i] = STA_TONE_BANDBOOST_2;
		ret = of_property_read_string_index(
					np, "filter-type", i, &str);
		if (!ret) {
			ret = st_codec_dsp_dts_lut_match(
				    tones_filter_type_dts_lut,
				    ARRAY_SIZE(tones_filter_type_dts_lut),
				    str,
				    &module->tones.filter_type[i]);
			if (ret < 0)
				return ret;
		}
	}
	/* Middle Filter forced to be band boost 2 */
	module->tones.filter_type[1] = STA_TONE_BANDBOOST_2;

	return 0;
}

static int st_codec_dsp_of_gain(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	int ret = -EINVAL;

	ret = of_property_read_u32(
		np, "tup", &module->gains.time_up);
	if (ret) {
		dev_err(
			&pdev->dev,
			"smooth gain tup not found! Set to defalut tup = 100\n");
		module->gains.time_up = 100;
	}

	ret = of_property_read_u32(
		np, "tdown", &module->gains.time_down);
	if (ret) {
		dev_err(
			&pdev->dev,
			"smooth tdown not found! Set to defalut tdown = 100\n");
		module->gains.time_down = 100;
	}

	ret = of_property_read_u32(np, "mute-up",
				   &module->gains.mute_up);
	if (ret)
		module->gains.mute_up = module->gains.time_up;

	ret = of_property_read_u32(np, "mute-down",
				   &module->gains.mute_down);
	if (ret)
		module->gains.mute_down = module->gains.time_down;

	module->gains.vol = devm_kcalloc(
		&pdev->dev,
		module->num_channels,
		sizeof(s16),
		GFP_KERNEL);
	if (module->gains.vol == NULL) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to init gain volume!\n",
			__func__);
		return -ENOMEM;
	}

	module->gains.mute = devm_kcalloc(
		&pdev->dev,
		module->num_channels,
		sizeof(u8),
		GFP_KERNEL);
	if (module->gains.mute == NULL) {
		dev_err(
			&pdev->dev,
			"%s: ERROR: Failed to init gain mute!\n",
			__func__);
		return -ENOMEM;
	}


	return 0;
}

static int st_codec_dsp_of_delay(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	int ret = -EINVAL;
	int j = 0;
	int samples = 0;
	int num_items = 0;
	const char *str_esamples;

	switch (module->type) {
	case STA_DLAY_Y_2CH:
	case STA_DLAY_X_2CH:
		module->num_channels = 2;
		break;
	case STA_DLAY_Y_4CH:
	case STA_DLAY_X_4CH:
		module->num_channels = 4;
		break;
	case STA_DLAY_Y_6CH:
	case STA_DLAY_X_6CH:
		module->num_channels = 6;
		break;
	}

	ret = of_property_read_u32(
		np, "samples", &samples);

	if (ret) {
		dev_dbg(
			&pdev->dev,
			"samples not found! Default delay samples = 0\n");
		samples = 0;
	}
	for (j = 0; j < module->num_channels; j++)
		module->delay.samples[j] = samples;

	num_items = of_property_count_strings(np, "samples-list");
	if (num_items < module->num_channels)
		dev_err(
			&pdev->dev,
			"samples-list expected %d values, read %d values\n",
			module->num_channels,
			num_items);

	for (j = 0; j < num_items; j++) {
		of_property_read_string_index(
			np, "samples-list", j, &str_esamples);
		ret = kstrtos32(str_esamples, 10, &module->delay.samples[j]);
		if (ret < 0)
			return ret;
	}

	module->delay.length = DELAY_SAMPLES_MAX;
	/* delay length impacts on DSP mem */
	ret = of_property_read_u32(
		np, "length", &module->delay.length);
	if (module->delay.length < DELAY_SAMPLES_MIN)
		module->delay.length = DELAY_SAMPLES_MIN;
	if (ret)
		dev_err(
			&pdev->dev,
			"Set to default delay length = %d\n",
			module->delay.length);

	return 0;
}

static int st_codec_dsp_of_mux(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module,
	int mod_type)
{
	int i = 0;
	u32 mux_val = 0;

	switch (mod_type) {
	case STA_MUX_2OUT:
		module->mux.num_out = 2;
	break;
	case STA_MUX_4OUT:
		module->mux.num_out = 4;
	break;
	case STA_MUX_6OUT:
		module->mux.num_out = 6;
	break;
	case STA_MUX_8OUT:
		module->mux.num_out = 8;
	break;
	case STA_MUX_10OUT:
		module->mux.num_out = 10;
	break;
	default:
	break;
	}

	module->mux.sel = devm_kcalloc(
		&pdev->dev,
		module->mux.num_out,
		sizeof(u8),
		GFP_KERNEL);
	if (module->mux.sel == NULL)
		return -ENOMEM;

	for (i = 0; i < module->mux.num_out; i++) {
		of_property_read_u32_index(np, "output", i, &mux_val);
		module->mux.sel[i] = (u8) mux_val;
	}

	return 0;
}

static int st_codec_dsp_of_mixer(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module,
	int mod_type)
{
	int out, in;
	int num_ins = mix_ins_count(module);
	int in_gain;
	int ret;

	for (out = 0; out < module->num_channels; out++) {
		for (in = 0; in < num_ins; in++) {
			ret = of_property_read_u32_index(np, "in-gains",
							 out * num_ins + in,
							 &in_gain);
			if (!ret) {
				if (in_gain > MIXER_VOLUME_MAX) {
					dev_err(&pdev->dev, "in-gains %d too high\n", in_gain);
					return ret;
				}
				module->mixer.vol[out][in] = VAL_TO_DB(in_gain);
			}
		}
	}

	return 0;
}

static int st_codec_dsp_of_control_channels(
	struct platform_device *pdev,
	struct device_node *np,
	struct sta_module *module)
{
	int i, ret;

	module->num_ins = of_property_count_strings(np, "in-names");
	if (module->num_ins > 0) {
		module->in_names = devm_kcalloc(&pdev->dev,
						module->num_ins,
						sizeof(char *),
						GFP_KERNEL);
		if (!module->in_names)
			return -ENOMEM;
		for (i = 0; i < module->num_ins; i++) {
			ret = of_property_read_string_index(
				np, "in-names", i,
				(const char **)&module->in_names[i]);
		}
	}
	module->num_out = of_property_count_strings(np, "out-names");
	if (module->num_out > 0) {
		module->out_names = devm_kcalloc(&pdev->dev,
						module->num_out,
						sizeof(char *),
						GFP_KERNEL);
		if (!module->out_names)
			return -ENOMEM;
		for (i = 0; i < module->num_out; i++) {
			ret = of_property_read_string_index(
				np, "out-names", i,
				(const char **)&module->out_names[i]);
		}
	}

	return 0;
}

static int st_codec_dsp_of_modules(struct platform_device *pdev,
				   struct device_node *nc)
{
	struct device_node *ncca;
	struct device_attribute *dev_attr;
	struct sta_module *module;
	u32 u32_conf, mod_id;
	const char *str_conf;
	int type;
	int i, ret;

	for_each_child_of_node(nc, ncca) {

		if (strcmp(ncca->name, "connections") == 0)
			continue;

		ret = of_property_read_u32(ncca, "dsp-id", &u32_conf);
		if (ret) {
			dev_err(
				&pdev->dev,
				"%s node: wrong or missing dsp-id\n",
				ncca->name);
			return -EINVAL;
		}
		ret = of_property_read_string(ncca, "type", &str_conf);
		if (ret) {
			dev_err(&pdev->dev,
			 "%s node: wrong or missing type\n",
			 ncca->name);
			return -EINVAL;
		}

		type = st_codec_dsp_dts_mdtypelut_match(
			modtype_dts_lut,
			ARRAY_SIZE(modtype_dts_lut),
			str_conf);
		module = devm_kzalloc(
			&pdev->dev,
			sizeof(struct sta_module),
			GFP_KERNEL);
		module->type = type;
		module->name = ncca->name;
		module->dsp_id = u32_conf;
		module->dev = &pdev->dev;

		ret = of_property_read_u32(ncca, "mod-id", &mod_id);
		if (ret && type < STA_USER_0) {
			dev_dbg(&pdev->dev, "STA_AddModule %d %d\n",
				module->dsp_id, type);
			module->mod = STA_AddModule(module->dsp_id, type);
		} else if (ret && type >= STA_USER_0) {
			dev_dbg(&pdev->dev, "STA_AddUserModule %d %d\n",
				module->dsp_id, type);
			user_info.m_dspType = 0;
			module->mod = STA_AddUserModule(module->dsp_id,
							type, &user_info);
		} else if (!ret && type < STA_USER_0) {
			dev_dbg(&pdev->dev, "STA_AddModule2 %d %d %d\n",
				module->dsp_id, type, mod_id);
			module->mod = STA_AddModule2(module->dsp_id,
						     type, mod_id, 0,
						     module->name);
		} else {
			dev_dbg(&pdev->dev, "STA_AddUserModule2 %d %d %d\n",
				module->dsp_id, type, mod_id);
			user_info.m_dspType = 0;
			module->mod = STA_AddUserModule2(module->dsp_id,
							 type, &user_info, mod_id,
							 0, module->name);
		}
		if (!module->mod)
			return -ENOMEM;

		ret = of_property_read_string(
			ncca, "prefix", &module->prefix);

		module->num_channels = 2;
		ret = of_property_read_u32(
			ncca, "num-channels", &u32_conf);
		if (ret == 0) {
			module->mod = STA_SetNumChannels(
				module->mod, u32_conf);
			if (!module->mod) {
				if (STA_GetError() == STA_OUT_OF_MEMORY)
					return -ENOMEM;
				else
					return -EINVAL;
			}
			module->num_channels = (int) u32_conf;
		}

		ret = of_property_read_u32(ncca, "update-slot", &u32_conf);
		if (!ret) 
			STA_SetUpdateSlot(module->mod, u32_conf);

		ret = 0;
		switch (module->type) {
		case STA_MUX_2OUT:
		case STA_MUX_4OUT:
		case STA_MUX_6OUT:
		case STA_MUX_8OUT:
		case STA_MUX_10OUT:
			ret = st_codec_dsp_of_mux(
				pdev, ncca, module, module->type);
			break;
		case STA_MIXE_2INS_NCH:
		case STA_MIXE_3INS_NCH:
		case STA_MIXE_4INS_NCH:
		case STA_MIXE_5INS_NCH:
		case STA_MIXE_6INS_NCH:
		case STA_MIXE_7INS_NCH:
		case STA_MIXE_8INS_NCH:
		case STA_MIXE_9INS_NCH:
			ret = st_codec_dsp_of_mixer(pdev, ncca, module,
						    module->type);
			break;
		case STA_GAIN_STATIC_NCH:
		case STA_GAIN_LINEAR_NCH:
		case STA_GAIN_SMOOTH_NCH:
			ret = st_codec_dsp_of_gain(
				pdev, ncca, module);
			break;
		case STA_TONE_STATIC_DP:
		case STA_TONE_STATIC_STEREO_DP:
			ret = st_codec_dsp_of_tones(
				pdev, ncca, module);
			break;
		case STA_LOUD_STATIC_DP:
		case STA_LOUD_STATIC_STEREO_DP:
			ret = st_codec_dsp_of_loudness(
				pdev, ncca, module);
			break;
		case STA_EQUA_STATIC_1BAND_DP:
		case STA_EQUA_STATIC_2BANDS_DP:
		case STA_EQUA_STATIC_3BANDS_DP:
		case STA_EQUA_STATIC_4BANDS_DP:
		case STA_EQUA_STATIC_5BANDS_DP:
		case STA_EQUA_STATIC_6BANDS_DP:
		case STA_EQUA_STATIC_7BANDS_DP:
		case STA_EQUA_STATIC_8BANDS_DP:
		case STA_EQUA_STATIC_9BANDS_DP:
		case STA_EQUA_STATIC_10BANDS_DP:
		case STA_EQUA_STATIC_11BANDS_DP:
		case STA_EQUA_STATIC_12BANDS_DP:
		case STA_EQUA_STATIC_13BANDS_DP:
		case STA_EQUA_STATIC_14BANDS_DP:
		case STA_EQUA_STATIC_15BANDS_DP:
		case STA_EQUA_STATIC_16BANDS_DP:
		case STA_EQUA_STATIC_1BAND_SP:
		case STA_EQUA_STATIC_2BANDS_SP:
		case STA_EQUA_STATIC_3BANDS_SP:
		case STA_EQUA_STATIC_4BANDS_SP:
		case STA_EQUA_STATIC_5BANDS_SP:
		case STA_EQUA_STATIC_6BANDS_SP:
		case STA_EQUA_STATIC_7BANDS_SP:
		case STA_EQUA_STATIC_8BANDS_SP:
		case STA_EQUA_STATIC_9BANDS_SP:
		case STA_EQUA_STATIC_10BANDS_SP:
		case STA_EQUA_STATIC_11BANDS_SP:
		case STA_EQUA_STATIC_12BANDS_SP:
		case STA_EQUA_STATIC_13BANDS_SP:
		case STA_EQUA_STATIC_14BANDS_SP:
		case STA_EQUA_STATIC_15BANDS_SP:
		case STA_EQUA_STATIC_16BANDS_SP:
			ret = st_codec_dsp_of_equalizer(
				pdev, ncca, module, module->type);
			break;
		case STA_LMTR_1CH:
		case STA_LMTR_2CH:
		case STA_LMTR_4CH:
		case STA_LMTR_6CH:
			ret = st_codec_dsp_of_limiter(
				pdev, ncca, module);
			break;
		case STA_CLIPLMTR_NCH:
			ret = st_codec_dsp_of_clip_limiter(
				pdev, ncca, module);
			break;
		case STA_COMP_1CH:
		case STA_COMP_2CH:
		case STA_COMP_4CH:
		case STA_COMP_6CH:
			ret = st_codec_dsp_of_compander(
				pdev, ncca, module);
			break;
		case STA_SINE_2CH:
			ret = st_codec_dsp_of_sinegen(
				pdev, ncca, module);
			break;
		case STA_PCMCHIME_12BIT_Y_2CH:
		case STA_PCMCHIME_12BIT_X_2CH:
			ret = st_codec_dsp_of_pcmchime(
				pdev, ncca, module);
			break;
		case STA_CHIMEGEN:
			ret = st_codec_dsp_of_chimegen(pdev, ncca, module);
			break;
		case STA_BITSHIFTER_NCH:
			ret = st_codec_dsp_of_bitshifter(
				pdev, ncca, module);
			break;
		case STA_DLAY_Y_2CH:
		case STA_DLAY_X_2CH:
		case STA_DLAY_Y_4CH:
		case STA_DLAY_X_4CH:
		case STA_DLAY_Y_6CH:
		case STA_DLAY_X_6CH:
			st_codec_dsp_of_delay(
				pdev, ncca, module);
			for (i = 0; i < module->num_channels; i++)
				STA_DelaySetLength(module->mod,	i,
						module->delay.length + 1);
			break;
		case STA_PEAKDETECT_NCH:
			dev_attr = (struct device_attribute *)devm_kmemdup(&pdev->dev,
						(const char *)&dev_attr_peak,
 						sizeof(struct device_attribute),
						GFP_KERNEL);
			if (!dev_attr)
    				return -ENOMEM;
			dev_attr->attr.name = module->name;

			ret = device_create_file(&pdev->dev, dev_attr);
			if (ret)
				dev_info(&pdev->dev, "error creating sysfs files\n");

			break;
		}
		if (ret)
			return ret;

		list_add(&module->list, &module_list);

		ret = st_codec_dsp_of_control_channels(pdev, ncca, module);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int st_codec_dsp_of_connections(struct platform_device *pdev,
				       struct device_node *np)
{
	int num_routes;
	int i, ret = -EINVAL;
	const char *s_from, *s_chin, *s_to, *s_chout;
	STAModule from, to;
	struct sta_connection *conn;
	u32 chin, chout;

	num_routes = of_property_count_strings(np, "connections");
	if (num_routes & 1) {
		dev_err(
			&pdev->dev,
			"Connections property length is not even!\n");
		return -EINVAL;
	}

	num_routes /= 4;
	if (!num_routes) {
		dev_err(
			&pdev->dev,
			"Connections length is zero\n");
		return -EINVAL;
	}

	for (i = 0; i < num_routes; i++) {
		ret = of_property_read_string_index(
			np, "connections",
			4 * i, (const char **)&s_from);
		ret = of_property_read_string_index(
			np, "connections",
			(4 * i) + 1, (const char **)&s_chin);
		ret = of_property_read_string_index(
			np, "connections",
			(4 * i) + 2, (const char **)&s_to);
		ret = of_property_read_string_index(
			np, "connections",
			(4 * i) + 3, (const char **)&s_chout);

		from = get_dsp_component(&pdev->dev, s_from);
		if (from == -EINVAL)
			return from;
		to = get_dsp_component(&pdev->dev, s_to);
		if (to == -EINVAL)
    			return to;

		ret = kstrtou32(s_chin, 10, &chin);
		if (ret < 0)
			return ret;
		ret = kstrtou32(s_chout, 10, &chout);
		if (ret < 0)
			return ret;

		STA_Connect(from, chin, to, chout);
		conn = devm_kzalloc(
				&pdev->dev,
				sizeof(struct sta_connection),
				GFP_KERNEL);
		if (!conn)
    			return -ENOMEM;
		strlcpy(conn->from_name, s_from, MAX_MODULE_NAME_LEN);
		conn->from_mod = from;
		conn->from_ch = chin;
		strlcpy(conn->to_name, s_to, MAX_MODULE_NAME_LEN);
		conn->to_mod = to;
		conn->to_ch = chout;
		list_add(&conn->list, &connection_list);

		dev_dbg(
			&pdev->dev,
			"STA_Connect from %x(%x) to %x(%x)\n",
			from, chin, to, chout);
	}

	return 0;
}

static inline int fw_is_loaded(char *mmio)
{
	u32 xram_dbg = readl(mmio + DSP_XRAM_DBG);
	u32 yram_dbg = readl(mmio + DSP_YRAM_DBG);

	return (xram_dbg == DSP_XRAM_DBG_VALUE) &&
	       (yram_dbg == DSP_YRAM_DBG_VALUE);
}

#ifdef CONFIG_STAUDIOLIB_EMBEDDED_FW
static int st_codec_dsp_load(struct device *dev)
{
	static char *dspaddr;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);
	int i, core;

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		core = dsp_codec_drv->dsp_data[i].core;
		dspaddr = dsp_codec_drv->dsp_data[i].dsp_addr;
		if (fw_is_loaded(dspaddr))
			dev_info(dev, "DSP%d firmware is already loaded!\n", core);
	}

	dsp_skip_write(0);

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		core = dsp_codec_drv->dsp_data[i].core;
		STA_LoadDSP_ext(core,
				dsp_p_start, dsp_p_size,
				dsp_x_start, dsp_x_size,
				dsp_y_start, dsp_y_size);
		dev_info(dev, "DSP%d firmware embedded loaded\n", core);
	}
	STA_Reset();

	return 0;
}
#else
static int st_codec_dsp_load(struct device *dev)
{
	const char *st_fw = "emerald-firmware";
	const char *suffix[3] = {"P.noheader", "X.noheader", "Y.noheader"};
	const struct firmware *fw[3];
	char *file;
	int err = 0, i, core, xyp;
	static char *dspaddr;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

#if IS_BUILTIN(CONFIG_SND_SOC_STA_STAUDIOLIB)
	dev_err(dev, "can't load DSP if not built as module!\n");
	return -ENOENT;
#endif

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		core = dsp_codec_drv->dsp_data[i].core;
		dspaddr = dsp_codec_drv->dsp_data[i].dsp_addr;
		if (fw_is_loaded(dspaddr))
			dev_info(dev, "DSP%d firmware is already loaded!\n", core);
	}

	/* Disable early init flag */
	dsp_skip_write(0);

	/* Read the DSP FW from firmware files */
	for (xyp = 0; xyp < ARRAY_SIZE(suffix); xyp++) {
		file = kasprintf(GFP_KERNEL, "%s/%s.%s", ST_CODEC_DSP_FW_SUBPATH, st_fw, suffix[xyp]);
		err = request_firmware(&fw[xyp], file, dev);
		kfree(file);
		if (err < 0)
			goto end_load;
	}

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		core = dsp_codec_drv->dsp_data[i].core;

		/* Upload DSP FW */
		STA_LoadDSP_ext(core,
				(char *)fw[0]->data, fw[0]->size,
				(char *)fw[1]->data, fw[1]->size,
				(char *)fw[2]->data, fw[2]->size);

		dev_warn(dev, "DSP%d ST FW loaded\n", core);
	}

end_load:
	for (i = 0; i < xyp; i++)
		release_firmware(fw[i]);

	STA_Reset();
	return err;
}
#endif

static bool st_codec_dsp_started(struct codec_dsp_drvdata *dsp_codec_drv)
{
	u32 ssy;
	int i, core;

	regmap_read(dsp_codec_drv->auss_regmap, AUSS_SSY_CR, &ssy);
	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
		core = dsp_codec_drv->dsp_data[i].core;
		if (!(ssy & BIT(SSY_EMERCORERESET + core))) {
			dev_info(dsp_codec_drv->dev, "DSP not started\n");
			return false;
		}
	}
	return true;
}

static int dsp_init_thread(void *arg)
{
	struct codec_dsp_drvdata *dsp_codec_drv = (struct codec_dsp_drvdata *)arg;

	initialize_pre();
	STA_BuildFlow();
	STA_Play();
	STA_WaitDspReady(dsp_codec_drv->dspmask, 100);
	initialize_post();
	dsp_skip_write(0);
	complete_all(&dsp_init_completion);

	return 0;
}

static int st_codec_dsp_of_probe(
	struct platform_device *pdev,
	struct device_node *np)
{
	struct device_node *nc;
	int ret;
	int i, cnt, gpio;
	int dco_cr, dco_bit;
	char *net_id = "";
	struct codec_dsp_drvdata *dsp_codec_drv;
	struct task_struct *kthread;
	STA_Params params = {0};

	params.dev = (void *)&pdev->dev;
	dsp_codec_drv = platform_get_drvdata(pdev);
	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
	    if (dsp_codec_drv->dsp_data[i].core == 0)
		    params.dsp0_base = dsp_codec_drv->dsp_data[i].dsp_addr;
	    else if (dsp_codec_drv->dsp_data[i].core == 1)
		    params.dsp1_base = dsp_codec_drv->dsp_data[i].dsp_addr;
	    else if (dsp_codec_drv->dsp_data[i].core == 2)
		    params.dsp2_base = dsp_codec_drv->dsp_data[i].dsp_addr;
	}
	params.cutversion = 0;

	ret = of_property_read_string(
		np, "network-id", (const char **)&net_id);
	if (ret)
		dev_warn(&pdev->dev, "network-id missing!\n");

	if (dspnet && strlen(dspnet)) {
		dev_info(&pdev->dev, "force dspnet = %s\n", dspnet);
		net_id = dspnet;
	}
	nc = np;
	if (net_id) {
		nc = of_get_child_by_name(np, net_id);
		if (!nc) {
			dev_err(&pdev->dev, "network not found!\n");
			return -ENOENT;
		}
	}

	cnt = of_property_count_u32_elems(np, "dco-gpios");
	if (cnt > 6) {
		dev_err(&pdev->dev, "dco-gpios > 6!\n");
		return -EINVAL;
	}
	for (i = 0; i < cnt; i++) {
		ret = of_property_read_u32_index(np, "dco-gpios", i, &gpio);
		if (ret < 0)
			return ret;
		ret = devm_gpio_request(&pdev->dev, gpio,
					devm_kasprintf(&pdev->dev, GFP_KERNEL,
						       "dco%i", i));
		if (ret < 0)
			return ret;
		gpio_direction_input(gpio);
		dco_cr = i < 3 ? AUSS_DCO_CR0 : AUSS_DCO_CR1;
		dco_bit = (i % 3) * 6;
		regmap_update_bits(dsp_codec_drv->auss_regmap, dco_cr,
				   GENMASK(dco_bit + 5, dco_bit),
				   gpio << dco_bit);
	}

	STA_Init(&params);

	dsp_codec_drv->early_init =
	    (STA_WaitDspReady(dsp_codec_drv->dspmask, 0) &&
	     st_codec_dsp_started(dsp_codec_drv));
	if (dsp_codec_drv->early_init) {
		dev_info(&pdev->dev, "DSP early initialized\n");
		dsp_skip_write(1);
	} else {
		ret = st_codec_dsp_load(&pdev->dev);
		if (ret)
			return ret;
	}

	ret = st_codec_dsp_of_modules(pdev, nc);
	if (ret)
		return ret;

	ret = st_codec_dsp_of_connections(pdev, nc);
	if (ret)
		return ret;

	kthread = kthread_run(dsp_init_thread, dsp_codec_drv, "dsp_init");
	if (IS_ERR(kthread)) {
		dev_err(&pdev->dev, "DSP init thread Creation failed");
		return -ENOMEM;
	}

	dev_info(
		&pdev->dev, "staudiolib dsp version %d\n",
		readl(dsp_codec_drv->dsp_data[0].dsp_addr + DSP_FW_VER_OFFSET));

	return 0;
}

static int early_audio_volume(struct audio_data *audio_data, void *arg)
{
	struct sta_module *module;
	struct device *dev = (struct device *)arg;
	int ch;

	dev_info(dev, "early volume %s [%d]\n", audio_data->desc, audio_data->arg);
	wait_dsp_init();
	module = get_sta_module_by_name(audio_data->desc);
	if (module) {
		module->early_init = 1;
		for (ch = 0; ch < module->num_channels; ch++) {
			module->gains.vol[ch] = VAL_TO_DB(audio_data->arg);
			STA_GainSetGain(module->mod, ch, module->gains.vol[ch]);
		}
	}

	return 0;
}

static ssize_t trace_set(struct device *dev,
			 struct device_attribute *attr,
			 const char *buf, size_t count)
{
	long l;

	if (buf[0] < '0' || buf[0] > '9') {
		dev_err(dev, "not a number\n");
		return -EINVAL;
	}

	if (kstrtol(buf, 0, &l) < 0) {
		dev_err(dev, "kstrtol err\n");
		return count;
	}

	if (l) {
		dsp_trace_enable(true);
	} else {
		dsp_trace_enable(false);
	}

	return count;
}

static DEVICE_ATTR(trace, 0200, NULL, trace_set);

static ssize_t modinfo_get(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct sta_module *module;
	char *p = buf;
	unsigned long pfn;
	struct page *page;
	u32 xaddr, yaddr;

	list_for_each_entry(module, &module_list, list) {
		xaddr = STA_GetModuleInfo(module->mod, STA_INFO_XDATA);
		page = vmalloc_to_page((void *)xaddr);
		pfn = page_to_pfn(page);
		xaddr = (pfn << PAGE_SHIFT) | (xaddr & (PAGE_SIZE-1));

		yaddr = STA_GetModuleInfo(module->mod, STA_INFO_YDATA);
		page = vmalloc_to_page((void *)yaddr);
		pfn = page_to_pfn(page);
		yaddr = (pfn << PAGE_SHIFT) | (yaddr & (PAGE_SIZE-1));

		p += sprintf(p, "%s %x %x %x\n", module->name, xaddr, yaddr, module->type);
	}

	return p - buf;
}

static DEVICE_ATTR(modinfo, 0444, modinfo_get, NULL);

static ssize_t
sta_audio_write_ctx(struct device *dev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	struct sta_module *module;
	int ret, slot = 0, val;
	char name[32];

	ret = sscanf(buf, "%d %s %d", &slot, name, &val);
	if (slot < 0 || slot > 6)
		return -EINVAL;
	if (ret == 2) {
		/* backup_slot api_name */
		st_codec_early_audio_send_args(EARLY_AUDIO_BACKUP_API,
					       name, 0, slot);
	} else if (ret == 3) {
		/* backup_slot module_name gain_value */
		module = get_sta_module_by_name(name);
		if (!module)
			return -EINVAL;
		if (module->type != STA_GAIN_SMOOTH_NCH)
			return -EINVAL;
		if (val < GAIN_VOLUME_MIN || val > GAIN_VOLUME_MAX)
			return -EINVAL;
		st_codec_early_audio_send_args(EARLY_AUDIO_BACKUP_VOL,
					       (char *)module->name, val, slot);
	} else {
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR(audio_ctx, 0200, NULL, sta_audio_write_ctx);

static int st_codec_dsp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct resource *res = NULL;
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
		if (res <= 0){
			/* DSP not managed by this device */
			dsp_codec_drv->dsp_data[i].dsp_addr = NULL;
    			continue;
		}
		dsp_codec_drv->dsp_data[i].dsp_addr = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(dsp_codec_drv->dsp_data[i].dsp_addr))
			return PTR_ERR(dsp_codec_drv->dsp_data[i].dsp_addr);
		sprintf(key, "dpmem%d", core);
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, key);
		dsp_codec_drv->dsp_data[i].dsp_dpmem = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(dsp_codec_drv->dsp_data[i].dsp_dpmem))
			return PTR_ERR(dsp_codec_drv->dsp_data[i].dsp_dpmem);
		dsp_codec_drv->dsp_data[i].core = core;
		dsp_codec_drv->num_dsp++;
                dsp_codec_drv->dspmask |= BIT(core);
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

	ret = st_codec_dsp_of_probe(pdev, np);
	if (ret) {
		dev_err(dev, "Failed to initialize DSP!\n");
		goto err_dsp_probe;
	}

	for (i = 0; i < ARRAY_SIZE(st_codec_dsp_dai); i++) {
		ret = snd_soc_register_component(
			dev,
			&st_snd_soc_component[i],
			&st_codec_dsp_dai[i],
			1);
		if (ret) {
			dev_err(&pdev->dev, "Failed to register component!\n");
			goto err_dsp_probe;
		}
	}

	msg_audio_register(EARLY_AUDIO_VOLUME, early_audio_volume, &pdev->dev);
	st_codec_early_audio_send(EARLY_AUDIO_VOLUME);

	ret = device_create_file(&pdev->dev, &dev_attr_trace);
	if (ret)
		dev_info(dev, "error creating sysfs files\n");

	ret = device_create_file(&pdev->dev, &dev_attr_modinfo);
	if (ret)
		dev_info(dev, "error creating sysfs files\n");

	ret = device_create_file(&pdev->dev, &dev_attr_audio_ctx);
	if (ret)
		dev_info(dev, "error creating sysfs files\n");

	dsp_trace_init();

	return ret;

err_dsp_probe:
	clk_disable_unprepare(dsp_codec_drv->emrclk);
err_dsp_clk:
	clk_disable_unprepare(dsp_codec_drv->mclk);

	return ret;
}

static int st_codec_dsp_remove(struct platform_device *pdev)
{
	struct codec_dsp_drvdata *dsp_codec_drv	=
		dev_get_drvdata(&pdev->dev);

	snd_soc_unregister_component(&pdev->dev);

	STA_Exit();

	clk_disable_unprepare(dsp_codec_drv->emrclk);
	clk_disable_unprepare(dsp_codec_drv->mclk);

	return 0;
}

/* dsp wrapper functions exported to staudiolib */
u32 dsp_readl(const void *addr)
{
	return _dsp_readl(addr);
}

int dsp_writel(void *addr, u32 value)
{
	TRACE_PRINT((u32)addr, value);
	if (SKIP_WRITE)
		return 0;

    	return _dsp_writel(addr, value);
}

static inline int dsp_memset(u32 *b2, u32 val, int len)
{
	int i;

	for (i = 0; i < len / 4; i++)
		dsp_writel(b2 + i, val);

	return 0;
}

int dsp_clken(u32 *d, int core)
{
	struct device *dev = (struct device *)d;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

    	return _dsp_clken(dsp_codec_drv->auss_regmap, core);
}

int dsp_clkdis(u32 *d, int core)
{
	struct device *dev = (struct device *)d;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

    	return _dsp_clkdis(dsp_codec_drv->auss_regmap, core);
}

int dsp_clk_is_en(u32 *d, int core)
{
	struct device *dev = (struct device *)d;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

    	return _dsp_clk_is_en(dsp_codec_drv->auss_regmap, core);
}

int dsp_start(u32 *d, int core)
{
	struct device *dev = (struct device *)d;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

    	return _dsp_start(dsp_codec_drv->auss_regmap, core);
}

int dsp_stop(u32 *d, int core)
{
	struct device *dev = (struct device *)d;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);

    	return _dsp_stop(dsp_codec_drv->auss_regmap, core);
}

int dsp_xin_dmabus_clear(u32 *d, int core)
{
	struct device *dev = (struct device *)d;
	struct codec_dsp_drvdata *dsp_codec_drv	= dev_get_drvdata(dev);
	int i;

	for (i = 0; i < dsp_codec_drv->num_dsp; i++) {
    		if (dsp_codec_drv->dsp_data[i].core == core) {
			dsp_memset((u32 *)dsp_codec_drv->dsp_data[i].dsp_dpmem, 0, XIN_SIZE);
			return 0;
		}
	}
	return -ENOENT;
}

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

static struct platform_driver st_codec_dsp_driver = {
	.driver = {
		.name = "st-codec-dsp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(st_codec_dsp_dt_ids),
		.pm =		&dsp_pm_ops,
	},
	.probe =    st_codec_dsp_probe,
	.remove =   st_codec_dsp_remove,
};

module_platform_driver(st_codec_dsp_driver);

MODULE_DESCRIPTION("SoC st_codec_dsp driver");
MODULE_AUTHOR("Gabriele Simone gabriele.simone@st.com");
MODULE_LICENSE("GPL");
