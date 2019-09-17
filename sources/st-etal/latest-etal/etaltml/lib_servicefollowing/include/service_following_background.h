//!
//!  \file      service_following_background.h
//!  \brief     <i><b> This header file contains internal functions and variable for background task  of service following c files  </b></i>
//!  \details   This header contains declarations related to service following feature
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_BACKGROUND_H_
#define SERVICE_FOLLOWING_BACKGROUND_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************
* DEFINE SECTION
**********************
*/
/* MAX bands for background scan = 3
* BAND_III
* BAND_L
* BAND_FM
*/
#define DABMW_SF_BG_MAX_BAND_TO_SCAN	3
#define DABMW_SF_NB_DAB_BAND_TO_SCAN 	2




// DISS values for fast PI
// mode 0 = auto
// mode 1 = sleep
//
// FILTER = set to 4 may be better for RDS detection in FAST PI mode
// 0x03 =  60 Khz : normal one
// 0x04 =  80 Khz = FAST PI

#define DABMW_SF_DISS_MODE_FAST_PI		0x01
#define DABMW_SF_DISS_FILTER_FAST_PI	0x04

#define DABMW_SF_DISS_MODE_NORMAL_PI	0x00
#define DABMW_SF_DISS_FILTER_NORMAL_PI	0x03




/* define the PI validity time (in ms) prior to consider a need for PI check
* let's say : 5 minutes
*/
#define DABMW_SF_PI_VALIDITY_TIME_MS    300000

/* Tune parameter */
/* keep decoding while search */
#define SF_BACKGROUND_TUNE_KEEP_DECODING	0 
/* auto mode */
#define SF_BACKGROUND_TUNE_INJECTION_SIDE	0 
/* no switch while in search */
#define SF_BACKGROUND_TUNE_NO_OUTPUT_SWITCH	true

/* FIG 02 MAX NB TO WAIT */
/* fig02 contain the service list 
* it is broadcast in several fig02
* we define the max here
*/
// Code ER : #300437 : sometime, Some Sid are not seen... 
// increment to 15 at least : assuming we have usually up to 15 services... + repetitions
//
#define SF_NB_MAX_FIG02_TO_WAIT        15


// specific data for band step
//band step of 100 kHZ minimum
#define DABMW_SF_MIN_FREQ_BAND_STEP     100

/*
*********************
* STRUCTURE SECTION
**********************
*/
	
/* -----------
* GLOBAL  part : store list of Frequency to be checked 
* 
* -----------
*/

typedef enum
{
	DABMW_SF_BG_RESULT_NOT_FOUND						= 0,
	DABMW_SF_BG_RESULT_FOUND_PI_CONFIRMED 				= 1,
	DABMW_SF_BG_RESULT_FOUND_PI_NOT_CONFIRMED 			= 2,
	DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_CONFIRMED		= 3,
	DABMW_SF_BG_RESULT_FOUND_ALTERNATE_PI_NOT_CONFIRMED = 4,
	DABMW_SF_BG_RESULT_FOUND_INITIAL_PI_CONFIRMED       = 5,
	DABMW_SF_BG_RESULT_FOUND_INITIAL_PI_NOT_CONFIRMED   = 6
} DABMW_SF_BG_SearchedResultTy;


typedef enum
{
	DABMW_SF_BG_SID_NOT_EQUAL							= 0,
	DABMW_SF_BG_SID_EQUAL								= 1,
	DABMW_SF_BG_SID_REGIONAL_VARIANT			 		= 2,
	DABMW_SF_BG_SID_EQUIVALENT_HARD_LINK				= 3,
	DABMW_SF_BG_SID_EQUIVALENT_HARD_LINK_REGIONAL 		= 4,
	DABMW_SF_BG_SID_EQUIVALENT_SOFT_LINK				= 5,
	DABMW_SF_BG_SID_EQUIVALENT_SOFT_LINK_REGIONAL 		= 6,
	DABMW_SF_BG_SID_EQUAL_INITIAL_SEARCHED              = 7,
	DABMW_SF_BG_SID_EQUAL_INITIAL_SEARCHED_REGIONAL     = 8
} DABMW_SF_BG_SidPi_MatchingResultTy;



/* Linking information storage */
typedef struct
{	
	tU32 id;
	tU32 eid;
	tU32 frequency;
    DABMW_idTy kindOfId;
	tBool IdIsInLandscape;
	tBool IsHardLink;
} DABMW_SF_BackgroundLinkInfo;


typedef struct
{
	/* frequency range */
	
	tU32	startFreq;
	tU32	stopFreq;
	tU32	bandStep;
	tU32	bg_Freq;
    tBool 	fig00_read;
	tBool 	fig01_read;
    tBool 	fig02_read;
	tU8	    fig02_readCounter;
    tBool   seek_ongoing;
    DABMW_SF_mwAppTy seek_app;
	tU32	seek_frequency;
    tBool   VPA_IsOnAtStart;
    tBool   VPA_HasBeenBroken;
    tBool   AF_proc_ongoing;
    DABMW_SF_mwAppTy afproc_app;
		
// RDS enabled information 
	tBool IsRdsEnabled_app_1;
	tBool IsRdsEnabled_app_2;
	tBool IsFastPiEnabled_app_1;
	tBool IsFastPiEnabled_app_2;
		
	/* store the current targetted Searched Service
	* NOTE THAT : It should be set to INVALID if a full scan for 'landscape' is requested.
	*/
	tU32 	searchedServiceID;
	DABMW_SF_mwAppTy bgScan_App;
	// Add-on ETAL
	// Addition for ETAL 
	// 
	DABMW_SF_mwEtalHandleTy bgScan_Handle;
	DABMW_SF_mwEtalHandleTy bgScan_HandleDatapth;
	
	DABMW_SF_BG_SearchedResultTy	succesfulSearched;

	tBool	AFCheckRequested;
    tBool   AFCheckdone;
	tBool	backgroundCheckRequested; 
    tBool   backgroundCheckDone;
	tBool	backgroundScanRequested; 
    tBool	backgroundScanDone; 
	tBool 	backgroundScanSkipOriginalFreq;
	
	tU8 	numBearerToScan;
	tU8		currentScanBandIndex;
	DABMW_SF_systemBandsTy 	requestedBandToScan[DABMW_SF_BG_MAX_BAND_TO_SCAN];

	/* Link ID list used in bg*/
	DABMW_SF_BackgroundLinkInfo *p_LinkIdList_DAB;
	tU8	nb_LinkId_DAB;
	DABMW_SF_BackgroundLinkInfo *p_LinkIdList_FM;
	tU8	nb_LinkId_FM;
	
	/* store in case needed, the linkage info from database */
	DABMW_BrDbLinkageSetTy *p_DAB_linkageSetPtr;
	tU32 DAB_linkageSet_size;
	DABMW_BrDbLinkageSetTy *p_FM_linkageSetPtr;
	tU32 FM_linkageSet_size;

    // save some information for background scan suspend/resume
    tU32 lastBgFreq;
    DABMW_SF_mwAppTy lastBgScanApp;
    
} DABMW_SF_BackgroundScanInfoTy;

typedef struct
{

/* Set of parameter for the audio Switch setting */
	
	tBool	keepDecoding;
	tU8		injectionSide;
	tBool	noOutputSwitch;

} DABMW_SF_BackgroundAudioInfoTy;
  

/*
*********************
* VARIABLE SECTION
**********************
*/

/* variables are belonging to SERVICE_FOLLOWING_BACKGROUND_C
*/
#ifndef SERVICE_FOLLOWING_BACKGROUND_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

#ifdef SERVICE_FOLLOWING_BACKGROUND_C 

/* CONST
*/
GLOBAL const DABMW_SF_systemBandsTy DabMw_SF_Bg_DabbandToScan[DABMW_SF_NB_DAB_BAND_TO_SCAN] = {DABMW_BAND_DAB_III, DABMW_BAND_DAB_L};

GLOBAL DABMW_SF_BackgroundScanInfoTy 		DABMW_SF_BackgroundScanInfo;
GLOBAL DABMW_SF_BackgroundAudioInfoTy		DABMW_SF_BackgroundAudioInfo;

#endif 

#if defined(SERVICE_FOLLOWING_BACKGROUND_C) || defined(SERVICE_FOLLOWING_EXTERNAL_INTERFACE_DABMW_SERVICES_C)

GLOBAL DABMW_SF_BackgroundScanInfoTy 		DABMW_SF_BackgroundScanInfo;
#endif 



#undef GLOBAL

/*
*********************
* FUNCTIONS SECTION
**********************
*/
#ifndef SERVICE_FOLLOWING_BACKGROUND_C
#define GLOBAL	extern
#else
#define GLOBAL	
#endif

/*
* Procedure to init the number background scan procedure : setting the band order to scan.
* returns the number of band to scan
* 
*/
GLOBAL tU8 DABMW_ServiceFollowing_Background_ScanInit(tU32 vI_SearchedServiceID, DABMW_SF_systemBandsTy vI_band1, DABMW_SF_systemBandsTy vI_band2,DABMW_SF_systemBandsTy vI_band3);

/*
* Procedure to continue  the background scan procedure to next frequency within the set bands: setting the band order to scan.
* returns the next band to scan information, if NONE = no more band requested in background scan.
* 
*/


GLOBAL tU32 DABMW_ServiceFollowing_Background_NextScan(tVoid);

/* Procedure to retrieve current scan band
*/

GLOBAL DABMW_SF_systemBandsTy DABMW_ServiceFollowing_Background_GetCurrentScanBand(tVoid);

/* Procedure to retrieve next band to scan 
*/
GLOBAL DABMW_SF_systemBandsTy DABMW_ServiceFollowing_Background_GetNextScanBand(tVoid);


/* Return Current selected freq for BG scan
*/
GLOBAL tU32 DABMW_ServiceFollowing_Background_GetCurrentScanFreq(tVoid);


/* Procedure to select the right app & tuner to to the procedure depending on current band/techno, and free tuner
*/

GLOBAL DABMW_SF_mwAppTy DABMW_ServiceFollowing_Background_SelectApp(DABMW_SF_systemBandsTy vI_band);

/* Procedure to select the right app & tuner to to the procedure depending on current band/techno, and free tuner
*/

GLOBAL DABMW_SF_mwAppTy DABMW_ServiceFollowing_AFCheck_SelectApp(DABMW_SF_systemBandsTy vI_band);

/* Procedure handling the main algorithm for background check/scan
*/
GLOBAL tSInt DABMW_ServiceFollowing_BackgroundMainLoop(tVoid);

/* Procedure to continue the background scan, in charge of tuning to next frequency 
* 
*/
GLOBAL tSInt DABMW_ServiceFollowing_BackgroundScan(tVoid);

/* Procedure to act while on PI awaiting state
* wake-up status in boolean parameter
* TRUE = PI received (event awake..)
* FALSE = Timeout !
*/
GLOBAL tSInt DABMW_ServiceFollowing_PI_processing(tBool vI_PIreceptionStatus);

/* Procedure to act while on PS awaiting state
* wake-up status in boolean parameter
* TRUE = PS received (event awake..)
* FALSE = Timeout !
*/
GLOBAL tSInt DABMW_ServiceFollowing_PS_processing(tBool vI_PSreceptionStatus);


/* Procedure to act while on DAB tune awaiting state
* wake-up status in boolean parameter
* TRUE = TUNE received (event awake..)
* FALSE = Timeout !
*/
GLOBAL tSInt DABMW_ServiceFollowing_DAB_processing(tBool vI_TuneStatus);

/* Procedure to act while on FIC Reading tune awaiting state
* wake-up status in boolean parameter
* TRUE = FIC reading received (event awake..)
* FALSE = Timeout !
*/
GLOBAL tSInt DABMW_ServiceFollowing_FIC_ReadyProcessing(tBool vI_FIC_Ready_Status);
	
/* 
* Start Service Selection Request
*/

//GLOBAL tSInt DABMW_ServiceFollowing_ServiceIDSelection(DABMW_SF_mwAppTy vI_app, tU32 vI_SearchedServiceID, tBool vI_keepDecoding,tU8 vI_injectionSide, tBool vI_noOutputSwitch, tBool vI_isInternalTune);


/* Callback notification function for PI  case
*/
GLOBAL tVoid DABMW_ServiceFollowing_OnPI_Callback (tU32 vI_pi, tU32 vI_freq, tPVoid p_paramPtr);

/* Callback notification function for DAB Tune case
*/
GLOBAL tVoid DABMW_SF_DabAfCheckCallbackNotification(DABMW_SF_mwAppTy vI_application, tU32 vI_frequency, tPVoid pIO_paramPtr, DABMW_dabStatusTy *pI_notificationStatus);


/* Procedure which prepare the Alternate Freq in FM for a given PI
* returns
* --> OSAL_ERROR if no frequency found
* -->  
* else return number of frequency added to AF list
*/

GLOBAL tSInt DABMW_ServiceFollowing_BuildAlternateFrequency_FM(tU32 vI_PiAF, tU32 vl_freqToRemove1, tU32 vl_freqToRemove2);

/* Procedure which set the source of alternate
*/

GLOBAL tSInt DABMW_ServiceFollowing_SetAlternateFrequency_SourceInfo(tVoid);

/* Procedure which set the source of original
*/

GLOBAL tSInt DABMW_ServiceFollowing_SetOriginalFrequency_SourceInfo(tVoid);


/* Procedure which prepare the Alternate Freq in FM for a given PI
* returns
* --> OSAL_ERROR if no frequency found
* -->  
* else return number of frequency added to AF list
*/

GLOBAL tSInt DABMW_ServiceFollowing_BuildAlternateFrequency_DAB(tU32 vI_Sid, tU32 vl_freqToRemove1, tU32 vl_freqToRemove2);

/* procedure to init the Global list of alternate Frequency
* specify if init is required for FM and/or DAB
*/
GLOBAL tVoid DABMW_ServiceFollowing_InitAlternateFrequency(tBool vI_initFM, tBool vI_initDAB);


/* Procedure which provides the next frequency to check
* based on the Freq list
* Order = DAB First then FM.
*/
GLOBAL tU32 DABMW_ServiceFollowing_Background_NextCheck(tVoid);

/* Procedure to continue the background scan, in charge of tuning to next frequency 
*/
GLOBAL tSInt DABMW_ServiceFollowing_BackgroundCheck(tVoid);

/* Procedure to check the AF quality and return if quality is acceptable
*/
GLOBAL tBool DABMW_ServiceFollowing_StoredAndEvaluate_AF_FMQuality(tVoid);

// Measure current candidate on same path than alternate (if alternate is FM)
//
GLOBAL tSInt DABMW_ServiceFollowing_Measure_AF_FMQuality_OnAlternatePath(tVoid);

/* Procedure to check the AF on AF check quality and return if quality is acceptable
*/
GLOBAL tBool DABMW_ServiceFollowing_StoredAndEvaluate_AFCheck_AF_FMQuality(tVoid);


/* Call back to get notified FIG 0/1  */
GLOBAL tVoid DABMW_ServiceFollowing_OnFIG00_Callback (tU8 vI_fig, tU8 vI_ext, tU32 vI_freq, tU8 vI_ecc, tU16 vI_id, tPVoid pI_params);

/* FM   AF Check request
*/
GLOBAL tSInt DABMW_SF_AmFmAFCheckRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality);

/* FM   AF Check request
*/
GLOBAL tSInt DABMW_SF_AmFmAFStartRequest(DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, DABMW_SF_amfmQualityTy *pO_quality);

/* FM   AF END request
*/

GLOBAL tSInt DABMW_SF_AmFmAFEndRequest(tVoid);

/* Procedure which provides the next frequency to check
* based on the Freq list
* Order = DAB First then FM.
*/
GLOBAL tU32 DABMW_ServiceFollowing_AFcheck_NextCheck(tVoid);


/* Procedure to continue the background scan, in charge of tuning to next frequency 
* 
*/
GLOBAL tSInt DABMW_ServiceFollowing_AFCheck(tVoid);


/* AUTO SEEK CALL BACK 
*/
GLOBAL tVoid DABMW_SF_AmFmLearnFrequencyAutoSeekCallback (DABMW_SF_mwAppTy vI_app, tU32 vI_frequency, tPVoid p_I_param);


/* Procedure to End the Seek */
GLOBAL tSInt DABMW_SF_AutoSeekEnd (tVoid);

// procedure to check if Bg AF is on going.
GLOBAL tBool DABMW_ServiceFollowing_IsBackgroundAFCheckOnGoing(tVoid);


GLOBAL tSInt DABMW_SF_AmFmLearnFrequencyAutoSeekImmediate (DABMW_SF_mwAppTy app, tS16 deltaFreq);

/* AUTO SEEK RESULT PROCESSING */
GLOBAL tSInt DABMW_ServiceFollowing_Seek_processing(tBool vI_PIreceptionStatus);


/* Service Recovery init */
GLOBAL tSInt DABMW_ServiceFollowing_ServiceRecoveryScan(tU32 vI_SearchedServiceID);

/* Last search result 
*/
GLOBAL tBool DABMW_ServiceFollowing_LastBackgroundSearchResult(tU32 vI_SearchedServiceID);

/* Procedure to request a background service AF scan
* input parameter = 
* target searched service + choice of bearer. (DAB / FM)
*/
GLOBAL tSInt DABMW_ServiceFollowing_AFScan(tU32 vI_SearchedServiceID, tBool vI_initDAB, tBool vI_initFM, tBool vI_fullScanDAB, tBool VI_fullScanFM);


/* Procedure to request a background search for full landscape scanning
* Bearer to scan is computed autonomously
* the procedure trigger is : searched service = FFFF
*/
GLOBAL tSInt DABMW_ServiceFollowing_LandscapeScan(tVoid);


/* procedure to compare Sid versus requested one
* either (implicit link) the Sid match directly 
* either look for equivalent SID : ie matching the Hard link or Soft Link
*/

GLOBAL DABMW_SF_BG_SidPi_MatchingResultTy DABMW_ServiceFollowing_CompareSidPI(tU32 vI_refSid, tU32 vI_SidToCompare, tBool bearerIsDab);

/* procedure to set search success depending on SID / PI matching
*/
GLOBAL DABMW_SF_BG_SearchedResultTy DABMW_ServiceFollowing_GetSearchResult(tU32 vI_Sid, tBool bearerIsDab, tBool ID_received);

/* Retrieve the DAB_III band to use depending on country set
*/
GLOBAL DABMW_SF_systemBandsTy DABMW_ServiceFollowingGetDABBandIII(tVoid);

/* Retrieve the DAB_L band to use depending on country set
*/
GLOBAL DABMW_SF_systemBandsTy DABMW_ServiceFollowingGetDABBandL(tVoid);

/* Retrieve the FM band to use depending on country set
*/
GLOBAL DABMW_SF_systemBandsTy DABMW_ServiceFollowingGetFMBand(tVoid);

// retrieve band step
GLOBAL tU32 DABMW_ServiceFollowing_GetBandInfoTable_step (DABMW_SF_systemBandsTy vI_bandValue, DABMW_SF_mwCountryTy vI_countryId);


/* Built and store the hard linking information on a given Sid
* should be the searched one
* this is based on original stored one.
* DAB only...
* FM : reuse the stored one...
*/ 
GLOBAL tSInt DABMW_ServiceFollowing_BuildLinkIdList(DABMW_SF_mwAppTy vI_app, tU32 vI_ServiceId, tBool vI_dabRequested, tBool vI_fmRequested);

/* Update the service linking information 
* set the field for Id Linking info
*/
GLOBAL tVoid DABMW_ServiceFollowing_LinkInfo_SetIdPresentInLandscape(tU32 vI_Sid, tU32 vI_freq, tBool vI_bearerIsDab);

/* Get the frequency associated to a service ID
*/
GLOBAL tU32 DABMW_ServiceFollowing_LinkInfo_GetFrequencyFromId(tU32 vI_Sid, tBool serviceLinking_isDab);


/* Init Service Linking infos
*/
GLOBAL tVoid DABMW_ServiceFollowing_LinkInfo_Init(tVoid);

/* procedure to release any Link Info
*/

GLOBAL tVoid DABMW_ServiceFollowing_LinkInfo_Release(tVoid);

// procedure to restart the background scan
GLOBAL tSInt DABMW_ServiceFollowing_RestartBackgroundScan(tVoid);

// procedure to control the VPA : on/off
GLOBAL tVoid DABMW_ServiceFollowing_AmFMStopVpa(DABMW_SF_mwAppTy vI_app);

GLOBAL tVoid DABMW_ServiceFollowing_AmFMRestartVpa(tVoid);

/* procedure to End and set back field before leaving baackground scan()
*/
GLOBAL tVoid DABMW_ServiceFollowing_EndBackgroundScan(tVoid);

// procedure for suspend/resume the RDS during seek...
GLOBAL tSInt DABMW_ServiceFollowing_ResumeRds(DABMW_SF_mwAppTy app);

GLOBAL tSInt DABMW_ServiceFollowing_SuspendRds(DABMW_SF_mwAppTy app);

GLOBAL tBool DABMW_ServiceFollowing_IsLanscapeScanOnGoing(tVoid);

GLOBAL tVoid DABMW_ServiceFollowing_OnPS_Callback (tU32 vI_pi, tU32 vI_freq, tPVoid p_paramPtr);

// procedure to access log info from Bg task : 
// only from main loop
#if defined (SERVICE_FOLLOWING_BACKGROUND_C) || defined (SERVICE_FOLLOWING_MAINLOOP_C)
GLOBAL DABMW_serviceFollowingLogInfoBgSearchProcTy DABMW_ServiceFollowing_BackgroundLastSearchLogInfo(tVoid);
#endif

//
// Procdure which return the path to be used for Alternate measurements...
// if background scan thru auto-seek in FM is on-going... this cannot be interupted...
// it required 
//
GLOBAL tBool DABMW_ServiceFollowing_SuspendBgForAlternateFMMeasurement(DABMW_SF_mwAppTy vI_app);

GLOBAL  tVoid DABMW_ServiceFollowing_ResumeBgAfterAlternateFMMeasurement(tVoid);

GLOBAL tSInt DABMW_ServiceFollowing_StopBgScan(tVoid);

#undef GLOBAL


#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_INTERNAL_H_

// End of file

