# Copyright (C) 2024 NXP
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Add packages for i.MX Matter openthread components"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit packagegroup

RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mmevk-matter", ' openthread openthread-iwxxx-spi openthread-iwxxx-uart ', '', d)}"

RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx93evk", ' openthread-iwxxx-spi ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx93evk-iwxxx-matter", ' openthread-iwxxx-spi openthread-iwxxx-uart ', '', d)}"

RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx93evk-iwxxx-matter", ' openthread-iwxxx-spi openthread-iwxxx-uart ', '', d)}"

RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx6ul9x9evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx6ull14x14evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx6ull9x9evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx6ulz14x14evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8dxla1-ddr3l-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8dxla1-lpddr4-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8dxlb0-ddr3l-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8dxlb0-fips-lpddr4-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8dxlb0-lpddr4-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8dxlevk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mmevk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mnddr3levk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mnevk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mpevk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mpul-lpddr4-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8mqevk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8ulp-9x9-lpddr4-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx8ulpevk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx91-11x11-lpddr4-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx91evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx95-15x15-lpddr4x-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx95-19x19-lpddr5-evk", ' openthread ', '', d)}"
RDEPENDS:${PN} += "${@bb.utils.contains('MACHINE', "imx95evk", ' openthread ', '', d)}"

