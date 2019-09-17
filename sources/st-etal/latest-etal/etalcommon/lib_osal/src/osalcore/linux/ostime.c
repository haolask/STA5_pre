//!
//!  \file 		ostime.c
//!  \brief 	<i><b>OSAL Timers Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) timer Functions.
//!  \author 	Raffaele Belardi
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		07 Sept 2010
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
#include <signal.h>


/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#define USEC_MAX   1000000

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/
static struct timeval startup_tv;

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT

/**
 *
 * @brief    OSAL_u32TimerGetResolution
 *
 * @details  This Function returns the time resolution of the timer in Hz.
 *              A resolution of 100 Hz, for example, means that the minimum
 *              distinguishable time interbval rapresents 10 milliseconds. It
 *              is assumed that the resolution of all stop is identical.
 *
 *
 * @return   Time resolution
 *
 */
tU32 OSAL_u32TimerGetResolution(void)
{
    struct timespec res;
    
    if (clock_getres(CLOCK_REALTIME, &res) == 0)
    {
        return (tU32) 1 / res.tv_sec;
    }
    /* OSAL spec does not allow for an error return, but the Linux primitives can return error */
    ASSERT_ON_DEBUGGING(1);
    return 0;
}

/**
 *
 * @brief    OSAL_s32TimerCreate
 *
 * @details  This function creates a timer. After expiry of a time interval
 *              the specified parameter is called. After creation of the timer
 *              no time interval is set.
 *
 * @param   pCallback pointer to a callback function during expiry of time interval
 * @param   pvArg     argument of callback function
 * @param   phTimer   pointer to the timer handle
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32TimerCreate(OSAL_tpfCallback pCallback,
                         tPVoid pvArg,
                         OSAL_tTimerHandle* phTimer)

{
	struct sigevent ev;
    typedef void (*func_callback)(union sigval); /* used below for compiler warning removal */

    if((pCallback != NULL) && (phTimer != NULL))
    {
        ev.sigev_notify = SIGEV_THREAD;
        ev.sigev_notify_function = (func_callback)pCallback;
        ev.sigev_value = (union sigval) pvArg;
        ev.sigev_notify_attributes = NULL;


        if (timer_create(CLOCK_REALTIME, &ev, phTimer) == 0)
        {
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
            if (STRACE_CONTROL) OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "TimerCreate (0x%x)", *phTimer);
#endif
            return OSAL_OK;
        }
    }
    else
    {
        errno = EINVAL;
    }

#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "TimerCreate (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}


/**
 *
 * @brief   OSAL_s32TimerDelete
 *
 * @details This function delete a timer.
 *
 * @param   hTimer timer handle to be removed (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32TimerDelete(OSAL_tTimerHandle hTimer)
{
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "TimerDelete (0x%x)", hTimer);
#endif

    if (hTimer != NULL)
    {
        if (timer_delete(hTimer) == 0)
        {
            return OSAL_OK;
        }
    }
    else
    {
        errno = EINVAL;
    }

#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "TimerDelete (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
 *
 * @brief   OSAL_s32TimerGetTime
 *
 * @details With this function the remaining time interval of a timer up
 *              to the calling of callback function can be determinated.
 *
 * @param  hTimer Timer handle to get time (I)
 * @param  pMSec remainig time in ms or OSAL_NULL (->O)
 * @param  pInterval repetition Interval in ms or OSAL_NULL (->O)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32TimerGetTime(OSAL_tTimerHandle hTimer,
                          OSAL_tMSecond* pMSec,
                          OSAL_tMSecond* pInterval)
{
    struct itimerspec curr_value;

    if (hTimer != NULL)
    {
        if (timer_gettime(hTimer, &curr_value) == 0)
        {
            if (pMSec != NULL)
            {
                *pMSec = curr_value.it_value.tv_sec * 1000 + curr_value.it_value.tv_nsec / 1000000;
            }
            if (pInterval != NULL)
            {
                *pInterval = curr_value.it_interval.tv_sec * 1000 + curr_value.it_interval.tv_nsec / 1000000;
            }
            return OSAL_OK;
        }
    }
    else
    {
        errno = EINVAL;
    }
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "TimerGetTime (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

/**
 *
 * @brief    OSAL_s32TimerSetTime
 *
 * @details  With vthis function the timer is started ("wound up").The time
 *              interval up to the first time triggering of the call back
 *              function is relative to the current point of time. Through the
 *              specification of an additional interval the call back function
 *              can be triggered periodically. If the timer is active, i.e. a
 *              trigger time point id defined, wich has not been reached yet,
 *              the time point can be changed by renewed calling of this
 *              function or deleted by specific 0 as second parameter
 *
 * @param   hTimer Timer handle to set time (I)
 * @param   msec   Time point up till the first time triggering (I)
 * @param   interval repetition Interval in ms (period) (I)
 *
 * @return
 *        - OSAL_OK if everything goes right
 *        - OSAL_ERROR otherwise
 *
 */
tS32 OSAL_s32TimerSetTime(OSAL_tTimerHandle hTimer,
                          OSAL_tMSecond msec,
                          OSAL_tMSecond interval)
{
    struct itimerspec new_value;

	if (hTimer != NULL)
    {
		new_value.it_value.tv_sec = msec / 1000;
	    new_value.it_value.tv_nsec = (msec % 1000) * 1000000;
	    new_value.it_interval.tv_sec = interval / 1000;
	    new_value.it_interval.tv_nsec = (interval % 1000) * 1000000;

	    if (timer_settime(hTimer, 0, &new_value, NULL) == 0)
	    {
	    	return OSAL_OK;
	    }
	}
    else
    {
        errno = EINVAL;
    }
    
#if defined(CONFIG_ENABLE_CLASS_OSALCORE) && (CONFIG_ENABLE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
    OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "TimerSetTime (%s)", OSAL_coszErrorText(errno2osal()));
#endif
    OSAL_s32CallErrorHook(errno2osal());
    return OSAL_ERROR;
}

#endif

/**
 *
 * @brief    OSAL_ClockGetElapsedTime;
 *
 * @details  This Function returns the elapsed time since the start of the
 *              system through OSAL_Boot() in milliseconds.
 *
 * @return   Time in milliseconds
 *
 */
OSAL_tMSecond OSAL_ClockGetElapsedTime(void)
{
	struct timeval tv;
	int sec, usec;

	gettimeofday(&tv, NULL);
	if (startup_tv.tv_usec > tv.tv_usec)
	{
		tv.tv_sec--;
		tv.tv_usec += USEC_MAX;
	}
	sec = tv.tv_sec - startup_tv.tv_sec;
	usec = tv.tv_usec - startup_tv.tv_usec;
    return (OSAL_tMSecond)((sec * 1000) + (usec / 1000));
}

void OSAL_ClockResetTime(void)
{
	gettimeofday(&startup_tv, NULL);
}

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
