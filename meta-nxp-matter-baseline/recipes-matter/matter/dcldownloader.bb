DESCRIPTION = "DCL Downloader"
SECTION = "utils"
PRIORITY = "optional"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

python () {
    distro_version = d.getVar('DISTRO_VERSION')
    if "scarthgap" in distro_version:
        d.setVar('WORKPATH', '${WORKDIR}')
    elif "nanbield" in distro_version:
        d.setVar('WORKPATH', '${WORKDIR}')
    else:
        d.setVar('WORKPATH', '${UNPACKDIR}')
        d.setVar('S', '${UNPACKDIR}')
}


SRC_URI = "file://dcldownloader.tar.gz"
SRC_URI[sha256sum] = "ada4ecc67e807614256bad84f49cad732d35cd8991958c0c3a999c61e0caf97b"

DEPENDS += " jsoncpp curl "

do_compile() {
         cd ${WORKPATH}
         oe_runmake
}

do_install() {
         install -d ${D}${bindir}
         install -m 0755 ${WORKPATH}/dcldownloader ${D}${bindir}
}
