//!
//!  \file      service_following_external_DAB.h
//!  \brief     <i><b> Service following implementation : external interface definition => dab part </b></i>
//!  \details   This file provides functionalities for external interface : dab part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_H

#include "fic_common.h"
#include "fic_broadcasterdb.h"

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
	
/* EPR change */
/* Add structures for callback registering */
typedef tVoid (*DABMW_SF_DabFigCallbackTy) (tU8 vI_fig, tU8 vI_ext, tU32 vI_freq, tU8 vI_ecc, tU16 id, tPVoid params);;

typedef struct
{
    tU32 fic_ber;
    tU32 audio_ber;
    tU8  audio_ber_level;
    tU8  reed_solomon_information;
	tU8  sync_status;
    tBool mute_flag;
    tBool service_selected;
    DABMW_componentTypeTy kindOfComponentType;  
    DABMW_componentEnumTy component_type;
} DABMW_SF_dabQualityTy;


/*
*********************
* VARIABLE SECTION
**********************
*/
#ifdef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_C
static 	EtalBcastQualityContainer v_etalDabQuality;
#endif

/*
*********************
* FUNCTIONS SECTION
**********************
*/
// this concerns not all SF


#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetCurrentEnsemble (DABMW_SF_mwAppTy app);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_SF_mwAppTy vI_app, tU8 vI_selectionType, tU32 vI_ensembleId, tU32 vI_Sid);
GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetApplicationEnsembleId(DABMW_SF_mwAppTy app);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_GetServiceList(tU32 vI_ensembleId, tVoid** dataPtr);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_ServiceSelect(DABMW_SF_mwAppTy vI_app, tU8 vI_selectionType, tU32 vI_ensembleId, tU32 vI_Sid);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_GetServiceTypeFromServiceId(tU32 vI_ensembleId, tU32 vI_ServiceId, DABMW_componentEnumTy* pO_componentType, DABMW_componentTypeTy* pO_streamType);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_GetEnsembleList (DABMW_ensembleUniqueIdTy **pO_dataPtr);


GLOBAL DABMW_SF_dabQualityTy DABMW_ServiceFollowing_ExtInt_GetDabQuality(DABMW_SF_mwAppTy vI_app);
GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetDabQualityFicBer(DABMW_SF_mwAppTy vI_app);
GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBer(DABMW_SF_mwAppTy vI_app);
GLOBAL tU8 DABMW_ServiceFollowing_ExtInt_GetDabQualityReedSolomon(DABMW_mwAppTy app);
GLOBAL tU8 DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioBerLevel(DABMW_SF_mwAppTy vI_app);
GLOBAL tU8 DABMW_ServiceFollowing_ExtInt_GetDabQualityAudioMuteFlag(DABMW_mwAppTy app);
GLOBAL tU8 DABMW_ServiceFollowing_ExtInt_GetDabSyncStatus(DABMW_SF_mwAppTy vI_app);

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_DabFigLandscapeRegisterForFigAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_SF_DabFigCallbackTy callback, tU8 fig, tU8 ext, tBool reuse);
GLOBAL tBool DABMW_ServiceFollowing_ExtInt_DabFigLandscapeDeRegister(DABMW_SF_DabFigCallbackTy callback, tU32 frequency, tU8 fig, tU8 ext);

GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_BrDbLinkageSetBuildCheckArray(DABMW_SF_mwAppTy app, tU32 id, DABMW_idTy kindOfId, DABMW_BrDbLinkageSetTy **lsarray, tU32 *size, tU32 filter);

#undef GLOBAL


#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DAB_H
