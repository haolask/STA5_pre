//!
//!  \file 		target_config_validate.c
//!  \brief 	<i><b> Build configuration validator </b></i>
//!  \details   This file is compiled before any build;
//!             it only contains preprocessor macros to verify the build configuration
//!  \author 	Raffaele Belardi
//!

#include "target_config.h"

/*
 * To bypass the configuration checks put the whole content of this
 * file under #if 0 (and keep the pieces when ETAL breaks)
 */

/*************************************************
 * Configuration checks for Board support
 *
 */
#if !defined (CONFIG_BOARD_CMOST_MAIN) && !defined (CONFIG_BOARD_ACCORDO2) && !defined(CONFIG_BOARD_ACCORDO5)
	#error "Undefined board configuration"
#endif

#if defined (CONFIG_MODULE_INDEPENDENT) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) && !defined (CONFIG_ETAL_ENABLE_CMOST_SDM_CLOCK)
	#warning "CMOST SDM clock not enabled, communication with DAB DCOP may fail"
#endif

/* TODO does not work */
#if !defined (CONFIG_MODULE_INTEGRATED) && (ETAL_CAPA_MAX_TUNER >= 2)
	#warning "ETAL configured for single CMOST application but ETAL_CAPA_MAX_TUNER is set for multiple CMOST"
#endif

/* TODO does not work */
#if !defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL) && (ETAL_CAPA_MAX_DIVERSITY >= 2) 
	#warning "ETAL configured for multiple diversity modes, but CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL not defined"
#endif

/*************************************************
 * Configuration checks for OS support
 *
 */
#if !defined (CONFIG_HOST_OS_LINUX) && !defined (CONFIG_HOST_OS_WIN32)
	#error "Usupported or not configured OS"
#endif

/*************************************************
 * Configuration checks for TOOLCHAIN
 *
 * When building for CONFIG_BOARD_ACCORDO2 or CONFIG_BOARD_ACCORDO5 normally you want to use the cross-compiler,
 * not the native compiler, because you'll be building for an ARM target.
 *
 * Viceversa when building for CONFIG_BOARD_CMOST_MAIN since ETAL does not support
 * natively the FTDI drivers used to connect to the board, you'll need to build
 * for a Windows PC, thus you'll need the native GCC.
 */
#if defined (CONFIG_HOST_OS_WIN32) && defined (CONFIG_COMPILER_GCC_CROSS)
	#error "CONFIG_HOST_OS_WIN32 does not support CONFIG_COMPILER_GCC_CROSS"
#endif

#if defined (CONFIG_BOARD_ACCORDO2) && !defined (CONFIG_COMPILER_GCC_CROSS)
	#warning "You probably want CONFIG_COMPILER_GCC_CROSS with CONFIG_BOARD_ACCORDO2"
#endif

#if defined (CONFIG_BOARD_ACCORDO5) && !defined (CONFIG_COMPILER_GCC_CROSS)
	#warning "You probably want CONFIG_COMPILER_GCC_CROSS with CONFIG_BOARD_ACCORDO5"
#endif

#if defined (CONFIG_BOARD_CMOST_MAIN) && !defined (CONFIG_COMPILER_GCC_NATIVE)
	#warning "You probably want CONFIG_COMPILER_GCC_NATIVE with CONFIG_BOARD_CMOST_MAIN"
#endif

/*************************************************
 * Configuration checks for Read Write Parameter
 *
 * CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS is available as external API but also
 * required by some internal commands (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT and CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
 *
 * Older firmware revisions do not provide the device-dependent naming
 * scheme for the memory map file, we cannot support them
 */
#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)

    #if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC) || \
        defined (CONFIG_ETAL_SUPPORT_CMOST_DOT_T_CUT_AB)  || \
        defined (CONFIG_ETAL_SUPPORT_CMOST_DOT_S_CUT_2_1)
        #error "Read/Write parameter not supported on the selected CMOST silicon"
        #error "Disable:"
        #error "  ETAL_HAVE_READ_WRITE_PARAMETERS"
        #error "  ETAL_HAVE_XTAL_ALIGNMENT"
        #error "  ETALTML_HAVE_SERVICE_FOLLOWING"
    #endif

#endif

/*************************************************
 * Configuration checks for Seek
 *
 * Seek for HD is implemented in part in the STAR seek procedure
 * thus the presence of a STAR is a requirement
 */
#if defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) || defined (CONFIG_ETAL_HAVE_ALL_API)

    #if !defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
        #error "Seek for HD requires STAR"
    #endif

#endif

/*************************************************
 * Configuration checks for Service Following
 *
 */
#if defined CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

    #if !defined (CONFIG_ETAL_SUPPORT_DCOP)
        #error "CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING requires CONFIG_ETAL_SUPPORT_DCOP"
    #endif
#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

/*************************************************
 * Configuration checks for Seamless Switching
 *
 */
#if defined (CONFIG_ETAL_HAVE_SEAMLESS)

    #if !defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
        #error "CONFIG_ETAL_HAVE_SEAMLESS requires CONFIG_ETAL_SUPPORT_DCOP_MDR"
    #endif

#endif // CONFIG_ETAL_HAVE_SEAMLESS

/*************************************************
 * Configuration checks for Receiver Alive
 *
 * CONFIG_COMM_DRIVER_EMBEDDED is required because the test accesses the BSP functions
 * to issue a reset to the HW; it should be updated to use the external driver interfaces
 * if CONFIG_COMM_DRIVER_EXTERNAL is selected
 */
#if defined (CONFIG_APP_TEST_RECEIVER_ALIVE)

    #if !defined (CONFIG_ETAL_HAVE_RECEIVER_ALIVE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_RECEIVER_ALIVE requires ETAL_HAVE_RECEIVER_ALIVE"
    #endif

    #if !defined (CONFIG_COMM_DRIVER_EMBEDDED)
        #error"CONFIG_APP_TEST_RECEIVER_ALIVE requires CONFIG_COMM_DRIVER_EMBEDDED"
    #endif

#endif // CONFIG_APP_TEST_RECEIVER_ALIVE

/*************************************************
 * Configuration checks for OSALCORE TARGET
 *
 */
#if defined (CONFIG_APP_OSALCORE_TESTS)

    #if defined (CONFIG_TRACE_ASYNC)
        #error "CONFIG_APP_OSALCORE_TESTS does not support CONFIG_TRACE_ASYNC"
    #endif

#endif // CONFIG_APP_OSALCORE_TESTS

/*************************************************
 * Configuration checks for TUNER DRIVER TARGET
 *
 * CONFIG_COMM_DRIVER_EMBEDDED: we presume CONFIG_APP_TUNERDRIVER_LIBRARY will be used
 * to build the CMOST driver for embedded use (small footprint) so requiring
 * TCP/IP (as the CONFIG_COMM_DRIVER_EXTERNAL does) makes no sense.
 */
#if defined (CONFIG_APP_TUNERDRIVER_LIBRARY)

    #if !defined (CONFIG_COMM_DRIVER_EMBEDDED)
		#error "CONFIG_APP_TUNERDRIVER_LIBRARY requires CONFIG_COMM_DRIVER_EMBEDDED"
	#endif

#endif

/*************************************************
 * Configuration checks for Hardware configuration
 *
 */
#if defined (CONFIG_BOARD_ACCORDO2) || defined (CONFIG_BOARD_ACCORDO5)

    #if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) || defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
        #if !defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && !defined (CONFIG_ETAL_SUPPORT_CMOST_DOT)
            #error "CONFIG_ETAL_SUPPORT_DCOP_MDR or CONFIG_ETAL_SUPPORT_DCOP_HDRADIO requires CONFIG_ETAL_SUPPORT_CMOST_STAR or CONFIG_ETAL_SUPPORT_CMOST_DOT"
        #endif
    #endif

#endif // CONFIG_BOARD_ACCORDO2||CONFIG_BOARD_ACCORDO5


/*************************************************
 * Configuration test common to all TEST
 *
 * Ensure some some output is produced for the test application otherwise
 * it is barely usable
 */
#if defined (CONFIG_APP_ETAL_TEST) && defined (CONFIG_TRACE_ENABLE)

    #if !defined (CONFIG_TRACE_DEFAULT_LEVEL) || (CONFIG_TRACE_DEFAULT_LEVEL < TR_LEVEL_SYSTEM_MIN)
        #warning "CONFIG_APP_ETAL_TEST with CONFIG_TRACE_DEFAULT_LEVEL < TR_LEVEL_SYSTEM_MIN will not produce any useful output"
    #endif

#endif

/*
 * Check if DCOP is needed
 */
#if defined (CONFIG_APP_TEST_DAB) && !defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
	#error "Cannot do DAB without DAB DCOP"
#endif

#if defined (CONFIG_APP_TEST_HDRADIO_FM) && !defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
	#error "Cannot do HD without HD DCOP"
#endif

/*
 * Concurrent test checks
 */
#if defined (CONFIG_APP_TEST_ONLY_CONCURRENT) || defined (CONFIG_APP_TEST_BOTH_CONCURRENT_AND_SEQUENTIAL)
	#if !defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL) && !defined(CONFIG_MODULE_INTEGRATED)
		#error "Cannot do concurrent tests without STAR-T or DOT-T or the MTD"
	#endif
	#if !defined(CONFIG_APP_TEST_DAB) && !defined (CONFIG_APP_TEST_FM)
		#error "Cannot do concurrent tests without DAB or FM"
	#endif
#endif

/*************************************************
 * Configuration checks for Initialization TEST
 *
 */
#if defined (CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM)

    #if !defined (CONFIG_COMM_CMOST_CUSTOMER_SETTINGS)
        #error "CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM requires CONFIG_COMM_CMOST_CUSTOMER_SETTINGS"
    #endif

#endif // CONFIG_APP_TEST_INITIALIZATION_CUSTOMPARAM

/*************************************************
 * Configuration checks for Tune Receiver TEST
 *
 */
#if defined (CONFIG_APP_TEST_TUNE_RECEIVER)

    #if !defined (CONFIG_ETAL_HAVE_TUNE_BOTH) && !defined (CONFIG_ETAL_HAVE_TUNE_BLOCKING) && \
        !defined (CONFIG_ETAL_HAVE_TUNE_ASYNC)&& !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_TUNE_RECEIVER requires CONFIG_ETAL_HAVE_TUNE"
    #endif

#endif // CONFIG_APP_TEST_TUNE_RECEIVER

/*************************************************
 * Configuration checks for Destroy Receiver TEST
 *
 * CONFIG_ETAL_HAVE_QUALITY_MONITOR is required because the test sets up a monitor
 * and checks if no events are received after receiver destruction
 */
#if defined (CONFIG_APP_TEST_DESTROY_RECEIVER)

    #if !defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_DESTROY_RECEIVER required CONFIG_ETAL_HAVE_QUALITY_MONITOR"
    #endif

#endif // CONFIG_APP_TEST_DESTROY_RECEIVER

/*************************************************
 * Configuration checks for Get Quality TEST
 *
 */
#if defined (CONFIG_APP_TEST_GETQUALITY)

    #if !defined (CONFIG_ETAL_HAVE_QUALITY) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_GETQUALITY requires CONFIG_ETAL_HAVE_QUALITY"
    #endif

    #if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
        #error "CONFIG_APP_TEST_GETQUALITY requires CONFIG_ETAL_SUPPORT_CMOST_STAR"
    #endif

#endif // CONFIG_APP_TEST_GETQUALITY

/*************************************************
 * Configuration checks for Quality Monitor TEST
 *
 */
#if defined (CONFIG_APP_TEST_SETMONITOR)

    #if !defined (CONFIG_ETAL_HAVE_QUALITY_MONITOR) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_SETMONITOR requires CONFIG_ETAL_HAVE_QUALITY_MONITOR"
    #endif

    #if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
        #error "CONFIG_APP_TEST_SETMONITOR requires CONFIG_ETAL_SUPPORT_CMOST_STAR"
    #endif

#endif // CONFIG_APP_TEST_SETMONITOR

/*************************************************
 * Configuration checks for Audio TEST
 *
 */
#if defined (CONFIG_APP_TEST_AUDIO)

    #if !defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error i"CONFIG_APP_TEST_AUDIO requires CONFIG_ETAL_HAVE_AUDIO_CONTROL"
    #endif

#endif // CONFIG_APP_TEST_AUDIO

/*************************************************
 * Configuration checks for Advanced Tuning TEST
 *
 */
#if defined (CONFIG_APP_TEST_ADVANCED_TUNING)

    #if !defined (CONFIG_ETAL_HAVE_ADVTUNE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error i"CONFIG_APP_TEST_ADVANCED_TUNING requires CONFIG_ETAL_HAVE_ADVTUNE"
    #endif

#endif // CONFIG_APP_TEST_ADVANCED_TUNING


#if defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY)

 
	// no specific dependances
	// RDS / TML requiredstr

#endif

/*************************************************
 * Configuration checks for Get RDS TEST
 *
 */
#if defined (CONFIG_APP_TEST_GETRDS)

    #if !defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
        #error "CONFIG_APP_TEST_GETRDS requires CONFIG_ETALTML_HAVE_RADIOTEXT"
    #endif

    #if !defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
        #error "CONFIG_APP_TEST_GETRDS requires CONFIG_ETAL_SUPPORT_CMOST_STAR"
    #endif

#endif // CONFIG_APP_TEST_GETRDS

/*************************************************
 * Configuration checks for Get RAW RDS TEST
 *
 */
#if defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH)

    #if !defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
        #error "CONFIG_APP_TEST_GETRAWRDS_DATAPATH requires CONFIG_ETAL_SUPPORT_CMOST_STAR"
    #endif

#endif

/*************************************************
 * Configuration checks for Get Radiotext TEST
 *
 */
#if defined (CONFIG_APP_TEST_GETRADIOTEXT)

    #if !defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
        #error "CONFIG_APP_TEST_GETRADIOTEXT requires CONFIG_ETALTML_HAVE_RADIOTEXT"
    #endif

    #if !defined (CONFIG_ETAL_HAVE_ADVTUNE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_GETRADIOTEXT requires CONFIG_ETAL_HAVE_ADVTUNE"
    #endif

#endif // CONFIG_APP_TEST_GETRADIOTEXT

/*************************************************
 * Configuration checks for Manual Seek TEST
 *
 */
#if defined (CONFIG_APP_TEST_MANUAL_SEEK)

    #if !defined (CONFIG_ETAL_HAVE_MANUAL_SEEK) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_MANUAL_SEEK requires CONFIG_ETAL_HAVE_MANUAL_SEEK"
    #endif

#endif // CONFIG_APP_TEST_MANUAL_SEEK

/*************************************************
 * Configuration checks for Learn TEST
 *
 */
#if defined (CONFIG_APP_TEST_LEARN)

    #if !defined (CONFIG_ETALTML_HAVE_LEARN)
        #error "CONFIG_APP_TEST_LEARN requires CONFIG_ETALTML_HAVE_LEARN"
    #endif

#endif // CONFIG_APP_TEST_LEARN

/*************************************************
 * Configuration checks for Scan TEST
 *
 */
#if defined (CONFIG_APP_TEST_SCAN)

    #if !defined (CONFIG_ETALTML_HAVE_SCAN)
        #error "CONFIG_APP_TEST_SCAN requires CONFIG_ETALTML_HAVE_SCAN"
    #endif

#endif // CONFIG_APP_TEST_SCAN

/*************************************************
 * Configuration checks for Dab Data TEST
 *
 */
#if defined (CONFIG_APP_TEST_DABDATA)

    #if !defined (CONFIG_ETAL_HAVE_ADVTUNE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_DABDATA requires CONFIG_ETAL_HAVE_ADVTUNE"
    #endif

#endif // CONFIG_APP_TEST_DABDATA

/*************************************************
 * Configuration checks for System Data TEST
 *
 */
#if defined (CONFIG_APP_TEST_SYSTEMDATA)

    #ifdef CONFIG_APP_TEST_DAB
        #if !defined (CONFIG_ETAL_HAVE_SYSTEMDATA_DAB) && !defined (CONFIG_ETAL_HAVE_ALL_API)
            #error "CONFIG_APP_TEST_SYSTEMDATA requires CONFIG_ETAL_HAVE_SYSTEMDATA_DAB"
        #endif
    #endif

    #if !defined (CONFIG_ETAL_HAVE_SYSTEMDATA) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_SYSTEMDATA requires CONFIG_ETAL_HAVE_SYSTEMDATA"
    #endif

    #if !defined (CONFIG_ETAL_HAVE_ADVTUNE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_SYSTEMDATA requires CONFIG_ETAL_HAVE_ADVTUNE"
    #endif

#endif // CONFIG_APP_TEST_SYSTEMDATA

/*************************************************
 * Configuration checks for DCOP Service Select TEST
 *
 */
#if defined (CONFIG_APP_TEST_SERVICE_SELECT_DCOP)

    #if !defined (CONFIG_ETAL_HAVE_ADVTUNE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_SERVICE_SELECT_DCOP requires CONFIG_ETAL_HAVE_ADVTUNE"
    #endif

#endif // CONFIG_APP_TEST_SERVICE_SELECT_DCOP

/*************************************************
 * Configuration checks for Read Write Parameter TEST
 *
 */
#if defined (CONFIG_APP_TEST_READ_WRITE_PARAMETER)

    #if !defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_READ_WRITE_PARAMETER requires CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS"
    #endif

#endif // CONFIG_APP_TEST_READ_WRITE_PARAMETER

/*************************************************
 * Configuration checks for Data Services TEST
 *
 */
#if defined (CONFIG_APP_TEST_DATASERVICES)

    #if !defined (CONFIG_ETAL_HAVE_DATASERVICES) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_DATASERVICES requires CONFIG_ETAL_HAVE_DATASERVICES"
    #endif

#endif // CONFIG_APP_TEST_DATASERVICES

/*************************************************
 * Configuration checks for AF TEST
 *
 */
#if defined (CONFIG_APP_TEST_ALTERNATE_FREQUENCY)

    #if !defined (CONFIG_ETAL_HAVE_ALTERNATE_FREQUENCY) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_ALTERNATE_FREQUENCY requires CONFIG_ETAL_HAVE_ALTERNATE_FREQUENCY"
    #endif

#endif // CONFIG_APP_TEST_ALTERNATE_FREQUENCY

/*************************************************
 * Configuration checks for Seamless Switching TEST
 *
 * CONFIG_ETAL_HAVE_ADVTUNE is needed for the etal_service_select API
 */
#if defined (CONFIG_APP_TEST_SEAMLESS)

    #if (!defined (CONFIG_ETAL_HAVE_ALL_API)) && (!defined(CONFIG_ETAL_HAVE_SEAMLESS))
        #error "CONFIG_APP_TEST_SEAMLESS requires CONFIG_ETAL_HAVE_ALL_API or CONFIG_ETAL_HAVE_SEAMLESS"
    #endif

    #if !defined (CONFIG_ETAL_HAVE_AUDIO_CONTROL) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_SEAMLESS requires CONFIG_ETAL_HAVE_ALL_API or CONFIG_ETAL_HAVE_AUDIO_CONTROL"
    #endif

    #if !defined (CONFIG_ETAL_HAVE_ADVTUNE) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_SEAMLESS requires CONFIG_ETAL_HAVE_ALL_API or CONFIG_ETAL_HAVE_ADVTUNE"
    #endif

#endif // CONFIG_APP_TEST_SEAMLESS

/*************************************************
 * Configuration checks for GET VERSION
 *
 */
#if defined (CONFIG_APP_TEST_GET_VERSION)

    #if !defined (CONFIG_ETAL_HAVE_GET_VERSION) && !defined (CONFIG_ETAL_HAVE_ALL_API)
        #error "CONFIG_APP_TEST_GET_VERSION requires CONFIG_ETAL_HAVE_GET_VERSION"
    #endif

#endif // CONFIG_APP_TEST_GET_VERSION


/*
 * add other checks
 */

