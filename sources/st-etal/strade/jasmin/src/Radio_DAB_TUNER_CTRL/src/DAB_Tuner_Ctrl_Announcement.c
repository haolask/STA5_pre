/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file DAB_Tuner_Ctrl_Announcement.c											                        *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains function definition for handling DAB Announcements	            *
*																											*
*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "DAB_Tuner_Ctrl_Announcement.h"
#include "lib_bitmanip.h"
/*-----------------------------------------------------------------------------
    variables (static)
-----------------------------------------------------------------------------*/
extern Tu16 u16_usermask;

extern Tu16		CIFCount_New;

extern Tbool	b_Anno_ongoingflag ;

extern Tbool	b_Announcement_Cancel_Flag;

extern Tbool	b_Announcement_Stop_Flag;


/*===========================================================================*/
/*   findAnnoServiceInfoentryindatabase                          */
/*===========================================================================*/
void findAnnoServiceInfoentryindatabase(Ts_Service_Info	 *st_Service_Info,Tu8 *u8_Index)
{
	Tu8 entryindex	= 0;

	for (entryindex =0; entryindex < MAX_ENSEMBLE_SERVICES ; entryindex++)
	{
		if(st_Service_Info[entryindex].SId == 0)
		{
			*u8_Index = entryindex;
			break;
		}
	}
}

/*===========================================================================*/
/*   findMatchedSidAnnoServiceInfoentryindatabase                          */
/*===========================================================================*/
void findMatchedSidAnnoServiceInfoentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index)
{
	Tu8 entryindex	= 0;

	for (entryindex =0; ((entryindex < MAX_ENSEMBLE_SERVICES) && (DAB_Tuner_Ctrl_me->st_Service_Info[entryindex].SId != 0)) ; entryindex++)
	{
	
		if(DAB_Tuner_Ctrl_me->st_Service_Info[entryindex].SId == DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId)
		{
			*u8_Index = entryindex;
			break;
		}
	}

}

/*===========================================================================*/
/*   findAnnoentryindatabase                          */
/*===========================================================================*/
void findAnnoentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index)
{
	Tu8 entryindex = 0;

	for (entryindex =0; entryindex < MAX_ENSEMBLE_SERVICES ; entryindex++)
	{
	
		if(DAB_Tuner_Ctrl_me->Announcement_data[entryindex].SId == 0)
		{
			*u8_Index = entryindex;
			break;
		}
	}
}

/*===========================================================================*/
/*   findMatchedSidAnnoentryindatabase                          */
/*===========================================================================*/
void findMatchedSidAnnoentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index)
{
	Tu8 entryindex = 0;

	for (entryindex =0; ((entryindex < MAX_ENSEMBLE_SERVICES) && (DAB_Tuner_Ctrl_me->Announcement_data[entryindex].SId != 0)) ; entryindex++)
	{
	
		if(DAB_Tuner_Ctrl_me->Announcement_data[entryindex].SId == DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.SId)
		{
			*u8_Index = entryindex;
			break;
		}
	}

}


/*===========================================================================*/
/*   check_same_service                          */
/*===========================================================================*/
Tbool check_same_service(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tbool 	b_match 	= FALSE;
	Tu32 	i			= 0;
	Tu32 	j			= 0;

	for(i=0; i< MAX_ENSEMBLE_SERVICES; i++)
	{
		if(DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId == DAB_Tuner_Ctrl_me->st_Service_Info[i].SId)
		{
			memset(&(DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info),0,sizeof(Ts_Service_Info));
		
			DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.SId 					= DAB_Tuner_Ctrl_me->st_Service_Info[i].SId;				
			DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.b_LocalFlag 			= DAB_Tuner_Ctrl_me->st_Service_Info[i].b_LocalFlag;
			DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.CAId 					= DAB_Tuner_Ctrl_me->st_Service_Info[i].CAId;
			DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.NoOfServiceComponents 	= DAB_Tuner_Ctrl_me->st_Service_Info[i].NoOfServiceComponents;
			for ( j=0; j < DAB_Tuner_Ctrl_me->st_Service_Info[i].NoOfServiceComponents; j++)
			{
				DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.TMID[j] 					= DAB_Tuner_Ctrl_me->st_Service_Info[i].TMID[j];
				//if(DAB_Tuner_Ctrl_me->st_Service_Info[i].TMID[j] == 0)
				//{				
					DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.SubchId[j] 				= DAB_Tuner_Ctrl_me->st_Service_Info[i].SubchId[j];
				//}
			}
		    i = MAX_ENSEMBLE_SERVICES; /*to break the loop*/
		}
	}
				
	for ( i=0; i < DAB_MAX_NUM_SUBCHANNELID; i++)
	{
		if(DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info.SubchId[i] == DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId)/*FIG 0/19 SubchId received is checked with the current service SubchId*/
		{
			b_match = TRUE;
			/*To break the loop*/
			i=DAB_MAX_NUM_SUBCHANNELID;			
		}
	}
	return b_match;					
}
/*===========================================================================*/
/*   FindSID_Anno_SubChID                                                      */
/*===========================================================================*/
Tu32 FindSID_Anno_SubChID(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 u8_SubChId)
{
    Tu32    u32_SID = 0;
	Tu32 	i			= 0;
 	Tu32 	j			= 0;
	Tbool	b_matchFound 	= FALSE;
	for(i=0; i< MAX_ENSEMBLE_SERVICES; i++)
	{
			for ( j=0; j < DAB_Tuner_Ctrl_me->st_Service_Info[i].NoOfServiceComponents; j++)
			{
				//if(DAB_Tuner_Ctrl_me->st_Service_Info[i].TMID[j] == 0)
				//{				
					if((DAB_Tuner_Ctrl_me->st_Service_Info[i].SubchId[j] == u8_SubChId))
					{
					   u32_SID = DAB_Tuner_Ctrl_me->st_Service_Info[i].SId;
					   b_matchFound = TRUE;
					   break;
					}
				//}
			}
			if(b_matchFound == TRUE)
			  break;
	}		
	return u32_SID;
}
/*===========================================================================*/
/*   DAB_Tuner_Ctrl_UpdateCurrentSIDAnnoInfo - updation of Announcement support information for the tuned SID */
/*===========================================================================*/
    
void DAB_Tuner_Ctrl_UpdateCurrentSIDAnnoInfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu32 i 		= 0;
	Tu32 j 		= 0;
	
	memset(&(DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info),0,sizeof(Ts_Announcement_Support));
	for(i=0; ((i< MAX_ENSEMBLE_SERVICES) && (DAB_Tuner_Ctrl_me->Announcement_data[i].SId != 0)); i++) /* Ann review :64 is huge for iteration. change it to np. of announcement support information*/
	{

		if(DAB_Tuner_Ctrl_me->Announcement_data[i].SId == DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_SId)
		{
			DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.SId = DAB_Tuner_Ctrl_me->Announcement_data[i].SId;
			DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.AsuFlag = DAB_Tuner_Ctrl_me->Announcement_data[i].AsuFlag;
			DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.Numof_clusters = DAB_Tuner_Ctrl_me->Announcement_data[i].Numof_clusters; 
			for(j=0; j<DAB_Tuner_Ctrl_me->Announcement_data[i].Numof_clusters;j++)
			{
				DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.Clusterid[j] = DAB_Tuner_Ctrl_me->Announcement_data[i].Clusterid[j];
				
			}
		}
		
	}
				
}

/*===========================================================================*/
/*   DAB_Tuner_Ctrl_StoreAnnouncementSwitchingNotificationInfo                          */
/*===========================================================================*/

void DAB_Tuner_Ctrl_StoreAnnouncementSwitchingNotificationInfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 u8_index		 	= 0;
	Tu8 u8_update_index		= 0;
    Tbool	b_New_Anno_data = TRUE;
	
	if(DAB_Tuner_Ctrl_me->b_FirstOccurFlag == FALSE)
	{
		/* Interrupt */
		/* Store Fig 0/19 information, CIF count, Notification_sent_flag and make first occurrence flag as TRUE */
		SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index]), &(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info),sizeof(Ts_Anno_Swtch_Info));
	
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index].u16_Anno_CIFCount = CIFCount_New; 
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index].b_Notification_sent_flag = FALSE;
		if(DAB_Tuner_Ctrl_me->u8_Anno_index < 12)
		{
			DAB_Tuner_Ctrl_me->u8_Anno_index++;
		}
		DAB_Tuner_Ctrl_me->b_FirstOccurFlag = TRUE;
	}
	else
	{
		/* Compare the data with existing data */
		for(u8_index = 0; u8_index < MAX_NUM_OF_ANNO_DATA ; u8_index++)
		{
			//Tu8 u8_cmp = 0 ;
			//u8_cmp = memcmp(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_Anno_index]).st_Anno_Swtch_Info,&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info), sizeof(Ts_Anno_Swtch_Info));
			if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_Clusterid) == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid)
					&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_SubChId) == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId)
					&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u16_AswFlag == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag)))
			{
				b_New_Anno_data = FALSE;
				u8_update_index = u8_index;
				u8_index = MAX_NUM_OF_ANNO_DATA; /* condition to break For loop */
			}
			else
			{
				//b_New_Anno_data = TRUE;
			}
		}
		
		if(b_New_Anno_data == TRUE)
		{
			/* Store New Fig 0/19 information, CIF count, Notification_sent_flag = FALSE */
			SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index]), &(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info),sizeof(Ts_Anno_Swtch_Info));
			
		/*	else
			{
				 No need to store region information
			}
			*/
			DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index].u16_Anno_CIFCount = CIFCount_New;  
			DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index].b_Notification_sent_flag = FALSE;
			if(DAB_Tuner_Ctrl_me->u8_Anno_index < 12)
			{
				DAB_Tuner_Ctrl_me->u8_Anno_index++;
			}
			
		}
		else
		{

			DAB_Tuner_Ctrl_me->CIFCountDiff = (Tu8)(CIFCount_New - DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].u16_Anno_CIFCount);

			if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].b_Notification_sent_flag != TRUE)
			{
				DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].u16_Anno_CIFCount = CIFCount_New;
			}
						
			if(DAB_Tuner_Ctrl_me->CIFCountDiff <= ((Tu16)12)) /* to be chnaged later*/
			{
				
				if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].b_Notification_sent_flag != TRUE)
				{
					/* Set Notification_sent_flag for this particular announcement as TRUE*/
					DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].b_Notification_sent_flag = TRUE;
										
										
					/* Update structure st_DAB_Tuner_Anno_Swtch_Info to be sent to DAB App */
					DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u16_AswFlag = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].st_Anno_Swtch_Info.u16_AswFlag;
					DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_SubChId = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].st_Anno_Swtch_Info.u8_SubChId;
					DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_Clusterid = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_update_index].st_Anno_Swtch_Info.u8_Clusterid;
					DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.e_announcement_type = DAB_Tuner_Ctrl_me->e_announcement_type;
					DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u32_SId = FindSID_Anno_SubChID(DAB_Tuner_Ctrl_me,DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_SubChId);
					

					/* Send Announcement Notification to DAB APP */
					DAB_Tuner_Ctrl_Notify_AnnoStatus(DAB_TUNER_CTRL_ANNO_START,DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info);
					//printf("Sending Announcement Notification to DAB APP");

				}
				else
				{
					/* Dont send notification to DAB APP */
				}
				
			}
			else
			{
				 /* Ignore Interrupt as it is ongoing announcement */
			}
		}
	}
}

/*===========================================================================*/
/*   DAB_Tuner_Ctrl_AnnouncementSwitchingFilter                           */
/*===========================================================================*/
void DAB_Tuner_Ctrl_AnnouncementSwitchingFilter(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 		u8_index			 = 0;
	Tu8      	u8_match_position	 = 0xff;
	/*Tu8 		u8_cmp 				 = 0;*/
	Tbool		b_ClusteridFound_flag  = FALSE;
	Tbool		b_sid_matched 		 = FALSE;
	Tbool		b_ASWinfo_found	     = FALSE;

	if(u16_usermask != 0)
	{	
		if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid == ((Tu8) DAB_TUNER_CTRL_ANNO_CLUSTERID_ALL_ONE))/* Cluster ID - '1111 1111' case*/
		{
			/* Check  Alarm flag is set or not */
			if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag & ((Tu16)DAB_TUNER_CTRL_ALARM_ANNO_SET) & (u16_usermask))
			{
				//RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Cluster id:0xff, Alarm announcement start notification");
				/* Interrupt */
				/* Call function to check and store new announcement information */
				DAB_Tuner_Ctrl_StoreAnnouncementSwitchingNotificationInfo(DAB_Tuner_Ctrl_me);
			}
			else /*   Alarm flag is not set */
			{
				/* No Interrupt (Need to take care if Cluster id is '11111111' for stop case)*/
	#if 1	
				//RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Cluster id:0xff, Alarm announcement stop notification ");
				
				if( (DAB_Tuner_Ctrl_me->u8_Anno_index > 0) && (DAB_Tuner_Ctrl_me->u8_Anno_index < MAX_NUM_OF_ANNO_DATA)) /* only if there is Announcement switching information sent already, then send stop*/
				{
					/* finding and clearing the announcement switching data from announcement switching structure 'st_Anno_Swtch_Notify_Info' of that particular announcement which has stopped  */
					for(u8_index=0; ((u8_index < DAB_Tuner_Ctrl_me->u8_Anno_index) && (b_ASWinfo_found == FALSE) ); u8_index++)
					{
						//Tu8 u8_cmp = 0 ;
						//u8_cmp = memcmp(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_Anno_index].st_Anno_Swtch_Info),&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info), sizeof(Ts_Anno_Swtch_Info));
						if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_Clusterid) == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid)
							&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_SubChId) == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId)
							&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag == ((Tu16)DAB_TUNER_CTRL_NO_ANNO_SET)))
						{
							u8_match_position = u8_index;
							b_ASWinfo_found = TRUE; /* condition to break the For loop*/
						}
						else
						{
							/* Do Nothing */
						}
					}
				
					if(u8_match_position != 0xff)
					{

						if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].b_Notification_sent_flag == TRUE)
						 && (b_Anno_ongoingflag == TRUE))
						{
							/* Call function to update announcement type */
							DAB_Tuner_Ctrl_UpdateAnnouncementType(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info),&(DAB_Tuner_Ctrl_me->e_announcement_type));
							
							/* Update structure st_DAB_Tuner_Anno_Swtch_Info to be sent to DAB App */
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u16_AswFlag = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info.u16_AswFlag;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_SubChId = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info.u8_SubChId;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_Clusterid = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info.u8_Clusterid;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.e_announcement_type = DAB_Tuner_Ctrl_me->e_announcement_type;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u32_SId = FindSID_Anno_SubChID(DAB_Tuner_Ctrl_me,DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_SubChId);
													
							memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position]),0,sizeof(Ts_Anno_Swtch_Notify_Info));
							
							/* Moving the remaining announcement switching data to eliminate empty array element */
							SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position]), &(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position+1]),(12 - u8_match_position - 1)*sizeof(Ts_Anno_Swtch_Notify_Info));
						
							/* Clear last array element */
							memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index]),0,sizeof(Ts_Anno_Swtch_Notify_Info));
							
							if(DAB_Tuner_Ctrl_me->u8_Anno_index > 0)
							{
								DAB_Tuner_Ctrl_me->u8_Anno_index--;
							}
							
							
							/* Send stop announcement request to DAB App*/
							//DAB_App_Request_StopAnnouncement(DAB_Tuner_Ctrl_me->e_announcement_type,DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId);
							/* Send Announcement stop Notification to DAB APP */
							DAB_Tuner_Ctrl_Notify_AnnoStatus(DAB_TUNER_CTRL_ANNO_STOP,DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info);
						}
						else
						{
							/* No need to send stop announcement request to DAB App*/
						}
					}
					else
					{
						/* Do Nothing */
					}
				}
	#endif
			}
		}
		else /* if Cluster ID other than - '1111 1111' case*/
		{
			/* Update  structure st_CurrentSIDAnno_Info */
			DAB_Tuner_Ctrl_UpdateCurrentSIDAnnoInfo(DAB_Tuner_Ctrl_me);
			
			/* Check if ASw is set - All Announcement types are supported */
			if(LIB_AND(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag, u16_usermask)) /* pls modify while testing (Tu16)DAB_TUNER_CTRL_ANNO_SUPPORT_FILTER 0x07ff)*/
			{
				/* Check if Clusterid belongs to current tuned service*/
				for(u8_index=0; u8_index<DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.Numof_clusters;u8_index++)
				{
					if(DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.Clusterid[u8_index] == DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid)
					{
						b_ClusteridFound_flag = TRUE;
						u8_index = DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.Numof_clusters; /* condition to break For loop*/
					}
				}
				
				/* Clusterid belongs to current tuned service */
				if(b_ClusteridFound_flag == TRUE)
				{
					/* Check if Announcement in Asw flag (Fig 0/19) is supported current tuned Sid by comparing with Asu flag in Fig 0/18 data*/
					if(LIB_AND((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag), (DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info.AsuFlag)))
					{
						/* Call function to check and store new announcement information */
						DAB_Tuner_Ctrl_StoreAnnouncementSwitchingNotificationInfo(DAB_Tuner_Ctrl_me);
					}
					else
					{
						/* No Interrupt */
					}
					
				}
				else /* Clusterid doesnt belong to current tuned service */
				{
					
					/* Check if Cluster ID  is  '0000 0000' case */
					if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid == ((Tu8)DAB_TUNER_CTRL_ANNO_CLUSTERID_ALL_ZERO))
					{
					    //RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Cluster id:0x00, Announcement start notification ");
						
						/* Call function to check whether Sid of the Subchid in fig 0/19 is matching with current tuned Sid*/
						b_sid_matched = check_same_service(DAB_Tuner_Ctrl_me);
						
						/* Sid of the Subchid in fig 0/19 is matching with current tuned Sid - Interrupt*/
						if(b_sid_matched == TRUE)
						{
							/* Call function to check and store new announcement information and also which checks the criteria to be satisfied and to Notify DAB App*/
							DAB_Tuner_Ctrl_StoreAnnouncementSwitchingNotificationInfo(DAB_Tuner_Ctrl_me);
						}
						else /* Sid of the Subchid in fig 0/19 is not matching with current tuned Sid */
						{
							/* No Interrupt */
						}
					}
					else /* if Cluster ID other than - '0000 0000' case*/
					{
						/* No Interrupt */
					}
				}
			}
			else /* if ASw is not set */
			{
				//RADIO_DEBUG_LOG (RADIO_LOG_LVL_NOTICE, "[RADIO][DAB_TC] Cluster id!=0xff, Announcement stop notification");
				/* No Interuppt (Need to take care if Cluster id is 0 for stop case) */
		#if 1	
				if( (DAB_Tuner_Ctrl_me->u8_Anno_index > 0) && (DAB_Tuner_Ctrl_me->u8_Anno_index < MAX_NUM_OF_ANNO_DATA)) /* only if there is Announcement switching information sent already, then send stop*/
				{
					/* finding and clearing the announcement switching data from announcement switching structure 'st_Anno_Swtch_Notify_Info' of that particular announcement which has stopped  */
					for(u8_index=0; ((u8_index < DAB_Tuner_Ctrl_me->u8_Anno_index) && (b_ASWinfo_found == FALSE) ); u8_index++)
					{
						//Tu8 u8_cmp = 0 ;
						//u8_cmp = memcmp(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_Anno_index].st_Anno_Swtch_Info),&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info), sizeof(Ts_Anno_Swtch_Info));
						if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_Clusterid) == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid)
							&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_index].st_Anno_Swtch_Info.u8_SubChId) == (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId)
							&& (DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag == 0x0000))
						{
							u8_match_position = u8_index;
							b_ASWinfo_found = TRUE; /* condition to break the For loop*/
						}
						else
						{
							/* Do Nothing */
						}
					}
				
					if(u8_match_position != 0xff)
					{
						if((DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].b_Notification_sent_flag == TRUE)
						 && (b_Anno_ongoingflag == TRUE))
						{
							
							/* Call function to update announcement type */
							DAB_Tuner_Ctrl_UpdateAnnouncementType(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info),&(DAB_Tuner_Ctrl_me->e_announcement_type));
							
							/* Update structure st_DAB_Tuner_Anno_Swtch_Info to be sent to DAB App */
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u16_AswFlag = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info.u16_AswFlag;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_SubChId = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info.u8_SubChId;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_Clusterid = DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position].st_Anno_Swtch_Info.u8_Clusterid;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.e_announcement_type = DAB_Tuner_Ctrl_me->e_announcement_type;
							DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u32_SId = FindSID_Anno_SubChID(DAB_Tuner_Ctrl_me,DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info.u8_SubChId);
							
							memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position]),0,sizeof(Ts_Anno_Swtch_Notify_Info));
							
							/* Moving the remaining announcement switching data to eliminate empty array element */
							SYS_RADIO_MEMCPY(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position]), &(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[u8_match_position+1]),(12 - u8_match_position - 1)*sizeof(Ts_Anno_Swtch_Notify_Info));
						
							/* Clear last array element */
							memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info[DAB_Tuner_Ctrl_me->u8_Anno_index]),0,sizeof(Ts_Anno_Swtch_Notify_Info));
							if(DAB_Tuner_Ctrl_me->u8_Anno_index > 0)
							{		
								DAB_Tuner_Ctrl_me->u8_Anno_index--;
							}		
							
							/* Send stop announcement request to DAB App*/
							//DAB_App_Request_StopAnnouncement(DAB_Tuner_Ctrl_me->e_announcement_type,DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId);
							
							/* Send Announcement stop Notification to DAB APP */
							DAB_Tuner_Ctrl_Notify_AnnoStatus(DAB_TUNER_CTRL_ANNO_STOP,DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info);
						}
						else
						{
							/* No need to send stop announcement request to DAB App*/
						}
					}
					else
					{
						/* Do Nothing */
					}
				}
	#endif
			}
		}
	}
	else
	{
		/* Do nothing*/
	}
}

/*==============================================================================================================================================*/
/*   void DAB_Tuner_Ctrl_UpdateAnnouncementType(Ts_Anno_Swtch_Info *st_Anno_Swtch_Info,Te_DAB_Tuner_Ctrl_announcement_type *e_announcement_type)*/
/*===============================================================================================================================================*/
void DAB_Tuner_Ctrl_UpdateAnnouncementType(Ts_Anno_Swtch_Info *st_Anno_Swtch_Info,Te_DAB_Tuner_Ctrl_announcement_type *e_announcement_type)
{
	switch(st_Anno_Swtch_Info->u16_AswFlag)
	{
		case 0x0001:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_ALARM_ANNO;
			break;
		}

		case 0x0002:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_ROADTRAFFIC_ANNO;
			break;
		}

		case 0x0004:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_TRANSPORT_ANNO;
			break;
		}
		
		case 0x0008:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_WARNING_ANNO;
			break;
		}
		
		case 0x0010:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_NEWS_ANNO;
			break;
		}

		case 0x0020:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_WEATHER_ANNO;
			break;
		}

		case 0x0040:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_EVENT_ANNO;
			break;
		}

		case 0x0080:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_SPECIAL_ANNO;
			break;
		}

		case 0x0100:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_PROGRAMME_ANNO;
			break;
		}

		case 0x0200:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_SPORT_ANNO;
			break;
		}

		case 0x0400:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_FINANCIAL_ANNO;
			break;
		}

		default:
		{	
			*e_announcement_type = DAB_TUNER_CTRL_ANNO_TYPE_INVALID;
			break;
		}
	}
}

/*===========================================================================*/
/*                  DAB_Tuner_Ctrl_Update_CurrentTunedAnnoData              */
/*===========================================================================*/
void DAB_Tuner_Ctrl_Update_CurrentTunedAnnoData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	Tu8 Searchindex = 0;
	Tbool b_Sidmatched = FALSE;
	Tu8 j = 0;
	for(j = 0; ((j < MAX_ENSEMBLE_SERVICES) && (b_Sidmatched != TRUE)); j++)
	{
		Tu8 i = 0;
		for (i=0;(( i < DAB_MAX_NUM_SUBCHANNELID) && (i < DAB_Tuner_Ctrl_me->st_Service_Info[j].NoOfServiceComponents)); i++)
		{
			if(DAB_Tuner_Ctrl_me->st_Service_Info[j].SubchId[i] == DAB_Tuner_Ctrl_me->u8_Subchannelid)/*FIG 0/19 SubchId received is checked with the current service SubchId*/
			{
				DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.u32_SId = DAB_Tuner_Ctrl_me->st_Service_Info[j].SId;
				DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.u32_Frequency = DAB_Tuner_Ctrl_me->st_currentEnsembleData.u32_Frequency;
				DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.u16_EId 		= DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.EnsembleIdentifier;
				DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.u8_ECC 		= DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.ECC;
				for(Searchindex = 0; ((Searchindex < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) && (b_Sidmatched != TRUE)); Searchindex ++)
				{
					if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].ProgServiceId	==  DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.u32_SId)
					{
						DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.u16_SCIdI = DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[0].InternalCompId;
						SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.service_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.servicecomponent_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].st_compInfo.Component[0].LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						SYS_RADIO_MEMCPY((DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.Ensemble_label.au8_label),(DAB_Tuner_Ctrl_me->st_GetEnsembleProperties_reply.LabelString),DAB_TUNER_CTRL_MAX_LABEL_LENGTH);
						b_Sidmatched = TRUE;
						if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].CharSet == 0x00)
						{
							DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_EBU;
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].CharSet == 0x06)
						{
							DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UCS2;
						}
						else if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[Searchindex].CharSet == 0x0f)
						{
							DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo.service_label.u8_CharSet = DAB_TUNER_CTRL_CHARSET_UTF8;	
						}
						else
						{
						/*Doing nothing */ 	
						}/* For MISRA C */	
					}
				}
			}		
		}
	}
}


/*===========================================================================*/
/*   DAB_Tuner_Ctrl_ClearingAnnoDatabases                                   */
/*===========================================================================*/
void DAB_Tuner_Ctrl_ClearingAnnoDatabases(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	//b_Anno_Onstate			= FALSE;
	//b_Anno_ongoingflag 			= FALSE;
	//b_Announcement_Cancel_Flag 	= FALSE;
	//b_Announcement_Stop_Flag 		= FALSE;
	DAB_Tuner_Ctrl_me->CIFCountDiff	= 0;	
	DAB_Tuner_Ctrl_me->b_FirstOccurFlag = FALSE;
	DAB_Tuner_Ctrl_me->u8_Anno_index = 0;

	memset(&(DAB_Tuner_Ctrl_me->st_Temp_Anno_Info),0,sizeof(Ts_Announcement_Support));
	memset(DAB_Tuner_Ctrl_me->Announcement_data,0,(sizeof(Ts_Announcement_Support) * MAX_ENSEMBLE_SERVICES));
	memset(&(DAB_Tuner_Ctrl_me->st_CurrentSIDAnno_Info),0,sizeof(Ts_Announcement_Support));
	memset(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Notify_Info,0,(sizeof(Ts_Anno_Swtch_Notify_Info) * 12));
	memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info),0,sizeof(Ts_Anno_Swtch_Info));
	memset(&(DAB_Tuner_Ctrl_me->st_DAB_Tuner_Anno_Swtch_Info),0,sizeof(Ts_DAB_Tuner_Anno_Swtch_Info));
	memset(&(DAB_Tuner_Ctrl_me->st_CurrentSIDService_Info),0,sizeof(Ts_Service_Info));
	memset(&(DAB_Tuner_Ctrl_me->st_CurrentTunedAnnoInfo),0,sizeof(Ts_Tuner_Ctrl_CurrentEnsembleInfo));	
	
	//RADIO_DEBUG_LOG (RADIO_LOG_LVL_DEBUG, "[RADIO][DAB_TC] Announcement database is cleared");
	//memset(&(DAB_Tuner_Ctrl_me->st_Service_Info),0,(sizeof(Ts_Service_Info) * MAX_ENSEMBLE_SERVICES));
	//memset(&(DAB_Tuner_Ctrl_me->st_Service_Temp_Info),0,sizeof(Ts_Service_Info));
}
/*=============================================================================
    end of file
=============================================================================*/
