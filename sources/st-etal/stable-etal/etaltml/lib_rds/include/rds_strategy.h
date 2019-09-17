//!  \file      rds_strategy.h
//!  \brief     <i><b> RDS strategy header </b></i>
//!  \details   RDS strategy related management header
//!  \author    SZ Tuner Team
//!  \version   1.0
//!  \date      2016.05.28
//!  \bug       Unknown
//!  \warning   None
//!

#ifndef _RDS_STRATEGY_H_
#define _RDS_STRATEGY_H_

#ifdef __cplusplus
extern "C" {
#endif


#define ETALTML_RDS_STRATEGY_AFSTRENGTH_DELTA 							4
#define ETALTML_RDS_STRATEGY_AFSTRENGTH_FAST							20	/*190 */
#define ETALTML_RDS_STRATEGY_AFSTRENGTH_SLOW 							40	/*216*/
#define ETALTML_RDS_STRATEGY_AFSTRENGTH_EON 							22	/*190*/
#define ETALTML_RDS_STRATEGY_AFCHECK_DISABLE1STIMERINIT 				(90 - 15)
#define ETALTML_RDS_STRATEGY_AFCHECK_BETWEENLIST1STIMERINIT			10
#define ETALTML_RDS_STRATEGY_AFCHECK_BETWEENFREQ10MSTIMERINIT		100
#define ETALTML_RDS_STRATEGY_AFFASTCHECK_BETWEENLIST1STIMERINIT		1
#define ETALTML_RDS_STRATEGY_AFFASTCHECK_BETWEENFREQ10MSTIMERINIT	10
#define ETALTML_RDS_STRATEGY_AFDISABLE100MSTIMER						50
#define ETALTML_RDS_STRATEGY_PISEEK100MSTIMERINIT						600
#define ETALTML_RDS_STRATEGY_TASEEK100MSTIMERINIT						600
#define ETALTML_RDS_STRATEGY_AFSTART_WAITTIME							18	/*18 ms*/

/*Thresholds for RDS seek, reference table FMSeekNoiseThreshold in Etaltml_thresholds.h */
#define ETALTML_RDS_STRATEGY_THRESHOLD_DETUNING						0x15	/*depend on the measurement time of rds detector*/	
#define ETALTML_RDS_STRATEGY_THRESHOLD_MP								0x40
#define ETALTML_RDS_STRATEGY_THRESHOLD_ADJ								0xA0


#define MAX_AF_NUMBER														25
#define EON_BUFF_SIZE														15

// Task Define

// we need to have a non empty flag for waiting else it is failing in t-kernel at least
#define ETALTML_RDS_SEEK_NO_EVENT_FLAG        0x00

#define ETALTML_RDS_SEEK_EVENT_SLEEP			30
#define ETALTML_RDS_SEEK_EVENT_SLEEP_FLAG		((tU32)0x01 << ETALTML_RDS_SEEK_EVENT_SLEEP)

#define ETALTML_RDS_SEEK_EVENT_FLAGS          ETALTML_RDS_SEEK_EVENT_SLEEP_FLAG
#define ETALTML_RDS_SEEK_EVENT_WAIT_MASK      (ETALTML_RDS_SEEK_EVENT_FLAGS)

// Kill event
#define ETALTML_RDS_SEEK_EVENT_KILL					0
// WAKEUP EVENT
#define ETALTML_RDS_SEEK_EVENT_WAKE_UP					31

#define ETALTML_RDS_SEEK_EVENT_KILL_FLAG				((tU32)0x01 << ETALTML_RDS_SEEK_EVENT_KILL)
#define ETALTML_RDS_SEEK_EVENT_WAKEUP_FLAG				((tU32)0x01 << ETALTML_RDS_SEEK_EVENT_WAKE_UP)

#define ETALTML_RDS_SEEK_WAKEUP_FLAGS					(ETALTML_RDS_SEEK_EVENT_KILL_FLAG | ETALTML_RDS_SEEK_EVENT_WAKEUP_FLAG)
#define ETALTML_RDS_SEEK_EVENT_WAIT_WAKEUP_MASK		(ETALTML_RDS_SEEK_WAKEUP_FLAGS)

#define ETALTML_RDS_SEEK_EVENT_ALL_FLAG				(0xFFFFFFFF)

#define ETALTML_RDS_SEEK_THREAD_PRIORITY      OSAL_C_U32_THREAD_PRIORITY_NORMAL
#define ETALTML_RDS_SEEK_STACK_SIZE           4096
#define ETALTML_RDS_SEEK_EVENT_WAIT_TIME_MS   10U



typedef enum
{
	AFCHECK_MODE_IDLE,
	AFCHECK_MODE_REQ,
	AFCHECK_MODE_CHECK,
	AFCHECK_MODE_CHECKWAIT,
	AFCHECK_MODE_CHECKEND,
	AFCHECK_MODE_CHECKRDS,
	AFCHECK_MODE_CHECKPI,
	AFCHECK_MODE_SEARCHREQ,
	AFCHECK_MODE_SEARCHRDS,
	AFCHECK_MODE_SEARCHPI
}RDS_AFModeInfoTy;


typedef enum
{
	EON_MODE_IDLE,
	EON_MODE_WAIT,
	EON_MODE_OK,
	EON_MODE_NOT_FIND,
	EON_MODE_NEXT,
}RDS_EONModeInfoTy;


typedef enum
{
	RDS_TIMER_10MS,			/* RDS_Timer.Timer10ms */
	RDS_TIMER_100MS,			/* RDS_Timer.Timer100ms */
	RDS_TIMER_FUNC_AUTO_TA_SEEK,	/* RDS.FuncTimer.AutoTASeekTimer */
	RDS_TIMER_FUNC_PTY_SELECT,		/* RDS.FuncTimer.PTYSelectTimer */
	RDS_TIMER_FUNC_IN_TA_DELAY,		/* RDS.FuncTimer.InTADelayTimer */
	RDS_TIMER_FUNC_ENTER_TA_DELAY,	/* RDS.FuncTimer.EnterTADelayTimer */
	RDS_TIMER_FUNC_ENTER_EONTA_DELAY,  /* RDS.FuncTimer.EnterEONTADelayTimer */
	RDS_TIMER_FUNC_TA_WAIT_AF_CHECK,    /* RDS.FuncTimer.TAWaitAFCheckTimer */
	RDS_TIMER_AFINFO_AF,				 /* RDS.AFInfo.AFTimer */
	RDS_TIMER_AFINFO_AF_SMETER_CHECKF_REQ,  /* RDS.AFInfo.AFSmeterCheckFreqTimer  */
	RDS_TIMER_AFINFO_AF_CHECK,  		/* RDS.AFInfo.AFCheckWaitTimer  */
	RDS_TIMER_AFINFO_AF_DISABLE_100MS,	/* RDS.AFInfo.AFDisable100msTimer */
	RDS_TIMER_AFINFO_AF_SEARCH,     /* RDS.AFInfo.AFSearchWaitTimer */
	RDS_TIMER_AFINFO_RDSSTATION_DELAY,    /* RDS.AFInfo.RDSStationDelayTimer */
	RDS_TIMER_AFINFO_RDSSTATION_FAST_DELAY,    /* For RDS STATION FAST INDICATE, EVENT FOR DISPLAY FOR RTA*/
	RDS_TIMER_PIINFO_PISEEK,		/* RDS.PIInfo.PISeekTimer  */
	RDS_TIMER_EONINFO_EONAF,		/* RDS.EONInfo.EONAFTimer */
	RDS_TIMER_EONINFO_EONEXIT,    /* RDS.EONInfo.EONExitTimer */
}RDS_TimerType;


typedef enum
{
	SWITCH_TO_TUNER,
	SWITCHBACK_FROM_TUNER		
}RDS_TASwitchSystemMode;


/*
	0 - RDS_LOCK_TIMER - used for RDS timer get/set
*/
typedef enum
{
	RDS_LOCK_TIMER,
	
	RDS_STATEGY_LOCK_MAX_NUMBER
} ETALTML_RDS_StrategyLockTy;


typedef enum
{
	RDS_FUNC_AF,
	RDS_FUNC_TA,
	RDS_FUNC_REG,
	RDS_FUNC_PIMUTE,
	RDS_FUNC_PTY,
	RDS_FUNC_PTYSELECTTYPE,
	RDS_FUNC_EON,
	RDS_FUNC_PISeek,
}RDS_FuncType;


/* align with the defincation used in rds_data,  DABMW_charIntTy*/
typedef union
{
    tU8 		byte[2];
    tU16 		dbyte;
} RDS_charIntTy;


typedef struct RDS_Timer_struct {
	tU32 	Timer10ms;
	tU32 	Timer100ms;
} RDS_TimerStruct;


typedef struct 
{
	tU32 	PISeekTimer;
	tU32		PISeek100msTimerInitVal;
	tU32		BackupPI;
} RDS_PIInfoTy;


typedef struct 
{
	tU8 		BackupTP;
	tU8 		BackupTA;
	tBool 	F_GetNewTP;
	tBool 	F_GetNewTA;
} RDS_TPTAInfoTy;


typedef struct RDS_AF_Info_Ty
{
	tU8 		AFFreq[MAX_AF_NUMBER];
	tU8 		AFNum;
	tU8 		AFFreqBackup;
	tU8 		AFMethod[2];
	tS32 	AFSmeter[MAX_AF_NUMBER];
	tU8 		AFDisable[MAX_AF_NUMBER];
	RDS_AFModeInfoTy AFCheckMode;
	tU8 		AFTimer;
	tU32 	AFDisable100msTimer;
	
	tU8 		AFCheckPoint;
	tU8 		AFCheckStartTimer;
	tU8 		AFCheckStartTimerSet;
	tU8 		AFCheckWaitTimer;
	tU8 		AFCheckWaitTimerSet;
	tU8 		AFSearchWaitTimer;
	tU8 		RDSStationDelayTimer;
	tU8 		AFSmeterLevel;
	tU8 		AFSmeterCheckFreqTimer;
	tU8 		AFStrengthDelta;
	tU8 		AFStrengthFast;
	tU8 		AFStrengthSlow;
	tU8 		AFStrengthDeltaReal;
	tU8 		AFStrengthEON;
	tU8 		AFCheckDisable1sTimerInitVal;
	tU8 		AFCheckBetweenList1sTimerInitVal;
	tU8 		AFCheckBetweenFreq10msTimerInitVal;
	tU8 		AFFastCheckBetweenList1sTimerInitVal;
	tU8 		AFFastCheckBetweenFreq10msTimerInitVal;	
	tU8 		F_GetNewAF;
	tU8 		F_GetAFList;
	tU8 		F_AFMethodB;
	tU8 		F_RDSStationSlow;
} RDS_AFInfoTy;


typedef struct
{
	tU8		F_RDSEnable;
	tU8 		F_AFEnable;
	tU8 		F_TAEnable;
	tU8 		F_REGEnable;
	tU8 		F_PIMuteEnable;
	tU8 		F_PTYEnable;
	tU8 		PTYSelectType;
	tU8 		F_EONEnable;
}RDS_FunctionInfoTy;


typedef struct
{
	tU32		AutoTASeekTimer;
	tU8		PTYSelectTimer;
	tU8		InTADelayTimer;
	tU8		TAWaitAFCheckTimer;
	tU8		EnterTADelayTimer;
	tU8		EnterEONTADelayTimer;
}RDS_FunctionTimerTy;


typedef struct 
{
	tU8		Level;
	tU8		MaxLevel;
} RDS_QualityInfoTy;


typedef struct 
{
	tS32		Smeter;
	tU32		Detuning;
	tU32		Multipath;
	tS32 	AdjChannel;
} RDS_TunerQualInfoTy;


typedef struct
{
	tU8 		F_InTASwitch;
	tU8 		F_NoPTY;
	tU8 		F_InEONTASwitch;
	tU8 		F_PISeek;
	tU8		F_InTASeek;
}RDS_ProcessInfoTy;


#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
typedef struct 
{
	tU8 		EONSave_EONTP;
	tU8 		EONSave_EONTPLast;
	tU8		EONSave_EONTA;
	tU8		EONSave_EONTALast;
	tU8		EONSave_F_GetNewEONTP;
	tU8		EONSave_F_GetNewEONTA;
} RDS_EONSaveInfoTy;


typedef struct RDS_EON_Info_Ty
{	
	tU8 		EONAFFreq[MAX_AF_NUMBER];
	tU8 		EONAF[MAX_AF_NUMBER];
	tU8 		EONAFNum;
	tU8 		EONAFTimer;
	tU32    	EONPI;
	tU8 		EONPS[8];
	
	tS32 		EONAFSmeter[MAX_AF_NUMBER];
	tU8		EONAFNumBackup;
	tU8		EONAFTimerBackup;
	tU32    	EONPIBackup;
	tU8		EONPSBackup[8];
	tU8		EONExitTimer;
	
	tU32		EONSave_EONPI[EON_BUFF_SIZE];
	tU8		EONSave_EONPS[EON_BUFF_SIZE][8];
	tU8		EONSave_EONAF[EON_BUFF_SIZE][MAX_AF_NUMBER];
	tU8		EONSave_EONAFNum[EON_BUFF_SIZE];
	tU8		EONSave_EONCheckDisable[EON_BUFF_SIZE];
	
	RDS_EONSaveInfoTy EONSave_Flags[EON_BUFF_SIZE];
	tU8		EONCheck;
	RDS_EONModeInfoTy EONMode;

	tU8		EONTP;
	tU8		EONTPLast;
	tU8		EONTA;
	tU8		EONTALast;

	tU8		F_GetNewEONTP;
	tU8		F_GetNewEONTA;
	tU8		F_InEONTA;
	tU8		F_InEONTAReal;
	tU8		F_NoMatchEON;
} RDS_EONInfoTy;
#endif


typedef struct RDS_Mgr_Struct
{
	RDS_QualityInfoTy 	Qual;
	RDS_PIInfoTy 		PIInfo;
	RDS_TPTAInfoTy 		TATPInfo;
	RDS_AFInfoTy 		AFInfo;

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
	RDS_EONInfoTy 		EONInfo;
#endif

	RDS_TunerQualInfoTy 	TunerCFQual;
	RDS_TunerQualInfoTy 	TunerAFQual;

	RDS_FunctionInfoTy 	FunctionInfo;
	RDS_ProcessInfoTy 	ProcessFlag;
	RDS_FunctionTimerTy 	FuncTimer;
} RDSMgrStruct;


tSInt ETALTML_RDS_Strategy_InitLock(tVoid);
tSInt ETALTML_RDS_Strategy_DeinitLock(tVoid);
tVoid ETALTML_RDS_Strategy_Data_Init(ETAL_HANDLE hReceiver);
ETAL_STATUS ETALTML_RDS_Strategy_Init(ETAL_HANDLE hReceiver, RDS_FunctionInfoTy RdsFuncPara);
ETAL_STATUS ETALTML_RDS_Strategy_BreakAFCheck(ETAL_HANDLE hReceiver);
ETAL_STATUS ETALTML_RDS_Strategy_GetRuntimeData(ETAL_HANDLE hReceiver, EtalRDSStrategyStatus *pRDSData);
ETAL_STATUS ETALTML_RDS_Strategy_AFSearch_Start(ETAL_HANDLE hReceiver, EtalRDSAFSearchData afSearchData);
ETAL_STATUS ETALTML_RDS_Strategy_AFSearch_Stop(ETAL_HANDLE hReceiver);
tVoid ETALTML_RDS_Strategy_Main(ETAL_HANDLE hReceiver);
tU32  ETALTML_RDS_Strategy_RDSTimer_Get(ETAL_HANDLE hReceiver, RDS_TimerType timerType);
tVoid ETALTML_RDS_Strategy_RDSTimer_Set(ETAL_HANDLE hReceiver, RDS_TimerType timerType, tU32 tCount);
tVoid ETALTML_RDS_Strategy_RDSEnable(ETAL_HANDLE hReceiver);
tVoid ETALTML_RDS_Strategy_RDSDisable(ETAL_HANDLE hReceiver);
tVoid ETALTML_RDS_Strategy_RDSStation_Indicator(tSInt slot);
tBool ETALTML_RDS_Strategy_IsPISeekOk(ETAL_HANDLE hReceiver);
tVoid ETALTML_RDS_Strategy_SetBackupPI(ETAL_HANDLE hReceiver, tU32 backupPI);
tBool ETALTML_RDS_Strategy_GetSwitchStatus(ETAL_HANDLE hReceiver);
tBool ETALTML_RDS_Strategy_GetFunctionStatus(ETAL_HANDLE hReceiver, RDS_FuncType rdsRuncType);
tVoid ETALTML_RDS_Strategy_SetFunctionStatus(ETAL_HANDLE hReceiver, RDS_FuncType rdsRuncType, tBool value);
tVoid ETALTML_RDS_Strategy_ResetPISeekTimer(tSInt slot, tBool bForced);
tVoid ETALTML_RDS_Strategy_AFData_Decode(tSInt slot, RDS_charIntTy AFtemp);
tVoid ETALTML_RDS_Strategy_EONData_Decode(tSInt slot, tU32 piVal, tU8 realGroup,
									                            tBool availBlock_A, tU32 block_A, 
									                            tBool availBlock_B, tU32 block_B, 
									                            tBool availBlock_C, tU32 block_C,
									                            tBool availBlock_Cp, tU32 block_Cp,
									                            tBool availBlock_D, tU32 block_D);


#if defined (CONFIG_ETAL_HAVE_AUTOSEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETALTML_rds_seek_start
 *
 **************************/
ETAL_STATUS ETALTML_rds_seek_start(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, EtalRDSSeekTy rdsSeekOption);

#if (defined CONFIG_ETALTML_HAVE_LEARN) || (defined CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
tSInt ETALTML_RdsTaskInit(tVoid);
tSInt ETALTML_RdsTaskDeinit(tVoid);
#endif


#endif

#ifdef __cplusplus
}
#endif

#endif // _RDS_STRATEGY_H_

// End of file

