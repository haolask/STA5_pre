/*==================================================================================================
    start of file
==================================================================================================*/
/**************************************************************************************************/
/** \file DAB_Tuner_Ctrl_app.h 																	   *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.											   *
*  All rights reserved. Reproduction in whole or part is prohibited								   *
*  without the written permission of the copyright owner.										   *
*																								   *
*  Project              : ST_Radio_Middleware													   *
*  Organization			: Jasmin Infotech Pvt. Ltd.												   *
*  Module				: Radio DAB Tuner Control												   *
*  Description			: This file contains function declarations of DAB Tuner Control            *
                          Initialisation APIs.					                                   *
*																								   *
*																								   *
***************************************************************************************************/

#ifndef DAB_TUNER_CTRL_APP_
#define DAB_TUNER_CTRL_APP_

/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "DAB_Tuner_Ctrl_Types.h"
/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define DAB_TUNER_CTRL_AUDIO_TYPE_MPEG1_LAYER_II		(0x00u)
#define DAB_TUNER_CTRL_AUDIO_TYPE_MPEG2_LAYER_II		(0x01u)
#define DAB_TUNER_CTRL_AUDIO_TYPE_AAC_PLUS_DAB_PLUS		(0x02u)
#define DAB_TUNER_CTRL_AUDIO_TYPE_AAC_PLUS_TDMB_PLUS	(0x03u)
#define DAB_TUNER_CTRL_AUDIO_TYPE_BSAC 					(0x04u)
#define MAXIMUM_LEVEL_DATA					0x2000
#define	MINIMUM_LEVEL_DATA					0x1000

/*--------------------------------------------------------------------------------------------------
    Function declarations
--------------------------------------------------------------------------------------------------*/

/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control component initialisation.
*   \param[in]				  None
*   \param[out]				  None
*   \pre-condition			  System is powered on.
*   \details                  Whenever the system is powered on, platform invokes DAB Tuner Control 
                              component. 
*   \post-condition			  DAB Tuner control is initialised.
*   \ErrorHandling    		  None
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_Component_Init();
/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM start.
*   \param[in]				  dest_cid
*   \param[in]				  comp_msgid
*   \pre-condition			  DAB Tuner Control main HSM receives up notification from SoC.
*   \details                  When DAB Tuner Control main HSM receives up notification from SoC it
                              starts instance HSM via this API.
*   \post-condition			  DAB Tuner Control instance HSM is in active start state.
*   \ErrorHandling    		  N/A 
* 
***************************************************************************************************/
void DAB_Tuner_Ctrl_hsm_inst_start(Tu16 dest_cid, Tu16 comp_msgid);

/**************************************************************************************************/
/**	 \brief                   DAB Tuner Control instance HSM stop.
*   \param[in]				  dest_cid
*   \param[in]				  comp_msgid
*   \pre-condition			  DAB Tuner Control main HSM receives shut down request from upper layer.
*   \details                  When DAB Tuner Control main HSM receives shut down request from upper
                              layer, then instance HSM sends shut request to instance HSM via this API.
*   \post-condition			  DAB Tuner Control instance HSM is in inactive state.
*   \ErrorHandling    		  N/A 
* 
***************************************************************************************************/

/**
 * @brief Structure comprises the notification information of ProgrammeServListChanged_not 
 */
void DAB_Tuner_Ctrl_hsm_inst_stop(Tu16 dest_cid, Tu16 comp_msgid);
void Update_CurSId_Hardlink_DataBase(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void ClearLinkingData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_CheckDelayValues(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Updated_CurrentEnsembleData(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_CheckForDABAlternative(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_TUNER_CTRL_Check_Same_PI(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me , Ts_Tuner_Ctrl_ServiceInfo Serviceinfo[] ,Tu16 Noofservices);

void UpdateCurrentEnsembleServiceList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_ProgrammeServListChanged_not);
void UpdateEnsembleInfo(Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData);

void UpdateServiceInfo(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData);

void UpdateServiceCompInfo(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData);

void UpdateCurrentServiceCompsToServiceCompInfo(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData, Tu8 u8_ServiceIndex);
void DAB_TUNER_CTRL_Tune_To_Same_PI(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);


void Check_Hardlinks_For_Tuned_SID(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void Notify_Hardlinks_To_FM(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
Tbool Check_Hardlink_Repeat(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Check_Leveldata(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
Tbool CompareCurrentEnsembleServiceList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_ProgrammeServListChanged_not, Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply) ;
void UpdateCurrentEnsembleProperties(Ts_DabTunerGetEnsembleProperties_reply	*pst_GetEnsembleProperties_reply,Ts_DabTunerMsg_R_ScanStatus_Not *pst_DAB_Tuner_Ctrl_ScanNotification) ;
void UpdateCurrentEnsembleProgramList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply,Ts_DabTunerMsg_R_ScanStatus_Not *pst_DAB_Tuner_Ctrl_ScanNotification) ;
Tu8 FindFrequencyIndex(Tu32 u32_Station_Frequency);
Tu8 FindSeekFrequencyIndex(Tu32 u32_SeekStartFrequency, Te_RADIO_DirectionType e_Direction);
void DAB_Tuner_Ctrl_UpdateDABServiceType(Ts_GetAudioProperties_repl *pst_GetAudioProperties_repl,Te_Tuner_Ctrl_ServiceType *pe_ServiceType);
Ts32 DAB_Tuner_Ctrl_CaluculateDelayValues(Ts32 delay1 , Ts32 delay2);
void Tuner_Ctrl_Sort_CurrEnsembleProgList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *CurrEnsembleProgList);
Ts32 Tuner_Ctrl_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size);
void Tuner_Ctrl_Sort_ProgrammeServListChanged_not(Ts_DabTunerMsg_ProgrammeServListChanged_not *pst_ProgrammeServListChanged_not);
Tbool CompareCurrentEnsembleServices(Ts_DabTunerMsg_ProgrammeServListChanged_not *pst_ProgrammeServListChanged_not, Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply) ;
void DAB_Tuner_Ctrl_Check_DAB_DAB_In_LearnMem(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Check_For_Alternate(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
Tbool DAB_Tuner_Ctrl_DAB_HARDLINK_Freq_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
Tbool DAB_Tuner_Ctrl_DABAlternate_EID_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
Tbool DAB_Tuner_Ctrl_DABAlternate_Freq_Available_Check(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void Update_Stationlist_Into_LearnMem(void);
void DAB_Tuner_Ctrl_Check_Same_PI_InLearnMem(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Update_LearnMem_After_Tune(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void UpdateStationListInfo(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
Tbool DAB_Tuner_Ctrl_Check_Service_Available(Ts_dab_tuner_ctrl_inst_hsm* DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Update_Label(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *pst_GetCurrEnsembleProgListReply);
void DAB_Tuner_Ctrl_Check_Service_Present_In_Stl(Ts_dab_tuner_ctrl_inst_hsm *DAB_Tuner_Ctrl_me);
void DAB_Tuner_Ctrl_Update_EnsembleLabel(Ts_Tuner_Ctrl_CurrentEnsembleInfo *pst_currentEnsembleData);
#endif
/*=============================================================================
    end of file
=============================================================================*/