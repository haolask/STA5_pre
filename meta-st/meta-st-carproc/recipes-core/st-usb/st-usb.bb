DESCRIPTION = "usb gadget start script for A5"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

RDEPENDS_${PN} = "bash"

USB_SCRIPT_PATH = "${OEROOT}/meta-st/meta-st-carproc/binary/"
USB_SCRIPT_PATH[vardepsexclude] = "OEROOT"

USB_GADGET_SCRIPT = "a5_usb_gadget.sh"
USB_CHANGE_MODE_SCRIPT = "usb_change_mode.sh"
USB_CAT_MODE_SCRIPT = "usb_cat_mode.sh"

SRC_URI = "file://${USB_SCRIPT_PATH}/${USB_GADGET_SCRIPT} \
		   file://${USB_SCRIPT_PATH}/${USB_CHANGE_MODE_SCRIPT} \
		   file://${USB_SCRIPT_PATH}/${USB_CAT_MODE_SCRIPT}"

FILES_${PN} = "${bindir}/${USB_GADGET_SCRIPT} \
			   ${bindir}/${USB_CHANGE_MODE_SCRIPT} \
			   ${bindir}/${USB_CAT_MODE_SCRIPT}"

do_install() {
	install -d ${D}${bindir}
	install -m 0777 ${USB_SCRIPT_PATH}/${USB_GADGET_SCRIPT} ${D}${bindir}/${USB_GADGET_SCRIPT}
	install -m 0777 ${USB_SCRIPT_PATH}/${USB_CHANGE_MODE_SCRIPT} ${D}${bindir}/${USB_CHANGE_MODE_SCRIPT}
	install -m 0777 ${USB_SCRIPT_PATH}/${USB_CAT_MODE_SCRIPT} ${D}${bindir}/${USB_CAT_MODE_SCRIPT}
}
