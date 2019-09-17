//!
//!  \file 		etalcomm_mdr.c
//!  \brief 	<i><b> ETAL communication layer for MDR devices </b></i>
//!  \details   Communication with MDR3 device
//!  \author 	Raffaele Belardi
//!
#include "osal.h"
#include "etalinternal.h"
#include "common_trace.h"

#ifdef CONFIG_COMM_DRIVER_DIRECT
#include "rif_tasks.h"
#include "rif_msg_queue.h"
#include "rif_protocol_router.h"
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR


/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

#define ETAL_MDR_ILLEGAL_COMMAND_ID            0xFFFF

#define DABMW_DATACHANNEL_HEADER0                (tU8)0xD6
#define DABMW_DATACHANNEL_HEADER1_ASCII          (tU8)0x84
#define DABMW_DATACHANNEL_GET_MSGTYPE(_buf_)  (((_buf_)[1] >> 5) & 0x07)
#define DABMW_DATACHANNEL_GET_APP(_buf_)      ((tU32)((_buf_)[1] >> 4) & 0x01)
#define DABMW_DATACHANNEL_GET_VERSION(_buf_)  ((tU32)((_buf_)[1] >> 2) & 0x01)
#define DABMW_DATACHANNEL_GET_TYPE(_buf_)      (DABMWDataServiceType)((_buf_)[4] & 0xFF)
#define DABMW_DATACHANNEL_GET_PACKETADDRESS(_buf_)(((_buf_)[2] & 0xFF) | (((_buf_)[3] << 2) & 0x0300))
#define DABMW_DATACHANNEL_GET_SUBCH(_buf_)     ((_buf_)[3] & 0x3F)
#define DABMW_DATACHANNEL_GET_SIZE(_buf_)    ((((tU32)(_buf_)[5] << 12) & 0x0F0000) | (((tU32)(_buf_)[6] << 8) & 0x00FF00) | (((tU32)(_buf_)[7] << 0) & 0x0000FF))
#define DABMW_DATACHANNEL_GET_CONTINUITY(_buf_)((_buf_)[5] & 0x0F)
#define DABMW_DATACHANNEL_GET_PAYLOAD(_buf_)   ((_buf_) + 8)
#define DABMW_RAWDATACHANNEL_GET_SIZE(_buf_)(((((tU32)(_buf_)[0] >> 6) & 0x03) + 1) * 24)

#define DABMW_ASCIICHANNEL_GET_CONTINUITY(_buf_)((_buf_)[3] & 0x0F)

/* DABMW Autonotification */
#define DABMW_AUTONOTIFICATION_HEADER0_MASK      0xF5
#define DABMW_AUTONOTIFICATION_HEADER0           0xF0
#define DABMW_AUTONOTIFICATION_TYPE_OFFSET          2

#define DABMW_AUTONOTIFICATION_DAB_STATUS        0x01
#define DABMW_AUTONOTIFICATION_RECONFIGURATION   0x02
#define DABMW_AUTONOTIFICATION_ANNOUNCEMENT      0x03
#define DABMW_AUTONOTIFICATION_ANNOUNCEMENT_RAW  0x05
#define DABMW_AUTONOTIFICATION_DAB_DATA_STATUS   0x06
#define DABMW_AUTONOTIFICATION_TUNEREQUEST       0xFE

#define DABMW_AUTONOTIFICATION_GET_SIZE(_buf_) ((_buf_)[3])
#define DABMW_AUTONOTIFICATION_DAB_STATUS_SIZE         12
#define DABMW_AUTONOTIFICATION_RECONFIGURATION_SIZE    10
#define DABMW_AUTONOTIFICATION_DAB_DATA_STATUS_SIZE    2
#define DABMW_AUTONOTIFICATION_ANNOUNCEMENT_SIZE       14
#define DABMW_AUTONOTIFICATION_ANNOUNCEMENT_RAW_SIZE   6
#define DABMW_AUTONOTIFICATION_TUNE_REQ_SIZE           5

#define DABMW_DATACHANNEL_DATAGROUP_HEADER_SIZE 5

/*
 * continuity index for Data Channel protocol is 4 bits wide,
 * so this value is surely not valid
 */
#define DABMW_DATACHANNEL_CONTINUITY_ILLEGAL     (tU8)0xFF
#define DABMW_DATACHANNEL_CONTINUITY_MAX         0x10

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
static OSAL_tThreadID       ETAL_commThreadId_MDR; /* only used for deinit */
static OSAL_tSemHandle      ETAL_toMDRCmdBufferSem;
static etalToMDRCmdTy       ETAL_toMDRCmdBuffer;
static etalToMDRCmdStatusTy ETAL_toMDRCmdStatus;
static tU16                 ETAL_toMDROutstandingCmdId;
static tBool                ETAL_toMDRCmdBufferHaveNew;
#ifdef CONFIG_COMM_DRIVER_DIRECT
static tBool                ETAL_directToMDRCmdMode;
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

static tU8                  ETAL_continuityIndexExpected;

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
#ifdef CONFIG_HOST_OS_TKERNEL
	static tVoid ETAL_CommunicationLayer_ThreadEntry_MDR(tSInt stacd, tPVoid dummy);
#else
	static tVoid ETAL_CommunicationLayer_ThreadEntry_MDR(tPVoid dummy);
#endif

static tVoid ETAL_CommunicationLayer_Receive_MDR(tU8 *data, tU32 len, etalToMDRCmdTy *cmd, etalToMDRCmdStatusTy *status);
static tVoid ETAL_CommunicationLayer_ReceiveData_MDR(tU8 *data, tU32 len);
static tVoid ETAL_CommunicationLayer_ReceiveAutonotification_MDR(tU8 *data, tU32 len);
static tVoid ETAL_CommunicationLayer_ReceiveBroadcast_MDR(tU8 *data, tU32 len, etalToMDRCmdTy *cmd, etalToMDRCmdStatusTy *status);



#ifdef CONFIG_ETAL_CPU_IMPROVEMENT

static OSAL_tEventHandle ETAL_commMDR_TaskEventHandler;
static OSAL_tSemHandle      ETAL_toMDRCmdMsgTransmitted;
static OSAL_tSemHandle      ETAL_toMDRCmdMsgComplete;


//static tVoid ETAL_CommMdr_TaskClearEvent (tU32 event);
static tVoid ETAL_CommMdr_TaskClearEventFlag(tU32 eventFlag);
static tVoid ETAL_CommMdr_TaskWakeUpOnEvent (tU32 event);
static tVoid ETAL_CommMdr_ManageDataReception(tVoid);

// External function to manage the signals  
tVoid ETAL_CommMdr_DataToTx(tVoid);
tVoid ETAL_CommMdr_DataRx(tVoid);
	

/// WAITING TASK 

// EVENTS

// Data To Tx event bit 0
#define	ETAL_COMM_MDR_EVENT_DATA_TO_TX				0

// Data Rx event bit 0
#define ETAL_COMM_MDR_EVENT_DATA_RX					1


// FLAGS
#define	ETAL_COMM_MDR_EVENT_DATA_TO_TX_FLAG				((tU32)0x01 << ETAL_COMM_MDR_EVENT_DATA_TO_TX)	

#define ETAL_COMM_MDR_EVENT_DATA_RX_FLAG				((tU32)0x01 << ETAL_COMM_MDR_EVENT_DATA_RX)


// WAKEUP FULL FLAGS & MASK
#define ETAL_COMM_MDR_EVENT_FLAGS					(ETAL_COMM_MDR_EVENT_DATA_TO_TX_FLAG | ETAL_COMM_MDR_EVENT_DATA_RX_FLAG)

#define ETAL_COMM_MDR_EVENT_WAIT_MASK				(ETAL_COMM_MDR_EVENT_FLAGS)

#define ETAL_COMM_MDR_EVENT_WAIT_ALL						(0xFFFFFFFF)

#define ETAL_COMM_MDR_EVENT_WAIT_TIME_MS				OSAL_C_TIMEOUT_FOREVER

#define ETAL_COMM_MDR_NO_EVENT_FLAG					0x00

#endif

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
#if 0
//!
//! \brief      <i><b> tVoid ETAL_CommMdr_TaskClearEvent (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_CommMdr_TaskClearEvent (tU32 event)
{

		 // Clear old event if any (this can happen after a stop)
	 ETAL_TaskClearEvent (ETAL_commMDR_TaskEventHandler, event);
}
#endif
//!
//! \brief      <i><b> tVoid ETAL_CommMdr_TaskClearEventFlag (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_CommMdr_TaskClearEventFlag(tU32 eventFlag)
{

	ETAL_TaskClearEventFlag(ETAL_commMDR_TaskEventHandler, eventFlag);
}

//!
//! \brief      <i><b> ETAL_CommMdr_TaskWakeUpOnEvent </b></i>
//! \details    Functions to wake-up the middlewar task on event.
//!             The main loop for the task is waked-up here.
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
static tVoid ETAL_CommMdr_TaskWakeUpOnEvent (tU32 event)
{
 
	if ((ETAL_COMM_MDR_EVENT_DATA_TO_TX == event)
		|| (ETAL_COMM_MDR_EVENT_DATA_RX == event)
		)
		{
		ETAL_TaskWakeUpOnEvent(ETAL_commMDR_TaskEventHandler, event);
		}
}


tVoid ETAL_CommMdr_DataToTx(tVoid)
{
	ETAL_CommMdr_TaskWakeUpOnEvent(ETAL_COMM_MDR_EVENT_DATA_TO_TX);
}

tVoid ETAL_CommMdr_DataRx(tVoid)
{
	ETAL_CommMdr_TaskWakeUpOnEvent(ETAL_COMM_MDR_EVENT_DATA_RX);
}


static tVoid ETAL_CommMdr_ManageDataReception(tVoid)
{
	tU32 len;
	tU8 lun;
	tBool got_data = FALSE;
	
	/*
	 * This buffer is used to hold data received from the DCOP
	 * The max size depends on the LUN (e.g. ETAL_CONTROL_LUN has much
	 * smaller max size than ETAL_DATA_LUN) it must be dimensioned
	 * to accept the largest payload
	 */
	static tU8 data[ETAL_MAX_RESPONSE_LEN];

	do
	{
		got_data = FALSE;
		if (DAB_Get_Response(data, ETAL_MAX_RESPONSE_LEN, &len, &lun)== OSAL_OK)
		{
		    if(len != 0)
		    {
		        got_data = TRUE;
		    }
		}

		if (got_data)
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received message ID 0x%.3x from DCOP on LUN 0x%.2x, %d bytes", ETAL_MDR_COMMAND_ID(data), lun, len);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
			COMMON_tracePrintBufComponent(TR_CLASS_APP_ETAL_COMM, data, len, NULL);
#endif
#ifdef CONFIG_COMM_DRIVER_DIRECT
			if (ETAL_directToMDRCmdMode == FALSE)
			{
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT
				switch (lun)
				{
					case ETAL_CONTROL_LUN:
						ETAL_CommunicationLayer_Receive_MDR(data, len, &ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus);
						break;
					case ETAL_DATA_LUN:
						ETAL_CommunicationLayer_ReceiveData_MDR(data, len);
						break;
					case ETAL_AUTONOTIF_LUN:
						ETAL_CommunicationLayer_ReceiveAutonotification_MDR(data, len);
						break;
					case ETAL_BROADCAST_LUN:
						ETAL_CommunicationLayer_ReceiveBroadcast_MDR(data, len, &ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus);
						break;
					default:
						ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unexpected message from DCOP on LUN 0x%x", lun);
					break;
				}
#ifdef CONFIG_COMM_DRIVER_DIRECT
			}
		else
			{
				ETAL_CommunicationLayer_Receive_Direct_MDR(data, len, &ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus, lun);
			}
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "DONE / message ID 0x%.3x from DCOP on LUN 0x%.2x, %d bytes", ETAL_MDR_COMMAND_ID(data), lun, len);
		}
	} while (got_data);

	return;
}

#endif // CONFIG_ETAL_CPU_IMPROVEMENT

/***************************
 *
 * ETAL_ResetDabAutonotification_MDR
 *
 **************************/
tVoid ETAL_ResetDabAutonotification_MDR(etalReceiverStatusTy *recvp)
{
	if(recvp != NULL)
	{
		OSAL_pvMemorySet((tVoid *)&(recvp->DABStatusNotif), 0xFF, sizeof(etalAutoNotificationStatusTy));
	}
}

/***************************
 *
 * ETAL_ResetCommandBuffer_MDR
 *
 **************************/
static tVoid ETAL_ResetCommandBuffer_MDR(void)
{
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Resetting DCOP command buffer");
	OSAL_pvMemorySet((tVoid *)&ETAL_toMDRCmdBuffer, 0x00, sizeof(ETAL_toMDRCmdBuffer));
	ETAL_toMDRCmdStatus = cmdStatusInitMDR;
	ETAL_toMDRCmdBufferHaveNew = FALSE;
}

/***************************
 *
 * ETAL_initCommunicationPrimitives_MDR
 *
 **************************/
static tSInt ETAL_initCommunicationPrimitives_MDR(void)
{
	if (OSAL_s32SemaphoreCreate(ETAL_SEM_CMD_QUEUE_MDR, &ETAL_toMDRCmdBufferSem, 1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	ETAL_ResetCommandBuffer_MDR();
	ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;
	ETAL_continuityIndexExpected = DABMW_DATACHANNEL_CONTINUITY_ILLEGAL;

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT	

	// Create semaphore to control the message sending / FSM response wait
	// initial value 0 to be blocking

	if (OSAL_s32SemaphoreCreate(ETAL_SEM_COMM_MDR_MSG_TX, &ETAL_toMDRCmdMsgTransmitted, 0) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (OSAL_s32SemaphoreCreate(ETAL_SEM_COMM_MDR_MSG_COMPLETE, &ETAL_toMDRCmdMsgComplete, 0) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
		
#endif 

	return OSAL_OK;
}

/***************************
 *
 * ETAL_deinitCommunicationPrimitives_MDR
 *
 **************************/
static tSInt ETAL_deinitCommunicationPrimitives_MDR(void)
{
	tSInt retosal = OSAL_OK;

	if (OSAL_s32SemaphoreClose(ETAL_toMDRCmdBufferSem) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}
	if (OSAL_s32SemaphoreDelete(ETAL_SEM_CMD_QUEUE_MDR) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT	
	
	if (OSAL_s32SemaphoreClose(ETAL_toMDRCmdMsgTransmitted) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}
	if (OSAL_s32SemaphoreDelete(ETAL_SEM_COMM_MDR_MSG_TX) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}	

	if (OSAL_s32SemaphoreClose(ETAL_toMDRCmdMsgComplete) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}
	if (OSAL_s32SemaphoreDelete(ETAL_SEM_COMM_MDR_MSG_COMPLETE) != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}
#endif

	return retosal;
}

/***********************************
 *
 * ETAL_initCommunication_MDR
 *
 **********************************/
/*
* \param[in]	isBootMode - indicate if DCOP is started in fw download mode.
 * \param[in]	 vI_manageReset - indicate if the DCOP Reset should be managed
* 
 * Returns:
 *
 * OSAL_ERROR
 *  Semaphore creation
 *
 * OSAL_ERROR_DEVICE_INIT
 *  Memory allocation/thread allocation error
 *
 * OSAL_ERROR_DEVICE_NOT_OPEN
 *  Timeout while trying to communicate with ProtocolLayer
 *
 * OSAL_OK
 */
tSInt ETAL_initCommunication_MDR(EtalDcopBootType isBootMode, tBool vI_manageReset)
{
	OSAL_trThreadAttribute thread1_attr;
    tyDABDeviceConfiguration DABDeviceConfig;

    ETAL_getDeviceConfig_DAB(&DABDeviceConfig);

    if(isBootMode == ETAL_DCOP_BOOT_FLASH)
    {
        DABDeviceConfig.communicationBus.spi.speed = DAB_ACCORDO2_SPI_SPEED_FLASH_MODE;
        DABDeviceConfig.isBootMode = TRUE;
    }

    DAB_setTcpIpAddress(ETAL_getIPAddressForExternalDriver());

	if (DAB_Driver_Init(&DABDeviceConfig, vI_manageReset) != OSAL_OK)
	{
#ifdef CONFIG_COMM_DRIVER_EXTERNAL
		ETAL_initStatusSetDCOPStatus(deviceCommunication);
#endif
		return OSAL_ERROR_DEVICE_NOT_OPEN;
	}
	
#if 0
	if (TRUE == vI_manageReset)
	{
		/*
		 * This issues a HW reset to the DCOP and loads the FW from Flash
		 */
		if (DAB_Reset() != OSAL_OK)
		{
#ifdef CONFIG_COMM_DRIVER_EXTERNAL
			ETAL_initStatusSetDCOPStatus(deviceCommunication);
#endif
			return OSAL_ERROR_DEVICE_NOT_OPEN;
		}
	}
#endif

	if (ETAL_initCommunicationPrimitives_MDR() != OSAL_OK)
	{
		return OSAL_ERROR;
	}



#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	
	// Init globals
	ETAL_commMDR_TaskEventHandler = (OSAL_tEventHandle)0;
	OSAL_s32EventCreate ((tCString)ETAL_EVENT_HANDLER_COMM_MDR, &ETAL_commMDR_TaskEventHandler);	

#endif //  CONFIG_ETAL_CPU_IMPROVEMENT

	thread1_attr.szName = (tChar *)ETAL_COMM_MDR_THREAD_NAME;
	thread1_attr.u32Priority = ETAL_COMM_MDR_THREAD_PRIORITY;
	thread1_attr.s32StackSize = ETAL_COMM_MDR_STACK_SIZE;
	thread1_attr.pfEntry = ETAL_CommunicationLayer_ThreadEntry_MDR;
	thread1_attr.pvArg = NULL;

	ETAL_commThreadId_MDR = OSAL_ThreadSpawn(&thread1_attr);
	if (ETAL_commThreadId_MDR == OSAL_ERROR)
	{
		return OSAL_ERROR_DEVICE_INIT;
	}

	// manage the wait time after the reset 
	// in boot mode we do not need to wait the DCOP to load its fw
	if (ETAL_DCOP_BOOT_REGULAR == isBootMode)
	{
		if (TRUE == vI_manageReset)
		{
			// asume the reset is done and fw loaded
			OSAL_s32ThreadWait(DAB_FW_LOADING_TIME);
		}
	}

	return OSAL_OK;
}

/***********************************
 *
 * ETAL_initCommunication_MDR
 *
 **********************************/
/*
* \brief	   Primitive to reset the MDR
* \details	   The function performs the MDR Reset and only the Reset
*
* \param[in]	isBootMode - indicate if DCOP is started in fw download mode.

* 
 * Returns:
 *
 * OSAL_ERROR
 *  Semaphore creation
 *
 * OSAL_ERROR_DEVICE_INIT
 *  Memory allocation/thread allocation error
 *
 * OSAL_ERROR_DEVICE_NOT_OPEN
 *  Timeout while trying to communicate with ProtocolLayer
 *
 * OSAL_OK
 */
tSInt ETAL_Reset_MDR(tVoid)
{
	tSInt ret = OSAL_OK;
	
#ifdef CONFIG_COMM_DRIVER_EMBEDDED
	tyDABDeviceConfiguration DABDeviceConfig;

    ETAL_getDeviceConfig_DAB(&DABDeviceConfig);

	// Dab_Driver_init function is initializing the DAB driver, and without setting  the DCOP in reset
	if (DAB_StartupReset(&DABDeviceConfig) != OSAL_OK)
	{
		ret = OSAL_ERROR_DEVICE_NOT_OPEN;
	}
		
#elif defined (CONFIG_COMM_DRIVER_EXTERNAL)
	if (DAB_Reset() != OSAL_OK)
	{
		ret = OSAL_ERROR_DEVICE_NOT_OPEN;
	}
#endif
	return ret;
}


/***********************************
 *
 * ETAL_deinitCommunication_MDR
 *
 **********************************/
tSInt ETAL_deinitCommunication_MDR(tVoid)
{
	tSInt retosal = OSAL_OK;

		// this is the end of the task.
		// Delete the event and end

	if ((ETAL_commThreadId_MDR != OSAL_ERROR) && 
		(OSAL_s32ThreadDelete(ETAL_commThreadId_MDR) == OSAL_OK))
	{
		ETAL_commThreadId_MDR = OSAL_ERROR; /* invalidate the ID */
		// give the killed thread the opportunity to do cleanup if required
		OSAL_s32ThreadWait(ETAL_API_THREAD_SCHEDULING);
	}
	else
	{
		retosal = OSAL_ERROR;
	}
	
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		
#ifndef OSAL_EVENT_SKIP_NAMES
		if (OSAL_s32EventClose(ETAL_commMDR_TaskEventHandler) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
		
		OSAL_s32ThreadWait(100);
		
		if (OSAL_s32EventDelete(ETAL_EVENT_HANDLER_COMM_MDR) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
#else
		if (OSAL_s32EventFree(ETAL_commMDR_TaskEventHandler) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
#endif
	
		ETAL_commMDR_TaskEventHandler = (OSAL_tEventHandle)0;
	
#endif // CONFIG_ETAL_CPU_IMPROVEMENT

	if (ETAL_deinitCommunicationPrimitives_MDR() != OSAL_OK)
	{
		retosal = OSAL_ERROR;
	}

	retosal = DAB_Deinit();
	return retosal;
}

/***************************
 *
 * ETAL_CommunicationLayer_Send_MDR
 *
 **************************/
/*
 * Send to MDR using the CONTROL_LUN
 */
static tVoid ETAL_CommunicationLayer_Send_MDR(etalToMDRCmdTy *cmd, etalToMDRCmdStatusTy *status)
{
	if (cmd->cmd == NULL)
	{
		return;
	}

	if (ETAL_toMDRCmdBufferHaveNew)
	{
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Sending message ID 0x%.3x to DCOP DAB (%d)", ETAL_MDR_COMMAND_ID(cmd->cmd), cmd->cmdLen);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
		COMMON_tracePrintBufComponent(TR_CLASS_APP_ETAL_COMM, cmd->cmd, cmd->cmdLen, NULL);
#endif

		if (DAB_Send_Message(cmd->LunId, cmd->cmd,
            cmd->specific0, cmd->specific1, cmd->specific2, cmd->specific3, (tU16)cmd->cmdLen) < 0)
		{
			/* error condition, message not sent: don't wait for a response */
			ASSERT_ON_DEBUGGING(0);
			return;
		}

		*status = cmdStatusWaitNotificationMDR;
	
		ETAL_toMDRCmdBufferHaveNew = FALSE; // don't change the order, first update status then haveNew

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		// indicate message has been transmitted
		OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgTransmitted);
#endif			

        switch (cmd->LunId)
        {
            case ETAL_BROADCAST_LUN:
                /* only 8 bits used for cmdId in case of ETAL_BROADCAST_LUN */
                ETAL_toMDROutstandingCmdId = (tU16)cmd->specific0;
                break;
            case ETAL_CONTROL_LUN:
            case ETAL_DATA_LUN:
            case ETAL_AUTONOTIF_LUN:
                ETAL_toMDROutstandingCmdId = ETAL_MDR_COMMAND_ID(cmd->cmd);
                break;
            default:
                /* error unknown LUN */
                ASSERT_ON_DEBUGGING(0);
                return;
        }
	}
		
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	else
	{
		// in that config we should always come here with something to transmit...
		// this is strange...
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_Send_MDR : no message to send");
	}
#endif

	return;
}

/***************************
 *
 * ETAL_invokeEventCallback_MDR
 *
 **************************/
tVoid ETAL_invokeEventCallback_MDR(tU8 *data, tU32 len)
{
	void *param = NULL, *internal_callback_param = NULL;
	tU32 param_len, internal_callback_paramLen;
#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
	EtalSeamlessEstimationStatus seamlessEstimation;
	EtalSeamlessSwitchingStatus seamlessSwitching;
#endif
	// external callback event
	ETAL_EVENTS event;
	// internal callback event
	etalIntCbCallTy vl_intEvent = callNoEvent;
	EtalTuneStatus tune;
	EtalTuneInfoInternal vl_tuneStatusInternal;

	if (len == 0)
	{
		return;
	}

	switch (ETAL_MDR_COMMAND_ID(data))
	{
#if defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_SEAMLESS)
		case ETAL_MDR_SEAMLESS_ESTIMATION:
			if (ETAL_extractSeamlessEstimation_MDR(data, len, &seamlessEstimation) == OSAL_OK)
			{
				event = ETAL_INFO_SEAMLESS_ESTIMATION_END;
				vl_intEvent = callAtSeamlessEstimationResponse;
				param = (tU8 *) &seamlessEstimation;
				param_len = sizeof(seamlessEstimation);

				internal_callback_param = param;
				internal_callback_paramLen = param_len;

				// invoke the internal call back
				ETAL_intCbScheduleCallbacks(seamlessEstimation.m_receiverHandle, vl_intEvent, internal_callback_param, internal_callback_paramLen);

				// invoke the external call back
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, event, (tVoid *)param, param_len);
				
				if (true == ETAL_receiverIsSpecialInProgress(seamlessEstimation.m_receiverHandle, cmdSpecialExternalSeamlessEstimationRequestInProgress))
				{
				    // Disable external seamless estimation event
					ETAL_receiverSetSpecial(seamlessEstimation.m_receiverHandle, cmdSpecialExternalSeamlessEstimationRequestInProgress, cmdActionStop);
				}
			}
		    else
		    {
		        ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unhandled event id 0x%x", ETAL_MDR_COMMAND_ID(data));
		    }
			break;

		case ETAL_MDR_SEAMLESS_SWITCHING:
			if (ETAL_extractSeamlessSwitching_MDR(data, len, &seamlessSwitching) == OSAL_OK)
			{
				event = ETAL_INFO_SEAMLESS_SWITCHING_END;
				vl_intEvent = callAtSeamlessSwitchingResponse;
				param = (tU8 *) &seamlessSwitching;
				param_len = sizeof(seamlessSwitching);

				internal_callback_param = param;
				internal_callback_paramLen = param_len;

				// invoke the internal call back
				ETAL_intCbScheduleCallbacks(seamlessSwitching.m_receiverHandle, vl_intEvent, internal_callback_param, internal_callback_paramLen);

				// invoke the external call back
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, event, (tVoid *)param, param_len);
				
				if (true == ETAL_receiverIsSpecialInProgress(seamlessSwitching.m_receiverHandle, cmdSpecialExternalSeamlessSwitchingRequestInProgress))
				{		
				    // Disable external seamless switching event
					ETAL_receiverSetSpecial(seamlessSwitching.m_receiverHandle, cmdSpecialExternalSeamlessSwitchingRequestInProgress, cmdActionStop);
				}
			}
		    else
		    {
		        ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unhandled event id 0x%x", ETAL_MDR_COMMAND_ID(data));
		    }
			break;
#endif // CONFIG_ETAL_HAVE_SEAMLESS

		case ETAL_MDR_TUNE:
			if (ETAL_extractTune_MDR(data, len, &tune) == OSAL_OK)
			{
				event = ETAL_INFO_TUNE;
				vl_intEvent = callAtTuneFrequency;

	 			/* Do not send any event or notification if the stop frequency is set to 0 */
	 			if(tune.m_stopFrequency != 0)
	 			{
	 				param = (void *) &tune;
	 				param_len = sizeof(tune);

					vl_tuneStatusInternal.m_Frequency = tune.m_stopFrequency;
					vl_tuneStatusInternal.m_receiverHandle = tune.m_receiverHandle;
					vl_tuneStatusInternal.m_syncInternal = tune.m_sync;
					vl_tuneStatusInternal.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(vl_tuneStatusInternal.m_receiverHandle, cmdSpecialExternalTuneRequestInProgress);

					internal_callback_param = &vl_tuneStatusInternal;
					internal_callback_paramLen = sizeof(vl_tuneStatusInternal);

					// invoke the internal and external call back
					ETAL_intCbScheduleCallbacks(tune.m_receiverHandle, vl_intEvent, internal_callback_param, internal_callback_paramLen);

                    ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, event, (tVoid *)param, param_len);

					if (true == ETAL_receiverIsSpecialInProgress(tune.m_receiverHandle, cmdSpecialExternalTuneRequestInProgress))
					{
						ETAL_receiverSetSpecial(tune.m_receiverHandle, cmdSpecialExternalTuneRequestInProgress, cmdActionStop);
					}
	 			}
			}
		    else
		    {
		        ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unhandled event id 0x%x", ETAL_MDR_COMMAND_ID(data));
		    }
			break;

		default:
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unhandled event id 0x%x", ETAL_MDR_COMMAND_ID(data));
			break;
	}
}

/***************************
 *
 * ETAL_CommunicationLayer_Receive_MDR
 *
 **************************/
/*
 * Something received from MDR on the CONTROL_LUN.
 * Waits the response and possibly the notification from the MDR
 */
static tVoid ETAL_CommunicationLayer_Receive_MDR(tU8 *data, tU32 len, etalToMDRCmdTy *cmd, etalToMDRCmdStatusTy *status)
{
	tBool consumed = FALSE;
	static etalPADDLSTy padDLS; // static for size, not persistency
	static etalPADDLPLUSTy padDLPLUS; // static for size, not persistency
	static etalPADDLPLUSTy padDLPLUSbis; // static for size, not persistency
	tU8 *tmp_pad = NULL;
	EtalDataServiceType etalType;
	EtalDataBlockStatusTy data_status;

	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	ETAL_HANDLE hDatapathBis = ETAL_INVALID_HANDLE;

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
	ETAL_HANDLE hReceiver = ETAL_INVALID_HANDLE;
	tU8 app;
#endif //#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)


	switch (*status)
	{
		case cmdStatusWaitNotificationMDR:
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP cmdStatusWaitNotification");
			if (ETAL_MDR_IS_NOTIFICATION(data) &&
				(ETAL_MDR_COMMAND_ID(data) == ETAL_toMDROutstandingCmdId))
			{
				if (ETAL_MDR_NOTIFICATION_STATUS(data) != 0)
				{
					cmd->cmdStatus = (tU8)ETAL_MDR_NOTIFICATION_STATUS(data);

					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_COMM, "DCOP Notification status 0x%x (%s)", cmd->cmdStatus, ETAL_MDRStatusToString(cmd->cmdStatus));
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusError");
					*status = cmdStatusErrorMDR;
					ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
					// indicate message has been ended / notification / response
					OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif	
				}
				else
				{
					if (cmd->cmdHasResponse)
					{
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusWaitResponse");
						*status = cmdStatusWaitResponseMDR;
					}
					else
					{
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusComplete");
						*status = cmdStatusCompleteMDR;
						ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;
						if (ETAL_MDR_IS_FAST_NOTIFICATION(data) && ETAL_MDR_FAST_NOTIF_HAS_DATA(data))
						{
							if (len <= ETAL_MAX_RESPONSE_LEN_CONTROL_LUN)
							{
								OSAL_pvMemoryCopy((tVoid *)&(cmd->rsp), (tPCVoid)data, len);
								cmd->rspLen = len;
							}
							else
							{
								cmd->rspLen = 0;
								ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Received DCOP notification larger (%d bytes) than supported (%d bytes)", len, ETAL_MAX_RESPONSE_LEN_CONTROL_LUN);
							}
						}
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
						// indicate message has been ended / notification / response
						OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif	

					}
				}
				consumed = TRUE;
			}
			break;

		case cmdStatusWaitResponseMDR:
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP cmdStatusWaitResponse");
			if (ETAL_MDR_IS_RESPONSE(data) &&
				(ETAL_MDR_COMMAND_ID(data) == ETAL_toMDROutstandingCmdId))
			{
				if (len <= ETAL_MAX_RESPONSE_LEN_CONTROL_LUN)
				{
					OSAL_pvMemoryCopy((tVoid *)&(cmd->rsp), (tPCVoid)data, len);
					cmd->rspLen = len;
				}
				else
				{
					cmd->rspLen = 0;
					ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Received DCOP response larger (%d bytes) than supported (%d bytes)", len, ETAL_MAX_RESPONSE_LEN_CONTROL_LUN);
				}

				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusComplete");
				*status = cmdStatusCompleteMDR;
				ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
				if (ETAL_MDR_RESPONSE_STATUS(data) != 0)
				{
					if ((ETAL_MDR_RESPONSE_STATUS(data) == DABMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE) ||
						(ETAL_MDR_RESPONSE_STATUS(data) == DABMW_CMD_STATUS_ERR_TIMEOUT) ||
						(ETAL_MDR_RESPONSE_STATUS(data) == DABMW_CMD_STATUS_ERR_CMD_IS_NOT_ONGOING))
					{
						/*
						 * Avoid "Response no data available", thay are normally received
						 * for radiotext functions
						 * Avoid "Timeout error", it normally means the DCOP tuned to a frequency
						 * not containing any DAB signal
						 * Avoid "Command is ongoing", it could be that DCOP SEEK stop command be sent
						 * while it has been already stopped (found frequency for instance)
						 */
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP Response status 0x%x (%s)", ETAL_MDR_RESPONSE_STATUS(data), ETAL_MDRStatusToString(ETAL_MDR_RESPONSE_STATUS(data)));
					}
					else 
					{
#ifndef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
						ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "DCOP Response status 0x%x (%s)", ETAL_MDR_RESPONSE_STATUS(data), ETAL_MDRStatusToString(ETAL_MDR_RESPONSE_STATUS(data)));
#endif
					}
				}

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
				// indicate message has been ended / notification / response
				OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif

#endif // CONFIG_TRACE_CLASS_ETAL
				consumed = TRUE;
			}
			break;

		default:
			if (ETAL_MDR_IS_RESPONSE(data))
			{
				break;
			}
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unexpected Notification (waiting command id 0x%x, got 0x%x) from DCOP while in state %d", ETAL_toMDROutstandingCmdId, ETAL_MDR_COMMAND_ID(data), *status);
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusError");
			*status = cmdStatusErrorMDR;
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
			// indicate message has been ended / notification / response
			OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif

			consumed = TRUE;
	}


	if (consumed)
	{
		return;
	}

	/* process uncatched notifications/responses */

	if (ETAL_MDR_IS_RESPONSE(data))
	{
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
		if ((ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_GETMONITORDABINFO) ||
			(ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_GETMONITORAMFMINFO))
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Monitor Information, cmd id 0x%x", ETAL_MDR_COMMAND_ID(data));

			/* app is in the payload BYTE0 */
			app = (tU8)((ETAL_DABMW_GET_DABMON_APP(data + ETAL_MDR_GETMONITORINFO_PAYLOAD_OFFSET) == 0x01) ? DABMW_MAIN_AUDIO_DAB_APP: DABMW_SECONDARY_AUDIO_DAB_APP);
			hReceiver = ETAL_receiverSearchFromApplication(app);

			if (ETAL_INVALID_HANDLE == hReceiver)
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Monitor Information, invalid app / receiver , receiver = %d, app = %d", hReceiver, app);
			}
			else
			{
				if(ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
				{
					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_controlPollQuality : Monitoring on %d suspended", hReceiver);
				}
				else
				{
					ETAL_callbackInvoke(ETAL_COMM_QUALITY_CALLBACK_HANDLER, cbTypeQuality_MDR, ETAL_INFO_UNDEF, (tVoid *)data, len);
				}
			}
		}
		else
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API
		if (ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_GETPAD)
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Get PAD DLS, cmd id 0x%x", ETAL_MDR_COMMAND_ID(data));

			if (ETAL_extractPADDLS_MDR(data, len, &padDLS, &app) == OSAL_OK)
			{
				ETAL_statusSetDABPAD(&padDLS);
				hReceiver = ETAL_receiverSearchFromApplication(app);

				if (ETAL_INVALID_HANDLE == hReceiver)
				{
					// we have a problem,
					// the app is not matching a receiver
					// in some older DCOP FW version, DLS did not broadcast the APP
					// search by trying the Main and secondary
					// and consider it is for the 1st found...
					//
					
					ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "receiver not found from APP on DLS command : receiver = %d, app = %d", hReceiver, app);
					// search for one.
					// simulate app main DAB
					// 
					// 
					hReceiver = ETAL_receiverSearchFromApplication(DABMW_MAIN_AUDIO_DAB_APP);
					if (ETAL_INVALID_HANDLE == hReceiver)
					{
						// we have a DAB receiver for main
						// check if we have a service
						hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLS);
						if (hDatapath == ETAL_INVALID_HANDLE)
						{
							// no data service on this app, check on bg
							hReceiver = ETAL_INVALID_HANDLE;

							hReceiver = ETAL_receiverSearchFromApplication(DABMW_SECONDARY_AUDIO_DAB_APP);
							if (ETAL_INVALID_HANDLE != hReceiver)
							{
								// we have a DAB receiver for main
								// check if we have a service
								hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLS);

								if (hDatapath == ETAL_INVALID_HANDLE)
								{
									// no data service on this app, check on bg
									hReceiver = ETAL_INVALID_HANDLE;
								}
							}
						}							
					}

				}	
					
				if (ETAL_INVALID_HANDLE != hReceiver)
				{
					hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLS);
					if (hDatapath != ETAL_INVALID_HANDLE)
					{
						ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)&padDLS, sizeof(etalPADDLSTy), NULL);
					}

					hDatapathBis = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DATA_SERVICE);
					if (hDatapathBis != ETAL_INVALID_HANDLE)
					{
						OSAL_pvMemorySet((tVoid *)&data_status, 0x00, sizeof(EtalDataBlockStatusTy));
						data_status.m_validData = TRUE;
						etalType = ETAL_DATASERV_TYPE_DLS;

						//At this point, we should add Type bytes to the payload that we send through callback
						tmp_pad = (tU8 *)OSAL_pvMemoryAllocate(sizeof(etalPADDLSTy) + 4);
						if (tmp_pad == NULL)
						{
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Warning Out of Memory when receiving DAB_DLS");
							ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)((tULong)hDatapathBis), sizeof(ETAL_HANDLE));
							return;
						}
						OSAL_pvMemoryCopy((tVoid *)tmp_pad, (tPCVoid)&etalType, 4);
						OSAL_pvMemoryCopy((tVoid *)(tmp_pad+4), (tPCVoid)&padDLS, sizeof(etalPADDLSTy));
						ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapathBis, (tVoid *)tmp_pad, sizeof(etalPADDLSTy) + 4, &data_status);
						OSAL_vMemoryFree((tVoid *)tmp_pad);
					}

					if((hDatapath == ETAL_INVALID_HANDLE) && (hDatapathBis == ETAL_INVALID_HANDLE))
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_COMM, "Datapath not configured to received ETAL_DATA_TYPE_DATA_SERVICE nor ETAL_DATA_TYPE_DAB_DLS");
					}
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Error in PAD DLS extraction");
			}
		}
		else if (ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_GETPADPLUS)
		{
			ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Get PAD DL PLUS, cmd id 0x%x", ETAL_MDR_COMMAND_ID(data));

			if (ETAL_extractPADDLPLUS_MDR(data, len, &padDLPLUS, &app) == OSAL_OK)
			{
				hReceiver = ETAL_receiverSearchFromApplication(app);
				
				if (ETAL_INVALID_HANDLE == hReceiver)
				{
					// we have a problem,
					// the app is not matching a receiver
					// in some older DCOP FW version, DLS did not broadcast the APP
					// search by trying the Main and secondary
					// and consider it is for the 1st found...
					//
						
					ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "receiver not found from APP on DL PLUS command : receiver = %d, app = %d", hReceiver, app);
					// search for one.
					// simulate app main DAB
					// 
					// 
					hReceiver = ETAL_receiverSearchFromApplication(DABMW_MAIN_AUDIO_DAB_APP);
					if (ETAL_INVALID_HANDLE == hReceiver)
					{
						// we have a DAB receiver for main
						// check if we have a service
						hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLPLUS);
						if (hDatapath == ETAL_INVALID_HANDLE)
						{
							// no data service on this app, check on bg
							hReceiver = ETAL_INVALID_HANDLE;

							hReceiver = ETAL_receiverSearchFromApplication(DABMW_SECONDARY_AUDIO_DAB_APP);
							if (ETAL_INVALID_HANDLE != hReceiver)
							{
								// we have a DAB receiver for main
								// check if we have a service
								hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLPLUS);

								if (hDatapath == ETAL_INVALID_HANDLE)
								{
									// no data service on this app, check on bg
									hReceiver = ETAL_INVALID_HANDLE;
								}
							}
						}							
					}
				}

				if (ETAL_INVALID_HANDLE != hReceiver)
				{		
					hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DLPLUS);
					if (hDatapath != ETAL_INVALID_HANDLE)
					{
						ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)&padDLPLUS, sizeof(etalPADDLPLUSTy), NULL);
					}

					padDLPLUSbis = padDLPLUS;

					hDatapathBis = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DATA_SERVICE);
					if (hDatapathBis != ETAL_INVALID_HANDLE)
					{
						OSAL_pvMemorySet((tVoid *)&data_status, 0x00, sizeof(EtalDataBlockStatusTy));
						data_status.m_validData = TRUE;
						etalType = ETAL_DATASERV_TYPE_DLPLUS;

						//At this point, we should add Type bytes to the payload that we send through callback
						tmp_pad = (tU8 *)OSAL_pvMemoryAllocate(sizeof(etalPADDLPLUSTy) + 4);
						if (tmp_pad == NULL)
						{
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Warning Out of Memory when receiving DAB_DLPLUS");
							ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)((tULong)hDatapathBis), sizeof(ETAL_HANDLE));
							return;
						}
						OSAL_pvMemoryCopy((tVoid *)tmp_pad, (tPCVoid)&etalType, 4);
						OSAL_pvMemoryCopy((tVoid *)(tmp_pad+4), (tPCVoid)&padDLPLUSbis, sizeof(etalPADDLPLUSTy));
						ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapathBis, (tVoid *)tmp_pad, sizeof(etalPADDLPLUSTy) + 4, &data_status);
						OSAL_vMemoryFree((tVoid *)tmp_pad);
					}

					if((hDatapath == ETAL_INVALID_HANDLE) && (hDatapathBis == ETAL_INVALID_HANDLE))
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_COMM, "Datapath not configured to received ETAL_DATA_TYPE_DATA_SERVICE nor ETAL_DATA_TYPE_DAB_DLPLUS");
					}
				}
			}
			else
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Error in PAD DL PLUS extraction");
			}
		}
		else if (((ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_LEARN) && ETAL_receiverIsSpecialInProgress(ETAL_INVALID_HANDLE, cmdSpecialLearn)) ||
			((ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_SEEK) && ETAL_receiverIsSpecialInProgress(ETAL_INVALID_HANDLE, cmdSpecialSeek)))
		{
			/*
			 * This code catches the 'delayed' responses, that is responses
			 * not waited for after sending the command. This feature is used
			 * for special commands, where the response normally arrives
			 * much after the notification.
			 */
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Learn or Seek, cmd id 0x%x", ETAL_MDR_COMMAND_ID(data));
			ETAL_invokeEventCallback_MDR(data, len);
			/*
			 * Signal the FSM is stopped to avoid sending 'stop learn' or 'stop seek' command to the MDR
			 * which would result in an error from the MDR (since the FSM in the MDR
			 * is already stopped after the ETAL_MDR_LEARN/ETAL_MDR_SEEK response)
			 * NOTE: stop all FSM is an overkill but currently there is no way
			 * to address device/application; once this will be place stop only the proper receiver
			 */
			if (ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_LEARN)
			{
				ETAL_receiverStopAllSpecial(cmdSpecialLearn);
			}
		}
		else if (((ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_SEAMLESS_ESTIMATION) && ETAL_receiverIsSpecialInProgress(ETAL_INVALID_HANDLE, cmdSpecialSeamlessEstimation)) ||
				((ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_SEAMLESS_SWITCHING) && ETAL_receiverIsSpecialInProgress(ETAL_INVALID_HANDLE, cmdSpecialSeamlessSwitching)))
		{
			/*
			 * This code catches the 'delayed' responses, that is responses
			 * not waited for after sending the command. This feature is used
			 * for special commands, where the response normally arrives
			 * much after the notification.
			 */
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Seamless Estimation or Seamless Switching, cmd id 0x%x", ETAL_MDR_COMMAND_ID(data));
			ETAL_invokeEventCallback_MDR(data, len);
			/*
			 * Signal the FSM is stopped
			 * NOTE: stop all FSM is an overkill but currently there is no proper way
			 * to address device/application; once this will be place stop only the proper receiver
			 */
			if (ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_SEAMLESS_ESTIMATION)
			{
				ETAL_receiverStopAllSpecial(cmdSpecialSeamlessEstimation);
			}
			else if (ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_SEAMLESS_SWITCHING)
			{
				ETAL_receiverStopAllSpecial(cmdSpecialSeamlessSwitching);
			}
		}	
		else if ((ETAL_MDR_COMMAND_ID(data) == ETAL_MDR_TUNE) && ETAL_receiverIsSpecialInProgress(ETAL_INVALID_HANDLE, cmdSpecialTune))
		{
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received DCOP Response: Tune, cmd id 0x%x", ETAL_MDR_COMMAND_ID(data));
			ETAL_invokeEventCallback_MDR(data, len);
			ETAL_receiverStopAllSpecial(cmdSpecialTune);

		}
		else
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unexpected DCOP Response (waiting command id 0x%x, got 0x%x, state 0x%x)", ETAL_toMDROutstandingCmdId, ETAL_MDR_COMMAND_ID(data), *status);
		}
	}
	else // notification
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unexpected notification (waiting command id 0x%x, got 0x%x) from DCOP while in state 0x%x", ETAL_toMDROutstandingCmdId, ETAL_MDR_COMMAND_ID(data), *status);
	}
}

#ifdef CONFIG_COMM_DRIVER_DIRECT
/***************************
 *
 * ETAL_CommunicationLayer_Receive_Direct_MDR
 *
 **************************/
/*
 * Something received from MDR on the CONTROL_LUN.
 * Waits the response and possibly the notification from the MDR
 */
static tVoid ETAL_CommunicationLayer_Receive_Direct_MDR(tU8 *data, tU32 len, etalToMDRCmdTy *cmd, etalToMDRCmdStatusTy *status, tU8 lun)
{
	tBool consumed = FALSE;
	rif_pr_msg_header_t *msg_p;

	if (rif_pr_msg_queue_p != NULL)
	{
		/* create msg for rif protocol router */
		msg_p = (rif_pr_msg_header_t *)OSAL_pvMemoryAllocate(sizeof(rif_pr_msg_header_t) + len);
		if (msg_p == NULL)
		{
			ASSERT_ON_DEBUGGING(0);
		}
		else
		{
			OSAL_pvMemorySet(msg_p, 0, sizeof(rif_pr_msg_header_t));
			msg_p->msg_id = MSG_ID_RIF_PR_TX_RESP_DABMW;
			msg_p->lun = lun;

			/* copy response in msg */
			OSAL_pvMemoryCopy((tPVoid)(msg_p + sizeof(rif_pr_msg_header_t)), data, len);
			msg_p->num_bytes = len;
			msg_p->msg_buf_ptr = (tU8 *)(msg_p + sizeof(rif_pr_msg_header_t));
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "ETAL_Comm msg_p %x lun %x sz %d", msg_p, msg_p->lun, msg_p->num_bytes);
			/* send msg to rif protocol router */
			if (rif_msg_send(rif_pr_msg_queue_p, msg_p) != OSAL_OK)
			{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_ERRORS)
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "MSG_ID_RIF_PR_TX send error");
#endif
			}
		}
	}

	switch (*status)
	{
		case cmdStatusWaitNotificationMDR:
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP direct cmdStatusWaitNotification");

			if (ETAL_MDR_IS_NOTIFICATION(data) &&
				(ETAL_MDR_COMMAND_ID(data) == ETAL_toMDROutstandingCmdId))
			{

				if (ETAL_MDR_NOTIFICATION_STATUS(data) != 0)
				{
					cmd->cmdStatus = (tU8)ETAL_MDR_NOTIFICATION_STATUS(data);

					ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_COMM, "DCOP Notification status 0x%x (%s)", cmd->cmdStatus, ETAL_MDRStatusToString(cmd->cmdStatus));
					ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusError");
					*status = cmdStatusErrorMDR;
					ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;
					
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
					// indicate message has been ended / notification / response
					OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif	
				}
				else
				{
					if (cmd->cmdHasResponse)
					{
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusWaitResponse");
						*status = cmdStatusWaitResponseMDR;
					}
					else
					{
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusComplete");
						*status = cmdStatusCompleteMDR;
						ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;
						if (ETAL_MDR_IS_FAST_NOTIFICATION(data) && ETAL_MDR_FAST_NOTIF_HAS_DATA(data))
						{
							if (len <= ETAL_MAX_RESPONSE_LEN_CONTROL_LUN)
							{
								OSAL_pvMemoryCopy((tVoid *)&(cmd->rsp), (tPCVoid)data, len);
								cmd->rspLen = len;
							}
							else
							{
								cmd->rspLen = 0;
								ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Received DCOP notification larger (%d bytes) than supported (%d bytes)", len, ETAL_MAX_RESPONSE_LEN_CONTROL_LUN);
							}
						}
						
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
						// indicate message has been ended / notification / response
						OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif	

					}
				}
				consumed = TRUE;
			}
			break;

		case cmdStatusWaitResponseMDR:
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP direct cmdStatusWaitResponse");
			if (ETAL_MDR_IS_RESPONSE(data) &&
				(ETAL_MDR_COMMAND_ID(data) == ETAL_toMDROutstandingCmdId))
			{
				if (len <= ETAL_MAX_RESPONSE_LEN_CONTROL_LUN)
				{
					OSAL_pvMemoryCopy((tVoid *)&(cmd->rsp), (tPCVoid)data, len);
					cmd->rspLen = len;
				}
				else
				{
					cmd->rspLen = 0;
					ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Received DCOP response larger (%d bytes) than supported (%d bytes)", len, ETAL_MAX_RESPONSE_LEN_CONTROL_LUN);
				}

				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusComplete");
				*status = cmdStatusCompleteMDR;
				ETAL_toMDROutstandingCmdId = ETAL_MDR_ILLEGAL_COMMAND_ID;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
				if (ETAL_MDR_RESPONSE_STATUS(data) != 0)
				{
					if ((ETAL_MDR_RESPONSE_STATUS(data) == DABMW_CMD_STATUS_RSP_NO_DATA_AVAILABLE) ||
						(ETAL_MDR_RESPONSE_STATUS(data) == DABMW_CMD_STATUS_ERR_TIMEOUT))
					{
						/*
						 * Avoid "Response no data available", thay are normally received
						 * for radiotext functions
						 * Avoid "Timeout error", it normally means the DCOP tuned to a frequency
						 * not containing any DAB signal
						 */
						ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP Response status 0x%x (%s)", ETAL_MDR_RESPONSE_STATUS(data), ETAL_MDRStatusToString(ETAL_MDR_RESPONSE_STATUS(data)));
					}
					else 
					{
#ifndef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
						ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "DCOP Response status 0x%x (%s)", ETAL_MDR_RESPONSE_STATUS(data), ETAL_MDRStatusToString(ETAL_MDR_RESPONSE_STATUS(data)));
#endif
					}
				}
#endif // CONFIG_TRACE_CLASS_ETAL

				
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
				// indicate message has been ended / notification / response
				OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif	

				consumed = TRUE;
			}
			break;

		default:
			if (ETAL_MDR_IS_RESPONSE(data))
			{
				break;
			}
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unexpected Notification (waiting command id 0x%x, got 0x%x) from DCOP while in state %d", ETAL_toMDROutstandingCmdId, ETAL_MDR_COMMAND_ID(data), *status);
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DCOP new status cmdStatusError");
			*status = cmdStatusErrorMDR;
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
			// indicate message has been ended / notification / response
			OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif
			consumed = TRUE;
	}

	if (consumed)
	{
		return;
	}
}
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

/***************************
 *
 * ETAL_extractRAWMOTObjectFromDataChannel
 *
 **************************/
/*
 * <data> is a 'RAW MOT object payload'
 * Extracts the payload (i.e. MOTBody), placed at a variable offset from the
 * start, depending on the header fields included and the payload length
 *
 * RAW MOT object payload is used for DECODED DATA containing
 * - EPG RAW       (type 0x01)
 * - SLS           (type 0x02)
 * - SLS OVER XPAD (type 0x03)
 */
static tSInt ETAL_extractRAWMOTObjectFromDataChannel(tU8 *data, tU32 size, tU8 **payload_data, tU32* payload_size)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	static tBool warning_issued = FALSE;
#endif
	tU16 bodyinfo_len;
	tS32 totallen;

	bodyinfo_len = ETAL_utilityGetU16(data, 0);
	totallen = (tS32)size - bodyinfo_len;

	if (totallen <= 0)
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
		if (!warning_issued)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Incoherency in DataGroup size for RAW object");
			warning_issued = TRUE;
		}
#endif
		*payload_data = NULL;
		*payload_size = 0;
		return OSAL_ERROR;
	}

	*payload_data = data + bodyinfo_len;
	*payload_size = (tU32)totallen;
	return OSAL_OK;
}

/***************************
 *
 * ETAL_extractDataGroupObjectFromDataChannel
 *
 **************************/
/*
 * <data> is a 'RAW DataGroup object payload'
 * Extracts the payload (i.e. MOTBody), placed at a variable offset from the
 * start, depending on the header fields included and the payload length
 *
 * RAW DataGroup object payload is used for DECODED DATA containing
 * - TPEG RAW      (type 0x05)
 */
static tSInt ETAL_extractDataGroupObjectFromDataChannel(tU8 *data, tU32 size, tU8 **payload_data, tU32* payload_size)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	static tBool warning_issued = FALSE;
#endif
	tU16 datagroup_headerlen;
	tU16 datagroup_datalen;
	tU8  datagroup_crcflag;
	tU32 totallen;

	datagroup_headerlen = ETAL_utilityGetU16(data, 0);
	datagroup_datalen = ETAL_utilityGetU16(data, 2);
	datagroup_crcflag = ETAL_utilityGetU8(data, 4);
	totallen = datagroup_headerlen + datagroup_datalen + DABMW_DATACHANNEL_DATAGROUP_HEADER_SIZE;
	totallen += (datagroup_crcflag == (tU8)1) ? 2 : 0;

	if (totallen != size)
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
		if (!warning_issued)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Incoherency in DataGroup size, totallen=%d, size=%d", totallen, size);
			warning_issued = TRUE;
		}
#endif
		*payload_data = NULL;
		*payload_size = 0;
		return OSAL_ERROR;
	}

	*payload_data = data + DABMW_DATACHANNEL_DATAGROUP_HEADER_SIZE + datagroup_headerlen;
	*payload_size = datagroup_datalen;
	return OSAL_OK;
}

/***************************
 *
 * ETAL_CommunicationLayer_ReceiveData_MDR
 *
 **************************/
/*
 * Something received from MDR on the DATA_LUN.
 * Check active data paths and notify the appropriate one based on the
 * Data Protocol Type byte
 */
static tVoid ETAL_CommunicationLayer_ReceiveData_MDR(tU8 *data, tU32 len)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	static tBool warning1_issued = FALSE;
	static tBool warning2_issued = FALSE;
	static tBool warning3_issued = FALSE;
	static tBool warning4_issued = FALSE;
#endif
	tU32 size;
	DABMWDataServiceType type;
	EtalDataServiceType etalType = ETAL_DATASERV_TYPE_NONE;
	tU8 app;
	tU8 *payload, *handler_payload, *tmp_payload;
	tVoid *context;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	ETAL_HANDLE hReceiver;
	tU8 curr_continuity;
	EtalDataBlockStatusTy data_status;

	OSAL_pvMemorySet((tVoid *)&data_status, 0x00, sizeof(EtalDataBlockStatusTy));

	/*
	 * Preliminary sanity checks
	 */
	if ((data[0] != DABMW_DATACHANNEL_HEADER0) ||
		(DABMW_DATACHANNEL_GET_VERSION(data) != 1))
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
		if (!warning1_issued)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unrecognized header format on DATA LUN");
			warning1_issued = TRUE;
		}
#endif
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, EtalCommStatus_ProtocolHeaderError, 0, data, len);		
		return;
	}

	/*
	 * Data Channel continuity check
	 */

	if (data[1] == DABMW_DATACHANNEL_HEADER1_ASCII)
	{
		curr_continuity = DABMW_ASCIICHANNEL_GET_CONTINUITY(data);
	}
	else
	{
		curr_continuity = DABMW_DATACHANNEL_GET_CONTINUITY(data);
	}
	//ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Continuity %d", curr_continuity);

	if (ETAL_continuityIndexExpected == DABMW_DATACHANNEL_CONTINUITY_ILLEGAL)
	{
		ETAL_continuityIndexExpected = curr_continuity;
	}
	else if (curr_continuity != ETAL_continuityIndexExpected)
	{
		data_status.m_continuityError = TRUE;
#ifdef INCLUDE_CONTINUITY_COUNTER_DEBUG
		data_status.m_expectedContinuity = ETAL_continuityIndexExpected;
		data_status.m_receivedContinuity = curr_continuity;
#endif
		/*
		 * restart the continuity counter with the last packet received
		 */
		ETAL_continuityIndexExpected = curr_continuity;
	}
	ETAL_continuityIndexExpected = (ETAL_continuityIndexExpected + 1) % DABMW_DATACHANNEL_CONTINUITY_MAX;

	if (data[1] == DABMW_DATACHANNEL_HEADER1_ASCII)
	{
		(LINT_IGNORE_RET) ETAL_tracePrintAsciiLUN(data);
		return;
	}

	/*
	 * There's no checksum on the Data Protocol, so m_validData only
	 * says that the Data Protocol header is formatted as expected
	 */
	data_status.m_validData = TRUE;

	/*
	 * app is in the HEADER BYTE1 and thus is present for all data protocol formats
	 */
	app = (tU8)((DABMW_DATACHANNEL_GET_APP(data) == 0) ? DABMW_MAIN_AUDIO_DAB_APP: DABMW_SECONDARY_AUDIO_DAB_APP);
	payload = DABMW_DATACHANNEL_GET_PAYLOAD(data);
	hReceiver = ETAL_receiverSearchFromApplication(app);
	handler_payload = payload;

	/*
	 * The format of the payload depends on the TYPE header field.
	 */
	switch (DABMW_DATACHANNEL_GET_MSGTYPE(data))
	{
		case DABMW_DATACHANNEL_MSGTYPE_DECODED:
			type = DABMW_DATACHANNEL_GET_TYPE(data);
			hDatapath = ETAL_receiverGetDatapathFromDABType(hReceiver, type, &context);
			/*
			 * extract the payload; the format depends on the type byte
			 * overwrite payload and size with the ones extracted from the payload
			 */
			switch (type)
			{
				case DABMW_DATACHANNEL_DECODED_TYPE_EPG_RAW:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_EPG_RAW;
				case DABMW_DATACHANNEL_DECODED_TYPE_SLS:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_SLS;
				case DABMW_DATACHANNEL_DECODED_TYPE_SLS_XPAD:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_SLS_XPAD;
					if (ETAL_extractRAWMOTObjectFromDataChannel(payload, DABMW_DATACHANNEL_GET_SIZE(data), &handler_payload, &size) != OSAL_OK)
					{
						return;
					}
					break;

				case DABMW_DATACHANNEL_DECODED_TYPE_TPEG_RAW:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_TPEG_RAW;
					if (ETAL_extractDataGroupObjectFromDataChannel(payload, DABMW_DATACHANNEL_GET_SIZE(data), &handler_payload, &size) != OSAL_OK)
					{
						return;
					}
					break;

				case DABMW_DATACHANNEL_DECODED_TYPE_TPEG_SNI:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_TPEG_SNI;
                case DABMW_DATACHANNEL_DECODED_TYPE_SLI:
                    if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_SLI;
				case DABMW_DATACHANNEL_DECODED_TYPE_EPG_BIN:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_EPG_BIN;
				case DABMW_DATACHANNEL_DECODED_TYPE_EPG_SRV:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_EPG_SRV;
				case DABMW_DATACHANNEL_DECODED_TYPE_EPG_PRG:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_EPG_PRG;
				case DABMW_DATACHANNEL_DECODED_TYPE_EPG_LOGO:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_EPG_LOGO;
				case DABMW_DATACHANNEL_DECODED_TYPE_JML_OBJ:
					if (ETAL_DATASERV_TYPE_NONE == etalType) etalType = ETAL_DATASERV_TYPE_JML_OBJ;
					/* In that case, payload is the object itself
					* it can be a SNI table or a JML Object
					* there is nothing to extract ; pass directly the payload
					*/
					handler_payload = payload;
					size = DABMW_DATACHANNEL_GET_SIZE(data);
					ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "DATASERV type (0x%x)", etalType);
					break;

				default:
					// TODO implement other decoders
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
					if (!warning2_issued)
					{
						ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unsupported DECODED DATA TYPE (0x%x)", type);
						warning2_issued = TRUE;
					}
#endif
					return;
			}
//At this point, we should add Type bytes to the payload that we send through callback
			tmp_payload = (tU8 *)OSAL_pvMemoryAllocate(size+4);
			if (tmp_payload == NULL)
			{
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_WARNING_OUT_OF_MEMORY, (tVoid *)((tULong)hDatapath), sizeof(ETAL_HANDLE));
				return;
			}
			OSAL_pvMemoryCopy((tVoid *)tmp_payload, (tPCVoid)&etalType, 4);
			OSAL_pvMemoryCopy((tVoid *)(tmp_payload+4), (tPCVoid)handler_payload, size);
			handler_payload = tmp_payload;
			size = size+4;
			break;

		case DABMW_DATACHANNEL_MSGTYPE_FIC:
			/* In FIC case, whole payload is transmitted
			 * so size is the total payload minus the Data Channel Mandatory Header (8 bytes)
			 */
			size = len-8;
			hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_FIC);
			break;

		case DABMW_DATACHANNEL_MSGTYPE_RAW:
			size = DABMW_RAWDATACHANNEL_GET_SIZE(payload); // TODO: WARNING assumes only one Network Packet per buffer, not true if sending the whole subch
			hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_DATA_RAW);
			break;

		case DABMW_DATACHANNEL_MSGTYPE_AUDIO:
			size = DABMW_DATACHANNEL_GET_SIZE(data);
			hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_AUDIO_RAW);
			break;

		default:
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
			if (!warning4_issued)
			{
				ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unrecognized binary payload format");
				warning4_issued = TRUE;
			}
#endif
			break;
	}

	if (hDatapath != ETAL_INVALID_HANDLE)
	{
		ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)handler_payload, size, &data_status);
	}
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	else if (!warning3_issued)
	{
		ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Undefined callback for DATA LUN, data type = %d", DABMW_DATACHANNEL_GET_MSGTYPE(data));
		if (DABMW_DATACHANNEL_MSGTYPE_DECODED == DABMW_DATACHANNEL_GET_MSGTYPE(data))
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Undefined callback for DATA LUN, data type = %d, type %d", DABMW_DATACHANNEL_GET_MSGTYPE(data), type);
		}
		else
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Undefined callback for DATA LUN, data type = %d", DABMW_DATACHANNEL_GET_MSGTYPE(data));
		}
		warning3_issued = TRUE;
		}
#endif
	if (DABMW_DATACHANNEL_MSGTYPE_DECODED == DABMW_DATACHANNEL_GET_MSGTYPE(data))
	{
		OSAL_vMemoryFree((tVoid *)tmp_payload);
	}
}

/***************************
 *
 * ETAL_DABStatusAutonotification2String_MDR
 *
 **************************/
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
static tChar *ETAL_DABStatusAutonotification2String_MDR(tU8 reason)
{
	switch (reason)
	{
		case 0:
			return "get DAB status";
		case 1:
			return "tune";
		case 2:
			return "search for ensemble";
		case 4:
			return "sync";
		case 5:
			return "reconfiguration";
		case 6:
			return "ber fic";
		case 7:
			return "mute";
		default :
			return "Undefined";
	}
}

static tChar *ETAL_DABAnnouncementAutonotification2String_MDR(tU16 type)
{
	switch (type)
	{
		case 0x0001:
			return "Alarm";
		case 0x0002:
			return "Road traffic flash";
		case 0x0004:
			return "Transport flash";
		case 0x0008:
			return "Warning/Service";
		case 0x0010:
			return "New flash";
		case 0x0020:
			return "Area weather flash";
		case 0x0040:
			return "Event announcement";
		case 0x0080:
			return "Special event";
		case 0x0100:
			return "Programme information";
		case 0x0200:
			return "Sport report";
		case 0x0400:
			return "Financial report";
		default :
			return "Undefined";
	}
}
#endif

/***************************
 *
 * ETAL_CommunicationLayer_DABStatusAutonotification_MDR
 *
 **************************/
/*
 * DAB auto notification is used
 * DAB status message. */
/*
 * DAB Status auto notification payload format (offset from Header0 byte)
 * byte 4     = application
 * byte 5     = notify reason
 * byte 6     = search
 * byte 7     = transmission mode
 * byte 8     = BER FIC
 * byte 9     = mute status
 * byte 10-13 = tuned frequency
 * byte 14    = reconfiguration
 * byte 15    = SYNC bits
 */
static tVoid ETAL_CommunicationLayer_DABStatusAutonotification_MDR(ETAL_HANDLE hReceiver, tU8 *data, tU32 len)
{
	etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
	etalAutoNotificationStatusTy *autoNotifp = &(recvp->DABStatusNotif);
	EtalTuneStatus vl_tune;
	etalAutoNotificationStatusTy previousNotif;

	if((recvp != NULL) && (autoNotifp != NULL))
	{
		/* Save temporary previous notification */
		previousNotif = recvp->DABStatusNotif;

		autoNotifp->receiverHandle = hReceiver;
		autoNotifp->type = autoDABStatus;
		autoNotifp->status.DABStatus.reason           = ETAL_utilityGetU8(data, 5); // the first 4 bytes are the common auto notification header
		autoNotifp->status.DABStatus.search           = ETAL_utilityGetU8(data, 6);
		autoNotifp->status.DABStatus.transmissionMode = ETAL_utilityGetU8(data, 7);
		autoNotifp->status.DABStatus.ber              = ETAL_utilityGetU8(data, 8);
		autoNotifp->status.DABStatus.mute             = ETAL_utilityGetU8(data, 9);
		autoNotifp->status.DABStatus.tunedFrequency   = ETAL_utilityGetU32(data, 10);
		autoNotifp->status.DABStatus.reconfiguration  = ETAL_utilityGetU8(data, 14);
		autoNotifp->status.DABStatus.sync             = ETAL_utilityGetU8(data, 15);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB STATUS auto-notification(previous), reason=%s (0x%x), search=%d, trans=%d, sync=0x%x, mute=%d, ber=%d, tunFre=%d, reconf=%d",
				ETAL_DABStatusAutonotification2String_MDR(previousNotif.status.DABStatus.reason),
				previousNotif.status.DABStatus.reason,
				previousNotif.status.DABStatus.search,
				previousNotif.status.DABStatus.transmissionMode,
				previousNotif.status.DABStatus.sync,
				previousNotif.status.DABStatus.mute,
				previousNotif.status.DABStatus.ber,
				previousNotif.status.DABStatus.tunedFrequency,
				previousNotif.status.DABStatus.reconfiguration);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB STATUS auto-notification(new), reason=%s (0x%x), search=%d, trans=%d, sync=0x%x, mute=%d, ber=%d, tunFre=%d, reconf=%d",
				ETAL_DABStatusAutonotification2String_MDR(autoNotifp->status.DABStatus.reason),
				autoNotifp->status.DABStatus.reason,
				autoNotifp->status.DABStatus.search,
				autoNotifp->status.DABStatus.transmissionMode,
				autoNotifp->status.DABStatus.sync,
				autoNotifp->status.DABStatus.mute,
				autoNotifp->status.DABStatus.ber,
				autoNotifp->status.DABStatus.tunedFrequency,
				autoNotifp->status.DABStatus.reconfiguration);

		ETAL_intCbScheduleCallbacks(hReceiver, callAtDABAutonotification, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_DAB_AUTONOTIFICATION, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));

		if((previousNotif.status.DABStatus.sync != autoNotifp->status.DABStatus.sync) ||
		   (previousNotif.status.DABStatus.mute != autoNotifp->status.DABStatus.mute))
		{
			(void)OSAL_pvMemorySet((tVoid *)&vl_tune, 0x00, sizeof(EtalTuneStatus));

			vl_tune.m_receiverHandle = hReceiver;
			vl_tune.m_serviceId      = ETAL_INVALID_PROG;
			vl_tune.m_stopFrequency  = ETAL_utilityGetU32(data, 10);
			vl_tune.m_sync           = autoNotifp->status.DABStatus.sync;
			vl_tune.m_muteStatus     = autoNotifp->status.DABStatus.mute;

			if(vl_tune.m_stopFrequency != 0)
			{
				ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_TUNE, (tVoid *)&vl_tune, sizeof(EtalTuneStatus));
			}
		}
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_DABStatusAutonotification_MDR (recvp: 0x%x)", recvp);
	}
}

/***************************
 *
 * ETAL_CommunicationLayer_DABAnnouncementAutonotification_MDR
 *
 **************************/
/*
 * DAB auto notification is used to report
 * DAB announcement switching message.
 */
/*
 * DAB Announcement auto notification payload format (offset from Header0 byte)
 * byte 4     = application
 * byte 5-6   = announcement switching
 * byte 7-10  = SID
 * byte 11-14 = mute status
 * byte 15    = sub channel Id
 * byte 16    = new flag
 * byte 17    = region Id
 */
static tVoid ETAL_CommunicationLayer_DABAnnouncementAutonotification_MDR(ETAL_HANDLE hReceiver, tU8 *data, tU32 len)
{
	etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
	etalAutoNotificationStatusTy *autoNotifp = &(recvp->DABAnnouncementNotif);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	etalAutoNotificationStatusTy previousNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

	if((recvp != NULL) && (autoNotifp != NULL))
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
		/* Save temporary previous notification */
		previousNotif = recvp->DABAnnouncementNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

		autoNotifp->receiverHandle = hReceiver;
		autoNotifp->type = autoDABAnnouncementSwitching;
		autoNotifp->status.DABAnnouncement.announcementType = ETAL_utilityGetU16(data, 5); // the first 4 bytes are the common auto notification header
		autoNotifp->status.DABAnnouncement.sid = ETAL_utilityGetU32(data, 7);
		autoNotifp->status.DABAnnouncement.subChId = ETAL_utilityGetU8(data, 15);
		autoNotifp->status.DABAnnouncement.newFlag = ETAL_utilityGetU8(data, 16);
		autoNotifp->status.DABAnnouncement.regionId = ETAL_utilityGetU8(data, 17);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Announcement Switching auto-notification(previous), type=%s (0x%x), sid=0x%x, subChId=0x%x, reg=0x%x, newFlag=%d",
				ETAL_DABAnnouncementAutonotification2String_MDR(previousNotif.status.DABAnnouncement.announcementType),
				previousNotif.status.DABAnnouncement.announcementType,
				previousNotif.status.DABAnnouncement.sid,
				previousNotif.status.DABAnnouncement.subChId,
				previousNotif.status.DABAnnouncement.regionId,
				previousNotif.status.DABAnnouncement.newFlag);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Announcement Switching auto-notification(new), type=%s (0x%x), sid=0x%x, subChId=0x%x, reg=0x%x, newFlag=%d",
				ETAL_DABAnnouncementAutonotification2String_MDR(autoNotifp->status.DABAnnouncement.announcementType),
				autoNotifp->status.DABAnnouncement.announcementType,
				autoNotifp->status.DABAnnouncement.sid,
				autoNotifp->status.DABAnnouncement.subChId,
				autoNotifp->status.DABAnnouncement.regionId,
				autoNotifp->status.DABAnnouncement.newFlag);

		ETAL_intCbScheduleCallbacks(hReceiver, callAtDABAnnouncement, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_DAB_AUTONOTIFICATION, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_DABAnnouncementSwitchingAutonotification_MDR (recvp: 0x%x)", recvp);
	}
}


/***************************
 *
 * ETAL_CommunicationLayer_DABAnnouncementRawAutonotification_MDR
 *
 **************************/
/*
 * DAB auto notification is used to report
 * DAB announcement switching raw message.
 */
/*
 * DAB Announcement raw auto notification payload format (offset from Header0 byte)
 * byte 4     = application + alarm flag (MSB)
 * byte 5     = cluster Id
 * byte 6-7   = announcement switching
 * byte 8     = sub channel Id
 * byte 9     = region Id
 * repeated X times (X max = 16)
 *
 *
 */
static tVoid ETAL_CommunicationLayer_DABAnnouncementRawAutonotification_MDR(ETAL_HANDLE hReceiver, tU8 *data, tU32 len)
{
	etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
	etalAutoNotificationStatusTy *autoNotifp = &(recvp->DABAnnouncementRawNotif);
	tU8 i, *payload;
	tS32 payload_len;

	payload_len = (tS32)ETAL_getGenericPayloadLen_MDR(data);
    payload = (tU8 *)ETAL_getGenericPayload_MDR(data);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	etalAutoNotificationStatusTy previousNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

	if((recvp != NULL) && (autoNotifp != NULL))
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
		/* Save temporary previous notification */
		previousNotif = recvp->DABAnnouncementRawNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

        OSAL_pvMemorySet((tVoid *)&(autoNotifp->status.DABAnnouncementRaw), 0x00, sizeof(etalDABAnnouncementRawAutonotifTy));
		autoNotifp->receiverHandle = hReceiver;
		autoNotifp->type = autoDABAnnouncementSwitchingRaw;
        autoNotifp->status.DABAnnouncementRaw.m_nbOfItems = (tU8)(payload_len / 6);

        if(autoNotifp->status.DABAnnouncementRaw.m_nbOfItems <= ETAL_DEF_MAX_ANNOUNCEMENT_SWITCHING_RAW_ITEM)
        {
            for(i = 0; i < autoNotifp->status.DABAnnouncementRaw.m_nbOfItems; i++)
            {
                autoNotifp->status.DABAnnouncementRaw.m_item[i].alarmFlag = (ETAL_utilityGetU8(payload, 0) & 0x80) ? TRUE : FALSE;
                autoNotifp->status.DABAnnouncementRaw.m_item[i].clusterId = ETAL_utilityGetU8(payload, 1);
                autoNotifp->status.DABAnnouncementRaw.m_item[i].announcementType = ETAL_utilityGetU16(payload, 2);
                autoNotifp->status.DABAnnouncementRaw.m_item[i].subChId = ETAL_utilityGetU8(payload, 4) & 0x3F;
                autoNotifp->status.DABAnnouncementRaw.m_item[i].newFlag = (ETAL_utilityGetU8(payload, 4) & 0x80) ? TRUE : FALSE;
                if(ETAL_utilityGetU8(payload, 4) & 0x40)
                {
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].regionId = ETAL_utilityGetU8(payload, 5);
                }
                else
                {
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].regionId = 0xFF;
                }
                payload += 6;
            }
        }
        else
        {
            ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_DABAnnouncementSwitchingRawAutonotification_MDR (recHan: %d, nbOfItem: %d)",
                    *recvp, autoNotifp->status.DABAnnouncementRaw.m_nbOfItems);
            autoNotifp->status.DABAnnouncementRaw.m_nbOfItems = 0;
        }


#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
        ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Announcement Switching RAW auto-notification(previous), recHan=%d, nbOfItem=%d",
                *recvp, previousNotif.status.DABAnnouncementRaw.m_nbOfItems);

        for(i = 0; i < previousNotif.status.DABAnnouncementRaw.m_nbOfItems; i++)
        {
            ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Item[%d]: typ=%s (0x%x), cluId=0x%x, subChId=0x%x, regId=0x%x, alaFla=%d, newFla=%d",
                    i, ETAL_DABAnnouncementAutonotification2String_MDR(previousNotif.status.DABAnnouncementRaw.m_item[i].announcementType),
                    previousNotif.status.DABAnnouncementRaw.m_item[i].announcementType,
                    previousNotif.status.DABAnnouncementRaw.m_item[i].clusterId,
                    previousNotif.status.DABAnnouncementRaw.m_item[i].subChId,
                    previousNotif.status.DABAnnouncementRaw.m_item[i].regionId,
                    previousNotif.status.DABAnnouncementRaw.m_item[i].alarmFlag,
                    previousNotif.status.DABAnnouncementRaw.m_item[i].newFlag);
        }

        ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Announcement Switching RAW auto-notification(New), recHan=%d, nbOfItem=%d",
                *recvp, autoNotifp->status.DABAnnouncementRaw.m_nbOfItems);

        for(i = 0; i < autoNotifp->status.DABAnnouncementRaw.m_nbOfItems; i++)
        {
            ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Item[%d]: type=%s (0x%x), cluId=0x%x, subChId=0x%x, regId=0x%x, alaFla=%d, newFla=%d",
                    i, ETAL_DABAnnouncementAutonotification2String_MDR(autoNotifp->status.DABAnnouncementRaw.m_item[i].announcementType),
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].announcementType,
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].clusterId,
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].subChId,
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].regionId,
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].alarmFlag,
                    autoNotifp->status.DABAnnouncementRaw.m_item[i].newFlag);
        }
#endif

		ETAL_intCbScheduleCallbacks(hReceiver, callAtDABAnnouncement, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_DAB_AUTONOTIFICATION, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_DABAnnouncementSwitchingRawAutonotification_MDR (recvp: 0x%x)", recvp);
	}
}

/***************************
 *
 * ETAL_CommunicationLayer_DABReconfigurationAutonotification_MDR
 *
 **************************/
/*
 * DAB auto notification is used to report
 * DAB reconfiguration message.
 */
/*
 * DAB reconfiguration auto notification payload format (offset from Header0 byte)
 * byte 4     = application
 * byte 5     = reconfiguration type
 * byte 6-9   = occurrence time
 * byte 10-13 = cif counter,
 */
static tVoid ETAL_CommunicationLayer_DABReconfigurationAutonotification_MDR(ETAL_HANDLE hReceiver, tU8 *data, tU32 len)
{
	etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
	etalAutoNotificationStatusTy *autoNotifp = &(recvp->DABReconfigurationNotif);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	etalAutoNotificationStatusTy previousNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

	if((recvp != NULL) && (autoNotifp != NULL))
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
		/* Save temporary previous notification */
		previousNotif = recvp->DABReconfigurationNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

		autoNotifp->receiverHandle = hReceiver;
		autoNotifp->type = autoDABReconfiguration;
		autoNotifp->status.DABReconfiguration.reconfigurationType = ETAL_utilityGetU8(data, 5);
		autoNotifp->status.DABReconfiguration.occurrenceTime      = ETAL_utilityGetU32(data, 6);
		autoNotifp->status.DABReconfiguration.cifCounter          = ETAL_utilityGetU32(data, 10);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Reconfiguration auto-notification(previous), recTyp=0x%x, occTim=%d, cifCou=%d",
				previousNotif.status.DABReconfiguration.reconfigurationType,
				previousNotif.status.DABReconfiguration.occurrenceTime,
				previousNotif.status.DABReconfiguration.cifCounter);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Reconfiguration auto-notification(new), recTyp=0x%x, occTim=%d, cifCou=%d",
				autoNotifp->status.DABReconfiguration.reconfigurationType,
				autoNotifp->status.DABReconfiguration.occurrenceTime,
				autoNotifp->status.DABReconfiguration.cifCounter);

		ETAL_intCbScheduleCallbacks(hReceiver, callAtDABAnnouncement, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_DAB_AUTONOTIFICATION, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));

	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_DABReconfigurationAutonotification_MDR (recvp: 0x%x, autoNotifp:0x%x)", recvp, autoNotifp);
	}
}

/***************************
 *
 * ETAL_CommunicationLayer_DABDataStatusAutonotification_MDR
 *
 **************************/
/*
 * DAB auto notification is used to report
 * DAB data status message.
 */
/*
 * DAB data status auto notification payload format (offset from Header0 byte)
 * byte 4     = application
 * byte 5     = reconfiguration type
 * byte 6-9   = occurrence time
 * byte 10-13 = cif counter,
 */
static tVoid ETAL_CommunicationLayer_DABDataStatusAutonotification_MDR(ETAL_HANDLE hReceiver, tU8 *data, tU32 len)
{
	etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);
	etalAutoNotificationStatusTy *autoNotifp = &(recvp->DABDataStatusNotif);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
	etalAutoNotificationStatusTy previousNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

	if((recvp != NULL) && (autoNotifp != NULL))
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
		/* Save temporary previous notification */
		previousNotif = recvp->DABDataStatusNotif;
#endif //#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)

		autoNotifp->receiverHandle = hReceiver;
		autoNotifp->type = autoDABDataStatus;
		autoNotifp->status.DABDataStatus.epgComplete = ETAL_utilityGetU8(data, 5) & 0x01;

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Data Status auto-notification(previous), epgCom=%d",
				previousNotif.status.DABDataStatus.epgComplete);

		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "DAB Data Status auto-notification(new), epgCom=%d",
				autoNotifp->status.DABDataStatus.epgComplete);

		ETAL_intCbScheduleCallbacks(hReceiver, callAtDABAnnouncement, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_DAB_AUTONOTIFICATION, (tVoid *)autoNotifp, sizeof(etalAutoNotificationStatusTy));

	}
	else
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_DABDataStatusAutonotification_MDR (recvp: 0x%x)", recvp);
	}
}

/***************************
 *
 * ETAL_CommunicationLayer_ReceiveAutonotification_MDR
 *
 **************************/
/*
 * Something received from MDR on the AUTONOTIF_LUN.
 * Only Learn and Seek state machine running on the
 * MDR are allowed to send something on this LUN
 *
 * The MDR autonotification format is:
 * byte 0   = HEADER0
 * byte 1   = rfu
 * byte 2   = autonotification type (0xFE for TUNE_REQ)
 * byte 3   = payload size
 * byte 4   = start of the payload
 */
static tVoid ETAL_CommunicationLayer_ReceiveAutonotification_MDR(tU8 *data, tU32 len)
{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
	static tBool warning1_issued = FALSE;
	static tBool warning2_issued = FALSE;
	static tBool warning3_issued = FALSE;
#endif
	ETAL_HANDLE hReceiver;

	/*
	 * Preliminary sanity checks
	 */
	if (((tU32)data[0] & DABMW_AUTONOTIFICATION_HEADER0_MASK) != DABMW_AUTONOTIFICATION_HEADER0)
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
		if (!warning1_issued)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unrecognized header format on Autonotification LUN");
			warning1_issued = TRUE;
		}
#endif
		ETAL_sendCommunicationErrorEvent(ETAL_INVALID_HANDLE, EtalCommStatus_ProtocolHeaderError, 0, data, len);		
		return;
	}

	/*
	 * Extract the receiver
	 *
	 * Use the application id contained in the auto notification to identify the receiver;
	 * we are assuming implicitly that only one DCOP is connected to the system.
	 * TODO add device dependency based on the physical bus
	 */
	hReceiver = ETAL_receiverSearchFromApplication(data[4]);
	if (hReceiver == ETAL_INVALID_HANDLE)
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
		if (!warning2_issued)
		{
			ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Illegal Receiver on Autonotification LUN");
			warning2_issued = TRUE;
		}
#endif
		return;
	}

	switch (data[DABMW_AUTONOTIFICATION_TYPE_OFFSET])
	{
		case DABMW_AUTONOTIFICATION_DAB_STATUS:
			if (DABMW_AUTONOTIFICATION_GET_SIZE(data) == (tU8)DABMW_AUTONOTIFICATION_DAB_STATUS_SIZE)
			{
				ETAL_CommunicationLayer_DABStatusAutonotification_MDR(hReceiver, data, len);
			}
			break;

		case DABMW_AUTONOTIFICATION_DAB_DATA_STATUS:
			if (DABMW_AUTONOTIFICATION_GET_SIZE(data) == (tU8)DABMW_AUTONOTIFICATION_DAB_DATA_STATUS_SIZE)
			{
				ETAL_CommunicationLayer_DABDataStatusAutonotification_MDR(hReceiver, data, len);
			}
			break;

		case DABMW_AUTONOTIFICATION_RECONFIGURATION:
			if (DABMW_AUTONOTIFICATION_GET_SIZE(data) == (tU8)DABMW_AUTONOTIFICATION_RECONFIGURATION_SIZE)
			{
				ETAL_CommunicationLayer_DABReconfigurationAutonotification_MDR(hReceiver, data, len);
			}
			break;

		case DABMW_AUTONOTIFICATION_ANNOUNCEMENT:
			if (DABMW_AUTONOTIFICATION_GET_SIZE(data) == (tU8)DABMW_AUTONOTIFICATION_ANNOUNCEMENT_SIZE)
			{
				ETAL_CommunicationLayer_DABAnnouncementAutonotification_MDR(hReceiver, data, len);
			}
			break;

		case DABMW_AUTONOTIFICATION_ANNOUNCEMENT_RAW:
		    ETAL_CommunicationLayer_DABAnnouncementRawAutonotification_MDR(hReceiver, data, len);
			break;

		default:
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_SYSTEM_MIN)
			if (!warning3_issued)
			{
				ETAL_tracePrintSysmin(TR_CLASS_APP_ETAL_COMM, "Unhandled autonotification %d", data[DABMW_AUTONOTIFICATION_TYPE_OFFSET]);
				warning3_issued = TRUE;
			}
#endif
			break;
	}
}

/***************************
 *
 * ETAL_CommunicationLayer_Receive_MDR
 *
 **************************/
/*
 * Something received from MDR on the ETAL_BROADCAST_LUN.
 * Waits the response from the MDR
 */
static tVoid ETAL_CommunicationLayer_ReceiveBroadcast_MDR(tU8 *data, tU32 len, etalToMDRCmdTy *cmd, etalToMDRCmdStatusTy *status)
{
    if (*status == cmdStatusWaitNotificationMDR)
    {
        *status = cmdStatusWaitResponseMDR;
    }
    if (*status == cmdStatusWaitResponseMDR)
    {
        /* only 8 bits used for cmdId in case of ETAL_BROADCAST_LUN */
        if ((len > 8) && ((tU16)(data[8]) == ETAL_toMDROutstandingCmdId))
        {
            /* copy response message */
            OSAL_pvMemoryCopy((tVoid *)&(cmd->rsp), (tPCVoid)data, len);
            cmd->rspLen = len;
            *status = cmdStatusCompleteMDR;
			// This is done : post a message to indicate completed
        }
        else
        {
            *status = cmdStatusErrorMDR;
        }

	
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		// indicate message has been ended / notification / response
		OSAL_s32SemaphorePost(ETAL_toMDRCmdMsgComplete);
#endif	


    }
}

/***************************
 *
 * ETAL_CommunicationLayer_ThreadEntry_MDR
 *
 **************************/
/*
 * ETAL communication thread entry point
 * Loops endlessly waiting for data arriving from the
 * IPForward task, then pre-processes it and dispatches it
 */
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid ETAL_CommunicationLayer_ThreadEntry_MDR(tSInt stacd, tPVoid dummy)
#else
static tVoid ETAL_CommunicationLayer_ThreadEntry_MDR(tPVoid dummy)
#endif
{
#ifndef CONFIG_ETAL_CPU_IMPROVEMENT
	tU32 len;
	tU8 lun;
	tBool got_data = FALSE;
	OSAL_tMSecond	vl_currentTime;
	static OSAL_tMSecond vl_mytime = 0;
#else 
    tSInt vl_res;
	OSAL_tEventMask vl_resEvent = ETAL_COMM_MDR_NO_EVENT_FLAG;
#endif //CONFIG_ETAL_CPU_IMPROVEMENT

	
#define TMP_MARGIN_SCHEDULING 10

#ifndef CONFIG_ETAL_CPU_IMPROVEMENT
	/*
	 * This buffer is used to hold data received from the DCOP
	 * The max size depends on the LUN (e.g. ETAL_CONTROL_LUN has much
	 * smaller max size than ETAL_DATA_LUN) it must be dimensioned
	 * to accept the largest payload
	 */
	static tU8 data[ETAL_MAX_RESPONSE_LEN];
#endif 

	while (TRUE)
	{

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_ThreadEntry_MDR: wait  event ETAL_commMDR_TaskEventHandler %d",  ETAL_commMDR_TaskEventHandler);

        vl_res = OSAL_s32EventWait (ETAL_commMDR_TaskEventHandler,
                                 ETAL_COMM_MDR_EVENT_WAIT_ALL, 
                                 OSAL_EN_EVENTMASK_OR, 
                                 ETAL_COMM_MDR_EVENT_WAIT_TIME_MS,
                                 &vl_resEvent);


		//ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_ThreadEntry_MDR: scheduling event vl_res = %d, vl_resEvent = 0x%x", vl_res, vl_resEvent);
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_ThreadEntry_MDR: scheduling event vl_res = %d, vl_resEvent = 0x%x, ETAL_commMDR_TaskEventHandler %d", vl_res, vl_resEvent, ETAL_commMDR_TaskEventHandler);

		if (OSAL_ERROR == vl_res)
		{
			// Event wait failure ==> break;
			
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_ThreadEntry_MDR: wait error");
			break;
		}
		else if ((vl_resEvent == ETAL_COMM_MDR_NO_EVENT_FLAG) || (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res))
		{
			// this is a timeout : no even received
		}
		else if (OSAL_OK == vl_res)
		{
			// This is a ETAL_COMM_MDR_EVENT_DATA_TO_TX_FLAG call the Tx event handler
			if (ETAL_COMM_MDR_EVENT_DATA_TO_TX_FLAG == (ETAL_COMM_MDR_EVENT_DATA_TO_TX_FLAG & vl_resEvent))
			{	
				// clear event now that it will be processed
				ETAL_CommMdr_TaskClearEventFlag(ETAL_COMM_MDR_EVENT_DATA_TO_TX_FLAG);
				// process		
				ETAL_CommunicationLayer_Send_MDR(&ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus);				
			}
			
			// This is a ETAL_COMM_MDR_EVENT_DATA_RX_FLAG call the Rx event handler
			if (ETAL_COMM_MDR_EVENT_DATA_RX_FLAG == (ETAL_COMM_MDR_EVENT_DATA_RX_FLAG & vl_resEvent))
			{
				// clear event now that it will be processed
				ETAL_CommMdr_TaskClearEventFlag(ETAL_COMM_MDR_EVENT_DATA_RX_FLAG);
				// process	
				// Manage data reception
				ETAL_CommMdr_ManageDataReception();				
			}
			
			// unexepected event case
			if (!(ETAL_COMM_MDR_EVENT_WAIT_MASK & vl_resEvent))
			{
				// just clear the event
				ETAL_CommMdr_TaskClearEventFlag(vl_resEvent);
				
				ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_ThreadEntry_MDR: unexpected event received vl_res = %d, vl_resEvent = 0x%x, ETAL_commMDR_TaskEventHandler %d ", vl_res, vl_resEvent, ETAL_commMDR_TaskEventHandler);
			}
		}
		else
		{
			// should not come here all processed before
		}


#else // CONFIG_ETAL_CPU_IMPROVEMENT
		vl_currentTime = OSAL_ClockGetElapsedTime();

		if ((vl_currentTime - vl_mytime) > TMP_MARGIN_SCHEDULING)
		{
			
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "ETAL_CommunicationLayer_ThreadEntry_MDR: wrong scheduling %d",	(vl_currentTime - vl_mytime) );
		}

		vl_mytime = vl_currentTime;

		ETAL_CommunicationLayer_Send_MDR(&ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus);

		do
		{
			got_data = FALSE;
			if (DAB_Get_Response(data, ETAL_MAX_RESPONSE_LEN, &len, &lun)== OSAL_OK)
			{
			    if(len != 0)
			    {
			        got_data = TRUE;
			    }
			}

			if (got_data)
			{
				ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Received message ID 0x%.3x from DCOP on LUN 0x%.2x, %d bytes", ETAL_MDR_COMMAND_ID(data), lun, len);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
				COMMON_tracePrintBufComponent(TR_CLASS_APP_ETAL_COMM, data, len, NULL);
#endif
#ifdef CONFIG_COMM_DRIVER_DIRECT
				if (ETAL_directToMDRCmdMode == FALSE)
				{
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT
					switch (lun)
					{
						case ETAL_CONTROL_LUN:
							ETAL_CommunicationLayer_Receive_MDR(data, len, &ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus);
							break;
						case ETAL_DATA_LUN:
							ETAL_CommunicationLayer_ReceiveData_MDR(data, len);
							break;
						case ETAL_AUTONOTIF_LUN:
							ETAL_CommunicationLayer_ReceiveAutonotification_MDR(data, len);
							break;
						case ETAL_BROADCAST_LUN:
							ETAL_CommunicationLayer_ReceiveBroadcast_MDR(data, len, &ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus);
							break;
						default:
							ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Unexpected message from DCOP on LUN 0x%x", lun);
						break;
					}
#ifdef CONFIG_COMM_DRIVER_DIRECT
				}
				else
				{
					ETAL_CommunicationLayer_Receive_Direct_MDR(data, len, &ETAL_toMDRCmdBuffer, &ETAL_toMDRCmdStatus, lun);
				}
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

			ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "DONE / message ID 0x%.3x from DCOP on LUN 0x%.2x, %d bytes", ETAL_MDR_COMMAND_ID(data), lun, len);

			}
		} while (got_data);

		OSAL_s32ThreadWait(ETAL_COMM_MDR_THREAD_SCHEDULING);
#endif // CONFIG_ETAL_CPU_IMPROVEMENT


	}

}

#ifdef CONFIG_COMM_DRIVER_DIRECT
/***************************
 *
 * ETAL_sendDirectCommandTo_MDR
 *
 **************************/
/*
 * Function called by all ETAL API functions to send
 * command to the MDR and wait a response.
 *
 * Returns:
 *  OSAL_ERROR
 *  	command sent, got a response with error
 *  	the command status is in <cstat>
 *  OSAL_ERROR_TIMEOUT_EXPIRED
 *  	timeout during command send or response/notify receive
 *  OSAL_ERROR_DEVICE_INIT
 *  	the IPForward thread is not connected to the ProtocolLayer process
 *  OSAL_OK
 *  	the response, if requested with <has_response>, is in <resp> 
 *
 *  IMPORTANT: <resp> remains vaild until the next call to ETAL_sendCommandTo_MDR.
 *
 */
tSInt ETAL_sendDirectCommandTo_MDR(tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen)
{
	return ETAL_sendCommandWithRetryTo_MDR(Cmd_Mdr_Type_Direct, cmd, clen, cstat, has_response, resp, rlen, 0);
}
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

/***************************
 *
 * ETAL_sendCommandSpecialTo_MDR
 *
 **************************/
/*
 * Function called by all ETAL API functions to send
 * special command to the MDR and wait a response.
 *
 * Returns:
 *  OSAL_ERROR
 *  	command sent, got a response with error
 *  	the command status is in <cstat>
 *  OSAL_ERROR_TIMEOUT_EXPIRED
 *  	timeout during command send or response/notify receive
 *  OSAL_ERROR_DEVICE_INIT
 *  	the IPForward thread is not connected to the ProtocolLayer process
 *  OSAL_OK
 *  	the response, if requested with <has_response>, is in <resp> 
 *
 *  IMPORTANT: <resp> remains vaild until the next call to ETAL_sendCommandTo_MDR.
 *
 */
tSInt ETAL_sendCommandSpecialTo_MDR(tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen)
{
	return ETAL_sendCommandWithRetryTo_MDR(Cmd_Mdr_Type_Special, cmd, clen, cstat, has_response, resp, rlen, 0);
}

/***************************
 *
 * ETAL_sendCommandTo_MDR
 *
 **************************/
/*
 * Function called by all ETAL API functions to send
 * command to the MDR and wait a response.
 *
 * Returns:
 *  OSAL_ERROR
 *  	command sent, got a response with error
 *  	the command status is in <cstat>
 *  OSAL_ERROR_TIMEOUT_EXPIRED
 *  	timeout during command send or response/notify receive
 *  OSAL_ERROR_DEVICE_INIT
 *  	the IPForward thread is not connected to the ProtocolLayer process
 *  OSAL_OK
 *  	the response, if requested with <has_response>, is in <resp> 
 *
 *  IMPORTANT: <resp> remains vaild until the next call to ETAL_sendCommandTo_MDR.
 *
 */
tSInt ETAL_sendCommandTo_MDR(tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen)
{
	return ETAL_sendCommandWithRetryTo_MDR(Cmd_Mdr_Type_Normal, cmd, clen, cstat, has_response, resp, rlen, 0);
}

/***************************
 *
 * ETAL_sendCommandWithRetryTo_MDR
 *
 **************************/
/*
 * Same as ETAL_sendCommandTo_MDR but also implements
 * a retry: if the command requires a response and none is 
 * received within <retry_delay>, re-send the command.
 * This is used in GetQuality commands.
 *
 * Note that the new command will find the ETAL_CommunicationLayer_Receive_MDR
 * in the cmdStatusWaitResponseMDR (it is still waiting for
 * the response to the previous command) thus the notification
 * may result in a "Unexpected notification" message, which is
 * perfectly normal in this case.
 */
tSInt ETAL_sendCommandWithRetryTo_MDR(etalCmdMdrTypeTy cmdType, tU8 *cmd, tU32 clen, tU16 *cstat, tBool has_response, tU8 **resp, tU32 *rlen, tU32 retry_delay)
{
	OSAL_tMSecond start_time, end_time;
	tSInt retval = OSAL_OK;
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT	
	tS32 vl_res = OSAL_OK;
#else
	OSAL_tMSecond current_time, retry_time = 0;
#endif

	/*
	 * protect against concurrent accesses from the ETAL API since there is only
	 * one command buffer
	 */
	OSAL_s32SemaphoreWait(ETAL_toMDRCmdBufferSem, OSAL_C_TIMEOUT_FOREVER);

	if (ETAL_toMDRCmdBufferHaveNew)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Consistency check fail (would overwrite the DCOP command buffer)");
		ETAL_ResetCommandBuffer_MDR();
	}

	start_time = OSAL_ClockGetElapsedTime();
    if ((cmdType & Cmd_Mdr_Type_Special) == Cmd_Mdr_Type_Special)
    {
        /* check minimum size for special command */
        if (clen < 6)
        {
            return OSAL_ERROR;
        }
        /* set special commands type 1, 7, 8 */
        ETAL_toMDRCmdBuffer.LunId = ETAL_BROADCAST_LUN;
        ETAL_toMDRCmdBuffer.specific0 = cmd[0];
        ETAL_toMDRCmdBuffer.specific1 = cmd[1];
        ETAL_toMDRCmdBuffer.specific2 = cmd[2];
        ETAL_toMDRCmdBuffer.specific3 = cmd[3];
        ETAL_toMDRCmdBuffer.cmdLen = ((((tU32)cmd[4]) << 8) | cmd[5]);
        ETAL_toMDRCmdBuffer.cmd = &(cmd[6]);
		end_time = start_time + ETAL_TO_MDR_SPECIAL_CMD_TIMEOUT_IN_MSEC;
    }
    else
    {
        /* set normal commands type 0 */
        ETAL_toMDRCmdBuffer.LunId = ETAL_CONTROL_LUN;
        ETAL_toMDRCmdBuffer.specific0 = (tU8)0;
        ETAL_toMDRCmdBuffer.specific1 = (tU8)0;
        ETAL_toMDRCmdBuffer.specific2 = (tU8)0;
        ETAL_toMDRCmdBuffer.specific3 = (tU8)0;
    	ETAL_toMDRCmdBuffer.cmdLen = clen;
        ETAL_toMDRCmdBuffer.cmd = cmd;
		end_time = start_time + ETAL_TO_MDR_CMD_TIMEOUT_IN_MSEC;
    }
	ETAL_toMDRCmdBuffer.rspLen = 0;
	ETAL_toMDRCmdBuffer.cmdHasResponse = has_response;

	ETAL_toMDRCmdBufferHaveNew = TRUE;
#ifdef CONFIG_COMM_DRIVER_DIRECT
	if ((cmdType & Cmd_Mdr_Type_Direct) == Cmd_Mdr_Type_Direct)
	{
		ETAL_directToMDRCmdMode = TRUE;
	}
	else
	{
		ETAL_directToMDRCmdMode = FALSE;
	}
#endif // #ifdef CONFIG_COMM_DRIVER_DIRECT

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT	
	ETAL_CommMdr_DataToTx();
#endif


	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "SendCommandTo_MDR cmd id 0x%.3x", ETAL_MDR_COMMAND_ID(cmd));
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT	

	vl_res = OSAL_s32SemaphoreWait(ETAL_toMDRCmdMsgTransmitted, (end_time - start_time));

	if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res)
	{
		// not yet answered !! 		
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "SendCommandTo_MDR cmd id 0x%.3x TIMEOUT sending, ETAL_toMDRCmdStatus = %d", ETAL_MDR_COMMAND_ID(cmd), ETAL_toMDRCmdStatus);		
	}
	else
	{	
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "ETAL_sendCommandWithRetryTo_MDR: ETAL_toMDRCmdMsgTransmitted received");
	}
		
#else
	if ((retry_delay > 0) && (start_time + retry_delay < end_time))
	{
		retry_time = start_time + retry_delay;
	}

	/* wait for FSM to send command to MDR */
	while (OSAL_ClockGetElapsedTime() < end_time)
	{
		if (!ETAL_toMDRCmdBufferHaveNew)
		{
			break;
		}
		OSAL_s32ThreadWait(ETAL_API_THREAD_SCHEDULING);
	}
#endif

	if (ETAL_toMDRCmdBufferHaveNew)
	{
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "SendCommandTo_MDR cmd id 0x%.3x TIMEOUT, ETAL_toMDRCmdStatus = %d", ETAL_MDR_COMMAND_ID(cmd), ETAL_toMDRCmdStatus);
	}
	else
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
        if ((cmdType & Cmd_Mdr_Type_Special) == Cmd_Mdr_Type_Normal)
        {
    		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "SendCommandTo_MDR cmd id 0x%.3x sent", ETAL_MDR_COMMAND_ID(cmd));
        }
        else
        {
            ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "SendCommandSpecialTo_MDR LUN %x H0 %d H1 %d H2 %d H3 %d sent", ETAL_toMDRCmdBuffer.LunId, ETAL_toMDRCmdBuffer.specific0, ETAL_toMDRCmdBuffer.specific1, ETAL_toMDRCmdBuffer.specific2, ETAL_toMDRCmdBuffer.specific3);
        }
#endif

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT	
		// why should we retry ? 
		// we know it has been sent !!
		// 
		// get current time to master total elapsed time.
		start_time = OSAL_ClockGetElapsedTime();

		vl_res = OSAL_s32SemaphoreWait(ETAL_toMDRCmdMsgComplete, (end_time - start_time));
	
		if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res)
		{
			// not yet answered !! 
			ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "SendCommandTo_MDR cmd id 0x%.3x TIMEOUT, no answer from DCOP", ETAL_MDR_COMMAND_ID(cmd), ETAL_toMDRCmdStatus);		
		}
		else
		{
			ETAL_tracePrintUser1(TR_CLASS_APP_ETAL_COMM, "ETAL_sendCommandWithRetryTo_MDR: ETAL_toMDRCmdMsgComplete received");
		}
#else
		/* wait for the response(s) from MDR */
		while ((current_time = OSAL_ClockGetElapsedTime()) < end_time)
		{
			if ((ETAL_toMDRCmdStatus & ETAL_MDR_FSM_STATUS_RUNNING) != ETAL_MDR_FSM_STATUS_RUNNING)
			{
				break;
			}
			if ((retry_time > 0) && (current_time >= retry_time))
			{
				ETAL_toMDRCmdBufferHaveNew = TRUE;
				retry_time = current_time + retry_time;
			}
			OSAL_s32ThreadWait(ETAL_API_THREAD_SCHEDULING);
		}
#endif 

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
        if ((cmdType & Cmd_Mdr_Type_Special) == Cmd_Mdr_Type_Normal)
        {
    		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "SendCommandTo_MDR cmd id 0x%.3x finish, status %s", ETAL_MDR_COMMAND_ID(cmd), ETAL_cmdStatusToASCII_MDR(ETAL_toMDRCmdStatus));
        }
        else
        {
            ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "SendCommandSpecialTo_MDR LUN %x H0 %d H1 %d H2 %d H3 %d finish, status %s", ETAL_toMDRCmdBuffer.LunId, ETAL_toMDRCmdBuffer.specific0, ETAL_toMDRCmdBuffer.specific1, ETAL_toMDRCmdBuffer.specific2, ETAL_toMDRCmdBuffer.specific3, ETAL_cmdStatusToASCII_MDR(ETAL_toMDRCmdStatus));
        }
#endif

		if (resp != NULL)
		{
			*resp = NULL;
		}
		if (rlen != NULL)
		{
			*rlen = 0;
		}
		if (cstat != NULL)
		{
			*cstat = 0;
		}
	}

	/* process the response */
	if (ETAL_toMDRCmdStatus == cmdStatusCompleteMDR)
	{
		if (resp != NULL)
		{
			*resp = ETAL_toMDRCmdBuffer.rsp;
		}
		if (rlen != NULL)
		{
			*rlen = ETAL_toMDRCmdBuffer.rspLen;
		}
	}
	else if (ETAL_toMDRCmdStatus == cmdStatusErrorMDR)
	{
		if (cstat != NULL)
		{
			*cstat = (tU16)ETAL_toMDRCmdBuffer.cmdStatus;
		}
		retval = OSAL_ERROR;
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Error returned from DCOP (code 0x%x)", ETAL_toMDRCmdBuffer.cmdStatus);
	}
	else
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_ETAL >= TR_LEVEL_COMPONENT)
		/* leave the ifdef to avoid duplicated message (component level followed by error level) */
		ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "Timeout sending message to DCOP (state %s)", ETAL_cmdStatusToASCII_MDR(ETAL_toMDRCmdStatus));
#else
		ETAL_tracePrintError(TR_CLASS_APP_ETAL_COMM, "Timeout sending message to DCOP");
#endif
		retval = OSAL_ERROR_TIMEOUT_EXPIRED;
	}
	if (retval != OSAL_OK)
	{
		ETAL_ResetCommandBuffer_MDR();
	}
	OSAL_s32SemaphorePost(ETAL_toMDRCmdBufferSem);
	return retval;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
