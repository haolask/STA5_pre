/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_task.c																				    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains sys task related API definitions								*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
//#include "app_tuner.h"
#include "sys_task.h"
#include "sys_message.h"
#ifdef UITRON
#include "iodefine.h"
#include "sw_dab_drv.h"
#endif
#include "Tuner_core_sys_main.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
extern Te_DAB_Status e_dab_status; 
extern Tu8           u8_rds_status; 
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (Global)
-----------------------------------------------------------------------------*/
ER_ID 			MtxInfo[MAX_MUTEX_COUNT];		/* DATA BASE FOR MUTEX ID'S*/
Tu8          	au8_task_index[SYS_TASK_CNT];   /* TO FETCH TASK ATTRIBUTES*/
Ts_task_info	st_sys_task_info[SYS_TASK_CNT];	/* DATA BASE FOR TASKS AND MBX*/
Tu8				u8_max_tsk_cnt;				    /* NUMBER OF TASKS TO CREATE*/
Tu8 			u8_max_mutex_cnt;				/* NUMBER OF MUTEX TO CREATE*/

/* CALIBRATION TOOL STUFF */
Tu32 u32_Mempool_Overflow_Count = 0;			/* MEMPOOL OVERFLOW COUNT */
Tu16 u16_Overflow_Srcid			= 0;			/* OVERFLOWED MSG SOURCE ID */
Tu16 u16_Overflow_Msgid			= 0;			/* OVERFLOWED MSG MESSAGE ID */
Tu16 u16_Overflow_Destid		= 0;			/* OVERFLOWED MSG DESTINATION ID */

/* DEBUG CODE FOR MSG TRACK */
Ts_Debug_msg	st_debug_snd;					/* TO TRACK SENT MESSAGES */
Ts_Debug_msg	st_debug_rcv;					/* TO TRACK RECEIVED MESSAGES */
Tu8 			u8_snd_msg		= 0;			/* INDEX FOR SENT MESSAGES */
Tu8 			u8_rcv_msg		= 0;			/* INDEX FOR RECEIVED MESSAGES */

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/
#ifdef UITRON
Ts_System_Task ast_system_task_info[] = 
{
	{RADIO_MGR_TASK_PRI, 	osal_sys_task1,  NULL,	RADIO_MGR_TASK_STK_SZ,	"APPLICATION TASK"     },
	{TUNE_CTRL_TASK_PRI, 	osal_sys_task2,  NULL,	TUNE_CTRL_TASK_STK_SZ,	"DAB_TUNER_CTRL TASK"  },
	{AMFM_TASK_PRI, 		osal_sys_task3,  NULL,	AMFM_TASK_STK_SZ,		"AMFM_TUNER_CTRL TASK" },
	{FMRDS_TASK_PRI, 		osal_sys_task4,  NULL,	FMRDS_TASK_STK_SZ,		"RDS_TASK"       	   }
};
#elif defined OS_WIN32
/* Mailbox buffer to hold the pointers to the messages posed in mailbox */
Ts_Sys_Msg* T0_mb_buffer[T0_MB_SIZE];
Ts_Sys_Msg* T1_mb_buffer[T1_MB_SIZE];
Ts_Sys_Msg* T2_mb_buffer[T2_MB_SIZE];
Ts_Sys_Msg* T3_mb_buffer[T3_MB_SIZE];

Ts_System_Task ast_system_task_info[] =
{
	{ RADIO_MGR_TASK_PRI,	0U, (Tu32*)T0_mb_buffer, T0_MB_SIZE, osal_sys_task1, NULL, RADIO_MGR_TASK_STK_SZ,	"APPLICATION TASK"		},
	{ TUNE_CTRL_TASK_PRI,	0U, (Tu32*)T1_mb_buffer, T1_MB_SIZE, osal_sys_task2, NULL, TUNE_CTRL_TASK_STK_SZ,	"DAB_TUNER_CTRL TASK"	},
	{ AMFM_TASK_PRI,		0U, (Tu32*)T2_mb_buffer, T2_MB_SIZE, osal_sys_task3, NULL, AMFM_TASK_STK_SZ,		"AMFM_TUNER_CTRL TASK"	},
	{ FMRDS_TASK_PRI,		0U, (Tu32*)T3_mb_buffer, T3_MB_SIZE, osal_sys_task4, NULL, FMRDS_TASK_STK_SZ,		"RDS_TASK"				}
};
#endif

/*===========================================================================*/
/*  void SYS_TASK_CreateTasks                                                */
/*===========================================================================*/
unsigned char SYS_TASK_CreateTasks(void)
{
	unsigned char u8_task_cre;
#ifdef UITRON
	/*For Task creation providing task attributes */	
	u8_task_cre	= Radio_Sys_Task_Create(&ast_system_task_info[0]); 
#elif defined OS_WIN32
	Tu32 u32_ret = 0u;
	/* Static message pool creation */
	SYS_MSG_Init();

	/* Timer Queue Initialization */
	u32_ret = WIN32_Timer_Init();
	if (u32_ret == 0)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] TimerQueue Creation is failed \n ");
	}
	else
	{
		/*MISRA*/
	}

	/*For Task creation providing task attributes */
	u8_task_cre = Radio_Sys_Task_Create(&ast_system_task_info[0]);
#endif
	return u8_task_cre;
}

/*===========================================================================*/
/*  ER Sys_Get_MemPtr                                                        */
/*===========================================================================*/
#ifdef UITRON
ER Sys_Get_MemPtr(ER_ID mpfid, Ts_Sys_Msg **pst_snd_msg)
#elif defined OS_WIN32
Ts8 Sys_Get_MemPtr(Ts_Sys_Msg **pst_snd_msg)
#endif
{	
#ifdef UITRON
	return pget_mpf((ID)mpfid,(VP *)pst_snd_msg);
#elif defined OS_WIN32
	*pst_snd_msg = SYS_MSG_Alloc();

	if (*pst_snd_msg != NULL)
	{
		return 0;
	}
	else
	{
		return -1;
	}
#endif
}
/*===========================================================================*/
/*  ER Sys_Rel_MemPtr                                                        */
/*===========================================================================*/
#ifdef UITRON
ER Sys_Rel_MemPtr(ER_ID mpfid, Ts_Sys_Msg *pst_rcv_msg)
#elif defined OS_WIN32
Ts8 Sys_Rel_MemPtr(Ts_Sys_Msg *pst_rcv_msg)
#endif
{	
#ifdef UITRON
	return rel_mpf((ID)mpfid,(VP)pst_rcv_msg);
#elif defined OS_WIN32
	Tbool RET;

	RET = SYS_MSG_Free(pst_rcv_msg);

	if (RET == TRUE)
	{
		return 1;
	}
	else
	{
		return -1;
	}
#endif
}
/*===========================================================================*/
/*  ER Sys_Send_Msg                                                          */
/*===========================================================================*/
void Sys_Send_Msg(Ts_Sys_Msg *pst_snd_msg)
{	
/** Local variable declarations, using for find taskid and component **/	
    Tu8 		u8_taskCnt;	
    Tu8 		u8_compCnt ;
	Ts_Sys_Msg	*pst_mempoolPtr =  NULL;
    Tu8 		u8_compFnd	= 0;   
    ER 			ER_REL 		= 0;
    Tu8 		u8_tcnt 	= 0;
	Tlong 		long_ret 	= 0; 
	
    for(u8_taskCnt = 0; u8_taskCnt < SYS_TASK_CNT; u8_taskCnt++)
    {
        for(u8_compCnt = 0; u8_compCnt < st_sys_task_info[u8_taskCnt].Num_of_Comp; u8_compCnt++)
        {
            if(pst_snd_msg->dest_id == st_sys_task_info[u8_taskCnt].Comp_Info[u8_compCnt])
            {
                u8_compFnd = TRUE;
            	u8_compCnt = st_sys_task_info[u8_taskCnt].Num_of_Comp;
            }
			else
			{
				/*Do nothing */
			}			
        }		
        if(u8_compFnd == TRUE)
        {
            u8_tcnt 	= u8_taskCnt;
			u8_taskCnt	= SYS_TASK_CNT;        	
        }
		else
		{
			/*Do nothing */	
		}
    }	
    if(u8_compFnd == TRUE)
    { 		
#ifdef UITRON
        long_ret = Sys_Get_MemPtr(st_sys_task_info[u8_tcnt].mpfid, &pst_mempoolPtr);
#elif defined OS_WIN32
		long_ret = Sys_Get_MemPtr(&pst_mempoolPtr);
#endif
        if(long_ret == E_OK)
        {	
           SYS_RADIO_MEMCPY(pst_mempoolPtr, pst_snd_msg, sizeof(Ts_Sys_Msg));			
            pst_mempoolPtr->msg.msghead = (VP)NULL;		/* fixed zero */
#ifdef UITRON
            ER_REL = isnd_mbx((ID)st_sys_task_info[u8_tcnt].mbxid, (T_MSG*)pst_mempoolPtr);
#elif defined OS_WIN32
			ER_REL = WIN32_PutMessage((Ts_MailBox*)st_sys_task_info[u8_tcnt].mbxid, pst_mempoolPtr);
#endif

            if(ER_REL == E_OK)
            {
				if (u8_snd_msg < MAX_NUM_OF_MSG)
				{
					st_debug_snd.msg_id[u8_snd_msg] = pst_snd_msg->msg_id;
					st_debug_snd.dest_id[u8_snd_msg] = pst_snd_msg->dest_id;
					u8_snd_msg++;
				}
				else
				{
					u8_snd_msg = 0;
					st_debug_snd.msg_id[u8_snd_msg] = pst_snd_msg->msg_id;
					st_debug_snd.dest_id[u8_snd_msg] = pst_snd_msg->dest_id;
					u8_snd_msg++;
				}
				//RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] isnd_mbx is success for Src_id:%x, Dest_id:%x, Msg_id:%x\n ",pst_snd_msg->src_id, pst_snd_msg->dest_id, pst_snd_msg->msg_id);
            }
			else
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] isnd_mbx is failed for Src_id:%x, Dest_id:%x, Msg_id:%x\n ",pst_snd_msg->src_id, pst_snd_msg->dest_id, pst_snd_msg->msg_id);
			}
        }
		else
		{
		
			u32_Mempool_Overflow_Count++;
   			u16_Overflow_Srcid = pst_snd_msg->src_id;
			u16_Overflow_Msgid = pst_snd_msg->msg_id;
			u16_Overflow_Destid = pst_snd_msg->dest_id;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][FW] Sys_Get_MemPtr is failed for Src_id:%x, Dest_id:%x, Msg_id:%x\n ",pst_snd_msg->src_id, pst_snd_msg->dest_id, pst_snd_msg->msg_id);
		}
    }
	else
	{
		/* Do nothing */
	}
}
/*===========================================================================*/
/*  ER Sys_Rcv_Msg                                                           */
/*===========================================================================*/
#ifdef UITRON
ER Sys_Rcv_Msg(ER_ID taskid, Ts_Sys_Msg** pst_rcv_msg)
#elif defined OS_WIN32
Ts32 Sys_Rcv_Msg(Tu32 taskid, Ts_Sys_Msg **pst_rcv_msg)
#endif
{
/** Local variable declarations, using for find taskid **/	
	Tu8		u8_taskCount ;
    ER 		ER_REL	  = -1;
	ER		ER_RetVal = -1;	
	
	for(u8_taskCount = 0; u8_taskCount < SYS_TASK_CNT; u8_taskCount++)
	{
        if(taskid == st_sys_task_info[u8_taskCount].taskid)
        {   
#ifdef UITRON
            ER_REL = rcv_mbx((ID)st_sys_task_info[u8_taskCount].mbxid, (T_MSG**)pst_rcv_msg);
#elif defined OS_WIN32
			ER_REL = WIN32_GetMessage((Ts_MailBox *)st_sys_task_info[u8_taskCount].mbxid, (Ts_Sys_Msg**)pst_rcv_msg);
#endif
            if(ER_REL == E_OK)
            {
                if(pst_rcv_msg != NULL)
                {
	                ER_RetVal = E_OK;
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] rcv_mbx is success for Src_id:%x, Dest_id:%x, Msg_id:%x\n ",(*pst_rcv_msg)->src_id, (*pst_rcv_msg)->dest_id, (*pst_rcv_msg)->msg_id);
                }
                else
                {
					/*Do Nothing*/
                }	
            }
			else
			{
				/* No message to read */
			}
			break;
        }
        else
        {
			/*Do Nothing*/
        }
	}		
	return ER_RetVal;	
}
/*===========================================================================*/
/*  void Sys_Mutex_Lock                                                      */
/*===========================================================================*/
void Sys_Mutex_Lock(Tu16 u16_SrcId)
{
	/** Local varible declaration,to update mutexid based on u16_SrcId **/
	ER_ID	Mutexid = 0;
	ER_ID RET;

	switch (u16_SrcId)
	{
	case STL_RM_AMFM_APP:
		Mutexid = MtxInfo[0];
		break;

	case STL_AMFM_APP_AMFM_TC:
		Mutexid = MtxInfo[1];
		break;

	case STL_RM_DAB_APP:
		Mutexid = MtxInfo[2];
		break;

	case STL_DAB_APP_DAB_TC:
		Mutexid = MtxInfo[3];
		break;
	case SLS_RM_DAB_APP:
		Mutexid = MtxInfo[4];
		break;

	case SLS_DAB_APP_DAB_TC:
		Mutexid = MtxInfo[5];
		break;

	default:
		break;
	}

	RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][FW] MUTEX LOCK Source ID: %x, MUTEX ID:%x\n", u16_SrcId, Mutexid);

	RET = OSAL_Mutex_Lock(Mutexid);
	if (RET != E_OK)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] MUTEX LOCK IS FAILED, ERROR CODE: %d, MUTEX ID:%x", RET, Mutexid);
	}
	else
	{
		/*Do Nothing*/
	}
}
/*===========================================================================*/
/*  void Sys_Mutex_Unlock                                                    */
/*===========================================================================*/
void Sys_Mutex_Unlock(Tu16 u16_SrcId)
{
	/** Local varible declaration,to update mutexid based on u16_SrcId **/
	ER_ID Mutexid = 0;
	ER_ID RET;
	switch (u16_SrcId)
	{
	case STL_RM_AMFM_APP:
		Mutexid = MtxInfo[0];
		break;

	case STL_AMFM_APP_AMFM_TC:
		Mutexid = MtxInfo[1];
		break;

	case STL_RM_DAB_APP:
		Mutexid = MtxInfo[2];
		break;

	case STL_DAB_APP_DAB_TC:
		Mutexid = MtxInfo[3];
		break;

	case SLS_RM_DAB_APP:
		Mutexid = MtxInfo[4];
		break;

	case SLS_DAB_APP_DAB_TC:
		Mutexid = MtxInfo[5];
		break;

	default:
		break;
	}

	RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][FW] MUTEX UNLOCK Source ID: %x, MUTEX ID:%x\n", u16_SrcId, Mutexid);
	RET = OSAL_Mutex_Unlock(Mutexid);
	if (RET != E_OK)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][FW] MUTEX UNLOCK IS FAILED, ERROR CODE: %d, MUTEX ID:%x", RET, Mutexid);
	}
	else
	{
		/*Do Nothing*/
	}
}
/*===========================================================================*/
/*  void osal_sys_task1      Task for Radio_Mngr,AMFM_APP ,DAB_APP           */
/*===========================================================================*/
void osal_sys_task1(void)
{
/** Local varible declaration and message structure declaration,to receive message from mailbox **/
		
	Ts_Sys_Msg	*pst_sys_rcv_msg;
    Tu8 		u8_flag		= (Tu8)1;
	ER			ER_RetVal 	= -1;
	Tbool		eventRet;

	while(u8_flag)
	{
		eventRet = OSAL_IsThreadTerminated(st_sys_task_info[0].terminateEvent);

		if (eventRet == OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] Terminating Task 1...\n");

			TaskCleanup(&st_sys_task_info[0]);

			break;
		}
		else
		{
			/* For MISRA */
		}

        ER_RetVal = Sys_Rcv_Msg(st_sys_task_info[0].taskid,&pst_sys_rcv_msg);
		if(ER_RetVal == E_OK)
		{
			if(pst_sys_rcv_msg != NULL)
			{
				if (u8_rcv_msg < MAX_NUM_OF_MSG)
				{
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
				else
				{
					u8_rcv_msg = 0;
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
				switch(pst_sys_rcv_msg->dest_id)
				{
					case RADIO_MNGR_APP:
						Radio_Mngr_App_Msg_HandleMsg(pst_sys_rcv_msg);
						break;
						
					case RADIO_DAB_APP:
						DAB_APP_MSG_HandleMsg(pst_sys_rcv_msg);
						break;
						
					case RADIO_AM_FM_APP:
						AMFM_APP_HSM_MessageHandler(pst_sys_rcv_msg);
						break;
						
					default: 
						break;
				}
#ifdef UITRON
				if((Sys_Rel_MemPtr(st_sys_task_info[0].mpfid, pst_sys_rcv_msg)) < 0)
#elif defined OS_WIN32
				if ((Sys_Rel_MemPtr(pst_sys_rcv_msg)) < 0)
#endif
				{					
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task1:Release Memptr is failed\n");
				}
				else
				{
					/* Do nothing*/
				}
			}
			else
			{				
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task1: System Memory Pointer is NULL \n");
			}	
		}
		else
		{
			/* Do nothing*/
		}		
	}
}
/*===========================================================================*/
/*  void osal_sys_task2       Task for DAB_TUNER_CTRL                        */
/*===========================================================================*/

void osal_sys_task2(void)
{
/** Local varible declaration and message structure declaration**/	
    
    Ts_Sys_Msg 	*pst_sys_rcv_msg;
    Tu8			u8_flag		= (Tu8)1;
	ER 			ER_RetVal 	= -1;		
	Tbool		eventRet;

	while (u8_flag)
	{
		eventRet = OSAL_IsThreadTerminated(st_sys_task_info[1].terminateEvent);

		if (eventRet == OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] Terminating Task 2...\n");

			TaskCleanup(&st_sys_task_info[1]);

			break;
		}
		else
		{
			/* For MISRA */
		}

        /** Receive message from mailbox ***/
        ER_RetVal = Sys_Rcv_Msg(st_sys_task_info[1].taskid,&pst_sys_rcv_msg);
	
        if(ER_RetVal == E_OK)
        {
            if(pst_sys_rcv_msg != NULL)
            {
				if (u8_rcv_msg < MAX_NUM_OF_MSG)
				{
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
				else
				{
					u8_rcv_msg = 0;
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
                switch(pst_sys_rcv_msg->dest_id)
                {              		
					case RADIO_DAB_TUNER_CTRL:
					{
						DAB_Tuner_Ctrl_MsgHandlr(pst_sys_rcv_msg);
					}
					break;
													
	                default: 
	                break;
                }	
#ifdef UITRON
				if ((Sys_Rel_MemPtr(st_sys_task_info[1].mpfid, pst_sys_rcv_msg)) < 0)
#elif defined OS_WIN32
				if ((Sys_Rel_MemPtr(pst_sys_rcv_msg)) < 0)
#endif
				{                   
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task2:Release Memptr is failed\n");
				}
				else
				{
					/*Do Nothing */
				}
			}
			else
			{				
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task2: System Memory Pointer is NULL \n");
			}	
		}
		else
		{
			/* Do nothing*/
		}
	}
}
/*===========================================================================*/
/*  void osal_sys_task3      Task for AMFM_TUNER_CTRL                        */
/*===========================================================================*/
void osal_sys_task3(void)
{
/** Local varible declaration and message structure declaration**/	
    	
    Ts_Sys_Msg	*pst_sys_rcv_msg;
    Tu8 		u8_flag		= (Tu8)1;
	ER 			ER_RetVal	= -1;
	Tbool		eventRet;

	while (u8_flag)
	{
		eventRet = OSAL_IsThreadTerminated(st_sys_task_info[2].terminateEvent);

		if (eventRet == OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] Terminating Task 3...\n");

			TaskCleanup(&st_sys_task_info[2]);

			break;
		}
		else
		{
			/* For MISRA */
		}

		/** Receive message from mailbox ***/
        ER_RetVal = Sys_Rcv_Msg(st_sys_task_info[2].taskid,&pst_sys_rcv_msg);
		
		if(ER_RetVal == E_OK)
		{
			if(pst_sys_rcv_msg != NULL)
			{
				if (u8_rcv_msg < MAX_NUM_OF_MSG)
				{
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
				else
				{
					u8_rcv_msg = 0;
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
				switch(pst_sys_rcv_msg->dest_id)
				{
					case RADIO_AM_FM_TUNER_CTRL:
						AMFM_TUNER_CTRL_MSG_HandleMsg(pst_sys_rcv_msg);
					break;				
									
					default: 
					break;
				}

#ifdef UITRON
				if ((Sys_Rel_MemPtr(st_sys_task_info[2].mpfid, pst_sys_rcv_msg)) < 0)
#else
				if ((Sys_Rel_MemPtr(pst_sys_rcv_msg)) < 0)
#endif
				{                   
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task3:Release Memptr is failed\n");					            
				}
				else
				{
					/* Do Nothing */
				}
			}
			else
			{				
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task3: System Memory Pointer is NULL \n");			      
			}	
		}
		else
		{
			/* Do nothing*/
		}		
	}	
}
/*===========================================================================*/
/*  void osal_sys_task4         Task for RDS                                 */
/*===========================================================================*/
void osal_sys_task4(void)
{   	
    Ts_Sys_Msg	*pst_sys_rcv_msg;
	Tu8 		u8_flag		= (Tu8)1;
	ER 			ER_RetVal	= -1;	
	Tbool		eventRet;

	while (u8_flag)
	{
		eventRet = OSAL_IsThreadTerminated(st_sys_task_info[3].terminateEvent);

		if (eventRet == OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][FW] Terminating Task 4...\n");

			TaskCleanup(&st_sys_task_info[3]);

			break;
		}
		else
		{
			/* For MISRA */
		}

		/** Receive message from mailbox ***/
        ER_RetVal = Sys_Rcv_Msg(st_sys_task_info[3].taskid,&pst_sys_rcv_msg);	
		
        if(ER_RetVal == E_OK)
        {
	        if(pst_sys_rcv_msg != NULL)
	        {
				if (u8_rcv_msg < MAX_NUM_OF_MSG)
				{
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
				else
				{
					u8_rcv_msg = 0;
					st_debug_rcv.msg_id[u8_rcv_msg] = pst_sys_rcv_msg->msg_id;
					st_debug_rcv.dest_id[u8_rcv_msg] = pst_sys_rcv_msg->dest_id;
					u8_rcv_msg++;
				}
	            switch(pst_sys_rcv_msg->dest_id)
	            {
		            case RADIO_FM_RDS_DECODER:
		            	FM_RDS_DECODER_HandleMsg(pst_sys_rcv_msg);
		            break;			
								
		            default: 
		            break;
	            } 
	
#ifdef UITRON
				if ((Sys_Rel_MemPtr(st_sys_task_info[3].mpfid, pst_sys_rcv_msg)) < 0)
#elif defined OS_WIN32
				if ((Sys_Rel_MemPtr(pst_sys_rcv_msg)) < 0)
#endif
				{               
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task4:Release Memptr is failed\n");
				}
				else
				{
					/* Do nothing */
				}
	        }
	        else
	        {			
	        	RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] osal_sys_task4: System Memory Pointer is NULL \n");	
	        }	
	    }
		else
		{
			/* Do nothing*/
		}
	}
}

/*===========================================================================*/
/*  unsigned char Radio_Sys_Task_Create(Ts_System_Task* pst_system_task_info)*/
/*===========================================================================*/
unsigned char Radio_Sys_Task_Create(Ts_System_Task* pst_system_task_info)
{
	Tu8 			u8_update_comp		= 0;
	Tu8 			u8_resource_cnt		= 0;		
	Tu8 			u8_mtx_cnt	 		= 0;
	Tu8 			u8_taskact_cntl 	= 0;
	Tu8 			u8_tsk_act			= 0;
	unsigned char	u8_ret				= OS_RESOURCE_CREATE_ERR;;		
	
	/* Finding number of tasks */    
    u8_max_tsk_cnt = Sys_num_of_tasks();
	
	/* Array value updation for task creation */
    Sys_task_array_database();
	
	/* Updating Component info database */
	for(u8_update_comp = 0; u8_update_comp < u8_max_tsk_cnt; u8_update_comp++)
	{
		switch(au8_task_index[u8_update_comp])
		{
			case 0:
			{
				if((e_dab_status == DAB_AVAILABLE)||(e_dab_status == DAB_SLEEP_STATE))
				{
					st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[0] 	= RADIO_MNGR_APP;
					st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[1] 	= RADIO_DAB_APP;
					st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[2] 	= RADIO_AM_FM_APP;
					st_sys_task_info[au8_task_index[u8_update_comp]].Num_of_Comp 	= 	3;
				}
				else
				{
					st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[0] 	= RADIO_MNGR_APP;
					st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[1] 	= RADIO_AM_FM_APP;
					st_sys_task_info[au8_task_index[u8_update_comp]].Num_of_Comp 	= 2;
				}
			}
			break;
			
			case 1:
			{
				st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[0]	= RADIO_DAB_TUNER_CTRL;
				st_sys_task_info[au8_task_index[u8_update_comp]].Num_of_Comp 	= 1;
			}
			break;

			case 2:
			{
				st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[0]	= RADIO_AM_FM_TUNER_CTRL;
				st_sys_task_info[au8_task_index[u8_update_comp]].Num_of_Comp	= 1;
			}
			break;

			case 3:
			{
				st_sys_task_info[au8_task_index[u8_update_comp]].Comp_Info[0]	= RADIO_FM_RDS_DECODER;
				st_sys_task_info[au8_task_index[u8_update_comp]].Num_of_Comp	= 1;
			}
			break;

			default:
			{
				/*Do nothing*/
			}
			break;
		}
	}
	
	/* Creating Mailbox, Semaphore, and Memorypool, Task */
	for(u8_resource_cnt =0; u8_resource_cnt < u8_max_tsk_cnt; u8_resource_cnt++)
	{
#ifdef UITRON
		if((st_sys_task_info[au8_task_index[u8_resource_cnt]].mbxid = OSAL_CreateMB()) < 0)
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].mbxid	= 0;
			u8_resource_cnt 							= u8_max_tsk_cnt;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MAILBOX:%d CREATION IS FAILED\n",u8_resource_cnt);
		}
#elif defined OS_WIN32
		if ((st_sys_task_info[au8_task_index[u8_resource_cnt]].mbxid = OSAL_CreateMB(pst_system_task_info[au8_task_index[u8_resource_cnt]].mbbuf, pst_system_task_info[au8_task_index[u8_resource_cnt]].mbsize)) == NULL)
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].mbxid = 0;
			u8_resource_cnt = u8_max_tsk_cnt;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MAILBOX:%d CREATION IS FAILED\n", u8_resource_cnt);
		}
#endif
		else if((st_sys_task_info[au8_task_index[u8_resource_cnt]].sempid = OSAL_CreateSemaphore()) == (Ts32)-1)
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].sempid	= 0;
			u8_resource_cnt 											= u8_max_tsk_cnt;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] SEMAPHORE CREATION IS FAILED\n");
		}
#ifdef UITRON
		else if((st_sys_task_info[au8_task_index[u8_resource_cnt]].mpfid = OSAL_CreateMemPool(pst_system_task_info[au8_task_index[u8_resource_cnt]].task_name)) < 0)
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].mpfid	= 0;
			u8_resource_cnt 										= u8_max_tsk_cnt;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MEMPOOL:%d CREATION IS FAILED\n",u8_resource_cnt);
		}
#endif
		else if((st_sys_task_info[au8_task_index[u8_resource_cnt]].taskid = OSAL_CreateTask(	pst_system_task_info[au8_task_index[u8_resource_cnt]].task_priority,
																								pst_system_task_info[au8_task_index[u8_resource_cnt]].task_funcptr,
																								pst_system_task_info[au8_task_index[u8_resource_cnt]].task_stackptr,
																								pst_system_task_info[au8_task_index[u8_resource_cnt]].stack_size)) == 0u)
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].taskid	= 0;
			u8_resource_cnt 											= u8_max_tsk_cnt;
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] TASK:%d CREATION IS FAILED\n",u8_resource_cnt);
		}
#ifdef OS_WIN32
		else if ((st_sys_task_info[au8_task_index[u8_resource_cnt]].terminateEvent = OSAL_CreateEvent(st_sys_task_info[au8_task_index[u8_resource_cnt]].taskid)) == OS_RESOURCE_CREATE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] EVENT CREATION FAILED\n");
		}
#endif
		else
		{
			u8_taskact_cntl++;                   
		}
	}
	
	if(u8_taskact_cntl == u8_max_tsk_cnt)
	{
		/* Mutex Creation for Radio Tuner System */
		for (u8_mtx_cnt = 0; u8_mtx_cnt < u8_max_mutex_cnt; u8_mtx_cnt++)
		{
			if ((MtxInfo[u8_mtx_cnt] = OSAL_CreateMutex()) < 0)
			{
				MtxInfo[u8_mtx_cnt] = 0;
				u8_mtx_cnt = u8_max_mutex_cnt;
				u8_ret = OS_RESOURCE_CREATE_ERR;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MUTEX:%d CREATION IS FAILED\n", u8_mtx_cnt);
			}
			else
			{
				/* Do Nothing */
			}
		}
		/* TASK ACTIVATION */
		for(u8_tsk_act = 0; u8_tsk_act < u8_max_tsk_cnt; u8_tsk_act++)
		{
			if((u8_tsk_act == (Tu8)1) &&(e_dab_status == DAB_SLEEP_STATE))
			{
				/* DAB_Tuner_Ctrl task should not be active, Variant is supporting DAB but Diag is disable for DAB */
				SYS_RADIO_MEMCPY(st_sys_task_info[u8_tsk_act].task_status,"DE_ACTIVATE",sizeof("DE_ACTIVATE"));

			}
			else
			{
				/* Activating Tasks for Radio tuner */
				if(OSAL_TaskActivate(st_sys_task_info[au8_task_index[u8_tsk_act]].taskid) < 0)
				{	
					u8_ret = OS_RESOURCE_CREATE_ERR;
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] TASK%d ACTIVATION IS FAILED\n",u8_mtx_cnt);
				}
				else
				{
					u8_ret = OS_RESOURCE_CREATE_SUCCESS;
					SYS_RADIO_MEMCPY(st_sys_task_info[u8_tsk_act].task_status,"ACTIVE",sizeof("ACTIVE"));
				}
			}
		}			
	}
	else
	{
		u8_ret = OS_RESOURCE_CREATE_ERR;
	}
	return u8_ret;	
}

/*==============================================================================*/
/* Tu8 Radio_Sys_Task_Create(void) */
/*==============================================================================*/
Tu8 Radio_Sys_Task_Delete(void)
{
	/* Local variables declaration */
	Tu8 u8_taskact_cntl 	= 0;
	Tu8 u8_resource_cnt 	= 0;
	Tu8 u8_mtx_cnt			= 0;
	Tu8 u8_mtx_dlt_cnt		= 0;
	Tu8	u8_resrc_release	= OS_RESOURCE_RELEASE_ERR;	
	
	/* Deleting Mailbox, Semaphore, and Memorypool, Task*/
	for(u8_resource_cnt =0; u8_resource_cnt < u8_max_tsk_cnt; u8_resource_cnt++)
	{
#ifndef UITRON
		if ((OSAL_DeleteTask(st_sys_task_info[au8_task_index[u8_resource_cnt]].taskid)) != OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] TASK:%d DELETION IS FAILED\n", u8_resource_cnt);
		}
		else
		{

		}
#endif
		if((OSAL_DeleteMB(st_sys_task_info[au8_task_index[u8_resource_cnt]].mbxid)) != OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MAILBOX:%d DELETION IS FAILED\n",u8_resource_cnt);
		}
		else
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].mbxid = NULL;
		}
		if((OSAL_DeleteSemaphore(st_sys_task_info[au8_task_index[u8_resource_cnt]].sempid)) != OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] SEMAPHORE DELETION IS FAILED\n");
		}
		else
		{
			st_sys_task_info[au8_task_index[u8_resource_cnt]].sempid = (Ts32)INVALID_HANDLE_VALUE;
		}
#ifdef UITRON
		if((OSAL_DeleteMemPool(st_sys_task_info[au8_task_index[u8_resource_cnt]].mpfid)) != OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MEMPOOL:%d DELETION IS FAILED\n",u8_resource_cnt);
		}

		if((OSAL_DeleteTask(st_sys_task_info[au8_task_index[u8_resource_cnt]].taskid)) != OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] TASK:%d DELETION IS FAILED\n",u8_resource_cnt);
		}
		else
		{
			u8_taskact_cntl++;
		}
#else
		u8_taskact_cntl++;
#endif
	}

	/* Mutex Deletion for Radio Tuner System */
	for (u8_mtx_cnt = 0; u8_mtx_cnt < u8_max_mutex_cnt; u8_mtx_cnt++)
	{
		if ((OSAL_DeleteMutex(MtxInfo[u8_mtx_cnt])) != OS_RESOURCE_RELEASE_SUCCESS)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ASSERT, "[RADIO][FW] MUTEX:%d DELETION IS FAILED\n", MtxInfo[u8_mtx_cnt]);
		}
		else
		{
			u8_mtx_dlt_cnt++;
		}
	}
	if ((u8_taskact_cntl == u8_max_tsk_cnt) && (u8_mtx_dlt_cnt == u8_max_mutex_cnt))
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_INFO, "[RADIO][FW] RADIO OS RESOURCES ARE RELEASED SUCCESSFULLY\n");
#ifdef UITRON
		(void)memset(st_sys_task_info, 0, sizeof(st_sys_task_info));
#endif
		u8_resrc_release = OS_RESOURCE_RELEASE_SUCCESS;
	}
	else
	{
		RADIO_DEBUG_LOG (RADIO_LOG_LVL_ASSERT, "[RADIO][FW] RADIO OS RESOURCES ARE NOT RELEASED\n");
		u8_resrc_release	= OS_RESOURCE_RELEASE_ERR;
	}
	return u8_resrc_release;
}

/*===========================================================================*/
/* 	Tu8 sys_num_of_tasks()                   								 */
/*===========================================================================*/
Tu8 Sys_num_of_tasks(void)
{
	Tu8 u8_num_of_tasks = MIN_TASK_CNT;

	if ((e_dab_status == DAB_AVAILABLE) || (e_dab_status == DAB_SLEEP_STATE))
	{
		u8_num_of_tasks += 1;  /* Task for DAB Tuner control */
		u8_max_mutex_cnt = MAX_MUTEX_COUNT;
	}
	else
	{
		u8_max_mutex_cnt = MIN_MUTEX_COUNT;
	}
	if (u8_rds_status == RDS_AVAILABLE)
	{
		u8_num_of_tasks++;
	}
	else
	{
		/*Do nothing*/
	}
	return u8_num_of_tasks;
}

/*===========================================================================*/
/* 	void sys_details_task_index(void)										 */
/*===========================================================================*/
void Sys_task_array_database(void)
{
	Tu8 u8_index = 0;
	au8_task_index[u8_index] = 0;

	if ((e_dab_status == DAB_AVAILABLE) || (e_dab_status == DAB_SLEEP_STATE))
	{
		au8_task_index[u8_index + 1] = 1;
		au8_task_index[u8_index + 2] = 2;
		if (u8_rds_status == RDS_AVAILABLE)
		{
			au8_task_index[u8_index + 3] = 3;	  /* RDS TASK */

		}
		else
		{
			/* Do nothing */
		}
	}
	else
	{
		au8_task_index[u8_index + 1] = 2;
		if (u8_rds_status == RDS_AVAILABLE)
		{
			au8_task_index[u8_index + 2] = 3;
		}
		else
		{
			/*Do nothing*/
		}
	}
}

void TaskCleanup(Ts_task_info* st_sys_tsk_info)
{
	st_sys_tsk_info->taskid = 0;
	memset(st_sys_tsk_info->Comp_Info, 0, sizeof(st_sys_tsk_info->Comp_Info) * (st_sys_tsk_info->Num_of_Comp));
	st_sys_tsk_info->Num_of_Comp = 0;
	SYS_RADIO_MEMCPY(st_sys_tsk_info->task_status, "INACTIVE", sizeof("INACTIVE"));
	if (!OSAL_DeleteEvent(st_sys_tsk_info->terminateEvent))
	{
		st_sys_tsk_info->terminateEvent = 0;
	}
	else
	{
		/* For MISRA */
	}
}

/*=============================================================================
    end of file
=============================================================================*/
