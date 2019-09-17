//!
//!  \file 		etaldemo_SS.c
//!  \brief 	<i><b> ETAL application demo for Seamless Switching </b></i>
//!  \details   supports FM and DAB
//!  \details   Tune, Seek, Scan, Seamless Switch
//!  \details   supports Seamless Switching
//!  \author 	Erwan Preteseille Yann Hemon
//!

#include "target_config.h"

#include "osal.h"
#ifndef CONFIG_ETAL_HAVE_ETALTML
#error "CONFIG_ETAL_HAVE_ETALTML must be defined"
#else
#if defined(CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING) && (!defined(CONFIG_ETAL_HAVE_SEAMLESS)&&!defined(CONFIG_ETAL_HAVE_ALL_API))
#error "CONFIG_ETAL_HAVE_SEAMLESS must be defined too"
#endif
#endif // CONFIG_ETAL_HAVE_ETALTML
#if defined(HAVE_DAB) && !defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
#error "Check your configuration! Accordo is configured to support DAB and ETAL not"
#endif
#if defined (HAVE_HD) && !defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#error "Check your configuration! Accordo is configured to support HD and ETAL not"
#endif

#include "etal_api.h"
#include "etaltml_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#if (defined(CONFIG_ETAL_HAVE_SEAMLESS) || defined(CONFIG_ETAL_HAVE_ALL_API))
#include <math.h>
#endif

#ifdef CONFIG_HOST_OS_WIN32
	#include <windows.h>
#else
	#include <unistd.h>  // usleep
#endif

#include "etaldemo_SS.h"

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#define ETALDEMO_NB_TUNER_MAX           2
#define ETALDEMO_NB_CHANNEL_PER_TUNER   2
#define ETALDEMO_NB_TUNER_CHANNEL_MAX   (ETALDEMO_NB_TUNER_MAX * ETALDEMO_NB_CHANNEL_PER_TUNER)
#define ETALDEMO_NB_DATAPATH_MAX        10

#define ETALDEMO_PARAM_DELIMITER        " "
#define ETALDEMO_API_CMD_NB             88

enum {
	ETALDEMO_TUNERCH_NONE = 0xFFFFFFFF,
	ETALDEMO_TUNERCH_FG   = 0x00,
	ETALDEMO_TUNERCH_BG   = 0x01,
	ETALDEMO_TUNERCH_T1FG = 0x10,
	ETALDEMO_TUNERCH_T1BG = 0x11,
	ETALDEMO_TUNERCH_T2FG = 0x20,
	ETALDEMO_TUNERCH_T2BG = 0x21,
} ETALDEMO_TUNERCH_ty;

enum {
	ETALDEMO_SE_STATUS_NONE           = 0x00,
	ETALDEMO_SE_STATUS_SUCCESS        = 0x01,
	ETALDEMO_SE_STATUS_FAILURE        = 0x02,
	ETALDEMO_SE_STATUS_STOPPED        = 0x04,
	ETALDEMO_SE_STATUS_ERR_DELAY      = 0x08,
	ETALDEMO_SE_STATUS_ERR_LOUDNESS   = 0x10,
	ETALDEMO_SE_STATUS_ERR_BUFFERING  = 0x20,
	ETALDEMO_SE_STATUS_ERR_ESTIMATION = 0x40
} ETALDEMO_SE_STATUS_ty;

/*****************************************************************
| type definitions
|----------------------------------------------------------------*/
typedef ETAL_STATUS (*etaldemoApiCmdCb) (tVoid);
typedef struct
{
	char *api_name;
	char *api_short_name;
	int nb_param_min;
	int nb_param_max;
	etaldemoApiCmdCb api_cmd_cb;
} etaldemo_api_cmt_ty;


/*****************************************************************
| variables
|----------------------------------------------------------------*/
ETAL_HANDLE hReceivers[ETALDEMO_NB_TUNER_CHANNEL_MAX + 1];
ETAL_HANDLE hDatapaths[ETALDEMO_NB_DATAPATH_MAX];
ETAL_HANDLE hMonitors[ETALDEMO_NB_DATAPATH_MAX];
EtalSeamlessEstimationStatus etaldemo_seamless_estimation_status = {
	ETAL_INVALID_HANDLE, ETALDEMO_SE_STATUS_NONE, 0, 8888888, 0, 0, 0, 1, 1 };

etaldemo_api_cmt_ty etaldemo_api_cmd_list[ETALDEMO_API_CMD_NB] = {
	{"etal_initialize",                "ei",     4,  4, etaldemo_initialize},
	{"etal_reinitialize",              "er",     0,  0, etaldemo_reinitialize},
	{"etal_deinitialize",              "ed",     0,  0, etaldemo_deinitialize},
	{"etal_config_receiver",           "ecr",    7,  7, etaldemo_config_receiver},
	{"etal_destroy_receiver",          "edr",    1,  1, etaldemo_destroy_receiver},
	{"etal_config_datapath",           "ecd",    3,  3, etaldemo_config_datapath},
	{"etal_destroy_datapath",          "edd",    1,  1, etaldemo_destroy_datapath},
	{"etal_config_audio_path",         "ecap",   5,  5, etaldemo_config_audio_path},
	{"etal_audio_select",              "eas",    2,  2, etaldemo_audio_select},
	{"etal_force_mono",                "efm",    2,  2, etaldemo_force_mono},
	{"etal_mute",                      "em",     2,  2, etaldemo_mute},
	{"etal_event_FM_stereo_start",     "eeFssta",1,  1, etaldemo_event_FM_stereo_start},
	{"etal_event_FM_stereo_stop",      "eeFssto",1,  1, etaldemo_event_FM_stereo_stop},
	{"etal_debug_config_audio_alignment","eedcaa",1, 1, etaldemo_debug_config_audio_alignment},
	{"etal_receiver_alive",            "era",    1,  1, etaldemo_receiver_alive},
	{"etal_xtal_alignment",            "exa",    1,  1, etaldemo_xtal_alignment},
	{"etal_get_version",               "egv",    0,  0, etaldemo_get_version},
	{"etal_get_capabilities",          "egc",    0,  0, etaldemo_get_capabilities},
	{"etal_get_init_status",           "egis",   0,  0, etaldemo_get_init_status},
	{"etal_tune_receiver",             "etr",    2,  2, etaldemo_tune_receiver},
	{"etal_tune_receiver_async",       "etra",   2,  2, etaldemo_tune_receiver_async},
	{"etal_change_band_receiver",      "ecbr",   2,  5, etaldemo_change_band_receiver},
	{"etal_get_current_ensemble",      "egce",   1,  1, etaldemo_get_current_ensemble},
	{"etal_get_ensemble_list",         "egel",   0,  0, etaldemo_get_ensemble_list},
	{"etal_get_ensemble_data",         "eged",   1,  1, etaldemo_get_ensemble_data},
	{"etal_get_fic",                   "egf",    1,  1, etaldemo_get_fic},
	{"etal_get_service_list",          "egsl",   4,  4, etaldemo_get_service_list},
	{"etal_get_specific_service_data_DAB", "egssdD", 2, 2, etaldemo_get_specific_service_data_DAB},
	{"etal_service_select_audio",      "essa",   4,  6, etaldemo_service_select_audio},
	{"etal_service_select_data",       "essd",   4,  6, etaldemo_service_select_data},
	{"etal_get_reception_quality",     "egrq",   1,  1, etaldemo_get_reception_quality},
	{"etal_get_channel_quality",       "egcq",   1,  1, etaldemo_get_channel_quality},
	{"etal_config_reception_quality_monitor", "ecrqm", 67, 67, etaldemo_config_reception_quality_monitor},
	{"etal_destroy_reception_quality_monitor", "edrqm", 1, 1, etaldemo_destroy_reception_quality_monitor},
	{"etal_get_receiver_frequency",    "egrf",   1,  1, etaldemo_get_receiver_frequency},
	{"etal_get_CF_data",               "egCd",   3,  3, etaldemo_get_CF_data},
	{"etal_seek_start_manual",         "esstam", 3,  3, etaldemo_seek_start_manual},
	{"etal_seek_continue_manual",      "escm",   1,  1, etaldemo_seek_continue_manual},
	{"etal_seek_stop_manual",          "esstom", 2,  2, etaldemo_seek_stop_manual},
	{"etal_seek_get_status_manual",    "esgsm",  1,  1, etaldemo_seek_get_status_manual},
	{"etal_autoseek_start",            "essta",  6,  6, etaldemo_autoseek_start},
	{"etal_autoseek_stop",             "essto",  2,  2, etaldemo_autoseek_stop},
	{"etal_set_autoseek_thresholds_value","esstv",8,  8, etaldemo_set_autoseek_thresholds_value},
	{"etal_AF_switch",                 "eAsw",   2,  2, etaldemo_AF_switch},
	{"etal_AF_check",                  "eAc",    3,  3, etaldemo_AF_check},
	{"etal_AF_start",                  "eAst",   4,  4, etaldemo_AF_start},
	{"etal_AF_end",                    "eAe",    2,  2, etaldemo_AF_end},
	{"etal_AF_search_manual",          "eAsm",   3,  3, etaldemo_AF_search_manual},
	{"etal_enable_data_service",       "eeds",   7,  7, etaldemo_enable_data_service},
	{"etal_disable_data_service",      "edds",   2,  2, etaldemo_disable_data_service},
	{"etal_setup_PSD",                 "esP",   14, 14, etaldemo_setup_PSD},
	{"etal_read_parameter",            "erp",    3, 12, etaldemo_read_parameter},
	{"etal_write_parameter",           "ewp",    3, 12, etaldemo_write_parameter},
	{"etal_start_RDS",                 "estaR",  5,  6, etaldemo_start_RDS},
	{"etal_stop_RDS",                  "estoR",  1,  1, etaldemo_stop_RDS},
	{"etaltml_get_textinfo",           "egt",    1,  1, etaltmldemo_get_textinfo},
	{"etaltml_start_textinfo",         "estat",  1,  1, etaltmldemo_start_textinfo},
	{"etaltml_stop_textinfo",          "estot",  1,  1, etaltmldemo_stop_textinfo},
	{"etaltml_get_decoded_RDS",        "egdR",   1,  1, etaltmldemo_get_decoded_RDS},
	{"etaltml_start_decoded_RDS",      "estadR", 2,  2, etaltmldemo_start_decoded_RDS},
	{"etaltml_stop_decoded_RDS",       "estodR", 2,  2, etaltmldemo_stop_decoded_RDS},
	{"etaltml_get_validated_RDS_block_manual","egvRbm",1,1, etaltmldemo_get_validated_RDS_block_manual},
	{"etaltml_RDS_AF",                 "eRA",    3,  3, etaltmldemo_RDS_AF},
	{"etaltml_RDS_TA",                 "eTA",    2,  2, etaltmldemo_RDS_TA},
	{"etaltml_RDS_EON",                "eRE",    2,  2, etaltmldemo_RDS_EON},
	{"etaltml_RDS_REG",                "eRR",    2,  2, etaltmldemo_RDS_REG},
	{"etaltml_RDS_AFSearch_start",     "eRAsta", 2, 28, etaltmldemo_RDS_AFSearch_start},
	{"etaltml_RDS_AFSearch_stop",      "eRAsto", 1,  1, etaltmldemo_RDS_AFSearch_stop},
	{"etaltml_RDS_seek_start",         "eRss",   4,  4, etaltmldemo_RDS_seek_start},
	{"etaltml_scan_start",             "escsta", 4,  4, etaltmldemo_scan_start},
	{"etaltml_scan_stop",              "escsto", 2,  2, etaltmldemo_scan_stop},
	{"etaltml_learn_start",            "elsta",  4,  4, etaltmldemo_learn_start},
	{"etaltml_learn_stop",             "elsto",  2,  2, etaltmldemo_learn_stop},
	{"etaltml_getFreeReceiverForPath", "egFRFP", 2,  2, etaltmldemo_getFreeReceiverForPath},
	{"etaltml_TuneOnServiceId",        "eTOSI",  2,  2, etaltmldemo_TuneOnServiceId},
	{"etaltml_ActivateServiceFollowing","eASF",  0,  0, etaltmldemo_ActivateServiceFollowing},
	{"etaltml_DisableServiceFollowing","eDSF",   0,  0, etaltmldemo_DisableServiceFollowing},
	{"ETALTML_ServiceFollowing_SeamlessEstimationRequest","eSFSER",0,0, etaltmldemo_ServiceFollowing_SeamlessEstimationRequest},
	{"ETALTML_ServiceFollowing_SeamlessEstimationStop","eSFSES",0,0, etaltmldemo_ServiceFollowing_SeamlessEstimationStop},
	{"ETALTML_ServiceFollowing_SeamlessSwitchRequest","eSFSSR",1,1, etaltmldemo_ServiceFollowing_SeamlessSwitchRequest},
	{"etal_seamless_estimation_start", "esesta", 2,  5, etaldemo_seamless_estimation_start},
	{"etal_seamless_estimation_stop" , "esesto", 2,  2, etaldemo_seamless_estimation_stop},
	{"etal_seamless_switching"       , "essw",   2, 10, etaldemo_seamless_switching},
	{"etal_debug_DISS_control"       , "edDc",   4,  4, etaldemo_debug_DISS_control},
	{"etal_debug_get_WSP_Status"     , "edgWS",  1,  1, etaldemo_debug_get_WSP_Status},
	{"etal_debug_VPA_control"        , "edVc",   3,  3, etaldemo_debug_VPA_control},
	{"etal_trace_config"             , "etc",    5, 29, etaldemo_trace_config}
};

/***************************
 *
 * ETALDEMO_STATUS_toString
 *
 **************************/
/*!
 * \brief		Convert an #ETAL_STATUS to a string
 * \param[in]	s - the status to be converted
 * \return		A pointer to the string, or to "Illegal" if *s* does
 * 				not correspond to any known value.
 * \callgraph
 * \callergraph
 */
tCString ETALDEMO_STATUS_toString(ETAL_STATUS s)
{
	switch (s)
	{
		case ETAL_RET_SUCCESS:
			return "Success";
		case ETAL_RET_ERROR:
			return "Error";
		case ETAL_RET_ALREADY_INITIALIZED:
			return "Already Initialized";
		case ETAL_RET_DATAPATH_SINK_ERR:
			return "Datapath Sink error";
		case ETAL_RET_FRONTEND_LIST_ERR:
			return "Invalid Frontend List";
		case ETAL_RET_FRONTEND_NOT_AVAILABLE:
			return "Frontend Not Available";
		case ETAL_RET_INVALID_BCAST_STANDARD:
			return "Invalid Broadcast Standard";
		case ETAL_RET_INVALID_DATA_TYPE:
			return "Invalid Data Type";
		case ETAL_RET_INVALID_HANDLE:
			return "Invalid Handle";
		case ETAL_RET_INVALID_RECEIVER:
			return "Invalid Receiver";
		case ETAL_RET_NO_DATA:
			return "No Data";
		case ETAL_RET_NO_HW_MODULE:
			return "No HW Module";
		case ETAL_RET_NOT_INITIALIZED:
			return "Not Initialized";
		case ETAL_RET_NOT_IMPLEMENTED:
			return "Not Implemented";
		case ETAL_RET_PARAMETER_ERR:
			return "Parameter Error";
		case ETAL_RET_QUAL_CONTAINER_ERR:
			return "Quality Container Error";
		case ETAL_RET_QUAL_FE_ERR:
			return "Quality Frontend Error";
		case ETAL_RET_ALREADY_USED:
			return "Resource Already Configured";
		case ETAL_RET_IN_PROGRESS:
			return "Command in progress";
		default:
			return "Illegal";
	}
}

/***************************
 *
 * standard2Ascii
 *
 **************************/
static char *standard2Ascii(EtalBcastStandard std)
{
	switch (std)
	{
		case ETAL_BCAST_STD_UNDEF:
			return "Undefined";
		case ETAL_BCAST_STD_DRM:
			return "DRM";
		case ETAL_BCAST_STD_DAB:
			return "DAB";
		case ETAL_BCAST_STD_FM:
			return "FM";
		case ETAL_BCAST_STD_HD_FM:
			return "HDFM";
		case ETAL_BCAST_STD_HD_AM:
			return "HDAM";
		case ETAL_BCAST_STD_AM:
			return "AM";
		default:
			return "Illegal";
	}
}

tCString etaldemo_se_strstatus(int se_status)
{
	tCString strstatus;

	switch (se_status)
	{
		case ETALDEMO_SE_STATUS_NONE:
			strstatus = "none";
			break;
		case ETALDEMO_SE_STATUS_SUCCESS:
			strstatus = "success";
			break;
		case ETALDEMO_SE_STATUS_FAILURE:
			strstatus = "failure";
			break;
		case ETALDEMO_SE_STATUS_STOPPED:
			strstatus = "stopped";
			break;
		case ETALDEMO_SE_STATUS_ERR_DELAY:
			strstatus = "error on delay estimation";
			break;
		case ETALDEMO_SE_STATUS_ERR_LOUDNESS:
			strstatus = "error on loudness estimation";
			break;
		case ETALDEMO_SE_STATUS_ERR_BUFFERING:
			strstatus =  "error on buffering";
			break;
		case ETALDEMO_SE_STATUS_ERR_ESTIMATION:
			strstatus = "error on seamless estimation";
			break;
		default:
			strstatus = "invalid status code";
			break;
	}
	return strstatus;
}

tCString etaldemo_dec_etalSeekStatusTy(etalSeekStatusTy status)
{
	tCString strstatus;

	switch (status)
	{
		case ETAL_SEEK_STARTED:
			strstatus = "started";
			break;
		case ETAL_SEEK_RESULT:
			strstatus = "result";
			break;
		case ETAL_SEEK_FINISHED:
			strstatus = "finished";
			break;
		case ETAL_SEEK_ERROR:
			strstatus = "error";
			break;
		case ETAL_SEEK_ERROR_ON_START:
			strstatus = "error on start";
			break;
		case ETAL_SEEK_ERROR_ON_STOP:
			strstatus =  "error on stop";
			break;
		default:
			strstatus = "invalid seek status code";
			break;
	}
	return strstatus;
}

void etaldemo_print_EtalBcastQualityContainer(EtalBcastQualityContainer *quality_result)
{
	EtalFmQualityEntries *amfmQualEntryPtr;
	EtalHdQualityEntries *hdQualEntryPtr;
	EtalDabQualityEntries *dabQualEntryPtr;

	etalDemoPrintf(TR_LEVEL_SYSTEM, "EtalBcastQualityContainer: TimeStamp=%d ms, standard=%s(%d), context=%p", 
		quality_result->m_TimeStamp, standard2Ascii(quality_result->m_standard), quality_result->m_standard, quality_result->m_Context);
	if ((quality_result->m_standard == ETAL_BCAST_STD_FM) || (quality_result->m_standard == ETAL_BCAST_STD_AM) ||
		(quality_result->m_standard == ETAL_BCAST_STD_HD_FM) || (quality_result->m_standard == ETAL_BCAST_STD_HD_AM))
	{
		if ((quality_result->m_standard == ETAL_BCAST_STD_FM) || (quality_result->m_standard == ETAL_BCAST_STD_AM))
		{
			amfmQualEntryPtr = &(quality_result->EtalQualityEntries.amfm);
		}
		else
		{
			amfmQualEntryPtr = &(quality_result->EtalQualityEntries.hd.m_analogQualityEntries);
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, ", RFFS=%d dBuV, BBFS=%d dBuV, FreqOff=%.2f Hz", amfmQualEntryPtr->m_RFFieldStrength, 
			amfmQualEntryPtr->m_BBFieldStrength, ((float)(amfmQualEntryPtr->m_FrequencyOffset) * 50000 / 255));
		if ((quality_result->m_standard == ETAL_BCAST_STD_FM) || (quality_result->m_standard == ETAL_BCAST_STD_HD_FM))
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", Mp=%.2f%%, MPX=%.2f%%", ((float)(amfmQualEntryPtr->m_Multipath) * 100 / 255),
				((float)(amfmQualEntryPtr->m_UltrasonicNoise) * 100 / 255));
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, ", adj=%.2f%%, snr=%.2f%%", 
			(amfmQualEntryPtr->m_AdjacentChannel == 0x80)?(float)-100:((float)((amfmQualEntryPtr->m_AdjacentChannel) * 100 / 127)),
			((float)(amfmQualEntryPtr->m_SNR) * 100 / 255));
		if ((quality_result->m_standard == ETAL_BCAST_STD_FM) || (quality_result->m_standard == ETAL_BCAST_STD_HD_FM))
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", coch=%.2f%%, ST=%d", 
				(amfmQualEntryPtr->m_coChannel >= 0)?((float)(amfmQualEntryPtr->m_coChannel) * 100 / 127):
				((float)(amfmQualEntryPtr->m_coChannel) * 100 / 128), amfmQualEntryPtr->m_StereoMonoReception);
		}
	}
	if ((quality_result->m_standard == ETAL_BCAST_STD_HD_FM) || (quality_result->m_standard == ETAL_BCAST_STD_HD_AM))
	{
		hdQualEntryPtr = &(quality_result->EtalQualityEntries.hd);
		etalDemoPrintf(TR_LEVEL_SYSTEM, ", isDigit=%d", hdQualEntryPtr->m_isValidDigital);
		if (hdQualEntryPtr->m_isValidDigital)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", QI=%d, Cd/No=%d, DSQM=%d, AAA=%d", hdQualEntryPtr->m_QI, hdQualEntryPtr->m_CdToNo, 
				hdQualEntryPtr->m_DSQM, hdQualEntryPtr->m_AudioAlignment);
		}
	}
	if (quality_result->m_standard == ETAL_BCAST_STD_DAB)
	{
		dabQualEntryPtr = &(quality_result->EtalQualityEntries.dab);
		etalDemoPrintf(TR_LEVEL_SYSTEM, ", RFFS=%d dBm, BBFS=%d", dabQualEntryPtr->m_RFFieldStrength, 
			dabQualEntryPtr->m_BBFieldStrength);
		if (dabQualEntryPtr->m_isValidFicBitErrorRatio)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", FicBER=%d", dabQualEntryPtr->m_FicBitErrorRatio);
		}
		if (dabQualEntryPtr->m_isValidMscBitErrorRatio)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", MscBER=%d", dabQualEntryPtr->m_MscBitErrorRatio);
		}
		if (dabQualEntryPtr->m_isValidDataSubChBitErrorRatio)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", dataSubChBER=%d", dabQualEntryPtr->m_dataSubChBitErrorRatio);
		}
		if (dabQualEntryPtr->m_isValidAudioSubChBitErrorRatio)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, ", audioSubChBER=%d", dabQualEntryPtr->m_audioSubChBitErrorRatio);
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, ", audioBERL=%u, RS=%u, sync=%u, mute=%u", dabQualEntryPtr->m_audioBitErrorRatioLevel, 
			dabQualEntryPtr->m_reedSolomonInformation, dabQualEntryPtr->m_syncStatus, dabQualEntryPtr->m_muteFlag);
	}
	etalDemoPrintf(TR_LEVEL_SYSTEM, "\n");
}

void etaldemo_print_EtalTextInfo(EtalTextInfo *Radiotext)
{
	etalDemoPrintf(TR_LEVEL_SYSTEM, "BCastStandard=%d (%s) serviceNameIsNew=%d (%.*s) currentInfoIsNew=%d (%.*s)\n", 
		Radiotext->m_broadcastStandard, standard2Ascii(Radiotext->m_broadcastStandard), Radiotext->m_serviceNameIsNew, 
		ETAL_DEF_MAX_SERVICENAME, Radiotext->m_serviceName, Radiotext->m_currentInfoIsNew, ETAL_DEF_MAX_INFO, 
		Radiotext->m_currentInfo);
}

void etaldemo_print_EtalRDSData(EtalRDSData *RDSdata)
{
	tU32 i;

	etalDemoPrintf(TR_LEVEL_SYSTEM, "validityBitmap=0x%x PS=%.*s DI=%d PI=%d PTY=%d TP=%d TA=%d MS=%d time=%d:%d offset=%d MJD=%d RT=%.*s AFListLen=%d AFListPI=%d AFList=", 
		RDSdata->m_validityBitmap, ETAL_DEF_MAX_PS_LEN, RDSdata->m_PS, RDSdata->m_DI, RDSdata->m_PI,
		RDSdata->m_PTY, RDSdata->m_TP, RDSdata->m_TA, RDSdata->m_MS, RDSdata->m_timeHour, RDSdata->m_timeMinutes,
		RDSdata->m_offset, RDSdata->m_MJD, ETAL_DEF_MAX_RT_LEN, RDSdata->m_RT, RDSdata->m_AFListLen, RDSdata->m_AFListPI);
	for(i = 0; i < ETAL_DEF_MAX_AFLIST; i++)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, " %d", RDSdata->m_AFList[i]);
	}
	etalDemoPrintf(TR_LEVEL_SYSTEM, "\n");
}

void etaldemo_print_EtalRDSRawData(tU32 len, EtalRDSRawData *RDSRawData)
{
	tU32 i, RNR, RDS_Data;

	if (len >= 3)
	{
		RNR = (((tU32)(RDSRawData->m_RNR[0]) << 16) & 0xFF0000) | (((tU32)(RDSRawData->m_RNR[1]) << 8) & 0xFF00) | ((tU32)(RDSRawData->m_RNR[2]) & 0xFF);
		etalDemoPrintf(TR_LEVEL_SYSTEM, "RNR=%s %s %s %s %s %s", ((RNR & STAR_RNR_DATARDY)?"DATARDY":"       "), ((RNR & STAR_RNR_SYNC)?"SYNC":"    "), 
			((RNR & STAR_RNR_BOFL)?"BOFL":"    "), ((RNR & STAR_RNR_BNE)?"BNE":"   "), ((RNR & STAR_RNR_TUNCH2)?"TUNCH2":"      "), 
			((RNR & STAR_RNR_TUNCH1)?"TUNCH1":"      "));
	}
	for(i = 0; i < STAR_RDS_MAX_BLOCK; i++)
	{
		if (((i * 3) + 3) < len)
		{
			RDS_Data = (((tU32)(RDSRawData->m_RDS_Data[(i * 3)]) << 16) & 0xFF0000) | (((tU32)(RDSRawData->m_RDS_Data[((i * 3) + 1)]) << 8) & 0xFF00) | ((tU32)(RDSRawData->m_RDS_Data[((i * 3) + 2)]) & 0xFF);
			etalDemoPrintf(TR_LEVEL_SYSTEM, " ERRCOUNT: %d, RDS block offset %c%c, RDS raw data: 0x%04X", ((RDS_Data & START_RDS_DATA_ERRCOUNT_MASK) >> START_RDS_DATA_ERRCOUNT_SHIFT),
				((RDS_Data & START_RDS_DATA_BLOCKID_MASK) >> START_RDS_DATA_BLOCKID_SHIFT) + 65,
				((RDS_Data & START_RDS_DATA_CTYPE_MASK) >> START_RDS_DATA_CTYPE_SHIFT)?'\'':' ',
				(RDS_Data & START_RDS_DATA_MASK));
		}
	}
}

ETAL_STATUS etaldemo_initialize(tVoid)
{
	tChar *ptok;
	EtalHardwareAttr HWInit;
	int i;

	memset(&HWInit, 0, sizeof(HWInit));
	HWInit.m_cbNotify = etalDemo_userNotificationHandler;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		HWInit.m_CountryVariant = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		HWInit.m_DCOPAttr.m_isDisabled = atoi(ptok);
	}
	for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			HWInit.m_tunerAttr[i].m_isDisabled = atoi(ptok);
		}
	}
	return etal_initialize(&HWInit);
}

ETAL_STATUS etaldemo_reinitialize(tVoid)
{
	tChar *ptok;
	EtalNVMLoadConfig NVMLoadConfig;

	memset(&NVMLoadConfig, 0, sizeof(NVMLoadConfig));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		NVMLoadConfig.m_load_DAB_landscape = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		NVMLoadConfig.m_load_AMFM_landscape = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		NVMLoadConfig.m_load_HD_landscape = atoi(ptok);
	}
	return etal_reinitialize(NVMLoadConfig);
}

ETAL_STATUS etaldemo_deinitialize(tVoid)
{
	return etal_deinitialize();
}

ETAL_STATUS etaldemo_config_receiver(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, i;
	EtalReceiverAttr ReceiverConfig;
	ETAL_HANDLE frontEnd;

	hReceiver_idx = 0;
	memset(&ReceiverConfig, 0, sizeof(ReceiverConfig));
	ReceiverConfig.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_UNSPECIFIED;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ReceiverConfig.m_Standard = atoi(ptok);
	}
	
	for(i = 0; i < ETAL_CAPA_MAX_FRONTEND; i++)
	{
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			frontEnd = atoi(ptok);
			if (frontEnd != ETAL_INVALID_HANDLE)
			{
				ReceiverConfig.m_FrontEnds[ReceiverConfig.m_FrontEndsSize] = frontEnd;
				ReceiverConfig.m_FrontEndsSize++;
			}
		}
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ReceiverConfig.processingFeatures.u.m_processing_features = atoi(ptok);
	}
	return etal_config_receiver(&hReceivers[hReceiver_idx], &ReceiverConfig);
}

ETAL_STATUS etaldemo_destroy_receiver(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etal_destroy_receiver(&hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaldemo_config_datapath(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, hDatapath_idx;
	EtalDataPathAttr DpathConfig;

	hReceiver_idx = hDatapath_idx = 0;
	memset(&DpathConfig, 0, sizeof(DpathConfig));
	DpathConfig.m_sink.m_CbProcessBlock = etalDemo_defaultDataCallback;
	/* use parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hDatapath_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
		DpathConfig.m_receiverHandle = hReceivers[hReceiver_idx];
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		DpathConfig.m_dataType = (EtalBcastDataType)atoi(ptok);
	}
	if (DpathConfig.m_dataType == ETAL_DATA_TYPE_TEXTINFO)
	{
		DpathConfig.m_sink.m_CbProcessBlock = etalDemo_textinfoDatPathCallback;
	}
	else if (DpathConfig.m_dataType == ETAL_DATA_TYPE_FM_RDS)
	{
		DpathConfig.m_sink.m_CbProcessBlock = etalDemo_fmRdsDatPathCallback;
	}
	else if (DpathConfig.m_dataType == ETAL_DATA_TYPE_FM_RDS_RAW)
	{
		DpathConfig.m_sink.m_CbProcessBlock = etalDemo_rdsRawDatPathCallback;
	}
	return etal_config_datapath(&hDatapaths[hDatapath_idx], &DpathConfig);
}

ETAL_STATUS etaldemo_destroy_datapath(tVoid)
{
	tChar *ptok;
	tU32 hDatapath_idx;

	hDatapath_idx = 0;
	/* use datapath parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hDatapath_idx = atoi(ptok);
	}
	return etal_destroy_datapath(&hDatapaths[hDatapath_idx]);
}

ETAL_STATUS etaldemo_config_audio_path(tVoid)
{
	tChar *ptok;
	tU32 tunerIndex;
	EtalAudioInterfTy intf;

	tunerIndex = 0;
	memset(&intf, 0, sizeof(intf));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		tunerIndex = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		intf.m_dac = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		intf.m_sai_out = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		intf.m_sai_in = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		intf.m_sai_slave_mode = atoi(ptok);
	}
	return etal_config_audio_path(tunerIndex, intf);
}

ETAL_STATUS etaldemo_audio_select(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalAudioSourceTy src;

	hReceiver_idx = 0;
	src = ETAL_AUDIO_SOURCE_STAR_AMFM;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		src = atoi(ptok);
	}
	return etal_audio_select(hReceivers[hReceiver_idx], src);
}

ETAL_STATUS etaldemo_force_mono(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tBool forceMonoFlag;

	hReceiver_idx = 0;
	forceMonoFlag = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		forceMonoFlag = atoi(ptok);
	}
	return etal_force_mono(hReceivers[hReceiver_idx], forceMonoFlag);
}

ETAL_STATUS etaldemo_mute(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tBool muteFlag;

	hReceiver_idx = 0;
	muteFlag = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		muteFlag = atoi(ptok);
	}
	return etal_mute(hReceivers[hReceiver_idx], muteFlag);
}

ETAL_STATUS etaldemo_event_FM_stereo_start(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etal_event_FM_stereo_start(hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaldemo_event_FM_stereo_stop(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etal_event_FM_stereo_stop(hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaldemo_debug_config_audio_alignment(tVoid)
{
	tChar *ptok;
	EtalAudioAlignmentAttr alignmentParams;

	alignmentParams.m_enableAutoAlignmentForFM = 0;
	alignmentParams.m_enableAutoAlignmentForAM = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		alignmentParams.m_enableAutoAlignmentForFM = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		alignmentParams.m_enableAutoAlignmentForAM = atoi(ptok);
	}
	return etal_debug_config_audio_alignment(&alignmentParams);
}

ETAL_STATUS etaldemo_receiver_alive(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etal_receiver_alive(hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaldemo_xtal_alignment(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 calculatedAdjustment;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	calculatedAdjustment = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_xtal_alignment(hReceivers[hReceiver_idx], &calculatedAdjustment);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "xtal alignement calculatedAdjustment: %u\n", calculatedAdjustment);
	}
	return ret;
}

ETAL_STATUS etaldemo_get_version(tVoid)
{
	EtalVersion vers;
	ETAL_STATUS ret;
	int i;

	memset(&vers, 0, sizeof(vers));
	ret = etal_get_version(&vers);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (vers.m_ETAL.m_isValid)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "ETAL version %d.%d.%d build %d %s\n", 
				vers.m_ETAL.m_major, vers.m_ETAL.m_middle, vers.m_ETAL.m_minor, vers.m_ETAL.m_build, vers.m_ETAL.m_name);
		}
		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			if (vers.m_CMOST[i].m_isValid)
			{
				etalDemoPrintf(TR_LEVEL_SYSTEM, "CMOST[%d] version %d.%d.%d build %d %s\n", 
					i, vers.m_CMOST[i].m_major, vers.m_CMOST[i].m_middle, vers.m_CMOST[i].m_minor, vers.m_CMOST[i].m_build, vers.m_CMOST[i].m_name);
			}
		}
		if (vers.m_MDR.m_isValid)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "MDR version %d.%d.%d build %d %s\n", 
				vers.m_MDR.m_major, vers.m_MDR.m_middle, vers.m_MDR.m_minor, vers.m_MDR.m_build, vers.m_MDR.m_name);
		}
		if (vers.m_HDRadio.m_isValid)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "HDR version %d.%d.%d build %d %s\n", 
				vers.m_HDRadio.m_major, vers.m_HDRadio.m_middle, vers.m_HDRadio.m_minor, vers.m_HDRadio.m_build, vers.m_HDRadio.m_name);
		}
	}
	return ret;
}

ETAL_STATUS etaldemo_get_capabilities(tVoid)
{
	EtalHwCapabilities *pCapabilities;
	ETAL_STATUS ret;
	int i, j;

	ret = etal_get_capabilities(&pCapabilities);
	if (ret == ETAL_RET_SUCCESS)
	{
		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "Tuner[%d] (TunerDevice(deviceType %d busType %d busAddress 0x%x channels %d) standards(", 
				i, pCapabilities->m_Tuner[i].m_TunerDevice.m_deviceType, pCapabilities->m_Tuner[i].m_TunerDevice.m_busType,
				pCapabilities->m_Tuner[i].m_TunerDevice.m_busAddress, pCapabilities->m_Tuner[i].m_TunerDevice.m_channels);
			for(j = 0; j < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; j++)
			{
				if (j == 0)
					etalDemoPrintf(TR_LEVEL_SYSTEM, "0x%x", pCapabilities->m_Tuner[i].m_standards[j]);
				else
					etalDemoPrintf(TR_LEVEL_SYSTEM, " 0x%x", pCapabilities->m_Tuner[i].m_standards[j]);
			}
			etalDemoPrintf(TR_LEVEL_SYSTEM, ") dataType(");
			for(j = 0; j < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; j++)
			{
				if (j == 0)
					etalDemoPrintf(TR_LEVEL_SYSTEM, "0x%x", pCapabilities->m_Tuner[i].m_dataType[j]);
				else
					etalDemoPrintf(TR_LEVEL_SYSTEM, " 0x%x", pCapabilities->m_Tuner[i].m_dataType[j]);
			}
			etalDemoPrintf(TR_LEVEL_SYSTEM, ")\n");
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, "DCOP (deviceType %d busType %d busAddress 0x%x channels %d)\n", 
			pCapabilities->m_DCOP.m_deviceType, pCapabilities->m_DCOP.m_busType,
			pCapabilities->m_DCOP.m_busAddress, pCapabilities->m_DCOP.m_channels);
	}
	return ret;
}

ETAL_STATUS etaldemo_get_init_status(tVoid)
{
	EtalInitStatus status;
	ETAL_STATUS ret;
	int i;

	memset(&status, 0, sizeof(status));
	ret = etal_get_init_status(&status);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "lastInitState %d warningStatus %d", 
			status.m_lastInitState, status.m_warningStatus);
		for(i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, " tunerStatus[%d](deviceStatus %d expectedSilicon %s, detectedSilicon %s)", 
				i, status.m_tunerStatus[i].m_deviceStatus, status.m_tunerStatus[i].m_expectedSilicon,
				status.m_tunerStatus[i].m_detectedSilicon);
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, " DCOPStatus(deviceStatus %d expectedSilicon %s, detectedSilicon %s)", 
			status.m_DCOPStatus.m_deviceStatus, status.m_DCOPStatus.m_expectedSilicon, status.m_DCOPStatus.m_detectedSilicon);
	}
	return ret;
}

ETAL_STATUS etaldemo_tune_receiver(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 Frequency;

	hReceiver_idx = 0;
	Frequency = ETAL_INVALID_FREQUENCY;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		Frequency = atoi(ptok);
	}
	return etal_tune_receiver(hReceivers[hReceiver_idx], Frequency);
}

ETAL_STATUS etaldemo_tune_receiver_async(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 Frequency;

	hReceiver_idx = 0;
	Frequency = ETAL_INVALID_FREQUENCY;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		Frequency = atoi(ptok);
	}
	return etal_tune_receiver_async(hReceivers[hReceiver_idx], Frequency);
}

ETAL_STATUS etaldemo_change_band_receiver(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 band, fmin, fmax;
	EtalProcessingFeatures processingFeatures;

	hReceiver_idx = 0;
	band = ETAL_BAND_UNDEF;
	fmin = fmax = 0;
	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_UNSPECIFIED;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		band = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		fmin = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		fmax = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		processingFeatures.u.m_processing_features = (tU8)atoi(ptok);
	}
	return etal_change_band_receiver(hReceivers[hReceiver_idx], band, fmin, fmax, processingFeatures);
}

ETAL_STATUS etaldemo_get_current_ensemble(tVoid)
{
	ETAL_STATUS ret = ETAL_RET_NOT_IMPLEMENTED;
#if defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 UEId;

	hReceiver_idx = 0;
	UEId = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_get_current_ensemble(hReceivers[hReceiver_idx], &UEId);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "UEId: %d (0x%x)\n", UEId, UEId);
	}
#endif
#endif
	return ret;
}

ETAL_STATUS etaldemo_get_ensemble_list(tVoid)
{
	ETAL_STATUS ret = ETAL_RET_NOT_IMPLEMENTED;
#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tU32 i;
	EtalEnsembleList EnsembleList;

	memset(&EnsembleList, 0, sizeof(EnsembleList));
	ret = etal_get_ensemble_list(&EnsembleList);
	if (ret == ETAL_RET_SUCCESS)
	{
		for(i = 0; i < EnsembleList.m_ensembleCount; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "Ecc=0x%x EId=0x%x Freq=%d ", 
				EnsembleList.m_ensemble[i].m_ECC, EnsembleList.m_ensemble[i].m_ensembleId,
				EnsembleList.m_ensemble[i].m_frequency);
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, "\n");
	}
#endif
#endif
	return ret;
}

ETAL_STATUS etaldemo_get_ensemble_data(tVoid)
{
	ETAL_STATUS ret = ETAL_RET_NOT_IMPLEMENTED;
#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tChar *ptok;
	tU32 eid;
	tU8 charset;
	tChar label[ETAL_DEF_MAX_LABEL_LEN];
	tU16 bitmap;

	eid = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		eid = atoi(ptok);
	}
	ret = etal_get_ensemble_data(eid, &charset, label, &bitmap);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "EId=0x%x charset=%d label=%s bitmap=0x%x\n", 
			eid, charset, label, bitmap);
	}
#endif
#endif
	return ret;
}

ETAL_STATUS etaldemo_get_fic(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etal_get_fic(hReceivers[hReceiver_idx]);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaldemo_get_service_list(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 eid, i;
	tBool bGetAudioServices;
	tBool bGetDataServices;
	EtalServiceList ServiceList;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	eid = 0;
	bGetAudioServices = bGetDataServices = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		eid = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		bGetAudioServices = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		bGetDataServices = atoi(ptok);
	}
	ret = etal_get_service_list(hReceivers[hReceiver_idx], eid, bGetAudioServices, bGetDataServices, &ServiceList);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "m_serviceCount: %d (0x%x)\n", ServiceList.m_serviceCount, ServiceList.m_serviceCount);
		for(i = 0; i < ServiceList.m_serviceCount; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "m_service[%d]: %d (0x%x)\n", i, ServiceList.m_service[i], ServiceList.m_service[i]);
		}
	}
	return ret;
}

ETAL_STATUS etaldemo_get_specific_service_data_DAB(tVoid)
{
	ETAL_STATUS ret = ETAL_RET_NOT_IMPLEMENTED;
#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tChar *ptok;
	tU32 eid, sid, dummy, i;
	EtalServiceInfo serv_info;
	EtalServiceComponentList sclist;

	eid = sid = 0;
	memset(&serv_info, 0, sizeof(serv_info));
	memset(&sclist, 0, sizeof(sclist));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		eid = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		sid = atoi(ptok);
	}
	ret = etal_get_specific_service_data_DAB(eid, sid, &serv_info, &sclist, &dummy);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "svcBitrate=%d subchId=%d pktAddr=0x%x svcLanguage=%d cType=%d sType=%d scCnt=%d svcLblCharset=%d svcLbl=%s svcLblCharflag=%d\n", 
			serv_info.m_serviceBitrate, serv_info.m_subchId, serv_info.m_packetAddress, serv_info.m_serviceLanguage, serv_info.m_componentType,
			serv_info.m_streamType, serv_info.m_scCount, serv_info.m_serviceLabelCharset, serv_info.m_serviceLabel, serv_info.m_serviceLabelCharflag);
		for(i = 0; i < sclist.m_scCount; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "scIndex=%d datSvcType=%d scType=%d scLblCharset=%d scLbl=%s scLblCharflag=%d\n", 
				sclist.m_scInfo[i].m_scIndex, sclist.m_scInfo[i].m_dataServiceType, sclist.m_scInfo[i].m_scType, 
				sclist.m_scInfo[i].m_scLabelCharset, sclist.m_scInfo[i].m_scLabel, sclist.m_scInfo[i].m_scLabelCharflag);
		}
	}
#endif
#endif
	return ret;
}

ETAL_STATUS etaldemo_service_select_audio(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalServiceSelectMode mode;
	tU32 UEId, service;
	tSInt sc, subch;

	hReceiver_idx = 0;
	mode = ETAL_SERVSEL_MODE_SERVICE;
	UEId = ETAL_INVALID_UEID;
	service = ETAL_INVALID_SID;
	sc = subch = ETAL_INVALID;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		UEId = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		service = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		sc = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		subch = atoi(ptok);
	}
	return etal_service_select_audio(hReceivers[hReceiver_idx], mode, UEId, service, sc, subch);
}

ETAL_STATUS etaldemo_service_select_data(tVoid)
{
	tChar *ptok;
	tU32 hDatapath_idx;
	EtalServiceSelectMode mode;
	EtalServiceSelectSubFunction type;
	tU32 UEId, service;
	tSInt sc, subch;

	hDatapath_idx = 0;
	mode = ETAL_SERVSEL_MODE_SERVICE;
	type = ETAL_SERVSEL_SUBF_SET;
	UEId = ETAL_INVALID_UEID;
	service = ETAL_INVALID_SID;
	sc = subch = ETAL_INVALID;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hDatapath_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		type = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		UEId = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		service = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		sc = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		subch = atoi(ptok);
	}
	return etal_service_select_data(hDatapaths[hDatapath_idx], mode, type, UEId, service, sc, subch);
}

ETAL_STATUS etaldemo_get_reception_quality(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalBcastQualityContainer BcastQuality;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	memset(&BcastQuality, 0, sizeof(BcastQuality));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_get_reception_quality(hReceivers[hReceiver_idx], &BcastQuality);
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&(BcastQuality)));
	}
	return ret;
}

ETAL_STATUS etaldemo_get_channel_quality(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalBcastQualityContainer BcastQuality[2];
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	memset(BcastQuality, 0, sizeof(BcastQuality));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_get_channel_quality(hReceivers[hReceiver_idx], BcastQuality);
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&(BcastQuality[0])));
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&(BcastQuality[1])));
	}
	return ret;
}

ETAL_STATUS etaldemo_config_reception_quality_monitor(tVoid)
{
	tChar *ptok;
	tU32 hMonitor_idx, i;
	EtalBcastQualityMonitorAttr Monitor;

	hMonitor_idx = 0;
	memset(&Monitor, 0, sizeof(Monitor));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hMonitor_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		Monitor.m_receiverHandle = hReceivers[atoi(ptok)];
	}
	for(i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
	{
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			Monitor.m_monitoredIndicators[i].m_MonitoredIndicator = atoi(ptok);
		}
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			Monitor.m_monitoredIndicators[i].m_InferiorValue = atoi(ptok);
		}
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			Monitor.m_monitoredIndicators[i].m_SuperiorValue = atoi(ptok);
		}
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			Monitor.m_monitoredIndicators[i].m_UpdateFrequency = atoi(ptok);
		}
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		Monitor.m_Context = (tVoid *)(long)atoi(ptok);
	}
	return etal_config_reception_quality_monitor(&(hMonitors[hMonitor_idx]), &Monitor);
}

ETAL_STATUS etaldemo_destroy_reception_quality_monitor(tVoid)
{
	tChar *ptok;
	tU32 hMonitor_idx;

	hMonitor_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hMonitor_idx = atoi(ptok);
	}
	return etal_destroy_reception_quality_monitor(&(hMonitors[hMonitor_idx]));
}

ETAL_STATUS etaldemo_get_receiver_frequency(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, Frequency;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	Frequency = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_get_receiver_frequency(hReceivers[hReceiver_idx], &Frequency);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Frequency=%d", Frequency);
	}
	return ret;
}

ETAL_STATUS etaldemo_get_CF_data(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, nbOfAverage, period;
	EtalCFDataContainer Resp;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	nbOfAverage = 0;
	period = 0;
	memset(&Resp, 0, sizeof(Resp));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		nbOfAverage = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		period = atoi(ptok);
	}
	ret = etal_get_CF_data(hReceivers[hReceiver_idx], &Resp, nbOfAverage, period);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Frequency=%d, Band=0x%x ", Resp.m_CurrentFrequency, Resp.m_CurrentBand);
		etaldemo_print_EtalBcastQualityContainer(&(Resp.m_QualityContainer));
	}
	return ret;
}

ETAL_STATUS etaldemo_seek_start_manual(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 step, freq;
	etalSeekDirectionTy direction;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	direction = cmdDirectionUp;
	step = 0;
	freq = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		direction = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		step = atoi(ptok);
	}
	ret = etal_seek_start_manual(hReceivers[hReceiver_idx], direction, step, (tU32 *)(&freq));
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Seek freq=%d Hz\n", freq);
	}
	return ret;
}

ETAL_STATUS etaldemo_seek_continue_manual(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 freq;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	freq = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_seek_continue_manual(hReceivers[hReceiver_idx], (tU32 *)(&freq));
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Seek freq=%d Hz\n", freq);
	}
	return ret;
}

ETAL_STATUS etaldemo_seek_stop_manual(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 freq;
	etalSeekAudioTy exitSeekAction;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	exitSeekAction = cmdAudioUnmuted;
	freq = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		exitSeekAction = atoi(ptok);
	}
	ret = etal_seek_stop_manual(hReceivers[hReceiver_idx], exitSeekAction, (tU32 *)(&freq));
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Seek freq=%d Hz\n", freq);
	}
	return ret;
}

ETAL_STATUS etaldemo_seek_get_status_manual(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalSeekStatus seekStatus;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	memset(&seekStatus, 0, sizeof(seekStatus));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_seek_get_status_manual(hReceivers[hReceiver_idx], (EtalSeekStatus *)(&seekStatus));
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Seek status: recvHandle=%d, status=%d(%s), freq=%d Hz, freqfound=%d, HDprogfound=%d, fullcycle=%d, SvcId=0x%x ",
			seekStatus.m_receiverHandle, seekStatus.m_status, etaldemo_dec_etalSeekStatusTy(seekStatus.m_status), seekStatus.m_frequency,
			seekStatus.m_frequencyFound, seekStatus.m_HDProgramFound, seekStatus.m_fullCycleReached, seekStatus.m_serviceId);
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&(seekStatus.m_quality)));
	}
	return ret;
}

ETAL_STATUS etaldemo_autoseek_start(tVoid)
{
#if (defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_AUTOSEEK))
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 step;
	etalSeekDirectionTy direction;
	etalSeekAudioTy exitSeekAction;
	etalSeekHdModeTy seekHDSPS;
	tBool updateStopFrequency;

	hReceiver_idx = 0;
	direction = cmdDirectionUp;
	step = 0;
	exitSeekAction = cmdAudioUnmuted;
	seekHDSPS = dontSeekInSPS;
	updateStopFrequency = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		direction = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		step = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		exitSeekAction = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekHDSPS = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		updateStopFrequency = atoi(ptok);
	}
	return etal_autoseek_start(hReceivers[hReceiver_idx], direction, step, exitSeekAction, seekHDSPS, updateStopFrequency);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaldemo_autoseek_stop(tVoid)
{
#if (defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_AUTOSEEK))
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalSeekTerminationModeTy terminationMode;

	hReceiver_idx = 0;
	terminationMode = initialFrequency;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		terminationMode = atoi(ptok);
	}
	return etal_autoseek_stop(hReceivers[hReceiver_idx], terminationMode);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaldemo_set_autoseek_thresholds_value(tVoid)
{
#if (defined (CONFIG_ETAL_HAVE_ALL_API) || defined (CONFIG_ETAL_HAVE_AUTOSEEK))
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalSeekThreshold seekThreshold;

	hReceiver_idx = 0;
	memset(&seekThreshold, 0, sizeof(seekThreshold));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdBBFieldStrength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdDetune = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdAdjacentChannel = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdMultipath = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdSignalNoiseRatio = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdMpxNoise = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seekThreshold.SeekThresholdCoChannel = atoi(ptok);
	}
	return etal_set_autoseek_thresholds_value(hReceivers[hReceiver_idx], (EtalSeekThreshold *)&seekThreshold);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaldemo_AF_switch(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 alternateFrequency;

	hReceiver_idx = 0;
	alternateFrequency = ETAL_INVALID_UEID;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		alternateFrequency = atoi(ptok);
	}
	return etal_AF_switch(hReceivers[hReceiver_idx], alternateFrequency);
}

ETAL_STATUS etaldemo_AF_check(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 alternateFrequency, antennaSelection;
	EtalBcastQualityContainer result;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	alternateFrequency = ETAL_INVALID_UEID;
	antennaSelection = 0;
	memset(&result, 0, sizeof(result));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		alternateFrequency = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		antennaSelection = atoi(ptok);
	}
	ret = etal_AF_check(hReceivers[hReceiver_idx], alternateFrequency, antennaSelection, (EtalBcastQualityContainer *)(&result));
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&result));
	}
	return ret;
}

ETAL_STATUS etaldemo_AF_start(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	etalAFModeTy AFMode;
	tU32 alternateFrequency, antennaSelection;
	EtalBcastQualityContainer result;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	AFMode = cmdRestartAFMeasurement;
	alternateFrequency = ETAL_INVALID_UEID;
	antennaSelection = 0;
	memset(&result, 0, sizeof(result));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		AFMode = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		alternateFrequency = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		antennaSelection = atoi(ptok);
	}
	ret = etal_AF_start(hReceivers[hReceiver_idx], AFMode, alternateFrequency, antennaSelection, (EtalBcastQualityContainer *)(&result));
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&result));
	}
	return ret;
}

ETAL_STATUS etaldemo_AF_end(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 alternateFrequency;
	EtalBcastQualityContainer result;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	alternateFrequency = ETAL_INVALID_UEID;
	memset(&result, 0, sizeof(result));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		alternateFrequency = atoi(ptok);
	}
	ret = etal_AF_end(hReceivers[hReceiver_idx], alternateFrequency, (EtalBcastQualityContainer *)(&result));
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&result));
	}
	return ret;
}

ETAL_STATUS etaldemo_AF_search_manual(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 i, *AFList, nbOfAF, antennaSelection;
	EtalBcastQualityContainer *AFQualityList;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	AFList = (tU32 *)NULL;
	AFQualityList = (EtalBcastQualityContainer *)NULL;
	nbOfAF = 0;
	antennaSelection = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		antennaSelection = atoi(ptok);
	}
	while ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		if (nbOfAF == 0)
		{
			AFList = (tU32 *)malloc(sizeof(tU32));
			AFQualityList = (EtalBcastQualityContainer *)malloc(sizeof(EtalBcastQualityContainer));
		}
		else
		{
			AFList = (tU32 *)realloc(AFList, (sizeof(tU32) * (nbOfAF + 1)));
			AFQualityList = (EtalBcastQualityContainer *)realloc(AFQualityList, (sizeof(EtalBcastQualityContainer) * (nbOfAF + 1)));
		}
		if ((AFList == NULL) || (AFQualityList == NULL))
		{
			if (AFList != NULL)
				free(AFList);
			if (AFQualityList != NULL)
				free(AFQualityList);
			return ETAL_RET_ERROR;
		}
		AFList[nbOfAF] = atoi(ptok);
		nbOfAF++;
	}
	memset(AFQualityList, 0, (sizeof(EtalBcastQualityContainer) * nbOfAF));
	ret = etal_AF_search_manual(hReceivers[hReceiver_idx], antennaSelection, AFList, nbOfAF, AFQualityList);
	if (ret == ETAL_RET_SUCCESS)
	{
		for(i = 0; i < nbOfAF; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "AFList[%d] freq=%d Hz ", i, AFList[i]);
			etaldemo_print_EtalBcastQualityContainer((EtalBcastQualityContainer *)(&(AFQualityList[i])));
		}
	}
	if (AFList != NULL)
		free(AFList);
	if (AFQualityList != NULL)
		free(AFQualityList);
	return ret;
}

ETAL_STATUS etaldemo_enable_data_service(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 ServiceBitmap, EnabledServiceBitmap;
	EtalDataServiceParam ServiceParameters;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	ServiceBitmap = EnabledServiceBitmap = 0;
	memset(&ServiceParameters, 0, sizeof(ServiceParameters));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceBitmap = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceParameters.m_ecc = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceParameters.m_eid = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceParameters.m_sid = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceParameters.m_logoType = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceParameters.m_JMLObjectId = atoi(ptok);
	}
	ret = etal_enable_data_service(hReceivers[hReceiver_idx], ServiceBitmap, &EnabledServiceBitmap, ServiceParameters);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "EnabledServiceBitmap=0x%x", EnabledServiceBitmap);
	}
	return ret;
}

ETAL_STATUS etaldemo_disable_data_service(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU32 ServiceBitmap;

	hReceiver_idx = 0;
	ServiceBitmap = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ServiceBitmap = atoi(ptok);
	}
	return etal_disable_data_service(hReceivers[hReceiver_idx], ServiceBitmap);
}

ETAL_STATUS etaldemo_setup_PSD(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tU16 PSDServiceEnableBitmap;
	EtalPSDLength *pConfigLenSet, ConfigLenSet, ConfigLenGet;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	PSDServiceEnableBitmap = 0;
	pConfigLenSet = NULL;
	memset(&ConfigLenSet, 0, sizeof(ConfigLenSet));
	memset(&ConfigLenGet, 0, sizeof(ConfigLenGet));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		PSDServiceEnableBitmap = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		pConfigLenSet = &ConfigLenSet;
		ConfigLenSet.m_PSDTitleLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDArtistLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDAlbumLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDGenreLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDCommentShortLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDCommentLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDUFIDLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDCommercialPriceLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDCommercialContactLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDCommercialSellerLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDCommercialDescriptionLength = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		ConfigLenSet.m_PSDXHDRLength = atoi(ptok);
	}
	ret = etal_setup_PSD(hReceivers[hReceiver_idx], PSDServiceEnableBitmap, pConfigLenSet, &ConfigLenGet);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "len PSD Title=%d Artiste=%d Album=%d Genre=%d CommentShort=%d Comment=%d UFID=%d CommercialPrice=%d ComercialContact=%d ComercialSeller=%d CommercialDescription=%d XHDR=%d", 
			ConfigLenGet.m_PSDTitleLength, ConfigLenGet.m_PSDArtistLength, ConfigLenGet.m_PSDAlbumLength, ConfigLenGet.m_PSDGenreLength, 
			ConfigLenGet.m_PSDCommentShortLength, ConfigLenGet.m_PSDCommentLength, ConfigLenGet.m_PSDUFIDLength, 
			ConfigLenGet.m_PSDCommercialPriceLength, ConfigLenGet.m_PSDCommercialContactLength, ConfigLenGet.m_PSDCommercialSellerLength, 
			ConfigLenGet.m_PSDCommercialDescriptionLength, ConfigLenGet.m_PSDXHDRLength);
	}
	return ret;
}

ETAL_STATUS etaldemo_read_parameter(tVoid)
{
	tChar *ptok;
	tU32 tunerIndex, *param, *response, i;
	tU16 length, responseLength;
	etalReadWriteModeTy mode;
	ETAL_STATUS ret;

	tunerIndex = 0;
	param = response = NULL;
	mode = fromIndex;
	length = responseLength = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		tunerIndex = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	while ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		if (param == NULL)
		{
			param = (tU32 *)malloc(sizeof(tU32) * (length + 1));
		}
		else
		{
			param = (tU32 *)realloc(param, sizeof(tU32) * (length + 1));
		}
		param[length] = atoi(ptok);
		length++;
	}
	if (length)
	{
		response = (tU32 *)malloc(sizeof(tU32) * length);
	}
	ret = etal_read_parameter(tunerIndex, mode, param, length, response, &responseLength);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "resp:");
		for(i = 0; i < responseLength; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, " %x", response[i]);
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, "\n");
	}
	if (param != NULL)
	{
		free(param);
	}
	if (response != NULL)
	{
		free(response);
	}
	return ret;
}

ETAL_STATUS etaldemo_write_parameter(tVoid)
{
	tChar *ptok;
	tU32 tunerIndex, *paramValueArray;
	tU16 length;
	etalReadWriteModeTy mode;
	ETAL_STATUS ret;

	tunerIndex = 0;
	paramValueArray = NULL;
	mode = fromIndex;
	length = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		tunerIndex = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	while ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		if (paramValueArray == NULL)
		{
			paramValueArray = (tU32 *)malloc(sizeof(tU32) * (length + 1));
		}
		else
		{
			paramValueArray = (tU32 *)realloc(paramValueArray, sizeof(tU32) * (length + 1));
		}
		paramValueArray[length] = atoi(ptok);
		length++;
	}
	ret = etal_write_parameter(tunerIndex, mode, paramValueArray, length);
	if (paramValueArray != NULL)
	{
		free(paramValueArray);
	}
	return ret;
}

ETAL_STATUS etaldemo_start_RDS(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	tBool forceFastPi;
	tU8 numPi;
	EtalRDSRBDSModeTy mode;
	tU8 errthresh;
	tBool groupouten;

	hReceiver_idx = 0;
	forceFastPi = 0;
	numPi = 0;
	mode = ETAL_RDS_MODE;
	errthresh = 0;
	groupouten = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		forceFastPi = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		numPi = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		errthresh = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		groupouten = atoi(ptok);
	}
	return etal_start_RDS(hReceivers[hReceiver_idx], forceFastPi, numPi, mode | ((groupouten != 0) << 1), errthresh);
}

ETAL_STATUS etaldemo_stop_RDS(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etal_stop_RDS(hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaltmldemo_get_textinfo(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalTextInfo Radiotext;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	memset(&Radiotext, 0, sizeof(Radiotext));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etaltml_get_textinfo(hReceivers[hReceiver_idx], &Radiotext);
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalTextInfo(&Radiotext);
	}
	return ret;
}

ETAL_STATUS etaltmldemo_start_textinfo(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etaltml_start_textinfo(hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaltmldemo_stop_textinfo(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etaltml_stop_textinfo(hReceivers[hReceiver_idx]);
}

ETAL_STATUS etaltmldemo_get_decoded_RDS(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalRDSData RDSdata;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	memset(&RDSdata, 0, sizeof(RDSdata));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etaltml_get_decoded_RDS(hReceivers[hReceiver_idx], &RDSdata);
	if (ret == ETAL_RET_SUCCESS)
	{
		etaldemo_print_EtalRDSData(&RDSdata);
	}
	return ret;
}

ETAL_STATUS etaltmldemo_start_decoded_RDS(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, RDSServiceList;

	hReceiver_idx = RDSServiceList = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		RDSServiceList = atoi(ptok);
	}
	return etaltml_start_decoded_RDS(hReceivers[hReceiver_idx], RDSServiceList);
}

ETAL_STATUS etaltmldemo_stop_decoded_RDS(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, RDSServiceList;

	hReceiver_idx = RDSServiceList = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		RDSServiceList = atoi(ptok);
	}
	return etaltml_stop_decoded_RDS(hReceivers[hReceiver_idx], RDSServiceList);
}

ETAL_STATUS etaltmldemo_get_validated_RDS_block_manual(tVoid)
{
#if 0
	tChar *ptok;
	tU32 hReceiver_idx, pBlocks[(STAR_RDS_MAX_BLOCK + 1)], BlocksNum;
	ETAL_STATUS ret;

	hReceiver_idx = BlocksNum = 0;
	memset(pBlocks, 0, sizeof(tU32) * (STAR_RDS_MAX_BLOCK + 1));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etaltml_get_validated_RDS_block_manual(hReceivers[hReceiver_idx], pBlocks, &BlocksNum);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "BlocksNum=%d ", BlocksNum);
		//etaldemo_print_EtalRDSRawData((BlocksNum * 3), pBlocks);
	}
	return ret;
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_AF(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx, hReceiverB_idx;
	tBool AFOn;

	hReceiver_idx = hReceiverB_idx = AFOn = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiverB_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		AFOn = atoi(ptok);
	}
	return etaltml_RDS_AF(hReceivers[hReceiver_idx], hReceivers[hReceiverB_idx], AFOn);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_TA(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx;
	tBool TAOn;

	hReceiver_idx = TAOn = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		TAOn = atoi(ptok);
	}
	return etaltml_RDS_TA(hReceivers[hReceiver_idx], TAOn);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_EON(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx;
	tBool EONOn;

	hReceiver_idx = EONOn = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		EONOn = atoi(ptok);
	}
	return etaltml_RDS_EON(hReceivers[hReceiver_idx], EONOn);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_REG(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx;
	tBool REGOn;

	hReceiver_idx = REGOn = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		REGOn = atoi(ptok);
	}
	return etaltml_RDS_REG(hReceivers[hReceiver_idx], REGOn);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_AFSearch_start(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx, i;
	EtalRDSAFSearchData afSearchData;

	hReceiver_idx = REGOn = 0;
	memset(&afSearchData, 0, sizeof(afSearchData));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		afSearchData.m_PI = atoi(ptok);
	}
	for(i = 0; i < ETAL_DEF_MAX_AFLIST; i++)
	{
		if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
		{
			afSearchData.m_AFList[i] = atoi(ptok);
			afSearchData.m_AFListLen++;
		}
	}
	return etaltml_RDS_AFSearch_start(hReceivers[hReceiver_idx], afSearchData);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_AFSearch_stop(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx;

	hReceiver_idx = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	return etaltml_RDS_AFSearch_stop(hReceivers[hReceiver_idx]);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_RDS_seek_start(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx, step;
	etalSeekDirectionTy direction
	etalSeekAudioTy exitSeekAction;
	EtalRDSSeekTy RDSSeekOption;
	ETAL_STATUS ret;

	hReceiver_idx = step = 0;
	direction = cmdDirectionUp;
	exitSeekAction = cmdAudioUnmuted;
	memset(&RDSSeekOption, 0, sizeof(RDSSeekOption));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		direction = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		step = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		exitSeekAction = atoi(ptok);
	}
	ret = etaltml_RDS_seek_start(hReceivers[hReceiver_idx], direction, step, exitSeekAction, &RDSSeekOption);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "tpSeek=%d taSeek=%d ptySeek=%d ptyCode=%d piSeek=%d pi=%d", RDSSeekOption.tpSeek,
		    RDSSeekOption.taSeek, RDSSeekOption.ptySeek, RDSSeekOption.ptyCode, RDSSeekOption.piSeek, RDSSeekOption.pi);
	}
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_scan_start(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx, audioPlayTime, step;
	etalSeekDirectionTy direction;

	hReceiver_idx = audioPlayTime = step = 0;
	direction = cmdDirectionUp;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		audioPlayTime = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		direction = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		step = atoi(ptok);
	}
	return etaltml_scan_start(hReceivers[hReceiver_idx], audioPlayTime, direction, step);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_scan_stop(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalSeekTerminationModeTy terminationMode;

	hReceiver_idx = 0;
	terminationMode = initialFrequency;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		terminationMode = atoi(ptok);
	}
	return etaltml_scan_stop(hReceivers[hReceiver_idx], terminationMode);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_learn_start(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx, step, nbOfFreq, i;
	EtalFrequencyBand bandIndex;
	etalLearnReportingModeStatusTy mode;
	EtalLearnFrequencyTy *freqList;
	ETAL_STATUS ret;

	hReceiver_idx = step = nbOfFreq = 0;
	mode = normalMode;
	freqList = NULL;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		bandIndex = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		step = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		nbOfFreq = atoi(ptok);
		if (nbOfFreq != 0)
		{
			freqList = (EtalLearnFrequencyTy *)malloc(sizeof(EtalLearnFrequencyTy) * nbOfFreq);
		}
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	ret = etaltml_learn_start(hReceivers[hReceiver_idx], bandIndex, step, nbOfFreq, mode, freqList);
	if (ret == ETAL_RET_SUCCESS)
	{
		for(i = 0; i < nbOfFreq; i++)
		{
			etalDemoPrintf(TR_LEVEL_SYSTEM, "fieldStrength=%d freq=%d HDSvc=%d ChId=%d", freqList[i].m_fieldStrength,
			    freqList[i].m_frequency, freqList[i].m_HDServiceFound, freqList[i].m_ChannelID);
		}
	}
	if (freqList != NULL)
	{
		free(freqList);
	}
	return ret;
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_learn_stop(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalSeekTerminationModeTy terminationMode;

	hReceiver_idx = 0;
	terminationMode = initialFrequency;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		terminationMode = atoi(ptok);
	}
	return etaltml_learn_stop(hReceivers[hReceiver_idx], terminationMode);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_getFreeReceiverForPath(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 hReceiver_idx, i;
	EtalPathName vI_path;
	EtalReceiverAttr O_attr;
	ETAL_STATUS ret;

	hReceiver_idx  = 0;
	vI_path = ETAL_PATH_NAME_UNDEF;
	memset(&O_attr, 0, sizeof(O_attr));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		vI_path = atoi(ptok);
	}
	ret = etaltml_getFreeReceiverForPath(&(hReceivers[hReceiver_idx]), vI_path, &O_attr);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "Standard=%d (%s) FrontEnds=", O_attr.m_Standard, 
		    standard2Ascii(O_attr.m_Standard));
		for(i = 0; i < O_attr.m_FrontEndsSize; i++)
		{
			if (i < ETAL_CAPA_MAX_FRONTEND)
			{
				etalDemoPrintf(TR_LEVEL_SYSTEM, " 0x%x", O_attr.m_FrontEnds[i]);
			}
		}
		etalDemoPrintf(TR_LEVEL_SYSTEM, "\n";
	}
	if (freqList != NULL)
	{
		free(freqList);
	}
	return ret;
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_TuneOnServiceId(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tU32 vI_SearchedServiceID;
	EtalPathName vI_path;

	vI_SearchedServiceID = 0;
	vI_path = ETAL_PATH_NAME_UNDEF;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		vI_path = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		vI_SearchedServiceID = atoi(ptok);
	}
	return etaltml_TuneOnServiceId(vI_path, vI_SearchedServiceID);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_ActivateServiceFollowing(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	return etaltml_ActivateServiceFollowing();
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_DisableServiceFollowing(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	return etaltml_DisableServiceFollowing();
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_ServiceFollowing_SeamlessEstimationRequest(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	return ETALTML_ServiceFollowing_SeamlessEstimationRequest();
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_ServiceFollowing_SeamlessEstimationStop(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	return ETALTML_ServiceFollowing_SeamlessEstimationStop();
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaltmldemo_ServiceFollowing_SeamlessSwitchRequest(tVoid)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)
	tChar *ptok;
	tBool isSeamless;

	isSeamless = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		isSeamless = atoi(ptok);
	}
	return ETALTML_ServiceFollowing_SeamlessSwitchRequest(isSeamless);
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

ETAL_STATUS etaldemo_seamless_estimation_start(tVoid)
{
	tChar *ptok;
	etalSeamlessEstimationConfigTy seamlessEstimationConfig;
	tU32 hRecvIdxFas, hRecvIdxSas;

	hRecvIdxFas = 0;
	hRecvIdxSas = 1;
	seamlessEstimationConfig.mode = 1;
	seamlessEstimationConfig.startPosition = 0;
	seamlessEstimationConfig.stopPosition = -480000;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdxFas = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdxSas = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessEstimationConfig.mode = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessEstimationConfig.startPosition = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessEstimationConfig.stopPosition = atoi(ptok);
	}
	return etal_seamless_estimation_start(hReceivers[hRecvIdxFas], hReceivers[hRecvIdxSas], &seamlessEstimationConfig);
}

ETAL_STATUS etaldemo_seamless_estimation_stop(tVoid)
{
	tChar *ptok;
	tU32 hRecvIdxFas, hRecvIdxSas;

	hRecvIdxFas = 0;
	hRecvIdxSas = 1;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdxFas = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdxSas = atoi(ptok);
	}
	return etal_seamless_estimation_stop(hReceivers[hRecvIdxFas], hReceivers[hRecvIdxSas]);
}

ETAL_STATUS etaldemo_seamless_switching(tVoid)
{
	tChar *ptok;
	etalSeamlessSwitchingConfigTy seamlessSwitchingConfig;
	tU32 hRecvIdxFas, hRecvIdxSas;

	hRecvIdxFas = 0;
	hRecvIdxSas = 1;
	seamlessSwitchingConfig.systemToSwitch = 1;
	if (etaldemo_seamless_estimation_status.m_status == ETALDEMO_SE_STATUS_SUCCESS)
	{
		/* use last successfull seamless estimation result */
		seamlessSwitchingConfig.providerType = etaldemo_seamless_estimation_status.m_providerType;
		seamlessSwitchingConfig.absoluteDelayEstimate = etaldemo_seamless_estimation_status.m_absoluteDelayEstimate;
		seamlessSwitchingConfig.delayEstimate = etaldemo_seamless_estimation_status.m_delayEstimate;
		seamlessSwitchingConfig.timestampFAS = etaldemo_seamless_estimation_status.m_timestamp_FAS;
		seamlessSwitchingConfig.timestampSAS = etaldemo_seamless_estimation_status.m_timestamp_SAS;
		seamlessSwitchingConfig.averageRMS2FAS = etaldemo_seamless_estimation_status.m_RMS2_FAS;
		seamlessSwitchingConfig.averageRMS2SAS = etaldemo_seamless_estimation_status.m_RMS2_SAS;
	}
	else
	{
		/* use default value */
		seamlessSwitchingConfig.providerType = 0;
		seamlessSwitchingConfig.absoluteDelayEstimate = 8888888;
		seamlessSwitchingConfig.delayEstimate = 0;
		seamlessSwitchingConfig.timestampFAS = 0;
		seamlessSwitchingConfig.timestampSAS = 0;
		seamlessSwitchingConfig.averageRMS2FAS = 1;
		seamlessSwitchingConfig.averageRMS2SAS = 1;
	}
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdxFas = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdxSas = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.systemToSwitch = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.providerType = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.absoluteDelayEstimate = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.delayEstimate = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.timestampFAS = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.timestampSAS = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.averageRMS2FAS = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		seamlessSwitchingConfig.averageRMS2SAS = atoi(ptok);
	}
	return etal_seamless_switching(hReceivers[hRecvIdxFas], hReceivers[hRecvIdxSas], &seamlessSwitchingConfig);
}

ETAL_STATUS etaldemo_debug_DISS_control(tVoid)
{
	tChar *ptok;
	tU32 hRecvIdx;
	etalChannelTy tuner_channel;
	EtalDISSMode mode;
	tU8 filter_index;

	hRecvIdx = filter_index = 0;
	tuner_channel = ETAL_CHN_UNDEF;
	mode = ETAL_DISS_MODE_AUTO;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hRecvIdx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		tuner_channel = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		mode = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		filter_index = atoi(ptok);
	}
	return etal_debug_DISS_control(hReceivers[hRecvIdx], tuner_channel, mode, filter_index);
}

ETAL_STATUS etaldemo_debug_get_WSP_Status(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx;
	EtalWSPStatus WSPStatus;
	ETAL_STATUS ret;

	hReceiver_idx = 0;
	memset(&WSPStatus, 0, sizeof(WSPStatus));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	ret = etal_debug_get_WSP_Status(hReceivers[hReceiver_idx], &WSPStatus);
	if (ret == ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "filter idx=%d %d softmute=%d highcut=%d lowcut=%d stereoblend=%d highblend=%d rolloff=%d\n", 
			WSPStatus.m_filter_index[0], WSPStatus.m_filter_index[1], WSPStatus.m_softmute, WSPStatus.m_highcut, 
			WSPStatus.m_lowcut, WSPStatus.m_stereoblend, WSPStatus.m_highblend, WSPStatus.m_rolloff);
	}
	return ret;
}

ETAL_STATUS etaldemo_debug_VPA_control(tVoid)
{
	tChar *ptok;
	tU32 hReceiver_idx, hReceiver_bg_idx;
	tBool status;
	ETAL_STATUS ret;

	hReceiver_idx = hReceiver_bg_idx = 0;
	status = 0;
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_idx = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		status = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		hReceiver_bg_idx = atoi(ptok);
	}
	ret = etal_debug_VPA_control(hReceivers[hReceiver_idx], status, &hReceivers[hReceiver_bg_idx]);
	if ((ret == ETAL_RET_SUCCESS) && (hReceivers[hReceiver_bg_idx] != ETAL_INVALID_HANDLE))
	{
		etalDemoPrintf(TR_LEVEL_SYSTEM, "hReceiver_bg=0x%x\n", hReceivers[hReceiver_bg_idx]);
	}
	return ret;
}

ETAL_STATUS etaldemo_trace_config(tVoid)
{
	tChar *ptok;
	EtalTraceConfig config;
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	int i;
#endif

	memset(&config, 0, sizeof(config));
	/* use received parameters if present */
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		config.m_disableHeader = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		config.m_disableHeaderUsed = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		config.m_defaultLevel = atoi(ptok);
	}
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		config.m_defaultLevelUsed = atoi(ptok);
	}
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	if ((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL)
	{
		config.m_filterNum = atoi(ptok);
	}
	for(i = 0; i < config.m_filterNum; i++)
	{
		if (((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL) && (i < CONFIG_OSUTIL_TRACE_NUM_FILTERS))
		{
			config.m_filterClass[i] = atoi(ptok);
		}
	}
	for(i = 0; i < config.m_filterNum; i++)
	{
		if (((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL) && (i < CONFIG_OSUTIL_TRACE_NUM_FILTERS))
		{
			config.m_filterMask[i] = atoi(ptok);
		}
	}
	for(i = 0; i < config.m_filterNum; i++)
	{
		if (((ptok = strtok(NULL, ETALDEMO_PARAM_DELIMITER)) != NULL) && (i < CONFIG_OSUTIL_TRACE_NUM_FILTERS))
		{
			config.m_filterLevel[i] = atoi(ptok);
		}
	}
#endif
	return etal_trace_config(&config);
}

void etalDemo_textinfoDatPathCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemoPrintf(TR_LEVEL_COMPONENT, "textinfo data received len=%d Ctx=%p ", dwActualBufferSize, pvContext);
	etaldemo_print_EtalTextInfo((EtalTextInfo *)pBuffer);
}

void etalDemo_fmRdsDatPathCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemoPrintf(TR_LEVEL_COMPONENT, "FM RDS data received len=%d Ctx=%p ", dwActualBufferSize, pvContext);
}

void etalDemo_rdsRawDatPathCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemoPrintf(TR_LEVEL_COMPONENT, "FM RDS RAW data received len=%d Ctx=%p ", dwActualBufferSize, pvContext);
}

tBool etaldemo_save_new_hDatapath(ETAL_HANDLE hDatapath)
{
	int i;

	i = 0;
	while ((i < ETALDEMO_NB_DATAPATH_MAX) && (hDatapaths[i] != ETAL_INVALID_HANDLE))
	{
		i++;
	}
	if (i < ETALDEMO_NB_DATAPATH_MAX)
	{
		hDatapaths[i] = hDatapath;
		return 0;
	}
	else
	{
		return 1;
	}
}

#ifdef HAVE_FM
#if 0
/***************************
 *
 * etalDemo_FMRadio
 *
 **************************/
static ETAL_HANDLE etalDemo_FMSeekRadio(tU32 vI_StartFreq, EtalFrequencyBand FMBand)
{
	EtalReceiverAttr attr;
	char line[256];
	ETAL_HANDLE vl_handlefm = ETAL_INVALID_HANDLE;
	etalSeekDirectionTy vl_direction;
    EtalProcessingFeatures processingFeatures;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
	etalDemoModeTy vl_mode = ETALDEMO_MODE_INVALID;
	ETAL_STATUS ret;
	tU32 seekFreq;
	tBool vl_continue = false;

	char XMLstring[512];

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO TUNING ***************\n");

	if (ETAL_BAND_HD == FMBand)
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handlefm, ETAL_PATH_NAME_FM_HD_FG, &attr);
	}
	else
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handlefm, ETAL_PATH_NAME_FM_FG, &attr);
	}
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath %s ERROR (%d)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return vl_handlefm;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											(ETAL_BAND_HD==FMBand)?"HD":"FM",
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handlefm, FMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ERROR", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handlefm;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");
	}
	
	/*
	 * Tune to an FM station
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to %s freq %d\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", vI_StartFreq);

//HD mod: better use async interface to get quicker feedback
// then we should monitor through ETAL_INFO_TUNE
#ifndef HAVE_HD
	ret = etal_tune_receiver(vl_handlefm, vI_StartFreq);
#else
	if (ETAL_BAND_HD == FMBand)
	{
		ret = etal_tune_receiver_async(vl_handlefm, vI_StartFreq);
	}
	else
	{
		ret = etal_tune_receiver(vl_handlefm, vI_StartFreq);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver %s (%d)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return ETAL_INVALID_HANDLE;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", vI_StartFreq);
	}

#ifndef HAVE_HD
	ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM);
#else
	if (ETAL_BAND_HD != FMBand)
	{
		ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM);
	}
	else
	{
		ret = etal_audio_select(vl_handlefm, ETAL_AUDIO_SOURCE_AUTO_HD);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select %s error", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handlefm;
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE DONE ***************\n");

	// start the RDS
	//
	if ((ret = etal_start_RDS(vl_handlefm, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_start_RDS (%d)", ret);
		return vl_handlefm;
	}

	// seek loop
	do
	{
		vl_continue = false;

		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
#ifndef HAVE_HD
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 's' -> to scan up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'r' -> to scan down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'l' -> to start learn procedure\n");
#endif
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'x' -> to toggle RDS on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> to quit seek/scan mode **** \n");

		if (fgets(line, sizeof(line), stdin)) vl_continue = true;

		if (vl_continue)
		{
			switch (line[0])
			{
				case 'u' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 'd' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
#ifndef HAVE_HD
				case 's' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_SCAN;
					break;
				case 'r' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					vl_mode = ETALDEMO_MODE_SCAN;
					break;
				case 'l' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_LEARN;
					break;
#endif
				case 'q' :
					vl_continue = false;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
				case 'x' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					if (RDS_on)
					{
						RDS_on = false;
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS is OFF\n");
						break;
					}
					else
					{
						RDS_on = true;
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS is ON\n");
						break;
					}
				default :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
			}

			switch (vl_mode)
			{
				case ETALDEMO_MODE_SEEK:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SEEKING %s ***************\n",
							((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

					etalDemoSeek_CmdWaitResp(vl_handlefm, vl_direction, ETAL_DEMO_SEEK_FM_WAIT_TIME);

					if ((ret = etal_autoseek_stop(vl_handlefm, lastFrequency)) != ETAL_RET_SUCCESS)
					{
						etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
						return vl_handlefm;
					}

					//Get the new frequency
					etal_get_receiver_frequency(vl_handlefm, &seekFreq);
					if (!IsDemonstrationMode)
					{
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);
					}
					else
					{
					    fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
					    lastFreqUsed = seekFreq;
					    snprintf(XMLstring, sizeof(XMLstring),
							"<info><Band></Band><Frequency>%d</Frequency><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
							lastFreqUsed);
					    write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
					    close(fd_RadioText);
					}

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SEEKING END ***************\n");

					// start RadioText
					etalDemo_startRadioText(vl_handlefm, &vl_hDatapathRds);
					break;
				case ETALDEMO_MODE_SCAN:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SCANNING %s ***************\n",
							((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

					etalDemoScan(vl_handlefm, vl_direction, &vl_hDatapathRds);

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO SCANNING END ***************\n");
					break;
				case ETALDEMO_MODE_LEARN:
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO LEARNING ***************\n");

					etalDemoLearn(vl_handlefm, FMBand);

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM RADIO LEARNING END ***************\n");
					break;
				default:
					break;
			}
		}
	} while((true == vl_continue));

	if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
	{
		hInitialDatapath = vl_hDatapathRds;
	}
		
	return vl_handlefm;
}
#endif


static ETAL_HANDLE etalDemo_TuneFMFgRadio(tU32 vI_freq, EtalFrequencyBand FMBand, tBool select_audio)
{
	EtalReceiverAttr attr_fm;
	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
    EtalProcessingFeatures processingFeatures;

	/*
	 * Create a receiver configuration
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** FM FG RADIO TUNING ***************\n");

#ifdef HAVE_HD
	if (ETAL_BAND_HD == FMBand)
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_HD_FG, &attr_fm);
	}
	else
	{
		ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_FG, &attr_fm);
	}
#else
	ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_FM_FG, &attr_fm);
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMFgRadio / etaltml_getFreeReceiverForPath %s ERROR\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneFMFgRadio / etaltml_getFreeReceiverForPath %s ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											(ETAL_BAND_HD==FMBand)?"HD":"FM",
											attr_fm.m_Standard, 
											attr_fm.m_FrontEndsSize,
											attr_fm.m_FrontEnds[0], attr_fm.m_FrontEnds[1]);
	
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set %s band\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");

	etalDemoPrintf(TR_LEVEL_COMPONENT, "TuneFMFgRadio: %s band (%x)\n", (ETAL_BAND_HD==FMBand)?"HD":"FM", FMBand);

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, FMBand, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ERROR", (ETAL_BAND_HD==FMBand)?"HD":"FM");
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver %s ok\n", (ETAL_BAND_HD==FMBand)?"HD":"FM");
	}
	/*
	 * Tune to an FM station
	 */
	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to FM freq %d\n", vI_freq);

//HD mod: better use async interface to get quicker feedback
// then we should monitor through ETAL_INFO_TUNE
#ifndef HAVE_HD
	ret = etal_tune_receiver(vl_handle, vI_freq);
#else
	if (ETAL_BAND_HD == FMBand)
	{
		//ret = etal_tune_receiver_async(vl_handle, vI_freq);
		ret = etal_tune_receiver_async(vl_handle, vI_freq);
	}
	else
	{
		ret = etal_tune_receiver(vl_handle, vI_freq);
	}
#endif
	if (ETAL_RET_SUCCESS != ret)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver %s (%d)\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM", ret);
		return vl_handle;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver %s ok, freq = %d\n",
			(ETAL_BAND_HD==FMBand)?"HD":"FM", vI_freq);
	}

	if (select_audio)
	{
#ifndef HAVE_HD
		ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);
#else
		if (ETAL_BAND_HD != FMBand)
		{
			ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_STAR_AMFM);
		}
		else
		{
			ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_AUTO_HD);
		}
#endif
		if (ETAL_RET_SUCCESS != ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select %s error", (ETAL_BAND_HD==FMBand)?"HD":"FM");
			return vl_handle;
		}
	}

	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** TUNE FM FG DONE ***************\n");

	// start the RDS
	//
	if ((ret = etal_start_RDS(vl_handle, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_start_RDS (%d)", ret);
		return vl_handle;
	}

	// start RadioText
	etalDemo_startRadioText(vl_handle, &vl_hDatapathRds);

	if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
	{
		hInitialDatapath = vl_hDatapathRds;
		etaldemo_save_new_hDatapath(vl_hDatapathRds);
	}


	return vl_handle;
}
#endif // HAVE FM


#ifdef HAVE_DAB
#if 0
/***************************
 *
 * etalDemo_DABRadioSeek
 *
 **************************/
static ETAL_HANDLE etalDemo_DABRadioSeek(tU32 vI_StartFreq)
{
	EtalReceiverAttr attr;
	ETAL_HANDLE handleDab = ETAL_INVALID_HANDLE;
	ETAL_HANDLE handleDab2 = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;
	etalSeekDirectionTy vl_direction = cmdDirectionUp;
	etalDemoModeTy vl_mode;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
    EtalProcessingFeatures processingFeatures;

	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	unsigned int ueid;
	int i;

	char line[256];
	int Service = 0;
	tU32 seekFreq;
	tBool vl_continue = false;
	tBool vl_manualSeek = false;
	tBool vl_serviceSelectLoop = true;
	tBool vl_2ndSeek = false;

	char XMLstring[512];

	/*
	 * Create a receiver configuration
	 */

	if ((ret = etaltml_getFreeReceiverForPath(&handleDab, ETAL_PATH_NAME_DAB_1, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath DAB ERROR (%d)\n", ret);
		return handleDab;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	/*
	 * Create a 2nd receiver configuration
	  */

	if ((ret = etaltml_getFreeReceiverForPath(&handleDab2, ETAL_PATH_NAME_DAB_2, &attr)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath DAB ERROR (%d)\n", ret);
		return handleDab;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr.m_Standard, 
											attr.m_FrontEndsSize,
											attr.m_FrontEnds[0], attr.m_FrontEnds[1]);
	}

	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(handleDab, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR");
		return handleDab;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	// set band for 2nd receiver
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband for 2nd receiver\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(handleDab2, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB2 ERROR");
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB2  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	if (!IsDemonstrationMode)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_StartFreq);
	}
	else
	{
		fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
		lastFreqUsed = vI_StartFreq;
		snprintf(XMLstring, sizeof(XMLstring),
			"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
			lastFreqUsed, tunedEnsemble);
		write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
		close(fd_RadioText);
	}

	ret = etal_tune_receiver(handleDab, vI_StartFreq);
	if ((ETAL_RET_SUCCESS == ret) || (ret == ETAL_RET_NO_DATA))
	{
		Sleep(USLEEP_ONE_SEC);
		if ((ret = etalDemo_DABServiceListAndSelect(handleDab)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceListAndSelect error" );
			return handleDab;
		}

		tunedEnsemble = true;
		seekFreq = vI_StartFreq;
		
		if (IsDemonstrationMode)
		{
			//Print the new frequency
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
				lastFreqUsed, tunedEnsemble);
			write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
			close(fd_RadioText);
		}
		
		// start RadioText
		etalDemo_startRadioText(handleDab, &vl_hDatapathRds);
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}

	if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
		return handleDab;
	}

	/*
	 * Tune to a DAB frequency ; only to set an initial frequency for 2nd receiver
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune 2nd receiver to DAB freq %d\n", vI_StartFreq);
	etal_tune_receiver(handleDab2, vI_StartFreq);

	do
	{
		vl_continue = false;


		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'u' -> to seek up\n");
		if (tunedEnsemble)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 's' -> to select a service in current ensemble\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'n' -> to select next service in current ensemble\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'p' -> to select previous service in current ensemble\n");
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'd' -> to seek down\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'l' -> to start learn procedure\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'i' -> to manual seek up\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'k' -> to manual seek down\n");
		if (vl_manualSeek)
		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'c' -> to test continue manual seek\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'x' -> to toggle RDS on/off\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t '2' -> launch a seek on second DAB receiver and display service list\n");
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> continue test with Service Following **** \n");
#else
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> proceed to the exit **** \n");
#endif

		if (!IsDemonstrationMode)
		{
		    if (fgets(line, sizeof(line), stdin)) vl_continue = true;
		}
		else
		{
#ifndef CONFIG_HOST_OS_WIN32
		    fd_CtrlFIFO = open(CtrlFIFO, O_RDONLY);
		    Sleep(10000);
		    if (read(fd_CtrlFIFO, line, sizeof(line))) vl_continue = true;
		    close(fd_CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
		}

		if (vl_continue)
		{
			switch (line[0])
			{
				case 'u' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					tunedEnsemble = false;
					Service = 0;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = false;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 'd' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					tunedEnsemble = false;
					Service = 0;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = false;
					vl_mode = ETALDEMO_MODE_SEEK;
					break;
				case 's' :
					if (tunedEnsemble)
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_SERVICE_SELECT;
					}
					else
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
					}
					break;
				case 'n' :
					if (tunedEnsemble)
					{
						vl_continue = true;
						vl_direction = cmdDirectionUp;
						vl_mode = ETALDEMO_MODE_SEEK;
					}
					else
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
					}
					break;
				case 'p' :
					if (tunedEnsemble)
					{
						vl_continue = true;
						vl_direction = cmdDirectionDown;
						vl_mode = ETALDEMO_MODE_SEEK;
					}
					else
					{
						vl_continue = true;
						vl_mode = ETALDEMO_MODE_INVALID;
					}
					break;
				case 'l' :
					vl_continue = true;
					tunedEnsemble = false;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = false;
					vl_mode = ETALDEMO_MODE_LEARN;
					break;
				case 'i' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					tunedEnsemble = false;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = true;
					vl_mode = ETALDEMO_MODE_MANUAL_SEEK;
					break;
				case 'k' :
					vl_continue = true;
					vl_direction = cmdDirectionDown;
					tunedEnsemble = false;
					if (vl_manualSeek)
					{
						etal_seek_stop_manual(handleDab, false, &seekFreq);
					}
					vl_manualSeek = true;
					vl_mode = ETALDEMO_MODE_MANUAL_SEEK;
					break;
				case 'c' :
					vl_continue = true;
					tunedEnsemble = false;
					vl_mode = ETALDEMO_MODE_CONTINUE_MANUAL_SEEK;
					break;
				case 'q' :
					vl_continue = false;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
				case 'x' :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					if (RDS_on)
					{
						RDS_on = false;
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS is OFF\n");
						break;
					}
					else
					{
						RDS_on = true;
						etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t RDS is ON\n");
						break;
					}
				case '2' :
					vl_continue = true;
					vl_direction = cmdDirectionUp;
					vl_mode = ETALDEMO_MODE_INVALID;
					vl_2ndSeek = true;
					break;
				default :
					vl_continue = true;
					vl_mode = ETALDEMO_MODE_INVALID;
					break;
			}

			if (vl_mode == ETALDEMO_MODE_SEEK)
			{
				/* This is the case of 'next service' or 'previous service' command
				 * It's looping around service list within the current ensemble
				 */
				if (tunedEnsemble)
				{
				    if (vl_direction)
				    {
					    //down
					    if (Service == 0)
					    {
					        Service = serv_list.m_serviceCount - 1;
					    }
					    else Service--;
				    }
				    else
				    {
					    //up
					    if (Service == (serv_list.m_serviceCount - 1))
					    {
					        Service = 0;
					    }
					    else Service++;
				    }
				}
				else
				{
				    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

				    etalDemoSeek_CmdWaitResp(handleDab, vl_direction, ETAL_DEMO_SEEK_DAB_WAIT_TIME);

				    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** GET ENSEMBLE DATA ***************\n");

				    etal_get_receiver_frequency(handleDab, &seekFreq);

				    if ((ret = etal_autoseek_stop(handleDab, lastFrequency)) != ETAL_RET_SUCCESS)
				    {
				        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
				    }

				    //Print the new frequency
				    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);
				}

				if (IsDemonstrationMode)
				{
					//Print the new frequency
					fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
					lastFreqUsed = seekFreq;
					snprintf(XMLstring, sizeof(XMLstring),
						"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
						lastFreqUsed, tunedEnsemble);
					write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
					close(fd_RadioText);
				}

				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the current ensemble\n");

				// wait the dab to be ready before tune
				//
				Sleep(USLEEP_ONE_SEC *1);
			}
			if ((vl_mode == ETALDEMO_MODE_SEEK) || (vl_mode == ETALDEMO_MODE_SERVICE_SELECT))
			{
				if ((ret = etal_get_current_ensemble(handleDab, &ueid)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
				}
				else
				{
					// get the new ensemble id
					etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", ueid );

					if (ueid != ETAL_INVALID_UEID)
					{
						if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
							return handleDab;
						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s, frequency is %d\n", label, seekFreq);

						/*
						 * A DAB ensemble contains one or more services: fetch the list
						 * and present it to the user for selection. Some services will be
						 * audio only, others data only. The ETAL interface permits to select
						 * which service list to fetch, here we fetch only the audio services
						 */
						if ((ret = etal_get_service_list(handleDab, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
							return handleDab;
						}

						etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);
						
						/*
						 * etal_get_service_list returns a list of numerical ids, not very useful
						 * to show; so now for every service we read the service label
						 * and present it
						 */
						for (i = 0; i < serv_list.m_serviceCount; i++)
						{
							memset(&svinfo, 0, sizeof(EtalServiceInfo));
							memset(&scinfo, 0, sizeof(EtalServiceComponentList));
							if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
							{
								etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x\n", ret, i, ueid, serv_list.m_service[i] );
							}
							etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);

						}

						if ((!IsDemonstrationMode) && ((vl_mode == ETALDEMO_MODE_SERVICE_SELECT) || (tunedEnsemble == false)))
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

							do
							{
								if (fgets(line, sizeof(line), stdin))
								{
									if (1 == sscanf(line, "%d", &Service))
									{
										if ((Service > serv_list.m_serviceCount)||(Service <= 0))
										{
											etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
											etalDemoPrintf(TR_LEVEL_COMPONENT, "Please enter your selection:\n");
											vl_serviceSelectLoop = true;
										}
										else
										{
											/* i can be safely used */
											Service--;
											memset(&svinfo, 0, sizeof(EtalServiceInfo));
											memset(&scinfo, 0, sizeof(EtalServiceComponentList));
											if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
											{
										        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, ueid, serv_list.m_service[i] );
											}
											vl_serviceSelectLoop = false;
										}
									}
								}
							}while(vl_serviceSelectLoop);

							etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n",
												Service+1,
												serv_list.m_service[Service], svinfo.m_serviceLabel);
						}
						if ((ret = etal_service_select_audio(handleDab, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
						{
							etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
					        return handleDab;
						}
						else
						{
							tunedEnsemble = true;

							if (IsDemonstrationMode)
							{
								//Print the new frequency
								fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
								snprintf(XMLstring, sizeof(XMLstring),
									"<info><Band>DAB</Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
									lastFreqUsed, tunedEnsemble);
								write(fd_RadioText,XMLstring, strlen(XMLstring)+1);
								close(fd_RadioText);
							}

							// start RadioText
							etalDemo_startRadioText(handleDab, &vl_hDatapathRds);
						}
					}
				}
				
				if (vl_mode == ETALDEMO_MODE_SEEK) etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO SEEKING END ***************\n");

				if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
					return handleDab;
				}
			}
			else if (vl_mode == ETALDEMO_MODE_LEARN)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO LEARNING ***************\n");

				etalDemoLearn(handleDab, ETAL_BAND_DAB3);

				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO LEARNING END ***************\n");
			}
			else if (vl_mode == ETALDEMO_MODE_MANUAL_SEEK)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

				if ((ret = etal_seek_start_manual(handleDab, vl_direction, 0, &seekFreq)) != ETAL_RET_NO_DATA)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);

					tunedEnsemble = true;

					// wait the dab to be ready before tune
					Sleep(USLEEP_ONE_SEC *1);

				    if ((ret = etalDemo_DABServiceListAndSelect(handleDab)) != ETAL_RET_SUCCESS)
				    {
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceListAndSelect error" );
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
					    return handleDab;
				    }

				    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");

				    if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
				    {
					   etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
					   return handleDab;
				    }

				    // start RadioText
				    etalDemo_startRadioText(handleDab, &vl_hDatapathRds);
				}
				else
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);
					etalDemoPrintf(TR_LEVEL_COMPONENT, "No DAB service found !\n");
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
				}
			}
			else if (vl_mode == ETALDEMO_MODE_CONTINUE_MANUAL_SEEK)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

				if ((ret = etal_seek_continue_manual(handleDab, &seekFreq)) == ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);

					tunedEnsemble = true;

					// wait the dab to be ready before tune
					//
					Sleep(USLEEP_ONE_SEC *1);

				    if ((ret = etalDemo_DABServiceListAndSelect(handleDab)) != ETAL_RET_SUCCESS)
				    {
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceListAndSelect error" );
					    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
					    return handleDab;
				    }

				    if ((ret = etal_audio_select(handleDab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
				    {
					   etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error" );
					   etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
					   return handleDab;
				    }

					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");

				    // start RadioText
				    etalDemo_startRadioText(handleDab, &vl_hDatapathRds);
				}
				else if (ETAL_RET_NO_DATA == ret)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "frequency %d\n", seekFreq);
					etalDemoPrintf(TR_LEVEL_COMPONENT, "No DAB service found !\n");
					etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB RADIO MANUAL SEEKING END ***************\n");
				}
			}
			if (true == vl_2ndSeek)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB2 RADIO SEEKING %s ***************\n",
						((cmdDirectionUp == vl_direction)?"UP":"DOWN"));

			    etalDemoSeek_CmdWaitResp(handleDab2, vl_direction, ETAL_DEMO_SEEK_DAB_WAIT_TIME);

				etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** GET ENSEMBLE DATA ***************\n");

				etal_get_receiver_frequency(handleDab2, &seekFreq);

			    if ((ret = etal_autoseek_stop(handleDab2, lastFrequency)) != ETAL_RET_SUCCESS)
			    {
			        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_seek (%d)", ret);
			    }

			    //Print the new frequency
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", seekFreq);

				// get the new ensemble id
				etalDemoPrintf(TR_LEVEL_COMPONENT, "Reading the current ensemble\n");

				// wait the dab to be ready before tune
				//
				Sleep(USLEEP_ONE_SEC *1);

			    if ((ret = etalDemo_DABServiceList(handleDab2)) != ETAL_RET_SUCCESS)
			    {
				    etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_DABServiceList error" );
			    }
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB2 RADIO SEEKING END ***************\n");
				
				vl_2ndSeek = false;
			}
		}
	} while((true == vl_continue));

	if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
	{
		hInitialDatapath = vl_hDatapathRds;
	}

	if (ETAL_INVALID_HANDLE != handleDab2)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy secondary receiver\n");
		if ((ret = etal_destroy_receiver(&handleDab2)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}

	return handleDab;
}
#endif


/***************************
 *
 * etalDemo_TuneDAB1Radio
 *
 **************************/
static ETAL_HANDLE etalDemo_TuneDAB1Radio(tU32 vI_freq, tBool select_audio)
{
	EtalReceiverAttr attr_dab_1;

	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;

	ETAL_HANDLE vl_hDatapathRds = ETAL_INVALID_HANDLE;
    EtalProcessingFeatures processingFeatures;

	ETAL_STATUS ret;

	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	tU32 ueid;
	int i;

	char line[256];
	int Service;
	tBool vl_selectedSidPresent = false;
	tBool vl_continue = false;

	/*
	 * Create a receiver configuration
	 */

		
	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB1 RADIO TUNING ***************\n");
		
	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_DAB_1, &attr_dab_1)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB1Radio / etaltml_getFreeReceiverForPath DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB1Radio / etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr_dab_1.m_Standard, 
											attr_dab_1.m_FrontEndsSize,
											attr_dab_1.m_FrontEnds[0], attr_dab_1.m_FrontEnds[1]);
	
	}


	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_freq);
	ret = etal_tune_receiver(vl_handle, vI_freq);

	if ((ETAL_RET_SUCCESS == ret) || (ETAL_RET_NO_DATA == ret))
	{
		if (ETAL_RET_SUCCESS == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_freq);
		}
		else if (ETAL_RET_NO_DATA == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB no data, freq = %d\n", vI_freq);
		}

		/*
		 * Unlike FM, for DAB we need to allow the tuner some time
		 * to capture and decode the ensemble information
		 * Without this information the tuner is unable to perform
		 * any operation on the ensemble except quality measure
		 */
		Sleep(USLEEP_ONE_SEC);

		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, first pass\n");

		if ((ret = etal_get_current_ensemble(vl_handle, &ueid)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)\n", ret);
            goto exit;
		}

		/*
		 * Now that we know the UEId let's get the ensemble label; this is not required
		 * but nicer to show rather than the ensemble frequency
		 */
		if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)\n", ret);
			goto exit;
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

		/*
		 * A DAB ensemble contains one or more services: fetch the list
		 * and present it to the user for selection. Some services will be
		 * audio only, others data only. The ETAL interface permits to select
		 * which service list to fetch, here we fetch only the audio services
		 */
		if ((ret = etal_get_service_list(vl_handle, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)\n", ret);
			goto exit;
		}
		/*
		 * etal_get_service_list returns a list of numerical ids, not very useful
		 * to show; so now for every service we read the service label
		 * and present it
		 */
		for (i = 0; i < serv_list.m_serviceCount; i++)
		{
			memset(&svinfo, 0, sizeof(EtalServiceInfo));
			memset(&scinfo, 0, sizeof(EtalServiceComponentList));
			if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d)\n", ret);
			}
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
		}

		if (false == vl_selectedSidPresent)
		{
			do
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

				if (fgets(line, sizeof(line), stdin))
				{
					if (1 == sscanf(line, "%d", &Service))
					{
						if ((Service > serv_list.m_serviceCount)||(Service <= 0))
						{
							etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
							vl_continue=true;
						}
						else
						{
							/* i can be safely used */
							Service--;
							memset(&svinfo, 0, sizeof(EtalServiceInfo));
							memset(&scinfo, 0, sizeof(EtalServiceComponentList));
							if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
							{
						        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x\n", ret, i, ueid, serv_list.m_service[i] );
							}
							vl_continue=false;
						}
					}
				}
			}while (true == vl_continue);
		}

		/*
		 * Now the user could select the service
		 * We do it for him
		 */

		/*
		 * This is the DAB equivalent of tuning to an FM station.
		 * Audio may be available after this step, depending on the CMOST CUT:
		 * - CUT 1.0 no audio
		 * - CUT 2.x audio
		 * See above for hints on how to find out the CUT version.
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n", 
						Service+1, 
						serv_list.m_service[Service], svinfo.m_serviceLabel); 
	
		if ((ret = etal_service_select_audio(vl_handle, ETAL_SERVSEL_MODE_SERVICE, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)\n", ret);
			goto exit;
		}

		if (select_audio)
		{
			if ((ret = etal_audio_select(vl_handle, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_audio_select DAB error\n" );
				goto exit;
			}

			// start RadioText
			etalDemo_startRadioText(vl_handle, &vl_hDatapathRds);

			if (ETAL_INVALID_HANDLE != vl_hDatapathRds)
			{
				hInitialDatapath = vl_hDatapathRds;
				etaldemo_save_new_hDatapath(vl_hDatapathRds);
			}
		}
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}
exit:
			return vl_handle;
		}

/***************************
 *
 * etalDemo_TuneDAB2Radio
 *
 **************************/
static ETAL_HANDLE etalDemo_TuneDAB2Radio(tU32 vI_freq)
{
	EtalReceiverAttr attr_dab_2;

	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;

	ETAL_HANDLE vl_hDatapathAudioDataChannel = ETAL_INVALID_HANDLE;
	EtalDataPathAttr dataPathAttr;

    EtalProcessingFeatures processingFeatures;

	ETAL_STATUS ret;

	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size
	tU8 charset;
	char label[17];
	tU16 bitmap;
	tU32 ueid;
	int i;
	
	char line[256];
	int Service;
	tBool vl_selectedSidPresent = false;
	tBool vl_continue = false;

	if (0 == vI_freq)
		{
		goto exit;
		}
	
	/*
	 * Create a receiver configuration
	 */


	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** DAB2 RADIO TUNING ***************\n");

	if ((ret = etaltml_getFreeReceiverForPath(&vl_handle, ETAL_PATH_NAME_DAB_2, &attr_dab_2)) != ETAL_RET_SUCCESS)
		{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB2Radio / etaltml_getFreeReceiverForPath DAB ERROR\n");
		goto exit;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_TuneDAB2Radio / etaltml_getFreeReceiverForPath dab ok  : standard %d,  m_FrontEndsSize %d, FRONT END LIST : 1 => %d, 2 => %d\n", 
											attr_dab_2.m_Standard,
											attr_dab_2.m_FrontEndsSize,
											attr_dab_2.m_FrontEnds[0], attr_dab_2.m_FrontEnds[1]);
	
	}


	// set band
	etalDemoPrintf(TR_LEVEL_COMPONENT, "set DABband\n");

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(vl_handle, ETAL_BAND_DAB3, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB ERROR\n");
		goto exit;
		}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_change_band_receiver DAB  ok\n");
	}

	/*
	 * Tune to a DAB ensemble
	 */

	etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune to DAB freq %d\n", vI_freq);
	ret = etal_tune_receiver(vl_handle, vI_freq);

	if ((ETAL_RET_SUCCESS == ret) || (ETAL_RET_NO_DATA == ret))
	{
		if (ETAL_RET_SUCCESS == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB ok, freq = %d\n", vI_freq);
		}
		else if (ETAL_RET_NO_DATA == ret)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_tune_receiver DAB no data, freq = %d\n", vI_freq);
		}

		/*
		 * Unlike FM, for DAB we need to allow the tuner some time
		 * to capture and decode the ensemble information
		 * Without this information the tuner is unable to perform
		 * any operation on the ensemble except quality measure
		 */
		Sleep(USLEEP_ONE_SEC);

		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, first pass\n");

		if ((ret = etal_get_current_ensemble(vl_handle, &ueid)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)\n", ret);
            goto exit;
		}

		/*
		 * Now that we know the UEId let's get the ensemble label; this is not required
		 * but nicer to show rather than the ensemble frequency
		 */
		if ((ret = etal_get_ensemble_data(ueid, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)\n", ret);
            goto exit;
		}
		etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

		/*
		 * A DAB ensemble contains one or more services: fetch the list
		 * and present it to the user for selection. Some services will be
		 * audio only, others data only. The ETAL interface permits to select
		 * which service list to fetch, here we fetch only the audio services
		 */
		if ((ret = etal_get_service_list(vl_handle, ueid, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)\n", ret);
			goto exit;
		}
		/*
		 * etal_get_service_list returns a list of numerical ids, not very useful
		 * to show; so now for every service we read the service label
		 * and present it
		 */
		for (i = 0; i < serv_list.m_serviceCount; i++)
		{
			memset(&svinfo, 0, sizeof(EtalServiceInfo));
			memset(&scinfo, 0, sizeof(EtalServiceComponentList));
			if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d)\n", ret);
			}
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
		}

		if (false == vl_selectedSidPresent)
		{
			do
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

				if (fgets(line, sizeof(line), stdin))
				{
					if (1 == sscanf(line, "%d", &Service))
					{
						if ((Service > serv_list.m_serviceCount)||(Service <= 0))
						{
							etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
							vl_continue=true;
	}
	else
	{
							/* i can be safely used */
							Service--;
							memset(&svinfo, 0, sizeof(EtalServiceInfo));
							memset(&scinfo, 0, sizeof(EtalServiceComponentList));
							if ((ret = etal_get_specific_service_data_DAB(ueid, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
							{
						        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x\n", ret, i, ueid, serv_list.m_service[i] );
							}
							vl_continue=false;
						}
					}
				}
			}while (true == vl_continue);
		}

		/*
		 * Now the user could select the service
		 * We do it for him
		 */

		/*
		 * This is the DAB equivalent of tuning to an FM station.
		 * Audio may be available after this step, depending on the CMOST CUT:
		 * - CUT 1.0 no audio
		 * - CUT 2.x audio
		 * See above for hints on how to find out the CUT version.
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n",
						Service+1,
						serv_list.m_service[Service], svinfo.m_serviceLabel);

		/*
		 * Create a datapath for RAW audio
		 */

		memset(&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
		dataPathAttr.m_receiverHandle = vl_handle;
		dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_AUDIO_RAW;
		dataPathAttr.m_sink.m_context = (void *)0;
		dataPathAttr.m_sink.m_BufferSize = 0; // unused
		dataPathAttr.m_sink.m_CbProcessBlock = etalDemo_DAB2DataCallback;
		if ((ret = etal_config_datapath(&vl_hDatapathAudioDataChannel, &dataPathAttr)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_config_datapath for DAB2 (%d)\n", ret);
			goto exit;
		}
		etaldemo_save_new_hDatapath(vl_hDatapathAudioDataChannel);

		/*
		 * Select a data service
		 */
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a DATA service from the ensemble\n");
		if ((ret = etal_service_select_data(vl_hDatapathAudioDataChannel, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ueid, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
			goto exit;
		}

	}
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_tune_receiver DAB (%d)\n", ret);
	}
exit:
	return vl_handle;
}


/***************************
 *
 * etalDemo_DAB2DataCallback
 *
 **************************/

void etalDemo_DAB2DataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemoPrintf(TR_LEVEL_COMPONENT, "DAB2 data received\n");
}

#if 0
static ETAL_STATUS etalDemo_DABServiceList(ETAL_HANDLE handleDab)
{
	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size

	tU32 uEId;

	tU8 charset;
	char label[17];
	tU16 bitmap;

	int i;

	ETAL_STATUS ret;

    if ((ret = etal_get_current_ensemble(handleDab, &uEId)) != ETAL_RET_SUCCESS)
	{
        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
    }
    else
	{
	    // get the new ensemble id
		etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", uEId);

		if (uEId != ETAL_INVALID_UEID)
		{
		    if ((ret = etal_get_ensemble_data(uEId, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
			{
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
				return 1;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

			/*
			 * A DAB ensemble contains one or more services: fetch the list
			 * and present it to the user for selection. Some services will be
			 * audio only, others data only. The ETAL interface permits to select
			 * which service list to fetch, here we fetch only the audio services
			 */
			if ((ret = etal_get_service_list(handleDab, uEId, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
				return ETAL_RET_ERROR;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);

			/*
			 * etal_get_service_list returns a list of numerical ids, not very useful
			 * to show; so now for every service we read the service label
			 * and present it
			 */
			for (i = 0; i < serv_list.m_serviceCount; i++)
			{
				memset(&svinfo, 0, sizeof(EtalServiceInfo));
				memset(&scinfo, 0, sizeof(EtalServiceComponentList));
				if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
				}
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
			}

		}
	}
	return ret;
}

static ETAL_STATUS etalDemo_DABServiceListAndSelect(ETAL_HANDLE handleDab)
{
	EtalServiceList serv_list;
	EtalServiceInfo svinfo;
	static EtalServiceComponentList scinfo; // static due to size

	tU32 uEId;

	tU8 charset;
	char label[17];
	tU16 bitmap;

	char line[256];
	int Service;
	int i;

	tBool vl_serviceSelectLoop = true;

	ETAL_STATUS ret;

    if ((ret = etal_get_current_ensemble(handleDab, &uEId)) != ETAL_RET_SUCCESS)
	{
        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_current_ensemble (%d)", ret);
    }
    else
	{
	    // get the new ensemble id
		etalDemoPrintf(TR_LEVEL_COMPONENT, "current ensemble 0x%x\n", uEId);

		if (uEId != ETAL_INVALID_UEID)
		{
		    if ((ret = etal_get_ensemble_data(uEId, &charset, label, &bitmap)) != ETAL_RET_SUCCESS)
			{
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_ensemble_data (%d)", ret);
				return 1;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "The ensemble label is: %s\n", label);

			/*
			 * A DAB ensemble contains one or more services: fetch the list
			 * and present it to the user for selection. Some services will be
			 * audio only, others data only. The ETAL interface permits to select
			 * which service list to fetch, here we fetch only the audio services
			 */
			if ((ret = etal_get_service_list(handleDab, uEId, 1, 0, &serv_list)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_get_service_list (%d)", ret);
				return ETAL_RET_ERROR;
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "found: %d services \n", serv_list.m_serviceCount);

			/*
			 * etal_get_service_list returns a list of numerical ids, not very useful
			 * to show; so now for every service we read the service label
			 * and present it
			 */
			for (i = 0; i < serv_list.m_serviceCount; i++)
			{
				memset(&svinfo, 0, sizeof(EtalServiceInfo));
				memset(&scinfo, 0, sizeof(EtalServiceComponentList));
				if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[i], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
				{
					etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
				}
			    etalDemoPrintf(TR_LEVEL_COMPONENT, "\tSid %d: 0x%x (%s)\n", (i+1), serv_list.m_service[i], svinfo.m_serviceLabel);
			}

			if (!IsDemonstrationMode)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "*** choose the service to select in the list : --> ");

				do
				{
					if (fgets(line, sizeof(line), stdin))
					{
						if (1 == sscanf(line, "%d", &Service))
						{
							if ((Service > serv_list.m_serviceCount)||(Service <= 0))
							{
								etalDemoPrintf(TR_LEVEL_ERRORS, "Wrong service selection !\n");
								etalDemoPrintf(TR_LEVEL_COMPONENT, "Please enter your selection:\n");
								vl_serviceSelectLoop = true;
							}
							else
							{
								/* i can be safely used */
								Service--;
								memset(&svinfo, 0, sizeof(EtalServiceInfo));
								memset(&scinfo, 0, sizeof(EtalServiceComponentList));
								if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
								{
						    	    etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
								}
								vl_serviceSelectLoop = false;
							}
						}
					}
				}while(vl_serviceSelectLoop);
			}
			else
			{
				Service = 0;
				memset(&svinfo, 0, sizeof(EtalServiceInfo));
				memset(&scinfo, 0, sizeof(EtalServiceComponentList));
				if ((ret = etal_get_specific_service_data_DAB(uEId, serv_list.m_service[Service], &svinfo, &scinfo, NULL)) != ETAL_RET_SUCCESS)
				{
			        etalDemoPrintf(TR_LEVEL_COMPONENT, "WARNING: etal_get_specific_service_data_DAB (%d) : index %d ueid 0x%x service 0x%x", ret, i, uEId, serv_list.m_service[i] );
				}
			}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "Selecting a service from the ensemble, service number = %d, service ID = 0x%x, Service Name = %s\n",
							Service+1, serv_list.m_service[Service], svinfo.m_serviceLabel);
			if ((ret = etal_service_select_audio(handleDab, ETAL_SERVSEL_MODE_SERVICE, uEId, serv_list.m_service[Service], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_service_select (%d)", ret);
				return ret;
			}
		}
	}
	return ret;
}
#endif
#endif // HAVE_DAB

void etalDemo_defaultDataCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemoPrintf(TR_LEVEL_COMPONENT, "data received len:%u context:%p\n", dwActualBufferSize, pvContext);
}

static tVoid etalDemoEndTest()
{
    ETAL_STATUS ret;

	/*
	  * Destroy all receivers
	  */

	if (ETAL_INVALID_HANDLE != hCurrentReceiver)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy main receiver\n");
		if ((ret = etal_destroy_receiver(&hCurrentReceiver)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}

	if (ETAL_INVALID_HANDLE != hSecondReceiver)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Destroy secondary receiver\n");
		if ((ret = etal_destroy_receiver(&hSecondReceiver)) != ETAL_RET_SUCCESS)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_destroy_receiver(%d)\n", ret);
		}
	}
	return;
}

/***************************
 *
 * etalDemoSeek_CmdWaitResp sends seek command and wait response
 *
 **************************/
#if 0
tSInt etalDemoSeek_CmdWaitResp(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, OSAL_tMSecond response_timeout)
{
    ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
    OSAL_tEventMask vl_resEvent;
	
	  etalDemoPrintf(TR_LEVEL_COMPONENT, "***************  RADIO SEEKING ***************\n");
	  
	  	
	  if ((ret = etal_autoseek_start(receiver, vI_direction, ETAL_DEMO_FREQ_STEP, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
	  {
		  etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR:etalDemoSeek_CmdWaitResp  etal_service_seek (%d)", ret);
		  return 1;
	  }

	etalDemo_ClearEvent(ETALDEMO_EVENT_TUNED_RECEIVED);

	// Wait here 
	 vl_res = OSAL_s32EventWaitForDemo (etalDemo_EventHandler,
							  ETALDEMO_EVENT_WAIT_MASK, 
							  OSAL_EN_EVENTMASK_OR, 
							  response_timeout,
							  &vl_resEvent);
	
	etalDemo_ClearEventFlag(vl_resEvent);

	if (OSAL_ERROR == vl_res) 
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response error");
		}	
	
	if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res) 
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response timeout");
		}	
	else if (vl_resEvent == ETALDEMO_EVENT_TUNED_RECEIVED_FLAG)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response event received \n");
		}
	else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_seek response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	
    return vl_res;
}
#endif

/***************************
 *
 * etalDemoScan
 *
 **************************/
#if 0
static tSInt etalDemoScan(ETAL_HANDLE receiver, etalSeekDirectionTy vI_direction, ETAL_HANDLE *vl_hDatapathRds)
{
	ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
	OSAL_tEventMask vl_resEvent;
	OSAL_tThreadID userInterruptThreadID;
	OSAL_trThreadAttribute userInterruptThread_attr;
	etalDemoThreadAttrTy threadAttr;

	tU32 scanFreq;

	if ((ret = etaltml_scan_start(receiver, ETAL_DEMO_SCAN_PLAY_TIME, vI_direction, ETAL_DEMO_FREQ_STEP)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etalDemoScan etaltml_scan_start (%d)", ret);
		return 1;
	}

// abort scan on user key press
// Need one thread alive for keyboard scanning, then calling etaltml_scan_stop

	threadAttr.hReceiver = receiver;
	threadAttr.mode = ETALDEMO_MODE_SCAN;

	userInterruptThread_attr.szName = "USER_INT_THREAD";
	userInterruptThread_attr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	userInterruptThread_attr.s32StackSize = 4096;
	userInterruptThread_attr.pfEntry = (tPVoid)etalDemo_UserInt_ThreadEntry;
	userInterruptThread_attr.pvArg = (tPVoid)&threadAttr;

	vl_res = OSAL_ThreadSpawn(&userInterruptThread_attr);
	if (OSAL_ERROR != vl_res)
	{
		userInterruptThreadID = vl_res;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Can't spawn user interrupt thread");
		return vl_res;
	}

// check for ETALDEMO_EVENT_SCAN_FINISHED_FLAG before exiting the loop

	do
	{

		// Wait here
		vl_res = OSAL_s32EventWait (etalDemo_EventHandler,
							  ETALDEMO_EVENT_SCAN_WAIT_MASK,
							  OSAL_EN_EVENTMASK_OR,
							  ETAL_DEMO_SEEK_FM_WAIT_TIME,
							  &vl_resEvent);

		etalDemo_ClearEventFlag(vl_resEvent);

		if (OSAL_ERROR == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan response error");
		}
		if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan response timeout");
		}
		else if (vl_resEvent & ETALDEMO_EVENT_SCAN_FOUND_FLAG)
		{
			// display Frequency + RadioText when station is found

			etal_get_receiver_frequency(receiver, &scanFreq);

			etalDemoPrintf(TR_LEVEL_COMPONENT, "Frequency %d\n", scanFreq);

			etalDemo_startRadioText(receiver, vl_hDatapathRds);
		}
		else if (vl_resEvent & ETALDEMO_EVENT_SCAN_FINISHED_FLAG)
		{
			OSAL_s32ThreadDelete(userInterruptThreadID);
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan finished event received \n");
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_scan response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	}while ((vl_resEvent & ETALDEMO_EVENT_SCAN_FINISHED_FLAG) == 0);

    return vl_res;
}
#endif

/***************************
 *
 * etalDemoLearn
 *
 **************************/
#if 0
static tSInt etalDemoLearn(ETAL_HANDLE receiver, EtalFrequencyBand vI_band)
{
	ETAL_STATUS ret;
	tSInt vl_res = OSAL_OK;
	OSAL_tEventMask vl_resEvent;
	OSAL_tThreadID userInterruptThreadID;
	OSAL_trThreadAttribute userInterruptThread_attr;
	etalDemoThreadAttrTy threadAttr;
	int i;

	etaldemo_learn_freq_nb = 0;

	if ((ret = etaltml_learn_start(receiver, vI_band, ETAL_DEMO_FREQ_STEP, ETAL_LEARN_MAX_NB_FREQ, normalMode, etaldemo_learn_list)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etalDemoLearn etaltml_learn_start (%d)", ret);
		return 1;
	}

// abort learn on user key press
// Need one thread alive for keyboard scanning, then calling etaltml_learn_stop

	threadAttr.hReceiver = receiver;
	threadAttr.mode = ETALDEMO_MODE_LEARN;

	userInterruptThread_attr.szName = "USER_INT_THREAD";
	userInterruptThread_attr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	userInterruptThread_attr.s32StackSize = 4096;
	userInterruptThread_attr.pfEntry = (tPVoid)etalDemo_UserInt_ThreadEntry;
	userInterruptThread_attr.pvArg = (tPVoid)&threadAttr;

	vl_res = OSAL_ThreadSpawn(&userInterruptThread_attr);
	if (OSAL_ERROR != vl_res)
	{
		userInterruptThreadID = vl_res;
	}
	else
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "Can't spawn user interrupt thread");
		return vl_res;
	}

// check for ETALDEMO_EVENT_LEARN_FINISHED_FLAG before exiting the loop

	do
	{

		// Wait here
		vl_res = OSAL_s32EventWait (etalDemo_EventHandler,
							  ETALDEMO_EVENT_LEARN_WAIT_MASK,
							  OSAL_EN_EVENTMASK_OR,
							  ETAL_DEMO_SEEK_FM_WAIT_TIME,
							  &vl_resEvent);

		etalDemo_ClearEventFlag(vl_resEvent);

		if (OSAL_ERROR == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn response error");
		}
		if (OSAL_ERROR_TIMEOUT_EXPIRED == vl_res)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn response timeout");
		}
		else if (vl_resEvent & ETALDEMO_EVENT_LEARN_FINISHED_FLAG)
		{
			OSAL_s32ThreadDelete(userInterruptThreadID);
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn finished event received \n");
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etaltml_learn response event received 0x%x, vl_res = %d\n", vl_resEvent, vl_res);
		}
	}while ((vl_resEvent & ETALDEMO_EVENT_LEARN_FINISHED_FLAG) == 0);

	if (etaldemo_learn_freq_nb)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Learned %d frequencies\n", etaldemo_learn_freq_nb);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Freq List\n");
		for (i=0; i<etaldemo_learn_freq_nb; i++)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "%d: %dkHz\t", i+1, etaldemo_learn_list[i].m_frequency);
			if (((i+1)%5==0) && (i>0))
			{
				etalDemoPrintf(TR_LEVEL_COMPONENT, "\n");
			}
		}
	}
	etalDemoPrintf(TR_LEVEL_COMPONENT, "\n");

    return vl_res;
}

/***************************
 *
 * etalDemo_UserInt_ThreadEntry
 *
 **************************/
static tVoid etalDemo_UserInt_ThreadEntry(tPVoid param)
{
	etalDemoThreadAttrTy *ThreadAttr;
	etalDemoModeTy Mode;
	ETAL_HANDLE InScanReceiver;
	char line[256];

	ThreadAttr = (etalDemoThreadAttrTy*) param;
	InScanReceiver = ThreadAttr->hReceiver;
	Mode = ThreadAttr->mode;

	if (Mode == ETALDEMO_MODE_SCAN)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** HIT ANY KEY TO STOP SCANNING ***************\n");

		fgets(line, sizeof(line), stdin);

		etaltml_scan_stop(InScanReceiver, lastFrequency);
	}
	else if (Mode == ETALDEMO_MODE_LEARN)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** HIT ANY KEY TO STOP LEARNING ***************\n");

		fgets(line, sizeof(line), stdin);

		etaltml_learn_stop(InScanReceiver, initialFrequency);
	}

	OSAL_vThreadExit();
}
#endif

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
// etalDemo_SFEventHandling
static tVoid etalDemo_SFEventHandling(EtalTuneServiceIdStatus *pI_SFInformation)
{

	// Print information
	static ETAL_HANDLE hDatapathData = ETAL_INVALID_HANDLE;

	if (false == pI_SFInformation->m_IsFoundstatus)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : Service Not Found \n");
		return;
	}
	else
	{
		if ((tU32)0x00FFFFFF != pI_SFInformation->m_Ueid)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : DAB SELECTED\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t Frequency %d, Ueid = 0x%x, Sid = 0x%x\n",
					pI_SFInformation->m_freq,
					pI_SFInformation->m_Ueid,
					pI_SFInformation->m_SidPi);
		}
		else
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_SFEventHandling : FM SELECTED\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\t\t Frequency %d, Sid = 0x%x\n",
					pI_SFInformation->m_freq,
					pI_SFInformation->m_SidPi);
		}

		if (ETAL_INVALID_HANDLE != hInitialDatapath)
		{
			etal_destroy_datapath(&hInitialDatapath);
			hInitialDatapath = ETAL_INVALID_HANDLE;
		}
		etalDemo_startRadioText(pI_SFInformation->m_receiverHandle, &hDatapathData);
	}
}
#endif

/***************************
 *
 * etalDemo_userNotificationHandler
 *
 **************************/
static void etalDemo_userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pstatus)
{
	char XMLstring[512];
	EtalStereoStatus *stereo_status;

	if (dwEvent == ETAL_INFO_TUNE)
	{
		EtalTuneStatus *status = (EtalTuneStatus *)pstatus;
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Tune info event, Frequency %d, good station found %d m_sync:0x%x\n", 
						status->m_stopFrequency, 
						(status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND, status->m_sync);
		
//HD mod: consider bits for HD
// ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED : 1 = HD content is present
// ETAL_TUNESTATUS_SYNCMASK_COMPLETE : 1 = HD audio acquired
#ifdef HAVE_HD
		etaldemo_HD_status = 0;

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_SYNC)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD signal found\n");
			etaldemo_HD_status = 1;
		}

		if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "HD digital audio acquired\n");
			etaldemo_HD_status = 2;
		}

		if (IsDemonstrationMode)
		{
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			lastFreqUsed = status->m_stopFrequency;
			if (etaldemo_HD_status == 0)
			{
				snprintf(XMLstring, sizeof(XMLstring),
					"<info><Band></Band><Frequency>%d</Frequency><HDStatus></HDStatus><HDServiceId></HDServiceId><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
					lastFreqUsed);
			}
			if (etaldemo_HD_status == 1)
			{
				snprintf(XMLstring, sizeof(XMLstring),
					"<info><Band></Band><Frequency>%d</Frequency><HDStatus>HD Sync</HDStatus><HDServiceId></HDServiceId><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
					lastFreqUsed);
			}
			if (etaldemo_HD_status == 2)
			{
				snprintf(XMLstring, sizeof(XMLstring),
					"<info><Band></Band><Frequency>%d</Frequency><HDStatus>HD Audio</HDStatus><HDServiceId>Service ID: %d</HDServiceId><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
					lastFreqUsed, status->m_serviceId);
			}
			write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
			close(fd_RadioText);
		}
#endif
		if (((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_FOUND) == ETAL_TUNESTATUS_SYNCMASK_FOUND)
			||
			(status->m_stopFrequency == etaldemo_seek_frequency_start)
			)
			{
			// found or end of the seek
			
			etaldemo_seek_on_going = false;
			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
			}
	}
	else if (dwEvent == ETAL_INFO_FM_STEREO)
	{
		stereo_status = (EtalStereoStatus *)pstatus;
		etalDemoPrintf(TR_LEVEL_COMPONENT, "event INFO_FM_STEREO: hrecv: %d, stereo:%d\n", stereo_status->m_hReceiver, stereo_status->m_isStereo);
	}
	else if (dwEvent == ETAL_INFO_SEEK)
	{
		EtalSeekStatus *status = (EtalSeekStatus *)pstatus;

		//HD mod: consider bits for HD
		// ETAL_TUNESTATUS_SYNCMASK_SYNC_ACQUIRED : 1 = HD content is present
		// ETAL_TUNESTATUS_SYNCMASK_COMPLETE : 1 = HD audio acquired
		etaldemo_HD_status = 0;

		if (IsDemonstrationMode)
		{
			fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
			if ((status->m_frequency >= ETAL_BAND_FMJP_MIN) && (status->m_frequency <= ETAL_BAND_FMEU_MAX))
			{
				lastFreqUsed = status->m_frequency;
				if (etaldemo_HD_status == 0)
				{
					snprintf(XMLstring, sizeof(XMLstring),
						"<info><Band></Band><Frequency>%d</Frequency><HDStatus></HDStatus><BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
						lastFreqUsed);
				}
				write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
				close(fd_RadioText);
			}
		}

		if (((true == status->m_frequencyFound)
			||
			(status->m_frequency == etaldemo_seek_frequency_start)
			||
			(true == status->m_fullCycleReached))
			||
			(ETAL_SEEK_FINISHED == status->m_status)
			)
		{
			// found or end of  the seek
			etalDemoPrintf(TR_LEVEL_COMPONENT, "AutoSeek info event, Frequency %d, good station found %d, status = %d\n", 
						status->m_frequency, 
						status->m_frequencyFound,
						status->m_status);
			
			etaldemo_seek_on_going = false;
			etalDemo_PostEvent(ETALDEMO_EVENT_TUNED_RECEIVED);
		}

	}
	else if (dwEvent == ETAL_INFO_SCAN)
	{
		EtalScanStatusTy *status = (EtalScanStatusTy *)pstatus;

		if (true == status->m_frequencyFound)
		{
			// found during scan
			etalDemoPrintf(TR_LEVEL_COMPONENT, "Scan info event, good station found at Frequency %d\n",
						status->m_frequency);
			etalDemo_PostEvent(ETALDEMO_EVENT_SCAN_FOUND);
		}
		if (ETAL_SCAN_FINISHED == status->m_status)
		{
			etalDemo_PostEvent(ETALDEMO_EVENT_SCAN_FINISHED);
		}
	}
	else if (dwEvent == ETAL_INFO_LEARN)
	{
		EtalLearnStatusTy *status = (EtalLearnStatusTy *)pstatus;

		if (ETAL_LEARN_FINISHED == status->m_status)
		{
			etaldemo_learn_freq_nb = status->m_nbOfFrequency;
			etalDemo_PostEvent(ETALDEMO_EVENT_LEARN_FINISHED);
		}
	}
#if (defined(CONFIG_ETAL_HAVE_SEAMLESS) || defined(CONFIG_ETAL_HAVE_ALL_API))
	else if (dwEvent == ETAL_INFO_SEAMLESS_ESTIMATION_END)
	{
		EtalSeamlessEstimationStatus *seamless_estimation_status = (EtalSeamlessEstimationStatus *)pstatus;

		if (NULL != pstatus)
		{
			etaldemo_seamless_estimation_status = *seamless_estimation_status;
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_userNotificationHandler : Seamless estimation complete, Status=%d (%s), absDelay=%f s delay=%f s Conf level=%d loudness fas=%f dB loudness sas=%f dB\n",
					seamless_estimation_status->m_status, etaldemo_se_strstatus(seamless_estimation_status->m_status),
					((tF32)seamless_estimation_status->m_absoluteDelayEstimate / 48000),
					((tF32)seamless_estimation_status->m_delayEstimate / 48000), seamless_estimation_status->m_confidenceLevel,
					(tF32)(((tF32)5 * log10(seamless_estimation_status->m_RMS2_FAS)) - (tF32)0.691),
					(tF32)(((tF32)5 * log10(seamless_estimation_status->m_RMS2_SAS)) - (tF32)0.691));
		}
	}
	else if (dwEvent == ETAL_INFO_SEAMLESS_SWITCHING_END)
	{
		EtalSeamlessSwitchingStatus *seamless_switching_status = (EtalSeamlessSwitchingStatus *)pstatus;

		if (NULL != pstatus)
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_userNotificationHandler : Seamless switching complete, status=%d\n",
					seamless_switching_status->m_status);
		}
	}
#endif
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	else if (dwEvent == ETAL_INFO_SERVICE_FOLLOWING_NOTIFICATION_INFO)
	{

		EtalTuneServiceIdStatus *pl_TuneServiceIdStatus = (EtalTuneServiceIdStatus *) pstatus;

		etalDemo_SFEventHandling(pl_TuneServiceIdStatus);

		hCurrentReceiver = pl_TuneServiceIdStatus->m_receiverHandle;

	}
#endif
	else
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Unexpected event %d\n", dwEvent);
	}
}

/***************************
 *
 * etalDemo_parseParameters
 *
 **************************/
int etalDemo_parseParameters(int argc, char **argv, EtalFrequencyBand *pI_Band, tU32 *pO_freq, tU32 *pO_tunerch)
{
#define ETAL_DEMO_BAD_PARAM -1
#define ETAL_DEMO_RETURN_OK 0


	int vl_res = ETAL_DEMO_RETURN_OK;
	int i = 0, idx = 0, c;
	
	// Let's set the default parameters
	for(i = 0; i < ETALDEMO_NB_TUNER_CHANNEL_MAX; i++)
	{
		pI_Band[i] = ETAL_BAND_UNDEF;
		pO_tunerch[i] = 0;
		pO_freq[i] = 0;
		pO_tunerch[i] = ETALDEMO_TUNERCH_NONE;
	}

	for (i=1; (i < argc) && (vl_res == ETAL_DEMO_RETURN_OK); i++)
	{
		switch (argv[i][0])
		{
			case 'f':
#ifdef HAVE_FM
				pI_Band[idx] = ETAL_BAND_FMEU;
				if (0 == pO_freq[idx]) pO_freq[idx] = ETAL_BAND_FMEU_MIN;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
				vl_res = ETAL_DEMO_BAD_PARAM;
#endif
				break;
			case 'j':
#ifdef HAVE_FM
				pI_Band[idx] = ETAL_BAND_FMJP;
				if (0 == pO_freq[idx]) pO_freq[idx] = ETAL_BAND_FMJP_MIN;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
				vl_res = ETAL_DEMO_BAD_PARAM;
#endif
				break;
			case 'u':
#ifdef HAVE_FM
				pI_Band[idx] = ETAL_BAND_FMUS;
				if (0 == pO_freq[idx]) pO_freq[idx] = ETAL_BAND_FMUS_MIN;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
				vl_res = ETAL_DEMO_BAD_PARAM;
#endif
				break;
			case 'h':
#ifdef HAVE_HD
				pI_Band[idx] = ETAL_BAND_HD;
				if (0 == pO_freq[idx]) pO_freq[idx] = ETAL_BAND_FMUS_MIN;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_HD was not activated during build\n");
				vl_res = ETAL_DEMO_BAD_PARAM;
#endif
				break;
			case 'd':
#ifdef HAVE_DAB
				pI_Band[idx] = ETAL_BAND_DAB3;
#else
				etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_DAB was not activated during build\n");
				vl_res = ETAL_DEMO_BAD_PARAM;
#endif
				break;
			case 't':
				pO_tunerch[idx] = ETALDEMO_TUNERCH_FG;
				c = argv[i][1];
				if ((c >= '1') && (c <= ('0' + ETALDEMO_NB_TUNER_MAX)))
				{
					pO_tunerch[idx] |= (c << 4);
				}
				// TODO : check if frequency is valid (is a number and is in adequate range)
				if ((i+1 < argc) && (isdigit(argv[i+1][0])))
				{
					pO_freq[idx] = atoi(argv[i+1]);
					i++; //Skip parsing next parameter again
				}
				else
				{
					etalDemoPrintf(TR_LEVEL_SYSMIN, "frequency missing, takes %d\n", pO_freq[idx]);
				}
				idx++;
				break;
			case 'b':
				pO_tunerch[idx] = ETALDEMO_TUNERCH_BG;
				c = argv[i][1];
				if ((c >= '1') && (c <= ('0' + ETALDEMO_NB_TUNER_MAX)))
				{
					pO_tunerch[idx] |= (c << 4);
				}
				// TODO : check if frequency is valid (is a number and is in adequate range)
				if ((i+1 < argc) && (isdigit(argv[i+1][0])))
				{
					pO_freq[idx] = atoi(argv[i+1]);
					i++; //Skip parsing next parameter again
				}
				else
				{
					etalDemoPrintf(TR_LEVEL_SYSMIN, "frequency missing, takes %d\n", pO_freq[idx]);
				}
				idx++;
				break;
			default:
				break;
		}
	}

	// Check if anything is missing
	if ((pI_Band[0] == ETAL_BAND_UNDEF) || (vl_res != ETAL_DEMO_RETURN_OK))
	{
		vl_res = ETAL_DEMO_BAD_PARAM;
	}

	if ( ETAL_DEMO_BAD_PARAM == vl_res)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "Usage : %s [f|j|u|h|d] [t|b|tx|bx <freq>]\n", argv[0]);
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tf = FM radio (Europe band) as start\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tj = FM radio (Japanese band) as start\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tu = FM radio (US band) as start\n");
#ifdef HAVE_HD
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\th = FM/HD radio (US band) as start\n");
#endif
#ifdef HAVE_DAB
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\td = DAB radio as start\n");
#endif
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tt = tune forground on frequency <freq>\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\ttx = tune tuner x [1, 2] foreground channel on frequency <freq>\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tb = tune background on frequency <freq>\n");
		etalDemoPrintf(TR_LEVEL_COMPONENT, "\tbx = tune tuner x [1, 2] background channel on frequency <freq>\n");
	}

	return vl_res;
}

// event creation
// Init globals
tSInt etalDemo_initEvent()
{
	tSInt vl_res;


	vl_res = OSAL_s32EventCreate ((tCString)"EtalDemo", &etalDemo_EventHandler);

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_initEvent : ret %d\n", vl_res);
		
	return(vl_res);

 

}

tSInt etalDemo_PostEvent(tU8 event)
{
	tSInt vl_res;

	etalDemoPrintf(TR_LEVEL_COMPONENT, "etalDemo_PostEvent : postEvent %d\n", event);
		
	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
                       ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);

	return(vl_res);

}

tSInt etalDemo_ClearEvent(tU8 event)
{
	tSInt vl_res;

	
		// Clear old event if any (this can happen after a stop)


	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~((tU32)0x01 << event)), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

}

tSInt etalDemo_ClearEventFlag(OSAL_tEventMask event)
{
	tSInt vl_res;

	
	// Clear old event if any (this can happen after a stop)


	vl_res = OSAL_s32EventPost (etalDemo_EventHandler, 
						  (~event), OSAL_EN_EVENTMASK_AND);

	return(vl_res);

}

/***************************
 *
 * printRadioText
 *
 **************************/
static void etalDemo_printRadioText(char * prefix, EtalTextInfo *radio_text)
{
	char XMLstring[512];

    if ((!IsDemonstrationMode)&&(RDS_on))
    {
        if ((radio_text->m_serviceNameIsNew) || (radio_text->m_currentInfoIsNew))
        {
            printf("%s Radio text Broadcast Standard: %s\n", prefix, standard2Ascii(radio_text->m_broadcastStandard));
        }
        if (radio_text->m_serviceNameIsNew)
        {
            printf("%s Radio text Service name: %s\n", prefix, radio_text->m_serviceName);
        }

        if (radio_text->m_currentInfoIsNew)
        {
            printf("%s Radio text Current Info: %s\n", prefix, radio_text->m_currentInfo);
        }
    }
    else
    {
		if (RDS_on)
		{
            snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band></Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><HDStatus></HDStatus>"\
				"<BroadcastStandard>%s</BroadcastStandard><ServiceName>%s</ServiceName><RadioText>%s</RadioText></info>\n",
				lastFreqUsed, tunedEnsemble,
			standard2Ascii(radio_text->m_broadcastStandard), radio_text->m_serviceName, radio_text->m_currentInfo);
		}
		else
		{
            snprintf(XMLstring, sizeof(XMLstring),
				"<info><Band></Band><Frequency>%d</Frequency><Ensemble>%d</Ensemble><HDStatus></HDStatus>"\
				"<BroadcastStandard></BroadcastStandard><ServiceName></ServiceName><RadioText></RadioText></info>\n",
			lastFreqUsed, tunedEnsemble);
		}
        fd_RadioText = open(RadioTextFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
        write(fd_RadioText, XMLstring, strlen(XMLstring)+1);
        close(fd_RadioText);
    }
}

/***************************
 *
 * startRadioText
 *
 **************************/
static int etalDemo_startRadioText(ETAL_HANDLE vl_handle, ETAL_HANDLE *vl_hDatapathRds)
{
	int ret;
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	EtalTextInfo vl_Radiotext;
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

	EtalDataPathAttr vl_dataRadioTextPathAttr;

	// manage the RadioText

	/*
	 * [Optional] set up an event to get signalled when new Radio Text Data are available.
	 * since the (dynamic info
	 */
	if (ETAL_INVALID_HANDLE != vl_handle)
	{
		// stop prior radioText
		etaltml_stop_textinfo(vl_handle);
	}

    if (ETAL_INVALID_HANDLE != *vl_hDatapathRds)
    {
		// destroy the datapath
		etal_destroy_datapath(vl_hDatapathRds);
	}

	*vl_hDatapathRds = ETAL_INVALID_HANDLE;
	memset(&vl_dataRadioTextPathAttr, 0x00, sizeof(EtalDataPathAttr));
	vl_dataRadioTextPathAttr.m_receiverHandle = vl_handle;
	vl_dataRadioTextPathAttr.m_dataType = ETAL_DATA_TYPE_TEXTINFO;
	vl_dataRadioTextPathAttr.m_sink.m_context = (void *)0;
	vl_dataRadioTextPathAttr.m_sink.m_BufferSize = sizeof(EtalTextInfo);
	vl_dataRadioTextPathAttr.m_sink.m_CbProcessBlock = etalDemo_RadiotextCallback;

	if ((ret = etal_config_datapath(vl_hDatapathRds, &vl_dataRadioTextPathAttr)) != ETAL_RET_SUCCESS)
		{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etal_config_datapath for Radiotext (%d)\n", ret);
		return ret;
		}

#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	if ((ret = etaltml_start_textinfo(vl_handle)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_ERRORS, "ERROR: etaltml_start_textinfo (%d)\n", ret);
		return ret;
	}

	// radio Text notification will happen only if something new
	// so get current
	etaltml_get_textinfo(vl_handle, &vl_Radiotext);
	etalDemo_printRadioText("  etalDemo_currentRadioTextInfo : ", &vl_Radiotext);
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)

	return ret;
}

/***************************
 *
 * etalTestRadiotextCallback
 *
 **************************/

void etalDemo_RadiotextCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalDemo_printRadioText("  etalDemo_RadioTextInfo : ", (EtalTextInfo *) pBuffer);
}


/***************************
 *
 * main
 *
 **************************/
/*
 * Returns:
 * 1 error(s)
 * 0 success
 */
int main(int argc, char **argv)
{
	EtalFrequencyBand vl_Band[ETALDEMO_NB_TUNER_CHANNEL_MAX];
	tU32 vl_freq[ETALDEMO_NB_TUNER_CHANNEL_MAX], vl_tunerch[ETALDEMO_NB_TUNER_CHANNEL_MAX];
	int i;

	if (etalDemo_parseParameters(argc, argv, vl_Band, vl_freq, vl_tunerch))
	{
		return 1;
	}

	/* initialize receivers handle */
	for(i = 0; i < (ETALDEMO_NB_TUNER_CHANNEL_MAX + 1); i++)
	{
		hReceivers[i] = ETAL_INVALID_HANDLE;
	}

	/* initialize datapath handle */
	for(i = 0; i < (ETALDEMO_NB_DATAPATH_MAX); i++)
	{
		hDatapaths[i] = ETAL_INVALID_HANDLE;
	}

	/* initialize monitor handle */
	for(i = 0; i < (ETALDEMO_NB_DATAPATH_MAX); i++)
	{
		hMonitors[i] = ETAL_INVALID_HANDLE;
	}

	etalDemo_Appli_entryPoint(vl_Band, vl_freq, vl_tunerch);

	return 0;
}


// Configure the audio path for Tuner
//
static void etalDemo_ConfigureAudioForTuner()
{
	// this is based on alsa mixer in Linux
	// This requires the appropriate audio fwk

	// ETAL audio select has been modified to support digital audio configuration,
	// so no need to care about digital audio or band choice anymore
	
#ifdef HAVE_HD
	int ret;
	EtalAudioInterfTy audioIf;

//HD mod: in this case, DCOP680 is the I2S master, so CMOST has to be configured as SAI slave

	/* Configure audio path on CMOST */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "etal_config_audio_path (%d)", ret);
		return;
	}
#endif // have_hd	

#ifndef CONFIG_HOST_OS_TKERNEL
#ifdef CONFIG_BOARD_ACCORDO5
#if (!defined(CONFIG_DIGITAL_AUDIO)) && (!defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST))
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
	if (!IsDemonstrationMode) system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
#endif

#ifdef CONFIG_BOARD_ACCORDO2
	// select the audio source
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
#if (defined(CONFIG_BOARD_ACCORDO2) || defined(CONFIG_BOARD_ACCORDO5))
	// Set the output volume 
	if (!IsDemonstrationMode) system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
#endif // CONFIG_BOARD_ACCORDO2 || CONFIG_BOARD_ACCORDO5
#endif // #ifndef CONFIG_HOST_OS_TKERNEL
}

// vI_Band : indicate if we work on FM or DAB
// vI_modeIsSeek : indicates if we start by a tune or a seek
// vI_frequency (for tune case)
//
void etalDemo_Appli_entryPoint(EtalFrequencyBand *p_Band, tU32 *p_freq, tU32 *p_tunerch)
{
	int ret, i;
	ETAL_HANDLE vl_handle = ETAL_INVALID_HANDLE;
	EtalHardwareAttr init_params;
	char line[256], *ptok;
	tBool vl_continue = true, vl_cmd_found = false;
	tBool looped = false;
	ETAL_STATUS vl_status;
#if 0
	char XMLstring[512];
#endif

	/*
	 * Initialize audio path
	 */
#if defined(CONFIG_BOARD_ACCORDO5) && defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST) && (defined(CONFIG_ETAL_HAVE_SEAMLESS) || defined(CONFIG_ETAL_HAVE_ALL_API))
#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	// audio path selection
	system("amixer -c 3 sset Source tunerss > /dev/null" );

	// select the MSP clock
	system("amixer -c2 set Clock ext > /dev/null");
#endif
#endif


	/*
	 * Initialize ETAL
	 */
	
	lastFreqUsed = -1;
	tunedEnsemble = false;
	memset(&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = etalDemo_userNotificationHandler;

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_initialize (%d)\n", ret);
		return;
	}
	
	// create an event to work on notification
	//
	etalDemo_initEvent();

	/* do tuner command line configuration, audio selected for i = 0 */
	for(i = 0; i < ETALDEMO_NB_TUNER_CHANNEL_MAX; i++)
	{
		if (p_Band[i] != ETAL_BAND_UNDEF)
		{
			if ((ETAL_BAND_FMEU == p_Band[i]) || (ETAL_BAND_FMJP == p_Band[i]) ||
				(ETAL_BAND_FMUS == p_Band[i]) || (ETAL_BAND_HD == p_Band[i]))
			{
#ifdef HAVE_FM
				if (p_tunerch[i] == ETALDEMO_TUNERCH_FG)
				{
					hReceivers[i] = etalDemo_TuneFMFgRadio(p_freq[i], p_Band[i], (i == 0));
				}
				else if (p_tunerch[i] == ETALDEMO_TUNERCH_BG)
				{
					/* TODO */
				}
#endif
			}
			else if (ETAL_BAND_DAB3 == p_Band[i])
			{				
#ifdef HAVE_DAB
				if (p_tunerch[i] == ETALDEMO_TUNERCH_FG)
				{
					hReceivers[i] = etalDemo_TuneDAB1Radio(p_freq[i], (i == 0));
				}
				else if (p_tunerch[i] == ETALDEMO_TUNERCH_BG)
				{
					hReceivers[i] = etalDemo_TuneDAB2Radio(p_freq[i]);
				}
#endif
			}
			
		}
		if (!IsDemonstrationMode)
		{
			etalDemo_ConfigureAudioForTuner();
			looped = TRUE;
		}
	}

	do
	{
#if 0
		if ((IsDemonstrationMode) || (looped))
		{
			etalDemoPrintf(TR_LEVEL_COMPONENT, "******************* BAND SELECTION ********************\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tf = FM radio (Europe band)\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tj = FM radio (Japanese band)\n");
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\tu = FM radio (US band)\n");
#ifdef HAVE_HD
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\th = FM/HD radio (US band)\n");
#endif
#ifdef HAVE_DAB
			etalDemoPrintf(TR_LEVEL_COMPONENT, "\td = DAB radio\n");
#endif
			etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'q' -> proceed to the exit **** \n");
		}
		else
		{

				if (false == IsDemonstrationMode)
				{
					etalDemo_ConfigureAudioForTuner();
				}

			etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		}
#endif
		if (IsDemonstrationMode)
		{
			//Destroy current receiver, if any
			etalDemoEndTest();
		}

		if (fgets(line, sizeof(line), stdin)) vl_continue = true;

		if (IsDemonstrationMode)
		{
			etalDemo_ConfigureAudioForTuner();
		}

		if ((IsDemonstrationMode) || (looped))
		{
			switch (line[0])
			{
				case 'f':
#ifdef HAVE_FM
					p_Band[0] = ETAL_BAND_FMEU;
					p_freq[0] = ETAL_BAND_FMEU_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return;
#endif
					break;
				case 'j':
#ifdef HAVE_FM
					p_Band[0] = ETAL_BAND_FMJP;
					p_freq[0] = ETAL_BAND_FMJP_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return;
#endif
					break;
				case 'u':
#ifdef HAVE_FM
					p_Band[0] = ETAL_BAND_FMUS;
					p_freq[0] = ETAL_BAND_FMUS_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_FM was not activated during build\n");
					return;
#endif
					break;
				case 'h':
#ifdef HAVE_HD
					p_Band[0] = ETAL_BAND_HD;
					p_freq[0] = ETAL_BAND_FMUS_MIN;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_HD was not activated during build\n");
					return;
#endif
					break;
				case 'd':
#ifdef HAVE_DAB
					p_Band[0] = ETAL_BAND_DAB3;
#else
					etalDemoPrintf(TR_LEVEL_FATAL, "HAVE_DAB was not activated during build\n");
					return;
#endif
					break;
				case 'e':
					vl_cmd_found = false;
					ptok = strtok(line, ETALDEMO_PARAM_DELIMITER);
					/* remove CR LF for command without parameters */
					if ((strlen(ptok) > 0) && ((ptok[(strlen(ptok) - 1)] == '\r') || (ptok[(strlen(ptok) - 1)] == '\n')))
					{
						ptok[(strlen(ptok) - 1)] = 0;
					}
					if (ptok != NULL)
					{
						for(i = 0; i < ETALDEMO_API_CMD_NB; i++)
						{
#if 0
							if ((strlen(ptok) == strlen(etaldemo_api_cmd_list[i].api_name)) &&
								(strncmp(ptok, etaldemo_api_cmd_list[i].api_name, strlen(etaldemo_api_cmd_list[i].api_name)) == 0))
							{
								vl_status = etaldemo_api_cmd_list[i].api_cmd_cb();
								vl_cmd_found = true;
								break;
							}
#endif
							if ((strlen(ptok) == strlen(etaldemo_api_cmd_list[i].api_short_name)) && 
								(strncmp(ptok, etaldemo_api_cmd_list[i].api_short_name, strlen(etaldemo_api_cmd_list[i].api_short_name)) == 0))
							{
								vl_status = etaldemo_api_cmd_list[i].api_cmd_cb();
								if (ETAL_RET_SUCCESS != vl_status)
								{
									etalDemoPrintf(TR_LEVEL_ERRORS, "%s (%d) = %s()\n", ETALDEMO_STATUS_toString(vl_status), vl_status, etaldemo_api_cmd_list[i].api_name);
								}
								vl_cmd_found = true;
								break;
							}
						}
					}
					else
					{
						etalDemoPrintf(TR_LEVEL_ERRORS, "ptok NULL\n");
					}
					if (vl_cmd_found == false)
					{
						etalDemoPrintf(TR_LEVEL_ERRORS, "etal api command not found (%s)\n", ptok);
					}
					break;
				case 'q' : case '\n' : case '\r' :
					vl_continue = false;
					break;
				default:
					break;
			}
			//vI_modeIsSeek = true;
		}

		if (vl_continue)
		{
#if 0
			//Destroy current receiver, if any
			etalDemoEndTest();
			if ((ETAL_BAND_FMEU == vI_Band)||(ETAL_BAND_FMJP == vI_Band)||
				(ETAL_BAND_FMUS == vI_Band)||(ETAL_BAND_HD == vI_Band))
			{
#ifdef HAVE_FM
				if (true == vI_modeIsSeek)
				{
					vl_handle = etalDemo_FMSeekRadio(vI_frequency, vI_Band);
				}
				else
				{
					vl_handle = etalDemo_TuneFMFgRadio(vI_frequency, vI_Band);
				}
#endif
			}
			else if (ETAL_BAND_DAB3 == vI_Band)
			{				
#ifdef HAVE_DAB
				if (true == vI_modeIsSeek)
				{			
					vl_handle = etalDemo_DABRadioSeek(ETAL_START_DAB_FREQ);
				}
				else
				{
					vl_handle = etalDemo_TuneDAB1Radio(vI_frequency);
					hSecondReceiver = etalDemo_TuneDAB2Radio(vI_BGfrequency);
				}
#endif
			}
			else
			{
				// not supported
			}
#endif
			hCurrentReceiver = vl_handle;
		}

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#if 0
    	if (!IsDemonstrationMode)
	    {
		    //now activate the SF

		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** ENTER A KEY TO CONTINUE ***************\n");
		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t 'f' -> to activate SF (then any key to exit)\n");
		    etalDemoPrintf(TR_LEVEL_COMPONENT, "*\t anyother -> end test **** \n");

		    if (fgets(line, sizeof(line), stdin))
		    {
	            if (line[0] == 'f')
    	        {
	                etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF activation ***************\n");

    	            if ((ret = etaltml_ActivateServiceFollowing()) != ETAL_RET_SUCCESS)
        	        {
            	        etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etaltml_ActivateServiceFollowing (%d)", ret);
                	    return;
	                }

    	            etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** SF activated ***************\n");
        	    }
		    }
	    }
#endif
#endif

		if ((!IsDemonstrationMode) && (vl_continue == false))
		{
#if 0
	    	etalDemoPrintf(TR_LEVEL_COMPONENT, "*************** Do you want to quit? (y/n) ***************\n");

		    if (fgets(line, sizeof(line), stdin))
		    {
		    	if (line[0] == 'y') 
				{	
					vl_continue = false;
		    	}
				else
				{
					vl_continue = true;
				}
				
		    }
#endif
			looped = true;
		}

	}while(true == vl_continue);
		
	// destroy handles
	etalDemoEndTest();
	
	/*
	 * Final cleanup
	 */
	
	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		etalDemoPrintf(TR_LEVEL_COMPONENT, "ERROR: etal_deinitialize\n");
	}

    if (IsDemonstrationMode)
    {
#ifndef CONFIG_HOST_OS_WIN32
		unlink(CtrlFIFO);
#endif // !CONFIG_HOST_OS_WIN32
    }
	
	return;
}

