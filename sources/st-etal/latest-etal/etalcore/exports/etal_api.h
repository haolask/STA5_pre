
//!
//!  \file 		etal_api.h
//!  \brief 	<i><b> ETAL API </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Raffaele Belardi
//!

#ifndef ETAL_API_H_
#define ETAL_API_H_

#include "etal_types.h"
#include "etaltml_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * System initialization and configuration
 */
ETAL_STATUS etal_initialize(const EtalHardwareAttr* pHWInit);
ETAL_STATUS etal_setTcpIpAddress(tChar* address);
ETAL_STATUS etal_tuner_initialize(tU32 deviceID, const EtalTunerAttr *tuner_init_params,tBool IsAlreadyStarted);
ETAL_STATUS etal_dcop_initialize(const EtalDCOPAttr *dcop_init_params, EtalDcopInitTypeEnum InitType);
ETAL_STATUS etal_get_capabilities(EtalHwCapabilities** pCapabilities);
ETAL_STATUS etal_config_receiver(ETAL_HANDLE* pReceiver, const EtalReceiverAttr* pReceiverConfig);
ETAL_STATUS etal_destroy_receiver(ETAL_HANDLE* pReceiver);
ETAL_STATUS etal_config_datapath(ETAL_HANDLE *pDatapath, const EtalDataPathAttr *pDatapathAttr);
ETAL_STATUS etal_destroy_datapath(ETAL_HANDLE* pDataPathHandle);
ETAL_STATUS etal_deinitialize(void);
ETAL_STATUS etal_config_audio_path(tU32 tunerIndex, EtalAudioInterfTy intf);
ETAL_STATUS etal_audio_select(ETAL_HANDLE hReceiver, EtalAudioSourceTy src);
ETAL_STATUS etal_force_mono(ETAL_HANDLE hReceiver, tBool forceMonoFlag);
ETAL_STATUS etal_mute(ETAL_HANDLE hReceiver, tBool muteFlag);
ETAL_STATUS etal_event_FM_stereo_start(ETAL_HANDLE hReceiver);
ETAL_STATUS etal_event_FM_stereo_stop(ETAL_HANDLE hReceiver);
ETAL_STATUS etal_debug_config_audio_alignment(EtalAudioAlignmentAttr *alignmentParams);
ETAL_STATUS etal_receiver_alive(ETAL_HANDLE hReceiver);
ETAL_STATUS etal_reinitialize(EtalNVMLoadConfig NVMLoadConfig);
ETAL_STATUS etal_xtal_alignment(ETAL_HANDLE hReceiver, tU32 *calculatedAlignment);
ETAL_STATUS etal_get_version(EtalVersion *vers);
ETAL_STATUS etal_clear_landscape(EtalMemoryClearConfig MemClearConfig);
ETAL_STATUS etal_save_landscape(EtalNVMSaveConfig MemSaveConfig);
ETAL_STATUS etal_get_init_status(EtalInitStatus *status);
ETAL_STATUS etal_backup_context_for_early_audio(const etalCtxBackupEarlyAudioTy *CtxEarlyAudio);

/*
 * Tune/Seek/Quality
 */
ETAL_STATUS etal_tune_receiver(ETAL_HANDLE hReceiver, tU32 Frequency);
ETAL_STATUS etal_tune_receiver_async(ETAL_HANDLE hReceiver, tU32 Frequency);
ETAL_STATUS etal_change_band_receiver(ETAL_HANDLE hReceiver, EtalFrequencyBand band, tU32 fmin, tU32 fmax, EtalProcessingFeatures processingFeatures);
ETAL_STATUS etal_get_reception_quality(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* pBcastQuality);
ETAL_STATUS etal_get_channel_quality(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality);
ETAL_STATUS etal_config_reception_quality_monitor(ETAL_HANDLE* pMonitorHandle, const EtalBcastQualityMonitorAttr* pMonitor);
ETAL_STATUS etal_destroy_reception_quality_monitor(ETAL_HANDLE* pMonitorHandle);
ETAL_STATUS etal_get_receiver_frequency(ETAL_HANDLE hReceiver, tU32* Frequency);
ETAL_STATUS etal_get_CF_data(ETAL_HANDLE hReceiver, EtalCFDataContainer* pResp, tU32 nbOfAverage, tU32 period);
ETAL_STATUS etal_seek_start_manual(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, tU32 *freq);
ETAL_STATUS etal_seek_continue_manual(ETAL_HANDLE hReceiver, tU32 *freq);
ETAL_STATUS etal_seek_stop_manual(ETAL_HANDLE hReceiver, etalSeekAudioTy exitSeekAction, tU32 *freq);
ETAL_STATUS etal_seek_get_status_manual(ETAL_HANDLE hReceiver, EtalSeekStatus *seekStatus);
ETAL_STATUS etal_AF_switch(ETAL_HANDLE hReceiver, tU32 alternateFrequency);
ETAL_STATUS etal_AF_check(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
ETAL_STATUS etal_AF_start(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* result);
ETAL_STATUS etal_AF_end(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* result);
ETAL_STATUS etal_AF_search_manual(ETAL_HANDLE hReceiver, tU32 antennaSelection, tU32 *AFList, tU32 nbOfAF, EtalBcastQualityContainer* AFQualityList);
ETAL_STATUS etal_audio_output_scaling(tU32 volume);

/*
 * Advanced Tuning
 */
ETAL_STATUS etal_get_current_ensemble(ETAL_HANDLE hReceiver, tU32 *pUEId);
ETAL_STATUS etal_service_select_audio(ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, tU32 UEId, tU32 service, tSInt sc, tSInt subch);
ETAL_STATUS etal_service_select_data(ETAL_HANDLE hDatapath, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 UEId, tU32 service, tSInt sc, tSInt subch);

/*
 * System Data
 */
ETAL_STATUS etal_get_ensemble_list(EtalEnsembleList *pEnsembleList);
ETAL_STATUS etal_get_ensemble_data(tU32 eid, tU8 *charset, tChar *label, tU16 *bitmap);
ETAL_STATUS etal_get_fic(ETAL_HANDLE hReceiver);
ETAL_STATUS etal_stop_fic(ETAL_HANDLE hReceiver);
ETAL_STATUS etal_get_service_list(ETAL_HANDLE hReceiver, tU32 eid, tBool bGetAudioServices, tBool bGetDataServices, EtalServiceList *pServiceList);
ETAL_STATUS etal_get_specific_service_data_DAB(tU32 eid, tU32 sid, EtalServiceInfo *serv_info, EtalServiceComponentList *sclist, void *dummy);
ETAL_STATUS etal_get_time(ETAL_HANDLE hReceiver, EtalTime *ETALTime);

/*
 * Data Services
 */
ETAL_STATUS etal_enable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap, tU32 *EnabledServiceBitmap, EtalDataServiceParam ServiceParameters);
ETAL_STATUS etal_disable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap);
ETAL_STATUS etal_setup_PSD (ETAL_HANDLE hReceiver, tU16 PSDServiceEnableBitmap, EtalPSDLength* pConfigLenSet, EtalPSDLength* pConfigLenGet);

/*
 * Read / write parameter
 */
ETAL_STATUS etal_read_parameter(tU32 tunerIndex, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength);
ETAL_STATUS etal_write_parameter(tU32 tunerIndex, etalReadWriteModeTy mode, tU32 *paramValueArray, tU16 length);
ETAL_STATUS etal_write_parameter_convert(tU32 *outbuf, tU32 *address, tU32 *value, tU16 length);

/*
 * RDS raw services
 */
ETAL_STATUS etal_start_RDS(ETAL_HANDLE hReceiver, tBool forceFastPi, tU8 numPi, EtalRDSRBDSModeTy mode, tU8 errthresh);
ETAL_STATUS etal_stop_RDS(ETAL_HANDLE hReceiver);

/*
 * Debug and Evaluation operations
 */
ETAL_STATUS etal_debug_DISS_control(ETAL_HANDLE hReceiver, etalChannelTy tuner_channel, EtalDISSMode mode, tU8 filter_index);
ETAL_STATUS etal_debug_get_WSP_Status(ETAL_HANDLE hReceiver, EtalWSPStatus *WSPStatus);
ETAL_STATUS etal_debug_VPA_control(ETAL_HANDLE hReceiver, tBool status, ETAL_HANDLE *hReceiver_bg);
ETAL_STATUS etal_trace_config(EtalTraceConfig *config);

/*
 * Seamless estimation and seamless switching
 */
ETAL_STATUS etal_seamless_estimation_start(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig_ptr);
ETAL_STATUS etal_seamless_estimation_stop(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS);
ETAL_STATUS etal_seamless_switching(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig_ptr);

/*
 * Tune/Seek/Quality
 */

ETAL_STATUS etal_autoseek_start(ETAL_HANDLE hReceiver, etalSeekDirectionTy direction, tU32 step, etalSeekAudioTy exitSeekAction, etalSeekHdModeTy seekHDSPS, tBool updateStopFrequency);
ETAL_STATUS etal_autoseek_stop(ETAL_HANDLE hReceiver, EtalSeekTerminationModeTy terminationMode);
ETAL_STATUS etal_autoseek_continue(ETAL_HANDLE hReceiver);
ETAL_STATUS etal_set_autoseek_thresholds_value(ETAL_HANDLE hReceiver, EtalSeekThreshold* seekThreshold);

/*
 * Auto notification
 */
ETAL_STATUS etal_setup_autonotification(ETAL_HANDLE hReceiver, tU16 eventBitmap, tU16 *enabledEventBitmap);

#ifdef __cplusplus
}
#endif

#endif /* ETAL_API_H_ */
