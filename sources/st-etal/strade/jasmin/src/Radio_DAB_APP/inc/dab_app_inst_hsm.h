/*=============================================================================
    start of file
=============================================================================*/


/****************************************************************************************************************/
/** \file dab_app_inst_hsm.h 																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.															*
*  All rights reserved. Reproduction in whole or part is prohibited												*
*  without the written permission of the copyright owner.														*
*																												*
*  Project              : ST_Radio_Middleware																					*
*  Organization			: Jasmin Infotech Pvt. Ltd.																*
*  Module				: SC_DAB_APP															     			*
*  Description			: The file contains Type Definitions and functions for handling DAB APP Instance HSM.	*
*																												*
*																												*
*****************************************************************************************************************/
#ifndef __DAB_APP_HSM_INST_H__
#define __DAB_APP_HSM_INST_H__

/** \file */
/** \page DAB_APP_INST_HSM_top DAB Application Instance HSM package

\subpage DAB_APP_INST_HSM_Overview
\n
\subpage DAB_APP_INST_HSM_Functions
\n
*/

/**\page DAB_APP_INST_HSM_Overview Overview   
    \n
     DAB Application Instance HSM package consists of function handlers of Instance HSM.   
    \n\n
*/

/** \page DAB_APP_INST_HSM_Functions Instance HSM Function Handlers 
    <ul>
        <li> #DAB_APP_INST_HSM_TopHndlr						: This is the handler function to the top state of instance hsm. </li>
        <li> #DAB_APP_INST_HSM_InactiveHndlr				: This is the handler function to the Inactive state of instance hsm.</li>
		<li> #DAB_APP_INST_HSM_ActiveHndlr					: This is the handler function to the Active state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveBusyHndlr				: This is the handler function to the Active busy state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveStartHndlr				: This is the handler function to the Active start state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveBusyScanHndlr    		: This is the handler function to the Active  Scan state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveIdleHndlr				: This is the handler function to the Active Idle state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveIdleListenedHndlr		: This is the handler function to the Active Idle state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveStopHndlr				: This is the handler function to the active stop state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr	: This is the handler function to the Active Busy Play Select Station state of instance hsm. </li>
		<li> #DAB_APP_INST_HSM_ActiveSleepHndlr				: This is the handler function to the Active Sleep state of instance hsm. </li>
    </ul>
*/



/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/

#include "dab_app_types.h"
#include "cfg_types.h"
#include "cfg_variant_market.h"



/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/
/**
 * @brief Structure definition of instance hsm object
 *
 * @details Holds the current message, next message, current state handler name, cid and other varaiables for instance HSM
 */
typedef struct 
{
	Ts_hsm												hsm;					/* the base HSM object, have to be the first member of this struct (handles state transitions) */
	Tu8 												str_state[100]; 
	const Tu8											*ptr_curr_hdlr_name;	/* Pointer to store the current state handler name */
	
	Ts_DAB_APP_Curr_Anno_Info							st_CurrAnnoInfo;
	Ts_Tuner_Ctrl_CurrentEnsembleInfo 					st_CurrentTunedAnnoInfo;
	Ts_DAB_App_AFList									st_DAB_App_AFList;
	Ts_DAB_APP_CurStationInfo							st_SeekInfo;
	Ts_DAB_App_CurrentStationInfo						s_DAB_App_CurrentStationInfo;
	Ts_DAB_APP_GetVersion_Reply 						st_DAB_APP_Vesrion_Reply;
	Ts_DAB_App_CurrentStationInfo						s_DAB_App_Lsmstationinfo;
	Ts_DAB_PICodeList 									st_DAB_PICodeList;
	Ts_DAB_DLS_Data										st_DLSData;
	Ts_DAB_APP_fmdab_linkinfo							st_FM_DAB_linking_station;
	Ts_DAB_App_Tunable_StationInfo						st_tunableinfo;         /* variable for structure comprising information used for station tuning purpose */
	Ts_DAB_APP_Curr_Anno_Info							st_Tuned_Anno_Info;
	Ts_DAB_App_CurrEnsembleProgList						st_DAB_App_CurrEnsembleProgList;
	Ts_DAB_APP_Status_Notification						st_TunerStatusNotify;
	

	Te_DAB_Tuner_Ctrl_announcement_type					e_announcement_type;
	Te_DAB_App_BestPI_Type								e_BestPI_Type;
	Te_DAB_App_CancelType 								e_CancelType;
	Te_RADIO_Comp_Status    							e_ComponentStatus;
	Te_DAB_App_ActivateDeactivateStatus					e_DAB_App_ActivateDeactivateStatus;
	Te_DAB_App_AnnoIndication							e_DAB_App_AnnoIndication;
	Te_DAB_APP_DAB_DAB_Status 							e_DAB_APP_DAB_DAB_Status;
	Te_DAB_APP_Eng_Mode_Request							e_DAB_APP_Eng_Mode_Request;
	Te_DAB_App_LearnMemAFStatus 						e_DAB_App_LearnMemAFStatus;
	Te_DAB_App_RequestCmd  								e_DAB_App_RequestCmd ;
	Te_DAB_App_AF_Switch 								e_DAB_App_RDS_Settings;
	Te_DAB_App_Satrtup_Request_Type						e_DAB_App_Satrtup_Request_Type;	
	Te_DAB_App_SignalStatus 							e_DAB_App_SignalStatus;
	Te_DAB_App_StationNotAvailStrategyStatus 			e_DAB_App_StationNotAvailStrategyStatus;
	Te_RADIO_DABFM_LinkingStatus						e_LinkingStatus;
	Te_Dab_App_FmtoDAB_Reqstatus 						e_FmtoDAB_Reqstatus;
	Te_RADIO_Comp_Status    							e_AMFMTUNERStatus ;	
	Te_DAB_App_DABFMLinking_Switch						e_Linking_Switch_Status;	
	Te_DAB_App_Market									e_Market;
	Te_DAB_App_ReConfigType								e_ReConfigType;	
	Te_RADIO_ReplyStatus						        e_Replystatus;		
	Te_RADIO_ReplyStatus								e_LinkingSettings;	
	Te_RADIO_ReplyStatus								e_DLS_EnableReplyStatus;
	Te_DAB_APP_fmdab_linkstatus							e_FM_DAB_linking_status;			
	Te_RADIO_DirectionType								e_SeekDirection;
	Te_RADIO_SharedMemoryType							e_SharedMemoryType;
	Te_DAB_App_TuneRequest								e_TuneRequest;
	Te_RADIO_DirectionType								e_TuneUpDownDirection;
	
	Tbool                                               b_SeekStarted;

	Tu32												u32_Frequency_Change;
	Tu32												u32_SeekFrequency;
	Tu16 												PICode;
	//Tu16												u16_FM_DAB_PI;
	Tu16												u16_AnnoConfig;
	Tu16 												u16_FM_DAB_SCID;
	Tu16 												u16_FM_DAB_SID;
	
	Tu8													au8_ChannelName[DAB_APP_MAX_CHANNEL_NAME_SIZE];
	Tu8													u8_Curr_Frequency_Index;
	Tu8													u8_Curr_Station_Index;
	Tu8													u8_DAB_APP_announcement_status;	
	Tu8													u8_DAB_App_InitialStartUpFlag;
	Tu8 												u8_DAB_SignalStatus;	
	
	Tu8													u8_FM_DAB_Station_Quality;
	Tu8 												u8_FS;
	Tu8													u8_Quality;	
	Tu8 												u8_QualityMax;			
	Tu8 												u8_QualityMin;
	Tu8													u8_SameChannelClusterid;
	Tu8         										u8_SameChannelSubChId;             
	Tu8													u8_SettingStatus;
	Tu8													u8_StartType;
	Tu8                                                 u8_CurrentServiceIndex_Seek;
	Tu8                                                 u8_CurrentServiceCompIndex_Seek;	 
	Tu8         										u8_SubChId;
	Tu8  												u8_Handler_Check;

	
}Ts_dab_app_inst_hsm;

/*-----------------------------------------------------------------------------
					function declarations 
-----------------------------------------------------------------------------*/
void SearchSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst);
void UpdateSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst);
void Dab_App_Sort_CurrEnsembleProgList(Ts_DabTunerMsg_GetCurrEnsembleProgListReply *CurrEnsembleProgList);
void Search_Next_Frequency(Ts_dab_app_inst_hsm *pst_me_dab_app_inst ,Te_RADIO_DirectionType e_Direction);
void SearchServiceSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst);
void SearchServiceCompSeekInfo(Ts_dab_app_inst_hsm *pst_me_dab_app_inst);

/*****************************************************************************************************/
/**	 \brief 				This function Initializes instance hsm 'dab_app_inst_hsm'.
*   \param[in]				pst_me Pointer to the hsm object.
*   \param[out]				none
*   \pre					DAB Application Instance HSM is not initialized.
*   \details 				This function initializes the dab_app_inst_hsm and transits to Inactive state.  
*   \post					DAB application Instance HSM is in Inactive state\n
*   \ErrorHandling    		none.
* 
******************************************************************************************************/

void DAB_APP_INST_Init(Ts_dab_app_inst_hsm *pst_me);


/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the top state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the top state handler of the dab_app_inst_hsm. In this \n 
*							handler it is only allowed to take the transition to the inactive state. 
*							No other transitions are allowed. 
*   \post					if a message is not handled in the top state a system error occurs.
*   \ErrorHandling    		A unhandled message in top state causes a system error.
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_TopHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Inactive state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the inactive state handler of the dab_app_inst_hsm In this \n 
*							handler it is only allowed to take the transition with a startup message from the main hsm 'dab_app_hsm'\n 
*							No other transitions are allowed. 
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_InactiveHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active state handler of the dab_app_inst_hsm  \n 
*							This is the child state of top state. Under this state only\n 
*							all the operations related to dab application will be handled.
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveHndlr(Ts_dab_app_inst_hsm* pst_me, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active busy state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active busy state handler of the dab_app_inst_hsm. 
*   \post					if a message is not handled in the top state a system error occurs.
*   \ErrorHandling    		A unhandled message in top state causes a system error.
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveBusyHndlr(Ts_dab_app_inst_hsm* pst_me, Ts_Sys_Msg* pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active start state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active start handler of the dab_app_inst_hsm.\n 
*							This is the child of active state,it transits to active idle state\n
*							on receiving message "DAB_APP_SELECT_DAB_REQID"
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveStartHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Scan state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active Background Scan handler of the dab_app_inst_hsm.\n 
*							This is the child of active state. In this state after receiving "DAB_APP_STL_UPDATE_NOTIFYID"\n
*							message from Tuner Ctrl, DAB app reads the shared memory between DAB APP and TUNER CTRL,\n
*							sorts and writes the station list in shared memory between RADIO MNGR and DAB APP.
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveBusyScanHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Idle state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active idle state handler of the dab_app_inst_hsm, hsm waits for commands in this state,\n
*                           after the command is processed it will come back to active idle state.
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveIdleHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Idle Listened state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active idle state handler of the dab_app_inst_hsm, hsm waits for commands in this state,
 *                          after the command is processed it will come back to active idle state.
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveIdleListenedHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the active stop state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active stop handler of the dab_app_inst_hsm.This is the child of active state.\n
*							After receiving DAB_APP_INST_HSM_SHUTDOWN request main hsm transition takes to this state and\n
*							then to inactive state in instance hsm.
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveStopHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Busy Play Select Station state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the child of active busy state,all the tune related commands\n
*							are handled in this state. In this handler DAB Application triggers Play Select Station\n
*							request to Tuner Ctrl by passing frequency, Eid, sid and scid information. 
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveBusyPlaySelStnHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Busy Service Component Seek state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the child of active busy state, seek related commands\n
*							are handled in this state. In this handler DAB Application triggers Service Component Seek\n
*							request with direction as a parameter to Tuner Ctrl . 
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

Ts_Sys_Msg* DAB_APP_INST_HSM_ActiveBusysSerCompSeekHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg) ;

/*****************************************************************************************************/
/**	 \brief 				This is the handler function to the Active Sleep state of instance hsm.
*   \param[in]				pst_me Pointer to the hsm object
*   \param[out]				pst_msg Pointer to the message. A result code has to be set if message is handled in this state\n
*   \pre					HSM is initialized and msg is not NULL.
*   \details 				This is the active sleep handler of the dab_app_inst_hsm.This is the child of active state.\n
*							This state handles the selection and de-selection of DAB band.
*   \post					if a message is not handled handle the message in parent state.
*   \ErrorHandling    		none
* 
******************************************************************************************************/

//Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveSleepHndlr(Ts_dab_app_inst_hsm *pst_me, Ts_Sys_Msg *pst_msg);

Ts_Sys_Msg*	DAB_APP_INST_HSM_Active_BGScanHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg);

Ts_Sys_Msg*	DAB_APP_INST_HSM_ActiveIdleAnnouncementHndlr(Ts_dab_app_inst_hsm *pst_me_dab_app_inst, Ts_Sys_Msg *pst_msg);

#endif /* __DAB_APP_HSM_INST_H__ */

/*=============================================================================
    end of file
=============================================================================*/