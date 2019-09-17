//!
//!  \file 		etal_ProdTestApp.h
//!  \brief 	<i><b> ETAL application for MTD production test </b></i>
//!  \details   supports FM/DAB/HD
//!  \details   Tune, Report Quality
//!  \author 	Yann Hemon
//!


/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/

/*
 * Adjust ETAL_VALID_*_FREQ to some known good FM/HD station possibly with RDS
 */
#define ETAL_VALID_FM_FREQ     98000
#define ETAL_EMPTY_FM_FREQ     0

#define ETAL_BAND_FMEU_MIN     87500
#define ETAL_BAND_FMEU_MAX    108000

#define ETAL_BAND_FMUS_MIN     87900
#define ETAL_BAND_FMUS_MAX    107900

#define ETAL_BAND_FMJP_MIN     76000
#define ETAL_BAND_FMJP_MAX     90000

#define ETAL_PTA_FM_LEVEL_INJ     20
#define ETAL_PTA_FM_QUAL_MARGIN    5

/*
 * AM
 */
#define ETAL_VALID_AM_FREQ      1071

#define ETAL_PTA_AM_LEVEL_INJ     47
#define ETAL_PTA_AM_QUAL_MARGIN    5

//#define etalDemoPrintf(level, ...)	OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW_SF,  __VA_ARGS__)

#define etalDemoPrintf(level, ...) \
	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); \
		if (level <= TR_LEVEL_SYSTEM_MIN) \
		{ \
			fd_Log = open(LogFile, O_WRONLY|O_CREAT|O_APPEND); \
			snprintf(LogBuffer, (size_t)sizeof(LogBuffer),__VA_ARGS__); \
			write(fd_Log, LogBuffer, strlen(LogBuffer)); \
			close(fd_Log); \
		} \
	} while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)


#define ETAL_DEMO_SID_TABLE_SIZE		15
/*
 * Adjust DAB defines to some known values
 * The default are for ETI stream DE-Augsburg-20090713
 */
#define ETAL_VALID_DAB3_FREQ    206352
#define ETAL_EMPTY_DAB_FREQ          0
#define ETAL_START_DAB3_FREQ    174928

#define ETAL_PTA_DAB3_LEVEL_INJ    -81
#define ETAL_PTA_DAB3_QUAL_MARGIN    5

#define ETAL_DAB_DEFAULT_SID    0x1D12

#define ETAL_VALID_DABL_FREQ   1471792

#define ETAL_PTA_DABL_LEVEL_INJ    -81
#define ETAL_PTA_DABL_QUAL_MARGIN    5

/*
 * HD
 */
#define ETAL_VALID_HDAM_FREQ    1071
#define ETAL_VALID_HDFM_FREQ   98000

#define ETAL_PTA_WAIT_FOR_HD_AUDIO_ACQ 10

#define PREFIX0 "    "
#define PREFIX1 "        "

#define USLEEP_ONE_SEC        1000000
#define NB_SEC_TO_SLEEP_BEFORE_END	60
#define USLEEP_NB_SEC_END	NB_SEC_TO_SLEEP_BEFORE_END * USLEEP_ONE_SEC

// nominal step default
#define ETAL_DEMO_FREQ_STEP 100

/* EVENT for PI reception and triggering SF */
#define ETALDEMO_EVENT_TUNED_RECEIVED    		    0
#define ETALDEMO_EVENT_SCAN_FOUND					1
#define ETALDEMO_EVENT_SCAN_FINISHED				2
#define ETALDEMO_EVENT_LEARN_FINISHED				3
#define ETALDEMO_LAST_EVENT							4

// EVENT FLAGS 
#define  ETALDEMO_EVENT_TUNED_RECEIVED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_TUNED_RECEIVED)
#define ETALDEMO_EVENT_SCAN_FOUND_FLAG		((tU32)0x01 << ETALDEMO_EVENT_SCAN_FOUND)
#define ETALDEMO_EVENT_SCAN_FINISHED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_SCAN_FINISHED)
#define ETALDEMO_EVENT_LEARN_FINISHED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_LEARN_FINISHED)


#define ETALDEMO_EVENT_WAIT_MASK	(ETALDEMO_EVENT_TUNED_RECEIVED_FLAG)
#define ETALDEMO_EVENT_SCAN_WAIT_MASK (ETALDEMO_EVENT_SCAN_FOUND_FLAG | ETALDEMO_EVENT_SCAN_FINISHED_FLAG)
#define ETALDEMO_EVENT_LEARN_WAIT_MASK (ETALDEMO_EVENT_LEARN_FINISHED_FLAG)

//ms to wait before quality measurements
#define ETAL_PTA_CMOST_QUALITY_SETTLE_TIME 20

#define ETAL_PTA_FOREGROUND TRUE
#define ETAL_PTA_BACKGROUND FALSE
#define ETAL_PTA_AUDIO_ON TRUE
#define ETAL_PTA_AUDIO_OFF FALSE

#define ETAL_PTA_SETTINGS_FILE "pta.settings"
#define ETAL_PTA_LOCAL_LOGFILE_ROOT "/usr/share/"
#define ETAL_PTA_LOGFILE_NAME "logs/PTALogFile"
#define ETAL_PTA_A5_USB_DEVICE_ROOT "/run/media/sda"
#define ETAL_PTA_DCOP_FW_FILENAME "/usr/bin/DCOP_flash/dcop_fw.bin"
#define ETAL_PTA_DCOP_HD_FW_FILENAME "/usr/bin/DCOP_HD_flash/dcop_hd_fw.bin"

/*****************************************************************
| type definitions
|----------------------------------------------------------------*/
typedef	enum {
	ETALDEMO_MODE_INVALID = 0,
	ETALDEMO_MODE_SEEK,
	ETALDEMO_MODE_MANUAL_SEEK,
	ETALDEMO_MODE_CONTINUE_MANUAL_SEEK,
	ETALDEMO_MODE_SCAN,
	ETALDEMO_MODE_LEARN,
	ETALDEMO_MODE_SERVICE_SELECT
}etalDemoModeTy;

typedef enum {
	ETALPTA_HD_INVALID = 0,
	ETALPTA_HD_SYNC_OK,
	ETALPTA_HD_SYNC_FAILED,
	ETALPTA_HD_AUDIO_ACQUIRED,
	ETALPTA_HD_AUDIO_FAILED,
}etalPtaHdStatusTy;

typedef struct
{
	ETAL_HANDLE    hReceiver;
	etalDemoModeTy mode;
}etalDemoThreadAttrTy;

typedef void (*DABCheckQualityFunc) (ETAL_HANDLE vl_handle);

/*****************************************************************
| variables;
|----------------------------------------------------------------*/

tU32 Sid;

/* Demonstration mode enables to control ETAL demo application by FIFO and not keyboard via console
  * One channel for control
  * Radiotext will be written in a file
  */
tBool IsDemonstrationMode = false;
char * CtrlFIFO = "/tmp/ETALCtrlFIFO";
int fd_CtrlFIFO;
char * RadioTextFile = "/tmp/ETALRadioText";
int fd_RadioText;

char LogFile[256];
char ExtLogFile[128];
int fd_Log;
char LogBuffer[256];
char SettingsFile[128];
#ifdef HAVE_DAB
char DABFWFile[128];
#endif
#ifdef HAVE_HD
char HDFWFile[128];
#endif

time_t startTime, endTime;

/* Default values */
tU32 AM_Freq = ETAL_VALID_AM_FREQ;
tS32 AM_ExpectedLevel = ETAL_PTA_AM_LEVEL_INJ;
tS32 AM_Margin = ETAL_PTA_AM_QUAL_MARGIN;
tU32 FM_Freq = ETAL_VALID_FM_FREQ;
tS32 FM_ExpectedLevel = ETAL_PTA_FM_LEVEL_INJ;
tS32 FM_Margin = ETAL_PTA_FM_QUAL_MARGIN;
tU32 DAB3_Freq = ETAL_VALID_DAB3_FREQ;
tS32 DAB3_ExpectedLevel = ETAL_PTA_DAB3_LEVEL_INJ;
tS32 DAB3_Margin = ETAL_PTA_DAB3_QUAL_MARGIN;
tU32 DABL_Freq = ETAL_VALID_DABL_FREQ;
tS32 DABL_ExpectedLevel = ETAL_PTA_DABL_LEVEL_INJ;
tS32 DABL_Margin = ETAL_PTA_DABL_QUAL_MARGIN;
tU32 HDFM_Freq = ETAL_VALID_HDFM_FREQ;
tU8 HD_AudioAcqWait = (tU8)ETAL_PTA_WAIT_FOR_HD_AUDIO_ACQ;

tBool RDS_on = true;

ETAL_HANDLE handleFG1 = ETAL_INVALID_HANDLE;
ETAL_HANDLE handleBG1 = ETAL_INVALID_HANDLE;
ETAL_HANDLE handleFG2 = ETAL_INVALID_HANDLE;
ETAL_HANDLE handleBG2 = ETAL_INVALID_HANDLE;

tU32 boardID;

tBool verdictRF_FM_1A, verdictRF_FM_1B, verdictRF_FM_2A, verdictRF_FM_2B;
tBool verdictBB_FM_1A, verdictBB_FM_1B, verdictBB_FM_2A, verdictBB_FM_2B;
tBool verdictAudio_FM_1A;
tBool verdictRF_AM_1A, verdictRF_AM_2A;
tBool verdictBB_AM_1A, verdictBB_AM_2A;
tBool verdictAudio_AM_1A;
tBool verdictRF_DAB3_2A, verdictRF_DAB3_2B;
tBool verdictAudio_DAB3_2A, verdictAudio_DAB3_2B;
tBool verdictRF_DABL_2A, verdictRF_DABL_2B;
tBool verdictAudio_DABL_2A, verdictAudio_DABL_2B;
tBool verdict_HD_sync_1A, verdictAudio_HD_1A, verdictAudio_HD_sync_1A;

tBool etaldemo_seek_on_going; 
tU32 etaldemo_seek_frequency_start;

EtalLearnFrequencyTy etaldemo_learn_list[ETAL_LEARN_MAX_NB_FREQ];
tU32 etaldemo_learn_freq_nb;

OSAL_tEventHandle etalDemo_EventHandler;

tBool tunedEnsemble;
static tU32 lastFreqUsed;

etalPtaHdStatusTy etaldemo_HD_status;

/*****************************************************************
| functions;
|----------------------------------------------------------------*/

#ifdef HAVE_FM
static ETAL_HANDLE etal_ProdTestApp_TuneFMRadio(tU32 vI_freq, EtalFrequencyBand FMBand, tBool isForeGround, tBool isAudioOn);
static void etal_ProdTestApp_AMCheckQuality(void);
static void etal_ProdTestApp_FMCheckQuality(void);
#endif
static void etal_ProdTestApp_CheckAudio(tBool *verdictAudio);
#ifdef HAVE_DAB
static ETAL_HANDLE etal_ProdTestApp_TuneDABRadio(tU32 vI_freq, EtalFrequencyBand vI_band, tBool isForeGround);
static void etal_ProdTestApp_DAB3CheckQuality(ETAL_HANDLE vl_handle);
static void etal_ProdTestApp_DABLCheckQuality(ETAL_HANDLE vl_handle);
#endif

static void etal_ProdTestApp_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);
static void etal_ProdTestApp_createDir(char* filePath);
static int etal_ProdTestApp_parseParameters(int argc, char **argv);
static tVoid etal_ProdTestAppEndTest();

static void etal_ProdTestApp_entryPoint();
static void etal_ProdTestApp_exitPoint();

static void etal_ProdTestApp_ConfigureAudioForTuner();

// event
// event creation
// Init globals
tSInt etal_ProdTestApp_initEvent();
tSInt etal_ProdTestApp_PostEvent(tU8 event);
tSInt etal_ProdTestApp_ClearEvent(tU8 event);
tSInt etal_ProdTestApp_ClearEventFlag(OSAL_tEventMask event);


