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
SteciPushNotifyCallback p_SteciPush_Callback = NULL;

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
static OSAL_tThreadID ETAL_steciEventIRQThreadId;

#ifndef ETAL_EVENT_HANDLER_STECI
#define ETAL_EVENT_HANDLER_STECI					"ETAL_STECI_EventHandler"
#endif

static tVoid ETAL_STECI_REQ_IRQ_ThreadEntry(tPVoid dummy);
#endif // #ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI

extern tVoid ETAL_CommMdr_DataRx(tVoid);
#endif

/***************************
 *
 * ETAL_InitSteciProtocol
 *
 **************************/
tSInt ETAL_InitSteciProtocol(tVoid *arg)
{
	OSAL_trThreadAttribute thread1_attr;
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
	OSAL_trThreadAttribute thread2_attr;
#endif

	p_SteciPush_Callback = ETAL_CommMdr_DataRx;
#endif 

	BSP_SteciSetCS_MDR(TRUE);
	if (STECI_fifoInit(p_SteciPush_Callback) != OSAL_OK)
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

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI	
		// Init globals
		ETAL_Steci_TaskEventHandler = (OSAL_tEventHandle)0;
		OSAL_s32EventCreate ((tCString)ETAL_EVENT_HANDLER_STECI, &ETAL_Steci_TaskEventHandler);	

#endif //  CONFIG_ETAL_CPU_IMPROVEMENT_STECI


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

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
	thread2_attr.szName = (tChar *)ETAL_COMM_STECI_EVENT_REQ_THREAD_NAME;
	thread2_attr.u32Priority = ETAL_COMM_STECI_IRQ_THREAD_PRIORITY;
	thread2_attr.s32StackSize = ETAL_COMM_STECI_IRQ_STACK_SIZE;
	thread2_attr.pfEntry = ETAL_STECI_REQ_IRQ_ThreadEntry;
	thread2_attr.pvArg = NULL;

	ETAL_steciEventIRQThreadId = OSAL_ThreadSpawn(&thread2_attr);
	if (ETAL_steciEventIRQThreadId == OSAL_ERROR) 
	{
		STECI_tracePrintError(TR_CLASS_STECI, "STECI IRQ thread creation error");
		return OSAL_ERROR;
	}
#endif

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
		// give the killed thread the opportunity to do cleanup if required
		OSAL_s32ThreadWait(20);
		
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

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
		if ((ETAL_steciEventIRQThreadId != OSAL_ERROR) &&
			(OSAL_s32ThreadDelete(ETAL_steciEventIRQThreadId) == OSAL_OK))
		{
			ETAL_steciEventIRQThreadId = OSAL_ERROR;
			// give the killed thread the opportunity to do cleanup if required
			OSAL_s32ThreadWait(20);
		}

#ifndef OSAL_EVENT_SKIP_NAMES
		if (OSAL_s32EventClose(ETAL_Steci_TaskEventHandler) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
		
		OSAL_s32ThreadWait(100);
		
		if (OSAL_s32EventDelete(ETAL_EVENT_HANDLER_STECI) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
#else
		if (OSAL_s32EventFree(ETAL_Steci_TaskEventHandler) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
#endif
	
		ETAL_Steci_TaskEventHandler = (OSAL_tEventHandle)0;
	
#endif // CONFIG_ETAL_CPU_IMPROVEMENT_STECI
		
	// give the killed thread the opportunity to do cleanup if required
	OSAL_s32ThreadWait(20);

	return retosal;
}

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
static tVoid ETAL_STECI_REQ_IRQ_ThreadEntry(tPVoid dummy)
{
#if 0
	tBool currentLevel, new_level;

	currentLevel = BSP_SteciReadREQ_MDR();
#endif

	while (1)
	{
		BSP_WaitForREQInterrupt_MDR();
#if 0
		do
		{
			OSAL_s32ThreadWait (1);	
			new_level = BSP_SteciReadREQ_MDR();			
		} while (new_level == currentLevel);

		currentLevel = new_level;
#endif

		ETAL_STECI_DataRx();
	}
}
#endif // CONFIG_ETAL_CPU_IMPROVEMENT_STECI

#endif // CONFIG_COMM_DRIVER_EMBEDDED
#endif // CONFIG_HOST_OS_LINUX && CONFIG_ETAL_SUPPORT_DCOP_MDR


