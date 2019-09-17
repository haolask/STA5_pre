/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_request.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of all  Request APIs which will be		*
*						  provided to Radio Manager Application 											*
*																											*
*************************************************************************************************************/
#ifndef AMFM_APP_REQUEST_H
#define AMFM_APP_REQUEST_H

/** \file */
/** \page AMFM_APP_REQUEST_top AMFM Application Request 

\subpage AMFM_APP_REQUEST_Overview
\n
\subpage AMFM_APP_REQUEST_API_Functions
\n
*/

/** \page AMFM_APP_REQUEST_Overview Overview   
    \n
     AMFM Application Request consists of Request API's which are called by Radio Manager Application.   
    \n\n
*/

/** \page AMFM_APP_REQUEST_API_Functions API Functions 
    <ul>
        <li> #AMFM_App_Request_Startup         	: Request for start of AMFM Application component. </li>
        <li> #AMFM_APP_Request_Shutdown       	: Request for shut-down of AMFM Application component. </li>
		<li> #AMFM_App_Request_GetStationList   : Request for generating AM/FM station list. </li>
		<li> #AMFM_App_Request_SelectBand       : Request for selecting AM/FM band. </li>
		<li> #AMFM_App_Request_DeSelectBand     : Request for Deselecting AM/FM band. </li>
		<li> #AMFM_App_Request_PlaySelectSt     : Request for tuning to station in station list. </li>
		<li> #AMFM_App_Request_SeekUpDown		: Request for performing seek operation  either in AM/FM band. </li>
		<li> #AMFM_App_Request_Cancel			: Request for cancelling on going AM/FM seek operation. </li>
		<li> #AMFM_App_Request_FindBestPI		: Request for finding Best PI for DAB-FM service followinf purpose. </li>
		<li> #AMFM_App_Request_BlendingStatus	: Request used to indicate DAB-FM linking status. </li>
    </ul>
*/
/*-----------------------------------------------------------------------------
      File Inclusions
-----------------------------------------------------------------------------*/

#include "amfm_app_msg.h"
#include "amfm_app_msg_id.h"
#include "cfg_types.h"
#include "cfg_variant_market.h"

/*-----------------------------------------------------------------------------
    Public Function Declarations
-----------------------------------------------------------------------------*/

/*****************************************************************************************************/
/**	 \brief 				            Request for starting AMFM Application component.
*
*   \param[in]				
*				e_Market   				enum holds present market type #Te_AMFM_App_Market.
*				u8_switch_setting		variable holds the initial startup setting values.
*				u8_start_up_type		variable holds the initial startup type.
*   \param[out]			
*				st_amfm_app_req_msg     structure holds startup request message  
*
*   \pre-condition			            AMFM Application component is not started
*
*   \details 				            This API initialises AMFM APP software component. 
* 							            In turns it triggers start up request to Tuner Ctrl. 
*   \post-condition			            AMFM Application component is started \n
*
*   \error		    		            NA
* 
******************************************************************************************************/
void AMFM_App_Request_Startup(Te_AMFM_App_Market e_market,Tu8 u8_switch_setting,Tu8 u8_start_up_type);


/*****************************************************************************************************/
/**	 \brief 				               Request for shut-down of AMFM Application component.
*
*   \param[in]				
*				void   		
*   \param[out]			
*				st_amfm_app_req_msg		    structure holds shutdown request message  
*
*   \pre-condition			                AMFM Application component is started
*
*   \details 				                This API de-initialises AMFM APP software component. 
* 							                In turns it triggers shut down request to Tuner Ctrl. 
*
*   \post-condition			                AMFM Application component is stopped  \n
*
*   \error		    		                NA
* 
******************************************************************************************************/
void AMFM_APP_Request_Shutdown(void);

/*****************************************************************************************************/
/**	 \brief 				               Request for generating AM/FM station list
*
*   \param[in]				
*				e_Mode                     enum to indicate the mode type #Te_AMFM_App_mode  		
*   \param[out]			
*				st_amfm_app_req_msg		   structure holds GetStation list request message  
*
*   \pre-condition			               AMFM Application component is started
*
*   \details 				               This Request is for performing the station list update by scanning through entire specified band.
*                                          In turns it wil invoke scan request to Tuner Ctrl. 
*
*   \post-condition			               Station list is generated if no error .
*
*   \error		    		               NA
* 
******************************************************************************************************/
void AMFM_App_Request_GetStationList(Te_AMFM_App_mode e_Mode);


/*****************************************************************************************************/
/**	 \brief 				               Request for selecting AM/FM band.
*
*   \param[in]				
*				e_Mode                     enum to indicate the mode   		
*   \param[out]			
*				st_amfm_app_req_msg		   structure holds select band request message  
*
*   \pre-condition			                AMFM Application component is inactive
*
*   \details 				                This API for activating AMFM Application componenet.This API is called band change is between DAB to AM,DAB to FM.
*                                           But if Band change is between AM to FM and vice versa ,then it is not called by Radio Manager Application.     
*                                           In turns it triggers select request to Tuner Ctrl.    
* 							               
*   \post-condition			                AMFM Application component is active  \n
*
*   \error		    		                NA
* 
******************************************************************************************************/
void AMFM_App_Request_SelectBand(Te_AMFM_App_mode e_Mode);


/*****************************************************************************************************/
/**	 \brief 				               Request for Deselecting AM/FM band
*
*   \param[in]				
*				e_Mode                     enum to indicate the mode of type #Te_AMFM_App_mode  		
*   \param[out]			
*				st_amfm_app_req_msg		   structure holds De-select band request message  
*
*   \pre-condition			                AMFM Application component is active
*
*   \details 				                This API for de-activating AMFM Application componenet. This API is called band change is between AM to DAB , FM to DAB.
*                                           But if Band change is between AM to FM and vice versa ,then it is not called by Radio Manager Application.     
*                                           In turns it triggers de select request to Tuner Ctrl.    
* 							               
*   \post-condition			                AMFM Application component is inactive  \n
*
*   \error		    		                NA
* 
******************************************************************************************************/
void AMFM_App_Request_DeSelectBand(Te_AMFM_App_mode e_Mode);


/*****************************************************************************************************/
/**	 \brief 				               Request for tuning to station in station list
*
*   \param[in]	
*               u32_freq				   Frequency of the station to be tuned  of type #Tu32   
*	\param[in]	
*               e_Mode                     enum to indicate the mode of type #Te_AMFM_App_mode  		
*   \param[out]			
*				st_amfm_app_req_msg		   structure holds Play select station request message  
*
*   \pre-condition			                AMFM Application component is active
*
*   \details 				                This API is called for tuning to an AM or FM station.The only way to tune a station is selecting them from the stationlist.
*                                           In turns it triggers tune request to Tuner Ctrl.    
* 							               
*   \post-condition			                Desired Station is selected  \n
*
*   \error		    		                NA
* 
******************************************************************************************************/
void AMFM_App_Request_PlaySelectSt(Tu32 u32_freq, Te_AMFM_App_mode e_mode);

/*****************************************************************************************************/
/**	 \brief 				            Request for performing seek operation  either in AM/FM band 
*
*   \param[in]				
*               u32_startfreq           variable holds frequency at which seek should start
*				e_Direction   			enum holds direction for seek operation of type #Te_CMN_Direction.
*   \param[out]			
*				st_amfm_app_req_msg     structure holds Seek Up/Down request message  
*
*   \pre-condition			            Tuned to an AM/FM station 
*
*   \details 				            This API seeks for the next/previous available AM/FM stations if direction value is 
*										RADIO_FRMWK_DIRECTION_UP/RADIO_FRMWK_DIRECTION_DOWN respectively. 
*
*   \post-condition			            Tuned to next/previous station if any station is available
*
*   \error		    		            NA
* 
******************************************************************************************************/
void AMFM_App_Request_SeekUpDown(Tu32 u32_startfreq,Te_RADIO_DirectionType  e_Direction);

/*****************************************************************************************************/
/**	 \brief 				            Request for cancelling on going AM/FM seek operation.  
*
*   \param[in]				
*               void
*   \param[out]			
*				st_amfm_app_req_msg     Structure holds Seek cancel request message  
*
*   \pre-condition			            Seek operation is under progress
*
*   \details 							This API is used for cancelling on going AM/FM seek operation whenever 
*										Band is changed.   
*
*   \post-condition			            Requested Band is selected and plays last tuned station.
*
*   \error		    		            NA
* 
******************************************************************************************************/
void AMFM_App_Request_Cancel(void);
/*****************************************************************************************************/
/**	 \brief 				               Request for finding Best PI for DAB-FM service followinf purpose
*
*   \param[in]	
*				st_FM_PIList			   Structure holds list of PI codes sent by DAB Tuner Ctrl 
*   \param[in]	
*               u32_QualityMin			   Variable holds thershold for Minimum Quality 	   
*   \param[in]	
*				u32_QualityMax			   Variable holds thershold for Maximum Quality	 		
*   \param[out]			
*				st_amfm_app_req_msg		   Structure holds Find Best PI request message  
*
*   \pre-condition			               DAB service component is selected and should have service following 
*
*   \details 				               This API provides list of PI codes for DAB-FM service following.Best PI
*										   has to be found among that list for linking whenever DAB signal goes down.
* 							               
*   \post-condition			               Best PI will be notified to DAB tuner control via Radio Manager if PI is available\n
*
*   \error		    		                NA
* 
******************************************************************************************************/
void AMFM_App_Request_FindBestPI(Ts_AMFM_App_PIList st_FM_PIList,Tu32 u32_QualityMin,Tu32 u32_QualityMax,Tu32 u32_Implicit_sid,Tu8 u8_LinkType);
/*****************************************************************************************************/
/**	 \brief 				               Request used to indicate DAB-FM linking status 
*
*   \param[in]	
*				e_LinkingStatus			   Enum holds list of PI codes sent by DAB Tuner Ctrl of type #Te_RADIO_DABFM_LinkingStatus
*   \param[out]			
*				st_amfm_app_req_msg		   Structure holds Blending Status request message  
*
*   \pre-condition			               DAB service component is selected
*
*   \details 				               This API provides list of PI codes for DAB-FM service following.Best PI
*										   has to be found among that list for linking whenever DAB signal goes down.
* 							               
*   \post-condition			               Monitoring state will be changed as per status value\n
*
*   \error		    		                NA
* 
******************************************************************************************************/
void AMFM_App_Request_BlendingStatus(Te_RADIO_DABFM_LinkingStatus	e_LinkingStatus);

void AMFM_App_Request_TuneUpDown(Te_RADIO_DirectionType  e_Direction,Tu32	u32_No_of_Steps);
void AMFM_App_Request_AFSwitch(Te_AMFM_App_AF_Switch  e_AF_Switch);
void AMFM_APP_Request_SetAFRegionalSwitch(Te_AMFM_AF_REGIONAL_Switch	e_AF_REGIONAL_Switch);
void AMFM_APP_Request_TASwitch(Te_AMFM_App_TA_Switch  e_TA_Switch);
void AMFM_APP_Request_FM_to_DAB_Switch(Te_AMFM_App_FM_To_DAB_Switch  e_FM_to_DAB_Switch);
void AMFM_App_Request_AFTune(Tu16 u16_PI);
void AMFM_App_Request_AnnoCancel(Te_AMFM_App_Anno_Cancel_Request e_Anno_Cancel_Request);
void AMFM_App_Request_ENG_Mode(Te_AMFM_App_Eng_Mode_Switch	e_ENG_ModeSwitch);
void AMFM_App_Request_GetCT_Info(void);
void AMFM_App_Request_FactoryReset(void);

/*****************************************************************************************************/
/**	 \brief 				            Request for sending message to main HSM from inst HSM
*
*   \param[in]	
*               pst_Msg        	        Pointer to the message of type #Ts_Sys_Msg
*	\param[in]	
*               u16_msgid	            Local ID (Message ID ) of the message.Message can be either AMFM_APP_INST_HSM_START_DONE or 
*							            AMFM_APP_INST_HSM_SHUTDOWN_DONE
*   \param[out]			
*				st_amfm_app_req_msg		Structure holds message to be sent to main HSM 
*
*   \pre-condition			            Both HSMs are initialized 
*
*   \details 				            This API is called to send AMFM_APP_INST_HSM_START_DONE or AMFM_APP_INST_HSM_SHUTDOWN_DONE message 
*							            to main HSM from inst HSM during startup or shutdown process respectively.
* 							               
*   \post-condition			            Main hsm either Active idle/inactive state is reached while sending AMFM_APP_INST_HSM_START_DONE/ 
*							            AMFM_APP_INST_HSM_SHUTDOWN_DONE message respectively.
*
*   \error		    		            NA
* 
******************************************************************************************************/
void AMFM_APP_SendMsgtoMainhsm(Ts_Sys_Msg *pst_Msg,Tu16 u16_msgid);
    

#endif /* End of AMFM_APP_REQUEST_H */