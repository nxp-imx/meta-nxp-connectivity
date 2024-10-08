# Copyright (C) 2024 NXP
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Add packages for i.MX Matter baseline components"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit packagegroup

RDEPENDS:${PN} = " \
    matter \
    dcldownloader \
"
