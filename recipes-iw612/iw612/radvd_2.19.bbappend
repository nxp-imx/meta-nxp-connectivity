FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://scripts/config/radvd.conf"

do_install:append () {
    install -d ${D}/etc

    install -m 644 ${WORKDIR}/scripts/config/radvd.conf ${D}/etc
}
