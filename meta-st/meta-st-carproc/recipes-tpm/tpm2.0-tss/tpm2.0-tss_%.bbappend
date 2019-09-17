FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=17067aa50a585593d421b16cffd805a9"

SRCREV = "8e25d0cbb287d30c93b2b77e99bc761dc67e31a9"

SRC_URI = " \
	git://github.com/01org/TPM2.0-TSS.git;protocol=git;branch=master;name=TPM2.0-TSS;destsuffix=TPM2.0-TSS \
	file://ax_pthread.m4 \
	file://fix_musl_select_include.patch \
	"

inherit autotools pkgconfig systemd

INHERIT += "extrausers"
EXTRA_USERS_PARAMS = "\
	useradd -p '' tss; \
	groupadd tss; \
	"

PACKAGES_append = "${PN}"

SYSTEMD_PACKAGES += "resourcemgr"
SYSTEMD_SERVICE_resourcemgr = "resourcemgr.service"
SYSTEMD_AUTO_ENABLE_resourcemgr = "enable"

do_patch[postfuncs] += "fix_systemd_unit"
fix_systemd_unit () {
    sed -i -e 's;^ExecStart=.*/resourcemgr;ExecStart=${sbindir}/resourcemgr;' ${S}/contrib/resourcemgr.service
}

do_install_append() {
    install -d ${D}${systemd_system_unitdir}
    install -m0644 ${S}/contrib/resourcemgr.service ${D}${systemd_system_unitdir}/resourcemgr.service
}

FILES_resourcemgr += "${systemd_system_unitdir}/resourcemgr.service"
