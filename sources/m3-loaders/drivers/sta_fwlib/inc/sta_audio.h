/**
 * @file sta_audio.h
 * @brief This file provides all the Audio driver definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include "sta_dsp.h"

typedef enum {
	DEFAULT_MODE,
	MSP_LOOPBACK_MODE,
	SAI4_LOOPBACK_MODE,
	SAI4_BYPASS,
	SAI4_DMABUS,
} Audio_Mode;

struct audio_context {
	bool have_dsp[DSP_MAX_NO];
	void *audiolib_addr;
	uint8_t aux_detect;
	void *w1, *w2;
};

int sta_audio_play_pcm(int file_no);
void dac_power_on();
int early_src_init(void);

#endif  /* _AUDIO_H_ */


