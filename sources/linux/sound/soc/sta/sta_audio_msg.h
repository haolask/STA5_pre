/*
 * Copyright (C) ST Microelectronics 2017
 *
 * Author:	Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */
#ifndef ST_AUDIO_MSG_H
#define ST_AUDIO_MSG_H

enum audio_msg {
	EARLY_AUDIO_SOURCE = 0xea000001,
	EARLY_AUDIO_PLAY,
	EARLY_AUDIO_END_PLAY,
	EARLY_AUDIO_END,
	EARLY_AUDIO_END_PCM,
	EARLY_AUDIO_VOLUME,
	EARLY_AUDIO_BACKUP_API,
	EARLY_AUDIO_BACKUP_VOL,
};

struct audio_data {
	u32 msg_id;
	u32 id;
	u32 arg;
	char desc[32];
};

typedef int (*msg_audio_cb_t)(struct audio_data *, void *);

int st_codec_early_audio_send(enum audio_msg msg);
int st_codec_early_audio_send_args(enum audio_msg m, char *n, u32 v, u32 i);
int msg_audio_register(u32 msg_id, msg_audio_cb_t cb, void *arg);

#endif
