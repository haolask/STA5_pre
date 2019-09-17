FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

#Expected name is MACHINE, without the memory extension
SRC_URI += "file://sta1385-mtp-asound.conf"
SRC_URI += "file://sta1195-evb-asound.conf"

do_install_append() {
  # Install default asound.conf
  install -d ${D}/etc
  PREFIX=$(echo ${MACHINE} | cut -d"-" -f-2)
  if [ -e ${WORKDIR}/${PREFIX}-asound.conf ]; then
    install -m 0644 ${WORKDIR}/${PREFIX}-asound.conf ${D}/etc/asound.conf
  fi
}
