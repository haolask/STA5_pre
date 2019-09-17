DESCRIPTION = "DT Overlay"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

DEPENDS += "dtc-native"

inherit externalsrc

DT_DIR_NAME = "linux-dt-overlay"
DT_OUT_DIR_NAME = "linux-dt-overlay-output"
SYSROOT_DIRS += " /${DT_DIR_NAME}"
FILES_${PN}-dev = "/${DT_DIR_NAME}/*"


EXTERNALSRC_pn-linux-dt-overlay ?= "${ST_LOCAL_SRC}/${DT_DIR_NAME}"


do_compile () {
	rm -rf ${WORKDIR}/${DT_OUT_DIR_NAME}
	mkdir -p ${WORKDIR}/${DT_OUT_DIR_NAME}
	dts_overlay_file_list=$(ls ${S}/*.dt*)
	for full_dtb in ${dts_overlay_file_list}; do
		dtb=$(basename $full_dtb)
		dtbo=$(echo $dtb | sed 's/dts$/dtbo/')
		bbnote "Build DeviceTree Overlay ${dtb} => ${dtbo}"
		dtc -O dtb -o ${WORKDIR}/${DT_OUT_DIR_NAME}/${dtbo} -b 0 -@ ${S}/${dtb}
	done
}

do_install () {
    rm -rf ${D}/${DT_DIR_NAME}
    SOC=$(echo ${SOC_FAMILY} | sed "s?.*:\(.*\)?\1?g")
    if [ "$(ls ${WORKDIR}/${DT_OUT_DIR_NAME}/ | grep "${SOC}" | grep "\.dtbo$" | wc -l)" != "0" ]; then
	mkdir -p ${D}/${DT_DIR_NAME}
	cp ${WORKDIR}/${DT_OUT_DIR_NAME}/*${SOC}*.dtbo ${D}/${DT_DIR_NAME}
    fi
}

do_build[depends] += "virtual/kernel:do_deploy"
