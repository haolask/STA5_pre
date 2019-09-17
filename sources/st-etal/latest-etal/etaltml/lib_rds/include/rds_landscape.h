//!
//!  \file 		rds_landscape.h
//!  \brief 	<i><b> RDS landscape header </b></i>
//!  \details	RDS landscape header
//!  \author 	Alberto Saviotti
//!  \author 	(original version) Alberto Saviotti
//!  \version   1.0
//!  \date 		2012.02.06
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef RDS_LANDSCAPE_H_
#define RDS_LANDSCAPE_H_

#ifdef __cplusplus
extern "C" {
#endif

// Storage space related definition
#if defined (CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER)
    #define DABMW_AMFM_LANSCAPE_SIZE                    (tU16)CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER
#else
    #define DABMW_AMFM_LANSCAPE_SIZE                    (tU8)24
#endif // #if defined (CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER)

#define DABMW_AF_LIST_NUMBER_PER_PI                 (tU8)20

//#define DABMW_AF_LIST_BFR_LEN                       (tU8)26 // 25 + 1 (base)

// The AF list type must be confirmed at least this number of types before to
// be declcared good because if not the AF list could be completely wrong
#define DABMW_AF_LIST_TYPE_CONFIRMED_NUMBER         3

// Type for AF data storage
#ifndef CONFIG_TARGET_CPU_COMPILER_GNU
typedef __packed struct
#else
typedef struct
#endif
{
    DABMW_storageStatusEnumTy status;

    DABMW_storageStatusEnumTy sendStatus;
    
    tU8 afNum;
    
    tU8 af[DABMW_AF_LIST_BFR_LEN];
} DABMW_afStorageTy;

// AF db entries
#ifndef CONFIG_TARGET_CPU_COMPILER_GNU
typedef __packed struct
#else
typedef struct
#endif
{
    DABMW_storageStatusEnumTy status;
    
    tU8 baseFreq;
    tBool isAfListTypeB;

    tSInt afListTypeConfirmed;

    DABMW_afStorageTy afEntries[DABMW_AF_LIST_NUMBER_PER_PI];
} DABMW_afLandscapeTy;

typedef tVoid (*DABMW_AmFmRdsCallbackTy) (tU32 pi, tU32 freq, tPVoid params);

extern tVoid DABMW_RdsAfDbOnChannelChange (tVoid);

extern tSInt DABMW_RdsGetAfList (ETAL_HANDLE hReceiver, tPU8 dataPtr, tBool forcedGet, tU32 piVal, tU32 baseFreq, 
                                 tBool mode, tU32 maxNumberRetrieved);

extern tSInt DABMW_AmFmLandscapeSetFrequencyWithoutPi (tU32 freq, tU32 overallQuality);

extern tU32 DABMW_AmFmLandscapeGetFreqFromPi (tU32 piVal);

extern tSInt DABMW_AmFmLandscapeSetPi (tU32 piVal, tU32 freq);

extern tSInt DABMW_AmFmLandscapeSetPs (tU32 piVal, tPU8 psPtr, tU32 freq);

extern tSInt DABMW_AmFmLandscapeSetPTY (tU32 piVal, tU8 pty, tU32 freq);
extern tSInt DABMW_AmFmLandscapeGetPTY (tU32 piVal, tU8 *pty, tU32 freq);

extern tSInt DABMW_AmFmLandscapeSetFreq (tU32 piVal, tU32 freq);

extern tSInt DABMW_AmFmLandscapeSetAf (tU32 piVal, tU8 rxFreqNum, tPU16 afValPtr, tU32 freq);

extern tSInt DABMW_RdsLandscapeInit (tBool nvmRead);

extern tSInt DABMW_RdsLandscapeClose (tVoid);

extern tSInt DABMW_RdsLandscapeClear (tBool clearRam);

extern tSInt DABMW_GetAmFmServicesList (tPU8 dstPtr);

extern tSInt DABMW_GetAmFmSpecificServiceData (tPU8 dstPtr, tU32 piValue, tU32 freqValue);

extern tBool DABMW_AmFmLandscapeRegisterForPiAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_AmFmRdsCallbackTy callback, tBool reuse);

extern tBool DABMW_AmFmLandscapeRegisterForPsAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_AmFmRdsCallbackTy callback, tBool reuse);

extern tBool DABMW_AmFmLandscapeRegisterForAfListAtFreq (tU32 frequency, DABMW_afStorageTy *paramPtr, DABMW_AmFmRdsCallbackTy callback);

extern tBool DABMW_AmFmLandscapeDeRegister (DABMW_AmFmRdsCallbackTy callback, tU32 frequency);

extern tU32 DABMW_AmFmLandscapeSearchForPi (tU32 piVal, tU32 freq);

extern tSInt DABMW_AmFmNumberOfStations (tVoid);

/* EPR CHANGE */
/* Add procedures for landscape */
/* procedure to get the list of Freq which maps to a pi */
tSInt DABMW_AmFmLandscapeSearchForPI_FreqList (tPU32 dstPtr, tU32 piValue, tU8 vI_ptrLen);
tSInt DABMW_AmFmLandscapeSearchForValidPI_FreqList (tPU32 dstPtr, tU32 piValue, OSAL_tMSecond vI_piValidityDuration);


/* procedure to get the number of Freq which maps to a pi */
tSInt DABMW_AmFmLandscapeSearchForPI_GetNumber(tU32 piValue);

/* procedure to get the time last stored/confirmed PI value */
extern tBool DABMW_AmFmLandscapeGetPiValidity(tU32 vI_freq, OSAL_tMSecond vI_piValidityDuration);
extern tU32 DABMW_AmFmLandscapeGetPiFromFreq (tU32 vI_freq);
extern tU32 DABMW_AmFmLandscapeGetValidPiFromFreq(tU32 vI_freq, OSAL_tMSecond vI_piValidityDuration);
/* Procedure to update the PI stored time
*/
extern tSInt DABMW_AmFmLandscapeUpdatePiStoreTime(tU32 piVal, tU32 freq);

/* END EPR CHANGE */

// EPR CHANGE 
// ADD Lanscape functionnality
//---------------------
tU32 DABMW_AmFmLandscapeGetNbValid_FreqList (OSAL_tMSecond vI_piValidityDuration);

/* 
* Add procedure to get all frequency existing for a given PI (ie a list of and not only one)
*/
tU32 DABMW_AmFmLandscapeGetValid_FreqList (EtalRdsLandscapeExternalInfo *dstPtr, tU8 vI_ptrLen, OSAL_tMSecond vI_piValidityDuration);

// Procedure to look for PS 
tPU8 DABMW_AmFmLandscapeGetPsFromFreq (tU32 vI_freq);


#ifdef __cplusplus
}
#endif

#endif // RDS_LANDSCAPE_H_

// End of file

