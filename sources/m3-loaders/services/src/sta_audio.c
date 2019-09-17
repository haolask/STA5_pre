/**
 * @file sta_audio.c
 * @early audio setup
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "utils.h"
#include "queue.h"
#include "sta_common.h"
#include "sta_mtu.h"
#include "audiolib.h"
#include "sta_gpio.h"
#include "sta_pinmux.h"
#include "sta_rpmsg_audio.h"
#include "sta_dma.h"
#include "sta_audio.h"
#include "sta_rotary.h"
#include "sta_math.h"
#include "sta_ampli.h"

/* AIF */
#define DMABUS_CR		0x48d04000
#define DMABUS_TCM		0x48d06000
#define DMABUS_TCM_SIZE	512
#define SAI1			0x48d08C00
#define SAI2			0x48d08C04
#define SAI3			0x48d08c08
#define SAI4			0x48d08C0C
#define SRC0			0x48d08000
#define SRC1			0x48d08004
#define SRC2			0x48d08008
#define SRC3			0x48d0800c
#define SRC4			0x48d08010
#define SRC5			0x48d08004
#define AIMUX			0x48d08800
#define LPF			0x48d08400
#define AOMUX			0x48d08804
#define ADCAUX			0x48d10000
/* AUSS */
#define ADCMIC			0x48060004
#define DAC_CR			0x48060010
#define SSY_CR			0x48060100
#define MUX_CR			0x48061000
/* DPM */
#define DPM_FIFO2		0x48C00200
#define DPM_FIFO_LEN	32
#define MAX_API 50
/* DSP*/
#define XIN_SIZE		128
static int XIN[] = {0x48984000, 0x48a84000, 0x48b8c000};
static int XOUT[] = {0x48984200, 0x48a84200, 0x48b8c200};
static int DPXIN[] = {0x48d14000, 0x48d18000, 0x48d1c000};

#define AUDIO_BACKUP_MAGIC_WORD	0xad10ca13
#define audio_backup ((struct t_audio_backup *)BACKUP_RAM_AUDIO_CONTEXT_BASE)
#define AUDIO_BACKUP_NUM ((BACKUP_RAM_AUDIO_CONTEXT_SIZE - 4) / 4)
#define AUDIO_BACKUP_VOL 1
#define AUDIO_BACKUP_API 2

struct t_audio_backup {
	unsigned long magic;
	struct {
		uint32_t type:2;
		uint32_t mod_id:8;
		uint32_t value:15;
	} slot[AUDIO_BACKUP_NUM];
};

#define DIV_ROUND_CLOSEST(n, d) (((n) + ((d) / 2)) / (d))
#define MIN(a,b) (((a) > (b)) ? (b) : (a))
#define ABS(a) (((a) < 0) ? -(a) : (a))
#define CLAMP(a, min, max)	((a)<(min) ? (min): ((a)>(max) ? (max) : (a)))
#define STA_GAIN_STATIC_NCH 	8
#define STA_GAIN_SMOOTH_NCH	9
#define STA_GAIN_LINEAR_NCH	10
#define PCM_DMA_CHAN	7

struct audio_feature_set {
	uint8_t pcm_play;
	uint8_t aux_detect;
	uint8_t chime;
	uint8_t volume;
	uint8_t ampli;
};

static struct audio_feature_set features = {
	.chime = false,
	.pcm_play = true,
	.aux_detect = true,
	.volume = true,
	.ampli = false,
};

static struct sta_audio {
	char *current_source;
	uint32_t audio_status;
	void *audiolib_addr;
	QueueHandle_t pcm_queue;
	QueueHandle_t aux_queue;
	QueueHandle_t vol_queue;
	struct mtu_device *dac_mtu;
	int vol;
} sta_audio;

enum early_audio_msg {
	EARLY_AUDIO_SOURCE = 0xea000001,
	EARLY_AUDIO_PLAY,
	EARLY_AUDIO_END_PLAY,
	EARLY_AUDIO_END,
	EARLY_AUDIO_END_PCM,
	EARLY_AUDIO_VOLUME,
	EARLY_AUDIO_BACKUP_API,
	EARLY_AUDIO_BACKUP_VOL,
};

typedef struct {
	char           id[4];
	unsigned long  size;
} subchunk_hdr;

typedef struct {
    char                riff[4];        /* RIFF Header */
    unsigned long       chunk_size;     /* RIFF Chunk Size */
    char                wave[4];        /* WAVE Header */
    char                fmt[4];         /* FMT header */
    unsigned long       subchunk_size; /* Size of the fmt chunk */
    unsigned short      audio_format;   /* 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM */
    unsigned short      num_of_chan;    /* 1=Mono 2=Stereo */
    unsigned long       sample_per_sec; /* Sampling Frequency in Hz */
    unsigned long       bytes_per_sec;  /* bytes per second */
    unsigned short      block_align;    /* 2=16-bit mono, 4=16-bit stereo */
    unsigned short      bits_per_sample;/* Number of bits per sample */
    subchunk_hdr        subchunk2;
} wav_hdr;

typedef struct {
	long        gain;
	long        gain_goal;
	int         lshift;
	long        inc;
	long        inc_factor;
} t_gain_smooth_ch;

typedef struct {
	int         sync;
	int         numch;
	t_gain_smooth_ch *pch;
} t_gain_smooth;

#undef DEBUG
#ifdef DEBUG
void toggle_gpio()
{
	struct gpio_config pin;
	static int init = 1;

	pin.direction   = GPIO_DIR_OUTPUT;
	pin.mode        = GPIO_MODE_SOFTWARE;
	pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
	pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
	gpio_set_pin_config(A7_GPIO(23), &pin);
	if (init)
		gpio_clear_gpio_pin(A7_GPIO(23));
	else
		gpio_set_gpio_pin(A7_GPIO(23));
	init = !init;
}
#else
#define toggle_gpio()
#endif

/**
 * @brief	Turn on DAC
 * @return	None
 */
void dac_power_on()
{
	toggle_gpio();

	write_reg(0x00000009, SAI3); /* clock on */
	write_reg(0, DAC_CR); /* unmute */

	sta_audio.dac_mtu = mtu_start_timer(500000, ONE_SHOT_MODE, NULL);
}

/**
 * @brief	Initialize SRC filters
 * @return	None
 */
int early_src_init(void)
{
	audsscr_regs->mux_cr.bit.msp2_en = 1;
	aif_regs->aimux_csr.reg = 0x222222;
	aif_regs->aomux_csr.reg = 0xeeeeee;
	aif_regs->lpf_csr.reg = 0xaa5;
	aif_regs->src0csr.reg = 7;
	aif_regs->sai2csr.bit.ena = 1;
	msp2_regs->msp_cgr.bit.tcksel = 1; /* master */
	msp2_regs->msp_cgr.bit.sgen = 1;
	vTaskDelay(pdMS_TO_TICKS(5));
	msp2_regs->msp_cgr.reg = 0;

	return 0;
}

static api_header *get_api(char *name, unsigned int signature)
{
	api_header *api;
	void *addr = sta_audio.audiolib_addr;

	TRACE_INFO("%s [%s]\n", __func__, name);
	while (1) {
		api = (api_header *) addr;
		TRACE_INFO("%x: %x,[%s] %x,%x\n",
			   addr - sta_audio.audiolib_addr,
			   api->id, api->name, api->hsize, api->size);
		if (!IS_VALID(api->id))
			return NULL;

		if ((api->id == signature) && !strncmp(name, api->name, strlen(name)))
			break;

		addr += api->hsize;
		addr += api->size;
	}

	return api;
}

static int get_module_id(char *module)
{
	modinfo *m;
	void *start, *end;
	api_header *api;
	int n;

	api = get_api("modinfo", MOD_SIGNATURE);
	if (!api) {
		TRACE_ERR("modinfo not found\n");
		return -1;
	}
	start = (void *)api + api->hsize;
	end = start + api->size;
	for (m = (modinfo *)start, n = 0;
	     m < (modinfo *)end;
	     m = (void *)m + m->size, n++)
		if (!strncmp(module, m->name, strlen(module)))
			return n;

	return -1;
}

static modinfo *get_module_by_id(int mod_id)
{
	modinfo *m;
	void *start, *end;
	api_header *api;
	int n = 0;

	api = get_api("modinfo", MOD_SIGNATURE);
	if (!api) {
		TRACE_ERR("modinfo not found\n");
		return NULL;
	}
	start = (void *)api + api->hsize;
	end = start + api->size;
	for (m = (modinfo *)start, n = 0;
	     m < (modinfo *)end;
	     m = (void *)m + m->size, n++)
		if (mod_id == n)
			return m;

	return NULL;
}

static modinfo *get_module(char *module)
{
	modinfo *m;
	void *start, *end;
	api_header *api;

	api = get_api("modinfo", MOD_SIGNATURE);
	if (!api) {
		TRACE_ERR("modinfo not found\n");
		return NULL;
	}
	start = (void *)api + api->hsize;
	end = start + api->size;
	for (m = (modinfo *)start; m < (modinfo *)end; m = (void *)m + m->size)
		if (!strncmp(module, m->name, strlen(module)))
			return m;

	return NULL;
}

static int get_volume(char *gain_module)
{
	modinfo *m;
	int ret;
	t_gain_smooth_ch *coef;
	long newgain;

	m = get_module(gain_module);
	if (!m)
		return -ENOENT;

	coef = (t_gain_smooth_ch *)(m->yaddr + sizeof(t_gain_smooth));
	newgain  = read_reg((uint32_t)&coef->gain_goal);
	ret = STA_lin2db(newgain) + 1200;
	TRACE_INFO("%s: %d\n", __func__, ret);
	return ret;
}

/* NOTE:
 * static gain not supported 
 * polarity always positive, no changes to shift, ramp fixed
 */
static int set_volume(char *gain_module, int db)
{
	modinfo *m;
	t_gain_smooth *params;
	t_gain_smooth_ch *coef;
	int ch;
	long curgain, newgain;
	long inc, inc0, incfactor;
	float incf;
	long m_kup = 100;
	long m_kdown = 100;

	if (db > 1440 || db < 0)
		return -EINVAL;
	TRACE_INFO("%s: %d\n", __func__, db);
	db -= 1200; /* use same scale of Linux ALSA controls */

	m = get_module(gain_module);
	if (!m)
		return -ENOENT;

	newgain  = STA_db2lin(db);
	if (m->type == STA_GAIN_STATIC_NCH)
		return -EINVAL;
    
	params = (t_gain_smooth *)m->yaddr;
	write_reg(1, (uint32_t)&params->sync);
	mtu_wait_delay(21);
	for (ch = 0; ch < params->numch; ch++) {
		coef = (t_gain_smooth_ch *)(m->yaddr
					 + sizeof(t_gain_smooth)
					 + sizeof(t_gain_smooth_ch) * ch);
		curgain = (read_reg((uint32_t)&coef->gain) << 8) >> 8;
		incfactor = (ABS(newgain) > ABS(curgain)) ? m_kup : m_kdown;
		inc	= newgain - curgain;
		if (m->type == STA_GAIN_LINEAR_NCH) {
			inc0 = inc;
			incf = (float)inc / incfactor;
			inc  = (long)incf;
			inc  = CLAMP(inc, (long)0xFF800000, (long)0x7FFFFF);
			if (inc == 0 && inc0 != 0)
				inc = (inc0 > 0) ? +1 : -1;
			incfactor = 0;
		} else {
			inc = ABS(newgain - curgain);
		}
		write_reg(curgain, (uint32_t)&coef->gain);
		write_reg(newgain, (uint32_t)&coef->gain_goal);
		write_reg(inc, (uint32_t)&coef->inc);
		write_reg(incfactor, (uint32_t)&coef->inc_factor);
	}
	write_reg(0, (uint32_t)&params->sync);
	return 0;
}

static int mute(char *gain_module)
{
	int vol = get_volume(gain_module);
	set_volume(gain_module, 0);
	return vol;
}

static void unmute(char *gain_module, int vol)
{
	set_volume(gain_module, vol);
}

/**
 * @brief	Applies an audiolib function
 * @return	None
 */
static void early_audio_exec_api(api_header *api)
{
	setreg *s;
	void *start, *end;

	start = (void *)api + api->hsize;
	end = start + api->size;
	for (s = (setreg *)start; s < (setreg *)end; s++)
		if (s->addr)
			write_reg(s->val, s->addr);
}

static int early_audio_exec(char *api_name)
{
	api_header *api;

	TRACE_INFO("%s [%s]\n", __func__, api_name);
	api = get_api(api_name, API_SIGNATURE);
	if (!api) {
		TRACE_ERR("API %s not found\n", api_name);
		return -EINVAL;
	}

	early_audio_exec_api(api);

	return 0;
}

static void dsp_clean(int dsp)
{
	int i;

	for (i = 0; i < (XIN_SIZE); i++) {
		write_reg(0, (XIN[dsp] + (i * 4)));
		write_reg(0, (XOUT[dsp] + (i * 4)));
		write_reg(0, (DPXIN[dsp] + (i * 4)));
	}
	/* write to DPXIN[127] swaps DP, then clear the other side of XOUT */
	for (i = 0; i < (XIN_SIZE); i++)
		write_reg(0, (XOUT[dsp] + (i * 4)));
}

/**
 * @brief	Applies an audio source
 * @return	None
 */
static int early_audio_init(char *source)
{
	int i, ret, vol;

	TRACE_INFO("%s\n",source);

/* Clear DMABUS_TCM */
	write_reg(0, DMABUS_CR);
	for (i = 0; i < DMABUS_TCM_SIZE; i++)
		write_reg(0, (DMABUS_TCM + (i * 4)));

/* Clear DPmailbox FIFOOUT */
	for (i = 0; i < DPM_FIFO_LEN; i++)
		write_reg(0, DPM_FIFO2);

	vol = mute("gains_out");
	ret = early_audio_exec(source);
	if (ret)
		return ret;

/* DMABUS RUN */
	write_reg(0x10, DMABUS_CR);
	unmute("gains_out", vol);

	sta_audio.current_source = source;

	return 0;
}

static void audio_status_init(void)
{
	int i;
	api_header *api;
	modinfo *m;
	bool main_vol_init = false;

	if (audio_backup->magic != AUDIO_BACKUP_MAGIC_WORD) {
		audio_backup->magic = AUDIO_BACKUP_MAGIC_WORD;
		for (i = 0; i < AUDIO_BACKUP_NUM; i++)
			audio_backup->slot[i].type = 0;
	}

	/* apply audio status stored in backup RAM */
	for (i = 0; i < AUDIO_BACKUP_NUM; i++) {
		if (audio_backup->slot[i].type == AUDIO_BACKUP_VOL) {
			m = get_module_by_id(audio_backup->slot[i].mod_id);
			if (!m) {
				audio_backup->slot[i].type = 0;
				continue;
			}
			TRACE_INFO("Set volume %s = %d\n",
				   m->name, audio_backup->slot[i].value);
			if (!main_vol_init) {
				/* assume main volume is the first in backup */
				sta_audio.vol = audio_backup->slot[i].value;
				main_vol_init = true;
			}
			set_volume(m->name, audio_backup->slot[i].value);
		} else if (audio_backup->slot[i].type == AUDIO_BACKUP_API) {
			api = (api_header *)(sta_audio.audiolib_addr
					     + audio_backup->slot[i].value);
			if (!IS_VALID(api->id)) {
				audio_backup->slot[i].type = 0;
				continue;
			}
			TRACE_INFO("Backup %d = %s\n", i, api->name);
			early_audio_exec_api(api);
		}
	}

	if (!main_vol_init) {
		/* get the default vol in the DSP image */
		sta_audio.vol = get_volume("gains_out");
		if (features.ampli) {
			TRACE_INFO("ampli\n");
			sta_audio.vol -= 300;
			ampli_poweron();
			set_volume("gains_out", sta_audio.vol);
		}
	}
}

static int early_audio_backup_volume(struct audio_data *rx_audio_data)
{
	int slot = rx_audio_data->id;
	int mod_id;

	if (slot >= AUDIO_BACKUP_NUM)
		return -1;

	mod_id = get_module_id(rx_audio_data->desc);
	if (mod_id > 0) {
		audio_backup->slot[slot].type = AUDIO_BACKUP_VOL;
		audio_backup->slot[slot].mod_id = mod_id;
		audio_backup->slot[slot].value = rx_audio_data->arg;
	}

	return 0;
}

static int early_audio_backup_api(struct audio_data *rx_audio_data)
{
	api_header *api;
	int slot = rx_audio_data->id;

	if (slot >= AUDIO_BACKUP_NUM)
		return -1;

	api = get_api(rx_audio_data->desc, API_SIGNATURE);
	if (api) {
		audio_backup->slot[slot].type = AUDIO_BACKUP_API;
		audio_backup->slot[slot].value =
				((void *)api - sta_audio.audiolib_addr);
	}
	return !api;
}

static int early_audio_end_volume(struct audio_data *rx_audio_data)
{
	uint32_t msg_id = 0xdead;
	struct audio_data audio_data;

	if (sta_audio.vol_queue)
		xQueueSend(sta_audio.vol_queue, &msg_id, 0);

	audio_data.msg_id = EARLY_AUDIO_VOLUME;
	strcpy(audio_data.desc, "gains_out");
	audio_data.arg = sta_audio.vol;
	msg_audio_send((void *)&audio_data);
	strcpy(audio_data.desc, "gains_primary_media");
	audio_data.arg = get_volume("gains_primary_media");
	msg_audio_send((void *)&audio_data);

	return 0;
}

static int early_audio_end_pcm(struct audio_data *rx_audio_data)
{
	uint32_t msg_id = 0xdead;

	if (sta_audio.pcm_queue)
		return xQueueSend(sta_audio.pcm_queue, &msg_id, 0);

	return 0;
}

static int early_audio_source(void *q)
{
	struct audio_data audio_data;
	uint32_t msg_id = 0xdead;

	if (sta_audio.aux_queue)
		xQueueSend(sta_audio.aux_queue, &msg_id, 0);

	if (sta_audio.current_source) {
		audio_data.msg_id = EARLY_AUDIO_SOURCE;
		strcpy(audio_data.desc, sta_audio.current_source);
		msg_audio_send((void *)&audio_data);
	}

	return 0;
}

static unsigned long mspclk()
{
	uint32_t fxtal = (get_soc_id() == SOCID_STA1385 ? 26 : 24);
	unsigned long long mspclk;

	mspclk = (unsigned long long)2 * src_m3_regs->pll2fctrl.bit.ndiv * fxtal * 1000000 / src_m3_regs->pll2fctrl.bit.idf / 24;
	return (unsigned long)mspclk;
}

static DMA_LinkedListTy lli[20] __attribute__((aligned (16)));

static int msp2dma(wav_hdr *w)
{
	DMACChannelTy *dma0_ch = &dma0_regs->DMACChannelReg[PCM_DMA_CHAN];
	int m3_sram_base = 0x10000000;
	int width = 4;
	int num_words = 0;
	void *pcm_data;
	unsigned int i;
	subchunk_hdr *subchunk;
	struct audio_data audio_data;

	if (w == NULL){
		dma0_ch->DMACCConfiguration.enabled = 0;
		msp2_regs->msp_cgr.bit.txen = 0;
		msp2_regs->msp_cgr.bit.fgen = 0;
		msp2_regs->msp_cgr.bit.sgen = 0;
		audio_data.msg_id = EARLY_AUDIO_END_PLAY;
		audio_data.arg = (uint32_t)msp2_regs;
		msg_audio_send((void *)&audio_data);
		return 0;
	}

	if (strncmp(w->riff, "RIFF", 4))
		return -1;

	if (w->bits_per_sample != 16)
		return -1;

	subchunk = &w->subchunk2;
	while(subchunk < (subchunk_hdr *)((void *)w + w->chunk_size)){
		if (!strncmp(subchunk->id, "data", 4)){
			num_words = subchunk->size/width;
			pcm_data = (void *)(subchunk + 1);
		}
		subchunk = (void *)(subchunk + 1) + subchunk->size;
	}

	/* stop ongoing play */
	dma0_ch->DMACCConfiguration.enabled = 0;
	msp2_regs->msp_cgr.bit.fgen = 0;
	msp2_regs->msp_cgr.bit.sgen = 0;
	msp2_regs->msp_cgr.bit.txen = 0;
	vTaskDelay(pdMS_TO_TICKS(1));

	TRACE_NOTICE("PCM @%dHz, %d x %dbits\n",
		     w->sample_per_sec, w->num_of_chan, w->bits_per_sample);

	for(i = 0; i < sizeof(lli)/sizeof(lli[0]) && num_words > 0; i++) {
		lli[i].DMACCSrcAddr = (unsigned int)pcm_data;
		lli[i].DMACCDestAddr = 0x48020000;
		lli[i].DMACCLLI.LLI = ((unsigned int)&lli[i+1]|m3_sram_base)>>2;
		lli[i].DMACCLLI.LM = 1;
		lli[i].DMACCControl.transferSize = MIN(num_words, 4095);
		lli[i].DMACCControl.sourceWidth = DMA_WORD_WIDTH;
		lli[i].DMACCControl.destinationWidth = DMA_HALFWORD_WIDTH;
		lli[i].DMACCControl.sourceMaster = 1;
		lli[i].DMACCControl.destinationMaster = 1;
		lli[i].DMACCControl.sourceAddressIncrement = 1;
		lli[i].DMACCControl.destinationAddressIncrement = 0;
		lli[i].DMACCControl.priviledgeMode = 1;
		lli[i].DMACCControl.terminalCounterInterruptEnabled = 0;
		pcm_data += lli[i].DMACCControl.transferSize*width;
		num_words -= lli[i].DMACCControl.transferSize;
	}
	if (!i)
		return -1;
	lli[i - 1].DMACCLLI.LLI = 0;
	lli[i - 1].DMACCControl.terminalCounterInterruptEnabled = 1;

	dma0_ch->DMACCSrcAddr = lli[0].DMACCSrcAddr;
	dma0_ch->DMACCDestAddr = lli[0].DMACCDestAddr;
	dma0_ch->DMACCLLI = lli[0].DMACCLLI;
	dma0_ch->DMACCControl = lli[0].DMACCControl;

	dma0_ch->DMACCConfiguration.flowControl
				    = MEMORY_TO_PERIPHERAL_DMA_CONTROLLER;
	dma0_ch->DMACCConfiguration.interruptErrorMaskEnable = 1;
	dma0_ch->DMACCConfiguration.terminalCountMaskEnable = 1;
	dma0_ch->DMACCConfiguration.destinationPeripheral = DMA0_MSP2_TX;
	dma0_ch->DMACCConfiguration.enabled = 1;

	/* MSP2 */
	msp2_regs->msp_cgr.bit.tcksel = 1; /* master */
	msp2_regs->msp_cgr.bit.tckpol = 1; /* falling */
	msp2_regs->msp_cgr.bit.tfssel = 3; /* master */
	msp2_regs->msp_cgr.bit.tfspol = 1; /* active low */
	msp2_regs->msp_cgr.bit.tffen = 1;  /* fifo enabled */
#ifdef TDM8
	msp2_regs->msp_srg.bit.sckdiv =
		    DIV_ROUND_CLOSEST(mspclk(), w->sample_per_sec * 128) - 1;
	msp2_regs->msp_tcf.bit.tp1flen = 7; /* 8 element */
	msp2_regs->msp_tcf.bit.tp1elen = 4; /* 16-bit */
	msp2_regs->msp_srg.bit.frper = 127; /* frame 128bit */
	msp2_regs->msp_srg.bit.frwid = 0; /* 1 bit pulse */
	msp2_regs->msp_mcr.bit.tmcen = 1;
	msp2_regs->msp_tce[0] = 1;
	/* SAI2 */
	aif_regs->sai2csr.bit.syn = 2; /* FSYNC 1st bit */
	aif_regs->sai2csr.bit.cnt = 3; /* 8w */
#else
	msp2_regs->msp_srg.bit.sckdiv =
		    DIV_ROUND_CLOSEST(mspclk(),	w->sample_per_sec * 32) - 1;
	msp2_regs->msp_tcf.bit.tp1flen = 0; /* 1 element */
	msp2_regs->msp_tcf.bit.tp1elen = 7; /* 32-bit */
	msp2_regs->msp_srg.bit.frper = 31; /* frame 32bit */
	msp2_regs->msp_srg.bit.frwid = 15; /* 16 bit pulse */
	/* SAI2 */
	aif_regs->sai2csr.bit.syn = 0; /* FSYNC half */
	aif_regs->sai2csr.bit.cnt = 0; /* 2w */
#endif
	aif_regs->sai2csr.bit.ena = 1;
	msp2_regs->msp_dmacr.bit.tdmae = 1;
	msp2_regs->msp_cgr.bit.sgen = 1;
	msp2_regs->msp_cgr.bit.fgen = 1;
	msp2_regs->msp_cgr.bit.txen = 1;   /* tx enabled */

	audio_data.msg_id = EARLY_AUDIO_PLAY;
	audio_data.arg = (uint32_t)msp2_regs;
	msg_audio_send((void *)&audio_data);

	return 0;
}

static int aux_detect_hdl(void)
{
	uint32_t msg_id = 0;
	portBASE_TYPE xHigherPriorityTaskWoken;

	if (!sta_audio.aux_queue)
		return -ENOENT;

	if (xQueueGenericSendFromISR(sta_audio.aux_queue, &msg_id,
				     &xHigherPriorityTaskWoken,
				     queueSEND_TO_BACK))
		return -1;

	if (xHigherPriorityTaskWoken)
		taskYIELD();

	return 0;
}

static void aux_detect_task(struct audio_context *audio_context)
{
	struct gpio_config aux_config;
	int val, val_prev, aux_detect = -1;
	int debounce;
	uint32_t msg_id = 0;

	sta_audio.aux_queue = xQueueCreate(1, sizeof(uint32_t));
	if (!sta_audio.aux_queue)
		goto end_aux;

	/* Configure the aux detect GPIO in interrupt mode */
	aux_config.direction = GPIO_DIR_INPUT;
	aux_config.mode	= GPIO_MODE_SOFTWARE;
	aux_config.level = GPIO_LEVEL_LEAVE_UNCHANGED;
	aux_config.trig = GPIO_TRIG_BOTH_EDGES;

	gpio_set_pin_config(audio_context->aux_detect, &aux_config);
	if (gpio_request_irq(audio_context->aux_detect, aux_detect_hdl)) {
		TRACE_ERR("%s: failed to request irq\n", __func__);
		goto end_aux;
	}

	do {
		if (msg_id == 0xdead)
			break;

		gpio_read_gpio_pin(audio_context->aux_detect, &val);
		val_prev = val;
		debounce = 0;
		do {
			vTaskDelay(pdMS_TO_TICKS(10));
			gpio_read_gpio_pin(audio_context->aux_detect, &val);
			debounce++;
			if (val != val_prev) {
				debounce = 0;
				val_prev = val;
			}
		} while (debounce < 30);

		if (aux_detect != (val == GPIO_DATA_LOW)) {
			aux_detect = (val == GPIO_DATA_LOW);
			if (aux_detect)
				early_audio_init("auxmedia");
			else
				early_audio_init("sai4rx1fm");
		}
	} while (xQueueReceive(sta_audio.aux_queue, &msg_id, portMAX_DELAY));
end_aux:
	if (sta_audio.aux_queue)
		vQueueDelete(sta_audio.aux_queue);
	vTaskDelete(NULL);
}

static void chime_task(struct audio_context *audio_context)
{
	do {
		toggle_gpio();
		early_audio_exec("chime");

		vTaskDelay(pdMS_TO_TICKS(400));
	} while(1);

	vTaskDelete(NULL);
}

static void volume_task(struct audio_context *audio_context)
{
	int step = 10, ret;

	sta_audio.vol_queue = xQueueCreate(10, sizeof(uint32_t));
	if (!sta_audio.vol_queue)
		goto end_vol;

	ret = rotary_enable(sta_audio.vol_queue);
	if (ret < 0)
		goto end_vol;

	while (xQueueReceive(sta_audio.vol_queue, &step, portMAX_DELAY)) {
		if (step == 0xdead)
			break;

		set_volume("gains_out", sta_audio.vol += (step * 10));
	}

end_vol:
	rotary_disable(sta_audio.vol_queue);

	if (sta_audio.vol_queue)
		vQueueDelete(sta_audio.vol_queue);
	vTaskDelete(NULL);
}

int sta_audio_play_pcm(int file_no)
{
	portBASE_TYPE xHigherPriorityTaskWoken;
	int i = 5;

	if (!features.pcm_play)
		return -1;

	while (!sta_audio.pcm_queue){
		if (0 >= i--)
			return -ENOENT;
		vTaskDelay(pdMS_TO_TICKS(100));
	}

	if (xQueueGenericSendFromISR(sta_audio.pcm_queue, &file_no,
					 &xHigherPriorityTaskWoken,
					 queueSEND_TO_BACK))
		return -1;

	if (xHigherPriorityTaskWoken)
		taskYIELD();

	return 0;
}

void pcm_interrupt_handler(int status)
{
	TRACE_NOTICE("end pcm %d\n", status);
}

static void pcm_file_task(struct audio_context *audio_context)
{
	uint32_t		msg_id;
	struct audio_data audio_data;

	dma0_request(PCM_DMA_CHAN, pcm_interrupt_handler);
	sta_audio.pcm_queue = xQueueCreate(1, sizeof(uint32_t));
	if (!sta_audio.pcm_queue)
		goto end_pcm;

	while (xQueueReceive(sta_audio.pcm_queue, &msg_id, portMAX_DELAY)) {
		switch (msg_id) {
		case 1:
			msp2dma(audio_context->w1);
			break;
		case 2:
			msp2dma(audio_context->w2);
			break;
		case 0:
			msp2dma(NULL);
			break;
		case 0xdead:
			goto end_pcm;
			break;
		}
	}
end_pcm:
	audio_data.msg_id = EARLY_AUDIO_END_PLAY;
	msg_audio_send((void *)&audio_data);
	if (sta_audio.pcm_queue)
		vQueueDelete(sta_audio.pcm_queue);
	vTaskDelete(NULL);
}

/**
 * @brief	Initialize audio
 * @return	None
 */
void early_audio_task(struct audio_context *audio_context)
{
	int ret = 0;
	int dsp;
	
	sta_audio.audiolib_addr = audio_context->audiolib_addr;

	TRACE_NOTICE("\nearly audio");

	TRACE_INFO("Start DSP..\n");
	/* Remove DSPx from reset if enable */
	for (dsp = DSP0; dsp < DSP_MAX_NO; dsp++) {
		if (audio_context->have_dsp[dsp]) {
			dsp_start(dsp);
			TRACE_NOTICE("\nDSP%d STARTED", dsp);
			dsp_clean(dsp);
		}
	}
	TRACE_NOTICE("\n");

	ret = msg_audio_init(sizeof(struct audio_data));
	if (ret)
		goto end_task;

	audio_status_init();

	/* Loop till the DAC is unmuted */
	if (sta_audio.dac_mtu) {
		while (mtu_read_timer_value(sta_audio.dac_mtu))
			vTaskDelay(pdMS_TO_TICKS(1));
		mtu_stop_timer(sta_audio.dac_mtu);
	}

	pinmux_request("sai4_mux");

	if (features.aux_detect) {
		ret = xTaskCreate((pdTASK_CODE) aux_detect_task,
				(char *)"AUX detect", 130,
				audio_context, TASK_PRIO_MAX, NULL);
		if (ret != pdPASS)
			goto end_task;
	}
	else {
		early_audio_init("sai4rx1fm");
	}

	if (features.chime) {
		ret = xTaskCreate((pdTASK_CODE) chime_task,
				(char *)"Chime", 50,
				audio_context, TASK_PRIO_MAX, NULL);
		if (ret != pdPASS)
			goto end_task;
	}
	if (features.pcm_play) {
		ret = xTaskCreate((pdTASK_CODE) pcm_file_task,
				(char *)"PCM file", 126,
				audio_context, TASK_PRIO_MAX, NULL);
		if (ret != pdPASS)
			goto end_task;

		sta_audio_play_pcm(1);
		msg_audio_register(EARLY_AUDIO_END_PLAY, sta_audio_play_pcm);
		msg_audio_register(EARLY_AUDIO_END_PCM, early_audio_end_pcm);
	}
	if (features.volume) {
		ret = xTaskCreate((pdTASK_CODE) volume_task,
				  (char *)"Volume", 120,
				   audio_context, TASK_PRIO_MAX, NULL);
		if (ret != pdPASS)
			goto end_task;
	}
	msg_audio_register(EARLY_AUDIO_SOURCE, early_audio_source);
	msg_audio_register(EARLY_AUDIO_VOLUME, early_audio_end_volume);
	msg_audio_register(EARLY_AUDIO_BACKUP_VOL, early_audio_backup_volume);
	msg_audio_register(EARLY_AUDIO_BACKUP_API, early_audio_backup_api);
end_task:
	vTaskDelete(NULL);
}

