

do_patch_uboot_env () {
	#Should be only patched when building main image
	if [ "${INITRAMFS_IMAGE}" != "${IMAGE_BASENAME}" ]; then
		UBOOT_ENV_CONFIG_NAME=$(basename ${UBOOT_ENV_CONFIG})
		install -d ${DEPLOY_DIR_IMAGE}
		install -m644 ${UBOOT_ENV_CONFIG} ${DEPLOY_DIR_IMAGE}
		if [ "${INITRAMFS_IMAGE}" != "" ]; then
			#Update the command line to boot on the initramfs if it is enabled
			sed -i 's#rootfstype=[^ ]*#rootfstype=ramfs#g' ${DEPLOY_DIR_IMAGE}/${UBOOT_ENV_CONFIG_NAME}
			echo "Patching cmdline for initramfs"
		fi
	fi
}


python do_nfs_rootfs_config () {
    if d.getVar('ROOTFS_DEVICE', True) == "NFS":
        readme_file = open(d.getVar('DEPLOY_DIR_IMAGE',True) + "/NFS_README.txt", "w")
        readme_file.write("Execute "+ d.getVar('OEROOT',True) +"/meta-st/meta-st-carproc/scripts/rootfs_init.sh to generate your NFS filesystem\n")
        readme_file.write("Please set your board mac@ in uboot (XX XX is your board ID):\n")
        readme_file.write("\tsetenv eth_mac_addr 00 80 E1 A5 XX XX\n")
        readme_file.write("\tsaveenv\n")
        bb.warn("=============================================================================================================================")
        bb.warn("NFS image generated, please run " + d.getVar('OEROOT',True) + "/meta-st/meta-st-carproc/scripts/rootfs_init.sh BEFORE flashing")
        bb.warn("Read "+ d.getVar('DEPLOY_DIR_IMAGE',True) + "/NFS_README.txt for details")
        bb.warn("=============================================================================================================================")
        readme_file.close()
}


do_generate_flashing_package () {
	if [ ! -z "${SIGN_BINARIES}" ]; then
		if [ -e ${OEROOT}/meta-st/meta-st-carproc/scripts/sign_binaries.sh ]; then
			${OEROOT}/meta-st/meta-st-carproc/scripts/sign_binaries.sh
		fi
	fi

	if [ ! -z "${GENERATE_FLASHING_PACKAGE}" ]; then

		tmpworkdir=$(mktemp -d)
		tmpFlashingPkgDir=$tmpworkdir/${CARPROC_TAG}_${IMAGE_LINK_NAME}
		FLASHING_PACKAGE_NAME=${DEPLOY_DIR_IMAGE}/flashloader_image_${CARPROC_TAG}_${IMAGE_LINK_NAME}.tar
		rm -f $FLASHING_PACKAGE_NAME
		rm -rf $tmpFlashingPkgDir
		mkdir -p $tmpFlashingPkgDir
		#filter out undeed files
		cp -rL ${DEPLOY_DIR_IMAGE}/* $tmpFlashingPkgDir
		ls $tmpFlashingPkgDir/* | grep '[0-9]\{14\}' | xargs rm -f
		rm -f $tmpFlashingPkgDir/modules-*
		ls $tmpFlashingPkgDir/u-boot* | grep -v -e "u-boot*.bin$" -e "u-boot-flashloader.bin$" | xargs rm -f
		ls $tmpFlashingPkgDir/uImage* | grep -v -e "uImage$" -e "\.dtb$" | xargs rm -f
		ls $tmpFlashingPkgDir/m3_xl* | grep -v '.bin' | xargs rm -f
		cd $tmpworkdir
		echo "============== Package contents ========================"
		ls -A ${tmpFlashingPkgDir}
		echo "========================================================"
		tar cf $FLASHING_PACKAGE_NAME ${CARPROC_TAG}_${IMAGE_LINK_NAME}
		cd -
		rm -rf $tmpworkdir
	fi
	install -d ${DEPLOY_DIR}/images
	cp -f ${OEROOT}/meta-st/meta-st-carproc/scripts/sta_extract_deploy.sh ${DEPLOY_DIR}/images/sta_extract_deploy.sh
}


do_add_overlay_partition_file () {
	if [ "${INITRAMFS_IMAGE}" != "" ]; then
		#Create the overlay partition file, containg the overlayfs (will be formatted at first boot)
		echo "overlay" > ${IMGDEPLOYDIR}/overlay
	fi
}


do_generate_env_debug () {
	ENV_DEBUG_FILE=${IMGDEPLOYDIR}/debug/env_debug_image.txt
	mkdir -p ${IMGDEPLOYDIR}/debug

	echo "STAGING_DIR_NATIVE=${STAGING_DIR_NATIVE}" > ${ENV_DEBUG_FILE}
	echo "STA_CROSS_COMPILE=${TARGET_PREFIX}" >> ${ENV_DEBUG_FILE}
	echo "STA_M3_CROSS_COMPILE=${HOST_PREFIX_toolchain-bare-metal}" >> ${ENV_DEBUG_FILE}
	echo "STA_DEPLOY_DIR_IMAGE=${DEPLOY_DIR_IMAGE}" >> ${ENV_DEBUG_FILE}
	echo "KERNEL_DEVICETREE=${KERNEL_DEVICETREE}" >> ${ENV_DEBUG_FILE}
	echo "KERNEL_IMAGETYPE=${KERNEL_IMAGETYPE}" >> ${ENV_DEBUG_FILE}
	echo "STA_MACHINE_ARCH=${MACHINE_ARCH}" >> ${ENV_DEBUG_FILE}
	echo "STA_M3_OS=m3_${FEATURE_ENTRY}" >> ${ENV_DEBUG_FILE}
}


IMAGE_PREPROCESS_COMMAND += " do_generate_env_debug;"
IMAGE_POSTPROCESS_COMMAND += " do_patch_uboot_env; do_nfs_rootfs_config; do_add_overlay_partition_file;"

python __anonymous() {
    initramfs_image_name = d.getVar("INITRAMFS_IMAGE", True)
    current_image_name = d.getVar("IMAGE_BASENAME", True)

    if initramfs_image_name is not None:
        if initramfs_image_name != current_image_name:
            bb.build.addtask('do_generate_flashing_package', 'virtual/kernel:do_build', 'do_patch_flashloader_config', d)
    else:
        bb.build.addtask('do_generate_flashing_package', 'do_build', 'do_patch_flashloader_config', d)
}
do_generate_flashing_package[nostamp] = "1"
