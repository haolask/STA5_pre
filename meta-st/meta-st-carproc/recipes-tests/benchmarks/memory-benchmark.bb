DESCRIPTION = "Memory benchmark scripts for A5"
SECTION = "extras"
LICENSE = "CLOSED"
PR = "1.0"

RDEPENDS_${PN} += "bash"
MEMORY_BENCHMARK_PATH = "${ST_LOCAL_TESTS}benchmarks/rootfs"

SRC_URI = "file://${MEMORY_BENCHMARK_PATH}/"
FILES_${PN} = "*"

do_install() {
	if [ $(ls -A ${MEMORY_BENCHMARK_PATH}|wc -c) -ne 0 ]; then
		install -d ${D}
		cp -rf ${MEMORY_BENCHMARK_PATH}/* ${D}
	fi
}

ALLOW_EMPTY_${PN}="1"
