//!
//!  \file    mcp_protocol.cpp
//!  \brief   <i><b> MCP protocol </b></i>
//!  \details This module implements the "Logical Messaging Layer" of the MCP specifications.
//!  \author  David Pastor
//!

#include "target_config.h"

#ifdef CONFIG_APP_RADIO_IF
#include "target_config_radio_if.h"
#endif // CONFIG_APP_RADIO_IF

///
// This define shall be defined just after inlcusion of target config because 
// it is used to correctly setup printf level
///
#define CONFIG_TRACE_CLASS_MODULE       CONFIG_TRACE_CLASS_MCP

#if defined (CONFIG_BUILD_DRIVER) && defined (CONFIG_COMM_MCP)
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "types.h"

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW) || defined (CONFIG_HOST_OS_LINUX_DESKTOP)
#include "common_helpers.h"
#elif defined(CONFIG_HOST_OS_LINUX_EMBEDDED) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
#include "osal.h"
#include "common_trace.h"
#include "DAB_Protocol.h"
#include "radio_if_util.h"
#endif

#if defined(CONFIG_HOST_OS_LINUX_EMBEDDED) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
#include "etaldefs.h"
#include "ipfcomm.h"
#include "DAB_Protocol.h"
#endif
#include "TcpIpProtocol.h"

#include "steci_defines.h"
#include "connection_modes.h"
#include "steci_protocol.h"
#include "steci_uart_protocol.h"

#include "mcp_protocol.h"

#include "uart_mngm.h"

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///
#define MCP_STR_PREFIX_TX   "MCP Tx: "
#define MCP_STR_PREFIX_RX   "MCP Rx: "

///
// Type definitions
///

///
// Local variables
///
static tU8 MCPUartTempRxBuffer[MCP_UART_RX_BUFFER_SIZE];

///
// Local function prototypes
///

///
// Global function definitions
///

/***************************
 *
 * MCP_ProtocolHandle
 *
 **************************/
/*!
 * \brief		MCP protocol handling for uart Rx
 * \param[in]	pParams :  STECI UART parameters pointer
 * \return		STECI_STATUS_SUCCESS, STECI_STATUS_ERROR
 * \callgraph
 * \callergraph
 */
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
tUInt __stdcall MCP_ProtocolHandle(tVoid *pParams)
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
tVoid MCP_ProtocolHandle(tVoid *pParams)
#endif
{
	tU16 ReceivedBytes, TotalReceivedBytes = 0;
	tS32 RdPos = 0;
	STECI_UART_paramsTy *paramsPtr = (STECI_UART_paramsTy *)pParams;
	tVoid *deviceHandle;
	MPC_RawFrameStateTy mcp_rawFrameState = MCP_rawFrameWaitHeader;

	// Get device handler
	deviceHandle = paramsPtr->devHandle;

	paramsPtr->txRxData.TotalBytesNumRxUart = 0;

	while (1)
	{
		// Read some data on UART
		ReceivedBytes = CommReceiveData(deviceHandle, &(MCPUartTempRxBuffer[TotalReceivedBytes]), (sizeof(MCPUartTempRxBuffer) - TotalReceivedBytes), -1);

		if (ReceivedBytes > 0)
		{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
			UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, &(MCPUartTempRxBuffer[TotalReceivedBytes]), ReceivedBytes, MCP_STR_PREFIX_RX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

			TotalReceivedBytes += ReceivedBytes;
		}

		if (TotalReceivedBytes > 0)
		{
			while ((paramsPtr->txRxData.dataToBeRx == false) && (RdPos < TotalReceivedBytes))
			{
				// fill the STECI UART Rx buffer only when not used by STECI UART protocol
				switch(mcp_rawFrameState)
				{
					case MCP_rawFrameWaitHeader:
						if (MCPUartTempRxBuffer[RdPos] == MCP_FRAME_START_STOP_BYTE)
						{
							/* start flag: read msg */
							paramsPtr->txRxData.TotalBytesNumRxUart = 0;
							mcp_rawFrameState = MCP_rawFrameInMsg;
						}
						break;

					case MCP_rawFrameInMsg:
						if (MCPUartTempRxBuffer[RdPos] == MCP_FRAME_START_STOP_BYTE)
						{
							/* start or end flag */
							if (paramsPtr->txRxData.TotalBytesNumRxUart > 0)
							{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
								OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "MCP_rawFrameInMsg end flag received %d bytes", paramsPtr->txRxData.TotalBytesNumRxUart);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

								/* end flag: decode msg*/
								paramsPtr->txRxData.dataToBeRx = true;

								/* copy last raw frame bytes to begining of buffer if buffer not empty */
								if ((RdPos + 1) < TotalReceivedBytes)
								{
									/* copy last raw frame bytes to begining of buffer, use memmove because memory overlap */
									OSAL_pvMemoryMove(MCPUartTempRxBuffer, &(MCPUartTempRxBuffer[RdPos + 1]), (TotalReceivedBytes - RdPos - 1));
									TotalReceivedBytes -= (RdPos + 1);
								}
								else
								{
									/* raw frame buffer empty */
									TotalReceivedBytes = 0;
								}
								RdPos = -1;	/* read raw frame position will be incremented to 0 at end of while loop */

								mcp_rawFrameState = MCP_rawFrameWaitHeader;
							}
							else
							{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
								OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "MCP_rawFrameInMsg start flag received !");
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

								/* start flag: read new msg */
								paramsPtr->txRxData.TotalBytesNumRxUart = 0;
							}
						}
						else if (MCPUartTempRxBuffer[RdPos] == MCP_FRAME_DLE_BYTE)
						{
							/* Data Link Escape character: wait next character */
							mcp_rawFrameState = MCP_rawFrameAfterEsc;
						}
						else
						{
							/* msg byte */
							if (paramsPtr->txRxData.TotalBytesNumRxUart >= STECI_UART_MAX_MESSAGE_LEN)
							{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
								OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "MCP_rawFrameInMsg too big frame received %d bytes", paramsPtr->txRxData.TotalBytesNumRxUart);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

								/* msg too long: decode msg and drop end of msg */
								paramsPtr->txRxData.dataToBeRx = true;

								/* copy last raw frame bytes to begining of buffer if buffer not empty */
								if (RdPos < TotalReceivedBytes)
								{
									/* copy last raw frame bytes to begining of buffer, use memmove because memory overlap */
									OSAL_pvMemoryMove(MCPUartTempRxBuffer, &(MCPUartTempRxBuffer[RdPos]), (TotalReceivedBytes - RdPos));
									TotalReceivedBytes -= RdPos;
								}
								else
								{
									/* raw frame buffer empty */
									TotalReceivedBytes = 0;
								}
								RdPos = -1;	/* read raw frame position will be incremented to 0 at end of while loop */

								mcp_rawFrameState = MCP_rawFrameWaitHeader;
							}
							else
							{
								/* read msg byte */
								paramsPtr->txRxData.DataBufferRx.DataBufferRxUart[paramsPtr->txRxData.TotalBytesNumRxUart] = MCPUartTempRxBuffer[RdPos];
								paramsPtr->txRxData.TotalBytesNumRxUart++;
							}
						}
						break;

					case MCP_rawFrameAfterEsc:
						if (paramsPtr->txRxData.TotalBytesNumRxUart >= STECI_UART_MAX_MESSAGE_LEN)
						{
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)
							OSAL_TRACE_PRINTF(TR_LEVEL_SYSTEM_MIN, TR_CLASS_EXTERNAL, "MCP_rawFrameAfterEsc too big frame received %d bytes", paramsPtr->txRxData.TotalBytesNumRxUart);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_SYSTEM_MIN)

							/* msg too long: decode msg and drop end of msg */
							paramsPtr->txRxData.dataToBeRx = true;

							/* copy last raw frame bytes to begining of buffer if buffer not empty */
							if (RdPos < TotalReceivedBytes)
							{
								/* copy last raw frame bytes to begining of buffer, use memmove because memory overlap */
								OSAL_pvMemoryMove(MCPUartTempRxBuffer, &(MCPUartTempRxBuffer[RdPos]), (TotalReceivedBytes - RdPos));
								TotalReceivedBytes -= RdPos;
							}
							else
							{
								/* raw frame buffer empty */
								TotalReceivedBytes = 0;
							}
							RdPos = -1;	/* read raw frame position will be incremented to 0 at end of while loop */

							mcp_rawFrameState = MCP_rawFrameWaitHeader;
						}
						else
						{
							/* read msg byte */
							paramsPtr->txRxData.DataBufferRx.DataBufferRxUart[paramsPtr->txRxData.TotalBytesNumRxUart] = MCPUartTempRxBuffer[RdPos];
							paramsPtr->txRxData.TotalBytesNumRxUart++;

							mcp_rawFrameState = MCP_rawFrameInMsg;
						}
						break;

					default:
						/* This should not happen */
						ASSERT_ON_DEBUGGING(0);
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
						return -1;
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
						return;
#endif
						break;
				}
				/* increment read raw frame position */
				RdPos++;
			}
		}
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW) || defined (CONFIG_HOST_OS_LINUX)
		// We will rest if we do not have something to do already
		OSAL_s32ThreadWait(1);
#endif // #if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW) || defined (CONFIG_HOST_OS_LINUX)
	}

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	return 0;
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	return;
#endif
}

/***************************
 *
 * MCP_MessageSend
 *
 **************************/
/*!
 * \brief		Add MCP protections bytes to payload and send STECI UART frame
 * \param[in]	deviceInfoPtr :  UART device handle pointer
 * \param[in]	txBufferPtr :  Tx buffer pointer
 * \param[in]	len :  Tx buffer length
 * \return		STECI_STATUS_SUCCESS, STECI_STATUS_ERROR
 * \callgraph
 * \callergraph
 */
tS32 MCP_MessageSend(tVoid *deviceHandle, tU8 *txBufferPtr, tU16 len)
{
	tS32 res = STECI_STATUS_SUCCESS;
	tUChar buf[1];
	tU16 curSrcPos, curSrcLastPos;
	tU32 SentBytes;

#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	OSAL_TRACE_PRINTF(TR_LEVEL_COMPONENT, TR_CLASS_EXTERNAL, "MCP_MessageSend(%d bytes):", len);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	// Mark begining of frame
	buf[0] = MCP_FRAME_START_STOP_BYTE;
	while ((SentBytes = CommTransmitData(deviceHandle, buf, 1)) != 1)
	{
	}
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, buf, 1, MCP_STR_PREFIX_TX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	// send message
	curSrcPos = curSrcLastPos = 0;
	while (curSrcPos < len)
	{
		if ((txBufferPtr[curSrcPos] == MCP_FRAME_START_STOP_BYTE) || (txBufferPtr[curSrcPos] == MCP_FRAME_DLE_BYTE))
		{
			// send message fragment
			while(curSrcPos >= (curSrcLastPos + 1))
			{
				SentBytes = CommTransmitData(deviceHandle, &(txBufferPtr[curSrcLastPos]), (curSrcPos - curSrcLastPos));
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
				UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, &(txBufferPtr[curSrcLastPos]), SentBytes, MCP_STR_PREFIX_TX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
				curSrcLastPos += SentBytes;
			}

			// send DLE
			buf[0] = MCP_FRAME_DLE_BYTE;
			while((SentBytes = CommTransmitData(deviceHandle, buf, 1)) != 1)
			{
			}
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
			UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, buf, 1, MCP_STR_PREFIX_TX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
		}
		curSrcPos++;
	}
	// send message fragment
	while(curSrcPos >= (curSrcLastPos + 1))
	{
		SentBytes = CommTransmitData(deviceHandle, &(txBufferPtr[curSrcLastPos]), (curSrcPos - curSrcLastPos));
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
		UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, &(txBufferPtr[curSrcLastPos]), SentBytes, MCP_STR_PREFIX_TX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
		curSrcLastPos += SentBytes;
	}

	// Mark end of frame
	buf[0] = MCP_FRAME_START_STOP_BYTE;
	while ((SentBytes = CommTransmitData(deviceHandle, buf, 1)) != 1)
	{
	}
#if defined(CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)
	UTIL_TRACE_BUF_COMPONENT(TR_CLASS_EXTERNAL, buf, 1, MCP_STR_PREFIX_TX);
#endif // #if (defined CONFIG_TRACE_CLASS_MODULE) && (CONFIG_TRACE_CLASS_MODULE >= TR_LEVEL_COMPONENT)

	return res;
}

#ifdef __cplusplus
}
#endif

#endif // #if defined (CONFIG_BUILD_DRIVER) && defined (CONFIG_COMM_MCP)

// End of file
