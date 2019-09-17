
do_install_basefilesissue () {
	if [ "${hostname}" ]; then
		echo ${hostname} > ${D}${sysconfdir}/hostname
	fi

	install -m 644 ${WORKDIR}/issue*  ${D}${sysconfdir}
		if [ -n "${DISTRO_NAME}" ]; then
		printf "${DISTRO_NAME} " >> ${D}${sysconfdir}/issue
		printf "${DISTRO_NAME} " >> ${D}${sysconfdir}/issue.net
		if [ -n "${DISTRO_VERSION}" ]; then
			printf "${DISTRO_VERSION} " >> ${D}${sysconfdir}/issue
			printf "${DISTRO_VERSION} " >> ${D}${sysconfdir}/issue.net
		fi
		printf "\\\n ${CARPROC_TAG} \\\l\n" >> ${D}${sysconfdir}/issue
		echo >> ${D}${sysconfdir}/issue
		echo "${CARPROC_TAG} %h"    >> ${D}${sysconfdir}/issue.net
		echo >> ${D}${sysconfdir}/issue.net
	fi

	echo "export QT_QPA_FONTDIR=/usr/share/fonts/ttf" >> ${D}${sysconfdir}/profile
}
