/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file hmi_if_app_notify.c																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: HMI IF															     			*
*  Description			: This file contains API's for notification to HMI IF								*
*																											*
*																											*																											*
*************************************************************************************************************/
/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include <string.h>


#include "hmi_if_extern.h"

#include "hmi_if_app_request.h"

/* added from HMI_Server code*/
//#include "TCP_Server.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

//#include "sw_system_api.h"
#include "radio_mngr_app_inst_hsm.h"
#include "radio_mngr_app_response.h"
#include "debug_log.h"

#ifdef CALIBRATION_TOOL
Tu8	u8_calib_res_buffer_size = 10;
Tu8 au8_HMI_Response_buffer[10];
Tu8 u8_Response_Track_Count = 0;
#endif

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    variables (extern)
-----------------------------------------------------------------------------*/

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
/* void Notify_UpdateCurStationInfo_Display									*/
/*===========================================================================*/
void Notify_UpdateCurStationInfo_Display(Te_Radio_Mngr_App_Band eBand, Tu32 nFreq, Tu8 *stName, Tu8* radioText, Tu8 CharSet, Tu8* ChannelName,Tu8* Prgm_Name, Tu8* Ensemble_Name, Tu8 DAB_Current_Playing_Service, Tu8 DAB_TotalNoofServiceInEnsemble,Tu8 TA,Tu8 TP,Tu16 PI)
{
#ifdef HMI_TCP_PROTOCOL
	Tu8  msg_length= 0;

	switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
		{
			char msg_am_freq[9]={'\0'};//freq_array+id+mode
			char am_freq_array[9]={'\0'};
			unsigned int i=0,j=2;
			msg_am_freq[0]=CURRENTSTATIONINFO_FREQ;
			msg_am_freq[1]='2';
			_itoa(nFreq,am_freq_array,10);
			Info_Extraction(i, j, (char*)&am_freq_array[0], &msg_am_freq[0], msg_length);
			sendtoclient(msg_am_freq);			
		}
        break;

        case RADIO_MNGR_APP_BAND_FM:
		{	
			char msg_fm_stname[30]={'\0'};
			char msg_fm_rt[67]={'\0'};
			char msg_fm_pty[19] = {'\0'};
			char msg_fm_ta[4] = { '\0' };
			char msg_fm_tp[4] = { '\0' };
			char msg_fm_pi[8] = { '\0' };
			char msg_fm_freq[9]={'\0'};//freq_array+id+mode
			char fm_freq_array[8]={'\0'};
			char fm_PI_array[6] = {'\0' };
			unsigned int i=0,j=2;

			msg_fm_freq[0]=CURRENTSTATIONINFO_FREQ;
			msg_fm_freq[1]='1';
			_itoa(nFreq, fm_freq_array, 10);
			for(i=0;i<(strlen(fm_freq_array));i++,j++)
			{
				msg_fm_freq[j]=fm_freq_array[i];
			}
			msg_fm_freq[j]='\0';
			sendtoclient(msg_fm_freq);	

			
			if(stName != NULL)//fm station name
			{
				msg_length = 8;
				j=2;
				msg_fm_stname[0]=CURRENTSTATIONINFO_PSN;
				msg_fm_stname[1]='1';
				Info_Extraction(i, j, (char*)&stName[0], &msg_fm_stname[0], msg_length);
				sendtoclient(msg_fm_stname);
				msg_length = 0;
			}
			else
			{
				//FOR MISRA C
			}

			
			if (radioText != NULL)//fm radio text
			{
				j=2;
				msg_fm_rt[0]=CURRENTSTATIONINFO_RT;
				msg_fm_rt[1]='1';
				Info_Extraction(i, j, (char*)&radioText[0], &msg_fm_rt[0], msg_length);
				sendtoclient(msg_fm_rt);
			}
			else
			{
				//FOR MISRA C
			}

			
			if (Prgm_Name != NULL)//fm program name
			{
				j = 2;
				msg_fm_pty[0] = CURRENTSTATIONINFO_PTY;
				msg_fm_pty[1] = '1';
				Info_Extraction(i, j, (char*)&Prgm_Name[0], &msg_fm_pty[0], msg_length);
				sendtoclient(msg_fm_pty);
			}
			else
			{
				//FOR MISRA C
			}

			//fm programme identification
			j = 2;
			msg_fm_pi[0] = CURRENTSTATIONINFO_PI;
			msg_fm_pi[1] = '1';
			_itoa(PI, fm_PI_array, 10);
			for (i = 0; i < (strlen(fm_PI_array)); i++, j++)
			{
				msg_fm_pi[j] = fm_PI_array[i];
			}
			msg_fm_pi[j] = '\0';
			sendtoclient(msg_fm_pi);
		
			//fm traffic announcement
			msg_fm_ta[0] = CURRENTSTATIONINFO_TA;
			msg_fm_ta[1] = '1';
			msg_fm_ta[2] = (TA + 48);/*converting to ASCII for HMI*/
			sendtoclient(msg_fm_ta);

			//fm traffic programme
			msg_fm_tp[0] = CURRENTSTATIONINFO_TP;
			msg_fm_tp[1] = '1';
			msg_fm_tp[2] = (TP + 48);/*converting to ASCII for HMI*/
			sendtoclient(msg_fm_tp);

		}
        break;

        case RADIO_MNGR_APP_BAND_DAB:
		{
			char msg_dab_stname[35]={'\0'};
			char msg_dab_chname[15]={'\0'};
			char msg_dab_ensemblename[20]={'\0'};
			char msg_dab_rt[131]={'\0'};
			char msg_dab_freq[12]={'\0'};
			char dab_freq_array[9]={'\0'};
			unsigned int i=0,j=2;

			msg_dab_freq[0]=CURRENTSTATIONINFO_FREQ;
			msg_dab_freq[1]='3';
			_itoa(nFreq, dab_freq_array, 10);
			Info_Extraction(i, j, (char*)&dab_freq_array[0], &msg_dab_freq[0], msg_length);
			//sendtoclient(msg_dab_freq);	

			if (ChannelName != NULL)//dab channel name
			{
				j=2;
				msg_dab_chname[0]=CURRENTSTATIONINFO_CHANNELNAME;
				msg_dab_chname[1]='3';
				Info_Extraction(i, j, (char*)&ChannelName[0], &msg_dab_chname[0], msg_length);
				sendtoclient(msg_dab_chname);
			}
			else
			{
				//FOR MISRA C
			}

			if(stName != NULL)//dab station name
			{	
				j=2;
				msg_dab_stname[0]=CURRENTSTATIONINFO_PSN;
				msg_dab_stname[1]= '3';
				Info_Extraction(i, j, (char*)&stName[0], &msg_dab_stname[0], msg_length);
				sendtoclient(msg_dab_stname);
			}
			else
			{
				//FOR MISRA C
			}

			
			if (radioText != NULL)//dab radio text
			{
				j=2;
				msg_dab_rt[0]=CURRENTSTATIONINFO_RT;
				msg_dab_rt[1]='3';
				Info_Extraction(i, j, (char*)&radioText[0], &msg_dab_rt[0], msg_length);
				sendtoclient(msg_dab_rt);
			}
			else
			{
				//FOR MISRA C
			}


		}
		break;
        default:
		{
			//do nothing
		}
		break;
    }	

#else
	memset(&_radioCommonData, 0, sizeof(_radioCommonData));
    switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
		{
			_radioCommonData.nBand = RADIO_MODE_AM;
		}
        break;

        case RADIO_MNGR_APP_BAND_FM:
		{	
			_radioCommonData.nBand = RADIO_MODE_FM;
			if(stName != NULL)
			{
				SYS_RADIO_MEMCPY((void*)_radioCommonData.ServiceCompName,(const void*)stName, RADIO_MAX_CHAN_NAME_SIZE);
			}
			else{/*FOR MISRA C*/}

			if (Prgm_Name != NULL)
			{
				SYS_RADIO_MEMCPY((void*)_radioCommonData.Programme_Type, (const void*)Prgm_Name, RADIO_MAX_SIZE_CHAN_PTYNAME);
			}
			else{/*FOR MISRA C*/ }

			_radioCommonData.TA = TA;
			_radioCommonData.TP = TP;
			_radioCommonData.PI = PI;
		}
        break;

        case RADIO_MNGR_APP_BAND_DAB:
		{
			_radioCommonData.nBand = RADIO_MODE_DAB;
			if(stName != NULL)
			{
				SYS_RADIO_MEMCPY((void*)_radioCommonData.ServiceCompName,(const void*)stName, RADIO_MAX_COMPONENT_LABEL_SIZE);
			}
			else{/*FOR MISRA C*/}
		}
		break;	

        default:
			_radioCommonData.nBand = RADIO_MODE_INVALID;
            break;
    }
	
	/*Copying the band information after linking to be displayed on HMI*/
	_radioCommonData.Aud_Band =  (MODE_TYPE)eBand;
	
	_radioCommonData.Frequency =  nFreq;
	_radioCommonData.Char_set  =  CharSet;
	
	/*Copying currently playing service and Total number of services present in that ensemble*/
	_radioCommonData.DAB_Current_Service 					= DAB_Current_Playing_Service;
	_radioCommonData.DAB_Total_Num_Services_In_Ensemble		= DAB_TotalNoofServiceInEnsemble;
	
	if(_radioCommonData.Frequency == 0)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Zero frequency observed in Foreground: Frequency: %d",_radioCommonData.Frequency);
	}else{/*FOR MISRA C*/}

	if(radioText != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioCommonData.DLS_Radio_Text,(const void*)radioText, RADIO_MAX_DLS_DATA_SIZE);
	}
	else{/*FOR MISRA C*/}
	
	if(ChannelName != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioCommonData.Channel_Name,(const void*)ChannelName, RADIO_MAX_CHANNEL_LABEL_SIZE);
	}
	else{/*FOR MISRA C*/}
	
	/*Copy Ensemble label for Active band DAB*/
	if(Ensemble_Name != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioCommonData.Ensemble_Label,(const void*)Ensemble_Name, RADIO_MAX_ENSEMBLELABEL_LENGTH);
	}
	else{/*FOR MISRA C*/}
	
    if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDataObject);
	}
	else
	{
		/*Do nothing*/
	}/* For MISRA C */

#endif
	
}

#ifdef HMI_TCP_PROTOCOL
void Info_Extraction(unsigned int source_index, unsigned int dest_index, char *source, char *destination, Tu8 length)
{
	if (length == 0)
	{
		for (source_index = 0; source_index < strlen(source); source_index++, dest_index++)
		{
			destination[dest_index] = source[source_index];
		}
		destination[dest_index] = '\0';
	}
	else
	{
		for (source_index = 0; source_index < length; source_index++, dest_index++)
		{
			destination[dest_index] = source[source_index];
		}
		destination[dest_index] = '\0';
	}
}
#endif

/*===========================================================================*/
/* void AMFM_Notify_UpdateCurStationInfo_Diag										         */
/*===========================================================================*/
void AMFM_Notify_UpdateCurStationInfo_Diag(Te_Radio_Mngr_App_Band eBand, Tu32 u32_Freq, Tu32 u32_Quality, Tu8 *StName, Tu16 u16_PI, Tu8 u8_TA, Tu8 u8_TP, Tu8 u8_Char_Set)
{
    memset(_radioAMFMStnInfoData.PServiceName, 0, sizeof(_radioAMFMStnInfoData.PServiceName));

    _radioAMFMStnInfoData.Frequency   = u32_Freq;

    switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
        {
            _radioAMFMStnInfoData.nBand = RADIO_MODE_AM;
        }
        break;

        case  RADIO_MNGR_APP_BAND_FM:
        {
              _radioAMFMStnInfoData.nBand = RADIO_MODE_FM;

            if(StName != NULL)
	        {
                SYS_RADIO_MEMCPY(_radioAMFMStnInfoData.PServiceName, StName, RADIO_MAX_CHAN_NAME_SIZE);
	        }
	        else{/*FOR MISRA C*/}

            _radioAMFMStnInfoData.u16_PI      = u16_PI;

            _radioAMFMStnInfoData.u32_Quality  = u32_Quality;

            _radioAMFMStnInfoData.u8_TA       = u8_TA;

            _radioAMFMStnInfoData.u8_TP       = u8_TP;

            _radioAMFMStnInfoData.u8_Char_Set = u8_Char_Set;

        }
        break;

         default:
         {
             _radioAMFMStnInfoData.nBand = RADIO_MODE_INVALID;
         }
         break;
    }
	
	#ifdef CALIBRATION_TOOL
	au8_HMI_Response_buffer[u8_Response_Track_Count] = 2;
	if(u8_Response_Track_Count < u8_calib_res_buffer_size -1)
	{
		u8_Response_Track_Count++;
	}
	else
	{
		u8_Response_Track_Count = 0;
	}
	#endif
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioAMFMStnInfoObject);
	}else{/* For MISRA C */}
}

/*===========================================================================*/
/* void AMFM_Notify_UpdateFMSTL															*/
/*===========================================================================*/
void AMFM_Notify_UpdateFMSTL_Diag(Ts_Radio_Mngr_App_FM_SL *st_FM_StL, Tu8 u8_numStl)
{
    UINT index = 0;

    noFMStations = u8_numStl;
    _radioCommonData.nBand = RADIO_MODE_FM;
	
	if(st_FM_StL != NULL)
	{
	    for(index = 0; index < u8_numStl; index++)
	    {
			_radioStationListData_FM[index].nBand = RADIO_MODE_FM;
			_radioStationListData_FM[index].Frequency = st_FM_StL->ast_Stations[index].u32_frequency;

			_radioStationListData_FM[index].PI = st_FM_StL->ast_Stations[index].u16_PI;

			_radioStationListData_FM[index].Char_set = st_FM_StL->u8_CharSet;

			memset(_radioStationListData_FM[index].PServiceName, 0, sizeof(_radioStationListData_FM[0].PServiceName));
			SYS_RADIO_MEMCPY(_radioStationListData_FM[index].PServiceName, st_FM_StL->ast_Stations[index].au8_PSN,RADIO_MAX_CHAN_NAME_SIZE);

	    }
	}
	else{/*FOR MISRA C*/}
	
	#ifdef CALIBRATION_TOOL
	au8_HMI_Response_buffer[u8_Response_Track_Count] = 3;
	if(u8_Response_Track_Count < u8_calib_res_buffer_size -1)
	{
		u8_Response_Track_Count++;
	}
	else
	{
		u8_Response_Track_Count = 0;
	}
	#endif
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStaionDataObjectFM);
	}else{/*FOR MISRA C*/}
}

/*===========================================================================*/
/* void AMFM_Notify_UpdateAMSTL															*/
/*===========================================================================*/
void AMFM_Notify_UpdateAMSTL_Diag(Ts_Radio_Mngr_App_AM_SL *st_AM_StL, Tu8 u8_numStl)
{
    UINT index = 0;

    noAMStations = u8_numStl;
    _radioCommonData.nBand = RADIO_MODE_AM;

	if(st_AM_StL != NULL)
	{
	    for(index = 0; index < u8_numStl; index++)
	    {
	        _radioStationListData_AM[index].nBand = RADIO_MODE_AM;
			_radioStationListData_AM[index].Frequency = st_AM_StL->ast_Stations[index].u32_Freq;
	    }
	}
	else{/*FOR MISRA C*/}
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStaionDataObjectAM);
	}else{/*FOR MISRA C*/}
}

/*===========================================================================*/
/* void AMFM_Notify_Quality																*/
/*===========================================================================*/
void AMFM_Notify_Quality_Diag(Te_Radio_Mngr_App_Band eBand, Ts32 s32_RFFieldStrength, Ts32 s32_BBFieldStrength, Tu32 u32_UltrasonicNoise, Tu32 u32_SNR, Tu32 u32_AdjacentChannel, Tu32 u32_CoChannel, Tu32 u32_StereoMonoReception, Tu32 u32_Multipath, Tu32 u32_FrequencyOffset, Tu32 u32_ModulationDetector)
{
#ifdef HMI_TCP_PROTOCOL
	unsigned int i=0; 
	unsigned int j=2;
	char  msg_length= 0;

    switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
        {
			if(s32_RFFieldStrength != '\0')
			{
				char msg_am_RFFS[5]={'0'};
				char am_RFFS_array[4]={'\0'};
				msg_am_RFFS[0]= DIAG_RF_FIELD_STRENGTH;
				msg_am_RFFS[1]= '2';//mode
				_itoa(s32_RFFieldStrength, am_RFFS_array, 10); // 10 is for radix(decimal)
				Info_Extraction(i, j, &am_RFFS_array[0], &msg_am_RFFS[0], msg_length);
				sendtoclient(msg_am_RFFS);
			}
			else
			{
				// for MSIRA C
			}
			//Sleep(200);

			if(s32_BBFieldStrength != '\0')
			{
				char msg_am_BBFS[5]={'0'};
				char am_BBFS_array[4]={'\0'};
				msg_am_BBFS[0]= DIAG_BB_FIELD_STRENGTH;
				msg_am_BBFS[1]= '2';//mode
				_itoa(s32_BBFieldStrength, am_BBFS_array, 10);
				Info_Extraction(i, j, &am_BBFS_array[0], &msg_am_BBFS[0], msg_length);
				sendtoclient(msg_am_BBFS);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(u32_FrequencyOffset != '\0')
			{
				char msg_am_freq_off[5]={'0'};
				char am_freq_off_array[4]={'\0'};
				msg_am_freq_off[0]= DIAG_FREQUENCY_OFFSET;
				msg_am_freq_off[1]= '2';//mode
				_itoa(u32_FrequencyOffset, am_freq_off_array, 10);
				Info_Extraction(i, j, &am_freq_off_array[0], &msg_am_freq_off[0], msg_length);
				sendtoclient(msg_am_freq_off);
			}
			else
			{
				//for MISRA C 
			}
			//Sleep(200);

			if(u32_AdjacentChannel != '\0')
			{
				char msg_am_adj_chnl[5]={'0'};
				char am_adj_chnl_array[4]={'\0'};
				msg_am_adj_chnl[0]= DIAG_ADJACENT_CHANNEL;
				msg_am_adj_chnl[1]= '2';//mode
				_itoa(u32_AdjacentChannel, am_adj_chnl_array, 10);
				Info_Extraction(i, j, &am_adj_chnl_array[0], &msg_am_adj_chnl[0], msg_length);
				sendtoclient(msg_am_adj_chnl);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(u32_ModulationDetector != '\0')
			{
				char msg_am_mod_det[8]={'0'};
				char am_mod_det_array[6]={'\0'};
				msg_am_mod_det[0]= DIAG_MODULATION_DETECTOR;
				msg_am_mod_det[1]= '2';//mode
				_itoa(u32_ModulationDetector, am_mod_det_array, 10);
				Info_Extraction(i, j, &am_mod_det_array[0], &msg_am_mod_det[0], msg_length);
				sendtoclient(msg_am_mod_det);
			}
			else
			{
				//for MISRA C 
			}

        }
        break;

        case RADIO_MNGR_APP_BAND_FM:
        {
			if(s32_RFFieldStrength != '\0')
			{
				char msg_fm_RFFS[5]={'0'};
				char fm_RFFS_array[4]={'\0'};
				msg_fm_RFFS[0]= DIAG_RF_FIELD_STRENGTH;
				msg_fm_RFFS[1]= '1';
				_itoa(s32_RFFieldStrength, fm_RFFS_array, 10);
				Info_Extraction(i, j, &fm_RFFS_array[0], &msg_fm_RFFS[0], msg_length);
				sendtoclient(msg_fm_RFFS);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(s32_BBFieldStrength != '\0')
			{
				char msg_fm_BBFS[5]={'0'};
				char fm_BBFS_array[4]={'\0'};
				msg_fm_BBFS[0]= DIAG_BB_FIELD_STRENGTH;
				msg_fm_BBFS[1]= '1';
				_itoa(s32_BBFieldStrength, fm_BBFS_array, 10);
				Info_Extraction(i, j, &fm_BBFS_array[0], &msg_fm_BBFS[0], msg_length);
				sendtoclient(msg_fm_BBFS);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(u32_FrequencyOffset != '\0')
			{
				char msg_fm_freq_off[5]={'0'};
				char fm_freq_off_array[4]={'\0'};
				msg_fm_freq_off[0]= DIAG_FREQUENCY_OFFSET;
				msg_fm_freq_off[1]= '1';//mode
				_itoa(u32_FrequencyOffset, fm_freq_off_array, 10);
				Info_Extraction(i, j, &fm_freq_off_array[0], &msg_fm_freq_off[0], msg_length);
				sendtoclient(msg_fm_freq_off);
			}
			else
			{
				//for MISRA C 
			}
			//Sleep(200);

			if(u32_Multipath != '\0')
			{
				char msg_fm_multipath[5]={'0'};
				char fm_multipath_array[4]={'\0'};
				msg_fm_multipath[0]= DIAG_MULTIPATH;
				msg_fm_multipath[1]= '1';//mode
				_itoa(u32_Multipath, fm_multipath_array, 10);
				Info_Extraction(i, j, &fm_multipath_array[0], &msg_fm_multipath[0], msg_length);
				sendtoclient(msg_fm_multipath);
			}
			else
			{
				//for MISRA C 
			}
			//Sleep(200);

			if(u32_UltrasonicNoise != '\0')
			{
				char msg_fm_USNoise[5]={'0'};
				char fm_USNoise_array[4]={'\0'};
				msg_fm_USNoise[0]= DIAG_ULTRASONIC_NOISE;
				msg_fm_USNoise[1]= '1';
				_itoa(u32_UltrasonicNoise, fm_USNoise_array, 10);
				Info_Extraction(i, j, &fm_USNoise_array[0], &msg_fm_USNoise[0], msg_length);
				sendtoclient(msg_fm_USNoise);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(u32_SNR != '\0')
			{
				char msg_fm_SNR[5]={'0'};
				char fm_SNR_array[4]={'\0'};
				msg_fm_SNR[0]= DIAG_SNR;
				msg_fm_SNR[1]= '1';
				_itoa(u32_SNR, fm_SNR_array, 10);
				Info_Extraction(i, j, &fm_SNR_array[0], &msg_fm_SNR[0], msg_length);
				sendtoclient(msg_fm_SNR);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(u32_AdjacentChannel != '\0')
			{
				char msg_fm_adj_chnl[5]={'0'};
				char fm_adj_chnl_array[4]={'\0'};
				msg_fm_adj_chnl[0]= DIAG_ADJACENT_CHANNEL;
				msg_fm_adj_chnl[1]= '1';//mode
				_itoa(u32_AdjacentChannel, fm_adj_chnl_array, 10);
				Info_Extraction(i, j, &fm_adj_chnl_array[0], &msg_fm_adj_chnl[0], msg_length);
				sendtoclient(msg_fm_adj_chnl);
			}
			else
			{
				//for MISRA C
			}
			//Sleep(200);

			if(u32_CoChannel != '\0')
			{
				char msg_fm_co_chnl[6]={'0'};
				char fm_co_chnl_array[4]={'\0'};
				msg_fm_co_chnl[0]= DIAG_CO_CHANNEL;
				msg_fm_co_chnl[1]= '1';//mode
				_itoa(u32_CoChannel, fm_co_chnl_array, 10);
				Info_Extraction(i, j, &fm_co_chnl_array[0], &msg_fm_co_chnl[0], msg_length);
				sendtoclient(msg_fm_co_chnl);
			}
			else
			{
				//for MISRA C 
			}
			//Sleep(200);

			if(u32_ModulationDetector != '\0')
			{
				char msg_fm_mod_det[8]={'0'};
				char fm_mod_det_array[6]={'\0'};
				msg_fm_mod_det[0]= DIAG_MODULATION_DETECTOR;
				msg_fm_mod_det[1]= '1';//mode
				_itoa(u32_ModulationDetector, fm_mod_det_array, 10);
				Info_Extraction(i, j, &fm_mod_det_array[0], &msg_fm_mod_det[0], msg_length);
				sendtoclient(msg_fm_mod_det);
			}
			else
			{
				//for MISRA C 
			}

			if(u32_StereoMonoReception != '\0')
			{
				char msg_fm_StereoMono[3]={'0'};
				msg_fm_StereoMono[0]= DIAG_MONO_STEREO;
				msg_fm_StereoMono[1]= '1';//mode
				msg_fm_StereoMono[2]=u32_StereoMonoReception;
				sendtoclient(msg_fm_StereoMono);
			}
			else
			{
				//for MISRA C 
			}
			//Sleep(200);
        }
        break;

        default:
        {
           //for MISRA C
        }
        break;
    }
#else
    switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
        {
            _radioAMFMQualityData.nBand = RADIO_MODE_AM;
        }
        break;

        case RADIO_MNGR_APP_BAND_FM:
        {
            _radioAMFMQualityData.nBand = RADIO_MODE_FM;
        }
        break;

        default:
        {
            _radioAMFMQualityData.nBand = RADIO_MODE_INVALID;
        }
        break;
    }

	_radioAMFMQualityData.s32_RFFieldStrength		= s32_RFFieldStrength;
	_radioAMFMQualityData.s32_BBFieldStrength		= s32_BBFieldStrength;
	_radioAMFMQualityData.u32_UltrasonicNoise		= u32_UltrasonicNoise;
	_radioAMFMQualityData.u32_SNR					= u32_SNR;
	_radioAMFMQualityData.u32_AdjacentChannel		= u32_AdjacentChannel;
	_radioAMFMQualityData.u32_coChannel				= u32_CoChannel;
	_radioAMFMQualityData.u32_StereoMonoReception	= u32_StereoMonoReception;
	_radioAMFMQualityData.u32_Multipath				= u32_Multipath;
	_radioAMFMQualityData.u32_FrequencyOffset		= u32_FrequencyOffset;
	_radioAMFMQualityData.u32_ModulationDetector	= u32_ModulationDetector;

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioAMFMQualityObject);
	}else{/*FOR MISRA C*/}

#endif
}

/*===========================================================================*/
/* void Notify_UpdateCurBand															*/
/*===========================================================================*/
void Notify_UpdateCurRadioMode(Te_RADIO_ReplyStatus replyStatus, Te_Radio_Mngr_App_Band e_activeBand)
{
    if(replyStatus == REPLYSTATUS_SUCCESS)
    {
		_radioStatus.nStatus = RADIO_SELECTBAND_SUCCESS;
		
		/*Updating the Active Band in to the common data, used in GetRadioMode return function*/
		_radioCommonData.Aud_Band = (MODE_TYPE)e_activeBand;
    }
    else
    {
		_radioStatus.nStatus = RADIO_SELECTBAND_FAILURE;
    }

	RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Select Band Response: nBand: %d, Audio_Band: %d, Response: %d", _radioCommonData.nBand, _radioCommonData.Aud_Band, _radioStatus.nStatus);
    if(_pfnNotifyHdlr != NULL)
	{	
    	_pfnNotifyHdlr(&_radioStatusObject);
	}
	else
	{/*Do nothing*/}

    if(_pfnNotifyHdlr != NULL)
	{
		
		_pfnNotifyHdlr(&_radioDataObject);
	}
	else
	{/*Do nothing*/}
}

/*===========================================================================*/
/* void Notify_UpdateStartRadioStatus													*/
/*===========================================================================*/
void Notify_UpdateStartRadioStatus(Te_RADIO_ReplyStatus replyStatus)
{
    if(replyStatus == REPLYSTATUS_SUCCESS)
    {
		_radioStatus.nStatus = RADIO_STARTUP_SUCCESS;
    }
    else
    {
		_radioStatus.nStatus = RADIO_STARTUP_FAILURE;
    }

    if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else
	{/*Do nothing*/}
}

/*===========================================================================*/
/* void Notify_UpdateShutdownTunerStatus												*/
/*===========================================================================*/
void Notify_UpdateShutdownTunerStatus(Te_RADIO_ReplyStatus replyStatus)
{

    if(replyStatus == REPLYSTATUS_SUCCESS)
    {
		_radioStatus.nStatus = RADIO_SHUTDOWN_SUCCESS;
    }
    else
    {
		_radioStatus.nStatus = RADIO_SHUTDOWN_FAILURE;
    }
	 
	 if(_pfnNotifyHdlr != NULL)
	 {
		_pfnNotifyHdlr(&_radioStatusObject);
	 }else{/*For MISRA C*/}
}

/*===========================================================================*/
/* void Notify_UpdateTunerMute															*/
/*===========================================================================*/
void Notify_UpdateTunerMute(Te_Radio_Mngr_App_Band eBand, BOOL muteStatus)
{
    Te_RADIO_ReplyStatus rm_muteReplyStatus;
    Te_RADIO_ReplyStatus rm_demuteReplyStatus;
    //IBaseModule* pModule = NULL;
    //INT32 ret;

	//pModule = (IBaseModule*)GetSysModuleInstance();
	UNUSED(eBand);

	if(muteStatus == RADIO_HMI_IF_DEMUTE)
	{
#if 0
        switch(eBand)
        {
        	case RADIO_MNGR_APP_BAND_AM:
			{
				ret = ((ISystem*)pModule)->SrcReqMute(MODE_TUNER, SUB_MODE_TUNER_AM, REQ_UNMUTE);
        	}
			break;

			case RADIO_MNGR_APP_BAND_FM:
			{
				ret = ((ISystem*)pModule)->SrcReqMute(MODE_TUNER, SUB_MODE_TUNER_FM, REQ_UNMUTE);
			}
			break;

			case RADIO_MNGR_APP_BAND_DAB:
			{
				ret = ((ISystem*)pModule)->SrcReqMute(MODE_TUNER, SUB_MODE_TUNER_DAB, REQ_UNMUTE);
			}
			break;

			default:

			break;
        }
        
        if(ret  == SYS_RET_OK)
        {
            rm_demuteReplyStatus = REPLYSTATUS_SUCCESS;
        }
        else
        {
            rm_demuteReplyStatus = REPLYSTATUS_FAILURE;
        }
#endif
		rm_demuteReplyStatus = REPLYSTATUS_SUCCESS;
		Radio_Mngr_App_Response_DeMute(rm_demuteReplyStatus);
		}
		else
		{
#if 0
        switch(eBand)
        {
        	case RADIO_MNGR_APP_BAND_AM:
			{
				ret = ((ISystem*)pModule)->SrcReqMute(MODE_TUNER, SUB_MODE_TUNER_AM, REQ_MUTE);
        	}
			break;

			case RADIO_MNGR_APP_BAND_FM:
			{
				ret = ((ISystem*)pModule)->SrcReqMute(MODE_TUNER, SUB_MODE_TUNER_FM, REQ_MUTE);
			}
			break;

			case RADIO_MNGR_APP_BAND_DAB:
			{
				ret = ((ISystem*)pModule)->SrcReqMute(MODE_TUNER, SUB_MODE_TUNER_DAB, REQ_MUTE);
			}
			break;

			default:

			break;
        }

        if(ret  == SYS_RET_OK)
        {
            rm_muteReplyStatus = REPLYSTATUS_SUCCESS;
        }
        else
        {
            rm_muteReplyStatus = REPLYSTATUS_FAILURE;
        }
#endif
		rm_muteReplyStatus = REPLYSTATUS_SUCCESS;
		Radio_Mngr_App_Response_Mute(rm_muteReplyStatus);
		}
}


/*===========================================================================*/
/* void DAB_Notify_UpdateStationInfo_Diag													*/
/*===========================================================================*/
void DAB_Notify_UpdateStationInfo_Diag(Tu32 u32_nFreq,Tu16 u16_EId,Tu32 u32_SId,Tu16 u16_SCId,Tu8 *u8_ServiceName, Tu8 CharSet)
{
#ifdef HMI_TCP_PROTOCOL
	unsigned int i=0;
	unsigned int j=2;
	char  msg_length = 0;

	if(u32_nFreq != '\0')
	{
		char msg_dab_freq[10]={'0'};
		char dab_freq_array[8]={'\0'};
		msg_dab_freq[0]= CURRENTSTATIONINFO_FREQ;
		msg_dab_freq[1]= '3';
		_itoa(u32_nFreq, dab_freq_array, 10);// 10 is for radix (decimal)
		Info_Extraction(i, j, &dab_freq_array[0], &msg_dab_freq[0], msg_length);
		sendtoclient(msg_dab_freq);
	}
	else
	{
		//for MISRA C
	}

	if(u16_EId != '\0')
	{
		char msg_dab_EId[9]={'0'};
		char dab_EId_array[7]={'\0'};
		msg_dab_EId[0]= CURRENTSTATIONINFO_EId;
		msg_dab_EId[1]= '3';
		_itoa(u16_EId, dab_EId_array, 10);// 10 is for radix (decimal)
		Info_Extraction(i, j, &dab_EId_array[0], &msg_dab_EId[0], msg_length);
		sendtoclient(msg_dab_EId);
	}
	else
	{
		//for MISRA C
	}
	//Sleep(200);

	if(u32_SId != '\0')
	{
		char msg_dab_SId[9]={'0'};
		char dab_SId_array[7]={'\0'};
		msg_dab_SId[0]= CURRENTSTATIONINFO_SId;
		msg_dab_SId[1]= '3';
		_itoa(u32_SId, dab_SId_array, 10);// 10 is for radix (decimal)
		Info_Extraction(i, j, &dab_SId_array[0], &msg_dab_SId[0], msg_length);
		sendtoclient(msg_dab_SId);
	}
	else
	{
		//for MISRA C
	}
	//Sleep(200);

	if(u16_SCId != '\0')
	{
		char msg_dab_SCId[9]={'0'};
		char dab_SCId_array[7]={'\0'};
		msg_dab_SCId[0]= CURRENTSTATIONINFO_SCId;
		msg_dab_SCId[1]= '3';
		_itoa(u16_SCId, dab_SCId_array, 10);// 10 is for radix (decimal)
		Info_Extraction(i, j, &dab_SCId_array[0], &msg_dab_SCId[0], msg_length);
		sendtoclient(msg_dab_SCId);
	}
	else
	{
		//for MISRA C
	}
	//Sleep(200);
#else
	_radioDABStnInfoData.uEnsembleID  = u16_EId;
	_radioDABStnInfoData.ServiceID    = u32_SId;
	_radioDABStnInfoData.CompID       = u16_SCId;
	_radioDABStnInfoData.Frequency    = u32_nFreq;
	_radioDABStnInfoData.Char_set     = CharSet;
	/*Updating the band information for DAB current station Info Information in diag Mode*/
	_radioDABStnInfoData.eBand		 = RADIO_MODE_DAB;

    memset(_radioDABStnInfoData.ServiceName, 0, sizeof(_radioDABStnInfoData.ServiceName));
	if (u8_ServiceName != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioDABStnInfoData.ServiceName, (const void*)u8_ServiceName, RADIO_MAX_COMPONENT_LABEL_SIZE);
	}else{/*FOR MISRA C*/}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDABStnInfoObject);
	}else{/*For MISRA C*/}

#endif
}

/*===========================================================================*/
/* void DAB_Notify_NormalUpdateStl															*/
/*===========================================================================*/
void DAB_Notify_UpdateNormalStl_Diag(Ts_Radio_Mngr_App_DAB_SL *st_DAB_StL, Tu8 u8_numStl)
{
    INT index = 0;
	Ts32 s32_StringCompare_RetValue 	= RADIO_MNGR_APP_VALUE_ZERO;	
	Tu32 u32_strlen 					= RADIO_MNGR_APP_VALUE_ZERO;
	Tu8  u8_char_loc;
	Tu8  u8_dest_loc ;

    noDABStations = u8_numStl;
    _radioCommonData.nBand = RADIO_MODE_DAB;

	if(st_DAB_StL != NULL)
	{
	    for(index = 0; index < u8_numStl; index++)
	    {
			_radioStationListData_DAB[index].nBand = RADIO_MODE_DAB;
			_radioStationListData_DAB[index].Frequency =  st_DAB_StL->ast_Stations[index].u32_Frequency;
			_radioStationListData_DAB[index].ServiceID = st_DAB_StL->ast_Stations[index].u32_Sid;
			_radioStationListData_DAB[index].ServiceCompID = st_DAB_StL->ast_Stations[index].u16_SCIdI;
			_radioStationListData_DAB[index].uEnsembleID = st_DAB_StL->ast_Stations[index].u16_EId;
			_radioStationListData_DAB[index].Char_set = st_DAB_StL->ast_Stations[index].u8_CharSet;

			memset((void *)(_radioStationListData_DAB[index].Service_ComponentName),(int)0,sizeof(_radioStationListData_DAB[index].Service_ComponentName));
			
			/* copying the service label & component label HMI IF*/			
			for(u8_char_loc = 0,u8_dest_loc = 0 ; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL ; u8_char_loc++)
			{
				if( st_DAB_StL->ast_Stations[index].au8_SrvLabel[u8_char_loc] != RADIO_ASCII_SPACE)
				{
					_radioStationListData_DAB[index].Service_ComponentName[u8_dest_loc] = st_DAB_StL->ast_Stations[index].au8_SrvLabel[u8_char_loc];
					u8_dest_loc++;
				}
				else
				{
					if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL)
					{
						u8_char_loc++;
					}
					else
					{
						/*FOR MISRA C*/
					}
					if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL && st_DAB_StL->ast_Stations[index].au8_SrvLabel[u8_char_loc] == RADIO_ASCII_SPACE)
					{
						u8_char_loc--;
						break;
					}else{/*FOR MISRA C*/}

					u8_char_loc--;
					/*copying the characters from the service label if the second char is not space*/
					_radioStationListData_DAB[index].Service_ComponentName[u8_dest_loc] = st_DAB_StL->ast_Stations[index].au8_SrvLabel[u8_char_loc];
					u8_dest_loc++;
				}
			}

			u32_strlen = SYS_RADIO_STR_LEN(_radioStationListData_DAB[index].Service_ComponentName);
			/*String comparison function to check if both Service and service component Labels are same then no need to merge*/
			s32_StringCompare_RetValue = SYS_RADIO_STR_CMP((Tu8*)(_radioStationListData_DAB[index].Service_ComponentName), 
																(st_DAB_StL->ast_Stations[index].au8_CompLabel),
																(Tu8)u32_strlen);


			/*Comparing the return value of SYS_RADIO_STR_CMP function*/														
			if(s32_StringCompare_RetValue != RADIO_MNGR_APP_VALUE_ZERO)

			{
				/*Apending the service component label into radio manager component name*/
				SYS_RADIO_MEMCPY(&(_radioStationListData_DAB[index].Service_ComponentName[u8_dest_loc]),
																(st_DAB_StL->ast_Stations[index].au8_CompLabel),
																sizeof((st_DAB_StL->ast_Stations[index].au8_CompLabel)));


			}else{/*No Need to copy if service and component lables are same*/}
	    }
	}
	else{/*FOR MISRA C*/}
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStaionDataObjectDAB);
	}else{/*For MISRA C*/}
}

/*===========================================================================*/
/* void DAB_Notify_MultiplexUpdateStl															*/
/*===========================================================================*/
void DAB_Notify_UpdateMultiplexStl_Diag(Ts_Radio_Mngr_App_DAB_MultiplexStationList *st_DAB_MultiplexStl, Tu8 u8_NoOfEnsembleList)
{
	INT index = 0;

    _radioCommonData.nBand = RADIO_MODE_DAB;
	if(st_DAB_MultiplexStl != NULL)
	{
		for(index = 0; index < u8_NoOfEnsembleList; index++)
		{
			_radioStationListData_DAB[index].nBand = RADIO_MODE_DAB;
			_radioStationListData_DAB[index].uEnsembleID = st_DAB_MultiplexStl->ast_EnsembleInfo->u16_EId;
			_radioStationListData_DAB[index].Char_set = st_DAB_MultiplexStl->ast_EnsembleInfo->u8_CharSet;
		}
	}
	else{/*FOR MISRA C*/}
}

/*===========================================================================*/
/* void Radio_Response_PlaySelectStation													*/
/*===========================================================================*/
void Radio_Response_PlaySelectStation(Te_RADIO_ReplyStatus replyStatus)
{
	if(replyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_STNLISTSELECT_REQ_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_STNLISTSELECT_REQ_FAILURE;
	}

    if(_pfnNotifyHdlr != NULL)
	{	
    	_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/* For MISRA C */}
}

/*===========================================================================*/
/* void Radio_Response_SeekStation													*/
/*===========================================================================*/
void Radio_Response_SeekStation(Te_RADIO_ReplyStatus e_SeekReplyStatus)
{
#ifdef HMI_TCP_PROTOCOL
	char Seek_Res_msg[2]={0};
	Seek_Res_msg[0]=SEEK_RESP;
	if(e_SeekReplyStatus == REPLYSTATUS_SUCCESS)
	{
		Seek_Res_msg[1]=REPLYSTATUS_SUCCESS;		
	}
	else if(e_SeekReplyStatus == REPLYSTATUS_NO_SIGNAL)
	{
		Seek_Res_msg[1]=REPLYSTATUS_SUCCESS;		
	}
	else
	{
		Seek_Res_msg[1]=REPLYSTATUS_SUCCESS;		
	}	
#else
	if (e_SeekReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_SEEK_REQ_SUCCESS;
	}
	else if(e_SeekReplyStatus == REPLYSTATUS_NO_SIGNAL)
	{
		_radioStatus.nStatus = RADIO_SEEK_NO_SIGNAL;
	}
	else
	{
		_radioStatus.nStatus = RADIO_SEEK_REQ_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

#endif
}


#if 0
/*===========================================================================*/
/* void Radio_Notify_AddListener														*/
/*===========================================================================*/
void Radio_Response_AddListener(Tu8 ListenerIdx, Radio_ResultCode Status)
{
	if(Status == RADIO_ADDLISTENER_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_ADDLISTENER_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_ADDLISTENER_FAILURE;
	}
	_pfnNotifyHdlr[ListenerIdx](&_radioStatusObject);
}
#endif
/*===========================================================================*/
/* void DAB_Notify_Quality_Diag																*/
/*===========================================================================*/
void DAB_Notify_Quality_Diag(Ts32 s32_RFFieldStrength, Ts32 s32_BBFieldStrength, Tu32 u32_FicBitErrorRatio, Tbool bool_isValidMscBitErrorRatio, Tu32 u32_MscBitErrorRatio)
{
#ifdef HMI_TCP_PROTOCOL
	unsigned int i=0;
	unsigned int j=2;
	char  msg_length = 0;

	if(s32_RFFieldStrength != '\0')
	{
		char msg_dab_RFFS[5]={'0'};
		char dab_RFFS_array[4]={'\0'};
		msg_dab_RFFS[0]= DIAG_RF_FIELD_STRENGTH;
		msg_dab_RFFS[1]= '3';
		_itoa(s32_RFFieldStrength, dab_RFFS_array, 10);// 10 is for radix (decimal)
		Info_Extraction(i, j, &dab_RFFS_array[0], &msg_dab_RFFS[0], msg_length);
		sendtoclient(msg_dab_RFFS);
	}
	else
	{
		//for MISRA C
	}
	//Sleep(200);

	if(s32_BBFieldStrength != '\0')
	{
		char msg_dab_BBFS[5]={'0'};
		char dab_BBFS_array[4]={'\0'};
		msg_dab_BBFS[0]= DIAG_BB_FIELD_STRENGTH;
		msg_dab_BBFS[1]= '3';
		_itoa(s32_BBFieldStrength, dab_BBFS_array, 10);
		Info_Extraction(i, j, &dab_BBFS_array[0], &msg_dab_BBFS[0], msg_length);
		sendtoclient(msg_dab_BBFS);
	}
	else
	{
		//for MISRA C
	}
	//Sleep(200);

	if(u32_FicBitErrorRatio != '\0')
	{
		char msg_dab_FICBER[7]={'0'};
		char dab_FICBER_array[5]={'\0'};
		msg_dab_FICBER[0]= DIAG_FICBER;
		msg_dab_FICBER[1]= '3';
		_itoa(u32_FicBitErrorRatio, dab_FICBER_array, 10);
		Info_Extraction(i, j, &dab_FICBER_array[0], &msg_dab_FICBER[0], msg_length);
		sendtoclient(msg_dab_FICBER);
	}
	else
	{
		//for MISRA C
	}
	//Sleep(200);

	if(u32_MscBitErrorRatio != '\0')
	{
		char msg_dab_MSCBER[7]={'0'};
		char dab_MSCBER_array[5]={'\0'};
		msg_dab_MSCBER[0]= DIAG_MSCBER;
		msg_dab_MSCBER[1]= '3';
		_itoa(u32_MscBitErrorRatio, dab_MSCBER_array, 10);
		Info_Extraction(i, j, &dab_MSCBER_array[0], &msg_dab_MSCBER[0], msg_length);
		sendtoclient(msg_dab_MSCBER);
	}
	else
	{
		//for MISRA C
	}
#else
	_radioDABQualityData.s32_RFFieldStrength		= s32_RFFieldStrength;
	_radioDABQualityData.s32_BBFieldStrength		= s32_BBFieldStrength;
	_radioDABQualityData.u32_FicBitErrorRatio		= u32_FicBitErrorRatio;
	_radioDABQualityData.b_isValidMscBitErrorRatio	= bool_isValidMscBitErrorRatio;
	_radioDABQualityData.u32_MscBitErrorRatio		= u32_MscBitErrorRatio;
	
#if 0
	/*Updating the Audio Decoding Status, Audio Quality, Audio Level for DAB station*/
	_radioDABQualityData.DAB_Audio_Decoding_Status 	= AudioDecodingStatus;
	_radioDABQualityData.DAB_Audio_Quality 			= AudioQuality;
	_radioDABQualityData.DAB_Audio_Level 			= AudioLevel;
#endif
	
	/*Updating the band information for DAB Quality parametrs Information in diag Mode*/
	_radioDABQualityData.eBand		 = RADIO_MODE_DAB;
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDABQualityObject);
	}else{/*For MISRA C*/}

#endif
}
/*===========================================================================*/
/* void AMFM_Notify_AFSwitchStatus												*/
/*===========================================================================*/
void AMFM_Notify_AFSwitchStatus(Te_Radio_Mngr_App_AF_Status e_AF_Notify_Status)
{
	switch(e_AF_Notify_Status)
	{
	case RADIO_MNGR_APP_AF_LIST_AVAILABLE:
		_radioAFSwitchStatusData.Status = RADIO_AF_LIST_AVAILABLE;
		break;

	case RADIO_MNGR_APP_AF_LIST_BECOMES_ZERO:
		_radioAFSwitchStatusData.Status = RADIO_AF_LIST_BECOMES_ZERO;
		break;

	case RADIO_MNGR_APP_AF_LIST_EMPTY:
		_radioAFSwitchStatusData.Status = RADIO_AF_LIST_EMPTY;
		break;

	case RADIO_MNGR_APP_AF_LINK_INITIATED:
		_radioAFSwitchStatusData.Status = RADIO_AF_LINK_INITIATED;
		break;

	case RADIO_MNGR_APP_AF_LINK_ESTABLISHED:
		_radioAFSwitchStatusData.Status = RADIO_AF_LINK_ESTABLISHED;
		break;

	case RADIO_MNGR_APP_DAB_LINK_ESTABLISHED:
		_radioAFSwitchStatusData.Status = RADIO_DAB_LINK_ESTABLISHED;
		break;

	case RADIO_MNGR_APP_NO_LINK:
		_radioAFSwitchStatusData.Status = RADIO_NO_LINK;
		break;

	default:
		/* Do Nothing */
		break;
	}
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioAFSwitchStatusObject);
	}else{/*For MISRA C*/}
}

/*===========================================================================*/
/* void Notify_StationList															*/
/*===========================================================================*/
void Notify_StationList(Te_Radio_Mngr_App_Band e_Band, void *StList, Tu8 numStl, Tu8 Match_Stn_Index)
{
#ifdef HMI_TCP_PROTOCOL
	char stationlist_msg[100]={0};
	stationlist_msg[0] = NOTIFY_STATIONLIST;

	switch(e_Band)
    {
		case RADIO_MNGR_APP_BAND_AM:
		{	
			Tu8 index = 0;
			Tu32 i,j;	
			Ts_Radio_Mngr_App_RadioStationList *StList_AM = (Ts_Radio_Mngr_App_RadioStationList*)StList;

			stationlist_msg[1]='2';//MODE
			stationlist_msg[2]=numStl;//STN COUNT
			for(index = 0; index < numStl; index++)
			{				
				char freq_array[5]={0};
				Tu32 freq = StList_AM->st_AM_StationList.ast_Stations[index].u32_Freq;
				_itoa(freq,freq_array,10);
				for (i = 0,j = 3; i<(strlen(freq_array)); i++, j++)
				{
					stationlist_msg[j]=freq_array[i];
				}
				stationlist_msg[j]='\0';
				Sleep(100);
				sendtoclient(stationlist_msg);	
			}
		}
		break;

		case RADIO_MNGR_APP_BAND_FM:
		{
			Tu8 index = 0;
			Tu32 i,j;	
			Ts_Radio_Mngr_App_RadioStationList *StList_FM = (Ts_Radio_Mngr_App_RadioStationList*)StList;
			stationlist_msg[1]='1';	//MODE
			stationlist_msg[2]=numStl;//STN COUNT
			for(index = 0; index < numStl; index++)
			{	
				if (strlen(StList_FM->st_FM_StationList.ast_Stations[index].au8_PSN) != 0)
				{
					for (i = 0, j = 3; ((i< RADIO_MNGR_APP_CHAN_NAME) && (StList_FM->st_FM_StationList.ast_Stations[index].au8_PSN[i] != '\0')); i++, j++)
					{
						stationlist_msg[j] = StList_FM->st_FM_StationList.ast_Stations[index].au8_PSN[i];
					}
					stationlist_msg[j]='\0';
					Sleep(100);
					sendtoclient(stationlist_msg);
				}
				else
				{
					char freq_array[7]={0};
					Tu32 freq = StList_FM->st_FM_StationList.ast_Stations[index].u32_frequency;
					_itoa(freq,freq_array,10);
					for(i=0,j=3;i<(strlen(freq_array));i++,j++)
					{
						stationlist_msg[j]=freq_array[i];
					}
					stationlist_msg[j]='\0';
					Sleep(100);
					sendtoclient(stationlist_msg);	
				}
			}
		}
		break;

		case RADIO_MNGR_APP_BAND_DAB:
		{
			Tu8  u8_char_loc;
			Tu8  u8_dest_loc ;
			Tu8 index = 0;
			Tu32 i = 0, j = 3;
			Ts_Radio_Mngr_App_DAB_SL *StList_DAB = (Ts_Radio_Mngr_App_DAB_SL*)StList;

			stationlist_msg[1]='3';	//MODE
			stationlist_msg[2]=numStl;//STN COUNT

			if (numStl == 0)
			{
				stationlist_msg[j] = '\0';
				sendtoclient(stationlist_msg);
			}
			else
			{
				for (index = 0; index < numStl; index++)
				{
					if (StList != NULL)
					{
						for (u8_char_loc = 0, u8_dest_loc = 3; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL; u8_char_loc++)
						{
							stationlist_msg[u8_dest_loc] = StList_DAB->ast_Stations[index].au8_SrvLabel[u8_char_loc];
							u8_dest_loc++;
						}
						stationlist_msg[u8_dest_loc] = '\0';
						sendtoclient(stationlist_msg);
					}
				}
			}
		}
		break;

		default:
		{
			//do nothing
		}
		break;
	}		
#else
	UINT8 index = 0;
	Ts32 s32_StringCompare_RetValue 	= RADIO_MNGR_APP_VALUE_ZERO;	
	Tu32 u32_strlen 					= RADIO_MNGR_APP_VALUE_ZERO;
	Tu8  u8_char_loc;
	Tu8  u8_dest_loc ;
	
	memset(_radioStationListData_Display,0,sizeof(_radioStationListData_Display[0]) * MAX_RADIO_STATIONS);
	
	switch(e_Band)
    {
	    case RADIO_MNGR_APP_BAND_AM:
	    {
			Ts_Radio_Mngr_App_RadioStationList *StList_AM = (Ts_Radio_Mngr_App_RadioStationList*)StList;
			noStationsDisplay = numStl;
		    for(index = 0; index < numStl; index++)
		    {
			    _radioStationListData_Display[index].nBand = RADIO_MODE_AM;		    
		    	_radioStationListData_Display[index].Index = index;
				_radioStationListData_Display[index].Frequency = StList_AM->st_AM_StationList.ast_Stations[index].u32_Freq;
		    	memset((void *)_radioStationListData_Display[index].ServiceName, (int)0 , sizeof(_radioStationListData_Display[index].ServiceName));

				if(Match_Stn_Index == index)
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = SET_FLAG;
				}
				else
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = CLEAR_FLAG;
				}

		    }

		    break;
	    }

	    case RADIO_MNGR_APP_BAND_FM:
	    {
			Ts_Radio_Mngr_App_RadioStationList *StList_FM = (Ts_Radio_Mngr_App_RadioStationList*)StList;
			noStationsDisplay = numStl;
		    for(index = 0; index < numStl; index++)
		    {
		    	_radioStationListData_Display[index].nBand = RADIO_MODE_FM;	    
		    	_radioStationListData_Display[index].Index = (UINT8)index;
				_radioStationListData_Display[index].Frequency = StList_FM->st_FM_StationList.ast_Stations[index].u32_frequency;
				_radioStationListData_Display[index].Char_set  = StList_FM->st_FM_StationList.u8_CharSet;

		    	memset((void *)(_radioStationListData_Display[index].ServiceName),(int)0,sizeof(_radioStationListData_Display[index].ServiceName));
				if(StList != NULL)
				{
					SYS_RADIO_MEMCPY((void *)(_radioStationListData_Display[index].ServiceName), (const void*)(StList_FM->st_FM_StationList.ast_Stations[index].au8_PSN), RADIO_MAX_CHAN_NAME_SIZE);
				}
				else{/*FOR MISRA C*/}

				if(Match_Stn_Index == index)
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = SET_FLAG;
				}
				else
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = CLEAR_FLAG;
				}

		    }
		}
		break;

	    case RADIO_MNGR_APP_BAND_DAB:
	    {
			Ts_Radio_Mngr_App_DAB_SL *StList_DAB = (Ts_Radio_Mngr_App_DAB_SL*)StList;
			noStationsDisplay = numStl;
		    for(index = 0; index < numStl; index++)
		    {
		    	_radioStationListData_Display[index].nBand = RADIO_MODE_DAB;
			    _radioStationListData_Display[index].Index = (UINT8)index;
				_radioStationListData_Display[index].Frequency = StList_DAB->ast_Stations[index].u32_Frequency;
				_radioStationListData_Display[index].Char_set = StList_DAB->ast_Stations[index].u8_CharSet;

			    memset((void *)(_radioStationListData_Display[index].ServiceName),(int)0, sizeof(_radioStationListData_Display[index].ServiceName));
				if (StList_DAB != NULL)
				{
					/* copying the service label & component label HMI IF*/			
					for(u8_char_loc = 0,u8_dest_loc = 0 ; u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL ; u8_char_loc++)
					{
						if (StList_DAB->ast_Stations[index].au8_SrvLabel[u8_char_loc] != RADIO_ASCII_SPACE)
						{
							_radioStationListData_Display[index].ServiceName[u8_dest_loc] = StList_DAB->ast_Stations[index].au8_SrvLabel[u8_char_loc];
							u8_dest_loc++;
						}
						else
						{
							if(u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL)
							{
								u8_char_loc++;
							}
							else
							{
								/*FOR MISRA C*/
							}
							if (u8_char_loc < RADIO_MNGR_APP_NUMCHAR_LABEL && StList_DAB->ast_Stations[index].au8_SrvLabel[u8_char_loc] == RADIO_ASCII_SPACE)
							{
								u8_char_loc--;
								break;
							}else{/*FOR MISRA C*/}

							u8_char_loc--;
							/*copying the characters from the service label if the second char is not space*/
							_radioStationListData_Display[index].ServiceName[u8_dest_loc] = StList_DAB->ast_Stations[index].au8_SrvLabel[u8_char_loc];
							u8_dest_loc++;
						}
					}

					u32_strlen = SYS_RADIO_STR_LEN(_radioStationListData_Display[index].ServiceName);
					/*String comparison function to check if both Service and service component Labels are same then no need to merge*/
					s32_StringCompare_RetValue = SYS_RADIO_STR_CMP((Tu8*)(_radioStationListData_Display[index].ServiceName), 
																		(StList_DAB->ast_Stations[index].au8_CompLabel),
																		(Tu8)u32_strlen);


					/*Comparing the return value of SYS_RADIO_STR_CMP function*/														
					if(s32_StringCompare_RetValue != RADIO_MNGR_APP_VALUE_ZERO)

					{
						/*Appending the service component label into radio manager component name*/
						SYS_RADIO_MEMCPY(&(_radioStationListData_Display[index].ServiceName[u8_dest_loc]),
																		(StList_DAB->ast_Stations[index].au8_CompLabel),
																		sizeof((StList_DAB->ast_Stations[index].au8_CompLabel)));
																		
					}else{/*No Need to copy if service and component lables are same*/}
				}
				else{/*FOR MISRA C*/}

				if(Match_Stn_Index == index)
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = SET_FLAG;
				}
				else
				{
					_radioStationListData_Display[index].Matched_Stn_Index_Flag = CLEAR_FLAG;
				}
		    }
	    }
	    break;
	    
		default:
	    {
		    
	    }
		break;
	
	}

	if(_pfnNotifyHdlr != NULL)	
	{
		_pfnNotifyHdlr(&_radioStaionDataObjectDisplay);
	}
	else{/* For MISRA C */}

#endif
}


/*===========================================================================*/
/* void Notify_UpdateScanStatus															*/
/*===========================================================================*/
void Notify_UpdateScanStatus(Te_Radio_Mngr_App_ScanStatus replyStatus)
{
	switch(replyStatus)
	{
		case RADIO_MNGR_APP_SCAN_STARTED:
			_radioStatus.nStatus = RADIO_SCAN_STARTED;
			break;
		case RADIO_MNGR_APP_SCAN_INPROGRESS:
			_radioStatus.nStatus = RADIO_SCAN_INPROGRESS;
			break;
		case RADIO_MNGR_APP_SCAN_COMPLETE:
			_radioStatus.nStatus = RADIO_SCAN_COMPLETE;
			break;

		default:
			break;
	}
		
    if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/* For MISRA C */}	
}


/*===========================================================================*/
/* void Radio_Response_EnableDABFMLinking													*/
/*===========================================================================*/
void Radio_Response_EnableDABFMLinking(Te_RADIO_ReplyStatus replyStatus)
{
	switch(replyStatus)
	{
	case REPLYSTATUS_SUCCESS:
		_radioStatus.nStatus = RADIO_DABFM_BLENDING_REQ_SUCCESS;
		break;
	case REPLYSTATUS_FAILURE:
		_radioStatus.nStatus = RADIO_DABFM_BLENDING_REQ_FAILURE;
		break;
	default:
		/* Do Nothing */
		break;
	}
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}
}


/*===========================================================================*/
/* void AMFM_Notify_AFList_Diag															*/
/*===========================================================================*/
void AMFM_Notify_AFList_Diag(Tu8 Num_AF, Tu32 *AFList, Tu32 *Quality, Tu16 *PIList)
{
	Tu8 index = 0;

    _radioAMFMAFListData.NumAFList = Num_AF;

	_radioAMFMAFListData.nBand = RADIO_MODE_FM;
	if(AFList != NULL && Quality != NULL && PIList != NULL)
	{
		for(index=0; index < Num_AF ; index++)
		{
            _radioAMFMAFListData.AFList[index]  =  AFList[index];

            _radioAMFMAFListData.Quality[index] =  Quality[index];

            _radioAMFMAFListData.PIList[index]  =  PIList[index];
            
		}
	}else{/*FOR MISRA C*/}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioAMFMAFListObject);
	}else{/*For MISRA C*/}

}
/*===========================================================================*/
/* void DAB_Notify_AFList_Diag															*/
/*===========================================================================*/
void DAB_Notify_AFList_Diag(Tu32 *DAB_AltFreq, Tu16* DAB_AltEnsemble, Tu32* DAB_HardlinkSid, Tu8 NumAltFreq, Tu8 NumAltEnsemble, Tu8 NumAltHardlinkSid)
{
	/*Copying the Number of Alternate Frequency, Ensemble and Number of Hardlink Sid's*/
	_radioDABAFListData.NumofAltFrequecy = NumAltFreq;
	
	_radioDABAFListData.NumofAltEnsemble = NumAltEnsemble;
	
	_radioDABAFListData.NumofHardlinkSid = NumAltHardlinkSid;
	
	/*Updating the band as DAB band*/
	_radioDABAFListData.nBand = RADIO_MODE_DAB;
	
	memset(_radioDABAFListData.DAB_AltFreqList, 0, sizeof(_radioDABAFListData.DAB_AltFreqList));
    if(DAB_AltFreq != NULL)
    {
		SYS_RADIO_MEMCPY((void*)_radioDABAFListData.DAB_AltFreqList, (const void*)DAB_AltFreq, sizeof(_radioDABAFListData.DAB_AltFreqList));
    }else{/*FOR MISRA C*/}
	
	memset(_radioDABAFListData.DAB_AltEnsembleList, 0, sizeof(_radioDABAFListData.DAB_AltEnsembleList));
    if(DAB_AltFreq != NULL)
    {
		SYS_RADIO_MEMCPY((void*)_radioDABAFListData.DAB_AltEnsembleList, (const void*)DAB_AltEnsemble, sizeof(_radioDABAFListData.DAB_AltEnsembleList));
    }else{/*FOR MISRA C*/}
	
	memset(_radioDABAFListData.DAB_HardlinkSidList, 0, sizeof(_radioDABAFListData.DAB_HardlinkSidList));
    if(DAB_AltFreq != NULL)
    {
		SYS_RADIO_MEMCPY((void*)_radioDABAFListData.DAB_HardlinkSidList, (const void*)DAB_HardlinkSid, sizeof(_radioDABAFListData.DAB_AltFreqList));
    }else{/*FOR MISRA C*/}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDABAFListObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void DABFM_Notify_DABFMLinkingStatus															*/
/*===========================================================================*/
void DABFM_Notify_DABFMLinkingStatus(Te_RADIO_DABFM_LinkingStatus e_linking_Status)
{
	switch(e_linking_Status)
	{
	case RADIO_FRMWK_DAB_FM_HARDLINKS_RECEIVED:
		_radioDABFMLinkStatusData.Status = RADIO_DAB_FM_HARDLINKS_RECEIVED;
		break;

	case RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED:
		_radioDABFMLinkStatusData.Status = RADIO_DAB_FM_BEST_PI_RECEIVED;
		break;

	case RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED:
		_radioDABFMLinkStatusData.Status = RADIO_FM_IMPLICIT_PI_RECEIVED;
		break;

	case RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE:
		_radioDABFMLinkStatusData.Status = RADIO_DAB_FM_LINKING_NOT_AVAILABLE;
		break;

	case RADIO_FRMWK_DAB_FM_LINKING_CANCELLED:
		_radioDABFMLinkStatusData.Status = RADIO_FM_LINKING_CANCELLED;
		break;

	case RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS:
		_radioDABFMLinkStatusData.Status = RADIO_DAB_FM_BLENDING_SUCCESS;
		break;

	case RADIO_FRMWK_DAB_FM_BLENDING_FAILURE:
		_radioDABFMLinkStatusData.Status = RADIO_DAB_FM_BLENDING_FAILURE;
		break;

	case RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS:
		_radioDABFMLinkStatusData.Status = RADIO_IMPLICIT_FM_BLENDING_SUCCESS;
		break;

	case RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_FAILURE:
		_radioDABFMLinkStatusData.Status = RADIO_FM_IMPLICIT_BLENDING_FAILURE;
		break;

	case RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED:
		_radioDABFMLinkStatusData.Status = RADIO_FM_BLENDING_SUSPENDED;
		break;

	case RADIO_FRMWK_DAB_RESUME_BACK:
		_radioDABFMLinkStatusData.Status = RADIO_DAB_RESUME_BACK;
		break;

	default:
		/* Do Nothing */
		break;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDABFMLinkStatusObject);
	}else{/*For MISRA C*/}

}


/*===========================================================================*/
/* void Radio_Response_StoreMemoryList															*/
/*===========================================================================*/
void Radio_Response_StoreMemoryList(Te_RADIO_ReplyStatus e_presetStoreReplyStatus)
{
	if(e_presetStoreReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_PRESET_STORE_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_PRESET_STORE_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}
	
}

/*===========================================================================*/
/* void Radio_Response_PlaySelectMemoryList															*/
/*===========================================================================*/
void Radio_Response_PlaySelectMemoryList(Te_RADIO_ReplyStatus e_presetRecallReplyStatus)
{
	if(e_presetRecallReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_PRESET_RECALL_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_PRESET_RECALL_FAILURE;
	}

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void Radio_Response_RDSFollowing												*/
/*===========================================================================*/
void Radio_Response_RDSFollowing(Te_RADIO_ReplyStatus e_RDSSettingsReplyStatus)
{
	if(e_RDSSettingsReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_RDS_FOLLOWING_REQ_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_RDS_FOLLOWING_REQ_FAILURE;
	}
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void Radio_Response_ManualUpdateSTL											*/
/*===========================================================================*/
void Radio_Response_ManualUpdateSTL(Te_RADIO_ReplyStatus e_StationListreplyStatus)
{
	if(e_StationListreplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_MANUAL_UPDATE_STL_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_MANUAL_UPDATE_STL_FAILURE;
	}

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}



/*===========================================================================*/
/* void Notify_UpdateMemoryList															*/
/*===========================================================================*/
void Notify_UpdateMemoryList(Ts_Radio_Mngr_App_Preset_Mixed_List *MemoryStList)
{
	if(MemoryStList != NULL)
	{
		for(preset_index=0; preset_index < MAX_RADIO_PRESET_STATIONS; preset_index++)
		{
			switch(MemoryStList->ast_presetlist[preset_index].e_Band)
			{
				case RADIO_MNGR_APP_BAND_AM:
				{
					_radioMemoryListData[preset_index].nBand = RADIO_MODE_AM;
					_radioMemoryListData[preset_index].index = preset_index;
					_radioMemoryListData[preset_index].Char_set = 0;
					_radioMemoryListData[preset_index].Frequency = MemoryStList->ast_presetlist[preset_index].u_PresetStInfo.st_AMStnInfo.u32_Freq;
					memset(_radioMemoryListData[preset_index].MemoryStName, 0, sizeof(_radioMemoryListData[preset_index].MemoryStName));
					
				}
				break;

				case RADIO_MNGR_APP_BAND_FM:
				{
					_radioMemoryListData[preset_index].nBand = RADIO_MODE_FM;
					_radioMemoryListData[preset_index].index = preset_index;
					_radioMemoryListData[preset_index].Frequency = MemoryStList->ast_presetlist[preset_index].u_PresetStInfo.st_FMStnInfo.u32_frequency;
					_radioMemoryListData[preset_index].Char_set = MemoryStList->ast_presetlist[preset_index].u8_CharSet;
					
					memset((void *)_radioMemoryListData[preset_index].MemoryStName, 0, sizeof(_radioMemoryListData[preset_index].MemoryStName));
					SYS_RADIO_MEMCPY((_radioMemoryListData[preset_index].MemoryStName),(MemoryStList->ast_presetlist[preset_index].u_PresetStInfo.st_FMStnInfo.au8_PSN),RADIO_MAX_CHAN_NAME_SIZE);

				}
				break;

				case RADIO_MNGR_APP_BAND_DAB:
				{
					_radioMemoryListData[preset_index].nBand = RADIO_MODE_DAB;
					_radioMemoryListData[preset_index].index = preset_index;
					_radioMemoryListData[preset_index].Frequency = MemoryStList->ast_presetlist[preset_index].u_PresetStInfo.st_DABStnInfo.u32_Frequency;
					_radioMemoryListData[preset_index].Char_set = MemoryStList->ast_presetlist[preset_index].u8_CharSet;

					memset(_radioMemoryListData[preset_index].MemoryStName, 0, sizeof(_radioMemoryListData[preset_index].MemoryStName));
					SYS_RADIO_MEMCPY((_radioMemoryListData[preset_index].MemoryStName),(MemoryStList->ast_presetlist[preset_index].u_PresetStInfo.st_DABStnInfo.st_ComponentName.au8_CompLabel),
										RADIO_MAX_COMPONENT_LABEL_SIZE);
					/*Clearing & Copying the DAB Channel Name*/					
					memset(_radioMemoryListData[preset_index].DAB_Channel_Name, 0, sizeof(_radioMemoryListData[preset_index].DAB_Channel_Name));
					SYS_RADIO_MEMCPY((_radioMemoryListData[preset_index].DAB_Channel_Name),(MemoryStList->ast_presetlist[preset_index].u_PresetStInfo.st_DABStnInfo.au8_ChannelName),
										RADIO_MAX_CHANNEL_LABEL_SIZE);

				}
				break;

				default:
				{
					/*Do nothing*/	
				}
				break;
				
			}
		}
	}
	else{/*Do nothing*/}

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioMemoryDataObjectDisplay);
	}
	else{/*For MISRA C*/}
}

/*===========================================================================*/
/* void Radio_Response_TuneUpDown												*/
/*===========================================================================*/
void Radio_Response_TuneUpDown(Te_RADIO_ReplyStatus e_TuneUpDownreplyStatus)
{
	if(e_TuneUpDownreplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_TUNE_UP_DOWN_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_TUNE_UP_DOWN_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}


/*===========================================================================*/
/* void Radio_Response_EnableTAAnnouncement										*/
/*===========================================================================*/
void Radio_Response_EnableTAAnnouncement(Te_RADIO_ReplyStatus e_Anno_EnableReplystatus)
{
	if(e_Anno_EnableReplystatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_TA_ANNOUNCEMENT_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_TA_ANNOUNCEMENT_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void Radio_Response_CancelAnnouncement										*/
/*===========================================================================*/
void Radio_Response_CancelAnnouncement(Te_RADIO_ReplyStatus e_Anno_CancelReplystatus)
{
	if(e_Anno_CancelReplystatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_CANCEL_ANNOUNCEMENT_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_CANCEL_ANNOUNCEMENT_FAILURE;
	}

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}
}

/*===========================================================================*/
/* void Radio_Notify_Announcement													*/
/*===========================================================================*/
void Radio_Notify_Announcement(Te_Radio_Mngr_App_Anno_Status e_Anno_Status)
{

#ifdef HMI_TCP_PROTOCOL
	char msg_TAAnno[2]={0};
	msg_TAAnno[0]=TA_NOTIFY;
	switch(e_Anno_Status)
	{
		case RADIO_MNGR_APP_ANNO_START:
			{
			msg_TAAnno[1]='1';
			}
			break;

		case RADIO_MNGR_APP_ANNO_END:
			{
			msg_TAAnno[1]='2';
			}		
			break;

		case RADIO_MNGR_APP_ANNO_INVALID:
			break;
		default:
			break;
	}
	sendtoclient(msg_TAAnno);

#else
	switch(e_Anno_Status)
	{
		case RADIO_MNGR_APP_ANNO_START:
			_radioAnnouncementInfoData.Anno_Status = RADIO_ANNOUNCEMENT_START;
			break;

		case RADIO_MNGR_APP_ANNO_END:
			_radioAnnouncementInfoData.Anno_Status = RADIO_ANNOUNCEMENT_END;
			break;

		case RADIO_MNGR_APP_ANNO_INVALID:
			_radioAnnouncementInfoData.Anno_Status = RADIO_ANNOUNCEMENT_INVALID;
			break;

		default:
			/* Do Nothing */
			break;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioAnnouncementObject);
	}
	else{/* For MISRA C */}

#endif
}

/*===========================================================================*/
/* void Radio_Notify_Components_Status												*/
/*===========================================================================*/
void Radio_Notify_Components_Status(Te_Radio_Mngr_App_Band eBand, Te_RADIO_AMFMTuner_Status e_AMFMTunerStatus, Te_RADIO_Comp_Status e_DABTunerStatus, Te_Radio_Mngr_App_DAB_UpNotification_Status	e_DAB_UpNot_Status)
{
	switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
        {
			_radioComponentStatusData.nBand = RADIO_MODE_AM;
        }
        break;

        case RADIO_MNGR_APP_BAND_FM:
        {
			_radioComponentStatusData.nBand = RADIO_MODE_FM;
        }
        break;

        case RADIO_MNGR_APP_BAND_DAB:
        {
			_radioComponentStatusData.nBand = RADIO_MODE_DAB;
        }
		break;

        default:
		{
			_radioComponentStatusData.nBand = RADIO_MODE_INVALID;
		}
        break;
	}
	
    switch(e_AMFMTunerStatus)
    {
        case RADIO_FRMWK_AMFMTUNER_NORMAL:
		{
            _radioComponentStatusData.e_AMFMTuner_Status = RADIO_AMFMTUNER_NORMAL;
		}
        break;

        case RADIO_FRMWK_AMFMTUNER_I2CERROR:
		{
            _radioComponentStatusData.e_AMFMTuner_Status = RADIO_AMFMTUNER_I2CERROR;
		}
        break;

		case RADIO_FRMWK_AMFMTUNER_INTERNAL_RESET:
		{
            _radioComponentStatusData.e_AMFMTuner_Status = RADIO_AMFMTUNER_INTERNAL_RESET;
		}
		break;
		
        case RADIO_FRMWK_AMFMTUNER_STATUS_INVALID:
		{
            _radioComponentStatusData.e_AMFMTuner_Status = RADIO_AMFMTUNER_STATUS_INVALID;
		}
        break;

        default:
		{/*Do Nothing*/}
		break;
    }

    switch(e_DABTunerStatus)
    {
        case RADIO_FRMWK_COMP_STATUS_NORMAL:
		{
            _radioComponentStatusData.e_DABTuner_Status = RADIO_COMP_STATUS_NORMAL;
		}
        break;

        case RADIO_FRMWK_COMP_STATUS_ABNORMAL:
		{
            _radioComponentStatusData.e_DABTuner_Status = RADIO_COMP_STATUS_ABNORMAL;
		}
        break;

        case RADIO_FRMWK_COMP_STATUS_INVALID:
		{
        	_radioComponentStatusData.e_DABTuner_Status = RADIO_COMP_STATUS_INVALID;
		}
        break;

        default:
		{/* Do Nothing */}
		break;
    }
	
	switch(e_DAB_UpNot_Status)
	{
		case RADIO_MNGR_APP_DAB_UP_NOTIFICATION_NOT_RECEIVED:
		{
			_radioComponentStatusData.e_DAB_UpNot_Status = RADIO_DAB_UP_NOTIFICATION_NOT_RECEIVED;
		}
	    break;

		case RADIO_MNGR_APP_DAB_UP_NOTIFICATION_RECEIVED:
		{
			_radioComponentStatusData.e_DAB_UpNot_Status = RADIO_DAB_UP_NOTIFICATION_RECEIVED;
		}
		break;

		case RADIO_MNGR_APP_DAB_UP_NOTIFICATION_INVALID:
		{
			_radioComponentStatusData.e_DAB_UpNot_Status = RADIO_DAB_UP_NOTIFICATION_INVALID;
		}
		break;

		default:
		{/* Do Nothing */}
		break;
	}

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioComponentStatusObject);
	}
	else
	{
		/*Do nothing*/
	}/* For MISRA C */
}

/*===========================================================================*/
/* void Radio_Notify_Settings												*/
/*===========================================================================*/
void Radio_Notify_Settings(Te_Radio_Mngr_App_DABFMLinking_Switch e_DABFM_Status, Te_Radio_Mngr_App_EnableTAAnno_Switch e_TA_Anno_Status, Te_Radio_Mngr_App_RDSSettings e_AF_Status, Te_Radio_Mngr_App_EnableInfoAnno_Switch e_Info_Anno_Status, Te_Radio_Mngr_App_Multiplex_Switch e_Multiplex_Switch_Status)
{
	switch(e_DABFM_Status)
	{
		case RADIO_MNGR_APP_DABFMLINKING_ENABLE:
			_radioSettingsStatusData.e_DABFM_Status = RADIO_SWITCH_SETTING_ON;
			break;

		case RADIO_MNGR_APP_DABFMLINKING_DISABLE:
			_radioSettingsStatusData.e_DABFM_Status = RADIO_SWITCH_SETTING_OFF;
			break;

		case RADIO_MNGR_APP_DABFMLINKING_INVALID:
			 _radioSettingsStatusData.e_DABFM_Status = RADIO_SWITCH_SETTING_INVALID;
			 break;

		default:
				/* Do Nothing */
		break;
	}

	switch(e_TA_Anno_Status)
	{
		case RADIO_MNGR_APP_TA_ANNO_ENABLE:
			 _radioSettingsStatusData.e_TA_Anno_Status = RADIO_SWITCH_SETTING_ON;
			 break;

		case RADIO_MNGR_APP_TA_ANNO_DISABLE:
			 _radioSettingsStatusData.e_TA_Anno_Status = RADIO_SWITCH_SETTING_OFF;
			 break;

		case RADIO_MNGR_APP_TA_ANNO_REQ_INVALID:
			 _radioSettingsStatusData.e_TA_Anno_Status = RADIO_SWITCH_SETTING_INVALID;
			 break;

		default:
				/* Do Nothing */
		break;
	}

	switch(e_AF_Status)
	{
		case RADIO_MNGR_APP_RDS_SETTINGS_ENABLE:
			_radioSettingsStatusData.e_RDS_Status = RADIO_SWITCH_SETTING_ON;
			break;

		case RADIO_MNGR_APP_RDS_SETTINGS_DISABLE:
			 _radioSettingsStatusData.e_RDS_Status = RADIO_SWITCH_SETTING_OFF;
			 break;

		case RADIO_MNGR_APP_RDS_SETTINGS_INVALID:
			_radioSettingsStatusData.e_RDS_Status = RADIO_SWITCH_SETTING_INVALID;
			break;

		default:
				/* Do Nothing */
		break;
	}
	
	switch(e_Info_Anno_Status)
	{
		case RADIO_MNGR_APP_INFO_ANNO_ENABLE:
			 _radioSettingsStatusData.e_Info_Anno_Status = RADIO_SWITCH_SETTING_ON;
			 break;

		case RADIO_MNGR_APP_INFO_ANNO_DISABLE:
			 _radioSettingsStatusData.e_Info_Anno_Status = RADIO_SWITCH_SETTING_OFF;
			 break;

		case RADIO_MNGR_APP_INFO_ANNO_REQ_INVALID:
			 _radioSettingsStatusData.e_Info_Anno_Status = RADIO_SWITCH_SETTING_INVALID;
			 break;

		default:
				/* Do Nothing */
		break;
	}
	
	switch(e_Multiplex_Switch_Status)
	{
		case RADIO_MNGR_APP_MULTIPLEX_ENABLE:
			 _radioSettingsStatusData.e_Multiplex_Switch_Status = RADIO_SWITCH_SETTING_ON;
			 break;

		case RADIO_MNGR_APP_MULTIPLEX_DISABLE:
			 _radioSettingsStatusData.e_Multiplex_Switch_Status = RADIO_SWITCH_SETTING_OFF;
			 break;

		case RADIO_MNGR_APP_MULTIPLEX_INVALID:
			 _radioSettingsStatusData.e_Multiplex_Switch_Status = RADIO_SWITCH_SETTING_INVALID;
			 break;

		default:
				/* Do Nothing */
		break;
	}
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioSettingsStatusObject);
	}else{/* For MISRA C */}
}


/*===========================================================================*/
/* void Notify_Activity_State												*/
/*===========================================================================*/
void Notify_Activity_State(Te_Radio_Mngr_App_Band eBand, Te_Radio_Mngr_App_Activity_Status e_Activity_State)
{
    switch(eBand)
    {
        case RADIO_MNGR_APP_BAND_AM:
        {
            _radioActivityStatusData.nBand = RADIO_MODE_AM;
        }
        break;

        case RADIO_MNGR_APP_BAND_FM:
        {
            _radioActivityStatusData.nBand = RADIO_MODE_FM;
        }
        break;

        case RADIO_MNGR_APP_BAND_DAB:
        {
              _radioActivityStatusData.nBand = RADIO_MODE_DAB;  
        }
		break;

        default:
			_radioActivityStatusData.nBand = RADIO_MODE_INVALID;
            break;
    
    }

    switch(e_Activity_State)
    {
        case RADIO_MNGR_APP_STATION_NOT_AVAILABLE:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_STATION_NOT_AVAILABLE;
        }
        break;

        case RADIO_MNGR_APP_FM_AF_PROCESSING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_FM_AF_PROCESSING;    
        }
        break;

        case RADIO_MNGR_APP_FM_INTERNAL_SCAN_PROCESS:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_FM_INTERNAL_SCAN_PROCESS;
        }
        break;

        case RADIO_MNGR_APP_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING;
        }
        break;

        case RADIO_MNGR_APP_DAB_AF_PROCESSING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DAB_AF_PROCESSING;
        }
        break;

        case RADIO_MNGR_APP_DAB_INTERNAL_SCAN_PROCESS:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DAB_INTERNAL_SCAN_PROCESS;
        }
        break;

        case RADIO_MNGR_APP_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING;
        }
        break;
		
		case RADIO_MNGE_APP_FM_LEARNMEM_AF_PROCESSING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_FM_LEARNMEM_AF_PROCESSING;
        }
        break;
		
		case RADIO_MNGR_APP_DAB_LEARNMEM_AF_PROCESSING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DAB_LEARNMEM_AF_PROCESSING;
        }
        break;
		
		case RADIO_MNGR_APP_SIGNAL_LOSS:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_SIGNAL_LOSS;
        }
        break;
		
		case RADIO_MNGR_APP_DAB_DAB_STARTED:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DAB_DAB_STARTED;
        }
        break;
		
		case RADIO_MNGR_APP_DAB_DAB_LINKING_DONE:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DAB_DAB_LINKING_DONE;
        }
        break;
		
        case RADIO_MNGR_APP_LISTENING:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_LISTENING;
        }
        break;

        case RADIO_MNGR_APP_IN_SCAN:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_IN_SCAN;    
        }
        break;

        case RADIO_MNGR_APP_TUNE_UPDOWN:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_TUNE_UPDOWN;
        }
        break;

        case RADIO_MNGR_APP_ANNOUNCEMENT:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_ANNOUNCEMENT;
        }
        break;

        case RADIO_MNGR_APP_DABFMLINKING_DONE:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DABFMLINKING_DONE;
        }
        break;

        case RADIO_MNGR_APP_IMPLICIT_LINKING_DONE:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_IMPLICIT_LINKING_DONE;
        }
        break;
		
		case RADIO_MNGR_APP_AF_SWITCHING_ESTABLISHED:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_AF_SWITCHING_ESTABLISHED;
        }
        break;
		
		case RADIO_MNGR_APP_DABTUNER_ABNORMAL:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_DABTUNER_ABNORMAL;
        }
        break;
		case RADIO_MNGR_APP_STATUS_INVALID:
        {
            _radioActivityStatusData.e_Activity_Status = RADIO_STATUS_INVALID;
        }
        break;

        default:
            break;

    }
	 
	RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO,"[Radio][HMI IF]Activity Status Notified to HMI: Activity_State: %d",e_Activity_State);
	 
    if(_pfnNotifyHdlr != NULL)
    {
        _pfnNotifyHdlr(&_radioActivityStatusObject);
    }else{/* For MISRA C */}
}

/*===========================================================================*/
/* void Notify_BestPIStation												 */
/*===========================================================================*/
void Notify_BestPIStation(Tu32 u32_Freq, Tu16 u16_PI, Tu32 u32_Quality)
{
    _radioBestPIStationData.Frequency = u32_Freq;
    _radioBestPIStationData.Best_PI   = u16_PI;
    _radioBestPIStationData.Quality   = u32_Quality;

    if(_pfnNotifyHdlr != NULL)
    {
        _pfnNotifyHdlr(&_radioBestPIStationObject);
    }else{/* For MISRA C */}
}

/*===========================================================================*/
/* void Radio_Response_SRCActivateDeactivate										*/
/*===========================================================================*/
void Radio_Response_SRCActivateDeactivate(Te_RADIO_ReplyStatus e_ActivateDeactivate_ReplyStatus)
{
	if(e_ActivateDeactivate_ReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_SRC_ACTIVATE_DEACTIVATE_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_SRC_ACTIVATE_DEACTIVATE_FAILURE;
	}

	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}
}


/*===========================================================================*/
/* void Radio_Response_GetClockTime										*/
/*===========================================================================*/
void Radio_Response_GetClockTime(Te_RADIO_ReplyStatus e_CTInfoReplyStatus)
{
	if(e_CTInfoReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_GET_CT_INFO_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_GET_CT_INFO_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void Radio_Notify_ClockTime												*/
/*===========================================================================*/
void Radio_Notify_ClockTime(Tu8 u8_hour, Tu8 u8_Min, Tu8 u8_day, Tu8 u8_Month, Tu16 u16_Year)
{
	_radioClockTimeInfoData.Hour 		= u8_hour;
	_radioClockTimeInfoData.Minutes 	= u8_Min;
	_radioClockTimeInfoData.Day 		= u8_day;
	_radioClockTimeInfoData.Month 		= u8_Month;
	_radioClockTimeInfoData.Year 		= u16_Year;

    if(_pfnNotifyHdlr != NULL)
    {
        _pfnNotifyHdlr(&_radioClockTimeInfoObject);
    }else{/* For MISRA C */}
}

/*===========================================================================*/
/* void Radio_Response_CancelManualSTLUpdate										*/
/*===========================================================================*/
void Radio_Response_CancelManualSTLUpdate(Te_RADIO_ReplyStatus e_CancelManualSTLReplyStatus)
{
	if(e_CancelManualSTLReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_CANCEL_MANUAL_UPDATE_STL_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_CANCEL_MANUAL_UPDATE_STL_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}
/*===========================================================================*/
/* void Radio_Response_EnableInfoAnnouncement										*/
/*===========================================================================*/
void Radio_Response_EnableInfoAnnouncement(Te_RADIO_ReplyStatus e_Info_Switch_Status)
{
	if(e_Info_Switch_Status == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_INFO_ANNOUNCEMENT_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_INFO_ANNOUNCEMENT_FAILURE;
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void Radio_Response_BG_AFStationInfo												*/
/*===========================================================================*/
void Radio_Response_BG_AFStationInfo(Te_Radio_Mngr_App_Band e_SRC, Te_Radio_Mngr_App_Band e_AudBand, Tu32 nFreq, Tu8 *stName, Tu8 CharSet, Tu8* radioText, Tu8* ChannelName, Tu8* Ensemble_Name, Tu8 DAB_Current_Playing_Service, Tu8 DAB_TotalNoofServiceInEnsemble)
{
	memset(_radioCommonData.ServiceCompName, 0, sizeof(_radioCommonData.ServiceCompName));
	switch (e_SRC)
	{
	case RADIO_MNGR_APP_BAND_FM:
	{
								   _radioCommonData.nBand = RADIO_MODE_FM;
								   if (stName != NULL)
								   {
									   SYS_RADIO_MEMCPY((void*)_radioCommonData.ServiceCompName, (const void*)stName, RADIO_MAX_COMPONENT_LABEL_SIZE);
								   }
								   else{/*FOR MISRA C*/ }
	}
		break;

	case RADIO_MNGR_APP_BAND_DAB:
	{
									_radioCommonData.nBand = RADIO_MODE_DAB;
									if (stName != NULL)
									{
										SYS_RADIO_MEMCPY((void*)_radioCommonData.ServiceCompName, (const void*)stName, RADIO_MAX_CHAN_NAME_SIZE);
									}
									else{/*FOR MISRA C*/ }
	}
		break;

	default:
		_radioCommonData.nBand = RADIO_MODE_INVALID;
		break;
	}
	_radioCommonData.Aud_Band =  (MODE_TYPE)e_AudBand;

	_radioCommonData.Frequency = nFreq;
	_radioCommonData.Char_set = CharSet;
	/*Copying currently playing service and Total number of services present in that ensemble*/
	_radioCommonData.DAB_Current_Service 					= DAB_Current_Playing_Service;
	_radioCommonData.DAB_Total_Num_Services_In_Ensemble		= DAB_TotalNoofServiceInEnsemble;
	
	if(_radioCommonData.Frequency == 0)
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_NOTICE,"[Radio][HMI IF]Radio Zero frequency observed in Background: Frequency: %d",_radioCommonData.Frequency);
	}else{/*FOR MISRA C*/}

	memset(_radioCommonData.DLS_Radio_Text, 0, sizeof(_radioCommonData.DLS_Radio_Text));
	if (radioText != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioCommonData.DLS_Radio_Text, (const void*)radioText, RADIO_MAX_DLS_DATA_SIZE);
	}
	else{/*FOR MISRA C*/ }

	memset(_radioCommonData.Channel_Name, 0, sizeof(_radioCommonData.Channel_Name));
	if (ChannelName != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioCommonData.Channel_Name, (const void*)ChannelName, RADIO_MAX_CHANNEL_LABEL_SIZE);
	}
	else{/*FOR MISRA C*/ }

	/*Code to copy Ensemble label for Active band DAB*/
	memset(_radioCommonData.Ensemble_Label, 0, sizeof(_radioCommonData.Ensemble_Label));
	if(Ensemble_Name != NULL)
	{
		SYS_RADIO_MEMCPY((void*)_radioCommonData.Ensemble_Label,(const void*)Ensemble_Name, RADIO_MAX_ENSEMBLELABEL_LENGTH);
	}
	else{/*FOR MISRA C*/}
	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDataObject);
	}
	else
	{
		/*Do nothing*/
	}/* For MISRA C */
}

/*===========================================================================*/
/* void Radio_Notify_AudioSwitch												*/
/*===========================================================================*/
void Radio_Notify_AudioSwitch(Te_Radio_Mngr_App_Band e_ReqAudioChangeBand)
{
	/*Updating the common data band which is useful to switch audio in Get Radio Mode function*/
	_radioCommonData.Aud_Band = (MODE_TYPE)e_ReqAudioChangeBand;

	/*Sending select band success response to Mode Manager after which it will call Get Radio Mode function*/
	_radioStatus.nStatus = RADIO_SELECTBAND_SUCCESS;


	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioDataObject);
	}
	else{/*For MISRA C*/ }
#if 0
	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/*For MISRA C*/ }
#endif
}

/*===========================================================================*/
/* void Radio_Response_PowerOnStatus												*/
/*===========================================================================*/
void Radio_Response_PowerOnStatus(Te_RADIO_ReplyStatus  e_PowerOnReplyStatus)
{
	if (e_PowerOnReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_POWERON_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_POWERON_FAILURE;
	}



	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/*For MISRA C*/ }
}

/*===========================================================================*/
/* void Radio_Response_PowerOffStatus												*/
/*===========================================================================*/
void Radio_Response_PowerOffStatus(Te_RADIO_ReplyStatus  e_PowerOffReplyStatus)
{
	if (e_PowerOffReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_POWEROFF_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_POWEROFF_FAILURE;
	}


	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/*For MISRA C*/ }
}
/*===========================================================================*/
/* void Radio_Response_FactoryReset												*/
/*===========================================================================*/
void Radio_Response_FactoryReset(Te_RADIO_ReplyStatus e_FactoryResetReplyStatus)
{
	if (e_FactoryResetReplyStatus == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_FACTORY_RESET_SUCCESS;
		memset(_radioStationListData_Display, 0, sizeof(_radioStationListData_Display[0]) * MAX_RADIO_STATIONS);

		memset(_radioMemoryListData, 0, sizeof(_radioMemoryListData[0]) * MAX_RADIO_PRESET_STATIONS);
	}
	else
	{
		_radioStatus.nStatus = RADIO_FACTORY_RESET_FAILURE;
	}



	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/*For MISRA C*/ }
}

/*===========================================================================*/
/* void Radio_Notify_FirmwareVersion												*/
/*===========================================================================*/
void Radio_Notify_FirmwareVersion(Tu8* DABTunerSWVersion, Tu8* DABTunerHWVersion, Tu8* AMFMTunerVersion)
{
	/*Checking if valid FW versions are received then only update and notify to HMI*/
	if (DABTunerSWVersion != NULL)
	{
		/*Copying DABTuner software firmware version, DABTuner Hardware Firmware version & AMFMTuner Firmware version*/
		SYS_RADIO_MEMCPY((void*)_radioFirmwareVersionData.DABTuner_FW_SWVersion, (const void*)DABTunerSWVersion, RADIO_MAX_DABTUNER_SW_VERSION_LENGTH);
	}
	else{/*FOR MISRA C*/ }

	if (DABTunerHWVersion != NULL)
	{
		/*Copying DABTuner software firmware version, DABTuner Hardware Firmware version & AMFMTuner Firmware version*/
		SYS_RADIO_MEMCPY((void*)_radioFirmwareVersionData.DABTuner_FW_HWVersion, (const void*)DABTunerHWVersion, RADIO_MAX_DABTUNER_HW_VERSION_LENGTH);
	}
	else{/*FOR MISRA C*/ }

	if (AMFMTunerVersion != NULL)
	{
		/*Copying DABTuner software firmware version, DABTuner Hardware Firmware version & AMFMTuner Firmware version*/
		SYS_RADIO_MEMCPY((void*)_radioFirmwareVersionData.AMFMTuner_FW_Version, (const void*)AMFMTunerVersion, RADIO_MAX_AMFMTUNER_VERSION_LENGTH);
	}
	else{/*FOR MISRA C*/ }

	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioFirmwareVersionInfoObject);
	}
	else{/*For MISRA C*/ }
}
/*===========================================================================*/
/* void Notify_DAB_Dataservice_To_Display								*/
/*===========================================================================*/
void Notify_DAB_Dataservice_To_Display(Ts_Radio_Mngr_App_DataServiceRaw* Dataserviceraw)
{
#ifndef HMI_TCP_PROTOCOL
	if (Dataserviceraw->e_Header == RADIO_FRMWK_DAB_DATASERV_TYPE_SLS_XPAD)
	{
		SYS_RADIO_MEMCPY(&_radioDABDataServiceRawData, Dataserviceraw, sizeof(tRadioDABDataServiceInfoDataBox_Display));

		if (_pfnNotifyHdlr != NULL)
		{
			_pfnNotifyHdlr(&_radioDABDataServiceRawDataObject);
		}
		else{/*For MISRA C*/ }
	}
#else
	UNUSED(Dataserviceraw);
#endif
}

/*===========================================================================*/
/* void Radio_Response_EnableMultiplexInfoSwitch										*/
/*===========================================================================*/
void Radio_Response_EnableMultiplexInfoSwitch(Te_RADIO_ReplyStatus e_Multiplex_Switch_Status)
{
	if(e_Multiplex_Switch_Status == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_MULTIPLEX_SWITCH_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_MULTIPLEX_SWITCH_FAILURE; 
	}
	
	if(_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}else{/*For MISRA C*/}

}

/*===========================================================================*/
/* void Radio_Response_ETALHwConfig												*/
/*===========================================================================*/
void Radio_Response_ETALHwConfig(Te_RADIO_ReplyStatus e_etal_startup_reply_status)
{
	if (e_etal_startup_reply_status == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_ETALHWCONFIG_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_ETALHWCONFIG_FAILURE;
	}

	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/*For MISRA C*/ }
}

/*===========================================================================*/
/* void Radio_Response_ETALHwDeConfig										 */
/*===========================================================================*/
void Radio_Response_ETALHwDeConfig(Te_RADIO_ReplyStatus e_etal_deinit_reply_status)
{
	if (e_etal_deinit_reply_status == REPLYSTATUS_SUCCESS)
	{
		_radioStatus.nStatus = RADIO_ETALHWDECONFIG_SUCCESS;
	}
	else
	{
		_radioStatus.nStatus = RADIO_ETALHWDECONFIG_FAILURE;
	}

	if (_pfnNotifyHdlr != NULL)
	{
		_pfnNotifyHdlr(&_radioStatusObject);
	}
	else{/*For MISRA C*/ }
}
/*=============================================================================
    end of file
============================================================================ */