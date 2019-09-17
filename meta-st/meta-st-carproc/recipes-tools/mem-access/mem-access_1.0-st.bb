DESCRIPTION = "Mem access"
SECTION = "extras"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://COPYING;md5=6762ed442b3822387a51c92d928ead0d"

inherit externalsrc

EXTERNALSRC_pn-mem-access ?= "${ST_LOCAL_TOOLS}mem-access"

FILES_${PN} = "${bindir}/mem-access"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} ${S}/mem-access.c -o ${WORKDIR}/mem-access
}

do_install_append() {
	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/mem-access ${D}${bindir}
}
