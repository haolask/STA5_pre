/*==================================================================================================
    start of file
==================================================================================================*/
/************************************************************************************************************/
/** \file DAB_Tuner_Ctrl_App.c																       *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: DAB Tuner Control															     	*
*  Description			: This file contains DAB Tuner Control  HSM related  functions             *
                          and common function definitions.		                                   *
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "DAB_Tuner_Ctrl_main_hsm.h"
#include "DAB_Tuner_Ctrl_app.h"
#include "dab_app_freq_band.h"
#include "lib_bitmanip.h"
#include "sys_nvm.h"
/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
Ts_Tuner_Ctrl_main st_DAB_Tuner_Ctrl_hsm;
extern Ts_Tuner_Ctrl_EnsembleInfo		ast_EnsembleInfo[DAB_APP_MAX_ENSEMBLES];
extern Ts_Tuner_Ctrl_ServiceInfo 		ast_ServiceInfo[DAB_APP_MAX_SERVICES];
extern Ts_Tuner_Ctrl_ComponentInfo		ast_ComponentInfo[DAB_APP_MAX_COMPONENTS];
extern Ts_dab_tuner_ctrl_inst_timer st_TimerId;
extern  Tu8 	ALternateFreq;
extern 	Ts_dab_app_frequency_table dab_app_freq_band_eu[] ;
extern Tu16 	ensembleIndex;
extern Tu16 	Components;
extern Tu16 	Services;

Tu8 Freq_index = 0;

Tu16 Learnmem_station_count = 0;

extern Ts_Tuner_Ctrl_Tunableinfo		ast_LearnMem[DAB_APP_MAX_COMPONENTS];
extern Tu8								u8_Alt_Ensemble_ID;
extern Tu8								AltEnsembleindex;
extern Tu8								Hardlinkindex;
extern Tu8 								Altfreqindex;

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_MsgHandlr(Ts_Sys_Msg* msg)                                                 */
/*================================================================================================*/

void DAB_Tuner_Ctrl_MsgHandlr(Ts_Sys_Msg* msg)
{
	HSM_ON_MSG(&st_DAB_Tuner_Ctrl_hsm,msg);
}

/*================================================================================================*/
/* void DAB_Tuner_Ctrl_Component_Init()                                                           */
/*================================================================================================*/
void DAB_Tuner_Ctrl_Component_Init()
{
	
	DAB_Tuner_Ctrl_HSM_Init(&st_DAB_Tuner_Ctrl_hsm);
}

/*================================================================================================*/
/*  void DAB_Tuner_Ctrl_hsm_inst_start(Tu16 dest_cid, Tu16 comp_msgid)                            */
/*================================================================================================*/
void DAB_Tuner_Ctrl_hsm_inst_start(Tu16 dest_cid, Tu16 comp_msgid)
{
  Ts_Sys_Msg* p_msg = NULL;
  p_msg = SYS_MSG_HANDLE_Call(dest_cid,comp_msgid);
  SYS_SEND_MSG(p_msg);

}

/*================================================================================================*/
/*  void DAB_Tuner_Ctrl_hsm_inst_stop(Tu16 dest_cid, Tu16 comp_msgid)                             */
/*================================================================================================*/
void DAB_Tuner_Ctrl_hsm_inst_stop(Tu16 dest_cid, Tu16 comp_msgid)
{
  Ts_Sys_Msg* p_msg = NULL;
  p_msg = SYS_MSG_HANDLE_Call(dest_cid,comp_msgid);
  SYS_SEND_MSG(p_msg);

}
void UpdateCurrentEnsembleServiceList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_ProgrammeServListChanged_not)
{
	Tu8	  u8_ServiceIndex = 0, u8_LabelIndex = 0, u8_ServiceCompIndex ;
	memset(pst_GetCurrEnsembleProgListReply,0,sizeof(Ts_DabTunerMsg_GetCurrEnsembleProgListReply));
		
	pst_GetCurrEnsembleProgListReply->u8_NumOfServices = pst_ProgrammeServListChanged_not->u8_NumOfServices;
	if(pst_ProgrammeServListChanged_not->u8_NumOfServices > MAX_ENSEMBLE_SERVICES)
	{
		pst_GetCurrEnsembleProgListReply->u8_NumOfServices = MAX_ENSEMBLE_SERVICES;	
	}
	else
	{
		/*Doing nothing */	
	}/* For MISRA C */
	
	for(u8_ServiceIndex = 0; (u8_ServiceIndex < pst_GetCurrEnsembleProgListReply->u8_NumOfServices) && (u8_ServiceIndex < MAX_ENSEMBLE_SERVICES); u8_ServiceIndex++)
	{
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CA_Applied	= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].CA_Applied ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CAId 			= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].CAId ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CharSet  		= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].CharSet ;
		
		for(u8_LabelIndex = 0 ; u8_LabelIndex < DAB_TUNER_CTRL_MAX_LABEL_LENGTH; u8_LabelIndex++)
		{
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].LabelString[u8_LabelIndex] = pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].LabelString[u8_LabelIndex] ;
		}
		
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ShortLabelFlags	= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].ShortLabelFlags ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ProgServiceId  	= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].ProgServiceId ;	
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].InternTableIdSPTy	= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].InternTableIdSPTy ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].StatPTy          	= pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].StatPTy ;		 
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].InternTableIdDPTy = pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].InternTableIdDPTy ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].DynPTy            = pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].DynPTy ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.NoOfComponents = 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.NoOfComponents ;
		for(u8_ServiceCompIndex = 0 ;u8_ServiceCompIndex < pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.NoOfComponents ; u8_ServiceCompIndex++)
		{
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ShortLabelFlags	= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ShortLabelFlags ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].UAType 			= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].UAType ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].InternalCompId 	= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].InternalCompId ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Primary 			= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Primary ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CA_Applied 		= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CA_Applied ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Activated 			= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Activated ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet 			= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Language 			= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Language ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].TransportMech 		= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].TransportMech ;
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ComponentType 		= 	pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ComponentType ;
			for(u8_LabelIndex = 0 ; u8_LabelIndex < DAB_TUNER_CTRL_MAX_LABEL_LENGTH; u8_LabelIndex++)
			{
				pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].LabelString[u8_LabelIndex] = pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].LabelString[u8_LabelIndex];
			}			
		}	
			 
	}
}

void UpdateEnsembleInfo(Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData)
{
	/* If the station list is not created then ensembleIndex == 0, in that case ensmble is added to ast_EnsembleInfo */
	if(ensembleIndex == 0)
	{
		ast_EnsembleInfo[0].st_BasicEnsInfo.u32_Frequency 	= pst_currentEnsembleData->u32_Frequency ; 
		ast_EnsembleInfo[0].st_BasicEnsInfo.u16_EId 		= pst_currentEnsembleData->u16_EId ; 
		ast_EnsembleInfo[0].st_BasicEnsInfo.u8_ECC			= pst_currentEnsembleData->u8_ECC ;
		ensembleIndex++ ;
	}
	else
	{
		/* Doing nothing */	
	}/* For MISRA C */
	
}

void UpdateServiceInfo(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData)
{
	Tu8 u8_ServiceIndex = 0, u8_ServiceCount = 0, u8_ServiceStartIndex = 0;
	
	for(u8_ServiceIndex = 0 ; u8_ServiceIndex < Services ; u8_ServiceIndex++)
	{
		/* Finding the no.of services belongs to current ensemble in ast_ServiceInfo */
		if(ast_ServiceInfo[u8_ServiceIndex].st_BasicEnsInfo.u32_Frequency == pst_currentEnsembleData->u32_Frequency
			&&	ast_ServiceInfo[u8_ServiceIndex].st_BasicEnsInfo.u16_EId == pst_currentEnsembleData->u16_EId )
		{
			u8_ServiceCount++; 
			if(u8_ServiceCount == 1)/* Finding the start index of services belongs to current ensemble */
			{
				u8_ServiceStartIndex = 	u8_ServiceIndex ; 
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
		}
		else
		{
			/*Doing nothing */ 	
		}/* For MISRA C */
		
	}
	/* Removing the current ensemble services in ast_ServiceInfo */
	memmove(&ast_ServiceInfo[u8_ServiceStartIndex],&ast_ServiceInfo[(u8_ServiceStartIndex + u8_ServiceCount)],sizeof(Ts_Tuner_Ctrl_ServiceInfo) * (DAB_APP_MAX_SERVICES - (u8_ServiceStartIndex + u8_ServiceCount)));
	memset(&ast_ServiceInfo[(Services - u8_ServiceCount)],0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * (DAB_APP_MAX_SERVICES - (Services - u8_ServiceCount)));
	
	Services = (Tu16)(Services - u8_ServiceCount) ; /* Updating the no.of services */
	
	for(u8_ServiceIndex = 0 ; u8_ServiceIndex < pst_GetCurrEnsembleProgListReply->u8_NumOfServices ; u8_ServiceIndex++, Services++)
	{
		ast_ServiceInfo[Services].st_BasicEnsInfo.u32_Frequency = pst_currentEnsembleData->u32_Frequency ; 
		ast_ServiceInfo[Services].st_BasicEnsInfo.u16_EId 		= pst_currentEnsembleData->u16_EId ; 
		ast_ServiceInfo[Services].st_BasicEnsInfo.u8_ECC 		= pst_currentEnsembleData->u8_ECC ;
		
		SYS_RADIO_MEMCPY((ast_ServiceInfo[Services].st_ServiceLabel.au8_label),(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) ;
		
		if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CharSet == 0x00)
		{
			ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
		}
		else if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CharSet == 0x06)
		{
			ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
		}
		else if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CharSet == 0x0f)
		{
			ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
		}
		else
		{
			/*Doing nothing */ 	
		}/* For MISRA C */
		
		ast_ServiceInfo[Services].st_ServiceLabel.u16_ShortLabelFlags =	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ShortLabelFlags ;
		ast_ServiceInfo[Services].u8_CA_Applied 	= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CA_Applied ;
		ast_ServiceInfo[Services].u8_PDFlag 		= 	0; /* Indicates Programe service */
		ast_ServiceInfo[Services].u32_SId 			= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ProgServiceId ;
		ast_ServiceInfo[Services].u8_CAId 			= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CAId ;
		ast_ServiceInfo[Services].u8_ptyCode 		= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].StatPTy ;
		ast_ServiceInfo[Services].u8_dynPtyCode 	= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].DynPTy ;
	}	 
}

void UpdateServiceCompInfo(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData)
{
	Tu8 u8_ServiceCompIndex = 0, u8_ServiceCompCount = 0, u8_ServiceCompStartIndex = 0, u8_ServiceIndex = 0;
	
	for(u8_ServiceCompIndex = 0 ; u8_ServiceCompIndex < Components ; u8_ServiceCompIndex++)
	{
		/* Finding the no.of service components belongs to current ensemble in ast_ComponentInfo */
		if(ast_ComponentInfo[u8_ServiceCompIndex].st_BasicEnsInfo.u32_Frequency == pst_currentEnsembleData->u32_Frequency
			&&	ast_ComponentInfo[u8_ServiceCompIndex].st_BasicEnsInfo.u16_EId == pst_currentEnsembleData->u16_EId )
		{
			u8_ServiceCompCount++; 
			if(u8_ServiceCompCount == 1)/* Finding the start index of the service components belongs to current ensemble */
			{
				u8_ServiceCompStartIndex = 	u8_ServiceCompIndex ; 
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
		}
		else
		{
			/*Doing nothing */ 	
		}/* For MISRA C */
	}
	/* Removing the current ensemble service components in ast_ComponentInfo */
	memmove(&ast_ComponentInfo[u8_ServiceCompStartIndex],&ast_ComponentInfo[(u8_ServiceCompStartIndex + u8_ServiceCompCount)],sizeof(Ts_Tuner_Ctrl_ServiceInfo) * (DAB_APP_MAX_COMPONENTS - (u8_ServiceCompStartIndex + u8_ServiceCompCount)));
	memset(&ast_ComponentInfo[(Components - u8_ServiceCompCount)],0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * (DAB_APP_MAX_COMPONENTS - (Components - u8_ServiceCompCount)));
	
	Components = (Tu16)(Components - u8_ServiceCompCount); /* Updating the no.of components */
	
	for(u8_ServiceIndex = 0 ; u8_ServiceIndex < pst_GetCurrEnsembleProgListReply->u8_NumOfServices ; u8_ServiceIndex++)
	{
		for(u8_ServiceCompIndex = 0 ; u8_ServiceCompIndex < pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.NoOfComponents ; u8_ServiceCompIndex++,Components++)
		{
			ast_ComponentInfo[Components].st_BasicEnsInfo.u32_Frequency	= pst_currentEnsembleData->u32_Frequency ; 
			ast_ComponentInfo[Components].st_BasicEnsInfo.u16_EId 		= pst_currentEnsembleData->u16_EId ; 
			ast_ComponentInfo[Components].st_BasicEnsInfo.u8_ECC 		= pst_currentEnsembleData->u8_ECC ;
			
			SYS_RADIO_MEMCPY((ast_ComponentInfo[Components].st_compLabel.au8_label),(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) ;
			
			if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet == 0x00)
			{
				ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
			}
			else if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet == 0x06)
			{
				ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
			}
			else if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet == 0x0f)
			{
				ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
			
			ast_ComponentInfo[Components].st_compLabel.u16_ShortLabelFlags	= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ShortLabelFlags ;
					
			ast_ComponentInfo[Components].u32_SId			=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ProgServiceId ;
			ast_ComponentInfo[Components].u16_SCIdI			=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].InternalCompId ;	
			ast_ComponentInfo[Components].u8_Primary		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Primary ;
			ast_ComponentInfo[Components].u8_CA_Applied		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CA_Applied ;
			ast_ComponentInfo[Components].u16_UAType		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].UAType ;
			ast_ComponentInfo[Components].u8_Language		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Language ;
			ast_ComponentInfo[Components].u8_CAId			=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CAId ;
			ast_ComponentInfo[Components].u8_TransportMech	=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].TransportMech ;
			ast_ComponentInfo[Components].u8_ComponentType	=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ComponentType ;
		}
	}
}

void UpdateCurrentServiceCompsToServiceCompInfo(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData, Tu8 u8_ServiceIndex)
{
	Tu8 u8_ServiceCompIndex = 0, u8_ServiceCompCount = 0, u8_ServiceCompStartIndex = 0 ;
	
	for(u8_ServiceCompIndex = 0 ; u8_ServiceCompIndex < Components ; u8_ServiceCompIndex++)
	{
		/* Finding the no.of service components belongs to current SId in ast_ComponentInfo */
		if(ast_ComponentInfo[u8_ServiceCompIndex].st_BasicEnsInfo.u32_Frequency == pst_currentEnsembleData->u32_Frequency
			&&	ast_ComponentInfo[u8_ServiceCompIndex].st_BasicEnsInfo.u16_EId == pst_currentEnsembleData->u16_EId
			&&  ast_ComponentInfo[u8_ServiceCompIndex].u32_SId == pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ProgServiceId)
		{
			u8_ServiceCompCount++; 
			if(u8_ServiceCompCount == 1)/* Finding the start index of the service components belongs to current SId */
			{
				u8_ServiceCompStartIndex = 	u8_ServiceCompIndex ; 
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */
		}
		else
		{
			/*Doing nothing */ 	
		}/* For MISRA C */
	}
	/* Removing the current service components in ast_ComponentInfo */
	memmove(&ast_ComponentInfo[u8_ServiceCompStartIndex],&ast_ComponentInfo[(u8_ServiceCompStartIndex + u8_ServiceCompCount)],sizeof(Ts_Tuner_Ctrl_ServiceInfo) * (DAB_APP_MAX_COMPONENTS - (u8_ServiceCompStartIndex + u8_ServiceCompCount)));
	memset(&ast_ComponentInfo[(Components - u8_ServiceCompCount)],0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * (DAB_APP_MAX_COMPONENTS - (Components - u8_ServiceCompCount)));
	
	Components = (Tu16)(Components - u8_ServiceCompCount) ; /* Updating the no.of components */
	
	for(u8_ServiceCompIndex = 0 ; u8_ServiceCompIndex < pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.NoOfComponents ; u8_ServiceCompIndex++,Components++)
	{
		ast_ComponentInfo[Components].st_BasicEnsInfo.u32_Frequency	= pst_currentEnsembleData->u32_Frequency ; 
		ast_ComponentInfo[Components].st_BasicEnsInfo.u16_EId 		= pst_currentEnsembleData->u16_EId ; 
		ast_ComponentInfo[Components].st_BasicEnsInfo.u8_ECC 		= pst_currentEnsembleData->u8_ECC ;
		
		SYS_RADIO_MEMCPY((ast_ComponentInfo[Components].st_compLabel.au8_label),(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) ;
		
		if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet == 0x00)
		{
			ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
		}
		else if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet == 0x06)
		{
			ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
		}
		else if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CharSet == 0x0f)
		{
			ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
		}
		else
		{
			/*Doing nothing */ 	
		}/* For MISRA C */
		
		ast_ComponentInfo[Components].st_compLabel.u16_ShortLabelFlags	= 	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ShortLabelFlags ;
				
		ast_ComponentInfo[Components].u32_SId			=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ProgServiceId ;
		ast_ComponentInfo[Components].u16_SCIdI			=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].InternalCompId ;	
		ast_ComponentInfo[Components].u8_Primary		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Primary ;
		ast_ComponentInfo[Components].u8_CA_Applied		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].CA_Applied ;
		ast_ComponentInfo[Components].u16_UAType		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].UAType ;
		ast_ComponentInfo[Components].u8_Language		=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].Language ;
		ast_ComponentInfo[Components].u8_CAId			=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CAId ;
		ast_ComponentInfo[Components].u8_TransportMech	=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].TransportMech ;
		ast_ComponentInfo[Components].u8_ComponentType	=	pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].st_compInfo.Component[u8_ServiceCompIndex].ComponentType ;
	}
}



/*===============================================================================================================================================================================================================*/
/* Tbool CompareCurrentEnsembleServiceList                                                                                                                                                                       */
/*================================================================================================================================================================================================================*/
Tbool CompareCurrentEnsembleServiceList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_ProgrammeServListChanged_not, Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply)
{
	Tu8 u8_ServiceCompareIndex = 0 ;
	Tbool b_ResultCode = FALSE ;
	
	for(u8_ServiceCompareIndex = 0 ; u8_ServiceCompareIndex < pst_ProgrammeServListChanged_not->u8_NumOfServices ; u8_ServiceCompareIndex++)
	{
		if((pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].CA_Applied) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].CA_Applied) 
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].CAId) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].CAId)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].CharSet) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].CharSet)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].ShortLabelFlags) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].ShortLabelFlags)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].ProgServiceId) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].ProgServiceId)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].InternTableIdSPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].InternTableIdSPTy)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].StatPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].StatPTy)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].InternTableIdDPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].InternTableIdDPTy)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].DynPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].DynPTy)
			&& (strcmp((char *)pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].LabelString,(char *)pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].LabelString) == 0)
			&& (pst_ProgrammeServListChanged_not->st_serviceinfo[u8_ServiceCompareIndex].st_compInfo.NoOfComponents == pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceCompareIndex].st_compInfo.NoOfComponents)
		)
		{
			/* If the Service information is matched then updating b_ResultCode = TRUE and further checking happens */
			b_ResultCode = TRUE ;
		}
		else
		{
			/* If the Service information is not matched then updating b_ResultCode = FALSE and further checking is stopped */
			b_ResultCode = FALSE ;
		}
		
		/* Either service or else component is not mathced then further checking is stopped */
		if(b_ResultCode == FALSE )
		{
			break;
		}
		else
		{
			/* Doing nothing */	
		}/* For MISRA C */
	}
	return b_ResultCode ;
}

void UpdateCurrentEnsembleProperties(Ts_DabTunerGetEnsembleProperties_reply	*pst_GetEnsembleProperties_reply,Ts_DabTunerMsg_R_ScanStatus_Not *pst_DAB_Tuner_Ctrl_ScanNotification)
{
	Tu8 u8_LabelIndex ;
	pst_GetEnsembleProperties_reply->ECC 				= 	pst_DAB_Tuner_Ctrl_ScanNotification->ECC ;
	pst_GetEnsembleProperties_reply->EnsembleIdentifier = 	pst_DAB_Tuner_Ctrl_ScanNotification->EnsembleIdentifier ;
	pst_GetEnsembleProperties_reply->CharSet 			= 	pst_DAB_Tuner_Ctrl_ScanNotification->CharSet ;
	pst_GetEnsembleProperties_reply->ShortLabelFlags 	= 	(Tu16)(LIB_CONCATENATE((pst_DAB_Tuner_Ctrl_ScanNotification->ShortLabelFlags[0] & 0xFF), 8, (pst_DAB_Tuner_Ctrl_ScanNotification->ShortLabelFlags[1])));
					                                            
	for(u8_LabelIndex=0;u8_LabelIndex < DAB_TUNER_CTRL_MAX_LABEL_LENGTH ;u8_LabelIndex++)
	{
		pst_GetEnsembleProperties_reply->LabelString[u8_LabelIndex] =  pst_DAB_Tuner_Ctrl_ScanNotification->LabelString[u8_LabelIndex];
	}
	
}

void UpdateCurrentEnsembleProgramList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_DabTunerMsg_R_ScanStatus_Not *pst_DAB_Tuner_Ctrl_ScanNotification)
{
	Tu8 u8_ServiceIndex, u8_LabelIndex ;
	pst_GetCurrEnsembleProgListReply->u8_NumOfServices = pst_DAB_Tuner_Ctrl_ScanNotification->num_of_Ser ;
	
	for(u8_ServiceIndex = 0 ; u8_ServiceIndex < pst_DAB_Tuner_Ctrl_ScanNotification->num_of_Ser ; u8_ServiceIndex++ )
	{
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ProgServiceId 		=	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].ServiceId ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].ShortLabelFlags 		=( Tu8)	(((pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].ShortLabelFlags[0]&0xFF)<<8)|
																										(pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].ShortLabelFlags[1])) ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CA_Applied 			=	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].CA_Applied ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CAId 					= 	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].CAId ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].CharSet 				= 	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].CharSet ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].InternTableIdSPTy 	= 	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].InternTableIdSPTy ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].StatPTy 				= 	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].StatPTy ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].InternTableIdDPTy 	= 	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].InternTableIdDPTy ;
		pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].DynPTy 				= 	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].DynPTy ;
		
		for(u8_LabelIndex = 0 ; u8_LabelIndex < DAB_TUNER_CTRL_MAX_LABEL_LENGTH ; u8_LabelIndex++)
		{
			pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_ServiceIndex].LabelString[u8_LabelIndex] 	=	pst_DAB_Tuner_Ctrl_ScanNotification->t_sat_ScanStatusNot[u8_ServiceIndex].LabelString[u8_LabelIndex] ;
		}	
	}
}

Tu8 FindFrequencyIndex(Tu32 u32_Station_Frequency)
{
	Tu8 u8_CurrentFrequencyIndex = (Tu8)0 ; 
	Tu8 u8_Returned_IndexValue = 50;
	
	for(u8_CurrentFrequencyIndex = (Tu8)0 ; u8_CurrentFrequencyIndex < (Tu8)DAB_TUNER_CTRL_EU_MAX_FREQUENCIES ; u8_CurrentFrequencyIndex++)
	{
		/* Finding the current frequency in Band III */
		if(dab_app_freq_band_eu[u8_CurrentFrequencyIndex].u32_frequency == u32_Station_Frequency)
		{
			u8_Returned_IndexValue = u8_CurrentFrequencyIndex;
			break;
		}
		else
		{
			/* Doing nothing */
		}/* for MISRA C */
	}
 	return u8_Returned_IndexValue;	
	
}

Tu8 FindSeekFrequencyIndex(Tu32 u32_SeekStartFrequency, Te_RADIO_DirectionType e_Direction)
{
	Tu8 u8_CurrentFrequencyIndex = (Tu8)0 ; 
		
	if((u32_SeekStartFrequency == DAB_TUNER_CTRL_5A_EU)&&(e_Direction == RADIO_FRMWK_DIRECTION_DOWN)) 
	{
		u8_CurrentFrequencyIndex = (Tu8)(DAB_TUNER_CTRL_EU_MAX_FREQUENCIES - 1);
	}
	else if((u32_SeekStartFrequency == DAB_TUNER_CTRL_13F_EU)&&(e_Direction == RADIO_FRMWK_DIRECTION_UP))
	{
		u8_CurrentFrequencyIndex = (Tu8)0 ;
	}
	else
	{
		for(u8_CurrentFrequencyIndex = (Tu8)0 ; u8_CurrentFrequencyIndex < (Tu8)DAB_TUNER_CTRL_EU_MAX_FREQUENCIES ; u8_CurrentFrequencyIndex++)
		{
			/* Finding the current frequency in Band III */
			if(dab_app_freq_band_eu[u8_CurrentFrequencyIndex].u32_frequency == u32_SeekStartFrequency)
			{
				break ;
			}
			else
			{
				/* Doing nothing */
			}/* for MISRA C */
		}
		
		if(e_Direction == RADIO_FRMWK_DIRECTION_UP)
		{
			u8_CurrentFrequencyIndex = (Tu8)(u8_CurrentFrequencyIndex + 1) ;
		}
		else if(e_Direction == RADIO_FRMWK_DIRECTION_DOWN)
		{
			u8_CurrentFrequencyIndex = (Tu8)(u8_CurrentFrequencyIndex - 1) ;
		}
		else
		{
			/* Doing nothing */
		}/* for MISRA C */
	}
	
	return u8_CurrentFrequencyIndex ;
} 

void DAB_Tuner_Ctrl_UpdateDABServiceType(Ts_GetAudioProperties_repl *pst_GetAudioProperties_repl,Te_Tuner_Ctrl_ServiceType *pe_ServiceType)
{
	if(pst_GetAudioProperties_repl->SourceEncoding == (Tu8)DAB_TUNER_CTRL_AUDIO_TYPE_MPEG1_LAYER_II)
	{
		*pe_ServiceType = DAB_TUNER_CTRL_SERVICE_TYPE_DAB ;
	}
	else if(pst_GetAudioProperties_repl->SourceEncoding == (Tu8)DAB_TUNER_CTRL_AUDIO_TYPE_MPEG2_LAYER_II)
	{
		*pe_ServiceType = DAB_TUNER_CTRL_SERVICE_TYPE_DAB ;
	}
	else if(pst_GetAudioProperties_repl->SourceEncoding == (Tu8)DAB_TUNER_CTRL_AUDIO_TYPE_AAC_PLUS_DAB_PLUS)
	{
		*pe_ServiceType = DAB_TUNER_CTRL_SERVICE_TYPE_DAB_PLUS ;
	}
	else if(pst_GetAudioProperties_repl->SourceEncoding == (Tu8)DAB_TUNER_CTRL_AUDIO_TYPE_AAC_PLUS_TDMB_PLUS)
	{
		*pe_ServiceType = DAB_TUNER_CTRL_SERVICE_TYPE_DMB ;
	}
	else if(pst_GetAudioProperties_repl->SourceEncoding == (Tu8)DAB_TUNER_CTRL_AUDIO_TYPE_BSAC)
	{
		*pe_ServiceType = DAB_TUNER_CTRL_SERVICE_TYPE_DMB ;
	}
	else 
	{
		*pe_ServiceType = DAB_TUNER_CTRL_SERVICE_TYPE_INVALID ;
	}
}

/*================================================================================================*/
/* Ts32 Tuner_Ctrl_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size)   */
/*================================================================================================*/
Ts32 Tuner_Ctrl_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size)
{
    Tu8 count=0;
    while( (count < size -1) && ( src[count]!=0 && dst[count]!=0) )
    {
        if(src[count]!=dst[count])
        {
            return src[count]-dst[count];
        }

        count++;
    }
    return src[count]-dst[count];
}
/*================================================================================================*/
/* void Tuner_Ctrl_Sort_CurrEnsembleProgList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply* CurrEnsembleProgList)   */
/*================================================================================================*/
void Tuner_Ctrl_Sort_CurrEnsembleProgList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *CurrEnsembleProgList)
{
	
	Tu16 i,j;
	Ts_CurrEnsemble_serviceinfo st_TempServiceInfo;
	
		
	for(i = 0 ; ((i < CurrEnsembleProgList->u8_NumOfServices) && (i < MAX_ENSEMBLE_SERVICES)) ; i++)
	{
		for(j = (Tu16)(i+1) ; ((j < CurrEnsembleProgList->u8_NumOfServices) && ( j < MAX_ENSEMBLE_SERVICES)) ; j++)
		{
						
			if(Tuner_Ctrl_String_comparison(CurrEnsembleProgList->st_serviceinfo[i].LabelString,CurrEnsembleProgList->st_serviceinfo[j].LabelString,DAB_TUNER_CTRL_MAX_LABEL_LENGTH) > (Tu8)0)
			{
				st_TempServiceInfo = CurrEnsembleProgList->st_serviceinfo[i];
				CurrEnsembleProgList->st_serviceinfo[i] = CurrEnsembleProgList->st_serviceinfo[j] ;
				CurrEnsembleProgList->st_serviceinfo[j] = st_TempServiceInfo ;
			}
			else
			{
				
			}/* For misra c */
		
		}
	}
	
}
/*================================================================================================*/
/* void Tuner_Ctrl_Sort_ProgrammeServListChanged_not(Ts_DabTunerMsg_ProgrammeServListChanged_not* pst_ProgrammeServListChanged_not)   */
/*================================================================================================*/
void Tuner_Ctrl_Sort_ProgrammeServListChanged_not(Ts_DabTunerMsg_ProgrammeServListChanged_not *pst_ProgrammeServListChanged_not)
{
	
	Tu16 i,j;
	Ts_Msg_Service_Info st_TempServiceInfo;
	
		
	for(i = 0 ; ((i < pst_ProgrammeServListChanged_not->NrOfServices) && (i < MAX_ENSEMBLE_SERVICES)) ; i++)
	{
		for(j = (Tu16)(i+1) ; ((j < pst_ProgrammeServListChanged_not->NrOfServices) && ( j < MAX_ENSEMBLE_SERVICES)) ; j++)
		{
						
			if(strcmp((char *)pst_ProgrammeServListChanged_not->Service[i].LabelString,(char *)pst_ProgrammeServListChanged_not->Service[j].LabelString) > 0)
			{
				st_TempServiceInfo = pst_ProgrammeServListChanged_not->Service[i];
				pst_ProgrammeServListChanged_not->Service[i] = pst_ProgrammeServListChanged_not->Service[j] ;
				pst_ProgrammeServListChanged_not->Service[j] = st_TempServiceInfo ;
			}
			else
			{
				
			}/* For misra c */
		}
	}
}
/*=======================================================*/
/* Tbool CompareCurrentEnsembleServices	 */
/*=======================================================*/
Tbool CompareCurrentEnsembleServices(Ts_DabTunerMsg_ProgrammeServListChanged_not *pst_ProgrammeServListChanged_not, Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply)
{
	Tu8 u8_CompareIndex = 0;
	Tbool b_ResultCode = FALSE ;
	
	for(u8_CompareIndex = 0 ; u8_CompareIndex < pst_ProgrammeServListChanged_not->NrOfServices ; u8_CompareIndex++)
	{
		if((pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].CA_Applied) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].CA_Applied) 
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].CAId) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].CAId)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].CharSet) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].CharSet)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].ShortLabelFlags) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].ShortLabelFlags)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].ProgServiceId) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].ProgServiceId)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].InternTableIdSPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].InternTableIdSPTy)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].StatPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].StatPTy)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].InternTableIdDPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].InternTableIdDPTy)
			&& (pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].DynPTy) == (pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].DynPTy)
			&& (strcmp((char *)pst_ProgrammeServListChanged_not->Service[u8_CompareIndex].LabelString,(char *)pst_GetCurrEnsembleProgListReply->st_serviceinfo[u8_CompareIndex].LabelString) == 0)
		)
		{
			b_ResultCode = TRUE;
		}
		else
		{
			b_ResultCode = FALSE ;
		}
	}
	return b_ResultCode ;
}

void DAB_Tuner_Ctrl_Update_Label(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply)
{
	
	Tu8 Service_index = 0;
	Tu8 Service_Station_index = 0;
	for(Service_index = 0; Service_index < pst_GetCurrEnsembleProgListReply->u8_NumOfServices; Service_index++)
	{
	
		if(pst_GetCurrEnsembleProgListReply->st_serviceinfo[Service_index].LabelString[0] == 0 && pst_GetCurrEnsembleProgListReply->st_serviceinfo[Service_index].LabelString[1] == 0)
		{
			Service_Station_index = 0;
			while(Service_Station_index < DAB_APP_MAX_SERVICES && ast_ServiceInfo[Service_Station_index].u32_SId != 0)
			{
				if(ast_ServiceInfo[Service_Station_index].u32_SId == pst_GetCurrEnsembleProgListReply->st_serviceinfo[Service_index].ProgServiceId)
				{
					SYS_RADIO_MEMCPY((pst_GetCurrEnsembleProgListReply->st_serviceinfo[Service_index].LabelString),(ast_ServiceInfo[Service_Station_index].st_ServiceLabel.au8_label),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) ;
					break;
				}
				else
				{
					Service_Station_index++;
				}	
			
			 }
		
		}
		else
		{
		
		}
	
	}
	
	
}

void Update_Stationlist_Into_LearnMem(void)
{
	Tu16 NoofStations = 0;
	Tu16 NoofStations1 = 0;
	Tu8 u8_NVM_ret = 1;
	Tu32 nvm_write = 0;
	Tbool b_Stored = FALSE;
	Learnmem_station_count = 0;

	for(NoofStations = 0; (NoofStations < DAB_APP_MAX_COMPONENTS && ast_LearnMem[NoofStations].u16_SId !=0); NoofStations++)
	{
		Learnmem_station_count++;
	}


	for(NoofStations = 0; (NoofStations < DAB_APP_MAX_COMPONENTS && ast_ComponentInfo[NoofStations].u32_SId!=0) ; NoofStations++)
	{
		b_Stored = FALSE;
		for(NoofStations1 = 0; (NoofStations1 < DAB_APP_MAX_COMPONENTS && ast_LearnMem[NoofStations1].u16_SId !=0);NoofStations1++)
		{
				if(ast_LearnMem[NoofStations1].u16_SId == ast_ComponentInfo[NoofStations].u32_SId)
				{
					Freq_index = FindFrequencyIndex(ast_ComponentInfo[NoofStations].st_BasicEnsInfo.u32_Frequency);
					if(Freq_index == ast_LearnMem[NoofStations1].u8_Freq_Index)
					{
						b_Stored = TRUE;
						break;
					}
					else
					{
					
					}
				}
		}		
			
		if(b_Stored == FALSE && Learnmem_station_count < DAB_APP_MAX_COMPONENTS)
		{
			ast_LearnMem[Learnmem_station_count].u16_SId 		= ast_ComponentInfo[NoofStations].u32_SId;
			ast_LearnMem[Learnmem_station_count].u16_EId 		= ast_ComponentInfo[NoofStations].st_BasicEnsInfo.u16_EId;
			ast_LearnMem[Learnmem_station_count].u16_SCIdI 		= ast_ComponentInfo[NoofStations].u16_SCIdI;
			ast_LearnMem[Learnmem_station_count].u8_ECC 		= ast_ComponentInfo[NoofStations].st_BasicEnsInfo.u8_ECC;
			ast_LearnMem[Learnmem_station_count].u8_Freq_Index 	= FindFrequencyIndex(ast_ComponentInfo[NoofStations].st_BasicEnsInfo.u32_Frequency);
			Learnmem_station_count++;
		}
	}

	u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LEARN_MEMORY, ast_LearnMem, (sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS) ,&nvm_write );
	if(	u8_NVM_ret ==1)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Learn Memory write failed ");
	}
	else
	{

	}
}

void DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{

	Tu8 Freqindex = 0;
	Tu8 Ensemindex = 0 ;
	Tu8 Serviceindex = 0;

	Tu8 Serviceindex1 = 0;
	Tu32 u32_Frequency = 0;
	Tbool b_Stored = FALSE;
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Checking learn memory for alternates");

	for(Serviceindex = 0;(Serviceindex < DAB_APP_MAX_COMPONENTS && ast_LearnMem[Serviceindex].u16_SId!=0 && DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId!=0 && DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency!=0); Serviceindex++)
	{
		if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == ast_LearnMem[Serviceindex].u16_SId) && (FindFrequencyIndex(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency) != ast_LearnMem[Serviceindex].u8_Freq_Index))
		{
			u32_Frequency = ((dab_app_freq_band_eu[ast_LearnMem[Serviceindex].u8_Freq_Index].u32_frequency) / 16);

			if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId == ast_LearnMem[Serviceindex].u16_EId || DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId == 0x00)
			{
				for(Freqindex = 0;(Freqindex < ALternateFreq && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Freqindex]!=0); Freqindex ++)
				{
					if(u32_Frequency == DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Freqindex])
					{
						b_Stored = TRUE;
						break;
					}
					else
					{
						b_Stored = FALSE;
					}
				}

				if(b_Stored == FALSE)
				{
					if(ALternateFreq >= MAX_ALT_FREQUENCY)
						ALternateFreq =0;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[ALternateFreq++] = (u32_Frequency);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Updated DAB alternative frequency for tuned SID %d",u32_Frequency *  16);
					DAB_Tuner_Ctrl_me->st_Linkingstatus.b_AlternateDABFreqAvailable = TRUE;
				}
			}	
			else
			{
				for(Ensemindex = 0;(Ensemindex < u8_Alt_Ensemble_ID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[Ensemindex].Freq!=0); Ensemindex ++)
				{
					if(u32_Frequency == DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[Ensemindex].Freq)
					{
						b_Stored = TRUE;
						break;
					}
					else
					{
						b_Stored = FALSE;
					}
				}
				if(b_Stored == FALSE)
				{
					if(u8_Alt_Ensemble_ID >= MAX_ALT_EID)
						u8_Alt_Ensemble_ID = 0;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_Alt_Ensemble_ID].Freq = (u32_Frequency);
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Updated DAB alternative ensmble frequency for tuned SID %d",u32_Frequency * 16);
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId [u8_Alt_Ensemble_ID++].EId = ast_LearnMem[Serviceindex].u16_EId; 
					DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidFreqAvailable = TRUE;
				}
			} 
	
		}
		else
		{
			for(Serviceindex1 = 0; ((Serviceindex1 < MAX_HARDLINK_SID) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Serviceindex1].SId != 0)) ; Serviceindex1++)
			{
				if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Serviceindex1].SId == ast_LearnMem[Serviceindex].u16_SId)
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Serviceindex1].EId_Data.Freq = ((dab_app_freq_band_eu[ast_LearnMem[Serviceindex].u8_Freq_Index].u32_frequency) / 16);
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Serviceindex1].EId_Data.EId = ast_LearnMem[Serviceindex].u16_EId;
					DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksFreqAvailable = TRUE;
					break;	
				}
				else
				{
				
				}	
			} 
		
		}
	}
}

void DAB_Tuner_Ctrl_Check_Same_PI_InLearnMem(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 u8_Stationindex =0; Tu8 u8_Samestationfound =0;

	memset(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo,0,(sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo)*MAX_FMDAB_SID));

	for(u8_Stationindex = 0;(u8_Stationindex < DAB_APP_MAX_COMPONENTS && ast_LearnMem[u8_Stationindex].u16_SId!=0 && u8_Samestationfound < MAX_FMDAB_SID);u8_Stationindex++)
	{
		if(DAB_Tuner_Ctrl_me->u16_FM_DAB_SID == ast_LearnMem[u8_Stationindex].u16_SId)
		{
			 DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[u8_Samestationfound].u16_SId 		= ast_LearnMem[u8_Stationindex].u16_SId;
			 DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[u8_Samestationfound].u16_EId 		= ast_LearnMem[u8_Stationindex].u16_EId;
			 DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[u8_Samestationfound].u8_ECC 		= ast_LearnMem[u8_Stationindex].u8_ECC;
			 DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[u8_Samestationfound].u16_SCIdI 	= ast_LearnMem[u8_Stationindex].u16_SCIdI;
			 DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[u8_Samestationfound].u32_Frequency = dab_app_freq_band_eu[ast_LearnMem[u8_Stationindex].u8_Freq_Index].u32_frequency;
			 u8_Samestationfound++;
		}
	}	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] FM_DAB Same PI station's found count is  %d", u8_Samestationfound);	
}

void UpdateStationListInfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)   //Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData,
{		
	Tbool b_stationTuned = TRUE;
	Tbool b_Eid_Present = FALSE;
	Tu8 labelIndex = 0;
	Tu8 ensembleIndx = 0;
	Tu8  serviceIndex = 0;				
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] Services count is %d ", Services);	
	if(DAB_Tuner_Ctrl_me->b_TunedtoNewEnsemble == TRUE)
	{	
		for(ensembleIndx = 0 ; ((ensembleIndx < ensembleIndex) && (ensembleIndx<DAB_APP_MAX_ENSEMBLES)); ensembleIndx++)
		{
			if((ast_EnsembleInfo[ensembleIndx].st_BasicEnsInfo.u32_Frequency == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency) 
				&& (ast_EnsembleInfo[ensembleIndx].st_BasicEnsInfo.u16_EId	 == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId))  //&& (ast_EnsembleInfo[ensembleIndx].st_BasicEnsInfo.u8_ECC    == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC)
			{
				/* Station found already exists in station list */
				b_stationTuned = FALSE;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] b_stationTuned is FALSE so it is same ensemble so Station found already exists in station list");
				if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label[0] !=0 && DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.au8_label[1]!=0)
					SYS_RADIO_MEMCPY(&(ast_EnsembleInfo[ensembleIndx].st_EnsembleLabel),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label),sizeof(Ts_Tuner_Ctrl_Label));
				//ensembleIndex++;
				//DAB_Tuner_Ctrl_me->e_TunedtoNewEnsemble = FALSE ;
				DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);//, DAB_Tuner_Ctrl_me->e_RequestCmd);
				memset(&(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo),0,sizeof(Ts_CurrentSidLinkingInfo));
				break;
			}
			else if(ast_EnsembleInfo[ensembleIndx].st_BasicEnsInfo.u16_EId	 == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId) //&& (ast_EnsembleInfo[ensembleIndx].st_BasicEnsInfo.u8_ECC    == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC)
			{
				b_Eid_Present = TRUE;																							
			}
			else
			{
				/* Station is not present in stationlist structure */																							
			}
		}

		/* Add Newly found station to stationlist structure*/
		if(b_stationTuned == TRUE)
		{
			ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u32_Frequency = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency;
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] b_stationTuned is TRUE so it is different ensemble so Add Newly found station to stationlist structure");
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Added newly tuned station to stationlist structure for frequency:%d", DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
			ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u16_EId	      = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId;
			ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u8_ECC		  = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC;
			SYS_RADIO_MEMCPY(&(ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel),&(DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label),sizeof(Ts_Tuner_Ctrl_Label));
			

			if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEEK || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER)
			{
				ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI;
			}
			else
			{
				if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Altfreqindex-1] * 16)) && (DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_SORTED))
				{
					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[Altfreqindex-1];
				}
				else if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex -1].Freq * 16)) && (DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_SORTED))
				{
					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex -1].Freq_RSSI;
				}
				else if((DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].EId_Data.Freq * 16)) && (DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_SORTED))
				{
					ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex-1].EId_Data.Freq_RSSI;
				}
				else
				{
					/* For MISRA */
				}
			}	
			
			
			if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet == 0x00)
			{
				ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
			}
			else if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet == 0x06)
			{
				ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
			}
			else if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.Ensemble_label.u8_CharSet == 0x0f)
			{
				ast_EnsembleInfo[ensembleIndex].st_EnsembleLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
			}
			else
			{
				/*Doing nothing */ 	
			}/* For MISRA C */

	 	    
			/* Updating Service List Structure */
			for(serviceIndex=0;((serviceIndex<DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) && (Services < DAB_APP_MAX_SERVICES));serviceIndex++,Services++)
			{

				if(Services < DAB_APP_MAX_SERVICES)
				{
					SYS_RADIO_MEMCPY(&(ast_ServiceInfo[Services].st_BasicEnsInfo),&(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo),sizeof(ast_ServiceInfo[Services].st_BasicEnsInfo));
					ast_ServiceInfo[Services].st_ServiceLabel.u16_ShortLabelFlags = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].ShortLabelFlags;
										
					if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].CharSet == 0x00)
					{
						ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
					}
					else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].CharSet == 0x06)
					{
						ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
					}
					else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].CharSet == 0x0f)
					{
						ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
					}
					else
					{
						/*Doing nothing */ 	
					}/* For MISRA C */

					for(labelIndex=0;labelIndex<16;labelIndex++)
					{
						ast_ServiceInfo[Services].st_ServiceLabel.au8_label[labelIndex] = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].LabelString[labelIndex];	    
					}
					ast_ServiceInfo[Services].u8_CA_Applied	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].CA_Applied;
					ast_ServiceInfo[Services].u8_PDFlag		= ((Tu8)0x00);
					ast_ServiceInfo[Services].u32_SId		= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].ProgServiceId; 
					ast_ServiceInfo[Services].u8_CAId		= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].CAId;
					ast_ServiceInfo[Services].u8_ptyCode	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].StatPTy;
					ast_ServiceInfo[Services].u8_dynPtyCode	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].DynPTy;
				}
	

				for(DAB_Tuner_Ctrl_me->CompIndex=0; ((DAB_Tuner_Ctrl_me->CompIndex < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.NoOfComponents) && (Components < DAB_APP_MAX_COMPONENTS));DAB_Tuner_Ctrl_me->CompIndex++,Components++)
				{
					if(Components < DAB_APP_MAX_COMPONENTS)
					{
						SYS_RADIO_MEMCPY(&(ast_ComponentInfo[Components].st_BasicEnsInfo),&(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo),sizeof(ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo));
						ast_ComponentInfo[Components].u16_SCIdI        =	  DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].InternalCompId;
						ast_ComponentInfo[Components].u8_Primary       =      DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].Primary ;
						ast_ComponentInfo[Components].u8_CA_Applied	   =	  DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CA_Applied;
						ast_ComponentInfo[Components].u32_SId		   =	  ast_ServiceInfo[Services].u32_SId;
						SYS_RADIO_MEMCPY((ast_ComponentInfo[Components].st_compLabel.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						ast_ComponentInfo[Components].st_compLabel.u16_ShortLabelFlags   =    DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].ShortLabelFlags;

						if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x00)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x06)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x0f)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
						}
						else
						{
						/*Doing nothing */ 	
						}/* For MISRA C */

						ast_ComponentInfo[Components].u8_Language				=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].Language;
						ast_ComponentInfo[Components].u8_TransportMech			=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].TransportMech;
						ast_ComponentInfo[Components].u8_ComponentType			=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].ComponentType;
						ast_ComponentInfo[Components].u16_UAType				=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[serviceIndex].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].UAType;
					}
				}
			}
			if((DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEEK || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_SEL_SER) && (b_Eid_Present != TRUE))
			{
				DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);//, DAB_Tuner_Ctrl_me->e_RequestCmd);
			}
			else if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE )
			{
				DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
			}	
			else
			{
				
			}

			ensembleIndex++;
		}
		else
		{

		}/*For MISRA*/
	}
	else
	{

	}/*For MISRA*/
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] Services count is %d ", Services);	
}

void DAB_Tuner_Ctrl_Update_LearnMem_After_Tune(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	Tu8 u8_Stationindex =0; Tbool b_Stored = FALSE; Tu8 u8_Serviceindex = 0; Tu8 u8_NVM_ret =1;

	while(u8_Stationindex < DAB_APP_MAX_COMPONENTS && ast_LearnMem[u8_Stationindex].u16_SId != 0)
	{
		if(ast_LearnMem[u8_Stationindex].u8_Freq_Index == FindFrequencyIndex(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency))
		{
			b_Stored = TRUE;
			break;	
		}	
	 	u8_Stationindex++;	
	}

	if(b_Stored == FALSE)
	{
		for(u8_Serviceindex = 0; u8_Serviceindex < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices && u8_Stationindex < DAB_APP_MAX_COMPONENTS; u8_Serviceindex ++)
		{
			ast_LearnMem[u8_Stationindex].u16_SId 		= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_Serviceindex].ProgServiceId;
			ast_LearnMem[u8_Stationindex].u16_SCIdI 	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[0].st_compInfo.Component[0].InternalCompId;
			ast_LearnMem[u8_Stationindex].u16_EId 		= DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId;
			ast_LearnMem[u8_Stationindex].u8_ECC 		= DAB_Tuner_Ctrl_me->st_currentEnsembleData.u8_ECC;
			ast_LearnMem[u8_Stationindex].u8_Freq_Index	= FindFrequencyIndex(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
			u8_Stationindex++;
		}	
	}

	u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LEARN_MEMORY, ast_LearnMem, (sizeof(Ts_Tuner_Ctrl_Tunableinfo)*DAB_APP_MAX_COMPONENTS) ,&DAB_Tuner_Ctrl_me->nvm_write );
	if(	u8_NVM_ret ==1)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Learn Memory write failed ");
	}
	else
	{
	
	}
	
}

/*================================================================================================*/
/*  void DAB_Tuner_Ctrl_hsm_inst_SystemAbnormal(Tu16 dest_cid, Tu16 comp_msgid)                            */
/*================================================================================================*/
void DAB_Tuner_Ctrl_hsm_inst_SystemAbnormal(Tu16 dest_cid, Tu16 comp_msgid)
{
  Ts_Sys_Msg* p_msg = NULL;
  p_msg = SYS_MSG_HANDLE_Call(dest_cid,comp_msgid);
  SYS_SEND_MSG(p_msg);

}

void DAB_Tuner_Ctrl_Check_Service_Present_In_Stl(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 Search_index = 0;
	Tbool b_stored = FALSE;
	Tu8 u8_ensemble_index = 0;
	Tu8 u8_service_index = 0;
	Tu8 labelIndex = 0;
	for(Search_index = 0; Search_index < DAB_APP_MAX_SERVICES && ast_ServiceInfo[Search_index].u32_SId !=0;Search_index++)
	{
		if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == ast_ServiceInfo[Search_index].u32_SId)
		{
			b_stored = TRUE;
			if(ast_ServiceInfo[Search_index].st_ServiceLabel.au8_label[0] == 0 && ast_ServiceInfo[Search_index].st_ServiceLabel.au8_label[1] == 0)
			{
				if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label[0] == 0 && DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label[1] == 0)
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Label is not there in Prog list also");
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Label Updated to Current ensmeble data");
					SYS_RADIO_MEMCPY((ast_ServiceInfo[Search_index].st_ServiceLabel.au8_label),(DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
					DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);
				}	
				break;	
			}
			
		}
	}
	
	if(b_stored == FALSE)
	{
		for(u8_service_index = 0; u8_service_index < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices; u8_service_index++)
		{
			if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].ProgServiceId)
			{
				break;
			}
		}	
			
		for(u8_ensemble_index = 0; u8_ensemble_index < DAB_APP_MAX_ENSEMBLES && ast_EnsembleInfo[u8_ensemble_index].st_BasicEnsInfo.u32_Frequency!=0 && Search_index < DAB_APP_MAX_SERVICES; u8_ensemble_index++)
		{
			if(ast_EnsembleInfo[u8_ensemble_index].st_BasicEnsInfo.u32_Frequency == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency)
			{
				SYS_RADIO_MEMCPY(&(ast_ServiceInfo[Search_index].st_BasicEnsInfo),&(ast_EnsembleInfo[u8_ensemble_index].st_BasicEnsInfo),sizeof(ast_ServiceInfo[Search_index].st_BasicEnsInfo));
				ast_ServiceInfo[Search_index].st_ServiceLabel.u16_ShortLabelFlags = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].ShortLabelFlags;
				if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].CharSet == 0x00)
				{
					ast_ServiceInfo[Search_index].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
				}
				else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].CharSet == 0x06)
				{
					ast_ServiceInfo[Search_index].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
				}
				else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].CharSet == 0x0f)
				{
					ast_ServiceInfo[Search_index].st_ServiceLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
				}
				else
				{
					/*Doing nothing */ 	
				}/* For MISRA C */

				for(labelIndex=0;labelIndex<16;labelIndex++)
				{
					ast_ServiceInfo[Search_index].st_ServiceLabel.au8_label[labelIndex] = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].LabelString[labelIndex];	    
				}
				ast_ServiceInfo[Search_index].u8_CA_Applied	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].CA_Applied;
				ast_ServiceInfo[Search_index].u8_PDFlag		= ((Tu8)0x00);
				ast_ServiceInfo[Search_index].u32_SId		= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].ProgServiceId; 
				ast_ServiceInfo[Search_index].u8_CAId		= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].CAId;
				ast_ServiceInfo[Search_index].u8_ptyCode	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].StatPTy;
				ast_ServiceInfo[Search_index].u8_dynPtyCode	= DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].DynPTy;

				for(DAB_Tuner_Ctrl_me->CompIndex=0; DAB_Tuner_Ctrl_me->CompIndex < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.NoOfComponents;DAB_Tuner_Ctrl_me->CompIndex++,Components++)
				{
					if(Components < DAB_APP_MAX_COMPONENTS)
					{
						SYS_RADIO_MEMCPY(&(ast_ComponentInfo[Components].st_BasicEnsInfo),&(ast_EnsembleInfo[u8_ensemble_index].st_BasicEnsInfo),sizeof(ast_EnsembleInfo[u8_ensemble_index].st_BasicEnsInfo));
						ast_ComponentInfo[Components].u16_SCIdI        =	  DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].InternalCompId;
						ast_ComponentInfo[Components].u8_Primary       =      DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].Primary ;
						ast_ComponentInfo[Components].u8_CA_Applied	   =	  DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CA_Applied;
						ast_ComponentInfo[Components].u32_SId		   =	  ast_ServiceInfo[Services].u32_SId;
						SYS_RADIO_MEMCPY((ast_ComponentInfo[Components].st_compLabel.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						ast_ComponentInfo[Components].st_compLabel.u16_ShortLabelFlags   =    DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].ShortLabelFlags;

						if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x00)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x06)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].CharSet == 0x0f)
						{
							ast_ComponentInfo[Components].st_compLabel.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
						}
						else
						{
						/*Doing nothing */ 	
						}/* For MISRA C */

						ast_ComponentInfo[Components].u8_Language				=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].Language;
						ast_ComponentInfo[Components].u8_TransportMech			=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].TransportMech;
						ast_ComponentInfo[Components].u8_ComponentType			=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].ComponentType;
						ast_ComponentInfo[Components].u16_UAType				=	 DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[u8_service_index].st_compInfo.Component[DAB_Tuner_Ctrl_me->CompIndex].UAType;
						Services++;
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] New service added to station list");
						DAB_Tuner_Ctrl_Notify_STLUpdated(DAB_TUNER_APP);	
					}	
				}
			}
 		}
 	}
}  	 

Tbool DAB_Tuner_Ctrl_Check_Service_Available(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	Tu8 Service_Index = 0;
	Tbool b_resultcode = TRUE;
	
	for(Service_Index = 0; Service_Index < 	DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices; Service_Index++)
	{	
		if(DAB_Tuner_Ctrl_me->requestedinfo.u32_SId == DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Service_Index].ProgServiceId)
		{
			b_resultcode = TRUE;
			break;
		}
		else
		{
			b_resultcode = FALSE;
		}
	}
	return b_resultcode;	
}

void DAB_Tuner_Ctrl_Update_EnsembleLabel(Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData)
{
	Tu8 u8_EnsembleIndex = 0;
	if(pst_currentEnsembleData->Ensemble_label.au8_label[0] == 0 && pst_currentEnsembleData->Ensemble_label.au8_label[1] == 0)
	{
		for(u8_EnsembleIndex = 0;((u8_EnsembleIndex < DAB_APP_MAX_ENSEMBLES) && (ast_EnsembleInfo[u8_EnsembleIndex].st_BasicEnsInfo.u16_EId != 0));u8_EnsembleIndex++)
		{
			if(pst_currentEnsembleData->u16_EId == ast_EnsembleInfo[u8_EnsembleIndex].st_BasicEnsInfo.u16_EId)
			{
				SYS_RADIO_MEMCPY((pst_currentEnsembleData->Ensemble_label.au8_label),(ast_EnsembleInfo[u8_EnsembleIndex].st_EnsembleLabel.au8_label),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) ;
				break;
			}
			else
			{
			
			}
		}
	}
}

/*=============================================================================
    end of file
=============================================================================*/

