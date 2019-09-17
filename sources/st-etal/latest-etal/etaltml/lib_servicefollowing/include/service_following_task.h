//!
//!  \file 		DabMwTask.h
//!  \brief 	<i><b> Dab Middleware task header file </b></i>
//!  \details	This file is the Dab Middleware header.
//!  \author 	Alberto Saviotti
//!  \author 	(original version) Alberto Saviotti
//!  \version 	1.0
//!  \date 		Maybe someone knows
//!  \bug 		Unknown
//!  \warning	None
//!


#ifndef SERVICE_FOLLOWING_TASK_H_
#define SERVICE_FOLLOWING_TASK_H_



#ifdef __cplusplus
extern "C" {
#endif


/*
*********************
* DEFINE SECTION
**********************
*/
#ifdef CONFIG_ETAL_HAVE_ETALTML

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

// manage the compile switches for Servicefollowing : mapped to ETAL
#if defined(CONFIG_ETAL_HAVE_SEAMLESS) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifndef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
#define CONFIG_APP_SEAMLESS_SWITCHING_BLOCK 1
#endif
#endif

#ifndef CONFIG_DABMW_SERVICEFOLLOWING_SUPPORTED
#define CONFIG_DABMW_SERVICEFOLLOWING_SUPPORTED
#endif
#endif

#ifdef CONFIG_ETALTML_HAVE_RADIOTEXT
#ifndef CONFIG_TARGET_DABMW_RDS_COMM_ENABLE
#define CONFIG_TARGET_DABMW_RDS_COMM_ENABLE
#endif
#endif

#ifdef CONFIG_ENABLE_CLASS_APP_DABMW_SF
#undef CONFIG_ENABLE_CLASS_APP_DABMW
#ifndef CONFIG_ENABLE_CLASS_APP_DABMW
#define CONFIG_ENABLE_CLASS_APP_DABMW TR_LEVEL_SYSTEM 
#endif
#endif

// other defines 
#define CONFIG_TARGET_DABMW_AMFMDEV_COMM_ENABLE
#define CONFIG_TARGET_DABMW_TUNERDEV_COMM_ENABLE

// tuner config
// indicate that DAB and FM do not share tuners...
// correction : the ETAL_CAPA_MAX_TUNER does not reflect the real number of TUNER now
// we should rely on MTD module or not
//
//#if defined (ETAL_CAPA_MAX_TUNER) && (ETAL_CAPA_MAX_TUNER > 1)
#ifdef CONFIG_MODULE_INTEGRATED
#define CONFIG_SERVICE_FOLLOWING_DEDICATED_TUNER_DAB_FM
#endif  

#endif // CONFIG_ETAL_HAVE_ETALTML


// Event handler
/* EPR comment :  EVENT BUFFER Size is used to define several event queue if needed
* keep 1 
*/
#define DABMW_EVENT_BUFFER_SIZE                 1
	
	
// Call mask
#define DABMW_INITIAL_CALL_MASK                 (tU32)0x80000000
	

	/* EPR CHANGE 
	* ADD EVENT for SERVICE FOLLOWING WAKE-UP
	*/
	/* EVENT for PI reception and triggering SF */
#define DABMW_SF_EVENT_PI_RECEIVED    			    1
	/* EVENT for DAB tuning information and  triggering SF */
#define DABMW_SF_EVENT_DAB_TUNE						2
	/* EVENT for DAB tuning information and  triggering SF */
#define DABMW_SF_EVENT_DAB_FIC_READ					3
	/* EVENT auto seek received */
#define DABMW_SF_EVENT_AUTO_SEEK					4
	
	/* EVENT for SS ESTIMATION RESPONSE */
#define DABMW_SF_EVENT_SS_ESTIMATION_RSP			5
	
	/* EVENT for SS SWITCHING RESPONSE */
#define DABMW_SF_EVENT_SS_SWITCHING_RSP 			6

	/* EVENT for PS reception and triggering SF */
#define DABMW_SF_EVENT_PS_RECEIVED    			    7

	
	
	/* Add a number of event */
#define DABMW_LAST_EVENT	DABMW_SF_EVENT_SS_SWITCHING_RSP
	/* END EPR CHANGE */


	/* EPR CHANGE */
	/* ADD EVENT for SERVICE FOLLOWING WAKE-UP */
#define DABMW_SF_NO_EVENT_FLAG					0x00



	/* EVENT for PI reception and triggering SF */
#define DABMW_SF_EVENT_PI_RECEIVED_FLAG         ((tU32)0x01 << DABMW_SF_EVENT_PI_RECEIVED)		 
	/* EVENT for DAB tuning information and  triggering SF */
#define DABMW_SF_EVENT_DAB_TUNE_FLAG			((tU32)0x01 << DABMW_SF_EVENT_DAB_TUNE)
#define DABMW_SF_EVENT_DAB_FIC_READ_FLAG		((tU32)0x01 << DABMW_SF_EVENT_DAB_FIC_READ)
	/* FM AUTO SEEK EVENT */
#define DABMW_SF_EVENT_AUTO_SEEK_FLAG			((tU32)0x01 << DABMW_SF_EVENT_AUTO_SEEK)
	
	/* SS Estimation Response */
#define DABMW_SF_EVENT_SS_ESTIMATION_RSP_FLAG	((tU32)0x01 << DABMW_SF_EVENT_SS_ESTIMATION_RSP)
	
	// SS Switch Response
#define DABMW_SF_EVENT_SS_SWITCHING_RSP_FLAG	((tU32)0x01 << DABMW_SF_EVENT_SS_SWITCHING_RSP)


	/* EVENT for PS reception and triggering SF */
#define DABMW_SF_EVENT_PS_RECEIVED_FLAG         ((tU32)0x01 << DABMW_SF_EVENT_PS_RECEIVED)	

	
#define DABMW_SF_EVENT_FLAGS	(DABMW_SF_EVENT_PI_RECEIVED_FLAG | DABMW_SF_EVENT_DAB_TUNE_FLAG | DABMW_SF_EVENT_DAB_FIC_READ_FLAG | DABMW_SF_EVENT_AUTO_SEEK_FLAG | DABMW_SF_EVENT_SS_ESTIMATION_RSP_FLAG | DABMW_SF_EVENT_SS_SWITCHING_RSP_FLAG | DABMW_SF_EVENT_PS_RECEIVED_FLAG)
	
#define SF_EVENT_WAIT_MASK	(DABMW_SF_EVENT_FLAGS )



/// WAITING TASK 

// EVENTS

// for Kill

// Kill event	
#define	DABMW_SF_EVENT_KILL							0

// for Wake-up
#define DABMW_SF_EVENT_WAKE_UP						31


// FLAGS
#define	DABMW_SF_EVENT_KILL_FLAG				((tU32)0x01 << DABMW_SF_EVENT_KILL)	

#define DABMW_SF_EVENT_WAEKUP_FLAG				((tU32)0x01 << DABMW_SF_EVENT_WAKE_UP)


// WAKEUP FULL FLAGS & MASK
#define DABMW_SF_WAKEUP_FLAGS					(DABMW_SF_EVENT_KILL_FLAG | DABMW_SF_EVENT_WAEKUP_FLAG)

#define SF_EVENT_WAIT_WAKEUP_MASK				(DABMW_SF_WAKEUP_FLAGS)

#define SF_EVENT_WAIT_ALL						(0xFFFFFFFF)

//#define SF_EVENT_WAIT_TIME_MS					OSAL_C_TIMEOUT_FOREVER
#define SF_EVENT_WAIT_TIME_MS					100



#define ETALTML_SERVICE_FOLLOWING_THREAD_PRIORITY 	OSAL_C_U32_THREAD_PRIORITY_NORMAL
#define ETALTML_SERVICE_FOLLOWING_STACK_SIZE		16*1024


/*
*********************
* STRUCTURE SECTION
**********************
*/

/*
*********************
* VARIABLE SECTION
**********************
*/

#ifdef SERVICE_FOLLOWING_TASK_C	
static OSAL_tMSecond DABMW_deltaExecTime = 0;
static OSAL_tEventHandle ServiceFollowing_TaskEventHandler;
#endif

/*
*********************
* FUNCTIONS SECTION
**********************
*/

#ifndef SERVICE_FOLLOWING_TASK_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

// static internal.
#ifdef SERVICE_FOLLOWING_TASK_C
static tVoid ETALTML_ServiceFollowing_Task (tPVoid);
#endif

GLOBAL tSInt ETALTML_ServiceFollowing_task_init (tVoid);
GLOBAL tSInt ETALTML_ServiceFollowing_task_deinit (tVoid);

GLOBAL tVoid ETALTML_ServiceFollowing_TaskWakeUpOnEvent(tU32 event);
GLOBAL tVoid ETALTML_ServiceFollowing_TaskClearEvent(tU32 event);
GLOBAL tVoid ETALTML_ServiceFollowing_TaskClearEventFlag(tU32 eventFlag);
GLOBAL OSAL_tMSecond ETALTML_ServiceFollowing_GetDeltaExecTime(tVoid);
GLOBAL OSAL_tMSecond ETALTML_ServiceFollowing_CalculateDeltaExecTime(tVoid);

#undef GLOBAL


#endif // #ifndef SERVICE_FOLLOWING_TASK_H_

// End of file

