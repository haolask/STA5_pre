/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file osal.c																			    			*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains osal related API definitions									*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "osal_api.h"
#ifdef UITRON
#include <app_tuner.h>
#endif
#include "sys_task.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (Global)
-----------------------------------------------------------------------------*/
#ifdef OS_WIN32
Ts_MailBox		cmbx[SYS_TASK_CNT];
#endif

/* CALIBRATION TOOL STUFF */
Ts8	S8_MutexLockErrorCode = 0;
Ts8 S8_MutexUnLockErrorCode = 0;

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/


/******************CREATION OF OS RESOURCES***********************************/


#ifdef UITRON
/*===========================================================================*/
/* ER_ID OSAL_CreateMB                                                       */
/*===========================================================================*/
ER_ID OSAL_CreateMB(void)
{
    T_CMBX 	cmbx;
	ER_ID   mailbxid;
    ER_ID   RET;
	
	cmbx.mbxatr		= (TA_TFIFO | TA_MFIFO);
	cmbx.maxmpri 	= MAIL_BX_PRI;
	cmbx.mprihd		= NULL;
	mailbxid 		= acre_mbx(&cmbx);
	
	if(mailbxid <= 0)
	{
		RET = -1;
	}
	else
	{
		RET = mailbxid;
	}
    return RET;
}
#elif defined OS_WIN32
/*===========================================================================*/
/* Ts32 OSAL_CreateMB                                                        */
/*===========================================================================*/
Tu32* OSAL_CreateMB(Tu32 *mbbuf, Tu16 mbsize)
{
	Tu32*  mailboxid;
	Tu32*  RET;
	static Tu16 mbx_count = 0;

	if (mbx_count <= SYS_TASK_CNT)
	{
		WIN32_CreateMB(&cmbx[mbx_count], mbsize, (void*)mbbuf);

		mailboxid = (Tu32 *)&cmbx[mbx_count];

		if (mailboxid == NULL)
		{
			/*FAILURE*/
			RET = NULL;
		}
		else
		{
			/*SUCCESS*/
			mbx_count++;
			RET = mailboxid; 
		}
	}
	else
	{
		/*FAILURE*/
		RET = NULL;
	}
	return RET;
}
#endif

/*===========================================================================*/
/* ER_ID OSAL_CreateSemaphore                                                */
/*===========================================================================*/
#ifdef UITRON
ER_ID OSAL_CreateSemaphore(void)
#elif defined OS_WIN32
Ts32 OSAL_CreateSemaphore(void)
#endif
{    
#ifdef UITRON
	ER_ID   RET;
	T_CSEM 	csem;
	ER_ID   Semaphoreid;
	csem.sematr		= TA_TFIFO;
	csem.isemcnt	= SEM_CNT_VAL;
	csem.maxsem		= MAX_SEM_VAL;
	Semaphoreid		= acre_sem(&csem);
#elif defined OS_WIN32
	t_SemaphoreHandle   Semaphoreid;
	Ts32   RET;
	WIN32_CreateSemaphore(&Semaphoreid, SEM_CNT_VAL, MAX_SEM_VAL);
#endif
	if (Semaphoreid == NULL)
	{
		/*FAILURE*/
		RET = -1;
	}
	else
	{
		/*SUCCESS*/
		RET = (ER_ID)Semaphoreid;
	}
    return RET;
}
#ifdef UITRON
/*===========================================================================*/
/* ER_ID OSAL_CreateMemPool                                                  */
/*===========================================================================*/
ER_ID OSAL_CreateMemPool(const char *task_name)
{
	T_CMPF  cmpf;
	ER_ID   MemPoolid;
    ER_ID   RET;
	
	cmpf.mpfatr		= TA_TFIFO;

	if(strcmp(task_name, "DAB_TUNER_CTRL TASK") == 0)
	{
		cmpf.blkcnt		= DAB_TUNER_CTRL_MEMPOOL_BLK_CNT;
		cmpf.blksz		= MEMPOOL_BLK_SZ;
	}
	else if(strcmp(task_name, "SPI_READ_TASK") == 0)
	{
		cmpf.blkcnt		= 1;
		cmpf.blksz		= MEMPOOL_BLK_SZ;
	}
	else
	{
		cmpf.blkcnt		= MEMPOOL_BLK_CNT;
		cmpf.blksz		= MEMPOOL_BLK_SZ;
	}
	cmpf.mpf		= NULL;

	MemPoolid = acre_mpf(&cmpf);	
	if(MemPoolid <= 0)
	{
		RET = -1;
	}
	else
	{
		RET = MemPoolid;
	}
    return RET;
}
#endif
/*===========================================================================*/
/*       ER_ID OSAL_CreateMutex                                               */
/*===========================================================================*/
ER_ID OSAL_CreateMutex(void)
{
	ER_ID   MutexId;
    ER_ID   RET; 
#ifdef UITRON
	T_CMTX  cmtx;
	cmtx.mtxatr		= TA_CEILING;
	cmtx.ceilpri	= 9; //// Change from 1 to 9 as the task priority 11_16_16
	MutexId = acre_mtx(&cmtx);
#elif defined OS_WIN32
	MutexId = WIN32_CreateMutex();
#endif
	if(MutexId <= 0)
	{
		/*FAILURE*/
		RET = -1;
	}
	else
	{
		/*SUCCESS*/
		RET = MutexId;
	}
    return RET;
}

/*===========================================================================*/
/*       ER_ID OSAL_CreateMutex                                               */
/*===========================================================================*/
ER_ID OSAL_Mutex_Lock(ER_ID mutexId)
{
    ER_ID RET;
#ifdef UITRON
	S8_MutexLockErrorCode = loc_mtx(mutexId);
#elif defined OS_WIN32
	S8_MutexLockErrorCode = WIN32_Mutex_Lock(mutexId);
#endif
	if (S8_MutexLockErrorCode != E_OK)
	{
		/*FAILURE*/
		RET = S8_MutexLockErrorCode;
	}
	else
	{
		/*SUCCESS*/
		RET = E_OK;
	}

    return RET;
}

/*===========================================================================*/
/*       ER_ID OSAL_CreateMutex                                               */
/*===========================================================================*/
ER_ID OSAL_Mutex_Unlock(ER_ID mutexId)
{
    ER_ID RET;
#ifdef UITRON
	S8_MutexUnLockErrorCode = unl_mtx(mutexId);
#elif defined OS_WIN32
	S8_MutexUnLockErrorCode = WIN32_Mutex_Unlock(mutexId);
#endif
	if (S8_MutexUnLockErrorCode != E_OK)/*SUCCESS*/
	{		
		RET = E_OK;
	}
	else
	{		
		/*FAILURE*/
		RET = S8_MutexUnLockErrorCode;
	}

    return RET;
}

/*===========================================================================*/
/*       Tu32 OSAL_CreateEvent                                               */
/*===========================================================================*/
Tu32 OSAL_CreateEvent(Tu32 taskId)
{
	Tu32 hEvent;
	
	hEvent = WIN32_CreateEvent(taskId);

	return hEvent;
}

/*===========================================================================*/
/* ER_ID OSAL_CreateTask                                                     */
/*===========================================================================*/
#ifdef UITRON
ER_ID OSAL_CreateTask(Tu8 priority, OSAL_TASK_FUNC func, void* stack, Tu32 stack_size)
{
	ER_ID     	Taskid;
	ER_ID       RET;

	T_CTSK  ctsk;
	ctsk.tskatr	    = TA_HLNG;
	ctsk.exinf	 	= (VP_INT)NULL;
	ctsk.task	 	= (void *)func;
	ctsk.itskpri 	= priority;
	ctsk.stksz	 	= stack_size; 
	ctsk.stk	 	= (void *)stack;

	Taskid = acre_tsk(&ctsk);	

	if(Taskid <= 0)
	{
		/*FAILURE*/
		RET = -1;
	}
	else
	{
		/*SUCCESS*/
		RET = Taskid;
	}

    return RET;
}
#elif defined OS_WIN32
/*===========================================================================*/
/* ER_ID OSAL_CreateTask                                                     */
/*===========================================================================*/
Tu32 OSAL_CreateTask(Tu8 priority, OSAL_TASK_FUNC func, void* stack, Tu32 stack_size)
{
	Tu32     	u32_Taskid;
	Ts_Win32Task ctsk;

	ctsk.priority = priority;
	ctsk.func = func;
	ctsk.stacksize = stack_size;
	ctsk.stack = (void*)stack;

	u32_Taskid = WIN32_CreateTask(&ctsk);

	return u32_Taskid;
}

#endif
/*===========================================================================*/
/* ER_ID OSAL_TaskActivate                                                   */
/*===========================================================================*/
#ifdef UITRON
ER_ID OSAL_TaskActivate(ER_ID taskid)
#elif defined OS_WIN32
Ts32 OSAL_TaskActivate(Tu32 taskid)
#endif
{
	ER Taskactresult;
    ER_ID RET = 0;
#ifdef UITRON	
	Taskactresult = act_tsk(taskid);
#elif defined OS_WIN32
	Taskactresult = WIN32_TaskActivate(taskid);
#endif
	if(Taskactresult < 0)
	{
		/*FAILURE*/
		RET = -1;
	}
	else
	{
		/*SUCCESS*/
		RET = 0;
	}
    return RET;
}

/****************************DELETION OF OS RESOURCES***************************/
/*===========================================================================*/
/* Tu8 OSAL_DeleteMB(ER_ID mbx_id)                                           */
/*===========================================================================*/
#ifdef UITRON
Tu8 OSAL_DeleteMB(ER_ID mbx_id)
#elif defined OS_WIN32
Tu8 OSAL_DeleteMB(Tu32 *mbx_id)
#endif
{
	ER mbx_del_status;
	Tu8 u8_ret;
#ifdef UITRON		
	mbx_del_status = del_mbx ( mbx_id);
#elif defined OS_WIN32
	mbx_del_status = WIN32_Delete_MailBox(mbx_id);
#endif
	
	if(mbx_del_status == (ER)0)
	{
		/*SUCCESS*/
		u8_ret = 0;
	}
	else
	{
		/*FAILURE*/
		u8_ret = (Tu8)mbx_del_status;
	}
    return u8_ret;
}

/*===========================================================================*/
/* Tu8 OSAL_DeleteSemaphore(ER_ID sem_id)                                    */
/*===========================================================================*/
Tu8 OSAL_DeleteSemaphore(ER_ID sem_id)
{
	ER sem_del_status;
	Tu8 u8_ret;
#ifdef UITRON
	sem_del_status =  del_sem (sem_id);
#elif defined OS_WIN32
	sem_del_status = WIN32_DeleteSemaphore(sem_id);
#endif
	if(sem_del_status == (ER)0)
	{
		/*SUCCESS*/
		u8_ret = 0;
	}
	else
	{
		/*FAILURE*/
		u8_ret = (Tu8)sem_del_status;
	}
    return u8_ret;
}

#ifdef UITRON
/*===========================================================================*/
/* Tu8 OSAL_DeleteMemPool(ER_ID mpf_id)                                      */
/*===========================================================================*/
Tu8 OSAL_DeleteMemPool(ER_ID mpf_id)
{
	ER mpf_del_status = 0;
	Tu8 u8_ret = 0;

	mpf_del_status =  del_mpf ( mpf_id);
	
	if(mpf_del_status == (ER)0)
	{
		u8_ret = 0;
	}
	else
	{
		u8_ret = (Tu8)mpf_del_status;
	}
    return u8_ret;
}
#endif
/*===========================================================================*/
/* Tu8 RTOS_DeleteMutex(ER_ID   MutexId)                                  	 */
/*===========================================================================*/
Tu8 OSAL_DeleteMutex(ER_ID  MutexId)
{
	Tu8 RET = 0;
#ifdef UITRON
	if ((del_mtx(MutexId)) != E_OK)
#elif defined OS_WIN32
	if (WIN32_Delete_Mutex(MutexId) != E_OK)
#endif
	{
		/* FAILURE */
		RET = 1;
	}
	else
	{
		/* SUCCESS */
		RET = E_OK;
	}
	return RET;
}

/*===========================================================================*/
/* Tu8 OSAL_DeleteTask(ER_ID tsk_id)                                      	 */
/*===========================================================================*/
Tu8 OSAL_DeleteTask(ER_ID tsk_id)
{
	ER task_del_status;
	Tu8 u8_ret;
#ifdef UITRON
	task_del_status =  del_tsk ( tsk_id);
#elif defined OS_WIN32
	task_del_status = WIN32_DeleteTask(tsk_id);
#endif	
	if(task_del_status == (ER)0)
	{
		/*SUCCESS*/
		u8_ret = 0;
	}
	else
	{
		/*FAILURE*/
		u8_ret = (Tu8)task_del_status;
	}
    return u8_ret;
}

/****************************PUT TASKS IN SLEEP ******************************/
/*===========================================================================*/
/* Tu8 OSAL_Slp_Task(ER_ID tsk_id)                                      	 */
/*===========================================================================*/
Tu8 OSAL_Terminate_Task(ER_ID tsk_id)
{
	ER task_ter_status;
	Tu8 u8_ret;
#ifdef UITRON
	task_ter_status	= ter_tsk (tsk_id);
#elif defined OS_WIN32
	task_ter_status = WIN32_Terminate_Task(tsk_id);
#endif
	
	if(task_ter_status == (ER)0)
	{
		/*SUCCESS*/
		u8_ret = 0;
	}
	else
	{
		/*FAILURE*/
		u8_ret = (Tu8)task_ter_status;
	}
    return u8_ret;
}

/*===========================================================================*/
/* Tu8 OSAL_DeleteEvent(ER_ID sem_id)										 */
/*===========================================================================*/
Tu8 OSAL_DeleteEvent(Tu32 eventId)
{
	Tbool event_del_status;

	event_del_status = WIN32_DeleteEvent(eventId);

	return (Tu8)event_del_status;
}

/*===========================================================================*/
/* Tbool OSAL_IsThreadTerminated(Tu32 eventID)								 */
/*===========================================================================*/
Tbool OSAL_IsThreadTerminated(Tu32 eventID)
{
	Tbool b_threadStatus;

	b_threadStatus = WIN32_IsThreadTerminated(eventID);

	return b_threadStatus;
}

/*=============================================================================
    end of file
=============================================================================*/
