
/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file AMFM_Tuner_Ctrl_App.h																				*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																		        *
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: SC_AMFM_TUNER_CTRL																*
*  Description			: Declarations of instance hsm handle message										*
*																											*
*																											*
*************************************************************************************************************/

#ifndef AMFM_TUNER_CTRL_APP_H_
#define AMFM_TUNER_CTRL_APP_H_

/*-----------------------------------------------------------------------------
    includes
-----------------------------------------------------------------------------*/
#include "AMFM_HAL_Interface.h"
#include "AMFM_Tuner_Ctrl_Main_hsm.h"
#include "amfm_app_types.h"
#include "interpolation.h"
#include "AMFM_HAL_RDS_decoder.h"
/*-----------------------------------------------------------------------------
    defines
-----------------------------------------------------------------------------*/

#ifndef  CALIBRATION_TOOL

#define CS_RSSI		    	(Tu32)(16+(17*2))		//(16+(18*2))
#define CS_WAM		    	(Tu32)((255*23)/100)
#define CS_USN		    	(Tu32)((255*18)/100)
#define CS_FOF_FM    	 	(Tu32)0x0A


#define CS_FOF_AM			(Tu32)0x02
#define CS_FOF_AM_FAST		(Tu32)0x04
#define CS_AM_RSSI			(Tu32)(16+(36*2))


#define CS_RSSI_FAST		    (Tu32)(16+(15*2))		//(16+(18*2))
#define CS_WAM_FAST	    		(Tu32)((255*30)/100)
#define CS_USN_FAST	    		(Tu32)((255*23)/100)
#define CS_FOF_FAST	    		 0x14

#endif

#define INVALID_BAND    (Tu8)0

#define LW_WEU_START	144

#define MW_WEU_STEP		(Tu8)9
#define MW_WEU_UPPER	(Tu16)1629
/*-----------------------------------------------------------------------------
    variable declarations (extern)
-----------------------------------------------------------------------------*/
extern Tu8 scan_index_FM;
extern Tu8 scan_index_AM;
extern SOC_QUAL AMFMTuner_FMQual_parametrs;
extern SOC_QUAL AMFMTuner_AMQual_parametrs;
extern SOC_QUAL AMFMTuner_AMQual_parametrs;
extern SOC_QUAL AMFMTuner_AMFastScanQual_parametrs;
extern SOC_QUAL AMFMTuner_AMSeekQual_parameters;
extern SOC_QUAL AMFMTuner_AMFastSeekQual_parameters;

extern SOC_QUAL AMFMTuner_FMScanQual_parametrs;
extern SOC_QUAL AMFMTuner_FMFastScanQual_parametrs;
extern SOC_QUAL st_AMFMTuner_FMSeekQual_parameters;
extern SOC_QUAL st_AMFMTuner_FMFastSeekQual_parameters;
extern SOC_QUAL AMFMTuner_TunerStatusNot;
extern SOC_QUAL AMFMTuner_Tune_AFQuality;

extern EtalBcastQualityContainer st_amfm_tuner_ctrl_Quality_params;
extern Ts_AMFM_Tuner_Ctrl_CurrStationInfo st_CurrentStationInfo;
extern Ts_AMFM_Tuner_Ctrl_CurrStationInfo  ast_Scaninfo[AMFM_TUNER_CTRL_MAX_STATIONS];



/*-----------------------------------------------------------------------------
    Function declarations
-----------------------------------------------------------------------------*/


/*===========================================================================*/
/* 
 * Description				Message intended for amfm_app_hsm HSM will be processed via this function.
 *							This is message Handler function for AMFM_Tuner_Ctrl_Main_hsm .
 *
 * param[in]  
 *			 pst_msg		Pointer to the message to be handled 
 *
 * Return_Val				None
 *
 * pre[mandatory]			AMFM tuner control main hsm is initialized 
 *
 * post [mandatory]			Message will be sent to  AMFM_Tuner_Ctrl_Main_hsm and processed
 *
 */
/*===========================================================================*/
void   AMFM_TUNER_CTRL_MSG_HandleMsg(Ts_Sys_Msg* pst_msg);
/*===========================================================================*/
/* 
 * Description			       This function should be called first in order to initialise the AMFM tuner control Instance hsm  .
 *
 * param[in]  
 *		pst_me_amfm_tuner_ctrl Pointer to the HSM object of type Ts_AMFM_Tuner_Ctrl_Inst_hsm 
 *
 * Return_Val				    None	
 *
 * pre[mandatory]			    N/A
 * 
 * post [mandatory]			   Instance hsm is initialised and inactive state is reached 
 *
 */
/*===========================================================================*/
void AMFM_TUNER_CTRL_MSG_INST_HandleMsg(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_Sys_Msg* pst_msg);

/*===========================================================================*/
/* 
 * Description			    This function should be called first in order to initialise the AMFM tuner control main hsm  .
 *
 * param[in]                None
 *		 
 * Return_Val				None	
 *
 * pre[mandatory]			N/A
 *
 * post [mandatory]			HSM is initialised and inactive state is reached 
 *
 */
/*===========================================================================*/

void AMFM_Tuner_Ctrl_Component_Init(void);
void AMFM_Tuner_ctrl_UpdateParameterIntoMessage(Tchar* pu8_data, const void *vp_parameter, Tu16 u8_ParamLength, Tu16 *pu16_Datalength);
void AMFM_Tuner_ctrl_ExtractParameterFromMessage(void* vp_Parameter, const Tchar *pu8_DataSlot, Tu16 u16_ParamLength, Tu32 *pu32_index);
void AMFM_Tuner_Ctrl_MessageInit(Ts_Sys_Msg *pst_msg, Tu16 u16_DestID, Tu16 u16_MsgID);
Ts_Sys_Msg* SYS_MSG_HANDLE(Tu16 cid ,Tu16 msgid);
void AMFM_Tuner_Ctrl_Update_Rcvd_RDS_Data(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst, Ts_AMFM_Tuner_Ctrl_RDS_Data st_recv_data);
/*--------moved the functions from Msg.c-----*/

void AMFM_Tuner_Ctrl_GetjumpQualityCheck(EtalBcastQualityContainer* ST_Tuner_FMQual_Check, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);


void AMFM_Tuner_Ctrl_GetListenQuality(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);


void AMFM_Tuner_Ctrl_QualUpdate_AM_Scan(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst);
void AMFM_Tuner_Ctrl_QualUpdate_FM_Scan(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst);

void AMFM_Tuner_Ctrl_QualUpdate_AM_Learn(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst);
void AMFM_Tuner_Ctrl_QualUpdate_FM_Learn(EtalBcastQualityContainer* pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_amfm_tuner_ctrl_inst);

void AMFM_Tuner_Ctrl_SOC_Reply(Tu16 cid,Tu16 msg_id);

void AMFM_Tuner_Ctrl_startRDSDecode(Tu16 cid, Tu16 msg_id, Tu32 u32_freq);

void AMFM_Tuner_Ctrl_startreply(Tu16 cid,Tu16 msg_id);

void AMFM_Tuner_Ctrl_stop(Tu16 cid,Tu16 msg_id);

void AMFM_Tuner_Ctrl_stopreply(Tu16 u16_cid,Tu16 msg_id);

void AMFM_Tuner_Ctrl_Current_Station_Quality_Update(EtalBcastQualityContainer *st_amfm_tuner_ctrl_Quality_params, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);

void AMFM_Tuner_Ctrl_AM_QualParameters_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer* st_amfm_tuner_ctrl_Quality);

void AMFM_Tuner_Ctrl_FM_QualParameters_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer *st_amfm_tuner_ctrl_Quality_params);



/** functions newly added for ST_Micro--- same functions there for original projct has to been removed later*/
void AMFM_Tuner_Ctrl_GetFMQualityCheck_Tune(EtalBcastQualityContainer * pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm *pst_me_amfm_tuner_ctrl_inst);
void AMFM_Tuner_Ctrl_GetAMQualityCheck_Tune(EtalBcastQualityContainer *pBcastQualityContainer,Ts_AMFM_Tuner_Ctrl_Inst_hsm *pst_me_amfm_tuner_ctrl_inst);
void AMFM_Tuner_Ctrl_GetBG_FMQualityCheck_Tune(EtalBcastQualityContainer * pBcastQualityContainer, Ts_AMFM_Tuner_Ctrl_Inst_hsm *pst_me_amfm_tuner_ctrl_inst);
void AMFM_Tuner_Ctrl_FM_QualParams_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer *pBcastQuality_Container_FM);
void AMFM_Tuner_Ctrl_AM_QualParams_Conversion(Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst, EtalBcastQualityContainer *pBcastQuality_Container_AM);
void AMFM_Tuner_Ctrl_Current_Station_Qual_Update(EtalBcastQualityContainer *pBcastQuality_Container, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);
void AMFM_Tuner_Ctrl_QualUpdate_FG_FM_Seek(EtalBcastQualityContainer *pBcastQuality_Container_FM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);
void AMFM_Tuner_Ctrl_QualUpdate_FG_AM_Seek(EtalBcastQualityContainer *pBcastQuality_Container_AM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);
void AMFM_Tuner_Ctrl_QualUpdate_FG_FM_PISeek(EtalBcastQualityContainer *pBcastQuality_Container_FM, Ts_AMFM_Tuner_Ctrl_Inst_hsm* pst_me_inst);
#endif
/*=============================================================================
    end of file
=============================================================================*/