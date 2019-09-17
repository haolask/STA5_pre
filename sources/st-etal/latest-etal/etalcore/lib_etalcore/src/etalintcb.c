//!
//!  \file 		etalintcb.c
//!  \brief 	<i><b> ETAL internal callback management </b></i>
//!  \details   This file provides internal interfaces to dynamically register
//!             callbacks at specific ETAL operational points. These callbacks
//!             are called 'internal' to avoid confusion with the ETAL library
//!             user callbacks.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

/*****************************************************************
| Local types
|----------------------------------------------------------------*/
/*!
 * \struct		etalIntCbPeriodicTy
 * 				Describes a periodical internal callback
 */
typedef struct 
{
/*! TRUE if the entry is used             */	tBool isActive;
/*! Pointer to the callback               */	etalIntCbPeriodicFuncPtrTy periodicFuncPtr;
/*! Receiver or Datapath or other handle  */	ETAL_HANDLE        hGeneric;
/*! When the cb should be invoked next    */	OSAL_tMSecond      nextExpiration;
/*! The delay between invocations in msec */	OSAL_tMSecond      delay;
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
/*! The timer ID */					OSAL_tTimerHandle		TimerId;
/*! Timer Status : enabled/disabled	*/	tBool					TimerIsEnabled;
#endif
} etalIntCbPeriodicTy;

/*!
 * \struct		etalIntCbTy
 * 				Describes an internal callback
 */
typedef struct 
{
/*! TRUE if the entry is used             */	tBool isActive;
/*! When the callback should be invoked   */	etalIntCbCallTy    mode;
/*! Pointer to the callback               */	etalIntCbFuncPtrTy funcPtr;
/*! Receiver or Datapath or other handle  */	ETAL_HANDLE        hGeneric;
/*! Field passed as-is to the callback    */	tU32               context;
} etalIntCbTy;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
/*!
 * \var			etalIntCb
 * 				Array containing the currently registered internal callbacks
 */
static etalIntCbTy         etalIntCb[ETAL_MAX_INTCB];
/*!
 * \var			etalIntCbPeriodic
 * 				Array containing the currently registered periodic internal callbacks
 */
static etalIntCbPeriodicTy etalIntCbPeriodic[ETAL_MAX_INTCB_PERIODIC];

/***************************
 *
 * ETAL_intCbInit
 *
 **************************/
/*!
 * \brief		Initializes the internal callback mechanism
 * \remark		Unique initialization function for both periodic and non-periodic callbacks.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_intCbInit(tVoid)
{
	(void)OSAL_pvMemorySet((tVoid *)etalIntCb, 0x00, sizeof(etalIntCb));
	(void)OSAL_pvMemorySet((tVoid *)etalIntCbPeriodic, 0x00, sizeof(etalIntCbPeriodic));
}

/***************************
 *
 * ETAL_intCbGet
 *
 **************************/
static etalIntCbTy *ETAL_intCbGet(tU32 index)
{
	/* no need to cope with the case when ASSERT not defined
	 * leave the ASSERT anyway as memo */
	ASSERT_ON_DEBUGGING(index < ETAL_MAX_INTCB);
	return &etalIntCb[index];
}

/***************************
 *
 * ETAL_intCbGetPeriodic
 *
 **************************/
static etalIntCbPeriodicTy *ETAL_intCbGetPeriodic(tU32 index)
{
	/* no need to cope with the case when ASSERT not defined
	 * leave the ASSERT anyway as memo */
	ASSERT_ON_DEBUGGING(index < ETAL_MAX_INTCB_PERIODIC);
	return &etalIntCbPeriodic[index];
}

/***************************
 *
 * ETAL_intCbSearch
 *
 **************************/
static etalIntCbTy *ETAL_intCbSearch(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric)
{
	etalIntCbTy *pt;
	tU32 i;
	etalIntCbTy *ret = NULL;

	for (i = 0; i < ETAL_MAX_INTCB; i++)
	{
		pt = ETAL_intCbGet(i);
		if (!pt->isActive)
		{
			continue;
		}
		if ((pt->mode == mode) &&
			(pt->funcPtr == fptr) &&
			(!ETAL_handleIsValid(pt->hGeneric) || 
			(pt->hGeneric == hGeneric)))
		{
			ret = pt;
			goto exit;
		}
	}
	
exit:
	return ret;
}

/***************************
 *
 * ETAL_intCbSearchPeriodic
 *
 **************************/
static etalIntCbPeriodicTy *ETAL_intCbSearchPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric)
{
	etalIntCbPeriodicTy *ptp;
	etalIntCbPeriodicTy *ret = NULL;
	tU32 i;

	for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
	{
		ptp = ETAL_intCbGetPeriodic(i);
		if (!ptp->isActive)
		{
			continue;
		}
		if ((ptp->periodicFuncPtr == fptr) &&
			(ptp->hGeneric == hGeneric))
		{
			ret = ptp;
			goto exit;
		}
	}

exit:
	return ret;
}


#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
/***************************
 *
 * ETAL_intCbTimerCallBack
 *
 **************************/
static void ETAL_intCbTimerCallBack(tPVoid pI_Arg)
{
	// call the periodic function
	//
	etalIntCbPeriodicTy *pt = (etalIntCbPeriodicTy *)pI_Arg;

	ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "ETAL_intCbTimerCallBack: handler %d, pt_function %p, TimerId 0x%x", pt->hGeneric, pt->periodicFuncPtr, pt->isActive, pt->TimerId);	

	if ((NULL != pt)
		&& (NULL != pt->periodicFuncPtr)
		&& (TRUE == pt->isActive))
	{
		pt->periodicFuncPtr(pt->hGeneric);
	}

	return;
}

/***************************
 *
 * ETAL_intCb_PeriodCallbackEnable
 *
 **************************/
/*!
 * \brief			Enable timers for an internal callback
 * \details		Internal periodic callback are enabled/disabled by timer 
 *				This functions parse the periodic check active one which needs
 *				to be enable / disabled
 * \remark		
 * \param[in]	
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_ERROR         - if the system-allowed number of internal callbacks is exceeded
 * \callgraph
 * \callergraph
 */

tVoid ETAL_intCb_PeriodicCallbackEnable()
{
	etalIntCbPeriodicTy *ptp;
	tU32 i;
	tU8	vl_CountEnabling = 0;
	
	for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
	{
		ptp = ETAL_intCbGetPeriodic(i);
		
		if (TRUE == ptp->isActive)
		{
			if (FALSE == ptp->TimerIsEnabled)
			{
				// a need to be enabled
				//

				// Create a tilmer
				// Timer creation 
				// put the detailled element as info
				(void)OSAL_s32TimerCreate(ETAL_intCbTimerCallBack,
		                         (tPVoid) ptp,
		                         &ptp->TimerId);
					
				// set scehduling
				(void)OSAL_s32TimerSetTime(ptp->TimerId, ptp->delay, ptp->delay);
				ptp->TimerIsEnabled = TRUE;
				
				ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "ETAL_intCb_PeriodicCallbackEnable: Enabling 0x%x Timer for Periodic CB, duration %d, rec. %db function 0x%x", ptp->TimerId, ptp->delay, ptp->hGeneric, ptp->periodicFuncPtr );

				vl_CountEnabling++;

			}
			else
			{
				// timer already exist ...
				// nothing to do
				ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "ETAL_intCb_PeriodicCallbackEnable: Timer for Periodic CB %d already exists", i);
			}
		}
	}
	
	// Manage a print for information 
	
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_intCb_PeriodicCallbackEnable: Enabling %d Timers for Periodic CB", vl_CountEnabling);
}	

/***************************
 *
 * ETAL_intCb_PeriodCallbackDisable
 *
 **************************/
/*!
 * \brief			Disable timers for an internal callback
 * \details		Internal periodic callback are enabled/disabled by timer 
 *				This functions parse the periodic check active one which needs
 *				to be enable / disabled
 * \remark		
 * \param[in]	
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_ERROR         - if the system-allowed number of internal callbacks is exceeded
 * \callgraph
 * \callergraph
 */

tVoid ETAL_intCb_PeriodicCallbackDisable()
{
	etalIntCbPeriodicTy *ptp;
	tU32 i;
	tU8	vl_CountDisabling = 0;
	
	for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
	{
		ptp = ETAL_intCbGetPeriodic(i);
		
		if (FALSE == ptp->isActive)
		{
			if (TRUE == ptp->TimerIsEnabled)
			{
				// a need to be disabled
				//
			
				// set scehduling
				(void)OSAL_s32TimerSetTime(ptp->TimerId, 0, 0);

				ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "ETAL_intCb_PeriodicCallbackDisable: disable 0x%x Timer for Periodic CB", ptp->TimerId);

				// delete a tilmer
				// Timer creation 
				// put the detailled element as info
				(void)OSAL_s32TimerDelete(ptp->TimerId);

				ptp->TimerIsEnabled = FALSE;
				vl_CountDisabling++;
				
					
			}
			else
			{
				// strange : timer was not enabled...
				ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "ETAL_intCb_PeriodicCallbackDisable: Timer for Periodic CB %d already disbaled", i);
	
			}
		}
	}

	// Manage a print for information 
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_intCb_PeriodicCallbackDisable: Disabling %d Timers for Periodic CB", vl_CountDisabling);
	
}	


#endif

/***************************
 *
 * ETAL_intCbRegister
 *
 **************************/
/*!
 * \brief		Registers an internal callback
 * \details		ETAL invokes the internal callback when some predefined events
 * 				(depending on *mode*) occur, passing *Generic*, *handle_type* and 
 * 				*context* to the callback.
 * 				For the description of the supported modes, see *mode*'s type definition.
 *
 * 				ETAL uses *mode*, *fptr*, *hGeneric* and *handle_type* to uniquely
 * 				identify the internal callback within the list of registered callbacks
 * 				for deregistration purposes, unless *handle_type* is notAnHandle in which
 * 				case it matches only *mode* and *fptr*.
 * \remark		*fptr* is invoked directly by ETAL from the point defined with *mode*.
 * 				No check is done on the callback execution duration so take care,
 * 				long callback execution time may disrupt the system timings!
 * \param[in]	mode        - the operational mode of the callback; used by ETAL to decide when to
 *                            invoke the callback
 * \param[in]	fptr        - the pointer to the function that must be invoked by ETAL
 * \param[in]	hGeneric    - parameter passed to the *fptr*
 * \param[in]	context     - parameter passed to the *fptr*
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_PARAMETER_ERR - invalid *fptr*
 * \return		ETAL_RET_ERROR         - if the system-allowed number of internal callbacks is exceeded
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_intCbRegister(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric, tU32 context)
{
	etalIntCbTy *pt;
	tU32 i;
	ETAL_STATUS ret = ETAL_RET_ERROR;

	if (fptr == NULL)
	{
		ASSERT_ON_DEBUGGING(0);
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}

	for (i = 0; i < ETAL_MAX_INTCB; i++)
	{
		pt = ETAL_intCbGet(i);
		if (pt->isActive)
		{
			if ((hGeneric == pt->hGeneric) &&
				(fptr == pt->funcPtr) &&
				(mode == pt->mode))
			{
				//callback is already register for same receiver, same info : do nothing
				ret = ETAL_RET_SUCCESS;
				goto exit;
			}
		}
		else
		{
			pt->isActive = TRUE;
			pt->mode = mode;
			pt->funcPtr = fptr;
			pt->hGeneric = hGeneric;
			pt->context = context;
			ret = ETAL_RET_SUCCESS;
			goto exit;
		}
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_intCbRegisterPeriodic
 *
 **************************/
/*!
 * \brief		Registers an internal periodic callback
 * \details		A periodic callback is one invoked at fixed time interval, specified
 * 				in parameter *delay_ms*.  ETAL invokes the internal callback passing 
 * 				*hGeneric* and *handle_type* as parameters.
 *
 * 				ETAL uses *fptr*, and *handle_type* and *hGeneric* to uniquely
 * 				identify the internal callback within the list of registered callbacks
 * 				for deregistration purposes.
 *
 * 				*delay_ms* must be grater than #ETAL_CONTROL_THREAD_SCHEDULING
 * 				and should be a multiple of #ETAL_CONTROL_THREAD_SCHEDULING because
 * 				periodic internal callbacks are invoked from the Control Thread which
 * 				is scheduled every #ETAL_CONTROL_THREAD_SCHEDULING ms.
 *
 * 				The internal callback will be invoked when the time interval since
 * 				last invocation becomes be grater or equal to the requested *delay_ms*.
 * \remark		*fptr* is invoked directly by ETAL from the point defined with *mode*.
 * 				No check is done on the callback execution duration so take care,
 * 				long callback execution time may disrupt the system timings!
 * \param[in]	fptr        - the pointer to the function that must be invoked by ETAL
 * \param[in]	hGeneric    - parameter passed to the *fptr*
 * \param[in]	delay_ms    - defines the delay in ms between successive invocations
 * 				              Must be greater than #ETAL_CONTROL_THREAD_SCHEDULING.
 * 				              Should be a multiple of #ETAL_CONTROL_THREAD_SCHEDULING.
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_PARAMETER_ERR - invalid *fptr* or illegal delay
 * \return		ETAL_RET_ERROR         - if the system-allowed number of internal callbacks is exceeded
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_intCbRegisterPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric, tU32 delay_ms)
{
	etalIntCbPeriodicTy *ptp;
	tU32 i;
	ETAL_STATUS ret = ETAL_RET_ERROR;

	if (delay_ms < ETAL_CONTROL_THREAD_SCHEDULING)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "Illegal delay to ETAL_intCbRegister (%d ms)", delay_ms);
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}
	if (fptr == NULL)
	{
		ASSERT_ON_DEBUGGING(0);
		ret = ETAL_RET_PARAMETER_ERR;
		goto exit;
	}

	for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
	{
		ptp = ETAL_intCbGetPeriodic(i);
		if (ptp->isActive)
		{
			continue;
		}

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		// indicate that the timer is not enabled
		if (TRUE == ptp->TimerIsEnabled)
		{
			// function is not active
			// but timer not yet disabled
			// ie disabling waiting
			// continue
			continue;
		}
#endif

		ptp->isActive = TRUE;
		ptp->periodicFuncPtr = fptr;
		ptp->hGeneric = hGeneric;
		ptp->nextExpiration = OSAL_ClockGetElapsedTime() + delay_ms;
		ptp->delay = delay_ms;
		ret = ETAL_RET_SUCCESS;

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		// indicate that the timer is not enabled
		ptp->TimerIsEnabled = FALSE;
		
		//Post an event to create the timer
		// so that it runs in appropriate etal thread (control)
		// 
		ETAL_control_PeriodicCallbackStartEvent();

#endif // #ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		goto exit;
	}

exit:
	return ret;
}


/***************************
 *
 * ETAL_intCbIsRegisteredPeriodic
 *
 **************************/
/*!
 * \brief		Checks if an internal periodic callback is registered
 * \param[in]	fptr        - the pointer to the function that must be invoked by ETAL
 * \param[in]	hGeneric    - parameter passed to the *fptr*
 * \return		TRUE  - the callback is registered
 * \return		FALSE - the callback is **not** registered
 * \callgraph
 * \callergraph
 */
tBool ETAL_intCbIsRegisteredPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric)
{
	return (ETAL_intCbSearchPeriodic(fptr, hGeneric) != NULL);
}


/***************************
 *
 * ETAL_intCbIsRegistered
 *
 **************************/
/*!
 * \brief       Checks if an internal callback is registered
 * \param[in]   mode        - the operational mode of the callback; used by ETAL to decide when to
 *                            invoke the callback
 * \param[in]   fptr        - the pointer to the function that must be invoked by ETAL
 * \param[in]   hGeneric    - parameter passed to the *fptr*
 * \return      TRUE  - the callback is registered
 * \return      FALSE - the callback is **not** registered
 * \callgraph
 * \callergraph
 */
tBool ETAL_intCbIsRegistered(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric)
{
    return (ETAL_intCbSearch(mode, fptr, hGeneric) != NULL);
}


/***************************
 *
 * ETAL_intCbDeregister
 *
 **************************/
/*!
 * \brief		Removes the callback from the list of internal callbacks
 * \details		This function searches and removes only the first callback that
 * 				matches the passed *mode*, *fptr*, *hGeneric* and *handle_type*.
 * \remark		*mode*, *fptr*, *hGeneric* and *handle_type* must be the same values
 * 				used at callback registration.
 * \param[in]	mode        - the operational mode of the callback; used by ETAL to decide when to
 *                            invoke the callback
 * \param[in]	fptr        - the pointer to the function that must be invoked by ETAL
 * \param[in]	hGeneric    - parameter passed to the *fptr*
 * \return		ETAL_RET_SUCCESS
 * 				ETAL_RET_ERROR   - the internal callback was not found in the list of registered callbacks
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_intCbDeregister(etalIntCbCallTy mode, etalIntCbFuncPtrTy fptr, ETAL_HANDLE hGeneric)
{
	etalIntCbTy *pt;
	ETAL_STATUS ret = ETAL_RET_ERROR;

	pt = ETAL_intCbSearch(mode, fptr, hGeneric);
	if (pt)
	{
		(void)OSAL_pvMemorySet((tVoid *)pt, 0x00, sizeof(etalIntCbTy));
		ret = ETAL_RET_SUCCESS;
	}

	return ret;
}

/***************************
 *
 * ETAL_intCbDeregisterDatapath
 *
 **************************/
/*!
 * \brief		Remove all the non-periodic internal callbacks associated with a Datapath
 * \remark		ETAL has no way to distinguish handle types, so it trusts the *handle_type* field
 * 				passed at internal callback registration time.
 * \param[in]	hDatapath - Datapath handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_intCbDeregisterDatapath(ETAL_HANDLE hDatapath)
{
	etalIntCbTy *ptp;
	tU32 i;

	if (!ETAL_handleIsDatapath(hDatapath))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		for (i = 0; i < ETAL_MAX_INTCB; i++)
		{
			ptp = ETAL_intCbGet(i);
			if (!ptp->isActive)
			{
				continue;
			}
			if (ptp->hGeneric == hDatapath)
			{
				(void)OSAL_pvMemorySet((tVoid *)ptp, 0x00, sizeof(etalIntCbTy));
			}
		}
	}

	return;
}

/***************************
 *
 * ETAL_intCbDeregisterPeriodic
 *
 **************************/
/*!
 * \brief		Removes the periodic callback from the list of internal callbacks
 * \details		This function searches and removes only the first callback that
 * 				matches the passed *fptr*, *handle_type* and *hGeneric*
 * \remark		*fptr*, *handle_type* and *hGeneric* must be the same values
 * 				used at callback registration.
 * \param[in]	fptr        - the pointer to the function that must be invoked by ETAL
 * \param[in]	hGeneric    - parameter passed to the *fptr*
 * \return		ETAL_RET_SUCCESS
 * 				ETAL_RET_ERROR   - the internal callback was not found in the list of registered callbacks
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_intCbDeregisterPeriodic(etalIntCbPeriodicFuncPtrTy fptr, ETAL_HANDLE hGeneric)
{
	etalIntCbPeriodicTy *ptp;
	ETAL_STATUS ret = ETAL_RET_ERROR;

	ptp = ETAL_intCbSearchPeriodic(fptr, hGeneric);
	if (ptp)
	{
		// reset field
		ptp->isActive = FALSE;
		ptp->hGeneric = ETAL_INVALID_HANDLE;
		ptp->nextExpiration = 0;
		ptp->delay = 0;
		ptp->periodicFuncPtr = NULL;

		// do not reset timer info, it will be reset when disabled
		//
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		// Post an event to deregister periodic
		// 
		ETAL_control_PeriodicCallbackStopEvent();
#endif
		ret = ETAL_RET_SUCCESS;
	}
	

	return ret;
}


/***************************
 *
 * ETAL_intCbDeregisterPeriodicDatapath
 *
 **************************/
/*!
 * \brief		Remove all the periodic internal callbacks associated with a Datapath
 * \remark		ETAL has no way to distinguish handle types, so it trusts the
 * 				*handle_type* field passed at internal callback registration time.
 * \param[in]	hDatapath - Datapath handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_intCbDeregisterPeriodicDatapath(ETAL_HANDLE hDatapath)
{
	etalIntCbPeriodicTy *ptp;
	tU32 i;

	if (!ETAL_handleIsDatapath(hDatapath))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
		{
			ptp = ETAL_intCbGetPeriodic(i);
			if (!ptp->isActive)
			{
				continue;
			}
			if (ptp->hGeneric == hDatapath)
			{
				(void)OSAL_pvMemorySet((tVoid *)ptp, 0x00, sizeof(etalIntCbPeriodicTy));
			}
		}
	}
	
	return;
}

/***************************
 *
 * ETAL_intCbDeregisterPeriodicReceiver
 *
 **************************/
/*!
 * \brief		Remove all the periodic internal callbacks associated with a Receiver
 * \remark		ETAL has no way to distinguish handle types, so it trusts the
 * 				*handle_type* field passed at internal callback registration time.
 * \param[in]	hReceiver - Receiver handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_intCbDeregisterPeriodicReceiver(ETAL_HANDLE hReceiver)
{
	etalIntCbPeriodicTy *ptp;
	tU32 i;

	for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
	{
		ptp = ETAL_intCbGetPeriodic(i);
		
		if (!ptp->isActive)
		{
			continue;
		}
		if (ptp->hGeneric == hReceiver)
		{
			// dereigster
			// reset field
			ptp->isActive = FALSE;
			ptp->hGeneric = ETAL_INVALID_HANDLE;
			ptp->nextExpiration = 0;
			ptp->delay = 0;
			ptp->periodicFuncPtr = NULL;
			
			// do not reset timer info, it will be reset when disabled
			//
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
			// Post an event to deregister periodic
			// 
			ETAL_control_PeriodicCallbackStopEvent();
#endif

		}
	}
}

/***************************
 *
 * ETAL_intCbScheduleCallbacks
 *
 **************************/
/*!
 * \brief		Invoke all the callbacks of a given type
 * \details		This function should be called from one of the callback invocation points associated with
 * 				*mode* values: whenever defining a new #etalIntCbCallTy value, the implementer should also identify
 * 				the appropriate point in the code where to insert a call to this function.
 *
 * 				The parameters *receiver*, *handle_type*, *param* and *param_len* are passed to the callback.
 * 				The parameters *receiver*, *handle_type* are also used to search the correct
 * 				callback in the callback array #etalIntCb: the function considers only the entries that:
 * 				- were registered with ETAL_INVALID_HANDLE handle
 * 				- match the *receiver* and *handle_type*
 *
 * \param[in]	hGeneric    - a generic handle
 * \param[in]	mode        - the operational mode of the callback; used by ETAL to decide when to
 * 				              invoke the callback
 * \param[in]	param       - pointer to a data buffer passed as-is to the callback
 * \param[in]	param_len   - the size of the data buffer passer to the callback
 * \callgraph
 * \callergraph
 */
tVoid ETAL_intCbScheduleCallbacks(ETAL_HANDLE hGeneric, etalIntCbCallTy mode, tVoid *param, tU32 param_len)
{
	etalIntCbTy *pt;
	tU32 i;

	for (i = 0; i < ETAL_MAX_INTCB; i++)
	{
		pt = ETAL_intCbGet(i);
		if ((pt->isActive) &&
			(pt->mode == mode))
		{
			if (pt->funcPtr)
			{
				// check if registered for this receiver
				// either registered for any receiver
				// either filter on the receiver
				if ((!ETAL_handleIsValid(pt->hGeneric)) ||
					((pt->hGeneric == hGeneric) && ETAL_handleIsReceiver(hGeneric)))
				{
					pt->funcPtr(hGeneric, param, param_len, pt->context);
				}
				else
				{
					// no client registered
				}
			}
			else
			{
				ASSERT_ON_DEBUGGING(0);
			}
		}
	}
}

/***************************
 *
 * ETAL_intCbSchedulePeriodicCallbacks
 *
 **************************/
/*!
 * \brief		Invokes periodic callbacks for which the timeout is expired
 * \details		Normally invoked by the #ETAL_Control_ThreadEntry
 * \remark		This function changes the global ETAL status so must be called
 * 				with system locked.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_intCbSchedulePeriodicCallbacks(void)
{
	etalIntCbPeriodicTy *ptp;
	tU32 i;
	OSAL_tMSecond now;

	now = OSAL_ClockGetElapsedTime();
	for (i = 0; i < ETAL_MAX_INTCB_PERIODIC; i++)
	{
		ptp = ETAL_intCbGetPeriodic(i);
		if ((ptp->isActive) &&
			(ptp->nextExpiration <= now))
		{
			/*
			 * timer expired, time to schedule
			 */
			ptp->nextExpiration = now + ptp->delay;
			ptp->periodicFuncPtr(ptp->hGeneric);
		}
	}
}

