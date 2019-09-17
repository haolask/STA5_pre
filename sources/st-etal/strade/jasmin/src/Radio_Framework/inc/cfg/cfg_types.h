/*=============================================================================
    start of file
=============================================================================*/

/************************************************************************************************************/
/** \file cfg_types.h																				    	*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: Radio Framework															     	*
*  Description			: This file contains typedefinitions of datatypes.									*
*																											*
*																											*
*************************************************************************************************************/

#ifndef CFG_TYPES_H_
#define CFG_TYPES_H_

/******************************************************************************
    includes
******************************************************************************/
//#include <sw_debug_api.h>
//#include <itron.h>

/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/
#define OS_WIN32
#define CALIBRATION_TOOL


/*-----------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

/** character type */
typedef char Tchar;

typedef unsigned char Tbool;

/** wide character type (UTF16 - 16 bit) */
typedef unsigned short Twchar;

/** unsigned 1Byte integer */
typedef unsigned char Tu8;

/** signed 1Byte integer */
typedef signed char Ts8;

/** unsigned 2Byte integer */
typedef unsigned short Tu16;

/** signed 2Byte integer */
typedef signed short Ts16;

/** unsigned 4Byte integer */
typedef unsigned int Tu32;

/** signed 4Byte integer */
typedef signed int Ts32;

/** unsigned 4Byte integer */
//typedef unsigned long Tu32;

/** signed 4Byte integer */
typedef signed long Tsint32;

typedef unsigned long Tuint32;
/**
 * @brief signed integer
 *
 * @attention This typedef is only to be used for interfaces to external
 * software which is not MISRA conform, for example hardware drivers.
 * The typedef should never be used for radio core internal interfaces.
 */
typedef int Tint;

/**
 * @brief unsigned integer
 *
 * @attention This typedef is only to be used for interfaces to external
 * software which is not MISRA conform, for example hardware drivers.
 * The typedef should never be used for radio core internal interfaces.
 */
typedef unsigned int Tuint;

/**
 * @brief signed long
 *
 * @attention This typedef is only to be used for interfaces to external
 * software which is not MISRA conform, for example hardware drivers.
 * The typedef should never be used for radio core internal interfaces.
 */
typedef long Tlong;

/** 8Byte float */
typedef float Tfloat;

/** 8Byte double */
typedef double Tdouble;

typedef Tfloat Tf32;

typedef Tdouble Tf64;


/** unsigned 8Byte integer */
typedef unsigned long long Tu64;

/** signed 8Byte integer */
typedef signed long long Ts64;

#if defined OS_WIN32
typedef signed long ER_ID;

typedef signed long ER;

typedef unsigned long RELTIM;

typedef unsigned long FLGPTN;

typedef unsigned char UINT8;

typedef unsigned long DWORD;
typedef int BOOL;
typedef unsigned short UINT16;

typedef unsigned int UINT32, UINT;
typedef int INT;

typedef char CHAR;

typedef void			*VP;			/* pointer to variable data type	*/

typedef struct Ts_msg{
			VP			msghead;		/* message header					*/
} T_MSG;

#define E_OK			0L				/* normal end						*/
#endif
typedef enum
{
        INVALID=-1,					
		E_FALSE,
		E_TRUE

}Te_Bool;



#ifndef TRUE                                            /* conditional check */
#define TRUE      ((Tbool) 1)             				/**< \brief Boolean TRUE value */
#endif

#ifndef FALSE                                           /* conditional check */
#define FALSE     ((Tbool) 0)             				/**< \brief Boolean FALSE value */
#endif

#define UNUSED(x) (void)(x)


/* Enum used to indicate seek direction either up/down */
typedef enum
{
	RADIO_FRMWK_DIRECTION_INVALID = -1,							   /* -1.Enum for Direction Invalid */
	RADIO_FRMWK_DIRECTION_DOWN,                                    /*  0.Enum for Direction 'Down' */
	RADIO_FRMWK_DIRECTION_UP                                       /*  1.Enum for Direction 'Up' */
	
}Te_RADIO_DirectionType;

/* Enum used to indicate seek type either normal/PIseek */
typedef enum
{
	RADIO_SEEK_TYPE_INVALID = -1,							  
	RADIO_SEEK,                                    
	RADIO_PI_SEEK                                       
}Te_RADIO_SeekType;

/* Enum used to indicate scan type either FG/BG */
typedef enum
{
	RADIO_SCAN_TYPE_INVALID = -1,							   
	RADIO_SCAN_FG,                                  
	RADIO_SCAN_BG                                      
}Te_RADIO_ScanType;

/**
 * @brief This enum describes for Reply Status for Radio Requests
 */
typedef enum{
	REPLYSTATUS_INVALID_PARAM = -1,							/* -1.Enum for Reply status Invalid */
	REPLYSTATUS_SUCCESS,       								/*  0.Enum for Reply status Success */
	REPLYSTATUS_FAILURE,        							/*  1.Enum for Reply status Failure */
	REPLYSTATUS_ALTERNATE,									/*  2.Enum for Reply status Alternate */
	REPLYSTATUS_REQ_TIMEOUT,   								/*  3.Enum for Reply status Request timeout */
	REPLYSTATUS_REQ_CANCELLED,								/*  4.Enum for Reply status Request cancelled */					
	REPLYSTATUS_REQ_NOT_HANDLED,							/*  5.Enum for Reply status Request not handled */ 				
	REPLYSTATUS_REQ_ERROR,									/*  6.Enum for Reply status Rquest error */					
	REPLYSTATUS_MEMORY_ERROR, 								/*  7.Enum for Reply status Memory error */   				
	REPLYSTATUS_NO_SIGNAL,									/*  8.Enum for Reply status No signal */					
	REPLYSTATUS_SERVICE_NOT_AVAILABLE,						/*  9.Enum for Reply status Service not avialable*/			
	REPLYSTATUS_AUDIO_ERROR,								/*  10.Enum for Reply status Audio error */					
	REPLYSTATUS_EMPTY,										/*  11.Enum for Reply status Emplty */						
	REPLYSTATUS_DAB_COMPONENT_NOT_AVAILABLE,				/*  12.Enum for Reply status DAB Componenet not available */		
	REPLYSTATUS_SYSTEMISCURRENTLYSEARCHING,					/*  13.Enum for Reply status System is currently searching */	
	REPLYSTATUS_NVM_WRITE_ERROR,							/*  14.Enum for Reply status NVM Write error */					
	REPLYSTATUS_NVM_READ_ERROR,								/*  15.Enum for Reply status NVM Read error */				
	REPLYSTATUS_FMTODABFOLLOW,								/*  16.Enum for Reply status FM to DAB Followup */				
	REPLYSTATUS_AF_SWITCH_OFF,								/*  17.Enum for Reply status AF Switch OFF */					
	REPLYSTATUS_BLENDING_NO_SIGNAL,							/*  18.Enum for Reply status No Signal during Blending */
	REPLYSTATUS_PRESET_RECALL_STATION_VALID,				/*  19.Enum for Reply status for preset recall station valid */
	REPLYSTATUS_PRESET_RECALL_STATION_INVALID				/*  20.Enum for Reply status for preset recall station invalid */
}Te_RADIO_ReplyStatus;

/**
 * @brief This enum is used to indicate the type of shared memory used between radio manager,AMFM /DAB app and AMFM/DAB tuner 
 */ 
typedef enum
{
	SHAREDMEMORY_INVALID = -1,						/* -1.Invalid parameter */
	AM_APP_RADIO_MNGR,								/*  0.Shared memory between AM and Radio Manager */
	FM_APP_RADIO_MNGR,								/*  1.Shared memory between FM and Radio Manager */
	DAB_APP_RADIO_MNGR,								/*  2.Shared memory between DAB and Radio Manager */
	AM_TUNER_APP,									/*  3.Shared memory between TunerCtrl and AM*/
	FM_TUNER_APP,									/*  4.Shared memory between TunerCtrl and FM */
	DAB_TUNER_APP									/*  5.Shared memory between TunerCtrl and DAB */
	
}Te_RADIO_SharedMemoryType;

/**
 * @brief This enum describes the DAB-FM linking status
 */
typedef enum
{
    RADIO_FRMWK_DAB_FM_HARDLINKS_RECEIVED,								/* 0.Notify Enum, After every service tune PI's recieved. Informing to HMI */
    RADIO_FRMWK_DAB_FM_BEST_PI_RECEIVED,								/* 1.Notify Enum, After best PI found by AMFM_App, notify to HMI from Radio Manager */
	RADIO_FRMWK_DAB_FM_IMPLICIT_PI_RECEIVED,							/* 2.Notify Enum, After Implicit PI found by AMFM_App, notify to HMI from Radio Manager */
    RADIO_FRMWK_DAB_FM_LINKING_NOT_AVAILABLE,							/* 3.Notify Enum, Once Best PI found, then after some time if the Quality < QualityMin, notify to HMI and DAB */
	RADIO_FRMWK_DAB_FM_LINKING_CANCELLED,								/* 4.Notify Enum, FM linking cancel */
    RADIO_FRMWK_DAB_FM_BLENDING_SUCCESS,								/* 5.Notify Enum, DAB FM Blending Success */
    RADIO_FRMWK_DAB_FM_BLENDING_FAILURE,								/* 6.Notify Enum, DAB FM Blending Failure */
	RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_SUCCESS,						/* 7.Notify Enum, Implicit Blending Success */
    RADIO_FRMWK_DAB_FM_IMPLICIT_BLENDING_FAILURE,						/* 8.Notify Enum, Implicit Blending Failure */
    RADIO_FRMWK_DAB_FM_BLENDING_SUSPENDED,								/* 9.Notify Enum, FM Blending Suspended */
    RADIO_FRMWK_DAB_RESUME_BACK,										/* 10.Notify Enum, DAB Resume back */
    RADIO_FRMWK_DAB_FM_IMPLICIT_PI_REQUEST_SENT,						/* 11.Notify Enum, Implicit PI request sent to FM */
	RADIO_FRMWK_DAB_NORMAL_PLAYBACK,									/* 12.Notify Enum, DAB Normal Playback */
	RADIO_FRMWK_DAB_FM_IMPLICITPI_AND_HARDLINK_REQUEST_SENT, 			/* 13.Notify Enum, Implicit PI & Hardlink request sent to FM */
	RADIO_FRMWK_DAB_FM_LINKING_DISABLED    								/* 14.Notify Enum, DAB to FM Linking Disabled */
	
}Te_RADIO_DABFM_LinkingStatus;

/**
 * @brief enum describes the AMFM TUNER's status
 */
typedef enum
{
	RADIO_FRMWK_AMFMTUNER_NORMAL,						/* 0.Enum for AMFM TUNER status is Normal */
	RADIO_FRMWK_AMFMTUNER_I2CERROR,						/* 1.Enum for AMFM TUNER status is I2C Error */
	RADIO_FRMWK_AMFMTUNER_INTERNAL_RESET,				/* 2.Enum for AMFM TUNER status is Internal Reset */
	RADIO_FRMWK_AMFMTUNER_STATUS_INVALID				/* 3.Enum for AMFM TUNER status is Invalid */

}Te_RADIO_AMFMTuner_Status;

/**
 * @brief enum describes the Component status
 */
typedef enum
{
    RADIO_FRMWK_COMP_STATUS_NORMAL,                   	/* 0.Enum for SOC status is Normal */
    RADIO_FRMWK_COMP_STATUS_ABNORMAL,               	/* 1.Enum for SOC status is Abnormal */
    RADIO_FRMWK_COMP_STATUS_INVALID                     /* 2.Enum for SOC status is Invalid */

}Te_RADIO_Comp_Status;

/*enum representing the dataservice types*/
typedef enum
{
	RADIO_FRMWK_DAB_DATASERV_TYPE_NONE,
	RADIO_FRMWK_DAB_DATASERV_TYPE_ALL,
	RADIO_FRMWK_DAB_DATASERV_TYPE_EPG_RAW,
	RADIO_FRMWK_DAB_DATASERV_TYPE_SLS,
	RADIO_FRMWK_DAB_DATASERV_TYPE_SLS_XPAD,
	RADIO_FRMWK_DAB_DATASERV_TYPE_TPEG_RAW,
	RADIO_FRMWK_DAB_DATASERV_TYPE_TPEG_SNI,
	RADIO_FRMWK_DAB_DATASERV_TYPE_SLI,
	RADIO_FRMWK_DAB_DATASERV_TYPE_EPG_BIN,
	RADIO_FRMWK_DAB_DATASERV_TYPE_EPG_SRV,
	RADIO_FRMWK_DAB_DATASERV_TYPE_EPG_PRG,
	RADIO_FRMWK_DAB_DATASERV_TYPE_EPG_LOGO,
	RADIO_FRMWK_DAB_DATASERV_TYPE_JML,
	RADIO_FRMWK_DAB_DATASERV_TYPE_FIDC,
	RADIO_FRMWK_DAB_DATASERV_TYPE_TMC,
	RADIO_FRMWK_DAB_DATASERV_TYPE_DLPLUS,
	RADIO_FRMWK_DAB_DATASERV_TYPE_PSD
}Te_RADIO_EtalDataServiceType;


/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/


#endif /* CFG_TYPES_H_ */

/*=============================================================================
    end of file
=============================================================================*/
