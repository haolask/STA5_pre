/*============================================================================================================
start of file
=============================================================================================================*/
/************************************************************************************************************/
/** \file DAB_HAL_Interface.c																			    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: RADIO_DAB_HAL														     	        *
*  Description			: This file contains definitions of functions that interface DAB Tuner Ctrl with    *
*                         SoC		                                                                    *
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
includes
-----------------------------------------------------------------------------*/
#include "DAB_HAL_Interface.h"
#include "DAB_Tuner_Ctrl_Request.h"
#include "lib_bitmanip.h"
#include "DAB_HAL_FIC_Parsing.h"
#include "DAB_Tuner_Ctrl_Response.h"

ETAL_HANDLE dabFG_recvhdl = ETAL_INVALID_HANDLE;
ETAL_HANDLE dabBG_recvhdl = ETAL_INVALID_HANDLE;
ETAL_HANDLE dabFG_datapath = ETAL_INVALID_HANDLE;
ETAL_HANDLE dabBG_datapath = ETAL_INVALID_HANDLE;
ETAL_HANDLE dabFG_FICData = ETAL_INVALID_HANDLE;
ETAL_HANDLE dabFG_Monitor_Handle = ETAL_INVALID_HANDLE;

EtalEnsembleinfo					Dabtuner_EtalEnsmblinfo;	//FOR DAB ENSEMBLE PROPERTIES
EtalServiceList						Dabtuner_Etalservinfo;		//For Service List
EtalServiceComponentList			pSCInfo;					//For Service Component Info
EtalServiceInfo						pServiceInfo;				//For Service properties

EtalBcastQualityContainer			Etal_DabAudioQuality;		//For DAB Quality Info
EtalServiceSelectMode				Etal_servselMode;
Ts_Dabtunerctrl_EtalQualityEntries	st_DabTuner_Qualityparams;	// Temp structure to store quality params - To be removed in future
EtalBcastQualityMonitorAttr			st_BcastQualityMonitor;
Tu32								Scanfrequency = 0;
Ts_DAB_DataServiceRaw				st_SLS_Data;
EtalDataServiceType					e_Enabled_Servicesbitmap;
/*================================================================================================*/
/*  Ts_DabTunerMsg_sCmd* DabTuner_MsgGetSndCmdPtr( void )                                         */
/*================================================================================================*/

Ts_DabTunerMsg_sCmd* DabTuner_MsgGetSndCmdPtr(void)
{
	Ts_DabTunerMsg_sCmd *ret = NULL;
	//ret = DABTuner_MsgGetSndCmdPtr();
	return ret;
}
/*================================================================================================*/
/* Tbool DabTuner_MsgRcvUpNot(Ts_DabTunerMsg_N_UpNot * DAB_TunerRcvUpNotification,
char * msg)                                            */
/*================================================================================================*/
Tbool DabTuner_MsgRcvUpNot(Ts_DabTunerMsg_N_UpNot * DAB_TunerRcvUpNotification, char * msg)
{
	Tbool b_ret = FALSE;
	UNUSED(DAB_TunerRcvUpNotification);
	UNUSED(msg);
	// b_ret = DABTuner_MsgRcvUpNot(DAB_TunerRcvUpNotification, msg);
	return b_ret;
}
/*================================================================================================*/
/*  Tbool DabTuner_SystemError_not(Ts_SystemError_not *pst_SystemError_not, Ts8 *pu8_Msg)       */
/*================================================================================================*/

Tbool DabTuner_SystemError_not(Ts_SystemError_not *pst_SystemError_not, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_SystemError_not);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_SystemError_not(pst_SystemError_not, pu8_Msg);
	return b_ret;
}

/*================================================================================================*/
/*  void DabTuner_MsgSndCreateReciver_Cmd()                                                       */
/*================================================================================================*/

void DabTuner_MsgSndCreateReciver_Cmd(void)
{
	//DABTuner_MsgSndCreateReciver_Cmd();	
}

/*================================================================================================*/
/*  void DabTuner_MsgSndVersionRequest_Cmd()                                                      */
/*================================================================================================*/
void DabTuner_MsgSndVersionRequest_Cmd(void)
{
	//DABTuner_MsgSndVersionRequest_Cmd();
}
/*================================================================================================*/
/*  Tbool DabTuner_MsgRcvVersionRequest(Ts_DabTunerMsg_R_GetVersion*
                                                     DAB_Tuner_Ctrl_GetVersionReply,char *msgdata)*/
/*================================================================================================*/

Tbool DabTuner_MsgRcvVersionRequest(Ts_DabTunerMsg_R_GetVersion *DAB_Tuner_Ctrl_CreateRcvrReply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(DAB_Tuner_Ctrl_CreateRcvrReply);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvVersionRequest(DAB_Tuner_Ctrl_CreateRcvrReply,msgdata); 
	return b_ret;
}
/*================================================================================================*/
/* Tbool DabTuner_MsgRcvCreateReceiver(Ts_DabTunerMsg_R_CreateReceiver *
                                                    DAB_Tuner_Ctrl_CreateRcvrReply,char *msgdata) */
/*================================================================================================*/
Tbool DabTuner_MsgRcvCreateReceiver(Ts_DabTunerMsg_R_CreateReceiver * DAB_Tuner_Ctrl_CreateRcvrReply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(DAB_Tuner_Ctrl_CreateRcvrReply);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvCreateReceiver(DAB_Tuner_Ctrl_CreateRcvrReply,msgdata);
	return b_ret;
}
/*================================================================================================*/
/*  void DabTuner_MsgSndTuneTo_Cmd( unsigned  int u32Freq )                                       */
/*================================================================================================*/

ETAL_STATUS DabTuner_MsgSndTuneTo_Cmd(Tu32 u32Freq)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	ETAL_Ret = etal_tune_receiver(dabFG_recvhdl, u32Freq);

	return ETAL_Ret;
}
/*================================================================================================*/
/*  void DabTuner_MsgSndSetTuneTocmd( void)                                                       */
/*================================================================================================*/

void DabTuner_MsgSndSetTuneTocmd(Tu8 u8_Trigger_Options)
{
	UNUSED(u8_Trigger_Options);
	//DABTuner_MsgSndSetTuneTocmd();
}

/*================================================================================================*/
/*  Tbool DabTuner_MsgRcvSetTuneStatus(Ts_DabTunerMsg_R_SetTuneStatus * SetTuneNot,
                                                    char *msgdata)                                */
/*================================================================================================*/

Tbool DabTuner_MsgRcvSetTuneStatus(Ts_DabTunerMsg_R_SetTuneStatus * SetTuneNot, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(SetTuneNot);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSetTuneStatus(SetTuneNot,msgdata);	
	return b_ret;
}

/*================================================================================================*/
/*  Tbool DabTuner_MsgRcvTuneTo(Ts_DabTunerMsg_R_TuneTo *msgdata,char *msg)          */
/*================================================================================================*/

Tbool DabTuner_MsgRcvTuneTo(Ts_DabTunerMsg_R_TuneTo *msgdata, char *msg)
{
	Tbool b_ret = FALSE;
	UNUSED(msgdata);
	UNUSED(msg);
	//ret = DABTuner_MsgRcvTuneTo(msgdata,msg);
	return b_ret;
}

/*================================================================================================*/
/* ETAL_STATUS DabTuner_Msg_GetCurrEnsemble_cmd(void)         */
/*================================================================================================*/
ETAL_STATUS DabTuner_Msg_GetCurrEnsemble_cmd(void)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	ETAL_Ret = etal_get_current_ensemble(dabFG_recvhdl, &Dabtuner_EtalEnsmblinfo.pUEId);

	return ETAL_Ret;
}

/*================================================================================================*/
/*  Tbool DabTuner_MsgRcvSetTuneStatusNot(Ts_DabTunerMsg_R_SetTuneStatusNot *msgdata,
char* msg)                                 */
/*================================================================================================*/
Tbool DabTuner_MsgRcvSetTuneStatusNot(Ts_DabTunerMsg_R_SetTuneStatusNot *msgdata, char* msg)
{
	Tbool b_ret = FALSE;
	UNUSED(msgdata);
	UNUSED(msg);
	//b_ret = DABTuner_MsgRcvSetTuneStatusNot(msgdata,msg);
	return b_ret;
}
/*================================================================================================*/
/* ETAL_STATUS DabTunerCtrl_StartScan2_cmd(Tbool b_Scanstarted, Te_DAB_Tuner_Ctrl_RequestCmd e_RequestCmd)								         */
/*================================================================================================*/
ETAL_STATUS DabTunerCtrl_StartScan2_cmd(Tbool b_Scanstarted, Te_DAB_Tuner_Ctrl_RequestCmd e_RequestCmd)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;
	Tbool b_UpdateStopFreq = FALSE;
	etalSeekAudioTy e_AudiomuteORUnmute;

	if (e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN) 
	{
		/* For SCAN */
		e_AudiomuteORUnmute = cmdAudioUnmuted;
	}
	else/* For LEARN */
	{
		e_AudiomuteORUnmute = cmdAudioMuted;
	}
	/* Start of the scan */
	if (b_Scanstarted == FALSE)
	{
		b_UpdateStopFreq = TRUE;
	}
	else /* Scan is in progress */
	{
		b_UpdateStopFreq = FALSE;
	}

	/* If it is scan, always call etal_autoseek_start. If it is learn, call etal_autoseek_start only for the first time */
	/* Successive learn calls should be only through etal_autoseek_continue */
	if (e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN || (e_RequestCmd == DAB_TUNER_CTRL_LEARN && b_UpdateStopFreq == TRUE)) // SCAN AND LEARN
	{
		ETAL_Ret = etal_autoseek_start(dabFG_recvhdl, cmdDirectionUp, ETAL_SEEK_STEP_UNDEFINED, e_AudiomuteORUnmute, dontSeekInSPS, b_UpdateStopFreq);
	}
	else if (e_RequestCmd == DAB_TUNER_CTRL_LEARN && b_UpdateStopFreq == FALSE) // FOR LEARN
	{
		ETAL_Ret = etal_autoseek_continue(dabFG_recvhdl);
	}
	else
	{
		/* For MISRA */
	}
	
	return ETAL_Ret;
}
/*===========================================================================================================*/
/*  ETAL_STATUS DabTuner_AutoSeekStart_Cmd(Te_RADIO_DirectionType e_SeekDirection, Tbool b_UpdateStopFreq)	 */									     
/*===========================================================================================================*/
ETAL_STATUS DabTuner_AutoSeekStart_Cmd(Te_RADIO_DirectionType e_SeekDirection, Tbool b_UpdateStopFreq)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;
	etalSeekDirectionTy e_AutoSeekDirection = cmdDirectionUp;	//Initialisation done for Lint Warnings

	/* SEEK direction*/
	if(e_SeekDirection == RADIO_FRMWK_DIRECTION_UP)
	{
		e_AutoSeekDirection = cmdDirectionUp;
	}
	else if(e_SeekDirection == RADIO_FRMWK_DIRECTION_DOWN)
	{
		e_AutoSeekDirection = cmdDirectionDown;
	}
	
	ETAL_Ret = etal_autoseek_start(dabFG_recvhdl, e_AutoSeekDirection, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, dontSeekInSPS, b_UpdateStopFreq);

	return ETAL_Ret;
}
/*================================================================================================*/
/*  ETAL_STATUS DabTuner_AbortAutoSeek_Cmd(void)												      */
/*================================================================================================*/
ETAL_STATUS DabTuner_AbortAutoSeek_Cmd(void)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	ETAL_Ret = etal_autoseek_stop(dabFG_recvhdl, lastFrequency);

	return ETAL_Ret;
}
/*================================================================================================*/
/*  ETAL_STATUS DabTuner_AbortScan2_Cmd(Te_DAB_Tuner_Ctrl_RequestCmd e_RequestCmd)												      */
/*================================================================================================*/
ETAL_STATUS DabTuner_AbortScan2_Cmd(Te_DAB_Tuner_Ctrl_RequestCmd e_RequestCmd)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;
	EtalSeekTerminationModeTy e_AutoseekexitFreq;
	if (e_RequestCmd == DAB_TUNER_CTRL_BANDSCAN)
	{
		/* For SCAN */
		e_AutoseekexitFreq = lastFrequency;
	}
	else  /* For LEARN */
	{
		e_AutoseekexitFreq = initialFrequency;
	}
	ETAL_Ret = etal_autoseek_stop(dabFG_recvhdl, e_AutoseekexitFreq);
	return ETAL_Ret;
}
/*================================================================================================*/
/*  Tbool DabTuner_MsgRcv_AbortScan2Reply(Ts_Select_ComponentReply * msgdata,char *msg )         */
/*================================================================================================*/
Tbool DabTuner_MsgRcv_AbortScan2Reply(Ts_AbortScan2Reply * msgdata, char *msg)
{
	Tbool b_ret = FALSE;
	UNUSED(msgdata);
	UNUSED(msg);
	//b_ret = DABTuner_MsgRcv_AbortScan2Reply(msgdata,msg);
	return 	b_ret;

}
/*================================================================================================*/
/*  DabTuner_AbortSearch_cmd(void)															      */
/*================================================================================================*/
void DabTuner_AbortSearch_cmd(void)
{
	//DABTuner_AbortSearch_cmd();

}
/*===========================================================================================================*/
/*  Tbool DabTuner_MsgRcvStartScan(Ts_DabTunerMsg_R_StartScan_Cmd *DAB_Tuner_Ctrl_ScanReply,char *msgdata)   */

/*===========================================================================================================*/
Tbool DabTuner_MsgRcvStartScan(Ts_DabTunerMsg_R_StartScan_Cmd * DAB_Tuner_Ctrl_ScanReply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(msgdata);
	UNUSED(DAB_Tuner_Ctrl_ScanReply);
	//b_ret = DABTuner_MsgRcvStartScan(DAB_Tuner_Ctrl_ScanReply,msgdata);	
	return 	b_ret;
}

/*===============================================================================================================*/
/* Tbool DabTuner_MsgRcvScanStatusNot(Ts_DabTunerMsg_R_ScanStatus_Not *DAB_Tuner_Ctrl_ScanNotification )*/
/*===============================================================================================================*/
Tbool DabTuner_MsgRcvScanStatusNot(Ts_DabTunerMsg_R_ScanStatus_Not *DAB_Tuner_Ctrl_ScanNotification)
{
	Tbool b_ret = FALSE;
	if (DAB_Tuner_Ctrl_ScanNotification != NULL)
	{
		DAB_Tuner_Ctrl_ScanNotification->frequency = Scanfrequency;
		b_ret = TRUE;
	}
	return b_ret;
}

/*================================================================================================*/
/*  void DabTuner_MsgSndScan2GetComponentListReq_Cmd(Tu32 Sid)                                    */
/*================================================================================================*/
ETAL_STATUS DabTuner_MsgSndScan2GetComponentListReq_Cmd(Tu32 Sid)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	ETAL_Ret = etal_get_specific_service_data_DAB(Dabtuner_EtalEnsmblinfo.pUEId, Sid, &pServiceInfo, &pSCInfo, NULL);

	return ETAL_Ret;
}

/*=========================================================================================================================================*/
/*Tbool DabTuner_GetServListpropertiesReply(Ts_CurrEnsemble_serviceinfo * ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply)*/

/*==========================================================================================================================================*/
Tbool DabTuner_GetServListpropertiesReply(Ts_CurrEnsemble_serviceinfo * ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply)
{
	Tbool b_ret = FALSE;
	Tu8 Servindex = 0;
	Tu8 Lablindx = 0;
	if (ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply != NULL)
	{
		for (Servindex = 0; Servindex < Dabtuner_Etalservinfo.m_serviceCount; Servindex++)
		{
			if (ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply->ProgServiceId == Dabtuner_Etalservinfo.m_service[Servindex])
			{
				ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply->CharSet = pServiceInfo.m_serviceLabelCharset;
				ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply->ShortLabelFlags = pServiceInfo.m_serviceLabelCharflag;
				for (Lablindx = 0; Lablindx < ETAL_DEF_MAX_LABEL_LEN; Lablindx++)
				{
					ETAL_DAB_Tnr_Ctrl_GetServListinfo_Reply->LabelString[Lablindx] = pServiceInfo.m_serviceLabel[Lablindx];
				}
			}
		}

		b_ret = TRUE;
	}

	return b_ret;
}

/*=========================================================================================================================================*/
/* Tbool DabTuner_MsgRcvScan2GetComponentListReply(Ts_DabTunerMsg_GetComponentList_Reply * DAB_Tuner_Ctrl_GetComponent_Reply)*/

/*==========================================================================================================================================*/

Tbool DabTuner_MsgRcvScan2GetComponentListReply(Ts_DabTunerMsg_GetComponentList_Reply * DAB_Tuner_Ctrl_GetComponent_Reply)
{
	Tbool b_ret = FALSE;
	Tu8 Servcompindex = 0;
	Tu8 Lablindx = 0;
	if (DAB_Tuner_Ctrl_GetComponent_Reply != NULL)
	{
		DAB_Tuner_Ctrl_GetComponent_Reply->NoOfComponents = pSCInfo.m_scCount;// No of available servcomponents
		if (DAB_Tuner_Ctrl_GetComponent_Reply->NoOfComponents > MAX_COMPONENTS_PER_SERVICE)
		{
			DAB_Tuner_Ctrl_GetComponent_Reply->NoOfComponents = MAX_COMPONENTS_PER_SERVICE;
		}

		for (Servcompindex = 0; Servcompindex < DAB_Tuner_Ctrl_GetComponent_Reply->NoOfComponents; Servcompindex++)
		{
			DAB_Tuner_Ctrl_GetComponent_Reply->Component[Servcompindex].CharSet = pSCInfo.m_scInfo[Servcompindex].m_scLabelCharset; //SERVICE COMPONENT CHARSET
			DAB_Tuner_Ctrl_GetComponent_Reply->Component[Servcompindex].ComponentType = pSCInfo.m_scInfo[Servcompindex].m_scType; // SERVICE COMPONENT TYPE
			DAB_Tuner_Ctrl_GetComponent_Reply->Component[Servcompindex].InternalCompId = pSCInfo.m_scInfo[Servcompindex].m_scIndex; //SERVICE COMPONENT ID

			for (Lablindx = 0; Lablindx < ETAL_DEF_MAX_LABEL_LEN; Lablindx++)
			{
				DAB_Tuner_Ctrl_GetComponent_Reply->Component[Servcompindex].LabelString[Lablindx] = pSCInfo.m_scInfo[Servcompindex].m_scLabel[Lablindx]; //SERVICE COMPONENT CHARLABEL
			}
			DAB_Tuner_Ctrl_GetComponent_Reply->Component[Servcompindex].ShortLabelFlags = pSCInfo.m_scInfo[Servcompindex].m_scLabelCharflag; //SERVICE COMPONENT CHARFLAG
		}
		b_ret = TRUE;
	}

	return b_ret;
}

/*================================================================================================*/
/* void DabTuner_MsgSndDeSelectService_Cmd(Tu32 u32_Sid)										  */
/*================================================================================================*/
void DabTuner_MsgSndDeSelectService_Cmd(Tu32 u32_Sid)
{
	UNUSED(u32_Sid);
   //DABTuner_MsgSndDeSelectService_Cmd(u32_Sid);
}
/*================================================================================================*/
/* void DabTuner_MsgSndSelectService_Cmd(Tu32 u32_Sid)*/
/*================================================================================================*/
ETAL_STATUS DabTuner_MsgSndSelectService_Cmd(Tu32 u32_Sid)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	/* ETAL_SERVSEL_MODE_SERVICE is used to select the service and first servcomponent in the service */
	ETAL_Ret = etal_service_select_audio(dabFG_recvhdl, ETAL_SERVSEL_MODE_SERVICE, Dabtuner_EtalEnsmblinfo.pUEId, u32_Sid, ETAL_INVALID, ETAL_INVALID);

	return ETAL_Ret;
}

/*================================================================================================*/
/* Tbool DabTuner_MsgRcvSelectServiceReply(Ts_DabTunerMsg_SelectServiceReply *
selectServiceReply,char *msgdata)         */
/*================================================================================================*/
Tbool DabTuner_MsgRcvSelectServiceReply(Ts_DabTunerMsg_SelectServiceReply * selectServiceReply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(selectServiceReply);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSelectServiceReply(selectServiceReply,msgdata);
	return b_ret;

}

/*================================================================================================*/
/* ETAL_STATUS DabTuner_MsgCmd_SelectComponent( Tu16 u16InternalCompId, Tu32 u32Sid )                    */
/*================================================================================================*/
ETAL_STATUS DabTuner_MsgCmd_SelectComponent(Tu16 u16InternalCompId, Tu32 u32Sid)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	Etal_servselMode = ETAL_SERVSEL_MODE_DAB_SC; /* mode for selecting a particular Service component in a service */
	ETAL_Ret = etal_service_select_audio(dabFG_recvhdl, Etal_servselMode, Dabtuner_EtalEnsmblinfo.pUEId, u32Sid, u16InternalCompId, ETAL_INVALID);

	return ETAL_Ret;
}
/*================================================================================================*/
/*  void DabTuner_MsgReply_SelectComponent(Ts_Select_ComponentReply * msgdata,char *msg )         */
/*================================================================================================*/
void DabTuner_MsgReply_SelectComponent(Ts_Select_ComponentReply * msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_MsgReply_SelectComponent(msgdata,msg );                                                                             

}
/*================================================================================================*/
/* void DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd(void)          */
/*================================================================================================*/
ETAL_STATUS DabTuner_MsgSndGetCurrEnsembleProgrammeServiceList_Cmd(void)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;

	Tbool bGetAudioServices = TRUE;  //for Audio services
	Tbool bGetDataServices = FALSE;	// for Data services

	ETAL_Ret = etal_get_service_list(dabFG_recvhdl, Dabtuner_EtalEnsmblinfo.pUEId, bGetAudioServices, bGetDataServices, &Dabtuner_Etalservinfo);

	return ETAL_Ret;
}
/*===============================================================================================================================================================*/
/* void DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(Ts_DabTunerMsg_GetCurrEnsembleProgListReply* st_GetCurrEnsembleProgListReply)          */
/*===============================================================================================================================================================*/
void DabTuner_MsgRcvGetCurrEnsembleProgrammeServiceListReply(Ts_DabTunerMsg_GetCurrEnsembleProgListReply* Etal_GetCurrEnsembleServListReply)
{
	Tu8 Servindex = 0;
	if (Etal_GetCurrEnsembleServListReply != NULL)
	{
		/*getting the no of services in curr ensemble */
		Etal_GetCurrEnsembleServListReply->u8_NumOfServices = Dabtuner_Etalservinfo.m_serviceCount;

		for (Servindex = 0; Servindex < Etal_GetCurrEnsembleServListReply->u8_NumOfServices; Servindex++)
		{
			/* storing the serive id of the current ensemble*/
			Etal_GetCurrEnsembleServListReply->st_serviceinfo[Servindex].ProgServiceId = Dabtuner_Etalservinfo.m_service[Servindex];
		}
	}
}
/*================================================================================================*/
/*   void DabTuner_MsgSndSearchNext_cmd(Te_RADIO_DirectionType e_direction, Tu32 startfrequency) */
/*================================================================================================*/
void DabTuner_MsgSndSearchNext_cmd(Te_RADIO_DirectionType e_direction, Tu32 startfrequency)
{
	UNUSED(e_direction);
	UNUSED(startfrequency);
	//DABTuner_MsgSndSearchNext_cmd(e_direction,startfrequency);
}
/*================================================================================================*/
/*Tbool DabTuner_MsgReply_SearchNext(Ts_SearchNext_Reply * msgdata,char *msg )    */
/*================================================================================================*/
Tbool DabTuner_MsgReply_SearchNext(Ts_SearchNext_Reply * msgdata, char *msg)
{
	Tbool b_ret = FALSE;
	UNUSED(msgdata);
	UNUSED(msg);
	//b_ret = DABTuner_MsgReply_SearchNext(msgdata,msg );
	return b_ret;
}
/*================================================================================================*/
/*void DabTuner_MsgSndGetEnsembleProperties(void)*/
/*================================================================================================*/
ETAL_STATUS DabTuner_MsgSndGetEnsembleProperties(void)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;


	ETAL_Ret = etal_get_ensemble_data(Dabtuner_EtalEnsmblinfo.pUEId, &Dabtuner_EtalEnsmblinfo.pCharset,
									Dabtuner_EtalEnsmblinfo.pLabel, &Dabtuner_EtalEnsmblinfo.pCharflag);

	return ETAL_Ret;
}
/*================================================================================================*/
/*  void DabTuner_MsgCmdDestroyReciver( void )                                                    */
/*================================================================================================*/
void DabTuner_MsgReplyGetEnsembleProperties(Ts_DabTunerGetEnsembleProperties_reply *Etal_msgdata)
{
	Tu8 Lablindx;
	if (Etal_msgdata != NULL)
	{
		Etal_msgdata->EnsembleIdentifier = (Tu16)(LIB_AND(Dabtuner_EtalEnsmblinfo.pUEId, 0x0000FFFF));
		Etal_msgdata->ECC = (Tu8)(LIB_MASK_AND_SHIFT(Dabtuner_EtalEnsmblinfo.pUEId, 0x00FF0000, 16));
		Etal_msgdata->CharSet = Dabtuner_EtalEnsmblinfo.pCharset;
		Etal_msgdata->ShortLabelFlags = Dabtuner_EtalEnsmblinfo.pCharflag;
		for (Lablindx = 0; Lablindx < ETAL_DEF_MAX_LABEL_LEN; Lablindx++)
		{
			Etal_msgdata->LabelString[Lablindx] = Dabtuner_EtalEnsmblinfo.pLabel[Lablindx];
		}
	}
}
/*================================================================================================*/
/*  void DabTuner_MsgCmdDestroyReciver( void )                                                    */
/*================================================================================================*/
void DabTuner_MsgCmdDestroyReciver(void)
{
	//DABTuner_MsgCmdDestroyReciver( );
}
/*=================================================================================================*/
/*  void DabTuner_MsgReplyDestroyReceiver(Ts_DabTunerMsg_R_DestroyReceiver_Cmd *msgdata, char *msg)*/
/*=================================================================================================*/
void DabTuner_MsgReplyDestroyReceiver(Ts_DabTunerMsg_R_DestroyReceiver_Cmd *msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_MsgReplyDestroyReceiver(msgdata,msg);
}

/*=================================================================================================*/
/*  void DabTuner_MsgSndSetAudioStatusNotifier_cmd ()*/
/*=================================================================================================*/
void DabTuner_MsgSndSetAudioStatusNotifier_cmd(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetAudioStatusNotifier_cmd (Trigger);
}

/*=================================================================================================*/
/* Tbool DabTuner_MsgRcvSetAudioStatusNotifier_reply(Ts_DabTunerMsg_R_SetAudioStatusReply * SetAudioStatus,
                                                char *msgdata)*/
/*=====================================================================================================================*/
Tbool DabTuner_MsgRcvSetAudioStatusNotifier_reply(Ts_DabTunerMsg_R_SetAudioStatusReply * SetAudioStatus, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(SetAudioStatus);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSetAudioStatusNotifier_reply(SetAudioStatus,msgdata);
	return b_ret;
}
/*=======================================================================================================================*/
/* Tbool DabTuner_MsgRcvSetAudioStatusNotifier(Ts_DabTunerMsg_R_AudioStatus_notify * AudioStatus_notify,
char *msgdata)*/
/*=================================================================================================*/

Tbool DabTuner_MsgRcvSetAudioStatusNotifier(Ts_DabTunerMsg_R_AudioStatus_notify * AudioStatus_notify, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(AudioStatus_notify);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSetAudioStatusNotifier( AudioStatus_notify,msgdata);
	return b_ret;
}

/*=================================================================================================*/
/*  void DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd()*/
/*=================================================================================================*/


void DabTuner_MsgSndSetPeriodicalBERQualityNotifier(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetPeriodicalBERQualityNotifier(Trigger);
}
/*=================================================================================================*/
/*  void DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd()*/
/*=================================================================================================*/
void DabTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetPeriodicalBERQualityNotifier_cmd(Trigger);
}

/*=================================================================================================================================*/
/*  Tbool DabTuner_MsgRcvSetPeriodicalBERQualityNotifier_reply(Ts_DabTunerMsg_R_PeriodicalBERQual_reply *PeriodicalBERQual_reply,
                                                                        char *msgdata)*/
/*=================================================================================================================================*/
Tbool DabTuner_MsgRcvSetPeriodicalBERQualityNotifier_reply(Ts_DabTunerMsg_R_PeriodicalBERQual_reply *PeriodicalBERQual_reply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(PeriodicalBERQual_reply);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSetPeriodicalBERQualityNotifier_reply(PeriodicalBERQual_reply,msgdata);
	return b_ret;
}

/*=================================================================================================================*/
/*  Tbool DabTuner_MsgRcvSetPeriodicalBERQualityNotifier(Ts_DabTunerMsg_R_PeriodicalBERQual_Notify *BERQual_Notify,
                                                                  char *msgdata)*/
/*=================================================================================================================*/
Tbool DabTuner_MsgRcvSetPeriodicalBERQualityNotifier(Ts_DabTunerMsg_R_PeriodicalBERQual_Notify *BERQual_Notify, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(BERQual_Notify);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSetPeriodicalBERQualityNotifier(BERQual_Notify,msgdata);
	return b_ret;
}
/*=================================================================================================*/
/*  void DabTuner_MsgSndSetRSSINotifier_cmd														   */
/*=================================================================================================*/

void DabTuner_MsgSndSetRSSINotifier_cmd(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetRSSINotifier_cmd(Trigger);
}
/*=================================================================================================*/
/*  void DabTuner_MsgSndSetRSSINotifier(Tu8 Trigger)											   */
/*=================================================================================================*/

void DabTuner_MsgSndSetRSSINotifier(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetRSSINotifier(Trigger);
}
/*=================================================================================================*/
/*  void DabTuner_MsgSndSetRSSINotifierSettings_cmd()*/
/*=================================================================================================*/
void DabTuner_MsgSndSetRSSINotifierSettings_cmd(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetRSSINotifierSettings_cmd(Trigger);
}

/*=================================================================================================*/
/*  Tbool DabTuner_MsgRcvSetRSSINotifierSettings_reply(Ts_DabTunerMsg_R_RSSINotifierSettings_reply *RSSINotifier,
char *msgdata)*/
/*=================================================================================================*/

Tbool DabTuner_MsgRcvSetRSSINotifierSettings_reply(Ts_DabTunerMsg_R_RSSINotifierSettings_reply *RSSINotifier, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(RSSINotifier);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSetRSSINotifierSettings_reply(RSSINotifier,msgdata);
	return b_ret;

}

/*=================================================================================================*/
/*  Tbool DabTuner_MsgRcvRSSI_notify(Ts_DabTunerMsg_R_RSSI_notify *RSSINotifier,
                                                                        char *msgdata)*/
/*=================================================================================================*/
Tbool DabTuner_MsgRcvRSSI_notify(Ts_DabTunerMsg_R_RSSI_notify *RSSINotifier, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(RSSINotifier);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvRSSI_notify(RSSINotifier, msgdata); 
	return b_ret;
}

/*=================================================================================================*/
/*  void DabTuner_MsgSndSetSNRNotifierSettings_cmd()*/
/*=================================================================================================*/
void DabTuner_MsgSndSetSNRNotifierSettings_cmd(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_MsgSndSetSNRNotifierSettings_cmd(Trigger);
}


/*=================================================================================================================*/
/*  Tbool DabTuner_MsgRcvSetSNRNotifierSettings_reply(Ts_DabTunerMsg_R_SetSNRNotifier_reply *SNRNotifier_reply,
                                                                        char *msgdata)*/
/*=================================================================================================================*/
Tbool DabTuner_MsgRcvSetSNRNotifierSettings_reply(Ts_DabTunerMsg_R_SetSNRNotifier_reply *SNRNotifier_reply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(SNRNotifier_reply);
	UNUSED(msgdata);
	//b_ret = //DABTuner_MsgRcvSetSNRNotifierSettings_reply(SNRNotifier_reply,msgdata); 
	return b_ret;
}

/*=================================================================================================*/
/*  Tbool DabTuner_MsgRcvSNRNotifier(Ts_DabTunerMsg_R_SNRNotifier *SNRNotifier,
                                                                        char *msgdata)*/
/*=================================================================================================*/
Tbool DabTuner_MsgRcvSNRNotifier(Ts_DabTunerMsg_R_SNRNotifier *SNRNotifier, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(SNRNotifier);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvSNRNotifier(SNRNotifier,msgdata); 
	return b_ret;
}
/*=================================================================================================*/
/* void DabTuner_SetSynchronisationNotifier_cmd(Tu8 Trigger) */
/*=================================================================================================*/
void DabTuner_SetSynchronisationNotifier_cmd(Tu8 Trigger)
{
	UNUSED(Trigger);
	//DABTuner_SetSynchronisationNotifier_cmd(Trigger);
}
/*=================================================================================================*/
/* void DabTuner_SetSynchronisationNotifier_reply(Ts_DabTunerMsg_R_SynchReply *SynchReply, char *msgdata) */
/*=================================================================================================*/
void DabTuner_SetSynchronisationNotifier_reply(Ts_DabTunerMsg_R_SynchReply *SynchReply, char *msgdata)
{
	UNUSED(SynchReply);
	UNUSED(msgdata);
	//DABTuner_SetSynchronisationNotifier_reply(SynchReply,msgdata);
}
/*=================================================================================================*/
/* void DabTuner_MsgRcvSetSynchronisationNotification(Ts_DabTunerMsg_R_SynchNotification *SynchNotify, char *msgdata)*/
/*=================================================================================================*/
void DabTuner_MsgRcvSetSynchronisationNotification(Ts_DabTunerMsg_R_SynchNotification *SynchNotify, char *msgdata)
{
	UNUSED(SynchNotify);
	UNUSED(msgdata);
	//DABTuner_MsgRcvSetSynchronisationNotification(SynchNotify,msgdata);
}

/*=================================================================================================*/
/*  void DabTuner_set_forward_FIB()*/
/*=================================================================================================*/

void DabTuner_set_forward_FIB(void)
{
	//DABTuner_set_forward_FIB();
}
/*=================================================================================================*/
/*  void DabTuner_SetFIG0_21_filter_command(Tu16 u16_extension, Tu8 muxid ,Tu8 filterid)*/
/*=================================================================================================*/
void DabTuner_SetFIG0_21_filter_command(Tu16 u16_extension, Tu8 muxid, Tu8 filterid)
{
	UNUSED(u16_extension);
	UNUSED(muxid);
	UNUSED(filterid);
	//DABTuner_SetFIG0_21_filter_command(u16_extension, muxid ,filterid);

}
/*=================================================================================================*/
/*  void DabTuner_SetFIG_filter_command(Tu16 u16_extension, Tu8 muxid ,Tu8 filterid)*/
/*=================================================================================================*/
void DabTuner_SetFIG_filter_command(Tu16 u16_extension, Tu8 muxid, Tu8 filterid)
{
	UNUSED(u16_extension);
	UNUSED(muxid);
	UNUSED(filterid);
	//DABTuner_SetFIG_filter_command(u16_extension, muxid ,filterid);	
}
/*=================================================================================================*/
/* void DabTuner_Get_Audio_Status_cmd(void) */
/*=================================================================================================*/
void DabTuner_Get_Audio_Status_cmd(void)
{
	//DABTuner_Get_Audio_Status_cmd();
}
/*=================================================================================================*/
/* Tbool DabTuner_Get_Audio_Status_Notification(Ts_AudioStatus2_not *pst_AudioStatus2_not, Ts8 *pu8_Msg) */
/*=================================================================================================*/
Tbool DabTuner_Get_Audio_Status_Notification(Ts_AudioStatus2_not *pst_AudioStatus2_not, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_AudioStatus2_not);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_Get_Audio_Status_Notification(pst_AudioStatus2_not,pu8_Msg);
	return b_ret;
}
/*=================================================================================================*/
/* void DabTuner_Set_Audio_Status_Notifier_Cmd(void) */
/*=================================================================================================*/
void DabTuner_Set_Audio_Status_Notifier_Cmd(void)
{
	//DABTuner_Set_Audio_Status_Notifier_Cmd();
}
/*=================================================================================================*/
/*  void DabTuner_Get_Audio_Status2_Req(void)*/
/*=================================================================================================*/
void DabTuner_Get_Audio_Status2_Req(void)
{
	//DABTuner_Get_Audio_Status2_Req();
}
/*=================================================================================================*/
/* void DabTuner_Get_Audio_Status2_Res(Ts_DabTunerMsg_GetAudioStatus2_reply *pst_GetAudioStatus2_reply, char* msgdata)*/
/*=================================================================================================*/
void DabTuner_Get_Audio_Status2_Res(Ts_DabTunerMsg_GetAudioStatus2_reply *pst_GetAudioStatus2_reply, char* msgdata)
{
	UNUSED(pst_GetAudioStatus2_reply);
	UNUSED(msgdata);
	//DABTuner_Get_Audio_Status2_Res(pst_GetAudioStatus2_reply,msgdata);
}
/*=================================================================================================*/
/* void DabTuner_Set_Audio_Status_Notifier_Cmd(void) */
/*=================================================================================================*/
void DabTuner_Set_Audio_Status_Notifier_Reply(Ts_Set_Audio_Status_Notifier_Reply *st_Set_Audio_Status_reply, char*  msg)
{
	UNUSED(st_Set_Audio_Status_reply);
	UNUSED(msg);
	//DABTuner_Set_Audio_Status_Notifier_Reply(st_Set_Audio_Status_reply, msg);
}
/*=================================================================================================*/
/* void DabTuner_ResetBlending_cmd(void) */
/*=================================================================================================*/
void DabTuner_ResetBlending_cmd(void)
{
	//DABTuner_ResetBlending_cmd( );

}
/*=================================================================================================*/
/*  void  DabTuner_ResetBlending_cmd(Ts_ResetBlending_Reply *msgdata, char *msg)				*/
/*=================================================================================================*/
void DabTuner_ResetBlending_Reply(Ts_ResetBlending_Reply *msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_ResetBlending_Reply(msgdata,msg);
}
/*=================================================================================================*/
/*  void DabTuner_PrepareForBlending_cmd()*/
/*=================================================================================================*/
void DabTuner_PrepareForBlending_cmd(void)
{
	//DABTuner_PrepareForBlending_cmd();
}
/*=================================================================================================*/
/*  void DabTuner_PrepareForBlending_Reply(Ts_PrepareForBlending_Reply *msgdata, char *msg)*/
/*=================================================================================================*/
void DabTuner_PrepareForBlending_Reply(Ts_PrepareForBlending_Reply *msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_PrepareForBlending_Reply(msgdata,msg);
}

/*==========================================================================================================================*/
/*  Tbool DabTuner_PrepareForBlending_Notify(Ts_PrepareForBlending_Notify *pst_PrepareForBlending_not, Ts8 *pu8_Msg)	*/
/*==========================================================================================================================*/
Tbool DabTuner_PrepareForBlending_Notify(Ts_PrepareForBlending_Notify *pst_PrepareForBlending_not, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_PrepareForBlending_not);
	UNUSED(pu8_Msg);
	//b_ret =  DABTuner_PrepareForBlending_Notify(pst_PrepareForBlending_not,pu8_Msg);
	return b_ret;
}

/*=================================================================================================*/
/*  void DabTuner_StartTimeAlignmentForBlending_cmd()*/
/*=================================================================================================*/
void DabTuner_StartTimeAlignmentForBlending_cmd(Ts32 Delay)
{
	UNUSED(Delay);
	//DABTuner_StartTimeAlignmentForBlending_cmd(Delay);
}
/*=================================================================================================*/
/*  Tbool DabTuner_StartTimeAlignmentForBlending_Reply(Ts_StartTimeAlignmentForBlending_Reply *msgdata, char *msg)*/
/*=================================================================================================*/
void DabTuner_StartTimeAlignmentForBlending_Reply(Ts_StartTimeAlignmentForBlending_repl *msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_StartTimeAlignmentForBlending_Reply(msgdata,msg);
}
/*==========================================================================================================================================*/
/*  Tbool DabTuner_TimeAlignmentForBlending_Notify(Ts_TimeAlignmentForBlending_Notify *pst_TimeAlignmentForBlending_not, Ts8 *pu8_Msg)	*/
/*==========================================================================================================================================*/
Tbool DabTuner_TimeAlignmentForBlending_Notify(Ts_TimeAlignmentForBlending_Notify *pst_TimeAlignmentForBlending_not, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_TimeAlignmentForBlending_not);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_TimeAlignmentForBlending_Notify(pst_TimeAlignmentForBlending_not,pu8_Msg);
	return b_ret;
}
/*=================================================================================================*/
/*  void DabTuner_Set_Drifttracking_Cmd()								*/
/*=================================================================================================*/
void DabTuner_Set_Drifttracking_Cmd()
{
	//DABTuner_Set_Drifttracking_Cmd();

}
/*=================================================================================================*/
/*  void DabTuner_Set_Drifttracking_Cmd()								*/
/*=================================================================================================*/
void DabTuner_Set_Drift_Tracking_Reply(Ts_Set_DriftTracking_Reply *st_DriftTacking_Reply, char *msg)
{
	UNUSED(st_DriftTacking_Reply);
	UNUSED(msg);
	//DABTuner_Set_Drift_Tracking_Reply(st_DriftTacking_Reply ,msg);
}
/*=================================================================================================*/
/* void DabTuner_Drift_Tracking_Not(Ts_Drift_Tracking_Notification *st_Drift_Tracking_Notification, char *msg)*/
/*=================================================================================================*/
void DabTuner_Drift_Tracking_Not(Ts_Drift_Tracking_Notification *st_Drift_Tracking_Notification, char *msg)
{
	UNUSED(st_Drift_Tracking_Notification);
	UNUSED(msg);
	//DABTuner_Drift_Tracking_Not(st_Drift_Tracking_Notification,msg);
}
/*=================================================================================================*/
/*  void DabTuner_StartBlending_cmd(Tu8 u8_BlendTarget,Tu16	u16_LevelData)									*/
/*=================================================================================================*/
void DabTuner_StartBlending_cmd(Tu8 u8_BlendTarget,Tu16	u16_LevelData)
{
	UNUSED(u8_BlendTarget);
	UNUSED(u16_LevelData);
	//DABTuner_StartBlending_cmd(u8_BlendTarget,u16_LevelData);
}
/*=================================================================================================*/
/*  void DabTuner_StartBlending_Without_delay(Tu8 Blend_Target)									*/
/*=================================================================================================*/
void DabTuner_StartBlending_Without_delay(Tu8 Blend_Target)
{
	UNUSED(Blend_Target);
	//DABTuner_StartBlending_Without_delay(Blend_Target);
}
/*=================================================================================================*/
/*  void DabTuner_StartBlending_for_implicit(Tu8 u8_BlendTarget)*/
/*=================================================================================================*/
void DabTuner_StartBlending_for_implicit(Tu8 u8_BlendTarget)
{
	UNUSED(u8_BlendTarget);
	//DABTuner_StartBlending_for_implicit(u8_BlendTarget);
}
/*=================================================================================================*/
/*  void DabTuner_StartBlending_Reply(Ts_StartBlending_Reply *msgdata, char *msg)*/
/*=================================================================================================*/
void DabTuner_StartBlending_Reply(Ts_StartBlending_Reply *msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_StartBlending_Reply(msgdata,msg);
}
/*=================================================================================================*/
/*   DAB_Tuner_Ctrl_RegisterSinkToServComp_cmd(void)											   */
/*=================================================================================================*/
void DAB_Tuner_Ctrl_RegisterSinkToServComp_cmd(void)
{
	//DABTuner_RegisterSinkToServComp_cmd();
}
/*==================================================================================================================*/
/*  void DAB_Tuner_Ctrl_RegisterSinkToServComp_reply(Ts_DabTunerMsg_RegisterSinkToServComp_reply *msgdata,char *msg)*/
/*===================================================================================================================*/
void DAB_Tuner_Ctrl_RegisterSinkToServComp_reply(Ts_DabTunerMsg_RegisterSinkToServComp_reply *msgdata, char *msg)
{
	UNUSED(msgdata);
	UNUSED(msg);
	//DABTuner_RegisterSinkToServComp_reply(msgdata,msg);
}
/*****************************************************************************************/
/*		void DabTunerCtrl_SetServListChangedNotifier_cmd(void)							 */
/*****************************************************************************************/
void DabTunerCtrl_SetServListChangedNotifier_cmd(Tu8 u8_status)
{
	UNUSED(u8_status);
	//DABTuner_SetServListChangedNotifier_cmd(u8_status);
}
/**************************************************************************************************************************************************/
/*	 void DabTunerCtrl_SetServListChangedNotifier_repl(Ts_DabTunerMsg_SetServListChangedNotifier_repl *ServListChangedNotifierRply, Tu8 *msg )    */
/**************************************************************************************************************************************************/
void DabTunerCtrl_SetServListChangedNotifier_repl(Ts_DabTunerMsg_SetServListChangedNotifier_repl *ServListChangedNotifierRply, char *msg)
{
	UNUSED(ServListChangedNotifierRply);
	UNUSED(msg);
	//DABTuner_SetServListChangedNotifier_repl(ServListChangedNotifierRply, msg);
}
/**************************************************************************************************************************************************/
/*	    void DabTunerCtrl_ProgrammeServListChanged_not(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *ProgrammeServListChangedNot, Tu8 *msg)	      */
/**************************************************************************************************************************************************/
void DabTunerCtrl_ProgrammeServListChanged_not(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *ProgrammeServListChangedNot, Tu8 *msg)
{
	UNUSED(ProgrammeServListChangedNot);
	UNUSED(msg);
	//DABTuner_ProgrammeServListChanged_not(ProgrammeServListChangedNot,msg);
}

/*****************************************************************************************/
/*		void DabTunerCtrl_SetServPropsChangedNotifier_cmd(void)							 */
/*****************************************************************************************/
void DabTunerCtrl_SetServPropsChangedNotifier_cmd(void)
{
	//DABTuner_SetServPropsChangedNotifier_cmd();

}
/*================================================================================================*/
/*  void DabTuner_MsgSndAnnouncementSwitching_cmd()                                                       */
/*================================================================================================*/
void DabTuner_MsgSndAnnouncementSwitching_cmd(Tu8 subchannelid)
{
	UNUSED(subchannelid);
	//DABTuner_MsgSndAnnouncementSwitching_cmd(subchannelid);
}
/*================================================================================================*/
/*  void DabTuner_MsgSndAnnouncementStop_cmd(void)                                                       */
/*================================================================================================*/
void DabTuner_MsgSndAnnouncementStop_cmd(void)
{
	//DABTuner_MsgSndAnnouncementStop_cmd();
}
/*================================================================================================*/
/* Tbool DabTuner_MsgRcvAnnouncementSwitching(Ts_DabTunerMsg_R_AnnouncementSwitching * DAB_Tuner_Ctrl_AnnouncementSwitchingReply,char *msgdata) */
/*================================================================================================*/
Tbool DabTuner_MsgRcvAnnouncementSwitching(Ts_DabTunerMsg_R_AnnouncementSwitching *DAB_Tuner_Ctrl_AnnouncementSwitchingReply, char *msgdata)
{
	Tbool b_ret = FALSE;
	UNUSED(DAB_Tuner_Ctrl_AnnouncementSwitchingReply);
	UNUSED(msgdata);
	//b_ret = DABTuner_MsgRcvAnnouncementSwitching(DAB_Tuner_Ctrl_AnnouncementSwitchingReply,msgdata);
	return b_ret;
}
/**************************************************************************************************************************************************/
/*	 void DabTunerCtrl_SetServPropsChangedNotifier_repl(Ts_DabTunerMsg_SetServPropsChangedNotifier_repl *ServPropsChangedNotifier_repl, Tu8 *msg) */
/**************************************************************************************************************************************************/
void DabTunerCtrl_SetServPropsChangedNotifier_repl(Ts_DabTunerMsg_SetServPropsChangedNotifier_repl *ServPropsChangedNotifier_repl, char *msg)
{
	UNUSED(ServPropsChangedNotifier_repl);
	UNUSED(msg);
	//DABTuner_SetServPropsChangedNotifier_repl(ServPropsChangedNotifier_repl, msg);
}
/**************************************************************************************************************************************************/
/*     void DabTunerCtrl_ServPropsChanged_not(Ts_DabTunerMsg_ServPropsChanged_not *ServPropsChanged_not, Tu8 *msg)    */
/**************************************************************************************************************************************************/
void DabTunerCtrl_ServPropsChanged_not(Ts_DabTunerMsg_ServPropsChanged_not *ServPropsChanged_not, char *msg)
{
	UNUSED(ServPropsChanged_not);
	UNUSED(msg);
	//DABTuner_ServPropsChanged_not(ServPropsChanged_not,msg);
}
/**************************************************************************************************************************************************/
/* void DabTunerCtrl_GetAudioStatus_req(void)       */
/**************************************************************************************************************************************************/
ETAL_STATUS DabTunerCtrl_GetAudioStatus_req(void)
{
	ETAL_STATUS ETAL_Ret = ETAL_RET_ERROR;
	memset(&Etal_DabAudioQuality, 0, sizeof(EtalBcastQualityContainer));
	ETAL_Ret = etal_get_reception_quality(dabFG_recvhdl, &Etal_DabAudioQuality);

	return ETAL_Ret;
}
/**************************************************************************************************************************************************/
/*   void DabTunerCtrl_GetAudioProperties_req(void)    */
/**************************************************************************************************************************************************/
void DabTunerCtrl_GetAudioProperties_req(void)
{
	//DABTuner_GetAudioProperties_req();
}
/**************************************************************************************************************************************************/
/* void DabTunerCtrl_GetSynchronisationState_req(void)   */
/**************************************************************************************************************************************************/
void DabTunerCtrl_GetSynchronisationState_req(void)
{
	//DABTuner_GetSynchronisationState_req();
}
/**************************************************************************************************************************************************/
/*Tbool DabTunerCtrl_GetAudioStatus_repl(Ts_GetAudioStatus_repl *pst_GetAudioStatus_repl)   */
/**************************************************************************************************************************************************/
void DabTunerCtrl_GetAudioStatus_repl(Ts_GetAudioStatus_repl *pst_GetAudioStatus_repl)
{
	/*currently the Audio quality parameters are not copied in to reply status temporarily storing in qualityparams separate structure  */

	pst_GetAudioStatus_repl->RFFieldStrength = Etal_DabAudioQuality.EtalQualityEntries.dab.m_RFFieldStrength;
	pst_GetAudioStatus_repl->BBFieldStrength = Etal_DabAudioQuality.EtalQualityEntries.dab.m_BBFieldStrength;
	pst_GetAudioStatus_repl->FicBitErrorRatio = Etal_DabAudioQuality.EtalQualityEntries.dab.m_FicBitErrorRatio;
	pst_GetAudioStatus_repl->isValidMscBitErrorRatio = Etal_DabAudioQuality.EtalQualityEntries.dab.m_isValidMscBitErrorRatio;
	pst_GetAudioStatus_repl->MscBitErrorRatio = Etal_DabAudioQuality.EtalQualityEntries.dab.m_MscBitErrorRatio;

}
/**************************************************************************************************************************************************/
/*Tbool DabTunerCtrl_GetSynchronisationState_repl(Ts_GetSynchronisationState_repl *pst_GetSynchronisationState_repl, Ts8 *pu8_Msg)   */
/**************************************************************************************************************************************************/
Tbool DabTunerCtrl_GetSynchronisationState_repl(Ts_GetSynchronisationState_repl *pst_GetSynchronisationState_repl, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_GetSynchronisationState_repl);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_GetSynchronisationState_repl(pst_GetSynchronisationState_repl, pu8_Msg);
	return b_ret;
}
/**************************************************************************************************************************************************/
/*Tbool DabTunerCtrl_GetAudioProperties_repl(Ts_GetAudioProperties_repl *pst_GetAudioProperties_repl, Ts8 *pu8_Msg)*/
/**************************************************************************************************************************************************/
Tbool DabTunerCtrl_GetAudioProperties_repl(Ts_GetAudioProperties_repl *pst_GetAudioProperties_repl, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_GetAudioProperties_repl);
	UNUSED(pu8_Msg);
	//ret = DABTuner_GetAudioProperties_repl(pst_GetAudioProperties_repl,pu8_Msg);
	return b_ret;
}
/*==========================================================*/
/*  void DabTunerCtrl_SystemControl_cmd						*/
/*==========================================================*/
void DabTunerCtrl_SystemControl_cmd(Tu16 u16_ActionId, Tu16 u16_Parameter)
{
	UNUSED(u16_ActionId);
	UNUSED(u16_Parameter);
	//DABTuner_SystemControl_cmd(u16_ActionId,u16_Parameter);
}
/*==========================================================*/
/*  void DabTuner_MsgSndGetAudioErrorConcealment2Settings_Cmd						*/
/*==========================================================*/
void DabTuner_MsgSndGetAudioErrorConcealment2Settings_Cmd(void)
{
	//DABTuner_MsgSndGetAudioErrorConcealment2Settings_Cmd();
}
/*==========================================================*/
/*  Tbool DabTuner_MsgRcvGetAudioErrorConcealment2SettingsReply						*/
/*==========================================================*/
Tbool DabTuner_MsgRcvGetAudioErrorConcealment2SettingsReply(Ts_DabTunerMsg_GetAudioErrorConcealment2SettingsReply* st_GetAudioErrorConcealment2SettingsReply,char *msgdata)
{
	Tbool b_ReturnStatus = FALSE;
	
	UNUSED(st_GetAudioErrorConcealment2SettingsReply);
	UNUSED(msgdata);

	//b_ReturnStatus = DABTuner_MsgRcvGetAudioErrorConcealment2SettingsReply(st_GetAudioErrorConcealment2SettingsReply, msgdata);
		
	return b_ReturnStatus;
}

/*================================================================================================*/
/*  void DabTuner_SetAudioErrorConcealment2_cmd(void)                           */
/*================================================================================================*/
void DabTuner_SetAudioErrorConcealment2_cmd(void)
{
	//DABTuner_SetAudioErrorConcealment2_cmd();
	
}
/*=================================================================================================*/
/*  void DabTuner_MsgReplySetAudioErrorConcealment2_repl(Ts_DabTunerMsg_R_AudioErrorConcealment2_repl *msgdata, char *msg)*/
/*=================================================================================================*/
Tbool DabTuner_MsgReplySetAudioErrorConcealment2_repl(Ts_DabTunerMsg_R_AudioErrorConcealment2_repl *msgdata, char *msg)
{
	Tbool b_ReturnStatus = FALSE;
	
	UNUSED(msg);
	UNUSED(msgdata);

	//b_ReturnStatus = DABTuner_MsgReplySetAudioErrorConcealment2_repl(msgdata, msg);
	
	return b_ReturnStatus;
}
/*==========================================================*/
/*  Tbool DabTunerCtrl_SystemControl_repl		*/
/*==========================================================*/
Tbool DabTunerCtrl_SystemControl_repl(Ts_SystemControl_repl *pst_SystemControl_repl, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_SystemControl_repl);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_SystemControl_repl(pst_SystemControl_repl,pu8_Msg);
	return 	b_ret;
}

/*==========================================================*/
/*  Tbool DabTunerCtrl_SystemMonitoring_not	*/
/*==========================================================*/
Tbool DabTunerCtrl_SystemMonitoring_not(Ts_SystemMonitoring_not *pst_SystemMonitoring_not, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_SystemMonitoring_not);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_SystemMonitoring_not(pst_SystemMonitoring_not,pu8_Msg);
	return 	b_ret;
}

/*=================================================================================================*/
/*  void DABTuner_CheckSum(void) */
/*=================================================================================================*/
void DAB_CheckSum(void)
{
	//DABTuner_CheckSum();
}

/*================================================================================================*/
/*  Tbool DAB_RxCmdAnalysis(Tu8 *buffer)                                             */
/*================================================================================================*/
Tbool DAB_Tuner_Ctrl_RxCmdAnalysis(Tu8 *buffer)
{
	Tbool b_ret = FALSE;
	UNUSED(buffer);
	//b_ret = DABTuner_RxCmdAnalysis(buffer);
	return 	b_ret;
}

/*================================================================================================*/
/* void DabTunerMsg_XPAD_DLS_Data(Ts_dab_tuner_ctrl_DLS_data *msgdata, char *msg)                 */
/*================================================================================================*/
void DabTunerMsg_XPAD_DLS_Data(Ts_dab_tuner_ctrl_DLS_data *msgdata, char *msg)
{
	Tu8 u8_MsgIndex = 0;
	Tu8 u8_DLS_LabelIndex = 0;
	Tu8 u8_Msglength = 0;

	u8_Msglength = msg[u8_MsgIndex++];
	msgdata->Runningstatus = (Tu8)((msg[u8_MsgIndex] & 0x80) >> 7);
	msgdata->Contenttype   = (Tu8)((msg[u8_MsgIndex++] & 0x7F));
	msgdata->CharacterSet  = msg[u8_MsgIndex++];
	msgdata->LabelLength   = u8_Msglength - 3;
	memset(msgdata->DLS_LabelByte, 0, 128);

	for (u8_DLS_LabelIndex = 0; u8_DLS_LabelIndex<(msgdata->LabelLength); u8_DLS_LabelIndex++)
	{
		msgdata->DLS_LabelByte[u8_DLS_LabelIndex] = msg[u8_MsgIndex++];

	}
}
/*===============================================================================================================================*/
/*void DABDataService_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)              */
/*===============================================================================================================================*/
void DABDataService_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	Ts_Sys_Msg  DAB_data_readmsg;
	Ts_Sys_Msg* p_msg = &DAB_data_readmsg;
	EtalGenericDataServiceRaw *GenericDataServiceRaw = (EtalGenericDataServiceRaw*)pBuffer;	
	Tu32 u32_ImageBufferSize = 0;

	UNUSED(status);
	UNUSED(pvContext);

	switch (GenericDataServiceRaw->m_dataType)
	{
		case ETAL_DATASERV_TYPE_DLPLUS:
		{
			memset(p_msg, 0, sizeof(Ts_Sys_Msg));
			UpdateParameterIntoMessage(p_msg->data, &GenericDataServiceRaw->m_data, (Tu8)dwActualBufferSize, &(p_msg->msg_length));
			p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
			p_msg->msg_id = DLS_DATA_NOTIFICATION;
			SYS_SEND_MSG(p_msg);
		}
		break;

		case ETAL_DATASERV_TYPE_SLS_XPAD:
		{
			memset(p_msg, 0, sizeof(Ts_Sys_Msg));
			memset(&st_SLS_Data, 0, sizeof(Ts_DAB_DataServiceRaw));
			p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
			p_msg->msg_id = SLS_DATA_NOTIFICATION;
			u32_ImageBufferSize = dwActualBufferSize - DATA_SERVICE_TYPE_SIZE;

			if (u32_ImageBufferSize <= MAX_DATA_SERVICE_PAYLOAD_SIZE)
			{
				SYS_RADIO_MEMCPY(&st_SLS_Data, GenericDataServiceRaw, dwActualBufferSize);
				st_SLS_Data.u32_PayloadSize = u32_ImageBufferSize;
				SYS_SEND_MSG(p_msg);
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_HAL] SLS Data exceeds 50 KB size limit! Not sending to upper layers!");
			}
		}
		break;

		default:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_HAL] INVALID DATATYPE RECEIVED FROM CALLBACK FUNCTION");
		}
		break;
	}

}

/*================================================================================================*/
/* void DABFICData_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize,								  */
/*								  EtalDataBlockStatusTy *status, void* pvContext)                 */
/*================================================================================================*/
void DABFICData_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	Ts_Sys_Msg  FIC_Data_readmsg;
	Ts_Sys_Msg* p_msg = &FIC_Data_readmsg;
	UNUSED(status);
	UNUSED(pvContext);
	memset(p_msg, 0, sizeof(Ts_Sys_Msg));
	UpdateParameterIntoMessage(p_msg->data, pBuffer, dwActualBufferSize, &(p_msg->msg_length));
	p_msg->dest_id = RADIO_DAB_TUNER_CTRL;
	p_msg->msg_id = FIG_DATA_NOTIFICATION;

	SYS_SEND_MSG(p_msg);
}

/*=============================================================================*/
/* ETAL_STATUS DabTunerCtrl_Config_Receiver_cmd(Te_Frontend_type e_Frontend_type)  */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Config_Receiver_cmd(Te_Frontend_type e_Frontend_type)
{
	ETAL_HANDLE hFrontend = ETAL_INVALID_HANDLE;
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	ETAL_HANDLE *Receiver_Handle = NULL;
	EtalReceiverAttr st_Receiver_Config;

	memset(&st_Receiver_Config, 0, sizeof(EtalReceiverAttr));
	
	st_Receiver_Config.m_Standard = ETAL_BCAST_STD_DAB;
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiver_Handle = &dabFG_recvhdl;
		hFrontend = ETAL_MAKE_FRONTEND_HANDLE(TUNER_1, ETAL_FE_FOREGROUND);
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Receiver_Handle = &dabBG_recvhdl;
		hFrontend = ETAL_MAKE_FRONTEND_HANDLE(TUNER_1, ETAL_FE_BACKGROUND);
	}
	else
	{
		/*MISRA C*/
	}
	st_Receiver_Config.m_FrontEndsSize = 1;
	st_Receiver_Config.m_FrontEnds[0] = hFrontend;
	st_Receiver_Config.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_UNSPECIFIED;
	e_RetStatus = etal_config_receiver(Receiver_Handle, &st_Receiver_Config);
	return e_RetStatus;
}

/*=============================================================================*/
/* ETAL_STATUS DabTunerCtrl_Config_Datapath_cmd(Te_Frontend_type e_Frontend_type) */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Config_Datapath_cmd(Te_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	EtalDataPathAttr st_DatapathAttr;
	ETAL_HANDLE *Datapath_Handle = NULL;
	Ts_DabData st_DabData;

	memset(&st_DatapathAttr, 0, sizeof(EtalDataPathAttr));

	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Datapath_Handle = &dabFG_datapath;
		st_DatapathAttr.m_receiverHandle = dabFG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Datapath_Handle = &dabBG_datapath;
		st_DatapathAttr.m_receiverHandle = dabBG_recvhdl;
	}
	else
	{
		/*MISRA C*/
	}
	st_DatapathAttr.m_dataType = ETAL_DATA_TYPE_DATA_SERVICE;
	st_DatapathAttr.m_sink.m_context = &st_DabData;
	st_DatapathAttr.m_sink.m_CbProcessBlock = DABDataService_cbFunc;
	st_DatapathAttr.m_sink.m_BufferSize = sizeof(EtalGenericDataServiceRaw);
	e_RetStatus = etal_config_datapath(Datapath_Handle, &st_DatapathAttr);
	return  e_RetStatus;
}

/*================================================================================================*/
/* ETAL_STATUS DabTunerCtrl_Config_FICData_cmd(Te_Frontend_type e_Frontend_type)				  */
/*================================================================================================*/
ETAL_STATUS DabTunerCtrl_Config_FICData_cmd(Te_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	EtalDataPathAttr st_DatapathAttr;
	ETAL_HANDLE *Datapath_Handle = NULL;
	Ts_DabData st_DabData;

	memset(&st_DatapathAttr, 0, sizeof(EtalDataPathAttr));

	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Datapath_Handle = &dabFG_FICData;
		st_DatapathAttr.m_receiverHandle = dabFG_recvhdl;
	}
	else
	{
		/*MISRA C*/
	}
	st_DatapathAttr.m_dataType = ETAL_DATA_TYPE_DAB_FIC;
	st_DatapathAttr.m_sink.m_context = &st_DabData;
	st_DatapathAttr.m_sink.m_CbProcessBlock = DABFICData_cbFunc;
	st_DatapathAttr.m_sink.m_BufferSize = FIC_DATA_BUFFER_MAX_SIZE;
	e_RetStatus = etal_config_datapath(Datapath_Handle, &st_DatapathAttr);
	return  e_RetStatus;
}
 /*=============================================================================*/
 /* ETAL_STATUS DabTunerCtrl_Audio_Source_Select_cmd(void)                                    */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Audio_Source_Select_cmd(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	EtalAudioSourceTy e_EtalAudioSourceType;
	e_EtalAudioSourceType = ETAL_AUDIO_SOURCE_DCOP_STA660;
	e_RetStatus = etal_audio_select(dabFG_recvhdl, e_EtalAudioSourceType);
	return e_RetStatus;
}
/*=============================================================================*/
/* DabTunerCtrl_destroy_datapath(Te_Frontend_type e_Frontend_type)            */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Destroy_Datapath_cmd(Te_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	ETAL_HANDLE *Datapath_Handle = NULL;
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Datapath_Handle = &dabFG_datapath;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Datapath_Handle = &dabBG_datapath;
	}
	else
	{
		/*MISRA C*/
	}
	e_RetStatus = etal_destroy_datapath(Datapath_Handle);
	return  e_RetStatus;
}

/*=============================================================================*/
/* DabTunerCtrl_destroy_datapath(Te_Frontend_type e_Frontend_type)            */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Destroy_FICData_cmd(Te_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	ETAL_HANDLE *Datapath_Handle = NULL;
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Datapath_Handle = &dabFG_FICData;
	}
	else
	{
		/*MISRA C*/
	}
	e_RetStatus = etal_destroy_datapath(Datapath_Handle);
	return  e_RetStatus;
}

/*=============================================================================*/
/* DabTunerCtrl_destroy_receiver(Te_Frontend_type e_Frontend_type)             */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Destroy_Receiver_cmd(Te_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	ETAL_HANDLE *Receiver_Handle = NULL;
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiver_Handle = &dabFG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Receiver_Handle = &dabBG_recvhdl;
	}
	else
	{
		/*MISRA C*/
	}
	e_RetStatus = etal_destroy_receiver(Receiver_Handle);
	return e_RetStatus;
}
/*=============================================================================*/
/* DabTunerCtrl_Enable_Data_Service_cmd(void)                                  */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Enable_Data_Service_cmd(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	EtalDataServiceType e_Servicebitmap;
	EtalDataServiceParam st_ServiceParameters;
	/* Not enabling ETAL_DATASERV_TYPE_DLPLUS as ETAL library doesn't support */
	/* To be updated in future */
	e_Servicebitmap = ETAL_DATASERV_TYPE_SLS_XPAD;
	st_ServiceParameters.m_ecc = 0;
	st_ServiceParameters.m_eid = 0;
	st_ServiceParameters.m_sid = 0;
	st_ServiceParameters.m_logoType = ETAL_EPG_LOGO_TYPE_UNKNOWN;
	st_ServiceParameters.m_JMLObjectId = 0;
	e_RetStatus = etal_enable_data_service(dabFG_recvhdl, e_Servicebitmap, (Tu32*)(&e_Enabled_Servicesbitmap), st_ServiceParameters);

	if (e_Enabled_Servicesbitmap != e_Servicebitmap)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_HAL] Few Data Services are not enabled!");
	}

	return e_RetStatus;
}

/*=============================================================================*/
/* DabTunerCtrl_Disable_Data_Service_cmd(void)                                 */
/*=============================================================================*/
ETAL_STATUS DabTunerCtrl_Disable_Data_Service_cmd(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	EtalDataServiceType e_Servicebitmap = ETAL_DATASERV_TYPE_SLS_XPAD;
	e_RetStatus = etal_disable_data_service(dabFG_recvhdl, e_Servicebitmap);
	return e_RetStatus;
}
/*===========================================================================*/
/* ETAL_STATUS DabTunerCtrl_Enable_FICData_cmd(void)						 */
/*===========================================================================*/
ETAL_STATUS DabTunerCtrl_Enable_FICData_cmd(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	e_RetStatus = etal_get_fic(dabFG_recvhdl);
	return e_RetStatus;
}

/*==========================================================*/
/*  void DabTuner_StartTimeStretch_cmd(Tu32 u32_TimeToStretch)					*/
/*==========================================================*/
void DabTuner_StartTimeStretch_cmd(Tu32 u32_TimeToStretch)
{
	UNUSED(u32_TimeToStretch);
	//DABTuner_StartTimeStretch_cmd(u32_TimeToStretch);
}

/*==========================================================*/
/*  void DabTuner_CancelStartTimeStretch_cmd(Tu32 u32_TimeToStretch)					*/
/*==========================================================*/
void DabTuner_CancelStartTimeStretch_cmd(Tu32 u32_TimeToStretch)
{
	UNUSED(u32_TimeToStretch);
	//DABTuner_CancelStartTimeStretch_cmd(u32_TimeToStretch);
}

/*==========================================================*/
/*  Tbool DabTuner_StartTimeStretch_repl					*/
/*==========================================================*/
Tbool DabTuner_StartTimeStretch_repl(Ts_StartTimeStretch_repl *pst_StartTimeStretch_repl, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_StartTimeStretch_repl);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_StartTimeStretch_repl(pst_StartTimeStretch_repl, pu8_Msg);
	return b_ret;
}

/*==========================================================*/
/*  Tbool DabTuner_StartTimeStretch_not						*/
/*==========================================================*/
Tbool DabTuner_StartTimeStretch_not(Ts_StartTimeStretch_not *pst_StartTimeStretch_not, Ts8 *pu8_Msg)
{
	Tbool b_ret = FALSE;
	UNUSED(pst_StartTimeStretch_not);
	UNUSED(pu8_Msg);
	//b_ret = DABTuner_StartTimeStretch_not(pst_StartTimeStretch_not, pu8_Msg);
	return b_ret;
}

/*==========================================================*/
/*  ETAL_STATUS DabTuner_ConfigQualityMonitor				*/
/*==========================================================*/
ETAL_STATUS DabTuner_Config_FGQualityMonitor(Te_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	ETAL_HANDLE Receiver_Handle = ETAL_INVALID_HANDLE;

	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiver_Handle = dabFG_recvhdl;
	}
	else 
	{
		//do nothing
	}
	memset(&(st_BcastQualityMonitor), 0, sizeof(EtalBcastQualityMonitorAttr));
	st_BcastQualityMonitor.m_receiverHandle = Receiver_Handle;
	st_BcastQualityMonitor.m_CbBcastQualityProcess = DAB_Quality_Monitor_Callback;
	st_BcastQualityMonitor.m_Context =NULL ;
	st_BcastQualityMonitor.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_DabMscBer;
	st_BcastQualityMonitor.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR; // DON'T CARE
	st_BcastQualityMonitor.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR;//// DON'T CARE
	st_BcastQualityMonitor.m_monitoredIndicators[0].m_UpdateFrequency = QUALITYMONITOR_NOTIFYTIME;
	st_BcastQualityMonitor.m_monitoredIndicators[1].m_MonitoredIndicator = EtalQualityIndicator_DabFicErrorRatio;
	st_BcastQualityMonitor.m_monitoredIndicators[1].m_InferiorValue = ETAL_INVALID_MONITOR; // DON'T CARE
	st_BcastQualityMonitor.m_monitoredIndicators[1].m_SuperiorValue = ETAL_INVALID_MONITOR;// DON'T CARE
	st_BcastQualityMonitor.m_monitoredIndicators[1].m_UpdateFrequency = QUALITYMONITOR_NOTIFYTIME;
	
	e_RetStatus = etal_config_reception_quality_monitor(&dabFG_Monitor_Handle, &st_BcastQualityMonitor);
	return e_RetStatus;
}

/*==========================================================*/
/*  ETAL_STATUS  DabTuner_DestroyQualityMonitor			*/
/*==========================================================*/
ETAL_STATUS  DabTuner_Destroy_FGQualityMonitor(void)
{
	ETAL_STATUS e_RetStatus = ETAL_RET_ERROR;
	e_RetStatus = etal_destroy_reception_quality_monitor(&dabFG_Monitor_Handle);
	return e_RetStatus;
}

/*==========================================================*/
/* 	void DAB_Quality_Monitor_Callback						*/
/*==========================================================*/
void DAB_Quality_Monitor_Callback(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	Ts_Sys_Msg* msg = NULL;

	UNUSED(pQuality);
	UNUSED(vpContext);

	msg = SYS_MSG_HANDLE_Call(RADIO_DAB_TUNER_CTRL, DAB_QUALITY_NOTIFICATION_MSGID);
	UpdateParameterIntoMessage((Tchar *)(msg->data), pQuality, (Tu8)(sizeof(EtalBcastQualityContainer)), &(msg->msg_length));
	SYS_SEND_MSG(msg);
}
/*=============================================================================
end of file
=============================================================================*/

