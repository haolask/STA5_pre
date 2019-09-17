//!
//!  \file 		etalapi_autonotif.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
/******************************************************************************
 * Local functions
 *****************************************************************************/
static tSInt ETAL_autonotificationCheckBitMap(tU16 eventBitmap);

/******************************************************************************
 * Local types
 *****************************************************************************/

/***************************
 *
 * ETAL_autonotificationCheckBitMap
 *
 **************************/
static tSInt ETAL_autonotificationCheckBitMap(tU16 eventBitMap)
{
	if ((eventBitMap & ETAL_AUTONOTIF_TYPE_UNDEFINED) != 0)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * ETAL_updateAutonotification
 *
 **************************/
ETAL_STATUS ETAL_updateAutonotification(ETAL_HANDLE hReceiver, EtalAutonotificationEventType eventBitMap)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if(ETAL_cmdSetupEventNotification_MDR(hReceiver, eventBitMap) != OSAL_OK)
	{
		ret = ETAL_RET_ERROR;
	}

	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * ETAL_setup_autonotification
 *
 **************************/
ETAL_STATUS ETAL_setup_autonotification(ETAL_HANDLE hReceiver, tU16 eventBitMap, tU16 *enabledEventBitMap)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU16 current_eventBitMap;

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
    if (ETAL_autonotificationCheckBitMap(eventBitMap) != OSAL_OK)
	{
		ret = ETAL_RET_ERROR;
	}
	else
	{
		ETAL_receiverGetAutonotification_MDR(hReceiver, &current_eventBitMap);

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_STATUS) != ETAL_AUTONOTIF_TYPE_DAB_STATUS) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_STATUS) == ETAL_AUTONOTIF_TYPE_DAB_STATUS))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABStatusRequestInProgress, cmdActionStart);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS) != ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS) == ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABDataStatusRequestInProgress, cmdActionStart);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION) != ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION) == ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABReconfigurationRequestInProgress, cmdActionStart);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW) != ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW) == ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABAnnouncementRequestInProgress, cmdActionStart);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING) != ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING) == ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABAnnouncementRequestInProgress, cmdActionStart);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_STATUS) == ETAL_AUTONOTIF_TYPE_DAB_STATUS) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_STATUS) != ETAL_AUTONOTIF_TYPE_DAB_STATUS))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABStatusRequestInProgress, cmdActionStop);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS) == ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS) != ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABDataStatusRequestInProgress, cmdActionStop);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION) == ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION) != ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABReconfigurationRequestInProgress, cmdActionStop);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING) == ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING) != ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABAnnouncementRequestInProgress, cmdActionStop);
		}

		if(((current_eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW) == ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW) &&
		   ((eventBitMap & ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW) != ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW))
		{
	        ETAL_receiverSetSpecial(hReceiver, cmdSpecialExternalDABAnnouncementRequestInProgress, cmdActionStop);
		}


		if (ETAL_updateAutonotification(hReceiver, eventBitMap) == ETAL_RET_SUCCESS)
		{
			(LINT_IGNORE_RET)ETAL_receiverSetAutonotification_MDR(hReceiver, eventBitMap);
		}
		else
		{
			ret = ETAL_RET_ERROR;
		}

		if (enabledEventBitMap)
		{
			(LINT_IGNORE_RET)ETAL_receiverGetAutonotification_MDR(hReceiver, enabledEventBitMap);
		}
	}
#else
    ret = ETAL_RET_NOT_IMPLEMENTED;
#endif

    return ret;
}

/***************************
 *
 * etal_setup_autonotification
 *
 **************************/
ETAL_STATUS etal_setup_autonotification(ETAL_HANDLE hReceiver, tU16 eventBitmap, tU16 *enabledEventBitmap)
{
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_setup_autonotification(rec: %d, eveBitMap: 0x%x, enaEveBitMap: 0x%x)",
			hReceiver, eventBitmap, enabledEventBitmap);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_handleIsValid(hReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(hReceiver) ||
				(ETAL_receiverGetStandard(hReceiver) != ETAL_BCAST_STD_DAB))
		{
			/*
			 * auto notification events are only supported in DAB and only some event are supported
			 */
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			ret = ETAL_receiverGetLock(hReceiver);
			if (ret == ETAL_RET_SUCCESS)
			{
				ret = ETAL_setup_autonotification(hReceiver, eventBitmap, enabledEventBitmap);
				ETAL_receiverReleaseLock(hReceiver);
			}
		}
		ETAL_statusReleaseLock();
	}

    if (enabledEventBitmap != NULL)
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_setup_autonotification(enaEveBitMap : 0x%x) = %s"
        		, *enabledEventBitmap, ETAL_STATUS_toString(ret));
    }
    else
    {
        ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_setup_autonotification(*enaEveBitMap : 0x%x) = %s"
        		, NULL, ETAL_STATUS_toString(ret));
    }
	return ret;
}
