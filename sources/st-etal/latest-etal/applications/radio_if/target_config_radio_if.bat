@echo off
rem target_config.bat

rem Set WSPATH to work space directory
set WSPATH=%~dp0..\..\..

rem Set BUILDPATH to current directory
set BUILDPATH=%~dp0

rem cd /D %BUILDPATH%
..\..\tools\windows\wingconf\msvc\Release\wingconf.exe .\target_config_radio_if.cml .\target_config_radio_if.def .\target_config_radio_if.mak .\target_config_radio_if.h

rem delete temporary config file
del ..config.tmp
