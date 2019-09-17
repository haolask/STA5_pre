//!
//!  \file 		etaltest_config.c
//!  \brief 	<i><b> ETAL test, receiver configuration </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST

#include "etal_api.h"
#include "etaltest.h"

#define DESTROY_RECEIVER_CB_MIN 1
#define DESTROY_RECEIVER_CB_MAX 5

#ifdef CONFIG_APP_TEST_CONFIG_RECEIVER
typedef enum {
	CFG_RCV_DAB_CREATE = 1,
	CFG_RCV_DAB_MODIFY,
	CFG_RCV_DAB_CREATE_WRONG_STD_FE,
	CFG_RCV_FM_CREATE,
	CFG_RCV_FM_MODIFY,
	CFG_RCV_FM_CREATE_WRONG_STD_FE,
	CFG_RCV_FM_CREATE_VPA,
	CFG_RCV_FM_RECONF_VPA_OFF,
	CFG_RCV_FM_RECONF_VPA_ON,
	CFG_RCV_HDFM_CREATE,
	CFG_RCV_HDFM_MODIFY,
	CFG_RCV_HDFM_RECREATE,
	CFG_RCV_HDAM_CREATE,
	CFG_RCV_HDAM_MODIFY,
	CFG_RCV_HDAM_RECREATE,
	CFG_RCV_AM_CREATE,
	CFG_RCV_AM_MODIFY,
	CFG_RCV_AM_CREATE_FE2,
	CFG_RCV_DAB_CREATE_AGAIN,
    CFG_RCV_FM_CREATE_BUSY_DAB_FE,
    CFG_RCV_DAB_RECONF
} ConfigReceiverTestTy;
#endif

#ifdef CONFIG_APP_TEST_DESTROY_RECEIVER
typedef enum {
	DESTROY_RCV_CHECK_MON_RCV = 1,
    DESTROY_RCV_CHECK_MON_DESTROYED_RCV
} DestroyReceiverTestTy;
#endif

#ifdef CONFIG_APP_TEST_GET_VERSION
typedef enum {
	GET_VERS_CHECK_RETURN = 1,
	GET_VERS_CHECK_ETAL_VERS_SANITY,
	GET_VERS_CHECK_CMOST_VERS_SANITY,
	GET_VERS_CHECK_CMOST_VERS_SANITY_2ND_TUNER,
	GET_VERS_CHECK_DAB_DCOP_VERS_SANITY,
	GET_VERS_CHECK_HD_DCOP_VERS_SANITY
} GetVersionTestTy;
#endif

/***************************
 *
 * etalTestResetEventCount
 *
 **************************/
tVoid etalTestResetEventCount(etalTestEventCountTy *count)
{
	OSAL_pvMemorySet((tPVoid) count, 0x00, sizeof(etalTestEventCountTy));
}


/***************************
 *
 * etalTestCapabilities
 *
 **************************/
tSInt etalTestCapabilities(void)
{
#ifdef CONFIG_APP_TEST_CAPABILITIES
	EtalHwCapabilities *cap;

	if (etal_get_capabilities(&cap) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}
	etalTestPrintCapabilites(cap);



#endif // CONFIG_APP_TEST_CAPABILITIES
	return OSAL_OK;
}

/***************************
 *
 * etalTestConfigReceiver
 *
 **************************/
tSInt etalTestConfigReceiver(void)
{
#ifdef CONFIG_APP_TEST_CONFIG_RECEIVER
	EtalReceiverAttr attr;
	ETAL_HANDLE hReceiver;
#if defined (CONFIG_APP_TEST_DAB) && defined (CONFIG_APP_TEST_FM)
	ETAL_HANDLE hReceiver2;
#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
	EtalBcastQualityMonitorAttr mondab;
	ETAL_HANDLE monitor_dab_handle = ETAL_INVALID_HANDLE;
#endif
#endif
	ETAL_STATUS ret;
	tBool pass;
#if defined (CONFIG_APP_TEST_AM)
	ETAL_STATUS expected_ret;
#endif

	etalTestStartup();
	pass = TRUE;

	// 1st part : test basics config / destroy
	//
#if defined (CONFIG_APP_TEST_DAB)
	OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));

	/* Create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_DAB_CREATE, ETAL_TEST_MODE_DAB, "Create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

#if defined (CONFIG_APP_TEST_FM)
	/* Modify a valid receiver */

	etalTestPrintReportPassStart(CFG_RCV_DAB_MODIFY, ETAL_TEST_MODE_DAB, "Modify receiver");
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
#endif // CONFIG_APP_TEST_FM

	/* Destroy a valid receiver */

	etalTestPrintNormal("* Destroy valid receiver");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Create invalid receiver */

	etalTestPrintReportPassStart(CFG_RCV_DAB_CREATE_WRONG_STD_FE, ETAL_TEST_MODE_DAB, "Create receiver with wrong standard/frontend combination");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	attr.m_FrontEnds[0] = ETAL_FE_INVALID_FOR_DAB_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_FRONTEND_LIST_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
#endif // CONFIG_APP_TEST_DAB

#if defined (CONFIG_APP_TEST_FM)
	OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));

	/* Create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_FM_CREATE, ETAL_TEST_MODE_FM, "Create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Modify a valid receiver */
#if defined (CONFIG_APP_TEST_DAB)
	etalTestPrintReportPassStart(CFG_RCV_FM_MODIFY, ETAL_TEST_MODE_FM, "Modify FM receiver");
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;

	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2b failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
#endif // CONFIG_APP_TEST_DAB

	/* Destroy a valid receiver */

	etalTestPrintNormal("* Destroy FM receiver");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	hReceiver = ETAL_INVALID_HANDLE;

	/* Create invalid receiver */

	etalTestPrintReportPassStart(CFG_RCV_FM_CREATE_WRONG_STD_FE, ETAL_TEST_MODE_FM, "Create receiver with wrong standard/frontend combination");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_AM; // the background channel of CMOST does not support AM
	attr.m_FrontEnds[0] = ETAL_INVALID_HANDLE;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_FRONTEND_LIST_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2c failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (hReceiver != ETAL_INVALID_HANDLE)
	{
		/* Destroy a valid receiver */
		etalTestPrintNormal("* Destroy valid FM receiver");
		if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}

	// VPA PART
	if (ETAL_FE_FOR_FM_TEST2 != ETAL_INVALID_HANDLE)
	{
		/* Create a receiver with VPA ON */
		etalTestPrintReportPassStart(CFG_RCV_FM_CREATE_VPA, ETAL_TEST_MODE_FM, "Create receiver with VPA ON");
		hReceiver = ETAL_INVALID_HANDLE;
		attr.m_Standard = ETAL_BCAST_STD_FM;
		attr.m_FrontEndsSize = 2;
		attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
		attr.m_FrontEnds[1] = ETAL_FE_FOR_FM_TEST2;
		attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_FM_VPA;
		if ((ret = etal_config_receiver(&hReceiver, &attr)) != 
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			ETAL_RET_SUCCESS
#else
			ETAL_RET_FRONTEND_LIST_ERR
#endif
			)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2d failed: etal_config_receiver VPA ON (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Reconfigure a receiver with VPA OFF */

		etalTestPrintReportPassStart(CFG_RCV_FM_RECONF_VPA_OFF, ETAL_TEST_MODE_FM, "Reconfigure receiver with VPA OFF");
		attr.m_Standard = ETAL_BCAST_STD_FM;
		attr.m_FrontEndsSize = 1;
		attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
		attr.m_FrontEnds[1] = ETAL_INVALID_HANDLE;
		attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2e failed: etal_config_receiver reconfiguration VPA OFF (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Reconfigure a receiver with VPA ON */

		etalTestPrintReportPassStart(CFG_RCV_FM_RECONF_VPA_ON, ETAL_TEST_MODE_FM, "Reconfigure receiver with VPA ON");
		attr.m_Standard = ETAL_BCAST_STD_FM;
		attr.m_FrontEndsSize = 2;
		attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
		attr.m_FrontEnds[1] = ETAL_FE_FOR_FM_TEST2;
		attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_FM_VPA;
		if ((ret = etal_config_receiver(&hReceiver, &attr)) != 
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			ETAL_RET_SUCCESS
#else
			ETAL_RET_FRONTEND_LIST_ERR
#endif
			)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2f failed: etal_config_receiver reconfiguration VPA ON (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* Reconfigure a receiver with VPA OFF */

		etalTestPrintReportPassStart(CFG_RCV_FM_RECONF_VPA_OFF, ETAL_TEST_MODE_FM, "Reconfigure receiver with VPA OFF");
		attr.m_Standard = ETAL_BCAST_STD_FM;
		attr.m_FrontEndsSize = 1;
		attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
		attr.m_FrontEnds[1] = ETAL_INVALID_HANDLE;
		attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2g failed: etal_config_receiver reconfiguration VPA OFF (%s, %d)", ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));

		/* Destroy a valid receiver */

		etalTestPrintNormal("* Destroy FM receiver");
		if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
#endif // CONFIG_APP_TEST_FM

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));

	/* Create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_HDFM_CREATE, ETAL_TEST_MODE_HD_FM, "Create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3a failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Modify a valid receiver */

	etalTestPrintReportPassStart(CFG_RCV_HDFM_MODIFY, ETAL_TEST_MODE_HD_FM, "Modify receiver");
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
	/*
	 * DAB and HD mutually exclusive, error expected
	 * the reconfiguration destroys the receiver in case of error
	 */
	if ((ret = etal_config_receiver(&hReceiver, &attr)) == ETAL_RET_SUCCESS) // valid on INDEPENDENT module, TODO on other
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3b failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Re-create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_HDFM_RECREATE, ETAL_TEST_MODE_HD_FM, "Re-create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_config_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy a valid receiver */

	etalTestPrintNormal("* Destroy HDRADIO receiver");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#if defined (CONFIG_APP_TEST_HDRADIO_AM)
	OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));

	/* Create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_HDAM_CREATE, ETAL_TEST_MODE_HD_AM, "Create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_HD_AM;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3a failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Modify a valid receiver */

	etalTestPrintReportPassStart(CFG_RCV_HDAM_MODIFY, ETAL_TEST_MODE_HD_AM, "Modify receiver");
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
	/*
	 * DAB and HD mutually exclusive, error expected
	 * the reconfiguration destroys the receiver in case of error
	 */
	if ((ret = etal_config_receiver(&hReceiver, &attr)) == ETAL_RET_SUCCESS) // valid on INDEPENDENT module, TODO on other
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass3b failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Re-create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_HDAM_RECREATE, ETAL_TEST_MODE_HD_AM, "Re-create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_HD_AM;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_config_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy a valid receiver */

	etalTestPrintNormal("* Destroy HDRADIO AM receiver");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_AM


#if defined (CONFIG_APP_TEST_AM)
	OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));

	/* Create a receiver */

	etalTestPrintReportPassStart(CFG_RCV_AM_CREATE, ETAL_TEST_MODE_AM, "Create receiver");
	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_AM;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_AM_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass4a failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Modify a valid receiver */

	etalTestPrintReportPassStart(CFG_RCV_AM_MODIFY, ETAL_TEST_MODE_AM, "Modify receiver");
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_AM_TEST;

	// check the result : if FE supported for DAB
	// i.e. if 
#ifdef	CONFIG_MODULE_INTEGRATED
	// module integrated, allow FE are 3 or 4
		expected_ret = ETAL_RET_FRONTEND_LIST_ERR;
#else
		expected_ret = ETAL_RET_SUCCESS;
#endif
	
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != expected_ret)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass4b failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (expected_ret != ETAL_RET_SUCCESS)
	{
		// an error occured, so we assume receiver is now invalid
		/* if no DCOP is present the previous pass resulted in receiver being
		 * destroyed, so this step will rightfully fail, which is confusing.
		 * Better compile it out */
		hReceiver = ETAL_INVALID_HANDLE;
	}
	/* Destroy a valid receiver */


	if (ETAL_INVALID_HANDLE != hReceiver)
	{
		etalTestPrintNormal("* Destroy AM receiver");
		if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
		hReceiver = ETAL_INVALID_HANDLE;
	}


	/* Create invalid receiver */

	etalTestPrintReportPassStart(CFG_RCV_AM_CREATE_FE2, ETAL_TEST_MODE_AM, "Create receiver on FE 2 for AM");

	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_AM; // the background channel of CMOST does not support AM
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_2;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass4c failed");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	// destroy the receiver
	etalTestPrintNormal("* Destroy AM receiver");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
			etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
	}
	hReceiver = ETAL_INVALID_HANDLE;
	
#endif // CONFIG_APP_TEST_AM

#if defined (CONFIG_APP_TEST_DAB) & defined (CONFIG_APP_TEST_FM)

	/* 
	 * Create a DAB receiver then try to create an FM
	 * receiver using the same frontend
	 */

	etalTestPrintReportPassStart(CFG_RCV_DAB_CREATE_AGAIN, ETAL_TEST_MODE_DAB, "Create receiver again");

	hReceiver = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass5a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(CFG_RCV_FM_CREATE_BUSY_DAB_FE, ETAL_TEST_MODE_FM, "Create receiver using non-free DAB frontend");

	hReceiver2 = ETAL_INVALID_HANDLE;
	attr.m_Standard = ETAL_BCAST_STD_FM;
	// set Front End Size
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
	if ((ret = etal_config_receiver(&hReceiver2, &attr)) != ETAL_RET_FRONTEND_LIST_ERR)
	{
	 	pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass5b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintNormal("* Destroy valid DAB receiver");
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

#if defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) || defined (CONFIG_ETAL_HAVE_ALL_API)
	/* 
	 * Create a configuration with monitors for DAB and change it to a FM, 
	 * ensure the monitors are destroyed
	 */

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_DAB, hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("* Create a Monitor for the DAB receiver");
	OSAL_pvMemorySet(&mondab, 0x00, sizeof(EtalBcastQualityMonitorAttr));
	mondab.m_receiverHandle = hReceiver;
	mondab.m_CbBcastQualityProcess = etalTestMonitorcb;
	mondab.m_monitoredIndicators[0].m_MonitoredIndicator = EtalQualityIndicator_DabFieldStrength;
	mondab.m_monitoredIndicators[0].m_InferiorValue = ETAL_TEST_MIN_DAB_RFFIELDSTRENGTH;
	mondab.m_monitoredIndicators[0].m_SuperiorValue = ETAL_TEST_MAX_DAB_RFFIELDSTRENGTH;
	mondab.m_monitoredIndicators[0].m_UpdateFrequency = 500; // millesec
	if ((ret = etal_config_reception_quality_monitor(&monitor_dab_handle, &mondab)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_reception_quality_monitor for DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(CFG_RCV_DAB_RECONF, ETAL_TEST_MODE_FM, "Reconfigure DAB receiver and ensure monitor is stopped");
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if ((ret = etal_destroy_receiver(&hReceiver)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	
	if ((ret = etal_config_receiver(&hReceiver, &attr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintNormal("pass5c FAILED");
	}

	QCount.DabCbInvocations = 0;
	etalTestPrintNormal("* Waiting for responses to hit the callback...");
	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);
	etalTestPrintNormal("= %d DAB callback invocations", QCount.DabCbInvocations);
	etalTestPrintNormal("= (expected 0)");
	if (QCount.DabCbInvocations != 0)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass5d FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (etalTestUndoConfigSingle(&hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_ETAL_HAVE_QUALITY_MONITOR || CONFIG_ETAL_HAVE_ALL_API

#endif // CONFIG_APP_TEST_DAB & CONFIG_APP_TEST_FM

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_CONFIG_RECEIVER
	return OSAL_OK;
}

#ifdef CONFIG_APP_TEST_DESTROY_RECEIVER
/***************************
 *
 * etalTestDestroyReceiver_internal
 *
 **************************/
static tSInt etalTestDestroyReceiver_internal(etalTestBroadcastTy test_mode, tBool *pass)
{
	ETAL_STATUS ret;
	ETAL_HANDLE monitor_handle;
	EtalBcastQualityMonitorAttr monitor;
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	EtalBcastQaIndicators quality_indicator;
	ETAL_HANDLE *phReceiver;
	tCString string_mode;
	tU32 *counter;
	tS32 quality_lower_limit, quality_upper_limit;

	OSAL_pvMemorySet(&monitor, 0x00, sizeof(monitor));

	switch (test_mode)
	{
		case ETAL_TEST_MODE_DAB:
			phReceiver = &handledab;
			config_mode = ETAL_CONFIG_DAB;
			tune_mode = ETAL_TUNE_DAB;
			counter = &QCount.DabCbInvocations;
			quality_indicator = EtalQualityIndicator_DabFieldStrength;
			quality_lower_limit = ETAL_TEST_MIN_DAB_RFFIELDSTRENGTH;
			quality_upper_limit = ETAL_TEST_MAX_DAB_RFFIELDSTRENGTH;
			break;

		case ETAL_TEST_MODE_FM:
			phReceiver = &handlefm;
			config_mode = ETAL_CONFIG_FM1;
			tune_mode = ETAL_TUNE_FM;
			counter = &QCount.FmCbInvocations;
			quality_indicator = EtalQualityIndicator_FmFieldStrength;
			quality_lower_limit = ETAL_TEST_MIN_FM_FIELDSTRENGTH;
			quality_upper_limit = ETAL_TEST_MAX_FM_FIELDSTRENGTH;
			break;

		case ETAL_TEST_MODE_AM:
			phReceiver = &handleam;
			config_mode = ETAL_CONFIG_AM;
			tune_mode = ETAL_TUNE_AM;
			counter = &QCount.AmCbInvocations;
			quality_indicator = EtalQualityIndicator_FmFieldStrength; /* not a typo, same name as FM */
			quality_lower_limit = ETAL_INVALID_MONITOR;
			quality_upper_limit = ETAL_INVALID_MONITOR;
			break;

		case ETAL_TEST_MODE_HD_FM:
			phReceiver = &handlehd;
			config_mode = ETAL_CONFIG_HDRADIO_FM;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			counter = &QCount.HdCbInvocations;
			quality_indicator = EtalQualityIndicator_FmFieldStrength; /* use the analogue FS instead of a HD quantity */
			quality_lower_limit = ETAL_TEST_MIN_FM_FIELDSTRENGTH;
			quality_upper_limit = ETAL_TEST_MAX_FM_FIELDSTRENGTH;
			break;

		case ETAL_TEST_MODE_HD_AM:
			phReceiver = &handleam;
			config_mode = ETAL_CONFIG_HDRADIO_AM;
			tune_mode = ETAL_TUNE_HDRADIO_AM;
			counter = &QCount.HdamCbInvocations;
			quality_indicator = EtalQualityIndicator_FmFieldStrength; /* use the analogue FS instead of a HD quantity */
			quality_lower_limit = ETAL_INVALID_MONITOR;
			quality_upper_limit = ETAL_INVALID_MONITOR;
			break;

		default:
			return OSAL_ERROR;
	}

	string_mode = etalTestStandard2String(test_mode);

	/* Create a receiver */

	if (etalTestDoConfigSingle(config_mode, phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Create a Monitor for the receiver */

	etalTestPrintNormal("* Create a Monitor for the %s receiver", string_mode);
	*counter = 0;
	monitor_handle = ETAL_INVALID_HANDLE;
	monitor.m_monitoredIndicators[0].m_MonitoredIndicator = quality_indicator;
	monitor.m_monitoredIndicators[0].m_InferiorValue = quality_lower_limit;
	monitor.m_monitoredIndicators[0].m_SuperiorValue = quality_upper_limit;
	monitor.m_monitoredIndicators[0].m_UpdateFrequency = ETAL_TEST_ONE_SECOND;
	monitor.m_receiverHandle = *phReceiver;
	monitor.m_CbBcastQualityProcess = etalTestMonitorcb;
	if ((ret = etal_config_reception_quality_monitor(&monitor_handle, &monitor)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_reception_quality_monitor for %s (%s, %d)", string_mode, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Ensure the callback is invoked */

	etalTestPrintNormal("* Waiting for responses to hit the Monitor callback...");
	OSAL_s32ThreadWait(3 * ETAL_TEST_ONE_SECOND);
	etalTestPrintNormal("= %d %s callback invocations", *counter, string_mode);
	etalTestPrintNormal("= (expected more than %d, less than %d)", DESTROY_RECEIVER_CB_MIN, DESTROY_RECEIVER_CB_MAX);

	etalTestPrintReportPassStart(DESTROY_RCV_CHECK_MON_RCV, test_mode, "Check Monitor associated with receiver");
	if ((*counter <= DESTROY_RECEIVER_CB_MIN) || (*counter >= DESTROY_RECEIVER_CB_MAX))
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Destroy the receiver */

	if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* Ensure the callback is no longer invoked */

	OSAL_s32ThreadWait(50);
	*counter = 0;
	etalTestPrintNormal("* Waiting for responses to hit the callback...");
	OSAL_s32ThreadWait(3 * ETAL_TEST_ONE_SECOND);
	etalTestPrintNormal("= %d %s callback invocations", *counter, string_mode);
	etalTestPrintNormal("= (expected 0)");
	etalTestPrintReportPassStart(DESTROY_RCV_CHECK_MON_DESTROYED_RCV, test_mode, "Check Monitor associated with destroyed receiver");

	if (*counter != 0)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DESTROY_RECEIVER

/***************************
 *
 * etalTestDestroyReceiver
 *
 **************************/
/*
 * This test verifies the Receiver destruction.
 * It creates a monitor, tunes the receiver and counts the number
 * of times the monitor callback is hit (must be greater than 0).
 * Then it destroys the Receiver but not the monitor and verifies 
 * that the callback is no longer invoked
 */
tSInt etalTestDestroyReceiver(void)
{
#ifdef CONFIG_APP_TEST_DESTROY_RECEIVER
	tBool pass = TRUE;

	etalTestStartup();

#if defined (CONFIG_APP_TEST_DAB)
	if (etalTestDestroyReceiver_internal(ETAL_TEST_MODE_DAB, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_FM)
	if (etalTestDestroyReceiver_internal(ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_AM)
	/* AM is not received in Agrate without external antenna so this
	 * test always fails without a generator*/
	if (etalTestDestroyReceiver_internal(ETAL_TEST_MODE_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	if (etalTestDestroyReceiver_internal(ETAL_TEST_MODE_HD_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_HDRADIO_AM)
	if (etalTestDestroyReceiver_internal(ETAL_TEST_MODE_HD_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_DESTROY_RECEIVER
	return OSAL_OK;
}


#ifdef CONFIG_APP_TEST_GET_VERSION
/***************************
 *
 * etalTestPrintVersion
 *
 **************************/
static tVoid etalTestPrintVersion(tChar *str, EtalComponentVersion *vers)
{
	if (vers->m_isValid)
	{
		etalTestPrintNormal("%s version %d.%d.%d build %d", str, vers->m_major, vers->m_middle, vers->m_minor, vers->m_build);
		etalTestPrintNormal("%s version string: %s", str, vers->m_name);
	}
	else
	{
		etalTestPrintNormal("%s not present", str);
	}
}

/***************************
 *
 * etalTestPrintAllVersion
 *
 **************************/
static tVoid etalTestPrintAllVersion(EtalVersion *vers)
{
	tU32 i;

	etalTestPrintVersion("ETAL", &vers->m_ETAL);
	etalTestPrintNormal("");
	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		etalTestPrintVersion("CMOST", &vers->m_CMOST[i]);
		etalTestPrintNormal("");
	}
	etalTestPrintVersion("MDR", &vers->m_MDR);
	etalTestPrintNormal("");
	etalTestPrintVersion("HDRadio", &vers->m_HDRadio);
	etalTestPrintNormal("");
}
#endif // CONFIG_APP_TEST_GET_VERSION

/***************************
 *
 * etalTestGetVersion
 *
 **************************/
tSInt etalTestGetVersion(void)
{
#ifdef CONFIG_APP_TEST_GET_VERSION
	ETAL_STATUS ret;
	tBool pass;
	EtalVersion version;

	etalTestStartup();
	pass = TRUE;

	etalTestPrintReportPassStart(GET_VERS_CHECK_RETURN, ETAL_TEST_MODE_NONE, "Check return value");
	ret = etal_get_version(&version);
	if (ret != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1 FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintAllVersion(&version);
	}

	if (pass)
	{
		etalTestPrintReportPassStart(GET_VERS_CHECK_ETAL_VERS_SANITY, ETAL_TEST_MODE_NONE, "Check ETAL version string sanity");
		if ((version.m_ETAL.m_isValid != TRUE) ||
			(version.m_ETAL.m_major == 0) ||
			(version.m_ETAL.m_build == 0) ||
			(OSAL_u32StringLength(version.m_ETAL.m_name) == 0))
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2a FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
#if defined (CONFIG_ETAL_SUPPORT_CMOST)
		etalTestPrintReportPassStart(GET_VERS_CHECK_CMOST_VERS_SANITY, ETAL_TEST_MODE_NONE, "Check CMOST version string sanity");
		if ((version.m_CMOST[0].m_isValid != TRUE) ||
			(version.m_CMOST[0].m_major == 0) ||
			(version.m_CMOST[0].m_build != 0) ||
			(OSAL_u32StringLength(version.m_CMOST[0].m_name) == 0))
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2b FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
#if defined (CONFIG_MODULE_INTEGRATED)
		etalTestPrintReportPassStart(GET_VERS_CHECK_CMOST_VERS_SANITY_2ND_TUNER, ETAL_TEST_MODE_NONE, "Check CMOST version string sanity, second tuner");
		if ((version.m_CMOST[1].m_isValid != TRUE) ||
			(version.m_CMOST[1].m_major == 0) ||
			(version.m_CMOST[1].m_build != 0) ||
			(OSAL_u32StringLength(version.m_CMOST[1].m_name) == 0))
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
#endif
#endif

#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
		etalTestPrintReportPassStart(GET_VERS_CHECK_DAB_DCOP_VERS_SANITY, ETAL_TEST_MODE_NONE, "Check DAB DCOP version string sanity");
		if ((version.m_MDR.m_isValid != TRUE) ||
			(version.m_MDR.m_build == 0) ||
			(OSAL_u32StringLength(version.m_MDR.m_name) == 0))
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2c FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
#endif

#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
		etalTestPrintReportPassStart(GET_VERS_CHECK_HD_DCOP_VERS_SANITY, ETAL_TEST_MODE_NONE, "Check HD DCOP version string sanity");
		if ((version.m_HDRadio.m_isValid != TRUE) ||
			(version.m_HDRadio.m_major == 0) ||
			(version.m_HDRadio.m_build > 999) ||
			(OSAL_u32StringLength(version.m_HDRadio.m_name) == 0))
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2d FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
#endif
	}

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

