cmake_minimum_required(VERSION 2.8.12)
project(xloaders)

# Project options
# FIXME have to force default build
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(build_types "None;Debug;Release;RelWithDebInfo;MinSizeRel;Coverage")
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
	"Choose the type of build, options are: ${build_types}." FORCE
)
# Enable cmake-gui to display a drop down list for CMAKE_BUILD_TYPE
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${build_types}")

set(boot_devtypes "NAND;MMC;SQI")
set(BOOT_DEVTYPE NAND CACHE STRING
	"Choose the boot device types, options are: ${boot_devtypes}."
)
# Enable cmake-gui to display a drop down list for BOOT_DEVTYPE
set_property(CACHE BOOT_DEVTYPE PROPERTY STRINGS "${boot_devtypes}")

set(board_ids "BOARD_A5_EVB;BOARD_A5_VAB;BOARD_TC3_EVB;BOARD_TC3P_CARRIER;BOARD_TC3P_MTP")
set(BOARD_ID BOARD_A5_EVB CACHE STRING
	"Choose the board id: ${board_ids}"
)
# Enable cmake-gui to display a drop down list for BOARD IDs
set_property(CACHE BOARD_ID PROPERTY STRINGS "${board_ids}")

set(soc_ids "SOCID_STA1295;SOCID_STA1195;SOCID_STA1385;SOCID_STA1275")
set(SOC_ID SOCID_STA1295 CACHE STRING
	"Choose the STA1XXX SoC id: ${soc_ids}"
)
# Enable cmake-gui to display a drop down list for SOC IDs
set_property(CACHE SOC_ID PROPERTY STRINGS "${soc_ids}")

set(ddr_cfg_alt "CFG_DDR_ALT0;CFG_DDR_ALT1;CFG_DDR_ALT2;CFG_DDR_ALT3")
set(DDR_CFG CFG_DDR_ALT0 CACHE STRING
	"Define DDR Alternate Config ${ddr_cfg_alt}"
)
# Enable cmake-gui to display a drop down list for DDRCFG
set_property(CACHE DDR_CFG PROPERTY STRINGS "${ddr_cfg_alt}")

set(LPDDR2 0	 CACHE BOOL
	"LPDDR2 config selection (disabled by default)."
)

# Early Tuner Feature activation.
set(EARLY_TUNER_FTR 0 CACHE BOOL
	"Early Audio Tuner feature (disabled by default)."
)

set(ATF 1 CACHE BOOL
	"ARM Trusted Firmware boot support (enabled by default)."
)

set(COT 1 CACHE BOOL
	"Chain Of Trust support (disabled by default)."
)

set(BOOT_M3OS_FROM_M3XL 0 CACHE BOOL
	"M3 XL shadows and starts M3 OS first instead of ATF BL1"
)

# eHSM definition
# set EHSM_TEST variable to enable or disable the eHSM test framework
set(EHSM_TEST 0 CACHE BOOL
	"eHSM test framework and key storage (disabled by default)"
)

set(STA_MEM_MAP_DIR ${STA_MEM_MAP_DIR} CACHE PATH
	"Path to external common includes (sta_mem_map & shared_data)."
)

set(CLI_ENABLED 0 CACHE BOOL
	"To enable or not CLI through UART console."
)

if (CLI_ENABLED)
	set(CLI_CMDS_DDR 0 CACHE BOOL
		"To enable DDRAM CLI commands."
	)
	set(CLI_CMDS_EXTRA 0 CACHE BOOL
		"To enable EXTRA CLI commands."
	)
	set(CLI_CMDS_CLK 0 CACHE BOOL
		"To enable Clock CLI commands."
	)
	set(CLI_CMDS_MISC 0 CACHE BOOL
		"To enable MISC CLI commands."
	)
endif (CLI_ENABLED)

set(ks_memtypes "NVM_NONE;NVM_MMC;NVM_SQI")
set(KS_MEMTYPE NVM_NONE CACHE STRING
	"Choose the NVM Key Storage device type: ${ks_memtypes}"
)
# Enable cmake-gui to display a drop down list for NVM Key Storage type
set_property(CACHE KS_MEMTYPE PROPERTY STRINGS "${ks_memtypes}")

# XIP only SQI NOR is supported
set(XIP 0 CACHE BOOL
	"Enable XIP for SQI only"
)

set(CMAKE_C_FLAGS_RELEASE "-O3 -DTRACE_LEVEL=10")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DTRACE_LEVEL=10")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -g -DTRACE_LEVEL=20")
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -Os -g -DTRACE_LEVEL=50")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DTRACE_LEVEL=10")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DTRACE_LEVEL=10")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -g -DTRACE_LEVEL=20")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -Os -g -DTRACE_LEVEL=50")

# Setup flags for Code Coverage build mode
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} --coverage" CACHE STRING
    "Flags used by the C++ compiler for building with code coverage."
    FORCE )
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG} --coverage" CACHE STRING
    "Flags used by the C compiler for building with code coverage."
    FORCE )
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage" CACHE STRING
    "Flags used for linking binaries with code coverage."
    FORCE )
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} --coverage" CACHE STRING
    "Flags used by the shared libraries linker during builds with code coverage."
    FORCE )
mark_as_advanced(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE )

# Set project version using Git tag and hash.
execute_process(
  COMMAND git describe --dirty --tags --always
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  RESULT_VARIABLE GIT_VERSION_FOUND
  ERROR_QUIET
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (GIT_VERSION_FOUND)
  if(NOT VERSION)
    set(VERSION "unknown")
  endif()
else (GIT_VERSION_FOUND)
  set(VERSION ${GIT_VERSION})
endif (GIT_VERSION_FOUND)

string(TIMESTAMP BUILD_TS "%Y/%m/%d %H:%M:%S")

add_definitions(-DVERSION="${VERSION} - ${BUILD_TS}")
add_definitions(-DSOC_ID=${SOC_ID})
add_definitions(-DBOARD_ID=${BOARD_ID})
add_definitions(-DKS_MEMTYPE=${KS_MEMTYPE})

if (SOC_ID STREQUAL SOCID_STA1385)
	# Warn: USE_FLASHCACHE must be not set for TC3P CUT1
	set(PREPROC_FLAGS ${PREPROC_FLAGS} -DUSE_FLASHCACHE)
endif ()

# Console Settings
if (CLI_ENABLED)
	add_definitions(-DCLI_ENABLED)
	if (CLI_CMDS_DDR)
		add_definitions(-DCLI_CMDS_DDR)
	endif (CLI_CMDS_DDR)
	if (CLI_CMDS_EXTRA)
		add_definitions(-DCLI_CMDS_EXTRA)
	endif (CLI_CMDS_EXTRA)
	if (CLI_CMDS_CLK)
		add_definitions(-DCLI_CMDS_CLK)
	endif (CLI_CMDS_CLK)
	if (CLI_CMDS_MISC)
		add_definitions(-DCLI_CMDS_MISC)
	endif (CLI_CMDS_MISC)
endif (CLI_ENABLED)

if (XIP)
	if (BOOT_DEVTYPE STREQUAL NAND OR BOOT_DEVTYPE STREQUAL MMC)
		MESSAGE("${BOOT_DEVTYPE} does not support XIP")
		set(XIP 0)
	else (BOOT_DEVTYPE STREQUAL NAND OR BOOT_DEVTYPE STREQUAL MMC)
		MESSAGE("Enable ${BOOT_DEVTYPE} XIP")
		add_definitions(-DXIP)
		set(PREPROC_FLAGS ${PREPROC_FLAGS} -DXIP)
	endif (BOOT_DEVTYPE STREQUAL NAND OR BOOT_DEVTYPE STREQUAL MMC)
endif (XIP)

if (BOOT_M3OS_FROM_M3XL)
	add_definitions(-DBOOT_M3OS_FROM_M3XL)
endif (BOOT_M3OS_FROM_M3XL)

if (LPDDR2)
	add_definitions(-DLPDDR2)
endif (LPDDR2)
add_definitions(-D${DDR_CFG})

if (ATF)
	add_definitions(-DATF)
	set(PREPROC_FLAGS ${PREPROC_FLAGS} -DATF)
endif (ATF)

if (COT)
	add_definitions(-DCOT)
endif (COT)

if (EARLY_TUNER_FTR)
    add_definitions(-DEARLY_TUNER_FTR)
endif (EARLY_TUNER_FTR)

add_definitions(-DDEBUG_EXCEPTION)

message(STATUS "xloaders version: ${VERSION}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Some compiler options used globally
set(CMAKE_C_FLAGS "-Wall -Werror -Wextra -Wno-strict-prototypes -Wno-missing-field-initializers -Wno-unused-parameter -std=c99 ${CMAKE_C_FLAGS}")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/freeRTOS/Source/include)
include_directories(${PROJECT_SOURCE_DIR}/freeRTOS/Source/portable/GCC/ARM_CM3)
include_directories(${PROJECT_SOURCE_DIR}/drivers/sta_fwlib/inc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/ccc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/hsm/inc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/hsm/private_cmd/inc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/hsm/test/generic/inc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/hsm/test/internal/inc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/hsm/ks_proxy/inc)
include_directories(${PROJECT_SOURCE_DIR}/middleware/i2c_service/inc)
include_directories(${PROJECT_SOURCE_DIR}/middleware/rpmsg/inc)
include_directories(${PROJECT_SOURCE_DIR}/middleware/hmac/inc)
include_directories(${PROJECT_SOURCE_DIR}/middleware/spi_service/inc)
if (EARLY_TUNER_FTR)
include_directories(${PROJECT_SOURCE_DIR}/middleware/st-etal/etalcore/exports)
endif (EARLY_TUNER_FTR)
include_directories(${PROJECT_SOURCE_DIR}/services/inc)
include_directories(${PROJECT_SOURCE_DIR}/platform)
include_directories(${STA_MEM_MAP_DIR})

add_subdirectory(freeRTOS/Source)
add_subdirectory(drivers)
add_subdirectory(middleware)
add_subdirectory(platform)
add_subdirectory(services/src)
add_subdirectory(src)

