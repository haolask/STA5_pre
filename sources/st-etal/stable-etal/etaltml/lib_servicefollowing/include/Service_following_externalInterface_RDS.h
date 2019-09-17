//!
//!  \file      service_following_external_RDS.h
//!  \brief     <i><b> Service following implementation : external interface definition => RDS</b></i>
//!  \details   This file provides functionalities for external interface : RDS part
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2015.08.10
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_H
#define SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_H

/*
*********************
* DEFINE SECTION
**********************
*/

/* Defines for RDS control */
#define SF_RDS_ENABLE_BLOCK_TO_ACTIVATE 0x03
#define SF_RDS_ENABLE_Z	true
#define SF_RDS_ENABLE_SYN	true
#define SF_RDS_ENABLE_DOK	true
#define SF_RDS_ENABLE_CRITICAL_DATA_THR (tU8)1
#define SF_RDS_ENABLE_QC_THR	(tU8)6
#define SF_RDS_ENABLE_CP_THR	(tU8)1

// value for fast PI
#define SF_RDS_ENABLE_Z_FAST_PI			true
#define SF_RDS_ENABLE_SYN_FAST_PI		false
#define SF_RDS_ENABLE_DOK_FAST_PI		false
#define SF_RDS_ENABLE_CRITICAL_DATA_THR_FAST_PI (tU8)1
#define SF_RDS_ENABLE_QC_THR_FAST_PI	(tU8)0x0F
#define SF_RDS_ENABLE_CP_THR_FAST_PI	(tU8)2


/*
*********************
* STRUCTURE SECTION
**********************
*/
	
typedef tVoid (*DABMW_SF_AmFmRdsCallbackTy) (tU32 pi, tU32 freq, tPVoid params);
typedef DABMW_storageSourceTy DABMW_SF_RDSSourceTy;

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

#ifndef SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

GLOBAL tBool DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPiAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_SF_AmFmRdsCallbackTy callback, tBool reuse);
GLOBAL tBool DABMW_ServiceFollowing_ExtInt_AmFmLandscapeRegisterForPsAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_SF_AmFmRdsCallbackTy callback, tBool reuse);
GLOBAL tBool DABMW_ServiceFollowing_ExtInt_AmFmLandscapeDeRegister (DABMW_SF_AmFmRdsCallbackTy callback, tU32 frequency);

GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPiFromFreq (tU32 vI_freq);
GLOBAL tPU8 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetPsFromFreq (tU32 vI_freq);
GLOBAL tU32 DABMW_ServiceFollowing_ExtInt_AmFmLandscapeGetValidPiFromFreq(tU32 vI_freq, SF_tMSecond vI_piValidityDuration);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_GetNumber(tU32 piValue);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_AmFmLandscapeSearchForPI_FreqList (tPU32 dstPtr, tU32 piValue, tU8 vI_ptrLen);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RdsGetAfList (tPU8 pO_AFList_dataPtr, tU32 vI_piVal, tU32 vI_referenceFreqForAf, tU32 vI_maxNumberRetrieved);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RdsDataSetupPiDetectionMode (DABMW_SF_mwAppTy vI_app, tBool vI_Isfast_PI_detection);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RdsEnable (DABMW_SF_mwAppTy vI_app, tBool vI_Isfast_PI_detection);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_RdsDisable (DABMW_SF_mwAppTy vI_app);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOn(DABMW_SF_mwAppTy vI_app);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_DABMW_RdsDataAcquisitionOff(DABMW_SF_mwAppTy vI_app);
GLOBAL tSInt DABMW_ServiceFollowing_ExtInt_IncreasePiMatchingAF (DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, tU32 vI_expected_PI);



#undef GLOBAL


#endif // SERVICE_FOLLOWING_EXTERNAL_INTERFACE_RDS_H

