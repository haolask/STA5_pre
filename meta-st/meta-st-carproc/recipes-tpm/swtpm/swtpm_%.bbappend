# configure checks for the tools already during compilation and
# then swtpm_setup needs them at runtime
DEPENDS += "tpm-tools-native expect-native socat-native"
RDEPENDS_${PN} += "tpm-tools"

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRCREV = "073e71f99eaa7a0ff9499339176af1af62c090b2"
SRC_URI = " \
	git://github.com/stefanberger/swtpm.git \
	file://fix_signed_issue.patch \
	file://fix_lib_search_path.patch \
	file://fix_fcntl_h.patch \
	file://ioctl_h.patch \
	"

PACKAGECONFIG += "cuse"
PACKAGECONFIG[cuse] = "--with-cuse, --without-cuse"

# dup bootstrap
do_configure_prepend () {
	libtoolize --force --copy
	autoheader
	aclocal
	automake --add-missing -c
	autoconf
}

BBCLASSEXTEND = "native nativesdk"
