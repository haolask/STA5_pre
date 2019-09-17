#@provides the Car Processor STA1XXX platforms common settings

include conf/machine/include/soc-family.inc
include sta1xxx-optionnal-providers.inc

MACHINE_EXTRA_RRECOMMENDS = " kernel-modules kernel-devicetree"

EXTRA_IMAGEDEPENDS += "m3-loaders"
EXTRA_IMAGEDEPENDS +=  "${@bb.utils.contains('MACHINE_FEATURES', 'atf', 'fip-image', 'u-boot', d)}"
EXTRA_IMAGEDEPENDS += "bc-native"

PREFERRED_PROVIDER_virtual/kernel ?= "linux-sta12xx"

#Use linux 4.9 libc headers
LINUXLIBCVERSION = "4.9"

KERNEL_IMAGETYPE ?= "fitImage"
KERNEL_CLASSES ?= " kernel-fitimage "
KERNEL_EXTRA_ARGS += "LOADADDR=${UBOOT_ENTRYPOINT}"

MACHINE_FEATURES_append = " usbgadget usbhost vfat alsa ext4"

IMAGE_BOOT_FILES ?= "u-boot.${UBOOT_SUFFIX}"

# Linux Test Project
PREFERRED_VERSION_ltp ?= "20160126"

# Flashloader config for all generated images
IMAGE_CLASSES += " image-flashloader-config"
IMAGE_CLASSES += " postprocess-image"

#Default size for initramfs (MB)
INITRAMFS_DEFAULT_SIZE = "10"
#Default size for bootfs (MB)
BOOTFS_DEFAULT_SIZE="16"

# ST CarProc baseline paths
#
ST_LOCAL_SRC ?= "${OEROOT}/sources/"
ST_LOCAL_SRC[vardepsexclude] = "OEROOT"
ST_LOCAL_TOOLS ?= "${OEROOT}/tools/"
ST_LOCAL_TOOLS[vardepsexclude] = "OEROOT"
ST_LOCAL_TESTS ?= "${OEROOT}/validation-tests/"
ST_LOCAL_TESTS[vardepsexclude] = "OEROOT"

#Use the GPLv2+ version of bash
PREFERRED_VERSION_bash = "3.2.57"

EULA_FILE_ST_sta = "${LAYER_PATH}/conf/eula/ST_EULA"

UBOOT_SIGN_KEYDIR = "${DEPLOY_DIR_IMAGE}/keys"
UBOOT_SIGN_KEYNAME = "dev"
UBOOT_SIGN_ENABLE = "1"

TOOLCHAIN_HOST_TASK_append = " nativesdk-u-boot-mkimage"

UBOOT_ENV_CONFIG[vardepsexclude] = "LAYER_PATH"
FLASHLOADER_CONFIG[vardepsexclude] = "LAYER_PATH"
SPLASH_ANIMATION_CONFIG[vardepsexclude] = "LAYER_PATH"
SPLASH_CONFIG[vardepsexclude] = "LAYER_PATH"
MEMORY_MAPPING_CONFIG[vardepsexclude] = "LAYER_PATH"
EULA_FILE_ST[vardepsexclude] = "LAYER_PATH"
