#!/bin/bash -
#=============================================================================
#          FILE: build.sh
#
#         USAGE: ./build.sh
#
#   DESCRIPTION: Build xloaders with cmake invocation and GNU make
#
#        AUTHOR: Philippe LANGLAIS
#  ORGANIZATION: ST
#=============================================================================
if [ -z $STA_MEM_MAP_DIR ]
then
	echo "Please, setup STA_MEM_MAP_DIR environment variable and make sure it includes a sta_mem_map.h file."
	echo "As example: export STA_MEM_MAP_DIR="`pwd | sed 's,/sources.*$,,'`/build*/tmp/deploy/images/sta1*/sta_mem_map
	exit 1
fi

set -o nounset     # Treat unset variables as an error

if ! test -d build; then
	mkdir build
fi

if [ ! -d $STA_MEM_MAP_DIR ]
then
	echo "$STA_MEM_MAP_DIR is not a valid directory"
	exit 1
fi

cd build
cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-gcc-arm-none-eabi.cmake -DSTA_MEM_MAP_DIR=$STA_MEM_MAP_DIR ..
make

