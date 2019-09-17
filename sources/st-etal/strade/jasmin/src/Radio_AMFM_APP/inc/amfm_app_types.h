/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_types.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file contains type definitions,enumerations and structure template	*
*						  definition which are used in Request,Response and Notification APIs of AMFM		*	
*						  application component			 													*
*************************************************************************************************************/
#ifndef AMFM_APP_TYPES_H
#define AMFM_APP_TYPES_H

/*-----------------------------------------------------------------------------
					File Inclusions
-----------------------------------------------------------------------------*/
#include "cfg_types.h"


/*-----------------------------------------------------------------------------
					Macro Definitions
-----------------------------------------------------------------------------*/
/**
 * \breif   Macro to indicate value 0 
 */
#define AMFM_APP_CONSTANT_ZERO                          (0u) 

/**
 * \breif   Macro to indicate value 0 
 */
#define AMFM_APP_CHECK_HARDLINK               	        (0u)

/**
 * \breif   Macro to indicate value 0 
 */
#define AMFM_APP_CHECK_IMPLICIT_LINKING                 (1u)

/**
 * \breif   Macro to indicate value 0 
 */
#define AMFM_APP_CHECK_BOTH_HARDLINK_AND_IMPLICIT_LINK  (2u)
    
/**
 * \breif   Macro to indicate invalid index   
 */
#define AMFM_APP_INVALID_INDEX							(-1)	
/**
 * \breif   Maximum length for  program station name of a station 
 */
#define MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME		(8u)	

/**
 * \breif   Maximum number of possible FM stations in FM Band .For Brazil market(76 - 108MHz ),at max 320 stations are possible
 */
#define AMFM_APP_NO_STATIONS_FM_BAND					(325u)
/**
 * \breif   Maximum Number of stations can be stored in the Hardlink Freq List
 */
#define AMFM_APP_MAX_NO_OF_FREQ_IN_LIST					(25u)

/**
 * \breif Maximum number of bytes for a single UTF character 
 */
#define MAX_NO_BYTES_PER_UTF_CHAR						(4u)	
/**
 * \breif Maximum number of AM stations in the list 
 */
#define AMFM_APP_MAX_AM_STL_SIZE     					(60u)   
/**
 * \breif Maximum number of FM stations in the list 
 */
#define AMFM_APP_MAX_FM_STL_SIZE     					(60u)	

/**
 * \breif Maximum number of Non-RDS FM station should present in the station list for RDS supported Market 
 */
#define AMFM_APP_MAX_NON_RDS_STATIONS_IN_FM_STL	(5u)

/**
 * \breif Maximum number of PI Codes send from DAB while DAB-FM linking 
 */
#define AMFM_APP_MAX_NO_OF_PI							(12u)
/**
 * \breif Maximum number of Programs being transmitted 
 */
#define AMFM_APP_MAX_NO_OF_PGRMS       				   (32u)

/**
 * \breif Maximum size of program name
 */
#define AMFM_APP_MAX_SIZE_PRGRM_NAME   					(9u)

/**
 * \breif Maximum Length of Radio text   
 */
#define AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT   			(64u)
/**
 * \breif Macro to indicate no best PI 
 */
#define AMFM_APP_NO_BEST_PI_FOUND						(0xffu)
/**
 *\Enable the eight char display program name
 */
#define EIGHTCHARDISPLAY                      1

#define AMFM_APP_NORMAL_RSSI		    (Tu16)(16+(18*2))
#define AMFM_APP_NORMAL_WAM 			(Tu16)((255*23)/100)
#define AMFM_APP_NORMAL_USN				(Tu16)((255*18)/100)


#define AMFM_APP_REGIONAL_RSSI			(Tu16)(16+(18*2))
#define AMFM_APP_REGIONAL_WAM		    (Tu16)((255*33)/100)
#define AMFM_APP_REGIONAL_USN		    (Tu16)((255*33)/100)


#define AMFM_APP_NORMAL_THRESHOLD		    (65)
#define AMFM_APP_REGIONAL_THRESHOLD			(20)
#define AMFM_APP_SIGLOW_THRESHOLD			(25)
#define AMFM_APP_SIG_RESUME_BACK_THRESHOLD  (40u)

#define AMFM_APP_DELTA_DIFFERENCE			(Tu8)5

#define AMFM_APP_LOW_QUALITY_DELTA_DIFFERENCE			(Tu8)5
#define AMFM_APP_MID_QUALITY_DELTA_DIFFERENCE			(Tu8)10
#define AMFM_APP_HIGH_QUALITY_DELTA_DIFFERENCE			(Tu8)20

#define AMFM_APP_LOW_QUALITY_DELTA_THRESHOLD			(Tu8)10
#define AMFM_APP_MID_QUALITY_DELTA_THRESHOLD			(Tu8)40
#define AMFM_APP_HIGH_QUALITY_DELTA_THRESHOLD			(Tu8)60


#define AMFM_APP_FS_CONVERSION(FS)          (Ts8)((FS/2)-8)      
#define AMFM_APP_WAM_CONVERSION(WAM)        (Ts8)((359 * (Ts32)(WAM)) / 1024)  
#define AMFM_APP_USN_CONVERSION(USN)        (Ts8)((77 * (Ts16)(USN)) / 1024) 
#define AMFM_APP_OFS_CONVERSION(OFS)        (Tu8)((100 * (OFS)) / 127u)


#define  AMFM_APP_CHARSET_EBU					  (Tu8)1
#define  AMFM_APP_CHARSET_UCS2                    (Tu8)2
#define  AMFM_APP_CHARSET_UTF8                    (Tu8)3
#define  AMFM_APP_CHARSET_INVALID				  (Tu8)4


#define AMFM_APP_AF_NEXT_FREQ_UPDATE_DELAY		50

#define AMFM_APP_STRATEGY_AF_NEXT_FREQ_UPDATE_DELAY		10

#define AMFM_APP_MAX_NO_AF_STATIONS				(Tu8)25

#define	AMFM_APP_CURRENT_STATION_QUALITY_CHECK_DELAY 2000

#define AMFM_APP_ALPHA_DIV_FACTOR 10
#define AMFM_APP_CURR_STATION_CURR_QUAL_INCREASED_ALPHA 8
#define AMFM_APP_CURR_STATION_CURR_QUAL_DECREASED_ALPHA 9

#define AMFM_APP_AF_STATION_CURR_QUAL_INCREASED_ALPHA 5
#define AMFM_APP_AF_STATION_CURR_QUAL_DECREASED_ALPHA 5


#define AMFM_APP_AF_STATION_MODIFIED_INCREASED_ALPHA 6
#define AMFM_APP_AF_STATION_MODIFIED_DECREASED_ALPHA 7

#define AMFM_APP_QUALITY_DROP_MARGIN 				 20		

#define AMFM_APP_RDS_SENSITIVITY_THRESHOLD 			60


#define AMFM_APP_WARM_START_UP (Tu8)(0xAA)		/*0xAA = 170 */

#define AMFM_APP_FM_TIMER_ID_5 					5

#define AMFM_APP_CURR_QUAL_CHECK_COUNT 			4
#define AMFM_APP_NOTIFY_COUNT 					5

#define AMFM_APP_MAX_NO_EON_AF_STATIONS 		2

#define AMFM_APP_BG_STL_UPDATE_INITIAL_DELAY 	10
#define AMFM_APP_BG_STL_UPDATE_DELAY 			(1000 * 60 * 2)				/* 2 mins Delay*/
#define AMFM_APP_TEN_SECONDS_DELAY 				(1000 * 10)					/* 10 seconds */
#define AMFM_APP_ONE_SECOND_DELAY 				(1000 )						/* 1 second */

#define AMFM_APP_AF_STATUS_CHECK_TIMEOUT  5000 /* time in ms (5 sec)*/

#define AMFM_APP_AF_STATUS_RESET_TIMEOUT  12 /* time in number of cycles of  AMFM_APP_AF_STATUS_CHECK_TIMEOUT. here 12*5= 60 sec =  1min*/

#define AMFM_APP_AF_STRATEGY_UPDATE_LOOP_COUNT 1

#define AMFM_APP_AF_UPDATE_LOOP_COUNT 3
#define AMFM_APP_BG_AF_TUNE_THRESHOLD_VALUE		(20u)


/*-----------------------------------------------------------------------------
    Type Definitions
-----------------------------------------------------------------------------*/

typedef enum
{
	AMFM_APP_STARTUP_SCAN,
	AMFM_APP_NON_STARTUP_SCAN	
}Te_AMFM_App_Scan_Type;











typedef enum
{
	AMFM_APP_SET_AF_REGIONAL_SWITCH_ON_SUCCESS,
	AMFM_APP_SET_AF_REGIONAL_SWITCH_OFF_SUCCESS,
	AMFM_APP_SET_AF_REGIONAL_SWITCH_FAIL,
	AMFM_APP_SET_AF_REGIONAL_INVLAID
}Te_AMFM_APP_AFRegionalSwitchReplyStatus ;


/**
 * \brief enum describes Factory Reset status to RM. 
 */		
typedef enum		
{		
    AMFM_APP_FACTORY_RESET_SUCCESS = 1 ,	/**<1- Enum, for Factory Reset success Response*/ 	
    AMFM_APP_FACTORY_RESET_FAILURE,	/**<2- Enum, for Factory Reset failure Response*/ 	
   AMFM_APP_FACTORY_RESET_INVALID	/**<3- Enum, for Factory Reset Invalid Response*/ 	
}Te_AMFM_App_FactoryResetReplyStatus;		

typedef enum
{
	AMFM_APP_DAB_SAME_PI_STATION,
	AMFM_APP_DAB_REGIONAL_PI_STATION,
	AMFM_APP_DAB_PI_STATION_UNIDENTIFIED
}Te_AMFM_DAB_PI_TYPE;


typedef enum
{
    AMFM_APP_ANNOUNCEMENT_START,
    AMFM_APP_ANNOUNCEMENT_COMPLETED,
    AMFM_APP_ANNOUNCEMENT_END_SIGNAL_LOSS,
    AMFM_APP_ANNOUNCEMENT_END_USER_CANCEL,
    AMFM_APP_ANNOUNCEMENT_END_TA_SWITCH_OFF,
    AMFM_APP_ANNOUNCEMENT_NOT_AVAILABLE
}Te_AMFM_APP_Announcement_Status;
 

/**
 * \brief This enum is used to indicate the different types of mode. 
 */
typedef enum
{
	AMFM_APP_MODE_AM,  				/*AM wave mode .It can be either LW,MW,SW*/ 
    AMFM_APP_MODE_FM,       		/* FM wave mode */
    AMFM_APP_MODE_AM_MW,    		/* AM Medium wave mode */
    AMFM_APP_MODE_AM_LW,    		/* AM Long wave mode */
    AMFM_APP_MODE_AM_SW,    		/* AM Short wave mode */
    AMFM_APP_MODE_WB,       		/* US Weather mode */	
	AMFM_APP_MODE_INVALID   	/* invalid mode, can be used  if tuner is off */	
} Te_AMFM_App_mode;

typedef enum
{
	AMFM_APP_AF_SWITCH_ON,
	AMFM_APP_AF_SWITCH_OFF,
	AMFM_APP_AF_SWITCH_INVALID
}Te_AMFM_App_AF_Switch;


	
typedef enum
{
	AMFM_APP_AF_REGIONAL_SWITCH_ON,
	AMFM_APP_AF_REGIONAL_SWITCH_OFF,
	AMFM_APP_AF_REGIONAL_SWITCH_INVALID
}Te_AMFM_AF_REGIONAL_Switch;


	
typedef enum
{
	AMFM_APP_TA_SWITCH_ON,
	AMFM_APP_TA_SWITCH_OFF,
	AMFM_APP_TA_SWITCH_INVALID
}Te_AMFM_App_TA_Switch;

typedef enum
{
	AMFM_APP_FM_TO_DAB_SWITCH_ON,
	AMFM_APP_FM_TO_DAB_SWITCH_OFF,
	AMFM_APP_FM_TO_DAB_SWITCH_INVALID
}Te_AMFM_App_FM_To_DAB_Switch;
			
	


/**
 *   \brief This enum represents different markets type
 */
typedef enum
{      																	
    AMFM_APP_MARKET_WESTERN_EUROPE,  																
    AMFM_APP_MARKET_LATIN_AMERICA,          														
    AMFM_APP_MARKET_ASIA_CHINA,         																
    AMFM_APP_MARKET_ARABIA,        																	
    AMFM_APP_MARKET_USA_NORTHAMERICA,            													
	AMFM_APP_MARKET_JAPAN,             															
	AMFM_APP_MARKET_KOREA, 
	AMFM_APP_MARKET_BRAZIL,
	AMFM_APP_MARKET_SOUTH_AMERICA,
	AMFM_APP_MARKET_INVALID           														
}Te_AMFM_App_Market;

typedef enum
{
	AMFM_APP_FOREGROUND,
	AMFM_APP_BACKGROUND
}Te_AMFM_App_Processing_Status;


typedef enum
{
	AMFM_APP_HARDLINK = 0,
	AMFM_APP_IMPLICIT_LINK,
	AMFM_APP_NO_LINK_AVAILABLE	
}Te_AMFM_APP_BestPI_Type;

typedef enum
{

	AMFM_APP_AF_LIST_AVAILABLE,
	AMFM_APP_AF_LIST_BECOMES_ZERO,
	AMFM_APP_AF_LIST_EMPTY,
	AMFM_APP_AF_LINK_INITIATED,
	AMFM_APP_AF_LINK_ESTABLISHED,
	AMFM_APP_DAB_LINK_ESTABLISHED,
	AMFM_APP_NO_LINK,
	AMFM_APP_PI_CHANGE_DETECTED,
	AMFM_APP_PI_CHANGE_LINK_ESTABLISHED
}Te_AMFM_App_AF_Status;

/**********Announcement cancel********/

typedef enum
{
	AMFM_APP_ANNO_CANCEL_BY_NEW_REQUEST,
	AMFM_APP_ANNO_CANCEL_BY_USER,
	AMFM_APP_NO_ANNO_CANCEL, 
}Te_AMFM_App_Anno_Cancel_Request;

typedef enum
{
	AF_FREQUENCY_EXIST,
	AF_FREQUENCY_TO_BE_ADDED,
	AF_FREQUENCY_INVALID
	
}Te_AF_Freq_Availabilty_Check;

typedef enum
{	
	AMFM_APP_SAME_PSN,				/* 0 ===> PSN is not changed .same as existing */
	AMFM_APP_NEW_PSN,				/* 1 ===> PSN is changed.New PSN 	*/
}Te_AMFM_App_PSNChangeFlag;

typedef enum
{
	AMFM_APP_INVALID = -1,
	AMFM_APP_ORIGINAL_STATION,		/* Select Station Response is for Original station */
	AMFM_APP_EON_STATION,			/* Select Station Response is for EON station */
}Te_AMFM_App_Select_Station_Response_Flag;

/**
* @brief enum describes Eng mode request either ON/OFF values
*/
typedef enum
{
	AMFM_APP_ENG_MODE_SWITCH_OFF,               /* 0 ===> ENG mode switch is OFF*/    
	AMFM_APP_ENG_MODE_SWITCH_ON               	/* 1 ===> ENG mode switch is ON*/                                                            
}Te_AMFM_App_Eng_Mode_Switch;

typedef enum
{
	AMFM_APP_PI_STATUS_ZERO,
	AMFM_APP_PI_STATUS_SAME,
	AMFM_APP_PI_STATUS_NEG,
	AMFM_APP_PI_STATUS_REG
}Te_AMFM_App_AF_PI_STATUS;


typedef enum
{
	REGIONAL_PI_COMPITABLE,
	REGIONAL_PI_NON_COMPITABLE
}Te_AMFM_App_Regional_PI_Check;				


/**
* @brief enum describes Current station signal Quality
*/
typedef enum
{
	AMFM_APP_LOW_QUALITY_SIGNAL=1,					/* This enum describes that the current station have low signal strength */
	AMFM_APP_GOOD_QUALITY_SIGNAL					/* This enum describes that the current station have good signal strength*/
}Te_AMFM_App_SigQuality;


/**
* @brief enum describes Current station signal Quality
*/
typedef enum
{
	AMFM_APP_IDLE,								/*Being idle*/
	AMFM_APP_LISTEN_FM_STATION,					/* Tuned to FM station and updating quality alone*/
	AMFM_APP_AF_SWITCHING,						/* Tuned to FM station and updating quality  and proceeding with  AF strategy*/
	AMFM_APP_PI_CHANGE,							/* Tuned to FM station ,its PI changed proceeding with  AF strategy*/
	AMFM_APP_AF_TUNE_REQ_RECEIVED,				/* Processing AF Tune request in Foreground*/
	AMFM_APP_AF_TUNE_SCAN_STARTED,				/* In foreground AF tune req failed starting AFTune strategy scan*/
	AMFM_APP_AF_TUNE_SCAN_COMPLETED,			/* In foreground AF tune req failed in AFTune strategy scan completed trying for AF Tune processing*/
	AMFM_APP_AF_TUNE_STRATEGY_SUCCESS,			/* In foreground AF tune req completed Sucessfully*/
	AMFM_APP_AF_TUNE_STRATEGY_FAILED,			/* In foreground AF tune req failed */
	AMFM_APP_WAIT_TUNE_REQ,						/* In foreground waiting for oringinal Freq tune req */
	AMFM_APP_ANNOUNCEMENT_HANDLING				/* Announcement Handling in Progress*/
	
}Te_AMFM_App_CurrStatus;

/**		
* 	@brief enum describes StationNotAvai Strategy Status.Strategy is followed during 
*		   Preset/Station list Selection/Warm-start-Up/Band change 	
*/		
typedef enum		
{		
    AMFM_APP_STATIONNOTAVAIL_STRATEGY_END,		/*<0- Enum, for Strategy Process Started */ 	
    AMFM_APP_STATIONNOTAVAIL_STRATEGY_START,	/*<1- Enum, for Strategy Process Ended   */ 	
    AMFM_APP_STATIONNOTAVAIL_STRATEGY_INVALID	/*<2- Enum, for Strategy Status Invalid  */ 	
}Te_AMFM_App_StationNotAvailStrategyStatus;		
		
/**		
* @brief enum describes AF Operaration Status in Updated Learn Memory		
*/		
typedef enum		
{		
    AMFM_APP_LEARN_MEM_AF_SUCCESS,	/**<0- Enum, for AF operation SUCCESS in Updated Learn Memory*/ 	
    AMFM_APP_LEARN_MEM_AF_FAIL,		/**<1- Enum, for AF operation FAIL in Updated Learn Memory*/ 	
    AMFM_APP_LEARN_MEM_AF_INVALID,	/**<2- Enum, for AF Operaration Status in Updated Learn Memory Invalid*/	
}Te_AMFM_App_LearnMemAFStatus;	


/**		
* @brief enum describes FM-DAB linking status when FM is in foreground		
*/		
typedef enum
{
	AMFM_APP_FM_STATION_TUNED,								/*Being idle*/
	AMFM_APP_PI_NOTIFICATION_SENT,				/* Tuned to FM station and notification not yet sent */
	AMFM_APP_FM_DAB_LINKED,						/* Tuned to FM station and linked to same PI DAB station */
}Te_AMFM_FM_DAB_Status;


typedef enum
{
    AMFM_APP_DAB_LINKING_PI_CHANGE=1,    
    AMFM_APP_DAB_LINKING_NEW_REQUEST,
    AMFM_APP_DAB_LINKING_SIG_LOST,
	AMFM_APP_DAB_LINKING_INVALID
}Te_AMFM_FMDAB_STOP_status;

/**
*	@brief This enum used to indicate the status of BG AF tune 
*                 whenever StationNotAvaliable strategy is ongoing.
*/
typedef enum
{
	AMFM_APP_INVALID_STATUS = -1,
	AMFM_APP_SEARCHING_OLD_LM,								/**<  0  BG AF tune search is done in Old LM only.Still Scan is not done */		
	AMFM_APP_STRATEGY_SCAN_ONGOING,						/**<  1   Strategy scan is ongoing */
	AMFM_APP_SEARCHING_NEW_LM,								/**<  2  Scan is done.BG AF tune is done in new LM only  */
	AMFM_APP_BG_FM_AF_STRA_SUCCESS,							/**<  3  BG FM  AF startegy success 	*/
	AMFM_APP_BG_FM_AF_STRA_FAIL,							/**<  4  BG FM  AF startegy failure 	*/
}Te_AMFM_App_BG_AF_Tune_Status;


/**
 * \brief This enum uniquely identifies a AMFM Application current executing rameter: "DebuggingStatus" 
 */
typedef enum
{
	AMFM_APP_AF_STRATEGY_AFLIST_SORTED_QUALITIES,
	AMFM_APP_AFLIST_SORTED_QUALITIES,
	AMFM_APP_EON_STATION_INFO,
	AMFM_APP_AF_STRATEGYCHECK_REQUEST,
	AMFM_APP_AFCHECK_REQUEST,
	AMFM_APP_AF_FREQ_APPENDED_FROM_LEARN_MEMORY,
	AMFM_APP_AF_FREQ_APPENDED,
	AMFM_APP_EON_FREQ_APPENDED_FROM_LEARN_MEMORY
	
}Te_AMFM_APP_DebuggingStatus;



/*-----------------------------------------------------------------------------
				Type Defintions
-----------------------------------------------------------------------------*/

/**
 * \brief This Structure gives the frequency information of requested band which  includes parameters to know the start, end frequency, and step size   
 */
typedef struct 
{
	Tu32	u32_StartFreq;		/* Start frequency for the band. This value Changes as per Market */
	Tu32	u32_EndFreq;		/* End frequency for the band. This value Changes as per Market */
	Tu32	u32_StepSize;		/* Step size value  */
}Ts_AMFM_App_FreqInfo;


/**
 * \brief This structure holds frequency information of AM and FM band as per market 
 */
typedef struct
{
	Ts_AMFM_App_FreqInfo	st_AMbandMW_FreqInfo;			/*	Structure to hold  AM band(MW) frequency information according to market */
	Ts_AMFM_App_FreqInfo	st_AMbandLW_FreqInfo;			/*	Structure to hold  AM band(LW) frequency information according to market */
	Ts_AMFM_App_FreqInfo	st_FMband_FreqInfo;				/*	Structure to hold  FM band frequency information according to market */
}Ts_AMFM_App_MarketInfo;

/**
 *   \brief Structure  holds information of an AM station
 */
typedef struct
{
	Tu32									u32_Freq;           /* Frequency in KHz */
}Ts_AMFM_App_AMStationInfo;


/**
 * \brief  Structure used for maintaining AM Station list  
 */
typedef struct
{
	Ts_AMFM_App_AMStationInfo    			ast_Stations[AMFM_APP_MAX_AM_STL_SIZE];  	 /* Holds information about available AM Stations */
	Tu8	                          			u8_NumberOfStationsInList;                   /* Number of stations present in AM station list */
    Tu8                             		au8_padding[3];                              /* Three bytes padded */        
}Ts_AMFM_App_AM_STL;


/**
 * \brief This structure contains  information about an FM station 
 */
typedef struct
{
	Tu32   					u32_frequency;      /*	frequency in KHz */
	Tu16                  	u16_PI;  		    /*	Program Identifier of the FM station */
	Tu16					u16_padding;
	Tu8						au8_psn[(MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME) /* * MAX_NO_BYTES_PER_UTF_CHAR*/];  			/*		Program station Name */
}Ts_AMFM_App_FMStationInfo;

/**
 * \brief Structure used for maintaining FM Station list  
 */
typedef struct
{
	Ts_AMFM_App_FMStationInfo    		ast_Stations[AMFM_APP_MAX_FM_STL_SIZE];  	 /* Holds information about available FM Stations*/
	Tu8	                         		u8_NumberOfStationsInList;                   /* Number of stations present in FM station list */	
	Tu8									u8_charset;									 /*	character set encoding */
	Tu8									au8_padding[2];								 /* Two bytes padded */ 	
}Ts_AMFM_App_FM_STL;

typedef struct
{
	Tu32	u32_frequency;
	Tu16	u16_PI;
	
}Ts_AMFM_App_AF_learn_mem;

/**
 * \brief structure used for holding information about AF list of FM station 
 */
typedef struct
{
	Tu8									u8_NumAFList;		/*Total no of AF stations*/
	Tu8									u8_Padding[3];
	Tu32								au32_AFList[AMFM_APP_MAX_NO_AF_STATIONS];		/* Array of AF frequencies*/	
}Ts_AMFM_App_AflistInfo;


/**
 * \brief structure used for holding information about either AM or FM station 
 */
typedef struct
{
	Te_AMFM_App_mode		e_mode; 
	union
	{
		Ts_AMFM_App_AMStationInfo st_AMstation;
		Ts_AMFM_App_FMStationInfo st_FMstation;
	}un_station;
	Ts32					s32_BBFieldStrength;
	Ts32					s32_RFFieldStrength;									/*	Field Strength (RSSI)*/
	Tu32  			   		u32_UltrasonicNoise;									/*	USN */
	Tu32					u32_Multipath;											/*  wide-band AM detector */ 
	Tu8						u8_Status;												/*  Quality of the station */
	Tu32					u32_FrequencyOffset;									/*  Frequency offset in kHz */
	Tu8						u8_IF_BW;												/*  IF bandwidth */
	Tu32					u32_AdjacentChannel;
	Tu32					u32_SNR;
	Tu32					u32_coChannel;
	Tu32					u32_StereoMonoReception;
	Tu32					u32_ModulationDetector;								/* 	Modulation */
	Tu8						u8_charset;												/*	character set encoding */
	Tu8                     u8_PTY_Code;
	Tu8                     au8_PTY_Name[AMFM_APP_MAX_SIZE_PRGRM_NAME];				/*FM station Program Name */
	Tu8						au8_RadioText[AMFM_APP_MAX_NO_OF_CHAR_FOR_RT_TEXT ];    /*  Radio text of the tuned station*/	
	Ts_AMFM_App_AflistInfo 	st_Aflist;	
	Tu8						u8_TA;													/* TA bit flag */
	Tu8						u8_TP;													/* TP bit flag */				
	Tu8						au8_padding[2];											/*  Two bytes padded */
}Ts_AMFM_App_StationInfo;


typedef struct
{
	Tu16	au16_PIList[AMFM_APP_MAX_NO_OF_PI];
	Tu8		u8_PIcount;
	Tu8		u8_padding[3];
}Ts_AMFM_App_PIList;

typedef struct
{
	Tu16	u16_PI;
	Tu32	u32_Freq;
	Ts32	s32_BBFieldStrength;
	Tu8 	u8_padding[3];	
}Ts_AMFM_App_HardLink_Freq_Quality_List;

typedef struct 
{
	Tu8		u8_FreqIndex;
	Tu8		u8_CountforQuality ;
	Tu8		u8_TuneIndex;
	Tbool	b_IsBeginning;
}Ts_AMFM_App_Linking_Trace;

typedef struct
{
	Tu8						au8_psn[(MAX_NO_CHARACTERS_FOR_PROGRAM_STATION_NAME)];  	/*		Program station Name */
	Tu32					u32_Frequency;			/* Corresponding frequency for PIcode */		
	Te_AMFM_APP_BestPI_Type	e_BestPIType;
	Tu16					u16_PIcode;				/* PI code sent by DAB Tuner Ctrl*/
	Tu32					u32_Quality;				/* Quality of Best tuned FM staton  */
	Tbool					b_PIchangedFlag;	
}Ts_AMFM_App_Best_PI_Info;

typedef struct
{
	Ts_AMFM_App_PIList			st_PIList;
	Ts_AMFM_App_HardLink_Freq_Quality_List	ast_HL_Freq_Quality_List[AMFM_APP_MAX_NO_OF_FREQ_IN_LIST];

#ifdef	AMFM_APP_ENABLE_BGSCANFLAG
	Tbool									b_Request_Received;
#endif
	Tbool									b_DAB2FM_LinkingFlag;
	Tu8										u8_HL_Freq_Count;
	Tu8										u8_HL_Freq_in_AFlist;
	Tu8										u8_AF_Count;
	Tu8										u8_TotalFreq;
	Tbool									b_IsHLfreqAvailable;
	
	Ts_AMFM_App_Linking_Trace				st_Linking_trace;
	Ts_AMFM_App_Best_PI_Info				st_Best_PI_Info;
	Tu32							        u32_QualityMin; 
	Tu32							        u32_QualityMax;	
	Tu32						u32_Implicit_sid;
	Tu8							u8_LinkingType;		/* 0 ===>  HARDLINK PIs
													   1 ===>  IMPLICIT LINKING (SID)
													   2 ===>  BOTH HARDLINK & IMPLICIT */		
	Tbool						b_IsBeginning;		/* Flag to indicate the beginning
														TRUE  - Beginning  
														FALSE - Not Beginning */				
	Tu8							u8_ActualPICount;													
	
}Ts_AMFM_App_LinkingParam;

typedef struct 
{
	Tu16							u16_AFtune_PI;
	Te_AMFM_App_BG_AF_Tune_Status	e_BG_AF_Tune_Status;
	Ts_AMFM_App_StationInfo 		st_BG_AF_TunedStation_Info;
}Ts_AFM_App_BG_AFtune_param;

typedef struct
{
	Te_AMFM_DAB_PI_TYPE 			e_DAB_PI_TYPE;
	Te_AMFM_FM_DAB_Status			e_FM_DAB_status;
	Tu32							u32_Quality;
	Tu8								au8_Padding[3];
}Ts_AMFM_APP_DAB_FollowUp_StationInfo;

typedef struct
{
	Te_RADIO_DirectionType		e_TuneUpDown_Direction;
	Tu32						u32_No_of_Steps;
}Ts_AMFM_App_TuneUpDown_parameters;   

typedef struct 
{
	Tu16	u16_EON_PI;
	Tu32	u32_EON_AF_List[AMFM_APP_MAX_NO_AF_STATIONS];
	Tu8		u8_AF_Count;
	Tu8		u8_TA_AF_ListIndex;
 	Tu32 	u32_Qua_avg ;
	Tu32 	u32_Qua_old ;
	Tu32 	u32_Qua_curr;
	Tu32 	u32_Qua_old_avg;
	Tu8 	au8_paddingbytes[2];
}Ts_AMFM_App_EON_PI_Info;
 
typedef struct
{
	Tu8 						u8_PI_Count;
	Tu8							u8_EONlist_PIindex;
	Tu8 						au8_paddingbytes[2];
	Ts_AMFM_App_EON_PI_Info 	st_EON_PI_Info[AMFM_APP_MAX_NO_AF_STATIONS];   
}Ts_AMFM_App_EON_List;

typedef struct
{
	Tu32						u32_LSM_FM_Freq;
	Tu16						u16_LSM_PI;
	Tu8							au8_padding[2];	
	Ts_AMFM_App_AflistInfo		st_LSM_AFList;
}Ts_AMFM_App_LSM_FM_Band;

/**
 * \brief structure used for holding the timerid
 */
typedef struct
{
	Tu32 u32_AMFM_APP_TimerId;
	Tu32 u32_Status_Check_TimerId;
}Ts_AMFM_App_TimerId;
 

#ifdef CALIBRATION_TOOL

typedef struct
{
	Tu32 	u32_AF_Freq;		
	Tu16 	u16_AF_PI;
	Tu32	u32_IP_Quality;											/*	Interpolation Quality Value */
    Ts32	s32_BBFieldStrength     ;                               /*	Baseband Field Strength */
    Tu32	u32_Multipath           ;                               /*  wide-band AM detector */ 
	Tu32	u32_UltrasonicNoise     ;
	
}Ts_AMFM_App_Calibration_AFStation_Info;

typedef struct
{
	Tu32 	u32_curr_freq;
	Tu8 	u8_NumAFList;
	Tu8 	u8_AF_index;
	Tu8 	au8_padding[2];
	Ts_AMFM_App_Calibration_AFStation_Info  st_current_AFStation_Info[AMFM_APP_MAX_NO_AF_STATIONS];
	
}Ts_AMFM_App_Calibration_AFList_Info;

#endif




typedef struct
{
	Tu32 	u32_AF_Freq;		
	Tu16    u16_PI;
	Tu32	u32_avg_qual;
	Tu32 	u32_old_qual;
	Tu32	u32_curr_qual;
	Tbool	b_af_check;
	Ts32	s32_BBFieldStrength;
	Tu32	u32_UltrasonicNoise;
	Tu32	u32_Multipath;
	Tu32	u32_FrequencyOffset;
	Tu8 	u8_TimerCount;
	Tu8		au8_paddingbytes[3];
	Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS;
}Ts_AMFM_App_AFStation_Info;

typedef struct
{
	Tu32 	u32_curr_freq;
	Tu16	u16_curr_PI;
	Tu32    u32_af_update_newfreq;
	Tu32	u32_Qua_old;
	Tu32 	u32_Qua_curr;
	Tu32 	u32_Qua_avg;
	Tu32 	u32_Qua_old_avg;
	Tu8 	u8_NumAFList;
	Tu8 	u8_AF_index;
	Tu8		u8_Best_AF_station_Index;
	Tbool	b_af_check_flag;
	Tbool   b_quality_degraded;
	
	Tu8     u8_Num_SAME_PI_AFList;
	Tu8     u8_Num_REG_PI_AFList;
	Tu8     u8_AF_Update_Index;
	Te_AMFM_App_CurrStatus		e_curr_status;
	Ts_AMFM_App_AFStation_Info  *pst_current_AFStation_Info;
	Ts_AMFM_App_AFStation_Info  ast_current_SAME_PI_AFStation_Info[AMFM_APP_MAX_NO_AF_STATIONS];
	Ts_AMFM_App_AFStation_Info  ast_current_REG_PI_AFStation_Info[AMFM_APP_MAX_NO_AF_STATIONS];

	Ts_AMFM_App_AFStation_Info  ast_current_AFStation_Info[AMFM_APP_MAX_NO_AF_STATIONS];

}Ts_AMFM_App_AFList_Info;

typedef struct
{
	Tu32 	u32_AF_Freq;		
	Tu16    u16_PI;
	Tu32	u32_avg_qual;
	Tu8     u8_padding[3];	
}Ts_AMFM_App_ENG_AF_Station_Info;

typedef struct
{
	Tu8 	u8_NumAFList;
	Tu8     u8_padding[3];	
	Ts_AMFM_App_ENG_AF_Station_Info  ast_ENG_AF_List[AMFM_APP_MAX_NO_AF_STATIONS];
}Ts_AMFM_App_ENG_AFList_Info;


typedef  struct
{
	Tu8                                 u8_Hour;
    Tu8                                 u8_Min;
    Tu8                                 u8_Day;                    
	Tu8                                 u8_Month;
	Tu8									u8_OffsetSign;
    Tu8                                 u8_Localtime_offset;       
    Tu16                                u16_Year;
}Ts_AMFM_AppRDS_CT_Info;

typedef  struct
{
    Tu8 u8_Hour;
    Tu8 u8_Min;
	Tu8 u8_day;
    Tu8 u8_Month;
    Tu16 u16_Year;
    Tu8  au8_PaddingBytes[2];
}Ts_AMFM_App_CT_Info;

#endif /* End of AMFM_APP_TYPES_H*/