/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_can_irq.h
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN driver interrupt service routines
*                      definitions
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

#ifndef _CAN_IRQ_H_
#define _CAN_IRQ_H_

#include "sta_can.h"
#include "sta_canif.h"
#include "sta_canif_cfg.h"
#include "sta_canif_types.h"
#include "sta_can_irq.h"
#include "sta_can_p.h"

// Private LLD prototypes
extern uint8_t CCCanLLD_ControllerDefaultState(volatile CCan * CAN_ptr, uint16_t Mode);
extern uint8_t CCanLLD_CheckIFxReady(volatile CCan * CAN_ptr, uint8_t ifc);
extern void CCanLLD_StopController(volatile CCan * CAN_ptr);
extern void CCanLLD_StartController(volatile CCan * CAN_ptr);
extern uint8_t CCanLLD_CheckController(uint8_t Controller);
extern uint8_t CCanLLD_CheckIFxMessObjReady(volatile CCan * CAN_ptr, uint8_t ifc, uint16_t mo, uint16_t what );
extern uint8_t CCanLLD_CheckIFxMessObjFlush(volatile CCan * CAN_ptr, uint8_t ifc, uint16_t mo, uint16_t what );

#endif
