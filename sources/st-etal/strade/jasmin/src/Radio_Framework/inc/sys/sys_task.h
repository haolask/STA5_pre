/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_task.h																				    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of osal task private								*
*																											*
*************************************************************************************************************/

#ifndef _SYS_TASK_H
#define _SYS_TASK_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "sys_timer.h"
#include "osal_api.h"
#include "cfg_variant_market.h"
#include "lib_string.h"
#ifdef UITRON
#include "app_tuner.h"
#elif defined OS_WIN32
#include "win32_os_api.h"
#endif

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#ifdef UITRON
#define SYS_MSG_HEADER_SIZE 		(sizeof(T_MSG)+sizeof(Tu16)+sizeof(Tu16)+sizeof(Tu16)+sizeof(Tu16)) /*Total header size is 12*/
#define SYS_MSG_SIZE 				(Tu8)(MEMPOOL_BLK_SZ - SYS_MSG_HEADER_SIZE)							/*Mempool size - 256 ,Message size - 244 */
#elif defined OS_WIN32
#define SYS_MSG_SIZE 			(Tu16)350															/*In Message Data slot size - changed from 256 to 350 as frequencies are of type Tu32 */
#endif

#define SYS_SEND_MSG  				Sys_Send_Msg														/*Macro for Sys_Send_Msg function*/
#define SYS_RECEIVE_MSG  			Sys_Rcv_Msg															/*Macro for Sys_Rcv_Msg function*/
#define SYS_MUTEX_LOCK  			Sys_Mutex_Lock														/*Macro for Sys_Mutex_Lock function*/
#define SYS_MUTEX_UNLOCK  			Sys_Mutex_Unlock													/*Macro for Sys_Mutex_Unlock function*/
#define OS_RESOURCE_CREATE_SUCCESS	(Tu8)0x00														  	/* Success for Resource creation */
#define OS_RESOURCE_CREATE_ERR		(Tu8)0x01  															/* Failure for Resource creation */
#define OS_RESOURCE_RELEASE_SUCCESS (Tu8)0x00															/* Success for Resource Release */
#define OS_RESOURCE_RELEASE_ERR		(Tu8)0x01															/* Failure for Resource Release */

#ifdef UITRON
#define TASK_NAME_SIZE			0x15    /* Size of task name 		 */
#define RADIO_MGR_TASK_PRI  	9  		/* @note 20160614 Temporary solution, other tasks are not scheduled */
#define TUNE_CTRL_TASK_PRI  	9  		/* @note 20160614 Temporary solution, other tasks are not scheduled */
#define AMFM_TASK_PRI  			9  		/* @note 20160614 Temporary solution, other tasks are not scheduled */
#define FMRDS_TASK_PRI			9 		/* @note 20160614 Temporary solution, other tasks are not scheduled */

#elif defined OS_WIN32
#define RADIO_MGR_TASK_PRI  	THREAD_PRIORITY_ABOVE_NORMAL  		/* Process's Default Priority Class - NORMAL_PRIORITY_CLASS , Priority level - THREAD_PRIORITY_ABOVE_NORMAL*/	
#define TUNE_CTRL_TASK_PRI  	THREAD_PRIORITY_ABOVE_NORMAL  		
#define AMFM_TASK_PRI  			THREAD_PRIORITY_ABOVE_NORMAL  		
#define FMRDS_TASK_PRI			THREAD_PRIORITY_ABOVE_NORMAL 		

/* Mailbox buffer size */
#define T0_MB_SIZE				(Tu16)256
#define T1_MB_SIZE				(Tu16)256
#define T2_MB_SIZE				(Tu16)256
#define T3_MB_SIZE				(Tu16)256
#endif

#define RADIO_MGR_TASK_STK_SZ  	3000  	/* osal_sys_task1 stack size 3000*/
#define TUNE_CTRL_TASK_STK_SZ  	3000  	/* osal_sys_task2 stack size 2048 */
#define AMFM_TASK_STK_SZ  		2048  	/* osal_sys_task3 stack size 2048 */
#define FMRDS_TASK_STK_SZ  		2048  	/* osal_sys_task4 stack size 2048 */

/* DEBUG CODE TO TRACK MSG ID */
#define MAX_NUM_OF_MSG			0x05	/* Max no. of msgs storing for calibration tool */

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
typedef struct Ts_sys_msg
{	
	T_MSG  	msg;						/* T_MSG for message header 	*/
	Tu16	src_id;						/* source id 					*/
	Tu16 	dest_id;					/* destination id 				*/
	Tu16	msg_id;						/* message id 					*/
	Tu16 	msg_length; 				/* message length 				*/		
	Tchar 	data[SYS_MSG_SIZE]; 	/* message data 				*/
}Ts_Sys_Msg;

#ifdef UITRON
typedef struct Ts_Sys_Task
{
	unsigned char	task_priority;				/* TASK PRIORITY*/
	void			*task_funcptr;   			/* TASK FUNCTION*/
	void			*task_stackptr;   			/* STACK POINTER*/
	unsigned int	stack_size;     			/* STACK SIZE*/
	unsigned char   task_name[TASK_NAME_SIZE];  /* TASK NAME*/
} Ts_System_Task;
#elif defined OS_WIN32
typedef struct Ts_Sys_Task
{
	Tu8                 task_priority;			/* system priority					*/
	DWORD               threadid;				/* the thread's id					*/
	Tu32                *mbbuf;					/* pointer to the mailbox			*/
	Tu16                mbsize;					/* sizeof the mailbox				*/
	OSAL_TASK_FUNC      task_funcptr;           /* pointer to the task fuction		*/
	void                *task_stackptr;         /* stack pointer					*/
	Tu32                stack_size;				/* the thread's stack size			*/
	const char*			task_name;				/* task name						*/
} Ts_System_Task;
#endif

/* DEBUG CODE TO TRACK MSG ID */
typedef struct Ts_debug_msg
{
	Tu16	msg_id[MAX_NUM_OF_MSG];
	Tu16	dest_id[MAX_NUM_OF_MSG];
}Ts_Debug_msg;


/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    function declarations extern
-----------------------------------------------------------------------------*/
extern void Radio_Mngr_App_Msg_HandleMsg(Ts_Sys_Msg* pst_msg);
extern void AMFM_APP_HSM_MessageHandler(Ts_Sys_Msg *pst_msg);
extern void DAB_APP_MSG_HandleMsg(Ts_Sys_Msg* pst_msg);
extern void AMFM_TUNER_CTRL_MSG_HandleMsg(Ts_Sys_Msg* pst_msg);
extern void DAB_Tuner_Ctrl_MsgHandlr(Ts_Sys_Msg* msg);
extern void FM_RDS_DECODER_HandleMsg(Ts_Sys_Msg* pst_msg);
extern unsigned char Radio_Sys_Task_Create(Ts_System_Task* pst_system_task_info);
extern void DAB_HAL_SPI_Read_Handler(void);
extern void osal_sys_task1(void);
extern void osal_sys_task2(void);
extern void osal_sys_task3(void);
extern void osal_sys_task4(void);
/*-----------------------------------------------------------------------------
    function declarations intern
-----------------------------------------------------------------------------*/
#ifdef UITRON
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Get_MemPtr is get memory pointer from mailbox
*   \param[in]				ER_ID mpfid --memorypool id 
*                           Ts_Sys_Msg **snd_msg -- structure pointer
*   \param[out]				ER
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to get memory pointer from mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
* 
******************************************************************************************************/
ER Sys_Get_MemPtr(ER_ID mpfid, Ts_Sys_Msg **pst_snd_msg);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Rel_MemPtr release memory pointer.  
*   \param[in]				ER_ID mpfid --memorypoolid
*                           Ts_Sys_Msg *rcv_msg ---structure pointer
*   \param[out]				ER
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API is used to release memory pointer from mailbox 
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
ER Sys_Rel_MemPtr(ER_ID mpfid, Ts_Sys_Msg *pst_rcv_msg);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Send_Msg send message to mailbox based on dest_id.  
*   \param[in]				Ts_Sys_Msg *snd_msg --structure pointer
*   \param[out]				ER
*   \pre-condition			OSAL layer must be fully operational.
*   \details                Sys_Send_Msg is a function to send message to mailbox.
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Send_Msg(Ts_Sys_Msg *pst_snd_msg);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Rcv_Msg receive message from mailbox based on dest_id.  
*   \param[in]				ER_ID taskid -- Taskid
*                           Ts_Sys_Msg **rcv_msg --- structure pointer
*   \param[out]				ER
*   \pre-condition			OSAL layer must be fully operational.
*   \details                Sys_Rcv_Msg is a function to receive message from mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
ER Sys_Rcv_Msg(ER_ID taskid, Ts_Sys_Msg **pst_rcv_msg);

#elif defined OS_WIN32
/*****************************************************************************************************/
/**	 \brief                 The API Function  Sys_Get_MemPtr is get memory pointer from mailbox
*   \param[in]				Ts_Sys_Msg **snd_msg -- structure pointer
*   \param[out]				Ts8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to get memory pointer from mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts8 Sys_Get_MemPtr(Ts_Sys_Msg **pst_snd_msg);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Rel_MemPtr release memory pointer.
*   \param[in]				Ts_Sys_Msg *rcv_msg ---structure pointer
*   \param[out]				Ts8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API is used to release memory pointer from mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts8 Sys_Rel_MemPtr(Ts_Sys_Msg *pst_rcv_msg);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Send_Msg send message to mailbox based on dest_id.
*   \param[in]				Ts_Sys_Msg *snd_msg --structure pointer
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                Sys_Send_Msg is a function to send message to mailbox.
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Send_Msg(Ts_Sys_Msg *pst_snd_msg);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Rcv_Msg receive message from mailbox based on dest_id.
*   \param[in]				Tu32 taskid -- Taskid
*                           Ts_Sys_Msg **rcv_msg --- structure pointer
*   \param[out]				Ts32
*   \pre-condition			OSAL layer must be fully operational.
*   \details                Sys_Rcv_Msg is a function to receive message from mailbox
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts32 Sys_Rcv_Msg(Tu32 taskid, Ts_Sys_Msg **pst_rcv_msg);
#endif
/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Mutex_Lock lock source.
*   \param[in]				Tu16 SrcId -- sourceid
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                Sys_Mutex_Lock is a function to lock the source
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Mutex_Lock(Tu16 u16_srcId);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_Mutex_Unlock unlock source.
*   \param[in]				None
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                Sys_Mutex_Unlock is a function to lock the source
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_Mutex_Unlock(Tu16 u16_srcId);

/*****************************************************************************************************/
/**	 \brief                 This API function Radio_Sys_Task_Delete is to delete number of tasks.
*   \param[in]				void
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API will release the number of tasks are created.
*   \post-condition			Given number of tasks.
*   \ErrorHandling    		Return Resoure release SUCCESS
*												   FAILURE
*
******************************************************************************************************/
Tu8 Radio_Sys_Task_Delete(void);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_num_of_tasks is to get number of tasks.
*   \param[in]				Ts_variant_info st_variant_data
*   \param[out]				Tu8
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API will update the number of tasks.
*   \post-condition			Given number of tasks.
*   \ErrorHandling    		Return number of tasks for particular variant and market.
*
******************************************************************************************************/
Tu8 Sys_num_of_tasks(void);

/*****************************************************************************************************/
/**	 \brief                 This API function Sys_details_task_index is to update task index array.
*   \param[in]				void
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API update the task index array, to fetch tasks attributes.
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void Sys_task_array_database(void);


unsigned char SYS_TASK_CreateTasks(void);

/*****************************************************************************************************/
/**	 \brief                 This API function TaskCleanup is to cleanup a task before deletion.
*   \param[in]				Ts_task_info* st_sys_task_info
*   \param[out]				None
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API resets the task specific attributes.
*   \post-condition			Task cleanup is done and is ready for termination.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void TaskCleanup(Ts_task_info* st_sys_task_info);


#endif /* SYS_TASK_H */
/*=============================================================================
    end of file
=============================================================================*/


