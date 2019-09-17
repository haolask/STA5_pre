//!
//!  \file 		etalcallback.c
//!  \brief 	<i><b> ETAL callback Thread </b></i>
//!  \details   Thread responsible for invoking the user callbacks.
//!				This file implements the thread and the internal interfaces provided
//!				by ETAL to support user notification handler. The user notification
//!				handler (or callback) is registered at #etal_initialize invocation. In practice it
//!				is a user-provided function that ETAL calls whenever there is the need to notify
//!				an Event to the user.
//!
//!				Since the callback is executed in the ETAL context and ETAL has no control on the
//!				duration of the callback it would be possible for a badly coded user callback to
//!				disrupt the ETAL time-critical processing. To avoid this ETAL implements a dedicated
//!				Callback Thread whose only task is to call user callbacks (this way if the callback
//!				hangs it will only affect the Callback Thread, not the whole ETAL).
//!
//!				Internally ETAL modules can request the invocation of the user callback
//!				by calling the #ETAL_callbackInvoke or #ETAL_sendCommunicationErrorEvent functions.
//!				Both functions just push an invocation request onto the Callback FIFO
//!				and return immediately.
//!
//!				The Callback FIFO is emptied asynchronously by the #ETAL_CallbackHandler_ThreadEntry
//!				which implements the Callback Thread.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	#include "common_trace.h"
#endif

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

/*****************************************************************
| local types
|----------------------------------------------------------------*/
/*!
 * \struct	ETAL_cbMaxSizeTy
 * 			Callback FIFO elements must be dimensioned to hold the largest possible
 * 			message that can be generated by ETAL. To let the compiler calculate
 * 			this maximum size each possible message that can be sent to the
 * 			Callback Thread must be listed in the #ETAL_cbMaxSizeTy union.
 */
typedef union
{
	/*! parameter of the #ETAL_INFO_TUNE event */
	EtalTuneStatus       tuneStatus;
	/*! parameter of the #ETAL_INFO_SEEK event */
	EtalSeekStatus 		 seekStatus;
	/*! parameter of Quality callbacks for CMOST and HDRadio DCOP
	 * \see ETAL_CallbackHandler_ThreadEntry */
	etalQualityCbTy      etalQuality;
#if defined (CONFIG_ETALTML_HAVE_LEARN)
	/*! parameter of the #ETAL_INFO_LEARN event */
	EtalLearnStatusTy    etalLearn;
#endif
#if defined (CONFIG_ETALTML_HAVE_SCAN)
	/*! parameter of the #ETAL_INFO_SCAN event */
	EtalScanStatusTy     etalScan;
#endif
	/*! parameter of the #ETAL_ERROR_COMM_FAILED event */
	EtalCommErrStatus    etalCommError;
#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
	/*! parameter of the #ETAL_INFO_SEAMLESS_ESTIMATION_END event */
	EtalSeamlessEstimationStatus seamlessEstimationStatus;
	/*! parameter of the #ETAL_INFO_SEAMLESS_SWITCHING_END event */
	EtalSeamlessSwitchingStatus  seamlessSwitchingStatus;
#endif
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
	/*! parameter of the #ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO event and
	 *                   #ETAL_INFO_TUNE_SERVICE_ID event */
	EtalTuneServiceIdStatus	serviceFollowingStatus;
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	/*! parameter of Quality callback for DAB DCOP
	 * \remark	AM/FM info is normally provided by the CMOST device through the #etalQualityCbTy;
	 * 			this structure is here for the MDR3 case, where the DCOP supports both DAB and AM/FM
	 * \see 	ETAL_qualityHandler_MDR */
	tU8             getMonitorAMFM[ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET + ETAL_MDR_GETMONITORAMFMINFO_LEN];
	/*! parameter of Quality callback for DAB DCOP
	 * \see		ETAL_qualityHandler_MDR */
	tU8             getMonitorDAB [ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET + ETAL_MDR_GETMONITORDABINFO_LEN];
#endif

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	EtalRDSStrategyStatus rdsStrategyStatus;
#endif
	/*! parameter of the #ETAL_RECEIVER_ALIVE_ERROR event */
	ETAL_HANDLE hReceiver;
} ETAL_cbMaxSizeTy;

/*!
 * \struct	ETAL_cbFifoElemTy
 * 			Callback FIFO element
 */
typedef struct
{
	/*! Describes the type of message; used to decide which callback to invoke */
	etalCallbackTy   msgType;
	/*! Event that originated the callback; this is passed to the callback and not
	 *  used by the callback handler
	 */
	ETAL_EVENTS      event;
	/*! Parameter passed to the callback as-is */
	ETAL_cbMaxSizeTy param;
	/*! Size in bytes of *param* */
	tU32             paramSize;
} ETAL_cbFifoElemTy;

/*!
 * \struct	ETAL_cbStatusTy
 * 			The FIFO storage
 */
typedef struct
{
	/*! The FIFO meta-information */
	COMMON_fifoStatusTy cbFifo;
	/*! The FIFO information */
	ETAL_cbFifoElemTy   cbFifoStorage[ETAL_CALLBACK_FIFO_ELEMENTS];
	/*! Auxiliary buffer used to pass data to the callback and release the FIFO element */
	ETAL_cbMaxSizeTy    cbStorage;
} ETAL_cbStatusTy;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
/*!
 * \var		Etal_callbackStatus
 * 			The Callback thread private status. There is one status per
 * 			callback thread.
 */
static ETAL_cbStatusTy Etal_callbackStatus[ETAL_CALLBACK_HANDLERS_NUM];

/*****************************************************************
| function definition
|----------------------------------------------------------------*/

/***************************
 *
 * ETAL_callbackFifoReset
 *
 **************************/
/*!
 * \brief		Initializes the Callback FIFO
 * \param[in]	pfifo - pointer of the FIFO to be initialized
 * \return		OSAL_ERROR - error during the creation of the COMMON fifo used by the Callback FIFO
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_callbackFifoReset(ETAL_cbStatusTy *pfifo)
{
	(void)OSAL_pvMemorySet((tVoid *)pfifo->cbFifoStorage, 0x00, sizeof(ETAL_cbFifoElemTy) * ETAL_CALLBACK_FIFO_ELEMENTS);
	(void)OSAL_pvMemorySet((tVoid *)&pfifo->cbStorage, 0x00, sizeof(ETAL_cbMaxSizeTy));
	if (COMMON_fifoInit(&pfifo->cbFifo, ETAL_FIFO_CALLBACK_NAME, ETAL_CALLBACK_FIFO_ELEMENTS) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_callbackFifoPush
 *
 **************************/
/*!
 * \brief		Pushes a status event or quality event onto the Callback FIFO.
 * \details		The data pointed by *param* is copied to a local storage,
 * 				so *param* can be de-allocated after return from this function.
 * \param[in]	pfifo - pointer to the FIFO to be used
 * \param[in]	type - the type of message (status event or quality event)
 * \param[in]	event - the event code (this is just passed as-is to the push function=
 * \param[in]	param - the message
 * \param[in]	param_size - the size of *param*. The max supported size is defined by
 * 				             the size of a union (#ETAL_cbMaxSizeTy) listing all the
 * 				             possible users of this FIFO.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - *param_size* is larger than the max supported size or
 * 				             the FIFO is full. In both cases the message is discarded.
 * \callgraph
 * \callergraph
 */
static tSInt /*@alt void@*/ETAL_callbackFifoPush(ETAL_cbStatusTy *pfifo, etalCallbackTy type, ETAL_EVENTS event, tVoid *param, tU32 param_size)
{
	ETAL_cbFifoElemTy *elem;
	tS16 new_write_ptr;

	if (param_size > sizeof(ETAL_cbMaxSizeTy))
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Message too long in Callback fifo (req %d, max %d)", param_size, sizeof(ETAL_cbMaxSizeTy));
		return OSAL_ERROR;
	}

	COMMON_fifoGetLock(&pfifo->cbFifo);

	if (COMMON_fifoPush(&pfifo->cbFifo, &new_write_ptr) != OSAL_OK)
	{
		// FIFO overflow
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL, "Callback FIFO overflow");
		COMMON_fifoReleaseLock(&pfifo->cbFifo);
		return OSAL_ERROR;
	}
	elem = &pfifo->cbFifoStorage[new_write_ptr];
	if ((param_size > 0) && (param != NULL))
	{
		(void)OSAL_pvMemoryCopy((tVoid *)&elem->param, (tPCVoid)param, param_size);
	}
	elem->msgType = type;
	elem->event = event;
	elem->paramSize = param_size;

	COMMON_fifoReleaseLock(&pfifo->cbFifo);

	return OSAL_OK;
}

/***************************
 *
 * ETAL_callbackFifoPop
 *
 **************************/
/*!
 * \brief		Extracts a status event or quality event from the Callback FIFO.
 * \details		
 * \remark		
 * \param[in]	pfifo - pointer to the FIFO to be used
 * \param[out]	type - pointer to location where the function stores
 * 				       the type of message (status event or quality event)
 * \param[out]	event - pointer to location where the function stores
 * 				        the event code (or whatever data was passed to #ETAL_callbackFifoPush
 * 				        in the *event* parameter)
 * \param[out]	param - pointer to location where the function stores
 * 				        the message. This should point to a buffer as large
 * 				        as sizeof(#ETAL_cbMaxSizeTy)
 * \param[out]	param_size - pointer to location where the function stores
 * 				        the size of the data stored to *param*
 * \return		OSAL_OK - message extracted
 * \return		OSAL_ERROR - the extracted message size is larger than the max supported size or
 * 				             the FIFO is full. In both cases the message is discarded.
 * 				             The first case indicates memory corruption and is not
 * 				             a normal condition.
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_callbackFifoPop(ETAL_cbStatusTy *pfifo, etalCallbackTy *type, ETAL_EVENTS *event, tVoid *param, tU32 *param_size)
{
	tSInt retval = OSAL_OK;
	tU32 size;
	ETAL_cbFifoElemTy *elem;
	tS16 new_read_ptr;

	COMMON_fifoGetLock(&pfifo->cbFifo);

	if (COMMON_fifoPop(&pfifo->cbFifo, &new_read_ptr) != OSAL_OK)
	{
		// FIFO empty condition
		COMMON_fifoReleaseLock(&pfifo->cbFifo);
		return OSAL_ERROR;
	}

	elem = &pfifo->cbFifoStorage[new_read_ptr];
	size = elem->paramSize;
	if (size > 0)
	{
		if (size > sizeof(ETAL_cbMaxSizeTy))
		{
			ASSERT_ON_DEBUGGING(0);
			retval = OSAL_ERROR;
		}
		else
		{
			(void)OSAL_pvMemoryCopy((tVoid *)param, (tPCVoid)&elem->param, size);
		}
	}
	*type = elem->msgType;
	*event = elem->event;
	*param_size = size;

	COMMON_fifoReleaseLock(&pfifo->cbFifo);

	return retval;
}

/***************************
 *
 * ETAL_callbackInit
 *
 **************************/
/*!
 * \brief		Initializes the Callback handler storage
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_callbackInit(tVoid)
{
	tU32 i;

	for (i = 0; i < ETAL_CALLBACK_HANDLERS_NUM; i++)
	{
		if (ETAL_callbackFifoReset(&Etal_callbackStatus[i]) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_callbackDeinit
 *
 **************************/
/*!
 * \brief		De-initializes the Callback handler storage
 * \details		Mainly destroys the semaphores created for the FIFO.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
tSInt ETAL_callbackDeinit(tVoid)
{
	tSInt ret = OSAL_OK;
	tU32 i;

	for (i = 0; i < ETAL_CALLBACK_HANDLERS_NUM; i++)
	{
		COMMON_fifoDeinit(&Etal_callbackStatus[i].cbFifo, ETAL_FIFO_CALLBACK_NAME);
	}

	return ret;
}

/***************************
 *
 * ETAL_sendCommunicationErrorEvent
 *
 **************************/
/*!
 * \brief		Generates a Communication Error event
 * \details		This function must be called by internal ETAL interfaces to request the
 * 				invocation of the user notification handler (a.k.a. callback) for an
 * 				#ETAL_ERROR_COMM_FAILED event. It formats the #EtalCommErrStatus buffer
 * 				that will be passed to the callback	and inserts the event in the callback
 * 				handler FIFO using the generic #ETAL_callbackInvoke	interface.
 * \remark		If the user did not register any event handler the function outputs a message
 * 				on the terminal.
 * \param[in]	hReceiver - Receiver handle
 * \param[in]	err       - the decoded error
 * \param[in]	err_raw   - the raw error code
 * \param[in]	buf       - the message that originated the error; this buffer is actually copied
 * 				            as-is to the #EtalCommErrStatus buffer, the only check performed is if the
 * 				            buffer fits in the storage available in the #EtalCommErrStatus
 * \param[in]	buf_len   - the size of the *buf*; this value is copied to #EtalCommErrStatus size field;
 * 				            if the size of the buffer passed in *buf_len* is larger than available
 * 				            (#ETAL_MAX_COMM_ERR_MSG) the function copies only the max available
 * 				            locations and puts -ETAL_MAX_COMM_ERR_MSG in the #EtalCommErrStatus size field.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_sendCommunicationErrorEvent(ETAL_HANDLE hReceiver, EtalCommErr err, tU32 err_raw, tU8 *buf, tU32 buf_len)
{
	EtalCommErrStatus comm_err_status;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
	if (ETAL_statusHardwareAttrGetNotifycb() == NULL)
	{
		// Ensure some feedback is given in case there is no user callback
		ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Communication failure: receiver %d err \"%s\" (%d), ret status 0x%x", hReceiver, ETAL_commErrToString(err), err, err_raw);
	}
#endif

	comm_err_status.m_commErr = err;
	comm_err_status.m_commErrRaw = err_raw;
	comm_err_status.m_commErrReceiver = hReceiver;
	if ((buf != NULL) && (ETAL_MAX_COMM_ERR_MSG > 0))
	{
		if (buf_len <= ETAL_MAX_COMM_ERR_MSG)
		{
			(void)OSAL_pvMemoryCopy((tVoid *)&comm_err_status.m_commErrBuffer, (tPCVoid)buf, buf_len);
			comm_err_status.m_commErrBufferSize = (tS32)buf_len;
		}
		else
		{
			(void)OSAL_pvMemoryCopy((tVoid *)&comm_err_status.m_commErrBuffer, (tPCVoid)buf, ETAL_MAX_COMM_ERR_MSG);
			comm_err_status.m_commErrBufferSize = -ETAL_MAX_COMM_ERR_MSG; // negative value notifies the buffer was truncated
		}
	}
	else
	{
		(void)OSAL_pvMemorySet((tVoid *)&comm_err_status.m_commErrBuffer, 0x00, ETAL_MAX_COMM_ERR_MSG);
		comm_err_status.m_commErrBufferSize = 0;
	}
	ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_ERROR_COMM_FAILED, (tVoid *)&comm_err_status, sizeof(comm_err_status));
}

/***************************
 *
 * ETAL_callbackInvoke
 *
 **************************/
/*!
 * \brief		Indirectly invokes the user notification callback
 * \details		Function called by all ETAL API functions to indirectly
 * 				invoke the callbacks provided by the ETAL API user.
 * 				Puts the data into a FIFO processed by the Callback thread.
 *
 * 				ETAL supports two broad categories of Callbacks:
 * 				- event callbacks
 * 				- quality callback
 *
 * 				Event callbacks are used to notify the API user of special
 * 				condition. It is set up by etal_initialize through the #m_cbNotify
 * 				field of the *init_params* parameter.
 *
 * 				Quality callbacks are used to notify the API user of a change
 * 				in some quality parameter for which a Monitor was registered.
 * 				Registration is done through the #etal_config_reception_quality_monitor
 * 				API.
 *
 * 				The *param* data buffer is copied to a Datahandler local storage
 * 				so the caller can re-use it after this function returns.
 *
 * 				The functions support multiple callback handlers.
 * \param[in]	handler_index - ranges 0..#ETAL_CALLBACK_HANDLERS_NUM and indicates
 * 				                which handler to use
 * \param[in]	msg_type      - The type of event, will be used to decide which handler to invoke
 * \param[in]	event         - The event
 * \param[in]	param         - Array of bytes whose interpretation depends on the *event* and *msg_type*.
 * 				                The description of the buffer interpretation is provided in the
 * 				                ETAL API specification.
 * \param[in]	param_len     - The size in bytes of *param*
 * \callgraph
 * \callergraph
 */
tVoid ETAL_callbackInvoke(tU32 handler_index, etalCallbackTy msg_type, ETAL_EVENTS event, tVoid *param, tU32 param_len)
{
	EtalSeekStatus* extAutoSeekStatus;
	EtalSeamlessEstimationStatus* extSeamlessEstimationStatus;
	EtalSeamlessSwitchingStatus* extSeamlessSwitchingStatus;
	EtalLearnStatusTy* extLearnStatus;
	EtalScanStatusTy* extScanStatus;
	EtalTuneStatus* extTuneStatus;
	etalAutoNotificationStatusTy* extDabAutonotificationStatus;

	if (handler_index >= ETAL_CALLBACK_HANDLERS_NUM)
	{
		ASSERT_ON_DEBUGGING(0);
		return;
	}

	switch(event)
	{
		case ETAL_INFO_TUNE:
			extTuneStatus = (EtalTuneStatus*)param;
			if (TRUE == ETAL_receiverIsSpecialInProgress(extTuneStatus->m_receiverHandle, cmdSpecialExternalTuneRequestInProgress))
			{
				(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			}
			break;

		case ETAL_INFO_DAB_AUTONOTIFICATION:
			extDabAutonotificationStatus = (etalAutoNotificationStatusTy*)param;

			switch(extDabAutonotificationStatus->type)
			{
				case autoDABAnnouncementSwitching:
					if (TRUE == ETAL_receiverIsSpecialInProgress(extDabAutonotificationStatus->receiverHandle, cmdSpecialExternalDABAnnouncementRequestInProgress))
					{
						(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
					}
					break;

				case autoDABAnnouncementSwitchingRaw:
					if (TRUE == ETAL_receiverIsSpecialInProgress(extDabAutonotificationStatus->receiverHandle, cmdSpecialExternalDABAnnouncementRequestInProgress))
					{
						(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
					}
					break;

				case autoDABReconfiguration:
					if (TRUE == ETAL_receiverIsSpecialInProgress(extDabAutonotificationStatus->receiverHandle, cmdSpecialExternalDABReconfigurationRequestInProgress))
					{
						(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
					}
					break;

				case autoDABStatus:
					if (TRUE == ETAL_receiverIsSpecialInProgress(extDabAutonotificationStatus->receiverHandle, cmdSpecialExternalDABStatusRequestInProgress))
					{
						(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
					}
					break;

				case autoDABDataStatus:
					if (TRUE == ETAL_receiverIsSpecialInProgress(extDabAutonotificationStatus->receiverHandle, cmdSpecialExternalDABDataStatusRequestInProgress))
					{
						(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
					}
					break;

				default:
					(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
					break;
			}
			break;

		case ETAL_INFO_SEEK:
			extAutoSeekStatus = (EtalSeekStatus*)param;
			if (TRUE == ETAL_receiverIsSpecialInProgress(extAutoSeekStatus->m_receiverHandle, cmdSpecialExternalSeekRequestInProgress))
			{
				(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			}
			break;

		case ETAL_INFO_SEAMLESS_ESTIMATION_END:
			extSeamlessEstimationStatus = (EtalSeamlessEstimationStatus*)param;
			if (TRUE == ETAL_receiverIsSpecialInProgress(extSeamlessEstimationStatus->m_receiverHandle, cmdSpecialExternalSeamlessEstimationRequestInProgress))
			{
				(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			}
			break;

		case ETAL_INFO_SEAMLESS_SWITCHING_END:
			extSeamlessSwitchingStatus = (EtalSeamlessSwitchingStatus*)param;
			if (TRUE == ETAL_receiverIsSpecialInProgress(extSeamlessSwitchingStatus->m_receiverHandle, cmdSpecialExternalSeamlessSwitchingRequestInProgress))
			{
				(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			}
			break;

		case ETAL_INFO_LEARN:
			extLearnStatus = (EtalLearnStatusTy*)param;
			if (TRUE == ETAL_receiverIsSpecialInProgress(extLearnStatus->m_receiverHandle, cmdSpecialExternalLearnRequestInProgress))
			{
				(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			}
			break;

		case ETAL_INFO_SCAN:
			extScanStatus = (EtalScanStatusTy*)param;
			if (TRUE == ETAL_receiverIsSpecialInProgress(extScanStatus->m_receiverHandle, cmdSpecialExternalScanRequestInProgress))
			{
				(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			}
			break;
		default:
			(void)ETAL_callbackFifoPush(&Etal_callbackStatus[handler_index], msg_type, event, param, param_len);
			break;
	}
}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_extractFilter_MDR
 *
 **************************/
static tSInt ETAL_extractFilter_MDR(tU8 *cmd, tU32 len, etalFilterMaskTy *filterp)
{
	tU32 filter_offset;
	tU32 expected_command_len;

	if (ETAL_MDR_COMMAND_ID(cmd) == ETAL_MDR_GETMONITORDABINFO)
	{
		// FILTER MASK are the last bytes of the payload
		/* Due to change of payload length in DCOP FW, we have to support different payload lengths */
		//filter_offset = ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET + ETAL_MDR_GETMONITORDABINFO_LEN - 2;
		//expected_command_len = ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET + ETAL_MDR_GETMONITORDABINFO_LEN;
		filter_offset = len - 2;
		expected_command_len = len;
	}
	else // (ETAL_MDR_COMMAND_ID(cmd) == ETAL_MDR_GETMONITORAMFMINFO)
	{
		// FILTER MASK are the last bytes of the payload
		filter_offset = ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET + ETAL_MDR_GETMONITORAMFMINFO_LEN - 2;
		expected_command_len = ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET + ETAL_MDR_GETMONITORAMFMINFO_LEN;
	}

	if (len != expected_command_len)
	{
		return OSAL_ERROR;
	}

	*filterp = (((tU16)cmd[filter_offset] << 8) & 0xFF00) | (cmd[filter_offset + 1] & 0xFF);
	return OSAL_OK;
}

/***************************
 *
 * ETAL_searchMonitor_MDR
 *
 **************************/
/*!
 * \brief		
 * \details		Returns the Monitor associated with the filter that caused the DABMW notification
 * 				contained in *cmd*. Since several filters can be associated with a single *cmd*,
 * 				parameter *filter_statusp* specifies which of the filters to consider. It is updated
 * 				by the function on every call.
 *
 *	 			To ensure all the monitors are extracted from a *cmd*, the function must be invoked
 * 				repeatedly for the same *cmd* buffer until it returns #ETAL_INVALID_HANDLE.
 * \param[in]	cmd - the command buffer to consider
 * \param[in]	len - the size of the command buffer, in bytes
 * \param[in,out] filter_statusp - function status, must be initialized by calling #ETAL_initFilterMask_MDR
 * 				                   the first time the function is called on a *cmd* buffer, then passed
 * 				                   to the function every time it is called on the same *cmd* buffer.
 * \return		a valid monitor handle and the function updates *filter_statusp* with the filters still
 * 				to be considered if the monitor is found
 * \return		#ETAL_INVALID_HANDLE if no monitor is found
 * \callgraph
 * \callergraph
 * \todo		describe the function parameter *cmd* format
 */
static ETAL_HANDLE ETAL_searchMonitor_MDR(tU8 *cmd, tU32 len, etalFilterMaskTy *filter_statusp)
{
	ETAL_HANDLE hMonitor;
	ETAL_HANDLE hMonitor_ret = ETAL_INVALID_HANDLE;
	etalFilterMaskTy filter_recv = 0, filter_recv_tmp;
	etalMonitorTy *monp = NULL;
	ETAL_HINDEX monitor_index;
	tU32 i, j = 0;

	if (*filter_statusp == 0)
	{
		return ETAL_INVALID_HANDLE;
	}
	if (ETAL_extractFilter_MDR(cmd, len, &filter_recv) == OSAL_ERROR)
	{
		return ETAL_INVALID_HANDLE;
	}
	if ((filter_recv & *filter_statusp) == 0)
	{
		*filter_statusp = (etalFilterMaskTy)0;
		return ETAL_INVALID_HANDLE;
	}

	for (i = 0; i < ETAL_MAX_MONITORS; i++)
	{
		monitor_index = (ETAL_HINDEX)i;
		hMonitor = ETAL_handleMakeMonitor(monitor_index);

		monp = ETAL_statusGetMonitor(hMonitor);

		if(monp != NULL)
		{
			if (!monp->isValid)
			{
				continue;
			}
			if ((monp->monitorConfig.MDR.filterMask & filter_recv) != 0)
			{
				/*
				 * this is the monitor that set up the filter:
				 * all the filters associated with this monitor can be
				 * considered processed
				 */
				hMonitor_ret = ETAL_handleMakeMonitor(monitor_index);
				*filter_statusp &= (etalFilterMaskTy) ~(monp->monitorConfig.MDR.filterMask);

				if(filter_recv != 0)
				{
					filter_recv_tmp = filter_recv;
					while(filter_recv_tmp != 0)
					{
						if((filter_recv_tmp & 1) == TRUE)
						{
							ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_API, "DAB quality monitoring triggered by %d", monp->requested.m_monitoredIndicators[j].m_MonitoredIndicator);
						}
						filter_recv_tmp>>= 1;
						j++;
					}
				}
				break;
			}
		}
		else
		{
			ASSERT_ON_DEBUGGING(0);
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_searchMonitor_MDR monp: 0x%x", monp);
		}
	}

	return hMonitor_ret;
}

/***************************
 *
 * ETAL_createQualityContainer_MDR
 *
 **************************/
/*!
 * \brief		Extracts the quality information from the raw data received from the MDR
 * \remark
 * \param[in]	pmon - the monitor description, used to extract the Broadcast Standard
 * \param[in]	cmd  - the raw data (the notification) received from the MDR
 * \param[in]	len  - size in bytes of the *cmd* buffer; currently unused
 * \param[out]	qual - Pointer to DAB quality container
 *
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_createQualityContainer_MDR(etalMonitorTy *pmon, tU8 *cmd, tU32 len, EtalBcastQualityContainer *qual)
{
	etalReceiverStatusTy *recvp;

	recvp = ETAL_receiverGet(ETAL_statusGetReceiverFromMonitor(pmon));

	if((recvp != NULL) && (cmd != NULL) && (qual != NULL))
	{
		switch (recvp->currentStandard)
		{
			case ETAL_BCAST_STD_DAB:
				/* Initialize DAB quality container */
				ETAL_resetQualityContainer(recvp->currentStandard, qual);

				/* Extract DAB quality information */
				qual->m_standard = recvp->currentStandard;
				qual->m_TimeStamp = OSAL_ClockGetElapsedTime();
				qual->m_Context = pmon->requested.m_Context;
				ETAL_extractDabQualityFromNotification_MDR(&qual->EtalQualityEntries.dab, cmd + ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET);

				/* Insert RF field strength from the CMOST */
				if (ETAL_cmdGetChannelQuality_CMOST(pmon->requested.m_receiverHandle, qual) != OSAL_OK)
				{
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error in getting CMOST channel quality");
				}

			// ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_cmdGetAudioQuality_MDR %d", pmon->requested.m_receiverHandle);

			/* Insert DAB audio quality */
			if (ETAL_cmdGetAudioQuality_MDR(pmon->requested.m_receiverHandle, qual) != OSAL_OK)
			{
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error in getting DAB audio quality");
			}

			
			// ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_cmdGetAudioQuality_MDR %d done");
			break;

			default:
				break;
		}
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error in ETAL_createQualityContainer_MDR recvp: 0x%x, cmd: 0x%x, qual: 0x%x",
				recvp, cmd, qual);
	}
}

/***************************
 *
 * ETAL_qualityHandler_MDR
 *
 **************************/
/*!
 * \brief		Invokes the user callback for quality events generated by DAB DCOP
 * \details		DAB DCOP quality events are generated by configuring the DAB DCOP
 * 				with quality filters. When the filter thresholds are hit, the DAB DCOP
 * 				sends a notification to the Host (ETAL). The ETAL communication layer
 * 				just receives the DCOP notification and hands it over to the Callback handler.
 * 				The Callback handler, through this function, needs to find out which
 * 				ETAL monitor originated the DCOP notification and then invoke the
 * 				user notification handler.
 * \remark		The Callback handler is prepared to cope with quality events from the
 * 				DAB DCOP for _AM/FM_ also, for historical reasons: initially ETAL
 * 				was developed on an MDR3 platform where the DCOP (a.k.a. STA662) processed
 * 				both DAB and AM/FM
 * \param[in]	param     - the DAB DCOP notification containing quality info
 * \param[in]	param_len - the length in bytes of the *param* buffer
 * \callgraph
 * \callergraph
 * \see			ETAL_CommunicationLayer_Receive_MDR, code under ETAL_MDR_GETMONITORDABINFO /
 * 				ETAL_MDR_GETMONITORAMFMINFO
 */
static tVoid ETAL_qualityHandler_MDR(tU8 *param, tU32 param_len)
{
	ETAL_HANDLE hMonitor;
	etalMonitorTy *monp;
	etalFilterMaskTy filter_mask;
	etalQualityCbTy qual_cb_param;

	ETAL_initFilterMask_MDR(&filter_mask);
	while ((hMonitor = ETAL_searchMonitor_MDR(param, param_len, &filter_mask)) != ETAL_INVALID_HANDLE)
	{
		monp = ETAL_statusGetMonitor(hMonitor);

		if(monp != NULL)
		{
			(void)OSAL_pvMemorySet((tVoid *)&qual_cb_param, 0x00, sizeof(etalQualityCbTy));
			qual_cb_param.hMonitor = hMonitor;
			ETAL_createQualityContainer_MDR(monp, param, param_len, &qual_cb_param.qual);

			/* overwrite input data !!! */
			(void)OSAL_pvMemoryCopy((tPVoid)param, (tPVoid)&qual_cb_param, sizeof(etalQualityCbTy));

			// invoke the callback
			if (monp->requested.m_CbBcastQualityProcess)
			{
				monp->requested.m_CbBcastQualityProcess(&qual_cb_param.qual, monp->requested.m_Context);
			}
		}
		else
		{
			ASSERT_ON_DEBUGGING(0);
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_qualityHandler_MDR monp: 0x%x", monp);
		}
	}
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
/***************************
 *
 * ETAL_qualityHandler
 *
 **************************/
/*!
 * \brief		Invokes the quality handler defined by the ETAL API user
 * \details		Quality monitors may be defined by the ETAL API user request
 * 				ETAL to invoke a user callback function each time some defined
 * 				set of quality values is exeeded. This function invokes
 * 				the user callback.
 * \param[in]	qual_cb_param - the monitor descriptor
 * \param[in]	len - size of the *qual_cb_param* object, for sanity checks only
 * \see			etal_config_reception_quality_monitor and the ETAL specification for the same
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_qualityHandler(etalQualityCbTy *qual_cb_param, tU32 len)
{
	etalMonitorTy *pmon = NULL;

	if (len == sizeof(etalQualityCbTy))
	{
		if(qual_cb_param != NULL)
		{
			pmon = ETAL_statusGetMonitor(qual_cb_param->hMonitor);

			if(pmon != NULL)
			{
				if(pmon->requested.m_CbBcastQualityProcess != NULL)
				{
					pmon->requested.m_CbBcastQualityProcess(&qual_cb_param->qual, pmon->requested.m_Context);
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_qualityHandler pmon: 0x%x", pmon);
				ASSERT_ON_DEBUGGING(0);
			}
		}
		else
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_qualityHandler qual_cb_param: 0x%x", qual_cb_param);
			ASSERT_ON_DEBUGGING(0);
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_API, "ETAL_qualityHandler len: %d, sizeof(etalQualityCbTy): %d", len, sizeof(etalQualityCbTy));
		ASSERT_ON_DEBUGGING(0);
	}
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR || CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_CallbackHandler_ThreadEntry
 *
 **************************/
/*!
 * \brief		Callback Thread main entry point
 * \details		Implements the Callback Thread. The thread endlessly tries to
 * 				pop a value from the callback FIFO, invokes the callback and sleeps.
 * 				Callbacks are invoked this way rather than directly from ETAL internals
 * 				to avoid locking up the whole ETAL in case of misbehaving user handler.
 *
 * 				It is possible to spawn more than one callback handler, provided each
 * 				one is initialized with unique *thread_param* value. This feature is
 * 				implemented but not tested.
 * \remark		The function does not perform any check on the user handler duration.
 * 				It is up to the user to avoid locking up the callback mechanism for too long.
 * \param[in]	thread_param - interpreted as the callback handler index. The function
 * 				               uses this parameter to access its private storage (required
 * 				               in case more than one callback handler thread is spawned).
 * \callgraph
 * \callergraph
 */
#ifdef CONFIG_HOST_OS_TKERNEL
tVoid ETAL_CallbackHandler_ThreadEntry(tSInt stacd, tPVoid thread_param)
#else
tVoid ETAL_CallbackHandler_ThreadEntry(tPVoid thread_param)
#endif
{
	ETAL_cbStatusTy *pstatus;
	etalCallbackTy msg_type = 0;
	EtalCbNotify cbnotify;
	tU32 thread_index;
	tU32 param_len = 0;
	ETAL_EVENTS event = 0;

	thread_index = (tU32)((tULong)thread_param);
	if (thread_index >= ETAL_CALLBACK_HANDLERS_NUM)
	{
		ASSERT_ON_DEBUGGING(0);
		return;
	}

	pstatus = &Etal_callbackStatus[thread_index];

	while (TRUE)
	{
		// while being awaked, let's push all the callback which are in the fifo, instead of queuing and waiting next wake up
		// so call the ETAL_callbackFifoPop until it returns an OSAL_ERROR which means it is empty
		
		while (ETAL_callbackFifoPop(pstatus, &msg_type, &event, (tVoid *)&pstatus->cbStorage, &param_len) == OSAL_OK)
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "Callback Thread received message type %s, for event %d", ETAL_MsgTypeToString(msg_type), event);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
			COMMON_tracePrintBufComponent(TR_CLASS_APP_ETAL, (tU8 *)&pstatus->cbStorage, param_len, NULL);
#endif
			switch (msg_type)
			{
				case cbTypeEvent:
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "Invoking %s callback", ETAL_MsgTypeToString(msg_type));
					if ((cbnotify = ETAL_statusHardwareAttrGetNotifycb()) != NULL)
					{
						if (param_len != 0)
						{
							cbnotify(ETAL_statusHardwareAttrGetNotifyContext(), event, (tVoid *)&pstatus->cbStorage);
						}
						else
						{
							cbnotify(ETAL_statusHardwareAttrGetNotifyContext(), event, NULL);
						}
					}
					break;

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				case cbTypeQuality_MDR:
					// Be careful pstatus->cbStorage type change inside the function tU8 --> etalQualityCbTy
					ETAL_qualityHandler_MDR((tU8 *)&pstatus->cbStorage, param_len);
					break;
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
				case cbTypeQuality:
					ETAL_qualityHandler((etalQualityCbTy *)&pstatus->cbStorage, param_len);
					break;
#endif
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API
			}

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM)
			ETAL_traceEvent(msg_type, event, &pstatus->cbStorage);
#endif
		}

		// FIFO is empty : wait next wake-up
		(void)OSAL_s32ThreadWait(ETAL_CALLBACK_THREAD_SCHEDULING);
	}
}

