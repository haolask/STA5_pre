//!
//!  \file    steci_protocol.cpp
//!  \brief   <i><b> STECI protocol</b></i>
//!  \details STECI protocol used by ETAL and by MDR_Protocol
//!  \author  Raffaele Belardi (copied from MDR_CONTROL_APPLICATION)
//!

#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

#include "osal.h"
#include "common_trace.h"
#include "steci_trace.h"

#include "steci_helpers.h"

#include "connection_modes.h"

#include "DAB_Protocol.h"
#include "DAB_internal.h"
#include "steci_lld.h"
#include "steci_crc.h"
#include "dcop_boot.h"
#include "steci_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

///
// DEFINES
///
// STECI state machine
#define STECI_SPI_IDLE                              0
#define STECI_TX_FRAME                              1
#define STECI_ERROR                                 2
#define STECI_RFU_1                                 3  // Unused
#define STECI_WAIT_REQ_PIN_TOGGLE                   4  
#define STECI_RX_HEADER                             5  
#define STECI_RX_PAYLOAD                            6  
#define STECI_DECODE_HEADER                         7  
#define STECI_MANAGE_PAYLOAD                        8   
#define STECI_MULTIFRAME_WAIT_REQ_PIN_TOGGLE		9

// Clash threshold
#define STECI_CLASH_DURING_RX_THRESHOLD				2

///
// MACROs
///

///
// Typedefs
///
typedef enum
{
	STECI_COMM_INIT,
	STECI_COMM_SEND_FRAME,
	STECI_COMM_NEW_SLOT_REQ,
	STECI_COMM_EXIT,
	STECI_COMM_ERROR
} STECI_commStateMachineEnumTy;

typedef enum
{
	STECI_COMM_STATUS_OK,
	STECI_COMM_STATUS_ERR_UNKNOWN,
	STECI_COMM_STATUS_ERR_NO_BOARD_RSP
} STECI_commStatusEnumTy;

typedef enum
{
	STECI_FEEDBACK_SENDER_IS_PROTOCOL,
	STECI_FEEDBACK_SENDER_IS_DEVICE
} STECI_feedbackSenderEnumTy;

typedef enum
{
	STECI_SEND_MODE_NONE		= 0,
	STECI_SEND_MODE_TX			= 1,
	STECI_SEND_MODE_RX_HEADER	= 2,
	STECI_SEND_MODE_RX_PAYLOAD	= 3
} STECI_sendModeEnumTy;

typedef struct
{
	tU16 status;
	tU16 lastStatus;
	tBool reqNextActiveStatus;
	tS32 error;
} STECI_statusTy;

typedef struct
{
	tU8 lun;
	tU32 totalLen;
	tU8 *msgStorageContentPtr;
} STECI_messageStorageTy;

typedef struct
{
	tS32 totalLen;
	tU8 *msgContentPtr;

	struct
	{
		STECI_headerType header;
		tU8 payload[STECI_MAX_PACKET_LENGTH];
	} msg;

	STECI_messageStorageTy messageStorage;
} STECI_messageType;


///
// Local variables
///

///
// GLOBAL variables
///

OSAL_tSemHandle      STECI_ProtocolHandle_MsgSendingSem;
OSAL_tSemHandle      STECI_ProtocolHandle_MsgSentSem;


///
// Local functions declarations
///
static tS32 STECI_SendDataSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tBool *reqNextActiveStatusPtr, tU8 *txDataBuffer, tU32 txBytesNum,
									 tU8 *rxDataBuffer, tU32 rxMaxSize, tU32 *rxBytesNum, tU8 *tmpTxDataPtr,
									 tBool waitReadyCondition, tBool closeCommunication, STECI_sendModeEnumTy sendMode);

static tS32 STECI_MessageSend (STECI_deviceInfoTy *deviceInfoPtr, tBool *reqNextActiveStatusPtr, STECI_txRxInfo *infoPtr,
							   tU8 *rxDataBuffer, tU8 *txBufferPtr, tU32 rxMaxSize, tU32 *receivedBytesPtr, tU8 *tmpTxDataPtr);

static tBool STECI_CheckDeviceRequest (STECI_deviceInfoTy *deviceInfoPtr, STECI_statusTy *statusPtr);

static tVoid STECI_ChangeStatus (STECI_statusTy *statusPtr, tU8 newStatus);

static tVoid STECI_ExecuteSpecialCommands (STECI_deviceInfoTy *deviceInfoPtr, STECI_cmdModeEnumTy cmd, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr);

static tVoid STECI_ReturnToIdleStatus (STECI_deviceInfoTy *deviceInfoPtr, STECI_statusTy *steciStatusPtr, tBool toggleReq);

static tS32 STECI_MessageFeedback (tVoid *deviceHandle, tU8 *dataPtr, tU32 numBytes, STECI_commStatusEnumTy errMsg, STECI_feedbackSenderEnumTy feedbackSender);

static tS32 STECI_HeaderDecode (tU8 *dataPtr, tS32 *dataToSend, STECI_messageType *messageDataPtr);

static tS32 STECI_StorePartialMessage (STECI_messageStorageTy *messageStoragePtr, tU8 *msgContentPtr, tU32 messageLen, tU8 newLun, tBool firstFragment);

static tS32 STECI_MessageDecode (tVoid *deviceHandle, tU8 *dataPtr, tU32 numBytes, STECI_messageType *messageDataPtr);

#if (defined CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_ERRORS)
static tChar *STECI_Status2String(tU32 status)
{
	switch (status)
	{
		case STECI_SPI_IDLE:
			return "STECI_SPI_IDLE";
		case STECI_TX_FRAME:
			return "STECI_TX_FRAME";
		case STECI_ERROR:
			return "STECI_ERROR";
		case STECI_WAIT_REQ_PIN_TOGGLE:
			return "STECI_WAIT_REQ_PIN_TOGGLE";
		case STECI_RX_HEADER:
			return "STECI_RX_HEADER";
		case STECI_RX_PAYLOAD:
			return "STECI_RX_PAYLOAD";
		case STECI_DECODE_HEADER:
			return "STECI_DECODE_HEADER";
		case STECI_MANAGE_PAYLOAD:
			return "STECI_MANAGE_PAYLOAD";
		default:
			return "UNKNOWN";
	}
}
#endif


///
// Global functions
///


#ifdef CONFIG_HOST_OS_LINUX 
void STECI_ProtocolHandle (tPVoid pParams)
#elif defined (CONFIG_HOST_OS_FREERTOS) 
void STECI_ProtocolHandle (tPVoid pParams)
#elif defined (CONFIG_HOST_OS_TKERNEL) 
tVoid STECI_ProtocolHandle(tSInt stacd, tPVoid pParams)
#else
	#error "Unsupported OS"
#endif
{
    tS32 dataToSendLength;
    tU32 receivedBytes;
    tU32 payloadDataAlreadyInsideRxBfr;
    tU32 startLocation;
	STECI_statusTy steciStatus;
    tU32 totalRxBytes;
    STECI_paramsTy *paramsPtr = (STECI_paramsTy *)pParams;
    STECI_txRxInfo *txRxInfoPtr;
	STECI_messageType messageData;
	tSInt consecutiveClashNumber;
	tBool closeCs;
	tBool waitDeviceReady;
	STECI_deviceInfoTy deviceInfo;

    // Init the device handler used by this protocol
	deviceInfo.deviceHandle = paramsPtr->devHandle;
	deviceInfo.deviceSecondaryHandle = paramsPtr->devSecondaryHandle;
	deviceInfo.deviceMode = paramsPtr->deviceMode;
    txRxInfoPtr = &paramsPtr->txRxData;



	
    // Init crc
    STECI_CrcInit ();

    // Init state machine variables
	steciStatus.lastStatus = STECI_SPI_IDLE;
	steciStatus.status = STECI_WAIT_REQ_PIN_TOGGLE;
	steciStatus.reqNextActiveStatus = false;
	steciStatus.error = STECI_STATUS_SUCCESS;
	//steciStatus.reqNextActiveStatus = STECI_ToggleReqLevel (steciStatus.reqNextActiveStatus); // TODO: remove, it is for clash testing

	OSAL_pvMemorySet ((tVoid *)&messageData, 0x00, sizeof (messageData));
	messageData.msgContentPtr = &messageData.msg.payload[0];
	messageData.messageStorage.msgStorageContentPtr = NULL;
	messageData.messageStorage.lun = STECI_INVALID_LUN;
	messageData.messageStorage.totalLen = 0;

	consecutiveClashNumber = 0;

	
#if defined (CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED)
		(tVoid) consecutiveClashNumber;
#endif

    while (TRUE) 
    {
        // Handle current protocol status
		switch (steciStatus.status)
        {
            case STECI_SPI_IDLE:
            case STECI_WAIT_REQ_PIN_TOGGLE:		
				// Check if some acion shall be taken:
				// 1. Check if a special command has been received (i.e. RESET device)
				// 2. Check if some data has to be send (this shall be prioritized to receive data
				//    in order to always being able to send command, even with high data port dat out)
				// 3. Check if the device has to send something controlling REQ line status
				// If none of these events occur then wait for 1ms
				if (STECI_NORMAL_MODE != paramsPtr->cmdMode)
                {
					// Execute the special command
					STECI_ExecuteSpecialCommands (&deviceInfo, paramsPtr->cmdMode, &paramsPtr->inDataBuffer[0],
						                          paramsPtr->txRxData.TotalBytesNum, paramsPtr->txRxData.DataBuffer);

                    // post the semaphore : to indicate message is sent
                    OSAL_s32SemaphorePost(STECI_ProtocolHandle_MsgSentSem);

					// Return to normal operations
					paramsPtr->cmdMode = STECI_NORMAL_MODE;
				}
				else if (true == STECI_CheckDeviceRequest (&deviceInfo, &steciStatus))
				{
					// We have data to RX, prepare buffers
					OSAL_pvMemorySet ((tVoid *)paramsPtr->inDataBuffer, 0x00, STECI_MAX_PACKET_LENGTH);
					OSAL_pvMemorySet ((tVoid *)paramsPtr->outDataBuffer, 0x00, STECI_MAX_PACKET_LENGTH);
					payloadDataAlreadyInsideRxBfr = 0;
					dataToSendLength = 0;

					// We do not need to wait device to be ready, it is
					waitDeviceReady = false;

					// Set status
					STECI_ChangeStatus (&steciStatus, STECI_RX_HEADER);
				}
				// half duplex : check Tx after Rx
				// Full duplex : keep also Rx before Tx to avoid clashes
				// ie 1st check the Rx since Tx - Rx will lead to clash with priority to Rx
				else if (true == txRxInfoPtr->dataToBeTx)
                {
					// We have data to TX, prepare buffers
					OSAL_pvMemorySet ((tVoid *)paramsPtr->inDataBuffer, 0x00, STECI_MAX_PACKET_LENGTH);
					OSAL_pvMemorySet ((tVoid *)paramsPtr->outDataBuffer, 0x00, STECI_MAX_PACKET_LENGTH);
					payloadDataAlreadyInsideRxBfr = 0;
					dataToSendLength = 0;
					consecutiveClashNumber = 0;

					// Set status
					STECI_ChangeStatus (&steciStatus, STECI_TX_FRAME);
                }
				else
				{
					// We will rest if we do not have something to do already
					OSAL_s32ThreadWait (ETAL_STECI_THREAD_SCHEDULING);
				}
            break;

            case STECI_RX_HEADER:
                dataToSendLength = STECI_HEADER_LENGTH;

				// Send a zero buffer to clock the device and get the header back
				steciStatus.error = STECI_SendDataSteciMode (&deviceInfo,
					&steciStatus.reqNextActiveStatus,
					&paramsPtr->outDataBuffer[0], 
					(tU32)dataToSendLength,// txBytesNum
					&paramsPtr->inDataBuffer[0], 
					STECI_MAX_PACKET_LENGTH, // rxMaxSize
					&receivedBytes,
					&paramsPtr->tmpBuffer[0],
					waitDeviceReady, false, STECI_SEND_MODE_RX_HEADER);

                // Check the number of bytes received
				if (STECI_STATUS_SUCCESS == steciStatus.error)
                {
					// We did not have a clash
					consecutiveClashNumber = 0;

                    // Set the data already inside the rx payload buffer to 0
                    // to indicate that we did not received any data
                    payloadDataAlreadyInsideRxBfr = 0;

					// Set status
					STECI_ChangeStatus (&steciStatus, STECI_DECODE_HEADER);
                }
				else if (STECI_STATUS_DEV_NOT_READY == steciStatus.error)
				{
					// We did not have a clash
					consecutiveClashNumber = 0;

					// Return to idle status
					STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, false);

					// Warning message
					STECI_tracePrintSystem(TR_CLASS_STECI, "WARNING: device not ready during RX of header");
				}
//#if (!defined(CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED))
				else if (STECI_STATUS_HOST_DEVICE_CLASH == steciStatus.error)
				{
					// Increment the number of consecutive clashes of the type: 'HOST think DEVICE is TX'
					consecutiveClashNumber++;

					if (consecutiveClashNumber < STECI_CLASH_DURING_RX_THRESHOLD)
					{
						// Return to idle status
						STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, false);
					}
					else
					{
						// Enter error case because the device is probably off or stuck
						STECI_ChangeStatus (&steciStatus, STECI_ERROR);
					}

					// Warning message
					STECI_tracePrintSystem(TR_CLASS_STECI, "WARNING: data clash recognized during RX of header");
					
				}
//#endif // #if (!defined CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED)
                else
                {
					// Enter error case
					STECI_ChangeStatus (&steciStatus, STECI_ERROR);
                }
            break;

            case STECI_RX_PAYLOAD:
                // Check if we already have something (COLLISION or not)
                if (0 == payloadDataAlreadyInsideRxBfr)
                {
                    // Prepare the RX buffer               
                    startLocation = 0;
                }
                else
                {
                	// Here we focus on the Rx and payload
                	// If we come here it means we were in clash situation : Rx received while Tx
                	// the Rx is the Rx frame : so it includes the STECI header + Payload
                	// 
                	// calculate remaining payload : 
                    dataToSendLength -= payloadDataAlreadyInsideRxBfr;

					// set the start location to avoid erasing data
					// Prepare the received buffer
					//
					// copy the 1st payload received in the buffer ie remove the header which may be in
					//
					if (payloadDataAlreadyInsideRxBfr > 0)
					{
						OSAL_pvMemoryCopy ((tVoid *)&paramsPtr->inDataBuffer[0], (tPCVoid)&paramsPtr->inDataBuffer[STECI_HEADER_LENGTH], payloadDataAlreadyInsideRxBfr);
					}

					startLocation = payloadDataAlreadyInsideRxBfr;

					// reset where we put the data
                    //startLocation = payloadDataAlreadyInsideRxBfr + STECI_HEADER_LENGTH;
					OSAL_pvMemorySet ((tVoid *)(paramsPtr->inDataBuffer + startLocation), 0x00, (STECI_MAX_PACKET_LENGTH - startLocation));

					//+DEBUG
					// log the data
//					printf("STECI_ProtocolHandle Rx payload :dataToSendLength = %d, payloadDataAlreadyInsideRxBfr = %d, startLocation = %d\n", 
//						dataToSendLength, payloadDataAlreadyInsideRxBfr, startLocation);
					//-DEBUG
                }


				// Try : reset the data content
				// the In data buffer will be overwritten anyway
				//
//  				OSAL_pvMemorySet ((tVoid *)(paramsPtr->inDataBuffer+startLocation), 0x00, (STECI_MAX_PACKET_LENGTH- startLocation));
									
                // If we have still data to receive then clock the device
                if (dataToSendLength > 0)
                {
					// We move CS only if this is the last header
					if (SET == messageData.msg.header.h0.bitfield.lastFragment)
					{
						closeCs = true; // We close the CS because this is the last block
					}
					else
					{
						closeCs = false; // We do not close CS because we are receiving an intermediate block
					}

					steciStatus.error = STECI_SendDataSteciMode (&deviceInfo,
						&steciStatus.reqNextActiveStatus,
						&paramsPtr->outDataBuffer[0],
						(tU32)dataToSendLength,
						&paramsPtr->inDataBuffer[startLocation],
						(STECI_MAX_PACKET_LENGTH-startLocation),
						&receivedBytes,
						&paramsPtr->tmpBuffer[0],
						false, closeCs, STECI_SEND_MODE_RX_PAYLOAD);
                }
                else
                {
                    // Set to 0 the received bytes, we did not call the function to write
                    receivedBytes = 0;

                    // Force success status because this means we already have in the buffer
                    // the data we need
					steciStatus.error = STECI_STATUS_SUCCESS;

					// Else : it means on the clash we have received the full frame 
					// 
					// the CS is stil present : we should release it
					//??
					// We move CS only if this is the last header
					if (SET == messageData.msg.header.h0.bitfield.lastFragment)
					{
						// We close the CS because this is the last block
						STECI_ChipSelectDeassert(&deviceInfo);
                	}
					else
					{
						 // We do not close CS because we are receiving an intermediate block
					}

                }

				
                // Save received bytes number
                totalRxBytes = receivedBytes + payloadDataAlreadyInsideRxBfr + STECI_HEADER_LENGTH;

				//+DEBUG
					// log the data
#if 1					
					if (payloadDataAlreadyInsideRxBfr > 0)
					{
						STECI_tracePrintSystem(TR_CLASS_STECI, "STECI_ProtocolHandle Rx clash : totalRxBytes = %d, receivedBytes = %d, payloadDataAlreadyInsideRxBfr = %d\n", 
							totalRxBytes, receivedBytes, payloadDataAlreadyInsideRxBfr);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_SYSTEM)
						COMMON_tracePrintBufSystem(TR_CLASS_STECI, paramsPtr->inDataBuffer, totalRxBytes, "STECI_ProtocolHandle Rx clash data : ");
#endif
					}
#endif 
				//-DEBUG

                // Check the numbe rof bytes received
				if (STECI_STATUS_SUCCESS == steciStatus.error)
                {
                    
					// Set status
					STECI_ChangeStatus (&steciStatus, STECI_MANAGE_PAYLOAD);
                }
				else if (STECI_STATUS_DEV_NOT_READY == steciStatus.error)
				{
					// Return to idle status
					STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, false);
				}
                else
                {
					// Enter error case
					STECI_ChangeStatus (&steciStatus, STECI_ERROR);
                }
            break;

            case STECI_DECODE_HEADER:
                // Parse header
				steciStatus.error = STECI_HeaderDecode (&paramsPtr->inDataBuffer[0], &dataToSendLength, &messageData);

                // Check result
				if (STECI_STATUS_DATA_NOT_PRESENT == steciStatus.error)
                {
					// Return to idle status
					STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, false);
				}
				else if (STECI_STATUS_SUCCESS == steciStatus.error)
                {
					// Set status
					STECI_ChangeStatus (&steciStatus, STECI_RX_PAYLOAD);
                }                
				else if (STECI_STATUS_HEADER_RE_TX_FLAG == steciStatus.error)
                {
#if (defined CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_ERRORS)
				/* keep under ifdef for STECI_Status2String */
				STECI_tracePrintError(TR_CLASS_STECI, "STECI_DECODE_HEADER / STECI_STATUS_HEADER_RE_TX_FLAG : %d from status %s", steciStatus.error, STECI_Status2String(steciStatus.status));
#endif
					// Return to idle status
					STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, false);
                }
                else
                {
                    // Enter error case
					STECI_ChangeStatus (&steciStatus, STECI_ERROR);
                }
            break;
            
            case STECI_MANAGE_PAYLOAD:
                // Decode data and pass it to TCP/IP layer
                // we pass only the data without the HEADER : PAYLOAD is here
                //
				steciStatus.error = STECI_MessageDecode (deviceInfo.deviceHandle, &paramsPtr->inDataBuffer[0], (totalRxBytes - STECI_HEADER_LENGTH), &messageData);

                // All data used, clear info
                totalRxBytes = 0;

                // Check errors
				if (STECI_STATUS_INTERMEDIATE_FRAME == steciStatus.error)
                {
                    // In this case we need to receive other frames from this payload,
                    // avoid to check if some data shall be send until all pieces are received
					STECI_ChangeStatus (&steciStatus, STECI_MULTIFRAME_WAIT_REQ_PIN_TOGGLE);
					//STECI_tracePrintError(TR_CLASS_STECI, "STECI_STATUS_INTERMEDIATE_FRAME");

                }
				else if (STECI_STATUS_SUCCESS == steciStatus.error)
                {
					// Set next REQ active level
					steciStatus.reqNextActiveStatus = STECI_ToggleReqLevel (steciStatus.reqNextActiveStatus);

                    // Data correclty received and sent to host so we can return idle status
					STECI_ChangeStatus (&steciStatus, STECI_WAIT_REQ_PIN_TOGGLE);
                }                 
                else
                {
					// Enter error case
					STECI_ChangeStatus (&steciStatus, STECI_ERROR);
                }                
            break;

			case STECI_MULTIFRAME_WAIT_REQ_PIN_TOGGLE:
				// Set next REQ active level
				steciStatus.reqNextActiveStatus = STECI_ToggleReqLevel (steciStatus.reqNextActiveStatus);

				// We need to wait the device to become ready
				waitDeviceReady = true;

				// Another piece of this multi-fragment trasmission is ready, 
				// change status and get it
				STECI_ChangeStatus (&steciStatus, STECI_RX_HEADER);
			break;

            case STECI_TX_FRAME:
                // Send message
                
				
				// Send the data
				STECI_tracePrintComponent(TR_CLASS_STECI, "STECI_TX_FRAME Tx: %d bytes", txRxInfoPtr->TotalBytesNum);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
				COMMON_tracePrintBufComponent(TR_CLASS_STECI, (tU8 *)txRxInfoPtr->DataBuffer, txRxInfoPtr->TotalBytesNum, "Tx: ");
#endif						

				steciStatus.error = STECI_MessageSend (&deviceInfo, &steciStatus.reqNextActiveStatus, txRxInfoPtr,
					                     &paramsPtr->inDataBuffer[0], &paramsPtr->outDataBuffer[0],
										                STECI_MAX_PACKET_LENGTH, &receivedBytes, &paramsPtr->tmpBuffer[0]);

                // If success then we return to the wait status, otherwise in case of errors:
                // FULL DUPLEX, no more CLASH

			
				// - If there's a CLASH we do not close the CS because we need to serve the device request
				// - If the device is NOT READY we return to wait status letting the device to return to ready status
				// - Other errors are managed entering error status
				if (STECI_STATUS_HOST_DEVICE_CLASH == steciStatus.error)
                {
					// Warning message
					STECI_tracePrintSystem(TR_CLASS_STECI, "WARNING: Rx recognized during TX of frame");

	                // Set the data already received during full-duplex communication, we need
                    // to remove the steci header length because we consider only the payload
                    payloadDataAlreadyInsideRxBfr = receivedBytes - STECI_HEADER_LENGTH;

					// clear the data output 
					// for Rx
					// Input will be reused
					// datatosend should be cleared will be updated on STECI_DECODE_HEADER
					//
					OSAL_pvMemorySet ((tVoid *)paramsPtr->outDataBuffer, 0x00, STECI_MAX_PACKET_LENGTH);
					dataToSendLength = 0;

					// We do not need to wait device to be ready, it is
					waitDeviceReady = false;

#if defined (CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED)
					// consider the data sent ok 
					// 
					// Update data for caller, we transmitted all data
					txRxInfoPtr->dataToBeTx = false;

					// post the semaphore : to indicate message is sent
					OSAL_s32SemaphorePost(STECI_ProtocolHandle_MsgSentSem);
					
					// no other changes : process the Rx
					//
					
#else
					
					// We need to re-send the message with re-transmission bit properly set
					txRxInfoPtr->retry = true;

#endif // (!defined CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED)					


					//+DEBUG
					// log the data
#if 0
					STECI_tracePrintSystem(TR_CLASS_STECI, "STECI_ProtocolHandle Rx clash : totalRxBytes = %d, receivedBytes = %d, payloadDataAlreadyInsideRxBfr = %d\n", 
							totalRxBytes, receivedBytes, payloadDataAlreadyInsideRxBfr);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_SYSTEM
						COMMON_tracePrintBufSystem(TR_CLASS_STECI, paramsPtr->inDataBuffer, totalRxBytes, "STECI_ProtocolHandle Rx clash data : ");
#endif

					STECI_tracePrintSystem(TR_CLASS_STECI, "STECI_ProtocolHandle Tx clash :receivedBytes = %d, payloadDataAlreadyInsideRxBfr = %d\n", 
						receivedBytes, payloadDataAlreadyInsideRxBfr);
#if defined(CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_SYSTEM					
					COMMON_tracePrintBufSystem(TR_CLASS_STECI, paramsPtr->inDataBuffer, receivedBytes, "STECI_ProtocolHandle Tx clash data :  ");
#endif

#endif
					//-DEBUG
					
					// We need to toggle the REQ next active level to re-align at the device after a clash
					//steciStatus.reqNextActiveStatus = STECI_ToggleReqLevel (steciStatus.reqNextActiveStatus);

                    // Go to DECODE HEADER status to check if we need just to get the data or clock more,
                    // received data are inside 'inDataBuffer'
					STECI_ChangeStatus (&steciStatus, STECI_DECODE_HEADER);
                }
				else if (STECI_STATUS_DEV_NOT_READY == steciStatus.error)
				{
	
					// The device seems to be lock-down, we return to wait status and we try to send the data again
					// data have not been transmitted on SPI so no retransmission
					txRxInfoPtr->retry = false;

					// Return to idle status
					STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, false);

					// Warning message
					STECI_tracePrintComponent(TR_CLASS_STECI, "WARNING: STECI_STATUS_DEV_NOT_READY");

					OSAL_s32ThreadWait(ETAL_STECI_THREAD_SCHEDULING); // [RB] solve ETAL hang after 6h of dataservice test. Not present in STECI repo
				}
				else if (receivedBytes > 0 && STECI_STATUS_SUCCESS == steciStatus.error)
                {

					// Update data for caller, we transmitted all data
					txRxInfoPtr->dataToBeTx = false;


					// post the semaphore : to indicate message is sent
					OSAL_s32SemaphorePost(STECI_ProtocolHandle_MsgSentSem);
					
					// Return to idle status
					STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, true);
                }
				else
				{
					// Enter error case
					STECI_ChangeStatus (&steciStatus, STECI_ERROR);
				}
            break;

            case STECI_ERROR:

				// If we have data to TX we increment the number of re-trial
				if (true == txRxInfoPtr->dataToBeTx && txRxInfoPtr->numberOfRetry < STECI_DATA_SEND_RETRY_MAX_NUM)
				{
					txRxInfoPtr->numberOfRetry++;
				}
				else if (true == txRxInfoPtr->dataToBeTx)
				{
					// Signal that data will not be transmitted again
					txRxInfoPtr->dataToBeTx = false;

					// Call data sender							
					(LINT_IGNORE_RET) STECI_MessageFeedback (deviceInfo.deviceHandle, &paramsPtr->txRxData.DataBuffer[0], 
										   paramsPtr->txRxData.RemainingBytesNum, STECI_COMM_STATUS_ERR_NO_BOARD_RSP,
										   STECI_FEEDBACK_SENDER_IS_PROTOCOL);

					// post the semaphore : to indicate message is sent
					OSAL_s32SemaphorePost(STECI_ProtocolHandle_MsgSentSem);
				}

				// Return to idle status
				STECI_ReturnToIdleStatus (&deviceInfo, &steciStatus, true);
            break;

            default:
                // Default manager: no code
            break;
        } 
    } 

    // We CAN never get there
    //STECI_tracePrintError(TR_CLASS_STECI, "Unwanted SPI EXIT state: request to exit from Steci protocol, entering infinite loop...");

    //return;
} 

tS32 STECI_SendMessageStart (tVoid *memoryPtr, tU8 LunId, tU8 *DataToSend, 
	                         tU8 specific0, tU8 specific1, tU8 specific2, tU8 specific3, tU16 BytesNumber)
{
    STECI_paramsTy *dataInfoPtr = (STECI_paramsTy *)memoryPtr;
#if 0
	tU32 vl_index;
#endif
	
    if ((tU32)BytesNumber > STECI_MAX_MESSAGE_LEN)
    {
        return STECI_STATUS_ERROR;
    }
    else 
    {
    	// take the semaphore to gurantee message sending are serialized
    	if (OSAL_s32SemaphoreWait(STECI_ProtocolHandle_MsgSendingSem, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
		{
			return STECI_STATUS_ERROR;
		}
  
        OSAL_pvMemoryCopy ((tPVoid)dataInfoPtr->txRxData.DataBuffer, (tPCVoid)DataToSend, BytesNumber);

		// Check modes
		if (STECI_RESET_CMD == specific0)
		{
			dataInfoPtr->cmdMode = STECI_RESET_MODE;
		}
		else if (STECI_BOOT_CMD == specific0)
		{
			dataInfoPtr->cmdMode = STECI_BOOT_MODE;
			dataInfoPtr->inDataBuffer[0] = specific1; // Here it is saved the boot mode 
		}
		else if (STECI_FLASH_CMD == specific0)
		{
			dataInfoPtr->cmdMode = STECI_FLASH_MODE;
			dataInfoPtr->inDataBuffer[0] = specific1; // Here it is saved the operation mode 
			dataInfoPtr->inDataBuffer[1] = specific2; // Here it is saved SPI2FLASH(=0x00) or SPI2MEM(=0x01) information
            dataInfoPtr->inDataBuffer[2] = specific3; // Here it is saved dataMode (=0x00) or FileMode(=0x01) information
            dataInfoPtr->txRxData.TotalBytesNum = BytesNumber;
			dataInfoPtr->txRxData.RemainingBytesNum = BytesNumber;
		}
		else
		{
			dataInfoPtr->cmdMode = STECI_NORMAL_MODE;
			dataInfoPtr->txRxData.TotalBytesNum = BytesNumber;
			dataInfoPtr->txRxData.RemainingBytesNum = BytesNumber;
			dataInfoPtr->txRxData.Lun = LunId;
			dataInfoPtr->txRxData.SnInfo = 0;
			dataInfoPtr->txRxData.retry = false;
			dataInfoPtr->txRxData.numberOfRetry = 0;		
			dataInfoPtr->txRxData.dataToBeTx = true;
		}

    	// wait the semaphore to gurantee message is sent
    	if (OSAL_s32SemaphoreWait(STECI_ProtocolHandle_MsgSentSem, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
		{
			// errorr in semaphore...
			// should not happen
			
    }

		
		// free the semaphore : the message is sent ok
		OSAL_s32SemaphorePost(STECI_ProtocolHandle_MsgSendingSem);

        return STECI_STATUS_SUCCESS;
} 
} 

///
// Local functions definitions
///
static tS32 STECI_SendDataSteciMode (STECI_deviceInfoTy *deviceInfoPtr, tBool *reqNextActiveStatusPtr, tU8 *txDataBuffer, tU32 txBytesNum,
									 tU8 *rxDataBuffer, tU32 rxMaxSize, tU32 *rxBytesNum, tU8 *tmpTxDataPtr,
									 tBool waitReadyCondition, tBool closeCommunication, STECI_sendModeEnumTy sendMode)
{
    tS32 res = STECI_STATUS_SUCCESS;

    tU32 cnt;
	tS32 writeResult;
	tU32 bytesEqualToZero;

    // Write data to device
	writeResult = STECI_SpiWriteSteciMode (deviceInfoPtr, (tVoid *)txDataBuffer, (tU32)txBytesNum,
		                                   (tVoid *)rxDataBuffer, reqNextActiveStatusPtr, tmpTxDataPtr,
		                                   waitReadyCondition, closeCommunication);

	// Check if we got an error code: negative values
	if (writeResult < 0)
	{
		return writeResult;
	}

	// Save read byte number
	*rxBytesNum = (tU32)writeResult;

//#if (!defined(CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED))
#if 1
    // Check for clash:
	// - When TX DATA we have clash if some byte is different from 0
	// - When RX DATA we have clash if all bytes are 0
	if (STECI_SEND_MODE_TX == sendMode)
    {
        for (cnt = 0; cnt < txBytesNum; cnt++)
        {
            if (STECI_MISO_UNDEFINED_DATA_BYTE != *(rxDataBuffer + cnt))
            {
                res = STECI_STATUS_HOST_DEVICE_CLASH;

                break;
            }
        }
    }
	else if (STECI_SEND_MODE_RX_HEADER == sendMode)
	{
		bytesEqualToZero = 0;

		for (cnt = 0; cnt < txBytesNum; cnt++)
		{
			if (0 == *(rxDataBuffer + cnt))
			{
				bytesEqualToZero++;
			}
		}

		if (bytesEqualToZero == txBytesNum)
		{
			res = STECI_STATUS_HOST_DEVICE_CLASH;
		}
	}

	// Warning messages
	if (STECI_STATUS_HOST_DEVICE_CLASH == res)
	{
		if (STECI_SEND_MODE_RX_HEADER == sendMode)
		{
			STECI_tracePrintSystem(TR_CLASS_STECI, "WARNING: Clash during HEADER rx");
		}
		else if (STECI_SEND_MODE_TX == sendMode)
		{
			STECI_tracePrintSystem(TR_CLASS_STECI, "WARNING: Rx during data tx");
		}
		else
		{
			STECI_tracePrintSystem(TR_CLASS_STECI, "WARNING: Clash");
        }
    }
#else
	UNUSED (cnt);
	UNUSED (bytesEqualToZero);
#endif // #if (!defined CONFIG_MDR_STECI_FULL_DUPLEX_ENABLED)

    return res;
}

static tS32 STECI_MessageSend (STECI_deviceInfoTy *deviceInfoPtr, tBool *reqNextActiveStatusPtr, STECI_txRxInfo *infoPtr,
                               tU8 *rxDataBuffer, tU8 *txBufferPtr, tU32 rxMaxSize, tU32 *receivedBytesPtr, tU8 *tmpTxDataPtr)
{
    tS32 remainingBytes;
    tS32 curSrcPos;
    tS32 curSlotSize, payloadLen, curSlotPaddingSize = 0;
    tS32 cnt;
    tBool firstPacket = true;
    tS32 res = STECI_STATUS_SUCCESS;

    // Get remaining bytes
    remainingBytes = (tS32)infoPtr->TotalBytesNum;

    // Set current read position in the stream
    curSrcPos = 0;

    // Loop until all data is TX
    while (remainingBytes > 0)
    {
        // Prepare the data to transmit
        if (remainingBytes <= STECI_MAX_PAYLOAD_LENGTH)
        {
            curSlotSize = STECI_HEADER_LENGTH + remainingBytes;
            payloadLen = remainingBytes;

            // Fill with zeroes to trasmit data as multiple of four
            curSlotPaddingSize = 0;
			while (0 != (curSlotSize & (STECI_PACKET_MODULO - 1)))
            {
                curSlotSize++;
                curSlotPaddingSize++;
            }

            remainingBytes = 0;
        }
        else
        {
            curSlotSize = STECI_MAX_PACKET_LENGTH;
            payloadLen = STECI_MAX_PAYLOAD_LENGTH;

            remainingBytes -= STECI_MAX_PAYLOAD_LENGTH;
        }

        // Clear buffer data
        OSAL_pvMemorySet ((void *)txBufferPtr, 0x00, STECI_MAX_PACKET_LENGTH);

        // Set data
        for (cnt = 0; cnt < (curSlotSize - STECI_HEADER_LENGTH - curSlotPaddingSize); cnt++)
        {
            *(txBufferPtr + STECI_HEADER_LENGTH + cnt) = infoPtr->DataBuffer[cnt];
        }

        // Set header
		if (false == infoPtr->retry)
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.reTransmission = 0;
		}
		else
		{
			// MDR3 interprets this bit as request to retransmit
			//((STECI_headerType *)txBufferPtr)->h0.reTransmission = 1;
		}

        if (true == firstPacket)
        {
            // First time we use as paylaod indication
            if (STECI_HEADER_LENGTH == curSlotSize)
            {
                ((STECI_headerType *)txBufferPtr)->h0.bitfield.noPayload = 1;
            }
            else
            {
                ((STECI_headerType *)txBufferPtr)->h0.bitfield.noPayload = 0;
            }

            firstPacket = false;
        }
        else
        {
            // Here we use as toggle
            ((STECI_headerType *)txBufferPtr)->h0.bitfield.noPayload ^= 1; 
        }

        ((STECI_headerType *)txBufferPtr)->lenLsb = (tU8)((payloadLen - 1) & (tU16)0x00FF);
        ((STECI_headerType *)txBufferPtr)->h0.bitfield.lenMsb = (tU8)(((payloadLen - 1) >> 8) & (tU16)0x00F);
            ((STECI_headerType *)txBufferPtr)->lun = infoPtr->Lun;
        ((STECI_headerType *)txBufferPtr)->h3.bitfield.crc = 
            STECI_CalculateCRC ((txBufferPtr + STECI_HEADER_LENGTH), payloadLen);

        if (0 == curSrcPos)
        {
            ((STECI_headerType *)txBufferPtr)->h0.bitfield.firstFragment = 1;
        }
        else
        {
            ((STECI_headerType *)txBufferPtr)->h0.bitfield.firstFragment = 0;
        }

        if (remainingBytes > 0)
        {
            ((STECI_headerType *)txBufferPtr)->h0.bitfield.lastFragment = 0;
        }
        else
        {
            ((STECI_headerType *)txBufferPtr)->h0.bitfield.lastFragment = 1;
        }

        ((STECI_headerType *)txBufferPtr)->h3.bitfield.parity = STECI_CalculateParity (txBufferPtr, STECI_HEADER_LENGTH);

        // Increment source pointer
        curSrcPos += curSlotSize;

		// Send data frame
		res = STECI_SendDataSteciMode (deviceInfoPtr,
									   reqNextActiveStatusPtr, 
									   txBufferPtr,
									   (tU32)curSlotSize, 
									   rxDataBuffer,
                                       rxMaxSize, 
									   receivedBytesPtr, 
									   tmpTxDataPtr,
									   true, false, STECI_SEND_MODE_TX); // TODO: set to 'true, true' to close communication

        // Check errors (for host the device clash is not an error, is the device that shall re-transmit)
        if (STECI_STATUS_SUCCESS != res)
        {

			// Error we return reporting the problem
			break;
        }
    }

    // Return operation result
    return res;
}

static tBool STECI_CheckDeviceRequest (STECI_deviceInfoTy *deviceInfoPtr, STECI_statusTy *statusPtr)
{
	tBool res = false;
	tBool currentPinStatus;

	// Get REQ current status
	currentPinStatus = STECI_GetReqLevel (deviceInfoPtr);



	// If the REQ line toggled than we start to read data, device has something for us
	if (statusPtr->reqNextActiveStatus == currentPinStatus)
	{
		// Set next REQ active level
		//statusPtr->reqNextActiveStatus = STECI_ToggleReqLevel (statusPtr->reqNextActiveStatus);
		
		// +DEBUG
//		STECI_tracePrintError(TR_CLASS_STECI, "STECI_CheckDeviceRequest reqNextActiveStatus %d, currentPinStatus %d\n", 
//				statusPtr->reqNextActiveStatus, currentPinStatus);
		
		//-DEBUG

		res = true;
	}

	return res;
}

static tVoid STECI_ChangeStatus (STECI_statusTy *statusPtr, tU8 newStatus)
{
	// Debug and log message
	if (STECI_ERROR == newStatus)
	{		
#if (defined CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_ERRORS)
		/* keep under ifdef for STECI_Status2String */
		STECI_tracePrintError(TR_CLASS_STECI, "%d from status %s", statusPtr->error, STECI_Status2String(statusPtr->status));
#endif
	}
	else if (STECI_SPI_IDLE != newStatus && STECI_WAIT_REQ_PIN_TOGGLE != newStatus)
	{
#if (defined CONFIG_TRACE_CLASS_STECI) && (CONFIG_TRACE_CLASS_STECI >= TR_LEVEL_COMPONENT)
		/* keep under ifdef for STECI_Status2String */
		STECI_tracePrintComponent(TR_CLASS_STECI, "enter %s from status %s", STECI_Status2String(newStatus), STECI_Status2String(statusPtr->status));
#endif
	}

// + DEBUG
//	if (statusPtr->lastStatus != statusPtr->status)
//	STECI_tracePrintError(TR_CLASS_STECI, "enter %s from status %s", STECI_Status2String(newStatus), STECI_Status2String(statusPtr->status));
// - DEBUG
	// Set new status
	statusPtr->lastStatus = statusPtr->status;
	statusPtr->status = newStatus;
}

static tVoid STECI_ReturnToIdleStatus (STECI_deviceInfoTy *deviceInfoPtr, STECI_statusTy *steciStatusPtr, tBool toggleReq)
{
	// Set next REQ active level
	if (true == toggleReq)
	{
		steciStatusPtr->reqNextActiveStatus = STECI_ToggleReqLevel (steciStatusPtr->reqNextActiveStatus);
	}
	
	// Return to normal REQ check status 
	STECI_ChangeStatus (steciStatusPtr, STECI_WAIT_REQ_PIN_TOGGLE);

	// Reset CS to close communication
	STECI_ChipSelectDeassert (deviceInfoPtr);
}

static tVoid STECI_ExecuteSpecialCommands (STECI_deviceInfoTy *deviceInfoPtr, STECI_cmdModeEnumTy cmd, tU8 *paramPtr, tU16 numBytes, tU8 *dataPtr)
{
    tU16 res = STECI_COMM_STATUS_OK;
	tU8 dataBfr = 0;
	STECI_cmdModeEnumTy mode;
	DCOP_flashModeEnumTy flashOperation;
    DCOP_targetMemEnumTy targetMemory;
    DCOP_accessModeEnumTy accessMode;

	if (STECI_RESET_MODE == cmd)
	{
		// Reset DCOP device
		STECI_ResetDevice (deviceInfoPtr);

		// Set the payload for the response
		dataBfr = STECI_RESET_CMD;
	}
	else if (STECI_BOOT_MODE == cmd)
	{
		// Get the mode
		mode = (STECI_cmdModeEnumTy)*paramPtr;

		// Check if the mode is legal
		if (STECI_NORMAL_MODE == mode)
		{
			// Restart DCOP device in NORMAL mode
			if (STECI_SetDeviceBootModeAndRestart (deviceInfoPtr, false) != STECI_STATUS_SUCCESS)
			{
				res = STECI_COMM_STATUS_ERR_UNKNOWN;
			}
		}
		else if (STECI_BOOT_MODE == mode)
		{
			// Restart DCOP device in BOOT mode
			if (STECI_SetDeviceBootModeAndRestart (deviceInfoPtr, true) != STECI_STATUS_SUCCESS)
			{
				res = STECI_COMM_STATUS_ERR_UNKNOWN;
			}
		}

		// Set the payload for the response
		dataBfr = STECI_BOOT_CMD;
	}
	else if (STECI_FLASH_MODE == cmd)
	{
		flashOperation = (DCOP_flashModeEnumTy)*paramPtr;
        targetMemory = (DCOP_targetMemEnumTy)*(paramPtr + 1);
        accessMode = (DCOP_accessModeEnumTy)*(paramPtr + 2);

		res = DCOP_ExecuteFlashOperations (deviceInfoPtr, flashOperation, targetMemory, accessMode, numBytes, dataPtr);
        if (STECI_COMM_STATUS_OK != res)
        {
            res = STECI_COMM_STATUS_ERR_UNKNOWN;
        }
        // Set the payload for the response
        dataBfr = STECI_FLASH_CMD;
	}

	// Send information to upper layer of the reset:
	// - Device handler for the port
	// - Data buffer containing a single byte that is the command
	// - Len of the payload (1)
	/* STECI_MessageFeedback always returns success so it is safe to ignore its return value */
	(LINT_IGNORE_RET) STECI_MessageFeedback (deviceInfoPtr->deviceHandle, &dataBfr, 1, (STECI_commStatusEnumTy)res, STECI_FEEDBACK_SENDER_IS_DEVICE);
}

tS32 STECI_MessageFeedback (tVoid *deviceHandle, tU8 *dataPtr, tU32 numBytes, STECI_commStatusEnumTy errMsg, STECI_feedbackSenderEnumTy feedbackSender)
{
	tS32 res = STECI_STATUS_SUCCESS;
#if 0
	tU8 spareBytesRsp[4];

	// Fill spare bytes
	spareBytesRsp[0] = 0;
	spareBytesRsp[1] = (tU8)errMsg;
	spareBytesRsp[2] = 0;
	spareBytesRsp[3] = 0;

	// Send to TCP/IP the feedback that the command did not went through
	if (STECI_FEEDBACK_SENDER_IS_PROTOCOL == feedbackSender)
	{
		ForwardPacketToCtrlAppPort (deviceHandle, dataPtr, numBytes, PROTOCOL_LUN, INVALID_PORT_NUMBER, (tU8 *)&spareBytesRsp[0]);
	}
	else
	{
		ForwardPacketToCtrlAppPort (deviceHandle, dataPtr, numBytes, BROADCAST_LUN, INVALID_PORT_NUMBER, (tU8 *)&spareBytesRsp[0]);
	}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
    tU8 *resp;

    resp = (tU8 *)OSAL_pvMemoryAllocate(8 + numBytes);
	if (resp == NULL)
	{
		res = STECI_STATUS_ERROR;
	}
	else
	{
		if (STECI_FEEDBACK_SENDER_IS_PROTOCOL == feedbackSender)
		{
			resp[0] = PROTOCOL_LAYER_MESSAGE_SYNC_BYTE;
			resp[1] = PROTOCOL_LUN;
			// TODO: wrong response value for H0 .. H3
			resp[2] = (tU8)0;                        // H0
			resp[3] = (tU8)0;                        // H1
			resp[4] = (tU8)0;                        // H2
			resp[5] = (tU8)0;                        // H3
			resp[6] = (tU8)((numBytes >> 8) & 0xFF); // LEN MSB
			resp[7] = (tU8)(numBytes & 0xFF);        // LEN LSB
		}
		else
		{
			resp[0] = TARGET_MESSAGE_SYNC_BYTE;
			resp[1] = BROADCAST_LUN;
			resp[2] = (tU8)0;                        // H0
			resp[3] = (tU8)errMsg;                   // H1
			resp[4] = (tU8)0;                        // H2
			resp[5] = (tU8)0;                        // H3
			resp[6] = (tU8)((numBytes >> 8) & 0xFF); // LEN MSB
			resp[7] = (tU8)(numBytes & 0xFF);        // LEN LSB
		}
		OSAL_pvMemoryCopy((tPVoid)(&resp[8]), (tPCVoid)dataPtr, numBytes);
		STECI_fifoPush(resp, (8 + numBytes), resp[1]);
		OSAL_vMemoryFree((tVoid *)resp);
	}
#endif

	return res;
}

static tS32 STECI_MessageDecode (tVoid *deviceHandle, tU8 *dataPtr, tU32 numBytes, STECI_messageType *messageDataPtr)
{
	tS32 res;
	tU32 payloadLen;
#if 0
	tU8 dataType;
	tBool itIsRawBinary = false;
	tBool itIsPcmAudio = false;
	tBool forwardToDataPortConsumer = false;
#endif

	// Decode the data
	OSAL_pvMemoryCopy ((tVoid *)&messageDataPtr->msg.payload[0], (tPCVoid)dataPtr, numBytes);

	// Check crc
	payloadLen = (((tU32)messageDataPtr->msg.header.h0.bitfield.lenMsb << 8 & (tU32)0x0F00) |
		(tU32)messageDataPtr->msg.header.lenLsb) + 1;

	res = STECI_ValidateCRC (messageDataPtr->msg.header.h3.bitfield.crc, &messageDataPtr->msg.payload[0], payloadLen);

	if (STECI_STATUS_SUCCESS != res)
	{
		// Return the error
		return STECI_STATUS_DATA_CRC_ERROR;
	}

	// Save the data to a temporary buffer in order to fill with all message pieces
	res = STECI_StorePartialMessage (&messageDataPtr->messageStorage, messageDataPtr->msgContentPtr,
		payloadLen, messageDataPtr->msg.header.lun, messageDataPtr->msg.header.h0.bitfield.firstFragment);

	// If we got success then check if the message has to be sent
	if (STECI_STATUS_SUCCESS == res)
	{
		// Check FIRST/LAST packet indications and if it last packet send to TCP/IP
		if (SET == messageDataPtr->msg.header.h0.bitfield.lastFragment)
		{
#if 0
			// If the data is coming from the data channel then extract data type and if it is 
			// binary or ASCII information
			if (DEVICE_LUN_IS_DATA_CHANNEL == messageDataPtr->messageStorage.lun)
			{
				// Get the data type
				dataType = messageDataPtr->messageStorage.msgStorageContentPtr[4];

				// Get the infromation if it is binary data
				if (0x40 == (messageDataPtr->messageStorage.msgStorageContentPtr[1] & (tU8)0xE0))
				{
					itIsRawBinary = true;
				}
				else
				{
					itIsRawBinary = false;
				}

				// Get the information if it is PCM audio
				if (0xE0 == (messageDataPtr->messageStorage.msgStorageContentPtr[1] & (tU8)0xE0))
				{
					itIsPcmAudio = true;
				}
				else
				{
					itIsPcmAudio = false;
				}

				// Get the information if it is COMPRESSED AUDIO
				if (0xC0 == (messageDataPtr->messageStorage.msgStorageContentPtr[1] & (tU8)0xE0))
				{
					itIsCompressedAudio = true;
				}
				else
				{
					itIsCompressedAudio = false;
				}

				// Depending on the LUN we use a different dispatch function so the TCP/IP interface can understand
				// what kind of data is coming. We send to the data channel only AUDIO BINARY information, not ASCII, for this
				// one we prefer to use normal channel			 
				if (true == itIsPcmAudio)
				{
					// Send data channel data to TCP/IP as PCM AUDIO frame
					forwardToDataPortConsumer = true;
				}
				else if ((true == itIsRawBinary) && (0x40 == dataType))
				{
					// Send data channel data to TCP/IP as BINARY DRM OBJ frame
					forwardToDataPortConsumer = true;
				}
				else if ((true == itIsRawBinary) && (0x46 == dataType))
				{
					// Send data channel data to TCP/IP ase BINARY DRM MDI frame
					forwardToDataPortConsumer = true;
				}
				else if (true == itIsCompressedAudio)
				{
					// Send data channel data to TCP/IP as COMPRESSED AUDIO frame
					forwardToDataPortConsumer = true;
				}
			}

			// Forward the data in the proper way depending on previous checks
			if (true == forwardToDataPortConsumer)
			{
				// Send data as DATA to the TCP/IP (depending on user parameters could be send to file and/or TCP/IP)
				ForwardDataChannelPacketToCtrlAppPort (deviceHandle, messageDataPtr->messageStorage.msgStorageContentPtr,
					messageDataPtr->messageStorage.totalLen, messageDataPtr->messageStorage.lun,
					INVALID_PORT_NUMBER, (tU8 *)NULL, dataType);
			}
			else
			{
				// Send data to TCP/IP
				ForwardPacketToCtrlAppPort (deviceHandle, messageDataPtr->messageStorage.msgStorageContentPtr,
					messageDataPtr->messageStorage.totalLen, messageDataPtr->messageStorage.lun,
					INVALID_PORT_NUMBER, (tU8 *)NULL);
			}
#elif defined (CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
            STECI_fifoPush(messageDataPtr->messageStorage.msgStorageContentPtr, messageDataPtr->messageStorage.totalLen, messageDataPtr->messageStorage.lun);
#endif // 0
			// Set to used the message
			if (NULL != messageDataPtr->messageStorage.msgStorageContentPtr)
			{
				free (messageDataPtr->messageStorage.msgStorageContentPtr);
			}

			messageDataPtr->messageStorage.msgStorageContentPtr = NULL;
			messageDataPtr->messageStorage.totalLen = 0;
			messageDataPtr->messageStorage.lun = STECI_INVALID_LUN;
		}
		else
		{
			// In this case we need to indicate that we expect to receive another
			// piece belonging to this message. It is possible even to check for data sending
			res = STECI_STATUS_INTERMEDIATE_FRAME;
		}
	}
	else
	{
		// Set to used the message
		if (NULL != messageDataPtr->messageStorage.msgStorageContentPtr)
		{
			free (messageDataPtr->messageStorage.msgStorageContentPtr);

			messageDataPtr->messageStorage.msgStorageContentPtr = NULL;
		}

		messageDataPtr->totalLen = 0;
	}

	return res;
}

static tS32 STECI_HeaderDecode (tU8 *dataPtr, tS32 *dataToSend, STECI_messageType *messageDataPtr)
{
	tS32 tmpLen;
	tU8 tmpByte;
	STECI_headerType *tmpHeader = (STECI_headerType *)dataPtr;

	// Pre-set the number of bytes to receive to 0 in case of errors
	*dataToSend = 0;

	// Check the special condition of zero buffer received
	if (0 == tmpHeader->lenLsb && 0 == tmpHeader->h0.bitfield.lenMsb)
	{
		// We have a situation where both are thinking to be the receiver, it is 
		// not an error, just report 0 and return in IDLE state
		return STECI_STATUS_DATA_NOT_PRESENT;
	}

	// Control the first byte of the header
	messageDataPtr->msg.header.h0.h0val = *(dataPtr);

	// Get LEN
	messageDataPtr->msg.header.lenLsb = *(dataPtr + 1);

	// Get LUN
	messageDataPtr->msg.header.lun = *(dataPtr + 2);

	// Calculate real paylaod length
	messageDataPtr->totalLen = (((tS32)messageDataPtr->msg.header.h0.bitfield.lenMsb << 8 & (tS32)0x0F00) |
		(tS32)messageDataPtr->msg.header.lenLsb) + 1;

	// Get the parity and crc
	messageDataPtr->msg.header.h3.h3val = *(dataPtr + 3);

	// Save byte before modify it
	tmpByte = *(dataPtr + 3);

	// Clear parity inside original buffer for the check
	*(dataPtr + 3) &= (tU8)0x7F;

	// Check parity
	if (messageDataPtr->msg.header.h3.bitfield.parity != STECI_CalculateParity (dataPtr, STECI_HEADER_LENGTH))
	{
		// Restore saved byte before
		*(dataPtr + 3) = tmpByte;

		return STECI_STATUS_HEADER_PARITY_ERROR;
	}

	// Restore saved byte before
	*(dataPtr + 3) = tmpByte;

	// Now check the re-transmission, if it is set we are in presence of an error
	if (STECI_BIT_SET == messageDataPtr->msg.header.h0.bitfield.reTransmission)
	{
		return STECI_STATUS_HEADER_RE_TX_FLAG;
	}

	// Check LUN (0 is not valid)
	if (STECI_INVALID_LUN == messageDataPtr->msg.header.lun)
	{
		// Invalid LUN detected. This seems to be an empty header, leading to 
		// think that the device is receiving data. Best thing to do is to return into idle status
		return STECI_STATUS_HOST_DEVICE_CLASH;
	}

	// Now set the bytes amount that we would like to receive                        
	tmpLen = messageDataPtr->totalLen;

	// Fill with zeroes to trasmit data as multiple of four
	while (0 != (tmpLen % STECI_PAYLOAD_MODULO)) // % operator instead of Y&(X-1) as the payload modulo can be != ^2
	{
		tmpLen++;
	}

	// Report the wanted number of bytes
	*dataToSend = tmpLen;

	return STECI_STATUS_SUCCESS;
}

static tS32 STECI_StorePartialMessage (STECI_messageStorageTy *messageStoragePtr, tU8 *msgContentPtr, tU32 messageLen, tU8 newLun, tBool firstFragment)
{
	tU8 *tmpPtr;

	// If the lun is equal to the one already stored or this is the first message piece
	// we can go further, if not, it is an error
	if (newLun != messageStoragePtr->lun && STECI_INVALID_LUN != messageStoragePtr->lun && false == firstFragment)
	{
		return STECI_STATUS_ERROR;
	}
	else
	{
		// If we have a request for 1st fragment whereas prior fragment decoding is on-going
		// free prior fragment pointer
		// we know if a prior fragment reconstituion is on-going because msgStorageContentPtr should is not null
		//
		if ((true == firstFragment) && (NULL != messageStoragePtr->msgStorageContentPtr))
		{
			OSAL_vMemoryFree(messageStoragePtr->msgStorageContentPtr);
			messageStoragePtr->msgStorageContentPtr = NULL;
		}
		if (NULL == messageStoragePtr->msgStorageContentPtr)
		{
			messageStoragePtr->msgStorageContentPtr = (tU8 *)OSAL_pvMemoryAllocate (messageLen);

			if (NULL == messageStoragePtr->msgStorageContentPtr)
			{
				return STECI_STATUS_ERROR;
			}
			else
			{
				OSAL_pvMemoryCopy ((tVoid *)messageStoragePtr->msgStorageContentPtr, (tVoid *)msgContentPtr, messageLen);

				messageStoragePtr->totalLen = messageLen;
				messageStoragePtr->lun = newLun;
			}
		}
		else
		{
			tmpPtr = (tU8 *)OSAL_pvMemoryReAllocate ((tVoid *)messageStoragePtr->msgStorageContentPtr, (messageStoragePtr->totalLen + messageLen));

			if (NULL == tmpPtr)
			{
				return STECI_STATUS_ERROR;
			}
			else
			{
				messageStoragePtr->msgStorageContentPtr = tmpPtr;

				OSAL_pvMemoryCopy ((tVoid *)(messageStoragePtr->msgStorageContentPtr + messageStoragePtr->totalLen),
					(tVoid *)msgContentPtr, messageLen);

				messageStoragePtr->totalLen += messageLen;
				messageStoragePtr->lun = newLun;
			}
		}
	}

	return STECI_STATUS_SUCCESS;
}

#ifdef __cplusplus
}
#endif

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_MDR
// End of file

