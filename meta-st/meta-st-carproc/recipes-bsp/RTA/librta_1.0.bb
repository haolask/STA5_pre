DESCRIPTION = "Build the RTA library"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "r0"

inherit externalsrc
EXTERNALSRC_pn-librta  = "${ST_LOCAL_SRC}RTA"

DEPENDS += "st-etal"
DEPENDS += "pkgconfig"
RDEPENDS_${PN} += "bash"

SYSROOT_DIRS += "/rta_exports"
FILES_${PN}-staticdev = "/rta_exports/*"

do_compile() {
	make -C ${S} clean
	make -C ${S} CC="${CC}" ETAL_INCDIR="${STAGING_DIR_HOST}/etal_exports/" ETAL_LIBDIR="${STAGING_DIR_HOST}/etal_exports/" AR="${AR}" LDFLAGS="${LDFLAGS}"
}

do_install() {
	install -d ${D}/rta_exports
	install ${S}/librta.a ${D}/rta_exports
	install ${S}/include/*.h ${D}/rta_exports
}
