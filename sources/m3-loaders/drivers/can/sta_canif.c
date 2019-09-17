/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_canif.c
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN interface routines
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

#include "sta_can.h"
#include "sta_canif.h"
#include "sta_canif_cfg.h"
#include "sta_canif_types.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"


static Can_PduType IfPduInfo;
static uint8_t DataSdu[CAN_SDU_SIZE];

CanIf_DeviceModeType CanIf_ControllerStatus[MAX_CONFIG_CAN_CONTROLLERS];

uint8_t CanIf_FoundUser(uint8_t Handle, uint8_t Tx_RX);

#ifdef UTEST
uint8_t Utest;
#endif

extern CanIf_ConfigType CanIf_Config;
extern Can_ConfigType Can_Config;

// ****************************************************************************
// CanIf_Init
//
// Desc.: service initializes internal and external interfaces of the CAN
//   	  Interface for the further processing.
//
// Parameters (in)	:	rxcb: user callback called when receiving data.
//				input_clock: can input clock rate
// Parameters (out)	:	None.
// ****************************************************************************
void CanIf_Init(RxIndication rxcb, uint32_t input_clock)
{
	uint8_t i;
	CanIf_DeviceModeType ControllerMode;
	Can_ControllerConfigType *Controller_ptr;

	CanIf_InitSdu(DataSdu, CAN_SDU_SIZE);
	IfPduInfo.Id = 100;
	IfPduInfo.Sdu = DataSdu;
	IfPduInfo.Lenght = CAN_SDU_SIZE;

	if (rxcb)
		Can_Config.Can_RxIndication = rxcb;

	Can_Config.input_clock = input_clock;

	// Init Driver
	Can_Init(&Can_Config);

	// Queue and FIFO Initializatin
	CanIf_ResetQueueAndFIFO();

	// Verification Controller Status
	i=0;
	Controller_ptr = Can_Config.Controllers_ptr;
	while ( i <  MAX_CONFIG_CAN_CONTROLLERS)// Number of CAN controllers
	{
		ControllerMode = CanIf_GetControllerMode( (uint8_t) Controller_ptr->Controller_Number );

		if ( ControllerMode != CANIF_CS_UNINIT && ControllerMode != CANIF_CS_STOP)
			return; // (ERRORE)

		// All the  states are: CANIF_CS_UNINIT, CANIF_CS_INIT, CANIF_CS_STOP, CANIF_CS_RUNNING, CANIF_CS_SLEEP
		if (ControllerMode == CANIF_CS_UNINIT)
		{
			CanIf_ControllerStatus[i] = CANIF_CS_STOP;
			CanIf_SetControllerMode( i, CANIF_CS_STOP);
		}

		Controller_ptr++;
		i++;
	}
}


// ****************************************************************************
// CanIf_InitController
//
// Desc.: This service executes CAN driver and CAN controller initialization.
//
// Parameters (in):	Controller 	Requested CAN controller
//			ConfigPtr 	Pointer to configuration setup
// Parameters (out):	None.
// ****************************************************************************
void CanIf_InitController ( uint8_t Controller, CanIf_ConfigType *ConfigPtr)
{
 Can_ControllerConfigType *Controller_ptr;
 CanIf_DeviceModeType ControllerMode;
 uint8_t i = 0;

 if (Can_Config.Controllers_ptr == (Can_ControllerConfigType *) NULL_PTR)
 {
	return;
 }
 if ( Controller >  MAX_CONFIG_CAN_CONTROLLERS)
	return;

 Controller_ptr = Can_Config.Controllers_ptr;

 while ( i <  MAX_CONFIG_CAN_CONTROLLERS)// Number of CAN controllers
 {
	if ( i == Controller)
	{
		ControllerMode = CanIf_GetControllerMode( (uint8_t) Controller_ptr->Controller_Number );
		if ( ControllerMode == CANIF_CS_UNINIT || ControllerMode == CANIF_CS_STOP)
		{
			Can_InitController (Controller, Controller_ptr);
			CanIf_ControllerStatus[i] = CANIF_CS_STOP;
		}
		break;
	}
	Controller_ptr++;
	i++;
 }
}

// ****************************************************************************
// CanIf_SetControllerMode
//
// Desc.: Set control Mode
//
// Parameters (in):	Controller		Controller
//			RequestMode 		Requested mode transition
//
// Parameters (out):	Std_ReturnType Driver CAN state
// ****************************************************************************
Std_ReturnType CanIf_SetControllerMode(uint8_t Controller, CanIf_DeviceModeType	RequestMode)
{
 Std_ReturnType ReturnValue;
 CanIf_DeviceModeType ActualMode;
 Can_ControllerConfigType *Controller_ptr;
 uint8_t i;

 ReturnValue = E_NOT_OK;
 // All the  states are: CANIF_CS_UNINIT, CANIF_CS_INIT, CANIF_CS_STOP, CANIF_CS_RUNNING, CANIF_CS_SLEEP
 i = 0;
 Controller_ptr = Can_Config.Controllers_ptr;
 while ( i <  MAX_CONFIG_CAN_CONTROLLERS)// Number of CAN controllers
 {
	if ( i == Controller)
		break;

	Controller_ptr++;
	i++;
 }

 // Error
 if (i == MAX_CONFIG_CAN_CONTROLLERS)
	return(ReturnValue); // Error

 ActualMode = CanIf_GetControllerMode( (uint8_t) Controller_ptr->Controller_Number );

 if(RequestMode == CANIF_CS_RUNNING)
 {
	switch (ActualMode)
	{
		case CANIF_CS_SLEEP:
		 	ReturnValue = Can_SetControllerMode(Controller, CAN_T_STOP);
			CanIf_ControllerStatus[i] = CANIF_CS_STOP;
		break;

		case CANIF_CS_UNINIT:
			Can_InitController (Controller, Controller_ptr);
			CanIf_ControllerStatus[i] = CANIF_CS_STOP;
		 	ReturnValue = Can_SetControllerMode(Controller, CAN_T_STOP);
		break;

		default:
		break;
	}
	ReturnValue = Can_SetControllerMode (Controller, CAN_T_START);
 }
 else if(RequestMode == CANIF_CS_SLEEP)
 {
	switch (ActualMode)
	{
		case CANIF_CS_RUNNING:
		 	ReturnValue = Can_SetControllerMode (Controller, CAN_T_STOP);
			CanIf_ControllerStatus[i] = CANIF_CS_STOP;
		break;

		case CANIF_CS_UNINIT:
			Can_InitController (Controller, Controller_ptr);
			CanIf_ControllerStatus[i] = CANIF_CS_STOP;
		 	ReturnValue = Can_SetControllerMode (Controller, CAN_T_STOP);
		break;

		default:
		break;
	}
	ReturnValue = Can_SetControllerMode (Controller, CAN_T_SLEEP);
 }
 else if(RequestMode == CANIF_CS_STOP)
 {
	ReturnValue = Can_SetControllerMode (Controller, CAN_T_STOP);
 }
 else if(RequestMode == CANIF_CS_UNINIT)
	ReturnValue = E_NOT_OK;
 else if(RequestMode == CANIF_CS_INIT)
	ReturnValue = E_NOT_OK;
 else
	ReturnValue = E_NOT_OK;

 // Check return value
 if ( ReturnValue == E_NOT_OK)
	ReturnValue = E_NOT_OK;
 else if ( ReturnValue == CAN_OK)
 {
	ReturnValue = E_OK;
	CanIf_ControllerStatus[Controller] = RequestMode;
 }
 else
	ReturnValue = E_NOT_OK;

 return(ReturnValue);
}

// ****************************************************************************
// CanIf_GetControllerMode
//
// Desc.: Service reports about the current status of the CAN driver and CAN
//   	  controller.
//
// Parameters (in)	:	Controller 		Symbolic name of the CAN Controller
// Parameters (out)	:	CanIf_DeviceModeType	Current status of the controller.
// ****************************************************************************
CanIf_DeviceModeType CanIf_GetControllerMode( uint8_t Controller)
{
 CanIf_DeviceModeType ReturnValue;

 if ( Controller >  MAX_CONFIG_CAN_CONTROLLERS)
	return(CANIF_CS_UNINIT);

 ReturnValue = CanIf_ControllerStatus[Controller];

 return(ReturnValue);
}

// ****************************************************************************
// CanIf_MainFunction
//
// Desc.: This service is used for processing the polling mode.
//
// Parameters (in)	:	None.
// Parameters (out)	:	None.
// ****************************************************************************
void CanIf_MainFunction(void)
{

}


// ****************************************************************************
// CanIf_Transmit
//
// Desc.: This service initiates a request for transmission of the CAN L-PDU
//	  specified by the CanTxPduId and CAN related data in the L-PDU structure.
//
// Parameters (in)	:	CanTxPduId 	ID of CAN L-PDU to be transmitted.
//						In this case CanTxPduId rappresents the index of the array
// 						"CanTxPduHd" for Basic or Full CAN mode
//				*PduInfoPtr 	Pointer to a structure with CAN L-PDU
// Parameters (out)	:	Std_ReturnType	Net mode request has been accepted
// ****************************************************************************
Std_ReturnType CanIf_Transmit(PduIdType CanTxPduId, Can_PduType *PduInfoPtr)
{
 uint8_t i;
 Std_ReturnType ReturnValue = E_OK;
 Can_ReturnType ReturnWrite;
 uint8_t Controller = 0;
 uint8_t NetworkID;
 uint8_t Handle;
 Can_HwHandleType MessageObject;
 Can_Mode CAN_Mode;

 // Start transmission
 MessageObject = (Can_HwHandleType) CanIf_HTH(PduInfoPtr->Id);

 // Message Object Basic or Full CAN
 CAN_Mode = CanIf_CanMode(MessageObject);

 //*//
 if (CAN_Mode == FULL_CAN)
 {
 	if (CanIf_Config.CanTxPduHd[CanTxPduId].ID != PduInfoPtr->Id)
 		return( E_NOT_OK );
 	else
 	{
		NetworkID = CanIf_Config.CanTxPduHd[CanTxPduId].Network;
		Handle = CanIf_Config.CanTxPduHd[CanTxPduId].CanTxPduId;
 	}
 }
 else
 {
	NetworkID = CanIf_Config.CanTxPduHd[CanTxPduId].Network;
	Handle = CanIf_Config.CanTxPduHd[CanTxPduId].CanTxPduId;
 }

 // Definition of the CAN controller to be used
 for(i = 0; i < CanIf_Config.NumberNetwork; i++)
 {
	if (NetworkID == CanIf_Config.Network[i].Network)
	{
		Controller = CanIf_Config.Network[i].NetworkController;
		break;
	}
 }

 if(CanIf_ControllerStatus[Controller] == CANIF_CS_STOP)
 	return(E_NOT_OK);

 ReturnWrite = Can_Write (Controller,  MessageObject, PduInfoPtr, CanTxPduId );

 if (ReturnWrite == E_OK)
 	return(ReturnValue);

 if (CanIf_Config.TransmitQueueType == NO)
 	return(ReturnValue);

 if ( ReturnWrite == CAN_BUSY)
 {
  	// Copying into the buffer (Queue)
	if (InsertPduQueue(Handle, PduInfoPtr, Controller) == E_OK)
		ReturnValue = E_OK;
	else
		ReturnValue = E_NOT_OK;
 }

return(ReturnValue);
}

// ****************************************************************************
// CanIf_RxIndication
//
// Desc.: This servive is called by the CAN driver after a CAN frame has been received.
///
// Parameters (in)	:	Hrh 		ID of the corresponding hardware object
//				CanId 		ID of CAN frame that has been successfully received
//				CanDlc 		Data length code (length of CAN frame payload)
//				*CanSduPtr	Pointer to received L-SDU (payload)
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_RxIndication(uint8_t Hrh, Can_IdType CanId, uint8_t CanDlc, uint8_t *CanSduPtr)
{
 uint8_t Handle;
 uint8_t i;
 Can_Mode CAN_Mode;

 // Message Object Basic or Full CAN
 CAN_Mode = CanIf_CanMode(Hrh);

 Handle = CanIf_Config.CanRxPduHd[Hrh-MAX_TX_OBJECTS].CanRxPduId;
// DlcPDU = CanIf_Config.CanRxPduHd[Hrh-MAX_TX_OBJECTS].DLC;

 // Software Filtering (only BasicCAN)
 if (CAN_Mode == BASIC_CAN)
 {
	if ((CanIf_Config.SoftwareReceiveFilter[Handle].CanIf_SoftwareFilterAlgorithms(CanId)) == DISPATCH_NOT_OK)
	{
#ifdef UTEST
  		for (i=0; i<8; i++)
		DataSdu[i] = 3;
#endif
		return;
	}
 }
 else	// Full CAN
 {
	if (CanId != CanIf_Config.CanRxPduHd[Hrh-MAX_TX_OBJECTS].ID)
	{
#ifdef UTEST
  		for (i=0; i<8; i++)
		DataSdu[i] = 1;
#endif
		return;
	}
 }

 // DLC check (optionally)
 if (CanIf_Config.DLC_Check == true)
 {
	if (CanIf_Config.DlcErrorNotification[Handle].CanIf_UserDlcErrorNotification(CanId, CanDlc) == DLC_E_FAILED)
	{
#ifdef UTEST
 		if (CAN_Mode == BASIC_CAN)
		{
		  for (i=0; i<8; i++)
			DataSdu[i] = 3;
		}
		else
		{
		  for (i=0; i<8; i++)
			DataSdu[i] = 2;
		}
#endif
		return;
	}
 }

 //  Dispatcher
 CanIf_Config.UsersUpperLayer[Handle].CanIf_UserRxIndication(CanId, CanSduPtr);
}

// ****************************************************************************
// CanIf_TxConfirmation
//
// Desc.: This service is called by the CAN driver after the CAN frame has been
//	  transmitted on the CAN network.
//	  specified by the CanTxPduId and CAN related data in the L-PDU structure.
//
// Parameters (in)	:	CanTxPduId 		ID of CAN L-PDU to be transmitted.
//							In this case CanTxPduId rappresents the index of the array
// 							"CanTxPduHd" that has been passed with CanIf_Transmit
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_TxConfirmation( PduIdType CanTxPduId)
{
 Can_ReturnType ReturnWrite;
 Std_ReturnType ReturnValue;
 uint8_t NetworkID = 0;
 uint8_t Controller;
 uint8_t i, Handle;
 uint8_t Index = 0;
 Can_HwHandleType MessageObject;

 if (CanIf_Config.TransmitQueueType == NO)
 {
 	Handle = CanIf_Config.CanTxPduHd[CanTxPduId].CanTxPduId;
 	CanIf_Config.UsersUpperLayer[Handle].CanIf_UserTxConfirmation(CanTxPduId);
	return;
 }

 // The transmit queue of the CAN Interface checked, whether a pending L-PDU is stored or not.
 for (i=0; i<MAX_CONFIG_CAN_CONTROLLERS; i++)
 {
 	Handle = CanIf_TxQueueTreatment(i);
	if (IfPduInfo.Id != 100)
	{
#ifdef UTEST
  	Utest = Utest + 1;
#endif
		break;
	}
 }

 if (IfPduInfo.Id != 100)
 {
 	// Found ID
	for (i=0; i < MAX_TX_PDU; i++)
 	{
		if (IfPduInfo.Id == CanIf_Config.CanTxPduHd[i].ID)
		{
#ifdef UTEST
  	Utest = Utest + 2;
#endif
			Handle = CanIf_Config.CanTxPduHd[i].CanTxPduId;
			NetworkID = CanIf_Config.CanTxPduHd[i].Network;
			Index = i;
			break;
		}
	}

	// if Don't found the ID is basic mode and found Handle
	if (i == MAX_TX_PDU)
	{
	 	// Found Handle
		for (i=0; i < MAX_TX_PDU; i++)
	 	{
			if (Handle == CanIf_Config.CanTxPduHd[i].CanTxPduId)
			{
				NetworkID = CanIf_Config.CanTxPduHd[i].Network;
				Index = i;
				break;
			}
		}
	}

	// In case of pending L-PDUs in the transmit queue the highest priority order the latest
	Controller = CanIf_FoundController(NetworkID);

 	// Start transmission
	MessageObject = (Can_HwHandleType) CanIf_HTH(IfPduInfo.Id);

 	// Start transmission
	ReturnWrite = Can_Write (Controller, MessageObject, (Can_PduType *) &IfPduInfo, Index );

 	if ( ReturnWrite == E_OK)
 	{
#ifdef UTEST
  	Utest = Utest + 4;
#endif
 	}
 	else if ( ReturnWrite == CAN_BUSY)
 	{
 		// Copying into the buffer (Queue)
		if (InsertPduQueue(Handle, (Can_PduType *) &IfPduInfo, Controller) == E_OK)
			ReturnValue = E_OK;
		else
			ReturnValue = E_NOT_OK;
        (void)ReturnValue;
#ifdef UTEST
  	Utest = Utest + 8;
#endif
 	}
 }

 Handle = CanIf_Config.CanTxPduHd[CanTxPduId].CanTxPduId;
 CanIf_Config.UsersUpperLayer[Handle].CanIf_UserTxConfirmation(CanTxPduId);
#ifdef UTEST
  	Utest = Utest + 32;
#endif
}



// ****************************************************************************
// CanIf_ControllerBusOff
//
// Desc.: This service indicates a CAN controller BusOff event referring to the
//   	  correspon,ding CAN controller.
//
// Parameters (in)	:	Controller 	Requested CAN controller
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_ControllerBusOff( uint8_t Controller)
{
	// Init
	CanIf_Init(NULL, Can_Config.input_clock);

	//  Dispatcher
 	CanIf_Config.UsersController[Controller].CanIf_UserControllerBusOff(Controller);
}

// ****************************************************************************
// CanIf_ControllerWakeup
//
// Desc.: Service indicates a wake up event initiated from the bus
//
//
// Parameters (in)	:	Controller 	Requested CAN controller
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_ControllerWakeup( uint8_t Controller)
{
	//  Dispatcher
 	CanIf_Config.UsersController[Controller].CanIf_UserControllerWakeup(Controller);

	CanIf_ControllerStatus[Controller] = CANIF_CS_STOP;
	//Can_SetControllerMode (Controller, (Can_StateTransitionType)CANIF_CS_STOP);
	CanIf_SetControllerMode( Controller, CANIF_CS_STOP);
}

// ****************************************************************************
// CanIf_SetChannelMode
//
// Desc.: This service initiates a request for transmission of the CAN L-PDU
//	  specified by the CanTxPduId and CAN related data in the L-PDU structure.
//
// Parameters (in)	:	Channel 		Predefined physical channel
//				ChannelMode 		Requested mode
// Parameters (out)	:	Std_ReturnType	Net mode request has been accepted
// ****************************************************************************
Std_ReturnType CanIf_SetChannelMode( uint8_t Channel, CanIf_ChannelModeType ChannelMode)
{
    Std_ReturnType ReturnValue;

    /// To be defined
    /* Make compiler happy */
    ReturnValue = E_OK;

    return(ReturnValue);
}

// ****************************************************************************
// CanIf_GetChannelMode
//
// Desc.: This service initiates a request for transmission of the CAN L-PDU
//	  specified by the CanTxPduId and CAN related data in the L-PDU structure.
//
// Parameters (in)	:	Channel 		Predefined physical channel
// Parameters (out)	:	CanIf_ChannelModeType	Net mode request has been accepted
// ****************************************************************************
CanIf_ChannelModeType CanIf_GetChannelMode( uint8_t Channel)
{
    CanIf_ChannelModeType ReturnValue;

    /// To be defined
    /* Make compiler happy */
    ReturnValue = E_OK;

    return(ReturnValue);
}

// ****************************************************************************
// InsertPduQueue
//
// Desc.: This service puts in the queue a request for transmission of the CAN L-PDU
//	  specified by the CanTxPduId and CAN related data in the L-PDU structure.
//
// Parameters (in)	:	CanTxPduId 	ID of CAN L-PDU to be transmitted.
//				*PduInfoPtr 	Pointer to a structure with CAN L-PDU
// Parameters (out)	:	Std_ReturnType	Net mode request has been accepted
// ****************************************************************************
Std_ReturnType InsertPduQueue(PduIdType CanTxPduId, Can_PduType *PduInfoPtr, uint8_t Controller)
{
  bool InserPdu = false;
  uint8_t i, j;
  uint16_t	QueueSize;
  uint16_t	FifoSize;

  QueueSize = CanIf_Config.TransmitQueueSize;
  for (i=0; i<QueueSize; i++)
  {
	// Found Handle
	if (CanIf_Config.TransmitQueue[i].CanTxPduId == CanTxPduId)
	{
		if(CanIf_Config.TransmitQueue[i].QueueElementStatus == EMPTY)
		{
			// Disable Interrupts
			CCanLLD_DisableControllerInterrupts (Controller);
			// Insert in array
			CanIf_Config.TransmitQueue[i].QueueElementStatus = FULL;
			CanIf_Config.TransmitQueue[i].ID = PduInfoPtr->Id;
			CanIf_CopySdu(PduInfoPtr->Sdu, CanIf_Config.TransmitQueue[i].CanTxPduSdu, CAN_SDU_SIZE);
			InserPdu = true;
			// Enable Interrupts
			CCanLLD_EnableControllerInterrupts (Controller);
			break;
		}
		else
		{
			FifoSize = CanIf_Config.TransmitQueue[i].CanTxPduIdQueueSize;
			for (j=0; j<FifoSize; j++)
			{
				if(CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].ElementStatusFIFO == EMPTY)
				{
					// Disable Interrupts
					CCanLLD_DisableControllerInterrupts (Controller);
					// Insert in FIFO
					CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].ID = PduInfoPtr->Id;
					CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].ElementStatusFIFO = FULL;
					CanIf_CopySdu(PduInfoPtr->Sdu, CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].CanTxPduSduFIFO, CAN_SDU_SIZE);
					InserPdu = true;
					// Enable Interrupts
					CCanLLD_EnableControllerInterrupts (Controller);
					break;
				}
			}
		}
		break;
	}
  }

  if (InserPdu == true)
	return(E_OK);
  else
	return(E_NOT_OK);
}

// ****************************************************************************
// CanIf_RxPduFilter
//
// Desc.: This service performs the SW filtering and CAN DLC check for the received PDU.
//
//
// Parameters (in)	:	Controller 	Requested CAN controller
//				Hrh 		ID of the corresponding hardware object
//				CanId 		ID of CAN frame that has been successfully received
//				CanDlc 		Data length code (length of CAN frame payload)
//
// Parameters (out)	:	index		index of array CanRxPduHd that contain ID of CAN L-PDU that has been received in BasicCAN
//						and which was found by filtering. Identifies the data that has been received.
//						Range: 0..(maximum number of L-PDU IDs received by upper layers - 1)
// ****************************************************************************
int CanIf_RxPduFilter(uint8_t Controller, uint8_t Hrh, Can_IdType CanId, uint8_t CanDlc)
{
 int i;

 // Software Filtering (only BasicCAN)
  for(i=0; i < MAX_RX_PDU; i++)
  {
	if (CanId == CanIf_Config.CanRxPduHd[i].ID)
		return(i);
  }
  return(-1);
}

// ****************************************************************************
// CanIf_TxQueueTreatment
//
// Desc.: This service is used by the CAN driver for treating the transmit queue.
// 	  If available the next pending transmit request (FIFO or highest priority)
//	  is transmitted and then taken out of the queue. This service is called by
//	  the CAN driver only in the transmit confirmation.
//	  In case of multiple controller usage this service must be re-entrant.
//
//
// Parameters (in)	:	Controller 	Requested CAN controller
//
// Parameters (out)	:
// ****************************************************************************
uint8_t CanIf_TxQueueTreatment( uint8_t Controller)
{
 uint8_t i;
 uint16_t	QueueSize;
 uint8_t  PDU_Handle = 0;

  IfPduInfo.Id = 100;
  QueueSize = CanIf_Config.TransmitQueueSize;
  for (i=0; i<QueueSize; i++)
  {
	if(CanIf_Config.TransmitQueue[i].QueueElementStatus == FULL)
	{
		// Disable Interrupts
		CCanLLD_DisableControllerInterrupts (Controller);
		CanIf_CopySdu(CanIf_Config.TransmitQueue[i].CanTxPduSdu, IfPduInfo.Sdu, CAN_SDU_SIZE);
 	 	IfPduInfo.Id = CanIf_Config.TransmitQueue[i].ID;
		PDU_Handle = CanIf_Config.TransmitQueue[i].CanTxPduId;
		CanIf_Config.TransmitQueue[i].QueueElementStatus = EMPTY;

		if (CanIf_Config.TransmitQueue[i].CanTxPduIdQueueSize != 0)
		{
			if (CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[0].ElementStatusFIFO == FULL)
			{
				CanIf_CopySdu(CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[0].CanTxPduSduFIFO, CanIf_Config.TransmitQueue[i].CanTxPduSdu, CAN_SDU_SIZE);
				CanIf_Config.TransmitQueue[i].QueueElementStatus = FULL;
				CanIf_Config.TransmitQueue[i].ID = CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[0].ID;
				CanIf_ShiftSxSduFIFO(CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO, CanIf_Config.TransmitQueue[i].CanTxPduIdQueueSize);
			}
		}
		// Enable Interrupts
		CCanLLD_EnableControllerInterrupts (Controller);
	}
	break;
  }
  return (PDU_Handle);
}



// ****************************************************************************
// CanIf_FoundController
//
// Desc.: This service receve the PDU end return the Controller
//
// Parameters (in)	:	NetworkID 	ID of the network
//
// Parameters (out)	:	unit8		Controller
// ****************************************************************************
uint8_t CanIf_FoundController(uint8_t NetworkID)
{
 uint8_t Controller = 0;
 uint8_t i;

  // Definition of the CAN controller to be used
 for(i = 0; i < CanIf_Config.NumberNetwork; i++)
 {
	if (NetworkID == CanIf_Config.Network[i].Network)
	{
		Controller = CanIf_Config.Network[i].NetworkController;
		break;
	}
 }

 return(Controller);
}



// ****************************************************************************
// CanIf_InitSdu
//
// Desc.: Initialization SDU
//
// Parameters (in)	:	Buffer  	Buffer
//				Size		Size
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_InitSdu(uint8_t *Buffer, uint8_t Size)
{
	uint8_t i;

	for (i=0; i<Size; i++)
		Buffer[i] = 0;
}


// ****************************************************************************
// CanIf_CopySdu
//
// Desc.: Copy SDU
//
// Parameters (in)	:	Source  	Source Buffer
// 				Destination  	Destination Buffer
//				Size		Size
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_CopySdu(uint8_t *Source, uint8_t *Destination, uint8_t Size)
{
	uint8_t i;

	for (i=0; i<Size; i++)
		Destination[i] = Source[i];
}

// ****************************************************************************
// CanIf_ShiftSxSduFIFO
//
// Desc.: Copy SDU	Shift to left all element FIFO
//
// Parameters (in)	:	Source  	Source Buffer
// 				Destination  	Destination Buffer
//				SizeFIFO	Size FIFO
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_ShiftSxSduFIFO(CanIf_TransmitPduFIFO *PduFIFO, uint8_t SizeFIFO)
{
 uint8_t i;

 if (PduFIFO[0].ElementStatusFIFO == EMPTY)
	return;

 for (i=1; i<SizeFIFO; i++)
 {
	PduFIFO[i-1].ElementStatusFIFO = PduFIFO[i].ElementStatusFIFO;
	CanIf_CopySdu(PduFIFO[i].CanTxPduSduFIFO, PduFIFO[i-1].CanTxPduSduFIFO, CAN_SDU_SIZE);
	CanIf_Config.TransmitQueue[i-1].CanTransmitPduFIFO[0].ID = CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[0].ID;
	if (PduFIFO[i].ElementStatusFIFO == EMPTY)
		return;
 }

 // Reset last element
 PduFIFO[SizeFIFO-1].ElementStatusFIFO = EMPTY;
 CanIf_InitSdu(PduFIFO[SizeFIFO-1].CanTxPduSduFIFO, CAN_SDU_SIZE);
}

// ****************************************************************************
// CanIf_ResetQueueAndFIFO
//
// Desc.: Copy SDU	Reset Queue e FIFO
//
// Parameters (in)	:	None
// Parameters (out)	:	None
// ****************************************************************************
void CanIf_ResetQueueAndFIFO(void)
{
 uint8_t i, j;
 uint16_t	QueueSize;
 uint16_t	FifoSize;

 // Queue and FIFO Initializatin
 QueueSize = CanIf_Config.TransmitQueueSize;
 for (i=0; i<QueueSize; i++)
 {
	CanIf_Config.TransmitQueue[i].QueueElementStatus = EMPTY;
	CanIf_Config.TransmitQueue[i].ID = 0x0;
	// Init SDU
	CanIf_InitSdu(CanIf_Config.TransmitQueue[i].CanTxPduSdu, CAN_SDU_SIZE);

	FifoSize = CanIf_Config.TransmitQueue[i].CanTxPduIdQueueSize;
	for (j=0; j<FifoSize; j++)
	{
		CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].ElementStatusFIFO = EMPTY;
		CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].ID = 0x0;
		// Init SDU
		CanIf_InitSdu(CanIf_Config.TransmitQueue[i].CanTransmitPduFIFO[j].CanTxPduSduFIFO, CAN_SDU_SIZE);
	}
 }
}


// ****************************************************************************
// CanIf_FoundUser
//
// Desc.: This service receve the PDU end return the users
//
// Parameters (in)	:	CanTxPduId 	ID of CAN L-PDU to be transmitted.
//				Tx_RX           0 TX, 1 RX
//
// Parameters (out)	:	unit8		Index Array Users
// ****************************************************************************
uint8_t CanIf_FoundUser(uint8_t Handle, uint8_t Tx_Rx)
{
 uint8_t NumberUsers;
 uint8_t User;
 uint8_t i;

 User = 100;

 NumberUsers = CanIf_Config.NumberUsers;
 for (i=0; i < NumberUsers; i++)
 {
	if (Tx_Rx == 0)
	{
		if (Handle == CanIf_Config.UsersUpperLayer[i].CanTxPduId)
			return(i);
	}
	else
	{
		if (Handle == CanIf_Config.UsersUpperLayer[i].CanRxPduId)
			return(i);
	}
 }

 return(User);
}


// ****************************************************************************
// CanIf_CanMode
//
// Desc.: Determines the mode FULL and BASIC
//
// Parameters (in) :	ID of the corresponding hardware object
//			Range: 0..(total number of Hardware Object Handles - 1)
//
// Parameters (out):	BASIC_CAN or FULL_CAN
// ****************************************************************************
Can_Mode CanIf_CanMode( uint8_t Hrh )
{
	Can_HW_Object	*HW_Object_ptr;
	uint8_t i;

	HW_Object_ptr = Can_Config.HW_Object;
	for(HW_Object_ptr = Can_Config.HW_Object, i = 0; i < Can_Config.Objects_num ; i++, HW_Object_ptr++ )// Number of Hardware objects
	{
		if (HW_Object_ptr->Handle_number == Hrh)
		{
			switch(HW_Object_ptr->Type)
			{ // Operative Mode
				case TX_Basic:
					return(BASIC_CAN);
				//break;

				case TX_Full:
					return(FULL_CAN);
				//break;

				case RX_Basic:
					return(BASIC_CAN);
				//break;

				case RX_Full:
					return(FULL_CAN);
				//break;

				default:
	 			break;
			}
		}
	}
	return(BASIC_CAN);
}



// ****************************************************************************
// CanIf_HTH
//
// Desc.: Determines handler of the ID
//
// Parameters (in) :	Can_IdType		Id_Value ID of CAN frame that has been successfully transmited
//
// Parameters (out):	Handle of the corresponding hardware object
//			Range: 0..(total number of Hardware Object Handles - 1)
// ****************************************************************************
uint8_t CanIf_HTH( Can_IdType Id_Value )
{
 uint8_t i;
 Can_HwHandleType MessageObject;
 Can_Filter_Mask *Object_Filters_Config;
 Can_HW_Object	*HW_Objects_Config;

 MessageObject = 0;
 Object_Filters_Config 	= Can_Config.Object_Filter_Mask;
 HW_Objects_Config	= Can_Config.HW_Object;

 for(i=0; i< MAX_HW_OBJECTS; i++)
 {
 	if(HW_Objects_Config[i].Id_Value == Id_Value)
	{
//			controller= Object_Filters_Config[HW_Objects_Config[i].Associated_Mask_number - 1].Associated_Controller;
		MessageObject= HW_Objects_Config[i].Handle_number;
		return (MessageObject);
 	}
 }

 for(i=0; i< MAX_HW_OBJECTS; i++)
 {
 	if(HW_Objects_Config[i].Id_Value <= Id_Value && (HW_Objects_Config[i].Id_Value + ~Object_Filters_Config[HW_Objects_Config[i].Associated_Mask_number - 1].Filter_Mask_Value) >= Id_Value)
	{
//		controller= Object_Filters_Config[HW_Objects_Config[i].Associated_Mask_number - 1].Associated_Controller;
		MessageObject= HW_Objects_Config[i].Handle_number;
	}
 }

 return (MessageObject);
}

