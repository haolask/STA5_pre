include ../demo_common.mk

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif
PATH_ETALDEMO_SOURCES  = .

INCLUDES = \
		-I$(PATH_ETALDEMO_SOURCES) \
		-I$(PATH_LIBETAL_ROOT) \
		-I$(PATH_LIBETAL_EXPORTS)

ETALDEMO_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_ETALDEMO_SOURCES)/*.c))

ifneq ($(OS),Windows_NT)
DEFINES += -DCONFIG_BOARD_ACCORDO2
endif

all : etaldemoData$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
etaldemoData$(EXE_SUFFIX) : $(ETALDEMO_OBJECTS) $(PATH_LIBETAL_EXPORTS)/etal.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(ETALDEMO_OBJECTS) etaldemoData$(EXE_SUFFIX)

 
