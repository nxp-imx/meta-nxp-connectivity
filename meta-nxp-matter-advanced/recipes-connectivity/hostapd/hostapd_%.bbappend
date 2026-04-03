FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

do_compile:prepend() {
    echo "CONFIG_NXP_EASYMESH=y" >> ${B}/hostapd/.config
}
