DESCRIPTION = "Audio tests scripts for A5"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

AUDIO_TESTS_PATH = "${ST_LOCAL_TESTS}audio/audio_rootfs"

SRC_URI = "file://${AUDIO_TESTS_PATH}/"
FILES_${PN} = "*"

do_install() {
	if [ $(ls -A ${AUDIO_TESTS_PATH}|wc -c) -ne 0 ]; then
		install -d ${D}
		cp -rf ${AUDIO_TESTS_PATH}/* ${D}
	fi
}

ALLOW_EMPTY_${PN}="1"

#Always copy the file, do not use cache.
