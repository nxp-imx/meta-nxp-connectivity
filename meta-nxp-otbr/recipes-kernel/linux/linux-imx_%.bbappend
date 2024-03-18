FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-matter imx93evk-matter imx6ullevk imx8ulpevk", 'file://0001-Add-Matter-and-OTBR-configs.patch', '', d)}"
SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-matter imx93evk-matter imx6ullevk imx8ulpevk", 'file://0002-Enable-otbr-and-firewall-configs-for-imx6-7.patch', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE', "imx93evk-matter", 'file://0003-imx93-otbr-devicetree.patch', '', d)}"
#SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-matter imx93evk-matter", 'file://0001-MATTER-1869-1-Integrate-trusty-drivers.patch', '', d)}"
#SRC_URI += "${@bb.utils.contains('MACHINE', "imx8mmevk-matter", 'file://0002-MATTER-1869-2-Enable-Trusty-drivers-for-matter.patch', '', d)}"

# patch Makefile for dtb
SRC_URI += "${@bb.utils.contains('MACHINE', "imx8mmevk-iw612-matter", 'file://patches/0001-Add-imx8mm-evk-iw612-otbr-dtb-support.patch', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE', "imx93evk-iw612-matter", 'file://patches/0002-Add-i.MX93-device-tree-to-support-IW612-SPI-RCP.patch', '', d)}"

# Kernel config
SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-iw612-matter imx93evk-iw612-matter", 'file://WCS-kernel-config/0001-otbr-agent-v2-imx-v8.cfg', '', d)}"
SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-iw612-matter imx93evk-iw612-matter", 'file://WCS-kernel-config/0001-otbr-firewall-imx-v8.cfg', '', d)}"

addtask copy_dts after do_unpack before do_prepare_recipe_sysroot
do_copy_dts () {
    if [ -n "${DTS_FILE}" ]; then
        for i in ${DTS_FILE}; do
            if [ -f ${i} ]; then
                echo "do_copy_dts: copying ${i} in ${S}/arch/arm64/boot/dts/freescale"
                cp ${i} ${S}/arch/arm64/boot/dts/freescale/
            fi
        done
    fi
}
