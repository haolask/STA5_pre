require u-boot-common_${PV}.inc

SRC_URI += "file://default-gcc.patch"

SUMMARY = "U-Boot bootloader fw_printenv/setenv utilities"
DEPENDS = "mtd-utils mem-map-config m3-loaders"

INSANE_SKIP_${PN} = "already-stripped"

CFLAGS += '-I${STAGING_INCDIR}/sta_mem_map'
EXTRA_OEMAKE_class-target = 'CROSS_COMPILE=${TARGET_PREFIX} CC="${CC} ${CFLAGS} ${LDFLAGS}" HOSTCC="${BUILD_CC} ${BUILD_CFLAGS} ${BUILD_LDFLAGS}" V=1'
EXTRA_OEMAKE_class-cross = 'ARCH=${TARGET_ARCH} CC="${CC} ${CFLAGS} ${LDFLAGS}" V=1'

UBOOT_ATF_CONFIG = "# CONFIG_ARMV7_NONSEC is not set"
UBOOT_NON_ATF_CONFIG = "CONFIG_ARMV7_NONSEC=y\n# CONFIG_ARMV7_BOOT_SEC_DEFAULT is not set\nCONFIG_ARMV7_PSCI_NR_CPUS=4"

#Let's use our external source tree
inherit externalsrc
EXTERNALSRC_pn-${PN} = "${ST_LOCAL_SRC}/u-boot"
SRCTREECOVEREDTASKS = "do_unpack do_fetch"

inherit uboot-config

do_configure () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS

	UBOOT_SOC_CONFIG="$(mktemp)"
	case "${TARGET_SOC_ID}" in
		"SOCID_STA1295")
			echo "CONFIG_SOC_STA1295=y" > ${UBOOT_SOC_CONFIG}
			;;
		"SOCID_STA1195")
			echo "CONFIG_SOC_STA1195=y" > ${UBOOT_SOC_CONFIG}
			;;
		"SOCID_STA1385")
			echo "CONFIG_SOC_STA1385=y" > ${UBOOT_SOC_CONFIG}
			;;
		"SOCID_STA1275")
			echo "CONFIG_SOC_STA1275=y" > ${UBOOT_SOC_CONFIG}
			;;
		*)
			;;
	esac
	echo "${@bb.utils.contains('MACHINE_FEATURES', 'atf', '${UBOOT_ATF_CONFIG}', '${UBOOT_NON_ATF_CONFIG}', d)}" >> ${UBOOT_SOC_CONFIG}
	oe_runmake -C ${S} O=${B} ${CURRENT_XLDR_CONFIG}
	${S}/scripts/kconfig/merge_config.sh -m -r -O "${B}" "${B}/.config" ${UBOOT_SOC_CONFIG} 1>&2
	cat ${UBOOT_SOC_CONFIG}
	rm ${UBOOT_SOC_CONFIG}
}

do_compile () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS

	oe_runmake -C ${S} O=${B} env
}

do_install () {
	install -d ${D}${base_sbindir}
	install -d ${D}${sysconfdir}
	install -m 755 ${B}/tools/env/fw_printenv ${D}${base_sbindir}/fw_printenv
	install -m 755 ${B}/tools/env/fw_printenv ${D}${base_sbindir}/fw_setenv
	install -m 0644 ${S}/tools/env/fw_env.config ${D}${sysconfdir}/fw_env.config
}

do_install_class-cross () {
	install -d ${D}${bindir_cross}
	install -m 755 ${B}/tools/env/fw_printenv ${D}${bindir_cross}/fw_printenv
	install -m 755 ${B}/tools/env/fw_printenv ${D}${bindir_cross}/fw_setenv
}

SYSROOT_DIRS_append_class-cross = " ${bindir_cross}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
BBCLASSEXTEND = "cross"

python __anonymous() {
    list = d.getVar("UBOOT_MACHINE", True).lstrip().split(' ')
    xloader_item = [item for item in list if "xloader" in item]
    d.setVar('CURRENT_XLDR_CONFIG', ''.join(xloader_item))
}
