
FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"
SRC_URI += "file://0001-rules-storage-Disable-information-request-from-mmcbl.patch \
			file://0001-Change-MountFlags-to-shared-to-let-udev-manage-the-a.patch \
			file://0001-units-systemd-hostnamed-Remove-Private-network-optio.patch \
			"

