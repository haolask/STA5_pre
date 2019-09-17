include ../demo_common.mk

PATH_TUNERDRIVER_EXPORTS  = $(ETAL_ROOT)/tuner_driver/exports
PATH_DCOPDRIVER_EXPORTS   = $(ETAL_ROOT)/dcop_driver/exports

PATH_BOOTDEMO_SOURCES  = .

INCLUDES = \
		-I$(PATH_BOOTDEMO_SOURCES) \
		-I$(PATH_TUNERDRIVER_EXPORTS) \
		-I$(PATH_DCOPDRIVER_EXPORTS)
BOOTDEMO_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_BOOTDEMO_SOURCES)/*.c))

all : bootdemo$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
bootdemo$(EXE_SUFFIX) : $(BOOTDEMO_OBJECTS) $(PATH_TUNERDRIVER_EXPORTS)/tuner_driver.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(BOOTDEMO_OBJECTS) bootdemo$(EXE_SUFFIX)

 
