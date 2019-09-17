//!
//!  \file 		etaltmlinternal.h
//!  \brief 	<i><b>ETALTML private header</b></i>
//!  \details	ETALTML definitions to be used only within the ETAL library
//!

/***********************************
 *
 * Source configuration
 *
 **********************************/


/***********************************
 *
 * Defines
 *
 **********************************/

// MACRO FOR TML LOGGING

#if defined(CONFIG_TRACE_CLASS_ETAL) 
#define ETALTML_PRINTF(level, ...) \
		do { (void)OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)

#else
#define ETALTML_PRINTF(level, ...) 
#endif

#if defined(CONFIG_TRACE_CLASS_RDS_STRATEGY) 
#define ETALTML_RDS_STRATEGY_PRINTF(level, ...) \
		do { (void)OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_RDS_STRATEGY, __VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)

#else
#define ETALTML_RDS_STRATEGY_PRINTF(level, ...) 
#endif

 
#define ETAL_PATH_NAME_NUMBER	9


#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_LEARN)
// we need to have a non empty flag for waiting else it is failing in t-kernel at least
#define ETALTML_LEARN_NO_EVENT_FLAG        0x00

#define ETALTML_LEARN_EVENT_SLEEP			30
#define ETALTML_LEARN_EVENT_SLEEP_FLAG		((tU32)0x01 << ETALTML_LEARN_EVENT_SLEEP)

#define ETALTML_LEARN_EVENT_FLAGS          ETALTML_LEARN_EVENT_SLEEP_FLAG
#define ETALTML_LEARN_EVENT_WAIT_MASK      (ETALTML_LEARN_EVENT_FLAGS)

// Kill event
#define ETALTML_LEARN_EVENT_KILL					0
// WAKEUP EVENT
#define ETALTML_LEARN_EVENT_WAKE_UP					31

#define ETALTML_LEARN_EVENT_KILL_FLAG				((tU32)0x01 << ETALTML_LEARN_EVENT_KILL)
#define ETALTML_LEARN_EVENT_WAKEUP_FLAG				((tU32)0x01 << ETALTML_LEARN_EVENT_WAKE_UP)

#define ETALTML_LEARN_WAKEUP_FLAGS					(ETALTML_LEARN_EVENT_KILL_FLAG | ETALTML_LEARN_EVENT_WAKEUP_FLAG)
#define ETALTML_LEARN_EVENT_WAIT_WAKEUP_MASK		(ETALTML_LEARN_WAKEUP_FLAGS)

#define ETALTML_LEARN_EVENT_ALL_FLAG				(0xFFFFFFFF)

#define ETALTML_LEARN_THREAD_PRIORITY      OSAL_C_U32_THREAD_PRIORITY_NORMAL
#define ETALTML_LEARN_STACK_SIZE           4096
#define ETALTML_LEARN_EVENT_WAIT_TIME_MS   10U
#undef DEBUG_LEARN
#endif //defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_LEARN)

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SCAN)
// we need to have a non empty flag for waiting else it is failing in t-kernel at least
#define ETALTML_SCAN_NO_EVENT_FLAG          0x00

#define ETALTML_SCAN_EVENT_SLEEP            30
#define ETALTML_SCAN_EVENT_SLEEP_FLAG       ((tU32)0x01 << ETALTML_SCAN_EVENT_SLEEP)

#define ETALTML_SCAN_EVENT_FLAGS            ETALTML_SCAN_EVENT_SLEEP_FLAG
#define ETALTML_SCAN_EVENT_WAIT_MASK        (ETALTML_SCAN_EVENT_FLAGS)

// Kill event
#define ETALTML_SCAN_EVENT_KILL             0
// WAKEUP EVENT
#define ETALTML_SCAN_EVENT_WAKE_UP          31

#define ETALTML_SCAN_EVENT_KILL_FLAG        ((tU32)0x01 << ETALTML_SCAN_EVENT_KILL)
#define ETALTML_SCAN_EVENT_WAKEUP_FLAG      ((tU32)0x01 << ETALTML_SCAN_EVENT_WAKE_UP)

#define ETALTML_SCAN_WAKEUP_FLAGS           (ETALTML_SCAN_EVENT_KILL_FLAG | ETALTML_SCAN_EVENT_WAKEUP_FLAG)
#define ETALTML_SCAN_EVENT_WAIT_WAKEUP_MASK (ETALTML_SCAN_WAKEUP_FLAGS)

#define ETALTML_SCAN_EVENT_ALL_FLAG         (0xFFFFFFFF)

#define ETALTML_SCAN_THREAD_PRIORITY        OSAL_C_U32_THREAD_PRIORITY_NORMAL
#define ETALTML_SCAN_STACK_SIZE             4096
#define ETALTML_SCAN_EVENT_WAIT_TIME_MS     10U
#undef DEBUG_SCAN
#endif //defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SCAN)

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_LEARN)
extern OSAL_tSemHandle    etalLearnSem;
#endif //defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_LEARN)

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SCAN)
extern OSAL_tSemHandle    etalScanSem;
#endif //defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_SCAN)

/***********************************
 *
 * Types
 *
 **********************************/
/*
 * etalScanStatusTy
 */
typedef struct
{
	tBool			  scanForcedStop;
	tBool			  scanNewCmd;
	tBool			  scanStopCross;
	tBool			  paramScanStart_playLastBadStation;
	tBool			  paramScanStart_directionDown;
	tBool			  paramScanStop_playLastBadStation;
	tU8				  scanLastStopSide;
	tU8				  scanLastFetchedStatus;
	tU32			  scanPlayCounter;
	tU32			  scanDoNext;
	tU32			  scanSeekStep;
	tU32			  scanUpperBandLimit;
	tU32			  scanLowerBandLimit;
	tU32			  scanOriginalFrequency;
	tU32			  scanLastFetchedFrequency;
	tU32			  scanLastGoodFrequency;
	tU32			  paramScanStart_playTimeSec;
	tS32			  scanResult;
} etalScanStatusTy;


/***********************************
 *
 * Variables
 *
 **********************************/



/***********************************
 *
 * Function prototypes
 *
 **********************************/

/*
 * Initialization
 */
tSInt ETALTML_init(const EtalHardwareAttr *init_params, tBool power_up);
tSInt ETALTML_deinit(tBool power_up);

/*
 * Internal interfaces
 */

/*
 * RDS decoder
 */
#if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
tVoid       ETALTML_RDSresetData(ETAL_HANDLE hReceiver);
tVoid       ETALTML_RDScheckDecoded(ETAL_HANDLE hReceiver, tU8 *buf, tU32 len);
tVoid 		ETALTML_RDSInit(tVoid);
#endif

/*
 * Path management
 */
ETAL_STATUS ETALTML_getFreeReceiverForPathInternal(ETAL_HANDLE *pReceiver, EtalPathName vI_path, EtalReceiverAttr *pO_attr);
ETAL_STATUS ETALTML_getReceiverForPathInternal(ETAL_HANDLE *pReceiver, EtalPathName vI_path, EtalReceiverAttr *pO_attr, tBool vI_reuse);
tVoid ETALTML_PathAllocationInit(tVoid);
ETAL_HANDLE ETALTML_GetPathAllocation(EtalPathName vI_path);


#if (defined CONFIG_ETALTML_HAVE_SCAN) || (defined CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
tSInt ETALTML_scanTaskInit(tVoid);
tSInt ETALTML_scanTaskDeinit(tVoid);
#endif

#if (defined CONFIG_ETALTML_HAVE_LEARN) || (defined CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
tSInt ETALTML_learnTaskInit(tVoid);
tSInt ETALTML_learnTaskDeinit(tVoid);
#endif

// Add ON TUNERS PATH MGT:
//
EtalBcastStandard ETALTML_pathGetStandard(EtalPathName vI_path);
tSInt ETALTML_pathGetNextFEList(etalDiversTy **pO_Felist, EtalPathName vI_path, tU8 vI_Index);

ETAL_STATUS ETALTML_get_PIStatus(ETAL_HANDLE hReceiver, DABMW_storageStatusEnumTy *pPIStatus, tU32 *pCurrentPI, tU32 * pLastPI, tU32 * pBackupPI);
ETAL_STATUS ETALTML_get_TATPStatus(ETAL_HANDLE hReceiver, tVoid **ppTATPData);
ETAL_STATUS ETALTML_get_RDSStationFlag(ETAL_HANDLE hReceiver,  tBool * bRDSStation);

// ETAL TML API INTERNAL
// LANDSCAPE FUNCTION
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

ETAL_STATUS ETALTML_GetEnsembleList (DABMW_ensembleUniqueIdTy *pO_dataPtr, tPU16 pO_NbEnsemble );
tSInt ETALTML_GetServiceList(tU32 vI_ensembleId, EtalServiceList *pO_dataPtr);

#endif // #ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
tVoid ETALTML_RDSgetDecodedBlock(ETAL_HANDLE hReceiver, EtalRDSData *pRDSdata, tBool forcedGet);
#endif 
#endif 

#if defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)

tSInt ETALTML_RdsSeekTaskInit (tVoid);
tSInt ETALTML_RdsSeekTaskDeinit (tVoid);

#endif // CONFIG_ETALTML_HAVE_RDS_STRATEGY

// include the service from service following
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#include "Service_following_externalInterface_API.h"
#endif

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
#include "rds_strategy.h"
#endif
