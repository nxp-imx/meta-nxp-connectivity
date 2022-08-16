SUMMARY = "NXP IW612 stuffs for Matter"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://gpl-2.0.txt;md5=ab04ac0f249af12befccb94447c08b77"

SRC_URI = "file://gpl-2.0.txt file://bin_sdw61x file://FwImage"

S = "${WORKDIR}"

do_install () {
    install -d ${D}${datadir}/nxp_iw612
    install -d ${D}${datadir}/nxp_iw612/config
    install -d ${D}/lib/firmware/nxp

    install -m 644 bin_sdw61x/version ${D}${datadir}/nxp_iw612
    install -m 644 bin_sdw61x/mlan.ko ${D}${datadir}/nxp_iw612
    install -m 644 bin_sdw61x/sdw61x.ko ${D}${datadir}/nxp_iw612
    install -m 755 bin_sdw61x/fw_loader_imx_lnx ${D}${datadir}/nxp_iw612
    install -m 755 bin_sdw61x/mlanutl ${D}${datadir}/nxp_iw612
    install -m 755 bin_sdw61x/uaputl.exe ${D}${datadir}/nxp_iw612
    install -m 644 bin_sdw61x/README ${D}${datadir}/nxp_iw612
    install -m 644 bin_sdw61x/README_MLAN ${D}${datadir}/nxp_iw612
    install -m 644 bin_sdw61x/README_UAP ${D}${datadir}/nxp_iw612
    install -m 644 bin_sdw61x/config/* ${D}${datadir}/nxp_iw612/config
    install -m 444 FwImage/* ${D}/lib/firmware/nxp
}

INHIBIT_PACKAGE_STRIP = "1"

FILES:${PN} = "${datadir}/nxp_iw612 /lib/firmware/nxp"

RRECOMMENDS:${PN} = "wireless-tools"
