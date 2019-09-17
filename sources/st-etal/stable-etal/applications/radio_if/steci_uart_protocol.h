//!
//!  \file    steci_uart_protocol.h
//!  \brief   <i><b> STECI UART protocol main function entry point </b></i>
//!  \details Interface file for upper software layer. State machine is implemented here.
//!  \author  Alberto Saviotti
//!

#ifndef STECI_UART_PROTOCOL_H
#define STECI_UART_PROTOCOL_H

#include "DAB_Protocol.h"

#include "steci_protocol.h"


#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///
#define STECI_UART_MAX_MESSAGE_LEN              ((tU32)(STECI_MAX_PACKET_LENGTH * 2))

#define STECI_UART_LUN_TARGET_MASK              0xF0

#define STECI_UART_CMOST1_I2C_ADDR              0xC2
#define STECI_UART_CMOST2_I2C_ADDR              0xC8

#define STECI_UART_CMOST1_SPI_ADDR              0x00
#define STECI_UART_CMOST2_SPI_ADDR              0x01

///
// Enums
///
typedef enum
{
	STECI_UART_NORMAL_MODE = 0,
	STECI_UART_RESET_MODE = 1,
	STECI_UART_BOOT_MODE = 2,
	STECI_UART_FLASH_MODE = 3
} STECI_UART_cmdModeEnumTy;

typedef enum
{
	STECI_UART_BROADCAST_LUN        = BROADCAST_LUN,
	STECI_UART_PROTOCOL_LUN         = PROTOCOL_LUN,
	STECI_UART_CMOST2_LUN           = 0xE0,
	STECI_UART_DCOP_HD_LUN          = 0xD0,
	STECI_UART_CMOST_LUN            = 0xC0,
	STECI_UART_MCP_LUN_AUTO_NOTIF   = 0x42, // LUN auto notificaiton to MCP, e.g. A2/A5
	STECI_UART_MCP_LUN_DATA_CH      = 0x41, // LUN data channel to MCP, e.g. A2/A5
	STECI_UART_MCP_LUN              = 0x40, // LUN message to MCP, e.g. A2/A5
	STECI_UART_LAST_MDR3_LUN        = 0x3F, // last MDR3 DCOP DAB LUN
	STECI_UART_FIRST_MDR3_LUN       = 0x01, // first MDR3 DCOP DAB LUN
	STECI_UART_DUMMY_LUN            = 0x00
} STECI_UART_LunEnumTy;

///
// Types
///
typedef struct
{
	// from Protocol Layer
	tU8  DataBuffer[STECI_UART_MAX_MESSAGE_LEN];        // Tx data buffer to UART (PL header + payload)
	tU16 TotalBytesNum;
	tU16 RemainingBytesNum;
	tU8  Lun;
	tU8  SnInfo;
	tBool retry;
	tBool dataToBeTx;       // Tx data ready flag
	tSInt numberOfRetry;

	// from UART
	union {
		tU8  DataBufferRxUart[STECI_MAX_PACKET_LENGTH];  // Rx data buffer from UART (STECI header + payload)
		STECI_headerType DataBufferRxUartSteciHeader[1];
	}DataBufferRx;
	tU16 TotalBytesNumRxUart;   // Length of valid DataBufferRxUart bytes, set by MCP protocol. Set to 0 by STECI UART protocol before setting dataToBeRx to false.
	tU8  LunRxUart;
	tBool dataToBeRx;           // Rx data ready flag. set to true by MCP protocol and set to false by STECI UART protocol
} STECI_UART_txRxInfo;

// Parameter structure
typedef struct
{
	tVoid *devHandle;
	tU32 id;
	tU32 speed;

	tU32 sourceAndDestinationIdentifier; // This is the port number that send the data and retrieve them

	tU8 deviceMode;

	STECI_UART_cmdModeEnumTy cmdMode;

	STECI_UART_txRxInfo txRxData;
	tU8 outDataBuffer[STECI_MAX_PACKET_LENGTH];     // STECI UART Tx data buffer to UART (STECI header + payload)
	tU8 inDataBuffer[STECI_UART_MAX_MESSAGE_LEN];   // STECI UART Rx data buffer from UART (PL payload)
	tU16 totalBytesNumInDataBuf;                    // number of valid bytes in inDataBuffer
} STECI_UART_paramsTy;

///
// Exported functions
///
extern tS32 STECI_UART_init(tVoid);
extern tS32 STECI_UART_SendMessageStart(tVoid *memoryPtr, tU8 LunId, tU8 *DataToSend,
                                  tU8 specific0, tU8 specific1, tU8 specific2, tU8 specific3, tU16 BytesNumber,
                                  tU32 dataSourceAndDestinationId, DEV_protocolEnumTy devProtocol);

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
extern tUInt __stdcall STECI_UART_ProtocolHandle(tVoid *pParams);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
extern tVoid STECI_UART_ProtocolHandle(tVoid *pParams);
#endif

#ifdef __cplusplus
}
#endif

#endif // STECI_UART_PROTOCOL_H

// End of file
