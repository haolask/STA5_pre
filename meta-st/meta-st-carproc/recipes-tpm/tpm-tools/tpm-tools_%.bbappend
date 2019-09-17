FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRCREV = "5c5126bedf2da97906358adcfb8c43c86e7dd0ee"
SRC_URI = " \
	git://git.code.sf.net/p/trousers/tpm-tools \
	file://tpm-tools-extendpcr.patch \
	"

PV = "1.3.9.1+git${SRCPV}"

inherit autotools-brokensep gettext

S = "${WORKDIR}/git"

do_configure_prepend () {
	mkdir -p po
	mkdir -p m4
	cp -R po_/* po/
	touch po/Makefile.in.in
	touch m4/Makefile.am
}
