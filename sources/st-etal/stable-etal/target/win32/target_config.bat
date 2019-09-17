@echo off
rem target_config.bat

rem Set WSPATH to work space directory
set WSPATH=%~dp0..\..\..

rem Set BUILDPATH to current directory
set BUILDPATH=%~dp0

rem cd /D %BUILDPATH%
..\..\tools\windows\wingconf\msvc\Release\wingconf.exe ..\..\target_config.cml target_config.def target_config.mak target_config.h

rem del tmpconfig.*
del ..config.tmp

