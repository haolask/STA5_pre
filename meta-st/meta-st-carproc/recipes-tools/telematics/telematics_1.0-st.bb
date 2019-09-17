DESCRIPTION = "ST Telematics application"
SECTION = "extras"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

TELEMATICS_INSTALL = "/opt/demo/telematics"
TELEMATICS_BASE_FLDR = "${S}/rootfs"
TELEMATICS_WWW = "www"
TELEMATICS_WWW_sta1385 = "www-1385"
inherit externalsrc systemd

#DEPENDS = "gnuplot"

EXTERNALSRC_pn-telematics ?= "${ST_LOCAL_SRC}carproc-app/telematics"

FILES_${PN} = "${bindir}/telematics"
FILES_${PN} += "${bindir}/socket_redirect.bin"
FILES_${PN} += "${bindir}/start_telematics.sh"
FILES_${PN} += "${TELEMATICS_INSTALL}"
FILES_${PN} += "/var/www"
FILES_${PN} += "/lib"
FILES_${PN} += "${sysconfdir}/udhcpd.conf"
FILES_${PN} += "${sysconfdir}/hostapd.mrvl.conf"
FILES_${PN} += "${sysconfdir}/me909-datacall"
FILES_${PN} += "${sysconfdir}/syslog.conf"

SYSTEMD_PACKAGES = "${@bb.utils.contains('DISTRO_FEATURES','systemd','${PN}','',d)}"

#Enable systemd service to start
SYSTEMD_SERVICE_${PN} = "${@bb.utils.contains('DISTRO_FEATURES','systemd','telematics.service','',d)}"
SYSTEMD_SERVICE_${PN} += " ${@bb.utils.contains('DISTRO_FEATURES','systemd','startchime.service','',d)}"
SYSTEMD_SERVICE_${PN} += " ${@bb.utils.contains('DISTRO_FEATURES','systemd','wifi-hotspot.service','',d)}"

EXTRA_CFLAGS = "-Os -Wall -DDEBUG"
EXTRA_LDFLAGS = " -lpthread "

do_compile() {
	${CC} ${EXTRA_CFLAGS} ${EXTRA_LDFLAGS} ${CFLAGS} ${LDFLAGS} ${S}/telematics.c -o telematics
	${CC} ${EXTRA_CFLAGS} ${EXTRA_LDFLAGS} ${CFLAGS} ${LDFLAGS} ${S}/socket_redirect.c -o socket_redirect.bin
}

do_install() {
	install -d ${D}${bindir}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/start_telematics.sh ${D}${bindir}
	install -m 0777 ${B}/telematics ${D}${bindir}
	install -m 0777 ${B}/socket_redirect.bin ${D}${bindir}

	install -d ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/kill_audio.sh ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/papcall-le910 ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/papcall-he910 ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/papcall-me909u ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/papcall-ue910 ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/setup.sh ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/sta1195html.sh ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/timetofix.sh ${D}${TELEMATICS_INSTALL}
	install -m 0755 ${TELEMATICS_BASE_FLDR}/telematics.ini ${D}${TELEMATICS_INSTALL}
	install -d ${D}${TELEMATICS_INSTALL}/audio
	install -m 0666 ${TELEMATICS_BASE_FLDR}/audio/* ${D}${TELEMATICS_INSTALL}/audio
	install -m 0777 ${TELEMATICS_BASE_FLDR}/startchime.sh ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/alsastart.up ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/modem_audio.sh ${D}${TELEMATICS_INSTALL}
# Wifi Hotspot installation
	install -m 0777 ${TELEMATICS_BASE_FLDR}/hotspot.sh ${D}${TELEMATICS_INSTALL}
#	install -m 0777 ${TELEMATICS_BASE_FLDR}/monitor_demo.sh ${D}${TELEMATICS_INSTALL}
	install -m 0777 ${TELEMATICS_BASE_FLDR}/modem_pinctrl.sh ${D}${TELEMATICS_INSTALL}
	install -d ${D}${sysconfdir}
	install -m 0755 ${TELEMATICS_BASE_FLDR}/udhcpd.conf ${D}${sysconfdir}
	install -m 0755 ${TELEMATICS_BASE_FLDR}/hostapd.mrvl.conf ${D}${sysconfdir}
	install -m 0755 ${TELEMATICS_BASE_FLDR}/me909-datacall ${D}${sysconfdir}
#Disable for now, require gnuplot, which requires X11 !
#	install -m 0777 ${TELEMATICS_BASE_FLDR}/plot_accel ${D}${TELEMATICS_INSTALL}
	install -d ${D}/var/www
	install -m 0666 ${TELEMATICS_BASE_FLDR}/${TELEMATICS_WWW}/* ${D}/var/www
#Install systemd service
	if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
		install -d ${D}${systemd_unitdir}/system
		install -m 644 ${TELEMATICS_BASE_FLDR}/*.service ${D}/${systemd_unitdir}/system
	fi
}
