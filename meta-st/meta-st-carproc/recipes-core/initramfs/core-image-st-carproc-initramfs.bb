SUMMARY = "ST Car Processor image - Dependency of other images - DO NOT USE DIRECTLY"
LICENSE = "MIT"

PV = "1.0.0"
PR = "r1"

inherit core-image

#PACKAGE_INSTALL += ""

#We want an almost empty image
CORE_IMAGE_BASE_INSTALL = "st-init busybox util-linux mtd-utils-ubifs e2fsprogs"


IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

#Should be placed in machine or local.conf
#INITRAMFS_IMAGE = "core-image-st-carproc-initramfs"
#INITRAMFS_IMAGE_BUNDLE = "1"
