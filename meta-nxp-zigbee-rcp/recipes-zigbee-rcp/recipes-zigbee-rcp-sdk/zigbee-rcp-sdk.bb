PN = "zigbee-rcp-sdk"
SUMMARY = "Zigbee RCP SDK on i.MX boards for IWxxx 3-radios on SPI"
DESCRIPTION = "Zigbee RCP SDK"
LICENSE = "CLOSED"
#LIC_FILES_CHKSUM = "file://LICENSE;md5=87109e44b2fda96a8991f27684a7349c"

S = "${WORKDIR}"
FILES:${PN} += "usr/lib/systemd"
PACKAGES = "${PN} ${PN}-dev ${PN}-dbg ${PN}-staticdev"

SRC_URI = ""
# Zigbee tarball
SRC_URI += "file://zigbee-rcp-sdk-IW612-Q3-24-R3.tar"

INSANE_SKIP:${PN} += "already-stripped"

do_install() {
    install -d ${D}${libdir}
    install -m 0644 ${S}/libs/libzboss.a ${D}${libdir}
    install -m 0644 ${S}/libs/libzboss.ed.a ${D}${libdir}

    install -d ${D}${sbindir}
    install -m 0744 ${S}/bin/zb_mux ${D}${sbindir}
    install -m 0744 ${S}/services/usr/sbin/zb_app.sh ${D}${sbindir}
    install -m 0744 ${S}/services/usr/sbin/zb_config.sh ${D}${sbindir}
    install -m 0744 ${S}/services/usr/sbin/zb_mux.sh ${D}${sbindir}

    install -d ${D}${includedir}
    install -d ${D}${includedir}/ha
    install -d ${D}${includedir}/platform
    install -d ${D}${includedir}/zcl
    install -m 0644 ${S}/libs/zb_vendor.h ${D}${includedir}
    install -m 0644 ${S}/include/*.h ${D}${includedir}
    install -m 0644 ${S}/include/ha/*.h ${D}${includedir}/ha
    install -m 0644 ${S}/include/platform/*.h ${D}${includedir}
    install -m 0644 ${S}/include/zcl/*.h ${D}${includedir}/zcl

    install -d ${D}${sysconfdir}
    install -d ${D}${sysconfdir}/default
    install -m 0644 ${S}/services/etc/default/zb_app.env ${D}${sysconfdir}/default
    install -m 0644 ${S}/services/etc/default/zb_mux.env ${D}${sysconfdir}/default

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/services/usr/lib/systemd/system/zb_app.service ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/services/usr/lib/systemd/system/zb_config.service ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/services/usr/lib/systemd/system/zb_mux.service ${D}${systemd_system_unitdir}
}
