FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRCREV = "4b9a70d5789b0b74f43957a6c19ab2156a72d3e0"
PV = "0.3.14+git${SRCPV}"

SRC_URI = " \
	git://git.code.sf.net/p/trousers/trousers \
	file://trousers.init.sh \
	file://trousers-udev.rules \
	file://tcsd.service \
	file://get-user-ps-path-use-POSIX-getpwent-instead-of-getpwe.patch \
	"

S = "${WORKDIR}/git"

do_install_append() {
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/trousers.init.sh ${D}${sysconfdir}/init.d/trousers
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${WORKDIR}/trousers-udev.rules ${D}${sysconfdir}/udev/rules.d/45-trousers.rules

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system
        install -m 0644 ${WORKDIR}/tcsd.service ${D}${systemd_unitdir}/system/
        sed -i -e 's#@SBINDIR@#${sbindir}#g' ${D}${systemd_unitdir}/system/tcsd.service
    fi
}

# libtspi needs tcsd for most (all?) operations, so suggest to
# install that.
RRECOMMENDS_libtspi = "${PN}"

FILES_libtspi_prepend += " \
	${libdir}/*.so.1 \
	"

FILES_libtspi-dbg = " \
	${libdir}/.debug \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/tspi \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/trspi \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/include/*.h \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/include/tss \
	"

FILES_libtspi-dev = " \
	${includedir} \
	${libdir}/*.so \
	"
FILES_${PN}-dbg = " \
	${sbindir}/.debug \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/tcs \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/tcsd \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/tddl \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/trousers \
	${prefix}/src/debug/${BPN}/${PV}-${PR}/git/src/include/trousers \
	"

GROUPADD_PARAM_${PN} = "--system tss"
USERADD_PARAM_${PN} = "--system -M -d /var/lib/tpm -s /bin/false -g tss tss"

