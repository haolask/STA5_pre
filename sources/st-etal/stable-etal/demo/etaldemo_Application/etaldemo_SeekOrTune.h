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

#define ETAL_BAND_FMEU_MIN     87500
#define ETAL_BAND_FMEU_MAX    108000

#define ETAL_BAND_FMUS_MIN     87900
#define ETAL_BAND_FMUS_MAX    107900

#define ETAL_BAND_FMJP_MIN     76000
#define ETAL_BAND_FMJP_MAX     95000

#define ETAL_BAND_AM_MIN         531 //144
#define ETAL_BAND_AM_MAX       1629 //30000

//#define etalDemoPrintf(level, ...)	OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW_SF,  __VA_ARGS__)

#define etalDemoPrintf(level, ...) \
	do { printf(__VA_ARGS__); } while (0)
	
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __FUNCTION__ "(): "__VA_ARGS__); } while (0)
//	do { printf(__VA_ARGS__); } while (0)
//	do { OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_ETAL, __VA_ARGS__); } while (0)


#define ETAL_DEMO_SEEK_DAB_WAIT_TIME	(200*1000)
#define ETAL_DEMO_SEEK_FM_WAIT_TIME		(30*1000)
#define ETAL_DEMO_SEEK_AM_WAIT_TIME		(50*1000)

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
#define ETAL_DEMO_AM_FREQ_STEP 9

/* EVENT for PI reception and triggering SF */
#define ETALDEMO_EVENT_TUNED_RECEIVED    		    0
#define ETALDEMO_EVENT_SCAN_FOUND					1
#define ETALDEMO_EVENT_SCAN_FINISHED				2
#define ETALDEMO_EVENT_LEARN_FINISHED				3
#define ETALDEMO_EVENT_SEEK_FINISHED    		    5
#define ETALDEMO_LAST_EVENT							5

// EVENT FLAGS 
#define  ETALDEMO_EVENT_TUNED_RECEIVED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_TUNED_RECEIVED)
#define ETALDEMO_EVENT_SCAN_FOUND_FLAG		((tU32)0x01 << ETALDEMO_EVENT_SCAN_FOUND)
#define ETALDEMO_EVENT_SCAN_FINISHED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_SCAN_FINISHED)
#define ETALDEMO_EVENT_LEARN_FINISHED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_LEARN_FINISHED)
#define ETALDEMO_EVENT_SEEK_FINISHED_FLAG	((tU32)0x01 << ETALDEMO_EVENT_SEEK_FINISHED)



#define ETALDEMO_EVENT_WAIT_MASK	(ETALDEMO_EVENT_TUNED_RECEIVED_FLAG)
#define ETALDEMO_EVENT_SEEK_WAIT_MASK	(ETALDEMO_EVENT_SEEK_FINISHED_FLAG)
#define ETALDEMO_EVENT_SCAN_WAIT_MASK (ETALDEMO_EVENT_SCAN_FOUND_FLAG | ETALDEMO_EVENT_SCAN_FINISHED_FLAG)
#define ETALDEMO_EVENT_LEARN_WAIT_MASK (ETALDEMO_EVENT_LEARN_FINISHED_FLAG)

#define ETAL_DEMO_FMAM_BAND  (ETAL_BAND_FM | ETAL_BAND_FMEU | ETAL_BAND_FMUS | ETAL_BAND_FMJP | ETAL_BAND_AM)
#define ETAL_DEMO_DAB_BAND ETAL_BAND_DAB3

#define ETAL_DEMO_QUALITY_PERIODICITY 1000 

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

typedef struct
{
	ETAL_HANDLE    hReceiver;
	etalDemoModeTy mode;
}etalDemoThreadAttrTy;

/*****************************************************************
| variables;
|----------------------------------------------------------------*/

tU32 Sid;

/* Demonstration mode enables to control ETAL demo application by FIFO and not keyboard via console
  * One channel for control
  * Radiotext will be written in a file
  */
tBool IsDemonstrationMode = false;
#ifndef CONFIG_HOST_OS_WIN32
char * CtrlFIFO = "/tmp/ETALCtrlFIFO";
int fd_CtrlFIFO;
char * RadioTextFile = "/tmp/ETALRadioText";
#else
char * RadioTextFile = "ETALRadioText";
#endif // !CONFIG_HOST_OS_WIN32
int fd_RadioText;

tBool RDS_on = true;
tBool RDS_decoded_on = false;
tBool QualityMonitor_on = false;

tBool earlyAudio = false;

tBool DCOP_is_NVMless = false;

#define ETAL_DCOP_LOADER_BOOTSTRAP_FILENAME  "spiloader.bin"
#define ETAL_DCOP_SECTIONS_DESCR_FILENAME    "dcop_fw_sections.cfg"

#define ETAL_DCOP_BOOT_FILENAME_MAX_SIZE      256

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
static tChar ETALDcop_bootstrap_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_LOADER_BOOTSTRAP_FILENAME;
static tChar ETALDcop_sections_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_SECTIONS_DESCR_FILENAME;
#endif

//ETAL_HANDLE hDatapathAudio_dab_1;
ETAL_HANDLE hCurrentReceiver;
ETAL_HANDLE hInitialDatapath;

ETAL_HANDLE hSecondReceiver;

tBool etaldemo_seek_on_going; 
tU32 etaldemo_seek_frequency_start;

EtalLearnFrequencyTy etaldemo_learn_list[ETAL_LEARN_MAX_NB_FREQ];
tU32 etaldemo_learn_freq_nb;

OSAL_tEventHandle etalDemo_EventHandler;

tBool tunedEnsemble;
static tU32 lastFreqUsed;

tU8 etaldemo_HD_status;


/*****************************************************************
| functions;
|----------------------------------------------------------------*/

#ifdef HAVE_FM
static ETAL_HANDLE etalDemo_FMSeekRadio(tU32 vI_StartFreq, EtalFrequencyBand FMBand);

static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq, EtalFrequencyBand FMBand);

#endif

#ifdef HAVE_AM
static ETAL_HANDLE etalDemo_AMSeekRadio(tU32 vI_StartFreq, EtalFrequencyBand AMBand);
static ETAL_HANDLE etalDemo_TuneAMRadio(tU32 vI_freq, EtalFrequencyBand AMBand);
#endif

#ifdef HAVE_DAB
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq);
static ETAL_HANDLE etalDemo_TuneDAB2Radio(tU32 vI_freq);
static void etalDemo_DAB2DataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

static ETAL_HANDLE etalDemo_DABRadioSeek(tU32 vI_StartFreq);

static ETAL_STATUS etalDemo_DABServiceList(ETAL_HANDLE handleDab);

static ETAL_STATUS etalDemo_DABServiceListAndSelect(ETAL_HANDLE handleDab);
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
static int etalDemo_GetDCOPImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap);
#endif
#ifdef ETAL_DEMO_CREATE_FIC_DATAPATH
static tVoid etalDemoSeekOrTuneDataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
#endif
#endif

static tSInt etalDemoSeek_CmdWaitResp(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, tU32 step, OSAL_tMSecond response_timeout);

static tSInt etalDemoScan(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, ETAL_HANDLE *vl_hDatapathRds);
static tSInt etalDemoLearn(ETAL_HANDLE receiver, EtalFrequencyBand vI_band);


static tVoid etalDemo_UserInt_ThreadEntry(tPVoid param);

static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);

static int etalDemo_parseParameters(int argc, char **argv, EtalFrequencyBand *pI_Band, tBool *pO_isSeek, tU32 *pO_freq, tU32 *pO_BGfreq);
static tVoid etalDemoEndTest();

static void etalDemo_Appli_entryPoint(EtalFrequencyBand vI_Band, tBool vI_modeIsSeek, tU32 vI_frequency, tU32 vI_BGfrequency);

static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text);
static int etalDemo_startRadioText(ETAL_HANDLE vl_handle, ETAL_HANDLE *vl_hDatapathRds);
static void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
static int etalDemo_startRadioRDSDecoded(ETAL_HANDLE vl_handle, ETAL_HANDLE *pO_hDatapathRds);
static int etalDemo_stopRadioRDSDecoded(ETAL_HANDLE vl_handle, ETAL_HANDLE vI_hDatapathRds);

static tVoid etalDemo_PrintRDS(EtalRDSData *rds);
static void etalDemoRDSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);



static void etalDemo_ConfigureAudioForTuner();
static void etalDemo_startQualityMonitor(ETAL_HANDLE vl_handle, EtalFrequencyBand vI_band);
static void etalDemo_FMAMQualityMonitorCallback(EtalBcastQualityContainer* pQuality, void* vpContext);
static void etalDemo_DABQualityMonitorCallback(EtalBcastQualityContainer* pQuality, void* vpContext);
static void etalDemo_stopQualityMonitor(void);

// event
// event creation
// Init globals
tSInt etalDemo_initEvent();
tSInt etalDemo_PostEvent(tU8 event);
tSInt etalDemo_ClearEvent(tU8 event);
tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event);


