/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file sys_message.c																				    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains definitions of sys message APIs								*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/

#include "sys_message.h"


/*-----------------------------------------------------------------------------
variables (Global)
-----------------------------------------------------------------------------*/

CRITICAL_SECTION msg_handle;

/*-----------------------------------------------------------------------------
variables (static)
-----------------------------------------------------------------------------*/

static Ts_Sys_Msg		SYS_MSG_dummy_msg;						/* dummy message which is used to delete a message from a mailbox */

static Ts_Msg_Pool		SYS_MSG_MsgSlots[MAX_MSG_POOL_SIZE];	/* array with messages           */
static Ts_Msg_Pool*	SYS_MSG_FirstFreeSlot	= NULL;             /* first free slot in the array  */
static Ts_Msg_Pool*	SYS_MSG_LastFreeSlot	= NULL;             /* last free slot in the array   */

/*-----------------------------------------------------------------------------
private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void SYS_MSG_Init(void)                                                  */
/*===========================================================================*/
void SYS_MSG_Init(void)
{
	Tu16 Tu16_lTemp;

	InitializeCriticalSection(&msg_handle);

	SYS_MSG_InitMsg(&SYS_MSG_dummy_msg);

	SYS_MSG_FirstFreeSlot = NULL;
	SYS_MSG_LastFreeSlot = NULL;

	for (Tu16_lTemp = 0U; Tu16_lTemp < MAX_MSG_POOL_SIZE; Tu16_lTemp++)
	{
		SYS_MSG_InitMsg(&SYS_MSG_MsgSlots[Tu16_lTemp].msg);
		SYS_MSG_AddFreeSlot(&SYS_MSG_MsgSlots[Tu16_lTemp]);
	}
}

/*===========================================================================*/
/*   void SYS_MSG_InitMsg(Ts_Sys_Msg* msg)                                   */
/*===========================================================================*/
void SYS_MSG_InitMsg(Ts_Sys_Msg* msg)
{
	msg->dest_id	= 0U;
	msg->src_id		= 0U;
	msg->msg_id		= 0U;
	msg->msg_length = 0U;

	WIN32_MemorySet(msg->data, 0U, SYS_MSG_SIZE);
	
}

/*===========================================================================*/
/*   void SYS_MSG_AddFreeSlot(Ts_Msg_Pool* slot)                             */
/*===========================================================================*/
void SYS_MSG_AddFreeSlot(Ts_Msg_Pool* slot)
{
	EnterCriticalSection(&msg_handle);

	slot->status = SYS_MSG_IN_FREE_QUEUE;
	slot->nextFreeSlot = NULL;

	if (SYS_MSG_LastFreeSlot != NULL)
	{
		SYS_MSG_LastFreeSlot->nextFreeSlot = slot;
	}

	SYS_MSG_LastFreeSlot = slot;

	if (SYS_MSG_FirstFreeSlot == NULL)
	{
		SYS_MSG_FirstFreeSlot = slot;
	}

	LeaveCriticalSection(&msg_handle);
}

/*===========================================================================*/
/*  void SYS_MSG_FreeParamSlot(Ts_Sys_Msg* msg)                              */
/*===========================================================================*/
void SYS_MSG_FreeParamSlot(Ts_Sys_Msg* msg)
{
	WIN32_MemorySet(msg->data, 0U, SYS_MSG_SIZE);			// To be revisited
}

/*===========================================================================*/
/*  void SYS_MSG_FreeMsgSlot(Ts_Msg_Pool* slot)                              */
/*===========================================================================*/
void SYS_MSG_FreeMsgSlot(Ts_Msg_Pool* slot)
{
	if (slot->status != SYS_MSG_USED)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][FW][SYS_MSG] Message should be used");
	}
	else
	{
		/* For MISRA */
	}

	slot->status = SYS_MSG_STATE_FREE;
}

/*===========================================================================*/
/*  Ts_Sys_Msg* SYS_MSG_Alloc(void)                                          */
/*===========================================================================*/
Ts_Sys_Msg* SYS_MSG_Alloc(void)
{
	Ts_Msg_Pool* slot;

	EnterCriticalSection(&msg_handle);

	if (SYS_MSG_FirstFreeSlot == NULL)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][FW][SYS_MSG] Message slot is not free");
		return NULL;
	}
	else
	{
		slot = SYS_MSG_FirstFreeSlot;
		SYS_MSG_FirstFreeSlot = SYS_MSG_FirstFreeSlot->nextFreeSlot;

		LeaveCriticalSection(&msg_handle);

		SYS_MSG_InitMsg(&slot->msg);

		slot->status = SYS_MSG_USED;
		slot->nextFreeSlot = NULL;
		return &slot->msg;
	}
}

/*===========================================================================*/
/*  Tbool SYS_MSG_Free(Ts_Sys_Msg* msg)                                      */
/*===========================================================================*/
Tbool SYS_MSG_Free(Ts_Sys_Msg* msg)
{
	/*lint -save -e946 Relational or subtract operator applied to pointers [MISRA 2004 Rule 17.2, required], [MISRA 2004 Rule 17.3, required] */
	if (((void*)msg >= (void*)&SYS_MSG_MsgSlots[0]) && ((void*)msg <= (void*)&SYS_MSG_MsgSlots[MAX_MSG_POOL_SIZE]))
	{
		Ts_Msg_Pool* slot = (Ts_Msg_Pool*)msg;

		SYS_MSG_FreeParamSlot(msg);
		
		EnterCriticalSection(&msg_handle);

		msg->dest_id	= 0U;
		msg->src_id		= 0U;
		msg->msg_id		= 0U;
		msg->msg_length = 0U;

		SYS_MSG_FreeMsgSlot(slot);

		SYS_MSG_AddFreeSlot(slot);		

		LeaveCriticalSection(&msg_handle);

		return TRUE;
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][FW][SYS_MSG] Message free is not done due to error. Out of boundary");
		return FALSE;
	}
}
/*=============================================================================
end of file
=============================================================================*/