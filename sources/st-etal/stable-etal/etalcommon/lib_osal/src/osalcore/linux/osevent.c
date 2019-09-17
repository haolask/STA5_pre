//!
//!  \file 		osevent.c
//!  \brief 	<i><b>OSAL Event Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) Event Functions.
//!  \author 	Raffaele Belardi
//!  \author 	(original version) Raffaele Belardi
//!  \version 	1.0
//!  \date 		25.03.2010
//!  \bug 		Unknown
//!  \warning	None
//!
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/

#include "target_config.h"

#include "osal.h"
#include <sys/time.h> /* for gettimeofday */
#include <sys/timeb.h>

/* fine control over COMPONENT TracePrintf */
/* 1 to enable traces for this module */
#define STRACE_CONTROL 0

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#ifndef	OSAL_EVENT_SKIP_NAMES
	#define LINUX_C_EVENT_MAX_NAMELENGHT (OSAL_C_U32_MAX_NAMELENGTH - 1) // ??
#endif
/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

typedef struct EventInfo_t
{
    tBool isThreadInfoUsed;
    OSAL_tThreadID thread_id;
    unsigned int  requestedEvent;
    tBool waitAllEvents;
    pthread_mutex_t mutex;
    pthread_cond_t condvar;
} EventInfo_t;

typedef struct EventElement_t
{
    tBool isUsed;
    tBool pendingDelete;
    unsigned int currentEvent;
    unsigned int prevEvent;
    unsigned int openCounter;
    EventInfo_t EventInfo[LINUX_OSAL_THREAD_MAX];
#ifndef	OSAL_EVENT_SKIP_NAMES
    char   name[OSAL_C_U32_MAX_NAMELENGTH];
#endif
} EventElement_t;

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/
EventElement_t  EventTable[LINUX_OSAL_EVENT_MAX + 1]; /* see linux_EventTableCreate for explanation on +1 */
unsigned int    EventTableEntryCount;
pthread_mutex_t event_table_lock;

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/

/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/
static void linux_EventTableAddEntry(int i, tCString coszName);
static void linux_EventTableDeleteEntry(int i);
static int linux_EventTableGetFreeEntry(void);
static int isValid(OSAL_tEventHandle hEvent);
void Linux_initEvent(void);
char *strace_flag_to_string(OSAL_tEventMask mask);

#ifndef OSAL_EVENT_SKIP_NAMES
static int linux_EventTableSearchEntryByName(tCString coszName);
#endif

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
char *strace_flag_to_string(OSAL_tEventMask mask)
{
	switch (mask)
	{
	case OSAL_EN_EVENTMASK_AND:
		return "AND";
	case OSAL_EN_EVENTMASK_OR:
		return "OR";
	case OSAL_EN_EVENTMASK_REPLACE:
		return "REPLACE";
	case OSAL_EN_EVENTMASK_XOR:
		return "XOR";
	default:
		return "UNSUPP";
	}
}
#endif

/**
* linux_EventInfoSearchThread
*
*/
static EventInfo_t *linux_EventInfoSearchThread(EventElement_t *entry,
                                               OSAL_tThreadID thread_id)
{
    int j;
    
    for (j = 0; j < LINUX_OSAL_THREAD_MAX; j++)
    {
        EventInfo_t *thread_entry = &entry->EventInfo[j];
        
        if ((thread_entry->isThreadInfoUsed == TRUE) && (thread_entry->thread_id == thread_id))
        {
            return thread_entry;
        }
    }
    return NULL;
}

/**
* linux_EventInfoAddThread
*
*/
static EventInfo_t *linux_EventInfoAddThread(EventElement_t *entry,
    unsigned int requestedEvent,
    tBool waitAllEvents)
{
    int j;
    
    for (j = 0; j < LINUX_OSAL_THREAD_MAX; j++)
    {
        EventInfo_t *thread_entry = &entry->EventInfo[j];

        if (thread_entry->isThreadInfoUsed == FALSE)
        {
            thread_entry->isThreadInfoUsed = TRUE;
            thread_entry->thread_id = OSAL_ThreadWhoAmI();
            thread_entry->requestedEvent = requestedEvent;
            thread_entry->waitAllEvents = waitAllEvents;
            pthread_mutex_init(&thread_entry->mutex, NULL);
            pthread_cond_init(&thread_entry->condvar, NULL);
            return thread_entry;
        }
    }
    return NULL;
}

/**
* linux_EventInfoChangeThread
*
*/
static void linux_EventInfoChangeThread(EventInfo_t *thread_entry,
                                        unsigned int requestedEvent,
                                        tBool waitAllEvents)
{
    thread_entry->requestedEvent = requestedEvent;
    thread_entry->waitAllEvents = waitAllEvents;
}

/**
* linux_EventTableAddEntry
*
*/
static void linux_EventTableAddEntry(int i,
                                     tCString coszName)
{
    EventElement_t *entry = &EventTable[i];
    
    entry->isUsed = TRUE;
    entry->pendingDelete = FALSE;
    entry->currentEvent = 0;
    entry->prevEvent = 0;
    entry->openCounter = 1; /* as soon as it is created it is already open */
#ifndef OSAL_EVENT_SKIP_NAMES
    OSAL_szStringNCopy((tString)entry->name, coszName, OSAL_C_U32_MAX_NAMELENGTH);
#endif
    EventTableEntryCount++;
}

/**
* linux_EventTableDeleteEntry
*
*/
static void linux_EventTableDeleteEntry(int i)
{
    int j;
    EventElement_t *entry = &EventTable[i];
    
    entry->isUsed = FALSE;
    for (j = 0; j < LINUX_OSAL_THREAD_MAX; j++)
    {
        EventInfo_t *thread_entry = &entry->EventInfo[j];

        if (thread_entry->isThreadInfoUsed == TRUE)
        {
            thread_entry->isThreadInfoUsed = FALSE;
            pthread_mutex_destroy(&thread_entry->mutex);
            pthread_cond_destroy(&thread_entry->condvar);
        }
    }
    EventTableEntryCount--;
}

/**
* linux_EventTableGetFreeEntry
*
*/
static int linux_EventTableGetFreeEntry(void)
{
    int i;
    
    if (EventTableEntryCount == LINUX_OSAL_EVENT_MAX)
    {
        return -1;
    }
    for (i = 1; i <= LINUX_OSAL_EVENT_MAX; i++)
    {
        if (EventTable[i].isUsed == FALSE)
        {
            return i;
        }
    }
    return -1;
}

/**
* isValid
*
*/
static int isValid(OSAL_tEventHandle hEvent)
{
    if ((hEvent > 0) &&
        (hEvent <= LINUX_OSAL_EVENT_MAX) &&
        (EventTable[hEvent].isUsed == TRUE))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
* linux_EventTableSearchEntryByName
*
*/
#ifndef	OSAL_EVENT_SKIP_NAMES
static int linux_EventTableSearchEntryByName(tCString coszName)
{
    int i;   

    for (i = 1; i <= LINUX_OSAL_EVENT_MAX; i++)
    {
        if (EventTable[i].isUsed == TRUE &&
            OSAL_s32StringCompare(coszName,(tString)EventTable[i].name) == 0)
        {
            return i;
        }
    }
    return -1;
}
#endif // OSAL_EVENT_SKIP_NAMES

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

/**
* Linux_initEvent
*
*/
void Linux_initEvent(void)
{
    int i, j;
    
    EventTableEntryCount = 0;
    /* to compy with Test_EV_LV1_TC15 which assumes hEvent=NULL as invalid, */
    /* the first table entry is initialized but never used; the table has */
    /* LINUX_OSAL_EVENT_MAX+1 entries */
    for (i = 0; i <= LINUX_OSAL_EVENT_MAX; i++)
    {
        EventTable[i].isUsed = FALSE;
        for (j = 0; j < LINUX_OSAL_THREAD_MAX; j++)
        {
            EventTable[i].EventInfo[j].isThreadInfoUsed = FALSE;
        }
    }
    pthread_mutex_init(&event_table_lock, NULL);
}

void Linux_deinitEvent(void)
{
    EventTableEntryCount = 0;
	OSAL_pvMemorySet((tVoid *)EventTable, 0x00, sizeof(EventTable));
    pthread_mutex_destroy(&event_table_lock);
}

/**
* @brief     OSAL_s32EventCreate
*
* @details   Generates and opens an event field of 32 bits.
*
* @param     coszName event name to create (I)
* @param     phEvent pointer to the event handle (->O)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventCreate(tCString coszName,
                         OSAL_tEventHandle* phEvent)
{
    int free_entry;
    errno = 0;
    
    pthread_mutex_lock(&event_table_lock);
	if (errno == EINTR)
	{
		/* According to the pthread_mutex_lock man page:
		 * "These functions shall not return an error code of [EINTR]"
		 * Nevertheless on A2 it happens, expecialy during the etalTestReceiverAlive
		 * test and the OSAL_s32EventCreate exits with an error.
		 * Since it seems that the error is just a false error, reset errno and continue.
		 */
		errno = 0;
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_SYSTEM_MIN)
		OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_SYSTEM_MIN, TR_CLASS_OSALCORE, "EventCreate got EINTR, ignoring it");
#endif
	}
#ifndef OSAL_EVENT_SKIP_NAMES
    if (coszName)
    {
        if (OSAL_u32StringLength(coszName) <= LINUX_C_EVENT_MAX_NAMELENGHT)
        {
            int current_entry;
            current_entry = linux_EventTableSearchEntryByName(coszName);
            if (current_entry != -1)
            {
                errno = EEXIST;
            }
        }
        else
        {
            errno = ENAMETOOLONG;
        }
    }
    else
    {
        errno = EINVAL;
    }
    
    if (errno)
    {
        goto event_create_error;
    }
#endif
    if (phEvent)
    {
        free_entry = linux_EventTableGetFreeEntry();
        
        if (free_entry >= 0)
        {
#ifdef OSAL_EVENT_SKIP_NAMES
            linux_EventTableAddEntry(free_entry, NULL);
#else
            linux_EventTableAddEntry(free_entry, coszName);
#endif
            *phEvent = (OSAL_tEventHandle)free_entry;
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
	if (STRACE_CONTROL) OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "EventCreate (%s, 0x%x)", coszName, *phEvent);
#endif
        }
        else
        {
            errno = ENOMEM;
        }
    }
    else
    {
        errno = EINVAL;
    }
    
event_create_error:
        
    pthread_mutex_unlock(&event_table_lock);
    if (errno == 0)
    {
        return OSAL_OK;
    }
    
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventCreate (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
* @brief     OSAL_s32EventDelete
*
* @details   Deletes the event, or scheules the event for deletion if it is still open.
*
* @param     coszName event name to be removed (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
#ifndef OSAL_EVENT_SKIP_NAMES
tS32 OSAL_s32EventDelete(tCString coszName)
{
    EventElement_t *entry; 
    int entry_id;
    errno = 0;

    if (coszName)
    {
        pthread_mutex_lock(&event_table_lock);
        entry_id = linux_EventTableSearchEntryByName(coszName);
        
        if (entry_id >= 0)
        {
            entry = &EventTable[entry_id];
            if (entry->openCounter == 0)
            {
                linux_EventTableDeleteEntry(entry_id);
            }
            else
            {
                entry->pendingDelete = TRUE;
                errno = EBUSY; /* per spec should be OSAL_OK condition! */
            }
        }
        else
        {
            errno = ENOENT;
        }
        pthread_mutex_unlock(&event_table_lock);
    }
    else
    {
        errno = EINVAL;
    }

    if (errno == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventDelete (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}
#endif //#ifndef OSAL_EVENT_SKIP_NAMES

/**
* @brief     OSAL_s32EventOpen
*
* @details   This function returns a valid handle to an OSAL event
*            already created.
*
* @param     coszName event name to be removed (I)
* @param     phEvent pointer to the event handle (->O)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
#ifndef OSAL_EVENT_SKIP_NAMES
tS32 OSAL_s32EventOpen(tCString coszName,
                       OSAL_tEventHandle* phEvent)
{
    int entry_id;
    errno = 0;
    
    if (coszName && phEvent)
    {
        pthread_mutex_lock(&event_table_lock);
        entry_id = linux_EventTableSearchEntryByName(coszName);
        if (entry_id >= 0)
        {
            EventTable[entry_id].openCounter++;
            *phEvent = (OSAL_tEventHandle)entry_id;
        }
        else
        {
            errno = ENOENT;
        }        
        pthread_mutex_unlock(&event_table_lock);
    }
    else
    {
        errno = EINVAL;
    }

    if (errno == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventOpen (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}
#endif //#ifndef OSAL_EVENT_SKIP_NAMES

/**
* @brief     OSAL_s32EventClose
*
* @details   This function closes an OSAL event.
*
* @param     hEvent event handle (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventClose(OSAL_tEventHandle hEvent)
{
    EventElement_t *entry;
    errno = 0;
    
    pthread_mutex_lock(&event_table_lock);
    if (isValid(hEvent))
    {
        entry = &EventTable[hEvent];
        if (entry->openCounter > 0)
        {
            entry->openCounter--;
            if ((entry->openCounter == 0) &&
                (entry->pendingDelete == TRUE))
            {
                linux_EventTableDeleteEntry(hEvent);
            }
        }
        else
        {
            errno = EPERM;
        }
    }
    else
    {
        if (hEvent == 0)
        {
            errno = EINVAL;
        }
        else
        {
            errno = ENOENT; /* should be EINVAL, changed to ENOENT to comply with Test_EV_LV1_TC16 */
        }
    }
    pthread_mutex_unlock(&event_table_lock);
    
    if (errno == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventClose (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
* @brief     OSAL_s32EventFree
*
* @details   This function closes an OSAL event.
*
* @param     hEvent event handle (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
#ifdef OSAL_EVENT_SKIP_NAMES
tS32 OSAL_s32EventFree(OSAL_tEventHandle hEvent)
{
    errno = 0;
    
    pthread_mutex_lock(&event_table_lock);
    
    if (hEvent == (OSAL_tEventHandle)NULL)
    {
        errno = EINVAL;
    }
    else if (isValid(hEvent))
    {
        /* the OS21 behavior is to delete the event regardless of the open counter */
        linux_EventTableDeleteEntry(hEvent);
    }
    else
    {
        errno = ENOENT;
    }
    pthread_mutex_unlock(&event_table_lock);
    
    if (errno == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventFree (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}
#endif // OSAL_EVENT_SKIP_NAMES

/**
* @brief     OSAL_s32EventWait
*
* @details   This function waits for an OSAL event to occur,
*            where an event occurrence means the link operation
*            (given by enFlags) between the EventField present
*            in the EventGroup structure and the provided EventMask
*            is catched. Allowed link operation are AND/OR
*            So the event resets the calling thread only if within
*            the requested timeout one of the following conditions
*            is verified:
*               EventMask || EventField is TRUE or
*               EventMask && EventField is true
*            depending on the requested link operation.
*
* @param     hEvent event handle (I)
* @param     mask event mask (I)
* @param     enFlags event flag (I)
* @param     msec waiting time (I)
* @param     pResultMask pointer to the previous event mask (->O)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventWait(OSAL_tEventHandle      hEvent,
                       OSAL_tEventMask        mask,
                       OSAL_tenEventMaskFlag  enFlags,
                       OSAL_tMSecond          msec,
                       OSAL_tEventMask       *pResultMask)
{
    EventElement_t *entry;
    EventInfo_t *event_thread_info;
    tU32 current_event;
    
    errno = 0;
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    if (STRACE_CONTROL) OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "EventWait (0x%x) 0x%.8x %s", hEvent, mask, strace_flag_to_string(enFlags));
#endif
    pthread_mutex_lock(&event_table_lock);

    if (isValid(hEvent) &&
        (enFlags == OSAL_EN_EVENTMASK_OR || enFlags == OSAL_EN_EVENTMASK_AND))
    {
        tBool wait_all = (enFlags == OSAL_EN_EVENTMASK_AND) ? TRUE : FALSE;

        entry = &EventTable[hEvent];
        event_thread_info = linux_EventInfoSearchThread(entry, OSAL_ThreadWhoAmI());
        
        if (event_thread_info == NULL)
        {
            /* The calling thread never waited on this event, create new entry */
            event_thread_info = linux_EventInfoAddThread(entry, mask, wait_all);
            if (event_thread_info == NULL)
            {
                errno = ENOMEM;
            }
        }
        else
        {
            linux_EventInfoChangeThread(event_thread_info, mask, wait_all);
        }
        
        current_event = entry->currentEvent;
        pthread_mutex_unlock(&event_table_lock);

        if (errno != 0)
        {
            goto eventWaitFinish;
        }
        
         /* check if the wait can be already satisfied with existing events */
        if (((wait_all == TRUE) &&
            ((mask & current_event) == mask)) ||
            ((wait_all == FALSE) &&
            (mask & current_event) != 0))
        {
            if (pResultMask != NULL)
            {
                *pResultMask = current_event;
            }
            goto eventWaitFinish;
        }

        switch ((tU32)msec)
        {
            case OSAL_C_TIMEOUT_FOREVER:
            {
                errno = pthread_mutex_lock(&event_thread_info->mutex);
		ASSERT_ON_DEBUGGING(errno == 0);
                errno = pthread_cond_wait(&event_thread_info->condvar, &event_thread_info->mutex);
		ASSERT_ON_DEBUGGING(errno == 0);
                pthread_mutex_unlock(&event_thread_info->mutex);
            }
            break;
            case OSAL_C_TIMEOUT_NOBLOCKING:
            {
                // nothing to do, this is equivalent to polling the event
            }
            break;
            default:
            {
						
				struct timespec ts;
				unsigned long int vl_nanoSec = 0;
				struct timeval currtime_tv;
#define OSAL_MSEC_TO_NANO_SEC	(1000*1000)
#define OSAL_NANO_SEC_PAR_SECOND (1000*1000*1000)
#define OSAL_NANO_SEC_PAR_MICRO_SEC (1000)
#define OSAL_MSEC_SEC_PAR_SECOND (1000)
				
				gettimeofday(&currtime_tv, NULL);

				// set ts
				ts.tv_sec = currtime_tv.tv_sec;
				ts.tv_nsec = currtime_tv.tv_usec * OSAL_NANO_SEC_PAR_MICRO_SEC;

				// printf("BEFORE msec = %d, ts.tv_sec = %d, ts.tv_nsec = %d\n", msec,  (int) ts.tv_sec, (int) ts.tv_nsec);
				

				// increase by msec
				// msec is
				//	(msec / OSAL_MSEC_SEC_PAR_SECOND) second
				// + ((msec % OSAL_MSEC_SEC_PAR_SECOND) * OSAL_MSEC_TO_NANO_SEC) nano second

				// add the second
				ts.tv_sec += (msec / OSAL_MSEC_SEC_PAR_SECOND);

				// manage the nano second addition
				// calculate the next nano second reference
				vl_nanoSec = ts.tv_nsec + ((msec % OSAL_MSEC_SEC_PAR_SECOND) * OSAL_MSEC_TO_NANO_SEC);

				// set now the proper sec and nano sec
				//
				ts.tv_sec += (vl_nanoSec / OSAL_NANO_SEC_PAR_SECOND);

				// change
				ts.tv_nsec = (vl_nanoSec % OSAL_NANO_SEC_PAR_SECOND); 
				
				// printf("AFTER msec = %d,  ts.tv_sec = %d, ts.tv_nsec = %d\n", msec, (int) ts.tv_sec, (int) ts.tv_nsec);
									        

                errno = pthread_mutex_lock(&event_thread_info->mutex);
				ASSERT_ON_DEBUGGING(errno == 0);
                errno = pthread_cond_timedwait(&event_thread_info->condvar, &event_thread_info->mutex, &ts);
				if (!((errno == 0) || (errno == ETIMEDOUT)))
					{
					perror("Event Wait pthread_cond_timedwait errno : ");
					printf("msec = %d, vl_nanoSec = %d, ts.tv_sec = %d, ts.tv_nsec = %d\n", msec, (int) vl_nanoSec, (int) ts.tv_sec, (int) ts.tv_nsec);
                	ASSERT_ON_DEBUGGING((errno == 0) || (errno == ETIMEDOUT));
					}
                pthread_mutex_unlock(&event_thread_info->mutex);
            }
            break;
        }
        if (pResultMask != NULL)
        {
            pthread_mutex_lock(&event_table_lock);
            current_event = entry->currentEvent;
            *pResultMask = current_event;
            pthread_mutex_unlock(&event_table_lock);
        }
    }
    else
    {
        pthread_mutex_unlock(&event_table_lock);
        if (hEvent != 0)
        {
            errno = ENOENT; /* To comply with Test_EV_LV1_TC25 */
        }
        else
        {
            errno = EINVAL;
        }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
		OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventWait invalid event, hevent = %d, errno = %d", hEvent, errno);
#endif

    }

eventWaitFinish:
    
//    if ((errno == 0) || (errno == ETIMEDOUT))
	if (errno == 0)
    {
		errno = 0; /* comply with EV_LV1_TC27 which does not expect error for timeout condition */
        return OSAL_OK;
    }
	else if (errno == ETIMEDOUT)
	{
		return OSAL_ERROR_TIMEOUT_EXPIRED;
	}

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventWait errno = %d, (%s)", errno, OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}
/**
* @brief     OSAL_s32EventPost()
*
* @details   This function Posts an OSAL event using the OS21 ones.
*
* @param     hEvent event handle (I)
* @param     mask event mask (I)
* @param     enFlags event flag (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventPost(OSAL_tEventHandle hEvent,
                       OSAL_tEventMask mask,
                       OSAL_tenEventMaskFlag enFlags)
{
    EventElement_t *entry; 
    tU32 new_event;
    
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    if (STRACE_CONTROL) OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "EventPost (0x%x) 0x%.8x %s", hEvent, mask, strace_flag_to_string(enFlags));
#endif
    errno = 0;
    pthread_mutex_lock(&event_table_lock);
    if (isValid(hEvent))
    {
        entry = &EventTable[hEvent];
        
        switch (enFlags)
        {
            case OSAL_EN_EVENTMASK_AND:
                entry->currentEvent &= mask;  /* OS21 implementation performs a sem_clear(~mask) here! */
                new_event = 0;
            break;
            case OSAL_EN_EVENTMASK_OR:
                entry->currentEvent |= mask;
                new_event = mask;
            break;
            case OSAL_EN_EVENTMASK_REPLACE:
                entry->currentEvent = mask;
                new_event = mask;
            break;
            case OSAL_EN_EVENTMASK_XOR:
                entry->currentEvent ^= mask;
                new_event = entry->currentEvent;
            break;
            default:
                errno = ENOSYS;
                new_event = 0; /* lint */
            break;
        }
        if (errno == 0)
        {
            int j;
            
            for (j = 0; j < LINUX_OSAL_THREAD_MAX; j++)
            {
                EventInfo_t *thread_entry = &entry->EventInfo[j];
                
                if (thread_entry->isThreadInfoUsed == TRUE)
                {
                    int wake = 0;
                    
		//printf("===========>waitAll %d, newEvent 0x%x, currentEvent 0x%x, reqEvent 0x%x)\n", thread_entry->waitAllEvents, new_event, entry->currentEvent, thread_entry->requestedEvent);
                    if (thread_entry->waitAllEvents &&
                        ((new_event & thread_entry->requestedEvent) != 0) && /* the presence of at least one new requested event triggers the comparison */
                        ((entry->currentEvent & thread_entry->requestedEvent) == thread_entry->requestedEvent))
                    {
                        wake = 1;
                    }
                    else if (!thread_entry->waitAllEvents &&
                            (new_event & thread_entry->requestedEvent) != 0)
                    {
                        wake = 1;
                    }
                    if (wake == 1)
                    {
                        pthread_mutex_unlock(&event_table_lock);
                        pthread_cond_broadcast(&thread_entry->condvar);
                        pthread_mutex_lock(&event_table_lock);
			sched_yield();
		//printf("++++++++++>cond broadcast (0x%x)\n", entry->currentEvent);
                    }
                }
            }
            pthread_mutex_unlock(&event_table_lock);
        }
        else
        {
            pthread_mutex_unlock(&event_table_lock);
        }
    }
    else
    {
        pthread_mutex_unlock(&event_table_lock);
        if (hEvent == 0)
        {
            errno = EINVAL;
        }
        else
        {
            errno = ENOENT; /* To comply with Test_EV_LV1_TC22 */
        }
    }

    if (errno == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventPost errno = %d, (%s)", errno, OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
* @brief     OSAL_s32EventStatus
*
* @details   This function creates an OSAL event using the OS21 ones.
*
* @param     hEvent event handle (I)
* @param     mask event mask (I)
* @param     pMask Pointer to the Return status value (I)
*
* @return
*           - OSAL_OK if everything goes right
*           - OSAL_ERROR otherwise
*
*/
tS32 OSAL_s32EventStatus(OSAL_tEventHandle hEvent,
                         OSAL_tEventMask mask,
                         OSAL_tEventMask* pMask)
{
    EventElement_t *entry; 
    tU32 current_event;

    errno = 0;
    pthread_mutex_lock(&event_table_lock);
    if (isValid(hEvent))
    {
        entry = &EventTable[hEvent];
        current_event = entry->currentEvent;
        *pMask = (mask & current_event);
    }
    else
    {
        if (hEvent == 0)
        {
            errno = EINVAL;
        }
        else
        {
            errno = ENOENT; /* To comply with Test_EV_LV1_TC19 */
        }
    }
    pthread_mutex_unlock(&event_table_lock);
    
    if (errno == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "EventStatus (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

#ifdef __cplusplus
}
#endif
/** @} */

/* End of File */
