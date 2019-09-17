/*=============================================================================
    start of file
=============================================================================*/
/****************************************************************************************************************/
/** \file AMFM_HAL_INTERFACE.h																		             *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														     *
*  All rights reserved. Reproduction in whole or part is prohibited											     *
*  without the written permission of the copyright owner.													     *
*																											     *
*  Project              : ST_Radio_Middleware																		             *
*  Organization			: Jasmin Infotech Pvt. Ltd.															     *
*  Module				: SC_AMFM_HAL													                         *
*  Description			: Response api's from SOC                                                                *
                                                                                                                 *																											*
*																											     *
******************************************************************************************************************/

#ifndef AMFM_HAL_INTERFACE_H_
#define AMFM_HAL_INTERFACE_H_


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "AMFM_Tuner_Ctrl_Instance_hsm.h"

/*-----------------------------------------------------------------------------
defines
-----------------------------------------------------------------------------*/
#define ETAL_RDS_MAX_BUFFER_SIZE (Tu8) (54)

#define TUNER_0							0

/*--------------------------------------------------------------------
Variables (extern)
-----------------------------------------------------------------------*/
extern Tu8   scan_index_FM;
extern Tu8   scan_index_AM;
extern Tbool e_audio_source_select;
/*-----------------------------------------------------------------------------
type definitions
-----------------------------------------------------------------------------*/
typedef struct
{
	Tu8 a;
	Tu32 b;
}Ts_Sink_Buffer;// to use in RDS callback


typedef enum
{
	FOREGROUND_CHANNEL,
	BACKGROUND_CHANNEL
}Te_AMFM_Tuner_Ctrl_Frontend_type;


/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/

void SOC_CMD_Quality_Read(SOC_QUAL *AMFMTuner_Qual_Param);
void SOC_CMD_POR_Read(SOC_QUAL *AMFMTuner_Qual_Param);
void SOC_CMD_AF_UPDATE(Tu32 u32_freq);
void SOC_CMD_FM_CHECK(Tu32 u32_freq);
void SOC_CMD_FM_JUMP(Tu32 u32_freq);
void SOC_CMD_READ_AF_UPDATE(SOC_QUAL *AMFMTuner_AF_Quality);

/***new func***/
ETAL_STATUS SOC_CMD_TUNER_CONFIGURATION(Te_AMFM_Tuner_Ctrl_Market e_Market, Te_AMFM_Tuner_Ctrl_Band e_Requested_Band, Te_AMFM_Tuner_State e_Tuner_State);

ETAL_STATUS SOC_CMD_TUNER_DEINIT(void);

ETAL_STATUS SOC_CMD_GET_QUALITY(EtalBcastQualityContainer *pBcastQuality_Container, Te_AMFM_Tuner_Ctrl_Band e_Band);

ETAL_STATUS SOC_CMD_Config_Receiver(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_Config_DataPath(Te_AMFM_Tuner_Ctrl_Frontend_type U32_Frontend_type);

ETAL_STATUS SOC_CMD_SET_SEEK_THRESHOLDS(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_Audio_Source_Select(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band);

ETAL_STATUS SOC_CMD_Destroy_Datapath(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_Destroy_Receiver(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_DESTROY_RDS(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_TUNE(Tu32  u32_freq, Te_AMFM_Tuner_Ctrl_Band Band);

ETAL_STATUS SOC_CMD_SEEK_START(Te_RADIO_DirectionType e_SeekDirection, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band, Te_RADIO_SeekType e_SeekType);

ETAL_STATUS SOC_CMD_SEEK_CONTINUE(Te_RADIO_DirectionType e_SeekDirection, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band);

ETAL_STATUS SOC_CMD_SEEK_STOP(Te_AMFM_Tuner_Ctrl_Band e_Band);

ETAL_STATUS  SOC_CMD_SCAN_START(Te_RADIO_ScanType e_ScanType, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band e_Band);

ETAL_STATUS SOC_CMD_SCAN_CONTINUE(Te_RADIO_ScanType e_ScanType, Tu32 U32_Stepsize, Te_AMFM_Tuner_Ctrl_Band Scan_Band);

ETAL_STATUS SOC_CMD_Change_Receiver_Band(Te_AMFM_Tuner_Ctrl_Market e_Market, Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_Start_RDS(Te_AMFM_Tuner_Ctrl_Band  e_Config_Band, Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_Stop_RDS(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

void RDS_cbFunc(Tu8* pBuffer, Tu32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

ETAL_STATUS SOC_CMD_Config_Quality_Monitor(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);

ETAL_STATUS SOC_CMD_Destroy_Quality_Monitor(Te_AMFM_Tuner_Ctrl_Frontend_type e_Frontend_type);
#endif /*AMFM_HAL_INTERFACE_H_*/

/*=============================================================================
    end of file
=============================================================================*/
