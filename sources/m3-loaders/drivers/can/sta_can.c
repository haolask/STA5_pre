/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_can.c
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN driver routines
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

#include "sta_map.h"
#include "sta_nvic.h"
#include "sta_can.h"
#include "sta_canif.h"
#include "sta_canif_cfg.h"
#include "sta_canif_types.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"
#include "sta_common.h"
#include "sta_pinmux.h"

#include "sta_can_irq.c"

Can_GlobalConfigData 		GlobalConfigData;
ControllerMO_private 		ControllerMO[MAX_CONFIG_CAN_CONTROLLERS];
HandlerMO_private		    HandlerMO[MAX_HANDLES + 1];
Can_ControllerConfigType 	*Can_CtrlConfigData[MAX_CONFIG_CAN_CONTROLLERS + 1];

/*****************************************************************************
   external declarations
*****************************************************************************/
extern          Can_ConfigType  Can_Config;
extern          Can_HW_Object	HW_Objects_Config[];
extern const	Can_Filter_Mask	Object_Filters_Config[];

/*****************************************************************************
   function prototypes (scope: module-local)
*****************************************************************************/
void NM_A_TxConfirmation(PduIdType CanTxPduId);
void NM_A_RxIndication(Can_IdType CanRxPduId, const uint8_t *CanSduPtr);
void NM_A_ControllerBusOff(uint8_t Controller);
void NM_A_ControllerWakeup(uint8_t Controller);
void NM_B_TxConfirmation(PduIdType CanTxPduId);
void NM_B_RxIndication(Can_IdType CanRxPduId, const uint8_t *CanSduPtr);
void NM_B_ControllerBusOff(uint8_t Controller);
void NM_B_ControllerWakeup(uint8_t Controller);

PDU_Dispatcher                  NM_A_FilterSwRX(Can_IdType CanRxPduId);
PDU_Dispatcher                  NM_B_FilterSwRX(Can_IdType CanRxPduId);
CanIf_DlcErrorReturnType        NM_A_DlcErrorNotification( PduIdType CanRxPduId, uint8_t CanDlc);
CanIf_DlcErrorReturnType        NM_B_DlcErrorNotification( PduIdType CanRxPduId, uint8_t CanDlc);
extern CanIf_DeviceModeType     CanIf_ControllerStatus[MAX_CONFIG_CAN_CONTROLLERS];

extern void ipc_test_tx_msg(char data);

/*****************************************************************************
   defines and macros (scope: module-local)
*****************************************************************************/
const Can_ControllerConfigType Controllers_ptr_Config[] =
{
	// CAN CONTROLLER CONFIGURATION
	{   (uint8_t)0,			    // Controller_Number
        CCAN_CONTROLLER_0,  // Controller_Symbolic_Name phys. address of controller
        CAN_BAUDRATE500kB,	// Baudrate 500 kB/s
        90,	                // Allowed_Tollerance !!!
        125,	            // Propagation_delay !!!
        1000,	            // Tseg1 !!!
        500
    },
	{   (uint8_t)1,			    // Controller_Number
        CCAN_CONTROLLER_1,  // Controller_Symbolic_Name phys. address of controller
        CAN_BAUDRATE500kB,	// Baudrate 500 kB/s
        90,	                // Allowed_Tollerance !!!
        125,	            // Propagation_delay !!!
        1000,	            // Tseg1 !!!
        500
    }
};

// PLEASE USE CASTING TO PREVENT COMPILER AUTOMATIC RECASTING
const	Can_Filter_Mask	Object_Filters_Config[] =
{
    // Filter Mask 1 for Controller 0
    {   (uint8_t)1, 	    // Mask no.
        (uint8_t)0,	    // Asociated_Controller
        0x00000000, //Filter_Mask_Value.
    },
	// Filter Mask 2 for Controller 0
    {   (uint8_t)2, 	    // Mask no.
        (uint8_t)0,	    // Asociated_Controller
        0x00000000, // Filter_Mask_Value
    },
	// Filter Mask 3 for Controller 0
    {   (uint8_t)3, 	    // Mask no.
        (uint8_t)0,	    // Asociated_Controller
        0x00000000, // Filter_Mask_Value
    },
	// Filter Mask 4 for Controller 0
    {   (uint8_t)4, 	    // Mask no.
        (uint8_t)0,	    // Asociated_Controller
        0x00000000 	// Filter_Mask_Value
    }
};

Can_HW_Object	HW_Objects_Config[] =
{
    // Can Hardware object 1  for Controller 0
    {   (uint8_t)1,	    // Associated_Mask_number
        (uint8_t)0,	    // Handle_number (just to be sure, that is setted somehow)
        TX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x400,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 2  for Controller 0
    {   (uint8_t)1,	    // Associated_Mask_number
        (uint8_t)1,	    // Handle_number (just to be sure, that is setted somehow)
        TX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x500,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 3  for Controller 1
    {   (uint8_t)2,	    // Associated_Mask_number
        (uint8_t)2,	    // Handle_number (just to be sure, that is setted somehow)
        TX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x600,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 4  for Controller 1
    {   (uint8_t)2,	    // Associated_Mask_number
        (uint8_t)3,	    // Handle_number (just to be sure, that is setted somehow)
        TX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x700,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 5 for Controller 0
    {   (uint8_t)1,	    // Associated_Mask_number
        (uint8_t)4,	    // Handle_number (just to be sure, that is setted somehow)
        RX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x00,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 6  for Controller 0
    {   (uint8_t)1,	    // Associated_Mask_number
        (uint8_t)5,	    // Handle_number (just to be sure, that is setted somehow)
        RX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x100,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 7 for Controller 1
    {   (uint8_t)2,	    // Associated_Mask_number
        (uint8_t)6,	    // Handle_number (just to be sure, that is setted somehow)
        RX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x200,		// Id_Value (should to be setted somehow)
        (uint8_t)8,	    // Dlc_Value (should to be setted somehow)
    },
    // Can Hardware object 8  for Controller 1
    {   (uint8_t)2,	    // Associated_Mask_number
        (uint8_t)7,	    // Handle_number (just to be sure, that is setted somehow)
        RX_Basic,	// Type;(Can_Operative_mode)
        STANDARD,	// Id_Type(Can_IdType);
        0x300,		// Id_Value (should to be setted somehow)
        (uint8_t)8	    // Dlc_Value (should to be setted somehow
    }
};

Can_ConfigType Can_Config =
{
	(uint8_t)1,			    // Number of CAN controllers //DF
	(uint8_t)8,				// Number of CAN Objects for whole CAN Driver
	STANDARD,			// Supported frame ID format (Standard/Extended)(Can_Id_Format)
	CAN_VENDOR_ID,		// Ventor_Id
	CAN_DEVICE_ID,		// Device_Id
	CAN_MODULE_ID,		// Module_Id
	CAN_MAJOR_VERSION,	// Major_Version
	CAN_MINOR_VERSION,	// Minor_Version
	CAN_PATCH_VERSION,	// Driver_Patch_Version

	// Pointer to the controllers config data structure
	// ----------------------------------------------------------
	(Can_ControllerConfigType *) &Controllers_ptr_Config,

	// Pointer to the filters config data structure
	// ----------------------------------------------------------
	(Can_Filter_Mask *)&Object_Filters_Config,

	// Pointer to the Objects config data structure
	// ----------------------------------------------------------
	(Can_HW_Object	*) &HW_Objects_Config,
	// Pointer to CallBack function Rx_Indication defined in Can Interface to notify upper layer
	// ----------------------------------------------------------
	&Can_RxIndication,
	// Pointer to CallBack function Tx_CnfOk defined in Can Interface to notify upper layer
	// ----------------------------------------------------------
	&Can_TxCnfOk,
	// Pointer to CallBack function TxCnfAbort defined in Can Interface to notify upper layer
	// ----------------------------------------------------------
	&Can_TxCnfAbort,
	// Pointer to CallBack function ControllerWakeup defined in Can Interface to notify upper layer
	// ----------------------------------------------------------
	&Can_ControllerWakeup,
	// Pointer to CallBack function ControllerBusOff defined in Can Interface to notify upper layer
	// ----------------------------------------------------------
	&Can_ControllerBusOff
};

/******************************************************* CONFIGURATION *********************************************************/
/*                                                                                                                             */
/*                                                         B A S I C                                                           */
/*                                                                                                                             */
/*******************************************************************************************************************************/


/*                                                                                 */
/*						CONFIGURATION CAN INTERFACE					               */
/*                                                                                 */
const	CanIf_Network	Can_Networks[] =
{
	{   (uint8_t)1, 			                        // Logical handle of CAN network
        (uint8_t)CAN_CONTROLER_NETWORK_SIMBOLIC_1,	// Name of CAN controller assigned to a CAN network
    },
	{   (uint8_t)2, 			                        // Logical handle of CAN network
	(uint8_t)CAN_CONTROLER_NETWORK_SIMBOLIC_2,	    // Name of CAN controller assigned to a CAN network
	},
	{   (uint8_t)3, 			                        // Logical handle of CAN network
        (uint8_t)CAN_CONTROLER_NETWORK_SIMBOLIC_3	// Name of CAN controller assigned to a CAN network
    }
};

CanIf_TransmitPduFIFO	CanIf_TransmitPdu_Handle_0[] =
{
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	}
};

CanIf_TransmitPduFIFO	CanIf_TransmitPdu_Handle_1[] =
{
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	},
	{   EMPTY,				// Status EMPTY or FULL
        0x10,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
	}
};

CanIf_TransmitQueue	CanIf_TxQueue[] =
{
	{   (uint8_t)0,			    // Locical HANDLE of CanTxPduID
        0x0,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
        EMPTY,				// Status EMPTY or FULL
		// Size of per Tx L-PDU transmit queue
        sizeof(CanIf_TransmitPdu_Handle_0) / sizeof(CanIf_TransmitPduFIFO),
        (CanIf_TransmitPduFIFO *) &CanIf_TransmitPdu_Handle_0,	// Pointer FIFO for Multiple elements
    },
	{   (uint8_t)1,			    // Locical HANDLE of CanTxPduID
        0x0,				// ID of CanTxPduID
        {0,0,0,0,0,0,0,0},	// Pointer to SDU
        EMPTY,				// Status EMPTY or FULL
		// Size of per Tx L-PDU transmit queue
        sizeof(CanIf_TransmitPdu_Handle_1) / sizeof(CanIf_TransmitPduFIFO),
        (CanIf_TransmitPduFIFO *) &CanIf_TransmitPdu_Handle_1	// Pointer FIFO for Multiple elements
    }
};

const CanIf_TxPduHandle CanIf_CanTxPduHandle[] =
{
	{   (uint8_t)0,			// Locical HANDLE of CanTxPduID
        0x00,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
    },
	{   (uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x401,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
    },
	{   (uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x402,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{   (uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x403,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{   (uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x404,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{   (uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x405,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{   (uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x406,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x407,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x408,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x409,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x40A,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x40B,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x40C,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x40D,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x40E,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	},
	{  	(uint8_t)1,			// Locical HANDLE of CanTxPduID
        0x40F,			// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
        STATIC,			// Type of CanTxPduId
	}
};

const CanIf_RxPduHandle CanIf_CanRxPduHandle[] =
{
	{   (uint8_t)0,			// Locical HANDLE of CanTxPduID
        0x00,				// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
    },
	{   (uint8_t)0,			// Locical HANDLE of CanTxPduID
        0x01,				// ID of CanTxPduID
        (uint8_t)8,			// DLC of CanTxPduId
        (uint8_t)1,			// CAN Network of CanTxPduId
    }
};

const CanIf_UsersUpperLayer CanIf_Users[] =
{
	{   "NM_A",
        (uint8_t)1,				// Locical HANDLE of CanTxPduID
        &NM_A_TxConfirmation,
        (uint8_t)1,				// Locical HANDLE of CanRxPduID
        &NM_A_RxIndication,
	},
	{   "NM_B",
        (uint8_t)2,				// Locical HANDLE of CanTxPduID
        &NM_B_TxConfirmation,
        (uint8_t)2,				// Locical HANDLE of CanRxPduID
        &NM_B_RxIndication,
    }
};

const CanIf_SoftwareFilterRX CanIf_FiterSwRX[] =
{
	{   &NM_A_FilterSwRX,
	},
	{   &NM_B_FilterSwRX,
	}
};

const CanIf_DlcErrorNotification CanIfDlcErrorNotification[] =
{
	{   &NM_A_DlcErrorNotification,
	},
	{   &NM_B_DlcErrorNotification,
	}
};

const CanIf_UsersController UsersController[] =
{
	{   &NM_A_ControllerBusOff,
        &NM_A_ControllerWakeup,
    },
	{   &NM_B_ControllerBusOff,
        &NM_B_ControllerWakeup,
	}
};

const  CanIf_ConfigType CanIf_Config =
{
	INTERRUPT, 				// Tx/Rx notification method
	2,					// SoftwareFilter;
	false,					// DLC Check support
	true,					// CAN controller wakeup used / not used
	YES,					// Transmission queue operation type
	1,					// Size of all single queue buffers
	(CanIf_TransmitQueue * )&CanIf_TxQueue,	// Transmit Queue
	(CanIf_TxPduHandle *) &CanIf_CanTxPduHandle,// PDU Handle TX
	(CanIf_RxPduHandle *) &CanIf_CanRxPduHandle,// PDU Handle RX
	(uint8_t)2,				// Number of Users
	(CanIf_UsersUpperLayer *) &CanIf_Users,	// Array of Users
	(uint8_t)3,				// Number of CAN networks
	(CanIf_Network *)&Can_Networks,		// Network
	(CanIf_SoftwareFilterRX *)&CanIf_FiterSwRX,	// Network
	(CanIf_DlcErrorNotification *) CanIfDlcErrorNotification,	// Software Receive Filter
	(CanIf_UsersController *) UsersController	// Software Receive Filter
};

/*****************************************************************************
   typedefs and structures (scope: module-local)
*****************************************************************************/

/*****************************************************************************
   global variable definitions  (scope: module-exported)
*****************************************************************************/
uint32_t next, ctrl = 1;

/*****************************************************************************
   global variable definitions  (scope: module-local)
*****************************************************************************/


/*****************************************************************************
   function implementations (scope: module-local)
*****************************************************************************/
void Can_TxCnfOk(PduIdType	SwPduHandle)
{

}

void Can_TxCnfAbort(PduIdType	SwPduHandle)
{

}

void Can_ControllerBusOff(uint8_t Controller)
{
    CCanLLD_SetControllerMode (Controller, CAN_T_START);
}

void Can_ControllerWakeup(uint8_t Controller)
{

}

CanIf_DlcErrorReturnType NM_A_DlcErrorNotification( PduIdType CanRxPduId, uint8_t CanDlc)
{
	return(DLC_OK);
}

CanIf_DlcErrorReturnType NM_B_DlcErrorNotification( PduIdType CanRxPduId, uint8_t CanDlc)
{
	return(DLC_OK);
}

void NM_A_TxConfirmation(PduIdType CanTxPduId)
{
	///CanIf_Config.CanTxPduHd[CanTxPduId].CanTxPduId;
}

void NM_A_RxIndication(Can_IdType CanRxPduId, const uint8_t *CanSduPtr)
{

}

void NM_B_TxConfirmation(PduIdType CanTxPduId)
{
	///CanIf_Config.CanTxPduHd[CanTxPduId].CanTxPduId;
}

void NM_B_RxIndication(Can_IdType CanRxPduId, const uint8_t *CanSduPtr)
{

}

void User_ControllerBusOff(uint8_t Controller)
{

}

void User_ControllerWakeup(uint8_t Controller)
{

}

void NM_A_ControllerBusOff(uint8_t Controller)
{

}

void NM_A_ControllerWakeup(uint8_t Controller)
{

}

void NM_B_ControllerBusOff(uint8_t Controller)
{

}

void NM_B_ControllerWakeup(uint8_t Controller)
{

}

PDU_Dispatcher NM_A_FilterSwRX(Can_IdType CanRxPduId)
{
	return(DISPATCH_OK);
}

PDU_Dispatcher NM_B_FilterSwRX(Can_IdType CanRxPduId)
{
	return(DISPATCH_OK);
}

void CAN_Engine_Start(RxIndication rxcb, uint32_t input_clock)
{
	struct nvic_chnl irq_chnl;
	uint8_t i;

	if (get_soc_id() != SOCID_STA1385)
		pinmux_request("can_mux");

	/* Enable CAN interrupt */
	irq_chnl.id                    = CAN0_IRQChannel;
	irq_chnl.preempt_prio  = IRQ_LOW_PRIO;
	irq_chnl.enabled                 = true;
	nvic_chnl_init(&irq_chnl);

	for (i = 0; i < MAX_CONFIG_CAN_CONTROLLERS; i++)
		CanIf_ControllerStatus[i] = CANIF_CS_UNINIT;

	CanIf_Init(rxcb, input_clock);

	for (i=0; i < MAX_CONFIG_CAN_CONTROLLERS; i++)
		CanIf_SetControllerMode(i, CANIF_CS_RUNNING);

	return;
}

// ****************************************************************************
// Name:        CCanLLD_InitializeControllerDefaultState
// Parameters:
//		CAN_ptr:       Can Controller Pointer
//              Controller:    CAN Controller 0,1,2
//              baudrate:      value to be put to BTR register
//                             (see CAN_GET_RELOAD_VALUE macro)
//              mode:          set different modes
//                             (see header file)
// Returns:
//              OK, FAIL_REGWRITE_CxCSR, FAIL_REGWRITE_CxBTR
// Description:
//              'mode' parametr sets bits IE, EIE, SIE, DAR, TEST, BASIC,
//              SILENT, LOOPBACK, TXCTRL, RXMON from CANxCR and CANxTR registers
//              All other bits are controlled automatically.
//              CCAN cell is connected to CAN network after calling this function.
// ****************************************************************************
uint16_t CCanLLD_InitializeControllerDefaultState(volatile CCan * CAN_ptr, uint8_t Controller, uint16_t baudrate, uint16_t mode)
{

        uint16_t i;

        //switch CAN0 to configuration mode
        CAN_ptr->CR.reg = CANC_INITIALIZATION_EN | CANC_CONFIG_CHANGE_EN;

        CAN_ptr->BTR.reg = baudrate;
        if (CAN_ptr->BTR.reg != (baudrate & 0x7FFF)) return FAIL_REGWRITE_C0BTR;

        //BRP Extension register set to 0
        CAN_ptr->BRP = 0;

        //set mode to Test register
        if (mode & 0x0FC0) CAN_ptr->CR.reg |= CANC_TEST_MODE_EN;
        CAN_ptr->TEST.reg = 0;
        if (mode & CAN_TSTMODE_BASIC)           {CAN_ptr->TEST.reg |= CAN_TSTMODE_BASIC    >> 4;}
        if (mode & CAN_TSTMODE_SILENT)          {CAN_ptr->TEST.reg |= CAN_TSTMODE_SILENT   >> 4;}
        if (mode & CAN_TSTMODE_LOOPBACK)        {CAN_ptr->TEST.reg |= CAN_TSTMODE_LOOPBACK >> 4;}

        if (mode & CAN_TSTMODE_TXCTRL_MONITOR)  {CAN_ptr->TEST.reg |= CAN_TSTMODE_TXCTRL_MONITOR  >> 4;}
        if (mode & CAN_TSTMODE_TXCTRL_DOMINANT) {CAN_ptr->TEST.reg |= CAN_TSTMODE_TXCTRL_DOMINANT >> 4;}
        if (mode & CAN_TSTMODE_TXCTRL_RECESIVE) {CAN_ptr->TEST.reg |= CAN_TSTMODE_TXCTRL_RECESIVE >> 4;}
        if (mode & CAN_TSTMODE_RXCTRL_RECESIVE) {CAN_ptr->TEST.reg |= CAN_TSTMODE_RXCTRL_RECESIVE >> 4;}

        //initialize all MOs - turn everything off
        CAN_ptr->IF1MASK1      = 0x0000;         // Clear arbitration masks
        CAN_ptr->IF1MASK2.reg  = CANC_COMPUTE_MASKSHORT(0xFFFF);
        CAN_ptr->IF1AR1        = 0x0000;         // Clear message ID's
        CAN_ptr->IF1AR2.reg    = CANC_MO_IF_ID_SHORT | CANC_MO_IF_DIRECTION_RX;
        CAN_ptr->IF1MCR.reg    = CANC_MO_IF_EOB | CANC_MO_IF_UMASK | CANC_MO_DATALENGTH_0;  // No FIFO
        CAN_ptr->IF1DATAA1.reg = 0x0000;         // Clear all data
        CAN_ptr->IF1DATAA2.reg = 0x0000;
        CAN_ptr->IF1DATAB1.reg = 0x0000;
        CAN_ptr->IF1DATAB2.reg = 0x0000;

        CAN_ptr->IF1CMR.reg    = CANC_IF_ALL;    // Write to all fields

        // Write to each object
        for( i = CAN_MO_1; i <= CAN_MO_32; i++ ){
            CAN_IF_WAIT(1);
            CAN_ptr->IF1CR.reg = i;
        }

        // Enable Pins for Controller
        CCanLLD_InitControllerPins (Controller);

        //connect to CAN bus (CCE = 0 and INIT = 0)
        CAN_ptr->CR.reg = mode & 0x000E;
        if (CAN_ptr->CR.reg != (mode & 0x000E)) return FAIL_REGWRITE_C0CSR;
            return CAN_OK;


}//CCanLLD_InitializeControllerDefaultState

// ****************************************************************************
// CCanLLD_Init
//
// Desc.: Driver Module Initialization.
//
// Parameters (in)	:	Can_ControllerConfigType Pointer to CAN controller
//				configuration set. This is a global configuration data.
// Parameters (out)	:	None.
// ****************************************************************************
void CCanLLD_Init(Can_ControllerConfigType * Controller_ptr)
{
	CCanLLD_InitController ((uint8_t) Controller_ptr->Controller_Number,Controller_ptr);
}

struct btr_cfg {
	uint32_t baudrate;
	uint32_t btr_val;
};

static const struct btr_cfg btr_cfgs_51_2mhz[] = {
	{ 250000, 0x228 },
	{ 500000, 0x310 },
	{ 0, 0},
};

static const struct btr_cfg btr_cfgs_52mhz[] = {
	{ 250000, 0x10c },
	{ 500000, 0x50c },
	{ 0, 0},
};

static uint32_t CCanLLD_Get_BTR(uint32_t baudrate, uint32_t input_clock)
{
	const struct btr_cfg *cfg;

	switch (input_clock) {
	case 51200000:
		cfg = btr_cfgs_51_2mhz;
		break;
	case 52000000:
		cfg = btr_cfgs_52mhz;
		break;
	default:
		TRACE_ERR("%s: unsupported input_clock: %d\n", __func__,
			  input_clock);
		return 0;
	}
	while (cfg->baudrate != 0) {
		if (cfg->baudrate == baudrate)
			return cfg->btr_val;
		cfg++;
	}
	TRACE_ERR("%s: unsupported baudrate: %d\n", __func__,
			  baudrate);
	return 0;
}

// ****************************************************************************
// CCanLLD_InitController
//
// Desc.: This routine re-initializes only CAN Controller specific settings.
//
//
// Parameters (in)	:	Controller  CAN Controller to be initialized
// 			:	Config	    Pointer to controller configuration
// Parameters (out)	: 	None
// Development E. Codes :	CAN_E_UNINIT: Driver not yet initialized
//				CAN_E_PARAM_CONFIG: Config is a null pointer
//				CAN_E_PARAM_CONTROLLER: Parameter Controller is out of range
//				CAN_E_TRANSITION: the controller is not 'stopped'
//
// ****************************************************************************
void CCanLLD_InitController (uint8_t Controller, Can_ControllerConfigType *Config)
{
	volatile CCan * CAN_ptr;
	uint8_t msg_obj, CountRx = 1;
	uint16_t BTR;


	 // checking if the pointer is not NULL
	if(NULL_PTR == Config)
	{
		return;
	}

	// Checking of critical parameters
	// checking of address of controller
	if(false == CCanLLD_CheckController(Controller))
	{
		return;
	}


	CAN_ptr = NULL_PTR;
	switch(Controller)
	{
		case 0:
		case 1:
		case 2:
		Can_CtrlConfigData[ Controller ] = Config;
		if(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == (CCAN_CONTROLLER_0))
		{
			CAN_ptr = CCAN_CONTROLLER_0;
		}
		if(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == (CCAN_CONTROLLER_1))
		{
			CAN_ptr = CCAN_CONTROLLER_1;
		}
		break;
		default:
		return;
	}



	CCanLLD_EnableControllerInterrupts (Controller);
	 // set CAN controller pins into active mode
    CCanLLD_InitControllerPins(Controller);

	BTR = CCanLLD_Get_BTR(Config->Baudrate * 1000, Can_Config.input_clock);

	if (CCanLLD_InitializeControllerDefaultState(CAN_ptr,Controller,BTR, (CAN_IE | CAN_EIE | CAN_DAR)) != CAN_OK )
	{
		return;

	}


	// now we are going through the HW-s and we are sorted properly into Tx or RX
	for( msg_obj = CAN_MO_1;  msg_obj <= MAX_HW_OBJECTS && msg_obj <= CAN_MO_32 ; msg_obj++)
	{
		if(ControllerMO[Controller].HW_Object_ptr[msg_obj] != NULL_PTR)
		{
			switch(ControllerMO[Controller].HW_Object_ptr[msg_obj]->Type)
			{ // Operative Mode
				case TX_Basic:
				case TX_Full:
					CCanLLD_SetTxMsgObj(Can_CtrlConfigData[ Controller ]->Controller_Symbolic_Name, msg_obj, ControllerMO[Controller].HW_Object_ptr[msg_obj]->Id_Type, ControllerMO[Controller].HW_Object_ptr[msg_obj]->Id_Value, ControllerMO[Controller].HW_Object_ptr[msg_obj]->Dlc_Value);
				default: break;
			}
		}
	}


	for( msg_obj = CAN_MO_1 ; msg_obj <= MAX_HW_OBJECTS && msg_obj <= CAN_MO_32; msg_obj++ ) // Rest of MO for receiving
	{  // if there is enough HW MO-s, then assign field of HW MO-s to one CAN ID
    // in this situation will be HW MO-s linked in FIFO mode
    // Write init values to each message object
    	if(ControllerMO[Controller].HW_Object_ptr[msg_obj] != NULL_PTR)
    	{
			switch(ControllerMO[Controller].HW_Object_ptr[msg_obj]->Type)
			{ // Operative Mode
				case RX_Basic:
				case RX_Full:

					if(CountRx < MAX_RX_OBJECTS)
					{ // next Message object points to same HW_object >>> configured as a FIFO
						CCanLLD_SetRxMsgObj(Can_CtrlConfigData[ Controller ]->Controller_Symbolic_Name, msg_obj, ControllerMO[Controller].HW_Object_ptr[msg_obj]->Id_Type, ControllerMO[Controller].HW_Object_ptr[msg_obj]->Id_Value, ControllerMO[Controller].Filter_Mask_ptr[msg_obj]->Filter_Mask_Value, false);
						CountRx++;
					}
					else	if(CountRx == MAX_RX_OBJECTS)
                                            { // is it the end of FIFO?	>>> change type of this MO to 'End of FIFO'
                                                     CCanLLD_SetRxMsgObj(Can_CtrlConfigData[ Controller ]->Controller_Symbolic_Name, msg_obj  , ControllerMO[Controller].HW_Object_ptr[msg_obj]->Id_Type, ControllerMO[Controller].HW_Object_ptr[msg_obj]->Id_Value, ControllerMO[Controller].Filter_Mask_ptr[msg_obj]->Filter_Mask_Value, true);
                                                     CountRx++;
	                                   }
					break;
					default: break;
			}
	}
	}


}

//******************************************************************************
// Name:        CCanLLD_CheckController(uint8_t Controller)
// Parameters:  no. of CCan Controller
// Returns:     true - OK
//		false - ERROR
// Description: Checking parameter 'Controller' if belongs to CCanLLD
//******************************************************************************
uint8_t CCanLLD_CheckController(uint8_t Controller)
{ // checking of address of controller
// checking if controller is in expected range of CCan controllers
	switch(Controller)
	{
		case 0:
		case 1:
		case 2:	break;
		default: return false;
	}

	 if(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == (CCAN_CONTROLLER_0) &&
	 Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == (CCAN_CONTROLLER_1))
	 {
	 	return false;
	 }
	return true;
}

// ****************************************************************************
// CCanLLD_InitControllerPins
//
// Desc.: This routine enable only CAN Controller TX / RX pins.
//
//
// Parameters (in)	:	Controller  CAN Controller to be initialized
//
// Parameters (out)	: 	None
// ****************************************************************************
void CCanLLD_InitControllerPins(uint8_t Controller)
{
	if (Controller == 0)
	{
	}

	else  	if (Controller == 1)
	{
	}
	else
	{
	}
}

// ****************************************************************************
// CCanLLD_DisableControllerInterrupts
//
// Desc.: This function stores the information which interrupts are currently enabled.
//        Then it disables all interrupts for this CAN Controller.
//
//
// Parameters (in)	:	Controller for which interrupts shall be disabled
//						this no. of controller is not a our no. of controller
//						but a no. from config. file !!!
// Parameters (out)	:	None
// ****************************************************************************
void CCanLLD_DisableControllerInterrupts (uint8_t Controller)
{
	 if(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == CCAN_CONTROLLER_0  && CCAN_CONTROLLER_0 != NULL_PTR)
	 {
//		LLD_VIC_ClearSoftwareInterrupt(VIC_CAN_0_LINE);        // clear pending bit
//		LLD_VIC_DisableChannel(VIC_CAN_0_LINE);		// disable CAN0 interrupts
	 }
	 if( Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == CCAN_CONTROLLER_1  && CCAN_CONTROLLER_1 != NULL_PTR)
	 {
//		LLD_VIC_ClearSoftwareInterrupt(VIC_CAN_1_LINE);        // clear pending bit
//		LLD_VIC_DisableChannel(VIC_CAN_1_LINE);		// disable CAN1 interrupts
	 }
}// CCanLLD_DisableControllerInterrupts

// ****************************************************************************
// CCanLLD_EnableControllerInterrupts
//
// Desc.: This function re-enables all interrupts that have been disabled by
//	  Can_DisableControllerInterrupts.
//
//
// Parameters (in)	:	Controller for which interrupts shall be re-enabled
//						this no. of controller is not a our no. of controller
//						but a no. from config. file !!!
// Parameters (out)	:	None
// ****************************************************************************
void CCanLLD_EnableControllerInterrupts (uint8_t Controller)
{
	if(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == CCAN_CONTROLLER_0  && CCAN_CONTROLLER_0 != NULL_PTR)
	{
//        LLD_VIC_ClearSoftwareInterrupt(VIC_CAN_0_LINE);        // clear pending bit
//		LLD_VIC_EnableChannel(VIC_CAN_0_LINE);		// enable CAN0 interrupts
		return;
	}
	if( Can_CtrlConfigData[Controller]->Controller_Symbolic_Name == CCAN_CONTROLLER_1  && CCAN_CONTROLLER_1 != NULL_PTR)
	{
//		LLD_VIC_ClearSoftwareInterrupt(VIC_CAN_1_LINE);        // clear pending bit
//		LLD_VIC_EnableChannel(VIC_CAN_1_LINE);		// enable CAN1 interrupts
		return;
	}
}//CCanLLD_EnableControllerInterrupts

// ****************************************************************************
// Name:        CCanLLD_MObject_UpdateValid
// Parameters:
//              CAN_ptr:      CAN Control pointer
//              object:       pointer to message object
//              state:        sets object to invalid, otherwise it will be valid
//                            (see CAN_MOBJECT_VALID and CAN_MOBJECT_INVALID
//                            macros)
// Returns:
//              none
// Description:
//              sets given MO as valid/invalid. The function is non-blocking.
//              But it may block when switching from VALID to INVALID.
//              It depends on TXRQ and RMTPND bits in MO's MCR register. Both
//              flags has to be zero.
// ****************************************************************************
void CCanLLD_MObject_UpdateValid(volatile CCan * CAN_ptr, uint8_t object, uint8_t state)
{


        CAN_IF_WAIT(2);
        CAN_IF_FILL_WAIT(2,object & CAN_CHANNEL_MASK,CANC_IF_ARB);
/*///AL object & CAN_CHANNEL_MASK poich� in object � presente anche l'individuazione dell canale
				CANC_IF_ARB � definito in main essere 0x0020.
				si scrive su command Mask Register  l'opzione per trasferire l"arbitraggio"	di can0.if2
				in seguito si si opera il trasferimento dalla  HWobject=bject & CAN_CHANNEL_MASK scrivendo sul Command Request Register*/

        CAN_IF_WAIT(1);
//////////ALTEMP  perch� trasferisce identificatore ricevuto su IF2 su IF1 + la valitit� del messaggio ricevuto???
        if (state == CAN_MOBJECT_VALID){
            CAN_ptr->IF1AR1     = CAN_ptr->IF2AR1;
           CAN_ptr->IF1AR2.reg = CAN_ptr->IF2AR2.reg | CANC_MO_IF_MSGVAL;
        }
        else {
            CAN_ptr->IF1AR1     = CAN_ptr->IF2AR1;
            CAN_ptr->IF1AR2.reg = CAN_ptr->IF2AR2.reg & ~CANC_MO_IF_MSGVAL;
        }

        CAN_IF_FLUSH(1,object & CAN_CHANNEL_MASK,CANC_IF_ARB);


}//CCanLLD_MObject_UpdateValid

// *********************************************************************************
// Function Name  : CCanLLD_SetTxMsgObj
// Description    : Configure the message object as TX
//					For receiving direction is used CAN Controller interface IF2
//					For transmission direction is used CAN Controller interface IF1
//					There isn't used a any MASK and arbitration, this 'MailBox' will
//					send every CAN frame.
// Input          : -CAN_ptr: pointer to the CAN peripheral
//                  -Message object number, from 1 to 32
//                  -STANDARD = 0, EXTENDED
//		    - range start
//		    - DLC
// Output         : None
// Return         : None
// *********************************************************************************/
void CCanLLD_SetTxMsgObj(volatile CCan * CAN_ptr, uint8_t Object, Can_Id_Format idType, uint32_t range_start, uint8_t DLC_Val)
{

	if(Object >= CAN_MO_1 && Object <= CAN_MO_32 && CAN_ptr != NULL_PTR)
	{	// CAN Controller pointer OK, Object OK

		CAN_IF_WAIT(1);

			switch(idType)
			{
				case STANDARD:	 // standard frame
				CAN_ptr->IF1CMR.reg = CANC_IF_WRRD | CANC_IF_MASK | CANC_IF_ARB | CANC_IF_CNTRL | CANC_IF_DATA_A | CANC_IF_DATA_B;
				CAN_ptr->IF1AR1 	=  0x0000;
	 			CAN_ptr->IF1AR2.reg = CANC_MO_IF_MSGVAL | CANC_MO_IF_DIRECTION_TX | CANC_COMPUTE_MASKSHORT(range_start);
	 			break;
	 			case EXTENDED: // extended frame
	 			CAN_ptr->IF1CMR.reg = CANC_IF_WRRD | CANC_IF_ARB | CANC_IF_CNTRL | CANC_IF_DATA_A | CANC_IF_DATA_B;
				CAN_ptr->IF1AR1     = CANC_COMPUTE_MASKLONG_L(range_start);
	 			CAN_ptr->IF1AR2.reg = CANC_MO_IF_MSGVAL | CANC_MO_IF_ID_LONG | CANC_MO_IF_DIRECTION_TX | CANC_COMPUTE_MASKLONG_U(range_start);
	 			break;
	 			default: break; // unknown Id_Type of the Frame
	 		}

	 		CAN_ptr->IF1MCR.reg = CANC_MO_IF_TXIE | DLC_Val;
	 		CAN_ptr->IF1DATAA1.reg = (uint16_t) 0x0000;
	 		CAN_ptr->IF1DATAA2.reg = (uint16_t) 0x0000;
	 		CAN_ptr->IF1DATAB1.reg = (uint16_t) 0x0000;
	 		CAN_ptr->IF1DATAB2.reg = (uint16_t) 0x0000;
	 		CAN_ptr->IF1CR.reg = (uint16_t) Object;
	}
}

// ****************************************************************************
// CCanLLD_Write
//
// Desc.: This routine transmits L-PDU frame
//           For receiving direction is used CAN Controller interface IF2
//		  For transmission direction is used CAN Controller interface IF1
//
// Parameters (in)	:
//           Controller -- CAN Controller for the transmission
//				this parameter is not necessary, because this information
//				about Controller is in Hth
//           Hth -- information which HW-transmit handle shall
//				be used for transmit it is unique no.
//  			      no. of MO <1,0x20> for one CAN Controller
//	        PduInfo -- Pointer to SDU user memory,DLC and Identifier,
//	        SwPduHandle -- SW Handle that will identify the request in the
//	                   confirmation callback function.
//	                   Memory area for this data is alocated in upper layer
//
// Parameters (out)	:	Can_ReturnType, CAN_OK or CAN_BUSY
// ****************************************************************************
Can_ReturnType CCanLLD_Write (uint8_t Controller, Can_HwHandleType Hth, Can_PduType *PduInfo, PduIdType SwPduHandle )
{
	volatile CCan * CAN_ptr;

	// CHECKING OF INPUT PARAMETERS & CHECKING IF THEY FITS OUR CONFIGURATION
	// Hth is a unique no. of the handler, within this information is controller no. too
	CAN_ptr = HandlerMO[Hth].CAN_ptr;
	if(CAN_ptr == NULL_PTR)
	{ // internal SW error -> structure is not setted
		return CAN_BUSY;
	}
	if(HandlerMO[Hth].MailBox > CAN_MO_32 || HandlerMO[Hth].MailBox == 0)
	{ // internal SW error -> structure is not setted or setted wrong
		return CAN_BUSY;
	}
	if(HandlerMO[Hth].can_controller >= MAX_CONFIG_CAN_CONTROLLERS)
	{ // internal SW error -> structure is not setted or setted wrong
		return CAN_BUSY;
	}
	if(ControllerMO[HandlerMO[Hth].can_controller].HW_Object_ptr == NULL_PTR)
	{ // internal SW error -> structure is not setted or setted wrong
		return CAN_BUSY;
	}
	if(ControllerMO[HandlerMO[Hth].can_controller].HW_Object_ptr[HandlerMO[Hth].MailBox]->Handle_number != Hth)
	{ // internal SW error -> structure is not setted or setted wrong
		return CAN_BUSY;
	}
	if(ControllerMO[HandlerMO[Hth].can_controller].HW_Object_ptr[HandlerMO[Hth].MailBox]->Type == RX_Basic ||ControllerMO[HandlerMO[Hth].can_controller].HW_Object_ptr[HandlerMO[Hth].MailBox]->Type == RX_Full )
	{ // checking if configured MO is of requested typefor transmition
		return CAN_BUSY;	// internal SW error -> structure is not setted or setted wrong
	}

	// maybe checking of mask & DLC & ID-s for future it's nice to have there too


	// checking if CAN controller is in running / operational mode
	if((CAN_ptr->CR.reg & (CANC_INITIALIZATION_EN | CANC_CONFIG_CHANGE_EN)) != 0)
	{
		return CAN_BUSY; // CCAN cell is not in operation mode, just wait
	}

	// checking if CAN controller is in error passive level
	if((CAN_ptr->ECR.reg & CCAN_RECEIVE_ERROR_PASSIVE) != 0)
	{
		return CAN_BUSY; // CCAN cell is not in operation mode, just wait
	}

	// checking of re-entrancy
	if((ControllerMO[HandlerMO[Hth].can_controller].state & STATE_WRITE_REENTRANT) != 0)
	{
		return CAN_BUSY; // another CAN_write is in action, just wait
	}

	ControllerMO[HandlerMO[Hth].can_controller].state |= STATE_WRITE_REENTRANT;

	// END OF CHECKING PARAMETERS
	// look if there is some message pending for sending
        if(HandlerMO[Hth].MailBox <= 16)
        {
  	         if(((CAN_ptr->NEWDATR1 | CAN_ptr->TRR1) & ( 1 << (HandlerMO[Hth].MailBox - 1))) != 0)
  		{ // look if CAN is in operative mode and not in BusOff or passive error state
  			ControllerMO[HandlerMO[Hth].can_controller].state &= (~STATE_WRITE_REENTRANT);
  			return CAN_BUSY;
  		}
        }
        else if(HandlerMO[Hth].MailBox > 16)
        {
  	        if(((CAN_ptr->NEWDATR2 | CAN_ptr->TRR2) & ( 1 << (HandlerMO[Hth].MailBox - 17))) != 0)
  		{ // look if CAN is in operative mode and not in BusOff or passive error state
  			ControllerMO[HandlerMO[Hth].can_controller].state &= (~STATE_WRITE_REENTRANT);
  			return CAN_BUSY;
  		}
        }

	// HW object = Mailbox of the CAN controller is free for new data


	CAN_IF_WAIT (1);

 	if(((CAN_ptr->IF1CR.reg & CANC_IF_BUSY) != 0) || ((CAN_ptr->CR.reg & (CANC_INITIALIZATION_EN | CANC_CONFIG_CHANGE_EN)) != 0))
	{ 	// internal HW error -> IF register = interface register is busy to long
		// or CAN Controller is not in operable mode
		ControllerMO[HandlerMO[Hth].can_controller].state &= (~STATE_WRITE_REENTRANT);
		return CAN_BUSY;
	}

	// NOW *** IS NECESSARY TO DO ATOMIC OPERATION NON-INTERRUPTABLE SEQUENCE ***
	// if during next operations will become CAN Controller into Bus Off state
	// or another strange states, is possible that CAN Controller will become
	// inactive and in this state behavior of the CAN Controller is unpredictable
	// Othervise Pending interrupt will be 'saved' and recovering from
	// 'strange' states of the CAN Controller will be handled afterwords

	CCanLLD_DisableControllerInterrupts	(HandlerMO[Hth].can_controller);

	// now begin work with CAN Controller interface
	// and will not be disturbed by another interrupts
	CAN_ptr->IF1CMR.reg = CANC_IF_ARB | CANC_IF_CNTRL;
	CAN_ptr->IF1CR.reg = HandlerMO[Hth].MailBox;
	CAN_IF_WAIT (1);

 	if(((CAN_ptr->IF1CR.reg & CANC_IF_BUSY) != 0) || ((CAN_ptr->CR.reg & (CANC_INITIALIZATION_EN | CANC_CONFIG_CHANGE_EN)) != 0))
	{ 	// internal HW error -> IF register = interface register is busy to long
		// or CAN Controller is not in operable mode
		// re-enable interrupt
		CCanLLD_EnableControllerInterrupts	(HandlerMO[Hth].can_controller);
		// release proceeding of the CAN_Write function
		ControllerMO[HandlerMO[Hth].can_controller].state &= (~STATE_WRITE_REENTRANT);
		return CAN_BUSY;
	}
	// update the contents needed for transmission
	CAN_ptr->IF1CMR.reg = CANC_IF_WRRD | CANC_IF_ARB | CANC_IF_CNTRL | CANC_IF_DATA_A | CANC_IF_DATA_B;
	if ((CAN_ptr->IF1AR2.reg & CANC_MO_IF_ID_LONG) != 0)
	{ // extended frame
		CAN_ptr->IF1AR1	= ((uint16_t)(PduInfo->Id & 0x0000ffff));
		CAN_ptr->IF1AR1     = CANC_COMPUTE_MASKLONG_L(PduInfo->Id);
	         CAN_ptr->IF1AR2.reg = (CAN_ptr->IF1AR2.reg & 0xE000) | CANC_COMPUTE_MASKLONG_U(PduInfo->Id);
	}
	else
	{ // standard frame
 		CAN_ptr->IF1AR1	= (uint16_t) 0x0000;
                  CAN_ptr->IF1AR2.reg = (CAN_ptr->IF1AR2.reg & 0xE000) | CANC_COMPUTE_MASKSHORT(PduInfo->Id);
 	}
 	CAN_ptr->IF1MCR.reg = (CAN_ptr->IF1MCR.reg & 0xfef0) | CANC_M0_IF_NEWDAT | CANC_MO_IF_TXRQST | MIN(8,(PduInfo->Lenght));
 	// Fill Message Data
	switch(PduInfo->Lenght)
         {
		case 8 :  CAN_ptr->IF1DATAB2.bit.data7 = *PduInfo->Sdu; PduInfo->Sdu++;
	       	case 7 :  CAN_ptr->IF1DATAB2.bit.data6 = *PduInfo->Sdu; PduInfo->Sdu++;
	       	case 6 :  CAN_ptr->IF1DATAB1.bit.data5 = *PduInfo->Sdu; PduInfo->Sdu++;
	       	case 5 :  CAN_ptr->IF1DATAB1.bit.data4 = *PduInfo->Sdu; PduInfo->Sdu++;
	       	case 4 :  CAN_ptr->IF1DATAA2.bit.data3 = *PduInfo->Sdu; PduInfo->Sdu++;
	         case 3 :  CAN_ptr->IF1DATAA2.bit.data2 = *PduInfo->Sdu; PduInfo->Sdu++;
	         case 2 :  CAN_ptr->IF1DATAA1.bit.data1 = *PduInfo->Sdu; PduInfo->Sdu++;
	       	case 1 :  CAN_ptr->IF1DATAA1.bit.data0 = *PduInfo->Sdu;
	       	default:  break;
	}
 	CAN_ptr->IF1CR.reg = HandlerMO[Hth].MailBox;

 	// re-enable interrupt
 	CCanLLD_EnableControllerInterrupts	(HandlerMO[Hth].can_controller);
 	// END OF *** ATOMIC NON-INTERRUPTABLE SEQUENCE ***

 	// release proceeding of the CAN_Write function
 	ControllerMO[HandlerMO[Hth].can_controller].state &= (~STATE_WRITE_REENTRANT);
 	// copying data especialy SwPduHandle
 	ControllerMO[HandlerMO[Hth].can_controller].SwPduHandle[HandlerMO[Hth].MailBox] =SwPduHandle;
 	return CAN_OK;
}//CCanLLD_Write

// **************************************************************************************
// Function Name  : CCanLLD_SetRxMsgObj
// Description    : Configure the message object as RX
//					For receiving direction is used CAN Controller interface IF2
// Input          : - CAN_ptr: pointer to the CAN peripheral
//                  - Object - Message object number, from <1 to 32>
//                  - Can_Id_Format { STANDARD = 0,  EXTENDED }
//                  - start address of the identifier range used for acceptance filtering
//                  - stop address of the identifier range used for acceptance filtering
//                  - true for a single receive object or a FIFO receive object that
//                    is the last one of the FIFO
//                  - false for a FIFO receive object that is not the last one
// Output         : None
// Return         : None
// **************************************************************************************/
void CCanLLD_SetRxMsgObj(volatile CCan * CAN_ptr, uint8_t Object, Can_Id_Format idType, uint32_t range_start, uint32_t range_end, uint8_t EndOfBlock)
{


	if(Object >= CAN_MO_1 && Object <= CAN_MO_32 && CAN_ptr != NULL_PTR)
	{	// CAN Controller pointer OK, Object OK

		CAN_IF_WAIT (2);

	 	if((CAN_ptr->IF2CR.reg  & CANC_IF_BUSY) == CANC_IF_BUSY)
		{ 	// error -> IF register = interface register is busy to long
			return ;
		}

	  	switch(idType)
		{
			case STANDARD:	// standard frame
			CAN_ptr->IF2CMR.reg = CANC_IF_WRRD | CANC_IF_MASK | CANC_IF_ARB | CANC_IF_CNTRL | CANC_IF_DATA_A | CANC_IF_DATA_B;
			CAN_ptr->IF2MASK1 = (uint16_t) 0x0000;
			CAN_ptr->IF2MASK2.reg = CANC_COMPUTE_MASKSHORT(range_end);
			CAN_ptr->IF2AR1 	= (uint16_t) 0x0000;
			CAN_ptr->IF2AR2.reg = CANC_MO_IF_MSGVAL | CANC_COMPUTE_MASKSHORT(range_start);
			break;
			case EXTENDED: 	// extended frame
			CAN_ptr->IF2CMR.reg = CANC_IF_WRRD | CANC_IF_MASK | CANC_IF_ARB | CANC_IF_CNTRL | CANC_IF_DATA_A | CANC_IF_DATA_B;
			CAN_ptr->IF2MASK1 = CANC_COMPUTE_MASKLONG_L(range_end);
			CAN_ptr->IF2MASK2.reg = CANC_MO_IF_MXTD | CANC_COMPUTE_MASKLONG_U(range_end);
			CAN_ptr->IF2AR1 = CANC_COMPUTE_MASKLONG_L(range_start);
			CAN_ptr->IF2AR2.reg = CANC_MO_IF_MSGVAL | CANC_MO_IF_ID_LONG | CANC_COMPUTE_MASKLONG_U(range_start);
			break;
		default: break;	// unknown type of the frame
		}
		CAN_ptr->IF2MCR.reg = CANC_MO_IF_RXIE |CANC_MO_IF_UMASK  | (EndOfBlock ? CANC_MO_IF_EOB : 0);
		CAN_ptr->IF2DATAA1.reg = (uint16_t) 0x0000;
		CAN_ptr->IF2DATAA2.reg = (uint16_t) 0x0000;
		CAN_ptr->IF2DATAB1.reg = (uint16_t) 0x0000;
		CAN_ptr->IF2DATAB2.reg = (uint16_t) 0x0000;
		CAN_ptr->IF2CR.reg = Object;

	}
}//CCanLLD_SetRxMsgObj

// ****************************************************************************
// CCanLLD_SetControllerMode
//
// Desc.: This routine is used to set CAN Controller into defined state.
//
//
// Parameters (in)	:	Controller  CAN Controller for which the status
//				shall be changed
// 	  		:	Transition  Possible transitions
// Parameters (out)	: 	CAN_OK
//				CAN_WAKEUP will be generated itself by the
//				ISR wake_up routine
// ****************************************************************************
Can_ReturnType CCanLLD_SetControllerMode (uint8_t Controller, Can_StateTransitionType  Transition)
{


	if( false == CCanLLD_CheckController(Controller))
	{
		return CAN_OK;
	}

	switch(Transition)
	{
		case CAN_T_WAKEUP:
			CCanLLD_EnableWakeUp(Controller);
			CCanLLD_EnableCLK(Controller);
			CCanLLD_StartController(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name);
			break;
		case CAN_T_STOP: // stopping of controller and disable initiate interrupts
			// disable Wake-up
			CCanLLD_DisableWakeUp(Controller);
			CCanLLD_EnableCLK(Controller);
			CCanLLD_StopController(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name);
			break;
		case CAN_T_SLEEP:
			// activate Wake-up interrupt of the can controller
			CCanLLD_EnableWakeUp(Controller);
			CCanLLD_EnableCLK(Controller);
			CCanLLD_StartController(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name);
			CCanLLD_DisableCLK(Controller);
			// should be wooked up by ISR wake-up routine
			break;
		case CAN_T_START:
			// disable Wake-up
			CCanLLD_DisableWakeUp(Controller);
			CCanLLD_EnableCLK(Controller);
			// enable interrupts -> param. is the no. of controller which should to be enabled
			CCanLLD_EnableControllerInterrupts (Controller);
			// set CAN controller pins into active mode
	 		CCanLLD_InitControllerPins(Controller);
			CCanLLD_StartController(Can_CtrlConfigData[Controller]->Controller_Symbolic_Name);
			break;
		default:
		break;
	}
return CAN_OK;
}// CCanLLD_SetControllerMode

//******************************************************************************
// Name:        CCanLLD_StopController
//				Deinitializes the CANx peripheral registers to their default
//              reset values.
// Parameters:  pointer to CAN
// Returns:     void
// Description:  CAN controller and disable interrupts
//******************************************************************************
void CCanLLD_StopController(volatile CCan * CAN_ptr)
{
	if(CAN_ptr != NULL_PTR)
	{
    	CAN_ptr->CR.reg |= 0x0041;         //switch CANx to stop configuration mode
    }
}//CCanLLD_StopController


//******************************************************************************
// Name:        CCanLLD_StartController
// Parameters:  pointer to CAN
// Returns:     void
// Description: Start CAN controller to normal mode
//******************************************************************************
void CCanLLD_StartController(volatile CCan * CAN_ptr)
{
	if(CAN_ptr != NULL_PTR)
	{
    	CAN_ptr->CR.reg &= 0x000e;         //switch CANx to normal operation mode
    }
}//CCanLLD_StartController

// ****************************************************************************
// CCanLLD_EnableCLK
//
// Desc.: Driver Module Enabling of CLK for CAN
//
//
// Parameters (in)	:	Controller no. of CAN Controller
// Parameters (out)	:	None.
// ****************************************************************************
void CCanLLD_EnableCLK(uint8_t Controller)
{
	switch(Controller)
	{
		case 0: break;
		case 1: break;
		case 2: break;
		default: // !!! how to pass the error code to the upper layer ?
			break;
	}
}//CCanLLD_EnableCLK

// ****************************************************************************
// CCanLLD_DisableCLK
//
// Desc.: Driver Module Disabling of CLK for CAN
//
//
// Parameters (in)	:	Controller no. of CAN Controller
// Parameters (out)	:	None.
// ****************************************************************************
void CCanLLD_DisableCLK(uint8_t Controller)
{
	switch(Controller)
	{
		case 0: break;
		case 1: break;
		case 2: break;
		default: // !!! how to pass the error code to the upper layer ?
			break;
	}
}//CCanLLD_DisableCLK

// ****************************************************************************
// CCanLLD_EnableWakeUp
//
// Desc.: Driver Module Enabling of Wake Up possibility
//
//
// Parameters (in)	:	Controller no. of CAN Controller
// Parameters (out)	:	None.
// ****************************************************************************
void CCanLLD_EnableWakeUp(uint8_t Controller)
{
	switch(Controller)
	{
		case 0: break;
		case 1: break;
		case 2: break;
		default: // !!! how to pass the error code to the upper layer ?
			break;
	}
}//CCanLLD_EnableWakeUp


// ****************************************************************************
// CCanLLD_DisableWakeUp
//
// Desc.: Driver Module Disabling of Wake Up possibility
//
//
// Parameters (in)	:	Controller no. of CAN Controller
// Parameters (out)	:	None.
// ****************************************************************************
void CCanLLD_DisableWakeUp(uint8_t Controller)
{
	switch(Controller)
	{
		case 0: break;
		case 1: break;
		case 2: break;
		default: // !!! how to pass the error code to the upper layer ?
			break;
	}
}//CCanLLD_DisableWakeUp

//************************* End of Can_LLD.c **********************************

//************************* Start of Can.c ************************************


// ****************************************************************************
// Can_Init
//
// Desc.: Driver Module Initialization.
// For future in case of implementation of another type of CAN cell (B) should
// be used for naming BCanLLD... and call initialization of this HW Unit
// from this function to initialize Can controllers belongs to this HW unit
//
// Parameters (in)	:	ConfigPtr	Pointer to configuration set
// Parameters (out)	:	None.
// ****************************************************************************
void Can_Init(Can_ConfigType* Config)
{
	uint16_t i, j, k;
	uint8_t can_controller;
	uint8_t msg_obj;
	uint8_t no_txx[MAX_CONFIG_CAN_CONTROLLERS + 1];	// no. of trasmission objects
	uint8_t no_rxx[MAX_CONFIG_CAN_CONTROLLERS + 1];	// no. of reception objects
	Can_Filter_Mask	*		Filter_Mask_ptr;
	Can_HW_Object	*		HW_Object_ptr;
	Can_ControllerConfigType *	Controller_ptr;



	// Check NULL pointer and  VendorID, ModuleID, Device ID, Version, Patch version
	  if ((NULL_PTR == Config) || ( (Config->Vendor_Id != CAN_VENDOR_ID) ||
	      (Config->Module_Id != CAN_MODULE_ID) || (Config->Device_Id != CAN_DEVICE_ID) ||
	      (Config->Major_Version != CAN_MAJOR_VERSION) || (Config->Minor_Version != CAN_MINOR_VERSION) ||
	      (Config->Driver_Patch_Version != CAN_PATCH_VERSION) || (Config->Can_RxIndication == NULL_PTR)||
		(Config->Can_TxCnfOk == NULL_PTR) || (Config->Can_TxCnfAbort == NULL_PTR) || (Config->Can_ControllerWakeup == NULL_PTR) ||
		(Config->Can_ControllerBusOff == NULL_PTR)))

	  {
	      return;
	  }

	// From SPAL
	GlobalConfigData.Vendor_Id=Config->Vendor_Id;
	GlobalConfigData.Device_Id=Config->Device_Id;
	GlobalConfigData.Module_Id = Config->Module_Id;
	GlobalConfigData.Major_Version = Config->Major_Version;
	GlobalConfigData.Minor_Version = Config->Minor_Version;
	GlobalConfigData.Driver_Patch_Version = Config->Driver_Patch_Version;
	GlobalConfigData.Objects_num = Config->Objects_num;	//	Number of CAN Objects for each CAN controller
	GlobalConfigData.Id_format = Config->Id_format;		//	Supported frame ID format (Standard/Extended)

	// CallBack Function Configuration
	GlobalConfigData.Can_RxIndication=Config->Can_RxIndication;
	GlobalConfigData.Can_TxCnfOk=Config->Can_TxCnfOk;
	GlobalConfigData.Can_TxCnfAbort=Config->Can_TxCnfAbort;
	GlobalConfigData.Can_ControllerWakeup=Config->Can_ControllerWakeup;
	GlobalConfigData.Can_ControllerBusOff=Config->Can_ControllerBusOff;

	// clear all HW_Objects configured CAN controller
	for(i = 0; i <= MAX_HANDLES; i++)
	{
		HandlerMO[i].CAN_ptr = NULL_PTR;	// pointer to HW CAN
		HandlerMO[i].can_controller = (uint8_t)0xff;	// internal can no.
		HandlerMO[i].MailBox = 0;	//internal MailBox no. inside of one CAN controller
	}


	// clear pointers of all HW_Objects configured for CAN controller-s
	for(j = 0; j < MAX_CONFIG_CAN_CONTROLLERS; j++)
	{
		for(i = 0; i <= MAX_HW_OBJECTS ; i++)
		{
			ControllerMO[j].HW_Object_ptr[i] = NULL_PTR;
			ControllerMO[j].Filter_Mask_ptr[i] = NULL_PTR;
			ControllerMO[j].state = (uint16_t)0;
		}
		// clearing of the rx/tx hardware message objects counters
		no_rxx[j] = 0x00;
		no_txx[j] = 0x00;
	}

         // cycle going through Hardware objects & Filter masks to fill internall structures

	for(HW_Object_ptr = Config->HW_Object, i = 0; i < Config->Objects_num; i++, HW_Object_ptr++ )// Number of Hardware objects
	 {
		if(HW_Object_ptr->Id_Type != Config->Id_format)
		{
			return;
		}
		// handle number is in array allocated for Handlers and other params seems that is OK too
 		for( Filter_Mask_ptr = Config->Object_Filter_Mask, j = 0; (j < Config->Objects_num) && (j <= MAX_HANDLES); j++, Filter_Mask_ptr++)
	 	{ // we are searching associated filter mask
 			if(Filter_Mask_ptr->Mask_number == HW_Object_ptr->Associated_Mask_number)
 			{ // mask no. was found >>> we are searching a symbolic name for this controller
 				for(Controller_ptr = Config->Controllers_ptr, k = 0; k < Config->Controller_num; k ++, Controller_ptr++)
 				{
 					if(Controller_ptr->Controller_Number == Filter_Mask_ptr->Associated_Controller)
	 				{// search no. of internal structure ControllerMO no. of internal structure belongs to ISR
 						can_controller = 0xff;
 						if((Controller_ptr->Controller_Symbolic_Name == CCAN_CONTROLLER_0) && (CCAN_CONTROLLER_0 != NULL_PTR))
 						{ can_controller = 0; }
 						if((Controller_ptr->Controller_Symbolic_Name == CCAN_CONTROLLER_1) && (CCAN_CONTROLLER_1 != NULL_PTR))
 						{ can_controller = 1; }
 						if(can_controller < 0xff && can_controller <= MAX_CONFIG_CAN_CONTROLLERS)
	 					{ 	// now collect transmission handlers first
 							switch(HW_Object_ptr->Type)
							{ // Operative Mode
							   case TX_Basic:
							   case TX_Full:
							    // search free MailBox inside of the CAN Controller (internal structure)
								for(msg_obj = CAN_MO_1; msg_obj <= CAN_MO_32 && msg_obj <=  MAX_HW_OBJECTS; msg_obj++ )
								{
									if(((Can_HW_Object*)ControllerMO[can_controller].HW_Object_ptr[msg_obj]) == NULL_PTR)
									{ // free MailBox was found
                                        HandlerMO[HW_Object_ptr->Handle_number].can_controller = can_controller;
                                        HandlerMO[HW_Object_ptr->Handle_number].CAN_ptr = Controller_ptr->Controller_Symbolic_Name;

 										// internal numbering of MO inside of CAN controller
                                        HandlerMO[HW_Object_ptr->Handle_number].MailBox = msg_obj;

                                            ControllerMO[can_controller].HW_Object_ptr[msg_obj] = HW_Object_ptr;
                                            ControllerMO[can_controller].Filter_Mask_ptr[msg_obj] = Filter_Mask_ptr;
                                            no_txx[can_controller] ++;

										// exit from all cycles except 'HW_Object_ptr'
										j = Config->Objects_num + 1;
										k = Config->Controller_num + 1;
										break;
									} // end if 'ControllerMO[can_controller].HW_Object_ptr[msg_obj]'
								} // end for 'msg_obj'
							     break;
							     case RX_Full:
							     case RX_Basic:
									no_rxx[can_controller]++;
									// exit from all cycles except 'HW_Object_ptr'
									j = Config->Objects_num + 1;
									k = Config->Controller_num + 1;
							     break;

							     default: break;
							} // end switch
	 					} // end if 'can_controller'
	 				} // end if 'Controller_ptr->Controller_Number'
	 			} // end for 'Controller_ptr'
	 		} // end if 'Filter_Mask_ptr->Mask_number'
		} // end for 'Filter_Mask_ptr'
	} // end for 'HW_Object_ptr'




	// now we have a no. of reception hardware objects, create FIFO from them if it is possible
	for(i = 0; i < MAX_CONFIG_CAN_CONTROLLERS; i ++)
	{  // have we enough HW MO-s? to full fill configuration requirements?
		// for each CAN Controller
		if((no_rxx[i] + no_txx[i]) > MAX_HW_OBJECTS )
		{ // in this version we will not support more then 32 HO/MO per one CAN Controller

			return;
		}
	}


	// cycle going through Hardware objects & Filter masks to fill internall structures
	// this cycle belongs to receive hardware objects
	for(HW_Object_ptr = Config->HW_Object, i = 0; i < Config->Objects_num; i++ , HW_Object_ptr++)// Number of Hardware objects
	 { // handle number is in array allocated for Handlers and other params seems that is OK too
	 	for( Filter_Mask_ptr = Config->Object_Filter_Mask, j = 0; (j < Config->Objects_num) && (j <= MAX_HANDLES); j++, Filter_Mask_ptr++)
	 	{ // we are searching associated filter mask
	 		if(Filter_Mask_ptr->Mask_number == HW_Object_ptr->Associated_Mask_number)
	 		{ // mask no. was found >>> we are searching a symbolic name for this controller
	 			for(Controller_ptr = Config->Controllers_ptr, k = 0; k < Config->Controller_num; k ++, Controller_ptr++)
	 			{
	 				if(Controller_ptr->Controller_Number == Filter_Mask_ptr->Associated_Controller)
	 				{// search no. of internal structure ControllerMO no. of internal structure belongs to ISR
	 					can_controller = 0xff;
	 					if((Controller_ptr->Controller_Symbolic_Name == CCAN_CONTROLLER_0) && (CCAN_CONTROLLER_0 != NULL_PTR))
	 					{ can_controller = 0; }
	 					if((Controller_ptr->Controller_Symbolic_Name == CCAN_CONTROLLER_1)&& (CCAN_CONTROLLER_1 != NULL_PTR))
	 					{ can_controller = 1; }
	 					if(can_controller < 0xff && can_controller <= MAX_CONFIG_CAN_CONTROLLERS)
	 					{ 	// now collect transmission handlers first
	 						switch(HW_Object_ptr->Type)
							{ // Operative Mode
							case TX_Basic:
							case TX_Full:
                                                                       j = Config->Objects_num + 1;
                                                                       k = Config->Controller_num + 1;
                                                                       break;
							case RX_Basic:
							case RX_Full:
								 // search free MailBox inside of the CAN Controller (internal structure)
								for(msg_obj = CAN_MO_1; msg_obj <= CAN_MO_32 && msg_obj <=  MAX_HW_OBJECTS && no_rxx[can_controller] > 0; msg_obj++ )
								{
									if(((Can_HW_Object*)ControllerMO[can_controller].HW_Object_ptr[msg_obj]) == NULL_PTR)
									{ 	// free MailBox was found
 										HandlerMO[HW_Object_ptr->Handle_number].MailBox = msg_obj;
										HandlerMO[HW_Object_ptr->Handle_number].can_controller = can_controller;
 										HandlerMO[HW_Object_ptr->Handle_number].CAN_ptr = Controller_ptr->Controller_Symbolic_Name;

										ControllerMO[can_controller].HW_Object_ptr[msg_obj] = HW_Object_ptr;
										ControllerMO[can_controller].Filter_Mask_ptr[msg_obj] = Filter_Mask_ptr;
										// exiting from all cycles except 'HW_Object_ptr'
										k = Config->Controller_num + 1;
										j = Config->Objects_num + 1;
										no_rxx[can_controller]--;
										break;
									}
								} // end for 'msg_obj'
							default:
									break;
							} // end of switch
	 					} // end if 'can_controller'
	 				} // end if 'Controller_ptr->Controller_Number'
	 			} // end for 'Controller_ptr'
	 		} // end if 'Filter_Mask_ptr->Mask_number'
		} // end for 'Filter_Mask_ptr'
	} // end for 'HW_Object_ptr'

	Controller_ptr	= Config->Controllers_ptr;
	for(i = 0; i < Config->Controller_num; i++, Controller_ptr++ )// Number of CAN controllers
	{
		CCanLLD_Init(Controller_ptr);
	}





}

// ****************************************************************************
// Can_MainFunction
//
// Desc.: This function performs the polling of all events that are configured
//	   statically as 'to be polled'
//
//
// Parameters (in)	:	None
// Parameters (out)	: 	None
// ****************************************************************************
void Can_MainFunction (void)
{
}

//************************* End of Can.c ************************************

void Can_RxIndication( uint8_t Handle_number, uint32_t Identifier, uint8_t CanDlc, uint8_t *CanSduPtr )
{
	//uint8_t Data[10];
	//Can_PduType PduInfo;
	//PduIdType SwPduHandle;
	//uint8_t i;
	//uint8_t controller;
	//Can_HwHandleType MessageObject;
	//PduIdType CanTxPduId;

	//ipc_test_tx_msg(10);
	switch (Identifier)
	{
		case 0x1A0:
		break;
		default:
		break;
	}

	if (ctrl)
	{
		next = Identifier;
		ctrl = 0;
	}

}
