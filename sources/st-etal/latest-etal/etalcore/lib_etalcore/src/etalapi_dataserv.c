//!
//!  \file 		etalapi_dataserv.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

/******************************************************************************
 * Local functions
 *****************************************************************************/

/******************************************************************************
 * Local types
 *****************************************************************************/
#if defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/***************************
 *
 * ETAL_dataServiceCheckBitmap
 *
 **************************/
static tSInt ETAL_dataServiceCheckBitmap(tU32 ServiceBitmap)
{
	if ((ServiceBitmap & ETAL_DATASERV_TYPE_UNDEFINED) != 0)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_loopOverServiceBitmap
 *
 **************************/
static EtalDataServiceType ETAL_loopOverServiceBitmap(tU32 *status, tU32 *next_index)
{
	EtalDataServiceType candidate_service;
	tU32 local_status;
	tU32 i;

	local_status = *status;
	for (i = *next_index ; i < sizeof(EtalDataServiceType) * 8; i++)
	{
		candidate_service = (EtalDataServiceType)(1 << i);
		if ((candidate_service & ETAL_DATASERV_TYPE_UNDEFINED) != 0)
		{
			/*
			 * assuming the valid values in EtalDataServiceType
			 * are tightly ordered from smaller to bigger values
			 */
			return ETAL_DATASERV_TYPE_NONE;
		}
		if ((local_status & candidate_service) != 0)
		{
			*status = local_status & ~candidate_service;
			*next_index = i + 1;
			return candidate_service;
		}
	}
	return ETAL_DATASERV_TYPE_NONE;
}

/***************************
 *
 * ETAL_enableDataService
 *
 **************************/
static tSInt ETAL_enableDataService(ETAL_HANDLE hReceiver, EtalDataServiceType service, etalCmdActionTy action, EtalDataServiceParam param)
{
	tSInt ret = OSAL_ERROR;

	switch (service)
	{
		case ETAL_DATASERV_TYPE_EPG_RAW:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdStartStopMOT_MDR(hReceiver, etalMOTServiceEPGraw, action);
#else
			ret = ETAL_cmdStartStopMOT_DABMWoH(hReceiver, etalMOTServiceEPGraw, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_SLS:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdStartStopMOT_MDR(hReceiver, etalMOTServiceSLS, action);
#else
			ret = ETAL_cmdStartStopMOT_DABMWoH(hReceiver, etalMOTServiceSLS, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_SLS_XPAD:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdStartStopMOT_MDR(hReceiver, etalMOTServiceSLSoverXPAD, action);
#else
			ret = ETAL_cmdStartStopMOT_DABMWoH(hReceiver, etalMOTServiceSLSoverXPAD, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_TPEG_RAW:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetTPEGRAW_MDR(hReceiver, action);
#else
			ret = ETAL_cmdGetTPEGRAW_DABMWoH(hReceiver, action);
#endif			
			break;

		case ETAL_DATASERV_TYPE_TPEG_SNI:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetTPEGSNI_MDR(hReceiver, action);
#else
			ret = ETAL_cmdGetTPEGSNI_DABMWoH(hReceiver, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_SLI:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
            ret = ETAL_cmdGetSLI_MDR(hReceiver, action);
#else
            ret = ETAL_cmdGetSLI_DABMWoH(hReceiver, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_EPG_BIN:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetEPGBIN_MDR(hReceiver, action);
#else
			ret = ETAL_cmdGetEPGBIN_DABMWoH(hReceiver, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_EPG_SRV:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetEPGSRV_MDR(hReceiver, action, param.m_ecc, param.m_eid);
#else
			ret = ETAL_cmdGetEPGSRV_DABMWoH(hReceiver, action, param.m_ecc, param.m_eid);
#endif
			break;

		case ETAL_DATASERV_TYPE_EPG_PRG:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetEPGPRG_MDR(hReceiver, action, param.m_ecc, param.m_eid, param.m_sid);
#else
			ret = ETAL_cmdGetEPGPRG_DABMWoH(hReceiver, action, param.m_ecc, param.m_eid, param.m_sid);
#endif
			break;

		case ETAL_DATASERV_TYPE_EPG_LOGO:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetEPGLogo_MDR(hReceiver, action, param.m_ecc, param.m_eid, param.m_sid, param.m_logoType);
#else
			ret = ETAL_cmdGetEPGLogo_DABMWoH(hReceiver, action, param.m_ecc, param.m_eid, param.m_sid, param.m_logoType);
#endif

			break;

		case ETAL_DATASERV_TYPE_JML_OBJ:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetJMLOBJ_MDR(hReceiver, action, param.m_JMLObjectId);
#else
			ret = ETAL_cmdGetJMLOBJ_DABMWoH(hReceiver, action, param.m_JMLObjectId);
#endif

			break;

		case ETAL_DATASERV_TYPE_DLS:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetPADDLSEvent_MDR(hReceiver, action);
#else
			ret = ETAL_cmdGetPADDLSevent_DABMWoH(hReceiver, action);
#endif
			break;

		case ETAL_DATASERV_TYPE_DLPLUS:
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
			ret = ETAL_cmdGetPADDLPlusEvent_MDR(hReceiver, action);
#else
			ret = ETAL_cmdGetPADDLPlusEvent_DABMWoH(hReceiver, action);
#endif

			if ((ret == OSAL_OK) && (action == cmdActionStart))
			{
				ret = ETAL_cmdSetPADDLPlusEvent_MDR(hReceiver);
#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST
				ret = ETAL_cmdSetPADDLPlusEvent_MDR(hReceiver);
#else
				ret = ETAL_cmdSetPADDLPlusEvent_DABMWoH(hReceiver);
#endif

			}
			break;

		case ETAL_DATASERV_TYPE_UNDEFINED:
			ASSERT_ON_DEBUGGING(0);
			break;

		case ETAL_DATASERV_TYPE_FIDC:
		case ETAL_DATASERV_TYPE_TMC:
		case ETAL_DATASERV_TYPE_PSD:
		case ETAL_DATASERV_TYPE_NONE:
		case ETAL_DATASERV_TYPE_ALL:
			break;
	}
	return ret;
}

/***************************
 *
 * ETAL_processDataServiceRequest
 *
 **************************/
static ETAL_STATUS ETAL_processDataServiceRequest(ETAL_HANDLE hReceiver, tU32 ServiceBitmap, tU32 *EnabledServiceBitmap, etalCmdActionTy action, EtalDataServiceParam param)
{
	tU32 service_bitmap_status;
	EtalDataServiceType current_service;
	ETAL_STATUS ret;
	tU32 next_index;

	ret = ETAL_RET_SUCCESS;

	if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if ((ETAL_dataServiceCheckBitmap(ServiceBitmap) != OSAL_OK) || 
		(ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_DAB))
	{
		/*
		 * data services are only supported in DAB
		 */
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		ret = ETAL_receiverGetLock(hReceiver);
		if (ret == ETAL_RET_SUCCESS)
		{
			if (ServiceBitmap == ETAL_DATASERV_TYPE_NONE)
			{
				/*
				 * special case, just return the EnabledServiceBitmap
				 * nothing to do here
				 */
			}
			else
			{
				/*
				 * loop over the services specified in the ServiceBitmap and enable or disable them
				 * one by one
				 */
				service_bitmap_status = ServiceBitmap;
				next_index = 0;
				while ((current_service = ETAL_loopOverServiceBitmap(&service_bitmap_status, &next_index)) != ETAL_DATASERV_TYPE_NONE)
				{
					if (ETAL_enableDataService(hReceiver, current_service, action, param) == OSAL_OK)
					{
						if (action == cmdActionStart)
						{
							(LINT_IGNORE_RET)ETAL_receiverSetDataService_MDR(hReceiver, current_service);
						}
						else
						{
							(LINT_IGNORE_RET)ETAL_receiverClearDataService_MDR(hReceiver, current_service);
						}
					}
					else
					{
						ret = ETAL_RET_ERROR;
					}
				}
			}
			if (EnabledServiceBitmap)
			{
				(LINT_IGNORE_RET)ETAL_receiverGetDataServices_MDR(hReceiver, EnabledServiceBitmap);
			}
			ETAL_receiverReleaseLock(hReceiver);
		}
	}
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
/***************************
 *
 * etal_setup_PSD
 *
 **************************/
ETAL_STATUS etal_setup_PSD (ETAL_HANDLE hReceiver, tU16 PSDServiceEnableBitmap, EtalPSDLength* pConfigLenSet, EtalPSDLength* pConfigLenGet)
{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	tSInt retval;
#endif // #if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

	if (!ETAL_handleIsValid(hReceiver))
	{
		return ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if (!ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		/* Nothing to do */
	}

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	// Ensure that Reserved bits are set to 0
	PSDServiceEnableBitmap = PSDServiceEnableBitmap & 0x00FF;

	retval = ETAL_cmdPSDCnfgFieldsEn_HDRADIO(hReceiver, &PSDServiceEnableBitmap);
	if (OSAL_OK != retval)
	{
		return ETAL_RET_ERROR;
	}

	if (NULL != pConfigLenSet)
	{
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_TITLE_LENGTH, pConfigLenSet->m_PSDTitleLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_ARTIST_LENGTH, pConfigLenSet->m_PSDArtistLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_ALBUM_LENGTH, pConfigLenSet->m_PSDAlbumLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_GENRE_LENGTH, pConfigLenSet->m_PSDGenreLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_COMMENT_SHORT_CONTENT_DESCRIPTION_LENGTH, pConfigLenSet->m_PSDCommentShortLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_COMMENT_ACTUAL_TEXT_LENGTH, pConfigLenSet->m_PSDCommentLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_UFID_OWNER_IDENTIFIER_LENGTH, pConfigLenSet->m_PSDUFIDLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_COMMERCIAL_PRICE_STRING_LENGTH, pConfigLenSet->m_PSDCommercialPriceLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_COMMERCIAL_CONTACT_URL_LENGTH, pConfigLenSet->m_PSDCommercialContactLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_COMMERCIAL_NAME_OF_SELLER_LENGTH, pConfigLenSet->m_PSDCommercialSellerLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_COMMERCIAL_DESCRIPTION_LENGTH, pConfigLenSet->m_PSDCommercialDescriptionLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLen_HDRADIO(hReceiver, PSD_XHDR_LENGTH, pConfigLenSet->m_PSDXHDRLength);
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
	}

	if (NULL != pConfigLenGet)
	{
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_TITLE_LENGTH, &(pConfigLenGet->m_PSDTitleLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_ARTIST_LENGTH, &(pConfigLenGet->m_PSDArtistLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_ALBUM_LENGTH, &(pConfigLenGet->m_PSDAlbumLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_GENRE_LENGTH, &(pConfigLenGet->m_PSDGenreLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_COMMENT_SHORT_CONTENT_DESCRIPTION_LENGTH, &(pConfigLenGet->m_PSDCommentShortLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_COMMENT_ACTUAL_TEXT_LENGTH, &(pConfigLenGet->m_PSDCommentLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_UFID_OWNER_IDENTIFIER_LENGTH, &(pConfigLenGet->m_PSDUFIDLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_COMMERCIAL_PRICE_STRING_LENGTH, &(pConfigLenGet->m_PSDCommercialPriceLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_COMMERCIAL_CONTACT_URL_LENGTH, &(pConfigLenGet->m_PSDCommercialContactLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_COMMERCIAL_NAME_OF_SELLER_LENGTH, &(pConfigLenGet->m_PSDCommercialSellerLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_COMMERCIAL_DESCRIPTION_LENGTH, &(pConfigLenGet->m_PSDCommercialDescriptionLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
		retval = ETAL_cmdPSDCnfgLenGet_HDRADIO(hReceiver, PSD_XHDR_LENGTH, &(pConfigLenGet->m_PSDXHDRLength));
		if (OSAL_OK != retval)
		{
			return ETAL_RET_ERROR;
		}
	}
	return ETAL_RET_SUCCESS;
#else
	return ETAL_RET_NOT_IMPLEMENTED;
#endif
}

#endif // (CONFIG_ETAL_HAVE_DATASERVICES) || (CONFIG_ETAL_HAVE_ALL_API)

/******************************************************************************
 * Exported functions
 *****************************************************************************/

#if defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * ETAL_enable_data_service
 *
 **************************/
/*
 * <ServiceBitmap> is a combination of EtalDataServiceType values
 */
ETAL_STATUS ETAL_enable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap, tU32 *EnabledServiceBitmap, EtalDataServiceParam ServiceParameters)
{
    ETAL_STATUS ret;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
    ret = ETAL_processDataServiceRequest(hReceiver, ServiceBitmap, EnabledServiceBitmap, cmdActionStart, ServiceParameters);
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
    //check that receiver is HD
    if (!ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
    {
    	ret = ETAL_RET_PARAMETER_ERR;
    }
    else
    {
    	//check ServiceBitmap for ETAL_DATASERV_TYPE_PSD
    	if (0 != (ServiceBitmap & ETAL_DATASERV_TYPE_PSD))
    	{
    		ret = ETAL_cmdStartPSDDataServ_HDRADIO(hReceiver);
    		if (EnabledServiceBitmap != NULL)
    		{
    			*EnabledServiceBitmap = ETAL_DATASERV_TYPE_PSD;
    		}
    	}
    }
#else
    ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

    return ret;
}

/***************************
 *
 * etal_enable_data_service
 *
 **************************/
/*
 * <ServiceBitmap> is a combination of EtalDataServiceType values
 */
ETAL_STATUS etal_enable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap, tU32 *EnabledServiceBitmap, EtalDataServiceParam ServiceParameters)
{
	ETAL_STATUS ret;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_enable_data_service(rec: %d, serBitMap: %u, ecc: %u, eid: %u, sid: %u, logoType: %u, JMLObjId: %u)", hReceiver, ServiceBitmap,
		ServiceParameters.m_ecc, ServiceParameters.m_eid, ServiceParameters.m_sid, ServiceParameters.m_logoType, ServiceParameters.m_JMLObjectId);

	/* HDRadio adds a periodic callback in #ETAL_cmdStartPSDDataServ_HDRADIO
	 * so need to take the global lock */
	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_enable_data_service(hReceiver, ServiceBitmap, EnabledServiceBitmap, ServiceParameters);
		ETAL_statusReleaseLock();
	}
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_enable_data_service() = %s", ETAL_STATUS_toString(ret));
    if (EnabledServiceBitmap != NULL)
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "enaSerBitMap : 0x%x", *EnabledServiceBitmap);
    }
	return ret;
}

/***************************
 *
 * ETAL_disable_data_service
 *
 **************************/
/*
 * <ServiceBitmap> is a combination of EtalDataServiceType values
 */
ETAL_STATUS ETAL_disable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap)
{
    ETAL_STATUS ret;
	EtalDataServiceParam param;

	(void)OSAL_pvMemorySet((tVoid *)&param, 0, sizeof(param));

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	ret = ETAL_processDataServiceRequest(hReceiver, ServiceBitmap, NULL, cmdActionStop, param);
#elif defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	//check that receiver is HD
	if (!ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		//check ServiceBitmap for ETAL_DATASERV_TYPE_PSD
		if (0 != (ServiceBitmap & ETAL_DATASERV_TYPE_PSD))
		{
			ret = ETAL_cmdStopPSDDataServ_HDRADIO(hReceiver);
		}
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

	return ret;
}

/***************************
 *
 * etal_disable_data_service
 *
 **************************/
/*
 * <ServiceBitmap> is a combination of EtalDataServiceType values
 */
ETAL_STATUS etal_disable_data_service(ETAL_HANDLE hReceiver, tU32 ServiceBitmap)
{
    ETAL_STATUS ret;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_disable_data_service(rec: %d, serBitMap: %d)", hReceiver, ServiceBitmap);

	/* HDRadio removes a periodic callback in #ETAL_cmdStopPSDDataServ_HDRADIO
	 * so need to take the global lock */
	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		ret = ETAL_disable_data_service(hReceiver, ServiceBitmap);
		ETAL_statusReleaseLock();
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_disable_data_service() = %s", ETAL_STATUS_toString(ret));
    return ret;
}
#endif // CONFIG_ETAL_HAVE_DATASERVICES || CONFIG_ETAL_HAVE_ALL_API

