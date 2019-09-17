DESCRIPTION = "GNSS TESEO"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "r0"

GNSS_TESEO_PACKAGE_PATH ?= "gnss-teseo"
inherit externalsrc
EXTERNALSRC_pn-gnss-teseo = "${ST_LOCAL_SRC}${GNSS_TESEO_PACKAGE_PATH}"

DR_ENABLED ?= "0"
DR_OPTIONS = "${@bb.utils.contains("DR_ENABLED", "0", "", "-DDR_CODE_LINKED", d)}"

FILES_${PN} = "${bindir}/*"

EXTRA_LDFLAGS = "-lpthread"

do_compile() {
   mkdir -p ${B}/bin
   #${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${DR_OPTIONS} ${S}/test_app/gnss_app_client.c -o ${B}/bin/gnss_app_client.bin
   ${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${DR_OPTIONS} ${S}/test_app/gnss_socket_client.c -o ${B}/bin/gnss_socket_client.bin
   ${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/test_app/gnss_socket_redirect.c -o ${B}/bin/gnss_socket_redirect.bin
   ${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/test_app/gnss_standalone_read.c -o ${B}/bin/gnss_standalone_read.bin
   ${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/test_app/gnss_uart_download.c -o ${B}/bin/gnss_uart_download.bin
   ${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/test_app/gnss_teseo3_flasher.c -o ${B}/bin/gnss_teseo3_flasher.bin
   ${CC} ${CFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/test_app/gnss_teseo5_flasher.c -o ${B}/bin/gnss_teseo5_flasher.bin
   ${CXX} ${CPPFLAGS_FOR_BUILD} ${EXTRA_LDFLAGS} ${LDFLAGS} ${S}/test_app/gnss_redirect_to_usb.cpp -o ${B}/bin/gnss_redirect_to_usb.bin
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${B}/bin/*.bin ${D}${bindir}
}
