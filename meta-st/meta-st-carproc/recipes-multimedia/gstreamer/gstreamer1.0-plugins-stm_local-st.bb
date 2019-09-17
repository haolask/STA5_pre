DEFAULT_PREFERENCE = "-1"

include gstreamer1.0-plugins-stm.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=6762ed442b3822387a51c92d928ead0d"


DEPENDS += " libdrm"

inherit externalsrc

#for local source
EXTERNALSRC_pn-gstreamer1.0-plugins-stm ?= "${ST_LOCAL_SRC}gst-plugins-st"

FILES_${PN} += " ${libdir}/gstreamer-1.0/*.so ${bindir}/gst-interactive-1.0"

CFLAGS_append = " -D__EXPORTED_HEADERS__ -DUSE_V4L2_MEMORY_DMABUF -I${STAGING_KERNEL_DIR}/include/uapi -I${STAGING_KERNEL_DIR}/include"

EXTRA_OECONF += " \
    --disable-fb \
"

#temporary for gstreamer 1.6
EXTRA_OECONF += " \
    --disable-egl \
"

inherit autotools pkgconfig gettext

do_configure_prepend() {
    ${S}/autogen.sh --noconfigure
}

do_install_append() {
    cp -f ${S}/tools/scripts/avtranscode.sh ${D}/${bindir}/avtranscode
    cp -f ${S}/tools/scripts/*.py ${D}/${bindir}/
}

FILES_${PN} += "${bindir}/avtranscode ${bindir}/*.py"
