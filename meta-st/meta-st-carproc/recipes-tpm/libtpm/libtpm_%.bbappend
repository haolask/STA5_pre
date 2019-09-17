FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRCREV = "ad44846dda5a96e269ad2f78a532e01e9a2f02a1"
SRC_URI = " \
	git://github.com/stefanberger/libtpms.git \
	file://Convert-another-vdprintf-to-dprintf.patch \
	file://Use-format-s-for-call-to-dprintf.patch \
	"
