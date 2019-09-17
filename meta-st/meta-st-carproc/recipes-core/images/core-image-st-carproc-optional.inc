
EXTRA_IMAGEDEPENDS_append = "${@bb.utils.contains('DISABLE_GPLv3_PACKAGES', 'true', '', ' gdb-cross-arm', d)}"

OPTIONAL_DEBUG_TOOLS = "mem-access \
						${@bb.utils.contains('DISABLE_GPLv3_PACKAGES', 'true', '', 'binutils', d)} \
						packagegroup-core-tools-debug \
						"

#	libdrm-tools   

OPTIONAL_MM_TOOLS = " \
	gstreamer1.0-plugins-crypto \
	${@'${IMAGE_QT_MANDATORY_PART}' if os.path.isfile('${OEROOT}/meta-qt5/conf/layer.conf') else ''} \
	${@'${IMAGE_QT_OPTIONAL_PART}' if os.path.isfile('${OEROOT}/meta-qt5/conf/layer.conf') else ''} \
	${@'${IMAGE_QT_EXAMPLES_PART}' if os.path.isfile('${OEROOT}/meta-qt5/conf/layer.conf') else ''} \
	"


#
# QT part addons
#
IMAGE_QT_MANDATORY_PART = " \
	qtbase                  \
	qtbase-plugins          \
	qtbase-tools            \
	qtdeclarative           \
	qtdeclarative-plugins   \
	qtdeclarative-qmlplugins\
	qtdeclarative-tools     \
	qtgraphicaleffects-qmlplugins \
	qtmultimedia            \
	qtmultimedia-plugins    \
	qtmultimedia-qmlplugins \
	qtscript                \
	qtwayland               \
	qtwayland-plugins       \
	fontconfig		    \
	"

#	qtwebkit                

IMAGE_QT_OPTIONAL_PART = " \
	qt3d                    \
	qt3d-qmlplugins         \
	qtsvg                   \
	qtsvg-plugins           \
	qtlocation              \
	qtlocation-qmlplugins   \
	qtlocation-plugins      \
	qtwebkit                \
	qtquickcontrols         \
	qtquickcontrols-qmlplugins \
	qtscript                \
	qtsensors               \
	qtlocation              \
	qtlocation-plugins      \
	qtlocation-qmlplugins   \
	"

IMAGE_QT_EXAMPLES_PART = " \
	qtbase-examples         \
	qtdeclarative-examples  \
	qtmultimedia-examples   \
	qtwayland-examples      \
	qtwebkit-examples-examples \
	qtquickcontrols-examples \
	qtscript-examples       \
	qtsensors-examples      \
	qt3d-examples           \
	qtlocation-examples     \
	qt5nmapcarousedemo      \
	cinematicexperience     \
	qtsmarthome             \
	"

