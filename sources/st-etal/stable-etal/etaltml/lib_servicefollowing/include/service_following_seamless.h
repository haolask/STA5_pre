//!
//!  \file      service_following_seamless.h
//!  \brief     <i><b> Service following seamless implementation </b></i>
//!  \details   This file provides functionalities for service following seamless
//!  \author    David Pastor
//!  \author    (original version) David Pastor
//!  \version   1.0
//!  \date      2013.11.04
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_SEAMLESS_H_
#define SERVICE_FOLLOWING_SEAMLESS_H_

#include "seamless_switching_common.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
*********************
* DEFINE SECTION
**********************
*/
    /* some conversion macros */
#define DABMW_SF_SEAMLESS_SAMPLING_RATE_PER_MS                    48
#define DABMW_SF_SEAMLESS_SAMPLING_RATE_PER_S                     48000

/* default configuration for SS */

//
// Values for the window for estimation
//
// Value is in sample
//
// the rate is 48 Khz => 1 sample = 1/48000 s , 1s = 48000 samples
//
// the default configuration is DAB to FM with FM first, with a delay up to 10s between DAB & FM
// so delay is between -10s to 0s
// ie start = -10s, = -480000 samples
// stop = 0s, = 0 sample
//
// if we dive into seamless estimation : 
// a block is 16384 samples , downsampled. so there is 16384*DownSamplingFactor sample/s, 
// correlation is 16384 samples. 
//
//
//

// here : the window for full estimation the default for full estimation
#define DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_MS      (DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_SAMPLE))
#define DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_MS       (DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_SAMPLE))
#define DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_SAMPLE  (SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_SAMPLE)
#define DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_SAMPLE   (SD_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_SAMPLE)

// here : the window for reconfirmation estimation
// We want a reconfirmation with +- 0.5 s
// so 0.5 s = 500 MS => 500 * 48 sample
#define DABMW_SF_DEFAULT_CONFIG_SS_RECONF_WINDOWS_HALF_SIZE_IN_MS       500
#define DABMW_SF_DEFAULT_CONFIG_SS_RECONF_WINDOWS_HALF_SIZE_IN_SAMPLES  (DABMW_SF_SEAMLESS_SAMPLING_RATE_PER_MS*DABMW_SF_DEFAULT_CONFIG_SS_RECONF_WINDOWS_HALF_SIZE_IN_MS)

// Loudness window duration in second
// default value is 3s
//
#define DABMW_SF_DEFAULT_CONFIG_SS_LOUDNESS_DURATION_SECOND	            3

// the DownSampling rate for complete window estimation
#define DABMW_SF_DEFAULT_CONFIG_SS_DOWN_SAMPLING			            0x08

// the DownSampling rate for reconf window 
#define DABMW_SF_DEFAULT_CONFIG_SS_DOWN_SAMPLING_RECONF		            0x08


#define DAMBW_SF_MEASURE_VALIDITY_INFINITE                      0
#define DABMW_SF_DEFAULT_CONFIG_SS_MEASURE_VALIDITY             DAMBW_SF_MEASURE_VALIDITY_INFINITE // means infinite
#define DABMW_SF_DEFAULT_CONFIG_SS_RECONF_PERIODICY             600000 // every 10 minutes      
#define DABMW_SF_DEFAULT_CONFIG_SS_MINIMUM_SPACING_TIME         60000  // every minute    
#define DABMW_SF_DEFAULT_CONFIG_SS_CONFIDENCE_LEVEL_THRESHOLD   87

//
#define DABMW_SF_SS_MARGIN_BEFORE_RECONF_LAUNCH_MS              1000 
#define DABMW_SF_SS_DELAY_FOR_ESTIMATION_AFTER_ALTERNATE_TUNED  (DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_STOP_IN_MS - DABMW_SF_DEFAULT_CONFIG_SS_FULL_WINDOWS_START_IN_MS)

/* MSG DEFINE */
#define DABMW_SEAMLESS_ESTIMATION_MSG_LEN 	DABMW_SEAMLESS_ESTIMATION_PAYLOAD_LEN + 1
#define DABMW_SEAMLESS_ESTIMATION_START_CMD_MODE_1			0x01
#define DABMW_SEAMLESS_ESTIMATION_START_CMD_MODE_2			0x02
#define DABMW_SEAMLESS_ESTIMATION_START_CMD_MODE_3			0x03
#define DABMW_SEAMLESS_ESTIMATION_RESPONSE_PAYLOAD_LEN      ((tU8)30)

/* Database Size */
// increase to more than 1 : now we can imagine several DAB / FM.. let's say 5 
//
#define DABMW_SF_SS_DATABASE_SIZE							5

// error counter THRESHOLD
#define DABMW_SF_SS_THRESHOLD_ERROR_COUNTER                 20   


// convertion sample to MS
#define DABMW_SEAMLESS_ESTIMATION_CONVERT_SAMPLE_IN_MS(x)       (x / DABMW_SF_SEAMLESS_SAMPLING_RATE_PER_MS)    
#define DABMW_SEAMLESS_ESTIMATION_CONVERT_MS_IN_SAMPLE(x)       (x * DABMW_SF_SEAMLESS_SAMPLING_RATE_PER_MS)

// absolute value
#define DABMW_ABS(x) ((x>0)?x:(-x))


// loudness conversion
// Loudness in db
#define DABMW_SEAMLESS_ESTIMATION_CONVERT_LOUDNESS_IN_DB(x)     ((tDouble)(5 * log10(x)) - 0.691)



/* some macros for logging */
#define DABMW_SF_SEAMLESS_LOG_STATUS(status)       ((status == SD_SEAMLESS_ESTIMATION_STATUS_SUCCESS)?"SUCCESS":\
                                                     ((status == SD_SEAMLESS_ESTIMATION_STATUS_FAILURE)?"FAILURE":\
                                                      ((status == SD_SEAMLESS_ESTIMATION_STATUS_STOPPED)?"STOPPED":\
                                                       ((status == SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_DELAY_ESTIMATION)?"ERROR_ON_DELAY_ESTIMATION":\
                                                        ((status == SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_LOUDNESS_ESTIMATION)?"ERROR_ON_LOUDNESS_ESTIMATION":\
                                                         ((status == SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_BUFFERING)?"ERROR_ON_BUFFERING":\
                                                          ((status == SD_SEAMLESS_ESTIMATION_STATUS_ERROR_ON_SEAMLESS_ESTIMATION)?"ERROR_ON_SEAMLESS_ESTIMATION":"NONE")))))))
                                                          
                                                        
#define DABMW_SF_SEAMLESS_LOG_PROVIDER_TYPE(provider_type) ((provider_type == SD_SEAMLESS_PROVIDER_IS_DAB)?"PROVIDER_IS_DAB":\
                                                            ((provider_type == SD_SEAMLESS_PROVIDER_IS_FM)?"PROVIDER_IS_FM":"PROVIDER_IS_NONE"))


#define DABMW_SF_SEAMLESS_SYSTEM_TO_SWITCH(system_to_switch) ((system_to_switch == SD_SEAMLESS_SWITCHING_SWITCH_TO_DAB)?"SS_SWITCH_TO_DAB":\
																((system_to_switch == SD_SEAMLESS_SWITCHING_SWITCH_TO_FM)?"SS_SWITCH_TO_FM":"SWITCH_TO_EARLY_FM"))

/*
*********************
* STRUCTURE SECTION
**********************
*/
/* strucutre to store the parameter configuration for Seamless estimation */
typedef struct
{
	tS32 FullEstimationStartWindowInSample; //  the start window for full estimation in sample
	tS32 FullEstimationStartWindowInMs;     //  the start window for full estimation in millisecond    
	tS32 FullEstimationStopWindowInSample;  // the stop window stone for full estimation
	tS32 FullEstimationStopWindowInMs;     //  the stop window for full estimation in millisecond  
    tS32 ReconfEstimationWindowSizeInSample;        // the size of the reconf window in sample
    tS32 ReconfEstimationWindowSizeInMs;        // the size of the reconf window in sample
	tU8 LoudnessEstimationDurationInSec; // the loudness estimations duration in second
	tU8 DownSampling; // down sampling factor applied for delay estimation.
	tU8 DownSamplingReconf;
	tBool serviceSelected;
    SF_tMSecond measureValidityDuration;
    SF_tMSecond reconfPeriodicity;
    SF_tMSecond minimumTimeBetween2Seamless;
    tU32 ConfidenceLevelThreshold;
} DABMW_SS_EstimationParameterTy;

/* strucutre to store the parameter configuration for Seamless estimation */
typedef struct
{
	tBool IsResponseReceived;
	tBool IsEstimationOnGoing;
	SD_SeamlessEstimationModeTy requestedMode;
    SF_tMSecond timeLastEstimationDone;
} DABMW_SS_EstimationResponseParameterTy;

/* Database for result storing
* what we store here is a database of the delay so that it can be reuse and save time for switch
*
* The result is a pair of 
* 1) DAB Frequency / EID / SID
* 2) FM PI
* note it is assume that whatever the FM frequency the FM being synchronize, the delay DAB - FM will remain unchange
*
* for each pair what is store is 
* 1) time when delay as been estimated
* 2) Estimation Delay result
*
*/
typedef struct
{
	tU32 DAB_Frequency;
	tU32 DAB_Eid;
    tU32 DAB_Sid;
	tU32 FM_PI;
	SF_tMSecond TimeLastEstimated;
	SF_msg_SeamlessEstimationResponse SS_EstimationResult;
    tU8  error_counter;
} DABMW_SS_EstimationResutDatabaseTy;

// Structure to store the last requested switch
typedef struct
{
	SF_tMSecond TimeLastSwitchRequested;
	SF_msg_SeamlessSwitchingRequest SS_SwitchRequest;
	SF_msg_SeamlessSwitchingResponse SS_SwitchResponse;	
	tBool switchRequestValidity;
} DABMW_SF_SS_LastSwitchRequestInfoTy;


/*
*********************
* VARIABLE SECTION
**********************
*/
#ifndef SERVICE_FOLLOWING_SEAMLESS_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

/* CONST
*/


/* variables are belonging to SERVICE_FOLLOWING_SEAMLESS_C
*/

GLOBAL DABMW_SS_EstimationParameterTy DABMW_SS_EstimationParameter;

/* Variable to store the SS _ estimation result*/
GLOBAL DABMW_SS_EstimationResponseParameterTy DABMW_SF_SS_EstimationResult;

/* Variable to store the SS database */
GLOBAL DABMW_SS_EstimationResutDatabaseTy DABMW_SF_SSEstimation_Database[DABMW_SF_SS_DATABASE_SIZE];

// variable to store last switch information
GLOBAL DABMW_SF_SS_LastSwitchRequestInfoTy	DABMW_SF_SS_LastSwitchRequestInfo;

/*
*********************
* FUNCTIONS SECTION
**********************
*/
/* Procedure to init the SeamLess Parameters */
GLOBAL tVoid DABMW_ServiceFollowing_SeamLessParametersInit(tVoid);

/* procedure to request the stop of seamless estimation */
GLOBAL tSInt DABMW_ServiceFollowing_SeamlessEstimationStart(tVoid);


/* procedure to request the start of seamless estimation */
GLOBAL tSInt DABMW_ServiceFollowing_SeamlessEstimationStop(tVoid);

/* procedure to handle the SS estimation Response */
GLOBAL tVoid DABMW_ServiceFollowing_SeamlessEstimationResponse (tPVoid msgPayload);

/* procedure to handle the SS Switching Response */
GLOBAL tVoid DABMW_ServiceFollowing_SeamlessSwitchingResponse (tPVoid msgPayload);

/* Procedure to init the database */
GLOBAL tVoid DABMW_ServiceFollowing_SSDatabaseInit(tVoid);

/* Procedure to reset a given info in the database 
* return the OSAL_ERROR if not found, else the index in database
*
*/
GLOBAL tSInt DABMW_ServiceFollowing_SSDatabaseReset(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI);

/* Procedure to store a new result in the database 
* can also be use to free a result : 
* if result is not success then free...
*
*/
GLOBAL tU8 DABMW_ServiceFollowing_SSDatabaseStore(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI);

/* Procedure to retrieve an existing valid result in the database 
* output :  the pointer on the response
*/
GLOBAL DABMW_SS_EstimationResutDatabaseTy* DABMW_ServiceFollowing_SSGetStoredInfoFromDatabase(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI, SF_tMSecond vI_validityDuration);

/* Procedure to check if an existing valid result in the database 
* output : bool true false 
*/
GLOBAL tBool DABMW_ServiceFollowing_SSIsStoredInfoFromDatabase(tU32 vI_DAB_Frequency, tU32 vI_DAB_Eid, tU32 vI_DAB_Sid, tU32 vI_FM_PI, SF_tMSecond vI_validityDuration);

// procedure to check if an action around seamless may be needed.
//

GLOBAL tBool DABMW_ServiceFollowing_CheckIfSeamlessActionIsNeeded(tVoid);

// procedure to check if seamless estimation or switch can be done
//

GLOBAL tBool DABMW_ServiceFollowing_CheckIfReadyToSeamlessSwitch(tVoid);


// Procedure to do a seamless switch
// retrieve the information from the database concerning the parameters...
//
GLOBAL tS32 DABMW_ServiceFollowingSeamlessSwitchWithDatabaseInfo(SD_SeamlessSwitchingSystemToSwitchTy vI_systemToSwitch);

GLOBAL tS32 DABMW_ServiceFollowingSeamlessSwitchDefault(SD_SeamlessSwitchingSystemToSwitchTy vI_systemToSwitch);

GLOBAL tVoid DABMW_ServiceFollowing_ConfigureSeamlessForOriginal(tVoid);

GLOBAL DABMW_SF_SS_LastSwitchRequestInfoTy DABMW_ServiceFollowing_RetrieveLastSwitchInfo(tVoid);

GLOBAL SD_SeamlessSwitchingSystemToSwitchTy DABMW_ServiceFollowing_RetrieveLastSwitchSystemToSwitch(tVoid);

GLOBAL tVoid DABMW_ServiceFollowing_CleanSeamlessInfo(tVoid);

GLOBAL tVoid DABMW_ServiceFollowing_SetReconfNeeded(tU32 vI_DAB_Frequency);

#undef GLOBAL	

#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_SEAMLESS_H_

// End of file

