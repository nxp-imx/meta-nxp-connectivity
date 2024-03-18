PN = "tayga"
SUMMARY = "Tayga on i.MX boards"
DESCRIPTION = "Tayga applications"
LICENSE = "GPL-2.0"
LIC_FILES_CHKSUM = "file://${S}/COPYING;md5=94d55d512a9ba36caa9b7df079bae19f"

SRC_URI = "git://github.com/openthread/tayga.git;branch=master;protocol=https"
SRCREV = "0adcda4096f654318aca81a2b66c371cc0ac5d77"

PATCHTOOL = "git"

inherit autotools
EXTRA_OEMAKE += 'CFLAGS+="-DHAVE_CONFIG_H"'

S = "${WORKDIR}/git"
