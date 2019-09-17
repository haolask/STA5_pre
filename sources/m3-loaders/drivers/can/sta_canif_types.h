/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_canif_cfg.h
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN types definitions
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

#ifndef _CANIF_TYPES_H_
#define _CANIF_TYPES_H_

#include "sta_can.h"
#include "sta_canif.h"
#include "sta_canif_cfg.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"

/************************************************************************
| includes
| 1) system and project includes
| 2) needed interfaces from external units
     SWD - Software Quality - Embedded C/C++ Programming Style Guideline
     S2SD00492 Rev 1.0 19/47
     Confidential Released
| 3) internal and external interfaces from this unit
|-----------------------------------------------------------------------*/

typedef enum
{
	CANIF_CS_UNINIT = 0,
	CANIF_CS_INIT,
	CANIF_CS_STOP,
	CANIF_CS_RUNNING,
	CANIF_CS_SLEEP
}CanIf_DeviceModeType;

typedef enum
{
	CANIF_OFFLINE = 0,
	CANIF_ONLINE,
	CANIF_TX_OFFLINE,
	CANIF_TX_ONLINE,
	CANIF_RX_OFFLINE,
	CANIF_RX_ONLINE,
	CANIF_TX_OFFLINE_ACTIVE
}CanIf_ChannelModeType;

typedef enum
{
	DLC_OK  = 0,
	DLC_E_FAILED
}CanIf_DlcErrorReturnType;

// Operative Mode
typedef	enum
{
	BASIC_CAN = 0,
	FULL_CAN
}Can_Mode;

typedef enum
{
  DISPATCH_OK = 0,
  DISPATCH_NOT_OK
} PDU_Dispatcher;



typedef enum
{
  STATIC = 0,
  DYNAMIC
} CanIf_TxPduIdType;

typedef enum
{
  EMPTY = 1,
  FULL
} CanIf_TxQueuePduStatus;

typedef enum
{
  NO = 0,
  YES
} CanIf_TxQueueUsed;


typedef enum
{
  POLLING = 0,
  INTERRUPT
} CanIf_ModeNotification;

typedef uint32_t *CanIf_ControllerType;
typedef Can_PduType *PduInfoType;

#define UTEST 1

#endif
