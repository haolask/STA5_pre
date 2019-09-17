FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

PATCH_VERSION = "5.3.99+"
PATCH_NAME = "file://0001-Fix-narrowing-errors-found-with-clang.patch"

python __anonymous () {
    pv = d.getVar('PV', True)
    if pv.startswith(d.getVar('PATCH_VERSION', True)):
        d.setVar('SRC_URI', d.getVar('SRC_URI', False) + ' ' + d.getVar('PATCH_NAME', True))
}
