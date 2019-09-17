DESCRIPTION = "Rear View Camera Service for Accordo 5 (As example)"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

inherit systemd
inherit externalsrc
inherit pkgconfig

DEPENDS += "gstreamer1.0"
RDEPENDS_${PN} += "bash"

FILESEXTRAPATHS_append := ":${THISDIR}/${PN}_${PV}"
EXTRA_FILES = "file://rvc_service.sh \
               file://rvc.service    \
               file://novip.conf     \
               "

SRC_URI += "${EXTRA_FILES}"

EXTERNALSRC_pn-rvc ?= "${ST_LOCAL_SRC}carproc-app/rvc"

SYSTEMD_PACKAGES += "rvc"
SYSTEMD_SERVICE_rvc = "rvc.service"
SYSTEMD_AUTO_ENABLE_rvc = "enable"

EXTRA_INC_FLAGS = "`pkg-config --cflags  gstreamer-1.0`"
EXTRA_LD_FLAGS = "`pkg-config --libs gstreamer-1.0`"

do_compile() {
    ${CC} ${EXTRA_INC_FLAGS} ${EXTRA_LD_FLAGS} ${CFLAGS} ${LDFLAGS} ${S}/rvc_handler.c -o rvc_handler
}

do_install() {
    install -Dm 0755 ${B}/rvc_handler ${D}${bindir}/rvc_handler
    install -Dm 0755 ${WORKDIR}/rvc_service.sh ${D}${bindir}/rvc_service.sh
    install -Dm 0644 ${WORKDIR}/novip.conf ${D}${sysconfdir}/modprobe.d/novip.conf
    install -Dm 0644 ${WORKDIR}/rvc.service ${D}${systemd_unitdir}/system/rvc.service
}


