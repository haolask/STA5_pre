# Disable code coverage for this subdirectory
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

include_directories(${PROJECT_SOURCE_DIR}/drivers/can)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../inc)
include_directories(${PROJECT_SOURCE_DIR}/drivers/vpu/source/inc)
if (EARLY_TUNER_FTR)
include_directories(${PROJECT_SOURCE_DIR}/middleware/st-etal/etalcore/exports)
include_directories(${PROJECT_SOURCE_DIR}/middleware/st-etal/tuner_driver/exports)
endif (EARLY_TUNER_FTR)

if (EHSM_TEST)
set (EHSM_TEST_SRC "sta_hsm_test.c")
endif()

if (EARLY_TUNER_FTR)
set(TUNER_SERVICE_SRC "sta_tuner.c")
endif (EARLY_TUNER_FTR)

if (NOT BOARD_ID MATCHES "TC3")
	set(CAR_RADIO_SERVICES_SRC
		sta_audio.c
		sta_rpmsg_audio.c
		sta_splash_animation.c
		sta_mm.c
		sta_rpmsg_mm.c
		sta_rvc_test.c
		sta_rotary.c
		${TUNER_SERVICE_SRC}
	)
endif ()

set(services_SRCS
	${CAR_RADIO_SERVICES_SRC}
	sta_can_test.c
	sta_i2c_test.c
	sta_mbox_test.c
	sta_rpmsg_test.c
	sta_ccc_test.c
	sta_sqi_nor_test.c
	sta_cli.c
	sta_cli_cmd.c
	sta_ddr_cli_cmd.c
	sta_board_config.c
	${EHSM_TEST_SRC}
	sta_ks.c
	sta_rpmsg_rdm.c
	sta_remote_gpio.c
)

add_library(services STATIC ${services_SRCS})

add_library(services_noscheduler_verbose STATIC ${services_SRCS})

target_compile_definitions(services_noscheduler_verbose PRIVATE
	VERBOSE_DDR
	NO_SCHEDULER
	CLI_CMDS_DDR
	CLI_CMDS_MEM_EXTRA
	CLI_CMDS_CLK
	CLI_CMDS_MISC
)

install(TARGETS services DESTINATION lib${LIB_SUFFIX})
