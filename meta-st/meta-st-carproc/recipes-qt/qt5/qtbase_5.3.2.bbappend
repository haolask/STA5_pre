FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-Fix-issue-building-qtbase-5.3.patch"

INSANE_SKIP_${PN}-examples-dev += "dev-elf"
