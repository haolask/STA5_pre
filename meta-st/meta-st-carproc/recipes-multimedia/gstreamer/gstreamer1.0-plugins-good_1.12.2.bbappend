FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/:"

PACKAGECONFIG ?= " \
    ${GSTREAMER_ORC} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'pulseaudio', 'pulseaudio', '', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'x11', 'x11', '', d)} \
    cairo flac gdk-pixbuf gudev jpeg libpng soup speex taglib v4l2 \
    libv4l2 \
"

EXTRA_OECONF += " \
    --enable-v4l2-probe \
    "

do_configure_prepend() {
    ${S}/autogen.sh --noconfigure
}
