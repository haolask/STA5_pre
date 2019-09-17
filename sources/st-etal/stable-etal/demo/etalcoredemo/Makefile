include ../demo_common.mk

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_ETALDEMO_SOURCES  = .
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif

INCLUDES = \
		-I$(PATH_ETALDEMO_SOURCES) \
		-I$(PATH_LIBETAL_EXPORTS) \
		-I$(PATH_LIBETAL_ROOT)

ETALDEMO_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_ETALDEMO_SOURCES)/*.c))

ETAL_LIB := $(wildcard  $(PATH_LIBETAL_EXPORTS)/etal*.a)

ifeq "" "$(findstring DCONFIG_BOARD_ACCORDO5, $(DEFINES))"
ifneq ($(OS),Windows_NT)
DEFINES += -DCONFIG_BOARD_ACCORDO2
endif
else
ifneq "" "$(findstring DHAVE_DAB_SD, $(DEFINES))"
LIBRARIES += -lrt
ETAL_LIB += $(PATH_LIBETAL_EXPORTS)/libciDec.a
endif
endif

hd : DEFINES += -DHAVE_FM -DHAVE_HD

dab : DEFINES += -DHAVE_FM -DHAVE_DAB

fm : DEFINES += -DHAVE_FM

all : DEFINES += -DHAVE_FM

all fm dab hd : etalcoredemo$(EXE_SUFFIX)


# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites 
etalcoredemo$(EXE_SUFFIX) : $(ETALDEMO_OBJECTS) $(ETAL_LIB)
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(ETALDEMO_OBJECTS) etalcoredemo$(EXE_SUFFIX) 


 
