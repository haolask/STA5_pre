//!
//!  \file    mcp_protocol.h
//!  \brief   <i><b> MCP protocol main function entry point </b></i>
//!  \details Interface file for upper software layer. State machine is implemented here.
//!  \author  Alberto Saviotti
//!

#ifndef MCP_PROTOCOL_H
#define MCP_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///
#define MCP_UART_RX_BUFFER_SIZE            65535

#define MCP_FRAME_START_STOP_BYTE       0x7E	/* Start stop character */
#define MCP_FRAME_DLE_BYTE              0x7D	/* Data Link Escape character */


///
// Enums
///
typedef enum
{
	MCP_rawFrameWaitHeader = 0,     /* wait for header 7E */
	MCP_rawFrameInMsg,              /* in message */
	MCP_rawFrameAfterEsc            /* after escape 0x7D */
} MPC_RawFrameStateTy;

///
// Exported functions
///
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
extern tUInt __stdcall MCP_ProtocolHandle(tVoid *pParams);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
extern tVoid MCP_ProtocolHandle(tVoid *pParams);
#endif
extern tS32 MCP_MessageSend(tVoid *deviceHandle, tU8 *txBufferPtr, tU16 len);

#ifdef __cplusplus
}
#endif

#endif // STECI_UART_PROTOCOL_H

