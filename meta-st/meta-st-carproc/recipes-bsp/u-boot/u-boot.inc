SUMMARY = "Universal Boot Loader for embedded devices"
PROVIDES = "virtual/bootloader"

DEPENDS_prepend = "mem-map-config m3-loaders u-boot-key-gen-native "

PACKAGE_ARCH = "${MACHINE_ARCH}"

UBOOT_SIGN += "${@bb.utils.contains('MACHINE_FEATURES', 'no_uboot', '', 'uboot-sign', d)}"

inherit uboot-config uboot-extlinux-config deploy cml1uboot ${UBOOT_SIGN}

inherit pythonnative
DEPENDS += "python-native python"

SYSROOT_DIRS += " /boot"

BUILD_CFLAGS += '-I${STAGING_INCDIR}/sta_mem_map'

EARLY_TUNER_FTR ?= "0"

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX} CC="${TARGET_PREFIX}gcc ${TOOLCHAIN_OPTIONS}" V=1'
EXTRA_OEMAKE += 'HOSTCC="${BUILD_CC} ${BUILD_CFLAGS} ${BUILD_LDFLAGS}"'
EXTRA_OEMAKE += 'NOSTDINC_FLAGS="-I${STAGING_INCDIR}/sta_mem_map"'
EXTRA_CFLAGS += "EARLY_TUNER_FTR=${EARLY_TUNER_FTR}"

PACKAGECONFIG ??= "openssl"
# u-boot will compile its own tools during the build, with specific
# configurations (aka when CONFIG_FIT_SIGNATURE is enabled) openssl is needed as
# a host build dependency.
PACKAGECONFIG[openssl] = ",,openssl-native"

# Allow setting an additional version string that will be picked up by the
# u-boot build system and appended to the u-boot version.  If the .scmversion
# file already exists it will not be overwritten.
UBOOT_LOCALVERSION ?= ""

# Some versions of u-boot use .bin and others use .img.  By default use .bin
# but enable individual recipes to change this value.
UBOOT_SUFFIX ??= "bin"
UBOOT_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.${UBOOT_SUFFIX}"
UBOOT_SYMLINK ?= "u-boot-${MACHINE}.${UBOOT_SUFFIX}"
UBOOT_MAKE_TARGET ?= "all"

# Output the ELF generated. Some platforms can use the ELF file and directly
# load it (JTAG booting, QEMU) additionally the ELF can be used for debugging
# purposes.
UBOOT_ELF ?= ""
UBOOT_ELF_SUFFIX ?= "elf"
UBOOT_ELF_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.${UBOOT_ELF_SUFFIX}"
UBOOT_ELF_BINARY ?= "u-boot.${UBOOT_ELF_SUFFIX}"
UBOOT_ELF_SYMLINK ?= "u-boot-${MACHINE}.${UBOOT_ELF_SUFFIX}"

# Some versions of u-boot build an SPL (Second Program Loader) image that
# should be packaged along with the u-boot binary as well as placed in the
# deploy directory.  For those versions they can set the following variables
# to allow packaging the SPL.
SPL_BINARY ?= ""
SPL_BINARYNAME ?= "${@os.path.basename(d.getVar("SPL_BINARY", True))}"
SPL_IMAGE ?= "${SPL_BINARYNAME}-${MACHINE}-${PV}-${PR}"
SPL_SYMLINK ?= "${SPL_BINARYNAME}-${MACHINE}"

# Additional environment variables or a script can be installed alongside
# u-boot to be used automatically on boot.  This file, typically 'uEnv.txt'
# or 'boot.scr', should be packaged along with u-boot as well as placed in the
# deploy directory.  Machine configurations needing one of these files should
# include it in the SRC_URI and set the UBOOT_ENV parameter.
UBOOT_ENV_SUFFIX ?= "txt"
UBOOT_ENV ?= ""
UBOOT_ENV_BINARY ?= "${UBOOT_ENV}.${UBOOT_ENV_SUFFIX}"
UBOOT_ENV_IMAGE ?= "${UBOOT_ENV}-${MACHINE}-${PV}-${PR}.${UBOOT_ENV_SUFFIX}"
UBOOT_ENV_SYMLINK ?= "${UBOOT_ENV}-${MACHINE}.${UBOOT_ENV_SUFFIX}"

# U-Boot EXTLINUX variables. U-Boot searches for /boot/extlinux/extlinux.conf
# to find EXTLINUX conf file.
UBOOT_EXTLINUX_INSTALL_DIR ?= "/boot/extlinux"
UBOOT_EXTLINUX_CONF_NAME ?= "extlinux.conf"
UBOOT_EXTLINUX_SYMLINK ?= "${UBOOT_EXTLINUX_CONF_NAME}-${MACHINE}-${PR}"


UBOOT_ATF_CONFIG = "# CONFIG_ARMV7_NONSEC is not set"
UBOOT_NON_ATF_CONFIG = "CONFIG_ARMV7_NONSEC=y\n# CONFIG_ARMV7_BOOT_SEC_DEFAULT is not set\nCONFIG_ARMV7_PSCI_NR_CPUS=4"

#$1 current UBOOT_MACHINE processed
configure_uboot_soc() {
    config=$1
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
    oe_runmake -C ${S} O=${WORKDIR}/${config} ${config}
    ${S}/scripts/kconfig/merge_config.sh -m -r -O "${WORKDIR}/${config}" "${WORKDIR}/${config}/.config" ${UBOOT_SOC_CONFIG} 1>&2
    cat ${UBOOT_SOC_CONFIG}
    rm ${UBOOT_SOC_CONFIG}
}


do_configure () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS

	if [ ! -e ${B}/.scmversion -a ! -e ${S}/.scmversion ]
	then
		echo ${UBOOT_LOCALVERSION} > ${B}/.scmversion
		echo ${UBOOT_LOCALVERSION} > ${S}/.scmversion
	fi

    if [ -n "${UBOOT_CONFIG}" ]
    then
        unset i j k
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]
                then
                    oe_runmake -C ${S} O=${B}/../${config} ${config}
                    configure_uboot_soc ${config}
                    unset k
                fi
            done
            unset  j
        done
        unset  i
    else
        oe_runmake -C ${S} O=${B} ${UBOOT_MACHINE}
        configure_uboot_soc
    fi

}

do_compile () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS

    if [ -n "${UBOOT_CONFIG}" ]
    then
        unset i j k
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]
                then
                    oe_runmake -C ${S} O=${B}/../${config} ${UBOOT_MAKE_TARGET} DEVICE_TREE=${UBOOT_DEVICETREE} ${EXTRA_CFLAGS}
                    for binary in ${UBOOT_BINARIES}; do
                        k=$(expr $k + 1);
                        if [ $k -eq $i ]; then
                            cp ${B}/../${config}/${binary} ${B}/../${config}/u-boot-${type}.${UBOOT_SUFFIX}
                        fi
                    done
                    unset k
                fi
            done
            unset  j
        done
        unset  i
    else
        oe_runmake -C ${S} O=${B} ${UBOOT_MAKE_TARGET} DEVICE_TREE=${UBOOT_DEVICETREE}
    fi

}



do_install () {
    if [ -n "${UBOOT_CONFIG}" ]
    then
        rm -f ${D}/boot/u-boot.${UBOOT_SUFFIX}
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]
                then
                    install -d ${D}/boot
                    install -m 644 ${B}/../${config}/u-boot-${type}.${UBOOT_SUFFIX} ${D}/boot/u-boot-${config}.${UBOOT_SUFFIX}

                    boot_config="$(echo ${config} | sed 's#.*-\([^_]*\)_defconfig#\1#')"
                    ln -sf u-boot-${config}.${UBOOT_SUFFIX} ${D}/boot/u-boot-${boot_config}.${UBOOT_SUFFIX}
                    if [ "${boot_config}" = "xloader" ]; then
                        ln -sf u-boot-${config}.${UBOOT_SUFFIX} ${D}/boot/u-boot.${UBOOT_SUFFIX}
                    fi
                fi
            done
            unset  j
        done
        unset  i
    else
        install -d ${D}/boot
        install -m 644 ${B}/${UBOOT_BINARY} ${D}/boot/${UBOOT_IMAGE}
        ln -sf ${UBOOT_IMAGE} ${D}/boot/${UBOOT_BINARY}
    fi

}
FILES_${PN} = "/boot "

do_deploy () {
    if [ -n "${UBOOT_CONFIG}" ]
    then
        rm -f ${DEPLOYDIR}/u-boot.${UBOOT_SUFFIX}
        for config in ${UBOOT_MACHINE}; do
            i=$(expr $i + 1);
            for type in ${UBOOT_CONFIG}; do
                j=$(expr $j + 1);
                if [ $j -eq $i ]
                then
                    install -d ${DEPLOYDIR}
                    install -m 644 ${B}/../${config}/u-boot-${type}.${UBOOT_SUFFIX} ${DEPLOYDIR}/u-boot-${config}.${UBOOT_SUFFIX}
                    install -m 644 ${B}/../${config}/u-boot ${DEPLOYDIR}/u-boot-${config}

                    boot_config="$(echo ${config} | sed 's#.*-\([^_]*\)_defconfig#\1#')"
                    ln -sf u-boot-${config}.${UBOOT_SUFFIX} ${DEPLOYDIR}/u-boot-${boot_config}.${UBOOT_SUFFIX}
                    if [ "${boot_config}" = "xloader" ]; then
                        ln -sf u-boot-${config}.${UBOOT_SUFFIX} ${DEPLOYDIR}/u-boot.${UBOOT_SUFFIX}
                        ln -sf u-boot-${config} ${DEPLOYDIR}/u-boot
                    else
                        ln -sf u-boot-${config}.${UBOOT_SUFFIX} ${DEPLOYDIR}/u-boot-flashloader.${UBOOT_SUFFIX}
                        ln -sf u-boot-${config} ${DEPLOYDIR}/u-boot-flashloader
                    fi
                fi
            done
            unset  j
        done
        unset  i
    else
        install -d ${DEPLOYDIR}
        install -m 644 ${B}/${UBOOT_BINARY} ${DEPLOYDIR}/${UBOOT_IMAGE}
   fi

}

do_savedefconfig() {
    for config in ${UBOOT_MACHINE}; do
        bbplain "Saving defconfig to:\n${WORKDIR}/${config}/defconfig"
        oe_runmake -C ${WORKDIR}/${config} savedefconfig
    done
}
do_savedefconfig[nostamp] = "1"
addtask savedefconfig after do_configure

addtask deploy before do_build after do_compile

#we would like to have fip image generated if u-boot has been compiled
python __anonymous() {
    if bb.utils.contains('MACHINE_FEATURES', 'atf', 'ATF', None, d) is not None:
        d.appendVarFlag('do_build', 'depends', ' fip-image:do_deploy')

    list = d.getVar("UBOOT_MACHINE", True).lstrip().split(' ')
    xloader_item = [item for item in list if "xloader" in item]
    other_item = [item for item in list if not "xloader" in item]
    d.setVar('MENUCFG_XLDR', d.getVar("WORKDIR",True)+'/'+''.join(xloader_item))
    d.setVar('MENUCFG', d.getVar("WORKDIR",True)+'/'+''.join(other_item))

    # Set build folder pointing by default on xloader build.
    d.setVar('B', d.getVar("WORKDIR",True)+'/'+''.join(xloader_item))
}
