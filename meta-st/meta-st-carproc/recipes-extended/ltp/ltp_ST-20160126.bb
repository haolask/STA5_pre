SUMMARY = "Linux Test Project"
DESCRIPTION = "The Linux Test Project is a joint project with SGI, IBM, OSDL, and Bull with a goal to deliver test suites to the open source community that validate the reliability, robustness, and stability of Linux. The Linux Test Project is a collection of tools for testing the Linux kernel and related features."
HOMEPAGE = "http://ltp.sourceforge.net"
SECTION = "console/utils"
LICENSE = "GPLv2 & GPLv2+ & LGPLv2+ & LGPLv2.1+ & BSD-2-Clause"
LIC_FILES_CHKSUM = "\
    file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
    file://testcases/kernel/controllers/freezer/COPYING;md5=0636e73ff0215e8d672dc4c32c317bb3 \
    file://testcases/kernel/controllers/freezer/run_freezer.sh;beginline=5;endline=17;md5=86a61d2c042d59836ffb353a21456498 \
    file://testcases/kernel/hotplug/memory_hotplug/COPYING;md5=e04a2e542b2b8629bf9cd2ba29b0fe41 \
    file://testcases/kernel/hotplug/cpu_hotplug/COPYING;md5=e04a2e542b2b8629bf9cd2ba29b0fe41 \
    file://testcases/open_posix_testsuite/COPYING;md5=216e43b72efbe4ed9017cc19c4c68b01 \
    file://testcases/realtime/COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e \
    file://tools/netpipe-2.4/COPYING;md5=9e3781bb5fe787aa80e1f51f5006b6fa \
    file://tools/netpipe-2.4-ipv6/COPYING;md5=9e3781bb5fe787aa80e1f51f5006b6fa \
    file://tools/top-LTP/proc/COPYING;md5=aefc88eb8a41672fbfcfe6b69ab8c49c \
    file://tools/pounder21/COPYING;md5=94d55d512a9ba36caa9b7df079bae19f \
    file://utils/benchmark/kernbench-0.42/COPYING;md5=94d55d512a9ba36caa9b7df079bae19f \
    file://utils/ffsb-6.0-rc2/COPYING;md5=c46082167a314d785d012a244748d803 \
"

DEPENDS = "attr libaio libcap acl openssl zip-native bc-native"
DEPENDS_append_libc-musl = " fts "
EXTRA_OEMAKE_append_libc-musl = " LIBC=musl "
CFLAGS_append_powerpc64 = " -D__SANE_USERSPACE_TYPES__"
CFLAGS_append_mips64 = " -D__SANE_USERSPACE_TYPES__"

inherit externalsrc
EXTERNALSRC_pn-ltp = "${ST_LOCAL_SRC}/ltp/"
EXTERNALSRC_BUILD_pn-ltp = "${ST_LOCAL_SRC}/ltp/"

inherit autotools-brokensep deploy

TARGET_CC_ARCH += "${LDFLAGS}"

export prefix = "/opt/ltp"
export exec_prefix = "/opt/ltp"

PACKAGECONFIG[numa] = "--with-numa, --without-numa, numactl,"
EXTRA_AUTORECONF += "-I ${S}/testcases/realtime/m4"
EXTRA_OECONF = " --with-power-management-testsuite --with-realtime-testsuite "
# ltp network/rpc test cases ftbfs when libtirpc is found
EXTRA_OECONF += " --without-tirpc "

# The makefiles make excessive use of make -C and several include testcases.mk
# which triggers a build of the syscall header. To reproduce, build ltp,
# then delete the header, then "make -j XX" and watch regen.sh run multiple
# times. Its easier to generate this once here instead.
do_compile_prepend () {
	( make -C ${B}/testcases/kernel include/linux_syscall_numbers.h )
}


do_install(){

	install -d ${D}/opt/ltp/
	oe_runmake DESTDIR=${D} SKIP_IDCHECK=1 install

	# fixup not deploy STPfailure_report.pl to avoid confusing about it fails to run
	# as it lacks dependency on some perl moudle such as LWP::Simple
	# And this script previously works as a tool for analyzing failures from LTP
	# runs on the OSDL's Scaleable Test Platform (STP) and it mainly accesses
	# http://khack.osdl.org to retrieve ltp test results run on
	# OSDL's Scaleable Test Platform, but now http://khack.osdl.org unaccessible
	rm -rf ${D}/opt/ltp/bin/STPfailure_report.pl

	# Copy POSIX test suite into ${D}/opt/ltp/testcases by manual
	cp -r testcases/open_posix_testsuite ${D}/opt/ltp/testcases

	# Calculate the NAND size to Externalize or not the ltp stuff.
	# We choosen 490 as threshlod because it is just under 512 and
	# this size could be use in one of our futur targets.

	SIZE_LIMIT=$(echo "490")

	while getopts "e:c:m:" opt ${MKUBIFS_ARGS}
	do
		case $opt in
			e)
				BLOCK_SIZE=$(printf '%d\n' ${OPTARG})
				;;
			c)
				BLOCK_NUMBER=$(printf '%d\n' ${OPTARG})
				;;
		esac
	done

	SIZE_TEST=$(echo ${BLOCK_SIZE}*${BLOCK_NUMBER}/1024/1024 | bc)

	if [ ${SIZE_TEST} \< ${SIZE_LIMIT} ];
	then
		cd ${D}/opt
		tar czf ${WORKDIR}/ltp.tar.gz ltp
		cd -
		rm -rf ${D}/opt/ltp/*
		
		rm -f ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt

		echo "Not enought memory to place ltp stuff in target rootfs - You'll find a tar file under ${DEPLOY_DIR_IMAGE} which is containing the ltp." > ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "" >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "Copy the tar file in an usb key. Uncompress the tar file directly in the USB key." >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "Boot up your soft and plug the USB key. If the USB key is not	automatically mounted you'll have to it. Found the partition name in /dev tree and mount it." >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "Commands :" >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "ls /dev/sd*" >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "mount /dev/sda1 /run/media/sda1 (for example)" >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "cd  /run/media/sda1/ltp" >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt
		echo "At this point, all ltp stuff is availbale (Test SW application and LTP Test Suite." >> ${D}/opt/ltp/WHY_IT_IS_EMPTY_README_ltp.txt

	else
		rm -f ${WORKDIR}/ltp.tar.gz
	fi
}

do_deploy(){

	if [ -e ${WORKDIR}/ltp.tar.gz ]
	then
		install -m 0644 ${WORKDIR}/ltp.tar.gz ${DEPLOYDIR}
		bbwarn "Not enought memory to place ltp stuff in target rootfs - You'll find a tar file under ${DEPLOY_DIR_IMAGE} which is containing the ltp."
	fi
}

RDEPENDS_${PN} = "perl e2fsprogs-mke2fs python-core libaio bash gawk expect ldd"

FILES_${PN}-staticdev += "/opt/ltp/lib/libmem.a"

FILES_${PN} += "/opt/ltp/* /opt/ltp/runtest/* /opt/ltp/scenario_groups/* /opt/ltp/testcases/bin/* /opt/ltp/testcases/bin/*/bin/* /opt/ltp/testscripts/* /opt/ltp/testcases/open_posix_testsuite/* /opt/ltp/testcases/open_posix_testsuite/conformance/* /opt/ltp/testcases/open_posix_testsuite/Documentation/* /opt/ltp/testcases/open_posix_testsuite/functional/* /opt/ltp/testcases/open_posix_testsuite/include/* /opt/ltp/testcases/open_posix_testsuite/scripts/* /opt/ltp/testcases/open_posix_testsuite/stress/* /opt/ltp/testcases/open_posix_testsuite/tools/*"

# Avoid generated binaries stripping. Otherwise some of the ltp tests such as ldd01 & nm01 fails
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
# However, test_arch_stripped is already stripped, so...
INSANE_SKIP_${PN} += "already-stripped"

addtask deploy before do_build after do_install
