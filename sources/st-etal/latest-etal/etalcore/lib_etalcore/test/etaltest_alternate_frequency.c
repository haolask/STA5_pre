//!
//!  \file 		etaltest_alternate_frequency.c
//!  \brief 	<i><b> ETAL test, alternate frequency </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#define NB_OF_FREQUENCY_LIST			   10
#define ETAL_TEST_AF_LOOP				   10
#define ETAL_TEST_MAX_RFSTRENGTH_DEVIATION  5

/* Quality measurements must be done on frequency with good
 * signal, so list here some known FM stations.
 * For compatibility with HDRadio, select only frequencies that are
 * within the HDRadio band (87900-107900) */
#ifdef CONFIG_APP_TEST_IN_LE_MANS
	#define GOOD_FREQUENCY_LIST {95400, 97600, 100300, 100700, 102900, 103500, 104700, 105900, 106900, 107300}
#else
	#define GOOD_FREQUENCY_LIST {94800, 88500, 89300, 90100, 90800, 91100, 92800, 93100, 104800, 95300}
#endif

#ifdef CONFIG_APP_TEST_ALTERNATE_FREQUENCY
typedef enum {
	AF_START_CHECK_PARAM = 1,
	AF_END_CHECK_PARAM,
	AF_CHECK_PARAM_CHECK,
	AF_SWITCH_PARAM_CHECK,
	AF_CHECK_LOOP,
	AF_SWITCH_FREQ1,
	AF_SWITCH_FREQ1_CHECK,
	AF_SWITCH_FREQ2,
	AF_SWITCH_FREQ2_CHECK,
	AF_START_NORMAL_FREQ1,
	AF_START_CHECK_AFTER_NORMAL_FREQ1,
	AF_START_RESTART_FREQ1,
	AF_START_CHECK_AFTER_RESTART_FREQ1,
	AF_START_NORMAL_FREQ2,
	AF_START_CHECK_AFTER_NORMAL_FREQ2,
	AF_START_RESTART_FREQ2,
	AF_START_CHECK_AFTER_RESTART_FREQ2,
	AF_START_NORMAL_FREQ1_P2,
	AF_START_CHECK_AFTER_NORMAL_FREQ1_P2,
	AF_START_RESTART_FREQ1_P2,
	AF_START_CHECK_AFTER_RESTART_FREQ1_P2,
	AF_END,
	AF_END_CHECK,
	AF_SEARCH_MANUAL,
	AF_CHECK_FREQ_LIST
} AlternateFrequencyTestTy;
#endif

/***************************
 * function prototypes
 **************************/

#ifdef CONFIG_APP_TEST_ALTERNATE_FREQUENCY
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
/***************************
 *
 * etalTestAFCheck
 *
 **************************/
static tSInt etalTestMeasureQuality(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tU32 freq, EtalBcastQualityContainer *measured)
{
	ETAL_STATUS ret;

	/* hReceiver may be FM or HD; in the latter case depending on 
	 * <freq> the tune command may return ETAL_RET_NO_DATA but since
	 * we are measuring only the analogue (FM) quality, we don't care */
	ret = etal_tune_receiver(hReceiver, freq);
	if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
	{
		etalTestPrintError("etal_tune_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(ETAL_TEST_CMOST_QUALITY_SETTLE_TIME);

	ret = etal_get_channel_quality(hReceiver, measured);
	if (ret != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_channel_quality (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	else
	{
		if (mode == ETAL_TEST_MODE_FM)
		{
			etalTestPrintVerbose("* Reference quality freq %d: %d", freq, measured->EtalQualityEntries.amfm.m_RFFieldStrength);
		}
		else
		{
			etalTestPrintVerbose("* Reference quality freq %d: %d", freq, measured->EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength);
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestCheckFrequency
 *
 **************************/
static tSInt etalTestCheckFrequency(ETAL_HANDLE hReceiver, tU32 expected_freq, tBool *pass)
{
	ETAL_STATUS ret;
	tU32 curr_freq;

	ret = etal_get_receiver_frequency(hReceiver, &curr_freq);
	if (ret != ETAL_RET_SUCCESS)
	{
		 etalTestPrintError("etal_get_receiver_frequency (%s, %d)", ETAL_STATUS_toString(ret), ret);
		 return OSAL_ERROR;
	}
	if (curr_freq != expected_freq)
	{
		*pass = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestCheckQuality
 *
 **************************/
static tVoid etalTestCheckQuality(EtalBcastQualityContainer *measured, EtalBcastQualityContainer *expected, tBool *pass)
{
	if ((abs(measured->EtalQualityEntries.amfm.m_RFFieldStrength - expected->EtalQualityEntries.amfm.m_RFFieldStrength)) > ETAL_TEST_MAX_RFSTRENGTH_DEVIATION)
	{
		etalTestPrintVerbose("quality mismatch: measured %d, expected %d", measured->EtalQualityEntries.amfm.m_RFFieldStrength, expected->EtalQualityEntries.amfm.m_RFFieldStrength);
		*pass = FALSE;
	}
}

/***************************
 *
 * etalTestCheckFrequencyAndQuality
 *
 **************************/
static tSInt etalTestCheckFrequencyAndQuality(ETAL_HANDLE hReceiver, tU32 expected_freq, EtalBcastQualityContainer *measured, EtalBcastQualityContainer *expected, tBool *pass)
{
	if (etalTestCheckFrequency(hReceiver, expected_freq, pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestCheckQuality(measured, expected, pass);
	return OSAL_OK;
}

/***************************
 *
 * etalTestAFCheckInternal
 *
 **************************/
static tSInt etalTestAFCheckInternal(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tChar *str, tChar *strnum, tU32 f1, tU32 f2, tBool *pass_out)
{
	ETAL_STATUS ret;
	tU32 count;
	tBool pass;
	EtalBcastQualityContainer result; 
	EtalBcastQualityContainer expected;

	pass = TRUE;

	/* Measure the quality on f1 and later use it as reference */
	if (etalTestMeasureQuality(hReceiver, mode, f1, &expected) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Tune the receiver to f2 */
	etalTestPrintNormal("* Tune to %s freq %d", etalTestStandard2String(mode), f2);
	if ((ret = etal_tune_receiver(hReceiver, f2)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Test AF check on f1 */
	etalTestPrintReportPassStart(AF_CHECK_LOOP, mode, "AF check on %s, %d loops", str, ETAL_TEST_AF_LOOP);
	for (count = 0; count < ETAL_TEST_AF_LOOP; count++)
	{
		etalTestPrintVerbose("* AF check starts: on %s channel freq: %d with Automatic antenna selected", str, f1);
		etal_AF_check(hReceiver, f1, 0, &result);
		etalTestPrintVerboseContainer(&result);
		etalTestCheckQuality(&result, &expected, &pass);
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%s FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestAFCheck
 *
 **************************/
/*
 * Tunes and measures quality on f2, tunes to f1 and then AFChecks on f2
 * The returned quality from f2 must match the quality measured initially.
 * Repeats for fore and background channels, if available (that is if the
 * Frontend is not already employed in HD or DAB).
 */
static tSInt etalTestAFCheck(ETAL_HANDLE hReceiver1, ETAL_HANDLE hReceiver2, etalTestBroadcastTy mode, tU32 freq, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalAudioInterfTy audioIf;
	tBool pass;

	etalTestPrintNormal("AF Check on %s", etalTestStandard2String(mode));
	pass = TRUE;

	/* Select audio FM */
	etalTestPrintNormal("* Select audio FM");
	/* Configure audio path */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
	if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_AM) || (mode == ETAL_TEST_MODE_DAB))
	{
		audioIf.m_dac = 0;
	}
	else
	{
		system("amixer -c 3 sset Source adcauxdac > /dev/null" );

		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
	}
#endif
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif

	// Audio path should be correctly set before
#if 0

	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		 etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
		 return OSAL_ERROR;
	}
#endif

	/* Select audio source */
	if ((ret = etal_audio_select(hReceiver1, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (etalTestAFCheckInternal(hReceiver1, mode, "foreground", "1", ETAL_VALID_FM_FREQ2, freq, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Use the second receiver */

	if (hReceiver2 != ETAL_INVALID_HANDLE)
	{
		if (etalTestAFCheckInternal(hReceiver2, mode, "background", "2", ETAL_VALID_FM_FREQ2, ETAL_VALID_FM_FREQ, &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	if (!pass)
	{
		*pass_out = 0;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestAFSwitchInternal
 *
 **************************/
static tSInt etalTestAFSwitchInternal(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tChar *str, tChar *strnum, tU32 f1, tU32 f2, tBool *pass_out)
{
	tSInt ret;
	tBool pass;

	/* Tune the receiver */
	etalTestPrintNormal("* Tune to %s freq %d", etalTestStandard2String(mode), f2);
	if ((ret = etal_tune_receiver(hReceiver, f2)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Test AF switch on the foreground channel without smoothing */
	etalTestPrintReportPassStart(AF_SWITCH_FREQ1, mode, "AF_switch %s channel freq %d", etalTestStandard2String(mode), f1);
	if (etal_AF_switch(hReceiver, f1) != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(AF_SWITCH_FREQ1_CHECK, mode, "AF_switch check frequency, pass1");
	pass = TRUE;
	if (etalTestCheckFrequency(hReceiver, f1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sa FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Test AF switch on the foreground channel with smoothing */
	/* TODO: smoothing parameter is no longer present so this test could be removed */
	etalTestPrintReportPassStart(AF_SWITCH_FREQ2, mode, "AF_switch %s channel freq %d", etalTestStandard2String(mode), f2);
	if (etal_AF_switch(hReceiver, f2) != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(AF_SWITCH_FREQ2_CHECK, mode, "AF_switch check frequency, pass2");
	pass = TRUE;
	if (etalTestCheckFrequency(hReceiver, f2, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
	/* To give time to find HD frequency, if any */
	OSAL_s32ThreadWait(3 * ETAL_TEST_ONE_SECOND);
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)

	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sb FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	return OSAL_OK;
}

/***************************
 *
 * etalTestAFSwitch
 *
 **************************/
/*
 * Tunes to f2 and AFSwitches to f1, then checks if the tuner is on f1.
 * Repeats for fore and background channels, if available (that is if the
 * Frontend is not already employed in HD or DAB).
 */
static tSInt etalTestAFSwitch(ETAL_HANDLE hReceiver1, ETAL_HANDLE hReceiver2, etalTestBroadcastTy mode, tU32 freq, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalAudioInterfTy audioIf;
	tBool pass;

	etalTestPrintNormal("AF Switch on %s", etalTestStandard2String(mode));

	/* Select audio FM */
	etalTestPrintNormal("* Select audio FM");
	/* Configure audio path */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
	if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_AM) || (mode == ETAL_TEST_MODE_DAB))
	{
		audioIf.m_dac = 0;
	}
	else
	{
		system("amixer -c 3 sset Source adcauxdac > /dev/null" );

		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
	}
#endif
	 // Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	 audioIf.m_sai_slave_mode = TRUE;
#endif

	 // Audio path should be correctly set before
#if 0
		 
	 if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	 {
		etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
#endif

	/* Select audio source */
	if ((ret = etal_audio_select(hReceiver1, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	pass = TRUE;
	if (etalTestAFSwitchInternal(hReceiver1, mode, "foreground", "1", ETAL_VALID_FM_FREQ2, freq, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (hReceiver2 != ETAL_INVALID_HANDLE)
	{
		if (etalTestAFSwitchInternal(hReceiver2, mode, "background", "2", ETAL_VALID_FM_FREQ2, ETAL_VALID_FM_FREQ, &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestAFStartEndInternal
 *
 **************************/
static tSInt etalTestAFStartEndInternal(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tChar *str, tChar *strnum, tU32 f1, tU32 f2, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalBcastQualityContainer result; 
	EtalBcastQualityContainer expected1, expected2;
	tBool pass;

	/* Measure the quality and later use it as reference */
	if (etalTestMeasureQuality(hReceiver, mode, f1, &expected1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestMeasureQuality(hReceiver, mode, f2, &expected2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* start on f2 */
	etalTestPrintNormal("* Tune to %s freq %d", etalTestStandard2String(mode), f2);
	if ((ret = etal_tune_receiver(hReceiver, f2)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(AF_START_NORMAL_FREQ1, mode, "AF_start normal %s channel freq %d", str, f1);
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, f1, 0, &result)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_AF_start %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sa FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_START_CHECK_AFTER_NORMAL_FREQ1, mode, "AF_start check frequency and quality, pass1");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f1, &result, &expected1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sb FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintReportPassStart(AF_START_RESTART_FREQ1, mode, "AF_start restart %s channel freq %d", str, f1);
	if ((ret = etal_AF_start(hReceiver, cmdRestartAFMeasurement, f1, 0, &result)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_AF_start %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sc FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_START_CHECK_AFTER_RESTART_FREQ1, mode, "AF_start check frequency and quality, pass2");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f1, &result, &expected1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sd FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);


	etalTestPrintReportPassStart(AF_START_NORMAL_FREQ2, mode, "AF_start normal %s channel freq %d", str, f2);
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, f2, 0, &result)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_AF_start %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%se FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_START_CHECK_AFTER_NORMAL_FREQ2, mode, "AF_start check frequency and quality, pass3");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f2, &result, &expected2, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sf FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintReportPassStart(AF_START_RESTART_FREQ2, mode, "AF_start restart on %s channel freq %d", str, f2);
	if (etal_AF_start(hReceiver, cmdRestartAFMeasurement, f2, 0, &result) != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sg FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_START_CHECK_AFTER_RESTART_FREQ2, mode, "AF_start check frequency and quality, pass4");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f2, &result, &expected2, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sh FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintReportPassStart(AF_START_NORMAL_FREQ1_P2, mode, "AF_start normal %s channel freq %d", str, f1);
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, f1, 0, &result)) != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_AF_start (%s, %d)", ETAL_STATUS_toString(ret), ret);	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_START_CHECK_AFTER_NORMAL_FREQ1_P2, mode, "AF_start check frequency and quality, pass5");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f1, &result, &expected1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%si FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintReportPassStart(AF_START_RESTART_FREQ1_P2, mode, "AF_start restart %s channel freq %d", str, f1);
	if (etal_AF_start(hReceiver, cmdRestartAFMeasurement, f1, 0, &result) != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sj FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_START_CHECK_AFTER_RESTART_FREQ1_P2, mode, "AF_start check frequency and quality, pass6");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f1, &result, &expected1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sk FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);


	etalTestPrintReportPassStart(AF_END, mode, "AF_end %s channel freq %d", str, f1);
	if (etal_AF_end(hReceiver, f1, &result) != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sl FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerboseContainer(&result);

	etalTestPrintReportPassStart(AF_END_CHECK, mode, "AF_end check frequency and quality, pass7");
	pass = TRUE;
	if (etalTestCheckFrequencyAndQuality(hReceiver, f1, &result, &expected1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
	/* To give time to find HD frequency, if any */
	OSAL_s32ThreadWait(3 * ETAL_TEST_ONE_SECOND);
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)

	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%sm FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestAFStartEnd
 *
 **************************/
/*
 * Measures quality on f1 and f2 and then AFStarts between the
 * two frequencies checking that the quality is the same as the
 * one previously measured and that the tuner actually is on the AF frequency.
 * Repeats for fore and background channels, if available (that is if the
 * Frontend is not already employed in HD or DAB).
 */
static tSInt etalTestAFStartEnd(ETAL_HANDLE hReceiver1, ETAL_HANDLE hReceiver2, etalTestBroadcastTy mode, tU32 freq, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalAudioInterfTy audioIf;
	tBool pass;

	etalTestPrintNormal("AF Start/End on %s", etalTestStandard2String(mode));

	/* Select audio FM */
	etalTestPrintNormal("* Select audio FM");

	/* Configure audio path */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
	if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_AM) || (mode == ETAL_TEST_MODE_DAB))
	{
		audioIf.m_dac = 0;
	}
	else
	{
		system("amixer -c 3 sset Source adcauxdac > /dev/null" );

		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
	}
#endif
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	 audioIf.m_sai_slave_mode = TRUE;
#endif

	 // Audio path should be correctly set before
#if 0
		 

	 if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	 {
		 etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
		 return OSAL_ERROR;
	 }
#endif

	 /* Select audio source */
	if ((ret = etal_audio_select(hReceiver1, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Test AF start on the foreground channel */
	etalTestPrintNormal("Test AF start on the foreground channel");

	etalTestPrintVerbose("***********************************************");
	etalTestPrintVerbose("\tYou should not hear audio ...");
	etalTestPrintVerbose("***********************************************");

	pass = TRUE;
	if (etalTestAFStartEndInternal(hReceiver1, mode, "foreground", "1", ETAL_VALID_FM_FREQ2, freq, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Use the second receiver */

	if (hReceiver2 != ETAL_INVALID_HANDLE)
	{
		etalTestPrintNormal("Test AF start on the background channel");

		etalTestPrintVerbose("***********************************************");
		etalTestPrintVerbose("\tYou should hear audio ...");
		etalTestPrintVerbose("***********************************************");

		if (etalTestAFStartEndInternal(hReceiver2, mode, "background", "2", ETAL_VALID_FM_FREQ2, ETAL_VALID_FM_FREQ, &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestAFSearchManualInternal
 *
 **************************/
static tSInt etalTestAFSearchManualInternal(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tChar *str, tChar *strnum,  tU32 f2, tBool *pass_out)
{
	tSInt ret;
	tU32 i, AFList[NB_OF_FREQUENCY_LIST] = GOOD_FREQUENCY_LIST;
	EtalBcastQualityContainer AFQualityList[NB_OF_FREQUENCY_LIST];
	EtalBcastQualityContainer AFQualityListReference[NB_OF_FREQUENCY_LIST];
	tBool pass;

	/* fill the array of reference quality values */
	for (i = 0; i < NB_OF_FREQUENCY_LIST; i++)
	{
		if (etalTestMeasureQuality(hReceiver, mode, AFList[i], &AFQualityListReference[i]) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintNormal("* Tune to %s freq %d", etalTestStandard2String(mode), f2);
	if ((ret = etal_tune_receiver(hReceiver, f2)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", etalTestStandard2String(mode), ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(AF_SEARCH_MANUAL, mode, "AF_search_manual %s channel freq %d", str, f2);

	/* Test AF Search Manual */
	pass = TRUE;
	if ((ret = etal_AF_search_manual(hReceiver, 0, AFList, NB_OF_FREQUENCY_LIST, AFQualityList)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_AF_search_manual (%s, %d)", ETAL_STATUS_toString(ret), ret);	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(AF_CHECK_FREQ_LIST, mode, "Check frequency list");
	for(i = 0; i < NB_OF_FREQUENCY_LIST; i++)
	{
		etalTestPrintVerbose("AF frequency = %d", AFList[i]);
		etalTestPrintVerboseContainer(&AFQualityList[i]);
		etalTestCheckQuality(&AFQualityList[i], &AFQualityListReference[i], &pass);
	}

#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
	/* To give time to find HD frequency, if any */
	OSAL_s32ThreadWait(3 * ETAL_TEST_ONE_SECOND);
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)

	if (!pass)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%s FAILED", strnum);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestAFSearchManual
 *
 **************************/
/*
 * Measures quality on NB_OF_FREQUENCY_LIST different frequencies then
 * tunes to some frequency and issues the AFStart. Then it checks
 * if the returned qualities are same as the measured ones.
 * Repeats for fore and background channels, if available (that is if the
 * Frontend is not already employed in HD or DAB).
 *
 * NOTE: the test fails if any of the NB_OF_FREQUENCY_LIST frequencies does
 * not tune to a good station because the quality floats for no signal situation.
 */
static tSInt etalTestAFSearchManual(ETAL_HANDLE hReceiver1, ETAL_HANDLE hReceiver2, etalTestBroadcastTy mode, tU32 freq, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalAudioInterfTy audioIf;
	tBool pass;

	etalTestPrintNormal("AF Search Manual on %s", etalTestStandard2String(mode));

	/* Select audio FM */
	etalTestPrintNormal("* Select audio FM");
	/* Configure audio path */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
	if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_AM) || (mode == ETAL_TEST_MODE_DAB))
	{
		audioIf.m_dac = 0;
	}
	else
	{
		system("amixer -c 3 sset Source adcauxdac > /dev/null" );

		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
	}
#endif
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif

	// Audio path should be correctly set before
#if 0
		

	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
#endif

	/* Select audio source */
	if ((ret = etal_audio_select(hReceiver1, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	pass = TRUE;
	if (etalTestAFSearchManualInternal(hReceiver1, mode, "foreground", "1", freq, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (hReceiver2 != ETAL_INVALID_HANDLE)
	{
		if (etalTestAFSearchManualInternal(hReceiver2, mode, "background", "2", ETAL_VALID_FM_FREQ, &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_HDRADIO_FM

/***************************
 *
 * etalTestAFParameters
 *
 **************************/
static tSInt etalTestAFParameters(ETAL_HANDLE hReceiver, etalTestBroadcastTy test_mode, etalTestTuneTy tune_mode, tU32 freq, tBool *pass_out)
{
	ETAL_STATUS ret, exp_ret;
	EtalBcastQualityContainer qual;
	tBool pass = TRUE;

	if ((test_mode == ETAL_TEST_MODE_FM) || (test_mode == ETAL_TEST_MODE_HD_FM))
	{
		exp_ret = ETAL_RET_PARAMETER_ERR;
	}
	else
	{
		exp_ret = ETAL_RET_INVALID_BCAST_STANDARD;
	}

	/* invalid tuned frequency */
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, freq, 0, &qual)) != ETAL_RET_ERROR)
	{
		etalTestPrintNormal("etal_AF_start 1 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* now tune the receiver so other parameters can be checked */
	if (etalTestDoTuneSingle(tune_mode, hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(AF_START_CHECK_PARAM, test_mode, "AF_start parameter checks");

	/* invalid quality container */
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, freq, 0, NULL)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_start 2 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* invalid etalAFModeTy */
	if ((ret = etal_AF_start(hReceiver, 100, freq, 0, &qual)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_start 3 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* invalid frequency */
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, 0, 0, &qual)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_start 4 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* invalid antenna selection */
	if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, freq, 100, &qual)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_start 5 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	if (pass)
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	else
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}

	if ((test_mode == ETAL_TEST_MODE_FM) || (test_mode == ETAL_TEST_MODE_HD_FM))
	{
		etalTestPrintReportPassStart(AF_END_CHECK_PARAM, test_mode, "AF_end parameter checks");

		if ((ret = etal_AF_start(hReceiver, cmdNormalMeasurement, freq, 0, &qual)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintNormal("etal_AF_start 6 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
			pass = FALSE;
		}

		/* invalid quality container */
		if ((ret = etal_AF_end(hReceiver, freq, NULL)) != exp_ret)
		{
			etalTestPrintNormal("etal_AF_end 1 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
			pass = FALSE;
		}

		/* invalid frequency */
		if ((ret = etal_AF_end(hReceiver, 0, &qual)) != exp_ret)
		{
			etalTestPrintNormal("etal_AF_end 2 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
			pass = FALSE;
		}

		if ((ret = etal_AF_end(hReceiver, freq, &qual)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintNormal("etal_AF_end 3 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
			pass = FALSE;
		}

		if (pass)
		{
			etalTestPrintReportPassEnd(testPassed);
		}
		else
		{
			*pass_out = FALSE;
			etalTestPrintReportPassEnd(testFailed);
		}
	}
	else
	{
		etalTestPrintReportPassStart(AF_END_CHECK_PARAM, test_mode, "AF_end parameter checks");

		/* invalid quality container */
		if ((ret = etal_AF_end(hReceiver, freq, NULL)) != exp_ret)
		{
			etalTestPrintNormal("etal_AF_end 1 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
			pass = FALSE;
		}

		/* invalid frequency */
		if ((ret = etal_AF_end(hReceiver, 0, &qual)) != exp_ret)
		{
			etalTestPrintNormal("etal_AF_end 2 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
			pass = FALSE;
		}

		if (pass)
		{
			etalTestPrintReportPassEnd(testPassed);
		}
		else
		{
			*pass_out = FALSE;
			etalTestPrintReportPassEnd(testFailed);
		}
	}

	etalTestPrintReportPassStart(AF_CHECK_PARAM_CHECK, test_mode, "AF_check parameter checks");

	/* invalid quality container */
	if ((ret = etal_AF_check(hReceiver, freq, 0, NULL)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_check 1 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* invalid frequency */
	if ((ret = etal_AF_check(hReceiver, 0, 0, &qual)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_check 2 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* invalid antenna selection */
	if ((ret = etal_AF_check(hReceiver, freq, 100, &qual)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_check 3 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	if (pass)
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	else
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}

	etalTestPrintReportPassStart(AF_SWITCH_PARAM_CHECK, test_mode, "AF_switch parameter checks");

	/* invalid frequency */
	if ((ret = etal_AF_switch(hReceiver, 0)) != exp_ret)
	{
		etalTestPrintNormal("etal_AF_switch 1 %s (%s, %d)", etalTestStandard2String(test_mode), ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	if (pass)
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	else
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestAF_internal
 *
 **************************/
static tSInt etalTestAF_internal(etalTestBroadcastTy test_mode, tBool *pass)
{
	tU32 valid_freq;
	ETAL_HANDLE *phReceiver1, *phReceiver2, local_handle;
	etalTestConfigTy config_mode1, config_mode2;
	etalTestTuneTy tune_mode;

	local_handle = ETAL_INVALID_HANDLE;
	phReceiver2 = &local_handle;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_FM:
			valid_freq = ETAL_VALID_FM_FREQ;
			tune_mode = ETAL_TUNE_FM;
			phReceiver1 = &handlefm;
			config_mode1 = ETAL_CONFIG_FM1;
			config_mode2 = ETAL_CONFIG_NONE;
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
#if !defined (CONFIG_APP_TEST_DAB) && !defined (CONFIG_APP_TEST_HDRADIO_FM)
			phReceiver2 = &handlefm2;
			config_mode2 = ETAL_CONFIG_FM2; /* ETAL_CONFIG_FM2 selects the background channel */
#endif
#endif
			break;

		case ETAL_TEST_MODE_HD_FM:
			/* The config utilities are more DAB- and FM-oriented than HD.
			 * Selecting a background FM tuner and at the same time a foreground HD
			 * tuner is a mess, we give up testing dual channel configuration */
			valid_freq = ETAL_VALID_HD_FREQ;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			phReceiver1 = &handlehd;
			config_mode1 = ETAL_CONFIG_HDRADIO_FM;
			config_mode2 = ETAL_CONFIG_NONE;
			break;

		case ETAL_TEST_MODE_AM:
			valid_freq = ETAL_VALID_AM_FREQ;
			tune_mode = ETAL_TUNE_AM;
			phReceiver1 = &handleam;
			config_mode1 = ETAL_CONFIG_AM;
			config_mode2 = ETAL_CONFIG_NONE;
			break;

		case ETAL_TEST_MODE_HD_AM:
			valid_freq = ETAL_VALID_HD_AM_FREQ;
			tune_mode = ETAL_TUNE_HDRADIO_AM;
			phReceiver1 = &handlehdam;
			config_mode1 = ETAL_CONFIG_HDRADIO_AM;
			config_mode2 = ETAL_CONFIG_NONE;
			break;

		default:
			return OSAL_ERROR;
	}

	/* config */

	if(etalTestDoConfigSingle(config_mode1, phReceiver1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if(etalTestDoConfigSingle(config_mode2, phReceiver2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* start tests */
	if (etalTestAFParameters(*phReceiver1, test_mode, tune_mode, valid_freq, pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
	if ((test_mode == ETAL_TEST_MODE_FM) || (test_mode == ETAL_TEST_MODE_HD_FM))
	{
		if(etalTestAFCheck(*phReceiver1, *phReceiver2, test_mode, valid_freq, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if(etalTestAFSwitch(*phReceiver1, *phReceiver2, test_mode, valid_freq, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if(etalTestAFStartEnd(*phReceiver1, *phReceiver2, test_mode, valid_freq, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if(etalTestAFSearchManual(*phReceiver1, *phReceiver2, test_mode, valid_freq, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#endif

	/* cleanup */

	if (etalTestUndoConfigSingle(phReceiver2) == OSAL_ERROR)
	{
		return OSAL_ERROR;
	}

	if (etalTestUndoConfigSingle(phReceiver1) == OSAL_ERROR)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_ALTERNATE_FREQUENCY

/***************************
 *
 * etalTestAF
 *
 **************************/
tSInt etalTestAF(void)
{
#ifdef CONFIG_APP_TEST_ALTERNATE_FREQUENCY
	tBool pass;
	tSInt ret = OSAL_OK;

	if (ret != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestStartup();
	pass = TRUE;

	/* if CONFIG_APP_TEST_FM is defined this selects the foreground for FM receiver;
	 * if CONFIG_APP_TEST_HDRADIO_FM is defined, this selects the foreground for HD receiver;
	 * both defined is not supported
	 */

#ifdef CONFIG_APP_TEST_FM
	if (etalTestAF_internal(ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_AM
	if (etalTestAF_internal(ETAL_TEST_MODE_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestAF_internal(ETAL_TEST_MODE_HD_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestAF_internal(ETAL_TEST_MODE_HD_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_ALTERNATE_FREQUENCY
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

