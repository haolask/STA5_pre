//!
//!  \file 		etaltest_quality.c
//!  \brief 	<i><b> ETAL test, quality readout </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

/*
 * Empirical  values
 * The min value is just a guess.
 */


/* Using the signal generator we get the max quality */
#define ETAL_TEST_MAX_HD_QI              15
#define ETAL_TEST_HD_QI_THRESHOLD         8
#define ETAL_TEST_MIN_HD_QI               1

#define ETAL_TEST_DAB_MONITOR_CONTEXT   ((tVoid *)0xDAB01234)
#define ETAL_TEST_FM_MONITOR_CONTEXT    ((tVoid *)0xFF005678)
#define ETAL_TEST_HD_MONITOR_CONTEXT    ((tVoid *)0xDD009ABC)
#define ETAL_TEST_AM_MONITOR_CONTEXT    ((tVoid *)0xAA009ABC)

#define SET_MONITOR_DAB1_MIN 2
#define SET_MONITOR_DAB1_MAX 10
#define SET_MONITOR_FM1_MIN 2
#define SET_MONITOR_FM1_MAX 10
#define SET_MONITOR_HD_MIN 2
#define SET_MONITOR_HD_MAX 10
#define SET_MONITOR_AM_MIN 2
#define SET_MONITOR_AM_MAX 10

#ifdef CONFIG_APP_TEST_GETQUALITY
typedef enum {
	GET_QUAL_GET_RECEP_QUAL = 1,
	GET_QUAL_CHECK_RECEP_QUAL,
	GET_QUAL_CHECK_NULL_RECEP_QUAL_FIELDS,
	GET_QUAL_GET_CHANNEL_QUAL,
	GET_QUAL_CHECK_CHANNEL_QUAL,
	GET_QUAL_CHECK_NULL_CHANNEL_QUAL_FIELDS,
	GET_QUAL_HD_GET_RECEP_QUAL_NOT_TUNED,
	GET_QUAL_HD_CHECK_DIGITAL_QI_QUAL,
	GET_QUAL_HD_GET_DIGITAL_RECEP_QUAL,
	GET_QUAL_HD_CHECK_DIGITAL_DSQM_QUAL
} GetQualityTestTy;
#endif

#ifdef CONFIG_APP_TEST_SETMONITOR
typedef enum {
	SET_MONITOR_CREATE_NULL_ATTR = 1,
	SET_MONITOR_CREATE_INVALID_HANDLE,
	SET_MONITOR_CREATE_NULL_CB,
	SET_MONITOR_CREATE_INVALID_ATTR,
	SET_MONITOR_CREATE_XMS_INTERVAL,
	SET_MONITOR_CHECK_XMS_INVOC,
	SET_MONITOR_CHECK_XMS_INVOC_CTXT,
	SET_MONITOR_MODIFY_FM_MONIT,
	SET_MONITOR_CHECK_INVOC,
	SET_MONITOR_DESTROY
} SetMonitorTestTy;
#endif

#if defined(CONFIG_APP_TEST_GETCFQUALITY)
typedef enum {
	GET_CFQUAL_GET_CF_INVALID_STD = 1,
	GET_CFQUAL_GET_CF,
	GET_CFQUAL_CHECK_CF,
	GET_CFQUAL_VERIFY_AM,
	GET_CFQUAL_GET_CF_INVALID_CONTAINER,
	GET_CFQUAL_GET_CF_INVALID_AVG,
	GET_CFQUAL_GET_CF_INVALID_SAMPLE_TIME
} GetCFQualityTestTy;
#endif

/*****************************************************************
| variable defintion (scope: module-local)
|----------------------------------------------------------------*/
#if defined (CONFIG_APP_TEST_CONFIG_RECEIVER) || defined (CONFIG_APP_TEST_DESTROY_RECEIVER) || defined (CONFIG_APP_TEST_SETMONITOR)
etalTestMonitorCounterTy QCount;
#endif

#if defined (CONFIG_APP_TEST_CONFIG_RECEIVER) || defined (CONFIG_APP_TEST_DESTROY_RECEIVER) || defined (CONFIG_APP_TEST_SETMONITOR)
/***************************
 *
 * etalTestMonitorcb
 *
 **************************/
void etalTestMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext)
{
	if (pQuality->m_standard == ETAL_BCAST_STD_DAB)
	{
		QCount.DabCbInvocations++;
		if (vpContext == ETAL_TEST_DAB_MONITOR_CONTEXT)
		{
			QCount.DabValidContext++;
		}
	}
	else if (pQuality->m_standard == ETAL_BCAST_STD_FM)
	{
		QCount.FmCbInvocations++;
		if ((pQuality->EtalQualityEntries.amfm.m_RFFieldStrength < ETAL_TEST_MAX_FM_FIELDSTRENGTH) &&
			(vpContext == ETAL_TEST_FM_MONITOR_CONTEXT))
		{
			QCount.FmValidContext++;
		}
	}
	else if (pQuality->m_standard == ETAL_BCAST_STD_AM)
	{
		QCount.AmCbInvocations++;
		if ((pQuality->EtalQualityEntries.amfm.m_RFFieldStrength < ETAL_TEST_MAX_AM_FIELDSTRENGTH) &&
			(vpContext == ETAL_TEST_AM_MONITOR_CONTEXT))
		{
			QCount.AmValidContext++;
		}
	}
	else if (pQuality->m_standard == ETAL_BCAST_STD_HD_FM)
	{
		QCount.HdCbInvocations++;
		// TODO add check on RF strength to validate the invocation
		if (vpContext == ETAL_TEST_HD_MONITOR_CONTEXT)
		{
			QCount.HdValidContext++;
		}
	}
	else if (pQuality->m_standard == ETAL_BCAST_STD_HD_AM)
	{
		QCount.HdamCbInvocations++;
		// TODO add check on RF strength to validate the invocation
		if (vpContext == ETAL_TEST_HD_MONITOR_CONTEXT)
		{
			QCount.HdamValidContext++;
		}
	}
	etalTestPrintVerboseContainer(pQuality);
}
#endif // #if defined (CONFIG_APP_TEST_CONFIG_RECEIVER) || (CONFIG_APP_TEST_DESTROY_RECEIVER) || defined (CONFIG_APP_TEST_SETMONITOR)

#ifdef CONFIG_APP_TEST_GETQUALITY
#if defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_AM) || defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
/***************************
 *
 * etalTestGetQuality_internal
 *
 **************************/
static tSInt etalTestGetQuality_internal(etalTestBroadcastTy test_mode, tBool *pass)
{
	EtalBcastQualityContainer qual;
	tBool pass_l = TRUE;
	tSInt count;
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	ETAL_HANDLE *phReceiver;
	tS32 *measure, *measure_ch,measure_lower, measure_upper, measure_lower_ch, measure_upper_ch; 
	tU32 settle_time;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_DAB:
			config_mode = ETAL_CONFIG_DAB;
			tune_mode = ETAL_TUNE_DAB;
			phReceiver = &handledab;
			measure =    &qual.EtalQualityEntries.dab.m_RFFieldStrength;
			measure_ch = &qual.EtalQualityEntries.dab.m_BBFieldStrength;
			measure_lower    = ETAL_TEST_MIN_DAB_RFFIELDSTRENGTH;
			measure_lower_ch = ETAL_TEST_MIN_DAB_BBFIELDSTRENGTH; // unused
			measure_upper    = ETAL_TEST_MAX_DAB_RFFIELDSTRENGTH;
			measure_upper_ch = ETAL_TEST_MAX_DAB_BBFIELDSTRENGTH; // unused
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME;
			break;

		case ETAL_TEST_MODE_FM:
			config_mode = ETAL_CONFIG_FM1;
			tune_mode = ETAL_TUNE_FM;
			phReceiver = &handlefm;
			measure =    &qual.EtalQualityEntries.amfm.m_RFFieldStrength;
			measure_ch = &qual.EtalQualityEntries.amfm.m_RFFieldStrength;
			measure_lower    = ETAL_TEST_MIN_FM_FIELDSTRENGTH;
			measure_lower_ch = ETAL_TEST_MIN_FM_FIELDSTRENGTH;
			measure_upper    = ETAL_TEST_MAX_FM_FIELDSTRENGTH;
			measure_upper_ch = ETAL_TEST_MAX_FM_FIELDSTRENGTH;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME;
			break;

		case ETAL_TEST_MODE_AM:
			config_mode = ETAL_CONFIG_AM;
			tune_mode = ETAL_TUNE_AM;
			phReceiver = &handleam;
			measure =    &qual.EtalQualityEntries.amfm.m_RFFieldStrength;
			measure_ch = &qual.EtalQualityEntries.amfm.m_RFFieldStrength;
			measure_lower    = ETAL_TEST_MIN_AM_FIELDSTRENGTH;
			measure_lower_ch = ETAL_TEST_MIN_AM_FIELDSTRENGTH;
			measure_upper    = ETAL_TEST_MAX_AM_FIELDSTRENGTH;
			measure_upper_ch = ETAL_TEST_MAX_AM_FIELDSTRENGTH;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME_AM;
			break;

		case ETAL_TEST_MODE_HD_FM:
			config_mode = ETAL_CONFIG_HDRADIO_FM;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			phReceiver = &handlehd;
			measure = (tS32*) &qual.EtalQualityEntries.hd.m_QI;
			measure_ch =      &qual.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			measure_lower    = ETAL_TEST_HD_QI_THRESHOLD;
			measure_lower_ch = ETAL_TEST_MIN_FM_FIELDSTRENGTH;
			measure_upper    = ETAL_TEST_MAX_HD_QI;
			measure_upper_ch = ETAL_TEST_MAX_FM_FIELDSTRENGTH;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME;
			break;

		case ETAL_TEST_MODE_HD_AM:
			config_mode = ETAL_CONFIG_HDRADIO_AM;
			tune_mode = ETAL_TUNE_HDRADIO_AM;
			phReceiver = &handlehdam;
			measure = (tS32*) &qual.EtalQualityEntries.hd.m_QI;
			measure_ch =      &qual.EtalQualityEntries.hd.m_analogQualityEntries.m_RFFieldStrength;
			measure_lower    = ETAL_TEST_HD_QI_THRESHOLD;
			measure_lower_ch = ETAL_TEST_MIN_AM_FIELDSTRENGTH;
			measure_upper    = ETAL_TEST_MAX_HD_QI;
			measure_upper_ch = ETAL_TEST_MAX_AM_FIELDSTRENGTH;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME_AM;
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

	count = 1;
	while (count <= 3)
	{
		OSAL_pvMemorySet(&qual, 0x00, sizeof(EtalBcastQualityContainer));

		if (count == 1)
		{
			OSAL_s32ThreadWait(settle_time);
		}

		etalTestPrintReportPassStart(GET_QUAL_GET_RECEP_QUAL, test_mode, "Get Reception Quality, loop %d", count);
		if (etal_get_reception_quality(*phReceiver, &qual) != ETAL_RET_SUCCESS)
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass1a FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);

			etalTestPrintReportPassStart(GET_QUAL_CHECK_RECEP_QUAL, test_mode, "Check Reception Quality, loop %d", count);
			if ((*measure < measure_lower) || (*measure > measure_upper)) 
			{
				pass_l = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass1b FAILED (%d)", *measure);
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}

			if (test_mode == ETAL_TEST_MODE_AM)
			{
				/* check if fields not available in AM are set to 0 */
				etalTestPrintReportPassStart(GET_QUAL_CHECK_NULL_RECEP_QUAL_FIELDS, test_mode, "Check null Reception quality fields, loop %d", count);
				if ((qual.EtalQualityEntries.amfm.m_Multipath != 0) ||
					(qual.EtalQualityEntries.amfm.m_UltrasonicNoise != 0) ||
					(qual.EtalQualityEntries.amfm.m_SNR != 0) ||
					(qual.EtalQualityEntries.amfm.m_coChannel != 0) ||
					(qual.EtalQualityEntries.amfm.m_StereoMonoReception != 0))
				{
					pass_l = FALSE;
					etalTestPrintReportPassEnd(testFailed);
					etalTestPrintNormal("pass1c FAILED");
				}
				else
				{
					etalTestPrintReportPassEnd(testPassed);
				}
			}

		}

		count++;
	}

	/* repeat the test for the Channel quality */
	// channel quality tests are not valid for DAB standard
	// 

	if (test_mode != ETAL_TEST_MODE_DAB)
	{
		count = 1;
		while (count <= 3)
		{
			OSAL_pvMemorySet(&qual, 0x00, sizeof(EtalBcastQualityContainer));

			etalTestPrintReportPassStart(GET_QUAL_GET_CHANNEL_QUAL, test_mode, "Get Channel Quality, loop %d", count);
			if (etal_get_channel_quality(*phReceiver, &qual) != ETAL_RET_SUCCESS)
			{
				pass_l = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass2a FAILED");
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);

				etalTestPrintReportPassStart(GET_QUAL_CHECK_CHANNEL_QUAL, test_mode, "Check Channel Quality, loop %d", count);
				if (((test_mode != ETAL_TEST_MODE_DAB) && ((*measure < measure_lower) || (*measure > measure_upper))) ||
					((test_mode != ETAL_TEST_MODE_DAB) && ((*measure_ch < measure_lower_ch) || (*measure_ch > measure_upper_ch))))
				{
					pass_l = FALSE;
					etalTestPrintReportPassEnd(testFailed);
					etalTestPrintNormal("Measure: %d, Min: %d, Max: %d", *measure, measure_lower, measure_upper);
					etalTestPrintNormal("Measure: %d, Min: %d, Max: %d", *measure_ch, measure_lower_ch, measure_upper_ch);
					etalTestPrintNormal("pass2b FAILED");
				}
				else
				{
					etalTestPrintReportPassEnd(testPassed);
				}

				if (test_mode == ETAL_TEST_MODE_AM)
				{
					/* check if fields not available in AM are set to 0 */
					etalTestPrintReportPassStart(GET_QUAL_CHECK_NULL_CHANNEL_QUAL_FIELDS, test_mode, "Check null Channel quality fields, loop %d", count);
					if ((qual.EtalQualityEntries.amfm.m_Multipath != 0) ||
						(qual.EtalQualityEntries.amfm.m_UltrasonicNoise != 0) ||
						(qual.EtalQualityEntries.amfm.m_SNR != 0) ||
						(qual.EtalQualityEntries.amfm.m_coChannel != 0) ||
						(qual.EtalQualityEntries.amfm.m_StereoMonoReception != 0))
					{
						pass_l = FALSE;
						etalTestPrintReportPassEnd(testFailed);
						etalTestPrintNormal("pass2c FAILED");
					}
					else
					{
						etalTestPrintReportPassEnd(testPassed);
					}
				}
			}

			count++;
		}
	}
	else
	{
		// for DAB standard, check it is rejected
		//
		OSAL_pvMemorySet(&qual, 0x00, sizeof(EtalBcastQualityContainer));

		etalTestPrintReportPassStart(GET_QUAL_GET_CHANNEL_QUAL, test_mode, "Get Channel Quality");
		if (etal_get_channel_quality(*phReceiver, &qual) != ETAL_RET_INVALID_BCAST_STANDARD)
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass2dab FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	
	/*
	 * Additional tests for HD
	 */
	if ((test_mode == ETAL_TEST_MODE_HD_FM) || (test_mode == ETAL_TEST_MODE_HD_AM))
	{
		/* 
		 * Immediately after the tune to empty freq 
	 	 * (as done by etalTestUndoTuneSingle) the QI should be 0 (no digital audio)
		 */ 	
		if (etalTestUndoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		OSAL_pvMemorySet(&qual, 0x00, sizeof(EtalBcastQualityContainer));

		etalTestPrintReportPassStart(GET_QUAL_HD_GET_RECEP_QUAL_NOT_TUNED, test_mode, "Get Reception Quality with no frequency tuned");
		if (etal_get_reception_quality(*phReceiver, &qual) != ETAL_RET_SUCCESS)
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass3a FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);

			etalTestPrintReportPassStart(GET_QUAL_HD_CHECK_DIGITAL_QI_QUAL, test_mode, "Check digital QI Quality");
			etalTestPrintVerboseContainer(&qual);
			if (qual.EtalQualityEntries.hd.m_QI > 0)
			{
				pass_l = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass3b FAILED");
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}

		/*
		 * Now check the DSQM value
		 *
		 * For HD AM the expected DSQM is 0
		 */
		OSAL_pvMemorySet(&qual, 0x00, sizeof(EtalBcastQualityContainer));

		if (etalTestDoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		etalTestPrintNormal("* Wait some time for DSQM to settle (5s)");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintReportPassStart(GET_QUAL_HD_GET_DIGITAL_RECEP_QUAL, test_mode, "Get digital Reception Quality");
		if (etal_get_reception_quality(*phReceiver, &qual) != ETAL_RET_SUCCESS)
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass3c FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);

			etalTestPrintReportPassStart(GET_QUAL_HD_CHECK_DIGITAL_DSQM_QUAL, test_mode, "Check digital DSQM Quality");
			etalTestPrintVerboseContainer(&qual);
			if (((test_mode == ETAL_TEST_MODE_HD_FM) && (qual.EtalQualityEntries.hd.m_DSQM == 0)) ||
				((test_mode == ETAL_TEST_MODE_HD_AM) && (qual.EtalQualityEntries.hd.m_DSQM != 0)))
			{
				pass_l = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass3d FAILED");
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}
	}

	if (etalTestUndoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass_l)
	{
		*pass = FALSE;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DAB || CONFIG_APP_TEST_FM || CONFIG_APP_TEST_HDRADIO_FM
#endif // CONFIG_APP_TEST_GETQUALITY

/***************************
 *
 * etalTestGetQuality
 *
 **************************/
/*
 * Read the quality, one-shot, and verify if some fields
 * are within predefined range.
 * Note: channel quality and CF quality for HD is returned in an
 * FM container
 */
tSInt etalTestGetQuality(void)
{
#ifdef CONFIG_APP_TEST_GETQUALITY
	tBool pass;

	etalTestStartup();
	pass = TRUE;

#if defined (CONFIG_APP_TEST_DAB)
	if (etalTestGetQuality_internal(ETAL_TEST_MODE_DAB, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_FM)
	if (etalTestGetQuality_internal(ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	if (etalTestGetQuality_internal(ETAL_TEST_MODE_HD_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_HDRADIO_AM)
	if (etalTestGetQuality_internal(ETAL_TEST_MODE_HD_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif
#if defined (CONFIG_APP_TEST_AM)
	if (etalTestGetQuality_internal(ETAL_TEST_MODE_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_GETQUALITY
	return OSAL_OK;
}

#ifdef CONFIG_APP_TEST_SETMONITOR
/***************************
 *
 * etalTestSetMonitor_internal
 *
 **************************/
/*
 * Pass1:
 * Set up some monitors and verify if they are hit a predefined number of times
 *
 * Pass2, Pass3:
 * Same as Pass1 but with different monitor intervals.
 *
 * Pass4:
 * Set up a monitor for FM and HD quantities
 */
static tSInt etalTestSetMonitor_internal(etalTestBroadcastTy test_mode, tBool *pass)
{
	EtalBcastQualityMonitorAttr monitor_attr;
	tBool pass_l = TRUE;
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	ETAL_HANDLE *phReceiver;
	tU32 settle_time;
	tVoid *context;
	EtalBcastQaIndicators indicator;
	tS32 monitor_lower, monitor_upper;
	tU32 count_lower, count_upper;
	ETAL_HANDLE monitor = ETAL_INVALID_HANDLE;
	tU32 *cb_count, *cb_valid_count;
	tU32 loop, interval, divisor;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_DAB:
			config_mode = ETAL_CONFIG_DAB;
			tune_mode = ETAL_TUNE_DAB;
			phReceiver = &handledab;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME;
			context = ETAL_TEST_DAB_MONITOR_CONTEXT;
			indicator = EtalQualityIndicator_DabFieldStrength;
			monitor_lower = ETAL_TEST_MIN_DAB_RFFIELDSTRENGTH;
			monitor_upper = ETAL_TEST_MAX_DAB_RFFIELDSTRENGTH;
			count_lower = SET_MONITOR_DAB1_MIN;
			count_upper = SET_MONITOR_DAB1_MAX;
			cb_count =       &QCount.DabCbInvocations;
			cb_valid_count = &QCount.DabValidContext;
			break;

		case ETAL_TEST_MODE_FM:
			config_mode = ETAL_CONFIG_FM1;
			tune_mode = ETAL_TUNE_FM;
			phReceiver = &handlefm;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME;
			context = ETAL_TEST_FM_MONITOR_CONTEXT;
			indicator = EtalQualityIndicator_FmFieldStrength;
			monitor_lower = 5;
			monitor_upper = ETAL_INVALID_MONITOR;
			count_lower = SET_MONITOR_FM1_MIN;
			count_upper = SET_MONITOR_FM1_MAX;
			cb_count =       &QCount.FmCbInvocations;
			cb_valid_count = &QCount.FmValidContext;
			break;

		case ETAL_TEST_MODE_AM:
			config_mode = ETAL_CONFIG_AM;
			tune_mode = ETAL_TUNE_AM;
			phReceiver = &handleam;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME_AM;
			context = ETAL_TEST_AM_MONITOR_CONTEXT;
			indicator = EtalQualityIndicator_FmFieldStrength;
			monitor_lower = ETAL_TEST_MIN_AM_FIELDSTRENGTH;
			monitor_upper = ETAL_INVALID_MONITOR;
			count_lower = SET_MONITOR_AM_MIN;
			count_upper = SET_MONITOR_AM_MAX;
			cb_count =       &QCount.AmCbInvocations;
			cb_valid_count = &QCount.AmValidContext;
			break;

		case ETAL_TEST_MODE_HD_FM:
			config_mode = ETAL_CONFIG_HDRADIO_FM;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			phReceiver = &handlehd;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME;
			context = ETAL_TEST_HD_MONITOR_CONTEXT;
			indicator = EtalQualityIndicator_HdQI;
			monitor_lower = 5;
			monitor_upper = ETAL_INVALID_MONITOR;
			count_lower = SET_MONITOR_HD_MIN;
			count_upper = SET_MONITOR_HD_MAX;
			cb_count =       &QCount.HdCbInvocations;
			cb_valid_count = &QCount.HdValidContext;
			break;

		case ETAL_TEST_MODE_HD_AM:
			config_mode = ETAL_CONFIG_HDRADIO_AM;
			tune_mode = ETAL_TUNE_HDRADIO_AM;
			phReceiver = &handlehdam;
			settle_time = ETAL_TEST_CMOST_QUALITY_SETTLE_TIME_AM;
			context = ETAL_TEST_HD_MONITOR_CONTEXT;
			indicator = EtalQualityIndicator_HdQI;
			monitor_lower = 5;
			monitor_upper = ETAL_INVALID_MONITOR;
			count_lower = SET_MONITOR_HD_MIN;
			count_upper = SET_MONITOR_HD_MAX;
			cb_count =       &QCount.HdamCbInvocations;
			cb_valid_count = &QCount.HdamValidContext;
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

	OSAL_s32ThreadWait(settle_time);

	/* pass1 interval = 1000ms
	 * pass2 interval =  500ms
	 * pass2 interval =  250ms*/

	interval = ETAL_TEST_ONE_SECOND;
	divisor = 1;
#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	/* The external driver requires a long delay between command
	 * to DCOP and DCOP answer (~300ms) thus passes with interval
	 * shorter than 1000ms would need to be reviewed.
	 * In addition, there is a high risk of resource starvation
	 * (see email "ETAL resource starvation?" dated 16/2/2017) */
	loop = 1;
#else
	for (loop = 1; loop <= 3 ; loop++)
#endif
	{
		etalTestPrintReportPassStart(SET_MONITOR_CREATE_XMS_INTERVAL, test_mode, "Create Monitor with %dms interval", interval);
		OSAL_pvMemorySet(&monitor_attr, 0x00, sizeof(EtalBcastQualityMonitorAttr));
		monitor_attr.m_receiverHandle = *phReceiver;
		monitor_attr.m_Context = context;
		monitor_attr.m_CbBcastQualityProcess = etalTestMonitorcb;
		monitor_attr.m_monitoredIndicators[0].m_MonitoredIndicator = indicator;
		monitor_attr.m_monitoredIndicators[0].m_InferiorValue = monitor_lower;
		monitor_attr.m_monitoredIndicators[0].m_SuperiorValue = monitor_upper;
		monitor_attr.m_monitoredIndicators[0].m_UpdateFrequency = interval;
		if (etal_config_reception_quality_monitor(&monitor, &monitor_attr) != ETAL_RET_SUCCESS)
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%da FAILED", loop);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		OSAL_pvMemorySet((tVoid *)&QCount, 0x00, sizeof(etalTestMonitorCounterTy));

		etalTestPrintNormal("* Waiting 5s for responses to hit the callbacks...");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);


		etalTestPrintNormal("= pass%d complete, %d DAB, %d FM, %d AM, %d HDFM and %d HDAM callback invocations", loop, QCount.DabCbInvocations, QCount.FmCbInvocations, QCount.AmCbInvocations, QCount.HdCbInvocations, QCount.HdamCbInvocations);
		etalTestPrintNormal("= (expected more than %d, less than %d)", count_lower * divisor, count_upper * divisor);

		etalTestPrintReportPassStart(SET_MONITOR_CHECK_XMS_INVOC, test_mode, "Check %dms Monitor invocations", interval);
		if ((*cb_count > count_lower * divisor) && (*cb_count < count_upper * divisor))
		{
			etalTestPrintReportPassEnd(testPassed);
		}
		else
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%de FAILED, receiver %d", loop, *cb_count);
		}

		etalTestPrintReportPassStart(SET_MONITOR_CHECK_XMS_INVOC_CTXT, test_mode, "Check %dms Monitor invocations, context", interval);
		if ((*cb_valid_count > count_lower * divisor) && (*cb_valid_count < count_upper * divisor))
		{
			etalTestPrintReportPassEnd(testPassed);
		}
		else
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%df FAILED received %d", loop, *cb_valid_count);
		}

		divisor *= 2;
		interval = ETAL_TEST_ONE_SECOND / divisor;
	}

	/* additionally for HD, modify the monitor to also check the analogue quality */
	if ((test_mode == ETAL_TEST_MODE_HD_FM) || (test_mode == ETAL_TEST_MODE_HD_AM))
	{
		OSAL_pvMemorySet((tVoid *)&QCount, 0x00, sizeof(etalTestMonitorCounterTy));

		/* the HD monitor is already created (m_monitoredIndicators[0]), create a monitor on HD analog quality results */
		etalTestPrintReportPassStart(SET_MONITOR_MODIFY_FM_MONIT, test_mode, "Modify FM Monitor");
		monitor_attr.m_monitoredIndicators[1].m_MonitoredIndicator = EtalQualityIndicator_FmFieldStrength;
		monitor_attr.m_monitoredIndicators[1].m_InferiorValue = 5;
		monitor_attr.m_monitoredIndicators[1].m_SuperiorValue = ETAL_INVALID_MONITOR;
		monitor_attr.m_monitoredIndicators[1].m_UpdateFrequency = ETAL_TEST_ONE_SECOND;
		if (etal_config_reception_quality_monitor(&monitor, &monitor_attr) != ETAL_RET_SUCCESS)
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass4a FAILED");
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		etalTestPrintNormal("* Waiting 5s for responses to hit the callbacks...");
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

		etalTestPrintNormal("= pass4 complete, %d DAB, %d FM and %d HD callback invocations", QCount.DabCbInvocations, QCount.FmCbInvocations, QCount.HdCbInvocations);
		etalTestPrintNormal("= (expected 0 for DAB,");
		etalTestPrintNormal("= (expected 0 for FM,");
		etalTestPrintNormal("=           more than %d, less than %d for HD)", SET_MONITOR_HD_MIN * 4, SET_MONITOR_HD_MAX * 4 + 5);

		etalTestPrintReportPassStart(SET_MONITOR_CHECK_INVOC, test_mode, "Check Monitor invocations");
		/* expect in 5s at least:
		 * 20 for the 250ms monitor previously created, plus
		 *  4 for the 1s monitor just created */
		if ((QCount.DabCbInvocations != 0) ||
			(QCount.FmCbInvocations != 0) ||
			((*cb_count <= (SET_MONITOR_HD_MIN * 4)) || (*cb_count >= (SET_MONITOR_HD_MAX *4 + 5))))
		{
			pass_l = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass4b FAILED received %d", *cb_count);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	/* final pass, cleanup */

	etalTestPrintReportPassStart(SET_MONITOR_DESTROY, test_mode, "Destroy Monitor");
	if (etal_destroy_reception_quality_monitor(&monitor) != ETAL_RET_SUCCESS)
	{
		pass_l = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass5a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (etalTestUndoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass_l)
	{
		*pass = FALSE;
	}
	return OSAL_OK;
}


/***************************
 *
 * etalTestCheckParams
 *
 **************************/
static tSInt etalTestCheckParams(tBool *pass)
{
	ETAL_STATUS ret;
	EtalBcastQualityMonitorAttr mon;
	ETAL_HANDLE hMonitor;
	ETAL_HANDLE hReceiver;

	etalTestStartup();

	etalTestPrintReportPassStart(SET_MONITOR_CREATE_NULL_ATTR, ETAL_TEST_MODE_NONE, "Create monitor with NULL attribute parameter");
	hMonitor = ETAL_INVALID_HANDLE;
	if (etal_config_reception_quality_monitor(&hMonitor, NULL) != ETAL_RET_PARAMETER_ERR)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_pvMemorySet(&mon, 0x00, sizeof(EtalBcastQualityMonitorAttr));

	etalTestPrintReportPassStart(SET_MONITOR_CREATE_INVALID_HANDLE, ETAL_TEST_MODE_NONE, "Create monitor with invalid handle parameter");
	mon.m_receiverHandle = ETAL_INVALID_HANDLE;
	if ((ret = etal_config_reception_quality_monitor(&hMonitor, &mon)) != ETAL_RET_INVALID_HANDLE)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1b FAILED ret = %d", ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* Create a valid Receiver handle for the next test */
	/* Try to cope with all possible test configs */
#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	hReceiver = handledab;
#elif CONFIG_APP_TEST_FM //CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	hReceiver = handlefm;
#elif CONFIG_APP_TEST_HDRADIO_FM //CONFIG_APP_TEST_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	hReceiver = handlehd;
#elif CONFIG_APP_TEST_HDRADIO_AM //CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	hReceiver = handlehdam;
#else // CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	hReceiver = handleam;
#endif //CONFIG_APP_TEST_AM

	etalTestPrintReportPassStart(SET_MONITOR_CREATE_NULL_CB, ETAL_TEST_MODE_NONE, "Create monitor with NULL callback parameter");
	mon.m_receiverHandle = hReceiver;
	mon.m_CbBcastQualityProcess = NULL;
	if ((ret = etal_config_reception_quality_monitor(&hMonitor, &mon)) != ETAL_RET_QUAL_CONTAINER_ERR)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1c FAILED ret = %d", ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(SET_MONITOR_CREATE_INVALID_ATTR, ETAL_TEST_MODE_NONE, "Create monitor with invalid monitored attribute");
	mon.m_CbBcastQualityProcess = etalTestMonitorcb;
	mon.m_monitoredIndicators[0].m_MonitoredIndicator = 100; // insane value
	mon.m_monitoredIndicators[0].m_InferiorValue = ETAL_TEST_MIN_AM_FIELDSTRENGTH; // any valid value
	mon.m_monitoredIndicators[0].m_SuperiorValue = ETAL_INVALID_MONITOR;
	mon.m_monitoredIndicators[0].m_UpdateFrequency = ETAL_TEST_ONE_SECOND;
	if ((ret = etal_config_reception_quality_monitor(&hMonitor, &mon)) != ETAL_RET_QUAL_CONTAINER_ERR)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1d FAILED ret = %d", ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#elif CONFIG_APP_TEST_FM //CONFIG_APP_TEST_DAB
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#elif CONFIG_APP_TEST_HDRADIO_FM //CONFIG_APP_TEST_FM
	if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#elif CONFIG_APP_TEST_HDRADIO_AM //CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#else // CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif //CONFIG_APP_TEST_AM

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_SETMONITOR

/***************************
 *
 * etalTestSetMonitor
 *
 **************************/
tSInt etalTestSetMonitor(void)
{
#ifdef CONFIG_APP_TEST_SETMONITOR
	tBool pass;

	pass = TRUE;

	if (etalTestCheckParams(&pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

#if defined (CONFIG_APP_TEST_DAB)
	if (etalTestSetMonitor_internal(ETAL_TEST_MODE_DAB, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_FM)
	if (etalTestSetMonitor_internal(ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_AM)
	if (etalTestSetMonitor_internal(ETAL_TEST_MODE_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	if (etalTestSetMonitor_internal(ETAL_TEST_MODE_HD_FM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_HDRADIO_AM)
	if (etalTestSetMonitor_internal(ETAL_TEST_MODE_HD_AM, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_SETMONITOR
	return OSAL_OK;
}

#if defined (CONFIG_APP_TEST_GETCFQUALITY)
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_AM) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
/***************************
 *
 * etalTestPrintBand
 *
 **************************/
static tChar *etalTestPrintBand(EtalFrequencyBand band)
{
	switch (band)
	{
		case ETAL_BAND_UNDEF:
			return "Undefined";
		case ETAL_BAND_FM:
			return "FM";
		case ETAL_BAND_FMEU:
			return "FM EU";
		case ETAL_BAND_FMUS:
			return "FM US"; /* also for HD FM */
		case ETAL_BAND_FMJP:
			return "FM JP";
		case ETAL_BAND_FMEEU:
			return "FM East EU";
		case ETAL_BAND_WB:
			return "WB";
		case ETAL_BAND_USERFM:
			return "User FM";
		case ETAL_BAND_DRMP:
			return "DRM+";
		case ETAL_BAND_DRM30:
			return "DRM3";
		case ETAL_BAND_DAB3:
			return "DAB3";
		case ETAL_BAND_DABL:
			return "DABL";
		case ETAL_BAND_AM:
			return "AM";
		case ETAL_BAND_LW:
			return "LW";
		case ETAL_BAND_MWEU:
			return "MW EU";
		case ETAL_BAND_MWUS:
			return "MW US"; /* also for HD AM */
		case ETAL_BAND_SW:
			return "SW";
		case ETAL_BAND_CUSTAM:
			return "Custom AM";
		case ETAL_BAND_USERAM:
			return "USER AM";
		default:
			return "Illegal";
	}
}
#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_AM || CONFIG_APP_TEST_HDRADIO_FM

/***************************
 *
 * etalTestGetAnalogQuality
 *
 **************************/
static EtalFmQualityEntries *etalTestGetAnalogQuality(etalTestBroadcastTy mode, EtalCFDataContainer *CFData)
{
	switch (mode)
	{
		case ETAL_TEST_MODE_FM:
		case ETAL_TEST_MODE_AM:
			return &CFData->m_QualityContainer.EtalQualityEntries.amfm;

		case ETAL_TEST_MODE_HD_FM:
		case ETAL_TEST_MODE_HD_AM:
			return &CFData->m_QualityContainer.EtalQualityEntries.hd.m_analogQualityEntries;

		default:
			return NULL;
	}
}

/***************************
 *
 * etalTestGetCFDataInternal
 *
 **************************/
static tSInt etalTestGetCFDataInternal(etalTestBroadcastTy mode, ETAL_HANDLE hReceiver, tU32 pass_id, tBool *pass)
{
	EtalCFDataContainer CFData;
	EtalFmQualityEntries *qual;
	tS32 min, max;

	OSAL_pvMemorySet(&CFData, 0x00, sizeof(EtalCFDataContainer));

	switch (mode)
	{
		case ETAL_TEST_MODE_FM:
		case ETAL_TEST_MODE_HD_FM:
			min = ETAL_TEST_MIN_FM_FIELDSTRENGTH;
			max = ETAL_TEST_MAX_FM_FIELDSTRENGTH;
			OSAL_s32ThreadWait(ETAL_TEST_CMOST_QUALITY_SETTLE_TIME);
			break;

		case ETAL_TEST_MODE_AM:
		case ETAL_TEST_MODE_HD_AM:
			min = ETAL_TEST_MIN_AM_FIELDSTRENGTH;
			max = ETAL_TEST_MAX_AM_FIELDSTRENGTH;
			OSAL_s32ThreadWait(ETAL_TEST_CMOST_QUALITY_SETTLE_TIME_AM);
			break;

		default:
			return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(GET_CFQUAL_GET_CF, mode, "Get CF data");
	if (etal_get_CF_data(hReceiver, &CFData, 3, 100) != ETAL_RET_SUCCESS)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass%da FAILED", pass_id);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintVerbose("CF Data Container for frequency %d, band %s (0x%x)", CFData.m_CurrentFrequency, etalTestPrintBand(CFData.m_CurrentBand), CFData.m_CurrentBand);
		etalTestPrintVerboseContainer(&(CFData.m_QualityContainer));

		qual = etalTestGetAnalogQuality(mode, &CFData);
		if (qual == NULL)
		{
			return OSAL_ERROR;
		}
		etalTestPrintReportPassStart(GET_CFQUAL_CHECK_CF, mode, "Check CF data");
		if ((qual->m_RFFieldStrength > max) ||
			(qual->m_RFFieldStrength < min))
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%db FAILED (%d)", pass_id, qual->m_RFFieldStrength);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		/* check if fields not available in AM are set to 0 */
		if (mode == ETAL_TEST_MODE_AM)
		{
			etalTestPrintReportPassStart(GET_CFQUAL_VERIFY_AM, mode, "Verify AM data set to null");
			if ((qual->m_Multipath != 0) ||
				(qual->m_UltrasonicNoise != 0) ||
				(qual->m_SNR != 0) ||
				(qual->m_coChannel != 0) ||
				(qual->m_StereoMonoReception != 0))
			{
				*pass = FALSE;
				etalTestPrintReportPassEnd(testFailed);
				etalTestPrintNormal("pass%dc FAILED", pass_id);
			}
			else
			{
				etalTestPrintReportPassEnd(testPassed);
			}
		}

		/* Parameter check */
		etalTestPrintReportPassStart(GET_CFQUAL_GET_CF_INVALID_CONTAINER, mode, "Get CF data with invalid container");
		if (etal_get_CF_data(hReceiver, NULL, 3, 100) != ETAL_RET_PARAMETER_ERR)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%dd FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		etalTestPrintReportPassStart(GET_CFQUAL_GET_CF_INVALID_AVG, mode, "Get CF data with invalid average number");
		if (etal_get_CF_data(hReceiver, &CFData, 11, 100) != ETAL_RET_PARAMETER_ERR)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%de FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}

		etalTestPrintReportPassStart(GET_CFQUAL_GET_CF_INVALID_SAMPLE_TIME, mode, "Get CF data with invalid sample time");
		if (etal_get_CF_data(hReceiver, &CFData, 3, 101) != ETAL_RET_PARAMETER_ERR)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("pass%df FAILED", pass_id);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_GETCFQUALITY

/***************************
 *
 * etalTestGetCFData
 *
 **************************/
tSInt etalTestGetCFData(void)
{
#if defined(CONFIG_APP_TEST_GETCFQUALITY)
	tBool pass;
#ifdef CONFIG_APP_TEST_DAB
	EtalCFDataContainer CFData;
#endif

	etalTestStartup();
	pass = TRUE;

#ifdef CONFIG_APP_TEST_DAB
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(GET_CFQUAL_GET_CF_INVALID_STD, ETAL_TEST_MODE_DAB, "Get CF data for invalid standard");
	if (etal_get_CF_data(handledab, &CFData, 3, 100) != ETAL_RET_INVALID_BCAST_STANDARD)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass1 FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestGetCFDataInternal(ETAL_TEST_MODE_FM, handlefm, 1, &pass) != OSAL_OK)
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
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestGetCFDataInternal(ETAL_TEST_MODE_HD_FM, handlehd, 2, &pass) != OSAL_OK)
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
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_AM, &handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestGetCFDataInternal(ETAL_TEST_MODE_HD_AM, handlehdam, 3, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handlehdam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#ifdef CONFIG_APP_TEST_AM
	if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestGetCFDataInternal(ETAL_TEST_MODE_AM, handleam, 4, &pass) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
	{
		return OSAL_ERROR;

	}
#endif // CONFIG_APP_TEST_AM

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_GETCFQUALITY
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

