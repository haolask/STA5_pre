#
# Copyright (C) 2014 STMicroelectronics
#

ifneq ($(EARLY_TUNER_FTR), 0)
ccflags-y += -DEARLY_TUNER_FTR
endif

obj-y	:= sta-evb.o

ifdef CONFIG_OF_LIBFDT_OVERLAY
obj-y += sta-dtbo.o
endif
