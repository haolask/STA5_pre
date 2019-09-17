# Common definitions and macros
add_definitions(-D${BOOT_DEVTYPE} -DSOC_ID=${SOC_ID} -DBOARD_ID=${BOARD_ID})

set(LINKER_FLAGS "-Wl,-n,--gc-sections,--cref,-Map")
# M3XL/M3OS common symbols in libraries we want to be linked with
set(M3XL_FORCED_LINKED_SYMBOLS "-u i2c_init -u LLD_SSP_Init -u pinmux_request -u print_process_registers")
set(M3OS_FORCED_LINKED_SYMBOLS "${M3XL_FORCED_LINKED_SYMBOLS} -u _sbrk_r")

# Create 1 output binary file
macro(st_add_postbuild_command target)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${GNU_ARM_SIZE_TOOL}" $<TARGET_FILE:${target}>
		COMMAND "${GNU_ARM_OBJCOPY}" -O binary $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.bin
		COMMENT "${target}.bin built"
		VERBATIM
	)
endmacro (st_add_postbuild_command)

# Create 2 output binary files for M3 OS
#    Split output binary in 2 files mapped in different memroy area
#    1 file with excluding the given section
#    1 file with only the given section
macro(st_add_postbuild_multi_out_command target section)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND "${GNU_ARM_SIZE_TOOL}" $<TARGET_FILE:${target}>
		COMMAND "${GNU_ARM_OBJCOPY}" -O binary -R ${section} $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.bin
		COMMAND "${GNU_ARM_OBJCOPY}" -O binary -j ${section} $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>_file2.bin
		COMMENT "${target}.bin & ${target}_file2.bin built"
		VERBATIM
	)
endmacro (st_add_postbuild_multi_out_command)

# Pre-process ld.E linker file to create output ld file
macro(preproc_ld input target options)
	add_custom_target(${target}
		COMMAND "${CMAKE_C_COMPILER}" -E -x c ${input} -I ${STA_MEM_MAP_DIR} ${PREPROC_FLAGS} ${options} | grep -v "^#" > ${target}
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src
		SOURCES ${input}
		COMMENT "pre-processing ${target}"
		VERBATIM
	)
endmacro (preproc_ld)

# Common library
set(common_SRCS
	init.c
	printf-stdarg.c
	trace.c
	sta_common.c
	sta_image.c
	syscalls.c
	debug_regs.c
)

add_library(common STATIC ${common_SRCS})

# M3 xloader
set(m3_xloader_SRCS
	startup.s
	freeRTOS_stubs.c
	malloc_stubs.c
	sta_xloader.c
)

# M3 car radio application
set(m3_car_radio_SRCS
	startup.s
	sta_car_radio.c
)

# M3 telematics application
set(m3_telematics_SRCS
	startup.s
	sta_telematics.c
)

if (BOARD_ID MATCHES "TC3")
	set(M3OS_TARGET m3_telematics)
else ()
	set(M3OS_TARGET m3_car_radio)
endif ()

set(XL_LDSCRIPT m3_xloader.ld)
set(OS_LDSCRIPT m3_os.ld)
# Default split section
set(SPLIT_SECTION .text2)

# Target to pass C pre-processor on .ld.E files
preproc_ld(${OS_LDSCRIPT}.E ${OS_LDSCRIPT} "")
preproc_ld(${XL_LDSCRIPT}.E ${XL_LDSCRIPT} "")
preproc_ld(${OS_LDSCRIPT}.E m3_mini.ld "-D MINI_OS")
preproc_ld(m3_xl_uflashloader.ld.E m3_xl_uflashloader.ld "")

# eHSM configuration
if (EHSM_TEST)
	add_definitions(-DEHSM_TEST)
	preproc_ld(${OS_LDSCRIPT}.E m3_os_ehsm_test.ld -DEHSM_TEST)
	set(OS_LDSCRIPT m3_os_ehsm_test.ld)
	set(SPLIT_SECTION .rodata)
endif (EHSM_TEST)

set_property(SOURCE startup.s PROPERTY LANGUAGE C)

add_executable(m3_xloader ${m3_xloader_SRCS})
add_dependencies(m3_xloader ${XL_LDSCRIPT})
target_link_libraries(m3_xloader
	"-T${CMAKE_CURRENT_SOURCE_DIR}/${XL_LDSCRIPT} ${M3XL_FORCED_LINKED_SYMBOLS} ${LINKER_FLAGS},m3_xloader.map"
	common
	platform
	stdrivers_noscheduler
	middleware
)

add_executable(${M3OS_TARGET} ${${M3OS_TARGET}_SRCS})
add_dependencies(${M3OS_TARGET} ${OS_LDSCRIPT})
target_link_libraries(${M3OS_TARGET}
	"-T${CMAKE_CURRENT_SOURCE_DIR}/${OS_LDSCRIPT} ${M3OS_FORCED_LINKED_SYMBOLS} ${LINKER_FLAGS},${M3OS_TARGET}.map"
	common
	services
	platform
	stdrivers
	middleware
	freeRTOS
)
# Add post-build commands to create output binary files
st_add_postbuild_multi_out_command(${M3OS_TARGET} ${SPLIT_SECTION})

#target_compile_definitions(m3_xloader PRIVATE
#	DEBUG;OTHER_OPTIONS
#)

# Add post-build commands to create output binary files
st_add_postbuild_command(m3_xloader)

# M3 xloader for u-boot base flashloader tool
set(m3_xl_uflashloader_SRCS
	startup.s
	freeRTOS_stubs.c
	sta_xl_uflashloader.c
)

add_executable(m3_xl_uflashloader ${m3_xl_uflashloader_SRCS})
add_dependencies(m3_xl_uflashloader m3_xl_uflashloader.ld)
target_link_libraries(m3_xl_uflashloader
	"-T${CMAKE_CURRENT_SOURCE_DIR}/m3_xl_uflashloader.ld ${LINKER_FLAGS} ${M3XL_FORCED_LINKED_SYMBOLS} -Wl,m3_xl_uflashloader.map"
	common
	platform
	stdrivers_noscheduler
	middleware
)
target_include_directories(m3_xl_uflashloader
	PRIVATE ${PROJECT_SOURCE_DIR}/middleware/liblzg/include
)
target_compile_definitions(m3_xl_uflashloader PRIVATE)
st_add_postbuild_command(m3_xl_uflashloader)

# M3 minimal executable
set(m3_mini_SRCS
	startup.s
	sta_m3_mini.c
)

add_executable(m3_mini ${m3_mini_SRCS})
add_dependencies(m3_mini m3_mini.ld)
target_link_libraries(m3_mini
	"-T${CMAKE_CURRENT_SOURCE_DIR}/m3_mini.ld ${LINKER_FLAGS} ${M3XL_FORCED_LINKED_SYMBOLS} -Wl,m3_mini.map"
	common
	services_noscheduler_verbose
	platform
	stdrivers_noscheduler_verbose
	middleware
	freeRTOS
)

target_compile_definitions(m3_mini PRIVATE)
st_add_postbuild_command(m3_mini)

