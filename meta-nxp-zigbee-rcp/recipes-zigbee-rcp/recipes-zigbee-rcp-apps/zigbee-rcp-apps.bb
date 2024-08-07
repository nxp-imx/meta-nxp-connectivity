PN = "zigbee-rcp-apps"
SUMMARY = "Zigbee RCP example applications on i.MX boards for IWxxx 3-radios on SPI"
DESCRIPTION = "Zigbee RCP Applications"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = "file://LICENSE;md5=87109e44b2fda96a8991f27684a7349c"

S = "${WORKDIR}"
FILES:${PN} += "${bindir}"

DEPENDS += "zigbee-rcp-sdk"

SRC_URI = ""
# Zigbee RCP applications
SRC_URI += "file://zigbee-rcp-apps.tar"

inherit pkgconfig cmake
