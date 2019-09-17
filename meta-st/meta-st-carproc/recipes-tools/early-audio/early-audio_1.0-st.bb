DESCRIPTION = "Early Audio"
SECTION = "extras"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

inherit externalsrc

EXTERNALSRC_pn-early-audio ?= "${ST_LOCAL_SRC}carproc-app/early_audio"

FILES_${PN} = "${bindir}/early_make"
FILES_${PN} += "${bindir}/early_make.sh"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} ${S}/early_make.c -o early_make
}

do_install_append() {
	install -d ${D}${bindir}
	install -m 0755 ${B}/early_make ${D}${bindir}
	install -m 0755 ${S}/early_make.sh ${D}${bindir}
}
