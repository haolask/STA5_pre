include ../applications_common.mk

PATH_LIBETAL_EXPORTS   = $(ETAL_ROOT)/etalcore/exports
PATH_LIBETAL_OSAL_CORE = $(ETAL_ROOT)/etalcommon/lib_osal/include/osalcore
ifeq ($(OS),Windows_NT)
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/win32
else
PATH_LIBETAL_ROOT      = $(ETAL_ROOT)/target/linux
endif
PATH_LIBETAL_COMMON		 = $(ETAL_ROOT)/etalcommon/lib_common/include
PATH_APPLICATIONS_SOURCES  = .

INCLUDES = \
		-I$(PATH_APPLICATIONS_SOURCES) \
		-I$(PATH_LIBETAL_OSAL_CORE) \
		-I$(PATH_LIBETAL_COMMON) \
		-I$(PATH_LIBETAL_ROOT) \
		-I$(PATH_LIBETAL_EXPORTS)


ETAL_PRODTESTAPP_OBJECTS := $(patsubst %.c,%.o,$(wildcard $(PATH_APPLICATIONS_SOURCES)/*.c))

ifeq "" "$(findstring DCONFIG_BOARD_ACCORDO5, $(DEFINES))"
	ifneq ($(OS),Windows_NT)
		DEFINES += -DCONFIG_BOARD_ACCORDO2
	endif
	DEFINE_ALL = -DHAVE_DAB -DHAVE_FM
endif

hd : DEFINES += -DHAVE_FM -DHAVE_HD

dab : DEFINES += -DHAVE_FM -DHAVE_DAB

fm : DEFINES += -DHAVE_FM

fm_star_t : DEFINES += -DHAVE_FM -DHAVE_STAR_T

fm_star_s : DEFINES += -DHAVE_FM -DHAVE_STAR_S

all : DEFINES += $(DEFINE_ALL)

all fm dab hd fm_star_t fm_star_s : etal_ProdTestApp$(EXE_SUFFIX)

# the following rules use make's automatic variables summarized here:
# $@ is the target name
# $^ is the list of prerequisites
etal_ProdTestApp$(EXE_SUFFIX) : $(ETAL_PRODTESTAPP_OBJECTS) $(filter $(PATH_LIBETAL_EXPORTS)/etal%.a, $(wildcard $(PATH_LIBETAL_EXPORTS)/*.a))
	$(LD) $(LDFLAGS) $(DEFINES) -I$(INCLUDES) -o $@ -Wl,--start-group $^ $(LIBRARIES) -Wl,--end-group

clean:
	rm -f $(ETAL_PRODTESTAPP_OBJECTS) etal_ProdTestApp$(EXE_SUFFIX) 


 
