DESCRIPTION = "memory mapping .h file generation"
SECTION = "extras"
LICENSE = "GPLv2"
PR = "1.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

DEPENDS += "python-native"
inherit deploy pythonnative

MEMORY_MAPPING_CONFIG_NAME = "${@os.path.basename(MEMORY_MAPPING_CONFIG)}"
MEMORY_MAPPING_CONFIG_PATH = "${@os.path.dirname(MEMORY_MAPPING_CONFIG)}"
FILESEXTRAPATHS_prepend = "${MEMORY_MAPPING_CONFIG_PATH}:"
SRC_URI = "file://${MEMORY_MAPPING_CONFIG_NAME}"
FILESEXTRAPATHS_prepend = "${ST_LOCAL_TOOLS}/regdec:"
SRC_URI += "file://regdec.py"

FILES_${PN} += "sta_mem_map/*"

do_compile () {
    python ${WORKDIR}/regdec.py -p ${WORKDIR} -m spirit -a print -o c -f ${WORKDIR}/${MEMORY_MAPPING_CONFIG_NAME} > sta_mem_map.h
    python ${WORKDIR}/regdec.py -p ${WORKDIR} -m spirit -a print -o plot -f ${WORKDIR}/${MEMORY_MAPPING_CONFIG_NAME} > sta_mem_map.txt
}

do_install () {
    mkdir -p ${D}/${includedir}/sta_mem_map
    install -m 0644 sta_mem_map.h ${D}/${includedir}/sta_mem_map/sta_mem_map.h
}

do_deploy () {
    mkdir -p ${DEPLOYDIR}/sta_mem_map
    install -m 0644 ${B}/sta_mem_map.txt ${DEPLOYDIR}/sta_mem_map
    install -m 0644 sta_mem_map.h ${DEPLOYDIR}/sta_mem_map
    install -m 0644 ${WORKDIR}/${MEMORY_MAPPING_CONFIG_NAME} ${DEPLOYDIR}/sta_mem_map/memory_mapping.xml
}
addtask deploy before do_build after do_install
