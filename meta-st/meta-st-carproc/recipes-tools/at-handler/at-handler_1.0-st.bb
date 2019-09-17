DESCRIPTION = "AT handler"
SECTION = "extras"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

inherit externalsrc

EXTERNALSRC_pn-at-handler ?= "${ST_LOCAL_SRC}carproc-app/at-handler"

FILES_${PN} = "${bindir}/at-handler"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} ${S}/at-handler.c -o at-handler
}

do_install_append() {
	install -d ${D}${bindir}
	install -m 0755 ${B}/at-handler ${D}${bindir}
}
