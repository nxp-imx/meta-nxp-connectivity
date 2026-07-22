# Install 98-btnxpuart.rules and btnxpuart.sh
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
RDEPENDS:${PN} += "bash"

SRC_URI:append = " file://98-btnxpuart.rules "
SRC_URI:append = " file://btnxpuart.sh "

do_install:append () {
    if [ -e "${UNPACKDIR}/98-btnxpuart.rules" ]; then
        install -m 0644 ${UNPACKDIR}/98-btnxpuart.rules ${D}${sysconfdir}/udev/rules.d
    fi
    if [ -e "${UNPACKDIR}/btnxpuart.sh" ]; then
        install -d ${D}${sysconfdir}/udev/scripts
        install -m 0744 ${UNPACKDIR}/btnxpuart.sh ${D}${sysconfdir}/udev/scripts/btnxpuart.sh
    fi
}

