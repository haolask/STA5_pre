/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_response.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application														     	*
*  Description			: This header file consists of declaration of all  Response APIs which will be		*
*						  provided to HMI-IF 											*
*																											*
*************************************************************************************************************/
#ifndef RADIO_MNGR_APP_RESPONSE_H
#define RADIO_MNGR_APP_RESPONSE_H

/** \file */
/** \page RADIO_MNGR_APP_RESPONSE_top Radio Manager Application Response package

\subpage RADIO_MNGR_APP_RESPONSE_Overview
\n
\subpage RADIO_MNGR_APP_RESPONSE_API_Functions
\n
*/

/**\page RADIO_MNGR_APP_RESPONSE_Overview Overview   
    \n
     Radio Manager Application Response package consists of Reponse API's which are called by Radio Manager Application in response to HMI IF.   
    \n\n
*/

/** \page RADIO_MNGR_APP_RESPONSE_API_Functions API Functions 
    <ul>
        <li> #Radio_Mngr_App_Response_StartTuner					: Response for start up request. </li>
        <li> #Radio_Mngr_App_Response_Shutdown      				: Response for the shut down request.</li>
		<li> #Radio_Mngr_App_Response_SelectBand					: Response for the Select Band Request. </li>
		<li> #Radio_Mngr_App_Response_PlaySelectSt					: Response for the Play select station request. </li>
		<li> #Radio_Mngr_App_Response_Mute         					: Response for the Mute Request. </li>
		<li> #Radio_Mngr_App_Response_DeMute         				: Response for the De Mute Request. </li>
		<li> #Radio_Mngr_App_Response_SeekUpDown         			: Response for the Seek Up/Down Request. </li>
		<li> #Radio_Mngr_App_Response_UpdateCurStationInfo_Display  : Response for the Current station information Request. </li>
		<li> #Radio_Mngr_App_Response_FMDABSwitch		         	: Response for the DABtoFM linking feature enable/disable Request. </li>
		<li> #Radio_Mngr_App_Response_PresetStore         			: Response for the Mixed Preset Store Request. </li>
		<li> #Radio_Mngr_App_Response_PresetRecall         			: Response for the Mixed Preset Recall Request. </li>
		<li> #Radio_Mngr_App_Response_PresetList         			: Response for the Mixed Preset List Request. </li>
		<li> #Radio_Mngr_App_Response_Update_StationList         	: Response for the Manual Station List Refresh Request. </li>
		<li> #Radio_Mngr_App_Response_RDS_Switch_Status         	: Response for the RDS following Request. </li>
		<li> #Radio_Mngr_App_Response_TuneUpDown         			: Response for the Tune Up/Down Request. </li> 
		<li> #Radio_Mngr_Response_Anno_Switch         				: Response for the Announcement switch Enable/Disable Request. </li> 
		<li> #Radio_Mngr_App_Response_AnnoCancel         			: Response for the Announcement cancel Request. </li>
		<li> #Radio_Mngr_App_Response_ClockTime         			: Response for the Clock Time Request. </li>
	
    </ul>
*/

/*-----------------------------------------------------------------------------
      							File Inclusions
-----------------------------------------------------------------------------*/

#include "radio_mngr_app_types.h"
#include "hmi_if_app_notify.h"


/*-----------------------------------------------------------------------------
    								defines
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    							Type definitions
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    					variable declarations (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
							Function Declaration 
-----------------------------------------------------------------------------*/


/*****************************************************************************************************/

/**	 \brief 				Response for start up request
*   \param[in]				e_StartupReplyStatus  	enum provides the reply status for the startup request
*   \param[out]				None
*   \pre					Startup Request is Received to Radio Manager Application and started the Radio Manager
*   \details 				This API provides the status of Radio Manager Application after receiving the startup request, 
*							if all initializations are completed with out any error during startup then the reply status contains success. 
*							otherwise it contains failure.
*   \post					Radio Application is in active state\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Response_StartTuner(Te_RADIO_ReplyStatus e_startTunerReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				Response for the shut down request
*   \param[in]				e_ShutdownReplyStatus   enum provides the reply status for the shutdown request
*   \param[out]				None
*   \pre					Shutdown request received to Radio Manager Application
*   \details 				This API provides the status of Radio Manager Application after receiving the shutdown request, 
*							if layers are completed shutdown with out any error during shutdown then the reply status contains success. 
*							otherwise it contains failure.
*   \post					Radio application is in inactive state\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Response_Shutdown(Te_RADIO_ReplyStatus e_ShutdownReplyStatus);

/*****************************************************************************************************/
/**	 \brief 				Response for the Select Band Request
*   \param[in]				pst_me_radio_mngr_inst  - from this structure getting the select band reply status enum
*   \param[out]				None
*   \pre					Select band request is received to the radio application
*   \details 				This API provides the response from Radio Application for select band request, 
*							if selcetion of requested band is succeeded then the reply status contains success otherwise it contains failure.
*   \post					Response sent to the HMI IF\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Response_SelectBand(Te_RADIO_ReplyStatus e_selectbandreplystatus, Te_Radio_Mngr_App_Band e_activeBand);

/*****************************************************************************************************/
/**	 \brief 				Response for the Play select station request
*   \param[in]				pst_me_radio_mngr_inst  	from this structure getting the Replystatus, Freq, Eid, Sid, SCid, Labels, PI and PSN
*   \param[out]				None
*   \pre					Tune request received by Radio Application
*   \details 				This API provides the response of tune request,
 *							if the requested station/frequency is tuned successfully,then e_TuneReplyStatus  contains success and
 *							pst_me_radio_mngr_inst contains Replystatus, Freq, Eid, Sid, SCid, Labels, PI and PSN.
*   \post					Sent response to the HMI IF.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Response_PlaySelectSt(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

/*****************************************************************************************************/
/**	 \brief 				Response for the Mute Request
*   \param[in]				e_muteReplayStatus  		enum Provides the reply status of Mute Request
*   \param[out]				None
*   \pre					Audio mute requested
*   \details 				This API provides the response of mute request after completing the mute function
*   \post					Mute response received\n
*   \ErrorHandling    		If mute response is fail, again send for mute request.
* 
******************************************************************************************************/
void Radio_Mngr_App_Response_Mute(Te_RADIO_ReplyStatus e_muteReplayStatus);

/*****************************************************************************************************/
/**	 \brief 				Response for the De Mute Request
*   \param[in]				e_demuteReplayStatus  		enum Provides the reply status of De Mute Request
*   \param[out]				None
*   \pre					Audio demute requested
*   \details 				This API provides the response of de mute request after completing the de mute function
*   \post					De Mute response received\n
*   \ErrorHandling    		If de mute response is fail, again send for de mute request.
* 
******************************************************************************************************/
void Radio_Mngr_App_Response_DeMute(Te_RADIO_ReplyStatus e_demutereplaystatus);

/******************************************************************************************************/
/**
 *  \brief                  Response for seek up/down Request.
 *  \param[in]              pst_me_radio_mngr_inst  provides the seek reply status of the seek up/down request and current station information.
 *  \param[out]             None
 *  \pre                    Radio Seek requested
 *  \details                This API provides the response of the seek up/down request after completing the seek request.
 *  \post                   Seek response receives.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_SeekUpDown(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Response for active band currently tuned station information
 *  \param[in]              e_Band-  active band , Freq- Currently tuned frequency, stName- Currently playing station name
 *  \param[out]             None
 *  \pre                    Active band tuned to any station.
 *  \details                This API provides the response of Active band currently tuned station information to the HMI.
 *  \post                   Sent Response of the Quality to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_UpdateCurStationInfo_Display(Te_Radio_Mngr_App_Band e_ActBand, Tu32 Freq, Tu8 *pu8_Name, Tu8 u8_CharSet, Tu8 *pu8_RadioText, Tu8 *pu8_ChannelName, Tu8 *pu8Ensemble_Label, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Response of DAB to FM Linking feature Enable/Disable request
 *  \param[in]              e_EnableDABtoFMLinkingStatus-  status of DABtoFM linking feature enable/disable request.
 *  \param[out]             None
 *  \pre                    Radio is in DAB mode.
 *  \details                This API provides the response of DABtoFM linking feature enable/disable request status.
 *  \post                   Sent Response of the DABtoFM linking feature enable/disable status.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_FMDABSwitch(Te_RADIO_ReplyStatus e_EnableDABFMLinkingStatus);
/******************************************************************************************************/
/**
 *  \brief                  Response of Preset Store 
 *  \param[in]              e_PresetStoreReplyStatus - Reply status for the Preset Store Request,
 *							st_PrestMixedList		 - Preset Mixed List Structure.
 *  \param[out]             None
 *  \pre                    Radio is having the updated preset Mixed list.
 *  \details                This API provides the response of current Preest Store request and Updated Preset Mixed List.
 *  \post                   Sent Reply status and preset Mixed list to HMI.\n
 *  \ErrorHandling          NA
 */
/***********************************************************************************************************************************************************************/
void Radio_Mngr_App_Response_PresetStore(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_RADIO_ReplyStatus e_PresetStoreReplyStatus);
/*****************************************************************************************************************************************************************/
/**
 *  \brief                  Response of Preset Recall
 *  \param[in]              pst_me_radio_mngr_inst - structure holiding the Preset mixed list recall reply status.
 *  \param[out]             None
 *  \pre                    Radio is tuned to preset station.
 *  \details                This API provides the response of preset recall request.
 *  \post                   Sent Preset Recall reply status.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_PresetRecall(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Response of preset list
 *  \param[in]              st_PrestMixedList - Preset mixed list structure.
 *  \param[out]             None
 *  \pre                    Radio is having the updated preset mixed list.
 *  \details                This API provides the updated preset mixed list.
 *  \post                   Sent preset mixed list to HMi.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_PresetList(Ts_Radio_Mngr_App_Preset_Mixed_List *st_PrestMixedList);
/******************************************************************************************************/
/**
 *  \brief                  Response of Update station list
 *  \param[in]              e_StationListReplyStatus - reply status of the update station list request.
 *  \param[out]             None
 *  \pre                    Radio updated with stationlist of active band
 *  \details                This API provides the response of manual station list refresh request.
 *  \post                   Sent replystatus response to the HMI .\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_Update_StationList(Te_RADIO_ReplyStatus e_StationListReplyStatus);
/******************************************************************************************************/
/**
 *  \brief                  Response of RDS following Request.
 *  \param[in]              e_RDSReplayStatus - Replystatus of RDS following request.
 *  \param[out]             None
 *  \pre                    Radio is enable/desable the RDS following.
 *  \details                This API provides the response of RDS following.
 *  \post                   Sent Reply status response to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_RDS_Switch_Status(Te_RADIO_ReplyStatus e_RDSSwitchStatus);
/******************************************************************************************************/
/**
 *  \brief                  Response for tune up/down request.
 *  \param[in]              e_Band-  active band , Freq- Currently tuned frequency, stName- Currently playing station name
 *  \param[out]             None
 *  \pre                    Active band tuned to any station.
 *  \details                This API provides the response of Active band currently tuned station information to the HMI.
 *  \post                   Sent Response of the Quality to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_TuneUpDown(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Response of Announcement switch Request.
 *  \param[in]              e_TA_Switch_Status - Replystatus of Announcement switch request.
 *  \param[out]             None
 *  \pre                    Radio is enable/disable the Announcement switch.
 *  \details                This API provides the response of Announcement switch.
 *  \post                   Sent Reply status response to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_Response_TA_Anno_Switch(Te_RADIO_ReplyStatus e_TA_Switch_Status);
/******************************************************************************************************/
/**
 *  \brief                  Response of Announcement cancel Request.
 *  \param[in]              e_AnnoCancelStatus - Replystatus of Announcement cancel request.
 *  \param[out]             None
 *  \pre                    Radio is playing announcement station.
 *  \details                This API provides the response of Announcement cancel Request.
 *  \post                   NA
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_AnnoCancel(Te_RADIO_ReplyStatus e_AnnoCancelStatus);
/******************************************************************************************************/
/**
 *  \brief                  Response of Clock Time Request.
 *  \param[in]              e_CTInfoReplyStatus - Replystatus of Clock Time Request.
 *  \param[out]             None
 *  \pre                    Radio is received CT request with ENG mode is ON.
 *  \details                This API provides the response of Clock Time Request based on Clock Time availability and ENG mode Status.
 *  \post                   NA
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_ClockTime(Te_RADIO_ReplyStatus e_CTInfoReplyStatus);
/******************************************************************************************************/
/**
 *  \brief                  Response of cancelling of Manual updation of station List.
 *  \param[in]              e_CancelManualSTLReplyStatus - Replystatus of cancelling of Manual updation of station List.
 *  \param[out]             None
 *  \pre                    Radio is received cancelling of Manual updation of station List request.
 *  \details                This API provides the response of cancelling of Manual updation of station List Request.
 *  \post                   NA
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_CancelManualSTlUpdate(Te_RADIO_ReplyStatus e_CancelManualSTLReplyStatus);
/******************************************************************************************************/
/**
 *  \brief                  Response of Info Announcement switch Request.
 *  \param[in]              e_Info_Switch_Status - Replystatus of Info Announcement switch request.
 *  \param[out]             None
 *  \pre                    Radio is enable/disable the Info Announcement switch.
 *  \details                This API provides the response of Info Announcement switch.
 *  \post                   Sent Reply status response to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_Response_Info_Anno_Switch(Te_RADIO_ReplyStatus e_Info_Switch_Status);
/******************************************************************************************************/
/**
 *  \brief                  Response of Multiplex switch Request.
 *  \param[in]              e_MultiplexSettings_ReplyStatus - Replystatus of Multiplex switch request.
 *  \param[out]             None
 *  \pre                    Radio is enable/disable the Multiplex switch.
 *  \details                This API provides the response of Multiplex switch.
 *  \post                   Sent Reply status response to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Response_Multiplex_Switch_Status(Te_RADIO_ReplyStatus e_MultiplexSettings_ReplyStatus);
/******************************************************************************************************/

void Radio_Mngr_App_Response_BGStationInfo(Te_Radio_Mngr_App_Band e_ActSRC, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);

void Radio_Mngr_App_Response_Power_ON(Te_RADIO_ReplyStatus e_PowerOnReplyStatus);

void Radio_Mngr_App_Response_Power_OFF(Te_RADIO_ReplyStatus e_PowerOffReplyStatus);

void Radio_Mngr_App_Response_Factory_Reset(Te_RADIO_ReplyStatus e_FactoryReset_ReplyStatus);

#endif /* End of RADIO_MNGR_APP_RESPONSE_H */












