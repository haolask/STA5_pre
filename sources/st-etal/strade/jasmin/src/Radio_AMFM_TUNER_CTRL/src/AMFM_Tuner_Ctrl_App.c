
/*=============================================================================
    start of file
=============================================================================*/
/*************************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_App.c																		                     *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														             *
*  All rights reserved. Reproduction in whole or part is prohibited											             *
*  without the written permission of the copyright owner.													             *
*																											             *
*  Project              : ST_Radio_Middleware																		                     *
*  Organization			: Jasmin Infotech Pvt. Ltd.															             *
*  Module				: SC_AMFM_TUNER_CTRL																             *
*  Description			: Definitions of main hsm handler message and instance hsm handler message                       *                     
                                                                                                                         *																											*
*																											             *
**************************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "AMFM_Tuner_Ctrl_App.h"
#include "AMFM_Tuner_Ctrl_Response.h"
#include <math.h>

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
extern Ts_AMFM_Tuner_Ctrl_Timer_Ids st_AMFM_Tuner_Ctrl_Timer_Ids;


/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/

Ts_AMFM_Tuner_Ctrl_Main_hsm      st_tuner_ctrl_package;

Tu8 		scan_index_AM=0;
Tu8 		TA;
Tu8 		TP;

SOC_QUAL AMFMTuner_FMQual_parametrs;
SOC_QUAL AMFMTuner_AMQual_parametrs;
SOC_QUAL AMFMTuner_AMScanQual_parametrs;
SOC_QUAL AMFMTuner_AMFastScanQual_parametrs;
SOC_QUAL AMFMTuner_AMSeekQual_parameters;
SOC_QUAL AMFMTuner_AMFastSeekQual_parameters;

SOC_QUAL AMFMTuner_FMScanQual_parametrs;
SOC_QUAL AMFMTuner_FMFastScanQual_parametrs;
SOC_QUAL st_AMFMTuner_FMSeekQual_parameters;
SOC_QUAL st_AMFMTuner_FMFastSeekQual_parameters;
SOC_QUAL AMFMTuner_TunerStatusNot;
EtalBcastQualityContainer st_amfm_tuner_ctrl_Quality_params;
SOC_QUAL AMFMTuner_Tune_AFQuality;
Ts_Sys_Msg	st_intmsg;
Ts_Sys_Msg st_sendmsg;
Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStationInfo;


#ifdef CALIBRATION_TOOL

#define CS_FM_RFFieldStrength		    (Ts32)0x10		
#define CS_Multipath		    	    (Tu32)0x20
#define CS_UltrasonicNoise		    	(Tu32)0x2D
#define CS_FM_FOF    					(Tu32)0x0A

#define CS_AM_RFFieldStrength			(Ts32)0x19
#define CS_AM_FOF						(Tu32)0x02


/*Tu8 	CS_QUAL			=	35;
Tu8 	CS_FAST_QUAL	=	29;*/
Tu32 	CS_QUAL			=	25;        /**<Defined real threshold value,parameters are FS,USN,WAM & OFS*/
Tu32 	CS_FAST_QUAL	=	15;        /**<Defined fast threshold value,parameters are FS,USN,WAM & OFS*/
#endif

/*-----------------------------------------------------------------------------
    private function declarations
-----------------------------------------------------------------------------*/
/*=========================================================================== */
/*			void AMFM_Tuner_Ctrl_MessageInit										  */
/*=========================================================================== */
void AMFM_Tuner_Ctrl_MessageInit(Ts_Sys_Msg *pst_msg, Tu16 u16_DestID, Tu16 u16_MsgID)
{
	if (pst_msg != NULL)
	{
		/* Clearing the message buffer */
		memset((void *)pst_msg, 0x00, sizeof(Ts_Sys_Msg));

		pst_msg->dest_id = u16_DestID;
		pst_msg->msg_id = u16_MsgID;

	}
	else
	{
		// Send error message
	}
}
/*===========================================================================*/
/*  Ts_Sys_Msg*  SYS_MSG_HANDLE										 */
/*===========================================================================*/
Ts_Sys_Msg* SYS_MSG_HANDLE(Tu16 cid, Tu16 msgid)
{

	memset(&st_sendmsg, 0x00, sizeof(st_sendmsg));
	st_sendmsg.dest_id = cid;
	st_sendmsg.msg_id = msgid;
	return (&st_sendmsg);
}

/*===========================================================================*/
/* AMFM_Tuner_ctrl_UpdateParameterIntoMessage                                        */
/*===========================================================================*/
void AMFM_Tuner_ctrl_UpdateParameterIntoMessage(Tchar *pu8_data, const void *vp_parameter, Tu16 u8_ParamLength, Tu16 *pu16_Datalength)
{
	/* copying parameter to the data slots in Ts_Sys_Msg structure */
	SYS_RADIO_MEMCPY((pu8_data + *pu16_Datalength), vp_parameter, (size_t)(u8_ParamLength));

	/*  Updating msg_length for each parameter which represents length of data in  Ts_Sys_Msg structure   */
	*pu16_Datalength = (Tu16)(*pu16_Datalength + u8_ParamLength);
}

/*===========================================================================*/
/*AMFM_Tuner_ctrl_ExtractParameterFromMessage                                                                                  */
/*=========================================================================== */
void AMFM_Tuner_ctrl_ExtractParameterFromMessage(void *vp_Parameter, const Tchar *pu8_DataSlot, Tu16 u16_ParamLength, Tu32 *pu32_index)
{
	/* reading parameter from the data slot present in Ts_Sys_Msg structure  */
	SYS_RADIO_MEMCPY(vp_Parameter, pu8_DataSlot + *pu32_index, (size_t)(u16_ParamLength));

	/*  Updating index inorder to point to next parameter present in the data slot in  Ts_Sys_Msg structure   */
	*pu32_index = (Tu32)(*pu32_index + u16_ParamLength);
}


/*===========================================================================*/
/* void AMFM_TUNER_CTRL_MSG_HandleMsg    									 */
/*===========================================================================*/
void AMFM_TUNER_CTRL_MSG_HandleMsg(Ts_Sys_Msg* pst_msg)
{
	HSM_ON_MSG((Ts_hsm*)(&st_tuner_ctrl_package), pst_msg);
}


/*===========================================================================*/
/*  void  AMFM_TUNER_CTRL_MSG_INST_HandleMsg								 */
/*===========================================================================*/

void AMFM_TUNER_CTRL_MSG_INST_HandleMsg(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst,Ts_Sys_Msg* pst_msg)
{
  /* comment has to change using the case for RES/ case for REQ */
 
	if((pst_me_amfm_tuner_ctrl_inst != NULL) && (pst_msg != NULL))
	{
	      HSM_ON_MSG(pst_me_amfm_tuner_ctrl_inst, pst_msg);
			
	}	
	else
	{
		/* Send error message */
	}
}


void AMFM_Tuner_Ctrl_Component_Init(void)
{

	AMFM_TUNER_CTRL_MAIN_HSM_Init(&st_tuner_ctrl_package);

}


/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_startRDSDecode       						         */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_startRDSDecode(Tu16 cid, Tu16 msg_id, Tu32 u32_freq)
{
	Ts_Sys_Msg* msg = NULL;
	msg = SYS_MSG_HANDLE(cid, msg_id);
	AMFM_Tuner_ctrl_UpdateParameterIntoMessage(msg->data, &u32_freq,sizeof(Tu32),&(msg->msg_length));
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_startreply									         */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_startreply(Tu16 cid,Tu16 msg_id)
{
	Ts_Sys_Msg* msg = NULL;
	msg = SYS_MSG_HANDLE(cid, msg_id);
	SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_stop									             */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_stop(Tu16 cid,Tu16 msg_id)
{
   Ts_Sys_Msg* msg = NULL;
   msg = SYS_MSG_HANDLE(cid, msg_id);
   SYS_SEND_MSG(msg);
}

void AMFM_Tuner_Ctrl_stopreply(Tu16 u16_cid,Tu16 msg_id)
{
    Ts_Sys_Msg* msg = NULL;
	msg = SYS_MSG_HANDLE(u16_cid, msg_id);
	 SYS_SEND_MSG(msg);
}

/*===========================================================================*/
/*  void AMFM_Tuner_Ctrl_SOC_Reply								             */
/*===========================================================================*/
void  AMFM_Tuner_Ctrl_SOC_Reply(Tu16 cid,Tu16 msg_id)
{
	Ts_Sys_Msg* pst_msg = &st_intmsg;

	AMFM_Tuner_Ctrl_MessageInit(pst_msg ,cid,msg_id);
		
   	SYS_SEND_MSG(pst_msg);
}


void AMFM_Tuner_Ctrl_QualUpdate_AM_Scan(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_AM_QualParams_Conversion(pst_me_inst, pBcastQualityContainer);

	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][AMFM_TC] : AM scan is in progress Frequency is %d , Scan index is %d",pst_me_inst->st_Getstationreq.U32_Lowerfreq,scan_index_AM);
	ast_Scaninfo[scan_index_AM].u32_freq 								= 	pst_me_inst->u32_Seek_Freq;
	ast_Scaninfo[scan_index_AM].e_Band									=   pst_me_inst->st_Getstationreq.e_Band;
	ast_Scaninfo[scan_index_AM].s32_RFFieldStrength 					= 	pst_me_inst->st_interpolation.s32_RFFieldStrength;
	ast_Scaninfo[scan_index_AM].u32_Multipath 							= 	pst_me_inst->st_interpolation.u32_Multipath;
	ast_Scaninfo[scan_index_AM].u32_UltrasonicNoise 					= 	pst_me_inst->st_interpolation.u32_UltrasonicNoise;
	ast_Scaninfo[scan_index_AM].u32_FrequencyOffset 					= 	pst_me_inst->st_interpolation.u32_FrequencyOffset;
	ast_Scaninfo[scan_index_AM].u32_quality_interpolation               =	pst_me_inst->st_interpolation.u32_interpolation;

	scan_index_AM = (Tu8)(scan_index_AM + (Tu8)(1));

	if (scan_index_AM >= AMFM_TUNER_CTRL_MAX_STATIONS)
	{
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
	}
	else
	{
		st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_AUTOSCAN_AUDIO_PLAY_TIME, AM_FM_TUNER_CTRL_SOC_AMSCAN_CONTINUE_MSGID, RADIO_AM_FM_TUNER_CTRL);
		if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : AM Scan continue start timer is failed");
		}
		else
		{
			/* MISRA C */
		}
	}
}

/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_QualUpdate_AM_Learn  */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_QualUpdate_AM_Learn(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_AM_QualParams_Conversion(pst_me_inst, pBcastQualityContainer);

	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_TC] : AM scan is in progress Frequency is %d , Scan index is %d", pst_me_inst->st_Getstationreq.U32_Lowerfreq, scan_index_AM);
	ast_Scaninfo[scan_index_AM].u32_freq					= pst_me_inst->u32_Seek_Freq;
	ast_Scaninfo[scan_index_AM].e_Band						= pst_me_inst->st_Getstationreq.e_Band;
	ast_Scaninfo[scan_index_AM].s32_RFFieldStrength			= pst_me_inst->st_interpolation.s32_RFFieldStrength;
	ast_Scaninfo[scan_index_AM].u32_Multipath				= pst_me_inst->st_interpolation.u32_Multipath;
	ast_Scaninfo[scan_index_AM].u32_UltrasonicNoise			= pst_me_inst->st_interpolation.u32_UltrasonicNoise;
	ast_Scaninfo[scan_index_AM].u32_FrequencyOffset			= pst_me_inst->st_interpolation.u32_FrequencyOffset;
	ast_Scaninfo[scan_index_AM].u32_quality_interpolation	= pst_me_inst->st_interpolation.u32_interpolation;

	scan_index_AM = (Tu8)(scan_index_AM + (Tu8)(1));

	if (scan_index_AM >= AMFM_TUNER_CTRL_MAX_STATIONS)
	{
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_STOP_MSGID);
	}
	else
	{
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AM_FM_TUNER_CTRL_SOC_AMSCAN_CONTINUE_MSGID);
	}
}

/********************************************************************************************/
/* AMListen Check*/
/********************************************************************************************/
void AMFM_Tuner_Ctrl_GetListenQuality(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	
	if(pst_me_inst->st_Current_StationInfo.e_Band == TUN_BAND_FM)
	{
		AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(pst_me_inst, pBcastQualityContainer);
	}
	else
	{
		AMFM_Tuner_Ctrl_AM_QualParameters_Conversion(pst_me_inst, pBcastQualityContainer);
	}
	
	pst_me_inst->st_Current_StationInfo.u32_freq 				= 	pst_me_inst->st_Tunereq.u32_freq;	
	pst_me_inst->st_Current_StationInfo.e_Band 					= 	pst_me_inst->st_Tunereq.e_Band;
		
	AMFM_Tuner_Ctrl_Current_Station_Quality_Update(pBcastQualityContainer, pst_me_inst);
	AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_FMAMTUNER_STATUS_NOTIFICATION);
}

/********************************************************************************************/
/* AMFM_Tuner_Ctrl_GetjumpQualityCheck                                                   	*/
/********************************************************************************************/
void AMFM_Tuner_Ctrl_GetjumpQualityCheck(EtalBcastQualityContainer* ST_Tuner_FMQual_Check, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	
	//if(pst_me_inst->st_interpolation.u8_interpolation < CS_QUAL)
	if ((ST_Tuner_FMQual_Check->EtalQualityEntries.amfm.m_BBFieldStrength < CS_FM_RFFieldStrength) || (ST_Tuner_FMQual_Check->EtalQualityEntries.amfm.m_UltrasonicNoise > CS_UltrasonicNoise) || (ST_Tuner_FMQual_Check->EtalQualityEntries.amfm.m_Multipath> CS_Multipath) || (ST_Tuner_FMQual_Check->EtalQualityEntries.amfm.m_FrequencyOffset  > CS_FM_FOF))
	{
		/* send failure response of Tune command*/
		pst_me_inst->st_Current_StationInfo.e_Band 		= 	TUN_BAND_FM;
		pst_me_inst->st_Current_StationInfo.u32_freq 	= 	pst_me_inst->st_Tunereq.u32_freq;	
		pst_me_inst->st_Current_StationInfo.SOC_Status 	= 	REPLYSTATUS_FAILURE;
		
		AMFM_Tuner_Ctrl_Current_Station_Quality_Update(ST_Tuner_FMQual_Check, pst_me_inst);
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);
			
	}
	else
	{
		/* send Success response of Tune command*/
		pst_me_inst->st_Current_StationInfo.e_Band 	 = 	TUN_BAND_FM;
		pst_me_inst->st_Current_StationInfo.u32_freq = 	pst_me_inst->st_Tunereq.u32_freq;
		
		AMFM_Tuner_Ctrl_Current_Station_Quality_Update(ST_Tuner_FMQual_Check, pst_me_inst);
		
		pst_me_inst->st_Current_StationInfo.u8_NumAFeqList = 0;

		memset(pst_me_inst->st_Current_StationInfo.u32_AFeqList,0x00,sizeof(pst_me_inst->st_Current_StationInfo.u32_AFeqList));
	
		pst_me_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_SUCCESS;

		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);
	}	
}
/********************************************************************************************/
void AMFM_Tuner_Ctrl_QualUpdate_FM_Scan(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm  *pst_me_amfm_tuner_ctrl_inst)
{
	AMFM_Tuner_Ctrl_FM_QualParams_Conversion(pst_me_amfm_tuner_ctrl_inst, pBcastQualityContainer);
	
	ast_Scaninfo[scan_index_FM].u32_freq 						= 		pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
	ast_Scaninfo[scan_index_FM].e_Band   						=  		pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
	ast_Scaninfo[scan_index_FM].s32_RFFieldStrength 			= 		pst_me_amfm_tuner_ctrl_inst->st_interpolation.s32_RFFieldStrength;
	ast_Scaninfo[scan_index_FM].u32_Multipath	 				= 		pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_Multipath;
	ast_Scaninfo[scan_index_FM].u32_UltrasonicNoise 			= 		pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_UltrasonicNoise;
	ast_Scaninfo[scan_index_FM].u32_FrequencyOffset 			= 		pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_FrequencyOffset;
	ast_Scaninfo[scan_index_FM].u32_quality_interpolation		=		pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation;
	if ((pst_me_amfm_tuner_ctrl_inst->e_Market == AMFM_TUNER_CTRL_ASIA_CHINA) || (pst_me_amfm_tuner_ctrl_inst->e_Market == AMFM_TUNER_CTRL_JAPAN))
	{
		/* Incrementing STL index */
		scan_index_FM = (Tu8)(scan_index_FM + (Tu8)1);
		if (scan_index_FM >= AMFM_TUNER_CTRL_MAX_STATIONS)
		{
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
		}
		else
		{
			st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_AUTOSCAN_AUDIO_PLAY_TIME, AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID, RADIO_AM_FM_TUNER_CTRL);
			if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : FM Scan continue start timer is failed");
			}
			else
			{
				/* MISRA C */
			}
		}
	}
	else
	{
		st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id = SYS_TIMER_START(AMFM_TUNER_CTRL_AUTOSCAN_AUDIO_PLAY_TIME, AMFM_TUNER_CTRL_SCAN_RDS_START_REQID, RADIO_AM_FM_TUNER_CTRL);
		if (st_AMFM_Tuner_Ctrl_Timer_Ids.u32_AMFM_TC_Timer_Id == 0)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][AMFM_TC]  : FM Scan continue start timer is failed");
		}
		else
		{
			/* MISRA C */
		}
	}
}

/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_GetFMQualityCheck_Tune */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_QualUpdate_FM_Learn(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm  *pst_me_amfm_tuner_ctrl_inst)
{
	AMFM_Tuner_Ctrl_FM_QualParams_Conversion(pst_me_amfm_tuner_ctrl_inst, pBcastQualityContainer);

	ast_Scaninfo[scan_index_FM].u32_freq					= pst_me_amfm_tuner_ctrl_inst->u32_Seek_Freq;
	ast_Scaninfo[scan_index_FM].e_Band						= pst_me_amfm_tuner_ctrl_inst->st_Getstationreq.e_Band;
	ast_Scaninfo[scan_index_FM].s32_RFFieldStrength			= pst_me_amfm_tuner_ctrl_inst->st_interpolation.s32_RFFieldStrength;
	ast_Scaninfo[scan_index_FM].u32_Multipath				= pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_Multipath;
	ast_Scaninfo[scan_index_FM].u32_UltrasonicNoise			= pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_UltrasonicNoise;
	ast_Scaninfo[scan_index_FM].u32_FrequencyOffset			= pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_FrequencyOffset;
	ast_Scaninfo[scan_index_FM].u32_quality_interpolation	= pst_me_amfm_tuner_ctrl_inst->st_interpolation.u32_interpolation;
	if ((pst_me_amfm_tuner_ctrl_inst->e_Market == AMFM_TUNER_CTRL_ASIA_CHINA) || (pst_me_amfm_tuner_ctrl_inst->e_Market == AMFM_TUNER_CTRL_JAPAN))
	{
		/* Incrementing STL index */
		scan_index_FM = (Tu8)(scan_index_FM + (Tu8)1);
		if (scan_index_FM >= AMFM_TUNER_CTRL_MAX_STATIONS)
		{
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_DONE_RESID);
		}
		else
		{
			AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID);

		}
	}
	else
	{
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_SCAN_RDS_START_REQID);
	}

}

/*******************************************************************************************************************************/	

void AMFM_Tuner_Ctrl_Current_Station_Quality_Update(EtalBcastQualityContainer *st_amfm_tuner_ctrl_Quality_params , Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
//	pst_me_inst->st_Current_StationInfo.u8_status 								=  	AMFMTuner_FMQual_Check->u8_status;
	pst_me_inst->st_Current_StationInfo.s32_BBFieldStrength 					=  	pst_me_inst->st_interpolation.s32_BBFieldStrength;
	pst_me_inst->st_Current_StationInfo.u32_Multipath 					        =  	pst_me_inst->st_interpolation.u32_Multipath;
	pst_me_inst->st_Current_StationInfo.u32_UltrasonicNoise 					=  	pst_me_inst->st_interpolation.u32_UltrasonicNoise;
	pst_me_inst->st_Current_StationInfo.u32_FrequencyOffset 					=  	pst_me_inst->st_interpolation.u32_FrequencyOffset;
	pst_me_inst->st_Current_StationInfo.u32_ModulationDetector					=   st_amfm_tuner_ctrl_Quality_params->EtalQualityEntries.amfm.m_ModulationDetector;
	pst_me_inst->st_Current_StationInfo.u32_quality_interpolation               =	pst_me_inst->st_interpolation.u32_interpolation;
	
}

void AMFM_Tuner_Ctrl_Update_Rcvd_RDS_Data(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_AMFM_Tuner_Ctrl_RDS_Data st_recv_data)
{
	if(st_recv_data.b_NonRDS_Station == TRUE)
	{
		/* Received station does not have the RDS */
	}
	else
	{
		if(st_recv_data.u16_PI_Code != (Tu16)0)
		{
			/* Updating PI value from RDS data to Current station info structure */
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u16_pi = st_recv_data.u16_PI_Code;
		}
		if (st_recv_data.b_PTY_Update == TRUE)
		{
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_PTYCode = st_recv_data.u8_PTY_Code;
		}
		if (st_recv_data.b_TA_Update == TRUE)
		{
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_TA = st_recv_data.st_FM_RDS_TA.u8_TA;
			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_TP = st_recv_data.st_FM_RDS_TA.u8_TP;
		}
		if(st_recv_data.b_PSN_Update == TRUE)
		{
			SYS_RADIO_MEMCPY(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_ps, st_recv_data.st_FM_RDS_PSN.u8_ps,AMFM_TUNER_CTRL_MAX_PS_SIZE);
		}
		
		if(st_recv_data.b_AF_Update == TRUE)
		{	
			/*Copying the AF method A/B  into tuner control current station info*/
			if(st_recv_data.st_FM_RDS_AF.b_AF_MethodB != FALSE)
			{
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_AF_Methods = AMFM_TUNER_CTRL_AF_METHOD_B ;
			}
			else 
			{
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.e_AF_Methods = AMFM_TUNER_CTRL_AF_METHOD_A;	
			}

			if(st_recv_data.st_FM_RDS_AF.u8_Num_Same_Pgm != (Tu8)0)
			{
				SYS_RADIO_MEMCPY(&pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList[0], 
					   	&st_recv_data.st_FM_RDS_AF.u32_Same_AF_Freq[0], 
						st_recv_data.st_FM_RDS_AF.u8_Num_Same_Pgm*(sizeof(Tu32)));
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumofAF_SamePgm = st_recv_data.st_FM_RDS_AF.u8_Num_Same_Pgm;
			}

			if(st_recv_data.st_FM_RDS_AF.u8_Num_Rgnl_Pgm != (Tu8)0)
			{
				SYS_RADIO_MEMCPY(&pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u32_AFeqList[st_recv_data.st_FM_RDS_AF.u8_Num_Same_Pgm], 
						&st_recv_data.st_FM_RDS_AF.u32_Rgnl_AF_Freq[0], 
						st_recv_data.st_FM_RDS_AF.u8_Num_Rgnl_Pgm*(sizeof(Tu32)));
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumofAF_RgnlPgm = st_recv_data.st_FM_RDS_AF.u8_Num_Rgnl_Pgm;
			}

			pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_NumAFeqList = (Tu8)(st_recv_data.st_FM_RDS_AF.u8_NumAFeqList);

		}
		if(st_recv_data.b_RT_Update == TRUE)
		{
			memset(pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_RadioText, 0x00, AMFM_TUNER_CTRL_MAX_RT_SIZE);

			if(st_recv_data.st_FM_RDS_RT.u8_RT_len <= AMFM_TUNER_CTRL_MAX_RT_SIZE )
			{
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_RT_size = st_recv_data.st_FM_RDS_RT.u8_RT_len;
			
				SYS_RADIO_MEMCPY(&pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.au8_RadioText[0], 
			       &st_recv_data.st_FM_RDS_RT.u8_RT_Data[0], 
				   st_recv_data.st_FM_RDS_RT.u8_RT_len);
			}
			else
			{
				pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.u8_RT_size = 0;
			}
				   

		}		
		if(st_recv_data.b_CT_Update == TRUE)	
		{
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Hour = st_recv_data.st_FM_RDS_CT.u8_Hour;	
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Min = st_recv_data.st_FM_RDS_CT.u8_Min;	
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Localtime_offset = st_recv_data.st_FM_RDS_CT.u8_Decode_Localtime_offset;	
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_offset_sign = st_recv_data.st_FM_RDS_CT.u8_offset_sign;	
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Day = st_recv_data.st_FM_RDS_CT.u8_Day;	
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u8_Month = st_recv_data.st_FM_RDS_CT.u8_Month;	
		   pst_me_amfm_tuner_ctrl_inst->st_Current_StationInfo.st_CT_Info.u16_Year = st_recv_data.st_FM_RDS_CT.u16_Curr_Year;	
		}
		if(pst_me_amfm_tuner_ctrl_inst->b_PI_Updated == TRUE)
		{
			if(st_recv_data.b_TA_Update == TRUE)
			{
				/* Need to be  update */
				pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u16_EON_PI			=	st_recv_data.st_FM_RDS_TA.u16_EONpi;
				pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[0]   = st_recv_data.st_FM_RDS_TA.u32_AF[0];
				pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[1]   = st_recv_data.st_FM_RDS_TA.u32_AF[1];

				TA = st_recv_data.st_FM_RDS_TA.u8_TA;
				TP = st_recv_data.st_FM_RDS_TA.u8_TP;

				if((TP == (Tu8)1) && (TA == (Tu8)1))
				{
    				if(	pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage == FALSE)
					{
						/* Send Bit Status */
						AMFM_Tuner_Ctrl_Send_TAStatus(TA,TP);
						pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage = TRUE;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : Traffic Announcement is broadcasted at present TA=%d , TP=%d, b_TATP_Flage=%d",TA,TP,pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage);
					}

				}
				else if(((TP == (Tu8)0) && (TA == (Tu8)1)) || ((TP == (Tu8)1) && (TA == (Tu8)0)) )
				{
				if((pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage == TRUE) || (pst_me_amfm_tuner_ctrl_inst->b_EON_Flage == TRUE))
				{
					/* Send Bit Status that cleared the Annc */
					AMFM_Tuner_Ctrl_Send_TAStatus(TA,TP);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] :OnGoing Traffic Announcement is Ended  TA=%d, TP=%d TA_STATUS =%d,EON_TA_STAUS=%d",TA,TP,pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage,pst_me_amfm_tuner_ctrl_inst->b_EON_Flage);
					pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage = FALSE;
				}
				else if(pst_me_amfm_tuner_ctrl_inst->b_EON_Flage == FALSE)
				{
					/* Send Bit Status */
					AMFM_Tuner_Ctrl_Send_TAStatus(TA,TP);
					pst_me_amfm_tuner_ctrl_inst->b_EON_Flage = TRUE;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : EON station is broadcasting Announcement TA=%d, TP=%d TA_STATUS =%d,EON_TA_STAUS=%d",TA,TP,pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage,pst_me_amfm_tuner_ctrl_inst->b_EON_Flage);
				}

				}
				else if((TP == (Tu8)0) && (TA == (Tu8)0))
				 {
					if((pst_me_amfm_tuner_ctrl_inst->b_EON_Flage == TRUE) ||(pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage == TRUE))
				 	{
						/*Send the TATP Status TA is Cleared */
						AMFM_Tuner_Ctrl_Send_TAStatus(TA,TP);
						pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage = FALSE;	
						pst_me_amfm_tuner_ctrl_inst->b_EON_Flage = FALSE;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : Traffic Announcement is cleared TA=%d,TP=%d, TA_STATUS =%d,EON_TA_STAUS=%d",TA,TP,pst_me_amfm_tuner_ctrl_inst->b_TATP_Flage,pst_me_amfm_tuner_ctrl_inst->b_EON_Flage);
					 }

				}
				else
				{
					/* Do Nothing */
				}
			}
		}
		if((st_recv_data.b_EON_AF_Update != FALSE))
		{
			pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u16_EON_PI = st_recv_data.st_FM_RDS_TA.u16_EONpi;
			pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[0] = st_recv_data.st_FM_RDS_TA.u32_AF[0];
			pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[1] = st_recv_data.st_FM_RDS_TA.u32_AF[1];
		
		//	if(((pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[0] > 0)&&(pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[0] < 1081))||
		//	((pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[1] > 0)&&(pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo.u32_EON_aflist[1] < 1081)))
			{
				AMFM_Tuner_Ctrl_EON_TAdata(pst_me_amfm_tuner_ctrl_inst->st_EON_StationInfo);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][AMFM_TC] : EON DATA is TX at present PI=%x , AF1=%d, AF2=%d", st_recv_data.st_FM_RDS_TA.u16_EONpi, st_recv_data.st_FM_RDS_TA.u32_AF[0], st_recv_data.st_FM_RDS_TA.u32_AF[1]);
			}
		}
		if(pst_me_amfm_tuner_ctrl_inst->b_PI_Updated == TRUE)
		{
			if(st_recv_data.b_EONTA_Update == TRUE)
			{
				if((st_recv_data.st_FM_RDS_TA.u16_EONpi != (Tu16)0))
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][AMFM_TC] : EON Announcement is Started with PI=%x",st_recv_data.st_FM_RDS_TA.u16_EONpi);
					AMFM_Tuner_Ctrl_Send_EONTA(st_recv_data.st_FM_RDS_TA.u16_EONpi);
				}
			}
		}
	}
}

void AMFM_Tuner_Ctrl_AM_QualParameters_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer* st_amfm_tuner_ctrl_Quality)
{
	memset(&(pst_me_inst->st_interpolation),0x00,sizeof(pst_me_inst->st_interpolation));
	st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_FrequencyOffset = (Tu8)(LIB_AND((FOF_EXTRACT_MASK_VALUE), (st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_FrequencyOffset)));          //macro addition
	
	/*Converting QualParameters and updating into st_interpolation structure, present in Ts_AMFM_Tuner_Ctrl_Inst_hsm structure*/
	pst_me_inst->st_interpolation.s32_BBFieldStrength = AMFM_FS_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_BBFieldStrength);//s32_BBFieldStrength);
	pst_me_inst->st_interpolation.u32_UltrasonicNoise = AMFM_USN_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_UltrasonicNoise);//u32_UltrasonicNoise);
	pst_me_inst->st_interpolation.u32_Multipath = AMFM_WAM_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_Multipath);//u32_Multipath);
	pst_me_inst->st_interpolation.u32_FrequencyOffset = AMFM_OFS_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_FrequencyOffset);//u32_FrequencyOffset);
	pst_me_inst->st_interpolation.s32_RFFieldStrength = st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_RFFieldStrength;//s32_RFFieldStrength;
	pst_me_inst->st_interpolation.u32_AdjacentChannel = st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_AdjacentChannel;//u32_AdjacentChannel;
	pst_me_inst->st_interpolation.u32_ModulationDetector = st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_ModulationDetector;//u32_ModulationDetector;

}

void AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer* st_amfm_tuner_ctrl_Quality)
{
	memset(&(pst_me_inst->st_interpolation),0x00,sizeof(pst_me_inst->st_interpolation));
	st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_FrequencyOffset	= (Tu8)(LIB_AND((FOF_EXTRACT_MASK_VALUE), (st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_FrequencyOffset)));          //macro addition
	/*Converting QualParameters and updating into st_interpolation structure, present in Ts_AMFM_Tuner_Ctrl_Inst_hsm structure*/
	pst_me_inst->st_interpolation.s32_BBFieldStrength						= AMFM_FS_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_BBFieldStrength);//->s32_BBFieldStrength);
	pst_me_inst->st_interpolation.u32_UltrasonicNoise						= AMFM_USN_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_UltrasonicNoise);//u32_UltrasonicNoise);
	pst_me_inst->st_interpolation.u32_Multipath								= AMFM_WAM_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_Multipath);//u32_Multipath);
	pst_me_inst->st_interpolation.u32_FrequencyOffset						= AMFM_OFS_CONVERSION(st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_FrequencyOffset);// u32_FrequencyOffset);
	pst_me_inst->st_interpolation.s32_RFFieldStrength						= st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_RFFieldStrength;// s32_RFFieldStrength;
	pst_me_inst->st_interpolation.u32_SNR									= st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_SNR;// u32_SNR;
	pst_me_inst->st_interpolation.u32_AdjacentChannel						= st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_AdjacentChannel;// u32_AdjacentChannel;
	pst_me_inst->st_interpolation.u32_coChannel								= st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_coChannel;// u32_coChannel;
	pst_me_inst->st_interpolation.u32_ModulationDetector					= st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_ModulationDetector;// u32_ModulationDetector;
	pst_me_inst->st_interpolation.u32_StereoMonoReception					= st_amfm_tuner_ctrl_Quality->EtalQualityEntries.amfm.m_StereoMonoReception;// u32_StereoMonoReception;
	pst_me_inst->st_interpolation.u32_interpolation 						= QUAL_CalcFmQual(pst_me_inst->st_interpolation.s32_BBFieldStrength, pst_me_inst->st_interpolation.u32_UltrasonicNoise, 
																	           pst_me_inst->st_interpolation.u32_Multipath,NULL ,NULL, pst_me_inst->st_interpolation.u32_FrequencyOffset);

}

/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_GetFMQualityCheck_Tune */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_GetFMQualityCheck_Tune(EtalBcastQualityContainer *pBcastQuality_Container_FM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_FM_QualParams_Conversion(pst_me_inst,pBcastQuality_Container_FM);
	
	if (pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_RFFieldStrength < CS_FM_RFFieldStrength) 
	{
		/* send failure response of Tune command*/
		pst_me_inst->st_Current_StationInfo.e_Band 					= 	TUN_BAND_FM;
		pst_me_inst->st_Current_StationInfo.u32_freq 				= 	pst_me_inst->st_Tunereq.u32_freq;	
		pst_me_inst->st_Current_StationInfo.SOC_Status              = 	REPLYSTATUS_FAILURE;
		AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_FM,pst_me_inst);	
	}
	else
	{
		pst_me_inst->st_Current_StationInfo.e_Band 	   = TUN_BAND_FM;
		pst_me_inst->st_Current_StationInfo.u32_freq   = pst_me_inst->st_Tunereq.u32_freq;
		pst_me_inst->st_Current_StationInfo.SOC_Status = REPLYSTATUS_SUCCESS;
        AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_FM,pst_me_inst);
     	memset(pst_me_inst->st_Current_StationInfo.u32_AFeqList, 0x00, sizeof(pst_me_inst->st_Current_StationInfo.u32_AFeqList));
		pst_me_inst->st_Current_StationInfo.u8_NumAFeqList = 0;
	}
	AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);	
}


/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_GetAMQualityCheck_Tune */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_GetAMQualityCheck_Tune(EtalBcastQualityContainer *pBcastQuality_Container_AM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst )
{
	AMFM_Tuner_Ctrl_AM_QualParams_Conversion(pst_me_inst,pBcastQuality_Container_AM);
	
	if(pBcastQuality_Container_AM->EtalQualityEntries.amfm.m_RFFieldStrength < CS_AM_RFFieldStrength) 
	{
		/* send failure response of Tune command*/
		pst_me_inst->st_Current_StationInfo.u32_freq					= 	pst_me_inst->st_Tunereq.u32_freq;					
		pst_me_inst->st_Current_StationInfo.e_Band 						= 	pst_me_inst->st_Tunereq.e_Band;
		pst_me_inst->st_Current_StationInfo.SOC_Status					= 	REPLYSTATUS_FAILURE;
	}
	else
	{
		/* send success response of Tune command*/
		pst_me_inst->st_Current_StationInfo.u32_freq 					= 	pst_me_inst->st_Tunereq.u32_freq;	
		pst_me_inst->st_Current_StationInfo.e_Band 						= 	pst_me_inst->st_Tunereq.e_Band;
		pst_me_inst->st_Current_StationInfo.SOC_Status 					= 	REPLYSTATUS_SUCCESS;
		AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_AM,pst_me_inst);
	}	
	AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_TUNE_DONE_RESID);
}


/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_GetBG_FMQualityCheck_Tune */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_GetBG_FMQualityCheck_Tune(EtalBcastQualityContainer *pBcastQuality_Container_FM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_FM_QualParams_Conversion(pst_me_inst, pBcastQuality_Container_FM);
	
	if (pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_RFFieldStrength < CS_FM_RFFieldStrength)
	{
		/* send failure response of Tune command*/
		pst_me_inst->st_Current_StationInfo.u32_freq					= 	pst_me_inst->st_Tunereq.u32_freq;					
		pst_me_inst->st_Current_StationInfo.e_Band 						= 	pst_me_inst->st_Tunereq.e_Band;
		pst_me_inst->st_Current_StationInfo.SOC_Status					= 	REPLYSTATUS_FAILURE;
		AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_FM, pst_me_inst);
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_BGTUNE_DONE_RESID);
	}
	else
	{
		/* send success response of Tune command*/
		pst_me_inst->st_Current_StationInfo.u32_freq 					= 	pst_me_inst->st_Tunereq.u32_freq;	
		pst_me_inst->st_Current_StationInfo.e_Band 						= 	pst_me_inst->st_Tunereq.e_Band;
		pst_me_inst->st_Current_StationInfo.SOC_Status 					= 	REPLYSTATUS_SUCCESS;
		memset(pst_me_inst->st_Current_StationInfo.u32_AFeqList, 0x00, sizeof(pst_me_inst->st_Current_StationInfo.u32_AFeqList));
		pst_me_inst->st_Current_StationInfo.u8_NumAFeqList = 0;
		AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_FM, pst_me_inst);
		AMFM_Tuner_Ctrl_SOC_Reply(RADIO_AM_FM_TUNER_CTRL, AMFM_TUNER_CTRL_RDS_DONE_RESID);
	}	
	
}


/********************************************************************************************/
/* void ETAl_AMFM_Tuner_Ctrl_AM_QualParameters_Conversion */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_AM_QualParams_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer *pBcastQuality_Container_AM)
{
	memset(&(pst_me_inst->st_interpolation),0x00,sizeof(pst_me_inst->st_interpolation));
     
    /*Converting QualParameters and updating into st_interpolation structure, present in Ts_AMFM_Tuner_Ctrl_Inst_hsm structure*/
	pst_me_inst->st_interpolation.s32_RFFieldStrength			= AMFM_FS_CONVERSION(pBcastQuality_Container_AM->EtalQualityEntries.amfm.m_RFFieldStrength);
	pst_me_inst->st_interpolation.s32_BBFieldStrength			= AMFM_FS_CONVERSION(pBcastQuality_Container_AM->EtalQualityEntries.amfm.m_BBFieldStrength);
	pst_me_inst->st_interpolation.u32_FrequencyOffset			= AMFM_OFS_CONVERSION(pBcastQuality_Container_AM->EtalQualityEntries.amfm.m_FrequencyOffset);
	pst_me_inst->st_interpolation.u32_ModulationDetector		= AMFM_MOD_CONVERSION(pBcastQuality_Container_AM->EtalQualityEntries.amfm.m_ModulationDetector);
	pst_me_inst->st_interpolation.u32_AdjacentChannel			= AMFM_ADJ_CONVERSION(pBcastQuality_Container_AM->EtalQualityEntries.amfm.m_AdjacentChannel);
}

/********************************************************************************************/
/* void ETAl_AMFM_Tuner_Ctrl_FM_QualParameters_Conversion */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_FM_QualParams_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer *pBcastQuality_Container_FM)
{
	memset(&(pst_me_inst->st_interpolation),0x00,sizeof(pst_me_inst->st_interpolation));
	
	/*Converting QualParameters and updating into st_interpolation structure, present in Ts_AMFM_Tuner_Ctrl_Inst_hsm structure*/
	pst_me_inst->st_interpolation.s32_BBFieldStrength			= AMFM_FS_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_BBFieldStrength);
	pst_me_inst->st_interpolation.s32_RFFieldStrength			= AMFM_FS_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_RFFieldStrength);
	pst_me_inst->st_interpolation.u32_Multipath					= AMFM_WAM_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_Multipath);
	pst_me_inst->st_interpolation.u32_UltrasonicNoise			= AMFM_USN_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_UltrasonicNoise);
	pst_me_inst->st_interpolation.u32_FrequencyOffset			= AMFM_OFS_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_FrequencyOffset);
	pst_me_inst->st_interpolation.u32_AdjacentChannel			= AMFM_ADJ_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_AdjacentChannel);
	pst_me_inst->st_interpolation.u32_ModulationDetector		= AMFM_MOD_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_ModulationDetector);
	pst_me_inst->st_interpolation.u32_SNR						= AMFM_SNR_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_SNR);
	pst_me_inst->st_interpolation.u32_coChannel					= AMFM_COCHANNEL_CONVERSION(pBcastQuality_Container_FM->EtalQualityEntries.amfm.m_coChannel);
	pst_me_inst->st_interpolation.u32_interpolation 	        =  QUAL_CalcFmQual(pst_me_inst->st_interpolation.s32_RFFieldStrength, pst_me_inst->st_interpolation.u32_UltrasonicNoise,
                                                                   pst_me_inst->st_interpolation.u32_Multipath,NULL ,NULL, pst_me_inst->st_interpolation.u32_FrequencyOffset);
}
/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_Current_Station_Quality_Update */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_Current_Station_Qual_Update(EtalBcastQualityContainer *pBcastQuality_Container, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	
	pst_me_inst->st_Current_StationInfo.s32_BBFieldStrength 	  = pst_me_inst->st_interpolation.s32_BBFieldStrength;
	pst_me_inst->st_Current_StationInfo.s32_RFFieldStrength       = pst_me_inst->st_interpolation.s32_RFFieldStrength; 
	pst_me_inst->st_Current_StationInfo.u32_Multipath 			  = pst_me_inst->st_interpolation.u32_Multipath;
	pst_me_inst->st_Current_StationInfo.u32_UltrasonicNoise 	  = pst_me_inst->st_interpolation.u32_UltrasonicNoise;
	pst_me_inst->st_Current_StationInfo.u32_FrequencyOffset		  = pst_me_inst->st_interpolation.u32_FrequencyOffset;
	pst_me_inst->st_Current_StationInfo.u32_AdjacentChannel		  = pst_me_inst->st_interpolation.u32_AdjacentChannel; /* Revisit: For ADJ Conversion formula instead of USN */
	pst_me_inst->st_Current_StationInfo.u32_ModulationDetector    = pst_me_inst->st_interpolation.u32_ModulationDetector;
	pst_me_inst->st_Current_StationInfo.u32_SNR					  = pst_me_inst->st_interpolation.u32_SNR;
	pst_me_inst->st_Current_StationInfo.u32_coChannel		      = pst_me_inst->st_interpolation.u32_coChannel;
	pst_me_inst->st_Current_StationInfo.u32_StereoMonoReception   = pBcastQuality_Container->EtalQualityEntries.amfm.m_StereoMonoReception;
	pst_me_inst->st_Current_StationInfo.u32_quality_interpolation =	pst_me_inst->st_interpolation.u32_interpolation;

}
/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_QualUpdate_FG_FM_Seek */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_QualUpdate_FG_FM_Seek(EtalBcastQualityContainer *pBcastQuality_Container_FM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_FM_QualParams_Conversion(pst_me_inst,pBcastQuality_Container_FM);

	pst_me_inst->st_Current_StationInfo.e_Band = TUN_BAND_FM;
	pst_me_inst->st_Current_StationInfo.u32_freq = pst_me_inst->u32_Seek_Freq;
	pst_me_inst->e_SOC_ReplyStatus = REPLYSTATUS_SUCCESS;

	AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_FM, pst_me_inst);
}
/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_QualUpdate_FG_FM_PISeek */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_QualUpdate_FG_FM_PISeek(EtalBcastQualityContainer *pBcastQuality_Container_FM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_FM_QualParams_Conversion(pst_me_inst, pBcastQuality_Container_FM);

	pst_me_inst->st_Current_StationInfo.e_Band   = TUN_BAND_FM;
	pst_me_inst->st_Current_StationInfo.u32_freq = pst_me_inst->u32_Seek_Freq;

	AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_FM, pst_me_inst);
}

/********************************************************************************************/
/* void AMFM_Tuner_Ctrl_QualUpdate_FG_AM_Seek */
/********************************************************************************************/
void AMFM_Tuner_Ctrl_QualUpdate_FG_AM_Seek(EtalBcastQualityContainer *pBcastQuality_Container_AM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst)
{
	AMFM_Tuner_Ctrl_AM_QualParams_Conversion(pst_me_inst,pBcastQuality_Container_AM);

	pst_me_inst->st_Current_StationInfo.u32_freq = pst_me_inst->u32_Seek_Freq;
	pst_me_inst->st_Current_StationInfo.e_Band = pst_me_inst->st_Getstationreq.e_Band;
	pst_me_inst->e_SOC_ReplyStatus = REPLYSTATUS_SUCCESS;

	AMFM_Tuner_Ctrl_Current_Station_Qual_Update(pBcastQuality_Container_AM, pst_me_inst);
}
/*=============================================================================
    end of file
=============================================================================*/

