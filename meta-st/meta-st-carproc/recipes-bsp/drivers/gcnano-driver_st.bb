SUMMARY = "GCNano kernel drivers"
DESCRIPTION = "GCNano kernel drivers"
# todo please review me, as it is dual license MIT/GPL
LICENSE = "GPLv1 & MIT"
LIC_FILES_CHKSUM = "file://Makefile;endline=53;md5=d77ff5896dbbf8a8bc3f7c5e8f905fcc"

inherit module

PV="6.2.4"
PR="p4"

inherit externalsrc

EXTERNALSRC_pn-gcnano-driver ?= "${ST_LOCAL_SRC}/gcnano-driver"
EXTERNALSRC_BUILD_pn-gcnano-driver ?= "${ST_LOCAL_SRC}/gcnano-driver"

# todo manage debug/release mode
# todo add a dedicated platform
# todo manage android build (sync)
EXTRA_OEMAKE  = "KERNEL_DIR=${STAGING_KERNEL_BUILDDIR}"
EXTRA_OEMAKE += "SOC_PLATFORM=st-st"

do_compile() {
  oe_runmake -C ${S}
}

do_install() {
  install -d ${D}/lib/modules/${KERNEL_VERSION}
  install -m 0755 ${B}/galcore.ko ${D}/lib/modules/${KERNEL_VERSION}/galcore.ko
}

do_clean_gcnano() {
  cd ${S}
  git status --porcelain | grep \?\? | cut -d ' ' -f 2 | xargs rm -rf
}
addtask clean_gcnano after do_clean before do_cleansstate
