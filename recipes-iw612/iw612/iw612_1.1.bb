PN = "iw612"
PR = "r0"
SUMMARY = "NXP IW612 stuffs for Matter"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://gpl-2.0.txt;md5=ab04ac0f249af12befccb94447c08b77"

SRC_URI = "file://gpl-2.0.txt file://etc file://bin_sdw61x file://scripts file://network file://systemd_units file://FwImage"

S = "${WORKDIR}"

do_install () {
    install -d ${D}/etc
    install -d ${D}${datadir}/nxp_iw612/bin_sdw61x
    install -d ${D}${datadir}/nxp_iw612/bin_sdw61x/config
    install -d ${D}${datadir}/nxp_iw612/scripts
    install -d ${D}${datadir}/nxp_iw612/scripts/config
    install -d ${D}/lib/systemd/network
    install -d ${D}/lib/systemd/system
    install -d ${D}/etc/systemd/system/multi-user.target.wants
    install -d ${D}/lib/firmware/nxp

    install -m 644 etc/mrk_release ${D}/etc
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
    install -m 644 network/* ${D}/lib/systemd/network
    install -m 644 systemd_units/* ${D}/lib/systemd/system
    ln -s -r ${D}/lib/systemd/system/iw612-driver.service ${D}/etc/systemd/system/multi-user.target.wants/iw612-driver.service
    ln -s -r ${D}/lib/systemd/system/iw612-client.service ${D}/etc/systemd/system/multi-user.target.wants/iw612-client.service
    ln -s -r ${D}/lib/systemd/system/iw612-initbt.service ${D}/etc/systemd/system/multi-user.target.wants/iw612-initbt.service
    ln -s -r ${D}/lib/systemd/system/iw612-otbr.service ${D}/etc/systemd/system/multi-user.target.wants/iw612-otbr.service
    install -m 444 FwImage/* ${D}/lib/firmware/nxp
    ln -s -r ${D}/lib/firmware/nxp/sduart_nw61x_v1.bin.se.mrk ${D}/lib/firmware/nxp/sduart_nw61x_v1.bin.mrk
    ln -s -r ${D}/lib/firmware/nxp/sd_w61x_v1.bin.se.mrk ${D}/lib/firmware/nxp/sd_w61x_v1.bin.mrk
    ln -s -r ${D}/lib/firmware/nxp/uartspi_n61x_v1.bin.se.mrk ${D}/lib/firmware/nxp/uartspi_n61x_v1.bin.mrk
    ln -s -r ${D}${datadir}/nxp_iw612/bin_sdw61x/config/WlanCalData_ext_RD-IW61x-QFN-IPA-2A_V2_without_diversity.conf ${D}/lib/firmware/nxp/WlanCalData_ext_IW612.conf
}

INHIBIT_PACKAGE_STRIP = "1"

FILES:${PN} = "/etc ${datadir}/nxp_iw612/bin_sdw61x ${datadir}/nxp_iw612/scripts /etc/systemd/system /lib/firmware/nxp /lib/systemd/network /lib/systemd/system"

RDEPENDS:${PN} = "bash"

RRECOMMENDS:${PN} = "wireless-tools"
