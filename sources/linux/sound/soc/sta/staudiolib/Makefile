MOD_PATH ?= sound/soc/sta/staudiolib
KDIR ?= .
SRC_DIR := FreeRTOS_M3/Demo/Common/STAudioLib/src
INC_DIR := FreeRTOS_M3/Demo/Common/STAudioLib/inc
SRC_FILES := \
	$(SRC_DIR)/API.c \
	$(SRC_DIR)/biquadFixed.c \
	$(SRC_DIR)/chime.c \
	$(SRC_DIR)/dsp.c \
	$(SRC_DIR)/internal.c \
	$(SRC_DIR)/list.c \
	$(SRC_DIR)/mixer.c \
	$(SRC_DIR)/module.c \
	$(SRC_DIR)/mathFixed.c \
	$(SRC_DIR)/dco.c \
	$(SRC_DIR)/gain.c \
	$(SRC_DIR)/spectrumMeter.c \
	$(SRC_DIR)/sta_cmd_parser.c \
	$(SRC_DIR)/sta_preset_loader.c \
	trace.c \
	user.c \
	st_codec_dsp.c

OBJ_FILES := $(patsubst %.c,%.o,$(SRC_FILES))

ifdef FORCE_MODULE
CONFIG_SND_SOC_STA_STAUDIOLIB = m
endif

obj-$(CONFIG_SND_SOC_STA_STAUDIOLIB) += staudiolib.o
staudiolib-y := $(OBJ_FILES)

ifdef CONFIG_STAUDIOLIB_EMBEDDED_FW
$(obj)/firmware.o: $(src)/dspfw/emerald-firmware.X.noheader $(src)/dspfw/emerald-firmware.Y.noheader $(src)/dspfw/emerald-firmware.P.noheader FORCE
staudiolib-y += firmware.o
AFLAGS_firmware.o := -I$(MOD_PATH)/
endif

ccflags-y = -DLINUX_OS -I$(MOD_PATH)/$(INC_DIR) -I$(MOD_PATH)/$(SRC_DIR) -I$(MOD_PATH)/Emerald/audio_lib/inc -I$(MOD_PATH)/Emerald/audio_programmable/inc \
-I$(MOD_PATH)/Emerald/dco_detect/dco_detect_ref/src/  -I$(MOD_PATH)/FreeRTOS_M3/Demo/Common/drivers/ST/STA10XXFWLib/inc/ \
-Isound/soc/sta

modules clean modules_install:
	$(MAKE) -C $(KDIR) M=$(MOD_PATH) $@

