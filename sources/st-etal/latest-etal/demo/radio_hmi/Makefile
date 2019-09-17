#
# radio_hmi - Linux only
#
include ../demo_common.mk

ifeq ($(OS),Windows_NT)
OBJ_DIR = $(ETAL_ROOT)/target/win32/o
else
OBJ_DIR = $(ETAL_ROOT)/target/linux/o
endif

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_RADIOHMI_SOURCES  = .

INCLUDES = \
		-I$(PATH_RADIOHMI_SOURCES) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(ETAL_ROOT)/target/linux

RADIOHMI_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_RADIOHMI_SOURCES)/*.c))

ifneq ($(OS),Windows_NT)
DEFINES += -DCONFIG_BOARD_ACCORDO2
endif

LIBRARIES += -lm -levdev

#
# Set SYSROOT to the buildroot staging folder to compile
#
SYSROOT = $(A2_ROOT_VIEW)/buildroot/output/staging/
CFLAGS   += --sysroot=$(SYSROOT)
CXXFLAGS += --sysroot=$(SYSROOT)
LDFLAGS  += --sysroot=$(SYSROOT)

all : radio_hmi

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
radio_hmi : $(RADIOHMI_OBJECTS) $(PATH_LIBETAL_EXPORTS)/etal.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(RADIOHMI_OBJECTS) radio_hmi
