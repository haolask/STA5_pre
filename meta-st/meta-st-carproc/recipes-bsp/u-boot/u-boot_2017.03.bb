HOMEPAGE = "http://www.denx.de/wiki/U-Boot/WebHome"
SECTION = "bootloaders"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://Licenses/README;md5=a2c678cfd4a4d97135585cad908541c6"
PE = "1"

require u-boot.inc

#Let's use our external source tree
inherit externalsrc
EXTERNALSRC_pn-${PN} = "${ST_LOCAL_SRC}/u-boot"
EXTERNALSRC_BUILD_pn-${PN} = "${B}"
SRC_URI=""

#Big Hack -- Remove the fdt file brought by dtc-native that are conflicting with u-boot ones.
do_configure_prepend () {
    rm -f ${STAGING_INCDIR_NATIVE}/libfdt.h
    rm -f ${STAGING_INCDIR_NATIVE}/libfdt_env.h

}

DEPENDS_append = "dtc-native mem-map-config"
