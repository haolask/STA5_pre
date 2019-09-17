SUMMARY = "Cortex-M3 Xloaders/Application for STA1XXX SOCs"
DESCRIPTION = " \
		m3-loaders are the primary-level bootloader for STA1XXX SoCs.\
		It provides a set of functions to manage the hardware \
		initialization and to download the Cortex-A7 U-boot/A7 bootloader  \
		from different boot media and start it."

AUTHOR = "ST APG/MID"
SECTION = "bootloaders"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=6426001521b8d9c99f18daa6d9ad3d41"

DEPENDS_prepend = "cmake-native doxygen-native mem-map-config "

inherit deploy

TOOLCHAIN_pn-m3-loaders = 'bare-metal'

# We need to unset CCACHE otherwise cmake gets too confused
CCACHE = ""

inherit externalsrc

EXTERNALSRC_pn-m3-loaders = "${ST_LOCAL_SRC}/m3-loaders/"

B = "${WORKDIR}/build"

SYSROOT_DIRS += " /boot"
FILES_${PN}-dev = "/boot/* ${includedir}/*"


ATF_OPT = "${@bb.utils.contains('MACHINE_FEATURES', 'atf', '-DATF=1', '-DATF=0', d)}"
COT_OPT = "${@bb.utils.contains('MACHINE_FEATURES', 'ChainOfTrust', '-DCOT=1', '-DCOT=0', d)}"
BOOT_OPT = "${@bb.utils.contains('MACHINE_FEATURES', 'boot_m3os_from_m3xl', '-DBOOT_M3OS_FROM_M3XL=1', '-DBOOT_M3OS_FROM_M3XL=0', d)}"
EARLY_TUNER_OPT = "${@bb.utils.contains('EARLY_TUNER_FTR', '1', '-DEARLY_TUNER_FTR=1', '', d)}"

LPDDR2_OPT = "${@bb.utils.contains('MACHINE_FEATURES', 'lpddr2', '-DLPDDR2=1', '', d)}"
DDR_CFG ?= "CFG_DDR_ALT0"

OPTS_CMAKE = "-DSOC_ID=${TARGET_SOC_ID} -DBOARD_ID=${TARGET_BOARD_ID} -DBOOT_DEVTYPE=${TARGET_BOOT_DEVICE} ${ATF_OPT} ${COT_OPT} ${BOOT_OPT} ${LPDDR2_OPT} -DDDR_CFG=${DDR_CFG} ${EARLY_TUNER_OPT}"

OPTS_CMAKE += "-DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-gcc-arm-none-eabi.cmake -DSTA_MEM_MAP_DIR=${STAGING_INCDIR}/sta_mem_map"

do_configure () {
    unset LDFLAGS
    rm -f ${S}/src/*.ld
    rm -rf ${B}
    mkdir -p ${B}
    cd ${B}
    cmake ${OPTS_CMAKE} ${S}
}

do_compile () {
    unset LDFLAGS
    cd ${B}
    base_do_compile
}

do_install () {
    install -d ${D}/boot
    if [ -e "${B}/src/m3_car_radio.bin" ]; then
        install -m 644 ${B}/src/m3_car_radio*.bin ${D}/boot
    fi
    if [ -e "${B}/src/m3_telematics.bin" ]; then
        install -m 644 ${B}/src/m3_telematics*.bin ${D}/boot
    fi
    mkdir -p ${D}/${includedir}/sta_mem_map
    install -m 0644 ${S}/include/shared_data.h ${D}/${includedir}/sta_mem_map/
}

do_deploy () {
    install -d ${DEPLOYDIR}

    install -m644 ${B}/src/m3_xl_uflashloader.bin ${DEPLOYDIR}
    install -m644 ${B}/src/m3_xloader.bin ${DEPLOYDIR}
    install -m644 ${B}/src/m3_mini.bin ${DEPLOYDIR}
    # Recopy elf files for JTAG debugging
    install ${B}/src/m3_xl_uflashloader ${DEPLOYDIR}
    install ${B}/src/m3_xloader ${DEPLOYDIR}
    install ${B}/src/m3_mini ${DEPLOYDIR}

    mkdir -p ${DEPLOYDIR}/sta_mem_map
    install -m 0644 ${S}/include/shared_data.h ${DEPLOYDIR}/sta_mem_map

    if [ -e "${B}/src/m3_car_radio.bin" ]; then
        install -m644 ${B}/src/m3_car_radio*.bin ${DEPLOYDIR}
        install ${B}/src/m3_car_radio ${DEPLOYDIR}
    fi
    if [ -e "${B}/src/m3_telematics.bin" ]; then
        install -m644 ${B}/src/m3_telematics*.bin ${DEPLOYDIR}
        install ${B}/src/m3_telematics ${DEPLOYDIR}
    fi

    #Install early audio files in deploy folder
    if [ ! -z "${ST_EARLY_AUDIO_SUBPATH}" ]; then
        for f in ${ST_EARLY_AUDIO_SUBPATH}/*; do
            echo "Installing $f file"
            install -m 0666 ${S}/$f ${DEPLOYDIR}
        done
    fi
}

addtask deploy before do_build after do_install

#we would like to have fip image generated if m3-loaders has been compiled
python __anonymous() {
    if bb.utils.contains('MACHINE_FEATURES', 'atf', 'ATF', None, d) is not None:
        d.appendVarFlag('do_build', 'depends', ' fip-image:do_deploy')
}

# Warning: normally we have to define a new architecture for Cortex-M3
# and not use the default Cortex-A7 ARCH
PACKAGE_ARCH = "${MACHINE_ARCH}"
