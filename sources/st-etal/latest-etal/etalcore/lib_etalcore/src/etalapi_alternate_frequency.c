//!
//!  \file 		etalapi_alternate_frequency.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application of alternate frequency feature
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_HAVE_ALTERNATE_FREQUENCY) || defined (CONFIG_ETAL_HAVE_ALL_API)

/***************************
 * Defines
 **************************/

/***************************
 * Local functions
 **************************/
static ETAL_STATUS ETAL_checkAFCheckParameters(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p);
static ETAL_STATUS ETAL_checkAFSwitchParameters(ETAL_HANDLE hReceiver, tU32 alternateFrequency);
static ETAL_STATUS ETAL_checkAFStartParameters(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alt_freq, tU32 antennaSelection, EtalBcastQualityContainer* p);
static ETAL_STATUS ETAL_checkAFEndParameters(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p);
static ETAL_STATUS ETAL_checkGetAFQualityParameters(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p);

/***************************
 *
 * ETAL_checkAFReceiver
 *
 **************************/
/* This function is used by all the external APIs to check Receiver validity,
 * but some APIs expect the AF to be in progress (e.g. AF_end) others
 * expect no cmdSpecialAnyChangingFrequency in progress.
 * Parameter <expect_active> defines this behavior */
static ETAL_STATUS ETAL_checkAFReceiver(ETAL_HANDLE hReceiver, tBool expect_active)
{
	if (!ETAL_handleIsValid(hReceiver))
	{
		return ETAL_RET_INVALID_HANDLE;
	}
	if (!ETAL_receiverIsValidHandle(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if (!ETAL_receiverSupportsQuality(hReceiver))
	{
		return ETAL_RET_INVALID_RECEIVER;
	}
	else if(ETAL_receiverGetFrequency(hReceiver) == ETAL_INVALID_FREQUENCY)
	{
		return ETAL_RET_ERROR;
	}
	else if ((ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_FM) &&
			 (ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_HD_FM))
	{
		return ETAL_RET_INVALID_BCAST_STANDARD;
	}
	else
	{
		/* Nothing to do */
	}

	if (expect_active)
	{
		if (!ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialManualAFCheck))
		{
			return ETAL_RET_INVALID_RECEIVER;
		}
	}
	else
	{
		if (ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialAnyChangingFrequency))
		{
			return ETAL_RET_IN_PROGRESS;
		}
	}

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_checkAFCheckParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkAFCheckParameters(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret;
	etalFrequencyBandInfoTy band_info;

	ret = ETAL_checkAFReceiver(hReceiver, FALSE);
	if (ret != ETAL_RET_SUCCESS)
	{
		return ret;
	}

	if((antennaSelection != 0x00) && (antennaSelection != 0x01) && (antennaSelection != 0x02))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	if(p == NULL)
    {
        return ETAL_RET_PARAMETER_ERR;
    }

	if (ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}

	if ((alternateFrequency < band_info.bandMin) || (alternateFrequency > band_info.bandMax))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_AF_check
 *
 **************************/
ETAL_STATUS ETAL_AF_check(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ret = ETAL_checkAFCheckParameters(hReceiver, alternateFrequency, antennaSelection, p);
	if(ret == ETAL_RET_SUCCESS)
	{
		/* Send AF Check command */
		if(ETAL_cmdAFCheck_CMOST(hReceiver, alternateFrequency, antennaSelection, p) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_checkGetAFQualityParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkGetAFQualityParameters(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
    ETAL_STATUS ret;

    ret = ETAL_checkAFReceiver(hReceiver, FALSE);
    if (ret != ETAL_RET_SUCCESS)
    {
        return ret;
    }

    if(p == NULL)
    {
        return ETAL_RET_PARAMETER_ERR;
    }

    return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_Get_AF_quality
 *
 **************************/
ETAL_STATUS ETAL_get_AF_quality(ETAL_HANDLE hReceiver, EtalBcastQualityContainer* p)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ret = ETAL_checkGetAFQualityParameters(hReceiver, p);
    if(ret == ETAL_RET_SUCCESS)
    {
        /* Send AF Check command */
        if(ETAL_cmdGetAFQuality_CMOST(hReceiver, p) != OSAL_OK)
        {
            ret = ETAL_RET_ERROR;
        }
    }
    return ret;
}

/***************************
 *
 * ETAL_AF_check_and_get_AF_quality
 *
 **************************/
ETAL_STATUS ETAL_AF_check_and_get_AF_quality(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
    EtalBcastQualityContainer quality_ignored;

    ret = ETAL_AF_check(hReceiver, alternateFrequency, antennaSelection, &quality_ignored);

    if(ret == ETAL_RET_SUCCESS)
    {
        /* AF check command is not able to provide result of the current measurement */
        /* in the command answer. It reports only the latest AF check measurement. */
        /* In order to get result of the current measurement, the Get AF quality command */
        /* has to be sent to CMOST */

        ret = ETAL_get_AF_quality(hReceiver, p);
    }
    return ret;
}

/***************************
 *
 * etal_AF_check
 *
 **************************/
ETAL_STATUS etal_AF_check(ETAL_HANDLE hReceiver, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_AF_check(rec: %d, altFre: %d, antSel: %d)", hReceiver, alternateFrequency, antennaSelection);

	ret = ETAL_receiverGetLock(hReceiver);
	if(ret == ETAL_RET_SUCCESS)
	{
		if((ret = ETAL_AF_check_and_get_AF_quality(hReceiver, alternateFrequency, antennaSelection, p)) == ETAL_RET_SUCCESS)
		{
		    if(p != NULL)
		    {
		        ETAL_tracePrintQuality(TR_LEVEL_COMPONENT, p);
		    }
		}
        ETAL_receiverReleaseLock(hReceiver);
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_AF_check() = %s", ETAL_STATUS_toString(ret));

	return ret;
}

/***************************
 *
 * ETAL_checkAFSwitchParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkAFSwitchParameters(ETAL_HANDLE hReceiver, tU32 alternateFrequency)
{
	ETAL_STATUS ret;
	etalFrequencyBandInfoTy band_info;

	ret = ETAL_checkAFReceiver(hReceiver, FALSE);
	if (ret != ETAL_RET_SUCCESS)
	{
		return ret;
	}

	if (ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}

	if ((alternateFrequency < band_info.bandMin) || (alternateFrequency > band_info.bandMax))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	return ETAL_RET_SUCCESS;
}


/***************************
 *
 * ETAL_AF_switch
 *
 **************************/
ETAL_STATUS ETAL_AF_switch(ETAL_HANDLE hReceiver, tU32 alternateFrequency)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
    EtalTuneInfoInternal vl_TuneStatus;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tU8 cmd[1];
	tU32 device_list;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	ret = ETAL_checkAFSwitchParameters(hReceiver, alternateFrequency);
	if(ret == ETAL_RET_SUCCESS)
	{
		/* Send AF Check command */
		if(ETAL_cmdAFSwitch_CMOST(hReceiver, alternateFrequency) == OSAL_OK)
		{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);

			if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
				ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
			{

				(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);

				/* start the FSM that sends the tune command to the DCOP, waits the response
				 * and sends event to the API caller */
				cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
				if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
				{
					ret = ETAL_RET_ERROR;
				}
			}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

			ETAL_receiverSetFrequency(hReceiver, alternateFrequency, FALSE);

            vl_TuneStatus.m_receiverHandle      = hReceiver;
            vl_TuneStatus.m_Frequency           = alternateFrequency;
            vl_TuneStatus.m_serviceId           = ETAL_INVALID_PROG;
            vl_TuneStatus.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress);;
            vl_TuneStatus.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
            ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&vl_TuneStatus, sizeof(vl_TuneStatus));
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}
	}
	return ret;
}


/***************************
 *
 * etal_AF_switch
 *
 **************************/
ETAL_STATUS etal_AF_switch(ETAL_HANDLE hReceiver, tU32 alternateFrequency)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_AF_switch(rec: %d, altFre: %d)", hReceiver, alternateFrequency);

	ret = ETAL_receiverGetLock(hReceiver);
	if(ret == ETAL_RET_SUCCESS)
	{
        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress, cmdActionStart);

		ret = ETAL_AF_switch(hReceiver, alternateFrequency);

        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress, cmdActionStop);

		ETAL_receiverReleaseLock(hReceiver);
	}

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_AF_switch() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * ETAL_checkAFStartParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkAFStartParameters(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alt_freq, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret;
	etalFrequencyBandInfoTy band_info;

	ret = ETAL_checkAFReceiver(hReceiver, FALSE);

	if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_IN_PROGRESS))
	{
		return ret;
	}

	if((AFMode != cmdNormalMeasurement) && (AFMode != cmdRestartAFMeasurement))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	if(p == NULL)
	{
        return ETAL_RET_PARAMETER_ERR;
	}

	if((antennaSelection != 0x00) && (antennaSelection != 0x01) && (antennaSelection != 0x02))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	if (ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}
	if ((alt_freq < band_info.bandMin) || (alt_freq > band_info.bandMax))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_AF_start
 *
 **************************/
ETAL_STATUS ETAL_AF_start(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
    EtalTuneInfoInternal vl_TuneStatus;

	ret = ETAL_checkAFStartParameters(hReceiver, AFMode, alternateFrequency, antennaSelection, p);
	if(ret == ETAL_RET_SUCCESS)
	{
		/* Set manual AF Check flag to active */
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualAFCheck, cmdActionStart);

		/* Send AF Check command */
		if(ETAL_cmdAFStart_CMOST(hReceiver, AFMode, alternateFrequency, antennaSelection, p) == OSAL_OK)
		{
			ETAL_receiverSetFrequency(hReceiver, alternateFrequency, FALSE);

            vl_TuneStatus.m_receiverHandle      = hReceiver;
            vl_TuneStatus.m_Frequency           = alternateFrequency;
			vl_TuneStatus.m_serviceId           = ETAL_INVALID_PROG;
			vl_TuneStatus.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress);
			vl_TuneStatus.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
            ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&vl_TuneStatus, sizeof(vl_TuneStatus));
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}
	}

	return ret;
}

/***************************
 *
 * etal_AF_start
 *
 **************************/
ETAL_STATUS etal_AF_start(ETAL_HANDLE hReceiver, etalAFModeTy AFMode, tU32 alternateFrequency, tU32 antennaSelection, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
    EtalBcastQualityContainer result_ignored;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_AF_start(rec: %d, AFmod: %d, altFre: %d, antSel: %d)", hReceiver, AFMode, alternateFrequency, antennaSelection);

	ret = ETAL_receiverGetLock(hReceiver);

		
	if(ret == ETAL_RET_SUCCESS)
	{
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress, cmdActionStart);
		
		ret = ETAL_AF_start(hReceiver, AFMode, alternateFrequency, antennaSelection, p);
		
		if(ret == ETAL_RET_SUCCESS)
		{
			/* cannot call ETAL_get_AF_quality because it expects
			 * cmdSpecialAnyChangingFrequency not set and it was
			 * just set by ETAL_AF_start */
			if(ETAL_cmdGetAFQuality_CMOST(hReceiver, p) != OSAL_OK)
			{
				(LINT_IGNORE_RET) ETAL_AF_end(hReceiver, ETAL_receiverGetFrequency(hReceiver), &result_ignored);
				ret = ETAL_RET_ERROR;
			}
			else
			{
				if(p != NULL)
			    {
				    ETAL_tracePrintQuality(TR_LEVEL_COMPONENT, p);
			    }
			}
		}

		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress, cmdActionStop);

		ETAL_receiverReleaseLock(hReceiver);
	}


	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_AF_start() = %s", ETAL_STATUS_toString(ret));
	return ret;
}


/***************************
 *
 * ETAL_checkAFEndParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkAFEndParameters(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret;
	etalFrequencyBandInfoTy band_info;

	ret = ETAL_checkAFReceiver(hReceiver, TRUE);
	if (ret != ETAL_RET_SUCCESS)
	{
		return ret;
	}

	if(p == NULL)
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	if (ETAL_receiverGetBandInfo(hReceiver, &band_info) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}

	if ((frequency < band_info.bandMin) || (frequency > band_info.bandMax))
	{
		return ETAL_RET_PARAMETER_ERR;
	}

	return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_AF_end
 *
 **************************/
ETAL_STATUS ETAL_AF_end(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalTuneInfoInternal vl_TuneStatus;

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	tU8 cmd[1];
	tU32 device_list;
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO

	/* Set manual AF Check flag to active */
	ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualAFCheck, cmdActionStop);

	/* Send AF Check command */
	if(ETAL_cmdAFEnd_CMOST(hReceiver, frequency, p) == OSAL_OK)
	{
		ETAL_receiverSetFrequency(hReceiver, frequency, FALSE);

		vl_TuneStatus.m_receiverHandle      = hReceiver;
		vl_TuneStatus.m_Frequency           = frequency;
		vl_TuneStatus.m_serviceId           = ETAL_INVALID_PROG;
		vl_TuneStatus.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress);
		vl_TuneStatus.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
		ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&vl_TuneStatus, sizeof(vl_TuneStatus));

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		device_list = ETAL_cmdRoutingCheck(hReceiver, commandBandSpecific);

		if (ETAL_CMDROUTE_TO_DCOP(device_list) &&
			ETAL_IS_HDRADIO_STANDARD(ETAL_receiverGetStandard(hReceiver)))
		{
			(LINT_IGNORE_RET) ETAL_tuneFSM_HDRADIO(hReceiver, 0, tuneFSMHDRestart, FALSE, ETAL_HDRADIO_TUNEFSM_API_USER);

			/* start the FSM that sends the tune command to the DCOP, waits the response
			 * and sends event to the API caller */
			cmd[0] = (tU8)HDRADIO_SEEK_START_SPECIAL;
			if (ETAL_queueCommand_HDRADIO(hReceiver, cmd, 1) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
		}
#endif //CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	}
	else
	{
		ret = ETAL_RET_ERROR;
	}
	return ret;
}

/***************************
 *
 * etal_AF_end
 *
 **************************/
ETAL_STATUS etal_AF_end(ETAL_HANDLE hReceiver, tU32 frequency, EtalBcastQualityContainer* p)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
    EtalBcastQualityContainer result_ignored;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_AF_end(rec: %d, fre: %d)", hReceiver, frequency);

	if ((ret = ETAL_checkAFEndParameters(hReceiver, frequency, p)) != ETAL_RET_SUCCESS)
	{
		return ret;
	}

	ret = ETAL_receiverGetLock(hReceiver);
	if(ret == ETAL_RET_SUCCESS)
	{
	
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress, cmdActionStart);
		
		if((ret = ETAL_AF_end(hReceiver, frequency, &result_ignored)) == ETAL_RET_SUCCESS)
		{
            if((ret = ETAL_get_AF_quality(hReceiver, p)) == ETAL_RET_SUCCESS)
            {
        	    if(p != NULL)
        	    {
        	        ETAL_tracePrintQuality(TR_LEVEL_COMPONENT, p);
        	    }
            }
		}

		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress, cmdActionStop);

		ETAL_receiverReleaseLock(hReceiver);
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_AF_end() = %s", ETAL_STATUS_toString(ret));

	return ret;
}

/***************************
 *
 * etal_AF_search_manual
 *
 **************************/
ETAL_STATUS etal_AF_search_manual(ETAL_HANDLE hReceiver, tU32 antennaSelection, tU32 *AFList, tU32 nbOfAF, EtalBcastQualityContainer* AFQualityList)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
    tU32 i;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_AF_search_manual(rec: %d, antSel: %d, nbOfAF: %d)", hReceiver, antennaSelection, nbOfAF);

	ret = ETAL_receiverGetLock(hReceiver);
    if (ret == ETAL_RET_SUCCESS)
    {
    
		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalTuneRequestInProgress, cmdActionStart);

		ret = ETAL_AF_search_manual(hReceiver, antennaSelection, AFList, nbOfAF, AFQualityList);

		ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalTuneRequestInProgress, cmdActionStop);

		ETAL_receiverReleaseLock(hReceiver);
    }

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_AF_search_manual() = %s", ETAL_STATUS_toString(ret));
    for(i = 0; i < nbOfAF; i++)
    {
        if(AFList != NULL)
        {
            ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "AFList[%d]: %d", i, AFList[i]);
        }
        if(AFQualityList != NULL)
        {
            ETAL_tracePrintQuality(TR_LEVEL_COMPONENT, &(AFQualityList[i]));
        }
    }
    return ret;
}

/***************************
 *
 * ETAL_checkAFSearchManualParameters
 *
 **************************/
static ETAL_STATUS ETAL_checkAFSearchManualParameters(ETAL_HANDLE hReceiver, tU32 antennaSelection, tU32 *AFList, tU32 nbOfAF, EtalBcastQualityContainer *AFQualityList)
{
    ETAL_STATUS ret;

    /* TODO: validate frequency based on hReceiver standard? */

    ret = ETAL_checkAFReceiver(hReceiver, FALSE);
    if (ret != ETAL_RET_SUCCESS)
    {
        return ret;
    }

    if((antennaSelection != 0x00) && (antennaSelection != 0x01) && (antennaSelection != 0x02))
    {
        return ETAL_RET_PARAMETER_ERR;
    }

    if(nbOfAF > ETAL_AF_SEARCH_MANUAL_MAX_AF_LIST)
    {
        return ETAL_RET_PARAMETER_ERR;
    }

    if((AFList == NULL) || (AFQualityList == NULL))
    {
        return ETAL_RET_PARAMETER_ERR;
    }
    return ETAL_RET_SUCCESS;
}

/***************************
 *
 * ETAL_AF_search_manual
 *
 **************************/
ETAL_STATUS ETAL_AF_search_manual(ETAL_HANDLE hReceiver, tU32 antennaSelection, tU32 *AFList, tU32 nbOfAF, EtalBcastQualityContainer *AFQualityList)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
    EtalBcastQualityContainer result_ignored;
    EtalBcastQualityContainer *p_quality_container = NULL;
    EtalTuneInfoInternal vl_TuneStatus;
    tU32 i;

    ret = ETAL_checkAFSearchManualParameters(hReceiver, antennaSelection, AFList, nbOfAF, AFQualityList);
    if(ret == ETAL_RET_SUCCESS)
    {
        for(i = 0; i < nbOfAF; i++)
        {
            if(i == 0)
            {
        		/* Set manual AF Check flag to active */
        		ETAL_receiverSetSpecial(hReceiver, cmdSpecialManualAFCheck, cmdActionStart);
        		p_quality_container = &result_ignored;
            }
            else
            {
        		p_quality_container = &AFQualityList[i-1];
            }

            /* Send AF Check command */
    		if(ETAL_cmdAFStart_CMOST(hReceiver, cmdNormalMeasurement, AFList[i], antennaSelection, p_quality_container) == OSAL_OK)
    		{
    			ETAL_receiverSetFrequency(hReceiver, AFList[i], FALSE);

                vl_TuneStatus.m_receiverHandle      = hReceiver;
                vl_TuneStatus.m_Frequency           = AFList[i];
    			vl_TuneStatus.m_serviceId           = ETAL_INVALID_PROG;
    			vl_TuneStatus.m_externalRequestInfo = ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialExternalAlternateFrequencyRequestInProgress);
    			vl_TuneStatus.m_syncInternal        = ETAL_TUNESTATUS_SYNCMASK_FOUND;
                ETAL_intCbScheduleCallbacks(hReceiver, callAtTuneFrequency, (tVoid *)&vl_TuneStatus, sizeof(vl_TuneStatus));
    		}
    		else
    		{
    			ret = ETAL_RET_ERROR;
    		}
        }

        if(ret == ETAL_RET_SUCCESS)
        {
			/* cannot call ETAL_get_AF_quality because it expects
			 * cmdSpecialAnyChangingFrequency not set and it was
			 * just set by ETAL_AF_start */
			if(ETAL_cmdGetAFQuality_CMOST(hReceiver, &AFQualityList[nbOfAF-1]) != OSAL_OK)
			{
				(LINT_IGNORE_RET) ETAL_AF_end(hReceiver, AFList[nbOfAF-1], &result_ignored);
				ret = ETAL_RET_ERROR;
			}
			else
			{
				ret = ETAL_AF_end(hReceiver, AFList[nbOfAF-1], &result_ignored);
			}
        }
        else
        {
			(LINT_IGNORE_RET) ETAL_AF_end(hReceiver, AFList[nbOfAF-1], &result_ignored);
        }
    }

    return ret;
}
#endif // CONFIG_ETAL_HAVE_ALTERNATE_FREQUENCY || CONFIG_ETAL_HAVE_ALL_API
