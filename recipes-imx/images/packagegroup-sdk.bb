PACKAGE_ARCH = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS:${PN} += " matter otbr "

IMAGE_FEATURES:remove = " ssh-server-dropbear packagegroup-core-weston packagegroup-core-x11 packagegroup-core-x11-sato packagegroup-core-tools-testapps packagegroup-core-nfs-server packagegroup-core-nfs-client packagegroup-core-eclipse-debug"
