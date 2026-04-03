PN = "otbr-iwxxx"
SUMMARY = "OTBR on i.MX boards for IWxxx 3-radios on SPI"
DESCRIPTION = "OTBR applications"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=87109e44b2fda96a8991f27684a7349c"

PATCHTOOL = "git"

FILES:${PN} += "lib/systemd"
FILES:${PN} += "usr/share"
FILES:${PN} += "usr/lib"

DEPENDS += " jsoncpp avahi boost pkgconfig-native mdns libnetfilter-queue ipset libnftnl nftables protobuf-c protobuf protobuf-native dbus "
RDEPENDS:${PN} += " jsoncpp mdns radvd libnetfilter-queue ipset libnftnl nftables bash protobuf protobuf-c "

do_configure:prepend () {
    export OTBRWEB_PREBUILT_FRONTEND=1
}

inherit cmake pkgconfig
include iw612_otbr_src_rev_opts_patches.inc
SRC_URI += "file://0002-copy-prebuilt-frontend-files-instead-of-build.patch"

EXTRA_OECMAKE += " -DCMAKE_CXX_FLAGS="${CXXFLAGS}"" -Wno-error=attributes -Wno-overloaded-virtual""" -DOTBR_WEB=ON "
BIN_NAME_PATTERN="-iwxxx"

INSANE_SKIP:${PN} += "buildpaths"

# GCC15 Compatibility with CMake < 3.5 has been removed from CMake.
EXTRA_OECMAKE += "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
