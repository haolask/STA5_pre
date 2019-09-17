//!
//!  \file      service_following_log_status_info.h
//!  \brief     <i><b> This header file contains  functions and variable related to log and status evacuation  </b></i>
//!  \details   This header contains declarations related to service following feature
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2014.08.01
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_LOG_STATUS_INFO_H_
#define SERVICE_FOLLOWING_LOG_STATUS_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************
* DEFINE SECTION
**********************
*/
#define DABMW_SF_LOG_VERSION_REQUESTED_V1 0x00
#define DABMW_SF_LOG_VERSION_REQUESTED_V2 0x01
#define DABMW_SF_LOG_VERSION_REQUESTED_V3 0x02
#define DABMW_SF_LOG_VERSION_REQUESTED_V4 0x03



#define DABMW_SF_LOG_VERSION_V1    0x01
#define DABMW_SF_LOG_VERSION_V2    0x02
#define DABMW_SF_LOG_VERSION_V3    0x03
#define DABMW_SF_LOG_VERSION_V4    0x04
#define DABMW_SF_LOG_VERSION       DABMW_SF_LOG_VERSION_V4
#define DABMW_SF_LOG_VERSION_BG    (DABMW_SF_LOG_VERSION | 0x80)
#define DABMW_SF_MASK_LOG_VERSION       0x7F
#define DABMW_SF_MASK_LOG_VERSION_BG   0x80


#define DABMW_SF_LOG_INFO_STATUS_PERIODICITY_TIME 1000

// define the max number of AF supported for FM & DAB
//
// FM
// Max per broadcast = DABMW_AMFM_LANSCAPE_SIZE
// may have several broadcast....
// so increase it : 40 ?
#define DABMW_SF_LOG_MAX_NUM_FM_AF	30
#define DABMW_SF_LOG_FM_AF_SIZE		12 // (4+4+1+1+2)
#define DABMW_SF_LOG_DAB_AF_SIZE	8 // (4+1+1+2)
#define DABWM_SG_LOG_MIN_ROOM_DAB_AF	2


/*
*********************
* MACRO  SECTION
**********************
*/
// Usefull for loging update
#define DABMW_SF_LOG_INFO_UPDATE_EVACAUTION_NEEDED  (DABMW_serviceFollowingLogInfo.evacuationNeeded = true)
#define DABMW_SF_LOG_INFO_UPDATE_EVACAUTION_DONE    \
    { \
        DABMW_serviceFollowing_LogInfo.evacuationNeeded = false;\
        DABMW_serviceFollowing_LogInfo.lastStatusSentOut = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();\
        DABMW_serviceFollowing_LogInfo.stateChange = false;\
        DABMW_serviceFollowing_LogInfo.measurementUpdate = false;\
        DABMW_serviceFollowing_LogInfo.originalCellUpdate = false;\
        DABMW_serviceFollowing_LogInfo.alternateCellUpdate = false;\
        DABMW_serviceFollowing_LogInfo.seamlessUpdate = false;\
        DABMW_serviceFollowing_LogInfo.databaseUpdate = false;\
    } 

#define DABMW_SF_LOG_INFO_UPDATE_BG_PROC_EVACAUTION_DONE    \
    { \
        DABMW_serviceFollowing_LogInfo.lastStatusAFSentOut = DABMW_ServiceFollowing_ExtInt_ClockGetElapsedTime();\
        DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted = false;\
    } 


#define DABMW_SF_LOG_INFO_UPDATE_STATE_CHANGE       (DABMW_serviceFollowing_LogInfo.stateChange = true)
#define DABMW_SF_LOG_INFO_UPDATE_MEASUREMENT        (DABMW_serviceFollowing_LogInfo.measurementUpdate = true)
#define DABMW_SF_LOG_INFO_UPDATE_ORIGINAL_CELL      (DABMW_serviceFollowing_LogInfo.originalCellUpdate = true)
#define DABMW_SF_LOG_INFO_UPDATE_ALTERNATE_CELL    (DABMW_serviceFollowing_LogInfo.alternateCellUpdate = true)
#define DABMW_SF_LOG_INFO_UPDATE_BG_SEARCH_DONE     (DABMW_serviceFollowing_LogInfo.backgroundSearchCompleted = true)
#define DABMW_SF_LOG_INFO_UPDATE_SEAMLESS           (DABMW_serviceFollowing_LogInfo.seamlessUpdate = true)
#define DABMW_SF_LOG_INFO_UPDATE_DATABASE          (DABMW_serviceFollowing_LogInfo.databaseUpdate = true)



/*******************
* usefull BUFFER MGT macros
******************
*/
/*
#define EXTRACT_U32(_b_) (((((tU32)(_b_)[0]) << 24) & 0xFF000000) | ((((tU32)(_b_)[1]) << 16) & 0x00FF0000) | ((((tU32)(_b_)[2]) << 8) & 0x0000FF00) | ((((tU32)(_b_)[3]) << 0) & 0x000000FF))
#define EXTRACT_U16(_b_) (((((tU16)(_b_)[0]) << 8) & 0x0000FF00) | ((((tU16)(_b_)[1]) << 0) & 0x000000FF))
#define EXTRACT_U8(_b_)  ((((tU8)(_b_)[0]) << 0) & 0xFF)
*/
#define EXTRACT_U32(_b_) ((((_b_)[0] << 24) & 0xFF000000) | (((_b_)[1] << 16) & 0x00FF0000) | (((_b_)[2] << 8) & 0x0000FF00) | (((_b_)[3] << 0) & 0x000000FF))
#define EXTRACT_U16(_b_) ((((_b_)[0] << 8) & 0x0000FF00) | (((_b_)[1] << 0) & 0x000000FF))
#define EXTRACT_U8(_b_)  (((_b_)[0] << 0) & 0xFF) 


/* get an U8 from an U32 
* U32 is byte3, byte2, byte 1, byte 0
* get byte 3, 2, 1 or 0
*/ 
#define EXTRACT_FROM_U32_BYTE(_b_, byte_num) ((tU8) (((_b_) >> (8*byte_num)) & 0xFF))
/* get an U8 from an U16 
* U16 is byte 1, byte 0
* get byte 1 or 0
*/ 
#define EXTRACT_FROM_U16_BYTE(_b_, byte_num) ((tU8) (((_b_) >> (8*byte_num)) & 0xFF))


/* get an U8 from an U32 
  * U32 is byte3, byte2, byte 1, byte 0
  * get byte 3, 2, 1 or 0
    */ 
#define PUT_U32_TO_U8_ARRAY(b, ptr) \
        { *ptr = EXTRACT_FROM_U32_BYTE(b,3); \
          *(ptr+1) = EXTRACT_FROM_U32_BYTE(b,2); \
          *(ptr+2) = EXTRACT_FROM_U32_BYTE(b,1); \
          *(ptr+3) = EXTRACT_FROM_U32_BYTE(b,0); \
        }
    
    /* get an U8 from an U16 
    * U16 is byte 1, byte 0
    * get byte 1 or 0
    */ 
#define PUT_U16_TO_U8_ARRAY(b, ptr) \
            { *ptr = EXTRACT_FROM_U16_BYTE(b,1); \
              *(ptr+1) = EXTRACT_FROM_U16_BYTE(b,0); \
            }

/* get an U8 from an U32 
  * U32 is byte3, byte2, byte 1, byte 0
  * get byte 3, 2, 1 or 0
    */ 

#define PUT_U32_TO_U8_ARRAY_WITH_INDEX(b, ptr, index) \
        { PUT_U32_TO_U8_ARRAY(b, (ptr+index));\
          index+=4;\
        }
    
    /* get an U8 from an U16 
    * U16 is byte 1, byte 0
    * get byte 1 or 0
    */ 
#define PUT_U16_TO_U8_ARRAY_WITH_INDEX(b, ptr, index) \
        {  PUT_U16_TO_U8_ARRAY(b, (ptr+index));\
           index+=2;\
        }
              

    /* get an U8 from an U16 
    * U16 is byte 1, byte 0
    * get byte 1 or 0
    */ 
#define PUT_U8_TO_U8_ARRAY_WITH_INDEX(b, ptr, index) \
        {  *(ptr+index)=b;\
           index+=1;\
        }


/*
*********************
* ENUM  SECTION
**********************
*/

/*
*********************
* STRUCTURE  SECTION
**********************
*/
typedef union
{
    struct 
    {
        tU8 AFCheckRequested:1;
        tU8 AFCheckdone:1;
    	tU8	backgroundCheckRequested:1; 
        tU8 backgroundCheckDone:1;
    	tU8	backgroundScanRequested:1; 
    	tU8 backgroundScanDone:1;
        tU8 filler_1:2;
    } field;

    tU8 value;
    
} DABMW_serviceFollowingLogInfoBgSearchProcTy;

typedef union
{
    struct 
    {
          tU32  stateChange:1;
          tU32  measurementUpdate:1;
          tU32  originalCellUpdate:1;
          tU32  alternateCellUpdate:1;
          tU32  backgroundSearchCompleted:1;
          tU32  seamlessUpdate:1;
          tU32  databaseUpdate:1;
          tU32  filler_1:25;
    } field;

    tU32 value;
    
} DABMW_serviceFollowingLogChangeFieldInfoTy;


// STATUS LOGGING/EVACUATION PART 
typedef struct
{
    // Store the system time when last inform done
    SF_tMSecond lastStatusSentOut;
    SF_tMSecond lastStatusAFSentOut;

    // Variable to notify evacuation is need : info change
    tBool evacuationNeeded;
    tBool stateChange;
    tBool measurementUpdate;
    tBool originalCellUpdate;
    tBool alternateCellUpdate;
    tBool backgroundSearchCompleted;
    tBool seamlessUpdate;
    tBool databaseUpdate;  
    
} DABMW_serviceFollowingLogInfoTy;


// Cell Information
typedef struct
{
    // Original data source (user tuned one)
    DABMW_SF_mwAppTy app;
    tU32 Frequency;
    tU32 Sid;
    tU32 Eid;
    DABMW_SF_QualityTy Quality;
    /* quality status */
    DABMW_SF_QualityStatusTy qualityStatus;
    // source present flag
    DABMW_SF_FreqStoringFlagTy source;
    
} DABMW_serviceFollowingLogInfoCellTy;

// Seamless  Information between 2 cells
typedef enum
{
    DABMW_SF_LOG_INFO_SEAMLESS_NOT_ACTIVATED  			= 0, // case = SS not activated,
    DABMW_SF_LOG_INFO_SEAMLESS_NOT_APPLICABLE 			= 1, // or FM-FM & DAB-DAB configuration between alternate & original
    DABMW_SF_LOG_INFO_SEAMLESS_NO_STORED_INFO 			= 2, // SS activated, nothing in DB yet
    DABMW_SF_LOG_INFO_SEAMLESS_STORED_INFO    			= 3, // SS activated, nothing in DB yet       
    DABMW_SF_LOG_INFO_SEAMLESS_EVALUATION_ON_GOING	  	= 4, // SS estimation on going.
}DABMW_SF_LogInfoSeamlessInformationStatusTy;

typedef enum
{
    SF_SEAMLESS_PROVIDER_IS_DAB,
    SF_SEAMLESS_PROVIDER_IS_FM,
    SF_SEAMLESS_PROVIDER_IS_NONE
} DABMW_SF_LogInfoSeamlessInformationProviderTy;

typedef enum
{
    SF_SEAMLESS_SWITCHING_SWITCH_TO_DAB,
    SF_SEAMLESS_SWITCHING_SWITCH_TO_FM,
    SF_SEAMLESS_SWITCHING_SWITCH_TO_EARLY_FM
} DABMW_SF_LogInfoSeamlessSwitchingSystemToSwitchTy;


typedef enum
{
    SF_SEAMLESS_SWITCHING_STATUS_SUCCESSFUL					    		= 0x00,
    SF_SEAMLESS_SWITCHING_STATUS_SUCCESSFUL_WITH_DEFAULT_VALUE		    = 0x01,
    SF_SEAMLESS_SWITCHING_STATUS_SUCCESSFUL_APPROXIMATED			  	= 0x02,
    SF_SEAMLESS_SWITCHING_STATUS_FAILURE					   			= 0x03,
    SF_SEAMLESS_SWITCHING_STATUS_NO_SWITCH_STORED						= 0xFF,
} DABMW_SF_LogInfoSeamlessSwitching_SwitchingStatusTy;


typedef struct
{
	tS32 delay_estimate;									// Delay estimate in samples to be converted in seconds
	tU32 average_RMS2_FAS;									// Average of squared RMS on FAS (DAB Loudness)
	tU32 average_RMS2_SAS;									// Average of squared RMS on SAS (FM Loudness)
	tU32 confidence_level;									// Confidence level of delay estimate
	SF_tMSecond lastStoredTime; 							// when it has been stored
	DABMW_SF_LogInfoSeamlessInformationProviderTy			provider_type;
}DABMW_serviceFollowingLogInfoSeamlessDataBaseTy;


typedef struct
{
	tS32 delay_estimate;									// Delay estimate in samples to be converted in seconds
	tU32 average_RMS2_FAS;									// Average of squared RMS on FAS (DAB Loudness)
	tU32 average_RMS2_SAS;									// Average of squared RMS on SAS (FM Loudness)
	SF_tMSecond lastSwitchTime; 							// when it has been stored
	DABMW_SF_LogInfoSeamlessSwitchingSystemToSwitchTy		systemToSwitch;
	DABMW_SF_LogInfoSeamlessSwitching_SwitchingStatusTy 	status;
}DABMW_serviceFollowingLogInfoSeamlessLastSwitchInfoTy;


typedef struct
{
    DABMW_SF_LogInfoSeamlessInformationStatusTy 	seamlessInformationStatus;                          // flag to indicate if measures below are 
   	DABMW_serviceFollowingLogInfoSeamlessDataBaseTy databaseInfo;
	DABMW_serviceFollowingLogInfoSeamlessLastSwitchInfoTy lastSwitchInfo;
} DABMW_serviceFollowingLogInfoSeamlessTy;


// Bg Information
typedef struct
{
    // Last bg search reason
    DABMW_SF_TypeOfScanTy dab_TypeOfScan;
    DABMW_SF_TypeOfScanTy fm_TypeOfScan;

    // last bg search result
    tBool                 last_searchSuccessfull;
    DABMW_serviceFollowingLogInfoBgSearchProcTy last_searchProcInfo;

    // AF/BG checked frequency & status result
    DABMW_FreqCheckListEntryTy DABMW_SF_freqCheckList;
 
    // Found frequency
    // TODO
    //
    
    
} DABMW_serviceFollowingBGProcessingInfoTy;


// Structure for Logging evacuation
//
typedef struct
{
    // Version Information for compatibility
    //
    tU8 sf_log_version;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // evacuation flags info
    //////////////////////////////////////////////////////////////////////////////////////////////////
    DABMW_serviceFollowingLogChangeFieldInfoTy changeFieldInfo;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Generic INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    tU32  searchSid_Pi;
    DABMW_serviceFollowingStateMachineTy state;
    tU8 generic_filler_1;
    tU8 generic_filler_2;
    tU8 generic_filler_3;
    
    // timing data for system switch
    //
    SF_tMSecond deltaTimeSinceLastSwitch;
	SF_tMSecond deltaTimeSinceLastServiceRecoverySearch;
	SF_tMSecond idle_timerStart;
	SF_tMSecond deltaTimeSinceLastSearchForAF;
    SF_tMSecond deltaTimeSinceLastFullScan;
	SF_tMSecond deltaTimeSinceLastLandscaspeBuilding;
	SF_tMSecond deltaTimeSinceLastLandscapeDelayEstimation;  
    SF_tMSecond deltaTimeFromAlternateSwitch;
           
    //////////////////////////////////////////////////////////////////////////////////////////////////
    // CURRENT CELL INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    DABMW_serviceFollowingLogInfoCellTy originalCell;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // Alternate CELL INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////
    DABMW_serviceFollowingLogInfoCellTy alternateCell;
    DABMW_serviceFollowingLogInfoSeamlessTy seamlessInfo;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // BG PROCESSING INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////  
    DABMW_serviceFollowingBGProcessingInfoTy bgProcessingInformation;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    // SEAMLESS PROCESSING INFORMATION 
    //////////////////////////////////////////////////////////////////////////////////////////////////  
    // to be added : seamless related info 
    //
    
} DABMW_serviceFollowing_LogInfoMsgTy;


/*
*********************
* VARIABLE SECTION
**********************
*/
/* variables are belonging to SERVICE_FOLLOWING_C
*/
#ifndef SERVICE_FOLLOWING_MAINLOOP_C
#define GLOBAL	extern
#else
#define GLOBAL
#endif

GLOBAL DABMW_serviceFollowingLogInfoTy DABMW_serviceFollowing_LogInfo;

/*
*********************
* FUNCTIONS SECTION
**********************
*/

GLOBAL tVoid DABMW_ServiceFollowing_InitLogInfo(tVoid);

// Procedure specific to log the BgProcessing Information : AF...
//
GLOBAL tU16 DABMW_ServiceFollowing_BuildLogPayloadMsgBgProcessing(tPU8 pO_payloadBuffer, tU16 vI_bufferLen, tBool vI_isAuto, tU8 vI_version, DABMW_serviceFollowing_LogInfoMsgTy *pI_logInfo);


GLOBAL DABMW_serviceFollowingLogInfoSeamlessTy DABMW_ServiceFollowing_SetSeamlessLogInfoStatus(tVoid);


#undef GLOBAL

#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_INTERNAL_H_

// End of file

