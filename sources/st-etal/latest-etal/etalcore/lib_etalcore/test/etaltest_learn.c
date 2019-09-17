//!
//!  \file         etaltest_learn.c
//!  \brief     <i><b> ETAL test, learn </b></i>
//!  \details   Simulates the ETAL User application
//!  \author     Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltml_api.h"
#include "etaltest.h"

#define ETAL_TEST_LEARN_FM_STEP_FREQ          100
#define ETAL_TEST_LEARN_AM_STEP_FREQ          10

/* Time to perform learn on the whole DAB, AM, FM and HD bands:
 * this depends on the number of good stations
 * so it is site-dependent
 */
#define ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME   (40 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME    (10 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME    (20 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME    (10 * ETAL_TEST_ONE_SECOND)

/* Less then the time to perform the whole AM, FM and HD learn.
 * Used to check it the learn stop command is effective:
 * the number of stations found in this interval must be less than
 * ETAL_TEST_xx_LEARN_LONG_FREQ_FOUND
 */
#ifdef CONFIG_APP_TEST_IN_LE_MANS
#define ETAL_TEST_DAB_LEARN_PARTIAL_BAND_TIME  (4 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_FM_LEARN_PARTIAL_BAND_TIME   (4 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_HD_LEARN_PARTIAL_BAND_TIME   (8 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_AM_LEARN_PARTIAL_BAND_TIME   (4 * ETAL_TEST_ONE_SECOND)
#else
#define ETAL_TEST_DAB_LEARN_PARTIAL_BAND_TIME  (4 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_FM_LEARN_PARTIAL_BAND_TIME   (4 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_HD_LEARN_PARTIAL_BAND_TIME   (4 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_AM_LEARN_PARTIAL_BAND_TIME   (4 * ETAL_TEST_ONE_SECOND)
#endif

/* When learn is stopped in HD an HD tune takes some time blocking the callback event
 * so time has to be given to wait for the event.
 */
#define ETAL_TEST_LEARN_HD_TUNE_TIMEOUT      8000

/* DAB tests normally run from the signal generator so
 * expect only one good signal found
 */
#define ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND   1
#define ETAL_TEST_DAB_LEARN_EVENT_COUNT      1

/* FM tests run from the air so this value may need to be
 * tuned for different sites
 */
#ifdef CONFIG_APP_TEST_IN_LE_MANS
#define ETAL_TEST_FM_LEARN_LONG_FREQ_FOUND     7
#define ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND     7
#define ETAL_TEST_AM_LEARN_LONG_FREQ_FOUND     7
#define ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND    5
#define ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND    5
#define ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND    1
#else
#define ETAL_TEST_FM_LEARN_LONG_FREQ_FOUND     10
#define ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND     10
#define ETAL_TEST_AM_LEARN_LONG_FREQ_FOUND     10
#define ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND    3
#define ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND    5
#define ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND    1
#endif

/*
 * Some tests verify whether the callback is called;
 * since the callback is called from a separate thread it needs
 * some time to be executed, this is the max time to wait
 */
 // for Learn, callback can be relatively long :
 // retune on initial frequency potentially (termination mode)
 // + retune on 1st freq found ??
 // increase from 2 to 3
#define ETAL_TEST_LEARN_CALLBACK_WAIT     3000

etalTestEventCountTy etalTestLearnCountDAB;
etalTestEventCountTy etalTestLearnCountFM;
etalTestEventCountTy etalTestLearnCountAM;
etalTestEventCountTy etalTestLearnCountHD;

OSAL_tSemHandle etalTestLearnSem;
tBool vl_etalTestLearn_LearnOnGoing = false;

/***************************
 * function prototypes
 **************************/
tSInt etalTestLearnDAB(ETAL_HANDLE handledab);
tSInt etalTestLearnFM(ETAL_HANDLE hReceiver, etalTestBroadcastTy test_type);
tSInt etalTestLearnHD(ETAL_HANDLE hReceiver, etalTestBroadcastTy test_type);
tSInt etalTestLearnAM(ETAL_HANDLE handleam);

/***************************
 *
 * etalTestLearnDAB
 *
 **************************/
tSInt etalTestLearnDAB(ETAL_HANDLE handledab)
{
#ifdef CONFIG_APP_TEST_LEARN
    tBool pass1 = TRUE;
    tBool pass2 = TRUE;
    tBool pass3 = TRUE;

#ifdef CONFIG_APP_TEST_DAB
    ETAL_STATUS ret;
    EtalLearnFrequencyTy freqList[ETAL_LEARN_MAX_NB_FREQ];

    memset(freqList, 0, ETAL_LEARN_MAX_NB_FREQ * sizeof(EtalLearnFrequencyTy));
    
    etalTestPrintNormal("Learn on DAB test start");

    /*
     * pass1, plain learn command
     */
    
    etalTestPrintNormal("Learn, pass1");

    etalTestResetEventCount(&etalTestLearnCountDAB);

    etalTestPrintNormal("* Learn start");
    if ((ret = etaltml_learn_start(handledab, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /*
     * Long delay to cope with the almost empty DAB broadcast used during the tests with
     * a single signal generator
     */
    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME);

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountDAB.aux, ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND,
    		etalTestLearnCountDAB.STARTED, etalTestLearnCountDAB.RESULT,
    		etalTestLearnCountDAB.FINISHED, etalTestLearnCountDAB.NB_ERROR);

    if ((etalTestLearnCountDAB.STARTED != ETAL_TEST_DAB_LEARN_EVENT_COUNT) ||
        (etalTestLearnCountDAB.FINISHED != ETAL_TEST_DAB_LEARN_EVENT_COUNT) ||
        (etalTestLearnCountDAB.aux < ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND))
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1 FAILED");

		// note that if pass 1 fails => we should stop the learn to get a chance to have pass 2 running !!!
	    etalTestPrintNormal("* stop the learn");
	    if ((ret = etaltml_learn_stop(handledab, lastFrequency)) != ETAL_RET_SUCCESS)
	    {
	        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
	        return OSAL_ERROR;
	    }		
		// wait the end of the learn
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
    }

    /*
     * pass2, verify commands given during learn or with wrong parameters are rejected
     */

    etalTestPrintNormal("Learn, pass2");

    etalTestResetEventCount(&etalTestLearnCountDAB);

    etalTestPrintNormal("* Learn start with wrong parameter");
    if (etaltml_learn_start(handledab, ETAL_BAND_UNDEF, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList) != ETAL_RET_PARAMETER_ERR)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2a FAILED");
    }

    etalTestPrintNormal("* Learn start with illegal receiver");
    if (etaltml_learn_start(ETAL_INVALID_HANDLE, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList) != ETAL_RET_INVALID_HANDLE)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2b FAILED");
    }

    etalTestPrintNormal("* Learn start");
    if ((ret = etaltml_learn_start(handledab, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("* Learn start while learn is already in progress");
    if (etaltml_learn_start(handledab, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList) != ETAL_RET_IN_PROGRESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2c FAILED");
    }

    etalTestPrintNormal("* Tune receiver while performing learn");
    if (etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ) == ETAL_RET_SUCCESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2d FAILED");
    }

    /*
     * Let the learn started earlier finish and check if the stations are found.
     * Use a long delay to cope with the almost empty DAB broadcast used during the tests with
     * a single signal generator
     */
    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME);

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountDAB.aux, ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND,
    		etalTestLearnCountDAB.STARTED, etalTestLearnCountDAB.RESULT,
    		etalTestLearnCountDAB.FINISHED, etalTestLearnCountDAB.NB_ERROR);

    /* Check reported events */
    if ((etalTestLearnCountDAB.STARTED != ETAL_TEST_DAB_LEARN_EVENT_COUNT) ||
        (etalTestLearnCountDAB.aux < ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND))
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2e FAILED");
    }

    /*
     * pass3, stop and restart learn
     */

    etalTestPrintNormal("Learn, pass3");

    etalTestResetEventCount(&etalTestLearnCountDAB);

    etalTestPrintNormal("* Learn start with abortion");
    if ((ret = etaltml_learn_start(handledab, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /*
     * Short delay, not sufficient to scan the whole band, just to make sure
     * that when we issue the etaltml_learn_stopstd the learn is still in progress
     */
    etalTestPrintNormal("Wait some time (%d sec)", (ETAL_TEST_DAB_LEARN_PARTIAL_BAND_TIME) / ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_DAB_LEARN_PARTIAL_BAND_TIME);

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(handledab, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

	// only frequency reported/found on field are available
	// no min constraint
    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountDAB.aux, 0, //ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND,
    		etalTestLearnCountDAB.STARTED, etalTestLearnCountDAB.RESULT,
    		etalTestLearnCountDAB.FINISHED, etalTestLearnCountDAB.NB_ERROR);

    /*
     * If the learn in pass1 concluded correctly the MDR will report 1 ensemble found
     * even if the learn just concluded did not find any.
     */
    if ((etalTestLearnCountDAB.FINISHED != ETAL_TEST_DAB_LEARN_EVENT_COUNT) 
//		||        (etalTestLearnCountDAB.aux < ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND)
        )
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3a FAILED");
    }

    etalTestResetEventCount(&etalTestLearnCountDAB);

    etalTestPrintNormal("* Learn start on short interval");
    if ((ret = etaltml_learn_start(handledab, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /*
     * Short delay, not sufficient to scan the whole band, just to make sure
     * that when we issue the etaltml_learn_stopstd the learn is still in progress
     */
    etalTestPrintNormal("Wait some time (%d sec)", (ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME / 8) / ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME / 4);

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(handledab, initialFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

	// only frequency reported/found on field are available
	// no min constraint

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountDAB.aux, 0, //ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND,
    		etalTestLearnCountDAB.STARTED, etalTestLearnCountDAB.RESULT,
    		etalTestLearnCountDAB.FINISHED, etalTestLearnCountDAB.NB_ERROR);

    /*
     * If the learn in pass1 concluded correctly the MDR will report 1 ensemble found
     * even if the learn just concluded did not find any.
     */
    if ((etalTestLearnCountDAB.FINISHED != ETAL_TEST_DAB_LEARN_EVENT_COUNT)
//		||(etalTestLearnCountDAB.aux < ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND)
		)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3b FAILED");
    }

    /*
     * Restart the learn, this time allowing the time for the complete scan
     */
    etalTestPrintNormal("* Learn on BAND3, long interval");
    etalTestPrintNormal("* Learn start");

    etalTestResetEventCount(&etalTestLearnCountDAB);

    if ((ret = etaltml_learn_start(handledab, ETAL_BAND_DAB3, ETAL_SEEK_STEP_UNDEFINED, 5, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /*
     * Long delay to cope with the almost empty DAB broadcast used during the tests with
     * a single signal generator
     */
    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_DAB_LEARN_WHOLE_BAND_TIME);

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountDAB.aux, ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND,
    		etalTestLearnCountDAB.STARTED, etalTestLearnCountDAB.RESULT,
    		etalTestLearnCountDAB.FINISHED, etalTestLearnCountDAB.NB_ERROR);

    /* Check reported events */
    if ((etalTestLearnCountDAB.STARTED != ETAL_TEST_DAB_LEARN_EVENT_COUNT) ||
        (etalTestLearnCountDAB.FINISHED != ETAL_TEST_DAB_LEARN_EVENT_COUNT) ||
        (etalTestLearnCountDAB.aux < ETAL_TEST_DAB_LEARN_MIN_FREQ_FOUND))
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3c FAILED");
    }
#endif // CONFIG_APP_TEST_DAB

    if (!pass1 || !pass2 || !pass3)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_LEARN
    return OSAL_OK;
}

/***************************
 *
 * etalTestLearnFM
 *
 **************************/
tSInt etalTestLearnFM(ETAL_HANDLE hReceiver, etalTestBroadcastTy test_type)
{
#ifdef CONFIG_APP_TEST_LEARN
    tBool pass1 = TRUE;
    tBool pass2 = TRUE;
    tBool pass3 = TRUE;

#if defined (CONFIG_APP_TEST_FM)
    ETAL_STATUS ret;
    EtalLearnFrequencyTy freqList[ETAL_LEARN_MAX_NB_FREQ];
    EtalAudioInterfTy audioIf;
    tU32 freq;
    tChar *std;
    etalTestEventCountTy *count;
    EtalProcessingFeatures proc_features;

    if (test_type == ETAL_TEST_MODE_FM)
    {
        freq = ETAL_VALID_FM_FREQ;
        std = "FM";
        count = &etalTestLearnCountFM;
    }
    else
    {
        return OSAL_ERROR;
    }

    etalTestPrintNormal("Learn on %s test start", std);

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
    etalTestPrintNormal("* Tune to %s freq %d, handle %d", std, freq, hReceiver);
    if ((ret = etal_tune_receiver(hReceiver, freq)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_tune_receiver %s (%s, %d), handle %d", std, ETAL_STATUS_toString(ret), ret, hReceiver);
        return OSAL_ERROR;
    }

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
	if ((test_type == ETAL_TEST_MODE_FM) || (test_type == ETAL_TEST_MODE_AM) || (test_type == ETAL_TEST_MODE_DAB))
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
    // Audio path should be correctly set before

    /* Select audio source */
    ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_STAR_AMFM);
    if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_INVALID_RECEIVER))
    {
        etalTestPrintError("etal_audio_select %s (%s, %d), handle %d", std, ETAL_STATUS_toString(ret), ret, hReceiver);
        return OSAL_ERROR;
    }

    OSAL_s32ThreadWait(1 * ETAL_TEST_ONE_SECOND);

    memset(freqList, 0, ETAL_LEARN_MAX_NB_FREQ * sizeof(EtalLearnFrequencyTy));

    /* pass1, plain learn command */

    etalTestPrintNormal("Learn, pass1");

    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start limited to %d freq, sorted mode", ETAL_LEARN_MAX_NB_FREQ);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* The test does not use an OSAL_s32ThreadWait but rather sets up a semaphore
     * which is released by the UserNotificationHandler upon detection of the
     * learn FINISH event. Thus the wait time may be less than the
     * predefined ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME (unlike the DAB test,
     * where the time is fixed)
     */
    etalTestPrintNormal("Wait whole band time (%d sec or less)", ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME);

   // mark the end of the learn
   // to avoid further event posted in cross case and on Learn interruption
   vl_etalTestLearn_LearnOnGoing = false;


    /* Check reported events */
    if (count->STARTED != 1)
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1a FAILED");
    }


    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_FM_LEARN_LONG_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* Check reported events */
    if ((count->FINISHED != 1) || (count->NB_ERROR != 0))
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1b FAILED");
    }
    else if (count->aux < ETAL_TEST_FM_LEARN_LONG_FREQ_FOUND)
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1c FAILED");
    }
    /* pass2, verify commands given during learn or with wrong parameters are rejected */

    etalTestPrintNormal("Learn, pass2");

    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start with NULL array parameter");
    if (etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, NULL) != ETAL_RET_PARAMETER_ERR)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2a FAILED");
    }
    etalTestPrintNormal("* Learn start with invalid band parameter, SKIPPED");
    if (0/*etaltml_learn_start(hReceiver, ETAL_BAND_UNDEF, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, freqList) != ETAL_RET_PARAMETER_ERR*/)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2b FAILED");
    }

    etalTestPrintNormal("* Learn start with illegal number of freq parameter");
    if (etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ + 1, normalMode, freqList) != ETAL_RET_PARAMETER_ERR)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2c FAILED");
    }

    etalTestPrintNormal("* Learn start with illegal receiver");
    if (etaltml_learn_start(ETAL_INVALID_HANDLE, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, freqList) != ETAL_RET_INVALID_HANDLE)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2d FAILED");
    }

    etalTestPrintNormal("* Learn start limited to %d freq, sorted mode", ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("* Learn start while learn is already in progress");
    if (etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND, sortMode, freqList) != ETAL_RET_IN_PROGRESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2e FAILED");
    }

    etalTestPrintNormal("* Tune while performing learn");
    if (etal_tune_receiver(hReceiver, freq) == ETAL_RET_SUCCESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2f FAILED");
    }

    /* We issued a number of learn start commands but we expect only one to
     * succeed; we check that by looking a the event counter.
     * But we need to allow some time to ensure the callback was invoked and updates
     * the counters:
     */
    etalTestPrintNormal("Wait short time (%d sec or less)", ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;



    /* Expect only one started event, all other commands should have failed */
    if (count->STARTED != 1)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2g FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* last learn was requested to stop after ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND,
     * check if it was respected
     */
    if ((count->FINISHED != 1) ||
        (count->aux != ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND) ||
        (count->NB_ERROR != 0))
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2h FAILED");
    }

    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start limited to %d freq, sorted mode", ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* We issued a number of learn start commands but we expect only one to
     * succeed; we check that by looking a the event counter.
     * But we need to allow some time to ensure the callback was invoked and updates
     * the counters:
     */
    etalTestPrintNormal("Wait short time (%d sec or less)", ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;



    /* Expect only one started event, all other commands should have failed */
    if (count->STARTED != 1)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2i FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* last learn was requested to stop after ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND,
     * check if it was respected
     */
    if ((count->FINISHED != 1) ||
        (count->aux != ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND) ||
        (count->NB_ERROR != 0))
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2j FAILED");
    }

    /* pass3, stop and restart learn */

    etalTestPrintNormal("Learn, pass3");

    etalTestResetEventCount(count);

    /* start learn for the whole band and stop it before it has time to
     * complete the band scan; expect less than the max number of stations found
     */
    etalTestPrintNormal("* Learn start with abortion");

    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    etalTestPrintNormal("Wait short time (%d sec)", ETAL_TEST_FM_LEARN_PARTIAL_BAND_TIME / ETAL_TEST_ONE_SECOND);

    /* wait less then the minimum time required to scan the whole band
     * Don't use sem wait here! */
    OSAL_s32ThreadWait(ETAL_TEST_FM_LEARN_PARTIAL_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, less than %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_FM_LEARN_LONG_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    if (count->STARTED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3a FAILED");
    }
    /* we stopped the learn prematurely so we don't expect the ETAL_TEST_FM_LEARN_LONG_FREQ_FOUND max number of
     * stations to be found
     */
    if ((count->FINISHED != 1) || (count->NB_ERROR != 0))
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3b FAILED");
    }

    /* the UserNotificationHandler posted this semaphore upon reception
     * of the etaltml_learn_stop; since we did not yet use it in the test
     * (it used the OSAL_s32ThreadWait), we need to reset it otherwise the
     * next test will find it already available and exit prematurely
     */
     	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_FM_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    /* Restart the learn, this time allowing the time for the complete scan
     * We want to be sure that the previous etaltml_learn_stop while learn
     * was in progress cleanly stopped the learn state machine
     */
    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start limited to %d freq, normal mode", ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("Wait short time (%d sec or less)", ETAL_TEST_FM_LEARN_PARTIAL_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_FM_LEARN_PARTIAL_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    if (count->STARTED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3c FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_FM_LEARN_SHORT_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* Check reported events */
    if ((count->FINISHED != 1) || (count->NB_ERROR != 0))
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3d FAILED");
    }

#endif // CONFIG_APP_TEST_FM

    if (!pass1 || !pass2 || !pass3)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_LEARN
    return OSAL_OK;
}


/***************************
 *
 * etalTestLearnHD
 *
 **************************/
tSInt etalTestLearnHD(ETAL_HANDLE hReceiver, etalTestBroadcastTy test_type)
{
#ifdef CONFIG_APP_TEST_LEARN
    tBool pass1 = TRUE;
    tBool pass2 = TRUE;
    tBool pass3 = TRUE;

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
    ETAL_STATUS ret;
    EtalLearnFrequencyTy freqList[ETAL_LEARN_MAX_NB_FREQ];
    EtalAudioInterfTy audioIf;
    tU32 freq;
    tChar *std;
    etalTestEventCountTy *count;

    if (test_type == ETAL_TEST_MODE_HD_FM)
    {
        freq = ETAL_VALID_HD_FREQ;
        std = "HD";
        count = &etalTestLearnCountHD;
    }
    else
    {
        return OSAL_ERROR;
    }

    etalTestPrintNormal("Learn on %s test start", std);

    /* Tune to valid FM freq */
    etalTestPrintNormal("* Tune to %s freq %d, handle %d", std, freq, hReceiver);
    if ((ret = etal_tune_receiver(hReceiver, freq)) != ETAL_RET_SUCCESS)
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

    OSAL_s32ThreadWait(1 * ETAL_TEST_ONE_SECOND);

    memset(freqList, 0, ETAL_LEARN_MAX_NB_FREQ * sizeof(EtalLearnFrequencyTy));

    /* pass1, plain learn command */

    etalTestPrintNormal("Learn, pass1");

    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start limited to %d freq, sorted mode", ETAL_LEARN_MAX_NB_FREQ);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* The test does not use an OSAL_s32ThreadWait but rather sets up a semaphore
     * which is released by the UserNotificationHandler upon detection of the
     * learn FINISH event. Thus the wait time may be less than the 
     * predefined ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME (unlike the DAB test,
     * where the time is fixed)
     */
    etalTestPrintNormal("Wait whole band time (%d sec or less)", ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME);
   
   // mark the end of the learn
   // to avoid further event posted in cross case and on Learn interruption
   vl_etalTestLearn_LearnOnGoing = false;


    /* Check reported events */
    if (count->STARTED != 1)
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1a FAILED");
    }

   
    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_HD_TUNE_TIMEOUT);

    etalTestPrintNormal("= Learn found %d good frequencies, min %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* Check reported events */
    if ((count->FINISHED != 1) || (count->NB_ERROR != 0))
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1b FAILED");
    }
    else if (count->aux < ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND)
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1c FAILED");
    }
    /* pass2, verify commands given during learn or with wrong parameters are rejected */

    etalTestPrintNormal("Learn, pass2");

    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start with NULL array parameter");
    if (etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, NULL) != ETAL_RET_PARAMETER_ERR)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2a FAILED");
    }
    etalTestPrintNormal("* Learn start with invalid band parameter, SKIPPED");
    if (0/*etaltml_learn_start(hReceiver, ETAL_BAND_UNDEF, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, freqList) != ETAL_RET_PARAMETER_ERR*/)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2b FAILED");
    }

    etalTestPrintNormal("* Learn start with illegal number of freq parameter");
    if (etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ + 1, normalMode, freqList) != ETAL_RET_PARAMETER_ERR)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2c FAILED");
    }

    etalTestPrintNormal("* Learn start with illegal receiver");
    if (etaltml_learn_start(ETAL_INVALID_HANDLE, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, freqList) != ETAL_RET_INVALID_HANDLE)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2d FAILED");
    }

    etalTestPrintNormal("* Learn start limited to %d freq, sorted mode", ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("* Learn start while learn is already in progress");
    if (etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND, sortMode, freqList) != ETAL_RET_IN_PROGRESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2e FAILED");
    }

    etalTestPrintNormal("* Tune while performing learn");
    if (etal_tune_receiver(hReceiver, freq) == ETAL_RET_SUCCESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2f FAILED");
    }

    /* We issued a number of learn start commands but we expect only one to
     * succeed; we check that by looking a the event counter.
     * But we need to allow some time to ensure the callback was invoked and updates
     * the counters:
     */
    etalTestPrintNormal("Wait short time (%d sec or less)", ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    /* Expect only one started event, all other commands should have failed */
    if (count->STARTED != 1)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2g FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_HD_TUNE_TIMEOUT);

    etalTestPrintNormal("= Learn found %d good frequencies, %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* last learn was requested to stop after ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND,
     * check if it was respected
     */
    if ((count->FINISHED != 1) ||
        (count->aux != ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND) ||
        (count->NB_ERROR != 0))
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2h FAILED");
    }

    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start limited to %d freq, sorted mode", ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* We issued a number of learn start commands but we expect only one to
     * succeed; we check that by looking a the event counter.
     * But we need to allow some time to ensure the callback was invoked and updates
     * the counters:
     */
    etalTestPrintNormal("Wait short time (%d sec or less)", ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    /* Expect only one started event, all other commands should have failed */
    if (count->STARTED != 1)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2j FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_HD_TUNE_TIMEOUT);

    etalTestPrintNormal("= Learn found %d good frequencies, %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* last learn was requested to stop after ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND,
     * check if it was respected
     */
    if ((count->FINISHED != 1) ||
        (count->aux != ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND) ||
        (count->NB_ERROR != 0))
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2k FAILED");
    }

    /* pass3, stop and restart learn */

    etalTestPrintNormal("Learn, pass3");

    etalTestResetEventCount(count);

    /* start learn for the whole band and stop it before it has time to
     * complete the band scan; expect less than the max number of stations found
     */
    etalTestPrintNormal("* Learn start with abortion");

    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
	
	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    etalTestPrintNormal("Wait short time (%d sec)", ETAL_TEST_HD_LEARN_PARTIAL_BAND_TIME / ETAL_TEST_ONE_SECOND);

    /* wait less then the minimum time required to scan the whole band
     * Don't use sem wait here! */
    OSAL_s32ThreadWait(ETAL_TEST_HD_LEARN_PARTIAL_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_HD_TUNE_TIMEOUT);

    etalTestPrintNormal("= Learn found %d good frequencies, less than %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    if (count->STARTED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3a FAILED");
    }
    /* we stopped the learn prematurely so we don't expect the ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND number of
     * stations to be found
     */
    if ((count->FINISHED != 1) ||
        (count->aux >= ETAL_TEST_HD_LEARN_LONG_FREQ_FOUND) ||
        (count->NB_ERROR != 0))
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3b FAILED");
    }

    /* the UserNotificationHandler posted this semaphore upon reception
     * of the etaltml_learn_stop; since we did not yet use it in the test
     * (it used the OSAL_s32ThreadWait), we need to reset it otherwise the
     * next test will find it already available and exit prematurely
     */
     	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;
	
    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_HD_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    /* Restart the learn, this time allowing the time for the complete scan
     * We want to be sure that the previous etaltml_learn_stop while learn
     * was in progress cleanly stopped the learn state machine
     */
    etalTestResetEventCount(count);

    etalTestPrintNormal("* Learn start limited to %d freq, normal mode", ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND);
    if ((ret = etaltml_learn_start(hReceiver, ETAL_BAND_FM, ETAL_TEST_LEARN_FM_STEP_FREQ, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("Wait short time (%d sec or less)", ETAL_TEST_HD_LEARN_PARTIAL_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_HD_LEARN_PARTIAL_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;

    if (count->STARTED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3c FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_HD_TUNE_TIMEOUT);

    etalTestPrintNormal("= Learn found %d good frequencies, less than %d expected (event S: %d, R: %d, F:%d, E:%d)",
    		count->aux, ETAL_TEST_HD_LEARN_SHORT_FREQ_FOUND,
    		count->STARTED, count->RESULT, count->FINISHED, count->NB_ERROR);

    /* Check reported events */
    if (count->FINISHED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3d FAILED");
    }

#endif // CONFIG_APP_TEST_HDRADIO_FM

    if (!pass1 || !pass2 || !pass3)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_LEARN
    return OSAL_OK;
}


/***************************
 *
 * etalTestLearnAM
 *
 **************************/
tSInt etalTestLearnAM(ETAL_HANDLE handleam)
{
#ifdef CONFIG_APP_TEST_LEARN
    tBool pass1 = TRUE;
    tBool pass2 = TRUE;
    tBool pass3 = TRUE;

#ifdef CONFIG_APP_TEST_AM
    ETAL_STATUS ret;
    EtalLearnFrequencyTy freqList[ETAL_LEARN_MAX_NB_FREQ];
    EtalAudioInterfTy audioIf;

    /* Tune to valid AM freq */
    etalTestPrintNormal("* Tune to AM freq %d, handle %d", ETAL_VALID_AM_FREQ, handleam);
    if ((ret = etal_tune_receiver(handleam, ETAL_VALID_AM_FREQ)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_tune_receiver AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, handleam);
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

    /* Select audio source */
    ret = etal_audio_select(handleam, ETAL_AUDIO_SOURCE_STAR_AMFM);
    if ((ret != ETAL_RET_SUCCESS) && (ret != ETAL_RET_INVALID_RECEIVER))
    {
        etalTestPrintError("etal_audio_select AM (%s, %d), handle %d", ETAL_STATUS_toString(ret), ret, handleam);
        return OSAL_ERROR;
    }

    OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

    memset(freqList, 0, ETAL_LEARN_MAX_NB_FREQ * sizeof(EtalLearnFrequencyTy));

    /* pass1, plain learn command */
	
    etalTestPrintNormal("Learn, pass1");

    etalTestResetEventCount(&etalTestLearnCountAM);

    etalTestPrintNormal("* Learn start");
    if ((ret = etaltml_learn_start(handleam, ETAL_BAND_AM, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_LEARN_MAX_NB_FREQ, sortMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Long delay to scan all the AM band */
    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;
	

    /* Check reported events */
    if (etalTestLearnCountAM.STARTED != 1)
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1a FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(handleam, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, min expected ? (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountAM.aux,
    		etalTestLearnCountAM.STARTED, etalTestLearnCountAM.RESULT,
    		etalTestLearnCountAM.FINISHED, etalTestLearnCountAM.NB_ERROR);

    /* Check reported events */
    if (etalTestLearnCountAM.FINISHED != 1)
    {
        pass1 = FALSE;
        etalTestPrintNormal("pass1b FAILED");
    }

    /* pass2, verify commands given during learn or with wrong parameters are rejected */

    etalTestPrintNormal("Learn, pass2");

    etalTestResetEventCount(&etalTestLearnCountAM);

    etalTestPrintNormal("* Learn start with wrong parameter");
    if (etaltml_learn_start(handleam, ETAL_BAND_UNDEF, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND, normalMode, NULL) != ETAL_RET_PARAMETER_ERR)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2a FAILED");
    }

    etalTestPrintNormal("* Learn start with illegal receiver");
    if (etaltml_learn_start(ETAL_INVALID_HANDLE, ETAL_BAND_AM, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND, normalMode, freqList) != ETAL_RET_INVALID_HANDLE)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2b FAILED");
    }

    etalTestPrintNormal("* Learn start");
    if ((ret = etaltml_learn_start(handleam, ETAL_BAND_AM, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_TEST_AM_LEARN_LONG_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("* Learn start while learn is already in progress");
    if (etaltml_learn_start(handleam, ETAL_BAND_AM, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND, normalMode, freqList) != ETAL_RET_IN_PROGRESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2c FAILED");
    }

    etalTestPrintNormal("* Tune receiver while performing learn");
    if (etal_tune_receiver(handleam, ETAL_VALID_AM_FREQ) == ETAL_RET_SUCCESS)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2d FAILED");
    }

    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;
	
    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME);
	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;
	

    /* Check reported events */
    if (etalTestLearnCountAM.STARTED != 1)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2e FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(handleam, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, min expected ? (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountAM.aux,
    		etalTestLearnCountAM.STARTED, etalTestLearnCountAM.RESULT,
    		etalTestLearnCountAM.FINISHED, etalTestLearnCountAM.NB_ERROR);
    /* Check reported events */
    if (etalTestLearnCountAM.FINISHED != 1)
    {
        pass2 = FALSE;
        etalTestPrintNormal("pass2f FAILED");
    }

    /* pass3, stop and restart learn */

    etalTestPrintNormal("Learn, pass3");

    etalTestResetEventCount(&etalTestLearnCountAM);

    etalTestPrintNormal("* Learn start with abortion");
    if ((ret = etaltml_learn_start(handleam, ETAL_BAND_AM, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_TEST_AM_LEARN_LONG_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_AM_LEARN_PARTIAL_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_AM_LEARN_PARTIAL_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;
		
    /* Check reported events */
    if (etalTestLearnCountAM.STARTED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3a FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(handleam, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    /* Check reported events */
    if (etalTestLearnCountAM.FINISHED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3b FAILED");
    }

    /* Restart the learn, this time allowing the time for the complete scan */
    etalTestPrintNormal("* Learn start with only the first frequencies");
    etalTestPrintNormal("* Learn start limited to %d freq, normal mode", ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND);

    etalTestResetEventCount(&etalTestLearnCountAM);

    if ((ret = etaltml_learn_start(handleam, ETAL_BAND_AM, ETAL_TEST_LEARN_AM_STEP_FREQ, ETAL_TEST_AM_LEARN_SHORT_FREQ_FOUND, normalMode, freqList)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintNormal("Wait max time (%d sec)", ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME / ETAL_TEST_ONE_SECOND);

	// init condition in case finished has been posted later than expected.
	vl_etalTestLearn_LearnOnGoing = true;

    OSAL_s32SemaphoreWait(etalTestLearnSem, ETAL_TEST_AM_LEARN_WHOLE_BAND_TIME);

	// mark the end of the learn
	// to avoid further event posted in cross case and on Learn interruption
	vl_etalTestLearn_LearnOnGoing = false;
	

    /* Check reported events */
    if (etalTestLearnCountAM.STARTED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3c FAILED");
    }

    etalTestPrintNormal("* Learn stop");
    if ((ret = etaltml_learn_stop(handleam, lastFrequency)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    /* Wait some time to ensure the callback is invoked */
    OSAL_s32ThreadWait(ETAL_TEST_LEARN_CALLBACK_WAIT);

    etalTestPrintNormal("= Learn found %d good frequencies, min expected ? (event S: %d, R: %d, F:%d, E:%d)",
    		etalTestLearnCountAM.aux,
    		etalTestLearnCountAM.STARTED, etalTestLearnCountAM.RESULT,
    		etalTestLearnCountAM.FINISHED, etalTestLearnCountAM.NB_ERROR);

    /* Check reported events */
    if (etalTestLearnCountAM.FINISHED != 1)
    {
        pass3 = FALSE;
        etalTestPrintNormal("pass3d FAILED");
    }

#endif // CONFIG_APP_TEST_DAB

    if (!pass1 || !pass2 || !pass3)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_LEARN
    return OSAL_OK;
}


/***************************
 *
 * etalTestLearn
 *
 **************************/
tSInt etalTestLearn(void)
{
#ifdef CONFIG_APP_TEST_LEARN
    tBool pass1 = TRUE;

    etalTestStartup();

    if (etalTestLearnSem == 0)
    {
        if (OSAL_s32SemaphoreCreate("Sem_etalTestLearn", &etalTestLearnSem, 0) == OSAL_ERROR)
        {
            return OSAL_ERROR;
        }
    }

    etalTestPrintNormal("<---Learn Test (sequential) start");

   if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
   {
       return OSAL_ERROR;
   }

    if (handledab != ETAL_INVALID_HANDLE)
    {
        if (etalTestLearnDAB(handledab) != OSAL_OK)
        {
            etalTestPrintNormal("****************************");
            etalTestPrintNormal("TestLearnDAB FAILED, handle %d", handledab);
            etalTestPrintNormal("****************************");
            pass1 = FALSE;
        }
        etalTestPrintNormal("");

        if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }

    if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    if (handlehd != ETAL_INVALID_HANDLE)
    {
        if (etalTestLearnHD(handlehd, ETAL_TEST_MODE_HD_FM) != OSAL_OK)
        {
        	etalTestPrintNormal("****************************");
        	etalTestPrintNormal("TestLearnHD FAILED, handle %d", handlehd);
        	etalTestPrintNormal("****************************");
        	pass1 = FALSE;
        }
        etalTestPrintNormal("");

        if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }

    if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    if (handlefm != ETAL_INVALID_HANDLE)
    {
        if (etalTestLearnFM(handlefm, ETAL_TEST_MODE_FM) != OSAL_OK)
        {
            etalTestPrintNormal("****************************");
            etalTestPrintNormal("TestLearnFM FAILED, handle %d", handlefm);
            etalTestPrintNormal("****************************");
            pass1 = FALSE;
        }
        etalTestPrintNormal("");

        if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }



#ifdef CONFIG_APP_TEST_AM
    if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    if (etalTestLearnAM(handleam) != OSAL_OK)
    {
        etalTestPrintNormal("****************************");
        etalTestPrintNormal("TestLearnAM FAILED, handle %d", handleam);
        etalTestPrintNormal("****************************");
        pass1 = FALSE;
    }

    if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_AM

    if (!pass1)
    {
        return OSAL_ERROR;
    }

#endif // CONFIG_APP_TEST_LEARN

    return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

