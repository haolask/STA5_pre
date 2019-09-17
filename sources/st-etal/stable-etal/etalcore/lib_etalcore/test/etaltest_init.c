//!
//!  \file 		etaltest_init.c
//!  \brief 	<i><b> ETAL test, system initialization </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST

#include "etal_api.h"
#include "etaltest.h"
#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BC)
	#include "../../../tuner_driver/exports/firmware/TDA7707_OM_CUT_BC.boot.h"
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BF)
	#include "../../../tuner_driver/exports/firmware/TDA7707_OM_CUT_BF.boot.h"
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BG)
	#include "../../../tuner_driver/exports/firmware/TDA7707_OM_CUT_BG.boot.h"
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_CA)
	#include "../../../tuner_driver/exports/firmware/TDA7707_OM_CUT_CA.boot.h"
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_DA)
	#include "../../../tuner_driver/exports/firmware/TDA7707_OM_CUT_DA.boot.h"
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_EB)
	#include "../../../tuner_driver/exports/firmware/TDA7707_OM_CUT_EB.boot.h"
#endif
#endif

#ifdef CONFIG_APP_TEST_INITIALIZATION
#include "tunerdriver.h" // for boot_cmost.h
#include "boot_cmost.h" // for firmware images

#if defined (CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM)
	#include "etalinternal.h" // for ETAL_tunerGetType, used in etalTestCustomParamPrepare
#endif

#ifdef CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM
	/*
	 * These includes and the array definitions are under the above ifdef
	 * to avoid hundreds of error messages during build
	 */
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_T
	#endif
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_S
	#endif
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_T
	#endif
	#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
		#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_S
	#endif
#endif // CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM
#endif // CONFIG_APP_TEST_INITIALIZATION

#ifdef CONFIG_ETAL_SUPPORT_DCOP

#if defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

	tChar *etaltest_dcopFlash_filename[2] = {
#ifndef CONFIG_HOST_OS_TKERNEL
	#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_SPI_CPHA1_CPOL1)
		"phase1_spi11.bin",
	#else
		"phase1_spi00.bin", /* spi00 or i2c */
	#endif
	#if defined(CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
		"dcop_hdr_fw_ram_spi11.bin"
	#else
		"dcop_hdr_fw_ram_spi00.bin" /* spi00 or i2c */
	#endif
#else
	#if defined(CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_SPI_CPHA1_CPOL1)
		"/uda/phase1_spi11.bin",
	#else
		"/uda/phase1_spi00.bin", /* spi00 or i2c */
	#endif
	#if defined(CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
		"/uda/dcop_hdr_fw_ram_spi11.bin"
	#else
		"/uda/dcop_hdr_fw_ram_spi00.bin" /* spi00 or i2c */
	#endif
#endif /* CONFIG_HOST_OS_TKERNEL */
	};

#elif defined(CONFIG_ETAL_SUPPORT_DCOP_MDR)

#ifndef CONFIG_HOST_OS_TKERNEL
	tChar *etaltest_dcopFlash_filename[2] = { "bootstrap.bin", "dcop_dab_fw.bin" };
#else
	tChar *etaltest_dcopFlash_filename[2] = { "/uda/bootstrap.bin", "/uda/dcop_dab_fw.bin" };
#endif /* CONFIG_HOST_OS_TKERNEL */

#else

	tChar *etaltest_dcopFlash_filename[2] = { "", "" };

#endif /* CONFIG_ETAL_SUPPORT_DCOP_HDRADIO */

#endif /* CONFIG_ETAL_SUPPORT_DCOP */

#define ETAL_TEST_INIT_FAIL_ALL   255

#ifdef CONFIG_APP_TEST_INITIALIZATION
typedef enum {
    INIT_NULL_PARAM = 1,
	INIT_ZERO_FILL_PARAM,
	INIT_VALID_CB_NOTIFY_PARAM,
	INIT_TUNE_DAB_FREQ,
	INIT_TUNE_DAB_OTHER_FREQ,
	INIT_EMPTY_CMOST_IMG_1ST_TUNER,
	INIT_EMPTY_CMOST_IMG_2ND_TUNER,
	INIT_EMPTY_CMOST_IMG_BOTH_TUNERS,
	INIT_ILLEGAL_CMOST_IMG_BOTH_TUNERS,
	INIT_ILLEGAL_CMOST_IMG_TUNER0,
	INIT_ILLEGAL_CMOST_IMG_TUNER1,
	INIT_GET_STATUS,
	INIT_CHECK_STATUS,
	INIT_VALID_CMOST_IMG,
	INIT_TUNER_VALID_STEP_BY_STEP,
	INIT_TUNER_VALID_STEP_BY_STEP_2,
	INIT_BOTH_TUNERS_ENABLED,
	INIT_2ND_TUNER_ENABLED,
	INIT_1ST_TUNER_ENABLED,
	INIT_BOTH_TUNERS_DISABLED,
	INIT_CHECK_STATUS_2,
	INIT_CREATE_RCV_1ST_TUNER,
	INIT_CREATE_RCV_2ND_TUNER,
	INIT_NO_PARAM_1ST_TUNER,
	INIT_CHECK_DEFAULT_PARAM_1ST_TUNER,
	INIT_DEFAULT_PARAM_1ST_TUNER,
	INIT_CHECK_CUSTOM_PARAM_1ST_TUNER,
	INIT_CUSTOM_PARAM_1ST_TUNER,
	INIT_CUSTOM_PARAM_BOTH_TUNERS,
	INIT_CHECK_CUSTOM_PARAM_BOTH_TUNERS,
	INIT_INVALID_CUSTOM_PARAM_1ST_TUNER,
	INIT_CHECK_STATUS_1ST_TUNER,
	INIT_ILLEGAL_CUSTOM_PARAM_SIZE_1ST_TUNER,
	INIT_CHECK_STATUS_1ST_TUNER_2,
	INIT_ILLEGAL_CUSTOM_PARAM_SIZE_2ND_TUNER,
	INIT_CHECK_STATUS_BOTH_TUNERS,
	INIT_DCOP_DISABLED,
	INIT_CREATE_DCOP_RECEIVER_EXPECTED_ERROR,
	INIT_DCOP_ENABLED,
	INIT_CREATE_DCOP_RECEIVER,
	INIT_DCOP_EN_INVALID_DOFLASH_CONFIG,
	INIT_DCOP_EN_INVALID_DOFLASH_DODL_CONFIG,
	INIT_DCOP_EN_INVALID_DODL_CONFIG,
	INIT_DCOP_EN_INVALID_DODUMP_CONFIG,
	INIT_DCOP_EN_VALID_DODL_CONFIG,
	INIT_XTAL_ALIGNMENT
} InitializationTestTy;
#endif

/***************************
 *
 * etalTestStartETAL
 *
 **************************/
/*
 * This function is called only once by the etaltest main
 * to perform ETAL initialization; it may be called either
 * to explicitly test the etal_initialize API, or just to
 * intialize ETAL for the other tests.
 * In the first case it should be invoked from the etalTestInitialize
 * function, not directly.
 */
tSInt etalTestStartETAL(EtalTraceConfig *tr_config)
{
	ETAL_STATUS ret;
	EtalHardwareAttr init_params;

#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
	tU8 i;
	tU32               downloadImage1Size = sizeof(CMOST_Firmware_STAR_T);
	tU8               *downloadImage1 = (tU8 *)CMOST_Firmware_STAR_T;
	/* The Tuner 2 firmware image */
	tU32            downloadImage2Size = sizeof(CMOST_Firmware_STAR_T);
	tU8             *downloadImage2 = (tU8 *)CMOST_Firmware_STAR_T;
#else
	/* The Tuner 1 uses embedded FW image */
	tU32            downloadImage1Size = 0;
	tU8             *downloadImage1 = NULL;
	/* The Tuner 2 uses embedded FW image */
	tU32            downloadImage2Size = 0;
	tU8             *downloadImage2 = NULL;
#endif


	OSAL_pvMemorySet((tVoid *)&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = userNotificationHandler;
	init_params.m_NVMLoadConfig.m_load_DAB_landscape = 1;

	init_params.m_tunerAttr[0].m_DownloadImageSize = downloadImage1Size;
	init_params.m_tunerAttr[0].m_DownloadImage = downloadImage1;

	init_params.m_tunerAttr[1].m_DownloadImageSize = downloadImage2Size;
	init_params.m_tunerAttr[1].m_DownloadImage = downloadImage2;

#ifdef CONFIG_COMM_CMOST_FIRMWARE_IMAGE
	for(i = 0; i < 2; i++)
	{
		 if ((init_params.m_tunerAttr[i].m_DownloadImageSize != 0) &&
		 	 (init_params.m_tunerAttr[i].m_DownloadImage != 0))
		{
			 init_params.m_tunerAttr[i].m_useDownloadImage = TRUE;
		}
	}
#endif

	if (tr_config != NULL)
	{
		OSAL_pvMemoryCopy((tVoid *)&init_params.m_traceConfig, tr_config, sizeof (EtalTraceConfig));
	}

	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_initialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_BOARD_ACCORDO5
	system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
#endif
#ifndef CONFIG_HOST_OS_WIN32
	system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
#endif

	return OSAL_OK;
}

#ifdef CONFIG_APP_TEST_INITIALIZATION
#if defined (CONFIG_APP_TEST_INITIALIZATION_PARAMETER) || \
	defined (CONFIG_APP_TEST_INITIALIZATION_TUNER) || \
	defined (CONFIG_APP_TEST_INITIALIZATION_DCOP) || \
	defined (CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM) || \
	defined (CONFIG_APP_TEST_XTAL_ALIGNMENT)
/***************************
 *
 * etalTestDeinit
 *
 **************************/
static tSInt etalTestDeinit(tVoid)
{
	ETAL_STATUS ret;

	/* etalTestPrint may use plain printf for initialization tests
	 * or if ETAL trace is disabled.
	 * Ensure the prints are not truncated by the etal_deinitialize */
#ifndef CONFIG_HOST_OS_TKERNEL
	fflush(NULL);
#endif

	ret = etal_deinitialize();
	if (ret != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_deinitialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif

#if defined (CONFIG_APP_TEST_INITIALIZATION_PARAMETER)
/***************************
 *
 * etalTestInitializeParameterCheck
 *
 **************************/
static tSInt etalTestInitializeParameterCheck(tBool *pass_out)
{
	EtalHardwareAttr attr;
	ETAL_STATUS ret;
	tBool pass;

	pass = TRUE;

	etalTestPrintReportPassStart(INIT_NULL_PARAM, ETAL_TEST_MODE_NONE, "NULL parameter");
	ret = etal_initialize(NULL);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED (%d)", ret);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintReportPassStart(INIT_ZERO_FILL_PARAM, ETAL_TEST_MODE_NONE, "Zero-filled parameter");
	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1b FAILED (%d)", ret);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintReportPassStart(INIT_VALID_CB_NOTIFY_PARAM, ETAL_TEST_MODE_NONE, "Valid m_cbNotify parameter");
	attr.m_cbNotify = userNotificationHandler;
	if (etal_initialize(&attr) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	/* TODO other parameters */

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;

}
#endif // CONFIG_APP_TEST_INITIALIZATION_PARAMETER

#if defined (CONFIG_APP_TEST_LANDSCAPE_MANAGEMENT)
/***************************
 *
 * etalTestLandscapeManagement
 *
 **************************/
static tSInt etalTestLandscapeManagement(tBool *pass_out)
{

	tBool pass = TRUE;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	EtalHardwareAttr init_params;
	ETAL_STATUS ret;
	EtalMemoryClearConfig MemoryClearConfig;

	EtalEnsembleList ens_list;

	pass = TRUE;
	OSAL_pvMemorySet((tVoid *)&init_params, 0x0, sizeof(EtalHardwareAttr));
	OSAL_pvMemorySet(&MemoryClearConfig, 0, sizeof(EtalMemoryClearConfig));

	init_params.m_cbNotify = userNotificationHandler;
	init_params.m_NVMLoadConfig.m_load_DAB_landscape = 0;

	/* Landscape is not loaded */
	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_initialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* No ensemble is expected : NVM is not loaded */
	if(ens_list.m_ensembleCount != 0)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	/* Tune on ETAL_VALID_DAB_FREQ */
	etalTestPrintReportPassStart(INIT_TUNE_DAB_FREQ, ETAL_TEST_MODE_DAB, "* Tune freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* One ensemble is expected : Tune on ETAL_VALID_DAB_FREQ */
	if(ens_list.m_ensembleCount != 1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	/* Tune on ETAL_VALID_DAB_OTHER_FREQ */
	etalTestPrintReportPassStart(INIT_TUNE_DAB_OTHER_FREQ, ETAL_TEST_MODE_DAB, "* Tune freq %d", ETAL_VALID_DAB_OTHER_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_OTHER_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* Two ensembles are expected : Tune on ETAL_VALID_DAB_FREQ and ETAL_VALID_DAB_OTHER_FREQ */
	if(ens_list.m_ensembleCount != 2)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	/* NVM is cleared */
	MemoryClearConfig.m_clear_DAB_volatile_memory = 1;
	MemoryClearConfig.m_clear_DAB_non_volatile_memory = 1;
	if ((ret = etal_clear_landscape(MemoryClearConfig)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_clear_landscape (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* One ensemble is expected : Still Tuned on ETAL_VALID_DAB_OTHER_FREQ */
	if(ens_list.m_ensembleCount != 1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	/* Tune on ETAL_VALID_DAB_FREQ */
	etalTestPrintReportPassStart(INIT_TUNE_DAB_FREQ, ETAL_TEST_MODE_DAB, "* Tune freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* Two ensembles are expected : Tune on ETAL_VALID_DAB_FREQ and ETAL_VALID_DAB_OTHER_FREQ */
	if(ens_list.m_ensembleCount != 2)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Landscape is saved in NVM */
	if((ret = etal_deinitialize()) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_deinitialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* Landscape is loaded */
	init_params.m_NVMLoadConfig.m_load_DAB_landscape = 1;
	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_initialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* Two ensembles are expected : Landscape has been loaded */
	if(ens_list.m_ensembleCount != 2)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	/* Landscape is cleared */
	MemoryClearConfig.m_clear_DAB_volatile_memory = 1;
	MemoryClearConfig.m_clear_DAB_non_volatile_memory = 1;
	if ((ret = etal_clear_landscape(MemoryClearConfig)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_clear_landscape (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}
	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* No ensemble is expected : NVM has been cleared */
	if(ens_list.m_ensembleCount != 0)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	/* Tune on ETAL_VALID_DAB_FREQ */
	etalTestPrintReportPassStart(INIT_TUNE_DAB_FREQ, ETAL_TEST_MODE_DAB, "* Tune freq %d", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	/* One ensemble is expected : Tune on ETAL_VALID_DAB_FREQ */
	if(ens_list.m_ensembleCount != 1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Landscape is saved in NVM */
	if((ret = etal_deinitialize()) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_deinitialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	/* Landscape is not loaded */
	init_params.m_NVMLoadConfig.m_load_DAB_landscape = 0;
	if ((ret = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_initialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* No ensemble is expected : Previous landscape has not been loaded */
	if ((ret = etal_get_ensemble_list(&ens_list)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_get_ensemble_list (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#ifdef CONFIG_TRACE_CLASS_ETAL
	etalTestPrintEnsembleList(&ens_list);

	if(ens_list.m_ensembleCount != 0)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
#endif

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Landscape is saved in NVM */
	if((ret = etal_deinitialize()) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_deinitialize (%s, %d)", ETAL_STATUS_toString(ret), ret);
		pass = FALSE;
	}
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

	if (!pass)
	{
		*pass_out = FALSE;
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_INITIALIZATION_PARAMETER

#if defined (CONFIG_APP_TEST_INITIALIZATION_TUNER)
/***************************
 *
 * etalTestInitializeTunerEmpty
 *
 **************************/
static tSInt etalTestInitializeTunerEmpty(tU32 pass_id, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalHardwareAttr attr;
	tBool pass;

	pass = TRUE;

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;
	attr.m_tunerAttr[0].m_useDownloadImage = 1;

	etalTestPrintReportPassStart(INIT_EMPTY_CMOST_IMG_1ST_TUNER, ETAL_TEST_MODE_NONE, "Empty CMOST image, first tuner");
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#if defined (CONFIG_MODULE_INTEGRATED)
	etalTestPrintReportPassStart(INIT_EMPTY_CMOST_IMG_2ND_TUNER, ETAL_TEST_MODE_NONE, "Empty CMOST image, second tuner");

	attr.m_tunerAttr[0].m_useDownloadImage = 0;
	attr.m_tunerAttr[1].m_useDownloadImage = 1;

	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintInitStatus();
		etalTestPrintNormal("pass%db FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintReportPassStart(INIT_EMPTY_CMOST_IMG_BOTH_TUNERS, ETAL_TEST_MODE_NONE, "Empty CMOST image, both tuners");

	attr.m_tunerAttr[0].m_useDownloadImage = 1;
	attr.m_tunerAttr[1].m_useDownloadImage = 1;

	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%db FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#endif
	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestInitializeTunerDummy
 *
 **************************/
static tSInt etalTestInitializeTunerDummy(tU32 failed_index, tU32 pass_id, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalHardwareAttr attr;
	tU32 dummy_image;
	EtalInitStatus init_status;
	tBool pass;
	tBool pass1;

	pass = TRUE;
	dummy_image = 0xFFFFFFFF; // end of file marker

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;
	attr.m_tunerAttr[0].m_useDownloadImage = 1;
#if defined (CONFIG_MODULE_INTEGRATED)
	attr.m_tunerAttr[1].m_useDownloadImage = 1;
#endif

	if (failed_index == ETAL_TEST_INIT_FAIL_ALL)
	{
		etalTestPrintReportPassStart(INIT_ILLEGAL_CMOST_IMG_BOTH_TUNERS, ETAL_TEST_MODE_NONE, "Illegal CMOST image, both tuners");
	}
	else if (failed_index == 0)
	{
		etalTestPrintReportPassStart(INIT_ILLEGAL_CMOST_IMG_TUNER0, ETAL_TEST_MODE_NONE, "Illegal CMOST image, tuner 0");
	}
	else if (failed_index == 1)
	{
		etalTestPrintReportPassStart(INIT_ILLEGAL_CMOST_IMG_TUNER1, ETAL_TEST_MODE_NONE, "Illegal CMOST image, tuner 1");
	}

	if ((failed_index == 0) || (failed_index == ETAL_TEST_INIT_FAIL_ALL))
	{
		attr.m_tunerAttr[0].m_DownloadImageSize = 3;
		attr.m_tunerAttr[0].m_DownloadImage = (tU8 *)&dummy_image;
	}
#if defined (CONFIG_MODULE_INTEGRATED)
	if ((failed_index == 1) || (failed_index == ETAL_TEST_INIT_FAIL_ALL))
	{
		attr.m_tunerAttr[1].m_DownloadImageSize = 3;
		attr.m_tunerAttr[1].m_DownloadImage = (tU8 *)&dummy_image;
	}
#endif
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_NO_HW_MODULE)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintReportPassStart(INIT_GET_STATUS, ETAL_TEST_MODE_NONE, "Get init status");
		if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%db FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);

			etalTestPrintReportPassStart(INIT_CHECK_STATUS, ETAL_TEST_MODE_NONE, "Check init status");
			pass1 = TRUE;
			if (failed_index == ETAL_TEST_INIT_FAIL_ALL)
			{
				if (init_status.m_tunerStatus[0].m_deviceStatus != deviceDownload)
				{
					pass1 = FALSE;
				}
#if defined (CONFIG_MODULE_INTEGRATED)
				if (init_status.m_tunerStatus[1].m_deviceStatus != deviceDownload)
				{
					pass1 = FALSE;
				}
#endif
				if (!pass1)
				{
					etalTestPrintNormal("pass%dc FAILED", pass_id);
					etalTestPrintInitStatus();
				}
			}
			else if (failed_index == 0)
			{
				if (init_status.m_tunerStatus[0].m_deviceStatus != deviceDownload)
				{
					pass1 = FALSE;
				}
#if defined (CONFIG_MODULE_INTEGRATED)
				if (init_status.m_tunerStatus[1].m_deviceStatus != deviceAvailable)
				{
					pass1 = FALSE;
				}
#endif
				if (!pass1)
				{
					etalTestPrintNormal("pass%dd FAILED", pass_id);
					etalTestPrintInitStatus();
				}
			}
			else if (failed_index == 1)
			{
				if (init_status.m_tunerStatus[0].m_deviceStatus != deviceAvailable)
				{
					pass1 = FALSE;
				}
#if defined (CONFIG_MODULE_INTEGRATED)
				if (init_status.m_tunerStatus[1].m_deviceStatus != deviceDownload)
				{
					pass1 = FALSE;
				}
#endif
			}
			if (!pass1)
			{
				pass = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass%de FAILED", pass_id);
				etalTestPrintInitStatus();
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}
	}

	if (ret == ETAL_RET_SUCCESS)
	{
		if (etalTestDeinit() != OSAL_OK)
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
 * etalTestInit_GetFWReferences
 *
 **************************/
static tSInt etalTestInit_GetFWReferences(EtalHardwareAttr *attr)
{
	BootTunerTy tuner_type;
#if defined (CONFIG_MODULE_INTEGRATED)
	BootTunerTy tuner_type1;
#endif

	/* This assumes that all tuners in the system are of the same type,
	 * This is not true for old MTD board. It would be better to get the
	 * type from etalTuner
	 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_S;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_S;
#endif
	if (BOOT_getFirmwareReferences_CMOST(tuner_type, NULL, &attr->m_tunerAttr[0].m_DownloadImage, &attr->m_tunerAttr[0].m_DownloadImageSize, NULL, NULL) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#if defined (CONFIG_MODULE_INTEGRATED)
#if defined (CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707)
	tuner_type1 = BOOT_TUNER_STAR_T;
#elif defined (CONFIG_MODULE_INTEGRATED_WITH_TDA7707_TDA7708)
	tuner_type1 = BOOT_TUNER_STAR_S;
#else
	tuner_type1 = BOOT_TUNER_UNKNOWN;
#endif

	if (BOOT_getFirmwareReferences_CMOST(tuner_type1, NULL, &attr->m_tunerAttr[1].m_DownloadImage, &attr->m_tunerAttr[1].m_DownloadImageSize, NULL, NULL) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
	return OSAL_OK;
}

/***************************
 *
 * etalTestInitializeTunerValid
 *
 **************************/
static tSInt etalTestInitializeTunerValid(tU32 pass_id, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalHardwareAttr attr;
	tChar descr[12];

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;
	attr.m_tunerAttr[0].m_useDownloadImage = 1;
#if defined (CONFIG_MODULE_INTEGRATED)
	attr.m_tunerAttr[1].m_useDownloadImage = 1;
	OSAL_s32NPrintFormat(descr, 12, "both tuners");
#else
	OSAL_s32NPrintFormat(descr, 12, "tuner 0");
#endif

	if (etalTestInit_GetFWReferences(&attr) != OSAL_OK)
	{
		etalTestPrintError("Fetching the CMOST images");
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(INIT_VALID_CMOST_IMG, ETAL_TEST_MODE_NONE, "Valid CMOST image, %s", descr);
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (ret == ETAL_RET_SUCCESS)
	{
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestInitializeTunerValidStepByStep
 *
 **************************/
static tSInt etalTestInitializeTunerValidStepByStep(tU32 pass_id, tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalHardwareAttr attr;
	tChar descr[12];

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;
	attr.m_tunerAttr[0].m_useDownloadImage = 1;
#if defined (CONFIG_MODULE_INTEGRATED)
	attr.m_tunerAttr[1].m_useDownloadImage = 1;
	OSAL_s32NPrintFormat(descr, 12, "both tuners");
#else
	OSAL_s32NPrintFormat(descr, 12, "tuner 0");
#endif

	if (etalTestInit_GetFWReferences(&attr) != OSAL_OK)
	{
		etalTestPrintError("Fetching the CMOST images");
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(INIT_TUNER_VALID_STEP_BY_STEP, ETAL_TEST_MODE_NONE, "etalTestInitializeTunerValidStepByStep  %s", descr);

	// Start by initializing etal only, no tuner no dcop
	attr.m_tunerAttr[0].m_isDisabled = TRUE;
	attr.m_tunerAttr[1].m_isDisabled = TRUE;
	attr.m_DCOPAttr.m_isDisabled = TRUE;
	ret = etal_initialize(&attr);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d etal_initialize FAILED", pass_id);
		etalTestPrintInitStatus();
	}


	// now start Tuner 1
	attr.m_tunerAttr[0].m_isDisabled = FALSE;
	ret = etal_tuner_initialize(0, &attr.m_tunerAttr[0], FALSE);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d etal_tuner_initialize FAILED", pass_id);
		etalTestPrintInitStatus();
	}
#if defined (CONFIG_MODULE_INTEGRATED)
	// now start Tuner 2
	attr.m_tunerAttr[1].m_isDisabled = FALSE;
	ret = etal_tuner_initialize(1, &attr.m_tunerAttr[1], FALSE);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d etal_tuner_initialize 2 FAILED", pass_id);
		etalTestPrintInitStatus();
	}
#endif

	// now start DCOP
	attr.m_DCOPAttr.m_isDisabled = FALSE;
	ret = etal_dcop_initialize(&attr.m_DCOPAttr, ETAL_DCOP_INIT_RESET_ONLY);

	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d DCOP RESET ONLY FAILED", pass_id);
	}
		
	ret = etal_dcop_initialize(&attr.m_DCOPAttr, ETAL_DCOP_INIT_FULL);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d DCOP FAILED", pass_id);
	}
	
	if (TRUE == *pass_out)	
	{
		etalTestPrintReportPassEnd(testPassed);
	}


	if (ret == ETAL_RET_SUCCESS)
	{
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}


	// let's now try 
	// imaging the DCOP already booted
	etalTestPrintReportPassStart(INIT_TUNER_VALID_STEP_BY_STEP_2, ETAL_TEST_MODE_NONE, "etalTestInitializeTunerValidStepByStep Step 2  %s", descr);
	
	// Start by initializing etal only, no tuner no dcop
	attr.m_tunerAttr[0].m_isDisabled = TRUE;
	attr.m_tunerAttr[1].m_isDisabled = TRUE;
	attr.m_DCOPAttr.m_isDisabled = TRUE;
	ret = etal_initialize(&attr);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d etal_initialize FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	
	
	// now start Tuner 1
	attr.m_tunerAttr[0].m_isDisabled = FALSE;
	ret = etal_tuner_initialize(0, &attr.m_tunerAttr[0], TRUE);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d etal_tuner_initialize FAILED", pass_id);
		etalTestPrintInitStatus();
	}
#if defined (CONFIG_MODULE_INTEGRATED)
	// now start Tuner 2
	attr.m_tunerAttr[1].m_isDisabled = FALSE;
	ret = etal_tuner_initialize(1, &attr.m_tunerAttr[1], TRUE);
		
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d etal_tuner_initialize 2 FAILED", pass_id);
		etalTestPrintInitStatus();
	}
#endif
	
	// resimulate a DAB Reset : 
	// the deinit is doing a DAB Power Down.
	// so we cannot simulate already started
	//
	attr.m_DCOPAttr.m_isDisabled = FALSE;
	ret = etal_dcop_initialize(&attr.m_DCOPAttr, ETAL_DCOP_INIT_RESET_ONLY);

	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d DCOP RESET FAILED", pass_id);
	}	

	// wait the DCOP boot time
	// 
	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);

		

	// now start DCOP
	// assuming already fully started
	// 
	attr.m_DCOPAttr.m_isDisabled = FALSE;
	ret = etal_dcop_initialize(&attr.m_DCOPAttr, ETAL_DCOP_INIT_ALREADY_STARTED);
	
	if (ret != ETAL_RET_SUCCESS)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%d DCOP FAILED", pass_id);
	}
	
	if (TRUE == *pass_out)	
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	
	if (ret == ETAL_RET_SUCCESS)
	{
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	
	return OSAL_OK;
}

#if defined (CONFIG_MODULE_INTEGRATED)
/***************************
 *
 * etalTestInitTuner_ConfigReceiver
 *
 **************************/
static tSInt etalTestInitTuner_ConfigReceiver(tU32 id, ETAL_STATUS expected_status, tBool *pass_out)
{
	EtalReceiverAttr receiver_attr;
	ETAL_HANDLE hReceiver;
	ETAL_HANDLE hFrontend;

	OSAL_pvMemorySet(&receiver_attr, 0x00, sizeof(EtalReceiverAttr));

	hReceiver = ETAL_INVALID_HANDLE;
	hFrontend = ETAL_MAKE_FRONTEND_HANDLE(id, ETAL_FE_FOREGROUND);

	receiver_attr.m_Standard = ETAL_BCAST_STD_FM;
	receiver_attr.m_FrontEnds[0] = hFrontend;

	if (etal_config_receiver(&hReceiver, &receiver_attr) != expected_status)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestInitCheckStatus
 *
 **************************/
static tSInt etalTestInitCheckStatus(EtalInitStatus *status, tBool t1_enabled, tBool t2_enabled)
{
	if (t1_enabled && t2_enabled)
	{
		if ((status->m_tunerStatus[0].m_deviceStatus != deviceAvailable) ||
			(status->m_tunerStatus[1].m_deviceStatus != deviceAvailable))
		{
			return OSAL_ERROR;
		}
	}
	else if (t1_enabled && !t2_enabled)
	{
		if ((status->m_tunerStatus[0].m_deviceStatus != deviceAvailable) ||
			(status->m_tunerStatus[1].m_deviceStatus != deviceDisabled))
		{
			return OSAL_ERROR;
		}
	}
	else if (!t1_enabled && t2_enabled)
	{
		if ((status->m_tunerStatus[0].m_deviceStatus != deviceDisabled) ||
			(status->m_tunerStatus[1].m_deviceStatus != deviceAvailable))
		{
			return OSAL_ERROR;
		}
	}
	else if (!t1_enabled && !t2_enabled)
	{
		if ((status->m_tunerStatus[0].m_deviceStatus != deviceDisabled) ||
			(status->m_tunerStatus[1].m_deviceStatus != deviceDisabled))
		{
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestInitializeTunerDisabled_internal
 *
 **************************/
static tSInt etalTestInitializeTunerDisabled_internal(EtalHardwareAttr *attr, tBool t1_enabled, tBool t2_enabled, tU32 pass_id, tBool *pass_out, tChar *str)
{
	tSInt ret;
	ETAL_STATUS retinit1;
	ETAL_STATUS retinit2;
	tBool deinit;
	EtalInitStatus status;
	tBool pass;
	tBool pass1;

	if (t1_enabled && t2_enabled)
	{
		etalTestPrintReportPassStart(INIT_BOTH_TUNERS_ENABLED, ETAL_TEST_MODE_NONE, "Both tuners enabled, %s", str);
		retinit1 = ETAL_RET_SUCCESS;
		retinit2 = ETAL_RET_SUCCESS;
	}
	else if (t1_enabled && !t2_enabled)
	{
		etalTestPrintReportPassStart(INIT_2ND_TUNER_ENABLED, ETAL_TEST_MODE_NONE, "Second tuner disabled, %s", str);
		retinit1 = ETAL_RET_SUCCESS;
		retinit2 = ETAL_RET_FRONTEND_LIST_ERR;
	}
	else if (!t1_enabled && t2_enabled)
	{
		etalTestPrintReportPassStart(INIT_1ST_TUNER_ENABLED, ETAL_TEST_MODE_NONE, "First tuner disabled, %s", str);
		retinit1 = ETAL_RET_FRONTEND_LIST_ERR;
		retinit2 = ETAL_RET_SUCCESS;
	}
	else if (!t1_enabled && !t2_enabled)
	{
		etalTestPrintReportPassStart(INIT_BOTH_TUNERS_DISABLED, ETAL_TEST_MODE_NONE, "Both tuners disabled, %s", str);
		retinit1 = ETAL_RET_FRONTEND_LIST_ERR;
		retinit2 = ETAL_RET_FRONTEND_LIST_ERR;
	}

#ifdef CONFIG_MODULE_DCOP_HDRADIO_CLOCK_FROM_CMOST
	/* The DCOP receives system clock from Tuner1 or Tuner2
	 * depending on MTD jumpers so ETAL initialization
	 * may fail. To avoid test failure due to HDRadio ping
	 * fail disable the DCOP from configuration */
	attr->m_DCOPAttr.m_isDisabled = TRUE;
#endif

	/* use the embedded FW */
	// this is an MTD configuration 
	// what is supported by default board configuration.
	// MTDv2.1 DAB : Tuner 1 only, T2 only, Tuner 1 + DAB, Tuner 1 + Tuner 2, T1+T2+DAB.   
	// MTDv2.3 DAB : T1 only, T2 only, T1+T2, T2+DAB, T1+T2+DAB (T2 required for DAB)... BUT T1 required for audio..
	// MTDv2.1 HD : T1 only, T2 only, T1+T2,  T1+T2+HD, (HD only not supported because no HD crystal, T1 required for audio...)
	// MTDv2.3 HD : T1 only, T2 only, T1+T2, T1+T2+HD, (HD only not supported because no HD crystal)

// Disable DCOP if tuner 2 disabled
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	if (false == t2_enabled)
	{
		attr->m_DCOPAttr.m_isDisabled = TRUE;
	}
#endif

	attr->m_tunerAttr[0].m_isDisabled = !t1_enabled;
	attr->m_tunerAttr[1].m_isDisabled = !t2_enabled;
	pass = TRUE;
	deinit = FALSE;
	ret = etal_initialize(attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		deinit = TRUE;
		etalTestPrintReportPassEnd(testPassed);
	}

	/* check the device status after init */
	if (etal_get_init_status(&status) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(INIT_CHECK_STATUS_2, ETAL_TEST_MODE_NONE, "Check init status");
	if (etalTestInitCheckStatus(&status, t1_enabled, t2_enabled) != OSAL_OK)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%db FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* try to create a receiver on first tuner */
	pass1 = TRUE;
	if (etalTestInitTuner_ConfigReceiver(0, retinit1, &pass1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(INIT_CREATE_RCV_1ST_TUNER, ETAL_TEST_MODE_NONE, "Create a receiver on first tuner");
	if (!pass1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dc FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* try to create a receiver on second tuner */
	pass1 = TRUE;
	if (etalTestInitTuner_ConfigReceiver(1, retinit2, &pass1) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(INIT_CREATE_RCV_2ND_TUNER, ETAL_TEST_MODE_NONE, "Create a receiver on second tuner");
	if (!pass1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dd FAILED", pass_id);
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (deinit)
	{
		if (etalTestDeinit() != OSAL_OK)
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
 * etalTestInitializeTunerDisabled
 *
 **************************/
static tSInt etalTestInitializeTunerDisabled(tU32 pass_id, tBool *pass)
{
	EtalHardwareAttr attr;

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;

	/* use the embedded FW */
	// this is an MTD configuration 
	// what is supported by default board configuration.
	// MTDv2.1 DAB : Tuner 1 only, T2 only, Tuner 1 + DAB, Tuner 1 + Tuner 2, T1+T2+DAB.   
	// MTDv2.3 DAB : T1 only, T2 only, T1+T2, T2+DAB, T1+T2+DAB (T2 required for DAB)... BUT T1 required for audio..
	// MTDv2.1 HD : T1 only, T2 only, T1+T2,  T1+T2+HD, (HD only not supported because no HD crystal, T1 required for audio...)
	// MTDv2.3 HD : T1 only, T2 only, T1+T2, T1+T2+HD, (HD only not supported because no HD crystal)

	if (etalTestInitializeTunerDisabled_internal(&attr, TRUE, FALSE, pass_id, pass, "embedded FW") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestInitializeTunerDisabled_internal(&attr, FALSE, TRUE, pass_id, pass, "embedded FW") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

// can work only in HD auto clock
//
#if defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO) && !defined(CONFIG_MODULE_DCOP_HDRADIO_CLOCK_FROM_CMOST)
	if (etalTestInitializeTunerDisabled_internal(&attr, FALSE, FALSE, pass_id, pass, "embedded FW") != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	/* use the FW passed as paramater */

	if (etalTestInit_GetFWReferences(&attr) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestInitializeTunerDisabled_internal(&attr, TRUE, FALSE, pass_id, pass, "custom FW") != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestInitializeTunerDisabled_internal(&attr, FALSE, TRUE, pass_id, pass, "custom FW") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

#if defined(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO) && !defined(CONFIG_MODULE_DCOP_HDRADIO_CLOCK_FROM_CMOST)
	if (etalTestInitializeTunerDisabled_internal(&attr, FALSE, FALSE, pass_id, pass, "custom FW") != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
	return OSAL_OK;
}
#endif // CONFIG_MODULE_INTEGRATED

/***************************
 *
 * etalTestInitializeTunerCheck
 *
 **************************/
static tSInt etalTestInitializeTunerCheck(tBool *pass_out)
{
	tBool pass;

	pass = TRUE;

	/* initialize CMOST with a empty image */
	if (etalTestInitializeTunerEmpty(1, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/* initialize CMOST with a wrong image, first tuner */
	if (etalTestInitializeTunerDummy(0, 2, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#if defined (CONFIG_MODULE_INTEGRATED)
	/* initialize CMOST with a wrong image, second tuner */
	if (etalTestInitializeTunerDummy(1, 3, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/* initialize CMOST with a wrong image, both tuners */
	if (etalTestInitializeTunerDummy(ETAL_TEST_INIT_FAIL_ALL, 4, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	/* use valid CMOST image */
	if (etalTestInitializeTunerValid(5, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

#if defined (CONFIG_MODULE_INTEGRATED)
	/* initialize with one tuner disabled */
	if (etalTestInitializeTunerDisabled(6, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	// test addition : step by step tuner initialization
	//
	/* use valid CMOST image */
	if (etalTestInitializeTunerValidStepByStep(7, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_INITIALIZATION_TUNER

#if defined (CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM)
#define ETAL_TEST_INIT_CUSTPARAM_SIZE 2
/***************************
 *
 * etalTestCustomParamPrepare
 *
 **************************/
static tVoid etalTestCustomParamPrepare(tU32 tunerIndex, tU32 *address, tU32 *value, tU16 *size)
{
	tU32 offset;
	ETAL_HANDLE hTuner;
	EtalDeviceType tuner_type;

	offset = 0;

	/* Choose a parameter set, not really knowing what they are
	 * we are only interested in reading back the same value */

	/* Parameters depend on Tuner type; during this test capabilities
	 * are not yet available (we are testing the etal initialization)
	 * so we need to use ETAL internal APIs */

	hTuner = ETAL_handleMakeTuner(tunerIndex);
	tuner_type = ETAL_tunerGetType(hTuner);

	if (tuner_type == deviceUnknown)
	{
	}
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	else if (tuner_type == deviceSTART)
	{
		/* use the same addresses as contained in TDA7707_Cust_settings.h
		 * but with different values */
		address[0] = TDA7707_systemConfig_audioMuteSlope;
		address[1] = TDA7707_amCoef_asp_volume;
	}
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	else if (tuner_type == deviceSTARS)
	{
		address[0] = TDA7708_systemConfig_audioMuteSlope;
		address[1] = TDA7708_amCoef_asp_volume;
	}
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	else if (tuner_type == deviceDOTT)
	{
		address[0] = STA710_agcX_ch0_gainDelay__0__;
		address[1] = STA710_agcX_ch0_gainDelay__1__;
	}
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	else if (tuner_type == deviceDOTS)
	{
		address[0] = CMT_agcX_ch0_gainDelay__0__;
		address[1] = CMT_agcX_ch0_gainDelay__1__;
	}
#endif
	else
	{
		etalTestPrintVerbose("Unprocessed tuner type 0x%x", tuner_type);
	}

	if (tunerIndex == 1)
	{
		/* differentiate values for tuner0 from tuner1 */
		offset = 0x111111;
	}
	value[0] = 0x5A5A5A + offset;
	value[1] = 0xA5A5A5 + offset;
	*size = ETAL_TEST_INIT_CUSTPARAM_SIZE;
}

/***************************
 *
 * etalTestCustomParamCheck
 *
 **************************/
static tSInt etalTestCustomParamCheck(tU32 tunerIndex, tU32 *address, tU32 *value, tU16 size, tBool *pass)
{
	tU32 i;
	tU32 response[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU16 responseLength;

	if (etal_read_parameter(tunerIndex, fromAddress, address, size, response, &responseLength) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	if (responseLength != size)
	{
		*pass = FALSE;
		etalTestPrintVerbose("Read size does not match %d, expected %d", responseLength, size);
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			if (response[i] != value[i])
			{
				*pass = FALSE;
			}
			etalTestPrintVerbose("Read address: 0x%x, exp value: 0x%x, read value: 0x%x", address[i], value[i], response[i]);
		}
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestInit_GetDefaultParams
 *
 **************************/
static tSInt etalTestInit_GetDefaultParams(tU32 *address, tU32 *value, tU32 *size)
{
	BootTunerTy tuner_type;
	tU32 i, index;
	tU8 *paramValueArray; // arbitrary!

	/* This assumes that all tuners in the system are of the same type,
	 * This is not true for old MTD board. It would be better to get the
	 * type from etalTuner
	 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_S;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_S;
#endif
	if (BOOT_getFirmwareReferences_CMOST(tuner_type, NULL, NULL, NULL, &paramValueArray, size) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (*size > 16)
	{
		/* overflow in paramValueArray */
		return OSAL_ERROR;
	}

	for (i = 0, index = 0; i < *size * 2; i += 2)
	{
		address[index] = ((tU32 *)paramValueArray + i)[0];
		value[index] =   ((tU32 *)paramValueArray + i)[1];
		index++;
	}

	return OSAL_OK;
}


/***************************
 *
 * etalTestCustomParamCheck_ValidParam
 *
 **************************/
static tSInt etalTestCustomParamCheck_ValidParam(tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalHardwareAttr attr;
	tU16 size;
	tU32 address0[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 value0[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 paramValueArray0[2 * ETAL_TEST_INIT_CUSTPARAM_SIZE];
#if defined (CONFIG_MODULE_INTEGRATED)
	tBool pass2;
	tU32 address1[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 value1[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 paramValueArray1[2 * ETAL_TEST_INIT_CUSTPARAM_SIZE];
#endif
	tU32 addressD[3]; // size of the array defined in TDA7707_Cust_settings.h
	tU32 valueD[3]; // size of the array defined in TDA7707_Cust_settings.h
	tU32 sizeD;
	tBool pass;
	tBool pass1;

	etalTestCustomParamPrepare(0, address0, value0, &size);
	etal_write_parameter_convert(paramValueArray0, address0, value0, size);
#if defined (CONFIG_MODULE_INTEGRATED)
	etalTestCustomParamPrepare(1, address1, value1, &size);
	etal_write_parameter_convert(paramValueArray1, address1, value1, size);
#endif

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;

	if (etalTestInit_GetDefaultParams(addressD, valueD, &sizeD) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(INIT_NO_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Init with no parameters, first tuner");
	pass = TRUE;
	attr.m_tunerAttr[0].m_useCustomParam = 0xFF; /* any value different from 0 or 1 will do */
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		/* read back using the ETAL api */
		pass1 = TRUE;
		if (etalTestCustomParamCheck(0, addressD, valueD, sizeD, &pass1) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		else
		{
			etalTestPrintReportPassStart(INIT_CHECK_DEFAULT_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Check default parameters, first tuner");
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_S_CUT_BC) && !defined (CONFIG_MODULE_INTEGRATED)
			if (!pass1) /* STAR-S BC is ROM-based and parameters are initialized to the same value as TDA7708_Cust_settings.h */
#else
			if (pass1) /* not a typo, expect that with no parameters loaded the valueD will not match */
#endif
			{
				pass = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass1b FAILED");
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintReportPassStart(INIT_DEFAULT_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Init with default parameter, first tuner");
	attr.m_tunerAttr[0].m_useCustomParam = 0;
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		/* read back using the ETAL api */
		pass1 = TRUE;
		if (etalTestCustomParamCheck(0, addressD, valueD, sizeD, &pass1) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		else
		{
			etalTestPrintReportPassStart(INIT_CHECK_CUSTOM_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Check custom parameters, first tuner");
			if (!pass1)
			{
				pass = FALSE;
				etalTestPrintNormal("pass1d FAILED");
				etalTestPrintReportPassEnd(testFailed);
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintReportPassStart(INIT_CUSTOM_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Init with custom parameter, first tuner");
	attr.m_tunerAttr[0].m_useCustomParam = 1;
	attr.m_tunerAttr[0].m_CustomParamSize = size;
	attr.m_tunerAttr[0].m_CustomParam = paramValueArray0;
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1e FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		/* read back using the ETAL api */
		pass1 = TRUE;
		if (etalTestCustomParamCheck(0, address0, value0, size, &pass1) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
		else
		{
			etalTestPrintReportPassStart(INIT_CHECK_CUSTOM_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Check custom parameters, first tuner");
			if (!pass1)
			{
				pass = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass1f FAILED");
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#if defined (CONFIG_MODULE_INTEGRATED)
	etalTestPrintReportPassStart(INIT_CUSTOM_PARAM_BOTH_TUNERS, ETAL_TEST_MODE_NONE, "Init with custom parameter, both tuners");
	attr.m_tunerAttr[1].m_useCustomParam = 1;
	attr.m_tunerAttr[1].m_CustomParamSize = size;
	attr.m_tunerAttr[1].m_CustomParam = paramValueArray1;
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1g FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		/* read back using the ETAL api */
		pass1 = TRUE;
		pass2 = TRUE;
		if ((etalTestCustomParamCheck(0, address0, value0, size, &pass1) != OSAL_OK) ||
			(etalTestCustomParamCheck(1, address1, value1, size, &pass2) != OSAL_OK))
		{
			return OSAL_ERROR;
		}
		else
		{
			etalTestPrintReportPassStart(INIT_CHECK_CUSTOM_PARAM_BOTH_TUNERS, ETAL_TEST_MODE_NONE, "Check custom parameters, both tuners");
			if (!pass1 || !pass2)
			{
				pass = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass1h FAILED");
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}
		if (etalTestDeinit() != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
#endif
	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestCustomParamCheck_InvalidParam
 *
 **************************/
static tSInt etalTestCustomParamCheck_InvalidParam(tBool *pass_out)
{
	ETAL_STATUS ret;
	EtalHardwareAttr attr;
	EtalInitStatus init_status;
	tU16 size;
	tU32 address0[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 value0[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 paramValueArray0[2 * ETAL_TEST_INIT_CUSTPARAM_SIZE];
#if defined (CONFIG_MODULE_INTEGRATED)
	tU32 address1[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 value1[ETAL_TEST_INIT_CUSTPARAM_SIZE];
	tU32 paramValueArray1[2 * ETAL_TEST_INIT_CUSTPARAM_SIZE];
#endif
	tBool pass;
	tBool pass1;

	etalTestCustomParamPrepare(0, address0, value0, &size);
	etal_write_parameter_convert(paramValueArray0, address0, value0, size);
#if defined (CONFIG_MODULE_INTEGRATED)
	etalTestCustomParamPrepare(1, address1, value1, &size);
	etal_write_parameter_convert(paramValueArray1, address1, value1, size);
#endif

	OSAL_pvMemorySet((tVoid *)&attr, 0x00, sizeof(EtalHardwareAttr));
	attr.m_cbNotify = userNotificationHandler;

	etalTestPrintReportPassStart(INIT_INVALID_CUSTOM_PARAM_1ST_TUNER, ETAL_TEST_MODE_NONE, "Init with invalid custom parameter, first tuner");
	pass = TRUE;
	attr.m_tunerAttr[0].m_useCustomParam = 1;
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintReportPassStart(INIT_CHECK_STATUS_1ST_TUNER, ETAL_TEST_MODE_NONE, "Check init status, first tuner");
		pass1 = TRUE;
		if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass2b FAILED");
		}
		else if (init_status.m_tunerStatus[0].m_deviceStatus != deviceParameters)
		{
			pass1 = FALSE;
			etalTestPrintInitStatus();
			etalTestPrintNormal("pass2c FAILED");
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
	}
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(INIT_ILLEGAL_CUSTOM_PARAM_SIZE_1ST_TUNER, ETAL_TEST_MODE_NONE, "Init with illegal custom parameter size, first tuner");
	attr.m_tunerAttr[0].m_useCustomParam = 1;
	attr.m_tunerAttr[0].m_CustomParamSize = ETAL_DEF_MAX_READWRITE_SIZE + 1;
	attr.m_tunerAttr[0].m_CustomParam = paramValueArray0;
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2d FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintReportPassStart(INIT_CHECK_STATUS_1ST_TUNER_2, ETAL_TEST_MODE_NONE, "Check init status, first tuner");
		pass1 = TRUE;
		if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass2e FAILED");
		}
		else if (init_status.m_tunerStatus[0].m_deviceStatus != deviceParameters)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass2f FAILED");
			etalTestPrintInitStatus();
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
	}
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}

#if defined (CONFIG_MODULE_INTEGRATED)
	etalTestPrintReportPassStart(INIT_ILLEGAL_CUSTOM_PARAM_SIZE_2ND_TUNER, ETAL_TEST_MODE_NONE, "Init with illegal custom parameter size, second tuner");
	attr.m_tunerAttr[0].m_CustomParamSize = ETAL_DEF_MAX_READWRITE_SIZE; /* restore valid value for first tuner */
	attr.m_tunerAttr[1].m_useCustomParam = 1;
	attr.m_tunerAttr[1].m_CustomParamSize = ETAL_DEF_MAX_READWRITE_SIZE + 1;
	attr.m_tunerAttr[1].m_CustomParam = paramValueArray1;
	ret = etal_initialize(&attr);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2g FAILED");
		etalTestPrintInitStatus();
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintReportPassStart(INIT_CHECK_STATUS_BOTH_TUNERS, ETAL_TEST_MODE_NONE, "Check init status, both tuners");
		pass1 = TRUE;
		if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass2h FAILED");
		}
		else if ((init_status.m_tunerStatus[0].m_deviceStatus != deviceAvailable) ||
				(init_status.m_tunerStatus[1].m_deviceStatus != deviceParameters))
		{
			pass1 = FALSE;
			etalTestPrintNormal("pass2i FAILED");
			etalTestPrintInitStatus();
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
	}
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif
	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestInitializeCustomParamCheck
 *
 **************************/
static tSInt etalTestInitializeCustomParamCheck(tBool *pass_out)
{
	tBool pass;

	pass = TRUE;

	if (etalTestCustomParamCheck_ValidParam(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestCustomParamCheck_InvalidParam(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}
#endif

#if defined (CONFIG_APP_TEST_INITIALIZATION_DCOP)
#ifdef CONFIG_ETAL_SUPPORT_DCOP
/***************************
 *
 * etalDcopFlash_GetImageCb
 *
 **************************/
static int etalTestInitialize_GetImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap)
{
	static FILE *fhbs = NULL, *fhpg = NULL;
	static long file_bs_size = 0, file_pg_size = 0;
	FILE **fhgi;
	long file_position = 0, *file_size = NULL;
	size_t retfread;
	int do_get_file_size = 0;

	/* open file bootstrap or program firmware if not already open */
	if (isBootstrap != 0)
	{
		fhgi = &fhbs;
		file_size = &file_bs_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(etaltest_dcopFlash_filename[0], "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, etaltest_dcopFlash_filename[0]);
			return errno;
		}
	}
	else
	{
		fhgi = &fhpg;
		file_size = &file_pg_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(etaltest_dcopFlash_filename[1], "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, etaltest_dcopFlash_filename[1]);
			return errno;
		}
	}

	if (do_get_file_size == 1)
	{
		/* read size of file */
		if (fseek(*fhgi, 0, SEEK_END) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_END) %d\n", errno);
			return errno;
		}
		if ((*file_size = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell end of file %d\n", errno);
			return errno;
		}
		if (fseek(*fhgi, 0, SEEK_SET) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_SET) %d\n", errno);
			return errno;
		}
	}

	/* set remaining bytes number */
	if (remainingByteNum != NULL)
	{
		if ((file_position = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell %d\n", errno);
			return errno;
		}
		*remainingByteNum = (*file_size - file_position);
	}

	/* read requestedByteNum bytes in file */
	retfread = fread(block, 1, requestedByteNum, *fhgi);
	*returnedByteNum = (tU32) retfread;
	if (*returnedByteNum != requestedByteNum)
	{
		if (ferror(*fhgi) != 0)
		{
			/* error reading file */
			if (isBootstrap != 0)
			{
				printf("Error %d reading file %s\n", errno, etaltest_dcopFlash_filename[0]);
			}
			else
			{
				printf("Error %d reading file %s\n", errno, etaltest_dcopFlash_filename[1]);
			}
			clearerr(*fhgi);
			fclose(*fhgi);
			*fhgi = NULL;
			return -1;
		}
	}

	/* Close file if EOF */
	if (feof(*fhgi) != 0)
	{
		/* DCOP bootstrap or flash program successful */
		clearerr(*fhgi);
		fclose(*fhgi);
		*fhgi = NULL;
	}
	return 0;
}
#endif

/***************************
 *
 * etalTestInitializeDCOPCheck
 *
 **************************/
static tSInt etalTestInitializeDCOPCheck(tBool *pass_out)
{
	EtalHardwareAttr hardware_attr;
#ifdef CONFIG_ETAL_SUPPORT_DCOP
	ETAL_STATUS ret;
	ETAL_HANDLE hReceiver;
	EtalReceiverAttr receiver_attr;
	EtalInitStatus init_status;
	tBool pass1;
#endif
	tBool pass;

	OSAL_pvMemorySet((tVoid *)&hardware_attr, 0x00, sizeof(EtalHardwareAttr));
	hardware_attr.m_cbNotify = userNotificationHandler;
	pass = TRUE;

#ifdef CONFIG_ETAL_SUPPORT_DCOP
	/* DCOP present per config, disable and check if not used */
	etalTestPrintReportPassStart(INIT_DCOP_DISABLED, ETAL_TEST_MODE_NONE, "DCOP disabled");
	hardware_attr.m_DCOPAttr.m_isDisabled = TRUE;
	pass1 = TRUE;
	if (etal_initialize(&hardware_attr) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1a FAILED");
	}
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceDisabled)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass1c FAILED");
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

	/* try to configure a DCOP receiver and expect error
	 * need to differentiate between DAB and HDRadio otherwise
	 * we'll get an error but for a different reason */
	OSAL_pvMemorySet(&receiver_attr, 0x00, sizeof(EtalReceiverAttr));

	etalTestPrintReportPassStart(INIT_CREATE_DCOP_RECEIVER_EXPECTED_ERROR, ETAL_TEST_MODE_NONE, "Create DCOP receiver, expect error");
	hReceiver = ETAL_INVALID_HANDLE;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	receiver_attr.m_Standard = ETAL_BCAST_STD_DAB;
	receiver_attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
#elif defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	receiver_attr.m_Standard = ETAL_BCAST_STD_HD_FM; /* doesn't matter AM or FM */
	receiver_attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
#endif
	if (etal_config_receiver(&hReceiver, &receiver_attr) != ETAL_RET_INVALID_BCAST_STANDARD)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	if (etalTestDeinit() != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* same check but with DCOP enabled this time */
	etalTestPrintReportPassStart(INIT_DCOP_ENABLED, ETAL_TEST_MODE_NONE, "DCOP enabled");
	pass1 = TRUE;
	hardware_attr.m_DCOPAttr.m_isDisabled = FALSE;
	if (etal_initialize(&hardware_attr) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
	}
	/* etal_get_init_status should not be called in case of etal_initialize success
	 * let's try anyway */
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass2b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceAvailable)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass2c FAILED");
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

	/* try to configure a DCOP receiver 
	 * need to differentiate between DAB and HDRadio otherwise
	 * we'll get an error */
	OSAL_pvMemorySet(&receiver_attr, 0x00, sizeof(EtalReceiverAttr));

	etalTestPrintReportPassStart(INIT_CREATE_DCOP_RECEIVER, ETAL_TEST_MODE_NONE, "Create DCOP receiver");
	hReceiver = ETAL_INVALID_HANDLE;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	receiver_attr.m_Standard = ETAL_BCAST_STD_DAB;
	receiver_attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
#elif defined CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
	receiver_attr.m_Standard = ETAL_BCAST_STD_HD_FM; /* doesn't matter AM or FM */
	receiver_attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
#endif
	ret = etal_config_receiver(&hReceiver, &receiver_attr);
#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM) || defined (CONFIG_APP_TEST_DAB)
	if (ret != ETAL_RET_SUCCESS)
#else
	if (ret != ETAL_RET_FRONTEND_LIST_ERR)
#endif
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	if (etalTestDeinit() != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* same check but with DCOP enabled and invalid doFlashProgram configuration */
	etalTestPrintReportPassStart(INIT_DCOP_EN_INVALID_DOFLASH_CONFIG, ETAL_TEST_MODE_NONE, "DCOP enabled invalid doFlashProgram configuration");
	pass1 = TRUE;
	hardware_attr.m_DCOPAttr.m_isDisabled = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashProgram = TRUE;
	hardware_attr.m_DCOPAttr.m_cbGetImage = NULL;           /* invalid in data mode */
	hardware_attr.m_DCOPAttr.m_pvGetImageContext = NULL;    /* invalid in file mode */
	hardware_attr.m_DCOPAttr.m_doDownload = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashDump = FALSE;
	hardware_attr.m_DCOPAttr.m_cbPutImage = NULL;
	hardware_attr.m_DCOPAttr.m_pvPutImageContext = NULL;
	if ((ret = etal_initialize(&hardware_attr)) != ETAL_RET_PARAMETER_ERR)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass3a FAILED");
	}
	/* etal_get_init_status should not be called in case of etal_initialize success
	 * let's try anyway */
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass3b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceDownload)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass3c FAILED");
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
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}

	/* same check but with DCOP enabled and invalid doFlashProgram doDownload configuration */
	etalTestPrintReportPassStart(INIT_DCOP_EN_INVALID_DOFLASH_DODL_CONFIG, ETAL_TEST_MODE_NONE, "DCOP enabled invalid doFlashProgram doDownload configuration");
	pass1 = TRUE;
	hardware_attr.m_DCOPAttr.m_isDisabled = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashProgram = TRUE;
	hardware_attr.m_DCOPAttr.m_cbGetImage = etalTestInitialize_GetImageCb;
	hardware_attr.m_DCOPAttr.m_pvGetImageContext = etaltest_dcopFlash_filename;
	hardware_attr.m_DCOPAttr.m_doDownload = TRUE; /* invalid */
	hardware_attr.m_DCOPAttr.m_doFlashDump = FALSE;
	hardware_attr.m_DCOPAttr.m_cbPutImage = NULL;
	hardware_attr.m_DCOPAttr.m_pvPutImageContext = NULL;
	if ((ret = etal_initialize(&hardware_attr)) != ETAL_RET_PARAMETER_ERR)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass4a FAILED");
	}
	/* etal_get_init_status should not be called in case of etal_initialize success
	 * let's try anyway */
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass4b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceDownload)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass4c FAILED");
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
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}

	/* same check but with DCOP enabled and invalid m_doDownload configuration */
	etalTestPrintReportPassStart(INIT_DCOP_EN_INVALID_DODL_CONFIG, ETAL_TEST_MODE_NONE, "DCOP enabled invalid m_doDownload configuration");
	pass1 = TRUE;
	hardware_attr.m_DCOPAttr.m_isDisabled = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashProgram = FALSE;
	hardware_attr.m_DCOPAttr.m_cbGetImage = NULL;           /* invalid in data mode */
	hardware_attr.m_DCOPAttr.m_pvGetImageContext = NULL;    /* invalid in file mode */
	hardware_attr.m_DCOPAttr.m_doDownload = TRUE;
	hardware_attr.m_DCOPAttr.m_doFlashDump = FALSE;
	hardware_attr.m_DCOPAttr.m_cbPutImage = NULL;
	hardware_attr.m_DCOPAttr.m_pvPutImageContext = NULL;
	if ((ret = etal_initialize(&hardware_attr)) != ETAL_RET_PARAMETER_ERR)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass5a FAILED");
	}
	/* etal_get_init_status should not be called in case of etal_initialize success
	 * let's try anyway */
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass5b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceDownload)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass5c FAILED");
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
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}

	/* same check but with DCOP enabled and invalid m_doFlashDump configuration */
	etalTestPrintReportPassStart(INIT_DCOP_EN_INVALID_DODUMP_CONFIG, ETAL_TEST_MODE_NONE, "DCOP enabled invalid m_doFlashDump configuration");
	pass1 = TRUE;
	hardware_attr.m_DCOPAttr.m_isDisabled = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashProgram = FALSE;
	hardware_attr.m_DCOPAttr.m_cbGetImage = etalTestInitialize_GetImageCb;
	hardware_attr.m_DCOPAttr.m_pvGetImageContext = etaltest_dcopFlash_filename;
	hardware_attr.m_DCOPAttr.m_doDownload = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashDump = TRUE;
	hardware_attr.m_DCOPAttr.m_cbPutImage = NULL;           /* invalid in data mode */
	hardware_attr.m_DCOPAttr.m_pvPutImageContext = NULL;    /* invalid in file mode */
	if ((ret = etal_initialize(&hardware_attr)) != ETAL_RET_PARAMETER_ERR)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass6a FAILED");
	}
	/* etal_get_init_status should not be called in case of etal_initialize success
	 * let's try anyway */
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass6b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceDownload)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass6c FAILED");
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
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}

#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
	/* same check but with DCOP enabled and valid m_doDownload configuration */
	/* need to copy files with same filenames than in etaltest_dcopFlash_filename in a valid directory */
	etalTestPrintReportPassStart(INIT_DCOP_EN_VALID_DODL_CONFIG, ETAL_TEST_MODE_NONE, "DCOP enabled valid m_doDownload configuration");
	pass1 = TRUE;
	hardware_attr.m_DCOPAttr.m_isDisabled = FALSE;
	hardware_attr.m_DCOPAttr.m_doFlashProgram = FALSE;
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE
	hardware_attr.m_DCOPAttr.m_cbGetImage = NULL;
	hardware_attr.m_DCOPAttr.m_pvGetImageContext = etaltest_dcopFlash_filename;
#else
	hardware_attr.m_DCOPAttr.m_cbGetImage = etalTestInitialize_GetImageCb;
	hardware_attr.m_DCOPAttr.m_pvGetImageContext = NULL;
#endif /* CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE */
	hardware_attr.m_DCOPAttr.m_doDownload = TRUE;
	hardware_attr.m_DCOPAttr.m_doFlashDump = FALSE;
	hardware_attr.m_DCOPAttr.m_cbPutImage = NULL;           /* invalid in data mode */
	hardware_attr.m_DCOPAttr.m_pvPutImageContext = NULL;    /* invalid in file mode */
	if ((ret = etal_initialize(&hardware_attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass7a FAILED");
	}
	/* etal_get_init_status should not be called in case of etal_initialize success
	 * let's try anyway */
	if (etal_get_init_status(&init_status) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass7b FAILED");
	}
	else if (init_status.m_DCOPStatus.m_deviceStatus != deviceAvailable)
	{
		pass1 = FALSE;
		etalTestPrintNormal("pass7c FAILED");
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
	if ((ret == ETAL_RET_SUCCESS) && (etalTestDeinit() != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
#endif // CONFIG_ETAL_SUPPORT_DCOP

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_INITIALIZATION_DCOP
#endif // CONFIG_APP_TEST_INITIALIZATION


#if defined (CONFIG_APP_TEST_INITIALIZATION) && defined (CONFIG_APP_TEST_XTAL_ALIGNMENT)
/***************************
 *
 * etalTestInitializeXTALalign
 *
 **************************/
static tSInt etalTestInitializeXTALalign(tBool *pass_out)
{
	tSInt ret;
	tU32 calculatedAlignment;
	EtalHardwareAttr init_params;
	tBool pass;

	OSAL_pvMemorySet((tVoid *)&init_params, 0x0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = userNotificationHandler;

	if (etalTestReadXTALalignmentRead(&calculatedAlignment) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(INIT_XTAL_ALIGNMENT, ETAL_TEST_MODE_NONE, "Init with XTAL alignment");
	pass = TRUE;

	init_params.m_tunerAttr[0].m_useXTALalignment = TRUE;
	init_params.m_tunerAttr[0].m_XTALalignment = calculatedAlignment;

	ret = etal_initialize(&init_params);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	if (ret == ETAL_RET_SUCCESS)
	{
		if (etalTestDeinit() != OSAL_OK)
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
#endif // CONFIG_APP_TEST_INITIALIZATION && CONFIG_APP_TEST_XTAL_ALIGNMENT

/***************************
 *
 * etalTestInitialize
 *
 **************************/
/*
 * This function tests the etal_initialize API
 */
tSInt etalTestInitialize(EtalTraceConfig *tr_config)
{
#ifdef CONFIG_APP_TEST_INITIALIZATION
	tBool pass;

	pass = TRUE;

#if defined (CONFIG_APP_TEST_LANDSCAPE_MANAGEMENT)
	/*
	 * Load, clear and save landscape
	 */
	if (etalTestLandscapeManagement(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_INITIALIZATION_PARAMETER)
	/*
	 * call etal_initialize with:
	 * - NULL parameter (expect defaults used)
	 * - zero-filled parameter (expect error)
	 * - m_cbNotify parameter only (expect success)
	 */
	if (etalTestInitializeParameterCheck(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_INITIALIZATION_TUNER)
	/*
	 * call etal_initialize with FW image for CMOST:
	 * - empty buffer (expect embedded image to be used)
	 * - wrong buffer contents (buffer contains only 'end of file' marker)
	 * - valid buffer (taken from the embedded image)
	 *
	 * call etal_initialize with CMOST enabled or disabled
	 * and verify if the config_receiver succeds. Only run
	 * for CONFIG_MODULE_INTEGRATED
	 */
	if (etalTestInitializeTunerCheck(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM)
	/*
	 * call etal_initialize with custom parametes for the CMOST:
	 * - empty array
	 * - valid array, read back to check
	 * - wrong array size (larger than max)
	 */
	if (etalTestInitializeCustomParamCheck(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_INITIALIZATION_DCOP)
	/*
	 * - call etal_initialize with DCOP disabled, check if
	 *   config_receiver fails
	 * - call etal_initialize with DCOP enabled, check if
	 *   config_receiver succeds
	 */
	if (etalTestInitializeDCOPCheck(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#ifdef CONFIG_APP_TEST_XTAL_ALIGNMENT
	/*
	 * - call etal_initialize with XTAL alignment
	 *   checks only the function return value, not
	 *   the functionality
	 */
	if (etalTestInitializeXTALalign(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_INITIALIZATION
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

