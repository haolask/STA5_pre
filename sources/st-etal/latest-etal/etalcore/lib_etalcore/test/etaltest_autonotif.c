//!
//!  \file 		etaltest_autonotif.c
//!  \brief 	<i><b> ETAL test, DAB autonotification functions </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Jean-Hugues
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etalinternal.h"

#include "etaltest.h"

/***************************
 * Local Macros
 **************************/
#define ETAL_TEST_DAB_RECONFIGURATION_TIME_WAIT					1000
#define ETAL_TEST_DAB_DAB_STATUS_TIME_WAIT						10000
#define ETAL_TEST_DAB_DAB_DATA_STATUS_TIME_WAIT					1000
#define ETAL_TEST_DAB_ANNOUNCEMENT_SWITCHING_TIME_WAIT			1000
#define ETAL_TEST_DAB_ANNOUNCEMENT_SWITCHING_RAW_TIME_WAIT		1000
#define ETAL_TEST_DAB_SERVICE_FOLLOWING_TIME_WAIT				1000
#define ETAL_TEST_DAB_ALL_EVENT_TIME_WAIT                       1000

/***************************
 * Local types
 **************************/

/***************************
 * Local variables
 **************************/

/***************************
 * function prototypes
 **************************/
#if defined(CONFIG_APP_TEST_AUTONOTIFICATION) && defined(CONFIG_APP_TEST_DAB) && defined (CONFIG_ETAL_HAVE_ALL_API)

tSInt etalTestAutonotification_DAB_Reconfiguration(void);
tSInt etalTestAutonotification_DAB_Status(void);
tSInt etalTestAutonotification_DAB_Announcement_Switching(void);
tSInt etalTestAutonotification_DAB_Announcement_Switching_RAW(void);
tSInt etalTestAutonotification_DAB_Data_Status(void);
tSInt etalTestAutonotification_DAB_Service_Following_Notification(void);
tSInt etalTestAutonotification_All_Event(void);


/***************************
 *
 * etalTestAutonotification_DAB_Reconfiguration
 *
 **************************/
tSInt etalTestAutonotification_DAB_Reconfiguration(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	tU16 enabledEvent = 0;

	etalTestPrintNormal("<---DAB Reconfiguration Test start");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
	
	/* enable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB Reconfiguration test FAILURE");
		retval = OSAL_ERROR;
	}
	
	if(enabledEvent != ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION)
	{
		etalTestPrintError("DAB Reconfiguration test FAILURE");
		retval = OSAL_ERROR;
	}

	OSAL_s32ThreadWait(ETAL_TEST_DAB_RECONFIGURATION_TIME_WAIT);

	/* disable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_NONE, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB Reconfiguration test FAILURE");
		retval = OSAL_ERROR;
	}

	if((retval == OSAL_OK) || (enabledEvent != ETAL_AUTONOTIF_TYPE_NONE))
	{
		etalTestPrintNormal("DAB Reconfiguration test SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestAutonotification_DAB_Status
 *
 **************************/
tSInt etalTestAutonotification_DAB_Status(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	tU16 enabledEvent = 0;

	etalTestPrintNormal("<---DAB_Status Test start");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/* enable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_DAB_STATUS, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB_Status test FAILED");
		retval = OSAL_ERROR;
	}
	
	if(enabledEvent != ETAL_AUTONOTIF_TYPE_DAB_STATUS)
	{
		etalTestPrintError("DAB_Status test FAILURE");
		retval = OSAL_ERROR;
	}

	OSAL_s32ThreadWait(ETAL_TEST_DAB_DAB_STATUS_TIME_WAIT);

	/* disable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_NONE, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB_Status test FAILED");
		retval = OSAL_ERROR;
	}

	if((retval == OSAL_OK) || (enabledEvent != ETAL_AUTONOTIF_TYPE_NONE))
	{
		etalTestPrintNormal("DAB_Status test SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestAutonotification_DAB_Announcement_Switching
 *
 **************************/
tSInt etalTestAutonotification_DAB_Announcement_Switching(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	tU16 enabledEvent = 0, i;
	tU16 service[10] = {ETAL_DAB_ETI8_SERV1_SID, ETAL_DAB_ETI8_SERV2_SID, ETAL_DAB_ETI8_SERV3_SID, ETAL_DAB_ETI8_SERV4_SID,
	        ETAL_DAB_ETI8_SERV5_SID, ETAL_DAB_ETI8_SERV6_SID, ETAL_DAB_ETI8_SERV7_SID, ETAL_DAB_ETI8_SERV8_SID,
	        ETAL_DAB_ETI8_SERV9_SID, ETAL_DAB_ETI8_SERV10_SID};

	etalTestPrintNormal("<---DAB_Announcement_Switching Test start");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	for(i=0; i<10; i++)
	{
        if ((ret = etal_service_select_audio(handledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI8_UEID, service[i], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
            etalTestPrintError("DAB_Announcement_Switching test FAILED");
        }

        /* enable DAB auto notification */
        if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING, &enabledEvent)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
            etalTestPrintError("DAB_Announcement_Switching test FAILED");
            retval = OSAL_ERROR;
        }

        if(enabledEvent != ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING)
        {
            etalTestPrintError("DAB_Announcement_Switching test FAILURE");
            retval = OSAL_ERROR;
        }

        OSAL_s32ThreadWait(ETAL_TEST_DAB_ANNOUNCEMENT_SWITCHING_TIME_WAIT);

        /* disable DAB auto notification */
        if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_NONE, &enabledEvent)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
            etalTestPrintError("DAB_Announcement_Switching test FAILED");
            retval = OSAL_ERROR;

        }
	}

	if((retval == OSAL_OK) || (enabledEvent != ETAL_AUTONOTIF_TYPE_NONE))
	{
		etalTestPrintNormal("DAB_Announcement_Switching test SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestAutonotification_DAB_Announcement_Switching_RAW
 *
 **************************/
tSInt etalTestAutonotification_DAB_Announcement_Switching_RAW(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	tU16 enabledEvent = 0, i;
    tU16 service[10] = {ETAL_DAB_ETI8_SERV1_SID, ETAL_DAB_ETI8_SERV2_SID, ETAL_DAB_ETI8_SERV3_SID, ETAL_DAB_ETI8_SERV4_SID,
            ETAL_DAB_ETI8_SERV5_SID, ETAL_DAB_ETI8_SERV6_SID, ETAL_DAB_ETI8_SERV7_SID, ETAL_DAB_ETI8_SERV8_SID,
            ETAL_DAB_ETI8_SERV9_SID, ETAL_DAB_ETI8_SERV10_SID};

	etalTestPrintNormal("<---DAB_Announcement_Switching_RAW Test start");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

    for(i=0; i<10; i++)
    {
        if ((ret = etal_service_select_audio(handledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI8_UEID, service[i], ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
            etalTestPrintError("DAB_Announcement_Switching_RAW test FAILED");
        }

        /* enable DAB auto notification */
        if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW, &enabledEvent)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
            etalTestPrintError("DAB_Announcement_Switching_RAW test FAILED");
            retval = OSAL_ERROR;
        }

        if(enabledEvent != ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW)
        {
            etalTestPrintError("DAB_Announcement_Switching_RAW test FAILURE");
            retval = OSAL_ERROR;
        }

        OSAL_s32ThreadWait(ETAL_TEST_DAB_ANNOUNCEMENT_SWITCHING_RAW_TIME_WAIT);

        /* disable DAB auto notification */
        if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_NONE, &enabledEvent)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
            etalTestPrintError("DAB_Announcement_Switching_RAW test FAILED");
            retval = OSAL_ERROR;

        }
    }

	if((retval == OSAL_OK) || (enabledEvent != ETAL_AUTONOTIF_TYPE_NONE))
	{
		etalTestPrintNormal("DAB_Announcement_Switching_RAW test SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestAutonotification_DAB_Data_Status
 *
 **************************/
tSInt etalTestAutonotification_DAB_Data_Status(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	tU16 enabledEvent = 0;

	etalTestPrintNormal("<---DAB_Data_Status Test start");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/* enable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB_Data_Status test FAILED");
		retval = OSAL_ERROR;
	}

	if(enabledEvent != ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS)
	{
		etalTestPrintError("DAB_Data_Status test FAILURE");
		retval = OSAL_ERROR;
	}

	OSAL_s32ThreadWait(ETAL_TEST_DAB_DAB_DATA_STATUS_TIME_WAIT);

	/* disable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_NONE, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB_Data_Status test FAILED");
		retval = OSAL_ERROR;
	}

	if((retval == OSAL_OK) || (enabledEvent != ETAL_AUTONOTIF_TYPE_NONE))
	{
		etalTestPrintNormal("DAB_Data_Status test SUCCESS");
	}
	return retval;
}


/***************************
 *
 *
 * etalTestAutonotification_All_Event
 *
 **************************/
tSInt etalTestAutonotification_All_Event(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	tU16 enabledEvent = 0;

	etalTestPrintNormal("<---DAB all Notification Test start");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/* enable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_ALL, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB all Notification test FAILURE");
		retval = OSAL_ERROR;
	}

	if(enabledEvent != ETAL_AUTONOTIF_TYPE_ALL)
	{
		etalTestPrintError("DAB all Notification test FAILURE");
		retval = OSAL_ERROR;
	}

	OSAL_s32ThreadWait(ETAL_TEST_DAB_ALL_EVENT_TIME_WAIT);

	/* disable DAB auto notification */
	if ((ret = etal_setup_autonotification(handledab, ETAL_AUTONOTIF_TYPE_NONE, &enabledEvent)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_autonotification (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DAB all Notification test FAILED");
		retval = OSAL_ERROR;
	}

	if((retval == OSAL_OK) || (enabledEvent != ETAL_AUTONOTIF_TYPE_NONE))
	{
		etalTestPrintNormal("DAB all Notification test SUCCESS");
	}
	return retval;
}


/***************************
 *
 * etalTestAutonotification_internal
 *
 **************************/
static tSInt etalTestAutonotification_internal(etalTestBroadcastTy test_mode, EtalAutonotificationEventType eventType, tBool *pass)
{
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	ETAL_HANDLE *phReceiver;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_DAB:
			config_mode = ETAL_CONFIG_DAB;
			tune_mode = ETAL_TUNE_DAB; // not used
			phReceiver = &handledab; // not used
			break;

		default:
			return OSAL_ERROR;
	}

	if (etalTestDoConfigSingle(config_mode, phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}


	if (test_mode == ETAL_TEST_MODE_DAB)
	{
		switch(eventType)
		{
		case ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION:
			if (OSAL_OK != etalTestAutonotification_DAB_Reconfiguration())
			{
				*pass = FALSE;
			}
			break;

		case ETAL_AUTONOTIF_TYPE_DAB_STATUS:
			if (OSAL_OK != etalTestAutonotification_DAB_Status())
			{
				*pass = FALSE;
			}
			break;

		case ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING:
			if (OSAL_OK != etalTestAutonotification_DAB_Announcement_Switching())
			{
				*pass = FALSE;
			}
			break;

		case ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW:
			if (OSAL_OK != etalTestAutonotification_DAB_Announcement_Switching_RAW())
			{
				*pass = FALSE;
			}
			break;

		case ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS:
			if (OSAL_OK != etalTestAutonotification_DAB_Data_Status())
			{
				*pass = FALSE;
			}
			break;

		case ETAL_AUTONOTIF_TYPE_ALL:
			if (OSAL_OK != etalTestAutonotification_All_Event())
			{
				*pass = FALSE;
			}
			break;

		default:
			*pass = FALSE;
			break;
		}
    }

	if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // #if defined(CONFIG_APP_TEST_AUTONOTIFICATION) && defined(CONFIG_APP_TEST_DAB) && defined (CONFIG_ETAL_HAVE_ALL_API)



/***************************
 *
 * etalTestAutonotification
 *
 **************************/
tSInt etalTestAutonotification(void)
{
#if defined(CONFIG_APP_TEST_AUTONOTIFICATION) && defined(CONFIG_APP_TEST_DAB) && defined (CONFIG_ETAL_HAVE_ALL_API)
	tBool pass = TRUE;

	etalTestStartup();

	if (OSAL_OK != etalTestAutonotification_internal(ETAL_TEST_MODE_DAB, ETAL_AUTONOTIF_TYPE_DAB_RECONFIGURATION, &pass))
	{
		return OSAL_ERROR;
	}

	if (OSAL_OK != etalTestAutonotification_internal(ETAL_TEST_MODE_DAB, ETAL_AUTONOTIF_TYPE_DAB_STATUS, &pass))
	{
		return OSAL_ERROR;
	}

	if (OSAL_OK != etalTestAutonotification_internal(ETAL_TEST_MODE_DAB, ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING, &pass))
	{
		return OSAL_ERROR;
	}

	if (OSAL_OK != etalTestAutonotification_internal(ETAL_TEST_MODE_DAB, ETAL_AUTONOTIF_TYPE_DAB_ANNOUNCEMENT_SWITCHING_RAW, &pass))
	{
		return OSAL_ERROR;
	}

	if (OSAL_OK != etalTestAutonotification_internal(ETAL_TEST_MODE_DAB, ETAL_AUTONOTIF_TYPE_DAB_DATA_STATUS, &pass))
	{
		return OSAL_ERROR;
	}

	if (OSAL_OK != etalTestAutonotification_internal(ETAL_TEST_MODE_DAB, ETAL_AUTONOTIF_TYPE_ALL, &pass))
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // #if defined(CONFIG_APP_TEST_AUTONOTIFICATION) && defined(CONFIG_APP_TEST_DAB)
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

