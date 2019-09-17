//!
//!  \file 		etaltest_seamless.c
//!  \brief 	<i><b> ETAL test, seamless estimation and seamless switching </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"

#include "etaltest.h"
#include "dabmw_import.h" // for DABMW_MAIN_AUDIO_DAB_APP

#if defined(CONFIG_APP_TEST_SEAMLESS) && (defined(CONFIG_ETAL_HAVE_ALL_API) || defined(CONFIG_ETAL_HAVE_SEAMLESS))

/***************************
 * Local types
 **************************/
typedef enum {
    ETALTEST_SE_SS_IDLE,
    ETALTEST_SE_SEND,
    ETALTEST_SE_RECEIVED,
    ETALTEST_SS_SEND,
    ETALTEST_SS_RECEIVED
}etaltest_ss_state_ty;

typedef enum
{
    ETALTEST_SE_STATUS_NONE                         = 0x00,
    ETALTEST_SE_STATUS_SUCCESS                      = 0x01,
    ETALTEST_SE_STATUS_FAILURE                      = 0x02,
    ETALTEST_SE_STATUS_STOPPED                      = 0x04,
    ETALTEST_SE_STATUS_ERROR_ON_DELAY_ESTIMATION    = 0x08,
    ETALTEST_SE_STATUS_ERROR_ON_LOUDNESS_ESTIMATION = 0x10,
    ETALTEST_SE_STATUS_ERROR_ON_BUFFERING           = 0x20,
    ETALTEST_SE_STATUS_ERROR_ON_SEAMLESS_ESTIMATION = 0x40
} etaltest_se_statusTy;

typedef enum
{
    ETALTEST_SS_STATUS_SUCCESSFUL                       = 0x00,
    ETALTEST_SS_STATUS_SUCCESSFUL_WITH_DEFAULT_VALUE    = 0x01,
    ETALTEST_SS_STATUS_SUCCESSFUL_APPROXIMATED          = 0x02,
    ETALTEST_SS_STATUS_FAILURE                          = 0x03
} etaltest_ss_statusTy;

/***************************
 * Local variables
 **************************/
tU32 etalTestSeamlessEstimationCount;
tU32 etalTestSeamlessSwitchingCount;
EtalSeamlessEstimationStatus etaltest_seamless_estimation_status;
EtalSeamlessSwitchingStatus etaltest_seamless_switching_status;
etaltest_ss_state_ty etaltest_se_ss_state = ETALTEST_SE_SS_IDLE;


/***************************
 * function prototype
 **************************/
static tSInt etalTestSeamlessDabFmDoCfgTune(ETAL_HANDLE *phandledab, ETAL_HANDLE *phandlefm, ETAL_HANDLE *phDatapath);
static tSInt etalTestSeamlessFmEarlyDabDoCfgTune(ETAL_HANDLE *phandledab, ETAL_HANDLE *phandlefm, ETAL_HANDLE *phDatapath);
static tSInt etalTestSeamlessDabFmUndoCfgTune(ETAL_HANDLE *phandledab, ETAL_HANDLE *phandlefm, ETAL_HANDLE *phDatapath);

/***************************
 *
 * etalTestSeamlessDabFmDoCfgTune
 * Tune DAB with audio switch, tune FM without audio switch
 *
 **************************/
static tSInt etalTestSeamlessDabFmDoCfgTune(ETAL_HANDLE *phandledab, ETAL_HANDLE *phandlefm, ETAL_HANDLE *phDatapath)
{
    ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;

    /* Configure DAB FM etal handles */

    if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, phandledab) != OSAL_OK)
    {
      return OSAL_ERROR;
    }
    if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, phandlefm) != OSAL_OK)
    {
      return OSAL_ERROR;
    }

    if ((*phandledab == ETAL_INVALID_HANDLE) || (*phandlefm == ETAL_INVALID_HANDLE))
    {
        return OSAL_ERROR;
    }
#if ((!defined(CONFIG_APP_TEST_DAB)) || (!defined(CONFIG_APP_TEST_FM)))
    return OSAL_ERROR;
#endif

    /* Tune to valid DAB freq with audio switch */

    etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ);
    if ((ret = etal_tune_receiver(*phandledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
    audioIf.m_dac = 0;
#endif

    /* depending on the test sequence FIC data may not be already available */
    OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

    if (etalTestDoServiceSelectAudio(*phandledab, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID))
    {
        return OSAL_ERROR;
    }
    /* Select audio source */
    if ((ret = etal_audio_select(*phandledab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    /* Tune to valid FM freq without audio switch */

    etalTestPrintNormal("* Tune to FM freq %d", 107700);
    if ((ret = etal_tune_receiver(*phandlefm, 107700)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    return OSAL_OK;
}


/***************************
 *
 * etalTestSeamlessFmEarlyDabDoCfgTune
 * Tune FM early with audio switch, tune DAB without audio switch
 *
 **************************/
static tSInt etalTestSeamlessFmEarlyDabDoCfgTune(ETAL_HANDLE *phandledab, ETAL_HANDLE *phandlefm, ETAL_HANDLE *phDatapath)
{
    ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;

    /* Configure DAB FM etal handles */

    if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, phandledab) != OSAL_OK)
    {
      return OSAL_ERROR;
    }
    if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, phandlefm) != OSAL_OK)
    {
      return OSAL_ERROR;
    }
    if ((*phandledab == ETAL_INVALID_HANDLE) || (*phandlefm == ETAL_INVALID_HANDLE))
    {
        return OSAL_ERROR;
    }
#if ((!defined(CONFIG_APP_TEST_DAB)) || (!defined(CONFIG_APP_TEST_FM)))
    return OSAL_ERROR;
#endif

    /* Tune to valid FM freq with audio switch */

    /* Configure audio path */
    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
    audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
    audioIf.m_dac = 0;
#endif

    etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
    if ((ret = etal_tune_receiver(*phandlefm, 107700)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }
    /* Select audio source */
    if ((ret = etal_audio_select(*phandlefm, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    /* Tune to valid DAB freq without audio switch */

    etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ);
    if ((ret = etal_tune_receiver(*phandledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    /* depending on the test sequence FIC data may not be already available */
    OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	/* Service Select by SID */
	etalTestPrintNormal("* Service Select by Sid");
	if ((ret = etal_service_select_audio(*phandledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI_UEID, ETAL_DAB_ETI_SERV3_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

    return OSAL_OK;
}


/***************************
 *
 * etalTestSeamlessDabFmUndoCfgTune
 * Undo Tune DAB FM, datapath and config
 *
 **************************/
static tSInt etalTestSeamlessDabFmUndoCfgTune(ETAL_HANDLE *phandledab, ETAL_HANDLE *phandlefm, ETAL_HANDLE *phDatapath)
{
	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, *phandledab) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
	if (etalTestUndoTuneSingle(ETAL_TUNE_FM, *phandlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    /* Destroy the receivers */
    if (etalTestUndoConfigSingle(phandledab) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoConfigSingle(phandlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

    return OSAL_OK;
}

/***************************
 *
 * etalTestSeamlessEstimation_SetConfig set full estimation configuration
 *
 **************************/
static tVoid etalTestSeamlessEstimation_SetConfig(etalSeamlessEstimationConfigTy *se_cfg_ptr) {
	se_cfg_ptr->mode = 1;
	se_cfg_ptr->startPosition = 0;
	se_cfg_ptr->stopPosition = -480000;
}

/***************************
 *
 * etalTestSeamlessEstimation_CmdWaitResp sends seamless estimation command and wait response
 *
 **************************/
static tSInt etalTestSeamlessEstimation_CmdWaitResp(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessEstimationConfigTy *seamless_estimation_config_ptr, OSAL_tMSecond response_timeout, tU8 status_mask)
{
    ETAL_STATUS ret;

    if ((ret = etal_seamless_estimation_start(hReceiverFAS, hReceiverSAS, seamless_estimation_config_ptr)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    /* wait response_timeout ms for the command response */
    etaltest_se_ss_state = ETALTEST_SE_SEND;
    while ((response_timeout > 0) && (etaltest_se_ss_state != ETALTEST_SE_RECEIVED)) {
        if (response_timeout >= 1000) {
            /* wait 1 s */
            OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
            response_timeout -= 1000;
        }
        else {
            OSAL_s32ThreadWait(response_timeout);
            response_timeout = 0;
        }
    }

    /* check command response timeout */
    if (etaltest_se_ss_state != ETALTEST_SE_RECEIVED)
    {
        etalTestPrintError("etal_seamless_estimation response timeout");
        return OSAL_ERROR_TIMEOUT_EXPIRED;
    }

    /* check response status */
    if ((etaltest_seamless_estimation_status.m_status & ~status_mask) != 0) {
        etalTestPrintError("etal_seamless_estimation unexpected response status=%d, status mask=0x%x",
                        etaltest_seamless_estimation_status.m_status, status_mask);
        return OSAL_ERROR;
    }

    return OSAL_OK;
}

/***************************
 *
 * etalTestSeamlessEstimation_Resp store seamless estimation response
 *
 **************************/
tVoid etalTestSeamlessEstimation_Resp(EtalSeamlessEstimationStatus *seamless_estimation_status)
{
    /* copy seamless estimation result */
    etaltest_seamless_estimation_status = *seamless_estimation_status;

    /* set etaltest seamless state */
    etaltest_se_ss_state = ETALTEST_SE_RECEIVED;
}
 
#ifdef CONFIG_APP_TEST_SEAMLESS
/***************************
 *
 * etalTestSeamlessSwitching_SetDefaultConfig set default configuration for seamless switch
 *
 **************************/
static tVoid etalTestSeamlessSwitching_SetDefaultConfig(etalSeamlessSwitchingConfigTy *ss_cfg_ptr, tU8 systemToSwitch) {
	ss_cfg_ptr->systemToSwitch = systemToSwitch;
	ss_cfg_ptr->providerType = 0;
	ss_cfg_ptr->absoluteDelayEstimate = 8888888;
	ss_cfg_ptr->delayEstimate = 0;
	ss_cfg_ptr->timestampFAS = 0;
	ss_cfg_ptr->timestampSAS = 0;
	ss_cfg_ptr->averageRMS2FAS = 1;
	ss_cfg_ptr->averageRMS2SAS = 1;
}

/***************************
 *
 * etalTestSeamlessSwitching_SetConfig set configuration for seamless switch from previous estimation result
 *
 **************************/
static tVoid etalTestSeamlessSwitching_SetConfig(etalSeamlessSwitchingConfigTy *ss_cfg_ptr, tU8 systemToSwitch) {
	ss_cfg_ptr->systemToSwitch = systemToSwitch;
	ss_cfg_ptr->providerType = etaltest_seamless_estimation_status.m_providerType;
	ss_cfg_ptr->absoluteDelayEstimate = etaltest_seamless_estimation_status.m_absoluteDelayEstimate;
	ss_cfg_ptr->delayEstimate = etaltest_seamless_estimation_status.m_delayEstimate;
	ss_cfg_ptr->timestampFAS = etaltest_seamless_estimation_status.m_timestamp_FAS;
	ss_cfg_ptr->timestampSAS = etaltest_seamless_estimation_status.m_timestamp_SAS;
	ss_cfg_ptr->averageRMS2FAS = etaltest_seamless_estimation_status.m_RMS2_FAS;
	ss_cfg_ptr->averageRMS2SAS = etaltest_seamless_estimation_status.m_RMS2_SAS;
}

/***************************
 *
 * etalTestSeamlessSwitching_CmdWaitResp sends seamless switching command and wait response
 *
 **************************/
static tSInt etalTestSeamlessSwitching_CmdWaitResp(ETAL_HANDLE hReceiverFAS, ETAL_HANDLE hReceiverSAS, etalSeamlessSwitchingConfigTy *seamless_switching_config_ptr, OSAL_tMSecond response_timeout, tU8 status_mask)
{
    ETAL_STATUS ret;

	if ((ret = etal_seamless_switching(hReceiverFAS, hReceiverSAS, seamless_switching_config_ptr)) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

    /* wait response_timeout ms for the command response */
    etaltest_se_ss_state = ETALTEST_SS_SEND;
    while ((response_timeout > 0) && (etaltest_se_ss_state != ETALTEST_SS_RECEIVED)) {
        if (response_timeout >= 1000) {
            /* wait 1 s */
            OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
            response_timeout -= 1000;
        }
        else {
            OSAL_s32ThreadWait(response_timeout);
            response_timeout = 0;
        }
    }

    /* check command response timeout */
    if (etaltest_se_ss_state != ETALTEST_SS_RECEIVED)
    {
        etalTestPrintError("etal_seamless_switching response timeout");
        return OSAL_ERROR_TIMEOUT_EXPIRED;
    }

    /* check response status */
    if ((etaltest_seamless_switching_status.m_status & ~status_mask) != 0) {
        etalTestPrintError("etal_seamless_switching unexpected response status=%d, status mask=0x%x",
                        etaltest_seamless_switching_status.m_status, status_mask);
        return OSAL_ERROR;
    }

    return OSAL_OK;
}
#endif // CONFIG_APP_TEST_SEAMLESS

/***************************
 *
 * etalTestSeamlessSwitching_Resp store seamless estimation response
 *
 **************************/
tVoid etalTestSeamlessSwitching_Resp(EtalSeamlessSwitchingStatus *seamless_switching_status)
{
    /* copy seamless switching result */
    etaltest_seamless_switching_status = *seamless_switching_status;

    /* set etaltest seamless state */
    etaltest_se_ss_state = ETALTEST_SS_RECEIVED;
}
#endif //#if defined(CONFIG_APP_TEST_SEAMLESS) && (defined(CONFIG_ETAL_HAVE_ALL_API) || defined(CONFIG_ETAL_HAVE_SEAMLESS))


/***************************
 *
 * etalTestSeamless
 *
 **************************/
tSInt etalTestSeamless(void)
{
#if defined(CONFIG_APP_TEST_SEAMLESS) && (defined(CONFIG_ETAL_HAVE_ALL_API) || defined(CONFIG_ETAL_HAVE_SEAMLESS))
	ETAL_STATUS ret;
    etalSeamlessEstimationConfigTy seamlessEstimationConfig;
    etalSeamlessSwitchingConfigTy seamlessSwitchingConfig;
    ETAL_HANDLE hDatapath;
	tSInt retosal;

	etalTestStartup();

    /* Configure tuners for seamless */
    if (etalTestSeamlessDabFmDoCfgTune(&handledab, &handlefm, &hDatapath) == OSAL_ERROR) {
        etalTestPrintError("etalTestSeamlessDabFmDoCfgTune error");
        return OSAL_ERROR;
    }

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

    /* Initial Seamless Switching to DAB configuration with default delay and loudness */
    etalTestSeamlessSwitching_SetDefaultConfig(&seamlessSwitchingConfig, 0);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 15000,
            ETALTEST_SS_STATUS_SUCCESSFUL_WITH_DEFAULT_VALUE)) != OSAL_OK) {
        return retosal;
    }

	/* Start Seamless Estimation */
	etalTestSeamlessEstimationCount = 0;

    etalTestSeamlessEstimation_SetConfig(&seamlessEstimationConfig);

    if ((retosal = etalTestSeamlessEstimation_CmdWaitResp(handledab, handlefm, &seamlessEstimationConfig, 60000,
            ETALTEST_SE_STATUS_SUCCESS)) != OSAL_OK) {
        return retosal;
    }

    /* start DAB mute */
	etalTestPrintNormal("* Mute DAB audio stream\n");
	etal_mute(handledab, true);

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

    /* stop DAB mute */
	etalTestPrintNormal("* Unmute DAB audio stream\n");
	etal_mute(handledab, false);

	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Seamless Switching DAB to FM */
    etalTestSeamlessSwitching_SetConfig(&seamlessSwitchingConfig, 1);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 10000,
            ETALTEST_SS_STATUS_SUCCESSFUL)) != OSAL_OK) {
        return retosal;
    }

    /* wait 5 s */
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

    /* start FM mute */
	etalTestPrintNormal("* Mute FM audio stream\n");
	etal_mute(handlefm, true);

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

    /* stop FM mute */
	etalTestPrintNormal("* Unmute FM audio stream\n");
	etal_mute(handlefm, false);

	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	/* Seamless Switching FM to DAB */
	etalTestSeamlessSwitchingCount = 0;

    etalTestSeamlessSwitching_SetConfig(&seamlessSwitchingConfig, 0);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 10000,
            ETALTEST_SS_STATUS_SUCCESSFUL)) != OSAL_OK) {
        return retosal;
    }

    /* wait 5 s */
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

    /* start DAB mute */
	etalTestPrintNormal("* Mute DAB audio stream\n");
	etal_mute(handledab, true);

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

    /* stop DAB mute */
	etalTestPrintNormal("* Unmute DAB audio stream\n");
	etal_mute(handledab, false);

	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

    /* Start Seamless Estimation */
    etalTestSeamlessEstimationCount = 0;

    etalTestSeamlessEstimation_SetConfig(&seamlessEstimationConfig);

    if ((ret = etal_seamless_estimation_start(handledab, handlefm, &seamlessEstimationConfig)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }
    OSAL_s32ThreadWait(10 * ETAL_TEST_ONE_SECOND);

    /* Stop seamless estimation */
    if ((ret = etal_seamless_estimation_stop(handledab, handlefm)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }
    OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);


	/* Start Seamless Estimation with wrong mode */
	etalTestSeamlessEstimationCount = 0;

    etalTestSeamlessEstimation_SetConfig(&seamlessEstimationConfig);
	seamlessEstimationConfig.mode = 10;

    if ((retosal = etalTestSeamlessEstimation_CmdWaitResp(handledab, handlefm, &seamlessEstimationConfig, 500,
            ETALTEST_SE_STATUS_ERROR_ON_SEAMLESS_ESTIMATION)) != OSAL_ERROR) {
        return retosal;
    }

	OSAL_s32ThreadWait(500);

	/* Seamless Switching DAB to FM with wrong parameters */
	seamlessSwitchingConfig.systemToSwitch = 10;
	seamlessSwitchingConfig.providerType = DABMW_MAIN_AUDIO_DAB_APP;
	seamlessSwitchingConfig.absoluteDelayEstimate = 0;
	seamlessSwitchingConfig.delayEstimate = 0;
	seamlessSwitchingConfig.timestampFAS = 0;
	seamlessSwitchingConfig.timestampSAS = 0;
	seamlessSwitchingConfig.averageRMS2FAS = 0;
	seamlessSwitchingConfig.averageRMS2SAS = 0;

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 500,
            ETALTEST_SS_STATUS_FAILURE)) != OSAL_OK) {
        return retosal;
    }

	/* Seamless Switching FM to DAB with wrong parameters */
	etalTestSeamlessSwitchingCount = 0;

	seamlessSwitchingConfig.systemToSwitch = 10;
	seamlessSwitchingConfig.providerType = DABMW_MAIN_AUDIO_DAB_APP;
	seamlessSwitchingConfig.absoluteDelayEstimate = 0;
	seamlessSwitchingConfig.delayEstimate = 0;
	seamlessSwitchingConfig.timestampFAS = 0;
	seamlessSwitchingConfig.timestampSAS = 0;
	seamlessSwitchingConfig.averageRMS2FAS = 0;
	seamlessSwitchingConfig.averageRMS2SAS = 0;

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 500,
            ETALTEST_SS_STATUS_FAILURE)) != OSAL_OK) {
        return retosal;
    }

    /* Unconfigure tuners */
    if (etalTestSeamlessDabFmUndoCfgTune(&handledab, &handlefm, &hDatapath) == OSAL_ERROR) {
        etalTestPrintError("etalTestSeamlessDabFmUndoCfgTune error");
        return OSAL_ERROR;
    }

    /* Configure tuners for seamless early FM */
    if (etalTestSeamlessFmEarlyDabDoCfgTune(&handledab, &handlefm, &hDatapath) == OSAL_ERROR) {
        etalTestPrintError("etalTestSeamlessDabFmDoCfgTune error");
        return OSAL_ERROR;
    }

    OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);
    /* Initial Seamless Switching to Early FM configuration with default delay and loudness */
    etalTestSeamlessSwitching_SetDefaultConfig(&seamlessSwitchingConfig, 2);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 15000,
            ETALTEST_SS_STATUS_SUCCESSFUL_WITH_DEFAULT_VALUE)) != OSAL_OK) {
        return retosal;
    }

    /* Start Seamless Estimation */
    etalTestSeamlessEstimation_SetConfig(&seamlessEstimationConfig);

    if ((retosal = etalTestSeamlessEstimation_CmdWaitResp(handledab, handlefm, &seamlessEstimationConfig, 60000,
            ETALTEST_SE_STATUS_SUCCESS)) != OSAL_OK) {
        return retosal;
    }

    /* Seamless Switching Early FM to DAB and select audio DAB */
    etalTestSeamlessSwitching_SetConfig(&seamlessSwitchingConfig, 0);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 10000,
            ETALTEST_SS_STATUS_SUCCESSFUL_WITH_DEFAULT_VALUE)) != OSAL_OK) {
        return retosal;
    }
    if ((ret = etal_audio_select(handledab, ETAL_AUDIO_SOURCE_DCOP_STA660)) != ETAL_RET_SUCCESS)
    {
        return OSAL_ERROR;
    }

    /* wait 10 s */
    OSAL_s32ThreadWait(10 * ETAL_TEST_ONE_SECOND);

    /* Seamless Switching DAB to FM */
    etalTestSeamlessSwitching_SetConfig(&seamlessSwitchingConfig, 1);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 10000,
            ETALTEST_SS_STATUS_SUCCESSFUL)) != OSAL_OK) {
        return retosal;
    }

    /* wait 10 s */
    OSAL_s32ThreadWait(10 * ETAL_TEST_ONE_SECOND);

    /* Seamless Switching FM to DAB */
    etalTestSeamlessSwitching_SetConfig(&seamlessSwitchingConfig, 0);

    if ((retosal = etalTestSeamlessSwitching_CmdWaitResp(handledab, handlefm, &seamlessSwitchingConfig, 10000,
            ETALTEST_SS_STATUS_SUCCESSFUL)) != OSAL_OK) {
        return retosal;
    }

    /* wait 10 s */
    OSAL_s32ThreadWait(10 * ETAL_TEST_ONE_SECOND);

    /* Unconfigure tuners */
    if (etalTestSeamlessDabFmUndoCfgTune(&handledab, &handlefm, &hDatapath) == OSAL_ERROR) {
        etalTestPrintError("etalTestSeamlessDabFmUndoCfgTune error");
        return OSAL_ERROR;
    }

#endif // CONFIG_APP_TEST_SEAMLESS && (CONFIG_ETAL_HAVE_ALL_API || CONFIG_ETAL_HAVE_SEAMLESS)
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

