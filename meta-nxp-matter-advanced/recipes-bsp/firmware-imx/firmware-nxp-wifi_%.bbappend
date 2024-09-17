# Use the latest revision

LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=ca53281cc0caa7e320d4945a896fb837"

IMX_FIRMWARE_SRC ?= "git://github.com/nxp-imx/imx-firmware.git;protocol=https"
SRC_URI = "${IMX_FIRMWARE_SRC};branch=${SRCBRANCH}"
SRCBRANCH = "lf-6.6.36_2.1.0"
SRCREV = "1b26d19284d202b1531837ce37a05afc49ad1d98"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://IW612-Q3-24-R4/sduart_nw61x_v1.bin.se"
SRC_URI += "file://IW612-Q3-24-R4/sd_w61x_v1.bin.se"
SRC_URI += "file://IW612-Q3-24-R4/uartspi_n61x_v1.bin.se"
SRC_URI += "file://IW612-Q3-24-R4/uartuart_n61x_v1.bin.se"
do_install:prepend() {
    rm -f ${S}/nxp/FwImage_IW612_SD/*
    cp ${WORKDIR}/IW612-Q3-24-R4/sduart_nw61x_v1.bin.se ${S}/nxp/FwImage_IW612_SD
    cp ${WORKDIR}/IW612-Q3-24-R4/sd_w61x_v1.bin.se ${S}/nxp/FwImage_IW612_SD
    cp ${WORKDIR}/IW612-Q3-24-R4/uartspi_n61x_v1.bin.se ${S}/nxp/FwImage_IW612_SD
    cp ${WORKDIR}/IW612-Q3-24-R4/uartuart_n61x_v1.bin.se ${S}/nxp/FwImage_IW612_SD
}

do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware/nxp
    oe_runmake install INSTALLDIR=${D}${nonarch_base_libdir}/firmware/nxp
}

FILES:${PN}-nxp8997-common = " \
    ${nonarch_base_libdir}/firmware/nxp/ed_mac_ctrl_V3_8997.conf \
    ${nonarch_base_libdir}/firmware/nxp/txpwrlimit_cfg_8997.conf \
    ${nonarch_base_libdir}/firmware/nxp/uart8997_bt_v4.bin \
"

FILES:${PN}-nxp9098-common = " \
    ${nonarch_base_libdir}/firmware/nxp/ed_mac_ctrl_V3_909x.conf \
    ${nonarch_base_libdir}/firmware/nxp/txpwrlimit_cfg_9098.conf \
    ${nonarch_base_libdir}/firmware/nxp/uart9098_bt_v1.bin \
"

FILES:${PN}-nxpiw612-sdio += " \
    ${nonarch_base_libdir}/firmware/nxp/uartuart_n61x_v1.bin.se \
"

PACKAGES += "${PN}-all-sdio ${PN}-all-pcie"

RDEPENDS:${PN}-all-sdio = " \
    ${PN}-nxp8801-sdio \
    ${PN}-nxp8987-sdio \
    ${PN}-nxp8997-sdio \
    ${PN}-nxp9098-sdio \
    ${PN}-nxpiw416-sdio \
    ${PN}-nxpiw612-sdio \
"

RDEPENDS:${PN}-all-pcie = " \
    ${PN}-nxp8997-pcie \
    ${PN}-nxp9098-pcie \
"

ALLOW_EMPTY:${PN}-all-sdio = "1"
ALLOW_EMPTY:${PN}-all-pcie = "1"
