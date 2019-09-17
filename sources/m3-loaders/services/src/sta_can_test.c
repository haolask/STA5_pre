/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : STA_can_test.c
* Author             : APG-MID Application Team
* Date First Issued  : 06/16/2014
* Description        : This file provides CAN test functions.
********************************************************************************
* History:
* 06/16/2014: V0.1
* 17/03/2015: Rework to handle incoming frames
*             Incoming frames are stored in a queue until
*             task call can_get_frame() function.
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
#include "utils.h"

/* Platform includes */
#include "sta_common.h"
#include "sta_can.h"
#include "sta_pm.h"
#include "sta_can_test.h"

#define FRAME_MAX_NUMBER 64
#define CAN_DETECT PMU_WAKE0

extern ControllerMO_private ControllerMO[MAX_CONFIG_CAN_CONTROLLERS];
extern HandlerMO_private HandlerMO[MAX_HANDLES + 1];

struct can_tsk_ctx {
	struct pm_notifier sby_notifier;
	struct pm_notifier reboot_notifier;
	bool end;
	bool ended;
	bool loopback;
	xQueueHandle frame_queue;
};

static struct can_tsk_ctx can_ctx;

/*******************************************************************************
* Function Name  : can_notifier_cb
* Description    : CAN power management notifier call-back
* Input          : None
* Return         : None
*******************************************************************************/
static void can_notifier_cb(void)
{
	can_ctx.end = true;
	while(!can_ctx.ended)
	        vTaskDelay(pdMS_TO_TICKS(10));
}

/*******************************************************************************
* Function Name  : can_rxcb
* Description    : CAN Rx indication call-back
* Input          : uint8_t handle, uint32_t identifier, uint8_t can_dlc, uint8_t *can_sdu
* Return         : None
*
* This function is called in interrupt context
*******************************************************************************/
static void can_rxcb(uint8_t handle, uint32_t identifier, uint8_t can_dlc, uint8_t *can_sdu)
{
	struct can_frame f;
	portBASE_TYPE xHigherPriorityTaskWoken;

	if (can_dlc > FRAME_SIZE)
		can_dlc = FRAME_SIZE;

	/* Fill frame content */
	f.id = identifier;
	memcpy(f.data, can_sdu, can_dlc);
	f.length = can_dlc;

	/* Wake up task waiting for event in the user frame queue */
	if (!xQueueSendToBackFromISR(can_ctx.frame_queue,
			&f, &xHigherPriorityTaskWoken)) {
		return;
	}
	if (xHigherPriorityTaskWoken)
		taskYIELD();
}

int can_get_frame(struct can_frame *f) {

	/* Wait for incoming can frame */
        return xQueueReceive(can_ctx.frame_queue, f, 0);
}

void can_send_frame(struct can_frame *f)
{
	Can_PduType tx_pdu_info;
	/* Setup the Protocol Data Unit */
	tx_pdu_info.Id      = f->id;
	tx_pdu_info.Sdu     = f->data;
	tx_pdu_info.Lenght  = f->length;

	/* Transfer the CAN message */
	CCanLLD_Write(0, 0, (Can_PduType *)(&tx_pdu_info),
		ControllerMO[0].SwPduHandle[HandlerMO[ControllerMO[0].HW_Object_ptr[CAN_MO_1]->Handle_number].MailBox]);
}

void can_set_loopback(bool enabled)
{
	can_ctx.loopback = enabled;
}

/**
 * @brief	CAN initialization, allows to start CAN and
 * instanciate context.
 */
void can_task(void *p)
{
	struct can_frame cf;
	struct feature_set *fs = (struct feature_set *)p;
	uint32_t input_clock = 0;

	memset(&can_ctx, 0, sizeof(struct can_tsk_ctx));

	can_ctx.frame_queue = xQueueCreate(FRAME_MAX_NUMBER,
		sizeof(struct can_frame));
	if (!can_ctx.frame_queue) {
		 goto end;
	}

	can_ctx.loopback = fs->can0_hello;
	can_ctx.sby_notifier.call_back = can_notifier_cb;
	can_ctx.reboot_notifier.call_back = can_notifier_cb;
	/* register to go to stand-by events */
	pm_register_standby(&can_ctx.sby_notifier);
	/* register to reboot events */
	pm_register_reboot(&can_ctx.reboot_notifier);

	switch (get_board_id()) {
	case BOARD_A5_EVB:
	case BOARD_A5_VAB:
	case BOARD_TC3_EVB:
		input_clock = 51200000;
		break;
	case BOARD_CR2_VAB:
	case BOARD_TC3P_CARRIER:
	case BOARD_TC3P_MTP:
		input_clock = 52000000;
		break;
	default:
		TRACE_ERR("%s: unknown board id:%d\n", __func__,
			  get_board_id());
		break;
	}

	/* Start CAN engine */
	CAN_Engine_Start(can_rxcb, input_clock);

	while(!can_ctx.end) {
		if (xQueueReceive(can_ctx.frame_queue, &cf,
				  pdMS_TO_TICKS(2000))) {
			/* Let send welcome frame to remote peer */
			if (can_ctx.loopback)
				can_send_frame(&cf);
			TRACE_NOTICE("can frame recv: %x [%x] %x %x %x %x %x %x %x %x\n",
				cf.id, cf.length,
				cf.data[0], cf.data[1], cf.data[2], cf.data[3],
				cf.data[4], cf.data[5], cf.data[6], cf.data[7]);
		}
	}
end:
	can_ctx.ended = true;
	vTaskDelete(NULL);
}
