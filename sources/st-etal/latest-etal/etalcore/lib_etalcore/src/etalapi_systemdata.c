//!
//!  \file 		etalapi_systemdata.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, System Data functionalities
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static char* ETAL_get_streamType(EtalServiceInfo svinfo);


/***************************
 *
 * etal_get_ensemble_list
 *
 **************************/
ETAL_STATUS etal_get_ensemble_list(EtalEnsembleList *pEnsembleList)
{
	tBool have_data;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU32 i;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_ensemble_list()");

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DAB DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */
	if (pEnsembleList == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		if (ETAL_cmdGetEnsembleList_MDR(pEnsembleList, &have_data) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			if (!have_data)
			{
				ret = ETAL_RET_NO_DATA;
			}
		}
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_ensemble_list() = %s", ETAL_STATUS_toString(ret));
    if(have_data && (pEnsembleList != NULL))
    {
        for(i = 0; i > pEnsembleList->m_ensembleCount; i++)
        {
            ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "Struct EtalEnsembleList : ECC: %d, ensId: %d, fre: %d",
                    pEnsembleList->m_ensemble[i].m_ECC, pEnsembleList->m_ensemble[i].m_ensembleId,
                    pEnsembleList->m_ensemble[i].m_frequency);
        }
    }
    return ret;
}

/***************************
 *
 * etal_get_ensemble_data
 *
 **************************/
ETAL_STATUS etal_get_ensemble_data(tU32 eid, tU8 *charset, tChar *label, tU16 *bitmap)
{
	tBool have_data;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_ensemble_data(eid: 0x%x)", eid);

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DAB DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */

	if ((charset == NULL) || (label == NULL) || (bitmap == NULL))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		if (ETAL_cmdGetEnsembleData_MDR(eid, charset, label, bitmap, &have_data) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			if (!have_data)
			{
				ret = ETAL_RET_NO_DATA;
			}
		}
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_ensemble_data() = %s", ETAL_STATUS_toString(ret));
    if((charset != NULL) && (label != NULL) &&(bitmap != NULL))
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "chaSet : %d, lab : %s, bitMap : %d", *charset, label, *bitmap);
    }
	return ret;
}

/***************************
 *
 * etal_get_fic
 *
 **************************/
ETAL_STATUS etal_get_fic(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_fic(rec:%d)", hReceiver);

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DAB DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */

	if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (ETAL_BCAST_STD_DAB != ETAL_receiverGetStandard(hReceiver))
	{
		ret = ETAL_RET_INVALID_BCAST_STANDARD;
	}
	else if (OSAL_OK != ETAL_cmdGetFIC_MDR(hReceiver, cmdActionStart))
	{
		ret = ETAL_RET_ERROR;
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_fic() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_stop_fic
 *
 **************************/
ETAL_STATUS etal_stop_fic(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_stop_fic(rec:%d)", hReceiver);

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DAB DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */

	if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else if (ETAL_BCAST_STD_DAB != ETAL_receiverGetStandard(hReceiver))
	{
		ret = ETAL_RET_INVALID_BCAST_STANDARD;
	}
	else if (OSAL_OK != ETAL_cmdGetFIC_MDR(hReceiver, cmdActionStop))
	{
		ret = ETAL_RET_ERROR;
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_stop_fic() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
#endif // CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA) || defined (CONFIG_ETAL_HAVE_ALL_API)
#if defined (CONFIG_ETAL_SUPPORT_DCOP)
/***************************
 *
 * ETAL_get_service_list
 *
 **************************/
ETAL_STATUS ETAL_get_service_list(ETAL_HANDLE hReceiver, tU32 eid, tBool bGetAudioServices, tBool bGetDataServices, EtalServiceList *pServiceList)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalBcastStandard std;

#if defined (CONFIG_ETAL_SUPPORT_DCOP)
	tBool have_data = FALSE;
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
    tU32 i;
	tS8 serv_s;
#endif //#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_service_list(rec: %d, eid: 0x%x, getAudSer: %d, getDatSer: %d)", hReceiver, eid, bGetAudioServices, bGetDataServices);

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */

	std = ETAL_receiverGetStandard(hReceiver);
	switch (std)
	{
		case ETAL_BCAST_STD_DAB:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			if (ETAL_cmdGetServiceList_MDR(eid, bGetAudioServices, bGetDataServices, pServiceList, &have_data) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}

			if (!have_data)
			{
				ret = ETAL_RET_NO_DATA;
			}
#else
			ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
			break;

		case ETAL_BCAST_STD_HD_FM:
		case ETAL_BCAST_STD_HD_AM:
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			if (ETAL_receiverHasSystemData_HDRADIO(hReceiver))
			{
				have_data = TRUE;
				pServiceList->m_serviceCount = 0;
				for (i = 0; i < ETAL_HD_MAX_PROGRAM_NUM; i++)
				{
					serv_s = ETAL_receiverGetService_HDRADIO(hReceiver, i);
					if (serv_s == ETAL_INVALID_PROG)
					{
						break;
					}
					pServiceList->m_service[i] = (tU32)serv_s;
					pServiceList->m_serviceCount++;
				}
			}

			if (!have_data)
			{
				ret = ETAL_RET_NO_DATA;
			}
#else
			ret = ETAL_RET_NOT_IMPLEMENTED;
#endif
			break;

		default:
			ret = ETAL_RET_INVALID_BCAST_STANDARD;
	}
	return ret;
}

/***************************
 *
 * etal_get_service_list
 *
 **************************/
ETAL_STATUS etal_get_service_list(ETAL_HANDLE hReceiver, tU32 eid, tBool bGetAudioServices, tBool bGetDataServices, EtalServiceList *pServiceList)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
    tU32 i;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_service_list(rec: %d, eid: 0x%x, getAudSer: %d, getDatSer: %d)", hReceiver, eid, bGetAudioServices, bGetDataServices);

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */

	if (pServiceList == NULL)
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else if (!ETAL_handleIsValid(hReceiver))
	{
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		ret = ETAL_RET_INVALID_RECEIVER;
	}
	else
	{
		ret = ETAL_get_service_list(hReceiver, eid, bGetAudioServices, bGetDataServices, pServiceList);
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_service_list() = %s", ETAL_STATUS_toString(ret));
    if(pServiceList != NULL)
    {
        if (ret == ETAL_RET_SUCCESS)
        {
            for(i = 0; i < pServiceList->m_serviceCount;i++)
            {
                ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "service[%d]: 0x%x", i, pServiceList->m_service[i]);
            }
        }
    }
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP
#endif // CONFIG_ETAL_HAVE_SYSTEMDATA || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) || defined (CONFIG_ETAL_HAVE_ALL_API)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
static char* ETAL_get_streamType(EtalServiceInfo svinfo)
{
	static char *p_streamType;
	
	if (svinfo.m_streamType == ETAL_COMPONENTTYPE_IS_MSC_STREAM_AUDIO)
	{
		if (svinfo.m_componentType == ETAL_ASCTY_DAB_PLUS)
		{
			p_streamType = "MODE_DAB_PLUS";
		}
		else
		{
			p_streamType = "MSC_MODE_DAB";
		}
	}
	else if (svinfo.m_streamType == ETAL_COMPONENTTYPE_IS_MSC_STREAM_DATA)
	{
		if (svinfo.m_componentType == ETAL_DSCTY_MPEG2TS)
		{
			p_streamType = "MSC_MODE_DMB";
		}
		else
		{
			p_streamType = "MSC_MODE_DATA";
		}
	}
	else
	{
			p_streamType = "UNKNOWN";
	}

	return p_streamType;
}


/***************************
 *
 * etal_get_specific_service_data_DAB
 *
 **************************/
ETAL_STATUS etal_get_specific_service_data_DAB(tU32 eid, tU32 sid, EtalServiceInfo *serv_info, EtalServiceComponentList *sclist, tVoid *dummy)
{
	tBool have_data;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	char *vl_streamType;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_get_specific_service_data_DAB(eid: 0x%x, sid: 0x%x)", eid, sid);

	/* TODO dummy parameter currently unused */

	/*
	 * This is receiver-exclusive but currently ETAL supports only
	 * one DAB DCOP so the API does not request the hReceiver.
	 * The lock will be taken by the communication layer
	 */

	if ((serv_info == NULL) && (sclist == NULL))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		if (ETAL_cmdGetSpecificServiceData_MDR(eid, sid, serv_info, sclist, &have_data) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			if (!have_data)
			{
				ret = ETAL_RET_NO_DATA;
			}
		}
	}


    //ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_specific_service_data_DAB() = %s", ETAL_STATUS_toString(ret));
	
	// Add log information which is part of the response in deed
	//
	vl_streamType = ETAL_get_streamType(*serv_info);
	
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_get_specific_service_data_DAB (Bitrate: %d, subchID: %d, compType: %d, streamType: %d, streamTypeName: %s, servLabel: %s) = %s", 
		serv_info->m_serviceBitrate, serv_info->m_subchId, serv_info->m_componentType, serv_info->m_streamType, vl_streamType, serv_info->m_serviceLabel, ETAL_STATUS_toString(ret));

	// vl_streamType used for log only => avoid warning
	(tVoid) vl_streamType;


    return ret;
}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
#endif // CONFIG_ETAL_HAVE_SYSTEMDATA_DAB || CONFIG_ETAL_HAVE_ALL_API

