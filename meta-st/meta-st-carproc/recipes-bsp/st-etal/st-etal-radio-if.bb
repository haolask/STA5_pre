DESCRIPTION = "ST-ETAL radio_if - ST Tuner"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "r0"
ETAL_SOURCE_PATH = "${S}"

inherit externalsrc
EXTERNALSRC_pn-st-etal-radio-if = "${ST_LOCAL_SRC}/st-etal/latest-etal/"

inherit pkgconfig

BUILD_DIR = "${ETAL_SOURCE_PATH}/target/linux"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

ST_ETAL_APP_RADIO_IF_DIR = "applications/radio_if"

DEFINES += "-DCONFIG_BOARD_ACCORDO5"
DEFINES_RADIO_IF = "${DEFINES} -DCONFIG_APP_RADIO_IF "

DEPENDS = "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', 'alsa-lib', '', d)}"
CC_HOST_SD = "${CC}"
CC_HOST_SD += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"
CXX_HOST_SD = "${CXX}"
CXX_HOST_SD += "${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs --cflags alsa`", '', d)}"

do_compile() {
	# clean radio_if_dab and radio_if_hdr build
	rm -f ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_dab
	rm -f ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_hdr
	rm -f ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_dab_host_sd
	make -C ${ETAL_SOURCE_PATH}/target/linux clean

	# build etal library DAB
	# always built it even if not in DAB environment selected
	# this is useful for test : GUI access to board thru A5
	#
	cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_dab_direct ${BUILD_DIR}/target_config.h
	cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_dab_direct ${BUILD_DIR}/target_config.mak
	make -C ${ETAL_SOURCE_PATH}/target/linux clean
	make -C ${ETAL_SOURCE_PATH}/target/linux all CC="${CC}" CXX="${CXX}" AR="${AR}" DEFINES="${DEFINES}"
	# build radio_if_dab application
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR} clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR} all CC="${CC}" CXX="${CXX}" LD="${CC}" DEFINES="${DEFINES_RADIO_IF}"
	mv ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_dab

	# build etal library HD Radio
	# always built it even if not in HD environment selected
	# this is useful for test : GUI access to board thru A5
	#
	cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_hd ${BUILD_DIR}/target_config.h
	cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_hd ${BUILD_DIR}/target_config.mak
	make -C ${ETAL_SOURCE_PATH}/target/linux clean
	make -C ${ETAL_SOURCE_PATH}/target/linux all CC="${CC}" CXX="${CXX}" AR="${AR}" DEFINES="${DEFINES}"
	# build radio_if_hdr application
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR} clean
	make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR} all CC="${CC}" CXX="${CXX}" LD="${CC}" DEFINES="${DEFINES_RADIO_IF}"
	mv ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_hdr

	# build etal library DAB_HOST_SD
	# compile it only if DAB_HOST_SD is requested
	#
	if [ "${ETAL_OPTION}" = "DAB_HOST_SD" ] ; then
	   cp -rf ${BUILD_DIR}/a5_configs/target_config.h.mtd_dab_direct_host_sd ${BUILD_DIR}/target_config.h
	   cp -rf ${BUILD_DIR}/a5_configs/target_config.mak.mtd_dab_direct_host_sd ${BUILD_DIR}/target_config.mak
	   make -C ${ETAL_SOURCE_PATH}/target/linux clean
	   make -C ${ETAL_SOURCE_PATH}/target/linux all CC="${CC}" CXX="${CXX}" AR="${AR}" DEFINES="${DEFINES}"
	   # build radio_if_dab_host_sd application
	   make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR} clean
	   make -C ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR} all CC="${CC_HOST_SD}" CXX="${CXX_HOST_SD}" LD="${CC_HOST_SD}" DEFINES="${DEFINES_RADIO_IF} -DHAVE_DAB_SD"
	   mv ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_dab_host_sd
	fi
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_dab ${D}/${bindir}/
	install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_hdr ${D}/${bindir}/
	if [ "${ETAL_OPTION}" = "DAB_HOST_SD" ] ; then
		install -m 0755 ${ETAL_SOURCE_PATH}/${ST_ETAL_APP_RADIO_IF_DIR}/radio_if_dab_host_sd ${D}/${bindir}/
	fi
}

do_configure[noexec] = "1"
