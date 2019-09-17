//!
//!  \file 		etaltest_debug.c
//!  \brief 	<i><b> ETAL test, debug set DISS, Get WSP Status </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	David Pastor
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

/*
 * Function Prototypes
 */
#ifdef CONFIG_APP_TEST_DEBUG_DISS_WSP
static tSInt etalTestDebugDISSControl(tBool *pass);
#endif /* CONFIG_APP_TEST_DEBUG_DISS_WSP */
#ifdef CONFIG_APP_TEST_DEBUG_VPA_CONTROL
static tSInt etalTestDebugVPAControl(tBool *pass);
#endif /* CONFIG_APP_TEST_DEBUG_VPA_CONTROL */

#if defined (CONFIG_APP_TEST_DEBUG_DISS_WSP) || defined (CONFIG_APP_TEST_DEBUG_VPA_CONTROL)
typedef enum {
	DBG_DISS_SET_DISS_GET_WSP = 1,
	DBG_DISS_SET_DISS_PARAM_CHECK,
	DBG_VPA_CTRL
} DebugTestTy;
#endif

/*
 * Functions
 */
 
#ifdef CONFIG_APP_TEST_DEBUG_DISS_WSP

typedef enum
{
	checkDISS_0    = 0,
	checkDISS_1    = 1,
	checkDISS_both = 2,
	checkDISS_none = 0xFF
} etalTestDISSCheckMode;

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
/***************************
 *
 * etalTestDebugDISSControl_applyAllFilters
 *
 **************************/
static tSInt etalTestDebugDISSControl_applyAllFilters(etalTestBroadcastTy test_mode, ETAL_HANDLE hReceiver, etalChannelTy chan, etalTestDISSCheckMode check_mode, tBool *pass)
{
    ETAL_STATUS ret;
	tU8 filter_idx;
    EtalWSPStatus WSPStatus;
	tU32 idx;
	tChar *chan_str;

	switch (chan)
	{
		case ETAL_CHN_FOREGROUND:
			chan_str = "fore channel";
			break;
		case ETAL_CHN_BACKGROUND:
			chan_str = "back channel";
			break;
		case ETAL_CHN_BOTH:
			chan_str = "both channels";
			break;
		default:
			return OSAL_ERROR;
	}

    etalTestPrintReportPassStart(DBG_DISS_SET_DISS_GET_WSP, test_mode, "Set DISS and Get WSP Status %s", chan_str);

    for(filter_idx = 0; filter_idx <= ETAL_DISS_FILTER_INDEX_MAX; filter_idx++)
    {
		if ((ret = etal_debug_DISS_control(hReceiver, chan, ETAL_DISS_MODE_MANUAL, filter_idx)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintNormal("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, filter_idx, ETAL_STATUS_toString(ret), ret);
			*pass = FALSE;
		}
		else if ((ret = etal_debug_get_WSP_Status(hReceiver, &WSPStatus)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintNormal("etal_debug_get_WSP_Status (%s, %d)", ETAL_STATUS_toString(ret), ret);
			*pass = FALSE;
		}
		else {
			etalTestPrintNormal("WSP status: filter_index0:%d filter_index1:%d softmute:%d highcut:%d lowcut:%d stereoblend:%d highblend:%d rolloff:0x%x",
				WSPStatus.m_filter_index[0], WSPStatus.m_filter_index[1], WSPStatus.m_softmute, WSPStatus.m_highcut,
				WSPStatus.m_lowcut, WSPStatus.m_stereoblend, WSPStatus.m_highblend,
				WSPStatus.m_rolloff);

			switch (check_mode)
			{
				case checkDISS_0:
				case checkDISS_1:
					idx = (tU32) check_mode;
					if (WSPStatus.m_filter_index[idx] != filter_idx)
					{
						etalTestPrintNormal("filter%d index not correctly set: expected %d, read %d", idx + 1, filter_idx, WSPStatus.m_filter_index[idx]);
						*pass = FALSE;
					}
					break;

				case checkDISS_both:
					if (WSPStatus.m_filter_index[0] != filter_idx)
					{
						etalTestPrintNormal("filter index not correctly set: expected %d, read %d", filter_idx, WSPStatus.m_filter_index[0]);
						*pass = FALSE;
					}
					if (WSPStatus.m_filter_index[1] != filter_idx)
					{
						etalTestPrintNormal("filter2 index not correctly set: expected %d, read %d", filter_idx, WSPStatus.m_filter_index[1]);
						*pass = FALSE;
					}
					break;

				case checkDISS_none:
					break;
			}
		}
	}

	if (*pass)
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	else
	{
		etalTestPrintReportPassEnd(testFailed);
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestDebugDISSControl_std
 *
 **************************/
static tSInt etalTestDebugDISSControl_std(etalTestBroadcastTy test_mode, tBool *pass)
{
	ETAL_STATUS ret;
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	ETAL_HANDLE *phReceiver;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_FM:
			config_mode = ETAL_CONFIG_FM1;
			tune_mode = ETAL_TUNE_FM;
			phReceiver = &handlefm;
			break;

		case ETAL_TEST_MODE_HD_FM:
			config_mode = ETAL_CONFIG_HDRADIO_FM;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			phReceiver = &handlehd;
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
#if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB) // otherwise FE1 is dedicated to DAB and not available for BG
#ifdef CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
	etalTestPrintNormal("* Config FM2 receiver");
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM2, &handlefm2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (handlefm2 != ETAL_INVALID_HANDLE)
	{
		etalTestPrintNormal("* Tune to FM2 freq %d", ETAL_VALID_FM_FREQ2);
		if ((ret = etal_tune_receiver(handlefm2, ETAL_VALID_FM_FREQ2)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
#else //CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
	handlefm2 = ETAL_INVALID_HANDLE;
#endif //CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL

#endif //defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB)

	if (etalTestDebugDISSControl_applyAllFilters(test_mode, *phReceiver, ETAL_CHN_FOREGROUND, checkDISS_0, pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (handlefm2 != ETAL_INVALID_HANDLE)
	{
		if (etalTestDebugDISSControl_applyAllFilters(test_mode, handlefm2, ETAL_CHN_BACKGROUND, checkDISS_1, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		if (etalTestDebugDISSControl_applyAllFilters(test_mode, *phReceiver, ETAL_CHN_BOTH, checkDISS_both, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		/* ETAL_DISS_MODE_AUTO applicable only to Foreground channel, expect error */
       	etalTestPrintReportPassStart(DBG_DISS_SET_DISS_PARAM_CHECK, ETAL_TEST_MODE_FM, "Set DISS parameter check");
		if ((ret = etal_debug_DISS_control(handlefm2, ETAL_CHN_BACKGROUND, ETAL_DISS_MODE_AUTO, 0)) != ETAL_RET_PARAMETER_ERR)
		{
			etalTestPrintNormal("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_BACKGROUND, 0, ETAL_STATUS_toString(ret), ret);
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass3 FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

    /*
     * cleanup
     */

    if (etalTestUndoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (handlefm2 != ETAL_INVALID_HANDLE)
    {
        if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm2) != OSAL_OK) // not a typo, handlefm2 is always FM
        {
            return OSAL_ERROR;
        }
        if (etalTestUndoConfigSingle(&handlefm2) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }
    return OSAL_OK;
}
#endif //defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)

/***************************
 *
 * etalTestDebugDISSControl
 *
 **************************/
static tSInt etalTestDebugDISSControl(tBool *pass)
{
    etalTestPrintNormal("<---Set DISS and Get WSP Status Test start");

    /* invoke set DISS with correct parameters and read filter index with Get WDP Status */

#if defined (CONFIG_APP_TEST_FM)
	if (etalTestDebugDISSControl_std(ETAL_TEST_MODE_FM, pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined (CONFIG_APP_TEST_FM)

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	if (etalTestDebugDISSControl_std(ETAL_TEST_MODE_HD_FM, pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM)

	return OSAL_OK;
}

#endif // CONFIG_APP_TEST_DEBUG_DISS_WSP

#ifdef CONFIG_APP_TEST_DEBUG_VPA_CONTROL
/***************************
 *
 * etalTestDebugVPAControl
 *
 **************************/
static tSInt etalTestDebugVPAControl(tBool *pass)
{
#if defined(CONFIG_APP_TEST_AM) || (defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB)) // otherwise FE1 is dedicated to DAB and not available for BG
    ETAL_STATUS ret;
    EtalProcessingFeatures proc_features;
#endif //defined(CONFIG_APP_TEST_AM) || (defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB))

#if (defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB))
    EtalAudioInterfTy audioIf;
#endif

#ifdef CONFIG_APP_TEST_AM
    ETAL_HANDLE handleam2 = ETAL_INVALID_HANDLE;
#endif /* CONFIG_APP_TEST_AM */

    etalTestPrintNormal("<---debug VPA control Test start");

#if (defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB))

    /* invoke debug VPA control */
    /*
     * create an FM receiver foreground and background with VPA ON
     */
    etalTestPrintNormal("* Configure FM VPA");
    if (etalTestDoConfigSingle(ETAL_CONFIG_FM1_FM2_VPA, &handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    /*
     * Configure audio path
     */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
    // Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
    audioIf.m_sai_slave_mode = true;
#endif
    if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * Configure audio source
     */
    if ((ret = etal_audio_select(handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * Tune FM receiver foreground
     */
    etalTestPrintNormal("* Tune to FM freq %d handle %d", ETAL_VALID_FM_FREQ_VPA, handlefm);
    if ((ret = etal_tune_receiver(handlefm, ETAL_VALID_FM_FREQ_VPA)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    etalTestPrintReportPassStart(DBG_VPA_CTRL, ETAL_TEST_MODE_FM, "Debug VPA control");
	/*
	 * wait for VPA ON user check
	 */
    etalTestPrintNormal("Wait 20: antenna tuner A good audio, antenna tuner B good audio");
    OSAL_s32ThreadWait(20 * ETAL_TEST_ONE_SECOND);
    /*
     * set FM in Debug VPA OFF
     */
    etalTestPrintNormal("* Set FM Debug VPA off");
    if ((ret = etal_debug_VPA_control(handlefm, FALSE, &handlefm2)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_VPA_control FM VPA off (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
	/*
	 * wait for VPA OFF user check
	 */
    etalTestPrintNormal("Wait 20: antenna tuner A good audio, antenna tuner B bad audio");
    OSAL_s32ThreadWait(20 * ETAL_TEST_ONE_SECOND);
    /*
     * destroy background receiver
     */
    if ((ret = etal_destroy_receiver(&handlefm2)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_destroy_receiver FM bg (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set FM in Debug VPA ON
     */
    etalTestPrintNormal("* Set FM Debug VPA on");
    if ((ret = etal_debug_VPA_control(handlefm, TRUE, NULL)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_VPA_control FM VPA on (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
	/*
	 * wait for VPA ON user check
	 */
    etalTestPrintNormal("Wait 20: antenna tuner A good audio, antenna tuner B good audio");
    OSAL_s32ThreadWait(20 * ETAL_TEST_ONE_SECOND);
    /*
     * set foreground manual DISS
     */
    etalTestPrintNormal("* Set FM foreground DISS manual");
    if ((ret = etal_debug_DISS_control(handlefm, ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, 5)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, 5, ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set foreground auto DISS
     */
    etalTestPrintNormal("* Set FM foreground DISS auto");
    if ((ret = etal_debug_DISS_control(handlefm, ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_AUTO, 0)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_AUTO, 0, ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set FM in Debug VPA OFF
     */
    etalTestPrintNormal("* Set FM Debug VPA off");
    if ((ret = etal_debug_VPA_control(handlefm, FALSE, &handlefm2)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_VPA_control FM VPA off (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
	/*
	 * wait for VPA OFF user check
	 */
    etalTestPrintNormal("Wait 20: antenna tuner A good audio, antenna tuner B bad audio");
    OSAL_s32ThreadWait(20 * ETAL_TEST_ONE_SECOND);
    /*
     * set foreground manual DISS
     */
    etalTestPrintNormal("* Set FM foreground DISS manual");
    if ((ret = etal_debug_DISS_control(handlefm, ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, 5)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, 5, ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set foreground auto DISS
     */
    etalTestPrintNormal("* Set FM foreground DISS auto");
    if ((ret = etal_debug_DISS_control(handlefm, ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_AUTO, 0)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_AUTO, 0, ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set foreground manual DISS
     */
    etalTestPrintNormal("* Set FM foreground DISS manual");
    if ((ret = etal_debug_DISS_control(handlefm, ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, 3)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_DISS_control channel:%d mode:%d filter index:%d (%s, %d)", ETAL_CHN_FOREGROUND, ETAL_DISS_MODE_MANUAL, 5, ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * destroy background receiver
     */
    if ((ret = etal_destroy_receiver(&handlefm2)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_destroy_receiver FM bg (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set FM in Debug VPA ON
     */
    etalTestPrintNormal("* Set FM Debug VPA on");
    if ((ret = etal_debug_VPA_control(handlefm, TRUE, NULL)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_VPA_control FM VPA on (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set FM in Debug VPA OFF
     */
    etalTestPrintNormal("* Set FM Debug VPA off");
    if ((ret = etal_debug_VPA_control(handlefm, FALSE, &handlefm2)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_VPA_control FM VPA off (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * set FM without VPA
     */
    etalTestPrintNormal("* Set FM without VPA");
    proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
    if ((ret = etal_change_band_receiver(handlefm, ETAL_BAND_FM, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_change_band_receiver FM without VPA (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * destroy background receiver
     */
    if ((ret = etal_destroy_receiver(&handlefm2)) != ETAL_RET_SUCCESS)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_destroy_receiver FM bg (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    /*
     * try to set FM in Debug VPA ON
     */
    etalTestPrintNormal("* Try FM Debug VPA on, should return error");
    if ((ret = etal_debug_VPA_control(handlefm, TRUE, NULL)) != ETAL_RET_ERROR)
    {
		etalTestPrintReportPassEnd(testFailed);
        etalTestPrintError("etal_debug_VPA_control should return error in FM VPA on (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

	etalTestPrintReportPassEnd(testPassed);

    /*
     * Undo tune FM background
     */
    if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoConfigSingle(&handlefm2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    /*
     * Undo tune FM foreground
     */
    if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
#endif //defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB)

#ifdef CONFIG_APP_TEST_AM
        /*
         * creat AM receiver foreground
         */
        if ((ret = etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam)) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
        if (etalTestDoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
        /*
         * set AM in VPA
         */
        etalTestPrintNormal("* Set AM in VPA");
        proc_features.u.m_processing_features = ETAL_PROCESSING_FEATURE_FM_VPA;
        if ((ret = etal_change_band_receiver(handleam, ETAL_BAND_AM, 0, 0, proc_features)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_change_band_receiver FM without VPA (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }

		etalTestPrintReportPassStart(DBG_VPA_CTRL, ETAL_TEST_MODE_AM, "Debug VPA control");

        /*
         * try to set AM in Debug VPA ON
         */
        etalTestPrintNormal("* Try AM Debug VPA on, should return error");
        if ((ret = etal_debug_VPA_control(handleam, TRUE, NULL)) == ETAL_RET_SUCCESS)
        {
			etalTestPrintReportPassEnd(testFailed);
            etalTestPrintError("etal_debug_VPA_control should return error in AM VPA on (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
        /*
         * try to set AM in Debug VPA OFF
         */
        etalTestPrintNormal("* Try AM Debug VPA off, should return error");
        if ((ret = etal_debug_VPA_control(handleam, FALSE, &handleam2)) == ETAL_RET_SUCCESS)
        {
			etalTestPrintReportPassEnd(testFailed);
            etalTestPrintError("etal_debug_VPA_control should return error in AM VPA off (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }

		etalTestPrintReportPassEnd(testPassed);

        /*
         * Undo tune FM background
         */
        if (etalTestUndoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
        if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
#endif /* CONFIG_APP_TEST_AM */
    return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DEBUG_VPA_CONTROL

/***************************
 *
 * etalTestDebug
 *
 **************************/
tSInt etalTestDebug(void)
{
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;

	etalTestStartup();

#ifdef CONFIG_APP_TEST_DEBUG_DISS_WSP
    if (etalTestDebugDISSControl(&pass1) != OSAL_OK)
    {
		return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_DEBUG_DISS_WSP

#ifdef CONFIG_APP_TEST_DEBUG_VPA_CONTROL
    if (etalTestDebugVPAControl(&pass2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
#endif // CONFIG_APP_TEST_DEBUG_VPA_CONTROL

	if (!pass1 || !pass2)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST
