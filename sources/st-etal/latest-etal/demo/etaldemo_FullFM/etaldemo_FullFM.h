//!
//!  \file 		etaldemo_SeekOrTune.h
//!  \brief 	<i><b> ETAL application demo for simple seek and tune </b></i>
//!  \details   supports FM and DAB
//!  \author 	Erwan Preteseille
//!


/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station possibly with RDS
 */
#define ETAL_VALID_FM_FREQ     87600
#define ETAL_EMPTY_FM_FREQ     0
#define ETAL_FIRST_FM_FREQ     87500

//#define etalDemoPrintf(level, ...)	OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW_SF,  __VA_ARGS__)

#define etalDemoPrintf(level, ...) \
	do { printf(__VA_ARGS__); } while (0)

//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)


#define ETAL_DEMO_SEEK_DAB_WAIT_TIME	(200*1000)
#define ETAL_DEMO_SEEK_FM_WAIT_TIME		(30*1000)

#define ETAL_DEMO_SCAN_PLAY_TIME		(5*1000)

#define ETAL_DEMO_SID_TABLE_SIZE		15
/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB_FREQ    225648
#define ETAL_EMPTY_DAB_FREQ    0
#define ETAL_START_DAB_FREQ    174928

#define ETAL_DAB_DEFAULT_SID 0x1D12

#define PREFIX0 "    "
#define PREFIX1 "        "

#ifdef CONFIG_HOST_OS_WIN32
	#define USLEEP_ONE_SEC        1000
#elif CONFIG_HOST_OS_TKERNEL
	#define USLEEP_ONE_SEC        1000
	#define Sleep				  tk_dly_tsk
#else
	#define Sleep                 usleep
	#define USLEEP_ONE_SEC        1000000
#endif
#define NB_SEC_TO_SLEEP_BEFORE_END	60
#define USLEEP_NB_SEC_END	NB_SEC_TO_SLEEP_BEFORE_END * USLEEP_ONE_SEC

// nominal step default
#define ETAL_DEMO_FREQ_STEP 100

/* EVENT for PI reception and triggering SF */
#define ETALDEMO_EVENT_TUNED_RECEIVED    		    0
#define ETALDEMO_EVENT_SCAN_FOUND					1
#define ETALDEMO_EVENT_SCAN_FINISHED				2
#define ETALDEMO_LAST_EVENT							3

// EVENT FLAGS
#define  ETALDEMO_EVENT_TUNED_RECEIVED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_TUNED_RECEIVED)
#define ETALDEMO_EVENT_SCAN_FOUND_FLAG		((tU32)0x01 << ETALDEMO_EVENT_SCAN_FOUND)
#define ETALDEMO_EVENT_SCAN_FINISHED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_SCAN_FINISHED)

#define ETALDEMO_EVENT_WAIT_MASK	(ETALDEMO_EVENT_TUNED_RECEIVED_FLAG)
#define ETALDEMO_EVENT_SCAN_WAIT_MASK (ETALDEMO_EVENT_SCAN_FOUND_FLAG | ETALDEMO_EVENT_SCAN_FINISHED_FLAG)


/*****************************************************************
| variables;
|----------------------------------------------------------------*/



tU32 Sid;


//ETAL_HANDLE hDatapathAudio_dab_1;
ETAL_HANDLE hCurrentReceiver;

tBool etaldemo_OneDABChannel;
tU32 etaldemo_seek_frequency_start;


OSAL_tEventHandle etalDemo_EventHandler;

enum etalDemoMode {
	ETALDEMO_MODE_INVALID = 0,
	ETALDEMO_MODE_SEEK,
	ETALDEMO_MODE_SCAN
};

/*****************************************************************
| functions;
|----------------------------------------------------------------*/

#ifdef HAVE_FM

static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq);

static ETAL_HANDLE etalDemo_TuneFMBgRadio(tU32 vI_freq);

#endif

#ifdef HAVE_DAB
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq);
#endif

static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);

int etalDemo_parseParameters(int argc, char **argv, tU32 *pO_freq1, tU32 *pO_freq2, tU32 *pO_freq3, tU32 *pO_freq4);

void etalDemo_Press_Any_Key(void);

void etalDemo_Appli_entryPoint(tU32 vI_frequency1, tU32 vI_frequency2, tU32 vI_frequency3, tU32 vI_frequency4);

// event
// event creation
// Init globals
tSInt etalDemo_initEvent();
tSInt etalDemo_PostEvent(tU8 event);
tSInt etalDemo_ClearEvent(tU8 event);
tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event);


