SUMMARY = "ARM Trusted Firmware for STA EVB Boards"
LICENSE = "BSD-3-Clause"

require recipes-bsp/atf/atf-common.inc
#Change oe-workdir symlink as multiple recipes are pointing to the same source folder
EXTERNALSRC_SYMLINKS = "oe-workdir-atf:${WORKDIR} oe-logs-atf:${T}"

do_compile() {
    unset LDFLAGS
    unset CFLAGS
    unset CPPFLAGS
    # Compile ATF & Link all bin file to generate atf.bin
    oe_runmake -C ${S}
}

do_install() {
    if [ -f ${BUILD_DIR}/${BL32_BIN} ]; then
        install -d $(dirname ${D}/${BL32_FILE})
        install -m 644 ${BUILD_DIR}/${BL32_BIN} ${D}/${BL32_FILE}
    fi

    if [ -f ${BUILD_DIR}/${BL2_BIN} ]; then
        install -d $(dirname ${D}/${BL2_FILE})
        install -m 644 ${BUILD_DIR}/${BL2_BIN} ${D}/${BL2_FILE}
    fi
}

do_deploy () {
    #Install the elf files in deploy to ease debug
    deployed_files="${BL2_ELF} ${BL32_ELF} ${BL2_BIN} ${BL32_BIN}"
    echo "File list to install: ${deployed_files}"
    for bin in $deployed_files; do
        echo "Installing $bin (${BUILD_DIR}) in ${DEPLOYDIR}/atf"
        mkdir -p ${DEPLOYDIR}/atf
        if [ -f ${BUILD_DIR}/${bin} ]; then
            install -m 644 ${BUILD_DIR}/${bin} ${DEPLOYDIR}/atf
        fi
    done
}

addtask deploy before do_build after do_install

#we would like to have fip image generated if ATF has been compiled
do_build[depends] += "fip-image:do_deploy"
