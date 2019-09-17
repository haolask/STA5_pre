FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI_append = " \
	file://0001-STM-bufferpool-add-error-trace-when-discarding-buffe.patch \
	file://0002-STM-tee-forward-allocation-query-downstream.patch \
	file://0003-STM-identity-add-a-property-to-set-the-size-to-dump.patch \
"
