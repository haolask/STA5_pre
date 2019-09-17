FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-Fix-warnings-when-building-with-gcc-4.8.patch"
SRC_URI += "file://0002-Fix-C-11-build-of-qtscript.patch"
SRC_URI += "file://0003-Fix-clang-C-11-build.patch"

