//!
//!  \file 		etaltest_main.c
//!  \brief 	<i><b> Main for ETAL test </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

/*****************************************************************
| local macros
|----------------------------------------------------------------*/
#define ETAL_TEST_SUMMARY_STRING_LEN    30
#define ETAL_TEST_NAME_STRING_LEN       30

#define ETAL_TEST_CALLBACK_ARRAY_SIZE    8

/*****************************************************************
| local types
|----------------------------------------------------------------*/
typedef struct
{
	tU32  testIndex;
	tBool testEnabled;
	tBool testSuccess;
	tU32  successCount;
	tU32  failCount;
	tChar testName[ETAL_TEST_NAME_STRING_LEN];
	int (*testFunction)(void);
} etalTestControlTy;

typedef enum
{
	testCapabilities,
	testConfigReceiver,
	testDestroyReceiver,
	testAudioSelect,
	testTuneReceiver,
	testGetQuality,
	testGetCFData,
	testSetMonitor,
	testReceiverAlive,
	testAudio,
	testGetFrequency,
	testAlternateFrequency,
	testChangeBand,
	testAdvancedTuning,
	testDataServices,
	testSystemData,
	testDabData,
	testGetRAWRDS,
	testGetRDS,
	testGetRadiotext,
	testSeek,
	testManualSeek,
	testScan,
	testLearn,
	testSeamless,
	testReadParameter,
	testWriteParameter,
	testServiceSelectDCOP,
	testTuneDCOP,
	testXTALalignment,
	testGetVersion,
    testFmVpa,
    testDebug,
    testRDSSeek,
    testRDSStrategy,
	testAutonotification,
	/* add further tests */
	testLast
} etalTestTy;

typedef struct
{
    tSInt numRegisteredFnct;
    EtalCallbackReasonEnumTy reason[ETAL_TEST_CALLBACK_ARRAY_SIZE];
    EtalTest_FunctPtr fnctPtr[ETAL_TEST_CALLBACK_ARRAY_SIZE];
} etalTestCallbackArrayTy;

#ifdef CONFIG_DEBUG_MEMORY_USAGE
typedef enum
{
	etalTestMemoryStart,
	etalTestMemoryProgress,
	etalTestMemoryEnd
} etalTestCheckMemoryModeTy;
#endif

/*****************************************************************
| variable defintion (scope: module-local)
|----------------------------------------------------------------*/
etalTestControlTy testControl[testLast] = {
/* By default all tests are disabled, update the etalTestInitStatus below */
/* For clean report ensure the first string in each row are aligned */
//                   "MAX LENGTH                   "
{ 1, FALSE, FALSE, 0, 0, "Capabilities",           etalTestCapabilities},
{ 2, FALSE, FALSE, 0, 0, "Config Receiver",        etalTestConfigReceiver},
{ 3, FALSE, FALSE, 0, 0, "Destroy Receiver",       etalTestDestroyReceiver},
{ 4, FALSE, FALSE, 0, 0, "Audio Select",           etalTestAudioSelect},
{ 5, FALSE, FALSE, 0, 0, "Tune Receiver",          etalTestTuneReceiver},
{ 6, FALSE, FALSE, 0, 0, "Get Quality",            etalTestGetQuality},
{ 7, FALSE, FALSE, 0, 0, "Get CF data",            etalTestGetCFData},
{ 8, FALSE, FALSE, 0, 0, "Set Monitor",            etalTestSetMonitor},
{ 9, FALSE, FALSE, 0, 0, "Tuner Alive",            etalTestReceiverAlive},
{10, FALSE, FALSE, 0, 0, "Audio",                  etalTestAudio},
{11, FALSE, FALSE, 0, 0, "Get Frequency",          etalTestGetFrequency},
{12, FALSE, FALSE, 0, 0, "Alternate frequency",    etalTestAF},
{13, FALSE, FALSE, 0, 0, "Change Band",            etalTestChangeBand},
{14, FALSE, FALSE, 0, 0, "Advanced Tuning",        etalTestAdvancedTuning},
{15, FALSE, FALSE, 0, 0, "Data Services",          etalTestDataServices},
{16, FALSE, FALSE, 0, 0, "System Data",            etalTestSystemData},
{17, FALSE, FALSE, 0, 0, "DAB Data",               etalTestDabData},
{18, FALSE, FALSE, 0, 0, "Get RAW RDS",            etalTestGetRAWRDS},
{19, FALSE, FALSE, 0, 0, "Get decoded RDS",        etalTestGetRDS},
{20, FALSE, FALSE, 0, 0, "Get Radiotext",          etalTestGetRadiotext},
{21, FALSE, FALSE, 0, 0, "Seek",                   etalTestSeek},
{22, FALSE, FALSE, 0, 0, "Manual Seek",            etalTestManualSeek},
{23, FALSE, FALSE, 0, 0, "Scan",                   etalTestScan},
{24, FALSE, FALSE, 0, 0, "Learn",                  etalTestLearn},
{25, FALSE, FALSE, 0, 0, "Seamless",               etalTestSeamless},
{26, FALSE, FALSE, 0, 0, "Read parameter",         etalTestReadParameter},
{27, FALSE, FALSE, 0, 0, "Write parameter",        etalTestWriteParameter},
{28, FALSE, FALSE, 0, 0, "Service Select DCOP",    etalTestServiceSelectDCOP},
{29, FALSE, FALSE, 0, 0, "Tune DCOP",              etalTestTuneDCOP},
{30, FALSE, FALSE, 0, 0, "XTAL alignment",         etalTestXTALalignment},
{31, FALSE, FALSE, 0, 0, "Get Version",            etalTestGetVersion},
{32, FALSE, FALSE, 0, 0, "VPA",                    etalTestFmVpa},
{33, FALSE, FALSE, 0, 0, "Debug and Eval",         etalTestDebug},
{34, FALSE, FALSE, 0, 0, "RDS Seek",               etalTestRDSSeek},
{35, FALSE, FALSE, 0, 0, "RDS AF Strategy",        etalTestRDSAFStrategy},	
{36, FALSE, FALSE, 0, 0, "Autonotification",       etalTestAutonotification},

/* add further tests */
};

tBool haveTest;
tChar outputFileName[OUTPUT_FILE_NAME_LEN];
FILE *outputFile;
etalTestOptionTy etalTestOption;
tBool etalTestOptionSkipDCOPReset;
static etalTestCallbackArrayTy etalTestCallbackArray;
EtalTraceConfig etalTestTraceConfig;
tBool etalTestInitializeDone;

/***************************
 *
 * etalTestInitStatus
 *
 **************************/
static void etalTestInitStatus(void)
{
	tU32 i;

	haveTest = FALSE;

	for (i = 0; i < testLast; i++)
	{
		testControl[i].testSuccess = TRUE;
		testControl[i].successCount = 0;
		testControl[i].failCount = 0;
	}
#ifdef CONFIG_APP_TEST_INITIALIZATION
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_CAPABILITIES
	testControl[testCapabilities].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_CONFIG_RECEIVER
	testControl[testConfigReceiver].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_DESTROY_RECEIVER
	testControl[testDestroyReceiver].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_AUDIO_SELECT
	testControl[testAudioSelect].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_TUNE_RECEIVER
	testControl[testTuneReceiver].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_GETQUALITY
	testControl[testGetQuality].testEnabled = TRUE;
	haveTest = TRUE;
#ifdef CONFIG_APP_TEST_GETCFQUALITY
		testControl[testGetCFData].testEnabled = TRUE;
		haveTest = TRUE;
#endif
#endif
#ifdef CONFIG_APP_TEST_SETMONITOR
	testControl[testSetMonitor].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_RECEIVER_ALIVE
    testControl[testReceiverAlive].testEnabled = TRUE;
    haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_AUDIO
	testControl[testAudio].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_GETFREQUENCY
	testControl[testGetFrequency].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_ALTERNATE_FREQUENCY
	testControl[testAlternateFrequency].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_CHANGEBAND
	testControl[testChangeBand].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_ADVANCED_TUNING
	testControl[testAdvancedTuning].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_DATASERVICES
	testControl[testDataServices].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_SYSTEMDATA
	testControl[testSystemData].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_DABDATA
	testControl[testDabData].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_GETRAWRDS_DATAPATH
	testControl[testGetRAWRDS].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_GETRDS
	testControl[testGetRDS].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_GETRADIOTEXT
	testControl[testGetRadiotext].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_SEEK
	testControl[testSeek].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_MANUAL_SEEK
	testControl[testManualSeek].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_SCAN
	testControl[testScan].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_LEARN
	testControl[testLearn].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_SEAMLESS
	testControl[testSeamless].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_READ_WRITE_PARAMETER
	testControl[testReadParameter].testEnabled = TRUE;
	haveTest = TRUE;
	testControl[testWriteParameter].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_SERVICE_SELECT_DCOP
	testControl[testServiceSelectDCOP].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_TUNE_DCOP
	testControl[testTuneDCOP].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_XTAL_ALIGNMENT
	testControl[testXTALalignment].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_GET_VERSION
	testControl[testGetVersion].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_FM_VPA
	testControl[testFmVpa].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#if defined(CONFIG_APP_TEST_DEBUG_DISS_WSP) || defined(CONFIG_APP_TEST_DEBUG_VPA_CONTROL)
	testControl[testDebug].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_RDS_SEEK
	testControl[testRDSSeek].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_RDS_AF_STRATEGY
	testControl[testRDSStrategy].testEnabled = TRUE;
	haveTest = TRUE;
#endif
#ifdef CONFIG_APP_TEST_AUTONOTIFICATION
	testControl[testAutonotification].testEnabled = TRUE;
	haveTest = TRUE;
#endif
/* add further tests */

	OSAL_pvMemorySet((tVoid *)&etalTestOption, 0x00, sizeof(etalTestOptionTy));
    OSAL_pvMemorySet((tVoid *)&etalTestCallbackArray, 0x00, sizeof(etalTestCallbackArray));
	OSAL_pvMemorySet((tVoid *)&etalTestTraceConfig, 0x00, sizeof(EtalTraceConfig));

}

/***************************
 *
 * etalTestRegisterCallbacks
 *
 **************************/
tSInt etalTestRegisterCallbacks(EtalTest_FunctPtr fnctPtr, EtalCallbackReasonEnumTy callbackReason)
{
    tSInt res = OSAL_ERROR;
    tSInt cnt;

    if (etalTestCallbackArray.numRegisteredFnct < ETAL_TEST_CALLBACK_ARRAY_SIZE)
    {
        // Find first slot free: due to de-registering method they are no more aligned
        for (cnt = 0; cnt < ETAL_TEST_CALLBACK_ARRAY_SIZE; cnt++)
        {
            if (NULL == etalTestCallbackArray.fnctPtr[cnt])
            {
                etalTestCallbackArray.fnctPtr[cnt] = fnctPtr;
                etalTestCallbackArray.reason[cnt] = callbackReason;

                etalTestCallbackArray.numRegisteredFnct++;

                res = etalTestCallbackArray.numRegisteredFnct;

                break;
            }
        }
    }

    return res;
}

/***************************
 *
 * etalTestDeRegisterCallbacks
 *
 **************************/
tSInt etalTestDeRegisterCallbacks(tSInt fnctId)
{
    tSInt res = OSAL_ERROR;
    tSInt fnctToDeRegister = fnctId - 1;

    if (etalTestCallbackArray.numRegisteredFnct < fnctToDeRegister)
    {
        etalTestCallbackArray.fnctPtr[fnctToDeRegister] = NULL;
        etalTestCallbackArray.reason[fnctToDeRegister] = 0;
        etalTestCallbackArray.numRegisteredFnct--;

        res = OSAL_OK;
    }

    return res;
}

/***************************
 *
 * etalTestServeCallbacks
 *
 **************************/
tBool etalTestServeCallbacks(EtalCallbackReasonEnumTy reason, tPVoid pvParam)
{
	tU32 cnt;
	tBool served;

	served = FALSE;

	for (cnt = 0; cnt < ETAL_TEST_CALLBACK_ARRAY_SIZE; cnt++)
	{
		if ((EtalTest_FunctPtr)NULL != etalTestCallbackArray.fnctPtr[cnt] &&
			reason == etalTestCallbackArray.reason[cnt])
		{
			etalTestCallbackArray.fnctPtr[cnt]((void *)pvParam);
			served = TRUE;
		}
	}
	return served;
}

#ifdef CONFIG_TRACE_ENABLE
/***************************
 *
 * etalTestWriteToFile
 *
 **************************/
static void etalTestWriteToFile(tU32 u32Level, tU32 u32Class, tU32 u16TraceMsgType, tU32 u32TraceMsgLength, tU8* u32TraceMsg)
{
	static tBool done = 0;
	tU32 msg_len;

	if (!OSALUTIL_s32TraceCheckFilter(u32Level, u32Class))
	{
		return;
	}
	msg_len = fwrite(u32TraceMsg, 1 , u32TraceMsgLength, outputFile);
	if (msg_len != u32TraceMsgLength)
	{
		if (done == 0)
		{
			done = 1;
			etalTestPrintError("Error writing to output file %s (req len %d, done %d)", outputFileName, u32TraceMsgLength, msg_len);
		}
	}
	fprintf(outputFile, "\n");
}
#endif

/********************************
 *
 * etalTestUsage
 *
 *******************************/
static tVoid etalTestUsage(char *name)
{
	printf("\n");
	printf("Usage: %s [-q <num>][-t <num>][-qh][-l][-o <filename>] [other options below]\n", name);
	printf("\n");
	printf("Performs various ETAL tests\n");
	printf("\n");
	printf("Options:\n");
	printf("   Trace options\n");
	printf("          -q <num>\n");
	printf("             Control the amount of messages generated by the %s application\n", name);
	printf("             Messages are divided in four categories:\n");
	printf("              ETAL_TEST_NORMAL_LEVEL:  describe what the test is doing\n");
	printf("              ETAL_TEST_VERBOSE_LEVEL: verbose test description (e.g. signal quality)\n");
	printf("              ETAL_TEST_ERROR_LEVEL:   errors that prevent a test from being run\n");
	printf("              ETAL_TEST_REPORT_LEVEL:  final report with pass/fail count and intermediate passes\n");
	printf("              ETAL_TEST_REPORT0_LEVEL: only the final report with pass/fail count\n");
	printf("             -q %d output all levels\n", ETAL_TEST_VERBOSE_ALL);
	printf("             -q %d output all levels except ETAL_TEST_VERBOSE_LEVEL (default)\n", ETAL_TEST_VERBOSE_NORMAL);
	printf("             -q %d output ETAL_TEST_REPORT_LEVEL and ETAL_TEST_ERROR_LEVEL\n", ETAL_TEST_VERBOSE_REPORT_ERROR);
	printf("             -q %d output ETAL_TEST_REPORT_LEVEL only\n", ETAL_TEST_VERBOSE_REPORT);
	printf("             -q %d output ETAL_TEST_REPORT0_LEVEL only\n", ETAL_TEST_VERBOSE_REPORT0);
	printf("             -q %d suppress all output\n", ETAL_TEST_VERBOSE_NONE);
#if defined (CONFIG_TRACE_ENABLE)
	printf("          -t <num>\n");
	printf("              Define the ETAL default trace level (tOSTraceConfig.u32DefaultLevel)\n");
	printf("              <num> is 0-8 and corresponds to the TR_LEVEL_ values\n");
	printf("              Default is TR_LEVEL_SYSTEM_MIN\n");
	printf("              Actual output depend on the ETAL build-time options\n");
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
	printf("          -tf <class> <mask> <level>\n");
	printf("              Define a trace filter. The filter changes the default trace level\n");
	printf("              for messages of class <class>. Only the subset of bits set in <mask>\n");
	printf("              are considered for the match. For example, given:\n");
	printf("                tOSTraceConfig.u32DefaultLevel = 1\n");
	printf("                <class> 0x80000000\n");
	printf("                <mask>  0xFFFF0000\n");
	printf("                <level> 3\n");
	printf("              a message of class 0x80000001, level 3 will be displayed;\n");
	printf("              a message of class 0x40000000, level 3 will not be displayed\n");
	printf("              <class> and <mask> are expressed in HEX; <level> in decimal\n");
	printf("              The option can be repeated up to %d times on the command line\n", CONFIG_OSUTIL_TRACE_NUM_FILTERS);
#endif
	printf("          -qh omit the time:level:class header. May be specified alone\n");
	printf("              or in addition to -q <num> or -t <num>\n");
	printf("          -o <filename>\n");
	printf("             Send diagnostic output to <filename>\n");
#endif // CONFIG_TRACE_ENABLE
	printf("   Loop options\n");
	printf("          -l <n>\n");
	printf("             For n=0  loop forever\n");
	printf("             For n!=0 loop <n> times then exit\n");
	printf("             Cannot be used with -L\n");
	printf("          -L <n>\n");
	printf("             For n=0  loop forever waiting for user input at the beginning of every loop\n");
	printf("             For n!=0 loop <n> times, wait for user input, loop <n> times, wait user input, ...\n");
	printf("             Cannot be used with -l\n");
	printf("          -r\n");
	printf("             Wait for user input after resetting the DCOP to allow loading it from the debugger\n");
	printf("             Requires build time support (INCLUDE_INTERACTIVE_DCOP_RESET_CODE)\n");
	printf("          -k\n");
	printf("             Stop on all errors except \'tune timeout on empty frequency\'\n");
	printf("             Available only for:\n");
	printf("             - DCOP Service Select test\n");
	printf("             - Tune FM test\n");
#if defined (CONFIG_APP_TEST_STACK) || defined (CONFIG_APP_TEST_STECI) || defined (CONFIG_APP_TEST_SIGNALLING_GPIO)
	printf("   Special test options\n");
#endif
#ifdef CONFIG_APP_TEST_STACK
	printf("          -s <stacks>\n");
	printf("             Performs run-time stack overflow test: spawns a thread with stack <stacks>\n");
	printf("             which tries to allocate on the stack an array of size %d\n", ETAL_TEST_ARRAY_SIZE);
#endif
#ifdef CONFIG_APP_TEST_STECI
	printf("          -S\n");
	printf("             Performs run-time STECI stess test\n");
#endif
#ifdef CONFIG_APP_TEST_SIGNALLING_GPIO
	printf("          -g\n");
	printf("             Performs run-time signalling GPIO test\n");
	printf("             which toggles a GPIO for some time\n");
	printf("             Connect an oscilloscope to check if the test is OK!\n");
#endif
	printf("\n");
	printf("Returns:\n");
	printf(" The number of failed tests\n");
	printf(" 0 if all tests pass\n");
	printf("\n");
}

/********************************
 *
 * etalTestProcessParams
 *
 *******************************/
static tS32 etalTestProcessParams(int argc, char **argv, int *proc_arg)
{
	tS32 processed_arg;

	processed_arg = 0;

	/* initialize defaults */
	etalTestOption.oQuietLevel = ETAL_TEST_VERBOSE_NORMAL;

	while (argc > 1)
	{
		if (strcmp(argv[1 + processed_arg], "-q") == 0)
		{
			if (argc < 3)
			{
				goto paramError;
			}
			etalTestOption.oQuietLevel = atoi(argv[2 + processed_arg]);
			processed_arg++;
			argc--;
			if (etalTestOption.oQuietLevel > ETAL_TEST_VERBOSE_NONE)
			{
				printf("\n\nIllegal value for -q (%d)\n\n", etalTestOption.oQuietLevel);
				goto paramError;
			}
		}
#if defined (CONFIG_TRACE_ENABLE)
		else if (strcmp(argv[1 + processed_arg], "-t") == 0)
		{
			tU32 tmp_lvl;

			if (argc < 3)
			{
				goto paramError;
			}
			tmp_lvl = atoi(argv[2 + processed_arg]);
			processed_arg++;
			argc--;
			if (tmp_lvl > TR_LEVEL_USER_4)
			{
				printf("\n\nIllegal value for -t (%d)\n\n", tmp_lvl);
				goto paramError;
			}
			etalTestTraceConfig.m_defaultLevel = tmp_lvl;
			etalTestTraceConfig.m_defaultLevelUsed = TRUE;
			etalTestTraceConfig.m_reserved = 1;
		}
#if defined (CONFIG_TRACE_INCLUDE_FILTERS)
		else if (strcmp(argv[1 + processed_arg], "-tf") == 0)
		{
			tU32 tmp_class, tmp_level;
			tU64 tmp_mask;

			if (argc < 5)
			{
				goto paramError;
			}
			if (etalTestTraceConfig.m_filterNum >= CONFIG_OSUTIL_TRACE_NUM_FILTERS)
			{
				printf("\n\nToo many filters specified with -tf (max %d)\n\n", CONFIG_OSUTIL_TRACE_NUM_FILTERS);
				goto paramError;
			}
			tmp_class = strtol(argv[2 + processed_arg], NULL, 16);
			processed_arg++;
			argc--;
			tmp_mask = strtoll(argv[2 + processed_arg], NULL, 16);
			processed_arg++;
			argc--;
			tmp_level = atoi(argv[2 + processed_arg]);
			processed_arg++;
			argc--;
			etalTestTraceConfig.m_filterClass[etalTestTraceConfig.m_filterNum] = tmp_class;
			etalTestTraceConfig.m_filterMask[etalTestTraceConfig.m_filterNum] = (tU32)tmp_mask;
			etalTestTraceConfig.m_filterLevel[etalTestTraceConfig.m_filterNum] = tmp_level;
			printf("Configured filter %d: class 0x%08X, mask 0x%08X, level %d\n", etalTestTraceConfig.m_filterNum + 1, 
				etalTestTraceConfig.m_filterClass[etalTestTraceConfig.m_filterNum],
				etalTestTraceConfig.m_filterMask[etalTestTraceConfig.m_filterNum],
				etalTestTraceConfig.m_filterLevel[etalTestTraceConfig.m_filterNum]);
			etalTestTraceConfig.m_filterNum++;
		}
#endif // CONFIG_TRACE_INCLUDE_FILTERS
		else if (strcmp(argv[1 + processed_arg], "-qh") == 0)
		{
			etalTestTraceConfig.m_disableHeader = TRUE;
			etalTestTraceConfig.m_disableHeaderUsed = TRUE;
			etalTestOption.oDisableTraceHeader = TRUE;
		}
		else if (strcmp(argv[1 + processed_arg], "-o") == 0)
		{
			if (argc < 3)
			{
				goto paramError;
			}
			OSAL_szStringCopy(outputFileName, argv[2 + processed_arg]);		
			processed_arg++;
			argc--;
			etalTestOption.oOutToFile = 1;
		}
#endif // CONFIG_TRACE_ENABLE
		else if (strcmp(argv[1 + processed_arg], "-l") == 0)
		{
			if (argc < 3)
			{
				goto paramError;
			}
			etalTestOption.oLoopNumber = atoi(argv[2 + processed_arg]);
			processed_arg++;
			argc--;
			if (etalTestOption.oLoopNumber == 0)
			{
				etalTestOption.oLoopEndless = 1;
			}
		}
		else if (strcmp(argv[1 + processed_arg], "-L") == 0)
		{
			if (argc < 3)
			{
				goto paramError;
			}
			etalTestOption.oLoopNumberInteractive = atoi(argv[2 + processed_arg]);
			processed_arg++;
			argc--;
			if (etalTestOption.oLoopNumberInteractive == 0)
			{
				etalTestOption.oLoopEndlessInteractive = 1;
			}
		}
#ifdef CONFIG_APP_TEST_STACK
		else if (strcmp(argv[1 + processed_arg], "-s") == 0)
		{
			if (argc < 3)
			{
				goto paramError;
			}
			etalTestOption.oStackSize = atoi(argv[2 + processed_arg]);
			processed_arg++;
			argc--;
			etalTestOption.oStackCheck = 1;
		}
#endif
#ifdef CONFIG_APP_TEST_STECI
		else if (strcmp(argv[1 + processed_arg], "-S") == 0)
		{
			etalTestOption.oSTECICheck = 1;
		}
#endif
#ifdef CONFIG_APP_TEST_SIGNALLING_GPIO
		else if (strcmp(argv[1 + processed_arg], "-g") == 0)
		{
			etalTestOption.oSignallingGPIO = 1;
		}
#endif
#ifdef CONFIG_APP_TEST_TUNE_TIME_MEASURE
		else if (strcmp(argv[1 + processed_arg], "-t") == 0)
		{
			etalTestOption.oTuneTime = 1;
		}
#endif
		else if (strcmp(argv[1 + processed_arg], "-k") == 0)
		{
			etalTestOption.oStopOnErrors = 1;
		}
		else if (strcmp(argv[1 + processed_arg], "-r") == 0)
		{
			etalTestOptionSkipDCOPReset = 1;
		}
		else
		{
			goto paramError;
		}
		processed_arg++;
		argc--;
	}

	if ((etalTestOption.oLoopEndless != 0 || etalTestOption.oLoopNumber != 0) && 
		(etalTestOption.oLoopEndlessInteractive != 0 || etalTestOption.oLoopNumberInteractive != 0))
	{
		// both interactive and non-interactive loop not allowed
		goto paramError;
	}

	if (etalTestOption.oOutToFile)
	{
		outputFile = fopen(outputFileName, "w");
		if (outputFile == NULL)
		{
			// normally this function is called before ETAL_init, so TracePrintf is not available
			printf("%s\n", strerror(errno));
			goto paramError;
		}
#ifdef CONFIG_TRACE_ENABLE
		OSALUTIL_vTraceSetOutputDriverCallback(etalTestWriteToFile);
#endif
	}

	if (!haveTest && !etalTestOption.oStackCheck && !etalTestOption.oSTECICheck && !etalTestOption.oSignallingGPIO && !etalTestOption.oTuneTime)
	{
		printf("No test compiled in and no run-time option!\n");
		printf("Exiting\n");
		return 1;
	}

	*proc_arg = processed_arg;
	return 0;

paramError:
	*proc_arg = 0;
	etalTestUsage(argv[0]);
	return 1;
}


/***************************
 *
 * etalTestResetStatus
 *
 **************************/
static tVoid etalTestResetStatus(void)
{
	extern tVoid ETAL_restart(void);

#if defined (CONFIG_TRACE_ASYNC)
	/* In this build mode the printf are output by an async thread
	 * In case of error the test main loop restarts the tests too quickly
	 * for the async thread and its input FIFO is overflown.
	 *
	 * So we allow some time to the ETAL_tracePrint_ThreadEntry to
	 * output some strings before restarting the test */
	OSAL_s32ThreadWait(500);
#endif

	etalTestPrintNormal("******* Forcing status reset *******");

	ETAL_restart();
	/* Ensure all the handles are reset in case some test
	 * does not call etalTestStartup and relies on the 
	 * previous tests to destroy the receiver handles */
	etalTestStartup();

	etalTestPrintNormal("");
}

/***************************
 *
 * etalTestUpdatePrintSingleTest
 *
 **************************/
static void etalTestUpdatePrintSingleTest(etalTestTy test_index)
{
	etalTestControlTy *pt = &testControl[test_index];
	tChar testnamestr[80+1];

	if (!pt->testEnabled)
	{
		return;
	}
	if (pt->testSuccess)
	{
		pt->successCount++;
	}
	else
	{
		pt->failCount++;
	}
	OSAL_szStringNCopy(testnamestr, pt->testName, 80);
	OSAL_szStringNConcat(testnamestr, " Test:", (80 - OSAL_u32StringLength(testnamestr)));
	etalTestPrintReport0("%-25s passed (%5d) FAILED (%5d)", testnamestr, pt->successCount, pt->failCount);
}

/***************************
 *
 * etalTestPrintSummary
 *
 **************************/
static void etalTestPrintSummary(int loop, tSInt init_test)
{
	tU32 i = 0;
	etalTestPrintReport0("Test summary:");

#ifdef CONFIG_APP_TEST_INITIALIZATION
	if (loop == 1)
	{
		if (init_test == OSAL_OK)
		{
			etalTestPrintReport0("Initialize Test:          passed");
		}
		else
		{
			etalTestPrintReport0("Initialize Test:          FAILED");
		}
	}
	else
	{
		etalTestPrintReport0("Initialize Test:          skipped");
	}
#endif
	for (i = 0; i < testLast; i++)
	{
		etalTestUpdatePrintSingleTest(i);
	}
	etalTestPrintReport0("");
}

#ifndef CONFIG_APP_TEST_ONLY_SEQUENTIAL
/***************************
 *
 * etalTestThreadWrapper
 *
 **************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid etalTestThreadWrapper(tSInt stacd, tPVoid param)
#else
static tVoid etalTestThreadWrapper(tVoid *param)
#endif
{
	etalTestThreadAttrTy *attr;
	attr = (etalTestThreadAttrTy *) param;

	if (attr->entry(attr->hReceiver) != OSAL_OK)
	{
		attr->retval = OSAL_ERROR;
	}
	else
	{
		attr->retval = OSAL_OK;
	}
	attr->status = THREAD_STOPPED;
	OSAL_vThreadExit();
}

/***************************
 *
 * etalTestThreadSpawn
 *
 **************************/
tSInt etalTestThreadSpawn(etalTestThreadAttrTy *attr)
{
	OSAL_trThreadAttribute tattr;
	tChar name[24];

	if (attr == NULL)
	{
		etalTestPrintError("etalTestThreadSpawn illegal attributes");
		return OSAL_ERROR;
	}

	OSAL_s32PrintFormat(name, "ETALTEST%d", attr->hReceiver);
	tattr.szName = name;
	tattr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	tattr.s32StackSize = 4096;
	tattr.pfEntry = etalTestThreadWrapper;
	tattr.pvArg = (tPVoid) attr;
	attr->threadId = OSAL_ThreadSpawn(&tattr);

	if (attr->threadId == OSAL_ERROR)
	{
		etalTestPrintError("etalTestThreadSpawn thread creation, handle %d", attr->hReceiver);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestThreadWait
 *
 **************************/
tSInt etalTestThreadWait(etalTestThreadAttrTy *attr)
{
	if (attr == NULL)
	{
		etalTestPrintError("etalTestThreadWait illegal attributes");
		return OSAL_ERROR;
	}
	while (attr->status == THREAD_RUNNING)
	{
		OSAL_s32ThreadWait(ETAL_TEST_MAIN_IDLE_SCHEDULING);
	}
	return attr->retval;
}
#endif // CONFIG_APP_TEST_ONLY_SEQUENTIAL

#ifdef CONFIG_DEBUG_MEMORY_USAGE
/***************************
 *
 * etalTestCheckFreeMemory
 *
 **************************/
/*
 * printf: command line equivalent of C library printf
 * free: coreutils application to report used/free system memory
 * grep: search ths string 'Mem:' in the output of free
 * tr: squeeze (-s) connsecutive spaces (' ') in a single instance
 * cut: take only the fourth (-f 4) field of the input, using space (-d ' ') as field delimiter
 */
static tVoid etalTestCheckFreeMemory(etalTestCheckMemoryModeTy mode)
{
	switch (mode)
	{
		case etalTestMemoryStart:
			system("printf \"Free memory start: %d\n\" `free | grep Mem: | tr -s ' ' | cut -d ' ' -f 4`");
			break;
		case etalTestMemoryProgress:
			system("printf \"Free memory progress: %d\n\" `free | grep Mem: | tr -s ' ' | cut -d ' ' -f 4`");
			break;
		case etalTestMemoryEnd:
			system("printf \"Free memory end: %d\n\" `free | grep Mem: | tr -s ' ' | cut -d ' ' -f 4`");
			break;
	}
}
#endif

/***************************
 *
 * etalTestStartup
 *
 **************************/
tVoid etalTestStartup(void)
{
	handledab = ETAL_INVALID_HANDLE; 
	handlefm = ETAL_INVALID_HANDLE;
	handlehd = ETAL_INVALID_HANDLE; 
	handlehdam = ETAL_INVALID_HANDLE;
	handlehd2 = ETAL_INVALID_HANDLE; 
	handleam = ETAL_INVALID_HANDLE; 
	handlefm2 = ETAL_INVALID_HANDLE;
	handledab1_5 = ETAL_INVALID_HANDLE;
	handlehd1_5 = ETAL_INVALID_HANDLE;
}

/***************************
 *
 * etalTestWaitInput
 *
 **************************/
static tVoid etalTestWaitInput(void)
{
	etalTestPrintNormal("Press RETURN to continue");
	fgetc(stdin);
}

/***************************
 *
 * etalTestPrintReportVersion
 *
 **************************/
static tVoid etalTestPrintReportVersion(tVoid)
{
	EtalVersion version;

	if (etal_get_version(&version) == ETAL_RET_SUCCESS)
	{
		if (version.m_ETAL.m_isValid)
		{
			etalTestPrintReport0("ETAL version %d.%d.%d build %d", version.m_ETAL.m_major, version.m_ETAL.m_middle, version.m_ETAL.m_minor, version.m_ETAL.m_build);
			return;
		}
	}
	etalTestPrintReport0("ETAL version not available");
}

/***************************
 *
 * main
 *
 **************************/
/*
 * Returns:
 * >0 number of failed tests
 * 0 all tests passed
 */
#ifdef CONFIG_HOST_OS_TKERNEL
int etaltest_EntryPoint(etalTestOptionTy *pI_etalTestOption, EtalTraceConfig *pI_etalTestTraceConfig )
#else
int main(int argc, char **argv)
#endif //HOST_OS_TKERNEL
{
	tU32 i;
	tU32 loop = 1;
	tS32 processed_arg = 0;
	etalTestControlTy *pt;
	tU32 ret;
	tSInt test_res;
	tBool forceStop = FALSE;
	tBool endlessLoop = FALSE;
	tBool TestInitialize = FALSE;
	tBool TestInitializeRes = TRUE;

#ifdef CONFIG_DEBUG_MEMORY_USAGE
	etalTestCheckFreeMemory(etalTestMemoryStart);
#endif

	etalTestInitStatus();

	// Fill in the test options
#ifdef CONFIG_HOST_OS_TKERNEL
	if (NULL != pI_etalTestOption)
	{
		OSAL_pvMemoryCopy((tVoid *)&etalTestOption, pI_etalTestOption, sizeof(etalTestOptionTy));
	}
	
	if (NULL != pI_etalTestTraceConfig)
	{
		OSAL_pvMemoryCopy((tVoid *)&etalTestTraceConfig, pI_etalTestTraceConfig, sizeof(EtalTraceConfig));
	}
			
#else
	if (etalTestProcessParams(argc, argv, &processed_arg) != 0)
	{
		return 1;
	}
#endif 

#ifdef CONFIG_APP_TEST_STACK
	if (etalTestOption.oStackCheck)
	{
		etalTestStack();
	}
#endif

	if ((etalTestOption.oLoopEndless != 0) ||
		(etalTestOption.oLoopEndlessInteractive != 0) ||
		(etalTestOption.oLoopNumberInteractive != 0))
	{
		// all the interactve options imply endless loop
		endlessLoop = 1;
	}

	if (etalTestTraceConfig.m_defaultLevelUsed == FALSE)
	{
		/* no user-specified default for ETAL trace, force a value */
		/* use the level defined for system integration tests */
		etalTestTraceConfig.m_defaultLevel = ETAL_TR_LEVEL_SYSTEM_MIN;
		etalTestTraceConfig.m_defaultLevelUsed = TRUE;
		etalTestTraceConfig.m_reserved = 1;
	}

#if defined(CONFIG_APP_TEST_INITIALIZATION)
	etalTestPrintReport0("********** Starting Test Suite, pre-loop **********");
	TestInitialize = TRUE;
	etalTestPrintReportTestStart("Initialize", 0);
	TestInitializeRes = etalTestInitialize(&etalTestTraceConfig);
	etalTestPrintReportTestEnd("Initialize", 0, TestInitializeRes);
	etalTestPrintNormal("");
#endif
	if (etalTestStartETAL(&etalTestTraceConfig) == OSAL_OK)
	{
		etalTestInitializeDone = TRUE;
#if ((defined(CONFIG_BOARD_ACCORDO2)) || (defined(CONFIG_BOARD_ACCORDO5)) && (!defined(CONFIG_HOST_OS_TKERNEL)))
		system("amixer -c 3 sset Source adcauxdac > /dev/null");
		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#ifdef CONFIG_BOARD_ACCORDO5
		system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
#endif
		system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
#endif
	}

	if (etalTestInitializeDone)
	{
		etalTestPrintReportVersion();
		do
		{
#ifdef CONFIG_APP_TEST_STECI
			if (etalTestOption.oSTECICheck)
			{
				etalTestSTECI(); // endless loop until error occurs
				etalTestPrintReport0("STECI stress test FAILED, aborting");
				return 1;
			}
#endif
#ifdef CONFIG_APP_TEST_SIGNALLING_GPIO
			if (etalTestOption.oSignallingGPIO)
			{
				etalTestSignallingGPIO();
			}
#endif
#ifdef CONFIG_APP_TEST_TUNE_TIME_MEASURE
			if (etalTestOption.oTuneTime)
			{
				etalTestTuneTiming();
			}
#endif
			etalTestPrintReport0("********** Starting Test Suite, loop %03d **********", loop);

			if (etalTestOption.oLoopEndlessInteractive ||
				((etalTestOption.oLoopNumberInteractive != 0) && (loop > 1) && (loop % etalTestOption.oLoopNumberInteractive) == 1))
			{
				etalTestWaitInput();
			}

			for (i = 0; i < testLast; i++)
			{
				pt = &testControl[i];
				if (!pt->testEnabled)
				{
					continue;
				}
				pt->testSuccess = TRUE;
				etalTestPrintReportTestStart(pt->testName, pt->testIndex);
				test_res = pt->testFunction();
				etalTestPrintReportTestEnd(pt->testName, pt->testIndex, test_res);
				etalTestPrintNormal("");
				if (test_res != OSAL_OK)
				{
					/* WARNING: in some conditions this function inserts a delay in the test */
					etalTestResetStatus();
					pt->testSuccess = FALSE;
					if (etalTestOption.oStopOnErrors == TRUE)
					{
						forceStop = TRUE;
						break; // leave the 'for' loop
					}
				}
			}

			etalTestPrintSummary(loop, TestInitializeRes);
#ifdef CONFIG_DEBUG_MEMORY_USAGE
			etalTestCheckFreeMemory(etalTestMemoryProgress);
#endif
			loop++;

			if (etalTestOption.oOutToFile)
			{
				fflush(outputFile);
			}
			if (forceStop)
			{
				forceStop = FALSE;
				if ((etalTestOption.oLoopEndlessInteractive != 0) ||
					(etalTestOption.oLoopNumberInteractive != 0))
				{
					etalTestWaitInput();
				}
				else
				{
					etalTestPrintReport0("Aborting test due to -k option");
					break;
				}
			}
		} while (endlessLoop || ((etalTestOption.oLoopNumber != 0) && (loop < etalTestOption.oLoopNumber + 1)));
	}

	if (etalTestInitializeDone && (etal_deinitialize() != ETAL_RET_SUCCESS))
	{
		etalTestPrintError("Failed to de-initialize ETAL");
	}

	if (etalTestOption.oOutToFile)
	{
		fclose(outputFile);
	}

	// count the etal_initialize test only if explicitly requested by config
	if (TestInitialize && !etalTestInitializeDone)
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	for (i = 0; i < testLast; i++)
	{
		pt = &testControl[i];
		if (pt->testEnabled && !pt->testSuccess)
		{
			ret++;
		}
	}

#ifdef CONFIG_DEBUG_MEMORY_USAGE
	etalTestCheckFreeMemory(etalTestMemoryEnd);
#endif

	return ret;
}

#endif // CONFIG_APP_ETAL_TEST

