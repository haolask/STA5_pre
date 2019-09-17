/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_rpmsg_test.c
* Author             : APG-MID Application Team
* Date First Issued  : 03/13/2014
* Description        : This file provides RPMSG test functions.
********************************************************************************
* History:
* 03/13/2014: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Platform includes */
#include "trace.h"
#include "sta_rpmsg.h"
#include "sta_can_test.h"
#include "sta_acc.h"
#include "MessageQCopy.h"
#include "NameMap.h"
#include "sta_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************/
/** Defines declaration **/
/*************************/
#define WARN_MSG        ("WARNING: Unexpected Rx message")
#define WARN_MSG_LEN    (sizeof(WARN_MSG) - 1)
#define ERROR_MSG       ("Oooops:Problem!")
#define ERROR_MSG_LEN   (sizeof(ERROR_MSG) - 1)
#define SUCCESS_MSG     ("Yooopiii:Successfull!")
#define SUCCESS_MSG_LEN (sizeof(SUCCESS_MSG) - 1)
#define LINUX_MSG       ("Remoteproc SRC Up!")
#define LINUX_MSG_LEN   (sizeof(LINUX_MSG) - 1)
#define MAX_MSG_LEN     (LINUX_MSG_LEN + SUCCESS_MSG_LEN)


/***************************/
/** Variables declaration **/
/***************************/

struct rpmsg_css_ctx {
	xList can_notifier_list;
	xList acc_notifier_list;
};

struct rpmsg_css_notifier {
	xListItem list_item;
	uint32_t endpoint;
};

struct rpmsg_css_ctx rc_ctx;
static char buf[RP_MSG_PAYLOAD_SIZE];

/*************************/
/** Function definition **/
/*************************/

static void notify_can_users(struct feature_set *fs, uint32_t myEndpoint, char *buf)
{
	struct css_msg_hdr *hdr;
	struct rpmsg_css_notifier *notifier, *first_notifier;
	xList *notifier_list;
	struct can_frame f;
	uint16_t len;

	if (!fs->can0)
		return;
	notifier_list = &rc_ctx.can_notifier_list;
	if (listLIST_IS_EMPTY(notifier_list))
		return;
	if (!can_get_frame(&f))
		return;

	hdr = (struct css_msg_hdr *)buf;
	hdr->type = CSS_CAN_RX;
	hdr->len = sizeof(struct can_frame);
	memcpy(hdr->data, &f, sizeof(struct can_frame));
	len = sizeof(*hdr) + hdr->len;

	listGET_OWNER_OF_NEXT_ENTRY(first_notifier, notifier_list);
	MessageQCopy_send(first_notifier->endpoint, myEndpoint,
		(void *)buf, len, portMAX_DELAY);

	do {
		listGET_OWNER_OF_NEXT_ENTRY(notifier, notifier_list);
		if (notifier == first_notifier)
			break;

		MessageQCopy_send(notifier->endpoint, myEndpoint,
			(void *)buf, len, portMAX_DELAY);
	} while (1);
}

static void notify_acc_users(struct feature_set *fs, uint32_t myEndpoint, char* buf)
{
	struct css_msg_hdr *hdr;
	struct rpmsg_css_notifier *notifier, *first_notifier;
	xList *notifier_list;
	int s; /* in bytes */
	uint16_t len;

	if (!fs->accelerometer)
		return;
	notifier_list = &rc_ctx.acc_notifier_list;
	if (listLIST_IS_EMPTY(notifier_list))
		return;

	hdr = (struct css_msg_hdr *)buf;
	s = acc_on_tap_get_xyz_datas((uint16_t *)(hdr->data + sizeof(int)));
	if (s <= 0)
		return;
	*((int *)hdr->data) = s;
	hdr->type = CSS_ACC_RX;
	hdr->len = s + sizeof(int);
	len = sizeof(*hdr) + s + sizeof(int);

	listGET_OWNER_OF_NEXT_ENTRY(first_notifier, notifier_list);
	MessageQCopy_send(first_notifier->endpoint, myEndpoint,
		(void *)buf, len, portMAX_DELAY);

	do {
		listGET_OWNER_OF_NEXT_ENTRY(notifier, notifier_list);
		if (notifier == first_notifier)
			break;

		MessageQCopy_send(notifier->endpoint, myEndpoint,
			(void *)buf, len, portMAX_DELAY);
	} while (1);
}

static void add_rpmsg_css_notifier(xList *notifier_list, uint32_t endpoint)
{
	struct rpmsg_css_notifier *notifier, *first_notifier;

	if (listLIST_IS_EMPTY(notifier_list))
		goto add_notifier;
	else {
		listGET_OWNER_OF_NEXT_ENTRY(first_notifier, notifier_list);
		if (first_notifier->endpoint == endpoint)
				return;
		do {
			listGET_OWNER_OF_NEXT_ENTRY(notifier,
				notifier_list);
			if (notifier->endpoint == endpoint)
				return;
		}
		while (notifier != first_notifier);
	}
add_notifier:
	notifier = (struct rpmsg_css_notifier *)
		pvPortMalloc(sizeof (struct rpmsg_css_notifier));
	if (!notifier) {
		TRACE_ERR("%s: failed to allocate notifier\n");
		return;
	}
	notifier->endpoint = endpoint;
	vListInitialiseItem(&(notifier->list_item));
	listSET_LIST_ITEM_OWNER(&(notifier->list_item), notifier);
	vListInsertEnd(notifier_list, &(notifier->list_item));
}

/*******************************************************************************
* Function Name  : RpmsgTestTask
* Description    : RPMSG Test task routine
* Input          : None
* Return         : None
*******************************************************************************/
void RpmsgTestTask(void *p)
{
	//portTickType        lastWake;
	MessageQCopy_Handle handle;
	int ret;
	uint32_t requestEndpoint = MessageQCopy_ASSIGN_ANY;
	uint32_t myEndpoint = 0;
	uint32_t remoteEndpoint;
	uint16_t len;
	struct css_msg_hdr *hdr;
	struct feature_set *fs = (struct feature_set *)p;

	TRACE_INFO("%s start\n", __func__);

	/** RPMSG framework initialisation procedure **/
	if (RPMSG_Init() != 0)
	{
		TRACE_ERR("%s init failure\n", __func__);
		goto end;
	}

	/* initialise CAN & ACCELEROMETER notifier lists */
	if (fs->can0)
		vListInitialise(&rc_ctx.can_notifier_list);
	if (fs->accelerometer)
		vListInitialise(&rc_ctx.acc_notifier_list);

	/* 1st argument must be less than MessageQCopy_MAX_RESERVED_ENDPOINT */
	/* or MessageQCopy_ASSIGN_ANY.                                       */
	handle = MessageQCopy_create(requestEndpoint, &myEndpoint);
	if (!handle) {
		TRACE_ERR("%s MessageQCopy_create failure\n", __func__);
		goto end;
	}

	NameMap_register("sta-rpmsg-CSS", myEndpoint);

	//lastWake = xTaskGetTickCount();
	while (1)
	{
		/* Set maximum data size that I can receive. */
		len = RP_MSG_PAYLOAD_SIZE;

		ret = MessageQCopy_recv(handle, (void *)buf, &len, &remoteEndpoint, portMAX_DELAY);

		/* MessageQCopy_recv was succeeded? */
		if (ret == MessageQCopy_S_SUCCESS && len <= RP_MSG_PAYLOAD_SIZE)
		{
			hdr = (struct css_msg_hdr *)buf;
			switch (hdr->type)
			{
			case CSS_CTRL_REQ:
				{
					struct css_ctrl_req *ctrlReq = (struct css_ctrl_req *)hdr->data;
					struct css_ctrl_rsp *ctrlRsp = (struct css_ctrl_rsp *)hdr->data;
					/* Format Ctrl message response */
					hdr->type = CSS_CTRL_RSP;
					hdr->len = sizeof(*ctrlRsp);
					len = sizeof(*hdr) + hdr->len;

					switch (ctrlReq->request)
					{
					case CSS_CONNECTION:
					case CSS_DISCONNECT:
						ctrlRsp->status = CSS_SUCCESS;
						ctrlRsp->addr = myEndpoint;
						break;

					default:
						TRACE_ERR("RpmsgTestTask: Wrong control request: %d\n", ctrlReq->request);
						ctrlRsp->status = CSS_ERROR;
						ctrlRsp->addr = myEndpoint;
						break;
					}
				}
				break;

			case CSS_RAW_MSG:
				/* For test purpose loop back received message to host */
				/* So just do nothing by sending back the msg as it is */
				///TRACE_INFO("RpmsgTestTask: Rx RAW_MSG: %s\n", &hdr->data[16]);
				break;
			case CSS_CAN_LISTEN:
				if (fs->can0)
					add_rpmsg_css_notifier(&rc_ctx.can_notifier_list,
						remoteEndpoint);
				break;
			case CSS_ACC_LISTEN:
				if (fs->accelerometer)
					add_rpmsg_css_notifier(&rc_ctx.acc_notifier_list,
						remoteEndpoint);
				break;;
			default:
				TRACE_ERR("RpmsgTestTask: Wrong Msg type: %d\n", hdr->type);
				len = WARN_MSG_LEN;
				memcpy(buf, WARN_MSG, len);
				break;
			}
			if (hdr->type != CSS_CAN_LISTEN &&
				hdr->type != CSS_ACC_LISTEN)
				MessageQCopy_send(remoteEndpoint, myEndpoint,
					(void *)buf, len, portMAX_DELAY);
		}
		/* Check if there is some remote css users to notify */
		notify_can_users(fs, myEndpoint, buf);
		notify_acc_users(fs, myEndpoint, buf);
	}
end:
	vTaskDelete(NULL);
}

/*******************************************************************************
* Function Name  : RpmsgSrcTask
* Description    : RPMSG SRC test task routine
* Input          : None
* Return         : None
*******************************************************************************/
void RpmsgSrcTask()
{
	portTickType        lastWake;
	MessageQCopy_Handle handle;
	int                 ret;
	uint32_t				requestEndpoint = MessageQCopy_ASSIGN_ANY;
	uint32_t              myEndpoint = 0;
	uint32_t              remoteEndpoint;
	uint16_t              len;
	/* Max data length of rpmsg send/recv is RP_MSG_PAYLOAD_SIZE. */
	/* rpmsg-client-sample Linux module for test pupose send few  */
	/* bytes at once, so this function reserve MAX_MSG_LEN buffer.*/
	char                buffer[MAX_MSG_LEN];

	TRACE_INFO("FreeRTOS RMPSG SRC task\n");

	/** RPMSG framework initialisation procedure **/
	if (RPMSG_Init() != 0)
	{
		TRACE_ERR("RpmsgSrcTask init failure\n");
		goto end;
	}

	/* 1st argument must be less than MessageQCopy_MAX_RESERVED_ENDPOINT */
	/* or MessageQCopy_ASSIGN_ANY.                                       */
	handle = MessageQCopy_create(requestEndpoint, &myEndpoint);
	NameMap_register("sta-rpmsg-CSS-SRC", myEndpoint);

	memset(buffer, 0, sizeof(buffer));

	lastWake = xTaskGetTickCount();
	vTaskDelayUntil(&lastWake, 15000);

	while (1)
	{
		/* Set maximum data size that I can receive. */
		len = (uint16_t)sizeof(buffer);

		ret = MessageQCopy_recv(handle, (void *)buffer, &len, &remoteEndpoint, portMAX_DELAY);
		/* MessageQCopy_recv was succeeded? */
		if (ret == MessageQCopy_S_SUCCESS &&
				len == LINUX_MSG_LEN &&
				memcmp(buffer, LINUX_MSG, LINUX_MSG_LEN) == 0)
		{
			len = SUCCESS_MSG_LEN;
			memcpy(buffer, SUCCESS_MSG, len);
		}
		else
		{
			len = ERROR_MSG_LEN;
			memcpy(buffer, ERROR_MSG, len);
		}

		/** Send back msg answer to host **/
		vTaskDelayUntil(&lastWake, 200);
		MessageQCopy_send(remoteEndpoint, myEndpoint, (void *)buffer, len, portMAX_DELAY);
	}
end:
	vTaskDelete(NULL);
}

#ifdef __cplusplus
}
#endif

