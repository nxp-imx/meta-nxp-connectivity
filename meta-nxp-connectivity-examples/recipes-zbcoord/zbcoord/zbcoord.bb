SUMMARY = "Zigbee Coordinator for Linux"
DESCRIPTION = "This recipe builds the Zigbee Coordinator for Linux."
LICENSE = "CLOSED"
PN = "zbcoord"

SRC_URI = "file://zb_coord_linux_mr4.tar.gz"

S = "${WORKDIR}"

INSANE_SKIP:${PN} += "file-rdeps"
INSANE_SKIP:${PN} += "arch"

FILES_${PN} += "/usr/bin/zb_coord_linux"

do_compile() {
}

do_install() {
   install -d ${D}${bindir}
   install -m 0755 ${S}/zb_coord_linux ${D}${bindir}
}

#force skip qa due the 32bit prebuilt binary
do_package_qa() {
}
