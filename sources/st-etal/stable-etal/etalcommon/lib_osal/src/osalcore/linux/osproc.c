//!
//!  \file 		osproc.c
//!  \brief 	<i><b>OSAL Thread Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) Thread Functions.
//!
#ifdef __cplusplus
extern "C" {
#endif

/* fine control over COMPONENT TracePrintf */
/* 1 to enable traces for this module */
#define STRACE_CONTROL 0

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/
#include "target_config.h"

#include "osal.h"

#ifdef CONFIG_HOST_OS_LINUX
#include <limits.h>
#include <unistd.h>
#endif

#ifdef CONFIG_HOST_OS_WIN32
#include <windows.h>
#ifndef PTHREAD_STACK_MIN
#include "win32\local_lim.h"
#endif
#endif

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#define LINUX_C_THREAD_MAX_NAMELENGTH (OSAL_C_U32_MAX_NAMELENGTH - 1)
#define LINUX_OSAL_THREAD_MAX  40

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/
struct thread_info_t
{
    tBool in_use;
    pthread_t tid;
    char name[LINUX_C_THREAD_MAX_NAMELENGTH];
    
    OSAL_tpfThreadEntry entry_point;
    void * entry_arg;
};

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/
static pthread_mutex_t thread_info_mutex;
static struct thread_info_t thread_info[LINUX_OSAL_THREAD_MAX]; 

/************************************************************************
|function prototype (scope: module-local)
-----------------------------------------------------------------------*/
#if 0	/* not used */
static tS32 Linux_to_OSAL_ThreadPriority(tSInt linux_prio, tSInt policy, tU32 *osal_prio);
#endif /* #if 0  not used */
static tS32 OSAL_to_Linux_ThreadPriority(tU32 osal_prio, tSInt policy, tSInt *linux_prio);

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/

/*
*  Linux_add_thread_info_entry
*
*/
static int Linux_add_thread_info_entry(OSAL_tpfThreadEntry entry_point,
                                                         void *entry_arg,
                                                         char *name)
{
    int i;
    struct thread_info_t *ti;
    
    pthread_mutex_lock(&thread_info_mutex);
    for (i = 0; i < LINUX_OSAL_THREAD_MAX; i++)
    {
        ti = &thread_info[i];
        if (ti->in_use == FALSE)
        {
            ti->in_use = TRUE;
            strncpy(ti->name, name, LINUX_C_THREAD_MAX_NAMELENGTH - 1);
			ti->name[LINUX_C_THREAD_MAX_NAMELENGTH - 1] = '\0'; // in case 'name' is not null-terminated
            /* tid will be available after pthread_create */
            memset(&(ti->tid), 0x00, (size_t)sizeof(pthread_t));
            ti->entry_point = entry_point;
            ti->entry_arg = entry_arg;
            
            pthread_mutex_unlock(&thread_info_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&thread_info_mutex);
    ASSERT_ON_DEBUGGING(0);
    return  -1;
}

static int Linux_search_thread_info_entry(pthread_t tid)
{
    int i;

    pthread_mutex_lock(&thread_info_mutex);
    for (i = 0; i < LINUX_OSAL_THREAD_MAX; i++)
    {
        if ((thread_info[i].in_use == TRUE) && (pthread_equal(thread_info[i].tid, tid)))
        {
            pthread_mutex_unlock(&thread_info_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&thread_info_mutex);
    //ASSERT_ON_DEBUGGING(0);
    return -1;
}

/*
*  Linux_search_thread_info_entry_by_name
*
*/
static struct thread_info_t *Linux_search_thread_info_entry_by_name(char *name)
{
    int i;

    if (name == NULL)
    {
	/* Test_PR_LV1_TC00 causes a segmentation fault on the strcmp below */
	/* in some particular situation not well understood */
        return NULL;
    }

    pthread_mutex_lock(&thread_info_mutex);
    for (i = 0; i < LINUX_OSAL_THREAD_MAX; i++)
    {
        if ((thread_info[i].in_use == TRUE) && (strcmp(thread_info[i].name, name) == 0))
        {
            pthread_mutex_unlock(&thread_info_mutex);
            return &thread_info[i];
        }
    }
    pthread_mutex_unlock(&thread_info_mutex);
    return NULL;
}

/*
*  Linux_thread_wrapper
*
*/
static void *Linux_thread_wrapper(void *arg)
{
    struct thread_info_t *ti = (struct thread_info_t *)arg;
	int old;

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    //OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "Activated after creation: %s", ti->name);
#endif

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old);
    ti->entry_point(ti->entry_arg);

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "Entry point termination: %s", ti->name);
#endif

	/* in OSAL_OS21 threads that terminate are not actually removed from the system */
    /* until explicitely cancelled. The loop below mimiks that behavior */
	while (TRUE)
	{
			OSAL_s32ThreadWait(500);
	}
	return NULL; // never reached
}

/*
*  internal_thread_spawn
*
*/
static OSAL_tThreadID internal_thread_spawn(const OSAL_trThreadAttribute* pcorAttr)
{	
    if ((pcorAttr->szName != NULL) && (pcorAttr->u32Priority <= OSAL_C_U32_THREAD_PRIORITY_LOWEST) &&
       (pcorAttr->pfEntry != NULL))
    {
        if(strlen((char*)pcorAttr->szName)<= LINUX_C_THREAD_MAX_NAMELENGTH)
        {
            pthread_t thread;
            pthread_attr_t attr;
            int stack_size, policy;
            int prio;
            struct sched_param sp;
			int i;

            errno = pthread_attr_init(&attr);
            if (errno < 0)
            {
				goto thread_spawn_error_noinit;
			}
			if (pcorAttr->s32StackSize < PTHREAD_STACK_MIN)
			{
				stack_size = PTHREAD_STACK_MIN;
			}
			else
			{
				stack_size = pcorAttr->s32StackSize;
			}
			errno = pthread_attr_setstacksize(&attr, stack_size);
            if (errno < 0)
            {
				goto thread_spawn_error;
			}
			if (Linux_search_thread_info_entry_by_name((char*)pcorAttr->szName) != NULL)
			{
					errno = EEXIST;
					goto thread_spawn_error;
			}
			i = Linux_add_thread_info_entry(pcorAttr->pfEntry, pcorAttr->pvArg, pcorAttr->szName);
			if (i < 0)
			{
				goto thread_spawn_error;
			}
			pthread_mutex_lock(&thread_info_mutex);
			/* if the new thread is scheduled after this pthread_create, it blocks immediately in the wrapper */
			/* to acquire thread_info_mutex so we are sure the thread_info is complete (except for system_tid) */
			/* before the sched_yield below */
			errno = pthread_create(&thread, &attr, Linux_thread_wrapper, &thread_info[i]);
			if (errno < 0)
			{
				pthread_mutex_unlock(&thread_info_mutex);
				goto thread_spawn_error;
			}
			memcpy(&(thread_info[i].tid), &thread, (size_t)sizeof(pthread_t));
			pthread_mutex_unlock(&thread_info_mutex);

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
			OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "ThreadCreate (%s, %d)", pcorAttr->szName, i);
#endif

            policy = SCHED_RR;
            if (OSAL_to_Linux_ThreadPriority(pcorAttr->u32Priority, policy, &prio) == OSAL_OK)
            {
                sp.sched_priority = prio;
                /* "man sched_setscheduler" for overview on sheduler policy */
                /* NOTE: setting SCHED_RR requires root privileges */
                /* not working for MinGW (errno == ENOTSUP, not root?) */
                /* not working for native ubuntu (errno == EPERM) */
                errno = pthread_setschedparam(thread, policy, &sp);
#if defined(CONFIG_HOST_OS_WIN32) || (defined(CONFIG_HOST_OS_LINUX) && defined(CONFIG_COMPILER_GCC_NATIVE))
                if ((errno == EPERM) || (errno == EINVAL) || (errno == ENOTSUP))
                {
                    /* on MinGW the pthread_setschedparam return error ENOTSUP and */
                    /* not working for native ubuntu (errno == EPERM) */
                    /* then do not change scheduler policy and change only priority */
                    /* get current Linux thread scheduling policy */
                    errno = pthread_getschedparam(thread, &policy, &sp);
                    if (errno == 0)
                    {
                        if (OSAL_to_Linux_ThreadPriority(pcorAttr->u32Priority, policy, &prio) == OSAL_OK)
                        {
                            /* change only thread priority */
                            sp.sched_priority = prio;
                            /* "man sched_setscheduler" for overview on sheduler policy */
                            /* NOTE: setting SCHED_RR requires root privileges */
                            errno = pthread_setschedparam(thread, policy, &sp);
                        }
                    }
                }
#endif // CONFIG_HOST_OS_WIN32 || (CONFIG_HOST_OS_LINUX && CONFIG_COMPILER_GCC_NATIVE)
            }
            else
            {
                /* get current Linux thread scheduling policy */
                errno = pthread_getschedparam(thread, &policy, &sp);
                if (errno == 0)
                {
                    if (OSAL_to_Linux_ThreadPriority(pcorAttr->u32Priority, policy, &prio) == OSAL_OK)
                    {
                        /* change only thread priority */
                        sp.sched_priority = prio;
                        /* "man sched_setscheduler" for overview on sheduler policy */
                        /* NOTE: setting SCHED_RR requires root privileges */
                        errno = pthread_setschedparam(thread, policy, &sp);
                    }
                }
            }

			if (errno == 0)
			{
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
                OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "ThreadCreate policy: %d OSAL prio: %d Linux prio: %d", policy, pcorAttr->u32Priority, sp.sched_priority);
#endif
				pthread_attr_destroy(&attr);
				return i;
            }
thread_spawn_error:
			pthread_attr_destroy(&attr);
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
thread_spawn_error_noinit:

#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "ThreadSpawn or ThreadCreate (%s)", OSAL_coszErrorText((tS32)errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/
/*****************************************************************************/

tSInt Linux_initProcesses(void)
{
    /* could be EBUSY: attempt to reinitialize the object */
    /* or EINVAL: The value specified by attr is invalid */
    /* the first one should not happen since we are using prhread_once() */
    /* the second one should not happen because we initialize with attr=NULL */
    if (pthread_mutex_init(&thread_info_mutex, NULL)!= 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	memset(&thread_info[0], 0x00, (size_t)sizeof(thread_info));
	return OSAL_OK;
}

tSInt Linux_deinitProcesses(void)
{
	memset(&thread_info[0], 0x00, (size_t)sizeof(thread_info));
    if (pthread_mutex_destroy(&thread_info_mutex)!= 0)
	{
		ASSERT_ON_DEBUGGING(0);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/**
* @brief    Linux_to_OSAL_ThreadPriority
*
* @details  This function convert Linux thread priority into OSAL priority.
*
* @param    linux_prio   Linux thread priority
*           policy      Linux scheduler policy to use for OSAL thread priority convertion
*           osal_prio  Pointer to OSAL thread priority result
*
* @return
*           - OSAL_OK and osal_prio OSAL thread priority convertion result
*           - OSAL_ERROR_INVALID_PARAM if the parameter policy does not identify a defined scheduling policy
*           - OSAL_ERROR otherwise if sched_get_priority_min or sched_get_priority_max return another error
*/
#if 0	/* not used */
static tS32 Linux_to_OSAL_ThreadPriority(tSInt linux_prio, tSInt policy, tU32 *osal_prio)
{
    tS32 ret = OSAL_OK;
    tSInt sched_priority_min, sched_priority_max;

    /* Get scheduler priority min */
    if ((sched_priority_min = sched_get_priority_min(policy)) == -1)
    {
        if (errno == EINVAL)
        {
            /* The parameter policy does not identify a defined scheduling policy */
            ret = OSAL_ERROR_INVALID_PARAM;
        }
        else
        {
            ret = OSAL_ERROR;
        }
    }
    if (ret == OSAL_OK)
    {
        /* Get scheduler priority max */
        if ((sched_priority_max = sched_get_priority_max(policy)) == -1)
        {
            if (errno == EINVAL)
            {
                /* The parameter policy does not identify a defined scheduling policy */
                ret = OSAL_ERROR_INVALID_PARAM;
            }
            else
            {
                ret = OSAL_ERROR;
            }
        }
    }
    if (ret == OSAL_OK)
    {
        /*
         * Calculate Linux sched priority from OSAL priority
         * sched_priority_max -> OSAL_C_U32_THREAD_PRIORITY_HIGHEST
         * sched_priority_min -> OSAL_C_U32_THREAD_PRIORITY_LOWEST
         */
        if (sched_priority_min != sched_priority_max)
        {
            *osal_prio = (tU32)(((((tSInt)OSAL_C_U32_THREAD_PRIORITY_LOWEST - (tSInt)OSAL_C_U32_THREAD_PRIORITY_HIGHEST) * linux_prio) 
                            - (sched_priority_max * (tSInt)OSAL_C_U32_THREAD_PRIORITY_LOWEST) 
                            + (sched_priority_min * (tSInt)OSAL_C_U32_THREAD_PRIORITY_HIGHEST)) 
                            / (sched_priority_min - sched_priority_max));
        }
        else
        {
            /* sched_priority_min == sched_priority_max */
            *osal_prio = (tSInt)OSAL_C_U32_THREAD_PRIORITY_NORMAL;
        }
    }

    return ret;
}
#endif /* #if 0  not used */

/**
* @brief    OSAL_to_Linux_ThreadPriority
*
* @details  This function convert OSAL thread priority into Linux priority.
*
* @param    osal_prio   OSAL thread priority
*           policy      Linux scheduler policy to use for Linux thread priority convertion
*           linux_prio  Pointer to Linux thread priority result
*
* @return
*           - OSAL_OK and linux_prio Linux thread priority convertion result
*           - OSAL_ERROR_INVALID_PARAM if the parameter policy does not identify a defined scheduling policy
*           - OSAL_ERROR otherwise if sched_get_priority_min or sched_get_priority_max return another error
*/
static tS32 OSAL_to_Linux_ThreadPriority(tU32 osal_prio, tSInt policy, tSInt *linux_prio)
{
    tS32 ret = OSAL_OK;
    tSInt sched_priority_min, sched_priority_max;

    /* Get scheduler priority min */
    if ((sched_priority_min = sched_get_priority_min(policy)) == -1)
    {
        if (errno == EINVAL)
        {
            /* The parameter policy does not identify a defined scheduling policy */
            ret = OSAL_ERROR_INVALID_PARAM;
        }
        else
        {
            ret = OSAL_ERROR;
        }
    }
    if (ret == OSAL_OK)
    {
        /* Get scheduler priority max */
        if ((sched_priority_max = sched_get_priority_max(policy)) == -1)
        {
            if (errno == EINVAL)
            {
                /* The parameter policy does not identify a defined scheduling policy */
                ret = OSAL_ERROR_INVALID_PARAM;
            }
            else
            {
                ret = OSAL_ERROR;
            }
        }
    }
    if (ret == OSAL_OK)
    {
        /*
         * Calculate Linux sched priority from OSAL priority
         * OSAL_C_U32_THREAD_PRIORITY_HIGHEST -> sched_priority_max
         * OSAL_C_U32_THREAD_PRIORITY_LOWEST  -> sched_priority_min
         */
        *linux_prio = ((((sched_priority_min - sched_priority_max) * (tSInt)osal_prio) 
                        + (sched_priority_max * (tSInt)OSAL_C_U32_THREAD_PRIORITY_LOWEST) 
                        - (sched_priority_min * (tSInt)OSAL_C_U32_THREAD_PRIORITY_HIGHEST)) 
                        / ((tSInt)OSAL_C_U32_THREAD_PRIORITY_LOWEST - (tSInt)OSAL_C_U32_THREAD_PRIORITY_HIGHEST));
    }

    return ret;
}

/**
* @brief    OSAL_ThreadCreate
*
* @details  This function creates a new Thread in the system.
*           The thread is however not activated i.e. there is no
*           possible allocation to the scheduler. The state of the
*           thread is initialized.
*
* @param    pcorAttr   pointer to a record that contains
*                      the attributes of new thread (I)
*
* @return
*           - ID of the Thread
*           - OSAL_ERROR otherwise
*/
OSAL_tThreadID OSAL_ThreadCreate(const OSAL_trThreadAttribute* pcorAttr)
{
    return internal_thread_spawn(pcorAttr);
}


/**
* @brief     OSAL_ThreadSpawn
*
* @details   This function creates a new thread with the options
*            transferred as parameter. The thread is started
*            immediately and readied the CPU-Queue (ready s32Status).
*            Each new thread receives an own stack with the specified states.
*
* @param    pcorAttr pointer to a record that contains the attributes of new thread (I)
*
* @return
*           - ID of the Thread
*           - OSAL_ERROR otherwise
*/
OSAL_tThreadID OSAL_ThreadSpawn(const OSAL_trThreadAttribute* pcorAttr)
{
    return internal_thread_spawn(pcorAttr);
}

/**
* @brief     OSAL_s32ThreadDelete
*
* @details   Delete a thread and release the stack again.
*
* @param     tid  Thread ID of the thread to be delete
*
* @return
*          - OSAL_OK if everything goes right
*          - OSAL_ERROR otherwise
*/
tS32 OSAL_s32ThreadDelete(OSAL_tThreadID tid)
{
    tS32 s32ReturnValue = OSAL_ERROR;
    void *val;
    struct thread_info_t *ti = NULL;

	if ((tid >= 0) && (tid < LINUX_OSAL_THREAD_MAX))
	{
    	pthread_mutex_lock(&thread_info_mutex);
    	ti = &thread_info[(int)tid];
    	/* unlock here because pthread_cancel may re-schedule */
    	pthread_mutex_unlock(&thread_info_mutex);
	}
    
    if (ti != NULL && ti->in_use == TRUE)
    {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "ThreadDelete %s", ti->name);
#endif
            
        errno = pthread_cancel(ti->tid);
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "Cancelled %s", ti->name);
#endif
        if (errno == 0)
        {
            /* OS21 behavior was to ensure the thread was terminated before exiting */
            errno = pthread_join(ti->tid, &val);
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
            OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "Joined %s", ti->name);
#endif
            if (errno == 0)
            {
                if (val == PTHREAD_CANCELED)
                {
    				pthread_mutex_lock(&thread_info_mutex);
            		ti->in_use = FALSE;
    				pthread_mutex_unlock(&thread_info_mutex);

                    s32ReturnValue = OSAL_OK;
                }
            }
        }
    }
    else
    {
        errno = EINVAL;
    }

    if (errno != 0)
    {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "ThreadDelete (%s)", OSAL_coszErrorText((tS32)errno2osal()));
#endif
        OSAL_s32CallErrorHook(errno2osal());
    }
    return s32ReturnValue;
}


/**
* @brief    OSAL_vThreadExit
*
* @details  This function closes the current thread (Return).
*           The stack is released.
*
*
* @return   NULL
*/
tVoid OSAL_vThreadExit(void)
{
    OSAL_tThreadID tid;
    struct thread_info_t *ti = NULL;
    int i;

    tid = OSAL_ThreadWhoAmI();
    
	if (tid >= 0)
	{
        pthread_mutex_lock(&thread_info_mutex);
        for (i = 0; i < LINUX_OSAL_THREAD_MAX; i++)
        {
            if ((thread_info[i].in_use == TRUE) && (pthread_equal(thread_info[i].tid, thread_info[tid].tid)))
           {
                ti = &thread_info[i];
                ti->in_use = FALSE;
                break;
            }
        }
        pthread_mutex_unlock(&thread_info_mutex);
	}
    ASSERT_ON_DEBUGGING(ti != NULL);
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "ThreadExit %s", ti->name);
#endif
        
    pthread_exit(NULL);
}

/**
* @brief    OSAL_s32ThreadWait
*
* @details  This function suspended the calling thread for a period
*           delayed status.
*
* @param    msec        Time interval in ms (I)
*
*
* @return
*           - OSAL_OK if everythin goes right
*           - OSAL_ERROR otherwise
*/
tS32 OSAL_s32ThreadWait( OSAL_tMSecond msec )
{
	if (msec == OSAL_C_U32_THREAD_SLEEP_NEVER)
	{
		return OSAL_OK;
	}
	return OSAL_s32ThreadWait_us(msec * 1000);
}

/**
* @brief    OSAL_s32ThreadWait_us
*
* @details  This function suspended the calling thread for a period
*           delayed status.
*
* @param    msec        Time interval in us (I)
*
*
* @return
*           - OSAL_OK if everythin goes right
*           - OSAL_ERROR otherwise
*/
tS32 OSAL_s32ThreadWait_us( OSAL_tuSecond usec )
{
#if defined(CONFIG_HOST_OS_LINUX)
    usleep(usec);
#elif defined(CONFIG_HOST_OS_WIN32)
    Sleep(usec / 1000);
#else
#error Unsupported OS
#endif
    return OSAL_OK;
}

/**
* @brief    OSAL_ThreadWhoAmI
*
* @details  This function returns the threadID for the current Thread
*
*
* @return
*           - ID of the Thread
*           - OSAL_ERROR otherwise
*/
OSAL_tThreadID OSAL_ThreadWhoAmI( void )
{
	return Linux_search_thread_info_entry(pthread_self());
}

#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
/**
 * @brief     OSAL_s32GetPriority
 *
 * @details   Get the Priority of the Thread
 *
 * @param     tid                  ID of the thread (I)
 * @param     pu32CurrentPriority Pointer to the priority value to return (O)
 *
 * @return
 *           - OSAL_OK if everythin goes right
 *           - OSAL_ERROR otherwise
 */
tS32 OSAL_s32GetPriority(OSAL_tThreadID tid, tU32 *pu32CurrentPriority)
{
	/* empty function, just a placeholder */

    *pu32CurrentPriority = OSAL_C_U32_THREAD_PRIORITY_LOWEST;

    return OSAL_OK;
}
#endif

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
