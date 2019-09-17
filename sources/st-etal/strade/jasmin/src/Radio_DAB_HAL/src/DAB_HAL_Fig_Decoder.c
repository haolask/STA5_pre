/*==================================================================================================
    start of file
==================================================================================================*/
/************************************************************************************************************/
/** \file DAB_HAL_Fig_Decoder.c																			    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: RADIO_DAB_HAL															     	    *
*  Description			: This file contains FIG related function definitions						        *
*																											*
*																											*
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "DAB_Tuner_Ctrl_main_hsm.h"
#include "dab_app_freq_band.h"
#include "lib_bitmanip.h"
#include "DAB_Tuner_Ctrl_Announcement.h"
#include "DAB_HAL_Fig_Decoder.h"

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/
extern Ts_dab_app_frequency_table dab_app_freq_band_eu[] ;

Tu16	CIFCount_New ;
/*===========================================================================*/
/*   Fig_parsing_ServiceInformation- Fig 0/2                          */
/*===========================================================================*/
void Fig_parsing_ServiceInformation(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{
		
	Tu8 msgindex 			= 0; 
	/*Tu8 Cn 				= 0;*/
	/*Tu8 OE 				= 0;*/
	Tu8 Pd 					= 0;
	Tbool b_DatabaseFlag	= FALSE;
	Tu8 i 					= 0;
	Tu8	j 					= 0;
	Tu8	TMID 				= 0;
	Tu8 position 			= 0xff;
	
	Tu8	u8_totallength		= 0;		/* No need to add in HEW code as this extraction is already taken care in 'Parsing_FIG0()' */
	Tu8 u8_extension = 0xff; /* Variable to store FIG 0 extension*/
	Tbool b_datavalidFlag = FALSE;	/* Variable to store whether FIG data received is valid or not */
	//Tbool b_tmidValidFlag = FALSE;	/* Variable to store whether TMID is valid or not. i.e., TMID = 0 for audio data */
	Tu8 u8_SubchId = 0 ;
	u8_totallength			= (Tu8)(LIB_AND(msg[msgindex], 0x1f));

	msgindex+=1; /* Increment to extract CN PD Extension */
	u8_extension = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	/*Cn = ((msg[msgindex] & (Tu8)0x80) >> 7);*/
	/*OE = ((msg[msgindex] & (Tu8)0x40) >> 6);*/
	Pd =(Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x20, 5));
	msgindex+=1; /*Increment to extract announcement support information */
	
	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&DAB_Tuner_Ctrl_me->u8_FigMinLength);
	
	while((msgindex < u8_totallength) && ((u8_totallength - msgindex) >= DAB_Tuner_Ctrl_me->u8_FigMinLength))
	{
		b_datavalidFlag = FALSE;	/* Variable to store whether FIG data received is valid or not */
		//b_tmidValidFlag = FALSE;	/* Variable to store whether TMID is valid or not. i.e., TMID = 0 for audio data */
		/* Clear temp Service Info structure */
		memset(&(DAB_Tuner_Ctrl_me->st_Service_Temp_Info),0,sizeof(Ts_Service_Info)) ;
		if(Pd == 1)
		{
    		DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId = ((((msg[msgindex] & 0x000000FF) << 24)|((msg[msgindex + 1] & 0x000000FF) << 16) | ((msg[msgindex+2] & 0x000000FF) << 8) | (msg[msgindex+3] & 0x000000FF)));			   	
    		msgindex+=4;
		}
		else
		{
			DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId = LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8u, (msg[msgindex+1] & 0x00FF));
    		msgindex+=2;
		}

		DAB_Tuner_Ctrl_me->st_Service_Temp_Info.b_LocalFlag		 		= (Tbool)(LIB_MASK_AND_SHIFT(msg[msgindex], 0x80, 7));
		DAB_Tuner_Ctrl_me->st_Service_Temp_Info.CAId             		= (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], 0x70, 4)); 
		DAB_Tuner_Ctrl_me->st_Service_Temp_Info.NoOfServiceComponents   = (Tu8)(LIB_AND(msg[msgindex], 0x0F)); 

		for ( j=0; ((j < DAB_Tuner_Ctrl_me->st_Service_Temp_Info.NoOfServiceComponents) && ((u8_totallength - msgindex) >= DAB_TUNER_CTRL_FIG02_MIN_SUBCHID_BYTES) && (j < 12)); j++)
		{
			msgindex+=1;
			TMID        =  (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], 0xC0, 6));
			msgindex+=1;
			DAB_Tuner_Ctrl_me->st_Service_Temp_Info.TMID[j]                 =        TMID;
			//if(DAB_Tuner_Ctrl_me->st_Service_Temp_Info.TMID[j] == 0)
			//{
				/* Set TMID as valid */
				//ebool_tmidValidFlag = E_TRUE ; 
				u8_SubchId = (Tu8) (LIB_MASK_AND_SHIFT(msg[msgindex], 0xFC, 2));
				DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SubchId[j]	= u8_SubchId;
			//}
		}

		msgindex+=1;
	
		/* Check if there is atleast one valid Clusterid and received SID is valid or not to store into database*/
		if((DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId != 0) && (DAB_Tuner_Ctrl_me->st_Service_Temp_Info.NoOfServiceComponents > 0)) //&& (b_tmidValidFlag == TRUE))
		{
			/* Set Flag to indicate data is valid to store into database */
			b_datavalidFlag = TRUE ;
		}
		else
		{
			/* Set Flag to indicate data is invalid to store into database */
			b_datavalidFlag = FALSE ;
		}
	
		if(b_datavalidFlag == TRUE)
		{
			/* whether new data we are receiving */
			for(i=0; ((i< MAX_ENSEMBLE_SERVICES) && (DAB_Tuner_Ctrl_me->st_Service_Info[i].SId != 0));i++)
			{
				if(DAB_Tuner_Ctrl_me->st_Service_Info[i].SId == DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId) // serviceindex should be submiited by i
				{
					b_DatabaseFlag = TRUE;
					break;
				}
			}
			if(b_DatabaseFlag == FALSE)
			{
				findAnnoServiceInfoentryindatabase(DAB_Tuner_Ctrl_me->st_Service_Info,&position);
				DAB_Tuner_Ctrl_me->u8_NoOfServicesIn_ServiceInfo++ ;
			}
			else
			{
				findMatchedSidAnnoServiceInfoentryindatabase(DAB_Tuner_Ctrl_me,&position);
				memset(&(DAB_Tuner_Ctrl_me->st_Service_Info[position]),0,sizeof(Ts_Service_Info));
			}
	
			if(position!=0xff) // where is position defined ?
			{
				DAB_Tuner_Ctrl_me->st_Service_Info[position].SId 					= DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId;
			
				/*the below line is added for testing purpose*/
				//DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[position].ProgServiceId = DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SId;
		
				DAB_Tuner_Ctrl_me->st_Service_Info[position].b_LocalFlag 			= DAB_Tuner_Ctrl_me->st_Service_Temp_Info.b_LocalFlag;

				DAB_Tuner_Ctrl_me->st_Service_Info[position].CAId 					= DAB_Tuner_Ctrl_me->st_Service_Temp_Info.CAId;
		
				DAB_Tuner_Ctrl_me->st_Service_Info[position].NoOfServiceComponents 	= DAB_Tuner_Ctrl_me->st_Service_Temp_Info.NoOfServiceComponents;
		
				for ( j=0; j < DAB_Tuner_Ctrl_me->st_Service_Temp_Info.NoOfServiceComponents; j++)
				{
					DAB_Tuner_Ctrl_me->st_Service_Info[position].TMID[j] = DAB_Tuner_Ctrl_me->st_Service_Temp_Info.TMID[j];
			
					//if(DAB_Tuner_Ctrl_me->st_Service_Temp_Info.TMID[j] == 0)
					//{
						DAB_Tuner_Ctrl_me->st_Service_Info[position].SubchId[j] = DAB_Tuner_Ctrl_me->st_Service_Temp_Info.SubchId[j];
					//}
				}
			}
		}
	}
}


/*===========================================================================*/
/*   Fig_parsing_AnnouncementSupport- FIG 0/18                          */
/*===========================================================================*/
void Fig_parsing_AnnouncementSupport(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{
	Tu8			msgindex 			= 0;
	/*Tu8		Cn					= 0;*/
	/*Tu8 		OE					= 0;*/
	Tu8 		Pd					= 0;
	Tbool 		b_DatabaseFlag 		= FALSE;
	Tbool		b_serviceFound 		= FALSE;
	Tu8 		i					= 0;
	Tu8			position 			= 0xff;
	Tu8 		u8_extension 		= 0xff; /* Variable to store FIG 0 extension*/
	Tu8			u8_totallength		= 0;		/* No need to add in HEW code as this extraction is already taken care in 'Parsing_FIG0()' */
	Tu8 		u8_ValidClusteridCount = 0 ; /* Variable to store valid No of Clusterid's */
	Tbool 		b_datavalidFlag = FALSE;	/* Variable to store whether FIG data received is valid or not */
	Tu8			u8_Clusterid			= 0;
	
	u8_totallength = (Tu8)(LIB_AND(msg[msgindex], 0x1f));	/* Total length of FIG 0/18 */
	
	msgindex++; /* Increment to extract CN PD Extension */
	/*Cn = ((msg[msgindex] & (Tu8)0x80) >> 7);*/
	/*OE = ((msg[msgindex] & (Tu8)0x40) >> 6);*/
	u8_extension = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	Pd = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x20, 5));
	msgindex++; /*Increment to extract announcement support information */
	
	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&(DAB_Tuner_Ctrl_me->u8_FigMinLength));
	
	while((msgindex < u8_totallength) && ((u8_totallength - msgindex) >= DAB_Tuner_Ctrl_me->u8_FigMinLength))
	{
		u8_ValidClusteridCount = 0 ; /* Variable to store valid No of Clusterid's */
		b_datavalidFlag = FALSE;	/* Variable to store whether FIG data received is valid or not */
		
		/* Clear temp Anno support structure */
		memset(&(DAB_Tuner_Ctrl_me->st_Temp_Anno_Info),0,sizeof(Ts_Announcement_Support)) ;
		
		DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.SId = (Tu16)(LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8, (msg[msgindex+1] & 0x00FF)));
		msgindex+=2;
		
		/* Checking whether SID rcvd in Fig0/18  is present in the currrent ensemble*/
		for(i = 0; ((i < DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.u8_NumOfServices) && (i < MAX_ENSEMBLE_SERVICES) && (b_serviceFound == FALSE)); i++)
		{
			if(DAB_Tuner_Ctrl_me->st_GetCurrEnsembleProgListReply.st_serviceinfo[i].ProgServiceId  == DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.SId)
			{
				b_serviceFound = TRUE;
			}
		}
		if(b_serviceFound == TRUE)
		{
			DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.AsuFlag = (Tu16)(LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8, (msg[msgindex+1] & 0x00FF)));
		
			msgindex+=2;

			if(Pd == (Tu8)0) /* For only program services, store the data*/
			{
				DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.Numof_clusters = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x1f));
				
				for(i=0; ((i<DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.Numof_clusters) && ((u8_totallength - msgindex) >= DAB_TUNER_CTRL_FIG018_MIN_CLUSTERID_BYTES));i++)
				{
					msgindex+=1;
					u8_Clusterid = msg[msgindex];
					if(u8_Clusterid != 0)
					{
						u8_ValidClusteridCount++;
						DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.Clusterid[i] = u8_Clusterid;
					}
				}
				msgindex+=1;
				DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.Numof_clusters = u8_ValidClusteridCount ;
				
				/* Check if there is atleast one valid Clusterid and received SID is valid or not to store in database*/
				if((u8_ValidClusteridCount > 0) && (DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.SId != 0))
				{
					/* Set Flag to indicate data is valid to store into database */
					b_datavalidFlag = TRUE ;
				}
			
				if(b_datavalidFlag == TRUE)
				{
					/* whether new data we are receiving */
					for(i=0; ( (i< MAX_ENSEMBLE_SERVICES)  && (b_DatabaseFlag == FALSE)) ;i++)
					{
						if(DAB_Tuner_Ctrl_me->Announcement_data[i].SId == DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.SId)
						{
							/* Announcement support information of Sid already exists in database*/
							b_DatabaseFlag = TRUE;
						}
					}

					if(b_DatabaseFlag == FALSE)
					{
						findAnnoentryindatabase(DAB_Tuner_Ctrl_me,&position);
					}
					else
					{
						findMatchedSidAnnoentryindatabase(DAB_Tuner_Ctrl_me,&position);
					
						/* Clear Announcement support information of Sid in the database as it is old */
						memset(&(DAB_Tuner_Ctrl_me->Announcement_data[position]),0,sizeof(Ts_Announcement_Support));
					}
				
					if(position!=0xff)
					{
						/* Update new announcement support information of Sid in the database */
						DAB_Tuner_Ctrl_me->Announcement_data[position].SId = DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.SId;
						DAB_Tuner_Ctrl_me->Announcement_data[position].AsuFlag = DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.AsuFlag;
						DAB_Tuner_Ctrl_me->Announcement_data[position].Numof_clusters = DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.Numof_clusters;
						for(i=0; i<DAB_Tuner_Ctrl_me->Announcement_data[position].Numof_clusters;i++)
						{
							DAB_Tuner_Ctrl_me->Announcement_data[position].Clusterid[i] = DAB_Tuner_Ctrl_me->st_Temp_Anno_Info.Clusterid[i];
						}
					}
				}
			}
		}
	}
}


/*===========================================================================*/
/*   Fig_parsing_AnnouncementSwitching- FIG 0/19                          */
/*===========================================================================*/
void Fig_parsing_AnnouncementSwitching(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{
  
	/*Tu8		Cn			= 0;*/
	/*Tu8		OE			= 0;*/
	/*Tu8		Pd			= 0;*/
	Tu8 	msgindex 		= 0; 
	/*Cn = ((msg[msgindex] & (Tu8)0x80) >> 7);
	OE = ((msg[msgindex] & (Tu8)0x40) >> 6);
	Pd = ((msg[msgindex] & (Tu8)0x20) >> 5);
	msgindex+=1;
	*/
	
	Tu8 	u8_extension 	= 0xff; /* Variable to store FIG 0 extension*/
	Tu8		u8_totallength		= 0;		/* No need to add in HEW code as this extraction is already taken care in 'Parsing_FIG0()' */
	
	u8_totallength = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	msgindex+=1;
	u8_extension = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	
	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&(DAB_Tuner_Ctrl_me->u8_FigMinLength));

	while((msgindex < u8_totallength) && ((u8_totallength - msgindex) >= DAB_Tuner_Ctrl_me->u8_FigMinLength))
	{
		msgindex+=1; /* Increment to point next byte and Ignoring CN PD Extension*/

		/* Clear temp Anno Switching structure */
		memset(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info),0,sizeof(Ts_Anno_Swtch_Info)) ;
		
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid = msg[msgindex];
		msgindex++;
	
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u16_AswFlag = (Tu16)(LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8, (msg[msgindex+1] & 0x00FF)));
		msgindex+=2;

		DAB_Tuner_Ctrl_UpdateAnnouncementType(&(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info),&(DAB_Tuner_Ctrl_me->e_announcement_type));
		
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.b_NewFlag    = (Tbool)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x80, 7));
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.b_RegionFlag = (Tbool)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x40, 6));
		DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_SubChId    = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x3F));
		
		if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.b_RegionFlag == TRUE)
		{
			msgindex+=1;
			DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_RegionID = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x3F, 5));
			/*msgindex++;*/
		}
	
		/* Check if Clusterid is valid or not to store into database */
		//if(DAB_Tuner_Ctrl_me->st_Anno_Swtch_Info.u8_Clusterid != 0)
		//{
			/* Calling function which checks the criteria to be satisfied and to Notify DAB App*/
		DAB_Tuner_Ctrl_AnnouncementSwitchingFilter(DAB_Tuner_Ctrl_me);
		//}
	}
}

/*================================================================================================*/
/*   Fig_parsing_OE_AnnouncementSupport - FIG 0/25												  */
/*================================================================================================*/
void Fig_parsing_OE_AnnouncementSupport(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{
	Tu8  u8_msgindex		= 0u;
	Tu8  u8_extension		= 0xFF;
	Tu8	 u8_totallength		= 0u;
	Tu8  u8_TempIndex		= 0u;
	Tu16 u16_EID			= 0u;
	Tu8  u8_ValidEIDCount	= 0u;

	u8_totallength = (Tu8)(LIB_AND(msg[u8_msgindex], 0x1f));
	u8_msgindex += 1;
	u8_extension = (Tu8)(LIB_AND(msg[u8_msgindex], 0x1f));

	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension, &(DAB_Tuner_Ctrl_me->u8_FigMinLength));

	while ((u8_msgindex < u8_totallength) && ((u8_totallength - u8_msgindex) >= DAB_Tuner_Ctrl_me->u8_FigMinLength))
	{
		u8_msgindex += 1; /* Increment to point next byte and Ignoring CN PD Extension*/

		/* Clear temp Anno Switching structure */
		memset(&(DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info), 0, sizeof(Ts_OE_Anno_Support));

		DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info.u16_SID = (Tu16)(LIB_CONCATENATE((msg[u8_msgindex] & 0x00FF), 8, (msg[u8_msgindex + 1] & 0x00FF)));
		u8_msgindex += 2;

		DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info.u16_ASU_Flags = (Tu16)(LIB_CONCATENATE((msg[u8_msgindex] & 0x00FF), 8, (msg[u8_msgindex + 1] & 0x00FF)));
		u8_msgindex += 2;

		DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info.u8_Noof_EIDs = (Tu8)(LIB_AND(msg[u8_msgindex], (Tu8)0xF));

		for (u8_TempIndex = 0; ((u8_TempIndex<DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info.u8_Noof_EIDs) && ((u8_totallength - u8_msgindex) >= DAB_TUNER_CTRL_FIG025_MIN_EID_BYTES)); u8_TempIndex++)
		{
			u8_msgindex += 2;
			u16_EID = (Tu16)(LIB_CONCATENATE((msg[u8_msgindex] & 0x00FF), 8, (msg[u8_msgindex + 1] & 0x00FF)));
			if (u16_EID != 0)
			{
				u8_ValidEIDCount++;
				DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info.au16_EID[u8_TempIndex] = u16_EID;
			}
		}
		u8_msgindex += 2;
		DAB_Tuner_Ctrl_me->st_OE_Anno_Support_Info.u8_Noof_EIDs = u8_ValidEIDCount;

		/* Function call for DAB APP Notify to be Add later */
	}
}

/*================================================================================================*/
/*   Fig_parsing_OE_AnnouncementSwitching - FIG 0/26											  */
/*================================================================================================*/
void Fig_parsing_OE_AnnouncementSwitching(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{
	Tu8 u8_msgindex = 0u;
	Tu8 u8_extension = 0xFF;
	Tu8	u8_totallength = 0u;

	u8_totallength = (Tu8)(LIB_AND(msg[u8_msgindex], 0x1f));
	u8_msgindex += 1;
	u8_extension = (Tu8)(LIB_AND(msg[u8_msgindex], 0x1f));

	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension, &(DAB_Tuner_Ctrl_me->u8_FigMinLength));

	while ((u8_msgindex < u8_totallength) && ((u8_totallength - u8_msgindex) >= DAB_Tuner_Ctrl_me->u8_FigMinLength))
	{
		u8_msgindex += 1; /* Increment to point next byte and Ignoring CN PD Extension*/

		/* Clear temp Anno Switching structure */
		memset(&(DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info), 0, sizeof(Ts_OE_Anno_Switching));

		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.u8_ClusterID_CE = msg[u8_msgindex];
		u8_msgindex++;

		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.u16_ASW_Flags = (Tu16)(LIB_CONCATENATE((msg[u8_msgindex] & 0x00FF), 8, (msg[u8_msgindex + 1] & 0x00FF)));
		u8_msgindex += 2;

		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.b_New_Flag			= (Tbool)(LIB_MASK_AND_SHIFT(msg[u8_msgindex], (Tu8)0x80, 7));
		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.b_Region_Flag		= (Tbool)(LIB_MASK_AND_SHIFT(msg[u8_msgindex], (Tu8)0x40, 6));
		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.u8_RegionID_CE	= (Tu8)(LIB_AND(msg[u8_msgindex], (Tu8)0x3F));
		u8_msgindex++;

		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.u16_EID_OE = (Tu16)(LIB_CONCATENATE((msg[u8_msgindex] & 0x00FF), 8, (msg[u8_msgindex + 1] & 0x00FF)));
		u8_msgindex += 2;

		DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.u8_ClusterID_OE = msg[u8_msgindex];
		u8_msgindex++;

		if (DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.b_Region_Flag == TRUE)
		{
			DAB_Tuner_Ctrl_me->st_OE_Anno_Switching_Info.u8_RegionID_OE = (Tu8)(LIB_AND(msg[u8_msgindex], (Tu8)0x3F));
		}

		/* Function call for DAB APP Notify to be Add later */
	}
}   

void DAB_Tuner_Ctrl_GetFigMinLength(Tu8 u8_extension,Tu8 *u8_FigMinLength)
{
	switch(u8_extension)
	{
		case 0:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG00_MIN_LENGTH ;
		}
		break;
		
		case 2:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG02_MIN_LENGTH ;
		}
		break;
		
		case 6:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG06_MIN_LENGTH ;
		}
		break;
		
		case 18:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG018_MIN_LENGTH ;
		}
		break;
		
		case 19:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG019_MIN_LENGTH ;
		}
		break;

		case 25:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG025_MIN_LENGTH;
		}
		break;

		case 26:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG026_MIN_LENGTH;
		}
		break;
		
		case 21:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG021_MIN_LENGTH ;
		}
		break;
		
		case 24:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG024_MIN_LENGTH ;
		}
		break;
		
		default:
		{
			*u8_FigMinLength = DAB_TUNER_CTRL_FIG_DEFAULT_MIN_LENGTH ;
		}
		break;
	}
}

/* for checking received data is already in database or not?*/
Tbool findlsn(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 *position)
{
	Tbool b_result = FALSE;
	Tu8 i;
	for(i=0;i<	MAX_LINKAGE_SETS ;i++)
	{
		if((DAB_Tuner_Ctrl_me->hardlinkinfo[i].Ils ==DAB_Tuner_Ctrl_me->linkagedata.Ils) &&
			(DAB_Tuner_Ctrl_me->hardlinkinfo[i].Lsn ==DAB_Tuner_Ctrl_me->linkagedata.Lsn)&&
			(DAB_Tuner_Ctrl_me->hardlinkinfo[i].OE ==DAB_Tuner_Ctrl_me->linkagedata.OE)&&
			(DAB_Tuner_Ctrl_me->hardlinkinfo[i].Pd ==DAB_Tuner_Ctrl_me->linkagedata.Pd))
		{
			*position = (i);
			b_result = TRUE;
			break;
		}
	}
	return b_result;
}





Tbool findlsnpart(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 position,Tu8 *part)
{
	Tbool b_result = FALSE;
	Tu8 partnum =0;
	for(partnum=0; partnum< MAX_LINKAGE_PARTS; partnum++)
	{
		if(DAB_Tuner_Ctrl_me->hardlinkinfo[position].Noofids[partnum] == (Tu8)0)
		{
			*part = partnum;
			b_result = TRUE;
			break;
		}
	}
	return b_result;
}






Tbool findlsnspace(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 *position)
{
	Tbool b_result = FALSE;
	Tu8 i = 0;
	for(i =0;i<MAX_LINKAGE_SETS ; i++)
	{
		if(DAB_Tuner_Ctrl_me->hardlinkinfo[i].Lsn == (Tu8)0)
		{
			*position = i;
			b_result = TRUE;
			break;
		}
	}
	return b_result;
}



void removefromdatabase(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 position)
{
	Tu8 i =0;	
	memset(&(DAB_Tuner_Ctrl_me->hardlinkinfo[position]),0,sizeof(Ts_Lsn_Info));
	for(i =0; i< MAX_LINKAGE_PARTS ; i++)
	{
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Idlq[i] = LINKAGE_INVALID;
	}
}	



void storelsnintodatabase(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me)
{
	
	Tu8 position =0xff;
	Tbool b_result = FALSE;
	Tu8 partnum =0;
	Tu8 part = 0xff;
	Tu8 sids;
		
	if((DAB_Tuner_Ctrl_me->linkagedata.Lsn != (Tu16)0)  && ((DAB_Tuner_Ctrl_me->linkagedata.Idlq == LINKING_DAB_SIDS) || (DAB_Tuner_Ctrl_me->linkagedata.Idlq == LINKING_FM_PIS)))
	{	
		/* checking continuation of database or not */
		if(findlsn(DAB_Tuner_Ctrl_me,&position) == TRUE)
		{
			
			if(DAB_Tuner_Ctrl_me->linkagedata.Cn == (Tu8)1)
			{
				/* if continuation of database then checking for part */
				if(findlsnpart(DAB_Tuner_Ctrl_me,position,&part) == TRUE)
				{
					
				}
				else
				{
					part = 3;	
				}
			}
			else
			{
				/* if not continuation of database then getting same data */
				for(partnum=0; partnum< MAX_LINKAGE_PARTS; partnum++)
				{
					if(DAB_Tuner_Ctrl_me->hardlinkinfo[position].Idlq[partnum] == DAB_Tuner_Ctrl_me->linkagedata.Idlq)
					{
						for(sids = 0; sids <DAB_Tuner_Ctrl_me->linkagedata.Noofids; sids++)
						{
							if(DAB_Tuner_Ctrl_me->hardlinkinfo[position].Sid[partnum][sids] != DAB_Tuner_Ctrl_me->linkagedata.id[sids])
							{
								part = partnum;
								b_result = TRUE;
								break;
							}
						}
						if(b_result == TRUE)
							break;				
					}
				}
			}		
		}
		else
		{
			/* if new data received then find position to add in database */
			if(findlsnspace(DAB_Tuner_Ctrl_me,&position) == TRUE)
			{
				/*if(DAB_Tuner_Ctrl_me->linkagedata.Cn != (Tu8)0) // if the received data is continuation of database then part is 1
					part =1;
				else*/
				part =0;               /* if the received data is not continuation of database then part is 0*/
			}
			else
			{
				memset(&(DAB_Tuner_Ctrl_me->hardlinkinfo[0]),0,sizeof(Ts_Lsn_Info));
				position = 0;
				part = 0;
			}
		}
	}
	if((part != 0xff) && (part < DAB_MAX_LINKAGE_PARTS) && (position < MAX_LINKAGE_SETS))
	{
		/* updating the data into databaase*/	
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Hardlink 		=  DAB_Tuner_Ctrl_me->linkagedata.Hardlink;
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Activelink   	=  DAB_Tuner_Ctrl_me->linkagedata.Activelink;
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Ils		 		=  DAB_Tuner_Ctrl_me->linkagedata.Ils ;	
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Lsn 		    	=  DAB_Tuner_Ctrl_me->linkagedata.Lsn ;
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Idlq[part]   	=  DAB_Tuner_Ctrl_me->linkagedata.Idlq;
		DAB_Tuner_Ctrl_me->hardlinkinfo[position].Noofids[part]	=  DAB_Tuner_Ctrl_me->linkagedata.Noofids ;	
		
		for(sids = 0; sids <DAB_Tuner_Ctrl_me->linkagedata.Noofids; sids++)
		{
			DAB_Tuner_Ctrl_me->hardlinkinfo[position].Sid[part][sids] = DAB_Tuner_Ctrl_me->linkagedata.id[sids];
		}
		DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available = TRUE;	
		//memcpy(&st_ensembleinfo,&st_currentensembledata,sizeof(Ts_basicensembleinfo));
	}	
}

void DAB_Tuner_Ctrl_Change_LA(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 position)
{
	
	Tu8 u8_Sid_Search;	
	if((DAB_Tuner_Ctrl_me->st_Linkingstatus.b_DABHardlinksAvailable == TRUE))
	{	
			for(u8_Sid_Search = 0; u8_Sid_Search < MAX_HARDLINK_SID ; u8_Sid_Search++)
			{
				if(DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_Sid_Search].lsn == DAB_Tuner_Ctrl_me->hardlinkinfo[position].Lsn)
				{
					DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.Hardlink_Sid[u8_Sid_Search].LA	=	DAB_Tuner_Ctrl_me->hardlinkinfo[position].Activelink;	
				}	
			}
	 }	
}


/*=================================================================================================*/
/*  Fig_parsing_Linkinginfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)*/
/*=================================================================================================*/
void Fig_parsing_Linkinginfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{

	Tu8 u8_extension = 0xff; /* Variable to store FIG 0 extension*/
	Tu8 msgindex =0;
	Tu8 position =0;
	Tu8	u8_totallength = 0;
	Tu8 u8_FigMinLength = 0;	/* Variable to store minimum bytes of all FIG's */
	Tu8 i;
	Tu8 u8_ValidSidCount = 0 ; /* Variable to store valid No of Sid's */
	Tu32 u32_sid_temp = 0 ;	/* Variable to store extracted Sid */
	Tu8 u8_Noofids = 0 ;

	u8_totallength = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	msgindex+=1;
	u8_extension = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	DAB_Tuner_Ctrl_me->linkagedata.Cn = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x80, 7));
	DAB_Tuner_Ctrl_me->linkagedata.OE = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x40, 6));
	DAB_Tuner_Ctrl_me->linkagedata.Pd = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x20, 5));

	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&u8_FigMinLength);
	msgindex+=1;
	while((msgindex < u8_totallength) && ((u8_totallength - msgindex) >= u8_FigMinLength))
	{
		u8_ValidSidCount 	= 0 ;
		u8_Noofids = 0 ;
		DAB_Tuner_Ctrl_me->linkagedata.idflag =(Tu8) (LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x80, 7));
		DAB_Tuner_Ctrl_me->linkagedata.Activelink =(Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x40, 6));
		DAB_Tuner_Ctrl_me->linkagedata.Hardlink =(Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x20, 5));
		DAB_Tuner_Ctrl_me->linkagedata.Ils = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x10, 4));
		DAB_Tuner_Ctrl_me->linkagedata.Lsn = (Tu16)(LIB_CONCATENATE((msg[msgindex] & 0x0f), 8u, (msg[msgindex+1] & 0x00ff)));
		msgindex+=2;


		if(DAB_Tuner_Ctrl_me->linkagedata.idflag == 0)			/* checking shortform or longform */
		{
			if(DAB_Tuner_Ctrl_me->linkagedata.Cn == 0)	
			{
				/* change in database or not */
				if(findlsn(DAB_Tuner_Ctrl_me , &position) == TRUE)
					removefromdatabase(DAB_Tuner_Ctrl_me,position);	
			}
			else
			{
				/* change in linkage actuator*/
				if(DAB_Tuner_Ctrl_me->linkagedata.Hardlink  == (Tu8)1)
				{
					DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available = TRUE;	
					if(DAB_Tuner_Ctrl_me->linkagedata.Lsn == DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LSN)
					//	DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA = DAB_Tuner_Ctrl_me->hardlinkinfo[position].Activelink;
						DAB_Tuner_Ctrl_me->st_CurrentSidLinkingInfo.PI_LA = DAB_Tuner_Ctrl_me->linkagedata.Activelink;
					if(findlsn(DAB_Tuner_Ctrl_me,&position) == TRUE)
					{
						DAB_Tuner_Ctrl_me->hardlinkinfo[position].Activelink   	=  DAB_Tuner_Ctrl_me->linkagedata.Activelink;
						DAB_Tuner_Ctrl_Change_LA(DAB_Tuner_Ctrl_me,position);
						DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG06_available = TRUE;	

					}	
				}
			}
		}
		else
		{
			DAB_Tuner_Ctrl_me->linkagedata.Idlq = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x60, (Tu8)5));
			if((DAB_Tuner_Ctrl_me->linkagedata.Pd == (Tu8)0) && (DAB_Tuner_Ctrl_me->linkagedata.Hardlink ==(Tu8)1))
			{
		
				DAB_Tuner_Ctrl_me->linkagedata.Noofids = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x0f));

				for(i=0; ((i<DAB_Tuner_Ctrl_me->linkagedata.Noofids) && ((u8_totallength - msgindex) >= DAB_TUNER_CTRL_FIG06_MIN_SID_BYTES) && (i < 12));i++)
				{
					msgindex+=1;
					/* Extracting only 16 bits as it is program data i.e., pd == 0 */
					u32_sid_temp = (LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8, (msg[msgindex+1] & 0x00FF)));
					if(u32_sid_temp != 0)
					{
						u8_ValidSidCount++;
						DAB_Tuner_Ctrl_me->linkagedata.id[i] = u32_sid_temp;
					}
					msgindex+=1;
				}
				if(u8_ValidSidCount > 0)
				{
					storelsnintodatabase(DAB_Tuner_Ctrl_me);
				}
			}
			else if(DAB_Tuner_Ctrl_me->linkagedata.Pd == (Tu8)1)
			{
				u8_Noofids = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x0f));
				u8_Noofids = u8_Noofids * 4 ;
				msgindex+= u8_Noofids ;
			}
			else if(DAB_Tuner_Ctrl_me->linkagedata.Hardlink  == (Tu8)0)
			{
				u8_Noofids = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x0f));
				u8_Noofids = u8_Noofids * 2 ;
				msgindex+= u8_Noofids ;
			}
			else
			{
				/* FOR MISRA */
			}
		}
	}
}


/*fig 0/21*/
/*===========================================================================*/
/*   DAB_addEntryToFreqInfo                                  */
/*===========================================================================*/
Tbool DAB_addEntryToFreqInfo(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Freq_Info *freqinfo, Tu8 freqPosition)
{

  Tu8   i     = 0u;
  Tbool b_found = FALSE;
  Tbool b_stored = FALSE;
  
  while((i < DAB_MAX_NUM_FREQ_INFO) && (b_found == FALSE))
  {
	  if((DAB_Tuner_Ctrl_me->freqInfo[i].EId == freqinfo->IDField) && (DAB_Tuner_Ctrl_me->freqInfo[i].frequency == freqinfo->Frequency[freqPosition]))
	  {
		  b_found = TRUE;
	  }
	  i++;
  }
  i = 0u;
  while((i < DAB_MAX_NUM_FREQ_INFO) && (b_found == FALSE))
  {
	  if(DAB_Tuner_Ctrl_me->freqInfo[i].EId == DAB_EID_INVALID)
	  {
		  b_found = TRUE;	 
		  DAB_Tuner_Ctrl_me->freqInfo[i].EId           = freqinfo->IDField;
		  DAB_Tuner_Ctrl_me->freqInfo[i].controlField  = freqinfo->ControlField[freqPosition];
		  DAB_Tuner_Ctrl_me->freqInfo[i].frequency     = freqinfo->Frequency[freqPosition];
		  b_stored = TRUE;
	  }
	  i++;
  }
  DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_available = TRUE;
  return b_stored;
}

void Fig_parsing_Frequencyinfo(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,  char *msg)
{
	Ts_Freq_Info     		frequencyInfoList[5];
	Tu8						LenOfFIList;
	Tu8						LenOfFreqList;
	Tu8 					msgindex =0;
	Tu8     				numNewFreqInfo  = 0;
	Tu8     				numFrequencies  = 0;
	Tu8 PI_Check = 0; Tu8 Frequency_check = 0;
	Tbool   				b_stored = FALSE;
	Tu8						u8_totallength		= 0;
	Tu8 					u8_extension = 0xff; /* Variable to store FIG 0 extension*/
	Tu8 					u8_FigMinLength = 0;	/* Variable to store minimum bytes of all FIG's */
 
	memset((void *)frequencyInfoList, 0u, (sizeof(Ts_Freq_Info)* 5u));

	u8_totallength = (Tu8)(LIB_AND(msg[msgindex], 0x1f));	/* Total length of FIG 0/21 */
	msgindex+=1;
	u8_extension = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	frequencyInfoList[numNewFreqInfo].Cn = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x80, 7));
	frequencyInfoList[numNewFreqInfo].OE = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x40, 6));
	frequencyInfoList[numNewFreqInfo].Pd = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x20, 5));
	msgindex+=1;
	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&u8_FigMinLength);
	
	while((msgindex < u8_totallength) && ((u8_totallength - msgindex) >= u8_FigMinLength))
	{
		msgindex+=1;
		frequencyInfoList[numNewFreqInfo].LenOfFIList = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x1F));
		LenOfFIList = frequencyInfoList[numNewFreqInfo].LenOfFIList;
		msgindex+=1;

  		while((LenOfFIList > DAB_FREQ_INFO_HEADER_SIZE) && ((LenOfFIList <= DAB_MAX_NUM_FI_LIST)))
		{
			numFrequencies = 0u;
		    frequencyInfoList[numNewFreqInfo].IDField = (Tu16)(LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8, (msg[msgindex+1] & 0x00FF)));
		    msgindex+=2;
		    frequencyInfoList[numNewFreqInfo].RangeModulation = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0xF0));
		    frequencyInfoList[numNewFreqInfo].b_ContinuityFlag = (Tbool)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x08, 3));
		    frequencyInfoList[numNewFreqInfo].LenOfFreqList = (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x07));
		    LenOfFreqList =  frequencyInfoList[numNewFreqInfo].LenOfFreqList;
		    msgindex+=1;
   
		    LenOfFIList -= DAB_FREQ_INFO_HEADER_SIZE;
			if(LenOfFIList >= LenOfFreqList)
			{
				LenOfFIList -= LenOfFreqList;
				/* At the moment we support DAB ensemble frequencies only.*/
				if(frequencyInfoList[numNewFreqInfo].RangeModulation == DAB_R_AND_M_DAB_ENSEMBLE)
				{
					while(LenOfFreqList >= DAB_FREQ_INFO_DAB_ENTRY_SIZE)
					{
						frequencyInfoList[numNewFreqInfo].ControlField[numFrequencies] = (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], ((Tu8)0xF8), 3));
						frequencyInfoList[numNewFreqInfo].Frequency[numFrequencies] = (((msg[msgindex] & 0x00000007) << 16 ) | ((msg[msgindex+1] & 0x000000FF) << 8) | (msg[msgindex+2] & 0x000000FF));
						msgindex+=3;
						LenOfFreqList -= DAB_FREQ_INFO_DAB_ENTRY_SIZE;
						//if(pTun->ensembleDataInformed == TRUE)
						// 
						/* Store entry to data base (if it was received for the first time) */
						b_stored = DAB_addEntryToFreqInfo(DAB_Tuner_Ctrl_me, &frequencyInfoList[numNewFreqInfo], numFrequencies);
						if(b_stored == TRUE)
						{
						}
						numFrequencies++;
					}
					frequencyInfoList[numNewFreqInfo].numFreq = numFrequencies;
				}
			  	else if(frequencyInfoList[numNewFreqInfo].RangeModulation == FM_RDS_PI)
		   	 	{
					 DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available = TRUE;
					for(PI_Check= 0 ; PI_Check < MAX_HARDLINK_PI; PI_Check++)
					{
						if(DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[PI_Check].PI == frequencyInfoList[numNewFreqInfo].IDField)
					 	{
							 b_stored = TRUE;
						 	break;
					 	}
					}
				   if(b_stored == FALSE)
				   	{	
				   	   	for(PI_Check= 0 ; PI_Check < MAX_HARDLINK_PI; PI_Check++)
				   	  	{
				   			if(DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[PI_Check].PI == 0x00)
							{
								DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[PI_Check].PI = frequencyInfoList[numNewFreqInfo].IDField;	
								DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG021_Hardlink_PI_available = TRUE;
								b_stored = TRUE;
								break;		
							}
					  	}
						if(b_stored == FALSE)
						{
						 	PI_Check = 0;
							DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[PI_Check].PI = frequencyInfoList[numNewFreqInfo].IDField;
					  	}		
				    	Frequency_check= 0;
				    	while(LenOfFreqList > 0 )
				    	{
							//frequency = msg[msgindex] ;
							if((Frequency_check < MAX_ALT_FREQUENCY) && (PI_Check < MAX_HARDLINK_PI))
								DAB_Tuner_Ctrl_me->Hardlink_FM_PI.HL_PI[PI_Check].PI_Freq[Frequency_check++] = msg[msgindex] ;  		 
					    	msgindex += 1;	
					    	LenOfFreqList--;
				    	}
		
					}   
				}
  				else
  				{
		  			/* Unhandled R&M. Continue with next entry */
		  			msgindex += (Tu16)LenOfFreqList;
  				}
  
			}
			numNewFreqInfo++;
		}
  	}   
}



Tbool findOEentry(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu32 u32_SId,Tu8 *u8_Index)
{
	Tbool b_result = FALSE;
	Tu8 Sidindex;	
		
	for(Sidindex =0; Sidindex <DAB_TUNER_CTRL_MAX_NUM_OE_SERVICES; Sidindex++)
	{
		
		if(DAB_Tuner_Ctrl_me->oeServices[Sidindex].u32_SId == u32_SId)
		{
			*u8_Index = Sidindex ;
			b_result  = TRUE;
			break;
		}

	}
	return b_result;
}

void findOEentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index)
{
	Tu8 entryindex ;
	for (entryindex =0; entryindex < DAB_TUNER_CTRL_MAX_NUM_OE_SERVICES ; entryindex++)
	{
		if(DAB_Tuner_Ctrl_me->oeServices[entryindex].u8_numEId == 0)
		{
			*u8_Index = entryindex;
			break;
		}
	}
}


/*===========================================================================*/
/*   DAB_Tuner_Ctrl_ParseOeServ			                                     */
/*===========================================================================*/
void DAB_Tuner_Ctrl_ParseOeServ(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg)
{
  
	Tu32    u32_SId             = 0u;
	Tu16    u16_EId             = 0u;
	Tu8     u8_numOfEIds        = 0u;
	Tu8     u8_EIdMax           = DAB_TUNER_CTRL_OE_SERVICES_MAX_NUM_EID;
	Tu8     Msgindex            = 0u;
  	Tu8     i                 	= 0u;
  	Tu8 	u8_Pd;
  	Tu8 	position 			= 0xff;
	Tu8		u8_totallength		= 0u; 
	Tu8 	u8_extension		= 0xff; /* Variable to store FIG 0 extension*/
	Tu8 	u8_FigMinLength 	= 0;	/* Variable to store minimum bytes of all FIG's */
	Tu8 	u8_ValidEidCount 	= 0 ; /* Variable to store valid No of Eid's */

	u8_totallength = (Tu8)(LIB_AND(msg[Msgindex], 0x1f));	/* Total length of FIG 0/24 */

  	Msgindex++; /* Increment to extract PD Extension */
	u8_extension = (Tu8)(LIB_AND(msg[Msgindex], 0x1f));
  	u8_Pd = (Tu8)(LIB_MASK_AND_SHIFT(msg[Msgindex], 0x20, 5));

   	Msgindex+=1;

	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&u8_FigMinLength);
	
	while((Msgindex < u8_totallength) && ((u8_totallength - Msgindex) >= u8_FigMinLength))
	{
		u8_ValidEidCount 	= 0 ;
		
  		if(u8_Pd == 0u)
		{
			/* Program Service */

		    u32_SId = (LIB_CONCATENATE((msg[Msgindex] & 0x00FF), 8, (msg[Msgindex+1] & 0x00FF)));

			Msgindex+=2;

		    u8_numOfEIds = (Tu8)(LIB_AND(msg[Msgindex], (0x0F))); 
		    if(u8_numOfEIds <= u8_EIdMax)
		    {
			 	if(findOEentry(DAB_Tuner_Ctrl_me,u32_SId,&position) == TRUE)
				{
		
				}
				else
				{
				 	findOEentryindatabase(DAB_Tuner_Ctrl_me,&position);
				}	

			 	if(position!=0xff)
				{	 
					 DAB_Tuner_Ctrl_me->oeServices[position].u32_SId     	= u32_SId;
					 DAB_Tuner_Ctrl_me->oeServices[position].u8_numEId  	= u8_numOfEIds;
					 for(i = 0u; ((i < u8_numOfEIds) && ((u8_totallength - Msgindex) >= DAB_TUNER_CTRL_FIG024_MIN_EID_BYTES)); i++)
					 {
						 Msgindex+=1;
						 u16_EId =  (Tu16)(LIB_CONCATENATE((msg[Msgindex] & 0x00FF), 8, (msg[Msgindex+1] & 0x00FF)));
						 if(u16_EId != 0)
						 {	
							 u8_ValidEidCount++;	
							 /* Verifying if New Eid is found */
							 if(DAB_Tuner_Ctrl_me->oeServices[position].au16_EId[i] != u16_EId)
							 {
								 DAB_Tuner_Ctrl_me->oeServices[position].au16_EId[i] = u16_EId;
							 }		
						 }	
						 else
						 {
							 /* Ignore */
						 }
						 Msgindex+=1;
					 }
					 DAB_Tuner_Ctrl_me->oeServices[position].u8_numEId = u8_ValidEidCount ;
				}
				if(position!=0xff)
				{
				/* Check if there is atleast one valid Eid to store into database*/
				if((u8_ValidEidCount > 0) && (DAB_Tuner_Ctrl_me->oeServices[position].u32_SId != 0))
				{
					/* Data in database is valid */
					DAB_Tuner_Ctrl_me->st_FIG_data_available.b_FIG024_available = TRUE;
				}
				else
				{
					/* Remove invalid data from database */
					memset(&(DAB_Tuner_Ctrl_me->oeServices[position]),0,sizeof(Ts_dab_oeServices));
					}
				}
				else
				{
					
				}/* For MISRA*/
			 }	
		 }
		else
		{
			break;
		}
  	}
}

void Parsing_FIC0(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me,char *msg)
{

	Tu8 u8_extension = 0xff; /* Variable to store FIG 0 extension*/

	u8_extension = (Tu8)(LIB_AND(msg[1], 0x1f));

	switch(u8_extension)
	{
		case 6:
				Fig_parsing_Linkinginfo(DAB_Tuner_Ctrl_me,msg);
			   	break;

		case 24:
				DAB_Tuner_Ctrl_ParseOeServ(DAB_Tuner_Ctrl_me,msg);
				break;
		case 21:
				Fig_parsing_Frequencyinfo(DAB_Tuner_Ctrl_me,msg);
				break;
		case 0:
				Fig_parsing_EnsembleInformation(&(DAB_Tuner_Ctrl_me->st_Ensemble_Info),&(DAB_Tuner_Ctrl_me->b_ReConfigurationFlag),msg);
				break;

		case 2:
				Fig_parsing_ServiceInformation(DAB_Tuner_Ctrl_me,msg);
				break;

		case 18:	
				Fig_parsing_AnnouncementSupport(DAB_Tuner_Ctrl_me,msg);
				break;

		case 19:
				Fig_parsing_AnnouncementSwitching(DAB_Tuner_Ctrl_me,msg);
				break;

		case 25:
				Fig_parsing_OE_AnnouncementSupport(DAB_Tuner_Ctrl_me, msg);
				break;

		case 26:
				Fig_parsing_OE_AnnouncementSwitching(DAB_Tuner_Ctrl_me, msg);
				break;
		default:
			{
				/* Do nothing*/
			}
			break;
	}
}

/*===========================================================================*/
/*   Extract_Figdata                                                         */
/*===========================================================================*/

void Extract_Figdata(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me,char *msg)
{
	Tu8 Type;
	Type = (Tu8)(LIB_AND(msg[0], 0xE0));
	switch(Type)
	{

	case 0 :
			Parsing_FIC0(DAB_Tuner_Ctrl_me,msg);
			break;
	default:
			break;

	}

}
/*===========================================================================*/
/*   Fig_parsing_EnsembleInformation - FIG 0/0                          */
/*===========================================================================*/
void Fig_parsing_EnsembleInformation(Ts_Ensemble_Info *st_Ensemble_Info, Tbool *pb_ReConfigurationFlag,char *msg)
{
	Tu8 msgindex = 0;
	Tu8 u8_extension = 0xff; /* Variable to store FIG 0 extension*/
	Tu8	u8_totallength	= 0;
	Tu8 u8_FigMinLength = 0;	/* Variable to store minimum bytes of all FIG's */		
	
	u8_totallength =(Tu8) (LIB_AND(msg[msgindex], 0x1f));
	
	msgindex+= 1 ;
	u8_extension = (Tu8)(LIB_AND(msg[msgindex], 0x1f));
	msgindex+= 1 ;
	
	/* Clear structure */
	memset(st_Ensemble_Info, 0, sizeof(Ts_Ensemble_Info));
	
	/* Call function to find the corresponding FIG minimum length required to store the data */
	DAB_Tuner_Ctrl_GetFigMinLength(u8_extension,&u8_FigMinLength);
	
	if(u8_totallength >= u8_FigMinLength)
	{	
		st_Ensemble_Info->EId        		= (Tu16)(LIB_CONCATENATE((msg[msgindex] & 0x00FF), 8, (msg[msgindex+1] & 0x00FF)));
		st_Ensemble_Info->EnsembleCountryID 	= (Tu8)(LIB_MASK_AND_SHIFT(st_Ensemble_Info->EId, 0xF000, 12));
		st_Ensemble_Info->EnsembleReference	= (Tu16)(LIB_AND(st_Ensemble_Info->EId, 0x0FFF));
		msgindex+=2;
	
		st_Ensemble_Info->ChangeFlag    	= (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0xC0, 6));
	
		st_Ensemble_Info->AlFlag        	= (Tu8)(LIB_MASK_AND_SHIFT(msg[msgindex], (Tu8)0x20, 5));
		st_Ensemble_Info->CIFCountHigh  	= (Tu8)(LIB_AND(msg[msgindex], (Tu8)0x1F));
		msgindex+=1;
		st_Ensemble_Info->CIFCountLow   	= msg[msgindex];
		msgindex+=1;
		/*CIF count used for Announcement*/
		CIFCount_New =  ((Tu16)(LIB_CONCATENATE(st_Ensemble_Info->CIFCountHigh, 8, st_Ensemble_Info->CIFCountLow)));
	
		st_Ensemble_Info->OccurrenceChange  = msg[msgindex];

		/* Check if Eid id invalid */
		if(st_Ensemble_Info->EId == 0)
		{
			/* Clear structure */
			memset(st_Ensemble_Info, 0, sizeof(Ts_Ensemble_Info));
		}
		else
		{
			/* For MISRA */	
		}

	}

	if(st_Ensemble_Info->ChangeFlag != 0)
	{
		/* Updated 6 seconds before ReConfiguration and cleared after getting ReConfiguration notification in listen handler */
		*pb_ReConfigurationFlag = TRUE ;
	}
	
}

/*=============================================================================
    end of file
=============================================================================*/
