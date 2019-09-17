@echo off
REM ==========================================================================
REM
REM        FILE: build.bat
REM
REM       USAGE: ./build.bat
REM
REM DESCRIPTION: Build xloaders with cmake invocation for emIDE
REM
REM      AUTHOR: Philippe LANGLAIS
REM ORGANIZATION: ST
REM ===========================================================================

echo First you have to install Perl and Ninja and put them in your PATH

if exist "build" goto havebuild
	mkdir build
:havebuild
cd build
cmake -G"CodeBlocks - Ninja" -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-gcc-arm-none-eabi.cmake ..
type xloaders.cbp | perl -pe "s,CodeBlocks,emIDE," > xloaders.emp

echo Now launch emIDE and load xloaders.emp present in build directory
