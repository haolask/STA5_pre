/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_mesage.h																		    			*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of message private								*
*																											*
*************************************************************************************************************/

#ifndef _SYS_MESSAGE_H
#define _SYS_MESSAGE_H

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "win32_os_private.h"
#include "sys_task.h"


/* ---------------------------------------------------------------------------- -
defines
---------------------------------------------------------------------------- - */
#define MAX_MSG_POOL_SIZE			(Tu16)1600	/* Max messages to be handled	*/

#define SYS_MSG_STATE_FREE          ((Tu8)0x11)		/* Message is free				*/
#define SYS_MSG_USED                ((Tu8)0x22)		/* Message is in use			*/
#define SYS_MSG_IN_FREE_QUEUE       ((Tu8)0x33)		/* Message is in free queue		*/


/*-----------------------------------------------------------------------------
type definitions
-----------------------------------------------------------------------------*/

typedef struct Ts_Msg_Pool
{
	Ts_Sys_Msg				msg;			/* the message itself */
	Tu8						status;			/* the status of the slot (FREE or USED) */
	struct Ts_Msg_Pool*     nextFreeSlot;   /* the next free slot */
}Ts_Msg_Pool;

/*-----------------------------------------------------------------------------
function declarations extern
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_Init is to Initialize Static message pool
*   \param[in]				void
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Static message pool
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MSG_Init(void);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_InitMsg is to Initialize message in message pool
*   \param[in]				Ts_Sys_Msg* msg -- pointer to message 
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to Initialize message in message pool
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MSG_InitMsg(Ts_Sys_Msg* msg);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_AddFreeSlot is to free message in message pool
*   \param[in]				Ts_Msg_Pool* slot  -- pointer to message pool
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to free message in message pool
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MSG_AddFreeSlot(Ts_Msg_Pool* slot);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_FreeParamSlot is to free message param
*   \param[in]				Ts_Sys_Msg* msg -- pointer to message
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to free message param
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MSG_FreeParamSlot(Ts_Sys_Msg* msg);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_FreeMsgSlot is to free message slot
*   \param[in]				Ts_Msg_Pool* slot  -- pointer to message pool
*   \param[out]				void
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to free message slot
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
void SYS_MSG_FreeMsgSlot(Ts_Msg_Pool* slot);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_Alloc is to allocate message from message pool
*   \param[in]				void
*   \param[out]				Ts_Sys_Msg* msg -- pointer to message
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to allocate message from message pool
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Ts_Sys_Msg* SYS_MSG_Alloc(void);

/*****************************************************************************************************/
/**	 \brief                 The API Function  SYS_MSG_Free is to free message
*   \param[in]				Ts_Sys_Msg* msg -- pointer to message
*   \param[out]				Tbool
*   \pre-condition			OSAL layer must be fully operational.
*   \details                This API function used to free message
*   \post-condition			The system starts up. Multitasking can begin.
*   \ErrorHandling    		None
*
******************************************************************************************************/
Tbool SYS_MSG_Free(Ts_Sys_Msg* msg);

#endif /* SYS_MESSAGE_H */
/*=============================================================================
end of file
=============================================================================*/