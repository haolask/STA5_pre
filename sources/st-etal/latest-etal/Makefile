#### ADD-ON for TKERNEL & ebinder case
#### this is controlled by USER_EB_HOME : to identify we are in ebinder
####
ifndef USER_EB_HOME
# Do not execute make from the directory containing this Makefile!
# Instead, go to one of the target/ subdirectories and execute from there
ifndef ODIR
 $(error Execute make from 'target/linux' or 'target/win32')
endif

include $(CURDIR)/target_config.mak
else
#ebinder compilation 
include $(USER_HOME)/make/macros.mk
ROOTDIR=$(PARTS_HOME)/$(OBJ_HOME)
# in pclint we compile all files
# so all includes should be in
# simulate DCOP/DCOP_MDR/DCOP_HDADIO and DRIVER_DIRECT
CONFIG_ETAL_SUPPORT_DCOP=y
CONFIG_ETAL_SUPPORT_DCOP_MDR=y
CONFIG_ETAL_SUPPORT_DCOP_HDRADIO=y
CONFIG_COMM_DRIVER_DIRECT=y
endif

# begin of section for etalmdr makefile
ifdef CONFIG_TARGET_APP_STREAMDEC__ENABLE
ifndef LDIR
LDIR=${CURDIR}/lib
endif
ifndef LIBDIR
LIBDIR:=${LDIR}
endif
ifndef TARGET_CONFIG_PATH
TARGET_CONFIG_PATH=${CURDIR}
endif
ifndef PATHSEP
PATHSEP='/'
endif
-include ${WSPATH}/lib_scripts/MakeTarget.mk
link:

endif

# end of section for etalmdr makefile

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
ifndef CONFIG_BOARD_ACCORDO5
MAKEFLAGS = -j 3
endif

ifdef CONFIG_BUILD_SILENT
 # The @ prefix instructs make to not echo the command being executed
 SILENT  = @
 # The next variable suppresses all 'ar' output; in particular, 'ar'
 # prints to standard error instead of standard output the string
 # "creating .../etal.a":
 # the '2>&1' is bash magic to redirect stdio and stderr 
 SILENT_AR = > /dev/null 2>&1
 # $(notdir NAME) removes the pathname part from NAME
 # $< expands to the name of the first prerequisite
 # in practice these lines print 'Compiling etalapi.c...' instead of the whole
 # gcc command line
 # Alternatively just use the part under 'else'
ifdef CONFIG_BOARD_ACCORDO5
 CC  ?= @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
 CXX ?= @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
else
 CC  = @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
 CXX = @echo "Compiling $(notdir $<)..."; $(PREFIX)gcc
endif
else
 SILENT  = 
 SILENT_AR = 
ifdef CONFIG_BOARD_ACCORDO5
 CC      ?= $(PREFIX)gcc
 CXX     ?= $(PREFIX)gcc
else
 CC      = $(PREFIX)gcc
 CXX     = $(PREFIX)gcc
endif
endif

ifdef CONFIG_BOARD_ACCORDO5
LD       ?= $(SILENT) $(PREFIX)gcc
AR       ?= $(SILENT) $(PREFIX)ar
else
LD       = $(SILENT) $(PREFIX)gcc
AR       = $(SILENT) $(PREFIX)ar
endif
#SPLINT   = $(SILENT) $(ROOTDIR)/splinter.py splint # splinter is a splint output colorizer: http://os.inf.tu-dresden.de/~doebel/projects/splinter/
SPLINT   = $(SILENT) splint
CTAGS    = $(SILENT) ctags
DOXYGEN  = doxygen
ECHO     = @ echo
GCOV     = gcov

ifdef CONFIG_COMPILER_GCC_NATIVE
 PREFIX =
endif
ifdef CONFIG_COMPILER_GCC_CROSS
 PREFIX ?= arm-v7-linux-uclibceabi-
endif

# All of the following utilities are used just to create/update
# the ETAL version file

# 'svnversion' is a utility normally included with the command line SVN client
# It scans through the SVN working directory and outputs the highest
# version number. The number may be followed by an 'M' to indicate that
# the local copy is modified. For other letters' meaning check svnversion --help
# Note that the build will fail if the version number contains a letter.
# This is intentional to ensure the version number is consistent with the repo
SVNVERSION = $(SILENT) svnversion

# 'cat' prints the file content to stdout
CAT      = $(SILENT) cat

#########
# FILES
#########

DOXYFILE   = $(PATH_GLOBAL_CONFIG)/Doxyfile
DOXYOUT    = $(ODIR)/doxy.log
GCOVOUT    = $(PATH_GCOV_OUT)/gcov.out
CONFIGFILE = $(PATH_CONFIG)/target_config.h
LIBEXT   = .a
OBJ_DIR  = $(ODIR)
ifdef CONFIG_HOST_OS_WIN32
 EXEEXT  = .exe
endif
ifdef CONFIG_HOST_OS_LINUX
 EXEEXT  =
endif
ETAL_VERSION_FILE = etalversion.h

#########
# FLAGS
#########

#
# GCC flags subsection
#

# GCC_WEXTRA_FLAGS
#
# Instead of using -Wextra we list explicitly the flags normally set by that flag
# so we can selectively turn off the ones we don't like.
# The removed ones are listed here:
#  -Wunused-parameter

GCC_WEXTRA_FLAGS = \
	-Wclobbered \
	-Wempty-body \
	-Wignored-qualifiers \
	-Wmissing-field-initializers \
	-Wmissing-parameter-type \
	-Wold-style-declaration \
	-Woverride-init \
	-Wsign-compare \
	-Wtype-limits \
	-Wuninitialized \
	-Wunused-but-set-parameter

# GCC_MORE_WARNING_FLAGS
#
# Other warning flags we want to use but are not included neither in -Wall neither in GCC_WEXTRA_FLAGS
# The following may be enabled in future:
# -Werror
#
# Note: GCC version 4.6.3 (and lower?) does not support -Wmaybe-uninitialized

GCC_MORE_WARNING_FLAGS = \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wuninitialized \
	-Werror=cpp \
	-Wformat=0 \
	-Werror=format-extra-args \
	-Werror=format \
	-Werror=switch \
	-Werror=strict-aliasing \
	-Werror=unused-variable \
	-Werror=implicit-function-declaration \
	-Wpointer-arith


# GCC flags:
# -x c: forces compilation as C sources also for lib_steci's *.cpp
#       files which are actually C sources named cpp for compatibility with
#       MDR_CONTROL_APPLICATION. Without this option there would be linker errors.
CFLAGS   = -Wall $(GCC_WEXTRA_FLAGS) $(GCC_MORE_WARNING_FLAGS) -x c $(INCLUDES) $(DEFINES)
CXXFLAGS = -Wall $(GCC_WEXTRA_FLAGS) $(GCC_MORE_WARNING_FLAGS) -x c $(INCLUDES) $(DEFINES)
LDFLAGS  = -Xlinker -Map=$(ODIR)/output.map

# coverage
ifdef CONFIG_DEBUG_COVERAGE
 CFLAGS   += -fprofile-arcs -ftest-coverage
 CXXFLAGS += -fprofile-arcs -ftest-coverage
 LDFLAGS  += -lgcov --coverage
 GCOV_FLAGS = -b
endif

# the option -fstack-usage creates a *.su file per source compiled
# containing the summary of the stack used by each function in the file
# (man gcc for details)
ifdef CONFIG_DEBUG_STACK_USAGE
 CFLAGS   += -fstack-usage
 CXXFLAGS += -fstack-usage
endif

ifdef CONFIG_DEBUG_SYMBOLS
 CFLAGS   += -g
 CXXFLAGS += -g
 LDFLAGS  += -g
endif

ifdef CONFIG_DEBUG_INST_FUNC
 CFLAGS   += -finstrument-functions -finstrument-functions-exclude-file-list=inst_func_trace.c -finstrument-functions-exclude-function-list=clock_gettime
 CXXFLAGS += -finstrument-functions -finstrument-functions-exclude-file-list=inst_func_trace.c -finstrument-functions-exclude-function-list=clock_gettime
endif

# -mthumb-interwork
#  Generate code that supports calling between the ARM and Thumb instruction sets.  Without this option, on
#  pre-v5 architectures, the two instruction sets cannot be reliably used inside one program.  The default
#  is -mno-thumb-interwork, since slightly larger code is generated when -mthumb-interwork is specified.
#  In AAPCS configurations this option is meaningless.
ifdef CONFIG_COMPILER_GCC_CROSS_THUMB
 CFLAGS   += -mthumb
 CXXFLAGS += -mthumb
endif

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

SPLINT_FLAGS_WEAK = \
	-preproc \
	-fileextensions \
	-emptyret \
	+posixlib \
	-unrecog \
	+skip-sys-headers \
	-Dpthread_t=int \
	-Dpthread_mutex_t=int \
	-Dpthread_attr_t=int \
	-Dpthread_cond_t=int \
	-Dsem_t=int

SPLINT_FLAGS_WEAK_ALL = \
	-weak \
	$(SPLINT_FLAGS_WEAK)

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

SPLINT_FLAGS_STANDARD = \
	-booltype tBool \
	+charindex \
	-noret \
	-usedef \
	-nullpass \
	-mustfreefresh \
	-boolcompare \
	-compdef \
	-uniondef \
	-observertrans \
	-statictrans \
	-nullret \
	-compmempass \
	-compdestroy \
	-onlytrans \
	-nullstate \
	-immediatetrans

SPLINT_FLAGS_STANDARD_ALL = \
	-standard \
	$(SPLINT_FLAGS_WEAK) \
	$(SPLINT_FLAGS_STANDARD)

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

SPLINT_FLAGS_OPTIONAL =

# SPLINT_FLAGS
#
# Final selection:
# select either SPLINT_FLAGS_STANDARD_ALL or SPLINT_FLAGS_WEAK_ALL
# in addition to SPLINT_FLAGS_OPTIONAL
#
# SPLINT_FLAGS_STANDARD_ALL is not reccomended due to the large number
# of false positives

SPLINT_FLAGS = \
	$(SPLINT_FLAGS_OPTIONAL) \
	$(SPLINT_FLAGS_STANDARD_ALL)

#ebinder compilation 
ifdef  USER_EB_HOME
#specific for PC-LINT
OBJ_HOME = tunerifCMOS/ETAL_trunk
LIB = etal
endif

#
# CTAGS flags subsection
#

# CTAGS_FLAGS
#
# -R: recurse into directories (DO NOT REMOVE THIS ONE)
# --exclude=t_kernel*: many functions doubly defined in linux and t-kernel, avoid duplicate entries in TAGS file
# --verbose=yes: [optional, for debug] print list of files being considered
CTAGS_FLAGS = \
	-R                  \
	--exclude=t_kernel* \
	--exclude=*.o       \
	--exclude=*.a 

# DOXYGEN flags
# No flags are specified here, refer to DOXYFILE

# various utility flags
# ECHO_NNFLAG = don't issue a newline after the echo
ECHO_NNFLAG = \
	-n

#########
# PATHS
#########

PATH_ETALCORE            = $(ROOTDIR)/etalcore
PATH_ETALTML             = $(ROOTDIR)/etaltml
PATH_ETALCOMMON          = $(ROOTDIR)/etalcommon
PATH_TUNERDRIVER         = $(ROOTDIR)/tuner_driver
PATH_DCOPDRIVER          = $(ROOTDIR)/dcop_driver
PATH_DCOPDRIVER_EXPORTS  = $(PATH_DCOPDRIVER)/exports
PATH_TOOLS               = $(ROOTDIR)/tools
PATH_ETALCORE_EXPORTS    = $(PATH_ETALCORE)/exports
PATH_TUNERDRIVER_EXPORTS = $(PATH_TUNERDRIVER)/exports
PATH_TESTBUILD           = $(CURDIR)/testbuild
PATH_GLOBAL_CONFIG       = $(ROOTDIR)/.
PATH_CONFIG              = $(CURDIR)/.
PATH_GCOV_OUT            = $(CURDIR)/gcov
PATH_APPLICATIONS        = $(ROOTDIR)/applications
PATH_ETALMDR             = $(ROOTDIR)/etalmdr

# when inserting new directories in the project
# update this variables section
# etalcore
PATH_LIBETALCORE_SOURCES = $(PATH_ETALCORE)/lib_etalcore/src
PATH_LIBETALCORE_INCLUDES= $(PATH_ETALCORE)/lib_etalcore/include
PATH_LIBETAL_PARAMS      = $(PATH_ETALCORE_EXPORTS)/etalparam
PATH_ETALTEST_SOURCES    = $(PATH_ETALCORE)/lib_etalcore/test
PATH_ETALTEST_INCLUDES   = $(PATH_ETALCORE)/lib_etalcore/test
PATH_LIBIPF_SOURCES      = $(PATH_ETALCORE)/lib_ipforward/src
PATH_LIBIPF_INCLUDES     = $(PATH_ETALCORE)/lib_ipforward/include
PATH_APPLICATIONS_SOURCES    =
ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
PATH_APPLICATIONS_SOURCES += $(PATH_APPLICATIONS)/etalDcopMdrFlash
endif
# etaltml
PATH_LIBRDS_SOURCES      = $(PATH_ETALTML)/lib_rds/src
PATH_LIBRDS_INCLUDES     = $(PATH_ETALTML)/lib_rds/include
PATH_LIBETALTML_SOURCES  = $(PATH_ETALTML)/lib_etaltml/src
PATH_LIBETALTML_INCLUDES = $(PATH_ETALTML)/lib_etaltml/include
PATH_LIBSF_SOURCES       = $(PATH_ETALTML)/lib_servicefollowing/src
PATH_LIBSF_INCLUDES      = $(PATH_ETALTML)/lib_servicefollowing/include
# etalcommon
ifndef USER_EB_HOME
PATH_LIBOSALCORE_SOURCES = $(PATH_ETALCOMMON)/lib_osal/src/osalcore/linux
PATH_LIBBSP_SOURCES      = $(PATH_ETALCOMMON)/lib_bsp/src/linux_sta1X95
else
PATH_LIBOSALCORE_SOURCES = $(PATH_ETALCOMMON)/lib_osal/src/osalcore/t_kernel
PATH_LIBBSP_SOURCES      = $(PATH_ETALCOMMON)/lib_bsp/src/t_kernel_sta1095
endif
PATH_LIBOSALCORE_INCLUDES= $(PATH_ETALCOMMON)/lib_osal/include/osalcore
ifdef USER_EB_HOME
PATH_LIBOSALCORE_INCLUDES_TK = $(PATH_LIBOSALCORE_INCLUDES)/t_kernel
endif
PATH_OSALTEST_SOURCES    = $(PATH_ETALCOMMON)/lib_osal/test
PATH_OSALTEST_INCLUDES   = $(PATH_ETALCOMMON)/lib_osal/test
PATH_LIBBSP_INCLUDES     = $(PATH_ETALCOMMON)/lib_bsp/include
PATH_LIBCOMMON_SOURCES   = $(PATH_ETALCOMMON)/lib_common/src
PATH_LIBCOMMON_INCLUDES  = $(PATH_ETALCOMMON)/lib_common/include
# tuner_driver
PATH_LIBCMOST_SOURCES    = $(PATH_TUNERDRIVER)/lib_cmost/src
PATH_LIBCMOST_INCLUDES   = $(PATH_TUNERDRIVER)/lib_cmost/include
PATH_LIBBOOT_SOURCES     = $(PATH_TUNERDRIVER)/lib_boot/src
PATH_LIBBOOT_INCLUDES    = $(PATH_TUNERDRIVER)/lib_boot/include
PATH_LIBTUNERDRV_SOURCES = $(PATH_TUNERDRIVER)/lib_tunerdriver/src
PATH_LIBTUNERDRV_INCLUDES= $(PATH_TUNERDRIVER)/lib_tunerdriver/include
PATH_TUNERDRIVER_FIRMWARE= $(PATH_TUNERDRIVER_EXPORTS)/firmware
PATH_TUNERDRIVER_PARAM   = $(PATH_TUNERDRIVER_EXPORTS)/param
# dcop_driver
PATH_LIBBOOT_DCOP_SOURCES     = $(PATH_DCOPDRIVER)/lib_boot/src
PATH_LIBBOOT_DCOP_INCLUDES    = $(PATH_DCOPDRIVER)/lib_boot/include
PATH_LIBSTECI_SOURCES    	  = $(PATH_DCOPDRIVER)/lib_steci/src
PATH_LIBSTECI_INCLUDES        = $(PATH_DCOPDRIVER)/lib_steci/include
PATH_LIBHDRADIO_SOURCES       = $(PATH_DCOPDRIVER)/lib_hdradio/src
PATH_LIBHDRADIO_INCLUDES      = $(PATH_DCOPDRIVER)/lib_hdradio/include
# tools
PATH_TOOLSINSTFUNC_SOURCES = $(PATH_TOOLS)/inst_func/src
# config validate
# since this is in the root of the project we explicitly list the sources here
# we want the .o located in the target/linux/o, see below CONFIG_VALIDATE_OBJECTS
CONFIG_VALIDATE_SOURCES  = $(PATH_GLOBAL_CONFIG)/target_config_validate.c
# etalmdr
ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
PATH_ETALMDR_LIBCOMMON_INCLUDES = $(PATH_ETALMDR)/lib_common/src
PATH_ETALMDR_STREAMDECADAPT_INCLUDES = $(PATH_ETALMDR)/lib_streamdecadapt/include
endif
ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
PATH_ETALMDR_PADMNGR_INCLUDES = $(PATH_ETALMDR)/lib_dabmw/src/core/dab/mngr/pad
PATH_ETALMDR_COMM_INCLUDES = $(PATH_ETALMDR)/lib_dabmw/src/comm/host
PATH_ETALMDR_SYSTEM_INCLUDES = $(PATH_ETALMDR)/lib_dabmw/src/core/system
endif

ifdef CONFIG_COMM_DRIVER_DIRECT
PATH_RADIO_IF_INCLUDES   = $(ROOTDIR)/applications/radio_if
endif
# etalcore
LIBETALCORE_SOURCES:= $(wildcard $(PATH_LIBETALCORE_SOURCES)/*.c)
LIBETALCORE_OBJECTS:= $(patsubst %.c,%.o,$(LIBETALCORE_SOURCES))
LIBIPF_SOURCES     := $(wildcard $(PATH_LIBIPF_SOURCES)/*.c)
LIBIPF_OBJECTS     := $(patsubst %.c,%.o,$(LIBIPF_SOURCES))
# etaltml
LIBRDS_SOURCES     := $(wildcard $(PATH_LIBRDS_SOURCES)/*.c)
LIBRDS_OBJECTS     := $(patsubst %.c,%.o,$(LIBRDS_SOURCES))
LIBETALTML_SOURCES := $(wildcard $(PATH_LIBETALTML_SOURCES)/*.c)
LIBETALTML_OBJECTS := $(patsubst %.c,%.o,$(LIBETALTML_SOURCES))
LIBSF_SOURCES      := $(wildcard $(PATH_LIBSF_SOURCES)/*.c)
LIBSF_OBJECTS      := $(patsubst %.c,%.o,$(LIBSF_SOURCES))
# etalcommon
LIBOSALCORE_SOURCES:= $(wildcard $(PATH_LIBOSALCORE_SOURCES)/*.c)
LIBOSALCORE_OBJECTS:= $(patsubst %.c,%.o,$(LIBOSALCORE_SOURCES))
LIBOSAL_SOURCES     = $(LIBOSALCORE_SOURCES)
LIBOSAL_OBJECTS     = $(LIBOSALCORE_OBJECTS)
LIBBSP_SOURCES     := $(wildcard $(PATH_LIBBSP_SOURCES)/*.c)
LIBBSP_OBJECTS     := $(patsubst %.c,%.o,$(LIBBSP_SOURCES))
LIBCOMMON_SOURCES  := $(wildcard $(PATH_LIBCOMMON_SOURCES)/*.c)
LIBCOMMON_OBJECTS  := $(patsubst %.c,%.o,$(LIBCOMMON_SOURCES))
OSALTEST_SOURCES   := $(wildcard $(PATH_OSALTEST_SOURCES)/*.c)
OSALTEST_OBJECTS   := $(patsubst %.c,%.o,$(OSALTEST_SOURCES))
ETALTEST_SOURCES   := $(wildcard $(PATH_ETALTEST_SOURCES)/*.c)
ETALTEST_OBJECTS   := $(patsubst %.c,%.o,$(ETALTEST_SOURCES))
APPLICATIONS_SOURCESC   := $(wildcard $(PATH_APPLICATIONS_SOURCES)/*.c)
APPLICATIONS_SOURCESCPP := $(wildcard $(PATH_APPLICATIONS_SOURCES)/*.cpp)
APPLICATIONS_OBJECTS := $(patsubst %.c,%.o,$(APPLICATIONS_SOURCESC)) $(patsubst %.cpp,%.o,$(APPLICATIONS_SOURCESCPP))
TOOLSINSTFUNC_SOURCES := $(wildcard $(PATH_TOOLSINSTFUNC_SOURCES)/*.c)
TOOLSINSTFUNC_OBJECTS := $(patsubst %.c,%.o,$(TOOLSINSTFUNC_SOURCES))
# tuner_driver
LIBTUNERDRV_SOURCES:= $(wildcard $(PATH_LIBTUNERDRV_SOURCES)/*.c)
LIBTUNERDRV_OBJECTS:= $(patsubst %.c,%.o,$(LIBTUNERDRV_SOURCES))
LIBCMOST_SOURCESC  := $(wildcard $(PATH_LIBCMOST_SOURCES)/*.c)
LIBCMOST_SOURCESCPP:= $(wildcard $(PATH_LIBCMOST_SOURCES)/*.cpp)
LIBCMOST_SOURCES   := $(LIBCMOST_SOURCES) $(LIBCMOST_SOURCES)
LIBCMOST_OBJECTS   := $(patsubst %.c,%.o,$(LIBCMOST_SOURCESC))  $(patsubst %.cpp,%.o,$(LIBCMOST_SOURCESCPP))
LIBBOOT_SOURCES    := $(wildcard $(PATH_LIBBOOT_SOURCES)/*.c)
LIBBOOT_OBJECTS    := $(patsubst %.c,%.o,$(LIBBOOT_SOURCES))
# dcop_driver
LIBSTECI_SOURCESC  := $(wildcard $(PATH_LIBSTECI_SOURCES)/*.c)
LIBSTECI_SOURCESCPP:= $(wildcard $(PATH_LIBSTECI_SOURCES)/*.cpp)
LIBSTECI_SOURCES   := $(LIBSTECI_SOURCESC) $(LIBSTECI_SOURCESCPP)
LIBSTECI_OBJECTS   := $(patsubst %.c,%.o,$(LIBSTECI_SOURCESC)) $(patsubst %.cpp,%.o,$(LIBSTECI_SOURCESCPP))
LIBHDRADIO_SOURCESC:= $(wildcard $(PATH_LIBHDRADIO_SOURCES)/*.c)
LIBHDRADIO_SOURCESCPP:= $(wildcard $(PATH_LIBHDRADIO_SOURCES)/*.cpp)
LIBHDRADIO_OBJECTS := $(patsubst %.c,%.o,$(LIBHDRADIO_SOURCESC)) $(patsubst %.cpp,%.o,$(LIBHDRADIO_SOURCESCPP))
LIBBOOT_DCOP_SOURCESC:= $(wildcard $(PATH_LIBBOOT_DCOP_SOURCES)/*.c)
LIBBOOT_DCOP_SOURCESCPP:= $(wildcard $(PATH_LIBBOOT_DCOP_SOURCES)/*.cpp)
LIBBOOT_DCOP_OBJECTS := $(patsubst %.c,%.o,$(LIBBOOT_DCOP_SOURCESC)) $(patsubst %.cpp,%.o,$(LIBBOOT_DCOP_SOURCESCPP))

# config validate
# We don't want to put the object in the root
CONFIG_VALIDATE_OBJECTS := $(OBJ_DIR)/target_config_validate.o

# etalcore.a library from tuner_driver, etalcore and etalcommon sources
LIBRARY_ETALCORE_SOURCES = \
			$(LIBETALCORE_SOURCES) \
			$(LIBIPF_SOURCES)
LIBRARY_ETALCORE_OBJECTS = \
			$(LIBETALCORE_OBJECTS) \
			$(LIBIPF_OBJECTS)
LIBRARY_ETALCORE_INCLUDES = \
			-I$(PATH_LIBETALCORE_INCLUDES) \
			-I$(PATH_ETALCORE_EXPORTS) \
			-I$(PATH_LIBETAL_PARAMS) \
			-I$(PATH_TUNERDRIVER_FIRMWARE) \
			-I$(PATH_TUNERDRIVER_PARAM) \
			-I$(PATH_LIBIPF_INCLUDES)	
ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
LIBRARY_ETALCORE_INCLUDES += \
			-I$(PATH_ETALMDR_LIBCOMMON_INCLUDES) \
			-I$(PATH_ETALMDR_STREAMDECADAPT_INCLUDES)
endif
ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
LIBRARY_ETALCORE_INCLUDES += \
			-I$(PATH_ETALMDR_PADMNGR_INCLUDES) \
			-I$(PATH_ETALMDR_COMM_INCLUDES) \
			-I$(PATH_ETALMDR_SYSTEM_INCLUDES)
endif
LIBRARY_ETALCORE_SOURCES_PATH = \
			$(PATH_LIBETALCORE_SOURCES) \
			$(PATH_ETALCORE_EXPORTS) \
			$(PATH_LIBETAL_PARAMS) \
			$(PATH_LIBIPF_SOURCES)
LIBRARY_ETALCORE_INCLUDES_PATH = \
			$(PATH_LIBETALCORE_INCLUDES) \
			$(PATH_ETALCORE_EXPORTS) \
			$(PATH_LIBETAL_PARAMS) \
			$(PATH_TUNERDRIVER_FIRMWARE) \
			$(PATH_TUNERDRIVER_PARAM) \
			$(PATH_LIBIPF_INCLUDES)
ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
LIBRARY_ETALCORE_INCLUDES_PATH += \
			$(PATH_ETALMDR_LIBCOMMON_INCLUDES)
endif
ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
LIBRARY_ETALCORE_INCLUDES_PATH += \
			$(PATH_ETALMDR_PADMNGR_INCLUDES)
endif

ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO 
LIBRARY_ETALCORE_INCLUDES += -I$(PATH_DCOPDRIVER_EXPORTS)
LIBRARY_ETALCORE_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS)
endif	

ifdef CONFIG_COMM_DRIVER_DIRECT
LIBRARY_ETALCORE_INCLUDES += \
			-I$(PATH_RADIO_IF_INCLUDES)
LIBRARY_ETALCORE_INCLUDES_PATH += \
			$(PATH_RADIO_IF_INCLUDES)
endif# etaltml library from tml sources, not exported
LIBRARY_ETALTML_SOURCES = \
			$(LIBETALTML_SOURCES) \
			$(LIBRDS_SOURCES) \
			$(LIBSF_SOURCES)
LIBRARY_ETALTML_OBJECTS = \
			$(LIBETALTML_OBJECTS) \
			$(LIBRDS_OBJECTS) \
			$(LIBSF_OBJECTS)
LIBRARY_ETALTML_INCLUDES = \
			-I$(PATH_LIBETALTML_INCLUDES) \
			-I$(PATH_LIBRDS_INCLUDES) \
			-I$(PATH_LIBSF_INCLUDES)
LIBRARY_ETALTML_SOURCES_PATH = \
			$(PATH_LIBETALTML_SOURCES) \
			$(PATH_LIBRDS_SOURCES) \
			$(PATH_LIBSF_SOURCES)
LIBRARY_ETALTML_INCLUDES_PATH = \
			$(PATH_LIBETALTML_INCLUDES) \
			$(PATH_LIBRDS_INCLUDES) \
			$(PATH_LIBSF_INCLUDES)

# common library, not exported
LIBRARY_ETALCOMMON_SOURCES = \
			$(LIBBSP_SOURCES) \
			$(LIBCOMMON_SOURCES) \
			$(LIBOSAL_SOURCES)
LIBRARY_ETALCOMMON_OBJECTS = \
			$(LIBBSP_OBJECTS) \
			$(LIBCOMMON_OBJECTS) \
			$(LIBOSAL_OBJECTS)

LIBRARY_ETALCOMMON_INCLUDES = \
			-I$(PATH_LIBBSP_INCLUDES) \
			-I$(PATH_LIBCOMMON_INCLUDES) \
			-I$(PATH_LIBOSALCORE_INCLUDES)

ifdef USER_EB_HOME
LIBRARY_ETALCOMMON_INCLUDES += -I$(PATH_LIBOSALCORE_INCLUDES_TK)
endif

LIBRARY_ETALCOMMON_SOURCES_PATH = \
			$(PATH_LIBBSP_SOURCES) \
			$(PATH_LIBCOMMON_SOURCES) \
			$(PATH_LIBOSALCORE_SOURCES)
LIBRARY_ETALCOMMON_INCLUDES_PATH = \
			$(PATH_LIBBSP_INCLUDES) \
			$(PATH_LIBCOMMON_INCLUDES) \
			$(PATH_LIBOSALCORE_INCLUDES)
ifdef USER_EB_HOME
LIBRARY_ETALCOMMON_INCLUDES_PATH += $(PATH_LIBOSALCORE_INCLUDES_TK)
endif

# tuner_driver.a library from tuner_driver and etalcommon sources
LIBRARY_TUNERDRIVER_SOURCES = \
			$(LIBTUNERDRV_SOURCES) \
			$(LIBCMOST_SOURCES) \
			$(LIBBOOT_SOURCES)

LIBRARY_TUNERDRIVER_OBJECTS = \
			$(LIBTUNERDRV_OBJECTS) \
			$(LIBCMOST_OBJECTS) \
			$(LIBBOOT_OBJECTS)

LIBRARY_TUNERDRIVER_INCLUDES = \
			-I$(PATH_TUNERDRIVER_EXPORTS) \
			-I$(PATH_TUNERDRIVER_FIRMWARE) \
			-I$(PATH_TUNERDRIVER_PARAM) \
			-I$(PATH_LIBTUNERDRV_INCLUDES) \
			-I$(PATH_LIBCMOST_INCLUDES) \
			-I$(PATH_LIBBOOT_INCLUDES)

LIBRARY_TUNERDRIVER_SOURCES_PATH = \
			$(PATH_LIBTUNERDRV_SOURCES) \
			$(PATH_LIBCMOST_SOURCES) \
			$(PATH_LIBBOOT_SOURCES)

LIBRARY_TUNERDRIVER_INCLUDES_PATH = \
			$(PATH_TUNERDRIVER_EXPORTS) \
			$(PATH_TUNERDRIVER_FIRMWARE) \
			$(PATH_TUNERDRIVER_PARAM) \
			$(PATH_LIBTUNERDRV_INCLUDES) \
			$(PATH_LIBCMOST_INCLUDES) \
			$(PATH_LIBBOOT_INCLUDES)

ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
LIBRARY_TUNERDRIVER_SOURCES += $(LIBHDRADIO_SOURCES)
LIBRARY_TUNERDRIVER_OBJECTS += $(LIBHDRADIO_OBJECTS)
LIBRARY_TUNERDRIVER_INCLUDES += -I$(PATH_LIBHDRADIO_INCLUDES) \
			-I$(PATH_DCOPDRIVER_EXPORTS)
LIBRARY_TUNERDRIVER_SOURCES_PATH += $(PATH_LIBHDRADIO_SOURCES)
LIBRARY_TUNERDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
			$(PATH_LIBHDRADIO_INCLUDES)
endif

ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
LIBRARY_TUNERDRIVER_SOURCES += $(LIBSTECI_SOURCES)
LIBRARY_TUNERDRIVER_OBJECTS += $(LIBSTECI_OBJECTS)
LIBRARY_TUNERDRIVER_INCLUDES += -I$(PATH_LIBSTECI_INCLUDES) \
			-I$(PATH_DCOPDRIVER_EXPORTS)			
LIBRARY_TUNERDRIVER_SOURCES_PATH += $(PATH_LIBSTECI_SOURCES)
LIBRARY_TUNERDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
			$(PATH_LIBSTECI_INCLUDES)			
endif

# dcop_driver.a library from dcop_driver and etalcommon sources
LIBRARY_DCOPDRIVER_SOURCES = 
LIBRARY_DCOPDRIVER_OBJECTS =
LIBRARY_DCOPDRIVER_INCLUDES =
LIBRARY_DCOPDRIVER_SOURCES_PATH =
LIBRARY_DCOPDRIVER_INCLUDES_PATH =

ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
LIBRARY_DCOPDRIVER_SOURCES += $(LIBHDRADIO_SOURCES) \
			$(LIBBOOT_DCOP_SOURCES)
LIBRARY_DCOPDRIVER_OBJECTS += $(LIBHDRADIO_OBJECTS) \
			$(LIBBOOT_DCOP_OBJECTS)
LIBRARY_DCOPDRIVER_INCLUDES += -I$(PATH_LIBHDRADIO_INCLUDES) \
			-I$(PATH_LIBBOOT_DCOP_INCLUDES) \
			-I$(PATH_DCOPDRIVER_EXPORTS)
LIBRARY_DCOPDRIVER_SOURCES_PATH += $(PATH_LIBHDRADIO_SOURCES) \
			$(PATH_LIBBOOT_DCOP_SOURCES)
LIBRARY_DCOPDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
			$(PATH_LIBHDRADIO_INCLUDES) \
			$(PATH_LIBBOOT_DCOP_INCLUDES)
endif

ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
LIBRARY_DCOPDRIVER_SOURCES += $(LIBSTECI_SOURCES)
LIBRARY_DCOPDRIVER_OBJECTS += $(LIBSTECI_OBJECTS)
LIBRARY_DCOPDRIVER_INCLUDES += -I$(PATH_LIBSTECI_INCLUDES) \
			-I$(PATH_DCOPDRIVER_EXPORTS)
LIBRARY_DCOPDRIVER_SOURCES_PATH += $(PATH_LIBSTECI_SOURCES)
LIBRARY_DCOPDRIVER_INCLUDES_PATH += $(PATH_DCOPDRIVER_EXPORTS) \
			$(PATH_LIBSTECI_INCLUDES)
endif

# etalmdr library from MDR sources, not exported
ifdef CONFIG_TARGET_APP_STREAMDEC__ENABLE
LIBRARY_ETALMDR_SOURCES =
LIBRARY_ETALMDR_ARCHIVES = $(LIBDIR)/libStreamDec/libStreamDec$(LIBEXT) \
	$(LIBDIR)/libOSAL/libOSAL$(LIBEXT) $(LIBDIR)/libCommon/libCommon$(LIBEXT) \
	$(LIBDIR)/libStreamDecAdapt/libStreamDecAdapt$(LIBEXT)
LIBRARY_ETALMDR_OBJECTS = 
ifdef CONFIG_TARGET_AUDIO_CODEC_CIDANA
LIBRARY_ETALMDR_ARCHIVES += $(LIBDIR)/libStreamDec/libciDec$(LIBEXT)
endif
ifdef CONFIG_TARGET_APP_DABMW__ENABLE
LIBRARY_ETALMDR_ARCHIVES += $(LIBDIR)/libDabMw/libDabMw$(LIBEXT) $(LIBDIR)/libDabMw/libDabMwCore$(LIBEXT)
ifdef CONFIG_UTILS_MSGS_SUPPORT
LIBRARY_ETALMDR_ARCHIVES += $(LIBDIR)/libUtils/libUtils$(LIBEXT)
endif
endif
LIBRARY_ETALMDR_INCLUDES =
LIBRARY_ETALMDR_SOURCES_PATH =
endif

# define lib
LIB_ETAL	= $(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT)

# variables section end

#########
# DEFINES
#########

DEFINES   = 

# board selection for etalcore/exports/etal_types.h which does not include target_config.h
# (it could, but this way it is clear which defines need to be exported)
ifdef CONFIG_BOARD_MMB
 DEFINES += -DCONFIG_BOARD_MMB
endif
ifdef CONFIG_BOARD_CMOST_MAIN
 DEFINES += -DCONFIG_BOARD_CMOST_MAIN
endif
ifdef CONFIG_BOARD_ACCORDO2
 DEFINES += -DCONFIG_BOARD_ACCORDO2
endif
ifdef CONFIG_MODULE_INTEGRATED
# assuming all MTD modules will have two CMOST dual channel devices
# this impacts on the ETAL_CAPA_MAX_TUNER definition in etal_types.h
 DEFINES += -DCONFIG_MODULE_INTEGRATED
endif

#########
# INCLUDES
#########

INCLUDES  = -I.

etal etaltest splint : INCLUDES += \
		$(LIBRARY_ETALCORE_INCLUDES)    \
		$(LIBRARY_ETALTML_INCLUDES)     \
		$(LIBRARY_TUNERDRIVER_INCLUDES) \
		$(LIBRARY_DCOPDRIVER_INCLUDES)  \
		$(LIBRARY_ETALCOMMON_INCLUDES)

etalcore : INCLUDES += \
		$(LIBRARY_ETALCORE_INCLUDES)    \
		$(LIBRARY_TUNERDRIVER_INCLUDES) \
		$(LIBRARY_DCOPDRIVER_INCLUDES)  \
		$(LIBRARY_ETALCOMMON_INCLUDES)

tuner_driver : INCLUDES += \
		$(LIBRARY_TUNERDRIVER_INCLUDES) \
		$(LIBRARY_ETALCOMMON_INCLUDES)

osaltest : INCLUDES += \
		$(LIBRARY_ETALCOMMON_INCLUDES)

# add the pclint parts
ifdef  USER_EB_HOME
#specific handling for pclint 
INCDIRS += \
	$(PARTS_HOME)/include/tunerif \
	$(PARTS_HOME)/include \
	$(LIBRARY_ETALCORE_INCLUDES_PATH)    \
	$(LIBRARY_ETALTML_INCLUDES_PATH)     \
	$(LIBRARY_TUNERDRIVER_INCLUDES_PATH) \
	$(LIBRARY_DCOPDRIVER_INCLUDES_PATH)  \
	$(LIBRARY_ETALCOMMON_INCLUDES_PATH)	
endif

# more target specific includes
etaltest  : INCLUDES  += -I$(PATH_ETALTEST_INCLUDES)
osaltest  : INCLUDES  += -I$(PATH_OSALTEST_INCLUDES)

#########
# PATHS for CTAGS, DOXYGEN and TAGS
#########

# Several applications use the same list of paths
# so we define them here an re-use
ETAL_LIBRARY_INCLUDES_PATH_LIST = \
		$(LIBRARY_ETALCORE_INCLUDES_PATH)    \
		$(LIBRARY_ETALTML_INCLUDES_PATH)     \
		$(LIBRARY_TUNERDRIVER_INCLUDES_PATH) \
		$(LIBRARY_DCOPDRIVER_INCLUDES_PATH)  \
		$(LIBRARY_ETALCOMMON_INCLUDES_PATH)

# We rely on the recursive behaviour of both
# DOXYGEN and CTAGS and pass only the top level directories
ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH = \
		$(PATH_ETALCORE)    \
		$(PATH_ETALTML)     \
		$(PATH_TUNERDRIVER) \
		$(PATH_DCOPDRIVER)  \
		$(PATH_ETALCOMMON)

ifdef CONFIG_APP_ETAL_LIBRARY
DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
endif
ifdef CONFIG_APP_ETAL_TEST
DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
endif
ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
DOXYGEN_INCLUDES_PATH = $(ETAL_LIBRARY_INCLUDES_PATH_LIST)
SPLINT_SOURCES_PATH = $(ETAL_LIBRARY_TOPLEVEL_SOURCES_PATH)
endif

ifdef CONFIG_APP_ETALCORE_LIBRARY
# same as CONFIG_APP_ETAL_LIBRARY but without LIBRARY_ETALTML_*
DOXYGEN_INCLUDES_PATH = \
		$(LIBRARY_ETALCORE_INCLUDES_PATH)    \
		$(LIBRARY_TUNERDRIVER_INCLUDES_PATH) \
		$(LIBRARY_DCOPDRIVER_INCLUDES_PATH)  \
		$(LIBRARY_ETALCOMMON_INCLUDES_PATH)

SPLINT_SOURCES_PATH = \
		$(PATH_ETALCORE)    \
		$(PATH_TUNERDRIVER) \
		$(PATH_DCOPDRIVER)  \
		$(PATH_ETALCOMMON)
endif

ifdef CONFIG_APP_TUNERDRIVER_LIBRARY

DOXYGEN_INCLUDES_PATH = $(LIBRARY_ETALCOMMON_INCLUDES_PATH)
ifdef CONFIG_ETAL_SUPPORT_CMOST
DOXYGEN_INCLUDES_PATH += $(LIBRARY_TUNERDRIVER_INCLUDES_PATH)
endif
ifdef CONFIG_ETAL_SUPPORT_DCOP
DOXYGEN_INCLUDES_PATH += $(LIBRARY_DCOPDRIVER_INCLUDES_PATH)
endif

SPLINT_SOURCES_PATH = $(PATH_ETALCOMMON)
ifdef CONFIG_ETAL_SUPPORT_CMOST
SPLINT_SOURCES_PATH += $(PATH_TUNERDRIVER)
endif
ifdef CONFIG_ETAL_SUPPORT_DCOP
SPLINT_SOURCES_PATH += $(PATH_DCOPDRIVER)
endif
endif

ifdef CONFIG_APP_OSALCORE_TESTS
DOXYGEN_INCLUDES_PATH = \
		$(LIBRARY_ETALCOMMON_INCLUDES_PATH)

SPLINT_SOURCES_PATH = \
		$(PATH_ETALCOMMON)
endif

TAGS_SOURCES_PATH = $(SPLINT_SOURCES_PATH)

doxy : DOXYGEN_INCLUDES_PATH += $(PATH_CONFIG)

#########
# LIBRARIES
#########

LIBRARIES = -lpthread

ifdef CONFIG_HOST_OS_WIN32
etal      : LIBRARIES += -lwsock32
etaltest  : LIBRARIES += -lwsock32
endif
ifdef CONFIG_HOST_OS_LINUX
etal      : LIBRARIES += -lm -lrt
etaltest  : LIBRARIES += -lm -lrt
# begin of section for etalmdr makefile
ifdef CONFIG_TARGET_APP_STREAMDEC__ENABLE
etal      : LIBRARIES += $(LIBDIR)/libStreamDec/libStreamDec$(LIBEXT) $(LIBDIR)/libOSAL/libOSAL$(LIBEXT) $(LIBDIR)/libCommon/libCommon$(LIBEXT) $(LIBDIR)/libStreamDecAdapt/libStreamDecAdapt$(LIBEXT)
etalcore  : LIBRARIES += $(LIBDIR)/libStreamDec/libStreamDec$(LIBEXT) $(LIBDIR)/libOSAL/libOSAL$(LIBEXT) $(LIBDIR)/libCommon/libCommon$(LIBEXT) $(LIBDIR)/libStreamDecAdapt/libStreamDecAdapt$(LIBEXT)
etaltest  : LIBRARIES += $(LIBDIR)/libStreamDec/libStreamDec$(LIBEXT) $(LIBDIR)/libOSAL/libOSAL$(LIBEXT) $(LIBDIR)/libCommon/libCommon$(LIBEXT) $(LIBDIR)/libStreamDecAdapt/libStreamDecAdapt$(LIBEXT) -lrt
ifdef CONFIG_BOARD_ACCORDO5
ifdef CONFIG_TARGET_APP_DABMW__ENABLE
etal      : LIBRARIES += $(LIBDIR)/libDabMw/libDabMw$(LIBEXT) $(LIBDIR)/libDabMw/libDabMwCore$(LIBEXT)
etalcore  : LIBRARIES += $(LIBDIR)/libDabMw/libDabMw$(LIBEXT) $(LIBDIR)/libDabMw/libDabMwCore$(LIBEXT)
etaltest  : LIBRARIES += $(LIBDIR)/libDabMw/libDabMw$(LIBEXT) $(LIBDIR)/libDabMw/libDabMwCore$(LIBEXT)
ifdef CONFIG_UTILS_MSGS_SUPPORT
etal      : LIBRARIES += $(LIBDIR)/libUtils/libUtils$(LIBEXT)
etalcore  : LIBRARIES += $(LIBDIR)/libUtils/libUtils$(LIBEXT)
etaltest  : LIBRARIES += $(LIBDIR)/libUtils/libUtils$(LIBEXT)
endif
endif
ifdef CONFIG_TARGET_AUDIO_CODEC_CIDANA
etal      : LIBRARIES += $(LIBDIR)/libStreamDec/libciDec.a
etalcore  : LIBRARIES += $(LIBDIR)/libStreamDec/libciDec.a
etaltest  : LIBRARIES += $(LIBDIR)/libStreamDec/libciDec.a
endif
etal      : LIBRARIES += $(shell pkg-config --libs alsa)
etalcore  : LIBRARIES += $(shell pkg-config --libs alsa)
etaltest  : LIBRARIES += $(shell pkg-config --libs alsa)
endif
endif
# end of section for etalmdr makefile
endif

#########
# TARGETS
#########

# select the default target
ifdef CONFIG_APP_ETAL_LIBRARY
all : etal
endif
ifdef CONFIG_APP_ETALCORE_LIBRARY
all : etalcore
endif
ifdef CONFIG_APP_TUNERDRIVER_LIBRARY
all : tuner_driver
endif
ifdef CONFIG_APP_ETAL_TEST
all : etaltest
endif
ifdef CONFIG_APP_OSALCORE_TESTS
all : osaltest
endif
ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
all : etaltest
endif

# If ever a file named 'version' is created in the working
# directory, the rule that creates/updates the version
# file will refuse to do anything; the .PHONY directive
# instructs make to ignore the file and always run the recipe
.PHONY: version testbuild

# aliases for targets
etal         : $(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT)
etalcore     : $(PATH_ETALCORE_EXPORTS)/etalcore$(LIBEXT)
tuner_driver : $(PATH_TUNERDRIVER_EXPORTS)/tuner_driver$(LIBEXT)
etaltest     : $(OBJ_DIR)/etaltest$(EXEEXT)
osaltest     : $(OBJ_DIR)/osaltest$(EXEEXT)

#########
# RULES
#########

# WARNING: this makefile does not re-make .o for .h changes!
#$(warning This makefile does not support .h changes!)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
$(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT) : $(CONFIG_VALIDATE_OBJECTS) \
		$(LIBRARY_ETALCORE_OBJECTS)   \
		$(LIBRARY_ETALTML_OBJECTS)    \
		$(LIBRARY_TUNERDRIVER_OBJECTS) \
		$(LIBRARY_DCOPDRIVER_OBJECTS) \
		$(LIBRARY_ETALCOMMON_OBJECTS) \
		$(LIBRARY_ETALMDR_ARCHIVES) \
		| $(OBJ_DIR) $(LIBRARY_ETALMDR_ARCHIVES)
	$(ECHO) Building library $@

ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
	$(AR) r $@ $(wildcard $(ODIR)/libStreamDec/*.o) \
		$(wildcard $(ODIR)/libStreamDecAdapt/*.o) \
		$(wildcard $(ODIR)/libOSAL/*.o) \
		$(wildcard $(ODIR)/libDabMw/*.o) \
		$(wildcard $(ODIR)/libUtils/*.o) \
		$(wildcard $(ODIR)/libCommon/*.o) $^ $(SILENT_AR)
	cp -f $(LIBDIR)/libStreamDec/libciDec$(LIBEXT) $(PATH_ETALCORE_EXPORTS)
else
	$(AR) r $@ $(wildcard $(ODIR)/libStreamDec/*.o) \
		$(wildcard $(ODIR)/libStreamDecAdapt/*.o) \
		$(wildcard $(ODIR)/libOSAL/*.o) \
		$(wildcard $(ODIR)/libCommon/*.o) $^ $(SILENT_AR)
	cp -f $(LIBDIR)/libStreamDec/libciDec$(LIBEXT) $(PATH_ETALCORE_EXPORTS)
endif
else
	$(AR) r $@ $^ $(SILENT_AR)
endif

$(PATH_ETALCORE_EXPORTS)/etalcore$(LIBEXT) : $(CONFIG_VALIDATE_OBJECTS) \
		$(LIBRARY_ETALCORE_OBJECTS)   \
		$(LIBRARY_TUNERDRIVER_OBJECTS)\
		$(LIBRARY_DCOPDRIVER_OBJECTS) \
		$(LIBRARY_ETALCOMMON_OBJECTS) \
		$(LIBRARY_ETALMDR_OBJECTS)    \
		$(LIBRARY_ETALMDR_ARCHIVES)   \
		| $(OBJ_DIR) $(LIBRARY_ETALMDR_ARCHIVES)
	$(ECHO) Building library $@ 
ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
	$(AR) r $@ $(wildcard $(ODIR)/libStreamDec/*.o) \
		$(wildcard $(ODIR)/libStreamDecAdapt/*.o) \
		$(wildcard $(ODIR)/libOSAL/*.o) \
		$(wildcard $(ODIR)/libDabMw/*.o) \
		$(wildcard $(ODIR)/libUtils/*.o) \
		$(wildcard $(ODIR)/libCommon/*.o) $^ $(SILENT_AR)
	cp -f $(LIBDIR)/libStreamDec/libciDec$(LIBEXT) $(PATH_ETALCORE_EXPORTS)
else
	$(AR) r $@ $(wildcard $(ODIR)/libStreamDec/*.o) \
		$(wildcard $(ODIR)/libStreamDecAdapt/*.o) \
		$(wildcard $(ODIR)/libOSAL/*.o) \
		$(wildcard $(ODIR)/libCommon/*.o) $^ $(SILENT_AR)
	cp -f $(LIBDIR)/libStreamDec/libciDec$(LIBEXT) $(PATH_ETALCORE_EXPORTS)
endif
else
	$(AR) r $@ $^ $(SILENT_AR)
endif

$(PATH_TUNERDRIVER_EXPORTS)/tuner_driver$(LIBEXT) : $(CONFIG_VALIDATE_OBJECTS) \
		$(LIBRARY_TUNERDRIVER_OBJECTS)\
		$(LIBRARY_ETALCOMMON_OBJECTS) \
		| $(OBJ_DIR)
	$(ECHO) Building library $@ 
	$(AR) r $@ $^ $(SILENT_AR)

$(OBJ_DIR)/etaltest$(EXEEXT) : $(CONFIG_VALIDATE_OBJECTS) \
		$(ETALTEST_OBJECTS) \
		$(APPLICATIONS_OBJECTS) \
		$(LIB_ETAL) \
		| $(OBJ_DIR) $(LIBRARY_ETALMDR_ARCHIVES)
	$(ECHO) Building test application $@ 
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group
ifdef CONFIG_APP_ETAL_DCOP_MDR_FLASH
	$(SILENT) mv $(OBJ_DIR)/etaltest$(EXEEXT) $(OBJ_DIR)/etalDcopMdrFlash$(EXEEXT)
endif

$(OBJ_DIR)/osaltest$(EXEEXT) : $(CONFIG_VALIDATE_OBJECTS) \
		$(OSALTEST_OBJECTS) \
		$(LIBOSAL_OBJECTS)  \
		| $(OBJ_DIR)
	$(ECHO) Building OSAL test application $@ 
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

# target: version
# 
version :
	$(ECHO) Updating $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(ECHO) $(ECHO_NNFLAG) '#define ETAL_VERSION_SVN    ' >  $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(SVNVERSION) >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(ECHO)                '#define ETAL_VERSION_MAJOR  (tU8)' >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(ECHO)                '#define ETAL_VERSION_MIDDLE (tU8)' >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(ECHO)                '#define ETAL_VERSION_MINOR  (tU8)' >> $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(ECHO) Current version file content:
	$(ECHO) ----------------------------
	$(CAT) $(PATH_ETALCORE_EXPORTS)/$(ETAL_VERSION_FILE)
	$(ECHO) ----------------------------
	$(ECHO) Manually update the version numbers before committing
	$(ECHO) WARNING: the svn version number must not contain trailing letters otherwise the build will fail

# This rule exists only to print the message and output the
# target_config_validate.o in the $(OBJ_DIR) instead of the project root
$(CONFIG_VALIDATE_OBJECTS) : $(CONFIG_VALIDATE_SOURCES) $(OBJ_DIR)
	$(ECHO) Validating build configuration
	$(CC) -c -o $@ $(INCLUDES) $(CONFIG_VALIDATE_SOURCES)

$(OBJ_DIR) :
	$(ECHO) mkdir $@
	$(SILENT) mkdir $@

# target: clean
#
clean ::
	$(ECHO) Cleaning up
	$(SILENT) rm -rf \
		$(CONFIG_VALIDATE_OBJECTS) \
		$(LIBRARY_ETALCORE_OBJECTS) \
		$(LIBRARY_ETALTML_OBJECTS) \
		$(LIBRARY_TUNERDRIVER_OBJECTS) \
		$(LIBRARY_DCOPDRIVER_OBJECTS) \
		$(LIBRARY_ETALCOMMON_OBJECTS) \
		$(OSALTEST_OBJECTS) \
		$(ETALTEST_OBJECTS) \
		$(APPLICATIONS_OBJECTS) \
		$(OBJ_DIR)/* \
		$(PATH_ETALCORE_EXPORTS)/etal$(LIBEXT) \
		$(PATH_ETALCORE_EXPORTS)/etalcore$(LIBEXT) \
		$(PATH_ETALCORE_EXPORTS)/libciDec$(LIBEXT) \
		$(PATH_TUNERDRIVER_EXPORTS)/tuner_driver$(LIBEXT) \
		$(PATH_TOOLSINSTFUNC_SOURCES)/inst_func_trace.o

# target: covclean
# 
covclean :
	$(ECHO) Cleaning up coverage info
	$(SILENT) rm -rf \
		$(patsubst %o,%gcno,$(LIBRARY_ETALCORE_OBJECTS)) \
		$(patsubst %o,%gcda,$(LIBRARY_ETALCORE_OBJECTS)) \
		$(patsubst %o,%gcno,$(LIBRARY_ETALTML_OBJECTS)) \
		$(patsubst %o,%gcda,$(LIBRARY_ETALTML_OBJECTS)) \
		$(patsubst %o,%gcno,$(LIBRARY_TUNERDRIVER_OBJECTS)) \
		$(patsubst %o,%gcda,$(LIBRARY_TUNERDRIVER_OBJECTS)) \
		$(patsubst %o,%gcno,$(LIBRARY_DCOPDRIVER_OBJECTS)) \
		$(patsubst %o,%gcda,$(LIBRARY_DCOPDRIVER_OBJECTS)) \
		$(patsubst %o,%gcno,$(LIBRARY_ETALCOMMON_OBJECTS)) \
		$(patsubst %o,%gcda,$(LIBRARY_ETALCOMMON_OBJECTS)) \
		$(patsubst %o,%gcno,$(OSALTEST_OBJECTS)) \
		$(patsubst %o,%gcda,$(OSALTEST_OBJECTS)) \
		$(patsubst %o,%gcno,$(ETALTEST_OBJECTS)) \
		$(patsubst %o,%gcda,$(ETALTEST_OBJECTS)) \
		$(PATH_GCOV_OUT)/*

# target: eclean
#
# clean only ETAL objects; good for ETAL development, may result in incorrect build so use with care
eclean :
	$(ECHO) Cleaning up \(PARTIAL\)
	$(SILENT) rm -rf $(LIBETALCORE_OBJECTS) $(ETALTEST_OBJECTS) $(APPLICATIONS_OBJECTS) $(LIB_ETAL) $(OBJ_DIR)/etaltest$(EXEEXT)

# target: splint
#
# The following directories are omitted from the splint
# check because they come from MDR3 which has its own checker
	#$(PATH_LIBRDS_SOURCES)/*.c
	#$(PATH_ETALTEST_SOURCES)/*.c
	#$(PATH_LIBSF_SOURCES)/*.c
splint : 
	$(SPLINT) $(SPLINT_FLAGS) $(INCLUDES) \
		$(PATH_LIBETALCORE_SOURCES)/*.c \
		$(PATH_LIBIPF_SOURCES)/*.c \
		$(PATH_LIBETALTML_SOURCES)/*.c \
		$(PATH_LIBSTECI_SOURCES)/*.c \
		$(PATH_LIBSTECI_SOURCES)/*.cpp \
		$(PATH_LIBHDRADIO_SOURCES)/*.c \
		$(PATH_LIBHDRADIO_SOURCES)/*.cpp \
		$(PATH_LIBCMOST_SOURCES)/*.c \
		$(PATH_LIBCMOST_SOURCES)/*.cpp \
		$(PATH_LIBBOOT_SOURCES)/*.c \
		$(PATH_LIBTUNERDRV_SOURCES)/*.c \
		$(PATH_LIBOSALCORE_SOURCES)/*.c \
		$(PATH_LIBBSP_SOURCES)/*.c \
		$(PATH_LIBCOMMON_SOURCES)/*.c

#add-on ebinder pclint
ifdef  USER_EB_HOME
SRCS +=  \
	$(PATH_LIBETALCORE_SOURCES)/*.c \
	$(PATH_LIBETALTML_SOURCES)/*.c \
	$(PATH_LIBSTECI_SOURCES)/*.c \
	$(PATH_LIBSTECI_SOURCES)/*.cpp \
	$(PATH_LIBHDRADIO_SOURCES)/*.c \
	$(PATH_LIBHDRADIO_SOURCES)/*.cpp \
	$(PATH_LIBBOOT_DCOP_SOURCES)/*.c \
	$(PATH_LIBBOOT_DCOP_SOURCES)/*.cpp \
	$(PATH_LIBCMOST_SOURCES)/*.c \
	$(PATH_LIBCMOST_SOURCES)/*.cpp \
	$(PATH_LIBBOOT_SOURCES)/*.c \
	$(PATH_LIBTUNERDRV_SOURCES)/*.c \
	$(PATH_LIBOSALCORE_SOURCES)/*.c \
	$(PATH_LIBBSP_SOURCES)/*.c \
	$(PATH_LIBCOMMON_SOURCES)/*.c \
	$(PATH_LIBSF_SOURCES)/*.c

	include $(MAKE_HOME)/rules.mk
endif

# target: TAGS
#
TAGS :
	$(ECHO) Building $@ 
	$(SILENT) $(CTAGS) $(CTAGS_FLAGS) $(TAGS_SOURCES_PATH) $(CONFIGFILE)
 
# target: testbuild
#
testbuild :
	$(SILENT) MAKEFLAGS="$(MAKEFLAGS)" $(PATH_TESTBUILD)/testbuild.sh all

# target: testsplint
#
testsplint :
	$(SILENT) $(PATH_TESTBUILD)/testbuild.sh splint

# target: doxy
# 
doxy :
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
	$(SILENT) cat $(DOXYFILE) | \
		sed -e"s|\bINPUT\b *=|INPUT = $(SPLINT_SOURCES_PATH)|" \
			-e"s|\bINCLUDE_PATH\b *=|INCLUDE_PATH = $(DOXYGEN_INCLUDES_PATH)|" \
			-e"s|\bWARN_LOGFILE\b *=|WARN_LOGFILE = $(DOXYOUT)|" \
			- | \
			$(DOXYGEN) -
		$(ECHO) "Complete, warnings logged to $(DOXYOUT)"

# target: coverage
#
coverage : | $(PATH_GCOV_OUT)
# 2>&1 redirects both stdout and stderr to the file
	$(SILENT) $(GCOV) $(GCOV_FLAGS) $(LIBRARY_ETALCORE_SOURCES) $(LIBRARY_ETALTML_SOURCES) $(LIBRARY_ETALCOMMON_SOURCES) $(LIBRARY_TUNERDRIVER_SOURCES) $(LIBRARY_DCOPDRIVER_SOURCES) > $(GCOVOUT) 2>&1
	$(SILENT) mv *.gcov $(PATH_GCOV_OUT)
	$(ECHO) "Complete, gcov files moved to $(PATH_GCOV_OUT)"
	$(ECHO) "Output logged to $(GCOVOUT)"

$(PATH_GCOV_OUT) :
	$(ECHO) mkdir $@
	$(SILENT) mkdir $@

help:
	$(ECHO)  'Available targets:'
	$(ECHO)  '  clean   - Remove most generated files but keep the config'
	$(ECHO)  '  eclean  - Remove only the files generated from lib_etal (not recommended)'
	$(ECHO)  '  all     - Build the library or executable selected by the target_config'
	$(ECHO)  '            (etaltest, etal.a library, tuner_driver.a library or osaltest)'
	$(ECHO)  '            This is the default target if nothing is specified'
	$(ECHO)  '  version - Update and overwrite $(ETAL_VERSION_FILE)'
	$(ECHO)  '            Use only on a svn directry containing no local modifications'
	$(ECHO)  '            The minor, middle and major number must be edited by hand in the generated file'
	$(ECHO)  '  TAGS    - Build the tags file for editors'
	$(ECHO)  '  splint  - Invoke the static checker on a subset of the project files'
	$(ECHO)  '  doxy    - Generate Doxygen documentation for the library selected by target_config'
	$(ECHO)  '  coverage- Generate coverage information for all the ETAL sources'
	$(ECHO)  '  covclean- Remove all the coverage files (these are not cleaned by make clean)'
	$(ECHO)
	$(ECHO)  'Special targets:'
	$(ECHO)  '  testbuild  - Automatically build the project with several reference configurations to ensure there are no build errors'
	$(ECHO)  '  testsplint - Same as testbuild but builds with splint'

