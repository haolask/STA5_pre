# Disable code coverage for this subdirectory
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/ARM_CM3)

set(freeRTOS_SRCS
	croutine.c
	event_groups.c
	list.c
	queue.c
	stream_buffer.c
	tasks.c
	timers.c
	FreeRTOS_CLI.c
	portable/MemMang/heap_2.c
	portable/GCC/ARM_CM3/port.c
)

add_library(freeRTOS STATIC ${freeRTOS_SRCS})

install(TARGETS freeRTOS DESTINATION lib${LIB_SUFFIX})
