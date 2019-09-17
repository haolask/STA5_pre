//!
//!  \file 		etaltest_tune.c
//!  \brief 	<i><b> ETAL test, tune </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

ETAL_HANDLE handledab, handlefm, handlefm2, handlehd, handlehdam, handlehd2, handleam, handledab1_5, handlehd1_5;

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
tU32 etalTestHDTuneEventCount;
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM)

#if defined (CONFIG_APP_TEST_HDRADIO_AM)
tU32 etalTestHDAMTuneEventCount;
#endif //defined (CONFIG_APP_TEST_HDRADIO_AM)

#define ETAL_TEST_FM_TUNE_SETTLE_TIME		 50
#define ETAL_TEST_AM_TUNE_SETTLE_TIME		 10
#define ETAL_TEST_HD_TUNE_SETTLE_TIME		  1
#define ETAL_TEST_SEEK_STEP_FREQ			 100
#define ETAL_TEST_GETFREQUENCY_COUNT		  10
#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	/* the round-trip delay for DCOP command with the EXTERNAL
	 * driver is influenced by the TCP/IP delay so it is
	 * much higher than the EMBEDDED driver case */
	#define ETAL_TEST_HD_TUNE_ASYNC_TIME		1000
#else
	#define ETAL_TEST_HD_TUNE_ASYNC_TIME		 100
#endif
/*
 * Some tests verify whether the callback is called;
 * since the callback is called from a separate thread it needs
 * some time to be executed, this is the max time to wait
 */
#define ETAL_TEST_DAB_SEEK_CALLBACK_WAIT	 100

#ifdef CONFIG_APP_TEST_CHANGEBAND
typedef enum {
	CHANGE_BAND_DAB_ILLEGAL_BAND = 1,
	CHANGE_BAND_DAB_ILLEGAL_RCV,
	CHANGE_BAND_DABL,
	CHANGE_BAND_DABL_TUNE,
	CHANGE_BAND_DAB3,
	CHANGE_BAND_DAB3_TUNE,
	CHANGE_BAND_ILLEGAL_BAND,
	CHANGE_BAND_ILLEGAL_RCV,
	CHANGE_BAND_ALTERNATE_BAND,
	CHANGE_BAND_TUNE_OUT_OF_BAND,
	CHANGE_BAND_USER_BAND,
	CHANGE_BAND_TUNE_OUT_OF_RANGE,
	CHANGE_BAND_NORMAL_BAND,
	CHANGE_BAND_TUNE_VALID
} ChangeBandTestTy;
#endif

#ifdef CONFIG_APP_TEST_TUNE_RECEIVER
typedef enum {
	TUNE_RCV_TUNE_FM_FREQ = 1,
	TUNE_RCV_TUNE_FM_EMPTY,
	TUNE_RCV_TUNE_FM_ILLEGAL,
	TUNE_RCV_TUNE_DAB_FREQ,
	TUNE_RCV_CREATE_FM_RCV,
	TUNE_RCV_TUNE_DAB_EMPTY,
	TUNE_RCV_TUNE_DAB_ILLEGAL,
	TUNE_RCV_TUNE_HDFM_FREQ,
	TUNE_RCV_TUNE_HDFM_EMPTY,
	TUNE_RCV_TUNE_HDFM_ASYNC,
	TUNE_RCV_HDFM_CHECK_DELAY,
	TUNE_RCV_HDFM_CHECK_EVENTS_AUDIO,
	TUNE_RCV_HDFM_CHECK_EVENTS,
	TUNE_RCV_TUNE_HDFM_ILLEGAL,
	TUNE_RCV_TUNE_HDAM_FREQ,
	TUNE_RCV_TUNE_HDAM_EMPTY,
	TUNE_RCV_TUNE_HDAM_ASYNC,
	TUNE_RCV_HDAM_CHECK_DELAY,
	TUNE_RCV_HDAM_CHECK_EVENTS_AUDIO,
	TUNE_RCV_HDAM_CHECK_EVENTS,
	TUNE_RCV_TUNE_HDAM_ILLEGAL,
	TUNE_RCV_TUNE_AM_FREQ,
	TUNE_RCV_TUNE_AM_EMPTY,
	TUNE_RCV_TUNE_AM_ILLEGAL
} TuneReceiverTestTy;
#endif

#ifdef CONFIG_APP_TEST_GETFREQUENCY
typedef enum {
	GET_FREQ_TUNE = 1,
	GET_FREQ_TUNE_EMPTY,
	GET_FREQ_TUNE_ILLEGAL,
	GET_FREQ_GET_DURING_SEEK
} GetFrequencyTestTy;
#endif


/***************************
 * external functions
 **************************/
tBool ETAL_isValidFrequency(ETAL_HANDLE hReceiver, tU32 f);

/***************************
 *
 * etalTestDoTuneSingle
 *
 **************************/
/*
 * Tunes to a frequency depending on the configuration
 *
 * Returns:
 *  OSAL_OK if no error
 *  OSAL_ERROR otherwise
 */
tSInt etalTestDoTuneSingle(etalTestTuneTy conf, ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;
	EtalAudioSourceTy audioSource;
	tBool do_tune = FALSE;

	if (hReceiver != ETAL_INVALID_HANDLE)
	{
		switch (conf)
		{
			case ETAL_TUNE_NONE:
				break;

			case ETAL_TUNE_DAB:
				etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ);
				if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
				{
					etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
					return OSAL_ERROR;
				}
				audioSource = ETAL_AUDIO_SOURCE_DCOP_STA660;
				do_tune = TRUE;
				break;

			case ETAL_TUNE_DAB2:
				etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_OTHER_FREQ);
				if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_OTHER_FREQ)) != ETAL_RET_SUCCESS)
				{
					etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
					return OSAL_ERROR;
				}
				audioSource = ETAL_AUDIO_SOURCE_DCOP_STA660;
				do_tune = TRUE;
				break;

			case ETAL_TUNE_FM:
#ifdef CONFIG_APP_TEST_FM
				etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
				if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
				{
					etalTestPrintError("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
					return OSAL_ERROR;
				}

				audioSource = ETAL_AUDIO_SOURCE_STAR_AMFM;
				do_tune = TRUE;
#endif //CONFIG_APP_TEST_FM
				break;

			case ETAL_TUNE_AM:
#ifdef CONFIG_APP_TEST_AM
				etalTestPrintNormal("* Tune to AM freq %d", ETAL_VALID_AM_FREQ);
				if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_AM_FREQ)) != ETAL_RET_SUCCESS)
				{
					etalTestPrintError("etal_tune_receiver AM (%s, %d)", ETAL_STATUS_toString(ret), ret);
					return OSAL_ERROR;
				}

				audioSource = ETAL_AUDIO_SOURCE_STAR_AMFM;
				do_tune = TRUE;
#endif //CONFIG_APP_TEST_AM
				break;

			case ETAL_TUNE_HDRADIO_FM:
#ifdef CONFIG_APP_TEST_HDRADIO_FM
				etalTestPrintNormal("* Tune to HD FM freq %d", ETAL_VALID_HD_FREQ);
				if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_HD_FREQ)) != ETAL_RET_SUCCESS)
				{
					etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
					return OSAL_ERROR;
				}

				audioSource = ETAL_AUDIO_SOURCE_AUTO_HD;
				do_tune = TRUE;
#endif //CONFIG_APP_TEST_HDRADIO_FM
				break;

			case ETAL_TUNE_HDRADIO_AM:
#ifdef CONFIG_APP_TEST_HDRADIO_AM
				etalTestPrintNormal("* Tune to HD AM freq %d", ETAL_VALID_HD_AM_FREQ);
				if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_HD_AM_FREQ)) != ETAL_RET_SUCCESS)
				{
					etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
					return OSAL_ERROR;
				}

				audioSource = ETAL_AUDIO_SOURCE_AUTO_HD;
				do_tune = TRUE;
#endif //CONFIG_APP_TEST_HDRADIO_AM
				break;

			default:
				ASSERT_ON_DEBUGGING(0);
				break;
		}

		if (do_tune)
		{
			/* Select audio source */
			if ((ret = etal_audio_select(hReceiver, audioSource)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_audio_select (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestUndoTuneSingle
 *
 **************************/
/*
 * Tunes to an empty frequency
 *
 * Returns:
 *  OSAL_OK if no error
 *  OSAL_ERROR otherwise
 */
tSInt etalTestUndoTuneSingle(etalTestTuneTy conf, ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret, exp_ret;
	tBool do_tune = FALSE;
	tU32 freq;

	exp_ret = ETAL_RET_SUCCESS;

	if(hReceiver != ETAL_INVALID_HANDLE)
	{
		switch(conf)
		{
			case ETAL_TUNE_NONE:
				break;
			case ETAL_TUNE_DAB:
			case ETAL_TUNE_DAB2:
				freq = ETAL_EMPTY_DAB_FREQ;
				etalTestPrintNormal("* Tune to empty DAB freq %d", freq);
				do_tune = TRUE;
				exp_ret = ETAL_RET_NO_DATA;
				break;
			case ETAL_TUNE_DRM:
				freq = ETAL_EMPTY_DRM_FREQ;
				etalTestPrintNormal("* Tune to empty DRM freq %d", freq);
				do_tune = TRUE;
				break;
			case ETAL_TUNE_FM:
				freq = ETAL_EMPTY_FM_FREQ;
				etalTestPrintNormal("* Tune to empty FM freq %d", freq);
				do_tune = TRUE;
				break;
			case ETAL_TUNE_AM:
				freq = ETAL_EMPTY_AM_FREQ;
				etalTestPrintNormal("* Tune to empty AM freq %d", freq);
				do_tune = TRUE;
				break;
			case ETAL_TUNE_HDRADIO_FM:
				freq = ETAL_EMPTY_FM_FREQ;
				etalTestPrintNormal("* Tune to empty HDFM freq %d", freq);
				do_tune = TRUE;
				exp_ret = ETAL_RET_NO_DATA;
				break;
			case ETAL_TUNE_HDRADIO_AM:
				freq = ETAL_EMPTY_AM_FREQ;
				etalTestPrintNormal("* Tune to empty HDAM freq %d", freq);
				do_tune = TRUE;
				exp_ret = ETAL_RET_NO_DATA;
				break;
		}

		if(do_tune)
		{
			if ((ret = etal_tune_receiver(hReceiver, freq)) != exp_ret)
			{
				etalTestPrintError("etal_tune_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
		}
	}
	return OSAL_OK;
}


#ifdef CONFIG_APP_TEST_CHANGEBAND
#if defined (CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_AM) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
/***************************
 *
 * etalTestChangeBand_Internal
 *
 **************************/
static tSInt etalTestChangeBand_Internal(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tU32 pass_id, tBool *pass_l)
{
	tSInt ret = OSAL_OK;
	tBool pass = TRUE;
	EtalProcessingFeatures proc_features;
	EtalFrequencyBand good_band, alt_band, bad_band, user_band;
	tU32 good_freq, outofband_freq, userlow_freq, userhigh_freq;
	ETAL_STATUS userband_ret;
	etalTestConfigTy test_config;
	etalTestTuneTy test_tune;

	/* Set up variables defining the test behaviour dependent on the test mode:
	 *
	 * test_config = the test configuration passed to etalTestDoConfigSingle
	 * test_tune   = the tune configuration passed to etalTestDoTuneSingle
	 * good_band   = the default band used for the hReceiver Broadcast standard
	 * alt_band    = an alternate band, valid for the hReceiver Broadcast standard
	 *               ideally should be a band with frequency limits diffrerent from the
	 *               good_band's limits. This is not possible for HD FM
	 * bad_band    = a band not allowed for hReceiver's Broadcast standard
	 * user_band   = for AM/FM receiver, the USERFM or USERAM bands which allow
	 *               defining custom band limits
	 * good_freq      = a valid frequency
	 * outofband_freq = a frequency outside of alt_band limits
	 * userlow_band   = the lower limit for user_band; 0 if hRceiver is not AM or FM
	 * userhigh_band  = the upper limit for user_band; 0 if hRceiver is not AM or FM
	 * userband_ret   = the return value expected on change band to USER band
	 */
	switch (mode)
	{
		case ETAL_TEST_MODE_FM:
			test_config = ETAL_CONFIG_FM1;
			test_tune = ETAL_TUNE_FM;
			good_band = ETAL_BAND_FM;
			alt_band = ETAL_BAND_FMEEU;
			bad_band = ETAL_BAND_DAB3;
			user_band = ETAL_BAND_USERFM;
			good_freq = ETAL_VALID_FM_FREQ;
			outofband_freq = 100000;
			userlow_freq = 95000;
			userhigh_freq = 100000;
			userband_ret = ETAL_RET_SUCCESS;
			break;

		case ETAL_TEST_MODE_AM:
			test_config = ETAL_CONFIG_AM;
			test_tune = ETAL_TUNE_AM;
			good_band = ETAL_BAND_AM;
			alt_band = ETAL_BAND_SW;
			bad_band = ETAL_BAND_DAB3;
			user_band = ETAL_BAND_USERAM;
			good_freq = ETAL_VALID_AM_FREQ;
			outofband_freq = 5000;
			userlow_freq = ETAL_VALID_AM_FREQ_LOW;
			userhigh_freq = ETAL_VALID_AM_FREQ_HIGH;
			userband_ret = ETAL_RET_SUCCESS;
			break;

		case ETAL_TEST_MODE_HD_FM:
			test_config = ETAL_CONFIG_HDRADIO_FM;
			test_tune = ETAL_TUNE_HDRADIO_FM;
			good_band = ETAL_BAND_HD;
			alt_band = ETAL_BAND_FMUS;
			bad_band = ETAL_BAND_DAB3;
			user_band = ETAL_BAND_USERFM;
			good_freq = ETAL_VALID_HD_FREQ;
			outofband_freq = 108000;
			userlow_freq = 0;
			userhigh_freq = 0;
			userband_ret = ETAL_RET_PARAMETER_ERR;
			break;

		case ETAL_TEST_MODE_HD_AM:
			test_config = ETAL_CONFIG_HDRADIO_AM;
			test_tune = ETAL_TUNE_HDRADIO_AM;
			good_band = ETAL_BAND_MWUS;
			alt_band = ETAL_BAND_MWUS;
			bad_band = ETAL_BAND_DAB3;
			user_band = ETAL_BAND_USERAM;
			good_freq = ETAL_VALID_HD_AM_FREQ;
			outofband_freq = 5000;
			userlow_freq = 0;
			userhigh_freq = 0;
			userband_ret = ETAL_RET_PARAMETER_ERR;
			break;

		default:
			ret = OSAL_ERROR;
			break;
	}

	if (ret == OSAL_OK)
	{
		if (etalTestDoConfigSingle(test_config, &hReceiver) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
		else if (etalTestDoTuneSingle(test_tune, hReceiver) != OSAL_OK)
		{
			ret = OSAL_ERROR;
		}
	}
	if (ret != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* verify wrong parameter processing */

	etalTestPrintReportPassStart(CHANGE_BAND_ILLEGAL_BAND, mode, "Change Band with illegal band");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(hReceiver, bad_band, 0, 0, proc_features) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_ILLEGAL_RCV, mode, "Change Band on illegal receiver");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(ETAL_INVALID_HANDLE, good_band, 0, 0, proc_features) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%db FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_ALTERNATE_BAND, mode, "Change Band to alternate valid band");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(hReceiver, alt_band, 0, 0, proc_features) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dc FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Try tune to a frequency outside of the range defined by the
	 * last etal_change_band_receiver and expect error, to ensure the command was
	 * effective */
	etalTestPrintReportPassStart(CHANGE_BAND_TUNE_OUT_OF_BAND, mode, "Tune to out of band frequency");
	if (etal_tune_receiver(hReceiver, outofband_freq) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dd FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* USER is only supported for AM and FM, expect error in other standards */
	etalTestPrintReportPassStart(CHANGE_BAND_USER_BAND, mode, "Change Band to USER band");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(hReceiver, user_band, userlow_freq, userhigh_freq, proc_features) != userband_ret)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%de FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* only for receivers where change band to USER is allowed (AM and FM) */
	if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_AM))
	{
		etalTestPrintReportPassStart(CHANGE_BAND_TUNE_OUT_OF_RANGE, mode, "Tune outside of range");
		if (etal_tune_receiver(hReceiver, userhigh_freq + 1000) != ETAL_RET_PARAMETER_ERR)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%df FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		etalTestPrintReportPassStart(CHANGE_BAND_NORMAL_BAND, mode, "Change Band to normal (non USER) band");
		proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if (etal_change_band_receiver(hReceiver, good_band, 0, 0, proc_features) != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%dg FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		etalTestPrintReportPassStart(CHANGE_BAND_TUNE_VALID, mode, "Tune to valid frequency");
		if (etal_tune_receiver(hReceiver, good_freq) != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%dh FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	if (etalTestUndoTuneSingle(test_tune, hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass)
	{
		*pass_l = FALSE;
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_AM || CONFIG_APP_TEST_HDRADIO_FM
#endif // CONFIG_APP_TEST_CHANGEBAND

/***************************
 *
 * etalTestChangeBand
 *
 **************************/
tSInt etalTestChangeBand(void)
{
#ifdef CONFIG_APP_TEST_CHANGEBAND
	tBool pass;
#if defined (CONFIG_APP_TEST_DAB)
	EtalProcessingFeatures proc_features;
#endif //defined (CONFIG_APP_TEST_DAB)

	etalTestStartup();
	pass = TRUE;

#if defined (CONFIG_APP_TEST_DAB)
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* pass1, verify wrong parameter processing for DAB */

	etalTestPrintReportPassStart(CHANGE_BAND_DAB_ILLEGAL_BAND, ETAL_TEST_MODE_DAB, "Change Band with illegal band");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(handledab, ETAL_BAND_FM, 0, 0, proc_features) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_DAB_ILLEGAL_RCV, ETAL_TEST_MODE_DAB, "Change Band on illegal receiver");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(ETAL_INVALID_HANDLE, ETAL_BAND_DAB3, 0, 0, proc_features) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_DABL, ETAL_TEST_MODE_DAB, "Change Band to DABL");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(handledab, ETAL_BAND_DABL, 0, 0, proc_features) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_DABL_TUNE, ETAL_TEST_MODE_DAB, "Tune to DAB3 frequency");
	if (etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_DAB3, ETAL_TEST_MODE_DAB, "Change Band to DAB3");
	proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if (etal_change_band_receiver(handledab, ETAL_BAND_DAB3, 0, 0, proc_features) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1e FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CHANGE_BAND_DAB3_TUNE, ETAL_TEST_MODE_DAB, "Tune to DAB3 frequency");
	if (etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1f FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined (CONFIG_APP_TEST_DAB)

#if defined (CONFIG_APP_TEST_FM)
	if (etalTestChangeBand_Internal(handlefm, ETAL_TEST_MODE_FM, 2, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined (CONFIG_APP_TEST_FM)

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	if (etalTestChangeBand_Internal(handlehd, ETAL_TEST_MODE_HD_FM, 3, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM)

#if defined (CONFIG_APP_TEST_HDRADIO_AM)
	if (etalTestChangeBand_Internal(handlehdam, ETAL_TEST_MODE_HD_AM, 3, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined (CONFIG_APP_TEST_HDRADIO_AM)

#if defined(CONFIG_APP_TEST_AM)
	if (etalTestChangeBand_Internal(handleam, ETAL_TEST_MODE_AM, 4, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined(CONFIG_APP_TEST_AM)

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_CHANGEBAND
	return OSAL_OK;
}

/***************************
 *
 * etalTestTuneReceiver
 *
 **************************/
tSInt etalTestTuneReceiver(void)
{
#ifdef CONFIG_APP_TEST_TUNE_RECEIVER
#if defined(CONFIG_APP_TEST_HDRADIO_FM) || defined(CONFIG_APP_TEST_HDRADIO_AM)
	OSAL_tMSecond start_time;
#endif
	ETAL_STATUS ret;
#ifdef CONFIG_APP_TEST_DAB
	ETAL_STATUS ret_time;
	EtalTime ETALTime, time_tmp;
#endif
	tBool pass;
#if defined(CONFIG_APP_TEST_DAB) && defined(CONFIG_APP_TEST_FM)
	EtalReceiverAttr attr;
#endif

	etalTestStartup();
	pass = TRUE;

#ifdef CONFIG_APP_TEST_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Tune to valid FM freq */
	etalTestPrintReportPassStart(TUNE_RCV_TUNE_FM_FREQ, ETAL_TEST_MODE_FM, "Tune to freq %d", ETAL_VALID_FM_FREQ);
	if ((ret = etal_tune_receiver(handlefm, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Select audio source */
	if ((ret = etal_audio_select(handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Listen to some radio (5sec)");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (25)");
	etal_audio_output_scaling(25);
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (24)");
	etal_audio_output_scaling(24);
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (18)");
    etal_audio_output_scaling(18);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (12)");
	etal_audio_output_scaling(12);
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (6)");
        etal_audio_output_scaling(6);
        OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (0)");
	etal_audio_output_scaling(0);
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (6)");
	    etal_audio_output_scaling(6);
	    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (12)");
	etal_audio_output_scaling(12);
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (18)");
    etal_audio_output_scaling(18);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("Listen to some audio (1sec) and change volume (24)");
	etal_audio_output_scaling(24);
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	/* Tune to invalid FM freq */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_FM_EMPTY, ETAL_TEST_MODE_FM, "Tune to empty freq (%d)", ETAL_EMPTY_FM_FREQ);
	if ((ret = etal_tune_receiver(handlefm, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Tune the FM receiver to a DAB frequency (expect error) */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_FM_ILLEGAL, ETAL_TEST_MODE_FM, "Tune to illegal DAB freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handlefm, ETAL_VALID_DAB_FREQ)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		if (etalTestOption.oStopOnErrors)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy the receivers */
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	ret_time = etal_get_time(handledab, &ETALTime);
	etalTestPrintNormal("etal_get_time (%s, %d)", ETAL_STATUS_toString(ret_time), ret_time);
	etalTestPrintNormal("Time: isVal: %d, mjd: %d, h: %d, min: %d, s: %d, ms: %d, isLtoVal; %d, lto:%d",
			ETALTime.m_isTimeValid, ETALTime.m_mjd, ETALTime.m_hours, ETALTime.m_minutes, ETALTime.m_seconds,
			ETALTime.m_milliseconds, ETALTime.m_isLtoValid, ETALTime.m_lto);

	if(ret_time != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}

	/* Tune to valid DAB freq */
	etalTestPrintReportPassStart(TUNE_RCV_TUNE_DAB_FREQ, ETAL_TEST_MODE_DAB, "* Tune freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		OSAL_s32ThreadWait(5000);

		ret_time = etal_get_time(handledab, &ETALTime);
		time_tmp = ETALTime;
		etalTestPrintNormal("etal_get_time (%s, %d)", ETAL_STATUS_toString(ret_time), ret_time);
		etalTestPrintNormal("Time: isVal: %d, mjd: %d, h: %d, min: %d, s: %d, ms: %d, isLtoVal; %d, lto:%d",
				ETALTime.m_isTimeValid, ETALTime.m_mjd, ETALTime.m_hours, ETALTime.m_minutes, ETALTime.m_seconds,
				ETALTime.m_milliseconds, ETALTime.m_isLtoValid, ETALTime.m_lto);

		if(ret_time != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
		}

		OSAL_s32ThreadWait(5000);

		ret_time = etal_get_time(handledab, &ETALTime);
		etalTestPrintNormal("etal_get_time (%s, %d)", ETAL_STATUS_toString(ret_time), ret_time);
		etalTestPrintNormal("Time: isVal: %d, mjd: %d, h: %d, min: %d, s: %d, ms: %d, isLtoVal; %d, lto:%d",
				ETALTime.m_isTimeValid, ETALTime.m_mjd, ETALTime.m_hours, ETALTime.m_minutes, ETALTime.m_seconds,
				ETALTime.m_milliseconds, ETALTime.m_isLtoValid, ETALTime.m_lto);

		if((ret_time != ETAL_RET_SUCCESS) &&
		   (((ETALTime.m_seconds - time_tmp.m_seconds) > 6) || ((ETALTime.m_seconds - time_tmp.m_seconds) < 4)))
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	/*
	 * depending on the test sequence FIC data may not be already available
	 */
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	if (etalTestDoServiceSelectAudio(handledab, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID))
	{
		return OSAL_ERROR;
	}
	/* Select audio source */
	if ((ret = etal_audio_select(handledab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Listen to some DAB audio (5sec)");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);


	// add-on :  receiver reconfiguration impact audio check
	// a receiver reconfiguration should not impact audio source
	// configure an FM receiver

#if defined (CONFIG_APP_TEST_FM)

	etalTestPrintReportPassStart(TUNE_RCV_CREATE_FM_RCV, ETAL_TEST_MODE_FM, "Create a FM receiver");
	handlefm = ETAL_INVALID_HANDLE;
	OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));
	
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if ((ret = etal_config_receiver(&handlefm, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver: step for fm reconfig failed (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	//modify the receiver
	// 
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if ((ret = etal_config_receiver(&handlefm, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	
	etalTestPrintNormal("Listen to some DAB audio (5sec)");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	
	etalTestPrintNormal("* Destroy FM receiver");
	if ((ret = etal_destroy_receiver(&handlefm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	handlefm = ETAL_INVALID_HANDLE;
#endif

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (0)");
    etal_audio_output_scaling(0);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (6)");
    etal_audio_output_scaling(6);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (12)");
    etal_audio_output_scaling(12);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (18)");
    etal_audio_output_scaling(18);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (24)");
    etal_audio_output_scaling(24);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (18)");
    etal_audio_output_scaling(18);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (12)");
    etal_audio_output_scaling(12);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (6)");
    etal_audio_output_scaling(6);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

    etalTestPrintNormal("Listen to some audio (1sec) and change volume (0)");
    etal_audio_output_scaling(0);
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	/* Tune to invalid DAB freq */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_DAB_EMPTY, ETAL_TEST_MODE_DAB,"Tune to empty freq (%d)", ETAL_EMPTY_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Tune the DAB receiver to an FM frequency (expect error) */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_DAB_ILLEGAL, ETAL_TEST_MODE_DAB, "Tune to illegal FM freq (%d)", ETAL_VALID_FM_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_FM_FREQ)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy the receivers */
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

	/* Tune to valid HD freq */
	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDFM_FREQ, ETAL_TEST_MODE_HD_FM, "Tune to freq %d", ETAL_VALID_HD_FREQ);
	if ((ret = etal_tune_receiver(handlehd, ETAL_VALID_HD_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Select audio source */
	if ((ret = etal_audio_select(handlehd, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Listen to some radio (5sec)");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Tune to invalid HD freq */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDFM_EMPTY, ETAL_TEST_MODE_HD_FM, "Tune to empty freq (%d)", ETAL_EMPTY_FM_FREQ);
	if ((ret = etal_tune_receiver(handlehd, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_NO_DATA)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Tune again using the async interface */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDFM_ASYNC, ETAL_TEST_MODE_HD_FM, "Tune async to freq %d", ETAL_VALID_HD_FREQ);
	start_time = OSAL_ClockGetElapsedTime();
	etalTestHDTuneEventCount = 0;
	if ((ret = etal_tune_receiver_async(handlehd, ETAL_VALID_HD_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(TUNE_RCV_HDFM_CHECK_DELAY, ETAL_TEST_MODE_HD_FM, "Check tune async delay less than %dmsec", ETAL_TEST_HD_TUNE_ASYNC_TIME);
	if (OSAL_ClockGetElapsedTime() - start_time > ETAL_TEST_HD_TUNE_ASYNC_TIME)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (ETAL_FE_FOR_HD_TEST == ETAL_FE_HANDLE_1)
	{
		/*
		 * HD audio available on channel 1 (or first instance) only
		 */
		etalTestPrintReportPassStart(TUNE_RCV_HDFM_CHECK_EVENTS_AUDIO, ETAL_TEST_MODE_HD_FM, "Check tune async events, with audio");
		etalTestPrintNormal("* Listen to some radio (7sec)");
		OSAL_s32ThreadWait(7 * ETAL_TEST_ONE_SECOND);
		if (etalTestHDTuneEventCount < 2)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass1b FAILED (HD audio)");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	else
	{
		etalTestPrintReportPassStart(TUNE_RCV_HDFM_CHECK_EVENTS, ETAL_TEST_MODE_HD_FM, "Check tune async events, without audio");
		etalTestPrintNormal("Allow some time for HD acquisition (1s)");
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND); // 800ms + 70ms (communication overhead) + ? (does not work with 900)
		if (etalTestHDTuneEventCount < 1)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass1c FAILED (HD data)");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	/* Tune the HD receiver to a DAB frequency (expect error) */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDFM_ILLEGAL, ETAL_TEST_MODE_HD_FM, "Tune to illegal DAB freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handlehd, ETAL_VALID_DAB_FREQ)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy the receivers */
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Tune to valid HD freq */
	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDAM_FREQ, ETAL_TEST_MODE_HD_AM, "Tune to freq %d", ETAL_VALID_HD_AM_FREQ);
	if ((ret = etal_tune_receiver(handlehdam, ETAL_VALID_HD_AM_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Select audio source */
	if ((ret = etal_audio_select(handlehdam, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Listen to some radio (5sec)");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Tune to invalid HD freq */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDAM_EMPTY, ETAL_TEST_MODE_HD_AM, "Tune to empty freq (%d)", ETAL_EMPTY_AM_FREQ);
	if ((ret = etal_tune_receiver(handlehdam, ETAL_EMPTY_AM_FREQ)) != ETAL_RET_NO_DATA)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Tune again using the async interface */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDAM_ASYNC, ETAL_TEST_MODE_HD_AM, "Tune async to freq %d", ETAL_VALID_HD_AM_FREQ);
	start_time = OSAL_ClockGetElapsedTime();
	etalTestHDAMTuneEventCount = 0;
	if ((ret = etal_tune_receiver_async(handlehdam, ETAL_VALID_HD_AM_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(TUNE_RCV_HDAM_CHECK_DELAY, ETAL_TEST_MODE_HD_AM, "Check tune async delay less than %dmsec", ETAL_TEST_HD_TUNE_ASYNC_TIME);
	if (OSAL_ClockGetElapsedTime() - start_time > ETAL_TEST_HD_TUNE_ASYNC_TIME)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (ETAL_FE_FOR_HD_TEST == ETAL_FE_HANDLE_1)
	{
		/*
		 * HD audio available on channel 1 (or first instance) only
		 */
		etalTestPrintReportPassStart(TUNE_RCV_HDAM_CHECK_EVENTS_AUDIO, ETAL_TEST_MODE_HD_AM, "Check tune async events, with audio");
		etalTestPrintNormal("* Listen to some radio (7sec)");
		OSAL_s32ThreadWait(7 * ETAL_TEST_ONE_SECOND);
		if (etalTestHDAMTuneEventCount < 2)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2b FAILED (HD audio)");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	else
	{
		etalTestPrintReportPassStart(TUNE_RCV_HDAM_CHECK_EVENTS, ETAL_TEST_MODE_HD_AM, "Check tune async events, without audio");
		etalTestPrintNormal("Allow some time for HD acquisition (1s)");
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND); // 800ms + 70ms (communication overhead) + ? (does not work with 900)
		if (etalTestHDAMTuneEventCount < 1)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2c FAILED (HD data)");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	/* Tune the HD receiver to a DAB frequency (expect error) */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_HDAM_ILLEGAL, ETAL_TEST_MODE_HD_AM, "Tune to illegal DAB freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handlehdam, ETAL_VALID_DAB_FREQ)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy the receivers */
	if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_AM

	/*
	 * On the CMOST the FM and AM receivers share the same frontend
	 * so the AM tests must be done after the FM receiver is destroyed
	 * otherwise ETAL will complain
	 */

#ifdef CONFIG_APP_TEST_AM

	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_AM_FREQ, ETAL_TEST_MODE_AM, "Tune to freq %d", ETAL_VALID_AM_FREQ);
	if ((ret = etal_tune_receiver(handleam, ETAL_VALID_AM_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver AM (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Select audio source */
	if ((ret = etal_audio_select(handleam, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select AM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Listen to some radio (5sec)");
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Tune to invalid AM freq */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_AM_EMPTY, ETAL_TEST_MODE_AM, "Tune to empty freq (%d)", ETAL_EMPTY_AM_FREQ);
	if ((ret = etal_tune_receiver(handleam, ETAL_EMPTY_AM_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver AM (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Tune the AM receiver to a DAB frequency (expect error) */

	etalTestPrintReportPassStart(TUNE_RCV_TUNE_AM_ILLEGAL, ETAL_TEST_MODE_AM, "Tune to illegal DAB freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handleam, ETAL_VALID_DAB_FREQ)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver AM (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_AM

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_TUNE_RECEIVER
	return OSAL_OK;
}

#if defined(CONFIG_APP_TEST_DAB) || defined(CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_AM) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
#ifdef CONFIG_APP_TEST_GETFREQUENCY
/***************************
 *
 * etalTestGetFrequencySimple
 *
 **************************/
static tSInt etalTestGetFrequencySimple(etalTestBroadcastTy mode, tU8 step, tBool *pass_out)
{
	ETAL_STATUS ret, intermediate_ret;
	ETAL_HANDLE hReceiver;
	tU32 freq, good_freq, empty_freq, invalid_freq;
	tU32 settle_time;
	tChar band_string[10];

	switch (mode)
	{
		case ETAL_TEST_MODE_DAB:
			hReceiver = handledab;
			good_freq = ETAL_VALID_DAB_FREQ;
			empty_freq = ETAL_EMPTY_DAB_FREQ;
			invalid_freq = ETAL_VALID_FM_FREQ;
			settle_time = ETAL_TEST_DAB_TUNE_SETTLE_TIME;
			OSAL_szStringCopy(band_string, "DAB");
			intermediate_ret = ETAL_RET_NO_DATA;
			break;

		case ETAL_TEST_MODE_FM:
			hReceiver = handlefm;
			good_freq = ETAL_VALID_FM_FREQ;
			empty_freq = ETAL_EMPTY_FM_FREQ;
			invalid_freq = ETAL_VALID_DAB_FREQ;
			settle_time = ETAL_TEST_FM_TUNE_SETTLE_TIME;
			OSAL_szStringCopy(band_string, "FM");
			intermediate_ret = ETAL_RET_SUCCESS;
			break;

		case ETAL_TEST_MODE_HD_FM:
			hReceiver = handlehd;
			good_freq = ETAL_VALID_HD_FREQ;
			empty_freq = ETAL_EMPTY_FM_FREQ;
			invalid_freq = ETAL_VALID_DAB_FREQ;
			settle_time = ETAL_TEST_HD_TUNE_SETTLE_TIME;
			OSAL_szStringCopy(band_string, "HD FM");
			intermediate_ret = ETAL_RET_NO_DATA;
			break;

		case ETAL_TEST_MODE_HD_AM:
			hReceiver = handlehdam;
			good_freq = ETAL_VALID_HD_AM_FREQ;
			empty_freq = ETAL_EMPTY_AM_FREQ;
			invalid_freq = ETAL_VALID_DAB_FREQ;
			settle_time = ETAL_TEST_HD_TUNE_SETTLE_TIME;
			OSAL_szStringCopy(band_string, "HD AM");
			intermediate_ret = ETAL_RET_NO_DATA;
			break;

		case ETAL_TEST_MODE_AM:
			hReceiver = handleam;
			good_freq = ETAL_VALID_AM_FREQ;
			empty_freq = ETAL_EMPTY_AM_FREQ;
			invalid_freq = ETAL_VALID_DAB_FREQ;
			settle_time = ETAL_TEST_AM_TUNE_SETTLE_TIME;
			OSAL_szStringCopy(band_string, "AM");
			intermediate_ret = ETAL_RET_SUCCESS;
			break;

		default:
			ASSERT_ON_DEBUGGING(0);
	}

	/* passXa: Tune to valid freq */

	etalTestPrintReportPassStart(GET_FREQ_TUNE, mode, "Tune to freq %d", good_freq);
	if ((ret = etal_tune_receiver(hReceiver, good_freq)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		etalTestPrintReportPassEnd(testFailed);
		return OSAL_ERROR;
	}
	OSAL_s32ThreadWait(settle_time);
	if ((ret = etal_get_receiver_frequency(hReceiver, &freq)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_receiver_frequency %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		etalTestPrintReportPassEnd(testFailed);
		return OSAL_ERROR;
	}

	if (freq != good_freq)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", step);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* passXb: Tune to invalid freq */

	etalTestPrintReportPassStart(GET_FREQ_TUNE_EMPTY, mode, "Tune to empty freq (%d)", empty_freq);
	if ((ret = etal_tune_receiver(hReceiver, empty_freq)) != intermediate_ret)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		etalTestPrintReportPassEnd(testFailed);
		return OSAL_ERROR;
	}
	OSAL_s32ThreadWait(settle_time);
	if ((ret = etal_get_receiver_frequency(hReceiver, &freq)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_receiver_frequency %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		etalTestPrintReportPassEnd(testFailed);
		return OSAL_ERROR;
	}
	if (freq != empty_freq)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%db FAILED", step);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* passXc: Tune the receiver to a invalid frequency (expect error and no change in frequency) */

	etalTestPrintReportPassStart(GET_FREQ_TUNE_ILLEGAL, mode, "Tune to illegal freq %d", invalid_freq);
	if ((ret = etal_tune_receiver(hReceiver, invalid_freq)) != ETAL_RET_PARAMETER_ERR)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		etalTestPrintReportPassEnd(testFailed);
		return OSAL_ERROR;
	}
	OSAL_s32ThreadWait(settle_time);
	if ((ret = etal_get_receiver_frequency(hReceiver, &freq)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_receiver_frequency %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		etalTestPrintReportPassEnd(testFailed);
		return OSAL_ERROR;
	}
	if (freq != empty_freq)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dc FAILED", step);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_GETFREQUENCY
#endif // CONFIG_APP_TEST_DAB || CONFIG_APP_TEST_FM || CONFIG_APP_TEST_AM || CONFIG_APP_TEST_HDRADIO_FM || CONFIG_APP_TEST_HDRADIO_AM

#if defined (CONFIG_APP_TEST_FM) && defined (CONFIG_APP_TEST_GETFREQUENCY)
/***************************
 *
 * etalTestGetFrequencySeek
 *
 **************************/
static tSInt etalTestGetFrequencySeek(etalTestBroadcastTy mode, tBool *pass_out)
{
	ETAL_STATUS ret;
	ETAL_HANDLE hReceiver;
	tS32 good_freq;
	tU32 freq;
	tChar band_string[10];
	tU32 count, i;

	switch (mode)
	{
		case ETAL_TEST_MODE_FM:
			hReceiver = handlefm;
			good_freq = ETAL_VALID_FM_FREQ;
			OSAL_szStringCopy(band_string, "FM");
			break;
		default:
			ASSERT_ON_DEBUGGING(0);
	}

	etalTestPrintNormal("* Tune to %s freq %d", band_string, good_freq);
	if ((ret = etal_tune_receiver(hReceiver, good_freq)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Seek up with fixed frequency step */

	count = 0;
	while (count < ETAL_TEST_GETFREQUENCY_COUNT)
	{
		if ((ret = etal_get_receiver_frequency(hReceiver, &freq)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_get_receiver_frequency %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

		if (!ETAL_isValidFrequency(hReceiver, freq))
		{
			etalTestPrintNormal("curr freq=%d INVALID, aborting test", freq);
			*pass_out = FALSE;
			break;
		}

		etalTestPrintNormal("* Seek up starting from current frequency %d, %dKHz step", freq, ETAL_TEST_SEEK_STEP_FREQ);
		if ((ret = etal_autoseek_start(hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		for (i = 0; i < 5; i++)
		{
			OSAL_s32ThreadWait(100);
			etalTestPrintReportPassStart(GET_FREQ_GET_DURING_SEEK, mode, "Get frequency during seek, pass %d", i);
			if ((ret = etal_get_receiver_frequency(hReceiver, &freq)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_get_receiver_frequency %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
			if (!ETAL_isValidFrequency(hReceiver, freq))
			{
				*pass_out = FALSE;
				etalTestPrintNormal("* curr freq=%d INVALID", freq);
				etalTestPrintReportPassEnd(testFailed);
			}
			else
			{
				etalTestPrintNormal("* curr freq=%d", freq);
				etalTestPrintReportPassEnd(testPassed);
			}
		}
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop %s (%s, %d)", band_string, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		// listen to some audio
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
		count++;
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_FM && CONFIG_APP_TEST_GETFREQUENCY 

/***************************
 *
 * etalTestGetFrequency
 *
 **************************/
tSInt etalTestGetFrequency(void)
{
#ifdef CONFIG_APP_TEST_GETFREQUENCY
	tBool pass;

	etalTestStartup();
	pass = TRUE;

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestGetFrequencySimple(ETAL_TEST_MODE_DAB, 1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestGetFrequencySimple(ETAL_TEST_MODE_FM, 2, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass2 FAILED");
		return OSAL_ERROR;
	}
#ifdef CONFIG_APP_TEST_GETFREQUENCY
	if (etalTestGetFrequencySeek(ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass3 FAILED");
		return OSAL_ERROR;
	}
#endif

	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestGetFrequencySimple(ETAL_TEST_MODE_HD_FM, 4, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass4 FAILED");
		return OSAL_ERROR;
	}

	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestGetFrequencySimple(ETAL_TEST_MODE_HD_AM, 4, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass5 FAILED");
		return OSAL_ERROR;
	}

	if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_AM

	/*
	 * On the CMOST the FM and AM receivers share the same frontend
	 * so the AM tests must be done after the FM receiver is destroyed
	 * otherwise ETAL will complain
	 */

#ifdef CONFIG_APP_TEST_AM

	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestGetFrequencySimple(ETAL_TEST_MODE_AM, 5, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass6 FAILED");
		return OSAL_ERROR;
	}

	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_AM

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

