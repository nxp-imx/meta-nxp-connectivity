PN = "openthread-iwxxx-spi"
SUMMARY = "OT-DAEMON on i.MX boards for IWxxx 3-radios on SPI"
DESCRIPTION = "OPENTHREAD applications"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=543b6fe90ec5901a683320a36390c65f"

python () {
    distro_version = d.getVar('DISTRO_VERSION')
    if "scarthgap" in distro_version:
        d.setVar('S', '${WORKDIR}/git')
    elif "nanbield" in distro_version:
        d.setVar('S', '${WORKDIR}/git')
    else:
        d.setVar('S', '${UNPACKDIR}/git')
}

DEPENDS += " avahi boost readline "
RDEPENDS_${PN} += " libavahi-client readline "

TARGET_CFLAGS += " -Wno-error "

inherit cmake

include iw612_ot_src_rev_opts_patches.inc
EXTRA_OECMAKE += "-DOT_POSIX_RCP_SPI_BUS=ON"
BIN_NAME_PATTERN="-iwxxx-spi"
