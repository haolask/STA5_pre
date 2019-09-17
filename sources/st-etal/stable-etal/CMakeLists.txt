#### ADD-ON for TKERNEL & ebinder case
#### this is controlled by USER_EB_HOME : to identify we are in ebinder
####
#ifndef USER_EB_HOME
# Do not execute make from the directory containing this Makefile!
# Instead, go to one of the target/ subdirectories and execute from there
#ifndef ODIR
# $(error Execute make from 'target/linux' or 'target/win32')
#endif
#
#include $(CURDIR)/target_config.mak
#else
#ebinder compilation 
#include $(USER_HOME)/make/macros.mk
#ROOTDIR=$(PARTS_HOME)/$(OBJ_HOME)
# in pclint we compile all files
# so all includes should be in
# simulate DCOP/DCOP_MDR/DCOP_HDADIO and DRIVER_DIRECT
#CONFIG_ETAL_SUPPORT_DCOP=y
#CONFIG_ETAL_SUPPORT_DCOP_MDR=y
#CONFIG_ETAL_SUPPORT_DCOP_HDRADIO=y
#CONFIG_COMM_DRIVER_DIRECT=y
#endif


# /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\
#
# This section must be generated from target_config.h inside /target/freeRTOS/ using the script toto.sh
#
# /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\ /!\


set(AUTOCONF_INCLUDED)
unset(CONFIG_BOARD_CMOST_MAIN)
unset(CONFIG_BOARD_ACCORDO2)
set(CONFIG_BOARD_ACCORDO5 1)
set(CONFIG_BOARD_ACCORDO2_SPI0_IS_32766 1)
unset(CONFIG_BOARD_ACCORDO2_SPI0_IS_0)
set(CONFIG_MODULE_INTEGRATED 1)
set(CONFIG_MODULE_INTEGRATED_WITH_2_TDA7707 1)
unset(CONFIG_MODULE_INTEGRATED_WITH_TDA7707_TDA7708)
set(CONFIG_DIGITAL_AUDIO 1)
unset(CONFIG_MODULE_INDEPENDENT)
unset(CONFIG_ETAL_ENABLE_CMOST_SDM_CLOCK)
set(CONFIG_BOARD_ACCORDO5_M3 1)
set(CONFIG_ETAL_SUPPORT_CMOST 1)
set(CONFIG_ETAL_SUPPORT_CMOST_STAR 1)
unset(CONFIG_ETAL_SUPPORT_CMOST_DOT)
set(CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL 1)
unset(CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
unset(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BC)
unset(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BF)
unset(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_BG)
unset(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_CA)
set(CONFIG_ETAL_SUPPORT_CMOST_STAR_T_CUT_DA 1)
unset(CONFIG_ETAL_SUPPORT_DCOP)
unset(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
unset(CONFIG_ETAL_SUPPORT_DCOP_MDR)
set(CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS 1)
set(CONFIG_ETAL_HAVE_ALL_API 1)
unset(CONFIG_ETAL_HAVE_SELECTED_API)
unset(CONFIG_ETAL_HAVE_ETALTML)
unset(CONFIG_ETAL_RECEIVER_ALIVE_PERIODIC_CHECK)
set(CONFIG_ETAL_RECEIVER_CHECK_STATE_PERIOD 333)
unset(CONFIG_ETAL_INIT_CHECK_SILICON_VERSION)
unset(CONFIG_ETALTML_HAVE_RDS)
unset(CONFIG_ETALTML_HAVE_AMFMLANDSCAPE)
unset(CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL)
unset(CONFIG_ETALTML_HAVE_RADIOTEXT)
unset(CONFIG_ETALTML_HAVE_AUTO_SEEK)
unset(CONFIG_ETALTML_AUTO_SEEK_INTERNAL)
unset(CONFIG_ETALTML_AUTO_SEEK_EXTERNAL)
unset(CONFIG_ETALTML_HAVE_SCAN)
unset(CONFIG_ETALTML_HAVE_LEARN)
unset(CONFIG_ETALTML_HAVE_SEAMLESS)
unset(CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
unset(CONFIG_ETALTML_HAVE_RDS_STRATEGY)
unset(CONFIG_COMM_DRIVER_EXTERNAL)
set(CONFIG_COMM_DRIVER_EMBEDDED 1)
set(CONFIG_COMM_CMOST_I2C 1)
unset(CONFIG_COMM_CMOST_SPI)
unset(CONFIG_COMM_CMOST_HAVE_DEDICATED_RESET_LINE)
unset(CONFIG_COMM_ENABLE_RDS_IRQ)
unset(CONFIG_COMM_CMOST_FIRMWARE_FILE)
set(CONFIG_COMM_CMOST_FIRMWARE_IMAGE 1)
unset(CONFIG_COMM_CMOST_FIRMWARE_EMBEDDED)
unset(CONFIG_COMM_CMOST_CUSTOMER_SETTINGS)
set(CONFIG_COMM_DCOP_MDR_FIRMWARE_NO_DOWNLOAD 1)
unset(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE)
set(CONFIG_COMM_DCOP_MDR_ENABLE_SPI_BOOT_GPIO 1)
unset(CONFIG_COMM_DRIVER_DIRECT)
set(CONFIG_APP_ETAL_LIBRARY 1)
unset(CONFIG_APP_ETALCORE_LIBRARY)
unset(CONFIG_APP_TUNERDRIVER_LIBRARY)
unset(CONFIG_APP_ETAL_TEST)
unset(CONFIG_APP_OSALCORE_TESTS)
unset(CONFIG_APP_ETAL_DCOP_MDR_FLASH)
set(CONFIG_TRACE_ENABLE 1)
unset(CONFIG_TRACE_ASYNC)
unset(CONFIG_TRACE_ETAL_SYSLOG)
set(CONFIG_TRACE_CLASS_OSALCORE 2)
set(CONFIG_TRACE_CLASS_ETAL 3)
set(CONFIG_TRACE_CLASS_BOOT 2)
set(CONFIG_TRACE_CLASS_CMOST 2)
set(CONFIG_TRACE_CLASS_TUNERDRIVER 2)
set(CONFIG_TRACE_CLASS_HDRADIO 2)
set(CONFIG_TRACE_CLASS_STECI 2)
set(CONFIG_TRACE_CLASS_BSP 2)
set(CONFIG_TRACE_CLASS_IPFORWARD 2)
set(CONFIG_ENABLE_CLASS_APP_DABMW 2)
unset(CONFIG_ENABLE_CLASS_APP_DABMW_SF)
set(CONFIG_TRACE_CLASS_EXTERNAL 2)
set(CONFIG_TRACE_DEFAULT_LEVEL 3)
unset(CONFIG_TRACE_INCLUDE_FILTERS)
set(CONFIG_BUILD_SILENT 1)
unset(CONFIG_HOST_OS_LINUX)
unset(CONFIG_HOST_OS_LINUX_EMBEDDED)
unset(CONFIG_HOST_OS_LINUX_DESKTOP)
unset(CONFIG_HOST_OS_WIN32)
unset(CONFIG_HOST_OS_TKERNEL)
set(CONFIG_HOST_OS_FREERTOS)
set(CONFIG_COMPILER_GCC_CROSS 1)
unset(CONFIG_COMPILER_GCC_NATIVE)
unset(CONFIG_COMPILER_GCC_CROSS_ARM)
set(CONFIG_COMPILER_GCC_CROSS_THUMB 1)
set(CONFIG_DEBUG_OSAL 1)
set(CONFIG_DEBUG_SYMBOLS 1)
unset(CONFIG_DEBUG_STACK_USAGE)
unset(CONFIG_DEBUG_MEMORY_USAGE)
unset(CONFIG_DEBUG_INST_FUNC)
unset(CONFIG_DEBUG_ETAL_CHECKS)

#####################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/target/freeRTOS)

#########
# TOOLCHAIN
#########

# Parallel builds
#
# Support for parallel builds of target testbuild' is provided through
# the bash environment variable MAKEFLAGS. Child make invocations
# take from this variable the make invocation options so 
# it is possible to pass use it to specify the number of concurrent
# make invocations with -j.
#
# Customize the argument to -j below to <number of CPUs> + 1
# used in your build environment.
# Each Intel Hyperthreaded core counts as 2 CPUs, thus
# the default MAKEFLAGS below is good for a single core with
# Hyperthreading, but should not cause too much performance
# degradation in single core (no-hyperthreaded) environment.
#
# MAKEFLAGS is defined only for 'testbuild' target; to obtain
# parallel make execution for other targets it is necessary to
# invoke make with -j, e.g.:
#   make -j 3 all
# If you use it this way, do not specify both 'clean' and 'all'
# targets on the same make command line, because make will parallelize
# them and may remove the output of the build in progress, resulting
# in build errors; use instead e.g.:
#   make clean && make -j 3 
#
# NOTE: do not specify -j on the make command line for target
# 'testbuild' because this will confuse make and result in
# non-parallel execution. Parallel execution for 'testbuild'
# requires only the definition of MAKEFLAGS.
#
# IMPORTANT: comment out if building for MinGW because may result
# in the build process hanging randomly
#MAKEFLAGS = -j 3

#if(CONFIG_BUILD_SILENT)
 # The @ prefix instructs make to not echo the command being executed
# SILENT  = @
 # The next variable suppresses all 'ar' output; in particular, 'ar'
 # prints to standard error instead of standard output the string
 # "creating .../etal.a":
 # the '2>&1' is bash magic to redirect stdio and stderr 
# SILENT_AR = > /dev/null 2>&1
 # $(notdir NAME) removes the pathname part from NAME
 # $< expands to the name of the first prerequisite
 # in practice these lines print 'Compiling etalapi.c...' instead of the whole
 # gcc command line
 # Alternatively just use the part under 'else'
#if(CONFIG_BOARD_ACCORDO5)
# CC  ?= @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
# CXX ?= @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
#else(CONFIG_BOARD_ACCORDO5)
# CC  = @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
# CXX = @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
#endif(CONFIG_BOARD_ACCORDO5)
#else(CONFIG_BUILD_SILENT)
# SILENT  = 
# SILENT_AR = 
#if(CONFIG_BOARD_ACCORDO5)
# CC      ?= $(PREFIX)gcc
# CXX     ?= $(PREFIX)gcc
#else(CONFIG_BOARD_ACCORDO5)
# CC      = $(PREFIX)gcc
# CXX     = $(PREFIX)gcc
#endif(CONFIG_BOARD_ACCORDO5)
#endif(CONFIG_BUILD_SILENT)

#if(CONFIG_BOARD_ACCORDO5)
#LD       ?= $(SILENT) $(PREFIX)gcc
#AR       ?= $(SILENT) $(PREFIX)ar
#else(CONFIG_BOARD_ACCORDO5)
#LD       = $(SILENT) $(PREFIX)gcc
#AR       = $(SILENT) $(PREFIX)ar
#endif(CONFIG_BOARD_ACCORDO5)
#SPLINT   = $(SILENT) $(ROOTDIR)/splinter.py splint # splinter is a splint output colorizer: http://os.inf.tu-dresden.de/~doebel/projects/splinter/
#SPLINT   = $(SILENT) splint
#CTAGS    = $(SILENT) ctags
#DOXYGEN  = doxygen
#ECHO     = @ echo
#GCOV     = gcov

#if(CONFIG_COMPILER_GCC_NATIVE)
# PREFIX =
#endif(CONFIG_COMPILER_GCC_NATIVE)
#if(CONFIG_COMPILER_GCC_CROSS)
# PREFIX ?= arm-v7-linux-uclibceabi-
#endif(CONFIG_COMPILER_GCC_CROSS)

# All of the following utilities are used just to create/update
# the ETAL version file

# 'svnversion' is a utility normally included with the command line SVN client
# It scans through the SVN working directory and outputs the highest
# version number. The number may be followed by an 'M' to indicate that
# the local copy is modified. For other letters' meaning check svnversion --help
# Note that the build will fail if the version number contains a letter.
# This is intentional to ensure the version number is consistent with the repo
#SVNVERSION = $(SILENT) svnversion

# 'cat' prints the file content to stdout
#CAT      = $(SILENT) cat

#########
# FILES
#########

#DOXYFILE   = $(PATH_GLOBAL_CONFIG)/Doxyfile
#DOXYOUT    = $(ODIR)/doxy.log
#GCOVOUT    = $(PATH_GCOV_OUT)/gcov.out
#CONFIGFILE = $(PATH_CONFIG)/target_config.h
#LIBEXT   = .a
#OBJ_DIR  = $(ODIR)
#if(CONFIG_HOST_OS_WIN32)
# EXEEXT  = .exe
#endif(CONFIG_HOST_OS_WIN32)
#if(CONFIG_HOST_OS_LINUX)
# EXEEXT  =
#endif(CONFIG_HOST_OS_LINUX)
#ETAL_VERSION_FILE = etalversion.h

#########
# FLAGS
#########

#######################################################################
# comment all FLAGS section and use Cmake defult option for the moment
#######################################################################

#
# GCC flags subsection
#

# GCC_WEXTRA_FLAGS
#
# Instead of using -Wextra we list explicitly the flags normally set by that flag
# so we can selectively turn off the ones we don't like.
# The removed ones are listed here:
#  -Wunused-parameter

#GCC_WEXTRA_FLAGS = \
#	-Wclobbered \
#	-Wempty-body \
#	-Wignored-qualifiers \
#	-Wmissing-field-initializers \
#	-Wmissing-parameter-type \
#	-Wold-style-declaration \
#	-Woverride-init \
#	-Wsign-compare \
#	-Wtype-limits \
#	-Wuninitialized \
#	-Wunused-but-set-parameter

# GCC_MORE_WARNING_FLAGS
#
# Other warning flags we want to use but are not included neither in -Wall neither in GCC_WEXTRA_FLAGS
# The following may be enabled in future:
# -Werror
#
# Note: GCC version 4.6.3 (and lower?) does not support -Wmaybe-uninitialized

#GCC_MORE_WARNING_FLAGS = \
#	-Wmissing-prototypes \
#	-Wstrict-prototypes \
#	-Wmaybe-uninitialized \
#	-Werror=cpp \
#	-Wformat=0 \
#	-Werror=format-extra-args \
#	-Werror=format \
#	-Werror=switch \
#	-Werror=strict-aliasing \
#	-Werror=unused-variable \
#	-Werror=implicit-function-declaration \
#	-Wpointer-arith


# GCC flags:
# -x c: forces compilation as C sources also for lib_steci's *.cpp
#       files which are actually C sources named cpp for compatibility with
#       MDR_CONTROL_APPLICATION. Without this option there would be linker errors.
#CFLAGS   = -Wall -fno-common -ffunction-sections -fdata-sections -Os $(GCC_WEXTRA_FLAGS) $(GCC_MORE_WARNING_FLAGS) -x c $(INCLUDES) $(DEFINES)
#CXXFLAGS = -Wall -fno-common -ffunction-sections -fdata-sections -Os $(GCC_WEXTRA_FLAGS) $(GCC_MORE_WARNING_FLAGS) -x c $(INCLUDES) $(DEFINES)
#LDFLAGS  = -Xlinker -Map=$(ODIR)/output.map

# coverage
#ifdef CONFIG_DEBUG_COVERAGE
# CFLAGS   += -fprofile-arcs -ftest-coverage
# CXXFLAGS += -fprofile-arcs -ftest-coverage
# LDFLAGS  += -lgcov --coverage
# GCOV_FLAGS = -b
#endif

# the option -fstack-usage creates a *.su file per source compiled
# containing the summary of the stack used by each function in the file
# (man gcc for details)
#ifdef CONFIG_DEBUG_STACK_USAGE
# CFLAGS   += -fstack-usage
# CXXFLAGS += -fstack-usage
#endif

#ifdef CONFIG_DEBUG_SYMBOLS
# CFLAGS   += -g
# CXXFLAGS += -g
# LDFLAGS  += -g
#endif

#ifdef CONFIG_DEBUG_INST_FUNC
# CFLAGS   += -finstrument-functions -finstrument-functions-exclude-file-list=inst_func_trace.c -finstrument-functions-exclude-function-#list=clock_gettime
# CXXFLAGS += -finstrument-functions -finstrument-functions-exclude-file-list=inst_func_trace.c -finstrument-functions-exclude-function-#list=clock_gettime
#endif

# -mthumb-interwork
#  Generate code that supports calling between the ARM and Thumb instruction sets.  Without this option, on
#  pre-v5 architectures, the two instruction sets cannot be reliably used inside one program.  The default
#  is -mno-thumb-interwork, since slightly larger code is generated when -mthumb-interwork is specified.
#  In AAPCS configurations this option is meaningless.
#ifdef CONFIG_COMPILER_GCC_CROSS_THUMB
# CFLAGS   += -mthumb
# CXXFLAGS += -mthumb
#endif

#
# SPLINT flags subsection
#

# SPLINT_FLAGS_WEAK
#
# Flags for Splint's -weak mode of operation
#
# -preproc: workaround splint problem with "cannot parse __VA_ARGS__ in TracePrintf"
# -fileextensions: avoid warnings due to .cpp files (which are really .c)
# -emptyret: avoid warnings about 'return' with no value specified
# +posixlib: tell Splint we're using Posix libraries
# -unrecog: due to the above many standard identifiers are not pulled in and cause false warnings, disable
# +skip-sys-headers: no errors are reported for files in system directories (otherwise splint exits due to a bug)
#
# Splint does not parse header files and when it finds types it does not know it
# exits with parseerror. This happens on many osalcore files, so here we
# workaround the problem by forcing the type definition.
#
# The following are workarounds for splint parse error on osalcore/osproc.c
# -Dpthread_t=int
# -Dpthread_mutex_t=int
# -Dpthread_attr_t=int
# The following are workarounds for splint parse error on osalcore/osevent.c (TML only)
# -Dpthread_cond_t=int
# The following are workarounds for splint parse error on osalcore/ossmphr.c
# -Dsem_t=int
# The following are workarounds for splint parse error on osalcore/ossmphr.c
# -Derrno=int

#SPLINT_FLAGS_WEAK = \
#	-preproc \
#	-fileextensions \
#	-emptyret \
#	+posixlib \
#	-unrecog \
#	+skip-sys-headers \
#	-Dpthread_t=int \
#	-Dpthread_mutex_t=int \
#	-Dpthread_attr_t=int \
#	-Dpthread_cond_t=int \
#	-Dsem_t=int

#SPLINT_FLAGS_WEAK_ALL = \
#	-weak \
#	$(SPLINT_FLAGS_WEAK)

# SPLINT_FLAGS_STANDARD
#
# Flags for Splint's -standard mode of operation
#
# -booltype: set name of boolean type
# -paramuse: avoid "A function parameter is not used in the body of the function" warnings (ENABLED)
# +charindex: allow char types to index array and avoid "Array fetch using non-integer" warning
# -noret: avoid "Path with no return in function declared to return tVoid"
#         the warning would be meaningful for non-tVoid returns but those are already catched by the compiler
#         This looks like a bug in splint that does not recognize typedef tVoid as void
# -usedef: avoid warning about use before initialization; it is not reliable without annotating the code with
#          information on the direction of function parameters (splint sees as not initialized all
#          parameters passed by reference and gives hundreds of false warnings).
# -nullpass: avoid "Null storage passed as non-null param" since we'd need to annotate the sources to make this
#            warning reliable
#
# The folowing flags suppress warnings that need to be reviewed:
#
# -mustfreefresh: avoid "Storage allocated locally is not released before the last reference to it is lost" (seem fake warnings)
# -boolcompare: allow "Use of == with tBool variables"
# -compdef: avoid "Passed storage XXX not completely defined"; this is normally a false warning due to the parameter being
#           passed as a reference to be initialized by the called function
# -uniondef: same as compdef, for unions
# -observertrans: unclear
# -statictrans: related to the previous one
# -nullret
# -compmempass
# -compdestroy
# -onlytrans
# -nullstate
# -immediatetrans

#SPLINT_FLAGS_STANDARD = \
#	-booltype tBool \
#	+charindex \
#	-noret \
#	-usedef \
#	-nullpass \
#	-mustfreefresh \
#	-boolcompare \
#	-compdef \
#	-uniondef \
#	-observertrans \
#	-statictrans \
#	-nullret \
#	-compmempass \
#	-compdestroy \
#	-onlytrans \
#	-nullstate \
#	-immediatetrans

#SPLINT_FLAGS_STANDARD_ALL = \
#	-standard \
#	$(SPLINT_FLAGS_WEAK) \
#	$(SPLINT_FLAGS_STANDARD)

# SPLINT_FLAGS_OPTIONAL
#
# Optional flags for Splint
#
# +show-scan: Show file names as they are processed
#
# Add the following ones if not specifying +skip-sys-headers:
# -Dasm=__asm__ 
# -D__signed__=signed 
# -D__attribute=__attribute__

#SPLINT_FLAGS_OPTIONAL =

# SPLINT_FLAGS
#
# Final selection:
# select either SPLINT_FLAGS_STANDARD_ALL or SPLINT_FLAGS_WEAK_ALL
# in addition to SPLINT_FLAGS_OPTIONAL
#
# SPLINT_FLAGS_STANDARD_ALL is not reccomended due to the large number
# of false positives

#SPLINT_FLAGS = \
#	$(SPLINT_FLAGS_OPTIONAL) \
#	$(SPLINT_FLAGS_STANDARD_ALL)

#ebinder compilation 
#ifdef  USER_EB_HOME
#specific for PC-LINT
#OBJ_HOME = tunerifCMOS/ETAL_trunk
#LIB = etal
#endif

#
# CTAGS flags subsection
#

# CTAGS_FLAGS
#
# -R: recurse into directories (DO NOT REMOVE THIS ONE)
# --exclude=t_kernel*: many functions doubly defined in linux and t-kernel, avoid duplicate entries in TAGS file
# --verbose=yes: [optional, for debug] print list of files being considered
#CTAGS_FLAGS = \
#	-R                  \
#	--exclude=t_kernel* \
#	--exclude=*.o       \
#	--exclude=*.a 

# DOXYGEN flags
# No flags are specified here, refer to DOXYFILE

# various utility flags
# ECHO_NNFLAG = don't issue a newline after the echo
#ECHO_NNFLAG = \
#	-n

#########
# PATHS
#########

set(PATH_ST_ETAL ${CMAKE_CURRENT_SOURCE_DIR})

set(PATH_ETALCORE ${CMAKE_CURRENT_SOURCE_DIR}/etalcore)
set(PATH_ETALTML ${CMAKE_CURRENT_SOURCE_DIR}/etaltml)
set(PATH_ETALCOMMON ${CMAKE_CURRENT_SOURCE_DIR}/etalcommon)
set(PATH_TUNERDRIVER ${CMAKE_CURRENT_SOURCE_DIR}/tuner_driver)
set(PATH_DCOPDRIVER ${CMAKE_CURRENT_SOURCE_DIR}/dcop_driver)
set(PATH_DCOPDRIVER_EXPORTS ${PATH_DCOPDRIVER}/exports)
set(PATH_TOOLS ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(PATH_ETALCORE_EXPORTS ${PATH_ETALCORE}/exports)
set(PATH_TUNERDRIVER_EXPORTS ${PATH_TUNERDRIVER}/exports)
#PATH_TESTBUILD           = $(CURDIR)/testbuild
set(PATH_GLOBAL_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/.)
set(PATH_CONFIG  ${CMAKE_CURRENT_SOURCE_DIR}/.)
#PATH_GCOV_OUT            = $(CURDIR)/gcov
set(PATH_APPLICATIONS ${CMAKE_CURRENT_SOURCE_DIR}/applications)

# when inserting new directories in the project
# update this variables section
# etalcore
set(PATH_LIBETALCORE_SOURCES ${PATH_ETALCORE}/lib_etalcore/src)
set(PATH_LIBETALCORE_INCLUDES ${PATH_ETALCORE}/lib_etalcore/include)
set(PATH_LIBETAL_PARAMS ${PATH_ETALCORE}/exports/etalparam)
set(PATH_ETALTEST_SOURCES ${PATH_ETALCORE}/lib_etalcore/test)
set(PATH_ETALTEST_INCLUDES ${PATH_ETALCORE}/lib_etalcore/test)
set(PATH_LIBIPF_SOURCES ${PATH_ETALCORE}/lib_ipforward/src)
set(PATH_LIBIPF_INCLUDES ${PATH_ETALCORE}/lib_ipforward/include)
if(CONFIG_APP_ETAL_DCOP_MDR_FLASH)
set(PATH_APPLICATIONS_SOURCES ${PATH_APPLICATIONS_SOURCES} ${PATH_APPLICATIONS}/etalDcopMdrFlash)
endif(CONFIG_APP_ETAL_DCOP_MDR_FLASH)
if(CONFIG_COMM_DRIVER_DIRECT)
set(PATH_APPLICATIONS_SOURCES ${PATH_APPLICATIONS_SOURCES} ${PATH_APPLICATIONS}/radio_if)
else(CONFIG_COMM_DRIVER_DIRECT)
if(CONFIG_APP_RADIO_IF)
set(PATH_APPLICATIONS_SOURCES ${PATH_APPLICATIONS_SOURCES} ${PATH_APPLICATIONS}/radio_if)
endif(CONFIG_APP_RADIO_IF)
endif(CONFIG_COMM_DRIVER_DIRECT)
# etaltml
set(PATH_LIBRDS_SOURCES ${PATH_ETALTML}/lib_rds/src)
set(PATH_LIBRDS_INCLUDES ${PATH_ETALTML}/lib_rds/include)
set(PATH_LIBETALTML_SOURCES ${PATH_ETALTML}/lib_etaltml/src)
set(PATH_LIBETALTML_INCLUDES ${PATH_ETALTML}/lib_etaltml/include)
set(PATH_LIBSF_SOURCES ${PATH_ETALTML}/lib_servicefollowing/src)
set(PATH_LIBSF_INCLUDES ${PATH_ETALTML}/lib_servicefollowing/include)
# etalcommon
# etalcommon
if(CONFIG_BOARD_ACCORDO5_M3)
#FREERTOS
set(PATH_LIBOSALCORE_SOURCES ${PATH_ETALCOMMON}/lib_osal/src/osalcore/freeRTOS)
set(PATH_LIBBSP_SOURCES ${PATH_ETALCOMMON}/lib_bsp/src/freeRTOS_sta1X95)
elseif(USER_EB_HOME)
#TKERNEL
set(PATH_LIBOSALCORE_SOURCES ${PATH_ETALCOMMON}/lib_osal/src/osalcore/t_kernel)
set(PATH_LIBBSP_SOURCES ${PATH_ETALCOMMON}/lib_bsp/src/t_kernel_sta1095)
else(CONFIG_BOARD_ACCORDO5_M3)
#LINUX
set(PATH_LIBOSALCORE_SOURCES ${PATH_ETALCOMMON}/lib_osal/src/osalcore/linux)
set(PATH_LIBBSP_SOURCES ${PATH_ETALCOMMON}/lib_bsp/src/linux_sta1X95)
endif(CONFIG_BOARD_ACCORDO5_M3)


#**************** To complet with new osal path  and bsp for freeRTOS

set(PATH_LIBOSALCORE_INCLUDES ${PATH_ETALCOMMON}/lib_osal/include/osalcore)
if(USER_EB_HOME)
set(PATH_LIBOSALCORE_INCLUDES_TK ${PATH_LIBOSALCORE_INCLUDES}/t_kernel)
endif(USER_EB_HOME)
set(PATH_OSALTEST_SOURCES ${PATH_ETALCOMMON}/lib_osal/test)
set(PATH_OSALTEST_INCLUDES ${PATH_ETALCOMMON}/lib_osal/test)
set(PATH_LIBBSP_INCLUDES ${PATH_ETALCOMMON}/lib_bsp/include)
set(PATH_LIBCOMMON_SOURCES ${PATH_ETALCOMMON}/lib_common/src)
set(PATH_LIBCOMMON_INCLUDES ${PATH_ETALCOMMON}/lib_common/include)
# tuner_driver
set(PATH_LIBCMOST_SOURCES ${PATH_TUNERDRIVER}/lib_cmost/src)
set(PATH_LIBCMOST_INCLUDES ${PATH_TUNERDRIVER}/lib_cmost/include)
set(PATH_LIBBOOT_SOURCES ${PATH_TUNERDRIVER}/lib_boot/src)
set(PATH_LIBBOOT_INCLUDES ${PATH_TUNERDRIVER}/lib_boot/include)
set(PATH_LIBTUNERDRV_SOURCES ${PATH_TUNERDRIVER}/lib_tunerdriver/src)
set(PATH_LIBTUNERDRV_INCLUDES ${PATH_TUNERDRIVER}/lib_tunerdriver/include)
set(PATH_TUNERDRIVER_FIRMWARE ${PATH_TUNERDRIVER_EXPORTS}/firmware)
set(PATH_TUNERDRIVER_PARAM ${PATH_TUNERDRIVER_EXPORTS}/param)
# dcop_driver
set(PATH_LIBBOOT_DCOP_SOURCES ${PATH_DCOPDRIVER}/lib_boot/src)
set(PATH_LIBBOOT_DCOP_INCLUDES ${PATH_DCOPDRIVER}/lib_boot/include)
set(PATH_LIBSTECI_SOURCES ${PATH_DCOPDRIVER}/lib_steci/src)
set(PATH_LIBSTECI_INCLUDES ${PATH_DCOPDRIVER}/lib_steci/include)
set(PATH_LIBHDRADIO_SOURCES ${PATH_DCOPDRIVER}/lib_hdradio/src)
set(PATH_LIBHDRADIO_INCLUDES ${PATH_DCOPDRIVER}/lib_hdradio/include)
# tools
set(PATH_TOOLSINSTFUNC_SOURCES ${PATH_TOOLS}/inst_func/src)
# config validate
# since this is in the root of the project we explicitly list the sources here
# we want the .o located in the target/linux/o, see below CONFIG_VALIDATE_OBJECTS
set(CONFIG_VALIDATE_SOURCES ${PATH_GLOBAL_CONFIG}/target_config_validate.c)

if(CONFIG_COMM_DRIVER_DIRECT)
set(PATH_RADIO_IF_INCLUDES ${ROOTDIR}/applications/radio_if)
else(CONFIG_COMM_DRIVER_DIRECT)
if(CONFIG_APP_RADIO_IF)
set(PATH_RADIO_IF_INCLUDES ${ROOTDIR}/applications/radio_if)
endif(CONFIG_APP_RADIO_IF)
endif(CONFIG_COMM_DRIVER_DIRECT)

# etalcore
#LIBETALCORE_SOURCES:= $(wildcard $(PATH_LIBETALCORE_SOURCES)/*.c)
#LIBETALCORE_OBJECTS:= $(patsubst %.c,%.o,$(LIBETALCORE_SOURCES))
set(libetalcore_SRCS
        ${PATH_LIBETALCORE_SOURCES}/etalcallback.c
        ${PATH_LIBETALCORE_SOURCES}/etalintcb.c
        ${PATH_LIBETALCORE_SOURCES}/etalcmd.c
        ${PATH_LIBETALCORE_SOURCES}/etaltuner.c
        ${PATH_LIBETALCORE_SOURCES}/etalruntimecheck.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_quality.c
        ${PATH_LIBETALCORE_SOURCES}/etalconfig_accordo2.c
        ${PATH_LIBETALCORE_SOURCES}/etalcomm_cmost.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_tune.c
        ${PATH_LIBETALCORE_SOURCES}/etalcmd_cmost.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_rds.c
        ${PATH_LIBETALCORE_SOURCES}/etalbootcmd_hdr.c
        ${PATH_LIBETALCORE_SOURCES}/etaldatahandler.c
        ${PATH_LIBETALCORE_SOURCES}/etalboot_hdr.c
        ${PATH_LIBETALCORE_SOURCES}/etalcmd_hdradio.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_debug.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_read_write_parameter.c
        ${PATH_LIBETALCORE_SOURCES}/etaltrace.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_alternate_frequency.c
        ${PATH_LIBETALCORE_SOURCES}/etalcomm_hdradio.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_systemdata.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_dataserv.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_audio.c
        ${PATH_LIBETALCORE_SOURCES}/etalcontrol.c
        ${PATH_LIBETALCORE_SOURCES}/etalconfig.c
        ${PATH_LIBETALCORE_SOURCES}/etalbootcmd_mdr.c
        ${PATH_LIBETALCORE_SOURCES}/etal_index_parameter_STA710.c
        ${PATH_LIBETALCORE_SOURCES}/etalstatus.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_advtune.c
        ${PATH_LIBETALCORE_SOURCES}/etal_index_parameter_TDA7707.c
        ${PATH_LIBETALCORE_SOURCES}/etalutil.c
        ${PATH_LIBETALCORE_SOURCES}/etalboot_mdr.c
        ${PATH_LIBETALCORE_SOURCES}/etalinit.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi_config.c
        ${PATH_LIBETALCORE_SOURCES}/etal_index_parameter_TDA7708.c
        ${PATH_LIBETALCORE_SOURCES}/etal_index_parameter_STA709.c
        ${PATH_LIBETALCORE_SOURCES}/etalreceiver.c
        ${PATH_LIBETALCORE_SOURCES}/etalapi.c
        ${PATH_LIBETALCORE_SOURCES}/etalseekutil.c
        ${PATH_LIBETALCORE_SOURCES}/etalconfig_cmostm.c
        ${PATH_LIBETALCORE_SOURCES}/etalcomm_mdr.c
        ${PATH_LIBETALCORE_SOURCES}/etalcmd_mdr.c
)


#LIBIPF_SOURCES     := $(wildcard $(PATH_LIBIPF_SOURCES)/*.c)
#LIBIPF_OBJECTS     := $(patsubst %.c,%.o,$(LIBIPF_SOURCES))
set(libipf_SRCS
        ${PATH_LIBIPF_SOURCES}/TcpIpProtocol.c
        ${PATH_LIBIPF_SOURCES}/InterLayerProtocol.c
        ${PATH_LIBIPF_SOURCES}/ipforward.c
        ${PATH_LIBIPF_SOURCES}/Utility.c
)


# etaltml
#LIBRDS_SOURCES     := $(wildcard $(PATH_LIBRDS_SOURCES)/*.c)
#LIBRDS_OBJECTS     := $(patsubst %.c,%.o,$(LIBRDS_SOURCES))
#LIBETALTML_SOURCES := $(wildcard $(PATH_LIBETALTML_SOURCES)/*.c)
#LIBETALTML_OBJECTS := $(patsubst %.c,%.o,$(LIBETALTML_SOURCES))
#LIBSF_SOURCES      := $(wildcard $(PATH_LIBSF_SOURCES)/*.c)
#LIBSF_OBJECTS      := $(patsubst %.c,%.o,$(LIBSF_SOURCES))


# etalcommon
#LIBOSALCORE_SOURCES:= $(wildcard $(PATH_LIBOSALCORE_SOURCES)/*.c)
#LIBOSALCORE_OBJECTS:= $(patsubst %.c,%.o,$(LIBOSALCORE_SOURCES))
#LIBOSAL_SOURCES     = $(LIBOSALCORE_SOURCES)
#LIBOSAL_OBJECTS     = $(LIBOSALCORE_OBJECTS)
set(libosal_SRCS
        ${PATH_LIBOSALCORE_SOURCES}/osmemory.c
        ${PATH_LIBOSALCORE_SOURCES}/osutilio.c
        ${PATH_LIBOSALCORE_SOURCES}/osproc.c
        ${PATH_LIBOSALCORE_SOURCES}/oserror.c
        ${PATH_LIBOSALCORE_SOURCES}/ostime.c
        ${PATH_LIBOSALCORE_SOURCES}/osinit.c
        ${PATH_LIBOSALCORE_SOURCES}/ossmphr.c
        ${PATH_LIBOSALCORE_SOURCES}/osevent.c
)


#LIBBSP_SOURCES     := $(wildcard $(PATH_LIBBSP_SOURCES)/*.c)
#LIBBSP_OBJECTS     := $(patsubst %.c,%.o,$(LIBBSP_SOURCES))
set(libbsp_SRCS
        ${PATH_LIBBSP_SOURCES}/bsp_freeRTOS.c
        ${PATH_LIBBSP_SOURCES}/bsp_sta1095evb.c
)


#LIBCOMMON_SOURCES  := $(wildcard $(PATH_LIBCOMMON_SOURCES)/*.c)
#LIBCOMMON_OBJECTS  := $(patsubst %.c,%.o,$(LIBCOMMON_SOURCES))
set(libcommon_SRCS
        ${PATH_LIBCOMMON_SOURCES}/common_fifo.c
        ${PATH_LIBCOMMON_SOURCES}/common_trace.c
)



#OSALTEST_SOURCES   := $(wildcard $(PATH_OSALTEST_SOURCES)/*.c)
#OSALTEST_OBJECTS   := $(patsubst %.c,%.o,$(OSALTEST_SOURCES))
set(osaltest_SRCS
        ${PATH_OSALTEST_SOURCES}/osaltest.c
        ${PATH_OSALTEST_SOURCES}/osaltest_main.c
        ${PATH_OSALTEST_SOURCES}/osaltest_util.c
        ${PATH_OSALTEST_SOURCES}/Test_EV_LV1.c
        ${PATH_OSALTEST_SOURCES}/Test_PR_LV1.c
        ${PATH_OSALTEST_SOURCES}/Test_SM_LV1.c
        ${PATH_OSALTEST_SOURCES}/Test_SM_LV2_TC1.c
)

#ETALTEST_SOURCES   := $(wildcard $(PATH_ETALTEST_SOURCES)/*.c)
#ETALTEST_OBJECTS   := $(patsubst %.c,%.o,$(ETALTEST_SOURCES))
#APPLICATIONS_SOURCESC   := $(wildcard $(PATH_APPLICATIONS_SOURCES)/*.c)
#APPLICATIONS_SOURCESCPP := $(wildcard $(PATH_APPLICATIONS_SOURCES)/*.cpp)
#APPLICATIONS_OBJECTS := $(patsubst %.c,%.o,$(APPLICATIONS_SOURCESC)) $(patsubst %.cpp,%.o,$(APPLICATIONS_SOURCESCPP))


#TOOLSINSTFUNC_SOURCES := $(wildcard $(PATH_TOOLSINSTFUNC_SOURCES)/*.c)
#TOOLSINSTFUNC_OBJECTS := $(patsubst %.c,%.o,$(TOOLSINSTFUNC_SOURCES))
set(toolsinsfunc_SRCS
        ${PATH_TOOLSINSTFUNC_SOURCES}/inst_func_trace.c
)


# tuner_driver

#LIBTUNERDRV_SOURCES:= $(wildcard $(PATH_LIBTUNERDRV_SOURCES)/*.c)
#LIBTUNERDRV_OBJECTS:= $(patsubst %.c,%.o,$(LIBTUNERDRV_SOURCES))
set(libtunerdriver_SRCS
        ${PATH_LIBTUNERDRV_SOURCES}/tunerdriver.c
)


#LIBCMOST_SOURCESC  := $(wildcard $(PATH_LIBCMOST_SOURCES)/*.c)
#LIBCMOST_SOURCESCPP:= $(wildcard $(PATH_LIBCMOST_SOURCES)/*.cpp)
#LIBCMOST_SOURCES   := $(LIBCMOST_SOURCES) $(LIBCMOST_SOURCES)
#LIBCMOST_OBJECTS   := $(patsubst %.c,%.o,$(LIBCMOST_SOURCESC))  $(patsubst %.cpp,%.o,$(LIBCMOST_SOURCESCPP))
set(libcmost_SRCS
        ${PATH_LIBCMOST_SOURCES}/cmost_crc.cpp
        ${PATH_LIBCMOST_SOURCES}/cmost_dump.c
        ${PATH_LIBCMOST_SOURCES}/cmost_helpers.cpp
        ${PATH_LIBCMOST_SOURCES}/cmost_lld.cpp
        ${PATH_LIBCMOST_SOURCES}/cmost_protocol.cpp
)

#Modify CPP file properties to compile in C
set_property(SOURCE ${libcmost_SRCS} PROPERTY LANGUAGE C)
set_property(SOURCE ${libcmost_SRCS} PROPERTY COMPILE_FLAGS -Wno-error)

#LIBBOOT_SOURCES    := $(wildcard $(PATH_LIBBOOT_SOURCES)/*.c)
#LIBBOOT_OBJECTS    := $(patsubst %.c,%.o,$(LIBBOOT_SOURCES))
set(libboot_SRCS
        ${PATH_LIBBOOT_SOURCES}/boot_cmost.c
)


# dcop_driver

#LIBSTECI_SOURCESC  := $(wildcard $(PATH_LIBSTECI_SOURCES)/*.c)
#LIBSTECI_SOURCESCPP:= $(wildcard $(PATH_LIBSTECI_SOURCES)/*.cpp)
#LIBSTECI_SOURCES   := $(LIBSTECI_SOURCESC) $(LIBSTECI_SOURCESCPP)
#LIBSTECI_OBJECTS   := $(patsubst %.c,%.o,$(LIBSTECI_SOURCESC)) $(patsubst %.cpp,%.o,$(LIBSTECI_SOURCESCPP))
set(libsteci_SRCS
        ${PATH_LIBSTECI_SOURCES}/DAB_Protocol.c
        ${PATH_LIBSTECI_SOURCES}/dcop_boot.cpp
        ${PATH_LIBSTECI_SOURCES}/dcop_lld.cpp
        ${PATH_LIBSTECI_SOURCES}/steci_crc.cpp
        ${PATH_LIBSTECI_SOURCES}/steci_fifo.c
        ${PATH_LIBSTECI_SOURCES}/steci_init.c
        ${PATH_LIBSTECI_SOURCES}/steci_lld.cpp
        ${PATH_LIBSTECI_SOURCES}/steci_protocol.cpp
        ${PATH_LIBSTECI_SOURCES}/utility.cpp
)

#LIBHDRADIO_SOURCESC:= $(wildcard $(PATH_LIBHDRADIO_SOURCES)/*.c)
#LIBHDRADIO_SOURCESCPP:= $(wildcard $(PATH_LIBHDRADIO_SOURCES)/*.cpp)
#LIBHDRADIO_OBJECTS := $(patsubst %.c,%.o,$(LIBHDRADIO_SOURCESC)) $(patsubst %.cpp,%.o,$(LIBHDRADIO_SOURCESCPP))
set(libhdradio_SRCS
        ${PATH_LIBHDRADIO_SOURCES}/HDRADIO_Protocol.c
        ${PATH_LIBHDRADIO_SOURCES}/hdr_boot.cpp
)

#LIBBOOT_DCOP_SOURCESC:= $(wildcard $(PATH_LIBBOOT_DCOP_SOURCES)/*.c)
#LIBBOOT_DCOP_SOURCESCPP:= $(wildcard $(PATH_LIBBOOT_DCOP_SOURCES)/*.cpp)
#LIBBOOT_DCOP_OBJECTS := $(patsubst %.c,%.o,$(LIBBOOT_DCOP_SOURCESC)) $(patsubst %.cpp,%.o,$(LIBBOOT_DCOP_SOURCESCPP))
set(libboot_dcop_SRCS
        ${PATH_LIBBOOT_DCOP_SOURCES}/hdr_boot.cpp
)


# config validate
# We don't want to put the object in the root
#CONFIG_VALIDATE_OBJECTS := $(OBJ_DIR)/target_config_validate.o

# etalcore.a library from tuner_driver, etalcore and etalcommon sources
#LIBRARY_ETALCORE_SOURCES = \
#			$(LIBETALCORE_SOURCES) \
#			$(LIBIPF_SOURCES)
#LIBRARY_ETALCORE_OBJECTS = \
#			$(LIBETALCORE_OBJECTS) \
#			$(LIBIPF_OBJECTS)
#LIBRARY_ETALCORE_INCLUDES = \
#			-I$(PATH_LIBETALCORE_INCLUDES) \
#			-I$(PATH_ETALCORE_EXPORTS) \
#			-I$(PATH_LIBETAL_PARAMS) \
#			-I$(PATH_TUNERDRIVER_FIRMWARE) \
#			-I$(PATH_TUNERDRIVER_PARAM) \
#			-I$(PATH_LIBIPF_INCLUDES)	
#LIBRARY_ETALCORE_SOURCES_PATH = \
#			$(PATH_LIBETALCORE_SOURCES) \
#			$(PATH_ETALCORE_EXPORTS) \
#			$(PATH_LIBETAL_PARAMS) \
#			$(PATH_LIBIPF_SOURCES)
#LIBRARY_ETALCORE_INCLUDES_PATH = \
#			$(PATH_LIBETALCORE_INCLUDES) \
#			$(PATH_ETALCORE_EXPORTS) \
#			$(PATH_LIBETAL_PARAMS) \
#			$(PATH_TUNERDRIVER_FIRMWARE) \
#			$(PATH_TUNERDRIVER_PARAM) \
#			$(PATH_LIBIPF_INCLUDES)
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO 
#LIBRARY_ETALCORE_INCLUDES += -I$(PATH_DCOPDRIVER_EXPORTS)
#LIBRARY_ETALCORE_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS)
#endif	
			
#ifdef CONFIG_COMM_DRIVER_DIRECT
#LIBRARY_ETALCORE_INCLUDES += \
#			-I$(PATH_RADIO_IF_INCLUDES)
#LIBRARY_ETALCORE_INCLUDES_PATH += \
#			$(PATH_RADIO_IF_INCLUDES)
#else
#ifdef CONFIG_APP_RADIO_IF
#LIBRARY_ETALCORE_INCLUDES += \
#			-I$(PATH_RADIO_IF_INCLUDES)
#LIBRARY_ETALCORE_INCLUDES_PATH += \
#			$(PATH_RADIO_IF_INCLUDES)
#endif
#endif
set(library_etalcore_SRCS
        ${libetalcore_SRCS}
#        ${libipf_SRCS}
)
	
include_directories(${PATH_LIBETALCORE_INCLUDES})
include_directories(${PATH_ETALCORE_EXPORTS})
include_directories(${PATH_LIBETAL_PARAMS})
include_directories(${PATH_TUNERDRIVER_FIRMWARE})
include_directories(${PATH_TUNERDRIVER_PARAM})
include_directories(${PATH_LIBIPF_INCLUDES})

#target_include_directories(library_etalcore
#	PRIVATE ${PATH_LIBETALCORE_INCLUDES}
#		${PATH_ETALCORE_EXPORTS}
#		${PATH_LIBETAL_PARAMS}
#		${PATH_TUNERDRIVER_FIRMWARE}
#		${PATH_TUNERDRIVER_PARAM}
#		${PATH_LIBIPF_INCLUDES}
#)

# etaltml library from tml sources, not exported
#LIBRARY_ETALTML_SOURCES = \
#			$(LIBETALTML_SOURCES) \
#			$(LIBRDS_SOURCES) \
#			$(LIBSF_SOURCES)
#LIBRARY_ETALTML_OBJECTS = \
#			$(LIBETALTML_OBJECTS) \
#			$(LIBRDS_OBJECTS) \
#			$(LIBSF_OBJECTS)
#LIBRARY_ETALTML_INCLUDES = \
#			-I$(PATH_LIBETALTML_INCLUDES) \
#			-I$(PATH_LIBRDS_INCLUDES) \
#			-I$(PATH_LIBSF_INCLUDES)
#LIBRARY_ETALTML_SOURCES_PATH = \
#			$(PATH_LIBETALTML_SOURCES) \
#			$(PATH_LIBRDS_SOURCES) \
#			$(PATH_LIBSF_SOURCES)
#LIBRARY_ETALTML_INCLUDES_PATH = \
#			$(PATH_LIBETALTML_INCLUDES) \
#			$(PATH_LIBRDS_INCLUDES) \
#			$(PATH_LIBSF_INCLUDES)
			
			
# common library, not exported
#LIBRARY_ETALCOMMON_SOURCES = \
#			$(LIBBSP_SOURCES) \
#			$(LIBCOMMON_SOURCES) \
#			$(LIBOSAL_SOURCES)
#LIBRARY_ETALCOMMON_OBJECTS = \
#			$(LIBBSP_OBJECTS) \
#			$(LIBCOMMON_OBJECTS) \
#			$(LIBOSAL_OBJECTS)
#
#LIBRARY_ETALCOMMON_INCLUDES = \
#			-I$(PATH_LIBBSP_INCLUDES) \
#			-I$(PATH_LIBCOMMON_INCLUDES) \
#			-I$(PATH_LIBOSALCORE_INCLUDES)
#			
#ifdef USER_EB_HOME
#LIBRARY_ETALCOMMON_INCLUDES += -I$(PATH_LIBOSALCORE_INCLUDES_TK)
#endif
#
#LIBRARY_ETALCOMMON_SOURCES_PATH = \
#			$(PATH_LIBBSP_SOURCES) \
#			$(PATH_LIBCOMMON_SOURCES) \
#			$(PATH_LIBOSALCORE_SOURCES)
#LIBRARY_ETALCOMMON_INCLUDES_PATH = \
#			$(PATH_LIBBSP_INCLUDES) \
#			$(PATH_LIBCOMMON_INCLUDES) \
#			$(PATH_LIBOSALCORE_INCLUDES)
#ifdef USER_EB_HOME
#LIBRARY_ETALCOMMON_INCLUDES_PATH += $(PATH_LIBOSALCORE_INCLUDES_TK)
#endif
set(library_etalcommon_SRCS
        ${libbsp_SRCS}
        ${libcommon_SRCS}
        ${libosal_SRCS}
)
include_directories(${PATH_LIBBSP_INCLUDES})
include_directories(${PATH_LIBCOMMON_INCLUDES})
include_directories(${PATH_LIBOSALCORE_INCLUDES})

# tuner_driver.a library from tuner_driver and etalcommon sources
#LIBRARY_TUNERDRIVER_SOURCES = \
#			$(LIBTUNERDRV_SOURCES) \
#			$(LIBCMOST_SOURCES) \
#			$(LIBBOOT_SOURCES)
#
#LIBRARY_TUNERDRIVER_OBJECTS = \
#			$(LIBTUNERDRV_OBJECTS) \
#			$(LIBCMOST_OBJECTS) \
#			$(LIBBOOT_OBJECTS)
#
#LIBRARY_TUNERDRIVER_INCLUDES = \
#			-I$(PATH_TUNERDRIVER_EXPORTS) \
#			-I$(PATH_TUNERDRIVER_FIRMWARE) \
#			-I$(PATH_TUNERDRIVER_PARAM) \
#			-I$(PATH_LIBTUNERDRV_INCLUDES) \
#			-I$(PATH_LIBCMOST_INCLUDES) \
#			-I$(PATH_LIBBOOT_INCLUDES)
#
#LIBRARY_TUNERDRIVER_SOURCES_PATH = \
#			$(PATH_LIBTUNERDRV_SOURCES) \
#			$(PATH_LIBCMOST_SOURCES) \
#			$(PATH_LIBBOOT_SOURCES)
#
#LIBRARY_TUNERDRIVER_INCLUDES_PATH = \
#			$(PATH_TUNERDRIVER_EXPORTS) \
#			$(PATH_TUNERDRIVER_FIRMWARE) \
#			$(PATH_TUNERDRIVER_PARAM) \
#			$(PATH_LIBTUNERDRV_INCLUDES) \
#			$(PATH_LIBCMOST_INCLUDES) \
#			$(PATH_LIBBOOT_INCLUDES)
#
set(library_tunerdriver_SRCS
        ${libtunerdriver_SRCS}
        ${libcmost_SRCS}
        ${libboot_SRCS}
)
include_directories(${PATH_TUNERDRIVER_EXPORTS})
include_directories(${PATH_TUNERDRIVER_FIRMWARE})
include_directories(${PATH_TUNERDRIVER_PARAM})
include_directories(${PATH_LIBTUNERDRV_INCLUDES})
include_directories(${PATH_LIBCMOST_INCLUDES})
include_directories(${PATH_LIBBOOT_INCLUDES})

if(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#LIBRARY_TUNERDRIVER_SOURCES += $(LIBHDRADIO_SOURCES)
#LIBRARY_TUNERDRIVER_OBJECTS += $(LIBHDRADIO_OBJECTS)
#LIBRARY_TUNERDRIVER_INCLUDES += -I$(PATH_LIBHDRADIO_INCLUDES) \
#			-I$(PATH_DCOPDRIVER_EXPORTS)
#LIBRARY_TUNERDRIVER_SOURCES_PATH += $(PATH_LIBHDRADIO_SOURCES)
#LIBRARY_TUNERDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
#			$(PATH_LIBHDRADIO_INCLUDES)
set(library_tunerdriver_SRCS
        ${library_tunerdriver_SRCS}
        ${libhdradio_SRCS}
)
include_directories({PATH_LIBHDRADIO_INCLUDES})
include_directories({PATH_DCOPDRIVER_EXPORTS})

endif(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

if(CONFIG_ETAL_SUPPORT_DCOP_MDR)
#LIBRARY_TUNERDRIVER_SOURCES += $(LIBSTECI_SOURCES)
#LIBRARY_TUNERDRIVER_OBJECTS += $(LIBSTECI_OBJECTS)
#LIBRARY_TUNERDRIVER_INCLUDES += -I$(PATH_LIBSTECI_INCLUDES) \
#			-I$(PATH_DCOPDRIVER_EXPORTS)			
#LIBRARY_TUNERDRIVER_SOURCES_PATH += $(PATH_LIBSTECI_SOURCES)
#LIBRARY_TUNERDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
#			$(PATH_LIBSTECI_INCLUDES)			
set(library_tunerdriver_SRCS
        ${library_tunerdriver_SRCS}
        ${libsteci_SRCS}
)
include_directories({PATH_LIBSTECI_INCLUDES})
include_directories({PATH_DCOPDRIVER_EXPORTS})

endif(CONFIG_ETAL_SUPPORT_DCOP_MDR)

# dcop_driver.a library from dcop_driver and etalcommon sources
#LIBRARY_DCOPDRIVER_SOURCES = 
#LIBRARY_DCOPDRIVER_OBJECTS =
#LIBRARY_DCOPDRIVER_INCLUDES =
#LIBRARY_DCOPDRIVER_SOURCES_PATH =
#LIBRARY_DCOPDRIVER_INCLUDES_PATH =

set(library_dcopdriver_SRCS "")

if(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
#LIBRARY_DCOPDRIVER_SOURCES += $(LIBHDRADIO_SOURCES) \
#			$(LIBBOOT_DCOP_SOURCES)
#LIBRARY_DCOPDRIVER_OBJECTS += $(LIBHDRADIO_OBJECTS) \
#			$(LIBBOOT_DCOP_OBJECTS)
#LIBRARY_DCOPDRIVER_INCLUDES += -I$(PATH_LIBHDRADIO_INCLUDES) \
#			-I$(PATH_LIBBOOT_DCOP_INCLUDES) \
#			-I$(PATH_DCOPDRIVER_EXPORTS)
#LIBRARY_DCOPDRIVER_SOURCES_PATH += $(PATH_LIBHDRADIO_SOURCES) \
#			$(PATH_LIBBOOT_DCOP_SOURCES)
#LIBRARY_DCOPDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
#			$(PATH_LIBHDRADIO_INCLUDES) \
#			$(PATH_LIBBOOT_DCOP_INCLUDES)
set(library_dcopdriver_SRCS 
        ${library_dcopdriver_SRCS}
        ${libhdradio_SRCS}
        ${libboot_dcop_SRCS}        
)
include_directories({PATH_LIBHDRADIO_INCLUDES})
include_directories({PATH_LIBBOOT_DCOP_INCLUDES})
include_directories({PATH_DCOPDRIVER_EXPORTS})

endif(CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)

if(CONFIG_ETAL_SUPPORT_DCOP_MDR)
#LIBRARY_DCOPDRIVER_SOURCES += $(LIBSTECI_SOURCES)
#LIBRARY_DCOPDRIVER_OBJECTS += $(LIBSTECI_OBJECTS)
#LIBRARY_DCOPDRIVER_INCLUDES += -I$(PATH_LIBSTECI_INCLUDES) \
#			-I$(PATH_DCOPDRIVER_EXPORTS)
#LIBRARY_DCOPDRIVER_SOURCES_PATH += $(PATH_LIBSTECI_SOURCES)
#LIBRARY_DCOPDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
#			$(PATH_LIBSTECI_INCLUDES)
set(library_dcopdriver_SRCS 
        ${library_dcopdriver_SRCS}
        ${libsteci_SRCS}
)

include_directories({PATH_LIBSTECI_INCLUDES})
include_directories({PATH_DCOPDRIVER_EXPORTS})

endif(CONFIG_ETAL_SUPPORT_DCOP_MDR)

# define lib
#LIB_ETAL	= $(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT)

# variables section end

#########
# DEFINES
#########
#
#DEFINES   = 
#
# board selection for etalcore/exports/etal_types.h which does not include target_config.h
# (it could, but this way it is clear which defines need to be exported)
if(CONFIG_BOARD_MMB)
 add_definitions(-DCONFIG_BOARD_MMB)
endif(CONFIG_BOARD_MMB)
if(CONFIG_BOARD_CMOST_MAIN)
 add_definitions(-DCONFIG_BOARD_CMOST_MAIN)
endif(CONFIG_BOARD_CMOST_MAIN)
if(CONFIG_BOARD_ACCORDO2)
 add_definitions(-DCONFIG_BOARD_ACCORDO2)
endif(CONFIG_BOARD_ACCORDO2)
if(CONFIG_MODULE_INTEGRATED)
# assuming all MTD modules will have two CMOST dual channel devices
# this impacts on the ETAL_CAPA_MAX_TUNER definition in etal_types.h
 add_definitions(-DCONFIG_MODULE_INTEGRATED)
endif(CONFIG_MODULE_INTEGRATED)

#########
# INCLUDES
#########

#INCLUDES  = -I.
#
#etal etaltest splint : INCLUDES += \
#		$(LIBRARY_ETALCORE_INCLUDES)    \
#		$(LIBRARY_ETALTML_INCLUDES)     \
#		$(LIBRARY_TUNERDRIVER_INCLUDES) \
#		$(LIBRARY_DCOPDRIVER_INCLUDES)  \
#		$(LIBRARY_ETALCOMMON_INCLUDES)
#
#etalcore : INCLUDES += \
#		$(LIBRARY_ETALCORE_INCLUDES)    \
#		$(LIBRARY_TUNERDRIVER_INCLUDES) \
#		$(LIBRARY_DCOPDRIVER_INCLUDES)  \
#		$(LIBRARY_ETALCOMMON_INCLUDES)

#tuner_driver : INCLUDES += \
#		$(LIBRARY_TUNERDRIVER_INCLUDES) \
#		$(LIBRARY_ETALCOMMON_INCLUDES)

#osaltest : INCLUDES += \
#		$(LIBRARY_ETALCOMMON_INCLUDES)

# add the pclint parts
#ifdef  USER_EB_HOME
#specific handling for pclint 
#INCDIRS += \
#	$(PARTS_HOME)/include/tunerif \
#	$(PARTS_HOME)/include \
#	$(LIBRARY_ETALCORE_INCLUDES_PATH)    \
#	$(LIBRARY_ETALTML_INCLUDES_PATH)     \
#	$(LIBRARY_TUNERDRIVER_INCLUDES_PATH) \
#	$(LIBRARY_DCOPDRIVER_INCLUDES_PATH)  \
#	$(LIBRARY_ETALCOMMON_INCLUDES_PATH)	
#endif

# more target specific includes
#etaltest  : INCLUDES  += -I$(PATH_ETALTEST_INCLUDES)
#osaltest  : INCLUDES  += -I$(PATH_OSALTEST_INCLUDES)

#########
# PATHS for CTAGS, DOXYGEN and TAGS
#########
#
# Several applications use the same list of paths
# so we define them here an re-use
#ETAL_LIBRARY_INCLUDES_PATH_LIST = \
#		$(LIBRARY_ETALCORE_INCLUDES_PATH)    \
#		$(LIBRARY_ETALTML_INCLUDES_PATH)     \
#		$(LIBRARY_TUNERDRIVER_INCLUDES_PATH) \
#		$(LIBRARY_DCOPDRIVER_INCLUDES_PATH)  \
#		$(LIBRARY_ETALCOMMON_INCLUDES_PATH)
#
# We rely on the recursive behaviour of both
# DOXYGEN and CTAGS and pass only the top level directories
#ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH = \
#		$(PATH_ETALCORE)    \
#		$(PATH_ETALTML)     \
#		$(PATH_TUNERDRIVER) \
#		$(PATH_DCOPDRIVER)  \
#		$(PATH_ETALCOMMON)
#
#ifdef CONFIG_APP_ETAL_LIBRARY
#DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
#SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
#endif
#ifdef CONFIG_APP_ETAL_TEST
#DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
#SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
#endif
#ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
#DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
#SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
#endif
#ifdef CONFIG_APP_RADIO_IF
#DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
#SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
#endif
#
#ifdef CONFIG_APP_ETALCORE_LIBRARY
# same as CONFIG_APP_ETAL_LIBRARY but without LIBRARY_ETALTML_*
#DOXYGEN_INCLUDES_PATH = \
#		$(LIBRARY_ETALCORE_INCLUDES_PATH)    \
#		$(LIBRARY_TUNERDRIVER_INCLUDES_PATH) \
#		$(LIBRARY_DCOPDRIVER_INCLUDES_PATH)  \
#		$(LIBRARY_ETALCOMMON_INCLUDES_PATH)
#
#SPLINT_SOURCES_PATH = \
#		$(PATH_ETALCORE)    \
#		$(PATH_TUNERDRIVER) \
#		$(PATH_DCOPDRIVER)  \
#		$(PATH_ETALCOMMON)
#endif
#
#ifdef CONFIG_APP_TUNERDRIVER_LIBRARY
#
#DOXYGEN_INCLUDES_PATH = $(LIBRARY_ETALCOMMON_INCLUDES_PATH)
#ifdef CONFIG_ETAL_SUPPORT_CMOST
#DOXYGEN_INCLUDES_PATH += $(LIBRARY_TUNERDRIVER_INCLUDES_PATH)
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP
#DOXYGEN_INCLUDES_PATH += $(LIBRARY_DCOPDRIVER_INCLUDES_PATH)
#endif
#
#SPLINT_SOURCES_PATH = $(PATH_ETALCOMMON)
#ifdef CONFIG_ETAL_SUPPORT_CMOST
#SPLINT_SOURCES_PATH += $(PATH_TUNERDRIVER)
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP
#SPLINT_SOURCES_PATH += $(PATH_DCOPDRIVER)
#endif
#endif
#
#ifdef CONFIG_APP_OSALCORE_TESTS
#DOXYGEN_INCLUDES_PATH = \
#		$(LIBRARY_ETALCOMMON_INCLUDES_PATH)
#
#SPLINT_SOURCES_PATH = \
#		$(PATH_ETALCOMMON)
#endif
#
#TAGS_SOURCES_PATH = $(SPLINT_SOURCES_PATH)
#
#doxy : DOXYGEN_INCLUDES_PATH += $(PATH_CONFIG)

#########
# LIBRARIES
#########
#
#LIBRARIES = -lpthread
#
#ifdef CONFIG_HOST_OS_WIN32
#etal      : LIBRARIES += -lwsock32
#etaltest  : LIBRARIES += -lwsock32
#endif
#ifdef CONFIG_HOST_OS_LINUX
#etal      : LIBRARIES += -lm
#etaltest  : LIBRARIES += -lm
#endif

#########
# TARGETS
#########
#
# select the default target
#ifdef CONFIG_APP_ETAL_LIBRARY
#all : etal
#endif
#ifdef CONFIG_APP_ETALCORE_LIBRARY
#all : etalcore
#endif
#ifdef CONFIG_APP_TUNERDRIVER_LIBRARY
#all : tuner_driver
#endif
#ifdef CONFIG_APP_ETAL_TEST
#all : etaltest
#endif
#ifdef CONFIG_APP_OSALCORE_TESTS
#all : osaltest
#endif
#ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
#all : etaltest
#endif
#ifdef CONFIG_APP_RADIO_IF
#all : etaltest
#endif
#
# If ever a file named 'version' is created in the working
# directory, the rule that creates/updates the version
# file will refuse to do anything; the .PHONY directive
# instructs make to ignore the file and always run the recipe
#.PHONY: version testbuild
#
# aliases for targets
#etal         : $(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT)
#etalcore     : $(PATH_ETALCORE_EXPORTS)/etalcore$(LIBEXT)
#tuner_driver : $(PATH_TUNERDRIVER_EXPORTS)/tuner_driver$(LIBEXT)
#etaltest     : $(OBJ_DIR)/etaltest$(EXEEXT)
#osaltest     : $(OBJ_DIR)/osaltest$(EXEEXT)

#########
# RULES
#########
#
# WARNING: this makefile does not re-make .o for .h changes!
#$(warning This makefile does not support .h changes!)
#
# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
#$(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT) : $(CONFIG_VALIDATE_OBJECTS) \
#		$(LIBRARY_ETALCORE_OBJECTS)   \
#		$(LIBRARY_ETALTML_OBJECTS)    \
#		$(LIBRARY_TUNERDRIVER_OBJECTS) \
#		$(LIBRARY_DCOPDRIVER_OBJECTS) \
#		$(LIBRARY_ETALCOMMON_OBJECTS) \
#		| $(OBJ_DIR)
#	$(ECHO) Building library $@ 
#	$(AR) r $@ $^ $(SILENT_AR)
#
#$(PATH_ETALCORE_EXPORTS)/etalcore$(LIBEXT) : $(CONFIG_VALIDATE_OBJECTS) \
#		$(LIBRARY_ETALCORE_OBJECTS)   \
#		$(LIBRARY_TUNERDRIVER_OBJECTS)\
#		$(LIBRARY_DCOPDRIVER_OBJECTS) \
#		$(LIBRARY_ETALCOMMON_OBJECTS) \
#		| $(OBJ_DIR)
#	$(ECHO) Building library $@ 
#	$(AR) r $@ $^ $(SILENT_AR)

set(etalcore_SRCS
    ${library_etalcore_SRCS}
    ${library_tunerdriver_SRCS}
    ${library_dcopdriver_SRCS}
    ${library_etalcommon_SRCS}
)

add_library(etalcore OBJECT ${etalcore_SRCS})

#$(PATH_TUNERDRIVER_EXPORTS)/tuner_driver$(LIBEXT) : $(CONFIG_VALIDATE_OBJECTS) \
#		$(LIBRARY_TUNERDRIVER_OBJECTS)\
#		$(LIBRARY_ETALCOMMON_OBJECTS) \
#		| $(OBJ_DIR)
#	$(ECHO) Building library $@ 
#	$(AR) r $@ $^ $(SILENT_AR)
#$(OBJ_DIR)/etaltest$(EXEEXT) : $(CONFIG_VALIDATE_OBJECTS) \
#		$(ETALTEST_OBJECTS) \
#		$(APPLICATIONS_OBJECTS) \
#		$(LIB_ETAL) \
#		| $(OBJ_DIR)
#	$(ECHO) Building test application $@ 
#	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group
#ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
#	$(SILENT) mv $(OBJ_DIR)/etaltest$(EXEEXT) $(OBJ_DIR)/etalDcopMdrFlash$(EXEEXT)
#endif
#ifdef CONFIG_APP_RADIO_IF
#	$(SILENT) mv $(OBJ_DIR)/etaltest$(EXEEXT) $(OBJ_DIR)/radio_if$(EXEEXT)
#endif

#$(OBJ_DIR)/osaltest$(EXEEXT) : $(CONFIG_VALIDATE_OBJECTS) \
#		$(OSALTEST_OBJECTS) \
#		$(LIBOSAL_OBJECTS)  \
#		| $(OBJ_DIR)
#	$(ECHO) Building OSAL test application $@ 
#	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

# target: version
# 
#version :
#	$(ECHO) Updating $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(ECHO) $(ECHO_NNFLAG) '#define ETAL_VERSION_SVN    ' >  $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(SVNVERSION) >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(ECHO)                '#define ETAL_VERSION_MAJOR  (tU8)' >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(ECHO)                '#define ETAL_VERSION_MIDDLE (tU8)' >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(ECHO)                '#define ETAL_VERSION_MINOR  (tU8)' >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(ECHO) Current version file content:
#	$(ECHO) ----------------------------
#	$(CAT) $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
#	$(ECHO) ----------------------------
#	$(ECHO) Manually update the version numbers before committing
#	$(ECHO) WARNING: the svn version number must not contain trailing letters otherwise the build will fail

# This rule exists only to print the message and output the
# target_config_validate.o in the $(OBJ_DIR) instead of the project root
#$(CONFIG_VALIDATE_OBJECTS) : $(CONFIG_VALIDATE_SOURCES) $(OBJ_DIR)
#	$(ECHO) Validating build configuration
#	$(CC) -c -o $@ $(INCLUDES) $(CONFIG_VALIDATE_SOURCES)

#$(OBJ_DIR) :
#	$(ECHO) mkdir $@
#	$(SILENT) mkdir $@

# target: clean
#
#clean :
#	$(ECHO) Cleaning up
#	$(SILENT) rm -f \
#		$(CONFIG_VALIDATE_OBJECTS) \
#		$(LIBRARY_ETALCORE_OBJECTS) \
#		$(LIBRARY_ETALTML_OBJECTS) \
#		$(LIBRARY_TUNERDRIVER_OBJECTS) \
#		$(LIBRARY_DCOPDRIVER_OBJECTS) \
#		$(LIBRARY_ETALCOMMON_OBJECTS) \
#		$(OSALTEST_OBJECTS) \
#		$(ETALTEST_OBJECTS) \
#		$(APPLICATIONS_OBJECTS) \
#		$(OBJ_DIR)/* \
#		$(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT) \
#		$(PATH_ETALCORE_EXPORTS)/etalcore$(LIBEXT) \
#		$(PATH_TUNERDRIVER_EXPORTS)/tuner_driver$(LIBEXT) \
#		$(PATH_TOOLSINSTFUNC_SOURCES)/inst_func_trace.o

# target: covclean
# 
#covclean :
#	$(ECHO) Cleaning up coverage info
#	$(SILENT) rm -f \
#		$(patsubst %o,%gcno,$(LIBRARY_ETALCORE_OBJECTS)) \
#		$(patsubst %o,%gcda,$(LIBRARY_ETALCORE_OBJECTS)) \
#		$(patsubst %o,%gcno,$(LIBRARY_ETALTML_OBJECTS)) \
#		$(patsubst %o,%gcda,$(LIBRARY_ETALTML_OBJECTS)) \
#		$(patsubst %o,%gcno,$(LIBRARY_TUNERDRIVER_OBJECTS)) \
#		$(patsubst %o,%gcda,$(LIBRARY_TUNERDRIVER_OBJECTS)) \
#		$(patsubst %o,%gcno,$(LIBRARY_DCOPDRIVER_OBJECTS)) \
#		$(patsubst %o,%gcda,$(LIBRARY_DCOPDRIVER_OBJECTS)) \
#		$(patsubst %o,%gcno,$(LIBRARY_ETALCOMMON_OBJECTS)) \
#		$(patsubst %o,%gcda,$(LIBRARY_ETALCOMMON_OBJECTS)) \
#		$(patsubst %o,%gcno,$(OSALTEST_OBJECTS)) \
#		$(patsubst %o,%gcda,$(OSALTEST_OBJECTS)) \
#		$(patsubst %o,%gcno,$(ETALTEST_OBJECTS)) \
#		$(patsubst %o,%gcda,$(ETALTEST_OBJECTS)) \
#		$(PATH_GCOV_OUT)/*

# target: eclean
#
# clean only ETAL objects; good for ETAL development, may result in incorrect build so use with care
#eclean :
#	$(ECHO) Cleaning up \(PARTIAL\)
#	$(SILENT) rm -f $(LIBETALCORE_OBJECTS) $(ETALTEST_OBJECTS) $(APPLICATIONS_OBJECTS) $(LIB_ETAL) $(OBJ_DIR)/etaltest$(EXEEXT)

# target: splint
#
# The following directories are omitted from the splint
# check because they come from MDR3 which has its own checker
	#$(PATH_LIBRDS_SOURCES)/*.c
	#$(PATH_ETALTEST_SOURCES)/*.c
	#$(PATH_LIBSF_SOURCES)/*.c
#splint : 
#	$(SPLINT) $(SPLINT_FLAGS) $(INCLUDES) \
#		$(PATH_LIBETALCORE_SOURCES)/*.c \
#		$(PATH_LIBIPF_SOURCES)/*.c \
#		$(PATH_LIBETALTML_SOURCES)/*.c \
#		$(PATH_LIBSTECI_SOURCES)/*.c \
#		$(PATH_LIBSTECI_SOURCES)/*.cpp \
#		$(PATH_LIBHDRADIO_SOURCES)/*.c \
#		$(PATH_LIBHDRADIO_SOURCES)/*.cpp \
#		$(PATH_LIBCMOST_SOURCES)/*.c \
#		$(PATH_LIBCMOST_SOURCES)/*.cpp \
#		$(PATH_LIBBOOT_SOURCES)/*.c \
#		$(PATH_LIBTUNERDRV_SOURCES)/*.c \
#		$(PATH_LIBOSALCORE_SOURCES)/*.c \
#		$(PATH_LIBBSP_SOURCES)/*.c \
#		$(PATH_LIBCOMMON_SOURCES)/*.c

#add-on ebinder pclint
#ifdef  USER_EB_HOME
#SRCS +=  \
#	$(PATH_LIBETALCORE_SOURCES)/*.c \
#	$(PATH_LIBETALTML_SOURCES)/*.c \
#	$(PATH_LIBSTECI_SOURCES)/*.c \
#	$(PATH_LIBSTECI_SOURCES)/*.cpp \
#	$(PATH_LIBHDRADIO_SOURCES)/*.c \
#	$(PATH_LIBHDRADIO_SOURCES)/*.cpp \
#	$(PATH_LIBBOOT_DCOP_SOURCES)/*.c \
#	$(PATH_LIBBOOT_DCOP_SOURCES)/*.cpp \
#	$(PATH_LIBCMOST_SOURCES)/*.c \
#	$(PATH_LIBCMOST_SOURCES)/*.cpp \
#	$(PATH_LIBBOOT_SOURCES)/*.c \
#	$(PATH_LIBTUNERDRV_SOURCES)/*.c \
#	$(PATH_LIBOSALCORE_SOURCES)/*.c \
#	$(PATH_LIBBSP_SOURCES)/*.c \
#	$(PATH_LIBCOMMON_SOURCES)/*.c \
#	$(PATH_LIBSF_SOURCES)/*.c
#
#	include $(MAKE_HOME)/rules.mk
#endif

# target: TAGS
#
#TAGS :
#	$(ECHO) Building $@ 
#	$(SILENT) $(CTAGS) $(CTAGS_FLAGS) $(TAGS_SOURCES_PATH) $(CONFIGFILE)
 
# target: testbuild
#
#testbuild :
#	$(SILENT) MAKEFLAGS="$(MAKEFLAGS)" $(PATH_TESTBUILD)/testbuild.sh all

# target: testsplint
#
#testsplint :
#	$(SILENT) $(PATH_TESTBUILD)/testbuild.sh splint

# target: doxy
# 
#doxy :
# Create on the fly the list of files to pass to Doxygen by
# altering several tags in the Doxygen configuration file
# and passing the configuration file through stdin instead of letting Doxygen read it
# from DOXYFILE.
#
# We use '|' as delimiter character in the 'sed' expression to avoid
# conflicts with the '/' contained in the SPLINT_SOURCES_PATH and DOXYGEN_INCLUDES_PATH
# If 'sed' is not available, insert in DOXYFILE the following:
#  INPUT        = "list of files taken from $(SPLINT_SOURCES_PATH)"
#  INCLUDE_PATH = "list of paths taken from $(DOXYGEN_INCLUDES_PATH)"
#  WARN_LOGFILE = "the filename where DOXYGEN will store the build log"
# without double quotes and change all the following line to just:
#  $(DOXYGEN)
#	$(SILENT) cat $(DOXYFILE) | \
#		sed -e"s|\bINPUT\b *=|INPUT = $(SPLINT_SOURCES_PATH)|" \
#			-e"s|\bINCLUDE_PATH\b *=|INCLUDE_PATH = $(DOXYGEN_INCLUDES_PATH)|" \
#			-e"s|\bWARN_LOGFILE\b *=|WARN_LOGFILE = $(DOXYOUT)|" \
#			- | \
#			$(DOXYGEN) -
#		$(ECHO) "Complete, warnings logged to $(DOXYOUT)"

# target: coverage
#
#coverage : | $(PATH_GCOV_OUT)
# 2>&1 redirects both stdout and stderr to the file
#	$(SILENT) $(GCOV) $(GCOV_FLAGS) $(LIBRARY_ETALCORE_SOURCES) $(LIBRARY_ETALTML_SOURCES) $(LIBRARY_ETALCOMMON_SOURCES) $(LIBRARY_TUNERDRIVER_SOURCES) $(LIBRARY_DCOPDRIVER_SOURCES) > $(GCOVOUT) 2>&1
#	$(SILENT) mv *.gcov $(PATH_GCOV_OUT)
#	$(ECHO) "Complete, gcov files moved to $(PATH_GCOV_OUT)"
#	$(ECHO) "Output logged to $(GCOVOUT)"

#$(PATH_GCOV_OUT) :
#	$(ECHO) mkdir $@
#	$(SILENT) mkdir $@

#help:
#	$(ECHO)  'Available targets:'
#	$(ECHO)  '  clean   - Remove most generated files but keep the config'
#	$(ECHO)  '  eclean  - Remove only the files generated from lib_etal (not recommended)'
#	$(ECHO)  '  all     - Build the library or executable selected by the target_config'
#	$(ECHO)  '            (etaltest, etal.a library, tuner_driver.a library or osaltest)'
#	$(ECHO)  '            This is the default target if nothing is specified'
#	$(ECHO)  '  version - Update and overwrite $(ETAL_VERSION_FILE)'
#	$(ECHO)  '            Use only on a svn directry containing no local modifications'
#	$(ECHO)  '            The minor, middle and major number must be edited by hand in the generated file'
#	$(ECHO)  '  TAGS    - Build the tags file for editors'
#	$(ECHO)  '  splint  - Invoke the static checker on a subset of the project files'
#	$(ECHO)  '  doxy    - Generate Doxygen documentation for the library selected by target_config'
#	$(ECHO)  '  coverage- Generate coverage information for all the ETAL sources'
#	$(ECHO)  '  covclean- Remove all the coverage files (these are not cleaned by make clean)'
#	$(ECHO)
#	$(ECHO)  'Special targets:'
#	$(ECHO)  '  testbuild  - Automatically build the project with several reference configurations to ensure there are no build errors'
#	$(ECHO)  '  testsplint - Same as testbuild but builds with splint'

