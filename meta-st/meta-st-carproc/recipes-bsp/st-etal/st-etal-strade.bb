DESCRIPTION = "Basic code for QT compilation testing"
SECTION = "extras"
LICENSE = "GPLv2+ & LGPLv2.1+"
PR = "r0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

inherit systemd externalsrc
EXTERNALSRC_pn-st-etal-strade = "${ST_LOCAL_SRC}st-etal/strade"

inherit pkgconfig

include recipes-qt/qt5/qt5.inc
DEPENDS += "st-etal qtbase qtmultimedia"
RDEPENDS_${PN} += "bash"

FILES_${PN} =  "${bindir}/st_dab_radio"
FILES_${PN} += "${bindir}/st_dab_radio_start.sh"
FILES_${PN} += "${bindir}/radio_logos"
FILES_${PN} += "${bindir}/image_sls"
FILES_${PN} += "${bindir}/Scripts"
FILES_${PN} += "${bindir}/settings"
FILES_${PN} += "${systemd_system_unitdir}/st_dab_radio.service"

SYSTEMD_SERVICE_${PN} = "st_dab_radio.service"
SYSTEMD_AUTO_ENABLE_${PN} = "enable"

EXTRA_QMAKEVARS_PRE += "-d CONFIG+=Etal"
EXTRA_QMAKEVARS_POST += "LIBS+=-L${S}/etal/lib/MTD_LINUX/ LIBS+=-letal LIBS+=-lrt"

EXTRA_QMAKEVARS_POST += "LIBS+=${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', "`pkg-config --libs alsa`", '', d)}"
EXTRA_QMAKEVARS_POST += "LIBS+=${@bb.utils.contains('ETAL_OPTION', 'DAB_HOST_SD', '-lciDec', '', d)}"

#OE_QMAKE_CXXFLAGS += "-I${STAGING_INCDIR}/qt5 -DQT_NO_DEBUG_OUTPUT"
OE_QMAKE_CXXFLAGS += "-I${STAGING_INCDIR}/qt5 -mno-unaligned-access -faligned-new -DCUSTOM_QTDEBUG_LOGFILE"

QMAKE_PROFILES="${S}/st_dab_radio.pro"

do_compile() {
	mkdir -p ${S}/etal/lib/MTD_LINUX
	cd ${S}/etal/lib/MTD_LINUX
	rm -rf libetal.a etal.lib
	cp -rf ${STAGING_DIR_HOST}/etal_exports/etal.a libetal.a
	if [ "${ETAL_OPTION}" = "DAB_HOST_SD" ] ; then	
		cp -rf ${STAGING_DIR_HOST}/etal_exports/libciDec.a libciDec.a
	fi
	cp -rf ${STAGING_DIR_HOST}/etal_exports/*.h ../..
	cp -rf ${STAGING_DIR_HOST}/etal_exports/target_config.h ../../etal_target_config.h
	cp -rf ${STAGING_DIR_HOST}/etal_exports/target_config.mak ../../etal_target_config.mak
	cd -
	oe_runmake
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${B}/st_dab_radio ${D}${bindir}
	install -d ${D}${bindir}/radio_logos
#	install -m 644 ${S}/radio_logos/* ${D}${bindir}/radio_logos
	install -d ${D}${bindir}/image_sls
#	install -m 644 ${S}/image_sls/* ${D}${bindir}/image_sls
	install -m 0755 ${THISDIR}/files/st_dab_radio_start.sh ${D}${bindir}
	install -d ${D}${systemd_system_unitdir}
	install -m 0777 ${THISDIR}/files/st_dab_radio.service ${D}${systemd_system_unitdir}
	install -d ${D}${bindir}/settings
	install -m 644 ${S}/settings/* ${D}${bindir}/settings
	install -d ${D}${bindir}/Scripts
	install -d ${D}${bindir}/Scripts/TDA7707EB_OM_v7.4.2
	install -d ${D}${bindir}/Scripts/TDA7707DA_OM_v6.3.5
	install -m 644 ${S}/Scripts/TDA7707EB_OM_v7.4.2/* ${D}${bindir}/Scripts/TDA7707EB_OM_v7.4.2
	install -m 644 ${S}/Scripts/TDA7707DA_OM_v6.3.5/* ${D}${bindir}/Scripts/TDA7707DA_OM_v6.3.5
#	install -m 644 ${S}/Scripts/* ${D}${bindir}/Scripts
}
