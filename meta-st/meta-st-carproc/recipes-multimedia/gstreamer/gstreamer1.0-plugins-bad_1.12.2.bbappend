FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# remove egl (glimagesink)
# PACKAGECONFIG_GL ?= "${@bb.utils.contains('DISTRO_FEATURES', 'opengl', 'gles2 egl', '', d)}"
PACKAGECONFIG_GL = ""

SRC_URI_append = " \
	file://0001-STM-h264parse-update-src-caps-if-resolution-change.patch \
	file://0002-STM-waylandsink-disable-last-sample-sink-feature.patch \
	file://0003-STM-waylandsink-increase-bufferpool-size.patch \
	file://0004-STM-waylandsink-set-video-alignment.patch \
	file://0005-STM-waylandsink-support-fullscreen.patch \
	file://0006-STM-waylandsink-rendering-window-size-setting.patch \
	file://0007-STM-waylandsink-fix-RGB888-SHM-format-conversion.patch \
	file://0008-STM-waylandsink-ranked-as-primary.patch \
	file://0009-STM-waylandsink-support-dmabuf-YUY2-and-I420-pixel-f.patch \
	file://0010-STM-waylandsink-memory-DMABuf-preferred-while-negoti.patch \
	file://0011-glupload-add-GST_CAPS_FEATURE_MEMORY_DMABUF.patch \
	file://0012-cavsvideoparser-add-a-Chinese-AVS-video-bitstream-pa.patch \
	file://0013-cavsvideoparse-add-a-Chinese-AVS-video-parser-elemen.patch \
	file://0014-cavsvideoparser-fix-shift-computation-in-READ_INT32-.patch \
	file://0015-cavsvideoparse-fix-media-type-and-src-caps-stream-fo.patch \
	file://0016-cavsvideoparser-drop-video_edit-user_data-extension-.patch \
	file://0017-gstwaylandsink-Dmabuf-re-integration.patch \
	file://0018-gstwaylandsink-Enable-16bits-RGB565-for-DMABUF-suppo.patch \
	file://0019-waylandsink-Add-YUV-Full-Range-support.patch \
	file://0020-glimagesink-don-t-use-wl_egl_window-as-a-proxy-objec.patch \
	file://0021-gstwaylandsink-adaptation-to-manage-IVI-shell.patch \
	file://0022-gstwaylandsink-release-wl_display-resources.patch \
	file://0023-waylandsink-remove-quark-usage-to-identify-couple-wl.patch \
	file://0024-waylandsink-fix-a-maximum-number-of-dma-buffers.patch \
	file://0025-h264parse-early-set-src-caps-when-input-is-avc.patch \
	file://0026-waylandpool-fix-bad-wlbuffer-assignement-in-case-of-.patch \
	file://0001-waylandsink-add-surface-id-to-props.patch \
	file://0001-waylandsink-add-support-of-UYVY-color-format.patch \
	file://0027-waylandsink-ensure-surface-creation.patch \
    file://0028-waylandsink-update-wl_display-resources-release.patch \
"

PACKAGECONFIG ?= " \
    ${GSTREAMER_ORC} \
    ${PACKAGECONFIG_GL} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'bluetooth', 'bluez', '', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'directfb', 'directfb', '', d)} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'wayland', '', d)} \
    bz2 curl dash hls neon sbc smoothstreaming sndfile uvch264  \
    faac \
"

ARM_INSTRUCTION_SET = "arm"

do_configure_prepend() {
    ${S}/autogen.sh --noconfigure
}

do_install_append() {
    install -d ${D}${libdir}/pkgconfig ${D}${includedir}/gstreamer-1.0/wayland
    install -m 644 ${B}/pkgconfig/gstreamer-wayland.pc ${D}${libdir}/pkgconfig/gstreamer-wayland-1.0.pc
    install -m 644 ${S}/gst-libs/gst/wayland/wayland.h ${D}${includedir}/gstreamer-1.0/wayland
}

# In 1.6.2, the "--enable-hls" configure option generated an installable package
# called "gstreamer1.0-plugins-bad-fragmented". In 1.7.1 that HLS plugin package
# has become "gstreamer1.0-plugins-bad-hls". See:
# http://cgit.freedesktop.org/gstreamer/gst-plugins-bad/commit/?id=efe62292a3d045126654d93239fdf4cc8e48ae08

PACKAGESPLITFUNCS_append = " handle_hls_rename "

python handle_hls_rename () {
    d.setVar('RPROVIDES_gstreamer1.0-plugins-bad-hls', 'gstreamer1.0-plugins-bad-fragmented')
    d.setVar('RREPLACES_gstreamer1.0-plugins-bad-hls', 'gstreamer1.0-plugins-bad-fragmented')
    d.setVar('RCONFLICTS_gstreamer1.0-plugins-bad-hls', 'gstreamer1.0-plugins-bad-fragmented')
}

