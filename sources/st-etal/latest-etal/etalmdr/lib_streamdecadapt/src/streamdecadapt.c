//!
//!  \file       streamdecadapt.c
//!  \brief      <i><b>This file contains functions for streamdecoder adaptation</b></i>
//!  \author     David Pastor
//!  \version    1.0
//!  \date       2017.11.21
//!  \bug        Unknown
//!  \warning    None
//!

#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------
// Includes and externally used resources inclusion
//----------------------------------------------------------------------
#include "osalIO.h"

#include "Signals.h"
#include "MMObject.h"
#include "StreamDecoder.h"

#include "streamdecadapt.h"

#include "ci_codec_type.h"
#include "ci_dmb_api.h"
#include "ci_h264bpdec.h"
#include "ci_jpegenc.h"
#include "dabplus_aacsuperframe.h"

#if defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
#include "etalinternal.h"
#include "seamless_switching_common.h"
#include "sd_messages.h"
#ifdef CONFIG_UTILS_MSGS_SUPPORT
#include "system_msgs.h"
#endif
#endif // #if defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK)
#if defined(CONFIG_TARGET_DABMW_STREAMDECODER_COMM_ENABLE)
#include "streamdec_comm.h"
#endif

//----------------------------------------------------------------------
// Global variables definition
//----------------------------------------------------------------------
extern MMStreamDecoder *streamDecoder;
extern OSAL_trDevDesc OSAL_arIODevTable[OSAL_EN_DEVID_LAST];

//----------------------------------------------------------------------
// Local variables definition
//----------------------------------------------------------------------
static tBool DABMW_DRCinfoStatus;
static tU8 DABMW_DRCinfoBuffer[2] = {0, 0}; 

//----------------------------------------------------------------------
// Global function implementations
//----------------------------------------------------------------------

//!
//! \brief     <i><b> Remove an osalio device </b></i>
//! \details   Frees pDevFunctionTable and pDevOpenTable allocated memory
//! \param[in] DevId      device Id of type OSAL_tenDevID
//! \return    OSAL_OK    destroy success
//! \sa n.a.
//! \callgraph
//! \callergraph
//!
tS32 OSALIO_s32RemoveDevice (/*OSAL_tenDevID*/ tU32 DevId)
{
	if (OSAL_arIODevTable[DevId].pDevFunctionTable != OSAL_NULL)
	{
		OSAL_vMemoryFree(OSAL_arIODevTable[DevId].pDevFunctionTable);
		OSAL_arIODevTable[DevId].pDevFunctionTable = OSAL_NULL;
	}
	if (OSAL_arIODevTable[DevId].pDevOpenTable != OSAL_NULL)
	{
		OSAL_vMemoryFree(OSAL_arIODevTable[DevId].pDevOpenTable);
		OSAL_arIODevTable[DevId].pDevOpenTable = OSAL_NULL;
	}
	return OSAL_OK;
}

//!
//! \brief   <i><b> Destroy StreamDecoder context </b></i>
//! \details Sends SIGNAL_END_OF_STREAM to StreamDecoder thread that stops loop,
//!          calls all StreamDecoder blocks destroy functions and delete MMObject
//! \return  OSAL_OK    destroy success
//! \return  OSAL_ERROR destroy failed, could be because 
//!                     streamDecoder->klass->get_callback_handler is null
//! \sa n.a.
//! \callgraph
//! \callergraph
//!
tS32 streamdecadapt_destructor(void)
{
	tS32 ret = OSAL_OK;
	tMMCallback mmCallback;

	if (streamDecoder != OSAL_NULL)
	{
		mmCallback = streamDecoder->klass->get_callback_handler(streamDecoder, HANDLER_TYPE_IS_DEF_CALLBACK);
		if (mmCallback != OSAL_NULL)
		{
			/* call desctructor of streamDecoder */
			(*mmCallback)(OSAL_NULL, SIGNAL_END_OF_STREAM, streamDecoder, OSAL_NULL);
		}
		else
		{
			ret = OSAL_ERROR;
		}
	}
	/* deinit audio_io */
#ifdef  CONFIG_TARGET_DEV_AUDIO_IO__ENABLE
	AUDIO_IO_vIODeInit();
#if (defined(CONFIG_TARGET_SYS_ACCORDO5) && defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK))
	AUDIO_IOIN_vIODeInit();
#endif
#endif

#ifdef CONFIG_UTILS_MSGS_SUPPORT
	if (ret == OSAL_OK)
	{
		/* deinit MW to/from SD and SD t/fromo LP_SD messages */
		if ((ret = UTILS_SystemMessagesDeinit()) != OSAL_OK)
		{
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
			OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_APP_DABMW, "UTILS_SystemMessagesDeinit return error 0x%x", ret);
#endif
		}
	}
#endif

	return ret;
}

#if defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK) && defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
/***************************
 *
 * DABMW_SeamlessInit
 *
 **************************/
tVoid DABMW_SeamlessInit(tVoid)
{
#if 0
    /* init the response */
    dabmw_msg_seamless_report.status = SD_SEAMLESS_ESTIMATION_STATUS_NONE;
#endif
}

/***************************
 *
 * DABMW_SeamlessEstimationResponse
 *
 **************************/
tVoid DABMW_SeamlessEstimationResponse (tU8 msgNum, tPU8 msgPayload)
{
	SD_msg_SeamlessEstimationResponse *sd_msg_seamless_estimation_rsp_ptr = (SD_msg_SeamlessEstimationResponse *)(msgPayload-1);
	EtalSeamlessEstimationStatus seamlessEstimation;
	tBool vl_externalNotificationToBeSent = true;

	seamlessEstimation.m_receiverHandle = ETAL_receiverSearchActiveSpecial(cmdSpecialSeamlessEstimation);
	if (seamlessEstimation.m_receiverHandle == ETAL_INVALID_HANDLE)
	{
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_APP_DABMW, "Unhandled event SeamlessEstimationResponse");
#endif
		return;
	}
	seamlessEstimation.m_status = sd_msg_seamless_estimation_rsp_ptr->status;
	seamlessEstimation.m_providerType = sd_msg_seamless_estimation_rsp_ptr->provider_type;
	seamlessEstimation.m_absoluteDelayEstimate = sd_msg_seamless_estimation_rsp_ptr->absolute_estimated_delay_in_samples;
	seamlessEstimation.m_delayEstimate = sd_msg_seamless_estimation_rsp_ptr->delay_estimate;
	seamlessEstimation.m_timestamp_FAS = sd_msg_seamless_estimation_rsp_ptr->timestamp_FAS;
	seamlessEstimation.m_timestamp_SAS = sd_msg_seamless_estimation_rsp_ptr->timestamp_SAS;
	seamlessEstimation.m_RMS2_FAS = sd_msg_seamless_estimation_rsp_ptr->average_RMS2_FAS;
	seamlessEstimation.m_RMS2_SAS = sd_msg_seamless_estimation_rsp_ptr->average_RMS2_SAS;
	seamlessEstimation.m_confidenceLevel = sd_msg_seamless_estimation_rsp_ptr->confidence_level;

	vl_externalNotificationToBeSent = ETAL_receiverIsSpecialInProgress(seamlessEstimation.m_receiverHandle, cmdSpecialExternalSeamlessEstimationRequestInProgress);
	
	if (true == vl_externalNotificationToBeSent)
	{		
		ETAL_receiverSetSpecial(seamlessEstimation.m_receiverHandle, cmdSpecialExternalSeamlessEstimationRequestInProgress, cmdActionStop);
	}

	// invoke the internal call back
	//
	ETAL_intCbScheduleCallbacks(ETAL_INVALID_HANDLE, callAtSeamlessEstimationResponse, (tVoid *)&seamlessEstimation, sizeof(seamlessEstimation));

	// temporary : for now, this concerns seamless cases only,
	// so this is put here.
	// in the future it should be in callbackInvoke itself that filter is done.
	//
	if (true == vl_externalNotificationToBeSent)
	{
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEAMLESS_ESTIMATION_END, (tVoid *)&seamlessEstimation, sizeof(seamlessEstimation));
	}
	ETAL_receiverStopAllSpecial(cmdSpecialSeamlessEstimation);
}

/***************************
 *
 * ETAL_cmdSeamlessEstimation_DABMW
 *
 **************************/
tSInt ETAL_cmdSeamlessEstimation_DABMW(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamlessEstimationConfig)
{
	tSInt res;
	SD_msg_SeamlessEstimationRequest dabmw_msg_seamless_estimation_request;

	dabmw_msg_seamless_estimation_request.msg_id                    = SD_MESSAGE_RX_SEAMLESS_ESTIMATION;
	dabmw_msg_seamless_estimation_request.mode                      = seamlessEstimationConfig->mode;
	dabmw_msg_seamless_estimation_request.start_position_in_samples = seamlessEstimationConfig->startPosition;
	dabmw_msg_seamless_estimation_request.stop_position_in_samples  = seamlessEstimationConfig->stopPosition;
	dabmw_msg_seamless_estimation_request.downSampling              = (tU8)8;
	dabmw_msg_seamless_estimation_request.loudness_duration         = (tU8)3;

	res = DABMW_StreamDecoderApiSendMessage((tPU8)&dabmw_msg_seamless_estimation_request, (tU32)sizeof(SD_msg_SeamlessEstimationRequest));
	return res;
}

/***************************
 *
 * DABMW_SeamlessSwitchingResponse
 *
 **************************/
tVoid DABMW_SeamlessSwitchingResponse (tU8 msgNum, tPU8 msgPayload)
{
	SD_msg_SeamlessSwitchingResponse *dabmw_msg_switching_report = (SD_msg_SeamlessSwitchingResponse *)(msgPayload-1);
	EtalSeamlessSwitchingStatus seamlessSwitching;
	tBool vl_externalNotificationToBeSent = true;

	/* Protection to avoid having SeamlessSwitchingResponse event before etal_seamless_switching return the ETAL_STATUS */
	if (ETAL_statusGetLock() != OSAL_OK)
	{
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_APP_DABMW, "DABMW_SeamlessSwitchingResponse ETAL_statusGetLock error");
		return;
	}

	/* do the same than ETAL_invokeEventCallback_MDR(DABMWrsp_SeamlessSwitching, sizeof(DABMWrsp_SeamlessSwitching)); */

	seamlessSwitching.m_receiverHandle = ETAL_receiverSearchActiveSpecial(cmdSpecialSeamlessSwitching);
	if (seamlessSwitching.m_receiverHandle == ETAL_INVALID_HANDLE)
	{
#if defined(CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_ERRORS)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_APP_DABMW, "Unhandled event SeamlessSwitchingResponse");
#endif
		return;
	}
	seamlessSwitching.m_absoluteDelayEstimate = dabmw_msg_switching_report->delay;
	seamlessSwitching.m_status = dabmw_msg_switching_report->status;

	vl_externalNotificationToBeSent = ETAL_receiverIsSpecialInProgress(seamlessSwitching.m_receiverHandle, cmdSpecialExternalSeamlessSwitchingRequestInProgress);

	if (true == vl_externalNotificationToBeSent)
	{		
		ETAL_receiverSetSpecial(seamlessSwitching.m_receiverHandle, cmdSpecialExternalSeamlessSwitchingRequestInProgress, cmdActionStop);
	}

	// invoke the internal call back
	//
	ETAL_intCbScheduleCallbacks(ETAL_INVALID_HANDLE, callAtSeamlessSwitchingResponse, (tVoid *)&seamlessSwitching, sizeof(seamlessSwitching));

	// temporary : for now, this concerns seamless cases only,
	// so this is put here.
	// in the future it should be in callbackInvoke itself that filter is done.
	//
	if (true == vl_externalNotificationToBeSent)
	{
		ETAL_callbackInvoke(ETAL_COMM_EVENT_CALLBACK_HANDLER, cbTypeEvent, ETAL_INFO_SEAMLESS_SWITCHING_END, (tVoid *)&seamlessSwitching, sizeof(seamlessSwitching));
	}
	ETAL_receiverStopAllSpecial(cmdSpecialSeamlessSwitching);

	ETAL_statusReleaseLock();
}

/***************************
 *
 * ETAL_cmdSeamlessSwitching_DABMW
 *
 **************************/
tSInt ETAL_cmdSeamlessSwitching_DABMW(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamlessSwitchingConfig)
{
	tSInt res;
	SD_msg_SeamlessSwitchingRequest dabmw_msg_seamless_switching_request;

	dabmw_msg_seamless_switching_request.msg_id                = SD_MESSAGE_RX_SEAMLESS_SWITCHING;
	dabmw_msg_seamless_switching_request.systemToSwitch        = seamlessSwitchingConfig->systemToSwitch;
	dabmw_msg_seamless_switching_request.provider_type         = seamlessSwitchingConfig->providerType;
	dabmw_msg_seamless_switching_request.confirmationRequested = 0;
	dabmw_msg_seamless_switching_request.absolute_estimated_delay_in_samples = seamlessSwitchingConfig->absoluteDelayEstimate;
	dabmw_msg_seamless_switching_request.delay_estimate        = seamlessSwitchingConfig->delayEstimate;
	dabmw_msg_seamless_switching_request.timestamp_FAS         = seamlessSwitchingConfig->timestampFAS;
	dabmw_msg_seamless_switching_request.timestamp_SAS         = seamlessSwitchingConfig->timestampSAS;
	dabmw_msg_seamless_switching_request.average_RMS2_FAS      = seamlessSwitchingConfig->averageRMS2FAS;
	dabmw_msg_seamless_switching_request.average_RMS2_SAS      = seamlessSwitchingConfig->averageRMS2SAS;

	res = DABMW_StreamDecoderApiSendMessage((tPU8)&dabmw_msg_seamless_switching_request, (tU32)sizeof(SD_msg_SeamlessSwitchingRequest));
	return res;
}
#endif // #if defined(CONFIG_APP_SEAMLESS_SWITCHING_BLOCK) && defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)

#if defined(CONFIG_TARGET_DABMW_STREAMDECODER_COMM_ENABLE)
tSInt DABMW_DRCinfoStatusSet (tU8 msgNum, tPU8 msgPayload)
{
    int i = 0;

	(tVoid) msgNum;   // lint

#if defined (CONFIG_TARGET_DABMW_STREAMDECODER_COMM_ENABLE)      
    DABMW_DRCinfoStatus = true;

    for (i = 0; i < 2; i++)
    {
        DABMW_DRCinfoBuffer[i] = (tU8)*(msgPayload+i);
    }
#else
    (void)DABMW_DRCinfoStatus; //make lint happy
    (void)DABMW_DRCinfoBuffer[0]; //make lint happy
#endif //#if defined (CONFIG_TARGET_DABMW_STREAMDECODER_COMM_ENABLE)     

    return i;
}
#endif

#ifdef __cplusplus
}
#endif

// END OF FILE

