//!
//!  \file      service_following_background.c
//!  \brief     <i><b> Service following implementation : external interface definition => OSAL </b></i>
//!  \details   This file provides functionalities for service following background check, scan and AF check
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!



#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_H


/*
*********************
* DEFINE SECTION
**********************
*/

// log for SF
#ifdef  CONFIG_ENABLE_CLASS_APP_DABMW_SF
#define DABMW_SF_PRINTF(level, ...) \
	OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW_SF, __VA_ARGS__)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
#else
#define DABMW_SF_PRINTF(level, ...)
#endif 

/*
*********************
* STRUCTURE SECTION
**********************
*/
	
typedef DABMW_mwCountryTy DABMW_SF_mwCountryTy;
typedef DABMW_systemBandsTy DABMW_SF_systemBandsTy;
typedef DABMW_mwAppTy DABMW_SF_mwAppTy;
typedef ETAL_HANDLE   DABMW_SF_mwEtalHandleTy;


/*
*********************
* VARIABLE SECTION
**********************
*/



/*
*********************
* FUNCTIONS SECTION
**********************
*/

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

GLOBAL DABMW_SF_mwCountryTy DABMW_ServiceFollowing_ExtInt_GetCountry (tVoid);

GLOBAL DABMW_SF_systemBandsTy DABMW_ServiceFollowing_ExtInt_GetApplicationTableSystemBand(tU32 vI_frequency);

GLOBAL ETAL_HANDLE DABMW_ServiceFollowing_ExtInt_GetEtalHandleFromApp(DABMW_SF_mwAppTy app);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_StoreEtalHandleFromApp(ETAL_HANDLE vI_etalHandle, DABMW_SF_mwAppTy app);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_IsApplicationDab (DABMW_SF_mwAppTy app);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_IsApplicationAmFm (DABMW_SF_mwAppTy app);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_IsFMBand (DABMW_SF_systemBandsTy band);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_IsDABBand (DABMW_SF_systemBandsTy band);

GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetFrequencyFromApp(DABMW_SF_mwAppTy vI_app);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_GetApplicationBusyStatus (DABMW_SF_mwAppTy vI_app);

GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmin (DABMW_SF_systemBandsTy vI_bandValue, DABMW_SF_mwCountryTy vI_countryId);

GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetBandInfoTable_freqmax (DABMW_SF_systemBandsTy vI_bandValue, DABMW_SF_mwCountryTy vI_countryId);

GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetNextFrequencyFromFreq (tU32 vI_frequency, DABMW_SF_systemBandsTy vI_systemBand, tBool vI_DirectionIsUp);


GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RegisterExternalTune(tVoid);
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_TuneCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RegisterExternalAudioSourceSelection(tVoid);
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_AudioSourceSelectionCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);


GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RegisterExternalServiceSelect(tVoid);
GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_ServiceSelectCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);

GLOBAL tVoid DABMW_ServiceFollowing_ExtInt_ReceiverDestroyCallback(ETAL_HANDLE hGeneric, void *param, tU32 param_len, tU32 context);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AllocateEtalHandleFromApp(DABMW_SF_mwAppTy vI_app);


#undef GLOBAL

#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_H

