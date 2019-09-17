/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/* \file DAB_Tuner_Ctrl_Types.h		
* Copyright (c) 2016, Jasmin Infotech Private Limited.
*  All rights reserved. Reproduction in whole or part is prohibited
*  without the written permission of the copyright owner.
*
*  Project              :  ST_Radio_Middleware
*  Organization			:  Jasmin Infotech Pvt. Ltd.
*  Module				:  Radio DAB Tuner Control
*  Description			:  This file contains type definitions for DAB Tuner Ctrl.
*
*
**********************************************************************************************************/

#ifndef DAB_TUNER_CTRL_TYPES_
#define DAB_TUNER_CTRL_TYPES_

/*--------------------------------------------------------------------------------------------------
    includes
--------------------------------------------------------------------------------------------------*/
#include "cfg_types.h"
#include "sys_task.h"

/*--------------------------------------------------------------------------------------------------
    defines
--------------------------------------------------------------------------------------------------*/
#define DAB_TUNER_CTRL_MAX_LABEL_LENGTH  			17u

#define DAB_TUNER_CTRL_MAX_EXTENDED_LABEL_LENGTH	64u /* Extended label maximum length */

#define MAX_SERVICE_COMPONENTS 						2u


#define MAX_SERVICES 								35u

#define SYS_MSG_GET_PID(cid_msgid)           		((cid_msgid).u16_cid)

#define SYS_MSG_GET_LID(cid_msgid)           		((cid_msgid).u16_msgid)

#define	DAB_APP_MAX_ENSEMBLES						41u

#define	DAB_APP_MAX_SERVICES						150u

#define	DAB_APP_MAX_COMPONENTS						200u

#define DAB_MAX_NUM_SUBCHANNELID 					12u /* max no. of subchannels in a service*/
#define MAX_NUM_OF_ANNO_DATA 						12u  /* index value to support all 11 announcements */

/* MACRO's For Character Set */
#define	DAB_TUNER_CTRL_CHARSET_EBU					(Tu8)1
#define	DAB_TUNER_CTRL_CHARSET_UCS2					(Tu8)2
#define	DAB_TUNER_CTRL_CHARSET_UTF8					(Tu8)3

/* Frequency Band III */
#define DAB_TUNER_CTRL_5A_EU      					( 174928u )
#define DAB_TUNER_CTRL_13F_EU      					( 239200u )
#define DAB_TUNER_CTRL_EU_MAX_FREQUENCIES   		( 41u )  /* Maximum frequencies in Band III EUROPE region */

#define GET_AUDIO_STATUS_REQ_MAX_TIME_RETRIGGER		(3u) 
#define MAX_DLS_LENGTH                              128u
#define MAX_ALT_FREQUENCY 					        8u
#define MAX_ALT_EID 						        5u
#define MAX_HARDLINK_SID 					        12u
#define MAX_HARDLINK_PI 					        12u

#define MAX_EIDS_IN_FIG								11u

/* As per DAB Standard SLS image maximum size is 50 KB */
#define		MAX_DATA_SERVICE_PAYLOAD_SIZE							51200

/*-----------------------------------------------------------------------------
                                        Type Definitions
-----------------------------------------------------------------------------*/
/**
 * @brief Differentiate the select service and seek request.
 */
typedef enum 
{
	DAB_TUNER_CTRL_SEL_SER,
	DAB_TUNER_CTRL_AFTUNE,
	DAB_TUNER_CTRL_AFTUNE_END,
	DAB_TUNER_CTRL_BANDSCAN,
	DAB_TUNER_CTRL_ESD_CHECK,
	DAB_TUNER_CTRL_SEEK,
	DAB_TUNER_CTRL_LEARN,
	DAB_TUNER_CTRL_INVALID
}Te_DAB_Tuner_Ctrl_RequestCmd;

/**
 * @brief This enum describes for different Cancel Types. Ex: Seek Cancel, Scan Cancel
 */
typedef enum{
    
    DAB_TUNER_CTRL_SEEK_CANCEL,            /**< Cancel type: SEEK */
	DAB_TUNER_CTRL_SCAN_CANCEL,                /**< Cancel type: SCAN   */
	DAB_TUNER_CTRL_AF_TUNE_CANCEL,
	DAB_TUNER_CTRL_TUNE_CANCEL,
    DAB_TUNER_CTRL_CANCEL_INVALID        /**< not a valid Cancel type              */
}Te_DAB_Tuner_Ctrl_CancelType;

/*@brief identifies different markets, This enumeration represents different markets information*/
typedef enum
{        
    DAB_TUNER_CTRL_ACTIVATE_REQUEST,                                     /**<0- Enum, for the SRC Active*/                            
    DAB_TUNER_CTRL_DEACTIVATE_REQUEST,                                   /**<1- Enum, for the SRC DeActive*/                            
    DAB_TUNER_CTRL_INVALID_REQUEST
	                                    									/**<2- Enum, for the SRC Invalid*/                                        
}Te_DAB_Tuner_Ctrl_ActivateDeActivateStatus;



/*@brief identifies different markets, This enumeration represents different markets information*/
typedef enum
{
                                                                                                                                                   
    TC_RCE_MARKET_WESTERN_EUROPE,                                                                                                                                  
    TC_RCE_MARKET_LATIN_AMERICA,                                                                                                                          
    TC_RCE_MARKET_ASIA_CHINA,                                                                                                                                         
    TC_RCE_MARKET_ARABIA,                                                                                                                                                
    TC_RCE_MARKET_USA_NORTHAMERICA,                                                                                                                    
    TC_RCE_MARKET_JAPAN,                                                                                                                                     
    TC_RCE_MARKET_KOREA,
	TC_RCE_MARKET_INVALID                                                                                                                             
}Te_DAB_Tuner_Market;

typedef enum
{
	TUNER_CTRL_NO_RECONFIG, 
    TUNER_CTRL_SERVICE_LIST_RECONFIG,
	TUNER_CTRL_SERVICE_RECONFIG	

}Te_Tuner_Ctrl_ReConfigType;

typedef enum 
{
	DAB_TUNER_CTRL,
	DAB_TUNER_APPL,
	RADIO_MASTER
}Te_DABComponent_SWC;

typedef enum
{
	TUNER_CTRL_INVALID,
	TUNER_CTRL_SCAN_STARTED,	                 
    TUNER_CTRL_SCAN_COMPLETED,
	TUNER_CTRL_SCAN_NOTIFICATION_REEIVED	
	
}Te_Tuner_Ctrl_ScanStatus;

  /**
 * @brief Reply status of tune.
 * @details Used in the context of tune reply.
 */
/*
typedef enum
{
        TUNER_CTRL_TUNE_SUCCESS,       // DAB tuner control TUNE successful  
        TUNER_CTRL_TUNE_FAILURE,        // TUNE request process is failed 
		
}Te_Tuner_Ctrl_TuneReplyStatus;
*/

typedef enum
{
	TUNER_CTRL_SERVICE_SAME_ENSEMBLE,
	TUNER_CTRL_SERVICE_OTHER_ENSEMBLE,
	TUNER_CTRL_SERVICE_NOT_IN_ENSEMBLE,
	TUNER_CTRL_SERVICE_SELECTED,
	TUNER_CTRL_SERVICECOMPONENT_SELECTED,
	TUNER_CTRL_SERVICE_GOT_PROGRAMMLIST,
	TUNER_CTRL_SERVICE_INVALID	
	
}Te_Tuner_Ctrl_SelectService_Request_status;

typedef enum
{
	DAB_APP_TUNER_CTRL_NONE,
	DAB_APP_TUNER_CTRL_SELSERV_REQID,
	DAB_APP_DAB_TUNER_CTRL_SERVCOMPSEEK_REQID,	
	DAB_APP_DAB_TUNER_CTRL_INVALID
}Ts_Dab_app_Tuner_Requests;

typedef enum
{
	DAB_TUNER_CTRL_SERVICE_TYPE_INVALID,
	DAB_TUNER_CTRL_SERVICE_TYPE_DAB,	
	DAB_TUNER_CTRL_SERVICE_TYPE_DAB_PLUS,
	DAB_TUNER_CTRL_SERVICE_TYPE_DMB
	
}Te_Tuner_Ctrl_ServiceType ;

typedef enum
{
    DAB_TUNER_CTRL_HARDLINK_PI,									/**<0.Notify Enum, Hardlink PI */
    DAB_TUNER_CTRL_IMPLICIT_SID,								/**<1.Notify Enum, Implicit PI */
	DAB_TUNER_CTRL_NO_IMPLICIT									/**<2.Notify Enum, No Implicit */

}Te_Tuner_Ctrl_BestPI_Type;

typedef enum
{
	
	DAB_TUNER_CTRL_FMDAB_LINKING_REQ_PI_CHANGE =1,	
	DAB_TUNER_CTRL_FMDAB_LINKING_REQ_NEW_REQUEST,
	DAB_TUNER_CTRL_FMDAB_LINKING_REQ_SIG_LOST,
	DAB_TUNER_CTRL_FMDAB_LINKING_REQ_INVALID
	
}Te_FmtoDAB_Reqstatus;

typedef enum
{
	DAB_TUNER_CTRL_DAB_AF_SETTINGS_ENABLE,
    DAB_TUNER_CTRL_DAB_AF_SETTINGS_DISABLE,
    DAB_TUNER_CTRL_DAB_AF_INVALID,
	
}Te_DAB_Tuner_Ctrl_DAB_AF_Settings;

/**
 * @brief structure containing about a Label,ShortLabel.

 * Detailed This structure represents the Label information parameters
   of the DAB Tuner Control.
 * Ensemble information parameters include Label,ShortLabelFlag.
 */
typedef struct
{
  Tu16	u16_ShortLabelFlags;      						/**< ShortLabel Flag*/
  Tu8	au8_label[DAB_TUNER_CTRL_MAX_LABEL_LENGTH]; 	/**< characters (the label itself) */
  Tu8	u8_CharSet;
  Tu8	au8_padding[1]; 
  
} Ts_Tuner_Ctrl_Label;

/**
 * @brief structure containing basic information about an BaiscEnsembleInformation.

 * Detailed This structure represents the BasicEnsembel information parameters
   of the DAB Tuner Control.
 * Ensemble information parameters include frequency,ECC,EnsembleId.
 */
typedef struct 
{
    Tu32 u32_Frequency;                   /**< Frequency in kHz */
    Tu16 u16_EId;                         /**< Ensemble Identifier */
    Tu8  u8_ECC;                          /**< Extended Country Code */
	Ts8  RSSI;
    //Tu8  padding; 
}  Ts_Tuner_Ctrl_BasicEnsembleInfo;
/**
 * @brief structure containing basic information about an Ensemble.

 * Detailed This structure represents the Ensemble information parameters
   of the DAB Tuner Control.
 * Ensemble information parameters include Structure to BasicEnsemble,NumberofAudioservice,NumberofDataservice.
 */
typedef struct
{
	Ts_Tuner_Ctrl_BasicEnsembleInfo st_BasicEnsInfo;
	Ts_Tuner_Ctrl_Label st_EnsembleLabel;
	Tu8  u8_NumberOfAudioServices;             /**< No of AudioServices in Ensemble */
    Tu8  u8_NumberOfDataServices;             /**< No of DataServices in Ensemble */
    Tu8                               padding[2]; 
} Ts_Tuner_Ctrl_EnsembleInfo;

/**
 * @brief structure conta0ining basic information about  service.
 
 * @details used in context of service Information of the selected 
	service in an Ensemble.
 */
typedef struct
{
	Ts_Tuner_Ctrl_BasicEnsembleInfo st_BasicEnsInfo;
	Ts_Tuner_Ctrl_Label     st_ServiceLabel;
    Tu8				       u8_CA_Applied;             /**< CA Flag */
  	Tu8				       u8_PDFlag;					/**< program/data service*/
	Tu32                   u32_SId;                  /**< Service Id */
//	Tu8                    u8_SCIdI;                /**identifier for component */
    Tu8                    u8_CAId; 	                /**< CA Identifier */
    Tu8                    u8_ptyCode;               /**< Program type */
    Tu8                    u8_dynPtyCode;            /**< Dynamic Program type */

}Ts_Tuner_Ctrl_ServiceInfo;

/**
 * @brief structure containing basic information about  ServiceComponent.
 
 * @details used in context of service Information of the selected 
	service component in a service.
 */
typedef struct
{
	Ts_Tuner_Ctrl_BasicEnsembleInfo st_BasicEnsInfo;
	Ts_Tuner_Ctrl_Label st_compLabel;
    Tu32       u32_SId;                  /**< Service Id */
	Tu16        u16_SCIdI;                /**identifier for component */
	Tu8      	u8_Primary;                /**< Primary Flag*/
	Tu8      	u8_CA_Applied;             /**< CA Flag */
    Tu16       u16_UAType;               /**< User Application Type*/
    Tu8        u8_Language;              /**< Language*/
	Tu8        u8_CAId; 	               /**< CA Identifier */
	Tu8        u8_TransportMech;         /**< Transport mechanism( MSC contains is audio/Data/X-Pad)  */
	Tu8        u8_ComponentType;         /**< Type of Component ASCTy/DSCTy (i.e MPEG I/II/III)*/

} Ts_Tuner_Ctrl_ComponentInfo;

/**
 * @brief structure containing basic information about  TunerStatus.
 * detailed structure represents the possible Tuner Status information 
   parameters of the DAB Tuner Control.
 */

#if 0
typedef struct
{
	Tu32		 u32_Frequency;			 /**<the frequency to be tuned in kHz */
	Tu32		 u32_Receptionquality;  /**<Reception Quality */
	Tu8		 	u8_Audio_status;        /**<Audio Status  */
	Tu8		 	u8_Mute_state;          /**<Audio Mute state */
    Tu8         padding[2]; 
}Ts_Tuner_Ctrl_TunerStatusNotify;
#endif

typedef struct
{
	Te_Tuner_Ctrl_ServiceType	e_ServiceType ;
	Tu32                 		u32_Frequency;         //Current tuned frequency in kHz 
	Tu32                 		u32_SId;					  //Current Sid
	Tu16                 		u16_EId;
	Tu16                 		u16_SCIdI;                // Current identifier for component 
	Tu8					 		u8_ECC ; 
	Tu8         				padding[3];

	Ts_Tuner_Ctrl_Label	 		Ensemble_label;
	Ts_Tuner_Ctrl_Label	 		service_label; 	/**< characters (the label itself) */
	Ts_Tuner_Ctrl_Label	 		servicecomponent_label; 	/**< characters (the label itself) */
	Tu8					 		au8_ChannelName[4]; 
		
}Ts_Tuner_Ctrl_CurrentEnsembleInfo;

typedef struct
{
  Tu16          EId;
  Tu8			EnsembleCountryID;
  Tu16			EnsembleReference;
  Tu8           ChangeFlag;
  Tu8           AlFlag;
  Tu8           CIFCountHigh;                  /**< Common Interleaved Frame counter, high part */
  Tu8           CIFCountLow;                   /**< Common Interleaved Frame counter, low part */
  Tu8           OccurrenceChange;              /**< Common Interleaved Frame lower part where next configuration takes place */       
	
	
}Ts_Ensemble_Info;

typedef enum
{        
    DAB_TUNER_CTRL_DABFMLINKING_ENABLE,                                                                
    DAB_TUNER_CTRL_DABFMLINKING_DISABLE,                                                                
    DAB_TUNER_CTRL_DABFMLINKING_INVALID      
                                                              
}Te_DAB_Tuner_DABFMLinking_Switch;

/*structures related to  Announcment*/
/* To store FIG 0/18 Database - Announcement support information*/

/**
 * @brief Announcement Types
 *
 * This structure contains all announcement types.
 */
typedef enum
{
  DAB_TUNER_CTRL_ANNO_TYPE_INVALID,
  DAB_TUNER_CTRL_ALARM_ANNO,
  DAB_TUNER_CTRL_ROADTRAFFIC_ANNO,
  DAB_TUNER_CTRL_TRANSPORT_ANNO,
  DAB_TUNER_CTRL_WARNING_ANNO,
  DAB_TUNER_CTRL_NEWS_ANNO,
  DAB_TUNER_CTRL_WEATHER_ANNO,
  DAB_TUNER_CTRL_EVENT_ANNO,
  DAB_TUNER_CTRL_SPECIAL_ANNO,
  DAB_TUNER_CTRL_PROGRAMME_ANNO,
  DAB_TUNER_CTRL_SPORT_ANNO,
  DAB_TUNER_CTRL_FINANCIAL_ANNO
}Te_DAB_Tuner_Ctrl_announcement_type;

typedef enum
{
    DAB_TUNER_CTRL_ANNO_ON,                                 
    DAB_TUNER_CTRL_ANNO_OFF,                    
    DAB_TUNER_CTRL_ANNO_OFF_SIGNAL_LOSS,                
    DAB_TUNER_CTRL_ANNO_OFF_USER_CANCEL,                
    DAB_TUNER_CTRL_ANNO_OFF_ANNO_SETTINGS_OFF,            
    DAB_TUNER_CTRL_ANNOUNCEMENT_NOT_AVAILABLE,              
    DAB_TUNER_CTRL_ANNO_ANNO_INVALID                                  
}Te_Tuner_Ctrl_AnnoIndication;

typedef enum
{
	
	DAB_TUNER_CTRL_FMDAB_SAME_PI_STATION,
	DAB_TUNER_CTRL_FMDAB_REGIONAL_PI_STATION,
	DAB_TUNER_CTRL_FMDAB_PI_STATION_UNIDENTIFIED,
	DAB_TUNER_CTRL_FMDAB_PI_RECEIVED,
	DAB_TUNER_CTRL_TUNED_TO_SAME_STATION,
	DAB_TUNER_CTRL_TUNED_STATIONS_SORTED,
	DAB_TUNER_CTRL_TUNED_STATION_NOTSTABLE,
	DAB_TUNER_CTRL_FMDAB_BLENDING_SUCCESS,
	DAB_TUNER_CTRL_FMDAB_BLENDING_CANCELLED,
	DAB_TUNER_CTRL_FMDAB_BLENDING_STOP,
	DAB_TUNER_CTRL_FMDAB_PI_INVALID
}Te_dab_tuner_ctrl_fmdab_linkstatus;

typedef struct
{
	Tu16 		SId;
	Tu16		AsuFlag;
	Tu8			Numof_clusters;
	Tu8			Clusterid[23];
}Ts_Announcement_Support;

/* To store FIG 0/19 - Announcement switching information*/
typedef struct
{
	Tbool		b_NewFlag;
	Tbool		b_RegionFlag;
	Tu16		u16_AswFlag;
	Tu8         u8_SubChId;             
	Tu8         u8_RegionID;            
	Tu8			u8_Clusterid;
	Tu8			padding[3];	
}Ts_Anno_Swtch_Info;

/* Struture used for notification to DAB APP*/
typedef struct
{
	Te_DAB_Tuner_Ctrl_announcement_type e_announcement_type;
	Tu32 								u32_SId;
	Tu16								u16_AswFlag;
	Tu8         						u8_SubChId;             
	Tu8									u8_Clusterid;
}Ts_DAB_Tuner_Anno_Swtch_Info;

/* Struture used for storing the history of Fig 0/19 notification*/
typedef struct
{
	Ts_Anno_Swtch_Info		st_Anno_Swtch_Info;
	Tu16 					u16_Anno_CIFCount;
	Tbool					b_Notification_sent_flag;
}Ts_Anno_Swtch_Notify_Info;

/**************************** FIG 0/25 *************************************/
typedef struct OE_Anno_Support
{
	Tu16 au16_EID[MAX_EIDS_IN_FIG];			/*max memory allocated for the EID field.*/
	Tu16 u16_SID;
	Tu16 u16_ASU_Flags;
	Tu8 u8_Noof_EIDs;
}Ts_OE_Anno_Support;

/**************************** FIG 0/26 *************************************/
typedef struct OE_Anno_Switching
{
	Tu8   u8_ClusterID_CE;
	Tu16  u16_EID_OE;
	Tu16  u16_ASW_Flags;
	Tbool b_New_Flag;
	Tbool b_Region_Flag;
	Tu8   u8_RegionID_CE;
	Tu8   u8_ClusterID_OE;
	Tu8   u8_RegionID_OE;
}Ts_OE_Anno_Switching;

typedef struct
{
  Tu32	SId;
  Tbool b_LocalFlag;
  Tu8 	CAId;
  Tu8	NoOfServiceComponents;
  Tu8	TMID[12];
  Tu8	SubchId[DAB_MAX_NUM_SUBCHANNELID]; // max 12 sub components 
}Ts_Service_Info;

/**
 * @brief Announcement update states
 *
 * This structure contains all announcement update states.
 */
typedef enum
{
  DAB_TUNER_CTRL_ANNO_INFO_AVAILABLE,
  DAB_TUNER_CTRL_ANNO_STARTED,
  DAB_TUNER_CTRL_ANNO_CANCEL_SUCCESS,
  DAB_TUNER_CTRL_ANNO_STOP_SUCCESS,
  DAB_TUNER_CTRL_ANNO_SWITCHING_FAILURE,
  DAB_TUNER_CTRL_ANNO_CANCEL_FAILURE,
  DAB_TUNER_CTRL_ANNO_STOP,
  DAB_TUNER_CTRL_ANNO_STOP_FAILURE, 
  
}Te_announcement_status;

/* Enum to store whether DAB Tuner Ctrl state is BG/FG before restarting SoC*/
typedef	enum
{
	DAB_TUNER_CTRL_BG_STATE,
	DAB_TUNER_CTRL_FG_STATE,
	DAB_TUNER_CTRL_INVALID_STATE

}Te_DAB_Tuner_Ctrl_State ;

/* Inst HSM Enum to store if Restart request is valid or not */
typedef	enum
{
	DAB_TUNER_CTRL_DABTUNER_RESTART_REQ_INVALID,
	DAB_TUNER_CTRL_DABTUNER_RESTART_REQ_VALID,
	DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_VALID,
	DAB_TUNER_CTRL_DABTUNER_ACTIVATE_REQ_INVALID,
	DAB_TUNER_CTRL_FCATORY_RESET_REQ_VALID,
	DAB_TUNER_CTRL_FCATORY_RESET_REQ_INVALID

}Te_DAB_Tuner_Ctrl_DABTUNERRestartCmd ;
typedef enum
{
	DAB_TUNER_CTRL_MODE_OFF,                                                                    /*1- Enum for ENG mode OFF*/
    DAB_TUNER_CTRL_MODE_ON                                                                    /*0- Enum for ENG mode ON*/                                                            
        
}Te_DAB_Tuner_Ctrl_Eng_Mode_Request;

typedef enum
{
	DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_INVALID,                                                                   
    DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_RECEIVED,                                                                                                                              
    DAB_TUNER_CTRL_SYS_MONITOR_NOTIFICATION_TIMEOUT
	    
}Te_SystemMonitoringNotReceived;

typedef struct
{
	Ts32 	s32_RFFieldStrength;
	Ts32 	s32_BBFieldStrength;
	Tu32 	u32_FicBitErrorRatio;
	Tbool 	b_isValidMscBitErrorRatio;
	Tu32 	u32_MscBitErrorRatio;
	Tu8  	u8_BER_Significant; 
	Ts8  	s8_BER_Exponent;		
	Ts8  	s8_SNR_Level;	
	Ts8   	s8_RSSI;
	Tu8		u8_Decodingstatus;
	Tu8 	u8_AudioQuality;
	Tu8 	u8_AudioLevel;
}Ts_Tuner_Status_Notification;

typedef struct
{
	Tu8 replystatus;
}Ts_DAB_Tuner_Ctrl_Set_FIG_Reply;

typedef struct
{
	Tu32 id[12];	
	Tu8 idflag;
	Tu8	Activelink;
	Tu8 Hardlink;
	Tu8 Ils;
	Tu16 Lsn;
	Tu8 Idlq;
	Tu8 Noofids;
	Tu8 Cn;
	Tu8 OE;
	Tu8 Pd;
}Ts_Linking_info;

/**
 * @brief Frequency information data base
 *
 * This structure contains all values needed for a frequency information data base.
 */
typedef struct
{
	Tu32  frequency;                                  /**< Frequency in KHz */
	Tu16  EId;                                        /**< Ensemble identifier */
	Tu8   controlField;                               /**< see ETSI EN 300 401, section 8.1.8 */
	Tu8   reserved;                                   /**< alignment value */
}Ts_dab_freqInfo;

typedef struct
{
	Tbool	b_DABHardlinksAvailable;
	Tbool	b_DABHardlinksEIDAvailable;
	Tbool	b_DABHardlinksFreqAvailable;
	Tbool	b_FMHardlinksAvailable;
	Tu8 	DAB2DABLinking;
	Tu8 	DAB2FmLinking;
	Tbool	b_DABSameSidAvialable;
	Tbool	b_AlternateDABFreqAvailable;
	Tbool	b_DABSameSidFreqAvailable;
	Tbool	b_Same_SID_FMHardlinkAvailable;
	
}Ts_LinkingStatus;

typedef struct
{
	Tu32 Freq;
	Ts8  Freq_RSSI;
	Tu16 EId;
}Ts_Hardlink_EId_Database;

typedef struct
{
	Ts_Hardlink_EId_Database EId_Data;	
	Tu32 SId;
	Tu16 lsn;
	Tu8	 LA;
}Ts_Hardlink_DataBase;

typedef struct
{
	Ts_Hardlink_EId_Database Alt_EId[MAX_ALT_EID];
	Tu32 u32_SId;
	
}Ts_Alt_Ens_Database;

typedef struct
{
	Tu32 					u32_TunedSId;
	Tu32 					Alt_Freq[MAX_ALT_FREQUENCY];
	Ts8						Alt_Freq_RSSI[MAX_ALT_FREQUENCY];
	Ts_Hardlink_DataBase 	Hardlink_Sid[MAX_HARDLINK_SID];
	Tu16 					Hardlink_PI[MAX_HARDLINK_PI];
	Tu8 					PI_LA;
	Ts8 					Best_BER;
	Tu16					PI_LSN;
	Tbool					b_Hardlinks_sent;
	Ts_Alt_Ens_Database		Alt_Ensbs;
	Tu8						NoofFMHardlinks;
	Ts8						s8_Tuned_Freq_RSSI;
}Ts_CurrentSidLinkingInfo;

typedef struct
{
	
	Tbool b_FIG06_available;
	Tbool b_FIG021_available;
	Tbool b_FIG024_available;
	Tbool b_FIG021_Hardlink_PI_available;
	
}Ts_FIG_Data_available;

typedef struct
{
	
	Tu16 FMHardlinks[12];
	Tu8 NoOfPICodes;
	Tu8 padding[3];
	
}Ts_PI_Data;

typedef struct
{
	Tu16 Best_PI;
	Tu8 Best_PI_Quality;	
	Tbool  b_Best_PI_Received;
	Ts32 Blending_Final_Delay;
	Tu16 Avg_Leveldata;
	Tu8	Confidence_level;

}Ts_Blending_info;

/**
* @brief structure to contains Dataservice rawdata
*/
typedef struct
{
	Te_RADIO_EtalDataServiceType e_Header;
	Tu8					u8_Payload[MAX_DATA_SERVICE_PAYLOAD_SIZE];
	Tu32				u32_PayloadSize;
}Ts_DAB_Tuner_Ctrl_DataServiceRaw;

typedef struct
{        
        Tu8 u8_CharSet;
        Tu8 au8_DLSData[MAX_DLS_LENGTH];
        
}Ts_dab_DLS_data;

typedef struct
{
	Tu32 											u32_Frequency;     	 						/*Frequency in kHz*/  
	Tu16											u16_SId;                					/*Service Id*/
	Tu16											u16_EId;                					/*Ensemble Id*/  
	Tu8												u8_ECC;           		 					/*Extended County Code*/
	Tu16											u16_SCIdI;               					/*Service Component Id*/
	Ts8												s8_RSSI;
}Ts_dab_tuner_ctrl_fmdab_linkinfo;

typedef struct
{
	Tu32 Alt_Freq[MAX_ALT_FREQUENCY];    // Alternate frequencies
	Tu16 Alt_Ens[MAX_ALT_EID];                                                // Alternate Ensembles , no need to send respective frequencies to higher layer, that database for tuner control only.
	Tu32 Hardlink_Sid[MAX_HARDLINK_SID];  // Hardlinks , no need to send respective ensemble and frequency .
}Ts_AFList;

typedef struct
{
	Tu16 PI;
	Tu8  PI_Freq[MAX_ALT_FREQUENCY];	

}Ts_Alternate_PI;

typedef struct
{
	Ts_Alternate_PI  HL_PI[MAX_HARDLINK_PI];	
}Ts_PI_Freq;

 /**
 * @brief Structure to update lower, start and lower frequency for scan command.
 */
typedef struct
{
	 Tu32		u32ScanStartFreq;
	 Tu32		u32UpperLimit;										/*!< Frequency Upper limit  */
	 Tu32		u32LowerLimit;										/*!< Frequency Lower limit  */

}Ts_DabTuner_ScanInput;

/**		
* @brief enum describes Station Not available Strategy Status		
*/		
typedef enum		
{		
    DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_END,	/**<0- Enum, for Strategy Process Started*/ 	
    DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_START,	/**<1- Enum, for Strategy Process Ended*/ 	
    DAB_TUNER_CTRL_STATIONNOTAVAIL_STRATEGY_INVALID,	/**<2- Enum, for Strategy Status Invalid*/ 
	DAB_TUNER_CTRL_FM_DAB_PI_RECEIVED		
	
}Te_DAB_Tuner_Ctrl_StationNotAvailStrategyStatus;

/**		
* @brief enum describes AF Operaration Status in Updated Learn Memory		
*/		
typedef enum		
{			
	DAB_TUNER_CTRL_LEARN_MEM_AF_SUCCESS,	/**<0- Enum, for AF operation SUCCESS in Updated Learn Memory*/ 	
    DAB_TUNER_CTRL_LEARN_MEM_AF_FAIL,		/**<1- Enum, for AF operation FAIL in Updated Learn Memory*/ 	
    DAB_TUNER_CTRL_LEARN_MEM_AF_INVALID		/**<2- Enum, for AF Operaration Status in Updated Learn Memory Invalid*/	
	
}Te_DAB_Tuner_Ctrl_LearnMemAFStatus;


/**
 * @brief This enum describes the signal status. Ex: Signal low/Signal high
 */

typedef enum
{
    DAB_TUNER_CTRL_SIGNAL_LOW = 1,  				
    DAB_TUNER_CTRL_SIGNAL_HIGH
	          	
}Te_DAB_Tuner_Ctrl_SignalStatus;


typedef enum
{
    DAB_TUNER_CTRL_DAB_DAB_TUNE_INVALID, 
	DAB_TUNER_CTRL_DAB_ALT_TUNE_STARTED,                         
    DAB_TUNER_CTRL_DAB_DAB_TUNE_SUCCESS,                     
    DAB_TUNER_CTRL_DAB_DAB_TUNE_FAILURE 
	      
}Te_DAB_Tuner_Ctrl_DAB_DAB_Status;

typedef enum
{
	DAB2DAB_ALTFREQ_TUNED,
	DAB2DAB_ALTFREQ_SORTED,
	DAB2DAB_ALTFREQ_BER_SORTED,
	DAB2DAB_ALTEID_FREQ_TUNED,
	DAB2DAB_ALTEID_FREQ_SORTED,
	DAB2DAB_ALTEID_BER_SORTED,
	DAB2DAB_HARDLINK_FREQ_TUNED,
	DAB2DAB_HARDLINK_FREQ_SORTED,
	DAB2DAB_HARDLINK_FREQ_BER_SORTED,	
	DAB2DAB_ORIGINAL_FREQ_TUNED,
	DAB2DAB_BEFORE_CHECK_ORIGINAL_TUNED,
	DAB2DAB_AFTER_CHECK_ORIGINAL_TUNED
	
}Te_DAB2DAB_Linking_Status;

typedef struct
{
	Tu16											u16_SId;                					/*Service Id*/
	Tu16											u16_EId;                					/*Ensemble Id*/  
	Tu16											u16_SCIdI;               					/*Service Component Id*/
	Tu8 											u8_Freq_Index;     	 						/*Frequency in kHz*/  
	Tu8												u8_ECC;
	
}Ts_Tuner_Ctrl_Tunableinfo;

#endif