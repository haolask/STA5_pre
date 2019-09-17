SUMMARY = "Network-related giomodule for glib using openssl"
HOMEPAGE = "https://github.com/GNOME/glib-openssl"
BUGTRACKER = "http://bugzilla.gnome.org/"

LICENSE = "LGPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=5f30f0716dfdd0d91eb439ebec522ec2"

SECTION = "libs"
DEPENDS = "glib-2.0 intltool-native"

SRC_URI[archive.md5sum] = "4c2efed3270eff406b95b7c2efefdcf0"
SRC_URI[archive.sha256sum] = "b3e573f745df875045c85144fb4b2dedc2e9f1621eee320feefb4cfcf70be17a"

PACKAGECONFIG ??= "ca-certificates openssl"

# No explicit dependency as it works without ca-certificates installed
PACKAGECONFIG[ca-certificates] = "--with-ca-certificates=${sysconfdir}/ssl/certs/ca-certificates.crt,--without-ca-certificates"

inherit gnomebase gettext upstream-version-is-even gio-module-cache

FILES_${PN} += "${libdir}/gio/modules/libgio*.so ${datadir}/dbus-1/services/"
FILES_${PN}-dev += "${libdir}/gio/modules/libgio*.la"
FILES_${PN}-staticdev += "${libdir}/gio/modules/libgio*.a"
