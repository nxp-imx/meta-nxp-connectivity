SUMMARY = "MCXW72 HCI/Spinel single-UART multiplexer daemon (muxd)"
DESCRIPTION = "Userspace daemon that lets a single physical UART between a Linux \
host (i.MX) and an MCXW72 carry both BLE HCI (H4) consumed by BlueZ and \
OpenThread Spinel (HDLC) consumed by ot-daemon. Ships a C implementation (muxd) \
and a stdlib-only Python implementation (muxd.py)."
HOMEPAGE = "https://www.nxp.com"
SECTION = "connectivity"

LICENSE = "CLOSED"

SRC_URI = " \
    file://src/muxd.c \
    file://Makefile \
    file://muxd.py \
    file://mcxw72-ble-ot-rcp-README.md \
    file://mcxw72_binaries/mcxw72_nbu_dyn_full.bin \
    file://mcxw72_binaries/zephyr.elf \
"

inherit deploy

S = "${UNPACKDIR}"

# Build the C daemon
do_compile() {
    oe_runmake CC="${CC}" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}"
}

do_install() {
    # C daemon
    install -d ${D}${sbindir}
    install -m 0755 ${S}/muxd ${D}${sbindir}/muxd

    # Python implementation (installed alongside as an alternative)
    install -m 0755 ${S}/muxd.py ${D}${sbindir}/muxd.py
}

FILES:${PN} += " \
    ${sbindir}/muxd \
    ${sbindir}/muxd.py \
"

RDEPENDS:${PN} += "python3-core"

# Stage the README and MCXW72 flash binaries into the build's deploy directory
# (DEPLOY_DIR_IMAGE/mcxw72-ble-ot-mux). These are intentionally NOT added to
# FILES:${PN}, so they never end up in the i.MX rootfs image; they are only
# available on the build host for flashing the FRDM-MCXW72 board.
do_deploy() {
    install -d ${DEPLOYDIR}/mcxw72-ble-ot-mux
    install -m 0644 ${S}/mcxw72-ble-ot-rcp-README.md \
        ${DEPLOYDIR}/mcxw72-ble-ot-rcp-README.md
    install -m 0644 ${S}/mcxw72_binaries/mcxw72_nbu_dyn_full.bin \
        ${DEPLOYDIR}/mcxw72-ble-ot-mux/mcxw72_nbu_dyn_full.bin
    install -m 0644 ${S}/mcxw72_binaries/zephyr.elf \
        ${DEPLOYDIR}/mcxw72-ble-ot-mux/zephyr.elf
}
addtask deploy after do_compile before do_build

