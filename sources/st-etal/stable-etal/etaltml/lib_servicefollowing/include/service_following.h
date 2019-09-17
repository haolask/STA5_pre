//!
//!  \file      service_following.h
//!  \brief     <i><b> This header file export functions for service following functionality </b></i>
//!  \details   This header contains declarations related to service following feature
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2012.07.24
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_H_
#define SERVICE_FOLLOWING_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Some good define to share 
*/
    /* Quality information */
    /* FM GOOD Threshold in DBuV for getting RDS.... */
#define DABMW_SF_FM_GOOD_QUALITY_VALUE_THRESHOLD	20	
#define DABMW_SF_DAB_FIC_BER_GOOD_QUALITY_VALUE_THRESHOLD	500

// Defines for SF setting : simulate lock & mobility

#define DABMW_SF_SIMULATE_FREQUENCY_ALL_DAB 0x00000001
#define DABMW_SF_SIMULATE_FREQUENCY_ALL_FM  0x00000002

/*
*********************
* ENUM  SECTION
**********************
*/
// Debug MSG to simulate/control mobility
// quality Level
typedef enum {
        DABMW_SF_SIMULATE_QUALITY_AUTO      = 0,
        DABMW_SF_SIMULATE_QUALITY_POOR      = 1,
        DABMW_SF_SIMULATE_QUALITY_MEDIUM    = 2,
        DABMW_SF_SIMULATE_QUALITY_GOOD      = 3
} DABMW_SF_QualitySimulateTy;


/* procedures */

extern tSInt DABMW_ServiceFollowingInit (tVoid);

extern tSInt DABMW_ServiceFollowingOnAudioPortUserChange (DABMW_mwAppTy application, tBool isInternalTune);

extern tSInt DABMW_ServiceFollowing (tVoid);

/* EPR TMP CHANGE */
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
/* add procedure for emulating the DABMW Level */
extern tSInt DABMW_ServiceFollowingSimulate_DAB_FM_LevelChange (tU32 vI_frequency1, tU8 vI_F1_levelChangeType, tU32 vI_frequency2, tU8 vI_F2_levelChangeType);

#endif //CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
/* END EPR CHANGE */


/* EPR change : add procedure
*  for Service Following initial selection  
*/


#if defined(DABMW_TASK_C) || defined (SERVICE_FOLLOWING_C) || defined(SERVICE_FOLLOWING_MAINLOOP_C)
/* Procedure for Service following event handling
*/
extern tSInt DABMW_ServiceFollowing_EventHandling(tU8 vI_event);

#endif



#if defined(RADIO_CONTROL_C) || defined (SERVICE_FOLLOWING_C) || defined(SERVICE_FOLLOWING_BACKGROUND_C) || defined(SERVICE_FOLLOWING_EXTERNAL_INTERFACE_API_C)

/*
* Procedure for ServiceID Selection
* app = provide information on selected tuner
* vI_SearchedServiceID =  service ID to be searched
* may be DAB Service ID, or RDS PI
* keepDecoding / InjectionSide, noOutputSwitch = same parameter than "tunefrequency"
* isInternalTune : identify if this is a internal or external request for Tune.
* 
*/
extern tSInt DABMW_ServiceFollowing_ServiceIDSelection(DABMW_mwAppTy vI_app, tU32 vI_SearchedServiceID, tBool vI_keepDecoding,tU8 vI_injectionSide, tBool vI_noOutputSwitch);

/* procedure for DAB notification callback
*/
extern tVoid DABMW_SF_DabAfCheckCallbackNotification(DABMW_mwAppTy vI_application, tU32 vI_frequency, tPVoid pIO_paramPtr, DABMW_dabStatusTy *pI_notificationStatus);

// codex #319038
// callback for audio status info

extern tVoid DABMW_ServiceFollowing_AudioStatusCallbackNotification(DABMW_mwAppTy vI_application);
    

#endif

#if defined(RADIO_CONTROL_C) || defined(SERVICE_FOLLOWING_MAINLOOP_C)

/* procedure to build payload
*/
extern tU8 DABMW_ServiceFollowing_BuildPayloadNotifications (tPU8 pO_payloadBuffer, tU16 vI_bufferLen);

extern tU16 DABMW_ServiceFollowing_BuildLogPayloadMsg (tPU8 pO_payloadBuffer, tU16 vI_bufferLen, tBool vI_isAuto, tU8 vI_version);


/* END EPR change */

#endif


extern tSInt DABWM_SetServiceFollowingParameters  (DABMW_mwAppTy app, tU16 *cellIndex, tS32 *cellParam, tS16 cellParamSize);


extern tSInt DABWM_GetServiceFollowingParameters  (DABMW_mwAppTy app, tU16 *cellIndex, tS32 *cellParam, tS16 cellParamSize);

// Procedure to enable Service Following
extern tVoid DABWM_ServiceFollowing_Enable(tVoid);

// Procedure to disable Service Following
extern tVoid DABWM_ServiceFollowing_Disable(tVoid);

// procedure to know if SF is enable or not
extern tBool DABWM_ServiceFollowing_IsEnable(tVoid);



#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_H_

// End of file

