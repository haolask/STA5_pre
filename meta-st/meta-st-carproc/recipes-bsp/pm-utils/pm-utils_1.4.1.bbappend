
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "${@bb.utils.contains('MACHINE_FEATURES', 'multimedia', 'file://00stopanimation.sh', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE_FEATURES', 'multimedia', 'file://01rvc.sh', '', d)}"
SRC_URI += "file://02usb.sh"

FILES_${PN} += "${@bb.utils.contains('MACHINE_FEATURES', 'multimedia', '/etc/pm/sleep.d/00stopanimation.sh', '', d)}"
FILES_${PN} += "${@bb.utils.contains('MACHINE_FEATURES', 'multimedia', '/etc/pm/sleep.d/01rvc.sh', '', d)}"
FILES_${PN} += "/etc/pm/sleep.d/02usb.sh"
FILES_${PN} += "/etc/pm/power.d/disable_wol"

do_install_append() {
    install -d ${D}/etc/pm/sleep.d/
    install -m 0755 ${WORKDIR}/*.sh ${D}/etc/pm/sleep.d/
    install -m 0644 ${S}/pm/power.d/disable_wol ${D}/etc/pm/power.d/disable_wol
}
