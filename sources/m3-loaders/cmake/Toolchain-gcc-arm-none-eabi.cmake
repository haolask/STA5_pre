# CMake Toolchain file for the gcc-arm-none-eabi toolchain.

include(CMakeForceCompiler)

# Targeting an embedded system, no OS.
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m3)

set(CROSS_COMPILE arm-none-eabi- CACHE STRING "Set your CROSS_COMPILE attribute")
mark_as_advanced(CROSS_COMPILE)

CMAKE_FORCE_C_COMPILER(${CROSS_COMPILE}gcc GNU)
CMAKE_FORCE_CXX_COMPILER(${CROSS_COMPILE}g++ GNU)
set(GNU_ARM_OBJCOPY ${CROSS_COMPILE}objcopy)
set(GNU_ARM_SIZE_TOOL ${CROSS_COMPILE}size)

# Find the target environment prefix..
# First see where gcc is keeping libc.a
execute_process(
COMMAND ${CMAKE_C_COMPILER} -print-file-name=libc.a
OUTPUT_VARIABLE CMAKE_INSTALL_PREFIX
OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Strip the filename off
get_filename_component(CMAKE_INSTALL_PREFIX
	"${CMAKE_INSTALL_PREFIX}" PATH
)
# Then find the canonical path to the directory one up from there
get_filename_component(CMAKE_INSTALL_PREFIX
	"${CMAKE_INSTALL_PREFIX}/.." REALPATH
)
set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE FILEPATH "Install path prefix, prepended onto install directories.")
message(STATUS "Cross compiling with ${CROSS_COMPILE}xxx tools ...")
message(STATUS "Toolchain prefix: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS}"
  "-fno-common -ffunction-sections -fdata-sections"
)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-r4")

  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS}"
    "-mcpu=cortex-r4 -march=armv7-r"
    "-msoft-float"
  )

elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-m3")

  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS}"
    "-mcpu=cortex-m3 -march=armv7-m -mthumb"
    "-msoft-float"
  )

elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-a7")

  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS}"
    "-mcpu=cortex-a7 -march=armv7a-neon"
	"-mfloat-abi=hard -mfpu=vfp-neon" # FIXME
  )

else ()
  message(WARNING
    "Processor not recognised in toolchain file, "
    "compiler flags not configured."
  )
endif ()

# When we break up long strings in CMake we get semicolon
# separated lists, undo this here...
string(REGEX REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "")

set(BUILD_SHARED_LIBS OFF)

