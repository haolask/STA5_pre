SUMMARY = "ST Car Processor image"

PV = "1.0.0"
PR = "r1"

include core-image-st-carproc.inc

CORE_IMAGE_EXTRA_INSTALL += " \
	${STCORE} \
	${DEBUG_TOOLS} \
	${@bb.utils.contains('MACHINE_FEATURES', 'telematic', '${TELEMATIC_FRW}', '', d)} \
	${@bb.utils.contains('DISTRO_FEATURES', 'tpm', 'packagegroup-security-tpm', '', d)} \
	"
