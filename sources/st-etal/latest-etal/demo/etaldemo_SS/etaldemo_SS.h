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
#define ETAL_BAND_FMJP_MAX     90000

//#define etalDemoPrintf(level, ...)	OSALUTIL_s32TracePrintf(0, level, TR_CLASS_APP_DABMW_SF,  __VA_ARGS__)

#define etalDemoPrintf(level, ...) do { printf(__VA_ARGS__); } while (0)

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
#if 0
static ETAL_HANDLE etalDemo_FMSeekRadio(tU32 vI_StartFreq, EtalFrequencyBand FMBand);
#endif

static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq, EtalFrequencyBand FMBand, tBool select_audio);

#endif

#ifdef HAVE_DAB
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq, tBool select_audio);
static ETAL_HANDLE etalDemo_TuneDAB2Radio(tU32 vI_freq);
static void etalDemo_DAB2DataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

#if 0
static ETAL_HANDLE etalDemo_DABRadioSeek(tU32 vI_StartFreq);

static ETAL_STATUS etalDemo_DABServiceList(ETAL_HANDLE handleDab);
static ETAL_STATUS etalDemo_DABServiceListAndSelect(ETAL_HANDLE handleDab);
#endif
#endif

#if 0
static tSInt etalDemoSeek_CmdWaitResp(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, OSAL_tMSecond response_timeout);

static tSInt etalDemoScan(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, ETAL_HANDLE *vl_hDatapathRds);
static tSInt etalDemoLearn(ETAL_HANDLE receiver, EtalFrequencyBand vI_band);


static tVoid etalDemo_UserInt_ThreadEntry(tPVoid param);
#endif

static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus);

static int etalDemo_parseParameters(int argc, char **argv, EtalFrequencyBand *pI_Band, tU32 *pO_freq, tU32 *pO_tunerch);
static tVoid etalDemoEndTest();

static void etalDemo_Appli_entryPoint(EtalFrequencyBand *p_Band, tU32 *p_freq, tU32 *p_tunerch);

static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text);
static int etalDemo_startRadioText(ETAL_HANDLE vl_handle, ETAL_HANDLE *vl_hDatapathRds);
static void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);

static void etalDemo_ConfigureAudioForTuner();

// event
// event creation
// Init globals
tSInt etalDemo_initEvent();
tSInt etalDemo_PostEvent(tU8 event);
tSInt etalDemo_ClearEvent(tU8 event);
tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event);

tCString etaldemo_se_strstatus(int se_status);

// Etal API demo functions
ETAL_STATUS etaldemo_initialize(tVoid);
ETAL_STATUS etaldemo_reinitialize(tVoid);
ETAL_STATUS etaldemo_deinitialize(tVoid);
ETAL_STATUS etaldemo_config_receiver(tVoid);
ETAL_STATUS etaldemo_destroy_receiver(tVoid);
ETAL_STATUS etaldemo_config_datapath(tVoid);
ETAL_STATUS etaldemo_destroy_datapath(tVoid);
ETAL_STATUS etaldemo_config_audio_path(tVoid);
ETAL_STATUS etaldemo_audio_select(tVoid);
ETAL_STATUS etaldemo_force_mono(tVoid);
ETAL_STATUS etaldemo_mute(tVoid);
ETAL_STATUS etaldemo_event_FM_stereo_start(tVoid);
ETAL_STATUS etaldemo_event_FM_stereo_stop(tVoid);
ETAL_STATUS etaldemo_debug_config_audio_alignment(tVoid);
ETAL_STATUS etaldemo_receiver_alive(tVoid);
ETAL_STATUS etaldemo_xtal_alignment(tVoid);
ETAL_STATUS etaldemo_get_version(tVoid);
ETAL_STATUS etaldemo_get_capabilities(tVoid);
ETAL_STATUS etaldemo_get_init_status(tVoid);
ETAL_STATUS etaldemo_tune_receiver(tVoid);
ETAL_STATUS etaldemo_tune_receiver_async(tVoid);
ETAL_STATUS etaldemo_change_band_receiver(tVoid);
ETAL_STATUS etaldemo_get_current_ensemble(tVoid);
ETAL_STATUS etaldemo_get_ensemble_list(tVoid);
ETAL_STATUS etaldemo_get_ensemble_data(tVoid);
ETAL_STATUS etaldemo_get_fic(tVoid);
ETAL_STATUS etaldemo_get_service_list(tVoid);
ETAL_STATUS etaldemo_get_specific_service_data_DAB(tVoid);
ETAL_STATUS etaldemo_service_select_audio(tVoid);
ETAL_STATUS etaldemo_service_select_data(tVoid);
ETAL_STATUS etaldemo_get_reception_quality(tVoid);
ETAL_STATUS etaldemo_get_channel_quality(tVoid);
ETAL_STATUS etaldemo_config_reception_quality_monitor(tVoid);
ETAL_STATUS etaldemo_destroy_reception_quality_monitor(tVoid);
ETAL_STATUS etaldemo_get_receiver_frequency(tVoid);
ETAL_STATUS etaldemo_get_CF_data(tVoid);
ETAL_STATUS etaldemo_seek_start_manual(tVoid);
ETAL_STATUS etaldemo_seek_continue_manual(tVoid);
ETAL_STATUS etaldemo_seek_stop_manual(tVoid);
ETAL_STATUS etaldemo_seek_get_status_manual(tVoid);
ETAL_STATUS etaldemo_autoseek_start(tVoid);
ETAL_STATUS etaldemo_autoseek_stop(tVoid);
ETAL_STATUS etaldemo_set_autoseek_thresholds_value(tVoid);
ETAL_STATUS etaldemo_AF_switch(tVoid);
ETAL_STATUS etaldemo_AF_check(tVoid);
ETAL_STATUS etaldemo_AF_start(tVoid);
ETAL_STATUS etaldemo_AF_end(tVoid);
ETAL_STATUS etaldemo_AF_search_manual(tVoid);
ETAL_STATUS etaldemo_disable_data_service(tVoid);
ETAL_STATUS etaldemo_enable_data_service(tVoid);
ETAL_STATUS etaldemo_setup_PSD(tVoid);
ETAL_STATUS etaldemo_read_parameter(tVoid);
ETAL_STATUS etaldemo_write_parameter(tVoid);
ETAL_STATUS etaldemo_start_RDS(tVoid);
ETAL_STATUS etaldemo_stop_RDS(tVoid);
ETAL_STATUS etaltmldemo_get_textinfo(tVoid);
ETAL_STATUS etaltmldemo_start_textinfo(tVoid);
ETAL_STATUS etaltmldemo_stop_textinfo(tVoid);
ETAL_STATUS etaltmldemo_get_decoded_RDS(tVoid);
ETAL_STATUS etaltmldemo_start_decoded_RDS(tVoid);
ETAL_STATUS etaltmldemo_stop_decoded_RDS(tVoid);
ETAL_STATUS etaltmldemo_get_validated_RDS_block_manual(tVoid);
ETAL_STATUS etaltmldemo_RDS_AF(tVoid);
ETAL_STATUS etaltmldemo_RDS_TA(tVoid);
ETAL_STATUS etaltmldemo_RDS_EON(tVoid);
ETAL_STATUS etaltmldemo_RDS_REG(tVoid);
ETAL_STATUS etaltmldemo_RDS_AFSearch_start(tVoid);
ETAL_STATUS etaltmldemo_RDS_AFSearch_stop(tVoid);
ETAL_STATUS etaltmldemo_RDS_seek_start(tVoid);
ETAL_STATUS etaltmldemo_scan_start(tVoid);
ETAL_STATUS etaltmldemo_scan_stop(tVoid);
ETAL_STATUS etaltmldemo_learn_start(tVoid);
ETAL_STATUS etaltmldemo_learn_stop(tVoid);
ETAL_STATUS etaltmldemo_getFreeReceiverForPath(tVoid);
ETAL_STATUS etaltmldemo_TuneOnServiceId(tVoid);
ETAL_STATUS etaltmldemo_ActivateServiceFollowing(tVoid);
ETAL_STATUS etaltmldemo_DisableServiceFollowing(tVoid);
ETAL_STATUS etaltmldemo_ServiceFollowing_SeamlessEstimationRequest(tVoid);
ETAL_STATUS etaltmldemo_ServiceFollowing_SeamlessEstimationStop(tVoid);
ETAL_STATUS etaltmldemo_ServiceFollowing_SeamlessSwitchRequest(tVoid);
ETAL_STATUS etaldemo_seamless_estimation_start(tVoid);
ETAL_STATUS etaldemo_seamless_estimation_stop(tVoid);
ETAL_STATUS etaldemo_seamless_switching(tVoid);
ETAL_STATUS etaldemo_debug_DISS_control(tVoid);
ETAL_STATUS etaldemo_debug_get_WSP_Status(tVoid);
ETAL_STATUS etaldemo_debug_VPA_control(tVoid);
ETAL_STATUS etaldemo_trace_config(tVoid);

void etalDemo_defaultDataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
void etalDemo_textinfoDatPathCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
void etalDemo_fmRdsDatPathCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
void etalDemo_rdsRawDatPathCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
