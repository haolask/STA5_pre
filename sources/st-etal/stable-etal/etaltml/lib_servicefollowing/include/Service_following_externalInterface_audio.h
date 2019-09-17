//!
//!  \file      service_following_external_audio.h
//!  \brief     <i><b> Service following implementation : external interface definition => audio part </b></i>
//!  \details   This file provides functionalities for external interface : audio part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_H

#include "seamless_switching_common.h"


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

typedef tVoid (*DABMW_SF_audio_msgsCallbackTy) (tPVoid);
typedef SD_msg_SeamlessEstimationRequest SF_msg_SeamlessEstimationRequest;
typedef SD_msg_SeamlessEstimationResponse SF_msg_SeamlessEstimationResponse;
typedef	SD_msg_SeamlessSwitchingRequest SF_msg_SeamlessSwitchingRequest;
typedef	SD_msg_SeamlessSwitchingResponse SF_msg_SeamlessSwitchingResponse;


/*
*********************
* VARIABLE SECTION
**********************
*/

#ifdef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_C
#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK
static DABMW_SF_audio_msgsCallbackTy v_SF_Callback_SeamlessEstimationResponse = NULL;
static DABMW_SF_audio_msgsCallbackTy v_SF_Callback_SeamlessSwitchingResponse = NULL;
#endif
static DABMW_SF_mwAppTy v_ServiceFollowing_ExtInt_currentAudioApp = DABMW_NONE_APP;
#endif

/*
*********************
* FUNCTIONS SECTION
**********************
*/

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

#ifdef CONFIG_APP_SEAMLESS_SWITCHING_BLOCK

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessEstimationResponse(DABMW_SF_audio_msgsCallbackTy callback);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderApiRegisterUserSeamlessSwitchingResponse(DABMW_SF_audio_msgsCallbackTy callback);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationRequest(SF_msg_SeamlessEstimationRequest vI_seamlessEstimationReq);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessEstimationStop(tVoid);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_StreamDecoderSeamlessSwitchingRequest(SF_msg_SeamlessSwitchingRequest vI_seamlessSwitchingReq);
#endif // #if defined (CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_SetCurrentAudioPortUser (DABMW_SF_mwAppTy app, tBool isInternalTune);
GLOBAL DABMW_SF_mwAppTy DABMW_ServiceFollowing_ExtInt_GetCurrentAudioPortUser (tVoid);

// callback for ETAL 
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_SeamlessEstimationResponseCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_SeamlessSwitchingResponseCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);

#undef GLOBAL


#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_AUDIO_H

