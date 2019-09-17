DESCRIPTION = "ST start animation handler provided as example"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

inherit systemd
inherit externalsrc


FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"
EXTRA_FILE = "file://anim-handler.service"

SRC_URI += "${EXTRA_FILE}"

EXTERNALSRC_pn-anim-handler ?= "${ST_LOCAL_SRC}carproc-app/anim-handler"

SYSTEMD_PACKAGES += "anim-handler"
SYSTEMD_SERVICE_anim-handler = "anim-handler.service"
SYSTEMD_AUTO_ENABLE_anim-handler = "enable"


do_compile() {
    ${CC} ${EXTRA_INC_FLAGS} ${EXTRA_LD_FLAGS} ${CFLAGS} ${LDFLAGS} ${S}/anim_handler.c -o anim_handler
}

do_install() {
    install -Dm 0755 ${B}/anim_handler ${D}${bindir}/anim_handler
    install -Dm 0644 ${WORKDIR}/anim-handler.service ${D}${systemd_unitdir}/system/anim-handler.service
}

