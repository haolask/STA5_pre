//!
//!  \file 		etalapi_quality.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, quality functions
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

/***************************
 * Defines
 **************************/

/***************************
 * Local functions
 **************************/
static tBool ETAL_get_CF_data_check(EtalCFDataContainer* pResp, tU32 nbOfAverage, tU32 period);

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_isMonitoredIndicatorCompatible
 *
 **************************/
static tBool ETAL_isMonitoredIndicatorCompatible(EtalBcastQaIndicators ind, EtalBcastStandard std)
{
	switch (ind)
	{
		case EtalQualityIndicator_Undef:
			return TRUE;

		case EtalQualityIndicator_DabFicErrorRatio:
		case EtalQualityIndicator_DabFieldStrength:
		case EtalQualityIndicator_DabMscBer:
		case EtalQualityIndicator_DabAudioBerLevel:
			if (std == ETAL_BCAST_STD_DAB)
			{
				return TRUE;
			}
			break;

		case EtalQualityIndicator_FmFieldStrength:
		case EtalQualityIndicator_FmFrequencyOffset:
		case EtalQualityIndicator_FmModulationDetector:
		case EtalQualityIndicator_FmMultipath:
		case EtalQualityIndicator_FmUltrasonicNoise:
			if ((std == ETAL_BCAST_STD_FM) ||
				(std == ETAL_BCAST_STD_AM) ||
				(ETAL_IS_HDRADIO_STANDARD(std)))
			{
				return TRUE;
			}
			break;

		case EtalQualityIndicator_HdQI:
		case EtalQualityIndicator_HdCdToNo:
		case EtalQualityIndicator_HdDSQM:
			if (ETAL_IS_HDRADIO_STANDARD(std))
			{
				return TRUE;
			}
			break;

		default:
			break;
	}
	return FALSE;
}

/***************************
 *
 * ETAL_checkQualityMonitor
 *
 **************************/
static ETAL_STATUS ETAL_checkQualityMonitor(const EtalBcastQualityMonitorAttr *q)
{
	tU32 i;
	EtalBcastStandard std_recv;
	EtalBcastQaIndicators indicator;

	if (q == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	if (!ETAL_receiverIsValidHandle(q->m_receiverHandle))
	{
		return ETAL_RET_INVALID_HANDLE;
	}
	if (q->m_CbBcastQualityProcess == NULL)
	{
		return ETAL_RET_QUAL_CONTAINER_ERR;
	}
	if (!ETAL_statusIsValidMonitor(q))
	{
		return ETAL_RET_QUAL_CONTAINER_ERR;
	}

	/*
	 * check that all the monitors' requested indicators
	 * are compatible with the receiver standard
	 */
	std_recv = ETAL_receiverGetStandard(q->m_receiverHandle);
	for (i = 0; i < ETAL_MAX_QUALITY_PER_MONITOR; i++)
	{
		indicator = q->m_monitoredIndicators[i].m_MonitoredIndicator;
		if (ETAL_isMonitoredIndicatorCompatible(indicator, std_recv))
		{
			continue;
		}
		return ETAL_RET_QUAL_CONTAINER_ERR;
	}

	return ETAL_RET_SUCCESS;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * ETAL_get_CF_data_check
 *
 **************************/
static tBool ETAL_get_CF_data_check(EtalCFDataContainer* pResp, tU32 nbOfAverage, tU32 period)
{
	tBool status = TRUE;

	if(pResp == NULL)
	{
		status = FALSE;
	}

	if(nbOfAverage > 10)
	{
		status = FALSE;
	}

	if(period > 100)
	{
		status = FALSE;
	}
	return status;
}

/***************************
 *
 * ETAL_get_reception_quality
 *
 **************************/
ETAL_STATUS ETAL_get_reception_quality_internal(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
	tU32 device_list;

	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (pBcastQuality == NULL)
	{
		ret = ETAL_RET_QUAL_CONTAINER_ERR;
	}
	else if (!ETAL_receiverSupportsQuality(hReceiver))
	{
		ret = ETAL_RET_QUAL_FE_ERR;
	}
	else
	{
		ETAL_resetQualityContainer(ETAL_receiverGetStandard(hReceiver), pBcastQuality);
		device_list = ETAL_cmdRoutingCheck(hReceiver, commandQuality);

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
		if (ETAL_CMDROUTE_TO_CMOST(device_list))
		{
			if (ETAL_cmdGetReceptionQuality_CMOST(hReceiver, pBcastQuality) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		if (ETAL_CMDROUTE_TO_DCOP(device_list))
		{
			if (ETAL_cmdGetChannelQuality_CMOST(hReceiver, pBcastQuality) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}

			if (ETAL_cmdGetAudioQuality_MDR(hReceiver, pBcastQuality) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		if (ETAL_CMDROUTE_TO_DCOP(device_list))
		{
			if (ETAL_cmdGetQuality_HDRADIO(hReceiver, pBcastQuality) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
#endif
	}

#else
	/* DOT does not support quality */
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
	return ret;
}


#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
/***************************
 *
 * ETAL_get_channel_quality_internal
 *
 **************************/
ETAL_STATUS ETAL_get_channel_quality_internal(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (pBcastQuality == NULL)
	{
		ret = ETAL_RET_QUAL_CONTAINER_ERR;
	}
	else if (!ETAL_receiverSupportsQuality(hReceiver))
	{
		ret = ETAL_RET_QUAL_FE_ERR;
	}
	else
	{
		std = ETAL_receiverGetStandard(hReceiver);
		ETAL_resetQualityContainer(std, pBcastQuality);

		if((std == ETAL_BCAST_STD_FM) || (std == ETAL_BCAST_STD_AM) ||
		   (std == ETAL_BCAST_STD_HD_FM) || (std == ETAL_BCAST_STD_HD_AM))
		{
			if (ETAL_cmdGetChannelQuality_CMOST(hReceiver, pBcastQuality) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
		else
		{
			ret = ETAL_RET_INVALID_BCAST_STANDARD;
		}
	}
	return ret;
}
#endif

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_get_reception_quality
 *
 **************************/
ETAL_STATUS etal_get_reception_quality(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_reception_quality(rec: %d)", hReceiver);

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_get_reception_quality_internal(hReceiver, pBcastQuality);

		ETAL_receiverReleaseLock(hReceiver);
	}
#else
	/* DOT does not support quality */
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

	if(pBcastQuality != NULL)
	{
		ETAL_tracePrintQuality(TR_LEVEL_SYSTEM, pBcastQuality);
	}
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_reception_quality() = %s", ETAL_STATUS_toString(ret));
	return ret;
}


#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_config_reception_quality_monitor
 *
 **************************/
ETAL_STATUS etal_config_reception_quality_monitor(ETAL_HANDLE* pMonitor, const EtalBcastQualityMonitorAttr* pMonitorAttr)
{
	ETAL_HANDLE hReceiver;
	ETAL_STATUS ret;
	tSInt retosal = OSAL_OK;
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	tU32 device_list;
#endif
	tU32 vl_count;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_config_reception_quality_monitor(pMon: 0x%x, pMonAttr 0x%x)",
    		(pMonitor == NULL) ? 0 : *pMonitor,
    		(pMonitorAttr == NULL) ? 0 : pMonitorAttr);
	
	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_statusIsValidMonitorHandle(pMonitor))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else
		{
		/* log parameters */
			if (pMonitorAttr != NULL) 
			{
				for (vl_count=0; vl_count < ETAL_MAX_QUALITY_PER_MONITOR; vl_count++)
				{
					if (EtalQualityIndicator_Undef != pMonitorAttr->m_monitoredIndicators[vl_count].m_MonitoredIndicator)
					{
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "rec: %d, Index[%d] => monInd: %d, infVal: %d, supVal: %d, updFreq: %d",
								pMonitorAttr->m_receiverHandle,
								vl_count, 
								pMonitorAttr->m_monitoredIndicators[vl_count].m_MonitoredIndicator,
								pMonitorAttr->m_monitoredIndicators[vl_count].m_InferiorValue,
								pMonitorAttr->m_monitoredIndicators[vl_count].m_SuperiorValue,
								pMonitorAttr->m_monitoredIndicators[vl_count].m_UpdateFrequency);
					}
				}
			}
			ret = ETAL_checkQualityMonitor(pMonitorAttr);
			if (ret == ETAL_RET_SUCCESS)
			{
				hReceiver = pMonitorAttr->m_receiverHandle;
				if (!ETAL_receiverSupportsQuality(hReceiver))
				{
					ret = ETAL_RET_QUAL_FE_ERR;
				}
				else
				{
					retosal = ETAL_statusCreateModifyMonitor(pMonitor, pMonitorAttr);
					if (retosal == OSAL_OK)
					{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
						device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);
						if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
							(ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_DAB))
						{
							retosal = ETAL_cmdStartMonitor_MDR(*pMonitor);
						}
#endif
						/*
						 * nothing to do for HD or STAR since monitors are managed from
						 * ETAL_controlPollQuality which is always running
						 */
					}
				}
			}
		}
		ETAL_statusReleaseLock();
	}
	if (retosal != OSAL_OK)
	{
		ret = ETAL_RET_ERROR;
	}

	if ((ETAL_RET_SUCCESS == ret) || (pMonitor != NULL))
	{
	    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_config_reception_quality_monitor(monHan: %d) = %s", *pMonitor, ETAL_STATUS_toString(ret));
	}
	else
	{
		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_config_reception_quality_monitor() = %s", ETAL_STATUS_toString(ret));
	}
	
	return ret;
}

/***************************
 *
 * etal_destroy_reception_quality_monitor
 *
 **************************/
ETAL_STATUS etal_destroy_reception_quality_monitor(ETAL_HANDLE *pMonitor)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if(pMonitor != NULL)
	{
    	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_destroy_reception_quality_monitor(pMon: %d)", *pMonitor);
	}
    else
    {
    	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_destroy_reception_quality_monitor(pMon: NULL)");
    }

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (pMonitor == NULL)
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else
		{
			if ((!ETAL_statusIsValidMonitorHandle(pMonitor)) ||
				(*pMonitor == ETAL_INVALID_HANDLE)) // ETAL_statusIsValidMonitorHandle considers this valid
			{
				ret = ETAL_RET_INVALID_HANDLE;
			}
			else if (ETAL_statusDestroyMonitor(*pMonitor) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
			else
			{
				/* Nothing to do */
			}

			*pMonitor = ETAL_INVALID_HANDLE;
		}
		ETAL_statusReleaseLock();
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_destroy_reception_quality_monitor() = %s", ETAL_STATUS_toString(ret));
	return ret;
}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

/***************************
 *
 * etal_get_CF_data
 *
 **************************/
ETAL_STATUS etal_get_CF_data(ETAL_HANDLE hReceiver, EtalCFDataContainer* pResp, tU32 nbOfAverage, tU32 period)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	etalFrequencyBandInfoTy band_info;
	tU32 count;
	EtalBcastQualityContainer qualityResultAvg;
	EtalFmQualityEntries *pRespFmAvg, *pRespFm;
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_CF_data(rec: %d, nbOfAve: %d, per: %d)", hReceiver, nbOfAverage, period);

	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		if (!ETAL_receiverIsValidHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
		{
			ret = ETAL_RET_IN_PROGRESS;
		}
		else if (!ETAL_get_CF_data_check(pResp, nbOfAverage, period))
		{
			ret = ETAL_RET_PARAMETER_ERR;
		}
		else if (!ETAL_receiverSupportsQuality(hReceiver))
		{
			ret = ETAL_RET_QUAL_FE_ERR;
		}
		else
		{
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
			if((ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_FM) ||
			   (ETAL_receiverGetStandard(hReceiver) == ETAL_BCAST_STD_AM) ||
			   ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
			{
				(void)OSAL_pvMemorySet((tVoid *)&qualityResultAvg, 0x00, sizeof(EtalBcastQualityContainer));
				(void)OSAL_pvMemorySet((tVoid *)pResp, 0x00, sizeof(EtalCFDataContainer));

				(LINT_IGNORE_RET) ETAL_receiverGetBandInfo(hReceiver, &band_info);
				pResp->m_CurrentBand = band_info.band;
				pResp->m_CurrentFrequency = ETAL_receiverGetFrequency(hReceiver);

				pRespFm = &(pResp->m_QualityContainer.EtalQualityEntries.amfm);
				pRespFmAvg = &(qualityResultAvg.EtalQualityEntries.amfm);

				count = 0;

				while(count < nbOfAverage)
				{
					if (ETAL_cmdGetReceptionQuality_CMOST(hReceiver, &(pResp->m_QualityContainer)) != OSAL_OK)
					{
						ETAL_receiverReleaseLock(hReceiver);
						ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_CF_data() = %s", ETAL_STATUS_toString(ETAL_RET_ERROR));
						return ETAL_RET_ERROR;
					}
					else
					{
						pRespFmAvg->m_RFFieldStrength 					+= pRespFm->m_RFFieldStrength;
						pRespFmAvg->m_BBFieldStrength 					+= pRespFm->m_BBFieldStrength;
						pRespFmAvg->m_FrequencyOffset 					+= pRespFm->m_FrequencyOffset;
						pRespFmAvg->m_ModulationDetector 				+= pRespFm->m_ModulationDetector;
						pRespFmAvg->m_Multipath 						+= pRespFm->m_Multipath;
						pRespFmAvg->m_UltrasonicNoise 					+= pRespFm->m_UltrasonicNoise;
						pRespFmAvg->m_AdjacentChannel					+= pRespFm->m_AdjacentChannel;
						pRespFmAvg->m_SNR								+= pRespFm->m_SNR;
						pRespFmAvg->m_coChannel							+= pRespFm->m_coChannel;
					}
					count++;

					(void)OSAL_s32ThreadWait(period);
				}
				pRespFmAvg->m_StereoMonoReception = pRespFm->m_StereoMonoReception;

				/* Average measurement results */
				pRespFmAvg->m_RFFieldStrength 					/= (tF32)nbOfAverage;
				pRespFmAvg->m_BBFieldStrength 					/= (tF32)nbOfAverage;
				pRespFmAvg->m_FrequencyOffset 					/= (tF32)nbOfAverage;
				pRespFmAvg->m_ModulationDetector 				/= (tF32)nbOfAverage;
				pRespFmAvg->m_Multipath 						/= (tF32)nbOfAverage;
				pRespFmAvg->m_UltrasonicNoise					/= (tF32)nbOfAverage;
				pRespFmAvg->m_AdjacentChannel					/= (tF32)nbOfAverage;
				pRespFmAvg->m_SNR								/= (tF32)nbOfAverage;
				pRespFmAvg->m_coChannel							/= (tF32)nbOfAverage;


				*pRespFm = *pRespFmAvg;
				ret = ETAL_RET_SUCCESS;
			}
			else
			{
				ret = ETAL_RET_INVALID_BCAST_STANDARD;
			}
#else
			ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
		}
		ETAL_receiverReleaseLock(hReceiver);
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_CF_data() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_get_channel_quality
 *
 **************************/
ETAL_STATUS etal_get_channel_quality(ETAL_HANDLE hReceiver, EtalBcastQualityContainer *pBcastQuality)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_channel_quality(rec: %d)", hReceiver);

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) || defined (CONFIG_ETAL_SUPPORT_DCOP)
	ret = ETAL_receiverGetLock(hReceiver);
	if (ret == ETAL_RET_SUCCESS)
	{
		ret = ETAL_get_channel_quality_internal(hReceiver, pBcastQuality);

		ETAL_receiverReleaseLock(hReceiver);
	}
#endif

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_channel_quality() = %s", ETAL_STATUS_toString(ret));
    if(pBcastQuality != NULL)
    {
        ETAL_tracePrintQuality(TR_LEVEL_SYSTEM, pBcastQuality);
    }
    return ret;
}



