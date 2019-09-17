/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Linking.c 											                                *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains function definition Linking related API definitions	            *
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_inst_hsm.h"
#include "DAB_Tuner_Ctrl_Response.h"
#include "DAB_Tuner_Ctrl_app.h"
#include "sys_nvm.h"
#include "lib_bitmanip.h"
/*#include "DAB_Tuner_Ctrl_messages.h"
#include "DAB_Tuner_Ctrl_Notify.h"
#include "DAB_Tuner_Ctrl_Response.h"
#include "DAB_Tuner_Ctrl_inst_hsm.h"
#include "hsm_api.h"
#include "cfg_types.h"
#include "sys_task.h"
#include <string.h>
*/
/*--------------------------------------------------------------------------------------------------
    variables (extern)
--------------------------------------------------------------------------------------------------*/

Tbool b_delayvalue;
extern Tu8 Altfreqindex;
extern Tu8	AltEnsembleindex;
extern Tu8	Hardlinkindex;
extern Tu8	Same_SID_Search;
Tu8 ALternateFreq;
extern Ts_dab_tuner_ctrl_inst_timer st_TimerId;
Tu8	u8_Alt_Ensemble_ID = 0;
extern Tbool b_Hardlinksused;

Tbool b_Tune_to_Orginal_freq_Check = FALSE;
Tbool b_Changein_AFlist_NVM_Write = FALSE;

/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/
void FindAltFreq(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 u8_NVM_ret = 0;
	Tu8 u8_FreqSearch_Index = 0 ; //u8_AltFreq_Index = 0;
	Tu8 u8_Ens_Index = 0; Tu8 No_ofindex = 0; Tbool b_Changein_AFlist_ENG_Mode = FALSE;
	DAB_Tuner_Ctrl_me->b_Frequency_Found = FALSE;


	for(u8_FreqSearch_Index = 0;(u8_FreqSearch_Index < DAB_MAX_NUM_FREQ_INFO && ALternateFreq < MAX_ALT_FREQUENCY && DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency != 0) ; u8_FreqSearch_Index++)
	{
		if(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier == DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].EId && DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency != (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency /16))
		{
				DAB_Tuner_Ctrl_me->Frequency = (DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency) ;
				for(No_ofindex = 0; No_ofindex < ALternateFreq ; No_ofindex++)
				{
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[No_ofindex] == DAB_Tuner_Ctrl_me->Frequency)
					{
						DAB_Tuner_Ctrl_me->b_Frequency_Found = TRUE;
						break;
					}	
					else
						DAB_Tuner_Ctrl_me->b_Frequency_Found = FALSE;
						
				}
				if(	DAB_Tuner_Ctrl_me->b_Frequency_Found == FALSE)
				{	 
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[ALternateFreq] = DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency ;
					b_Changein_AFlist_ENG_Mode = TRUE;
					b_Changein_AFlist_NVM_Write = TRUE;
					ALternateFreq++;				
					DAB_Tuner_Ctrl_me->st_Linkingstatus.b_AlternateDABFreqAvailable = TRUE;
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,DAB_ALTERNATE_FREQUENCY);
					if(ALternateFreq > MAX_ALT_FREQUENCY)
					{
						ALternateFreq = 0;
					}    /*This is not in new dab-dab code*/
				}
		}
	}
	
	
/* After finding out Hardlink SIDs for current SID then check for frquency */	

/*	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksEIDAvailable == TRUE)
	{
		for(u8_SID_index = 0; u8_SID_index < MAX_HARDLINK_SID ; u8_SID_index ++)
		{
			for(u8_AltFreq_Index = 0; u8_AltFreq_Index < DAB_MAX_NUM_FREQ_INFO ; u8_AltFreq_Index++)
			{
				if((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_SID_index].EId_Data.EId == DAB_Tuner_Ctrl_me->freqInfo[u8_AltFreq_Index].EId) && (DAB_Tuner_Ctrl_me->freqInfo[u8_AltFreq_Index].frequency != 0))
				{	
					DAB_Tuner_Ctrl_me->Frequency = (DAB_Tuner_Ctrl_me->freqInfo[u8_AltFreq_Index].frequency) ;	
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_SID_index].EId_Data.Freq != DAB_Tuner_Ctrl_me->Frequency)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_SID_index].EId_Data.Freq = DAB_Tuner_Ctrl_me->freqInfo[u8_AltFreq_Index].frequency;
						DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksFreqAvailable = TRUE;
						break;					
					}
					else
					{
						break;	
					}
				}
			}
		}
	}*/
					/*	for(No_ofindex1 = 0; No_ofindex1 < MAX_ALT_FREQUENCY ; No_ofindex1++)
						{
							if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_SID_index].EId_Data[u8_FreqSearch_Index].Freq[No_ofindex1] == DAB_Tuner_Ctrl_me->Frequency)
							{
							 	DAB_Tuner_Ctrl_me->b_Frequency_Found = TRUE;
								break;
							}	
							else
							 	DAB_Tuner_Ctrl_me->b_Frequency_Found = FALSE;
						
						}
						if(	DAB_Tuner_Ctrl_me->b_Frequency_Found == FALSE)
						{	
							for(No_ofindex1 = 0; No_ofindex1 < MAX_ALT_FREQUENCY ; No_ofindex1++)
							{
								if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_SID_index].EId_Data[u8_FreqSearch_Index].Freq[No_ofindex1] == 0x00000000)
								{
									DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_SID_index].EId_Data[u8_FreqSearch_Index].Freq[No_ofindex1] = DAB_Tuner_Ctrl_me->freqInfo[u8_AltFreq_Index].frequency;
									DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksFreqAvailable = TRUE;
									break;					
								}	
							}						
						}
					}
				}*/
					

/* AFTER FINDING OUT SAME SID IS TRANSMITTING IN AN OTHER ENSMEBLE THEN FIND OUT THE FREQUENCY FOR THAT OTHER ENSMEBLE	*/

	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidAvialable == TRUE)
	{
		for(u8_Ens_Index = 0; u8_Ens_Index < MAX_ALT_EID; u8_Ens_Index ++)
		{
			for(u8_FreqSearch_Index = 0; (u8_FreqSearch_Index < DAB_MAX_NUM_FREQ_INFO && DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency != 0); u8_FreqSearch_Index++)
			{
				if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_Ens_Index].EId == DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].EId )
				{	
					DAB_Tuner_Ctrl_me->Frequency = (Tu16)(DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency);	
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_Ens_Index].Freq != DAB_Tuner_Ctrl_me->Frequency && DAB_Tuner_Ctrl_me->Frequency != (DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency /16))
					{
						if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_Ens_Index].Freq == 0)
						{
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_Ens_Index].Freq = DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency;
							DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidFreqAvailable = TRUE;
						}
						else
						{
							for(No_ofindex = 0; No_ofindex < MAX_ALT_EID ; No_ofindex ++)
							{
								if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].Freq == DAB_Tuner_Ctrl_me->Frequency)
								{
									DAB_Tuner_Ctrl_me->b_SameSidFreq_Found = TRUE;
									break;
								}	
								else
									DAB_Tuner_Ctrl_me->b_SameSidFreq_Found = FALSE;
							}
							if(DAB_Tuner_Ctrl_me->b_SameSidFreq_Found == FALSE)
							{
								for(No_ofindex = 0; No_ofindex < MAX_ALT_EID ; No_ofindex ++)
								{
									if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].Freq == 0x00000000)
									{
										DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].EId = DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].EId;
										DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].Freq = DAB_Tuner_Ctrl_me->freqInfo[u8_FreqSearch_Index].frequency;
										DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidFreqAvailable = TRUE;	
										b_Changein_AFlist_NVM_Write = TRUE;
										RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Alternate EID freq updated for EID is %d %d",DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].EId,DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].Freq);
										DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,DAB_ALTERNATE_ENSEMBLE);
										break;
									}
								}	
							}	
						}
					}	
					else
					{
						
					}
				}
			}
			
		}
	}
	if(b_Changein_AFlist_ENG_Mode == TRUE)
	{
		if((DAB_Tuner_Ctrl_me->u8_ENGMODEStatus == DAB_TUNER_CTRL_MODE_ON) && (DAB_Tuner_Ctrl_me->st_Linkingstatus.b_AlternateDABFreqAvailable == TRUE))
		{
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_AFList.Alt_Freq),(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq),sizeof(DAB_Tuner_Ctrl_me->st_AFList.Alt_Freq));
			DAB_Tuner_Ctrl_Response_AFList(DAB_Tuner_Ctrl_me->st_AFList);
		}
		else
		{
			/*MISRA*/
		}
	}
	if(b_Changein_AFlist_NVM_Write == TRUE)
	{
		b_Changein_AFlist_NVM_Write = FALSE;
		u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LINKING, &(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), sizeof(Ts_CurrentSidLinkingInfo),&DAB_Tuner_Ctrl_me->nvm_write );
		/* NVM Write failed */
		if(u8_NVM_ret == 1)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] NVM write failed ");
		}
		else
		{
			/* for MISRA*/
		}
	}
	else
	{
		/* for MISRA */
	}
}



void FindAltEId(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 u8_EIdSearch_Index = 0 ; 
	Tu8 u8_NoofEids = 0; Tbool b_Changein_AFlist_ENG_Mode = FALSE;  Tu8 No_ofindex = 0;Tu8 u8_NVM_ret = 0; // Tu8 u8_AltEIdList_index = 0;
	
	for(u8_EIdSearch_Index = 0; u8_EIdSearch_Index < DAB_TUNER_CTRL_MAX_NUM_OE_SERVICES ; u8_EIdSearch_Index++)
	{
		if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].u32_SId)
		{
			for(u8_NoofEids = 0; ((u8_NoofEids <DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].u8_numEId) && (u8_NoofEids < MAX_ALT_EID)); u8_NoofEids++)
			{
				DAB_Tuner_Ctrl_me->Eid = DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].au16_EId[u8_NoofEids];
				for(No_ofindex = 0; No_ofindex < MAX_ALT_EID ; No_ofindex ++)
				{
					if(DAB_Tuner_Ctrl_me->Eid == DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[No_ofindex].EId)
					{
						DAB_Tuner_Ctrl_me->b_SameSidFound = TRUE;
						break;
					}	
					else
					{
						DAB_Tuner_Ctrl_me->b_SameSidFound = FALSE;
					}
					
				}
				if(DAB_Tuner_Ctrl_me->b_SameSidFound == FALSE && DAB_Tuner_Ctrl_me->Eid != DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_EId)
				{
					if(u8_Alt_Ensemble_ID > MAX_ALT_EID)
					{
						u8_Alt_Ensemble_ID = 0;
					}
					else
					{
						/* Do Nothing*/
					}
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_Alt_Ensemble_ID++].EId = DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].au16_EId[u8_NoofEids];
					DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidAvialable = TRUE;
					b_Changein_AFlist_ENG_Mode = TRUE;
					b_Changein_AFlist_NVM_Write = TRUE;
//					DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,DAB_ALTERNATE_ENSEMBLE);
				}
			}
		
		}
	
	}
	
	

/*	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksAvailable == TRUE)
	{
		while(u8_AltSId_Index < MAX_HARDLINK_SID)
		{	
			for(u8_EIdSearch_Index = 0; u8_EIdSearch_Index < DAB_TUNER_CTRL_MAX_NUM_OE_SERVICES ;u8_EIdSearch_Index++)
			{
				if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_AltSId_Index].SId == DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].u32_SId)
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_AltSId_Index].EId_Data.EId = DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].au16_EId[0];
					break;
					for(u8_NoofEids = 0; ((u8_NoofEids <DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].u8_numEId ) && (u8_NoofEids < MAX_ALT_EID)); u8_NoofEids++)
					{
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_AltSId_Index].EId_Data[u8_NoofEids].EId = DAB_Tuner_Ctrl_me->oeServices[u8_EIdSearch_Index].au16_EId[u8_NoofEids];
						DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksEIDAvailable = TRUE;
					}
					
				}
			}
			u8_AltSId_Index ++;	
			u8_AltEId_Search++;
		}
	}*/
	if(b_Changein_AFlist_ENG_Mode == TRUE)
	{
		if((DAB_Tuner_Ctrl_me->u8_ENGMODEStatus == DAB_TUNER_CTRL_MODE_ON) && (DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidAvialable == TRUE))
		{
			for(u8_NoofEids = 0; u8_NoofEids < MAX_ALT_EID ; u8_NoofEids++)
			{
				DAB_Tuner_Ctrl_me->st_AFList.Alt_Ens[u8_NoofEids] = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[u8_NoofEids].EId;
			}	
			DAB_Tuner_Ctrl_Response_AFList(DAB_Tuner_Ctrl_me->st_AFList);
		}
		else
		{
			/*MISRA*/
		}
	}
	if(b_Changein_AFlist_NVM_Write == TRUE)
	{
		b_Changein_AFlist_NVM_Write = FALSE;
		u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LINKING, &(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), sizeof(Ts_CurrentSidLinkingInfo),&DAB_Tuner_Ctrl_me->nvm_write );
		/* NVM Write failed */
		if(u8_NVM_ret == 1)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] NVM write failed ");
		}
		else
		{
			/* for MISRA */
		}
	}
	else
	{
			/* for MISRA*/	
	}
}

Tbool Check_Hardlink_Repeat(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	Tu8 Hardlink_index = 0; Tbool b_Retval = TRUE;

	for(Hardlink_index = 0; Hardlink_index < MAX_HARDLINK_PI ; Hardlink_index++)
	{
		if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[Hardlink_index] == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId)
		{
			b_Retval=  TRUE;
			break;	
		}
		else 
			b_Retval=  FALSE;	
		
	}
		
	return b_Retval;	
	
}
void FindHardlinks(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 u8_SIdSearch_Index = 0 ,u8_Hardlink_Index = 0, u8_Part = 0, u8_Hardlinks = 0 ; Tbool b_Changein_AFlist_ENG_Mode = FALSE;  Tu8 No_ofindex = 0; Tu8 u8_NVM_ret = 0; // Tu8 u8_AltSIdList_index = 0;
	
	Tbool b_Result = TRUE;
	for(u8_SIdSearch_Index = 0 ; u8_SIdSearch_Index < MAX_LINKAGE_SETS ; u8_SIdSearch_Index++)
	{
		for(u8_Part = 0; u8_Part < DAB_MAX_LINKAGE_PARTS ; u8_Part++)
		{
			if(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part] != 0)
			{
				if(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Idlq[u8_Part] == LINKING_DAB_SIDS)
				{
					if((DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[u8_Part][0] == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId) || (DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[0][0] == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId))
					{
						for(u8_Hardlink_Index = 0, u8_Hardlinks = 1 ; ((u8_Hardlink_Index < DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part]-1) && (u8_Hardlink_Index < MAX_HARDLINK_SID)) ; u8_Hardlink_Index++ , u8_Hardlinks++)
						{
							DAB_Tuner_Ctrl_me->Sid = DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[u8_Part][u8_Hardlinks] ;
							for(No_ofindex = 0; No_ofindex < DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part]-1 ; No_ofindex++)
							{
								if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[No_ofindex].SId == DAB_Tuner_Ctrl_me->Sid)
								{
								 	DAB_Tuner_Ctrl_me->b_Sid_Found = TRUE;
									break;
								}	
								else
								{
									DAB_Tuner_Ctrl_me->b_Sid_Found = FALSE;
								}
					
							}
							if(	DAB_Tuner_Ctrl_me->b_Sid_Found == FALSE)
							{	 
								DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_Hardlink_Index].SId = DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[u8_Part][u8_Hardlinks] ;
								DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_Hardlink_Index].lsn	=	DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Lsn;	
								DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_Hardlink_Index].LA	=	DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Activelink;	
								DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksAvailable = TRUE;
								b_Changein_AFlist_ENG_Mode = TRUE;
								b_Changein_AFlist_NVM_Write = TRUE;
								DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(DAB_Tuner_Ctrl_me);
							}
						}
						
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				}
				else if(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Idlq[u8_Part] == LINKING_FM_PIS)
				{
					if(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[u8_Part][0] == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId)
					{
						for(u8_Hardlink_Index = 0, u8_Hardlinks = 1 ; ((u8_Hardlink_Index < DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part]-1 )&& ( u8_Hardlink_Index < MAX_HARDLINK_PI)) ; u8_Hardlink_Index++ , u8_Hardlinks++)
						{
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] =(Tu16)( DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[u8_Part][u8_Hardlinks] );
							DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes  = (Tu8)(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part]) ;
							DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable = TRUE;
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA = DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Activelink;
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LSN = DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Lsn;
						} 
					  if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_Same_SID_FMHardlinkAvailable == TRUE)
					  {
						  for(u8_Hardlink_Index = 0; ((u8_Hardlink_Index < DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes) && (u8_Hardlink_Index < MAX_HARDLINK_PI)); u8_Hardlink_Index++)
						  {
							  if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] == 0x00)
							  {
								b_Result = Check_Hardlink_Repeat(DAB_Tuner_Ctrl_me);
								if(b_Result == FALSE)
									DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
								else
									DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes  = (Tu8)(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part] - 1) ;	
								break;	  
							  }
							  else
							  {
								  
							  }
							  
						  }
					  }
					  else
					  	DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes  = (Tu8)(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part] - 1) ;	
					}
				
					else if((DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[0][0] == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId)) 
					{
						for(u8_Hardlink_Index = 0, u8_Hardlinks = 0 ; ((u8_Hardlink_Index < DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part]-1 )&& ( u8_Hardlink_Index < MAX_HARDLINK_PI)) ; u8_Hardlink_Index++ , u8_Hardlinks++)
						{
							b_Result = Check_Hardlink_Repeat(DAB_Tuner_Ctrl_me);
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] =(Tu16)( DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Sid[u8_Part][u8_Hardlinks] );
							DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes  = (Tu8)(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part] + 1) ;
							DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable = TRUE;
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA = DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Activelink;
							DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LSN = DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Lsn;
						} 
						if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_Same_SID_FMHardlinkAvailable == TRUE)
					  	{
							for(u8_Hardlink_Index = 0; ((u8_Hardlink_Index < DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes) && (u8_Hardlink_Index < MAX_HARDLINK_PI)); u8_Hardlink_Index++)
							{
								if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] == 0x00)
								{
									b_Result = Check_Hardlink_Repeat(DAB_Tuner_Ctrl_me);
									if(b_Result == FALSE)
										DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId;
									else
										DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes  = (Tu8)(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part] - 1) ;	
									break;	  
								}
								else
								{

								}
  
							}
						}
						else
							DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes  = (Tu8)(DAB_Tuner_Ctrl_me->hardlinkinfo[u8_SIdSearch_Index].Noofids[u8_Part]) ;
						
					}
					else
					{
						/* Doing nothing */
					}/* For MISRA C */
				
				}
				else
				{
					/* Doing nothing */
				}/* For MISRA C */
			}
			else
			{
			//break ;
			}
		}
	}
	if(b_Changein_AFlist_ENG_Mode == TRUE)
	{
		if(DAB_Tuner_Ctrl_me->u8_ENGMODEStatus == DAB_TUNER_CTRL_MODE_ON)
		{
			for(No_ofindex = 0; No_ofindex < MAX_HARDLINK_SID ; No_ofindex++)
			{
				DAB_Tuner_Ctrl_me->st_AFList.Hardlink_Sid[No_ofindex] = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[No_ofindex].SId;
			}	
			DAB_Tuner_Ctrl_Response_AFList(DAB_Tuner_Ctrl_me->st_AFList);
		}
		else
		{
			/*MISRA*/
		}
	}
	if(b_Changein_AFlist_NVM_Write == TRUE)
	{
		b_Changein_AFlist_NVM_Write = FALSE;
		u8_NVM_ret = SYS_NVM_WRITE(NVM_ID_DAB_TC_LINKING, &(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo), sizeof(Ts_CurrentSidLinkingInfo),&DAB_Tuner_Ctrl_me->nvm_write );
		/* NVM Write failed */
		if(u8_NVM_ret == 1)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] NVM write failed ");
		}
		else
		{
			/* for MISRA*/
		}
	}
	else
	{
		/* For MISRA */
	}

}



void Find_Hardlink_PI(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{

	Tu8 u8_Hardlink_Index = 0; Tbool b_Changein_AFlist_ENG_Mode = FALSE; Tu8 No_ofindex = 0;
	
	Tu8 u8_Same_Sidindex = 0;
	
	for(u8_Same_Sidindex = 0;((u8_Same_Sidindex < MAX_HARDLINK_PI)); u8_Same_Sidindex++)
	{
		if(DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[u8_Same_Sidindex].PI == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId)
		{
			DAB_Tuner_Ctrl_me->st_Linkingstatus.b_Same_SID_FMHardlinkAvailable = TRUE;
			break;
		}
		else
			DAB_Tuner_Ctrl_me->st_Linkingstatus.b_Same_SID_FMHardlinkAvailable = FALSE;
	}
	
	
	if((DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable == TRUE)&&(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_Same_SID_FMHardlinkAvailable == TRUE))
	{
		for(u8_Hardlink_Index = 0;((u8_Hardlink_Index < MAX_HARDLINK_PI) && (u8_Same_Sidindex < MAX_HARDLINK_PI)); u8_Hardlink_Index++)
		{
			if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] == DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[u8_Same_Sidindex].PI)
			{
				break;
			}
				
			else if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[u8_Hardlink_Index] = DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[u8_Same_Sidindex].PI;		
				DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA = 1;
				DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes = u8_Hardlink_Index + 1;
				b_Changein_AFlist_ENG_Mode = TRUE;
				break;
			}
			else
			{
				
			}
		
		}
	}
	else if((DAB_Tuner_Ctrl_me->st_Linkingstatus.b_Same_SID_FMHardlinkAvailable == TRUE)&&(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable != TRUE))
	{
		if(u8_Same_Sidindex < MAX_HARDLINK_PI)
		{
			DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[0] = DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[u8_Same_Sidindex].PI; 
			DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable = TRUE ;
			DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA = 1;
			DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes = 1;
			b_Changein_AFlist_ENG_Mode = TRUE;
		}	
	}
	else
	{
		/*for MISRA*/		
	}
	
	if(b_Changein_AFlist_ENG_Mode == TRUE)
	{
		if(DAB_Tuner_Ctrl_me->u8_ENGMODEStatus == DAB_TUNER_CTRL_MODE_ON)
		{
			for(No_ofindex = 0; No_ofindex < MAX_HARDLINK_SID ; No_ofindex++)
			{
				DAB_Tuner_Ctrl_me->st_AFList.Hardlink_Sid[No_ofindex] = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[No_ofindex].SId;
			}	
			DAB_Tuner_Ctrl_Response_AFList(DAB_Tuner_Ctrl_me->st_AFList);
		}
		else
		{
			/*MISRA*/
		}
	}
	else
	{
		
	}
}




void Update_CurSId_Hardlink_DataBase(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	if((DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available))
			FindHardlinks(DAB_Tuner_Ctrl_me);
			
	if((DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_available))
			FindAltFreq(DAB_Tuner_Ctrl_me);

	if((DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available))
			Find_Hardlink_PI(DAB_Tuner_Ctrl_me);

	if((DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG024_available))
			FindAltEId(DAB_Tuner_Ctrl_me);
			
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksAvailable == TRUE)
			FindAltEId(DAB_Tuner_Ctrl_me);
	
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksEIDAvailable == TRUE)
			FindAltFreq(DAB_Tuner_Ctrl_me);
		
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidAvialable == TRUE)
			FindAltFreq(DAB_Tuner_Ctrl_me);
			
}


void Notify_Hardlinks_To_FM(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	if((DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable == TRUE) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
	{
		if((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.b_Hardlinks_sent == TRUE) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA == 0))
		{
			memset(&(DAB_Tuner_Ctrl_me->st_PI_data_available),0,sizeof(Ts_PI_Data));
			DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.b_Hardlinks_sent = FALSE;
			DAB_Tuner_Ctrl_Notify_Hardlinks_Status(RADIO_FRMWK_DAB_FM_LINKING_CANCELLED);
		}
		else
		{
			for(DAB_Tuner_Ctrl_me->Index=0;((DAB_Tuner_Ctrl_me->Index <DAB_Tuner_Ctrl_me->st_PI_data_available.NoOfPICodes)) ;DAB_Tuner_Ctrl_me->Index++)
			{
			  if((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks[DAB_Tuner_Ctrl_me->Index] != DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI[DAB_Tuner_Ctrl_me->Index]))	
			  {
				memcpy((DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks),(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_PI),sizeof(DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks));
				if(DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)   /*If FM is NORMAL onlywe will send Hardlinks to FM.*/
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Notifying PI list");
					DAB_Tuner_Ctrl_Notify_PICodeList((DAB_Tuner_Ctrl_me->st_PI_data_available),DAB_FM_LINKING_MIN_THRESHOULD,DAB_FM_LINKING_MAX_THRESHOULD,DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId,CHECK_HARDLINK);	
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.b_Hardlinks_sent = TRUE;
					DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received = FALSE;
					DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_HARDLINKS);
				}
				break;
			  }	
			}
		}	
	}
	
}


void Check_Hardlinks_For_Tuned_SID(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available = TRUE;
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_available = TRUE;
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG024_available = TRUE;
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available = TRUE;
	Update_CurSId_Hardlink_DataBase(DAB_Tuner_Ctrl_me);
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_TC] Notifying Hardlinks to FM from Listen Handler After tuned ");
	Notify_Hardlinks_To_FM(DAB_Tuner_Ctrl_me);
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available = FALSE;
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_available = FALSE;
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG024_available = FALSE;
	DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available = FALSE;
}

void DAB_Tuner_Ctrl_FindIndex(Ts_CurrentSidLinkingInfo st_CurrentSidLinkingInfo)
{
	Tu8 freqindex,ensindex =0 ;
	ALternateFreq = MAX_ALT_FREQUENCY;
	ensindex = MAX_ALT_EID;
	
	for(freqindex = 0; freqindex < MAX_ALT_FREQUENCY; freqindex++)
	{
		if(st_CurrentSidLinkingInfo.Alt_Freq[freqindex] == 0)
		{
			ALternateFreq = freqindex;
			break;	
		}
	}
	
	for(ensindex = 0;ensindex < MAX_ALT_EID; ensindex++)
	{
		if(st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[ensindex].EId == 0)
		{
			u8_Alt_Ensemble_ID = ensindex;
			break;
		}
		
	}
	
}

void ClearLinkingData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	DAB_Tuner_Ctrl_me->Start_Blending = FALSE;
	DAB_Tuner_Ctrl_me->b_Prepare_Blending = FALSE;
	memset(&(DAB_Tuner_Ctrl_me->e_LinkingStatus),0,sizeof(Te_RADIO_DABFM_LinkingStatus));
	memset(&(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo),0,sizeof(Ts_CurrentSidLinkingInfo));
	ALternateFreq = 0;
	u8_Alt_Ensemble_ID = 0;
	AltEnsembleindex		= 0;
//	HardlinkFreqindex 		= 0;
//	HardlinkEnseindex 		= 0;
	b_Hardlinksused 		= FALSE;
	Hardlinkindex			= 0;
	Altfreqindex			= 0;
	b_Tune_to_Orginal_freq_Check = FALSE;
	memset(&(DAB_Tuner_Ctrl_me->st_PI_data_available),0,sizeof(Ts_PI_Data));
	memset(&(DAB_Tuner_Ctrl_me->st_Blending_info),0,sizeof(Ts_Blending_info));
	DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksAvailable = FALSE;
	DAB_Tuner_Ctrl_me->st_Linkingstatus.b_FMHardlinksAvailable = FALSE;
}



Ts32 DAB_Tuner_Ctrl_CaluculateDelayValues(Ts32 delay1 , Ts32 delay2)
{
	
	Ts32 result = 0;
	
	if(delay1 > delay2 )
	{
		result = delay1 - delay2 ;
	}
	else
	{
		result = delay2 - delay1 ;
	}
	
	return 	result ;
}



void DAB_Tuner_Ctrl_Check_Leveldata(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 count =0;
	
	if(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.LevelStatus == 0x01)
	{
		for(count = 4; count >0 ;--count)
		{
			DAB_Tuner_Ctrl_me->Level_data[count] 	= DAB_Tuner_Ctrl_me->Level_data[count - 1];	
		}
		DAB_Tuner_Ctrl_me->Level_data[0] = DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.LevelData ; 

		DAB_Tuner_Ctrl_me->st_Blending_info.Avg_Leveldata = ((DAB_Tuner_Ctrl_me->Level_data[0]	+ DAB_Tuner_Ctrl_me->Level_data[1] + DAB_Tuner_Ctrl_me->Level_data[2] + DAB_Tuner_Ctrl_me->Level_data[3] + DAB_Tuner_Ctrl_me->Level_data[4] ) / 5);

		if(DAB_Tuner_Ctrl_me->st_Blending_info.Avg_Leveldata > MAXIMUM_LEVEL_DATA)
			DAB_Tuner_Ctrl_me->u16_LevelData = MINIMUM_LEVEL_DATA ;
		else
			DAB_Tuner_Ctrl_me->u16_LevelData = 	DAB_Tuner_Ctrl_me->st_Blending_info.Avg_Leveldata;

		DAB_Tuner_Ctrl_me->Level_data_count ++;
		if(	DAB_Tuner_Ctrl_me->Level_data_count > 4)
			b_delayvalue =  TRUE;	
	 
	}


}


void DAB_Tuner_Ctrl_CheckDelayValues(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{

	Ts32 Delayvalue1 = 0;
	Ts32 Delayvalue2 = 0;
	Ts32 Delayvalue3 = 0;
	Tu8 count = 0;										
	if((DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.DelayFound == TRUE) && (DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.ConfidenceLevel != ALLIGNMENT_LOST))
	{
		for(count = 4; count >0 ;--count)
		{
			DAB_Tuner_Ctrl_me->Blending_Delay[count] 	= DAB_Tuner_Ctrl_me->Blending_Delay[count - 1];	
			DAB_Tuner_Ctrl_me->Confidence_level[count]  = DAB_Tuner_Ctrl_me->Confidence_level[count - 1];
		}
		DAB_Tuner_Ctrl_me->Blending_Delay[0] 	= DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.Delay ; 
		DAB_Tuner_Ctrl_me->Confidence_level[0]	= DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.ConfidenceLevel ;

		if((DAB_Tuner_Ctrl_CaluculateDelayValues(DAB_Tuner_Ctrl_me->Blending_Delay[0],DAB_Tuner_Ctrl_me->Blending_Delay[1]) < PREPARE_BLENDING_DELTA_DELAY) && 
		((DAB_Tuner_Ctrl_me->Confidence_level[0] == RELIABLE) || (DAB_Tuner_Ctrl_me->Confidence_level[0] == PROBABLE)) &&
		((DAB_Tuner_Ctrl_me->Confidence_level[1] == RELIABLE) || (DAB_Tuner_Ctrl_me->Confidence_level[1] == PROBABLE)))
		{
			DAB_Tuner_Ctrl_me->Blending_delay_To_Chk_Allignment =(Tu32) DAB_Tuner_Ctrl_me->Blending_Delay[0];
			//DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = 	DAB_Tuner_Ctrl_me->Blending_Delay[0];
			//	DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = (((DAB_Tuner_Ctrl_me->Blending_Delay[0] + DAB_Tuner_Ctrl_me->Blending_Delay[1])/2));
			DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level	 =  RELIABLE ; 
			DAB_Tuner_Ctrl_me->u16_LevelData					 = 	DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.LevelData;
			b_delayvalue	= TRUE;
		}			
		else
		{
			Delayvalue1 = DAB_Tuner_Ctrl_CaluculateDelayValues(DAB_Tuner_Ctrl_me->Blending_Delay[0],DAB_Tuner_Ctrl_me->Blending_Delay[1]);
			Delayvalue2 = DAB_Tuner_Ctrl_CaluculateDelayValues(DAB_Tuner_Ctrl_me->Blending_Delay[1],DAB_Tuner_Ctrl_me->Blending_Delay[2]);
			Delayvalue3 = DAB_Tuner_Ctrl_CaluculateDelayValues(DAB_Tuner_Ctrl_me->Blending_Delay[0],DAB_Tuner_Ctrl_me->Blending_Delay[2]);

			if((Delayvalue1 <= PREPARE_BLENDING_DELTA_DELAY) && (Delayvalue2 <= PREPARE_BLENDING_DELTA_DELAY) && (Delayvalue3 <= PREPARE_BLENDING_DELTA_DELAY))
			{
				DAB_Tuner_Ctrl_me->Blending_delay_To_Chk_Allignment = (((DAB_Tuner_Ctrl_me->Blending_Delay[0] + DAB_Tuner_Ctrl_me->Blending_Delay[1] + DAB_Tuner_Ctrl_me->Blending_Delay[2])/3));
				//DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay = (((DAB_Tuner_Ctrl_me->Blending_Delay[0] + DAB_Tuner_Ctrl_me->Blending_Delay[1] + DAB_Tuner_Ctrl_me->Blending_Delay[2])/3));
				DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level	 =  PROBABLE ; 
				DAB_Tuner_Ctrl_me->u16_LevelData					 = 	DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify.LevelData;
				b_delayvalue	= TRUE;
			}
			else
			{
				DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay =  0 ;
				DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level	 =  UN_RELIABLE;
			}
		}
	}
}


void DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 Searchindex = 0, u8_ComponentIndexSearch = 0;

	for(Searchindex = 0; Searchindex < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices; Searchindex ++)
	{
		if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].ProgServiceId	==  DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId)
		{
			if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].CharSet == 0x00)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].CharSet == 0x06)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
			}
			else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].CharSet == 0x0f)
			{
				DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
			}
			else
			{
			/*Doing nothing */ 	
			}/* For MISRA C */
			SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.service_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
			for(u8_ComponentIndexSearch = 0 ; u8_ComponentIndexSearch < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.NoOfComponents ; u8_ComponentIndexSearch++)
			{
				if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[u8_ComponentIndexSearch].InternalCompId == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u16_SCIdI)
				{
					if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[u8_ComponentIndexSearch].CharSet == 0x00)
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
					}
					else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[u8_ComponentIndexSearch].CharSet == 0x06)
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
					}
					else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[u8_ComponentIndexSearch].CharSet == 0x0f)
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
					}
					else
					{
					/*Doing nothing */ 	
					}/* For MISRA C */
					SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_currentEnsembleData.servicecomponent_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[u8_ComponentIndexSearch].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
				}
				else
				{
				/*Doing nothing */
				}/* For MISRA C */			
			}
		}
		else
		{
		/*Doing nothing */
		}/* For MISRA C */
	}
}

void DAB_Tuner_Ctrl_Sort_DABAlternative(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu32 Frequency = 0;
	Ts8 RSSi = 0;
	Tu16 Eid = 0;
	Tu32 Sid =0;
	Tu8 index, index1 =0;

	if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_TUNED)
	{
		for (index = 0;((index < MAX_ALT_FREQUENCY) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[index]!=0)); index++)
		{
			for(index1 =index+1;((index1 < MAX_ALT_FREQUENCY) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[index1]!=0) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index]!=0)); index1++)
			{
				if((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index] < DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index1]) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index1]!=0))
				{
					Frequency = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[index];
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[index] = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[index1];
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[index1] = Frequency;
					RSSi = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index];
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index] = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index1];
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[index1] = RSSi;
				}
			}	
		
		}
	}

	if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_TUNED)
	{
		for(index = 0;((index < MAX_ALT_EID) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq!=0)); index++)
		{
			for(index1 = index+1;(index1 < MAX_ALT_EID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq !=0 && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq !=0); index1++)
			{
				if((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq_RSSI < DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq_RSSI && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq_RSSI!=0))
				{
					Frequency = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq = Frequency;
					RSSi = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq_RSSI;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].Freq_RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq_RSSI;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].Freq_RSSI = RSSi;
					Eid = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].EId;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index].EId = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].EId;	
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[index1].EId = Eid;
				}
			}
		}
	}

	if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_TUNED)
	{
		for(index=0 ;index < MAX_HARDLINK_SID ; index++)
		{
			if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq==0 && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].SId!=0)
			{
				for(index1= index+1; index1 < MAX_HARDLINK_SID ; index1++)
				{
					if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq!=0)
					{
						Frequency = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq = Frequency;
						RSSi = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq_RSSI;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI = RSSi;
						Eid = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId;	
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId = Eid;
						Sid = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].SId;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].SId = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].SId;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].SId = Sid;					
					}
					
				}
				
			}
		}
		for(index = 0;(index < MAX_HARDLINK_SID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq!=0);index++)
		{
			for(index1 = index+1;(index1 < MAX_HARDLINK_SID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq!=0);index1++)
			{
				if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI < DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq_RSSI)
				{
					Frequency = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq = Frequency;
					RSSi = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].EId_Data.Freq_RSSI;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.Freq_RSSI = RSSi;
					Eid = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId;	
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].EId_Data.EId = Eid;
					Sid = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].SId;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index].SId = DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].SId;
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[index1].SId = Sid;
				}
			}
		}
	}
	
}

void DAB_Tuner_Ctrl_Check_For_Alternate(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 Noofindex = 0;

	Tu8 Sidindex = 0;

	Tu8 AltEnsindex = 0;

	AltEnsindex = AltEnsembleindex ; 


	Sidindex = Hardlinkindex ;

	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidFreqAvailable == TRUE)
	{
		for(Noofindex = AltEnsindex ;Noofindex < MAX_ALT_EID ; Noofindex++)
		{
			if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[Noofindex].Freq!= 0)
			{
				AltEnsembleindex 		= 	Noofindex;
				break;
			}
			else
			{
			}	
		}
	}	
		
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksFreqAvailable == TRUE)
	{
		for(Noofindex = Sidindex ; Noofindex < MAX_HARDLINK_SID ; Noofindex++)
		{
			if((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Noofindex].EId_Data.Freq!=0) && (DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Noofindex].LA == 1))
			{
				Hardlinkindex 		= 	Noofindex ;
				break;
			}
			else
			{
			}		
		}
	}			
}

Tbool DAB_Tuner_Ctrl_DAB_HARDLINK_Freq_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	DAB_Tuner_Ctrl_Check_For_Alternate(DAB_Tuner_Ctrl_me);	
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksFreqAvailable == TRUE && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].EId_Data.Freq!=0 && Hardlinkindex < MAX_HARDLINK_SID && DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status != DAB2DAB_HARDLINK_FREQ_SORTED)
	{
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_HARDLINK_FREQ_TUNED;
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
		return TRUE;
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_TUNED)
	{
			DAB_Tuner_Ctrl_Sort_DABAlternative(DAB_Tuner_Ctrl_me);
			DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_HARDLINK_FREQ_SORTED;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
			Hardlinkindex = 0;
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,DAB_DAB_LINKING);
			if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].EId_Data.Freq!=0 && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].EId_Data.Freq_RSSI > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
			{
		//		DabTuner_MsgSndSetTuneTocmd(ENABLE_TUNESTATUS_NOTIFICATION);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Best Hardlink Frequency");
				return TRUE;
			}
			else
			{
				Hardlinkindex = MAX_HARDLINK_SID;
				DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
				return FALSE;
			}
	}   
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_SORTED)
	{
		if(DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus != SERVICE_AVAILABLE && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].EId_Data.Freq!=0 && Hardlinkindex < MAX_HARDLINK_SID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].EId_Data.Freq_RSSI > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
		{
			DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Next Best Hardlink Frequency");
			return TRUE;
		}
		else
		{
//			DabTuner_MsgSndSetTuneTocmd(DISABLE_TUNESTATUS_NOTIFICATION);
			Hardlinkindex = MAX_HARDLINK_SID;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			return FALSE;	
		}		
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_BER_SORTED)
	{
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_HARDLINK_FREQ_SORTED;
		return TRUE;
	}
	else
	{
//		DabTuner_MsgSndSetTuneTocmd(DISABLE_TUNESTATUS_NOTIFICATION);
		return FALSE;		
	}
	
}


Tbool DAB_Tuner_Ctrl_DABAlternate_EID_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	DAB_Tuner_Ctrl_Check_For_Alternate(DAB_Tuner_Ctrl_me);	
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABSameSidFreqAvailable == TRUE && AltEnsembleindex < MAX_ALT_EID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex].Freq!=0 && DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status != DAB2DAB_ALTEID_FREQ_SORTED)
	{
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTEID_FREQ_TUNED;
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
		return TRUE;
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_TUNED)
	{
			DAB_Tuner_Ctrl_Sort_DABAlternative(DAB_Tuner_Ctrl_me);
			DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTEID_FREQ_SORTED;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
			AltEnsembleindex = 0;
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,DAB_DAB_LINKING);
			if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex].Freq!=0 && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex].Freq_RSSI > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
			{
			//	DabTuner_MsgSndSetTuneTocmd(ENABLE_TUNESTATUS_NOTIFICATION);
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Best Alternaet Ensemble");
				return TRUE;
			}
			else
			{
				AltEnsembleindex = MAX_ALT_EID;
				DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
				return FALSE;
			}
	}   
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_SORTED)
	{
		if(DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus != SERVICE_AVAILABLE && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex].Freq!=0 && AltEnsembleindex < MAX_ALT_EID && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex].Freq_RSSI > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
		{
			DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Next Best Alternaet Ensemble");
			return TRUE;
		}
		else
		{
//			DabTuner_MsgSndSetTuneTocmd(DISABLE_TUNESTATUS_NOTIFICATION);
			AltEnsembleindex = MAX_ALT_EID;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			return FALSE;
		}			
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_BER_SORTED)
	{
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTEID_FREQ_SORTED;
		return TRUE;
	}
	else
	{
		return FALSE;		
	}
	
}

Tbool DAB_Tuner_Ctrl_DABAlternate_Freq_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	if(DAB_Tuner_Ctrl_me->st_Linkingstatus.b_AlternateDABFreqAvailable == TRUE && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Altfreqindex]!=0 && Altfreqindex < MAX_ALT_FREQUENCY && DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status != DAB2DAB_ALTFREQ_SORTED)
	{
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTFREQ_TUNED;
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
		return TRUE;
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_TUNED)
	{
	
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
		Altfreqindex = 0;
		DAB_Tuner_Ctrl_Sort_DABAlternative(DAB_Tuner_Ctrl_me);
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTFREQ_SORTED;
		DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,DAB_DAB_LINKING);
		if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Altfreqindex]!=0 && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[Altfreqindex] > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
		{
		//	DabTuner_MsgSndSetTuneTocmd(ENABLE_TUNESTATUS_NOTIFICATION);
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Best Alternaet Frequency");
			return TRUE;
		}
		else
		{
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			return TRUE;
		}
	}		
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_SORTED)
	{
		if(DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus != SERVICE_AVAILABLE && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Altfreqindex]!=0 && Altfreqindex < MAX_ALT_FREQUENCY && DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq_RSSI[Altfreqindex] > DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.s8_Tuned_Freq_RSSI)
		{
			DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Next Best Alternaet Frequency");
			return TRUE;
		}
		else
		{
//			DabTuner_MsgSndSetTuneTocmd(DISABLE_TUNESTATUS_NOTIFICATION);
			Altfreqindex = MAX_ALT_FREQUENCY;
			DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
			return FALSE;
		}	
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_BER_SORTED)
	{
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ALTFREQ_SORTED;
		return TRUE;
	}
	else
	{
//		DabTuner_MsgSndSetTuneTocmd(DISABLE_TUNESTATUS_NOTIFICATION);
		return FALSE;
	}
	
}

Tbool DAB_Tuner_Ctrl_DABAlternate_Original_Freq_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ORIGINAL_FREQ_TUNED && b_Tune_to_Orginal_freq_Check == FALSE)
	{
		if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency == 0)
			DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = DAB_Tuner_Ctrl_me->st_lsmdata.u32_Frequency;
		DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_BEFORE_CHECK_ORIGINAL_TUNED;
		return TRUE;
	}
	else if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ORIGINAL_FREQ_TUNED && b_Tune_to_Orginal_freq_Check == TRUE)
	{
		DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_AFTER_CHECK_ORIGINAL_TUNED;
		//DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;	
	}
}

void DAB_Tuner_Ctrl_CheckForDABAlternative(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS))
	{
		if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_DAB_AF_Settings != DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE)
		{
			DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = TRUE ;
			if(DAB_Tuner_Ctrl_DABAlternate_Original_Freq_Available_Check(DAB_Tuner_Ctrl_me)!= FALSE)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Original Frequency To take RSSI %d",DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
				if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status != DAB2DAB_AFTER_CHECK_ORIGINAL_TUNED)
					(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
				DabTuner_MsgSndSetRSSINotifier(0x01);
				st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				
			}
			else if(DAB_Tuner_Ctrl_DABAlternate_Freq_Available_Check(DAB_Tuner_Ctrl_me)!= FALSE)
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Checking for DAB alternative frequencie %d",DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Altfreqindex] *  16);
				(void)DabTuner_MsgSndTuneTo_Cmd((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq[Altfreqindex++] * 16));
				if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTFREQ_SORTED && DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted == FALSE)
				{
					DabTuner_MsgSndSetRSSINotifier(0x01);
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else
				{	
					DabTuner_MsgSndSetRSSINotifier_cmd(0x01);
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0;
				b_Hardlinksused = FALSE;
			}
			else if(DAB_Tuner_Ctrl_DABAlternate_EID_Available_Check(DAB_Tuner_Ctrl_me)!= FALSE)		 
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Checking for DAB alternative ensemble %d ",DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex].Freq *  16);
				(void)DabTuner_MsgSndTuneTo_Cmd((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.Alt_EId[AltEnsembleindex++].Freq * 16));
				if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_ALTEID_FREQ_SORTED && DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted == FALSE)
				{
					DabTuner_MsgSndSetRSSINotifier(0x01);
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else
				{	
					DabTuner_MsgSndSetRSSINotifier_cmd(0x01);
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0;
				b_Hardlinksused = FALSE;
			}
			else if(DAB_Tuner_Ctrl_DAB_HARDLINK_Freq_Available_Check(DAB_Tuner_Ctrl_me)!= FALSE)
			{
				b_Hardlinksused = TRUE;
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] Checking for DAB alternative Hard link SId %d",DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex].EId_Data.Freq *  16);
				(void)DabTuner_MsgSndTuneTo_Cmd((DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[Hardlinkindex++].EId_Data.Freq * 16));
				if(DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status == DAB2DAB_HARDLINK_FREQ_SORTED && DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted == FALSE)
				{
					DabTuner_MsgSndSetRSSINotifier(0x01);
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}
				else
				{	
					DabTuner_MsgSndSetRSSINotifier_cmd(0x01);
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
				}	
				DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0;
			}
			else
			{
				if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE || DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE_END)
				{
					// DabTuner_MsgSndSetTuneTocmd(ENABLE_TUNESTATUS_NOTIFICATION);
					DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
					if(DAB_Tuner_Ctrl_me->e_RequestCmd == DAB_TUNER_CTRL_AFTUNE_END)
					{
						DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency = 0;
						DAB_Tuner_Ctrl_me->e_RequestCmd = DAB_TUNER_CTRL_INVALID;
					}
					else
					{
						DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);
						DAB_Tuner_Ctrl_Transition_to_Scanstate(DAB_Tuner_Ctrl_me);
					}	
				}
				else if(b_Tune_to_Orginal_freq_Check == FALSE)
				{		
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Tuning to Original ensemble ");
					b_Tune_to_Orginal_freq_Check = TRUE;
					DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
					(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
					DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][DAB_TC] DAB to DAB Linking: Tuned to Original Frequency");
					st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
					}
					else
					{
						/*MISRA C*/	
					}
					b_Hardlinksused = FALSE;
				}
				else
				{
					DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE ;
					DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
					DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
					if((DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED) && (DAB_Tuner_Ctrl_me->e_LinkingStatus != RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
					{
						if(DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL)
						{
							DAB_Tuner_Ctrl_Notify_PICodeList(DAB_Tuner_Ctrl_me->st_PI_data_available,DAB_FM_LINKING_MIN_THRESHOULD,DAB_FM_LINKING_MAX_THRESHOULD,DAB_Tuner_Ctrl_me->st_lsmdata.u32_SId,CHECK_IMPLICIT);		/* commented for testing dab-dab*/ 				
							if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
							{
								if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
								}
								else
								{
									st_TimerId.u32_DAB_DABLinking_Timer = 0;
								}								
							}
							else
							{
							 	/*MISRA C*/	
							}
							st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(2000, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
							}
							else
							{
								/*MISRA C*/	
							}
							DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE;
							DAB_Tuner_Ctrl_me->e_LinkingStatus = RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT;
						}
					}
					else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED) && (DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality > DAB_FM_LINKING_MAX_THRESHOULD) && (DAB_Tuner_Ctrl_me->e_AMFMTUNERStatus == RADIO_FRMWK_COMP_STATUS_NORMAL) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC]Implicit command sent");
						DabTuner_StartBlending_for_implicit(0x80);
						/* Starting timer for StartBlending_cmd time out */
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}						
						}
						else
						{
							/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
							/*MISRA C*/	
						}
						b_Tune_to_Orginal_freq_Check = TRUE;
					}
				 	else if((DAB_Tuner_Ctrl_me->e_LinkingStatus == RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED) && (LIB_ISBITSET(DAB_Tuner_Ctrl_me->u8_SettingStatus, 1)))
					{
						b_Tune_to_Orginal_freq_Check = TRUE;
						DabTuner_StartBlending_Without_delay(0x80);
						if(st_TimerId.u32_StartBlending_Timer > 0)
						{
							if(SYS_StopTimer(st_TimerId.u32_StartBlending_Timer) == FALSE)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_BLENDING_REPLY_TIMEOUT failed");	
							}
							else
							{
								st_TimerId.u32_StartBlending_Timer = 0;
							}						
						}
						else
						{
						/* MISRA C*/	
						}
						st_TimerId.u32_StartBlending_Timer = SYS_StartTimer(DAB_START_BLENDING_CMD_TIMEOUT_TIME,START_BLENDING_REPLY_TIMEOUT,RADIO_DAB_TUNER_CTRL);
						if(st_TimerId.u32_StartBlending_Timer == 0)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_BLENDING_REPLY_TIMEOUT");	
						}
						else
						{
						/*MISRA C*/	
						}
					}
				 	else
					{
						if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
						{
							if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
							{
								if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
								{
									RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
								}
								else
								{
									st_TimerId.u32_DAB_DABLinking_Timer = 0;	
								}							
							}
							else
							{
							 	/*MISRA C*/	
							}
							st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(2000, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
							if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
							{
								RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
							}
							else
							{
								/*MISRA C*/	
							}
							DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE;
						}	
					}
				}	

			}
		}
		else
		{
			if(b_Tune_to_Orginal_freq_Check == FALSE)
			{		
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Tuning to Original ensemble ");
				//SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_currentEnsembleData),&(DAB_Tuner_Ctrl_me->st_lsmdata),sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));
				b_Tune_to_Orginal_freq_Check = TRUE;
				DAB_Tuner_Ctrl_me->b_DAB2DAB_Alternates_Sorted = TRUE;
				//		DabTuner_MsgSndSetTuneTocmd(ENABLE_TUNESTATUS_NOTIFICATION);
				(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
				DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status = DAB2DAB_ORIGINAL_FREQ_TUNED;
				st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
				if(st_TimerId.u32_TuneTimeOut_DABDABBlending_Timer == 0)
				{
					RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
				}
				else
				{
					/*MISRA C*/	
				}
				b_Hardlinksused = FALSE;
			}
			else
			{
				DAB_Tuner_Ctrl_me->b_DAB_Alternatecheck_Ongoing = FALSE;
				if(DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start != TRUE)
				{
					if(st_TimerId.u32_DAB_DABLinking_Timer > 0)
					{
						if(SYS_StopTimer(st_TimerId.u32_DAB_DABLinking_Timer) == FALSE)
						{
							RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Stop Timer for START_DAB_DAB_LINKING failed");	
						}
						else
						{
							st_TimerId.u32_DAB_DABLinking_Timer = 0;	
						}							
					}
					else
					{
					 	/*MISRA C*/	
					}
					st_TimerId.u32_DAB_DABLinking_Timer = SYS_StartTimer(2000, START_DAB_DAB_LINKING, RADIO_DAB_TUNER_CTRL);
					if(st_TimerId.u32_DAB_DABLinking_Timer == 0)
					{
						RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for START_DAB_DAB_LINKING");	
					}
					else
					{
						/*MISRA C*/	
					}
					DAB_Tuner_Ctrl_me->b_DAB_DAB_Linking_Timer_start = TRUE;
				}	
			}
		}	
	}
	else
	{
		/* for MISRA */	
	}
}

void DAB_Tuner_Ctrl_FM_DAB_StationSort(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	
	Tu8 index = 0, index1 = 0;
	Ts_dab_tuner_ctrl_fmdab_linkinfo st_temp_info;

	for(index = 0; index < MAX_FMDAB_SID && DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].u32_Frequency != 0; index++)
	{
		for(index1 = index+1;index1 < MAX_FMDAB_SID && DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].u32_Frequency != 0; index1++)
		{
			if(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].s8_RSSI < DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].s8_RSSI)
			{
				memmove(&st_temp_info,&(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1]),sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo));
				memmove(&(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1]),&(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index]),sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo));
				memmove(&(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index]),&st_temp_info,sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo));
				/*Frequency = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].u32_Frequency;
				DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].u32_Frequency = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].u32_Frequency;
				DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].u32_Frequency = Frequency;
				RSSI = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].s8_RSSI;
				DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].s8_RSSI = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].s8_RSSI;
				DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].s8_RSSI = RSSI;
				Eid = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].u16_EId;
				DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index].u16_EId = DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].u16_EId;
				DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[index1].u16_EId = Eid;*/
			}
		}
	}
}
void DAB_TUNER_CTRL_Tune_To_Same_PI(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me)
{
	DAB_Tuner_Ctrl_me->b_SameSidFound = FALSE;
	DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = FALSE;
	
	if(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search].u32_Frequency != 0 && Same_SID_Search < MAX_FMDAB_SID && DAB_Tuner_Ctrl_me->e_fmdab_linkstatus != DAB_TUNER_CTRL_TUNED_STATIONS_SORTED)
	{
		DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x02;
		(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search++].u32_Frequency);
		DabTuner_MsgSndSetRSSINotifier_cmd(0x01);
		/* Starting the timer for TUNE_TIME_OUT */
		st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
		}
		else
		{
			/*MISRA C*/	
		}
		DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_TUNED_TO_SAME_STATION;
	}
	else if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_TUNED_TO_SAME_STATION)
	{
		Same_SID_Search = 0;
		DAB_Tuner_Ctrl_FM_DAB_StationSort(DAB_Tuner_Ctrl_me);
		if(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search].u32_Frequency != 0)
		{
			DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_TUNED_STATIONS_SORTED;
			DAB_Tuner_Ctrl_Print_Debug_Logs(DAB_Tuner_Ctrl_me ,FM_DAB_LINKING_STATUS);
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] FM_DAB Linking: Tuned to Best One");
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search++].u32_Frequency);
			st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
			}
			else
			{
				/*MISRA C*/	
			}
		}
		else
		{
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START && DAB_Tuner_Ctrl_me->e_Scannstatus == TUNER_CTRL_SCAN_COMPLETED)
			{
				DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
			}
			else
			{	
				DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED;
				DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);
				DAB_TUNER_CTRL_Internal_Msg();		
			}	
		}
	}
	else if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_TUNED_STATIONS_SORTED)
	{
		if((DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus != SERVICE_AVAILABLE) && (DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search].u32_Frequency!=0))
		{
			DAB_Tuner_Ctrl_me->st_selectServiceReply.ReplyStatus = 0;
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] FM_DAB Linking: Tuned to Next Best One");
			DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
			(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search++].u32_Frequency);
			st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
			if(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer == 0)
			{
				RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
			}
			else
			{
				/*MISRA C*/	
			}
		}
		else
		{
			if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START && DAB_Tuner_Ctrl_me->e_Scannstatus == TUNER_CTRL_SCAN_COMPLETED)
			{
				DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
			}
			else
			{	
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] FM_DAB Same PI station service unavailable ");
				DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED;
				DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);
				DAB_TUNER_CTRL_Internal_Msg();		
			}	
		}	
	}
	else if(DAB_Tuner_Ctrl_me->e_fmdab_linkstatus == DAB_TUNER_CTRL_TUNED_STATION_NOTSTABLE && DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search].u32_Frequency!=0 && Same_SID_Search < MAX_FMDAB_SID)
	{
		DAB_Tuner_Ctrl_me->u8_cmd_recall_count = 0x00;
		(void)DabTuner_MsgSndTuneTo_Cmd(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo[Same_SID_Search++].u32_Frequency);
		st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer = SYS_StartTimer(250, TUNE_TIME_OUT, RADIO_DAB_TUNER_CTRL);
		if(st_TimerId.u32_TuneTimeOut_FMDABLinking_Timer == 0)
		{
			RADIO_DEBUG_LOG (RADIO_LOG_LVL_ERROR, "[RADIO][DAB_TC] Timer creation is failed for TUNE_TIME_OUT");	
		}
		else
		{
			/*MISRA C*/	
		}
	}
	else
	{
		if(DAB_Tuner_Ctrl_me->e_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus == DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START && DAB_Tuner_Ctrl_me->e_Scannstatus == TUNER_CTRL_SCAN_COMPLETED)
		{
			DAB_Tuner_Ctrl_Notify_UpdatedLearnMem_AFStatus(DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,DAB_Tuner_Ctrl_me->st_currentEnsembleData);
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_TC] FM_DAB Same PI station component unavailable ");
			DAB_Tuner_Ctrl_me->e_fmdab_linkstatus = DAB_TUNER_CTRL_FMDAB_PI_RECEIVED;		
			DAB_Tuner_Ctrl_Clear_Stationlist(DAB_Tuner_Ctrl_me);	
			DAB_TUNER_CTRL_Internal_Msg();	
		}	
	}

}



void DAB_Tuner_Ctrl_Print_Debug_Logs(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me ,Tu8 type)
{
	
	switch(type)
	{
		
		case FM_HARDLINKS:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB-FM Hardlinks sent to FM, Hardlinks are %s ", DAB_Tuner_Ctrl_me->st_PI_data_available.FMHardlinks);
		}	
		break;
		
		case DAB_ALTERNATE_FREQUENCY:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB-DAB Alternate Frequencies received for tuned SID%s", DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Freq);
		}
		break;
		
		case DAB_ALTERNATE_ENSEMBLE:
		{
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB-DAB Alternate Ensembles received for tuned SID %s", DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Alt_Ensbs.);			
		}
		break;
		
		case DAB_DAB_HARDLINKS:
		{
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB-DAB Hardlinks received for tuned SID %s", DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid);
		}
		break;
		
		case QUALITY_PARAMETERS:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_QUALITY, "[RADIO][DAB_TC] BER_Significant %d, BER_Exponent %d, SNR_Level %d, RSSI %d, Decodingstatus %d, AudioQuality %d, AudioLevel %d", DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_BER_Significant,DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_BER_Exponent, DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_SNR_Level, DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.s8_RSSI, DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_Decodingstatus, DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioQuality, DAB_Tuner_Ctrl_me->st_Tuner_Status_Notification.u8_AudioLevel);
		}
		break;
		
		case BLENDING_INFO:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Best_PI %d,Best_PI quality %d, Best_PI_Received %d, Blending_Final_Delay %d, Avg_Leveldata %d, Confidence_level %d ", DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI,DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality,DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received,DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay,DAB_Tuner_Ctrl_me->st_Blending_info.Avg_Leveldata,DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level);
		}
		break;
		
		case IMPLICIT_SID:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Implicit received from FM  Best_PI %d,Best_PI quality %d, Best_PI_Received %d, Blending_Final_Delay %d, Avg_Leveldata %d, Confidence_level %d ", DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI,DAB_Tuner_Ctrl_me->st_Blending_info.Best_PI_Quality,DAB_Tuner_Ctrl_me->st_Blending_info.b_Best_PI_Received,DAB_Tuner_Ctrl_me->st_Blending_info.Blending_Final_Delay,DAB_Tuner_Ctrl_me->st_Blending_info.Avg_Leveldata,DAB_Tuner_Ctrl_me->st_Blending_info.Confidence_level);
		}
		break;
		
		case BLENDED_TO_FM:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Blended to FM, Linking_status %d, Decoding status %d, quality %d ", DAB_Tuner_Ctrl_me->e_LinkingStatus,DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus,DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality);
		}
		break;
		
		case BLEND_BACK_TO_DAB:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Blended Back to DAB, Linking_status %d,Decoding status %d,quality %d ", DAB_Tuner_Ctrl_me->e_LinkingStatus,DAB_Tuner_Ctrl_me->st_Audiostatus.Decodingstatus,DAB_Tuner_Ctrl_me->st_Audiostatus.AudioQuality);
		}
		break;
		
		case FM_DAB_LINKING_STATUS:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Background: FM_DAB Linking status is  %d", DAB_Tuner_Ctrl_me->e_fmdab_linkstatus);
		}
		break;
		
		case FM_DAB_LINKING_DATA:
		{
			//RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Background: FM_DAB Same PI structure is  %d", DAB_Tuner_Ctrl_me->st_fmdab_linkinfo);			
		}
		break;
		
		case DAB_DAB_LINKING:
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] DAB to DAB Linking status is  %d", DAB_Tuner_Ctrl_me->e_DAB2DAB_Linking_Status);
		}
		break;
		
	}
}


void DAB_Tuner_Ctrl_Clear_FMDAB_LinkingData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	DAB_Tuner_Ctrl_me->b_SamePIFoundNotifySent = FALSE;
	memset(DAB_Tuner_Ctrl_me->st_fmdab_linkinfo,0,sizeof(Ts_dab_tuner_ctrl_fmdab_linkinfo) * MAX_FMDAB_SID);
	memset(&(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Notify),0,sizeof(Ts_PrepareForBlending_Notify));
	memset(&(DAB_Tuner_Ctrl_me->st_StartTimeStretch_not),0,sizeof(Ts_StartTimeStretch_not));
	memset(&(DAB_Tuner_Ctrl_me->u16_LevelData),0,sizeof(Tu16));
	memset(&(DAB_Tuner_Ctrl_me->st_StartTimeStretch_repl),0,sizeof(Ts_StartTimeStretch_repl));
	memset(&(DAB_Tuner_Ctrl_me->st_Start_Blending_Reply),0,sizeof(Ts_StartBlending_Reply));
	memset(&(DAB_Tuner_Ctrl_me->st_PrepareForBlending_Reply),0,sizeof(Ts_PrepareForBlending_Reply));
}



