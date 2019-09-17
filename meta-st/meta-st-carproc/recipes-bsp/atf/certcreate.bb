SUMMARY = "Certificate generation tool buidling"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://license.rst;md5=e927e02bca647e14efd87e9e914b2443"

DEPENDS += "openssl-native"

inherit externalsrc
EXTERNALSRC_pn-${PN} ?= "${ST_LOCAL_SRC}/atf/"
#Change oe-workdir symlink as multiple recipes are pointing to the same source folder
EXTERNALSRC_SYMLINKS = "oe-workdir-certcreate:${WORKDIR} oe-logs-certcreate:${T}"

# Extra make settings
EXTRA_OEMAKE += 'PLAT=sta ARCH=aarch32'
EXTRA_OEMAKE += 'USE_TBBR_DEFS=1'
EXTRA_OEMAKE += 'OPENSSL_DIR=${STAGING_EXECPREFIXDIR}'
EXTRA_OEMAKE += 'DEFINES="${BUILD_CFLAGS} ${BUILD_LDFLAGS} -DUSE_TBBR_DEFS=1"'
EXTRA_OEMAKE += 'MEMORY_MAP_XML=${MEMORY_MAPPING_CONFIG}'


do_compile () {
	rm -fR ${B}
	oe_runmake -C ${S}/tools/cert_create BUILD_BASE=${B}
}

do_install () {
	install -d ${D}${bindir}
	install -m 0755 ${B}/cert_create ${D}${bindir}/cert_create
}

BBCLASSEXTEND += "native nativesdk"
