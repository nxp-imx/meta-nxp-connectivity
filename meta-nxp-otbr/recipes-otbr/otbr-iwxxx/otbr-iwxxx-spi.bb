PN = "otbr-iwxxx-spi"
SUMMARY = "OTBR on i.MX boards for IWxxx 3-radios on SPI"
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

do_configure:prepend () {
    export OTBRWEB_PREBUILT_FRONTEND=1
}

inherit cmake
include iw612_otbr_src_rev_opts_patches.inc
SRC_URI += "file://0002-copy-prebuilt-frontend-files-instead-of-build.patch"

EXTRA_OECMAKE += " -DOT_POSIX_CONFIG_RCP_BUS=SPI -DOTBR_WEB=ON "
BIN_NAME_PATTERN="-iwxxx-spi"
