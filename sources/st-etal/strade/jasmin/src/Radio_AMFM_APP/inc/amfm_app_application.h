/*=============================================================================
    start of file
=============================================================================*/
/************************************************************************************************************/
/** \file amfm_app_application.h																			*
*  Copyright (c) 2016, Jasmin Infotech Private Limited.														*
*  All rights reserved. Reproduction in whole or part is prohibited											*
*  without the written permission of the copyright owner.													*
*																											*
*  Project              : ST_Radio_Middleware																				*
*  Organization			: Jasmin Infotech Pvt. Ltd.															*
*  Module				: AMFM Application															     	*
*  Description			: This header file consists of declaration of all the internal functions and		*
*						  Structures belonging to this component											*
*																											*
*************************************************************************************************************/

#ifndef AMFM_APP_APPLICATION_H
#define AMFM_APP_APPLICATION_H

/*-----------------------------------------------------------------------------
				File Inclusions
-----------------------------------------------------------------------------*/

#include "sys_task.h"
#include "amfm_app_types.h"
#include "amfm_app_inst_hsm.h"
#include "lib_bitmanip.h"


extern Ts_AMFM_Tuner_Ctrl_CurrStationInfo  ast_Scaninfo[AMFM_TUNER_CTRL_MAX_STATIONS];
extern Tu8 		scan_index_FM;
extern Tu8 		scan_index_AM;

typedef struct 
{
	Tu8 	u8_index;
	Tu32	u32_quality;
	Tu8		u8_padding[2];
}Ts_AMFM_App_temp_buffer;

/*-----------------------------------------------------------------------------
   Private Function Declaration
-----------------------------------------------------------------------------*/

/*===========================================================================*/
/*			void AMFM_App_Component_Init									 */
/*===========================================================================*/
void AMFM_App_Component_Init(void );


/*****************************************************************************************************/
/* 
 * Description				Message intended for amfm_app_hsm HSM will be processed via this function.
 *							This is message Handler function for amfm_app_hsm  HSM .
 *
 * param[in]  
 *			 pst_msg		Pointer to the message to be handled 
 *
 * Return_Val				None
 *
 * pre[mandatory]			amfm_app_hsm HSM is initialized 
 *
 * post [mandatory]			Message will be sent to amfm_app_hsm HSM and processed
 *
 */
/*****************************************************************************************************/
void AMFM_APP_HSM_MessageHandler(Ts_Sys_Msg *pst_msg);

/*****************************************************************************************************/
/* 
 * Description						Updates default frequency ranges for AM and FM bands into structure st_MarketInfo of type Ts_AMFM_App_MarketInfo
 *									as per market type	
 *
 * param[in]  
 *	 		 	e_Market			enum indicates market type
 *			 	pst_MarketInfo		Pointer to the market info structure to be updated
 *
 * Return_Val	 					None
 *			 				
 * pre[mandatory]					HSM has to be initialized 
 *
 * post [mandatory]					Market Info structure is updated with default frequency ranges  
 *
 */
/*****************************************************************************************************/
void AMFM_APP_SetMarketFrequency(Te_AMFM_App_Market e_Market,Ts_AMFM_App_MarketInfo *pst_MarketInfo);

/*****************************************************************************************************/
/* 
 * Description						This function checks whether that frequency to be tuned is valid or not.
 *									Before tuning to particular frequency, it is mandatory to verify it.  
 *
 * param[in]  
 *				u32_Frequency		frequency in KHz 
 *	 		 	pe_mode				pointer to the enum indicates mode type
 *				e_Market			enum indicates market type
 *			 	pst_MarketInfo		Pointer to the market info structure to be updated
 *
 * Return_Val	 					
 *				Tbool				Returns TRUE(1) if the frequency is valid otherwise returns FALSE(0) 
 *			 				
 * pre[mandatory]					Default frequency ranges of AM and FM band has to be updated into structure st_MarketInfo of type Ts_AMFM_App_MarketInfo.  
 *
 * post [mandatory]					If frequency is valid,then corresponding Tuner ctrl API is called for tuning.If it is invalid,throws error to the radio manager application.  
 *
 */
/*****************************************************************************************************/
Tbool AMFM_APP_VerifyFrequency(Tu32 u32_Frequency,Te_AMFM_App_mode *pe_mode,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo);

Ts_AMFM_App_FreqInfo	AMFM_APP_GetCurrentModeFrequencyInfo(Te_AMFM_App_mode e_CurrentMode,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo);
Tu32 AMFM_APP_GetNextTuneDownFrequency(Tu32 u32_CurrentFrequency,Te_AMFM_App_mode *pe_CurrentMode,Tu32 u32_No_of_Steps,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo);
Tu32 AMFM_APP_GetNextTuneUpFrequency(Tu32 u32_CurrentFrequency,Te_AMFM_App_mode *pe_CurrentMode,Tu32 u32_No_of_Steps,Te_AMFM_App_Market e_MarketType,Ts_AMFM_App_MarketInfo *pst_MarketInfo);
/*****************************************************************************************************/
/* 
 * Description							This function sorts the AM station list by frequency in ascending order 
 *
 * param[in]  
 *				pst_am_station_list		pointer to the array of AM stations 
 *	 		 	u8_STL_Size				Number of available stations in the list					
 *
 * Return_Val							None			 					
 *			
 * pre[mandatory]						Station list should be present. i.e  has to be read from the shared memory present between tuner ctrl and AMFM application.
 *
 * post [mandatory]						Station list is sorted 
 *
 */
/*****************************************************************************************************/
void AMFM_App_application_SortAMStationList(Ts_AMFM_App_AMStationInfo *pst_am_station_list,Tu8 u8_STL_Size);

/*****************************************************************************************************/
/* 
 * Description							This function generates AM station list by reading from the shared memory present 
 * 										between tuner ctrl and AMFM application.After completion of reading , station list is sorted by invoking 
 *										AMFM_App_application_SortAMStationList()  function.  
 *
 * param[in]  				 
 *				pst_am_station_list		pointer to the AM station list structure 	
 *
 * Return_Val	 						void
 *
 * pre[mandatory]						Station list should be present in the shared memory present between tuner ctrl and AMFM application.  
 *
 * post [mandatory]						Station is generated. 
 *
 */
/*****************************************************************************************************/
void AMFM_App_application_GenerateAMStationList(Ts_AMFM_App_AM_STL *pst_am_station_list);

/*****************************************************************************************************/
/* 
 * Description							This function generates FM station list by reading from the shared memory present 
 * 										between tuner ctrl and AMFM application.After completion of reading,station list is sorted by invoking 
 *										AMFM_App_application_SortFMStationList()  function.
 *
 * param[in]  				 
 *				pst_fm_station_list		pointer to the FM station list structure 
 *				u8_LastRDSindex			index value of last RDS station in the list  
 *
 * Return_Val	 						void
 *
 * pre[mandatory]						Station list should be read    
 *
 * post [mandatory]						RDS stations in the list is sorted by program station name (PSN) in ascending order.
 *
 */
/*****************************************************************************************************/
void AMFM_App_SortingAscending(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_LastRDSindex);

/*****************************************************************************************************/
/* 
 * Description							This function sorts the FM station list by program station name (PSN) lexicographically for RDS stations. For non RDS stations, sorts by frequency in ascending order.
 *
 * param[in]  
 *				pst_fm_station_list		pointer to the array of FM stations 
 *	 		 	u8_STL_Size				Number of available stations in the list					
 *
 * Return_Val							None			 					
 *			
 * pre[mandatory]						Station list should be present. i.e  has to be read from the shared memory present between tuner ctrl and AMFM application.
 *
 * post [mandatory]						Station list is sorted 
 *
 */
/*****************************************************************************************************/
Tu8 	AMFM_App_application_SortFMStationList(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_STL_Size);

void AMFM_App_Remove_Non_RDS_Stations(Ts_AMFM_App_FM_STL *pst_fm_station_list,Tu8 u8_Total_NonRDS_Station);

Tu32 AMFM_App_GetFreqQualityfrom_TunerCtrl_STL(Tu32 u32_Freq);

Tu32 AMFM_App_findMinQuality(Ts_AMFM_App_temp_buffer	*pst_temp_buffer);

void AMFM_App_Remove_FM_stationfromSTL(Ts_AMFM_App_FMStationInfo * pst_fm_station_list,Tu8 u8_STL_Size,Tu8 u8_remove_Station_Index);

Ts8 AMFM_App_CheckPIpresentInFMSTL(Tu8 u8_StationIndexLimit,Tu16 u16_NewStationPI,Tu8	*pu8_PSN ,Ts_AMFM_App_FM_STL *pst_fm_station_list);

/*****************************************************************************************************/
/* 
 * Description							This function generates FM station list by reading from the shared memory present 
 * 										between tuner ctrl and AMFM application.After completion of reading,station list is sorted by invoking 
 *										AMFM_App_application_SortFMStationList()  function.
 *
 * param[in]  				 
 *				pst_fm_station_list		pointer to the FM station list structure 	
 *
 * Return_Val	 						void
 *
 * pre[mandatory]						Station list should be present in the shared memory present between tuner ctrl and AMFM application.  
 *
 * post [mandatory]						Station is read and sorted. 
 *
 */
/*****************************************************************************************************/
Tu8 AMFM_App_application_GenerateFMStationList(Ts_AMFM_App_FM_STL *pst_fm_station_list,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem,Te_AMFM_App_Market	e_MarketType );

void AMFM_App_application_Remove_SpacefromStationList(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_STL_Size);
Tbool AMFM_App_Remove_SamePIfromFMSTL(Tu16 u16_NewStation_PI,Ts_AMFM_App_FM_STL *pst_fm_station_list);

Ts8 AMFM_App_CheckFreqPresentAMSTL(Tu32 u32_Freq,Ts_AMFM_App_AM_STL *pst_am_station_list);
	
Ts8 AMFM_App_CheckFreqPresentFMSTL(Tu32 u32_Freq,Ts_AMFM_App_FM_STL *pst_fm_station_list);
	
Tbool AMFM_App_ValidatePIcode(Ts_AMFM_App_LinkingParam  *pst_DAB_FM_LinkingParam,Tu16 u16_PI);

void AMFM_APP_Clear_LinkingParam(Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam) ;

void	AMFM_App_GenerateHardLinkFreq( Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Tu16 u16_STL_Size,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);

Tbool AMFM_App_FindNextValidFreq(Ts_AMFM_App_HardLink_Freq_Quality_List *pst_HL_Freq_Quality_List, Tu8 u8_TotalCount, Tu8 *pu8_index);

void	AMFM_App_Sort_HardLink_Freq_List(Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam);

void	AMFM_App_Append_AF_Into_List(Tu8 u8_AFCount,Tu32 *pu32_AFList,Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);

void	AMFM_App_Append_AF_Into_HL_Freq_List(Tu8 u8_AFCount,Tu32 *pu32_AFList,Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam);

Te_AF_Freq_Availabilty_Check AMFM_App_Check_freq_Repeat(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AFfreq);

Ts32 AMFM_App_String_comparison(Tu8 *src,Tu8 *dst,Tu8 size);

Tu8	AMFM_APP_String_length(Tu8 *pu8_PtrTostring,Tu8	u8_StringLen);

Tu8	AMFM_APP_Total_Non_RDS_Stations(Ts_AMFM_App_FMStationInfo *pst_fm_station_list,Tu8 u8_STL_Size);

#if 0
Ts8	AMFM_App_Check_PI_in_STL(Ts_AMFM_App_FM_STL *pst_fm_station_list,Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam);
#endif


Te_AF_Freq_Availabilty_Check AMFM_App_Freq_Existance_Check(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Tu32 u32_AFfreq);

void AMFM_App_frequency_Append(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerStatusInfo);


Ts8	AMFM_App_Check_PI_in_STL(Ts_AMFM_App_FM_STL *pst_fm_station_list,Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam);
/**/
void AMFM_APP_Update_EON_List(Ts_AMFM_App_EON_List *pst_EON_List,Ts_AMFM_TunerCtrl_EON_Info st_AMFM_TunerCtrl_Eon_Info);
Tbool AMFM_APP_Check_PI_Availability(Ts_AMFM_App_EON_List *pst_EON_List, Tu16 u16_pi);


void AMFM_APP_Adding_PIFrequency_From_LearnMemory(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Tu16 u16_pi,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info);
//void AMFM_APP_Adding_PIFrequency_From_FM_STL(Ts_AMFM_App_EON_List *pst_EON_List,Tu16 u16_pi, Ts_AMFM_App_FM_STL *pst_fm_station_list);

Tu8 AMFM_APP_AvailablePI_Index(Ts_AMFM_App_EON_List *pst_EON_List,Tu16 u16_pi);

Ts8 AMFM_App_Check_SID_in_STL(Ts_AMFM_App_FM_STL *pst_fm_station_list,Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Tu16 u16_Implicit_sid);

void AMFM_APP_Start_Seek(Tu32 u32_CurrentFrequency,Ts_AMFM_App_MarketInfo *pst_MarketInfo,Tu16 u16_PI);

void AMFM_APP_New_EONList_AF_update(Ts_AMFM_App_EON_List *pst_EON_List,Tu32 u32_CurrentFrequency,Ts_AMFM_App_MarketInfo *pst_MarketInfo);


/* New strategy */
Te_AF_Freq_Availabilty_Check AMFM_App_Freq_Existance_Check_New(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AFfreq);

void AMFM_App_frequency_Append_New(Tu32 au32_AFeqList[],Tu8 u8_NumAFeqList,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);

void AMFM_App_Sort_AF_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);


Tu8 AMFM_APP_New_AF_Update(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void AMFM_App_AFTune_Restart(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void AMFM_App_Flag_Reset(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

Tbool AMFM_App_Best_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void AMFM_APP_Best_Freq_AF_check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void AMFM_App_Current_Qual_Avg_Computation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void  AMFM_App_AF_Qual_Avg_computation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_qual_incr_alpha,	Tu32 u32_AF_qual_decr_alpha);

void AMFM_App_PI_Status_Update(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void AMFM_APP_BG_Generate_AF_list_from_LM(Tu16 u16_PI,Ts_AMFM_App_FreqInfo	*pst_FMband_FreqInfo,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem *pst_AF_Learn_mem);
void AMFM_App_AF_Append_From_learn_Memory(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);

Tu16 AMFM_App_Learn_memory_PI_Read( Tu32 u32_freq,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);

//void AMFM_App_Learn_Memory_updation(Tu16 u32_freq,Tu16 u16_PI,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);
Tbool AMFM_App_Learn_Memory_updation(Tu32 u32_freq,Tu16 u16_PI,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);

#ifdef CALIBRATION_TOOL
	void AMFM_App_Calib_Tune_Qual_Avg_Computation(void);
	
	Tu8 AMFM_App_Calib_get_index(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info,Tu8 u8_AFListIndex);
	void AMFM_App_Calib_Remove_AF_From_List(Tu8 *pu8_Tot_No_AF);
	Tu8 AMFM_App_Calib_get_AF_index(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info,Tu8 u8_NumAFList,Tu8 u8_index);
#endif

void AMFM_App_Eon_Station_Qual_Avg_Computation(Ts_AMFM_App_EON_List *pst_EON_List);
void AMFM_App_Update_LM_Freq(Ts_AMFM_App_FreqInfo *pst_FMband_FreqInfo,Ts_AMFM_App_AF_learn_mem	*pst_AF_Learn_mem);
Tu8 AMFM_App_PSN_RT_Copy(Ts_AMFM_App_StationInfo *pst_current_station,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo);
void AMFM_App_Read_TA_TP_info(Ts_AMFM_App_StationInfo *pst_current_station, Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo);
void AMFM_App_PTY_Copy(Ts_AMFM_App_StationInfo *pst_current_station,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo);
void AMFM_App_Append_HL_Freq_into_List(Ts_AMFM_App_LinkingParam *pst_DAB_FM_LinkingParam,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

void AMFM_App_ReadQuality(Ts_AMFM_App_StationInfo	*pst_StationInfo,Ts_AMFM_Tuner_Ctrl_CurrStationInfo  *pst_TunerctrlCurrentStationInfo);


/*regional additions*/
Te_AMFM_App_Regional_PI_Check AMFM_App_Regional_Pi_Validation(Tu16 u16_curr_station_PI, Tu16 u16_reff_station_PI);
Te_AF_Freq_Availabilty_Check AMFM_App_Freq_Duplication_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AFfreq);
void AMFM_App_Same_PI_List_Freq_Append(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_Freq,Tu16 u16_PI,Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS);
void AMFM_App_REG_PI_List_Freq_Append(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_Freq,Tu16 u16_PI,Te_AMFM_App_AF_PI_STATUS e_AF_PI_STATUS);
Tbool AMFM_App_AF_List_frequency_Append(Tu32 au32_AFeqList[],Tu8 u8_Num_Same_PI_AFeqList,Tu8 u8_Num_reg_PI_AFeqList,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst);
Tbool AMFM_App_AF_List_Append_From_learn_Memory(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Ts_AMFM_App_AF_learn_mem *pst_AF_Learn_mem);


Tbool AMFM_APP_AF_Update(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
void AMFM_App_Sort_SAME_PI_AF_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
void AMFM_App_Sort_REG_PI_AF_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
Tbool AMFM_App_SAME_PI_Best_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
void AMFM_APP_Curr_Best_Freq_AF_check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
void AMFM_App_Remove_AF_From_List(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu8 *pu8_Tot_No_AF);
void AMFM_APP_Clear_AF_Qual_Parameters(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
void  AMFM_App_AF_Quality_Avg_computation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu32 u32_AF_qual_incr_alpha,Tu32 u32_AF_qual_decr_alpha);

void AMFM_App_AFupdate_Restart(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
Tbool AMFM_App_SAME_PI_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
Tbool AMFM_App_REG_PI_Best_AF_Avaliability_Check(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

Tbool AMFM_APP_ENG_AF_Existance_Check(Tu32 u32_AF_Freq,Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info);
Tu8 AMFM_APP_Get_ENG_AF_Index(Tu32 u32_AF_Freq,Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info);
Tbool AMFM_APP_ENG_AF_Updation_Check(Ts_AMFM_App_ENG_AFList_Info *pst_AMFM_App_ENG_AFList_Info, Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info);

void  AMFM_APP_Curr_Station_Related_Clearing(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info);
void  AMFM_App_ReadCTinfo(Ts_AMFM_AppRDS_CT_Info *pst_RDS_CTinfo,Ts_AMFM_Tuner_Ctrl_CurrStationInfo *pst_TunerctrlCurrentStationInfo);
Te_RADIO_ReplyStatus  AMFM_App_Compute_CT_Info(Ts_AMFM_AppRDS_CT_Info *pst_RDS_CTinfo,Ts_AMFM_App_CT_Info *pst_CT_Info);
void AMFM_App_StartUp_Initialisation(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst);

void AMFM_App_RadioDebugLogPrint(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info,Te_AMFM_APP_DebuggingStatus e_DebuggingStatus);

/* Functions used for NEG Flag strategy in AF list */
Tbool AMFM_App_NEG_flag_status(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info);
void AMFM_App_AF_StatusCountIncrementation(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info);
void AMFM_App_AF_NEG_StatusReset(Ts_AMFM_App_AFList_Info * pst_AMFM_App_AFList_Info);

/* Functions used for NEG Flag strategy in AF list	 in DAB<=>FM linking */
Tbool AMFM_App_DAB2FM_Check_NEG_flag_AFList(	Tu8 	u8_NumAFList,Ts_AMFM_App_AFStation_Info	*pst_AFStation_Info);
void AMFM_App_AF_DAB2FM_Increase_NEGStatusCount(Tu8 	u8_NumAFList,Ts_AMFM_App_AFStation_Info	*pst_AFStation_Info);
void AMFM_App_AF_DAB2FM_Reset_NEG_Status(Tu8 	u8_NumAFList,Ts_AMFM_App_AFStation_Info	*pst_AFStation_Info);

void AMFM_App_clear_PreviousEON_Qualities(Ts_AMFM_App_EON_List *pst_EON_List);

Tu8 AMFM_APP_EON_AF_Index(Ts_AMFM_App_EON_List *pst_EON_List,Tu32 u32_CurrentFrequency);
Tu8 AMFM_APP_Check_EON_AF_Availability(Ts_AMFM_App_EON_List *pst_EON_List,Tu32 u32_CurrentFrequency);


void AMFM_App_DeltaComputation(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);
Tu8 AMFM_App_PI_Updation(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info,Tu16 u16_curr_station_PI);

void AMFM_App_AFListClear(Ts_AMFM_App_inst_hsm *pst_me_amfm_app_inst,Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

Tu8 AMFM_APP_AFStrategy_AFUpdate(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

Tu8 AMFM_App_BestAFAvailabilityCheck(Ts_AMFM_App_AFList_Info *pst_AMFM_App_AFList_Info);

#endif /*  End of AMFM_APP_APPLICATION_H */
/*=============================================================================
    end of file
=============================================================================*/