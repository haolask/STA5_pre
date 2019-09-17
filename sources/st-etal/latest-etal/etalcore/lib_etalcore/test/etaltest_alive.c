//!
//!  \file 		etaltest_alive.c
//!  \brief 	<i><b> ETAL test, alive check </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST

#include "etal_api.h"
#include "etaltest.h"

#ifdef CONFIG_APP_TEST_RECEIVER_ALIVE

#include <assert.h>
#define ETAL_TEST_ASSERT(a) (assert(a))
#include "etalinternal.h" // for ETAL_tunerGetAddress, ETAL_handleMakeTuner, ETAL_HINDEX, ETAL_EventToString 
#include "tunerdriver.h" // for TUNERDRIVER_reset_CMOST
#include "bsp_linux.h"   // for BSP_Linux_OpenGPIO and friends

/***************************
 *
 * global variables
 *
 **************************/
OSAL_tSemHandle etalTestAliveErrSem;
tBool etalTestReceiverAliveError;
ETAL_HANDLE etalTestReceiverAliveErrorHandleExpected = ETAL_INVALID_HANDLE;

typedef enum {
	RCV_ALIVE_CHECK_INVALID_HANDLE = 1,
	RCV_ALIVE_CHECK,
	RCV_ALIVE_CHECK_2ND_PASS,
	RCV_ALIVE_ETAL_RESTART
} ReceiverAliveTestTy;

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM)
/***************************
 *
 * etalTestReceiverAliveVerify
 *
 **************************/
static tSInt etalTestReceiverAliveVerify(etalTestBroadcastTy test_mode, tChar *str, tBool *pass)
{
	ETAL_STATUS ret;
#if 0
	EtalAudioInterfTy audioIf;
#endif

	if (test_mode == ETAL_TEST_MODE_FM)
	{
		/* Configure FM etal handles */
		if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		if (handlefm == ETAL_INVALID_HANDLE)
		{
			return OSAL_ERROR;
		}

// Audio path should be correctly set before
#if 0

		/* Configure audio path */
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
#endif

		/* check with valid handle */
		etalTestPrintReportPassStart(RCV_ALIVE_CHECK, ETAL_TEST_MODE_FM, "Alive check");
		if (etal_receiver_alive(handlefm) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("%sa FAILED", str);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Tune to valid FM freq with audio switch */
		
		etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
		if ((ret = etal_tune_receiver(handlefm, ETAL_VALID_FM_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		/* Select audio source */
		if ((ret = etal_audio_select(handlefm, ETAL_AUDIO_SOURCE_STAR_AMFM)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_audio_select FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

		/* check with valid handle a second time */
		etalTestPrintReportPassStart(RCV_ALIVE_CHECK_2ND_PASS, ETAL_TEST_MODE_FM, "Alive check, second pass");
		if (etal_receiver_alive(handlefm) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("%sb FAILED", str);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* wait 5 s */
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	else if (test_mode == ETAL_TEST_MODE_DAB)
	{
#ifdef CONFIG_APP_TEST_DAB
		/* Configure DAB etal handles */
		if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		if (handledab == ETAL_INVALID_HANDLE)
		{
			return OSAL_ERROR;
		}


// Audio path should be correctly set before
#if 0
		/* Configure audio path */
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
#endif

		/* check with valid handle */
		etalTestPrintReportPassStart(RCV_ALIVE_CHECK, ETAL_TEST_MODE_DAB, "Alive check");
		if (etal_receiver_alive(handledab) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("%sc FAILED", str);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Tune to valid DAB freq with audio switch */
		etalTestPrintNormal("* Tune to DAB freq %d", ETAL_VALID_DAB_FREQ);
		if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

		/* depending on the test sequence FIC data may not be already available */
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

		/* check with valid handle a second time */
		etalTestPrintReportPassStart(RCV_ALIVE_CHECK_2ND_PASS, ETAL_TEST_MODE_DAB, "Alive check, second pass");
		if (etal_receiver_alive(handledab) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("%sd FAILED", str);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* wait 5 s */
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
#endif // CONFIG_APP_TEST_DAB
	}
	else if (test_mode == ETAL_TEST_MODE_HD_FM)
	{
		/* Configure FM etal handles */
		if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		// Audio path should be correctly set before
#if 0

		/* Configure audio path */
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
#endif

#if 1 /* change to #if 0 if no HD signal is available, it is not realy mandatory for the test */
		etalTestPrintNormal("* Tune to HD freq %d", ETAL_VALID_HD_FREQ);
		if ((ret = etal_tune_receiver(handlehd, ETAL_VALID_HD_FREQ)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		/* ensure all the INFO TUNE event are delivered */
		OSAL_s32ThreadWait(100);

		// Audio path should be correctly set before
#if 0

		/* Configure audio path */
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
#endif

		/* Enable audio */
		if ((ret = etal_audio_select(handlehd, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_audio_select HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
#endif

		/* check with valid handle */
		etalTestPrintReportPassStart(RCV_ALIVE_CHECK, ETAL_TEST_MODE_HD_FM, "Alive check");
		if (etal_receiver_alive(handlehd) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("%se FAILED", str);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* check with valid handle a second time */
		etalTestPrintReportPassStart(RCV_ALIVE_CHECK_2ND_PASS, ETAL_TEST_MODE_HD_FM, "Alive check, second pass");
		if (etal_receiver_alive(handlehd) != ETAL_RET_SUCCESS)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("%sf FAILED", str);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* wait 5 s */
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestReceiverAliveCheckAndRestart
 *
 **************************/
static tVoid etalTestReceiverAliveCheckAndRestart(tBool *pass)
{
	tBool local_pass;
	EtalNVMLoadConfig NVMLoadConfig;

	etalTestPrintNormal("* Wait max 4 minutes for etal reinitialize");
	OSAL_s32SemaphoreWait(etalTestAliveErrSem, 4 * 60 * ETAL_TEST_ONE_SECOND);
	local_pass = FALSE;
	if (etalTestReceiverAliveError)
	{
		local_pass = TRUE;
		etalTestPrintReportPassStart(RCV_ALIVE_ETAL_RESTART, ETAL_TEST_MODE_NONE, "Restarting ETAL");

		NVMLoadConfig.m_load_DAB_landscape = 1;
		NVMLoadConfig.m_load_AMFM_landscape = 0;
		NVMLoadConfig.m_load_HD_landscape = 0;

		if (etal_reinitialize(NVMLoadConfig) != ETAL_RET_SUCCESS)
		{
			ASSERT_ON_DEBUGGING(0);
			local_pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass FAILED");
		}
		else
		{
			etalTestPrintNormal("* ETAL reinitialized");
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	
	if (!local_pass)
	{
		*pass = FALSE;
	}
}
#endif //defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM)

#ifdef CONFIG_APP_TEST_DAB
/***************************
 *
 * EtalTest_AssertReset_MDR
 *
 **************************/
static tSInt EtalTest_AssertReset_MDR(tVoid)
{
	tS32 fd;

	fd = BSP_Linux_OpenGPIO("20" /*MDR_ACCORDO2_RESET_GPIO*/, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (fd < 0)
	{
		return OSAL_ERROR;
	}

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, 0);

	BSP_Linux_CloseGPIO(fd);

	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(5);
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DAB

#ifdef CONFIG_APP_TEST_HDRADIO_FM
/***************************
 *
 * EtalTest_AssertReset_HDRADIO
 *
 **************************/
static tSInt EtalTest_AssertReset_HDRADIO(tVoid)
{
	tS32 fd;

	fd = BSP_Linux_OpenGPIO("20" /*HDRADIO_ACCORDO2_RESET_GPIO*/, GPIO_OPEN_WRITE, GPIO_IRQ_NONE, NULL);
	if (fd < 0)
	{
		return OSAL_ERROR;
	}

	/*
	 * Assert the reset line
	 */
	BSP_Linux_WriteGPIO(fd, 0);

	BSP_Linux_CloseGPIO(fd);
	/*
	 * Let the signal settle
	 */
	OSAL_s32ThreadWait(2);
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#endif // CONFIG_APP_TEST_RECEIVER_ALIVE

/***************************
 *
 * etalTestReceiverAlive
 *
 **************************/
tSInt etalTestReceiverAlive(void)
{
#ifdef CONFIG_APP_TEST_RECEIVER_ALIVE
	tBool pass;
	EtalDeviceDesc deviceDescription;

#ifdef CONFIG_APP_TEST_FM
	ETAL_HANDLE hTuner;
#endif

	etalTestStartup();
	pass = TRUE;

	if (etalTestAliveErrSem == 0)
	{
		if (OSAL_s32SemaphoreCreate("Sem_etalTestAliveErr", &etalTestAliveErrSem, 0) == OSAL_ERROR)
		{
			return OSAL_ERROR;
		}
	}

	/* check with invalid handle */
	etalTestPrintReportPassStart(RCV_ALIVE_CHECK_INVALID_HANDLE, ETAL_TEST_MODE_DAB, "Check invalid handle");
	if (etal_receiver_alive(handledab) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(RCV_ALIVE_CHECK_INVALID_HANDLE, ETAL_TEST_MODE_FM, "Check invalid handle");
	if (etal_receiver_alive(handlefm) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(RCV_ALIVE_CHECK_INVALID_HANDLE, ETAL_TEST_MODE_HD_FM, "Check invalid handle");
	if (etal_receiver_alive(handlehd) != ETAL_RET_INVALID_HANDLE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* simulation of FM alive error */
#ifdef CONFIG_APP_TEST_FM
	if (etalTestReceiverAliveVerify(ETAL_TEST_MODE_FM, "pass2", &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestReceiverAliveError = FALSE;
	etalTestReceiverAliveErrorHandleExpected = handlefm;
	etalTestPrintNormal("* Simulate FM alive error");
	hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)0);
	if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (TUNERDRIVER_reset_CMOST(ETAL_handleTunerGetIndex(hTuner)) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestReceiverAliveCheckAndRestart(&pass);
	if (pass)
	{
		if (etalTestReceiverAliveVerify(ETAL_TEST_MODE_FM, "pass3", &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#endif

	/* simulation of DAB alive error */
#ifdef CONFIG_APP_TEST_DAB
	if (etalTestReceiverAliveVerify(ETAL_TEST_MODE_DAB, "pass2", &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestReceiverAliveError = FALSE;
	etalTestReceiverAliveErrorHandleExpected = handledab;
	etalTestPrintNormal("* Simulate DAB alive error");
	if (EtalTest_AssertReset_MDR() != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestReceiverAliveCheckAndRestart(&pass);
	if (pass)
	{
		if (etalTestReceiverAliveVerify(ETAL_TEST_MODE_DAB, "pass4", &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#endif

	/* simulation of HD radio alive error */
#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestReceiverAliveVerify(ETAL_TEST_MODE_HD_FM, "pass2", &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestReceiverAliveError = FALSE;
	etalTestReceiverAliveErrorHandleExpected = handlehd;
	etalTestPrintNormal("* Simulate HD alive error");
	if (EtalTest_AssertReset_HDRADIO() != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestReceiverAliveCheckAndRestart(&pass);
	if (pass)
	{
		if (etalTestReceiverAliveVerify(ETAL_TEST_MODE_HD_FM, "pass5", &pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_RECEIVER_ALIVE
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

