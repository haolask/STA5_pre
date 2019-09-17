SUMMARY = "Optee examples"
DESCRIPTION = "TEE TAs and CAs examples in Linux"
SECTION = "security"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"

FILESEXTRAPATH_prepend := "${THISDIR}/${PN}:"

SRC_URI = "git://github.com/linaro-swg/optee_examples.git;protocol=https"
SRC_URI += "file://0001-Makefile-Add-support-to-build-in-external-folder.patch"
SRCREV = "029d583642ed62b833952ccb7125738f06918c8e"

PV = "3.0.0"
PR = "git${SRCPV}.r0"

S = "${WORKDIR}/git"

DEPENDS = "optee-os optee-client"
RDEPENDS_${PN} = "optee-client"

FILES_${PN} = "${bindir}*"
FILES_${PN} += "${base_libdir}/optee_armtz/*"

OPTEE_CLIENT_EXPORT = "${STAGING_DIR_TARGET}/optee_client/export"
TEEC_EXPORT         = "${STAGING_DIR_TARGET}/optee_client/export"
TA_DEV_KIT_DIR      = "${STAGING_DIR_TARGET}/optee_os/export-ta_arm32"
OUT                 = "${WORKDIR}/out"

EXTRA_OEMAKE = " TA_DEV_KIT_DIR=${TA_DEV_KIT_DIR} \
                 OPTEE_CLIENT_EXPORT=${OPTEE_CLIENT_EXPORT} \
                 TEEC_EXPORT=${TEEC_EXPORT} \
                 HOST_CROSS_COMPILE=${HOST_PREFIX} \
                 TA_CROSS_COMPILE=${HOST_PREFIX} \
                 V=1 \
                 OUTPUT_DIR=${OUT} \
               "

CFLAGS += "-I${STAGING_DIR_TARGET}/optee_client/export/include/"
LDFLAGS = ""

do_compile() {
    rm -rf ${OUT}
    mkdir -p ${OUT}
    oe_runmake -C ${S}
}

do_install() {
    install -d ${D}/${bindir}
    for f in $(ls ${OUT}/ca); do
        install -m 777 "${OUT}/ca/$f" ${D}${bindir}
    done
    install -d ${D}${base_libdir}/optee_armtz
    for f in $(ls ${OUT}/ta); do
        install -m 444 "${OUT}/ta/$f" ${D}${base_libdir}/optee_armtz
    done
}
