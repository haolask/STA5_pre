/*=============================================================================
start of file
=============================================================================*/

/************************************************************************************************************/
/** \file osal_private.h																			    	*
*  Copyright (c) 2017, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : STM Radio																	        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains declarations of osal private									*
*																											*
*************************************************************************************************************/

#ifndef _OSAL_PRIVATE_H
#define _OSAL_PRIVATE_H

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#ifdef UITRON
#include <itron.h>
#include <kernel.h>
#include "kernel_id.h"
#elif defined OS_WIN32
#include "win32_os_private.h"
#endif
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#define MAX_NUM_COMP 				6			/* NUMBER OF APPLICATION COMPONENTS*/
#define SYS_TASK_CNT        		(Tu8)4		/* MAX NUMBER OF TASKS FOR RADIO_LIB*/
#define MIN_TASK_CNT            	2
#define MIN_MUTEX_COUNT     		2			/* MIN NUMBER OF MUTEX*/
#define MAX_MUTEX_COUNT     		6			/* MAX NUMBER OF MUTEX*/
#define TASK_STATUS_SIZE			0x0F		/* TASK ACTIVE/DEACTIVE STATUS SIZE */

#define MAIL_BX_PRI  				1			/* MAILBOX PRIORITY ATTRIBUTE FOR MBX CREATION*/
#define SEM_CNT_VAL  				1			/* SEMAPHORE COUNT VALUE*/
#define MAX_SEM_VAL  				1			/* MAX SEMAPHORE VALUE*/
#ifdef UITRON
#define MEMPOOL_BLK_CNT  			0x12		/* MEMORYPOOL BLOCK COUNT IS 18*/
#define	DAB_TUNER_CTRL_MEMPOOL_BLK_CNT	0X1D
#define MEMPOOL_BLK_SZ   			0x100		/* MEMORYPOOL BLOCK SIZE IS 256*/
#endif

/* COMPONENT ID'S*/
#define RADIO_MNGR_APP 				0x2000    	/* RADIO MNGR APPLICATION ID*/   
#define RADIO_DAB_APP				0x200A		/* RADIO DAB APPLICATION ID*/
#define RADIO_AM_FM_APP				0x2014		/* RADIO AMFM APPLICATION ID*/
#define RADIO_DAB_TUNER_CTRL		0x201E		/* RADIO DAB TUNER CTRL ID*/
#define RADIO_AM_FM_TUNER_CTRL		0x2028		/* RADIO AMFM TUNER CTRL ID*/
#define RADIO_FM_RDS_DECODER		0x2032		/* RADIO FM RDS DECODER ID*/
#define RADIO_I2C_READ_WRITE   		0x203C		/* RADIO I2C READ WRITE ID*/
#define CALIBRATIONTOOL_TIMER_ID	0x2046		/* CALIBRATION TOOL ID*/
#define RADIO_DAB_SPI_READ			0x2050		/* DAB SPI READ ID */
#define UNKNOWN_APP					0x2fff		/* UNKNOWN APP FOR ERROR CASE*/

/*MUTEX LOCK ID'S */
#define STL_RM_AMFM_APP				0x00		/* MUTEX LOCK BETWEEN RM AND AMFM_APP FOR STATION LIST*/
#define STL_AMFM_APP_AMFM_TC		0x01		/* MUTEX LOCK BETWEEN AMFM_APP AND AMFM_TC FOR STATION LIST*/
#define STL_RM_DAB_APP				0x02		/* MUTEX LOCK BETWEEN RM AND DAB_APP FOR STATION LIST,ENSEMBLE INFO,SERVICE INFO AND COMPONENT INFO*/
#define STL_DAB_APP_DAB_TC			0x03		/* MUTEX LOCK BETWEEN DAB_APP AND DAB_TC FOR STATION LIST,ENSEMBLE INFO,SERVICE INFO AND COMPONENT INFO*/
#define SLS_RM_DAB_APP				0x04		/* MUTEX LOCK BETWEEN RM AND DAB_APP FOR STATION LIST,ENSEMBLE INFO,SERVICE INFO AND COMPONENT INFO*/
#define SLS_DAB_APP_DAB_TC			0x05		/* MUTEX LOCK BETWEEN DAB_APP AND DAB_TC FOR STATION LIST,ENSEMBLE INFO,SERVICE INFO AND COMPONENT INFO*/

/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
#ifdef UITRON
typedef struct Ts_task_inf
{
	ER_ID 	taskid;
	ER_ID 	mbxid;
	ER_ID 	mpfid;
	ER_ID 	sempid;
	Tu8 	Num_of_Comp;
	Tu8 	task_status[TASK_STATUS_SIZE];
	Tu16 	Comp_Info[MAX_NUM_COMP];
}Ts_task_info;

#elif defined OS_WIN32
typedef struct Ts_task_inf
{
	Tu32 	taskid;
	Tu32 	*mbxid;
	Ts32 	sempid;
	Tu8 	Num_of_Comp;
	Tu8 	task_status[TASK_STATUS_SIZE];
	Tu16 	Comp_Info[MAX_NUM_COMP];
	Tu32	terminateEvent;
}Ts_task_info;
#endif

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variable declarations (static)
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

#endif /* OSAL_PRIVATE_H */
/*=============================================================================
    end of file
=============================================================================*/
