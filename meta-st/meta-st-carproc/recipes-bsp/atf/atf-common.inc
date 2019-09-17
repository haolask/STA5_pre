DESCRIPTION = "ARM Trusted Firmware"
AUTHOR = "ST ADG/MID"
SECTION = "bootloaders"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://license.rst;md5=e927e02bca647e14efd87e9e914b2443"

#m3-loaders is required to have the shared_data.h file
DEPENDS_prepend = "libxslt-native m3-loaders "

PACKAGE_ARCH = "${MACHINE_ARCH}"

PV = "1.0"

inherit deploy externalsrc

TOOLCHAIN_pn-atf = 'bare-metal'

EXTERNALSRC_pn-${PN} = "${ST_LOCAL_SRC}/atf/"

B = "${WORKDIR}/build"
SYSROOT_DIRS += " /boot"

FILES_${PN}-dev = "/boot/*"

#define the release mode to be used for ATF (0 or 1)
ATF_DEBUG ?= "1"

# Define default ATF namings
ATF_BASENAME ?= "atf"

ATF_ELF_SUFFIX = "elf"
ATF_BIN_SUFFIX = "bin"
ATF_FIP_IMAGE = "fip.bin"

BL2_NAME ?= "bl2"
BL2_ELF = "${BL2_NAME}/${BL2_NAME}.${ATF_ELF_SUFFIX}"
BL2_BIN = "${BL2_NAME}.${ATF_BIN_SUFFIX}"
BL2_IMAGE = "${ATF_BASENAME}/${BL2_NAME}"
BL2_FILE = "boot/${BL2_BIN}"

BL32_NAME ?= "bl32"
BL32_ELF = "${BL32_NAME}/${BL32_NAME}.${ATF_ELF_SUFFIX}"
BL32_BIN = "${BL32_NAME}.${ATF_BIN_SUFFIX}"
BL32_IMAGE = "${ATF_BASENAME}/${BL32_NAME}"

# "sp_min" or "optee"
BL32_SP ?= "sp_min"

BL32_FILE = "${@bb.utils.contains('BL32_SP', 'optee', 'boot/tee-header_v2.bin', 'boot/${BL32_BIN}', d)}"
BL32_EXTRA1_FILE = "${@bb.utils.contains('BL32_SP', 'optee', '${STAGING_DIR_TARGET}/boot/tee-pager_v2.bin', '', d)}"
BL32_EXTRA2_FILE = "${@bb.utils.contains('BL32_SP', 'optee', '${STAGING_DIR_TARGET}/boot/tee-pageable_v2.bin', '', d)}"
BL33_FILE = "${@bb.utils.contains('MACHINE_FEATURES', 'no_uboot', 'kernel_binaries/zImage', 'boot/u-boot.bin', d)}"
M3OS_FILE = "${@bb.utils.contains('MACHINE_FEATURES', 'telematic', '${STAGING_DIR_TARGET}/boot/m3_telematics.bin', '${STAGING_DIR_TARGET}/boot/m3_car_radio.bin', d)}"
M3OS_FILE2 = "${@bb.utils.contains('MACHINE_FEATURES', 'telematic', '${STAGING_DIR_TARGET}/boot/m3_telematics_file2.bin', '${STAGING_DIR_TARGET}/boot/m3_car_radio_file2.bin', d)}"

# Extra make settings
EXTRA_OEMAKE = 'CROSS_COMPILE=${HOST_PREFIX} '
EXTRA_OEMAKE += "PLAT_EXTRA_INCLUDES='-I${STAGING_INCDIR}/sta_mem_map'"
EXTRA_OEMAKE += 'TARGET_SOC_ID=${TARGET_SOC_ID} '
EXTRA_OEMAKE += 'PLAT=sta ARCH=aarch32'
EXTRA_OEMAKE += 'V=1 DEBUG=${ATF_DEBUG} LOG_LEVEL=10 AARCH32_SP=${BL32_SP} BUILD_BASE=${B} '
EXTRA_OEMAKE += 'MEMORY_MAP_XML=${MEMORY_MAPPING_CONFIG}'
EXTRA_OEMAKE += 'MEMORY_BOOT_DEVICE=${MEMORY_BOOT_DEVICE}'
EXTRA_OEMAKE += "${@bb.utils.contains('MACHINE_FEATURES', 'ChainOfTrust', 'TRUSTED_BOARD_BOOT=1 GENERATE_COT=1', '', d)}"
EXTRA_OEMAKE += "MBEDTLS_DIR=${ST_LOCAL_SRC}atf/plat/st/sta/mbedtls"
EXTRA_OEMAKE += 'STA_ROTPK_LOCATION=devel_rsa ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem'

# Only if feature no_uboot (linux as BL33)
EXTRA_OEMAKE += "${@bb.utils.contains('MACHINE_FEATURES', 'no_uboot', 'DTB=${STAGING_DIR_TARGET}/kernel_binaries/${KERNEL_DEVICETREE}', '', d)}"

DUMMY_MAKEFILE_PATH = '${B}dummy_makefile_path'

BUILD_MODE="${@bb.utils.contains('ATF_DEBUG', '1', 'debug', 'release', d)}"
BUILD_DIR="${B}/sta/${BUILD_MODE}"

do_configure() {
    unset LDFLAGS
    unset CFLAGS
    unset CPPFLAGS
    oe_runmake -C ${S} plat_config
}

remove_build() {
	#Internal atf dependencies are not very efficient, so erase the object before configure
	rm -rf ${B}
}

do_configure[prefuncs] += "remove_build"
