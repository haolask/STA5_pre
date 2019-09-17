//!
//!  \file 		 steci_protocol.h
//!  \brief 	 <i><b> STECI main function entry point </b></i>
//!  \details Interface file for upper software layer. State machine is implemented here.
//!  \author 	Alberto Saviotti
//!
#ifndef STECI_PROTOCOL_H
#define STECI_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///
//
// If defined implements a minimum interval different than zero between two transmissions host to device. Used with IMG's peripherals in FPGA
//
//#define STECI_LIMITING_TX_TIMEFRAME
#undef STECI_LIMITING_TX_TIMEFRAME

//
// If defined checks the undefined MISO data for high level. Used with IMG's peripherals in FPGA
//
//#define STECI_MISO_HIGH_LEVEL_UNDEFINED_DATA
#undef STECI_MISO_HIGH_LEVEL_UNDEFINED_DATA

//
// May be used for boot protocol, disable to align PC and Accordo2 (ETAL) versions
//
//#define STECI_USE_LARGE_PAYLOAD
#undef STECI_USE_LARGE_PAYLOAD

//
// If defined uses a fixed frame size regardless the dimension of message to transfer. Used for example with IMG's peripherals in FPGA
//
//#define STECI_FIXED_PAYLOAD_LENGTH
#undef STECI_FIXED_PAYLOAD_LENGTH

// STECI related defines
// Clash management
#ifdef STECI_LIMITING_TX_TIMEFRAME
#define STECI_TX_MIN_TIMEFRAME_MS           49
#endif

// Data bytes
#ifdef STECI_MISO_HIGH_LEVEL_UNDEFINED_DATA
	#define STECI_MISO_UNDEFINED_DATA_BYTE      0xFF
#else
	#define STECI_MISO_UNDEFINED_DATA_BYTE      0x00
#endif

// Frame format
#define STECI_HEADER_LENGTH                 4

#ifdef STECI_USE_LARGE_PAYLOAD
	#define STECI_MAX_PAYLOAD_LENGTH            16380
#elif (defined STECI_FIXED_PAYLOAD_LENGTH)
	#define STECI_MAX_PAYLOAD_LENGTH            60
#else
	#define STECI_MAX_PAYLOAD_LENGTH            4088
#endif

#define STECI_MAX_PACKET_LENGTH             (STECI_HEADER_LENGTH + STECI_MAX_PAYLOAD_LENGTH)

#if (defined STECI_FIXED_PAYLOAD_LENGTH) && (! defined STECI_USE_LARGE_PAYLOAD)
#define STECI_PAYLOAD_MODULO                STECI_MAX_PAYLOAD_LENGTH
#define STECI_PACKET_MODULO                 STECI_MAX_PACKET_LENGTH // (It must be ^2)
#else
#define STECI_PAYLOAD_MODULO                4
#define STECI_PACKET_MODULO                 4 // (It must be ^2)
#endif

#define STECI_MAX_MESSAGE_LEN               ((tU32)(STECI_MAX_PACKET_LENGTH * 2))

// Numbers of retry before to give-up sending data to the device
#define STECI_DATA_SEND_RETRY_MAX_NUM		    3
#define DEVICE_LUN_IS_DATA_CHANNEL			    ((tU8)0x31)
#define ETAL_STECI_THREAD_SCHEDULING            1
#define STECI_MAX_TIME_TO_WAIT_FOR_MSG_SENDING	1000
#define ETAL_STECI_THREAD_TIME_BETWEEN_REQUESTS 1

///
// Enums
///
typedef enum
{
	STECI_NORMAL_MODE = 0,
	STECI_RESET_MODE  = 1,
	STECI_BOOT_MODE   = 2,
	STECI_FLASH_MODE  = 3
} STECI_cmdModeEnumTy;


///
// Types
//
typedef struct
{
    tU8  DataBuffer[STECI_MAX_MESSAGE_LEN];
    tU16 TotalBytesNum;
    tU16 RemainingBytesNum;
    tU8  Lun;
    tU8  SnInfo;
	tBool retry;
	tBool dataToBeTx;
	tSInt numberOfRetry;
} STECI_txRxInfo;

// Parameter structure
typedef struct
{
    tVoid *devHandle;
	tVoid *devSecondaryHandle;

	tU8 deviceMode;

	DEV_txModeEnumTy connectionMode;
	STECI_cmdModeEnumTy cmdMode;
    STECI_txRxInfo txRxData;
	tU8 outDataBuffer[STECI_MAX_PACKET_LENGTH];
	tU8 inDataBuffer[STECI_MAX_PACKET_LENGTH];

	tU8 tmpBuffer[STECI_MAX_PACKET_LENGTH];
} STECI_paramsTy;

typedef struct
{
	union
	{
		struct
		{
			unsigned char lenMsb : 4;
			unsigned char firstFragment : 1;
			unsigned char lastFragment : 1;
			unsigned char reTransmission : 1;
			unsigned char noPayload : 1;
		} bitfield;

		unsigned char h0val;
	} h0;

	unsigned char lenLsb;

	unsigned char lun;

	union
	{
		struct
		{
			unsigned char crc : 7;
			unsigned char parity : 1;
		} bitfield;

		unsigned char h3val;
	} h3;
} STECI_headerType;

// variables

#define STECI_ProtocolHandle_MsgSendingSem_STR    "STeciSendingSem"
#define STECI_ProtocolHandle_MsgSentSem_STR    "STeciSentSem"

extern OSAL_tSemHandle      STECI_ProtocolHandle_MsgSendingSem;
extern OSAL_tSemHandle      STECI_ProtocolHandle_MsgSentSem;

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
extern OSAL_tEventHandle ETAL_Steci_TaskEventHandler;
#endif

///
// Exported functions
///
#if defined (CONFIG_HOST_OS_LINUX )
extern void STECI_ProtocolHandle (tPVoid pParams);
#elif defined (CONFIG_HOST_OS_FREERTOS)
extern void STECI_ProtocolHandle (tPVoid pParams);
#elif defined (CONFIG_HOST_OS_TKERNEL) 
extern tVoid STECI_ProtocolHandle(tSInt stacd, tPVoid pParams);
#endif

extern tS32 STECI_SendMessageStart (tVoid *memoryPtr, tU8 LunId, tU8 *DataToSend,
                                    tU8 specific0, tU8 specific1, tU8 specific2, tU8 specific3, tU16 BytesNumber);

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI
// External function to manage the signals  
tVoid ETAL_STECI_DataToTx(tVoid);
tVoid ETAL_STECI_DataRx(tVoid);
tBool STECI_EventWaitReqReady (STECI_deviceInfoTy *deviceInfoPtr, tBool levelToWaitFor);
#endif


#ifdef __cplusplus
}
#endif

#endif // STECI_PROTOCOL_H
// End of file
