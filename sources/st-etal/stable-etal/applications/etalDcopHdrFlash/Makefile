include ../applications_common.mk

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_LIBETAL_TUNERDRIVER_EXPORTS   = $(ETAL_ROOT)/tuner_driver/exports
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif
PATH_APPLICATIONS_SOURCES  = .

INCLUDES = \
		-I$(PATH_APPLICATIONS_SOURCES) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(PATH_LIBETAL_TUNERDRIVER_EXPORTS) \
		-I$(PATH_LIBETAL_ROOT)

ETALDCOPHDRFLASH_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_APPLICATIONS_SOURCES)/*.c))

# use this compilation switch when etal is build with compilation switch CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE or CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
#override DEFINES += -DCONFIG_ETAL_DCOP_FLASH_FIRMWARE_DL_FILE_MODE

all : etalDcopHdrFlash$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
etalDcopHdrFlash$(EXE_SUFFIX) : $(ETALDCOPHDRFLASH_OBJECTS) $(filter $(PATH_LIBETAL_EXPORTS)/etal%.a, $(wildcard $(PATH_LIBETAL_EXPORTS)/*.a))
	$(LD) $(LDFLAGS) -I$(INCLUDES) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(ETALDCOPHDRFLASH_OBJECTS) etalDcopHdrFlash$(EXE_SUFFIX)

