//!
//!  \file 		ossmphr.c
//!  \brief 	<i><b>OSAL semaphores Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) semaphores Functions.
//!  \author 	Raffaele Belardi
//!  \author 	(original version) Raffaele Belardi
//!  \version 	1.0
//!  \date 		13.1.2010
//!  \bug 		Unknown
//!  \warning	None
//!
#ifdef __cplusplus
extern "C" {
#endif

/* fine control over COMPONENT TracePrintf */
/* 1 to enable traces for this module */
/* this does not follow the common TracePrintf format due to infinite recursion (TracePrints calls CreateSemaphore) */
#define STRACE_CONTROL 0

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/
#include "target_config.h"

#include "osal.h"
#include <sys/timeb.h>
#include <sys/time.h> /* for gettimeofday */


/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#define LINUX_C_SEMAPHORE_MAX_NAMELENGTH   (OSAL_C_U32_MAX_NAMELENGTH - 1)
#define LINUX_OSAL_MAX_SEM                 100

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

typedef struct
{
	int in_use;
	int open;
	char name[LINUX_C_SEMAPHORE_MAX_NAMELENGTH + 1];
	sem_t sem;
} tOSAL_Sem;

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/
static pthread_mutex_t sem_info_mutex;
static tOSAL_Sem sem_info[LINUX_OSAL_MAX_SEM];

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/
/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/
static int allocate_sem(void);
static int search_sem(tCString coszName);
static tVoid free_sem(int i);

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/
static int allocate_sem(void)
{
	int i;

    pthread_mutex_lock(&sem_info_mutex);
	for (i = 1; i < LINUX_OSAL_MAX_SEM; i++)
	{
		if (sem_info[i].in_use == 0)
		{
			sem_info[i].in_use = 1;
    		pthread_mutex_unlock(&sem_info_mutex);
			return i;
		}
	}
    pthread_mutex_unlock(&sem_info_mutex);
	return 0;
}

static int search_sem(tCString coszName)
{
	tOSAL_Sem *sm;
	int i;

    pthread_mutex_lock(&sem_info_mutex);
	for (i = 1; i < LINUX_OSAL_MAX_SEM; i++)
	{
		sm = &sem_info[i];
		if ((sm->in_use == 1) && (strcmp(sm->name, coszName) == 0))
		{
    		pthread_mutex_unlock(&sem_info_mutex);
			return i;
		}
	}
    pthread_mutex_unlock(&sem_info_mutex);
	return 0;
}

static tVoid free_sem(int i)
{
	if ((i > 0) && (i < LINUX_OSAL_MAX_SEM))
	{
    	pthread_mutex_lock(&sem_info_mutex);
		sem_info[i].in_use = 0;
    	pthread_mutex_unlock(&sem_info_mutex);
	}
}

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

tSInt Linux_initSemaphores(void)
{
	if (pthread_mutex_init(&sem_info_mutex, NULL) != 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	memset(&sem_info[0], 0x00, (size_t)sizeof(sem_info));
	return OSAL_OK;
}

tSInt Linux_deinitSemaphores(void)
{
	memset(&sem_info[0], 0x00, (size_t)sizeof(sem_info));
	if (pthread_mutex_destroy(&sem_info_mutex) != 0)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}


/**
 * @brief     OSAL_s32SemaphoreCreate
 *
 *
 * @details   This function create a OSAL Semaphore.
 *
 * @param     coszName name of semaphore (I)
 * @param     phSemaphore pseudo-handle for semaphore (O)
 * @param     uCount payload of semaphore (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreCreate(tCString coszName,
                              OSAL_tSemHandle * phSemaphore,
                              tU32 uCount)
{
	tOSAL_Sem *sm = NULL;
	int i;
    
    if ((coszName != NULL) && (phSemaphore != NULL))
    {
        if ((tU32)strlen(coszName) < OSAL_C_U32_MAX_NAMELENGTH )
        {
			if ((i = search_sem(coszName)) > 0)
			{
				errno = EEXIST;
			}
			else if ((i = allocate_sem()) != 0)
			{
				sm = &sem_info[i];
				if (sem_init(&sm->sem, 0, uCount) == 0)
				{
					strncpy(sm->name, coszName, (size_t)(OSAL_C_U32_MAX_NAMELENGTH - 1));
					sm->name[OSAL_C_U32_MAX_NAMELENGTH - 1] = '\0'; // in case coszName is not null-terminated
					sm->open = 1;
					*phSemaphore = i;
					return OSAL_OK;
				}
				else
				{
					free_sem(i);
				}
			}
			else
			{
				errno = ENOMEM;
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
    
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreCreate (%s): %s", coszName, OSAL_coszErrorText((tS32)errno2osal()));
#endif
#ifndef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
	ASSERT_ON_DEBUGGING(0);
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
 *
 * @brief   OSAL_s32SemaphoreDelete
 *
 * @details This function removes an OSAL Semaphore.
 *
 * @param   coszName semaphore name to be removed (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreDelete(tCString coszName)
{
	int i;

	errno = 0;
    if (coszName)
    {
		i = search_sem(coszName);
		if (i > 0)
		{
			if (sem_info[i].open == 1)
			{
				errno = EBUSY;
			}
	    	else if (sem_destroy(&sem_info[i].sem) == 0)
			{
				free_sem(i);
            	return OSAL_OK;
			}
        }
		else
		{
			errno = ENOENT;
		}
    }
    else
    {
        errno = EINVAL;
    }
    
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreDelete %s", OSAL_coszErrorText((tS32)errno2osal()));
#endif
#ifndef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
   ASSERT_ON_DEBUGGING(0);
#endif
   OSAL_s32CallErrorHook(errno2osal());
   return OSAL_ERROR;
}
/**
 *
 * @brief   OSAL_s32SemaphoreOpen
 *
 * @details This function returns a valid handle to an OSAL Semaphore
 *              already created.
 *
 * @param   coszName     semaphore name to be opened (I)
 * @param   phSemaphore  pointer to the semaphore handle (->O)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreOpen(tCString coszName,
                            OSAL_tSemHandle *phSemaphore)
{
	int i;
    
	errno = 0;
    if ((coszName != NULL) && (phSemaphore != NULL))
    {
		if (strlen(coszName) >= LINUX_C_SEMAPHORE_MAX_NAMELENGTH)
		{
			errno = ENOENT;
		}
		else
		{
	        i = search_sem(coszName);
	        if (i > 0)
	        {
				sem_info[i].open = 1;
	            *phSemaphore = i;
	            return OSAL_OK;
	        }
			else
			{
				errno = ENOENT;
			}
		}
    }
    else
    {
        errno = EINVAL;
    }
    
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
   OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreOpen %s", OSAL_coszErrorText((tS32)errno2osal()));
#endif	
#ifndef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
   ASSERT_ON_DEBUGGING(0);
#endif
   OSAL_s32CallErrorHook(errno2osal());
   return OSAL_ERROR;
}


/**
 *
 * @brief   OSAL_s32SemaphoreClose
 *
 * @details This function closes an OSAL Semaphore.
 *
 * @param   hSemaphore semaphore handle (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreClose(OSAL_tSemHandle hSemaphore)
{
	errno = 0;

	if ((hSemaphore > 0) && (hSemaphore < LINUX_OSAL_MAX_SEM))
	{
		sem_info[hSemaphore].open = 0;
	    return OSAL_OK;
	}
	else
	{
		errno = EINVAL;
	}

    if (errno == EINVAL)
    {
        /* translation required to support test Test_SM_LV1_TC11 */
        errno = ENOENT;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreClose %s", OSAL_coszErrorText((tS32)errno2osal()));
#endif
#ifndef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
    ASSERT_ON_DEBUGGING(0);
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
 *
 * @brief   OSAL_s32SemaphoreGetValue
 *
 * @details This function retrieve the actual countvalue.
 *
 * @param   hSemaphore semaphore handle (I)
 * @param   ps32Value semaphore value (->O)
 *
 * Semaphore value is expected also to reflect the number of waiting task :
 * negative value represents the number of waiting tasks. 0 means no waiting tasks. 
 * 
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreGetValue(OSAL_tSemHandle hSemaphore,
                               tPS32 ps32Value)
{
	errno = 0;
    if (hSemaphore == 0)
    {
        errno = ENOENT;
    }
	else if (hSemaphore >= LINUX_OSAL_MAX_SEM)
	{
		errno = EINVAL;
	}
    else if (sem_getvalue(&sem_info[hSemaphore].sem, ps32Value) == 0)
    {
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreGetValue %s", OSAL_coszErrorText((tS32)errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}


/**
 *
 * @brief    OSAL_s32SemaphorePost
 *
 * @details  This function release a semaphore
 *                The operation checks the queue of tasks waiting for the semaphore,
 * 				  if the list is not empty, then the first task on the list is restarted,
 * 				  possibly preempting the current task.
 * 				  Otherwise the semaphore count is incremented, and the task continues running.
 *
 * @param   hSemaphore semaphore handle (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphorePost(OSAL_tSemHandle hSemaphore)
{
	errno = 0;
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    if (STRACE_CONTROL) OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "SemPost (0x%x)\n", hSemaphore);
#endif
    if (hSemaphore == 0)
    {
        errno = ENOENT;
    }
	else if (hSemaphore >= LINUX_OSAL_MAX_SEM)
	{
		errno = EINVAL;
	}
    else if (sem_post(&sem_info[hSemaphore].sem) == 0)
    {
		// TBC : do we really need to sched_yield ??
		//
		// sched_yield();
        return OSAL_OK;
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphorePost %s", OSAL_coszErrorText((tS32)errno2osal()));
#endif
#ifndef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
    ASSERT_ON_DEBUGGING(0);
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
 *
 * @brief   OSAL_s32SemaphoreWait
 *
 * @details This function performs a wait operation on the specified semaphore.
 *                The operation checks the semaphore counter, and if it is 0,
 *                adds the current task to the list of queued tasks, before descheduling.
 *                Otherwise the semaphore counter is decremented, and the task continues running.
 *
 * @param   hSemaphore semaphore handle (I)
 * @param   msec max delaytime before timeout (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *	    - OSAL_ERROR_TIMEOUT_EXPIRED if the semaphore has not been called and timeout expired.
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32SemaphoreWait(OSAL_tSemHandle hSemaphore,
                           OSAL_tMSecond msec)
{
    int ret;
	struct timespec ts;
	unsigned long int vl_nanoSec = 0;
	struct timeval currtime_tv;
#define OSAL_MSEC_TO_NANO_SEC	(1000*1000)
#define OSAL_NANO_SEC_PAR_SECOND (1000*1000*1000)
#define OSAL_NANO_SEC_PAR_MICRO_SEC (1000)
#define OSAL_MSEC_SEC_PAR_SECOND (1000)

	errno = 0;
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    if (STRACE_CONTROL) OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "SemWait (0x%x)\n", hSemaphore);
#endif
    if ((hSemaphore == 0) || (hSemaphore >= LINUX_OSAL_MAX_SEM))
    {
        errno = EINVAL;
    }
    else
    {
        switch ((tU32)msec)
        {
            case OSAL_C_TIMEOUT_FOREVER:
                ret = sem_wait(&sem_info[hSemaphore].sem);
            break;
            case OSAL_C_TIMEOUT_NOBLOCKING:
                ret = sem_trywait(&sem_info[hSemaphore].sem);
		if (errno ==  EAGAIN)
		{
			errno = ETIMEDOUT;
		}
            break;
            default:
				gettimeofday(&currtime_tv, NULL);

				// set ts
				ts.tv_sec = currtime_tv.tv_sec;
				ts.tv_nsec = currtime_tv.tv_usec * OSAL_NANO_SEC_PAR_MICRO_SEC;

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

			ret = sem_timedwait(&sem_info[hSemaphore].sem, &ts);
            break;
        }
        if (ret == 0)
        {
            return OSAL_OK;
        }
    }
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
	if (errno != ETIMEDOUT)
	{
    		OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "SemaphoreWait %s", OSAL_coszErrorText((tS32)errno2osal()));
#ifndef CONFIG_APP_OSALCORE_SEMAPHORE_TEST
           ASSERT_ON_DEBUGGING(0);
#endif
           OSAL_s32CallErrorHook(errno2osal());
	}
#endif

	// add a timeout error to inform the caller
	if (errno == ETIMEDOUT)
	{
		return OSAL_ERROR_TIMEOUT_EXPIRED;
	}
	
   return OSAL_ERROR;
}

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
