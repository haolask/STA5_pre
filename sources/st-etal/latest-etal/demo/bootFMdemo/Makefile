include ../demo_common.mk

PATH_TUNERDRIVER_EXPORTS  = $(ETAL_ROOT)/tuner_driver/exports
PATH_BOOTFMDEMO_SOURCES  = .

INCLUDES = \
		-I$(PATH_BOOTFMDEMO_SOURCES) \
		-I$(PATH_TUNERDRIVER_EXPORTS)

BOOTFMDEMO_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_BOOTFMDEMO_SOURCES)/*.c))

all : bootFMdemo$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
bootFMdemo$(EXE_SUFFIX) : $(BOOTFMDEMO_OBJECTS) $(PATH_TUNERDRIVER_EXPORTS)/tuner_driver.a
	$(LD) $(LDFLAGS) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(BOOTFMDEMO_OBJECTS) bootFMdemo$(EXE_SUFFIX)

 
