inherit pkgconfig deploy

BUILD_DIR = "${S}/target/linux"

ST_ETAL_DEMO_DIR = "demo/etaldemo_Application"
ST_ETAL_CORE_DEMO_DIR = "demo/etalcoredemo"
ST_ETAL_FWM_DIR = "tuner_driver/exports/firmware"

DEFINES += "-DCONFIG_BOARD_ACCORDO5 -DHAVE_FM -DHAVE_AM "
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB', '-DHAVE_DAB', '', d)}"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'HD', '-DHAVE_HD', '', d)}"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-DHAVE_DAB', '', d)}"
DEFINES += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-DHAVE_DAB_SD', '', d)}"

DEPENDS = "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', 'alsa-lib', '', d)}"
CC += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"
CXX += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"

do_compile() {
	# clean ETAL library with current target_config
	# because objects depends on target_config

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

	# Compile the ETAL library

	# clean ETAL library with target_config
	# because objects depends on target_config

	if [ "${ETAL_DEV_CLEAN_OPTION}" = "NO_CLEAN" ] ; then
		echo "no clean..."
	else
		make -C ${ETAL_SOURCE_PATH}/target/linux clean
	fi

	make -C ${ETAL_SOURCE_PATH}/target/linux all CC="${CC}" CXX="${CXX}" LD="${CC}" AR="${AR}" DEFINES="${DEFINES}"

	# Compile the etal demo : etaldemoSeekorTune
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DEMO_DIR} clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DEMO_DIR} all CC="${CC}" LD="${CC}" DEFINES="${DEFINES}"

	# Compile the etal demo : etalcoredemo
  	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR} clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR} all CC="${CC}" LD="${CC}" DEFINES="${DEFINES}"
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_DEMO_DIR}/etaldemo_SeekOrTune ${D}/${bindir}
	install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR}/etalcoredemo ${D}/${bindir}
	install -m 0644 ${ETAL_SOURCE_PATH}/${ST_ETAL_FWM_DIR}/TDA7707_OM_CUT_*.boot ${D}/${bindir}/
	if [ "${ETAL_CMOST_VERSION}" != "" ] ; then
		install -m 0644 ${ETAL_SOURCE_PATH}/${ST_ETAL_FWM_DIR}/TDA7707_OM_CUT_${ETAL_CMOST_VERSION}.boot ${D}/${bindir}/TDA7707_OM_CUT.boot
	fi
}

#
# Redefine buildclean function used by externalsrc class.
# It is required to avoid crash using devtool
# else 'devtool reset' is calling make clean at etal source root dir which is failing for ETAL
# in etal make clean needs to be call in target/linux subdir
#
do_buildclean() {
	make -C ${ETAL_SOURCE_PATH}/target/linux clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_DEMO_DIR} clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_CORE_DEMO_DIR} clean
}

do_deploy() {
	install -d ${DEPLOYDIR}
	install -m 0644 ${ETAL_SOURCE_PATH}/${ST_ETAL_FWM_DIR}/TDA7707_OM_CUT_${ETAL_CMOST_VERSION}.boot.bin ${DEPLOYDIR}/tuner_fwm.bin
}

addtask deploy before do_build after do_install

do_configure[noexec] = "1"

