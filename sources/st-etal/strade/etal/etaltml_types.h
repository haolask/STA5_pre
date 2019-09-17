//!
//!  \file 		etaltml_types.h
//!  \brief 	<i><b> ETAL API </b></i>
//!  \details   Type definitions for the ETAL user application, extended functionalities (TML)
//!  \author 	Raffaele Belardi
//!

/*
 * DO NOT INCLUDE THIS FILE DIRECTLY!
 *
 * It is included by etaltml_api.h
 */

#ifndef ETALTML_TYPES_H_
#define ETALTML_TYPES_H_

/***********************************
 *
 * Defines
 *
 **********************************/
#if defined (CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER)
    #define ETAL_DEF_MAX_FM_LANDSCAPE_SIZE                    (tU16)CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER
#else
    #define ETAL_DEF_MAX_FM_LANDSCAPE_SIZE                    (tU8)24
#endif


/***********************************
 *
 * RDS
 *
 **********************************/
/* from DABMW spec */
#define ETAL_DEF_MAX_PS_LEN       8
#define ETAL_DEF_MAX_RT_LEN      64
/* from DABMW implementation, DABMW_AF_LIST_BFR_LEN */
#define ETAL_DEF_MAX_AFLIST      26

#define NB_OF_THRESHOLDS        16


/***********************************
 *
 * Events
 *
 **********************************/
/*
 * EtalLearnStatus
 */
typedef struct
{
	ETAL_HANDLE             m_receiverHandle;
	tU32                    m_services;
} EtalLearnStatus;

typedef struct
{
    tS32                        m_fieldStrength;
    tU32                        m_frequency;
} EtalLearnFrequencyListTy;

/*
 * Learn max number of reported frequencies
 */
#define ETAL_LEARN_MAX_NB_FREQ      30

/*
 * etalLearnEventStatusTy
 */
typedef enum
{
    ETAL_LEARN_STARTED,
    ETAL_LEARN_RESULT,
    ETAL_LEARN_FINISHED,
    ETAL_LEARN_ERROR
} etalLearnEventStatusTy;

/*
 * EtalLearnFrequencyTy
 */
typedef struct
{
    tS32                        m_fieldStrength;
    tU32                        m_frequency;
    tBool						m_HDServiceFound;
    tU32                        m_ChannelID;
} EtalLearnFrequencyTy;

/*
 * EtalLearnStatusTy
 */
typedef struct
{
    ETAL_HANDLE                 m_receiverHandle;
    etalLearnEventStatusTy      m_status;
    tU32                        m_frequency;
    tU32                        m_nbOfFrequency;
    EtalLearnFrequencyTy*       m_frequencyList;
} EtalLearnStatusTy;

/*
 * etalScanEventStatusTy
 */
typedef enum
{
    ETAL_SCAN_STARTED,
    ETAL_SCAN_RESULT,
    ETAL_SCAN_FINISHED,
    ETAL_SCAN_ERROR
} etalScanEventStatusTy;

/*
 * EtalScanStatusTy
 */
typedef struct
{
    ETAL_HANDLE                 m_receiverHandle;
    etalScanEventStatusTy       m_status;
    tU32                        m_frequency;
    tBool                       m_frequencyFound;
    tBool                       m_fullCycleReached;
} EtalScanStatusTy;

typedef struct
{
	ETAL_HANDLE              m_receiverHandle;
	tBool                    m_IsFoundstatus;
	tBool					 m_freqIsDab;
	tU32                     m_freq;
	tU32					 m_SidPi;
	tU32					 m_Ueid;
	tBool 					 m_AFisAvailable;
	tBool					 m_ssApplicableOnAF;
	tBool					 m_AFisSync;
} EtalTuneServiceIdStatus;


// Landscape Retrieval information 
typedef struct
{
    tU32 m_piValue;
    tU32 m_frequency;
	tU32 m_pi_StoredTime;
	// add one space for the \0
    tU8 m_label[ETAL_DEF_MAX_PS_LEN+1]; 
} EtalRdsLandscapeExternalInfo;

/*
 * EtalEnsembleList
 */
typedef struct
{
	tU32        m_UeId;
	tU32        m_frequency;
	tChar       m_ensembleLabel[ETAL_DEF_MAX_LABEL_LEN];
} EtalTmlLandscapeEnsembleList;

/*
 * EtalServiceList
 */
typedef struct
{
	tU32        m_serviceCount;
	tU32        m_service[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE];
	tChar       m_serviceLabel[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE][ETAL_DEF_MAX_LABEL_LEN];
} EtalTmlLandscapeServiceList;

// Landscape Retrieval information 
typedef struct
{
	tU32	    m_ensembleCount;
	EtalTmlLandscapeEnsembleList m_ensembleList[ETAL_DEF_MAX_ENSEMBLE];
	EtalTmlLandscapeServiceList  m_serviceList[ETAL_DEF_MAX_ENSEMBLE];
} EtalTmlDabLandscapeExternalInfo;


// Landscape Retrieval information 
typedef struct
{
	EtalRdsLandscapeExternalInfo m_FMLandscapeInfo[ETAL_DEF_MAX_FM_LANDSCAPE_SIZE];
	EtalTmlDabLandscapeExternalInfo m_DabLandscapeInfo;
	tU32 m_FMLanscapeLen;
	tU32 m_DABdbLen; 
} ETAL_DatabaseExternalInfo;


/***********************************
 *
 * API function interfaces
 *
 **********************************/

/***********************************
 *
 * Radio text (generic)
 *
 **********************************/
/*
 * EtalTextInfo
 */
typedef struct
{
	EtalBcastStandard m_broadcastStandard;
	tBool m_serviceNameIsNew;
	tChar m_serviceName[ETAL_DEF_MAX_SERVICENAME];
	tU8   m_serviceNameCharset;
	tBool m_currentInfoIsNew;
	tChar m_currentInfo[ETAL_DEF_MAX_INFO];
	tU8   m_currentInfoCharset;
} EtalTextInfo;


/***********************************
 *
 * RDS
 *
 **********************************/
/* from DABMW spec */
#define ETAL_DEF_MAX_PS_LEN       8
#define ETAL_DEF_MAX_RT_LEN      64
/* from DABMW implementation, DABMW_AF_LIST_BFR_LEN */
#define ETAL_DEF_MAX_AFLIST      26

#define NB_OF_THRESHOLDS        16

/*
 * bitmaps for m_validityBitmap and RDSServiceList parameters
 */
 typedef enum
{
    ETAL_DECODED_RDS_VALID_PS  =    0x001,
    ETAL_DECODED_RDS_VALID_DI  =    0x002,
    ETAL_DECODED_RDS_VALID_PI  =    0x004,
    ETAL_DECODED_RDS_VALID_TOM =    0x008,  /* Time, Offset, MJD */
    ETAL_DECODED_RDS_VALID_RT  =    0x010,
    ETAL_DECODED_RDS_VALID_AF  =    0x020,
    ETAL_DECODED_RDS_VALID_PTY =    0x040,
    ETAL_DECODED_RDS_VALID_TP  =    0x080,
    ETAL_DECODED_RDS_VALID_TA  =    0x100,
    ETAL_DECODED_RDS_VALID_MS  =    0x200,
    ETAL_DECODED_RDS_VALID_ALL =    0x3FF
} EtalEnumDecodedRDSBitmap;

/*
 * EtalRDSData
 */
typedef struct
{
	tU32		m_validityBitmap;   /* enum EtalEnumDecodedRDSBitmap can be used to check this parameter */
	tChar		m_PS[ETAL_DEF_MAX_PS_LEN];
	tU8			m_DI;
	tU16		m_PI;
    tU8         m_PTY:5;
    tU8         m_TP :1;
    tU8         m_TA :1;
    tU8         m_MS :1;
	tU8			m_timeHour;
	tU8			m_timeMinutes;
	tU8			m_offset;
	tU32		m_MJD;
	tChar		m_RT[ETAL_DEF_MAX_RT_LEN];
	tU32		m_AFListLen;
	tU32		m_AFListPI;
	tU8			m_AFList[ETAL_DEF_MAX_AFLIST];
} EtalRDSData;


 typedef enum
{
    ETAL_RDS_STRATEGY_F_AFENABLE 		= 0x0001,
    ETAL_RDS_STRATEGY_F_TAEANBLE 		= 0x0002,
    ETAL_RDS_STRATEGY_F_REGENABLE 		= 0x0004,
    ETAL_RDS_STRATEGY_F_EONENABLE 		= 0x0008,
    ETAL_RDS_STRATEGY_F_PTYENABLE 		= 0x0010,
    ETAL_RDS_STRATEGY_F_INTASWITCH 		= 0x0020,
    ETAL_RDS_STRATEGY_F_INEONTASWITCH 		= 0x0040,
    ETAL_RDS_STRATEGY_F_RDSSTATION 		= 0x0080,
    ETAL_RDS_STRATEGY_F_AFCHECKING 		= 0x0100,
    ETAL_RDS_STRATEGY_F_NoPTY 			= 0x0200,
    ETAL_RDS_STRATEGY_F_AFSWITCHED		= 0x0400,
    ETAL_RDS_STRATEGY_F_TASTATUS 		= 0x1800,
    ETAL_RDS_STRATEGY_F_INTASEEK		= 0x2000,
    ETAL_RDS_STRATEGY_F_INPISEEK		= 0x4000,
 }EtalEnumRDSStrategyBitmap;

#define 	ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_TO_TUNER 			0x1000
#define 	ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_BACK_FROM_TUNER	0x1800

/*
 * EtalRDSStrategyStatus
 */
typedef struct
{
	tU32   	m_RDSStrategyBitmap;
	tU8   	m_AFSwitchedFreq;		/*Used for the frequency display during AF switch, AF search*/	
} EtalRDSStrategyStatus;

/*
 * EtalRDSAFSearchData
 */
typedef struct
{
	tU16	 m_PI;
	tU32	 m_AFListLen;
	tU8	 m_AFList[ETAL_DEF_MAX_AFLIST];
} EtalRDSAFSearchData;

/***********************************
 *
 * Seek definition
 *
 **********************************/

// addition of information to manage Path Name Selection
//
/*
 * EtalPathName
 */
typedef enum
{
	ETAL_PATH_NAME_UNDEF	= 0,
	ETAL_PATH_NAME_DAB_1   	= 1,
	ETAL_PATH_NAME_DAB_2	= 2,
	ETAL_PATH_NAME_FM_FG   	= 3,
	ETAL_PATH_NAME_FM_BG	= 4,
	ETAL_PATH_NAME_AM 	    = 5,
	ETAL_PATH_NAME_FM_HD_FG	= 6,
	ETAL_PATH_NAME_FM_HD_BG = 7,
	ETAL_PATH_NAME_AM_HD    = 8,
	ETAL_PATH_NAME_DRM_1	= 9,
	ETAL_PATH_NAME_DRM_2	= 10
} EtalPathName;

typedef struct
{
	tBool 	tpSeek;
	tBool 	taSeek;
	tBool	ptySeek;
	tU8	 	ptyCode;
	tBool	piSeek;
	tU32 	pi;
} EtalRDSSeekTy;

/***********************************
 *
 * Learn
 *
 **********************************/
/*
 * etalLearnReportingModeStatusTy
 */
typedef enum
{
    normalMode,
    sortMode
} etalLearnReportingModeStatusTy;

#endif /* ETALTML_TYPES_H_ */
