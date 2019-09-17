//!
//!  \file 		etaltest.h
//!  \brief 	<i><b> Include for ETAL test </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

/*
 * NOTE: these tests use valid (ETAL_VALID_*, some broadcast present)
 * and invalid (ETAL_INVALID_*, no broadcast present) FM and DAB frequencies.
 * The values of the frequencies are defined below.
 *
 * Some tests are defined both for AMFM and DAB broadcasts. Selection between the
 * two can be made at compile time with the CONFIG_APP_TEST_DAB and CONFIG_APP_TEST_FM.
 * Whether both can be enabled depends mostly on the hardware configuration used.
 *
 */

/*
 * Automatically pull in initialization test for STECI
 */
#if defined (CONFIG_APP_TEST_STECI)
	#undef CONFIG_APP_TEST_INITIALIZATION
	#define CONFIG_APP_TEST_INITIALIZATION
#endif //defined (CONFIG_APP_TEST_STECI)

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*
 * time in ms
 */
#define ETAL_TEST_MAIN_IDLE_SCHEDULING 50

#define ETAL_VALID_DAB_FREQ    		225648
#define ETAL_VALID_DAB_OTHER_FREQ	229072
#define ETAL_VALID_DAB_FREQ1_5 		237488
#define ETAL_EMPTY_DAB_FREQ    		174928 //224096 // Seek test assume this is smaller than ETAL_VALID_DAB_FREQ
#define ETAL_VALID_AM_FREQ_LOW    	   144
#define ETAL_VALID_AM_FREQ_HIGH 	 30000
#define ETAL_EMPTY_AM_FREQ        	   800
#define ETAL_EMPTY_DRM_FREQ        	   144

#ifdef CONFIG_APP_TEST_IN_LE_MANS
	#define ETAL_VALID_FM_FREQ     100300 // 100300 FUN // 104700 EUROPE1 //100700 NOSTALGIE //103500 VIRGIN //95400 RIRE & //103900 RMC
	#define ETAL_VALID_FM_FREQ2     95400 // 100300 FUN // 104700 EUROPE1 //100700 NOSTALGIE //103500 VIRGIN //95400 RIRE & //103900 RMC
    #define ETAL_VALID_FM_FREQ_VPA  92600 // 92600 France Inter // 93300 Cartable FM // 97000 France musique
	#define ETAL_VALID_AM_FREQ       1629
    #define ETAL_VALID_HD_FREQ      88700
    #define ETAL_VALID_HD_FREQ2     88700
    #define ETAL_VALID_HD_AM_FREQ     720

	#define ETAL_EMPTY_FM_FREQ      99550
#else
	#define ETAL_VALID_FM_FREQ      94800 // Marconi
	#define ETAL_VALID_FM_FREQ2    107300 // Radio Dimensione Suono
    #define ETAL_VALID_FM_FREQ_VPA 104800 // choose a frequency with low field strength
	#define ETAL_VALID_AM_FREQ        900
	#define ETAL_VALID_HD_FREQ      88700
    #define ETAL_VALID_HD_FREQ2     88700
    #define ETAL_VALID_HD_AM_FREQ     720
	#define ETAL_EMPTY_FM_FREQ      98800
#endif //#ifdef CONFIG_APP_TEST_IN_LE_MANS


/*
 * Time in ms required after tune command to DAB before FIC (MCI)
 * can be considered acquired
 */
 // Update to 2 s. 1s is too short some time
#define ETAL_TEST_DAB_TUNE_SETTLE_TIME      2000

/*
 * Time in ms required for the CMOST quality measures to stabilize
 * after a tune command, for FM
 */
#define ETAL_TEST_CMOST_QUALITY_SETTLE_TIME   20

/*
 * Time in ms required for the CMOST quality measures to stabilize
 * after a tune command, for AM
 */
#define ETAL_TEST_CMOST_QUALITY_SETTLE_TIME_AM 30

/*
 * Time in ms to wait before HD audio is aquired
 * Empirical value
 */
#define ETAL_TEST_HD_AUDIO_SYNC_TIME      6000

/*
 * For two concurrent receivers avoid using the same
 * frontend for two receivers, ETAL will complain
 */
#if defined (CONFIG_MODULE_INTEGRATED)
	/*
	 * On the MTD the DCOP is only connected to second CMOST
	 * (a DOT on the MTD v1, STAR on v2)
	 * which maps to ETAL_FE_HANDLE_3/ETAL_FE_HANDLE_4
	 */
	#if defined (CONFIG_APP_TEST_DAB)
		#define ETAL_FE_FOR_DAB_TEST    	ETAL_FE_HANDLE_3
		#define ETAL_FE_FOR_DAB_TEST1_5 	ETAL_FE_HANDLE_4
		#define ETAL_FE_INVALID_FOR_DAB_TEST ETAL_FE_HANDLE_1
	#else //defined (CONFIG_APP_TEST_DAB)
		#define ETAL_FE_FOR_DAB_TEST    ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_DAB_TEST1_5 ETAL_INVALID_HANDLE
		#define ETAL_FE_INVALID_FOR_DAB_TEST ETAL_INVALID_HANDLE
	#endif //defined (CONFIG_APP_TEST_DAB)
	#if defined (CONFIG_APP_TEST_FM)
		#define ETAL_FE_FOR_FM_TEST     ETAL_FE_HANDLE_1
		#define ETAL_FE_FOR_FM_TEST2    ETAL_FE_HANDLE_2
	#else //defined (CONFIG_APP_TEST_FM)
		#define ETAL_FE_FOR_FM_TEST     ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_FM_TEST2    ETAL_INVALID_HANDLE
	#endif //defined (CONFIG_APP_TEST_FM)
	#if defined(CONFIG_APP_TEST_AM)
		#define ETAL_FE_FOR_AM_TEST     ETAL_FE_HANDLE_1
		#define ETAL_FE_FOR_AM_TEST_2   ETAL_FE_HANDLE_2
	#else //defined(CONFIG_APP_TEST_AM)
		#define ETAL_FE_FOR_AM_TEST     ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_AM_TEST_2   ETAL_INVALID_HANDLE
	#endif //defined(CONFIG_APP_TEST_AM)
	#if defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
		#define ETAL_FE_FOR_HD_TEST     ETAL_FE_HANDLE_1
	#else //defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
		#define ETAL_FE_FOR_HD_TEST     ETAL_INVALID_HANDLE
	#endif //defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
#elif defined (CONFIG_MODULE_INDEPENDENT) //defined (CONFIG_MODULE_INTEGRATED)
	/*
	 * On the CMOST only the foreground channel is available on the
	 * DAC output so if we want to hear
	 * audio on the Accordo2 we must use ETAL_FE_HANDLE_1 for AM/FM tests.
	 */
	#if defined (CONFIG_APP_TEST_DAB) && !defined (CONFIG_APP_TEST_FM)
		#define ETAL_FE_FOR_DAB_TEST	ETAL_FE_HANDLE_1
		#define ETAL_FE_FOR_DAB_TEST1_5 ETAL_FE_HANDLE_2
		#define ETAL_FE_INVALID_FOR_DAB_TEST ETAL_FE_HANDLE_3
		#define ETAL_FE_FOR_FM_TEST     ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_FM_TEST2    ETAL_INVALID_HANDLE
	#elif !defined(CONFIG_APP_TEST_DAB) && defined (CONFIG_APP_TEST_FM) //defined (CONFIG_APP_TEST_DAB) && !defined (CONFIG_APP_TEST_FM)
		#define ETAL_FE_FOR_DAB_TEST	ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_DAB_TEST1_5 ETAL_INVALID_HANDLE
		#define ETAL_FE_INVALID_FOR_DAB_TEST ETAL_FE_HANDLE_3
		#define ETAL_FE_FOR_FM_TEST     ETAL_FE_HANDLE_1
		#define ETAL_FE_FOR_FM_TEST2    ETAL_FE_HANDLE_2
	#elif defined(CONFIG_APP_TEST_DAB) && defined(CONFIG_APP_TEST_FM) //!defined(CONFIG_APP_TEST_DAB) && defined (CONFIG_APP_TEST_FM)
		#define ETAL_FE_FOR_DAB_TEST	ETAL_FE_HANDLE_2
		#define ETAL_FE_FOR_DAB_TEST1_5 ETAL_INVALID_HANDLE
		#define ETAL_FE_INVALID_FOR_DAB_TEST ETAL_FE_HANDLE_3
		#define ETAL_FE_FOR_FM_TEST     ETAL_FE_HANDLE_1
		#define ETAL_FE_FOR_FM_TEST2    ETAL_INVALID_HANDLE
	#elif !defined(CONFIG_APP_TEST_DAB) && !defined(CONFIG_APP_TEST_FM) //defined(CONFIG_APP_TEST_DAB) && defined(CONFIG_APP_TEST_FM)
		#define ETAL_FE_FOR_DAB_TEST	ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_DAB_TEST1_5 ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_FM_TEST     ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_FM_TEST2    ETAL_INVALID_HANDLE
	#endif //!defined(CONFIG_APP_TEST_DAB) && !defined(CONFIG_APP_TEST_FM)
	#if defined(CONFIG_APP_TEST_AM)
		#define ETAL_FE_FOR_AM_TEST     ETAL_FE_HANDLE_1
		#define ETAL_FE_FOR_AM_TEST_2   ETAL_FE_HANDLE_2
	#else //defined(CONFIG_APP_TEST_AM)
		#define ETAL_FE_FOR_AM_TEST     ETAL_INVALID_HANDLE
		#define ETAL_FE_FOR_AM_TEST_2   ETAL_INVALID_HANDLE
	#endif //defined(CONFIG_APP_TEST_AM)
	#if defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
		#define ETAL_FE_FOR_HD_TEST     ETAL_FE_HANDLE_1
	#else //defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
		#define ETAL_FE_FOR_HD_TEST     ETAL_INVALID_HANDLE
	#endif //defined(CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
#endif //defined (CONFIG_MODULE_INDEPENDENT)

/*
 * Some DAB tests check the values returned by ETAL
 * agains some known values. Obviously this requires
 * using a signal generator and a known ETI file.
 *
 * The values below are for WorldDMB's ETI Library file:
 * DE-Augsburg-20090713
 */
#define ETAL_DAB_ETI_ECC              0xE0
#define ETAL_DAB_ETI_EID              0x106E
#define ETAL_DAB_ETI_UEID             ((ETAL_DAB_ETI_ECC << 16) | ETAL_DAB_ETI_EID)
#define ETAL_DAB_ETI_LABEL            "Augsburg"
#define ETAL_DAB_ETI_BITMAP           0xFF00
#define ETAL_DAB_ETI_CHARSET          0

#define ETAL_DAB_ETI_SERV1_SID        0xd210
#define ETAL_DAB_ETI_SERV1_SC         0
#define ETAL_DAB_ETI_SERV1_SUBCH      6
#define ETAL_DAB_ETI_SERV1_LABEL     "Deutschlandfunk "
#define ETAL_DAB_ETI_SERV1_BITMAP     0x8110
#define ETAL_DAB_ETI_SERV1_CHARSET    0
#define ETAL_DAB_ETI_SERV1_SC_LABEL  "Deutschlandfunk "
#define ETAL_DAB_ETI_SERV1_SC_BITMAP  0x8110
#define ETAL_DAB_ETI_SERV1_SC_CHARSET 0

#define ETAL_DAB_ETI_SERV2_SID        0xe0d0106e  // MobiDat Augsburg (data)
#define ETAL_DAB_ETI_SERV2_LABEL     "MobiDat Augsburg"
#define ETAL_DAB_ETI_SERV2_BITMAP     0x8fe0
#define ETAL_DAB_ETI_SERV2_CHARSET    0
#define ETAL_DAB_ETI_SERV2_SC_LABEL  "MOT BWS BDR     "
#define ETAL_DAB_ETI_SERV2_SC_BITMAP  0xff00
#define ETAL_DAB_ETI_SERV2_SC_CHARSET 0

#define ETAL_DAB_ETI_SERV3_SID        0x1d12      // Fantasy Aktuell (audio)
#define ETAL_DAB_ETI_SERV4_SID        0xd220      // DKULTUR (audio) (SLS-XPAD)
#define ETAL_DAB_ETI_SERVD1_SID       0xe0d7106e  // EPG Augsburg    (data)
#define ETAL_DAB_ETI_SERVD1_ADDRESS   7           // EPG Augsburg    (data)
#define ETAL_DAB_ETI_SERVD2_SID       0xe0d0106e  // MobiDat Augsburg(data)
#define ETAL_DAB_ETI_SERVD2_ADDRESS   1           // MobiDat Augsburg(data)
#define ETAL_DAB_ETI_SERVD3_SID       0xe0d1106e  // TPEG Test D (data)

/*
 * Values for TPEG-Fraunhofer-2010-10-25-1
 */
#define ETAL_DAB_ETI2_ECC             0xE0
#define ETAL_DAB_ETI2_EID             0xDDAB
#define ETAL_DAB_ETI2_UEID            ((ETAL_DAB_ETI2_ECC << 16) | ETAL_DAB_ETI2_EID)

#define ETAL_DAB_ETI2_SERVD1_SID      0xE0D12345
#define ETAL_DAB_ETI2_SUBCH1          16 // RTM - TPEG


/*
 * Values for DE-Bayern-20090730
 */
#define ETAL_DAB_ETI3_ECC             0xE0
#define ETAL_DAB_ETI3_EID             0x1008
#define ETAL_DAB_ETI3_UEID            ((ETAL_DAB_ETI3_ECC << 16) | ETAL_DAB_ETI3_EID)
#define ETAL_DAB_ETI3_SERV1_SID       0xD319 // Rock Antenne - this contains SLS over X-PAD
#define ETAL_DAB_ETI3_SERV2_SID       0xD312 // BAYERN 2 plus
#define ETAL_DAB_ETI3_SERV3_SID       0xD317 // on3radio
#define ETAL_DAB_ETI3_SERV4_SID       0xD31B // Radio Galaxy - this contains SLS over X-PAD
#define ETAL_DAB_ETI3_SERVD1_SID      0xE0D01007 // MobileInfo     (address 4) - TPEG
#define ETAL_DAB_ETI3_SERVD2_SID      0xE0D01008 // MobiDat Bayern (address 1)
#define ETAL_DAB_ETI3_SERVD3_SID      0xE0D11008 // MILEurope      (address 9) - TPEG
#define ETAL_DAB_ETI3_SERVD4_SID      0xE0D21008 // TPEG-BR-Test   (address 2) - TPEG
#define ETAL_DAB_ETI3_SERVD5_SID      0xE0D41008 // EPG Bayern     (address 5) - EPG
#define ETAL_DAB_ETI3_SUBCH1          20 // BAYERN 2 plus - audio
#define ETAL_DAB_ETI3_SUBCH2          17 // BAYERN 4 Klassik - audio
#define ETAL_DAB_ETI3_SUBCH3          16 // on3radio - audio
#define ETAL_DAB_ETI3_SUBCH4           7 // Rock Antenne - audio
#define ETAL_DAB_ETI3_SUBCH5          11 // all the data services

/*
 * Values for FraunhoferIIS-JournalineVariety-2009-04-24
 */
#define ETAL_DAB_ETI4_ECC             0xE0
#define ETAL_DAB_ETI4_EID             0xDAB6
#define ETAL_DAB_ETI4_UEID            ((ETAL_DAB_ETI4_ECC << 16) | ETAL_DAB_ETI4_EID)
#define ETAL_DAB_ETI4_SERV1_SID       0xD121 // Layer II
#define ETAL_DAB_ETI4_SERV2_SID       0xD123 // Layer II short
#define ETAL_DAB_ETI4_SERVD1_SID      0xE0DCAFE1 // Journaline

/*
 * Values for TPEG-Fraunhofer-2010-10-25-1
 */
#define ETAL_DAB_ETI5_ECC             0xE0
#define ETAL_DAB_ETI5_EID             0xDAE1
#define ETAL_DAB_ETI5_UEID            ((ETAL_DAB_ETI5_ECC << 16) | ETAL_DAB_ETI5_EID)
#define ETAL_DAB_ETI5_SERV1_SID       0xD011 // Layer II

/*
 * Values for FraunhoferIIS-International_Fig2_DLPlus-2010-01-28.eti
 * ETI file for PAD DL PLUS
 */
#define ETAL_DAB_ETI6_ECC             0xE1
#define ETAL_DAB_ETI6_EID             0x1111
#define ETAL_DAB_ETI6_UEID            ((ETAL_DAB_ETI6_ECC << 16) | ETAL_DAB_ETI6_EID)
#define ETAL_DAB_ETI6_SERV1_SID       0x7234 // Moscow

/*
 * The values below are for HD Radio FM RDM1-IBDC1 module:
 */
#define ETAL_HDRADIO_SERV1_SIS_STATION_NAME     "WIBI-FM"
#define ETAL_HD_SPS_FM                0x03

/*
 * The values below are for HD Radio AM IQ file IB_AMr208_e1awfa05.wv:
 */
#define ETAL_HD_SPS_AM                0x00

#define ETAL_TEST_ARRAY_SIZE          1024

#ifdef CONFIG_HOST_OS_TKERNEL
// File name path : /sda = SDCard 0
#define ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME  "/sda/tuner_test_xtal.txt"
#else
#define ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME  "xtal.txt"
#endif


/* define for RDS test */
#define ETAL_TEST_GET_RDS_MIN           1
#define ETAL_TEST_GETRDS_FM_DURATION    (15 * ETAL_TEST_ONE_SECOND)

/* tracing */
#define ETAL_TEST_OUTPUT_LEVEL         TR_LEVEL_SYSTEM_MIN

/* no user configuration below this point */

#define OUTPUT_FILE_NAME_LEN          256

#define THREAD_RUNNING                0
#define THREAD_STOPPED                1

#define ETAL_TEST_ONE_SECOND          1000

#define ETAL_TEST_NORMAL_LEVEL         (TR_LEVEL_USER_4 + 1)
#define ETAL_TEST_VERBOSE_LEVEL        (TR_LEVEL_USER_4 + 2)
#define ETAL_TEST_ERROR_LEVEL          (TR_LEVEL_USER_4 + 3)
#define ETAL_TEST_REPORT_LEVEL         (TR_LEVEL_USER_4 + 4)
#define ETAL_TEST_REPORT0_LEVEL        (TR_LEVEL_USER_4 + 5)


// Quality values
#ifdef CONFIG_APP_TEST_IN_LE_MANS
#define ETAL_TEST_MIN_FM_FIELDSTRENGTH   10
#define ETAL_TEST_MAX_FM_FIELDSTRENGTH  100
#else
#define ETAL_TEST_MIN_FM_FIELDSTRENGTH   20
#define ETAL_TEST_MAX_FM_FIELDSTRENGTH  100
#endif

#ifdef CONFIG_APP_TEST_IN_LE_MANS
	#define ETAL_TEST_MIN_AM_FIELDSTRENGTH  15
	#define ETAL_TEST_MAX_AM_FIELDSTRENGTH  100
#else
#if 0
	/* values for on-air reception of AM in awful in Agrate */
	#define ETAL_TEST_MIN_AM_FIELDSTRENGTH  -10
	#define ETAL_TEST_MAX_AM_FIELDSTRENGTH    5
#else
	/* values for signal generator test with level set to 25dbuV */
	#define ETAL_TEST_MIN_AM_FIELDSTRENGTH  15
	#define ETAL_TEST_MAX_AM_FIELDSTRENGTH  30
#endif
#endif

/*
 * Empirical values, with the DAB signal generator the
 * signal strength is circa 90
 * Don't expect any FIC errors
 */
#define ETAL_TEST_MIN_DAB_RFFIELDSTRENGTH   -100
#define ETAL_TEST_MAX_DAB_RFFIELDSTRENGTH   0
#define ETAL_TEST_MIN_DAB_BBFIELDSTRENGTH   4
#define ETAL_TEST_MAX_DAB_BBFIELDSTRENGTH   600
#define ETAL_TEST_MAX_DAB_FICERROR         0


/*!
 * \def		etalTestPrintNormal
 * 			Test application print utility.
 * 			Use for normal test output, including test result (test failure,
 * 			test success).
 */
#define etalTestPrintNormal(...)       etalTestPrint(ETAL_TEST_NORMAL_LEVEL, __VA_ARGS__)
/*!
 * \def		etalTestPrintVerbose
 * 			Test application print utility.
 * 			Use for verbose test output e.g. detailed error cause,
 * 			quality printouts.
 */
#define etalTestPrintVerbose(...)      etalTestPrint(ETAL_TEST_VERBOSE_LEVEL, __VA_ARGS__)
/*!
 * \def		etalTestPrintError
 * 			Test application print utility.
 * 			Use to communicate abnormal conditions that prevent a test
 * 			from running to completion.
 * 			A failed test is **not** an abnormal condition.
 */
#define etalTestPrintError(...)        etalTestPrint(ETAL_TEST_ERROR_LEVEL, __VA_ARGS__)
/*!
 * \def		etalTestPrintReport
 * 			Test application print utility.
 * 			Use to output the test report.
 * 			This is a fairly terse output, including test passes result, the test name
 * 			and the PASS/FAIL count.
 */
#define etalTestPrintReport(...)       etalTestPrint(ETAL_TEST_REPORT_LEVEL, __VA_ARGS__)
/*!
 * \def		etalTestPrintReport0
 * 			Test application print utility.
 * 			Use to output the final test report.
 * 			This is a very terse output, including only the test name
 * 			and the PASS/FAIL count.
 */
#define etalTestPrintReport0(...)       etalTestPrint(ETAL_TEST_REPORT0_LEVEL, __VA_ARGS__)

#define ETAL_TEST_VERBOSE_ALL          0
#define ETAL_TEST_VERBOSE_NORMAL       1
#define ETAL_TEST_VERBOSE_REPORT_ERROR 2
#define ETAL_TEST_VERBOSE_REPORT       3
#define ETAL_TEST_VERBOSE_REPORT0      4
#define ETAL_TEST_VERBOSE_NONE         5

#define ETAL_IS_HDRADIO_STANDARD(_std_)   (((_std_) == ETAL_BCAST_STD_HD_FM) || ((_std_) == ETAL_BCAST_STD_HD_AM))

/*****************************************************************
| type definitions
|----------------------------------------------------------------*/
typedef enum
{
	ETAL_TEST_MODE_FM,
	ETAL_TEST_MODE_AM,
	ETAL_TEST_MODE_DAB,
	ETAL_TEST_MODE_HD_FM,
	ETAL_TEST_MODE_HD_AM,
	ETAL_TEST_MODE_NONE,
} etalTestBroadcastTy;

typedef tSInt (*etalThreadTy)(ETAL_HANDLE handle);

typedef struct
{
	etalThreadTy   entry;
	ETAL_HANDLE    hReceiver;
	OSAL_tThreadID threadId;
	tU32           status;
	tSInt          retval;
} etalTestThreadAttrTy;

typedef enum
{
	ETAL_CONFIG_NONE,
	ETAL_CONFIG_DAB,
	ETAL_CONFIG_DAB1_5,
	ETAL_CONFIG_FM1,
	ETAL_CONFIG_FM2,
	ETAL_CONFIG_FM1_FM2_VPA,
	ETAL_CONFIG_AM,
	ETAL_CONFIG_HDRADIO_FM,
	ETAL_CONFIG_HDRADIO_AM
} etalTestConfigTy;

typedef enum
{
	ETAL_TUNE_NONE,
	ETAL_TUNE_DAB,
	ETAL_TUNE_DRM,
	ETAL_TUNE_FM,
	ETAL_TUNE_AM,
	ETAL_TUNE_HDRADIO_FM,
	ETAL_TUNE_HDRADIO_AM
} etalTestTuneTy;

typedef struct {
	tU8   oQuietLevel; 
	tBool oDisableTraceHeader;
	tU8   oDoTraceConfig;
	tU32  oLoopNumber;
	tBool oLoopEndless;
	tU32  oLoopNumberInteractive;
	tBool oLoopEndlessInteractive;
	tBool oOutToFile;
	tBool oStackCheck;
#ifdef CONFIG_APP_TEST_STACK
	tU32  oStackSize;
#endif //CONFIG_APP_TEST_STACK
	tBool oSTECICheck;
	tBool oSignallingGPIO;
	tBool oTuneTime;
	tBool oStopOnErrors;
} etalTestOptionTy;

typedef enum
{
    ETAL_CALLBACK_REASON_IS_NONE          = 0,
    ETAL_CALLBACK_REASON_IS_DAB_STATUS    = 1,
    ETAL_CALLBACK_REASON_IS_COMM_ERROR    = 2
} EtalCallbackReasonEnumTy;

typedef void (*EtalTest_FunctPtr) (void *);

typedef struct
{
	tU32 STARTED;
	tU32 RESULT;
	tU32 FINISHED;
	tU32 NB_ERROR;
	tU32 FREQ_FOUND;
	tU32 FULL_CYCLE_REACHED;
	tU32 aux;      /* content depends on the test */
} etalTestEventCountTy;

/*
 * EtalRDSRaw
 */
typedef struct
{
    tU32            len;
    EtalRDSRawData  raw_data;
} EtalRDSRaw;

typedef struct
{
	tU32 DabCbInvocations;
	tU32 DabValidContext;
	tU32 FmCbInvocations;
	tU32 FmValidContext;
	tU32 AmCbInvocations;
	tU32 AmValidContext;
	tU32 HdCbInvocations;
	tU32 HdValidContext;
	tU32 HdamCbInvocations;
	tU32 HdamValidContext;
} etalTestMonitorCounterTy;

/*
 * etalTestResultTy
 */
typedef enum
{
	testPassed,
	testFailed,
	testSkipped
} etalTestResultTy;

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
#if defined (CONFIG_APP_TEST_CONFIG_RECEIVER) || defined(CONFIG_APP_TEST_DESTROY_RECEIVER) || defined (CONFIG_APP_TEST_SETMONITOR)
extern etalTestMonitorCounterTy QCount;
#endif //defined (CONFIG_APP_TEST_CONFIG_RECEIVER) || defined(CONFIG_APP_TEST_DESTROY_RECEIVER) || defined (CONFIG_APP_TEST_SETMONITOR)
#if defined (CONFIG_APP_TEST_GETRDS_DATAPATH)
	extern tU32 RdsDecodedCbInvocations;
#endif //defined (CONFIG_APP_TEST_GETRDS_DATAPATH)

extern ETAL_HANDLE handledab, handlefm, handlefm2, handlehd, handlehdam, handlehd2, handleam, handledab1_5, handlehd1_5;

extern etalTestEventCountTy etalTestSeekEventCountDAB;
extern etalTestEventCountTy etalTestSeekEventCountDAB1_5;
extern etalTestEventCountTy etalTestSeekEventCountFM;
extern etalTestEventCountTy etalTestSeekEventCountAM;
extern etalTestEventCountTy etalTestSeekEventCountHD;
extern tU32 etalTestHDTuneEventCount;
extern tU32 etalTestHDAMTuneEventCount;
extern etalTestEventCountTy etalTestScanCountFM;
extern etalTestEventCountTy etalTestScanCountAM;
extern etalTestEventCountTy etalTestScanCountHD;
extern OSAL_tSemHandle etalTestScanSem;
extern tBool vl_etalTestScan_ScanOnGoing;
extern etalTestEventCountTy etalTestLearnCountDAB;
extern etalTestEventCountTy etalTestLearnCountFM;
extern etalTestEventCountTy etalTestLearnCountAM;
extern etalTestEventCountTy etalTestLearnCountHD;
extern OSAL_tSemHandle etalTestLearnSem;
extern tBool vl_etalTestLearn_LearnOnGoing;
extern tU32 etalTestPADEventCount;
extern tU32 etalTestSeamlessEstimationCount;
extern tU32 etalTestSeamlessSwitchingCount;
#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
extern tU32 etalTestRDSSeekFrequencyFound;
OSAL_tSemHandle etalTestRDSSeekFinishSem;
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH

#ifdef CONFIG_APP_TEST_RDS_SEEK
extern OSAL_tSemHandle etalTestRDSSeekSem;
#endif //CONFIG_APP_TEST_RDS_SEEK

extern etalTestOptionTy etalTestOption;
extern tBool etalTestInitializeDone;

/* variables for RDS tests */
extern tU32 RdsDecodedCbInvocations, RdsDecodedCbInvocations2;
#if defined (CONFIG_ETAL_HAVE_ETALTML)
extern EtalRDSData rdsDecodedDatapathData, rdsDecodedDatapathData2;
extern EtalRDSData etalTestRDSReference, etalTestRDSReference2;
#endif //CONFIG_ETAL_HAVE_ETALTML
extern tU32 RdsRawCbInvocations, RdsRawCbInvocations2;
extern EtalRDSRaw rdsRawDatapathData, rdsRawDatapathData2;
extern EtalRDSRaw etalTestRDSRawReference;

/* variables for audio mono test */
extern tU32 etalTestInfoFmStereoCount;
extern tU8 etalTestInfoFmStereoExpected;
extern ETAL_HANDLE etalTestInfoFmStereoReceiverExpected;

/* variables for receiver alive test */
extern OSAL_tSemHandle etalTestAliveErrSem;
extern tBool etalTestReceiverAliveError;
extern ETAL_HANDLE etalTestReceiverAliveErrorHandleExpected;

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
#ifdef CONFIG_HOST_OS_TKERNEL
	int etaltest_EntryPoint(etalTestOptionTy *pI_etalTestOption, EtalTraceConfig *pI_etalTestTraceConfig);
#endif //CONFIG_HOST_OS_TKERNEL

/*
 * general use
 */
void  etalTestMonitorcb(EtalBcastQualityContainer* pQuality, void* vpContext);
tVoid userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pvParam);
tSInt etalTestDoConfigSingle(etalTestConfigTy conf, ETAL_HANDLE *handle);
tSInt etalTestUndoConfigSingle(ETAL_HANDLE *handle);
tSInt etalTestDoTuneSingle(etalTestTuneTy conf, ETAL_HANDLE hReceiver);
tSInt etalTestUndoTuneSingle(etalTestTuneTy conf, ETAL_HANDLE hReceiver);
tSInt etalTestDoServiceSelectAudio(ETAL_HANDLE hReceiver, tU32 ueid, tU32 sid);

/*
 * test entry points
 */
tSInt etalTestDirectCommands(void);
tSInt etalTestStartETAL(EtalTraceConfig *tr_config);
tSInt etalTestInitialize(EtalTraceConfig *tr_config);
tVoid etalTestStartup(void);
tSInt etalTestCapabilities(void);
tSInt etalTestConfigReceiver(void);
tSInt etalTestDestroyReceiver(void);
tSInt etalTestAudioSelect(void);
tSInt etalTestTuneReceiver(void);
tSInt etalTestGetQuality(void);
tSInt etalTestGetCFData(void);
tSInt etalTestSetMonitor(void);
tSInt etalTestReceiverAlive(void);
tSInt etalTestAudio(void);
tSInt etalTestGetFrequency(void);
tSInt etalTestAF(void);
tSInt etalTestChangeBand(void);
tSInt etalTestAdvancedTuning(void);
tSInt etalTestDataServices(void);
tSInt etalTestAutonotification(void);
tSInt etalTestSystemData(void);
tSInt etalTestDabData(void);
#if defined (CONFIG_ETAL_HAVE_ETALTML)
tBool etalTestGetRDSCompare(EtalRDSData *data, EtalRDSData *reference);
tVoid etalTestGetRDSAccumulate(EtalRDSData *dst, EtalRDSData *src);
#endif //CONFIG_ETAL_HAVE_ETALTML
tSInt etalTestGetRDSRawCompare(EtalRDSRaw *data, EtalRDSRaw *reference, tBool fastPiMode);
tVoid etalTestGetRDSRawAccumulate(EtalRDSRaw *dst, EtalRDSRawData *src, tU32 len);
tSInt etalTestGetRAWRDS(void);
tSInt etalTestGetRDS(void);
tSInt etalTestGetRadiotext(void);
tSInt etalTestRDSSeek(void);
tSInt etalTestRDSAFStrategy(void);
tSInt etalTestSeek(void);
tSInt etalTestManualSeek(void);
tSInt etalTestScan(void);
tSInt etalTestLearn(void);
tSInt etalTestReadParameter(void);
tSInt etalTestWriteParameter(void);
tSInt etalTestSeamless(void);
tVoid etalTestSeamlessEstimation_Resp(EtalSeamlessEstimationStatus *seamless_estimation_status);
tVoid etalTestSeamlessSwitching_Resp(EtalSeamlessSwitchingStatus *seamless_switching_status);
tSInt etalTestServiceSelectDCOP(void);
tSInt etalTestTuneDCOP(void);
tSInt etalTestXTALalignment(void);
tSInt etalTestGetVersion(void);
tSInt etalTestFmVpa(void);
tSInt etalTestDebug(void);
tVoid etalTestRDSSeek_SetSeekResult(EtalSeekStatus * pSeekStatus);

/*
 * special tests
 */
tVoid etalTestStack(void);
tU32  etalTestSTECI(void);
tVoid etalTestSignallingGPIO(void);
tVoid etalTestTuneTiming(void);

/*
 * concurrency tests
 */
tSInt etalTestThreadSpawn(etalTestThreadAttrTy *attr);
tSInt etalTestThreadWait(etalTestThreadAttrTy *attr);


/*
 * auxiliary functions
 */
tSInt etalTestReadXTALalignmentRead(tU32 *calculatedAlignment);
tSInt etalTestRegisterCallbacks(EtalTest_FunctPtr fnctPtr, EtalCallbackReasonEnumTy callbackReason);
tSInt etalTestDeRegisterCallbacks(tSInt fnctId);
tBool etalTestServeCallbacks(EtalCallbackReasonEnumTy reason, tPVoid pvParam);
tVoid etalTestResetEventCount(etalTestEventCountTy *count);
tVoid etalTestGetParameterCMOST(tU8 *cmd, tU32 *p);

/*
 * print functions
 */
tCString ETAL_STATUS_toString(ETAL_STATUS s); // duplicated from etalinternal.h to avoid including it
tCString etalTestStandard2Ascii(EtalBcastStandard std);
tVoid etalTestPrint(tU16 level, tCString coszFormat, ...);
tVoid etalTestPrintBuffer(tChar *prefix, tU8* pBuffer, tU32 len); // normally under #if 0
tVoid etalTestPrintCapabilites(EtalHwCapabilities *cap);
tVoid etalTestPrintRDSRaw(EtalRDSRaw *rds, ETAL_HANDLE hReceiver);
#if defined (CONFIG_ETAL_HAVE_ETALTML)
tVoid etalTestPrintRDS(EtalRDSData *rds, tPChar prefix);
tVoid etalTestPrintRadioText(EtalTextInfo *radio_text);
#endif //CONFIG_ETAL_HAVE_ETALTML
tVoid etalTestPrintEnsembleList(EtalEnsembleList *list);
tVoid etalTestPrintEnsembleData(tU32 eid, tU8 charset, tChar *label, tU16 bitmap);
tVoid etalTestPrintServiceList(tS32 eid, EtalServiceList *list);
tVoid etalTestPrintServiceDataDAB(tU32 sid, EtalServiceInfo *serv, EtalServiceComponentList *sc);
tVoid etalTestPrintVerboseContainer(EtalBcastQualityContainer *q);
tVoid etalTestPrintInitStatus(tVoid);
tVoid etalTestPrintReportTestStart(tCString coszFormat, tU32 test_index);
tVoid etalTestPrintReportTestEnd(tCString coszFormat, tU32 test_index, tSInt retosal);
tVoid etalTestPrintReportPassStart(tU8 subTestNb, etalTestBroadcastTy std, tCString coszFormat, ...);
tVoid etalTestPrintReportPassEnd(etalTestResultTy res);
tCString etalTestStandard2String(etalTestBroadcastTy std);
