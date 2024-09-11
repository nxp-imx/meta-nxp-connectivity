# Copyright (C) 2024 NXP
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Add packages for i.MX Matter Zigbee components"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit packagegroup

RDEPENDS:${PN} += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-matter imx8mnddr3levk-matter imx8mnevk-matter imx8mpevk-matter imx93evk-iwxxx-matter ", ' openthread-iwxxx-uart otbr-iwxxx-uart zigbee-rcp-sdk zigbee-rcp-apps ', '', d)}"
