CC        = $(PREFIX)gcc
LD        = $(PREFIX)gcc
CFLAGS    += -g -Wall $(INCLUDES) $(DEFINES)
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


