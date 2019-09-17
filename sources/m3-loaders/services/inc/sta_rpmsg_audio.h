/**
 * @file sta_rpmsg_audio.h
 * @brief audio messages
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __STA_RPMSG_AUDIO_H__
#define __STA_RPMSG_AUDIO_H__

struct audio_data {
	uint32_t msg_id;
	uint32_t id;
	uint32_t arg;
	char desc[32];
};

typedef struct {
	uint32_t msg_id;
	int (*cb)(struct audio_data *);
} t_msg_audio_hdl;

struct msg_task_info {
	QueueHandle_t to_host, from_host;
	TaskHandle_t task;
	int size;
	void *msg;
	t_msg_audio_hdl hdl[10];
};

int msg_audio_init(int size);
void msg_audio_end(void);
int msg_audio_send(void *buffer);
int msg_audio_register(uint32_t msg_id, int (*cb)());

#endif /* __STA_RPMSG_AUDIO_H__ */
