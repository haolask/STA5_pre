SUMMARY = "Gstreamer plugin provided by RidgeRun to decrypt videos"
DESCRITION = "gst-crypto is a GStreamer plugin that ecrypts/decrypts data streams on the fly, static or streaming. It is build directly on top of OpenSSL."
SECTION = "Digecor gstreamer"

LICENSE = "GPLv2+ & LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=cb8aedd3bced19bd8026d96a8b6876d7"

PV = "1.0"

SRC_URI = "https://github.com/RidgeRun/gst-crypto/archive/v${PV}.tar.gz"

SRC_URI[md5sum] = "d35c2a1a1c3f265389ae2880706f1396"
SRC_URI[sha256sum] = "1275db33d36768cad142a46289a8f4de1f7727bc7dc43252bb2881f89fb1f919"

S = "${WORKDIR}/gst-crypto-${PV}"

inherit autotools
inherit pkgconfig

DEPENDS = "gstreamer1.0 gstreamer1.0-plugins-base"

FILES_${PN} = "${libdir}/gstreamer-1.0/libgstcrypto.so"
FILES_${PN}-dbg += "${libdir}/gstreamer-1.0/.debug"
FILES_${PN}-dev += "${libdir}/gstreamer-1.0/*.la"
