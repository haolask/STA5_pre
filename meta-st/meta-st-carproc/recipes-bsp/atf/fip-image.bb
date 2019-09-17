SUMMARY = "ARM Trusted Firmware for STA EVB Boards"
LICENSE = "BSD-3-Clause"

DEPENDS_prepend = "fiptool-native libxslt-native u-boot m3-loaders atf certcreate-native "
DEPENDS_prepend = "${@bb.utils.contains('BL32_SP', 'optee', 'optee-os ', '', d)}"
DEPENDS_prepend = "virtual/kernel "

require recipes-bsp/atf/atf-common.inc
#Change oe-workdir symlink as multiple recipes are pointing to the same source folder
EXTERNALSRC_SYMLINKS = "oe-workdir-fipimage:${WORKDIR} oe-logs-fipimage:${T}"

EXTRA_DEPLOY_MAKE = "BL2=${STAGING_DIR_TARGET}/${BL2_FILE} ENABLE_BL2_BUILD='no' NEED_BL2=yes"
EXTRA_DEPLOY_MAKE += "BL32=${STAGING_DIR_TARGET}/${BL32_FILE} BL32_EXTRA1=${BL32_EXTRA1_FILE} BL32_EXTRA2=${BL32_EXTRA2_FILE}"
EXTRA_DEPLOY_MAKE += "BL33=${STAGING_DIR_TARGET}/${BL33_FILE} NEED_BL33=yes"
EXTRA_DEPLOY_MAKE += "${@bb.utils.contains('MACHINE_FEATURES', 'boot_m3os_from_m3xl', '', 'SCP_BL2=${M3OS_FILE}', d)}"
EXTRA_DEPLOY_MAKE += "${@bb.utils.contains('MACHINE_FEATURES', 'boot_m3os_from_m3xl', '', 'SCP_FILE2_BL2=${M3OS_FILE2}', d)}"
EXTRA_DEPLOY_MAKE += "FIPTOOLPATH=${DUMMY_MAKEFILE_PATH} FIPTOOL=${STAGING_BINDIR_NATIVE}/fiptool"
EXTRA_DEPLOY_MAKE += "CRTTOOLPATH=${DUMMY_MAKEFILE_PATH} CRTTOOL=${STAGING_BINDIR_NATIVE}/cert_create"
EXTRA_DEPLOY_MAKE += "LD_LIBRARY_PATH=${STAGING_LIBDIR_NATIVE}"

do_deploy() {
    install -d ${DEPLOYDIR}/${ATF_BASENAME}
    mkdir -p ${DUMMY_MAKEFILE_PATH}
    echo "all:" > ${DUMMY_MAKEFILE_PATH}/Makefile
    unset LDFLAGS
    unset CFLAGS
    unset CPPFLAGS
    oe_runmake -C ${S} ${EXTRA_DEPLOY_MAKE} fip
    install -m 644 ${BUILD_DIR}/${ATF_FIP_IMAGE} ${DEPLOYDIR}/${ATF_BASENAME}/${ATF_FIP_IMAGE}
}

addtask deploy before do_build after do_install

