//!
//!  \file      service_following_external_FM.h
//!  \brief     <i><b> Service following implementation : external interface definition => FM part </b></i>
//!  \details   This file provides functionalities for external interface : FM part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_H

/*
*********************
* DEFINE SECTION
**********************
*/
// some define
#define DABMW_SF_VPA_FEATURE_FLAG_MASK  (((tU8)0x01) << 2)
// reset frequency
#define DABMW_SF_RESET_FREQUENCY	0

#define SF_EXT_CMOST_MUTE	0
#define SF_EXT_CMOST_UNMUTE	1

/*
*********************
* STRUCTURE SECTION
**********************
*/
	
//!
//! \typedef The type of the SF  time variable	
typedef tVoid (*DABMW_SF_SeekCallbackTy)(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, tPVoid params);
typedef DABMW_amfmQualityTy DABMW_SF_amfmQualityTy;


typedef struct
{
	DABMW_SF_SeekCallbackTy v_DABMW_SF_FM_AutoSeek_Callback;
	DABMW_SF_mwAppTy v_app;
	ETAL_HANDLE 	 v_handle;
} DABMW_SF_SeekCallbackInfoTy;

/*
*********************
* VARIABLE SECTION
**********************
*/
#ifdef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_C
static DABMW_SF_SeekCallbackInfoTy v_DABMW_SF_FM_AutoSeek_CallbackInfo;
static EtalSeekStatus		   v_DABMW_SF_FM_AutoSeek_Result;
#endif

/*
*********************
* FUNCTIONS SECTION
**********************
*/

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFSwitchRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekStart(DABMW_SF_mwAppTy vI_app, tS16 deltaFreq, DABMW_SF_SeekCallbackTy pI_callBack );
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekContinue(DABMW_SF_mwAppTy vI_app);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmSeekEnd(DABMW_SF_mwAppTy vI_app);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFCheckRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFStartRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmAFEndRequest(DABMW_SF_mwAppTy vI_app);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_FM_MuteUnmute(DABMW_SF_mwAppTy vI_app, tBool vI_muteRequested);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_IsVpaEnabled(tVoid);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_EnableVPA(tVoid);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_DisableVPA(tVoid);


GLOBAL DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_ExtInt_AmFmGetQuality(DABMW_mwAppTy vI_app);
GLOBAL DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_ExtInt_AmFmQualityInit(tVoid);

GLOBAL tSInt DABWM_ServiceFollowing_ExtInt_SetAmFmAutoSeekThreshold(DABMW_mwAppTy vI_app, 
                                            tU16 vI_fieldStrenghThreshold,
                                            tU16 vI_adjacentChannel_Threshold,
                                            tU16 vI_detuningChannel_Threshold,
                                            tU16 vI_multipath_Threshold,
                                            tU16 vI_combinedQuality_Threshold, 
                                            tU16 vI_snr_Threshold,
                                            tU16 vI_MpxNoiseThreshold);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_SetAutoSeekData (DABMW_SF_mwAppTy app, tU32 startFrequency, tU32 stopFrequency, tBool directionIsUp, tU32 min_freq_band, tU32 max_freq_band);
GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetSeekFrequency(DABMW_SF_mwAppTy vI_app);
GLOBAL tBool DABMW_ServiceFollowing_ExtInt_SeekEndedOnGoodFreq(DABMW_SF_mwAppTy vI_app);
GLOBAL tBool DABMW_ServiceFollowing_ExtInt_SeekFullCycleDone(DABMW_SF_mwAppTy vI_app);
GLOBAL DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_ExtInt_GetSeekQuality(DABMW_SF_mwAppTy vI_app);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_SetDISSModeDSP(DABMW_mwAppTy vI_app, tU8 vI_DISSMode);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_SetDISSFilterDSP(DABMW_mwAppTy vI_app, tU32 vI_DISSFilter);

GLOBAL 	tVoid DABMW_ServiceFollowing_ExtInt_AmFmAutoSeekCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);

#undef GLOBAL


#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_FM_H

