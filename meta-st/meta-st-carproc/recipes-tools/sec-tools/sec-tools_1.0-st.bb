DESCRIPTION = "Security Tools"
SECTION = "extras"
LICENSE = "BSD-3-Clause"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9"

inherit externalsrc

DEPENDS = "openssl"
RDEPENDS_${PN} += "libcrypto"

EXTERNALSRC_pn-sec-tools ?= "${ST_LOCAL_TOOLS}sec-tools"

FILES_${PN} = "${bindir}/aes_gcm"
FILES_${PN} += "${bindir}/gen_pattern_files"

EXTRA_CFLAGS = "-Wall -Wextra -std=c99"
EXTRA_LDFLAGS = "-lcrypto"

do_compile() {
	${CC} ${CFLAGS} ${EXTRA_CFLAGS} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/aes_gcm.c -o aes_gcm
	${CC} ${CFLAGS} ${EXTRA_CFLAGS} ${LDFLAGS} ${S}/inc.c -o gen_pattern_files
}

do_install_append() {
	install -d ${D}${bindir}
	install -m 0755 ${B}/aes_gcm ${D}${bindir}
	install -m 0755 ${B}/gen_pattern_files ${D}${bindir}
}
