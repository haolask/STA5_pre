DESCRIPTION = "ST Init file for ramfs"
SECTION = "extras"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI = "file://init"
SRC_URI_sta1385-mtp += "file://init_sta1385"
SRC_URI_sta1385-carrier += "file://init_sta1385"

FILES_${PN} = "/init"
FILES_${PN} += "/dev/console"


do_install_append() {
	install -d ${D}
	install -m 0777 ${WORKDIR}/init* ${D}/init
	mkdir ${D}/dev
	mknod ${D}/dev/console c 5 1
}

do_clean_workdir () {
	rm -f ${WORKDIR}/init*
}

do_fetch[prefuncs] += "do_clean_workdir"
