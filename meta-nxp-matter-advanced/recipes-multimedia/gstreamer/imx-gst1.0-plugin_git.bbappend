FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

FILES:${PN} += "${sysconfdir}/systemd/system/gplay_matter.service"
FILES:${PN} += "${bindir}/gplay_matter-1.0"

SRC_URI += "file://0001-MATTER-1573-Modify-gplay2-to-support-Matter-media.patch"
SRC_URI += "file://0002-MATTER-1573-2-Add-gplay_matter-service.patch"
SRC_URI += "file://0003-MATTER-1573-3-set-volume-to-gplay_matter-by-environm.patch"
SRC_URI += "file://0004-Enhance-the-FIFO-communication.patch"
