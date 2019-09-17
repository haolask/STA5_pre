/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_canif_cfg.h
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN interface config. definitions
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


/************************************************************************
| includes
| 1) system and project includes
| 2) needed interfaces from external units
     SWD - Software Quality - Embedded C/C++ Programming Style Guideline
     S2SD00492 Rev 1.0 19/47
     Confidential Released
| 3) internal and external interfaces from this unit
|-----------------------------------------------------------------------*/

#ifndef _CANIF_CFG_H_
#define _CANIF_CFG_H_

#include "sta_canif.h"
#include "sta_canif_cfg.h"
#include "sta_canif_types.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"

/************************************************************************
| defines and macros
|-----------------------------------------------------------------------*/


#define CAN_SDU_SIZE	8

// this definitions belong to CAN Controller (HW)
#define MAX_TX_PDU	(uint8_t) 16
#define MAX_RX_PDU	(uint8_t) 16

#define CAN_CONTROLER_NETWORK_SIMBOLIC_1              0 //Simbolic Name network 1
#define CAN_CONTROLER_NETWORK_SIMBOLIC_2              0 //Simbolic Name network 2
#define CAN_CONTROLER_NETWORK_SIMBOLIC_3              0 //Simbolic Name network 3


#endif
