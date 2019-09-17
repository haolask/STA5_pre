//!
//!  \file      rds_strategy.c
//!  \brief     <i><b> RDS strategy source file </b></i>
//!  \details   RDS strategy related management
//!  \author    SZ TUNER APP TEAM
//!  \author    SZ TUNER APP TEAM
//!  \version   1.0
//!  \date      2016.05.28
//!  \bug       Unknown
//!  \warning   None
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY

#include "etalinternal.h"
#include "dabmw_import.h"
#include "rds_data.h"
#include "rds_af_data.h"
#include "rds_strategy.h"
#include "etaltml_api.h"


//From Rds_data.c, used to get TATP data
typedef struct
{
	union
	{
		struct
		{
			tU8 taValue:    1;
			tU8 tpValue:    1;
			tU8 padding0: 6;
		} fields;
		tU8 value;
	} data;

	union
	{
		struct
		{
			tU8 taValue:    1;
			tU8 tpValue:    1;
			tU8 padding0: 6;
		} fields;
		tU8 value;
	} lastData;

	tU8 confidenceThr;
	DABMW_storageStatusEnumTy newValue;
} DABMW_taTpDataTy;

/* Internal definations	*/
#define INVALID_AF					0xFF
#define FM_NORMAL_START_FREQ   	87500
#define FM_NORMAL_STOP_FREQ   		108000
#define FREQ_IN_NORMAL_FM(x)           ((x) >= FM_NORMAL_START_FREQ && (x) <= FM_NORMAL_STOP_FREQ)
#define FREQ_AF_TO_KHZ(x)			(((tU32)(x)) * 100 + 87500)
#define FREQ_KHZ_TO_AF(x)			(tU8)(((x) - 87500) / 100)
#define SOFTON						1
#define SOFTOFF						0

#define ETALTML_RDS_STRATEGY_NOTIFY_WAITTIME		3	/* 30ms  Notify wait time DURNING_AF_OPERATION*/
#define ETALTML_RDS_STRATEGY_TASEEK_COMP_WAITTIME 	100
#define ETALTML_RDS_STRATEGY_PISEEK_COMP_WAITTIME 	70
#define ETALTML_RDS_STRATEGY_FM_SEEK_STEP  			100


/***********************************************************************************
 * Local variables
 ***********************************************************************************/
OSAL_tSemHandle  ETALTML_RDS_Strategy_Sem[RDS_STATEGY_LOCK_MAX_NUMBER];
RDS_TimerStruct    RDS_Timer[DABMW_RDS_SOURCE_MAX_NUM];
RDSMgrStruct  	 RDSMgr[DABMW_RDS_SOURCE_MAX_NUM];
static tBool 		 RDSMgrLocksInit = FALSE;

static tVoid ETALTML_RDS_10msTimer_Act(tSInt slot);
static tVoid ETALTML_RDS_100msTimer_Act(tSInt slot);
static tVoid ETALTML_RDS_1sTimer_Act(tSInt slot);
static tVoid ETALTML_RDSTimer_Set_Internal(tSInt slot, RDS_TimerType timerType, tU32 tCount);
static ETAL_STATUS ETALTML_RDS_DataNotification(ETAL_HANDLE hReceiver, tSInt slot, RDS_TASwitchSystemMode * pTASwitchMode, tBool AFSwitched, tU8 afFreq, tBool TASeekStarted, tBool PISeekStarted);
static ETAL_STATUS ETALTML_RDS_Mute(ETAL_HANDLE hReceiver, tBool bMute);
static ETAL_STATUS ETALTML_RDS_AFCheckInit(ETAL_HANDLE hReceiver);
static ETAL_STATUS ETALTML_RDS_SetFreq(ETAL_HANDLE hReceiver, tU8 setfreq);
static ETAL_STATUS ETALTML_RDS_ClearTAFlags(tSInt slot);
static ETAL_STATUS ETALTML_RDSAF_AFChange(tSInt slot, tU8 num1, tU8 num2);
static ETAL_STATUS ETALTML_RDSAF_AFSort(tSInt slot, tU8 bg);
static ETAL_STATUS ETALTML_RDSAF_ReadTunerCFQual(ETAL_HANDLE hReceiver, RDS_TunerQualInfoTy *CFQual);
static ETAL_STATUS ETALTML_RDSAF_ReadTunerAFQual(ETAL_HANDLE hReceiver, RDS_TunerQualInfoTy *AFQual);
static ETAL_STATUS ETALTML_RDSAF_BreakAFSearch(ETAL_HANDLE hReceiver);
static ETAL_STATUS ETALTML_RDSAF_SetAFSearch(tSInt slot);
static ETAL_STATUS ETALTML_RDSAF_AFCheckBack(ETAL_HANDLE hReceiver, tSInt slot, tBool bFlag);
static ETAL_STATUS ETALTML_RDSAF_AFSearchNext(ETAL_HANDLE hReceiver, tSInt slot, tBool bAFEnd);
static ETAL_STATUS ETALTML_RDSTA_SwitchToTa(ETAL_HANDLE hReceiver, tSInt slot);
static ETAL_STATUS ETALTML_RDSTA_TaSwitchBack(ETAL_HANDLE hReceiver, tSInt slot);
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
static ETAL_STATUS ETALTML_RDSEON_ClearEONTAFlags(tSInt slot);
static ETAL_STATUS ETALTML_RDSEON_CheckEON(ETAL_HANDLE hReceiver, tSInt slot);
static ETAL_STATUS ETALTML_RDSEON_Backup(ETAL_HANDLE hReceiver, tSInt slot);
static ETAL_STATUS ETALTML_RDSEON_Restore(ETAL_HANDLE hReceiver, tSInt slot);
static ETAL_STATUS ETALTML_RDSTA_SwitchToEONTa(ETAL_HANDLE hReceiver, tSInt slot);
static ETAL_STATUS ETALTML_RDSTA_EONTaSwitchBack(ETAL_HANDLE hReceiver, tSInt slot);
static ETAL_STATUS ETALTML_RDSEON_CopyEON(tSInt slot);
#endif
static tBool ETALTML_RDS_Strategy_RadioIsBusy(ETAL_HANDLE hReceiver);
static tVoid ETALTML_RDSAF_MainProc(ETAL_HANDLE hReceiver, tSInt slot);


/**
* ETALTML_RDS_10msTimer_Act
* @desc: This is 10ms timer used for RDS function running
* @param: tSInt slot
* @return: void
* @note:
*/
tVoid ETALTML_RDS_10msTimer_Act(tSInt slot)
{
	//No need RDSBufferReadTimer timer, which has been set in RDS data decoding part.
	if(RDSMgr[slot].AFInfo.AFTimer > 0)  				RDSMgr[slot].AFInfo.AFTimer--;
	if(RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer > 0)	RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer--;
	if(RDSMgr[slot].AFInfo.AFCheckWaitTimer > 0)		RDSMgr[slot].AFInfo.AFCheckWaitTimer--;
}


/**
* ETALTML_RDS_100msTimer_Act
* @desc: This is 100ms timer used for RDS function running
* @param: tSInt slot
* @return: void 
* @note:
*/
tVoid ETALTML_RDS_100msTimer_Act(tSInt slot)
{
	if(RDSMgr[slot].AFInfo.AFDisable100msTimer > 0) 		RDSMgr[slot].AFInfo.AFDisable100msTimer--;
	if(RDSMgr[slot].AFInfo.AFSearchWaitTimer > 0)		RDSMgr[slot].AFInfo.AFSearchWaitTimer--;
	if(RDSMgr[slot].AFInfo.RDSStationDelayTimer > 0)		RDSMgr[slot].AFInfo.RDSStationDelayTimer--;
	if(RDSMgr[slot].PIInfo.PISeekTimer > 1)				RDSMgr[slot].PIInfo.PISeekTimer--;
	
	if(RDSMgr[slot].FuncTimer.AutoTASeekTimer > 1)		RDSMgr[slot].FuncTimer.AutoTASeekTimer--;
	if(RDSMgr[slot].FuncTimer.PTYSelectTimer > 1)		RDSMgr[slot].FuncTimer.PTYSelectTimer--;
	if(RDSMgr[slot].FuncTimer.InTADelayTimer > 0)		RDSMgr[slot].FuncTimer.InTADelayTimer--;
	if(RDSMgr[slot].FuncTimer.EnterTADelayTimer > 1)		RDSMgr[slot].FuncTimer.EnterTADelayTimer--;
	if(RDSMgr[slot].FuncTimer.EnterEONTADelayTimer > 0)	RDSMgr[slot].FuncTimer.EnterEONTADelayTimer--;
	if(RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer > 0) 	RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer--;
	
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	if(RDSMgr[slot].EONInfo.EONAFTimer > 0)			RDSMgr[slot].EONInfo.EONAFTimer--;
	if(!RDSMgr[slot].EONInfo.F_InEONTAReal)
	{
		if(RDSMgr[slot].EONInfo.EONExitTimer > 1)		RDSMgr[slot].EONInfo.EONExitTimer--;
	}
#endif
}


/**
* ETALTML_RDS_1sTimer_Act
* @desc: This is 1 second routine used for RDS 
* @param: slot
* @return: void
* @note:
*/
tVoid ETALTML_RDS_1sTimer_Act(tSInt slot)
{
	tInt i;
	
	for(i = 1; i < MAX_AF_NUMBER ;i++)
	{
		if(RDSMgr[slot].AFInfo.AFDisable[i] > 0)			RDSMgr[slot].AFInfo.AFDisable[i]--;
	}
		
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	for(i = 0; i < EON_BUFF_SIZE; i++)
	{
		if(RDSMgr[slot].EONInfo.EONSave_EONCheckDisable[i] > 0) 	RDSMgr[slot].EONInfo.EONSave_EONCheckDisable[i]--;
	}
#endif

	if(RDSMgr[slot].AFInfo.AFCheckStartTimer > 0)		RDSMgr[slot].AFInfo.AFCheckStartTimer--;
	
	RDSMgr[slot].Qual.MaxLevel = RDSMgr[slot].Qual.Level;
	RDSMgr[slot].Qual.Level = 0;		
}


/**
 *
 * ETALTML_RDS_Strategy_GetLock
* @desc: Semaphore lock function for rds strategy 
* @param: ETALTML_RDS_StrategyLockTy lck
* @return: void
* @note:
*/
static tVoid ETALTML_RDS_Strategy_GetLock(ETALTML_RDS_StrategyLockTy lck)
{
	OSAL_s32SemaphoreWait(ETALTML_RDS_Strategy_Sem[lck], OSAL_C_TIMEOUT_FOREVER);
}


/**
 *
 * ETALTML_RDS_Strategy_ReleaseLock
* @desc: Semaphore lock function for rds strategy 
* @param: ETALTML_RDS_StrategyLockTy lck
* @return: void
* @note:
*/
static tVoid ETALTML_RDS_Strategy_ReleaseLock(ETALTML_RDS_StrategyLockTy lck)
{
	OSAL_s32SemaphorePost(ETALTML_RDS_Strategy_Sem[lck]);
}


/*!
 * ETALTML_RDS_Strategy_InitLock
 * \brief		Initializes the ETALTML RDS strategy data internal locks
 * \details		Locks are used in ETALTML RDS Strategy to avoid concurrent access to shared resources which may result in
 *              corruption of the resource. Locks are implemented though OSAL semaphores.
 * \details		The function is called during RDS Strategy thread initiation.
 * \return		OSAL_ERROR - error during the creation of one of the semaphores;
 * 				this is normally a fatal error, ETAL cannot continue
 * \return		OSAL_OK    - no error
 */
tSInt ETALTML_RDS_Strategy_InitLock(tVoid)
{
	tChar Sem_name[16];
	tInt i;

	for (i = 0; i < RDS_STATEGY_LOCK_MAX_NUMBER; i++)
	{
		if (OSAL_s32NPrintFormat(Sem_name, 16, "RDS_AF_sem%.2u", i) < 0)
		{
			return OSAL_ERROR;
		}
		
		if (OSAL_s32SemaphoreCreate(Sem_name, &ETALTML_RDS_Strategy_Sem[i], 1) == OSAL_ERROR)
		{
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}


/*!
 * ETALTML_RDS_Strategy_DeinitLock
 * \brief		Destroys all the rds strategy locks
 * \remark		Normally called only at RDS strategy close down.
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - not all semaphores destroyed, this is a non-fatal error
 */
tSInt ETALTML_RDS_Strategy_DeinitLock(tVoid)
{
	tSInt ret = OSAL_OK;
	tSInt i;

	for (i = 0; i < RDS_STATEGY_LOCK_MAX_NUMBER; i++)
	{
		if (OSAL_s32SemaphoreClose(ETALTML_RDS_Strategy_Sem[i]) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
		}
	}
	return ret;
}


/**
* ETALTML_RDS_Strategy_RDSStation_Indicator
* @desc: used to do indicated rds station indictor for rds decode rountine
* @param : slot
* @return None
* @note: For RDS station Indicator, when received a group, call this function.
*/
tVoid ETALTML_RDS_Strategy_RDSStation_Indicator(tSInt slot)
{
	if(RDSMgr[slot].Qual.Level < 20) RDSMgr[slot].Qual.Level++;
}


/**
* ETALTML_RDSTimer_Set_Internal
* @desc: set the related timer timer counts, internal usage.
* @param : slot, timerType, tCount
* @return None
* @note: 
*/
tVoid ETALTML_RDSTimer_Set_Internal(tSInt slot, RDS_TimerType timerType, tU32 tCount)
{
	ETALTML_RDS_Strategy_GetLock(RDS_LOCK_TIMER);
	switch(timerType)
	{
		case RDS_TIMER_10MS:	
			RDS_Timer[slot].Timer10ms = tCount;
			break;

		case RDS_TIMER_100MS:
			RDS_Timer[slot].Timer100ms = tCount;
			break;

		case RDS_TIMER_FUNC_AUTO_TA_SEEK:
			RDSMgr[slot].FuncTimer.AutoTASeekTimer = tCount;
			break;

		case RDS_TIMER_FUNC_PTY_SELECT:
			RDSMgr[slot].FuncTimer.PTYSelectTimer = tCount;
			break;

		case RDS_TIMER_FUNC_IN_TA_DELAY:
			RDSMgr[slot].FuncTimer.InTADelayTimer = tCount;
			break;

		case RDS_TIMER_FUNC_ENTER_TA_DELAY:
			RDSMgr[slot].FuncTimer.EnterTADelayTimer = tCount;
			break;

		case RDS_TIMER_FUNC_ENTER_EONTA_DELAY:
			RDSMgr[slot].FuncTimer.EnterEONTADelayTimer = tCount;
			break;

		case RDS_TIMER_FUNC_TA_WAIT_AF_CHECK:
			RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer = tCount;
			break;

		case RDS_TIMER_AFINFO_AF:
			RDSMgr[slot].AFInfo.AFTimer = tCount;
			break;

		case RDS_TIMER_AFINFO_AF_SMETER_CHECKF_REQ:
			RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer = tCount;
			break;

		case RDS_TIMER_AFINFO_AF_CHECK:
			RDSMgr[slot].AFInfo.AFCheckWaitTimer = tCount;
			break;

		case RDS_TIMER_AFINFO_AF_DISABLE_100MS:
			RDSMgr[slot].AFInfo.AFDisable100msTimer = tCount;
			break;

		case RDS_TIMER_AFINFO_AF_SEARCH:
			RDSMgr[slot].AFInfo.AFSearchWaitTimer = tCount;
			break;

		case RDS_TIMER_AFINFO_RDSSTATION_DELAY:
			RDSMgr[slot].AFInfo.RDSStationDelayTimer = tCount;
			break;

		case RDS_TIMER_PIINFO_PISEEK:
			RDSMgr[slot].PIInfo.PISeekTimer = tCount;
			break;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
		case RDS_TIMER_EONINFO_EONAF:
			RDSMgr[slot].EONInfo.EONAFTimer = tCount;
			break;

		case RDS_TIMER_EONINFO_EONEXIT:
			RDSMgr[slot].EONInfo.EONExitTimer = tCount;
			break;
#endif

		default:	
			break;
			
	}
	ETALTML_RDS_Strategy_ReleaseLock(RDS_LOCK_TIMER);
}


/**
* ETALTML_RDS_Strategy_RDSTimer_Set
* @desc: set the related timer timer counts
* @param : hReceiver, timerType, tCount
* @return None
* @note: 
*/
tVoid ETALTML_RDS_Strategy_RDSTimer_Set(ETAL_HANDLE hReceiver, RDS_TimerType timerType, tU32 tCount)
{
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (slot < 0 ) return;

	ETALTML_RDSTimer_Set_Internal(slot, timerType, tCount);

	//printf("ETALTML_RDS_Strategy_RDSTimer_Set receiver = %d, slot %d, type = %d, time = %d\n", hReceiver, slot, timerType, tCount);
}


/**
* ETALTML_RDS_Strategy_RDSTimer_Get
* @desc: get the related timer left timer counts
* @param : hReceiver,   timerType
* @return : the related timer's counter value
* @note: 
*/
tU32 ETALTML_RDS_Strategy_RDSTimer_Get(ETAL_HANDLE hReceiver, RDS_TimerType timerType)
{
	tU32 timerValue;
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (slot < 0 ) return 0xFF;

	//printf("ETALTML_RDS_Strategy_RDSTimer_Get receiver = %d,  type = %d\n", hReceiver,  timerType);

	ETALTML_RDS_Strategy_GetLock(RDS_LOCK_TIMER);
	switch(timerType)
	{
		case RDS_TIMER_10MS:	
			timerValue = RDS_Timer[slot].Timer10ms;
			break;

		case RDS_TIMER_100MS:
			timerValue = RDS_Timer[slot].Timer100ms ;
			break;

		case RDS_TIMER_FUNC_AUTO_TA_SEEK:
			timerValue = RDSMgr[slot].FuncTimer.AutoTASeekTimer ;
			break;

		case RDS_TIMER_FUNC_PTY_SELECT:
			timerValue = RDSMgr[slot].FuncTimer.PTYSelectTimer ;
			break;

		case RDS_TIMER_FUNC_IN_TA_DELAY:
			timerValue = RDSMgr[slot].FuncTimer.InTADelayTimer ;
			break;

		case RDS_TIMER_FUNC_ENTER_TA_DELAY:
			timerValue = RDSMgr[slot].FuncTimer.EnterTADelayTimer ;
			break;

		case RDS_TIMER_FUNC_ENTER_EONTA_DELAY:
			timerValue = RDSMgr[slot].FuncTimer.EnterEONTADelayTimer ;
			break;

		case RDS_TIMER_FUNC_TA_WAIT_AF_CHECK:
			timerValue = RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer ;
			break;

		case RDS_TIMER_AFINFO_AF:
			timerValue = RDSMgr[slot].AFInfo.AFTimer ;
			break;

		case RDS_TIMER_AFINFO_AF_SMETER_CHECKF_REQ:
			timerValue = RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer ;
			break;

		case RDS_TIMER_AFINFO_AF_CHECK:
			timerValue = RDSMgr[slot].AFInfo.AFCheckWaitTimer ;
			break;

		case RDS_TIMER_AFINFO_AF_DISABLE_100MS:
			timerValue = RDSMgr[slot].AFInfo.AFDisable100msTimer ;
			break;

		case RDS_TIMER_AFINFO_AF_SEARCH:
			timerValue = RDSMgr[slot].AFInfo.AFSearchWaitTimer ;
			break;

		case RDS_TIMER_AFINFO_RDSSTATION_DELAY:
			timerValue = RDSMgr[slot].AFInfo.RDSStationDelayTimer ;
			break;
			
		case RDS_TIMER_PIINFO_PISEEK:
			timerValue = RDSMgr[slot].PIInfo.PISeekTimer ;
			break;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
		case RDS_TIMER_EONINFO_EONAF:
			timerValue = RDSMgr[slot].EONInfo.EONAFTimer ;
			break;

		case RDS_TIMER_EONINFO_EONEXIT:
			timerValue = RDSMgr[slot].EONInfo.EONExitTimer ;
			break;
#endif
	
		default :
			timerValue = 0xFF;
	}
	ETALTML_RDS_Strategy_ReleaseLock(RDS_LOCK_TIMER);
	//printf("ETALTML_RDS_Strategy_RDSTimer_Get receiver = %d,  type = %d, time = %d\n", hReceiver,  timerType, timerValue);

	return timerValue;
}


/**
* ETALTML_RDS_Strategy_Timer_hdr
* @desc: RDS timer processing
* @param : tSInt slot
* @return : none
* @note: 
*/
static tVoid ETALTML_RDS_Strategy_Timer_hdr(tSInt slot)
{
	static tU32 T10ms;	
	static tU32 T100ms;	
	//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML_RDS_Strategy_Timer_hdr %d \n", slot);
		
	ETALTML_RDS_Strategy_GetLock(RDS_LOCK_TIMER);
	
	T10ms++;
	//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML_RDS_Strategy_Timer_hdr in RDS_Timer[slot].Timer10ms %d\n", RDS_Timer[slot].Timer10ms);
	
	// 10ms timer count and act
	if(RDS_Timer[slot].Timer10ms > 0)  RDS_Timer[slot].Timer10ms--;

	
	//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML_RDS_Strategy_Timer_hdr OUT RDS_Timer[slot].Timer10ms %d\n", RDS_Timer[slot].Timer10ms);
	
	ETALTML_RDS_10msTimer_Act(slot);

	if(T10ms >= 10)
	{
		T10ms  = 0;

		// 100ms timer count and act
		if(RDS_Timer[slot].Timer100ms > 0) RDS_Timer[slot].Timer100ms--;

		ETALTML_RDS_100msTimer_Act(slot);

		// 1s timer count and act
		T100ms++;
		if(T100ms >= 10)
		{
			T100ms = 0;
			ETALTML_RDS_1sTimer_Act(slot);
		}
	}
	
	ETALTML_RDS_Strategy_ReleaseLock(RDS_LOCK_TIMER);

	
	//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML_RDS_Strategy_Timer_hdr done\n");
	
}


/**
* ETALTML_RDS_Strategy_GetRuntimeData
* @desc: get rds strategy run time data/status etc
* @param : hReceiver, pRDSData
* @return ETAL_STATUS : error code.
* @note: 
*/
ETAL_STATUS ETALTML_RDS_Strategy_GetRuntimeData(ETAL_HANDLE hReceiver, EtalRDSStrategyStatus *pRDSStrategyStatus)
{
	tSInt slot;

	// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (DABMW_INVALID_SLOT == slot) return ETAL_RET_ERROR;
	if (pRDSStrategyStatus == NULL) return ETAL_RET_ERROR;

	if (RDSMgr[slot].FunctionInfo.F_AFEnable) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_AFENABLE;
	if (RDSMgr[slot].FunctionInfo.F_TAEnable) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_TAEANBLE;
	if (RDSMgr[slot].FunctionInfo.F_REGEnable) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_REGENABLE;
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	if (RDSMgr[slot].FunctionInfo.F_EONEnable) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_EONENABLE;
#endif
	if (RDSMgr[slot].FunctionInfo.F_PTYEnable) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_PTYENABLE;

	if (RDSMgr[slot].ProcessFlag.F_InTASwitch) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_INTASWITCH;
	if (RDSMgr[slot].ProcessFlag.F_InEONTASwitch) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_INEONTASWITCH;
	if (RDSMgr[slot].AFInfo.F_RDSStationSlow) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_RDSSTATION;
	
	if (RDSMgr[slot].AFInfo.AFCheckMode != AFCHECK_MODE_IDLE) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_AFCHECKING;

	if (RDSMgr[slot].ProcessFlag.F_NoPTY) pRDSStrategyStatus->m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_NoPTY;

/*   Need consider this in the future
	pRDSStrategyStatus->m_RDSData.m_AFListLen = RDSMgr[slot].AFInfo.AFNum;

	for(i = 0; i < 8; i++)
	{
		pRDSStrategyStatus->m_RDSData.m_AFList[i] = RDSMgr[slot].AFInfo.AFFreq[i];
	}
*/
	return ETAL_RET_SUCCESS;
}


/**
* ETALTML_RDS_DataNotification
* @desc This routine is to send the notification while the RDS data used in application layer are changed
* @param: ETAL_HANDLE hReceiver, tSInt slot, pTASwitchMode, afFreq, TASeekStarted, PISeekStarted
* @return ETAL_STATUS : error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_DataNotification(ETAL_HANDLE hReceiver, tSInt slot, RDS_TASwitchSystemMode * pTASwitchMode, tBool AFSwitched, tU8 afFreq, tBool TASeekStarted, tBool PISeekStarted)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#if defined(CONFIG_ETAL_HAVE_ETALTML) && defined(CONFIG_ETALTML_HAVE_RDS_STRATEGY)

	EtalRDSStrategyStatus RDSStrategyStatus;

	if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialRDSStrategy))
	{
		RDSStrategyStatus.m_AFSwitchedFreq = 0xFF;
		RDSStrategyStatus.m_RDSStrategyBitmap = 0;
		ETALTML_RDS_Strategy_GetRuntimeData(hReceiver, &RDSStrategyStatus);
		if (pTASwitchMode)
		{
			if ((*pTASwitchMode) == SWITCH_TO_TUNER) 
			{
				RDSStrategyStatus.m_RDSStrategyBitmap |=  ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_TO_TUNER;
			}
			else
			{
				RDSStrategyStatus.m_RDSStrategyBitmap |=  ETAL_RDS_STRATEGY_F_TASTATUS_SWITCH_BACK_FROM_TUNER;				
			}
		}

		if (AFSwitched)
		{
			RDSStrategyStatus.m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_AFSWITCHED;
			RDSStrategyStatus.m_AFSwitchedFreq = afFreq;
		}

		if (TASeekStarted)
		{
			RDSStrategyStatus.m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_INTASEEK;			
		}

		if (PISeekStarted)
		{
			RDSStrategyStatus.m_RDSStrategyBitmap |= ETAL_RDS_STRATEGY_F_INPISEEK;	
		}
					
		/* Send ETAL_INFO_RDS_STRATEGY event */
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_RDS_STRATEGY, (tVoid *)(&RDSStrategyStatus),  sizeof(EtalRDSStrategyStatus));
	}

#endif

	return ret;
}


/**
* ETALTML_RDS_Strategy_AFData_Decode
* @desc This routine is used to decode AF data which used in RDS strategy.
* @param:  tSInt slot, RDS_charIntTy AFtemp
* @return :  None
* @note:
*/
tVoid ETALTML_RDS_Strategy_AFData_Decode(tSInt slot, RDS_charIntTy AFtemp)
{
	tInt i;
	
	// LF/MF
	if(AFtemp.byte[0] == 250)
	{
		AFtemp.byte[0] = INVALID_AF;
		AFtemp.byte[1] = INVALID_AF;
	}
	
	if(RDSMgr[slot].AFInfo.AFMethod[0] == INVALID_AF  &&  RDSMgr[slot].AFInfo.AFMethod[1] == INVALID_AF)
	{
		// first time, just backup
		RDSMgr[slot].AFInfo.AFMethod[0] = AFtemp.byte[0];
		RDSMgr[slot].AFInfo.AFMethod[1] = AFtemp.byte[1];
	}
	else
	{
		if(RDSMgr[slot].AFInfo.AFMethod[0] != 0xFE  &&  RDSMgr[slot].AFInfo.AFMethod[1] != 0xFE)
		{
			if( (RDSMgr[slot].AFInfo.AFMethod[0] == AFtemp.byte[0]  ||  RDSMgr[slot].AFInfo.AFMethod[1] == AFtemp.byte[0] ||
			      RDSMgr[slot].AFInfo.AFMethod[0] == AFtemp.byte[1]  ||  RDSMgr[slot].AFInfo.AFMethod[1] == AFtemp.byte[1])  &&
			    (RDSMgr[slot].AFInfo.AFMethod[0] != AFtemp.byte[0]  ||  RDSMgr[slot].AFInfo.AFMethod[1] != AFtemp.byte[1]) )
			{
				// get method B
				RDSMgr[slot].AFInfo.F_AFMethodB = TRUE;
			}
			RDSMgr[slot].AFInfo.AFMethod[0] = 0xFE;
			RDSMgr[slot].AFInfo.AFMethod[1] = 0xFE;
		}

		if(RDSMgr[slot].AFInfo.F_AFMethodB  &&  AFtemp.byte[0] > AFtemp.byte[1])
		{
			AFtemp.byte[0] = INVALID_AF;
			AFtemp.byte[1] = INVALID_AF;
		}

		for(i = 0; i < MAX_AF_NUMBER; i++)
		{
			if(AFtemp.byte[0] == RDSMgr[slot].AFInfo.AFFreq[i]  &&  AFtemp.byte[0] != INVALID_AF)
			{
				AFtemp.byte[0] = INVALID_AF;
			}
		}

		if(AFtemp.byte[0] < 205  &&  RDSMgr[slot].AFInfo.AFNum < MAX_AF_NUMBER)
		{
			RDSMgr[slot].AFInfo.AFFreq[RDSMgr[slot].AFInfo.AFNum] = AFtemp.byte[0];
			RDSMgr[slot].AFInfo.AFNum++;
			RDSMgr[slot].AFInfo.AFTimer = 50;
			RDSMgr[slot].AFInfo.F_GetNewAF = TRUE;
		}

		for(i = 0; i < MAX_AF_NUMBER; i++)
		{
			if(AFtemp.byte[1] == RDSMgr[slot].AFInfo.AFFreq[i]  &&  AFtemp.byte[1] != INVALID_AF)
			{
				AFtemp.byte[1] = INVALID_AF;
			}
		}
		
		if(AFtemp.byte[1] < 205  &&  RDSMgr[slot].AFInfo.AFNum < MAX_AF_NUMBER)
		{
			RDSMgr[slot].AFInfo.AFFreq[RDSMgr[slot].AFInfo.AFNum] = AFtemp.byte[1];
			RDSMgr[slot].AFInfo.AFNum ++ ;
			RDSMgr[slot].AFInfo.AFTimer = 50;
			RDSMgr[slot].AFInfo.F_GetNewAF = TRUE;
		}

		if(RDSMgr[slot].AFInfo.AFTimer == 0  &&  RDSMgr[slot].AFInfo.AFNum > 1  &&  RDSMgr[slot].AFInfo.F_GetNewAF)
		{
			RDSMgr[slot].AFInfo.F_GetNewAF = FALSE;
			RDSMgr[slot].AFInfo.F_GetAFList  = TRUE;
		}
	}
}


/**
* ETALTML_RDS_Strategy_IsPISeekOk
* @desc This routine is used check the PI Seek is OK or not
* @param: ETAL_HANDLE hReceiver
* @return: tBool,  TRUE means PI seek OK.
* @note:
*/
tBool ETALTML_RDS_Strategy_IsPISeekOk(ETAL_HANDLE hReceiver)
{
	DABMW_storageStatusEnumTy PIStatus = DABMW_STORAGE_STATUS_IS_EMPTY;
	tU32 currentPI 	= 0;
	tU32 lastPI 		= 0;
	tU32 backupPI 	= 0;
	ETAL_STATUS  ret;
	tBool vl_ret = FALSE;

	ret = ETALTML_get_PIStatus(hReceiver, &PIStatus, &currentPI, & lastPI, &backupPI);
	if( ( ETAL_RET_SUCCESS == ret) && 
	     ( (PIStatus == DABMW_STORAGE_STATUS_IS_VERIFIED)  ||
	       (PIStatus == DABMW_STORAGE_STATUS_IS_USED) ) )
	{
		tSInt slot;
#ifdef ETAL_RDS_IMPORT
		slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
		slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
		if (slot >= 0)
		{
			if(( ((currentPI == RDSMgr[slot].PIInfo.BackupPI) && RDSMgr[slot].FunctionInfo.F_REGEnable)  ||
			       (((currentPI & 0xFF) == (RDSMgr[slot].PIInfo.BackupPI &0xFF) ) && (!RDSMgr[slot].FunctionInfo.F_REGEnable))) &&
			       (currentPI != 0))
			{
				vl_ret = TRUE;
			}
		}
	}

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2)
	ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_USER_2, "ETALTML_RDS_Strategy_IsPISeekOk, receiver = %d, PI found status = %d, PI %d\n",
		hReceiver, vl_ret, currentPI);
#endif	
	return vl_ret;
}


/**
* ETALTML_RDS_Strategy_SetBackupPI
* @desc This routine is used to set the rds strategy backupPI
* @param: ETAL_HANDLE hReceiver, backupPI
* @return: None
* @note:
*/
tVoid ETALTML_RDS_Strategy_SetBackupPI(ETAL_HANDLE hReceiver, tU32 backupPI)
{
	tSInt slot;

	if (backupPI == 0) return;
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (slot >= 0)
	{
		RDSMgr[slot].PIInfo.BackupPI = backupPI;
	}
}


/**
* ETALTML_RDS_Strategy_GetSwitchStatus
* @desc This routine is used to get the rds strategy funcs switch status
* @param: ETAL_HANDLE hReceiver
* @return: tBool,  TRUE means RDS is eanbled and AF strategy is enabled.
* @note:
*/
tBool ETALTML_RDS_Strategy_GetSwitchStatus(ETAL_HANDLE hReceiver)
{
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (slot >= 0)
	{
  		return (RDSMgr[slot].FunctionInfo.F_RDSEnable && RDSMgr[slot].FunctionInfo.F_AFEnable);
	}
	
	return FALSE;
}


/**
* ETALTML_RDS_Strategy_ResetPISeekTimer
* @desc This routine is used to reset the PI seek timer
* @param: slot, bForced
* @return: None
* @note:
*/
tVoid ETALTML_RDS_Strategy_ResetPISeekTimer(tSInt slot, tBool bForced)
{
	if (slot >= 0)
	{
		if (( ((!bForced) && (RDSMgr[slot].FunctionInfo.F_AFEnable) &&  (!RDSMgr[slot].ProcessFlag.F_PISeek) ) &&
			(RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_IDLE) ) || bForced )
		{
			ETALTML_RDSTimer_Set_Internal(slot, RDS_TIMER_PIINFO_PISEEK, RDSMgr[slot].PIInfo.PISeek100msTimerInitVal);
		}					
	}
}


/**
* ETALTML_RDS_Strategy_GetFunctionStatus
* @desc This routine is used to get the rds strategy funcs  status
* @param: ETAL_HANDLE hReceiver, RDS_FuncType rdsRuncType
* @return: tBool,  TRUE means related RDS function is enabled.
* @note:
*/
tBool ETALTML_RDS_Strategy_GetFunctionStatus(ETAL_HANDLE hReceiver, RDS_FuncType rdsRuncType)
{
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (slot >= 0)
	{		
		if (rdsRuncType == RDS_FUNC_AF) 		return RDSMgr[slot].FunctionInfo.F_AFEnable;
		if (rdsRuncType == RDS_FUNC_TA) 		return RDSMgr[slot].FunctionInfo.F_TAEnable;
		if (rdsRuncType == RDS_FUNC_REG) 	return RDSMgr[slot].FunctionInfo.F_REGEnable;
		if (rdsRuncType == RDS_FUNC_PIMUTE) 	return RDSMgr[slot].FunctionInfo.F_PIMuteEnable;
		if (rdsRuncType == RDS_FUNC_PTY) 	return RDSMgr[slot].FunctionInfo.F_PTYEnable;
		if (rdsRuncType == RDS_FUNC_EON) 	return RDSMgr[slot].FunctionInfo.F_EONEnable;
		if (rdsRuncType == RDS_FUNC_PISeek) 	return RDSMgr[slot].ProcessFlag.F_PISeek;
		if (rdsRuncType == RDS_FUNC_PTYSELECTTYPE) 	return RDSMgr[slot].FunctionInfo.PTYSelectType;
	}
	
	return FALSE;
}


/**
* ETALTML_RDS_Strategy_SetFunctionStatus
* @desc This routine is used to set the rds strategy funcs  status
* @param: ETAL_HANDLE hReceiver, RDS_FuncType rdsRuncType, tBool value
* @return: none
* @note:
*/
tVoid ETALTML_RDS_Strategy_SetFunctionStatus(ETAL_HANDLE hReceiver, RDS_FuncType rdsRuncType, tBool value)
{
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (slot >= 0)
	{		
		if (rdsRuncType == RDS_FUNC_AF)  	RDSMgr[slot].FunctionInfo.F_AFEnable = value;
		if (rdsRuncType == RDS_FUNC_TA)  	RDSMgr[slot].FunctionInfo.F_TAEnable = value;
		if (rdsRuncType == RDS_FUNC_REG) 	RDSMgr[slot].FunctionInfo.F_REGEnable = value;
		if (rdsRuncType == RDS_FUNC_PIMUTE) 	RDSMgr[slot].FunctionInfo.F_PIMuteEnable = value;
		if (rdsRuncType == RDS_FUNC_PTY) 	RDSMgr[slot].FunctionInfo.F_PTYEnable = value;
		if (rdsRuncType == RDS_FUNC_PTYSELECTTYPE) 	RDSMgr[slot].FunctionInfo.PTYSelectType = value;
		if (rdsRuncType == RDS_FUNC_EON) 	RDSMgr[slot].FunctionInfo.F_EONEnable = value;
		if (rdsRuncType == RDS_FUNC_PISeek) 	RDSMgr[slot].ProcessFlag.F_PISeek = value;
	}
}


/**
* ETALTML_RDS_Strategy_RDSEnable
* @desc This routine is used to enable RDS 
* @param: ETAL_HANDLE hReceiver
* @return: none
* @note:
*/
tVoid ETALTML_RDS_Strategy_RDSEnable(ETAL_HANDLE hReceiver)
{
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (slot >= 0)	RDSMgr[slot].FunctionInfo.F_RDSEnable = TRUE; 
}


/**
* ETALTML_RDS_Strategy_RDSDisable
* @desc This routine is used to disable RDS 
* @param: ETAL_HANDLE hReceiver
* @return: none
* @note:
*/
tVoid ETALTML_RDS_Strategy_RDSDisable(ETAL_HANDLE hReceiver)
{
	tSInt slot;
	
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif

	if (slot >= 0)	RDSMgr[slot].FunctionInfo.F_RDSEnable = FALSE;
}


#if defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) || defined (CONFIG_ETAL_HAVE_ALL_API)

/**
* ETALTML_RDS_Mute
* @desc This routine is to mute on or off the radio audio
* @param       :hReceiver;  Is_Mute-1 mute on, -0 mute off
* @return ETAL_STATUS : error code.
 * @note 	       : 
 */
ETAL_STATUS ETALTML_RDS_Mute(ETAL_HANDLE hReceiver, tBool bMute)
{	
	return ETAL_mute(hReceiver, bMute);
}
#else
ETAL_STATUS ETALTML_RDS_Mute(ETAL_HANDLE hReceiver, tBool bMute)
{
	//Do nothing;
	return ETAL_RET_SUCCESS;
}
#endif


/**
* ETALTML_RDS_AFCheckInit
* @desc This routine is to initialize the RDS data when changing the frequency of the  RDS station
* @param: hReceiver
* @return ETAL_STATUS : error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_AFCheckInit(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	
	//Automatic reset the RDS buffer in FW side.
	ETALTML_RDSresetData(hReceiver);  //  CLEAR RDS DATA, reset rds data

	return ret;
}


/**
* ETALTML_RDS_SetFreq
* @desc This routine is to quickly switch the frequency of the RDS station
* @param: ETAL_HANDLE hReceiver, setfreq
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_SetFreq(ETAL_HANDLE hReceiver, tU8 setfreq)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU32 freq;

	freq = FREQ_AF_TO_KHZ(setfreq);

	ret = ETAL_AF_switch(hReceiver, freq);
	if (ret == ETAL_RET_SUCCESS)
	{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
		ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "ETALTML_RDS_SetFreq, set frequency to : %d\r\n", FREQ_AF_TO_KHZ(setfreq));
#endif
		ETALTML_RDS_AFCheckInit(hReceiver);		//  CLEAR RDS DATA, reset rds data
	}

	return ret;
}


/**
* ETALTML_RDS_ClearTAFlags
* @desc This routine is to clear the TA flags of the RDS station
* @param: tSInt slot
* @return ETAL_STATUS : error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_ClearTAFlags(tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	DABMW_CleanUpTPTA(slot);

	RDSMgr[slot].TATPInfo.F_GetNewTP = FALSE;
	RDSMgr[slot].TATPInfo.F_GetNewTA = FALSE;
	RDSMgr[slot].TATPInfo.BackupTP = FALSE;
	RDSMgr[slot].TATPInfo.BackupTA = FALSE;
	
	return ret;  
}


/**
* ETALTML_RDS_Strategy_BreakAFCheck
* @desc: This is the routine to stop the AF checking in the AF processing
* @param: hReceiver
* @return ETAL_STATUS
* @note:
*/
ETAL_STATUS ETALTML_RDS_Strategy_BreakAFCheck(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt slot;

	// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (DABMW_INVALID_SLOT == slot)  return ETAL_RET_ERROR;
	
	if(RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_IDLE) return ret;
	RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_IDLE;
	
	ret = ETALTML_RDS_SetFreq(hReceiver, RDSMgr[slot].AFInfo.AFFreq[0]);
	
	OSAL_s32ThreadWait(2);
	
	return ret;  
}


/**
* ETALTML_RDSAF_AFChange
* @desc: This is the sub routine used in the AF processing to change the AF frequrency and the related information in the AF lsit
* @param: tSInt slot;  
* @param: num1;  num2
* @return ETAL_STATUS
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_AFChange(tSInt slot, tU8 num1, tU8 num2)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tS32 tempLevel;
	tU8 temp;

	if (num1 == num2) return ret;
	
	temp = RDSMgr[slot].AFInfo.AFFreq[num1];
	RDSMgr[slot].AFInfo.AFFreq[num1] = RDSMgr[slot].AFInfo.AFFreq[num2];
	RDSMgr[slot].AFInfo.AFFreq[num2] = temp;
	tempLevel = RDSMgr[slot].AFInfo.AFSmeter[num1];
	RDSMgr[slot].AFInfo.AFSmeter[num1] = RDSMgr[slot].AFInfo.AFSmeter[num2];
	RDSMgr[slot].AFInfo.AFSmeter[num2] = tempLevel;
	temp = RDSMgr[slot].AFInfo.AFDisable[num1];
	RDSMgr[slot].AFInfo.AFDisable[num1] = RDSMgr[slot].AFInfo.AFDisable[num2];
	RDSMgr[slot].AFInfo.AFDisable[num2] = temp;
	
	return ret;
}


/**
* ETALTML_RDSAF_AFSort
* @desc: This is the sub routine used in the AF processing to sort the AF freqeuncy in the AF list according to the smeter
* @param: tSInt slot; bg: 0-sort the all frequncies in the AF list including the current reception frequency (AF[0]), 1-sort the frequencies in the AF list excluding the current reception frequency.
* @return ETAL_STATUS, error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_AFSort(tSInt slot, tU8 bg)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU8 i, j;
	
	for(i = bg; i < RDSMgr[slot].AFInfo.AFNum - 1; i++)
	{
		for(j = RDSMgr[slot].AFInfo.AFNum - 1; j > i;  j--)
		{
			if(RDSMgr[slot].AFInfo.AFSmeter[j] > RDSMgr[slot].AFInfo.AFSmeter[j - 1])
			{
				ETALTML_RDSAF_AFChange(slot, j,  j - 1);
			}
		}
	}
	return ret;
}


/**
* ETALTML_RDSAF_ReadTunerCFQual
* @desc: This is the sub routine used in the AF processing to get the quality on the current reception frequency (AF[0])
* @param: hReceiver, *CFQual for signal quality information
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_ReadTunerCFQual(ETAL_HANDLE hReceiver, RDS_TunerQualInfoTy *CFQual)
{
	ETAL_STATUS ret;
	EtalBcastQualityContainer fmQual;

	// Get tuner CF quality
	ret = ETAL_get_reception_quality_internal(hReceiver, &fmQual);

	if (ret == ETAL_RET_SUCCESS)
	{
		CFQual->Smeter = fmQual.EtalQualityEntries.amfm.m_BBFieldStrength;
		CFQual->Detuning = fmQual.EtalQualityEntries.amfm.m_FrequencyOffset;
		CFQual->Multipath = fmQual.EtalQualityEntries.amfm.m_Multipath;
		CFQual->AdjChannel = fmQual.EtalQualityEntries.amfm.m_UltrasonicNoise;		//Need double check,  m_AdjacentChannel
	}

	return ret;  
}


/**
* ETALTML_RDSAF_ReadTunerAFQual
* @desc: This is the sub routine used in the AF processing to get the quality on the AF checked frequency
* @param: ETAL_HANDLE hReceiver, *AFQual
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_ReadTunerAFQual(ETAL_HANDLE hReceiver, RDS_TunerQualInfoTy *AFQual)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastQualityContainer fmQual;
	tSInt retval;

	retval = ETAL_cmdGetAFQuality_CMOST(hReceiver, &fmQual);
	if (retval != OSAL_OK) 
	{
		ret = ETAL_RET_ERROR;
	}
	else
	{
		AFQual->Smeter = fmQual.EtalQualityEntries.amfm.m_BBFieldStrength;
		AFQual->Detuning = fmQual.EtalQualityEntries.amfm.m_FrequencyOffset;
		AFQual->Multipath = fmQual.EtalQualityEntries.amfm.m_Multipath;
		AFQual->AdjChannel = fmQual.EtalQualityEntries.amfm.m_UltrasonicNoise;		
	}
		
	return ret;  
}


/**
* ETALTML_RDSAF_BreakAFSearch
* @desc: This is the routine to stop the AF searching in the AF processing
* @param: ETAL_HANDLE hReceiver, 
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_BreakAFSearch(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt i, slot;

	// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (DABMW_INVALID_SLOT == slot)
	{
		return ETAL_RET_ERROR;
	}
	
	if(RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_SEARCHREQ ||
	   RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_SEARCHRDS ||
	   RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_SEARCHPI)
	{
		RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_IDLE;
		
		for(i = 0; i < MAX_AF_NUMBER; i++)
		{
			if(RDSMgr[slot].AFInfo.AFFreqBackup == RDSMgr[slot].AFInfo.AFFreq[i])
			{
				ETALTML_RDSAF_AFChange(slot, 0, i);
				break;
			}
		}
	}
	return ret;  
}


/**
* ETALTML_RDSAF_SetAFSearch
* @desc: This is the routine to start the AF searching in the AF processing
* @param: slot
* @return ETAL_STATUS : error code
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_SetAFSearch(tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	
	RDSMgr[slot].AFInfo.AFCheckStartTimerSet = 0;
	RDSMgr[slot].AFInfo.AFCheckWaitTimerSet  = 0xFF;
	RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_SEARCHREQ;
	RDSMgr[slot].AFInfo.AFDisable100msTimer = 0;
	RDSMgr[slot].AFInfo.AFSearchWaitTimer = 20;
	
	return ret;  
}


/**
* ETALTML_RDSAF_AFCheckBack
* @desc: This is the sub routine used in the AF processing to switch back the status from the last checked frequrency
* @param:ETAL_HANDLE hReceiver ; tSInt slot;  flag: 1-the frequncy is needed to be changed back apart from changing the status information from the checked frequency
* @return ETAL_STATUS :etal status
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_AFCheckBack(ETAL_HANDLE hReceiver, tSInt slot, tBool bFlag)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	
	if(bFlag)
	{
		ret = ETALTML_RDS_SetFreq(hReceiver, RDSMgr[slot].AFInfo.AFFreq[0]);
		
		//Wait 1ms
		OSAL_s32ThreadWait(1);

		ETALTML_RDS_Mute(hReceiver, SOFTOFF);
	}
	
	RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKWAIT;
	RDSMgr[slot].AFInfo.AFCheckWaitTimer = RDSMgr[slot].AFInfo.AFCheckWaitTimerSet;
	
	return ret;  
}


/**
* ETALTML_RDSAF_AFSearchNext
* @desc: This is the sub routine used in the AF processing to search the next frequency in the AF list
* @param: ETAL_HANDLE hReceiver; tSInt slot, tBool bAFEnd
* @return : ETAL_STATUS, error code
* @note:
*/
ETAL_STATUS ETALTML_RDSAF_AFSearchNext(ETAL_HANDLE hReceiver, tSInt slot, tBool bAFEnd)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU32 freq;
	EtalBcastQualityContainer afQual;
	tSInt i;
	
	for(i = RDSMgr[slot].AFInfo.AFCheckPoint; i < RDSMgr[slot].AFInfo.AFNum; i++)
	{
		if(RDSMgr[slot].AFInfo.AFSmeter[i] > RDSMgr[slot].AFInfo.AFStrengthFast)
		{
			freq = FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[i]);

			if (bAFEnd)
			{
				ret = ETAL_AF_end(hReceiver, freq, &afQual);
			}
			else
			{
				ret = ETAL_AF_switch(hReceiver, freq);
			}

			OSAL_s32ThreadWait(5);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "ETALTML_RDSAF_AFSearchNext, set frequency to : %d\r\n", FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[i]));
#endif
			ETALTML_RDSresetData(hReceiver);  //  CLEAR RDS DATA, reset rds data
			
			RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_SEARCHRDS;
			RDSMgr[slot].AFInfo.AFCheckWaitTimer = 60;
			RDSMgr[slot].AFInfo.AFCheckPoint = i;
			
			OSAL_s32ThreadWait(5);

			return ret;
		}
	}

	for(i = 0; i <RDSMgr[slot].AFInfo.AFNum; i++)
	{
		if(RDSMgr[slot].AFInfo.AFFreqBackup == RDSMgr[slot].AFInfo.AFFreq[i])
		{
			ETALTML_RDSAF_AFChange(slot, 0, i);
			break;
		}
	}

	freq = FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[0]);
	if (bAFEnd)
	{
		ret = ETAL_AF_end(hReceiver, freq, &afQual);
	}
	else
	{
		ret = ETAL_AF_switch(hReceiver, freq);
	}
	
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
	ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "ETALTML_RDSAF_AFSearchNext, set frequency to : %d\r\n", FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[0]));
#endif

	ETALTML_RDS_AFCheckInit(hReceiver);		//  CLEAR RDS DATA, reset rds data
	
	OSAL_s32ThreadWait(2);

	ETALTML_RDS_Mute(hReceiver, SOFTOFF);
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	if(RDSMgr[slot].EONInfo.F_InEONTA)
	{
		RDSMgr[slot].EONInfo.EONMode = EON_MODE_NOT_FIND;
		RDSMgr[slot].EONInfo.F_NoMatchEON = TRUE;
	}
#endif
	RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKEND;

	return ret;  
}


/**
* ETALTML_RDSTA_SwitchToTa
* @desc: This is used to set InTASwitch flag and noitfy RTA to change mode to tuner
* @param: ETAL_HANDLE hReceiver, tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSTA_SwitchToTa(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	RDS_TASwitchSystemMode TASwitchMode = SWITCH_TO_TUNER;
		
	if(RDSMgr[slot].ProcessFlag.F_InTASwitch) return ret;
	
	RDSMgr[slot].ProcessFlag.F_InTASwitch = TRUE;
	RDSMgr[slot].FuncTimer.InTADelayTimer = 10;
	
	ETALTML_RDS_DataNotification(hReceiver, slot, &TASwitchMode, FALSE, INVALID_AF, FALSE, FALSE);

	return ret;	
}


/**
* ETALTML_RDSTA_TaSwitchBack
* @desc: This is used to reset InTASwitch flag and noitfy RTA to change mode to backup mode
* @param: ETAL_HANDLE hReceiver, tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSTA_TaSwitchBack(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	RDS_TASwitchSystemMode TASwitchMode = SWITCHBACK_FROM_TUNER;
	
	if(!RDSMgr[slot].ProcessFlag.F_InTASwitch)  return ret;
	
	RDSMgr[slot].ProcessFlag.F_InTASwitch = FALSE;
	ETALTML_RDS_ClearTAFlags(slot);
	
	ETALTML_RDS_DataNotification(hReceiver, slot, &TASwitchMode, FALSE, INVALID_AF, FALSE, FALSE);
	return ret;	
}


#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON

#define GROUP14A(groupData)	 ((groupData) == 0x1C)
#define GROUP14B(groupData)	 ((groupData) == 0x1D)


/**
* ETALTML_RDS_Strategy_EONData_Decode
* @desc This routine is used to decode Eon data which used in RDS strategy.
* @param:  tSInt slot, RDS blocks data and flags
* @return None
* @note:
*/
tVoid ETALTML_RDS_Strategy_EONData_Decode(tSInt slot, tU32 piVal, tU8 realGroup,
									                            tBool availBlock_A, tU32 block_A, 
									                            tBool availBlock_B, tU32 block_B, 
									                            tBool availBlock_C, tU32 block_C,
									                            tBool availBlock_Cp, tU32 block_Cp,
									                            tBool availBlock_D, tU32 block_D)
{
	tSInt i, j;
	tU32  temp;
	RDS_charIntTy AFtemp;

	// EON
 	if(GROUP14A(realGroup) &&  availBlock_A &&  availBlock_B && availBlock_C &&  availBlock_D)
	{
		temp = (block_B) & 0x000F;

		switch(temp)
		{
			// EON PS
			case 0:
				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] ==  block_D)
					{
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][1] = (tU8)( block_C & 0x00FF);
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][0] = (tU8)( block_C >> 8);
					}
				}
				break;
				
			case 1:
				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] ==  block_D)
					{
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][3] = (tU8)( block_C & 0x00FF);
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][2] = (tU8)( block_C >> 8);
					}
				}
				break;
				
			case 2:
				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] ==  block_D)
					{
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][5] = (tU8)( block_C & 0x00FF);
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][4] = (tU8)( block_C >> 8);
					}
				}
				break;
				
			case 3:
				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] ==  block_D)
					{
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][7] = (tU8)( block_C & 0x00FF);
						 RDSMgr[slot].EONInfo.EONSave_EONPS[j][6] = (tU8)( block_C >> 8);
					}
				}
				break;

			// EON AF
			case 4:
				AFtemp.dbyte =  block_C;

				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] == 0 || RDSMgr[slot].EONInfo.EONSave_EONPI[j] ==  block_D)
					{
						if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] == 0)
						{
							 RDSMgr[slot].EONInfo.EONSave_EONPI[j] =  block_D;
						}

						for(i = 0;i < MAX_AF_NUMBER; i++)
						{
							if(AFtemp.byte[0] ==  RDSMgr[slot].EONInfo.EONSave_EONAF[j][i]) AFtemp.byte[0] = INVALID_AF;
							if(AFtemp.byte[1] ==  RDSMgr[slot].EONInfo.EONSave_EONAF[j][i]) AFtemp.byte[1] = INVALID_AF;
						}
						
						if(AFtemp.byte[0] < 205 &&  RDSMgr[slot].EONInfo.EONSave_EONAFNum[j] < MAX_AF_NUMBER)
						{
							 RDSMgr[slot].EONInfo.EONSave_EONAF[j][ RDSMgr[slot].EONInfo.EONSave_EONAFNum[j]] = AFtemp.byte[0];
							 RDSMgr[slot].EONInfo.EONSave_EONAFNum[j]++;
						}
						
						if(AFtemp.byte[1] < 205 &&  RDSMgr[slot].EONInfo.EONSave_EONAFNum[j] < MAX_AF_NUMBER)
						{
							 RDSMgr[slot].EONInfo.EONSave_EONAF[j][ RDSMgr[slot].EONInfo.EONSave_EONAFNum[j]] = AFtemp.byte[1];
							 RDSMgr[slot].EONInfo.EONSave_EONAFNum[j]++;
						}
						
						break;
					}
				}				
				
				break;

			case 5:
			case 6:
			case 7:
			case 8:
				AFtemp.dbyte =  block_C;

				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] == 0 ||  RDSMgr[slot].EONInfo.EONSave_EONPI[j] ==  block_D)
					{
						if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] == 0)
						{
							 RDSMgr[slot].EONInfo.EONSave_EONPI[j] =  block_D;
						}

						for(i = 0; i < MAX_AF_NUMBER; i++)
						{
							if(AFtemp.byte[1] ==  RDSMgr[slot].EONInfo.EONSave_EONAF[j][i]) AFtemp.byte[1] = INVALID_AF;
						}
						
						if(AFtemp.byte[1] < 205 &&  RDSMgr[slot].EONInfo.EONSave_EONAFNum[j] < MAX_AF_NUMBER)
						{
							 RDSMgr[slot].EONInfo.EONSave_EONAF[j][ RDSMgr[slot].EONInfo.EONSave_EONAFNum[j]] = AFtemp.byte[1];
							 RDSMgr[slot].EONInfo.EONSave_EONAFNum[j]++;
						}

						break;
					}
				}

				break;

		}
	}

	// EON TP,  EON TA
	if(availBlock_A && availBlock_B  &&  availBlock_Cp && availBlock_D && GROUP14B(realGroup))
	{
		for(j = 0; j < EON_BUFF_SIZE; j++)
		{
			if( RDSMgr[slot].EONInfo.EONSave_EONPI[j] == block_D &&  RDSMgr[slot].EONInfo.EONSave_EONPI[j] != 0)
			{

				if( block_B & 0x0010)
				{
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTP = TRUE;
				}
				else
				{
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTP = FALSE;
				}
				
				if( RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTPLast !=  RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTP)
				{
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTPLast =  RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTP;
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_F_GetNewEONTP = TRUE;
				}

				if( block_B & 0x0008)
				{
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA = TRUE;
				}
				else
				{
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA = FALSE;
				}
				
				if( RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTALast !=  RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA)
				{
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTALast =  RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA;
					 RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_F_GetNewEONTA = TRUE;
				}

			}
		}

		 RDSMgr[slot].EONInfo.EONExitTimer = 50;

	}
 }


/**
* ETALTML_RDSEON_ClearEONTAFlags
* @desc: This is used to reset EON flag
* @param: tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSEON_ClearEONTAFlags(tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	
	RDSMgr[slot].EONInfo.EONTA = FALSE;
	RDSMgr[slot].EONInfo.EONTALast = FALSE;
	RDSMgr[slot].EONInfo.EONTP = FALSE;
	RDSMgr[slot].EONInfo.EONTPLast = FALSE;
	RDSMgr[slot].EONInfo.F_GetNewEONTP = FALSE;
	RDSMgr[slot].EONInfo.F_GetNewEONTA = FALSE;
	
	return ret;
}


/**
* ETALTML_RDSEON_CheckEON
* @desc: This is used to active EON check
* @param: ETAL_HANDLE hReceiver, tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSEON_CheckEON(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	RDS_TunerQualInfoTy TunerAFQual;
	EtalBcastQualityContainer afQual;
	tU32 freq;
	tU8 i, j, temp;
	
	if(RDSMgr[slot].EONInfo.F_InEONTA)	 return ret;
	
	if((RDSMgr[slot].EONInfo.EONPI == 0))
	{
		RDSMgr[slot].EONInfo.EONMode = EON_MODE_NOT_FIND;

		RDSMgr[slot].EONInfo.EONPI = 0;
		RDSMgr[slot].EONInfo.EONAFNum = 0;
		for(i = 0; i < MAX_AF_NUMBER; i++)
		{
			RDSMgr[slot].EONInfo.EONAFFreq[i] = INVALID_AF;
		}
		for(i = 0; i < 8; i++)
		{
			RDSMgr[slot].EONInfo.EONPS[i] = 0;
		}
		RDSMgr[slot].EONInfo.EONTA = FALSE;
		RDSMgr[slot].EONInfo.EONTP = FALSE;
		RDSMgr[slot].EONInfo.F_GetNewEONTA = FALSE;
		
		return ETAL_RET_ERROR;
	}

	ETALTML_RDS_Mute(hReceiver, SOFTON);
	for(i = 0; i < RDSMgr[slot].EONInfo.EONAFNum; i++)
	{
		freq = FREQ_AF_TO_KHZ(RDSMgr[slot].EONInfo.EONAFFreq[i]);
		ret = ETAL_AF_start(hReceiver, cmdNormalMeasurement, freq, 0, &afQual);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
		ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "ETALTML_RDSEON_CheckEON, set frequency to : %d\r\n", FREQ_AF_TO_KHZ(RDSMgr[slot].EONInfo.EONAFFreq[i]));
#endif
		if (ret == ETAL_RET_SUCCESS)
		{
			ETALTML_RDS_AFCheckInit(hReceiver);		//  CLEAR RDS DATA, reset rds data
		
			if(i > 0)
			{
				RDSMgr[slot].EONInfo.EONAFSmeter[i - 1] = afQual.EtalQualityEntries.amfm.m_BBFieldStrength;	
			}
			OSAL_s32ThreadWait(ETALTML_RDS_STRATEGY_AFSTART_WAITTIME);
		}
	}
	
	ETALTML_RDSAF_ReadTunerAFQual(hReceiver, &TunerAFQual);
	
	if(i > 0)	RDSMgr[slot].EONInfo.EONAFSmeter[i - 1] = TunerAFQual.Smeter;

	freq = FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[0]);
	ret = ETAL_AF_end(hReceiver, freq, &afQual);
	
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
	ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "ETALTML_RDSEON_CheckEON, set frequency to : %d\r\n", FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[0]));
#endif

	ETALTML_RDS_AFCheckInit(hReceiver);		//  CLEAR RDS DATA, reset rds data
	
	ETALTML_RDS_Mute(hReceiver, SOFTOFF);

	for(i = 0; i < RDSMgr[slot].EONInfo.EONAFNum - 1; i++)
	{
		for(j = RDSMgr[slot].EONInfo.EONAFNum - 1; j > i;  j--)
		{
			if(RDSMgr[slot].EONInfo.EONAFSmeter[j] > RDSMgr[slot].EONInfo.EONAFSmeter[j - 1])
			{
				temp = RDSMgr[slot].EONInfo.EONAFFreq[j];
				RDSMgr[slot].EONInfo.EONAFFreq[j] = RDSMgr[slot].EONInfo.EONAFFreq[j - 1];
				RDSMgr[slot].EONInfo.EONAFFreq[j - 1] = temp;

				temp = RDSMgr[slot].EONInfo.EONAFSmeter[j];
				RDSMgr[slot].EONInfo.EONAFSmeter[j] = RDSMgr[slot].EONInfo.EONAFSmeter[j - 1];
				RDSMgr[slot].EONInfo.EONAFSmeter[j - 1] = temp;				
			}
		}
	}

	if(RDSMgr[slot].EONInfo.EONAFSmeter[0] < RDSMgr[slot].AFInfo.AFStrengthEON)
	{
		RDSMgr[slot].EONInfo.EONMode = EON_MODE_NOT_FIND;

		RDSMgr[slot].EONInfo.EONPI = 0;
		RDSMgr[slot].EONInfo.EONAFNum = 0;
		for(i = 0; i < MAX_AF_NUMBER; i++)
		{
			RDSMgr[slot].EONInfo.EONAFFreq[i] = INVALID_AF;
		}
		for(i = 0; i < 8; i++)
		{
			RDSMgr[slot].EONInfo.EONPS[i] = 0;
		}
		RDSMgr[slot].EONInfo.EONTA = FALSE;
		RDSMgr[slot].EONInfo.EONTP = FALSE;
		RDSMgr[slot].EONInfo.F_GetNewEONTA = FALSE;

		return ETAL_RET_ERROR;
		
	}

	return ret;	
}


/**
* ETALTML_RDSEON_Backup
* @desc: This is used to backup EON 
* @param:ETAL_HANDLE hReceiver,  tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSEON_Backup(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tInt i;
	
	RDSMgr[slot].EONInfo.F_InEONTAReal = TRUE;
	RDSMgr[slot].EONInfo.F_NoMatchEON = FALSE;
	
	for(i = 0; i < MAX_AF_NUMBER; i++)
	{
		RDSMgr[slot].EONInfo.EONAFSmeter[i] = RDSMgr[slot].AFInfo.AFFreq[i];
		RDSMgr[slot].AFInfo.AFFreq[i] = RDSMgr[slot].EONInfo.EONAFFreq[i];
		RDSMgr[slot].EONInfo.EONAFFreq[i] = INVALID_AF;
	}

	RDSMgr[slot].EONInfo.EONPIBackup = RDSMgr[slot].PIInfo.BackupPI;
	RDSMgr[slot].PIInfo.BackupPI = RDSMgr[slot].EONInfo.EONPI;
	RDSMgr[slot].EONInfo.EONPI = 0;
	RDSMgr[slot].EONInfo.EONAFNumBackup = RDSMgr[slot].AFInfo.AFNum;
	RDSMgr[slot].AFInfo.AFNum = RDSMgr[slot].EONInfo.EONAFNum;
	RDSMgr[slot].EONInfo.EONAFNum = 0;
	
	ETALTML_RDS_Mute(hReceiver, SOFTON);
	ETALTML_RDSAF_SetAFSearch(slot);
	
	return ret;	
}


/**
* ETALTML_RDSEON_Restore
* @desc: This is used to EON restore
* @param: hReceiver, slot
* @return ETAL_STATUS : error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSEON_Restore(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt i;
	
	RDSMgr[slot].EONInfo.F_InEONTAReal = FALSE;
	RDSMgr[slot].EONInfo.F_NoMatchEON = FALSE;
	
	for(i = 0; i < MAX_AF_NUMBER; i++)
	{
		RDSMgr[slot].AFInfo.AFFreq[i] = RDSMgr[slot].EONInfo.EONAFSmeter[i];
		RDSMgr[slot].EONInfo.EONAFFreq[i] = INVALID_AF;
	}

	RDSMgr[slot].EONInfo.EONPI = 0;
	RDSMgr[slot].AFInfo.AFNum = RDSMgr[slot].EONInfo.EONAFNumBackup;
	RDSMgr[slot].EONInfo.EONAFNum = 0;
	
	RDSMgr[slot].PIInfo.BackupPI = RDSMgr[slot].EONInfo.EONPIBackup;

	ETALTML_RDS_Mute(hReceiver, SOFTON);
	ETALTML_RDSAF_SetAFSearch(slot);
	
	return ret;	
}


/**
* ETALTML_RDSTA_SwitchToEONTa
* @desc: This is used to set InEONTASwitch flag and noitfy RTA to change mode to tuner
* @param: ETAL_HANDLE hReceiver,  tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSTA_SwitchToEONTa(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	RDS_TASwitchSystemMode TASwitchMode = SWITCH_TO_TUNER;

	if(ETALTML_RDSEON_CheckEON(hReceiver, slot) != ETAL_RET_SUCCESS)
	{
		RDSMgr[slot].FuncTimer.EnterEONTADelayTimer = 30;
		return ret;
	}
	
	if(RDSMgr[slot].ProcessFlag.F_InEONTASwitch) return ret;
 	
	RDSMgr[slot].ProcessFlag.F_InEONTASwitch = TRUE;

	RDSMgr[slot].FuncTimer.InTADelayTimer = 30;
 	RDSMgr[slot].EONInfo.F_InEONTA = TRUE;


	ETALTML_RDSEON_Backup(hReceiver, slot);
	ETALTML_RDSEON_ClearEONTAFlags(slot);
	ETALTML_RDS_ClearTAFlags(slot);
	
	ETALTML_RDS_DataNotification(hReceiver, slot, &TASwitchMode, FALSE, INVALID_AF, FALSE, FALSE);

	return ret;	
}


/**
* ETALTML_RDSTA_EONTaSwitchBack
* @desc: This is used to reset InEONTASwitch flag and noitfy RTA to change mode to backup mode
* @param: ETAL_HANDLE hReceiver, tSInt slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSTA_EONTaSwitchBack(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	RDS_TASwitchSystemMode TASwitchMode = SWITCHBACK_FROM_TUNER;
	
	RDSMgr[slot].ProcessFlag.F_InEONTASwitch = FALSE;

	RDSMgr[slot].FuncTimer.InTADelayTimer = 20;
 	RDSMgr[slot].EONInfo.F_InEONTA = FALSE;

	ETALTML_RDSEON_Restore(hReceiver, slot);
	ETALTML_RDSEON_ClearEONTAFlags(slot);
	ETALTML_RDS_ClearTAFlags(slot);

	ETALTML_RDS_DataNotification(hReceiver, slot, &TASwitchMode, FALSE, INVALID_AF, FALSE, FALSE);

	return ret;	
}


/**
* ETALTML_RDSEON_CopyEON
* @desc: This is used to copy EON 
* @param: slot
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDSEON_CopyEON(tSInt slot)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU8 i, j, flag;
	
	switch(RDSMgr[slot].EONInfo.EONMode)
	{
		case EON_MODE_IDLE:
			if(!RDSMgr[slot].EONInfo.F_InEONTA)
			{
				for(j = 0; j < EON_BUFF_SIZE; j++)
				{
					if(RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA  &&  (!RDSMgr[slot].EONInfo.EONCheck))
					{
						RDSMgr[slot].EONInfo.EONCheck = j;
						
						RDSMgr[slot].EONInfo.EONPI = RDSMgr[slot].EONInfo.EONSave_EONPI[j];
						RDSMgr[slot].EONInfo.EONAFNum = RDSMgr[slot].EONInfo.EONSave_EONAFNum[j];
						for(i = 0; i < MAX_AF_NUMBER; i++)
						{
							RDSMgr[slot].EONInfo.EONAFFreq[i] = RDSMgr[slot].EONInfo.EONSave_EONAF[j][i];
						}
						for(i = 0; i < 8; i++)
						{
							RDSMgr[slot].EONInfo.EONPS[i] = RDSMgr[slot].EONInfo.EONSave_EONPS[j][i];
						}
						RDSMgr[slot].EONInfo.EONTA = RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA;
						RDSMgr[slot].EONInfo.EONTP = RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTP;
						RDSMgr[slot].EONInfo.F_GetNewEONTA = TRUE;

						RDSMgr[slot].EONInfo.EONMode = EON_MODE_WAIT;
						break;
					}
				}
			}
			break;
			
		case EON_MODE_WAIT:
			if(!RDSMgr[slot].EONInfo.F_InEONTA)
			{
				if(!RDSMgr[slot].EONInfo.EONSave_Flags[RDSMgr[slot].EONInfo.EONCheck].EONSave_EONTA)
				{
					RDSMgr[slot].EONInfo.EONTA = FALSE;
					RDSMgr[slot].EONInfo.EONTP = RDSMgr[slot].EONInfo.EONSave_Flags[RDSMgr[slot].EONInfo.EONCheck].EONSave_EONTP;
					RDSMgr[slot].EONInfo.F_GetNewEONTA = TRUE;

					RDSMgr[slot].EONInfo.EONCheck = 0;
					RDSMgr[slot].EONInfo.EONMode = EON_MODE_IDLE;
					break;
				}
			}	
			break;
			
		case EON_MODE_OK:
			if(!RDSMgr[slot].EONInfo.F_InEONTA)
			{
				RDSMgr[slot].EONInfo.EONSave_Flags[RDSMgr[slot].EONInfo.EONCheck].EONSave_EONTA = FALSE;
				RDSMgr[slot].EONInfo.EONSave_Flags[RDSMgr[slot].EONInfo.EONCheck].EONSave_F_GetNewEONTA = FALSE;
				RDSMgr[slot].EONInfo.EONCheck = 0;
				RDSMgr[slot].EONInfo.EONMode = EON_MODE_IDLE;
			}
			break;
			
		case EON_MODE_NOT_FIND:
			RDSMgr[slot].EONInfo.EONMode = EON_MODE_NEXT;
			RDSMgr[slot].EONInfo.EONSave_EONCheckDisable[RDSMgr[slot].EONInfo.EONCheck] = 20;
			break;
			
		case EON_MODE_NEXT:
			if(!RDSMgr[slot].EONInfo.F_InEONTA)
			{
				flag = 0;
				for(j = RDSMgr[slot].EONInfo.EONCheck; j < EON_BUFF_SIZE; j++)
				{
					if(RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA && (!RDSMgr[slot].EONInfo.EONSave_EONCheckDisable[j]))
					{
						RDSMgr[slot].EONInfo.EONCheck = j;
						
						RDSMgr[slot].EONInfo.EONPI = RDSMgr[slot].EONInfo.EONSave_EONPI[j];
						RDSMgr[slot].EONInfo.EONAFNum = RDSMgr[slot].EONInfo.EONSave_EONAFNum[j];
						for(i = 0; i < MAX_AF_NUMBER; i++)
						{
							RDSMgr[slot].EONInfo.EONAFFreq[i] = RDSMgr[slot].EONInfo.EONSave_EONAF[j][i];
						}
						for(i = 0; i < 8; i++)
						{
							RDSMgr[slot].EONInfo.EONPS[i] = RDSMgr[slot].EONInfo.EONSave_EONPS[j][i];
						}
						RDSMgr[slot].EONInfo.EONTA = RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTA;
						RDSMgr[slot].EONInfo.EONTP = RDSMgr[slot].EONInfo.EONSave_Flags[j].EONSave_EONTP;
						RDSMgr[slot].EONInfo.F_GetNewEONTA = TRUE;

						RDSMgr[slot].EONInfo.EONMode = EON_MODE_WAIT;
						flag = 1;
						break;
					}
				}
				if(flag == 0)
				{
					RDSMgr[slot].EONInfo.EONCheck = 0;
					RDSMgr[slot].EONInfo.EONMode = EON_MODE_IDLE;
				}
			}
			break;
			
	}
	return ret;	
	
}

#endif


/**
* ETALTML_RDS_Strategy_Data_Init
* @desc: This is the routine to initilize the RDS data and RDS status information
* @param: hReceiver
* @return ETAL_STATUS : error code.
* @note:
*/
tVoid ETALTML_RDS_Strategy_Data_Init(ETAL_HANDLE hReceiver)
{
	tU32 currentFreq;
	tSInt i, slot;

	if (!RDSMgrLocksInit) return;


	if (!ETAL_receiverIsValidRDSHandle(hReceiver))
	{
		return;
	}
		
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (slot < 0 ) return;
	
	if (!RDSMgr[slot].FunctionInfo.F_RDSEnable) return;	//	if RDS Decoding is stopped, not run RDS strategy
	if (!RDSMgr[slot].FunctionInfo.F_AFEnable) return;
	
	//reset RDS deocode data in rds_data.c
	//ETALTML_RDSresetData(hReceiver);  //  CLEAR RDS DATA, reset
	
	RDSMgr[slot].Qual.MaxLevel  		= 0;	

	RDSMgr[slot].TATPInfo.F_GetNewTP 	= FALSE;
	RDSMgr[slot].TATPInfo.F_GetNewTA 	= FALSE;
	RDSMgr[slot].TATPInfo.BackupTP 	= FALSE;
	RDSMgr[slot].TATPInfo.BackupTA 	= FALSE;
	
	RDSMgr[slot].PIInfo.PISeekTimer 	= 0;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON	
	RDSMgr[slot].EONInfo.EONTA 		= FALSE;
	RDSMgr[slot].EONInfo.EONTALast 	= FALSE;
	RDSMgr[slot].EONInfo.EONTP 		= FALSE;
	RDSMgr[slot].EONInfo.EONTPLast 	= FALSE;
	RDSMgr[slot].EONInfo.F_GetNewEONTP = FALSE;
	RDSMgr[slot].EONInfo.F_GetNewEONTA = FALSE;
#endif

	RDSMgr[slot].AFInfo.F_RDSStationSlow = FALSE;
	
	if(RDSMgr[slot].ProcessFlag.F_PISeek) return;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	if(!RDSMgr[slot].EONInfo.F_InEONTA)
#endif
	{
		for(i = 1; i < MAX_AF_NUMBER; i++)
		{
			RDSMgr[slot].AFInfo.AFFreq[i] = INVALID_AF;
		}
		RDSMgr[slot].AFInfo.F_AFMethodB = FALSE;
		RDSMgr[slot].AFInfo.AFMethod[0] = INVALID_AF;
		RDSMgr[slot].AFInfo.AFMethod[1] = INVALID_AF;		
	}

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	for(i = 0; i < MAX_AF_NUMBER; i++)
	{
		RDSMgr[slot].EONInfo.EONAFFreq[i] = INVALID_AF;
	}
#endif

	RDSMgr[slot].AFInfo.F_AFMethodB = FALSE;
	RDSMgr[slot].AFInfo.AFMethod[0]  = INVALID_AF;
	RDSMgr[slot].AFInfo.AFMethod[1]  = INVALID_AF;		

       currentFreq = ETAL_receiverGetFrequency(hReceiver);

	if(FREQ_IN_NORMAL_FM(currentFreq))
	{
		RDSMgr[slot].AFInfo.AFFreq[0] = FREQ_KHZ_TO_AF(currentFreq);
		RDSMgr[slot].AFInfo.AFNum = 1;
	}
	else
	{
		RDSMgr[slot].AFInfo.AFFreq[0] = INVALID_AF;
		RDSMgr[slot].AFInfo.AFNum = 0;
	}

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	RDSMgr[slot].EONInfo.EONAFNum = 0;
	RDSMgr[slot].EONInfo.EONPI = 0;

	for(i = 0; i < EON_BUFF_SIZE; i++)
	{
		int j;
		RDSMgr[slot].EONInfo.EONSave_EONPI[i] = 0;
		RDSMgr[slot].EONInfo.EONSave_EONAFNum[i] = 0;
		for(j = 0; j < MAX_AF_NUMBER; j++)
		{
			RDSMgr[slot].EONInfo.EONSave_EONAF[i][j] = INVALID_AF;
		}
		
		for(j = 0; j < 8; j++)
		{
			RDSMgr[slot].EONInfo.EONSave_EONPS[i][j] = 0;
		}
	}
	
	RDSMgr[slot].EONInfo.EONCheck 	= 0;
	RDSMgr[slot].EONInfo.EONMode 		= EON_MODE_IDLE;
	RDSMgr[slot].EONInfo.F_NoMatchEON = FALSE;
	RDSMgr[slot].EONInfo.F_InEONTA 	= FALSE;
#endif
	RDSMgr[slot].AFInfo.F_GetNewAF 	= FALSE;
	RDSMgr[slot].AFInfo.F_GetAFList 		= FALSE;

}


/**
* ETALTML_RDS_Strategy_Init
* @desc This routine is to initialize the process of RDS
* @param: hReceiver,  RDS_FunctionInfoTy
* @return ETAL_STATUS :  error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_Strategy_Init(ETAL_HANDLE hReceiver, RDS_FunctionInfoTy RdsFuncPara)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt slot;

	if (!RDSMgrLocksInit)
	{
		RDSMgrLocksInit = TRUE;
		//ETALTML_RDS_Strategy_InitLock();
		
		for (slot = 0; slot < DABMW_RDS_SOURCE_MAX_NUM; slot++)
		{
			RDSMgr[slot].FunctionInfo.F_AFEnable 	= RdsFuncPara.F_AFEnable;
			RDSMgr[slot].FunctionInfo.F_TAEnable 	= RdsFuncPara.F_TAEnable;
			RDSMgr[slot].FunctionInfo.F_REGEnable 	= RdsFuncPara.F_REGEnable;
			RDSMgr[slot].FunctionInfo.F_PTYEnable	= RdsFuncPara.F_PTYEnable;
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
			RDSMgr[slot].FunctionInfo.F_EONEnable 	= RdsFuncPara.F_EONEnable;
#endif
			RDSMgr[slot].FunctionInfo.F_PIMuteEnable = TRUE;

			RDSMgr[slot].AFInfo.AFStrengthDelta 	= ETALTML_RDS_STRATEGY_AFSTRENGTH_DELTA;
			RDSMgr[slot].AFInfo.AFStrengthFast 	= ETALTML_RDS_STRATEGY_AFSTRENGTH_FAST;
			RDSMgr[slot].AFInfo.AFStrengthSlow 	= ETALTML_RDS_STRATEGY_AFSTRENGTH_SLOW;
			RDSMgr[slot].AFInfo.AFStrengthEON 	= ETALTML_RDS_STRATEGY_AFSTRENGTH_EON;
			RDSMgr[slot].AFInfo.AFCheckDisable1sTimerInitVal 				= ETALTML_RDS_STRATEGY_AFCHECK_DISABLE1STIMERINIT;
			RDSMgr[slot].AFInfo.AFCheckBetweenList1sTimerInitVal 			= ETALTML_RDS_STRATEGY_AFCHECK_BETWEENLIST1STIMERINIT;
			RDSMgr[slot].AFInfo.AFCheckBetweenFreq10msTimerInitVal 		= ETALTML_RDS_STRATEGY_AFCHECK_BETWEENFREQ10MSTIMERINIT;
			RDSMgr[slot].AFInfo.AFFastCheckBetweenList1sTimerInitVal 		= ETALTML_RDS_STRATEGY_AFFASTCHECK_BETWEENLIST1STIMERINIT;
			RDSMgr[slot].AFInfo.AFFastCheckBetweenFreq10msTimerInitVal 	= ETALTML_RDS_STRATEGY_AFFASTCHECK_BETWEENFREQ10MSTIMERINIT;
			RDSMgr[slot].AFInfo.AFDisable100msTimer 	= ETALTML_RDS_STRATEGY_AFDISABLE100MSTIMER;
			RDSMgr[slot].PIInfo.PISeek100msTimerInitVal 	= ETALTML_RDS_STRATEGY_PISEEK100MSTIMERINIT - ETALTML_RDS_STRATEGY_PISEEK_COMP_WAITTIME;
			RDSMgr[slot].AFInfo.F_RDSStationSlow  		= FALSE;
		}
		
		ETALTML_RDSresetData(hReceiver);     //CLEAR RDS DATA, reset
		ETALTML_RDS_Strategy_Data_Init(hReceiver);
	}
	else
	{
		//already done initiation, switch TA, etc.

		// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
		slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
		slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
		if (DABMW_INVALID_SLOT == slot) return ETAL_RET_ERROR;

		if ( RDSMgr[slot].FunctionInfo.F_AFEnable  &&  RdsFuncPara.F_AFEnable)
		{
			//break RDS AF Check
			(LINT_IGNORE_RET)ETALTML_RDS_Strategy_BreakAFCheck(hReceiver);
			
			if ( RdsFuncPara.F_TAEnable)
			{	//AF ON, Switch TA on
				DABMW_taTpDataTy * pTaTpData;
				ret = ETALTML_get_TATPStatus(hReceiver, (tVoid *) &pTaTpData);
				
				if ((ret == ETAL_RET_SUCCESS) &&
				    ((pTaTpData->newValue == DABMW_STORAGE_STATUS_IS_VERIFIED) ||
				     (pTaTpData->newValue == DABMW_STORAGE_STATUS_IS_USED) ))
				{
					if(pTaTpData->data.fields.taValue)
					{
						ETALTML_RDSTA_SwitchToTa(hReceiver, slot);
					}
				}
			}
			else
			{	//AF ON, swith TA off
				if(RDSMgr[slot].ProcessFlag.F_InTASwitch)
				{
					ETALTML_RDSTA_TaSwitchBack(hReceiver, slot);
				}
				
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
				if(RDSMgr[slot].ProcessFlag.F_InEONTASwitch)
				{
					ETALTML_RDSTA_EONTaSwitchBack(hReceiver, slot);
				}
#endif
			}
		}
		else if (!RdsFuncPara.F_AFEnable)
		{
			//reset the RDS receiving data buffer
			ETALTML_RDSresetData(hReceiver);  //  CLEAR RDS DATA, reset
		}

		RDSMgr[slot].FunctionInfo.F_AFEnable 	= RdsFuncPara.F_AFEnable;
		RDSMgr[slot].FunctionInfo.F_TAEnable 	= RdsFuncPara.F_TAEnable;
		RDSMgr[slot].FunctionInfo.F_REGEnable 	= RdsFuncPara.F_REGEnable;
		RDSMgr[slot].FunctionInfo.F_PTYEnable	= RdsFuncPara.F_PTYEnable;
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
		RDSMgr[slot].FunctionInfo.F_EONEnable 	= RdsFuncPara.F_EONEnable;
#endif
		
	}

	return ret;
}


/**
* ETALTML_RDS_Strategy_RadioIsBusy
* @desc This routine is main function of AF handling
* @param: hReceiver;  slot
* @return: tBool
* @note:
*/
tBool ETALTML_RDS_Strategy_RadioIsBusy(ETAL_HANDLE hReceiver)
{
	tBool bBusy;
	bBusy   = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualAFCheck);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualSeek);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeek);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialScan);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialLearn);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessEstimation);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialSeamlessSwitching);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialTune);
	if (!bBusy)  bBusy = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency);
	
	return bBusy;
}


/**
* ETALTML_RDSAF_MainProc
* @desc This routine is main function of AF handling
* @param: hReceiver;  slot
* @return: None
* @note:
*/
tVoid ETALTML_RDSAF_MainProc(ETAL_HANDLE hReceiver, tSInt slot)
{
	ETAL_STATUS ret;
	RDS_TunerQualInfoTy TunerCFQual, TunerAFQual;
	EtalBcastQualityContainer afQual;
	DABMW_storageStatusEnumTy PIStatus;
	tU32 currentPI, lastPI, backupPI, freq;
	tBool bRDSStation;
	static int notifyCnt = 0;

	notifyCnt ++;
				
	if(RDSMgr[slot].Qual.MaxLevel <= 4)
	{
		if(RDSMgr[slot].AFInfo.RDSStationDelayTimer == 0 && RDSMgr[slot].AFInfo.F_RDSStationSlow)
		{
			RDSMgr[slot].AFInfo.F_RDSStationSlow = FALSE;
			ETALTML_RDS_ClearTAFlags(slot);		
			
			//Notify
			ETALTML_RDS_DataNotification(hReceiver, slot, NULL, FALSE, INVALID_AF, FALSE, FALSE);
		}
	}
	else
	{
		RDSMgr[slot].AFInfo.RDSStationDelayTimer = 30; 	// 3s
		if (!RDSMgr[slot].AFInfo.F_RDSStationSlow)
		{
			//notify
			RDSMgr[slot].AFInfo.F_RDSStationSlow  = TRUE;
			ETALTML_RDS_DataNotification(hReceiver, slot, NULL, FALSE, INVALID_AF, FALSE, FALSE);
		}
	}

	if(RDSMgr[slot].FunctionInfo.F_AFEnable)
	{
		etalReceiverStatusTy *recvp = ETAL_receiverGet(hReceiver);

		if ( 	(recvp->isLearnInProgress) ||
			(recvp->isManualAFCheckInProgress) ||
			(recvp->isManualSeekInProgress) ||
			(recvp->isScanInProgress) ||
			(recvp->isSeamlessSwitchingInProgress) ||
			(recvp->isSeekInProgress) ||
			(recvp->isTuneInProgress) )		
		{
			RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_IDLE;
			return;
		}

		//RDS.AFInfo.AFSmeterCheckFreqTimer do not need data protection, only used in RDS strategy.
		if(RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer == 0)
		{
			if(RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_IDLE)
			{
				ret = ETALTML_RDSAF_ReadTunerCFQual(hReceiver, &TunerCFQual);
				if (ret == ETAL_RET_SUCCESS)
				{
					RDSMgr[slot].AFInfo.AFSmeter[0] += TunerCFQual.Smeter /3  - RDSMgr[slot].AFInfo.AFSmeter[0] /3;
				}
				
				PIStatus = DABMW_STORAGE_STATUS_IS_EMPTY;
				currentPI = 0;
				lastPI 	= 0;
				backupPI = 0;
				if (ETALTML_get_PIStatus(hReceiver, &PIStatus, &currentPI, &lastPI, &backupPI) == ETAL_RET_SUCCESS)
				{
					if (backupPI != 0) 	RDSMgr[slot].PIInfo.BackupPI = backupPI;
				}
			}
		}
	
		if(RDSMgr[slot].AFInfo.AFDisable100msTimer > 0)
		{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_3) 
			//ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "RDSMgr[slot].AFInfo.AFDisable100msTimer : %d,  RDSMgr[slot].AFInfo.AFCheckMode : %d, ETALTML_RDS_Strategy_BreakAFCheck. \r\n", RDSMgr[slot].AFInfo.AFDisable100msTimer, RDSMgr[slot].AFInfo.AFCheckMode);
#endif
			(LINT_IGNORE_RET)ETALTML_RDS_Strategy_BreakAFCheck(hReceiver);
		}
		else if( (RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_IDLE) &&
			     (RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer == 0))
		{
			RDSMgr[slot].AFInfo.AFSmeterCheckFreqTimer = 10;  		//100ms

			if(RDSMgr[slot].AFInfo.AFSmeter[0] > RDSMgr[slot].AFInfo.AFStrengthSlow)
			{
				if(RDSMgr[slot].AFInfo.AFSmeterLevel < 30 && RDSMgr[slot].AFInfo.AFSmeterLevel >= 20)
				{
					RDSMgr[slot].AFInfo.AFSmeterLevel ++;
				}
				else
				{
					RDSMgr[slot].AFInfo.AFSmeterLevel = 20;
				}
			}
			else if(RDSMgr[slot].AFInfo.AFSmeter[0] > RDSMgr[slot].AFInfo.AFStrengthFast)
			{
				if(RDSMgr[slot].AFInfo.AFSmeterLevel > 10)
				{
					RDSMgr[slot].AFInfo.AFSmeterLevel --;
				}
				else
				{
					RDSMgr[slot].AFInfo.AFSmeterLevel = 10;
				}
			}
			else
			{
				if(RDSMgr[slot].AFInfo.AFSmeterLevel > 0)
				{
					RDSMgr[slot].AFInfo.AFSmeterLevel --;
				}
			}
			
			if(RDSMgr[slot].AFInfo.AFSmeterLevel == 10 ||RDSMgr[slot].AFInfo.AFSmeterLevel == 0)
			{
				if(RDSMgr[slot].AFInfo.AFSmeterLevel == 0)
				{
					RDSMgr[slot].AFInfo.AFCheckStartTimerSet = RDSMgr[slot].AFInfo.AFFastCheckBetweenList1sTimerInitVal;
					RDSMgr[slot].AFInfo.AFCheckWaitTimerSet  = RDSMgr[slot].AFInfo.AFFastCheckBetweenFreq10msTimerInitVal;
					
					if(RDSMgr[slot].AFInfo.AFCheckStartTimer > RDSMgr[slot].AFInfo.AFCheckStartTimerSet)
					{
						RDSMgr[slot].AFInfo.AFCheckStartTimer = RDSMgr[slot].AFInfo.AFCheckStartTimerSet;
					}
					
					if(RDSMgr[slot].AFInfo.AFCheckWaitTimer > RDSMgr[slot].AFInfo.AFCheckWaitTimerSet)
					{
						RDSMgr[slot].AFInfo.AFCheckWaitTimer = RDSMgr[slot].AFInfo.AFCheckWaitTimerSet;	
					}
					
					RDSMgr[slot].AFInfo.AFStrengthDeltaReal = RDSMgr[slot].AFInfo.AFStrengthDelta;
				}
				else 
				{
					RDSMgr[slot].AFInfo.AFCheckStartTimerSet = RDSMgr[slot].AFInfo.AFCheckBetweenList1sTimerInitVal;
					RDSMgr[slot].AFInfo.AFCheckWaitTimerSet = RDSMgr[slot].AFInfo.AFCheckBetweenFreq10msTimerInitVal;					
					RDSMgr[slot].AFInfo.AFStrengthDeltaReal   = RDSMgr[slot].AFInfo.AFStrengthDelta * 2;
				}
				
				if(RDSMgr[slot].AFInfo.AFCheckMode == AFCHECK_MODE_IDLE)
				{
					if(RDSMgr[slot].AFInfo.AFCheckWaitTimerSet == 0xFF)
					{
						RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_SEARCHREQ;
					}
					else if(RDSMgr[slot].AFInfo.AFCheckStartTimer == 0)
					{
						RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_REQ;
					}
				}
			}
			else if(RDSMgr[slot].AFInfo.AFSmeterLevel == 30 && RDSMgr[slot].AFInfo.AFNum > 1)
			{
				if((!RDSMgr[slot].AFInfo.F_RDSStationSlow) && RDSMgr[slot].AFInfo.AFSearchWaitTimer == 0)
				{
					ETALTML_RDSAF_SetAFSearch(slot);
					RDSMgr[slot].AFInfo.AFSearchWaitTimer = 200;
				}
			}
		}
	}

	if (((RDSMgr[slot].AFInfo.AFCheckMode > AFCHECK_MODE_REQ) && (notifyCnt >= ETALTML_RDS_STRATEGY_NOTIFY_WAITTIME))
		&&
		// in wait mode to do notify : else we got a notif every 30 ms without purpose !!
		(RDSMgr[slot].AFInfo.AFCheckMode != AFCHECK_MODE_CHECKWAIT))
	{
		notifyCnt = 0;
		ETALTML_RDS_DataNotification(hReceiver, slot, NULL, FALSE, INVALID_AF, FALSE, FALSE);
	}
	
	switch(RDSMgr[slot].AFInfo.AFCheckMode)
	{
		case AFCHECK_MODE_IDLE:
			break;
			
		case AFCHECK_MODE_REQ:
			if(RDSMgr[slot].AFInfo.AFNum > 1)
			{
				RDSMgr[slot].AFInfo.AFCheckPoint = 1;
				
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECK;
			}
			else
			{
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_IDLE;
			}
			break;
			
		case AFCHECK_MODE_CHECK:
			if(RDSMgr[slot].AFInfo.AFDisable[RDSMgr[slot].AFInfo.AFCheckPoint] > 0)
			{
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKWAIT;
				break;
			}
			
			if (ETALTML_RDSAF_ReadTunerCFQual(hReceiver, &TunerCFQual) == ETAL_RET_SUCCESS)
			{
				RDSMgr[slot].AFInfo.AFSmeter[0] += TunerCFQual.Smeter /3 - RDSMgr[slot].AFInfo.AFSmeter[0] /3;
			}

			OSAL_s32ThreadWait(2);
			
			freq = FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[RDSMgr[slot].AFInfo.AFCheckPoint]);
			ret   = ETAL_AF_check_and_get_AF_quality(hReceiver, freq, 0, &afQual);
			
			if (ret == ETAL_RET_SUCCESS)
			{
				TunerAFQual.Smeter = afQual.EtalQualityEntries.amfm.m_BBFieldStrength;
				TunerAFQual.Detuning = afQual.EtalQualityEntries.amfm.m_FrequencyOffset;
				TunerAFQual.Multipath = afQual.EtalQualityEntries.amfm.m_Multipath;
				TunerAFQual.AdjChannel = afQual.EtalQualityEntries.amfm.m_UltrasonicNoise;		//Need double check,  m_AdjacentChannel
			}
			
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_CHECK, freq = %d, Smeter = %d, detuning = %d, Mp = %d, adj = %d\r\n", freq, TunerAFQual.Smeter, TunerAFQual.Detuning, TunerAFQual.Multipath, TunerAFQual.AdjChannel);
#endif				

			RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint] += TunerAFQual.Smeter /3 -RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint] /3;

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_CHECK, current freq - smeter: %d, AF(%d) - Smeter = %d, delta: %d\r\n", RDSMgr[slot].AFInfo.AFSmeter[0], freq, RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint], RDSMgr[slot].AFInfo.AFStrengthDeltaReal);
#endif				
								
			if((RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint] >= RDSMgr[slot].AFInfo.AFSmeter[0] + RDSMgr[slot].AFInfo.AFStrengthDeltaReal ) &&
			   (RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint] > RDSMgr[slot].AFInfo.AFStrengthFast))
			{				
				if (TunerAFQual.Smeter   > RDSMgr[slot].AFInfo.AFStrengthFast && 
				    TunerAFQual.Detuning < ETAL_CMOST_GET_AMFMMON_FM_DETUNING(ETALTML_RDS_STRATEGY_THRESHOLD_DETUNING) && 
				    TunerAFQual.Multipath < ETAL_CMOST_GET_AMFMMON_FM_MULTIPATH(ETALTML_RDS_STRATEGY_THRESHOLD_MP) && 
				    TunerAFQual.AdjChannel < ETAL_CMOST_GET_AMFMMON_FM_ADJACENT_CHANNEL(ETALTML_RDS_STRATEGY_THRESHOLD_ADJ))
				{
					if(RDSMgr[slot].FunctionInfo.F_PIMuteEnable)
					{
						ETALTML_RDS_Mute(hReceiver, SOFTON); 
					}
					
					OSAL_s32ThreadWait(7);

					ETALTML_RDS_SetFreq(hReceiver, RDSMgr[slot].AFInfo.AFFreq[RDSMgr[slot].AFInfo.AFCheckPoint]);
					RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKRDS;
					
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
					ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, " AFCHECK_MODE_CHECK,  AF station is good,  switch to AFCHECK_MODE_CHECKRDS \r\n ");
#endif				
					
					if(RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint] > RDSMgr[slot].AFInfo.AFStrengthSlow)
					{
						RDSMgr[slot].AFInfo.AFCheckWaitTimer = 60;
					}
					else
					{
						RDSMgr[slot].AFInfo.AFCheckWaitTimer = 120;
					}
					
					// Wait for rds stable, need be careful for the timing
					//RDSMgr[slot].AFInfo.AFCheckWaitTimer = 150;
					if(RDSMgr[slot].FunctionInfo.F_PIMuteEnable)
					{
						ETALTML_RDS_Mute(hReceiver, SOFTOFF); 
					}
				}
				else 
				{					
					ETALTML_RDSAF_AFCheckBack(hReceiver, slot, FALSE);
				}
			}
			else
			{
				ETALTML_RDSAF_AFCheckBack(hReceiver, slot, FALSE);
			}
			break;

		case AFCHECK_MODE_CHECKRDS:
			bRDSStation = FALSE;
			ETALTML_get_RDSStationFlag(hReceiver, &bRDSStation);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_CHECKRDS. rds station : %d\r\n", bRDSStation);
#endif
			if (bRDSStation)
			{
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKPI;
				
				if(RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint] > RDSMgr[slot].AFInfo.AFStrengthSlow)
				{
					RDSMgr[slot].AFInfo.AFCheckWaitTimer = 90;
				}
				else
				{
					RDSMgr[slot].AFInfo.AFCheckWaitTimer = 150;
				}				

				// Wait for rds stable, need be careful for the timing
				//RDSMgr[slot].AFInfo.AFCheckWaitTimer = 150;
				break;
			}
		
			if(RDSMgr[slot].AFInfo.AFCheckWaitTimer == 0)
			{
				RDSMgr[slot].AFInfo.AFDisable[RDSMgr[slot].AFInfo.AFCheckPoint] = RDSMgr[slot].AFInfo.AFCheckDisable1sTimerInitVal;
					
				if(!RDSMgr[slot].FunctionInfo.F_PIMuteEnable)
				{
					ETALTML_RDS_Mute(hReceiver, SOFTON); 
				}
				
				ETALTML_RDSAF_AFCheckBack(hReceiver, slot, TRUE);
			}
			break;
			
		case AFCHECK_MODE_CHECKPI:
			PIStatus = DABMW_STORAGE_STATUS_IS_EMPTY;
			currentPI = 0;
			lastPI 	= 0;
			backupPI = 0;
			
			ret = ETALTML_get_PIStatus(hReceiver, &PIStatus, &currentPI, &lastPI, &backupPI);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_CHECKPI, currentPI : %x, backupPI: %x  lastPI:%x \r\n", currentPI, RDSMgr[slot].PIInfo.BackupPI, lastPI);
#endif		
			if ( (ETAL_RET_SUCCESS == ret)  && (currentPI != 0))
			{
				// PI have been decoded
				if (  ((currentPI == RDSMgr[slot].PIInfo.BackupPI) && RDSMgr[slot].FunctionInfo.F_REGEnable) ||
			         (((currentPI & 0xFF) == (RDSMgr[slot].PIInfo.BackupPI  & 0xFF)) && (!RDSMgr[slot].FunctionInfo.F_REGEnable))  )
				{
					// PI is matching
					ETALTML_RDS_Mute(hReceiver, SOFTOFF);
					ETALTML_RDSAF_AFChange(slot, 0, RDSMgr[slot].AFInfo.AFCheckPoint);
					RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKEND;
					RDSMgr[slot].AFInfo.AFCheckStartTimerSet = RDSMgr[slot].AFInfo.AFCheckBetweenList1sTimerInitVal;
					RDSMgr[slot].AFInfo.AFDisable100msTimer = ETALTML_RDS_STRATEGY_AFDISABLE100MSTIMER;

					ETALTML_RDS_DataNotification(hReceiver, slot, NULL, TRUE, RDSMgr[slot].AFInfo.AFFreq[0], FALSE, FALSE);
					break;
				}
				else
				{
					// PI is not matching, 
					// back to prior station
					//
					RDSMgr[slot].AFInfo.AFDisable[RDSMgr[slot].AFInfo.AFCheckPoint] = RDSMgr[slot].AFInfo.AFCheckDisable1sTimerInitVal;
						
					if(!RDSMgr[slot].FunctionInfo.F_PIMuteEnable)
					{
						ETALTML_RDS_Mute(hReceiver, SOFTON); 
					}
					ETALTML_RDSAF_AFCheckBack(hReceiver, slot, TRUE);
				}
			}
			else
			{
				// PI not decoded
				if(RDSMgr[slot].AFInfo.AFCheckWaitTimer == 0)
				{
					RDSMgr[slot].AFInfo.AFDisable[RDSMgr[slot].AFInfo.AFCheckPoint] = RDSMgr[slot].AFInfo.AFCheckDisable1sTimerInitVal;
						
					if(!RDSMgr[slot].FunctionInfo.F_PIMuteEnable)
					{
						ETALTML_RDS_Mute(hReceiver, SOFTON); 
					}
					ETALTML_RDSAF_AFCheckBack(hReceiver, slot, TRUE);
				}
			}

		
			break;
			
		case AFCHECK_MODE_CHECKWAIT:

			if(RDSMgr[slot].AFInfo.AFCheckWaitTimer > 0) 
			{
				break;
			}

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_CHECKWAIT. \r\n");
#endif
			if(RDSMgr[slot].AFInfo.AFCheckPoint < RDSMgr[slot].AFInfo.AFNum - 1 &&  RDSMgr[slot].AFInfo.AFNum > 1)
			{
				RDSMgr[slot].AFInfo.AFCheckPoint ++;
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECK;
			}
			else
			{
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKEND;
			}

			break;

		case AFCHECK_MODE_CHECKEND:
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_CHECKEND. \r\n");
#endif
			if(RDSMgr[slot].AFInfo.AFNum > 1)
			{
				ETALTML_RDSAF_AFSort(slot, 1);
				if(RDSMgr[slot].AFInfo.AFSearchWaitTimer < 30)
				{
					RDSMgr[slot].AFInfo.AFSearchWaitTimer = 30;
				}
			}
			RDSMgr[slot].AFInfo.AFCheckStartTimer = RDSMgr[slot].AFInfo.AFCheckStartTimerSet;
			RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_IDLE;
			break;

		case AFCHECK_MODE_SEARCHREQ:
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_SEARCHREQ. \r\n");
#endif

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
			if(RDSMgr[slot].AFInfo.AFNum > 1 || (RDSMgr[slot].EONInfo.F_InEONTA))
#else
			if(RDSMgr[slot].AFInfo.AFNum > 1 )
#endif
			{
				RDSMgr[slot].AFInfo.AFCheckPoint = 1;

				ETALTML_RDS_Mute(hReceiver, SOFTON);
				RDSMgr[slot].AFInfo.AFFreqBackup = RDSMgr[slot].AFInfo.AFFreq[0];

				for(RDSMgr[slot].AFInfo.AFCheckPoint = 0; RDSMgr[slot].AFInfo.AFCheckPoint < RDSMgr[slot].AFInfo.AFNum; RDSMgr[slot].AFInfo.AFCheckPoint++)
				{
					freq = FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[RDSMgr[slot].AFInfo.AFCheckPoint]);
					ret = ETAL_AF_start(hReceiver, cmdNormalMeasurement, freq, 0, &afQual);
					if (ret == ETAL_RET_SUCCESS)
					{
						ETALTML_RDS_AFCheckInit(hReceiver);		//  CLEAR RDS DATA, reset rds data
						
						if(RDSMgr[slot].AFInfo.AFCheckPoint > 0)
						{
							RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint - 1] = afQual.EtalQualityEntries.amfm.m_BBFieldStrength;							
						}
						OSAL_s32ThreadWait(ETALTML_RDS_STRATEGY_AFSTART_WAITTIME); 		
	 				}
				}
				
				ETALTML_RDSAF_ReadTunerAFQual(hReceiver, &TunerAFQual);
				if(RDSMgr[slot].AFInfo.AFCheckPoint > 0)	 RDSMgr[slot].AFInfo.AFSmeter[RDSMgr[slot].AFInfo.AFCheckPoint - 1] = TunerAFQual.Smeter;

				ETALTML_RDSAF_AFSort(slot, 0);

#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
				{
					int i;
					ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_SEARCHREQ, after AF signal sort. \r\n");
					ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "		frequency	smeter\r\n");					
					for (i = 0; i < RDSMgr[slot].AFInfo.AFNum; i++)
					{
						ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "		%d		%d\r\n", FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[i]), RDSMgr[slot].AFInfo.AFSmeter[i]);
					}
				}
#endif
				RDSMgr[slot].AFInfo.AFCheckPoint = 0;
				ETALTML_RDSAF_AFSearchNext(hReceiver, slot, TRUE);

			}
			else
			{
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_IDLE;
			}
			
			break;

		case AFCHECK_MODE_SEARCHRDS:
			bRDSStation = FALSE;
			ETALTML_get_RDSStationFlag(hReceiver, &bRDSStation);
			
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_SEARCHRDS. freq: %d   RDSSTATION: %d \r\n", FREQ_AF_TO_KHZ(RDSMgr[slot].AFInfo.AFFreq[RDSMgr[slot].AFInfo.AFCheckPoint]), bRDSStation);
#endif			
			if (bRDSStation)			
			{
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_SEARCHPI;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
				if(RDSMgr[slot].EONInfo.F_InEONTA)
				{
					RDSMgr[slot].AFInfo.AFCheckWaitTimer = 150;
				}
				else
#endif						
				{
					RDSMgr[slot].AFInfo.AFCheckWaitTimer = 250;
				}
				
				break;
			}
			
			if(RDSMgr[slot].AFInfo.AFCheckWaitTimer == 0)
			{
				RDSMgr[slot].AFInfo.AFCheckPoint++;
				ETALTML_RDSAF_AFSearchNext(hReceiver, slot, FALSE);
			}
			
			break;

		case AFCHECK_MODE_SEARCHPI:
			PIStatus  = DABMW_STORAGE_STATUS_IS_EMPTY;
			currentPI = 0;
			lastPI 	= 0;
			backupPI = 0;
			
			ret = ETALTML_get_PIStatus(hReceiver, &PIStatus, &currentPI, &lastPI, &backupPI);
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_SEARCHPI.  RDSMgr[slot].PIInfo.BackupPI : %x, currentPI :%x\r\n", RDSMgr[slot].PIInfo.BackupPI, currentPI);
#endif		
			if( (ret == ETAL_RET_SUCCESS) && 
			     (currentPI != 0) &&
			     ( ((currentPI == RDSMgr[slot].PIInfo.BackupPI) && RDSMgr[slot].FunctionInfo.F_REGEnable) ||
			        (((currentPI & 0xFF) == (RDSMgr[slot].PIInfo.BackupPI & 0xFF)) && (!RDSMgr[slot].FunctionInfo.F_REGEnable)) ))
			{
#if defined(CONFIG_TRACE_CLASS_ETAL) && (CONFIG_TRACE_CLASS_RDS_STRATEGY >= TR_LEVEL_USER_2) 
				ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM_MIN, "AFCHECK_MODE_SEARCHPI. PI : %x, matched.\r\n", currentPI);
#endif
				ETALTML_RDS_Mute(hReceiver, SOFTOFF);
				ETALTML_RDSAF_AFChange(slot, 0, RDSMgr[slot].AFInfo.AFCheckPoint);
				RDSMgr[slot].AFInfo.AFCheckMode = AFCHECK_MODE_CHECKEND;
				RDSMgr[slot].AFInfo.AFCheckStartTimerSet = RDSMgr[slot].AFInfo.AFCheckBetweenList1sTimerInitVal;
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
				if(RDSMgr[slot].EONInfo.F_InEONTA)
				{
					RDSMgr[slot].EONInfo.EONMode = EON_MODE_OK;
				}
#endif
				ETALTML_RDS_DataNotification(hReceiver, slot, NULL, TRUE, RDSMgr[slot].AFInfo.AFFreq[0], FALSE, FALSE);

				break;
			}
			
			if(RDSMgr[slot].AFInfo.AFCheckWaitTimer == 0)
			{
				RDSMgr[slot].AFInfo.AFCheckPoint++;
				ETALTML_RDSAF_AFSearchNext(hReceiver, slot, FALSE);
			}

			break;
			
	}
	
}


/**
* RDS_Main
* @desc This is the main function for RDS processsing
* @param: hReceiver
* @return: none
* @note:
*/
tVoid ETALTML_RDS_Strategy_Main(ETAL_HANDLE hReceiver)
{
	tSInt slot;
	ETAL_STATUS ret;

	if (!ETAL_receiverIsValidHandle(hReceiver)) return;
	if (!ETAL_receiverIsValidRDSHandle(hReceiver)) return;

	// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (DABMW_INVALID_SLOT == slot) return;
	if (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM ) return;


	
	//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML_RDS_Strategy_Main %d \n", hReceiver);
	
	ETALTML_RDS_Strategy_Timer_hdr(slot);

	if (!RDSMgr[slot].FunctionInfo.F_RDSEnable) return;	//	if RDS Decoding is stopped, not run RDS strategy
	if (!RDSMgr[slot].FunctionInfo.F_AFEnable)    return;	//	RDS.FunctionInfo.F_AFEnable works as a whole switch, Maybe in future, the RDS seek will not depend on switch F_AFEanble.

	ETALTML_RDSAF_MainProc(hReceiver, slot);
	
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	// EON
 	if(RDSMgr[slot].FunctionInfo.F_TAEnable  && RDSMgr[slot].FunctionInfo.F_EONEnable && (!RDSMgr[slot].ProcessFlag.F_InTASwitch))
	{
		// EONTA switch
		ETALTML_RDSEON_CopyEON(slot);
 		if(RDSMgr[slot].EONInfo.F_GetNewEONTA && RDSMgr[slot].EONInfo.EONTA  &&  (!RDSMgr[slot].ProcessFlag.F_InEONTASwitch) &&
		   RDSMgr[slot].FuncTimer.InTADelayTimer == 0  &&  RDSMgr[slot].FuncTimer.EnterEONTADelayTimer == 0)
		{
			RDSMgr[slot].FuncTimer.EnterTADelayTimer = 0;
			ETALTML_RDSTA_SwitchToEONTa(hReceiver, slot);
		}

 		if(RDSMgr[slot].ProcessFlag.F_InEONTASwitch  &&  RDSMgr[slot].EONInfo.F_NoMatchEON )
		{
			ETALTML_RDSTA_EONTaSwitchBack(hReceiver, slot);
			RDSMgr[slot].FuncTimer.EnterEONTADelayTimer = 150;
		}

 		if(RDSMgr[slot].ProcessFlag.F_InEONTASwitch  &&  RDSMgr[slot].FuncTimer.InTADelayTimer == 0)
		{
			DABMW_taTpDataTy *pTaTpData;
			tBool 	bRDSStation = FALSE;
			ret = ETALTML_get_RDSStationFlag(hReceiver, &bRDSStation);

			if ((ret == ETAL_RET_SUCCESS) && (!bRDSStation))
			{
				if(RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer == 0)
				{
					ETALTML_RDSTA_EONTaSwitchBack(hReceiver, slot);
				}
			}
			else
			{
				ret = ETALTML_get_TATPStatus(hReceiver, (tVoid *) &pTaTpData);
				if ((ret == ETAL_RET_SUCCESS) &&
				    ((pTaTpData->newValue == DABMW_STORAGE_STATUS_IS_VERIFIED) ||
				     (pTaTpData->newValue == DABMW_STORAGE_STATUS_IS_USED) ))
				{
					if ((!pTaTpData->data.fields.taValue) && (RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer == 0))
					{
						ETALTML_RDSTA_EONTaSwitchBack(hReceiver, slot);
					}
				}
				else
				{
					RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer = 50;
				}
			}
		}
	}
#endif

	if((RDSMgr[slot].FuncTimer.EnterTADelayTimer == 1) && (!RDSMgr[slot].FunctionInfo.F_TAEnable))
	{
		RDSMgr[slot].FuncTimer.EnterTADelayTimer = 0;
	}

 	if(RDSMgr[slot].FunctionInfo.F_TAEnable && (!RDSMgr[slot].ProcessFlag.F_InEONTASwitch))
	{
		// TA switch
		DABMW_taTpDataTy * pTaTpData;
		
		ret = ETALTML_get_TATPStatus(hReceiver, (tVoid *) &pTaTpData);
		
		if ((ret == ETAL_RET_SUCCESS) &&
		    ((pTaTpData->newValue == DABMW_STORAGE_STATUS_IS_VERIFIED) ||
		     (pTaTpData->newValue == DABMW_STORAGE_STATUS_IS_USED) ))
		{
			if (RDSMgr[slot].TATPInfo.BackupTP != (tU8) (pTaTpData->data.fields.tpValue))
			{
				RDSMgr[slot].TATPInfo.F_GetNewTP = TRUE;
				RDSMgr[slot].TATPInfo.BackupTP = (tU8) (pTaTpData->data.fields.tpValue);
			}
			
			if (RDSMgr[slot].TATPInfo.BackupTA != (tU8) (pTaTpData->data.fields.taValue))
			{
				RDSMgr[slot].TATPInfo.F_GetNewTA = TRUE;
				RDSMgr[slot].TATPInfo.BackupTA = (tU8) (pTaTpData->data.fields.taValue);
			}
		}
		
		if(RDSMgr[slot].TATPInfo.F_GetNewTA || RDSMgr[slot].TATPInfo.F_GetNewTP)
		{
			RDSMgr[slot].TATPInfo.F_GetNewTA = FALSE;
			RDSMgr[slot].TATPInfo.F_GetNewTP = FALSE;

 			if (pTaTpData->data.fields.tpValue  &&  pTaTpData->data.fields.taValue)
			{
				RDSMgr[slot].FuncTimer.EnterTADelayTimer = 30;
			}
			else
			{
				//TA TP finished, switch back
				ETALTML_RDSTA_TaSwitchBack(hReceiver, slot); 
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
				if(RDSMgr[slot].EONInfo.EONTA)
				{
					RDSMgr[slot].FuncTimer.EnterEONTADelayTimer = 30;
				}
#endif				
			}
		}

		if(RDSMgr[slot].FuncTimer.EnterTADelayTimer == 1)
		{
			RDSMgr[slot].FuncTimer.EnterTADelayTimer = 0;
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
 			if((!RDSMgr[slot].EONInfo.EONTA) && pTaTpData->data.fields.tpValue  && pTaTpData->data.fields.taValue)
#else
 			if (pTaTpData->data.fields.tpValue  &&  pTaTpData->data.fields.taValue)
#endif
			{
				ETALTML_RDSTA_SwitchToTa(hReceiver, slot);
			}
		}

		if(RDSMgr[slot].ProcessFlag.F_InTASwitch && RDSMgr[slot].FuncTimer.InTADelayTimer == 0)
		{
			if(RDSMgr[slot].Qual.MaxLevel <= 4)
			{
				if(RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer == 0)
				{
					ETALTML_RDSTA_TaSwitchBack(hReceiver, slot);

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
					if(RDSMgr[slot].EONInfo.EONTA )
					{
						RDSMgr[slot].FuncTimer.EnterEONTADelayTimer = 30;
					}
#endif					
				}
			}
			else
			{
				RDSMgr[slot].FuncTimer.TAWaitAFCheckTimer = 100;
			}
		}

		// Auto TA seek
 		if (pTaTpData->data.fields.tpValue  || pTaTpData->data.fields.taValue)
		{
			RDSMgr[slot].FuncTimer.AutoTASeekTimer = 0;
		}
		else
		{
			if(RDSMgr[slot].FuncTimer.AutoTASeekTimer == 1 &&  (!ETALTML_RDS_Strategy_RadioIsBusy(hReceiver)) )
			{
				EtalRDSSeekTy rdsSeekOption;
				memset(& rdsSeekOption, 0x00, sizeof(EtalRDSSeekTy));
				
				rdsSeekOption.taSeek = TRUE;
				ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM, "ETALTML_RDS_Strategy_Main : request ETALTML_rds_seek_start for TA\n");
				

				ret = ETALTML_rds_seek_start(hReceiver, cmdDirectionUp, ETALTML_RDS_STRATEGY_FM_SEEK_STEP, cmdAudioUnmuted, rdsSeekOption);
				if (ret == ETAL_RET_SUCCESS)
				{
					ETALTML_RDS_DataNotification(hReceiver, slot, NULL, FALSE, INVALID_AF, TRUE, FALSE);
				}
			}
			
			if(RDSMgr[slot].FuncTimer.AutoTASeekTimer == 0 || RDSMgr[slot].FuncTimer.AutoTASeekTimer == 1)
			{
				RDSMgr[slot].FuncTimer.AutoTASeekTimer = ETALTML_RDS_STRATEGY_TASEEK100MSTIMERINIT - ETALTML_RDS_STRATEGY_TASEEK_COMP_WAITTIME;
			}
		}
	}

	// PI SEEK
	if((RDSMgr[slot].PIInfo.PISeekTimer == 1) && (!ETALTML_RDS_Strategy_RadioIsBusy(hReceiver)) )
	{
		RDSMgr[slot].PIInfo.PISeekTimer = 0;
		if(RDSMgr[slot].FunctionInfo.F_AFEnable && RDSMgr[slot].PIInfo.BackupPI)
		{
			EtalRDSSeekTy rdsSeekOption;
			memset(& rdsSeekOption, 0x00, sizeof(EtalRDSSeekTy));
			
			rdsSeekOption.piSeek = TRUE;
			ETALTML_RDS_STRATEGY_PRINTF(TR_LEVEL_SYSTEM, "ETALTML_RDS_Strategy_Main : request ETALTML_rds_seek_start for PI\n");
 
			ret = ETALTML_rds_seek_start(hReceiver, cmdDirectionUp, ETALTML_RDS_STRATEGY_FM_SEEK_STEP, cmdAudioUnmuted, rdsSeekOption);
			if (ret == ETAL_RET_SUCCESS)
			{
				RDSMgr[slot].ProcessFlag.F_PISeek = TRUE;

				ETALTML_RDS_DataNotification(hReceiver, slot, NULL, FALSE, INVALID_AF, FALSE, TRUE);
			}
		}
	}

	if(RDSMgr[slot].FuncTimer.PTYSelectTimer == 1)
	{
		RDSMgr[slot].FuncTimer.PTYSelectTimer = 0;
		RDSMgr[slot].FunctionInfo.F_PTYEnable = 0;
		RDSMgr[slot].ProcessFlag.F_NoPTY = 0;
		
		ETALTML_RDS_DataNotification(hReceiver, slot, NULL, FALSE, INVALID_AF, FALSE, FALSE);
	}

	//ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETALTML_RDS_Strategy_Main done\n");
}


/**
* ETALTML_RDS_Strategy_AFSearch_Start
* @desc This routine is used to start AF search
* @param: ETAL_HANDLE hReceiver; EtalRDSAFSearchData afSearchData (PI, AFNum, AFlist)
* @return ETAL_STATUS : RDS error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_Strategy_AFSearch_Start(ETAL_HANDLE hReceiver, EtalRDSAFSearchData afSearchData)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt slot;
	tU32 i;

	// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (DABMW_INVALID_SLOT == slot) return ETAL_RET_ERROR;
	if (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM ) return ETAL_RET_ERROR;
	
	//restore PI,  restore AF list
	RDSMgr[slot].AFInfo.AFNum = afSearchData.m_AFListLen;
	for (i = 0; i < afSearchData.m_AFListLen; i++)
	{
		RDSMgr[slot].AFInfo.AFFreq[i] = afSearchData.m_AFList[i];
		RDSMgr[slot].AFInfo.AFSmeter[i] = 0;
	}

	//Break AF check first
	(LINT_IGNORE_RET)ETALTML_RDS_Strategy_BreakAFCheck(hReceiver);
	RDSMgr[slot].PIInfo.PISeekTimer = RDSMgr[slot].PIInfo.PISeek100msTimerInitVal;
	
	//start AF Search
	ret = ETALTML_RDSAF_SetAFSearch(slot);
	
	return ret;	
}


/**
* ETALTML_RDS_Strategy_AFSearch_Stop
* @desc This routine is used to stop AF search
* @param: ETAL_HANDLE hReceiver;
* @return ETAL_STATUS : RDS error code.
* @note:
*/
ETAL_STATUS ETALTML_RDS_Strategy_AFSearch_Stop(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tSInt slot;

	// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot(hReceiver);
#else
	slot = DABMW_RdsGetSlotFromSource ((DABMW_storageSourceTy)hReceiver);
#endif
	if (DABMW_INVALID_SLOT == slot) return ETAL_RET_ERROR;
	if (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM ) return ETAL_RET_ERROR;

	ret = ETALTML_RDSAF_BreakAFSearch(hReceiver);
	
	return ret;
}

#endif // CONFIG_ETALTML_HAVE_RDSSTRATEGY

// End of file


