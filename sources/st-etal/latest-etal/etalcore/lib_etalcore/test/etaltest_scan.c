//!
//!  \file 		etaltest_scan.c
//!  \brief 	<i><b> ETAL test, seek </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltml_api.h"

#include "etaltest.h"

#define ETAL_TEST_SCAN_FM_STEP_FREQ          100
#define ETAL_TEST_SCAN_AM_STEP_FREQ            1

#define ETAL_TEST_FM_SCAN_WHOLE_BAND_TIME   (80 * ETAL_TEST_ONE_SECOND)

#define ETAL_TEST_FM_SCAN_WAIT_AFTER_STOP   ETAL_TEST_ONE_SECOND
/*
 * Some tests verify whether the callback is called;
 * since the callback is called from a separate thread it needs
 * some time to be executed, this is the max time to wait
 */
#define ETAL_TEST_SCAN_CALLBACK_WAIT        200
#define ETAL_TEST_SCAN_CALLBACK_WAIT_HD     7000

/***************************
 * global variables
 **************************/
etalTestEventCountTy etalTestScanCountFM;
etalTestEventCountTy etalTestScanCountAM;
etalTestEventCountTy etalTestScanCountHD;

OSAL_tSemHandle etalTestScanSem;
tBool vl_etalTestScan_ScanOnGoing = false;

#ifdef CONFIG_APP_TEST_SCAN
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
/***************************
 *
 * etalTestScanFM
 *
 **************************/
static tSInt etalTestScanFMHD(ETAL_HANDLE hReceiver, etalTestBroadcastTy test_type)
{
    tBool pass1 = TRUE;
    tBool pass2 = TRUE;
    ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;
	tU32 freq, initial_freq, current_freq;
	tChar *std;
	etalTestEventCountTy *count;

	if (test_type == ETAL_TEST_MODE_FM)
	{
		freq = ETAL_VALID_FM_FREQ;
		std = "FM";
		count = &etalTestScanCountFM;
	}
	else if (test_type == ETAL_TEST_MODE_HD_FM)
	{
		freq = ETAL_VALID_FM_FREQ;//ETAL_VALID_HD_FREQ;
		std = "HD";
		count = &etalTestScanCountHD;
	}
	else
	{
		return OSAL_ERROR;
	}

    /* Tune to valid FM freq */
    etalTestPrintNormal("* Tune to %s freq %d, handle %d", std, freq, hReceiver);
    ret = etal_tune_receiver(hReceiver, freq);

    if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
    {
        etalTestPrintError("etal_tune_receiver %s (%s, %d), handle %d", std, ETAL_STATUS_toString(ret), ret, hReceiver);
        return OSAL_ERROR;
    }

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
    // Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
    audioIf.m_sai_slave_mode = true;
#endif

    /* Select audio source */
    ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM);
    if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_INVALID_RECEIVER))
    {
        etalTestPrintError("etal_audio_select %s (%s, %d), handle %d", std, ETAL_STATUS_toString(ret), ret, hReceiver);
        return OSAL_ERROR;
    }

    OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

	etalTestResetEventCount(count);

    /* pass1, scan command */
    etalTestPrintNormal("Scan, pass1");
    etalTestPrintNormal("* Scan start");
    if ((ret = etaltml_scan_start(hReceiver, 2 * ETAL_TEST_ONE_SECOND, cmdDirectionUp, ETAL_TEST_SCAN_FM_STEP_FREQ)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_scan_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    // init condition in case finished has been posted later than expected.
    vl_etalTestScan_ScanOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestScanSem, ETAL_TEST_FM_SCAN_WHOLE_BAND_TIME);

    // mark the end of the scan
    // to avoid further event posted in cross case and on scan interruption
    vl_etalTestScan_ScanOnGoing = false;

    /* Check reported events */
    if ((count->STARTED != 1) ||
        (count->RESULT == 0))
    {
        pass1 = FALSE;
    }

    etalTestPrintNormal("* Scan stop");
    if ((ret = etaltml_scan_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_scan_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_SCAN_CALLBACK_WAIT);

    /* Check reported events */
    if (count->FINISHED != 1)
    {
        pass1 = FALSE;
    }

    if (!pass1)
    {
        etalTestPrintNormal("pass1 FAILED");
		etalTestPrintNormal("count not ok : started = %d, Result = %d, Finished = %d", count->STARTED, count->RESULT, count->FINISHED);
    }

	etalTestResetEventCount(count);

    /* pass2, plain scan command */
	if ((ret = etal_get_receiver_frequency(hReceiver, &initial_freq)) != ETAL_RET_SUCCESS)
	{
        etalTestPrintError("etal_get_receiver_frequency (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
	}
    etalTestPrintNormal("Scan, pass2");
    etalTestPrintNormal("* Scan start from %d", initial_freq);
    if ((ret = etaltml_scan_start(hReceiver, 2 * ETAL_TEST_ONE_SECOND, cmdDirectionUp, ETAL_TEST_SCAN_FM_STEP_FREQ)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_scan_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Short delay to abort scan */
    etalTestPrintNormal("Wait some time (%d sec)", ETAL_TEST_FM_SCAN_WHOLE_BAND_TIME / (10 * ETAL_TEST_ONE_SECOND));
    OSAL_s32ThreadWait(ETAL_TEST_FM_SCAN_WHOLE_BAND_TIME/10);

    /* Check reported events */
    if ((count->STARTED != 1) ||
        (count->RESULT == 0))
    {
        etalTestPrintNormal("pass2a FAILED (%d and %d events, expected 1 and 0)", count->STARTED, count->RESULT);
        pass2 = FALSE;
    }

    etalTestPrintNormal("* Scan stop on initial frequency %d", initial_freq);
	ret = etaltml_scan_stop(hReceiver, initialFrequency);
    if (ret != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_scan_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    if(ETAL_TEST_MODE_HD_FM == test_type)
    {
        OSAL_s32ThreadWait(ETAL_TEST_SCAN_CALLBACK_WAIT_HD);
    }
    else
    {
        /* Wait some time to ensure the callback is invoked */
        OSAL_s32ThreadWait(ETAL_TEST_SCAN_CALLBACK_WAIT);
    }

    /* Check reported events */
    if (count->FINISHED != 1)
    {
        etalTestPrintNormal("pass2b FAILED");
        pass2 = FALSE;
    }
	/* check if the tuner frequency is the requested one */
	if ((ret = etal_get_receiver_frequency(hReceiver, &current_freq)) != ETAL_RET_SUCCESS)
	{
        etalTestPrintError("etal_get_receiver_frequency (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
	}
	if (current_freq != initial_freq)
	{
        etalTestPrintNormal("pass2c FAILED (initial freq %d, current freq %d)", initial_freq, current_freq);
        pass2 = FALSE;
	}

    if (!pass2)
    {
        etalTestPrintNormal("pass2 FAILED");
    }

    if (!pass1 || !pass2)
    {
        return OSAL_ERROR;
    }
    return OSAL_OK;
}
#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_HDRADIO_FM
#endif // CONFIG_APP_TEST_SCAN


/***************************
 *
 * etalTestScan
 *
 **************************/
tSInt etalTestScan(void)
{
#ifdef CONFIG_APP_TEST_SCAN
	tBool pass1 = TRUE;

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
	ETAL_HANDLE hReceiver;
	etalTestBroadcastTy test_type;
#endif

#if defined (CONFIG_APP_TEST_FM)
    ETAL_STATUS ret;
    EtalProcessingFeatures proc_features;
#endif //#if defined (CONFIG_APP_TEST_FM)

    etalTestStartup();

    if (etalTestScanSem == 0)
    {
        if (OSAL_s32SemaphoreCreate("Sem_etalTestScan", &etalTestScanSem, 0) == OSAL_ERROR)
    {
        return OSAL_ERROR;
        }
    }

#if defined (CONFIG_APP_TEST_FM)
	etalTestPrintNormal("<---Scan Test (sequential) start FM test");

	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* CONFIG_APP_TEST_FM && CONFIG_APP_TEST_HDRADIO_FM not yet supported */
	if (handlefm != ETAL_INVALID_HANDLE)
	{
		hReceiver = handlefm;
		test_type = ETAL_TEST_MODE_FM;

		/* Change to FMUE band */
	    etalTestPrintNormal("* Set FMUE band");
	    proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	    if ((ret = etal_change_band_receiver(hReceiver, ETAL_BAND_FMEU, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
	    {
	        etalTestPrintReportPassEnd(testFailed);
	        etalTestPrintError("etal_change_band_receiver FMUE band (%s, %d)", ETAL_STATUS_toString(ret), ret);
	        return OSAL_ERROR;
	    }
	}
	else
	{
        return OSAL_ERROR;
	}

	if (etalTestScanFMHD(hReceiver, test_type) != OSAL_OK)
	{
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestScan FAILED, handle %d", hReceiver);
        etalTestPrintNormal("****************************");
		pass1 = FALSE;
	}

	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //#if defined (CONFIG_APP_TEST_FMO)

#if defined (CONFIG_APP_TEST_HDRADIO_FM)

    etalTestPrintNormal("<---Scan Test (sequential) start HD FM test");

    if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    if (handlehd != ETAL_INVALID_HANDLE)
    {
        hReceiver = handlehd;
        test_type = ETAL_TEST_MODE_HD_FM;
    }
    else
    {
        return OSAL_ERROR;
    }

    if (etalTestScanFMHD(hReceiver, test_type) != OSAL_OK)
    {
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestScan FAILED, handle %d", hReceiver);
        etalTestPrintNormal("****************************");
        pass1 = FALSE;
    }

	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
#endif //#if defined (CONFIG_APP_TEST_HDRADIO_FM)

	if (!pass1)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_SCAN

	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

