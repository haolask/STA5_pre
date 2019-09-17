//!
//!  \file 		etalstatus.c
//!  \brief 	<i><b> ETAL internal status management</b></i>
//!  \details   Contains device-independent functions to access the global ETAL internal status.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!  \see		etalStatusTy
//!

#include "osal.h"
#include "etalinternal.h"

#include "tunerdriver.h" // for boot_cmost.h
#include "boot_cmost.h" // for BOOT_SILICON_VERSION_

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		FIELD_IS_INITIALIZED
 * 			DABTuneStatus contains several flags that indicate the status
 * 			of Service Info, Service Label and PAD information. This flag is used
 * 			to access the bit indicating if the information is initialized.
 */
#define FIELD_IS_INITIALIZED   ((tU8)(0x01))
/*!
 * 			Access the bit indicating if the information is new.
 * \see		FIELD_IS_INITIALIZED
 */
#define FIELD_IS_NEW           ((tU8)(0x02))
/*!
 * 			TRUE if the DABTuneStatus information flag indicates new information
 * \see		FIELD_IS_INITIALIZED
 */
#define IS_NEW(_x_)            (((_x_) & FIELD_IS_NEW) == FIELD_IS_NEW)
/*!
 * 			TRUE if the DABTuneStatus information flag indicates information is present
 * \see		FIELD_IS_INITIALIZED
 */
#define IS_INITIALIZED(_x_)    (((_x_) & FIELD_IS_INITIALIZED) == FIELD_IS_INITIALIZED)

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| prototypes
|----------------------------------------------------------------*/
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
static tVoid ETAL_statusResetDataServiceInfo(tVoid);
#endif

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
/*!
 * \var		etalApiStatusSem
 * 			Semaphore taken by all API functions to avoid concurrent ETAL API requests
 */
OSAL_tSemHandle etalApiStatusSem;
/*!
 * \var		etalInternalStatusSem
 * 			Semaphore taken by internal functions to protect critical sections of
 * 			etalStatus from corruption due to concurrent accesses
 */
static OSAL_tSemHandle etalInternalStatusSem;
/*!
 * \var		etalTuneStatusSem
 * 			Semaphore used to protect only a particular subsection of #etalStatus
 * 			(DABTuneStatus) from corruption due to concurrent accesses
 * \see		ETAL_statusClearDABTuneStatus, ETAL_statusSetDABService
 */
static OSAL_tSemHandle etalTuneStatusSem;

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/*!
 * \var		etalDABSeekSem
 * 			Semaphore dedicated for DAB seek procedure
 */
OSAL_tSemHandle 		etalDABSeekSem;
#endif //#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
#endif //defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)


/*!
 * \var		etalStatus
 * 			Contains the ETAL global status.
 * 			The structure is protected by several semaphores:
 * 			- #etalInternalStatusSem as a global lock
 * 			- #etalTuneStatusSem for the DABTuneStatus field
 */
static etalStatusTy etalStatus;

/*!
 * \var		etalInitStatus
 * 			Details on the ETAL initialization procedure
 * 			Never updated after etal_initialize
 */
static EtalInitStatus etalInitStatus;

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
/***************************
 *
 * ETAL_resetMonitor
 *
 **************************/
/*!
 * \brief		Resets a Monitor's internal status
 * \param[in]	pmon - pointer to the Monitor status
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_resetMonitor(etalMonitorTy *pmon)
{
	if(pmon != NULL)
	{
		(void)OSAL_pvMemorySet((tVoid *)pmon, 0x00, sizeof(etalMonitorTy));
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_resetMonitor pmon: 0x%x", pmon);
		ASSERT_ON_DEBUGGING(0);
	}
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR || CONFIG_ETAL_SUPPORT_DCOP
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API


/***************************
 *
 * ETAL_initStatusSetState
 *
 **************************/
tVoid ETAL_initStatusSetState(EtalInitState state)
{
	tU32 i;
	ETAL_HINDEX tuner_index;
	ETAL_HANDLE hTuner;

	if (state == state_initStart)
	{
		(void)OSAL_pvMemorySet((tVoid *)&etalInitStatus, 0x00, sizeof(EtalInitStatus));

		/* initialize the expected silicon version string for all tuners in the system */
		for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			tuner_index = (ETAL_HINDEX) i;
			hTuner = ETAL_handleMakeTuner(tuner_index);
			switch (ETAL_tunerGetType(hTuner))
			{
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
				case deviceSTARS:
					OSAL_szStringCopy(etalInitStatus.m_tunerStatus[i].m_expectedSilicon, BOOT_SILICON_VERSION_STAR_S);
					break;
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
				case deviceSTART:
					OSAL_szStringCopy(etalInitStatus.m_tunerStatus[i].m_expectedSilicon, BOOT_SILICON_VERSION_STAR_T);
					break;
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
				case deviceDOTS:
					OSAL_szStringCopy(etalInitStatus.m_tunerStatus[i].m_expectedSilicon, BOOT_SILICON_VERSION_DOT_S);
					break;
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
				case deviceDOTT:
					OSAL_szStringCopy(etalInitStatus.m_tunerStatus[i].m_expectedSilicon, BOOT_SILICON_VERSION_DOT_T);
					break;
#endif
				default:
					break;
			}
		}
	}

	etalInitStatus.m_lastInitState = state;
}

/***************************
 *
 * ETAL_initStatusGetState
 *
 **************************/
EtalInitState ETAL_initStatusGetState(tVoid)
{
	return etalInitStatus.m_lastInitState;
}

/***************************
 *
 * ETAL_initStatusSetDCOPStatus
 *
 **************************/
tVoid ETAL_initStatusSetDCOPStatus(EtalDeviceStatus status)
{
	etalInitStatus.m_DCOPStatus.m_deviceStatus = status;
}

/***************************
 *
 * ETAL_initStatusGetDCOPStatus
 *
 **************************/
tVoid ETAL_initStatusGetDCOPStatus(EtalDeviceStatus *status)
{
	*status = etalInitStatus.m_DCOPStatus.m_deviceStatus;
}

/***************************
 *
 * ETAL_initStatusSetTunerStatus
 *
 **************************/
tVoid ETAL_initStatusSetTunerStatus(tU32 i, EtalDeviceStatus status)
{
	tU32 tuner_index;

	if (i >= ETAL_CAPA_MAX_TUNER)
	{
		for (tuner_index = 0; tuner_index < ETAL_CAPA_MAX_TUNER; tuner_index++)
		{
			etalInitStatus.m_tunerStatus[tuner_index].m_deviceStatus = status;
		}
	}
	else
	{
		etalInitStatus.m_tunerStatus[i].m_deviceStatus = status;
	}
}

/***************************
 *
 * ETAL_initStatusGetTunerStatus
 *
 **************************/
tSInt ETAL_initStatusGetTunerStatus(tU32 i, EtalDeviceStatus *status)
{
	tSInt retval;
	
	if (i >= ETAL_CAPA_MAX_TUNER)
	{
		retval = OSAL_ERROR;
	}
	else
	{
		*status = etalInitStatus.m_tunerStatus[i].m_deviceStatus;
		retval = OSAL_OK;
	}

	return retval;
}

/***************************
 *
 * ETAL_initStatusIsTunerStatusError
 *
 **************************/
/*!
 * \brief		Checks if a Tuner is in error state
 * \details		A Tuner is considere in error state if it is neither
 * 				deviceAvailable nor deviceUninitializedEntry.
 * 				The latter is the initialization status that is overwritten
 * 				when the first error occurs during the initialization.
 * \param		index - the Tuner index
 * \return		TRUE - the Tuner is in error state or the *index* is illegal
 * \return		FALSE- no error
 * \callgraph
 * \callergraph
 */
tBool ETAL_initStatusIsTunerStatusError(tU32 index)
{
	tBool retval = FALSE;

	if (index >= ETAL_CAPA_MAX_TUNER)
	{
		retval = TRUE;
	}
	else if ((etalInitStatus.m_tunerStatus[index].m_deviceStatus != deviceUninitializedEntry) &&
		(etalInitStatus.m_tunerStatus[index].m_deviceStatus != deviceAvailable))
	{
		retval = TRUE;
	}
	else
	{
		/* Notinhg to do */
	}
	
	return retval;
}

/***************************
 *
 * ETAL_initStatusIsTunerStatusReadyToUse
 *
 **************************/
/*!
 * \brief		Checks if a Tuner is in a ready to use state
 * \details		A Tuner is considere in error state if it is neither
 * 				deviceAvailable nor deviceUninitializedEntry.
 * 				The latter is the initialization status that is overwritten
 * 				when the first error occurs during the initialization.
 * \param		index - the Tuner index
 * \return		TRUE - the Tuner is in error state or the *index* is illegal
 * \return		FALSE- no error
 * \callgraph
 * \callergraph
 */
tBool ETAL_initStatusIsTunerStatusReadyToUse(tU32 index)
{
	tBool retval = FALSE;

	if (index >= ETAL_CAPA_MAX_TUNER)
	{
		retval = FALSE;
	}
	else if (etalInitStatus.m_tunerStatus[index].m_deviceStatus == deviceAvailable)
	{
		retval = TRUE;
	}
	else
	{
		/* Notinhg to do */
	}
	
	return retval;
}


/***************************
 *
 * ETAL_initStatusSetTunerVersion
 *
 **************************/
tVoid ETAL_initStatusSetTunerVersion(tU32 i, tChar *vers)
{
	(void)OSAL_s32NPrintFormat(etalInitStatus.m_tunerStatus[i].m_detectedSilicon, ETAL_SILICON_VERSION_MAX, vers);
}

#if defined (CONFIG_ETAL_INIT_CHECK_SILICON_VERSION)
/***************************
 *
 * ETAL_initStatusIsCompatibleTunerVersion
 *
 **************************/
tBool ETAL_initStatusIsCompatibleTunerVersion(tU32 i)
{
	return (OSAL_s32StringCompare(etalInitStatus.m_tunerStatus[i].m_detectedSilicon, etalInitStatus.m_tunerStatus[i].m_expectedSilicon) == 0);
}
#endif

/***************************
 *
 * ETAL_initStatusSetNonFatal
 *
 **************************/
tVoid ETAL_initStatusSetNonFatal(EtalNonFatalError warn)
{
	etalInitStatus.m_warningStatus |= warn;
}

/***************************
 *
 * ETAL_initStatusGet
 *
 **************************/
tVoid ETAL_initStatusGet(EtalInitStatus *dst)
{
	(void)OSAL_pvMemoryCopy((tVoid *)dst, (tPCVoid)&etalInitStatus, sizeof(EtalInitStatus));
}

/***************************
 *
 * ETAL_statusInitLock
 *
 **************************/
/*!
 * \brief		Initializes the ETAL internal locks
 * \details		Locks are used in ETAL to avoid concurrent access to shared resources which may result in
 *              corruption of the resource. Locks are implemented though OSAL semaphores.
 * \details		The function is called during the ETAL startup (from power-up) procedure.
 * \param		power_up - if set to TRUE creates all the semaphores;
 * 				           if set to FALSE assumes it is being called during an ETAL
 * 				           system reinitialization and creates all the semaphores
 * 				           except the global ETAL API Semaphore (which is needed during the
 * 				           re-initialization so nevere destroyed)
 * \return		OSAL_ERROR - error during the creation of one of the semaphores;
 * 				             this is normally a fatal error, ETAL cannot continue
 * \return		OSAL_OK    - no error
 * \see			ETAL_receiverInitLock
 * \see			ETAL_tunerInitLock
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusInitLock(tBool power_up)
{
	tSInt retval = OSAL_OK;
	
	if (power_up)
	{
		if (OSAL_s32SemaphoreCreate(ETAL_SEM_GLOBAL_EXTERNAL, &etalApiStatusSem, 1) != OSAL_OK)
		{
			retval = OSAL_ERROR;
			goto exit;
		}
	}

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
    if (OSAL_s32SemaphoreCreate(ETAL_SEM_DAB_SEEK, &etalDABSeekSem, 1) != OSAL_OK)
    {
		retval = OSAL_ERROR;
		goto exit;
    }

    if(ETAL_DABSeekTaskInit() != OSAL_OK)
    {
		retval = OSAL_ERROR;
		goto exit;
    }
#endif //#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
#endif //defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

	if ((OSAL_s32SemaphoreCreate(ETAL_SEM_GLOBAL_INTERNAL, &etalInternalStatusSem, 1) != OSAL_OK) ||
		(OSAL_s32SemaphoreCreate(ETAL_SEM_TUNE_STATUS, &etalTuneStatusSem, 1) != OSAL_OK))
	{
		retval = OSAL_ERROR;
		goto exit;
	}

	if (ETAL_receiverInitLock() != OSAL_OK)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

	if (ETAL_tunerInitLock() != OSAL_OK)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_statusDeinitLock
 *
 **************************/
/*!
 * \brief		Destroys and releases the ETAL internal locks
 * \details		Locks are used in ETAL to avoid concurrent access to shared resources which may result in
 * 				data corruption.
 * \remark		Depending on the Operating System, calling this function with parameter
 * 				*power_down* set to TRUE may be redundant (e.g. Linux releases all the
 * 				application resources when the application terminates).
 * \param[in]	power_down - if TRUE, destroys also the ETAL API semaphore;
 * 				             if FALSE, destroys all semaphores except the ETAL API semaphore
 * \return		OSAL_ERROR - error during the destruction of one of the semaphores
 * \return		OSAL_OK    - no error
 * \see			ETAL_receiverDeinitLock
 * \see			ETAL_tunerDeinitLock
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusDeinitLock(tBool power_down)
{
	tSInt ret = OSAL_OK;

	/*
	 * Close a semaphore with OSAL_s32SemaphoreClose before
	 * trying to delete it with OSAL_s32SemaphoreDelete because
	 * otherwise, at least on Linux, the call to OSAL_s32SemaphoreDelete
	 * will never return
	 */

	if (power_down)
	{
		/*
		 * etalApiStatusSem treated specially because it is used
		 * during ETAL system restart: we don't want to loose it in that
		 * case
		 */
		if (OSAL_s32SemaphoreClose(etalApiStatusSem) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
		else if (OSAL_s32SemaphoreDelete(ETAL_SEM_GLOBAL_EXTERNAL) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
		else
		{
			/* Use an invalid OSAL semaphore handle to make sure subsequent
			 * calls to ETAL_statusGetLock will fail */
			etalApiStatusSem = 0;
		}
	}

#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	if (ETAL_DABSeekTaskDeinit() == OSAL_OK)
	{
		if (OSAL_s32SemaphoreClose(etalDABSeekSem) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
		else if (OSAL_s32SemaphoreDelete(ETAL_SEM_DAB_SEEK) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
	}
	else
	{
		/* Nothing to do */
	}
#endif //#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
#endif //defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

	if (OSAL_s32SemaphoreClose(etalInternalStatusSem) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else if (OSAL_s32SemaphoreDelete(ETAL_SEM_GLOBAL_INTERNAL) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else
	{
		/* Nothing to do */
	}

	if (OSAL_s32SemaphoreClose(etalTuneStatusSem) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else if (OSAL_s32SemaphoreDelete(ETAL_SEM_TUNE_STATUS) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else
	{
		/* Nothing to do */
	}

	if (ETAL_receiverDeinitLock() != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}

	if (ETAL_tunerDeinitLock() != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}

	return ret;
}

/***************************
 *
 * ETAL_statusGetLock
 *
 **************************/
/*!
 * \brief		Takes the ETAL global lock
 * \details		Used by external APIs to avoid concurrent access to the library functions from the API user.
 * \return		OSAL_ERROR - ETAL not initialized, or invalid #etalApiStatusSem handle
 * 				             due to ETAL not initialized
 * \return		OSAL_OK    - no error
 * \see			ETAL_statusGetInternalLock
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusGetLock(tVoid)
{
	tSInt retval;
	if (etalApiStatusSem == 0) /* invalid semaphore handle */
	{
		/* Semaphore not yet initialized means ETAL not initialized */
		retval = OSAL_ERROR;
	}
	else if (OSAL_s32SemaphoreWait(etalApiStatusSem, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		/* May happen in case ETAL was not initialized and etalApiStatusSem not
		 * set to invalid handle */
		retval = OSAL_ERROR;
	}
	else if (!etalStatus.isInitialized)
	{
		(void)OSAL_s32SemaphorePost(etalApiStatusSem);
		retval = OSAL_ERROR;
	}
	else
	{
		ETAL_statusGetInternalLock();
		retval = OSAL_OK;
	}
	return retval;
}

/***************************
 *
 * ETAL_statusReleaseLock
 *
 **************************/
/*!
 * \brief		Releases the ETAL global lock
 * \details		Used by external APIs to avoid concurrent access to the library functions from the API user.
 * \see			ETAL_statusReleaseInternalLock
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusReleaseLock(tVoid)
{
	ETAL_statusReleaseInternalLock();
	(void)OSAL_s32SemaphorePost(etalApiStatusSem);
}

/***************************
 *
 * ETAL_statusGetInternalLock
 *
 **************************/
/*!
 * \brief		Takes the ETAL global internal lock
 * \details		Used by internal functions to protect global status from corruption due to concurrent accesses.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusGetInternalLock(tVoid)
{
	(void)OSAL_s32SemaphoreWait(etalInternalStatusSem, OSAL_C_TIMEOUT_FOREVER);
}

/***************************
 *
 * ETAL_statusReleaseInternalLock
 *
 **************************/
/*!
 * \brief		Releases the ETAL global internal lock
 * \details		Used by internal functions to protect global status from corruption due to concurrent accesses.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusReleaseInternalLock(tVoid)
{
	(void)OSAL_s32SemaphorePost(etalInternalStatusSem);
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * ETAL_statusGetTuneStatusLock
 *
 **************************/
/*!
 * \brief		Locks the DAB Tune status sub-structure of the ETAL Status
 * \details		This function can be called when there is need to update
 * 				only the DAB Tune-related fields, without locking the
 * 				whole ETAL Status.
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_statusGetTuneStatusLock(tVoid)
{
	OSAL_s32SemaphoreWait(etalTuneStatusSem, OSAL_C_TIMEOUT_FOREVER);
}

/***************************
 *
 * ETAL_statusReleaseTuneStatusLock
 *
 **************************/
/*!
 * \brief		Unlocks the DAB Tune status sub-structure of the ETAL Status
 * \see			etalDABTuneStatusTy, ETAL_statusGetTuneStatusLock
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_statusReleaseTuneStatusLock(tVoid)
{
	OSAL_s32SemaphorePost(etalTuneStatusSem);
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/***************************
 *
 * ETAL_statusGetReceiverLockFromDatapath
 *
 **************************/
/*!
 * \brief		Takes a Receiver lock given a Datapath
 * \details		Looks up the Receiver associated with the passed Datapath and locks it.
 * \param[in]	hDatapath - handle of the Datapath from which to extract the Receiver handle
 * \param[out]	hReceiver - pointer to location where the function stores the handle of the Receiver
 * 				            associated to the Datapath. The function makes no assumptions on how the caller
 * 				            uses this value.
 * \return		ETAL_RET_SUCCESS         - no error, the value in hReceiver is valid
 * \return		ETAL_RET_NOT_INITIALIZED - unable to take the global ETAL status lock
 * \return		ETAL_RET_INVALID_HANDLE  - *hDatapath* is not recognized as a Datapath handle
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_statusGetReceiverLockFromDatapath(ETAL_HANDLE hDatapath, ETAL_HANDLE *hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	/*
	 * Some receiver-exclusive APIs only have a Datapath handle as parameter
	 * To get the receiver handle we need to access the global status
	 * so we temporarily lock the system
	 */
	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_receiverDatapathIsValid(hDatapath))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else
		{
			*hReceiver = ETAL_receiverGetFromDatapath(hDatapath);
			/*
			 * now that we know the receiver swap the global lock for the receiver lock
			 */
			ret = ETAL_receiverGetLock(*hReceiver);
		}
		ETAL_statusReleaseLock();
	}
	return ret;
}

/***************************
 *
 * ETAL_statusReset
 *
 **************************/
/*!
 * \brief		Resets the ETAL Status
 * \remark		This function resets etalStatus to not initialized
 * 				so any subsequent ETAL API call is bound to fail!
 * \remark		To be used only at system startup.
 * \see			etalStatusTy
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_statusReset(tVoid)
{
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
    ETAL_HINDEX receiver_index;
#endif

	(void)OSAL_pvMemorySet((tVoid *)&etalStatus, 0x00, sizeof(etalStatusTy));
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
    for(receiver_index = 0; receiver_index < ETAL_MAX_RECEIVERS; receiver_index++)
    {
	    etalStatus.hReceiverRDSIRQ[receiver_index] = ETAL_INVALID_HANDLE;
    }
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	etalStatus.DABTuneStatus.UEId = ETAL_INVALID_UEID;
	etalStatus.DABTuneStatus.Service = ETAL_INVALID_SID;
	ETAL_statusResetDataServiceInfo();
#endif
	ETAL_tunerInitRDSSlot();
}

/***************************
 *
 * ETAL_statusHardwareAttrInit
 *
 **************************/
/*!
 * \brief		Initializes the Hardware attributes for ETAL
 * \param[in]	hardware_attr - the hardware attributes to use
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusHardwareAttrInit(const EtalHardwareAttr *hardware_attr)
{
	(void)OSAL_pvMemoryCopy((tVoid *)&etalStatus.hardwareAttr, (tPCVoid)hardware_attr, sizeof(EtalHardwareAttr));
}

/***************************
 *
 * ETAL_statusTunerHardwareAttrInit
 *
 **************************/
/*!
 * \brief		Initializes the Hardware attributes for ETAL
 * \param[in]	deviceID - the device ID of the CMOST 
 * \param[in]	tuner_hardware_attr - the tuner hardware attributes to use
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusHardwareAttrTunerInit( tU32 deviceID, const EtalTunerAttr *tuner_hardware_attr)
{
	if (deviceID < ETAL_CAPA_MAX_TUNER)
	{
		(void)OSAL_pvMemoryCopy((tVoid *)&etalStatus.hardwareAttr.m_tunerAttr[deviceID], (tPCVoid)tuner_hardware_attr, sizeof(EtalTunerAttr));
	}
	else
	{
		// should not happen : the device ID is incorrect
		ASSERT_ON_DEBUGGING(0);
	}
}

/***************************
 *
 * ETAL_statusHardwareAttrDcopInit
 *
 **************************/
/*!
 * \brief		Initializes the Hardware attributes for ETAL
 * \param[in]	dcop_hardware_attr - the dcop hardware attributes to use
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusHardwareAttrDcopInit(const EtalDCOPAttr *dcop_hardware_attr)
{
	(void)OSAL_pvMemoryCopy((tVoid *)&etalStatus.hardwareAttr.m_DCOPAttr, (tPCVoid)dcop_hardware_attr, sizeof(EtalDCOPAttr));
}


/***************************
 *
 * ETAL_statusHardwareAttrBackup
 *
 **************************/
/*!
 * \brief		Create a copy of the Hardware attributes used to initialize ETAL
 * \param[in]	hardware_attr - pointer to location where the function saves the current attributes
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusHardwareAttrBackup(EtalHardwareAttr *hardware_attr)
{
	(void)OSAL_pvMemoryCopy((tVoid *)hardware_attr, (tPCVoid)&etalStatus.hardwareAttr, sizeof(EtalHardwareAttr));
}

/***************************
 *
 * ETAL_statusHardwareAttrIsDCOPActive
 *
 **************************/
/*!
 * \brief		Checks if the DCOP is active by configuration
 * \details		Checks the parameter passed to #etal_initialize
 * \remark		Whether the DCOP is DAB or HD is defined by build time configuration
 * \return		TRUE - the DCOP is active
 * \return		FALSE - the DCOP is not active
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrIsDCOPActive(tVoid)
{
	return !etalStatus.hardwareAttr.m_DCOPAttr.m_isDisabled;
}

/***************************
 *
 * ETAL_statusHardwareAttrIsValidDCOPAttr
 *
 **************************/
/*!
 * \brief		Checks if the m_DCOPAttr is a valid configuration
 * \details		Checks the parameter passed to #etal_initialize
 * \remark		Whether the DCOP flashing support is defined by build time configuration
 * \return		TRUE - the m_DCOPAttr is valid configuration
 * \return		FALSE - the m_DCOPAttr is invalid configuration
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrIsValidDCOPAttr(tVoid)
{
	EtalDCOPAttr *dcop_attr = &(etalStatus.hardwareAttr.m_DCOPAttr);
	tBool retval = TRUE;
	
	if ((dcop_attr->m_isDisabled == FALSE) &&
		((((dcop_attr->m_doFlashProgram == TRUE) || (dcop_attr->m_doDownload == TRUE)) && 
#if (defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE) || defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE))
		(dcop_attr->m_pvGetImageContext == NULL)
#else
		(dcop_attr->m_cbGetImage == NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE) || defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE)
		) ||
		((dcop_attr->m_doFlashProgram == TRUE) && (dcop_attr->m_doDownload == TRUE)) ||
		((dcop_attr->m_doFlashDump == TRUE) &&
#if (defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE) || defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE))
		(dcop_attr->m_pvPutImageContext == NULL)
#else
		(dcop_attr->m_cbPutImage == NULL)
#endif // #if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE) || defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE)
		))
		)
	{
		retval = FALSE;
	}

	return retval;
}

/***************************
 *
 * ETAL_statusHardwareAttrGetDCOPAttr
 *
 **************************/
/*!
 * \brief		Returns the m_DCOPAttr pointer
 * \details		Returns the m_DCOPAttr pointer of type EtalDCOPAttr
 * \return		pointer to EtalDCOPAttr
 * \callgraph
 * \callergraph
 */
EtalDCOPAttr *ETAL_statusHardwareAttrGetDCOPAttr(tVoid)
{
	return &(etalStatus.hardwareAttr.m_DCOPAttr);
}

/***************************
 *
 * ETAL_statusHardwareAttrGetDCOPAttr
 *
 **************************/
/*!
 * \brief		Returns the doFlashProgram value
 * \details		Returns the doFlashProgram pointer
 * \return		the doFlashProgram value
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrGetDCOPAttrDoFlashProgram(tVoid)
{
	return etalStatus.hardwareAttr.m_DCOPAttr.m_doFlashProgram;
}

/***************************
 *
 * ETAL_statusHardwareAttrIsTunerActive
 *
 **************************/
/*!
 * \brief		Checks if the specified Tuner is active by configuration
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner to be checked
 * \return		TRUE - the Tuner is active
 * \return		FALSE - the Tuner is not active
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrIsTunerActive(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tBool retval;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		retval = FALSE;
	}
	else
	{
		//  (etalTuner[i].deviceDescr.m_deviceType != deviceUnknown)
		// 
		// we should have user enabled as well as system configured.
		// 
		retval = ((!etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_isDisabled)
		&& (etalTuner[tuner_index].deviceDescr.m_deviceType != deviceUnknown));
	}

	return retval;
}

/***************************
 *
 * ETAL_statusHardwareAttrUseXTALAlignment
 *
 **************************/
/*!
 * \brief		Checks if XTAL alignment should be used for a Tuner
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner to be checked
 * \return		TRUE - XTAL alignment was requested for the Tuner
 * \return		FALSE - XTAL alignment was not requested for the Tuner
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrUseXTALAlignment(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tBool retval;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		retval = FALSE;
	}
	else
	{
		retval = etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_useXTALalignment;
	}

	return retval;
}

/***************************
 *
 * ETAL_statusHardwareAttrGetXTALAlignment
 *
 **************************/
/*!
 * \brief		Returns the XTAL alignment for a Tuner
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner
 * \return		XTAL alignment
 * \callgraph
 * \callergraph
 */
tU32 ETAL_statusHardwareAttrGetXTALAlignment(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tU32 retval;
	
	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		retval = 0;
	}
	else
	{
		retval = etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_XTALalignment;
	}

	return retval;
}

/***************************
 *
 * ETAL_statusHardwareAttrUseTunerImage
 *
 **************************/
/*!
 * \brief		Checks if a Tuner image is available for download
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner
 * \return		TRUE - the Tuner image is present
 * \return		FALSE - the Tuner image is not present
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrUseTunerImage(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tBool retval;
	
	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		retval = FALSE;
	}
	else
	{
		retval = (etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_useDownloadImage != (tU8)0);
	}

	return retval;
}

/***************************
 *
 * ETAL_statusHardwareAttrGetTunerImage
 *
 **************************/
/*!
 * \brief		Returns the Tuner image to be used for download
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner
 * \param[out]	firmware - pointer to a location where the function stores the address of the
 * 				           firmware for this device
 * \param[out]	firmware_size - pointer to a location where the function stores the size of the
 * 				           data pointed by *firmware*
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusHardwareAttrGetTunerImage(ETAL_HANDLE hTuner, tU8 **firmware, tU32 *firmware_size)
{
	ETAL_HINDEX tuner_index;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		*firmware = NULL;
		*firmware_size = 0;
	}
	else
	{
		*firmware = etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_DownloadImage;
		if (*firmware == NULL)
		{
			*firmware_size = 0;
		}
		else
		{
			*firmware_size = etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_DownloadImageSize;
		}
	}
}

/***************************
 *
 * ETAL_statusHardwareAttrUseCustomParams
 *
 **************************/
/*!
 * \brief		Checks if Custom parameters should be loaded for a Tuner
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner
 * \return		TRUE - the Custom parameters should be used
 * \return		FALSE - the Custom parameters should not be used
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrUseCustomParams(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tBool retval;
	
	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		retval = FALSE;
	}
	else
	{
		retval = (etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_useCustomParam == (tU8)1);
	}
	return retval;
}

/***************************
 *
 * ETAL_statusHardwareAttrUseDefaultParams
 *
 **************************/
/*!
 * \brief		Checks if Default parameters should be loaded for aTuner
 * \details		Checks the parameter passed to #etal_initialize.
 *
 * 				The default parameters are embedded in ETAL at build-time
 * 				and available in the ETAL/tuner_driver/exports/param
 * 				directory.
 * \param[in]	hTuner - handle of the Tuner
 * \return		TRUE - the Custom parameters should be used
 * \return		FALSE - the Custom parameters should not be used
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusHardwareAttrUseDefaultParams(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tBool retval;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		retval = FALSE;
	}
	else
	{
		retval = (etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_useCustomParam == (tU8)0);
	}
	return retval;
}


/***************************
 *
 * ETAL_statusHardwareAttrGetCustomParams
 *
 **************************/
/*!
 * \brief		Returns the Custom parameters to be used for a Tuner
 * \details		Checks the parameter passed to #etal_initialize
 * \param[in]	hTuner - handle of the Tuner
 * \param[out]	params - pointer to a location where the function stores the address of the
 * 				         custom parameters for this device
 * \param[out]	params_size - pointer to a location where the function stores the size of the
 * 				              data pointed by *params*
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusHardwareAttrGetCustomParams(ETAL_HANDLE hTuner, tU32 **params, tU32 *params_size)
{
	ETAL_HINDEX tuner_index;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	if (tuner_index == ETAL_INVALID_HINDEX)
	{
		*params = NULL;
		*params_size = 0;
	}
	else
	{
		*params = etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_CustomParam;
		if (*params == NULL)
		{
			*params_size = 0;
		}
		else
		{
			*params_size = etalStatus.hardwareAttr.m_tunerAttr[tuner_index].m_CustomParamSize;
		}
	}
}

#if defined (CONFIG_TRACE_ENABLE)
/***************************
 *
 * ETAL_statusHardwareAttrGetTraceConfig
 *
 **************************/
/*!
 * \brief		Returns the trace configuration
 * \details		Checks the parameter passed to #etal_initialize
 * \remark		Returns a pointer to an internal ETAL status structure,
 * 				handle with care.
 * \return		pointer to the trace configuration
 * \callgraph
 * \callergraph
 */
EtalTraceConfig *ETAL_statusHardwareAttrGetTraceConfig(tVoid)
{
	return &etalStatus.hardwareAttr.m_traceConfig;
}
#endif // CONFIG_TRACE_ENABLE

/***************************
 *
 * ETAL_statusHardwareAttrGetNotifycb
 *
 **************************/
/*!
 * \brief		Returns the address of the user-provided ETAL callback
 * \details		During ETAL initialization the API user may specify a callback function that
 * 				ETAL will use for event notifications. The function returns the address of that function.
 * \return		Address of the user-provided callback function, or NULL if undefined
 * \see			etal_initialize
 * \callgraph
 * \callergraph
 */
EtalCbNotify ETAL_statusHardwareAttrGetNotifycb(tVoid)
{
	return etalStatus.hardwareAttr.m_cbNotify;
}

/***************************
 *
 * ETAL_statusHardwareAttrGetNotifyContext
 *
 **************************/
/*!
 * \brief		Returns the address of the user-provided callback context
 * \details		The user-provided callback is invoked with a user-specified context.
 * 				The function returns the context that was specified during ETAL initialization.
 * \remark		The context is in no way used or modified by ETAL.
 * \return		Address of the user-provided context, or NULL if undefined
 * \see			ETAL_statusHardwareAttrGetNotifycb
 * \callgraph
 * \callergraph
 */
tVoid *ETAL_statusHardwareAttrGetNotifyContext(tVoid)
{
	return etalStatus.hardwareAttr.m_context;
}

/***************************
 *
 * ETAL_statusIsInitialized
 *
 **************************/
/*!
 * \brief		Checks if ETAL status is initialized
 * \return		TRUE - the ETAL status is initialized
 * \return		FALSE - the ETAL status is not initialized
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusIsInitialized(tVoid)
{
	return etalStatus.isInitialized;
}

/***************************
 *
 * ETAL_statusSetInitialized
 *
 **************************/
/*!
 * \brief		Sets the ETAL initialization status
 * \param[in]	state - the new state
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetInitialized(tBool state)
{
	etalStatus.isInitialized = state;
}

/***************************
 *
 * ETAL_statusFillCapabilities
 *
 **************************/
/*!
 * \brief		Fills a capabilities structure
 * \details		Fills the structure based on the #etalTuner and 
 * 				the devices enabled at #etal_initialize time.
 * \param[out]	capa - pointer to a structure that the function will fill
 * \callgraph
 * \callergraph
 * \todo		DCOP capabilities need implementation
 */
tVoid ETAL_statusFillCapabilities(EtalHwCapabilities *capa)
{
	tU32 i, j;

	(void)OSAL_pvMemorySet((tPVoid) capa, 0x00, sizeof(EtalHwCapabilities));

#if defined (CONFIG_ETAL_SUPPORT_DCOP)
	if (!etalStatus.hardwareAttr.m_DCOPAttr.m_isDisabled)
	{
		(void)OSAL_pvMemoryCopy((tPVoid)&capa->m_DCOP, (tPCVoid)&etalDCOP, sizeof(EtalDeviceDesc));
	}
#endif // CONFIG_ETAL_SUPPORT_DCOP

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (!etalStatus.hardwareAttr.m_tunerAttr[i].m_isDisabled)
		{
			(void)OSAL_pvMemoryCopy((tPVoid)&capa->m_Tuner[i].m_TunerDevice, (tPCVoid)&etalTuner[i].deviceDescr, sizeof(EtalDeviceDesc));
			for (j = 0; j < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; j++)
			{
				capa->m_Tuner[i].m_standards[j] = etalTuner[i].frontEndList[j].standards;
				capa->m_Tuner[i].m_dataType[j] = etalTuner[i].frontEndList[j].dataTypes;
			}
		}
	}
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_statusGetCountMonitor
 *
 **************************/
/*!
 * \brief		Returns the number of monitors in the ETAL status
 * \return		number of monitors
 * \callgraph
 * \callergraph
 */
tU32 ETAL_statusGetCountMonitor(tVoid)
{
	return (tU32)etalStatus.monitorsCount;
}

/***************************
 *
 * ETAL_statusAddMonitorHandle
 *
 **************************/
/*!
 * \brief		Allocates and returns a Monitor handle
 * \details		Monitors are maintained in an array of the #etalStatus
 * 				of size #ETAL_MAX_MONITORS;	this function checks if
 * 				there is a free location and reserves it.
 * \return		The Monitor handle, or #ETAL_INVALID_HANDLE if all array entries
 * 				are occupied.
 * \callgraph
 * \callergraph
 */
static ETAL_HANDLE ETAL_statusAddMonitorHandle(tVoid)
{
	ETAL_HINDEX monitor_index;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;
	tU32 i;

	for (i = 0; i < ETAL_MAX_MONITORS; i++)
	{
		if (!etalStatus.monitors[i].isValid)
		{
			etalStatus.monitors[i].isValid = TRUE;
			etalStatus.monitorsCount += (tU8)1;
			monitor_index = (ETAL_HINDEX)i;
			retval = ETAL_handleMakeMonitor(monitor_index);
			goto exit;
		}
	}
	ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Exhausted monitor handles");

exit:
	return retval;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_statusAddReceiverHandle
 *
 **************************/
/*!
 * \brief		Adds a new Receiver handle to the ETAL global status
 * \remark		Receivers are statically allocated in an array of size #ETAL_MAX_RECEIVERS.
 * \return		The newly created Receiver handle
 * \return		ETAL_INVALID_HANDLE if the max number of Receivers is exceeded
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_statusAddReceiverHandle(tVoid)
{
	ETAL_HANDLE hReceiver;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;
	ETAL_HINDEX receiver_index;
	tU32 i;

	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		receiver_index = (ETAL_HINDEX)i;
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
		if (ETAL_receiverAllocateIfFree(hReceiver))
		{
			retval = hReceiver;
			goto exit;
		}
	}
	ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Exhausted receiver handles");

exit:
	return retval;
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * ETAL_statusClearDABTuneStatus
 *
 **************************/
/*!
 * \brief		Resets the DAB tune status
 * \param[in]	clear_all - if TRUE the function resets also the EnsembleId and ServiceId;
 * 				            if FALSE the function clear all other information only
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusClearDABTuneStatus(tBool clear_all)
{
	ETAL_statusGetTuneStatusLock();

	if (clear_all)
	{
		OSAL_pvMemorySet((tVoid *)&etalStatus.DABTuneStatus, 0x00, sizeof(etalDABTuneStatusTy));
		etalStatus.DABTuneStatus.UEId = ETAL_INVALID_UEID;
		etalStatus.DABTuneStatus.Service = ETAL_INVALID_SID;
	}
	else
	{
		tU32 eid = etalStatus.DABTuneStatus.UEId;
		tU32 srv = etalStatus.DABTuneStatus.Service;
		OSAL_pvMemorySet((tVoid *)&etalStatus.DABTuneStatus, 0x00, sizeof(etalDABTuneStatusTy));
		etalStatus.DABTuneStatus.UEId = eid;
		etalStatus.DABTuneStatus.Service = srv;
	}

	ETAL_statusReleaseTuneStatusLock();
}

/***************************
 *
 * ETAL_statusSetDABService
 *
 **************************/
/*!
 * \brief		Sets the EnsembleId and ServiceId
 * \details		Initializes or replaces the EnsembleId and ServiceId in the global ETAL DAB status.
 * 				Maintains also the information of whether the specified IDs are new or not (flag InfoIsNew
 * 				of #etalDABTuneStatusTy), used by ETAL to decide if to send update to the API user.
 * \param[in]	UEId    - the Unique Ensemble ID (ECC + EId)
 * \param[in]	service - the service ID
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetDABService(tU32 UEId, tU32 service)
{
	ETAL_statusGetTuneStatusLock();

	if ((UEId != etalStatus.DABTuneStatus.UEId) ||
		(service != etalStatus.DABTuneStatus.Service))
	{
		etalStatus.DABTuneStatus.UEId = UEId;
		etalStatus.DABTuneStatus.Service = service;
		etalStatus.DABTuneStatus.InfoIsNew = FIELD_IS_NEW;
		etalStatus.DABTuneStatus.ServiceLabelIsNew = (tU8)0;
		etalStatus.DABTuneStatus.PADDataIsNew = (tU8)0;
	}
	else
	{
		etalStatus.DABTuneStatus.InfoIsNew = (tU8)0;
	}
	etalStatus.DABTuneStatus.InfoIsNew |= FIELD_IS_INITIALIZED;;

	ETAL_statusReleaseTuneStatusLock();
}

/***************************
 *
 * ETAL_statusSetDABServiceLabel
 *
 **************************/
/*!
 * \brief		Sets the DAB Service Label
 * \details		Initializes or replaces the DAB service label, normally obtained from the Service Information tables.
 * \details		Maintains also the information of whether the label is new or not (flag ServiceLabelIsNew of
 * 				#etalDABTuneStatusTy), used by ETAL to decide if to send update to the API user.
 * \remark		ETAL maintains the Service Label only for the currently selected service.
 * \param[out]	label - pointer to a buffer where the function stores the null-terminated string containing the Service Label
 * 				        The max size of the string is #ETAL_DEF_MAX_SERVICENAME
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetDABServiceLabel(tChar *label, tU8 charSet)
{
	if (OSAL_s32StringCompare(etalStatus.DABTuneStatus.ServiceLabel, label) != 0)
	{
		OSAL_szStringCopy(etalStatus.DABTuneStatus.ServiceLabel, label);
		etalStatus.DABTuneStatus.ServiceLabelCharSet = charSet;
		etalStatus.DABTuneStatus.ServiceLabelIsNew = FIELD_IS_NEW;
	}
	else
	{
		/*
		 * this is reset also by a call to ETAL_statusSetDABService
		 */
		etalStatus.DABTuneStatus.ServiceLabelIsNew = (tU8)0;
	}
	etalStatus.DABTuneStatus.ServiceLabelIsNew |= FIELD_IS_INITIALIZED;
}

/***************************
 *
 * ETAL_statusSetDABPAD
 *
 **************************/
/*!
 * \brief		Sets the DAB PAD
 * \details		Initializes or replaces the DAB Program Associated Data, normally obtained from the Audio flow.
 * \details		Maintains also the information of whether the PAD is new or not (flag PADDataIsNew of #etalDABTuneStatusTy),
 * 				used by ETAL to decide if to send update to the API user.
 * \remark		ETAL maintains the PAD data only for the currently selected service.
 * \param[out]	ppad - pointer to a location where the function stores the PAD data, if it is available and new.
 * 				       In all other cases the location contents is unchanged.
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetDABPAD(etalPADDLSTy *ppad)
{
	ETAL_statusGetTuneStatusLock();

	if (OSAL_s32MemoryCompare((tPCVoid)&etalStatus.DABTuneStatus.PADDLS, (tPCVoid)ppad, sizeof(etalPADDLSTy)) != 0)
	{
		OSAL_pvMemoryCopy((tVoid *)&etalStatus.DABTuneStatus.PADDLS, (tPCVoid)ppad, sizeof(etalPADDLSTy));
		etalStatus.DABTuneStatus.PADDataIsNew = FIELD_IS_NEW;
	}
	else
	{
		/*
		 * this is reset also by a call to ETAL_statusSetDABService
		 */
		etalStatus.DABTuneStatus.PADDataIsNew = (tU8)0;
	}
	etalStatus.DABTuneStatus.PADDataIsNew |= FIELD_IS_INITIALIZED;

	ETAL_statusReleaseTuneStatusLock();
}

/***************************
 *
 * ETAL_statusGetDABService
 *
 **************************/
/*!
 * \brief		Gets the EnsembleId and ServiceId
 * \details		Returns the current EnsembleId and ServiceId and indicates if they changed since last call
 * 				to the function.
 * \param[out]	UEId    - pointer to the location where the function stores the Unique Ensemble ID (ECC + EId)
 * \param[out]	service - pointer to the location where the function stores the service ID
 * \param[out]	is_new  - pointer to the location where the function stores TRUE if the values changed
 * 				          since last call, FALSE otherwise
 * \return		OSAL_OK    - *UEId* and *service* contain valid values
 * \return		OSAL_ERROR - *UEId* and *service* do not contain valid values (because the relevant information
 * 				          was not stored yet)
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusGetDABService(tU32 *UEId, tU32 *service, tBool *is_new)
{
	if (IS_INITIALIZED(etalStatus.DABTuneStatus.InfoIsNew))
	{
		if (is_new)
		{
			*is_new =  IS_NEW(etalStatus.DABTuneStatus.InfoIsNew);
		}
		if (UEId)
		{
			*UEId =    etalStatus.DABTuneStatus.UEId;
		}
		if (service)
		{
			*service = etalStatus.DABTuneStatus.Service;
		}

		etalStatus.DABTuneStatus.InfoIsNew &= ~FIELD_IS_NEW;
		return OSAL_OK;
	}
	return OSAL_ERROR;
}

/***************************
 *
 * ETAL_statusGetDABServiceLabel
 *
 **************************/
/*!
 * \brief		Gets the Service Label
 * \details		Returns the current Service Label and indicates if it changed since last call
 * 				to the function.
 * \param[out]	label   - pointer to the location where the function stores the Service Label
 * \param[in]	max_len - max number of bytes to use in the *label*; if the Service Label is longer than
 * 				          *max_len* it will be truncated to *max_len*-1 and the last byte set to NULL
 * \param[out]	is_new  - pointer to the location where the function stores TRUE if the values changed
 * 				          since last call, FALSE otherwise
 * \return		OSAL_OK    - *label* contain valid values
 * \return		OSAL_ERROR - *label* does not contain valid value (because the relevant information
 * 				          was not stored yet)
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusGetDABServiceLabel(tChar *label, tU8 *charSet, tU32 max_len, tBool *is_new)
{
	if (IS_INITIALIZED(etalStatus.DABTuneStatus.ServiceLabelIsNew))
	{
		if (is_new)
		{
			*is_new = IS_NEW(etalStatus.DABTuneStatus.ServiceLabelIsNew);
		}
		OSAL_szStringNCopy(label, etalStatus.DABTuneStatus.ServiceLabel, max_len);
		label[max_len - 1] = '\0';
		*charSet = etalStatus.DABTuneStatus.ServiceLabelCharSet;

		etalStatus.DABTuneStatus.ServiceLabelIsNew &= ~FIELD_IS_NEW;
		return OSAL_OK;
	}
	return OSAL_ERROR;
}

/***************************
 *
 * ETAL_statusGetDABPAD
 *
 **************************/
/*!
 * \brief		Gets the DAB PAD
 * \details		Returns the current DAB Program Associated Data and indicates if it changed since last call
 * 				to the function.
 * \param[out]	ppad    - pointer to the location where the function stores the PAD
 * \param[out]	is_new  - pointer to the location where the function stores TRUE if the values changed
 * 				          since last call, FALSE otherwise
 * \return		TRUE    - label contain valid values
 * \return		FALSE   - label does not contain valid value (because the relevant information
 * 				          was not stored yet)
 * \see			etalDABTuneStatusTy
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusGetDABPAD(etalPADDLSTy *ppad, tBool *is_new)
{
	tBool ret = FALSE;

	ETAL_statusGetTuneStatusLock();

	if (IS_INITIALIZED(etalStatus.DABTuneStatus.PADDataIsNew))
	{
		if (is_new)
		{
			*is_new = IS_NEW(etalStatus.DABTuneStatus.PADDataIsNew);
		}
		OSAL_pvMemoryCopy((tVoid *)ppad, (tPCVoid)&etalStatus.DABTuneStatus.PADDLS, sizeof(etalPADDLSTy));
		etalStatus.DABTuneStatus.PADDataIsNew &= ~FIELD_IS_NEW;
		ret = TRUE;
	}

	ETAL_statusReleaseTuneStatusLock();
	return ret;
}

/***************************
 *
 * ETAL_statusConvertCountry
 *
 **************************/
/*!
 * \brief		Converts the ETAL country variant to the DABMW notation
 * \details		The country variant is currently used only for DAB DCOP
 * 				power up command (see #ETAL_cmdPowerUp_MDR).
 *
 * 				Only #ETAL_COUNTRY_VARIANT_EU is supported.
 * \param[in]	c - the country variant in ETAL representation
 * \return		The DABMW country variant 
 * \callgraph
 * \callergraph
 */
static DABMW_mwCountryTy ETAL_statusConvertCountry(EtalCountryVariant c)
{
	switch (c)
	{
		case ETAL_COUNTRY_VARIANT_EU:
			return DABMW_COUNTRY_EUROPE;

		case ETAL_COUNTRY_VARIANT_UNDEF:
			return DABMW_COUNTRY_NONE;

		default:
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Undefined country variant (%d)", c);
			return DABMW_COUNTRY_NONE;
	}
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/***************************
 *
 * ETAL_statusGetCountry
 *
 **************************/
/*!
 * \brief		Returns the ETAL country variant
 * \details		During ETAL initialization the API user may specify the country variant to be used
 * 				for customized band settings. This function returns the defined country variant.
 * \remark		ETAL does not currently use the country variant.
 * \return		The country variant, in #EtalCountryVariant format
 * \see			etal_initialize
 * \see			EtalCountryVariant
 * \callgraph
 * \callergraph
 */
EtalCountryVariant ETAL_statusGetCountry(tVoid)
{
	return etalStatus.hardwareAttr.m_CountryVariant;
}

/***************************
 *
 * ETAL_statusGetNvmLoadCfg
 *
 **************************/
/*!
 * \brief		Returns the NVM load configuration for the DCOP
 * \details		During ETAL initialization the API user may specify the NVM load configuration to be used.
 *              This function returns the NVM configuration.
 * \remark
 * \return		The NVM load configuration
 * \see			etal_initialize
 * \see			NVM configuration
 * \callgraph
 * \callergraph
 */
EtalNVMLoadConfig ETAL_statusGetNvmLoadCfg(tVoid)
{
	return etalStatus.hardwareAttr.m_NVMLoadConfig;
}

/***************************
 *
 * ETAL_statusInternalInit
 *
 **************************/
/*!
 * \brief		ETAL memory initialization
 * \details		Perform ETAL memory initialization; no communication with devices yet.
 * 				The *hardware_attr* passed to #etal_initialize are copied to the internal
 * 				status #etalStatus attr field, and are only accessed from there
 * \param[in]	hardware_attr - the ETAL-wide configuration items
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusInternalInit(const EtalHardwareAttr *hardware_attr)
{	
	ETAL_statusReset();

	if (hardware_attr != NULL)
	{
		ETAL_statusHardwareAttrInit(hardware_attr);
	}
}

/***************************
 *
 * ETAL_statusExternalInit
 *
 **************************/
/*!
 * \brief		ETAL final initialization
 * \details		Performs the following actions:
 *				- sends a power up command to the devices that support it (currently only the DAB DCOP)
 *				- resets the DAB DCOP filters used for the ETAL Monitors
 *				- initializes the HDRadio DCOP PSD polling
 *				- sets the ETAL state to initialized
 *
 * \return		OSAL_ERROR - error while communicating with one of the devices
 * \return		OSAL_OK    - initialization completed successfully
 * \callgraph
 * \callergraph
 * \todo		This function would be a good place to issue a reset to the MDR3
 *				but currently the command is not available, so just try to
 *				put in a known state
 */
tSInt ETAL_statusExternalInit(tVoid)
{
	tSInt retval = OSAL_OK;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tU8 NVM_Config_MDR = 0;
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	etalHDRXSWCnfgTy rx_sw_cnfg;
#endif

	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		NVM_Config_MDR = (ETAL_statusGetNvmLoadCfg().m_load_DAB_landscape == TRUE) ? ETAL_DAB_NVM_ACCESS_DAB_LANDSCAPE : 0;

		if (ETAL_cmdPowerUp_MDR(ETAL_statusConvertCountry(ETAL_statusGetCountry()), NVM_Config_MDR) != OSAL_OK)
		{
			retval = OSAL_ERROR;
		}
		else
		{
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
			/* 
			 * TODO see function header
			 */
			if (ETAL_cmdResetMonitorAndFilters_MDR() != OSAL_OK)
			{
				retval = OSAL_ERROR;
			}
#endif
		}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		if (ETAL_cmdGetSupportedServices_HDRADIO(&rx_sw_cnfg) == OSAL_OK)
		{
			ETAL_statusGetInternalLock();
			ETAL_statusSetHDSupportedServices(&rx_sw_cnfg);
			ETAL_statusReleaseInternalLock();

#if defined (CONFIG_MODULE_DCOP_HDRADIO_DISABLE_AAA)
			if (ETAL_cmdDisableAAA_HDRADIO() != OSAL_OK)
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Failed to disable AAA");
				retval = OSAL_ERROR;
			}
			else
#endif
			if (ETAL_configRadiotext_HDRADIO() != OSAL_OK)
			{
				retval = OSAL_ERROR;
			}
		}
		else
		{
			retval = OSAL_ERROR;
		}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	}

	etalStatus.isInitialized = TRUE;
	

	return retval;
}

#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
/***************************
 *
 * ETAL_statusStartRDSIrq
 *
 **************************/
/*!
 * \brief		Enables reading RDS through Interrupt instead of Polling for a Receiver
 * \details		Registers the Receiver destination for RDS data delivered by the CMOST
 * 				through Interrupt. The actual Interrupt processing is performed in
 * 				ETAL_IRQ_ThreadEntry that, when woken up by an RDS interrupt, needs
 * 				to check which receiver is registered for RDS reception.
 * \remark		The Receiver is stored in an array because potentially more than one
 * 				STAR device could be connected in a system to generate RDS interrupt.
 * 				In practice ETAL currently supports at most only two concurrent Receivers,
 *				both connected to the same STAR device, to the Foreground and Background channels.
 * \param[in]	hReceiver - Receiver handle
 * \return		OSAL_OK    - the Receiver was correctly registered
 * \return		OSAL_ERROR - the Receiver is not RDS capable (e.g. DOT flavour of CMOST device) thus the
 * 				        operation was rejected
 * \see			ETAL_statusIsRDSIrqThreadActive for reason why only one CMOST device is supported at a time
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusStartRDSIrq(ETAL_HANDLE hReceiver)
{
	ETAL_HINDEX receiver_index;
	tSInt retval;

    if (ETAL_receiverIsRDSCapable(hReceiver) == TRUE)
    {
		receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
        etalStatus.hReceiverRDSIRQ[receiver_index] = hReceiver;
        retval = OSAL_OK;
    }
    else
    {
        retval = OSAL_ERROR;
    }

		return retval;
}

/***************************
 *
 * ETAL_statusStopRDSIrq
 *
 **************************/
/*!
 * \brief		Disables reading RDS through Interrupt for a Receiver
 * \param[in]	hReceiver - Receiver handle
 * \see			ETAL_statusStartRDSIrq
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusStopRDSIrq(ETAL_HANDLE hReceiver)
{
	ETAL_HINDEX receiver_index;

    if (ETAL_receiverIsValidHandle(hReceiver) == TRUE)
    {
		receiver_index = ETAL_handleReceiverGetIndex(hReceiver);
        etalStatus.hReceiverRDSIRQ[receiver_index] = ETAL_INVALID_HANDLE;
    }
}

/***************************
 *
 * ETAL_statusIsRDSIrqThreadActive
 *
 **************************/
/*!
 * \brief		Checks if there is at least one Receiver configured for RDS reading by Interrupt
 * \remark		If more than one Receiver is configured for RDS through Interrupt, the function
 * 				returns the handle of the first one it finds.
 * \param[out]	phReceiver - location where the function stores the handle of the Receiver that
 * 				requested RDS through Interrupt, or ETAL_INVALID_HANDLE if no such Receiver is currently active
 * \return		TRUE  - there is a valid Receiver handle in *phReceiver*
 * \return		FALSE - no Receiver is currently configured for RDS through Interrupt
 * \see			ETAL_IRQ_ThreadEntry
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusIsRDSIrqThreadActive(ETAL_HANDLE *phReceiver)
{
	ETAL_HANDLE hReceiver;
    ETAL_HINDEX receiver_index;
		tBool retval = FALSE;

    for (receiver_index = 0; receiver_index < ETAL_MAX_RECEIVERS; receiver_index++)
    {
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
        if (etalStatus.hReceiverRDSIRQ[receiver_index] == hReceiver)
        {
            /* return the first hReceiver with RDS active */
            *phReceiver = hReceiver;
            retval = TRUE;
        		goto exit;
        }
    }
    /* no hReceiver with  RDS active, return invalid handle */
    *phReceiver = ETAL_INVALID_HANDLE;

exit:
	return retval;
}

/***************************
 *
 * ETAL_statusGetRDSIrqhReceiver
 *
 **************************/
/*!
 * \brief		Searches the Receiver that registered for RDS processing
 * \details		ETAL currently supports fetching RDS through Interrupt only from one
 * 				CMOST device at a time. The RDS data provided by the CMOST contains
 * 				the information on the CMOST channel (foreground or background) that
 * 				generated the RDS information. The function searches the Receiver that
 * 				is using that CMOST channel.
 * \param[in]	tunerId - handle of the Tuner (i.e. CMOST device) that received the data
 * \param[in]	channel - the CMOST channel used by the Tuner
 * \param[out]	phReceiver - location where the function stores the handle of the Receiver that
 * 				requested RDS through Interrupt, or ETAL_INVALID_HANDLE if no such Receiver is currently active
 * \return		TRUE  - there is a valid Receiver handle in *phReceiver*
 * \return		FALSE - no Receiver corresponds to the *tunerId*, *channel*
 * \see			ETAL_RDSRawPeriodicFunc
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusGetRDSIrqhReceiver(ETAL_HANDLE tunerId, etalChannelTy channel, ETAL_HANDLE *phReceiver)
{
    ETAL_HANDLE l_hTuner;
	ETAL_HANDLE hReceiver;
    ETAL_HINDEX receiver_index;
    etalChannelTy hchannel;
		tBool retval = FALSE;

    for (receiver_index = 0; receiver_index < ETAL_MAX_RECEIVERS; receiver_index++)
    {
		hReceiver = ETAL_handleMakeReceiver(receiver_index);
        if ((etalStatus.hReceiverRDSIRQ[receiver_index] == hReceiver) &&
            (ETAL_receiverGetTunerId(hReceiver, &l_hTuner) == OSAL_OK) && (l_hTuner == tunerId) &&
            (ETAL_receiverGetChannel(hReceiver, &hchannel) == OSAL_OK) && (hchannel == channel))
        {
            /* return the hReceiver with RDS active that match tunerId and channel */
            *phReceiver = hReceiver;
            retval = TRUE;
						goto exit;
        }
    }
    /* no hReceiver with  RDS active that match tunerId and channel found, return invalid handle */
    *phReceiver = ETAL_INVALID_HANDLE;

exit:
    return retval;
}
#endif // CONFIG_COMM_ENABLE_RDS_IRQ


#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_statusDestroyAllMonitorsForReceiver
 *
 **************************/
/*!
 * \brief		Destroys all the Monitors associated with a Receiver
 * \param[in]	hReceiver - the Receiver handle
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_statusDestroyAllMonitorsForReceiver(ETAL_HANDLE hReceiver)
{
	ETAL_HANDLE hMonitor;
	ETAL_HINDEX monitor_index;
	etalMonitorTy *pmon;
	tU32 i;

	for (i = 0; i < ETAL_MAX_MONITORS; i++)
	{
		monitor_index = (ETAL_HINDEX)i;
		hMonitor = ETAL_handleMakeMonitor(monitor_index);
		pmon = ETAL_statusGetMonitor(hMonitor);
		if (pmon == NULL)
		{
			goto exit;
		}
		else if (pmon->isValid && (ETAL_statusGetReceiverFromMonitor(pmon) == hReceiver))
		{
			/*
			 * an error return implies a communication error and a
			 * status misalignment; the only sane thing to do is to
			 * restart ETAL
			 * We rely on the ETAL delivering the COMMUNICATION ERROR
			 * event to the application
			 */
			(LINT_IGNORE_RET) ETAL_statusDestroyMonitor(hMonitor);
		}
		else
		{
			/* Nothing to do */
		}
	}

exit:
	return;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_statusDestroyReceiver
 *
 **************************/
/*!
 * \brief		Destroys a Receiver
 * \details		Stops all services and deletes all Datapaths associated with the Receiver, then
 * 				destroys the receiver. The corresponding handle is freed for re-use.
 * \param[in]	hReceiver - the handle of the Receiver to destroy
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusDestroyReceiver(ETAL_HANDLE hReceiver)
{

#if defined(CONFIG_ETAL_HAVE_ETALTML) && defined(CONFIG_ETALTML_HAVE_RADIOTEXT)
	ETALTML_RDSresetData(hReceiver);
#endif
	ETAL_receiverDestroyAllDatapathsForReceiver(hReceiver);
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
	ETAL_statusDestroyAllMonitorsForReceiver(hReceiver);
#endif
	ETAL_intCbDeregisterPeriodicReceiver(hReceiver);
	ETAL_receiverResetStatus(hReceiver);
}

#if defined (CONFIG_ETAL_SUPPORT_CMOST)
/***************************
 *
 * ETAL_statusSetTunerAudioStatus
 *
 **************************/
/*!
 * \brief		Initializes the audio status
 * \remark		Operates only on the ETAL status variable, does not perform any hardware reconfiguration.
 * \param[in]	hTuner - Tuner handle
 * \param[in]	status - The new audio status
 * \see			ETAL_cmdSelectAudioInterface_CMOST
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetTunerAudioStatus(ETAL_HANDLE hTuner, etalAudioIntfStatusTy status)
{
	ETAL_HINDEX tuner_index;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	etalStatus.audioInterfaceStatus[tuner_index] = status;
}

/***************************
 *
 * ETAL_statusGetTunerAudioStatus
 *
 **************************/
/*!
 * \brief		Returns the current audio status
 * \param[in]	hTuner - Tuner handle
 * \return		The tuner audio interface status
 * \callgraph
 * \callergraph
 */
etalAudioIntfStatusTy ETAL_statusGetTunerAudioStatus(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	return etalStatus.audioInterfaceStatus[tuner_index];
}

/***************************
 *
 * ETAL_statusSetAudioSource
 *
 **************************/
/*!
 * \brief		Initializes the audio source status
 * \remark		Operates only on the ETAL status variable, does not perform any hardware reconfiguration.
 * \param[in]	hReceiver - the handle of the Receiver that have audio source
 * \param[in]	source - The new audio source status
 * \see			ETAL_audioSelectInternal
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetAudioSource(ETAL_HANDLE hReceiver, EtalAudioSourceTy source)
{
	etalStatus.audioSourceReceiver = hReceiver;
	etalStatus.audioSource = source;
}

/***************************
 *
 * ETAL_statusGetTunerAudioStatus
 *
 **************************/
/*!
 * \brief		Returns the current audio source status
 * \param[out]	hReceiver - the handle of the Receiver that have audio source
 * \param[out]	source - The new audio source status
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusGetAudioSource(ETAL_HANDLE *phReceiver, EtalAudioSourceTy *psource)
{
	if (phReceiver != OSAL_NULL)
	{
		*phReceiver = etalStatus.audioSourceReceiver;
	}
	if (psource != OSAL_NULL)
	{
		*psource = etalStatus.audioSource;
	}
}
#endif // CONFIG_ETAL_SUPPORT_CMOST

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_statusFilterCountIncrement
 *
 **************************/
/*!
 * \brief		Increments the total number of filters
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusFilterCountIncrement(tVoid)
{
	etalStatus.filtersCount = etalStatus.filtersCount + (tS8)1;
}

/***************************
 *
 * ETAL_statusFilterCountDecrement
 *
 **************************/
/*!
 * \brief		Decrements the total number of filters
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusFilterCountDecrement(tVoid)
{
	etalStatus.filtersCount = etalStatus.filtersCount - (tS8)1;
}

/***************************
 *
 * ETAL_statusFilterCountGet
 *
 **************************/
/*!
 * \brief		Returns the total number of filters
 * \return		The current number of filters
 * \callgraph
 * \callergraph
 */
tS32 ETAL_statusFilterCountGet(tVoid)
{
	return (tS32)etalStatus.filtersCount;
}

/***************************
 *
 * ETAL_statusFilterGet
 *
 **************************/
/*!
 * \brief		Returns a filter's internal state
 * \remark		Returns a pointer to an internal ETAL status, handle with care
 * \param[in]	The filter to return
 * \return		pointer to the filter status
 * \callgraph
 * \callergraph
 */
etalDCOPFilterTy *ETAL_statusFilterGet(tU32 filter_index)
{
	return &etalStatus.filters[filter_index];
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/***************************
 *
 * ETAL_statusIsValidMonitorHandle
 *
 **************************/
/*!
 * \brief		Checks if a Monitor handle is valid
 * \details		A Monitor handle is considered valid if its value not out of allowed bounds and if
 * 				the corresponding Monitor has been initialized with valid values, or is not initialized.
 * \param[in]	pMonitor - pointer to a location containing the Monitor handle to check
 * \return		TRUE  - the Monitor handle is valid
 * \return		FALSE - the Monitor handle is invalid
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusIsValidMonitorHandle(ETAL_HANDLE *pMonitor)
{
	etalMonitorTy *pmon;
	ETAL_HINDEX monitor_index;
	tBool retval;

	if (pMonitor == NULL)
	{
		retval = FALSE;
		goto exit;
	}
	if (*pMonitor == ETAL_INVALID_HANDLE)
	{
		retval = TRUE; // valid, we need to initialize it
		goto exit;
	}
	monitor_index = ETAL_handleMonitorGetIndex(*pMonitor);
	if ((monitor_index == ETAL_INVALID_HINDEX) ||
		((tU32)monitor_index >= ETAL_MAX_MONITORS))
	{
		retval = FALSE;
		goto exit;
	}
	pmon = ETAL_statusGetMonitor(*pMonitor);
	if (pmon == NULL)
	{
		retval = FALSE;
		goto exit;
	}

	retval = pmon->isValid && (ETAL_statusGetReceiverFromMonitor(pmon) != ETAL_INVALID_HANDLE);

exit:
	return retval;
}

/***************************
 *
 * ETAL_statusIsValidMonitor
 *
 **************************/
/*!
 * \brief		Checks if a Monitor is valid
 * \details		Performs some sanity checks on the specified Monitor:
 * 				- if the Monitor limits are within bounds
 * 				- if the Monitor updated frequency is supported (it must be a multiple of #ETAL_POLL_MONITOR_SCHEDULING).
 *
 * \param[in]	q - pointer to a Monitor description
 * \return		TRUE  - the Monitor description is valid
 * \return		FALSE - the Monitor description is not valid
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusIsValidMonitor(const EtalBcastQualityMonitorAttr *q)
{
	const EtalQaMonitoredEntryAttr *pmonattr;
	tU32 i;
	tBool retval = TRUE;

	for (i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
	{
		pmonattr = &q->m_monitoredIndicators[i];
		if (pmonattr->m_MonitoredIndicator == EtalQualityIndicator_Undef)
		{
			break;
		}
		if (((pmonattr->m_InferiorValue != ETAL_INVALID_MONITOR) ||
			(pmonattr->m_SuperiorValue != ETAL_INVALID_MONITOR)) &&
			((tU32)pmonattr->m_UpdateFrequency == 0))
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Monitor interval m_UpdateFrequency must be specified if m_InferiorValue or m_SuperiorValue is specified");
			retval = FALSE;
			goto exit;
		}
		if (((tU32)pmonattr->m_UpdateFrequency % ETAL_POLL_MONITOR_SCHEDULING) != 0)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Monitor interval must be multiple of %dms", ETAL_POLL_MONITOR_SCHEDULING);
			retval = FALSE;
			goto exit;
		}
	}

exit:
	return retval;
}

/***************************
 *
 * ETAL_statusGetMonitor
 *
 **************************/
/*!
 * \brief		Returns a Monitor's internal description
 * \remark		Does not validate its argument.
 * \param[in]	hMonitor - handle of a valid Monitor
 * \return		pointer to the Monitor's internal data structure of type etalMonitorTy
 * \callgraph
 * \callergraph
 */
etalMonitorTy *ETAL_statusGetMonitor(ETAL_HANDLE hMonitor)
{
	ETAL_HINDEX monitor_index;
	etalMonitorTy *retval;

	monitor_index = ETAL_handleMonitorGetIndex(hMonitor);
	if (monitor_index == ETAL_INVALID_HINDEX)
	{
		retval = NULL;
	}
	else
	{
		retval = &etalStatus.monitors[monitor_index];
	}
	
	return retval;
}

/***************************
 *
 * ETAL_statusGetReceiverFromMonitor
 *
 **************************/
/*!
 * \brief		Returns the Receiver to which a Monitor is associated
 * \remark		Does not validate its argument.
 * \param[in]	pmon - pointer to a Monitor internal data structure of type etalMonitorTy
 * \return		Receiver handle
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_statusGetReceiverFromMonitor(etalMonitorTy *pmon)
{
	ETAL_HANDLE receiverHandle = ETAL_INVALID_HANDLE;

	if(pmon != NULL)
	{
		receiverHandle = pmon->requested.m_receiverHandle;
	}
	return receiverHandle;
}

/***************************
 *
 * ETAL_statusCountIndicatorsForMonitor
 *
 **************************/
/*!
 * \brief		Counts the indicators configured for a Monitor
 * \details		A Monitor is composed of up to #ETAL_MAX_QUALITY_PER_MONITOR indicators.
 * 				This function returns the number of initialized indicators.
 * \remark		Does not validate its argument.
 * \param[in]	pmon - pointer to a Monitor internal data structure of type etalMonitorTy
 * \return		Number of indicators
 * \callgraph
 * \callergraph
 */
tU32 ETAL_statusCountIndicatorsForMonitor(etalMonitorTy *pmon)
{
	tU32 i;
	tU32 count = 0;
	EtalBcastQualityMonitorAttr *mon = NULL;

	if(pmon != NULL)
	{
		mon = &pmon->requested;

		for (i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
		{
			if (mon->m_monitoredIndicators[i].m_MonitoredIndicator != EtalQualityIndicator_Undef)
			{
				count++;
			}
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_statusCountIndicatorsForMonitor pmon: 0x%x", pmon);
		ASSERT_ON_DEBUGGING(0);
	}
	return count;
}

/***************************
 *
 * ETAL_statusCreateModifyMonitor
 *
 **************************/
/*!
 * \brief		Creates a new or modifies an existing Monitor
 * \details		Checks if the requested Monitor can be added or modified; if so removes the previous
 * 				monitor's filters and copies the requested Monitor attributes to the etalStatus.
 * \param[in]	pMonitor     - pointer to a valid Monitor handle to modify, or to ETAL_INVALID_HANDLE;
 * 				               in the latter case the function creates a new Monitor an returns its handle
 * 				               in the location pointed by *pMonitor*
 * \param[in,out] pMonitorAttr - pointer to an EtalBcastQualityMonitorAttr describing the Monitor attributes
 * \return		OSAL_OK - no error
 * \return		OSAL_ERROR - for new Monitor: the Monitor cannot be added, max supported number (ETAL_MAX_MONITORS) reached;
 * 				             for existing Monitor modification: there was a communication error with the MDR
 * 				             device (it was not possible to remove existing MDR Filters) so the Monitor was not modified,
 * 				             or the handle pointed by *pMonitor* is invalid
 * \callgraph
 * \callergraph
 * \todo		Need some strategy in case some internal function call returns a error:
 * 				it means some filters have been removed, others not
 *				so potentially ETAL and MDR are not aligned;
 *				one possibility would be to reset all filters for
 *				the application and re-configure from scratch.
 */
tSInt ETAL_statusCreateModifyMonitor(ETAL_HANDLE* pMonitor, const EtalBcastQualityMonitorAttr* pMonitorAttr)
{
	etalMonitorTy *pmon = NULL;
	tBool create_new_handle = FALSE;
	tSInt retval = OSAL_OK;
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	EtalBcastStandard std;
	tU32 device_list;

	device_list = ETAL_cmdRoutingCheck(pMonitorAttr->m_receiverHandle, commandBandSpecific);
	std = ETAL_receiverGetStandard(pMonitorAttr->m_receiverHandle);
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		(std == ETAL_BCAST_STD_DAB))
	{
		if (ETAL_checkFilter_MDR(*pMonitor, pMonitorAttr) != OSAL_OK)
		{
			retval = OSAL_ERROR;
			goto exit;
		}
	}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

	/*
	 * create a new entry or overwrite the old one
	 */
	if (*pMonitor == ETAL_INVALID_HANDLE)
	{
		/*
		 * create a new entry
		 */
		if (etalStatus.monitorsCount >= (tU8)ETAL_MAX_MONITORS)
		{
			retval = OSAL_ERROR;
			goto exit;
		}
		create_new_handle = TRUE;
	}
	else
	{
		/*
	 	 * overwrite: destroy the old monitor's filters before overwriting
	 	 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
			(std == ETAL_BCAST_STD_DAB))
		{
			retval |= ETAL_cmdRemoveFiltersForMonitor_MDR(*pMonitor);
		}
#endif
	}

	if (retval == OSAL_OK)
	{
		if (create_new_handle)
		{
			*pMonitor = ETAL_statusAddMonitorHandle();
		}
		pmon = ETAL_statusGetMonitor(*pMonitor);
		if (pmon == NULL)
		{
			retval = OSAL_ERROR;
		}
		else
		{
			(void)OSAL_pvMemoryCopy((tVoid *)&pmon->requested, (tPCVoid)pMonitorAttr, sizeof(EtalBcastQualityMonitorAttr));
			pmon->standard = ETAL_receiverGetStandard(pMonitorAttr->m_receiverHandle);
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
				(pmon->standard == ETAL_BCAST_STD_DAB))
			{
				ETAL_resetFilter_MDR(pmon);
			}
#endif
		}
	}
	/*
	 * TODO see function header
	 */

exit:
	return retval;
}


/***************************
 *
 * ETAL_statusDestroyMonitor
 *
 **************************/
/*!
 * \brief		Destroys a Monitor
 * \details		Removes all filters associated with the Monitor, and then
 * 				destroys the monitor status.
 * \remark		If it detects that all filters or all monitors are empty,
 * 				sends command to DAB DCOP to stop all GetMonitor*Information
 * 				because this was configured with low SKIP COUNT and thus without
 * 				filters it could 'saturate' the bus.
 * \remark		UPDATE 9 Sept 2014: the above condition was set as a catch-all
 * 				in case of transmission errors, but it is breaking the behaviour
 * 				when DCOP+CMOST are in the system, since the #ETAL_cmdResetMonitorAndFilters_MDR
 * 				does not check if monitors are DCOP's or CMOST's and resets all.
 * \param[in]	hMonitor - handle of the Monitor to be destroyed
 * \return		OSAL_OK    - the Monitor was destroyed
 * \return		OSAL_ERROR - communication error while deleting the DCOP filters, or invalid *hMonitor*
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusDestroyMonitor(ETAL_HANDLE hMonitor)
{
	tSInt retval = OSAL_OK;
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
	etalMonitorTy *pmon = NULL;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tU32 device_list;
#endif

	if (etalStatus.monitorsCount == (tU8)0)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = OSAL_ERROR;
		goto exit;
	}

	pmon = ETAL_statusGetMonitor(hMonitor);
	if (pmon == NULL)
	{
		retval = OSAL_ERROR;
		goto exit;
	}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	device_list = ETAL_cmdRoutingCheck(ETAL_statusGetReceiverFromMonitor(pmon), commandBandSpecific);
	if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
		(pmon->standard == ETAL_BCAST_STD_DAB))
	{
		retval = ETAL_cmdRemoveFiltersForMonitor_MDR(hMonitor);
	}
#endif

	if (retval == OSAL_OK)
	{
		ETAL_resetMonitor(pmon);
		etalStatus.monitorsCount -= (tU8)1;
	}
#else
	/*
	 * Nothing to do for DOT, quality measurements are not available (and thus neither are Monitors)
	 */
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR || CONFIG_ETAL_SUPPORT_DCOP_MDR 
exit:
	return retval;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_statusRemoveDataServiceEntry
 *
 **************************/
/*!
 * \brief		Invalidates a Data Service Info entry
 * \details		The entry is considered invalid if the associated Datapath is #ETAL_INVALID_HANDLE.
 * \param[in]	dsi - pointer to the entry to invalidate.
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_statusRemoveDataServiceEntry(etalDataServiceInfoTy *dsi)
{
	if(dsi != NULL)
	{
		dsi->hDatapath = ETAL_INVALID_HANDLE;
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_statusRemoveDataServiceEntry dsi: 0x%x", dsi);
		ASSERT_ON_DEBUGGING(0);
	}
}

/***************************
 *
 * ETAL_statusResetDataServiceInfo
 *
 **************************/
/*!
 * \brief		Resets the Data Service Info
 * \details		The Data Service Info is a field of #etalStatus implemented
 * 				with an array of size #ETAL_MAX_DATASERVICE_CACHE. This function
 * 				clears the whole array.
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_statusResetDataServiceInfo(tVoid)
{
	etalDataServiceInfoTy *dsi;
	tU32 i;

	for (i = 0; i < ETAL_MAX_DATASERVICE_CACHE; i++)
	{
		dsi = &etalStatus.dataServiceCache[i];
		ETAL_statusRemoveDataServiceEntry(dsi);
	}
}

/***************************
 *
 * ETAL_statusSearchDataServiceInfo
 *
 **************************/
/*!
 * \brief		Returns a Data Service Info structure
 * \details		Searches in the Data Service Info the entry relative to 
 * 				*packet_address* and *subchid*. If *hDatapath* is valid
 * 				match it also, otherwise ignores it.
 * \param[in]	hDatapath - the Datapath handle to match or #ETAL_INVALID_HANDLE
 * 				            if the Datapath should not be matched
 * \param[in]	packet_address - the packet address to match
 * \param[in]	subchid - the Subchannel Id to match
 * \return		Pointer to the Data Service Info structure inside the #etalStatus,
 * 				or NULL if the matching entry is not found.
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
static etalDataServiceInfoTy *ETAL_statusSearchDataServiceInfo(ETAL_HANDLE hDatapath, tU16 packet_address, tU8 subchid)
{
	etalDataServiceInfoTy *dsi;
	tU32 i;

	for (i = 0; i < ETAL_MAX_DATASERVICE_CACHE; i++)
	{
		dsi = &etalStatus.dataServiceCache[i];
		if ((dsi->packetAddress == packet_address) &&
			(dsi->subchId == subchid))
		{
			if (hDatapath == ETAL_INVALID_HANDLE)
			{
				return dsi;
			}
			else if (dsi->hDatapath == hDatapath)
			{
				return dsi;
			}
		}
	}
	return NULL;
}

/***************************
 *
 * ETAL_statusSearchEmptyDataServiceInfo
 *
 **************************/
/*!
 * \brief		Returns the pointer to an empty Data Service Info entry
 * \return		The pointer to the empty entry, or NULL if there is none
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
static etalDataServiceInfoTy *ETAL_statusSearchEmptyDataServiceInfo(tVoid)
{
	etalDataServiceInfoTy *dsi;
	tU32 i;

	for (i = 0; i < ETAL_MAX_DATASERVICE_CACHE; i++)
	{
		dsi = &etalStatus.dataServiceCache[i];
		if (dsi->hDatapath == ETAL_INVALID_HANDLE)
		{
			return dsi;
		}
	}
	return NULL;
}

/***************************
 *
 * ETAL_statusAddDataServiceInfo
 *
 **************************/
/*!
 * \brief		Add a DAB Data Service information to the Data Service cache
 * \details		ETAL maintains a cache of the currently configured DAB Data Services
 * 				containing the DAB Packet Address, the DAB subchannel Id and the
 * 				handle of the Datapath handling the Data Service.
 * \details		The cache is read when a data packet arrives on the Data Channel to
 * 				determine which Datapath sink to invoke.
 * \remark		It is not allowed to register twice the same Data Service to two different Datapaths.
 * 				The function returns OSAL_ERROR in this case and does not modify the cache.
 * \param[in]	hDatapath      - handle of the Datapath that was configured to process this Data Service
 * \param[in]	packet_address - DAB Network Packet address
 * \param[in]	subchid        - DAB subchannel ID
 * \return		OSAL_OK    - the Data Service information was recorded (or was already present)
 * \return		OSAL_ERROR - the Data Service is already present in the cache for different Datapath,
 * 				             or the Data Service cache is full. The Data Service is a statically allocated
 * 				             array of size #ETAL_MAX_DATASERVICE_CACHE.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusAddDataServiceInfo(ETAL_HANDLE hDatapath, tU16 packet_address, tU8 subchid)
{
	etalDataServiceInfoTy *dsi;

	/*
	 * check if the service is already cached
	 */
	dsi = ETAL_statusSearchDataServiceInfo(hDatapath, packet_address, subchid);
	if (dsi != NULL)
	{
		if (dsi->hDatapath != hDatapath)
		{
			/*
			 * the user selected twice the same service, with different Datapath parameter
			 */
			return OSAL_ERROR;
		}
		return OSAL_OK;
	}

	/*
	 * add a new entry
	 */
	dsi = ETAL_statusSearchEmptyDataServiceInfo();
	if (dsi != NULL)
	{
		dsi->hDatapath = hDatapath;
		dsi->packetAddress = packet_address;
		dsi->subchId = subchid;
		return OSAL_OK;
	}

	ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Exhausted Data Service cache");
	return OSAL_ERROR;
}

/***************************
 *
 * ETAL_statusRemoveDataServiceInfo
 *
 **************************/
/*!
 * \brief		Remove a DAB Data Service information from the Data Service cache
 * \details		Remove the DAB Data Service cache entry matching *hDatapath*, *packet_address* and *subchid*.
 * \param[in]	hDatapath      - handle of the Datapath that was configured to process this Data Service
 * \param[in]	packet_address - DAB Network Packet address
 * \param[in]	subchid        - DAB subchannel ID
 * \return		OSAL_OK    - the Data Service information was removed
 * \return		OSAL_ERROR - the Data Service information is not present in the cache
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
tSInt ETAL_statusRemoveDataServiceInfo(ETAL_HANDLE hDatapath, tU16 packet_address, tU8 subchid)
{
	etalDataServiceInfoTy *dsi;
	
	dsi = ETAL_statusSearchDataServiceInfo(hDatapath, packet_address, subchid);
	if (dsi != NULL)
	{
		ETAL_statusRemoveDataServiceEntry(dsi);
		return OSAL_OK;
	}
	return OSAL_ERROR;
}

/***************************
 *
 * ETAL_statusRemoveDataServiceInfoForDatapath
 *
 **************************/
/*!
 * \brief		Remove all the DAB Data Service cache entries referring to a Datapath
 * \remark		Normally used before destroying a Datapath.
 * \param[in]	hDatapath - handle of the Datapath to be searched in the Data Service cache
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusRemoveDataServiceInfoForDatapath(ETAL_HANDLE hDatapath)
{
	etalDataServiceInfoTy *dsi;
	tU32 i;
	
	for (i = 0; i < ETAL_MAX_DATASERVICE_CACHE; i++)
	{
		dsi = &etalStatus.dataServiceCache[i];
		if (dsi->hDatapath == hDatapath)
		{
			ETAL_statusRemoveDataServiceEntry(dsi);
		}
	}
}

/***************************
 *
 * ETAL_statusGetDataServiceInfo
 *
 **************************/
/*!
 * \brief		Searches the Datapath registered for the DAB Data Service
 * \details		Returns the Datapath handle of the first Datapath found in the DAB Data Service cache
 * 				registered for processing some DAB Data Service.
 * \param[in]	packet_address - DAB Network Packet address
 * \param[in]	subchid        - DAB subchannel ID
 * \return		handle of the Datapath registered to process this Data Service,
 * 				or ETAL_INVALID_HANDLE if the Data Service is not present in the cache
 * \see			ETAL_statusAddDataServiceInfo
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_statusGetDataServiceInfo(tU16 packet_address, tU8 subchid)
{
	etalDataServiceInfoTy *dsi;

	dsi = ETAL_statusSearchDataServiceInfo(ETAL_INVALID_HANDLE, packet_address, subchid);
	if (dsi != NULL)
	{
		return dsi->hDatapath;
	}
	return ETAL_INVALID_HANDLE;
}

/***************************
 *
 * ETAL_statusIsPADActive
 *
 **************************/
/*!
 * \brief		Checks if the DAB PAD is active
 * \details		PAD is Program Associated Data, i.e. meta-information transmitted in the DAB audio stream.
 * 				It may be used to carry for example the song title and author.
 *
 * 				ETAL may configure the DAB DCOP to periodically communicate DAB PAD information.
 * 				The function checks if this process is started.
 * \return		TRUE  if the DAB PAD is already started
 * \return		FALSE otherwise
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusIsPADActive(tVoid)
{
	return etalStatus.PADActive;
}

/***************************
 *
 * ETAL_statusSetPADActive
 *
 **************************/
/*!
 * \brief		Informs ETAL that the DAB PAD is active
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetPADActive(tVoid)
{
	etalStatus.PADActive = TRUE;
}

/***************************
 *
 * ETAL_statusResetPADActive
 *
 **************************/
/*!
 * \brief		Informs ETAL that the DAB PAD is no longer active
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusResetPADActive(tVoid)
{
	etalStatus.PADActive = FALSE;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
/***************************
 *
 * ETAL_statusSetHDSupportedServices
 *
 **************************/
/*!
 * \brief		Set the DCOP HD Supported Services status
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusSetHDSupportedServices(etalHDRXSWCnfgTy *rx_sw_cnfg)
{
	(void)OSAL_pvMemoryCopy((tPVoid)etalStatus.HDSupportedServices.RX_SW_Cnfg, (tPVoid)rx_sw_cnfg, sizeof(etalHDRXSWCnfgTy));
}

/***************************
 *
 * ETAL_statusGetHDSupportedServices
 *
 **************************/
/*!
 * \brief		Get the DCOP HD Supported Services status
 * \callgraph
 * \callergraph
 */
tVoid ETAL_statusGetHDSupportedServices(etalHDRXSWCnfgTy *rx_sw_cnfg)
{
	(void)OSAL_pvMemoryCopy((tPVoid)rx_sw_cnfg, (tPVoid)etalStatus.HDSupportedServices.RX_SW_Cnfg, sizeof(etalHDRXSWCnfgTy));
}

/***************************
 *
 * ETAL_statusIsHDMRCSupportedServices
 *
 **************************/
/*!
 * \brief		Get the DCOP HD MRC Supported Services status
 * \callgraph
 * \callergraph
 */
tBool ETAL_statusIsHDMRCSupportedServices(tVoid)
{
	return ((etalStatus.HDSupportedServices.BitField.m_Instance_1_capability == (tU8)1) && 
			((etalStatus.HDSupportedServices.BitField.m_Instance_2_capability == (tU8)1) ||
			(etalStatus.HDSupportedServices.BitField.m_Instance_2_capability == (tU8)2)) &&
			(etalStatus.HDSupportedServices.BitField.m_Digital_FM_Instance_1_Available == (tU8)1) &&
			(etalStatus.HDSupportedServices.BitField.m_Digital_FM_Instance_2_Available == (tU8)1));
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO


