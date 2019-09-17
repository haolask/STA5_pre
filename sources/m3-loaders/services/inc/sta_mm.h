#if !defined STA_MM_H
#define STA_MM_H

#include "semphr.h"

enum mm_state_machine {
	MM_STATE_IDLE,
	MM_STATE_ANIM,
	MM_STATE_RVC
};

enum mm_event {
	MM_EVT_RVC_RGE,
	MM_EVT_RVC_RGD,
	MM_EVT_RVC_STARTED,
	MM_EVT_RVC_STOPPED,
	MM_EVT_RVC_TAKEOVER,
	MM_EVT_ANIM_START,
	MM_EVT_ANIM_STARTED,
	MM_EVT_ANIM_STOP,
	MM_EVT_ANIM_STOPPED,
};

struct sta_mm_ctx {
	enum mm_state_machine state;
	void *anim_shadow_addr;
	uint32_t anim_size;
	void *splash_shadow_addr;
	QueueHandle_t mm_queue;
	bool linux_ready;
};

int sta_mm_send_event(int event);

#endif
