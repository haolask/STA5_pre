#!/bin/bash -
#=============================================================================
#          FILE: build-ninja.sh
#
#         USAGE: ./build-ninja.sh
#
#   DESCRIPTION: Build xloaders with cmake invocation and use Ninja
#
#        AUTHOR: Philippe LANGLAIS
#  ORGANIZATION: ST
#=============================================================================

set -o nounset     # Treat unset variables as an error

if ! test -d build; then
	mkdir build
fi

cd build
cmake -GNinja -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-gcc-arm-none-eabi.cmake ..
ninja

