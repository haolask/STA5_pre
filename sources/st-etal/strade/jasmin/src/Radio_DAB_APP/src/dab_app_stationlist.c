/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file dab_app_stationlist.c																  				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains station list related API's for DAB Application.					*
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "dab_app_hsm.h"
#include "dab_app_stationlist.h"



/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
//Tu16 Testingcount = 0;
extern Ts_Tuner_Ctrl_EnsembleInfo		ast_EnsembleInfo[DAB_APP_MAX_ENSEMBLES];
extern Ts_Tuner_Ctrl_ServiceInfo 		ast_ServiceInfo[DAB_APP_MAX_SERVICES];
extern Ts_Tuner_Ctrl_ComponentInfo		ast_ComponentInfo[DAB_APP_MAX_COMPONENTS];
extern Tu16 ensembleIndex;
extern Tu16 Services;
extern Tu16 Components;


/*-----------------------------------------------------------------------------
    variables 
-----------------------------------------------------------------------------*/
Tu16 u16_Num_Of_Ensembles = 0u;
Tu16  u16_Num_Of_Services = 0u;
Tu16 u16_Num_Of_Components = 0u;

Ts_DAB_App_StationList			st_DAB_App_StationList;
Ts_DAB_App_MultiplexStationList 	st_DAB_App_MultiplexStationList;		/* For Ensembles */


/*-----------------------------------------------------------------------------
    private function definitions
-----------------------------------------------------------------------------*/

#if 0
/*===========================================================================*/
/*  void DAB_APP_AddServCompToSTL											 */
/*===========================================================================*/
void DAB_APP_AddServCompToSTL(Tu32 u32_Cur_Frequency, Tu16 u16_Cur_EId, Tu8 u8_Cur_ECC,  Tu16 i)
{
	Tu32	u32_Cur_SId = 0u;
	Tu16 	j,k;
	
	for(j = 0u ; j < u16_Num_Of_Services ; j++)
	{
		/* Checking for the services present in the current ensemble */
		if((u16_Cur_EId == ast_ServiceInfo[j].st_BasicEnsInfo.u16_EId) 
			&& (u8_Cur_ECC == ast_ServiceInfo[j].st_BasicEnsInfo.u8_ECC) 
			&& (u32_Cur_Frequency == ast_ServiceInfo[j].st_BasicEnsInfo.u32_Frequency))
		{
			if(ast_ServiceInfo[j].st_ServiceLabel.au8_label[0] == 0)
			{
				continue;	
			}
			u32_Cur_SId = ast_ServiceInfo[j].u32_SId;
			
			for(k = 0 ; k < u16_Num_Of_Components ; k++)
			{
				/*Checking for the secondary service component*/ 
				if((u16_Cur_EId == ast_ComponentInfo[k].st_BasicEnsInfo.u16_EId) 
					&& (u8_Cur_ECC == ast_ComponentInfo[k].st_BasicEnsInfo.u8_ECC) 
					&& (u32_Cur_SId == ast_ComponentInfo[k].u32_SId) 
					&& (u32_Cur_Frequency == ast_ComponentInfo[k].st_BasicEnsInfo.u32_Frequency))
				{
					if(u16_Num_Of_Stations < DAB_APP_MAX_STATIONS) /* checking if the no. of stations reached the max capacity */
					{
						/* Updating secondary service component information to the station list */
						ast_DAB_App_StationList[u16_Num_Of_Stations].u32_Frequency = ast_ComponentInfo[k].st_BasicEnsInfo.u32_Frequency ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].u16_EId = ast_ComponentInfo[k].st_BasicEnsInfo.u16_EId ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].u8_ECC = ast_ComponentInfo[k].st_BasicEnsInfo.u8_ECC ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].u32_SId = ast_ComponentInfo[k].u32_SId ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].u16_SCIdI = ast_ComponentInfo[k].u16_SCIdI ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].pst_ensembleInfo = &ast_EnsembleInfo[i] ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].pst_serviceInfo = &ast_ServiceInfo[j] ;
						ast_DAB_App_StationList[u16_Num_Of_Stations].pst_componentInfo = &ast_ComponentInfo[k] ;
						u16_Num_Of_Stations++;  /* Incrementing the station list index */
					}
					else
					{
							/* Nothing to do*/
					}/*for misra c guideline*/
				}
				else
				{
					/* Nothing to do*/
				}/*for misra c guideline*/
			}
		}
	}
}


/*===========================================================================*/
/*  void DAB_APP_CreateStationList											*/
/*===========================================================================*/
Te_RADIO_ReplyStatus DAB_APP_CreateStationList(Ts_dab_app_inst_hsm *pst_me_request)
{
	Te_RADIO_ReplyStatus e_CreateSTLStatus;
	Tu32		u32_Cur_Frequency = 0u;                  
    Tu16		u16_Cur_EId	=	0u;                        
    Tu8			u8_Cur_ECC	=	0u;                       
	Tu16		i;
	u16_Num_Of_Ensembles = ensembleIndex;
	u16_Num_Of_Services = Services;
    u16_Num_Of_Components = Components;
	e_CreateSTLStatus = REPLYSTATUS_SUCCESS;

	if((u16_Num_Of_Ensembles == 0u) || (u16_Num_Of_Services == 0u) || (u16_Num_Of_Components == 0u))
	{
		/*Happens if the EnsembleInfo, ServiceInfo, ComponentInfo is not updated properly*/  
		e_CreateSTLStatus = REPLYSTATUS_FAILURE ;
	}
	else
	{
		u16_Num_Of_Stations = 0u;
		memset(ast_DAB_App_StationList,0,sizeof(Ts_DAB_APP_StationInfo));
		for(i = 0u;(i < u16_Num_Of_Ensembles) && (u16_Num_Of_Stations < DAB_APP_MAX_STATIONS);i++)
		{
		
			/*updating the ensemble information */
			u32_Cur_Frequency = ast_EnsembleInfo[i].st_BasicEnsInfo.u32_Frequency ;
			u16_Cur_EId = ast_EnsembleInfo[i].st_BasicEnsInfo.u16_EId ;
			u8_Cur_ECC = ast_EnsembleInfo[i].st_BasicEnsInfo.u8_ECC ;
			DAB_APP_AddServCompToSTL(u32_Cur_Frequency, u16_Cur_EId, u8_Cur_ECC, i);
		
		}
		e_CreateSTLStatus =	REPLYSTATUS_SUCCESS ;
	}
	return e_CreateSTLStatus ;
}


#endif 

#if 1

/*===========================================================================*/
/*  void DAB_APP_AddEnsemblesToSTL											 */
/*===========================================================================*/
void DAB_APP_AddEnsemblesToSTL(Tu16 u16_Cur_EId, Tu16 u16_EnsembleIndex, Tu32 u32_Cur_Frequency)
{
	Te_DAB_App_LabelStatus e_LableStatus = DAB_APP_LABEL_INVALID ;
	
	Tbool  b_Stored = FALSE;
	Tu8  u8_labelindex = 0;
	Tu8  Station_Index = 0;
		
	if(u16_Cur_EId == ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u16_EId)
	{
		for(u8_labelindex=0; u8_labelindex < DAB_TUNER_CTRL_MAX_LABEL_LENGTH; u8_labelindex++)
		{
			 if((ast_EnsembleInfo[u16_EnsembleIndex].st_EnsembleLabel.au8_label[u8_labelindex] != 32) && (ast_EnsembleInfo[u16_EnsembleIndex].st_EnsembleLabel.au8_label[u8_labelindex] != 0))
			 {
				e_LableStatus =  DAB_APP_LABEL_PRESENT ; 
				break;
			 }
			 else
			 {
				 e_LableStatus =  DAB_APP_LABEL_NOT_PRESENT ;
			 }
		}
	}
		
	for(Station_Index = 0; Station_Index <= st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList-1 ; Station_Index++)
	{
		if(u16_Cur_EId == st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[Station_Index].u16_EId)
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Ensemble is already present in the stationlist ");
			b_Stored = TRUE;
			break;
		}
		else
		{
			b_Stored = FALSE;
		}
	}
	
	if((b_Stored == FALSE) && (e_LableStatus == DAB_APP_LABEL_PRESENT)) /* checking if the Ensemble lable is present */
	{	
		/* Checking for the service component */ 
		if((u16_Cur_EId == ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u16_EId) 
			&& (u32_Cur_Frequency == ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u32_Frequency))
		{
			if(st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList < DAB_APP_MAX_ENSEMBLES) /* checking if the no. of stations reached the max capacity */
			{
				/* Updating service component information to the station list */
				//st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList].u32_Frequency = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u32_Frequency ;
				st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList].u16_EId = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u16_EId ;
				st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList].RSSI = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.RSSI ;
				
				SYS_RADIO_MEMCPY((st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList].au8_EnsembleLabel),
									(ast_EnsembleInfo[u16_EnsembleIndex].st_EnsembleLabel.au8_label),	DAB_APP_MAX_LABEL_LENGTH );
									
				st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList].u8_CharSet = ast_EnsembleInfo[u16_EnsembleIndex].st_EnsembleLabel.u8_CharSet;
								
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Ensemble is added for this Eid:%d",u16_Cur_EId);
				
				st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList++;  /* Incrementing the station list index */
				
			}
			else
			{
				/*for misra c guideline*/
			}
		}
		else
		{
			/* Nothing to do*/
		}/*for misra c guideline*/
	}
}
/*===========================================================================*/
/*  void DAB_APP_AddServCompToSTL											 */
/*===========================================================================*/
void DAB_APP_AddServCompToSTL(Tu32 u32_Cur_Frequency, Tu16 u16_Cur_EId, Tu8 u8_Cur_ECC,  Tu16 u16_EnsembleIndex,Tu32 u32_previousfreq)
{
	Te_DAB_App_LabelStatus e_LableStatus = DAB_APP_LABEL_INVALID ;
	Tu32	u32_Cur_SId = 0;
	Tu16 	u16_ServiceIndex = 0, u16_ServiceCompIndex = 0;
	Tu8 u8_componentscheck = 0; Tu8 u8_servicecomponentcount = 0; Tu8 u8_labelindex = 0;

	UNUSED(u8_Cur_ECC);
	UNUSED(u32_previousfreq);
	
	for(u16_ServiceIndex = 0 ; u16_ServiceIndex < u16_Num_Of_Services ; u16_ServiceIndex++)
	{
		/* Checking for the services present in the current ensemble */
		if((u16_Cur_EId == ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u16_EId) 
			&& (u32_Cur_Frequency == ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u32_Frequency))
		{
			/* If the service label is present or not*/		
		/*	if(((ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label[0] == 0) && (ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label[1] == 0)) || ((ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label[0] == " ") && (ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label[1] == " ")))
			{
				e_LableStatus =  DAB_APP_LABEL_NOT_PRESENT ;	
			}
			else
			{
				e_LableStatus =  DAB_APP_LABEL_PRESENT ;
			}*/
			for(u8_labelindex=0; u8_labelindex < DAB_TUNER_CTRL_MAX_LABEL_LENGTH; u8_labelindex++)
			{
				 if((ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label[u8_labelindex] != 32) && (ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label[u8_labelindex] != 0))
				 {
					e_LableStatus =  DAB_APP_LABEL_PRESENT ; 
					break;
				 }
				 else
				 {
					 e_LableStatus =  DAB_APP_LABEL_NOT_PRESENT ;
				 }
			}
			
			u32_Cur_SId = ast_ServiceInfo[u16_ServiceIndex].u32_SId;
			//u8_ServiceCount = 0 ;
			u8_servicecomponentcount = 0;
			#if 0
			for(u16_ServiceMatchIndex = 0 ; u16_ServiceMatchIndex <= u16_Num_Of_Services ; u16_ServiceMatchIndex++)
			{	
				/* Checking the occurance of u32_Cur_SId in ast_ServiceInfo */
				if(u32_Cur_SId == ast_ServiceInfo[u16_ServiceMatchIndex].u32_SId)
				{
					u8_ServiceCount++ ;
				}
			}
			#endif
			
			if(e_LableStatus == DAB_APP_LABEL_PRESENT) /* checking if the SId is not repeated and label is present */
			{
				for(u16_ServiceCompIndex = 0 ; u16_ServiceCompIndex < u16_Num_Of_Components ; u16_ServiceCompIndex++)
				{
					/* Checking for the service component */ 
					if((u16_Cur_EId == ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u16_EId) 
						&& (u32_Cur_SId == ast_ComponentInfo[u16_ServiceCompIndex].u32_SId) 
						&& (u32_Cur_Frequency == ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u32_Frequency))
					{
						if(st_DAB_App_StationList.u8_NoOfStationsInList < DAB_APP_MAX_STATIONS) /* checking if the no. of stations reached the max capacity */
						{
							/* Updating service component information to the station list */
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u32_Frequency = ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u32_Frequency ;
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u16_EId = ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u16_EId ;
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u8_ECC = ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u8_ECC ;
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u32_SId = ast_ComponentInfo[u16_ServiceCompIndex].u32_SId ;
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u16_SCId = ast_ComponentInfo[u16_ServiceCompIndex].u16_SCIdI ;
							
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u8_CharSet = ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.u8_CharSet ;
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u8_CharSet = ast_ComponentInfo[u16_ServiceCompIndex].st_compLabel.u8_CharSet ;
							st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].RSSI = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.RSSI ;
							
							SYS_RADIO_MEMCPY((st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].st_ServiceLabel),
												(ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label),	DAB_APP_MAX_LABEL_LENGTH );
												
							SYS_RADIO_MEMCPY((st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].st_ServiceCompLabel),
												(ast_ComponentInfo[u16_ServiceCompIndex].st_compLabel.au8_label),	DAB_APP_MAX_LABEL_LENGTH );
													
							st_DAB_App_StationList.u8_NoOfStationsInList++;  /* Incrementing the station list index */
							
							//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Stations added for single service count for frequency:%d and SID:%d",u32_Cur_Frequency,u32_Cur_SId);
							//break;
							//RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Added newly tuned station to stationlist structure for frequency:%d", DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency);
						}
						else
						{
								/* Nothing to do*/
						}/*for misra c guideline*/
					}
					else
					{
						/* Nothing to do*/
					}/*for misra c guideline*/
				}
			}
			else
			{
				//b_Stored = E_FALSE;
				#if 0
				for(Station_Index = 0; Station_Index <= st_DAB_App_StationList.u8_NoOfStationsInList-1 ; Station_Index++)
				{
					if(u32_Cur_SId == st_DAB_App_StationList.ast_DAB_Stations[Station_Index].u32_SId)
					{
						//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Service already added to stationlist b_Storesd is TRUE ");
						b_Stored = TRUE;
						break;
					}
					else
					{
						//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Going to add services to stationlist b_Storesd is FALSE ");
						b_Stored = FALSE;
					}
				}
				#endif
				
				if(e_LableStatus == DAB_APP_LABEL_PRESENT)
				{
					for(u16_ServiceCompIndex = 0 ; u16_ServiceCompIndex < u16_Num_Of_Components ; u16_ServiceCompIndex++)
					{
						if((u16_Cur_EId == ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u16_EId) 
						&& (u32_Cur_SId == ast_ComponentInfo[u16_ServiceCompIndex].u32_SId) 
						&& (u32_Cur_Frequency == ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u32_Frequency))  //&& (u8_Cur_ECC == ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u8_ECC) 
						{
							for(u8_componentscheck =0; u8_componentscheck < st_DAB_App_StationList.u8_NoOfStationsInList; u8_componentscheck++)
							{
								if((ast_ComponentInfo[u16_ServiceCompIndex].u32_SId == st_DAB_App_StationList.ast_DAB_Stations[u8_componentscheck].u32_SId)&&(ast_ComponentInfo[u16_ServiceCompIndex].u16_SCIdI == st_DAB_App_StationList.ast_DAB_Stations[u8_componentscheck].u16_SCId))
								{
									u8_servicecomponentcount++;
									break;
								}
							}
							if(u8_servicecomponentcount < DAB_APP_SAME_COMPONENT_FOUND_ONCE)
							{
								u8_servicecomponentcount = 0;
								if(st_DAB_App_StationList.u8_NoOfStationsInList < DAB_APP_MAX_STATIONS) /* checking if the no. of stations reached the max capacity */
								{
									/* Updating service component information to the station list */
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u32_Frequency = ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u32_Frequency ;
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u16_EId = ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u16_EId ;
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u8_ECC = ast_ComponentInfo[u16_ServiceCompIndex].st_BasicEnsInfo.u8_ECC ;
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u32_SId = ast_ComponentInfo[u16_ServiceCompIndex].u32_SId ;
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u16_SCId = ast_ComponentInfo[u16_ServiceCompIndex].u16_SCIdI ;
						
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u8_CharSet = ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.u8_CharSet ;
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].u8_CharSet = ast_ComponentInfo[u16_ServiceCompIndex].st_compLabel.u8_CharSet ;
						
									st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].RSSI = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.RSSI ;
									SYS_RADIO_MEMCPY((st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].st_ServiceLabel),
														(ast_ServiceInfo[u16_ServiceIndex].st_ServiceLabel.au8_label),	DAB_APP_MAX_LABEL_LENGTH );
											
									SYS_RADIO_MEMCPY((st_DAB_App_StationList.ast_DAB_Stations[st_DAB_App_StationList.u8_NoOfStationsInList].st_ServiceCompLabel),
														(ast_ComponentInfo[u16_ServiceCompIndex].st_compLabel.au8_label),	DAB_APP_MAX_LABEL_LENGTH );
												
									st_DAB_App_StationList.u8_NoOfStationsInList++;  /* Incrementing the station list index */
							
									//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Stations added for new SID, which is not in the station list for frequency:%d and SID:%d",u32_Cur_Frequency,u32_Cur_SId);
									//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Stations added for single service count for frequency:%d",u32_Cur_SId);
									//break;
								}
								else
								{
										/* Nothing to do*/
								}/*for misra c guideline*/
							}
							else
							{
								/* if same service component is twice or more than neglecting it */
							}
						}
						else
						{
							/* Nothing to do*/
						}/*for misra c guideline*/	
					}				
				}
					
			}
		}
	}
}

/*===========================================================================*/
/*  void DAB_APP_CreateStationList											*/
/*===========================================================================*/
Te_RADIO_ReplyStatus DAB_APP_CreateStationList(Te_DAB_App_RequestCmd e_DAB_App_RequestCmd,Tu32 u32_CurrentlyTunedFrequency)
{
	Te_RADIO_ReplyStatus e_CreateSTLStatus;
	Tbool       b_Frequency_Stored = FALSE;
	Tu32		u32_Cur_Frequency = 0;                  
	Tu16		u16_Cur_EId	=	0;                        
	Tu8			u8_Cur_ECC	=	0; 
	Tu8         u8_EnsembleCount = 0 ;
	Tu8         u8_check = 0 , u8_check1 = 0 ,u8_check2 = 0;
	Tu8 		u8_Start_index = 0;
	Tu8 		u8_End_index = 0;  
	Tu32        u32_previousfreq = 0;
	Tu8 		index = 0;
	Tu16		u16_EnsembleIndex = 0,u16_EnsembleMatchIndex = 0 ,u16_Bestindex = 0;
	u16_Num_Of_Ensembles = ensembleIndex;
	u16_Num_Of_Services = Services;
    u16_Num_Of_Components = Components;
	e_CreateSTLStatus = REPLYSTATUS_SUCCESS;

	memset(&st_DAB_App_StationList,0,sizeof(Ts_DAB_App_StationList));
	memset(&st_DAB_App_MultiplexStationList,0,sizeof(Ts_DAB_App_MultiplexStationList));

	if((u16_Num_Of_Ensembles == (Tu16)0) || (u16_Num_Of_Services == (Tu16)0) || (u16_Num_Of_Components == (Tu16)0))
	{
		/*Happens if the EnsembleInfo, ServiceInfo, ComponentInfo is not updated properly*/  
		e_CreateSTLStatus = REPLYSTATUS_FAILURE ;
	
	}
	else
	{
		for(u16_EnsembleIndex = 0 ; ((u16_EnsembleIndex < u16_Num_Of_Ensembles) && (st_DAB_App_StationList.u8_NoOfStationsInList < DAB_APP_MAX_STATIONS) && (st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList < DAB_APP_MAX_MULTIPLEX_ENSEMBLES)); u16_EnsembleIndex++)
		{
			u8_EnsembleCount = 0;
			u16_Cur_EId = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u16_EId ;
			u8_Cur_ECC = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u8_ECC ;
			
			for(u16_EnsembleMatchIndex = 0 ; u16_EnsembleMatchIndex <= u16_EnsembleIndex ; u16_EnsembleMatchIndex++)
			{
				/* Checking the occurance of u16_Cur_EId in ast_EnsembleInfo*/
				if((u16_Cur_EId == ast_EnsembleInfo[u16_EnsembleMatchIndex].st_BasicEnsInfo.u16_EId))	//&& (u8_Cur_ECC == ast_EnsembleInfo[u16_EnsembleMatchIndex].st_BasicEnsInfo.u8_ECC)
				{
					u8_EnsembleCount++ ;
				}
			}
			
			/* checking if the EId is not repeated */
			if(u8_EnsembleCount < DAB_APP_ENSEMBLE_FOUND_TWICE)
			{
				/*updating the ensemble information */
				u32_Cur_Frequency = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u32_Frequency ;
				u32_previousfreq = u32_Cur_Frequency;
				//RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Adding Service components to stationlist ");
				DAB_APP_AddEnsemblesToSTL(u16_Cur_EId, u16_EnsembleIndex, u32_Cur_Frequency);
				DAB_APP_AddServCompToSTL(u32_Cur_Frequency, u16_Cur_EId, u8_Cur_ECC, u16_EnsembleIndex, u32_previousfreq);
			}
			else 
			{
				if(e_DAB_App_RequestCmd == DAB_APP_AFTUNE)
				{
					b_Frequency_Stored = FALSE;
					for(u8_check =0; u8_check < st_DAB_App_StationList.u8_NoOfStationsInList; u8_check++)
					{
						if(u16_Cur_EId == st_DAB_App_StationList.ast_DAB_Stations[u8_check].u16_EId && u32_CurrentlyTunedFrequency != st_DAB_App_StationList.ast_DAB_Stations[u8_check].u32_Frequency)
						{
							u8_Start_index = u8_check ;
							u8_End_index = u8_Start_index;
							for(u8_check1 = u8_check ;(u16_Cur_EId == st_DAB_App_StationList.ast_DAB_Stations[u8_check1].u16_EId && u32_CurrentlyTunedFrequency != st_DAB_App_StationList.ast_DAB_Stations[u8_check1].u32_Frequency && u8_check1 < st_DAB_App_StationList.u8_NoOfStationsInList); u8_check1++)
							{
								u8_End_index++ ;
							}
							
							for(u8_check2 = 0,u8_check1 = u8_Start_index ; u8_check1 < u8_End_index ; u8_check1++ ,u8_check2++ )
							{
								memset(&st_DAB_App_StationList.ast_DAB_Stations[u8_check1],0,sizeof(Ts_DAB_App_Station_Info));
								memmove(&st_DAB_App_StationList.ast_DAB_Stations[u8_check1],&st_DAB_App_StationList.ast_DAB_Stations[u8_End_index+u8_check2],sizeof(Ts_DAB_App_Station_Info));
			
								//memmove(&st_DAB_App_StationList.ast_DAB_Stations[u8_check],&st_DAB_App_StationList.ast_DAB_Stations[u8_check+1],sizeof(Ts_DAB_App_Station_Info));
								st_DAB_App_StationList.u8_NoOfStationsInList-- ;
								u8_check = 0;
							}
							for(index = 0; index < u16_Num_Of_Ensembles ; index++)
							{
								if(u32_CurrentlyTunedFrequency == ast_EnsembleInfo[index].st_BasicEnsInfo.u32_Frequency && u16_Cur_EId == ast_EnsembleInfo[index].st_BasicEnsInfo.u16_EId)
								{
									break;	
								}
								
							}
						}
						
					}
					for(u8_check =0; u8_check < st_DAB_App_StationList.u8_NoOfStationsInList; u8_check++)
					{
						if(u16_Cur_EId == st_DAB_App_StationList.ast_DAB_Stations[u8_check].u16_EId && u32_CurrentlyTunedFrequency == st_DAB_App_StationList.ast_DAB_Stations[u8_check].u32_Frequency)
						{
							b_Frequency_Stored = TRUE;
							break;
						}
					}
					
					if(b_Frequency_Stored == FALSE)
					{
							u32_Cur_Frequency = ast_EnsembleInfo[index].st_BasicEnsInfo.u32_Frequency;
							u16_Cur_EId = ast_EnsembleInfo[index].st_BasicEnsInfo.u16_EId;
							DAB_APP_AddServCompToSTL(u32_Cur_Frequency, u16_Cur_EId, u8_Cur_ECC, index, u32_previousfreq);
					}
					
				}
				else if((e_DAB_App_RequestCmd != DAB_APP_SER_COMP_SEEK)	&& (e_DAB_App_RequestCmd != DAB_APP_MANUAL_TUNEBY_CHNAME))
				{
					//u32_previousfreq = u32_Cur_Frequency;
					u16_Bestindex = u16_EnsembleIndex;
					for (index = 0; (index < DAB_APP_MAX_ENSEMBLES && ast_EnsembleInfo[index].st_BasicEnsInfo.u32_Frequency!=0); index++)
					{
						if(u16_Cur_EId == ast_EnsembleInfo[index].st_BasicEnsInfo.u16_EId)
						{
							if(ast_EnsembleInfo[u16_Bestindex].st_BasicEnsInfo.RSSI < ast_EnsembleInfo[index].st_BasicEnsInfo.RSSI) 
							{
									u16_Bestindex = index;	
														
							}	
						}	
											
					}	
					if(u16_Bestindex < u16_EnsembleIndex)
					{
						b_Frequency_Stored = TRUE;
					}
					else
					{
						b_Frequency_Stored = FALSE;
					}
								
					if(b_Frequency_Stored == FALSE)
					{
						for(u8_check =0; u8_check < st_DAB_App_StationList.u8_NoOfStationsInList; u8_check++)
						{
							if(u16_Cur_EId == st_DAB_App_StationList.ast_DAB_Stations[u8_check].u16_EId && ast_EnsembleInfo[u16_Bestindex].st_BasicEnsInfo.u32_Frequency != st_DAB_App_StationList.ast_DAB_Stations[u8_check].u32_Frequency)
							{
								u8_Start_index = u8_check ;
								u8_End_index = u8_Start_index;
								for( u8_check1 = u8_check ;(u16_Cur_EId == st_DAB_App_StationList.ast_DAB_Stations[u8_check1].u16_EId && ast_EnsembleInfo[u16_Bestindex].st_BasicEnsInfo.u32_Frequency != st_DAB_App_StationList.ast_DAB_Stations[u8_check1].u32_Frequency && u8_check1 < st_DAB_App_StationList.u8_NoOfStationsInList); u8_check1++)
								{
									u8_End_index++ ;
								}
								
								for(u8_check2 = 0,u8_check1 = u8_Start_index ; u8_check1 < u8_End_index ; u8_check1++ ,u8_check2++ )
								{
									memset(&st_DAB_App_StationList.ast_DAB_Stations[u8_check1],0,sizeof(Ts_DAB_App_Station_Info));
									memmove(&st_DAB_App_StationList.ast_DAB_Stations[u8_check1],&st_DAB_App_StationList.ast_DAB_Stations[u8_End_index+u8_check2],sizeof(Ts_DAB_App_Station_Info));		
									st_DAB_App_StationList.u8_NoOfStationsInList-- ;
								}
							}
						}
					}
					/*updating the ensemble information */
					//index = index--;
					u32_Cur_Frequency = ast_EnsembleInfo[u16_Bestindex].st_BasicEnsInfo.u32_Frequency ;
					u8_Cur_ECC = ast_EnsembleInfo[u16_Bestindex].st_BasicEnsInfo.u8_ECC ;
					/* Checking for the ensemble is already available is low rssi, removing from stationlist*/
					if(b_Frequency_Stored == FALSE)
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[RADIO][DAB_APP] Newly added frequency in StationList:%d", u32_Cur_Frequency);
						//DAB_APP_AddEnsemblesToSTL(u16_Cur_EId, u8_index, u32_Cur_Frequency);
						DAB_APP_AddServCompToSTL(u32_Cur_Frequency, u16_Cur_EId, u8_Cur_ECC, u16_Bestindex, u32_previousfreq);
						
					}
				}
				else
				{
					/* For MISRA */
				}
			}
		}
		e_CreateSTLStatus =	REPLYSTATUS_SUCCESS ;
	}
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Number of stations in APP is %d ", st_DAB_App_StationList.u8_NoOfStationsInList)	;
	return e_CreateSTLStatus ;
}


#endif
void DAB_App_Update_TunerCtrlLayer_EnsembleInfoBuffer(void)
{
	Tu8 u8_StationIndex = 0;
	
	Tu8 u8_EnsembleIndex = 0;
	
	Tbool b_Frequency_Stored = FALSE;
	
	/* Clearing no. of ensemble */
	ensembleIndex  = 0 ;
	/* Clearing ast_EnsembleInfo buffer  */
	memset(ast_EnsembleInfo,0,sizeof(Ts_Tuner_Ctrl_EnsembleInfo) * DAB_APP_MAX_ENSEMBLES);
	
	for(u8_StationIndex = 0 ; (u8_StationIndex <  st_DAB_App_StationList.u8_NoOfStationsInList && ensembleIndex < DAB_APP_MAX_ENSEMBLES && st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_Frequency !=0) ; u8_StationIndex++)
	{
		b_Frequency_Stored = FALSE;
		for(u8_EnsembleIndex = 0; u8_EnsembleIndex < ensembleIndex ; u8_EnsembleIndex++)
		{
			if((ast_EnsembleInfo[u8_EnsembleIndex].st_BasicEnsInfo.u32_Frequency 	== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_Frequency) &&
				(ast_EnsembleInfo[u8_EnsembleIndex].st_BasicEnsInfo.u16_EId 		== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u16_EId))
				{
					b_Frequency_Stored = TRUE;
					break;
				}
		}
		
		if(b_Frequency_Stored == FALSE)
		{
			ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u32_Frequency 	= st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_Frequency ;
			ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u16_EId 		= st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u16_EId ;
			ast_EnsembleInfo[ensembleIndex].st_BasicEnsInfo.u8_ECC 			= st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u8_ECC ;
			ensembleIndex += 1 ;
		}
	}	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Ensmeble count is %d ", ensembleIndex);	
}


void DAB_App_Update_TunerCtrlLayer_ServiceInfoBuffer(void)
{
	Tu8 u8_ECC = 0, u8_StationIndex = 0 ;
	Tu16 u16_EnsembleIndex = 0 , u16_EId = 0 ;
	Tu32 u32_Frequency = 0 ;
	Tu8 u8_Serviceindex = 0;
	Tbool b_Service_Stored = FALSE;
	
	/* Clearing no. of services */
	Services = 0 ; 
	/* Clearing ast_ServiceInfo buffer  */
	memset(ast_ServiceInfo,0,sizeof(Ts_Tuner_Ctrl_ServiceInfo) * DAB_APP_MAX_SERVICES );
	
	/* Checking if valid data is present or not */
		for(u16_EnsembleIndex = 0 ; u16_EnsembleIndex < ensembleIndex ; u16_EnsembleIndex++)
		{
			u32_Frequency 	= ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u32_Frequency ;
			u16_EId 		= ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u16_EId ;
			u8_ECC 			= ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo.u8_ECC ;
			
			for(u8_StationIndex = 0 ; (u8_StationIndex < st_DAB_App_StationList.u8_NoOfStationsInList) && (Services < DAB_APP_MAX_SERVICES); u8_StationIndex++)
			{
				b_Service_Stored = FALSE;
				/* If Frequency and EId are matched then adding the services into ast_ServiceInfo buffer */
				if((u32_Frequency 	== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_Frequency) && 
					(u16_EId 		== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u16_EId) &&
					(u8_ECC 		== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u8_ECC))
				{
					for(u8_Serviceindex = 0; u8_Serviceindex < Services ; u8_Serviceindex++)
					{
						if(ast_ServiceInfo[u8_Serviceindex].u32_SId == st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_SId && ast_ServiceInfo[u8_Serviceindex].st_BasicEnsInfo.u32_Frequency == st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_Frequency
							&& ast_ServiceInfo[u8_Serviceindex].st_BasicEnsInfo.u16_EId == st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u16_EId)
						{
							b_Service_Stored = TRUE;
							break;	
						}	
					}	
					
					if(b_Service_Stored == FALSE)
					{
						ast_ServiceInfo[Services].st_BasicEnsInfo = ast_EnsembleInfo[u16_EnsembleIndex].st_BasicEnsInfo ;
						ast_ServiceInfo[Services].u32_SId = st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_SId ;
						SYS_RADIO_MEMCPY(ast_ServiceInfo[Services].st_ServiceLabel.au8_label,st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].st_ServiceLabel,DAB_APP_MAX_LABEL_LENGTH);
						ast_ServiceInfo[Services].st_ServiceLabel.u8_CharSet = st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u8_CharSet ;
						Services += 1 ;
					}
				}
				else
				{
					/* Do nothing */
				}/* For MISRA C */
			}
		}	
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Services count is %d ", Services);	
}

void DAB_App_Update_TunerCtrlLayer_ComponentInfoBuffer(void)
{
	Tu8 u8_ECC = 0, u8_StationIndex = 0 ;
	Tu16 u16_ServiceIndex = 0 , u16_EId = 0 ;
	Tu32 u32_Frequency = 0, u32_SId = 0 ;
	
	/* Clearing no. of components */
	Components = 0 ;
	/* Clearing ast_ComponentInfo buffer  */
	memset(ast_ComponentInfo,0,sizeof(Ts_Tuner_Ctrl_ComponentInfo) * DAB_APP_MAX_COMPONENTS);
	
	/* Checking if valid data is present or not */
	if((ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u32_Frequency != 0) && 
		(ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u16_EId != 0) &&
		(ast_ServiceInfo[u16_ServiceIndex].u32_SId != 0))
	{
		for(u16_ServiceIndex = 0 ; u16_ServiceIndex < Services ; u16_ServiceIndex++)
		{
			u32_Frequency	= ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u32_Frequency ;
			u16_EId 		= ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u16_EId ;
			u8_ECC 			= ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo.u8_ECC ;
			u32_SId 		= ast_ServiceInfo[u16_ServiceIndex].u32_SId ;
			
			for(u8_StationIndex = 0 ; (u8_StationIndex < st_DAB_App_StationList.u8_NoOfStationsInList) && (Components < DAB_APP_MAX_COMPONENTS) ; u8_StationIndex++)
			{
				if((u32_Frequency 	== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_Frequency) && 
					(u16_EId 		== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u16_EId) &&
					(u8_ECC 		== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u8_ECC) &&
					(u32_SId 		== st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_SId))
				{  
					ast_ComponentInfo[Components].st_BasicEnsInfo 	= ast_ServiceInfo[u16_ServiceIndex].st_BasicEnsInfo ;
					ast_ComponentInfo[Components].u32_SId 			= st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u32_SId ;
					ast_ComponentInfo[Components].u16_SCIdI 		= st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u16_SCId ;			
					SYS_RADIO_MEMCPY(ast_ComponentInfo[Components].st_compLabel.au8_label,st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].st_ServiceCompLabel,DAB_APP_MAX_LABEL_LENGTH) ;
					ast_ComponentInfo[Components].st_compLabel.u8_CharSet = st_DAB_App_StationList.ast_DAB_Stations[u8_StationIndex].u8_CharSet ;
					Components += 1 ;
				}  
				else
				{
					/* Do nothing */
				}/* For MISRA C */
			}
		}
	}
	else
	{
		/* Do nothing */
	}/* For MISRA C */
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Components count is %d ", Components);	
}



void DAB_App_Update_TunerCtrlLayer_MultiplexInfo()
{
Tu8 u8_Ensembleindex = 0;

Tu8 u8_Ensembleindex1 = 0;


RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[RADIO][DAB_APP] Ensmeble count is %d ", ensembleIndex);

for(u8_Ensembleindex = 0; u8_Ensembleindex < st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList ; u8_Ensembleindex++)
{	
 for(u8_Ensembleindex1 = 0;(u8_Ensembleindex1 < ensembleIndex && ast_ServiceInfo[u8_Ensembleindex1].st_BasicEnsInfo.u32_Frequency!=0);u8_Ensembleindex1++)
 {
 	
		if(st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[u8_Ensembleindex].u16_EId == ast_EnsembleInfo[u8_Ensembleindex1].st_BasicEnsInfo.u16_EId) 
		{
				ast_EnsembleInfo[u8_Ensembleindex1].st_BasicEnsInfo.RSSI =  st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[u8_Ensembleindex].RSSI ;
				
				SYS_RADIO_MEMCPY((ast_EnsembleInfo[u8_Ensembleindex1].st_EnsembleLabel.au8_label),(st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[u8_Ensembleindex].au8_EnsembleLabel),
										DAB_APP_MAX_LABEL_LENGTH );
									
				ast_EnsembleInfo[u8_Ensembleindex1].st_EnsembleLabel.u8_CharSet = st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[u8_Ensembleindex].u8_CharSet;
								
		}
 }		
}

}
/*===========================================================================*/
/*  void DAB_APP_SortSTL											*/
/*===========================================================================*/
void DAB_APP_SortSTL(void)
{
	Ts_DAB_App_Station_Info st_TempStationInfo;
	Tu16 i,j;
		
	for(i = 0 ; i < st_DAB_App_StationList.u8_NoOfStationsInList ; i++)
	{
		for(j = (Tu16)(i+1) ; j < st_DAB_App_StationList.u8_NoOfStationsInList ; j++)
		{
			if(DAB_App_String_comparison((st_DAB_App_StationList.ast_DAB_Stations[i].st_ServiceLabel),
				(st_DAB_App_StationList.ast_DAB_Stations[j].st_ServiceLabel),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) > (Tu8)0)
			{
				st_TempStationInfo = st_DAB_App_StationList.ast_DAB_Stations[i] ;
				st_DAB_App_StationList.ast_DAB_Stations[i] = st_DAB_App_StationList.ast_DAB_Stations[j] ;
				st_DAB_App_StationList.ast_DAB_Stations[j] = st_TempStationInfo ;
			}
			else
			{
				
			}/* For misra c */
		
		}
	}
	
}
/*===========================================================================*/
/*  void DAB_APP_EnsembleSortSTL											*/
/*===========================================================================*/
void DAB_APP_EnsembleSortSTL(void)
{
	Ts_Tuner_Ctrl_MultiplexEnsembleInfo st_TempEnsembleStationInfo;
	
	Tu16 i,j;
		
	for(i = 0 ; i < st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList ; i++)
	{
		for(j = (Tu16)(i+1) ; j < st_DAB_App_MultiplexStationList.u8_NoOfStationsInEnsembleList ; j++)
		{
			if(DAB_App_String_comparison((st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[i].au8_EnsembleLabel),
				(st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[j].au8_EnsembleLabel),DAB_TUNER_CTRL_MAX_LABEL_LENGTH) > (Tu8)0)
			{
				st_TempEnsembleStationInfo = st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[i] ;
				st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[i] = st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[j] ;
				st_DAB_App_MultiplexStationList.st_Tuner_Ctrl_EnsembleInfo[j] = st_TempEnsembleStationInfo ;
			}
			else
			{
				
			}/* For misra c */
		}
	}

	//DAB_APP_SortSTL();
}

Ts32 DAB_App_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size)
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


/*=============================================================================
    end of file
=============================================================================*/