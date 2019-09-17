include ../demo_common.mk

EXE_ETALDEMO_NAME=etaldemo_SS$(EXE_SUFFIX)

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_ETALDEMO_SOURCES  = .
PATH_LIBETAL_OSAL_CORE = $(ETAL_ROOT)/etalcommon/lib_osal/include/osalcore
PATH_LIBETAL_COMMON    = $(ETAL_ROOT)/etalcommon/lib_common/include
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif

INCLUDES = \
		-I$(PATH_ETALDEMO_SOURCES) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(PATH_LIBETAL_COMMON) \
		-I$(PATH_LIBETAL_OSAL_CORE) \
		-I$(PATH_LIBETAL_ROOT)

ETALDEMO_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_ETALDEMO_SOURCES)/*.c))
ETALDEMO_LIBRARIES_OBJECTS =$(PATH_LIBETAL_EXPORTS)/etal.a

ifeq "" "$(findstring DCONFIG_BOARD_ACCORDO5, $(DEFINES))"
ifneq ($(OS),Windows_NT)
	DEFINES += -DCONFIG_BOARD_ACCORDO2
endif
	DEFINES = -DHAVE_DAB -DHAVE_FM
else
ifneq "" "$(findstring DHAVE_DAB_SD, $(DEFINES))"
LIBRARIES += -lrt
ETALDEMO_LIBRARIES_OBJECTS += $(PATH_LIBETAL_EXPORTS)/libciDec.a
endif
endif

all : $(EXE_ETALDEMO_NAME)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
$(EXE_ETALDEMO_NAME) : $(ETALDEMO_OBJECTS) $(ETALDEMO_LIBRARIES_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(ETALDEMO_OBJECTS) $(EXE_ETALDEMO_NAME)
