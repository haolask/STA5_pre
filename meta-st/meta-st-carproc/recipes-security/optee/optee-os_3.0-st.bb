SUMMARY = "Optee OS"
DESCRIPTION = "TEE OS in Linux using the ARM TrustZone technology"
SECTION = "security"
LICENSE = "BSD"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"

DEPENDS += "mem-map-config m3-loaders"

inherit externalsrc deploy

EXTERNALSRC_pn-optee-os ?= "${ST_LOCAL_SRC}/optee_os"

SYSROOT_DIRS += " /boot /optee_os"
FILES_${PN}-staticdev += "/optee_os/export-ta_arm32/lib/*.a"
FILES_${PN}-dev = "/boot/* /optee_os/*"

# target optee platform, "sta-1385" by default
OPTEE_PLATFORM ?= "$(echo ${SOC_FAMILY} | cut -d":" -f1)"
OPTEE_FLAVOR ?= "$(echo ${SOC_FAMILY} | sed 's#[^0-9]*##')"

# CFG_WITH_PAGER need to configurable by machine, activated by default
OPTEE_OS_WITH_PAGER ?= "n"

EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"
EXTRA_OEMAKE += "ARCH=${TARGET_ARCH}"
EXTRA_OEMAKE += "PLATFORM=${OPTEE_PLATFORM}"
EXTRA_OEMAKE += "PLATFORM_FLAVOR=${OPTEE_FLAVOR}"
EXTRA_OEMAKE += "comp-cflagscore='--sysroot=${STAGING_DIR_TARGET}'"
EXTRA_OEMAKE += "CPPFLAGS='-I${STAGING_INCDIR}/sta_mem_map'"
EXTRA_OEMAKE += "O=${B}"
EXTRA_OEMAKE += "${@bb.utils.contains('OPTEE_OS_WITH_PAGER','y','CFG_WITH_PAGER=y','',d)}"
EXTRA_OEMAKE += "${@bb.utils.contains('OPTEE_OS_WITH_PAGER','y','CFG_TEE_CORE_DEBUG=n','',d)}"
EXTRA_OEMAKE += "${@bb.utils.contains('OPTEE_OS_WITH_PAGER','y','DEBUG=0','DEBUG=1',d)}"
EXTRA_OEMAKE += "CFG_TEE_CORE_LOG_LEVEL=2"

do_compile() {
    unset LDFLAGS
    oe_runmake -C ${S}
}

do_install() {
    # Install binary files ot create fip file for flashing
    rm -f ${D}/boot/tee-header_v2.bin
    rm -f ${D}/boot/tee-pager_v2.bin
    rm -f ${D}/boot/tee-pageable_v2.bin
    install -d ${D}/boot
    install -m644 ${B}/core/tee-header_v2.bin ${D}/boot
    install -m644 ${B}/core/tee-pager_v2.bin ${D}/boot
    install -m644 ${B}/core/tee-pageable_v2.bin ${D}/boot

    # Install optee-os TA devkit for external reference
    install -d ${D}/optee_os
    cp -r ${B}/export-ta_arm32/ ${D}/optee_os/
}

do_deploy() {
    # Gather all security files in atf folder
    install -d ${DEPLOYDIR}/atf

    # Recopy binary files for flashing
    install -m644 ${B}/core/tee-header_v2.bin ${DEPLOYDIR}/atf
    install -m644 ${B}/core/tee-pager_v2.bin ${DEPLOYDIR}/atf
    install -m644 ${B}/core/tee-pageable_v2.bin ${DEPLOYDIR}/atf

    # Recopy elf files for JTAG debugging
    install -m644 ${B}/core/tee.elf ${DEPLOYDIR}/atf
    install -m644 ${B}/core/tee.dmp ${DEPLOYDIR}/atf
    install -m644 ${B}/core/tee.map ${DEPLOYDIR}/atf
}

addtask deploy before do_build after do_compile

#we would like to have fip image generated if optee has been compiled
do_build[depends] += "fip-image:do_deploy"
