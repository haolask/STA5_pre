/*
 * Automatically generated C config: don't edit
 * Linux kernel version: 
 * Thu Jul 13 18:03:27 2017
 */
#define AUTOCONF_INCLUDED

/*
 * Hardware System
 */
#undef CONFIG_BOARD_CMOST_MAIN
#undef CONFIG_BOARD_ACCORDO2
#define CONFIG_BOARD_ACCORDO5 1

/*
 * Target Board Configuration
 */
#define CONFIG_BOARD_ACCORDO2_SPI0_IS_32766 1
#undef CONFIG_BOARD_ACCORDO2_SPI0_IS_0
#define CONFIG_MODULE_INTEGRATED 1
#define CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707 1
#undef CONFIG_MODULE_INTEGRATED_WITH_TDA7707_TDA7708
#define CONFIG_DIGITAL_AUDIO 1
#undef CONFIG_MODULE_INDEPENDENT
#define CONFIG_BOARD_ACCORDO5_M3 1
#undef CONFIG_ETAL_ENABLE_CMOST_SDM_CLOCK

/*
 * Tuner support
 */
#define CONFIG_ETAL_SUPPORT_CMOST 1

/*
 * CMOST flavour
 */
#define CONFIG_ETAL_SUPPORT_CMOST_STAR 1
#undef CONFIG_ETAL_SUPPORT_CMOST_DOT

/*
 * Tuner channels
 */
#define CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL 1
#undef CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL

/*
 * Tuner silicon version (a.k.a CUT)
 */
#undef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC
#undef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF
#undef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG
#undef CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_CA
#define CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_DA 1

/*
 * Digital Co-processor support
 */
#undef CONFIG_ETAL_SUPPORT_DCOP
#undef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#undef CONFIG_ETAL_SUPPORT_DCOP_MDR
#define CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS 1

/*
 * ETAL options
 */

/*
 * ETAL APIs
 */
#define CONFIG_ETAL_HAVE_ALL_API 1
#undef CONFIG_ETAL_HAVE_SELECTED_API

/*
 * ETAL Optional features
 */
#undef CONFIG_ETAL_HAVE_ETALTML
#undef CONFIG_ETAL_RECEIVER_ALIVE_PERIODIC_CHECK
#define CONFIG_ETAL_RECEIVER_CHECK_STATE_PERIOD 333
#undef CONFIG_ETAL_INIT_CHECK_SILICON_VERSION

/*
 * ETALTML options
 */
#undef CONFIG_ETALTML_HAVE_RDS
#undef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
#undef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
#undef CONFIG_ETALTML_HAVE_RADIOTEXT
#undef CONFIG_ETALTML_HAVE_AUTO_SEEK
#undef CONFIG_ETALTML_AUTO_SEEK_INTERNAL
#undef CONFIG_ETALTML_AUTO_SEEK_EXTERNAL
#undef CONFIG_ETALTML_HAVE_SCAN
#undef CONFIG_ETALTML_HAVE_LEARN
#undef CONFIG_ETALTML_HAVE_SEAMLESS
#undef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#undef CONFIG_ETALTML_HAVE_RDS_STRATEGY

/*
 * Device Communication
 */
#undef CONFIG_COMM_DRIVER_EXTERNAL
#define CONFIG_COMM_DRIVER_EMBEDDED 1
#define CONFIG_COMM_CMOST_I2C 1
#undef CONFIG_COMM_CMOST_SPI
#undef CONFIG_COMM_CMOST_HAVE_DEDICATED_RESET_LINE
#undef CONFIG_COMM_ENABLE_RDS_IRQ
#undef CONFIG_COMM_CMOST_FIRMWARE_FILE
#define CONFIG_COMM_CMOST_FIRMWARE_IMAGE 1
#undef CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED
#undef CONFIG_COMM_CMOST_CUSTOMER_SETTINGS
#define CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD 1
#undef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
#define CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO 1
#undef CONFIG_COMM_DRIVER_DIRECT

/*
 * Application Configuration
 */
#define CONFIG_APP_ETAL_LIBRARY 1
#undef CONFIG_APP_ETALCORE_LIBRARY
#undef CONFIG_APP_TUNERDRIVER_LIBRARY
#undef CONFIG_APP_ETAL_TEST
#undef CONFIG_APP_OSALCORE_TESTS
#undef CONFIG_APP_ETAL_DCOP_MDR_FLASH

/*
 * Trace Settings
 */
#define CONFIG_TRACE_ENABLE 1
#undef CONFIG_TRACE_ASYNC
#undef CONFIG_TRACE_ETAL_SYSLOG

/*
 * Minimum Trace Level Configuration
 */

/*
 * Configure compile time trace levels.
 */

/*
 * Trace messages with lower level will not be compiled into the binary.
 */
#define CONFIG_TRACE_CLASS_OSALCORE 2
#define CONFIG_TRACE_CLASS_ETAL 3
#define CONFIG_TRACE_CLASS_BOOT 2
#define CONFIG_TRACE_CLASS_CMOST 2
#define CONFIG_TRACE_CLASS_TUNERDRIVER 2
#define CONFIG_TRACE_CLASS_HDRADIO 2
#define CONFIG_TRACE_CLASS_STECI 2
#define CONFIG_TRACE_CLASS_BSP 2
#define CONFIG_TRACE_CLASS_IPFORWARD 2
#define CONFIG_ENABLE_CLASS_APP_DABMW 2
#undef CONFIG_ENABLE_CLASS_APP_DABMW_SF
#define CONFIG_TRACE_CLASS_EXTERNAL 2
#define CONFIG_TRACE_DEFAULT_LEVEL 3
// #undef CONFIG_TRACE_CLASS_OSALCORE
// #undef CONFIG_TRACE_CLASS_ETAL
// #undef CONFIG_TRACE_CLASS_BOOT
// #undef CONFIG_TRACE_CLASS_CMOST
// #undef CONFIG_TRACE_CLASS_TUNERDRIVER
// #undef CONFIG_TRACE_CLASS_HDRADIO
// #undef CONFIG_TRACE_CLASS_STECI
// #undef CONFIG_TRACE_CLASS_BSP
// #undef CONFIG_TRACE_CLASS_IPFORWARD
// #undef CONFIG_ENABLE_CLASS_APP_DABMW
// #undef CONFIG_ENABLE_CLASS_APP_DABMW_SF
// #undef CONFIG_TRACE_CLASS_EXTERNAL
// #undef CONFIG_TRACE_DEFAULT_LEVEL
/*
 * Other Trace configuration
 */
#undef CONFIG_TRACE_INCLUDE_FILTERS

/*
 * Build Environment
 */
#define CONFIG_BUILD_SILENT 1
#undef CONFIG_HOST_OS_LINUX
#undef CONFIG_HOST_OS_LINUX_EMBEDDED
#undef CONFIG_HOST_OS_LINUX_DESKTOP
#undef CONFIG_HOST_OS_WIN32
#undef CONFIG_HOST_OS_TKERNEL
#define CONFIG_HOST_OS_FREERTOS
#define CONFIG_COMPILER_GCC_CROSS 1
#undef CONFIG_COMPILER_GCC_NATIVE
#undef CONFIG_COMPILER_GCC_CROSS_ARM
#define CONFIG_COMPILER_GCC_CROSS_THUMB 1

/*
 * Test and Debug Switches
 */
#define CONFIG_DEBUG_OSAL 1
#define CONFIG_DEBUG_SYMBOLS 1
#undef CONFIG_DEBUG_STACK_USAGE
#undef CONFIG_DEBUG_MEMORY_USAGE
#undef CONFIG_DEBUG_INST_FUNC

/*
 * ETAL build-time and run-time checks
 */
#undef CONFIG_DEBUG_ETAL_CHECKS