#@TYPE: Machine
#@NAME: Accordo5 STA1295 EVB board
#@DESCRIPTION: Common machine configuration for Accordo5 STA1295 EVB board

# Enable Cortex A7 thumb neon and hard floating point
DEFAULTTUNE = "cortexa7thf-neon"
include conf/machine/include/sta1xxx-base.inc
include conf/machine/include/sta1295-sw-base.inc
include conf/machine/include/sta1295-default-providers.inc
include conf/machine/include/sta1295-machine-features.inc
include conf/machine/include/tune-cortexa7.inc

#Wifi firmware support
MACHINE_EXTRA_RDEPENDS += "linux-firmware-sd8887"

## MISC ##
SOC_FAMILY = "sta:sta1295"
SERIAL_CONSOLES = "115200;ttyAMA2"

## KERNEL ##
KERNEL_DEVICETREE ?= "sta1295-evb.dtb"
KERNEL_CONFIG_FRAGMENTS ?= "sta1295.config"
KBUILD_DEFCONFIG ?= "sta1295_defconfig"

## M3 bootloaders target configuration ##
# SoC ST1295
TARGET_SOC_ID ?= "SOCID_STA1295"
# Board A5 EVB
TARGET_BOARD_ID ?= "BOARD_A5_EVB"
# Boot on MMC, NAND or SQI
TARGET_BOOT_DEVICE ?= "${MEMORY_BOOT_DEVICE}"

## U-BOOT ##
UBOOT_ENTRYPOINT = "0xA0000000"
UBOOT_LOADADDRESS = "0xA0000000"
#First one should always be for the flashloader, and second for boot
UBOOT_CONFIG ?= "${MEMORY_BOOT_DEVICE} ${MEMORY_BOOT_DEVICE}_XLDR"

UBOOT_CONFIG[NAND] = "sta-nand-flashloader_defconfig,ubifs"
UBOOT_CONFIG[NAND_XLDR] = "sta-nand-xloader_defconfig,ubifs"

UBOOT_CONFIG[MMC] = "sta-mmc-flashloader_defconfig,ext4"
UBOOT_CONFIG[MMC_XLDR] = "sta-mmc-xloader_defconfig,ext4"

UBOOT_CONFIG[SQI] = "sta-sqi-flashloader_defconfig,ubifs"
UBOOT_CONFIG[SQI_XLDR] = "sta-sqi-xloader_defconfig,ubifs"

UBOOT_CONFIG[SQI_NAND] = "sta-sqi-nand-flashloader_defconfig,ubifs"
UBOOT_CONFIG[SQI_NAND_XLDR] = "sta-sqi-nand-xloader_defconfig,ubifs"

UBOOT_CONFIG[SQI_MMC] = "sta-sqi-mmc-flashloader_defconfig,ext4"
UBOOT_CONFIG[SQI_MMC_XLDR] = "sta-sqi-mmc-xloader_defconfig,ext4"

UBOOT_DEVICETREE  = "sta1295-evb"

### GPU ###
MACHINE_FEATURES_append = " gcnanogpuimpl"

### Generic flag to enable MM modules deployment ###
MACHINE_FEATURES_append = " multimedia"

#ETAL
KERNEL_MODULE_AUTOLOAD += "spidev"

#DSP FW
ST_DSP_FW_SUBPATH = "sound/soc/sta/staudiolib/dspfw"
ST_DSP_FW = "emerald-firmware.P.noheader"
ST_DSP_FW += "emerald-firmware.X.noheader"
ST_DSP_FW += "emerald-firmware.Y.noheader"

#EARLY AUDIO
ST_EARLY_AUDIO_SUBPATH = "early_audio"
