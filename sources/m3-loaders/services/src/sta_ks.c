/**
 * @file sta_ks.c
 * @brief Key storage proxy service
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

/* Standard includes. */
#include <string.h>
#include <errno.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "sta_nvic.h"
#include "semphr.h"

#include "cse_client.h"
#include "cmd_mailbox.h"
#include "ks_proxy.h"

/* HSM framework includes */
#include "sta_ks.h"

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/
static xSemaphoreHandle s_sem;

/**
 * @brief	Key storage Interrupt Service Routine
 * @param	None
 * @return	None
 */
void ks_irq_handler(void)
{
#if KS_MEMTYPE != NVM_NONE
	static signed portBASE_TYPE xHigherPriorityTaskWoken;
	int rec_size = 0;

	/* Disable interrupt mask */
	MBOX_M3_HSM->IMR &= ~MBX_IPSR_ANSWER_MASK;

	/* Clear notification */
	clear_notification();

	/* Get Message ID and Data */
	mb_cmd = g_mbx_buffer.cmd;
	switch (mb_cmd) {
	case MBX_CMD_LS_READ:
		rec_size = sizeof(mb_cmd_data.rw) - sizeof(mb_cmd_data.rw.data);
		break;
	case MBX_CMD_LS_WRITE:
		rec_size = sizeof(mb_cmd_data.rw);
		break;
	case MBX_CMD_LS_WRITE_KEY_AND_KVT:
		rec_size = sizeof(mb_cmd_data.write_key_and_kvt);
		break;
	case MBX_CMD_MC_SET_ROOT_KEY:
#if KS_MEMTYPE == NVM_SQI
		rec_size = sizeof(mb_cmd_data.data);
#endif
		break;
	case MBX_CMD_MC_SET_SESS_KEY:
		rec_size = sizeof(mb_cmd_data.uhkr);
		break;
	case MBX_CMD_MC_GET_VALUE:
#if KS_MEMTYPE == NVM_SQI
		rec_size = sizeof(mb_cmd_data.rmc);
#endif
		break;
	case MBX_CMD_MC_INC_VALUE:
#if KS_MEMTYPE == NVM_SQI
		rec_size = sizeof(mb_cmd_data.imc);
#endif
		break;
	default:
		break;
	}
	if (rec_size)
		memcpy(mb_cmd_data.data, g_mbx_buffer.data, rec_size);

	xSemaphoreGiveFromISR(s_sem, &xHigherPriorityTaskWoken);
#endif /* KS_MEMTYPE != NVM_NONE */
}

/**
 * @brief	Main function for key storage proxy management
 * @param	None
 * @return	None
 */
void key_storage_task(void)
{
	struct nvic_chnl irq_chnl;

	if (ks_service_init() == 0) {
		/* Start KS mailbox server */
		s_sem = xSemaphoreCreateBinary();

		/* Clear pending requests */
		clear_notification();

		MBOX_M3_HSM->IMR = MBX_IPSR_ANSWER_MASK;

		/* Enable MBOX interrupt */
		irq_chnl.id = C3_IRQChannel;
		irq_chnl.preempt_prio = IRQ_MAX_PRIO_INDEX;
		irq_chnl.enabled = true;

		nvic_chnl_init(&irq_chnl);

		extern void ls_tests(t_nvm_ctx *ctx);
		ls_tests(g_ks_nvm_ctx);

		/* Task infinite loop */
		while (1) {
			/* Wait for semaphore unlocked from ISR */
			xSemaphoreTake(s_sem, portMAX_DELAY);

			/* Handle KS commands */
			ks_service_dispatcher();
		}
	}

	vTaskDelete(NULL);

	return;
}

