//!
//!  \file 		etaltest_advtune.c
//!  \brief 	<i><b> ETAL test, advanced tuning functions </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

/***************************
 * Local Macros
 **************************/
#define ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME  (15 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_DAB_LEARN_WAIT_AFTER_STOP  ETAL_TEST_ONE_SECOND

#ifdef CONFIG_APP_TEST_IN_LE_MANS
#define ETAL_TEST_SCAN_ALL_COUNT_MIN   3
#else
#define ETAL_TEST_SCAN_ALL_COUNT_MIN   10
#endif

#define ETAL_TEST_SCAN_FEW_COUNT_MIN   5

#define ETAL_TEST_SCAN_STEP_TIME       3
#define ETAL_TEST_SCAN_STEP_TIME_SHORT 1
/***************************
 * Local types
 **************************/

#ifdef CONFIG_APP_TEST_ADVANCED_TUNING
typedef enum {
	ADV_TUNE_CHECK_EID = 1,
	ADV_TUNE_SERVICE_SELECT_WRONG_SID,
	ADV_TUNE_SERVICE_SELECT_ILLEGAL_PARAM,
	ADV_TUNE_SERVICE_SELECT_SC,
	ADV_TUNE_SERVICE_SELECT_INVALID_SC,
	ADV_TUNE_SERVICE_SELECT_SUBCH,
	ADV_TUNE_SERVICE_SELECT_INVALID_SUBCH,
	ADV_TUNE_SERVICE_SELECT_ANALOG_ONLY_FM,
	ADV_TUNE_SERVICE_SELECT_ANALOG_ONLY_AM
} AdvancedTuningTestTy;
#endif

/***************************
 * Local variables
 **************************/
#if defined (CONFIG_APP_TEST_DAB) || defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
#if defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_APP_TEST_SEAMLESS)

/***************************
 *
 * etalTestDoServiceSelectAudio
 *
 **************************/
tSInt etalTestDoServiceSelectAudio(ETAL_HANDLE hReceiver, tU32 ueid, tU32 sid)
{
    EtalAudioInterfTy audioIf;
	ETAL_STATUS ret;

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;

	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = true;
#endif

// Audio path should be correctly set before
#if 0
    if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
    }
#endif

	/* Service Select by SID */

	etalTestPrintNormal("* Service Select by Sid");

	if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ueid, sid, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* optional but useful, select audio */
	etalTestPrintNormal("* Select DAB audio");
	if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintNormal("etal_audio_select (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	return OSAL_OK;
}
#endif // CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API
#endif // CONFIG_APP_TEST_DAB || CONFIG_APP_TEST_HDRADIO_FM || CONFIG_APP_TEST_HDRADIO_AM


#ifdef CONFIG_APP_TEST_ADVANCED_TUNING

#if defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
#if defined (CONFIG_ETAL_HAVE_ADVTUNE) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etalTestServiceSelect
 *
 **************************/
static tSInt etalTestServiceSelect(ETAL_HANDLE hReceiver, etalTestBroadcastTy dab_or_hd, tBool *pass)
{
	ETAL_STATUS ret, ret_ref;
	tSInt service, eid;

	if (dab_or_hd == ETAL_TEST_MODE_DAB)
	{
		if (etalTestDoServiceSelectAudio(hReceiver, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID))
		{
			return OSAL_ERROR;
		}
		eid = ETAL_DAB_ETI_UEID;
		service = ETAL_DAB_ETI_SERV1_SID;
	}
	else if (dab_or_hd == ETAL_TEST_MODE_HD_FM)
	{
		etalTestPrintNormal("* Service Select by SPS");

		eid = ETAL_INVALID_UEID;
		service = ETAL_HD_SPS_FM;

		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, eid, service, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	else
	{
		etalTestPrintNormal("* Service Select by SPS");

		eid = ETAL_INVALID_UEID;
		service = ETAL_HD_SPS_AM;

		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, eid, service, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}

	etalTestPrintNormal("Listen to some audio");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Service Select non existing SID */

	etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_WRONG_SID, dab_or_hd, "Service Select by non-existing Sid");
	if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, eid, service + 1, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_NO_DATA)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Service Select illegal parameters */

	etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_ILLEGAL_PARAM, dab_or_hd, "Service Select with illegal parameters");
if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ETAL_INVALID_UEID, ETAL_INVALID_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_PARAMETER_ERR)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Service Select by SC */

	etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_SC, dab_or_hd, "Service Select by SC");
	if (dab_or_hd == ETAL_TEST_MODE_DAB)
	{
		ret_ref = ETAL_RET_SUCCESS;
	}
	else
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		ret_ref = ETAL_RET_INVALID_HANDLE;
#else
		ret_ref = ETAL_RET_PARAMETER_ERR;
#endif
	}
	ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_DAB_SC, eid, service, ETAL_DAB_ETI_SERV1_SC, ETAL_INVALID);
	if (ret != ret_ref)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (dab_or_hd == ETAL_TEST_MODE_DAB)
	{
		/* Service Select by invalid SC */

		etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_INVALID_SC, dab_or_hd, "Service Select by invalid SC");
		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_DAB_SC, eid, service, ETAL_DAB_ETI_SERV1_SC + 1, ETAL_INVALID)) != ETAL_RET_NO_DATA)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Service Select by subchannel*/

		etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_SUBCH, dab_or_hd, "Service Select by subchannel");
		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_DAB_SUBCH, eid, service, ETAL_INVALID, ETAL_DAB_ETI_SERV1_SUBCH)) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Service Select by invalid subchannel*/

		etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_INVALID_SUBCH, dab_or_hd, "Service Select by invalid subchannel");
		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_DAB_SUBCH, eid, service, ETAL_INVALID, 0xFF)) != ETAL_RET_NO_DATA)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	else if(dab_or_hd == ETAL_TEST_MODE_HD_FM)
	{
		/* Tune to FM only (expect error) and try a service select */
		etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_NO_DATA)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_ANALOG_ONLY_FM, dab_or_hd, "Service Select to analog-only FM");

		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ETAL_INVALID_UEID, service, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_NO_DATA)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	else
	{
		/* Tune to AM only (expect error) and try a service select */
		etalTestPrintNormal("* Tune to AM freq %d", ETAL_VALID_AM_FREQ);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_AM_FREQ)) != ETAL_RET_NO_DATA)
		{
			etalTestPrintError("etal_tune_receiver AM (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		etalTestPrintReportPassStart(ADV_TUNE_SERVICE_SELECT_ANALOG_ONLY_AM, dab_or_hd, "Service Select to analog-only AM");

		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ETAL_INVALID_UEID, service, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_NO_DATA)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	return OSAL_OK;
}
#endif // CONFIG_ETAL_HAVE_ADVTUNE || CONFIG_ETAL_HAVE_ALL_API
#endif // CONFIG_APP_TEST_DAB || CONFIG_APP_TEST_HDRADIO_FM || CONFIG_APP_TEST_HDRADIO_AM
#endif // CONFIG_APP_TEST_ADVANCED_TUNING

/***************************
 *
 * etalTestAdvancedTuning
 *
 **************************/
tSInt etalTestAdvancedTuning(void)
{
#ifdef CONFIG_APP_TEST_ADVANCED_TUNING
#ifdef CONFIG_APP_TEST_DAB
	ETAL_STATUS ret;
	tU32 ueid;
#endif
	tBool pass1;
	tBool pass2;

	etalTestStartup();
	pass1 = TRUE;
	pass2 = TRUE;

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("* Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/*
	 * pass1, Get Ensemble 
	 */
	etalTestPrintReportPassStart(ADV_TUNE_CHECK_EID, ETAL_TEST_MODE_DAB, "Check EnsembleId");
	if ((ret = etal_get_current_ensemble(handledab, &ueid)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_current_ensemble (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if (ueid != ETAL_DAB_ETI_UEID)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintNormal("= Current EID: 0x%x", ueid);

	if (!pass1)
	{
		etalTestPrintNormal("pass1 FAILED");
	}


	/*
	 * pass2, ServiceSelect
	 */
	if (etalTestServiceSelect(handledab, ETAL_TEST_MODE_DAB, &pass2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass2)
	{
		etalTestPrintNormal("pass2 FAILED");
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * pass2, ServiceSelect
	 */
	if (ETAL_FE_FOR_HD_TEST == ETAL_FE_HANDLE_1)
	{
		if (etalTestServiceSelect(handlehd, ETAL_TEST_MODE_HD_FM, &pass2) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		etalTestPrintNormal("Skipping pass2 for HD since instance does not support audio");
	}
	if (!pass2)
	{
		etalTestPrintNormal("pass2 FAILED");
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * pass2, ServiceSelect
	 */
	if (ETAL_FE_FOR_HD_TEST == ETAL_FE_HANDLE_1)
	{
		if (etalTestServiceSelect(handlehd, ETAL_TEST_MODE_HD_AM, &pass2) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		etalTestPrintNormal("Skipping pass2 for HD since instance does not support audio");
	}
	if (!pass2)
	{
		etalTestPrintNormal("pass2 FAILED");
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	if (!pass1 || !pass2)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_ADVANCED_TUNING
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

