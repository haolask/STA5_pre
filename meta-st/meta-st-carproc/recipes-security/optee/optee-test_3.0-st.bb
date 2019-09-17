SUMMARY = "Optee test"
DESCRIPTION = "TEE sanity testsuite in Linux"
SECTION = "security"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"

SRC_URI = "git://github.com/OP-TEE/optee_test.git;protocol=https \
           file://0001-regression-4006-Add-nist-186-3-ecdsa-test-vectors.patch \
           file://0002-regression-4000-Add-4012-test-case.patch \
           file://0001-regression-1000-Add-test-case-1020.patch \
           file://0002-regression-4009-Disable-ECC-curve-P192-and-P224-if-n.patch \
           file://0001-regression-1020-Add-new-HSM-services-tests.patch \
           file://0001-regression-1020-Add-HSM-PTA-load-export-key-and-scal.patch \
           file://0002-regression-4000-Disable-ECC-curve-P521-if-not-ECC_FR.patch \
           file://0001-IOTMBL-438-Fix-optee_test-adbg_run.c-strncpy-Werror-.patch \
           file://0001-IOTMBL-438-Fix-optee_test-error-memmove-Werror-array.patch \
           file://0001-regression-4011-fix-test-failure-with-GCC-8.1-memmov.patch \
           "
SRCREV = "692efd172e46768d31700b8e4add274454ce8273"

PV = "3.0.0"
PR = "git${SRCPV}.r0"

S = "${WORKDIR}/git"

DEPENDS = "optee-os optee-client"

FILES_${PN} += "${bindir}"
FILES_${PN} += "${base_libdir}/optee_armtz"

# optee-test expects specific toolchains for client Appli and Trusted Appli
EXTRA_OEMAKE = "CROSS_COMPILE_HOST=${TARGET_PREFIX}"
EXTRA_OEMAKE += "CROSS_COMPILE_TA=${TARGET_PREFIX}"

# refer to optee-os for TA devkit installation path
EXTRA_OEMAKE += "TA_DEV_KIT_DIR=${STAGING_DIR_TARGET}/optee_os/export-ta_arm32"

# refer to optee-client for TA devkit installation path
EXTRA_OEMAKE += "OPTEE_CLIENT_EXPORT=${STAGING_DIR_TARGET}/optee_client/export"

do_compile() {
    oe_runmake -C ${S} O=${B}
}

do_install() {
    install -d ${D}/${bindir}
    install -m 544 ${B}/xtest/xtest ${D}/${bindir}

    install -d ${D}${base_libdir}/optee_armtz
    for f in `find ${B}/ta -name \*.ta`; do
        install -m 444 "$f" ${D}${base_libdir}/optee_armtz
    done
}

