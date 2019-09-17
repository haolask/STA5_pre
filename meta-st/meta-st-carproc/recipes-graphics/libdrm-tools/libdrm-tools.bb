SUMMARY = "Display tools using the libdrm interface"
LICENSE = "MIT & GPLv2"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=0835ade698e0bcf8506ecda2f7b4f302 \
                    file://COPYING.GPL-2.0;md5=751419260aa954499f7abaabaa882bbe"

inherit autotools pkgconfig

SRC_URI = "git://gerrit.st.com:29418/oeivi/oe/st/st-tools;protocol=ssh;branch=master"
SRCREV = "7d0a790bcd21e470b3ad9707cc301fd1a334e73e"

PV = "1.0"
PR = "git${SRCPV}.r0"

S = "${WORKDIR}/git/libdrm-tools"

DEPENDS += "libdrm udev"

RDEPENDS_${PN} = " \
       libudev \
       cairo \
       libdrm-kms \
"

do_install () {
    install -d ${D}${bindir}
    install -m 0755 ${B}/drmut/drmut ${D}${bindir}
    install -m 0755 ${B}/drmut_sta/drmut_sta ${D}${bindir}
    install -m 0755 ${B}/kmsdemo/kmsdemo ${D}${bindir}
    install -m 0755 ${B}/streamtest/streamtest ${D}${bindir}
}
