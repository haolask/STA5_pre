# look for files in the layer first
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://dhcp-client-interfaces"

#Override default configuration
SYSTEMD_AUTO_ENABLE_${PN}-client = "enable"

FILES_${PN}-client += "/etc/default/dhcp-client-interfaces"

do_install_append() {
    install -d ${D}/etc/default/
    install -m 0644 ${WORKDIR}/dhcp-client-interfaces ${D}/etc/default/
}
