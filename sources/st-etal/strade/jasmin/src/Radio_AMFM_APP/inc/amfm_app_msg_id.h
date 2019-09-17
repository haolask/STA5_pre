/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_msg_id.h																					*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of message IDs for all Request,Response  * 			
*						  and Notification APIs																*
*																											*
*************************************************************************************************************/
#ifndef AMFM_APP_MSG_ID_H
#define AMFM_APP_MSG_ID_H

/*-----------------------------------------------------------------------------
    Macro Definitions
-----------------------------------------------------------------------------*/
/*  Message Ids Range*/
#define AMFM_APP_REQID_BEGIN			((Tu16)0x5001)
#define AMFM_APP_RESID_BEGIN			((Tu16)0x5555)
#define AMFM_APP_NOTIFYID_BEGIN			((Tu16)0x5AAA)

#define AMFM_APP_MSGID_LIMIT			((Tu16)0x5FFF)


/* Request Ids for Internal Messages  */
#define AMFM_APP_INST_HSM_STARTUP							(AMFM_APP_REQID_BEGIN + (Tu16)1u)			/*0x5002*/		
#define AMFM_APP_INST_HSM_START_DONE						(AMFM_APP_REQID_BEGIN + (Tu16)2u)			/*0x5003*/
#define AMFM_APP_INST_HSM_SHUTDOWN							(AMFM_APP_REQID_BEGIN + (Tu16)3u)			/*0x5004*/
#define AMFM_APP_INST_HSM_SHUTDOWN_DONE						(AMFM_APP_REQID_BEGIN + (Tu16)4u)			/*0x5005*/


/*  Request IDs  */
#define AMFM_APP_STARTUP_REQID								(AMFM_APP_REQID_BEGIN + (Tu16)5u)			/*0x5006*/
#define AMFM_APP_SHUTDOWN_REQID								(AMFM_APP_REQID_BEGIN + (Tu16)6u)			/*0x5007*/
#define AMFM_APP_SELECT_BAND_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)7u)			/*0x5008*/
#define AMFM_APP_DESELECT_BAND_REQID						(AMFM_APP_REQID_BEGIN + (Tu16)8u)			/*0x5009*/
#define AMFM_APP_GET_STL_REQID								(AMFM_APP_REQID_BEGIN + (Tu16)9u)			/*0x500A*/
#define AMFM_APP_SELECT_STATION_REQID       				(AMFM_APP_REQID_BEGIN + (Tu16)10u)			/*0x500B*/
#define AMFM_APP_SEEK_UP_DOWN_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)11u)			/*0x500C*/
#define AMFM_APP_FIND_BEST_PI_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)12u)			/*0x500D*/
#define AMFM_APP_BLENDING_STATUS_REQID						(AMFM_APP_REQID_BEGIN + (Tu16)13u)			/*0x500E*/
#define AMFM_APP_CANCEL_REQID								(AMFM_APP_REQID_BEGIN + (Tu16)14u)			/*0x500F*/
#define AMFM_APP_AF_SWITCH_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)15u)			/*0x5010*/
#define AMFM_APP_SET_AF_REGIONAL_SWITCH_REQID				(AMFM_APP_REQID_BEGIN + (Tu16)16u)			/*0x5011*/
#define AMFM_APP_TA_SWITCH_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)17u)			/*0x5012*/
#define AMFM_APP_FM_TO_DAB_SWITCH_REQID						(AMFM_APP_REQID_BEGIN + (Tu16)18u)			/*0x5013*/
#define AMFM_APP_BACKGROUND_UPDATE_STL_REQID				(AMFM_APP_REQID_BEGIN + (Tu16)19u)			/*0x5014*/
#define AMFM_APP_TUNE_UP_DOWN_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)20u)			/*0x5015*/
#define AMFM_APP_AF_TUNE_REQID								(AMFM_APP_REQID_BEGIN + (Tu16)21u)			/*0x5016*/
#define AMFM_APP_ANNOUNCEMENT_CANCEL_REQID					(AMFM_APP_REQID_BEGIN + (Tu16)22u)			/*0x5017*/
#define AMFM_APP_ENG_MODE_SWITCH_REQID						(AMFM_APP_REQID_BEGIN + (Tu16)23u)			/*0x5018*/
#define AMFM_APP_GET_CT_INFO_REQID							(AMFM_APP_REQID_BEGIN + (Tu16)24u)			/*0x5019*/
#define AMFM_APP_FACTORY_RESET_REQID						(AMFM_APP_REQID_BEGIN + (Tu16)25u)			/*0x501A*/

/* Response IDs */
#define AMFM_APP_STARTUP_DONE_RESID							(AMFM_APP_RESID_BEGIN + (Tu16)1u)			/*0x5556*/		
#define AMFM_APP_SHUTDOWN_DONE_RESID						(AMFM_APP_RESID_BEGIN + (Tu16)2u)			/*0x5557*/
#define AMFM_APP_SELECT_BAND_DONE_RESID						(AMFM_APP_RESID_BEGIN + (Tu16)3u)			/*0x5558*/
#define AMFM_APP_DESELECT_BAND_DONE_RESID					(AMFM_APP_RESID_BEGIN + (Tu16)4u)			/*0x5559*/
#define AMFM_APP_GET_STL_DONE_RESID							(AMFM_APP_RESID_BEGIN + (Tu16)5u)			/*0x555A*/
#define AMFM_APP_SELECT_STATION_DONE_RESID				    (AMFM_APP_RESID_BEGIN + (Tu16)6u)			/*0x555B*/
#define AMFM_APP_SEEK_UP_DOWN_DONE_RESID					(AMFM_APP_RESID_BEGIN + (Tu16)7u)			/*0x555C*/
#define AMFM_APP_GET_PI_LIST_DONE_RESID						(AMFM_APP_RESID_BEGIN + (Tu16)8u)			/*0x555D*/
#define AMFM_APP_AFFREQ_UPDATE_DONE_RESID					(AMFM_APP_RESID_BEGIN + (Tu16)9u)			/*0x555E*/
#define AMFM_APP_AFFREQ_CHECK_DONE_RESID					(AMFM_APP_RESID_BEGIN + (Tu16)10u)			/*0x555F*/
#define AMFM_APP_CANCEL_DONE_RESID							(AMFM_APP_RESID_BEGIN + (Tu16)11u)			/*0x5560*/
#define AMFM_APP_ANNO_CANCEL_DONE_RESID						(AMFM_APP_RESID_BEGIN + (Tu16)12u)			/*0x5561*/
#define AMFM_APP_AF_LOW_SIGNAL_CHECK_DONE_RESID				(AMFM_APP_RESID_BEGIN + (Tu16)13u)			/*0x5562*/
#define AMFM_APP_FACTORY_RESET_DONE_RESID					(AMFM_APP_RESID_BEGIN + (Tu16)14u)			/*0x5563*/
#define AMFM_APP_INST_HSM_RESET_DONE						(AMFM_APP_RESID_BEGIN + (Tu16)15u)			/*0x5564*/

/* Notify IDs */
#define AMFM_APP_STL_UPDATED_NOTIFYID						(AMFM_APP_NOTIFYID_BEGIN + (Tu16)1u)		/*0x5AAB*/
#define	AMFM_APP_TUNER_STATUS_NOTIFYID						(AMFM_APP_NOTIFYID_BEGIN + (Tu16)2u)		/*0x5AAC*/
#define	AMFM_APP_CURR_FREQUENCY_NOTIFYID					(AMFM_APP_NOTIFYID_BEGIN + (Tu16)3u)		/*0x5AAD*/
#define	AMFM_APP_QUALITY_NOTIFYID							(AMFM_APP_NOTIFYID_BEGIN + (Tu16)4u)		/*0x5AAE*/
#define AMFM_AF_UPDATE_TIMERID								(AMFM_APP_NOTIFYID_BEGIN + (Tu16)5u)		/*0x5AAF*/
#define	AMFM_APP_TUNER_AFQUAL_STATUS_NOTIFYID				(AMFM_APP_NOTIFYID_BEGIN + (Tu16)6u)		/*0x5AB0*/
#define	AMFM_APP_DAB_FOLLOWUP_NOTIFYID					    (AMFM_APP_NOTIFYID_BEGIN + (Tu16)7u)		/*0x5AB1*/
#define	AMFM_APP_EON_ANNOUNCEMENT_START_NOTIFIED			(AMFM_APP_NOTIFYID_BEGIN + (Tu16)8u)		/*0x5AB2*/
#define	AMFM_APP_CURRENT_STATION_TA_TP_NOTIFIED				(AMFM_APP_NOTIFYID_BEGIN + (Tu16)9u)		/*0x5AB3*/
#define	AMFM_APP_EON_INFO_NOTIFIED					    	(AMFM_APP_NOTIFYID_BEGIN + (Tu16)10u)		/*0x5AB4*/
#define	AMFM_APP_STOP_DAB2FM_LINKING_NOTIFYID		    	(AMFM_APP_NOTIFYID_BEGIN + (Tu16)11u)		/*0x5AB5*/
#define AMFM_CURR_STATION_QUALITY_CHECK_TIMERID				(AMFM_APP_NOTIFYID_BEGIN + (Tu16)12u)		/*0x5AB6*/
#define AMFM_APP_TUNER_AMFMTUNER_ABNORMAL_NOTIFYID			(AMFM_APP_NOTIFYID_BEGIN + (Tu16)13u)		/*0x5AB7*/
#define AMFM_APP_DABTUNER_STATUS_NOTIFYID					(AMFM_APP_NOTIFYID_BEGIN + (Tu16)14u)		/*0x5AB8*/
#define AMFM_REGIONAL_QUALITY_CHECK_TIMERID					(AMFM_APP_NOTIFYID_BEGIN + (Tu16)15u)		/*0x5AB9*/
#define AMFM_AF_STATUS_CHECK_NOTIFYID						(AMFM_APP_NOTIFYID_BEGIN + (Tu16)16u)		/*0x5ABA*/
#define AMFM_APP_BG_NEG_STATUS_CHECK_NOTIFYID				(AMFM_APP_NOTIFYID_BEGIN + (Tu16)17u)		/*0x5ABB*/
#define AMFM_APP_NON_RADIO_MODE_NOTIFYID					(AMFM_APP_NOTIFYID_BEGIN + (Tu16)18u)		/*0x5ABC*/
#define AMFM_APP_START_AF_STRATEGY_NOTIFYID					(AMFM_APP_NOTIFYID_BEGIN + (Tu16)19u)		/*0x5ABD*/
#define AMFM_APP_STA_NOT_AVAIL_STRA_STATUS_NOTIFYID         (AMFM_APP_NOTIFYID_BEGIN + (Tu16)20u)       /*0x5ABE*/
#define AMFM_APP_BG_AF_TUNE_UPDATE_TIMERID					(AMFM_APP_NOTIFYID_BEGIN + (Tu16)21u)		/*0x5ABF*/
#define AMFM_STRATEGY_AF_UPDATE_TIMERID						(AMFM_APP_NOTIFYID_BEGIN + (Tu16)22u)		/*0x5AC0*/


/* Component ID (CID) of INST HSM of AMFM APP component */
#define AMFM_APP_INST_HSM_CID		(Tu16)(0x5FFE)


#endif /* End of AMFM_APP_MSG_ID_H */