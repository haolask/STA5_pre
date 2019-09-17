
//!
//!  \file 		etaltml_api.h
//!  \brief 	<i><b> ETAL API </b></i>
//!  \details   Interface functions for the ETAL user application, extended functionalities
//!  \author 	Raffaele Belardi
//!

/*
 * DO NOT INCLUDE THIS FILE DIRECTLY!
 *
 * It is included by etal_api.h
 */

#ifndef ETALTML_API_H_
#define ETALTML_API_H_

#include "etaltml_types.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*
 * Radio text
 */
ETAL_STATUS etaltml_get_textinfo(ETAL_HANDLE hReceiver, EtalTextInfo *pRadiotext);
ETAL_STATUS etaltml_start_textinfo(ETAL_HANDLE hReceiver);
ETAL_STATUS etaltml_stop_textinfo(ETAL_HANDLE hReceiver);
ETAL_STATUS etaltml_get_decoded_RDS(ETAL_HANDLE hReceiver, EtalRDSData *pRDSdata);
/* enum EtalEnumDecodedRDSBitmap can be used to fill RDSServiceList parameter */
ETAL_STATUS etaltml_start_decoded_RDS(ETAL_HANDLE hReceiver, tU32 RDSServiceList);
ETAL_STATUS etaltml_stop_decoded_RDS(ETAL_HANDLE hReceiver, tU32 RDSServiceList);

ETAL_STATUS etaltml_get_validated_RDS_block_manual(ETAL_HANDLE hReceiver, tU32 *pBlocks, tU32 *pBlocksNum);

ETAL_STATUS etaltml_RDS_AF(ETAL_HANDLE hReceiverA, ETAL_HANDLE hReceiverB, tBool AFOn);
ETAL_STATUS etaltml_RDS_TA(ETAL_HANDLE hReceiver, tBool TAOn);
ETAL_STATUS etaltml_RDS_EON(ETAL_HANDLE hReceiver, tBool EONOn);
ETAL_STATUS etaltml_RDS_REG(ETAL_HANDLE hReceiver, tBool REGOn);
ETAL_STATUS etaltml_RDS_AFSearch_start(ETAL_HANDLE hReceiver, EtalRDSAFSearchData afSearchData);
ETAL_STATUS etaltml_RDS_AFSearch_stop(ETAL_HANDLE hReceiver);
ETAL_STATUS etaltml_RDS_seek_start(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, EtalRDSSeekTy *pRDSSeekOption);


/*
 * Advanced Tuning
 */
ETAL_STATUS etaltml_scan_start(ETAL_HANDLE hReceiver, tU32 audioPlayTime, etalSeekDirectionTy direction, tU32 step);
ETAL_STATUS etaltml_scan_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);
ETAL_STATUS etaltml_learn_start(ETAL_HANDLE hReceiver, EtalFrequencyBand bandIndex, tU32 step, tU32 nbOfFreq, etalLearnReportingModeStatusTy mode, EtalLearnFrequencyTy* freqList);
ETAL_STATUS etaltml_learn_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);

// add on for get Free Tuner
ETAL_STATUS etaltml_getFreeReceiverForPath(ETAL_HANDLE *pReceiver, EtalPathName vI_path, EtalReceiverAttr *pO_attr);
ETAL_STATUS etaltml_TuneOnServiceId(EtalPathName vI_path,  tU32 vI_SearchedServiceID);
ETAL_STATUS etaltml_ActivateServiceFollowing(void);
ETAL_STATUS etaltml_DisableServiceFollowing(void);
ETAL_STATUS etaltml_SelectKindOfSwitchServiceFollowing(tBool vI_fmfm, tBool vI_dabfm, tBool vI_dabdab);


// landscape functionnality
ETAL_STATUS etaltml_landscape_GetServices(ETAL_DatabaseExternalInfo *pO_LanscapeInfo, tBool vI_AmFmRequested,  tBool vI_DabRequested, tU32 vI_piValidityDuration);
ETAL_STATUS etaltml_landscape_GetNbServices(tU32 *pO_NbFreq, tBool vI_AmFmRequested,  tBool vI_DabRequested	, tU32 vI_piValidityDuration);


#ifdef __cplusplus
}
#endif

#endif /* ETALTML_API_H_ */
