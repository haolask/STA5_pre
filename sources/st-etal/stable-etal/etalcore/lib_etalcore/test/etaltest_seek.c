//!
//!  \file 		etaltest_seek.c
//!  \brief 	<i><b> ETAL test, seek </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#ifdef CONFIG_APP_TEST_SEEK
etalTestEventCountTy etalTestSeekEventCountDAB;
etalTestEventCountTy etalTestSeekEventCountDAB1_5;
etalTestEventCountTy etalTestSeekEventCountFM;
etalTestEventCountTy etalTestSeekEventCountAM;
etalTestEventCountTy etalTestSeekEventCountHD;
tU32 etalTestSeekEstimatedDirection;

#define ETAL_TEST_DAB_SEEK_STEP_TIME        1500
#define ETAL_TEST_DAB_SEEK_BAND_TIME         (15 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME    (4 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_SEEK_COUNT                   2
#define ETAL_TEST_SEEK_HD_COUNT                4
#define ETAL_TEST_SEEK_FM_STEP_FREQ          100
#define ETAL_TEST_SEEK_AM_STEP_FREQ            1
#define ETAL_TEST_SEEK_AUDIO_LISTEN_HD_TIME   (5 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_SEEK_FM_FROM_HD_TIME        (600)

/*
 * Some tests verify whether the callback is called;
 * since the callback is called from a separate thread it needs
 * some time to be executed, this is the max time to wait
 */
#define ETAL_TEST_DAB_SEEK_CALLBACK_WAIT     500
#define ETAL_TEST_FM_SEEK_CALLBACK_WAIT     100

#ifdef CONFIG_APP_TEST_DAB
/***************************
 *
 * etalTestSeekDAB
 *
 **************************/
static tSInt etalTestSeekDAB(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;
	tBool pass = TRUE;
	tU8 i;
	etalSeekDirectionTy direction;

	if(hReceiver == handledab)
	{
#if 0
		for(i=0; i<30; i++)
		{
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}
		}
#else
		/* cmdAudioUnmuted mode */
		for(i=0; i<2; i++)
		{
			direction = (i == 0) ? cmdDirectionUp : cmdDirectionDown;

			/* Pass1, Start from an empty frequency, seek in Unmuted mode (autoseek must implicitly stops when a frequency is found) */
			/* etal_autoseek_continue must return an error */
			/* etal_autoseek_stop returns SUCCESS but the procedure is already stopped. */
			if(direction == cmdDirectionUp)
			{
				etalTestPrintNormal("test DAB Autoseek (seek up), handle %d", hReceiver);
			}
			else
			{
				etalTestPrintNormal("test DAB Autoseek (seek down), handle %d", hReceiver);
			}

			etalTestPrintNormal("Autoseek, pass1 (basic seek), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, dontSeekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_STEP_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
			{
				etalTestPrintError("etal_autoseek_continue DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass1 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}

			/* Pass2, Start from a valid frequency, seek in Unmuted mode until getting fullCycleReached */
			/* In this mode, autoseek stops when a frequency is found, to continue the seek, an etal_autoseek_start() */
			/* has to be called with updateFrequency set to FALSE */
			/* etal_autoseek_continue must return an error */
			/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
			etalTestPrintNormal("Autoseek, pass2 (DAB seek on the whole band), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Tune to valid DAB freq %d, handle %d", ETAL_VALID_DAB_FREQ, hReceiver);

			ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_FREQ);

			if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}

			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, FALSE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}

			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, FALSE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}

			/* Pass3, Start autoseek then stop it while in progress */
			/* An event SEEK_INFO FINISHED must be received on stop command */
			etalTestPrintNormal("Autoseek, pass3 (stop autoseek in progress), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
				OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass3 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}
		}

		/* cmdAudioMuted mode */
		for(i=0; i<2; i++)
		{
			direction = (i == 0) ? cmdDirectionUp : cmdDirectionDown;

			/* Pass11, Start from an empty frequency, seek in Muted mode (autoseek doesn't stop when a frequency is found) */
			/* etal_autoseek_continue is used to continue the seek */
			/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
			if(direction == cmdDirectionUp)
			{
				etalTestPrintNormal("test DAB Autoseek (seek up), handle %d", hReceiver);
			}
			else
			{
				etalTestPrintNormal("test DAB Autoseek (seek down), handle %d", hReceiver);
			}

			etalTestPrintNormal("Autoseek, pass11 (basic seek), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioMuted, dontSeekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}
			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_STEP_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass11 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}

			/* Pass12, Start from a valid frequency, seek in Muted mode until getting fullCycleReached */
			/* In this mode, autoseek doesn't stop when a frequency is found, to continue the seek, an etal_autoseek_continue() */
			/* has to be called and returned SUCCESS */
			/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
			etalTestPrintNormal("Autoseek, pass12 (DAB seek on the whole band), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Tune to valid DAB freq %d, handle %d", ETAL_VALID_DAB_FREQ, hReceiver);

			ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_FREQ);

			if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioMuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass12 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}

			/* Pass13, Start autoseek then stop it while in progress */
			/* An event SEEK_INFO FINISHED must be received on stop command */
			etalTestPrintNormal("Autoseek, pass13 (stop autoseek in progress), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioMuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
				OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB.STARTED != 1) ||
				(etalTestSeekEventCountDAB.RESULT < 1) ||
				(etalTestSeekEventCountDAB.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass13 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB.STARTED,
						etalTestSeekEventCountDAB.RESULT,
						etalTestSeekEventCountDAB.FINISHED);
			}
		}
#endif
	}
	else
	{
		/* cmdAudioUnmuted mode */
		for(i=0; i<2; i++)
		{
			direction = (i == 0) ? cmdDirectionUp : cmdDirectionDown;

			/* Pass1, Start from an empty frequency, seek in Unmuted mode (autoseek must implicitly stops when a frequency is found) */
			/* etal_autoseek_continue must return an error */
			/* etal_autoseek_stop returns SUCCESS but the procedure is already stopped. */
			if(direction == cmdDirectionUp)
			{
				etalTestPrintNormal("test DAB Autoseek (seek up), handle %d", hReceiver);
			}
			else
			{
				etalTestPrintNormal("test DAB Autoseek (seek down), handle %d", hReceiver);
			}

			etalTestPrintNormal("Autoseek, pass1 (basic seek), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_VALID_DAB_FREQ1_5, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_FREQ1_5)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, dontSeekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_STEP_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
			{
				etalTestPrintError("etal_autoseek_continue DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass1 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}

			/* Pass2, Start from a valid frequency, seek in Unmuted mode until getting fullCycleReached */
			/* In this mode, autoseek stops when a frequency is found, to continue the seek, an etal_autoseek_start() */
			/* has to be called with updateFrequency set to FALSE */
			/* etal_autoseek_continue must return an error */
			/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
			etalTestPrintNormal("Autoseek, pass2 (DAB seek on the whole band), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Tune to valid DAB freq %d, handle %d", ETAL_VALID_DAB_FREQ, hReceiver);

			ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_FREQ);

			if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}

			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, FALSE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}

			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, FALSE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}

			/* Pass3, Start autoseek then stop it while in progress */
			/* An event SEEK_INFO FINISHED must be received on stop command */
			etalTestPrintNormal("Autoseek, pass3 (stop autoseek in progress), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
				OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass3 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}
		}

		/* cmdAudioMuted mode */
		for(i=0; i<2; i++)
		{
			direction = (i == 0) ? cmdDirectionUp : cmdDirectionDown;

			/* Pass11, Start from an empty frequency, seek in Muted mode (autoseek doesn't stop when a frequency is found) */
			/* etal_autoseek_continue is used to continue the seek */
			/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
			if(direction == cmdDirectionUp)
			{
				etalTestPrintNormal("test DAB Autoseek (seek up), handle %d", hReceiver);
			}
			else
			{
				etalTestPrintNormal("test DAB Autoseek (seek down), handle %d", hReceiver);
			}

			etalTestPrintNormal("Autoseek, pass11 (basic seek), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioMuted, dontSeekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}
			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_STEP_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass11 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}

			/* Pass12, Start from a valid frequency, seek in Muted mode until getting fullCycleReached */
			/* In this mode, autoseek doesn't stop when a frequency is found, to continue the seek, an etal_autoseek_continue() */
			/* has to be called and returned SUCCESS */
			/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
			etalTestPrintNormal("Autoseek, pass12 (DAB seek on the whole band), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Tune to valid DAB freq %d, handle %d", ETAL_VALID_DAB_FREQ, hReceiver);

			ret = etal_tune_receiver(hReceiver, ETAL_VALID_DAB_FREQ);

			if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_NO_DATA))
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioMuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass12 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}

			/* Pass13, Start autoseek then stop it while in progress */
			/* An event SEEK_INFO FINISHED must be received on stop command */
			etalTestPrintNormal("Autoseek, pass13 (stop autoseek in progress), handle %d", hReceiver);
			etalTestResetEventCount(&etalTestSeekEventCountDAB1_5);

			etalTestPrintNormal("* Tune to empty DAB freq %d, handle %d", ETAL_EMPTY_DAB_FREQ, hReceiver);
			if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
			{
				etalTestPrintError("etal_tune_receiver DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_SEEK_STEP_UNDEFINED, cmdAudioMuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_start DAB (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME / ETAL_TEST_ONE_SECOND, hReceiver);
				OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_BAND_SHORT_TIME);

			etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
			if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_DAB_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
			OSAL_s32ThreadWait(ETAL_TEST_DAB_SEEK_CALLBACK_WAIT);

			if ((etalTestSeekEventCountDAB1_5.STARTED != 1) ||
				(etalTestSeekEventCountDAB1_5.RESULT < 1) ||
				(etalTestSeekEventCountDAB1_5.FINISHED != 1))
			{
				pass = FALSE;
				etalTestPrintNormal("pass13 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
						hReceiver,
						etalTestSeekEventCountDAB1_5.STARTED,
						etalTestSeekEventCountDAB1_5.RESULT,
						etalTestSeekEventCountDAB1_5.FINISHED);
			}
		}
	}

	if (pass == FALSE)
	{
		etalTestPrintNormal("DAB autoseek FAILED, handle %d", hReceiver);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_FM
static tSInt etalTestSeekFM(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;
	tBool pass = TRUE;
	tU8 i;
	etalSeekDirectionTy direction;
	EtalAudioInterfTy audioIf;
	EtalProcessingFeatures proc_features;

	/* cmdAudioUnmuted mode */
	for(i=0; i<2; i++)
	{
		direction = (i == 0) ? cmdDirectionUp : cmdDirectionDown;

		/* Pass1, Start from an empty frequency, seek in Unmuted mode (autoseek must implicitly stops when a frequency is found) */
		/* etal_autoseek_continue must return an error */
		/* etal_autoseek_stop returns SUCCESS but the procedure is already stopped. */
		if(direction == cmdDirectionUp)
		{
			etalTestPrintNormal("test FM Autoseek (seek up), handle %d", hReceiver);
		}
		else
		{
			etalTestPrintNormal("test FM Autoseek (seek down), handle %d", hReceiver);
		}

		etalTestPrintNormal("Autoseek, pass1 (basic seek), handle %d", hReceiver);
		etalTestResetEventCount(&etalTestSeekEventCountFM);

	    /* Change to FMUE band */
	    etalTestPrintNormal("* Set FMUE band");
	    proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	    if ((ret = etal_change_band_receiver(handlefm, ETAL_BAND_FMEU, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
	    {
	        etalTestPrintReportPassEnd(testFailed);
	        etalTestPrintError("etal_change_band_receiver FMUE band (%s, %d)", ETAL_STATUS_toString(ret), ret);
	        return OSAL_ERROR;
	    }

		/* Tune to valid FM freq */
		etalTestPrintNormal("* Tune to FM freq %d, handle %d", ETAL_EMPTY_FM_FREQ, hReceiver);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

	    /* Configure audio path */
	    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
		audioIf.m_dac = 0;
#endif
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
		audioIf.m_sai_slave_mode = true;
#endif

		/*
		 * Enable audio in case Seek test is performed without any preceding test
		 *
		 * During concurrent tests it may happen that etalTestSeekFM for the background CMOST
		 * tuner: in this case the audio_select is not supported so we must skip it
		 * or accept an error
		 */
	    /* Select audio source */
		ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM);
		if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_INVALID_RECEIVER))
		{
			etalTestPrintError("etal_audio_select FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, dontSeekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		// test continue is rejected 
		//
        if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
        {
        	etalTestPrintError("Autoseek FM, continue should be rejected (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
        	return OSAL_ERROR;
        }

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
		{
			etalTestPrintError("etal_autoseek_continue FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass1 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}

		/* Pass2, Start from a valid frequency, seek in Unmuted mode until getting fullCycleReached */
		/* In this mode, autoseek stops when a frequency is found, to continue the seek, an etal_autoseek_start() */
		/* has to be called with updateFrequency set to FALSE */
		/* etal_autoseek_continue must return an error */
		/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
		etalTestPrintNormal("Autoseek, pass2 (FM seek on the whole band), handle %d", hReceiver);
		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Tune to valid FM freq %d, handle %d", ETAL_VALID_FM_FREQ, hReceiver);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
		{
			etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}

		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, FALSE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
		{
			etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}

		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, FALSE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}

		/* Pass3, Start autoseek then stop it while in progress */
		/* An event SEEK_INFO FINISHED must be received on stop command */
		etalTestPrintNormal("Autoseek, pass3 (stop autoseek in progress), handle %d", hReceiver);
		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Tune to empty FM freq %d, handle %d", ETAL_EMPTY_FM_FREQ, hReceiver);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass3 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}
	}

	/* cmdAudioMuted mode */
	for(i=0; i<2; i++)
	{
		direction = (i == 0) ? cmdDirectionUp : cmdDirectionDown;

		/* Pass11, Start from an empty frequency, seek in Muted mode (autoseek doesn't stop when a frequency is found) */
		/* etal_autoseek_continue is used to continue the seek */
		/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
		if(direction == cmdDirectionUp)
		{
			etalTestPrintNormal("test FM Autoseek (seek up), handle %d", hReceiver);
		}
		else
		{
			etalTestPrintNormal("test FM Autoseek (seek down), handle %d", hReceiver);
		}

		etalTestPrintNormal("Autoseek, pass11 (basic seek), handle %d", hReceiver);
		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Tune to empty FM freq %d, handle %d", ETAL_EMPTY_FM_FREQ, hReceiver);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioMuted, dontSeekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass11 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}

		/* Pass12, Start from a valid frequency, seek in Muted mode until getting fullCycleReached */
		/* In this mode, autoseek doesn't stop when a frequency is found, to continue the seek, an etal_autoseek_continue() */
		/* has to be called and returned SUCCESS */
		/* etal_autoseek_stop returns SUCCESS even if the procedure is already stopped. */
		etalTestPrintNormal("Autoseek, pass12 (FM seek on the whole band), handle %d", hReceiver);
		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Tune to valid FM freq %d, handle %d", ETAL_VALID_FM_FREQ, hReceiver);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Autoseek the whole band and loop, handle %d", hReceiver);
		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioMuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Continue autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("***********************************************");
		etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
		etalTestPrintNormal("***********************************************");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass12 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}

		/* Pass13, Start autoseek then stop it while in progress */
		/* An event SEEK_INFO FINISHED must be received on stop command */
		etalTestPrintNormal("Autoseek, pass13 (stop autoseek in progress), handle %d", hReceiver);
		etalTestResetEventCount(&etalTestSeekEventCountFM);

		etalTestPrintNormal("* Tune to empty FM freq %d, handle %d", ETAL_EMPTY_FM_FREQ, hReceiver);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		if ((ret = etal_autoseek_start(hReceiver, direction, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioMuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start FM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* Stop autoseek, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		etalTestPrintNormal("Wait some time (%d sec), handle %d", ETAL_TEST_FM_SEEK_CALLBACK_WAIT / ETAL_TEST_ONE_SECOND, hReceiver);
		OSAL_s32ThreadWait(ETAL_TEST_FM_SEEK_CALLBACK_WAIT);

		if ((etalTestSeekEventCountFM.STARTED != 1) ||
			(etalTestSeekEventCountFM.RESULT < 1) ||
			(etalTestSeekEventCountFM.FINISHED != 1))
		{
			pass = FALSE;
			etalTestPrintNormal("pass13 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
					hReceiver,
					etalTestSeekEventCountFM.STARTED,
					etalTestSeekEventCountFM.RESULT,
					etalTestSeekEventCountFM.FINISHED);
		}
	}

	if (pass == FALSE)
	{
		etalTestPrintNormal("FM autoseek FAILED, handle %d", hReceiver);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_AM
/***************************
 *
 * etalTestSeekAM
 *
 **************************/
static tSInt etalTestSeekAM(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;
	tU32 count = 0;
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	tBool pass3 = TRUE;
    EtalAudioInterfTy audioIf;

	/* Tune to valid AM freq */

	etalTestPrintNormal("* Tune to AM freq %d, handle %d", ETAL_VALID_AM_FREQ, hReceiver);
	if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_AM_FREQ)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
		return OSAL_ERROR;
	}

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
	audioIf.m_dac = 0;
#endif
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

	/*
	 * Enable audio in case Seek test is performed without any preceding test
	 *
	 */
    /* Select audio source */
	ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM);
	if (ret != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
		return OSAL_ERROR;
	}

	/* Seek up with fixed frequency step */

	count = 0;
	etalTestResetEventCount(&etalTestSeekEventCountAM);

    while (count < ETAL_TEST_SEEK_COUNT)
	{
        etalSeekAudioTy mute = (count%2) ? cmdAudioUnmuted : cmdAudioMuted;

		etalTestPrintNormal("* Seek up starting from current frequency, %dKHz step, loop %d, handle %d", ETAL_TEST_SEEK_AM_STEP_FREQ, count, hReceiver);
		etalTestSeekEventCountAM.FREQ_FOUND = 0;
		etalTestSeekEventCountAM.FULL_CYCLE_REACHED = 0;
		if ((ret = etal_autoseek_start(hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_AM_STEP_FREQ, mute, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

		// test continue is rejected 
		//
        if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
        {
        	etalTestPrintError("etalTestSeekAM etal_autoseek_continue should be rejected (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
        	return OSAL_ERROR;
        }
		
        if(mute == cmdAudioMuted)
        {
            etalTestPrintNormal("***********************************************");
            etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
            etalTestPrintNormal("***********************************************");

 		// wait until freq found or full cycle reached
			while ((etalTestSeekEventCountAM.FREQ_FOUND == 0) && (TRUE != etalTestSeekEventCountAM.FULL_CYCLE_REACHED))
			{			
				OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
			}
				
        	if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
        	{
        		etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
				
        		return OSAL_ERROR;
        	}

            etalTestPrintNormal("***********************************************");
            etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
            etalTestPrintNormal("***********************************************");
        	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
        }
        else
        {
            etalTestPrintNormal("***********************************************");
            etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
            etalTestPrintNormal("***********************************************");

			while ((etalTestSeekEventCountAM.FREQ_FOUND == 0) && (TRUE != etalTestSeekEventCountAM.FULL_CYCLE_REACHED))
			{			
				OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
			}
			
        	if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
        	{
        		etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
        		return OSAL_ERROR;
        	}
        }

        etalTestPrintNormal("* Seek stopped, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}
		// listen to some audio
		OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);
		count++;
	}

    if ((etalTestSeekEventCountAM.STARTED != ETAL_TEST_SEEK_COUNT) ||
        (etalTestSeekEventCountAM.RESULT < ETAL_TEST_SEEK_COUNT) ||
        (etalTestSeekEventCountAM.FINISHED != ETAL_TEST_SEEK_COUNT))
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
                hReceiver,
                etalTestSeekEventCountAM.STARTED,
                etalTestSeekEventCountAM.RESULT,
                etalTestSeekEventCountAM.FINISHED);
    }
#if 0
    else
    {
        etalTestPrintVerbose("pass1 PASSED, handle %d", hReceiver);
    }
#endif

	/* Seek down with unspecified frequency step */

	count = 0;
	etalTestResetEventCount(&etalTestSeekEventCountAM);

    while (count < ETAL_TEST_SEEK_COUNT)
	{
        etalSeekAudioTy mute = (count%2) ? cmdAudioUnmuted : cmdAudioMuted;

		etalTestPrintNormal("* Seek down starting from current frequency, unspecified freq step, loop %d, handle %d", count, hReceiver);
		etalTestSeekEventCountAM.FREQ_FOUND = 0;
		etalTestSeekEventCountAM.FULL_CYCLE_REACHED = 0;
		if ((ret = etal_autoseek_start(hReceiver, cmdDirectionDown, ETAL_TEST_SEEK_AM_STEP_FREQ, mute, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}

        if(mute == cmdAudioMuted)
        {
            etalTestPrintNormal("***********************************************");
            etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
            etalTestPrintNormal("***********************************************");

			while ((etalTestSeekEventCountAM.FREQ_FOUND == 0) && (TRUE != etalTestSeekEventCountAM.FULL_CYCLE_REACHED))
			{			
				OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
			}

        	if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_SUCCESS)
        	{
        		etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
        		return OSAL_ERROR;
        	}

            etalTestPrintNormal("***********************************************");
            etalTestPrintNormal("\tYou should not hear audio if a frequency is found ...");
            etalTestPrintNormal("***********************************************");
        	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
        }
        else
        {
            etalTestPrintNormal("***********************************************");
            etalTestPrintNormal("\tYou should hear audio if a frequency is found ...");
            etalTestPrintNormal("***********************************************");

			while ((etalTestSeekEventCountAM.FREQ_FOUND == 0) && (TRUE != etalTestSeekEventCountAM.FULL_CYCLE_REACHED))
			{			
				OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
			}

        	if ((ret = etal_autoseek_continue(hReceiver)) != ETAL_RET_ERROR)
        	{
        		etalTestPrintError("etal_autoseek_continue (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
        		return OSAL_ERROR;
        	}
        }

        etalTestPrintNormal("* Seek stopped, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}
		// listen to some audio
		OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);
		count++;
	}

	if ((etalTestSeekEventCountAM.STARTED != ETAL_TEST_SEEK_COUNT) ||
	    (etalTestSeekEventCountAM.RESULT < ETAL_TEST_SEEK_COUNT) ||
	    (etalTestSeekEventCountAM.FINISHED != ETAL_TEST_SEEK_COUNT))
	{
	    pass2 = FALSE;
	    etalTestPrintNormal("pass2 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
	            hReceiver,
	            etalTestSeekEventCountAM.STARTED,
	            etalTestSeekEventCountAM.RESULT,
	            etalTestSeekEventCountAM.FINISHED);
	}
#if 0
	else
	{
	    etalTestPrintVerbose("pass2 PASSED, handle %d", hReceiver);
	}
#endif

	/* Seek down with unspecified frequency step */

	count = 0;
	etalTestResetEventCount(&etalTestSeekEventCountAM);

    while (count < ETAL_TEST_SEEK_COUNT)
	{
        etalSeekAudioTy mute = (count%2) ? cmdAudioUnmuted : cmdAudioMuted;

        etalTestPrintNormal("* Seek up starting from current frequency with abortion, loop %d, handle %d", count, hReceiver);

		etalTestSeekEventCountAM.FREQ_FOUND = 0;
		etalTestSeekEventCountAM.FULL_CYCLE_REACHED = 0;
		
		if ((ret = etal_autoseek_start(hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_AM_STEP_FREQ, mute, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

        etalTestPrintNormal("* Seek stopped, handle %d", hReceiver);
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, hReceiver);
			return OSAL_ERROR;
		}
		// listen to some audio
		OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);
		count++;
	}

    if ((etalTestSeekEventCountAM.STARTED != ETAL_TEST_SEEK_COUNT) ||
        (etalTestSeekEventCountAM.RESULT < ETAL_TEST_SEEK_COUNT) ||
        (etalTestSeekEventCountAM.FINISHED != ETAL_TEST_SEEK_COUNT))
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3 FAILED, handle %d : START %d, RESULT %d, FINISHED %d",
                hReceiver,
                etalTestSeekEventCountAM.STARTED,
                etalTestSeekEventCountAM.RESULT,
                etalTestSeekEventCountAM.FINISHED);
    }
#if 0
    else
    {
        etalTestPrintVerbose("pass3 PASSED, handle %d", hReceiver);
    }
#endif

	if (!pass1 || !pass2 || !pass3)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_AM

#ifdef CONFIG_APP_TEST_HDRADIO_FM
/***************************
 *
 * etalTestSeekHD_pass3
 *
 **************************/
static tBool etalTestSeekHD_pass3(ETAL_HANDLE hReceiver, etalSeekDirectionTy dir)
{
	ETAL_STATUS ret;
	tU32 count;
	tChar direction[10];
	tBool pass3 = TRUE;

	if (dir == cmdDirectionUp)
	{
		OSAL_szStringCopy(direction, "up");
	}
	else
	{
		OSAL_szStringCopy(direction, "down");
	}

	etalTestPrintNormal("* Pass3: Tune to valid HD freq %dKHz", ETAL_VALID_HD_FREQ);
	if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_HD_FREQ)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * ensure all the INFO TUNE event are delivered
	 */
	OSAL_s32ThreadWait(600);
	/*
	 * select first SPS */
	etalTestPrintNormal("* Service Select by SPS");
	if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ETAL_INVALID, ETAL_HD_SPS_FM, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return FALSE;
	}
	/*
	 * check (listen) if the SPS is present
	 */
	OSAL_s32ThreadWait(ETAL_TEST_SEEK_AUDIO_LISTEN_HD_TIME);

	count = 0;
	etalTestResetEventCount(&etalTestSeekEventCountHD);

	while (count < ETAL_TEST_SEEK_HD_COUNT)
	{
		etalTestPrintNormal("* Seek %s %dKHz freq step, loop %d", direction, ETAL_TEST_SEEK_FM_STEP_FREQ, count);
		if ((ret = etal_autoseek_start(hReceiver, dir, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

		OSAL_s32ThreadWait(ETAL_TEST_SEEK_FM_FROM_HD_TIME);

		etalTestPrintNormal("* Seek stop");
		if ((ret = etal_autoseek_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		// listen to some audio
		OSAL_s32ThreadWait(ETAL_TEST_SEEK_AUDIO_LISTEN_HD_TIME);
		count++;
	}
	/*
	 * From the SPS going up we expect to hit the next SPS (1 event)
	 * followed by an FM station (1 events)
	 */
	if (etalTestSeekEventCountHD.FINISHED != ETAL_TEST_SEEK_HD_COUNT)
	{
		etalTestPrintNormal("pass3a %s FAILED (received %d events, expected %d)", direction, etalTestSeekEventCountHD.FINISHED, ETAL_TEST_SEEK_HD_COUNT);
		pass3 = FALSE;
	}

	if (!pass3)
	{
		etalTestPrintNormal("pass3 %s FAILED", direction);
	}
	return pass3;
}

/***************************
 *
 * etalTestSeek_HDRADIO
 *
 **************************/
static tSInt etalTestSeek_HDRADIO(void)
{
	ETAL_STATUS ret;
	tU32 count = 0;
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	tBool pass3 = TRUE;
    EtalAudioInterfTy audioIf;

	if (ETAL_FE_FOR_HD_TEST != ETAL_FE_HANDLE_1)
	{
		/*
 	 	 * audio seek only make sense on INSTANCE_1
 		 */
		etalTestPrintNormal("Skipping HD seek on non-audio instance");
		return OSAL_OK;
	}

	/* Pass1: Tune to valid HD freq, seek up, expect SPS selected for the first two seek, then FM */

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

	/*
	 * Enable audio in case Seek test is performed without any preceding test
	 */
	if ((ret = etal_audio_select(handlehd, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_audio_select HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("* Pass1: Tune to HD freq %d", ETAL_VALID_HD_FREQ - 300);
	if ((ret = etal_tune_receiver(handlehd, (ETAL_VALID_HD_FREQ - 300))) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * ensure all the INFO TUNE event are delivered
	 */
	OSAL_s32ThreadWait(100);

	/* Pass1: seek up with fixed frequency step */

	/*
	 * the first two seeks switch to the SPS so are almost immediate
	 */
	count = 0;
	while (count < ETAL_TEST_SEEK_HD_COUNT)
	{
		etalTestResetEventCount(&etalTestSeekEventCountHD);
		etalTestPrintNormal("* Seek up starting from current frequency, %dKHz step, loop %d", ETAL_TEST_SEEK_FM_STEP_FREQ, count);
		if ((ret = etal_autoseek_start(handlehd, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		OSAL_s32ThreadWait(ETAL_TEST_SEEK_FM_FROM_HD_TIME);
		etalTestPrintNormal("* Seek stop");
		if ((ret = etal_autoseek_stop(handlehd, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		OSAL_s32ThreadWait(ETAL_TEST_SEEK_AUDIO_LISTEN_HD_TIME);
		/*
		 * the seek algo is not performed since we advance in the HD SPS
		 * thus only the etal_autoseek_stop generates the SEEK FINISHED event
		 */
		if (etalTestSeekEventCountHD.FINISHED != 1)
		{
			etalTestPrintNormal("pass1a FAILED (received %d events, expected 1)", etalTestSeekEventCountHD.FINISHED);
			pass1 = FALSE;
		}
		count++;
	}

	/*
	 * the next seek goes to FM
	 */
	count = 0;
	etalTestResetEventCount(&etalTestSeekEventCountHD);

	while (count < 1)
	{
		etalTestPrintNormal("* Seek up starting from current frequency, %dKHz step, loop %d", ETAL_TEST_SEEK_FM_STEP_FREQ, count + ETAL_TEST_SEEK_HD_COUNT);
		if ((ret = etal_autoseek_start(handlehd, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

		OSAL_s32ThreadWait(ETAL_TEST_SEEK_FM_FROM_HD_TIME);

		etalTestPrintNormal("* Seek stop");
		if ((ret = etal_autoseek_stop(handlehd, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		etalTestPrintNormal("Listen to some audio");
		OSAL_s32ThreadWait(ETAL_TEST_SEEK_AUDIO_LISTEN_HD_TIME);
		count++;
	}
	/*
	 * We expect one SEEK FINISHED from the etal_autoseek_stop
	 */
	if (etalTestSeekEventCountHD.FINISHED != 1)
	{
		etalTestPrintNormal("pass1b FAILED (received %d events, expected 1)", etalTestSeekEventCountHD.FINISHED);
		pass1 = FALSE;
	}

	if (!pass1)
	{
		etalTestPrintNormal("pass1 FAILED");
	}

	/*
	 * Pass2: Tune to freq slightly higher than HD, Seek down, expect MPS selected then FM selected
	 * This complies with the iBiquity seek algo, when seeking down only stop on the MPS (skip the SPS)
	 */

	etalTestPrintNormal("* Pass2: Tune to HD freq %d", ETAL_VALID_HD_FREQ + 300);
	if ((ret = etal_tune_receiver(handlehd, ETAL_VALID_HD_FREQ + 300)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	count = 0;
	etalTestResetEventCount(&etalTestSeekEventCountHD);

	while (count < ETAL_TEST_SEEK_HD_COUNT)
	{
		etalTestPrintNormal("* Pass2: Seek down starting from current, %dKHz freq step, loop %d", ETAL_TEST_SEEK_FM_STEP_FREQ, count);
		if ((ret = etal_autoseek_start(handlehd, cmdDirectionDown, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_start HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

		OSAL_s32ThreadWait(ETAL_TEST_SEEK_FM_FROM_HD_TIME);

		if ((ret = etal_autoseek_stop(handlehd, lastFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_autoseek_stop HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		OSAL_s32ThreadWait(ETAL_TEST_SEEK_AUDIO_LISTEN_HD_TIME);
		count++;
	}
	/*
	 * Exect two events for the first step and two for the second step
	 */
	if (etalTestSeekEventCountHD.FINISHED < ETAL_TEST_SEEK_HD_COUNT)
	{
		etalTestPrintNormal("pass2a FAILED (received %d event, expected %d)", etalTestSeekEventCountHD.FINISHED, ETAL_TEST_SEEK_HD_COUNT * 2);
		pass2 = FALSE;
	}

	if (!pass2)
	{
		etalTestPrintNormal("pass2 FAILED");
	}

	/* Pass3: Tune to HD, select SPS1, seek up, expect MPS, seek up again, expect FM */
	if (etalTestSeekHD_pass3(handlehd, cmdDirectionUp) != TRUE)
	{
		pass3 = FALSE;
	}
	if (etalTestSeekHD_pass3(handlehd, cmdDirectionDown) != TRUE)
	{
		pass3 = FALSE;
	}

	/* Tune to invalid HD freq */

	etalTestPrintNormal("* Tune to empty HD freq (%d)", ETAL_EMPTY_FM_FREQ);
	if ((ret = etal_tune_receiver(handlehd, ETAL_EMPTY_FM_FREQ)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (!pass1 || !pass2 || !pass3)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
/* CONFIG_APP_TEST_HDRADIO_FM */
#endif
/* CONFIG_APP_TEST_SEEK */
#endif

/***************************
 *
 * etalTestSeek
 *
 **************************/
tSInt etalTestSeek(void)
{
#ifdef CONFIG_APP_TEST_SEEK
	tBool pass1 = TRUE;
#if !defined (CONFIG_APP_TEST_ONLY_SEQUENTIAL)
	etalTestThreadAttrTy thread1_attr, thread2_attr;
	tSInt retval1, retval2;
#endif

	etalTestStartup();

#ifndef CONFIG_APP_TEST_ONLY_CONCURRENT

	etalTestPrintNormal("<---Seek Test (sequential) start");

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestSeekDAB(handledab) != OSAL_OK)
	{
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestSeekDAB FAILED, handle %d", handledab);
        etalTestPrintNormal("****************************");
	    pass1 = FALSE;
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

	if (etalTestSeekFM(handlefm) != OSAL_OK)
	{
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestSeekFM FAILED, handle %d", handlefm);
        etalTestPrintNormal("****************************");
		pass1 = FALSE;
	}

	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestSeekAM(handleam) != OSAL_OK)
	{
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestSeekAM FAILED, handle %d", handleam);
        etalTestPrintNormal("****************************");
		pass1 = FALSE;
	}

	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_AM

#endif // CONFIG_APP_TEST_ONLY_CONCURRENT

#ifndef CONFIG_APP_TEST_ONLY_SEQUENTIAL
#if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB)

	etalTestPrintNormal("<---Seek Test (concurrent FM) start");

	if (etalTestDoConfigSingle(ETAL_CONFIG_FM2, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	thread1_attr.entry = etalTestSeekFM;
	thread1_attr.hReceiver = handledab; // DAB handle used for FM
	thread1_attr.status = THREAD_RUNNING;
	thread1_attr.retval = OSAL_OK;

	thread2_attr.entry = etalTestSeekFM;
	thread2_attr.hReceiver = handlefm;
	thread2_attr.status = THREAD_RUNNING;
	thread2_attr.retval = OSAL_OK;

	if ((etalTestThreadSpawn(&thread1_attr) != OSAL_OK) || 
		(etalTestThreadSpawn(&thread2_attr) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
	retval1 = etalTestThreadWait(&thread1_attr);
	retval2 = etalTestThreadWait(&thread2_attr);
	if ((retval1 != OSAL_OK) || (retval2 != OSAL_OK))
	{
		pass1 = FALSE;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM && !CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_AM
    if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    if (etalTestSeekAM(handleam) != OSAL_OK)
    {
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestSeekAM FAILED, handle %d", handleam);
        etalTestPrintNormal("****************************");
        pass1 = FALSE;
    }

    if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_AM

#if defined (CONFIG_APP_TEST_DAB)

	etalTestPrintNormal("<---Seek Test (2 concurrent DAB) start");

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB1_5, &handledab1_5) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	thread1_attr.entry = etalTestSeekDAB;
	thread1_attr.hReceiver = handledab1_5;
	thread1_attr.status = THREAD_RUNNING;
	thread1_attr.retval = OSAL_OK;

	thread2_attr.entry = etalTestSeekDAB;
	thread2_attr.hReceiver = handledab;
	thread2_attr.status = THREAD_RUNNING;
	thread2_attr.retval = OSAL_OK;

	if ((etalTestThreadSpawn(&thread1_attr) != OSAL_OK) || 
		(etalTestThreadSpawn(&thread2_attr) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
	retval1 = etalTestThreadWait(&thread1_attr);
	retval2 = etalTestThreadWait(&thread2_attr);
	if ((retval1 != OSAL_OK) || (retval2 != OSAL_OK))
	{
		pass1 = FALSE;
	}

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab1_5) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DAB

#if defined (CONFIG_APP_TEST_DAB) && defined (CONFIG_APP_TEST_FM)

	etalTestPrintNormal("<---Seek Test (concurrent DAB and FM) start");

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	thread1_attr.entry = etalTestSeekDAB;
	thread1_attr.hReceiver = handledab;
	thread1_attr.status = THREAD_RUNNING;
	thread1_attr.retval = OSAL_OK;

	thread2_attr.entry = etalTestSeekFM;
	thread2_attr.hReceiver = handlefm;
	thread2_attr.status = THREAD_RUNNING;
	thread2_attr.retval = OSAL_OK;

	if ((etalTestThreadSpawn(&thread1_attr) != OSAL_OK) ||
		(etalTestThreadSpawn(&thread2_attr) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
	retval1 = etalTestThreadWait(&thread1_attr);
	retval2 = etalTestThreadWait(&thread2_attr);
	if ((retval1 != OSAL_OK) || (retval2 != OSAL_OK))
	{
		pass1 = FALSE;
	}

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DAB && CONFIG_APP_TEST_FM

#endif // !CONFIG_APP_TEST_ONLY_SEQUENTIAL
	if (!pass1)
	{
		return OSAL_ERROR;
	}

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestSeek_HDRADIO() == OSAL_ERROR)
	{
		return OSAL_ERROR;
	}

	/* Destroy the receivers */

	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM
#endif // CONFIG_APP_TEST_SEEK

	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST
