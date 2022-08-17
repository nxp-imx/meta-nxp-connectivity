SUMMARY = "NXP IW612 stuffs for Matter"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://gpl-2.0.txt;md5=ab04ac0f249af12befccb94447c08b77"

SRC_URI = "file://gpl-2.0.txt file://bin_sdw61x file://scripts file://systemd_units file://FwImage"

S = "${WORKDIR}"

do_install () {
    install -d ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -d ${D}${datadir}/nxp_iw612/bin_sdw61x/config
    install -d ${D}${datadir}/nxp_iw612/scripts
    install -d ${D}${datadir}/nxp_iw612/scripts/config
    install -d ${D}/lib/systemd/system
    install -d ${D}/lib/firmware/nxp

    install -m 644 bin_sdw61x/version ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 644 bin_sdw61x/mlan.ko ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 644 bin_sdw61x/sdw61x.ko ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 755 bin_sdw61x/fw_loader_imx_lnx ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 755 bin_sdw61x/mlanutl ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 755 bin_sdw61x/uaputl.exe ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 644 bin_sdw61x/README ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 644 bin_sdw61x/README_MLAN ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 644 bin_sdw61x/README_UAP ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -m 644 bin_sdw61x/config/* ${D}${datadir}/nxp_iw612/bin_sdw61x/config
    install -m 755 scripts/*.sh ${D}${datadir}/nxp_iw612/scripts
    install -m 644 scripts/config/* ${D}${datadir}/nxp_iw612/scripts/config
    install -m 644 systemd_units/* ${D}/lib/systemd/system
    install -m 444 FwImage/* ${D}/lib/firmware/nxp
}

INHIBIT_PACKAGE_STRIP = "1"

FILES:${PN} = "${datadir}/nxp_iw612/bin_sdw61x ${datadir}/nxp_iw612/scripts /lib/systemd/system /lib/firmware/nxp"

RRECOMMENDS:${PN} = "wireless-tools"
