PN = "storageproxyd"
SUMMARY = "NXP i.MX Securestorage proxy daemon"
DESCRIPTION = "This layer loads the securestorageproxy dameon"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "master"
SRC_URI = "gitsm://github.com/nxp-imx/libtrustymatter.git;protocol=https;branch=${SRCBRANCH}"
SRCREV = "95f8b30d83da8bf05f3e6b776919511f58766540"

FILES:${PN} += "${systemd_system_unitdir}/storageproxyd.service"

DEPENDS += " pkgconfig-native systemd "

inherit cmake pkgconfig

EXTRA_OECMAKE:append = " -DSYSTEMD_UNIT_DIR=${systemd_system_unitdir}"

do_configure:prepend() {
    export PKG_CONFIG_PATH="${STAGING_LIBDIR}/pkgconfig:${STAGING_DATADIR}/pkgconfig:$PKG_CONFIG_PATH"
}
