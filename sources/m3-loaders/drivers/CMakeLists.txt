# Video processing unit driver
add_subdirectory(vpu/source)
# CAN driver
add_subdirectory(can)
# Accelero/gyrometer driver
add_subdirectory(lis3dsh)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lis3dsh)
add_subdirectory(ais3624dq)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ais3624dq)
# others ST specific drivers
add_subdirectory(sta_fwlib/src)
# CCC driver
add_subdirectory(ccc)
# HSM Host driver
add_subdirectory(hsm/src)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hsm/private_cmd/src)
	add_subdirectory(hsm/private_cmd/src)
	set (EHSM_ST_DRIVER_SRC "$<TARGET_OBJECTS:st_hsmdriver>")
endif()
# HSM Key Storage Host driver
add_subdirectory(hsm/ks_proxy/src)
# HSM Test framework
if (EHSM_TEST)
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hsm/test/generic/src)
		add_subdirectory(hsm/test/generic/src)
		set (EHSM_TEST_SRC "$<TARGET_OBJECTS:hsmtest>")
	endif()
	if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hsm/test/internal/src)
		add_subdirectory(hsm/test/internal/src)
		set (EHSM_ST_TEST_SRC "$<TARGET_OBJECTS:st_hsmtest>")
	endif()
endif()

add_library(stdrivers STATIC
	$<TARGET_OBJECTS:candriver>
	$<TARGET_OBJECTS:lis3dshdriver>
	$<TARGET_OBJECTS:sta_fwlib>
	$<TARGET_OBJECTS:cccdriver>
	$<TARGET_OBJECTS:vpudriver>
	$<TARGET_OBJECTS:ais3624dqdriver>
	$<TARGET_OBJECTS:hsmdriver>
	$<TARGET_OBJECTS:hsm_ks_driver>
	${EHSM_TEST_SRC}
	${EHSM_ST_TEST_SRC}
	${EHSM_ST_DRIVER_SRC}
)

install(TARGETS stdrivers DESTINATION lib${LIB_SUFFIX})

add_library(stdrivers_noscheduler_verbose STATIC
	$<TARGET_OBJECTS:candriver>
	$<TARGET_OBJECTS:lis3dshdriver>
	$<TARGET_OBJECTS:sta_fwlib_noscheduler_verbose>
	$<TARGET_OBJECTS:cccdriver_noscheduler>
	$<TARGET_OBJECTS:vpudriver>
	$<TARGET_OBJECTS:ais3624dqdriver>
	$<TARGET_OBJECTS:hsmdriver>
	$<TARGET_OBJECTS:hsm_ks_driver>
	${EHSM_TEST_SRC}
	${EHSM_ST_TEST_SRC}
	${EHSM_ST_DRIVER_SRC}
)

install(TARGETS stdrivers_noscheduler_verbose DESTINATION lib${LIB_SUFFIX})

add_library(stdrivers_noscheduler STATIC
	$<TARGET_OBJECTS:candriver>
	$<TARGET_OBJECTS:lis3dshdriver>
	$<TARGET_OBJECTS:sta_fwlib_noscheduler>
	$<TARGET_OBJECTS:cccdriver_noscheduler>
	$<TARGET_OBJECTS:vpudriver>
	$<TARGET_OBJECTS:hsmdriver>
	${EHSM_ST_DRIVER_SRC}
)

install(TARGETS stdrivers_noscheduler DESTINATION lib${LIB_SUFFIX})

