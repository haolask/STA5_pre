FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}/2.4.83:"

SRC_URI += " \
        file://0001-modetest-consider-supported-formats-before-selecting.patch \
        file://0002-modetest-use-SMPTE-pattern-as-cursor.patch \
        file://0003-drm-add-DRM_MODE_FB_BFF-flag-definition.patch \
        file://0004-libdrm-Add-color-map-control.patch \
        file://0005-drm-Add-DRM_MODE_FB_YFCR-flag-definition.patch \
        file://0006-modetest-add-support-of-st.patch \
"

