FILESEXTRAPATHS:append := "${THISDIR}/files:"

SRC_URI += "file://80-wifi-station.network"

RDEPENDS:${PN} += "bash"

do_install:append() {
    # install network configuration
    install -d ${D}${sysconfdir}/systemd/network/
    install -m 0644 ${WORKDIR}/80-wifi-station.network  ${D}${sysconfdir}/systemd/network/
}

FILES:${PN} += "${sysconfdir}/systemd/network/*.network"

