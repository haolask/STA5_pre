FILESEXTRAPATHS_append := "${THISDIR}/${PN}"

SRC_URI += "file://sd8887_uapsta.bin"

do_install_prepend() {

	cp ${WORKDIR}/sd8887_uapsta.bin mrvl
}
