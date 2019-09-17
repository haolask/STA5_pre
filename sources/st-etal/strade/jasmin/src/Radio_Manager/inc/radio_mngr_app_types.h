/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file radio_mngr_app_types.h																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Manager	Application															*
*  Description			: This header file contains type definitions,enumerations and structure template	*
*						  definition which are used in Request,Response and Notification APIs of Radio 		*
*						  Manager application component			 											*
*																											*
*************************************************************************************************************/
#ifndef RADIO_MNGR_APP_TYPES_H
#define RADIO_MNGR_APP_TYPES_H

/*-----------------------------------------------------------------------------
					File Inclusions
-----------------------------------------------------------------------------*/
#include "cfg_types.h"
#include "hsm_api.h"




/*-----------------------------------------------------------------------------
					Macro Definitions
-----------------------------------------------------------------------------*/
/**
 * @brief Maximum characters present in the Physical channel-name, such as "5A", "11N"
 */
#define RADIO_MNGR_APP_CHAN_NAME             			(8u)

/**
 * @brief Maximum number of bytes for a single UTF character
 */	
#define MAX_NO_BYTES_PER_UTF							(4u)			
						
/**
 * @brief Number of bytes/characters in labels
 */
#define RADIO_MNGR_APP_NUMCHAR_LABEL         			(17u)	

/**
 * @brief Number of bytes/characters in component label after merging
 */
#define RADIO_MNGR_APP_COMPONENT_LABEL         			(32u)							

/**
 * @brief Maximum size of Service List in Ensembel
 */		
#define RADIO_MNGR_APP_MAX_SRVL_SIZE         			(64u)                  				

/**
 * @brief This defines the length of the max AM SL
 */
#define RADIO_APP_AM_MAX_STL_SIZE     					(60u)

/**
 * @brief This defines the length of the max FM SL
 */
#define RADIO_APP_FM_MAX_STL_SIZE     					(60u)

/**
 * @brief This defines the length of the max DAB SL
 */
#define RADIO_APP_DAB_MAX_STL_SIZE     					(200u)

/**
 * @brief This macro defines the value zero
 */
#define RADIO_MNGR_APP_VALUE_ZERO  						(0u)

/**
 * @brief This macro defines the value one
 */
#define RADIO_MNGR_APP_STL_SEARCH_FOUND_ONE  			(1u)

/**
 * @brief This macro defines the value one
 */
#define RADIO_MNGR_APP_AMFM_CANCEL						(1u)

/**
 * @brief This macro defines the value two
 */
#define RADIO_MNGR_APP_DAB_CANCEL						(2u)

/**
 * @brief This macro defines the value non zero
 */
#define RADIO_APP_VALUE_NONZERO    						(1u)

/**
 * @brief This Macro defines the Maximum size of PI List
 */		
#define RADIO_MNGR_APP_MAX_PI_LIST						(12u) 

/**
 * @brief Maximum size of Mixed Preset List AM/FM/DAB
 */		
#define RADIO_MNGR_APP_MAX_PSML_SIZE					(15u)  

/**
 * @brief Maximum size of Radio Text characters for FM
 */		
#define RADIO_MNGR_APP_CHAN_RADIOTEXT					(64u)

/**
* @brief Maximum size of program name for FM
*/
#define RADIO_MNGR_APP_CHAN_PTYNAME						(9u)
/**
 * @brief Maximum size of af List for FM
 */		
#define RADIO_MNGR_APP_MAX_AFLST_SIZE					(25u)

/**
 * @brief Maximum size of af List for alternate frequency in DAB
 */	
#define RADIO_MNGR_APP_MAX_ALT_FREQUENCY                (5u)

/**
 * @brief Maximum size of af List for alternate ensemble in DAB
 */	
#define RADIO_MNGR_APP_MAX_ALT_EID                      (5u)

/**
 * @brief Maximum size of af List for hardlink in DAB
 */	
#define RADIO_MNGR_APP_MAX_HARDLINK_SID                 (12u)

/**
 * @brief Maximum size DLS Data 
 */		
#define RADIO_MNGR_APP_MAX_DLS_DATA						(128u)

/**
 * @brief This defines macro value as one for cold start done
 */
#define COLD_START_DONE_ALREADY							(1u)

/**
 * @brief This macro defines value zero considered for cold start up
 */		
#define RADIO_MNGR_APP_COLD_START						(0u)

/**
 * @brief Warm Startup 0XAA write-shutdown/read-startup in/from NVM.
 */		
#define RADIO_MNGR_APP_WARM_START						(0XAA)

/**
 * @brief Warm Startup 0XAA write-shutdown/read-startup in/from first byte of NVM.
 */		
#define RADIO_MNGR_APP_WARM_START_BLOCKONE				(0XAA)

/**
 * @brief Warm Startup 0XBB write-shutdown/read-startup in/from second byte of NVM.
 */		
#define RADIO_MNGR_APP_WARM_START_BLOCKTWO				(0XBB)

/**
 * @brief Warm Startup 0XCC write-shutdown/read-startup in/from third byte of NVM.
 */		
#define RADIO_MNGR_APP_WARM_START_BLOCKTHREE			(0XCC)
	
#define START_TYPE										(01u)

/**
 * @brief LSM Station Info offset address
 */		
#define LSM_STATION_INFO								(02u)	

/**
 * @brief StationList offset address
 */		
#define STATION_LIST_ALL_BANDS							(03u)

/**
 * @brief Preset List offset address
 */		
#define PRESET_MIXED_LIST								(04u)

/**
 * @brief settings offset address
 */		
#define SETTINGS										(05u)

/**
 * @brief Start Type like Initial select band/Normal
 */		
#define SELECTBAND_INIT									(01u)

/**
 * @brief Start Type like Initial select band/Normal
 */		
#define SELECTBAND_NORMAL								(255u)

/**
 * @brief Mute Done completed successfully
 */		
#define MUTE_DONE										(1u)  

/**
 * @brief System is in FM and DAB scan progress
 */		
#define FM_DAB_IN_SCAN									(1u)

/**
 * @brief DAB and FM both scan done
 */		
#define FM_DAB_BOTH_SCAN_DONE							(2u)

/**
 * @brief TA Announcement is ON
 */		
#define RADIO_MNGR_APP_TA_ON_INFO_OFF					(Tu16)(0x0002)

/**
 * @brief Info Announcement is ON
 */		
#define RADIO_MNGR_APP_TA_OFF_INFO_ON					(Tu16)(0xFFFD)

/**
 * @brief Info Announcement is ON
 */		
#define RADIO_MNGR_APP_TA_ON_INFO_ON					(Tu16)(0xFFFF)

/**
 * @brief Maximum value
 */		
#define RADIO_MNGR_APP_TU8_MAX_VALUE					(0xFF)

/**
 * @brief This macro defines the Good Quality signal
 */		
#define GOOD_QUALITY									(1u)

/**
 * @brief This macro defines the Low Quality signal
 */		
#define LOW_QUALITY										(0u)

/**
 * @brief This macro defines the External Scan Request
 */		
#define RADIO_MNGR_APP_EXTERNAL_SCAN_REQ				(1u)


/**
 * @brief  Flag for showing scan as cancelled
 */		
#define RADIO_MNGR_APP_SCAN_CANCELLED					(1u)

 /**
 * @brief  Flag for showing seek as cancelled
 */		
#define RADIO_MNGR_APP_SEEK_CANCELLED					(1u)  

/**
 * @brief  Flag for showing AF Tune as cancelled
 */		
#define RADIO_MNGR_APP_AF_TUNE_CANCELLED				(1u)

/**
 * @brief  Flag for showing Tune as cancelled
 */		
#define RADIO_MNGR_APP_TUNE_CANCELLED					(1u)

/**
 * @brief Macro showing maximum size of channel name for DAB
 */	
#define RADIO_MNGR_APP_MAX_CHANNEL_NAME_SIZE			(4u)

/**
 * @brief Flag for timer is started
 */	
#define RADIO_MNGR_APP_SET_TIMER_FLAG					(1u)

/**
 * @brief Flag for timer is stopped
 */	
#define RADIO_MNGR_APP_CLEAR_TIMER_FLAG					(0u)

/**
 * @brief EEPROM Known valus - Validation Success
 */		
#define EEPROM_KNOWN_VALUES								(1u)

/**
 * @brief EEPROM Known valus - Validation Fail
 */		
#define EEPROM_UNKNOWN_VALUES							(0u)

/**
 * @brief Macro showing maximum time to wait for FM quality resume back
 */	
#define RADIO_MNGR_APP_FM_MAX_TIME_QUALITY_RESUME		((Tu16)60000)

/**
 * @brief Macro showing maximum time to Decode the FM PI
 */	
#define RADIO_MNGR_APP_FM_MAX_TIME_PI_DECODE		((Tu16)3000)

/**
 * @brief Macro showing maximum time to wait for DAB quality resume back
 */	
#define RADIO_MNGR_APP_DAB_MAX_TIME_QUALITY_RESUME		((Tu16)10000)

/**
 * @brief Macro showing timer name
 */	
#define RADIO_MNGR_APP_TIMER							(8u)

/**
 * @brief Macro gives DAB Band Not Support w.r.t Comp Info
 */	
#define RADIO_MANAGER_DAB_BAND_NOT_SUPPORTED			(0u)

/**
 * @brief Macro gives DAB Band Support w.r.t Comp Info
 */	
#define RADIO_MANAGER_DAB_BAND_SUPPORTED				(1u)

/**
 * @brief Macro gives AM Band Not Support w.r.t Diag Param and Activate/Deactivate Inputs
 */	
#define RADIO_MANAGER_AM_BAND_NOT_SUPPORTED				(0u)

/**
 * @brief Macro gives AM Band Support w.r.t Diag Param and Activate/Deactivate Inputs
 */	
#define RADIO_MANAGER_AM_BAND_SUPPORTED					(1u)

/**
 * @brief This macro defines the value one when Info anno setting is received
 */
#define RADIO_MNGR_APP_INFO_ANNO_REQ  					(1u)

/**
 * @brief This macro clears the info anno flag
 */
#define RADIO_MNGR_APP_CLEAR_INFO_ANNO_FLAG				(0u)


/**
 * @brief Macro represents the Maximum Length of DABTuner software Firmware version 
 */
#define RADIO_MNGR_APP_MAX_DABTUNER_SW_VERSION			(48u)

/**
 * @brief Macro represents the Maximum Length of DABTuner hardware firmware version 
 */
#define RADIO_MNGR_APP_MAX_DABTUNER_HW_VERSION			(10u)

/**
 * @brief This macro defines the value one when Station list index matched with already added in favorite list to high late 
 */
#define RADIO_MNGR_APP_MATCHED_PRESET_IN_STL  			(1u)

/**
 * @brief This macro defines the value zero when Station list index NOT matched with already added in favorite list 
 */
#define RADIO_MNGR_APP_NOT_MATCHED_PRESET_IN_STL		(0u)

/**
 * @brief This macro defines the value one when currently playing Station matched in station list to high late 
 */
#define RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_MATCH_IN_STL  			(1u)

/**
 * @brief This macro defines the value zero when currently playing Station not matched in station list to high late 
 */
#define RADIO_MNGR_APP_CURRENTLY_PLAYING_STATION_NOT_MATCH_IN_STL		(0u)

/**
 * @brief This defines the length of the max FM non RDS Stl
 */
#define RADIO_APP_FM_NONRDS_STL_SIZE     								(60u)

/**
 * @brief This macro defines return value for normal stationlist
 */
#define RADIO_MNGR_NORMAL_STATIONLIST   								(1u)

/**
 * @brief This macro defines return value for Multiplex stationlist
 */
#define RADIO_MNGR_MULTIPLEX_STATIONLIST   								(2u)

/**
 * @brief This macro defines ensemble select is requested or not
 */
#define RADIO_MNGR_ENSEMBLE_SELECT_REQUEST_RECEIVED   					(1u)

 /**
 * @brief  NVM StarttypeBlock, Settings, StL and Preset list size
 */		
#define RADIO_MNGR_APP_NVM_DATA_SIZE					(sizeof(Tu32) + sizeof(Ts_Radio_Mngr_App_RadioStationList) + sizeof(Ts_Radio_Mngr_App_DAB_MultiplexStationList) + sizeof(Ts_Radio_Mngr_App_Preset_Mixed_List))

/**
 * @brief  NVM Last Mode info size
 */		
#define RADIO_MNGR_APP_NVM_LASTMODE_SIZE				(sizeof(Ts_Radio_Mngr_Tunuable_Station_Info))

/**
 * @brief This macro defines that Preset Recallis success for FM band
 */
#define RADIO_MNGR_APP_PRESET_FM_SUCCESS				(1u)
#define RADIO_MNGR_APP_DAB_SCAN_REQUESTED				(1u)

/**
 * @brief This macro defines that Preset Recallselection is success for FM band
 */
#define RADIO_MNGR_APP_STATIONLIST_FM_SUCCESS			(2u)

#define PI_DECODE_TIMER_STARTED							(1u)

#define RADIO_MNGR_APP_AUDIO_SWITCH_NOT_NEEDED			(1u)

#define RADIO_MNGR_APP_AUDIO_SWITCH_NEEDED				(0u)

#define RAIO_MNGR_APP_TUNE_CANCEL_REQUESTED				(1u)

#define RADIO_MNGR_APP_FM_TO_DAB_STARTED				(1u)

#define RADIO_MNGR_APP_FM_TO_DAB_STOPPED				(0u)

/**
 * @brief This macro defines the maximum number of multiplex
 */
#define RADIO_MNGR_MULTIPLEX_ENSEMBLES					(20u)

/**
 * @brief This macro defines the maximum number of multiplex
 */
#define RADIO_MNGR_MAX_MULTIPLEX_STATIONS				(64u)

/**
 * @brief Character Set types values 
 */	
#define  RADIO_MNGR_APP_CHARSET_EBU						(Tu8)1
#define  RADIO_MNGR_APP_CHARSET_UCS2                    (Tu8)2
#define  RADIO_MNGR_APP_CHARSET_UTF8                    (Tu8)3

#define RADIO_MNGR_APP_NORMAL_RSSI		    			(Tu16)(16+(18*2))
#define RADIO_MNGR_APP_NORMAL_WAM 						(Tu16)((255*25)/100)
#define RADIO_MNGR_APP_NORMAL_USN						(Tu16)((255*20)/100)

/* As per DAB Standard SLS image maximum size is 50 KB */
#define	RADIO_MNGR_APP_MAX_DATA_SERVICE_PAYLOAD_SIZE	51200


/*-----------------------------------------------------------------------------
					Type Definitions
-----------------------------------------------------------------------------*/		

/**
 * @brief This enum describes variant types
 */
typedef enum
{
	VARIANTA_RM,											/*Variant A*/										                         														
    VARIANTB_RM,											/*Variant B*/                													
    VARIANTC_RM,                           					/*Variant C*/									
    VARIANTD_RM,											/*Variant D*/
	VARIANT_INVALID_RM,										/*Variant INVALID*/
	                           													
}Te_Radio_Mngr_App_Variant;

/**
 * @brief This enum describes different markets
 */
typedef enum
{
    RCE_MARKET_WESTERN_EUROPE_RM,  							/*0-Market Wester Europe*/													
    RCE_MARKET_LATIN_AMERICA_RM,							/*1-Market Latin America*/     														
    RCE_MARKET_ASIA_CHINA_RM,								/*2-Market Asia China*/  																
    RCE_MARKET_ARABIA_RM,									/*3-Market Arabia*/ 																
    RCE_MARKET_USA_NORTHAMERICA_RM,							/*4-Market USA NorthAmerica*/         													
	RCE_MARKET_JAPAN_RM,									/*5-Market Japan*/ 														
	RCE_MARKET_KOREA_RM,									/*6-Market Korea*/
	RCE_MARKET_BRAZIL_RM,									/*7-Market Brazil*/
	RCE_MARKET_SOUTHAMERICA_RM,								/*8-Market South america*/
	RCE_MARKET_INVALID_RM,									/*9-Market Invalid*/
	            														
}Te_Radio_Mngr_App_Market;
 

/**
 * @brief This enum describes different types of BANDS
 */
typedef enum
{  
 
    RADIO_MNGR_APP_BAND_AM,  									/*0-AM Band*/															
	RADIO_MNGR_APP_BAND_FM,  									/*1-FM Band*/								
	RADIO_MNGR_APP_BAND_DAB, 									/*2-DAB Band*/
	RADIO_MNGR_APP_RADIO_MODE,                                  /*3-Radio Mode*/
	RADIO_MNGR_APP_NON_RADIO_MODE,                              /*4-Non Radio Mode*/
	RADIO_MNGR_APP_BAND_INVALID,								/*6-Invalid Band*/
	  																
}Te_Radio_Mngr_App_Band;

/**
 * @brief This enum describes the Radio Manager Application Request ID's.
 */
typedef enum
{
	RADIO_MNGR_APP_STATIONLIST_SELECT = 1,						/**<1.Request ID for the select Stn from StL with Index*/
	RADIO_MNGR_APP_PLAY_SELECT_STATION,							/**<2.Request ID for the Play Select Station*/
	RADIO_MNGR_APP_SEEK_UPDOWN,									/**<3.Request ID for the Seek Up/Down */
	RADIO_MNGR_APP_INIT_SCAN,									/**<4.ID to know the Scan gets failed once*/
	RADIO_MNGR_APP_SELECTBAND,									/**<5.Request ID for the Select Band*/
	RADIO_MNGR_APP_PRESET_RECALL,								/**<6.Request ID for the Preset Recall*/	
	RADIO_MNGR_APP_UPDATE_STLIST,								/**<7.Request ID for the Manual Refrest Station List*/
	RADIO_MNGR_APP_TUNEUPDOWN,									/**<8.Request ID for the Tune Up/Down*/
	RADIO_MNGR_APP_SELECT_STATION_END,							/**<9.Request ID for the select station end during tune to original*/
	RADIO_MNGR_APP_SERVICE_RECONFIG,							/**<a.Request ID for the service reconfiguration*/
	RADIO_MNGR_APP_RADIOMODE,									/**<b.Request ID for the Radio mode*/
	RADIO_MNGR_APP_NONRADIOMODE,								/**<c.Request ID for the Non radio mode*/
	RADIO_MNGR_APP_TUNEBYFREQ,									/**<d.Request ID for the Tune By Frequency*/
	RADIO_MNGR_APP_RADIO_POWER_ON,								/**<e.Request ID for the Tune By Frequency*/
	RADIO_MNGR_APP_RADIO_POWER_OFF,								/**<f.Request ID for the Tune By Frequency*/
	RADIO_MNGR_APP_IN_STRATEGY,									/**<10.RM IN STRATEGY*/
	RADIO_MNGR_APP_PI_NOT_FOUND_AF_TUNE,						/**<11.PI Not found after 3sec then mute to AF handler mapping*/
	RADIO_MNGR_APP_PLAY_STATION_INSEARCHED_STL,					/**<12.Request ID for the select Stn from searched StL with Index*/
	RADIO_MNGR_APP_AUTOSCAN_PLAY_STATION,						/**<12.Request ID FOR to play the first service for each scan ensemble */
	RADIO_MNGR_APP_INVALID,										/**<12.Request ID Invalid*/
}Te_Radio_Mngr_App_Req_Id;										


/**
 * @brief This enum describes the Best PI Types.
 */
typedef enum
{
    RADIO_MNGR_APP_NORMAL_PI,									/**<0.Notify Enum, Normal PI */
    RADIO_MNGR_APP_IMPLICIT_SID,								/**<1.Notify Enum, Implicit PI */
	RADIO_MNGR_APP_NO_IMPLICIT									/**<2.Notify Enum, No Implicit */

}Te_Radio_Mngr_App_BestPI_Type;

/**
 * @brief This enum describes the PSN content changed or not during DAB FM linking.
 */
typedef enum
{    
    RADIO_MNGR_APP_SAME_PSN,                					/* 0-Notify Enum,PSN is not changed,same as existing */
    RADIO_MNGR_APP_NEW_PSN,                						/* 1-Notify Enum,PSN is changed */
}Te_Radio_Mngr_App_PSNChange;

/**
 * @brief This enum describes various AF Status Types.
 */
typedef enum
{
	RADIO_MNGR_APP_AF_LIST_AVAILABLE,							/**<0.Notify Enum, AF list available */
    RADIO_MNGR_APP_AF_LIST_BECOMES_ZERO,						/**<1.Notify Enum, AF list zero */
    RADIO_MNGR_APP_AF_LIST_EMPTY,								/**<2.Notify Enum, AF list empty */
	RADIO_MNGR_APP_AF_LINK_INITIATED,							/**<3.Notify Enum, AF link initiated */
    RADIO_MNGR_APP_AF_LINK_ESTABLISHED,							/**<4.Notify Enum, AF link established */
    RADIO_MNGR_APP_DAB_LINK_ESTABLISHED,						/**<5.Notify Enum, DAB link established */
    RADIO_MNGR_APP_NO_LINK,										/**<6.Notify Enum, No link available*/
	RADIO_MNGR_APP_PI_CHANGE_DETECTED,
    RADIO_MNGR_APP_PI_CHANGE_LINK_ESTABLISHED
        
}Te_Radio_Mngr_App_AF_Status;

/**
 * @brief This enum describes the Radio Scan Status
 */
typedef enum
{
    RADIO_MNGR_APP_SCAN_STARTED,								/**<Response Enum, for Scanning Status Started*/
    RADIO_MNGR_APP_SCAN_INPROGRESS,								/**<Response Enum, for Scanning Status In Progress*/
    RADIO_MNGR_APP_SCAN_COMPLETE,								/**<Response Enum, for Scanning Status Copleted*/
	RADIO_MNGR_APP_SCAN_FAILED									/**<Response Enum, for Scanning Status Failed*/
}Te_Radio_Mngr_App_ScanStatus;

/**
 * @brief This enum describes the AF switch request.
 */
typedef enum
{        
    RADIO_MNGR_APP_RDS_SETTINGS_ENABLE,							/**<Enum, for the AF switch enable*/							
    RADIO_MNGR_APP_RDS_SETTINGS_DISABLE,						/**<Enum, for the AF switch disable*/							
	RADIO_MNGR_APP_RDS_SETTINGS_INVALID							/**<Enum, for the PAF switch Invalid*/								        
}Te_Radio_Mngr_App_RDSSettings;


/**
 * @brief This enum describes the DAB-FM Linking switch request
 */
typedef enum
{        
    RADIO_MNGR_APP_DABFMLINKING_ENABLE,							/**<Enum, for the DABFM switch enable*/							
    RADIO_MNGR_APP_DABFMLINKING_DISABLE,						/**<Enum, for the DABFM switch disable*/							
	RADIO_MNGR_APP_DABFMLINKING_INVALID,						/**<Enum, for the DABFM switch invalid*/							
}Te_Radio_Mngr_App_DABFMLinking_Switch;



/**
 * @brief This enum describes the Announcement enable switch request
 */
typedef enum
{	
	RADIO_MNGR_APP_TA_ANNO_ENABLE,								/**<Enum, for the Announcement switch enable*/	
	RADIO_MNGR_APP_TA_ANNO_DISABLE,								/**<Enum, for the Announcement switch disable*/	
	RADIO_MNGR_APP_TA_ANNO_REQ_INVALID,							/**<Enum, for the Announcement switch invalid*/	
}Te_Radio_Mngr_App_EnableTAAnno_Switch;

/**
 * @brief This enum describes the Announcement enable switch request
 */
typedef enum
{	
	RADIO_MNGR_APP_INFO_ANNO_ENABLE,							/**<Enum, for the Announcement switch enable*/	
	RADIO_MNGR_APP_INFO_ANNO_DISABLE,							/**<Enum, for the Announcement switch disable*/	
	RADIO_MNGR_APP_INFO_ANNO_REQ_INVALID,						/**<Enum, for the Announcement switch invalid*/	
}Te_Radio_Mngr_App_EnableInfoAnno_Switch;

/**
 * @brief This enum describes the DAB Stationlist Multiplex setting switch request 
 */
typedef enum
{        
    RADIO_MNGR_APP_MULTIPLEX_ENABLE,							/**<Enum, for the DAB Stationlist ensemble view switch enable*/							
    RADIO_MNGR_APP_MULTIPLEX_DISABLE,							/**<Enum, for the DAB Stationlist ensemble view switch disable*/							
	RADIO_MNGR_APP_MULTIPLEX_INVALID							/**<Enum, for the switch Invalid*/								        
}Te_Radio_Mngr_App_Multiplex_Switch;

/**
 * @brief This enum describes the DAB Stationlist Multiplex setting switch Reply status
 */
typedef enum
{        
    RADIO_MNGR_APP_MULTIPLEX_SWITCH_SUCCESS,					/**<Reply Status Enum, for the DAB STL Multiplex setting success*/           
	RADIO_MNGR_APP_DAB_STL_MULTIPLEX_SETTINGS_FAIL,				/**<Reply Status Enum, for the DAB STL Multiplex setting failure*/           					
    RADIO_MNGR_APP_DAB_STL_MULTIPLEX_SETTINGS_RESP_INVALID		/**<Reply Status Enum, for the Invalid*/           					
				
}Te_Radio_Mngr_App_Multiplex_Switch_ReplyStatus;

/**
 * @brief This enum describes the status of the annoucement
 */
typedef enum
{
	RADIO_MNGR_APP_ANNO_START,									/**<0.Notify Enum, AF Announcement started */
	RADIO_MNGR_APP_ANNO_END,									/**<1.Notify Enum, AF Announcement End */
    RADIO_MNGR_APP_ANNOUNCEMENT_END_SIGNAL_LOSS,				/**<2.Notify Enum, AF Announcement signal loss */
    RADIO_MNGR_APP_ANNOUNCEMENT_END_USER_CANCEL,				/**<3.Notify Enum, Announcement cancel by user */
    RADIO_MNGR_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF,				/**<4.Notify Enum, Announcement cancel by switch */
    RADIO_MNGR_APP_ANNOUNCEMENT_NOT_AVAILABLE,					/**<5.Notify Enum, Announcement not available */
	RADIO_MNGR_APP_ANNO_INVALID,								/**<6.Notify Enum, Announcement invalid*/

}Te_Radio_Mngr_App_Anno_Status;


/**
 * @brief This enum describes the DAB-FM SID Types
 */
typedef enum
{
	RADIO_MNGR_APP_DAB_SAME_PI_STATION,								/*Reply status Enum, Same PI*/
	RADIO_MNGR_APP_DAB_REGIONAL_PI_STATION,							/*Reply status Enum, Regional PI*/
	RADIO_MNGR_APP_DAB_PI_STATION_UNIDENTIFIED,						/*Reply status Enum, unidentified*/
	RADIO_MNGR_APP_FMDAB_PI_RECEIVED,
	RADIO_MNGR_APP_TUNED_TO_SAME_STATION,
	RADIO_MNGR_APP_TUNED_STATIONS_SORTED,
	RADIO_MNGR_APP_TUNED_STATION_NOTSTABLE,
	RADIO_MNGR_APP_FMDAB_BLENDING_SUCCESS,
	RADIO_MNGR_APP_FMDAB_BLENDING_CANCELLED,
	RADIO_MNGR_APP_FMDAB_BLENDING_STOP,	
	RADIO_MNGR_APP_FMDAB_PI_INVALID
}Te_Radio_Mngr_App_DABFM_SID_Type;


/**
 * @brief This enum describes the cancelling of requests
 */
typedef enum
{
	RADIO_MNGR_APP_SEEK_CANCEL,										/**<Enum, for the Seek cancel*/
	RADIO_MNGR_APP_SCAN_CANCEL,										/**<Enum, for the Scan cancel*/
	RADIO_MNGR_APP_AF_TUNE_CANCEL,									/**<Enum, for the AF tune cancel*/
	RADIO_MNGR_APP_TUNE_CANCEL,										/**<Enum, for the Tune cancel*/
	RADIO_MNGR_APP_ANNO_CANCEL,										/**<Enum, for the Anno Cancel*/
	RADIO_MNGR_APP_CANCEL_INVALID									/**<Enum, for the invalid cancel*/
	
}Te_Radio_Mngr_App_Cancel_Req_Type;

/**
 * @brief enum describes the cancelling of request modes
 */
typedef enum
{
	RADIO_MNGR_APP_ANNO_CANCEL_BY_NEW_REQUEST,						/**<Enum, for the cancel by new request*/
	RADIO_MNGR_APP_ANNO_CANCEL_BY_HMI,								/**<Enum, for the cancel by HMI*/
	RADIO_MNGR_APP_ANNOCANCEL_INVALID								/**<Enum, for the cancel invalid*/

}Te_Radio_Mngr_App_Anno_Cancel_Request_Type;

/**
 * @brief enum describes the Cancelling of Scan Request Modes
 */
typedef enum
{
	RADIO_MNGR_APP_SCAN_CANCEL_BY_NEW_REQUEST,						/**<0- Enum, for the Scan cancel by new request*/
	RADIO_MNGR_APP_SCAN_CANCEL_BY_HMI,								/**<1- Enum, for the Scan cancel by HMI*/
	RADIO_MNGR_APP_SCAN_CANCEL_INVALID								/**<2- Enum, for the Scan cancel invalid*/

}Te_Radio_Mngr_App_Scan_Cancel_Request_Type;



/**
* @brief enum describes Eng mode request either ON/OFF values 
*/
typedef enum
{
	RADIO_MNGR_APP_ENG_MODE_OFF,									/*0- Enum for ENG mode OFF*/															
	RADIO_MNGR_APP_ENG_MODE_ON 										/*1- Enum for ENG mode ON*/	
}Te_Radio_Mngr_App_Eng_Mode_Request;

/**
* @brief enum describes Radio Activity Status 
*/
typedef enum
{
	RADIO_MNGR_APP_STATION_NOT_AVAILABLE = 1,						/*1-  Enum, Selected Station Not Available*/ 
	RADIO_MNGR_APP_FM_AF_PROCESSING,								/*2-  Enum, Active Band FM, FM AF Processing*/
	RADIO_MNGR_APP_FM_INTERNAL_SCAN_PROCESS,						/*3-  Enum, Active Band FM, internal Scan*/
	RADIO_MNGR_APP_FM_LEARNMEM_AF_AND_DAB_AF_PROCESSING,			/*4-  Enum, Active Band FM, LearnMem FM AF and DAB AF Processing*/
	RADIO_MNGR_APP_DAB_AF_PROCESSING,								/*5-  Enum, Active Band DAB, DAB AF Processing*/
	RADIO_MNGR_APP_DAB_INTERNAL_SCAN_PROCESS,						/*6-  Enum, Active Band DAB, DAB Internal Scan Processing*/
	RADIO_MNGR_APP_DAB_LEARNMEM_AF_AND_FM_AF_PROCESSING,			/*7-  Enum, Active Band DAB, LearnMem DAB AF and FM AF Processing*/
	RADIO_MNGE_APP_FM_LEARNMEM_AF_PROCESSING,						/*8-  Enum, Active Band FM, LearnMem FM AF Proecessing when D<=>F OFF*/
	RADIO_MNGR_APP_DAB_LEARNMEM_AF_PROCESSING,						/*9-  Enum, Active Band DAB, LearnMem DAB AF Proecessing when D<=>F OFF*/
	RADIO_MNGR_APP_SIGNAL_LOSS,										/*10- Enum, Update Signal Loss*/
	RADIO_MNGR_APP_DAB_DAB_STARTED,									/*11- Enum, DAB to DAB Started*/
	RADIO_MNGR_APP_DAB_DAB_LINKING_DONE,							/*12- Enum, DAB to DAB Happended*/
	RADIO_MNGR_APP_LISTENING,										/*13- Enum for Listening Activity Status*/
	RADIO_MNGR_APP_IN_SCAN,											/*14- Enum for Scan Activity Status*/
	RADIO_MNGR_APP_TUNE_UPDOWN,										/*16- Enum for Tune up/down Activity Status*/
	RADIO_MNGR_APP_ANNOUNCEMENT,									/*17- Enum for Announcement Activity Status*/
	RADIO_MNGR_APP_DABFMLINKING_DONE,								/*18- Enum for DAB<=>FM linking Activity Status*/
	RADIO_MNGR_APP_IMPLICIT_LINKING_DONE,							/*19- Enum for Implicit linking Activity Status*/
	RADIO_MNGR_APP_AF_SWITCHING_ESTABLISHED,						/*20- Enum for AF Switching Established*/
	RADIO_MNGR_APP_DABTUNER_ABNORMAL,								/*21- Enum for DABTuner Abnormal*/
	RADIO_MNGR_APP_STATUS_INVALID									/*22- Enum, for HMI Status Update Invalid*/
	
}Te_Radio_Mngr_App_Activity_Status;

/**
* @brief enum describes Service Type 
*/
typedef enum
{
    RADIO_MNGR_APP_SERVICE_TYPE_INVALID,							/*0- Service Type is invalid*/
    RADIO_MNGR_APP_SERVICE_TYPE_DAB,    							/*1- Service Type is DAB*/
    RADIO_MNGR_APP_SERVICE_TYPE_DAB_PLUS,							/*2- Service Type is DAB PLUS*/
    RADIO_MNGR_APP_SERVICE_TYPE_DMB									/*3- Service Type is DMB*/
    
}Te_Radio_Mngr_App_ServiceType;

/**
* @brief enum describes SRC Enable Disable Request Enum 
*/
typedef enum
{        
    RADIO_MNGR_APP_SRC_ACTIVE,                                     /**<0- Enum, for the SRC Active*/                            
    RADIO_MNGR_APP_SRC_DEACTIVE,                                   /**<1- Enum, for the SRC DeActive*/                            
    RADIO_MNGR_APP_SRC_INVALID,                                    /**<2- Enum, for the SRC Invalid*/                                        
}Te_Radio_Mngr_App_SRC_ActivateDeActivate;

/**
* @brief enum describes Strategy Status
*/
typedef enum
{
    RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_END,						/**<0- Enum, for Strategy Process Started*/ 
    RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_START,					/**<1- Enum, for Strategy Process Ended*/ 
	RADIO_MNGR_APP_STATIONNOTAVAIL_STRATEGY_INVALID					/**<2- Enum, for Strategy Status Invalid*/ 

}Te_Radio_Mngr_App_StationNotAvail_StrategyStatus;

/**
* @brief enum describes Strategy Flow
*/
typedef enum
{
    RM_STRATEGY_FLOW_FM_FM_AF = 1,									/**<1- Enum, for Strategy Flow ActiveBand_FM  and FM_AF  Tune Requested*/ 
    RM_STRATEGY_FLOW_FM_DAB_AF,										/**<2- Enum, for Strategy Flow ActiveBand_FM  and DAB_AF Tune Requested*/ 
	RM_STRATEGY_FLOW_DAB_DAB_AF,									/**<3- Enum, for Strategy Flow ActiveBand_DAB and DAB_AF Tune Requested*/ 
	RM_STRATEGY_FLOW_DAB_FM_AF,										/**<4- Enum, for Strategy Flow ActiveBand_DAB and FM_AF  Tune Requested*/
	RM_STRATEGY_FLOW_INVALID										/**<7- Enum, for Strategy Process Started*/

}Te_Radio_Mngr_App_StrategyFlow;

/**
* @brief enum describes AF Operaration Status in Updated Learn Memory
*/
typedef enum
{
    RADIO_MNGR_APP_LEARN_MEM_AF_SUCCESS,							/**<0- Enum, for AF operation SUCCESS in Updated Learn Memory*/ 
    RADIO_MNGR_APP_LEARN_MEM_AF_FAIL,								/**<1- Enum, for AF operation FAIL in Updated Learn Memory*/ 
	RADIO_MNGR_APP_LEARN_MEM_AF_INVALID,							/**<2- Enum, for AF Operaration Status in Updated Learn Memory Invalid*/

}Te_Radio_Mngr_App_LearnMemAFStatus;

/**
* @brief enum describes FM-DAB Linking Stop types
*/
typedef enum
{
    RADIO_MNGR_APP_FMDAB_LINKING_REQ_PI_CHANGE = 1,                   /**<1- Enum, for FM-DAB Linking Stop due to FM PI change*/
    RADIO_MNGR_APP_FMDAB_LINKING_REQ_NEW_REQUEST,                     /**<2- Enum, for FM-DAB Linking Stop due to New Request*/
    RADIO_MNGR_APP_FMDAB_LINKING_REQ_SIG_LOST,                         /**<3- Enum, for FM-DAB Linking Stop due to Signal Lost*/
    RADIO_MNGR_APP_FMDAB_LINKING_REQ_INVALID,                          /**<4- Enum, for FM-DAB Linking Invalid value*/
}Te_Radio_Mngr_App_FMDABLinking_Stop_Type;


/**
 * @brief This enum describes status for foreground band signal low/high
 */
typedef enum
{
    RADIO_MNGR_APP_SIGNAL_LOW	= 1,									/**<Reply Status Enum, for the Signal Low notification*/                                                                                          
    RADIO_MNGR_APP_SIGNAL_HIGH,											/**<Index Enum, for the Signal High notification*/              
    RADIO_MNGR_APP_SIGNAL_INVALID,										/**<Index Enum, for the Signal as Invalid notification*/      
}Te_Radio_Mngr_App_FG_Signal_Status;

/**
 * @brief This enum describes the DAB-DAB start status and tune status
 */
typedef enum
{
    RADIO_MNGR_APP_DAB_ALT_TUNE_INVALID,								/**<0- Enum, for DAB-DAB Invalid status*/ 	
    RADIO_MNGR_APP_DAB_ALT_TUNE_STARTED,								/**<1- Enum, for DAB Alternate is started*/
    RADIO_MNGR_APP_DAB_ALT_TUNE_SUCCESS,                     			/**<2- Enum, for DAB Alternate Tune is success*/
    RADIO_MNGR_APP_DAB_ALT_TUNE_FAILURE       							/**<3- Enum, for DAB Alternate Tune is failure*/
}Te_Radio_Mngr_App_DAB_Alt_Status;

/**
* @brief enum describes DAB Up Notification status
*/
typedef enum
{
    RADIO_MNGR_APP_DAB_UP_NOTIFICATION_NOT_RECEIVED,					/**<0- Enum, DAB Up-Notification not received*/ 
    RADIO_MNGR_APP_DAB_UP_NOTIFICATION_RECEIVED,						/**<1- Enum, DAB Up-Notification Received*/ 
	RADIO_MNGR_APP_DAB_UP_NOTIFICATION_INVALID							/**<2- Enum, DAB Up-Notification Invalid*/ 

}Te_Radio_Mngr_App_DAB_UpNotification_Status;

/**
* @brief enum describes search stationlist type
*/
typedef enum
{
	RADIO_MNGR_APP_FM_STL_SEARCH = 1,  									/**<i-FM RDS stationlist search*/								
	RADIO_MNGR_APP_DAB_STL_SEARCH, 										/**<2-DAB stationlist search*/
	RADIO_MNGR_APP_NON_RDS_STL_SEARCH,	  								/**<3-FM Non RDS stationlist search*/
	RADIO_MNGR_APP_STL_SEARCH_INVALID	  								/**<4-stationlist search invalid*/	
}Te_Radio_Mngr_App_STL_Search_Type;

/**
 * @brief This structure describes Station label 
 */
typedef struct
{
	Tu16  											u16_ShortLabelFlags;                		/*Short Label flags*/
	Tu8  											au8_Label[RADIO_MNGR_APP_NUMCHAR_LABEL];	/*Label in Bytes*/
	Tu8												u8_CharSet;									/*Flag for the Label Conversion*/
	Tu8												u8_padding[1];								/*Padding data*/
}Ts_Radio_Mngr_App_StationLabel;

/**
 * @brief This structure describes Timer ids
 */
typedef struct
{
	Tu32											u32_LowSig_ClearLabel_Timerid;                      /*Clear the PSN label and RT timer id*/			
	Tu32											u32_PI_Decode_Timerid;								/*Check for PI Decode*/		
}Ts_Radio_Mngr_App_TimerIds;
/**
 * @brief This structure describes DAB Component Name
 */
typedef struct
{
	Tu8												u8_CharSet;									/*Flag for the Label Conversion*/
	Tu8												au8_Padding[3];								/*Padding data*/
	Tu8											    au8_CompLabel[RADIO_MNGR_APP_COMPONENT_LABEL];/*Component Label*/	
}Ts_Radio_Mngr_App_ComponentName;



/**
 * @brief This structure describes DAB station information.
 */
typedef struct
{
	Ts_Radio_Mngr_App_ComponentName					st_ComponentName;							/*Component Name with Srv and Sc*/
	Tu32          									u32_Frequency;                    			/* Frequency in KHz */	
	Tu32                               				u32_Sid;									/* Service id */
	Tu16                              				u16_EId;									/* Ensemble id */
	Tu16                               				u16_SCIdI;									/* Service component ID */
	Tu8												au8_ChannelName[RADIO_MNGR_APP_MAX_CHANNEL_NAME_SIZE];  /*Channel Name*/
	
}Ts_Radio_Mngr_App_DAB_StationInfo;



/***************************************************Current Station Info Structures Started*******************************************/
/**
 * @brief Structure comprises information used for current station info of select station 
 */
typedef struct
{
	Tu32 											u32_Frequency;     	 						/*Frequency in kHz*/  
	Tu32											u32_SId;                					/*Service Id*/
	Tu16											u16_EId;                					/*Ensemble Id*/  
	Tu16											u16_SCIdI;               					/*Service Component Id*/
	Tu8												u8_ECC;           		 					/*Extended County Code*/
	Tu8												au8_padding[3];								/*Padding Bytes*/
} Ts_Radio_Mngr_App_DAB_Tunable_StationInfo;

/**
* @brief Structure contains information about current tuned station like ensemble info , service info , service component info 
*/
typedef struct
{
	Te_Radio_Mngr_App_ServiceType					e_ServiceType;
	Ts_Radio_Mngr_App_DAB_Tunable_StationInfo		st_Tunableinfo;											/*DAB Tunable station info*/
	Ts_Radio_Mngr_App_StationLabel  				st_EnsembleLabel;										/*Ensemble label*/	
    Ts_Radio_Mngr_App_StationLabel      			st_ServiceLabel;                        				/*Service  label*/
	Ts_Radio_Mngr_App_StationLabel      			st_ComponentLabel;                      				/*Component label*/
	Tu8												au8_ChannelName[RADIO_MNGR_APP_MAX_CHANNEL_NAME_SIZE];  /*Channel Name*/
}Ts_Radio_Mngr_App_DAB_CurrentStationInfo;



/**
 * @brief Structure  holds Information of an AM station 
 */
typedef struct
{
	Tu32											u32_Freq;									/*AM Frequency*/
}Ts_Radio_Mngr_App_AM_StationInfo;


/**
 * @brief This structure contains detailed information about currently tuned FM station
 */	
typedef struct
{
	Tu32   											u32_frequency;								/*Frequency to be tuned in kHz*/
  	Tu16                  							u16_PI;  									/*Program Identifier*/
	Tu16											u16_padding;								/**Padding Data*/
	Tu8												au8_PSN[RADIO_MNGR_APP_CHAN_NAME];			/*Program station Name */						
	
}Ts_Radio_Mngr_App_FMStationInfo;


/**
 * @brief structure of AF list
 */
typedef struct
{
	Tu8												u8_NumAFList;								/*Total no of AF stations*/
	Tu8												u8_Padding[3];								/*Padding Data*/
	Tu32											au32_AFList[RADIO_MNGR_APP_MAX_AFLST_SIZE];	/* Array of AF frequencies*/
}Ts_Radio_Mngr_App_AFList;


/**
 * @brief structure holds either AM or FM station
 */
typedef struct
{
	Te_Radio_Mngr_App_Band							e_Band;										/*Band of Current Station info*/

	union
	{
		Ts_Radio_Mngr_App_AM_StationInfo			st_AMCurrentStationInfo;					/*AM Current Station info*/
		Ts_Radio_Mngr_App_FMStationInfo				st_FMCurrentStationInfo;					/*FM Current Station info*/
	}un_station;

	Ts32								            s32_BBFieldStrength;						/*	Field Strength (RSSI)*/
	Ts32											s32_RFFieldStrength;
	Tu32  			   								u32_UltrasonicNoise;						/*	USN */
	Tu32											u32_Multipath;								/*  wide-band AM detector */
	Tu8												u8_POR_Status;								/*  Quality of the station */
	Tu32											u32_FrequencyOffset;						/*  Frequency offset in kHz */
	Tu8												u8_IF_BW;									/*  IF bandwidth */
	Tu32											u32_AdjacentChannel;
	Tu32											u32_SNR;
	Tu32											u32_coChannel;
	Tu32											u32_StereoMonoReception;
	Tu32											u32_ModulationDetector;						/* 	Modulation */
    Tu8												u8_CharSet;									/*Char Set flag for the conversion*/
	Tu8												u8_PTY_Code;
	Tu8												au8_Prgm_Name[RADIO_MNGR_APP_CHAN_PTYNAME];  /*FM Program Name*/
	Tu8												au8_RadioText[RADIO_MNGR_APP_CHAN_RADIOTEXT];/*Radio Text*/	
	Ts_Radio_Mngr_App_AFList						st_AFList;									/* FM Alternative Frequency List*/
	Tu8								                u8_TA;										/*Traffic Announcement for FM Announcement status*/
    Tu8 							                u8_TP;										/*Traffic Programme for FM Announcement status*/
	Tu8                        						au8_padding[2];                             /*  Two bytes padded */

}Ts_Radio_Mngr_App_AMFM_CurrentStationInfo;


/****************************************************Current Station Info Structures Ended***********************************************/




/****************************************************Station List Structures  For Diag Request Started*************************************/
/**
 * @brief This structure describes station list of AM Band
 */
typedef struct
{
    Ts_Radio_Mngr_App_AM_StationInfo    			ast_Stations[RADIO_APP_AM_MAX_STL_SIZE];  	/* stations present in the AM station list*/
	Tu8	                          					u8_numberStationsInList;                   	/* Number of stations present in DAB station list */
	Tu8												u8_padding[3];								/*padding bytes*/

}Ts_Radio_Mngr_App_AM_SL;



/**
 * @brief This structure describes station list of FM Band
 */
typedef struct
{
	Ts_Radio_Mngr_App_FMStationInfo    				ast_Stations[RADIO_APP_FM_MAX_STL_SIZE];  	/*Stations present in the FM station list */
    Tu8	                         					u8_numberStationsInList;                   	/*Number of stations present in DAB station list */
	Tu8												u8_CharSet;									/*Flag for the PSN, RT Conversion*/
	Tu8												u8_padding[2];								/*padding bytes*/
}Ts_Radio_Mngr_App_FM_SL;


/**
 * @brief This structure describes DAB station information without merging label.
 */
typedef struct
{
	   Tu32      									u32_Frequency ;								/*Frequency in kHz*/                                  
	   Tu32      									u32_Sid ;  									/*Service Id*/                                      
	   Tu16      									u16_EId ;  									/*Ensemble Id*/                                      
	   Tu16      									u16_SCIdI ;									/*Service Component Id*/  
	   Tu8        									au8_SrvLabel[RADIO_MNGR_APP_NUMCHAR_LABEL]; /*Service  label*/          
	   Tu8        									au8_CompLabel[RADIO_MNGR_APP_NUMCHAR_LABEL];/*Component label*/                                         
	   Tu8        									u8_ECC ;									/*Extended County Code*/        
	   Tu8        									u8_CharSet;  								/*Character set*/                               
	   Ts8     										s8_RSSI ;									/**< Signal strength */                               
	   Tu8        									u8_padding;  							/*Padding Bytes*/                                                             
 
}Ts_Radio_Mngr_App_DAB_StnInfo ;
/*
 * @brief This structure describes station list of DAB Band without merging label
 */
typedef struct
{
    Ts_Radio_Mngr_App_DAB_StnInfo        			ast_Stations[RADIO_APP_DAB_MAX_STL_SIZE];  	/*DAB station list info*/            
    Tu8          									u8_numberStationsInList;             		/*total number of DAB stl*/   
    Tu8          									u8_padding[3];  							/*Padding Bytes*/      
}Ts_Radio_Mngr_App_DAB_SL;

/**
 * @brief This structure describes All Bands Stations
 */
typedef struct
{
	Ts_Radio_Mngr_App_AM_SL							st_AM_StationList;							/*AM station list info*/                         
	Ts_Radio_Mngr_App_FM_SL							st_FM_StationList;							/*FM station list info*/ 
	Ts_Radio_Mngr_App_DAB_SL						st_DAB_StationList;							/*DAB station list info*/ 
}Ts_Radio_Mngr_App_RadioStationList;


/**
 * @brief This structure describes All DAB AF List during ENG Mode ON
 */
typedef struct
{
	Tu32											au32_AltFrequency[RADIO_MNGR_APP_MAX_ALT_FREQUENCY];  /*Alternate frequencylist */
	Tu16											au16_AltEnsemble[RADIO_MNGR_APP_MAX_ALT_EID];         /*Alternate ensemblelist */                                     
	Tu32											au32_HardlinkSid[RADIO_MNGR_APP_MAX_HARDLINK_SID];	  /*Hardlinksid */
	Tu8												u8_NumAltFreq;										  /*Number of DAB alternate frequencies*/
	Tu8												u8_NumAltEnsemble;									  /*Number of DAB alternate frequencies*/
	Tu8												u8_NumAltHardLinkSId;								  /*Number of DAB alternate frequencies*/
}Ts_Radio_Mngr_App_DAB_AFList;

/**
 * @brief This structure describes All FM AF List
 */
typedef struct
{
	Tu8												u8_NumAFList;								/*Total no of AF stations*/
	Tu8												u8_Padding[3];								/*Padding Data*/
	Tu32											au32_AFList[RADIO_MNGR_APP_MAX_AFLST_SIZE];	/* Array of AF frequencies*/
	Tu16											au16_PIList[RADIO_MNGR_APP_MAX_AFLST_SIZE];	/* Array of AF PI*/
	Tu32											au32_Quality[RADIO_MNGR_APP_MAX_AFLST_SIZE];/* Array of AF Quality*/
}Ts_Radio_Mngr_App_FM_AFList;

/**
 * @brief This structure describes One AF Station Info
 */
typedef struct
{
	Tu32											u32_AFFreq;									/*AF Freq Value*/
	Tu16    										u16_PI;										/*AFs PI*/
	Tu32      										u32_AvgQual;								/*AFs avarage Quality*/
	Tu8      										u8_Padding[3];								/*Padding Data*/
}Ts_Radio_Mngr_App_FMAFStationInfo;

/**
 * @brief This structure describes All FM AF Stations from APP TO RM
 */
typedef struct
{
	Tu8     										u8_NumAFList;								/*Number of AF list*/
	Tu8     										u8_Padding[3];    							/*Padding data*/
    Ts_Radio_Mngr_App_FMAFStationInfo  				ast_ENG_FM_AFList[RADIO_MNGR_APP_MAX_AFLST_SIZE];/*Array of AF station info*/
}Ts_Radio_Mngr_App_APP_TO_RM_FM_AFList;
/****************************************************Station List Structures For Diag Request Ended***********************************************/

/*************************************************************************************************************************************************/

/******************************************Tuner Status Notification Started***********************************************************************/
/**
 * @brief This structure describes DAB Tuner station nofication
 */
typedef struct 
{
	Ts32 											s32_RFFieldStrength;
	Ts32 											s32_BBFieldStrength;
	Tu32 											u32_FicBitErrorRatio;
	Tbool 											b_isValidMscBitErrorRatio;
	Tu32 											u32_MscBitErrorRatio;
	Tu8												u8_BER_Sig;									/*Bit error rate Signal*/
	Ts8												s8_BER_Exp;									/*BER Exponent*/
	Ts8												s8_SNR;										/*Signal to noise ratio*/
	Ts8												s8_RSSI;									/*Signal strength*/
	Tu8												u8_AudioDecodingStatus;						/*DAB Audio Decoding status*/
	Tu8												u8_AudioQuality;							/*DAB Audio Quality*/
	Tu8												u8_AudioLevel;								/*DAB Audio Level*/

}Ts_Radio_Mngr_App_DAB_TunerStatusNotify;

/******************************************Tuner Status Notification Ended*********************************************/
/**
 * @brief This structure describes PI List
 */
typedef struct
{
    Tu16											au16_PI[RADIO_MNGR_APP_MAX_PI_LIST];		/*Programe Identifier List*/												
	Tu8												u8_numPI;									/*Number of PIs in List*/
	Tu8												padding[3];									/*Padding Data*/
}Ts_Radio_Mngr_App_PIList;

/**
 * @brief This structure containing LSM/Tunable information
 */
typedef struct
{								
	Tu32											u32_AM_Freq;								/*Last AM Tuned Freq*/
	Tu32											u32_FM_Freq;								/*Last FM Tuned Freq*/
	Tu16                  							u16_PI;										/*Last Tuned AM Frequency PI*/
	Tu16                              				u16_EId;									/*Last Tuned Ensemble id */
	Tu32          									u32_DAB_Freq;								/*Last Tuned DAB Freq*/	
	Tu32                               				u32_Sid;									/*Last Tuned Service id */
	Tu16                               				u16_SCIdI;									/*Last Tuned Service component ID */
	Tu8												u8_StartType_BlockOne;						/*NVM read/write starttype address location first byte*/
	Tu8												u8_StartType_BlockTwo;						/*NVM read/write starttype address location second byte*/
	Tu8												u8_LSM_Band;								/*Last Active Band*/
	Tu8												u8_LSMMarket;								/*Validating the Market*/

}Ts_Radio_Mngr_Tunuable_Station_Info;

/**
 * @brief This structure describes Common Stations with PI's/Sid's
 */
typedef struct
{
    Tu8												au8_Common_Station_Index[12];				/*Common Stations Indexes array of PIs/SIds*/
	Tu8												u8_numIndex;								/*Number of Common Stations Indexes*/
	Tu8												u8_padding[3];								/*padding bytes*/
}Ts_Radio_Mngr_App_Common_Station_Indexes;

/******************************************MIXED PRESET Structures Started*********************************************/
/**
 * @brief a generic preset list, containing all band type information and the "union memory"
 */
typedef struct
{
	Te_Radio_Mngr_App_Band 							e_Band;										/*Preset Station Band*/
	Tu8												u8_CharSet;									/*Flag for the Preset station name conversion*/
	Tu8												u8_padding[3];								/*padding bytes*/
	union
	{	
		Ts_Radio_Mngr_App_AM_StationInfo     		st_AMStnInfo;								/*AM station info*/
		Ts_Radio_Mngr_App_FMStationInfo        		st_FMStnInfo;								/*FM Station Info*/  
		Ts_Radio_Mngr_App_DAB_StationInfo      		st_DABStnInfo;								/*DAB Station Info*/
	}u_PresetStInfo;
}Ts_Radio_Mngr_App_Preset_Info;

/**
 * @brief structure to contain a preset list, potentially of mixed bands
 */
typedef struct
{
    Tu8                		           				u8_NumPresetList;     						/* Number of available presets in list */
	Tu8												u8_padding[3];								/*padding bytes*/
    Ts_Radio_Mngr_App_Preset_Info      				ast_presetlist[RADIO_MNGR_APP_MAX_PSML_SIZE];/* stations present in the station list RADIO_APP_MAX_PSL_SIZE = 15. */
}Ts_Radio_Mngr_App_Preset_Mixed_List;

/******************************************MIXED PRESET Structures Ended*********************************************/
/**
 * @brief structure to contains DLS data
 */
typedef struct
{        
    Tu8												u8_CharSet;									/*Flag for the DLS Coversion*/
    Tu8												au8_DLSData[RADIO_MNGR_APP_MAX_DLS_DATA];	/*DLS Data*/

}Ts_Radio_Mngr_App_DLS_Data;


/**
 * @brief structure to contains Clock Time Information
 */
typedef struct
{        
    Tu8												u8_Hour;									/*Hours Info*/
    Tu8												u8_Min;										/*Minutes Info*/
    Tu8												u8_Day;										/*Day Info*/
    Tu8												u8_Month;									/*Month info*/
    Tu16											u16_Year;									/*Year Info*/
    Tu8												au8_PaddingBytes[2];

}Ts_Radio_Mngr_App_CT_Info;

/**
 * @brief structure to DAB version Information
 */
typedef struct
{
	Tu8												u8_NoOfCharInSWVersion;						/**<Number of characters in DAB software version information*/ 
	Tu8	                                			u8_NoOfCharInHWVersion;						/**<Number of characters in DAB Hardware version information*/
	Tu8												au8_DABTuner_SWVersion[RADIO_MNGR_APP_MAX_DABTUNER_SW_VERSION];/**<DAB Tuner software version information*/
	Tu8												au8_DABTuner_HWVersion[RADIO_MNGR_APP_MAX_DABTUNER_HW_VERSION];/**<DAB Tuner Hardware version information*/
}Ts_Radio_Mngr_App_DABVersion_Reply;

/**
 * @brief structure to contains Non RDS FM Detail Information
 */
typedef struct
{
	 Tu32   										au32_frequency[RADIO_APP_FM_NONRDS_STL_SIZE];/**<Non RDS frequency information*/
	 Tu8	                          				u8_numberStationsInList;                   	/**<Number of stations present in FM station list */
	 Tu8											u8_padding[3];								/**<padding bytes*/
}Ts_Radio_Mngr_App_FMNonRDS_StnSearchInfo;

/**
 * @brief structure to contains Stationlist search Information
 */ 
typedef struct
{
	Te_Radio_Mngr_App_STL_Search_Type 				e_STL_Search_Type;							/**<enum type for stationlist search type information*/
	union
	{ 
		Ts_Radio_Mngr_App_FMNonRDS_StnSearchInfo    st_FMNonRDS_StnListSearch;					/**<FM station list info of Non-RDS search*/ 
		Ts_Radio_Mngr_App_FM_SL						st_FMRDS_StnListSearch;						/**<FM station list info of given alphabet search*/ 
		Ts_Radio_Mngr_App_DAB_SL					st_DAB_StnListSearch;						/**<DAB station list info of given alphabet search*/ 
		
	}u_StationList_Search;
}Ts_Radio_Mngr_App_STL_Search;
/**
* @brief structure to contains Dataservice rawdata
*/
typedef struct
{
	Te_RADIO_EtalDataServiceType e_Header;
	Tu8					u8_Payload[RADIO_MNGR_APP_MAX_DATA_SERVICE_PAYLOAD_SIZE];
	Tu32				u32_PayloadSize;
}Ts_Radio_Mngr_App_DataServiceRaw;

/**
 * @brief structure to contains Multiplex ensemble Information
 */ 
typedef struct
{

	Tu8												au8_EnsembleLabel[RADIO_MNGR_APP_NUMCHAR_LABEL];/**< Ensemble label */
	Tu16    										u16_EId ;									/**< Ensemble Id */
	Ts8     										s8_RSSI ;									/**< Signal strength */	
	Tu8     										u8_CharSet;									/**<Flag for the ensemble name conversion*/
	
}Ts_Radio_Mngr_App_DAB_MultiplexEnsembleInfo;			

/**
 * @brief structure to contains Multiplex Stationlist Information
 */ 
typedef struct
{
	Ts_Radio_Mngr_App_DAB_MultiplexEnsembleInfo		ast_EnsembleInfo[RADIO_MNGR_MULTIPLEX_ENSEMBLES];/**< Ensemble list info */  
	Tu8												u8_NoOfEnsembleList;						/**< Number of Ensemble list info */
	Tu8     										au8_padding[3];								/**< Padding Bytes */

}Ts_Radio_Mngr_App_DAB_MultiplexStationList ;  

/**
 * @brief Structure contains information about service info , service component info for multiplex requirement
 */
typedef struct
{
	Ts_Radio_Mngr_App_DAB_StnInfo					ast_MultiplexStationInfo[RADIO_MNGR_MAX_MULTIPLEX_STATIONS] ;/**<Service info of each ensemble */
	Tu16 											u16_EId;									/**<Ensemble Id*/
	Tu8												u8_NoOfserviceInEnsemble;					/**<Number of services in each ensemble */
	Tu8     										u8_padding;								/**<Padding Bytes*/
	
}Ts_Radio_Mngr_App_MultiplexEnsembleInfo;			

/**
 * @brief Structure contains information about Ensemble info for multiplex requirement
 */
typedef struct
{
	Ts_Radio_Mngr_App_MultiplexEnsembleInfo			ast_EnsembleInfo[RADIO_MNGR_MULTIPLEX_ENSEMBLES];/**<Ensemble list info */  
	Tu8												u8_NoOfEnsembles;							/**< Number of ensemble */
	Tu8     										au8_padding[3];								/**< Padding Bytes  */

}Ts_Radio_Mngr_App_MultiplexStationListInfo ;  
#endif /* End of RADIO_MNGR_APP_TYPES_H*/