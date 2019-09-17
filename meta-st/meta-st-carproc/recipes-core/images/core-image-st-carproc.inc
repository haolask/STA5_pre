DISABLE_GPLv3_PACKAGES ?= ""

include ${DISABLE_GPLv3_PACKAGES}core-image-st-carproc-optional.inc
include license.inc

OPTIONAL_DEBUG_TOOLS ?= ""
OPTIONAL_MM_TOOLS ?= ""
GPU_IMAGE_INSTALL ?= ""
BL32_SP ?= ""
ETAL_PKG ?= ""

STCORE = " \
	coreutils \
	grep \
	sed \
	util-linux \
	dbus \
	openssl \
	cryptodev-linux \
	cryptodev-module \
	sec-tools \
	st-usb \
	udev-extraconf \
	iproute2 \
	alsa-modprobe \
	libasound \
	alsa-conf \
	alsa-utils \
	${@bb.utils.contains("LICENSE_FLAGS_WHITELIST", "commercial", "mpg123", "", d)} \
	crda \
	hostapd \
	systemd-analyze \
	e2fsprogs-resize2fs \
	gnss-teseo \
	ethtool \
	phytool \
	init-ifupdown \
	libmtp \
	simple-mtpfs \
	fuse \
	${@bb.utils.contains('MACHINE_FEATURES', 'atf', bb.utils.contains('BL32_SP', 'optee', '${OPTEE_BINARIES}', '', d), '', d)} \
	pm-utils \
	"

#Some Debug tools are under GPLv3 and should not be delivered.
DEBUG_TOOLS = " \
	i2c-tools \
	procps \
	iperf3 \
	iproute2 \
	iproute2-tc \
	sysstat \
	tcpdump \
	vlan \
	libnet \
	dhcp-client \
	net-tools \
	lrzsz \
	${OPTIONAL_DEBUG_TOOLS} \
	can-utils \
	stm-trace \
	mtdev \
	powertop \
	spitools \
	"

PROFILING_TOOLS = " \
	valgrind \
	oprofile \
	lmbench \
	"

MM_FRW = "\
	weston \
	weston-init \
	wayland \
	packagegroup-gstreamer1-0 \
	gstreamer1.0-plugins-stm \
	libdrm \
	libdrm-tests \
	tiff \
	libv4l \
	rvc \
	early-audio \
	staudio \
	${GPU_IMAGE_INSTALL} \
	${ETAL_PKG} \
	st-etal-radio-if \
	${OPTIONAL_MM_TOOLS} \
	${@bb.utils.contains('STWESTONSHELL','desktop_shell','','wayland-ivi-extension', d)} \
	${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'weston-examples', '', d)} \
	anim-handler \
	"


TELEMATIC_FRW = "\
	at-handler \
	monkey \
	telematics \
	iptables \
	"

OPTEE_BINARIES = "\
	optee-client \
	optee-test \
	"

IMAGE_FEATURES += " ssh-server-dropbear "

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

IMAGE_ROOTFS_SIZE ?= "8192"
