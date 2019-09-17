package etal::etaltml_types;
use strict;
use warnings;
use Exporter;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

our @ISA = qw(Exporter);
our @EXPORT = qw(
    ETAL_DEF_MAX_FM_LANDSCAPE_SIZE ETAL_DEF_MAX_FM_LANDSCAPE_SIZE ETAL_DEF_MAX_PS_LEN ETAL_DEF_MAX_RT_LEN ETAL_DEF_MAX_AFLIST NB_OF_THRESHOLDS ETAL_LEARN_MAX_NB_FREQ ETAL_LEARN_STARTED
    ETAL_LEARN_RESULT ETAL_LEARN_FINISHED ETAL_LEARN_ERROR ETAL_SCAN_STARTED ETAL_SCAN_RESULT ETAL_SCAN_FINISHED ETAL_SCAN_ERROR initialFrequency
    lastFrequency ETAL_DEF_MAX_PS_LEN ETAL_DEF_MAX_RT_LEN ETAL_DEF_MAX_AFLIST NB_OF_THRESHOLDS ETAL_DECODED_RDS_VALID_PS ETAL_DECODED_RDS_VALID_DI ETAL_DECODED_RDS_VALID_PI
    ETAL_DECODED_RDS_VALID_TOM ETAL_DECODED_RDS_VALID_RT ETAL_DECODED_RDS_VALID_AF ETAL_DECODED_RDS_VALID_PTY ETAL_DECODED_RDS_VALID_TP ETAL_DECODED_RDS_VALID_TA ETAL_DECODED_RDS_VALID_MS ETAL_DECODED_RDS_VALID_ALL
    ETAL_RDS_STRATEGY_F_AFENABLE ETAL_RDS_STRATEGY_F_TAEANBLE ETAL_RDS_STRATEGY_F_REGENABLE ETAL_RDS_STRATEGY_F_EONENABLE ETAL_RDS_STRATEGY_F_PTYENABLE ETAL_RDS_STRATEGY_F_INTASWITCH ETAL_RDS_STRATEGY_F_INEONTASWITCH ETAL_RDS_STRATEGY_F_RDSSTATION
    ETAL_RDS_STRATEGY_F_AFCHECKING ETAL_RDS_STRATEGY_F_NoPTY ETAL_RDS_STRATEGY_F_AFSWITCHED ETAL_RDS_STRATEGY_F_TASTATUS ETAL_RDS_STRATEGY_F_INTASEEK ETAL_RDS_STRATEGY_F_INPISEEK ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_TO_TUNER ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_BACK_FROM_TUNER
    ETAL_PATH_NAME_UNDEF ETAL_PATH_NAME_DAB_1 ETAL_PATH_NAME_DAB_2 ETAL_PATH_NAME_FM_FG ETAL_PATH_NAME_FM_BG ETAL_PATH_NAME_AM ETAL_PATH_NAME_FM_HD_FG ETAL_PATH_NAME_FM_HD_BG
    ETAL_PATH_NAME_AM_HD ETAL_PATH_NAME_DRM_1 ETAL_PATH_NAME_DRM_2 normalMode sortMode
    );
%EXPORT_TAGS = ( ALL => [ @EXPORT_OK, @EXPORT ] );

#!
#!  \file 		etaltml_types.h
#!  \brief 	<i><b> ETAL API </b></i>
#!  \details   Type definitions for the ETAL user application, extended functionalities (TML)
#!  \author 	Raffaele Belardi
#!

#
 # DO NOT INCLUDE THIS FILE DIRECTLY!
 #
 # It is included by etaltml_api.h
 #

#ifndef ETALTML_TYPES_H_
#define ETALTML_TYPES_H_

#**********************************
 #
 # Defines
 #
 #*********************************
#if defined (CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER)
#    use constant ETAL_DEF_MAX_FM_LANDSCAPE_SIZE                     =>CONFIG_DABMW_AMFM_STORED_SERVICE_NUMBER;
#else
    use constant ETAL_DEF_MAX_FM_LANDSCAPE_SIZE                     =>24;
#endif


#**********************************
 #
 # RDS
 #
 #*********************************
# from DABMW spec 
use constant ETAL_DEF_MAX_PS_LEN        =>8;
use constant ETAL_DEF_MAX_RT_LEN       =>64;
# from DABMW implementation, DABMW_AF_LIST_BFR_LEN 
use constant ETAL_DEF_MAX_AFLIST       =>26;

use constant NB_OF_THRESHOLDS         =>16;


#**********************************
 #
 # Events
 #
 #*********************************
#
 # EtalLearnStatus
 #
#typedef struct
#{
#	ETAL_HANDLE             m_receiverHandle;
#	tU32                    m_services;
#} EtalLearnStatus;

#typedef struct
#{
#    tS32                        m_fieldStrength;
#    tU32                        m_frequency;
#} EtalLearnFrequencyListTy;

#
 # Learn max number of reported frequencies
 #
use constant ETAL_LEARN_MAX_NB_FREQ       =>30;

#
 # etalLearnEventStatusTy
 #
use constant
{
    ETAL_LEARN_STARTED=> 0,# warning generated value
    ETAL_LEARN_RESULT=> 1,# warning generated value
    ETAL_LEARN_FINISHED=> 2,# warning generated value
    ETAL_LEARN_ERROR => 3 # warning generated value
}; # etalLearnEventStatusTy

#
 # EtalLearnFrequencyTy
 #
#typedef struct
#{
#    tS32                        m_fieldStrength;
#    tU32                        m_frequency;
#    tBool						m_HDServiceFound;
#    tU32                        m_ChannelID;
#} EtalLearnFrequencyTy;

#
 # EtalLearnStatusTy
 #
#typedef struct
#{
#    ETAL_HANDLE                 m_receiverHandle;
#    etalLearnEventStatusTy      m_status;
#    tU32                        m_frequency;
#    tU32                        m_nbOfFrequency;
#    EtalLearnFrequencyTy*       m_frequencyList;
#} EtalLearnStatusTy;

#
 # etalScanEventStatusTy
 #
use constant
{
    ETAL_SCAN_STARTED=> 0,# warning generated value
    ETAL_SCAN_RESULT=> 1,# warning generated value
    ETAL_SCAN_FINISHED=> 2,# warning generated value
    ETAL_SCAN_ERROR => 3 # warning generated value
}; # etalScanEventStatusTy

#
 # EtalScanStatusTy
 #
#typedef struct
#{
#    ETAL_HANDLE                 m_receiverHandle;
#    etalScanEventStatusTy       m_status;
#    tU32                        m_frequency;
#    tBool                       m_frequencyFound;
#} EtalScanStatusTy;

#typedef struct
#{
#	ETAL_HANDLE              m_receiverHandle;
#	tU8                      m_status;
#	tU8                      m_providerType;
#	tS32                     m_absoluteDelayEstimate;
#	tS32                     m_delayEstimate;
#	tU32                     m_timestamp_FAS;
#	tU32                     m_timestamp_SAS;
#	tU32                     m_RMS2_FAS;
#	tU32                     m_RMS2_SAS;
#	tU32					 m_confidenceLevel;
#} EtalSeamlessEstimationStatus;

#typedef struct
#{
#	ETAL_HANDLE              m_receiverHandle;
#	tU8                      m_status;
#	tS32                     m_absoluteDelayEstimate;
#} EtalSeamlessSwitchingStatus;

#typedef struct
#{
#	ETAL_HANDLE              m_receiverHandle;
#	tBool                    m_IsFoundstatus;
#	tBool					 m_freqIsDab;
#	tU32                     m_freq;
#	tU32					 m_SidPi;
#	tU32					 m_Ueid;
#	tBool 					 m_AFisAvailable;
#	tBool					 m_ssApplicableOnAF;
#	tBool					 m_AFisSync;
#} EtalTuneServiceIdStatus;


# Landscape Retrieval information 
#typedef struct
#{
#    tU32 m_piValue;
#    tU32 m_frequency;
#	tU32 m_pi_StoredTime;
	# add one space for the \0
#    tU8 m_label[ETAL_DEF_MAX_PS_LEN+1]; 
#} EtalRdsLandscapeExternalInfo;

#
 # EtalEnsembleList
 #
#typedef struct
#{
#	tU32        m_UeId;
#	tU32        m_frequency;
#	tChar       m_ensembleLabel[ETAL_DEF_MAX_LABEL_LEN];
#} EtalTmlLandscapeEnsembleList;

#
 # EtalServiceList
 #
#typedef struct
#{
#	tU32        m_serviceCount;
#	tU32        m_service[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE];
#	tChar       m_serviceLabel[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE][ETAL_DEF_MAX_LABEL_LEN];
#} EtalTmlLandscapeServiceList;

# Landscape Retrieval information 
#typedef struct
#{
#	tU32	    m_ensembleCount;
#	EtalTmlLandscapeEnsembleList m_ensembleList[ETAL_DEF_MAX_ENSEMBLE];
#	EtalTmlLandscapeServiceList  m_serviceList[ETAL_DEF_MAX_ENSEMBLE];
#} EtalTmlDabLandscapeExternalInfo;


# Landscape Retrieval information 
#typedef struct
#{
#	EtalRdsLandscapeExternalInfo m_FMLandscapeInfo[ETAL_DEF_MAX_FM_LANDSCAPE_SIZE];
#	EtalTmlDabLandscapeExternalInfo m_DabLandscapeInfo;
#	tU32 m_FMLanscapeLen;
#	tU32 m_DABdbLen; 
#} ETAL_DatabaseExternalInfo;


#**********************************
 #
 # API function interfaces
 #
 #*********************************

#**********************************
 #
 # Radio text (generic)
 #
 #*********************************
#
 # EtalTextInfo
 #
#typedef struct
#{
#	EtalBcastStandard m_broadcastStandard;
#	tBool m_serviceNameIsNew;
#	tChar m_serviceName[ETAL_DEF_MAX_SERVICENAME];
#	tBool m_currentInfoIsNew;
#	tChar m_currentInfo[ETAL_DEF_MAX_INFO];
#} EtalTextInfo;


#
 # EtalSeekTerminationModeTy
 #
use constant
{
	initialFrequency=> 0,# warning generated value
	lastFrequency => 1 # warning generated value
}; # EtalSeekTerminationModeTy


#**********************************
 #
 # RDS
 #
 #*********************************
# from DABMW spec 
use constant ETAL_DEF_MAX_PS_LEN        =>8;
use constant ETAL_DEF_MAX_RT_LEN       =>64;
# from DABMW implementation, DABMW_AF_LIST_BFR_LEN 
use constant ETAL_DEF_MAX_AFLIST       =>26;

use constant NB_OF_THRESHOLDS         =>16;

#
 # bitmaps for m_validityBitmap and RDSServiceList parameters
 #
 use constant
{
    ETAL_DECODED_RDS_VALID_PS  =>    0x001,
    ETAL_DECODED_RDS_VALID_DI  =>    0x002,
    ETAL_DECODED_RDS_VALID_PI  =>    0x004,
    ETAL_DECODED_RDS_VALID_TOM =>    0x008,  # Time, Offset, MJD 
    ETAL_DECODED_RDS_VALID_RT  =>    0x010,
    ETAL_DECODED_RDS_VALID_AF  =>    0x020,
    ETAL_DECODED_RDS_VALID_PTY =>    0x040,
    ETAL_DECODED_RDS_VALID_TP  =>    0x080,
    ETAL_DECODED_RDS_VALID_TA  =>    0x100,
    ETAL_DECODED_RDS_VALID_MS  =>    0x200,
    ETAL_DECODED_RDS_VALID_ALL =>    0x3FF
}; # EtalEnumDecodedRDSBitmap

#
 # EtalRDSData
 #
#typedef struct
#{
#	tU32		m_validityBitmap;   # enum EtalEnumDecodedRDSBitmap can be used to check this parameter 
#	tChar		m_PS[ETAL_DEF_MAX_PS_LEN];
#	tU8			m_DI;
#	tU16		m_PI;
#    tU8         m_PTY:5;
#    tU8         m_TP :1;
#    tU8         m_TA :1;
#    tU8         m_MS :1;
#	tU8			m_timeHour;
#	tU8			m_timeMinutes;
#	tU8			m_offset;
#	tU32		m_MJD;
#	tChar		m_RT[ETAL_DEF_MAX_RT_LEN];
#	tU32		m_AFListLen;
#	tU32		m_AFListPI;
#	tU8			m_AFList[ETAL_DEF_MAX_AFLIST];
#} EtalRDSData;


 use constant
{
    ETAL_RDS_STRATEGY_F_AFENABLE 		=> 0x0001,
    ETAL_RDS_STRATEGY_F_TAEANBLE 		=> 0x0002,
    ETAL_RDS_STRATEGY_F_REGENABLE 		=> 0x0004,
    ETAL_RDS_STRATEGY_F_EONENABLE 		=> 0x0008,
    ETAL_RDS_STRATEGY_F_PTYENABLE 		=> 0x0010,
    ETAL_RDS_STRATEGY_F_INTASWITCH 		=> 0x0020,
    ETAL_RDS_STRATEGY_F_INEONTASWITCH 		=> 0x0040,
    ETAL_RDS_STRATEGY_F_RDSSTATION 		=> 0x0080,
    ETAL_RDS_STRATEGY_F_AFCHECKING 		=> 0x0100,
    ETAL_RDS_STRATEGY_F_NoPTY 			=> 0x0200,
    ETAL_RDS_STRATEGY_F_AFSWITCHED		=> 0x0400,
    ETAL_RDS_STRATEGY_F_TASTATUS 		=> 0x1800,
    ETAL_RDS_STRATEGY_F_INTASEEK		=> 0x2000,
    ETAL_RDS_STRATEGY_F_INPISEEK		=> 0x4000,
 }; #EtalEnumRDSStrategyBitmap

use constant 	ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_TO_TUNER 			 =>0x1000;
use constant 	ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_BACK_FROM_TUNER	 =>0x1800;

#
 # EtalRDSStrategyStatus
 #
#typedef struct
#{
#	tU32   	m_RDSStrategyBitmap;
#	tU8   	m_AFSwitchedFreq;		#Used for the frequency display during AF switch, AF search	
#} EtalRDSStrategyStatus;


#**********************************
 #
 # Seamless switching
 #
 #*********************************
#
 # etalSeamlessSwitchingConfigTy
 #
#typedef struct
#{
#	tU8  	systemToSwitch;
#	tU8	    providerType;
#	tS32    absoluteDelayEstimate;
#	tS32    delayEstimate;
#	tU32 	timestampFAS;
#	tU32 	timestampSAS;
#	tU32	averageRMS2FAS;
#	tU32	averageRMS2SAS;
#} etalSeamlessSwitchingConfigTy;

#
 # etalSeamlessEstimationConfigTy
 #
#typedef struct
#{
#	tU8  	mode;
#	tS32    startPosition;
#	tS32    stopPosition;
#} etalSeamlessEstimationConfigTy;

#
 # EtalSeekThreshold
 #
#ifdef CONFIG_ETALTML_AUTO_SEEK_INTERNAL
#typedef struct
#{
#    tS8 SeekThresholdBBFieldStrength;
#    tU8 SeekThresholdDetune;
#	tU8	SeekThresholdAdjacentChannel;
#    tU8 SeekThresholdMultipath;
#	tU8 SeekThresholdSignalNoiseRatio;
#	tU8 SeekThresholdMpxNoise;
#} EtalSeekThreshold;
#else
#typedef struct
#{
#    tS8 SeekThresholdRFFieldStrength;
#    tS8 SeekThresholdBBFieldStrength;
#    tU8 SeekThresholdDetune;
#    tU8 SeekThresholdAdjacentChannel;
#    tU8 SeekThresholdMultipath;
#    tU8 SeekThresholdFSScaleMaxAdj;
#    tU8 SeekThresholdFSScaleMinAdj;
#    tU8 SeekThresholdFSScaleMaxMp;
#    tU8 SeekThresholdFSScaleMinMp;
#	tU8 SeekThresholdSignalNoiseRatio; # not used for now in external version for filtering
#	tU8 SeekThresholdMpxNoise; # not used for now in external version for filtering
#} EtalSeekThreshold;
#endif


#typedef struct
#{
#	tU16	 m_PI;
#	tU32	 m_AFListLen;
#	tU8	 m_AFList[ETAL_DEF_MAX_AFLIST];
#} EtalRDSAFSearchData;


#**********************************
 #
 # Seek definition
 #
 #*********************************

# addition of information to manage Path Name Selection
#
#
 # EtalPathName
 #
use constant
{
	ETAL_PATH_NAME_UNDEF	=> 0,
	ETAL_PATH_NAME_DAB_1   	=> 1,
	ETAL_PATH_NAME_DAB_2	=> 2,
	ETAL_PATH_NAME_FM_FG   	=> 3,
	ETAL_PATH_NAME_FM_BG	=> 4,
	ETAL_PATH_NAME_AM 	    => 5,
	ETAL_PATH_NAME_FM_HD_FG	=> 6,
	ETAL_PATH_NAME_FM_HD_BG => 7,
	ETAL_PATH_NAME_AM_HD    => 8,
	ETAL_PATH_NAME_DRM_1	=> 9,
	ETAL_PATH_NAME_DRM_2	=> 10
}; # EtalPathName

#typedef struct
#{
#	tBool 	tpSeek;
#	tBool 	taSeek;
#	tBool	ptySeek;
#	tU8	 	ptyCode;
#	tBool	piSeek;
#	tU32 	pi;
#} EtalRDSSeekTy;

#**********************************
 #
 # Learn
 #
 #*********************************
#
 # etalLearnReportingModeStatusTy
 #
use constant
{
    normalMode=> 0,# warning generated value
    sortMode => 1 # warning generated value
}; # etalLearnReportingModeStatusTy

#endif # ETALTML_TYPES_H_ 

1;
