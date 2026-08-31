# MATTER-4522: install a WirePlumber drop-in that disables the BLE-MIDI monitor,
# which otherwise breaks Matter BLE commissioning against newer BlueZ peers.
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://50-disable-bluez-midi.conf"

do_install:append() {
    install -d ${D}${datadir}/wireplumber/wireplumber.conf.d
    install -m 0644 ${UNPACKDIR}/50-disable-bluez-midi.conf \
        ${D}${datadir}/wireplumber/wireplumber.conf.d/50-disable-bluez-midi.conf
}

FILES:${PN} += "${datadir}/wireplumber/wireplumber.conf.d/50-disable-bluez-midi.conf"
