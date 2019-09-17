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
 *  @file       VirtQueue.c
 *
 *  @brief      Virtio Queue implementation for FreeRTOS
 */

#include <string.h>
#include <errno.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "InterruptProxy.h"
#include "VirtQueue.h"

#include "trace.h"


/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of two)
 */
#define RP_MSG_NUM_BUFS     M3_RPMSG_VQ_SIZE

#define DIV_ROUND_UP(n,d)   (((n) + (d) - 1) / (d))
#define RP_MSG_BUFS_SPACE   (RP_MSG_NUM_BUFS * RP_MSG_BUF_SIZE * 2)

/*
 * The alignment to use between consumer and producer parts of vring.
 * Note: this is part of the "wire" protocol. If you change this, you need
 * to update your BIOS image as well
 */
#define RP_MSG_VRING_ALIGN  (4096)

/* With 256 buffers, our vring will occupy 3 pages */
#define RP_MSG_RING_SIZE    ((DIV_ROUND_UP(vring_size(RP_MSG_NUM_BUFS, \
                            RP_MSG_VRING_ALIGN), PAGE_SIZE)) * PAGE_SIZE)

/* The total IPC space needed to communicate with a remote processor */
#define RPMSG_IPC_MEM       (RP_MSG_BUFS_SPACE + 2 * RP_MSG_RING_SIZE)


static struct VirtQueue_Object *queueRegistry[RP_NUM_VRING] = {NULL};


extern xQueueHandle xQueueRpmsg;
extern bool         vRpmsgInitDone;


/*!
 * ======== VirtQueue_kick ========
 */
void VirtQueue_kick(VirtQueue_Handle vq)
{
	/* For now, simply interrupt remote processor */
	if (vq->vring.avail->flags & VRING_AVAIL_F_NO_INTERRUPT)
		return;

	InterruptProxy_intSend(vq->id);
}

/*!
 * ======== VirtQueue_addUsedBuf ========
 */
int VirtQueue_addUsedBuf(VirtQueue_Handle vq, int16_t head, int len)
{
	struct vring_used_elem *used;

	if ((head > vq->vring.num) || (head < 0))
		return (-1);

	/*
	 * The virtqueue contains a ring of used buffers.  Get a pointer to the
	 * next entry in that used ring.
	 */
	used = &vq->vring.used->ring[vq->vring.used->idx % vq->vring.num];
	used->id = head;
	used->len = len;

	vq->vring.used->idx++;

	return (0);
}

/*!
 * ======== VirtQueue_getAvailBuf ========
 */
int16_t VirtQueue_getAvailBuf(VirtQueue_Handle vq, void **buf, int *len)
{
	uint16_t head;

	/* There's nothing available? */
	if (vq->last_avail_idx == vq->vring.avail->idx) {
		/* We need to know about added buffers */
		vq->vring.used->flags &= ~VRING_USED_F_NO_NOTIFY;

		return (-1);
	}
	/*
	 * Grab the next descriptor number they're advertising, and increment
	 * the index we've seen.
	 */
	head = vq->vring.avail->ring[vq->last_avail_idx++ % vq->vring.num];

	//*buf = mapPAtoVA(vq->vring.desc[head].addr);
	*buf = (void *)vq->vring.desc[head].addr;
	*len = vq->vring.desc[head].len;

	return (head);
}

/*!
 * ======== VirtQueue_isr ========
 */
void VirtQueue_isr(void *context, struct mbox_msg *Msg)
{
	uint32_t msg[2] = {0, 0};
	VirtQueue_Object *vq;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uint32_t i;

	/*
	 * Get Rx message from Mailbox IPC
	 * msg[0] contains the actual mailbox message code
	 * msg[1] may contain an optional parameter
	 */
	for (i = 0; i < Msg->dsize && i < 4; i++)
		msg[0] |= (Msg->pdata[i]) << (8 * i);

	for (i = 4; i < Msg->dsize && i < 8; i++)
		msg[1] |= (Msg->pdata[i]) << (8 * (i - 4));

	/* Special flag in case of an empty message */
	if (Msg->dsize == 0)
		msg[0] = RP_MSG_EMPTY_MSG;

	switch(msg[0]) {
	case (uint32_t)RP_MSG_MBOX_READY:
		return;

	case (uint32_t)RP_MBOX_ECHO_REQUEST:
		InterruptProxy_intSend((uint32_t)(RP_MBOX_ECHO_REPLY));
		return;

	case (uint32_t)RP_MSG_INIT_REQUEST:
		/*
		 * Wake up task waiting for event in xQueue to send Resource table address
		 * and perform RPMSG init according to the resource table parameters
		 */
		xQueueSendToBackFromISR( xQueueRpmsg, (void *) msg, &xHigherPriorityTaskWoken );
		return;

	case (uint32_t)RP_MSG_HIBERNATION:
	case (uint32_t)RP_MSG_HIBERNATION_FORCE:
		InterruptProxy_intSend((uint32_t)RP_MSG_HIBERNATION_CANCEL);
		return;

	case RP_ID_TX_VRING:
	case RP_ID_RX_VRING:
		/*
		 * If none of the above, msg[0] is most probably a virtqueue index
		 */
		vq = queueRegistry[msg[0]];
		if (vq)
			vq->callback(vq);

		/*
		 * 1st virtqueue message = host's kick
		 */
		if (vRpmsgInitDone == INIT_ONGOING) {
			msg[0] = RP_MSG_HOST_READY;
			xQueueSendToBackFromISR( xQueueRpmsg, (void *)msg, &xHigherPriorityTaskWoken );
		}
		break;

	default:
		/*
		 * Spurious interrupt. Forward the message to RPMSG_Init task as is,
		 * it will print an error message.
		 */
		xQueueSendToBackFromISR( xQueueRpmsg, (void *)msg, &xHigherPriorityTaskWoken );

		break;
	}

	return;
}

/*!
 * ======== VirtQueue_create ========
 */
VirtQueue_Object *VirtQueue_create(VirtQueue_callback callback, VirtQueue_Entry *vEntry)
{
	VirtQueue_Object *vq;

	vq = pvPortMalloc(sizeof(VirtQueue_Object));
	if (!vq)
		return (NULL);

	memset(vq, 0, sizeof(*vq));
	vq->callback = callback;
	vq->id = vEntry->NotifyId;
	vq->last_avail_idx = 0;

	vring_init(&(vq->vring), vEntry->NumBuf, vEntry->VringAddr, vEntry->Align);

	/*
	 *  Don't trigger a mailbox message every time MPU makes another buffer
	 *  available
	 */
	if (vEntry->VringId == RP_ID_TX_VRING) {
		vq->vring.used->flags |= VRING_USED_F_NO_NOTIFY;
	}

	queueRegistry[vq->id] = vq;

	return (vq);
}

/*!
 * ======== VirtQueue_getRscEntry ========
 */
int VirtQueue_getRscEntry(const VirtQueue_RscTable *RscTab, VirtQueue_Entry *Entry)
{
	struct fw_rsc_vdev       *RscVdev;
	struct fw_rsc_vdev_vring *RscVring;
#ifndef CLI_ENABLED
	struct fw_rsc_trace      *RscTrace;
#endif
	unsigned int i, j;
	int status = -EFAULT;

	for (i = 0; i < RscTab->num; i++) {
		int offset = RscTab->offset[i];
		void *rsc = (void *)RscTab + offset;
		struct fw_rsc_hdr *hdr = rsc;

		switch (hdr->type) {
		case TYPE_VDEV:
			/* Fetch VDEV resource entries from resource table */
			RscVdev = (struct fw_rsc_vdev *)rsc;
			RscVring = (void *)RscVdev + sizeof(*RscVdev);

			if (RscVdev->num_of_vrings != RP_NUM_VRING) {
				/* Wrong number of vring */
				status = -EFAULT;
				break;
			}
			/* Get 2 Vring parameters: 1st RX then Tx */
			for (j = 0; j < RscVdev->num_of_vrings; j++, Entry++, RscVring++) {
				Entry->VringAddr = (void *)RscVring->da;
				Entry->Align = RscVring->align;
				Entry->NumBuf = RscVring->num;
				Entry->NotifyId = RscVring->notifyid;
				Entry->VringId = j;
			}
			status = 0;
			break;

		case TYPE_TRACE:
#ifndef CLI_ENABLED
			/* Fetch TRACE resource entries from resource table */
			RscTrace = (struct fw_rsc_trace *)rsc;
			trace_init(NO_TRACE_PORT, (char *)RscTrace->da, RscTrace->len);
#endif
			break;

		case TYPE_CARVEOUT:
		case TYPE_DEVMEM:
			/* Resource type unsupported so far */
			break;

		default:
			/* Wrong resource type */
			return -EINVAL;
		}
	}
	return status;
}

#ifndef configMBOX_INTERRUPT_PRIORITY
#define configMBOX_INTERRUPT_PRIORITY configMAX_SYSCALL_INTERRUPT_PRIORITY
#elif configMBOX_INTERRUPT_PRIORITY < configMAX_SYSCALL_INTERRUPT_PRIORITY
#undef configMBOX_INTERRUPT_PRIORITY
#define configMBOX_INTERRUPT_PRIORITY configMAX_SYSCALL_INTERRUPT_PRIORITY
#endif

/*!
 * ======== VirtQueue_startup ========
 */
void VirtQueue_startup()
{
	/** Interrupt INIT moved to RPMSG_Init procedure **/
	//InterruptIpu_intRegister(VirtQueue_isr);
	//NVIC_SetPriority(MAIL_U2_M3_IRQ, (configMBOX_INTERRUPT_PRIORITY >> __NVIC_PRIO_BITS));
	//NVIC_EnableIRQ(MAIL_U2_M3_IRQ);
}

