SUMMARY = "Matter IDL Python parser from Project CHIP"
HOMEPAGE = "https://github.com/project-chip/connectedhomeip"
LICENSE = "Apache-2.0"
PN = "matter-idl"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

SRC_URI = "git://github.com/project-chip/connectedhomeip.git;protocol=https;branch=master"
SRCREV = "9151e79d6b5c010f18e3a3da0b048f38196f06c6"
BBCLASSEXTEND = "native"

inherit setuptools3

S = "${UNPACKDIR}/matter-idl-1.0/scripts/py_matter_idl"

RDEPENDS:${PN} += " \
    python3-click-native \
    python3-coloredlogs-native \
"
