package etal::etal_types;
use strict;
use warnings;
use Exporter;
use vars qw(@ISA @EXPORT @EXPORT_OK %EXPORT_TAGS $VERSION);

our @ISA = qw(Exporter);
our @EXPORT = qw(
    ETAL_CAPA_MAX_TUNER ETAL_CAPA_MAX_FRONTEND_PER_TUNER ETAL_SILICON_VERSION_MAX ETAL_DEF_MAX_READWRITE_SIZE ETAL_MAX_COMM_ERR_MSG ETAL_VERSION_NAME_MAX ETAL_INVALID ETAL_INVALID_HANDLE
    ETAL_INVALID_FREQUENCY ETAL_INVALID_UEID ETAL_INVALID_SID ETAL_INVALID_MONITOR ETAL_CAPA_MAX_FRONTEND ETAL_SEEK_STEP_UNDEFINED ETAL_MAX_BCAST_STD ETAL_MAX_QUALITY_PER_MONITOR
    ETAL_BAND_FM_BIT ETAL_BAND_DRM_BIT ETAL_BAND_AM_BIT ETAL_BAND_HD_BIT ETAL_BAND_DAB_BIT ETAL_HDRADIO_DSQM_DIVISOR ETAL_DISS_FILTER_INDEX_MAX ETAL_DEF_MAX_SERVICENAME
    ETAL_DEF_MAX_INFO_TITLE ETAL_DEF_MAX_INFO_ARTIST ETAL_DEF_MAX_INFO ETAL_DEF_MAX_INFO_ALBUM ETAL_DEF_MAX_INFO_GENRE ETAL_DEF_MAX_INFO_COMMENT_SHORT ETAL_DEF_MAX_INFO_COMMENT ETAL_DEF_MAX_INFO_UFID
    ETAL_DEF_MAX_INFO_COMMERCIAL_PRICE ETAL_DEF_MAX_INFO_COMMERCIAL_CONTACT ETAL_DEF_MAX_INFO_COMMERCIAL_SELLER ETAL_DEF_MAX_INFO_COMMERCIAL_DESC ETAL_DEF_MAX_INFO_XHDR ETAL_FE_FOREGROUND ETAL_FE_BACKGROUND ETAL_CAPA_MAX_DATATYPE
    deviceSingleFE deviceTwinFE ETAL_TUNESTATUS_SYNCMASK_FOUND ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE ETAL_TUNESTATUS_SYNCMASK_COMPLETE ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED
    ETAL_TUNESTATUS_SYNCMASK_SIS_FAILED ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED ETAL_TUNESTATUS_SYNCMASK_DABSYNC ETAL_TUNESTATUS_SYNCMASK_DABMCI ETAL_TUNESTATUS_SYNCMASK_HD_SYNC ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC ETAL_DEF_MAX_ENSEMBLE ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE
    ETAL_DEF_MAX_SC_PER_SERVICE ETAL_DEF_MAX_LABEL_LEN STAR_RDS_MAX_BLOCK START_RDS_DATA_ERRCOUNT_MASK START_RDS_DATA_CTYPE_MASK START_RDS_DATA_BLOCKID_MASK START_RDS_DATA_MASK START_RDS_DATA_ERRCOUNT_SHIFT
    START_RDS_DATA_CTYPE_SHIFT START_RDS_DATA_BLOCKID_SHIFT STAR_RNR_DATARDY STAR_RNR_SYNC STAR_RNR_BOFL STAR_RNR_BNE STAR_RNR_RDS_deviation STAR_RNR_TUNCH2
    STAR_RNR_TUNCH1 TRUE true FALSE false ETAL_FE_HANDLE_1 ETAL_FE_HANDLE_2 ETAL_FE_HANDLE_3
    ETAL_FE_HANDLE_4 ETAL_DATA_TYPE_UNDEF ETAL_DATA_TYPE_AUDIO ETAL_DATA_TYPE_DCOP_AUDIO ETAL_DATA_TYPE_DATA_SERVICE ETAL_DATA_TYPE_DAB_DATA_RAW ETAL_DATA_TYPE_DAB_AUDIO_RAW
    ETAL_DATA_TYPE_DAB_FIC ETAL_DATA_TYPE_TEXTINFO ETAL_DATA_TYPE_FM_RDS ETAL_DATA_TYPE_FM_RDS_RAW ETAL_DATASERV_TYPE_NONE ETAL_DATASERV_TYPE_EPG_RAW ETAL_DATASERV_TYPE_SLS ETAL_DATASERV_TYPE_SLS_XPAD
    ETAL_DATASERV_TYPE_TPEG_RAW ETAL_DATASERV_TYPE_TPEG_SNI ETAL_DATASERV_TYPE_SLI ETAL_DATASERV_TYPE_EPG_BIN ETAL_DATASERV_TYPE_EPG_SRV ETAL_DATASERV_TYPE_EPG_PRG ETAL_DATASERV_TYPE_EPG_LOGO ETAL_DATASERV_TYPE_JML_OBJ
    ETAL_DATASERV_TYPE_FIDC ETAL_DATASERV_TYPE_TMC ETAL_DATASERV_TYPE_DLPLUS ETAL_DATASERV_TYPE_PSD ETAL_DATASERV_TYPE_UNDEFINED ETAL_DATASERV_TYPE_ALL ETAL_BCAST_STD_UNDEF ETAL_BCAST_STD_DAB
    ETAL_BCAST_STD_DRM ETAL_BCAST_STD_FM ETAL_BCAST_STD_AM ETAL_BCAST_STD_HD_FM ETAL_BCAST_STD_HD ETAL_BCAST_STD_HD_AM ETAL_BAND_UNDEF ETAL_BAND_FM
    ETAL_BAND_FMEU ETAL_BAND_FMUS ETAL_BAND_HD ETAL_BAND_FMJP ETAL_BAND_FMEEU ETAL_BAND_WB ETAL_BAND_USERFM ETAL_BAND_DRMP
    ETAL_BAND_DRM30 ETAL_BAND_DAB3 ETAL_BAND_DABL ETAL_BAND_AM ETAL_BAND_LW ETAL_BAND_MWEU ETAL_BAND_MWUS ETAL_BAND_SW
    ETAL_BAND_CUSTAM ETAL_BAND_USERAM ETAL_COUNTRY_VARIANT_UNDEF ETAL_COUNTRY_VARIANT_EU ETAL_COUNTRY_VARIANT_CHINA ETAL_COUNTRY_VARIANT_KOREA ETAL_SERVSEL_MODE_SERVICE ETAL_SERVSEL_MODE_DAB_SC
    ETAL_SERVSEL_MODE_DAB_SUBCH ETAL_SERVSEL_SUBF_REMOVE ETAL_SERVSEL_SUBF_APPEND ETAL_SERVSEL_SUBF_SET ETAL_RET_SUCCESS ETAL_RET_ERROR ETAL_RET_ALREADY_INITIALIZED ETAL_RET_DATAPATH_SINK_ERR
    ETAL_RET_FRONTEND_LIST_ERR ETAL_RET_FRONTEND_NOT_AVAILABLE ETAL_RET_INVALID_BCAST_STANDARD ETAL_RET_INVALID_DATA_TYPE ETAL_RET_INVALID_HANDLE ETAL_RET_INVALID_RECEIVER ETAL_RET_NO_DATA ETAL_RET_NO_HW_MODULE
    ETAL_RET_NOT_INITIALIZED ETAL_RET_NOT_IMPLEMENTED ETAL_RET_PARAMETER_ERR ETAL_RET_QUAL_CONTAINER_ERR ETAL_RET_QUAL_FE_ERR ETAL_RET_ALREADY_USED ETAL_RET_IN_PROGRESS ETAL_INFO_TUNE
    ETAL_INFO_FM_STEREO ETAL_WARNING_OUT_OF_MEMORY ETAL_ERROR_COMM_FAILED ETAL_RECEIVER_ALIVE_ERROR ETAL_INFO_SEEK ETAL_INFO_LEARN ETAL_INFO_SCAN ETAL_INFO_SEAMLESS_ESTIMATION_END
    ETAL_INFO_SEAMLESS_SWITCHING_END ETAL_INFO_TUNE_SERVICE_ID ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO ETAL_INFO_RDS_STRATEGY ETAL_INFO_UNDEF ETAL_AUDIO_SOURCE_AUTO_HD ETAL_AUDIO_SOURCE_STAR_AMFM ETAL_AUDIO_SOURCE_DCOP_STA680
    ETAL_AUDIO_SOURCE_HD_ALIGN ETAL_AUDIO_SOURCE_DCOP_STA660 ETAL_SEEK_STARTED ETAL_SEEK_RESULT ETAL_SEEK_FINISHED ETAL_SEEK_ERROR ETAL_SEEK_ERROR_ON_START ETAL_SEEK_ERROR_ON_GET_STATUS
    ETAL_SEEK_ERROR_ON_STOP ETAL_PROCESSING_FEATURE_NONE ETAL_PROCESSING_FEATURE_FM_VPA ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING ETAL_PROCESSING_FEATURE_ALL ETAL_PROCESSING_FEATURE_UNSPECIFIED
    ETAL_TR_LEVEL_FATAL ETAL_TR_LEVEL_ERROR ETAL_TR_LEVEL_SYSTEM_MIN ETAL_TR_LEVEL_SYSTEM deviceUninitializedEntry deviceNotSupported deviceDisabled deviceSiliconVersionError
    deviceCommunication deviceDownload deviceParameters devicePingError deviceError deviceAvailable warningNone warningTraceDefault
    warningNoTrace state_initNotInitialized state_initStart state_OSALINIT state_tracePrintInit state_statusInitLock state_callbackInit state_datahandlerInit
    state_initCommunication_CMOST state_initCheckSiliconVersion_CMOST state_downloadFirmware_CMOST state_initCustomParameter_CMOST state_boardInit state_ipforwardInit state_initCommunication_MDR state_initCommunication_HDRADIO
    state_initReadSiliconVersion_DCOP state_initThreadSpawn state_initPing state_statusExternalInit state_ETALTML_init state_AudioInterfaceConfigure state_initComplete ETAL_EPG_LOGO_TYPE_UNKNOWN
    ETAL_EPG_LOGO_TYPE_UNRESTRICTED ETAL_EPG_LOGO_TYPE_MONO_SQUARE ETAL_EPG_LOGO_TYPE_COLOUR_SQUARE ETAL_EPG_LOGO_TYPE_MONO_RECTANGLE ETAL_EPG_LOGO_TYPE_COLOUR_RECTANGLE deviceUnknown deviceCMOST deviceSTAR
    deviceSTARS deviceSTART deviceDOT deviceDOTS deviceDOTT deviceDCOP deviceMDR deviceHD
    ETAL_BusI2C ETAL_BusSPI EtalQualityIndicator_Undef EtalQualityIndicator_DabFicErrorRatio EtalQualityIndicator_DabFieldStrength EtalQualityIndicator_DabMscBer EtalQualityIndicator_FmFieldStrength EtalQualityIndicator_FmFrequencyOffset
    EtalQualityIndicator_FmModulationDetector EtalQualityIndicator_FmMultipath EtalQualityIndicator_FmUltrasonicNoise EtalQualityIndicator_HdQI EtalQualityIndicator_HdCdToNo EtalQualityIndicator_HdDSQM EtalCommStatus_NoError EtalCommStatus_ChecksumError
    EtalCommStatus_TimeoutError EtalCommStatus_ProtocolHeaderError EtalCommStatus_MessageFormatError EtalCommStatus_BusError EtalCommStatus_ProtocolContinuityError EtalCommStatus_GenericError IDX_CMT_tunApp0_fm_wsp_smLevShp IDX_CMT_tunApp0_fm_wsp_smMpShp
    IDX_CMT_tunApp0_fm_wsp_smDistShp IDX_CMT_tunApp0_fm_wsp_sm IDX_CMT_tunApp0_fm_wsp_sbLevShp IDX_CMT_tunApp0_fm_wsp_sbMpShp IDX_CMT_tunApp0_fm_wsp_sbDistShp IDX_CMT_tunApp0_fm_wsp_sb IDX_CMT_tunApp0_fm_wsp_hcLevShp IDX_CMT_tunApp0_fm_wsp_hcMpShp
    IDX_CMT_tunApp0_fm_wsp_hcDistShp IDX_CMT_tunApp0_fm_wsp_hc IDX_CMT_tunApp0_fm_wsp_hbLevShp IDX_CMT_tunApp0_fm_wsp_hbMpShp IDX_CMT_tunApp0_fm_wsp_hbDistShp IDX_CMT_tunApp0_fm_wsp_hb IDX_CMT_tunApp0_fm_wsp_smDistProc IDX_CMT_tunApp0_fm_wsp_sbDistProc
    IDX_CMT_tunApp0_fm_wsp_hcDistProc IDX_CMT_tunApp0_fm_wsp_hbDistProc IDX_CMT_tunApp0_fm_qdR_fstLog IDX_CMT_tunApp0_fm_qdR_fstBB IDX_CMT_tunApp0_fm_qdR_fstRF IDX_CMT_tunApp0_fm_qdR_detune IDX_CMT_tunApp0_fm_qdR_mpxNoise IDX_CMT_tunApp0_fm_qdR_mp
    IDX_CMT_tunApp0_fm_qdR_adj IDX_CMT_tunApp0_fm_qdR_snr IDX_CMT_tunApp0_fm_qdR_coChannel IDX_CMT_tunApp0_fm_qdR_deviation IDX_CMT_tunApp0_fm_qdR_compressRegQ0 IDX_CMT_tunApp0_fm_qdR_compressRegQ1 IDX_CMT_tunApp0_fm_qdR_compressRegQ2 IDX_CMT_tunApp0_fm_qd_fstLog
    IDX_CMT_tunApp0_fm_qd_fstBB IDX_CMT_tunApp0_fm_qd_fstRF IDX_CMT_tunApp0_fm_qd_detune IDX_CMT_tunApp0_fm_qd_detuneFast IDX_CMT_tunApp0_fm_qd_mpxNoise IDX_CMT_tunApp0_fm_qd_mp IDX_CMT_tunApp0_fm_qd_adj IDX_CMT_tunApp0_fm_qd_snr
    IDX_CMT_tunApp0_fm_qd_deviation IDX_CMT_tunApp0_fm_qd_compressRegQ0 IDX_CMT_tunApp0_fm_qd_compressRegQ1 IDX_CMT_tunApp0_fm_qd_compressRegQ2 IDX_CMT_tunApp1_fm_qd_fstLog IDX_CMT_tunApp1_fm_qd_fstBB IDX_CMT_tunApp1_fm_qd_fstRF IDX_CMT_tunApp1_fm_qd_detune
    IDX_CMT_tunApp1_fm_qd_detuneFast IDX_CMT_tunApp1_fm_qd_mpxNoise IDX_CMT_tunApp1_fm_qd_mp IDX_CMT_tunApp1_fm_qd_adj IDX_CMT_tunApp1_fm_qd_snr IDX_CMT_tunApp1_fm_qd_deviation IDX_CMT_tunApp1_fm_qd_compressRegQ0 IDX_CMT_tunApp1_fm_qd_compressRegQ1
    IDX_CMT_tunApp1_fm_qd_compressRegQ2 IDX_CMT_tunApp0_fm_qdAf_compressRegQ0 IDX_CMT_tunApp0_fm_qdAf_compressRegQ1 IDX_CMT_tunApp0_fm_qdAf_compressRegQ2 IDX_CMT_tunApp1_fm_qdAf_compressRegQ0 IDX_CMT_tunApp1_fm_qdAf_compressRegQ1 IDX_CMT_tunApp1_fm_qdAf_compressRegQ2 IDX_CMT_tunApp0_am_wsp_smLevWght
    IDX_CMT_tunApp0_am_wsp_smDistWght IDX_CMT_tunApp0_am_wsp_sm IDX_CMT_tunApp0_am_wsp_lcLevWght IDX_CMT_tunApp0_am_wsp_lcDistWght IDX_CMT_tunApp0_am_wsp_lc IDX_CMT_tunApp0_am_wsp_hcLevWght IDX_CMT_tunApp0_am_wsp_hcDistWght IDX_CMT_tunApp0_am_wsp_hc
    IDX_CMT_tunApp0_am_qd_fstLog IDX_CMT_tunApp0_am_qd_fstBB IDX_CMT_tunApp0_am_qd_fstRF IDX_CMT_tunApp0_am_qd_detune IDX_CMT_tunApp0_am_qd_adj IDX_CMT_tunApp0_am_qd_modulation IDX_CMT_tunApp0_am_qd_compressRegQ0 IDX_CMT_tunApp0_am_qd_compressRegQ2
    IDX_CMT_tunApp1_am_qd_fstLog IDX_CMT_tunApp1_am_qd_fstBB IDX_CMT_tunApp1_am_qd_fstRF IDX_CMT_tunApp1_am_qd_detune IDX_CMT_tunApp1_am_qd_adj IDX_CMT_tunApp1_am_qd_modulation IDX_CMT_tunApp1_am_qd_compressRegQ0 IDX_CMT_tunApp1_am_qd_compressRegQ2
    ETAL_IDX_CMT_MAX_EXTERNAL fromIndex fromAddress cmdNormalMeasurement cmdRestartAFMeasurement ETAL_DISS_MODE_AUTO ETAL_DISS_MODE_MANUAL ETAL_CHN_UNDEF
    ETAL_CHN_FOREGROUND ETAL_CHN_BACKGROUND ETAL_CHN_BOTH ETAL_RDS_MODE ETAL_RBDS_MODE cmdDirectionUp cmdDirectionDown cmdAudioUnmuted
    cmdAudioMuted
    );
%EXPORT_TAGS = ( ALL => [ @EXPORT_OK, @EXPORT ] );

#!
#!  \file 		etal_types.h
#!  \brief 	<i><b> ETAL API </b></i>
#!  \details   Type definitions for the ETAL user application
#!  \author 	Raffaele Belardi
#!


#ifndef ETAL_TYPES_H_
#define ETAL_TYPES_H_

#**********************************
 #
 # Configurable defines
 #
 #*********************************
#**
 # \def		ETAL_CAPA_MAX_TUNER
 # 			A Tuner is a device that embeds one or more frontends
 # 			(currently ETAL supports only the CMOST Tuner, STAR or DOT version).
 #
 # 			This macro is the max number of Tuners in the application.
 #
 # \remark	Provided only for memory-constrained systems, it is suggested
 # 			to leave it to the default (2)
 #*
##define ETAL_CAPA_MAX_TUNER              1
use constant ETAL_CAPA_MAX_TUNER               =>2;

#**
 # \def		ETAL_CAPA_MAX_FRONTEND_PER_TUNER
 # 			Max number of frontends per tuner (e.g. for STAR-S it is 1, for STAR-T it is 2).
 #
 # \remark	Provided only for memory-constrained systems.
 # 			This affects the size of several statically allocated arrays. Normally it is better to leave
 # 			it to the default (2) to cover both -T and -S flavours of CMOST
 #*
use constant ETAL_CAPA_MAX_FRONTEND_PER_TUNER      =>2;

#**
 # \def		ETAL_SILICON_VERSION_MAX
 # 			Max length of the silicon version string returned
 # 			by #etal_get_init_status
 #*
use constant ETAL_SILICON_VERSION_MAX     =>16;

#**
 # def		ETAL_DEF_MAX_READWRITE_SIZE
 # 			Max number of addresses or indexes that can be 
 # 			passed to #etal_read_parameter or #etal_write_parameter
 #*
use constant ETAL_DEF_MAX_READWRITE_SIZE  =>32;

#**
 # \def		ETAL_MAX_COMM_ERR_MSG
 #			The max size of the buffer used to transfer the
 #			original message that originated a communication error event
 #			to the API user.
 #			If set to 0 this functionality is disabled and the only meaningful fields are:
 #			- m_commErr
 #			- m_commErrRaw
 #			- m_commErrReceiver
 #			- m_commErrBufferSize (always set to 0)
 #*
use constant ETAL_MAX_COMM_ERR_MSG    =>24;

#**
 # \def		ETAL_VERSION_NAME_MAX
 # 			Max size of the version strings returned by etal_get_version
 # 			in the EtalComponentVersion.m_name field
 #*
use constant ETAL_VERSION_NAME_MAX    =>48;

#**********************************
 #
 # Non-configurable defines
 #
 #*********************************
#
 # WARNING:
 #
 # These defines are NOT configurable and should NOT be changed by the API user
 # 
 #

#**
 # \def		ETAL_INVALID
 # 			A generic invalid value
 # \remark	The ETAL code makes assumptions about the ETAL_INVALID value so do not change it
 #*
use constant ETAL_INVALID           =>-1;

#**
 # \def		ETAL_INVALID_HANDLE
 # 			An invalid handle; may be used to indicate and error or an uninitalized value
 # \remark	The ETAL code makes assumptions about the ETAL_INVALID_HANDLE value so do not change it
 #*
use constant ETAL_INVALID_HANDLE     =>0;

#**
 # \def		ETAL_INVALID_FREQUENCY
 # 			An invalid frequency; may be used to indicate and error or an uninitalized value
 #*
use constant ETAL_INVALID_FREQUENCY 	 =>0xFFFFFFFF;
use constant ETAL_INVALID_UEID		 =>0x00FFFFFF;
use constant ETAL_INVALID_SID		 =>0xFFFFFFFF;
#**
 # \def		ETAL_INVALID_MONITOR
 # 			An invalid Monitor; used to initialize 'dont't care' entries
 # 			in a Monitor description
 #*
use constant ETAL_INVALID_MONITOR     =>0x7FFFFFFF;

#**
 # \def		ETAL_CAPA_MAX_FRONTEND
 #			Used to dimension several internal arrays
 #*
use constant ETAL_CAPA_MAX_FRONTEND           =>ETAL_CAPA_MAX_FRONTEND_PER_TUNER * ETAL_CAPA_MAX_TUNER;

#**
 # \def		ETAL_SEEK_STEP_UNDEFINED
 # 			Undefined seek stop for Automatic Seek procedure
 #*
use constant ETAL_SEEK_STEP_UNDEFINED	 =>0;

#**
 # \def		ETAL_MAX_BCAST_STD
 # 			MAX number of broadcast standards supported by ETAL.
 #
 # 			Currently only AM, FM, DAB are supported.
 #
 # 			This is used to dimension the capabilities array.
 #*
use constant ETAL_MAX_BCAST_STD               =>3;

#**
 # \def		ETAL_MAX_QUALITY_PER_MONITOR
 # 			Monitors are an ETAL feature that can be used to track a number of
 # 			quality detectors. This macro defines the max number of quality detectors
 # 			controlled by each Monitor. It is used to dimension internal arrays.
 #
 # 			It should be less than or equal to #ETAL_MAX_FILTERS
 #*
use constant ETAL_MAX_QUALITY_PER_MONITOR    =>16;

#
 # Frequecny band description
 #

#**
 # \def		ETAL_BAND_FM_BIT
 # 			Bit set in #EtalFrequencyBand for FM-type bands
 #*
use constant ETAL_BAND_FM_BIT       =>0x01000000;

#**
 # \def		ETAL_BAND_DRM_BIT
 # 			Bit set in #EtalFrequencyBand for DRM-type bands
 #*
use constant ETAL_BAND_DRM_BIT      =>0x02000000;

#**
 # \def		ETAL_BAND_AM_BIT
 # 			Bit set in #EtalFrequencyBand for AM-type bands
 #*
use constant ETAL_BAND_AM_BIT       =>0x04000000;

#**
 # \def		ETAL_BAND_HD_BIT
 # 			Bit set in #EtalFrequencyBand for HD-type bands
 #*
use constant ETAL_BAND_HD_BIT       =>0x08000000;

#**
 # \def		ETAL_BAND_DAB_BIT
 # 			Bit set in #EtalFrequencyBand for DAB-type bands
 #*
use constant ETAL_BAND_DAB_BIT      =>0x10000000;

#**
 # \def		ETAL_HDRADIO_DSQM_DIVISOR
 # 			The HDRadio DSQM is obtained dividing the
 # 			value returned by ETAL by this value
 #*
use constant ETAL_HDRADIO_DSQM_DIVISOR  =>65535;

#
 # Parameter for etal_debug_DISS_control
 #
 # DISS filter index range
 # DISS filter index 0 = cut off frequency 25 kHz (-3 dB)
 # DISS filter index 1 = cut off frequency 30 kHz (-3 dB)
 # DISS filter index 2 = cut off frequency 36 kHz (-3 dB)
 # DISS filter index 3 = cut off frequency 43 kHz (-3 dB)
 # DISS filter index 4 = cut off frequency 52 kHz (-3 dB)
 # DISS filter index 5 = cut off frequency 64 kHz (-3 dB)
 # DISS filter index 6 = cut off frequency 77 kHz (-3 dB)
 # DISS filter index 7 = cut off frequency 93 kHz (-3 dB)
 # DISS filter index 8 = cut off frequency 112 kHz (-3 dB)
 # DISS filter index 9 = cut off frequency 135 kHz (-3 dB)
 #
#**
 # \def		ETAL_DISS_FILTER_INDEX_MAX
 #			Max number of available DISS filter indexes
 #*
use constant ETAL_DISS_FILTER_INDEX_MAX           =>0x09;

#
 # TextInfo fields TODO move to etaltml
 #

#**
 # \def		ETAL_DEF_MAX_SERVICENAME
 # 			Max length of the Station Name (short form).
 # 			This is defined in the HD Radio specs (see 9.5.1.2) as
 # 			max 7 bytes, but for DAB it can be greater.
 #*
use constant ETAL_DEF_MAX_SERVICENAME     =>32;

#**
 # \def		ETAL_DEF_MAX_INFO_TITLE
 # 			Max length of the PSD tag for Title including the null termintation.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_TITLE      =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_ARTIST
 # 			Max length of the PSD tag for Artist including the null termintation.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_ARTIST     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO
 # 			The max length of the Textinfo.
 # 			The Textinfo is formatted as "title / artist"
 #*
use constant ETAL_DEF_MAX_INFO           =>ETAL_DEF_MAX_INFO_TITLE + 3 + ETAL_DEF_MAX_INFO_ARTIST;

#**
 # \def		ETAL_DEF_MAX_INFO_ALBUM
 # 			Max length of the PSD tag for Album including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_ALBUM     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_GENRE
 # 			Max length of the PSD tag for Genre including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_GENRE     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_COMMENT_SHORT
 # 			Max length of the PSD tag for Comment Short Content Description including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_COMMENT_SHORT     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_COMMENT
 # 			Max length of the PSD tag for Comment Actual Text including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0xFF, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_COMMENT     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_UFID
 # 			Max length of the PSD tag for UFID Owner Identifier Length including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0xFF, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_UFID     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_COMMERCIAL_PRICE
 # 			Max length of the PSD tag for Commercial Price String including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_COMMERCIAL_PRICE    =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_COMMERCIAL_CONTACT
 # 			Max length of the PSD tag for Commercial Contact URL including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_COMMERCIAL_CONTACT     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_COMMERCIAL_SELLER
 # 			Max length of the PSD tag for Commercial Name of Seller including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_COMMERCIAL_SELLER     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_COMMERCIAL_DESC
 # 			Max length of the PSD tag for Commercial Description including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0xFF, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_COMMERCIAL_DESC     =>64;

#**
 # \def		ETAL_DEF_MAX_INFO_XHDR
 # 			Max length of the PSD tag for XHDR including the null termination.
 # 			This is defined in the HD Radio specs (see Table 5-18)
 # 			as max 0x7F, it is limited due to ETAL buffer sizes
 #*
use constant ETAL_DEF_MAX_INFO_XHDR     =>64;

#
 # Device descriptions
 #

#**
 # \def		ETAL_FE_FOREGROUND
 # 			Identifier to be used for channel parameter 
 # 			of the #ETAL_MAKE_FRONTEND_HANDLE macro.
 # 			Identifies the foreground (or only, for single channel devices)
 # 			channel.
 # \remark	Used as array index, do not modify
 #*
use constant ETAL_FE_FOREGROUND            =>0;

#**
 # \def		ETAL_FE_BACKGROUND
 # 			Identifier to be used for channel parameter 
 # 			of the #ETAL_MAKE_FRONTEND_HANDLE macro.
 # 			Identifies the background channel.
 # \remark	Used as array index, do not modify
 #*
use constant ETAL_FE_BACKGROUND            =>1;

#**
 # \def		ETAL_CAPA_MAX_DATATYPE
 # 			Counts the number Main data types in #EtalBcastDataType
 #*
use constant ETAL_CAPA_MAX_DATATYPE      =>4;

#**
 # \def		deviceSingleFE
 # 			Broad Tuner category: identifies Tuners
 # 			with one frontend (or channel)
 #*
use constant deviceSingleFE            =>0x0001;

#**
 # \def		deviceTwinFE
 # 			Broad Tuner category: identifies Tuners
 # 			with two frontends (or channel)
 #*
use constant deviceTwinFE              =>0x0002;

# Sync status for Tune events
 #
 # Bit	Meaning
 # 0	set to 1 if station is found
 #      DAB: valid modulation present
 #      HD Radio: always 1
 # 1	set to 1 if station is not found
 # 2	set to 1 if sync acquired
 #      DAB: the Receiver is fully synchronized
 #      HD Radio: the signal contains HD contents and the Receiver is synchronized
 # 3	set to 1 if sync not acquired within a timeout
 # 4	set to 1 if sync complete
 #      DAB: the Receiver decoded the DAB MCI tables
 #      HD Radio: the Receiver acquired digital audio
 # 5	set to 1 if sync did not complete within a timeout
 # 6-31	Reserved
 #

#**
 # \def 	ETAL_TUNESTATUS_SYNCMASK_FOUND
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set if good
 #			signal was detected on the tuned frequency (any Broadcast Standard)
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_FOUND   			 =>0x0001;

#**
 # \def 	ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set if the synchronization for digital signal failed
 #			ie signal was not detected on the tuned frequency (any Broadcast Standard)
 #			for HD this is not applicable
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_NOT_FOUND   		 =>0x0002;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when digital radio synchronisation is reached
 #			signal is available on the tuned frequency 
 # 			set to 1 if sync acquired
 #			DAB: the Receiver is fully synchronized
 # 			HD Radio: the signal contains HD contents and the Receiver is synchronized
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED		 =>0x0004;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when digital radio synchronisation has failed
 #			set to 1 if sync not acquired within a timeout 
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_SYNC_FAILURE		 =>0x0008;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when full synchronisation of signal is reached
 #			DAB: the Receiver decoded the DAB MCI tables
 #			HD Radio: the Receiver acquired digital audio
 #			set to 1 if sync not acquired within a timeout 
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_COMPLETE			 =>0x0010;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when the SIS has been acquired
 #			HD Radio: the Receiver managed to acquired the SIS
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_SIS_ACQUIRED	 =>0x0020;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when the SIS has been acquired
 #			HD Radio: the Receiver did not managed to acquired the SIS !
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_SIS_FAILED		 =>0x0040;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when full synchronisation failed
 #			HD Radio: the Receiver did not manage to acquired digital audio
 #			set to 1 if sync not acquired within a timeout 
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_COMPLETION_FAILED	 =>0x0080;


# mapping define on DAB / HD status
#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set if SYNC
 #			was detected on the tuned frequency (DAB only)
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_DABSYNC		 =>ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set if MCI
 #			tables were decoded on the tuned frequency (DAB only)
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_DABMCI  		 =>ETAL_TUNESTATUS_SYNCMASK_COMPLETE;
#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when HD Radio
 #			signal is available on the tuned frequency (HD Radio only)
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_HD_SYNC	 	 =>ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED;

#**
 # 			The ETAL_TUNESTATUS_SYNCMASK_ may be used to access the
 # 			m_sync field of the #EtalTuneStatus structure.
 #			The bit identified by this macro is set when digital audio
 #			is available on the tuned frequency (HD Radio only)
 #*
use constant ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC 	 =>ETAL_TUNESTATUS_SYNCMASK_COMPLETE;

#
 # DAB defines
 #

use constant ETAL_DEF_MAX_ENSEMBLE                =>32;
use constant ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE    =>32;
use constant ETAL_DEF_MAX_SC_PER_SERVICE          =>32;

#**
 # \def		ETAL_DEF_MAX_LABEL_LEN
 #			DAB labels are 16 bytes long not null-terminated,
 # 			add one for the null-termination added in ETAL
 #*
use constant ETAL_DEF_MAX_LABEL_LEN               =>17;

#
 # RDS defines
 #

use constant STAR_RDS_MAX_BLOCK       =>20;

#
 # RDS Data
 #
use constant START_RDS_DATA_ERRCOUNT_MASK     =>0x700000;
use constant START_RDS_DATA_CTYPE_MASK        =>0x040000;
use constant START_RDS_DATA_BLOCKID_MASK      =>0x030000;
use constant START_RDS_DATA_MASK              =>0x00FFFF;

use constant START_RDS_DATA_ERRCOUNT_SHIFT    =>20;
use constant START_RDS_DATA_CTYPE_SHIFT       =>18;
use constant START_RDS_DATA_BLOCKID_SHIFT     =>16;

#
 # RDS Read Notification Register
 #
use constant STAR_RNR_DATARDY                 =>0x800000;
use constant STAR_RNR_SYNC                    =>0x400000;
use constant STAR_RNR_BOFL                    =>0x200000;
use constant STAR_RNR_BNE                     =>0x100000;
use constant STAR_RNR_RDS_deviation           =>0x0FFFF0;
use constant STAR_RNR_TUNCH2                  =>0x000002;
use constant STAR_RNR_TUNCH1                  =>0x000001;


#ifndef TRUE
    use constant TRUE                 =>1;
#endif
#ifndef true
    use constant true                 =>TRUE;
#endif

#ifndef FALSE
    use constant FALSE                =>0;
#endif
#ifndef false
    use constant false                =>FALSE;
#endif

#**********************************
 #
 # Macros
 #
 #*********************************

#**
 # \def		ETAL_MAKE_FRONTEND_HANDLE
 # 			Given a 'tuner index' (_t) and a 'tuner channel identifier' (_c)
 # 			returns a Frontend handle.
 # 			The tuner channel identifier should be
 # 			ETAL_FE_FOREGROUND for the foreground channel
 # 			ETAL_FE_BACKGROUND for the background channel
 # \remark	The macro does not validate its parameters
 # \remark	Do not use #etalChannelTy values to specify the
 # 			tuner channel identifier, the numerica values
 # 			are different and invalid handle (or valid one but
 # 			with unexpected frontend) might be created.
 #*
#define ETAL_MAKE_FRONTEND_HANDLE(_t, _c) ((0x05 << 13) | (((_t) & 0x1F) << 8) | ((_c) & 0xFF))

#**
 # \def		ETAL_FE_HANDLE_1
 # 			First Tuner channel (Foreground channel of the first Tuner device)
 # \remark	Defined for compatibility with legacy code
 #*
use constant ETAL_FE_HANDLE_1    => ((0x05 << 13) | ((0 & 0x1F) << 8) | (ETAL_FE_FOREGROUND & 0xFF));

#**
 # \def		ETAL_FE_HANDLE_2
 # 			Second Tuner channel (Background channel of the first Tuner device)
 # \remark	Defined for compatibility with legacy code
 #*
use constant ETAL_FE_HANDLE_2    => ((0x05 << 13) | ((0 & 0x1F) << 8) | (ETAL_FE_BACKGROUND & 0xFF));

#**
 # \def		ETAL_FE_HANDLE_3
 # 			Third Tuner channel (Foreground channel of the second Tuner device)
 # \remark	Defined for compatibility with legacy code
 #*
use constant ETAL_FE_HANDLE_3    => ((0x05 << 13) | ((1 & 0x1F) << 8) | (ETAL_FE_FOREGROUND & 0xFF));

#**
 # \def		ETAL_FE_HANDLE_4
 # 			Fourth Tuner channel (Background channel of the second Tuner device)
 # \remark	Defined for compatibility with legacy code
 #*
use constant ETAL_FE_HANDLE_4    => ((0x05 << 13) | ((1 & 0x1F) << 8) | (ETAL_FE_BACKGROUND & 0xFF));

#**********************************
 #
 # Types
 #
 #*********************************
#ifndef OSAL_PLAIN_TYPES_DEFINED
#define OSAL_PLAIN_TYPES_DEFINED
#typedef unsigned char           tBool;
#typedef unsigned char           tU8;
#typedef char                    tS8;
#typedef unsigned short          tU16;
#typedef short                   tS16;
#typedef unsigned int            tU32;
#typedef int                     tS32;
#typedef int                     tSInt;
#typedef float                   tF32;
#typedef char                    tChar;
# TKernel exception leading to warning : 
# put tVoid as void define
#ifdef CONFIG_HOST_OS_TKERNEL
#use constant tVoid					 =>void;
#else
#typedef void                    tVoid;
#endif
#endif

#**
 # \typedef	ETAL_HANDLE
 # 			System-wide unique identifier for Receiver, Datapath, Tuner, Monitor
 # 			or Frontend
 # 			ETAL ojects.
 #*
#typedef tU16 ETAL_HANDLE;

#**
 # \struct	ETAL_HINDEX
 # 			The 'index' bits of an #ETAL_HANDLE
 # \see		ETAL_handleMake
 #*
#typedef tU8 ETAL_HINDEX;

#**
 # \enum	EtalBcastDataType
 # 			Describes the ETAL Datapath data types. Some values
 # 			are also used in the #etalTuner to describe the
 # 			Front End capabilities.
 # \remark	Although these values are defined as a bitmap,
 # 			a Datapath can only have **one** Data Type.
 #*
use constant
{
	#** Undefined data type, used for uninitialized values or error returns 
	ETAL_DATA_TYPE_UNDEF        => 0x0000,

	#** Audio decoded on STAR
	 #  May be used in etalTuner's etalFrontendDescTy to indicate
	 #  a Front End part of a STAR device and thus capable of outputting
	 #**  decoded audio from AM/FM 
	ETAL_DATA_TYPE_AUDIO        => 0x0100,

	#** Audio decoded on DCOP
	 #  May be used in etalTuner's etalFrontendDescTy to indicate
	 #  a Front End part of a CMOST device not capable of outputting
	 #  decoded audio without DCOP connection
	 #  (e.g. a DOT used for an Host-based FM Receiver or a STAR used
	 #**  for a DAB Receiver) 
	ETAL_DATA_TYPE_DCOP_AUDIO   => 0x0200,

	#** DAB Data service (e.g. SLS, EPG, Journaline...), decoded by ETAL
	 #  May be used in etalTuner's etalFrontendDescTy in which case it implies:
	 #  - ETAL_DATA_TYPE_DAB_DATA_RAW
	 #  - ETAL_DATA_TYPE_DAB_AUDIO_RAW
	 #**  - ETAL_DATA_TYPE_DAB_FIC 
	ETAL_DATA_TYPE_DATA_SERVICE => 0x0407,
	#** DAB Data service, not decoded by ETAL 
	ETAL_DATA_TYPE_DAB_DATA_RAW => 0x0401,
	#** Not decoded audio 
	ETAL_DATA_TYPE_DAB_AUDIO_RAW=> 0x0402,
	#** Not decoded DAB FIC (Fast Information Channel) data 
	ETAL_DATA_TYPE_DAB_FIC      => 0x0404,

	#** Text Information (ETAL abstraction for DAB PAD, FM RDS and so on)
	 #  May be used in etalTuner's etalFrontendDescTy in which case it implies
	 #  (for FM Receivers only):
	 #  - ETAL_DATA_TYPE_FM_RDS
	 #**  - ETAL_DATA_TYPE_FM_RDS_RAW 
	ETAL_DATA_TYPE_TEXTINFO     => 0x0803,
	#** Decoded FM RDS information 
	ETAL_DATA_TYPE_FM_RDS       => 0x0801,
	#** Not decoded FM RDS information 
	ETAL_DATA_TYPE_FM_RDS_RAW   => 0x0802,

}; # EtalBcastDataType

#**
 # \enum	EtalDataServiceType
 # 			Describes the Data Service type, for DAB Data Services. Data Services
 # 			are carried over Datapaths of type ETAL_DATA_TYPE_DATA_SERVICE.
 # \remark	The code makes assumptions on these values, do not change them.
 #*
use constant
{
	ETAL_DATASERV_TYPE_NONE     => 0x00000000,
	ETAL_DATASERV_TYPE_EPG_RAW  => 0x00000001,
	ETAL_DATASERV_TYPE_SLS      => 0x00000002,
	ETAL_DATASERV_TYPE_SLS_XPAD => 0x00000004,
	ETAL_DATASERV_TYPE_TPEG_RAW => 0x00000008,
	ETAL_DATASERV_TYPE_TPEG_SNI => 0x00000010,
	ETAL_DATASERV_TYPE_SLI      => 0x00000020,
	ETAL_DATASERV_TYPE_EPG_BIN  => 0x00000040,
	ETAL_DATASERV_TYPE_EPG_SRV  => 0x00000080,
	ETAL_DATASERV_TYPE_EPG_PRG  => 0x00000100,
	ETAL_DATASERV_TYPE_EPG_LOGO => 0x00000200,
	ETAL_DATASERV_TYPE_JML_OBJ  => 0x00000400,
	ETAL_DATASERV_TYPE_FIDC     => 0x00000800,
	ETAL_DATASERV_TYPE_TMC      => 0x00001000,
	ETAL_DATASERV_TYPE_DLPLUS   => 0x00002000,
	ETAL_DATASERV_TYPE_PSD      => 0x00004000,
	ETAL_DATASERV_TYPE_UNDEFINED=> 0x7FFF8000, # for bitmap validity checks 
	ETAL_DATASERV_TYPE_ALL      => 0x7FFFFFFF
}; # EtalDataServiceType

#**
 # \enum	EtalBcastStandard
 #			Describes all possible Broadcast Standards supported by ETAL
 # \remark	Actually supported Broadcast Standards depend on a variety of
 # 			factors, for example the ETAL build configuration
 #*
use constant
{
	#** Undefined value used for not-initialized or error returns 
	ETAL_BCAST_STD_UNDEF => 0x0000,
	#** Digital Audio Broadcasting 
	ETAL_BCAST_STD_DAB   => 0x0001,
	#** Digital Radio Mondiale 
	ETAL_BCAST_STD_DRM   => 0x0002,
	#** Frequency Modulation 
	ETAL_BCAST_STD_FM    => 0x0004,
	#** Amplitude Modulation 
	ETAL_BCAST_STD_AM    => 0x0008,
	#** High Definition Radio FM 
	ETAL_BCAST_STD_HD_FM => 0x0010,
	#** **DEPRECATED** alias for ETAL_BCAST_STD_HD_FM, for backwards compatibility 
	ETAL_BCAST_STD_HD    => 0x0010,
	#** High Definition Radio AM 
	ETAL_BCAST_STD_HD_AM => 0x0020,
}; # EtalBcastStandard

#**
 # \enum	EtalFrequencyBand
 #			Lists the known frequency bands; the encoding of the values
 #			contains information on the broadcast standard.
 #*
use constant
{
	#** Undefined band, used for uninitialized values or error returns 
	ETAL_BAND_UNDEF         => 0x000000,
	#** Generic FM band          
	ETAL_BAND_FM            => 0x000001 | ETAL_BAND_FM_BIT, 
	#** Western Europe FM band   
	ETAL_BAND_FMEU          => 0x000002 | ETAL_BAND_FM_BIT,
	#** United States FM band    
	ETAL_BAND_FMUS          => 0x000004 | ETAL_BAND_FM_BIT | ETAL_BAND_HD_BIT,
	#** HDRadio FM band, alias for ETAL_BAND_FMUS 
	ETAL_BAND_HD            => 0x000004 | ETAL_BAND_FM_BIT | ETAL_BAND_HD_BIT,
	#** Japanese FM band         
	ETAL_BAND_FMJP          => 0x000010 | ETAL_BAND_FM_BIT,
	#** Eastern Europe FM band   
	ETAL_BAND_FMEEU         => 0x000020 | ETAL_BAND_FM_BIT,
	#** Weather Band             
	ETAL_BAND_WB            => 0x000040 | ETAL_BAND_FM_BIT,
	#** User-defined band limits 
	ETAL_BAND_USERFM        => 0x000080 | ETAL_BAND_FM_BIT,
	#** DRM Plus band            
	ETAL_BAND_DRMP          => 0x000100 | ETAL_BAND_DRM_BIT,
	#** DRM 3.0 band             
	ETAL_BAND_DRM30         => 0x000200 | ETAL_BAND_DRM_BIT,
	#** DAB Band 3               
	ETAL_BAND_DAB3          => 0x001000 | ETAL_BAND_DAB_BIT,
	#** DAB Band L               
	ETAL_BAND_DABL          => 0x002000 | ETAL_BAND_DAB_BIT,
	#** Generic AM band          
	ETAL_BAND_AM            => 0x010000 | ETAL_BAND_AM_BIT,
	#** Long Wave band           
	ETAL_BAND_LW            => 0x020000 | ETAL_BAND_AM_BIT,
	#** European Medium Wave band
	ETAL_BAND_MWEU          => 0x040000 | ETAL_BAND_AM_BIT,
	#** United States Medium Wave band 
	ETAL_BAND_MWUS          => 0x080000 | ETAL_BAND_AM_BIT | ETAL_BAND_HD_BIT,
	#** Short Wave band          
	ETAL_BAND_SW            => 0x100000 | ETAL_BAND_AM_BIT,
	#** Custom AM band           
	ETAL_BAND_CUSTAM        => 0x200000 | ETAL_BAND_AM_BIT,
	#** User-defined AM band     
	ETAL_BAND_USERAM        => 0x400000 | ETAL_BAND_AM_BIT
}; # EtalFrequencyBand

#**
 # \enum	EtalCountryVariant
 # 			The Country variant, for DAB DCOP initialization.
 #*
use constant
{
	#** Undefined variant 
	ETAL_COUNTRY_VARIANT_UNDEF => 0,
	#** Europe 
	ETAL_COUNTRY_VARIANT_EU=> 1,# warning generated value
	#** China, not supported 
	ETAL_COUNTRY_VARIANT_CHINA=> 2,# warning generated value
	#** Korea, not supported 
	ETAL_COUNTRY_VARIANT_KOREA=> 3,# warning generated value
}; # EtalCountryVariant

#**
 # \enum	EtalServiceSelectMode
 # \remark	Values used in command construction, do not change!
 #*
use constant
{
	ETAL_SERVSEL_MODE_SERVICE   => 0x00,
	ETAL_SERVSEL_MODE_DAB_SC    => 0x01,
	ETAL_SERVSEL_MODE_DAB_SUBCH => 0x02,
}; # EtalServiceSelectMode

#**
 # \enum	EtalServiceSelectSubFunction
 # \remark	Values used in command construction, do not change!
 #*
use constant
{
	ETAL_SERVSEL_SUBF_REMOVE => 0x01,
	ETAL_SERVSEL_SUBF_APPEND => 0x02,
	ETAL_SERVSEL_SUBF_SET    => 0x05
}; # EtalServiceSelectSubFunction

#**
 # \enum	ETAL_STATUS
 # 			Status returned by ETAL APIs
 #*
use constant
{
	#** No error 
	ETAL_RET_SUCCESS                 => 0,
	#** Generic error 
	ETAL_RET_ERROR                   => 1,
	#** #etal_initialize called on an already initialized system 
	ETAL_RET_ALREADY_INITIALIZED     => 2,
	#** Illegal (NULL) sink function specified for Datapath 
	ETAL_RET_DATAPATH_SINK_ERR       => 3,
	#** The Frontend:
	 # - does not exist in the system
	 # - is already used in another Receiver
	 # - cannot support the requested Broadcast Standard
	 # - Frontends from different Tuner devices where selected for diversity mode
	 #** - Is part of a Tuner that was disabled by etal_initialize 
	ETAL_RET_FRONTEND_LIST_ERR       => 4,
	#** The Frontend is not available for diversity mode 
	ETAL_RET_FRONTEND_NOT_AVAILABLE  => 5,
	#** The Broadcast Standard is not supported by the configuration 
	ETAL_RET_INVALID_BCAST_STANDARD  => 6,
	#** The data type / Broadcast Standard combination is not allowed 
	ETAL_RET_INVALID_DATA_TYPE       => 7,
	#** Invalid, non existing ETAL_HANDLE 
	ETAL_RET_INVALID_HANDLE          => 8,
	#** Invalid Receiver (e.g. not configured) 
	ETAL_RET_INVALID_RECEIVER        => 9,
	#** No data
	 # - no HD audio contents on the tuned frequency
	 #** - the requested service is not available 
	ETAL_RET_NO_DATA                 => 10,
	#** Communication failure with Tuner or DCOP 
	ETAL_RET_NO_HW_MODULE            => 11,
	#** #etal_initialize not yet called 
	ETAL_RET_NOT_INITIALIZED         => 12,
	#** Function not available because not implemented or not
	 #**  included in the build 
	ETAL_RET_NOT_IMPLEMENTED         => 13,
	#** A function was called with a wrong or illegal parameter value
	ETAL_RET_PARAMETER_ERR           => 14,
	#** A Quality container contains illegal values 
	ETAL_RET_QUAL_CONTAINER_ERR      => 15,
	#** Illegal Frontend for Quality request 
	ETAL_RET_QUAL_FE_ERR             => 16,
	#** The datapath for the Receiver is already configured and
	 #**  ETAL will not overwrite it 
	ETAL_RET_ALREADY_USED            => 17,
	#** The requested function cannot be executed due to some other function
	 #**  being in progress 
	ETAL_RET_IN_PROGRESS             => 18
}; # ETAL_STATUS

#**
 # \enum	ETAL_EVENTS
 # 			List all the events generated by ETAL
 #*
use constant
{
	# ETAL Core events 
	#** Tune event 
	ETAL_INFO_TUNE                    =>  0,
	# fm stereo/mono change event 
	ETAL_INFO_FM_STEREO               =>  1,
	#** Dynamic memory allocation failed 
	ETAL_WARNING_OUT_OF_MEMORY        => 10,
	#** Communication with Tuner or DCOP failure 
	ETAL_ERROR_COMM_FAILED            => 11,
	#** At least one device failed the Ping test 
	ETAL_RECEIVER_ALIVE_ERROR         => 12,

	# ETAL TML envents 
	#** Seek event 
	ETAL_INFO_SEEK                    => 20,
	#** Learn event 
	ETAL_INFO_LEARN                   => 21,
	#** Scan event 
	ETAL_INFO_SCAN                    => 22,
	#** Seamless estimation complete event 
	ETAL_INFO_SEAMLESS_ESTIMATION_END => 23,
	#** Seamless switching complete event 
	ETAL_INFO_SEAMLESS_SWITCHING_END  => 24,
	ETAL_INFO_TUNE_SERVICE_ID         => 25,
	ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO => 26,
	# RDS Strategy information
	ETAL_INFO_RDS_STRATEGY            => 27,
	# define an undef event
	# not applicable for others than msg (quality..)
	ETAL_INFO_UNDEF            		   => 28
}; # ETAL_EVENTS

#**
 # \struct	EtalAudioInterfTy
 # 			Parameter for the #etal_config_audio_path API
 #*
#typedef struct 
#{
	#** Enable / disable audio DAC 
#	tU8 m_dac:1;
	#** Enable / disable audio SAI output 
#	tU8 m_sai_out:1;
	#** Enable / disable audio SAI input 
#	tU8 m_sai_in:1;
	#** Reserved 
#	tU8 reserved:1;
	#** Enable / disable SAI Slave Mode 
#	tU8 m_sai_slave_mode:1;
	#** Unused bits, included for sanity checks 
#	tU8 unused:3;
#} EtalAudioInterfTy;

#**
 # \enum	EtalAudioSourceTy
 # 			Audio source selection for #etal_audio_select
 #*
use constant
{
	#** Automatic HD blending controlled via GPIO 
	ETAL_AUDIO_SOURCE_AUTO_HD           => 0x00, # 
	#** Select STAR input (AM/FM) 
	ETAL_AUDIO_SOURCE_STAR_AMFM         => 0x01, # 
	#** Select STA680 digital input (AM/FM HD) 
	ETAL_AUDIO_SOURCE_DCOP_STA680       => 0x02, # 
	#** HD alignment mode (left = analogue, right = digital) 
	ETAL_AUDIO_SOURCE_HD_ALIGN          => 0x03, # 
	#** DAB/DRM mode (digital input from STA660) 
	ETAL_AUDIO_SOURCE_DCOP_STA660       => 0x04  # 
}; # EtalAudioSourceTy

#**
 # \enum	etalSeekStatusTy
 #*
use constant
{
	ETAL_SEEK_STARTED=> 0,# warning generated value
	ETAL_SEEK_RESULT=> 1,# warning generated value
	ETAL_SEEK_FINISHED=> 2,# warning generated value
	ETAL_SEEK_ERROR=> 3,# warning generated value
	ETAL_SEEK_ERROR_ON_START=> 4,# warning generated value
	ETAL_SEEK_ERROR_ON_GET_STATUS=> 5,# warning generated value
	ETAL_SEEK_ERROR_ON_STOP => 6 # warning generated value
}; # etalSeekStatusTy

#**
 # \struct	EtalProcessingFeatures
 # 			Special features for #etal_change_band_receiver
 #*
#typedef struct
#{
	#**
	 # \union	u
	 # 			Union used to address the same bits
	 # 			as tU8 or as bitfield
	 #*
#	union
#	{
		#** #EtalProcessingFeaturesList can be used to fill this field, where:
		 # bit [0] 0: FM VPA off, 1: FM VPA on (FM only)
		 # bit [1] 0: antenna diversity off, 1: antenna diversity on (FM foreground channel only) (currently not implemented)
		 # bit [2] [3] reserved
		 # bit [4] HD radio / DRM digital BB interface and filter, 0: off, 1: on
		 # bit [5] HD radio onchip blending, 0: off, 1: on
		 #** bit [6] [7] reserved 
#		tU8 m_processing_features;
		#**
		 # \struct	bf
		 # 			Access the *m_processing_features* as bitfield
		 #*
#		struct
#		{
			#** bit [0] 0: FM VPA off, 1: FM VPA on (FM only) 
#			tU8 m_fm_vpa                     : 1;
			#** bit [1] 0: antenna diversity off, 1: antenna diversity on
			 #** (FM foreground channel only) (currently not implemented) 
#			tU8 m_antenna_diversity          : 1;
			#** bit [2] [3] reserved 
#			tU8 m_reserved_bit2_3            : 2;
			#** bit [4] HD radio / DRM digital BB interface and filter, 0: off, 1: on 
#			tU8 m_hd_radio_drm_digital_bb_if : 1;
			#** bit [5] HD radio onchip blending, 0: off, 1: on 
#			tU8 m_hd_radio_onchip_blending   : 1;
			#** bit [6] [7] reserved 
#			tU8 m_reserved_bit6_7            : 2;
#		} bf;
#	} u;
#} EtalProcessingFeatures;

#**
 # \enum	EtalProcessingFeaturesList
 # 			Defines values that can be used to initialize
 # 			the *m_processing_features* of #EtalProcessingFeatures
 #*
use constant
{
	#** Disable all processing features 
	ETAL_PROCESSING_FEATURE_NONE                        => 0x00,
	#** Enable FM VPA (FM only) 
	ETAL_PROCESSING_FEATURE_FM_VPA                      => 0x01,
	#** Enable antenna diversity (FM foreground channel only) (currently not implemented) 
	ETAL_PROCESSING_FEATURE_ANTENNA_DIVERSITY           => 0x02,
	#** Enable HD radio / DRM digital BB interface and filter 
	ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF  => 0x10,
 	#** Enable HD radio onchip blending 
	ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING    => 0x20,
	#** Enable all the above features except ETAL_PROCESSING_FEATURE_NONE 
	ETAL_PROCESSING_FEATURE_ALL                         => 0x33,
	#** No feature specified, use the defaults 
	ETAL_PROCESSING_FEATURE_UNSPECIFIED                 => 0xFF
}; # EtalProcessingFeaturesList

#typedef struct
#{
#	EtalDataServiceType	m_dataType;
#	tU8                 m_data;
#} EtalGenericDataServiceRaw;

#**
 # \enum	EtalTraceLevel
 # 			Describes the minimum trace level output by ETAL
 #*
use constant
{
	#** Fatal errors that prevent ETAL from running 
	ETAL_TR_LEVEL_FATAL      => 0,
	#** Non-fatal errors, ETAL continues with limited functionalities 
	ETAL_TR_LEVEL_ERROR      => 1,
	#** Warnings, Informative messages 
	ETAL_TR_LEVEL_SYSTEM_MIN => 2,
	#** More detailed messages 
	ETAL_TR_LEVEL_SYSTEM     => 3
}; # EtalTraceLevel

#**
 # \struct	EtalTraceConfig
 # 			Define the ETAL trace capabilities
 #*
#typedef struct
#{
	#** If set to TRUE ETAL does not prefix each message with the
	 #** time:level:class header 
#	tBool          m_disableHeader;
	#** if set to FALSE ETAL ignores the *m_disableHeader* field
	 #**  and uses the ibuilt-in default
#	tBool          m_disableHeaderUsed;
	#** The minimum trace level for messages to be printed: all
	 #  messages with #EtalTraceLevel equal or lower than this
	 #**  will be output 
#	EtalTraceLevel m_defaultLevel;
	#** If set to FALSE ETAL ignores the *m_defaultLevel* value
	 #**  and uses the built-in default 
#	tBool          m_defaultLevelUsed;
	#** Reserved, set to 0 
#	tU32           m_reserved;
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	#**	Number of trace filters to use 
#	tU32           m_filterNum;
	#** Class of the trace filters 
#	tU32           m_filterClass[CONFIG_OSUTIL_TRACE_NUM_FILTERS];
	#** Mask of the trace filters 
#	tU32           m_filterMask[CONFIG_OSUTIL_TRACE_NUM_FILTERS];
	#** Level of the trace filters 
#	tU32           m_filterLevel[CONFIG_OSUTIL_TRACE_NUM_FILTERS];
#endif
#} EtalTraceConfig;

#
 # Callbacks types for EtalHardwareAttr and friends
 #

#**
 # \typedef	EtalCbNotify
 # 			Pointer to function that ETAL will use for event communication
 #			pvContext - the pointer passed to etal_initialize through the EtalHardwareAttr.m_context
 #			dwEvent -   The event code
 #			pvParams -  The parameter of the event: the format depends on the event and is
 #			            described in ETAL specification.
 #*
#typedef void (*EtalCbNotify) (void * pvContext, ETAL_EVENTS dwEvent, void* pvParams);
#typedef int (*EtalCbGetImage) (void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap); 
#typedef void (*EtalCbPutImage) (void *pvContext, tU8* block, tU32 providedByteNum, tU32 remainingByteNum);

#**
 # \struct	EtalDDCOPAttr
 # 			Attributes for a DCOP device
 #*
#typedef struct 
#{ 
	#** If set to TRUE ETAL disables the DCOP; all other fields
	 #**  are ignored in this case 
#	tBool              m_isDisabled; 
#	tBool              m_doFlashDump; 
#	tBool              m_doFlashProgram; 
#	tBool              m_doDownload; 
#	EtalCbGetImage     m_cbGetImage; 
#	tVoid             *m_pvGetImageContext; 
#	EtalCbPutImage     m_cbPutImage; 
#	tVoid             *m_pvPutImageContext; 
#} EtalDCOPAttr;

#**
 # \struct	EtalTunerAttr
 # 			Attributes for a Tuner device
 #*
#typedef struct 
#{ 
	#** If set to TRUE ETAL disables the Tuner; all other fields
	 #**  are ignored in this case 
#	tBool              m_isDisabled;  
	#** If set to TRUE ETAL uses the XTAL alignment value specified
	 # in the *m_useXTALalignment* field; if FALSE the same field
	 #** is ignored 
#	tBool              m_useXTALalignment;
	#** The XTAL alignment value that ETAL programs into the Tuner
	 #  during the Tuner intialization procedure (if *m_useXTALalignment* 
	 #**  is set to TRUE) 
#	tU32               m_XTALalignment; 
	#** If set to TRUE ETAL programs the values contained in the
	 # *m_CustomParam* array into the Tuner device after initialization;
	 #** if FALSE the field *m_CustomParam* is ignored 
#	tU8                m_useCustomParam; 
	#** The size of the *m_CustomParam* parameter, in entries; each entry contains
	 #**  two integers, the parameter address and the parameter value 
#	tU32               m_CustomParamSize; 
	#** The custom parameters; the first integer is the parameter address,
	 #  the second one the parameter value, the third is the next parameter
	 #**  address, the fourth its value, and so on 
#	tU32              *m_CustomParam; 
	#** If set to TRUE ETAL uses the image contained in *m_DownloadImage* to
	 #  initialize the Tuner device, instead of using the embedded image;
	 #**  if set to FALSE ETAL ignores the next two parameters. 
#	tU8                m_useDownloadImage;
	#** The size in bytes of the *m_DownloadImage* array 
#	tU32               m_DownloadImageSize;
	#** The Tuner firmware image 
#	tU8               *m_DownloadImage;
#} EtalTunerAttr;

#**
 # \struct	EtalHardwareAttr
 # 			Describes the initialization parameters for ETAL
 # \see		etal_initialize
 #*
#typedef struct
#{
	#** Country in which the system is used (for DAB) 
#	EtalCountryVariant m_CountryVariant;
	#** The DCOP attributes 
#	EtalDCOPAttr       m_DCOPAttr;
	#** The Tuner attributes (one per max number of Tuners in the system) 
#	EtalTunerAttr      m_tunerAttr[ETAL_CAPA_MAX_TUNER];
	#** Pointer to User Notification handler (i.e. pointer to function) 
#	EtalCbNotify       m_cbNotify;
	#** Pointer to memory buffer that ETAL passes to the *m_cbNotify* function 
#	void              *m_context;
	#** Trace and log configuration 
#	EtalTraceConfig   m_traceConfig;
#} EtalHardwareAttr;

#**
 # \enum	EtalDeviceStatus
 # 			Provides details on a Device initialization
 # 			status.
 #*
use constant
{
	#** Reserved for entries which have not been processed
	 #  by ETAL yet. No information on device availability
	 #**  is present 
	deviceUninitializedEntry => 0,
	#** Support for this kind of device was not built in ETAL
	 #**  ETAL did not try to access the device at all 
	deviceNotSupported       => 1,
	#** etal_initialize() was told to ignore the device
	 #**  ETAL did not try to access the device at all 
	deviceDisabled           => 2,
	#** The device is present but has a silicon version different
	 #**  from the one expected by ETAL 
	deviceSiliconVersionError=> 3,
	#** Communication error 
	deviceCommunication      => 4,
	#** The silicon version matches but ETAL was unable to download
	 #**  the firmware to the device due to communication error or other 
	deviceDownload           => 5,
	#** The firmware was downloaded but there was an error writing
	 #**  the custom parameters 
	deviceParameters         => 6,
	#** The device was correctly initialized and firmware downloaded 
	 #**  but it did not respond to a test communication 
	devicePingError          => 7,
	#** Any other error 
	deviceError              => 0xFE,
	#** The device is ready for use 
	deviceAvailable          => 0xFF
}; # EtalDeviceStatus

#**
 # \enum	EtalNonFatalError
 # 			Provide details about the ETAL initialization
 # 			status, in particular lists the non-fatal errors
 # 			that may have occurres and result in reduced
 # 			ETAL functionality.
 #
 # 			These values may be provided OR'ed togather
 # 			to list multiple warnings.
 #*
use constant
{
	warningNone         => 0x0000,
	#** There was an error parsing the trace parameters,
	 #**  ETAL will use the defaults 
	warningTraceDefault => 0x0001,
	#** There was an error starting the trace system,
	 #**  tracing is not available 
	warningNoTrace      => 0x0002
}; # EtalNonFatalError

#**
 # \enum	EtalInitState
 # 			Lists the ETAL internal initialization states.
 # 			During #etal_initialize a state variable
 # 			is updated with the function progress, with
 # 			values taken from this enum.
 #
 # 			These values are useful only for STM debugging.
 #*
use constant
{
	#** #etal_initialize not yet called 
	state_initNotInitialized            => 0,
	#** preliminary checks (i.e. is ETAL already initialized?) 
	state_initStart                     => 1,
	#** OSAL initialization 
	state_OSALINIT                      => 2,
	#** ETAL print system initialization 
	state_tracePrintInit                => 3,
	#** ETAL global semaphores creation 
	state_statusInitLock                => 4,
	#** ETAL callback threads and semaphores creation 
	state_callbackInit                  => 5,
	#** ETAL datahandler threads and semaphores creation 
	state_datahandlerInit               => 6,
	#** CMOST communication startup (initialize the OS resources
	 #  for EMBEDDED driver, connect to the TCP/IP socket
	 #**  for EXTERNAL CMOST driver) 
	state_initCommunication_CMOST       => 7,
	#** CMOST check silicon version compatibility with embedded images 
	state_initCheckSiliconVersion_CMOST => 8,
	#** CMOST download firmware or patches 
	state_downloadFirmware_CMOST        => 9,
	#** ETAL program custom parameter into the Tuners 
	state_initCustomParameter_CMOST     => 10,
	#** ETAL initialize board-specific functionalities
	 #** (e.g. CMOST-to-DCOP clock) 
	state_boardInit                     => 11,
	#** IPFORWARD init, only entered for the external driver
	 #  implementation; this starts the TCP/IP communication
	 #**  with the external DCOP driver 
	state_ipforwardInit                 => 12,
	#** ETAL initialize OS resources for DAB DCOP communication
	 #**  (only for embedded DCOP driver) 
	state_initCommunication_MDR         => 13,
	#** ETAL initialize OS resources for HDRadio DCOP communication
	 #**  (only for embedded DCOP driver) 
	state_initCommunication_HDRADIO     => 14,
	#** ETAL read DCOP silicon version 
	state_initReadSiliconVersion_DCOP   => 15,
	#** ETAL start all the remaining threads 
	state_initThreadSpawn               => 16,
	#** ETAL test communication with the devices 
	state_initPing                      => 17,
	#** ETAL send commands to DCOP to put it in known state 
	state_statusExternalInit            => 18,
	#** ETALTML initialization (only if ETAL TML is included in the build) 
	state_ETALTML_init                  => 19,
	#** ETAL initialize the audio interface of the CMOST Tuner 
	state_AudioInterfaceConfigure       => 20,
	#** initialization completed successfully, all the m_deviceStatus
	 #  should be set to EtalDeviceInitStatus.deviceAvailable or
	 #  EtalDeviceInitStatus.deviceDisabled, possibly
	 #**  some warnings logged to EtalInitStatus.m_warningStatus 
	state_initComplete                  => 0xFF
}; # EtalInitState

#**
 # \struct	EtalDeviceInitStatus
 # 			Structure providing a Device (Tuner or DCOP)
 # 			initialization status.
 #*
#typedef struct
#{
	#** the device initialization status 
#	EtalDeviceStatus m_deviceStatus;
	#** the expected device silicon version. This is a string
	 #  with format dependent on the device. Currently supported
	 #**  only for Tuner devices 
#	tChar            m_expectedSilicon[ETAL_SILICON_VERSION_MAX]; 
	#** the detected device silicon version. This is a string
	 #  with format dependent on the device. Currently supported
	 #**  only for Tuner devices 
#	tChar            m_detectedSilicon[ETAL_SILICON_VERSION_MAX]; 
#} EtalDeviceInitStatus;

#**
 # \struct	EtalInitStatus
 #			Structure providing the ETAL initialization status
 #*
#typedef struct
#{
	#** The last state reached by #etal_initialize before exiting
	 #**  due to error 
#	EtalInitState         m_lastInitState;
	#** Non-fatal errors that occurred during ETAL initialization.
	 #  Indicate ETAL will operate with reduced functionalities.
	 #**  It is a bitmap of #EtalNonFatalError values 
#	tU32                  m_warningStatus;
	#** Tuner initialization status, one entry per Tuner 
#	EtalDeviceInitStatus  m_tunerStatus[ETAL_CAPA_MAX_TUNER];
	#** DCOP initialization status 
#	EtalDeviceInitStatus  m_DCOPStatus; 
#} EtalInitStatus;

#**
 # \struct	EtalReceiverAttr
 # 			The Receiver description
 #*
#typedef struct
#{
	#** The Broadcast Standard of the Receiver 
#	EtalBcastStandard    m_Standard;
	#** The number of Frontends configured in *m_FrontEnds* 
#	tU8                  m_FrontEndsSize;  # ignored, the value is inferred from the m_FrontEnds content
	#** The Frontends requested for this Receiver; may list more than
	 #**  one Frontend if some diversity mode is needed. 
#	ETAL_HANDLE          m_FrontEnds[ETAL_CAPA_MAX_FRONTEND];
	#** Processing features 
#	EtalProcessingFeatures    processingFeatures;
#} EtalReceiverAttr;

#**
 # \struct		EtalDataBlockStatusTy
 # 				Describes a data block status.
 #*
#typedef struct
#{
	#** Reserved, set to TRUE 
#	tBool m_isValid;
	#** DAB only: The data block was delivered with a properly formatted header
	 #  Note: this refers to the internal DCOP to ETAL protocol header, it
	 #**  says nothing about the data correctness. 
#	tBool m_validData;
	#** DAB only: There was a continuity error so some data blocks may be
	 #**  lost
#	tBool m_continuityError;
#} EtalDataBlockStatusTy;

#**
 # \typedef		EtalCbProcessBlock
 # 				Describes a data sink callback that is a function that ETAL calls
 # 				every time there is a data block ready for user processing.
 #				pBuffer - The pointer to the data buffer
 #				dwActualBufferSize - The number of data bytes in *pBuffer*
 #				status - The error status of the data block
 #				pvContext - the *m_context* parameter passed datapath creation to
 #				#etal_config_datapath
 #*
#typedef void (*EtalCbProcessBlock)(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

#**
 # \struct		EtalSink
 # 				Describes a Datapath sink
 #*
#typedef struct
#{
	#** Pointer to a user-defined memoy buffer (or NULL); ETAL blindly copies this
	 #**  field contents to the #EtalCbProcessBlock's *pvContext* parameter 
#	void*		       m_context;
	#** The size in bytes of the *m_CbProcessBlock* buffer 
#	tU32               m_BufferSize;
	#** Pointer to a the data block 
#	EtalCbProcessBlock m_CbProcessBlock;
#} EtalSink;

#**
 # \struct		EtalDataPathAttr
 # 				Describes a Datapath
 #*
#typedef struct
#{
	#** Handle of the Receiver to which this Datapath is attached 
#	ETAL_HANDLE	       m_receiverHandle;
	#** Type of data 
#	EtalBcastDataType  m_dataType;
	#** User function that shall process the data from this Datapath 
#	EtalSink           m_sink;
#} EtalDataPathAttr;

use constant
{
	ETAL_EPG_LOGO_TYPE_UNKNOWN=> 0,# warning generated value
	ETAL_EPG_LOGO_TYPE_UNRESTRICTED=> 1,# warning generated value
	ETAL_EPG_LOGO_TYPE_MONO_SQUARE=> 2,# warning generated value
	ETAL_EPG_LOGO_TYPE_COLOUR_SQUARE=> 3,# warning generated value
	ETAL_EPG_LOGO_TYPE_MONO_RECTANGLE=> 4,# warning generated value
	ETAL_EPG_LOGO_TYPE_COLOUR_RECTANGLE => 5 # warning generated value
}; # EtalDataService_EpgLogoType

#typedef struct
#{
#	tU8  m_ecc;
#	tU16 m_eid;
#	tU32 m_sid;
#	EtalDataService_EpgLogoType  m_logoType;
#	tU16 m_JMLObjectId;
#} EtalDataServiceParam;

#**
 # \struct	EtalPSDLength
 # 			This struct describes the PSD fields length
 #			it is used to configure or read the various lengths of PSD fields
 # 			through HD Radio DCOP commands :
 #			PSD_Decode (0x93) command, Set_PSD_Cnfg_Param (0x03) function
 #			PSD_Decode (0x93) command, Get_PSD_Cnfg_Param (0x04) function
 #*
#typedef struct
#{
#	tU8 m_PSDTitleLength;
#	tU8 m_PSDArtistLength;
#	tU8 m_PSDAlbumLength;
#	tU8 m_PSDGenreLength;
#	tU8 m_PSDCommentShortLength;
#	tU8 m_PSDCommentLength;
#	tU8 m_PSDUFIDLength;
#	tU8 m_PSDCommercialPriceLength;
#	tU8 m_PSDCommercialContactLength;
#	tU8 m_PSDCommercialSellerLength;
#	tU8 m_PSDCommercialDescriptionLength;
#	tU8 m_PSDXHDRLength;
#} EtalPSDLength;

#**********************************
 #
 # Capabilities
 #
 #*********************************
#**
 # \enum 	EtalDeviceType
 # 			Describes all types of Devices (Tuner, DCOP) that could be present in the system.
 # 			The encoding includes the number of channels supported by the device.
 # 			Macros are available to decode the EtalDeviceType values
 #*
#typedef enum
#{
	#** Undefined device                   
use constant	deviceUnknown=>               0x0000;
	#** Generic CMOST (STAR or DOT) device 
use constant	deviceCMOST  =>               0x0080;
	#** Generic STAR device                
use constant	deviceSTAR   => deviceCMOST | 0x0040;
	#** STAR single channel device         
use constant	deviceSTARS  => deviceSTAR  | deviceSingleFE;
	#** STAR dual channel device           
use constant	deviceSTART  => deviceSTAR  | deviceTwinFE;
	#** Generic DOT device                 
use constant	deviceDOT    => deviceCMOST | 0x0020;
	#** DOT single channel device          
use constant	deviceDOTS   => deviceDOT   | deviceSingleFE;
	#** DOT dual channel device            
use constant	deviceDOTT   => deviceDOT   | deviceTwinFE;
	#** Generic DCOP device                
use constant	deviceDCOP   =>               0x8000;
	#** DAB DCOP device                    
use constant	deviceMDR    => deviceDCOP  | 0x4000;
	#** HDRadio DCOP device                
use constant	deviceHD     => deviceDCOP  | 0x2000;
#} EtalDeviceType;

#**
 # \enum	EtalDeviceBus
 # 			Describes the type of communication bus used by a hardware device.
 #*
use constant
{
	ETAL_BusI2C=> 0,# warning generated value
	ETAL_BusSPI => 1 # warning generated value
}; # EtalDeviceBus

#**
 # \struct	EtalDeviceDesc
 # 			Hardware device description
 #*
#typedef struct
#{
	#** The Device type, i.e. STAR, DOT or DAB DCOP 
	 #  Set to NONE to indicate an unused entry (i.e. a Device potentially 
	 #**  supported by ETAL but not currently enabled). 
#	EtalDeviceType    m_deviceType;
	#** The hardware bus on which the Device is connected (I2C or SPI). 
	 #  Note: this information is provided only to easily associate the 
	 #**  hardware device to ETAL's Tuner index if required in the ETAL application.  
#	EtalDeviceBus     m_busType;
	#** The hardware address of the Device. 
	 #  Note: this information is provided only to easily associate the 
	 #**  hardware device to ETAL's Tuner index if required in the ETAL application. 
#	tU8               m_busAddress;
	#** The number of channels supported by the Device
	 #**  For DCOP this is the number of applications (DAB) or instances (HD Radio) 
#	tU8               m_channels;
#} EtalDeviceDesc;

#**
 # \struct	EtalTuner
 # 			Tuner capabilities
 #*
#typedef struct 
#{
	#** The device description 
#	EtalDeviceDesc    m_TunerDevice;
	#** One dimensional array describing for each Frontend the supported
	 #  Broadcast Standards. Each entry is a bitmap where each bit corresponds to 
	 #  one of the values listed in #EtalBcastStandard; if the bit is set the
	 #**  Frontend supports the standard 
#	tU32              m_standards[ETAL_CAPA_MAX_FRONTEND_PER_TUNER]; 
	#** One dimensional array describing for each Frontend the supported
	 #  data types. Each entry is a bitmap describing the Broadcast Standards
	 #  supported by the Tuner.  Each bit in the bitmap corresponds to one of the
	 #  data types listed #EtalBcastDataType and if set indicates the data
	 #**  type is supported 
#	tU32              m_dataType[ETAL_CAPA_MAX_FRONTEND_PER_TUNER];
#} EtalTuner;

#**
 # \struct	EtalHwCapabilities
 # 			Overall ETAL capabilities
 #*
#typedef struct
#{
	#** Describes the DCOP present in the system. If not present or not
	 #**  enabled the EtalDeviceDesc.m_deviceType is set to NONE 
#	EtalDeviceDesc        m_DCOP;
	#** Array listing the Tuners present in the system. The array has a fixed 
	 #  size, not all entries might be populated depending on the current
	 #  system configuration (see m_deviceType).
	 #  ETAL's Tuner index corresponds to the position in this array and
	 #  uniquely identifies the Tuner device in the system for e.g. custom
	 #**  parameter download. 
#	EtalTuner         m_Tuner[ETAL_CAPA_MAX_TUNER]; 
#} EtalHwCapabilities;


#**********************************
 #
 # Quality
 #
 #*********************************
#**
 # \enum	EtalBcastQaIndicators
 # 			Quality indicators usable to define quality monitors
 #			though the #etal_config_reception_quality_monitor API.
 #*
use constant
{
	#** Reserved for unused or illegal entries 
	EtalQualityIndicator_Undef => 0,
	#** DAB FIC error ratio
	EtalQualityIndicator_DabFicErrorRatio=> 1,# warning generated value
	#** DAB Field strength (measured by the DCOP) 
	EtalQualityIndicator_DabFieldStrength=> 2,# warning generated value
	#** DAB MSC Bit Error Ratio 
	EtalQualityIndicator_DabMscBer=> 3,# warning generated value
	#** AM/FM field strength 
	EtalQualityIndicator_FmFieldStrength=> 4,# warning generated value
	#** AM/FM Frequency offset 
	EtalQualityIndicator_FmFrequencyOffset=> 5,# warning generated value
	#** FM Deviation
	 #**  AM Modulation 
	EtalQualityIndicator_FmModulationDetector=> 6,# warning generated value
	#** FM Multipath 
	EtalQualityIndicator_FmMultipath=> 7,# warning generated value
	#** FM Ultrasonic noise 
	EtalQualityIndicator_FmUltrasonicNoise=> 8,# warning generated value
	#** HD Digital audio Quality Indicator 
	EtalQualityIndicator_HdQI=> 9,# warning generated value
	#** HD Digital power to noise power ratio 
	EtalQualityIndicator_HdCdToNo=> 10,# warning generated value
	#** HD Digital Signal Quality Meter 
	EtalQualityIndicator_HdDSQM => 11 # warning generated value
}; # EtalBcastQaIndicators

#**
 # \struct	EtalQaMonitoredEntryAttr
 # 			Describes a single quality attribute to be checked
 # 			in a Quality Monitor.
 #*
#typedef struct
#{
	#** The Quality indicator to monitor 
#	EtalBcastQaIndicators  m_MonitoredIndicator;
	#** The lower limit on the indicator; if the measure is
	 #  less than or equal, the monitor callback is invoked.
	 #**  Set to ETAL_INVALID_MONITOR for 'don't care' 
#	tS32                   m_InferiorValue;
	#** The upper limit on the indicator; if the measure is
	 #  more than or equal, the monitor callback is invoked.
	 #**  Set to ETAL_INVALID_MONITOR for 'don't care' 
#	tS32                   m_SuperiorValue;
	#** The polling interval in milliseconds 
#	tU32                   m_UpdateFrequency;
#} EtalQaMonitoredEntryAttr;

#
 # EtalDabQualityEntries
 #
#typedef struct
#{
#	tS32 m_RFFieldStrength;    # DAB B3: fstRF channel quality from CMOST 
#	tS32 m_BBFieldStrength;    # measured by the DCOP 
#	tU32 m_FicBitErrorRatio;
#	tU32 m_MscBitErrorRatio;
#	tBool m_isValidMscBitErrorRatio;
#ifdef	CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#	tU32 m_audio_ber;
#	tU8  m_audio_ber_level;
#	tU8  m_reed_solomon_information;
#	tU8  m_sync_status;
#	tBool m_mute_flag;
#endif
#} EtalDabQualityEntries;

#**
 # \struct	EtalFmQualityEntries
 # 			Quality container for AM/FM
 # \details	For each field it is indicated the corresponding CMOST
 # 			quality parameter, as obtained with the TUNER_Get_Reception_Quality or
 # 			TUNER_Get_Channel_Quality CMOST commands. This is just for reference,
 # 			the ETAL APIs do not need to export this information.
 #*
#typedef struct
#{
	#** RF Field strength
	 #** CMOST corrispondent: AM,FM,WX: fstRF 
#	tS32 m_RFFieldStrength;
	#** Baseband Field Strength
	 #**  CMOST corrispondent: AM,FM,WX: fstBB 
#	tS32 m_BBFieldStrength;
	#** Frequency offset
	 #**  CMOST corrispondent: AM,FM,WX: det 
#	tU32 m_FrequencyOffset;
	#** Modulatio detector
	 #**  CMOST corrispondent: FM: dev; AM: mod 
#	tU32 m_ModulationDetector;
	#** Multipath
	 #**  CMOST corrispondent: FM: mp
#	tU32 m_Multipath;
	#** Ultrasonic noise
	 #**  CMOST corrispondent: FM: MPX
#	tU32 m_UltrasonicNoise;
	#** Adjuatent channel
	 #**  CMOST corrispondent: AM,FM: adj 
#	tS32 m_AdjacentChannel;
	#** Signal To Noise Ratio
	 #**  CMOST corrispondent: AM,FM: snr 
#	tU32 m_SNR;
	#** Co Channel
	 #**  CMOST corrispondent: FM: coch 
#	tU32 m_coChannel;
	#** Stereo/Mono reception indicator 
#	tU32 m_StereoMonoReception;
#} EtalFmQualityEntries;

#**
 # \struct	EtalHdQualityEntries
 # 			Quality container for HDRadio
 # \details	Contains HD-only quality entries, plus the analogue
 # 			quality (measured by the CMOST on the same frequency)
 #*
#typedef struct
#{
	#** TRUE if there is HDRadio contents on the tuned frequency;
	 #  if FALSE the *m_QI*, *m_CdToNo*, *m_DSQM* and *m_AudioAlignment*
	 #**  fields should be ignored 
#	tBool m_isValidDigital;
	#** Digital Audio Quality Indicator, ranges 0..15  
#	tU32  m_QI;
	#** Digital power to noise power ratio 
#	tU32  m_CdToNo;
	#** Digital Signal Quality Meter
	 #  ranges 0x0000 (0.0) to 0xFFFF (1.0); divide by #ETAL_HDRADIO_DSQM_DIVISOR
	 #**  to obtain the ratio 
#	tU32  m_DSQM;
	#** TRUE if the DCOP obtained audio aligment (only for AAA-enabled DCOP versions) 
#	tBool m_AudioAlignment;
	#** The Analogue signal quality measured on the same frequency by the CMOST 
#	EtalFmQualityEntries m_analogQualityEntries;
#} EtalHdQualityEntries;

#**
 # \struct	EtalBcastQualityContainer
 # 			Generic, Broadcast Standard-independent quality container
 #*
#typedef struct
#{
	#** The time at which the quality was measured
	 #  This is the time in milliseconds from the initialization
	 #**  of ETAL 
#	tU32              m_TimeStamp;
	#** The Broadcast Standard for this container 
#	EtalBcastStandard m_standard;
	#** The m_context passed to #etal_config_reception_quality_monitor
	 #**  through parameter EtalBcastQualityMonitorAttr 
#	tVoid             *m_Context;

	#**
	 # \union	EtalQualityEntries
	 # 			The Quality measurements
	 #*
#	union
#	{
		#** Quality for DAB 
#		EtalDabQualityEntries dab;
		#** Quality for AM or FM 
#		EtalFmQualityEntries  amfm;
		#** Quality for HDradio 
#		EtalHdQualityEntries  hd;
#	} EtalQualityEntries;
#} EtalBcastQualityContainer;

#**
 # \struct	EtalBcastQualityMonitorAttr
 # 			The Quality Monitor attributes that is the
 # 			description of a quality monitor passed to
 # 			#etal_config_reception_quality_monitor
 #*
#typedef struct
#{
	#** Handle of the Receiver to which the Monitor is attached 
#	ETAL_HANDLE              m_receiverHandle;
	#** List of quality attributes to monitor 
#	EtalQaMonitoredEntryAttr m_monitoredIndicators[ETAL_MAX_QUALITY_PER_MONITOR];
	#** Pointer to user-defined buffer that ETAL will pass as-is
	 #**  to the Monitor callback. May be NULL 
#	tVoid                    *m_Context;
	#** Pointer to the function that ETAL will call every time one of the
	 #**  monitored entries thresholds listed in *m_monitoredIndicators* is passed 
#	tVoid (*m_CbBcastQualityProcess)( EtalBcastQualityContainer* pQuality, void* vpContext );
#} EtalBcastQualityMonitorAttr;

#**
 # \struct	EtalCFDataContainer
 # 			Current Frequency quality container.
 #** 			This container is used by the #etal_get_CF_data API 
#typedef struct
#{
	#** The frequency on which the quality was measured 
#	tU32			 			m_CurrentFrequency;
	#** The frequency band on which the Tuner is working 
#	EtalFrequencyBand 			m_CurrentBand;
	#** The quality measure 
#	EtalBcastQualityContainer   m_QualityContainer;
#} EtalCFDataContainer;

#**********************************
 #
 # Events
 #
 #*********************************

#**
 # \struct	EtalTuneStatus
 # 			Parameter for the ETAL_INFO_TUNE event
 #*
#typedef struct
#{
	#** Handle of the Receiver generating the event 
#	ETAL_HANDLE              m_receiverHandle;
	#** Frequeny on which the Receiver is tuned, in Hz 
#	tU32                     m_stopFrequency;
	#** Tune status; see #ETAL_TUNESTATUS_SYNCMASK_FOUND and following 
#	tU32                     m_sync;
	#** The service ID; applies only to HDRadio and DAB, and only if ETAL TML is available.
	 #**  In all other cases it is should be ignored 
#	tS8						 m_serviceId;
#} EtalTuneStatus;

#**
 # \struct	EtalSeekStatus
 # 			Parameter for the ETAL_INFO_SEEK event (ETAL TML only)
 # 			and for the #etal_seek_get_status_manual API
 #*
#typedef struct
#{
	#** Handle of the Receiver generating the event 
#    ETAL_HANDLE                 m_receiverHandle;
	#** Seek status 
#    etalSeekStatusTy            m_status;
	#** Current frequency in Hz 
#    tU32                        m_frequency;
	#** TRUE if the automatic seek operation stopped on
	 #  a frequency containing good signal quality
	 #**  Valid only for ETAL TML and for automatic seek commands 
#    tBool                       m_frequencyFound;
	#** TRUE if the automatic seek operation stopped on
	 #  a frequency containing good HDRadio signal quality
	 #  Valid only for ETAL TML and for automatic seek commands 
	 #**  issued to HDRadio Receivers 
#	tBool                       m_HDProgramFound;
	#** TRUE if the seeo operation operation wrapped around the whole
	 #  frequency band and returned to the frequency on which the seek
	 #**  started 
#    tBool                       m_fullCycleReached;
	#** Service Id , applies only to ETAL TML 
#	tS8						    m_serviceId;
	#** Quality measured on the seeked frequency.
	 #**  Valid ony for ETAL TML and for automatic seek commands 
#    EtalBcastQualityContainer   m_quality;
#} EtalSeekStatus;

#**
 # \enum		EtalCommErr
 # 				Decoded Communication Error event
 #*
use constant
{
	#** No error 
	EtalCommStatus_NoError=> 0,# warning generated value
	#** Error detected during checksum or CRC check in reception 
	EtalCommStatus_ChecksumError=> 1,# warning generated value
	#** Did not receive expected response within timeout 
	EtalCommStatus_TimeoutError=> 2,# warning generated value
	#** Unrecognized Protocol Header format 
	EtalCommStatus_ProtocolHeaderError=> 3,# warning generated value
	#** Message (command or response) format error 
	EtalCommStatus_MessageFormatError=> 4,# warning generated value
	#** Error accessing the hardware bus connected to the device 
	EtalCommStatus_BusError=> 5,# warning generated value
	#** Packets lost, or missed 
	EtalCommStatus_ProtocolContinuityError=> 6,# warning generated value
	#** Everything else 
	EtalCommStatus_GenericError => 7 # warning generated value
}; # EtalCommErr

#**
 # \struct		EtalCommErrStatus
 # 				Detailed description of the status of the device
 # 				that originated the Communication Error event
 #*
#typedef struct
#{
	#**
	 # The decoded error event
	 #*
#	EtalCommErr m_commErr;
	#**
	 # The raw error status returned by the device:
	 #  - for DAB DCOP the 'Status Code' described in section 11 of Digital_Radio_CoProcessor_Middleware.pdf
	 #  - for CMOST the MSByte of the Command Header described in section 1.2.1 of TDA770X_STAR_API_IF.pdf
	 #  - for HD DCOP the Logical Message Status field described in section 6.2.2.7 of RX_IDD_2206_HDRadio*_Command_and_Data_Interface_Definition.pdf
	 #  If set to 0 the raw error status is not available.
	 #*
#	tU32 m_commErrRaw;
	#**
	 # The Receiver which generated the event, or ETAL_INVALID_HANDLE if not available
	 #*
#	ETAL_HANDLE m_commErrReceiver;
	#**
	 # The communication message that caused the event; this field is present only if
	 # ETAL_MAX_COMM_ERR_MSG is greater than 0
	 #*
#	tU8 m_commErrBuffer[ETAL_MAX_COMM_ERR_MSG];
	#**
	 # The size of the m_deviceCommBuffer, in bytes;
	 # - 0 if the message is not available or does not apply or this functionality was
	 # disabled;
	 # - the size is negative if the message was truncted to fit the available buffer
	 # size of ETAL_MAX_COMM_ERR_MSG bytes 
	 #*
#	tS32 m_commErrBufferSize;
#} EtalCommErrStatus;

#**
 # \struct	EtalStereoStatus
 # 			Parameter for the ETAL_INFO_FM_STEREO event
 #*
#typedef struct
#{
	#** Handle of the Receiver generating the event 
#	ETAL_HANDLE  m_hReceiver;
	#** TRUE if the reception is in Stereo 
#	tBool        m_isStereo;
#} EtalStereoStatus;

#**********************************
 #
 # System Data	
 #
 #*********************************
#typedef struct
#{
#	tU8         m_ECC;
#	tU32        m_ensembleId;
#	tU32        m_frequency;
#} EtalEnsembleDesc;

#
 # EtalEnsembleList
 #
#typedef struct
#{
#	tU32	    m_ensembleCount;
#	EtalEnsembleDesc m_ensemble[ETAL_DEF_MAX_ENSEMBLE];
#} EtalEnsembleList;

#
 # EtalServiceList
 #
#typedef struct
#{
#	tU32        m_serviceCount;
#	tU32        m_service[ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE];
#} EtalServiceList;

#
 # EtalServiceInfo
 #
#typedef struct
#{
#	tU16        m_serviceBitrate;
#	tU8         m_subchId;
#	tU16        m_packetAddress;
#	tU8         m_serviceLanguage;
#	tU8         m_componentType;
#	tU8         m_streamType;
#	tU8         m_scCount;
#	tU8         m_serviceLabelCharset;
#	tChar       m_serviceLabel[ETAL_DEF_MAX_LABEL_LEN];
#	tU16        m_serviceLabelCharflag;
#} EtalServiceInfo;

#
 # EtalSCInfo
 #
#typedef struct
#{
#	tU8         m_scIndex;
#	tU8         m_dataServiceType;
#	tU8         m_scType;
#	tU8         m_scLabelCharset;
#	tChar       m_scLabel[ETAL_DEF_MAX_LABEL_LEN];
#	tU16        m_scLabelCharflag;
#} EtalSCInfo;

#
 # EtalServiceComponentList
 #
#typedef struct
#{
#	tU8         m_scCount;
#	EtalSCInfo  m_scInfo[ETAL_DEF_MAX_SC_PER_SERVICE];
#} EtalServiceComponentList;

#**********************************
 #
 # Read / write parameters
 #
 #*********************************
#**
 # \enum	etalReadWriteIndexTy
 # 			The list of CMOST registers accessible though the
 # 			etal_read_parameter/etal_write_parameter interfaces.
 # 			Actual availability of some registers depends on the CMOST
 # 			flavour (single vs dual channel) and also on the
 # 			CMOST silicon and firmware version.
 #*
use constant
{
	# FM foreground 
	IDX_CMT_tunApp0_fm_wsp_smLevShp => 0,
	IDX_CMT_tunApp0_fm_wsp_smMpShp=> 1,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_smDistShp=> 2,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_sm=> 3,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_sbLevShp=> 4,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_sbMpShp=> 5,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_sbDistShp=> 6,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_sb=> 7,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hcLevShp=> 8,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hcMpShp=> 9,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hcDistShp=> 10,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hc=> 11,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hbLevShp=> 12,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hbMpShp=> 13,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hbDistShp=> 14,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hb=> 15,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_smDistProc=> 16,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_sbDistProc=> 17,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hcDistProc=> 18,# warning generated value
	IDX_CMT_tunApp0_fm_wsp_hbDistProc=> 19,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_fstLog=> 20,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_fstBB=> 21,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_fstRF=> 22,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_detune=> 23,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_mpxNoise=> 24,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_mp=> 25,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_adj=> 26,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_snr=> 27,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_coChannel=> 28,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_deviation=> 29,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_compressRegQ0=> 30,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_compressRegQ1=> 31,# warning generated value
	IDX_CMT_tunApp0_fm_qdR_compressRegQ2=> 32,# warning generated value
	IDX_CMT_tunApp0_fm_qd_fstLog=> 33,# warning generated value
	IDX_CMT_tunApp0_fm_qd_fstBB=> 34,# warning generated value
	IDX_CMT_tunApp0_fm_qd_fstRF=> 35,# warning generated value
	IDX_CMT_tunApp0_fm_qd_detune=> 36,# warning generated value
	IDX_CMT_tunApp0_fm_qd_detuneFast=> 37,# warning generated value
	IDX_CMT_tunApp0_fm_qd_mpxNoise=> 38,# warning generated value
	IDX_CMT_tunApp0_fm_qd_mp=> 39,# warning generated value
	IDX_CMT_tunApp0_fm_qd_adj=> 40,# warning generated value
	IDX_CMT_tunApp0_fm_qd_snr=> 41,# warning generated value
	IDX_CMT_tunApp0_fm_qd_deviation=> 42,# warning generated value
	IDX_CMT_tunApp0_fm_qd_compressRegQ0=> 43,# warning generated value
	IDX_CMT_tunApp0_fm_qd_compressRegQ1=> 44,# warning generated value
	IDX_CMT_tunApp0_fm_qd_compressRegQ2=> 45,# warning generated value
	# FM background 
	IDX_CMT_tunApp1_fm_qd_fstLog=> 46,# warning generated value
	IDX_CMT_tunApp1_fm_qd_fstBB=> 47,# warning generated value
	IDX_CMT_tunApp1_fm_qd_fstRF=> 48,# warning generated value
	IDX_CMT_tunApp1_fm_qd_detune=> 49,# warning generated value
	IDX_CMT_tunApp1_fm_qd_detuneFast=> 50,# warning generated value
	IDX_CMT_tunApp1_fm_qd_mpxNoise=> 51,# warning generated value
	IDX_CMT_tunApp1_fm_qd_mp=> 52,# warning generated value
	IDX_CMT_tunApp1_fm_qd_adj=> 53,# warning generated value
	IDX_CMT_tunApp1_fm_qd_snr=> 54,# warning generated value
	IDX_CMT_tunApp1_fm_qd_deviation=> 55,# warning generated value
	IDX_CMT_tunApp1_fm_qd_compressRegQ0=> 56,# warning generated value
	IDX_CMT_tunApp1_fm_qd_compressRegQ1=> 57,# warning generated value
	IDX_CMT_tunApp1_fm_qd_compressRegQ2=> 58,# warning generated value
	IDX_CMT_tunApp0_fm_qdAf_compressRegQ0=> 59,# warning generated value
	IDX_CMT_tunApp0_fm_qdAf_compressRegQ1=> 60,# warning generated value
	IDX_CMT_tunApp0_fm_qdAf_compressRegQ2=> 61,# warning generated value
	IDX_CMT_tunApp1_fm_qdAf_compressRegQ0=> 62,# warning generated value
	IDX_CMT_tunApp1_fm_qdAf_compressRegQ1=> 63,# warning generated value
	IDX_CMT_tunApp1_fm_qdAf_compressRegQ2=> 64,# warning generated value
	# AM foreground 
	IDX_CMT_tunApp0_am_wsp_smLevWght=> 65,# warning generated value
	IDX_CMT_tunApp0_am_wsp_smDistWght=> 66,# warning generated value
	IDX_CMT_tunApp0_am_wsp_sm=> 67,# warning generated value
	IDX_CMT_tunApp0_am_wsp_lcLevWght=> 68,# warning generated value
	IDX_CMT_tunApp0_am_wsp_lcDistWght=> 69,# warning generated value
	IDX_CMT_tunApp0_am_wsp_lc=> 70,# warning generated value
	IDX_CMT_tunApp0_am_wsp_hcLevWght=> 71,# warning generated value
	IDX_CMT_tunApp0_am_wsp_hcDistWght=> 72,# warning generated value
	IDX_CMT_tunApp0_am_wsp_hc=> 73,# warning generated value
	IDX_CMT_tunApp0_am_qd_fstLog=> 74,# warning generated value
	IDX_CMT_tunApp0_am_qd_fstBB=> 75,# warning generated value
	IDX_CMT_tunApp0_am_qd_fstRF=> 76,# warning generated value
	IDX_CMT_tunApp0_am_qd_detune=> 77,# warning generated value
	IDX_CMT_tunApp0_am_qd_adj=> 78,# warning generated value
	IDX_CMT_tunApp0_am_qd_modulation=> 79,# warning generated value
	IDX_CMT_tunApp0_am_qd_compressRegQ0=> 80,# warning generated value
	IDX_CMT_tunApp0_am_qd_compressRegQ2=> 81,# warning generated value
	# AM background 
	IDX_CMT_tunApp1_am_qd_fstLog=> 82,# warning generated value
	IDX_CMT_tunApp1_am_qd_fstBB=> 83,# warning generated value
	IDX_CMT_tunApp1_am_qd_fstRF=> 84,# warning generated value
	IDX_CMT_tunApp1_am_qd_detune=> 85,# warning generated value
	IDX_CMT_tunApp1_am_qd_adj=> 86,# warning generated value
	IDX_CMT_tunApp1_am_qd_modulation=> 87,# warning generated value
	IDX_CMT_tunApp1_am_qd_compressRegQ0=> 88,# warning generated value
	IDX_CMT_tunApp1_am_qd_compressRegQ2=> 89,# warning generated value

	# needed for sanity checks: assumes the
	 # above identifier list start from 0
	 #* and none of them is explicitly assigned 
	ETAL_IDX_CMT_MAX_EXTERNAL => 91 # warning generated value
}; # etalReadWriteIndexTy

#**
 # \enum	etalReadWriteModeTy
 # 			Parameter for the  #etal_read_parameter
 # 			and #etal_write_parameter APIs
 #*
use constant
{
	#** Indicates the array parameter
	 #**  contains indexes 
	fromIndex=> 0,# warning generated value
	#** Indicates the array parameter
	 #**  contains absolute addresses 
	fromAddress => 1 # warning generated value
}; # etalReadWriteModeTy

#**
 # \enum	etalAFModeTy
 # 			Parameter for the #etal_AF_start function
 #*
use constant
{
	#** The function should continue a measurement
	 #**  previously started by another call to etal_AF_start 
	cmdNormalMeasurement => 0,
	#** The function should start a new measurement 
	cmdRestartAFMeasurement => 1
}; # etalAFModeTy

#**********************************
 #
 # ETAL version
 #
 #*********************************
#**
 # \struct	EtalComponentVersion
 # 			Describes the Firmware or Software version
 # 			of ETAL or one of the attached devices
 #*
#typedef struct
#{
	#** TRUE if the device is configured; if FALSE the following
	 #**  fields should be ignored 
#	tBool      m_isValid;
	#** Version major number 
#	tU8        m_major;
	#** Version middle number 
#	tU8        m_middle;
	#** Version minor number 
#	tU8        m_minor;
	#** Version build number 
#	tU32       m_build;
	#** Version string, may be a combination of the
	 #  previous fields or something else (see ETAL specification
	 #**  for the #etal_get_version API 
#	tChar      m_name[ETAL_VERSION_NAME_MAX];
#} EtalComponentVersion;

#**
 # \struct	EtalVersion
 # 			Version number of all ETAL components
 #*
#typedef struct
#{
	#** ETAL software version 
#	EtalComponentVersion m_ETAL;
	#** DAB DCOP firmware version 
#	EtalComponentVersion m_MDR;
	#** HDRadio DCOP firmware version 
#	EtalComponentVersion m_HDRadio;
	#** CMOST firmware version. This is an array
	 #  containing one entry per each Tuner supported
	 #**  by the ETAL build (#ETAL_CAPA_MAX_TUNER) 
#	EtalComponentVersion m_CMOST[ETAL_CAPA_MAX_TUNER];
#} EtalVersion;

#**********************************
 #
 # Debug types
 #
 #*********************************
#**
 # \enum	EtalDISSMode
 # 			Parameter for the #etal_debug_DISS_control API
 #*
 use constant
{
	#** Use auto DISS mode 
	ETAL_DISS_MODE_AUTO => 0,
	#** Use manual DISS mode 
	ETAL_DISS_MODE_MANUAL => 1,
}; # EtalDISSMode

#**
 # \enum	etalChannelTy
 # 			Identifies the Tuner channel(s).
 # \remark	To be used for the #etal_debug_DISS_control API only.
 # \remark	Must not be used to specify the channel in 
 # 			the #ETAL_MAKE_FRONTEND_HANDLE macro, use
 #			#ETAL_FE_FOREGROUND or ETAL_FE_BACKGROUND instead
 #*
use constant
{
	ETAL_CHN_UNDEF          => 0,
	ETAL_CHN_FOREGROUND     => 1,
	ETAL_CHN_BACKGROUND     => 2,
	ETAL_CHN_BOTH           => 3
}; # etalChannelTy

#**
 # \struct	EtalWSPStatus
 # 			Parameter for the #etal_debug_get_WSP_Status
 #*
#typedef struct
#{
#    tU8 m_filter_index[2];
#    tU8 m_softmute;
#    tS8 m_highcut;
#    tS8 m_lowcut;
#    tU8 m_stereoblend;
#    tU8 m_highblend;
#    tS32 m_rolloff;
#} EtalWSPStatus;

#**********************************
 #
 # STAR RDS definition
 #
 #*********************************
#**
 # \struct	EtalRDSRawData
 # 			Raw (undecoded) RDS data block
 #*
#typedef struct
#{
	#** CMOST Read Notification Register contents relative
	 #**  to this RAW data block 
#	tU8			m_RNR[3];
	#** The Raw RDS data block. See ETAL Specification for details 
#	tU8			m_RDS_Data[((STAR_RDS_MAX_BLOCK + 1) * 3)];
#} EtalRDSRawData;

#**
 # \enum	EtalRDSRBDSModeTy
 # 			Parameter for the #etal_start_RDS API:
 # 			defines the type of RDS
 #*
use constant
{
	#** Use plain RDS 
	ETAL_RDS_MODE  => 0,
	#** Use RBDS 
	ETAL_RBDS_MODE => 1
}; # EtalRDSRBDSModeTy

#**********************************
 #
 # SEEK definitions
 #
 #*********************************

#**
 # \enum	etalSeekDirectionTy
 # 			Defines the direction for a Seek command.
 #			Note that if the Seek is configured to wrap around the band
 #			when a band limit is reached there will be a small timeframe
 #			when the condition will not be respected.
 #*
use constant
{
	#** Direction is from lower frequency to higher frequency 
	cmdDirectionUp => 0,
	#** Direction is from higher frequency to lower frequency 
	cmdDirectionDown => 1
}; # etalSeekDirectionTy

#**
 # \enum	etalSeekAudioTy
 # 			Defines if at the end of a Seek command the audio
 # 			should be muted or not.
 #*
use constant
{
	#** Audio should be unmuted 
	cmdAudioUnmuted => 0,
	#** Audio should be muted 
	cmdAudioMuted => 1
}; # etalSeekAudioTy

#**********************************
 #
 # Audio Configuration Capabilities
 #
 #*********************************
#**
 # \enum 	EtalAudioAlignmentAttr
 # 			structure to define parameters for the Automatic Audio Alignment (AAA) algorithm configuration
 #*
#typedef struct
#{
	#** Controls the configuration for FM AAA enabling/disabling 
#	tBool             m_enableAutoAlignmentForFM;
	#** Controls the configuration for AM AAA enabling/disabling 
#	tBool             m_enableAutoAlignmentForAM;
#} EtalAudioAlignmentAttr;

#endif # ETAL_TYPES_H_ 


1;
