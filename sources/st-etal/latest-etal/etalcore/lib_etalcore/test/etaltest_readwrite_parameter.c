//!
//!  \file 		etaltest_readwrite_parameter.c
//!  \brief 	<i><b> ETAL test, read / write paramters </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"
#include "tunerdriver.h"
#include "boot_cmost.h"

#ifdef CONFIG_APP_TEST_READ_WRITE_PARAMETER
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

/***************************
 * Local types
 **************************/

#ifdef CONFIG_APP_TEST_READ_WRITE_PARAMETER
typedef enum {
	READ_PARAM_READ = 1,
	READ_PARAM_READ_AGAIN,
	READ_PARAM_COMPARE,
	READ_PARAM_ILLEGAL_LENGTH,
	READ_PARAM_ILLEGAL_ADDRESS,
	READ_PARAM_ILLEGAL_RESPONSE,
	READ_PARAM_ILLEGAL_RESPONSE_LENGTH,
	READ_PARAM_ILLEGAL_MODE,
	READ_PARAM_ILLEGAL_INDEX
} ReadParameterTestTy;

typedef enum {
	WRITE_PARAM_READ_REF = 1,
	WRITE_PARAM_WRITE,
	WRITE_PARAM_RE_READ,
	WRITE_PARAM_COMPARE,
	WRITE_PARAM_ILLEGAL_LENGTH,
	WRITE_PARAM_ILLEGAL_ADDRESS_VALUE,
	WRITE_PARAM_ILLEGAL_MODE,
	WRITE_PARAM_ILLEGAL_INDEX
} WriteParameterTestTy;
#endif

/***************************
 * Local variables
 **************************/

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
tU32 TDA7707_req_address[] = {
	TDA7707_tunApp0_fm_qd_fstRF,
	TDA7707_tunApp0_fm_wsp_smLevShp,
	TDA7707_tunApp0_fm_wsp_smMpShp,
	TDA7707_tunApp0_fm_wsp_smDistShp,
	TDA7707_tunApp0_fm_wsp_sm,
	TDA7707_tunApp0_fm_wsp_sbLevShp,
	TDA7707_tunApp0_fm_wsp_sbDistShp,
	TDA7707_tunApp0_fm_wsp_hb,
	TDA7707_tunApp0_fm_wsp_smDistProc,
	TDA7707_tunApp0_fm_wsp_sbDistProc,
	TDA7707_tunApp0_fm_wsp_hcDistProc,
	TDA7707_tunApp0_fm_wsp_hbDistProc,
	TDA7707_tunApp1_fm_qd_fstRF, /* see comment in etalTestWriteParamInternal */
	TDA7707_tunApp0_am_qd_fstRF,
	TDA7707_tunApp0_am_qd_adj,
	TDA7707_tunApp0_am_wsp_hc
};
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
tU32 TDA7708_req_address[] = {
	TDA7708_tunApp0_fm_qd_fstRF,
	TDA7708_tunApp0_fm_wsp_smLevShp,
	TDA7708_tunApp0_fm_wsp_smMpShp,
	TDA7708_tunApp0_fm_wsp_smDistShp,
	TDA7708_tunApp0_fm_wsp_sm,
	TDA7708_tunApp0_fm_wsp_sbLevShp,
	TDA7708_tunApp0_fm_wsp_sbDistShp,
	TDA7708_tunApp0_fm_wsp_hb,
	TDA7708_tunApp0_fm_wsp_smDistProc,
	TDA7708_tunApp0_fm_wsp_sbDistProc,
	TDA7708_tunApp0_fm_wsp_hcDistProc,
	TDA7708_tunApp0_fm_wsp_hbDistProc,
	TDA7708_tunApp0_am_qd_fstRF,
	TDA7708_tunApp0_am_qd_adj,
	TDA7708_tunApp0_am_wsp_hc
};
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
tU32 STA710_req_address[] = {
// TODO support for DOT-T devices
};
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
tU32 STA709_req_address[] = {
// TODO support for DOT-S devices
};
#endif

// TODO support multiple CMOST devices concurrently
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
/* WARNING: max size limited to ETAL_DEF_MAX_READWRITE_SIZE or the test will fail
 * with ETAL_RET_PARAMETER_ERR */
tU32 req_index[] = {
	IDX_CMT_tunApp0_fm_qdR_fstRF,
	IDX_CMT_tunApp0_fm_qdR_fstBB,
	IDX_CMT_tunApp0_fm_qdR_detune,
	IDX_CMT_tunApp0_fm_wsp_smLevShp,
	IDX_CMT_tunApp0_fm_wsp_smDistShp,
	IDX_CMT_tunApp0_fm_wsp_sm,
	IDX_CMT_tunApp0_fm_wsp_sbLevShp,
	IDX_CMT_tunApp0_fm_wsp_hbDistShp,
	IDX_CMT_tunApp0_fm_wsp_hb,
	IDX_CMT_tunApp0_fm_wsp_smDistProc,
	IDX_CMT_tunApp0_fm_wsp_sbDistProc,
	IDX_CMT_tunApp0_fm_wsp_hcDistProc,
	IDX_CMT_tunApp0_fm_wsp_hbDistProc,
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	/* see comment in etalTestWriteParamInternal */
	IDX_CMT_tunApp1_fm_qd_fstRF,
	IDX_CMT_tunApp1_fm_qd_fstBB,
	IDX_CMT_tunApp1_fm_qd_detune,
#endif
	IDX_CMT_tunApp0_am_qd_fstRF,
	IDX_CMT_tunApp0_am_qd_fstBB,
	IDX_CMT_tunApp0_am_qd_detune,
	IDX_CMT_tunApp0_am_qd_adj,
	IDX_CMT_tunApp0_am_qd_modulation,
	IDX_CMT_tunApp0_am_wsp_hc
};
#else
// TODO support for DOT devices
tU32 req_index[] = {
};
#endif

/***************************
 *
 * etalTestSetArray
 *
 **************************/
static tSInt etalTestSetArray (BootTunerTy tuner_type, tU32 **req_address, tU32 *req_len)
{
	tSInt ret = OSAL_ERROR;

	switch (tuner_type)
	{
		case BOOT_TUNER_STAR_T:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			*req_address = TDA7707_req_address;
			*req_len = sizeof(TDA7707_req_address) / sizeof(tU32);
			ret = OSAL_OK;
#endif
			break;

		case BOOT_TUNER_STAR_S:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
			*req_address = TDA7708_req_address;
			*req_len = sizeof(TDA7708_req_address) / sizeof(tU32);
			ret = OSAL_OK;
#endif
			break;

		case BOOT_TUNER_DOT_T:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			*req_address = STA710_req_address;
			*req_len = sizeof(STA710_req_address) / sizeof(tU32);
			ret = OSAL_OK;
#endif
			break;

		case BOOT_TUNER_DOT_S:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
			*req_address = STA709_req_address;
			*req_len = sizeof(STA709_req_address) / sizeof(tU32);
			ret = OSAL_OK;
#endif
			break;

		default:
			break;
	}

	return ret;
}


/***************************
 *
 * etalTestFakeConfig
 *
 **************************/
/*
 * Some of the memory locations used by the read/write parameter
 * test are not initialized until a change band command is issued
 * to the CMOST; depending on the tests selection, this may
 * have been done or not, so this function ensures that
 * it is done here.
 */
static tSInt etalTestFakeConfig(tVoid)
{
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestWriteParamInternal
 *
 **************************/
static tSInt etalTestWriteParamInternal(tU32 tunerIndex, BootTunerTy tuner_type, etalReadWriteModeTy mode, tBool *pass_out, tU32 pass_num, tChar *str)
{
	ETAL_STATUS ret;
	tU32 i;
	tU16 resp_len;
	tU32 req_value[ETAL_DEF_MAX_READWRITE_SIZE] = {0x000000, 0x111111, 0x222222, 0x333333, 0x444444, 0x555555,
			              0x666666, 0x777777, 0x888888, 0x999999, 0xAAAAAA, 0xBBBBBB,
	                      0xCCCCCC, 0xDDDDDD, 0xEEEEEE, 0xFFFFFF};
	tU32 *req_address;
	tU32 paramValueArray[ETAL_DEF_MAX_READWRITE_SIZE * 2];
	tU32 req_len;
	tU32 resp_value[ETAL_DEF_MAX_READWRITE_SIZE];
	tBool pass;
	tBool pass1;

	ret = ETAL_RET_SUCCESS;

	if (mode == fromAddress)
	{
		if (etalTestSetArray(tuner_type, &req_address, &req_len) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		req_address = req_index;
		req_len = sizeof(req_index) / sizeof (tU32);
	}

	/*
	 * read parameters into resp_value;
	 * this is used only for visual compare of the change before
	 * and after the test
	 */
	etalTestPrintReportPassStart(WRITE_PARAM_READ_REF, ETAL_TEST_MODE_NONE, "Read reference parameter %s", str);
	pass = TRUE;
	OSAL_pvMemorySet((tVoid *)resp_value, 0x00, sizeof(resp_value));
	ret = etal_read_parameter(tunerIndex, mode, req_address, req_len, resp_value, &resp_len);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_num);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		for (i = 0; i < resp_len; i++)
		{
			etalTestPrintVerbose("Read address: 0x%x, value: 0x%x", req_address[i], resp_value[i]);
		}
	}

	etalTestPrintVerbose("Values to be written %s", str);
	for (i = 0; i < req_len; i++)
	{
		etalTestPrintVerbose("Write address: 0x%x, value: 0x%x", req_address[i], req_value[i]);
	}

	/*
	 * write parameters
	 * Use a predefined array of values, req_value, and write them into the same
	 * locations used in the previous step to read references, req_address
	 */
	etalTestPrintReportPassStart(WRITE_PARAM_WRITE, ETAL_TEST_MODE_NONE, "Write parameters %s", str);
	OSAL_pvMemorySet((tVoid *)paramValueArray, 0x00, sizeof(paramValueArray));
	etal_write_parameter_convert(paramValueArray, req_address, req_value, req_len);
	ret = etal_write_parameter(tunerIndex, mode, paramValueArray, req_len);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%db FAILED %d", pass_num, ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	 * read back the location just written
	 *
	 * initialize the resp_value array with a value different
	 * from the one used to initialize paramValueArray
	 */
	etalTestPrintReportPassStart(WRITE_PARAM_RE_READ, ETAL_TEST_MODE_NONE, "Re-read parameter %s", str);
	OSAL_pvMemorySet((tVoid *)resp_value, 0xFF, sizeof(resp_value));
	ret = etal_read_parameter(tunerIndex, mode, req_address, req_len, resp_value, &resp_len);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dc FAILED", pass_num);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		for (i = 0; i < resp_len; i++)
		{
			etalTestPrintVerbose("Read address: 0x%x, value: 0x%x", req_address[i], resp_value[i]);
		}
	}

	/*
	 * compare the values just read with the values written
	 */
	etalTestPrintReportPassStart(WRITE_PARAM_COMPARE, ETAL_TEST_MODE_NONE, "Compare parameters");
	pass1 = TRUE;
	for (i = 0; i < resp_len; i++)
	{
		if (resp_value[i] != req_value[i])
		{
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			/*
			 * If this test is run after the etalTestConfigReceiver the
			 * comparison below fails for all the addresses or indexes
			 * relative to the App1, because it was not possible
			 * to find a way to set the background to idle after
			 * the tests that configure/reconfigure a VPA receiver.
			 * To avoid failing in that case, we manually exclude the
			 * App1 values from the comparison.
			 *
			 * Everything is fine (the exception below is not required)
			 * if the test is run just after reset/firmware load.
			 */
			if (mode == fromAddress)
			{
				if (req_address[i] == TDA7707_tunApp1_fm_qd_fstRF)
				{
					/* don't consider App1 addresses for the comparison */
					continue;
				}
			}
			else
			{
				/* don't consider App1 indexes for the comparison */
				if ((req_address[i] == IDX_CMT_tunApp1_fm_qd_fstRF) ||
					(req_address[i] == IDX_CMT_tunApp1_fm_qd_fstBB) ||
					(req_address[i] == IDX_CMT_tunApp1_fm_qd_detune))
				{
					continue;
				}
			}
#endif
			pass1 = FALSE;
		}
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

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestReadParamInternal
 *
 **************************/
static tSInt etalTestReadParamInternal(tU32 tunerIndex, BootTunerTy tuner_type, etalReadWriteModeTy mode, tBool *pass_out, tU32 pass_num, tChar *str)
{
	ETAL_STATUS ret;
	tU32 i;
	tU16 resp_len_before;
	tU16 resp_len;
	tU32 *req_address;
	tU32 req_len;
	tU32 resp_value_before[ETAL_DEF_MAX_READWRITE_SIZE];
	tU32 resp_value[ETAL_DEF_MAX_READWRITE_SIZE];
	tBool pass;
	tBool pass1;

	if (mode == fromAddress)
	{
		if (etalTestSetArray(tuner_type, &req_address, &req_len) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	else
	{
		req_address = req_index;
		req_len = sizeof(req_index) / sizeof (tU32);
	}

	etalTestPrintReportPassStart(READ_PARAM_READ, ETAL_TEST_MODE_NONE, "Read parameter %s", str);
	pass = TRUE;
	OSAL_pvMemorySet((tVoid *)resp_value_before, 0x00, sizeof(resp_value_before));
	ret = etal_read_parameter(tunerIndex, mode, req_address, req_len, resp_value_before, &resp_len_before);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_num);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		for (i = 0; i < resp_len_before; i++)
		{
			etalTestPrintVerbose("Read address: 0x%x, value: 0x%x", req_address[i], resp_value_before[i]);
		}
	}

	/*
	 * read again and compare, the values should be equal if the CMOST is idle
	 *
	 * initialize the resp_value to a vaule different than the one used in the
	 * previous step to ensure it is actually written by etal_read_parameter
	 */
	etalTestPrintReportPassStart(READ_PARAM_READ_AGAIN, ETAL_TEST_MODE_NONE, "Read parameter %s, again", str);
	OSAL_pvMemorySet((tVoid *)resp_value, 0xFF, sizeof(resp_value));
	ret = etal_read_parameter(tunerIndex, mode, req_address, req_len, resp_value, &resp_len);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%db FAILED", pass_num);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		for (i = 0; i < resp_len; i++)
		{
			etalTestPrintVerbose("Read address: 0x%x, value: 0x%x", req_address[i], resp_value[i]);
		}
	}

	etalTestPrintReportPassStart(READ_PARAM_COMPARE, ETAL_TEST_MODE_NONE, "Compare parameters");
	pass1 = FALSE; /* not a typo */
	if (resp_len_before == resp_len)
	{
		pass1 = TRUE;
		for (i = 0; i < resp_len; i++)
		{
			if (resp_value_before[i] != resp_value[i])
			{
				pass1 = FALSE;
			}
		}
	}
	if (!pass1)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%dd FAILED", pass_num);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

#endif // CONFIG_APP_TEST_READ_WRITE_PARAMETER


/***************************
 *
 * etalTestWriteParameter
 *
 **************************/
tSInt etalTestWriteParameter(void)
{
#ifdef CONFIG_APP_TEST_READ_WRITE_PARAMETER
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	BootTunerTy tuner_type;
	tU32 tunerIndex;
	tU32 paramValueArray[ETAL_DEF_MAX_READWRITE_SIZE * 2];
	tBool pass;

	// TODO modify test to support multiple CMOST devices
	// e.g. loop over etalTuner and get the device type of enabled devices
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_S;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_S;
#else
	#error "Unsupported CMOST configuration in CONFIG_APP_TEST_READ_WRITE_PARAMETER"
#endif

	etalTestStartup();
	pass = TRUE;

	/*
	 * This test relies on the CMOST being in idle mode,
	 * otherwise the fromIndex locations will read values
	 * written by the CMOST's DSP, not the values
	 * written by the test and fail.
	 * The function below destroys the receiver
	 * to ensure it is put in idle mode
	 */
	if (etalTestFakeConfig() != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	tunerIndex = 0;

	if (etalTestWriteParamInternal(tunerIndex, tuner_type, fromAddress, &pass, 1, "fromAddress") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestWriteParamInternal(tunerIndex, tuner_type, fromIndex, &pass, 2, "fromIndex") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Parameter sanity check");
	OSAL_pvMemorySet((tVoid *)paramValueArray, 0x00, sizeof(paramValueArray));

	etalTestPrintReportPassStart(WRITE_PARAM_ILLEGAL_LENGTH, ETAL_TEST_MODE_NONE, "Illegal length parameter");
	ret = etal_write_parameter(tunerIndex, fromAddress, paramValueArray, ETAL_DEF_MAX_READWRITE_SIZE + 1);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(WRITE_PARAM_ILLEGAL_ADDRESS_VALUE, ETAL_TEST_MODE_NONE, "Illegal address/value parameter");
	ret = etal_write_parameter(tunerIndex, fromAddress, NULL, ETAL_DEF_MAX_READWRITE_SIZE);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(WRITE_PARAM_ILLEGAL_MODE, ETAL_TEST_MODE_NONE, "Illegal mode parameter");
	ret = etal_write_parameter(tunerIndex, 2, paramValueArray, ETAL_DEF_MAX_READWRITE_SIZE);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(WRITE_PARAM_ILLEGAL_INDEX, ETAL_TEST_MODE_NONE, "Illegal index value");
	paramValueArray[0] = ETAL_IDX_CMT_MAX_EXTERNAL;
	ret = etal_write_parameter(tunerIndex, fromIndex, paramValueArray, ETAL_DEF_MAX_READWRITE_SIZE);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_READ_WRITE_PARAMETER

	return OSAL_OK;
}

/***************************
 *
 * etalTestReadParameter
 *
 **************************/
tSInt etalTestReadParameter(void)
{
#ifdef CONFIG_APP_TEST_READ_WRITE_PARAMETER
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU16 resp_len;
	tU32 resp_value_address[ETAL_DEF_MAX_READWRITE_SIZE];
	BootTunerTy tuner_type;
	tU32 tunerIndex;
	tU32 req_index_bak;
	tBool pass;

	/*
	 * TODO modify test to support multiple CMOST devices
	 * e.g. loop over etalTuner and get the device type of enabled devices
	 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_STAR_S;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_T;
#elif defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	tuner_type = BOOT_TUNER_DOT_S;
#else
	#error "Unsupported CMOST configuration in CONFIG_APP_TEST_READ_WRITE_PARAMETER"
#endif

	etalTestStartup();
	pass = TRUE;

	/*
	 * This test relies on the CMOST being in idle mode,
	 * otherwise the fromIndex locations will read values
	 * written by the CMOST's DSP, not the values
	 * written by the test and fail.
	 * The function below destroys the receiver
	 * to ensure it is put in idle mode
	 */
	if (etalTestFakeConfig() != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	tunerIndex = 0;

	if (etalTestReadParamInternal(tunerIndex, tuner_type, fromAddress, &pass, 1, "fromAddress") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (etalTestReadParamInternal(tunerIndex, tuner_type, fromIndex, &pass, 2, "fromIndex") != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("Parameter sanity check");

	etalTestPrintReportPassStart(READ_PARAM_ILLEGAL_LENGTH, ETAL_TEST_MODE_NONE, "Illegal length parameter");
	ret = etal_read_parameter(tunerIndex, fromIndex, req_index, ETAL_DEF_MAX_READWRITE_SIZE + 1, resp_value_address, &resp_len);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(READ_PARAM_ILLEGAL_ADDRESS, ETAL_TEST_MODE_NONE, "Illegal address parameter");
	ret = etal_read_parameter(tunerIndex, fromIndex, NULL, ETAL_DEF_MAX_READWRITE_SIZE, resp_value_address, &resp_len);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(READ_PARAM_ILLEGAL_RESPONSE, ETAL_TEST_MODE_NONE, "Illegal response parameter");
	ret = etal_read_parameter(tunerIndex, fromIndex, req_index, ETAL_DEF_MAX_READWRITE_SIZE, NULL, &resp_len);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(READ_PARAM_ILLEGAL_RESPONSE_LENGTH, ETAL_TEST_MODE_NONE, "Illegal responseLength parameter");
	ret = etal_read_parameter(tunerIndex, fromIndex, req_index, ETAL_DEF_MAX_READWRITE_SIZE, resp_value_address, NULL);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(READ_PARAM_ILLEGAL_MODE, ETAL_TEST_MODE_NONE, "Illegal mode parameter");
	ret = etal_read_parameter(tunerIndex, 2, req_index, ETAL_DEF_MAX_READWRITE_SIZE, resp_value_address, &resp_len);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3e FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(READ_PARAM_ILLEGAL_INDEX, ETAL_TEST_MODE_NONE, "Illegal index value");
	/* overwrite the first index with illegal value, but save
	 * the good one because the same array will be reused in the
	 * write test
	 */
	req_index_bak = req_index[0];
	req_index[0] = ETAL_IDX_CMT_MAX_EXTERNAL;
	ret = etal_read_parameter(tunerIndex, fromIndex, req_index, ETAL_DEF_MAX_READWRITE_SIZE, resp_value_address, &resp_len);
	if (ret != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3f FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	req_index[0] = req_index_bak;

	if (!pass)
	{
		return OSAL_ERROR;
	}

#endif // CONFIG_APP_TEST_READ_WRITE_PARAMETER

	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

