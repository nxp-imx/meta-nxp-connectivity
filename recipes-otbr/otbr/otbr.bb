PN = "otbr"
SUMMARY = "OTBR on i.MX boards"
DESCRIPTION = "OTBR applications"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=87109e44b2fda96a8991f27684a7349c"

SRC_URI = "gitsm://github.com/openthread/ot-br-posix.git;branch=main"

SRCREV = "67cac466d73a3313dff955464c0279da55428844"

S = "${WORKDIR}/git"
FILES:${PN} += "lib/systemd"
FILES:${PN} += "usr/share"

DEPENDS += " jsoncpp avahi boost pkgconfig-native libnetfilter-queue ipset "
RDEPENDS:${PN} += " jsoncpp libavahi-client mdns radvd libnetfilter-queue ipset "

inherit cmake
EXTRA_OECMAKE = "-GNinja -DOTBR_BORDER_ROUTING=ON -DOTBR_WEB=OFF -DBUILD_TESTING=OFF -DOTBR_DBUS=ON -DOTBR_DNSSD_DISCOVERY_PROXY=ON -DOTBR_SRP_ADVERTISING_PROXY=ON -DOT_THREAD_VERSION=1.2 -DOTBR_INFRA_IF_NAME=mlan0 -DOT_POSIX_CONFIG_RCP_BUS=SPI"

do_install:append() {
    install -m 755 ${WORKDIR}/build/tools/pskc ${D}${sbindir}/pskc
}
