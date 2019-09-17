//!
//!  \file      dabmw_import.h
//!  \brief     <i><b> Header file containing exported elements </b></i>
//!  \details   This file is the Dab Middleware header.
//!  \author    
//!  \author   
//!  \version  
//!  \date      
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef DABMW_IMPORT_H_
#define DABMW_IMPORT_H_


#define ETAL_RDS_IMPORT

// Invalid generic data
#define DABMW_INVALID_DATA                      ((tU32)0xFFFFFFFF)
#define DABMW_INVALID_DATA_BYTE                 ((tU8)0xFF)
// Invalid generic data
#define DABMW_INVALID_DATA_U16                  ((tU16)0xFFFF)
#define DABMW_INVALID_DATA_S16                  ((tS16)0xFFFF)


#define DABMW_INVALID_SDATA                     ((tS32)-1)

// Invalid frequency
#define DABMW_INVALID_FREQUENCY                 (DABMW_INVALID_DATA)

// Invalid EID
#define DABMW_INVALID_ENSEMBLE_ID               ((tU32)0x00FFFFFF)
#define DABMW_INVALID_ECC                       ((tU8)0xFF)
#define DABMW_INVALID_EID                       ((tU16)0xFFFF)

// Invalid service ID (valid for DAB and AM/FM)
#define DABMW_INVALID_SERVICE_ID                ((tU32)0xFFFFFFFF)

// Define invalid slot
#define DABMW_INVALID_SLOT                      ((tSInt)-1)

// Define RDS sources. Maximum of 2 sources are available for RDS, even if the third path is used
#define DABMW_RDS_SOURCE_MAX_NUM                ((tU8)ETAL_CAPA_MAX_FRONTEND)

/*
 * from rds_landscape.h
 */
#define DABMW_AF_LIST_BFR_LEN                    (ETAL_DEF_MAX_AFLIST) //   (tU8)26 // 25 + 1 (base)

/*
 * from rds_mngr.h
 */

// Defines for the AF command
#define DABMW_GET_AF_AUTODETECTMODE                     (FALSE)
#define DABMW_GET_AF_CURRENTSTATUSMODE                  ((tU8)1)


#ifndef CONFIG_TARGET_CPU_COMPILER_GNU
#define CONFIG_TARGET_CPU_COMPILER_GNU
#endif

typedef enum
{
    DABMW_NONE_APP                      = 0x00,
    DABMW_MAIN_AUDIO_DAB_APP            = 0x01, // Used also for DAB DATA
    DABMW_SECONDARY_AUDIO_DAB_APP       = 0x02, // Used also for DAB DATA
    DABMW_MAIN_AMFM_APP                 = 0x03, // AM/FM used for audio playback
    DABMW_BACKGROUND_AMFM_APP           = 0x04, // AM/FM used for background scan
    DABMW_MAIN_RDS_APP                  = 0x05, // RDS using internal RDS paths
    DABMW_SECONDARY_RDS_APP             = 0x06, // RDS using third path
    DABMW_HDRADIO_STANDALONE_APP        = 0x07, // HDRadio source
    DABMW_3RD_PATH_AMFM_APP             = 0x08, // AM/FM third path
    DABMW_DRM_FOREGROUND_APP            = 0x09, // DRM foregorund application
    DABMW_DRM_BACKGROUND_APP            = 0x0A, // DRM background primary application
    DABMW_ALL_APPLICATIONS              = 0x0F
} DABMW_mwAppTy;


typedef ETAL_HANDLE DABMW_RDS_mwAppTy;


// defines for dab

// Set the auto-notification reasons for DAB and for AM/FM
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_TUNE            ((tU8)0x01)
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_SEARCHENSEMBLE  ((tU8)0x02)
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_SYNC            ((tU8)0x04)
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_RECONF          ((tU8)0x05)
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_BERFIC          ((tU8)0x06)
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_MUTE            ((tU8)0x07)
#define DABMW_DAB_STATUS_NOTIFICATION_REASON_IS_TUNECANCELLED   ((tU8)0x08)

#define DABMW_AMFM_STATUS_NOTIFICATION_REASON_IS_AF_TUNED       ((tU8)0x10)


#define DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SIG          ((tU8)0x01)
#define DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SYNC         ((tU8)0x02)
#define DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_MCI          ((tU8)0x04)

// Auto-notification sync information
#define DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_NOTTUNED          ((tU8)0x00)
#define DABMW_DAB_STATUS_NOTIFICATION_SYNC_IS_TUNED             ((tU8)(DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SIG|DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_SYNC|DABMW_DAB_STATUS_NOTIFICATION_SYNC_FLAG_DAB_MCI))


#define DABMW_ENSEMBLE_UNIQUE_ID(ecc,id)    ((((tU32)(ecc) << 16) & (tU32)0x00FF0000) | \
                                              ((tU32)(id) & (tU32)0x0000FFFF))
                                              
// This field is also related to the number of bits used to indicate a 
// confirmed frequency after a learn, the number of available bits is 4 so this 
// number cannot be greater than 4
#define DABMW_LANDSCAPE_DB_ALT_FREQ_NUM     3

// Type of the exchanged data for PAD
#define DABMW_DATATPE_IS_DAB_DLS                (tU8)2

typedef struct
{
    // Ensemble identifier, this, in order to be unique, must
    // contains also the frequncy because an ensemble can be legally
    // transmitted in 2 frequencies with a different service organization
    tU8 ecc;
    tU16 id;
    tU32 frequency; // This is the last tuned frequency for the ensemble and also
                    // the only one if the ensemble is not available on multiple muxes

    tU32 alternativeFrequencies[DABMW_LANDSCAPE_DB_ALT_FREQ_NUM]; // Alternative frequencies

    // DABMW_ensembleAgeStatusTy learnAgeStatus;
} DABMW_ensembleUniqueIdTy;



// Events
typedef enum
{
    DABMW_EVENT_NONE                            = 0,
    DABMW_EVENT_FREQUENCY_TUNED                 = 1,
    DABMW_EVENT_FREQUENCY_DESELECTED            = 2,
    DABMW_EVENT_SYNC_REACHED                    = 10,
    DABMW_EVENT_SYNC_LOST                       = 11
} DABMW_eventTy;


// Storage source
typedef ETAL_HANDLE DABMW_storageSourceTy;
// were enum DABMW_storageSourceTy values;
// not used, only to avoid compiler error
#define DABMW_STORAGE_SOURCE_IS_STREAMDECODER       0
#define DABMW_STORAGE_SOURCE_IS_RDS_1               1
#define DABMW_STORAGE_SOURCE_IS_RDS_2               2
#define DABMW_STORAGE_SOURCE_IS_RDS_3               3
#define DABMW_STORAGE_SOURCE_IS_RDS_ALL             4


// Storage status
typedef enum
{
    DABMW_STORAGE_STATUS_IS_EMPTY               = 0x00,
    DABMW_STORAGE_STATUS_IS_STORED              = 0x01,
    DABMW_STORAGE_STATUS_IS_RETRIEVED           = 0x02,
    DABMW_STORAGE_STATUS_IS_MARKED_DELETE       = 0x04,
    DABMW_STORAGE_STATUS_IS_DUPLICATE           = 0x10,
    DABMW_STORAGE_STATUS_IS_VERIFIED            = 0x20,
    DABMW_STORAGE_STATUS_IS_USED                = 0x40,
    DABMW_STORAGE_STATUS_IS_UNKNOWN             = 0x80
} DABMW_storageStatusEnumTy;


// ADD ON FOR SF : defines DABMW
//

/*!
 * \enum	DABMW_mwCountryTy
 * 			Lists the country variants supported by DAB DCOP
 */
typedef enum
{
	/*! Unknown, no initialized */
    DABMW_COUNTRY_NONE              = 0,
	/*! Europe */
    DABMW_COUNTRY_EUROPE            = 1,
	/*! United States of America */
    DABMW_COUNTRY_USA               = 2,
	/*! Japan */
    DABMW_COUNTRY_JAPAN             = 3,
	/*! Eastern Europe */
    DABMW_COUNTRY_EAST_EU           = 4,
	/*! China */
    DABMW_COUNTRY_CHINA             = 5,
	/*! Korea */
    DABMW_COUNTRY_KOREA             = 6,
	/*! Canada */
    DABMW_COUNTRY_CANADA            = 7,
	/*! Unspecified */
    DABMW_COUNTRY_ANY               = 8    
} DABMW_mwCountryTy;

// Available bands
#define DABMW_AVAILABLE_BAND_NUM        27

typedef enum
{
    DABMW_BAND_NONE                     = 0,
    DABMW_BAND_FM_EU                    = 1,
    DABMW_BAND_FM_US                    = 2,
    DABMW_BAND_FM_JAPAN                 = 3,
    DABMW_BAND_FM_EAST_EU               = 4,
    DABMW_BAND_FM_WEATHER_US            = 5,
    DABMW_BAND_AM_MW_EU                 = 6,
    DABMW_BAND_AM_MW_US                 = 7,
    DABMW_BAND_AM_MW_JAPAN              = 8,
    DABMW_BAND_AM_MW_EAST_EU            = 9,
    DABMW_BAND_DAB_III                  = 10,
    DABMW_BAND_CHINA_DAB_III            = 11,
    DABMW_BAND_KOREA_DAB_III            = 12,
    DABMW_BAND_DAB_L                    = 13,
    DABMW_BAND_CANADA_DAB_L             = 14,
	DABMW_BAND_AM_LW					= 15,			  
	DABMW_BAND_AM_SW1					= 16,
	DABMW_BAND_AM_SW2					= 17,
	DABMW_BAND_AM_SW3					= 18,
	DABMW_BAND_AM_SW4					= 19,
	DABMW_BAND_AM_SW5					= 20,
	DABMW_BAND_AM_SW6					= 21,
	DABMW_BAND_AM_SW7					= 22,
	DABMW_BAND_AM_SW8					= 23,
	DABMW_BAND_AM_SW9					= 24,
	DABMW_BAND_AM_SW10					= 25,	
    DABMW_BAND_DRM30                    = 26
} DABMW_systemBandsTy;


// AMFM Quality infos
// EPR CHANGE : SF & Quality handling
// Defines structure forFM quality storing/handling.
//
typedef struct
{
    // Retrieved quality parameters
    tU32 combinedQ;
    tU16 multipath;
    tU16 adjacentChannel;
    tU16 deviation;
    tU16 fieldStrength;
    tU16 detuning;
    tS16 fieldStrength_dBuV;
	tU16 snr;
	tU16 mpxNoise;
} DABMW_amfmQualityTy;

// defines 
// Service selection subfunction codes
// 
#define DABMW_SERVICE_SELECT_SUBFNCT_SET                     (ETAL_SERVSEL_SUBF_SET)
#define DABMW_SERVICE_SELECT_SUBFNCT_REMOVE                  (ETAL_SERVSEL_SUBF_REMOVE)
#define DABMW_SERVICE_SELECT_SUBFNCT_APPEND                  (ETAL_SERVSEL_SUBF_APPEND)


// Dab status
typedef struct
{
   //  OSAL_tIODescriptor sourceDeviceId;  // Source device ID

    DABMW_mwAppTy targetApp;            // This 2-bit field, coded as an unsigned binary number,
                                        // indicates the target application whose status is notified to the host

    tBool search;                       // Search status, 0 = no search ongoing, 1 = search ongoing


    tU8 notifyReason;                   // This 4-bit field, coded as an unsigned binary number,
                                        // indicates the reason for the notification

    tU8 txMode;                         // This 3-bit field, coded as an unsigned binary number,
                                        // indicates the DAB transmission mode of the ensemble at the currently tuned frequency

                                     
    tU8 berFic;                         // This 3-bit unsigned number shall describe the current FIC channel
                                        // bit error rate BER (before the Viterbi decoder) as follows

    tU8 mute;                           // This 2-bit unsigned number shall indicate the current muting level
                                        // of a present audio source

    tU32 tunedFreq;                     // This 19-bit unsigned binary number indicates the currently tuned frequency as
                                        // defined in [1]. Except tune_freq = 0, this means receiver is in an idle state
                                        // (not tuned to any frequency)

    tU8 reconf;                         // This 4-bit field indicates if there is a service or sub-channel
                                        // reconfiguration expected as signalled in the multiplex configuration information (MCI)

    tU8 sync;                           // This 4-bit field indicates how far the receiver has synchronized on the DAB ensemble

    tU32 cif;                           // Last CIF received from FIG 0/0

    tBool dabProcFeatureChanged;        // This flag is set to true when flags changes (a BAND CHANGE is needed to apply them)

    tU8 backupSyncStatus;               // Stores last sync status. Shall be used together with sync to detect status changes 
} DABMW_dabStatusTy;




#endif // DABMW_IMPORT_H

