//!
//!  \file    steci_uart_protocol.cpp
//!  \brief   <i><b> STECI UART protocol </b></i>
//!  \details This module implements the "Logical Messaging Layer" of the STECI UART specifications.
//!  \author  Alberto Saviotti
//!

#include "target_config.h"

#ifdef CONFIG_APP_RADIO_IF
#include "target_config_radio_if.h"
#endif // CONFIG_APP_RADIO_IF

///
// This define shall be defined just after inlcusion of target config because 
// it is used to correctly setup printf level
///
#define CONFIG_TRACE_CLASS_MODULE       CONFIG_TRACE_CLASS_STECI_UART

#if defined (CONFIG_BUILD_DRIVER) && defined (CONFIG_COMM_STECI_UART)
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	#include <windows.h>
#elif defined (CONFIG_HOST_OS_LINUX)
	#include <pthread.h> // for osal.h
#endif
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "types.h"
#include "utility.h"

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	#include "common_helpers.h"
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	#include "osal.h"
#endif

#if defined(CONFIG_HOST_OS_LINUX_EMBEDDED) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	#include "etaldefs.h"
	#include "ipfcomm.h"
	#include "DAB_Protocol.h"
#endif
#include "TcpIpProtocol.h"
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	#include "interlayer_protocol.h"
#endif

#include "connection_modes.h"
#include "steci_defines.h"
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
#include "steci_lld.h"
#endif
#include "steci_protocol.h"
#include "steci_crc.h"

#if defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	#include "cmost_helpers.h"
#ifdef CONFIG_APP_RADIO_IF
	#include "rif_msg_queue.h"
	#include "rif_protocol_router.h"
	#include "radio_if_util.h"
#endif // CONFIG_APP_RADIO_IF
#if defined(CONFIG_HOST_OS_LINUX_EMBEDDED)
	#include "HDRADIO_Protocol.h"
#endif // CONFIG_HOST_OS_LINUX_EMBEDDED
#endif

#include "hdr_boot.h"

#include "mcp_protocol.h"

#include "uart_mngm.h"

//#include "mcp_lld.h"
#include "steci_uart_protocol.h"
#include "steci_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///
#define STECI_UART_ToTxBufferSem_STR    "STeciUTxSem"
#define STECI_UART_NB_MAX_LUN_PORT      5

#define STECI_UART_STR_PREFIX_RX    "STECI UART Rx: "
#define STECI_UART_STR_PREFIX_TX    "STECI UART Tx: "

///
// Type definitions
///
typedef enum
{
	STECI_UART_PROTOCOL_IDLE        = 0,
	STECI_UART_PROTOCOL_RX          = 1,
	STECI_UART_PROTOCOL_TX          = 3,
	STECI_UART_PROTOCOL_ERROR       = 0xFF
} STECI_UART_statusEnumTy;

typedef struct
{
	tVoid *deviceHandle;
	tVoid *deviceSecondaryHandle;

	tU8 deviceMode;
} STECI_UART_deviceInfoTy;

typedef struct
{
	tU32 port;
	tU8  lun;
} STECI_UART_Lun_Port;

typedef struct
{
	tU8 nb_lun_port;
	STECI_UART_Lun_Port lun_port[STECI_UART_NB_MAX_LUN_PORT];
} STECI_UART_map_Lun_Port;

///
// Local variables
///
static OSAL_tSemHandle      STECI_UART_toTxBufferSem = 0;
static STECI_UART_map_Lun_Port STECI_UART_LunPort = {0, {{0, STECI_INVALID_LUN}, {0, STECI_INVALID_LUN}, {0, STECI_INVALID_LUN},
	{0, STECI_INVALID_LUN}, {0, STECI_INVALID_LUN}}};

///
// Local function prototypes
///
static tVoid STECI_UART_ChangeStatus (STECI_UART_statusEnumTy *statusPtr, STECI_UART_statusEnumTy newStatus);
static tS32 STECI_UART_MessageSend(STECI_UART_deviceInfoTy *deviceInfoPtr, STECI_UART_txRxInfo *infoPtr, tU8 *txBufferPtr);
static void STECI_UART_setPortFromLun(tU8 lun, tU32 port);
static tS32 STECI_UART_isValidHeaderPayload(STECI_UART_txRxInfo *paramsPtr);
static tS32 STECI_UART_MessageDecode(STECI_UART_paramsTy *paramsPtr);
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
static tU8 STECI_UART_Lun2ProtocolLayerLun(tU8 LunSteciUart);
static tU32 STECI_UART_Lun2ProtocolLayerPort(tU8 lun);
#endif // #if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)

///
// Global function definitions
///
tS32 STECI_UART_init(tVoid)
{
	// create tx buffer semaphore
	if (OSAL_s32SemaphoreCreate(STECI_UART_ToTxBufferSem_STR, &STECI_UART_toTxBufferSem, 1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

tS32 STECI_UART_SendMessageStart(tVoid *memoryPtr, tU8 LunId, tU8 *DataToSend,
                           tU8 specific0, tU8 specific1, tU8 specific2, tU8 specific3, tU16 BytesNumber,
                           tU32 dataSourceAndDestinationId, DEV_protocolEnumTy  devProtocol)
{
	tS32 ret;
	STECI_UART_paramsTy *dataInfoPtr = (STECI_UART_paramsTy *)memoryPtr;

	// protect against concurrent accesses to STECI UART Tx since there is only one command buffer
	if ((ret = OSAL_s32SemaphoreWait(STECI_UART_toTxBufferSem, OSAL_C_TIMEOUT_FOREVER)) != OSAL_OK)
	{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART STECI_UART_toTxBufferSem (%d: %s)", ret, OSAL_coszErrorText(ret));
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
	}

#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
	OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "STECI UART message received %x %d %d %d", specific0, BytesNumber, dataSourceAndDestinationId, devProtocol);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

	/* protection against bufer overflow */
	if (BytesNumber > STECI_MAX_MESSAGE_LEN)
	{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART message too long %d > %d", BytesNumber, STECI_MAX_MESSAGE_LEN);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		ASSERT_ON_DEBUGGING(0);
		return -1;
	}

	// copy source and destination
	dataInfoPtr->sourceAndDestinationIdentifier = dataSourceAndDestinationId;

	switch(devProtocol)
	{
		case DEV_PROTOCOL_CMOST:
			// Check modes
			if ((CMOST_CMDMODE_CMD == specific0) || (CMOST_CMDMODE_RD == specific0) || (CMOST_CMDMODE_WR == specific0) ||
				(CMOST_CMDMODE_RD_DMA == specific0) || (CMOST_CMDMODE_WR_DMA == specific0) || (CMOST_CMDMODE_CMD_GEN == specific0))
			{
				dataInfoPtr->cmdMode = STECI_UART_NORMAL_MODE;

				OSAL_pvMemoryCopy((tPVoid)dataInfoPtr->txRxData.DataBuffer, (tPCVoid)DataToSend, BytesNumber);

				dataInfoPtr->sourceAndDestinationIdentifier = dataSourceAndDestinationId;

				dataInfoPtr->txRxData.TotalBytesNum = BytesNumber;
				dataInfoPtr->txRxData.RemainingBytesNum = BytesNumber;
				dataInfoPtr->txRxData.SnInfo = 0;
				dataInfoPtr->txRxData.retry = false;
				dataInfoPtr->txRxData.numberOfRetry = 0;
				dataInfoPtr->txRxData.dataToBeTx = false;
				if ((LunId == BROADCAST_LUN) || (LunId < 0x10))
				{
					// Map I2C address STECI_UART_CMOST1_I2C_ADDR or SPI chip select STECI_UART_CMOST1_SPI_ADDR to CMOST(0)
					// Map I2C address STECI_UART_CMOST2_I2C_ADDR or SPI chip select STECI_UART_CMOST2_SPI_ADDR to CMOST(1)
					if ((specific2 == STECI_UART_CMOST1_SPI_ADDR) || (specific2 == STECI_UART_CMOST1_I2C_ADDR))
					{
						dataInfoPtr->txRxData.Lun = STECI_UART_CMOST_LUN | (LunId & 0x0F);
						STECI_UART_setPortFromLun(dataInfoPtr->txRxData.Lun, dataSourceAndDestinationId);

						dataInfoPtr->txRxData.dataToBeTx = true;
					}
					else if ((specific2 == STECI_UART_CMOST2_SPI_ADDR) || (specific2 == STECI_UART_CMOST2_I2C_ADDR))
					{
						dataInfoPtr->txRxData.Lun = STECI_UART_CMOST2_LUN | (LunId & 0x0F);
						STECI_UART_setPortFromLun(dataInfoPtr->txRxData.Lun, dataSourceAndDestinationId);
		
						dataInfoPtr->txRxData.dataToBeTx = true;
					}
				}
				if (dataInfoPtr->txRxData.dataToBeTx == false )
				{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART invalid CMOST LUN 0x%x or address H2 0x%x", LunId, specific2);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

					// unprotect accesses to STECI UART Tx
					OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
				}
			}
			else
			{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART CMOST LUN 0x%x command type %d not handled", LunId, specific0);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			}
			break;

		case DEV_PROTOCOL_STECI:
			// Check modes
			if (STECI_NORMAL_CMD == specific0)
			{
				dataInfoPtr->cmdMode = STECI_UART_NORMAL_MODE;

				OSAL_pvMemoryCopy((tPVoid)dataInfoPtr->txRxData.DataBuffer, (tPCVoid)DataToSend, BytesNumber);

				dataInfoPtr->sourceAndDestinationIdentifier = dataSourceAndDestinationId;

				dataInfoPtr->txRxData.TotalBytesNum = BytesNumber;
				dataInfoPtr->txRxData.RemainingBytesNum = BytesNumber;
				dataInfoPtr->txRxData.SnInfo = 0;
				dataInfoPtr->txRxData.retry = false;
				dataInfoPtr->txRxData.numberOfRetry = 0;
				if ((LunId >= 0x01) && (LunId <= 0x3F))
				{
					dataInfoPtr->txRxData.Lun = (LunId & 0x3F);
					STECI_UART_setPortFromLun(dataInfoPtr->txRxData.Lun, dataSourceAndDestinationId);

					dataInfoPtr->txRxData.dataToBeTx = true;
				}
				else
				{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART invalid STECI LUN 0x%x", LunId);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

					// unprotect accesses to STECI UART Tx
					OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
				}
			}
			else
			{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART STECI LUN 0x%x command type %d not handled", LunId, specific0);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			}
			break;

		case DEV_PROTOCOL_HDR:
			// Check modes
			if (HDR_NORMAL_CMD == specific0)
			{
				dataInfoPtr->cmdMode = STECI_UART_NORMAL_MODE;

				/* copy HDR instance */
				dataInfoPtr->txRxData.DataBuffer[0] = specific1;
				/* copy HDR payload */
				OSAL_pvMemoryCopy((tPVoid)(&(dataInfoPtr->txRxData.DataBuffer[1])), (tPCVoid)DataToSend, BytesNumber);

				dataInfoPtr->sourceAndDestinationIdentifier = dataSourceAndDestinationId;

				dataInfoPtr->txRxData.TotalBytesNum = (BytesNumber + 1);
				dataInfoPtr->txRxData.RemainingBytesNum = (BytesNumber + 1);
				dataInfoPtr->txRxData.SnInfo = 0;
				dataInfoPtr->txRxData.retry = false;
				dataInfoPtr->txRxData.numberOfRetry = 0;
				if (LunId == BROADCAST_LUN)
				{
					dataInfoPtr->txRxData.Lun = STECI_UART_DCOP_HD_LUN | (LunId & 0x0F);
					STECI_UART_setPortFromLun(dataInfoPtr->txRxData.Lun, dataSourceAndDestinationId);

					dataInfoPtr->txRxData.dataToBeTx = true;
				}
				else
				{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART invalid HDR LUN 0x%x", LunId);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

					// unprotect accesses to STECI UART Tx
					OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
				}
			}
			else
			{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART HDR LUN 0x%x command type %d not handled", LunId, specific0);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			}
			break;

		case DEV_PROTOCOL_MCP:
			// Check modes
			if (STECI_NORMAL_CMD == specific0)
			{
				dataInfoPtr->cmdMode = STECI_UART_NORMAL_MODE;

				OSAL_pvMemoryCopy((tPVoid)dataInfoPtr->txRxData.DataBuffer, (tPCVoid)DataToSend, BytesNumber);

				dataInfoPtr->sourceAndDestinationIdentifier = dataSourceAndDestinationId;

				dataInfoPtr->txRxData.TotalBytesNum = BytesNumber;
				dataInfoPtr->txRxData.RemainingBytesNum = BytesNumber;
				dataInfoPtr->txRxData.SnInfo = 0;
				dataInfoPtr->txRxData.retry = false;
				dataInfoPtr->txRxData.numberOfRetry = 0;
				if ((LunId == BROADCAST_LUN) || (LunId == STECI_UART_MCP_LUN))
				{
					if (LunId == BROADCAST_LUN)
					{
						dataInfoPtr->txRxData.Lun = STECI_UART_MCP_LUN | 0x0F;
					}
					else
					{
						dataInfoPtr->txRxData.Lun = LunId;
					}
					STECI_UART_setPortFromLun(dataInfoPtr->txRxData.Lun, dataSourceAndDestinationId);

					dataInfoPtr->txRxData.dataToBeTx = true;
				}
				else
				{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART invalid MCP LUN 0x%x", LunId);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

					// unprotect accesses to STECI UART Tx
					OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
				}
			}
			else
			{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART MCP LUN 0x%x command type %d not handled", LunId, specific0);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			}
			break;

		default:
			// unprotect accesses to STECI UART Tx
			OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
			break;
	}

	return 0;
}

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
tUInt __stdcall STECI_UART_ProtocolHandle(tVoid *pParams)
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
tVoid STECI_UART_ProtocolHandle(tVoid *pParams)
#endif
{
	STECI_UART_statusEnumTy steciUartStatus;
	STECI_UART_paramsTy *paramsPtr = (STECI_UART_paramsTy *)pParams;
	STECI_UART_deviceInfoTy deviceInfo;
	tS32 ret, ret2;

	// Init the device handler used by this protocol
	deviceInfo.deviceHandle = paramsPtr->devHandle;
	deviceInfo.deviceMode = paramsPtr->deviceMode;

	// Init crc
	STECI_CrcInit();

	steciUartStatus = STECI_UART_PROTOCOL_IDLE;

	while (1)
	{
		// Handle current protocol status
		switch (steciUartStatus)
		{
			case STECI_UART_PROTOCOL_IDLE:
				if (true == paramsPtr->txRxData.dataToBeTx)
				{
					// Protocol Layer message received
					// Set status
					STECI_UART_ChangeStatus(&steciUartStatus, STECI_UART_PROTOCOL_TX);
				}
				else if (true == paramsPtr->txRxData.dataToBeRx)
				{
					// STECI UART frame received
					// Set status
					STECI_UART_ChangeStatus(&steciUartStatus, STECI_UART_PROTOCOL_RX);
				}
				else
				{
					// We will rest if we do not have something to do already
					OSAL_s32ThreadWait(1);
				}
				break;

			case STECI_UART_PROTOCOL_TX:
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
				OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "STECI UART Tx message processing");
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

				// Add STECI header and send message
				if (STECI_UART_MessageSend(&deviceInfo, &(paramsPtr->txRxData), paramsPtr->outDataBuffer) == STECI_STATUS_SUCCESS)
				{
					// No more data to send
					paramsPtr->txRxData.dataToBeTx = false;

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
#if 0
					// Send data to TCP/IP (echoing)
					ForwardPacketToCtrlAppPort(deviceInfo.deviceHandle, paramsPtr->txRxData.DataBuffer,
					                            paramsPtr->txRxData.TotalBytesNum, paramsPtr->txRxData.Lun,
												paramsPtr->sourceAndDestinationIdentifier, (tU8 *)NULL);
#endif
#endif

					// Set status
					STECI_UART_ChangeStatus(&steciUartStatus, STECI_UART_PROTOCOL_IDLE);

					// unprotect accesses to STECI UART Tx
					OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
				}
				else
				{
					// error retry to send same message
					paramsPtr->txRxData.numberOfRetry++;
					if (paramsPtr->txRxData.numberOfRetry < STECI_DATA_SEND_RETRY_MAX_NUM)
					{
						paramsPtr->txRxData.retry = true;
					}
					else
					{
						// No more data to send
						paramsPtr->txRxData.dataToBeTx = false;

#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART retry failed for Lun: 0x%x", paramsPtr->txRxData.Lun);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
#if 0
						// Send data to TCP/IP (echoing)
						ForwardPacketToCtrlAppPort(deviceInfo.deviceHandle, paramsPtr->txRxData.DataBuffer,
						                            paramsPtr->txRxData.TotalBytesNum, paramsPtr->txRxData.Lun,
													paramsPtr->sourceAndDestinationIdentifier, (tU8 *)NULL);
#endif
#endif

						// Set status
						STECI_UART_ChangeStatus(&steciUartStatus, STECI_UART_PROTOCOL_IDLE);

						// unprotect accesses to STECI UART Tx
						OSAL_s32SemaphorePost(STECI_UART_toTxBufferSem);
					}
				}
				break;

			case STECI_UART_PROTOCOL_RX:
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
				OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "STECI UART Rx message processing (%d bytes)", paramsPtr->txRxData.TotalBytesNumRxUart);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
				UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, paramsPtr->txRxData.DataBufferRx.DataBufferRxUart, paramsPtr->txRxData.TotalBytesNumRxUart, STECI_UART_STR_PREFIX_RX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

				// check STECI UART Rx header and payload
				ret = STECI_UART_isValidHeaderPayload(&(paramsPtr->txRxData));

				// Check if retry bit is requested by receiver
				if ((ret != STECI_STATUS_HEADER_PARITY_ERROR) && (paramsPtr->txRxData.TotalBytesNumRxUart >= STECI_HEADER_LENGTH) &&
					(paramsPtr->txRxData.DataBufferRx.DataBufferRxUartSteciHeader->h0.bitfield.reTransmission == 1))
				{
					// protect against concurrent accesses to STECI UART Tx since there is only one command buffer
					if ((ret2 = OSAL_s32SemaphoreWait(STECI_UART_toTxBufferSem, OSAL_C_TIMEOUT_FOREVER)) == OSAL_OK)
					{
						// TODO may be too late to update this here because message can be already overwritten by STECI_UART_SendMessageStart
						paramsPtr->txRxData.retry = 1;
						paramsPtr->txRxData.dataToBeTx = true;
					}
					else
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART STECI_UART_toTxBufferSem %d: %s)", ret2, OSAL_coszErrorText(ret2));
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					}
				}

				if (ret == STECI_STATUS_SUCCESS)
				{
					// decode and send message
					if ((ret = STECI_UART_MessageDecode(paramsPtr)) != STECI_STATUS_SUCCESS)
					{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
						OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message decode and send error %d", ret);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					}
				}
				else
				{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
					OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message invalid header payload %d", ret);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				}

				// No more data to receive
				paramsPtr->txRxData.TotalBytesNumRxUart = 0;
				paramsPtr->txRxData.dataToBeRx = false;

				// Set status
				STECI_UART_ChangeStatus(&steciUartStatus, STECI_UART_PROTOCOL_IDLE);
				break;

			default:
				break;
		}
	}

	// delete semaphore
	if (OSAL_s32SemaphoreDelete(STECI_UART_ToTxBufferSem_STR) != OSAL_OK)
	{
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
		return -1;
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
		return;
#endif
	}

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	return 0;
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	return;
#endif
}

///
// Local function definitions
///
static tVoid STECI_UART_ChangeStatus (STECI_UART_statusEnumTy *statusPtr, STECI_UART_statusEnumTy newStatus)
{
	// Chanhe status
	*statusPtr = newStatus;
}

/***************************
 *
 * STECI_UART_MessageSend
 *
 **************************/
/*!
 * \brief		Add STECI header to payload and send STECI UART frame
 * \param[in]	deviceInfoPtr :  STECI device information pointer
 * \param[in]	infoPtr :  STECI UART txRx information pointer
 * \param[in]	txBufferPtr :  Tx buffer pointer
 * \return		STECI_STATUS_SUCCESS, STECI_STATUS_ERROR
 * \callgraph
 * \callergraph
 */
static tS32 STECI_UART_MessageSend(STECI_UART_deviceInfoTy *deviceInfoPtr, STECI_UART_txRxInfo *infoPtr, tU8 *txBufferPtr)
{
	tS32 remainingBytes;
	tS32 curSrcPos;
	tS32 curSlotSize, payloadLen;
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
			payloadLen = remainingBytes;
			remainingBytes = 0;
		}
		else
		{
			payloadLen = STECI_MAX_PAYLOAD_LENGTH;
			remainingBytes -= STECI_MAX_PAYLOAD_LENGTH;
		}
		curSlotSize = STECI_HEADER_LENGTH + payloadLen;

		// Copy payload
		OSAL_pvMemoryCopy((void *)(txBufferPtr + STECI_HEADER_LENGTH), (void *)(&(infoPtr->DataBuffer[curSrcPos])), payloadLen);

		// Set header control byte reTransmission
		if (false == infoPtr->retry)
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.reTransmission = 0;
		}
		else
		{
			// MDR3 interprets this bit as request to retransmit
			((STECI_headerType *)txBufferPtr)->h0.bitfield.reTransmission = 1;
		}

		// Set header control byte noPayload
		if (STECI_HEADER_LENGTH == curSlotSize)
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.noPayload = 1;
		}
		else
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.noPayload = 0;
		}

		// Set header payload length
		((STECI_headerType *)txBufferPtr)->lenLsb = (tU8)((payloadLen - 1) & (tU16)0x00FF);
		((STECI_headerType *)txBufferPtr)->h0.bitfield.lenMsb = (tU8)(((payloadLen - 1) >> 8) & (tU16)0x00F);

		// Set header LUN
		((STECI_headerType *)txBufferPtr)->lun = infoPtr->Lun;

		// Set header checksum CRC7
		((STECI_headerType *)txBufferPtr)->h3.bitfield.crc = STECI_CalculateCRC((txBufferPtr + STECI_HEADER_LENGTH), payloadLen);

		// Set header control byte firstFragment
		if (0 == curSrcPos)
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.firstFragment = 1;
		}
		else
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.firstFragment = 0;
		}

		// Set header control byte lastFragment
		if (remainingBytes > 0)
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.lastFragment = 0;
		}
		else
		{
			((STECI_headerType *)txBufferPtr)->h0.bitfield.lastFragment = 1;
		}
		// Set header checksum byte CRC1 parity
		((STECI_headerType *)txBufferPtr)->h3.bitfield.parity = 0;
		((STECI_headerType *)txBufferPtr)->h3.bitfield.parity = STECI_CalculateParity(txBufferPtr, STECI_HEADER_LENGTH);

#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
		OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "STECI_UART_MessageSend(%d bytes):", curSlotSize);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
		UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, txBufferPtr, curSlotSize, STECI_UART_STR_PREFIX_TX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

		// Send data frame
		res = MCP_MessageSend(deviceInfoPtr->deviceHandle, txBufferPtr, (tU16)curSlotSize);

		// Check errors (for host the device clash is not an error, is the device that shall re-transmit)
		if (STECI_STATUS_SUCCESS != res)
		{
			// Error we return reporting the problem
			break;
		}

		// Increment source pointer
		curSrcPos += payloadLen;
	
	}

	// increment retry counter
	if (true == infoPtr->retry)
	{
		infoPtr->numberOfRetry++;
	}

	// Return operation result
	return res;
}

/***************************
 *
 * STECI_UART_isValidHeaderPayload
 *
 **************************/
/*!
 * \brief		Check if STECI UART header is valid
 * \param[in,out]	paramsPtr :  STECI UART txRx information pointer
 * \return		STECI_STATUS_SUCCESS, STECI_STATUS_ERROR
 * \callgraph
 * \callergraph
 */
static tS32 STECI_UART_isValidHeaderPayload(STECI_UART_txRxInfo *paramsPtr)
{
	STECI_headerType *steciHeaderPtr = (STECI_headerType *)paramsPtr->DataBufferRx.DataBufferRxUart;
	tU8 parity_bit;

	// check STECI UART message total length
	if (paramsPtr->TotalBytesNumRxUart < STECI_HEADER_LENGTH)
	{
		// wrong length received
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong total length: %d",
				paramsPtr->TotalBytesNumRxUart);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		return STECI_STATUS_DATA_NOT_PRESENT;
	}

	// save STECI UART header parity bit
	parity_bit = steciHeaderPtr->h3.bitfield.parity;

	// check STECI UART header parity bit
	steciHeaderPtr->h3.bitfield.parity = 0;
	if (parity_bit != STECI_CalculateParity(paramsPtr->DataBufferRx.DataBufferRxUart, STECI_HEADER_LENGTH))
	{
		// wrong STECI UART header parity received
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong parity bit: %d",
			steciHeaderPtr->h3.bitfield.parity);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)

		// restore STECI UART header parity bit
		steciHeaderPtr->h3.bitfield.parity = parity_bit;
		return STECI_STATUS_HEADER_PARITY_ERROR;
	}

	// restore STECI UART header parity bit
	steciHeaderPtr->h3.bitfield.parity = parity_bit;

	//check header noPayload bit
	if (((steciHeaderPtr->h0.bitfield.noPayload == 1) && (paramsPtr->TotalBytesNumRxUart != STECI_HEADER_LENGTH)) ||
		((steciHeaderPtr->h0.bitfield.noPayload == 0) && (paramsPtr->TotalBytesNumRxUart == STECI_HEADER_LENGTH)))
	{
		// wrong STECI UART header wrong noPayload received
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong noPayload bit: %d",
			steciHeaderPtr->h0.bitfield.noPayload);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		return STECI_STATUS_DATA_NOT_PRESENT;
	}

	// check header payload length
	if (paramsPtr->TotalBytesNumRxUart !=
		(STECI_HEADER_LENGTH + steciHeaderPtr->lenLsb + ((steciHeaderPtr->h0.bitfield.lenMsb << 8) & 0xFF00) + 1))
	{
		// wrong STECI UART payload length
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong payload length: %d %d",
			(steciHeaderPtr->lenLsb + ((steciHeaderPtr->h0.bitfield.lenMsb << 8) & 0xFF00) + 1),
			(paramsPtr->TotalBytesNumRxUart - STECI_HEADER_LENGTH));
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		return STECI_STATUS_DATA_NOT_PRESENT;
	}

	// check Lun Target
	switch (steciHeaderPtr->lun & STECI_UART_LUN_TARGET_MASK)
	{
		case STECI_UART_CMOST2_LUN:
		case STECI_UART_DCOP_HD_LUN:
		case STECI_UART_CMOST_LUN:
		case STECI_UART_MCP_LUN:
			// good Lun
			break;

		default:
			if ((steciHeaderPtr->lun >= STECI_UART_MCP_LUN) || (steciHeaderPtr->lun <= STECI_UART_DUMMY_LUN))
			{
				// wrong Lun Target upper 4 bits
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong Lun Target upper 4 bits: 0x%x",
					steciHeaderPtr->lun);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
				return STECI_STATUS_INVALID_LUN;
			}
			// good Lun
			break;
	}

	// check payload crc
	if ((steciHeaderPtr->h0.bitfield.noPayload == 0) &&
		(steciHeaderPtr->h3.bitfield.crc != STECI_CalculateCRC((paramsPtr->DataBufferRx.DataBufferRxUart + STECI_HEADER_LENGTH), (paramsPtr->TotalBytesNumRxUart - STECI_HEADER_LENGTH))))
	{
		// wrong STECI UART payload crc
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong payload crc: %d",
			steciHeaderPtr->h3.bitfield.crc);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		return STECI_STATUS_DATA_CRC_ERROR;
	}

	return STECI_STATUS_SUCCESS;
}

/***************************
 *
 * STECI_UART_MessageDecode
 *
 **************************/
/*!
 * \brief		decode STECI UART message, convert it to Protocol Layer and send it
 * \param[in,out]	paramsPtr :  STECI UART txRx information pointer
 * \return		STECI_STATUS_SUCCESS, STECI_STATUS_NO_RESOURCES, STECI_STATUS_INVALID_LUN
 * \callgraph
 * \callergraph
 */
static tS32 STECI_UART_MessageDecode(STECI_UART_paramsTy *paramsPtr)
{
	STECI_headerType *steciHeaderPtr = (STECI_headerType *)paramsPtr->txRxData.DataBufferRx.DataBufferRxUart;
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	tU8 dataType;
	tBool itIsRawBinary = false, itIsPcmAudio = false, forwardToDataPortConsumer = false;
	tU8 spareByteArray[4] = {0, 0, 0, 0}, *p_spareByteArray, *p_inDataBuffer;
	tS32 inDataBuffer_len;
#endif // #if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)

	// new message fragment
	if (steciHeaderPtr->h0.bitfield.firstFragment == 1)
	{
		if (paramsPtr->totalBytesNumInDataBuf != 0)
		{
			// STECI UART previous message not sent
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx previous message not sent: %d",
				paramsPtr->totalBytesNumInDataBuf);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		}

		paramsPtr->totalBytesNumInDataBuf = 0;
		paramsPtr->txRxData.LunRxUart = steciHeaderPtr->lun;
	}

	// limit the message size to STECI_UART_MAX_MESSAGE_LEN
	if (((tS32)paramsPtr->totalBytesNumInDataBuf + (tS32)paramsPtr->txRxData.TotalBytesNumRxUart - (tS32)STECI_HEADER_LENGTH) > (tS32)STECI_UART_MAX_MESSAGE_LEN)
	{
		// STECI UART payload length too big
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message length too big: %d",
			paramsPtr->totalBytesNumInDataBuf + paramsPtr->txRxData.TotalBytesNumRxUart - STECI_HEADER_LENGTH);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		paramsPtr->totalBytesNumInDataBuf = 0;
		paramsPtr->txRxData.LunRxUart = 0;
		return STECI_STATUS_NO_RESOURCES;
	}

	// check if same Lun
	if (paramsPtr->txRxData.LunRxUart != steciHeaderPtr->lun)
	{
		// wrong STECI UART Lun
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART Rx message wrong Lun: %d %d",
			steciHeaderPtr->lun, paramsPtr->txRxData.LunRxUart);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		paramsPtr->totalBytesNumInDataBuf = 0;
		paramsPtr->txRxData.LunRxUart = 0;
		return STECI_STATUS_INVALID_LUN;
	}

	// copy message fragment
	OSAL_pvMemoryCopy((tPVoid)(paramsPtr->inDataBuffer + paramsPtr->totalBytesNumInDataBuf),
		(tPCVoid)(paramsPtr->txRxData.DataBufferRx.DataBufferRxUart + STECI_HEADER_LENGTH), 
		paramsPtr->txRxData.TotalBytesNumRxUart - STECI_HEADER_LENGTH);
	paramsPtr->totalBytesNumInDataBuf += paramsPtr->txRxData.TotalBytesNumRxUart - STECI_HEADER_LENGTH;

	// last message fragment
	if (steciHeaderPtr->h0.bitfield.lastFragment == 1)
	{
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
		p_spareByteArray = (tU8 *)NULL;

		// If the data is coming from the data channel then extract data type and if it is 
		// binary or ASCII information
		if (DEVICE_LUN_IS_DATA_CHANNEL == paramsPtr->txRxData.LunRxUart)
		{
			// Get the data type
			dataType = paramsPtr->inDataBuffer[4];

			// Get the information if it is binary data
			if (0x40 == (paramsPtr->inDataBuffer[1] & (tU8)0xE0))
			{
				itIsRawBinary = true;
			}

			// Get the information if it is PCM audio
			if (0xE0 == (paramsPtr->inDataBuffer[1] & (tU8)0xE0))
			{
				itIsPcmAudio = true;
			}

			// Depending on the LUN we use a different dispatch function so the TCP/IP interface can understand
			// what kind of data is coming. We send to the data channel only AUDIO BINARY information, not ASCII, for this
			// one we prefer to use normal channel			 
			if (true == itIsPcmAudio)
			{
				// Send data channel data to TCP/IP: this is the PCM AUDIO data
				forwardToDataPortConsumer = true;
			}
			else if ((true == itIsRawBinary) && (0x40 == dataType))
			{
				// Send data channel data to TCP/IP: this is the BINARY DRM OBJ
				forwardToDataPortConsumer = true;
			}
			else if ((true == itIsRawBinary) && (0x46 == dataType))
			{
				// Send data channel data to TCP/IP: this is the BINARY DRM MDI frame
				forwardToDataPortConsumer = true;
			}

			p_inDataBuffer = paramsPtr->inDataBuffer;
			inDataBuffer_len = paramsPtr->totalBytesNumInDataBuf;
		}
		else if ((STECI_UART_DCOP_HD_LUN | (BROADCAST_LUN & 0x0F)) == paramsPtr->txRxData.LunRxUart)
		{
			spareByteArray[1] = paramsPtr->inDataBuffer[0];
			p_spareByteArray = spareByteArray;
			p_inDataBuffer = &(paramsPtr->inDataBuffer[1]);
			inDataBuffer_len = (paramsPtr->totalBytesNumInDataBuf - 1);
		}
		else
		{
			p_inDataBuffer = paramsPtr->inDataBuffer;
			inDataBuffer_len = paramsPtr->totalBytesNumInDataBuf;
		}

		// Forward the data in the proper way depending on previous checks
		if (true == forwardToDataPortConsumer)
		{
			// Send data as DATA to the TCP/IP (depending on user parameters could be send to file and/or TCP/IP)
			ForwardDataChannelPacketToCtrlAppPort(paramsPtr->devHandle, p_inDataBuffer,
				inDataBuffer_len, STECI_UART_Lun2ProtocolLayerLun(paramsPtr->txRxData.LunRxUart),
				STECI_UART_Lun2ProtocolLayerPort(paramsPtr->txRxData.LunRxUart), p_spareByteArray, dataType);
		}
		else
		{
			// Send data to TCP/IP
			ForwardPacketToCtrlAppPort(paramsPtr->devHandle, p_inDataBuffer,
				inDataBuffer_len, STECI_UART_Lun2ProtocolLayerLun(paramsPtr->txRxData.LunRxUart),
				STECI_UART_Lun2ProtocolLayerPort(paramsPtr->txRxData.LunRxUart), p_spareByteArray);
		}
#elif defined(CONFIG_APP_RADIO_IF) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
		// send message to rif protocol router
		ForwardRxPacketToProtocolRouter(paramsPtr->txRxData.LunRxUart, paramsPtr->inDataBuffer, paramsPtr->totalBytesNumInDataBuf);
#endif

		// set in data buffer length to 0
		paramsPtr->totalBytesNumInDataBuf = 0;
	}

	return STECI_STATUS_SUCCESS;
}

/***************************
 *
 * STECI_UART_setLunPortFromLun
 *
 **************************/
/*!
 * \brief		set Lun and port mapping
 * \param[in]	LunSteciUart :  LUN STECI UART
 * \return		LUN Protocol Layer or LUN invalid
 * \callgraph
 * \callergraph
 */
static void STECI_UART_setPortFromLun(tU8 lun, tU32 port)
{
	tU8 i;
	tS16 idx_found = -1;

	for(i = 0; i < STECI_UART_LunPort.nb_lun_port; i++)
	{
		if (((lun > STECI_UART_LAST_MDR3_LUN) && ((STECI_UART_LunPort.lun_port[i].lun & STECI_UART_LUN_TARGET_MASK) == (lun & STECI_UART_LUN_TARGET_MASK))) ||
			((lun >= STECI_UART_FIRST_MDR3_LUN) && (lun <= STECI_UART_LAST_MDR3_LUN) &&
			(STECI_UART_LunPort.lun_port[i].lun >= STECI_UART_FIRST_MDR3_LUN) &&
			(STECI_UART_LunPort.lun_port[i].lun <= STECI_UART_LAST_MDR3_LUN)))
		{
			idx_found = i;
			STECI_UART_LunPort.lun_port[i].port = port;
			break;
		}
	}
	if (idx_found == -1)
	{
		if (i < STECI_UART_NB_MAX_LUN_PORT)
		{
			STECI_UART_LunPort.lun_port[i].lun = lun;
			STECI_UART_LunPort.lun_port[i].port = port;
			STECI_UART_LunPort.nb_lun_port++;
		}
		else
		{
			// STECI_UART_LunPort list full
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
			OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART STECI_UART_LunPort list full: %d",
				STECI_UART_LunPort.nb_lun_port);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		}
	}
}

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
/***************************
 *
 * STECI_UART_Lun2ProtocolLayerLun
 *
 **************************/
/*!
 * \brief		convert STECI UART LUN into Protocol Layer LUN
 * \param[in]	LunSteciUart :  LUN STECI UART
 * \return		LUN Protocol Layer or LUN invalid
 * \callgraph
 * \callergraph
 */
static tU8 STECI_UART_Lun2ProtocolLayerLun(tU8 LunSteciUart)
{
	tU8 LunRet = STECI_INVALID_LUN;

	// convert STECI UART LUN into Protocol Layer LUN
	switch (LunSteciUart & STECI_UART_LUN_TARGET_MASK)
	{
		case STECI_UART_CMOST2_LUN:
		case STECI_UART_DCOP_HD_LUN:
		case STECI_UART_CMOST_LUN:
			LunRet = BROADCAST_LUN;
			break;

		case STECI_UART_MCP_LUN:
		default:
			if (((LunSteciUart > STECI_UART_DUMMY_LUN) && (LunSteciUart < STECI_UART_MCP_LUN)) ||
				((LunSteciUart & STECI_UART_LUN_TARGET_MASK) == STECI_UART_MCP_LUN))
			{
				LunRet = LunSteciUart;
			}
			break;
	}

	return LunRet;
}

/***************************
 *
 * STECI_UART_Lun2ProtocolLayerPort
 *
 **************************/
/*!
 * \brief		convert STECI UART LUN into Protocol Layer Port
 * \param[in]	LunSteciUart :  LUN STECI UART
 * \return		Protocol Layer Port or INVALID_PORT_NUMBER
 * \callgraph
 * \callergraph
 */
static tU32 STECI_UART_Lun2ProtocolLayerPort(tU8 lun)
{
	tU32 PortRet = INVALID_PORT_NUMBER;
	tU8 i;
	tS16 idx_found = -1;

	for(i = 0; i < STECI_UART_LunPort.nb_lun_port; i++)
	{
		if (((lun > STECI_UART_LAST_MDR3_LUN) && ((STECI_UART_LunPort.lun_port[i].lun & STECI_UART_LUN_TARGET_MASK) == (lun & STECI_UART_LUN_TARGET_MASK))) ||
			((lun >= STECI_UART_FIRST_MDR3_LUN) && (lun <= STECI_UART_LAST_MDR3_LUN) &&
			(STECI_UART_LunPort.lun_port[i].lun >= STECI_UART_FIRST_MDR3_LUN) &&
			(STECI_UART_LunPort.lun_port[i].lun <= STECI_UART_LAST_MDR3_LUN)))
		{
			idx_found = i;
			PortRet = STECI_UART_LunPort.lun_port[i].port;
			break;
		}
	}
	if (idx_found == -1)
	{
		// STECI UART port not found
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
		OSAL_TRACE_PRINTF(TR_LEVEL_ERRORS, TR_CLASS_EXTERNAL, "STECI UART port not found from Lun: %d", lun);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_ERRORS)
	}

	return PortRet;
}
#endif //#if  defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)

#ifdef __cplusplus
}
#endif

#endif // #if defined (CONFIG_BUILD_DRIVER) && defined (CONFIG_COMM_STECI_UART)

// End of file
