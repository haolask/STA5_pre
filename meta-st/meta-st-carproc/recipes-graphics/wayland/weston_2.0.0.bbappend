FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://0001-linux-dmabuf-align-DMABUF-exposed-formats-with-EGL-s.patch \
            file://0002-libinput-seat-Don-t-regard-no-input-devices-as-failu.patch \
            file://0003-compositor-add-xlog-debug-info.patch \
            file://0004-pixman-renderer-support-of-linux-dmabuf.patch \
            file://0005-pixman-renderer-support-SHM-and-DMABUF-NV12-pixel-fo.patch \
            file://0006-pixman-renderer-support-SHM-and-DMABUF-RGB-BGR-24-bi.patch \
            file://0007-pixman-renderer-support-SHM-and-DMABUF-BGR-24-32-bit.patch \
            file://0008-pixman-renderer-support-SHM-I420-pixel-format-debug-.patch \
            file://0009-compositor-st-initial-commit-copy-of-compositor-drm.patch \
            file://0010-compositor-st-enable-sprites.patch \
            file://0011-compositor-st-cursors_are_broken-per-output.patch \
            file://0012-compositor-st-add-sprite-and-cursor-logs-when-they-a.patch \
            file://0013-compositor-st-assign-dmabuf-buffers-in-DRM-planes-ov.patch \
            file://0014-compositor-st-manager-zorder.patch \
            file://0015-compositor-st-interlaced-buffers-support.patch \
            file://0016-compositor-st-ignore-the-waylandsink-1x1-area-surfac.patch \
            file://0017-compositor-st-use-DRM-planes-with-pixman.patch \
            file://0018-compositor-st-forbid-sprite-on-several-crtc.patch \
            file://0019-compositor-st-wait-for-vblank-of-all-sprites.patch \
            file://0020-compositor-st-fix-fd-leak-for-gbm_bo.patch \
            file://0021-ST-display-capture-support.patch \
            file://0022-gl-renderer-force-BT709-full-range-for-NV12-dmabuf-b.patch \
            file://0023-gl-renderer-implements-the-EGL_KHR_partial_update-ex.patch \
            file://0024-compositor-st-drmModeSetPlane-error-management.patch \
            file://0025-linux-dmabuf-add-backend-private-data-in-linux_dmabu.patch \
            file://0026-compositor-st-use-backend_user_data-to-store-dmabuf-.patch \
            file://0027-compositor-st-don-t-repaint-primary-plane-if-no-dama.patch \
            file://0028-compositor-st-lastly-call-DRM_IOCTL_MODE_PAGE_FLIP-i.patch \
            file://0029-compositor-st-release-buffer-when-drmModeSetPlane-fa.patch \
            file://0030-compositor-st-don-t-create-output-with-off-mode.patch \
            file://0031-compositor-st-consider-driver-DRM-plane-z-order-no-z.patch \
            file://0032-Desktop-shell-Position-maximized-surfaces-on-the-cor.patch \
            file://0033-compositor-drm-fix-z-order-inversion-in-plane-assign.patch \
            file://0034-ivi-shell-Do-not-set-transform-rotate-in-case-of-no-.patch \
            file://0035-compositor-st-scanout-do-not-use-gbm_bo_import-for-s.patch \
            file://0036-compositor-st-Add-support-for-YUV-Full-color-range.patch \
            file://0037-gl_renderer-Display-warning-in-case-of-YUV-full-rang.patch \
            file://0001-weston-simple-egl-add-options-to-provide-the-output-.patch \
            file://0001-gl-renderer-add-support-of-UYVY-color-format.patch \
            file://0040-compositor-st-Add-Gamma-control-service.patch \
            file://0041-clients-Add-gamma-control-client-example.patch \
            file://0042-clients-weston-gamma-add-negative-and-linear-options.patch \
            file://0043-compositor-st-Re-allow-dmabufs-for-scanout.patch \
            file://0044-libweston-Add-pixel-format-helpers.patch \
            file://0045-libweston-Allow-more-scanout-format.patch \
            file://0046-compositor-st-Use-plane_state_coords_for_view-for-sc.patch \
            file://0047-ivi-layout-add-screen-remove-layer-API.patch \
            "

SYSROOT_PREPROCESS_FUNCS += "weston_sysroot_preprocess"

weston_sysroot_preprocess() {
    install -d ${SYSROOT_DESTDIR}/${includedir}/weston/
    install ${S}/libweston/compositor.h ${SYSROOT_DESTDIR}/${includedir}/weston/

    install -d ${SYSROOT_DESTDIR}/${includedir}/libweston-2/
    install ${S}/libweston/linux-dmabuf.h ${SYSROOT_DESTDIR}/${includedir}/libweston-2/
}
