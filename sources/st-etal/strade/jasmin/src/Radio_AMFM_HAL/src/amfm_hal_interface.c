/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_HAL_INTERFACE.c                                                                    *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.                                                     * 
*  All rights reserved. Reproduction in whole or part is prohibited                                         *
*  without the written permission of the copyright owner.                                                   *
*                                                                                                           *
*  Project              : ST_Radio_Middleware                                                                               *
*  Organization			: Jasmin Infotech Pvt. Ltd.                                                         *
*  Module				: SC_AMFM_HAL_INTERFACE                                                             *
*  Description			: Sending the Soc Reply to AMFM tuner control instance hsm .                        *
*                                                                                                           *
*                                                                                                           * 
*************************************************************************************************************/

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "AMFM_HAL_Interface.h"
#include"AMFM_HAL_RDS_Collector.h"
#include "etal.h"
/*--------------------------------------------------------------------
	Variables
-----------------------------------------------------------------------*/

Tu8   u8_NUMPI;
Tbool b_ForceFast_PI;

ETAL_HANDLE *Receiver_handle;
ETAL_HANDLE *Datapath_Handle;
ETAL_HANDLE  Recvhdl;

ETAL_HANDLE AMFM_FG_recvhdl   = ETAL_INVALID_HANDLE;
ETAL_HANDLE AMFM_BG_recvhdl   = ETAL_INVALID_HANDLE;
ETAL_HANDLE FM_FG_recvhdl     = ETAL_INVALID_HANDLE;
ETAL_HANDLE FM_BG_recvhdl     = ETAL_INVALID_HANDLE;
ETAL_HANDLE AM_FG_recvhdl     = ETAL_INVALID_HANDLE;
ETAL_HANDLE FM_FG_datapathhdl = ETAL_INVALID_HANDLE;
ETAL_HANDLE FM_BG_datapathhdl = ETAL_INVALID_HANDLE;
ETAL_HANDLE FM_Quality_Monitorhdl  = ETAL_INVALID_HANDLE;
ETAL_HANDLE AM_Quality_Monitorhdl	= ETAL_INVALID_HANDLE;

EtalReceiverAttr          st_Receiver_Config;
EtalDataPathAttr          st_DatapathAttr;
Ts_Sink_Buffer	          st_context_cb;

ETAL_STATUS               e_AMFM_Tuner_Ctrl_ETAL_Status;
etalSeekAudioTy           e_audiostatus;
EtalAudioSourceTy         e_audiosource_type;
etalSeekDirectionTy       e_direction;
EtalSeekTerminationModeTy e_Seek_Termination_Mode;
Te_AMFM_Tuner_Ctrl_Band	  e_configured_Band = TUN_BAND_INVALID;
EtalBcastQualityMonitorAttr pBcastQualityMonitor;

/*-----------------------------------------------------------------------------
	private function definitions
-----------------------------------------------------------------------------*/

ETAL_STATUS SOC_CMD_TUNER_CONFIGURATION(Te_AMFM_Tuner_Ctrl_Market e_Market, Te_AMFM_Tuner_Ctrl_Band e_Requested_Band, Te_AMFM_Tuner_State e_Tuner_State)
{	
	Tbool en_audio = FALSE;
	if (AMFM_FG_recvhdl != ETAL_INVALID_HANDLE && e_Tuner_State != AMFM_TUNER_CTRL_BACKGROUND && e_audio_source_select != TRUE)
	{

		en_audio = TRUE;
	}
	else
	{
		/* MISRA C */
	}

	if (e_Requested_Band != e_configured_Band)
	{
		if (FM_FG_datapathhdl != ETAL_INVALID_HANDLE)
		{
			(void)SOC_CMD_DESTROY_RDS(FOREGROUND_CHANNEL);
		}
		else
		{
			/* MISRA C */
		}

		if (FM_Quality_Monitorhdl != ETAL_INVALID_HANDLE)
		{
			(void)SOC_CMD_Destroy_Quality_Monitor(FOREGROUND_CHANNEL);
		}
		else
		{
			/*For MISRA C*/
		}

		

		e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Config_Receiver(e_Requested_Band, FOREGROUND_CHANNEL);
		if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
		{
 			e_configured_Band = e_Requested_Band;
			e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Change_Receiver_Band(e_Market, e_Requested_Band, FOREGROUND_CHANNEL);
			if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
			{
				e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_SET_SEEK_THRESHOLDS(e_Requested_Band, FOREGROUND_CHANNEL);
				if ((e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS))
				{
					e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Config_Quality_Monitor(FOREGROUND_CHANNEL);
				
					if ((e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS) && (e_Requested_Band == TUN_BAND_FM))
					{
						e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Config_DataPath(FOREGROUND_CHANNEL);
						if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
						{
							e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Start_RDS(e_Requested_Band, FOREGROUND_CHANNEL);
							if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
							{
								/*sUCCESS*/
							}
							else
							{
								RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: start RDS error due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
							}
						}
						else
						{
							RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: Datapath error due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
						}
					}
					else
					{
						RADIO_DEBUG_LOG(RADIO_LOG_LVL_DEBUG, "[RADIO][AMFM_HAL]: Quality Monitor status:%d Requested band:%d", e_AMFM_Tuner_Ctrl_ETAL_Status, e_Requested_Band);
					}
				}
				else
				{
					RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: set seek threshold error due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
				}
			}
			else
			{
				RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: change receiver band error due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
			}
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: config receiver error due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
		}
	}
	else
	{
		/*NO NEED FOR ETAL CONFIGURATION*/
		e_AMFM_Tuner_Ctrl_ETAL_Status = ETAL_RET_SUCCESS;
	}

	if (en_audio == TRUE && e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
	{
		e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Audio_Source_Select(e_Requested_Band);

		if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
		{
			e_audio_source_select = TRUE;
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: Audio Source select failed due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
		}
	}
	else
	{
		/*Added for MISRA*/
	}

	return	e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	SOC_CMD_Configure_Receiver                                                              */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Config_Receiver(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	ETAL_HANDLE hFrontend;
	/* clearing the receiver config structure*/
	memset((void *)&st_Receiver_Config, 0x00, sizeof(EtalReceiverAttr));
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		if (e_Config_Band == TUN_BAND_FM)
		{
			st_Receiver_Config.m_Standard = ETAL_BCAST_STD_FM;
		}
		else
		{
			st_Receiver_Config.m_Standard = ETAL_BCAST_STD_AM;
		}

		st_Receiver_Config.m_FrontEndsSize = 1;
		hFrontend = ETAL_MAKE_FRONTEND_HANDLE(TUNER_0, ETAL_FE_FOREGROUND);
		st_Receiver_Config.m_FrontEnds[0] = hFrontend;
		Receiver_handle = &AMFM_FG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		st_Receiver_Config.m_Standard = ETAL_BCAST_STD_FM;
		st_Receiver_Config.m_FrontEndsSize = 1;
		hFrontend = ETAL_MAKE_FRONTEND_HANDLE(TUNER_0, ETAL_FE_BACKGROUND);
		st_Receiver_Config.m_FrontEnds[0] = hFrontend;
		Receiver_handle = &AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	st_Receiver_Config.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_UNSPECIFIED;
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_config_receiver(Receiver_handle, &st_Receiver_Config);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	 SOC_CMD_Configure_Datapath                                                             */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Config_DataPath(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		st_DatapathAttr.m_receiverHandle = AMFM_FG_recvhdl;
		Datapath_Handle = &FM_FG_datapathhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		st_DatapathAttr.m_receiverHandle = AMFM_BG_recvhdl;
		Datapath_Handle = &FM_BG_datapathhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	st_DatapathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS_RAW;
	st_DatapathAttr.m_sink.m_context = &st_context_cb;
	st_DatapathAttr.m_sink.m_BufferSize = ETAL_RDS_MAX_BUFFER_SIZE;
	st_DatapathAttr.m_sink.m_CbProcessBlock = RDS_cbFunc;

	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_config_datapath(Datapath_Handle, &st_DatapathAttr);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	SOC_CMD_Audio_Source_Select                                                             */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Audio_Source_Select(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band)
{
	Recvhdl = AMFM_FG_recvhdl;

	UNUSED(e_Config_Band);

	e_audiosource_type = ETAL_AUDIO_SOURCE_STAR_AMFM;

	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_audio_select(Recvhdl, e_audiosource_type);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}
/********************************************************************************************/
/* SOC_CMD_Change_Receiver_Band                                                             */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Change_Receiver_Band(Te_AMFM_Tuner_Ctrl_Market e_Market, Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_change_band_status;
	EtalFrequencyBand etal_frequency_band;
	ETAL_HANDLE Receiv_handle = ETAL_INVALID_HANDLE;
	Tu32 fmin = 0;
	Tu32 fmax = 0;
	EtalProcessingFeatures etal_processing_feature;
	etal_processing_feature.u.m_processing_features = ETAL_PROCESSING_FEATURE_UNSPECIFIED; /* Recommended Value As per ETAL Specification */

	switch (e_Config_Band)
	{
		case TUN_BAND_FM:
		{
			switch (e_Market)
			{
				case AMFM_TUNER_CTRL_WESTERN_EUROPE:
				case AMFM_TUNER_CTRL_LATIN_AMERICA:
				case AMFM_TUNER_CTRL_ASIA_CHINA:
				case AMFM_TUNER_CTRL_ARABIA:
				case AMFM_TUNER_CTRL_KOREA:
				{
					etal_frequency_band = ETAL_BAND_FMEU;
				}
				break;

				case AMFM_TUNER_CTRL_USA_NORTHAMERICA:
				{
					etal_frequency_band = ETAL_BAND_FMUS;
				}
				break;

				case AMFM_TUNER_CTRL_JAPAN:
				{
					etal_frequency_band = ETAL_BAND_FMJP;
				}
				break;

				default:
				{
					return ETAL_RET_ERROR;
				}
			}
		}
		break;

		case TUN_BAND_AM_MW:
		{
			etal_frequency_band = ETAL_BAND_MWEU;
		}
		break;

		default:
		{
			return ETAL_RET_ERROR;
		}		
	}

	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiv_handle = AMFM_FG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Receiv_handle = AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	e_change_band_status = etal_change_band_receiver(Receiv_handle, etal_frequency_band, fmin, fmax, etal_processing_feature);
	return e_change_band_status;
}
/********************************************************************************************/
/* SOC_CMD_Start_RDS                                                                        */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Start_RDS(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	ETAL_STATUS e_change_band_status;
	ETAL_HANDLE Receiv_handle = ETAL_INVALID_HANDLE;
	EtalRDSRBDSModeTy etal_rds_mode = ETAL_RDS_MODE;
	b_ForceFast_PI = FALSE;
	u8_NUMPI = 0; /* numPI value is the no of RDS blocks which may contain the PI information  to speed the PI acquisition on Host side */

	if ((e_Config_Band == TUN_BAND_FM) && (e_Frontend_type == FOREGROUND_CHANNEL))
	{
		Receiv_handle = AMFM_FG_recvhdl;
	}
	else if ((e_Config_Band == TUN_BAND_FM) && (e_Frontend_type == BACKGROUND_CHANNEL))
	{
		Receiv_handle = AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	e_change_band_status = etal_start_RDS(Receiv_handle, b_ForceFast_PI, u8_NUMPI, etal_rds_mode);
	return e_change_band_status;
}

/********************************************************************************************/
/* SOC_CMD_SET_SEEK_THRESHOLDS                                                              */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_SET_SEEK_THRESHOLDS(Te_AMFM_Tuner_Ctrl_Band  e_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	EtalSeekThreshold st_amfm_seek_threshold;
	if (e_Band == TUN_BAND_FM)
	{
		st_amfm_seek_threshold.SeekThresholdBBFieldStrength = 16;
		st_amfm_seek_threshold.SeekThresholdDetune = 10;
		st_amfm_seek_threshold.SeekThresholdAdjacentChannel = 0xEE;
		st_amfm_seek_threshold.SeekThresholdMultipath = 32;
		st_amfm_seek_threshold.SeekThresholdSignalNoiseRatio = 115;
		st_amfm_seek_threshold.SeekThresholdMpxNoise = 45;

		//values have to be revisited
		/*---------------------------------------------------------------------------
		Quality Parameter         Range in RCE(0-100)         Range calculated for ST
		BBFieldStrength            50                            -value directly tuk from ETAL_specification document.
		Detune					   9                           (10)  - (0 - 255)
		Multipath                  23                          (32)  - (0 - 255)
		USN                        18                          (45)  - (0 - 255)
		SNR                        -                           (115) - (0 - 255)
		Adjacent Channel		   -                           (-18)0xEE - value from RCE for USN
		-----------------------------------------------------------------------------------*/
	}
	else if (e_Band == TUN_BAND_AM_LW || e_Band == TUN_BAND_AM_MW)
	{
		st_amfm_seek_threshold.SeekThresholdBBFieldStrength = 25;
		st_amfm_seek_threshold.SeekThresholdDetune = 6;
		st_amfm_seek_threshold.SeekThresholdAdjacentChannel = 0xEE;
		st_amfm_seek_threshold.SeekThresholdMultipath = 0;
		st_amfm_seek_threshold.SeekThresholdSignalNoiseRatio = 150;
		st_amfm_seek_threshold.SeekThresholdMpxNoise = 0;
	}
	else
	{
		/*For MISRA C*/

	}
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Recvhdl = AMFM_FG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Recvhdl = AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}


	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_set_autoseek_thresholds_value(Recvhdl, &st_amfm_seek_threshold);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;

}
/********************************************************************************************/
/*	SOC_CMD_Config_Quality_Monitor                                                          */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Config_Quality_Monitor(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{

	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Recvhdl = AMFM_FG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Recvhdl = AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	memset(&pBcastQualityMonitor, 0x00, sizeof(EtalBcastQualityMonitorAttr));
	pBcastQualityMonitor.m_receiverHandle = Recvhdl;
	pBcastQualityMonitor.m_CbBcastQualityProcess = AMFM_Tuner_ctrl_Quality_Monitor_CB_Notify;
	pBcastQualityMonitor.m_Context = &st_context_cb;
	pBcastQualityMonitor.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;
	pBcastQualityMonitor.m_monitoredIndicators[0].m_InferiorValue = ETAL_INVALID_MONITOR;
	pBcastQualityMonitor.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	pBcastQualityMonitor.m_monitoredIndicators[0].m_UpdateFrequency = 2000;  // millisec Need to revisit.
	/*pBcastQualityMonitor.m_monitoredIndicators[1].m_MonitoredIndicator = EtalQualityIndicator_FmFrequencyOffset;
	pBcastQualityMonitor.m_monitoredIndicators[1].m_InferiorValue = ETAL_INVALID_MONITOR;
	pBcastQualityMonitor.m_monitoredIndicators[1].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	pBcastQualityMonitor.m_monitoredIndicators[1].m_UpdateFrequency = 250;  // millisec Need to revisit.
	pBcastQualityMonitor.m_monitoredIndicators[2].m_MonitoredIndicator = EtalQualityIndicator_FmMultipath;
	pBcastQualityMonitor.m_monitoredIndicators[2].m_InferiorValue = ETAL_INVALID_MONITOR;
	pBcastQualityMonitor.m_monitoredIndicators[2].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	pBcastQualityMonitor.m_monitoredIndicators[2].m_UpdateFrequency = 250;  // millisec Need to revisit.
	pBcastQualityMonitor.m_monitoredIndicators[3].m_MonitoredIndicator = EtalQualityIndicator_FmUltrasonicNoise;
	pBcastQualityMonitor.m_monitoredIndicators[3].m_InferiorValue = ETAL_INVALID_MONITOR;
	pBcastQualityMonitor.m_monitoredIndicators[3].m_SuperiorValue = ETAL_INVALID_MONITOR; // don't care
	pBcastQualityMonitor.m_monitoredIndicators[3].m_UpdateFrequency = 250;  // millisec Need to revisit.*/

	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_config_reception_quality_monitor(&FM_Quality_Monitorhdl, &pBcastQualityMonitor);
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}
/********************************************************************************************/	
/*	SOC_CMD_TUNE																			*/
/********************************************************************************************/
ETAL_STATUS SOC_CMD_TUNE(Tu32  freq, Te_AMFM_Tuner_Ctrl_Band Band)
{
	Recvhdl = AMFM_FG_recvhdl;
	UNUSED(Band);
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_tune_receiver(Recvhdl, freq);
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/	
/* SOC_CMD_SEEK_START																	    */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_SEEK_START(Te_RADIO_DirectionType e_SeekDirection, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band, Te_RADIO_SeekType e_SeekType)
{
	UNUSED(e_Band);
	if (e_SeekType == RADIO_PI_SEEK)
	{
		e_audiostatus = cmdAudioMuted;//for PI seek
	}
	else if (e_SeekType == RADIO_SEEK)
	{
		e_audiostatus = cmdAudioUnmuted;//for normal seek
	}
	else
	{
		//do nothing
	}

	Recvhdl = AMFM_FG_recvhdl;

	if (e_SeekDirection == RADIO_FRMWK_DIRECTION_UP)
	{
		e_direction = cmdDirectionUp;
	}
	else if (e_SeekDirection == RADIO_FRMWK_DIRECTION_DOWN)
	{
		e_direction = cmdDirectionDown;
	}
	else
	{
		/*Nothing*/
	}
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_autoseek_start(Recvhdl, e_direction, U32_Stepsize, e_audiostatus, dontSeekInSPS, TRUE);
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/	
/* SOC_CMD_SEEK_CONTINUE																    */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_SEEK_CONTINUE(Te_RADIO_DirectionType e_SeekDirection, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band)
{
	UNUSED(e_SeekDirection);
	UNUSED(U32_Stepsize);
	UNUSED(e_Band);

	Recvhdl = AMFM_FG_recvhdl;
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_autoseek_continue(Recvhdl);
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/	
/* SOC_CMD_SEEK_STOP																	    */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_SEEK_STOP(Te_AMFM_Tuner_Ctrl_Band e_Band)
{
	UNUSED(e_Band);
	e_Seek_Termination_Mode = lastFrequency;

	Recvhdl = AMFM_FG_recvhdl;
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_autoseek_stop(Recvhdl, e_Seek_Termination_Mode);
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/* SOC_CMD_SCAN_START                                                                       */
/********************************************************************************************/
ETAL_STATUS  SOC_CMD_SCAN_START(Te_RADIO_ScanType e_ScanType, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band)
{
	UNUSED(e_Band);
	e_direction = cmdDirectionUp;

	if (e_ScanType == RADIO_SCAN_FG)
	{
		e_audiostatus = cmdAudioUnmuted;//for scan
	}
	else if (e_ScanType == RADIO_SCAN_BG)
	{
		e_audiostatus = cmdAudioMuted;//for learn
	}
	else
	{
		//do nothing
	}

	Recvhdl = AMFM_FG_recvhdl;
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_autoseek_start(Recvhdl, e_direction, U32_Stepsize, e_audiostatus, dontSeekInSPS, TRUE);
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	SOC_CMD_SCAN_CONTINUE                                                                   */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_SCAN_CONTINUE(Te_RADIO_ScanType e_ScanType, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band Scan_Band)
{
	UNUSED(Scan_Band);

	e_direction = cmdDirectionUp;

	Recvhdl = AMFM_FG_recvhdl;

	if (e_ScanType == RADIO_SCAN_FG)
	{
		e_audiostatus = cmdAudioUnmuted;
		e_AMFM_Tuner_Ctrl_ETAL_Status = etal_autoseek_start(Recvhdl, e_direction, U32_Stepsize, e_audiostatus, dontSeekInSPS, FALSE);
	}
	else if (e_ScanType == RADIO_SCAN_BG)
	{
		e_AMFM_Tuner_Ctrl_ETAL_Status = etal_autoseek_continue(Recvhdl);
	}
	else
	{
		//do nothing
	}
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	 SOC_CMD_GET_QUALITY                                                                    */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_GET_QUALITY(EtalBcastQualityContainer *pBcastQuality_Container, Te_AMFM_Tuner_Ctrl_Band e_Band)
{

	if (e_Band == TUN_BAND_AM_LW || e_Band == TUN_BAND_AM_MW)
	{
		Recvhdl = AMFM_FG_recvhdl;
	}
	else if (e_Band == TUN_BAND_FM)
	{
		Recvhdl = AMFM_FG_recvhdl;
	}
	else
	{
		/*Added for MISRA*/
	}

	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_get_reception_quality(Recvhdl, pBcastQuality_Container);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	SOC_CMD_TUNER_DEINIT                                                                    */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_TUNER_DEINIT()
{
	if (e_configured_Band == TUN_BAND_FM)
	{
		e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_DESTROY_RDS(FOREGROUND_CHANNEL);

		if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
		{
			e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Destroy_Quality_Monitor(FOREGROUND_CHANNEL);
			if (e_AMFM_Tuner_Ctrl_ETAL_Status != ETAL_RET_SUCCESS)
			{
				/*FAILURE*/
				return e_AMFM_Tuner_Ctrl_ETAL_Status;
			}
			else
			{
				/*Success*/
			}
		}
		else
		{
			/*MisraC*/
		}
	}
	else
	{
		/*RDS ALREADY DESTROYED */
	}

	e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Destroy_Receiver(FOREGROUND_CHANNEL);

	if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
	{
		/* SUCCESS */
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: Destroy receiver failed due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
	}

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	 SOC_CMD_DESTROY_RDS                                                                    */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_DESTROY_RDS(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Stop_RDS(e_Frontend_type);
	if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
	{
		e_AMFM_Tuner_Ctrl_ETAL_Status = SOC_CMD_Destroy_Datapath(e_Frontend_type);
		if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
		{
			/* SUCCESS */
		}
		else
		{
			RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: DESTROY DATAPATH failed due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
		}
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: STOP RDS failed due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
	}
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/*	 SOC_CMD_Destroy_Datapath                                                               */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Destroy_Datapath(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{

	if(e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Datapath_Handle = &FM_FG_datapathhdl;
	}
	else if(e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Datapath_Handle = &FM_BG_datapathhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_destroy_datapath(Datapath_Handle);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/********************************************************************************************/
/* SOC_CMD_Stop_RDS                                                                         */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Stop_RDS(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	ETAL_HANDLE Receiv_handle = ETAL_INVALID_HANDLE;
	ETAL_STATUS  e_stop_rds_status;

	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiv_handle = AMFM_FG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Receiv_handle = AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	e_stop_rds_status = etal_stop_RDS(Receiv_handle);

	return e_stop_rds_status;
}

/********************************************************************************************/
/*	SOC_CMD_Destroy_Quality_Monitor                                                                */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Destroy_Quality_Monitor(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiver_handle = &FM_Quality_Monitorhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		/*Revisited during VPA mode*/
		//Receiver_handle = &AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}

	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_destroy_reception_quality_monitor(Receiver_handle);
	if (e_AMFM_Tuner_Ctrl_ETAL_Status == ETAL_RET_SUCCESS)
	{
		/* SUCCESS */
	}
	else
	{
		RADIO_DEBUG_LOG(RADIO_LOG_LVL_INFO, "[RADIO][AMFM_HAL]: Destroy reception quality monitor failed due to:%d", e_AMFM_Tuner_Ctrl_ETAL_Status);
	}
	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}
/*	SOC_CMD_Destroy_Receiver                                                                */
/********************************************************************************************/
ETAL_STATUS SOC_CMD_Destroy_Receiver(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type)
{
	
	if (e_Frontend_type == FOREGROUND_CHANNEL)
	{
		Receiver_handle = &AMFM_FG_recvhdl;
	}
	else if (e_Frontend_type == BACKGROUND_CHANNEL)
	{
		Receiver_handle = &AMFM_BG_recvhdl;
	}
	else
	{
		/*For MISRA C*/
	}
	
	e_AMFM_Tuner_Ctrl_ETAL_Status = etal_destroy_receiver(Receiver_handle);

	return e_AMFM_Tuner_Ctrl_ETAL_Status;
}

/*Old Functions*/
/********************************************************************************************/
/* SOC_CMD_Quality_Read                                                                     */
/********************************************************************************************/
void SOC_CMD_Quality_Read(SOC_QUAL *AMFMTuner_Qual_Param)
{
	if (AMFMTuner_Qual_Param != NULL)
	{
		// AMFMTuner_GetQualityRead(AMFMTuner_Qual_Param);
	}
	else
	{
		/*Added for MISRA*/

	}

}

/********************************************************************************************/
/* SOC_CMD_POR_Read                                                                         */
/********************************************************************************************/
void SOC_CMD_POR_Read(SOC_QUAL *AMFMTuner_Qual_Param)
{
	if (AMFMTuner_Qual_Param != NULL)
	{
		// AMFMTuner_GetQualityRead_POR(AMFMTuner_Qual_Param);
	}
	else
	{
		/*Added for MISRA*/

	}

}

/********************************************************************************************/
/*	SOC_CMD_AF_Update                                                                       */
/********************************************************************************************/
void SOC_CMD_AF_UPDATE(Tu32 u32_freq)
{
	UNUSED(u32_freq);
	//AMFMTuner_Tune_AF_UPDATE(u32_freq);

}

/********************************************************************************************/
/*	SOC_CMD_FM_Check                                                                        */
/********************************************************************************************/
void SOC_CMD_FM_CHECK(Tu32 u32_freq)
{
	UNUSED(u32_freq);
	//AMFMTuner_Tune_FM_CHECK(u32_freq);

}

/********************************************************************************************/
/*	SOC_CMD_FM_Jump                                                                         */
/********************************************************************************************/
void SOC_CMD_FM_JUMP(Tu32 u32_freq)
{
	UNUSED(u32_freq);
	//AMFMTuner_Tune_FM_JUMP(u32_freq);

}

/********************************************************************************************/
/*	SOC_CMD_Read_AF_Update                                                                  */
/********************************************************************************************/
void SOC_CMD_READ_AF_UPDATE(SOC_QUAL *AMFMTuner_AF_Quality)
{
	if (AMFMTuner_AF_Quality != NULL)
	{
		//AMFMTuner_Read_AF_Update(AMFMTuner_AF_Quality);
	}
	else
	{
		/*Added for MISRA*/
	}
}
/*=============================================================================
end of file
=============================================================================*/