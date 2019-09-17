DESCRIPTION = "Radio Tuner Application Control"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "r0"
RTA_SOURCE_PATH = "${S}"

inherit pkgconfig
inherit externalsrc
EXTERNALSRC_pn-rta-test  = "${ST_LOCAL_SRC}RTA"

#Add package depends required for compilation
# pkgconfig & bash for correct execution of command lines
DEPENDS += "pkgconfig"
DEPENDS += "st-etal"
DEPENDS += "librta"
RDEPENDS_${PN} += "bash"

EXTRA_LD_FLAGS = ""
EXTRA_INC_FLAGS +=" -I${S}/include"
EXTRA_LIB_FLAGS +=" -L${STAGING_BASELIBDIR}"

DEPENDS += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', 'alsa-lib', '', d)}"

DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-DHAVE_DAB_SD', '', d)}"
CC += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"

do_compile() {
	make -C ${RTA_SOURCE_PATH}/test/linux  clean
	make -C ${RTA_SOURCE_PATH}/test/linux  LD="${CC}" LDFLAGS="${LDFLAGS}" EXTRA_LD_FLAGS="${EXTRA_LD_FLAGS}" ETAL_INCDIR="${STAGING_DIR_HOST}/etal_exports/" ETAL_LIBDIR="${STAGING_DIR_HOST}/etal_exports/" RTA_LIBDIR="${STAGING_DIR_HOST}/rta_exports/" DEFINES="${DEFINES}"
}

do_install() {
	install -d ${D}${bindir}
	install -d ${D}${bindir}/RTA
	install -m 0755 ${S}/test/linux/rta-testApp ${D}${bindir}/RTA
}
