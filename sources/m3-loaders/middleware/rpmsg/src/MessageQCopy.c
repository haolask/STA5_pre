/* This file is based on SYS/BIOS RPMsg code.
 *
 * Repositories:
 *  http://git.omapzoom.org/?p=repo/sysbios-rpmsg.git;a=summary
 *
 * The original license terms are as follows.
 */
/*
 * Copyright (c) 2011-2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** ============================================================================
 *  @file       MessageQCopy.c
 *
 *  @brief      A simple copy-based MessageQ, to work with Linux virtio_rp_msg.
 *
 *  ============================================================================
 */

#include <string.h>

//#include "FreeRTOSConfig.h"
//#include "portmacro.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "list.h"

#include "MessageQCopy.h"
#include "sta_rpmsg.h"

/* =============================================================================
 * Structures & Enums
 * =============================================================================
 */

/* Various arbitrary limits: */
#define MAXMESSAGEQOBJECTS  256
#define QUE_LEN	32

/* Message Header: Must match mp_msg_hdr in virtio_rp_msg.h on Linux side. */
typedef struct MessageQCopy_MsgHeader {
    uint32_t srcAddr;                 /* source endpoint addr               */
    uint32_t dstAddr;                 /* destination endpoint addr          */
    uint32_t reserved;                /* reserved                           */
    uint16_t dataLen;                 /* data length                        */
    uint16_t flags;                   /* bitmask of different flags         */
    uint8_t  payload[];               /* Data payload                       */
} __attribute__((packed)) MessageQCopy_MsgHeader;

typedef MessageQCopy_MsgHeader *MessageQCopy_Msg;

/* The MessageQCopy Object */
typedef struct MessageQCopy_Object {
	xQueueHandle qHndl;
} MessageQCopy_Object;

typedef struct {
	uint16_t token;
	uint16_t dataLen;
	uint32_t srcAddr;
	uint8_t  *payload;
} Msgq_data;

static VirtQueue_Handle vQueFromHost;
static VirtQueue_Handle vQueToHost;
static VirtQueue_Entry vEntry[RP_NUM_VRING]; /* Tx and Rx Vring entry (and in this order) */
static MessageQCopy_Object mQueObj[MAXMESSAGEQOBJECTS];
static xList availBufList;
static volatile unsigned int waitAvaileBuf = 0;

static SemaphoreHandle_t MessageQCopy_sem;

#define MessageQCopy_enter_critical() \
{ \
	xSemaphoreTake(MessageQCopy_sem, portMAX_DELAY); \
	RPMSG_disable_rx(); \
}

#define MessageQCopy_exit_critical() \
{ \
	RPMSG_enable_rx(); \
	xSemaphoreGive(MessageQCopy_sem); \
}

#define IS_VALID_MSGQUEOBJ(obj) ((obj)->qHndl != NULL)


static bool MsgQueObjInit(MessageQCopy_Object *obj)
{
	obj->qHndl = xQueueCreate(QUE_LEN, sizeof(Msgq_data));
	return (obj->qHndl != NULL);
}

static bool MessageQCopy_SendToTask(uint16_t token,
									MessageQCopy_Msg msg)
{
    portBASE_TYPE xHigherPriorityTaskWoken;
	Msgq_data     data;
	uint32_t        endPnt;
	portBASE_TYPE ret;

	data.token = token;
	data.dataLen = msg->dataLen;
	data.srcAddr = msg->srcAddr;
	data.payload = msg->payload;

	if((endPnt = msg->dstAddr) >= MAXMESSAGEQOBJECTS ||
			!IS_VALID_MSGQUEOBJ(&mQueObj[endPnt])){
		return false;
	}

	ret = xQueueSendToBackFromISR(mQueObj[endPnt].qHndl, &data, &xHigherPriorityTaskWoken);
	return (ret);
}

static void callback_FromHost(VirtQueue_Handle vq)
{
    int16_t            token;
    MessageQCopy_Msg msg;
    bool             usedBufAdded = false;
    int              len;

	/* Process all available buffers: */
	while((token = VirtQueue_getAvailBuf(vq, (void **)&msg, &len)) >= 0){
		if(!MessageQCopy_SendToTask(token, msg)){
			VirtQueue_addUsedBuf(vq, token, RP_MSG_BUF_SIZE);
			usedBufAdded = true;
		}
	}

	if(usedBufAdded){
		/* Tell host we've processed the buffers: */
		VirtQueue_kick(vq);
	}
}

static void callback_ToHost(VirtQueue_Handle vq)
{
	uint16_t avail;

	if(listLIST_IS_EMPTY(&availBufList) != pdFALSE){
		return;
	}

	if((avail = GET_AVAIL_COUNT(vq)) == 0){
		return;
	}

	do{
		//signed portBASE_TYPE ret;

		/* Because this function is not an application code,             */
		/* there is not the problem with using xTaskRemoveFromEventList. */
		xTaskRemoveFromEventList(&availBufList);
	}
	while(--avail > 0);
}


/* =============================================================================
 *  MessageQCopy Functions:
 * =============================================================================
 */

/*
 *  ======== MessasgeQCopy_init ========
 */
int MessageQCopy_init(VirtQueue_RscTable *rsc)
{
	const VirtQueue_RscTable *rscTab = resources;     /* Set to default resource table */
	int i;

	if (!MessageQCopy_sem) {
		MessageQCopy_sem = xSemaphoreCreateBinary();
		xSemaphoreGive(MessageQCopy_sem);
	}

	for(i = 0; i < MAXMESSAGEQOBJECTS; i++){
		mQueObj[i].qHndl = NULL;
	}

	vListInitialise(&availBufList);

    /* Use received resource table from Linux CPU instead of the default one */
    /* in order to get VirtQueue configuration entries */
    if (rsc)
        rscTab = rsc;
    if (VirtQueue_getRscEntry(rscTab, vEntry) != 0)
		return MessageQCopy_E_FAIL;


    /* Initialize Transport related objects: */
    /*
     * Note: order of these calls determines the virtqueue indices identifying
     * the vrings toHost and fromHost:  toHost is first!
     */
	vQueToHost = VirtQueue_create(callback_ToHost, &vEntry[RP_ID_TX_VRING]);
	vQueFromHost = VirtQueue_create(callback_FromHost, &vEntry[RP_ID_RX_VRING]);
	if(vQueToHost == NULL || vQueFromHost == NULL){
		return MessageQCopy_E_MEMORY;
	}

	VirtQueue_startup();
	return MessageQCopy_S_SUCCESS;
}

/*
 *  ======== MessageQCopy_create ========
 */
MessageQCopy_Handle MessageQCopy_create(uint32_t reserved, uint32_t * endpoint)
{
	MessageQCopy_Object *obj = NULL;
	bool                found = false;
	int                 i;
	uint16_t              queueIndex = 0;

	MessageQCopy_enter_critical();

	if(reserved == MessageQCopy_ASSIGN_ANY) {
		/* Search the array for a free slot above reserved: */
		for(i = MessageQCopy_MAX_RESERVED_ENDPOINT + 1;
			(i < MAXMESSAGEQOBJECTS) && (found == false); i++) {

			if(!IS_VALID_MSGQUEOBJ(&mQueObj[i])) {
				queueIndex = i;
				found = true;
				break;
			}
		}
	}
	else if((queueIndex = reserved) <= MessageQCopy_MAX_RESERVED_ENDPOINT) {
		if(!IS_VALID_MSGQUEOBJ(&mQueObj[queueIndex])) {
			found = true;
		}
	}

	if(found) {
		if(MsgQueObjInit(&mQueObj[queueIndex]) != false) {
			obj = &mQueObj[queueIndex];
			*endpoint = queueIndex;
		}
	}

	MessageQCopy_exit_critical();
	return obj;
}

/*
 *  ======== MessageQCopy_recv ========
 */
int MessageQCopy_recv(MessageQCopy_Handle handle,
								void * data,
								uint16_t *len,
								uint32_t *rplyEndpt,
								portTickType timeout)
{
	Msgq_data     mdata;
	portBASE_TYPE qret;
	int           ret;

	qret = (int)xQueueReceive(handle->qHndl, &mdata, timeout);

	if(qret == (int)pdPASS){
		if(*len > mdata.dataLen){
			*len = mdata.dataLen;
		}
		memcpy(data, mdata.payload, *len);
		*rplyEndpt = mdata.srcAddr;

		MessageQCopy_enter_critical();
		VirtQueue_addUsedBuf(vQueFromHost, mdata.token, RP_MSG_BUF_SIZE);
		VirtQueue_kick(vQueFromHost);
		MessageQCopy_exit_critical();

		ret = MessageQCopy_S_SUCCESS;
	}
	else{
		ret = MessageQCopy_E_TIMEOUT;
	}

	return ret;
}

/*
 *  ======== MessageQCopy_send ========
 */
int MessageQCopy_send(uint32_t dstEndpt,
						uint32_t srcEndpt,
						void *    data,
						uint16_t len,
						portTickType timeout)
{
    int16_t            token;
    MessageQCopy_Msg msg;
    int              length;
	xTimeOutType     tmchk;

	/* Send to remote processor: */
	MessageQCopy_enter_critical();

	token = VirtQueue_getAvailBuf(vQueToHost, (void **)&msg, &length);
	if(token < 0){
		if(timeout == 0){
			MessageQCopy_exit_critical();
			return MessageQCopy_E_TIMEOUT;
		}

		vTaskSetTimeOutState(&tmchk);
		waitAvaileBuf++;

		for(;;){
			portTickType waitEvent;

			/* Because this function is not an application code,          */
			/* there is not the problem with using vTaskPlaceOnEventList. */
			waitEvent = (timeout > 5) ? 5 : timeout;
			vTaskPlaceOnEventList(&availBufList, waitEvent);

			token = VirtQueue_getAvailBuf(vQueToHost, (void **)&msg, &length);
			if(token < 0){
				if(xTaskCheckForTimeOut(&tmchk, &timeout) == pdFALSE){
					continue;
				}

				waitAvaileBuf--;
				MessageQCopy_exit_critical();
				return MessageQCopy_E_TIMEOUT;
			}

			waitAvaileBuf--;
			if(waitAvaileBuf == 0){
				/* No need to know be kicked about added buffers anymore */
				PROHIBIT_VRING_NOTIFY(vQueToHost);
			}

			break;
		}
	}

	if(len > RP_MSG_PAYLOAD_SIZE){
		len = RP_MSG_PAYLOAD_SIZE;
	}

	/* Copy the payload and set message header: */
	memcpy(msg->payload, data, len);
	msg->dataLen = len;
	msg->dstAddr = dstEndpt;
	msg->srcAddr = srcEndpt;
	msg->flags = 0;
	msg->reserved = 0;

	VirtQueue_addUsedBuf(vQueToHost, token, RP_MSG_BUF_SIZE);
	VirtQueue_kick(vQueToHost);

	MessageQCopy_exit_critical();
	return MessageQCopy_S_SUCCESS;
}

