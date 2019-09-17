/* This file is based on SYS/BIOS RPMsg code.
 *
 * Repositories:
 *  http://git.omapzoom.org/?p=repo/sysbios-rpmsg.git;a=summary
 *
 * The original license terms are as follows.
 */
/*
 * Copyright (c) 2011, Texas Instruments Incorporated
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
 *  @file       VirtQueue.h
 *
 *  @brief      Virtio Queue interface for FreeRTOS
 *
 *  Usage:
 *     This IPC only works between one processor designated as the Host (Linux)
 *     and one or more Slave processors (FreeRTOS).
 *
 *     For any Host/Slave pair, there are 2 VirtQueues (aka Vrings);
 *     Only the Host adds new buffers to the avail list of a vring;
 *     Available buffers can be empty or full, depending on direction;
 *     Used buffer means "processed" (emptied or filled);
 *
 *  Host:
 *    - To send buffer to the slave processor:
 *          add_avail_buf(slave_virtqueue);
 *          kick(slave_virtqueue);
 *          get_used_buf(slave_virtqueue);
 *    - To receive buffer from slave processor:
 *          add_avail_buf(host_virtqueue);
 *          kick(host_virtqueue);
 *          get_used_buf(host_virtqueue);
 *
 *  Slave:
 *    - To send buffer to the host:
 *          get_avail_buf(host_virtqueue);
 *          add_used_buf(host_virtqueue);
 *          kick(host_virtqueue);
 *    - To receive buffer from the host:
 *          get_avail_buf(slave_virtqueue);
 *          add_used_buf(slave_virtqueue);
 *          kick(slave_virtqueue);
 *
 *  All VirtQueue operations can be called in any context.
 *
 *  ============================================================================
 */

#ifndef ti_ipc_VirtQueue__include
#define ti_ipc_VirtQueue__include

#include "sta_type.h"
#include "virtio_ring.h"
#include "sta_mbox.h"
#include "resource.h"

#if defined (__cplusplus)
extern "C" {
#endif

/*
 * enum - Predefined Mailbox Messages
 *
 * @RP_MSG_MBOX_READY: informs the M3's that we're up and running. will be
 * followed by another mailbox message that carries the A9's virtual address
 * of the shared buffer. This would allow the A9's drivers to send virtual
 * addresses of the buffers.
 *
 * @RP_MSG_MBOX_STATE_CHANGE: informs the receiver that there is an inbound
 * message waiting in its own receive-side vring. please note that currently
 * this message is optional: alternatively, one can explicitly send the index
 * of the triggered virtqueue itself. the preferred approach will be decided
 * as we progress and experiment with those design ideas.
 *
 * @RP_MSG_MBOX_CRASH: this message indicates that the BIOS side is unhappy
 *
 * @RP_MBOX_ECHO_REQUEST: this message requests the remote processor to reply
 * with RP_MBOX_ECHO_REPLY
 *
 * @RP_MBOX_ECHO_REPLY: this is a reply that is sent when RP_MBOX_ECHO_REQUEST
 * is received.
 *
 * @RP_MBOX_ABORT_REQUEST:  tells the M3 to crash on demand
 *
 * @RP_MBOX_BOOTINIT_DONE: this message indicates the BIOS side has reached a
 * certain state during the boot process. This message is used to inform the
 * host that the basic BIOS initialization is done, and lets the host use this
 * notification to perform certain actions.
 */
enum {
    RP_MSG_MBOX_READY           = (int)0xFFFFFF00,
    RP_MSG_MBOX_STATE_CHANGE    = (int)0xFFFFFF01,
    RP_MSG_MBOX_CRASH           = (int)0xFFFFFF02,
    RP_MBOX_ECHO_REQUEST        = (int)0xFFFFFF03,
    RP_MBOX_ECHO_REPLY          = (int)0xFFFFFF04,
    RP_MBOX_ABORT_REQUEST       = (int)0xFFFFFF05,
    RP_MSG_INIT_REQUEST         = (int)0xFFFFFF06,
    RP_MSG_INIT_ACK             = (int)0xFFFFFF07,
    RP_MSG_INIT_FAILED          = (int)0xFFFFFF08,
    RP_MSG_HIBERNATION          = (int)0xFFFFFF10,
    RP_MSG_HIBERNATION_FORCE    = (int)0xFFFFFF11,
    RP_MSG_HIBERNATION_ACK      = (int)0xFFFFFF12,
    RP_MSG_HIBERNATION_CANCEL   = (int)0xFFFFFF13,
    RP_MSG_HOST_READY           = (int)0xFFFFFF15,
    RP_MSG_EMPTY_MSG            = (int)0xFFFFFF16
};

/*!
 *  @brief  VirtQueue Ids for the basic IPC transport rings.
 */
#define RP_ID_TX_VRING      0
#define RP_ID_RX_VRING      1


/* Used for defining the size of the virtqueue registry */
#define RP_NUM_VRING        2

/*!
 *  @brief  Size of buffer being exchanged in the VirtQueue rings.
 */
#if defined(STA_RPMSG_BUF_SIZE)
#define RP_MSG_BUF_SIZE     (STA_RPMSG_BUF_SIZE)
#else
#define RP_MSG_BUF_SIZE     (8192)
#endif

#define RP_MSG_PAYLOAD_SIZE	(RP_MSG_BUF_SIZE - 16)

struct VirtQueue_Object;

/*!
 *  @brief  a queue to register resource table entries.
 */
typedef struct resource_table VirtQueue_RscTable;

/*!
 *  @brief  a queue to register buffers for sending or receiving.
 */
typedef struct VirtQueue_Object *VirtQueue_Handle;

/*!
 *  @var     VirtQueue_callback
 *  @brief   Signature of any callback function that can be registered with the
 *           VirtQueue
 *
 *  @param[in]  VirtQueue     Pointer to the VirtQueue which was signalled.
 */
typedef void (*VirtQueue_callback)(VirtQueue_Handle);

/*!
 *  @brief      Queue object to register buffers for sending or receiving.
 *
 *  @id:            Id for this VirtQueue_Object
 *  @callback:      The function to call when buffers are consumed (can be NULL)
 *  @vring:         Shared vring state
 *  @num_free:      Number of free buffers
 *  @last_avail_idx:Last available index; updated by VirtQueue_getAvailBuf
 *  @last_used_idx: Last available index; updated by VirtQueue_addUsedBuf
 */
typedef struct VirtQueue_Object {
    uint16_t                  id;
    VirtQueue_callback      callback;
    struct vring            vring;
    uint16_t                  num_free;
    uint16_t                  last_avail_idx;
    uint16_t                  last_used_idx;
} VirtQueue_Object;

/*!
 *  @brief      Queue to register resource table entries.
 *
 *  @VringAddr:  Shared Memory address allocated by Linux
 *  @Align:      Alignment between the consumer and producer parts of the vring
 *  @NumBuf:     Num of buffers supported by this vring (must be power of two)
 *  @NotifyId:   Unique rproc-wide notify index for this vring. This notify
 *               index is used when kicking a remote processor, to let it know
 *               that this vring is triggered.
 *  @VringId:    Vring index between Tx and Rx entry
 */
typedef struct VirtQueue_Entry {
	void        *VringAddr;
	uint32_t      Align;
	uint32_t      NumBuf;
	uint32_t      NotifyId;
    uint32_t      VringId;
} VirtQueue_Entry;

/*!
 *  @brief      Initialize at runtime the VirtQueue
 *
 *  @param[in]  callback  the clients callback function.
 *  @param[in]  vEntry    Resource entries to configure this VirtQueue.
 *
 *  @Returns    Returns a handle to a new initialized VirtQueue.
 */
VirtQueue_Handle VirtQueue_create(VirtQueue_callback callback, VirtQueue_Entry *vEntry);

/*!
 *  @brief      Initialize at runtime the VirtQueue
 *
 *  @param[in]  callback  the clients callback function.
 *  @param[in]  vqId      VirtQueue ID for this VirtQueue.
 *
 *  @Returns    Returns a handle to a new initialized VirtQueue.
 */
 int VirtQueue_getRscEntry(const VirtQueue_RscTable *RscTab, VirtQueue_Entry *Entry);

/*!
 *  @brief      Notify other processor of new buffers in the queue.
 *
 *  After one or more add_buf calls, invoke this to kick the other side.
 *
 *  @param[in]  vq        the VirtQueue.
 *
 *  @sa         VirtQueue_addBuf
 */
void VirtQueue_kick(VirtQueue_Handle vq);

/*!
 *  @brief      Notify local processor of new buffers have been received.
 *
 *  Function registered as callback procedure in the Mailbox allocated
 *  to RPMSG framework.
 *
 *  @param[in]  context   Pointer to private data.
 *  @param[in]  Msg       Pointer to the Message received in the Mailbox.
 */
void VirtQueue_isr(void *context, struct mbox_msg *Msg);

/*!
 *  @brief       Used at startup-time for initialization
 *
 *  Should be called before any other VirtQueue APIs
 */
void VirtQueue_startup();


/*
 *  ============================================================================
 *  Host Only Functions:
 *  ============================================================================
 */

/*
 *  ============================================================================
 *  Slave Only Functions:
 *  ============================================================================
 */

/*!
 *  @brief      Get the next available buffer.
 *              Only used by Slave.
 *
 *  @param[in]  vq        the VirtQueue.
 *  @param[out] buf       Pointer to location of available buffer;
 *  @param[out] len       Length of the available buffer message.
 *
 *  @return     Returns a token used to identify the available buffer, to be
 *              passed back into VirtQueue_addUsedBuf();
 *              token is negative if failure to find an available buffer.
 *
 *  @sa         VirtQueue_addUsedBuf
 */
int16_t VirtQueue_getAvailBuf(VirtQueue_Handle vq, void **buf, int *len);

/*!
 *  @brief      Add used buffer to virtqueue's used buffer list.
 *              Only used by Slave.
 *
 *  @param[in]  vq        the VirtQueue.
 *  @param[in]  token     token of the buffer to be added to vring used list.
 *  @param[in]  len       length of the message being added.
 *
 *  @return     Remaining capacity of queue or a negative error.
 *
 *  @sa         VirtQueue_getAvailBuf
 */
int VirtQueue_addUsedBuf(VirtQueue_Handle vq, int16_t token, int len);


#define GET_AVAIL_COUNT(vq) (vq->vring.avail->idx - vq->last_avail_idx)
#define PROHIBIT_VRING_NOTIFY(vq) (vq->vring.used->flags |= VRING_USED_F_NO_NOTIFY)

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* ti_ipc_VirtQueue__include */
