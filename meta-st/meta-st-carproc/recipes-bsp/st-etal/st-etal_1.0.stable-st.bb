DESCRIPTION = "ST-ETAL - ST Tuner"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "r0"
ETAL_SOURCE_PATH = "${S}"

inherit externalsrc
EXTERNALSRC_pn-st-etal = "${ST_LOCAL_SRC}st-etal/stable-etal"

SYSROOT_DIRS += "/etal_exports"
FILES_${PN}-staticdev = "/etal_exports/*"

include st-etal.inc

do_install_append() {
	install -d ${D}/etal_exports
	install ${S}/etalcore/exports/etal.a ${D}/etal_exports
	install ${S}/etalcore/exports/etal.a ${D}/etal_exports/libetal.a
	install ${S}/etalcore/exports/*.h ${D}/etal_exports
	install ${S}/target/linux/target_config.h ${D}/etal_exports
	install ${S}/target/linux/target_config.mak ${D}/etal_exports
	if [ "${ETAL_OPTION}" = "DAB_HOST_SD" ] ; then
		install ${S}/target/linux/lib/libStreamDec/libciDec.a ${D}/etal_exports
	fi
}
