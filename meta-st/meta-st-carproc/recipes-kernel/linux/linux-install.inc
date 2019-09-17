

KERNEL_IMAGEDEST = "kernel_binaries"

#Originally, sysroot filing was disabled by kernel classes.
#So add only the extra image we want, overwritting default one
SYSROOT_DIRS = "/${KERNEL_IMAGEDEST}"


sysroot_stage_all() {
	sysroot_stage_dirs ${D} ${SYSROOT_DESTDIR}
}
