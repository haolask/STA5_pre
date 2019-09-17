#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "utils.h"
#include "queue.h"
#include "sta_rpmsg.h"
#include "MessageQCopy.h"
#include "NameMap.h"
#include "sta_common.h"
#include "sta_rpmsg_audio.h"

static struct msg_task_info msg_task;

/**
 * @brief	Initialize rpmsg for early audio
 * @return	None
 */
static void rpmsg_audio_task(void *p)
{
	struct msg_task_info *h = &msg_task;
	MessageQCopy_Handle rpmsg_rx;
	uint32_t myEndpoint, remoteEndpoint;
	uint16_t len;
	int i;
	int n = sizeof(h->hdl) / sizeof(h->hdl[0]);
	int ret = 0;
	struct audio_data *a;

	if (RPMSG_Init() != 0)
		goto end_rpmsg;

	rpmsg_rx = MessageQCopy_create(MessageQCopy_ASSIGN_ANY, &myEndpoint);
	if (!rpmsg_rx) {
		TRACE_ERR("%s MessageQCopy_create failure\n", __func__);
		goto end_rpmsg;
	}
	NameMap_register("early-audio", myEndpoint);
	while (1) {
		len = h->size;
		ret = MessageQCopy_recv(rpmsg_rx, h->msg,
					&len, &remoteEndpoint, 200);
		if (ret == MessageQCopy_S_SUCCESS) {
			a = (struct audio_data *)h->msg;
			for (i = 0; i < n; i++) {
				if (h->hdl[i].msg_id == a->msg_id &&
				    h->hdl[i].cb)
					h->hdl[i].cb(a);
			}
		}

		if (xQueueReceive(h->to_host, h->msg, 200))
			MessageQCopy_send(remoteEndpoint, myEndpoint, h->msg, h->size, 100);
	}

end_rpmsg:
	vTaskDelete(NULL);
}

/**
 * @brief	Allocate rpmsg task
 * @return	None
 */
int msg_audio_init(int size)
{
	int ret;
	struct msg_task_info *h = &msg_task;

	h->size = size;
	h->msg = pvPortMalloc(size);
	if (!h->msg)
		return -ENOMEM;
	h->to_host = xQueueCreate(4, size);
	if (!h->to_host)
		goto msg_audio_err;

	ret = xTaskCreate((pdTASK_CODE) rpmsg_audio_task,
			  (char *)"Rpmsg audio", 120,
			  (void *)h, TASK_PRIO_MAX, NULL);
	if (ret != pdPASS)
		goto msg_audio_err;

	return 0;

msg_audio_err:
	if (h->to_host)
		vQueueDelete(h->to_host);
	return -1;
}

int msg_audio_register(uint32_t msg_id,	int (*cb)(struct audio_data *))
{
	int i;
	struct msg_task_info *h = &msg_task;
	int n = sizeof(h->hdl) / sizeof(h->hdl[0]);

	for (i = 0; i < n; i++) {
		if (!h->hdl[i].cb || h->hdl[i].msg_id == msg_id) {
			h->hdl[i].cb = cb;
			h->hdl[i].msg_id = msg_id;
			break;
		}
	}
	if (i == n)
		return -1;
	return 0;
}

int msg_audio_send(void *buffer)
{
	struct msg_task_info *h = &msg_task;

	return xQueueSend(h->to_host, buffer, 0);
}

void msg_audio_end(void)
{
	struct msg_task_info *h = &msg_task;

	if (h->task)
		vTaskDelete(h->task);
	if (h->msg)
		vPortFree(h->msg);
	if (h->to_host)
		vQueueDelete(h->to_host);
}

