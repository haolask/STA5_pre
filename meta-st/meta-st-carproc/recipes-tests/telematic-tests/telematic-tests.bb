DESCRIPTION = "telematic tests scripts for A5"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

TELEMATIC_TESTS_PATH = "${ST_LOCAL_TESTS}telematic/telematic_rootfs"

SRC_URI = "file://${TELEMATIC_TESTS_PATH}/"
FILES_${PN} = "*"

do_install() {
	if [ $(ls -A ${TELEMATIC_TESTS_PATH}|wc -c) -ne 0 ]; then
		install -d ${D}
		cp -rf ${TELEMATIC_TESTS_PATH}/* ${D}
	fi
}

ALLOW_EMPTY_${PN}="1"
