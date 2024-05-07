FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "${@bb.utils.contains_any('MACHINE', "imx8mmevk-matter imx93evk-iwxxx-matter ", 'file://0001-MATTER-1869-1-Integrate-trusty-drivers.patch', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE', "imx8mmevk-matter", 'file://0002-MATTER-1869-2-Enable-Trusty-drivers-for-matter.patch', '', d)}"
