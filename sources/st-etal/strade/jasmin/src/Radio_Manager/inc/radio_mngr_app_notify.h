/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_notify.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application															     	*
*  Description			: This header file consists of declaration of all Notification APIs which send		*
*						  send notifications to HMI IF												*
*																											*
*************************************************************************************************************/
#ifndef RADIO_MNGR_APP_NOTIFY_H
#define RADIO_MNGR_APP_NOTIFY_H


/** \file */
/** \page RADIO_MNGR_APP_NOTIFY_top Radio Manager Application Notify package

\subpage RADIO_MNGR_APP_NOTIFY_Overview
\n
\subpage RADIO_MNGR_APP_NOTIFY_API_Functions
\n
*/

/**\page RADIO_MNGR_APP_NOTIFY_Overview Overview   
    \n
     Radio Manager Application Notify package consists of Notify API's which are called by Radio Manager Application in Notify to HMI IF.   
    \n\n
*/

/** \page RADIO_MNGR_APP_NOTIFY_API_Functions API Functions 
    <ul>
        <li> #Radio_Mngr_App_Notify_StationList      	 		: Notifies the station list information. </li>
		<li> #Radio_Mngr_App_Notify_UpdateScanStatus     		: Notifies the Active Band Scan Status. </li>
		<li> #Radio_Mngr_App_Notify_DABFMLinkingStatus   		: Notifies the DAB to FM liking Status. </li>
		<li> #Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag   : Notifies the currently tuned station information in diag/ENG Mode. </li>
		<li> #Radio_Mngr_App_Notify_AFList_Diag      	 		: Notifies the AF list information in diag/ENG Mode. </li>
		<li> #Radio_Mngr_App_Notify_Quality_Diag      	 		: Notifies the Quality information in diag/ENG Mode. </li>
		<li> #Radio_Mngr_App_Notify_UpdateSTL_Diag      		: Notifies the station list information in diag/ENG Mode. </li>
		<li> #Radio_Mngr_App_Notify_Announcement      	 		: Notifies the Announcement station information. </li>
		<li> #Radio_Mngr_App_Notify_AFStatus      	 			: Notifies the AF status information. </li>
		<li> #Radio_Mngr_App_Notify_Components_Status      	 	: Notifies the Component status information. </li>
		<li> #Radio_Mngr_App_Notify_Activity_State      	 	: Notifies the avtivity status of radio. </li>
		<li> #Radio_Mngr_App_Notify_DABFM_Linking_BestPI_Diag   : Notifies the Best PI information in ENG Mode. </li>		
		<li> #Radio_Mngr_App_Notify_ClockTime					: Notifies the Clock Time in ENG Mode. </li>
    </ul>
*/

/*-----------------------------------------------------------------------------
    File Inclusions
-----------------------------------------------------------------------------*/

#include "radio_mngr_app_types.h"
#include "radio_mngr_app_inst_hsm.h"
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
/**	\brief 					Notifies the station list information.
*   \param[in]				pst_me_radio_mngr_inst - From this structure, station list of Active band shall get.
*   \param[out]				None.
*   \pre					Station list get read by Radio manager from shared memory.
*   \details 				After tuned to one station in active band, then the active band stationlist shall provide to the HMI IF .
 *							(signal strength, noise etc...)about current tuned station.
*   \post					Active Band station list provided to the HMI IF.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Notify_StationList(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_Band);
/******************************************************************************************************/
/**	\brief 					Notifies the Active band Scan Status.
*   \param[in]				e_ScanStatus - enum which gives the Scan Status.
*   \param[out]				None.
*   \pre					Request received for the Scan.
*   \details 				Active band Scan status like Scan Started/Inprogress/Completed/Failed.
*   \post					Notified the Scan status of active band.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Notify_UpdateScanStatus(Te_Radio_Mngr_App_ScanStatus e_ScanStatus);
/******************************************************************************************************/
/**	\brief 					Notifies the DABtoFM linking status.
*   \param[in]				e_LinkingStatus - enum which gives DABtoFM linking status.
*   \param[out]				None.
*   \pre					DABtoFM linking Feature is in Enable mode.
*   \details 				Notifies the DABtoFM liking function status to the HMI.
*   \post					Notified the DABtoFM linking status to HMI.\n
*   \ErrorHandling    		NA
* 
******************************************************************************************************/
void Radio_Mngr_App_Notify_DABFMLinkingStatus(Te_RADIO_DABFM_LinkingStatus e_LinkingStatus);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Active band current station information in diag/ENG mode
 *  \param[in]              pst_me_radio_mngr_inst-  structure holiding the current active band currently tune station information structure.
 *  \param[out]             None
 *  \pre                    Radio is tuned to any station and received its station information
 *  \details                This API provides the notification of active band currently tuned station information to the 
 *							HMI in diag mode.
 *  \post                   Sent notification of the currently tuned station information.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_UpdateCurStationInfo_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Notify the FM/DAB currently tuned station AF list in Daig/ENG mode
 *  \param[in]              pst_me_radio_mngr_inst-  structure holiding the current FM/DAB station AF list.
 *  \param[out]             None
 *  \pre                    Radio is having the current FM/DAB AF list
 *  \details                This API provides the notification of current FM/DAB tuned station AF list to the HMI in diag mode.
 *  \post                   Sent notification of AF list to HMi.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_AFList_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Tuner status notification.
 *  \param[in]              st_TunerNotify-  active band Currently tuned station Quality, e_activeBand- Active band
 *  \param[out]             None
 *  \pre                    Active band tuned to any station and received its quality info.
 *  \details                This API provides the notification of Active band currently tuned station Quality info to the HMI.
 *  \post                   Sent notification of the Quality to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_Quality_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst, Te_Radio_Mngr_App_Band e_activeBand);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Active band Station list in the Diag/ENG mode
 *  \param[in]              pst_me_radio_mngr_inst-  structure holiding the current active band Stationlist structure.
 *  \param[out]             None
 *  \pre                    Radio is having its active band stationlist
 *  \details                This API provides the notification of active band Station list to the HMI in diag mode.
 *  \post                   Sent notification of the active band station list.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_UpdateSTL_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/


/**
 *  \brief                  Notify the Announcement Tuner status notification.
 *  \param[in]              e_Band- Active band, u32_Freq- Frequency,  StName- Station name, e_Anno_Status- Announcement status, u8_Charset- Charset.
 *  \param[out]             None
 *  \pre                    Active band tuned to announcement station.
 *  \details                This API provides the notification of Active band announcementstation info to the HMI.
 *  \post                   Sent notification of the announcement station to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_Announcement(Te_Radio_Mngr_App_Anno_Status e_Anno_Status);
/******************************************************************************************************/
/**
 *  \brief                  Notify the AF status notification.
 *  \param[in]              e_AFStatus-  AF status.
 *  \param[out]             None
 *  \pre                    Active band tuned to AF station .
 *  \details                This API provides the notification of Af status to the HMI.
 *  \post                   Sent notification of the AF status to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_AFStatus(Te_Radio_Mngr_App_AF_Status e_AFStatus);
/******************************************************************************************************/
/**
 *  \brief                  Notify the component status.
 *  \param[in]              e_ActiveBand-  active band, e_AMFMTunerStatus-AMFM tuner status, e_DABTunerStatus- DAB tuner status
 *  \param[out]             None
 *  \pre                    Radio application is in active state.
 *  \details                This API provides the notification of component status info to the HMI.
 *  \post                   Sent notification of the component status to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_Components_Status(Te_Radio_Mngr_App_Band e_ActiveBand, Te_RADIO_AMFMTuner_Status e_AMFMTunerStatus, 
															Te_RADIO_Comp_Status e_DABTunerStatus,Te_Radio_Mngr_App_DAB_UpNotification_Status	e_DAB_UpNot_Status);
/******************************************************************************************************/
/**
 *  \brief                  Notify the status of radio.
 *  \param[in]              e_ActiveBand-  active band ,e_Activity_State- status of radio
 *  \param[out]             None
 *  \pre                    Active band tuned to any station.
 *  \details                This API provides the notification of Active band radio status info to the HMI.
 *  \post                   Sent notification of the radio status to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_Activity_State(Te_Radio_Mngr_App_Band e_ActiveBand,Te_Radio_Mngr_App_Activity_Status e_Activity_State);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Best PI info in ENG mode.
 *  \param[in]              u32_frequency- Frequency, u32_Quality- Quality
 *  \param[out]             None
 *  \pre                    Active band tuned to any station.
 *  \details                This API provides the notification of Best PI info to the HMI.
 *  \post                   Sent notification of the Best PI to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_DABFM_Linking_BestPI_Diag(Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Clock Time to System
 *  \param[in]              st_AMFMTUNER_CTInfo- This structure holiding the Clock Parameters with Time and Date
 *  \param[out]             None
 *  \pre                    FM is Active Band , Radio Recieved CT Req with ENG Mode ON
 *  \details                This API provides the notification of Clock time from the AMFM TUNER to the System Continously till ENG mode is on.
 *  \post                   Sent notification of the Clock Time.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_ClockTime(Ts_Radio_Mngr_App_CT_Info* st_AMFMTUNER_CTInfo);
/******************************************************************************************************/
/**
 *  \brief                  Notify the switch settings to HMI
 *  \param[in]              e_DABFMLinking_Switch
 *  \param[in]              e_TA_Anno_Switch
 *  \param[in]              e_RDSSettings
 *  \param[in]              e_Info_Anno_Switch
 *  \param[in]              e_MultiplexSettings
 *  \param[out]             None
 *  \pre    				Radio application is in active state.                
 *  \details                This API provides the notification of setting info to the HMI.
 *  \post                   Sent notification of the setting info to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_Settings(Te_Radio_Mngr_App_DABFMLinking_Switch e_DABFMLinking_Switch, Te_Radio_Mngr_App_EnableTAAnno_Switch e_TA_Anno_Switch, Te_Radio_Mngr_App_RDSSettings e_RDSSettings, Te_Radio_Mngr_App_EnableInfoAnno_Switch e_Info_Anno_Switch, Te_Radio_Mngr_App_Multiplex_Switch e_MultiplexSettings);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Cleared data(PSN, RT, DLS) to HMI 
 *  \param[in]              pst_me_radio_mngr_inst- Pointer to instant me structure
 *  \param[out]             None
 *  \pre                    Radio is in Power ON Mode
 *  \details                This API Clears the PSN,RT,DLS buffers based on active band
 *  \post                   Sent notification of cleared data to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_Clear_HMI_Data(Te_Radio_Mngr_App_Band e_Band, Ts_Radio_Mngr_App_Inst_Hsm* pst_me_radio_mngr_inst);
/******************************************************************************************************/
/**
 *  \brief                  Notify the Firmware version to HMI 
 *  \param[in]              pst_me_radio_mngr- Pointer to main HSM structure
 *  \param[out]             None
 *  \pre                    Radio is in Power ON Mode
 *  \details                This API Notify the Firmware version of AMFM Tuner and DAB Tuner to HMI 
 *  \post                   Sent notification of Firmware version to the HMI.\n
 *  \ErrorHandling          NA
 */
/******************************************************************************************************/
void Radio_Mngr_App_Notify_FirmwareVersion(Ts_Radio_Mngr_App_Hsm* pst_me_radio_mngr);

void Radio_Mngr_App_Notify_AudioSwitch(Te_Radio_Mngr_App_Band e_ReqAudioChangeBand);

#endif /* End of RADIO_MNGR_APP_NOTIFY_H */