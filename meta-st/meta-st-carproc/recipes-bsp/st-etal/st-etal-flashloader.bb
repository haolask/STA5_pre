DESCRIPTION = "ST-ETAL embedded DCOP flashloader - ST Tuner"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "r0"
ETAL_SOURCE_PATH = "${S}"

inherit externalsrc
EXTERNALSRC_pn-st-etal-flashloader = "${ST_LOCAL_SRC}st-etal/latest-etal"

inherit pkgconfig

BUILD_DIR = "${S}/target/linux"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

ST_ETAL_DCOP_FLASH_DIR = "applications/etalDcopMdrFlash"
ST_ETAL_DCOP_HD_FLASH_DIR = "applications/etalDcopHdrFlash"

DEFINES += "-DCONFIG_BOARD_ACCORDO5"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-DHAVE_DAB_SD', '', d)}"
# use this compilation switch when etal is build with compilation switch CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DL_FILE_MODE or CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
#DEFINES += "-DCONFIG_ETAL_DCOP_FLASH_FIRMWARE_DL_FILE_MODE"

DEPENDS = "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', 'alsa-lib', '', d)}"
CC += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"
CXX += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"

do_compile() {
	make -C ${ETAL_SOURCE_PATH}/target/linux clean
    	if [ "${ETAL_DEV_TARGET_H_OPTION}" = "NO_RECOPY" ] ; then
		echo "no recopy for target_config.h  and target_config.mak"
	else
		if [ "${ETAL_OPTION}" = "DAB" ] ; then
			cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_dab_flash ${BUILD_DIR}/target_config.h
			cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_dab_flash ${BUILD_DIR}/target_config.mak
		elif [ "${ETAL_OPTION}" = "HD" ] ; then
			cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_hd_flash ${BUILD_DIR}/target_config.h
			cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_hd_flash ${BUILD_DIR}/target_config.mak
		elif [ "${ETAL_OPTION}" = "DAB_HOST_SD" ] ; then
			cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_dab_flash_host_sd ${BUILD_DIR}/target_config.h
			cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_dab_flash_host_sd ${BUILD_DIR}/target_config.mak
		fi
	fi

	make -C ${ETAL_SOURCE_PATH}/target/linux clean
	make -C ${ETAL_SOURCE_PATH}/target/linux all CC="${CC}" CXX="${CXX}" LD="${CC}" AR="${AR}" DEFINES="${DEFINES}"
	if [[ "${ETAL_OPTION}" = "DAB" || "${ETAL_OPTION}" = "DAB_HOST_SD" ]] ; then
		make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_FLASH_DIR} clean
		make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_FLASH_DIR} all CC="${CC}" LD="${CC}" DEFINES="${DEFINES}"
	else
		if [ "${ETAL_OPTION}" = "HD" ] ; then
			make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_HD_FLASH_DIR} clean
			make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_HD_FLASH_DIR} all CC="${CC}" LD="${CC}" DEFINES="${DEFINES}"
		fi
	fi
}

do_install() {
	install -d ${D}${bindir}
	install -d ${D}${bindir}/DCOP_flash
	if [[ "${ETAL_OPTION}" = "DAB" || "${ETAL_OPTION}" = "DAB_HOST_SD" ]] ; then
		install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_FLASH_DIR}/etalDcopMdrFlash ${D}/${bindir}/DCOP_flash
		install -m 0444 ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_FLASH_DIR}/*.bin ${D}/${bindir}/DCOP_flash
	else
		if [ "${ETAL_OPTION}" = "HD" ] ; then
			install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_HD_FLASH_DIR}/etalDcopHdrFlash ${D}/${bindir}/DCOP_flash
			install -m 0444 ${ETAL_SOURCE_PATH}/${ST_ETAL_DCOP_HD_FLASH_DIR}/*.bin ${D}/${bindir}/DCOP_flash
		fi
	fi

}

do_configure[noexec] = "1"
