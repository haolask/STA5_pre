SUMMARY = "Optee client"
DESCRIPTION = "TEE client library in Linux"
SECTION = "security"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"

SRC_URI = "git://github.com/OP-TEE/optee_client.git;protocol=https \
           file://0001-Do-not-set-Werror-by-default.patch \
           file://0002-libteec-refactor-_dprintf.patch \
           file://0003-tee-supplicant-rpmb.c-add-__attribute__-fallthrough.patch \
           "
SRCREV = "33fa3c1cc23c12c908baa09cb673373c5057b28c"

PV = "3.0.0"
PR = "git${SRCPV}.r0"

S = "${WORKDIR}/git"
B = "${S}/out"

DEPENDS = "optee-os"

SYSROOT_DIRS += " /optee_client"
FILES_${PN} = "/optee_client/*"
FILES_${PN}-staticdev = "/optee_client/export/lib/libteec.a"


FILES_${PN} += "${bindir}"
FILES_${PN} += "${base_libdir}"

# To package unversioned ".so" libraries
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

# Set PLATFORM and debug level
EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"
EXTRA_OEMAKE += "CFG_TEE_CLIENT_LOG_LEVEL=2"
EXTRA_OEMAKE += "CFG_TEE_SUPP_LOG_LEVEL=2"

do_compile() {
    oe_runmake -C ${S} O=${B}
}

do_install() {
    # Recopy optee-client TA devkit for external reference
    install -d ${D}/optee_client
    cp -r ${B}/export/ ${D}/optee_client/

    install -d ${D}/${bindir}
    install -m 544 ${B}/export/bin/tee-supplicant ${D}/${bindir}

    install -d ${D}${base_libdir}
    install -m 544 ${B}/export/lib/libteec.so.1.0 ${D}/${base_libdir}

    cd ${D}${base_libdir}
    if [ -e libteec.so ]; then
        rm libteec.so
    fi
    if [ -e libteec.so.1 ]; then
        rm libteec.so.1
    fi
    ln -s libteec.so.1.0 libteec.so.1
    ln -s libteec.so.1.0 libteec.so
    cd -
}

# Avoid QA Issue: No GNU_HASH in the elf binary
INSANE_SKIP_${PN} += "ldflags"

# Avoid QA issue: non -dev/-dbg/-nativesdk package contains symlink .so
INSANE_SKIP_${PN} += "dev-so"
