/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_canif.c
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN interface routines definitions
********************************************************************************
* History:
* 12/09/2013: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _CANIF_H_
#define _CANIF_H_

#include "sta_can.h"
#include "sta_canif_cfg.h"
#include "sta_canif_types.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"


/****************************************************************************
*
| typedefs (scope: global)
|---------------------------------------------------------------------------
*/
typedef void (*UserTxConfirmation)	(PduIdType CanTxPduId);
typedef void (*UserRxIndication)	(Can_IdType, const uint8_t *);
typedef void (*UserControllerBusOff)	(uint8_t);
typedef void (*UserControllerWakeup)	(uint8_t);
typedef PDU_Dispatcher (*UserSoftwareFilterAlgorithms)	(Can_IdType);
typedef CanIf_DlcErrorReturnType (*User_DlcErrorNotification)	(PduIdType, uint8_t);
typedef uint8_t Std_ReturnType;

#define E_NOT_OK 1
#define E_OK 	 0

typedef struct
{
	CanIf_TxQueuePduStatus	ElementStatusFIFO;		// Status EMPTY or FULL
	Can_IdType		ID;				// ID of CanTxPduId
	uint8_t			CanTxPduSduFIFO[CAN_SDU_SIZE];	// Pointer to SDU
} CanIf_TransmitPduFIFO;


typedef struct
{
	uint8_t			CanTxPduId;		// Handle.
 	Can_IdType		ID;			// ID of CanTxPduId
	uint8_t			CanTxPduSdu[CAN_SDU_SIZE];// Pointer to SDU
	CanIf_TxQueuePduStatus	QueueElementStatus;	// Status EMPTY or FULL
	uint8_t			CanTxPduIdQueueSize;	// Size of per Tx L-PDU transmit queue
	CanIf_TransmitPduFIFO	*CanTransmitPduFIFO;	// Pointer FIFO for Multiple elements
} CanIf_TransmitQueue;

typedef struct
{
	uint8_t			CanTxPduId;		// Logical handle of CanTxPduId
	Can_IdType		ID;			// ID of CanTxPduId
	uint8_t			DLC;			// DLC of CanTxPduId
	uint8_t			Network;		// CAN Network of CanTxPduId
	CanIf_TxPduIdType	Type;			// Type of CanTxPduId
} CanIf_TxPduHandle;

typedef struct
{
	uint8_t			CanRxPduId;		// Logical handle of CanRxPduId
	Can_IdType		ID;			// ID of CanTxPduId
	uint8_t			DLC;			// DLC of CanRxPduId
	uint8_t			Network;		// CAN Network of CanRxPduId
} CanIf_RxPduHandle;

typedef struct
{
	uint8_t			Network;		// Logical handle of CAN network
	uint8_t			NetworkController;	// Name of CAN controller assigned to a CAN network
} CanIf_Network;


typedef struct
{
	char  			*User;			// Confimation
	uint8_t			CanTxPduId;		// Logical handle of CanTxPduId
	UserTxConfirmation	CanIf_UserTxConfirmation;
	uint8_t			CanRxPduId;		// Logical handle of CanRxPduId
	UserRxIndication	CanIf_UserRxIndication;
} CanIf_UsersUpperLayer;


typedef struct
{
	UserControllerBusOff	CanIf_UserControllerBusOff;
	UserControllerWakeup	CanIf_UserControllerWakeup;
} CanIf_UsersController;


typedef struct
{
	UserSoftwareFilterAlgorithms	CanIf_SoftwareFilterAlgorithms;
} CanIf_SoftwareFilterRX;


typedef struct
{
	User_DlcErrorNotification	CanIf_UserDlcErrorNotification;
} CanIf_DlcErrorNotification;


typedef struct
{
	CanIf_ModeNotification	OperationMode;			// Tx/Rx notification method
	uint8_t			NumMethodSwFilterRX;		// Num method Software Filter Rx
	bool			DLC_Check;			// DLC Check support
	bool			ControllerWakeup;		// CAN controller wakeup used / not used
	CanIf_TxQueueUsed	TransmitQueueType;		// Transmission queue operation type
	uint16_t			TransmitQueueSize;		// Size of all single queue buffers
	CanIf_TransmitQueue	*TransmitQueue;			// Transmit Queue
	CanIf_TxPduHandle	*CanTxPduHd;			// PDU Handle TX
	CanIf_RxPduHandle	*CanRxPduHd;			// PDU Handle RX
	uint8_t			NumberUsers;			// Number of Users
	CanIf_UsersUpperLayer	*UsersUpperLayer;		// Users Upper Layer
	uint8_t			NumberNetwork;			// Number of CAN networks
	CanIf_Network		*Network;			// Network
	CanIf_SoftwareFilterRX	*SoftwareReceiveFilter;		// Software Receive Filter
	CanIf_DlcErrorNotification *DlcErrorNotification;	// Software Receive Filter
	CanIf_UsersController 	*UsersController;		// Software Receive Filter
} CanIf_ConfigType;


void CanIf_UserTxConfirmation	(PduIdType CanTxPduId);
void CanIf_UserRxIndication	(Can_IdType CanRxPduId, const uint8_t *CanSduPtr);
PDU_Dispatcher CanIf_SoftwareFilterAlgorithms(Can_IdType CanRxPduId);



/************************************************************************
| defines and macros
|-----------------------------------------------------------------------*/
void CanIf_Init (RxIndication rxcb, uint32_t input_clock);
void CanIf_InitController( uint8_t Controller, CanIf_ConfigType	*ConfigPtr);
Std_ReturnType CanIf_SetControllerMode( uint8_t Controller, CanIf_DeviceModeType	DeviceMode);
CanIf_DeviceModeType CanIf_GetControllerMode( uint8_t Controller);
void CanIf_MainFunction(void);
Std_ReturnType CanIf_Transmit(PduIdType CanTxPduId, Can_PduType *PduInfoPtr);
void CanIf_RxIndication(uint8_t Hrh, Can_IdType CanId, uint8_t CanDlc, uint8_t *CanSduPtr);
void CanIf_TxConfirmation( PduIdType CanTxPduId);
void CanIf_ControllerBusOff( uint8_t Controller);
void CanIf_ControllerWakeup( uint8_t Controller);
Std_ReturnType CanIf_SetChannelMode( uint8_t Channel, CanIf_ChannelModeType ChannelMode);
CanIf_ChannelModeType CanIf_GetChannelMode( uint8_t Channel);

// PRIVATE
Std_ReturnType InsertPduQueue(PduIdType CanTxPduId, Can_PduType *PduInfoPtr, uint8_t Controller);
int CanIf_RxPduFilter(uint8_t Controller, uint8_t Hrh, Can_IdType CanId, uint8_t CanDlc);
uint8_t CanIf_TxQueueTreatment(uint8_t Controller);
uint8_t CanIf_FoundController(uint8_t NetworkID);
void CanIf_InitSdu(uint8_t *Buffer, uint8_t Size);
void CanIf_CopySdu(uint8_t *Source, uint8_t *Destination, uint8_t Size);
void CanIf_ShiftSxSduFIFO(CanIf_TransmitPduFIFO *PduFIFO, uint8_t SizeFIFO);
void CanIf_ResetQueueAndFIFO(void);
void User_ControllerBusOff(uint8_t Controller);
void User_ControllerWakeup(uint8_t Controller);
Can_ReturnType Can_Write (uint8_t Controller, Can_HwHandleType Hth, Can_PduType *IfPduInfo, PduIdType SwPduHandle );
Can_Mode CanIf_CanMode(uint8_t Hrh);
uint8_t CanIf_HTH(Can_IdType Id_Value);



#define CAN_DEV_ERROR_DETECT		// Development error detection

#define CANIF_E_PARAM_CONFIG	(t_uint8)0x10
#define CANIF_E_PARAM_ADDRESS	(t_uint8)0x11
#define CANIF_E_PARAM_DATA	(t_uint8)0x12
#define CANIF_E_PARAM_LENGTH	(t_uint8)0x13
#define CANIF_E_PARAM_DEVICE	(t_uint8)0x14
#define CANIF_E_UNINIT		(t_uint8)0x20
#define CANIF_E_NOK_NOSUPPORT	(t_uint8)0x30
#define CANIF_E_INVALID_TXPDUID	(t_uint8)0x40
#define CANIF_E_INVALID_RXPDUID	(t_uint8)0x50

// Each API is identifie with a service nymber (see Specification of Module CAN Interface 1.0.0)
#define CANIF_INIT_ID	        	(t_uint8)0x01
#define CANIF_INITCONTROLLER_ID		(t_uint8)0x02
#define CANIF_SETCONTROLLERMODE_ID	(t_uint8)0x03
#define CANIF_GETCONTROLLERMODE_ID	(t_uint8)0x04
#define CANIF_MAIN_FUNCTION_ID		(t_uint8)0x05
#define CANIF_TRANSMIT_ID		(t_uint8)0x06
#define CANIF_RXINDICATION_ID		(t_uint8)0x07
#define CANIF_TXINDICATION_ID		(t_uint8)0x08
#define CANIF_CONTROLLERBUSOFF_ID	(t_uint8)0x09
#define CANIF_CONTROLLERWAKEUP_ID	(t_uint8)0x0A
#define CANIF_SETCHANNNELMODE_ID	(t_uint8)0x0B
#define CANIF_GETCHANNNELMODE_ID	(t_uint8)0x0C


#endif
