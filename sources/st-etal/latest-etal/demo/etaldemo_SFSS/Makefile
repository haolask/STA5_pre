include ../demo_common.mk

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_LIBETAL_OSAL_CORE = $(ETAL_ROOT)/etalcommon/lib_osal/include/osalcore
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif
PATH_LIBETAL_COMMON		 = $(ETAL_ROOT)/etalcommon/lib_common/include
PATH_ETALDEMO_SF_SEEK_TUNE_SOURCES  = .
PATH_ETALDEMO_SF_SOURCES  = .

INCLUDES = \
		-I$(PATH_ETALDEMO_SF_SOURCES) \
		-I$(PATH_LIBETAL_OSAL_CORE) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(PATH_LIBETAL_ROOT) \
		-I$(PATH_LIBETAL_COMMON)


ETALDEMO_SF_SEEK_TUNE_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_ETALDEMO_SF_SEEK_TUNE_SOURCES)/etaldemo_SF_StartFromSeekOrTune.c))

ETALDEMO_SF_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_ETALDEMO_SF_SOURCES)/etaldemo_SF_bySidSelection.c))

ifneq ($(OS),Windows_NT)
DEFINES += -DCONFIG_BOARD_ACCORDO2
endif
DEFINES += -DHAVE_FM -DHAVE_DAB

all : etaldemo_SF_bySidSelection$(EXE_SUFFIX) etaldemo_SF_StartFromSeekOrTune$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
etaldemo_SF_StartFromSeekOrTune$(EXE_SUFFIX) : $(ETALDEMO_SF_SEEK_TUNE_OBJECTS) $(PATH_LIBETAL_EXPORTS)/etal.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

etaldemo_SF_bySidSelection$(EXE_SUFFIX) : $(ETALDEMO_SF_OBJECTS) $(PATH_LIBETAL_EXPORTS)/etal.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group
	
clean:
	rm -f $(ETALDEMO_SF_SEEK_TUNE_OBJECTS) etaldemo_SF_bySidSelection$(EXE_SUFFIX) $(ETALDEMO_SF_OBJECTS) etaldemo_SF_StartFromSeekOrTune$(EXE_SUFFIX) 


 
