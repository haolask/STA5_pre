//!
//!  \file      service_following_internal.h
//!  \brief     <i><b> This header file contains internal functions and variable common to service following c files  </b></i>
//!  \details   This header contains declarations related to service following feature
//!  \author    Erwan Preteseille
//!  \author    (original version) Erwan Preteseille
//!  \version   1.0
//!  \date      2013.10.07
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef SERVICE_FOLLOWING_INTERNAL_H_
#define SERVICE_FOLLOWING_INTERNAL_H_

#include "service_following_task.h"

// addition of includes which are interface definitions for SF 
#include "Service_following_externalInterface_OSAL.h"
#include "Service_following_externalInterface_DABMWServices.h"
#include "Service_following_externalInterface_RDS.h"
#include "Service_following_externalInterface_FM.h"
#include "Service_following_externalInterface_DAB.h"
#include "Service_following_externalInterface_radio.h"
#include "Service_following_externalInterface_audio.h"


// 

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************
* DEFINE SECTION
**********************
*/
 

/* Timing information
*/
 
//#define DABMW_SF_PI_SEARCH_DELTA_TIME_MS 			3000
// in theory, PI is broadcast every 100 ms.
// we need the reconfirmation : so it leads to 200 ms in best case.
// now take margin.
// 500 ms is acceptable
// more make it too long !
//
#define DABMW_SF_PI_SEARCH_DELTA_TIME_MS 			500

// PS : around 1s is acceptable.
// put a bit of margin : 2s for now.
//
#define DABMW_SF_PS_SEARCH_DELTA_TIME_MS 			4000

#define DABMW_SF_DAB_TUNE_SEARCH_DELTA_TIME_MS 		2000
#define DABMW_SF_DAB_WAIT_FIC_ACQUISITION_TIME_MS 	5000
#define DABMW_SF_FM_WAIT_AUTO_SEEK_TIME_MS 			20000
#define DABMW_SF_DELTA_TIME_FOR_SWITCH				5000
#define DABMW_SF_MINIMUM_TIME_TO_KEEP_ALTERNATE		20000

/* as a start every 10 s 
* improvement could be : change periodicity depending on attempt counter
* ie : every 5s for the 10 1st time, every 10s then....
*/

#define DABMW_SF_SERVICE_RECOVERY_TIME				1000

/* Periodic background AF search time 
*/
#define DABMW_SF_SEARCH_AF_TIME						10000

// Periodic FULL scan for AF search time
#define DABMW_SF_FULL_SEARCH_AF_TIME				60000

/*
* Periodic for landscape building
* every 5 minutes ?
*/
//#define DABMW_SF_LANDSCAPE_BUILDING_PERIODICITY_TIME	300000
//#define DABMW_SF_LANDSCAPE_BUILDING_PERIODICITY_TIME	270000
#define DABMW_SF_LANDSCAPE_BUILDING_PERIODICITY_TIME	75000

//
// change : to put same as AF search
//
#define DABMW_SF_FM_LANDSCAPE_BUILDING_PERIODICITY_TIME 20000

/* Quality measurement check 
* every 500 ms ?
*/
// change default periodicity
#define DABMW_SF_TIME_FOR_MEASUREMENT				100


/* kind of switch */
#define DABMW_SF_MASK_DAB_DAB 	            (((tU8)0x01) << 0)
#define DABMW_SF_MASK_DAB_FM		        (((tU8)0x01) << 1)
#define DABMW_SF_MASK_FM_FM		            (((tU8)0x01) << 2)
#define DABMW_SF_MASK_KIND_OF_SWITCH		(DABMW_SF_MASK_FM_FM | DABMW_SF_MASK_DAB_FM | DABMW_SF_MASK_DAB_DAB)



/* DEFAULT INIT OF SERVICE FOLLOWING */

#define DABMW_SF_DEFAULT_CONFIG_SS_ACTIVATED							true
//#define DABMW_SF_DEFAULT_CONFIG_SS_ACTIVATED							false

#define DABMW_SF_DEFAULT_CONFIG_FOLLOW_SOFT_LINKAGE			            false
#define DABMW_SF_DEFAULT_CONFIG_FOLLOW_HARD_LINKAGE			            true
// change default config : FM-FM & FM-DAB, no DAB_DAB
#define DABMW_SF_DEFAULT_KIND_OF_SWITCH						            (DABMW_SF_MASK_DAB_FM | DABMW_SF_MASK_FM_FM)
//define DABMW_SF_DEFAULT_KIND_OF_SWITCH						            (DABMW_SF_MASK_FM_FM)


#define DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_THR		            20000
#define DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_THR		            50000

#define DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_DABPLUS_DMB_THR		60000
#define DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_DABPLUS_DMB_THR		100000

// codex #319038
// improve DAB threshold and quality management
// 
// After FT experience : change default setting for DAB
// set GOOD = POOR to avoid MEDIUM
// 
// BEFORE 
// #define DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_AUDIO_BER_LEVEL        5
// AFTER
#define DABMW_SF_DEFAULT_CONFIG_DAB_GOOD_QUALITY_AUDIO_BER_LEVEL        3

#define DABMW_SF_DEFAULT_CONFIG_DAB_POOR_QUALITY_AUDIO_BER_LEVEL        3

// FM FS dBUV qualitythr
// may be to be updated depending on tuner
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
// Field Strenght in dBuV
// SYSCONF_SEEK_FSTBB_TH_FM = 22; %| SYSCONF | Seek | -10:1:70 | dBuV | 2 | The threshold used for baseband fieldsthrength using within automatic seeking for FM

#define DABMW_SF_DEFAULT_CONFIG_FM_FIELD_STRENGH_GOOD_QUALITY_THR   20
#define DABMW_SF_DEFAULT_CONFIG_FM_FIELD_STRENGH_POOR_QUALITY_THR   4

// After FT experience : change default setting for FM GOOD QUALITY
// set GOOD = combined is rather reflected by 28000
// 
// BEFORE 
// #define DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_GOOD_QUALITY_THR		22000
// AFTER
#define DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_GOOD_QUALITY_THR		28000

#define DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_POOR_QUALITY_THR  		DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_COMBINED

// hysteresis
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_FIELD_STRENGH_GOOD_QUALITY    5
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_FIELD_STRENGH_POOR_QUALITY    5

#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_COMBINEDQ_GOOD_QUALITY        2000
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_COMBINEDQ_POOR_QUALITY        2000


// Adjacent channels this is in %
// SYSCONF_SEEK_ADJ_TH_FM = 100; %| SYSCONF | Seek | -100:1:100 | % | 2 | The threshold used for adjacent channel indication using within automatic seeking for FM (negative no adj, positive adj present)
//#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_ADJACENT_CHANNEL     30  
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_ADJACENT_CHANNEL     24

// Detuning value this is in kHz => parameter is provided in step of 195 Hz 
// but for seek we do no care of value ==> 
// default CMOS 2
//  SYSCONF_SEEK_DETUNE_TH_FM = 2.5; %| SYSCONF | Seek | 0:0.1:50 | kHz | 2 | The threshold used for detune using within automatic seeking for FM
//#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DETUNING             100
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DETUNING             207

// Multipaht this is in %
// default CMOS 
// SYSCONF_SEEK_MP_TH_FM = 55; %| SYSCONF | Seek | 0:1:100 | % | 2 | The threshold used for multipath indication within automatic seeking for FM
//#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MULTIPATH            30
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MULTIPATH            207

// Combined Q ombined set to 12500 = 0x30D4
// default CMOS
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_COMBINED             0x30D4

// Deviation Deviation not used
// default CMOS
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DEVIATION            0x7FFF

//_MPXNOISE_TH_FM is in %
// default CMOS
// SYSCONF_SEEK_MPXNOISE_TH_FM = 45; %| SYSCONF | Seek | 0:1:100 | % | 2 | The threshold used for MPX noise indication within automatic seeking for FM
//#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MPXNOISE	           30 
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MPXNOISE	           45

//_SYSCONF_SEEK_SNR_TH_FM is in %
// default CMOST
// FF = best quality
// SYSCONF_SEEK_SNR_TH_FM = 42;         %| SYSCONF | Seek | 0:1:100 | % | 2 | The threshold used for SNR indication within automatic seeking for FM
//#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_SNR	           		42
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_SNR	           		48


#else
// Field Strenght in dBuV
#define DABMW_SF_DEFAULT_CONFIG_FM_FIELD_STRENGH_GOOD_QUALITY_THR   20
#define DABMW_SF_DEFAULT_CONFIG_FM_FIELD_STRENGH_POOR_QUALITY_THR   4

#define DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_GOOD_QUALITY_THR		22000
#define DABMW_SF_DEFAULT_CONFIG_FM_COMBINEDQ_POOR_QUALITY_THR  		DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_COMBINED

// hysteresis
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_FIELD_STRENGH_GOOD_QUALITY    5
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_FIELD_STRENGH_POOR_QUALITY    5

#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_COMBINEDQ_GOOD_QUALITY        2000
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_FM_COMBINEDQ_POOR_QUALITY        2000


// Adjacent channels value = 0x3F00 = 16128
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_ADJACENT_CHANNEL     0x3F00  

// Detuning value = 0x0600	 = 1536
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DETUNING             0x0600

// Multipaht do not use multipath
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_MULTIPATH            0x7FFF

// cCombined Q ombined set to 12500 = 0x30D4
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_COMBINED             0x30D4
// Deviation Deviation not used
#define DABMW_SF_DEFAULT_AUTO_SEEK_THRESHOLD_DEVIATION            0x7FFF


#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR


#define DABMW_SF_DEFAULT_CONFIG_DAB_TO_FM_DELTA_TIME		            5
#define DABMW_SF_DEFAULT_CONFIG_DAB_TO_DAB_DELTA_TIME	            	5
#define DABMW_SF_DEFAULT_CONFIG_FM_TO_DAB_DELTA_TIME		            5
#define DABMW_SF_DEFAULT_CONFIG_FM_TO_FM_DELTA_TIME			            5



// Change default setting to FM combined Quality & DAB : audio ber level.
// #define DABMW_SF_DEFAULT_CONFIG_STRATEGY_FM                 DABMW_SF_STRATEGY_FM_COMBINED_QUALITY
#define DABMW_SF_DEFAULT_CONFIG_STRATEGY_DAB                DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL
#define DABMW_SF_DEFAULT_CONFIG_STRATEGY_FM                 DABMW_SF_STRATEGY_FM_FIELD_STRENGH
//#define DABMW_SF_DEFAULT_CONFIG_STRATEGY_DAB                DABMW_SF_STRATEGY_DAB_FIC_BER


#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_FIC_BER_GOOD_QUALITY         10000
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_FIC_BER_POOR_QUALITY         10000

// After FT experience : change default setting for DAB Hysteresis
// set rather set to 1 for 30000 ms
// 
// BEFORE 
//#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_AUDIO_BER_LEVEL_GOOD_QUALITY 2
//#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_AUDIO_BER_LEVEL_POOR_QUALITY 2
// #define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DURATION                         60
// AFTER

#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_AUDIO_BER_LEVEL_GOOD_QUALITY 1
#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DAB_AUDIO_BER_LEVEL_POOR_QUALITY 1


#define DABMW_SF_DEFAULT_CONFIG_HYSTERESIS_DURATION                         30



// Threshold for SEEK
// the default in AMFM drivers are 
/*
    AMFMState[1].param.w.seek.FM.th_fst = 0xB700;		
    AMFMState[1].param.w.seek.FM.th_adj = 0x3000;
    AMFMState[1].param.w.seek.FM.th_det = 0x0600;		
    AMFMState[1].param.w.seek.FM.th_mp = 0x7FFF;
    AMFMState[1].param.w.seek.FM.th_qu = 0x5000;
*/




/* thresholds & counter for quality criteria */

// Switch criteria : let's put 1000 ms, ie with 100 ms periodicity => 10
#define DABMW_SF_NB_MEASURE_START_AF_SEARCH_DAB					10

// After FT experience : change default setting for DAB Hysteresis
// set rather set to 1 for 30000 ms
// 
// BEFORE 
// transition to poor : immediate
//#define DABMW_SF_NB_MEASURE_LOSS_SERVICE_DAB					1
// the NO_AUDIO or OUT_OF_SYNC are immediate instead
#define DABMW_SF_NB_MEASURE_LOSS_SERVICE_DAB					2

// Switch criteria : let's put 500 ms, ie with 100 ms periodicity => 5
#define DABMW_SF_NB_MEASURE_CRITERIA_SWITCH_DAB					5

#define DABMW_SF_NB_MEASURE_START_AF_SEARCH_FM					10
// BEFORE 
// transition to poor : immediate
//#define DABMW_SF_NB_MEASURE_LOSS_SERVICE_FM				1
// 
#define DABMW_SF_NB_MEASURE_LOSS_SERVICE_FM					3


// Switch criteria : let's put 500 ms, ie with 100 ms periodicity => 5
#define DABMW_SF_NB_MEASURE_CRITERIA_SWITCH_FM				5



#define DABMW_SF_CURRENT_QUALITY_PONDERATION				1
#define DABMW_SF_NEW_QUALITY_PONDERATION					1

/* hysteresys for quality */
#define DABMW_SF_HYSTERESIS_DAB								500
#define DABMW_SF_HYSTERESIS_FM								3

#define DABMW_SF_HYSTERESIS_DAB_FICBER  					500
#define DABMW_SF_HYSTERESIS_FM_dBuV							3
#define DABMW_SF_HYSTERESIS_FM_COMBINED_Q                   1000


/* EPR TMP CHANGE */
/* add variable for simulating DAB, FM level */
#define DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO	0
#define DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD	1
#define DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM	2
#define DABMW_SERVICE_FOLLOWING_SIMULATE_LOW	3
/* END EPR CHANGE */

// define the max number of AF supported for FM & DAB
//
// FM
// Max per broadcast = DABMW_AMFM_LANSCAPE_SIZE
// may have several broadcast....
// so increase it : 40 ?
#define DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF	40

// DAB
// Max per broadcast = DABMW_FIC_MAX_SERVICES_NUMBER
// may have several broadcast....
// so increase it : 40 ?

#define DABMW_SERVICE_FOLLOWING_MAX_NUM_DAB_AF	40

// Specific values to avoid PI check
#define DABMW_SF_SIMULATE_VALID_PI			0xAAAA


// **********************
// parameters
// Procedure to Set the Service Following Paramters
// parameters index are contained in p_cellParamIndex
// for each parameter , the value is contained in p_cellParamValue
//
//	Input Parameters:
//	1) vI_app (application target)
//	2) p_cellParamIndex (parameters index)
//	2) p_cellParamValue (parameters value to write)
//	3) vI_cellParamSize  (number of parameters)
//
// Output Parameters:
// 	1) function return number of parameters written (vl_len)
// 

// PARAMETERS 


// ----- Service Following Parameters ----------
#define DABMW_API_SERVICE_FOLLOWING_MAXSIZE		100
#define DABMW_API_SERVICE_FOLLOWING_PARAM_S	    0x6000	
#define DABMW_API_SERVICE_FOLLOWING_PARAM_E	    0x6FFF 


/* ----------
*  configuration  storage
* -------------
*/
#define DABMW_API_SF_PARAM_ENABLE_SF                      1
#define DABMW_API_SF_PARAM_ENABLE_SS_MODE                 2
#define DABMW_API_SF_PARAM_FOLLOW_SOFT_LINKAGE            3
#define DABMW_API_SF_PARAM_FOLLOW_HARD_LINKAGE            4
#define DABMW_API_SF_PARAM_KIND_OF_SWITCH                 5
#define DABMW_API_SF_PARAM_REGIONAL_MODE                  6
#define DABMW_API_SF_PARAM_FM_STRATEGY                    7
#define DABMW_API_SF_PARAM_DAB_STRATEGY                   8

#define DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER      20
#define DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER      21
#define DABMW_API_SF_PARAM_FM_GOOD_QUAL_THR_FS_DBUV       22
#define DABMW_API_SF_PARAM_FM_POOR_QUAL_THR_FS_DBUV       23
#define DABMW_API_SF_PARAM_FM_GOOD_QUAL_THR_COMBINEDQ     24
#define DABMW_API_SF_PARAM_FM_POOR_QUAL_THR_COMBINEDQ     25
#define DABMW_API_SF_PARAM_FM_RDS_QUAL_THR_FS_DBUV        26
#define DABMW_API_SF_PARAM_FM_RDS_QUAL_THR_FS_COMBINEDQ   27
#define DABMW_API_SF_PARAM_HYSTERESIS_DAB_GOOD_QUAL_THR_FIC_BER 28
#define DABMW_API_SF_PARAM_HYSTERESIS_DAB_POOR_QUAL_THR_FIC_BER 29
#define DABMW_API_SF_PARAM_HYSTERESIS_FM_GOOD_QUAL_THR_FS_DBUV 30
#define DABMW_API_SF_PARAM_HYSTERESIS_FM_POOR_QUAL_THR_FS_DBUV 31
#define DABMW_API_SF_PARAM_HYSTERESIS_FM_GOOD_QUAL_THR_COMBINEDQ 32
#define DABMW_API_SF_PARAM_HYSTERESIS_FM_POOR_QUAL_THR_COMBINEDQ 33
#define DABMW_API_SF_PARAM_HYSTERESIS_PENALITY_DURATION          34
#define DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER_DABPLUS_DMB     35
#define DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER_DABPLUS_DMB     36
#define DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_AUDIO_BER_LEVEL         37
#define DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_AUDIO_BER_LEVEL         38



#define DABMW_API_SF_PARAM_DAB_TO_FM_DELTA_SWITCH_TIME    40
#define DABMW_API_SF_PARAM_DAB_TO_DAB_DELTA_SWITCH_TIME   41
#define DABMW_API_SF_PARAM_FM_TO_FM_DELTA_SWITCH_TIME     42
#define DABMW_API_SF_PARAM_FM_TO_DAB_DELTA_SWITCH_TIME    43

#define DABMW_API_SF_PARAM_HYSTERESIS_DAB_GOOD_QUAL_THR_AUDIO_BER_LEVEL 44
#define DABMW_API_SF_PARAM_HYSTERESIS_DAB_POOR_QUAL_THR_AUDIO_BER_LEVEL 45


#define DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_AF_SEARCH_DAB    50
#define DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_LOSS_SERVICE_DAB 51
#define DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_CELL_CHANGE_DAB  52

#define DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_AF_SEARCH_FM    	53
#define DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_LOSS_SERVICE_FM 	54
#define DABMW_API_SF_PARAM_NB_CRITERIA_MATCH_PRIOR_CELL_CHANGE_FM  	55


#define DABMW_API_SF_AVERAGING_PONDERATION_CURRENT_QUALITY      60
#define DABMW_API_SF_AVERAGING_PONDERATION_NEW_QUALITY          61

#define DABMW_API_SF_PERIODICITY_AF_SEARCH                      70
#define DABMW_API_SF_PERIODICITY_FULL_SCAN_AF_SEARCH            71
#define DABMW_API_SF_PERIODICITY_SERVICE_RECOVERY_SEARCH        72
#define DABMW_API_SF_PERIODICITY_LANDSCAPE_BUILDING             73
#define DABMW_API_SF_PERIODICITY_LANDSCAPE_FOR_SEAMLESS         74
#define DABMW_API_SF_PERIODICITY_MEASUREMENTS_DAB_SELECTED      75
#define DABMW_API_SF_PERIODICITY_MEASUREMENTS_FM_SELECTED       76
#define DABMW_API_SF_LOG_INFO_STATUS_PERIODICITY_TIME           77
#define DABMW_API_SF_RDS_PI_MAX_WAIT_TIME                       78
#define DABMW_API_SF_PARAM_PI_VALIDITY_TIME                     79


// SS settings
#define DABMW_API_SF_SS_CONFIDENCE_LEVEL_THRESHOLD              80
#define DABMW_API_SF_SS_MEAS_VALIDITY_DURATION                  81
#define DABMW_API_SF_SS_RECONF_PERIODICITY                      82
#define DABMW_API_SF_SS_MINIMUM_TIME_BETWEEN_SS                 83
#define DABMW_API_SF_SS_FULL_ESTIMATION_START_WINDOW_IN_MS      84
#define DABMW_API_SF_SS_FULL_ESTIMATION_STOP_WINDOW_IN_MS       85
#define DABMW_API_SF_SS_FULL_ESTIMATION_DOWN_SAMPLING           86
#define DABMW_API_SF_SS_RECONF_ESTIMATION_HALF_WINDOW_IN_MS     87
#define DABMW_API_SF_SS_RECONF_ESTIMATION_DOWN_SAMPLING         88

// Others
#define DABMW_API_SF_RDS_PS_MAX_WAIT_TIME                       90


#define DABMW_API_SF_NB_PARAM_MAX                               60

// Parameter value
// DABMW_API_SF_PARAM_ENABLE_SF : true/false value
// DABMW_API_SF_PARAM_ENABLE_SS_MODE : true/false value
//  DABMW_API_SF_PARAM_FOLLOW_SOFT_LINKAGE   : true/false value
// DABMW_API_SF_PARAM_FOLLOW_HARD_LINKAGE    : true/false value
//  DABMW_API_SF_PARAM_REGIONAL_MODE      : true/false value      
#define DABMW_API_SF_FEATURE_ENABLE         0x01
#define DABMW_API_SF_FEATURE_DISABLE      0x00

// DABMW_API_SF_PARAM_KIND_OF_SWITCH
//#define DABMW_SF_MASK_DAB_DAB 	            (((tU8)0x01) << 0)
//#define DABMW_SF_MASK_DAB_FM		        (((tU8)0x01) << 1)
//#define DABMW_SF_MASK_FM_FM		            (((tU8)0x01) << 2)
//#define DABMW_SF_MASK_KIND_OF_SWITCH		(DABMW_SF_MASK_FM_FM | DABMW_SF_MASK_DAB_FM | DABMW_SF_MASK_DAB_DAB)

// DABMW_API_SF_PARAM_FM_STRATEGY
#define DABMW_API_SF_FM_STRATEGY_FIELD_STRENGH  0x00
#define DABMW_API_SF_FM_STRATEGY_COMBINED_Q     0x01


// DABMW_API_SF_PARAM_DAB_STRATEGY
#define DABMW_API_SF_DAB_STRATEGY_FIC_BER       0x00

// DABMW_API_SF_PARAM_DAB_GOOD_QUAL_THR_FIC_BER    : unit is directly the right value 
// DABMW_API_SF_PARAM_DAB_POOR_QUAL_THR_FIC_BER  : unit is directly the right value 
#define DABMW_API_SF_DAB_FIC_BER_UNIT          0x01
//
// TIMING INFORMATION : unit is ms.
//
#define DABMW_API_SF_TIME_UNIT                 0x01

// TMP DEBUG LEVEL FORCING
/* add variable for simulating DAB, FM level */
// DABMW_API_SF_FM_LEVEL_SIMULATE             
// DABMW_API_SF_DAB_LEVEL_SIMULATE      
//
// #define DABMW_SERVICE_FOLLOWING_SIMULATE_AUTO	0
// #define DABMW_SERVICE_FOLLOWING_SIMULATE_GOOD	1
// #define DABMW_SERVICE_FOLLOWING_SIMULATE_MEDIUM	2
// #define DABMW_SERVICE_FOLLOWING_SIMULATE_LOW	3


/*
*********************
* MACRO  SECTION
**********************
*/
#define DABMW_SERVICE_FOLLOWING_CHANGE_STATE(vI_newState) DABMW_ServiceFollowing_ChangeState(vI_newState) 
#define DABMW_SERVICE_FOLLOWING_CHANGE_PREV_STATE(vI_prevState) DABMW_serviceFollowingStatus.prevStatus = vI_prevState
#define DABMW_SERVICE_FOLLOWING_CHANGE_NEXT_STATE(vI_nextState) DABMW_serviceFollowingStatus.nextStatus = vI_nextState

#define DABMW_SERVICE_FOLLOWING_LOG_BOOL(x) ((true == x)?"true":"false")


/* Quality information */
/* FM GOOD Threshold in DBuV for getting RDS.... */
#define DABMW_SF_FM_GOOD_QUALITY_VALUE_THRESHOLD	20	
#define DABMW_SF_DAB_FIC_BER_GOOD_QUALITY_VALUE_THRESHOLD	500

/*
*********************
* ENUM  SECTION
**********************
*/


/* Main states 
*/
typedef enum
{
	DABMW_SF_STATE_IDLE 								 = 0,
	DABMW_SF_STATE_INIT 								 = 1,	 
	DABMW_SF_STATE_INITIAL_SERVICE_SELECTION			 = 17,
	DABMW_SF_STATE_BACKGROUND_CHECK						 = 18,
	DABMW_SF_STATE_BACKGROUND_WAIT_AUTO_SEEK			 = 19,
	DABMW_SF_STATE_BACKGROUND_WAIT_PI					 = 20,
	DABMW_SF_STATE_BACKGROUND_WAIT_DAB					 = 21,
	DABMW_SF_STATE_BACKGROUND_WAIT_FIC					 = 22,
	DABMW_SF_STATE_BACKGROUND_SCAN						 = 23,
	DABMW_SF_STATE_AF_CHECK								 = 24,
	DABMW_SF_STATE_SERVICE_RECOVERY						 = 25,
	DABMW_SF_STATE_IDLE_CHECK_SEARCH_FOR_AF				 = 26,
	DABMW_SF_STATE_IDLE_AF_MONITORING					 = 27,
	DABMW_SF_STATE_IDLE_SEAMLESS						 = 28,
	DABMW_SF_STATE_IDLE_SEAMLESS_SWITCHING               = 29,
	// NEW : PS NAME for landscape generation
	DABMW_SF_STATE_BACKGROUND_WAIT_PS					 = 30,

	DABMW_SF_STATE_ERROR								 = 100	  
} DABMW_serviceFollowingStateMachineTy;


/* Enum to identify which check of time is needed
*/
typedef enum
{
	DABMW_SF_TimeCheck_FM_PI			 			= 0,
	DABMW_SF_TimeCheck_DAB_TUNE			 			= 1,	 
	DABMW_SF_TimeCheck_DAB_FIG						= 2,
	DABMW_SF_TimeCheck_Threshold					= 3,
	DABMW_SF_TimeCheck_AutoSeek						= 4,	
	DABMW_SF_TimeCheck_LastSwitch					= 5,
	DABMW_SF_TimeCheck_ServiceRecovery   			= 6,
	DABMW_SF_TimeCheck_AFSearch						= 7,
	DABMW_SF_TimeCheck_FullScanAFSearch				= 8,
	DABMW_SF_TimeCheck_LandscapeBuildingScan 		= 9,
	DABMW_SF_TimeCheck_FMLandscapeDelayEstimation	= 10,
	DABMW_SF_TimeCheck_LogInfoStatusEvacuation      = 11,
	DABMW_SF_TimeCheck_Hysteresis_DAB               = 12,
	DABMW_SF_TimeCheck_Hysteresis_FM                = 13,
	DABMW_SF_TimeCheck_MinToKeepAlternate			= 14,
	DABMW_SF_TimeCheck_FM_PS						= 15,
} DABMW_SF_TimeCheckTy;


/* Enum Type of scan
*/
typedef enum
{
    DABMW_SF_SCAN_NONE                      = 0,
    DABMW_SF_SCAN_ALTERNATE_DAB_AF          = 1,
    DABMW_SF_SCAN_ALTERNATE_DAB_FULL        = 2,
    DABMW_SF_SCAN_ALTERNATE_FM_AF           = 3,
    DABMW_SF_SCAN_ALTERNATE_FM_FULL         = 4,
    DABMW_SF_SCAN_LANDSCAPE                 = 5,
    DABMW_SF_SCAN_SEAMLESS                  = 6,
    DABMW_SF_SCAN_RECOVERY                  = 7
} DABMW_SF_TypeOfScanTy;
        

// STRATEGY selection 
// for FM
// Currently 2 strategy only : Fieldstrengh based, and CombinedQ based.
typedef enum
{
    DABMW_SF_STRATEGY_FM_FIELD_STRENGH = 0,
    DABMW_SF_STRATEGY_FM_COMBINED_QUALITY = 1    
} DABMW_SF_Strategy_FM;

// for DAB
// Currently 1 strategy only : fic ber based
typedef enum
{
    DABMW_SF_STRATEGY_DAB_FIC_BER = 0,
    DABMW_SF_STRATEGY_DAB_AUDIO_LEVEL = 1    
} DABMW_SF_Strategy_DAB;


typedef enum
{
    DABMW_SF_CHECK_STATUS_NOT_CHECKED                         = 0,
    DABMW_SF_CHECK_STATUS_FREQ_OK                             = 1,
    DABMW_SF_CHECK_STATUS_QUALITY_NOT_OK                      = 2,
    DABMW_SF_CHECK_STATUS_NOT_MATCHING_SID_PI                 = 3,
    DABMW_SF_CHECK_STATUS_PI_NOT_RECEIVED                     = 4,
    DABMW_SF_CHECK_STATUS_NOT_BETTER_THAN_ALTERNATE           = 5,
    DABMW_SF_CHECK_STATUS_IS_ALTERNATE                        = 6,
    DABMW_SF_CHECK_STATUS_IS_ORIGINAL                         = 7    
}
DABMW_SF_FrequencyStatusTy;
    


/*
*********************
* STRUCTURE  SECTION
**********************
*/

// add a dedicated structure for quality storing


/* quality enum */
typedef enum
{
	DABMW_SF_QUALITY_IS_NO_SYNC						 = -2,
    DABMW_SF_QUALITY_IS_NO_AUDIO                     = -1,
	DABMW_SF_QUALITY_IS_POOR						 = 0,
	DABMW_SF_QUALITY_IS_MEDIUM						 = 1,
	DABMW_SF_QUALITY_IS_GOOD						 = 2
} DABMW_SF_QualityStatusTy;

typedef struct
{
    DABMW_SF_amfmQualityTy  fmQuality;
    DABMW_SF_dabQualityTy dabQuality;
    DABMW_SF_QualityStatusTy qualityStatus;
} DABMW_SF_QualityTy;





/* structure for other frequency check/scan
* used for AF check, background check, and background scan.
*/
  
typedef union
{
    struct 
    {
    // Output data (final evaluation)
    tU8 isQualityAcceptable:1;
    tU8 is_inAFList:1;        
    // parameter to identify if comes from Service Linking (true) or DB (false)
    tU8 is_inServiceLinking:1;
    // parameter to know if part of landscape (true if in)
    tU8 is_inLandscape:1;
    tU8 SearchedSidFound:1;
    tU8 is_inAFListFromOtherFrequency:1;
    tU8 is_inAFListFromAlternate:1;
    tU8 filler:1;
    } field;
    
     tU8 value;        
} DABMW_SF_FreqStoringFlagTy;


/* Main strucuture & status for Service Following
*/ 

typedef struct
{
    // Algorithm status
    DABMW_serviceFollowingStateMachineTy status;
    DABMW_serviceFollowingStateMachineTy nextStatus;
    DABMW_serviceFollowingStateMachineTy prevStatus;
    
    // AF check ongoing flag
    tBool afCheckOngoing;

    DABMW_SF_TypeOfScanTy ScanTypeDab;
    DABMW_SF_TypeOfScanTy ScanTypeFm;

    // Store the system time when we started PI search
    SF_tMSecond piWaitTimeStart;
	SF_tMSecond bg_timerStart;
	
		
    // Input data for system switch
    /* EPR change 
	  * add some timer for monitoring 
    	*/
    SF_tMSecond deltaTimeFromLastSwitch;
	SF_tMSecond lastServiceRecoveryTime;
	SF_tMSecond idle_timerStart;
	SF_tMSecond lastSearchForAFTime;
    SF_tMSecond lastFullScanForAFTime;
	SF_tMSecond lastSwitchTime;
	SF_tMSecond lastLandscaspeBuildingTime;
	SF_tMSecond lastFMLandscapeDelayEstimationTime;
    SF_tMSecond deltaTimeFromAlternateSwitch;
	/* END EPR CHANGE */
	// monitor specific to dab case for now, to get information when DAB audio is muted
	// 
	SF_tMSecond originalLastTimeNoAudio;
	SF_tMSecond alternateLastTimeNoAudio;

    // Original data source (user tuned one)
    DABMW_SF_mwAppTy originalAudioUserApp;
	// Addition for ETAL 
	// 
	DABMW_SF_mwEtalHandleTy originalHandle;
	
	// end addition for ETAL
    DABMW_SF_systemBandsTy originalSystemBand;
    tU32 originalSid;
    tU32 originalEid;
    tU32 originalFrequency;
    tBool original_DabServiceSelected;
	 /* EPR change 
	  * add quality information : storing BER of FM quality depending on bearer
	  * add quality counter : usefull for evaluation and thresholds
    	 */
    DABMW_SF_QualityTy original_Quality;
    DABMW_SF_QualityTy original_Quality_onBackground;
	tU8 original_badQualityCounter; 
	tU8 original_mediumQualityCounter; 
	/* END EPR CHANGE */
    DABMW_SF_FreqStoringFlagTy originalSource;

	/* EPR CHANGE */
	/* Add a variable to store if DAB first or not */
	tBool configurationIsDabFirst;
	/* END EPR CHANGE */
	
    // Current data source
    DABMW_SF_mwAppTy currentAudioUserApp;
	// Addition for ETAL 
	// 
	DABMW_SF_mwEtalHandleTy currentHandle;
	
	// end addition for ETAL
    DABMW_SF_systemBandsTy currentSystemBand;    
    tU32 currentSid;
    tU32 currentEid;
    tU32 currentFrequency;
    DABMW_SF_QualityTy current_Quality;
    DABMW_SF_QualityTy current_Quality_OnAlternatePath;

    // Alternate data source
    DABMW_SF_mwAppTy alternateApp;
	// Addition for ETAL 
	// 
	DABMW_SF_mwEtalHandleTy alternateHandle;
	
	// end addition for ETAL
    tU32 alternateFrequency;
    DABMW_SF_systemBandsTy alternateSystemBand;
	tU32 alternateSid;
    tBool alternateTuned;
    tBool alternate_DabServiceSelected;
    DABMW_SF_FreqStoringFlagTy alternateSource;
	
	tU32 SwitchDelayReference;
	tU32 measurementPeriodicityReference;
	// counter reference
	tU8 counter_NbMeasureStartAfSearch;
	tU8 counter_NbMeasureLossService;
	tU8 counter_NbMeasureSwitch;
	
	 /* EPR change 
	  * add quality information & Sid  storing BER of FM quality depending on bearer
    	 */
    DABMW_SF_QualityTy alternate_Quality;
	tU8 alternate_qualityCounter; 
	tU8 alternate_badQualityCounter;
	/* END EPR CHANGE */
	tU32 alternateEid;

    // Add information for hysteresis
    SF_tMSecond dab_hysteresis_start_time;
    SF_tMSecond fm_hysteresis_start_time;
 	
	/* EPR change
	* add a variable to store the DAB sync status information received in notification
	*/
	tU8 dab_syncStatus;

    //add a boolean to know if a SS action is pending or not
    tBool seamless_action_on_going;
	/* END EPR Change */
} DABMW_serviceFollowingStatusTy;


/* structure for other frequency check/scan
* used for AF check, background check, and background scan.
*/

/* -----------
* FM part : Alternate Frequency
* -----------
*/

/* EPR 
* Structure for alternate FM data storage for AF check, background check
* mapped to DAB one as much as possible.
* will be filled either from landscape, either from pure AF info from RDS, either from AF info in DAB.
*/
typedef struct
{
    // Detection data
    tU32 frequency;
    tU32 piValue;
    DABMW_SF_QualityTy quality;
	tBool checkedForQuality;

    // flag to know if frequency comes from landscape db (default = false), or is part of RDS Alternate Freq List (if true)
    DABMW_SF_FreqStoringFlagTy  statusFlagInfo;
 
    // add a frequency status info : to store if  frequency is kept, if not, why it is skipped.
    DABMW_SF_FrequencyStatusTy    bgCheckStatus;        
} DABMW_afFmEntryTy;

typedef struct
{
    // Status
    DABMW_storageStatusEnumTy status;
    tSInt index;
	        
    // Output data (quality indicators)
    DABMW_afFmEntryTy entry[DABMW_SERVICE_FOLLOWING_MAX_NUM_FM_AF];   
} DABMW_afFmDataTy;

/* AF check specifics
*/
   

/* -----------
* DAB part : Alternate Frequency
* -----------
*/


/* New entry */

typedef struct
{
    // Detection data
    tU32 uniqueEid;
	tSInt  num_Sid;
    tU32 uniqueSidList[DABMW_FIC_MAX_SERVICES_NUMBER];
	tBool SearchedSidFound;
    tU32 frequency;
    tBool checkedForQuality;

    // Output data (quality indicators)
    tU16 ficBer;
    DABMW_SF_QualityTy quality;

    DABMW_SF_FreqStoringFlagTy  statusFlagInfo;
    // add a frequency status info : to store if  frequency is kept, if not, why it is skipped.
    DABMW_SF_FrequencyStatusTy    bgCheckStatus;
} DABMW_AF_DabEntryTy;

typedef struct
{
    // Status
    DABMW_storageStatusEnumTy status;
    tSInt index;

    DABMW_AF_DabEntryTy entry[DABMW_FIC_MAX_ENSEMBLE_NUMBER];
} DABMW_AF_DabDataTy;


/* -----------
* GLOBAL  part : store list of Frequency to be checked 
* 
* -----------
*/
typedef enum
{
	DABMW_SF_FreqCheck_DAB_BACKGROUND_CHECK				 = 0,
	DABMW_SF_FreqCheck_FM_BACKGROUND_CHECK 				 = 1,	 
	DABMW_SF_FreqCheck_FM_AF_CHECK						 = 2,
	DABMW_SF_FreqCheck_FM_AF_CHECK_ERROR				 = 100	  
} DABMW_SF_FreqCheckStatusTy;


typedef struct
{
	DABMW_AF_DabDataTy				DAB_FrequencyCheck;
	tU16							numDabFreqForCheck;
	DABMW_afFmDataTy				FM_FrequencyCheck;
	tU16							numFMFreqForCheck;
	DABMW_SF_FreqCheckStatusTy 		freqCheck_status;					
} DABMW_FreqCheckListEntryTy;


/* ----------
*  configuration  storage
* -------------
*/
typedef struct
{
    // Keep & store the initial requested PI
    tU32 initial_searchedPI;

    // Inputs from the user
	tBool enableServiceFollowing; 
	tBool seamlessSwitchingMode;
	tBool followSoftLinkage; 

	tU8 kindOfSwitch;								 
	tBool dabToDabIsActive;
	tBool dabToFmIsActive;
	tBool fmToDabIsActive;
	tBool fmToFmIsActive;

    // thresholds
    DABMW_SF_QualityTy  goodQuality_Threshold;
    DABMW_SF_QualityTy  poorQuality_Threshold;
    
    DABMW_SF_QualityTy  goodQuality_Threshold_dabplus_dmb;
    DABMW_SF_QualityTy  poorQuality_Threshold_dabplus_dmb;


    // CODEX #306445
    // add an hysteresis information 
    // it will be applied to limit the transition from DAB - FM after a switch is done
    // Switch from FM to DAB ... then level to come back to DAB should be highered with a hysteresys. 
    // Example :
    //      DAB threshold for being GOOD is fic_ber < good_threshold (10000)
    //      DAB is good with a BER = 8000, FM is GOOD. DAB  decrease to MEDIUM with BER of 20000, switch to FM is done.
    //      then for DAB to become GOOD again and switch to DAB to be done, an hystereys is added. DAB should be not less than good_threshold, but less than < good_threshold (10000) - Hysteresys...
    //      the Hysteresys will make a penality and smooth the too many transition at the border of GOOD/MEDIUMM levels...
    //      That Hysteresys may be decreasing with the duration... so that after some time we accept back the transitions... 
    //      this is applyed only on the alternate.
    //      
    // As on optimization, it is made variable in the time ... ie from (MAX to NULL) after a certain time with linear decrease
    DABMW_SF_QualityTy goodQuality_Threshold_Hysteresis;
    DABMW_SF_QualityTy poorQuality_Threshold_Hysteresis;
    tU32 Quality_Threshold_Hysteresis_Duration;

    // strategy
    DABMW_SF_Strategy_FM strategy_FM;
    DABMW_SF_Strategy_DAB strategy_DAB;    
	
	tU32 dabToFmDeltaTime; 
	tU32 fmToDabDeltaTime;
	tU32 dabToDabDeltaTime; 
	tU32 fmToFmDeltaTime;

	/* EPR change 
	* add new parameters 
	*/
	
	// input internal set for now : 
	// threshold and counter for measure and evaluation criteria
	/* tU8 counter_NbMeasureStartAfSearch;
	tU8 counter_NbMeasureLossService;
	tU8 counter_NbMeasureSwitch;
	*/
	// add different counter for FM and DAB to manage different measurement periodicity
	// 
	tU8 counter_NbMeasureStartAfSearch_FM;
	tU8 counter_NbMeasureLossService_FM;
	tU8 counter_NbMeasureSwitch_FM;
	tU8 counter_NbMeasureStartAfSearch_DAB;
	tU8 counter_NbMeasureLossService_DAB;
	tU8 counter_NbMeasureSwitch_DAB;
	// end counter DAB & FM
	tU8 ponderation_CurrentQuality;
	tU8 ponderation_NewQuality;

	/* periodicity information for measures, recovery algo and alternate check */
	tU32 measurementPeriodicityFM;
	tU32 measurementPeriodicityDAB;
	tU32 AFsearchPeriodicity;
    tU32 FullScanAFsearchPeriodicity;
	tU32 ServiceRecoverySearchPeriodicity;
	tU32 LandscapeBuildingScanPeriodicity;
	tU32 FMLandscapeDelayEstimationPeriodicity;
    tU32 LogInfoStatusPeriodicity;

	/* Internal follow hard linkage */
	tBool followHardLinkage;


	/* Regional service mode */
	tBool regionalModeIsOn;

    // stored the time to wait for a PI as configurable
    // and the threshold to decode a PI
    //
    tU32 maxTimeToDecodePi; 
	tU32 maxTimeToDecodePs;
    DABMW_SF_QualityTy  PI_decoding_QualityThreshold;
    tU32 PI_validityTime;
    
#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
	/* EPR TMP CHANGE */
	/* add variable for simulating DAB, FM mobility & level */
    tBool lockFeatureActivated_FM;
    tBool lockFeatureActivated_DAB;
    tBool lockFeatureActivated_2Freq;
    tU32 lockFrequency_1;
    tU8  lockFrequency_1_level;   
    tU32 lockFrequency_2;
    tU8  lockFrequency_2_level;   
    
    /* add variable for simulating DAB, FM level */
	tU8 FM_LevelSimulate;
	tU8 DAB_LevelSimulate;
	/* END EPR CHANGE */
#endif
} DABMW_serviceFollowingDataTy;


/*
*********************
* VARIABLE SECTION
**********************
*/
/* variables are belonging to SERVICE_FOLLOWING_C
*/
#ifndef SERVICE_FOLLOWING_C
#define GLOBAL	extern
#else
#define GLOBAL
#endif

// Alternate DAB data to search for alternatives to the current selection
// NOT USED 
//GLOBAL DABMW_afDabDataTy DABMW_afDabData;

// Structure where the AF LIST is copied for quality checks
// NOT USED 
// GLOBAL DABMW_afDataTy DABMW_currentAfData;

// Data currently used to check for a possible service following in the DAB or
// in the FM band

GLOBAL DABMW_serviceFollowingStatusTy DABMW_serviceFollowingStatus;


GLOBAL DABMW_FreqCheckListEntryTy DABMW_SF_freqCheckList;

// This variable stores the service followinng data input from the user.
// These data is transferred to the variable DABMW_serviceFollowingStatus
// according to the source status (band currenlty tuned)
GLOBAL DABMW_serviceFollowingDataTy DABMW_serviceFollowingData;

#undef GLOBAL

/*
*********************
* FUNCTIONS SECTION
**********************
*/
#ifndef SERVICE_FOLLOWING_C
#define GLOBAL	extern
#else
#define GLOBAL
#endif



GLOBAL tSInt DABMW_ServiceFollowingSetup (tBool enableServiceFollowing, tBool seamlessSwitchingMode,
                                          tBool followSoftLinkage, tU8 kindOfSwitch,                                 
                                          tU32 dabGoodQualityThr, tU32 dabPoorQualityThr,
                                          tU8 fmGoodQualityThr, tU8 fmPoorQualityThr,
                                          tU8 dabToFmDeltaTime, tU8 fmToDabDeltaTime,
                                          tU8 dabToDabDeltaTime, tU8 fmToFmDeltaTime);


/* Procedure handling the main algorithm for background check/scan
*/
GLOBAL tSInt DABMW_ServiceFollowing_BackgroundMainLoop(tVoid);

GLOBAL tPChar DABMW_ServiceFollowing_StateName(DABMW_serviceFollowingStateMachineTy vI_state);

GLOBAL tVoid DABMW_ServiceFollowing_ChangeState(DABMW_serviceFollowingStateMachineTy vI_NewState);

GLOBAL tSInt DABMW_ServiceFollowing_EnableRds(DABMW_SF_mwAppTy app, tBool vI_fastPI);

GLOBAL tSInt DABMW_ServiceFollowing_DisableRds(DABMW_SF_mwAppTy app, tBool vI_fastPI);


/* procedure to check if a frequency is an alternate Frequency FM
*/
GLOBAL tBool DABMW_ServiceFollowing_IsFrequencyInAlternateFM(tU32 vI_frequency);
    
/* procedure to check if a frequency is an alternate Frequency DAB
*/
GLOBAL tBool DABMW_ServiceFollowing_IsFrequencyInAlternateDAB(tU32 vI_frequency);
    
GLOBAL tVoid DABMW_ServiceFollowing_StopOnGoingActivities(tVoid);


#ifdef CONFIG_DABMW_SERVICE_FOLLOWING_SIMULATE_MOBILITY_DBG
GLOBAL tVoid DABMW_ServiceFollowingSimulate_DAB_FM_LevelChangeReset (tVoid);
#endif

// 
GLOBAL tSInt DABMW_ServiceFollowingOnTune (DABMW_SF_mwAppTy application, DABMW_systemBandsTy systemBand, tU32 frequency, tU32 ensemble, tU32 service, tBool isInternalTune, DABMW_SF_mwEtalHandleTy handle);

#undef GLOBAL

#ifdef __cplusplus
}
#endif

#endif // SERVICE_FOLLOWING_INTERNAL_H_

// End of file

