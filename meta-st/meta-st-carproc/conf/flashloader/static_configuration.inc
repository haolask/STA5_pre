
# This file contains the different flashloader configuration parts added
# according to the soc currently used.
# \n are used to add newlines in flashloader config file
#

#Supported pattern list:
export ENTRY_LIST="\
    FEATURE_ENTRY_ \
    DSP_FW_ENTRY_ \
    TUNER_FW_ENTRY_ \
    IMG_SPLASH_ENTRY_ \
    ANIM_SPLASH_ENTRY_ \
    AUDIOLIB_ENTRY_ \
    IMG_SPLASH_PART_ENTRY_\
    ANIM_SPLASH_PART_ENTRY_\
    AUDIO_EMMC_PART_ENTRY_\
    AUDIO_NAND_PART_ENTRY_\
    AUDIO_SQI_PART_ENTRY_ \
    M3_OS_NAND_PART_ENTRY_\
    TUNER_EMMC_PART_ENTRY_\
    TUNER_NAND_PART_ENTRY_\
    TUNER_SQI_PART_ENTRY_\
    KEY_STORAGE_MMC_PART_ENTRY_\
    KEY_STORAGE_SQI_PART_ENTRY_\
    "

##########################
# FW or file declaration #
##########################
export FEATURE_ENTRY_DEFAULT=""
export FEATURE_ENTRY_sta1295="car_radio"
export FEATURE_ENTRY_sta1275="${FEATURE_ENTRY_sta1295}"
export FEATURE_ENTRY_sta1195="telematics"
export FEATURE_ENTRY_sta1385="${FEATURE_ENTRY_sta1195}"
###
export DSP_FW_ENTRY_DEFAULT=""
export DSP_FW_ENTRY_sta1295="\
; -----------\n\
; DSP entries\n\
; -----------\n\
DSP0P = emerald-firmware.P.noheader, 0x48900000\n\
DSP0X = emerald-firmware.X0.noheader, 0x48980000\n\
DSP0Y = emerald-firmware.Y0.noheader, 0x489C0000\n\
DSP1P = emerald-firmware.P.noheader, 0x48A00000\n\
DSP1X = emerald-firmware.X1.noheader, 0x48A80000\n\
DSP1Y = emerald-firmware.Y1.noheader, 0x48AC0000\n\
;DSP2P = emerald-firmware.P.noheader, 0x48B00000\n\
;DSP2X = emerald-firmware.X.noheader, 0x48B80000\n\
;DSP2Y = emerald-firmware.Y.noheader, 0x48BC0000"
export DSP_FW_ENTRY_sta1275="${DSP_FW_ENTRY_sta1295}"
###
export TUNER_FW_ENTRY_DEFAULT=""
export TUNER_FW_ENTRY_sta1295="\
; -------------\n\
; TUNER entries\n\
; -------------\n\
;FIRMWARE1 = tuner_fwm.bin"
export TUNER_FW_ENTRY_sta1275="${TUNER_FW_ENTRY_sta1295}"
###
export IMG_SPLASH_ENTRY_DEFAULT=""
export IMG_SPLASH_ENTRY_sta1295="\
SPLASH			= splash.rgb"
export IMG_SPLASH_ENTRY_sta1275="\
SPLASH			= splash.rgb"
###
export ANIM_SPLASH_ENTRY_DEFAULT=""
export ANIM_SPLASH_ENTRY_sta1295="\
SPLASH_ANIMATION	= splash_animation.mov"
export ANIM_SPLASH_ENTRY_sta1275="\
SPLASH_ANIMATION	= splash_animation.mov"
###
export AUDIOLIB_ENTRY_DEFAULT=""
export AUDIOLIB_ENTRY_sta1295="\
AUDIOLIB		= early, 0x0\n\
FILE_1		= animation.wav, 0x0\n\
FILE_2		= rvc.wav, 0x0"
export AUDIOLIB_ENTRY_sta1275="${AUDIOLIB_ENTRY_sta1295}"

##################################
# Splash partition customization #
##################################
export IMG_SPLASH_PART_ENTRY_DEFAULT=""
export IMG_SPLASH_PART_ENTRY_sta1295=",1m(SPLASH)"
export IMG_SPLASH_PART_ENTRY_sta1275="${IMG_SPLASH_PART_ENTRY_sta1295}"
###
export ANIM_SPLASH_PART_ENTRY_DEFAULT=""
export ANIM_SPLASH_PART_ENTRY_sta1295=",2m(SPLASH_ANIMATION)"
export ANIM_SPLASH_PART_ENTRY_sta1275="${ANIM_SPLASH_PART_ENTRY_sta1295}"

###############################################
# eMMC configuration specific (32k bloc mini) #
###############################################
export AUDIO_EMMC_PART_ENTRY_DEFAULT=""
export AUDIO_EMMC_PART_ENTRY_sta1295="\
,32k(DSP0X),32k(DSP0Y),32k(DSP0P),32k(DSP1X),32k(DSP1Y),32k(DSP1P),\
128k(DSP2X),64k(DSP2Y),32k(DSP2P),512k(AUDIOLIB),512k(FILE_1),512k(FILE_2)"
export AUDIO_EMMC_PART_ENTRY_sta1275="${AUDIO_EMMC_PART_ENTRY_sta1295}"
###
export TUNER_EMMC_PART_ENTRY_DEFAULT=""
export TUNER_EMMC_PART_ENTRY_sta1295="\
,64k(FIRMWARE1)"
export TUNER_EMMC_PART_ENTRY_sta1275="${TUNER_EMMC_PART_ENTRY_sta1295}"
###
export KEY_STORAGE_MMC_PART_ENTRY_DEFAULT=""
export KEY_STORAGE_MMC_PART_ENTRY_sta1385=",64k(KEY_STORAGE)"

##################################################
# NAND configuration specific                    #
#   (512k*2 bloc mini) for A5/TC3_EVB            #
#   (128k*2 bloc mini) for TC3P_EVB              #
##################################################
export AUDIO_NAND_PART_ENTRY_DEFAULT=""
export AUDIO_NAND_PART_ENTRY_sta1295="\
,1m(DSP0X),1m(DSP0Y),1m(DSP0P),1m(DSP1X),1m(DSP1Y),\
1m(DSP1P),1m(DSP2X),1m(DSP2Y),1m(DSP2P),1m(AUDIOLIB),1m(FILE_1),1m(FILE_2)"
export AUDIO_NAND_PART_ENTRY_sta1275="${AUDIO_NAND_PART_ENTRY_sta1295}"
###
export M3_OS_NAND_PART_ENTRY_DEFAULT=",1m(M3_OS)"
export M3_OS_NAND_PART_ENTRY_sta1385=",256k(M3_OS)"
###
export TUNER_NAND_PART_ENTRY_DEFAULT=""
export TUNER_NAND_PART_ENTRY_sta1295="\
,1m(FIRMWARE1)"
export TUNER_NAND_PART_ENTRY_sta1275="${TUNER_NAND_PART_ENTRY_sta1295}"

################################################
# SQI configuration specific (16k*2 bloc mini) #
################################################
export AUDIO_SQI_PART_ENTRY_DEFAULT=""
export AUDIO_SQI_PART_ENTRY_sta1295="\
,64k(DSP0X),64k(DSP0Y),64k(DSP0P),64k(DSP1X),64k(DSP1Y),\
64k(DSP1P),128k(DSP2X),64k(DSP2Y),64k(DSP2P),512k(AUDIOLIB),512k(FILE_1),512k(FILE_2)"
export AUDIO_SQI_PART_ENTRY_sta1275="${AUDIO_SQI_PART_ENTRY_sta1295}"
###
export TUNER_SQI_PART_ENTRY_DEFAULT=""
export TUNER_SQI_PART_ENTRY_sta1295="\
,64k(FIRMWARE1)"
export TUNER_SQI_PART_ENTRY_sta1275="${TUNER_SQI_PART_ENTRY_sta1295}"
###
export KEY_STORAGE_SQI_PART_ENTRY_DEFAULT=""
export KEY_STORAGE_SQI_PART_ENTRY_sta1385=",128k(KEY_STORAGE)"
