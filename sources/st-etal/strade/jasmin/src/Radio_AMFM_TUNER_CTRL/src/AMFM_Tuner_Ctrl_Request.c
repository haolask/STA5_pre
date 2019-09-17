
/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Request.c                                                                         *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: SC_AMFM_TUNER_CTRL                                                                *
*  Description			:  AMFM tuner control Requesting api's definitions  .                               *
*                                                                                                           *
*                                                                                                           * 
*************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "AMFM_Tuner_Ctrl_Request.h"
#include "AMFM_Tuner_Ctrl_Instance_hsm.h"
#include "AMFM_Tuner_Ctrl_App.h"

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables 
-----------------------------------------------------------------------------*/
Ts_AMFM_Tuner_Ctrl_Tunereq_info st_Tunereq;

Ts_Sys_Msg st_reqmsg;
/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Startup									 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Request_Startup(Te_AMFM_Tuner_Ctrl_Market e_Market)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_STARTUP_REQID);
	/* Function to assign e_Market value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Market, (Tu16)(sizeof(Te_AMFM_Tuner_Ctrl_Market)), &(pst_reqmsg->msg_length));
	
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Shutdown									 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Shutdown(void)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_SHUTDOWN_REQID);
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_GetStationList							 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Request_GetStationList(Tu32 U32_Lowerfreq,Tu32 U32_Startfreq,Tu32 U32_Upperfreq,Tu32 U32_Stepsize,Te_AMFM_Tuner_Ctrl_Band e_Band ,Te_AMFM_Scan_Type Scantype )
//void AMFM_Tuner_Ctrl_Request_GetStationList(Tu16 U32_Lowerfreq,Tu16 U32_Startfreq,Tu16 U32_Upperfreq,Tu32 U32_Stepsize,Te_AMFM_Tuner_Ctrl_Band e_Band )
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_GETSTATIONLIST_REQID);
	
    /* Function to assign U32_Startfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Startfreq, (Tu16)(sizeof(U32_Startfreq)), &(pst_reqmsg->msg_length));
    /* Function to assign U32_Upperfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Upperfreq, (Tu16)(sizeof(U32_Upperfreq)), &(pst_reqmsg->msg_length));
    /* Function to assign U32_Lowerfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Lowerfreq, (Tu16)(sizeof(U32_Lowerfreq)), &(pst_reqmsg->msg_length));
    /* Function to assign U32_Stepsize value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Stepsize, (Tu16)(sizeof(U32_Stepsize)), &(pst_reqmsg->msg_length));
    /* Function to assign e_Band value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Band, (Tu16)(sizeof(e_Band)), &(pst_reqmsg->msg_length));
	
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &Scantype, (Tu16)(sizeof(Scantype)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}


/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Tune									     */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Tune(Te_AMFM_Tuner_Ctrl_Band e_Band,Tu32 u32_freq)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_TUNE_REQID);
    /* Function to assign u32_freq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u32_freq, (Tu16)(sizeof(u32_freq)), &(pst_reqmsg->msg_length));
    /* Function to assign e_Band value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Band, (Tu16)(sizeof(e_Band)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Activate									 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Activate(Te_AMFM_Tuner_Ctrl_Band e_Band)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_ACTIVATE_REQID);
    /* Function to assign e_Band value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Band, (Tu16)(sizeof(e_Band)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_DeActivate								 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_DeActivate(Te_AMFM_Tuner_Ctrl_Band e_Band)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_DEACTIVATE_REQID);
    /* Function to assign e_Band value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Band, (Tu16)(sizeof(e_Band)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_SeekUpDown								 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_SeekUpDown(Te_RADIO_DirectionType e_Direction, Tu32 U32_Lowerfreq, Tu32 U32_Startfreq, Tu32 U32_Upperfreq, Tu32 u32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band )
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_SEEK_UP_DOWN_REQID);

    /* Function to assign U32_Startfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Direction, (Tu16)(sizeof(e_Direction)), &(pst_reqmsg->msg_length));
    /* Function to assign U32_Lowerfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Lowerfreq, (Tu16)(sizeof(U32_Lowerfreq)), &(pst_reqmsg->msg_length));
	/* Function to assign U32_Startfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Startfreq, (Tu16)(sizeof(U32_Startfreq)), &(pst_reqmsg->msg_length));
	/* Function to assign U32_Upperfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Upperfreq, (Tu16)(sizeof(U32_Upperfreq)), &(pst_reqmsg->msg_length));

    /* Function to assign U32_Stepsize value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u32_Stepsize, (Tu16)(sizeof(u32_Stepsize)), &(pst_reqmsg->msg_length));
	/* Function to assign e_Mode value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &e_Band, (Tu16)(sizeof(e_Band)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_PISeek								 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_PISeek(Tu32 U32_Lowerfreq, Tu32 U32_Startfreq, Tu32 U32_Upperfreq, Tu32 u32_Stepsize,Tu16 u16_pi)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;


	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_PISEEK_REQID);

    /* Function to assign U32_Lowerfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Lowerfreq, (Tu16)(sizeof(U32_Lowerfreq)), &(pst_reqmsg->msg_length));
	/* Function to assign U32_Startfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Startfreq, (Tu16)(sizeof(U32_Startfreq)), &(pst_reqmsg->msg_length));
	/* Function to assign U32_Upperfreq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &U32_Upperfreq, (Tu16)(sizeof(U32_Upperfreq)), &(pst_reqmsg->msg_length));

    /* Function to assign U32_Stepsize value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u32_Stepsize, (Tu16)(sizeof(u32_Stepsize)), &(pst_reqmsg->msg_length));
	/* Function to assign e_Mode value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u16_pi, (Tu16)(sizeof(Tu16)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Bg_Quality								 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Bg_Quality(void)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_BGQUALITY_REQID);
	SYS_SEND_MSG(pst_reqmsg);
}
/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_AF_Update							     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_AF_Update(Tu32 u32_freq)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_AF_UPDATE_REQID);
	 /* Function to assign u32_freq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u32_freq, (Tu16)(sizeof(Tu32)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Cancel							     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Cancel(void)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_CANCEL_REQID);
	SYS_SEND_MSG(pst_reqmsg);
	
}
/*===========================================================================*/
/*  void  void AMFM_Tuner_Ctrl_Request_Announcement_Cancel							     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Announcement_Cancel(void)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_ANNOUNCEMENT_CANCEL_REQID);
	SYS_SEND_MSG(pst_reqmsg);
	
}
/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_AF_Update							     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_FM_Check(Tu32 u32_freq)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_AF_CHECK_REQID);
	 /* Function to assign u32_freq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u32_freq, (Tu16)(sizeof(Tu32)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_LowSignal_FM_Check				     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_LowSignal_FM_Check(Tu16 u16_freq)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_LOWSIGNAL_AF_CHECK_REQID);
	/* Function to assign U16_freq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u16_freq, (Tu16)(sizeof(Tu16)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_AF_Update							     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_FM_Jump(Tu32 u32_freq)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_AF_JUMP_REQID);
	 /* Function to assign u32_freq value in the corresponding message slot */
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage((Tchar *)(pst_reqmsg->data), &u32_freq, (Tu16)(sizeof(Tu32)), &(pst_reqmsg->msg_length));
	SYS_SEND_MSG(pst_reqmsg);
}
/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_Request_Factory_Reset							     	 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_Request_Factory_Reset(void)
{
	Ts_Sys_Msg* pst_reqmsg =&st_reqmsg;
	AMFM_Tuner_Ctrl_MessageInit(pst_reqmsg ,RADIO_AM_FM_TUNER_CTRL,AMFM_TUNER_CTRL_FACTORY_RESET_REQID);
	SYS_SEND_MSG(pst_reqmsg);
}

/*===========================================================================*/
/*  void  AMFM_Tuner_Ctrl_hsm_inst_start									 */
/*===========================================================================*/
void AMFM_Tuner_Ctrl_hsm_inst_start(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl, Tu16 u16_msgid)
{
	if(pst_me_amfm_tuner_ctrl != NULL)
	{	
		Ts_Sys_Msg   *pst_Internal_msg = NULL;			/* pointer to message structure defined globally */
		
		/* Updating header information like dest id,src id ,pst_reqmsg id  in st_amfm_app_req_msg structure defined in amfm_app_request.c */	
		pst_Internal_msg = SYS_MSG_HANDLE(RADIO_AM_FM_TUNER_CTRL, u16_msgid);
		
		HSM_ON_MSG(&pst_me_amfm_tuner_ctrl->hsm,pst_Internal_msg);	
	}
	else
	{
		/* Send error message */
	}	
}

