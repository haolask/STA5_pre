FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

FILES_${PN} += "${libdir}/weston/* ${libdir}/libweston-2/*"

# Avoid QA Issue: non -dev/-dbg/nativesdk- package contains symlink .so
INSANE_SKIP_${PN} += "dev-so"

do_install_append() {
	install -d ${D}${libdir}/libweston-2/
	ln -sf ../weston/ivi-input-controller.so ${D}${libdir}/libweston-2/ivi-input-controller.so
}
