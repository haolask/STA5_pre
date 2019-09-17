/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_HAL_Fig_Decoder.h															           *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.											   *
*  All rights reserved. Reproduction in whole or part is prohibited								   *
*  without the written permission of the copyright owner.										   *
*																								   *
*  Project              : ST_Radio_Middleware																	   *
*  Organization			: Jasmin Infotech Pvt. Ltd.												   *
*  Module				: RADIO_DAB_HAL														       *
*  Description			: This file contains FIG related function declarations                     *
                         		                                                                   *
*																								   *
*																								   *
***************************************************************************************************/
#ifndef __DAB_HAL_FIG_DECODER_H__
#define __DAB_HAL_FIG_DECODER_H__

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/

#define DAB_TUNER_CTRL_FIG00_MIN_LENGTH						0x0005 /* Fig 0/0 min length is 5 bytes */
#define DAB_TUNER_CTRL_FIG02_MIN_LENGTH						0x0003 /* Fig 0/2 min length is 3 bytes */
#define DAB_TUNER_CTRL_FIG06_MIN_LENGTH						0x0002 /* Fig 0/6 min length is 2 bytes */
#define DAB_TUNER_CTRL_FIG018_MIN_LENGTH					0x0005 /* Fig 0/18 min length is 5 bytes */
#define DAB_TUNER_CTRL_FIG019_MIN_LENGTH					0x0004 /* Fig 0/19 min length is 5 bytes */
#define DAB_TUNER_CTRL_FIG021_MIN_LENGTH					0x0002 /* Fig 0/21 min length is 2 bytes */
#define DAB_TUNER_CTRL_FIG024_MIN_LENGTH					0x0003 /* Fig 0/24 min length is 3 bytes */
#define DAB_TUNER_CTRL_FIG025_MIN_LENGTH					0x0005 /* Fig 0/25 min length is 5 bytes */
#define DAB_TUNER_CTRL_FIG026_MIN_LENGTH					0x0006 /* Fig 0/26 min length is 3 bytes */
#define DAB_TUNER_CTRL_FIG_DEFAULT_MIN_LENGTH				0x0001 /* Fig default min length is 1 bytes */
#define DAB_TUNER_CTRL_FIG02_MIN_SUBCHID_BYTES				0x0002 /* Fig 0/2 default min bytes for Subchid extraction is 2 bytes */
#define DAB_TUNER_CTRL_FIG018_MIN_CLUSTERID_BYTES			0x0001 /* Fig 0/18 default min bytes for Clusterid extraction is 1 bytes */
#define DAB_TUNER_CTRL_FIG025_MIN_EID_BYTES					0x0002 /* Fig 0/25 default min bytes for EID extraction is 2 bytes */
#define DAB_TUNER_CTRL_FIG024_MIN_EID_BYTES					0x0002 /* Fig 0/24 default min bytes for Eid extraction is 2 bytes */
#define DAB_TUNER_CTRL_FIG06_MIN_SID_BYTES					0x0002 /* Fig 0/6 default min bytes for Sid extraction is 2 bytes */
#define LINKAGE_INVALID 									0XFF
#define MAX_LINKAGE_PARTS 									4
#define FM_RDS_PI							                ((Tu8) 0x80)
#define DAB_FREQ_INFO_DAB_ENTRY_SIZE                        3u   /* Frequency Info: Size of entries when R&M = DAB */
#define DAB_R_AND_M_DAB_ENSEMBLE             	            0u   /* Frequency Info: R&M (Range & Modulation) value -> DAB */
/**
 * @brief Frequency Information:
 * Size of the frequency information header in Byte
 */
#define DAB_FREQ_INFO_HEADER_SIZE             ((Tu8) 3)
/**
 * @brief Maximum value:
 * Size of frequency information list inside FIC
 */
#define DAB_MAX_NUM_FI_LIST                   				((Tu8) 26)
#define DAB_EID_INVALID                     				((Tu16)0x00)
#define DAB_NOTIFY_FREQUENCY_INFO_MAX_CTRL	                5u
#define DAB_NOTIFY_FREQUENCY_INFO_MAX_FREQ		            5u

/*--------------------------------------------------------------------------------------------------
    type definitions
--------------------------------------------------------------------------------------------------*/
/**
 * @brief Frequency Information
 * List of frequencies related to one Id
 */
typedef struct
{
	Tu8		Cn;
	Tu8 	OE;
	Tu8 	Pd;
	Tu8   	numFreq;                                            /**< Number of frequencies */
	Tu8 	LenOfFIList;  
	Tu16 	IDField;
	Tu8 	RangeModulation; 				
	Tbool 	b_ContinuityFlag;
	Tu8 	LenOfFreqList;  
	Tu8   	ControlField[DAB_NOTIFY_FREQUENCY_INFO_MAX_CTRL];   /**< Control Field from FIG 0/21 */    
	Tu32  	Frequency[DAB_NOTIFY_FREQUENCY_INFO_MAX_FREQ];    /**< List of frequencies */      
}Ts_Freq_Info;

/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/

void Fig_parsing_ServiceInformation(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
void Fig_parsing_AnnouncementSupport(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
void Fig_parsing_AnnouncementSwitching(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
void Fig_parsing_OE_AnnouncementSupport(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
void Fig_parsing_OE_AnnouncementSwitching(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
void DAB_Tuner_Ctrl_GetFigMinLength(Tu8 u8_extension,Tu8 *u8_FigMinLength);


Tbool findlsn(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 *position);
Tbool findlsnpart(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 position,Tu8 *part);
Tbool findlsnspace(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 *position);
void removefromdatabase(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 position);
void storelsnintodatabase(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Change_LA(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, Tu8 position);
void Fig_parsing_Linkinginfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
Tbool DAB_addEntryToFreqInfo(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me, Ts_Freq_Info *freqinfo, Tu8 freqPosition);
void Fig_parsing_Frequencyinfo(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,  char *msg);
Tbool findOEentry(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu32 u32_SId,Tu8 *u8_Index);
void findOEentryindatabase(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me,Tu8 *u8_Index);
void DAB_Tuner_Ctrl_ParseOeServ(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me, char *msg);
void Parsing_FIC0(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me,char *msg);
void Extract_Figdata(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me,char *msg);
void Fig_parsing_EnsembleInformation(Ts_Ensemble_Info *st_Ensemble_Info, Tbool *pb_ReConfigurationFlag,char *msg);
#endif
/*=============================================================================
    end of file
=============================================================================*/