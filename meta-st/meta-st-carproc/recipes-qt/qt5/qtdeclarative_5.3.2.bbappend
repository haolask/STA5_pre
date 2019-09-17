do_install_append() {
    # Remove Qt5QmlDevTools.pc
    rm -f ${D}/${libdir}/pkgconfig/Qt5QmlDevTools.pc
}

INSANE_SKIP_${PN}-examples-dev += "dev-elf"
