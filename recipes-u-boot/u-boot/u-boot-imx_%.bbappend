FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "${@bb.utils.contains('MACHINE', "imx93evk-matter", 'file://0001-imx93evk-boot-from-imx93-11x11-evk-iw612-otbr.dtb-fo.patch', '', d)}"

SRC_URI += "${@bb.utils.contains('MACHINE', "imx8mmevk-iw612-matter", 'file://WCS-uboot-config/0001-imx8mm-iw612-otbr-dtb.cfg', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE', "imx93evk-iw612-matter", 'file://WCS-uboot-config/0001-imx93-iw612-otbr-dtb.cfg', '', d)}"
