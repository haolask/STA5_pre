//!
//!  \file 		etaltest_seek_manual.c
//!  \brief 	<i><b> ETAL test, seek manual </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

// only for etalTestPrintNormal
tU32 ETAL_receiverGetFrequency(ETAL_HANDLE hReceiver);

#define ETAL_TEST_BAND_FMEU_MAX				  108000 // same as ETAL_BAND_FMEU_MAX
#define ETAL_TEST_BAND_FMEU_MIN				   87500 // same as ETAL_BAND_FMEU_MIN
#define ETAL_TEST_BAND_FMUS_MIN				   87900 // same as ETAL_BAND_FMUS_MIN
#define ETAL_TEST_BAND_FMUS_MAX				  107900 // same as ETAL_BAND_FMUS_MAX
#define ETAL_TEST_BAND_AM_MIN			  	     531
#define ETAL_TEST_BAND_AM_MAX					1629
#define ETAL_TEST_BAND_AMUS_MIN					 530
#define ETAL_TEST_BAND_AMUS_MAX					1710
#define ETAL_TEST_BAND_DAB_MIN				  174928 // same as ETAL_BAND_DAB3_MIN
#define ETAL_TEST_BAND_DAB_MAX				  239200 // same as ETAL_BAND_DAB3_MAX
#define ETAL_TEST_MANUAL_SEEK_COUNT				   2
#define ETAL_TEST_MANUAL_SEEK_FM_STEP_FREQ	     500
#define ETAL_TEST_MANUAL_SEEK_HD_STEP_FREQ		 200
#define ETAL_TEST_MANUAL_SEEK_AM_STEP_FREQ		   1

#ifdef CONFIG_APP_TEST_MANUAL_SEEK
typedef enum {
	MAN_SEEK_START = 1,
	MAN_SEEK_START_WRONG_HANDLE,
	MAN_SEEK_START_NULL_FREQ,
	MAN_SEEK_START_WRONG_STEP,
	MAN_SEEK_START_ALREADY_STARTED,
	MAN_SEEK_STOP,
	MAN_SEEK_CONTINUE_WRONG_HANDLE,
	MAN_SEEK_CONTINUE_NULL_FREQ,
	MAN_SEEK_CONTINUE,
	MAN_SEEK_STOP_WRONG_HANDLE,
	MAN_SEEK_STOP_NULL_FREQ,
	MAN_SEEK_STOP_NOT_STARTED
} ManualSeekTestTy;
#endif

#ifdef CONFIG_APP_TEST_MANUAL_SEEK
/***************************
 *
 * checkManualSeekFreq
 *
 **************************/
static tSInt checkManualSeekFreq(etalTestBroadcastTy mode, etalSeekDirectionTy direction, tU32 step, tU32 start_freq, tU32 freq, tU32 band_max, tU32 band_min)
{
	if (mode == ETAL_TEST_MODE_DAB)
	{
		/* DAB */
		switch (direction)
		{
			case cmdDirectionUp:
				if (freq > start_freq)
				{
					return OSAL_OK;
				}
				/* wrap around the band */
				else if ((start_freq == band_max) && (freq >= band_min))
				{
					return OSAL_OK;
				}
				return OSAL_ERROR;

			case cmdDirectionDown:
				if (freq < start_freq)
				{
					return OSAL_OK;
				}
				/* wrap around the band */
				else if ((start_freq == band_min) && (freq <= band_max))
				{
					return OSAL_OK;
				}
				return OSAL_ERROR;
			}
		}
	else
	{
		/* AM or FM or HD AM or HD FM */
		switch (direction)
		{
			case cmdDirectionUp:
				if (freq == start_freq + step)
				{
					return OSAL_OK;
				}
				else if (((mode == ETAL_TEST_MODE_HD_FM) || (mode == ETAL_TEST_MODE_HD_AM)) &&
						(freq == start_freq))
				{
					/* HD requires special procesing for the up direction because
					 * if tuned to an HD freq with SPS the seek migth not change
					 * the frequency but just jump to the next SPS */
					return OSAL_OK;
				}
				/* wrap around the band */
				else if ((start_freq + step > band_max) && (freq >= band_min) && (freq <= band_min + step))
				{
					return OSAL_OK;
				}
				return OSAL_ERROR;

			case cmdDirectionDown:
				if (freq == start_freq - step)
				{
					return OSAL_OK;
				}
				/* wrap around the band */
				else if ((start_freq - step < band_min) && (freq <= band_max) && (freq >= band_max - step))
				{
					return OSAL_OK;
				}
				return OSAL_ERROR;
		}
	}

	return OSAL_ERROR;
}

/***************************
 *
 * etalTestManualSeekInternal
 *
 **************************/
static tSInt etalTestManualSeekInternal(etalTestBroadcastTy mode, tBool *pass_out)
{
	ETAL_STATUS ret;
	tU32 count, freq;
	tBool pass;
	tBool pass1;
	EtalSeekStatus seekStatus;
	tU32 i, nb_of_manual_seek;
	tBool direction = cmdDirectionUp;
	tBool mute = cmdAudioMuted;
	EtalAudioInterfTy audioIf;
	tChar standardString[10];
	ETAL_HANDLE* hReceiver;
	etalTestConfigTy conf;
	ETAL_STATUS expected_seek_ret;
	EtalProcessingFeatures processingFeatures;
	tU32 start_freq, valid_freq, step;
	tU32 band_min, band_max;
	EtalFrequencyBand band;
	EtalAudioSourceTy audio_source;

	switch (mode)
	{
		case ETAL_TEST_MODE_FM:
			hReceiver = &handlefm;
			OSAL_szStringCopy(standardString, "FM");
			conf = ETAL_CONFIG_FM1;
			expected_seek_ret = ETAL_RET_SUCCESS;
			band = ETAL_BAND_FMEU;
			band_min = ETAL_TEST_BAND_FMEU_MIN;
			band_max = ETAL_TEST_BAND_FMEU_MAX;
			valid_freq = ETAL_VALID_FM_FREQ;
			step = ETAL_TEST_MANUAL_SEEK_FM_STEP_FREQ;
			audio_source = ETAL_AUDIO_SOURCE_STAR_AMFM;
			break;

		case ETAL_TEST_MODE_HD_FM:
			hReceiver = &handlehd;
			OSAL_szStringCopy(standardString, "HD FM");
			conf = ETAL_CONFIG_HDRADIO_FM;
			expected_seek_ret = ETAL_RET_SUCCESS;
			band = ETAL_BAND_FMUS;
			band_min = ETAL_TEST_BAND_FMUS_MIN;
			band_max = ETAL_TEST_BAND_FMUS_MAX;
			valid_freq = ETAL_VALID_HD_FREQ;
			step = ETAL_TEST_MANUAL_SEEK_HD_STEP_FREQ;
			audio_source = ETAL_AUDIO_SOURCE_AUTO_HD;
			break;

		case ETAL_TEST_MODE_HD_AM:
			hReceiver = &handlehdam;
			OSAL_szStringCopy(standardString, "HD AM");
			conf = ETAL_CONFIG_HDRADIO_AM;
			expected_seek_ret = ETAL_RET_SUCCESS;
			band = ETAL_BAND_MWUS;
			band_min = ETAL_TEST_BAND_AMUS_MIN;
			band_max = ETAL_TEST_BAND_AMUS_MAX;
			valid_freq = ETAL_VALID_HD_AM_FREQ;
			step = ETAL_TEST_MANUAL_SEEK_AM_STEP_FREQ;
			audio_source = ETAL_AUDIO_SOURCE_AUTO_HD;
			break;

		case ETAL_TEST_MODE_AM:
			hReceiver = &handleam;
			OSAL_szStringCopy(standardString, "AM");
			conf = ETAL_CONFIG_AM;
			expected_seek_ret = ETAL_RET_SUCCESS;
			band = ETAL_BAND_AM;
			band_min = ETAL_TEST_BAND_AM_MIN;
			band_max = ETAL_TEST_BAND_AM_MAX;
			valid_freq = ETAL_VALID_AM_FREQ;
			step = ETAL_TEST_MANUAL_SEEK_AM_STEP_FREQ;
			audio_source = ETAL_AUDIO_SOURCE_STAR_AMFM;
			break;

		case ETAL_TEST_MODE_DAB:
			hReceiver = &handledab;
			OSAL_szStringCopy(standardString, "DAB");
			conf = ETAL_CONFIG_DAB;
			/* for DAB this is expected if no station is available */
			expected_seek_ret = ETAL_RET_NO_DATA;
			band = ETAL_BAND_DAB3;
			band_min = ETAL_TEST_BAND_DAB_MIN;
			band_max = ETAL_TEST_BAND_DAB_MAX;
			valid_freq = ETAL_VALID_DAB_FREQ;
			step = 0; // unused
			audio_source = ETAL_AUDIO_SOURCE_DCOP_STA660;
			break;

		default:
			return OSAL_ERROR;
	}

	pass = TRUE;
	etalTestPrintNormal("Test manual seek %s", standardString);

	/* Configure the receiver */
	if (etalTestDoConfigSingle(conf, hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	if ((ret = etal_change_band_receiver(*hReceiver, band, 0, 0, processingFeatures)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_change_band_receiver %s (%s, %d)", standardString, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Tune to valid freq */
	etalTestPrintNormal("* Tune to %s freq %d", standardString, valid_freq);
	ret = etal_tune_receiver(*hReceiver, valid_freq);

	if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
	{
		etalTestPrintError("etal_tune_receiver %s (%s, %d)", standardString, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	start_freq = valid_freq;

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

	/* Enable audio in case Seek test is performed without any preceding test */
	ret = etal_audio_select(*hReceiver, audio_source);
	if (ret != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select %s (%s, %d)", standardString, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Set the number of manual seek */
	if (mode == ETAL_TEST_MODE_DAB)
	{
		/* frequencies are fixed in DAB */
		nb_of_manual_seek = 41;
		etalTestPrintNormal("Warning: total seek duration is over 2.5m");
	}
	else
	{
		nb_of_manual_seek = ((band_max - band_min) / step) - 1;
	}

	etalTestPrintReportPassStart(MAN_SEEK_START, mode, "Seek start, direction %s, step %d", (direction == cmdDirectionUp) ? "up" : "down", step);
	count = 0;
	pass1 = TRUE;
	while (count < ETAL_TEST_MANUAL_SEEK_COUNT)
	{
		direction = (count%2 == 0) ? cmdDirectionUp : cmdDirectionDown;
		mute = (count%2 != 0) ? cmdAudioMuted : cmdAudioUnmuted;
		etalTestPrintNormal("* Test seek start, direction %s, step %d", (direction == cmdDirectionUp) ? "up" : "down", step);

		ret = etal_seek_start_manual(*hReceiver, direction, step, &freq);
		if ((ret != ETAL_RET_SUCCESS) && (ret != expected_seek_ret))
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1a FAILED");
		}

		if (checkManualSeekFreq(mode, direction, step, start_freq, freq, band_max, band_min) != OSAL_OK)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1b FAILED");
		}
		start_freq = freq;
		etalTestPrintVerbose("* etal_seek_start_manual: direction %d, step %dkHz, returned value : freq %lu", direction, step, freq);

		if ((mode == ETAL_TEST_MODE_HD_FM) || (mode == ETAL_TEST_MODE_HD_AM))
		{
			if(freq == valid_freq)
			{
				/* time to allow the HD to reach AUDIO ACQUIRED if test was not 
				 * started on an HD frequency
				 * Also useful to hear the newly tuned SPS in case of no frequency
				 * change */
				OSAL_s32ThreadWait(ETAL_TEST_HD_AUDIO_SYNC_TIME);
			}
		}

		if ((ret = etal_seek_get_status_manual(*hReceiver, &seekStatus)) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1c FAILED");
		}
		if (mode == ETAL_TEST_MODE_DAB)
		{
			etalTestPrintVerbose("* etal_seek_get_status_manual : ETAL Freq %ld, RF field strength %ld, BB field strength %ld, FIC BER %ld, MSC BER %ld",
					seekStatus.m_frequency,
					seekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength,
					seekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength,
					seekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio,
					seekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio);
		}
		else 
		{
			etalTestPrintVerbose("* etal_seek_get_status_manual : ETAL Freq %ld, CMOST Freq %ld, full band scanned status %d, BB field strength %ld, Detune %lu, Multipath %lu, Ultrasonic Noise %lu, Adjacent channel %ld",
					ETAL_receiverGetFrequency(*hReceiver),
					seekStatus.m_frequency,
					seekStatus.m_fullCycleReached,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_BBFieldStrength,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_FrequencyOffset,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_Multipath,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_UltrasonicNoise,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_AdjacentChannel);
		}

		/* not much can be checked automatically except the frequency */
		if (seekStatus.m_frequency != freq)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1d FAILED");
		}

		etalTestPrintNormal("* Test seek continue, %d loops", nb_of_manual_seek);
		for(i = 0; i <= nb_of_manual_seek; i++)
		{
			ret = etal_seek_continue_manual(*hReceiver, &freq);
			if ((ret != ETAL_RET_SUCCESS) && (ret != expected_seek_ret))
			{
				pass1 = FALSE;
				etalTestPrintNormal("pass1e FAILED (%d)", ret);
			}
			if (checkManualSeekFreq(mode, direction, step, start_freq, freq, band_max, band_min) != OSAL_OK)
			{
				pass1 = FALSE;
				etalTestPrintNormal("pass1f FAILED");
			}
			start_freq = freq;
			etalTestPrintVerbose("* etal_seek_continue_manual : returned value : freq %lu", freq);

			if ((ret = etal_seek_get_status_manual(*hReceiver, &seekStatus)) != ETAL_RET_SUCCESS)
			{
				pass1 = FALSE;
				etalTestPrintNormal("pass1g FAILED (%d)", ret);
			}

			if (mode == ETAL_TEST_MODE_DAB)
			{
				etalTestPrintVerbose("* etal_seek_get_status_manual : ETAL Freq %ld, RF field strength %ld, BB field strength %ld, FIC BER %ld, MSC BER %ld",
						seekStatus.m_frequency,
						seekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength,
						seekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength,
						seekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio,
						seekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio);
			}
			else 
			{
				etalTestPrintVerbose("* etal_seek_get_status_manual : ETAL Freq %ld, CMOST Freq %ld, full band scanned status %d, BB field strength %ld, Detune %lu, Multipath %lu, Ultrasonic Noise %lu, Adjacent channel %ld",
						ETAL_receiverGetFrequency(*hReceiver),
						seekStatus.m_frequency,
						seekStatus.m_fullCycleReached,
						seekStatus.m_quality.EtalQualityEntries.amfm.m_BBFieldStrength,
						seekStatus.m_quality.EtalQualityEntries.amfm.m_FrequencyOffset,
						seekStatus.m_quality.EtalQualityEntries.amfm.m_Multipath,
						seekStatus.m_quality.EtalQualityEntries.amfm.m_UltrasonicNoise,
						seekStatus.m_quality.EtalQualityEntries.amfm.m_AdjacentChannel);
			}

			/* not much can be checked automatically except the frequency */
			if (seekStatus.m_frequency != freq)
			{
				pass1 = FALSE;
				etalTestPrintNormal("pass1h FAILED");
			}
		}

		if ((ret = etal_seek_stop_manual(*hReceiver, mute, &freq)) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1i FAILED");
		}
		etalTestPrintNormal("* etal_seek_stop_manual : mute %d, returned value : freq %lu", mute, freq);

        if ((ret = etal_seek_get_status_manual(*hReceiver, &seekStatus)) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1l FAILED");
		}
		if (mode == ETAL_TEST_MODE_DAB)
		{
			etalTestPrintVerbose("* etal_seek_get_status_manual : ETAL Freq %ld, RF field strength %ld, BB field strength %ld, FIC BER %ld, MSC BER %ld",
				seekStatus.m_frequency,
				seekStatus.m_quality.EtalQualityEntries.dab.m_RFFieldStrength,
				seekStatus.m_quality.EtalQualityEntries.dab.m_BBFieldStrength,
				seekStatus.m_quality.EtalQualityEntries.dab.m_FicBitErrorRatio,
				seekStatus.m_quality.EtalQualityEntries.dab.m_MscBitErrorRatio);
		}
		else 
		{
			etalTestPrintVerbose("* etal_seek_get_status_manual : ETAL Freq %ld, CMOST Freq %ld, full band scanned status %d, BB field strength %ld, Detune %lu, Multipath %lu, Ultrasonic Noise %lu, Adjacent channel %ld",
					ETAL_receiverGetFrequency(*hReceiver),
					seekStatus.m_frequency,
					seekStatus.m_fullCycleReached,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_BBFieldStrength,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_FrequencyOffset,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_Multipath,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_UltrasonicNoise,
					seekStatus.m_quality.EtalQualityEntries.amfm.m_AdjacentChannel);
		}

		/* not much can be checked automatically except the frequency */
		if (seekStatus.m_frequency != freq)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass1m FAILED");
		}

		if (mode != ETAL_TEST_MODE_DAB)
		{
			if(mute == TRUE)
			{
				etalTestPrintNormal("***********************************************");
				etalTestPrintNormal("\tYou should not hear audio ...");
				etalTestPrintNormal("***********************************************");
			}
			else
			{
				etalTestPrintNormal("***********************************************");
				etalTestPrintNormal("\tYou should hear audio ...");
				etalTestPrintNormal("***********************************************");
			}
		}

		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		count++;
	}

	if (!pass1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	direction = cmdDirectionUp;
	mute = 0;

	/* pass2: Test error cases of manual seek start */

	/* Manual seek start with wrong handle */
	etalTestPrintReportPassStart(MAN_SEEK_START_WRONG_HANDLE, mode, "Manual seek start with wrong handle");
	if ((ret = etal_seek_start_manual(ETAL_INVALID_HANDLE, direction, 0, &freq)) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Manual seek start with NULL frequency */
	etalTestPrintReportPassStart(MAN_SEEK_START_NULL_FREQ, mode, "Manual seek start with NULL frequency");
	if ((ret = etal_seek_start_manual(*hReceiver, direction, 0, NULL)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (mode != ETAL_TEST_MODE_DAB)
	{
		/* Manual seek start with wrong frequency step */
		etalTestPrintReportPassStart(MAN_SEEK_START_WRONG_STEP, mode, "Manual seek start with wrong frequency step");
		if ((ret = etal_seek_start_manual(*hReceiver, direction, 5000, &freq)) != ETAL_RET_PARAMETER_ERR)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2a FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
		etalTestPrintVerbose("* etal_seek_start_manual : direction %d, step %dkHz, returned value : freq %lu", direction, 5000, freq);
	}

	/* Manual seek start while manual seek is already started */
	etalTestPrintReportPassStart(MAN_SEEK_START_ALREADY_STARTED, mode, "Manual seek start while seek is already started");
	ret = etal_seek_start_manual(*hReceiver, direction, step, &freq);
	if ((ret != ETAL_RET_SUCCESS) && (ret != expected_seek_ret))
	{
		pass = FALSE;
//		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2b FAILED");
	}
	else
	{
//		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerbose("* etal_seek_start_manual : direction %d, step %dkHz, returned value : freq %lu", direction, ETAL_TEST_MANUAL_SEEK_FM_STEP_FREQ, freq);

//	etalTestPrintReportPassStart(mode, "Manual seek start while seek is already started");
	if (etal_seek_start_manual(*hReceiver, direction, step, &freq) != ETAL_RET_IN_PROGRESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerbose("* etal_seek_start_manual : direction %d, step %dkHz, returned value : freq %lu", direction, ETAL_TEST_MANUAL_SEEK_AM_STEP_FREQ, freq);

	etalTestPrintReportPassStart(MAN_SEEK_STOP, mode, "Manual seek stop");
	if (etal_seek_stop_manual(*hReceiver, mute, &freq) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintVerbose("* etal_seek_stop_manual : mute %d, returned value : freq %lu", mute, freq);

	/* pass3: test error cases of manual seek continue */

	/* Manual seek continue with wrong handle */
	etalTestPrintReportPassStart(MAN_SEEK_CONTINUE_WRONG_HANDLE, mode, "Manual seek continue with wrong handle");
	if ((ret = etal_seek_continue_manual(ETAL_INVALID_HANDLE, &freq)) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Manual seek continue with NULL frequency */
	etalTestPrintReportPassStart(MAN_SEEK_CONTINUE_NULL_FREQ, mode, "Manual seek continue with NULL frequency");
	ret = etal_seek_start_manual(*hReceiver, direction, 0, &freq);
	if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(MAN_SEEK_CONTINUE, mode, "Manual seek continue");
	if ((ret = etal_seek_continue_manual(*hReceiver, NULL)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(MAN_SEEK_STOP, mode, "Manual seek stop");
	if (etal_seek_stop_manual(*hReceiver, mute, &freq) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* pass4: test error cases of manual seek stop */

	/* Manual seek stop with wrong handle */
	etalTestPrintReportPassStart(MAN_SEEK_STOP_WRONG_HANDLE, mode, "Manual seek stop with wrong handle");
	if ((ret = etal_seek_stop_manual(ETAL_INVALID_HANDLE, mute, &freq)) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass4a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Manual seek stop with NULL frequency */
	etalTestPrintReportPassStart(MAN_SEEK_STOP_NULL_FREQ, mode, "Manual seek stop with NULL frequency");
	if ((ret = etal_seek_stop_manual(*hReceiver, mute, NULL)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass4b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Manual seek stop while manual seek is not started */
	etalTestPrintReportPassStart(MAN_SEEK_STOP_NOT_STARTED, mode, "Manual seek stop while manual seek is not started");
	if ((ret = etal_seek_stop_manual(*hReceiver, mute, &freq)) != ETAL_RET_INVALID_RECEIVER)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass4c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* final cleanup */

	if ((ret = etal_destroy_receiver(hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver %s (%s, %d)", standardString, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (!pass)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_MANUAL_SEEK

/***************************
 *
 * etalTestSeek
 *
 **************************/
tSInt etalTestManualSeek(void)
{
#ifdef CONFIG_APP_TEST_MANUAL_SEEK
	tBool pass;

	etalTestStartup();
	pass = TRUE;

#ifdef CONFIG_APP_TEST_FM
	etalTestPrintNormal("<---Manual seek FM starts");
	if (etalTestManualSeekInternal(ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_AM
	etalTestPrintNormal("<---Manual seek AM starts");
	if (etalTestManualSeekInternal(ETAL_TEST_MODE_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //CONFIG_APP_TEST_AM

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	etalTestPrintNormal("<---Manual seek HD FM starts");
	if (etalTestManualSeekInternal(ETAL_TEST_MODE_HD_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //CONFIG_APP_TEST_HDRADIO_FM

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	etalTestPrintNormal("<---Manual seek HD AM starts");
	if (etalTestManualSeekInternal(ETAL_TEST_MODE_HD_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //CONFIG_APP_TEST_HDRADIO_AM

#ifdef CONFIG_APP_TEST_DAB
	etalTestPrintNormal("<---Manual seek DAB starts");
	if (etalTestManualSeekInternal(ETAL_TEST_MODE_DAB, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //CONFIG_APP_TEST_DAB

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_MANUAL_SEEK

	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

