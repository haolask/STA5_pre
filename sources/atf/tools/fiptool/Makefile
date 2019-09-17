#
# Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MAKE_HELPERS_DIRECTORY := ../../make_helpers/
include ${MAKE_HELPERS_DIRECTORY}build_macros.mk
include ${MAKE_HELPERS_DIRECTORY}build_env.mk

define MOVE_OBJS
        $(foreach obj,$(1),$(BUILD_BASE)/$(obj))
endef

BUILD_BASE ?= .
PROJECT := ${BUILD_BASE}/fiptool${BIN_EXT}
OBJECTS := fiptool.o tbbr_config.o
BUILD_BASED_OBJECTS := $(call MOVE_OBJS,$(OBJECTS))
V ?= 0

override CPPFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE=700
CFLAGS := -Wall -Werror -pedantic -std=c99
ifeq (${DEBUG},1)
  CFLAGS += -g -O0 -DDEBUG
else
  CFLAGS += -O2
endif
LDLIBS := -lcrypto

ifeq (${V},0)
  Q := @
else
  Q :=
endif

INCLUDE_PATHS := -I../../include/tools_share

HOSTCC ?= gcc

.PHONY: all clean distclean

all: ${PROJECT}

${PROJECT}: ${BUILD_BASED_OBJECTS} Makefile
	@echo "  LD      $@"
	${Q}${HOSTCC} ${BUILD_BASED_OBJECTS} -o $@ ${LDFLAGS} ${LDLIBS}
	@${ECHO_BLANK_LINE}
	@echo "Built $@ successfully"
	@${ECHO_BLANK_LINE}

${BUILD_BASE}/%.o: %.c %.h Makefile
	@echo "  CC      $<"
	${Q}${HOSTCC} -c ${CPPFLAGS} ${CFLAGS} ${INCLUDE_PATHS} $< -o $@

clean:
	$(call SHELL_DELETE_ALL, ${PROJECT} ${OBJECTS})
