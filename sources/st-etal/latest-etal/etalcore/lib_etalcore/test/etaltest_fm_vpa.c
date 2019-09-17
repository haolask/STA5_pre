//!
//!  \file 		etaltest_fm_vpa.c
//!  \brief 	<i><b> ETAL test, FM VPA configuration </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	David Pastor
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#if defined (CONFIG_APP_TEST_FM_VPA)
#include <math.h>
#include <limits.h> // for INT_MIN

/*
 * Function Prototypes
 */
#if (defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM)) && defined (CONFIG_ETAL_HAVE_ETALTML)
static void etalTestVpaRDSRawCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
#endif //CONFIG_APP_TEST_FM



/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX	6	/* dBuV */
#define ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID	pow(2,30)

typedef enum {
	FM_VPA_CONFIGURE_FG_BG_VPA_ON = 1,
	FM_VPA_RECONF_FG_VPA_OFF,
	FM_VPA_RECONF_FG_BG_VPA_ON,
	FM_VPA_CONFIGURE_INVALID_FG_FG_VPA_ON,
	FM_VPA_CONFIGURE_INVALID_BG_BG_VPA_ON,
	FM_VPA_CONFIGURE_INVALID_AM_BAND_VPA_ON,
	FM_VPA_CONFIGURE_INVALID_DAB_BAND_VPA_ON,
	FM_VPA_CHECK_VPA_ON_USING_QUALITY,
	FM_VPA_CHECK_VPA_OFF_USING_QUALITY_BOTH_CHANNELS,
	FM_VPA_RECONF_RCV_CHECK_VPA_ON,
	FM_VPA_RECONF_RCV_CHECK_VPA_OFF,
	FM_VPA_TUNE_ANOTHER_FREQ_CHECK_VPA_OFF,
	FM_VPA_TUNE_ANOTHER_FREQ_CHECK_VPA_ON,
	FM_VPA_CHECK_VPA_OFF_BG_USING_QUALITY,
	FM_VPA_RECONF_RCV_BG_FG_CHECK_VPA_ON,
	FM_VPA_RECONF_RCV_BG_CHECK_VPA_OFF,
	FM_VPA_CHECK_TUNER2_VPA_ON_USING_QUALITY,
	FM_VPA_VPA_ON_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON,
	FM_VPA_VPA_OFF_AF_CHECK_ON_FG_AUDIO_ON_AF_CHECK_ON,
	FM_VPA_VPA_OFF_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON,
	FM_VPA_VPA_OFF_AF_CHECK_ON_BG_AF_CHECK_ON,
	FM_VPA_RECONF_VPA_ON_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON,
	FM_VPA_RECONF_VPA_OFF_AF_CHECK_ON_BG_AF_CHECK_ON,
	FM_VPA_VPA_ON_VERIFY_ACTIVITY,
	FM_VPA_ON_AF_START_BG,
	FM_VPA_ON_AF_RESTART_BG,
	FM_VPA_ON_AF_END_BG,
	FM_VPA_ON_RDS_ON_FG,
	FM_VPA_RECONF_VPA_ON
} FMVPATestTy;


/*****************************************************************
| variable defintion (scope: module-local)
|----------------------------------------------------------------*/


#if (defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM)) && defined (CONFIG_ETAL_HAVE_ETALTML)
/***************************
 *
 * etalTestVpaRDSRawCallback
 *
 **************************/
static void etalTestVpaRDSRawCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	if (((ETAL_HANDLE)(tULong)pvContext == handlefm) || ((ETAL_HANDLE)(tULong)pvContext == handlehd))
	{
		etalTestGetRDSRawAccumulate(&rdsRawDatapathData, (EtalRDSRawData *)pBuffer, dwActualBufferSize);
		etalTestPrintRDSRaw(&rdsRawDatapathData, (ETAL_HANDLE)(tULong)pvContext);
		RdsRawCbInvocations++;
	}
	else if (((ETAL_HANDLE)(tULong)pvContext == handlefm2) || ((ETAL_HANDLE)(tULong)pvContext == handlehd2))
	{
		etalTestGetRDSRawAccumulate(&rdsRawDatapathData2, (EtalRDSRawData *)pBuffer, dwActualBufferSize);
		etalTestPrintRDSRaw(&rdsRawDatapathData2, (ETAL_HANDLE)(tULong)pvContext);
		RdsRawCbInvocations2++;
	}
}
#endif //CONFIG_APP_TEST_FM

/***************************
 *
 * etalTestVpaConfig
 *
 **************************/
static tSInt etalTestVpaConfig(tU8 tcn, tU8 processing_features, tBool *pass_out)
{
#if defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM)
	tBool pass1;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalReceiverAttr attr;
#endif

#if defined(CONFIG_APP_TEST_FM)
	etalTestStartup();
	pass1 = TRUE;

	etalTestPrintNormal("<---VpaConfig Test start");

	/*
	* create an FM receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_FG_BG_VPA_ON, ETAL_TEST_MODE_FM, "Configure FG and BG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((ret = etal_config_receiver(&handlefm, &attr)) != 
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		ETAL_RET_SUCCESS
#else
		ETAL_RET_FRONTEND_LIST_ERR
#endif
		)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%da failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* Created %s receiver, handle %d", etalTestStandard2Ascii(attr.m_Standard), handlefm);
	}

	/*
	* create an FM receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_FG_VPA_OFF, ETAL_TEST_MODE_FM, "Reconf FG VPA OFF pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(&handlefm, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%db failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an FM receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_FG_BG_VPA_ON, ETAL_TEST_MODE_FM, "Reconf FG and BG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((ret = etal_config_receiver(&handlefm, &attr)) != 
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		ETAL_RET_SUCCESS
#else
		ETAL_RET_FRONTEND_LIST_ERR
#endif
		)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%dc failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an FM receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_FG_VPA_OFF, ETAL_TEST_MODE_FM, "Reconf FG VPA OFF pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_NONE;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(&handlefm, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%dd failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo FM receiver configuration
	*/
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		pass1 = FALSE;
	}

	/*
	* create an invalid FM receiver foreground and foreground with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_INVALID_FG_FG_VPA_ON, ETAL_TEST_MODE_FM, "Configure invalid FG and FG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	attr.m_FrontEnds[1] = ETAL_FE_FOR_FM_TEST;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((ret = etal_config_receiver(&handlefm, &attr)) == ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%de failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an invalid FM receiver background and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_INVALID_BG_BG_VPA_ON, ETAL_TEST_MODE_FM, "Configure invalid BG and BG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((ret = etal_config_receiver(&handlefm, &attr)) == ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%df failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an invalid AM receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_INVALID_AM_BAND_VPA_ON, ETAL_TEST_MODE_AM, "Configure invalid band VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_AM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((ret = etal_config_receiver(&handlefm, &attr)) == ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%dg failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an invalid DAB receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_INVALID_DAB_BAND_VPA_ON, ETAL_TEST_MODE_DAB, "Configure invalid band VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_DAB;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((ret = etal_config_receiver(&handlefm, &attr)) == ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("FM VPA pass%dh failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo FM receiver configuration
	*/
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#if defined(CONFIG_APP_TEST_HDRADIO_FM)
	/*
	* create an HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_FG_BG_VPA_ON, ETAL_TEST_MODE_HD_FM, "Configure FG and BG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1/*ETAL_FE_FOR_HD_TEST*/;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED) {
		attr.processingFeatures.u.m_processing_features |= 
			(ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
	}
	if ((ret = etal_config_receiver(&handlehd, &attr)) != 
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		ETAL_RET_SUCCESS
#else
		ETAL_RET_FRONTEND_LIST_ERR
#endif
		)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("HD VPA pass%da failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* Created %s receiver, handle %d", etalTestStandard2Ascii(attr.m_Standard), handlehd);
	}

	/*
	* create an HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_FG_VPA_OFF, ETAL_TEST_MODE_HD_FM, "Reconf FG VPA OFF pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1/*ETAL_FE_FOR_HD_TEST*/;
	if (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED) {
		attr.processingFeatures.u.m_processing_features = 
			(ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(&handlehd, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("HD VPA pass%db failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_FG_BG_VPA_ON, ETAL_TEST_MODE_HD_FM, "ReconfFG and BG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1/*ETAL_FE_FOR_HD_TEST*/;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED) {
		attr.processingFeatures.u.m_processing_features |= 
			(ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
	}
	if ((ret = etal_config_receiver(&handlehd, &attr)) != 
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
		ETAL_RET_SUCCESS
#else
		ETAL_RET_FRONTEND_LIST_ERR
#endif
		)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("HD VPA failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_FG_VPA_OFF, ETAL_TEST_MODE_HD_FM, "ReconfFG VPA OFF pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1/*ETAL_FE_FOR_HD_TEST*/;
	if (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED) {
		attr.processingFeatures.u.m_processing_features = 
			(ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(&handlehd, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("HD VPA pass%dd failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	* create an invalid HD receiver foreground and foreground with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_INVALID_FG_FG_VPA_ON, ETAL_TEST_MODE_HD_FM, "Configure invalid FG and FG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1/*ETAL_FE_FOR_HD_TEST*/;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_1/*ETAL_FE_FOR_HD_TEST*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED) {
		attr.processingFeatures.u.m_processing_features |= 
			(ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
	}
	if ((ret = etal_config_receiver(&handlehd, &attr)) == ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("HD VPA pass%de failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an invalid HD receiver background and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CONFIGURE_INVALID_BG_BG_VPA_ON, ETAL_TEST_MODE_HD_FM,"Configure invalid BG and BG VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_HD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
	attr.m_FrontEnds[1] = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED) {
		attr.processingFeatures.u.m_processing_features |= 
			(ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
	}
	if ((ret = etal_config_receiver(&handlehd, &attr)) == ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("HD VPA pass%df failed: etal_config_receiver %s (%s, %d)", tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#if defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM)
	if (!pass1)
	{
		*pass_out = FALSE;
	}
#else
	etalTestPrintNormal("Test skipped");
#endif
	return OSAL_OK;
}

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
static tSInt etalTestVpaCheckVpaStateWithQuality(ETAL_HANDLE hReceiver, tBool isVpaModeOn, tBool *pass1, EtalReceiverAttr *attr, tU8 tcn, tU32 line)
{
	ETAL_STATUS ret;
	EtalBcastQualityContainer channelQualityTab[2];

	/*
	* wait 2s
	*/
	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND);
	OSAL_pvMemorySet(channelQualityTab, 0, sizeof(channelQualityTab));
	//channelQualityTab[1].EtalQualityEntries.amfm.m_RFFieldStrength = INT_MIN;

	if (isVpaModeOn == TRUE)
	{
		/*
		* check VPA is on with channel quality of both channel
		*/
		if ((ret = etal_get_channel_quality(hReceiver, channelQualityTab)) != ETAL_RET_SUCCESS)
		{
			*pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%d:%d failed: ETAL_cmdGetChannelQuality_CMOST %s (%s, %d)", etalTestStandard2Ascii(attr->m_Standard), tcn, line, etalTestStandard2Ascii(attr->m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		if (channelQualityTab[1].m_standard != attr->m_Standard)
		{
			*pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%d:%d failed: VPA mode not enabled %s (%s, %d) %d", etalTestStandard2Ascii(attr->m_Standard), tcn, line, etalTestStandard2Ascii(attr->m_Standard), ETAL_STATUS_toString(ret), ret, channelQualityTab[1].m_standard);
		}
	}
	else
	{
		/*
		* check VPA is off with channel quality of one channel
		*/
		if ((ret = etal_get_channel_quality(hReceiver, channelQualityTab)) != ETAL_RET_SUCCESS)
		{
			*pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%d:%d failed: ETAL_cmdGetChannelQuality_CMOST %s (%s, %d)", etalTestStandard2Ascii(attr->m_Standard), tcn, line, etalTestStandard2Ascii(attr->m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		if (channelQualityTab[1].m_standard != ETAL_BCAST_STD_UNDEF)
		{
			*pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%d:%d failed: VPA mode not disabled %s (%s, %d)", etalTestStandard2Ascii(attr->m_Standard), tcn, line, etalTestStandard2Ascii(attr->m_Standard), ETAL_STATUS_toString(ret), ret);
		}
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestVpaTune
 *
 **************************/
static tSInt etalTestVpaTune(tU8 tcn, EtalBcastStandard std, tU8 processing_features, tBool *pass_out)
{
	tBool pass1;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalReceiverAttr attr;
	EtalAudioInterfTy audioIf;
	ETAL_HANDLE *etaltest_handle, etaltest_fe_fg = 0, etaltest_fe_bg = 0;
#ifdef CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
	ETAL_HANDLE *etaltest_handle2, etaltest_fe2_fg = 0, etaltest_fe2_bg = 0;
#endif // CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
	EtalAudioSourceTy etaltest_audio_src;
	tU32 etaltest_freq, etaltest_freq2;
	tU8 etaltest_processing_features;
	etalTestTuneTy etaltest_tune_conf;
	etalTestBroadcastTy etaltest_std = ETAL_TEST_MODE_NONE;

	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		*pass_out = FALSE;
		return OSAL_ERROR;
	}
	if (std == ETAL_BCAST_STD_FM)
	{
		etaltest_handle = &handlefm;
		etaltest_fe_fg = ETAL_FE_FOR_FM_TEST;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_STAR_AMFM;
		etaltest_freq = ETAL_VALID_FM_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ2;
		etaltest_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		etaltest_tune_conf = ETAL_TUNE_FM;
		etaltest_std = ETAL_TEST_MODE_FM;
#ifdef CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
		etaltest_handle2 = &handlefm2;
		etaltest_fe2_fg = ETAL_FE_HANDLE_3;
		etaltest_fe2_bg = ETAL_FE_HANDLE_4;
#endif // CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
	}
	else if (std == ETAL_BCAST_STD_HD_FM)
	{
		etaltest_handle = &handlehd;
		etaltest_fe_fg = ETAL_FE_HANDLE_1 /*ETAL_FE_FOR_HD_TEST*/;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_AUTO_HD;
		etaltest_freq = ETAL_VALID_HD_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ;    /* no other alternate HD freq then use FM freq */
		etaltest_processing_features = (ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
		etaltest_tune_conf = ETAL_TUNE_HDRADIO_FM;
		etaltest_std = ETAL_TEST_MODE_HD_FM;
#ifdef CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
		etaltest_handle2 = &handlehd2;
		etaltest_fe2_fg = ETAL_FE_HANDLE_3;
		etaltest_fe2_bg = ETAL_FE_HANDLE_4;
#endif // CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
	}

	etalTestStartup();

	etalTestPrintNormal("<---VpaTune Test start");

	/*
	* create an FM or HD FM receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CHECK_VPA_ON_USING_QUALITY, etaltest_std, "Check VPA on using channel quality");
	pass1 = TRUE;

	/*
	* Configure audio path
	*/
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_audio_path (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
		

	etalTestPrintNormal("* Configure %s VPA ON pf=0x%x", etalTestStandard2Ascii(std), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}

	/*
	* Configure audio source
	*/
	if ((ret = etal_audio_select(*etaltest_handle, etaltest_audio_src)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_audio_select FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground on frequency 1
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}

	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_CHECK_VPA_OFF_USING_QUALITY_BOTH_CHANNELS, etaltest_std, "Check VPA off using channel quality, both channels");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}

	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_ON, etaltest_std, "Reconf receiver, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}

	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_OFF, etaltest_std, "Reconf receiver, check VPA off");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Tune another FM/HD receiver foreground on frequency 2
	*/
	etalTestPrintReportPassStart(FM_VPA_TUNE_ANOTHER_FREQ_CHECK_VPA_OFF, etaltest_std, "Tune another frequency, check VPA off");
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq2, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq2)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA OFF, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_ON, etaltest_std, "Reconf receiver, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}
	/*
	* Tune another FM/HD receiver foreground on frequency 1
	*/
	etalTestPrintReportPassStart(FM_VPA_TUNE_ANOTHER_FREQ_CHECK_VPA_ON, etaltest_std, "Tune another frequency, check VPA on");
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}


	/*
	* Undo tune FM/HD foreground
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle) != OSAL_OK)
	{
		pass1 = FALSE;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle) != OSAL_OK)
	{
		pass1 = FALSE;
	}

	/*
	* create an FM receiver background with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_CHECK_VPA_OFF_BG_USING_QUALITY, etaltest_std, "Check VPA off BG using channel quality");
	pass1 = TRUE;
	etalTestPrintNormal("* Configure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(std), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver background on frequency 2
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq2, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq2)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA OFF, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Modify FM/HD receiver background and foreground with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_BG_FG_CHECK_VPA_ON, etaltest_std, "Reconf receiver BG FG, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_bg;	/* first position:  BG */
	attr.m_FrontEnds[1] = etaltest_fe_fg;	/* second position: FG */
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}
	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_OFF, etaltest_std, "Reconf receiver, check VPA off");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_ON, etaltest_std, "Reconf receiver, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}
	/*
	* Modify FM/HD receiver background with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_BG_CHECK_VPA_OFF, etaltest_std, "Reconf receiver BG, check VPA off");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_bg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver background on frequency 1
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA OFF, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Modify FM/HD receiver background and foreground with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_BG_FG_CHECK_VPA_ON, etaltest_std, "Reconf receiver BG FG, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_bg;	/* first position:  BG */
	attr.m_FrontEnds[1] = etaltest_fe_fg;	/* second position: FG */
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}
	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_OFF, etaltest_std, "Reconf receiver, check VPA off");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_ON, etaltest_std, "Reconf receiver, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}
	/*
	* Modify FM/HD receiver foreground with VPA OFF AM
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_OFF, etaltest_std, "Reconf receiver, check VPA off");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(ETAL_BCAST_STD_AM), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_AM;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune AM receiver foreground on frequency 1
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), ETAL_VALID_AM_FREQ, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, ETAL_VALID_AM_FREQ)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is off with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, FALSE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is disabled");
	}
	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_CHECK_VPA_ON, etaltest_std, "Reconf receiver, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground on frequency 1
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}

#ifdef CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
	/*
	* Modify FM/HD receiver background and foreground with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_RCV_BG_FG_CHECK_VPA_ON, etaltest_std, "Reconf receiver BG FG, check VPA on");
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_bg;	/* first position:  BG */
	attr.m_FrontEnds[1] = etaltest_fe_fg;	/* second position: FG */
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}


	/*
	* create an FM receiver tuner 2 foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_CHECK_TUNER2_VPA_ON_USING_QUALITY, etaltest_std, "Check Tuner2 VPA on using channel quality");
	pass1 = TRUE;
	etalTestPrintNormal("* Configure Tuner2 %s VPA ON pf=0x%x", etalTestStandard2Ascii(ETAL_BCAST_STD_FM), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe2_fg;
	attr.m_FrontEnds[1] = etaltest_fe2_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle2, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground on frequency 1
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle2);
	if (((ret = etal_tune_receiver(*etaltest_handle2, etaltest_freq)) != ETAL_RET_SUCCESS) && 
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%d:%d failed: etal_tune_receiver FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, __LINE__, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle2);
	}
	/*
	* check VPA is on with channel quality
	*/
	etalTestVpaCheckVpaStateWithQuality(*etaltest_handle2, TRUE, &pass1, &attr, tcn, __LINE__);
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
		etalTestPrintVerbose("* VPA mode is enabled");
	}
#endif // CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707



	/*
	* Undo tune FM/HD foreground
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle) != OSAL_OK)
	{
		pass1 = FALSE;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle) != OSAL_OK)
	{
		pass1 = FALSE;
	}

#ifdef CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707
	/*
	* Undo tune FM/HD bg
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle2) != OSAL_OK)
	{
		pass1 = FALSE;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle2) != OSAL_OK)
	{
		pass1 = FALSE;
	}
#endif

	return OSAL_OK;
}

/***************************
 *
 * etalTestVpaAfCheck
 *
 **************************/
static tSInt etalTestVpaAfCheck(tU8 tcn, EtalBcastStandard std, tU8 processing_features, tBool *pass_out)
{
	tBool pass1;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalReceiverAttr attr;
	EtalAudioInterfTy audioIf;
	EtalBcastQualityContainer qualResult;
	tU32 count;
	tS32 last_RFFieldStrength = ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID;
	ETAL_HANDLE *etaltest_handle, *etaltest_handle2, etaltest_fe_fg = 0, etaltest_fe_bg = 0;
	EtalAudioSourceTy etaltest_audio_src;
	tU32 etaltest_freq, etaltest_freq2;
	tU8 etaltest_processing_features;
	etalTestTuneTy etaltest_tune_conf;
	etalTestBroadcastTy etaltest_std = ETAL_TEST_MODE_NONE;

	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		*pass_out = FALSE;
		return OSAL_ERROR;
	}
	if (std == ETAL_BCAST_STD_FM)
	{
		etaltest_handle = &handlefm;
		etaltest_handle2 = &handlefm2;
		etaltest_fe_fg = ETAL_FE_FOR_FM_TEST;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_STAR_AMFM;
		etaltest_freq = ETAL_VALID_FM_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ2;
		etaltest_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		etaltest_tune_conf = ETAL_TUNE_FM;
		etaltest_std = ETAL_TEST_MODE_FM;
	}
	else if (std == ETAL_BCAST_STD_HD_FM)
	{
		etaltest_handle = &handlehd;
		etaltest_handle2 = &handlehd2;
		etaltest_fe_fg = ETAL_FE_HANDLE_1 /*ETAL_FE_FOR_HD_TEST*/;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_AUTO_HD;
		etaltest_freq = ETAL_VALID_HD_FREQ;
		etaltest_freq2 = ETAL_VALID_HD_FREQ2;
		etaltest_processing_features = (ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
		etaltest_tune_conf = ETAL_TUNE_HDRADIO_FM;
		etaltest_std = ETAL_TEST_MODE_HD_FM;
	}

	etalTestStartup();

	etalTestPrintNormal("<---VpaAfCheck Test start");

	/*
	* create an FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_VPA_ON_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON, etaltest_std, "VPA on, AF check on BG, audio on %s2, AF check on %s1", etalTestStandard2Ascii(std), etalTestStandard2Ascii(std));
	pass1 = TRUE;

	/*
	* Configure audio path
	*/
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%db failed: etal_config_audio_path (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
		

	etalTestPrintNormal("* Configure FM VPA ON pf=0x%x", processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%da failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}

	/*
	* Configure audio source
	*/
	if ((ret = etal_audio_select(*etaltest_handle, etaltest_audio_src)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dc failed: etal_audio_select FM (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Tune to %d freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dd failed: etal_tune_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), handlefm);
	}
	/* Test AF check on the background channel, audio on antenna FM2/HD2, AF check on antenna FM1/HD1 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on background channel freq: %d with %s1 antenna selected", etaltest_freq2, etalTestStandard2Ascii(attr.m_Standard));
		if (etal_AF_check(*etaltest_handle, etaltest_freq2, 2, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%de failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%df failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%df failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%df failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_VPA_OFF_AF_CHECK_ON_FG_AUDIO_ON_AF_CHECK_ON, etaltest_std, "VPA off, AF check on FG, audio and AF check on %s1", etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dg failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/* Test AF check on the foreground channel, audio and AF check on FM1/HD1 antenna */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on foreground channel freq: %d", etaltest_freq2);
		if (etal_AF_check(*etaltest_handle, etaltest_freq2, 0, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dh failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%di failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%di failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%di failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_VPA_ON_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON, etaltest_std, "VPA on, AF check on BG, audio on %s2, AF check on %s1", etalTestStandard2Ascii(attr.m_Standard), etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dj failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/* Test AF check on the background channel, audio on antenna FM2/HD2, AF check on antenna FM1/HD1 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on background channel freq: %d with %s1 antenna selected", etaltest_freq2, etalTestStandard2Ascii(attr.m_Standard));
		if (etal_AF_check(*etaltest_handle, etaltest_freq2, 2, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dk failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dl failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dl failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dl failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_VPA_OFF_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON, etaltest_std, "VPA off, AF check on BG, audio and AF check on %s1", etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dm failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/* Test AF check on the background channel, audio and AF check on antenna FM1/HD1 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on foreground channel freq: %d", etaltest_freq2);
		if (etal_AF_check(*etaltest_handle, etaltest_freq2, 0, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%do failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dp failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dp failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dp failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* create an FM/HD receiver background with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_VPA_OFF_AF_CHECK_ON_BG_AF_CHECK_ON, etaltest_std, "VPA off, AF check on BG, AF check on %s2", etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Configure %s2 VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_bg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle2, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dq failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	last_RFFieldStrength = ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID;
	/*
	* Tune FM/HD receiver background
	*/
	etalTestPrintNormal("* Tune to %s2 freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle2);
	if (((ret = etal_tune_receiver(*etaltest_handle2, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dr failed: etal_tune_receiver %S2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s2 receiver with VPA OFF, handle %d", etalTestStandard2Ascii(attr.m_Standard), handlefm2);
	}
	/* Test AF check on the background channel, AF check on antenna FM2/HD2 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on background channel freq: %d", etaltest_freq2);
		if (etal_AF_check(*etaltest_handle2, etaltest_freq2, 0, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%ds failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dt failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dt failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dt failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo tune FM2/HD2 background
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM2/HD2 receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_VPA_ON_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON, etaltest_std, "Reconf VPA on, AF check on BG, audio on %s1, AF check on %s2", etalTestStandard2Ascii(attr.m_Standard), etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%du failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/* Test AF check on the background channel, audio on antenna FM1/HD1, AF check on antenna FM2/HD2 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on background channel freq: %d with %s2 antenna selected", etaltest_freq2, etalTestStandard2Ascii(attr.m_Standard));
		if (etal_AF_check(*etaltest_handle, etaltest_freq2, 1, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dw failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dx failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dx failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dx failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_VPA_OFF_AF_CHECK_ON_BG_AF_CHECK_ON, etaltest_std, "Reconf VPA off, AF check on BG, AF check on %s2", etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dy failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* create an FM/HD receiver background with VPA OFF
	*/
	etalTestPrintNormal("* Configure %s2 VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_bg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle2, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%daa failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	last_RFFieldStrength = ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID;
	/*
	* Tune FM/HD receiver background
	*/
	etalTestPrintNormal("* Tune to %s2 freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle2);
	if (((ret = etal_tune_receiver(*etaltest_handle2, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dab failed: etal_tune_receiver %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s2 receiver with VPA OFF, handle %d", etalTestStandard2Ascii(attr.m_Standard), handlefm2);
	}
	/* Test AF check on the background channel, AF check on antenna FM2/HD2 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on background channel freq: %d", etaltest_freq2);
		if (etal_AF_check(*etaltest_handle2, etaltest_freq2, 0, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dac failed: etal_AF_check %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dad failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dad failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dad failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo tune FM2/HD2 background
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM2/HD2 receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_VPA_ON_AF_CHECK_ON_BG_AUDIO_ON_AF_CHECK_ON, etaltest_std, "Reconf VPA on, AF check on BG, audio on %s1, AF check on %s2", etalTestStandard2Ascii(attr.m_Standard), etalTestStandard2Ascii(attr.m_Standard));
	pass1 = TRUE;
	etalTestPrintNormal("* Reconfigure %s VPA ON pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dae failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/* Test AF check on the background channel, audio on antenna FM1/HD1, AF check on antenna FM2/HD2 */
	count = 0;
	while(count < 2)
	{
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("* AF check starts: on background channel freq: %d with %s2 antenna selected", ETAL_VALID_FM_FREQ2, etalTestStandard2Ascii(attr.m_Standard));
		if (etal_AF_check(*etaltest_handle, etaltest_freq2, 1, &qualResult) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%daf failed: etal_AF_check %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintVerboseContainer(&qualResult);
			if (qualResult.m_standard == ETAL_BCAST_STD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dag failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
			}
			else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
			{
				if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
					(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
				{
					pass1 = FALSE;
					etalTestPrintNormal("%s VPA pass%dag failed: etal_AF_check amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
						ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
				}
				last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			}
			else
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dag failed: etal_AF_check qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
			}
		}
		count++;
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	
	/*
	* Undo tune FM/HD foreground
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestVpaFgActivity
 *
 **************************/
static tSInt etalTestVpaFgActivity(tU8 tcn, EtalBcastStandard std, tU8 processing_features, tBool *pass_out)
{
	tBool pass1;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalReceiverAttr attr;
	EtalAudioInterfTy audioIf;
	tU32 frequency;
	EtalSeekStatus seekStatus;
	ETAL_HANDLE *etaltest_handle, etaltest_fe_fg = 0, etaltest_fe_bg = 0;
	EtalAudioSourceTy etaltest_audio_src;
	tU32 etaltest_freq, etaltest_freq2;
	tU8 etaltest_processing_features;
	etalTestTuneTy etaltest_tune_conf;
	etalTestBroadcastTy etaltest_std = ETAL_TEST_MODE_NONE;

	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		*pass_out = FALSE;
		return OSAL_ERROR;
	}
	if (std == ETAL_BCAST_STD_FM)
	{
		etaltest_handle = &handlefm;
		etaltest_fe_fg = ETAL_FE_FOR_FM_TEST;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_STAR_AMFM;
		etaltest_freq = ETAL_VALID_FM_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ2;
		etaltest_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		etaltest_tune_conf = ETAL_TUNE_FM;
		etaltest_std = ETAL_TEST_MODE_FM;
	}
	else if (std == ETAL_BCAST_STD_HD_FM)
	{
		etaltest_handle = &handlehd;
		etaltest_fe_fg = ETAL_FE_HANDLE_1 /*ETAL_FE_FOR_HD_TEST*/;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_AUTO_HD;
		etaltest_freq = ETAL_VALID_HD_FREQ;
		etaltest_freq2 = ETAL_VALID_HD_FREQ2;
		etaltest_processing_features = (ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
		etaltest_tune_conf = ETAL_TUNE_HDRADIO_FM;
		etaltest_std = ETAL_TEST_MODE_HD_FM;
	}

	etalTestPrintNormal("<---VpaFgActivity Test start");
	etalTestStartup();

	etalTestPrintReportPassStart(FM_VPA_VPA_ON_VERIFY_ACTIVITY, etaltest_std, "VPA on, verify VPA activity on FG");
	pass1 = TRUE;
	/*
	* create an FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintNormal("* Configure %s VPA ON pf=0x%x", etalTestStandard2Ascii(std), processing_features);

	/*
	* Configure audio path
	*/
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%db failed: etal_config_audio_path (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
		

	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%da failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}

	/*
	* Configure audio source
	*/
	if ((ret = etal_audio_select(*etaltest_handle, etaltest_audio_src)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dc failed: etal_audio_select %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dd failed: etal_tune_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* Tune FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq2, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq2)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%de failed: etal_tune_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* manuel seek start FM/HD receiver foreground
	*/
	if ((ret = etal_seek_start_manual(*etaltest_handle, cmdDirectionDown, 500, &frequency)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%df failed: etal_seek_start_manual %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	etalTestPrintNormal("* etal_seek_start_manual : direction %d, step %dkHz, returned value : freq %lu", cmdDirectionDown, 500, frequency);
	if ((ret = etal_seek_get_status_manual(*etaltest_handle, &seekStatus)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dg failed: etal_seek_get_status_manual %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* manuel seek continue FM/HD receiver foreground
	*/
	if ((ret = etal_seek_continue_manual(*etaltest_handle, &frequency)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dh failed: etal_seek_continue_manual %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	etalTestPrintNormal("* etal_seek_continue_manual : returned value : freq %lu", frequency);
	if ((ret = etal_seek_get_status_manual(*etaltest_handle, &seekStatus)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%di failed: etal_seek_get_status_manual %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* manuel seek stop FM/HD receiver foreground
	*/
	if ((ret = etal_seek_stop_manual(*etaltest_handle, cmdAudioUnmuted, &frequency)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dj failed: etal_seek_stop_manual %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	etalTestPrintNormal("* etal_seek_stop_manual : mute %d, returned value : freq %lu", cmdAudioUnmuted, frequency);
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* AF switch FM/HD receiver foreground
	*/
	etalTestPrintNormal("* AF switch to freq %d", etaltest_freq);
	if ((ret = etal_AF_switch(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dk failed: etal_AF_switch (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo tune FM/HD foreground
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestVpaAfStart
 *
 **************************/
static tSInt etalTestVpaAfStart(tU8 tcn, EtalBcastStandard std, tU8 processing_features, tBool *pass_out)
{
	tBool pass1;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalReceiverAttr attr;
	EtalAudioInterfTy audioIf;
	EtalBcastQualityContainer qualResult;
	tS32 last_RFFieldStrength = ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID;
	tU32 count;
	ETAL_HANDLE *etaltest_handle, etaltest_fe_fg = 0, etaltest_fe_bg = 0;
	EtalAudioSourceTy etaltest_audio_src;
	tU32 etaltest_freq, etaltest_freq2;
	tU8 etaltest_processing_features;
	etalTestTuneTy etaltest_tune_conf;
	etalTestBroadcastTy etaltest_std = ETAL_TEST_MODE_NONE;

	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		*pass_out = FALSE;
		return OSAL_ERROR;
	}
	if (std == ETAL_BCAST_STD_FM)
	{
		etaltest_handle = &handlefm;
		etaltest_fe_fg = ETAL_FE_FOR_FM_TEST;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_STAR_AMFM;
		etaltest_freq = ETAL_VALID_FM_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ2;
		etaltest_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		etaltest_tune_conf = ETAL_TUNE_FM;
		etaltest_std = ETAL_TEST_MODE_FM;
	}
	else if (std == ETAL_BCAST_STD_HD_FM)
	{
		etaltest_handle = &handlehd;
		etaltest_fe_fg = ETAL_FE_HANDLE_1 /*ETAL_FE_FOR_HD_TEST*/;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_AUTO_HD;
		etaltest_freq = ETAL_VALID_HD_FREQ;
		etaltest_freq2 = ETAL_VALID_HD_FREQ2;
		etaltest_processing_features = (ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
		etaltest_tune_conf = ETAL_TUNE_HDRADIO_FM;
		etaltest_std = ETAL_TEST_MODE_HD_FM;
	}

	etalTestStartup();

	etalTestPrintNormal("<---VpaAfStart Test start");

	/*
	* Configure audio path
	*/
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%db failed: etal_config_audio_path (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
		

	/*
	* create an FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintNormal("* Configure %s VPA ON pf=0x%x", etalTestStandard2Ascii(std), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%da failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}

	/*
	* Configure audio source
	*/
	if ((ret = etal_audio_select(*etaltest_handle, etaltest_audio_src)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dc failed: etal_audio_select %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dd failed: etal_tune_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* AF start FM/HD receiver background
	*/
	etalTestPrintReportPassStart(FM_VPA_ON_AF_START_BG, etaltest_std, "VPA on, AF start BG");
	pass1 = TRUE;
	etalTestPrintNormal("* AF_start restart: on background channel freq: %d with Automatic antenna selected", etaltest_freq2);
	if ((ret = etal_AF_start(*etaltest_handle, cmdNormalMeasurement, etaltest_freq2, 0, &qualResult)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%de failed: etal_AF_start (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
	etalTestPrintVerboseContainer(&qualResult);
	if (qualResult.m_standard == ETAL_BCAST_STD_FM)
	{
		if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
			(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%df failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
				ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
		}
		last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
	}
	else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
	{
		if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
			(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%df failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
				ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
		}
		last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%df failed: etal_AF_start qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	etalTestPrintReportPassStart(FM_VPA_ON_AF_RESTART_BG, etaltest_std, "VPA on, AF restart BG");
	pass1 = TRUE;
	etalTestPrintNormal("* AF_start restart: on background channel freq: %d with Automatic antenna selected", etaltest_freq2);
	if ((ret = etal_AF_start(*etaltest_handle, cmdNormalMeasurement, etaltest_freq2, 0, &qualResult)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dg failed: etal_AF_start (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
	etalTestPrintVerboseContainer(&qualResult);
	if (qualResult.m_standard == ETAL_BCAST_STD_FM)
	{
		if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
			(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dh failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
				ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
		}
		last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
	}
	else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
	{
		if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
			(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dh failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
				ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
		}
		last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dh failed: etal_AF_start qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
	}

	last_RFFieldStrength = ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID;
	count = 0;
	while(count++ < 2)
	{
	/*
		* wait 1s
		*/
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		/* Test AF start on the background channel FM1/HD1 antenna, audio on foreground channel FM2/HD2 antenna */
		etalTestPrintNormal("* AF_start restart: on background channel freq: %d with %s1 antenna selected", etaltest_freq2, etalTestStandard2Ascii(attr.m_Standard));
		if ((ret = etal_AF_start(*etaltest_handle, cmdNormalMeasurement, etaltest_freq2, 2, &qualResult)) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%S VPA pass%di failed: etal_AF_start (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
		}
		etalTestPrintVerboseContainer(&qualResult);
		if (qualResult.m_standard == ETAL_BCAST_STD_FM)
		{
			if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
				(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dj failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
					ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
			}
			last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
		}
		else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
		{
			if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
				(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dj failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
					ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
			}
			last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
		}
		else
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dj failed: etal_AF_start qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
		}
	}

	last_RFFieldStrength = ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID;
	count = 0;
	while(count++ < 2)
	{
		/*
		* wait 1s
		*/
		OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

		/* Test AF start on the background channel FM2/HD2 antenna, audio on foreground channel FM1/HD1 antenna */
		etalTestPrintNormal("* AF_start restart: on background channel freq: %d with %s2 antenna selected", etaltest_freq2, etalTestStandard2Ascii(attr.m_Standard));
		if ((ret = etal_AF_start(*etaltest_handle, cmdNormalMeasurement, etaltest_freq2, 1, &qualResult)) != ETAL_RET_SUCCESS)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dk failed: etal_AF_start (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
		}
		etalTestPrintVerboseContainer(&qualResult);
		if (qualResult.m_standard == ETAL_BCAST_STD_FM)
		{
			if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
				(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dk failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
					ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
			}
			last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
		}
		else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
		{
			if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
				(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
			{
				pass1 = FALSE;
				etalTestPrintNormal("%s VPA pass%dk failed: etal_AF_start amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
					ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
			}
			last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
		}
		else
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dk failed: etal_AF_start qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
		}
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	/*
	* AF end FM receiver foreground
	*/
	etalTestPrintReportPassStart(FM_VPA_ON_AF_END_BG, etaltest_std, "VPA on, AF end BG");
	pass1 = TRUE;
	etalTestPrintNormal("* AF_end: on foreground channel freq: %d with Automatic antenna selected", etaltest_freq2);
	if ((ret = etal_AF_end(*etaltest_handle, etaltest_freq2, &qualResult)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintNormal("%s VPA pass%dm failed: etal_AF_end (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
		pass1 = FALSE;
	}
	etalTestPrintVerboseContainer(&qualResult);
	if (qualResult.m_standard == ETAL_BCAST_STD_FM)
	{
		if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
			(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dn failed: etal_AF_end amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.amfm.m_RFFieldStrength),
				ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
		}
		last_RFFieldStrength = qualResult.EtalQualityEntries.amfm.m_RFFieldStrength;
	}
	else if (qualResult.m_standard == ETAL_BCAST_STD_HD_FM)
	{
		if ((last_RFFieldStrength != ETAL_TEST_FM_RFFIELDSTRENGTH_INVALID) && 
			(abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength) > ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX))
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dn failed: etal_AF_end amfm.m_RFFieldStrength diff from previous:%d dBuV > %d dBuV", etalTestStandard2Ascii(attr.m_Standard), tcn, abs(last_RFFieldStrength - qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength),
				ETAL_TEST_FM_RFFIELDSTRENGTH_DIFF_MAX );
		}
		last_RFFieldStrength = qualResult.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dn failed: etal_AF_end qualResult.m_standard: %s", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(qualResult.m_standard));
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);

	/*
	* Undo tune FM/HD foreground
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/***************************
 *
 * etalTestVpaAfStartBgRds
 *
 **************************/
static tSInt etalTestVpaAfStartBgRds(tU8 tcn, EtalBcastStandard std, tU8 processing_features, tBool *pass_out)
{
	tBool pass1;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	EtalReceiverAttr attr;
	EtalAudioInterfTy audioIf;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE, hDatapath2 = ETAL_INVALID_HANDLE;
	EtalDataPathAttr dataPathAttr;
	EtalBcastQualityContainer qualResult;
	ETAL_HANDLE *etaltest_handle, *etaltest_handle2, etaltest_fe_fg = 0, etaltest_fe_bg = 0;
	EtalAudioSourceTy etaltest_audio_src;
	tU32 etaltest_freq, etaltest_freq2;
	tU8 etaltest_processing_features;
	etalTestTuneTy etaltest_tune_conf;
	etalTestBroadcastTy etaltest_std = ETAL_TEST_MODE_NONE;

	if ((std != ETAL_BCAST_STD_FM) && (std != ETAL_BCAST_STD_HD_FM))
	{
		*pass_out = FALSE;
		return OSAL_ERROR;
	}
	if (std == ETAL_BCAST_STD_FM)
	{
		etaltest_handle = &handlefm;
		etaltest_handle2 = &handlefm2;
		etaltest_fe_fg = ETAL_FE_FOR_FM_TEST;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_FM_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_STAR_AMFM;
		etaltest_freq = ETAL_VALID_FM_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ2;
		etaltest_processing_features = ETAL_PROCESSING_FEATURE_NONE;
		etaltest_tune_conf = ETAL_TUNE_FM;
		etaltest_std = ETAL_TEST_MODE_FM;
	}
	else if (std == ETAL_BCAST_STD_HD_FM)
	{
		etaltest_handle = &handlehd;
		etaltest_handle2 = &handlehd2;
		etaltest_fe_fg = ETAL_FE_HANDLE_1 /*ETAL_FE_FOR_HD_TEST*/;
		etaltest_fe_bg = ETAL_FE_HANDLE_2 /*ETAL_FE_FOR_HD_TEST2*/;
		etaltest_audio_src = ETAL_AUDIO_SOURCE_AUTO_HD;
		etaltest_freq = ETAL_VALID_FM_FREQ;
		etaltest_freq2 = ETAL_VALID_FM_FREQ2;
		etaltest_processing_features = (ETAL_PROCESSING_FEATURE_HD_RADIO_DRM_DIGITAL_BB_IF | ETAL_PROCESSING_FEATURE_HD_RADIO_ONCHIP_BLENDING);
		etaltest_tune_conf = ETAL_TUNE_HDRADIO_FM;
		etaltest_std = ETAL_TEST_MODE_HD_FM;
	}

	etalTestStartup();

	etalTestPrintNormal("<---VpaAfStartBgRds Test start");

	etalTestPrintReportPassStart(FM_VPA_ON_RDS_ON_FG, etaltest_std, "VPA on, RDS on FG");
	pass1 = TRUE;
	/*
	* create an FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintNormal("* Configure %s VPA ON pf=0x%x", etalTestStandard2Ascii(std), processing_features);

	/*
	* Configure audio path
	*/
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
	// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%db failed: etal_config_audio_path (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
		

	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%da failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}

	/*
	* Configure audio source
	*/
	if ((ret = etal_audio_select(*etaltest_handle, etaltest_audio_src)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dc failed: etal_audio_select %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dd failed: etal_tune_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* wait 1s
	*/
	OSAL_s32ThreadWait(ETAL_TEST_ONE_SECOND);
	/*
	* start RDS reception of FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Start RDS reception for %s", etalTestStandard2Ascii(attr.m_Standard));
	if ((ret = etal_start_RDS(*etaltest_handle, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%df failed: etal_start_RDS for %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	 * reset callback counter/data
	 */
	RdsRawCbInvocations = 0;
	OSAL_pvMemorySet(&rdsRawDatapathData, 0x00, sizeof(rdsRawDatapathData));
	/*
	* configure RDS datapath of FM/HD receiver foreground
	*/
	etalTestPrintNormal("* Config RDS raw datapath for %s", etalTestStandard2Ascii(attr.m_Standard));
	dataPathAttr.m_receiverHandle = *etaltest_handle;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS_RAW;
	dataPathAttr.m_sink.m_context = (tVoid *)(tULong)(*etaltest_handle);
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestVpaRDSRawCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dg failed: etal_config_datapath for %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Modify FM/HD receiver foreground with VPA OFF
	*/
	etalTestPrintNormal("* Reconfigure %s VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%di failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* create an FM/HD receiver background with VPA OFF
	*/
	etalTestPrintNormal("* Configure %s2 VPA OFF pf=0x%x", etalTestStandard2Ascii(attr.m_Standard), processing_features);
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 1;
	attr.m_FrontEnds[0] = etaltest_fe_bg;
	if (processing_features == ETAL_PROCESSING_FEATURE_FM_VPA) {
		attr.processingFeatures.u.m_processing_features = etaltest_processing_features;
	}
	else {
		attr.processingFeatures.u.m_processing_features = processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle2, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dk failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* Tune FM/HD receiver background
	*/
	etalTestPrintNormal("* Tune to %s2 freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle2);
	if (((ret = etal_tune_receiver(*etaltest_handle2, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dl failed: etal_tune_receiver %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s2 receiver with VPA OFF, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	/*
	* AF start FM/HD receiver background
	*/
	etalTestPrintNormal("* AF_start start: on background channel freq: %d", etaltest_freq2);
	if ((ret = etal_AF_start(*etaltest_handle2, cmdNormalMeasurement, etaltest_freq2, 0, &qualResult)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dm failed: etal_AF_start (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
	}
	/*
	* start RDS reception of FM2/HD2 receiver background
	*/
	etalTestPrintNormal("* Start RDS reception for %s2", etalTestStandard2Ascii(attr.m_Standard));
	if ((ret = etal_start_RDS(*etaltest_handle2, FALSE, 0, ETAL_RDS_MODE, 0)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dn failed: etal_start_RDS for %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	/*
	* reset callback counter/data
	*/
	RdsRawCbInvocations2 = 0;
	OSAL_pvMemorySet(&rdsRawDatapathData2, 0x00, sizeof(rdsRawDatapathData2));
	/*
	* configure RDS datapath of FM2/HD2 receiver background
	*/
	etalTestPrintNormal("* Config RDS raw datapath for %s2", etalTestStandard2Ascii(attr.m_Standard));
	dataPathAttr.m_receiverHandle = *etaltest_handle2;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS_RAW;
	dataPathAttr.m_sink.m_context = (tVoid *)(tULong)(*etaltest_handle2); // distinguish call from FM from FM2 in callback
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestVpaRDSRawCallback;
	if ((ret = etal_config_datapath(&hDatapath2, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%do failed: etal_config_datapath for %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}

	OSAL_s32ThreadWait(ETAL_TEST_GETRDS_FM_DURATION);

	/*
	* stop RDS reception and compare
	*/
	if ((ret = etal_destroy_datapath(&hDatapath2)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dr failed: etal_destroy_datapath for %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	if ((ret = etal_stop_RDS(*etaltest_handle2)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%ds failed: etal_stop_RDS for %s2 (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	if (RdsRawCbInvocations2 >= ETAL_TEST_GET_RDS_MIN)
	{
		if (etalTestGetRDSRawCompare(&rdsRawDatapathData2, &etalTestRDSRawReference, FALSE) != OSAL_OK)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dt failed: RDS data different from reference", etalTestStandard2Ascii(attr.m_Standard), tcn);
		}
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%du failed: complete, %d RDS callback invocations, expected more than %d", etalTestStandard2Ascii(attr.m_Standard), tcn, RdsRawCbInvocations2, ETAL_TEST_GET_RDS_MIN);
	}
	/*
	* AF end FM/HD receiver background
	*/
	etalTestPrintNormal("* AF_end: on background channel freq: %d", etaltest_freq2);
	if ((ret = etal_AF_end(*etaltest_handle2, etaltest_freq2, &qualResult)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintNormal("%s VPA pass%dv failed: etal_AF_end (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, ETAL_STATUS_toString(ret), ret);
		pass1 = FALSE;
	}

	/*
	 * stop RDS reception and compare
	 */
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dx failed: etal_destroy_datapath for %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	if ((ret = etal_stop_RDS(*etaltest_handle)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dy failed: etal_stop_RDS for %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	if (RdsRawCbInvocations >= ETAL_TEST_GET_RDS_MIN)
	{
		if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, FALSE) != OSAL_OK)
		{
			pass1 = FALSE;
			etalTestPrintNormal("%s VPA pass%dz failed: RDS data different from reference", etalTestStandard2Ascii(attr.m_Standard), tcn);
		}
	}
	else
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%daa failed: complete, %d RDS callback invocations, expected more than %d", etalTestStandard2Ascii(attr.m_Standard), tcn, RdsRawCbInvocations, ETAL_TEST_GET_RDS_MIN);
	}

	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo tune FM2/HD2 background
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM2/HD2 receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Modify FM/HD receiver foreground and background with VPA ON
	*/
	etalTestPrintReportPassStart(FM_VPA_RECONF_VPA_ON, etaltest_std, "Reconf VPA ON pf=0x%x", processing_features);
	pass1 = TRUE;
	OSAL_pvMemorySet(&attr, 0x00, sizeof(attr));
	attr.m_Standard = std;
	attr.m_FrontEndsSize = 2;
	attr.m_FrontEnds[0] = etaltest_fe_fg;
	attr.m_FrontEnds[1] = etaltest_fe_bg;
	attr.processingFeatures.u.m_processing_features = processing_features;
	if ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (processing_features != ETAL_PROCESSING_FEATURE_UNSPECIFIED))
	{
		attr.processingFeatures.u.m_processing_features |= etaltest_processing_features;
	}
	if ((ret = etal_config_receiver(etaltest_handle, &attr)) != ETAL_RET_SUCCESS)
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dab failed: etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	etalTestPrintNormal("* Tune to %s freq %d handle %d", etalTestStandard2Ascii(attr.m_Standard), etaltest_freq, *etaltest_handle);
	if (((ret = etal_tune_receiver(*etaltest_handle, etaltest_freq)) != ETAL_RET_SUCCESS) &&
		((attr.m_Standard == ETAL_BCAST_STD_FM) || ((attr.m_Standard == ETAL_BCAST_STD_HD_FM) && (ret != ETAL_RET_NO_DATA))))
	{
		pass1 = FALSE;
		etalTestPrintNormal("%s VPA pass%dac failed: etal_tune_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), tcn, etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintNormal("* Tuned %s receiver with VPA ON, handle %d", etalTestStandard2Ascii(attr.m_Standard), *etaltest_handle);
	}
	if (!pass1)
	{
		*pass_out = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	* Undo tune FM/HD foreground
	*/
	if (etalTestUndoTuneSingle(etaltest_tune_conf, *etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	* Undo FM/HD receiver configuration
	*/
	if (etalTestUndoConfigSingle(etaltest_handle) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}
#endif
#endif //defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)

#endif // CONFIG_APP_TEST_FM_VPA

/***************************
 *
 * etalTestFmVpa
 *
 **************************/
tSInt etalTestFmVpa(void)
{
#ifdef CONFIG_APP_TEST_FM_VPA
	tBool pass;

	etalTestStartup();

	pass = TRUE;
	if ((etalTestVpaConfig(1, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaConfig(1, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#if defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL) /*&& !defined(CONFIG_APP_TEST_DAB)*/
#if defined(CONFIG_APP_TEST_FM)
	if ((etalTestVpaTune(2, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaTune(2, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif
#if defined(CONFIG_APP_TEST_HDRADIO_FM)
	if ((etalTestVpaTune(2, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaTune(2, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif

#if defined(CONFIG_APP_TEST_FM)
	if ((etalTestVpaAfCheck(3, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaAfCheck(3, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif
#if defined(CONFIG_APP_TEST_HDRADIO_FM)
	if ((etalTestVpaAfCheck(3, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaAfCheck(3, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif

#if defined(CONFIG_APP_TEST_FM)
	if ((etalTestVpaFgActivity(4, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaFgActivity(4, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif
#if defined(CONFIG_APP_TEST_HDRADIO_FM)
	if ((etalTestVpaFgActivity(4, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaFgActivity(4, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif

#if defined(CONFIG_APP_TEST_FM)
	if ((etalTestVpaAfStart(5, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaAfStart(5, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif
#if defined(CONFIG_APP_TEST_HDRADIO_FM)
	if ((etalTestVpaAfStart(5, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) != OSAL_OK) || (etalTestVpaAfStart(5, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) != OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif

#if defined(CONFIG_APP_TEST_FM) && defined (CONFIG_ETAL_HAVE_ETALTML)
	if ((etalTestVpaAfStartBgRds(6, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) !=  OSAL_OK) || (etalTestVpaAfStartBgRds(6, ETAL_BCAST_STD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) !=  OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif
#if defined(CONFIG_APP_TEST_HDRADIO_FM) && defined (CONFIG_ETAL_HAVE_ETALTML)
	if ((etalTestVpaAfStartBgRds(6, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_FM_VPA, &pass) !=  OSAL_OK) || (etalTestVpaAfStartBgRds(6, ETAL_BCAST_STD_HD_FM, ETAL_PROCESSING_FEATURE_UNSPECIFIED, &pass) !=  OSAL_OK))
	{
		return OSAL_ERROR;
	}
#endif

#endif // defined(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM_VPA
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

