//!
//!  \file      service_following_mainloopl.h
//!  \brief     <i><b> This header file contains internal functions and variable for service following main loop  </b></i>
//!  \details   This header contains declarations related to service following feature
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_MAINLOOP_H_
#define SERVICE_FOLLOWING_MAINLOOP_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************
* DEFINE SECTION
**********************
*/


/*
*********************
* STRUCTURE SECTION
**********************
*/
	

/*
*********************
* VARIABLE SECTION
**********************
*/
#ifndef SERVICE_FOLLOWING_MAINLOOP_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

/* CONST
*/  
#ifdef SERVICE_FOLLOWING_MAINLOOP_C
GLOBAL tChar DABMW_SF_STATE_IDLE_NAME[] = "DABMW_SF_STATE_IDLE";
GLOBAL tChar DABMW_SF_STATE_INIT_NAME[] = "DABMW_SF_STATE_INIT";
GLOBAL tChar DABMW_SF_STATE_INITIAL_SERVICE_SELECTION_NAME[] = "DABMW_SF_STATE_INITIAL_SERVICE_SELECTION";
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_CHECK_NAME[] = "DABMW_SF_STATE_BACKGROUND_CHECK";
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK_NAME[] = "DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK";
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_PI_NAME[] = "DABMW_SF_STATE_BACKGROUND_WAIT_PI";
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_DAB_NAME[] = "DABMW_SF_STATE_BACKGROUND_WAIT_DAB";
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_FIC_NAME[] = "DABMW_SF_STATE_BACKGROUND_WAIT_FIC";
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_SCAN_NAME[] = "DABMW_SF_STATE_BACKGROUND_SCAN";
GLOBAL tChar DABMW_SF_STATE_AF_CHECK_NAME[] = "DABMW_SF_STATE_AF_CHECK";
GLOBAL tChar DABMW_SF_STATE_SERVICE_RECOVERY_NAME[] = "DABMW_SF_STATE_SERVICE_RECOVERY";
GLOBAL tChar DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF_NAME[] = "DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF";
GLOBAL tChar DABMW_SF_STATE_IDLE_AF_MONITORING_NAME[] = "DABMW_SF_STATE_IDLE_AF_MONITORING";
GLOBAL tChar DABMW_SF_STATE_IDLE_SEAMLESS_NAME[] = "DABMW_SF_STATE_IDLE_SEAMLESS";
GLOBAL tChar DABMW_SF_STATE_DEFAULT_NAME[] = "DABMW_SF_STATE_UNKOWN";

// NEW : PS NAME for landscape generation
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_PS_NAME[] = "DABMW_SF_STATE_BACKGROUND_WAIT_PS";


// for the band
GLOBAL tChar DABMW_BAND_NONE_NAME[] = "DABMW_BAND_NONE";
GLOBAL tChar DABMW_BAND_FM_EU_NAME[] = "DABMW_BAND_FM_EU";
GLOBAL tChar DABMW_BAND_FM_US_NAME[] = "DABMW_BAND_FM_US";
GLOBAL tChar DABMW_BAND_FM_JAPAN_NAME[] = "DABMW_BAND_FM_JAPAN";
GLOBAL tChar DABMW_BAND_FM_EAST_EU_NAME[] = "DABMW_BAND_FM_EAST_EU";
GLOBAL tChar DABMW_BAND_FM_WEATHER_US_NAME[] = "DABMW_BAND_FM_WEATHER_US";
GLOBAL tChar DABMW_BAND_AM_MW_EU_NAME[] = "DABMW_BAND_AM_MW_EU";
GLOBAL tChar DABMW_BAND_AM_MW_US_NAME[] = "DABMW_BAND_AM_MW_US";
GLOBAL tChar DABMW_BAND_AM_MW_JAPAN_NAME[] = "DABMW_BAND_AM_MW_JAPAN";
GLOBAL tChar DABMW_BAND_AM_MW_EAST_EU_NAME[] = "DABMW_BAND_AM_MW_EAST_EU";
GLOBAL tChar DABMW_BAND_DAB_III_NAME[] = "DABMW_BAND_DAB_III";
GLOBAL tChar DABMW_BAND_CHINA_DAB_III_NAME[] = "DABMW_BAND_CHINA_DAB_III";
GLOBAL tChar DABMW_BAND_KOREA_DAB_III_NAME[] = "DABMW_BAND_KOREA_DAB_III";
GLOBAL tChar DABMW_BAND_DAB_L_NAME[] = "DABMW_BAND_DAB_L";
GLOBAL tChar DABMW_BAND_CANADA_DAB_L_NAME[] = "DABMW_BAND_CANADA_DAB_L";
GLOBAL tChar DABMW_BAND_AM_LW_NAME[] = "DABMW_BAND_AM_LW";
GLOBAL tChar DABMW_BAND_AM_SW1_NAME[] = "DABMW_BAND_AM_SW1";
GLOBAL tChar DABMW_BAND_AM_SW2_NAME[] = "DABMW_BAND_AM_SW2";
GLOBAL tChar DABMW_BAND_AM_SW3_NAME[] = "DABMW_BAND_AM_SW3";
GLOBAL tChar DABMW_BAND_AM_SW4_NAME[] = "DABMW_BAND_AM_SW4";
GLOBAL tChar DABMW_BAND_AM_SW5_NAME[] = "DABMW_BAND_AM_SW5";
GLOBAL tChar DABMW_BAND_AM_SW6_NAME[] = "DABMW_BAND_AM_SW6";
GLOBAL tChar DABMW_BAND_AM_SW7_NAME[] = "DABMW_BAND_AM_SW7";
GLOBAL tChar DABMW_BAND_AM_SW8_NAME[] = "DABMW_BAND_AM_SW8";
GLOBAL tChar DABMW_BAND_AM_SW9_NAME[] = "DABMW_BAND_AM_SW9";
GLOBAL tChar DABMW_BAND_AM_SW10_NAME[] = "DABMW_BAND_AM_SW10";
GLOBAL tChar DABMW_BAND_DRM30_NAME[] = "DABMW_BAND_DRM30";

#else
GLOBAL tChar DABMW_SF_STATE_IDLE_NAME[];
GLOBAL tChar DABMW_SF_STATE_INIT_NAME[];
GLOBAL tChar DABMW_SF_STATE_INITIAL_SERVICE_SELECTION_NAME[];
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_CHECK_NAME[];
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK_NAME[];
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_PI_NAME[];
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_DAB_NAME[];
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_FIC_NAME[];
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_SCAN_NAME[];
GLOBAL tChar DABMW_SF_STATE_AF_CHECK_NAME[];
GLOBAL tChar DABMW_SF_STATE_SERVICE_RECOVERY_NAME[];
GLOBAL tChar DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF_NAME[];
GLOBAL tChar DABMW_SF_STATE_IDLE_AF_MONITORING_NAME[];
GLOBAL tChar DABMW_SF_STATE_IDLE_SEAMLESS_NAME[];
GLOBAL tChar DABMW_SF_STATE_DEFAULT_NAME[];

// NEW : PS NAME for landscape generation
GLOBAL tChar DABMW_SF_STATE_BACKGROUND_WAIT_PS_NAME[];


GLOBAL tChar DABMW_BAND_NONE_NAME[];
GLOBAL tChar DABMW_BAND_FM_EU_NAME[];
GLOBAL tChar DABMW_BAND_FM_US_NAME[];
GLOBAL tChar DABMW_BAND_FM_JAPAN_NAME[];
GLOBAL tChar DABMW_BAND_FM_EAST_EU_NAME[];
GLOBAL tChar DABMW_BAND_FM_WEATHER_US_NAME[];
GLOBAL tChar DABMW_BAND_AM_MW_EU_NAME[];
GLOBAL tChar DABMW_BAND_AM_MW_US_NAME[];
GLOBAL tChar DABMW_BAND_AM_MW_JAPAN_NAME[];
GLOBAL tChar DABMW_BAND_AM_MW_EAST_EU_NAME[];
GLOBAL tChar DABMW_BAND_DAB_III_NAME[];
GLOBAL tChar DABMW_BAND_CHINA_DAB_III_NAME[];
GLOBAL tChar DABMW_BAND_KOREA_DAB_III_NAME[];
GLOBAL tChar DABMW_BAND_DAB_L_NAME[];
GLOBAL tChar DABMW_BAND_CANADA_DAB_L_NAME[];
GLOBAL tChar DABMW_BAND_AM_LW_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW1_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW2_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW3_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW4_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW5_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW6_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW7_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW8_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW9_NAME[];
GLOBAL tChar DABMW_BAND_AM_SW10_NAME[];
GLOBAL tChar DABMW_BAND_DRM30_NAME[];

#endif


/* variables are belonging to SERVICE_FOLLOWING_MAINLOOP_C
*/


/*
*********************
* FUNCTIONS SECTION
**********************
*/


/* here it is defined in service_following_internal.h */

GLOBAL tSInt DABMW_ServiceFollowing_MainLoop(tVoid);

/* Procedure to check if a time has elapsed for a given event
*/
GLOBAL tBool DABMW_ServiceFollowing_CheckTimeAction (DABMW_SF_TimeCheckTy eventToCheck);

/* Handling of specific event for service following
* Current EVENT 
* DABMW_SF_EVENT_PI_RECEIVED : PI reception notification
* DABMW_SF_EVENT_DAB_TUNE : TUNE notification
*/

GLOBAL tSInt DABMW_ServiceFollowing_EventHandling(tU8 vI_event);

// Procedure which enter on a new cell 
//based on the current cell
// It does not handle the alternate : it is assumed there is none
//
GLOBAL tSInt DABMW_ServiceFollowing_SetCurrentCellAsOriginal(tVoid);

/* procedure to init the basic parameter for idle
* this is when entering in idle : 
* starting being tuned on a freq
*/
GLOBAL tVoid DABMW_ServiceFollowing_EnterIdleMode(DABMW_mwAppTy vI_App, DABMW_SF_systemBandsTy vI_SystemBand, tU32 vI_Sid, tU32 vI_Eid, tU32 vI_Frequency);

/* procedure to init the basic parameter of monitored AF
*/
GLOBAL tVoid DABMW_ServiceFollowing_SetMonitoredAF(DABMW_mwAppTy vI_App, DABMW_SF_systemBandsTy vI_SystemBand, tU32 vI_Sid, tU32 vI_Eid, tU32 vI_Frequency, DABMW_SF_QualityTy vI_quality, tBool dabServiceIsSelected);


/* procedure to reset the monitored AF information
*/
GLOBAL tVoid DABMW_ServiceFollowing_ResetMonitoredAF(tVoid);

GLOBAL tSInt DABMW_ServiceFollowing_SuspendMonitoredAF(tVoid);

/* procedure to reset the monitored AF information
*/
GLOBAL tSInt DABMW_ServiceFollowing_ResumeMonitoredAF(tVoid);


/* procedure to sent the auto notification on the API to the Host
*/
GLOBAL tVoid DABMW_ServiceFollowing_SendChangeNotificationsToHost (tVoid);

/* Procedure to put back a tuner to Idle 
*/
GLOBAL tSInt DABMW_ServiceFollowing_ResetTuner(DABMW_mwAppTy vI_App);

/* Procedure which cares of the processing following the Estimation Response Reception
*/
GLOBAL tSInt DABMW_ServiceFollowing_SeamlessEstimationResponseProcessing(tBool vI_ResponseIsReceived);

/* Procedure which cares of the processing following the Switch Response Reception
*/
GLOBAL tSInt DABMW_ServiceFollowing_SeamlessSwitchResponseProcessing(tBool vI_ResponseIsReceived);


/* procedure to Get the band name
*/
GLOBAL tPChar DABMW_ServiceFollowing_GetBandName (tU32 band);

/* Mute/unmute Function
*/
GLOBAL tSInt DABMW_ServiceFollowing_FM_MuteUnmute(DABMW_mwAppTy vI_app, tBool vI_muteRequested);

#undef GLOBAL


#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_MAINLOOP_H_

// End of file

