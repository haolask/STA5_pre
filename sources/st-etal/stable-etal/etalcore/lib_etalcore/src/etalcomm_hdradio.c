//!
//!  \file 		etalcomm_hdradio.c
//!  \brief 	<i><b> ETAL communication layer for HDRADIO devices </b></i>
//!  \details   Communication with HDRADIO device
//!  $Author$
//!  \author 	(original version) Roberto Allevi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
	#include "ipfcomm.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/***************************
 * Defines
 **************************/
/*!
 * \def		ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE
 * 			Max size of the commands that will be inserted in the
 * 			HD Radio command FIFO.
 */
#define ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE (HDRADIO_CP_FULL_SIZE)
/*!
 * \def		ETAL_CMD_TO_HDRADIO_FIFO_SIZE
 * 			Max number of elements in the HD Radio command FIFO.
 */
#define ETAL_CMD_TO_HDRADIO_FIFO_SIZE      8

#ifdef CONFIG_MODULE_DCOP_HDRADIO_I2C_SERIES_PULLUP_RES
	/*!
	 * \def		HDRADIO_RESPONSE_OCCURRENCE_TIME
	 * 			This is the minimum time in milliseconds to wait after sending a command
	 * 			to the STA680 before starting to read the response. The time
	 * 			depends on the speed of the I2C bus which in turn is affected
	 * 			by the hardware layout.
	 */
	#define HDRADIO_RESPONSE_OCCURRENCE_TIME   70
#endif

#ifdef CONFIG_MODULE_DCOP_HDRADIO_I2C_PULLUP_RES
	/*!
	 * \def		HDRADIO_RESPONSE_OCCURRENCE_TIME
	 * 			This is the minimum time in milliseconds to wait after sending a command
	 * 			to the STA680 before starting to read the response. The time
	 * 			depends on the speed of the I2C bus which in turn is affected
	 * 			by the hardware layout.
	 */
	#define HDRADIO_RESPONSE_OCCURRENCE_TIME   10
#endif

/*!
 * 	\def		HDRADIO_DRIVER_EXTERNAL_RESPONSE_TIME
 * 				Time in milliseconds needed by the external driver (the Protocol Layer)
 * 				to receive the command from ETAL, pass it to the DCOP
 * 				and send back the answer.
 * 				Since communication with the PL typically
 * 				occurs over TCP/IP, this time may be influenced by 
 * 				the network load/performance
 */
#define HDRADIO_DRIVER_EXTERNAL_RESPONSE_TIME  300

/*!
 * \def		PROCESSING_COMPLETE
 * 			Return value for #ETAL_checkTuneSeekCommand_HDRADIO.
 * 			Indicates that the command has been completely processed
 * 			by the function.
 */
#define PROCESSING_COMPLETE ((tU32)0)
/*!
 * \def		KEEP_PROCESSING
 * 			Return value for #ETAL_checkTuneSeekCommand_HDRADIO.
 * 			Indicates that the command needs further processing from the
 * 			caller of the function.
 */
#define KEEP_PROCESSING     ((tU32)1)

#if (HDRADIO_FAKE_RECEIVER <= ETAL_MAX_RECEIVERS)
	#error "HDRADIO_FAKE_RECEIVER must be greater than ETAL_MAX_RECEIVERS to avoid ambiguity"
#endif

/***************************
 * Types
 **************************/
/*!
 * \struct	ETAL_HDRADIOCmdFifoElemTy
 * 			Description of a single element of the HD Radio command FIFO.
 */
typedef struct
{
	/*! Datapath handle relative to the command described in *param* */
	ETAL_HANDLE hDatapath;
	/*! The command and its parameters */
	tU8         param[ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE];
	/*! The size in bytes of *param* */
	tU32        paramSize;
} ETAL_HDRADIOCmdFifoElemTy;

/*!
 * \struct	ETAL_HDRADIOStatusTy
 * 			The HD Radio command FIFO. This FIFO is filled by the APIs
 * 			and internal functions and emptied by the
 * 			#ETAL_CommunicationLayer_ThreadEntry_HDRADIO.
 */
typedef struct
{
	/*! The FIFO meta-information */
	COMMON_fifoStatusTy       hdFifo;
	/*! The FIFO information */
	ETAL_HDRADIOCmdFifoElemTy ETAL_CmdToHDRADIOFifo[ETAL_CMD_TO_HDRADIO_FIFO_SIZE];
} ETAL_HDRADIOStatusTy;

#ifdef HD_MONITORING
static ETAL_HANDLE hReceiverForTune[ETAL_MAX_HDINSTANCE];
#endif 

/***************************
 * Prototypes
 **************************/
#ifdef CONFIG_HOST_OS_TKERNEL
	static tVoid ETAL_CommunicationLayer_ThreadEntry_HDRADIO(tSInt stacd, tPVoid pvArg);
#else
	static tVoid ETAL_CommunicationLayer_ThreadEntry_HDRADIO(tPVoid pvArg);
#endif
static tSInt ETAL_initCommandFIFO_HDRADIO(ETAL_HDRADIOStatusTy *pfifo);
static tSInt ETAL_CmdToHDRADIOFifoPush(ETAL_HANDLE hDatapath, tU8 *cmd, tU32 cmd_size);
static tSInt ETAL_CmdToHDRADIOFifoPop(ETAL_HANDLE* phDatapath, tU8 *cmd, tU32 *cmd_size);


static tVoid ETAL_getHDRADIOReceiverMonitoringLock(tVoid);
static tVoid ETAL_releaseHDRADIOReceiverMonitoringLock(tVoid);


/***************************
 * Locals
 **************************/
 /*!
  * \var	ETAL_CommToHDRADIOThreadId
  * 		Thread ID of the #ETAL_CommunicationLayer_ThreadEntry_HDRADIO,
  * 		used only to destroy the thread at de-initialization time.
  */
static OSAL_tThreadID            ETAL_CommToHDRADIOThreadId;
/*!
 * \var		ETAL_StatusHDRADIODevSem
 * 			Semaphore used to ensure unique access to the HD Radio DCOP.
 */
static OSAL_tSemHandle           ETAL_StatusHDRADIODevSem;
/*!
 * \var		ETAL_StatusHDRADIO
 * 			The HD Radio command FIFO.
 */
static ETAL_HDRADIOStatusTy      ETAL_StatusHDRADIO;

/*!
 * \var		ETAL_TuneSem_HDRADIO
 * 			This semaphore is taken by #ETAL_tuneFSM_HDRADIO when it starts a tune command
 * 			and released when it completes the command with a TUNE_EVENT.
 * 			It is used to avoid that the receiver is deleted while the state machine is in 
 * 			progress. Thus #etal_destroy_receiver must take also this semaphore (in addition
 * 			to the global and receiver ones) before destroying an HD receiver.
 *
 * 			The receiver lock cannot be used in this context because it would mean locking
 * 			out all the HD functions while tune is in progress.
 */
static OSAL_tSemHandle           ETAL_TuneSem_HDRADIO[ETAL_MAX_HDINSTANCE];

static OSAL_tSemHandle           ETAL_HDRadio_ReceiverMonitoringSem;



/*!
 * \var		v_hd_ForceStationInfoResend
 * 			Variable for RadioText auto-notification 
 * 			In case of change of service, force to send again the radio text and SIS
 *			so that the client receive the service name even if kept unchanged
 */
static tBool v_hd_ForceStationInfoResend = false;


/*!
 * \var		v_hd_ForceRadioTextInfoResend
 * 			Variable for RadioText auto-notification 
 * 			In case of change of service, force to send again the radio text and SIS
 *			so that the client receive the service name even if kept unchanged
 */

static tBool v_hd_ForceRadioTextInfoResend = false;

/***************************
 *
 * ETAL_initCommandFIFO_HDRADIO
 *
 **************************/
/*!
 * \brief		Initialize the HD Radio command FIFO
 * \remark		The parameter is not checked for correctness
 * \param[in]	pfifo - pointer to the command FIFO
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error during the FIFO semaphore creation
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initCommandFIFO_HDRADIO(ETAL_HDRADIOStatusTy *pfifo)
{
	tSInt ret = OSAL_OK;
	
	(void)OSAL_pvMemorySet((tVoid *)pfifo->ETAL_CmdToHDRADIOFifo, 0x00, sizeof(ETAL_HDRADIOCmdFifoElemTy) * ETAL_CMD_TO_HDRADIO_FIFO_SIZE);
	if (COMMON_fifoInit(&pfifo->hdFifo, ETAL_FIFO_HDRADIO_NAME, ETAL_CMD_TO_HDRADIO_FIFO_SIZE) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else
	{
		COMMON_fifoReset(&pfifo->hdFifo);
	}
	return ret;
}

/***************************
 *
 * ETAL_isValidInstance_HDRADIO
 *
 **************************/
/*!
 * \brief		Checks if an HD Radio instance identifier is valid
 * \details		HD Radio DCOP supports 2 channels (or instance in the HD Radio terminology).
 * 				The function checks if the *instanceId* is a valid identifier.
 * 				Valid identifiers are those defined by #tyHDRADIOInstanceID although
 * 				the current hardware only supports two channels so 
 * 				the function returns true if the parameter is INSTANCE_1 or INSTANCE_2.
 * \param[in]	instanceId - the identifier to be checked
 * \return		TRUE - *instanceId* is INSTANCE_1 or INSTANCE_2, FALSE otherwise
 * \callgraph
 * \callergraph
 */
static tBool ETAL_isValidInstance_HDRADIO(tyHDRADIOInstanceID instanceId)
{
	tBool ret = TRUE;
	if ((instanceId != INSTANCE_1) && (instanceId != INSTANCE_2))
	{
		ret = FALSE;
	}
	return ret;
}

/***************************
 *
 * ETAL_getHDRADIODevLock
 *
 **************************/
/*!
 * \brief		Take the Receiver Monitoring Lock
 * \details		Access to a variable on HD ReceiverList to be monitored should be protected.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getHDRADIOReceiverMonitoringLock(tVoid)
{
	(void)OSAL_s32SemaphoreWait(ETAL_HDRadio_ReceiverMonitoringSem, OSAL_C_TIMEOUT_FOREVER);
}

/***************************
 *
 * ETAL_releaseHDRADIODevLock
 *
 **************************/
/*!
 * \brief		Releases the Receiver Monitoring Lock
 * \see			ETAL_releaseHDRADIOReceiverMonitoringLock
 * \callgraph
 * \callergraph
 */
tVoid ETAL_releaseHDRADIOReceiverMonitoringLock(tVoid)
{
	(void)OSAL_s32SemaphorePost(ETAL_HDRadio_ReceiverMonitoringSem);
}

/***************************
 *
 * ETAL_getHDRADIODevLock
 *
 **************************/
/*!
 * \brief		Locks the HD Radio DCOP device
 * \details		The HD Radio DCOP communication bus supports only one command
 * 				at a time. Taking this lock before accessing the communication
 * 				bus ensures there will be no corruption.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_getHDRADIODevLock(tVoid)
{
	(void)OSAL_s32SemaphoreWait(ETAL_StatusHDRADIODevSem, OSAL_C_TIMEOUT_FOREVER);
}

/***************************
 *
 * ETAL_releaseHDRADIODevLock
 *
 **************************/
/*!
 * \brief		Releases the HD Radio DCOP device
 * \see			ETAL_getHDRADIODevLock
 * \callgraph
 * \callergraph
 */
tVoid ETAL_releaseHDRADIODevLock(tVoid)
{
	(void)OSAL_s32SemaphorePost(ETAL_StatusHDRADIODevSem);
}


/***************************
 *
 * ETAL_getTuneFSMLock_HDRADIO
 *
 **************************/
/*!
 * \brief		Take the Tune FSM lock
 * \param[in]	instanceId - HD Radio instance identifier
 * \return		OSAL_OK
 * \return		OSAL_ERROR - illegal *instanceId* or error taking the semaphore
 * \see			ETAL_TuneSem_HDRADIO
 * \callgraph
 * \callergraph
 */
tSInt ETAL_getTuneFSMLock_HDRADIO(tyHDRADIOInstanceID instanceId)
{
	tU32 index;
	tSInt ret;

	if (!ETAL_isValidInstance_HDRADIO(instanceId))
	{
		ASSERT_ON_DEBUGGING(0);
		ret = OSAL_ERROR;
	}
	else
	{
		index = (tU32)instanceId - 1;
		
		ret = OSAL_s32SemaphoreWait(ETAL_TuneSem_HDRADIO[index], OSAL_C_TIMEOUT_FOREVER);
	}
	
	return ret;
}

/***************************
 *
 * ETAL_releaseTuneFSMLock_HDRADIO
 *
 **************************/
/*!
 * \brief		Release the Tune FSM lock
 * \param[in]	instanceId - HD Radio instance identifier
 * \see			ETAL_getTuneFSMLock_HDRADIO
 * \callgraph
 * \callergraph
 */
tVoid ETAL_releaseTuneFSMLock_HDRADIO(tyHDRADIOInstanceID instanceId)
{
	tU32 index;

	if (!ETAL_isValidInstance_HDRADIO(instanceId))
	{
		ASSERT_ON_DEBUGGING(0);
	}
	else
	{
		index = (tU32)instanceId - 1;
	
		(void)OSAL_s32SemaphorePost(ETAL_TuneSem_HDRADIO[index]);
	}

	return;	
}

/***************************
 *
 * ETAL_CmdToHDRADIOFifoPush
 *
 **************************/
/*!
 * \brief		Pushes a Command onto the HD Radio FIFO
 * \details		The data pointed by *cmd* is copied to a local storage,
 * 				so *cmd* can be de-allocated after return from this function.
 * \param[in]	hDatapath - handle of the Datapath to which the command is directed
 * \param[in]	cmd - the command, including the parameters
 * \param[in]	cmd_size - the size of *cmd* in bytes
 * \return		OSAL_OK
 * \return		OSAL_ERROR - *cmd_size* is larger than #ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE bytes, or
 * 				             the FIFO is full. In both cases the message is discarded.
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_CmdToHDRADIOFifoPush(ETAL_HANDLE hDatapath, tU8 *cmd, tU32 cmd_size)
{
	ETAL_HDRADIOCmdFifoElemTy *elem;
	ETAL_HDRADIOStatusTy *pfifo = &ETAL_StatusHDRADIO;
	tS16 new_write_ptr;
	tSInt ret = OSAL_OK;
#ifdef HD_MONITORING
	tyHDRADIOInstanceID instanceId;
	tU32 index;
#endif

	if (cmd_size > ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Message too long in CMD to HDRADIO fifo (req %d, max %d)", cmd_size, ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE);
		ret = OSAL_ERROR;
		goto exit;
	}

#ifdef HD_MONITORING

	
		// specific processing for STOP : 
		// it should not be queued but processed immediately
		if (HC_COMMAND_PACKET_GET_OPCODE(cmd) == (tU8)HDRADIO_SEEK_STOP_SPECIAL)
		{
			/*
			* INSTANCE_UNDEF is a valid return in this case and it is processed
			 * later so we can safely ignore the return value
			 */
			(LINT_IGNORE_RET) ETAL_receiverGetHdInstance(hDatapath, &instanceId);
	
			if (instanceId == INSTANCE_UNDEF)
			{
				/*
				 * any valid SEEK_START/SEEK_STOP command must have a valid instanceId,
				 * thus an INSTANCE_UNDEF could mean the command is referring to a receiver
				 * that has been reconfigured before the command was executed;
				 * ignore
				 */
				ret = OSAL_ERROR;
				goto exit;
			}
			
			index = (tU32)instanceId - 1;
			ETAL_getHDRADIOReceiverMonitoringLock();			
			if (hReceiverForTune[index] == hDatapath)
			{
				/*
				 * ETAL_tuneFSM_HDRADIO with cmdTuneRestart returns immediately
				 * without changing the receiver state, no need to take the receiver lock
				 */
				(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hDatapath, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_INTERNAL_USER);
			}
			else
			{
				// receiver lock error not good....
				
			}
	
			
			hReceiverForTune[index] = ETAL_INVALID_HANDLE;
	
			ETAL_releaseHDRADIOReceiverMonitoringLock();
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Stopping HD FSM recv %d, inst %d", hDatapath, instanceId);
	
			ret =	OSAL_OK;
			goto exit;
		}
#endif

	COMMON_fifoGetLock(&pfifo->hdFifo);

	if (COMMON_fifoPush(&pfifo->hdFifo, &new_write_ptr) != OSAL_OK)
	{
		/* FIFO overflow */
		COMMON_fifoReleaseLock(&pfifo->hdFifo);
		ret = OSAL_ERROR;
		goto exit;
	}
	elem = &pfifo->ETAL_CmdToHDRADIOFifo[new_write_ptr];

	elem->hDatapath = hDatapath;
	(void)OSAL_pvMemoryCopy((tVoid *)elem->param, (tPCVoid)cmd, cmd_size * sizeof(*cmd));
	elem->paramSize = cmd_size;

	COMMON_fifoReleaseLock(&pfifo->hdFifo);

exit:	
	return ret;
}

/***************************
 *
 * ETAL_CmdToHDRADIOFifoPop
 *
 **************************/
/*!
 * \brief		Pops a command from the HD Radio FIFO
 * \details		Copies the next command to the buffer provided by the caller.
 * \param[out]	phDatapath - pointer to a location where the function stores the
 * 				             Datapath handle passed when the command was pushed on the FIFO
 * \param[out]	cmd - pointer to a buffer capable of #ETAL_CMD_TO_HDRADIO_FIFO_DATA_SIZE bytes
 * \param[out]	cmd_size - number of bytes written to *cmd*
 * \return		OSAL_OK - the FIFO was non-empty, the parameters contain a valid command
 * \return		OSAL_ERROR - the FIFO was empty, the parameters are not updated
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_CmdToHDRADIOFifoPop(ETAL_HANDLE* phDatapath, tU8 *cmd, tU32 *cmd_size)
{
	ETAL_HDRADIOCmdFifoElemTy *elem;
	ETAL_HDRADIOStatusTy *pfifo = &ETAL_StatusHDRADIO;
	tS16 new_read_ptr;
	tSInt ret = OSAL_OK;

	COMMON_fifoGetLock(&pfifo->hdFifo);
	if (COMMON_fifoPop(&pfifo->hdFifo, &new_read_ptr) != OSAL_OK)
	{
		/* FIFO empty condition */
		COMMON_fifoReleaseLock(&pfifo->hdFifo);
		ret = OSAL_ERROR;
	}
	else
	{
		elem = &pfifo->ETAL_CmdToHDRADIOFifo[new_read_ptr];
	
		*phDatapath = elem->hDatapath;
		(void)OSAL_pvMemoryCopy((tVoid *)cmd, (tPCVoid)elem->param, elem->paramSize * sizeof(*elem->param));
		*cmd_size = elem->paramSize;
	
		COMMON_fifoReleaseLock(&pfifo->hdFifo);
		ret = OSAL_OK;
	}
	return ret;
}

/***************************
 *
 * ETAL_commandStatusToEventErrStatus_HDRADIO
 *
 **************************/
/*!
 * \brief		Translates an HD Radio status to an ETAL status
 * \details		The HD Radio status is the Logical Message status byte
 * 				returned by the HD Radio DCOP and documented in chapter 6.2.2.7
 * 				of the DCOP spec (see etalcmd_hdradio.c file description)
 * \param[in]	cstat - the Logical Message status byte
 * \return		the translated status byte
 * \callgraph
 * \callergraph
 */
static EtalCommErr ETAL_commandStatusToEventErrStatus_HDRADIO(etalToHDRADIOStatusTy cstat)
{
	EtalCommErr ret;
	
	if (cstat.bf.Success != (tU8)0)
	{
		ret = EtalCommStatus_NoError;
	}
	else if (cstat.bf.LMCountMismatch != (tU8)0)
	{
		ret = EtalCommStatus_ProtocolContinuityError;
	}
	else if (cstat.bf.ChecksumFailed != (tU8)0)
	{
		ret = EtalCommStatus_ChecksumError;
	}
	else if (cstat.bf.HeaderNotFound != (tU8)0)
	{
		ret = EtalCommStatus_ProtocolHeaderError;
	}
	else /* OverflowCondition, others? */
	{
		ret = EtalCommStatus_GenericError;
	}
	
	return ret;
}

/***************************
 *
 * ETAL_forceResendRadioInfo
 *
 **************************/
/*!
 * \brief		Set a Flag to force the Radio Text Info and Station Info to be resent
 * \details		
 * \param[in]	
 * \return		
 * \callgraph
 * \callergraph
 */
tVoid ETAL_forceResendRadioInfo(tVoid)
{
	v_hd_ForceStationInfoResend = true;
	v_hd_ForceRadioTextInfoResend = true;
}

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
/***************************
 *
 * ETAL_isValidInstanceId
 *
 **************************/
/*!
 * \brief		Checks if the integer parameter is a valid HD Radio instance ID
 * \details		This function is similar to #ETAL_isValidInstance_HDRADIO but
 * 				accepts a generic integer.
 * \param[in]	val - the value to be checked
 * \return		OSAL_OK - the value is a valid HD Radio instance identifier
 * \return		OSAL_ERROR - the value is not a valid HD Radio instance identifier
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_isValidInstanceId(tU32 val)
{
	if ((val != INSTANCE_1) && (val != INSTANCE_2))
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_sendCommandWithExternalDriver_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends a command to the HD Radio DCOP and waits for the answer, using the external driver
 * \details		The external driver is implemented in a stand-alone executable normally
 * 				named Protocol Layer.
 *
 * 				The *cmd* buffer format is the same as the one used for #HDRADIO_Write_Command:
 *
 * 				- 0            opcode
 * 				- 1 to 4       data length (N)
 * 				- 5 to (5+N-1) command payload starting with the function code byte
 * 				- N            operation (0 for read, 1 for write)
 * 				- N+1          reserved (0)
 * 				- N+2          command status
 *
 * \remark		The function waits a fixed amount of time for the Protocol Layer to
 * 				provide the command response.
 * \param[in]	instIdCmd - Instance ID of the HD Radio DCOP to which the command is sent
 * \param[in]	cmd - the command plus parameters
 * \param[in]	clen - the command length in bytes
 * \param[out]	instIdResp - the Instance ID extracted provided by the Protocol Layer, or
 * 				             INSTANCE_UNDEF if the provided data is not valid
 * \param[out]	resp - the response
 * \param[out]	rlen - the response length
 * \return		OSAL_OK
 * \return		OSAL_ERROR - no answer received from the Protocol Layer within the timeout
 * \return		OSAL_ERROR_DEVICE_INIT - the Protocol Layer is not active or not responding
 * \callgraph
 * \callergraph
 * \todo		Experiment with other delays for the command response.
 */
static tSInt ETAL_sendCommandWithExternalDriver_HDRADIO(tyHDRADIOInstanceID instIdCmd, tU8 *cmd, tU32 clen, tyHDRADIOInstanceID *instIdResp, tU8 *resp, tSInt *rlen)
{
	CtrlAppMessageHeaderType head;
	tU8 h0, h1, h2, h3;
	tU32 len;
	tU32 address;
	tBool useI2C;
	tSInt ret;
    EtalDeviceDesc deviceDescription;

	/* TODO see function header */

	if (!TcpIpProtocolIsConnected(ProtocolLayerIndexDCOP))
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "No connection to DCOP ProtocolLayer");
		return OSAL_ERROR_DEVICE_INIT;
	}

    ETAL_getDeviceDescription_HDRADIO(&deviceDescription);
    address = deviceDescription.m_busAddress;
    useI2c = (deviceDescription.m_busType == ETAL_BusI2C) ? TRUE : FALSE;

	/* No semaphore taken because the current HDRadio code always calls ETAL_sendCommandTo_HDRADIO
	 * with HDRadio device lock, or uses the ETAL_cmd*HDRADIO which take the device lock
	 */

	h0 = ETAL_EXTERNAL_DRIVER_TYPE_COMMAND;
	h1 = (tU8)instIdCmd;
	h2 = (tU8)address;
	h3 = ETAL_EXTERNAL_DRIVER_RESERVED; /* will be SPI phase for SPI communication */

	ForwardPacketToCtrlAppPort(ProtocolLayerIndexDCOP, cmd, clen, (tU8)0x1D, ETAL_BROADCAST_LUN, h0, h1, h2, h3);

	OSAL_s32ThreadWait(HDRADIO_DRIVER_EXTERNAL_RESPONSE_TIME); /* TODO see function header */

	ret = ProtocolLayer_FifoPop(ProtocolLayerIndexDCOP, resp, PROTOCOL_LAYER_INTERNAL_BUFFER_LEN, &len, &head);
	if (ret == OSAL_OK)
	{
		if (ETAL_isValidInstanceId((tU32)head.CtrlAppSpare_1) != OSAL_OK)
		{
			*instIdResp = INSTANCE_UNDEF;
		}
		else
		{
			*instIdResp = (tyHDRADIOInstanceID)head.CtrlAppSpare_1;
		}
		*rlen = (tSInt) len;
		return OSAL_OK;
	}
	else if (ret == OSAL_ERROR_TIMEOUT_EXPIRED)
	{
		/* ProtocolLayer_FifoPop was empty, could retry reading it (not implemented yet ) */
	}

	return OSAL_ERROR;
}
#endif // CONFIG_COMM_DRIVER_EXTERNAL

/***************************
 *
 * ETAL_sendCommandTo_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends immediately a command to the HD Radio DCOP and reads the response
 * \details		Function called by all ETAL API functions to send **immediately** a
 * 				Command Packet to the HDRADIO and wait a response.
 * 				This function blocks until it receives a response from the DCOP.
 *
 * 				For the EMBEDDED driver the function changes the *resp* parameter.
 * 				(Details: the BSP function uses the *resp* buffer to store the response but the first byte
 * 				of the response must be skipped so rather than allocating a temporary buffer
 * 				it advances the *resp* pointer).
 *
 * 				The function may be called for internal or internal driver.
 * \remark		This function must be called with the DCOP device lock taken,
 * 				it does not take any device lock itself.
 * \remark		*resp* remains valid until the next call to ETAL_sendCommandTo_HDRADIO.
 * \param[in]	hReceiver - handle of the Receiver to which the command should be sent
 * \param[in]	cmd - the command plus parameters
 * \param[in]	clen - the command length in bytes
 * \param[out]	resp - the response
 * \param[out]	rlen - the response length
 * \return		OSAL_OK
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - the Receiver is no longer available
 * \return		Errors returned by #ETAL_sendCommandToInstance_HDRADIO
 * \callgraph
 * \callergraph
 */
tSInt ETAL_sendCommandTo_HDRADIO(ETAL_HANDLE hReceiver, tU8 *cmd, tU32 clen, tU8 **resp, tSInt *rlen)
{
	tyHDRADIOInstanceID instIdCmd;
	tSInt ret;

	if (ETAL_receiverGetHdInstance(hReceiver, &instIdCmd) != OSAL_OK)
	{
		/* logical error, not communication error; maybe the receiver
		 * was reconfigured while the command was about to be executed */
		ASSERT_ON_DEBUGGING(0);
		ret = OSAL_ERROR_DEVICE_NOT_OPEN;
	}
	else
	{
		ret = ETAL_sendCommandToInstance_HDRADIO(instIdCmd, cmd, clen, resp, rlen);
	}

	return ret;
}

/***************************
 *
 * ETAL_sendCommandToInstance_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends immediately a command to the HD Radio DCOP and reads the response
 * \details		Used by #ETAL_sendCommandTo_HDRADIO to deliver to the DCOP;
 * 				used the DCOP instance id instead of the hReceiver
 * \remark		Assumes implicitly a single DCOP present in the system, thus
 * 				does not require its address
 * \param[in]	instIdCmd - InstanceId
 * \param[in]	cmd - the command plus parameters
 * \param[in]	clen - the command length in bytes
 * \param[out]	resp - the response
 * \param[out]	rlen - the response length
 * \return		OSAL_OK
 * \return		OSAL_ERROR_CANNOT_OPEN - error writing the command to the device
 * \return		OSAL_ERROR_FROM_SLAVE - the device response indicates there was an error
 * 				                        processing the command
 * \callgraph
 * \callergraph
 */
tSInt ETAL_sendCommandToInstance_HDRADIO(tyHDRADIOInstanceID instIdCmd, tU8 *cmd, tU32 clen, tU8 **resp, tSInt *rlen)
{
	tSInt ret, retval = OSAL_OK;
	tyHDRADIOInstanceID instIdRes = INSTANCE_UNDEF;
	etalToHDRADIOStatusTy cstat;
	tS8 ctype = (tS8)UNDEF_TYPE;

	cstat.B = (tU8)0;

#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	if (ETAL_sendCommandWithExternalDriver_HDRADIO(instIdCmd, cmd, clen, &instIdRes, *resp, rlen) != OSAL_OK)
	{
		retval = OSAL_ERROR_CANNOT_OPEN;
	}
#elif defined (CONFIG_COMM_DRIVER_EMBEDDED)
	if (-1 == HDRADIO_Write_Command(instIdCmd, cmd, clen))
	{
		retval = OSAL_ERROR_CANNOT_OPEN;
	}
	else
	{
		if((ret = HDRADIO_Read_Response(&instIdRes, *resp, rlen)) != OSAL_OK)
		{
			retval = ret;
		}
	}
#endif // CONFIG_COMM_DRIVER_EXTERNAL

	if (retval == OSAL_OK)
	{
		if (*rlen >= HDRADIO_CP_OVERHEAD)
		{
			cstat.B = (*resp)[*rlen - 1];
			ctype = (tS8)(*resp)[*rlen - 3];
			if ((cstat.bf.Success != (tU8)1) || (ctype != (tS8)READ_TYPE) || (instIdCmd != instIdRes))
			{
				retval = OSAL_ERROR_FROM_SLAVE;
			}
		}
		else
		{
			retval = OSAL_ERROR_FROM_SLAVE;
		}
	}

	return retval;
}

/***************************
 *
 * ETAL_processTuneSeekFSM_HDRADIO
 *
 **************************/
/*!
 * \brief		Invokes the #ETAL_tuneFSM_HDRADIO state machine
 * \details		Goes through the list of the Receivers currently configured for HD Radio
 * 				and runs the #ETAL_tuneFSM_HDRADIO for each one. Since the #ETAL_tuneFSM_HDRADIO
 * 				changes the Receiver state, the function takes the Receiver lock.
 * \param[in]	hReceiverList - pointer to array of Receiver handles of receivers configured for HDRadio.
 * 				                The array may contain ETAL_INVALID_HANDLE in non-used entries, they
 * 				                will be skipped.
 * \param[in]	receiverListSize - size of the *hReceiverList* array
 * \callgraph
 * \callergraph
 */
static tVoid ETAL_processTuneSeekFSM_HDRADIO(ETAL_HANDLE *hReceiverList, tU32 receiverListSize)
{
	ETAL_HANDLE  hReceiver;
	tBool waitAudio;
	tU32 index;
	ETAL_STATUS vl_res;

	for (index = 0; index < receiverListSize; index++)
	{
	
		ETAL_getHDRADIOReceiverMonitoringLock();

		hReceiver = hReceiverList[index];

		ETAL_releaseHDRADIOReceiverMonitoringLock();
		
		if (hReceiver != ETAL_INVALID_HANDLE)
		{
			/*
			 * ETAL_tuneFSM_HDRADIO changes the receiver state so
			 * take the receiver lock
			 */
			if (ETAL_receiverGetLock(hReceiver) == ETAL_RET_SUCCESS)
			{
				// Check the receiver is still valid
				// in case for some cross case, the lock was blocked by a destroying function
				//
				if (TRUE == ETAL_receiverIsValidHandle(hReceiver))
				{
		
					waitAudio = ETAL_receiverSupportsAudio(hReceiver);	
					vl_res = ETAL_tuneFSM_HDRADIO(hReceiver, ETAL_receiverGetFrequency(hReceiver), tuneFSMHDImmediateResponse, waitAudio, ETAL_HDRADIO_TUNEFSM_INTERNAL_USER);
#ifdef HD_MONITORING
					if (vl_res == ETAL_RET_ERROR)
					{
						// do not remove the tuner from the list
						// if the Sync is ok
						// because we need to continue permanently monitoring : 
						// this is done thru continuous call to the FSM
						// in case of ERROR or NO_DATA (means no HD) then we do not do any monitoring...
						//
						ETAL_getHDRADIOReceiverMonitoringLock();
						hReceiverList[index] = ETAL_INVALID_HANDLE;
						ETAL_releaseHDRADIOReceiverMonitoringLock();
					}

#else
					if (vl_res != ETAL_RET_IN_PROGRESS)
					{
						hReceiverList[index] = ETAL_INVALID_HANDLE;
					}
#endif
				}
				else
				{
					// an other task has destroy the receiver before we got the lock
					// 
					// simply do nothing
				}
				
				ETAL_receiverReleaseLock(hReceiver);
			}
		}
	}
}

/***************************
 *
 * ETAL_checkTuneSeekCommand_HDRADIO
 *
 **************************/
/*!
 * \brief		Processes the special HD Radio commands
 * \details		This function processes the special commands used to start and
 * 				stop the #ETAL_tuneFSM_HDRADIO: #HDRADIO_SEEK_START_SPECIAL
 * 				and #HDRADIO_SEEK_STOP_SPECIAL. If the *cmd* contains one such
 * 				command the function updates the list of HD Radio receivers
 * 				needing attention (*hReceiverList*).
 * \param[in,out] hReceiverList - pointer to an array of Receiver handles needing
 * 				                  action from the #ETAL_tuneFSM_HDRADIO
 * \param[in]	hReceiver - handle of the Receiver to which this *cmd* is addressed
 * \param[in]	cmd - the pointer to the command with parameters (only the firs byte is used)
 * \return		PROCESSING_COMPLETE - the command does not need further processing from the caller
 * \return		KEEP_PROCESSING - the command must be processed by the caller
 * \callgraph
 * \callergraph
 */
static tU32 ETAL_checkTuneSeekCommand_HDRADIO(ETAL_HANDLE *hReceiverList, ETAL_HANDLE hReceiver, tU8 *cmd)
{
	tyHDRADIOInstanceID instanceId;
	tU32 index;
	tU32 ret = KEEP_PROCESSING;

	if ((HC_COMMAND_PACKET_GET_OPCODE(cmd) == (tU8)HDRADIO_SEEK_START_SPECIAL) ||
		(HC_COMMAND_PACKET_GET_OPCODE(cmd) == (tU8)HDRADIO_SEEK_STOP_SPECIAL))
	{

		/*
		 * INSTANCE_UNDEF is a valid return in this case and it is processed
		 * later so we can safely ignore the return value
		 */
		(LINT_IGNORE_RET) ETAL_receiverGetHdInstance(hReceiver, &instanceId);
		if (instanceId == INSTANCE_UNDEF)
		{
			/*
			 * any valid SEEK_START/SEEK_STOP command must have a valid instanceId,
			 * thus an INSTANCE_UNDEF could mean the command is referring to a receiver
			 * that has been reconfigured before the command was executed;
			 * ignore
			 */
			ret = PROCESSING_COMPLETE;
			goto exit;
		}
		index = (tU32)instanceId - 1;
		if (HC_COMMAND_PACKET_GET_OPCODE(cmd) == (tU8)HDRADIO_SEEK_START_SPECIAL)
		{
			hReceiverList[index] = hReceiver;
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Starting HD FSM recv %d, inst %d", hReceiver, instanceId);
			ret = PROCESSING_COMPLETE;
			goto exit;
		}
		else if (HC_COMMAND_PACKET_GET_OPCODE(cmd) == (tU8)HDRADIO_SEEK_STOP_SPECIAL)
		{
			if (hReceiverList[index] == hReceiver)
			{
				/*
				 * ETAL_tuneFSM_HDRADIO with cmdTuneRestart returns immediately
				 * without changing the receiver state, no need to take the receiver lock
				 */
				(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, ETAL_receiverGetFrequency(hReceiver), tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_INTERNAL_USER);

				hReceiverList[index] = ETAL_INVALID_HANDLE;
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Stopping HD FSM recv %d, inst %d", hReceiver, instanceId);
			}
			else
			{
				/*
				 * received a stop command for a receiver which was not yet started
				 * ignore
				 */
			}
			ret = PROCESSING_COMPLETE;
			goto exit;
		}
		else
		{
			/* Nothing to do */
		}
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_processResponse_HDRADIO
 *
 **************************/
/*!
 * \brief		Processes a response received from an HD Radio Receiver
 * \details		The function parses the response to one of the following commands:
 * 				- HDRADIO_SYS_TUNE_CMD
 * 				- HDRADIO_PSD_DECODE_CMD
 * 				- HDRADIO_GET_EXT_SIS_DATA_CMD
 *
 * 				The first one is parsed to get the HD Radio status, needed by
 * 				the code that processes the HDRADIO_PSD_DECODE_CMD response.
 * 				The other two are parsed to extract the Textinfo information;
 * 				if the information is new, the function also invokes the
 * 				Datahandler callback function.
 * \param[in]	hReceiver - handle of the Receiver from which the response is coming,
 * 				            used only for error reporting.
 * \param[in]	hDatapath - handle of the Datapath to be used to process the Textinfo
 * 				            The function assumes that the Datapath id of the correct type
 * 				            (i.e. #ETAL_DATA_TYPE_TEXTINFO).
 * \param[in]	resp - the array of byte containing the response to be processed
 * \param[in]	rlen - the length in bytes of *resp*
 * \callgraph
 * \callergraph
 * \todo		 only MPS supported for HDRADIO_PSD_DECODE_CMD: implement for the other Audio Programs
 */
static tVoid ETAL_processResponse_HDRADIO(ETAL_HANDLE hReceiver, ETAL_HANDLE hDatapath, tU8 *resp, tSInt rlen)
{
	static etalToHDRADIODigiAcqStatusTy tDigiAcqStatus; /* static for persistency */

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	static tChar title[ETAL_DEF_MAX_INFO_TITLE];
	static tChar artist[ETAL_DEF_MAX_INFO_ARTIST];
	static tChar old_info[ETAL_DEF_MAX_INFO] = "\0";
	static EtalTextInfo radiotext;
	EtalBcastDataType datapathType;
	tU32 size;
	const tU32 PSDDataType = ETAL_DATASERV_TYPE_PSD;
	tU8 *tmp_payload;
#endif

	switch (HC_COMMAND_PACKET_GET_OPCODE(resp))
	{
		case HDRADIO_SYS_TUNE_CMD:
			if (HC_COMMAND_PACKET_GET_FUNCODE(resp) == (tU8)TUNE_GET_STATUS)
			{
				/*  Checking the Return Data */
				if (rlen < ETAL_HDRADIO_TUNESTATUS_DIG_MIN_FIELD_LEN)
				{
					(void)OSAL_pvMemorySet((tVoid *)&tDigiAcqStatus, 0, sizeof(etalToHDRADIODigiAcqStatusTy));

					if ((rlen == ETAL_HDRADIO_TUNESTATUS_DIG_SPECIALCASE_FIELD_LEN) &&
						(resp[ETAL_HDRADIO_TUNE_OPERATION_GETSTATUS_BAND_OFFSET] == (tU8)IDLE_MODE))
					{
						/*
						 * If the CMOST has not yet completed the tune operation
						 * the 680 will respond to the SYS_TUNE(TUNE_GET_STATUS) with an undocumented
						 * response format:
						 *  0x06 0x02 0x63 0x00 0x00
						 * where
						 *  0x06 = function code, TUNE_GET_STATUS
						 *  0x02 = only digital status requested
						 *  0x63 = Band byte signaling 'idle state'
						 *         (according to iBiquity spec this byte should only be 0x00 for AM or 0x01 for FM)
						 *  0x00 = frequency LSB
						 *  0x00 = frequency MSB
						 *
						 * This is not an error
						 */
						break;
					}
					ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_MessageFormatError, 0, resp, (tU32)rlen);
					break;
				}

				if (resp[ETAL_HDRADIO_TUNE_OPERATION_OFFSET] == (tU8)TUNE_OPERATION_PENDING)
				{
					tDigiAcqStatus.m_TuneOperationType = (tU8)TUNE_OPERATION_PENDING;
				}
				else
				{
					(void)OSAL_pvMemoryCopy((tVoid *)&tDigiAcqStatus, (tPCVoid)&resp[ETAL_HDRADIO_TUNE_OPERATION_OFFSET], sizeof(etalToHDRADIODigiAcqStatusTy));
				}
			}

			break;

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
		case HDRADIO_PSD_DECODE_CMD:
			if (HC_COMMAND_PACKET_GET_FUNCODE(resp) == (tU8)GET_PSD_DECODE)
			{
				/* TODO: see function header */
				if (tDigiAcqStatus.m_TuneOperationType == (tU8)ONLY_DIGITAL_TUNED)
				{
					if (rlen < ETAL_HDRADIO_GETPSD_MIN_FIELD_LEN + 1 * ETAL_HDRADIO_GETPSD_MIN_INFO_LEN)  // '1' because of MPS only supported
					{
						//ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_MessageFormatError, 0, resp, (tU32)rlen);
						break;
					}

					//Determine datapath type (RadioText or DataService)
					datapathType = ETAL_receiverGetDataTypeForDatapath(hDatapath);

					if (ETAL_DATA_TYPE_TEXTINFO == datapathType)
					{
						if (ETAL_parsePSDDecode_HDRADIO(resp, (tU16)rlen, (tU8)1, title, ETAL_DEF_MAX_INFO_TITLE, artist, ETAL_DEF_MAX_INFO_ARTIST) != OSAL_OK)
						{
							ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_GenericError, 0, resp, (tU32)rlen);
							break;
						}

						ETAL_stringToRadiotext_HDRADIO(&radiotext, title, artist);
						radiotext.m_broadcastStandard = ETAL_receiverGetStandard(hReceiver);
						if (OSAL_s32StringNCompare(old_info, radiotext.m_currentInfo, ETAL_DEF_MAX_INFO) == 0)
						{
							radiotext.m_currentInfoIsNew = FALSE;
						}
						else
						{
							OSAL_szStringNCopy(old_info, radiotext.m_currentInfo, (ETAL_DEF_MAX_INFO-1));
							old_info[ETAL_DEF_MAX_INFO-1] = '\0';
						}
						/*
						 * invoke the data sink callback for Radiotext
						 */
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Invoking the callback for HDRADIO_PSD_DECODE_CMD");
						if ((radiotext.m_currentInfoIsNew) || (true == v_hd_ForceRadioTextInfoResend))
						{
							v_hd_ForceRadioTextInfoResend = false;
							(void)ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)&radiotext, sizeof(EtalTextInfo), NULL);
						}
					}
					else if (ETAL_DATA_TYPE_DATA_SERVICE == datapathType)
					{
						/* Send RAW data to the callback only in case of PSD change
						 * Should use EtalGenericDataServiceRaw as specified in ETAL spec :
						 *  - DataType (ETAL_DATASERV_TYPE_PSD) on 4 bytes
						 *  - then m_data as PSD data structure
						 * PSD data structure is same as in DCOP response ; it is found directly at payload offset
						 */
						if ((tDigiAcqStatus.m_PSDChangeFlag & (tU8)MPS_AUDIO_MASK) != (tU8)0)
						{
							// size should also be equal to rlen - HDRADIO_CP_OVERHEAD
							size = HC_COMMAND_PACKET_GET_DATALEN(resp);
							ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "HDRADIO_PSD_DECODE_CMD response size : %d", size);
							tmp_payload = (tU8 *)OSAL_pvMemoryAllocate(size+4);
							if (tmp_payload == NULL)
							{
								ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, \
									(tVoid *)((tULong)hDatapath), sizeof(ETAL_HANDLE));
								goto exit;
							}
							(void)OSAL_pvMemoryCopy((tVoid *)tmp_payload, (tPCVoid)(&PSDDataType), 4);
							(void)OSAL_pvMemoryCopy((tVoid *)(tmp_payload+4), (tPCVoid)(resp+HDRADIO_CP_PAYLOAD_INDEX), size);
							resp = tmp_payload;
							size = size+4;
						
							/*
							 * invoke the data sink callback for Data Service
							 */
							ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, \
								"Invoking the Data Service callback for HDRADIO_PSD_DECODE_CMD");
							(void)ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)resp, size, NULL);
						}
					}
					else
					{
				   		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "HDRADIO processResponse: Unknown Data Type");
					}
				}
			}

			break;

		case HDRADIO_GET_EXT_SIS_DATA_CMD:
			if (HC_COMMAND_PACKET_GET_FUNCODE(resp) == (tU8)GET_BASIC_SIS_DATA)
			{
				/* TODO: see function header */
//				if (tDigiAcqStatus.m_TuneOperationType == (tU8)ONLY_DIGITAL_TUNED && (tDigiAcqStatus.m_PSDChangeFlag & (tU8)MPS_AUDIO_MASK) != (tU8)0)
				if ((tDigiAcqStatus.m_TuneOperationType == (tU8)ONLY_DIGITAL_TUNED)
					&&
					(tDigiAcqStatus.m_DigiAcquisitionStatus != (tU8)0))

				{
					if (rlen < ETAL_HDRADIO_GETBASICSIS_MIN_FIELD_LEN)
					{
						//ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_MessageFormatError, 0, resp, (tU32)rlen);
						break;
					}
					if (ETAL_parseSIS_HDRADIO(resp, (tU16)rlen, radiotext.m_serviceName, ETAL_DEF_MAX_SERVICENAME, &(radiotext.m_serviceNameIsNew)) != OSAL_OK)
					{
						ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_GenericError, 0, resp, (tU32)rlen);
						break;
					}
					radiotext.m_broadcastStandard = ETAL_receiverGetStandard(hReceiver);

					/*
					 * invoke the data sink callback for Radiotext
					 */
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Invoking the callback HDRADIO_GET_EXT_SIS_DATA_CMD");
					if ((radiotext.m_serviceNameIsNew) || (true == v_hd_ForceStationInfoResend))
					{
						v_hd_ForceStationInfoResend = false;
						(void)ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)&radiotext, sizeof(EtalTextInfo), NULL);
					}
				}
			}

			break;
#endif // #if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

		default:
			ETAL_sendCommunicationErrorEvent(hReceiver, EtalCommStatus_GenericError, 0, resp, (tU32)rlen);
			break;
	}

exit:
	return;
}

/***************************
 *
 * ETAL_CommunicationLayer_ThreadEntry_HDRADIO
 *
 **************************/
/*!
 * \brief		HD Radio communication thread entry point
 * \details		This thread processes the HD Radio commands that need to be
 * 				executed in the background, namely the async tune and seek
 * 				commands and the Textinfo related commands.
 * 				The function performs the following steps:
 * 				- advances the Tune state machines for all the registered
 * 				  Receivers (registration happens when the thread receives
 * 				  a #HDRADIO_SEEK_START_SPECIAL command for a Receiver). This
 * 				  is performed by the #ETAL_processTuneSeekFSM_HDRADIO function.
 * 				- pops a command from the HD Radio command FIFO
 * 				- processes any special commands received from the command FIFO
 * 				  (function #ETAL_checkTuneSeekCommand_HDRADIO).
 * 				- sends command to the HD Radio device and reads response
 * 				  (function #ETAL_sendCommandTo_HDRADIO). The only commands supported
 * 				  are those related to getting the tune status and reading
 * 				  the Textinfo.
 * 				- parses the responses, generates the Textinfo and invokes the
 * 				  Textinfo Datahandler (function #ETAL_processResponse_HDRADIO).
 *
 * \param[in]	pvArg - unused
 * \callgraph
 * \callergraph
 */
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETAL_CommunicationLayer_ThreadEntry_HDRADIO(tSInt stacd, tPVoid pvArg)
#else
static tVoid ETAL_CommunicationLayer_ThreadEntry_HDRADIO(tPVoid pvArg)
#endif
{
	static tU8 CP_threadBuf[HDRADIO_CP_FULL_SIZE]; /* static for size, not for persistency */
#ifndef HD_MONITORING
	static ETAL_HANDLE hReceiverForTune[ETAL_MAX_HDINSTANCE];
#endif
	ETAL_HANDLE hReceiver;
	ETAL_HANDLE hDatapath;
	tU8 *resp = CP_threadBuf;
	tU8 *cmd = CP_threadBuf;
	tU32 clen;
	etalToHDRADIOStatusTy cstat;
	tSInt rlen;
	tU32 index;
	EtalCommErr err;
	tU32 vl_res;

	for (index = 0; index < ETAL_MAX_HDINSTANCE; index++)
	{
		hReceiverForTune[index] = ETAL_INVALID_HANDLE;
	}

	while (TRUE)
	{

		/*
		 * Take the global lock to avoid receiver reconfiguration
		 * while processing commands
		 */
		ETAL_statusGetInternalLock();

		ETAL_processTuneSeekFSM_HDRADIO(hReceiverForTune, ETAL_MAX_HDINSTANCE);

		// a message may have been posted for HD DCOP 
		// check the FIFO if we want to avoid 10 ms loss before processing on next thread wait...
		//
		
		if (ETAL_CmdToHDRADIOFifoPop(&hDatapath, cmd, &clen) == OSAL_OK)
		{
			/*
			 * Checks for commands requesting to start or stop the HDRadio Tune/Seek state machine
			 * Note that for these commands the handle stored in the FIFO is actually
			 * a Receiver, not a Datapath
			 */
	
			ETAL_getHDRADIOReceiverMonitoringLock();
			vl_res = ETAL_checkTuneSeekCommand_HDRADIO(hReceiverForTune, hDatapath, cmd);
			ETAL_releaseHDRADIOReceiverMonitoringLock();
			
			if ( vl_res == KEEP_PROCESSING)
			{
				/*
				 * Regular HDRadio command processing
				 */

				ETAL_getHDRADIODevLock();

				hReceiver = ETAL_receiverGetFromDatapath(hDatapath);
				
				if (ETAL_sendCommandTo_HDRADIO(hReceiver, cmd, clen, &resp, &rlen) == OSAL_OK)
				{
					ETAL_processResponse_HDRADIO(hReceiver, hDatapath, resp, rlen);
				}
				else
				{
					cstat.B = resp[rlen - 1];
					err = ETAL_commandStatusToEventErrStatus_HDRADIO(cstat);
					ETAL_sendCommunicationErrorEvent(hReceiver, err, (tU32)cstat.B, resp, (tU32)rlen);
				}
				ETAL_releaseHDRADIODevLock();
			}
		}

		// Release Internal Lock
		ETAL_statusReleaseInternalLock();
						
		(void)OSAL_s32ThreadWait(ETAL_COMM_HDRADIO_THREAD_SCHEDULING);
	}
}

/***************************
 *
 * ETAL_initCommunication_HDRADIO
 *
 **************************/
/*!
 * \brief		Starts up the ETAL HD Radio communication layer
 * \details		The function performs the following operations:
 * 				- configures the Operating System drivers required to access the HD Radio device
 * 				- initializes the HD Radio command FIFO
 * 				- creates the semaphores
 * 				- starts the communication thread
 *
 * \remark		All of the errors below should be treated as fatal errors, communication
 * 				with the HD Radio device will not be possible.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error creating the semaphores
 * \return		OSAL_ERROR_DEVICE_INIT - error creating the communication thread
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - error accessing the Operating System driver, or the Protocol Layer
 * 				                             in case of external driver
 * \callgraph
 * \callergraph
 */
tSInt ETAL_initCommunication_HDRADIO(tBool isBootMode, tBool vI_manageReset)
{
	tChar semname[ETAL_SEM_NAME_LEN_HDRADIO];
	OSAL_trThreadAttribute thread_attr;
	tU32 i;
	tyHDRADIODeviceConfiguration HDDeviceConfig;
	tSInt ret = OSAL_OK;

	ETAL_getDeviceConfig_HDRADIO(&HDDeviceConfig);

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
    HDRADIO_setTcpIpAddress(ETAL_getIPAddressForExternalDriver());
#endif

#ifdef CONFIG_COMM_HDRADIO_SPI
	if(HDDeviceConfig.communicationBusType == BusSPI)
	{
		if(isBootMode == TRUE)
		{
			HDDeviceConfig.communicationBus.spi.mode = SPI_CPHA0_CPOL0;
			HDDeviceConfig.communicationBus.spi.speed = HDRADIO_ACCORDO2_SPI_SPEED_LOAD_PHASE1;
//			HDRADIO_reconfiguration(&HDDeviceConfig);
		}
	}
#endif

	if (HDRADIO_init(&HDDeviceConfig) != OSAL_OK)
	{
		ETAL_initStatusSetDCOPStatus(deviceCommunication);
		ret = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}

	if (TRUE == vI_manageReset)
	{
		if (HDRADIO_reset() != OSAL_OK)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}

		/*
		 * Time to load the FW from flash and execute it
		 */
		 if (FALSE == isBootMode)
		 {
			(void)OSAL_s32ThreadWait(HDRADIO_FW_LOADING_TIME);
		 }	
	}

	if (ETAL_initCommandFIFO_HDRADIO(&ETAL_StatusHDRADIO) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

	for (i = 0; i < ETAL_MAX_HDINSTANCE; i++)
	{
		if (OSAL_s32NPrintFormat(semname, ETAL_SEM_NAME_LEN_HDRADIO, "%s%u", ETAL_SEM_TUNEFSM_HDRADIO_BASE, i) < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
		if (OSAL_s32SemaphoreCreate(semname, &ETAL_TuneSem_HDRADIO[i], 1) != OSAL_OK)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
	}

	if (OSAL_s32SemaphoreCreate(ETAL_SEM_DEVICE_HDRADIO, &ETAL_StatusHDRADIODevSem, 1) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}


	if (OSAL_s32SemaphoreCreate(ETAL_SEM_RECLIST_HDRADIO_BASE, &ETAL_HDRadio_ReceiverMonitoringSem, 1) != OSAL_OK)
	{
		ret = OSAL_ERROR;
		goto exit;
	}
	

	thread_attr.szName       = ETAL_COMM_HDRADIO_THREAD_NAME;
	thread_attr.u32Priority  = ETAL_COMM_HDRADIO_THREAD_PRIORITY;
	thread_attr.s32StackSize = ETAL_COMM_HDRADIO_STACK_SIZE;
	thread_attr.pfEntry = &ETAL_CommunicationLayer_ThreadEntry_HDRADIO;
	thread_attr.pvArg = (tPVoid)NULL;

	ETAL_CommToHDRADIOThreadId = OSAL_ThreadSpawn(&thread_attr);
   	if (ETAL_CommToHDRADIOThreadId == OSAL_ERROR)
   	{
   		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL_COMM, "HDRADIO thread creation");
   		ret = OSAL_ERROR_DEVICE_INIT;
   		goto exit;
   	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_deinitCommunication_HDRADIO
 *
 **************************/
/*!
 * \brief		Destroys all the resources used by the ETAL HD Radio driver
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error while releasing one of the resources.
 * 				             The function tries anyway to release as many resources
 * 				             as possible.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_deinitCommunication_HDRADIO(tVoid)
{
	tChar semname[ETAL_SEM_NAME_LEN_HDRADIO];
	tU32 i;
	tSInt ret = OSAL_OK;

	if ((ETAL_CommToHDRADIOThreadId != OSAL_ERROR) &&
		(OSAL_s32ThreadDelete(ETAL_CommToHDRADIOThreadId) == OSAL_OK))
	{
		ETAL_CommToHDRADIOThreadId = OSAL_ERROR; /* invalidate the ID */
		/* give the killed thread the opportunity to do cleanup if required */
		(void)OSAL_s32ThreadWait(ETAL_API_THREAD_SCHEDULING);
	}
	else
	{
		ret = OSAL_ERROR;
	}

	if (OSAL_s32SemaphoreClose(ETAL_StatusHDRADIODevSem) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else if (OSAL_s32SemaphoreDelete(ETAL_SEM_DEVICE_HDRADIO) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else
	{
		/* Nothing to do */
	}

	if (OSAL_s32SemaphoreClose(ETAL_HDRadio_ReceiverMonitoringSem) != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	else if (OSAL_s32SemaphoreDelete(ETAL_SEM_RECLIST_HDRADIO_BASE) != OSAL_OK)
	{
		
		ret = OSAL_ERROR;
	}
	else
	{
		/* Nothing to do */
	}


	for (i = 0; i < ETAL_MAX_HDINSTANCE; i++)
	{
		if (OSAL_s32SemaphoreClose(ETAL_TuneSem_HDRADIO[i]) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
		else 
		{
			if (OSAL_s32NPrintFormat(semname, ETAL_SEM_NAME_LEN_HDRADIO, "%s%u", ETAL_SEM_TUNEFSM_HDRADIO_BASE, i) < 0)
			{
				ret = OSAL_ERROR;
			}
			else if (OSAL_s32SemaphoreDelete(semname) != OSAL_OK)
			{
				ret = OSAL_ERROR;
			}
			else
			{
				/* Nothing to do */
			}
		}
	}
	COMMON_fifoDeinit(&ETAL_StatusHDRADIO.hdFifo, ETAL_FIFO_HDRADIO_NAME);

	if(HDRADIO_deinit() != OSAL_OK)
	{
		ret = OSAL_ERROR;
	}
	return ret;
}

/***************************
 *
 * ETAL_queueCommand_HDRADIO
 *
 **************************/
/*!
 * \brief		Sends a command to the HD Radio for delayed (or async) processing
 * \details		The function pushes the command to the HD Radio communication
 * 				thread for processing in the background. Not all commands can
 * 				be issued this way: for the list see #ETAL_processResponse_HDRADIO.
 * \param[in]	hDatapath - the handle of the Datapath associated with this command.
 * 				            A Datapath handle is used instead of a Receiver because
 * 				            the subset of commands supported by the communication thread
 *				            operates on a Datapath.
 * \param[in]	cmd - pointer to the command buffer
 * \param[in]	clen - length of the command
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error pushing the command to the HD Radio command FIFO
 * \callgraph
 * \callergraph
 */
tSInt ETAL_queueCommand_HDRADIO(ETAL_HANDLE hDatapath, tU8 *cmd, tU32 clen)
{
	tSInt ret = OSAL_OK;

	if (ETAL_CmdToHDRADIOFifoPush(hDatapath, cmd, clen) != OSAL_OK)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_CMD, "Error queuing HDRADIO command");	
		ret = OSAL_ERROR;
	}

	return ret;
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
