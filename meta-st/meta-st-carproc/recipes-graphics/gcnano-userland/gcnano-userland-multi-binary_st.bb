# Recipe for installing gcnano-userland binaries (multi backends such as fbdev, gbm and wayland)
SUMMARY = "Vivante libraries OpenGL ES, OpenVG and EGL (multi backends such as fbdev, gbm and wayland)"
DESCRIPTION  = "STMicrolectronics port of the EGL, GLESv1_CM, GLES_v2 waylandegl libraries from Vivante for the gcnano 3D core."

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://${SEXTRACT}/LICENSE;md5=dd36864f287701862a189a69fc50f1d8"

inherit externalsrc

DEPENDS += " libdrm wayland "
RDEPENDS_${PN} += " gcnano-driver "

BACKEND="multi"

PV_GCNANO="6.2.4"
PR_GCNANO="p4"

GCNANO_USERLAND_FB_TARBALL_DATE= "20190205"

PACKAGE_ARCH = "${MACHINE_ARCH}"

PROVIDES += "gcnano-userland virtual/libgles1 virtual/libgles2 virtual/egl virtual/libvg virtual/gbm virtual/mesa"

PV="${PV_GCNANO}"
PR="${PR_GCNANO}-binary"

GCNANO_DV="${PV_GCNANO}.${PR_GCNANO}"

TAR_FILENAME="gcnano-userland-${BACKEND}-${GCNANO_DV}-${GCNANO_USERLAND_FB_TARBALL_DATE}"

#SRC_URI = "file://${ST_LOCAL_SRC}/gcnano-binaries/${TAR_FILENAME}.tar.xz"
EXTERNALSRC_pn-gcnano-userland-multi-binary = "${ST_LOCAL_SRC}/gcnano-binaries"


SEXTRACT = "${WORKDIR}/${TAR_FILENAME}"

# Action stubbed
do_configure[noexec] = "1"
do_compile[noexec] = "1"

#------------------------------------------
# Do install
#
do_install() {
    install -m 755 -d ${D}/usr/
    cp -R ${SEXTRACT}/usr ${D}/
}

addtask do_unpack_libs before do_populate_lic before do_install after do_compile

do_unpack_libs () {
	tar xf "${S}/${TAR_FILENAME}.tar.xz" -C ${WORKDIR}
}


# Cannot split or strip last added firmwares
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"

# Avoid QA Issue: No GNU_HASH in the elf binary
INSANE_SKIP_${PN} += "ldflags"
# Avoid QA Issue: non -dev/-dbg/nativesdk- package contains symlink .so
INSANE_SKIP_${PN} += "dev-so"

# Monolitic configuration
RPROVIDES_${PN}  = "libegl libegl1 libgles1 libglesv1-cm1 libgles2 libglesv2-2 libvg libgbm gbm_viv"
RREPLACES_${PN}  = "libegl libegl1 libgles1 libglesv1-cm1 libgles2 libglesv2-2 libvg libgbm gbm_viv"
RCONFLICTS_${PN} = "libegl libegl1 libgles1 libglesv1-cm1 libgles2 libglesv2-2 libvg libgbm gbm_viv"

PACKAGES = "${PN} ${PN}-dev "

SUMMARY_${PN}       = "${SUMMARY}"
FILES_${PN}         = "${libdir}/libEGL*.so*"
FILES_${PN}        += "${libdir}/libGLESv1_CM*.so"
FILES_${PN}        += "${libdir}/libGLESv2*.so* "
FILES_${PN}        += "${libdir}/libOpenVG*.so"
FILES_${PN}        += "${libdir}/libGAL*.so"
FILES_${PN}        += "${libdir}/libVSC*.so ${libdir}/libGLSLC*.so"
FILES_${PN}        += "${libdir}/libVDK*.so"
FILES_${PN}        += "${libdir}/libgbm*.so*"
FILES_${PN}        += "${libdir}/libwayland-viv*.so*"
FILES_${PN}        += "${libdir}/libwayland-egl*.so*"
FILES_${PN}        += "${libdir}/gbm_viv*.so*"


SUMMARY_${PN}-dev  = "${SUMMARY_${PN}} - Development files"
FILES_${PN}-dev    = "${includedir} ${libdir}/pkgconfig/"
