/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_request.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application														     	*
*  Description			: This header file consists of declaration of all  Request APIs which will be		*
*						  provided to HMI-IF 											*
*																											*
*************************************************************************************************************/
#ifndef RADIO_MNGR_APP_REQUEST_H
#define RADIO_MNGR_APP_REQUEST_H

/** \file */
/** \page RADIO_MNGR_APP_REQUEST_top Radio Manager Application Request package

\subpage RADIO_MNGR_APP_REQUEST_Overview
\n
\subpage RADIO_MNGR_APP_REQUEST_API_Functions
\n
*/

/**\page RADIO_MNGR_APP_REQUEST_Overview Overview   
    \n
     Radio Manager Application Request package cosists of Request API's which are called by HMI IF.   
    \n\n
*/

/** \page RADIO_MNGR_APP_REQUEST_API_Functions API Functions 
    <ul>
        <li> #Radio_Mngr_App_Request_StartTuner					: Request for startup. </li>
        <li> #Radio_Mngr_App_Request_ShutDownTuner				: Request for shutdown. </li>
		<li> #Radio_Mngr_App_Request_SelectBand					: Request for select band. </li>
		<li> #Radio_Mngr_App_Request_PlaySelectSt				: Request for select station from station list. </li>
		<li> #Audio_Manager_Request_Mute         				: Request for mute the audio. </li>
		<li> #Audio_Manager_Request_DeMute         				: Request for de mute the audio. </li>
		<li> #Radio_Mngr_App_Request_SeekUpDown         		: Request for Seek Up/Down. </li>
		<li> #Radio_Mngr_App_Request_GetRadioStationList    	: Request for Active Band Station List. </li>
		<li> #Radio_Mngr_App_Request_EnableDABFMLinking	    	: Request for Enable/Disable Following Feature. </li>
		<li> #Radio_Mngr_App_Request_GetAFList_Diag         	: Request for Currently tuned FM Station AF List. </li>
		<li> #Radio_Mngr_App_Request_GetQuality_Diag        	: Request for Currently tuned station Quality. </li>
		<li> #Radio_Mngr_App_Request_GetCurStationInfo_Diag 	: Request for Currently tuned station information. </li>
		<li> #Radio_Mngr_App_Request_ObtainStationList_Diag 	: Request for stationlist info during diag/ENG Mode. </li>
		<li> #Radio_Mngr_App_Request_PresetStore         		: Request for Mixed Preset Store. </li>
		<li> #Radio_Mngr_App_Request_PresetRecall         		: Request for Mixed Preset Recall. </li>
		<li> #Radio_Mngr_App_Request_GetPresetList         		: Request for Get Preset Mixed Preset List. </li>
		<li> #Radio_Mngr_App_Request_UpdateStationList      	: Request for Manual Refresh Station List. </li>
		<li> #Radio_Mngr_App_Request_TuneUpDown         		: Request for Tune Up/Down. </li>
		<li> #Radio_Mngr_App_Request_RadioStatus         		: Request for status of radio. </li>
		<li> #Radio_Mngr_App_Request_RDSSettings	        	: Request for Enable/Disable AF Feature. </li>
		<li> #Radio_Mngr_App_Request_EnableAnnouncement     	: Request for Enable/Disable Announcement Feature. </li>
		<li> #Radio_Mngr_App_Request_AnnoCancel         		: Request for Announcement cancel. </li>
		<li> #Radio_Mngr_App_Request_AMFMReTune         		: Request for AMFM retune. </li>
		<li> #Radio_Mngr_App_Request_ENG_Mode         			: Request for Enable/Disable ENG Mode Feature. </li>
		<li> #Radio_Mngr_App_Request_TuneByFreq         		: Request for tune by givien frequency. </li> 
		<li> #Radio_Mngr_App_Internal_HMI_Request_Message   	: Request for self posting the message. </li> 
		<li> #Radio_Mngr_App_Inst_Hsm_Start         			: Request for instant hsm to start. </li>
		<li> #Radio_Mngr_App_Request_RadioSRCActivateDeActivate : Request for Radio Source Activate and Deactivate Request. </li>
		<li> #Radio_Mngr_App_Request_GetClockTime         		: Request for Get Clock Time. </li>
    </ul>
*/




/*-----------------------------------------------------------------------------
      							File Inclusions
-----------------------------------------------------------------------------*/

#include "radio_mngr_app_types.h"

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

/**	 \brief 				Request for startup.
*   \param[in]				e_Market- market type, u8_RadioComponentInfo- Component Type
*   \param[out]				None.
*   \pre					Radio Manager Application is not in active state.
*   \details 				After receiving startup request, Radio Manager Application request AMFM and DAB
*							application for startup based on the Component Info.
*   \post					Radio Manager Application is in active state. \n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Request_StartTuner(Te_Radio_Mngr_App_Market e_Market, Tu8 u8_RadioComponentInfo);

/*****************************************************************************************************/
/**	 \brief 				Request for shutdown.
*   \param[in]				None.
*   \param[out]				None.
*   \pre					Radio Manager Application is in active state.
*   \details 				After receiving shutdown request, Radio Manager Application request to AMFM and DAB applications, 
*							for shutdown request.
*   \post					Radio Manager Application is in inactive state.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Request_ShutDownTuner(void);

/*****************************************************************************************************/
/**	 \brief 				Request for select band.
*   \param[in]				e_Band- Requested band.
*   \param[out]				None.
*   \pre					Radio Manager Application is in active state to receive any request.
*   \details 				After receiving select band request, Radio Manager Application stores current station information into LSM.
*							and activate the requested band and sends last played index to Application layer for select station.
*   \post					Radio Manager Application selected the requested band\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Request_SelectBand(Te_Radio_Mngr_App_Band e_Band);

/*****************************************************************************************************/
/**	 \brief 				Request for select station from station list.
*   \param[in]				u8_Index- the index represents the station in station list.
*   \param[out]				None.
*   \pre					Radio application is in active state.
*   \details 				After receiving select station request, Radio Manager fetch the respective station tunable info from 
*							the station list.
*   \post					State will move to Audio Mute for audio mute.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Request_PlaySelectSt(Tu8 u8_Index);

/*****************************************************************************************************/
/**	 \brief 				Request for mute the audio.
*   \param[in]				eBand- Active band.
*   \param[out]				None.
*   \pre					Radio application is in active state.
*   \details 				After receiving mute request audio manager will mute the audio and send the mute status.
*   \post					Audio muted.\n
*   \ErrorHandling    		State move to Active Idle state.
* 
******************************************************************************************************/
void Audio_Manager_Request_Mute(Te_Radio_Mngr_App_Band eBand);

/*****************************************************************************************************/
/**	 \brief 				Request for de mute the audio.
*   \param[in]				eBand- Active band.
*   \param[out]				None.
*   \pre					Radio application is in active state.
*   \details 				After receiving de mute request audio manager will de mute the audio and send the de mute status.
*   \post					Audio de muted.\n
*   \ErrorHandling    		Send for de mute request second time.
* 
******************************************************************************************************/
void Audio_Manager_Request_DeMute(Te_Radio_Mngr_App_Band eBand);

/******************************************************************************************************/
/**
 *  \brief                  Request to seek up/down.
 *  \param[in]              e_direction- direction Seek direction up/down.
 *  \param[out]             None.
 *  \pre                    Radio should be tuned to any station.
 *  \details                After receiving the Seek request Radio manager will seek the next valid station based on the given direction.
 *  \post                   Radio tuned to next seek station based on the direction.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_SeekUpDown(Te_RADIO_DirectionType e_direction);
/******************************************************************************************************/
/**
 *  \brief                  Request for Get Radio Stationlist.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager having station list of Active Band.
 *  \details                After receiving the Getstation list, Radio manager shall get the existed station list of active band.
 *  \post                   Radio manager written the active band station list in HMI IF strtucture.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_GetRadioStationList(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for Enable/Disable the DAB to FM linking Feature.
 *  \param[in]              e_DABtoFMLinkingSwitch -switch for enable/disable the feature.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode with current in DAB Band.
 *  \details                After receiving the Enable DAB to FM linking request, Radio Manager request to DAB application for Enable/Disable .
 *                          information of active band to HMI.
 *  \post                   Radio manager provides the status of the DAB to FM liniking feature.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_EnableDABFMLinking(Te_Radio_Mngr_App_DABFMLinking_Switch e_DABtoFMLinkingSwitch);

/******************************************************************************************************/
/**
 *  \brief                  Request for Get AF list for the Diag Mode.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager having AF list.
 *  \details                After receiving the Get AF list request, Radio Manager provide the Currently tuned FM station AF
 *							list to the HMI.
 *                          information of active band to HMI.
 *  \post                   Radio manager written the FM currently tuned station AF list to HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_GetAFList_Diag(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for Get the Quality of Active band for the Diag mode.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager tuned to any station.
 *  \details                After receiving the Get Quality request, Radio Manager provide the Currently tuned station.
 *							of active band Quality information to the HMI.
 *                          information of active band to HMI.
 *  \post                   Radio manager written the active band current station Quality information in HMI IF strtucture.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_GetQuality_Diag(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for Get the Current station informatio of Active band for the Diag mode.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager tuned to any station.
 *  \details                After receiving the Get Current Station Info request, Radio Manager provide the Currently tuned station.
 *                          information of active band to HMI.
 *  \post                   Radio manager written the active band current station information in HMI IF strtucture.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_GetCurStationInfo_Diag(void);
/******************************************************************************************************/
/**
 *  \brief                  Request of Radio manager for Station list in Diag Mode.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager having the active band updated station list.
 *  \details                After receiving the Diag Station list request, Radio Manager shall provide the active band statio list.
 *  \post                   Radio manager sent the active band station list to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_ObtainStationList_Diag(void);
/******************************************************************************************************/
/**
 *  \brief                  Request to Radio manager for Preset Store.
 *  \param[in]              u8_Preset_Index - To store the current station into the selected location in preset list.
 *  \param[out]             None.
 *  \pre                    Radio manager tuned to any station in active band.
 *  \details                After receiving the Preset Store, Radio Manager shall the active band current tuned station,
 *							in selected mixed preset list index with active band.
 *  \post                   Radio manager updated the latest Preset Mixed List to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_PresetStore(Tu8 u8_Preset_Index);
/******************************************************************************************************/
/**
 *  \brief                  Request to Radio manager for Preset Recall.
 *  \param[in]              u8_Preset_Index - Index of Preset Recall station in List.
 *  \param[out]             None.
 *  \pre                    Radio Manager having the Preset Mixed list.
 *  \details                After receiving the Preset recall request, radio manager shall change the band based on selected.
 *							preset station and tune to the selected preset station.
 *  \post                   Tuned to the selected preset station index.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_PresetRecall(Tu8 u8_Preset_Index);
/******************************************************************************************************/
/**
 *  \brief                  Request to Radio manager for send the updated Preset Mixed list.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager having the updated Preset Mixed List.
 *  \details                After receiving the Get Preset list, Radio manager shall provide the updated Preset Mixed list.
 *  \post                   Radio manager provided the Preset Mixed List to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_GetPresetList(void);
/******************************************************************************************************/
/**
 *  \brief                  Request to Radio manager for Refresh the Station list of active band.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in any band.
 *  \details                After receiving the Updatestation list Radio manager update the new staiton list of active band.
 *  \post                   Radio manager updated with new station list of active band.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_UpdateStationList(void);
/******************************************************************************************************/
/**
 *  \brief                  Request to Tune up/down.
 *  \param[in]              direction Tune direction up/down.
 *  \param[out]             None.
 *  \pre                    Radio should be tuned to any station.
 *  \details                After receiving the Tune request Radio manager will tune the next valid station based on the given direction.
 *  \post                   Radio tuned to next tune station based on the direction.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_TuneUpDown(Te_RADIO_DirectionType e_TuneUpDownDirection);
/******************************************************************************************************/
/**
 *  \brief                  Request for Enable/Disable the AF Feature.
 *  \param[in]              e_RDSSettings_Request -switch for enable/disable the AF feature.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving the Enable AF request, Radio Manager request FM/DAB for AF switching.
 *  \post                   Radio manager provides the status of the AF feature.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_RDSSettings(Te_Radio_Mngr_App_RDSSettings e_RDSSettings_Request);
/******************************************************************************************************/
/**
 *  \brief                  Request for Enable/Disable the TA Announcement Feature.
 *  \param[in]              e_TA_Anno_Switch -switch for enable/disable the Announcement feature.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving the Enable Announcement request, Radio Manager request FM/DAB for Announcement.
 *  \post                   Radio manager provides the status of the Announcement feature.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_EnableTAAnnouncement(Te_Radio_Mngr_App_EnableTAAnno_Switch e_TA_Anno_Switch);
/******************************************************************************************************/
/**
 *  \brief                  Request for cancelling the Announcement Feature.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode & announcement is running currently.
 *  \details                After receiving the cancel Announcement request, Radio Manager request FM/DAB for Announcement cancel.
 *  \post                   Radio manager provides the status of the Announcement cancel feature.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_AnnoCancel(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for AMFM Retune for component status Feature.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving the AMFM Retune request, Radio Manager request DAB for AMFM component status normal.
 *  \post                   Radio manager provides the status of component to HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_AMFMReTune(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for Engineering mode.
 *  \param[in]              e_EngModeRequest -switch for enable/disable the ENG mode feature.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving the Engineering mode request, Radio Manager request FM/DAB for ENG Mode.
 *  \post                   None.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_ENG_Mode(Te_Radio_Mngr_App_Eng_Mode_Request e_EngModeRequest);
/******************************************************************************************************/
/**
 *  \brief                  Request for tune by frequency.
 *  \param[in]              u32_Freq - Frequency information,pu8_ChannelName-DAB ChannelName. 
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving the tune by frequency request, Radio Manager request FM/DAB for tune.
 *  \post                   Radio tuned to the given frequency.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_TuneByFreq(Tu32 u32_Freq, Tu8 *pu8_ChannelName);
/******************************************************************************************************/
/**
 *  \brief                  Request for self posting the message to Radio manager layer itself.
 *  \param[in]              pst_msg- Msg structure to Self post Message.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving internal request message system shall process accordingly. 
 *  \post                   Self posted Message should be handled properly.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Internal_HMI_Request_Message(Ts_Sys_Msg *pst_msg);
/******************************************************************************************************/
/**
 *  \brief                  Request for instant hsm to start.
 *  \param[in]              e_Market  - Market Information.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving instant hsm start request, Radio Manager should request for AM/FM/DAB for same.
 *  \post                   AM/FM/DAB shall provide the response of instant hsm start done.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Inst_Hsm_Start(Te_Radio_Mngr_App_Market e_Market);
/******************************************************************************************************/
/**
 *  \brief                  Request for Radio Source Activate or Deactivate
 *  \param[in]              e_Band	              - Band information to which band need to Activate/Deactivate.
 *  \param[in]              e_ActivateDeactivate  - Source Activte Deactivate Request
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode and Requested Source Activate and Deactivate Band should be in Backgrournd
 *  \details                Radio shall Activate/Deactivate the Source based on Request only when the Requested SRC is  in Background
 *  \post                   Requested Activated/Deactivated Source will be Activated/Deactivated as per the Request.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_RadioSRCActivateDeActivate(Te_Radio_Mngr_App_Band e_Band, Te_Radio_Mngr_App_SRC_ActivateDeActivate e_ActivateDeactivate);
/******************************************************************************************************/
/**
 *  \brief                  Request for Clock time from AMFM TUNER in ENG Mode
 *  \param[in]              None.
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode and FM is in Active band and ENG mode is ON.
 *  \details                After receiving the Clock request in ENG mode, we are giving the Clock time to System based on AMFM TUNER giving time.
 *  \post                   Updated the Clock time to System.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_GetClockTime(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for Cancelling of Manual updation of station List
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode and Manual Update is going on.
 *  \details                After receiving the Cancelling of Manual updation of STL, we are processing New request or waiting for user input.
 *  \post                   Manual updation of station list is cancelled.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_ManualSTLUpdateCancel(void);
/******************************************************************************************************/
/**
 *  \brief                  Request for Enable/Disable the Info Announcement Feature.
 *  \param[in]              e_TA_Anno_Switch -switch for enable/disable the Announcement feature.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in active mode.
 *  \details                After receiving the Enable Announcement request, Radio Manager request DAB for Info Announcement.
 *  \post                   Radio manager provides the status of the Announcement feature.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_EnableAnnouncementInfo(Te_Radio_Mngr_App_EnableInfoAnno_Switch e_Info_Anno_Switch);
/******************************************************************************************************/
/**
 *  \brief                  Request for Radio Power ON
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in Sleep Mode
 *  \details                After receiving Request for Radio Power ON, tuning to LSM band based on active band.
 *  \post                   Radio is Power ON.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_RadioPowerON(void);

/******************************************************************************************************/
/**
 *  \brief                  Request for Radio Power OFF
 *  \param[in]              None.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in Active Mode
 *  \details                After receiving Request for Radio Power OFF, Radio will be in sleep state.
 *  \post                   Radio is Power OFF.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_RadioPowerOFF(void);

/******************************************************************************************************/
/**
 *  \brief                  Request for play station from searched stl
 *  \param[in]              u8_index - indesx to play station from searched stl.
 *  \param[out]             None.
 *  \pre                    Radio Manager is in Active Mode
 *  \details                Tune to given index from searched station list
 *  \post                   Selected station will be tuned.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Request_Play_SelectStation_From_StlSearch(Tu8 u8_index);

void Radio_Mngr_App_Request_FactoryReset(void);

void Radio_Mngr_App_Request_InstHSM_FactoryReset(void);

void Radio_Mngr_App_Response_InstHSMFactoryReset(void);

void Radio_Mngr_App_Request_StationSelect_From_EnsembleList(Tu8 u8_ServiceIndex);
void Radio_Mngr_App_Request_MultiplexList_Switch(Te_Radio_Mngr_App_Multiplex_Switch e_MultiplexSettings);
#endif /* End of RADIO_MNGR_APP_REQUEST_H */