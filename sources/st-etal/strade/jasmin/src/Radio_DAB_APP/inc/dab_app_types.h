/*=============================================================================
    start of file
=============================================================================*/


/************************************************************************************************************/
/** \file dab_app_types.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_DAB_APP															     		*
*  Description			: The file contains all structure and enum of DAB Application.					    *	
*																											*
*																											*
*************************************************************************************************************/


#ifndef __DAB_APP_TYPES_H__
#define __DAB_APP_TYPES_H__

#include "dab_app.h"
#include "DAB_Tuner_Ctrl_Types.h"
#include "DAB_HAL_Interface.h"

/*-----------------------------------------------------------------------------
					Macro Definitions
-----------------------------------------------------------------------------*/
 
#define	DAB_APP_MAX_LABEL_LENGTH								(17u)
#define DAB_APP_MAX_CHANNEL_NAME_SIZE							(4u)		/* Maximum length for channel name eg: 5A, CN 11D, ROK 12C, DAB17 */
#define	DAB_APP_MAX_STATIONS									200u	    /* Maximum number of DAB stations in the station list */	
#define DAB_APP_MAX_MULTIPLEX_ENSEMBLES							(20u)	
#define DAB_MAX_DLS_DATA										(128u)		/* Maximum DLS Data size */
#define DAB_APP_MAX_ALT_FREQUENCY								(5u)
#define DAB_APP_MAX_ALT_EID										(5u)
#define DAB_APP_MAX_HARDLINK_SID								(12u)
/* MACRO's for Character Set */
#define	DAB_APP_CHARSET_EBU										(Tu8)1
#define	DAB_APP_CHARSET_UCS2									(Tu8)2
#define	DAB_APP_CHARSET_UTF8									(Tu8)3
#define DAB_APP_ANNO_START										((Tu8) 0x00)	
#define DAB_APP_ANNO_STOP										((Tu8) 0x01)

#define	DAB_APP_SNR_THRESHOLD									(Ts8)(4)
#define	DAB_APP_MAX_SW_VERSION_LENGTH							(Tu8)48
#define	DAB_APP_MAX_HW_VERSION_LENGTH							(Tu8)10
/* MACRO's DAB signal status */
//#define DAB_APP_SIGNAL_LOW 		(Tu8)0
//#define DAB_APP_SIGNAL_HIGH 	(Tu8)1

/* As per DAB Standard SLS image maximum size is 50 KB */
#define	DAB_APP_MAX_DATA_SERVICE_PAYLOAD_SIZE					51200

/*-----------------------------------------------------------------------------
					Type Definitions
-----------------------------------------------------------------------------*/
/**
 * @brief This enum describes for different directions. Ex: Seek up/down
 */
/**
 * @brief This enum describes for different Cancel Types. Ex: Seek Cancel, Scan Cancel
 */
typedef enum{
    DAB_APP_SEEK_CANCEL,            
	DAB_APP_SCAN_CANCEL,
	DAB_APP_AF_TUNE_CANCEL,
	DAB_APP_TUNE_CANCEL,
    DAB_APP_CANCEL_INVALID        /**< not a valid cancel type    */
}Te_DAB_App_CancelType;


/**
 * @brief This enum describes for different directions. Ex: Seek up/down
 */
typedef enum{
    DAB_APP_REQUEST_TUNE_INVALID,  				
    DAB_APP_REQUEST_TUNE_UP_DOWN,           
	DAB_APP_REQUEST_MANUAL_TUNE,
	DAB_APP_REQUEST_PLAY_SELECT
	
}Te_DAB_App_TuneRequest;

 /**
 * @brief identifies different markets, This enumeration represents different markets information.
 */
typedef enum
{        
    DAB_APP_ACTIVATE_REQUEST,                                     /**<0- Enum, for the SRC Active*/                            
    DAB_APP_DEACTIVE_REQUEST,                                   /**<1- Enum, for the SRC DeActive*/                            
    DAB_APP_INVALID_REQUEST,                                    /**<2- Enum, for the SRC Invalid*/                                        
}Te_DAB_App_ActivateDeactivateStatus;

typedef enum
{
	DAB_APP_ACTIVATE_REQ_INVALID,
	DAB_APP_ACTIVATE_REQ_VALID,
	DAB_APP_FACTORY_RESET_REQ
}Te_DAB_App_Satrtup_Request_Type;


 /**
 * @brief identifies different markets, This enumeration represents different markets information.
 */
typedef enum 
{
                                                                                                                                               
    DAB_APP_MARKET_WESTERN_EUROPE,                                                                                                                                  
    DAB_APP_MARKET_LATIN_AMERICA,                                                                                                                          
    DAB_APP_MARKET_ASIA_CHINA,                                                                                                                                         
    DAB_APP_MARKET_ARABIA,                                                                                                                                                
    DAB_APP_MARKET_USA_NORTHAMERICA,                                                                                                                    
    DAB_APP_MARKET_JAPAN,                                                                                                                                     
    DAB_APP_MARKET_KOREA ,
	DAB_APP_MARKET_INVALID,         

}Te_DAB_App_Market;

typedef enum
{
	DAB_APP_DLS_ENABLE,
	DAB_APP_DLS_DISABLE
	
}Te_DAB_App_DLS_EnableStatus;

typedef enum
{
	DAB_APP_NO_RECONFIG, 
    DAB_APP_SERVICE_LIST_RECONFIG,
	DAB_APP_SERVICE_RECONFIG	

}Te_DAB_App_ReConfigType;

typedef enum
{        
    DAB_APP_DABFMLINKING_ENABLE,								
    DAB_APP_DABFMLINKING_DISABLE,								
	DAB_APP_DABFMLINKING_INVALID,								
        
}Te_DAB_App_DABFMLinking_Switch;


typedef enum
{
    DAB_APP_ANNO_ON,                                 
    DAB_APP_ANNO_OFF,                    
    DAB_APP_ANNO_OFF_SIGNAL_LOSS,                
    DAB_APP_ANNO_OFF_USER_CANCEL,                
    DAB_APP_ANNO_OFF_ANNO_SETTINGS_OFF,            
    DAB_APP_ANNOUNCEMENT_NOT_AVAILABLE,              
    DAB_APP_ANNO_ANNO_INVALID                                  
}Te_DAB_App_AnnoIndication;


typedef enum
{
	DAB_APP_SERVICE_TYPE_INVALID,
	DAB_APP_SERVICE_TYPE_DAB,	
	DAB_APP_SERVICE_TYPE_DAB_PLUS,
	DAB_APP_SERVICE_TYPE_DMB
	
}Te_DAB_App_ServiceType ;

typedef enum
{
    DAB_APP_HARDLINK_PI,									/**<0.Notify Enum, Normal PI */
    DAB_APP_IMPLICIT_SID,								/**<1.Notify Enum, Implicit PI */
	DAB_APP_NO_IMPLICIT									/**<2.Notify Enum, No Implicit */

}Te_DAB_App_BestPI_Type;
typedef enum		
{		
    DAB_APP_STATIONNOTAVAIL_STRATEGY_END,	/**<0- Enum, for Strategy Process Started*/ 	
    DAB_APP_STATIONNOTAVAIL_STRATEGY_START,	/**<1- Enum, for Strategy Process Ended*/ 	
    DAB_APP_STATIONNOTAVAIL_STRATEGY_INVALID	/**<2- Enum, for Strategy Status Invalid*/ 	
	
}Te_DAB_App_StationNotAvailStrategyStatus;

/**		
* @brief enum describes AF Operaration Status in Updated Learn Memory		
*/		
typedef enum		
{			
	DAB_APP_LEARN_MEM_AF_SUCCESS,		/**<0- Enum, for AF operation SUCCESS in Updated Learn Memory*/ 	
    DAB_APP_LEARN_MEM_AF_FAIL,			/**<1- Enum, for AF operation FAIL in Updated Learn Memory*/ 	
    DAB_APP_LEARN_MEM_AF_INVALID		/**<2- Enum, for AF Operaration Status in Updated Learn Memory Invalid*/	
	
}Te_DAB_App_LearnMemAFStatus;	
/**		
 * @brief This enum describes the AF switch request.		
 */		
typedef enum		
{        		
    DAB_APP_AF_ENABLE,	/**<Enum, for the AF switch enable*/	
    DAB_APP_AF_DISABLE,	/**<Enum, for the AF switch disable*/	
    DAB_APP_AF_INVALID,	/**<Enum, for the PAF switch Invalid*/	
	
}Te_DAB_App_AF_Switch;		



typedef enum
{
    DAB_APP_SIGNAL_LOW = 1,  				
    DAB_APP_SIGNAL_HIGH
		          	
}Te_DAB_App_SignalStatus;

/**
 * @brief Differentiate the select service and seek request.
 */
typedef enum 
{

	DAB_APP_SEL_SER,
	DAB_APP_SER_COMP_SEEK,
	DAB_APP_AFTUNE,
	DAB_APP_AFTUNE_END,
	DAB_APP_BANDSCAN,
	DAB_APP_MANUAL_TUNEBY_CHNAME,
	DAB_APP_INVALID
}Te_DAB_App_RequestCmd;

/**
 * @brief Structure comprises information used for station tuning purpose only 
 */
typedef struct
{
    Tu32	u32_Frequency;	/*	Frequency in kHz */  
    Tu32	u32_SId;		/*	Service Id	*/
    Tu16	u16_EId;		/*	Ensemble Id	*/  
    Tu16	u16_SCIdI;		/*	Service Component Id */
    Tu8		u8_ECC;			/*	Extended County Code */    
    Tu8		au8_padding[3];		/*Padding bytes	*/

} Ts_DAB_App_Tunable_StationInfo;

/**
 * @brief Structure contains information about current tuned station like ensemble info , service info , service component info 
 */

typedef struct
{
	Te_DAB_App_ServiceType			e_ServiceType ;
    Ts_DAB_App_Tunable_StationInfo	st_Tunableinfo;
    Ts_Tuner_Ctrl_Label				st_EnsembleLabel;            /* Ensemble label */    
    Ts_Tuner_Ctrl_Label				st_ServiceLabel;                /* Service  label */
    Ts_Tuner_Ctrl_Label				st_ComponentLabel;         /* Component label */
    Tu8								au8_ChannelName[DAB_APP_MAX_CHANNEL_NAME_SIZE]; 
       
}Ts_DAB_App_CurrentStationInfo;

typedef struct
{
	Tu8											au8_EnsembleLabel[DAB_APP_MAX_LABEL_LENGTH] ; 				/* 16bytes */	
	Tu16    									u16_EId ;													/* 2bytes */
	Ts8     									RSSI ;														/* 1bytes */	
	Tu8                                         u8_CharSet;                                    /**<Flag for the ensemble name conversion*/
		
}Ts_Tuner_Ctrl_MultiplexEnsembleInfo;			/* 26 bytes */

typedef struct
{
	Ts_Tuner_Ctrl_MultiplexEnsembleInfo		st_Tuner_Ctrl_EnsembleInfo[DAB_APP_MAX_MULTIPLEX_ENSEMBLES];    /* 26*20= 520 bytes */  
	Tu8										u8_NoOfStationsInEnsembleList;									/* 1bytes */
	Tu8     								au8_padding[3];													/* 3bytes */

}Ts_DAB_App_MultiplexStationList ;  /* 524 bytes */
/**
 * @brief Structure comprises information used for generating station list 
 */

typedef struct 
{
	Tu32 	 							 u32_Frequency;     		/* 	Frequency in kHz 			*/  
	Tu32  	 							u32_SId;                	/* 	Service Id 					*/
	Tu16   								u16_EId;                	/* 	Ensemble Id 				*/  
	Tu16   								u16_SCId;               	/* 	Service Component Id 		*/
	Tu8									st_ServiceLabel[DAB_APP_MAX_LABEL_LENGTH] ;
	Tu8									st_ServiceCompLabel[DAB_APP_MAX_LABEL_LENGTH] ;
	Tu8     							u8_ECC ;
	Tu8									u8_CharSet;
	Ts8									RSSI;
	Tu8     							au8_padding[1];
}Ts_DAB_App_Station_Info;

typedef struct
{
	Ts_DAB_App_Station_Info		ast_DAB_Stations[DAB_APP_MAX_STATIONS] ;
	Tu8    						u8_NoOfStationsInList ;
	Tu8     					au8_padding[3];

}Ts_DAB_App_StationList ;
/**
 * @brief Structure contains information about tuner status. This structure is provided by TUNER CONTROL.
 */

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

}Ts_DAB_APP_Status_Notification;



/**
 * @brief Structure contains information about Seek Info.This structure is provided by TUNER CONTROL.
 */
typedef struct
{
	Tu32                 u32_Frequency;         //Current tuned frequency in kHz 
	Tu32                 u32_SId;					  //Current Sid
	Tu16                 u16_EId;
	Tu16                 u16_SCIdI;                // Current identifier for component 
    Tu8                  u8_ECC;
	Ts_Tuner_Ctrl_Label  				 service_label; 	/**< characters (the label itself) */
	Ts_Tuner_Ctrl_Label  				 servicecomponent_label; 	/**< characters (the label itself) */
	Ts_Tuner_Ctrl_Label					 ensemble_label;
	

}Ts_DAB_APP_CurStationInfo;

typedef struct
{
	
	Tu16 FMHardlinks[12];
	Tu8 NoOfPICodes;
	Tu8 padding[3];

}Ts_DAB_PICodeList;

/**
* @brief structure to contains Dataservice rawdata
*/
typedef struct
{
	Te_RADIO_EtalDataServiceType e_Header;
	Tu8					u8_Payload[DAB_APP_MAX_DATA_SERVICE_PAYLOAD_SIZE];
	Tu32				u32_PayloadSize;
}Ts_DAB_App_DataServiceRaw;

typedef struct
{	
	Tu8 u8_CharSet;
	Tu8 au8_DAB_DLSData[DAB_MAX_DLS_DATA];
	
}Ts_DAB_DLS_Data;

typedef struct
{
	Tu32 											u32_Frequency;     	 						/*Frequency in kHz*/  
	Tu32											u32_SId;                					/*Service Id*/
	Tu16											u16_EId;                					/*Ensemble Id*/  
	Tu8												u8_ECC;           		 					/*Extended County Code*/
	Tu16											u16_SCIdI;               					/*Service Component Id*/
} Ts_DAB_APP_fmdab_linkinfo;

typedef struct
{
Tu32 Alt_Freq[DAB_APP_MAX_ALT_FREQUENCY];    // Alternate frequencies
Tu16 Alt_Ens[DAB_APP_MAX_ALT_EID];                                                // Alternate Ensembles , no need to send respective frequencies to higher layer, that database for tuner control only.
Tu32 Hardlink_Sid[DAB_APP_MAX_HARDLINK_SID];  // Hardlinks , no need to send respective ensemble and frequency .
}Ts_DAB_App_AFList;


typedef enum
{
	
	DAB_APP_FMDAB_SAME_PI_STATION,
	DAB_APP_FMDAB_REGIONAL_PI_STATION,
	DAB_APP_FMDAB_PI_STATION_UNIDENTIFIED,
	DAB_APP_FMDAB_PI_RECEIVED,
	DAB_APP_TUNED_TO_SAME_STATION,
	DAB_APP_TUNED_STATIONS_SORTED,
	DAB_APP_TUNED_STATION_NOTSTABLE,
	DAB_APP_FMDAB_BLENDING_SUCCESS,
	DAB_APP_FMDAB_BLENDING_CANCELLED,
	DAB_APP_FMDAB_BLENDING_STOP,
	DAB_APP_FMDAB_PI_INVALID

}Te_DAB_APP_fmdab_linkstatus;


typedef struct
{
	Te_DAB_Tuner_Ctrl_announcement_type	  e_announcement_type;
	Tu32 								u32_SId;
	Tu16		u16_AswFlag;
	Tu8         u8_SubChId;             
	Tu8			u8_Clusterid;
}Ts_DAB_APP_Curr_Anno_Info;

typedef struct
{
	Ts_CurrEnsemble_serviceinfo        st_serviceinfo[MAX_ENSEMBLE_SERVICES];
	Tu8	u8_ReplyStatus;
	Tu8 u8_NumOfServices;

}Ts_DAB_App_CurrEnsembleProgList;

typedef enum
{
	DAB_APP_MODE_OFF,                                                                    /*1- Enum for ENG mode OFF*/  
    DAB_APP_MODE_ON                                                                    /*0- Enum for ENG mode ON*/                                                                
}Te_DAB_APP_Eng_Mode_Request;

typedef enum
{
    DAB_APP_DAB_DAB_TUNE_INVALID, 
	DAB_APP_DAB_ALT_TUNE_STARTED,                         
    DAB_APP_DAB_DAB_TUNE_SUCCESS,                     
    DAB_APP_DAB_DAB_TUNE_FAILURE 
	      
}Te_DAB_APP_DAB_DAB_Status;
typedef enum
{
	
	DAB_APP_FMDAB_LINKING_REQ_PI_CHANGE =1,	
	DAB_APP_FMDAB_LINKING_REQ_NEW_REQUEST,
	DAB_APP_FMDAB_LINKING_REQ_SIG_LOST,
	DAB_APP_FMDAB_LINKING_REQ_INVALID
	
}Te_Dab_App_FmtoDAB_Reqstatus;

 /**
 * @brief Structure to update receiver reply from DABTuner for version request command.
 */
typedef struct
{
	Tu8         NoOfCharsSW;
    Tu8         NoOfCharsHW;
    Tu8         SWVersion[DAB_APP_MAX_SW_VERSION_LENGTH];
	Tu8         HWVersion[DAB_APP_MAX_HW_VERSION_LENGTH];

}Ts_DAB_APP_GetVersion_Reply;

#endif /* __DAB_APP_TYPES_H__ */

/*=============================================================================
    end of file
=============================================================================*/