DESCRIPTION = "ST-ETAL - ST Tuner - etalcoredemo"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

ETAL_SOURCE_PATH = "${S}"

inherit externalsrc
EXTERNALSRC_pn-st-etal-coredemo = "${ST_LOCAL_SRC}/st-etal/latest-etal/"

inherit pkgconfig

BUILD_DIR = "${ETAL_SOURCE_PATH}/target/linux"


ST_ETAL_CORE_DEMO_DIR = "demo/etalcoredemo"

DEFINES += "-DCONFIG_BOARD_ACCORDO5 -DHAVE_FM "
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB', '-DHAVE_DAB', '', d)}"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'HD', '-DHAVE_HD', '', d)}"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-DHAVE_DAB', '', d)}"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-DHAVE_DAB_SD', '', d)}"

DEPENDS = "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', 'alsa-lib', '', d)}"
CC += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"
CXX += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"

do_compile() {
	# clean the object with current target_configuration
	# (objects depend on target_config.mak)	
	if [ "${ETAL_DEV_CLEAN_OPTION}" = "NO_CLEAN" ] ; then
	   echo "no clean..."
	else
	   make -C ${ETAL_SOURCE_PATH}/target/linux clean
	fi

	if [ "${ETAL_DEV_TARGET_H_OPTION}" = "NO_RECOPY" ] ; then
		echo "no recopy for target_config.h and target_config.mak"
	else
		if [ "${ETAL_OPTION}" = "DAB" ] ; then
			cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_dab ${BUILD_DIR}/target_config.h
			cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_dab ${BUILD_DIR}/target_config.mak
		elif [ "${ETAL_OPTION}" = "HD" ] ; then
			cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_hd ${BUILD_DIR}/target_config.h
			cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_hd ${BUILD_DIR}/target_config.mak
		elif [ "${ETAL_OPTION}" = "DAB_HOST_SD" ] ; then
			cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_dab_host_sd ${BUILD_DIR}/target_config.h
			cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_dab_host_sd ${BUILD_DIR}/target_config.mak
		fi
	fi
	# clean the objects with the new target_configuration
	# (objects depend on target_config.mak)	
	if [ "${ETAL_DEV_CLEAN_OPTION}" = "NO_CLEAN" ] ; then
	   echo "no clean..."
	else
	   make -C ${ETAL_SOURCE_PATH}/target/linux clean
	fi
	make -C ${ETAL_SOURCE_PATH}/target/linux all CC="${CC}" CXX="${CXX}" LD="${CC}" AR="${AR}" DEFINES="${DEFINES}"
	# now compile the application and link it with etal 
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR} clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR} all CC="${CC}" LD="${CC}" DEFINES="${DEFINES}"
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR}/etalcoredemo ${D}/${bindir}
}

do_configure[noexec] = "1"
