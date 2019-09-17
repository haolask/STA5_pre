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

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
#if defined(CONFIG_HOST_OS_TKERNEL) && defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	OSAL_tSemHandle ETAL_CONTROL_IRQ_Sem;
#endif

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
}

#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
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
tVoid ETAL_IRQ_ThreadEntry(tPVoid dummy)
{
    ETAL_HANDLE hReceiver, hTuner;

	while (1)
	{
		ETAL_statusGetInternalLock();
		if (ETAL_statusIsRDSIrqThreadActive(&hReceiver))
		{
			ETAL_statusReleaseInternalLock();
			/*
			 * On Accordo2 Linux this call returns after a timeout
			 * even if there is no IRQ from the CMOST (thus no data).
			 * In practice due to this problem this thread acts
			 * just like the RDS polling mode.
			 */
			ETAL_receiverGetTunerId(hReceiver, &hTuner);
			BSP_WaitForRDSInterrupt_CMOST(ETAL_handleTunerGetIndex(hTuner));
			ETAL_statusGetInternalLock();
            ETAL_RDSRawPeriodicFunc(hReceiver);
		}
		ETAL_statusReleaseInternalLock();

		OSAL_s32ThreadWait(ETAL_IRQ_THREAD_SCHEDULING);
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


