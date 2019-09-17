//!
//!  \file 		etaltest_audio.c
//!  \brief 	<i><b> ETAL test, audio mute and force mono </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#ifdef CONFIG_APP_TEST_AUDIO

#define DO_MUTE    0
#define DO_MONO    1
#define DO_DIGITAL 2
#define DO_INFO_FM_STEREO 3

#define DO_DAB     0
#define DO_FM      1
#define DO_AM      2
#define DO_HD_FM   3
#define DO_HD_AM   4

tU32 etalTestInfoFmStereoCount;
tU8 etalTestInfoFmStereoExpected;
ETAL_HANDLE etalTestInfoFmStereoReceiverExpected;

typedef enum {
	AUDIO_MUTE = 1,
	AUDIO_UNMUTE,
	AUDIO_SET_MONO,
	AUDIO_SET_STEREO,
	AUDIO_ENABLE_ANALOG,
	AUDIO_ENABLE_DIGITAL,
	AUDIO_START_STEREO_EVENT,
	AUDIO_CHECK_STEREO_EVENT,
	AUDIO_CHECK_STEREO_EVENTS,
	AUDIO_STOP_STEREO_EVENT,
	AUDIO_COUNT_EVENTS
} AudioTestTy;
#endif

#ifdef CONFIG_APP_TEST_AUDIO_SELECT
typedef enum {
	AUDIO_SELECT_ILLEGAL_SOURCE = 1,
	AUDIO_SELECT_SOURCE,
	AUDIO_SELECT_RCV,
	AUDIO_SELECT_RCV_FORCE_DIGITAL,
	AUDIO_SELECT_RCV_AUTO
} AudioSelectTestTy;
#endif

#ifdef CONFIG_APP_TEST_AUDIO
#if ((defined(CONFIG_APP_TEST_DAB)) || (defined(CONFIG_APP_TEST_FM)) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM))
/***************************
 *
 * etalTestMuteMonoDigital
 *
 **************************/
static tSInt etalTestMuteMonoDigital(tU8 action, tBool *pass_out)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tChar *actionMute[2] = {"You should not hear audio ...", "You should hear audio ..."};
	tChar *actionMono[2] = {"Audio should be in mono ...",   "Audio should be in stereo ..."};
	tChar *actionDigital[2] = {"Analog audio only ...",   "Digital audio only ..."};
	tChar *actionString[2] ={NULL, NULL};
	etalTestBroadcastTy mode[5] = {ETAL_TEST_MODE_DAB, ETAL_TEST_MODE_FM, ETAL_TEST_MODE_AM, ETAL_TEST_MODE_HD_FM, ETAL_TEST_MODE_HD_AM};
	ETAL_HANDLE hReceiver;
	tU32 i;
	EtalAudioInterfTy intf;
	EtalAudioSourceTy audioSrc;
	tBool pass;

	/* default intf state */
	etalTestStartup();
	OSAL_pvMemorySet((tVoid *)&intf, 0x00, sizeof(EtalAudioInterfTy));
	pass = TRUE;

	/*
	 * This function sets handle* to a valid value only for the
	 * broadcast standards defined in the test configuration
	 * Other handles are left to INVALID
	 */

	switch (action)
	{
		case DO_MUTE:
			actionString[0] = actionMute[0];
			actionString[1] = actionMute[1];
			break;
		case DO_INFO_FM_STEREO:
		case DO_MONO:
			actionString[0] = actionMono[0];
			actionString[1] = actionMono[1];
			break;
		case DO_DIGITAL:
			actionString[0] = actionDigital[0];
			actionString[1] = actionDigital[1];
			break;
		default:
			return OSAL_ERROR;
	}

	/*
	 * Run one test per broadcast standard at a time
	 * 0 = DAB
	 * 1 = FM
	 * 2 = AM
	 * 3 = HD FM
	 * 4 = HD AM
	 */
	for (i = 0; i < 5; i++)
	{
#if 0
		/* config audio path analog and digital before change band */
		intf.m_dac = intf.m_sai_in = intf.m_sai_out = 1;
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
		intf.m_sai_slave_mode = true;
#endif
#if 1
		if ((ret = etal_config_audio_path(0, intf)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
#endif
#endif
		/* create the receiver */
		switch (i)
		{
			case DO_DAB:
				if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				hReceiver = handledab;
				audioSrc = ETAL_AUDIO_SOURCE_DCOP_STA660;
				break;

			case DO_FM:
				if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				hReceiver = handlefm;
				audioSrc = ETAL_AUDIO_SOURCE_STAR_AMFM;
				break;

			case DO_AM:
				if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				hReceiver = handleam;
				audioSrc = ETAL_AUDIO_SOURCE_STAR_AMFM;
				break;

			case DO_HD_FM:
				if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				hReceiver = handlehd;
				audioSrc = ETAL_AUDIO_SOURCE_DCOP_STA680;
				break;

			case DO_HD_AM:
				if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehdam) != OSAL_OK)
				{
					return OSAL_ERROR;
				}
				hReceiver = handlehdam;
				audioSrc = ETAL_AUDIO_SOURCE_DCOP_STA680;
				break;

			default:
				return OSAL_ERROR;
		}

		if ((handledab != ETAL_INVALID_HANDLE) && ((action == DO_MONO) || (action == DO_INFO_FM_STEREO)))
		{
			/*
			 * API not defined for DAB, skip the test.
			 * The next call resets handledab to invalid so the rest
			 * of the test will be skipped
			 */
			etalTestPrintNormal("Skipping Mono test for DAB since API not defined for DAB");
			etalTestUndoConfigSingle(&handledab);
			hReceiver = ETAL_INVALID_HANDLE;
		}

		if (((handlehdam != ETAL_INVALID_HANDLE) || (handleam != ETAL_INVALID_HANDLE)) &&
			((action == DO_MONO) || (action == DO_INFO_FM_STEREO)))
		{
			/*
			 * Mono and Stereo tests are not applicable in AM nor HD_AM.
			 */
			etalTestPrintNormal("Skipping Mono and FM stereo tests for (HD) AM, this is not applicable");
			etalTestUndoConfigSingle(&handlehdam);
			etalTestUndoConfigSingle(&handleam);
			hReceiver = ETAL_INVALID_HANDLE;
		}

		if (hReceiver != ETAL_INVALID_HANDLE)
		{
			etalTestInfoFmStereoCount = 0;
			etalTestInfoFmStereoExpected = 1;
			switch (i)
			{
				case DO_DAB:
#if defined (CONFIG_ETAL_SUPPORT_DCOP)
					if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
						OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
						etalTestPrintNormal("Select DAB audio service");
						ret = etal_service_select_audio(handledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID, ETAL_INVALID, ETAL_INVALID);
						if (ret != ETAL_RET_SUCCESS)
						{
							etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
							return OSAL_ERROR;
						}
					}
#else
					return OSAL_ERROR;
#endif
					break;

				case DO_FM:
					etalTestInfoFmStereoReceiverExpected = handlefm;
					if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
						if (action == DO_INFO_FM_STEREO)
						{
							etalTestPrintReportPassStart(AUDIO_START_STEREO_EVENT, ETAL_TEST_MODE_FM, "Start stereo event");
							if ((ret = etal_event_FM_stereo_start(handlefm)) != ETAL_RET_SUCCESS)
							{
								pass = FALSE;
								etalTestPrintReportPassEnd(testFailed);
								etalTestPrintNormal("etal_event_FM_stereo_start (%s, %d) for FM", ETAL_STATUS_toString(ret), ret);
							}
							else
							{
								etalTestPrintReportPassEnd(testPassed);

							}
						}
#endif
					}
					break;

				case DO_AM:
					etalTestInfoFmStereoReceiverExpected = ETAL_INVALID_HANDLE;
					if (etalTestDoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					break;

				case DO_HD_FM:
					etalTestInfoFmStereoReceiverExpected = handlehd;
					if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					else
					{
#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
						if (action == DO_INFO_FM_STEREO)
						{
							etalTestPrintReportPassStart(AUDIO_START_STEREO_EVENT, ETAL_TEST_MODE_HD_FM, "Start stereo event");
							if ((ret = etal_event_FM_stereo_start(handlehd)) != ETAL_RET_SUCCESS)
							{
								pass = FALSE;
								etalTestPrintReportPassEnd(testFailed);
								etalTestPrintNormal("etal_event_FM_stereo_start (%s, %d) for HD", ETAL_STATUS_toString(ret), ret);
							}
							else
							{
								etalTestPrintReportPassEnd(testPassed);
							}
						}
#endif
					}
					break;

				case DO_HD_AM:
					etalTestInfoFmStereoReceiverExpected = handlehdam;
					if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
					{
						ret = ETAL_RET_ERROR;
					}
					break;
			}
			if (ret != ETAL_RET_SUCCESS)
			{
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Select audio %s", etalTestStandard2String(mode[i]));
			etalTestPrintNormal("***********************************************");
			etalTestPrintNormal("\tYou should hear audio ...");
			etalTestPrintNormal("***********************************************");

			OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

			switch (action)
			{
				case DO_MUTE:
					/* this code executed for all broadcast standards */
					etalTestPrintReportPassStart(AUDIO_MUTE, mode[i], "Mute audio stream");
					if ((ret = etal_mute(hReceiver, true)) != ETAL_RET_SUCCESS)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("etal_mute (%s, %d)", ETAL_STATUS_toString(ret), ret);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					break;
				case DO_INFO_FM_STEREO:
#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
					/* this code not executed for DAB */
					etalTestPrintReportPassStart(AUDIO_CHECK_STEREO_EVENT, mode[i], "Check stereo event");
					if (etalTestInfoFmStereoCount != 1)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("event ETAL_INFO_FM_STEREO(%d, %d) not received", etalTestInfoFmStereoReceiverExpected, etalTestInfoFmStereoExpected);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					etalTestInfoFmStereoCount = 0;
					etalTestInfoFmStereoExpected = 0;
#endif
				case DO_MONO:
					/* this code not executed for DAB */
					etalTestPrintReportPassStart(AUDIO_SET_MONO, mode[i],"Set mono mode");
					if ((ret = etal_force_mono(hReceiver, true)) != ETAL_RET_SUCCESS)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("etal_force_mono (%s, %d)", ETAL_STATUS_toString(ret), ret);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					break;
				case DO_DIGITAL:
					/* this code executed for all broadcast standards */
					etalTestPrintReportPassStart(AUDIO_ENABLE_ANALOG, mode[i],"Enable analog audio stream");
					intf.m_dac = 1;
					intf.m_sai_in = intf.m_sai_out = 1;
					// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
					intf.m_sai_slave_mode = true;
#endif
					if ((ret = etal_config_audio_path(0, intf)) != ETAL_RET_SUCCESS)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
					}
					else
					{
#if ((defined(CONFIG_BOARD_ACCORDO2)) || (defined(CONFIG_BOARD_ACCORDO5)) && (!defined(CONFIG_HOST_OS_TKERNEL)))
						system("amixer -c 3 sset Source adcauxdac > /dev/null");

						// select the audio channel
						system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
						/* Select audio source */
						if ((ret = etal_audio_select(hReceiver, audioSrc)) != ETAL_RET_SUCCESS)
						{
							etalTestPrintError("etal_audio_select (%s, %d)", ETAL_STATUS_toString(ret), ret);
							return OSAL_ERROR;
						}
						etalTestPrintReportPassEnd(testPassed);
					}
					break;
			}

			etalTestPrintNormal("***********************************************");
			etalTestPrintNormal("\t%s", actionString[0]);
			etalTestPrintNormal("***********************************************");

			OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

			switch (action)
			{
				case DO_MUTE:
					/* this code executed for all broadcast standards */
					etalTestPrintReportPassStart(AUDIO_UNMUTE, mode[i], "Unmute audio stream");
					if ((ret = etal_mute(hReceiver, false)) != ETAL_RET_SUCCESS)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("etal_mute (%s, %d)", ETAL_STATUS_toString(ret), ret);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					break;
#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
				case DO_INFO_FM_STEREO:
					/* this code not executed for DAB */
					etalTestPrintReportPassStart(AUDIO_CHECK_STEREO_EVENTS, mode[i], "Check stereo events");
					if (etalTestInfoFmStereoCount != 1)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("event ETAL_INFO_FM_STEREO(%d, %d) not received", etalTestInfoFmStereoReceiverExpected, etalTestInfoFmStereoExpected);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					etalTestInfoFmStereoCount = 0;
					etalTestInfoFmStereoExpected = 1;
#endif
				case DO_MONO:
					/* this code not executed for DAB */
					etalTestPrintReportPassStart(AUDIO_SET_STEREO, mode[i], "Set stereo mode");
					if ((ret = etal_force_mono(hReceiver, false)) != ETAL_RET_SUCCESS)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("etal_force_mono (%s, %d)", ETAL_STATUS_toString(ret), ret);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					break;
				case DO_DIGITAL:
					/* this code executed for all broadcast standards */
					etalTestPrintReportPassStart(AUDIO_ENABLE_DIGITAL, mode[i], "Enable digital audio stream");
					intf.m_dac = 0;
					intf.m_sai_in = intf.m_sai_out = 1;
					// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
					intf.m_sai_slave_mode = TRUE;
#endif
					if ((ret = etal_config_audio_path(0, intf)) != ETAL_RET_SUCCESS)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
					}
					else
					{
						if ((ret = etal_audio_select(hReceiver, audioSrc)) != ETAL_RET_SUCCESS)
						{
							pass = FALSE;
							etalTestPrintReportPassEnd(testFailed);
							etalTestPrintNormal("etal_audio_select (%s, %d)", ETAL_STATUS_toString(ret), ret);
						}
						else
						{
							etalTestPrintReportPassEnd(testPassed);
						}
					}
					break;
			}

			etalTestPrintNormal("***********************************************");
			etalTestPrintNormal("\t%s", actionString[1]);
			etalTestPrintNormal("***********************************************");

			OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
			switch (action)
			{
				case DO_INFO_FM_STEREO:
					switch (i)
					{
						case DO_DAB:
						case DO_AM:
						case DO_HD_AM:
							break;
						case DO_FM:
							etalTestPrintReportPassStart(AUDIO_STOP_STEREO_EVENT, mode[i], "Stop stereo event");
							if ((ret = etal_event_FM_stereo_stop(handlefm)) != ETAL_RET_SUCCESS)
							{
								pass = FALSE;
								etalTestPrintReportPassEnd(testFailed);
								etalTestPrintNormal("etal_event_FM_stereo_stop (%s, %d) for FM", ETAL_STATUS_toString(ret), ret);
							}
							else
							{
								etalTestPrintReportPassEnd(testPassed);
							}
							break;
						case DO_HD_FM:
							etalTestPrintReportPassStart(AUDIO_STOP_STEREO_EVENT, mode[i], "Stop stereo event");
							if ((ret = etal_event_FM_stereo_stop(handlehd)) != ETAL_RET_SUCCESS)
							{
								pass = FALSE;
								etalTestPrintReportPassEnd(testFailed);
								etalTestPrintNormal("etal_event_FM_stereo_stop (%s, %d) for HD", ETAL_STATUS_toString(ret), ret);
							}
							else
							{
								etalTestPrintReportPassEnd(testPassed);
							}
							break;
					}
					etalTestPrintReportPassStart(AUDIO_COUNT_EVENTS, mode[i], "Count events after stop");
					if (etalTestInfoFmStereoCount != 1)
					{
						pass = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("event ETAL_INFO_FM_STEREO(%d, %d) not received", etalTestInfoFmStereoReceiverExpected, etalTestInfoFmStereoExpected);
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
					break;
			}
#endif
		}

		/* cleanup for next loop */

		if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	/* restore the analogue path */
	if (action == DO_DIGITAL)
	{
		intf.m_dac = 1;
		intf.m_sai_out = 0;
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
		intf.m_sai_slave_mode = TRUE;
#endif
		if ((ret = etal_config_audio_path(0, intf)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}
/* CONFIG_APP_TEST_DAB || CONFIG_APP_TEST_FM || CONFIG_APP_TEST_HDRADIO_FM */
#endif

/* CONFIG_APP_TEST_AUDIO */
#endif


/***************************
 *
 * etalTestAudioSelect
 *
 **************************/
tSInt etalTestAudioSelect(void)
{
#ifdef CONFIG_APP_TEST_AUDIO_SELECT
	tBool pass;
#ifdef CONFIG_APP_TEST_DAB
	ETAL_STATUS ret;
#endif

	etalTestStartup();
	pass = TRUE;

	/* pass1, verify wrong parameter processing */

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_ILLEGAL_SOURCE, ETAL_TEST_MODE_DAB, "Audio Select with illegal audio source");
	if (etal_audio_select(handledab, 0xFF) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
	etalTestPrintNormal("Select DAB audio service");
	ret = etal_service_select_audio(handledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID, ETAL_INVALID, ETAL_INVALID);
	if (ret != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(AUDIO_SELECT_SOURCE, ETAL_TEST_MODE_DAB, "Audio Select audio source");
	if (etal_audio_select(handledab, ETAL_AUDIO_SOURCE_DCOP_STA660) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some DAB audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
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

#ifdef CONFIG_APP_TEST_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_ILLEGAL_SOURCE, ETAL_TEST_MODE_FM, "Audio Select with illegal audio source");
	if (etal_audio_select(handlefm, 0xFF) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(AUDIO_SELECT_RCV, ETAL_TEST_MODE_FM, "Audio Select receiver");
	if (etal_audio_select(handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some FM audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
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
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_ILLEGAL_SOURCE, ETAL_TEST_MODE_HD_FM, "Audio Select with illegal audio source");
	if (etal_audio_select(handlehd, 0xFF) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1e FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(AUDIO_SELECT_RCV_FORCE_DIGITAL, ETAL_TEST_MODE_HD_FM, "Audio Select receiver (force digital audio)");
	if (etal_audio_select(handlehd, ETAL_AUDIO_SOURCE_AUTO_HD) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1f FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some HD audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_RCV_AUTO, ETAL_TEST_MODE_HD_FM, "Audio Select receiver (auto select digital/analog audio)");
	if (etal_audio_select(handlehd, ETAL_AUDIO_SOURCE_DCOP_STA680) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1g FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some HD audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
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
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_ILLEGAL_SOURCE, ETAL_TEST_MODE_HD_AM, "Audio Select with illegal audio source");
	if (etal_audio_select(handlehdam, 0xFF) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1h FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(AUDIO_SELECT_RCV_FORCE_DIGITAL, ETAL_TEST_MODE_HD_AM, "Audio Select receiver (force digital audio)");
	if (etal_audio_select(handlehdam, ETAL_AUDIO_SOURCE_AUTO_HD) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1i FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some HD audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_RCV_AUTO, ETAL_TEST_MODE_HD_AM, "Audio Select receiver (auto select digital/analog audio)");
	if (etal_audio_select(handlehdam, ETAL_AUDIO_SOURCE_DCOP_STA680) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1j FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some HD audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_AM

#ifdef CONFIG_APP_TEST_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_ILLEGAL_SOURCE, ETAL_TEST_MODE_AM, "Audio Select with illegal audio source");
	if (etal_audio_select(handleam, 0xFF) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1k FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	etalTestPrintReportPassStart(AUDIO_SELECT_RCV, ETAL_TEST_MODE_AM, "Audio Select receiver");
	if (etal_audio_select(handleam, ETAL_AUDIO_SOURCE_STAR_AMFM) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1l FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintNormal("Listen to some AM audio (5sec)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_AM

	if (!pass)
	{
		etalTestPrintNormal("pass1 FAILED");
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_AUDIO_SELECT
	return OSAL_OK;
}

/***************************
 *
 * etalTestAudio
 *
 **************************/
tSInt etalTestAudio(void)
{
#ifdef CONFIG_APP_TEST_AUDIO
#if ((defined(CONFIG_APP_TEST_DAB)) || (defined(CONFIG_APP_TEST_FM)) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM))
	tBool pass;

	etalTestStartup();
	pass = TRUE;

#ifdef CONFIG_APP_TEST_AUDIO_MUTE
	etalTestPrintNormal("Mute audio test start");
	if (etalTestMuteMonoDigital(DO_MUTE, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_AUDIO_MONO
	etalTestPrintNormal("");
	etalTestPrintNormal("Force mono test start");
	if (etalTestMuteMonoDigital(DO_MONO, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass2 FAILED");
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_AUDIO_DIGITAL
	etalTestPrintNormal("");
	etalTestPrintNormal("Digital audio test start");
	if (etalTestMuteMonoDigital(DO_DIGITAL, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass3 FAILED");
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
	etalTestPrintNormal("");
	etalTestPrintNormal("FM_STEREO audio test start");
	if (etalTestMuteMonoDigital(DO_INFO_FM_STEREO, &pass) != OSAL_OK)
	{
		etalTestPrintNormal("pass4 FAILED");
		return OSAL_ERROR;
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif
#else
	etalTestPrintNormal("Test skipped");
#endif

	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

