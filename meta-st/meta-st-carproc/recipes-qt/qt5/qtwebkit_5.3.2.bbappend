FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-Fix-issue-building-jsobject-with-gcc-5.0.patch"

do_install_append() {
    sed -i 's@-Wl,-no-whole-archive -L${B}[^ ]* @ @g' ${D}${libdir}/pkgconfig/Qt5WebKit.pc
}
