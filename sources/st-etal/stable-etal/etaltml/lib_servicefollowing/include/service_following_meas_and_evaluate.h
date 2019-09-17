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

#ifndef SERVICE_FOLLOWING_MEAS_AND_EVALUATE_H_
#define SERVICE_FOLLOWING_MEAS_AND_EVALUATE_H_

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
* MACRO SECTION
**********************
*/

#define DABMW_SERVICE_FOLLOWING_LOG_QUALITY_RANGE(x) ((DABMW_SF_QUALITY_IS_NO_SYNC == x)?"NO_SYNC":((DABMW_SF_QUALITY_IS_NO_AUDIO == x)?"NO_AUDIO":((DABMW_SF_QUALITY_IS_POOR == x)?"POOR":((DABMW_SF_QUALITY_IS_MEDIUM == x)?"MEDIUM":"GOOD"))))

//counter for original and alternate quality logging : range is multiple of 0.5s
#define DABMW_SERVICE_FOLLOWING_LOG_COUNTER_QUALITY  10 

/*
*********************
* STRUCTURE SECTION
**********************
*/

/* Structure defining the action depending on current cell
*/

/* Enum to identify which check of time is needed
*/
typedef enum
{
    DABMW_SF_EVALUATE_RES_NO_ACTION                     = 0,
    DABMW_SF_EVALUATE_RES_NO_ACTION_GOOD_CELL   		 = 1,
 	DABMW_SF_EVALUATE_RES_NO_ACTION_MEDIUM_CELL          = 2,
	DABMW_SF_EVALUATE_RES_NO_ACTION_POOR_CELL			 = 3,       
	DABMW_SF_EVALUATE_RES_SERVICE_RECOVERY				 = 4,
	DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_DAB_ONLY	 = 5,
	DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR			 = 6,
	DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_FM_ONLY	 = 7,
	DABMW_SF_EVALUATE_RES_EVALUATE_NEIGHBOOR_IS_POOR	 = 8,
	DABMW_SF_EVALUATE_CHANGE_CELL						 = 9,
	DABMW_SF_EVALUATE_CHANGE_CELL_SEAMLESS				 = 10
} DABMW_SF_EvaluationActionResultTy;


/*
*********************
* VARIABLE SECTION
**********************
*/

/* CONST
*/



/* variables are belonging to SERVICE_FOLLOWING_MEAS_AND_EVALUATE_C
*/

/*
*********************
* FUNCTIONS SECTION
**********************
*/
#ifndef SERVICE_FOLLOWING_MEAS_AND_EVALUATE_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

/* here it is defined in service_following_internal.h */

/*
*  Procedure to evaluate the original cell suitability
* 
*/

GLOBAL DABMW_SF_EvaluationActionResultTy DABMW_ServiceFollowing_EvaluateOriginalCellSuitability(tVoid);

/*
*  Procedure to measure the original cell 
*/
GLOBAL tVoid DABMW_ServiceFollowing_MeasureOrignalCell(tVoid);


/*
*  Procedure to measure the AF cell suitability
*/
GLOBAL tVoid DABMW_ServiceFollowing_MeasureAFCell(tVoid);

/*
* Procedure to evaluate the AF cell suitability
*/


GLOBAL DABMW_SF_EvaluationActionResultTy DABMW_ServiceFollowing_EvaluateAFCellSuitability(tVoid);

/*
*  Procedure to retrieve quality level of a given measure depending on techno
*
* quality level = poor, medium, good
*/
GLOBAL DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityLevelDAB(tU32 vI_ficBerValue);

/*
*  Procedure to retrieve quality level of a given measure depending on techno
*
* quality level = poor, medium, good
*/
GLOBAL DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityLevelFM(tU32 vI_ficBerValue);

/*
*  Procedure to check if an new found AF is better current Alternate
* output : true if new AF better
*
*/
GLOBAL tBool DABMW_ServiceFollowing_IsNewFoundAFBetter(DABMW_mwAppTy vI_newAFApp, DABMW_SF_QualityTy vI_newFoundQuality);

/*
*  Procedure to check if an new found AF is acceptable
* output : true if acceptable quality AF 
*/

GLOBAL tBool DABMW_ServiceFollowing_IsFoundAFAcceptable(DABMW_mwAppTy vI_newAFApp, DABMW_SF_QualityTy vI_newFoundQuality);

// procedure check if quality above threshold for RDS decoding
GLOBAL tBool DABMW_ServiceFollowing_IsFMQualityEnoughForRdsDecoding(DABMW_SF_amfmQualityTy vI_newFoundQuality);
    
// procedure to check if frequency is considered Sync
GLOBAL tBool DABMW_ServiceFollowing_IsOutOfSync(DABMW_SF_dabQualityTy vI_DabQualityValue);

// procedure to init the SF quality structure
//
GLOBAL DABMW_SF_QualityTy DABMW_ServiceFollowing_QualityInit(tVoid);

GLOBAL DABMW_SF_dabQualityTy DABMW_ServiceFollowing_DabQualityInit(tVoid);


/*
 *	Procedure to retrieve quality range  of a given measure for DAB
 *  Input = quality value structure
 *
 * Output 
 * quality level = poor, medium, good
 */
GLOBAL DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusDAB(DABMW_SF_QualityTy vI_qualityValue);

GLOBAL DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusDABwithHysteresis(DABMW_SF_QualityTy vI_qualityValue);

/*
 *	Procedure to retrieve quality range  of a given measure for FM
 *  Input = quality value structure
 *
 * Output 
 * quality level = poor, medium, good
 */
GLOBAL DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusFM(DABMW_SF_QualityTy vI_qualityValue);

GLOBAL DABMW_SF_QualityStatusTy DABMW_ServiceFollowing_GetQualityStatusFMwithHysteresis(DABMW_SF_QualityTy vI_qualityValue);


// procedure to compare 2 quality values in DAB.
//
GLOBAL tBool DABMW_ServiceFollowing_qualityA_better_qualityB_DAB(DABMW_SF_QualityTy vI_qualityA, DABMW_SF_QualityTy vI_qualityB, tU32 vI_freqA, tU32 vI_freqB);

// procedure to compare 2 quality values in FM.
//
GLOBAL tBool DABMW_ServiceFollowing_qualityA_better_qualityB_FM(DABMW_SF_QualityTy vI_qualityA, DABMW_SF_QualityTy vI_qualityB, tU32 vI_freqA, tU32 vI_freqB);

// Procedure which compare quality A and quality B for 1 DAB & 1 FM 
//
// returns true is quality A is better than B
//
// 
GLOBAL tBool DABMW_ServiceFollowing_qualityA_DAB_better_qualityB_FM(DABMW_SF_QualityTy vI_qualityA, DABMW_SF_QualityTy vI_qualityB, tU32 vI_freqA, tU32 vI_freqB);


// Procedure to retrieve DAB Quality information on an app.
GLOBAL DABMW_SF_dabQualityTy DABMW_ServiceFollowing_DAB_GetQuality(DABMW_mwAppTy vI_app);

// Procedure to handle the measurements ponderations for original freq
// ie : find the balance between the new meas, and the store one to get new value.
// This procedure does the new measurement of quality on vI_app 
// and return the ponderated value for this app
// based on ponderation choice.
// 
GLOBAL DABMW_SF_QualityTy DABMW_ServiceFollowing_MeasureAndPonderateOriginalCell(DABMW_mwAppTy vI_app);

// Procedure to handle the measurements ponderations for alternate freq
// ie : find the balance between the new meas, and the store one to get new value.
// This procedure does the new measurement of quality on vI_app 
// and return the ponderated value for this app
// based on ponderation choice.
// 
GLOBAL DABMW_SF_QualityTy DABMW_ServiceFollowing_MeasureAndPonderateAlternateCell(DABMW_mwAppTy vI_app);


// Procedure to handle the measurements ponderations for DAB quality
// ie : find the balance between the new meas, and the store one to get new value.
GLOBAL DABMW_SF_dabQualityTy DABMW_ServiceFollowing_MeasureAndPonderateCellDAB(DABMW_SF_dabQualityTy vI_current_DAB_Quality, DABMW_SF_dabQualityTy vI_measured_DAB_Quality);

// Procedure to handle the measurements ponderations for FM quality
// ie : find the balance between the new meas, and the store one to get new value.

GLOBAL DABMW_SF_amfmQualityTy DABMW_ServiceFollowing_MeasureAndPonderateCellFM(DABMW_SF_amfmQualityTy vI_current_FM_Quality, DABMW_SF_amfmQualityTy vI_measured_FM_Quality);


// procedure to configure the right parameter on seek interface
//
GLOBAL tVoid DABMW_ServiceFollowing_ConfigureAutoSeek(tVoid);

// procedure to display the FM quality
GLOBAL tVoid DABMW_ServiceFollowing_DisplayQualityFM(DABMW_SF_amfmQualityTy vI_quality, tPChar printHeader);

// procedure to display the DAB quality
GLOBAL tVoid DABMW_ServiceFollowing_DisplayQualityDAB(DABMW_SF_dabQualityTy vI_quality, tPChar printHeader);

GLOBAL tSInt DABMW_ServiceFollowing_GetDabAudioQuality(DABMW_mwAppTy vI_application);

#undef GLOBAL


#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_MEAS_AND_EVALUATE_H_

// End of file

