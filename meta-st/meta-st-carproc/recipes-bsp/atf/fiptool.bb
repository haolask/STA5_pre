SUMMARY = "Firmware Image Package image creation tool"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://license.rst;md5=e927e02bca647e14efd87e9e914b2443"

DEPENDS += "openssl"

inherit externalsrc
EXTERNALSRC_pn-${PN} ?= "${ST_LOCAL_SRC}/atf/"
#Change oe-workdir symlink as multiple recipes are pointing to the same source folder
EXTERNALSRC_SYMLINKS = "oe-workdir-fiptool:${WORKDIR} oe-logs-fiptool:${T}"

do_compile () {
	oe_runmake -C ${S}/tools/fiptool BUILD_BASE=${B}
}

do_install () {
	install -d ${D}${bindir}
	install -m 0755 ${B}/fiptool ${D}${bindir}/fiptool
}

BBCLASSEXTEND += "native nativesdk"
