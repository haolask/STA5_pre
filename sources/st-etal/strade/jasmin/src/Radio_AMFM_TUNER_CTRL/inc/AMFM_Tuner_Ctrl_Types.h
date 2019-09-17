

/*=============================================================================
    start of file
=============================================================================*/
/*************************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_Types.h																		                 *
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														             *
*  All rights reserved. Reproduction in whole or part is prohibited											             *
*  without the written permission of the copyright owner.													             *
*																											             *
*  Project              : ST_Radio_Middleware																		                     *
*  Organization			: Jasmin Infotech Pvt. Ltd.															             *
*  Module				: SC_AMFM_TUNER_CTRL																             *
*  Description			: The file contains all structure and enum of AMFM Tuner Control                                 *
                                                                                                                         *																											*
*																											             *
**************************************************************************************************************************/


/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "cfg_types.h"

/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

#ifndef AMFM_TUNER_CTRL_STRUCTURES_
#define AMFM_TUNER_CTRL_STRUCTURES_



#define AMFM_TUNER_CTRL_INST_HSM_CID 					            ((Tu16)(0x2029))				/* Comments this has to be modified according to design */


#define AMFM_TUNER_CTRL_MAX_PS_SIZE   						        ((Tu8)(0x08))  
#define AMFM_TUNER_CTRL_MAX_RT_SIZE									((Tu8)64u)

#define AMFM_TUNER_CTRL_MAX_AF_LIST  								((Tu8)25u)
#define AMFM_TUNER_CTRL_HSM_BASE                        			((Tu16)(0x7000))
#define AMFM_TUNER_CTRL_HSM_RES_BASE                    			((Tu16)(0x7500))
#define AMFM_TUNER_CTRL_HSM_NOTFY_BASE                  			((Tu16)(0x7A00))
#define FM_RDS_DECODE_MSG_BASE                          			((Tu16)(0x7B00))
#define AMFM_TUNER_CTRL_MAX_STATIONS                       			((Tu8)(60u))

/**
 * @brief startup request message id AMFM_TUNER_CTRL_STARTUP_REQID
 */
#define AMFM_TUNER_CTRL_STARTUP_REQID			          			(AMFM_TUNER_CTRL_HSM_BASE + 0u)

/**
 * @brief updatestationlist request message id AMFM_TUNER_CTRL_GETSTATIONLIST_REQID
 */
#define AMFM_TUNER_CTRL_GETSTATIONLIST_REQID			  			(AMFM_TUNER_CTRL_HSM_BASE + 1u)
/**
 * @brief tune request message id AMFM_TUNER_CTRL_TUNE_REQID
 */
#define AMFM_TUNER_CTRL_TUNE_REQID			              			(AMFM_TUNER_CTRL_HSM_BASE + 2u)
/**
 * @brief activate request message id AMFM_TUNER_CTRL_ACTIVATE_REQID
 */
#define AMFM_TUNER_CTRL_ACTIVATE_REQID			          			(AMFM_TUNER_CTRL_HSM_BASE + 3u)
/**
 * @brief deactivate request message id AMFM_TUNER_CTRL_DEACTIVATE_REQID
 */
#define AMFM_TUNER_CTRL_DEACTIVATE_REQID			      			(AMFM_TUNER_CTRL_HSM_BASE + 4u)
/**
 * @brief shutdown request message id AMFM_TUNER_CTRL_SHUTDOWN_REQID
 */
#define AMFM_TUNER_CTRL_SHUTDOWN_REQID			          			(AMFM_TUNER_CTRL_HSM_BASE + 5u)
/**
 * @brief instance hsm startup message id from main hsm to instance hsm AMFM_TUNER_CTRL_INST_HSM_STARTUP
 */
#define AMFM_TUNER_CTRL_INST_START_REQID			     			(AMFM_TUNER_CTRL_HSM_BASE + 6u)
/**
 * @brief instance hsm stop message id from main hsm to instance hsm AMFM_TUNER_CTRL_INST_HSM_STARTUP
 */
#define AMFM_TUNER_CTRL_STOP			                  			(AMFM_TUNER_CTRL_HSM_BASE + 7u)
/**
 * @brief shutdown request message id AMFM_TUNER_CTRL_ERROR_REQID
 */
#define AMFM_TUNER_CTRL_ERROR_REQID			              			(AMFM_TUNER_CTRL_HSM_BASE + 8u)

#define AMFM_TUNER_CTRL_SEEK_UP_DOWN_REQID	              			(AMFM_TUNER_CTRL_HSM_BASE + 9u)

#define AMFM_TUNER_CTRL_GET_PI_LIST_REQID	              			(AMFM_TUNER_CTRL_HSM_BASE + 10u)


#define AMFM_TUNER_CTRL_BGQUALITY_REQID                     	    (AMFM_TUNER_CTRL_HSM_BASE + 11u)
#define AMFM_TUNER_CTRL_AF_UPDATE_REQID								(AMFM_TUNER_CTRL_HSM_BASE + 12u)
#define AMFM_TUNER_CTRL_AF_CHECK_REQID                              (AMFM_TUNER_CTRL_HSM_BASE + 13u)
#define AMFM_TUNER_CTRL_AF_JUMP_REQID                               (AMFM_TUNER_CTRL_HSM_BASE + 14u)
#define AMFM_TUNER_CTRL_CANCEL_REQID           						(AMFM_TUNER_CTRL_HSM_BASE + 15u)                           
#define AMFM_TUNER_CTRL_PISEEK_REQID           						(AMFM_TUNER_CTRL_HSM_BASE + 16u)
#define STOP_RDS_MSG_ID												(AMFM_TUNER_CTRL_HSM_BASE + 17u)
#define AMFM_TUNER_CTRL_LISTEN_RDS_START_REQID						(AMFM_TUNER_CTRL_HSM_BASE + 18u)
#define AMFM_TUNER_CTRL_SCAN_RDS_START_REQID						(AMFM_TUNER_CTRL_HSM_BASE + 19u)
#define AMFM_TUNER_CTRL_ANNOUNCEMENT_CANCEL_REQID					(AMFM_TUNER_CTRL_HSM_BASE + 20u)
#define AMFM_TUNER_CTRL_QUAL_READ                                   (AMFM_TUNER_CTRL_HSM_BASE + 21u)
#define AMFM_TUNER_CTRL_QUAL_READ_FAIL                              (AMFM_TUNER_CTRL_HSM_BASE + 22u)
#define AMFM_TUNER_CTRL_SEEK_START_FAIL                             (AMFM_TUNER_CTRL_HSM_BASE + 23u)
#define AM_FM_TUNER_CTRL_SOC_SEEK_TUNE_NEXT_FREQ_REQ                (AMFM_TUNER_CTRL_HSM_BASE + 24u)    
#define AMFM_TUNER_CTRL_SEEK_CONTINUE_FAIL                          (AMFM_TUNER_CTRL_HSM_BASE + 25u) 
#define AMFM_TUNER_CTRL_SEEK_STOP                                   (AMFM_TUNER_CTRL_HSM_BASE + 26u)
#define AMFM_TUNER_CTRL_SEEK_STOP_FAILS                             (AMFM_TUNER_CTRL_HSM_BASE + 27u)
#define AMFM_TUNER_CTRL_RDS_REQID								    (AMFM_TUNER_CTRL_HSM_BASE + 28u)
#define AMFM_TUNER_CTRL_TUNE_FAIL                                   (AMFM_TUNER_CTRL_HSM_BASE + 29u)
#define AMFM_TUNER_CTRL_LOWSIGNAL_AF_CHECK_REQID					(AMFM_TUNER_CTRL_HSM_BASE + 30u)
#define AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_REQID				(AMFM_TUNER_CTRL_HSM_BASE + 31u)
#define AMFM_TUNER_CTRL_FACTORY_RESET_REQID							(AMFM_TUNER_CTRL_HSM_BASE + 32u)
#define AMFM_TUNER_CTRL_SEEK_INFO_RESID                            (AMFM_TUNER_CTRL_HSM_BASE + 33u)
#define AMFM_TUNER_CTRL_SEEK_RESULT_RESID                           (AMFM_TUNER_CTRL_HSM_BASE + 34u)
#define AMFM_TUNER_CTRL_SEEK_FINISH_RESID                           (AMFM_TUNER_CTRL_HSM_BASE + 35u)


/**
 * @brief SOC startup response message id AMFM_TUNER_CTRL_START_DONE_RESID
 */
 #define AMFM_TUNER_CTRL_START_DONE_RESID			      			(AMFM_TUNER_CTRL_HSM_RES_BASE + 0u)
/**
 * @brief SOC scan response message id AMFM_TUNER_CTRL_SCAN_DONE_RESID
 */
#define AMFM_TUNER_CTRL_SCAN_DONE_RESID			          			(AMFM_TUNER_CTRL_HSM_RES_BASE + 1u)
/**
 * @brief SOC tune response message id AMFM_TUNER_CTRL_TUNE_DONE_RESID
 */
#define AMFM_TUNER_CTRL_TUNE_DONE_RESID			          			(AMFM_TUNER_CTRL_HSM_RES_BASE + 2u)
/**
 * @brief SOC activate response message id AMFM_TUNER_CTRL_ACTIVATE_RESID
 */
#define AMFM_TUNER_CTRL_ACTIVATE_RESID			          			(AMFM_TUNER_CTRL_HSM_RES_BASE + 3u)
/**
 * @brief SOC deactivate response message id AMFM_TUNER_CTRL_ACTIVATE_RESID
 */
#define AMFM_TUNER_CTRL_DEACTIVATE_RESID			      			(AMFM_TUNER_CTRL_HSM_RES_BASE + 4u)
/**
 * @brief SOC stop response message id AMFM_TUNER_CTRL_STOP_DONE_RESID
 */
#define AMFM_TUNER_CTRL_STOP_DONE_RESID			          			(AMFM_TUNER_CTRL_HSM_RES_BASE + 5u)
/**
 * @brief instance hsm start internal message to main hsm id AMFM_TUNER_CTRL_INST_HSM_START_DONE 
 */
#define AMFM_TUNER_CTRL_INST_HSM_START_DONE			      			(AMFM_TUNER_CTRL_HSM_RES_BASE + 6u)
/**
 * @brief instance hsm start internal message to main hsm id AMFM_TUNER_CTRL_INST_HSM_STOP_DONE 
 */
#define AMFM_TUNER_CTRL_INST_HSM_STOP_DONE			      			(AMFM_TUNER_CTRL_HSM_RES_BASE + 7u)
/**
 * @brief instance hsm start internal message to main hsm id AMFM_TUNER_CTRL_INST_HSM_SHUTDOWN_DONE 
 */
#define AMFM_TUNER_CTRL_INST_HSM_SHUTDOWN_DONE			  			(AMFM_TUNER_CTRL_HSM_RES_BASE + 8u)
/**
 * @brief SOC quality response message id AMFM_TUNER_CTRL_QUALITY_DONE_RESID
 */
#define AMFM_TUNER_CTRL_FM_QUALITY_DONE_RESID      		 			(AMFM_TUNER_CTRL_HSM_RES_BASE + 9u)
/**
 * @brief SOC RDS response message id AMFM_TUNER_CTRL_RDS_DONE_RESID
 */
#define AMFM_TUNER_CTRL_RDS_DONE_RESID      		      			(AMFM_TUNER_CTRL_HSM_RES_BASE + 10u)
/**
 * @brief SOC scan quality response message id AMFM_TUNER_CTRL_AMFMSCAN_QUALITY_READ_MSGID
 */
#define AMFM_TUNER_CTRL_AMFMSCAN_QUALITY_READ_MSGID         		(AMFM_TUNER_CTRL_HSM_RES_BASE + 11u)
/**
 * @brief SOC scan RDS response message id AMFM_TUNER_CTRL_SCAN_RDS_DONE_RESID
 */
//#define AMFM_TUNER_CTRL_SCAN_RDS_DONE_RESID      		  			(AMFM_TUNER_CTRL_HSM_RES_BASE + 12u)
/**
 * @brief timer for startup
 */
#define AMFM_TUNER_CTRL_STARTUP_TIMER								(AMFM_TUNER_CTRL_HSM_RES_BASE + 13u)
/**
 * @brief SOC AM/FM scan Start message id AMFM_TUNER_CTRL_SCAN_START_MSGID
 */
#define AMFM_TUNER_CTRL_SCAN_START_MSGID		 			        (AMFM_TUNER_CTRL_HSM_RES_BASE + 14u)
/**
 * @brief SOC AM scan quality response message id AMFM_TUNER_CTRL_AM_QUALITY_DONE_RESID
 */
#define AMFM_TUNER_CTRL_AM_QUALITY_DONE_RESID						(AMFM_TUNER_CTRL_HSM_RES_BASE + 15u)

/**
 * @brief SOC tune response for FM scan continue message id AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID
 */
#define AM_FM_TUNER_CTRL_SOC_FMSCAN_CONTINUE_MSGID				    (AMFM_TUNER_CTRL_HSM_RES_BASE + 16u)

#define AM_FM_TUNER_CTRL_SOC_AMSCAN_CONTINUE_MSGID					(AMFM_TUNER_CTRL_HSM_RES_BASE + 17u)

#define AMFM_TUNER_CTRL_SCAN_ERROR_RESID							(AMFM_TUNER_CTRL_HSM_RES_BASE + 18u)

#define AMFM_TUNER_CTRL_TUNE_ERROR_RESID							(AMFM_TUNER_CTRL_HSM_RES_BASE + 19u)

#define AMFM_TUNER_CTRL_SEEK_READ_RDS_RESID							(AMFM_TUNER_CTRL_HSM_RES_BASE + 20u)

#define AMFM_TUNER_CTRL_FM_AF_UPDATE_QUALITY_DONE_RESID             (AMFM_TUNER_CTRL_HSM_RES_BASE + 21u)

#define AMFM_TUNER_CTRL_FM_AF_CHECK_QUALITY_DONE_RESID              (AMFM_TUNER_CTRL_HSM_RES_BASE + 22u)

#define AMFM_TUNER_CTRL_FM_AF_JUMP_QUALITY_DONE_RESID               (AMFM_TUNER_CTRL_HSM_RES_BASE + 23u)

#define AMFM_TUNER_CTRL_AMSEEK_FAST_QUALITY_RESID					(AMFM_TUNER_CTRL_HSM_RES_BASE + 24u)

#define AMFM_TUNER_CTRL_AF_QUALITY_DONE_RESID						(AMFM_TUNER_CTRL_HSM_RES_BASE + 25u)

#define AMFM_TUNER_CTRL_UPDATE_RDS_IN_FM_STL						(AMFM_TUNER_CTRL_HSM_RES_BASE + 26u)

#define AMFM_TUNER_CTRL_SEEK_DONE_RESID         					(AMFM_TUNER_CTRL_HSM_RES_BASE + 27u)

#define STOP_DONE_MSG_ID											(AMFM_TUNER_CTRL_HSM_RES_BASE + 28u)
/**
 * @brief SOC FM_Check response message id AMFM_TUNER_CTRL_FMCHECK_DONE_RESID
 */
#define AMFM_TUNER_CTRL_FMCHECK_DONE_RESID 							(AMFM_TUNER_CTRL_HSM_RES_BASE + 29u)
			
#define AMFM_TUNER_CTRL_BGTUNE_DONE_RESID							(AMFM_TUNER_CTRL_HSM_RES_BASE + 30u)

/**
* @brief message id which gives Configure Receiver response
*/
#define AMFM_TUNER_CTRL_CONFIG_RECEIVER_DONE_RESID                   (AMFM_TUNER_CTRL_HSM_RES_BASE + 31u)

/**
* @brief message id which gives Configure Datapath response
*/
#define AMFM_TUNER_CTRL_CONFIG_DATAPATH_DONE_RESID                   (AMFM_TUNER_CTRL_HSM_RES_BASE + 32u)

/**
* @brief message id which gives Destroy Datapath response
*/
#define AMFM_TUNER_CTRL_DESTROY_DATAPATH_DONE_RESID			         (AMFM_TUNER_CTRL_HSM_RES_BASE + 33u)
/** audio source select done resid**/
#define AMFM_TUNER_CTRL_AUDIO_SOURCE_SELECT_DONE_RESID				 (AMFM_TUNER_CTRL_HSM_RES_BASE + 34u)

/** change band receiver done resid**/
#define AMFM_TUNER_CTRL_CHANGE_BAND_RECEIVER_DONE_RESID				 (AMFM_TUNER_CTRL_HSM_RES_BASE + 35u)

/** change band receiver done resid**/
#define AMFM_TUNER_CTRL_STOP_RDS_DONE_RESID							 (AMFM_TUNER_CTRL_HSM_RES_BASE + 36u)

/**
* @brief message id which gives Scan start response
*/
#define AMFM_TUNER_CTRL_SCAN_START_FAIL_RESID                       (AMFM_TUNER_CTRL_HSM_RES_BASE + 37u)    

/**
* @brief message id which gives Scan continue response
*/
#define AMFM_TUNER_CTRL_SCAN_CONTINUE_FAIL_RESID                    (AMFM_TUNER_CTRL_HSM_RES_BASE + 38u)   

/**
* @brief message id which requests for scan stop
*/
#define AMFM_TUNER_CTRL_SCAN_STOP_MSGID                             (AMFM_TUNER_CTRL_HSM_RES_BASE + 39u)   

#define AMFM_TUNER_CTRL_LOWSIGNAL_FMCHECK_DONE_RESID				(AMFM_TUNER_CTRL_HSM_RES_BASE + 40u)

#define AMFM_TUNER_CTRL_INST_HSM_FACTORY_RESET_DONE					(AMFM_TUNER_CTRL_HSM_RES_BASE + 41u)

#define AMFM_TUNER_CTRL_START_RDS_DONE_RESID                        (AMFM_TUNER_CTRL_HSM_RES_BASE + 42u)

#define AMFM_TUNER_CTRL_AUTOSCAN_COMPLETE_MSGID                     (AMFM_TUNER_CTRL_HSM_RES_BASE + 43u)

#define AMFM_TUNER_CTRL_SEEK_STOP_MSGID                             (AMFM_TUNER_CTRL_HSM_RES_BASE + 44u)
/**
 * @brief Reading quality notification AMFM_TUNER_CTRL_NOTIFICATION
 */
#define AMFM_TUNER_CTRL_NOTIFICATION								(AMFM_TUNER_CTRL_HSM_NOTFY_BASE + 1u)

/**
 * @brief Tuner status information notification AMFM_TUNER_CTRL_NOTIFICATION
 */
#define AMFM_TUNER_CTRL_TUNER_STATUS_NOTIFICATION					(AMFM_TUNER_CTRL_HSM_NOTFY_BASE + 2u)

/**
 * @brief AM Tuner status information notification AMFM_TUNER_CTRL_NOTIFICATION
 */
#define AMFM_TUNER_CTRL_FMAMTUNER_STATUS_NOTIFICATION				(AMFM_TUNER_CTRL_HSM_NOTFY_BASE + 3u)

/**
 * @brief quality notification AMFM_TUNER_CTRL_NOTIFICATION
 */
#define QUALITY_NOTIFICATION_MSGID 									(AMFM_TUNER_CTRL_HSM_NOTFY_BASE + 4u)

/**
 * @brief  AMFM Tuner Abnormal status notification AMFM_TUNER_CTRL_AMFMTUNER_ABNORMAL_NOTIFICATION
 */
#define AMFM_TUNER_CTRL_AMFMTUNER_ABNORMAL_NOTIFICATION 					(AMFM_TUNER_CTRL_HSM_NOTFY_BASE + 5u)

/**
 * @brief RDS decoding started 
 */
#define FM_RDS_DECODE_START                      				   (FM_RDS_DECODE_MSG_BASE + 1u) 

#define RDS_CALLBACK_DATA_RDY_MSG_ID                               (FM_RDS_DECODE_MSG_BASE + 2u)

#define NON_RDS_NOTIFYID                                           (FM_RDS_DECODE_MSG_BASE + 3u)
/*---------------------------------------------------------------------------
    type definitions
-----------------------------------------------------------------------------*/

#define AMFM_FS_CONVERSION(FS)				(Ts32)((100u * (Ts32)(FS))  / 255u)      
#define AMFM_USN_CONVERSION(USNE)			(Tu32)((100u * (Tu32)(USNE))/ 255u)
#define AMFM_WAM_CONVERSION(WAM)			(Tu32)((100u * (Tu32)(WAM)) / 255u) 
#define AMFM_OFS_CONVERSION(OFS)			(Tu32)((100u * (Tu32)(OFS)) / 255u)
#define AMFM_SNR_CONVERSION(SNR)			(Tu32)((100u * (Tu32)(SNR)) / 255u) 
#define AMFM_MOD_CONVERSION(MOD)			(Tu32)((100u * (Tu32)(MOD)) / 255u) 
#define AMFM_ADJ_CONVERSION(ADJ)			(Tu32)((100u * (Tu32)(ADJ)) / 255u) 
#define AMFM_COCHANNEL_CONVERSION(COC)		(Tu32)((100u * (Tu32)(COC)) / 255u) 
/**
 *@subaddress read use is transmitted first bye followed by the desired read of one or more 
 */
#define AMFM_TUNER_CTRL_SUBADDR_READQUAL                 ((Tu8)(0x00))
/**
 *@For AM LW preset mode
 */
#define AMFM_TUNER_CTRL_AM_LW_PRESETMODE                  ((Tu8)(0x11))


/**
 *@For AM MW preset mode
 */
#define AMFM_TUNER_CTRL_AM_MW_PRESETMODE                  ((Tu8)(0x12))


/**
 *@For AM LW search  mode
 */
#define AMFM_TUNER_CTRL_AM_LW_SEARCHMODE                  ((Tu8)(0x21))

/**
 *@For AM MW search mode
 */
#define AMFM_TUNER_CTRL_AM_MW_SEARCHMODE                  ((Tu8)(0x22))

/**
 *@For FM preset mode
 */
#define AMFM_TUNER_CTRL_FM_PRESETMODE                     ((Tu8)(0x10))

/**
 *@For AF upadte mode
 */
#define AMFM_TUNER_CTRL_FM_AFUPDATEMODE                   ((Tu8)(0x30))

/**
 *@For FM check mode
 */
#define AMFM_TUNER_CTRL_FM_CHECKMODE                      ((Tu8)(0x50))

/**
 *@For FM jump mode
 */
#define AMFM_TUNER_CTRL_FM_JUMPMODE                       ((Tu8)(0x40))


/**
 *@For FM search mode
 */
#define AMFM_TUNER_CTRL_FM_SEARCHMODE                      ((Tu8)(0x20))

/**
 *@For FM AF bandwidth
 */
#define AMFM_TUNER_CTRL_AFUPDATE_BANDWIDTH                  ((Tu8)(0x08))

/**
 *@For AM LW end mode
 */
#define AMFM_TUNER_CTRL_AM_LW_ENDMODE                      ((Tu8)(0x71))

/**
 *@For AM MW end mode
 */
#define AMFM_TUNER_CTRL_AM_MW_ENDMODE                      ((Tu8)(0x72))

/**
 *@For AM LW end mode
 */
#define AMFM_TUNER_CTRL_FM_ENDMODE                          ((Tu8)(0x70))
/**
 *@For read back data
 */
#define AMFM_TUNER_CTRL_READBACK_WRITEDATA                   ((Tu8)(0xE0))
/**
 *@For masking the 8th bit of FOF byte from soc
 */
#define FOF_EXTRACT_MASK_VALUE			                     ((Tu8)(0x7F))

/**
 *@sub address indicating the first data byte to write to soc
 */
#define AMFM_TUNER_CTRL_SUBADDR_WRITEDATA                    ((Tu8)(0x00))

/**
 *@For average of jump quality parameters count
 */
#define AMFM_TUNER_CTRL_JUMPQUAL_AVGCOUNT                   ((Tu8)5)
/**
 *@For AM LW Stepsize
 */
#define AMFM_TUNER_CTRL_AM_LW_STEPSIZE                      ((Tu16)3)
/**
 *@For AM LW Stepsize
 */
#define AMFM_TUNER_CTRL_AM_MW_STEPSIZE                      ((Tu8)9)

/**
 *@For AM LW end frequency
 */
#define AMFM_TUNER_CTRL_AM_LW_ENDFREQ                       ((Tu16)288)

/**
 *@For AM MW startfrequency frequency
 */
#define AMFM_TUNER_CTRL_AM_MW_STARTFREQ                     ((Tu16)531)

/**
 *@For FM start frequency
 */
#define AMFM_TUNER_CTRL_FMSTARTFREQ                         ((Tu16)8750)


/**
 *@For size of i2c buffer
 */
#define AMFM_TUNER_CTRL_I2CBUFFSIZE                         ((Tu8)6)


/**
 *@For 8ms timer
 */
#define AMFM_TUNER_CTRL_AM_SEEK_SCAN_QUALDETECT_TIME        (8u)

/**
 *@For 4ms timer
 */
#define AMFM_TUNER_CTRL_FM_SEEK_SCAN_QUALDETECT_TIME         (4u)
/**
 *@For 5ms timer
 */
#define AMFM_TUNER_CTRL_AFUPDATE_QUALDETECT_TIME             (3u)

/**
 *@For 26ms timer
 */
#define AMFM_TUNER_CTRL_AM_FAST_SEEK_QUALDETECT_TIME         (26u)

/**
 *@For 26ms timer
 */
#define AMFM_TUNER_CTRL_AM_FAST_SCAN_QUALDETECT_TIME         (26u)

/**
 *@For 30ms timer
 */
#define AMFM_TUNER_CTRL_FM_FAST_SEEK_SCAN_QUALDETECT_TIME    (30u)

/**
 *@For 32ms timer
 */
#define AMFM_TUNER_CTRL_AMFMQUAL_DETECT_TIME                 (32u)
/**
 *@For 80ms timer
 */
#define AMFM_TUNER_CTRL_SOC_STARTUP_TIME                     (80u)

/**
*@For 400ms timer
*/
#define AMFM_TUNER_CTRL_FM_RDS_NOTIFY_TIME                   (400u)

/**
 *@For 2000ms timer
 */
#define AMFM_TUNER_CTRL_FMAM_TUNER_STATUS_NOTIFY_TIME        (2000u)


/**
 *@For maximun string size
 */
#define AMFM_TUNER_CTRL_MAXSTRSTATESIZE                      (100u)

/**
 *@For maximun count of core command req
 */
#define AMFM_TUNER_CTRL_MAX_REPEAT_COUNT                     (1u)

/**
 *@For maximun qual read count
 */
#define AMFM_TUNER_CTRL_MAX_QUALREAD_REPEAT_COUNT           (1u)

/**
 *@For maximun qual read count
 */
#define AMFM_TUNER_CTRL_MAX_SEEK_QUAL_READ_TIME             (10u)

/**
*@For Audio Playtime for each FGscan freq
*/
#define AMFM_TUNER_CTRL_AUTOSCAN_AUDIO_PLAY_TIME			(2000u)

/**
*@For maximun qual read count
*/
#define AMFM_TUNER_CTRL_MAX_TUNE_QUAL_READ_TIME             (20u)
typedef struct
{
 Tu32       u32_AMFM_TC_Timer_Id;                              								/**< Timer id for waiting to read quality paramters from soc */ 
 Tu32       u32_NonRDSFMAM_TunerStatus_Timerid;               								/**< Sending tuner status notification*/	
 Tu32       u32_RDS_Timer_id;                                									/**< RDS timer id */
 Tu32       u32_NonRDS_Timer_id;
}Ts_AMFM_Tuner_Ctrl_Timer_Ids;
/**
 *@brief declares the available wave bands which is used in tune services
 */
typedef enum
{
	TUN_BAND_AM,
    TUN_BAND_FM,																		/**< 0-FM wave band */
	TUN_BAND_AM_MW,																		/**< 1-AM Medium wave band */
	TUN_BAND_AM_LW,																		/**< 2-AM Long wave band */
	TUN_BAND_AM_SW,																		/**< 3-AM Short wave band */
    TUN_BAND_WB,																		/**< 4-Weather band */
	TUN_BAND_INVALID,																	/**< 5-invalid band, is used in case if tuner is off */

}Te_AMFM_Tuner_Ctrl_Band;


/**
 *@brief enum contains TP bit of the station as it was last received 
 */
typedef enum
{
	TP_TRUE,																			/**< 0-Traffic program is received */
	TP_FALSE																			/**< 1-Traffic program is not received */

}Te_AMFM_Tuner_Ctrl_tp_bool;

/**
 *@brief enum contains TA bit of station as it was last received  
 */
typedef enum
{
	TA_TRUE,																			/**< 0-Traffic announcement is received */
	TA_FALSE																			/**< 1-Traffic announcement is not received */

}Te_AMFM_Tuner_Ctrl_ta_bool;


/**
 *@brief enum contains methods of Alternating  Frequencies  
 */
 
typedef enum
{
    AMFM_TUNER_CTRL_AF_METHOD_A,                   										/**<0-Method A AF list is transmitting **/
    AMFM_TUNER_CTRL_AF_METHOD_B,                   										/**<1-Method B AF list is transmitting **/
    AMFM_TUNER_CTRL_INVALIDMETHOD                  										/**<2-Invalid method*/

}Te_AMFM_Tuner_Ctrl_AF_Methods;
 
 /** 
  *@brief structure comprises information of clock and time.
  */
 typedef  struct
{
	Tu8                                 u8_Hour;
    Tu8                                 u8_Min;
    Tu8		                            u8_offset_sign;
    Tu8                                 u8_Localtime_offset;       
    Tu8                                 u8_Day;                    
	Tu8                                 u8_Month;
	Tu16                                u16_Year;

}Ts_AMFM_Tuner_Ctrl_CT_Info;
 /** 
  *@brief structure comprises information of tuned station.
  */
typedef struct
{
	Te_AMFM_Tuner_Ctrl_Band                e_Band;										/**< the band to be tuned*/
	Tu32								   u32_freq;
    Tu16								   u16_pi;										/**< program identifier*/
    Tu8                                    au8_ps[AMFM_TUNER_CTRL_MAX_PS_SIZE];			/**< contains the program station name*/
 	Tu8                                    u8_status;									/**< quality of the station */
	Tu32								   u32_FrequencyOffset;										/**< frequency offset in kHz */
    Ts32							       s32_RFFieldStrength     ;
	Ts32								   s32_BBFieldStrength     ;                   /**< field strength of the station */
	Tu32								   u32_ModulationDetector  ;                   /**< modulation in case of FM */
	Tu32								   u32_Multipath           ;                   /**< multipath noise of station */
	Tu32								   u32_UltrasonicNoise     ;                   /**< ultrasonic noise of station */
	Tu32								   u32_AdjacentChannel     ;                   /**< Adjacent channel noise of station */
	Tu32								   u32_SNR                 ;                   /**< Signal to Noise Ratio */
	Tu32								   u32_coChannel           ;                   /**Cochannel*/
	Tu32								   u32_StereoMonoReception ;                   /**Whenther Audio is stereo or Mono  */
	Tu32								   u32_quality_interpolation;                    /**< calculated interpolation of station by using quality parametrs*/
	Te_RADIO_ReplyStatus		           SOC_Status;                                  /**< status of SOC*/
    //Added for AF List: Start 
	Tu32								   u32_AFeqList[AMFM_TUNER_CTRL_MAX_AF_LIST];  					/**< list of AF*/ 
	Tu8                                    u8_NumofAF_SamePgm;                          /**< Number of same Alternative frequencies */
	Tu8                                    u8_NumofAF_RgnlPgm;                          /**< Number of regional Alternative frequencies */
	Tu8									   u8_NumAFeqList;  							/**< Number of Alternative frequencies*/
	Tbool                                  b_AF_Checkbit ;                                 /**< enaled if any AF list changed */
    Te_AMFM_Tuner_Ctrl_AF_Methods          e_AF_Methods;                                /**< enum for methods of AF list is tramsnitting*/
	//Added for AF List: End
	Tu8                                    au8_RadioText[AMFM_TUNER_CTRL_MAX_RT_SIZE];           		/**< Radio text of the tuned station*/
	Tu8                                    u8_RT_size;                                  /**< size of Radio text */         
	//Added for CT : Start 
	Ts_AMFM_Tuner_Ctrl_CT_Info			    st_CT_Info;									/**< Structure of the Clock Time */    
	//Added for CT: End      
	Tu8								        u8_PTYCode;	     
	Tu8								        u8_TA;
	Tu8								        u8_TP;
}Ts_AMFM_Tuner_Ctrl_CurrStationInfo;


typedef struct 
{

    Tu8									 u8_status;										/**< quality of the station, see #TUN_QUAL_ERROR, #TUN_QUAL_MIN and #TUN_QUAL_MAX for valid ranges! */
    Ts32								 s32_BBFieldStrength;							/**< Baseband field strength in dBuV */
	Tu32								 u32_UltrasonicNoise;								        /**< neighbour channel disturbance indication */
    Tu32								 u32_Multipath;											/**< multipath indication */
    Tu32								 u32_FrequencyOffset;										/**< frequency offset in kHz */
   	Tu32								 u32_ModulationDetector;										/**< modulation in case of FM */
	Ts32								 s32_RFFieldStrength;
	Tu32								 u32_SNR;
	Tu32								 u32_AdjacentChannel;
	Tu32								 u32_coChannel;
	Tu32								 u32_StereoMonoReception;
}SOC_QUAL;

 /** 
  *@brief structure comprises information of station list.
  */
typedef struct 
{
   
	Tu32 		                            u32_Numberofstations;				   		/**<Number of stations*/
	Ts_AMFM_Tuner_Ctrl_CurrStationInfo	  ast_CurrentStationInfo[AMFM_TUNER_CTRL_MAX_STATIONS];  		/**< Sattion list*/

}Ts_AMFM_Tuner_Ctrl_Station_List;

 /** 
  *@brief structure comprises information of EON station.
  */
typedef struct
{
	Tu16			u16_EON_PI;                                                 
	Tu32			u32_EON_aflist[2];

}Ts_AMFM_TunerCtrl_EON_Info;
/**
 * \brief  This enum is used to indicate the Result of TA TP Found  
 */			
typedef enum
{
	AMFM_TUNER_CTRL_TATP,
	AMFM_TUNER_CTRL_EONTATP		
}Te_AMFM_Tuner_Ctrl_TATP;
/**
 *   \brief This enum represents different markets type
 */
typedef enum
{      																	
    AMFM_TUNER_CTRL_WESTERN_EUROPE,  																
    AMFM_TUNER_CTRL_LATIN_AMERICA,          														
    AMFM_TUNER_CTRL_ASIA_CHINA,         																
    AMFM_TUNER_CTRL_ARABIA,        																	
    AMFM_TUNER_CTRL_USA_NORTHAMERICA,            													
	AMFM_TUNER_CTRL_JAPAN,             															
	AMFM_TUNER_CTRL_KOREA,           
	AMFM_TUNER_CTRL_BRAZIL,    														
	AMFM_TUNER_CTRL_SOUTH_AMERICA,   														
	AMFM_TUNER_CTRL_MARKET_INVALID,  
}Te_AMFM_Tuner_Ctrl_Market;
/** 
 *@brief structure comprises information of get station request.
 */
 typedef struct
 {
	 Te_RADIO_DirectionType 	e_SeekDirection;								 	/**<Direction to perform seek operation */
	 Te_AMFM_Tuner_Ctrl_Band 	e_Band;												/**<the band to be tuned*/

     Tu32 U32_Startfreq;													    	/**<the start frequency to scan in kHz*/
	 Tu32 U32_Upperfreq;														 	/**<the upper frequency to scan in kHz*/
	 Tu32 U32_Lowerfreq;														 	/**<the lower frequency to scan in kHz*/
	 Tu32 U32_Stepsize;															 	/**<stepsize for scan in kHz*/

 }Ts_AMFM_Tuner_Ctrl_Getstationreq_info;

/** 
 *@brief structure comprises information of tune request.
 */
typedef struct 
{
	Te_AMFM_Tuner_Ctrl_Band   e_Band;											 	/**<the band to be tuned*/
	Tu32 u32_freq;	                                                                /**<the frequency to be tuned in kHz*/
}Ts_AMFM_Tuner_Ctrl_Tunereq_info;


typedef  struct
{
	Tu32                                 u32_interpolation		 ;
	Ts32							     s32_BBFieldStrength     ;
	Tu32								 u32_AdjacentChannel     ;
	Tu32								 u32_Multipath           ;
	Tu32								 u32_FrequencyOffset     ;
	Tu32								 u32_ModulationDetector  ;
	Tu32								 u32_UltrasonicNoise     ;
	Ts32								 s32_RFFieldStrength	 ;
	Tu32								 u32_SNR				 ;
	Tu32								 u32_coChannel			 ;
	Tu32								 u32_StereoMonoReception ;
}Ts_AMFM_Tuner_Ctrl_Interpolation_info;

typedef enum
{
    AMFM_STARTUP_SCAN,
    AMFM_NON_STARTUP_SCAN        
}Te_AMFM_Scan_Type;


typedef enum
{
	AMFM_TUNER_CTRL_FOREGROUND,
	AMFM_TUNER_CTRL_BACKGROUND
}Te_AMFM_Tuner_State;

/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/
#endif

/*=============================================================================
    end of file
=============================================================================*/








