# Manage our own kernel DT, configuration and configuration fragment in layer
# Also install shared folder for sources access from other recipes (modules)


KERNEL_NON_ATF_CONFIG = "CONFIG_ARM_PSCI=n\n"

KERNEL_OPTEE_CONFIG = "CONFIG_TEE=y\nCONFIG_OPTEE=y"
KERNEL_NO_OPTEE_CONFIG = "CONFIG_TEE=n\nCONFIG_OPTEE=n"

KERNEL_KMEMLEAK_TOOL_CONFIG = "CONFIG_DEBUG_KMEMLEAK=y\nCONFIG_DEBUG_KMEMLEAK_EARLY_LOG_SIZE=2000\nCONFIG_DEBUG_KMEMLEAK_TEST=n\nCONFIG_DEBUG_KMEMLEAK_DEFAULT_OFF=n\n"

DEPENDS += "dtc-native"

do_configure_prepend() {
    # remove previously installed defconfig file.
    # We need to be sure we are compiling with the configuration from the source tree
    if [ -f "${S}/arch/${ARCH}/configs/${KBUILD_DEFCONFIG}" ]; then
        if [ -f "${WORKDIR}/defconfig" ]; then
            set +e
            cmp "${WORKDIR}/defconfig" "${S}/arch/${ARCH}/configs/${KBUILD_DEFCONFIG}"
            if [ $? -ne 0 ]; then
                bbwarn "A defconfig has been detected in WORKDIR different from source tree (${KBUILD_DEFCONFIG}). Overwriting it."
                cp -f ${S}/arch/${ARCH}/configs/${KBUILD_DEFCONFIG} ${WORKDIR}/defconfig
                rm -f ${B}/.config
            fi
            set -e
        else
            cp -f ${S}/arch/${ARCH}/configs/${KBUILD_DEFCONFIG} ${WORKDIR}/defconfig
            rm -f ${B}/.config
        fi
    else
        bberror "Could not find kernel defconfig: ${KBUILD_DEFCONFIG}"
    fi

}

do_configure_append() {
    if [ ! -z "${THISDIR}/config/${KERNEL_CONFIG_FRAGMENTS}" ]
    then
        for f in ${THISDIR}/config/${KERNEL_CONFIG_FRAGMENTS}
        do
            # Check if the config fragment exists
            # the OE meta data
            if [ ! -e "$f" ]
            then
                bberror "Could not find kernel config fragment $f"
                exit 1
            fi
        done

        # Now that all the fragments are located merge them.
        (${S}/scripts/kconfig/merge_config.sh -m -r -O ${B} ${B}/.config ${THISDIR}/config/${KERNEL_CONFIG_FRAGMENTS} 1>&2 )
    fi

    #Also merge some flags according to ATF/OPTEE are enabled or not
    config_file=$(mktemp)
    echo "${@bb.utils.contains('MACHINE_FEATURES', 'atf', '', '${KERNEL_NON_ATF_CONFIG}', d)}" > $config_file

    ATF_ENABLED="${@bb.utils.contains('MACHINE_FEATURES', 'atf', 'True', 'False', d)}"
    #Enable Optee only if ATF is actiavted
    if [ "${ATF_ENABLED}" = "True" ]; then
	echo "${@bb.utils.contains('BL32_SP', 'optee', '${KERNEL_OPTEE_CONFIG}', '${KERNEL_NO_OPTEE_CONFIG}', d)}" >> $config_file
	bbnote "The following config switch(es) are overrided at compilation time: "$(cat $config_file)
    fi
    if  [ "${ATF_ENABLED}" = "False" ]; then
	echo "${KERNEL_NO_OPTEE_CONFIG}" >> $config_file
	if [ "${BL32_SP}" == "optee" ]; then
	    bbwarn "Optee cannot be enabled, please use an ATF compatible machine"
	fi
    fi

    #Check if KMEMLEAK_TOOL is enabled or not
    echo "${@bb.utils.contains('KMEMLEAK_TOOL', '1', '${KERNEL_KMEMLEAK_TOOL_CONFIG}', '', d)}" >> $config_file
    bbnote "The following config switch(es) are overrided at compilation time: "$(cat $config_file)


    ${S}/scripts/kconfig/merge_config.sh -m -r -O ${B} ${B}/.config $config_file 1>&2
    cat $config_file
    rm $config_file
}


#Install kernel sources in STAGING_KERNEL_DIR folder
python external_src_link_creation() {
    import os
    if d.getVar("EXTERNALSRC", True):
        s = d.getVar("S", True)
        kernsrc = d.getVar("STAGING_KERNEL_DIR", True)
        bb.utils.mkdirhier(kernsrc)
        bb.utils.remove(kernsrc, recurse=True)
        os.symlink(s, kernsrc)
}

do_patch[postfuncs] += "external_src_link_creation"
