PACKAGECONFIG = "udev bat"
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-Disable-systemd-services-start-Done-by-udev.patch"
SRC_URI_append_sta1385-mtp = " file://0002-alsa-utils-aplay-support-for-mtp-mic-capture.patch"

