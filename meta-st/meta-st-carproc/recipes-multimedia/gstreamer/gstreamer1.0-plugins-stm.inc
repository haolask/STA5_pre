require recipes-multimedia/gstreamer/gstreamer1.0-plugins.inc

# Untill patch task is fixed for devtool usage
include gstreamer1.0-fixdevtool.inc

LICENSE = "GPLv2+ & LGPLv2.1+"

DEPENDS += "gstreamer1.0-plugins-base gstreamer1.0-plugins-bad"

inherit gettext

# Add v4l2 below to use libv4l2
PACKAGECONFIG ??= " \
    libdrm \
    v4l2 \
    "

do_install_append() {
    if [ "${@bb.utils.contains('DISTRO_FEATURES', 'swdecode', 'swdecode', '', d)}" = "swdecode" ]; then
        rm ${D}${libdir}/gstreamer-1.0/libgstv4l2trans.*
        rm ${D}${libdir}/gstreamer-1.0/libgstv4l2enc.*
        rm ${D}${libdir}/gstreamer-1.0/libgstv4l2dec.*
    fi
}

PACKAGECONFIG[libdrm] = ",,libdrm"
PACKAGECONFIG[v4l2] = "--with-libv4l2,--without-libv4l2,v4l-utils"

EXTRA_OECONF += " \
    ${GSTREAMER_1_0_ORC} \
"
INSANE_SKIP_${PN} = "file-rdeps"
