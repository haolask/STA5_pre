FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

# Pulse audio configuration files
SRC_URI += "file://pulse_profile.sh \
            file://0009-Update-pulseaudio.service-to-start-in-system-mode.patch \
            file://0010-dbus-authorize-to-communicate-with-bluez.patch \
            file://0001-deamon-conf-disable-volume-flat.patch \
            "

# Rewrite PACKAGECONFIG for systemd to provide 'systemd_system_unitdir' var instead of 'systemd_user_unitdir' one
PACKAGECONFIG[systemd] = "--enable-systemd-daemon --enable-systemd-login --enable-systemd-journal --with-systemduserunitdir=${systemd_system_unitdir},--disable-systemd-daemon --disable-systemd-login --disable-systemd-journal,systemd"
PACKAGECONFIG_CONFARGS = "--with-system-user=root --with-system-group=root"

# Define systemd package for 'pulseaudio.service'
SYSTEMD_PACKAGES = "${PN}-server"

GROUPADD_PARAM_${PN} = "--system pulse-access;--system pulse"

CACHED_CONFIGUREVARS += " ax_cv_PTHREAD_PRIO_INHERIT=no"

# Pulse audio configuration files installation
do_install_append() {
    install -d ${D}${sysconfdir}/profile.d
    install -m 0644 ${WORKDIR}/pulse_profile.sh ${D}${sysconfdir}/profile.d/
}

FILES_${PN} += "/etc/profile.d"
