CC        = $(PREFIX)gcc
CXX       = $(PREFIX)gcc
LD        = $(PREFIX)gcc
CFLAGS    += -Wall $(INCLUDES) $(DEFINES)
CXXFLAGS  += -Wall -x c $(INCLUDES) $(DEFINES)
LDFLAGS   += -Xlinker
# add -g for debug
CFLAGS    += -g
CXXFLAGS  += -g
LDFLAGS   += -g
LIBRARIES = -lpthread

ifeq ($(OS),Windows_NT)
    PREFIX     = 
    EXE_SUFFIX = .exe
    # required only for EXTERNAL driver implementation
    LIBRARIES += -lwsock32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PREFIX     = arm-v7-linux-uclibceabi-
        EXE_SUFFIX =
        LIBRARIES += -lm
    endif
endif

ETAL_ROOT = ../..


