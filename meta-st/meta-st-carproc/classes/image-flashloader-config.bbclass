
include conf/flashloader/static_configuration.inc

do_patch_flashloader_config () {
	IMAGE_FLASHLOADER_CONFIG_NAME="config-${IMAGE_LINK_NAME}.txt"
	install -d ${DEPLOY_DIR_IMAGE}
	install -m644 ${FLASHLOADER_CONFIG} ${DEPLOY_DIR_IMAGE}
	if [ ! -z "${INITRAMFS_IMAGE}" -a x"${INITRAMFS_IMAGE_BUNDLE}" = x1 ]; then
		patch_flashloader_initramfs
	fi
	patch_flashloader_main
	mv ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME} ${DEPLOY_DIR_IMAGE}/${IMAGE_FLASHLOADER_CONFIG_NAME}

	if [ ${MEMORY_BOOT_DEVICE} = "MMC" ]; then
		if [ "${SOC_FAMILY}" != "sta:sta1385" ]; then
			BOOT_PARTITION_IMAGE_FLASHLOADER_CONFIG_NAME="config-${IMAGE_LINK_NAME}-BOOT_PARTITION_CUT3.txt"
			cp -f ${DEPLOY_DIR_IMAGE}/${IMAGE_FLASHLOADER_CONFIG_NAME} ${DEPLOY_DIR_IMAGE}/${BOOT_PARTITION_IMAGE_FLASHLOADER_CONFIG_NAME}
			#Do some change to insert the boot partition on M3_XL
			M3_XL_VALUE="$(grep "(M3_XL)" ${DEPLOY_DIR_IMAGE}/${BOOT_PARTITION_IMAGE_FLASHLOADER_CONFIG_NAME} | sed 's#.*[:,]\(.*(M3_XL)\),.*#\1#' | sed -e 's#@[^(]*##')"
			echo "M3_XL_VALUE = ${M3_XL_VALUE}"
			sed -i "s#^; Raw partitions.*#;mmc boot partition 0\nbootparts=mmc:${M3_XL_VALUE}\n&#" ${DEPLOY_DIR_IMAGE}/${BOOT_PARTITION_IMAGE_FLASHLOADER_CONFIG_NAME}
			sed -i 's#\([:,]\)\(.*(M3_XL)\),#\1#' ${DEPLOY_DIR_IMAGE}/${BOOT_PARTITION_IMAGE_FLASHLOADER_CONFIG_NAME}
		fi
	fi
}

patch_flashloader_initramfs () {
	FLASHLOADER_CONFIG_NAME=$(basename ${FLASHLOADER_CONFIG})
	cd ${IMGDEPLOYDIR} > /dev/null
	rootfs_Msize=$(echo "($(du -sh --apparent-size $(readlink ${IMAGE_LINK_NAME}.squashfs ) | cut -d'M' -f1)+1)/1"|bc)
	echo "Detected rootfs size: ${rootfs_Msize}MB"
	cd ${DEPLOY_DIR_IMAGE}> /dev/null
	if [ "$(echo ${FLASHLOADER_CONFIG_NAME}|grep 'mmc')" = "" ];then
		#NAND/SQI configuration

		if [ -e uImage-initramfs-${MACHINE}.bin ]; then
			kernel_Msize=$(echo "($(du -sh --apparent-size $(readlink uImage-initramfs-${MACHINE}.bin) | cut -d'M' -f1)+1)/1"|bc)
			echo "initramfs partition size detected: "$kernel_Msize"MB"
		else
			#File is not here ... Let's use a default value
			echo "Using default value for initramfs partition size (${INITRAMFS_DEFAULT_SIZE}MB)"
			kernel_Msize=${INITRAMFS_DEFAULT_SIZE}
		fi
		echo "Patching flashloader configuration for SQI/NAND"
		sed -i "s#-(rootfs|#${rootfs_Msize}m(lower|#" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
		sed -i "s?###GENERATED_IMAGE_NAME###.*?###GENERATED_IMAGE_NAME###.squashfs),-(overlay|overlay)?" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
		#Change the kernel uImage configuration to change initramfs one
		sed -i "s#\(AP_OS.*= uImage\)#\1-initramfs-${MACHINE}.bin#" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
		sed -i "s#,[0-9]*m(AP_OS)#,${kernel_Msize}m(AP_OS)#" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	else
		#MMC configuration
		echo "Using default value for initramfs partition size (${BOOTFS_DEFAULT_SIZE}MB)"
		kernel_Msize=${BOOTFS_DEFAULT_SIZE}
		if [ "$(grep 'overlay =' ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME})" = "" ]; then
			sed -i 's#.*boot = bootfs.ext4.*#overlay = overlay\n&#' ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
		fi
		sed -i "s?###GENERATED_IMAGE_NAME###.*?###GENERATED_IMAGE_NAME###.squashfs?" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
		sed -i "s#name=rootfs,size=-#name=rootfs,size=${rootfs_Msize}MiB#" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
		sed -i "s#uuid=\${uuid_gpt_rootfs}\$#uuid=\${uuid_gpt_rootfs};name=overlay,size=-,uuid=\${uuid_gpt_rootfs}#" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	fi
}

patch_flashloader_main () {
	FLASHLOADER_CONFIG_NAME=$(basename ${FLASHLOADER_CONFIG})
	UBOOT_ENV_CONFIG_NAME=$(basename ${UBOOT_ENV_CONFIG})
	SOC=$(echo ${SOC_FAMILY} | sed "s?.*:\(.*\)?\1?g")
	sed -i "s?###SOC###?${SOC}?g" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	sed -i "s?###GENERATED_IMAGE_NAME###?${IMAGE_LINK_NAME}?g" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	sed -i "s?###DEVICETREE_NAME###?${KERNEL_DEVICETREE}?g" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	sed -i "s?###UBOOT_ENV_CONFIG###?${UBOOT_ENV_CONFIG_NAME}?g" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}

	#Configure the AP_XL Shadowing
	if [ "$(grep '###AP_XL_BASE###' ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME})" != "" ]; then
		MEM_MAP="${STAGING_INCDIR}/sta_mem_map/sta_mem_map.h"
		ESRAM_A7_BASE=$(grep "#define ESRAM_A7_BASE" ${MEM_MAP} | cut -d" " -f3)
		AP_XL_ADDR="$(sed -n "/^#define [^(]*${AP_XL_NAME}.*/p" ${MEM_MAP} | sed 's?.*\(0x[0-9a-z]*\).*?\1?')"
		case "${AP_XL_ADDR}" in
			"")
				bberror "${AP_XL_NAME} can't be found in global memory mapping"
				break
				;;
			"${ESRAM_A7_BASE}")
				#A7 boot binary is not using the m3 trampoline code
				sed -i "s?###AP_XL_BASE###?${AP_XL_ADDR}?g" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
				bbnote "${AP_XL_NAME} @${AP_XL_ADDR} is used as AP_XL"
				break
				;;
			*)
				if [ $(echo "${AP_XL_ADDR}" | wc -l) -ne 1 ]; then
					bberror "Cannot figure out the AP_XL shadowing address -- Multiple ${AP_XL_NAME} definition found in memory mapping, should be unique"
					return -1
				fi
				#A7 boot binary is using the m3 trampoline code
				bbnote "The AP_XL is ${AP_XL_NAME} @${AP_XL_ADDR} -- Adding trampoline @${ESRAM_A7_BASE}"
				sed -i "s?###AP_XL_BASE###?${AP_XL_ADDR}\nAP_XL_ENTRY\t\t= ${AP_XL_ADDR}?g" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
				break
				;;
		esac
	fi
	if [ "${SPLASH_CONFIG}" != "" ]; then
		install -m644 ${SPLASH_CONFIG} ${DEPLOY_DIR_IMAGE}/splash.rgb
	fi
	if [ "${SPLASH_ANIMATION_CONFIG}" != "" ]; then
		install -m644 ${SPLASH_ANIMATION_CONFIG} ${DEPLOY_DIR_IMAGE}/splash_animation.mov
	fi

	if [ "${M3_XL_APPEND}" != "" -a "$(grep ^M3_XL ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME} | grep '${M3_XL_APPEND}')" = "" ]; then
		sed -i '/^M3_XL/ s/$/${M3_XL_APPEND}/' ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	fi

	# Do no quit on error, errors are when variable are unset
	set +e
	#Build the static configuration list and replace the pattern in the conf file
	for CONF in ${ENTRY_LIST}; do
		SHORT_NAME=$(echo ${CONF} | sed -e "s#_ENTRY_##")
		ENTRY_SOC_NAME="${CONF}${SOC}"
		ENTRY_DEFAULT_NAME="${CONF}DEFAULT"
		eval "export val=\"\$$ENTRY_SOC_NAME\""
		set -u
		#Test if variable has been set, even to null -- need the "set -u"
		test=$(eval "\$$ENTRY_SOC_NAME" 2>/dev/null)
		if [ $? -eq 1 ]; then
			eval "export val=\"\$$ENTRY_DEFAULT_NAME\"" 2>/dev/null
		fi
		set +u
		echo "Declaring $SHORT_NAME = ${val}"
		export $SHORT_NAME="${val}"
		sed -i "s?###${SHORT_NAME}###?${val}?" ${DEPLOY_DIR_IMAGE}/${FLASHLOADER_CONFIG_NAME}
	done
	set -e
}


python __anonymous() {
    initramfs_image_name = d.getVar("INITRAMFS_IMAGE", True)
    image_fstypes = d.getVar("IMAGE_FSTYPES", True)
    initramfs_fstypes = d.getVar("INITRAMFS_FSTYPES", True)

    if initramfs_image_name is not None:
        if initramfs_fstypes !=  image_fstypes:
            bb.build.addtask('do_patch_flashloader_config', 'do_build', 'virtual/kernel:do_bundle_initramfs do_image_complete', d)
    else:
        bb.build.addtask('do_patch_flashloader_config', 'do_build', 'virtual/kernel:do_deploy do_image_complete', d)

    d.setVar("AP_XL_NAME", bb.utils.contains('MACHINE_FEATURES', 'atf', 'BL1_BL2_BASE', 'ESRAM_A7_BASE', d))
    d.setVar("M3_XL_APPEND", bb.utils.contains('MACHINE_FEATURES', 'ChainOfTrust', ', st-CoT', '', d))
}
