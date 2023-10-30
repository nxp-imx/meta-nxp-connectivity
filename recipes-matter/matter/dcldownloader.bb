DESCRIPTION = "DCL Downloader"
SECTION = "utils"
PRIORITY = "optional"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://dcldownloader.tar.gz"
SRC_URI[sha256sum] = "f90f2f0861e6ab22f7b06d269061e10ef0c1ae12ff1654b2a4a615c2d84e675c"

S = "${WORKDIR}/dcldownloader"
DEPENDS += " jsoncpp curl "

do_compile() {
         oe_runmake
}

do_install() {
         install -d ${D}${bindir}
         install -m 0755 dcldownloader ${D}${bindir}
}
