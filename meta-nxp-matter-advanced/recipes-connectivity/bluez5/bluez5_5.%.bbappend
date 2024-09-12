FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://override.conf"

do_install:append() {
    install -d ${D}${systemd_unitdir}/system/bluetooth.service.d
    install -m 0644 ${WORKDIR}/override.conf ${D}${systemd_unitdir}/system/bluetooth.service.d/override.conf
}

FILES:${PN} += "${systemd_unitdir}/system/bluetooth.service.d/override.conf"
