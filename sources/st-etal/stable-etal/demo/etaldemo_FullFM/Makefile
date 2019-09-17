include ../demo_common.mk

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_LIBETAL_OSAL_CORE = $(ETAL_ROOT)/etalcommon/lib_osal/include/osalcore
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif
PATH_LIBETAL_COMMON    = $(ETAL_ROOT)/etalcommon/lib_common/include
PATH_ETALDEMO_FULL_FM_SOURCES  = .

INCLUDES = \
		-I$(PATH_ETALDEMO_FULL_FM_SOURCES) \
		-I$(PATH_LIBETAL_OSAL_CORE) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(PATH_LIBETAL_ROOT) \
		-I$(PATH_LIBETAL_COMMON)


ETALDEMO_FULL_FM_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_ETALDEMO_FULL_FM_SOURCES)/etaldemo_FullFM.c))

ifeq "" "$(findstring DCONFIG_BOARD_ACCORDO5, $(DEFINES))"
ifneq ($(OS),Windows_NT)
	DEFINES += -DCONFIG_BOARD_ACCORDO2
endif
	DEFINE_ALL = -DHAVE_DAB -DHAVE_FM
endif

dab : DEFINES += -DHAVE_DAB

fm : DEFINES += -DHAVE_FM

all : DEFINES += $(DEFINE_ALL)

all fm dab : etaldemo_FullFM$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
etaldemo_FullFM$(EXE_SUFFIX) : $(ETALDEMO_FULL_FM_OBJECTS) $(PATH_LIBETAL_EXPORTS)/etal.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(ETALDEMO_FULL_FM_OBJECTS) etaldemo_FullFM$(EXE_SUFFIX)



