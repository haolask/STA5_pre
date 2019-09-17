DEPENDS+=" linux-dt-overlay bc-native"

BOOTFS_NAME = "bootfs-${MACHINE}-${DATETIME}.${BOOTFS_FILETYPE}"
BOOTFS_NAME[vardepsexclude] = "DATETIME"

do_install_bootfs() {
	DTB_SYMLINK_NAME="${KERNEL_IMAGETYPE}-${KERNEL_DEVICETREE}"
	BOOTFS_MOUNT_POINT=$(mktemp -d)

	if [ -d ${STAGING_DIR_TARGET}/linux-dt-overlay ]; then
		if [ "$(ls ${STAGING_DIR_TARGET}/linux-dt-overlay | wc -l)" != "0" ]; then
			bbnote "Insert DeviceTree Overlay"
			mkdir ${BOOTFS_MOUNT_POINT}/dtbo
			cp ${STAGING_DIR_TARGET}/linux-dt-overlay/*.dtbo ${BOOTFS_MOUNT_POINT}/dtbo
		fi
	fi

	cp  ${DEPLOYDIR}/fitImage ${BOOTFS_MOUNT_POINT}

	if [ -z ${INITRAMFS_IMAGE} ]; then
		cp ${DEPLOYDIR}/${KERNEL_IMAGETYPE} ${BOOTFS_MOUNT_POINT}
	else
		cp ${DEPLOYDIR}/${KERNEL_IMAGETYPE}-initramfs-${MACHINE}.bin ${BOOTFS_MOUNT_POINT}/${KERNEL_IMAGETYPE}
		cd ${DEPLOYDIR}
		ln -sf $(readlink ${KERNEL_IMAGETYPE}-initramfs-${MACHINE}.bin) ${DEPLOYDIR}/${KERNEL_IMAGETYPE}-initramfs
		cd -
	fi

	#Evaluate the required bootfs size
	BOOTFS_SIZE="16384"
	bbnote "Generating ${BOOTFS_FILETYPE} boot imagefile of ${BOOTFS_SIZE} Kbytes"

	if [ "${BOOTFS_FILETYPE}" = "ubifs" ]; then
		#Calculating the bootfs bloc count to reserve
		if [ -z ${BLOCK_SIZE} ]; then
			bberror "BLOCK_SIZE is undefined !"
		fi
		BLOC_COUNT=$(echo "((${BOOTFS_SIZE}/${BLOCK_SIZE})+1)/1" | bc)
		echo "Calculated bloc count = $BLOC_COUNT"
		#set to 16MB max if smaller
		if [ $BLOC_COUNT -lt $(echo "(16*1024)/${BLOCK_SIZE}"|bc) ]; then
			BLOC_COUNT=$(echo "(16*1024)/${BLOCK_SIZE}"|bc)
		fi

		bbnote "mkfs.ubifs -r ${BOOTFS_MOUNT_POINT} -o ${DEPLOYDIR}/bootfs.${BOOTFS_FILETYPE}  ${MKUBIFS_COMMON_ARGS} -c ${BLOC_COUNT}"
		mkfs.ubifs -r ${BOOTFS_MOUNT_POINT} -o ${DEPLOYDIR}/${BOOTFS_NAME} ${MKUBIFS_COMMON_ARGS} -c ${BLOC_COUNT}
	else if [ "${BOOTFS_FILETYPE}" = "ext4" ]; then
			mkext234fs ${BOOTFS_FILETYPE} -i 4096 -O ^has_journal
		else
			bberror "Image tool for bootfs can\'t be selected"
		fi
	fi

	echo "Bootfs contents:"
	ls -l ${BOOTFS_MOUNT_POINT}
	#rm -rf ${BOOTFS_MOUNT_POINT}

	rm -f ${DEPLOYDIR}/bootfs.${BOOTFS_FILETYPE}
	ln -sf ${BOOTFS_NAME} ${DEPLOYDIR}/bootfs.${BOOTFS_FILETYPE}

}

FILES_kernel-base += "/lib/firmware/st/dspfw"

do_install_append() {
	if [ ! -z "${ST_DSP_FW_SUBPATH}" ]; then
		install -d ${D}/lib/firmware/st/dspfw
		for dsp_fw in ${S}/${ST_DSP_FW_SUBPATH}/*; do
			echo "Installing $dsp_fw file in rootfs"
			install -m 0666 $dsp_fw ${D}/lib/firmware/st/dspfw
			install -m 0666 $dsp_fw ${DEPLOYDIR}
		done
	fi
}

do_install_dsp() {
	#Install DSP FW file in deploy folder
	if [ ! -z "${ST_DSP_FW}" ]; then
		for dsp_fw in ${ST_DSP_FW}; do
			echo "Installing $dsp_fw file"
			install -m 0666 ${S}/${ST_DSP_FW_SUBPATH}/$dsp_fw ${DEPLOYDIR}
		done
	fi
}

do_deploy_append() {
	install -d ${DEPLOYDIR}

	if [ "${@bb.utils.contains('MACHINE_FEATURES', 'no_uboot', 'no_uboot', '', d)}" = "" ]; then
		do_initramfs_configure
		#install the kernel and DT files in a bootfs partition
		do_install_bootfs
	fi

	#Install DSP FW files file in deploy folder
	do_install_dsp

	install -m 0644 ${B}/vmlinux ${DEPLOYDIR}
}

mkext234fs() {
	fstype=$1
	extra_imagecmd=""

	if [ $# -gt 1 ]; then
		shift
		extra_imagecmd=$@
	fi

	# If generating an empty image the size of the sparse block should be large
	# enough to allocate an ext4 filesystem using 4096 bytes per inode, this is
	# about 60K, so dd needs a minimum count of 60, with bs=1024 (bytes per IO)
	eval local COUNT=\"0\"
	eval local MIN_COUNT=\"60\"
	if [ $BOOTFS_SIZE -lt $MIN_COUNT ]; then
		eval COUNT=\"$MIN_COUNT\"
	fi
	# Create a sparse image block
	mkdir -p ${DEPLOYDIR}
	echo "dd if=/dev/zero of=${DEPLOYDIR}/${BOOTFS_NAME} seek=$BOOTFS_SIZE count=$COUNT bs=1024"
	dd if=/dev/zero of=${DEPLOYDIR}/${BOOTFS_NAME} seek=$BOOTFS_SIZE count=$COUNT bs=1024
	echo "mkfs.$fstype -F $extra_imagecmd ${DEPLOYDIR}/${BOOTFS_NAME} -d ${BOOTFS_MOUNT_POINT}"
	mkfs.$fstype -F $extra_imagecmd ${DEPLOYDIR}/${BOOTFS_NAME} -d ${BOOTFS_MOUNT_POINT}
}


do_initramfs_configure () {
	if [ ! -z "${INITRAMFS_IMAGE}" -a x"${INITRAMFS_IMAGE_BUNDLE}" = x1 ]; then
		set +e
		IMAGE_FLASHLOADER_CONFIG_NAME_LIST="$(grep -sl overlay ${DEPLOY_DIR_IMAGE}/config-*.txt)"
		set -e
		for IMAGE_FLASHLOADER_CONFIG_NAME in ${IMAGE_FLASHLOADER_CONFIG_NAME_LIST}; do
			if [ -e "${IMAGE_FLASHLOADER_CONFIG_NAME}" ]; then
				echo "Updating flashloader configuration for ${IMAGE_FLASHLOADER_CONFIG_NAME}"
				cd ${DEPLOYDIR}
				if [ "$(echo ${FLASHLOADER_CONFIG_NAME}|grep 'mmc')" != "" ];then
					if [ -e uImage-initramfs-${MACHINE}.bin ]; then
						kernel_Msize=$(echo "($(du -sh --apparent-size $(readlink uImage-initramfs-${MACHINE}.bin) | cut -d'M' -f1)+1)/1"|bc)
						echo "initramfs partition size detected: "$kernel_Msize"MB"
					else
						#File is not here ... Let's use a default value
						echo "Using default value for initramfs partition size (${INITRAMFS_DEFAULT_SIZE}MB)"
						kernel_Msize=${INITRAMFS_DEFAULT_SIZE}
					fi
					cd -
					#Only size update is required here
					sed -i "s#:[0-9]*m(lower|#:${kernel_Msize}m(lower|#" ${IMAGE_FLASHLOADER_CONFIG_NAME}
					sed -i "s#,[0-9]*m(AP_OS)#,${kernel_Msize}m(AP_OS)#" ${IMAGE_FLASHLOADER_CONFIG_NAME}
				else
					if [ -e bootfs.ext4 ]; then
						kernel_Msize=$(echo "($(du -sh --apparent-size $(readlink bootfs.ext4) | cut -d'M' -f1)+1)/1"|bc)
						echo "bootfs partition size detected: "$kernel_Msize"MB"
					else
						#File is not here ... Let's use a default value
						echo "Using default value for initramfs partition size (${BOOTFS_DEFAULT_SIZE}MB)"
						kernel_Msize=${BOOTFS_DEFAULT_SIZE}
					fi
					cd -

					#Only size update is required here
					sed -i "s#name=boot,start=\([0-9]*\)MiB,size=[0-9]*MiB#name=boot,start=\1MiB,size=${kernel_Msize}MiB#" ${IMAGE_FLASHLOADER_CONFIG_NAME}
				fi
			fi
		done
	fi
}


#Add the correct deps to tools helping the bootfs image build
python __anonymous() {
    d.appendVar("DEPENDS", bb.utils.contains('UBOOT_CONFIG', 'MMC', ' e2fsprogs-native', ' mtd-utils-native', d))
    d.setVar('BOOTFS_FILETYPE', bb.utils.contains_any('UBOOT_CONFIG', 'MMC SQI_MMC', 'ext4', 'ubifs', d))
}
