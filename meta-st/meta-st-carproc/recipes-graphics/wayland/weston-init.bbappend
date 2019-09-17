FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://utilities-terminal.png \
            file://weston.sh \
            file://weston_profile.sh \
            file://Accordo5_wallpaper_800x480_background.png \
            file://weston \
            "
SRC_URI += " file://weston_${STWESTONSHELL}.ini "

FILES_${PN} += " ${datadir}/weston \
                ${systemd_system_unitdir}/weston.service \
                ${sysconfdir}/etc/profile.d \
                ${sysconfdir}/xdg/weston/weston.ini \
                ${datadir}/weston/backgrounds/Accordo5_wallpaper_800x480_background.png \
                ${sysconfdir}/default/weston \
                ${systemd_system_unitdir}/basic.target.wants/weston.service \
                "

CONFFILES_${PN} += "${sysconfdir}/xdg/weston/weston.ini"

do_install_append() {
    install -d ${D}${sysconfdir}/xdg/weston/
    install -m 0644 ${WORKDIR}/weston_*.ini ${D}${sysconfdir}/xdg/weston/weston.ini

    install -d ${D}${datadir}/weston/backgrounds
    install -m 0644 ${WORKDIR}/Accordo5_wallpaper_800x480_background.png ${D}${datadir}/weston/backgrounds/Accordo5_wallpaper_800x480_background.png

    install -d ${D}${datadir}/weston/icon
    install -m 0644 ${WORKDIR}/utilities-terminal.png ${D}${datadir}/weston/icon/utilities-terminal.png

    install -d ${D}${sysconfdir}/default/
    install -m 0644 ${WORKDIR}/weston ${D}${sysconfdir}/default/weston

    install -d ${D}${systemd_system_unitdir}/basic.target.wants
    ln -sf ${systemd_system_unitdir}/weston.service ${D}${systemd_system_unitdir}/basic.target.wants/weston.service

    install -d ${D}${sysconfdir}/profile.d
    install -m 0755 ${WORKDIR}/weston_profile.sh ${D}${sysconfdir}/profile.d/

    if ${@bb.utils.contains('DISTRO_FEATURES','xwayland','true','false',d)}; then
        # uncomment modules line for support of xwayland
        sed -i -e 's,#modules=xwayland.so,modules=xwayland.so,g' ${D}${sysconfdir}/xdg/weston/weston.ini
    fi
}

clean_weston_xxx_ini() {
  rm -f ${WORKDIR}/weston_*.ini
}

do_fetch[prefuncs] += " clean_weston_xxx_ini "
