
TELEMATIC_LOGS = 'local0.*        /var/telematics-debug.log'

do_install_prepend () {
    if grep -q "CONFIG_SYSLOGD=y" ${B}/.config; then
	echo '${TELEMATIC_LOGS}' >> ${WORKDIR}/syslog.conf
    fi
}
