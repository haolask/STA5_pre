//!
//!  \file    rif_msg_queue.c
//!  \brief   <i><b> Radio interface message queue handling </b></i>
//!  \details This module implements the Message queue handling of the Radio interface.
//!  \author  David Pastor
//!

#include "target_config.h"
#include "target_config_radio_if.h"

#include "osal.h"

#include "rif_msg_queue.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

///
// This define shall be defined just after inlcusion of target config because 
// it is used to correctly setup printf level
///
#define CONFIG_TRACE_CLASS_MODULE       CONFIG_TRACE_CLASS_RIF_MSG_QUEUE

#define RIF_MSG_QUEUE_INDEX_INVALID     (tS16)(-1)

#define RIF_MSG_QUEUE_PREFIX_SEM        "S_"
#define RIF_MSG_QUEUE_PREFIX_EVENT      "E_"

/*!
 * \def		MAX_QUEUE_SEM_NAME
 * 			Max length of the queue semaphore name
 */
#define MAX_QUEUE_SEM_NAME      32

/*!
 * \def		MAX_QUEUE_EVENT_NAME
 * 			Max length of the queue event name
 */
#define MAX_QUEUE_EVENT_NAME      32


/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/

/*****************************************************************
| functions
|----------------------------------------------------------------*/

/***************************
 *
 * rif_msg_create_queue
 *
 **************************/
/*!
 * \brief		Creates the resources used by a msg queue
 * \details		The function creates msg queue allocated memory and the semaphore connected to the msg queue.
 * \param[in]	name - pointer of the msg queue name
 * \param[in]	max_messages - maximum number of messages
 * \return		msg_queue_p - pointer to the allocated msg queue. On error return NULL
 * \callgraph
 * \callergraph
 */
rif_msg_queue_t *rif_msg_create_queue(tChar *name, tUInt max_messages)
{
	tChar sem_name[MAX_QUEUE_SEM_NAME], event_name[MAX_QUEUE_EVENT_NAME];
	rif_msg_queue_t *msg_queue_p;

	/* check parameters */
	if ((name == NULL) || (max_messages == 0))
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	/* allocate msg queue and set memory to 0 */
	msg_queue_p = OSAL_pvMemoryCAllocate(1, sizeof(rif_msg_queue_t) + (max_messages * sizeof(tPVoid)));
	if (msg_queue_p == NULL)
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	/* initialize rif_msg_queue_t */
	msg_queue_p->name = name;
	msg_queue_p->queueSize = max_messages;

	/* create semaphore */
	if (OSAL_s32NPrintFormat(sem_name, MAX_QUEUE_SEM_NAME, "%s%s", RIF_MSG_QUEUE_PREFIX_SEM, name) < 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}
	if (OSAL_s32SemaphoreCreate(sem_name, &msg_queue_p->lock, 1) == OSAL_ERROR)
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	/* create event */
	if (OSAL_s32NPrintFormat(event_name, MAX_QUEUE_EVENT_NAME, "%s%s", RIF_MSG_QUEUE_PREFIX_EVENT, name) < 0)
	{
		if (OSAL_s32SemaphoreClose(msg_queue_p->lock) != OSAL_OK)
		{
			ASSERT_ON_DEBUGGING(0);
			return NULL;
		}
		if(OSAL_s32SemaphoreDelete(sem_name) != OSAL_OK)
		{
			ASSERT_ON_DEBUGGING(0);
			return NULL;
		}
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}
	if (OSAL_s32EventCreate(event_name, &msg_queue_p->event))
	{
		if (OSAL_s32SemaphoreClose(msg_queue_p->lock) != OSAL_OK)
		{
			ASSERT_ON_DEBUGGING(0);
			return NULL;
		}
		if(OSAL_s32SemaphoreDelete(sem_name) != OSAL_OK)
		{
			ASSERT_ON_DEBUGGING(0);
			return NULL;
		}
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, TR_CLASS_EXTERNAL, "rif_msg_create_queue address: 0x%x  name: %s  rd: %d  wr: %d", 
		msg_queue_p, msg_queue_p->name, msg_queue_p->readPtr, msg_queue_p->writePtr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	return msg_queue_p;
}

/***************************
 *
 * rif_msg_delete_queue
 *
 **************************/
/*!
 * \brief		Releases the resources used by a msg queue pointer
 * \details		The function destroys the semaphore connected to the msg queue and free the msg queue.
 * \param[in]	msg_queue_p - pointer of the msg queue to release
 * \return		OSAL_OK - msg queue correctly deleted
 * \return		OSAL_ERROR - error on msg queue delete
 * \callgraph
 * \callergraph
 */
tSInt rif_msg_delete_queue(rif_msg_queue_t **msg_queue_p)
{
	tChar sem_name[MAX_QUEUE_SEM_NAME], event_name[MAX_QUEUE_EVENT_NAME];
	tS16 i;
	tVoid *msg_p;

	/* check parameters */
	if ((msg_queue_p == NULL) || (*msg_queue_p == NULL))
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, TR_CLASS_EXTERNAL, "rif_msg_delete_queue name: %s  rd: %d  wr: %d", 
		(*msg_queue_p)->name, (*msg_queue_p)->readPtr, (*msg_queue_p)->writePtr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	/* delete semaphore */
	if (OSAL_s32NPrintFormat(sem_name, MAX_QUEUE_SEM_NAME, "%s%s", RIF_MSG_QUEUE_PREFIX_SEM, (*msg_queue_p)->name) < 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	if (OSAL_s32SemaphoreClose((*msg_queue_p)->lock) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	if(OSAL_s32SemaphoreDelete((*msg_queue_p)->name) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* delete event */
	if (OSAL_s32NPrintFormat(event_name, MAX_QUEUE_EVENT_NAME, "%s%s", RIF_MSG_QUEUE_PREFIX_EVENT, (*msg_queue_p)->name) < 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	if (OSAL_s32EventClose((*msg_queue_p)->event) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	if(OSAL_s32EventDelete(event_name) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* free msg queue */
	for(i = 0; i < (*msg_queue_p)->queueSize; i++)
	{
		msg_p = ((tPVoid *)((tULong)(*msg_queue_p) + sizeof(rif_msg_queue_t)))[i];
		if (msg_p != NULL)
		{
			OSAL_vMemoryFree(msg_p);
		}
	}
	OSAL_vMemoryFree((*msg_queue_p));
	*msg_queue_p = NULL;

	return OSAL_OK;
}

tSInt rif_msg_send(rif_msg_queue_t *msg_queue_p, tVoid *msg_p)
{
	tVoid **msg_write_p;

	/* check parameters */
	if ((msg_queue_p == NULL) || (msg_p == NULL))
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* get msg queue lock */
	if (OSAL_s32SemaphoreWait(msg_queue_p->lock, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* check msg queue write address */
	msg_write_p = ((tPVoid)((tULong)msg_queue_p + sizeof(rif_msg_queue_t) + (msg_queue_p->writePtr * sizeof(tPVoid))));
	if (*msg_write_p != NULL)
	{
		/* msg queue full */
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif msg queue full name: %s  rd: %d  wd: %d", 
			msg_queue_p->name, msg_queue_p->readPtr, msg_queue_p->writePtr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* put msg in queue */
	*msg_write_p = msg_p;
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, TR_CLASS_EXTERNAL, "rif_msg_send name: %s  msg_write_p: 0x%x  msg_p: 0x%x  rd: %d  wr: %d", 
		msg_queue_p->name, msg_write_p, msg_p, msg_queue_p->readPtr, msg_queue_p->writePtr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	/* increase msg queue write pointer */
	msg_queue_p->writePtr = (msg_queue_p->writePtr + 1) % msg_queue_p->queueSize;
	if (msg_queue_p->writePtr == msg_queue_p->readPtr)
	{
		/* msg queue overflow */
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "rif msg queue overflow name: %s  rd: %d  wr: %d", 
			msg_queue_p->name, msg_queue_p->readPtr, msg_queue_p->writePtr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* free msg queue lock */
	if (OSAL_s32SemaphorePost(msg_queue_p->lock) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

tVoid *rif_msg_receive(rif_msg_queue_t *msg_queue_p)
{
	tVoid **msg_read_p;
	tVoid *ret;

	/* check parameters */
	if (msg_queue_p == NULL)
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	/* get msg queue lock */
	if (OSAL_s32SemaphoreWait(msg_queue_p->lock, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	/* get msg queue read address */
	msg_read_p = ((tPVoid)((tULong)msg_queue_p + sizeof(rif_msg_queue_t) + (msg_queue_p->readPtr * sizeof(tPVoid))));
	ret = *msg_read_p;
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	if (ret != NULL)
	{
		OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, TR_CLASS_EXTERNAL, "rif_msg_receive name: %s  msg_read_p: 0x%x  msg_p: 0x%x  rd: %d  wr: %d", 
			msg_queue_p->name, msg_read_p, ret, msg_queue_p->readPtr, msg_queue_p->writePtr);
	}
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	/* increase msg queue read pointer */
	if (msg_queue_p->writePtr != msg_queue_p->readPtr)
	{
		msg_queue_p->readPtr= (msg_queue_p->readPtr + 1) % msg_queue_p->queueSize;
	}

	/* free msg queue lock */
	if (OSAL_s32SemaphorePost(msg_queue_p->lock) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return NULL;
	}

	return ret;
}

tSInt rif_msg_release(rif_msg_queue_t *msg_queue_p, tVoid *msg_p)
{
	tVoid **msg_read_p;
	tS16 msg_read_index;

	/* check parameters */
	if ((msg_queue_p == NULL) || (msg_p == NULL))
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* get msg queue lock */
	if (OSAL_s32SemaphoreWait(msg_queue_p->lock, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	/* get msg queue read address */
	msg_read_index = msg_queue_p->readPtr;
	do
	{
		/* decrement msg_read_index */
		msg_read_index = (msg_read_index + (msg_queue_p->queueSize - 1)) % msg_queue_p->queueSize;

		/* check if msg queue read pointer match */
		msg_read_p = ((tPVoid)((tULong)msg_queue_p + sizeof(rif_msg_queue_t) + (msg_read_index * sizeof(tPVoid))));
		if (*msg_read_p == msg_p)
		{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
			OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, TR_CLASS_EXTERNAL, "rif_msg_release name: %s  msg_read_p: 0x%x  msg_p: 0x%x  pos: %d  rd: %d  wr: %d", 
				msg_queue_p->name, msg_read_p, msg_p, msg_read_index, msg_queue_p->readPtr, msg_queue_p->writePtr);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

			/* delete msg from queue */
			OSAL_vMemoryFree(msg_p);
			*msg_read_p = NULL;
			break;
		}
	}
	while(msg_read_index != msg_queue_p->writePtr);

	/* free msg queue lock */
	if (OSAL_s32SemaphorePost(msg_queue_p->lock) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

