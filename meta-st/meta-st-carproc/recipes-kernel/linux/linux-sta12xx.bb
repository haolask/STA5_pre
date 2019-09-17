inherit externalsrc

require recipes-kernel/linux/linux-yocto.inc
require linux-deploy.inc
include linux-config.inc
include linux-install.inc

DEPENDS += " u-boot-key-gen-native "

EXTERNALSRC_pn-${PN} ?= "${ST_LOCAL_SRC}linux/"

#Get do_patch task back for perf install
SRCTREECOVEREDTASKS_pn-${PN} = "do_fetch"

COMPATIBLE_MACHINE = "${MACHINE}"

LINUX_VERSION = "4.9.153"

PV = "${LINUX_VERSION}"

LINUX_VERSION_EXTENSION ?= "-sta1xxx"

MM_ENABLE = "${@bb.utils.contains('MACHINE_FEATURES', 'multimedia', 'ON', 'OFF', d)}"

do_deploy_append() {
    if [ "${MM_ENABLE}" = "ON" ]; then
	#Install a dtsi source file in the deploy dir if machine is Multimedia compatible
	install -m 0644 ${S}/arch/arm/boot/dts/sta1295-audio.dtsi ${DEPLOYDIR}/sta1295-audio.dtsi
    fi
}

#Header installation dependency
do_install[depends] += "virtual/kernel:do_shared_workdir"

#Add ncurses build to execute menuconfig
#  somehow, the already added ncurses native is sometimes not enough
do_menuconfig[depends] += "ncurses:do_populate_sysroot"

#disable execution of some kernel tasks
do_kernel_configme[noexec] = "1"
do_kernel_checkout[noexec] = "1"
do_validate_branches[noexec] = "1"
