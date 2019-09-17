#
# radio_if - Linux only
#
include ../applications_common.mk

ifeq ($(OS),Windows_NT)
OBJ_DIR = $(ETAL_ROOT)/target/win32/o
else
OBJ_DIR = $(ETAL_ROOT)/target/linux/o
endif

PATH_LIBETAL_EXPORTS      = $(ETAL_ROOT)/etalcore/exports
PATH_LIBETAL_EXPORTS_ETALPARAM = $(ETAL_ROOT)/etalcore/exports/etalparam
PATH_LIBTUNERDRIVER_EXPORTS=$(ETAL_ROOT)/tuner_driver/exports
PATH_LIBTUNERDRIVER_INCLUDES=$(ETAL_ROOT)/tuner_driver/lib_tunerdriver/include
PATH_LIBCOMMON_INCLUDES   = $(ETAL_ROOT)/etalcommon/lib_common/include
PATH_LIBOSAL_INCLUDES     = $(ETAL_ROOT)/etalcommon/lib_osal/include/osalcore
PATH_LIBSTECI_INCLUDES    = $(ETAL_ROOT)/dcop_driver/lib_steci/include
PATH_DCOP_DRIVER_EXPORTS  = $(ETAL_ROOT)/dcop_driver/exports
PATH_LIBBOOT_INCLUDES     = $(ETAL_ROOT)/dcop_driver/lib_boot/include
PATH_LIBIPFORWARD_INCLUDES= $(ETAL_ROOT)/etalcore/lib_ipforward/include
PATH_LIBETALCORE_INCLUDES = $(ETAL_ROOT)/etalcore/lib_etalcore/include
PATH_LIBCMOST_INCLUDES    = $(ETAL_ROOT)/tuner_driver/lib_cmost/include
PATH_LIBETALTML_INCLUDES  = $(ETAL_ROOT)/etaltml/lib_etaltml/include
PATH_LIBSERVICEFOLLOWING_INCLUDES=$(ETAL_ROOT)/etaltml/lib_servicefollowing/include
PATH_RADIO_IF_SOURCES     = .

INCLUDES = \
		-I$(PATH_RADIO_IF_SOURCES) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(PATH_LIBETAL_EXPORTS_ETALPARAM) \
		-I$(PATH_LIBTUNERDRIVER_EXPORTS) \
		-I$(PATH_LIBTUNERDRIVER_INCLUDES) \
		-I$(PATH_LIBCOMMON_INCLUDES) \
		-I$(PATH_LIBOSAL_INCLUDES) \
		-I$(PATH_LIBSTECI_INCLUDES) \
		-I$(PATH_DCOP_DRIVER_EXPORTS) \
		-I$(PATH_LIBBOOT_INCLUDES) \
		-I$(PATH_LIBIPFORWARD_INCLUDES) \
		-I$(PATH_LIBETALCORE_INCLUDES) \
		-I$(PATH_LIBCMOST_INCLUDES) \
		-I$(PATH_LIBETALTML_INCLUDES) \
		-I$(PATH_LIBSERVICEFOLLOWING_INCLUDES) \
		-I$(ETAL_ROOT)/target/linux

RADIO_IF_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_RADIO_IF_SOURCES)/*.c)) \
					$(patsubst %.cpp,%.o,$(wildcard $(PATH_RADIO_IF_SOURCES)/*.cpp))

DEFINES += 	-DCONFIG_APP_RADIO_IF

#DEFINES += 	-DCONFIG_BUILD_DRIVER \
#			-DCONFIG_COMM_STECI_UART \
#			-DCONFIG_COMM_MCP \
#			-DCONFIG_TRACE_CLASS_STECI_UART=1 \
#			-DCONFIG_TRACE_CLASS_MCP=1 \
#			-DCONFIG_TRACE_CLASS_RIF_PROTOCOL_ROUTER=1 \
#			-DCONFIG_TRACE_CLASS_RIF_MSG_QUEUE=1 \
#			-DCONFIG_TRACE_CLASS_RIF_RIMW=1
#			-DCONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#			-DCONFIG_ETAL_SUPPORT_DCOP_MDR

LIBRARIES += -lm
RADIO_IF_LIBRARIES_OBJECTS = $(PATH_LIBETAL_EXPORTS)/etal.a
ifneq "" "$(findstring DHAVE_DAB_SD, $(DEFINES))"
LIBRARIES += -lrt
RADIO_IF_LIBRARIES_OBJECTS += $(PATH_LIBETAL_EXPORTS)/libciDec.a
endif

hd : DEFINES += -DHAVE_FM -DHAVE_HD

dab : DEFINES += -DHAVE_FM -DHAVE_DAB

dab_sd : DEFINES += -DHAVE_FM -DHAVE_DAB -DHAVE_DAB_SD

fm : DEFINES += -DHAVE_FM

all fm hd dab dab_sd : radio_if

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
radio_if : $(RADIO_IF_OBJECTS) $(RADIO_IF_LIBRARIES_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
		rm -f $(RADIO_IF_OBJECTS) radio_if

