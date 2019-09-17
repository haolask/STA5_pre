//!
//!  \file 		etalcontrol.c
//!  \brief 	<i><b> ETAL Control thread </b></i>
//!  \details   The ETAL Control thread runs continuously and schedules the
//!				actions normally performed in the background, not directly
//!				influenced by user interaction.
//!
//!				This file also contains the IRQ thread, a thread created
//!				only in some ETAL configurations.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
	#include "bsp_sta1095evb.h"
#endif

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	// EVENTS
	
	// CALLBACK Periodic Registration 	event bit 0
#define	ETAL_CONTROL_EVENT_CALLBACK_START				0
		
	// CALLBACK Periodic Deregistration	event bit 1
#define	ETAL_CONTROL_EVENT_CALLBACK_STOP				1

	// CALLBACK Monirot event START bit 2
#define	ETAL_CONTROL_EVENT_MONITOR_START				2

	// CALLBACK Monirot event STOP bit 3
#define	ETAL_CONTROL_EVENT_MONITOR_STOP					3

	// FLAGS
#define	ETAL_CONTROL_EVENT_CALLBACK_START_FLAG			((tU32)0x01 << ETAL_CONTROL_EVENT_CALLBACK_START)	
#define	ETAL_CONTROL_EVENT_CALLBACK_STOP_FLAG			((tU32)0x01 << ETAL_CONTROL_EVENT_CALLBACK_STOP)	
#define	ETAL_CONTROL_EVENT_MONITOR_START_FLAG			((tU32)0x01 << ETAL_CONTROL_EVENT_MONITOR_START)	
#define ETAL_CONTROL_EVENT_MONITOR_STOP_FLAG			((tU32)0x01 << ETAL_CONTROL_EVENT_MONITOR_STOP)	
	
	// WAKEUP FULL FLAGS & MASK
#define ETAL_CONTROL_EVENT_FLAGS			(ETAL_CONTROL_EVENT_CALLBACK_START_FLAG|ETAL_CONTROL_EVENT_CALLBACK_STOP_FLAG|ETAL_CONTROL_EVENT_MONITOR_START_FLAG|ETAL_CONTROL_EVENT_MONITOR_STOP_FLAG)
	
#define ETAL_CONTROL_EVENT_WAIT_MASK		(ETAL_CONTROL_EVENT_FLAGS)
	
#define ETAL_CONTROL_EVENT_WAIT_ALL			(0xFFFFFFFF)

#define ETAL_CONTROL_EVENT_WAIT_TIME_MS		OSAL_C_TIMEOUT_FOREVER
//#define ETAL_CONTROL_EVENT_WAIT_TIME_MS		ETAL_CONTROL_THREAD_SCHEDULING
	
#define ETAL_CONTROL_NO_EVENT_FLAG			0x00

	//
	// IRQ PART :
	//
#define	ETAL_CONTROL_EVENT_IRQ_RDS_START				0

	// FLAGS
#define	ETAL_CONTROL_EVENT_IRQ_RDS_START_FLAG			((tU32)0x01 << ETAL_CONTROL_EVENT_IRQ_RDS_START)	
	
	// WAKEUP FULL FLAGS & MASK
#define ETAL_CONTROL_EVENT_IRQ_RDS_FLAGS			(ETAL_CONTROL_EVENT_IRQ_RDS_START_FLAG)
	
#define ETAL_CONTROL_EVENT_IRQ_RDS_WAIT_MASK		(ETAL_CONTROL_EVENT_IRQ_RDS_FLAGS)
	
#define ETAL_CONTROL_EVENT_IRQ_RDS_WAIT_ALL			(0xFFFFFFFF)

#define ETAL_CONTROL_EVENT_IRQ_RDS_WAIT_TIME_MS		OSAL_C_TIMEOUT_FOREVER
//#define ETAL_CONTROL_EVENT_IRQ_RDS_WAIT_TIME_MS		ETAL_CONTROL_THREAD_SCHEDULING
	
#define ETAL_CONTROL_IRQ_RDS_NO_EVENT_FLAG			0x00


#endif // CONFIG_ETAL_CPU_IMPROVEMENT

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
#if defined(CONFIG_HOST_OS_TKERNEL) && defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	OSAL_tSemHandle ETAL_CONTROL_IRQ_Sem;
#endif

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT

static OSAL_tEventHandle 	ETAL_control_TaskEventHandler;

#endif // CONFIG_ETAL_CPU_IMPROVEMENT

#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
static OSAL_tEventHandle 	ETAL_control_IrqRds_TaskEventHandler;
#endif


/*****************************************************************
| functions prototype
|----------------------------------------------------------------*/
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
static tBool ETAL_isMonitoredIndicatorTypeCMOST(EtalBcastQaIndicators ind);
static tBool ETAL_controlCheckMonitorValue_CMOST(EtalBcastQualityContainer *qual, EtalQaMonitoredEntryAttr *mon);
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
static tBool ETAL_controlCheckMonitorValue_HDRADIO(EtalBcastQualityContainer *qual, EtalQaMonitoredEntryAttr *mon);
static tBool ETAL_isMonitoredIndicatorTypeHDRADIO(EtalBcastQaIndicators ind);
#endif // #if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
static tBool ETAL_controlCheckMonitorValue_DAB(EtalBcastQualityContainer *qual, EtalQaMonitoredEntryAttr *mon);
static tBool ETAL_isMonitoredIndicatorTypeDAB(EtalBcastQaIndicators ind);
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
static tVoid ETAL_control_MonitoringCallbackEnable(tVoid);
static tVoid ETAL_control_MonitoringCallbackDisable(tVoid);
static tVoid ETAL_control_MonitorTimerCallBack(tPVoid pI_Arg);
#endif // CONFIG_ETAL_CPU_IMPROVEMENT

#endif // #if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
static tVoid ETAL_control_TaskClearEventFlag(tU32 eventFlag);
static tVoid ETAL_control_TaskWakeUpOnEvent (tU32 event);
//static tVoid ETAL_control_TaskWakeUpOnEventFlag (tU32 eventFlag);
#endif


#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
static tVoid ETAL_IRQ_Processing(tVoid);
static tVoid ETAL_control_IrqRds_TaskClearEventFlag(tU32 eventFlag);
static tVoid ETAL_control_IrqRds_TaskWakeUpOnEvent (tU32 event);
#endif




#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
//!
//! \brief      <i><b> tVoid ETAL_control_TaskClearEventFlag (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_control_TaskClearEventFlag(tU32 eventFlag)
{
	ETAL_TaskClearEventFlag(ETAL_control_TaskEventHandler, eventFlag);
}

//!
//! \brief      <i><b> ETAL_control_TaskWakeUpOnEvent </b></i>
//! \details    Functions to wake-up the middlewar task on event.
//!             The main loop for the task is waked-up here.
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_control_TaskWakeUpOnEvent (tU32 event)
{
	if ((ETAL_CONTROL_EVENT_CALLBACK_START == event)
		||
		(ETAL_CONTROL_EVENT_CALLBACK_STOP == event)
		||
		(ETAL_CONTROL_EVENT_MONITOR_START == event)
		||
		(ETAL_CONTROL_EVENT_MONITOR_STOP == event))
	{
		ETAL_TaskWakeUpOnEvent(ETAL_control_TaskEventHandler, event);
	}
 }

tVoid ETAL_control_PeriodicCallbackStartEvent(tVoid)
{
	ETAL_control_TaskWakeUpOnEvent(ETAL_CONTROL_EVENT_CALLBACK_START);
}

tVoid ETAL_control_PeriodicCallbackStopEvent(tVoid)
{
	ETAL_control_TaskWakeUpOnEvent(ETAL_CONTROL_EVENT_CALLBACK_STOP);
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)

tVoid ETAL_control_PeriodicMonitorStartEvent(tVoid)
{
	ETAL_control_TaskWakeUpOnEvent(ETAL_CONTROL_EVENT_MONITOR_START);
}

tVoid ETAL_control_PeriodicMonitorStopEvent(tVoid)
{
	ETAL_control_TaskWakeUpOnEvent(ETAL_CONTROL_EVENT_MONITOR_STOP);
}

// Procedure to enable the quality polling callback 
// ie creation of timers
static tVoid ETAL_control_MonitoringCallbackEnable(tVoid)
{
	EtalBcastQualityMonitorAttr *pmonattr = NULL;
	EtalQaMonitoredEntryAttr *pmonentry = NULL;
	etalMonitorTy *pmon = NULL;
	ETAL_HANDLE hMonitor;
	tU32 vl_monitorIndex, vl_qualityMonitorAttrIndex;
	tU8	vl_CountEnabling = 0;

	// we need to parse among all monitor to check if timer activated / monitoring enabled
	//
	/*
	 * go through the list of configured monitors and check
	 * if the time delay expired for anyone
	 */
	for (vl_monitorIndex = 0; vl_monitorIndex < ETAL_MAX_MONITORS; vl_monitorIndex++)
	{
		hMonitor = ETAL_handleMakeMonitor((ETAL_HINDEX)vl_monitorIndex);
		pmon = ETAL_statusGetMonitor(hMonitor);

		// monitor needs to be defined and valid 
		if ((pmon != NULL)
			&& (TRUE == pmon->isValid))			
		{
			// monitor standard must be valid ==> continue
			if ((pmon->standard == ETAL_BCAST_STD_FM) ||
				(pmon->standard == ETAL_BCAST_STD_AM) ||
				(pmon->standard == ETAL_BCAST_STD_DAB) |
				(pmon->standard == ETAL_BCAST_STD_DRM) ||
				(ETAL_IS_HDRADIO_STANDARD(pmon->standard)))
			{
				// here monitor 
				pmonattr = &pmon->requested;

				for (vl_qualityMonitorAttrIndex = 0; vl_qualityMonitorAttrIndex < ETAL_MAX_QUALITY_PER_MONITOR; vl_qualityMonitorAttrIndex++)
				{
					pmonentry = &pmonattr->m_monitoredIndicators[vl_qualityMonitorAttrIndex];

					// the quality indicator must be defined
					if (pmonentry->m_MonitoredIndicator != EtalQualityIndicator_Undef)
					{
						// check if timer enabling is required
						if (FALSE == pmon->timer_infos.TimerIsEnabled[vl_qualityMonitorAttrIndex])
						{
							// let's enabled !!
							// create the arg for callback
							monitorsTimerInfo[vl_monitorIndex*ETAL_MAX_MONITORS+vl_qualityMonitorAttrIndex].v_monitorIndex = vl_monitorIndex;
							monitorsTimerInfo[vl_monitorIndex*ETAL_MAX_MONITORS+vl_qualityMonitorAttrIndex].v_qualityMonitorAttrIndex = vl_qualityMonitorAttrIndex;
							
							// Create a timer
							// Timer creation 
							// put the detailled element as info
							(void)OSAL_s32TimerCreate(ETAL_control_MonitorTimerCallBack,
											 (tPVoid) &(monitorsTimerInfo[vl_monitorIndex*ETAL_MAX_MONITORS+vl_qualityMonitorAttrIndex]),
											 &pmon->timer_infos.TimerId[vl_qualityMonitorAttrIndex]);
								
							// set scehduling
							(void)OSAL_s32TimerSetTime(pmon->timer_infos.TimerId[vl_qualityMonitorAttrIndex],
											pmonentry->m_UpdateFrequency, pmonentry->m_UpdateFrequency);
							
							pmon->timer_infos.TimerIsEnabled[vl_qualityMonitorAttrIndex] = TRUE;
							
							vl_CountEnabling++;
						}
						else
						{
							// timer already enabled, nothing to do new
							//
							
						}
					}
					else
					{
						// if the quality indicator is not defined ==> continue 
					}
					
				}
			}
			else
			{
				// invalid standard => skip
			}
			
			// indicate timer have been activated
			pmon->isMonitorActiveForTimers = TRUE;
		}
		else
		{
			// invalid monitor => skip
		}
	}
	
	// Manage a print for information 
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_control_MonitoringCallbackEnable: Enabling %d Timers for Periodic CB", vl_CountEnabling);
	
}

// Procedure to disable the quality polling callback 
// ie deletion of timers

static tVoid ETAL_control_MonitoringCallbackDisable(tVoid)
{
  	etalMonitorTy *pmon = NULL;
	ETAL_HANDLE hMonitor;
	tU32 vl_monitorIndex, vl_qualityMonitorAttrIndex;
	tU8	vl_CountDisabling = 0;

	// we need to parse among all monitor to check if timer activated / monitoring enabled
	//
	/*
	 * go through the list of configured monitors and check
	 * if the time delay expired for anyone
	 */
	for (vl_monitorIndex = 0; vl_monitorIndex < ETAL_MAX_MONITORS; vl_monitorIndex++)
	{
		hMonitor = ETAL_handleMakeMonitor((ETAL_HINDEX)vl_monitorIndex);
		pmon = ETAL_statusGetMonitor(hMonitor);

		// monitor needs to be defined and valid 
		if ((pmon != NULL)
			&& (TRUE == pmon->isValid)
			&& (FALSE == pmon->isMonitorActiveForTimers)) // deactivate only if requested to
		{
			// monitor standard must be valid ==> continue
			if ((pmon->standard == ETAL_BCAST_STD_FM) ||
				(pmon->standard == ETAL_BCAST_STD_AM) ||
				(pmon->standard == ETAL_BCAST_STD_DAB) |
				(pmon->standard == ETAL_BCAST_STD_DRM) ||
				(ETAL_IS_HDRADIO_STANDARD(pmon->standard)))
			{ 
				for (vl_qualityMonitorAttrIndex = 0; vl_qualityMonitorAttrIndex < ETAL_MAX_QUALITY_PER_MONITOR; vl_qualityMonitorAttrIndex++)
				{ 
					// check if timer enabling is required
					if (TRUE == pmon->timer_infos.TimerIsEnabled[vl_qualityMonitorAttrIndex])
					{
						// let's disable !!
						// set scehduling
						(void)OSAL_s32TimerSetTime(&pmon->timer_infos.TimerId[vl_qualityMonitorAttrIndex], 0, 0);							
						// delete a timer
						// put the detailled element as info
						(void)OSAL_s32TimerDelete(pmon->timer_infos.TimerId[vl_qualityMonitorAttrIndex]);
							
						pmon->timer_infos.TimerIsEnabled[vl_qualityMonitorAttrIndex] = FALSE;
						
						vl_CountDisabling++;
					}
					else
					{
						// timer already disabled, nothing to do new
						//
						
					}
				}				
			}
			else
			{
				// invalid standard => skip
			}
		}
		else
		{
			// invalid monitor => skip
		}
	}

	// Manage a print for information 
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_control_MonitoringCallbackDisable: Disabling %d Timers for Periodic CB", vl_CountDisabling);

}

// procedure which handle timer expiration 
// for a monitored information
//
static void ETAL_control_MonitorTimerCallBack(tPVoid pI_Arg)
{
	etalMonitorTimerMgtTy *pl_monitorTimerInfo = (etalMonitorTimerMgtTy *)pI_Arg;
	
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	etalQualityCbTy qual_cb;
	ETAL_HANDLE hReceiver;
	EtalBcastQualityMonitorAttr *pmonattr = NULL;
	EtalQaMonitoredEntryAttr *pmonentry = NULL;
	etalMonitorTy *pmon = NULL;
	ETAL_HANDLE hMonitor;
	ETAL_HINDEX monitor_index;
	tVoid *context;
	tBool vl_receiverIsLocked = FALSE;
	
	// we have monitor index and quality info index, just work !!
	monitor_index = (ETAL_HINDEX)pl_monitorTimerInfo->v_monitorIndex;
	hMonitor = ETAL_handleMakeMonitor(monitor_index);
	pmon = ETAL_statusGetMonitor(hMonitor);
	if (pmon == NULL)
	{
		goto exit;
	}
	if (!pmon->isValid)
	{
		goto exit;
	}
	if (pmon->isMonitorActiveForTimers != TRUE)
	{
		// not active : under reconf or destroy
		goto exit;
	}		
	if ((pmon->standard != ETAL_BCAST_STD_FM) &&
		(pmon->standard != ETAL_BCAST_STD_AM) &&
		(pmon->standard != ETAL_BCAST_STD_DAB) &&
		(pmon->standard != ETAL_BCAST_STD_DRM) &&
		(!ETAL_IS_HDRADIO_STANDARD(pmon->standard)))
	{
		goto exit;
	}
	pmonattr = &pmon->requested;
	context = pmonattr->m_Context;	

	pmonentry = &pmonattr->m_monitoredIndicators[pl_monitorTimerInfo->v_qualityMonitorAttrIndex];
	if (pmonentry->m_MonitoredIndicator == EtalQualityIndicator_Undef)
	{
		goto exit;
	}

	// lock the receiver
	hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);

	// we should lock the receiver
	if (!ETAL_handleIsReceiver(hReceiver))
	{
		ASSERT_ON_DEBUGGING(0);
		goto exit;
	}
		
	/* get receiver lock */
	if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
	{
		goto exit;
	}

	vl_receiverIsLocked = TRUE;
	
	//check receiver is still valid and has not been destroyed !
	if (false == ETAL_receiverIsValidHandle(hReceiver))
	{
		goto exit;
	}	

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	if (ETAL_isMonitoredIndicatorTypeCMOST(pmonentry->m_MonitoredIndicator))
	{
		/*
		 * read quality only once, and only if at least one monitor is configured
		 */

		if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_control_MonitorTimerCallBack : Monitoring on %d suspended", hReceiver);
		}
		else
		{
			(void)OSAL_pvMemorySet((tVoid *)&qual_cb.qual, 0x00, sizeof(EtalBcastQualityContainer));
			if (ETAL_get_reception_quality_internal(hReceiver, &qual_cb.qual) == OSAL_OK)
			{
				if (ETAL_controlCheckMonitorValue_CMOST(&qual_cb.qual, pmonentry))
				{
					qual_cb.hMonitor = hMonitor;
					qual_cb.qual.m_Context = context;
					ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_control_MonitorTimerCallBack : CMOST TYPE / ETAL_get_reception_quality_internal failed");
			}

		}
	}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	if (ETAL_isMonitoredIndicatorTypeHDRADIO(pmonentry->m_MonitoredIndicator))
	{
		/*
		 * read quality only once, and only if at least one monitor is configured
		 */
		hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);

		if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_control_MonitorTimerCallBack : Monitoring on %d suspended", hReceiver);
		}
		else
		{
			(void)OSAL_pvMemorySet((tVoid *)&qual_cb.qual, 0x00, sizeof(EtalBcastQualityContainer));
			if (ETAL_get_reception_quality_internal(hReceiver, &qual_cb.qual) == OSAL_OK)
			{
				if (ETAL_controlCheckMonitorValue_HDRADIO(&qual_cb.qual, pmonentry))
				{
					qual_cb.hMonitor = hMonitor;
					qual_cb.qual.m_Context = context;
					ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_controlPollQuality : HD TYPE / ETAL_get_reception_quality_internal failed");
			}
		}
	}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	if (ETAL_isMonitoredIndicatorTypeDAB(pmonentry->m_MonitoredIndicator))
	{
		/*
		 * read quality only once, and only if at least one monitor is configured
		 */
		hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);

		if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_controlPollQuality : Monitoring on %d suspended", hReceiver);
		}
		else
		{
			(void)OSAL_pvMemorySet((tVoid *)&qual_cb.qual, 0x00, sizeof(EtalBcastQualityContainer));

			if (ETAL_get_reception_quality_internal(hReceiver, &qual_cb.qual) == OSAL_OK)
			{
				if (ETAL_controlCheckMonitorValue_DAB(&qual_cb.qual, pmonentry))
				{
					qual_cb.hMonitor = hMonitor;
					qual_cb.qual.m_Context = context;
					ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_controlPollQuality : DAB TYPE / ETAL_get_reception_quality_internal failed");
			}
		}
	}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR


exit:
	if (TRUE == vl_receiverIsLocked)
	{
		// release the lock
		ETAL_receiverReleaseLock(hReceiver);
	}

#endif // (CONFIG_ETAL_SUPPORT_CMOST_STAR) || (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

	return;
	
}
#endif // #if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)

#endif // CONFIG_ETAL_CPU_IMPROVEMENT


#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_controlCheckMonitorValue_CMOST
 *
 **************************/
/*!
 * \brief		Checks if a given quality value is within the programmed monitor bounds
 * \details		The function checks if the quality indicator contained in the *qual* parameter
 * 				is configured in the *mon* monitor and if so checks if it is within the bounds
 * 				there specified. The function operates on AM/FM quality indicators.
 * \param[in]	qual - the measured quality
 * \param[in]	mon  - a monitor
 * \return		TRUE  - *qual* is within the *mon* bounds or the bounds were not configured
 * \return		FALSE - *qual* is not within bounds
 * \callgraph
 * \callergraph
 * \todo		The BB Field Strength is not considered in the monitor
 */
static tBool ETAL_controlCheckMonitorValue_CMOST(EtalBcastQualityContainer *qual, EtalQaMonitoredEntryAttr *mon)
{
    tS32 value;
	tBool min_check, max_check;
	EtalFmQualityEntries *fm_qual;
	tBool ret = TRUE;

	if (ETAL_IS_HDRADIO_STANDARD(qual->m_standard))
	{
		fm_qual = &qual->EtalQualityEntries.hd.m_analogQualityEntries;
	}
	else
	{
		fm_qual = &qual->EtalQualityEntries.amfm;
	}

	min_check = TRUE;
	max_check = TRUE;
	switch (mon->m_MonitoredIndicator)
	{
		case EtalQualityIndicator_FmFieldStrength:
		    value = fm_qual->m_RFFieldStrength;
			break;

		case EtalQualityIndicator_FmFrequencyOffset:
		    value = (tS32)fm_qual->m_FrequencyOffset;
			break;

		case EtalQualityIndicator_FmModulationDetector:
		    value = (tS32)fm_qual->m_ModulationDetector;
			break;

		case EtalQualityIndicator_FmMultipath:
		    value = (tS32)fm_qual->m_Multipath;
			break;

		case EtalQualityIndicator_FmUltrasonicNoise:
		    value = (tS32)fm_qual->m_UltrasonicNoise;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			ret = FALSE;
	}
	
	if(ret != FALSE)
	{
		if ((mon->m_InferiorValue != ETAL_INVALID_MONITOR) && (value <= mon->m_InferiorValue))
		{
			min_check = FALSE;
		}
		else if ((mon->m_SuperiorValue != ETAL_INVALID_MONITOR) && (value >= mon->m_SuperiorValue))
		{
			max_check = FALSE;
		}
		else
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "CMOST quality monitoring triggered by %d", mon->m_MonitoredIndicator);
		}
		ret = min_check && max_check;
	}
	return ret;
}

/***************************
 *
 * ETAL_isMonitoredIndicatorTypeCMOST
 *
 **************************/
/*!
 * \brief		Checks if a quality indicator is managed by the CMOST	
 * \param[in]	ind - the quality indicator to check	
 * \return		TRUE if the quality indicator is managed by the CMOST
 * \callgraph
 * \callergraph
 */
static tBool ETAL_isMonitoredIndicatorTypeCMOST(EtalBcastQaIndicators ind)
{
	tBool ret = FALSE;

	switch (ind)
	{
		case EtalQualityIndicator_FmFieldStrength:
		case EtalQualityIndicator_FmFrequencyOffset:
		case EtalQualityIndicator_FmModulationDetector:
		case EtalQualityIndicator_FmMultipath:
		case EtalQualityIndicator_FmUltrasonicNoise:
			ret = TRUE;
			break;

		default:
			break;
	}
	return ret;
}
#endif  // CONFIG_ETAL_SUPPORT_CMOST_STAR

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
/***************************
 *
 * ETAL_controlCheckMonitorValue_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if a given quality value is within the programmed monitor bounds
 * \details		The function checks if the quality indicator contained in the *qual* parameter
 * 				is configured in the *mon* monitor and if so checks if it is within the bounds
 * 				there specified. The function operates on HDRadio quality indicators.
 * \param[in]	qual - the measured quality
 * \param[in]	mon  - a monitor
 * \return		TRUE  - *qual* is within the *mon* bounds or the bounds were not configured
 * \return		FALSE - *qual* is not within bounds
 * \callgraph
 * \callergraph
 */
static tBool ETAL_controlCheckMonitorValue_HDRADIO(EtalBcastQualityContainer *qual, EtalQaMonitoredEntryAttr *mon)
{
	tS32 value;
	tBool min_check, max_check;
	tBool ret = FALSE;

	min_check = TRUE;
	max_check = TRUE;
	switch (mon->m_MonitoredIndicator)
	{
		case EtalQualityIndicator_HdQI:
			value = (tS32)qual->EtalQualityEntries.hd.m_QI;
			break;

		case EtalQualityIndicator_HdCdToNo:
			value = (tS32)qual->EtalQualityEntries.hd.m_CdToNo;
			break;

		case EtalQualityIndicator_HdDSQM:
			value = (tS32)qual->EtalQualityEntries.hd.m_DSQM;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			ret = FALSE;
			goto exit;
	}

	if ((mon->m_InferiorValue != ETAL_INVALID_MONITOR) && (value <= mon->m_InferiorValue))
	{
		min_check = FALSE;
	}
	else if ((mon->m_SuperiorValue != ETAL_INVALID_MONITOR) && (value >= mon->m_SuperiorValue))
	{
		max_check = FALSE;
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "HD quality monitoring triggered by %d", mon->m_MonitoredIndicator);
	}
	ret = min_check && max_check;

exit:
	return ret;
}

/***************************
 *
 * ETAL_isMonitoredIndicatorTypeHDRADIO
 *
 **************************/
/*!
 * \brief		Checks if a quality indicator is managed by the HDRadio	
 * \param[in]	ind - the quality indicator to check	
 * \return		TRUE if the quality indicator is managed by the HDRadio
 * \callgraph
 * \callergraph
 */
static tBool ETAL_isMonitoredIndicatorTypeHDRADIO(EtalBcastQaIndicators ind)
{
	tBool ret = FALSE;
	
	switch (ind)
	{
		case EtalQualityIndicator_HdQI:
		case EtalQualityIndicator_HdCdToNo:
		case EtalQualityIndicator_HdDSQM:
			ret = TRUE;
			break;

		default:
			break;
	}

	return ret;
}
#endif  // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_controlCheckMonitorValue_DAB
 *
 **************************/
/*!
 * \brief		Checks if a given quality value is within the programmed monitor bounds
 * \details		The function checks if the quality indicator contained in the *qual* parameter
 * 				is configured in the *mon* monitor and if so checks if it is within the bounds
 * 				there specified. The function operates on DAB quality indicators.
 * \param[in]	qual - the measured quality
 * \param[in]	mon  - a monitor
 * \return		TRUE  - *qual* is within the *mon* bounds or the bounds were not configured
 * \return		FALSE - *qual* is not within bounds
 * \callgraph
 * \callergraph
 */
static tBool ETAL_controlCheckMonitorValue_DAB(EtalBcastQualityContainer *qual, EtalQaMonitoredEntryAttr *mon)
{
	tS32 value;
	tBool min_check, max_check;
	tBool ret = FALSE;

	min_check = TRUE;
	max_check = TRUE;
	switch (mon->m_MonitoredIndicator)
	{
		case EtalQualityIndicator_DabFicErrorRatio:
			if(qual->EtalQualityEntries.dab.m_isValidFicBitErrorRatio == TRUE)
			{
				value = (tS32)qual->EtalQualityEntries.dab.m_FicBitErrorRatio;
			}
			else
			{
				goto exit;
			}
			break;

		case EtalQualityIndicator_DabFieldStrength:
			value = (tS32)qual->EtalQualityEntries.dab.m_RFFieldStrength;
			break;

		case EtalQualityIndicator_DabMscBer:
			if(qual->EtalQualityEntries.dab.m_isValidMscBitErrorRatio == TRUE)
			{
				value = (tS32)qual->EtalQualityEntries.dab.m_MscBitErrorRatio;
			}
			else
			{
				goto exit;
			}
			break;

		case EtalQualityIndicator_DabAudioBerLevel:
			value = (tS32)qual->EtalQualityEntries.dab.m_audioBitErrorRatioLevel;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
			goto exit;
	}

	if ((mon->m_InferiorValue != ETAL_INVALID_MONITOR) && (value <= mon->m_InferiorValue))
	{
		min_check = FALSE;
	}
	else if ((mon->m_SuperiorValue != ETAL_INVALID_MONITOR) && (value >= mon->m_SuperiorValue))
	{
		max_check = FALSE;
	}
	else
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "DAB quality monitoring triggered by %d", mon->m_MonitoredIndicator);
	}
	ret = min_check && max_check;

exit:
	return ret;
}

/***************************
 *
 * ETAL_isMonitoredIndicatorTypeDAB
 *
 **************************/
/*!
 * \brief		Checks if a quality indicator is managed by the DAB
 * \param[in]	ind - the quality indicator to check
 * \return		TRUE if the quality indicator is managed by the DAB
 * \callgraph
 * \callergraph
 */
static tBool ETAL_isMonitoredIndicatorTypeDAB(EtalBcastQaIndicators ind)
{
	tBool ret = FALSE;

	switch (ind)
	{
		case EtalQualityIndicator_DabFicErrorRatio:
		case EtalQualityIndicator_DabFieldStrength:
		case EtalQualityIndicator_DabMscBer:
		case EtalQualityIndicator_DabAudioBerLevel:
			ret = TRUE;
			break;

		default:
			break;
	}

	return ret;
}
#endif
/***************************
 *
 * ETAL_controlPollQuality
 *
 **************************/
/*!
 * \brief		Processes the currently configured quality monitors
 * \details		Each quality monitor in the ETAL system has its own invocation frequency.
 * 				The function goes through all the system's monitors and for those that
 * 				need to be updated reads the device (CMOST or HDRadio) for the current
 * 				quality; if the quality is within the monitor's configured bounds it 
 * 				invokes the quality callback for that monitor.
 * \remark		The function processes only AM/FM and HDRadio monitors; DAB is treated
 * 				differently since the DAB DCOP is capable of managing directly the
 * 				quality monitors.
 * \see			ETAL_controlCheckMonitorValue_CMOST, ETAL_controlCheckMonitorValue_HDRADIO
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_controlPollQuality(tVoid)
{
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	etalQualityCbTy qual_cb;
	ETAL_HANDLE hReceiver;
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	tBool got_quality_cmost = FALSE;
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	tBool got_quality_hd = FALSE;
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	tBool got_quality_dab = FALSE;
#endif
	EtalBcastQualityMonitorAttr *pmonattr = NULL;
	EtalQaMonitoredEntryAttr *pmonentry = NULL;
	etalMonitorTy *pmon = NULL;
	ETAL_HANDLE hMonitor;
	ETAL_HINDEX monitor_index;
	tU32 i, j;
	OSAL_tMSecond now;
	tVoid *context;

	if (ETAL_statusGetCountMonitor() == 0)
	{
		goto exit;
	}

	now = OSAL_ClockGetElapsedTime();

	/*
	 * go through the list of configured monitors and check
	 * if the time delay expired for anyone
	 */
	for (i = 0; i < ETAL_MAX_MONITORS; i++)
	{
		monitor_index = (ETAL_HINDEX)i;
		hMonitor = ETAL_handleMakeMonitor(monitor_index);
		pmon = ETAL_statusGetMonitor(hMonitor);
		if (pmon == NULL)
		{
			continue;
		}
		if (!pmon->isValid)
		{
			continue;
		}
		if ((pmon->standard != ETAL_BCAST_STD_FM) &&
			(pmon->standard != ETAL_BCAST_STD_AM) &&
			(pmon->standard != ETAL_BCAST_STD_DAB) &&
			(pmon->standard != ETAL_BCAST_STD_DRM) &&
			(!ETAL_IS_HDRADIO_STANDARD(pmon->standard)))
		{
			continue;
		}
		pmonattr = &pmon->requested;
		context = pmonattr->m_Context;
		for (j = 0; j < ETAL_MAX_QUALITY_PER_MONITOR; j++)
		{
			pmonentry = &pmonattr->m_monitoredIndicators[j];
			if (pmonentry->m_MonitoredIndicator == EtalQualityIndicator_Undef)
			{
				continue;
			}

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
			if (ETAL_isMonitoredIndicatorTypeCMOST(pmonentry->m_MonitoredIndicator))
			{
				if (pmon->monitorConfig.STARHD.nextExpiration[j] > now)
				{
					continue;
				}
				/*
				 * delay expired, reload the nextExpiration, read the quality from the
				 * device and compare with the requested monitor
				 */
				pmon->monitorConfig.STARHD.nextExpiration[j] = now + pmonentry->m_UpdateFrequency;

				if (!got_quality_cmost)
				{
					/*
					 * read quality only once, and only if at least one monitor is configured
					 */
					hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);

					// we should lock the receiver
					if (!ETAL_handleIsReceiver(hReceiver))
					{
						ASSERT_ON_DEBUGGING(0);
						continue;
					}

					/* get receiver lock */
					if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
					{
						continue;
					}

					//check receiver is still valid and has not been destroyed !
					if (false == ETAL_receiverIsValidHandle(hReceiver))
					{
						ETAL_receiverReleaseLock(hReceiver);
						continue;
					}

					if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_controlPollQuality : Monitoring on %d suspended", hReceiver);
					}
					else
					{
						(void)OSAL_pvMemorySet((tVoid *)&qual_cb.qual, 0x00, sizeof(EtalBcastQualityContainer));
						if (ETAL_get_reception_quality_internal(hReceiver, &qual_cb.qual) == OSAL_OK)
						{
							if (ETAL_controlCheckMonitorValue_CMOST(&qual_cb.qual, pmonentry))
							{
								qual_cb.hMonitor = hMonitor;
								qual_cb.qual.m_Context = context;
								ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
							}
							got_quality_cmost = TRUE;
						}
						else
						{
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_controlPollQuality : CMOST TYPE / ETAL_get_reception_quality_internal failed");
						}
					}

					// release the receiver lock
					ETAL_receiverReleaseLock(hReceiver);

				}
				else
				{
					if (ETAL_controlCheckMonitorValue_CMOST(&qual_cb.qual, pmonentry))
					{
						qual_cb.hMonitor = hMonitor;
						qual_cb.qual.m_Context = context;
						ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
					}
				}
			}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
			if (ETAL_isMonitoredIndicatorTypeHDRADIO(pmonentry->m_MonitoredIndicator))
			{
				if (pmon->monitorConfig.STARHD.nextExpiration[j] > now)
				{
					continue;
				}
				/*
				 * delay expired, reload the nextExpiration, read the quality from the
				 * device and compare with the requested monitor
				 */
				pmon->monitorConfig.STARHD.nextExpiration[j] = now + pmonentry->m_UpdateFrequency;

				if (!got_quality_hd)
				{
					/*
					 * read quality only once, and only if at least one monitor is configured
					 */
					hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);
					// we should lock the receiver
					if (!ETAL_handleIsReceiver(hReceiver))
					{
						ASSERT_ON_DEBUGGING(0);
						continue;
					}

					/* get receiver lock */
					if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
					{
						continue;
					}

					//check receiver is still valid and has not been destroyed !
					if (false == ETAL_receiverIsValidHandle(hReceiver))
					{
						ETAL_receiverReleaseLock(hReceiver);
						continue;
					}

					if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_controlPollQuality : Monitoring on %d suspended", hReceiver);
					}
					else
					{
						(void)OSAL_pvMemorySet((tVoid *)&qual_cb.qual, 0x00, sizeof(EtalBcastQualityContainer));
						if (ETAL_get_reception_quality_internal(hReceiver, &qual_cb.qual) == OSAL_OK)
						{
							if (ETAL_controlCheckMonitorValue_HDRADIO(&qual_cb.qual, pmonentry))
							{
								qual_cb.hMonitor = hMonitor;
								qual_cb.qual.m_Context = context;
								ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
							}
							got_quality_hd = TRUE;
						}
						else
						{
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_controlPollQuality : HD TYPE / ETAL_get_reception_quality_internal failed");
						}
					}

					// release the receiver lock
					ETAL_receiverReleaseLock(hReceiver);
				}
				else
				{
					if (ETAL_controlCheckMonitorValue_HDRADIO(&qual_cb.qual, pmonentry))
					{
						qual_cb.hMonitor = hMonitor;
						qual_cb.qual.m_Context = context;
						ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
					}
				}
			}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
			if (ETAL_isMonitoredIndicatorTypeDAB(pmonentry->m_MonitoredIndicator))
			{
				if (pmon->monitorConfig.MDR.nextExpiration[j] > now)
				{
					continue;
				}
				/*
				 * delay expired, reload the nextExpiration, read the quality from the
				 * device and compare with the requested monitor
				 */
				pmon->monitorConfig.MDR.nextExpiration[j] = now + pmonentry->m_UpdateFrequency;

				if (!got_quality_dab)
				{
					/*
					 * read quality only once, and only if at least one monitor is configured
					 */
					hReceiver = ETAL_statusGetReceiverFromMonitor(pmon);

					// we should lock the receiver
					if (!ETAL_handleIsReceiver(hReceiver))
					{
						ASSERT_ON_DEBUGGING(0);
						continue;
					}

					/* get receiver lock */
					if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
					{
						continue;
					}

					//check receiver is still valid and has not been destroyed !
					if (false == ETAL_receiverIsValidHandle(hReceiver))
					{
						ETAL_receiverReleaseLock(hReceiver);
						continue;
					}

					if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_controlPollQuality : Monitoring on %d suspended", hReceiver);
					}
					else
					{
						(void)OSAL_pvMemorySet((tVoid *)&qual_cb.qual, 0x00, sizeof(EtalBcastQualityContainer));
						if (ETAL_get_reception_quality_internal(hReceiver, &qual_cb.qual) == OSAL_OK)
						{
							if (ETAL_controlCheckMonitorValue_DAB(&qual_cb.qual, pmonentry))
							{
								qual_cb.hMonitor = hMonitor;
								qual_cb.qual.m_Context = context;
								ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
							}
							got_quality_dab = TRUE;
						}
						else
						{
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_controlPollQuality : DAB TYPE / ETAL_get_reception_quality_internal failed");
						}
					}

					// release the receiver lock
					ETAL_receiverReleaseLock(hReceiver);
				}
				else
				{
					if (ETAL_controlCheckMonitorValue_DAB(&qual_cb.qual, pmonentry))
					{
						qual_cb.hMonitor = hMonitor;
						qual_cb.qual.m_Context = context;
						ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality, ETAL_INFO_UNDEF, (tVoid *)&qual_cb, sizeof(etalQualityCbTy));
					}
				}
			}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
		}
	}
#endif // (CONFIG_ETAL_SUPPORT_CMOST_STAR) || (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

exit:
	return;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_controlInit
 *
 **************************/
/*!
 * \brief		Initializes the control task
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_controlInit(tVoid)
{
	tSInt ret = OSAL_OK;
	
	// create the event handler
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	// Init globals
	ETAL_control_TaskEventHandler = (OSAL_tEventHandle)0;
	OSAL_s32EventCreate ((tCString)ETAL_EVENT_HANDLER_CONTROL, &ETAL_control_TaskEventHandler);	
#endif //  CONFIG_ETAL_CPU_IMPROVEMENT

#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	// Init globals
	ETAL_control_IrqRds_TaskEventHandler = (OSAL_tEventHandle)0;
	OSAL_s32EventCreate ((tCString)ETAL_EVENT_HANDLER_CONTROL_RDS_IRQ, &ETAL_control_IrqRds_TaskEventHandler);	
#endif

	return ret;
}

/***************************
 *
 * ETAL_controlDeinit
 *
 **************************/
/*!
 * \brief		De-initializes the control deinit
 * \details		Mainly destroys the events created.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_controlDeinit(tVoid)
{
	tSInt ret = OSAL_OK;

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT

#ifndef OSAL_EVENT_SKIP_NAMES
	if (OSAL_s32EventClose(ETAL_control_TaskEventHandler) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	goto exit;
	}
			
	OSAL_s32ThreadWait(100);
		
	if (OSAL_s32EventDelete(ETAL_EVENT_HANDLER_CONTROL) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
#else
	if (OSAL_s32EventFree(ETAL_control_TaskEventHandler) != OSAL_OK)
	{
				ret = OSAL_ERROR;
				goto exit;
	}
#endif
		
	ETAL_control_TaskEventHandler = (OSAL_tEventHandle)0;


#endif // CONFIG_ETAL_CPU_IMPROVEMENT

#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)

#ifndef OSAL_EVENT_SKIP_NAMES
	if (OSAL_s32EventClose(ETAL_control_IrqRds_TaskEventHandler) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	goto exit;
	}
			
	OSAL_s32ThreadWait(100);
		
	if (OSAL_s32EventDelete(ETAL_EVENT_HANDLER_CONTROL_RDS_IRQ) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
#else
	if (OSAL_s32EventFree(ETAL_control_IrqRds_TaskEventHandler) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
#endif
		
	ETAL_control_IrqRds_TaskEventHandler = (OSAL_tEventHandle)0;

#endif // CONFIG_COMM_ENABLE_RDS_IRQ

#if defined (CONFIG_ETAL_CPU_IMPROVEMENT) || defined(CONFIG_COMM_ENABLE_RDS_IRQ)
exit:
#endif

	return ret;

}

/***************************
 *
 * ETAL_Control_ThreadEntry
 *
 **************************/
/*!
 * \brief		The ETAL Control thread entry point
 * \details		This thread is in charge of executing the operations that need
 * 				to run in the background (i.e. without direct API user intervention).
 * 				The operations performed are:
 * 				- run the quality monitors and invoke the callbacks if necessary
 * 				- run the periodic internal callbacks
 * 				- run receiver-specific background operations
 *
 * \param[in]	dummy - unused parameter
 * \see			ETAL_controlPollQuality, ETAL_intCbSchedulePeriodicCallbacks, ETAL_controlTuneRequest
 * \todo		Periodic callbacks are invoked with global ETAL lock; while it is necessary
 * 				to protect the state during #ETAL_intCbSchedulePeriodicCallbacks, it should
 * 				be probably fine to release it just before invoking the callback
 * \callgraph
 * \callergraph
 */
#ifdef CONFIG_HOST_OS_TKERNEL
tVoid ETAL_Control_ThreadEntry(tSInt stacd, tPVoid dummy)
#else
tVoid ETAL_Control_ThreadEntry(tPVoid dummy)
#endif
{
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	tSInt vl_res;
	OSAL_tEventMask vl_resEvent = ETAL_CONTROL_NO_EVENT_FLAG;
#endif //CONFIG_ETAL_CPU_IMPROVEMENT

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	while (TRUE)
	{	
		vl_res = OSAL_s32EventWait (ETAL_control_TaskEventHandler,
											 ETAL_CONTROL_EVENT_WAIT_ALL, 
											 OSAL_EN_EVENTMASK_OR, 
											 ETAL_CONTROL_EVENT_WAIT_TIME_MS,
											 &vl_resEvent);
			
			
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "ETAL_Control_ThreadEntry: scheduling event vl_res = %d, vl_resEvent = 0x%x",
							vl_res, vl_resEvent);

		if (OSAL_ERROR == vl_res)
		{
			// Event wait failure ==> break;
			
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_Control_ThreadEntry: wait error");
			break;
		}
		else if ((vl_resEvent == ETAL_CONTROL_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
		{
			// this is a timeout : no even received
			// trigger the SF task normal processing

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)

			/*
			 * System wide actions, lock the system
			 */
			ETAL_statusGetInternalLock();

			ETAL_controlPollQuality();

			// Release the system lock
			ETAL_statusReleaseInternalLock();
#endif
		}
		else if (OSAL_OK == vl_res)
		{
			// This is a ETAL_CONTROL_EVENT_CALLBACK_START_FLAG call the  event handler
			if (ETAL_CONTROL_EVENT_CALLBACK_START_FLAG == (ETAL_CONTROL_EVENT_CALLBACK_START_FLAG & vl_resEvent))
			{			
				// clear event now that it will be processed
				ETAL_control_TaskClearEventFlag(ETAL_CONTROL_EVENT_CALLBACK_START_FLAG);
				// process
				ETAL_intCb_PeriodicCallbackEnable();
			}
			if (ETAL_CONTROL_EVENT_CALLBACK_STOP_FLAG == (ETAL_CONTROL_EVENT_CALLBACK_STOP_FLAG & vl_resEvent))
			{
				// clear event now that it will be processed
				ETAL_control_TaskClearEventFlag(ETAL_CONTROL_EVENT_CALLBACK_STOP_FLAG);
				// process
				ETAL_intCb_PeriodicCallbackDisable();
			}
			if (ETAL_CONTROL_EVENT_MONITOR_START_FLAG == (ETAL_CONTROL_EVENT_MONITOR_START_FLAG & vl_resEvent))
			{
				// clear event now that it will be processed
				ETAL_control_TaskClearEventFlag(ETAL_CONTROL_EVENT_MONITOR_START_FLAG);
				// process			
				ETAL_control_MonitoringCallbackEnable();
			}
			if (ETAL_CONTROL_EVENT_MONITOR_STOP_FLAG == (ETAL_CONTROL_EVENT_MONITOR_STOP_FLAG & vl_resEvent))
			{
				// clear event now that it will be processed
				ETAL_control_TaskClearEventFlag(ETAL_CONTROL_EVENT_MONITOR_STOP_FLAG);
				// process	
				ETAL_control_MonitoringCallbackDisable();
			}
		}
		else
		{
			// should not come here all processed before
		}
		
	}
#else	// CONFIG_ETAL_CPU_IMPROVEMENT

	while (TRUE)
	{

		/*
		 * System wide actions, lock the system
		 */
		ETAL_statusGetInternalLock();
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
		ETAL_controlPollQuality();
#endif
		/*
		 * WARNING: one function inside ETAL_intCbSchedulePeriodicCallbacks also locks the receiver
		 * TODO: see function header
		 */
		ETAL_intCbSchedulePeriodicCallbacks();

		ETAL_statusReleaseInternalLock();
		(void)OSAL_s32ThreadWait(ETAL_CONTROL_THREAD_SCHEDULING);
	}
#endif // CONFIG_ETAL_CPU_IMPROVEMENT

}

#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)

//!
//! \brief      <i><b> tVoid ETAL_control_IrqRds_TaskClearEventFlag (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_control_IrqRds_TaskClearEventFlag(tU32 eventFlag)
{
	ETAL_TaskClearEventFlag(ETAL_control_IrqRds_TaskEventHandler, eventFlag);
}

//!
//! \brief      <i><b> ETAL_control_IrqRds_TaskWakeUpOnEvent </b></i>
//! \details    Functions to wake-up the middlewar task on event.
//!             The main loop for the task is waked-up here.
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_control_IrqRds_TaskWakeUpOnEvent (tU32 event)
{
	if (ETAL_CONTROL_EVENT_IRQ_RDS_START == event)
	{
		ETAL_TaskWakeUpOnEvent(ETAL_control_IrqRds_TaskEventHandler, event);
	}
 }

tVoid ETAL_control_IrqRds_StartEvent(tVoid)
{
	ETAL_control_IrqRds_TaskWakeUpOnEvent(ETAL_CONTROL_EVENT_IRQ_RDS_START);
}


/***************************
 *
 * ETAL_IRQ_ThreadEntry
 *
 **************************/
/*!
 * \brief		The ETAL IRQ thread entry point
 * \details		When the RDS decoder is activated in the CMOST, the CMOST
 * 				fills an internal FIFO with the received Raw RDS data; the
 * 				CMOST user can either poll and empty the FIFO periodically,
 * 				or request the CMOST to activate an (GPIO) pin once the
 * 				FIFO reaches some fullness level. In the latter case ETAL
 * 				creates this thread that sleeps until woken by the interrupt,
 * 				then reads the Raw RDS data from the CMOST.
 * \remark		This thread is present only if ETAL was built with support
 * 				for RDS IRQ.
 * \param[in]	dummy - unused parameter
 * \see			ETAL_RDSRawPeriodicFunc
 * \callgraph
 * \callergraph
 */
#ifdef CONFIG_HOST_OS_TKERNEL
tVoid ETAL_IRQ_ThreadEntry(tSInt stacd, tPVoid dummy)
{
   	
	ETAL_HANDLE hTuner;
	hTuner = ETAL_handleMakeTuner(0);
	
 	if (OSAL_s32SemaphoreCreate("IRQ_RDSSem", &ETAL_CONTROL_IRQ_Sem, 0) == OSAL_ERROR)
	{
		goto exit;
	}

	while (1)
	{
	 	if (OSAL_s32SemaphoreWait (ETAL_CONTROL_IRQ_Sem,OSAL_C_TIMEOUT_FOREVER) == OSAL_OK)
	 	{
	 		
			//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_IRQ_ThreadEntry: awaked");
	 		ETAL_RDSRawPeriodicFunc(hTuner);
	 	}
		else
		{
			//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_IRQ_ThreadEntry: ERROR");
		}
	}

exit:
	return;

}
#else
tVoid ETAL_IRQ_ThreadEntry (tPVoid dummy)
{

	tSInt vl_res;
	OSAL_tEventMask vl_resEvent = ETAL_CONTROL_IRQ_RDS_NO_EVENT_FLAG;

	while (TRUE)
	{	
		vl_res = OSAL_s32EventWait (ETAL_control_IrqRds_TaskEventHandler,
											 ETAL_CONTROL_EVENT_IRQ_RDS_WAIT_ALL, 
											 OSAL_EN_EVENTMASK_OR, 
											 ETAL_CONTROL_EVENT_IRQ_RDS_WAIT_TIME_MS,
											 &vl_resEvent);
			
			
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "ETAL_IRQ_ThreadEntry: scheduling event vl_res = %d, vl_resEvent = 0x%x",
							vl_res, vl_resEvent);
			
		if (OSAL_ERROR == vl_res)
		{
			// Event wait failure ==> break;
			
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_IRQ_ThreadEntry: wait error");
			break;
		}
		else if ((vl_resEvent == ETAL_CONTROL_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
		{
			// this is a timeout : no even received
		}
		else if (OSAL_OK == vl_res)
		{
			// This is a ETAL_CONTROL_EVENT_CALLBACK_START_FLAG call the  event handler
			if (ETAL_CONTROL_EVENT_IRQ_RDS_START_FLAG == (ETAL_CONTROL_EVENT_IRQ_RDS_START_FLAG & vl_resEvent))
			{			
				// clear event now that it will be processed
				ETAL_control_IrqRds_TaskClearEventFlag(ETAL_CONTROL_EVENT_IRQ_RDS_START_FLAG);
				// process
				ETAL_IRQ_Processing();
			}
			else
			{
				//unexpected event : clear it
				//
				ETAL_control_IrqRds_TaskClearEventFlag(vl_resEvent);
			}
		}
		else
		{
			// should not come here all processed before
		}
	}
	
}


static tVoid ETAL_IRQ_Processing()
{
    ETAL_HANDLE hReceiver, hTuner;

	while (1)
	{
		ETAL_statusGetInternalLock();
		if (ETAL_statusIsRDSIrqThreadActive(&hReceiver))
		{
			ETAL_statusReleaseInternalLock();
			ETAL_receiverGetTunerId(hReceiver, &hTuner);
			BSP_WaitForRDSInterrupt_CMOST(ETAL_handleTunerGetIndex(hTuner));
			ETAL_statusGetInternalLock();
            ETAL_RDSRawPeriodicFunc(hReceiver);
		}
		else
		{
			// no IRQ active, break
			//
			ETAL_statusReleaseInternalLock();
			break;
		}
		ETAL_statusReleaseInternalLock();
		//OSAL_s32ThreadWait(ETAL_IRQ_THREAD_SCHEDULING);
	}
}

#endif // HOST_OS_TKERNEL

#if defined(CONFIG_HOST_OS_TKERNEL)
tVoid ETAL_IRQ_EntryTuner1(void)
{
	(void)OSAL_s32SemaphorePost(ETAL_CONTROL_IRQ_Sem);
}

#endif

#endif // CONFIG_COMM_ENABLE_RDS_IRQ


