PN = "storageproxyd"
SUMMARY = "NXP i.MX Securestorage proxy daemon"
DESCRIPTION = "This layer loads the securestorageproxy dameon"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRCBRANCH = "master"
SRC_URI = "gitsm://androidsource.ap.freescale.net/project/libtrustymatter.git;protocol=http;branch=${SRCBRANCH}"
SRCREV = "df112aa09f50610c67020463c4dbb39fbdac4c8f"

S = "${WORKDIR}/git"
FILES:${PN} += "lib/systemd"

DEPENDS += " pkgconfig-native systemd "

inherit cmake
