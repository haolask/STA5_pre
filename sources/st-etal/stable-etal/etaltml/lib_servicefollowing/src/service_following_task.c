//!
//!  \file      SERVICE_FOLLWING_TASK.C
//!  \brief     <i><b> Dab Middleware entry point </b></i>
//!  \details   This file contains the Dab Middleware entry point.
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      Maybe someone knows
//!  \bug       Unknown
//!  \warning   None
//!


 
#ifndef SERVICE_FOLLOWING_TASK_C
#define SERVICE_FOLLOWING_TASK_C

#include "osal.h"

#if defined(CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
#include "etalinternal.h"

#include "dabmw_import.h"
#include "service_following_task.h"

/* if seamless active */
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "seamless_switching_common.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)


#include "service_following.h"

#include "service_following_internal.h"

#include "service_following_log_status_info.h"

#include "service_following_background.h"

#include "service_following_mainloop.h"

#include "service_following_meas_and_evaluate.h"

#include "service_following_audioswitch.h"

/* if seamless active */
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#include "service_following_seamless.h"
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)



#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_ServiceFollowing_WaitingTask(tSInt stacd, tPVoid thread_param);
#else
static tVoid ETALTML_ServiceFollowing_WaitingTask (tPVoid thread_param);
#endif

OSAL_tThreadID ETALTML_SF_ThreadId;

OSAL_tMSecond ETALTML_ServiceFollowing_GetDeltaExecTime (tVoid);
OSAL_tMSecond ETALTML_ServiceFollowing_CalculateDeltaExecTime (tVoid);

//!
//! \brief      <i><b> DABMW_TaskWakeUpOnEvent </b></i>
//! \details    Functions to wake-up the middlewar task on event.
//!             The main loop for the task is waked-up here.
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETALTML_ServiceFollowing_TaskWakeUpOnEvent (tU32 event)
{
 
/* EPR change 
* ADD EVENT  for SERVICE FOLLOWING
*/
	if ((DABMW_SF_EVENT_PI_RECEIVED == event)
		|| (DABMW_SF_EVENT_PS_RECEIVED == event)
		|| (DABMW_SF_EVENT_DAB_TUNE == event)
		|| (DABMW_SF_EVENT_DAB_FIC_READ == event)
		|| (DABMW_SF_EVENT_AUTO_SEEK == event)
		|| (DABMW_SF_EVENT_SS_ESTIMATION_RSP == event)
		|| (DABMW_SF_EVENT_SS_SWITCHING_RSP == event)
		)
		{
		OSAL_s32EventPost (ServiceFollowing_TaskEventHandler, 
                           ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);
		}
	else if ((DABMW_SF_EVENT_WAKE_UP == event))
	{
		// wakeup case
		OSAL_s32EventPost (ServiceFollowing_TaskEventHandler, 
                           ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);
	}
	else if (DABMW_SF_EVENT_KILL == event)
	{
		// wakeup case
		OSAL_s32EventPost (ServiceFollowing_TaskEventHandler, 
                           ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);
	}
	
}


/* EPR change 
* ADD a CLEAR EVENT  for SERVICE FOLLOWING
*/

//!
//! \brief      <i><b> tVoid DABMW_TaskClearEvent (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETALTML_ServiceFollowing_TaskClearEvent (tU32 event)
{

		 // Clear old event if any (this can happen after a stop)
	 OSAL_s32EventPost (ServiceFollowing_TaskEventHandler, 
                           (~((tU32)0x01 << event)), OSAL_EN_EVENTMASK_AND);
}

//!
//! \brief      <i><b> tVoid DABMW_TaskClearEventFlag (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETALTML_ServiceFollowing_TaskClearEventFlag(tU32 eventFlag)
{

		 // Clear old event if any (this can happen after a stop)
	 OSAL_s32EventPost (ServiceFollowing_TaskEventHandler, 
                           ~eventFlag, OSAL_EN_EVENTMASK_AND);
}



#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETALTML_ServiceFollowing_WaitingTask(tSInt stacd, tPVoid thread_param)
#else
tVoid ETALTML_ServiceFollowing_WaitingTask (tPVoid thread_param)
#endif
{
	tSInt vl_res;
	OSAL_tEventMask vl_resEvent = DABMW_SF_NO_EVENT_FLAG;
	//tSInt printfCnt = 0;
	
	while (1)
	{
		DABMW_SF_PRINTF(TR_LEVEL_USER_4, "ETALTML_ServiceFollowing_WaitingTask : waiting SF activation\n");

		// Wait here the SF wake up event
		//
		vl_res = OSAL_s32EventWait (ServiceFollowing_TaskEventHandler,
									 SF_EVENT_WAIT_ALL, 
									 OSAL_EN_EVENTMASK_OR, 
									 OSAL_C_TIMEOUT_FOREVER,
									 &vl_resEvent);
	
			///* clear the received event */
			//		ETALTML_ServiceFollowing_TaskClearEventFlag(vl_resEvent);
	
			if ((vl_resEvent == DABMW_SF_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
			{
				// this is a timeout : no even received
				// trigger the SF task normal processing
				
			}
			// Service following event trigger Service Following event processing
			// else normal handling ie if res = OSAL_ERROR => this is a timeout, so 
			// normal processing or event return flag contains EVENT_CMD.. so normal processing
			else
			{
				// This is a SF EVENT WAKE UP call the SF event handler
				if (DABMW_SF_EVENT_WAEKUP_FLAG == (DABMW_SF_EVENT_WAEKUP_FLAG & vl_resEvent))
				{
					DABMW_SF_PRINTF(TR_LEVEL_USER_4, "ETALTML_ServiceFollowing_WaitingTask : awaked \n");
					// clear here all pending event
					// to avoid crossing cases
					// of any event which could have been posted ~
					ETALTML_ServiceFollowing_TaskClearEventFlag(DABMW_SF_EVENT_WAEKUP_FLAG);
					ETALTML_ServiceFollowing_Task(thread_param);
					/* clear the received event */
	
				}
				else if (DABMW_SF_EVENT_KILL_FLAG == (DABMW_SF_EVENT_KILL_FLAG & vl_resEvent))
				{
					// this is the end
					// go out of the while
					break;
				}
				else
				{
					// the event is not expected : this is a crossing case.
					// just clear the even
					
					ETALTML_ServiceFollowing_TaskClearEventFlag(vl_resEvent);
				}
			}
	}

	// this is the end of the task.
	// Delete the event and end

#ifndef OSAL_EVENT_SKIP_NAMES
	if (OSAL_s32EventClose(ServiceFollowing_TaskEventHandler) != OSAL_OK)
	{
		vl_res = OSAL_ERROR;
	}

	OSAL_s32ThreadWait(100);

	if (OSAL_s32EventDelete("ServiceFollowing_EventHandler") != OSAL_OK)
	{
		vl_res = OSAL_ERROR;
	}
#else
	if (OSAL_s32EventFree(ServiceFollowing_TaskEventHandler) != OSAL_OK)
    {
        return;
    }
#endif

	ServiceFollowing_TaskEventHandler = (OSAL_tEventHandle)0;

#ifdef CONFIG_HOST_OS_TKERNEL
	// the task should not exist on its own, else it leads to deinit crash
	// keep a waiting while loop infinit
	while(1) 
	{ 
		OSAL_s32ThreadWait(100);
	}
#endif
	

}

//!
//! \brief      <i><b> DABMW_Task </b></i>
//! \details    Entry point for the Middleware task.
//!             The main loop for the task is called here.
//! \param[in]  tVoid           None
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETALTML_ServiceFollowing_Task (tPVoid thread_param)
{

    tSInt vl_res;
	OSAL_tEventMask vl_resEvent = DABMW_SF_NO_EVENT_FLAG;
    //tSInt printfCnt = 0;


   

    while (1)
    {

	
//		DABMW_SF_PRINTF(TR_LEVEL_USER_1, "ETALTML_ServiceFollowing_Task : waiting event SF\n");
		
        // Wait here 
        vl_res = OSAL_s32EventWait (ServiceFollowing_TaskEventHandler,
                                 SF_EVENT_WAIT_ALL, 
                                 OSAL_EN_EVENTMASK_OR, 
                                 SF_EVENT_WAIT_TIME_MS,
                                 &vl_resEvent);

//		DABMW_SF_PRINTF(TR_LEVEL_USER_1, "ETALTML_ServiceFollowing_Task : vl_res = %d, vl_resEvent = %d\n", vl_res, vl_resEvent);
		

		///* clear the received event */
		//		ETALTML_ServiceFollowing_TaskClearEventFlag(vl_resEvent);

		if (false == DABWM_ServiceFollowing_IsEnable())
		{
			// SF not activated, leave that function.
			break;
		}	
		else if ((vl_resEvent == DABMW_SF_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
		{
			// this is a timeout : no even received
			// trigger the SF task normal processing
			DABMW_ServiceFollowing();
		}
		// Service following event trigger Service Following event processing
	    // else normal handling ie if res = OSAL_ERROR => this is a timeout, so 
		// normal processing or event return flag contains EVENT_CMD.. so normal processing
		else
		{
			// This is a SF EVENT WAKE UP call the SF event handler
			if (DABMW_SF_EVENT_PI_RECEIVED_FLAG == (DABMW_SF_EVENT_PI_RECEIVED_FLAG & vl_resEvent))
			{
				DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_PI_RECEIVED);
			}
			// This is a SF EVENT WAKE UP call the SF event handler
			if (DABMW_SF_EVENT_PS_RECEIVED_FLAG == (DABMW_SF_EVENT_PS_RECEIVED_FLAG & vl_resEvent))
			{
				DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_PS_RECEIVED);
			}
			if (DABMW_SF_EVENT_DAB_TUNE_FLAG == (DABMW_SF_EVENT_DAB_TUNE_FLAG & vl_resEvent))
			{
			    DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_DAB_TUNE);
			}
			if (DABMW_SF_EVENT_DAB_FIC_READ_FLAG == (DABMW_SF_EVENT_DAB_FIC_READ_FLAG & vl_resEvent))
			{
				DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_DAB_FIC_READ);
			} 
			if (DABMW_SF_EVENT_AUTO_SEEK_FLAG == (DABMW_SF_EVENT_AUTO_SEEK_FLAG & vl_resEvent))
			{
				DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_AUTO_SEEK);
			} 
			if (DABMW_SF_EVENT_SS_ESTIMATION_RSP_FLAG == (DABMW_SF_EVENT_SS_ESTIMATION_RSP_FLAG & vl_resEvent))
			{
				DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_SS_ESTIMATION_RSP);
			} 
            if (DABMW_SF_EVENT_SS_SWITCHING_RSP_FLAG == (DABMW_SF_EVENT_SS_SWITCHING_RSP_FLAG & vl_resEvent))
			{
				DABMW_ServiceFollowing_EventHandling(DABMW_SF_EVENT_SS_SWITCHING_RSP);
			}

			// unexepected event case
			if (!(SF_EVENT_WAIT_MASK & vl_resEvent))
			{
				// just clear the event
				ETALTML_ServiceFollowing_TaskClearEventFlag(vl_resEvent);
			}
		}
    }


}


//!
//! \brief      <i><b> ETALTML_ServiceFollowing_task_init </b></i>
//! \details    Entry point for the Middleware task.
//!             The task is created and the initialization of mandatory
//!             stuff is done here.
//! \param[in]  tVoid           None
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!

tSInt ETALTML_ServiceFollowing_task_init (tVoid)
{
    OSAL_trThreadAttribute attr0;
	tSInt vl_res = OSAL_OK;
	
    // Init globals
    ServiceFollowing_TaskEventHandler = (OSAL_tEventHandle)0;

    
    OSAL_s32EventCreate ((tCString)"ServiceFollowing_EventHandler", &ServiceFollowing_TaskEventHandler);    

    // Pass data to the service following in order to setup current user
    vl_res = DABMW_ServiceFollowingInit ();

	if (OSAL_OK != vl_res)
	{
    	DABMW_SF_PRINTF(TR_LEVEL_ERRORS, "DABMW Error: Service following init failed, return code is %d", vl_res);
	}
	
  	// TASK INIT

	attr0.szName = (tChar *)ETALTML_SERVICE_FOLLOWING_THREAD_NAME;
	attr0.u32Priority = ETALTML_SERVICE_FOLLOWING_THREAD_PRIORITY;
	attr0.s32StackSize = ETALTML_SERVICE_FOLLOWING_STACK_SIZE;
	attr0.pfEntry = ETALTML_ServiceFollowing_WaitingTask;
	attr0.pvArg = NULL;

	if ((ETALTML_SF_ThreadId = OSAL_ThreadSpawn(&attr0)) == OSAL_ERROR)
	{
		return OSAL_ERROR_DEVICE_INIT;
	}



	// init  the default trace level
#ifdef  CONFIG_ENABLE_CLASS_APP_DABMW_SF
#ifndef CONFIG_DEFAULT_CLASS_APP_DABMW_SF_LEVEL
#define CONFIG_DEFAULT_CLASS_APP_DABMW_SF_LEVEL 3
#endif
	OSALUTIL_s32TraceSetFilterWithMask( TR_CLASS_APP_DABMW_SF, TR_CLASS_APP_DABMW_SF, CONFIG_DEFAULT_CLASS_APP_DABMW_SF_LEVEL );
#endif

	return(OSAL_OK);


}

//!
//! \brief      Stops the Service Following task and releases its resources
//! \return     OSAL_OK, OSAL_ERROR
//! \callgraph
//! \callergraph
//!
tSInt ETALTML_ServiceFollowing_task_deinit (tVoid)
{
	tSInt vl_res = OSAL_OK;


	
	//disbale SF
	DABWM_ServiceFollowing_Disable();

	// send the kill event
	ETALTML_ServiceFollowing_TaskWakeUpOnEvent(DABMW_SF_EVENT_KILL);
	OSAL_s32ThreadWait(10);

	if (OSAL_s32ThreadDelete(ETALTML_SF_ThreadId) != OSAL_OK)
	{
		vl_res = OSAL_ERROR;
	}	


	DABMW_SF_PRINTF(TR_LEVEL_USER_4, "ETALTML_ServiceFollowing_task_deinit : deinit SF requested and done\n");

	return vl_res;
}


OSAL_tMSecond ETALTML_ServiceFollowing_GetDeltaExecTime (tVoid)
{
    return DABMW_deltaExecTime;
}

OSAL_tMSecond ETALTML_ServiceFollowing_CalculateDeltaExecTime (tVoid)
{
    static OSAL_tMSecond currentElapsedTime = 0;
    static OSAL_tMSecond lastElapsedTime = 0;

    currentElapsedTime = OSAL_ClockGetElapsedTime ();

    DABMW_deltaExecTime = abs(currentElapsedTime - lastElapsedTime);

    lastElapsedTime = currentElapsedTime;

    return DABMW_deltaExecTime;
}

#ifdef __cplusplus
}
#endif


#endif // #ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

#endif // service_following_task_c

// End of file
