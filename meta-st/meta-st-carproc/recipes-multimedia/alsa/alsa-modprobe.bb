DESCRIPTION = "modprobe script for alsa"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

TP_SCRIPT_PATH = "${THISDIR}/${PN}/"

TP_SCRIPT = "alsa.conf"

SRC_URI = "file://${TP_SCRIPT_PATH}/${TP_SCRIPT}"
FILES_${PN} = "/etc/modprobe.d/${TP_SCRIPT}"

do_install() {
	install -d ${D}/etc/modprobe.d/
	install -m 0666 ${TP_SCRIPT_PATH}/${TP_SCRIPT} ${D}/etc/modprobe.d/${TP_SCRIPT}
}
