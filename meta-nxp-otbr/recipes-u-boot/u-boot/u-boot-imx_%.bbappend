FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx93evk imx93evk-iwxxx-matter", 'file://uboot-config/0001-imx93-iw612-otbr-dtb.cfg', '', d)}"

