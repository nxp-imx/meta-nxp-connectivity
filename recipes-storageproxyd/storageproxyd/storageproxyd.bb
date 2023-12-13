PN = "storageproxyd"
SUMMARY = "NXP i.MX Securestorage proxy daemon"
DESCRIPTION = "This layer loads the securestorageproxy dameon"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "master"
SRC_URI = "gitsm://github.com/nxp-imx/libtrustymatter.git;protocol=https;branch=${SRCBRANCH}"
SRCREV = "95f8b30d83da8bf05f3e6b776919511f58766540"

S = "${WORKDIR}/git"
FILES:${PN} += "/usr/lib/systemd/system/storageproxyd.service"

DEPENDS += " pkgconfig-native systemd "

inherit cmake
