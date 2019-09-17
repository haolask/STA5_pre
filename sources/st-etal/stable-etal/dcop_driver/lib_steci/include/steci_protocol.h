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

// Numbers of retry before to give-up sending data to the device
#define STECI_DATA_SEND_RETRY_MAX_NUM		    3
#define DEVICE_LUN_IS_DATA_CHANNEL			    ((tU8)0x31)
#define ETAL_STECI_THREAD_SCHEDULING            1

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

#ifdef __cplusplus
}
#endif

#endif // STECI_PROTOCOL_H
// End of file
