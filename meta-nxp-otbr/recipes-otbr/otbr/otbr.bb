PN = "otbr"
SUMMARY = "OTBR on i.MX boards"
DESCRIPTION = "OTBR applications"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=87109e44b2fda96a8991f27684a7349c"

PATCHTOOL = "git"

S = "${WORKDIR}/git"
FILES:${PN} += "lib/systemd"
FILES:${PN} += "usr/share"
FILES:${PN} += "usr/lib"

DEPENDS += " jsoncpp avahi boost pkgconfig-native mdns libnetfilter-queue ipset libnftnl nftables protobuf-c protobuf protobuf-native "
RDEPENDS:${PN} += " jsoncpp mdns radvd libnetfilter-queue ipset libnftnl nftables bash protobuf protobuf-c "

def get_rcp_bus(d):
    for arg in (d.getVar('MACHINE') or '').split():
        if "imx93" in arg:
            return '-DOT_POSIX_CONFIG_RCP_BUS=SPI'
    for arg in (d.getVar('OT_RCP_BUS') or '').split():
        if arg == "SPI":
             return '-DOT_POSIX_CONFIG_RCP_BUS=SPI'
        if arg == "UART":
            return '-DOT_POSIX_CONFIG_RCP_BUS=UART'
    return ''

inherit cmake
OT_OTBR_SRC_REV_OPTS_PATCHES_INCLUDE="${@bb.utils.contains_any('MACHINE', "imx8mmevk-iw612-matter imx93evk-iw612-matter", 'iw612_otbr_src_rev_opts_patches.inc', 'ot_otbr_src_rev_opts_patches_standard.inc', d)}"
include ${OT_OTBR_SRC_REV_OPTS_PATCHES_INCLUDE}
EXTRA_OECMAKE += "${@get_rcp_bus(d)}"
