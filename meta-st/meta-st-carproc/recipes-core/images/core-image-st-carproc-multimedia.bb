SUMMARY = "ST Car Processor multimedia image"

PV = "1.0.0"
PR = "r1"

include core-image-st-carproc.inc

CORE_IMAGE_EXTRA_INSTALL += " \
	${STCORE} \
	${DEBUG_TOOLS} \
	${@bb.utils.contains('MACHINE_FEATURES', 'multimedia', '${MM_FRW}', 'CURRENT_MACHINE_IS_NOT_MULTIMEDIA_COMPATIBLE', d)} \
	"
