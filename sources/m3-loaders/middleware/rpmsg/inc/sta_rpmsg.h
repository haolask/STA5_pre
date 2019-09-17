/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_rpmsg.h
* Author             : APG-MID Application Team
* Date First Issued  : 12/17/2013
* Description        : This file provides the RPMSG definitions specific to
*                      STA platform.
********************************************************************************
* History:
* 03/13/2014: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _STA_RPMSG_H_
#define _STA_RPMSG_H_

#include "sta_lib.h"


/* ---------------------------------------------------------
   These definitions come from <linux/stm_rpmsg.h> and
   may change as the remote processor interface is updated.
   Make sure they match the ones used by your current kernel
   source.
   ---------------------------------------------------------
*/
/**
 * enum css_msg_types - various message types currently supported
 *
 * @CSS_CTRL_REQ: a control request message type.
 * @CSS_CTRL_RSP: a response to a control request.
 * @CSS_RAW_MSG: a message that should be propagated as-is to the user.
 */
enum css_msg_types {
	CSS_CTRL_REQ = 0,
	CSS_CTRL_RSP = 1,
	CSS_RAW_MSG = 2,
	CSS_CAN_LISTEN = 3,
	CSS_ACC_LISTEN = 4,
	CSS_CAN_RX = 5,
	CSS_ACC_RX = 6,
};

/**
 * enum css_ctrl_types - various control codes that will be used
 *
 * @CSS_CONNECTION: connection request
 * @CSS_DISCONNECT: connction disconnection
 * @CSS_ERROR: remote processor error
 */
enum css_ctrl_types {
	CSS_CONNECTION = 0,
	CSS_DISCONNECT = 1,
	CSS_SUCCESS = 2,
	CSS_ERROR = 3,
};

struct css_msg_hdr {
	uint32_t type;
	uint32_t len;
	int data[0];
} __attribute__ ((packed));

struct css_ctrl_rsp {
	uint32_t status;
	uint32_t addr;
} __attribute__ ((packed));

struct css_ctrl_req {
	uint32_t request;
	uint32_t addr;
} __attribute__ ((packed));


/*-----------------------------------------------------------------------------*
   DEFINEs
 *-----------------------------------------------------------------------------*/
#define INIT_NOT_STARTED    0
#define INIT_ONGOING        2
#define INIT_DONE           3


/*-----------------------------------------------------------------------------*
   TYPEDEF
 *-----------------------------------------------------------------------------*/
/**
 * struct RPMSG_DeviceTy - RPMSG device data
 * @MboxChanID: Mailbox channel ID allocated to RPMSG framework
 * @RscTable:	base address of the resource table
 */
typedef struct
{
	int			            MboxChanID;
	struct resource_table   *RscTable;
} RPMSG_DeviceTy;


/*-----------------------------------------------------------------------------*
   VARIABLEs
 *-----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
   FUNCTION INTERFACE
 *----------------------------------------------------------------------------*/
int     RPMSG_PreInit(void);
int     RPMSG_Init(void);
void    RPMSG_U32Send(uint32_t arg);
void	RPMSG_disable_rx(void);
void	RPMSG_enable_rx(void);

#endif  /* _STA_RPMSG_H_ */
