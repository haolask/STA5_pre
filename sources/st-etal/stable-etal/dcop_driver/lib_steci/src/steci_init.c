//!
//!  \file 		steci_init.c
//!  \brief 	<i><b> STECI initialization function for Linux/Accordo2 </b></i>
//!  \details   STECI initialization used by ETAL and by MDR_Protocol, Linux host only
//!  \author 	Raffaele Belardi
//!
#include "target_config.h"

#if (defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

#if defined (CONFIG_COMM_DRIVER_EMBEDDED)
#include "osal.h"

#include "DAB_Protocol.h"
#include "connection_modes.h"
#include "steci_defines.h"
#include "steci_helpers.h"
#include "steci_protocol.h"
#include "common_trace.h"
#include "steci_trace.h"

#if defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5)
	#include "bsp_sta1095evb.h"
#endif

/***************************
 *
 * Local variables
 *
 **************************/
/* only for deinit */
static OSAL_tThreadID ETAL_steciThreadId;

/***************************
 *
 * ETAL_InitSteciProtocol
 *
 **************************/
tSInt ETAL_InitSteciProtocol(tVoid *arg)
{
	OSAL_trThreadAttribute thread1_attr;

	BSP_SteciSetCS_MDR(TRUE);
	if (STECI_fifoInit() != OSAL_OK)
	{
		return OSAL_ERROR;
	}

    STECI_tracePrintComponent(TR_CLASS_STECI, "Starting STECI");

	// create semaphores
	
	// Need semaphore to guarantee msg process before new ones
	// create 2 semaphores : 
	// - one to make sure the Message sending request are serialized
	// - one to make sure the message sending procedure returns only when msg has been sent
	// 
	
	if (OSAL_s32SemaphoreCreate(STECI_ProtocolHandle_MsgSendingSem_STR, &STECI_ProtocolHandle_MsgSendingSem, 1) != OSAL_OK)
	{
		STECI_tracePrintError(TR_CLASS_STECI, "STECI semaphore %s creation error", STECI_ProtocolHandle_MsgSendingSem_STR);
		return OSAL_ERROR;
	}

	// create the semaphore for msg sent : 1st value is 0 
	//
	if (OSAL_s32SemaphoreCreate(STECI_ProtocolHandle_MsgSentSem_STR, &STECI_ProtocolHandle_MsgSentSem, 0) != OSAL_OK)
	{
		STECI_tracePrintError(TR_CLASS_STECI, "STECI semaphore %s creation error", STECI_ProtocolHandle_MsgSentSem_STR);
		return OSAL_ERROR;
	}


	thread1_attr.szName = (tChar *)ETAL_COMM_STECI_THREAD_NAME;
	thread1_attr.u32Priority = ETAL_COMM_STECI_THREAD_PRIORITY;
	thread1_attr.s32StackSize = ETAL_COMM_STECI_STACK_SIZE;
	thread1_attr.pfEntry = STECI_ProtocolHandle;
	thread1_attr.pvArg = arg;

	ETAL_steciThreadId = OSAL_ThreadSpawn(&thread1_attr);
	if (ETAL_steciThreadId == OSAL_ERROR)
	{
		STECI_tracePrintError(TR_CLASS_STECI, "STECI thread creation error");
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_DeinitSteciProtocol
 *
 **************************/
tSInt ETAL_DeinitSteciProtocol(tVoid)
{
	tSInt retosal = OSAL_OK;

	if ((ETAL_steciThreadId != OSAL_ERROR) &&
		(OSAL_s32ThreadDelete(ETAL_steciThreadId) == OSAL_OK))
	{
		ETAL_steciThreadId = OSAL_ERROR;
	}
	else
	{
		retosal = OSAL_ERROR;
	}

    if (OSAL_s32SemaphoreClose(STECI_ProtocolHandle_MsgSendingSem) != OSAL_OK)
    {
        retosal = OSAL_ERROR;
    }

    if (OSAL_s32SemaphoreDelete(STECI_ProtocolHandle_MsgSendingSem_STR) != OSAL_OK)
    {
        retosal = OSAL_ERROR;
    }

    if (OSAL_s32SemaphoreClose(STECI_ProtocolHandle_MsgSentSem) != OSAL_OK)
    {
        retosal = OSAL_ERROR;
    }

    if (OSAL_s32SemaphoreDelete(STECI_ProtocolHandle_MsgSentSem_STR) != OSAL_OK)
    {
        retosal = OSAL_ERROR;
    }

	STECI_fifoDeinit();

	return retosal;
}

#endif // CONFIG_COMM_DRIVER_EMBEDDED
#endif // CONFIG_HOST_OS_LINUX && CONFIG_ETAL_SUPPORT_DCOP_MDR


