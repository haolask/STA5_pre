//!
//!  \file 		etalapi_advtune.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"
#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
#include "libDABDevice.h"
#include "dabdevadapt.h"
#include "fic_common.h"
#endif

/******************************************************************************
 * Local functions
 *****************************************************************************/

/******************************************************************************
 * Local types
 *****************************************************************************/
#if defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * ETAL_checkServiceSelectParameters
 *
 **************************/
/*!
 * \brief		Checks if the parameters passed to #ETAL_serviceSelectInternal_MDR are valid
 * \details		UEId is considered vaid if it has NULL MSB (Ensemble ID is 16 bit + 8 bit of ECC).
 * 				Parameter *mode* is used to decide how to check the remaining
 * 				parameters.
 * \param[in]	mode - type of operation requested
 * \param[in]	UEId - Unique Ensemble ID
 * \param[in]	service - Serivce identifier (16 bit for audio, 32 bit for data service)
 * \param[in]	sc - Service Component
 * \param[in]	subch - Subchannel
 * \return		OSAL_ERROR - invalid parameter
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_checkServiceSelectParameters(EtalServiceSelectMode mode, tU32 UEId, tU32 service, tSInt sc, tSInt subch)
{
	tSInt ret = OSAL_OK;

	if ((UEId == ETAL_INVALID_UEID) ||
		((UEId & 0xFF000000) != 0)) // Ensemble ID is 16 bit + 8 bit of ECC
	{
		return OSAL_ERROR;
	}

	switch (mode)
	{
		case ETAL_SERVSEL_MODE_SERVICE:
			if (service == ETAL_INVALID_SID) // service ID can be 16 or 32 bits
			{
				ret = OSAL_ERROR;
			}
			break;

		case ETAL_SERVSEL_MODE_DAB_SC:
			if ((service == ETAL_INVALID_SID) ||
				(sc == ETAL_INVALID))
			{
				ret = OSAL_ERROR;
			}
			else if ((sc & 0xFFFFFF00) != 0) // SC is provided by MDR as 8 bits (it is not the internal SC, 12 bits)
			{
				ret = OSAL_ERROR;
			}
			break;

		case ETAL_SERVSEL_MODE_DAB_SUBCH:
			if (subch == ETAL_INVALID)
			{
				ret = OSAL_ERROR;
			}
			else if ((subch & 0xFFFFFF00) != 0) // subch is 8 bits
			{
				ret = OSAL_ERROR;
			}
			break;

		default:
			ret = OSAL_ERROR;
			break;
	}

	return ret;
}

/***************************
 *
 * ETAL_serviceSelectInternal_MDR
 *
 **************************/
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		
 */
static ETAL_STATUS ETAL_serviceSelectInternal_MDR(ETAL_HANDLE hDatapath, ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 UEId, tU32 service, tSInt sc, tSInt subch)
{
	EtalBcastDataType data_type = ETAL_DATA_TYPE_UNDEF;
	ETAL_STATUS ret;
	tSInt retval;
	tBool no_data;
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	etalReceiverStatusTy *recvp;
	EtalServiceInfo serv_info;
	EtalServiceComponentExtendedList sclist;
	tBool have_data;
	tU8 audio_mode;
	tU16 subch_size, i;
	tU8 old_subch;
	EtalSink *psink;
#endif

	ret = ETAL_RET_SUCCESS;

	/*
	 * we use hDatapath and hReceiver to distinguish audio vs non-audio cases
	 */
	if ((hDatapath == ETAL_INVALID_HANDLE) &&
		(hReceiver == ETAL_INVALID_HANDLE))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else if (hDatapath != ETAL_INVALID_HANDLE)
	{
		/*
		 * data service (or audio treated as data)
		 */
		if (hReceiver != ETAL_INVALID_HANDLE)
		{
			/*
			 * hReceiver will be overwritten below, so while this
			 * is a programming error we can accept it at run-time
			 */
			ASSERT_ON_DEBUGGING(0);
		}

		if (((data_type = ETAL_receiverGetDataTypeForDatapath(hDatapath)) == ETAL_DATA_TYPE_UNDEF) ||
			((type != ETAL_SERVSEL_SUBF_APPEND) && (type != ETAL_SERVSEL_SUBF_REMOVE)))
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
			if ((data_type != ETAL_DATA_TYPE_DATA_SERVICE) &&
				(data_type != ETAL_DATA_TYPE_DAB_DATA_RAW) &&
				(data_type != ETAL_DATA_TYPE_DAB_AUDIO_RAW))
			{
				ret = ETAL_RET_INVALID_DATA_TYPE;
			}
			else
			{
				hReceiver = ETAL_receiverGetFromDatapath(hDatapath);
			}
		}
	}
	else // (hDatapath == ETAL_INVALID_HANDLE) && (hReceiver != ETAL_INVALID_HANDLE)
	{
		/* 
		 * audio service
		 */
		if ((type != ETAL_SERVSEL_SUBF_SET) && (type != ETAL_SERVSEL_SUBF_REMOVE))
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
	}

	if (ret == ETAL_RET_SUCCESS)
	{
		if ((ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_DAB) ||
			(ETAL_receiverGetFrequency(hReceiver) == ETAL_INVALID_FREQUENCY))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (ETAL_checkServiceSelectParameters(mode, UEId, service, sc, subch) != OSAL_OK)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
			retval = ETAL_cmdServiceSelect_MDR(hDatapath, hReceiver, mode, type, (tU32)UEId, (tU32)service, (tU32)sc, (tU32)subch, &no_data);
			if (retval != OSAL_OK)
			{
				if ((retval == OSAL_ERROR) && no_data)
				{
					ret = ETAL_RET_NO_DATA;
				}
				else
				{
					ret = ETAL_RET_ERROR;
				}
			}
			else
			{
				if ((mode == ETAL_SERVSEL_MODE_SERVICE) ||
					(mode == ETAL_SERVSEL_MODE_DAB_SC))
				{
					// Correction : if this is an append, it means we already have an active main service
					// so consider that the 1st one is the reference selected one
					//
					if (ETAL_SERVSEL_SUBF_SET == type)
					{
						ETAL_statusSetDABService(UEId, service);
					}
				}
				else
				{
					ETAL_statusSetDABService(ETAL_INVALID_UEID, ETAL_INVALID_SID);
				}
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
				if (data_type == ETAL_DATA_TYPE_DAB_AUDIO_RAW)
				{
					recvp = ETAL_receiverGet(hReceiver);
					if ((recvp != NULL) && (recvp->isValid))
					{
						psink = ETAL_receiverDatapathGetSink(hDatapath);
						if ((psink != OSAL_NULL) && (psink->m_CbProcessBlock == DABDEV_CbDataPath_dab_audio_raw))
						{
							/* is called from etal_service_select_audio with internal datapath callback */

							/* get audio mode and subchId */
							if (ETAL_cmdGetSpecificServiceDataExtended_MDR(hReceiver, UEId, service, &serv_info, &sclist, &have_data) != OSAL_OK)
							{
								ret = ETAL_RET_ERROR;
							}
							if (ret == ETAL_RET_SUCCESS)
							{
								audio_mode = ETAL_MSC_MODE_UNDEFINED;
								if (serv_info.m_streamType == DABMW_COMPONENTTYPE_IS_MSC_STREAM_AUDIO)
								{
									if (serv_info.m_componentType == ETAL_ASCTY_DAB_PLUS)
									{
										audio_mode = ETAL_MSC_MODE_DAB_PLUS;
									}
									else
									{
										audio_mode = ETAL_MSC_MODE_DAB;
									}
								}
								else if (serv_info.m_streamType == DABMW_COMPONENTTYPE_IS_MSC_STREAM_DATA)
								{
									if (serv_info.m_componentType == ETAL_DSCTY_MPEG2TS)
									{
										audio_mode = ETAL_MSC_MODE_DMB;
									}
									else
									{
										audio_mode = ETAL_MSC_MODE_DATA;
									}
								}
								else if (serv_info.m_streamType == DABMW_COMPONENTTYPE_IS_MSC_PACKET_DATA)
								{
									audio_mode = ETAL_MSC_MODE_PACKET;
								}
								if (mode == ETAL_SERVSEL_MODE_SERVICE)
								{
									/* use the primary service in case of service mode */
									subch = serv_info.m_subchId;
								}
								else if (mode == ETAL_SERVSEL_MODE_DAB_SC)
								{
									/* find the subch corresponding to the sc index */
									for(i = 0; i < sclist.m_scCount; i++)
									{
										if (sclist.m_scInfo[i].m_scIndex == sc)
										{
											subch = sclist.m_scInfo[i].m_subchId;
										}
									}
								}
								else if (mode == ETAL_SERVSEL_MODE_DAB_SUBCH)
								{
									/* subch already valid */
								}
								/* this subch_size may not be the correct value */
								/* m_serviceBitrate / 8 to have kbyte/s, multipied by 24 bytes per 24 ms for 1 kbyte/s */
								subch_size = ((serv_info.m_serviceBitrate / 8) * 24);
								if (audio_mode == ETAL_MSC_MODE_DAB_PLUS)
								{
									/* DCOP sends 5 frames == superframe in dabplus */
									subch_size *= 5;
								}
								/* enable/disable audio stream */
								if (type == ETAL_SERVSEL_SUBF_REMOVE)
								{
									if (add_api_ms_notify_channel_selection(recvp->MDRConfig.application, INTERFACEREF_AUD_OUT, 0, audio_mode, subch, 0) != OSAL_OK)
									{
										ret = ETAL_RET_ERROR;
									}
									if (ETAL_receiverClearSubch_MDR(hReceiver) != OSAL_OK)
									{
										ret = ETAL_RET_ERROR;
									}
									ETAL_statusSetDABService(ETAL_INVALID_UEID, ETAL_INVALID_SID);
								}
								else if (type == ETAL_SERVSEL_SUBF_APPEND)
								{
									/* is second append of ETAL_DATA_TYPE_DAB_AUDIO_RAW ? */
									if (ETAL_receiverGetSubch_MDR(hReceiver, &old_subch) == OSAL_OK)
									{
										if ((old_subch != ETAL_INVALID_SUBCH_ID) && (old_subch != subch))
										{
											/* remove old ETAL_DATA_TYPE_DAB_AUDIO_RAW because only 1 possible for a receiver */
											retval = ETAL_cmdServiceSelect_MDR(hDatapath, hReceiver, ETAL_SERVSEL_MODE_DAB_SUBCH, ETAL_SERVSEL_SUBF_REMOVE, (tU32)UEId, (tU32)service, (tU32)sc, (tU32)old_subch, &no_data);
											if (retval != OSAL_OK)
											{
												if ((retval == OSAL_ERROR) && no_data)
												{
													ret = ETAL_RET_NO_DATA;
												}
												else
												{
													ret = ETAL_RET_ERROR;
												}
											}
										}
									}
									if (add_api_ms_notify_channel_selection(recvp->MDRConfig.application, INTERFACEREF_AUD_OUT, 1, audio_mode, subch, subch_size) != OSAL_OK)
									{
										ret = ETAL_RET_ERROR;
									}
									if (ETAL_receiverSetSubch_MDR(hReceiver, subch) != OSAL_OK)
									{
										ret = ETAL_RET_ERROR;
									}
									if (retval == OSAL_OK)
									{
										/* save main audio service */
										ETAL_statusSetDABService(UEId, service);
									}
								}
								else
								{
									ret = ETAL_RET_PARAMETER_ERR;
								}
							}
						}
					}
					else
					{
						ret = ETAL_RET_PARAMETER_ERR;
					}
				}
#endif
			}
		}
	}
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
#endif // CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
/***************************
 *
 * ETAL_serviceSelectInternal_HDRADIO
 *
 **************************/
/*!
 * \brief		Performs a service select operation for HDRadio
 * \param[in]	hReceiver - the HDRadio Receiver handle
 * \param[in]	service -  valid service number range 0 to #ETAL_HD_MAX_PROGRAM_NUM
 * 				           included
 * \return		#ETAL_RET_INVALID_RECEIVER - receiver not configured, or not
 * 				                            for HDRadio
 * \return		#ETAL_RET_PARAMETER_ERR - invalid *service* parameter
 * \return		#ETAL_RET_NO_DATA - no HDRadio contents on the tuned frequency
 * \return		#ETAL_RET_ERROR - communication error with the device
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
static ETAL_STATUS ETAL_serviceSelectInternal_HDRADIO(ETAL_HANDLE hReceiver, tU32 service)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (!ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)) ||
			(ETAL_receiverGetFrequency(hReceiver) == ETAL_INVALID_FREQUENCY))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if ((service == ETAL_INVALID_SID) || (service >= ETAL_HD_MAX_PROGRAM_NUM))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}	
	else
	{
		if (!ETAL_receiverHasService_HDRADIO(hReceiver, (tS8)service))
		{
			ret = ETAL_RET_NO_DATA;
		}
		else if (!ETAL_receiverHasDigitalAudio_HDRADIO(hReceiver))
		{
			ret = ETAL_RET_NO_DATA;
		}
		else if (ETAL_receiverSelectProgram_HDRADIO(hReceiver, (tS8) service) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

/******************************************************************************
 * Exported functions
 *****************************************************************************/

#if defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * etal_get_current_ensemble
 *
 **************************/
/*!
 * \brief		Returns the current Unique Ensemble ID for a DAB Receiver
 * \param[in]	hReceiver - handle of a DAB Receiver
 * \param[out]	pUEId - pointer to location where the function stores the UEId.
 * 				        This location will not be changed if the function
 * 				        returns an error.
 * \return		#ETAL_RET_INVALID_HANDLE - the *hReceiver* parameter is not fornally correct
 * \return		#ETAL_RET_INVALID_RECEIVER - the Receiver is not configured,
 * 				                            or not configured for DAB
 * \return		#ETAL_RET_ERROR - communication or semaphore access error
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_get_current_ensemble(ETAL_HANDLE hReceiver, tU32 *pUEId)
{
	ETAL_STATUS ret;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_current_ensemble(rec: %d, pUEId: %p)", 
					hReceiver, pUEId);
	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_DAB)
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (pUEId == NULL)
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else
		{
			if (ETAL_cmdGetCurrentEnsemble_MDR(hReceiver, pUEId) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}

		ETAL_receiverReleaseLock(hReceiver);
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_current_ensemble(%spUEId: 0x%x) = %s", 
		(pUEId != NULL)?"*":"", (pUEId != NULL)?*pUEId:0, ETAL_STATUS_toString(ret));
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP
/***************************
 *
 * etal_service_select_audio
 *
 **************************/
/*!
 * \brief		Selects an audio service for HDRadio or DAB Receiver
 * \param[in]	hReceiver - handle of HDRadio or DAB Receiver
 * \param[in]	mode - type of operation requested. For HDRadio receivers only
 * 				       #ETAL_SERVSEL_MODE_SERVICE is allowed.
 * \param[in]	UEId - Unique Ensemble ID
 * \param[in]	service - Serivce identifier (16 bit for audio, 32 bit for data service)
 * \param[in]	sc - Service Component
 * \param[in]	subch - Subchannel
 * \return		#ETAL_RET_INVALID_HANDLE - the *hReceiver* parameter is not fornally correct
 * \return		#ETAL_RET_INVALID_RECEIVER - the Receiver is not configured,
 * 				                            or not configured for DAB or HDRadio
 * \return		#ETAL_RET_PARAMETER_ERR - at least one invalid parameter
 * \return		#ETAL_RET_NO_DATA - no HDRadio or DAB contents on the tuned frequency
 * \return		#ETAL_RET_ERROR - communication or semaphore access error
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_service_select_audio(ETAL_HANDLE hReceiver, EtalServiceSelectMode mode, tU32 UEId, tU32 service, tSInt sc, tSInt subch)
{
	ETAL_STATUS ret;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	EtalServiceSelectionStatus vl_ServiceSelectStatus;
#endif
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	tBool vl_serviceFollowingHasBeenDisabled;
#endif
#if defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
	ETAL_HANDLE hDatapath;
	EtalDataPathAttr datapathAttr;
	tU8 datapath_already_used;
#endif

	ret = ETAL_RET_SUCCESS;


	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_service_select_audio(rec: %d, mode: %d, eid: 0x%x, sid: 0x%x, sc:%d, subch:%d)", 
					hReceiver, mode, UEId, service, sc, subch);

	// Disable Service Following if needed
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	vl_serviceFollowingHasBeenDisabled = FALSE;

	if (TRUE == DABWM_ServiceFollowing_ExtInt_IsEnable())
	{
		if (DABMW_ServiceFollowing_ExtInt_DisableSF() == OSAL_OK)
		{
			vl_serviceFollowingHasBeenDisabled = TRUE;
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}
	}
#endif

	switch (ETAL_receiverGetStandard(hReceiver))
	{
		case ETAL_BCAST_STD_DAB:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			// DAB status stored in the global status, not the receiver
			if (ETAL_statusGetLock() != OSAL_OK)
			{
				ret = ETAL_RET_NOT_INITIALIZED;
			}
			else
			{
				if (ret == ETAL_RET_SUCCESS)
				{
					ret = ETAL_receiverGetLock(hReceiver);
					if (ret == ETAL_RET_SUCCESS)
					{
						ETAL_statusClearDABTuneStatus(FALSE);
#if !defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
						ret = ETAL_serviceSelectInternal_MDR(ETAL_INVALID_HANDLE, hReceiver, mode, ETAL_SERVSEL_SUBF_SET, UEId, service, sc, subch);
#else
						datapath_already_used = 0;
						// Create DAB AUDIO_RAW datapath
						hDatapath = ETAL_INVALID_HANDLE;
						OSAL_pvMemorySet(&datapathAttr, 0, sizeof(datapathAttr));
						datapathAttr.m_receiverHandle = hReceiver;
						datapathAttr.m_dataType = ETAL_DATA_TYPE_DAB_AUDIO_RAW;
						datapathAttr.m_sink.m_context = (tPVoid)(tU32)hReceiver;
						datapathAttr.m_sink.m_BufferSize = 1;
						datapathAttr.m_sink.m_CbProcessBlock = DABDEV_CbDataPath_dab_audio_raw;
						ret = ETAL_configDatapathInternal(&hDatapath, &datapathAttr);
						if (ret == ETAL_RET_ALREADY_USED)
						{
							hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_DAB_AUDIO_RAW);
							if (hDatapath != ETAL_INVALID_HANDLE)
							{
								datapath_already_used = 1;
								ret = ETAL_RET_SUCCESS;
							}
						}
						if (ret  == ETAL_RET_SUCCESS)
						{
							ret = ETAL_serviceSelectInternal_MDR(hDatapath, ETAL_INVALID_HANDLE, mode, ETAL_SERVSEL_SUBF_APPEND, UEId, service, sc, subch);
						}
						if (ret == ETAL_RET_SUCCESS)
						{
							// TODO store datapath in receiver status data.
						}
						else
						{
							// destroy hDatapath when service select is failing only if new datapath
							if ((datapath_already_used == 0) && (hDatapath != ETAL_INVALID_HANDLE))
							{
								(void) ETAL_destroyDatapathInternal(&hDatapath);
							}
						}
#endif

						ETAL_receiverReleaseLock(hReceiver);
						
						if (ret == ETAL_RET_SUCCESS)
						{
							// external request for Tune : call the registered callback
							// note : it should be only in external case .. 
							// 
							vl_ServiceSelectStatus.m_receiverHandle = hReceiver;
							vl_ServiceSelectStatus.m_Frequency = ETAL_receiverGetFrequency(hReceiver);
							vl_ServiceSelectStatus.m_Ueid = (tU32) UEId; // validated by ETAL_serviceSelectInternal_MDR so the cast is legal
							vl_ServiceSelectStatus.m_Sid = (tU32)service;
							vl_ServiceSelectStatus.m_mode = mode;
#if !defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
							vl_ServiceSelectStatus.m_subFunction = ETAL_SERVSEL_SUBF_SET;
#else
							vl_ServiceSelectStatus.m_subFunction = ETAL_SERVSEL_SUBF_APPEND;
#endif
						
							ETAL_intCbScheduleCallbacks(hReceiver, callAtServiceSelection, (tVoid *)&vl_ServiceSelectStatus, sizeof(vl_ServiceSelectStatus));
						}
					}
				}
				ETAL_statusReleaseLock();
			}
#endif
			break;

		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			if (mode != ETAL_SERVSEL_MODE_SERVICE)
			{
				ret = ETAL_RET_PARAMETER_ERR;
			}
			else if ((ret = ETAL_receiverGetLock(hReceiver)) == ETAL_RET_SUCCESS)
			{
				ret = ETAL_serviceSelectInternal_HDRADIO(hReceiver, service);
				ETAL_receiverReleaseLock(hReceiver);
			}
			else
			{
				/* Nothing to do */
			}
#endif
			break;

		default:
			ret = ETAL_RET_INVALID_RECEIVER;
			break;
	}

	// renable Service Following
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	if (TRUE == vl_serviceFollowingHasBeenDisabled)
	{
		if (DABMW_ServiceFollowing_ExtInt_ActivateSF() == OSAL_OK)
		{
			vl_serviceFollowingHasBeenDisabled = FALSE;
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}
	}
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_service_select_audio() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_service_select_data
 *
 **************************/
/*!
 * \brief		Selects a DAB data service 
 * \param[in]	hDatapath - handle of Datapath attached to DAB Receiver
 * \param[in]	mode - type of operation requested
 * \param[in]	type - subfunction type (append, remove, set)
 * \param[in]	UEId - Unique Ensemble ID
 * \param[in]	service - Serivce identifier (16 bit for audio, 32 bit for data service)
 * \param[in]	sc - Service Component
 * \param[in]	subch - Subchannel
 * \return		#ETAL_RET_INVALID_HANDLE - the *hReceiver* parameter is not fornally correct
 * \return		#ETAL_RET_INVALID_RECEIVER - the Receiver is not configured,
 * 				                            or not configured for DAB or HDRadio
 * \return		#ETAL_RET_PARAMETER_ERR - at least one invalid parameter
 * \return		#ETAL_RET_NO_DATA - no HDRadio or DAB contents on the tuned frequency
 * \return		#ETAL_RET_ERROR - communication or semaphore access error
 * \return		#ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_service_select_data(ETAL_HANDLE hDatapath, EtalServiceSelectMode mode, EtalServiceSelectSubFunction type, tU32 UEId, tU32 service, tSInt sc, tSInt subch)
{
    ETAL_HANDLE hReceiver = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)
	EtalServiceSelectionStatus vl_ServiceSelectStatus;
#endif

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_service_select_data(rec: %d: mode: %d, type: %d, eid: 0x%x, sid: 0x%x sc:%d, subch:%d)", 
					hDatapath, mode, type, UEId, service, sc, subch);

	if (!ETAL_receiverDatapathIsValid(hDatapath))
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_service_select_data() = %s", ETAL_STATUS_toString(ETAL_RET_INVALID_HANDLE));
		return ETAL_RET_INVALID_HANDLE;
	}
	else
	{
		hReceiver = ETAL_receiverGetFromDatapath(hDatapath);
		
		switch (ETAL_receiverGetStandard(hReceiver))
		{
			case ETAL_BCAST_STD_DAB:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
				/*
				 * some DAB info is stored in the ETAL status so we need to take
				 * the global lock and the receiver lock
				 */
				if (ETAL_statusGetLock() != OSAL_OK)
				{
					ret = ETAL_RET_NOT_INITIALIZED;
				}
				else
				{
					ret = ETAL_receiverGetLock(hReceiver);
					if (ret == ETAL_RET_SUCCESS)
					{
						ETAL_statusClearDABTuneStatus(FALSE);
						ret = ETAL_serviceSelectInternal_MDR(hDatapath, ETAL_INVALID_HANDLE, mode, type, UEId, service, sc, subch);
						ETAL_receiverReleaseLock(hReceiver);
						if (ret == ETAL_RET_SUCCESS)
						{
							// external request for Tune : call the registered callback
							// note : it should be only in external case .. 
							// 
							vl_ServiceSelectStatus.m_receiverHandle = hReceiver;
							vl_ServiceSelectStatus.m_Frequency = ETAL_receiverGetFrequency(hReceiver);
							vl_ServiceSelectStatus.m_Ueid = (tU32) UEId; // validated by ETAL_serviceSelectInternal_MDR so the cast is legal
							vl_ServiceSelectStatus.m_Sid = (tU32)service;
							vl_ServiceSelectStatus.m_mode = mode;
							vl_ServiceSelectStatus.m_subFunction = type;
						
							ETAL_intCbScheduleCallbacks(hReceiver, callAtServiceSelection, (tVoid *)&vl_ServiceSelectStatus, sizeof(vl_ServiceSelectStatus));
						}
					}
					ETAL_statusReleaseLock();
				}
#endif
				break;

			default:
				ret = ETAL_RET_INVALID_RECEIVER;
				break;
		}
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_service_select_data() = %s", ETAL_STATUS_toString(ret));
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP
#endif // CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API

