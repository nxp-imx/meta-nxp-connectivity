FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

def get_arm_arch(d):
    for arg in (d.getVar('TUNE_FEATURES') or '').split():
        if arg == "cortexa7":
            return ''
        if arg == "armv8a":
            return "file://kernel-config/0001-Add-Matter-and-OTBR-configs.cfg"
    return "file://kernel-config/0001-Add-Matter-and-OTBR-configs.cfg"

SRC_URI += "file://patches/0002-Enable-otbr-and-firewall-configs-for-imx6-7.patch"
SRC_URI += "file://patches/0001-Add-imx8mm-evk-iw612-otbr-dtb-support.patch"
SRC_URI += "file://patches/0002-Add-i.MX91-device-tree-to-support-IW612-SPI-RCP.patch"
SRC_URI += "file://patches/Disable-Power-Save-mode-for-BT.patch"

SRC_URI += "${@get_arm_arch(d)}"

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
